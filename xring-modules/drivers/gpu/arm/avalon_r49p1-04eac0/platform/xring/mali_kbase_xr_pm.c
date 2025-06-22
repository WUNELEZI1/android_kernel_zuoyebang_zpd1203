// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <device/mali_kbase_device.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <platform/xring/mali_kbase_config_platform.h>
#include <platform/xring/mali_kbase_xr_hw_access.h>
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
#include <mali_kbase_xr_mdr.h>
#endif
#include <platform/xring/mali_kbase_xr_dvfs.h>
#include <mali_kbase_xr_pm.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
#include <platform/xring/mali_kbase_xr_power_backup.h>
#endif

#define REGULATOR_MAX_LOOP_COUNT 3
#define STEP_DELAY_TIME 5

#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
static void kbase_xr_dvfs_resume(struct kbase_device *kbdev)
{
	int gpufreq_refcount = 0;
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;
	if (atomic_read(&kbdev->gpu_dev_data.gpufreq_ready) == 1) {
		gpufreq_refcount = atomic_read(&kbdev->gpu_dev_data.gpufreq_refcount);
		if (gpufreq_refcount == 0 && gpufreq_ops->gpufreq_resume) {
			gpufreq_ops->gpufreq_resume();
			atomic_inc(&kbdev->gpu_dev_data.gpufreq_refcount);
		}
	}
}

static void kbase_xr_dvfs_suspend(struct kbase_device *kbdev)
{
	int gpufreq_refcount = 0;
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;

	if (atomic_read(&kbdev->gpu_dev_data.gpufreq_ready) == 1) {
		gpufreq_refcount = atomic_read(&kbdev->gpu_dev_data.gpufreq_refcount);
		if (gpufreq_refcount != 0 && gpufreq_ops->gpufreq_suspend) {
			gpufreq_ops->gpufreq_suspend();
			gpufreq_refcount = atomic_dec_return(&kbdev->gpu_dev_data.gpufreq_refcount);
			if (unlikely(gpufreq_refcount != 0))
				dev_err(kbdev->dev,
					"%s called not match, gpufreq_refcount:[%d]\n",
					__func__, gpufreq_refcount);
		}
	}
}
#endif

static int kbase_xr_regulator_enable(struct kbase_device *kbdev, struct regulator *regulator)
{
	int loop_count = 0;

	while (loop_count < REGULATOR_MAX_LOOP_COUNT && unlikely(regulator_enable(regulator))) {
		dev_err(kbdev->dev, "Failed to enable regulator, retry...%d", ++loop_count);
		udelay(STEP_DELAY_TIME * loop_count);
	}
	if (loop_count == REGULATOR_MAX_LOOP_COUNT) {
		dev_err(kbdev->dev, "Regulator enable failed\n");
		kbase_xr_mdr_system_error(kbdev, NULL, MODID_GPU_REGULATOR_ON_FAIL, 0, 0);
		return -1;
	}
	return 0;
}

static int kbase_xr_regulator_disable(struct kbase_device *kbdev, struct regulator *regulator)
{
	int loop_count = 0;

	while (loop_count < REGULATOR_MAX_LOOP_COUNT && unlikely(regulator_disable(regulator))) {
		dev_err(kbdev->dev, "Failed to disable regulator, retry...%d", ++loop_count);
		udelay(STEP_DELAY_TIME * loop_count);
	}
	if (loop_count == REGULATOR_MAX_LOOP_COUNT) {
		dev_err(kbdev->dev, "Regulator disable failed\n");
		kbase_xr_mdr_system_error(kbdev, NULL, MODID_GPU_REGULATOR_OFF_FAIL, 0, 0);
		return -1;
	}
	return 0;
}


static int kbase_xr_update_power_state(struct kbase_device *kbdev, bool power_on_desired)
{
	enum kbase_xr_power_state prev_state;
	lockdep_assert_held(&kbdev->gpu_dev_data.power_update_lock);
	if (!kbdev->gpu_dev_data.vdd_gpu || !kbdev->gpu_dev_data.gpu_subsys) {
		dev_err(kbdev->dev, "Invalid regulator\n");
		return -EINVAL;
	}
	do {
		prev_state = kbdev->gpu_dev_data.power_state;

		switch (kbdev->gpu_dev_data.power_state) {
		case KBASE_XR_POWER_OFF:
			if (power_on_desired) {
				if (unlikely(kbase_xr_regulator_enable(kbdev,
					kbdev->gpu_dev_data.vdd_gpu)))
					return -EINVAL;
				kbdev->gpu_dev_data.power_state = KBASE_XR_SUBCHIP_ON;
			}
			break;
		case KBASE_XR_SUBCHIP_ON:
			if (unlikely(kbase_xr_regulator_enable(kbdev,
				kbdev->gpu_dev_data.gpu_subsys)))
				return -EINVAL;
			kbdev->gpu_dev_data.power_state = KBASE_XR_SUBSYS_ON;
			break;
		case KBASE_XR_SUBSYS_ON:
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
			/* Resume dvfs after poweron */
			kbase_xr_dvfs_resume(kbdev);
#endif
			kbdev->gpu_dev_data.power_state = KBASE_XR_POWER_ON;
			break;
		case KBASE_XR_POWER_ON:
			if (!power_on_desired) {
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
				/* Suspend dvfs before poweroff */
				kbase_xr_dvfs_suspend(kbdev);
#endif
				if (unlikely(kbase_xr_regulator_disable(kbdev,
					kbdev->gpu_dev_data.gpu_subsys)))
					return -EINVAL;
				kbdev->gpu_dev_data.power_state = KBASE_XR_SUBSYS_OFF;
			}
			break;
		case KBASE_XR_SUBSYS_OFF:
			hrtimer_start(&kbdev->gpu_dev_data.power_off_hrtimer, HR_TIMER_DELAY_MSEC(BUCK_OFF_DELAY_MS), HRTIMER_MODE_REL);
			kbdev->gpu_dev_data.power_state = KBASE_XR_SUBCHIP_PEND_OFF;
			break;
		case KBASE_XR_SUBCHIP_PEND_OFF:
			if (power_on_desired) {
				hrtimer_cancel(&kbdev->gpu_dev_data.power_off_hrtimer);
				kbdev->gpu_dev_data.power_state = KBASE_XR_SUBCHIP_ON;
			}
			break;
		case KBASE_XR_SUBCHIP_OFF:
			kbdev->gpu_dev_data.power_state = KBASE_XR_POWER_OFF;
			break;
		default:
			WARN(1, "Invalid state in power_state: %d", kbdev->gpu_dev_data.power_state);
		}
	} while (kbdev->gpu_dev_data.power_state != prev_state);

	if (kbdev->gpu_dev_data.power_state != prev_state) {
		dev_dbg(kbdev->dev, "Power level transition: %d to %d\n",
				prev_state,
				kbdev->gpu_dev_data.power_state);
	}
	return 0;
}

static inline int kbase_xr_power_on(struct kbase_device *kbdev)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
	ret = kbase_xr_platform_on(kbdev);
	if (ret == -1)
		dev_err(kbdev->dev, "GPU power on backup failed");
#else
	mutex_lock(&kbdev->gpu_dev_data.power_update_lock);
	ret = kbase_xr_update_power_state(kbdev, true);
	mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
#endif
	return ret;
}

static inline int kbase_xr_power_off(struct kbase_device *kbdev)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
	ret = kbase_xr_platform_off(kbdev);
	if (ret == -1)
		dev_err(kbdev->dev, "GPU power off backup failed");
#else
	mutex_lock(&kbdev->gpu_dev_data.power_update_lock);
	ret = kbase_xr_update_power_state(kbdev, false);
	mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
#endif
	return ret;
}
enum hrtimer_restart power_off_timer_callback(struct hrtimer *timer)
{
	struct kbase_device *kbdev =
		container_of(timer, struct kbase_device, gpu_dev_data.power_off_hrtimer);
	queue_work(kbdev->gpu_dev_data.power_off_timer_workq, &kbdev->gpu_dev_data.power_off_timer_work);

	return HRTIMER_NORESTART;
}

static void kbase_xr_subchip_power_off_worker(struct work_struct *work)
{
	struct kbase_device *kbdev =
		container_of(work, struct kbase_device,
			     gpu_dev_data.power_off_timer_work);

	mutex_lock(&kbdev->gpu_dev_data.power_update_lock);

	if (kbdev->gpu_dev_data.power_state != KBASE_XR_SUBCHIP_PEND_OFF) {
		dev_dbg(kbdev->dev, "Subchip power off is breaked by power on");
		mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
		return;
	}

	kbase_xr_regulator_disable(kbdev, kbdev->gpu_dev_data.vdd_gpu);
	kbdev->gpu_dev_data.power_state = KBASE_XR_SUBCHIP_OFF;
	kbase_xr_update_power_state(kbdev, false);
	mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
}

int kbase_xr_power_init(struct kbase_device *kbdev)
{
#if defined(CONFIG_REGULATOR)
	kbdev->gpu_dev_data.vdd_gpu = devm_regulator_get(kbdev->dev, "vdd_gpu");
	if (IS_ERR(kbdev->gpu_dev_data.vdd_gpu)) {
		dev_err(kbdev->dev, "Failed to get vdd_gpu regulator\n");
		return -EINVAL;
	}

	kbdev->gpu_dev_data.gpu_subsys = devm_regulator_get(kbdev->dev, "gpu_subsys");
	if (IS_ERR(kbdev->gpu_dev_data.gpu_subsys)) {
		dev_err(kbdev->dev, "Failed to get gpu_subsys regulator\n");
		return -EINVAL;
	}
#else
	return -ENODEV;
#endif
	mutex_init(&kbdev->gpu_dev_data.power_update_lock);
	atomic_set(&kbdev->gpu_dev_data.regulator_refcount, 0);
	mutex_lock(&kbdev->gpu_dev_data.power_update_lock);
	kbdev->gpu_dev_data.power_state = KBASE_XR_POWER_OFF;
	mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
	kbdev->gpu_dev_data.power_off_timer_workq =
			alloc_workqueue("kbase_power_off_timer_work_queue", WQ_HIGHPRI | WQ_UNBOUND, 1);
	INIT_WORK(&kbdev->gpu_dev_data.power_off_timer_work, kbase_xr_subchip_power_off_worker);
	if (!kbdev->gpu_dev_data.power_off_timer_workq) {
		dev_err(kbdev->dev, "Alloc kbase_power_off_timer_work_queue failed");
		return -ENOMEM;
	}

	hrtimer_init(&kbdev->gpu_dev_data.power_off_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kbdev->gpu_dev_data.power_off_hrtimer.function = &power_off_timer_callback;

	return 0;
}

void kbase_xr_power_term(struct kbase_device *kbdev)
{
	mutex_lock(&kbdev->gpu_dev_data.power_update_lock);
	if (kbdev->gpu_dev_data.power_state == KBASE_XR_SUBCHIP_PEND_OFF) {
		hrtimer_cancel(&kbdev->gpu_dev_data.power_off_hrtimer);
		kbase_xr_regulator_disable(kbdev, kbdev->gpu_dev_data.vdd_gpu);
		kbdev->gpu_dev_data.power_state = KBASE_XR_SUBCHIP_OFF;
		kbase_xr_update_power_state(kbdev, false);
	}
	mutex_unlock(&kbdev->gpu_dev_data.power_update_lock);
	destroy_workqueue(kbdev->gpu_dev_data.power_off_timer_workq);
	mutex_destroy(&kbdev->gpu_dev_data.power_update_lock);
}

static int pm_callback_power_on(struct kbase_device *kbdev)
{
	int ret = 1; /* Assume GPU has been powered off */
	int error;
	unsigned long flags;

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	WARN_ON(kbdev->pm.backend.gpu_powered);
#if MALI_USE_CSF
	if (likely(kbdev->csf.firmware_inited)) {
		WARN_ON(!kbdev->pm.active_count);
		WARN_ON(kbdev->pm.runtime_active);
	}
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	kbase_xr_power_on(kbdev);
	CSTD_UNUSED(error);
#else
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

#ifdef KBASE_PM_RUNTIME
	error = pm_runtime_get_sync(kbdev->dev);
	if (error == 1) {
		/*
		 * Let core know that the chip has not been
		 * powered off, so we can save on re-initialization.
		 */
		ret = 0;
	}
	dev_dbg(kbdev->dev, "pm_runtime_get_sync returned %d\n", error);
#else
	kbase_xr_power_on(kbdev);
#endif /* KBASE_PM_RUNTIME */

#endif /* MALI_USE_CSF */

	return ret;
}

static void pm_callback_power_off(struct kbase_device *kbdev)
{
	unsigned long flags;

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	WARN_ON(kbdev->pm.backend.gpu_powered);
#if MALI_USE_CSF
	if (likely(kbdev->csf.firmware_inited)) {
#ifdef CONFIG_MALI_DEBUG
		WARN_ON(kbase_csf_scheduler_get_nr_active_csgs(kbdev));
#endif
		WARN_ON(kbdev->pm.backend.mcu_state != KBASE_MCU_OFF);
	}
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	/* Power down the GPU immediately */
	kbase_xr_power_off(kbdev);
#else  /* MALI_USE_CSF */
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

#ifdef KBASE_PM_RUNTIME
	pm_runtime_mark_last_busy(kbdev->dev);
	pm_runtime_put_autosuspend(kbdev->dev);
#else
	/* Power down the GPU immediately as runtime PM is disabled */
	kbase_xr_power_off(kbdev);
#endif
#endif /* MALI_USE_CSF */
}

#if MALI_USE_CSF && defined(KBASE_PM_RUNTIME)
static void pm_callback_runtime_gpu_active(struct kbase_device *kbdev)
{
	unsigned long flags;
	int error;

	lockdep_assert_held(&kbdev->pm.lock);

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	WARN_ON(!kbdev->pm.backend.gpu_powered);
	WARN_ON(!kbdev->pm.active_count);
	WARN_ON(kbdev->pm.runtime_active);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	if (pm_runtime_status_suspended(kbdev->dev)) {
		error = pm_runtime_get_sync(kbdev->dev);
	} else {
		/* Call the async version here, otherwise there could be
		 * a deadlock if the runtime suspend operation is ongoing.
		 * Caller would have taken the kbdev->pm.lock and/or the
		 * scheduler lock, and the runtime suspend callback function
		 * will also try to acquire the same lock(s).
		 */
		error = pm_runtime_get(kbdev->dev);
	}

	kbdev->pm.runtime_active = true;
}

static void pm_callback_runtime_gpu_idle(struct kbase_device *kbdev)
{
	unsigned long flags;

	lockdep_assert_held(&kbdev->pm.lock);

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	WARN_ON(!kbdev->pm.backend.gpu_powered);
	WARN_ON(kbdev->pm.backend.l2_state != KBASE_L2_OFF);
	WARN_ON(kbdev->pm.active_count);
	WARN_ON(!kbdev->pm.runtime_active);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	pm_runtime_mark_last_busy(kbdev->dev);
	pm_runtime_put_autosuspend(kbdev->dev);
	kbdev->pm.runtime_active = false;
}
#endif

#ifdef KBASE_PM_RUNTIME
static int kbase_device_runtime_init(struct kbase_device *kbdev)
{
	int ret = 0;

	pm_runtime_set_autosuspend_delay(kbdev->dev, AUTO_SUSPEND_DELAY);
	pm_runtime_use_autosuspend(kbdev->dev);

	pm_runtime_set_active(kbdev->dev);
	pm_runtime_enable(kbdev->dev);

	if (!pm_runtime_enabled(kbdev->dev)) {
		dev_warn(kbdev->dev, "pm_runtime not enabled");
		ret = -EINVAL;
	} else if (atomic_read(&kbdev->dev->power.usage_count)) {
		dev_warn(kbdev->dev,
			 "%s: Device runtime usage count unexpectedly non zero %d",
			__func__, atomic_read(&kbdev->dev->power.usage_count));
		ret = -EINVAL;
	}

	return ret;
}

static void kbase_device_runtime_disable(struct kbase_device *kbdev)
{
	if (atomic_read(&kbdev->dev->power.usage_count))
		dev_warn(kbdev->dev,
			 "%s: Device runtime usage count unexpectedly non zero %d",
			__func__, atomic_read(&kbdev->dev->power.usage_count));

	pm_runtime_disable(kbdev->dev);
}
#endif /* KBASE_PM_RUNTIME */

static int pm_callback_runtime_on(struct kbase_device *kbdev)
{
#if !MALI_USE_CSF
	kbase_xr_power_on(kbdev);
#endif
	return 0;
}

static void pm_callback_runtime_off(struct kbase_device *kbdev)
{
#if !MALI_USE_CSF
	kbase_xr_power_off(kbdev);
#endif
}

static void pm_callback_resume(struct kbase_device *kbdev)
{
	int ret = pm_callback_runtime_on(kbdev);

	WARN_ON(ret);
}

static void pm_callback_suspend(struct kbase_device *kbdev)
{
	pm_callback_runtime_off(kbdev);
}


struct kbase_pm_callback_conf pm_callbacks = {
	.power_on_callback = pm_callback_power_on,
	.power_off_callback = pm_callback_power_off,
	.power_suspend_callback = pm_callback_suspend,
	.power_resume_callback = pm_callback_resume,
#ifdef KBASE_PM_RUNTIME
	.power_runtime_init_callback = kbase_device_runtime_init,
	.power_runtime_term_callback = kbase_device_runtime_disable,
	.power_runtime_on_callback = pm_callback_runtime_on,
	.power_runtime_off_callback = pm_callback_runtime_off,
#else				/* KBASE_PM_RUNTIME */
	.power_runtime_init_callback = NULL,
	.power_runtime_term_callback = NULL,
	.power_runtime_on_callback = NULL,
	.power_runtime_off_callback = NULL,
#endif				/* KBASE_PM_RUNTIME */

#if MALI_USE_CSF && defined(KBASE_PM_RUNTIME)
	.power_runtime_gpu_idle_callback = pm_callback_runtime_gpu_idle,
	.power_runtime_gpu_active_callback = pm_callback_runtime_gpu_active,
#else
	.power_runtime_gpu_idle_callback = NULL,
	.power_runtime_gpu_active_callback = NULL,
#endif
};

void kbase_xr_config_memory_repair(struct kbase_device *kbdev)
{
	u64 shader_present = 0;
	u64 shader_ready = 0;
	u64 shader_trans = 0;
	u64 mem_repair_mask = 0;
	u32 mem_repair_count = 0;
	u32 mem_repair_val = 0;
	u32 shader_num = 0;
	u32 i = 0;
	unsigned long flags = 0;
	const u32 timeout_us = 10000000;
	const u32 delay_us = 1;
	int err;

	dev_info(kbdev->dev, "Start to change mali gpu memory repair mode.\n");
	shader_present = kbdev->gpu_props.shader_present;
	if (shader_present == 0) {
		dev_err(kbdev->dev, "Invalid shader present for mali gpu memory repair.\n");
		return;
	}
	shader_num = hweight64(shader_present);
	/* Power on all the shader cores. */
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	kbase_reg_write64(kbdev, GPU_CONTROL_ENUM(SHADER_PWRON), shader_present);
	err = kbase_reg_poll64_timeout(kbdev, GPU_CONTROL_ENUM(SHADER_READY), shader_ready,
			    (shader_ready & shader_present) == shader_present, delay_us, timeout_us, false);
	if (err) {
		spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
		dev_err(kbdev->dev, "Mali gpu shader core power up timeout\n");
		return;
	}
	shader_ready = kbase_reg_read64(kbdev, GPU_CONTROL_ENUM(SHADER_READY));
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	dev_info(kbdev->dev, "Mali gpu shader core power up success.\n");
	/*
	 * Calculate the memory repair mask, the shader present of krake is unique, and the
	 * corresponding bit mask of gpu_ctrl is unique always.
	 */
	for (i = 0; i < 64; ++i) {
		if (shader_ready & (1ul << i)) {
			mem_repair_mask |= 1ul << mem_repair_count;
			++mem_repair_count;
			if (mem_repair_count >= shader_num)
				break;
		}
	}

	/* Read back the value of gpu_ctrl. */
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	mem_repair_val = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg,
			GPU_CTRL_MEMREPAIR_OFFSET);
	/* Write the final value with the memory repair mask. */
	mem_repair_val |= mem_repair_mask;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, GPU_CTRL_MEMREPAIR_OFFSET,
		mem_repair_val);

	/* Power off all the shader cores. */
	kbase_reg_write64(kbdev, GPU_CONTROL_ENUM(SHADER_PWROFF), shader_present);

	/* wait for power off complete */
	err = kbase_reg_poll64_timeout(kbdev, GPU_CONTROL_ENUM(SHADER_READY), shader_ready,
			    (shader_ready & shader_present) == 0, delay_us, timeout_us, false);
	if (err) {
		spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
		dev_err(kbdev->dev, "Mali gpu shader ready power off timeout\n");
		return;
	}

	err = kbase_reg_poll64_timeout(kbdev, GPU_CONTROL_ENUM(SHADER_PWRTRANS), shader_trans,
			    (shader_trans & shader_present) == 0, delay_us, timeout_us, false);
	if (err) {
		spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
		dev_err(kbdev->dev, "Mali gpu shader trans power off timeout\n");
		return;
	}

	/* Read back again to print the actual value of gpu_ctrl. */
	mem_repair_val = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg,
			GPU_CTRL_MEMREPAIR_OFFSET);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
	dev_info(kbdev->dev,
		"Mali gpu memory repair mode change success, value: 0x%x\n", mem_repair_val);
}
KBASE_EXPORT_TEST_API(kbase_xr_config_memory_repair);