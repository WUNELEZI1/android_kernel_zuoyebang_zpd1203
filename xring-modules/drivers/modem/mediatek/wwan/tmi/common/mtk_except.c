// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/pm_runtime.h>

#include "mtk_debug.h"
#include "mtk_fsm.h"
#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
#include "mtk_pwrctl.h"
//#include "pcie-mediatek-gen3.h"
#endif
#ifdef CONFIG_TX00_UT_EXCEPT
#include "ut_except_fake.h"
#endif

#define TAG			"EXCEPT"

#define MTK_EXCEPT_HOST_RESET_TIME		(2)
#define MTK_EXCEPT_SELF_RESET_TIME		(35)
#define MTK_EXCEPT_INIT_FLAG			BIT(0)
#define MTK_EXCEPT_LINK_ERR_IGNORE		BIT(1)
#define MTK_EXCEPT_RESET_TYPE_PLDR		BIT(26)
#define MTK_EXCEPT_RESET_TYPE_FLDR		BIT(27)
bool mtk_except_link_err_trigger_panic;
int mtk_except_linkerr_type = -1;		// 0: no err  1:suspend  2:mtk_cldma_tx  3.mtk_dpmaif_doorbell_work

extern int xring_pcie_disable_data_trans(int port);
extern void mtk_pwrctl_fldr_remove_rescan(struct work_struct *work);

static void mtk_except_start_monitor(struct mtk_md_dev *mdev, unsigned long expires)
{
	struct mtk_md_except *except = &mdev->except;

	if (!timer_pending(&except->timer) && !mtk_hw_get_hp_status(mdev)) {
		except->timer.expires = jiffies + expires;
		add_timer(&except->timer);
		MTK_INFO(mdev, "Add timer to monitor PCI link\n");
	} else {
		MTK_INFO(mdev, "Add timer exists or HotPlug enabled\n");
	}
}

int mtk_except_report_evt(struct mtk_md_dev *mdev, enum mtk_except_evt evt)
{
	struct mtk_md_except *except = &mdev->except;
	int err, val;
	u32 dev_cfg;

	MTK_INFO(mdev, "%ps report evt:%d\n", __builtin_return_address(0), evt);

	if (!(atomic_read(&except->flag) & MTK_EXCEPT_INIT_FLAG))
		return -EFAULT;

	switch (evt) {
	case EXCEPT_LINK_ERR:
#if 1
		{
			//extern void t800_recovery_start(int value1, int value2);
			//t800_recovery_start(4, 1);
			extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
		       t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 5/*MODEM_MONITOR_PCIE_ERR*/, 4, mtk_except_linkerr_type, 0);
		}
		if (!(atomic_read(&except->flag) & MTK_EXCEPT_LINK_ERR_IGNORE)) {
			MTK_INFO(mdev, "EXCEPT_LINK_ERR in\n");
			err = mtk_hw_mmio_check(mdev);
			MTK_INFO(mdev, "EXCEPT_LINK_ERR after mtk_hw_mmio_check: err=%d\n", err);
			if (err) {
#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
				//mtk_pcie_disable_data_trans(MTK_PCIE_PORT_NUM);
				xring_pcie_disable_data_trans(0);
#endif
				//mtk_except_link_err_trigger_panic = true;
				{
					//extern void t800_recovery_start(int value1, int value2);
					//t800_recovery_start(5, 1);
					extern void t800_send_platform_uevent(int id, int sub_id, int value1, int value2, int value3);
					t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 5/*MODEM_MONITOR_PCIE_ERR*/, 5, mtk_except_linkerr_type, 0);
				}
				atomic_or(MTK_EXCEPT_LINK_ERR_IGNORE, &except->flag);
				mtk_except_linkerr_type = 0;
				//mtk_fsm_evt_submit(mdev, FSM_EVT_LINKDOWN, FSM_F_DFLT, NULL, 0, 0);
				schedule_delayed_work(&except->remove_rescan, msecs_to_jiffies(1));
			}
		}
#else
		{
			extern void t800_recovery_start(int value1, int value2);
			t800_recovery_start(4, 0);
		}
#endif
		break;
	case EXCEPT_RGU:
		dev_cfg = mtk_hw_get_dev_cfg(mdev);
		/* delay 20ms to make sure device ready for reset */
		msleep(20);

		val = mtk_hw_get_dev_state(mdev);
		MTK_INFO(mdev, "dev_state:0x%x, fsm state:%d, RGU reboot reason is 0x%x\n",
			 val, mdev->fsm->state, (dev_cfg & 0x1F));

		/* Invalid dev state will trigger PLDR */
		if (val & MTK_EXCEPT_RESET_TYPE_PLDR) {
			except->type = RESET_PLDR;
		} else if (val & MTK_EXCEPT_RESET_TYPE_FLDR) {
			except->type = RESET_FLDR;
		} else if (mdev->fsm->state >= FSM_STATE_READY) {
			MTK_INFO(mdev, "HW reboot\n");
			except->type = RESET_NONE;
		} else if (mdev->hw_ver != 0x0800) {
			MTK_INFO(mdev, "Invalid reset type\n");
			except->type = RESET_PLDR;
		} else {
			MTK_INFO(mdev, "RGU ignored\n");
			break;
		}

		pm_runtime_get_sync(mdev->dev);
		mtk_fsm_evt_submit(mdev, FSM_EVT_DEV_RESET_REQ, FSM_F_DFLT, NULL, 0, 0);
		pm_runtime_put_sync(mdev->dev);
		break;
	case EXCEPT_AER_DETECTED:
		mtk_fsm_evt_submit(mdev, FSM_EVT_AER, FSM_F_DFLT, NULL, 0, EVT_MODE_BLOCKING);
		break;
	case EXCEPT_AER_RESET:
		err = mtk_hw_reset(mdev, RESET_FLDR);
		if (err)
			mtk_hw_reset(mdev, RESET_RGU);
		break;
	case EXCEPT_AER_RESUME:
		mtk_except_start_monitor(mdev, HZ);
		break;
	default:
		break;
	}

	return 0;
}

void mtk_except_start(struct mtk_md_dev *mdev)
{
	struct mtk_md_except *except = &mdev->except;

	mtk_hw_unmask_irq(mdev, except->pci_ext_irq_id);
}

void mtk_except_stop(struct mtk_md_dev *mdev)
{
	struct mtk_md_except *except = &mdev->except;

	mtk_hw_mask_irq(mdev, except->pci_ext_irq_id);
}

#ifdef CONFIG_MTK_WWAN_PWRCTL_SUPPORT
static void mtk_except_pwrctl_handler(enum pwrctl_evt evt, void *data)
{
	struct mtk_md_except *except = data;
	struct mtk_md_dev *mdev = container_of(except, struct mtk_md_dev, except);

	MTK_INFO(mdev, "pwrctl event id:%d\n", evt);

	switch (evt) {
	case PWRCTL_EVT_RESET:
		del_timer_sync(&except->timer);
		mtk_fsm_evt_submit(mdev, FSM_EVT_PWROFF, FSM_F_DFLT, NULL, 0, EVT_MODE_BLOCKING);
		fallthrough;
	case PWRCTL_EVT_PWRON:
		mtk_except_start_monitor(mdev, MTK_EXCEPT_HOST_RESET_TIME * HZ);
		break;
	case PWRCTL_EVT_PWROFF:
		del_timer_sync(&except->timer);
		mtk_fsm_evt_submit(mdev, FSM_EVT_PWROFF, FSM_F_DFLT, NULL, 0, EVT_MODE_BLOCKING);
		break;
	default:
		break;
	}
}
#endif

static void mtk_except_fsm_pre_handler(struct mtk_fsm_param *param, void *data)
{
	struct mtk_md_except *except = data;
	struct mtk_md_dev *mdev;

	mdev = container_of(except, struct mtk_md_dev, except);

	if (param->to == FSM_STATE_OFF)
		mtk_pcimsg_send_msg_to_user(mdev, MTK_PCIMSG_H2C_EXCEPT);
}

static void mtk_except_fsm_post_handler(struct mtk_fsm_param *param, void *data)
{
	struct mtk_md_except *except = data;
	enum mtk_reset_type reset_type;
	struct mtk_md_dev *mdev;
	unsigned long expires;
	u32 dev_cfg;
	int err;

	mdev = container_of(except, struct mtk_md_dev, except);
	MTK_INFO(mdev, "fsm state:%d event id:%d\n", param->to, param->evt_id);

	switch (param->to) {
	case FSM_STATE_POSTDUMP:
		mtk_hw_mask_irq(mdev, except->pci_ext_irq_id);
		mtk_hw_clear_irq(mdev, except->pci_ext_irq_id);
		mtk_hw_unmask_irq(mdev, except->pci_ext_irq_id);
		break;
	case FSM_STATE_READY:
		dev_cfg = mtk_hw_get_dev_cfg(mdev);
		MTK_INFO(mdev, "AEE Config is 0x%x\n", (dev_cfg >> 5) & 0x3);
		break;
	case FSM_STATE_OFF:
		mtk_hw_reset_sys_irq(mdev);

		if (param->evt_id == FSM_EVT_DEV_RESET_REQ)
			reset_type = except->type;
		else if (param->evt_id == FSM_EVT_LINKDOWN)
			reset_type = RESET_FLDR;
		else
			break;

		if (reset_type == RESET_NONE) {
			expires = MTK_EXCEPT_SELF_RESET_TIME * HZ;
		} else {
			err = mtk_hw_reset(mdev, reset_type);
			if (err)
				expires = MTK_EXCEPT_SELF_RESET_TIME * HZ;
			else
				expires = MTK_EXCEPT_HOST_RESET_TIME * HZ;
		}
		atomic_andnot(MTK_EXCEPT_LINK_ERR_IGNORE, &except->flag);
		mtk_except_start_monitor(mdev, expires);
		break;
	default:
		break;
	}
}

static void mtk_except_link_monitor(struct timer_list *timer)
{
	struct mtk_md_except *except = container_of(timer, struct mtk_md_except, timer);
	struct mtk_md_dev *mdev = container_of(except, struct mtk_md_dev, except);
	int err;

	err = mtk_hw_link_check(mdev);
	if (!err) {
		MTK_INFO(mdev, "Append FSM reinit\n");
		mtk_fsm_evt_submit(mdev, FSM_EVT_REINIT, FSM_F_FULL_REINIT, NULL, 0, 0);
		del_timer(&except->timer);
	} else {
		mod_timer(timer, jiffies + HZ);
	}
}

int mtk_except_init(struct mtk_md_dev *mdev)
{
	struct mtk_md_except *except = &mdev->except;

	except->pci_ext_irq_id = mtk_hw_get_irq_id(mdev, MTK_IRQ_SRC_SAP_RGU);

#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
	mtk_pwrctl_event_register_callback(mtk_except_pwrctl_handler, except);
#endif
	mtk_fsm_notifier_register(mdev, MTK_USER_EXCEPT,
				  mtk_except_fsm_pre_handler, except, FSM_PRIO_1, true);
	mtk_fsm_notifier_register(mdev, MTK_USER_EXCEPT,
				  mtk_except_fsm_post_handler, except, FSM_PRIO_0, false);
	timer_setup(&except->timer, mtk_except_link_monitor, 0);
	atomic_set(&except->flag, MTK_EXCEPT_INIT_FLAG);
	INIT_DELAYED_WORK(&except->remove_rescan, mtk_pwrctl_fldr_remove_rescan);
	return 0;
}

int mtk_except_exit(struct mtk_md_dev *mdev)
{
	struct mtk_md_except *except = &mdev->except;

	atomic_set(&except->flag, 0);
	del_timer(&except->timer);
	mtk_fsm_notifier_unregister(mdev, MTK_USER_EXCEPT);
#if IS_ENABLED(CONFIG_MTK_WWAN_PWRCTL_SUPPORT)
	mtk_pwrctl_event_unregister_callback(mtk_except_pwrctl_handler);
#endif

	return 0;
}
