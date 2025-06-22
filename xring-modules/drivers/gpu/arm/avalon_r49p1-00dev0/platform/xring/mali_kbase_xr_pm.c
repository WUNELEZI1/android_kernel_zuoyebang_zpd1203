// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <device/mali_kbase_device.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <platform/xring/mali_kbase_config_platform.h>
#include <platform/xring/mali_kbase_xr_hw_access.h>
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
#include <mdr_pub.h>
#endif
#include <platform/xring/mali_kbase_xr_dvfs.h>
/*
 * AUTO_SUSPEND_DELAY - Autosuspend delay
 * The delay time (in milliseconds) to be used for autosuspend
 */
#define AUTO_SUSPEND_DELAY (10)

#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
static int kbase_xr_platform_on(struct kbase_device *kbdev)
{
	u32 value = 0;
	dev_info(kbdev->dev, "GPU power on start\n");

	if (kbdev->gpu_dev_data.pericrgreg == NULL) {
		dev_err(kbdev->dev, "pericrgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.crgreg == NULL) {
		dev_err(kbdev->dev, "crgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lpctrlreg == NULL) {
		dev_err(kbdev->dev, "lpctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.apbpctrlreg == NULL) {
		dev_err(kbdev->dev, "apbpctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.xctrlcpucfgreg == NULL) {
		dev_err(kbdev->dev, "xctrlcpucfgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lmscrgreg == NULL) {
		dev_err(kbdev->dev, "lmscrgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.ctrlreg == NULL) {
		dev_err(kbdev->dev, "ctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lpisreg == NULL) {
		dev_err(kbdev->dev, "lpisreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.gpusubreg == NULL) {
		dev_err(kbdev->dev, "gpusubreg is NULL\n");
		return -1;
	}

	// GPU Subchip On
	dev_info(kbdev->dev, "GPU Subchip on start\n");
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x08D0, 0x40000);

	if (kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.pericrgreg,
		0x0868, value, (value & 0x20000), 500000) == -1) {
		dev_err(kbdev->dev, "CRG clk on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0860, 0x80000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0050, 0x1000);

	// Xctrl CPU is power on or not
	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.apbpctrlreg,
		0x1484, value, !(value & 0x24), 500000) == -1) {
		dev_err(kbdev->dev, "XCtrl CPU is not power on\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x08D0, 0x100);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.xctrlcpucfgreg, 0x0E40, 0x4);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.pericrgreg,
		0x0808, value, (value & 0x200), 500000) == -1) {
		dev_err(kbdev->dev, "CFG Bus clk on failed\n");
		return -1;
	}

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lmscrgreg,
		0x0808, value, (value & 0x1000000), 500000) == -1) {
		dev_err(kbdev->dev, "Sys Peri clk on failed\n");
		return -1;
	}

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lmscrgreg,
		0x0818, value, (value & 0x400000), 500000) == -1) {
		dev_err(kbdev->dev, "Pll Logic clk on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.xctrlcpucfgreg, 0x0E10, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0870, 0x1000);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0850) | 0x20000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0850, value);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0820, 0x08200000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0860, 0x4000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0010, 0xBC);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0014, 0xBC);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0864, 0x4000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x0024, 0x6F0);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0860, 0x4000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x010, 0xBC);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x190, 0xC0000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x190, 0x20000);

	udelay(1000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpctrlreg,
		0x194, value, !(value & 0x2), 500000) == -1) {
		dev_err(kbdev->dev, "Memory shut down failed\n");
		return -1;
	}

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x008C) | 0x10000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x008C, value);

	// GPU Subsys On
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_SOFTWARE)
	dev_info(kbdev->dev, "GPU Software Subsys on start\n");
	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) | 0x80000000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x050, 0x2000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.pericrgreg,
		0x808, value, (value & 0x1), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus clk on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x870, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x874, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x870, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x464, 0x80000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x46C, value, !(value & 0x8), 500000) == -1) {
		dev_err(kbdev->dev, "Cfg Bus on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x4B0, 0x100000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x4B4, value, !(value & 0x10), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus on failed\n");
		return -1;
	}

	// Bypass GPC
	dev_info(kbdev->dev, "Bypass GPC\n");
	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x0804) | 0x2;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x0804, value);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0x2);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.crgreg,
		0x18, value, !(value & 0x2), 500000) == -1) {
		dev_err(kbdev->dev, "GPC bypass failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x30, 0x10000);
	// Bypass GPC Finish

	udelay(3000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00010001);

	udelay(1000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00300000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x80000);

	udelay(1000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x10, 0x1);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0x1);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x20000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00040004);

	udelay(1000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x714, value, (value & 0x1), 500000) == -1) {
		dev_err(kbdev->dev, "Memory repair failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x7);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x10, 0x1);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x8C) | 0x10000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x8C, value);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40) | 0x3C0000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40, value);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.ctrlreg,
		0x48, value, ((value &0x249) == 0x249), 500000) == -1) {
		dev_err(kbdev->dev, "adb request channel failed\n");
		return -1;
	}

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) & 0x7FFFFFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

#else
	dev_info(kbdev->dev, "GPU Hardware Subsys on start\n");
	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) | 0x80000000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x050, 0x2000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.pericrgreg,
		0x808, value, (value & 0x1), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus clk on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x870, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x874, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x870, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x464, 0x80000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x46C, value, !(value & 0x8), 500000) == -1) {
		dev_err(kbdev->dev, "CFG Bus on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x4B0, 0x100000);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x4B4, value, !(value & 0x10), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus on failed\n");
		return -1;
	}

	// Enable GPC Power on
	dev_info(kbdev->dev, "Enable GPC\n");
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x10, 0x1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x7);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x04, 0x00FF0001);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x804, 0xFFFFFFFC);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x808, 0x00100FFF);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x814) & 0xFFFFFFFC;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x814, value);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x030, 0x007F0007);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x100);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x10, 0x2);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.crgreg,
		0x18, value, (value & 0x2), 500000) == -1) {
		dev_err(kbdev->dev, "GPC clk on failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x800, 0x00010001);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x0, 0x00010001);

	udelay(500);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpctrlreg,
		0x0C, value, (value & 0x4), 500000) == -1) {
		dev_err(kbdev->dev, "GPC on failed\n");
		return -1;
	}
	dev_info(kbdev->dev, "Enable GPC successfully\n");
	// Enable GPC Power on Finish

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x8C) | 0x10000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x8C, value);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40) | 0x3C0000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40, value);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.ctrlreg,
		0x48, value, ((value & 0x249) == 0x249), 500000) == -1) {
		dev_err(kbdev->dev, "adb request channel failed\n");
		return -1;
	}

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) & 0x7FFFFFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);
#endif

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.gpusubreg, 0x0);
	dev_info(kbdev->dev, "GPU id = 0x%x\n", value);

	return 0;
}

static int kbase_xr_platform_off(struct kbase_device *kbdev)
{
	u32 value = 0;
	dev_info(kbdev->dev, "GPU power off start\n");

	if (kbdev->gpu_dev_data.pericrgreg == NULL) {
		dev_err(kbdev->dev, "pericrgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.crgreg == NULL) {
		dev_err(kbdev->dev, "crgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lpctrlreg == NULL) {
		dev_err(kbdev->dev, "lpctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.apbpctrlreg == NULL) {
		dev_err(kbdev->dev, "apbpctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.xctrlcpucfgreg == NULL) {
		dev_err(kbdev->dev, "xctrlcpucfgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lmscrgreg == NULL) {
		dev_err(kbdev->dev, "lmscrgreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.ctrlreg == NULL) {
		dev_err(kbdev->dev, "ctrlreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.lpisreg == NULL) {
		dev_err(kbdev->dev, "lpisreg is NULL\n");
		return -1;
	}
	if (kbdev->gpu_dev_data.gpusubreg == NULL) {
		dev_err(kbdev->dev, "gpusubreg is NULL\n");
		return -1;
	}

	// GPU Subsys Off
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_SOFTWARE)
	dev_info(kbdev->dev, "GPU Software Subsys off start\n");
	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) | 0x80000000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40) & 0xFFC3FFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40, value);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.ctrlreg,
		0x48, value, ((value & 0xFFF) == 0x0), 500000) == -1) {
		dev_err(kbdev->dev, "adb request channel failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x4B0, 0x00100010);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x4B4, value, ((value & 0x410) == 0x410), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus off failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x464, 0x80008);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x46C, value, ((value & 0x408) == 0x408), 500000) == -1) {
		dev_err(kbdev->dev, "CFG Bus off failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x874, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x054, 0x2000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0x2000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x28, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x28, 0x7);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00020002);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00080008);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00300030);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x100, 0x00010000);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) & 0x7FFFFFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

#else
	dev_info(kbdev->dev, "GPU Hardware Subsys off start\n");
	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) | 0x80000000;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40) & 0xFFC3FFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x40, value);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.ctrlreg,
		0x48, value, ((value & 0xFFF) == 0x0), 500000) == -1) {
		dev_err(kbdev->dev, "adb request channel failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x4B0, 0x100010);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x4B4, value, ((value & 0x410) == 0x410), 500000) == -1) {
		dev_err(kbdev->dev, "Main Bus off failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpisreg, 0x464, 0x80008);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpisreg,
		0x46C, value, ((value & 0x408) == 0x408), 500000) == -1) {
		dev_err(kbdev->dev, "CFG Bus off failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x874, 0x8000000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x28, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x054, 0x2000);

	// Disable GPC Power off
	dev_info(kbdev->dev, "Disable GPC\n");
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x800, 0x10000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0x2);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.crgreg,
		0x18, value, !(value & 0x2), 500000) == -1) {
		dev_err(kbdev->dev, "GPC Clk off failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x30, 0x10000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x24, 0x100);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0x1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x28, 0x7);
	// Disable GPC Power off Finish

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44) & 0x7FFFFFFF;
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0xD44, value);

#endif
	dev_info(kbdev->dev, "GPU Subchip power off start\n");

	// GPU Subchip Off
	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x190, 0x20002);

	if(kbase_xr_reg_read_poll32(kbdev, kbdev->gpu_dev_data.lpctrlreg,
		0x194, value, (value & 0x2), 500000) == -1) {
		dev_err(kbdev->dev, "Memory shut down failed\n");
		return -1;
	}

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.lpctrlreg, 0x190, 0xC000C);

	value = kbase_xr_reg_read32(kbdev, kbdev->gpu_dev_data.ctrlreg, 0x008C);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x14, 0xBC);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0864, 0x4000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.crgreg, 0x28, 0x6F0);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.xctrlcpucfgreg, 0x0E14, 0x8);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x0874, 0x1000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.xctrlcpucfgreg, 0xE44, 0x4);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x8D4, 0x100);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x054, 0x1000);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x864, 0x80000);

	udelay(1);

	kbase_xr_reg_write32(kbdev, kbdev->gpu_dev_data.pericrgreg, 0x8D4, 0x40000);
	dev_info(kbdev->dev, "GPU Subchip power off successfully\n");

	return 0;
}
#endif

static inline void kbase_platform_on(struct kbase_device *kbdev)
{
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
	if(kbase_xr_platform_on(kbdev) == -1)
		dev_err(kbdev->dev, "kbase_xr_platform_on failed");
#else
	/* gpu power on by regulator gpu_subsys */
	int regulator_refcount = 0;
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	int gpufreq_refcount = 0;
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;
#endif
	regulator_refcount = atomic_read(&kbdev->gpu_dev_data.regulator_refcount);
	if (kbdev->gpu_dev_data.gpu_subsys && regulator_refcount == 0) {
		int loop = 0;

		/* enable regulator */
		while (loop < MAX_LOOP &&
			   unlikely(regulator_enable(kbdev->gpu_dev_data.gpu_subsys))) {
			dev_err(kbdev->dev, "Failed to enable regulator, retry...%d", ++loop);
			udelay(DELAY_STEP * loop);
		}
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
		if (atomic_read(&kbdev->gpu_dev_data.gpufreq_ready) == 1) {
			gpufreq_refcount = atomic_read(&kbdev->gpu_dev_data.gpufreq_refcount);
			if (gpufreq_refcount == 0 && gpufreq_ops->gpufreq_resume) {
				gpufreq_ops->gpufreq_resume();
				atomic_inc(&kbdev->gpu_dev_data.gpufreq_refcount);
			}
		}
#endif
		if (loop == MAX_LOOP)
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
			mdr_system_error(MODID_GPU_REGULATOR_ON_FAIL, 0, 0);
#endif
		WARN_ON(loop == MAX_LOOP);
		atomic_inc(&kbdev->gpu_dev_data.regulator_refcount);
	} else {
		dev_err(kbdev->dev, "%s called zero,regulator_refcount:[%d]\n", __func__, regulator_refcount);
	}
#endif
}

static inline void kbase_platform_off(struct kbase_device *kbdev)
{
#if IS_ENABLED(CONFIG_MALI_XR_GPU_POWER_BACKUP)
	if(kbase_xr_platform_off(kbdev) == -1)
		dev_err(kbdev->dev, "kbase_xr_platform_off failed");
#else
	/* gpu power off by regulator gpu_subsys */
	int regulator_refcount = 0;
	int gpufreq_refcount = 0;
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;
#endif
	regulator_refcount = atomic_read(&kbdev->gpu_dev_data.regulator_refcount);
	if (kbdev->gpu_dev_data.gpu_subsys && regulator_refcount != 0) {
		int loop = 0;
		/* disable regulator*/
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
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
#endif
		while (loop < MAX_LOOP &&
			   unlikely(regulator_disable(kbdev->gpu_dev_data.gpu_subsys))) {
			dev_err(kbdev->dev, "Failed to disable regulator, retry...%d", ++loop);
			udelay(DELAY_STEP * loop);
		}
		if (loop == MAX_LOOP)
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
			mdr_system_error(MODID_GPU_REGULATOR_OFF_FAIL, 0, 0);
#endif
		WARN_ON(loop == MAX_LOOP);
		regulator_refcount = atomic_dec_return(&kbdev->gpu_dev_data.regulator_refcount);
		if (unlikely(regulator_refcount != 0))
			dev_err(kbdev->dev,
				"%s called not match, regulator_refcount:[%d]\n",
				__func__, regulator_refcount);
	}
#endif
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

	kbase_platform_on(kbdev);
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
	kbase_platform_on(kbdev);
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
	kbase_platform_off(kbdev);
#else  /* MALI_USE_CSF */
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

#ifdef KBASE_PM_RUNTIME
	pm_runtime_mark_last_busy(kbdev->dev);
	pm_runtime_put_autosuspend(kbdev->dev);
#else
	/* Power down the GPU immediately as runtime PM is disabled */
	kbase_platform_off(kbdev);
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
	kbase_platform_on(kbdev);
#endif
	return 0;
}

static void pm_callback_runtime_off(struct kbase_device *kbdev)
{
#if !MALI_USE_CSF
	kbase_platform_off(kbdev);
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
		dev_err(kbdev->dev, "Mali gpu shader ready power off timeout\n");
		return;
	}

	err = kbase_reg_poll64_timeout(kbdev, GPU_CONTROL_ENUM(SHADER_PWRTRANS), shader_trans,
			    (shader_trans & shader_present) == 0, delay_us, timeout_us, false);
	if (err) {
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