// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/acpi.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/pm_runtime.h>
#include <linux/pm_wakeup.h>
#include <linux/spinlock.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <linux/sched/clock.h>

#include "mtk_common.h"
#include "mtk_dev.h"
#include "mtk_debug.h"
#include "mtk_fsm.h"
#include "mtk_pci.h"
#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
#include "mtk_pwrctl.h"
//#include "pcie-mediatek-gen3.h"
#endif
#ifdef CONFIG_TX00_UT_PM
#include "ut_pm_fake.h"
#endif

#define TAG "PM"

#define LINK_CHECK_RETRY_COUNT		30
#define WAKELOCK_ACTIVE_TIME_MAX_MS	10000

static bool ds_lock_debug;
static bool d3l2_force_dsw;
static unsigned int ds_unlock_work_delay_ms = 100;
static unsigned short runtime_idle_delay_seconds = 20;
extern int mtk_except_linkerr_type;

static int mtk_pm_wait_ds_lock_done(struct mtk_md_dev *mdev, u32 delay)
{
	struct mtk_md_pm *pm = &mdev->pm;
	u32 polling_time = 0;
	u32 reg = 0;

	do {
		/* Delay some time to poll the deep sleep status. */
		udelay(pm->cfg.ds_lock_polling_interval_us);

		reg = mtk_hw_get_ds_status(mdev);
		if ((reg & pm->cfg.ds_lock_check_bitmask) == pm->cfg.ds_lock_check_val)
			return 0;

		polling_time += pm->cfg.ds_lock_polling_interval_us;
		if (mtk_hw_mmio_check(mdev))
			break;
	} while (polling_time < delay);

	MTK_ERR(mdev, "Max polling time %u, actual polling time %u, res_state = 0x%x\n",
		delay, polling_time, reg);

	return -ETIMEDOUT;
}

static int mtk_pm_try_lock_l1ss(struct mtk_md_dev *mdev, bool report)
{
	int ret;

	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), false, L1SS_PM);
	ret = mtk_pm_wait_ds_lock_done(mdev, mdev->pm.cfg.ds_lock_polling_max_us);

	if (ret) {
		MTK_ERR(mdev, "Failed to lock L1ss!\n");
		if (!mtk_hw_mmio_check(mdev))
			mtk_hw_dbg_dump(mdev);
		else if (report) {
			mtk_except_linkerr_type = 1;
			mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
		}
	}

	return ret;
}

static int mtk_pm_reset(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm = &mdev->pm;
	unsigned long flags;

	clear_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag);

	MTK_INFO(mdev, "cancel the delayed unlock workqueue.\n");
	cancel_delayed_work_sync(&pm->ds_unlock_work);
	if (!atomic_read(&pm->ds_lock_refcnt)) {
		spin_lock_irqsave(&pm->ds_spinlock, flags);
		mtk_hw_ds_unlock(mdev);
		spin_unlock_irqrestore(&pm->ds_spinlock, flags);
	} else {
		MTK_WARN(mdev, "ds_lock_refcnt = %d!\n",
			 atomic_read(&pm->ds_lock_refcnt));
	}

	if (!test_bit(PM_F_INIT, &pm->state)) {
		set_bit(PM_F_INIT, &pm->state);
		pm_runtime_get_noresume(mdev->dev);
	}
	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), true, L1SS_PM);

	return 0;
}

static int mtk_pm_init_late(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm = &mdev->pm;

	mtk_hw_unmask_ext_evt(mdev, pm->ext_evt_chs);
	mtk_hw_unmask_irq(mdev, pm->irq_id);
	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), true, L1SS_PM);
	if (!pm_runtime_set_active(mdev->dev))
		MTK_INFO(mdev, "Set runtime active!\n");

	/* Clear init flag */
	if (test_bit(PM_F_INIT, &pm->state)) {
		clear_bit(PM_F_INIT, &pm->state);
		pm_runtime_put_noidle(mdev->dev);
		pm_relax(mdev->dev);
	}

	return 0;
}

static bool mtk_pm_except_handle(struct mtk_md_dev *mdev, bool report)
{
	if (!mtk_hw_mmio_check(mdev)) {
		mtk_hw_dbg_dump(mdev);
	} else {
		/* report EXCEPT_LINK_ERR event if report is true; */
		if (report)
			mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
		return false;
	}

	return true;
}

/**
 * mtk_pm_ds_lock - Lock device power state to prevent it entering deep sleep.
 * @mdev: pointer to mtk_md_dev.
 * @user: user who issues lock request.
 *
 * This function locks device power state, any user who
 * needs to interact with device shall make sure that
 * device is not in deep sleep.
 *
 */
void mtk_pm_ds_lock(struct mtk_md_dev *mdev, enum mtk_user_id user)
{
	struct mtk_md_pm *pm = &mdev->pm;
	unsigned long flags;
	u32 reg;

	if (ds_lock_debug)
		MTK_INFO(mdev, "Requesting pm ds_lock user: %d\n", user);

	spin_lock_irqsave(&pm->ds_spinlock, flags);
	if (atomic_inc_return(&pm->ds_lock_refcnt) == 1) {
		reinit_completion(&pm->ds_lock_complete);
		mtk_hw_ds_lock(mdev);
		if (test_bit(PM_F_INIT, &pm->state) || test_bit(PM_F_SLEEP, &pm->state)) {
			complete_all(&pm->ds_lock_complete);
			goto exit;
		}
		reg = mtk_hw_get_ds_status(mdev);
		/* reg & 0xFE = 0b1111 1110 indicates linkdown,
		 * reg & 0xFE = 0b0001 1110 indicates ds lock is locked.
		 */
		if ((reg & pm->cfg.ds_lock_check_bitmask) == pm->cfg.ds_lock_check_val) {
			complete_all(&pm->ds_lock_complete);
			goto exit;
		}
		if (mdev->hw_ver != 0x4d75 && mdev->hw_ver != 0x4d80)
			mtk_hw_send_sw_evt(mdev, H2D_SW_EVT_PM_LOCK);
		else
			mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_DS_LOCK);

		spin_unlock_irqrestore(&pm->ds_spinlock, flags);
		MTK_DBG(mdev, MTK_DBG_PM, MTK_MEMLOG_RG_0,
			"user: %d, ds_lock_set=%llu, ds_state=0x%x\n",
			user, pm->ds_lock_sent++, reg);

		return;
	}

exit:
	spin_unlock_irqrestore(&pm->ds_spinlock, flags);
}

/**
 * mtk_pm_ds_try_lock - lock and poll device power state.
 * @mdev: pointer to mtk_md_dev.
 * @user: user who issues lock request.
 *
 * This function locks device power state, then poll the lock
 * status for a while.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_ds_try_lock(struct mtk_md_dev *mdev, enum mtk_user_id user)
{
	struct mtk_md_pm *pm = &mdev->pm;
	unsigned long flags;
	int ret = 0;

	if (ds_lock_debug)
		MTK_INFO(mdev, "Requesting pm ds_try_lock user: %d\n", user);

	spin_lock_irqsave(&pm->ds_spinlock, flags);
	if (atomic_inc_return(&pm->ds_lock_refcnt) == 1) {
		mtk_hw_ds_lock(mdev);
		if (test_bit(PM_F_INIT, &pm->state) || test_bit(PM_F_SLEEP, &pm->state))
			goto exit;
		ret = mtk_pm_wait_ds_lock_done(mdev,
					       pm->cfg.ds_lock_polling_min_us);
	}
exit:
	spin_unlock_irqrestore(&pm->ds_spinlock, flags);

	return ret;
}

/**
 * mtk_pm_ds_unlock - Unlock device power state.
 * @mdev: pointer to mtk_md_dev.
 * @user: user who issues unlock request.
 * @no_wait: flag to indicate if device should unlock deep sleep instantly.
 *
 * This function unlocks device power state, after all users
 * unlock device power state, the device will enter deep sleep.
 */
void mtk_pm_ds_unlock(struct mtk_md_dev *mdev, enum mtk_user_id user, bool no_wait)
{
	struct mtk_md_pm *pm = &mdev->pm;
	unsigned long flags;
	u32 unlock_timeout;

	if (ds_lock_debug)
		MTK_INFO(mdev, "Requesting pm ds_unlock user: %d\n", user);

	unlock_timeout = ds_unlock_work_delay_ms;
	spin_lock_irqsave(&pm->ds_spinlock, flags);
	if (!atomic_dec_return(&pm->ds_lock_refcnt)) {
		cancel_delayed_work(&pm->ds_unlock_work);
		if (no_wait || !unlock_timeout)
			mtk_hw_ds_unlock(mdev);
		else
			schedule_delayed_work(&pm->ds_unlock_work,
					      msecs_to_jiffies(unlock_timeout));
	}
	spin_unlock_irqrestore(&pm->ds_spinlock, flags);
}

/**
 * mtk_pm_ds_wait_complete -Try to get completion for a while.
 * @mdev: pointer to mtk_md_dev.
 * @user: user id.
 *
 * The function is not interruptible.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_ds_wait_complete(struct mtk_md_dev *mdev, enum mtk_user_id user)
{
	struct mtk_md_pm *pm = &mdev->pm;
	u32 unlock_timeout;
	int res;

	/* 0 if timed out, and positive (at least 1,
	 * or number of jiffies left  till timeout) if completed.
	 */
	unlock_timeout = pm->cfg.ds_lock_wait_timeout_ms;
	for(int i = 0; i < 5; i++) {
		res = wait_for_completion_timeout(&pm->ds_lock_complete, msecs_to_jiffies(10));
		if (res > 0)
			return 0;
		else {
			MTK_INFO(mdev, "mtk_pm_ds_wait_complete fail: %d\n", i);
			mtk_hw_dbg_dump(mdev);
		}
	}
	// res = wait_for_completion_timeout(&pm->ds_lock_complete, msecs_to_jiffies(unlock_timeout));

	// if (res > 0)
	// 	return 0;
	extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
	t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 8/*MODEM_MONITOR_865CREASON*/, 2, 0, 0);
	/* only dump register here */
	res = mtk_pm_except_handle(mdev, false);
	return res ? -ETIMEDOUT : -EIO;
}

/**
 * mtk_pm_ds_try_wait_complete - try to get completion once without
 *                               blocking.
 * @mdev: pointer to mtk_md_dev.
 * @user: user id.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_ds_try_wait_complete(struct mtk_md_dev *mdev, enum mtk_user_id user)
{
	struct mtk_md_pm *pm = &mdev->pm;
	int ret = 0;

	MTK_DBG(mdev, MTK_DBG_PM, MTK_MEMLOG_RG_0, "user id = %d, caller: %ps\n",
		user, __builtin_return_address(0));

	/* try_wait_for_completion returns 0 if completion
	 * is not available,  otherwise 1.
	 */
	ret = try_wait_for_completion(&pm->ds_lock_complete);

	return (ret > 0 ? 0 : -ETIMEDOUT);
}

static void mtk_pm_ds_unlock_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mtk_md_dev *mdev;
	struct mtk_md_pm *pm;
	unsigned long flags;

	pm = container_of(dwork, struct mtk_md_pm, ds_unlock_work);
	mdev = container_of(pm, struct mtk_md_dev, pm);

	spin_lock_irqsave(&pm->ds_spinlock, flags);
	if (!atomic_read(&pm->ds_lock_refcnt))
		mtk_hw_ds_unlock(mdev);
	spin_unlock_irqrestore(&pm->ds_spinlock, flags);
}

/**
 * mtk_pm_entity_register - Register pm entity into mtk_md_pm's list entry.
 * @mdev: pointer to mtk_md_dev.
 * @user: pm entity.
 *
 * After registration, pm entity's related callbacks
 * could be called upon pm event happening.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_entity_register(struct mtk_md_dev *mdev,
			   struct mtk_pm_entity *md_entity)
{
	struct mtk_md_pm *pm = &mdev->pm;
	struct mtk_pm_entity *tmp;

	MTK_INFO(mdev, "md_entity = %d\n", md_entity->user);
	mutex_lock(&pm->entity_mtx);
	list_for_each_entry(tmp, &pm->entities, entry) {
		if (tmp->user == md_entity->user) {
			MTK_WARN(mdev, "md_entity = %d already registered!\n", md_entity->user);
			mutex_unlock(&pm->entity_mtx);
			return -EALREADY;
		}
	}
	list_add_tail(&md_entity->entry, &pm->entities);
	mutex_unlock(&pm->entity_mtx);

	return 0;
}

/**
 * mtk_pm_entity_unregister - Unregister pm entity from mtk_md_pm's list entry.
 * @mdev: pointer to mtk_md_dev.
 * @user: pm entity.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_entity_unregister(struct mtk_md_dev *mdev,
			     struct mtk_pm_entity *md_entity)
{
	struct mtk_pm_entity *tmp, *cursor;
	struct mtk_md_pm *pm = &mdev->pm;

	MTK_INFO(mdev, "md_entity = %d\n", md_entity->user);
	mutex_lock(&pm->entity_mtx);
	list_for_each_entry_safe(cursor, tmp, &pm->entities, entry) {
		if (cursor->user == md_entity->user) {
			list_del(&cursor->entry);
			mutex_unlock(&pm->entity_mtx);
			return 0;
		}
	}
	mutex_unlock(&pm->entity_mtx);
	MTK_WARN(mdev, "md_entity = %d already deleted!\n", md_entity->user);

	return -EALREADY;
}

/**
 * mtk_pm_check_dev_reset - Check if device power off after suspended.
 * @mdev: pointer to mtk_md_dev.
 *
 * Return: true indicates device is powered off after suspended,
 * false indicates device is not powered off after suspended.
 */
bool mtk_pm_check_dev_reset(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm = &mdev->pm;

	return test_bit(PM_RESUME_FROM_L3, &pm->md_pm_flag);
}

static int mtk_pm_reinit(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm = &mdev->pm;

	if (!test_bit(PM_F_INIT, &pm->state)) {
		set_bit(PM_F_INIT, &pm->state);
		pm_runtime_get_noresume(mdev->dev);
		pm_wakeup_event(mdev->dev, WAKELOCK_ACTIVE_TIME_MAX_MS);
	}

	clear_bit(PM_F_SLEEP, &pm->state);

	/* in init stage, no need to report exception event */
	return mtk_pm_try_lock_l1ss(mdev, false);
}

static void mtk_pm_entity_resume_early(struct mtk_md_dev *mdev, bool is_runtime,
				       bool link_ready)
{
	struct mtk_md_pm *pm = &mdev->pm;
	struct mtk_pm_entity *entity;
	int ret;

	list_for_each_entry(entity, &pm->entities, entry) {
		if (test_bit(PM_SUSPEND_LATE_DONE, &entity->flag) && entity->resume_early) {
			ret = entity->resume_early(mdev, entity->param, is_runtime, link_ready);
			if (ret)
				MTK_ERR(mdev, "user:%d resume early failed!\n", entity->user);
		}
		clear_bit(PM_SUSPEND_LATE_DONE, &entity->flag);
	}
}

static void mtk_pm_entity_resume(struct mtk_md_dev *mdev, bool is_runtime, bool link_ready)
{
	struct mtk_md_pm *pm = &mdev->pm;
	struct mtk_pm_entity *entity;
	int ret;

	list_for_each_entry(entity, &pm->entities, entry) {
		if (test_bit(PM_SUSPEND_DONE, &entity->flag) && entity->resume) {
			ret = entity->resume(mdev, entity->param, is_runtime, link_ready);
			if (ret)
				MTK_ERR(mdev, "user:%d resume failed!\n", entity->user);
		}
		clear_bit(PM_SUSPEND_DONE, &entity->flag);
	}
}

static int mtk_pm_entity_suspend(struct mtk_md_dev *mdev, bool is_runtime)
{
	struct mtk_md_pm *pm = &mdev->pm;
	struct mtk_pm_entity *entity;
	int ret;

	list_for_each_entry(entity, &pm->entities, entry) {
		if (entity->suspend) {
			ret = entity->suspend(mdev, entity->param, is_runtime);
			if (ret) {
				MTK_ERR(mdev, "user:%d suspend failed!\n", entity->user);
				return ret;
			}
		}
		set_bit(PM_SUSPEND_DONE, &entity->flag);
	}

	return 0;
}

static int mtk_pm_entity_suspend_late(struct mtk_md_dev *mdev, bool is_runtime)
{
	struct mtk_md_pm *pm = &mdev->pm;
	struct mtk_pm_entity *entity;
	int ret;

	list_for_each_entry(entity, &pm->entities, entry) {
		if (entity->suspend_late) {
			ret = entity->suspend_late(mdev, entity->param, is_runtime);
			if (ret) {
				MTK_ERR(mdev, "user:%d suspend late failed!\n", entity->user);
				return ret;
			}
		}
		set_bit(PM_SUSPEND_LATE_DONE, &entity->flag);
	}

	return 0;
}

static void mtk_pm_timeout_resume(struct mtk_md_dev *mdev, bool is_md)
{
	u32 suspend_timeout, resume_timeout;
	struct mtk_md_pm *pm = &mdev->pm;

	resume_timeout = pm->cfg.resume_wait_timeout_ms;
	if (is_md) {
		reinit_completion(&pm->pm_ack);
		mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_RESUME_REQ);
		if (!wait_for_completion_timeout(&pm->pm_ack,
						 msecs_to_jiffies(resume_timeout)))
			MTK_ERR(mdev, "MD resume timeout in suspend timeout flow!\n");
	} else {
		suspend_timeout = pm->cfg.suspend_wait_timeout_ms;
		/* True indicates that sAP suspend ACK is received. */
		if (wait_for_completion_timeout(&pm->pm_ack_sap,
						msecs_to_jiffies(suspend_timeout))) {
			reinit_completion(&pm->pm_ack_sap);
			mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_RESUME_REQ_AP);
			if (!wait_for_completion_timeout(&pm->pm_ack_sap,
							 msecs_to_jiffies(resume_timeout)))
				MTK_ERR(mdev, "SAP resume timeout in suspend timeout flow!\n");
		} else {
			MTK_ERR(mdev, "sAP resume timeout too!\n");
		}
	}
}

static int mtk_pm_enable_wake(struct mtk_md_dev *mdev, u8 dev_state, u8 system_state, bool enable)
{
#ifdef CONFIG_ACPI
	union acpi_object in_arg[3];
	struct acpi_object_list arg_list = { 3, in_arg };
	struct pci_dev *bridge;
	acpi_status acpi_ret;
	acpi_handle handle;

	if (acpi_disabled) {
		MTK_ERR(mdev, "Unsupported, acpi function isn't enable\n");
		return -ENODEV;
	}

	bridge = pci_upstream_bridge(to_pci_dev(mdev->dev));
	if (!bridge) {
		MTK_ERR(mdev, "Unable to find bridge\n");
		return -ENODEV;
	}

	handle = ACPI_HANDLE(&bridge->dev);
	if (!handle) {
		MTK_ERR(mdev, "Unsupported, acpi handle isn't found\n");
		return -ENODEV;
	}
	if (!acpi_has_method(handle, "_DSW")) {
		MTK_ERR(mdev, "Unsupported, _DSW method isn't supported\n");
		return -ENODEV;
	}

	in_arg[0].type = ACPI_TYPE_INTEGER;
	in_arg[0].integer.value = enable;
	in_arg[1].type = ACPI_TYPE_INTEGER;
	in_arg[1].integer.value = system_state;
	in_arg[2].type = ACPI_TYPE_INTEGER;
	in_arg[2].integer.value = dev_state;
	acpi_ret = acpi_evaluate_object(handle, "_DSW", &arg_list, NULL);
	if (ACPI_FAILURE(acpi_ret))
		MTK_WARN(mdev, "_DSW method fail for parent: %s\n",
			 acpi_format_exception(acpi_ret));
	MTK_INFO(mdev, "_DSW execute successfully\n");

	return 0;
#else
	MTK_ERR(mdev, "Unsupported, CONFIG ACPI hasn't been set to 'y'\n");

	return -ENODEV;
#endif
}

static int mtk_pm_suspend_device(struct mtk_md_dev *mdev, bool is_runtime)
{
	struct mtk_md_pm *pm = &mdev->pm;
	unsigned long flags;
	u32 suspend_timeout;
	int ret;

	MTK_INFO(mdev, "Suspend enter, is_runtime = %d, Suspend_cnt = %u\n",
		 is_runtime, pm->suspend_cnt);

	mtk_fsm_pause(mdev);

	if (test_bit(PM_F_INIT, &pm->state)) {
		MTK_INFO(mdev, "Suspend exit for fsm handshake not done or in EE!\n");
		mtk_fsm_start(mdev);
		return -EBUSY;
	}

	ret = mtk_pm_try_lock_l1ss(mdev, true);
	if (ret) {
		MTK_ERR(mdev, "Failed to lock l1ss when suspend!\n");
		mtk_fsm_start(mdev);
		return -EAGAIN;
	}

	set_bit(PM_F_SLEEP, &pm->state);

	mtk_except_stop(mdev);

	ret = mtk_pm_entity_suspend(mdev, is_runtime);
	if (ret)
		goto err_suspend;

	/* write counter of suspend */
	mtk_hw_write_pm_cnt(mdev, pm->suspend_cnt++, true);
	reinit_completion(&pm->pm_ack);
	mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_SUSPEND_REQ);
	MTK_INFO(mdev, "Send MD suspend req, AP req = %lx\n", pm->md_pm_flag);
	if (test_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag)) {
		reinit_completion(&pm->pm_ack_sap);
		mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_SUSPEND_REQ_AP);
	}

	suspend_timeout = pm->cfg.suspend_wait_timeout_ms;
	ret = wait_for_completion_timeout(&pm->pm_ack, msecs_to_jiffies(suspend_timeout));
	if (!ret) {
		MTK_ERR(mdev, "MD suspend timeout!\n");
		mtk_pm_except_handle(mdev, true);
		mtk_pm_timeout_resume(mdev, false);
		goto err_suspend;
	}
	if (test_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag)) {
		suspend_timeout = pm->cfg.suspend_wait_timeout_sap_ms;
		ret = wait_for_completion_timeout(&pm->pm_ack_sap,
						  msecs_to_jiffies(suspend_timeout));
		if (!ret) {
			MTK_ERR(mdev, "sAP suspend timeout!\n");
			mtk_pm_except_handle(mdev, true);
			mtk_pm_timeout_resume(mdev, true);
			goto err_suspend;
		}
	}

	ret = mtk_pm_entity_suspend_late(mdev, is_runtime);
	if (ret)
		goto err_suspend_late;

	cancel_delayed_work_sync(&pm->ds_unlock_work);
	if (!atomic_read(&pm->ds_lock_refcnt)) {
		spin_lock_irqsave(&pm->ds_spinlock, flags);
		mtk_hw_ds_unlock(mdev);
		spin_unlock_irqrestore(&pm->ds_spinlock, flags);
	} else {
		MTK_WARN(mdev, "ds_lock_refcnt = %d!\n",
			 atomic_read(&pm->ds_lock_refcnt));
	}

	if (is_runtime && d3l2_force_dsw)
		mtk_pm_enable_wake(mdev, 3, 0, true);

	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), true, L1SS_PM);
	mtk_pci_flush_work(mdev);

	MTK_INFO(mdev, "Suspend success.\n");

	return 0;
err_suspend_late:
	mtk_pm_entity_resume_early(mdev, is_runtime, true);
err_suspend:
	mtk_pm_entity_resume(mdev, is_runtime, true);
	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), true, L1SS_PM);
	mtk_fsm_start(mdev);
	mtk_except_start(mdev);
	clear_bit(PM_F_SLEEP, &pm->state);
	return -EAGAIN;
}

static int mtk_pm_do_resume_device(struct mtk_md_dev *mdev, bool is_runtime)
{
	struct mtk_md_pm *pm = &mdev->pm;
	u32 resume_timeout;
	int ret;

	mtk_pm_try_lock_l1ss(mdev, true);

	mtk_pm_entity_resume_early(mdev, is_runtime, true);

	/* write counter of resume */
	mtk_hw_write_pm_cnt(mdev, pm->resume_cnt, false);

	reinit_completion(&pm->pm_ack);
	mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_RESUME_REQ);
	MTK_INFO(mdev, "Send MD resume req, AP req = %lx\n", pm->md_pm_flag);
	if (test_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag)) {
		reinit_completion(&pm->pm_ack_sap);
		mtk_hw_send_ext_evt(mdev, EXT_EVT_H2D_PCIE_PM_RESUME_REQ_AP);
	}

	resume_timeout = pm->cfg.resume_wait_timeout_ms;
	ret = wait_for_completion_timeout(&pm->pm_ack, msecs_to_jiffies(resume_timeout));
	if (!ret) {
		MTK_ERR(mdev, "MD resume timeout!\n");
		mtk_pm_except_handle(mdev, true);
		ret = -ETIMEDOUT;
	}
	if (test_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag)) {
		resume_timeout = pm->cfg.resume_wait_timeout_sap_ms;
		ret = wait_for_completion_timeout(&pm->pm_ack_sap,
						  msecs_to_jiffies(resume_timeout));
		if (!ret) {
			MTK_ERR(mdev, "sAP resume timeout!\n");
			mtk_pm_except_handle(mdev, true);
			ret = -ETIMEDOUT;
		}
	}
	ret = ret > 0 ? 0 : ret;

	clear_bit(PM_F_SLEEP, &pm->state);
	mtk_pm_entity_resume(mdev, is_runtime, true);

	mtk_hw_set_l1ss(mdev, L1SS_BIT_L1(L1SS_PM), true, L1SS_PM);

	mtk_fsm_start(mdev);
	mtk_except_start(mdev);
	clear_bit(PM_F_SLEEP, &pm->state);
	MTK_INFO(mdev, "Resume status = %d, Resume cnt = %u\n", ret, pm->resume_cnt++);

	return 0;
}

static void mtk_pm_hw_reinit_err_handle(struct mtk_md_dev *mdev, bool is_runtime)
{
	mtk_pm_except_handle(mdev, true);
	MTK_ERR(mdev, "Failed to reinit HW in resume routine!\n");
	mtk_pm_entity_resume_early(mdev, is_runtime, false);
	mtk_pm_entity_resume(mdev, is_runtime, false);
	mtk_fsm_start(mdev);
}

static int mtk_pm_resume_device(struct mtk_md_dev *mdev, bool is_runtime, bool atr_init)
{
	enum mtk_pm_resume_state resume_state;
	struct mtk_md_pm *pm = &mdev->pm;
	int ret = 0;

	if (is_runtime && d3l2_force_dsw)
		mtk_pm_enable_wake(mdev, 0, 0, false);

	if (unlikely(test_bit(PM_F_INIT, &pm->state))) {
		clear_bit(PM_F_SLEEP, &pm->state);
		MTK_INFO(mdev, "Resume exit for fsm handshake not done or in EE!\n");
		return 0;
	}

	resume_state = mtk_hw_get_resume_state(mdev);

	if ((resume_state == PM_RESUME_STATE_INIT && atr_init) ||
	    resume_state == PM_RESUME_STATE_L3)
		set_bit(PM_RESUME_FROM_L3, &pm->md_pm_flag);
	else
		clear_bit(PM_RESUME_FROM_L3, &pm->md_pm_flag);

	MTK_INFO(mdev, "Resume Enter: resume state = %d, is_runtime = %d, atr_init = %d\n",
		 resume_state, is_runtime, atr_init);
	switch (resume_state) {
	case PM_RESUME_STATE_INIT:
		if (!atr_init) {
			MTK_INFO(mdev, "Resume without device state change!\n");
			break;
		}
		fallthrough;
	case PM_RESUME_STATE_L3:
		ret = mtk_hw_reinit(mdev, REINIT_TYPE_RESUME);
		if (ret) {
			mtk_pm_hw_reinit_err_handle(mdev, is_runtime);
			return ret;
		}

		mtk_pm_entity_resume_early(mdev, is_runtime, true);
		mtk_pm_entity_resume(mdev, is_runtime, true);

		mtk_fsm_evt_submit(mdev, FSM_EVT_COLD_RESUME,
				   FSM_F_DFLT, NULL, 0, EVT_MODE_TOHEAD);
		/* No need to start except, for hw reinit will do it later. */
		mtk_fsm_start(mdev);
		/* Submit parital reinit */
		mtk_fsm_evt_submit(mdev, FSM_EVT_REINIT,
				   FSM_F_DFLT, NULL, 0, EVT_MODE_BLOCKING);
		MTK_INFO(mdev, "Resume status = %d\n", ret);
		return 0;
	case PM_RESUME_STATE_L2_EXCEPT:
		ret = mtk_hw_reinit(mdev, REINIT_TYPE_RESUME);
		if (ret) {
			mtk_pm_hw_reinit_err_handle(mdev, is_runtime);
			return ret;
		}
		mtk_hw_unmask_irq(mdev, pm->irq_id);
		fallthrough;
	case PM_RESUME_STATE_L1_EXCEPT:
		mtk_pm_entity_resume_early(mdev, is_runtime, true);
		mtk_pm_entity_resume(mdev, is_runtime, true);
		if (!test_bit(PM_F_INIT, &pm->state)) {
			set_bit(PM_F_INIT, &pm->state);
			pm_runtime_get_noresume(mdev->dev);
		}
		mtk_fsm_start(mdev);
		mtk_except_start(mdev);
		MTK_INFO(mdev, "Resume status = %d\n", ret);
		return 0;
	case PM_RESUME_STATE_L2:
		ret = mtk_hw_reinit(mdev, REINIT_TYPE_RESUME);
		if (ret) {
			mtk_pm_hw_reinit_err_handle(mdev, is_runtime);
			return ret;
		}
		mtk_hw_unmask_irq(mdev, pm->irq_id);
		fallthrough;
	case PM_RESUME_STATE_L1:
		break;
	default:
		MTK_INFO(mdev, "Resume but device not ready!\n");
		mtk_pm_entity_resume_early(mdev, is_runtime, false);
		mtk_pm_entity_resume(mdev, is_runtime, false);
		set_bit(PM_F_INIT, &pm->state);
		/* No need to start except, for hw reinit will do it */
		mtk_fsm_start(mdev);
		cancel_delayed_work_sync(&pm->resume_work);
		schedule_delayed_work(&pm->resume_work, HZ);
		return 0;
	}

	return mtk_pm_do_resume_device(mdev, is_runtime);
}

static void mtk_pm_resume_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mtk_md_dev *mdev;
	struct mtk_md_pm *pm;
	int cnt = 0;

	pm = container_of(dwork, struct mtk_md_pm, resume_work);
	mdev = container_of(pm, struct mtk_md_dev, pm);

	do {
		if (!mtk_hw_link_check(mdev))
			break;
		/* Wait for 1 second to check link state. */
		msleep(1000);
		cnt++;
	} while (cnt < LINK_CHECK_RETRY_COUNT);

	if (mtk_hw_link_check(mdev)) {
		mtk_except_report_evt(mdev, EXCEPT_LINK_ERR);
		return;
	}
	mtk_fsm_evt_submit(mdev, FSM_EVT_COLD_RESUME, FSM_F_DFLT, NULL, 0, EVT_MODE_TOHEAD);
	/* FSM_EVT_REINIT is full reinit */
	mtk_fsm_evt_submit(mdev, FSM_EVT_REINIT, FSM_F_FULL_REINIT, NULL, 0, 0);
	MTK_INFO(mdev, "Resume success from L3 within delayed work.\n");
}

int mtk_pm_prepare(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);
	pm_runtime_resume(dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
	dev_pm_set_driver_flags(mdev->dev, DPM_FLAG_NO_DIRECT_COMPLETE);
#else
	dev_pm_set_driver_flags(mdev->dev, DPM_FLAG_NEVER_SKIP);
#endif

	return 0;
}

int mtk_pm_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);

	if (mtk_pcimsg_pci_user_is_busy(mdev)) {
		MTK_INFO(mdev, "pci user is busy.\n");
		dev_pm_set_driver_flags(mdev->dev, DPM_FLAG_SMART_SUSPEND);
#if IS_ENABLED(CONFIG_PM)
		dev->power.runtime_status = RPM_SUSPENDED;
#endif
		return 0;
	}

	return mtk_pm_suspend_device(mdev, false);
}

int mtk_pm_suspend_noirq(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);

	return 0;
}

int mtk_pm_resume(struct device *dev, bool atr_init)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);
	if (mtk_pcimsg_pci_user_is_busy(mdev)) {
		MTK_INFO(mdev, "pci user is busy.\n");
		dev_pm_set_driver_flags(mdev->dev, 0);
#if IS_ENABLED(CONFIG_PM)
		dev->power.runtime_status = RPM_ACTIVE;
#endif
		return 0;
	}

	return mtk_pm_resume_device(mdev, false, atr_init);
}

int mtk_pm_resume_noirq(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);

	return 0;
}

int mtk_pm_freeze(struct device *dev)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);

	MTK_INFO(mdev, "Enter freeze.");

	return mtk_pm_suspend_device(mdev, false);
}

int mtk_pm_freeze_noirq(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);
	MTK_INFO(mdev, "Enter freeze noirq.");

	return 0;
}

int mtk_pm_poweroff(struct device *dev)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);
	MTK_INFO(mdev, "Enter poweroff.");

	return mtk_pm_suspend_device(mdev, false);
}

int mtk_pm_restore(struct device *dev, bool atr_init)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);
	MTK_INFO(mdev, "Enter restore.");

	return mtk_pm_resume_device(mdev, false, atr_init);
}

int mtk_pm_thaw(struct device *dev, bool atr_init)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);
	MTK_INFO(mdev, "Enter thaw.");

	return mtk_pm_resume_device(mdev, false, atr_init);
}

int mtk_pm_thaw_noirq(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);
	MTK_INFO(mdev, "Enter thaw noirq.");

	return 0;
}

int mtk_pm_runtime_suspend(struct device *dev)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);

	return mtk_pm_suspend_device(mdev, true);
}

int mtk_pm_runtime_resume(struct device *dev, bool atr_init)
{
	struct mtk_md_dev *mdev;
	struct pci_dev *pdev;

	pdev = to_pci_dev(dev);
	mdev = pci_get_drvdata(pdev);

	return mtk_pm_resume_device(mdev, true, atr_init);
}

int mtk_pm_runtime_idle(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct mtk_md_dev *mdev;

	mdev = (struct mtk_md_dev *)pci_get_drvdata(pdev);

	MTK_DBG(mdev, MTK_DBG_PM, MTK_MEMLOG_RG_0, "Schedule Runtime Suspend after %d seconds\n",
		runtime_idle_delay_seconds);
	pm_schedule_suspend(dev, runtime_idle_delay_seconds * MSEC_PER_SEC);

	return -EBUSY;
}

void mtk_pm_shutdown(struct mtk_md_dev *mdev)
{
	MTK_INFO(mdev, "Enter shutdown\n");
	mtk_pm_suspend_device(mdev, false);
}

static void mtk_pm_fsm_state_pre_handler(struct mtk_fsm_param *fsm_param, void *data)
{
	struct mtk_md_dev *mdev;
	struct mtk_md_pm *pm;

	pm = data;
	mdev = container_of(pm, struct mtk_md_dev, pm);
	switch (fsm_param->to) {
	case FSM_STATE_OFF:
		if (!mtk_hw_mmio_check(mdev)) {
			mtk_pm_reinit(mdev);
		} else if (!test_bit(PM_F_INIT, &pm->state)) {
			set_bit(PM_F_INIT, &pm->state);
			pm_runtime_get_noresume(mdev->dev);
		}
		break;
	default:
		break;
	}
}

static void mtk_pm_fsm_state_post_handler(struct mtk_fsm_param *fsm_param, void *data)
{
	struct mtk_md_dev *mdev;
	struct mtk_md_pm *pm;

	pm = data;
	mdev = container_of(pm, struct mtk_md_dev, pm);
	switch (fsm_param->to) {
	case FSM_STATE_ON:
		if (fsm_param->evt_id == FSM_EVT_REINIT)
			mtk_pm_reinit(mdev);
		break;
	case FSM_STATE_BOOTUP:
		if (fsm_param->fsm_flag & FSM_F_SAP_HS2_DONE)
			set_bit(PM_NEED_SUSPEND_SAP, &pm->md_pm_flag);
		break;
	case FSM_STATE_READY:
		mtk_pm_init_late(mdev);
		break;

	case FSM_STATE_OFF:
		mtk_pm_reset(mdev);
#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
		if (fsm_param->evt_id == FSM_EVT_SOFT_OFF)
			mtk_pwrctl_remove_dev();
#endif
		break;

	case FSM_STATE_MDEE:
		if (fsm_param->fsm_flag == FSM_F_MDEE_INIT)
			mtk_pm_reinit(mdev);
		break;

	default:
		break;
	}
}

static int mtk_pm_irq_handler(int irq_id, void *data)
{
	unsigned long long now, process_time;
	struct mtk_md_dev *mdev;
	struct mtk_md_pm *pm;

	pm = data;
	mdev = container_of(pm, struct mtk_md_dev, pm);

	mtk_hw_clear_sw_evt(mdev, D2H_SW_EVT_PM_LOCK_ACK);
	mtk_hw_clear_irq(mdev, irq_id);
	complete_all(&pm->ds_lock_complete);
	now = local_clock();
	mtk_hw_unmask_irq(mdev, irq_id);
	process_time = now - mdev->irq_timestamp;
	pr_info("mtk_pci_drv:pm irq start:%llu,end:%llu,process:%llu\n",
		mdev->irq_timestamp, now, process_time);

	return IRQ_HANDLED;
}

static int mtk_pm_ext_evt_handler(u32 status, void *data)
{
	struct mtk_md_pm *pm = (struct mtk_md_pm *)data;
	struct mtk_md_dev *mdev = container_of(pm, struct mtk_md_dev, pm);
	int ret;

	if (status & EXT_EVT_D2H_PCIE_DS_LOCK_ACK) {
		MTK_INFO(mdev, "EXT_EVT_D2H_PCIE_DS_LOCK_ACK received, ds_lock_recv = %d!\n",
			 pm->ds_lock_recv++);
		complete_all(&pm->ds_lock_complete);
	}

	if (status & EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK) {
		MTK_INFO(mdev, "EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK received! PM req status: 0x%x\n",
			 mtk_hw_get_pm_req_status(mdev));
		complete_all(&pm->pm_ack);
	}

	if (status & EXT_EVT_D2H_PCIE_PM_RESUME_ACK) {
		MTK_INFO(mdev, "EXT_EVT_D2H_PCIE_PM_RESUME_ACK received! PM req status: 0x%x\n",
			 mtk_hw_get_pm_req_status(mdev));
		MTK_INFO(mdev, "PM resume user: 0x%x\n", mtk_hw_get_pm_resume_user(mdev));
		complete_all(&pm->pm_ack);
	}

	if (status & EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK_AP) {
		MTK_INFO(mdev, "EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK_AP received!\n");
		complete_all(&pm->pm_ack_sap);
	}

	if (status & EXT_EVT_D2H_PCIE_PM_RESUME_ACK_AP) {
		MTK_INFO(mdev, "EXT_EVT_D2H_PCIE_PM_RESUME_ACK_AP received!\n");
		complete_all(&pm->pm_ack_sap);
	}

	if (status & EXT_EVT_D2H_SOFT_OFF_NOTIFY) {
		MTK_INFO(mdev, "EXT_EVT_D2H_SOFT_OFF_NOTIFY received!\n");
		ret = mtk_fsm_evt_submit(mdev, FSM_EVT_SOFT_OFF, 0, NULL, 0, 0);
		if (ret)
			MTK_ERR(mdev, "Failed to submit SOFT OFF event: %d.\n", ret);
	}

	mtk_hw_clear_ext_evt(mdev, status);
	mtk_hw_unmask_ext_evt(mdev, status);

	return IRQ_HANDLED;
}

/**
 * mtk_pm_init - Initialize pm fields of struct mtk_md_dev.
 * @mdev: pointer to mtk_md_dev.
 *
 * This function initializes pm fields of struct mtk_md_dev,
 * after that the driver is capable of performing pm related
 * functions.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_init(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm = &mdev->pm;
	int irq_id = -1;
	int ret;

	if (!mdev)
		return -EINVAL;

	INIT_LIST_HEAD(&pm->entities);

	spin_lock_init(&pm->ds_spinlock);
	mutex_init(&pm->entity_mtx);

	init_completion(&pm->ds_lock_complete);
	init_completion(&pm->pm_ack);
	init_completion(&pm->pm_ack_sap);

	INIT_DELAYED_WORK(&pm->ds_unlock_work, mtk_pm_ds_unlock_work);
	INIT_DELAYED_WORK(&pm->resume_work, mtk_pm_resume_work);

	atomic_set(&pm->ds_lock_refcnt, 0);
	pm->ds_lock_sent = 0;
	pm->ds_lock_recv = 0;
	pm->suspend_cnt = 0;
	pm->resume_cnt = 0;

	pm->cfg.ds_lock_wait_timeout_ms = 50;
	pm->cfg.suspend_wait_timeout_ms = 1500;
	pm->cfg.resume_wait_timeout_ms = 1500;
	pm->cfg.suspend_wait_timeout_sap_ms = 1500;
	pm->cfg.resume_wait_timeout_sap_ms = 1500;
	pm->cfg.ds_lock_polling_max_us = 10000;
	pm->cfg.ds_lock_polling_min_us = 2000;
	pm->cfg.ds_lock_polling_interval_us = 10;

	if (mdev->hw_ver == 0x0300) {
		pm->cfg.ds_lock_check_bitmask = 0xEF;
		pm->cfg.ds_lock_check_val = 0x2F;
	} else if (mdev->hw_ver == 0x0900) {
		pm->cfg.ds_lock_check_bitmask = 0xFE;
		pm->cfg.ds_lock_check_val = 0xFE;
	} else {
		pm->cfg.ds_lock_check_bitmask = 0xFE;
		pm->cfg.ds_lock_check_val = 0x1E;
	}

	mtk_pm_try_lock_l1ss(mdev, false);
	/* Set init event flag to prevent device from suspending. */
	set_bit(PM_F_INIT, &pm->state);
	device_init_wakeup(mdev->dev, true);

	/* register sw irq for ds lock. */
	if (mdev->hw_ver != 0x4d75 && mdev->hw_ver != 0x4d80) {
		irq_id = mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_PM_LOCK);
		if (irq_id < 0) {
			MTK_ERR(mdev, "Failed to allocate Irq id!\n");
			ret = -EFAULT;
			goto err_start_init;
		}
		ret = mtk_hw_register_irq(mdev, irq_id, mtk_pm_irq_handler, pm);
		if (ret) {
			MTK_ERR(mdev, "Failed to register irq!\n");
			ret = -EFAULT;
			goto err_start_init;
		}
		mtk_hw_unmask_irq(mdev, irq_id);
		pm->irq_id = irq_id;
	}

	/* register mhccif interrupt handler. */
	pm->ext_evt_chs = EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK |
			  EXT_EVT_D2H_PCIE_PM_RESUME_ACK |
			  EXT_EVT_D2H_PCIE_PM_SUSPEND_ACK_AP |
			  EXT_EVT_D2H_PCIE_PM_RESUME_ACK_AP |
			  EXT_EVT_D2H_PCIE_DS_LOCK_ACK |
			  EXT_EVT_D2H_SOFT_OFF_NOTIFY;

	ret = mtk_hw_register_ext_evt(mdev, pm->ext_evt_chs, mtk_pm_ext_evt_handler, pm);
	if (ret) {
		MTK_ERR(mdev, "Failed to register ext event!\n");
		ret = -EFAULT;
		goto err_reg_ext_evt;
	}

	/* register fsm notify callback */
	ret = mtk_fsm_notifier_register(mdev, MTK_USER_PM,
					mtk_pm_fsm_state_pre_handler, pm, FSM_PRIO_1, true);
	if (ret) {
		MTK_ERR(mdev, "Failed to register fsm notifier!\n");
		ret = -EFAULT;
		goto err_reg_fsm_pre_notifier;
		}

	ret = mtk_fsm_notifier_register(mdev, MTK_USER_PM,
					mtk_pm_fsm_state_post_handler, pm, FSM_PRIO_0, false);
	if (ret) {
		MTK_ERR(mdev, "Failed to register fsm notifier!\n");
		ret = -EFAULT;
		goto err_reg_fsm_post_notifier;
	}

	return 0;
err_reg_fsm_post_notifier:
	mtk_fsm_notifier_unregister(mdev, MTK_USER_PM);
err_reg_fsm_pre_notifier:
	mtk_hw_unregister_ext_evt(mdev, pm->ext_evt_chs);
err_reg_ext_evt:
	if (irq_id >= 0)
		mtk_hw_unregister_irq(mdev, irq_id);
err_start_init:
	device_init_wakeup(mdev->dev, false);
	return ret;
}

/**
 * mtk_pm_exit_early - Acquire device ds lock at the beginning
 *                     of driver exit routine.
 * @mdev: pointer to mtk_md_dev.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_exit_early(struct mtk_md_dev *mdev)
{
	/* In kernel device_del, system pm is already removed from pm entry list
	 * and runtime pm is forbidden as well, thus no need to disable
	 * PM here.
	 */

	return mtk_pm_try_lock_l1ss(mdev, false);
}

/**
 * mtk_pm_exit - PM exit cleanup routine.
 * @mdev: pointer to mtk_md_dev.
 *
 * Return: return value is 0 on success, a negative error
 * code on failure.
 */
int mtk_pm_exit(struct mtk_md_dev *mdev)
{
	struct mtk_md_pm *pm;

	if (!mdev)
		return -EINVAL;

	pm = &mdev->pm;

	cancel_delayed_work_sync(&pm->ds_unlock_work);
	cancel_delayed_work_sync(&pm->resume_work);

	mtk_fsm_notifier_unregister(mdev, MTK_USER_PM);

	device_init_wakeup(mdev->dev, false);

	mtk_hw_unregister_ext_evt(mdev, pm->ext_evt_chs);

	if (mdev->hw_ver != 0x4d75 && mdev->hw_ver != 0x4d80)
		mtk_hw_unregister_irq(mdev, pm->irq_id);
	return 0;
}

module_param(d3l2_force_dsw, bool, 0644);
MODULE_PARM_DESC(d3l2_force_dsw, "Force send _DSW in driver");

module_param(runtime_idle_delay_seconds, ushort, 0644);
MODULE_PARM_DESC(runtime_idle_delay_seconds, "RPM idle delay time");

module_param(ds_unlock_work_delay_ms, uint, 0644);
MODULE_PARM_DESC(ds_unlock_work_delay_ms, "DS unlock work delay time");

module_param(ds_lock_debug, bool, 0644);
MODULE_PARM_DESC(ds_lock_debug, "Enable PCIe PM deep sleep debug information");
