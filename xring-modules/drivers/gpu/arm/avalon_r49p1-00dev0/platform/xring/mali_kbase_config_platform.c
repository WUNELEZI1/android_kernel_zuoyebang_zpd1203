/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#include <mali_kbase.h>
#include <device/mali_kbase_device.h>
#include <mali_kbase_config.h>
#include <mali_kbase_defs.h>
#include "mali_kbase_config_platform.h"
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
#include <platform/xring/mali_kbase_xr_mdr.h>
#endif
#include <platform/xring/mali_kbase_xr_dvfs.h>
#include <platform/xring/mali_kbase_xr_virtual_freq.h>

static int kbase_gpu_init_device_tree(struct kbase_device * const kbdev)
{
	struct device_node *np = NULL;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

#ifdef CONFIG_OF
	/* read outstanding value from dts */
	np = of_find_compatible_node(NULL, NULL, "arm,mali-midgard");
	if (!np) {
		dev_err(kbdev->dev,
			"not find device node arm,mali_midgard!\n");
		return -EINVAL;
	}
#endif
	return 0;
}

/* Init GPU platform related register map */
static int kbase_gpu_register_map(struct kbase_device *kbdev)
{
	int err = 0;

	kbdev->gpu_dev_data.crgreg = ioremap(SYS_REG_GPU_CRG_BASE_ADDR,
		SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.crgreg) {
		dev_err(kbdev->dev, "Can't remap sys crg register window on platform\n");
		err = -EINVAL;
		goto out_crg_ioremap;
	}
	kbdev->gpu_dev_data.ctrlreg = ioremap(SYS_REG_GPU_CTRL_BASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.ctrlreg) {
		dev_err(kbdev->dev, "Can't remap sys ctrl register window on platform\n");
		err = -EINVAL;
		goto out_ctrl_ioremap;
	}
	kbdev->gpu_dev_data.dvfsreg = ioremap(SYS_REG_GPU_DVFS_BASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.dvfsreg) {
		dev_err(kbdev->dev, "Can't remap sys dvfs register window on platform\n");
		err = -EINVAL;
		goto out_dvfs_ioremap;
	}
	kbdev->gpu_dev_data.lpctrlreg = ioremap(SYS_REG_GPU_LPCTRL_BASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.lpctrlreg) {
		dev_err(kbdev->dev, "Can't remap sys lpctrl register window on platform\n");
		err = -EINVAL;
		goto out_lpctrl_ioremap;
	}
	kbdev->gpu_dev_data.gpusubreg = ioremap(SYS_REG_GPU_SUB_BASE_ADDR,
			SYS_REG_GPU_SUB_SIZE);
	if (!kbdev->gpu_dev_data.gpusubreg) {
		dev_err(kbdev->dev, "Can't remap gpu subsys gpusubreg register window on platform\n");
		err = -EINVAL;
		goto out_gpusub_ioremap;
	}
	kbdev->gpu_dev_data.pericrgreg = ioremap(APB_INNER_REG_PERI_CRG_BASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.pericrgreg) {
		dev_err(kbdev->dev, "Can't remap pericrgreg register window on platform\n");
		err = -EINVAL;
		goto out_pericrg_ioremap;
	}
	kbdev->gpu_dev_data.lmscrgreg = ioremap(LMS_APB_DCDR_REG_LMS_CRG_BASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.lmscrgreg) {
		dev_err(kbdev->dev, "Can't remap lmscrgreg register window on platform\n");
		err = -EINVAL;
		goto out_lmscrg_ioremap;
	}
	kbdev->gpu_dev_data.xctrlcpucfgreg = ioremap(XCTRL_REG_XCTRLCPUBASE_ADDR,
			SYS_REG_POWER_SIZE);
	if (!kbdev->gpu_dev_data.xctrlcpucfgreg) {
		dev_err(kbdev->dev, "Can't remap xctrlcpucfgreg register window on platform\n");
		err = -EINVAL;
		goto out_xctrlcpucfg_ioremap;
	}
	/* remap apb pctrl reg */
	kbdev->gpu_dev_data.apbpctrlreg = ioremap(APB_INNER_REG_PCTRL_BASE_ADDR,
			APB_INNER_REG_PCTRL_SIZE);
	if (!kbdev->gpu_dev_data.apbpctrlreg) {
		dev_err(kbdev->dev, "Can't remap apbpctrlreg register window on platform\n");
		err = -EINVAL;
		goto out_apbpctrl_ioremap;
	}
	kbdev->gpu_dev_data.lpisreg = ioremap(LPIS_ACTRL_REG_BASE_ADDR,
			LPIS_ACTRL_REG_SIZE);
	if (!kbdev->gpu_dev_data.lpisreg) {
		dev_err(kbdev->dev, "Can't remap lpisreg register window on platform\n");
		err = -EINVAL;
		goto out_lpis_ioremap;
	}
	return err;
/* Modidy gpuregs ioremap logic by 20230711 */
out_lpis_ioremap:
	iounmap(kbdev->gpu_dev_data.apbpctrlreg);
out_apbpctrl_ioremap:
	iounmap(kbdev->gpu_dev_data.xctrlcpucfgreg);
out_xctrlcpucfg_ioremap:
	iounmap(kbdev->gpu_dev_data.lmscrgreg);
out_lmscrg_ioremap:
	iounmap(kbdev->gpu_dev_data.pericrgreg);
out_pericrg_ioremap:
	iounmap(kbdev->gpu_dev_data.gpusubreg);
out_gpusub_ioremap:
	iounmap(kbdev->gpu_dev_data.lpctrlreg);
out_lpctrl_ioremap:
	iounmap(kbdev->gpu_dev_data.dvfsreg);
out_dvfs_ioremap:
	iounmap(kbdev->gpu_dev_data.ctrlreg);
out_ctrl_ioremap:
	iounmap(kbdev->gpu_dev_data.crgreg);
out_crg_ioremap:
	return err;
}

static void kbase_gpu_register_unmap(struct kbase_device *kbdev)
{
	if (kbdev->gpu_dev_data.crgreg != NULL)
		iounmap(kbdev->gpu_dev_data.crgreg);
	if (kbdev->gpu_dev_data.ctrlreg != NULL)
		iounmap(kbdev->gpu_dev_data.ctrlreg);
	if (kbdev->gpu_dev_data.dvfsreg != NULL)
		iounmap(kbdev->gpu_dev_data.dvfsreg);
	if (kbdev->gpu_dev_data.lpctrlreg != NULL)
		iounmap(kbdev->gpu_dev_data.lpctrlreg);
}

static int kbase_xr_platform_early_init(struct kbase_device *kbdev)
{
	struct device *dev = NULL;
	int err = 0;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	if (!kbdev)
		return -ENODEV;

	dev = kbdev->dev;
	dev->platform_data = kbdev;

	/* init gpu_outstanding value, chip_type from device tree */
	err = kbase_gpu_init_device_tree(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Init special device tree setting failed\n");
		return err;
	}

	/* register map for GPU, especially for chip_type related regitser */
	err = kbase_gpu_register_map(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Can't remap gpu register\n");
		return err;
	}

#if defined(CONFIG_REGULATOR)
	/* gpu kernel get gpu_subsys regulator */
	kbdev->gpu_dev_data.gpu_subsys = devm_regulator_get(dev, "gpu_subsys");
	if (IS_ERR(kbdev->gpu_dev_data.gpu_subsys)) {
		dev_err(kbdev->dev, "Failed to get regulator\n");
		return -EINVAL;
	}
#endif

	atomic_set(&kbdev->gpu_dev_data.regulator_refcount, 0);
	kbdev->gpu_dev_data.query_pid = -1;
	kbdev->gpu_dev_data.fence_count = 0;
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	kbdev->gpu_dev_data.mdr_type = 0;
#endif

#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	kbdev->gpu_dev_data.dvfs_type = 0;
	atomic_set(&kbdev->gpu_dev_data.gpufreq_ready, 0);
	atomic_set(&kbdev->gpu_dev_data.gpufreq_refcount, 0);
	kbdev->gpu_dev_data.opp_lowest_freq_khz = 0;
#endif

#if IS_ENABLED(CONFIG_MALI_XRING_CL_DVFS)
	kbdev->gpu_dev_data.cl_accel_flag = false;
#endif

#if IS_ENABLED(CONFIG_MALI_XR_HOTPLUG)
	kbdev->gpu_dev_data.enabled_hotplug = false;
#endif

	return err;
}

static void kbase_xr_platform_early_term(struct kbase_device *kbdev)
{
	KBASE_DEBUG_ASSERT(kbdev != NULL);
	kbase_gpu_register_unmap(kbdev);
}

static int kbase_xr_platform_late_init(struct kbase_device *kbdev)
{
	int err = 0;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	if (!kbdev)
		return -ENODEV;
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	err = kbase_xr_mdr_exception_register(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Mdr gpu exception register fail.");
		return err;
	}
#endif
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	err = kbase_xr_register_dvfs_op(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Failed to register DVFS ops!");
		return err;
	}
#endif
#if (IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ) || IS_ENABLED(CONFIG_MALI_XR_HOTPLUG))
	err = kbase_xr_init_all_coremask_table(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Failed to init all coremask table!");
		return err;
	}
#endif
#if (IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ) && IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS))
	err = kbase_xr_vfreq_init(kbdev);
	if (err) {
		dev_err(kbdev->dev, "Failed to initialize virtual devfreq!");
		return err;
	}
#endif
	return err;
}

static void kbase_xr_platform_late_term(struct kbase_device *kbdev)
{
	int err = 0;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	err = kbase_xr_mdr_exception_unregister(kbdev);
	if (err)
		dev_err(kbdev->dev, "MDR unregister failed.");
#else
	CSTD_UNUSED(err);
#endif
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	kbase_xr_unregister_dvfs_op(kbdev);
#endif
#if (IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ) || IS_ENABLED(CONFIG_MALI_XR_HOTPLUG))
	kbase_xr_term_all_coremask_table(kbdev);
#endif
}

struct kbase_platform_funcs_conf platform_funcs = {
	.platform_init_func = &kbase_xr_platform_early_init,
	.platform_term_func = &kbase_xr_platform_early_term,
	.platform_late_init_func = &kbase_xr_platform_late_init,
	.platform_late_term_func = &kbase_xr_platform_late_term
};