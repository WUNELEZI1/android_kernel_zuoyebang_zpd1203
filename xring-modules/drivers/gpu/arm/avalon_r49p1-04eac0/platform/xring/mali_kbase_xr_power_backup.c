/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <platform/xring/mali_kbase_xr_hw_access.h>
#include <platform/xring/mali_kbase_xr_power_backup.h>

int kbase_xr_platform_on(struct kbase_device *kbdev)
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

int kbase_xr_platform_off(struct kbase_device *kbdev)
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