// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#include <linux/io.h>
#include <linux/types.h>

#include <mali_kbase.h>

/*
 * When using this function to read or write the
 * target register, please first check whether
 * the address space of the register to be read
 * or written is out of bounds.
 */
u32 kbase_xr_reg_read32(struct kbase_device *kbdev, void __iomem *base_addr, u32 offset)
{
	u32 val = 0;
	void __iomem *gpu_reg;

	if (!kbdev)
		return 0;

	if (!base_addr) {
		dev_err(kbdev->dev, "read register failed!");
		return 0;
	}
	gpu_reg = base_addr + offset;
	val = readl(gpu_reg);
#if IS_ENABLED(CONFIG_DEBUG_FS)
	if (unlikely(kbdev->io_history.enabled))
		kbase_io_history_add(&kbdev->io_history, gpu_reg, val, 0);
#endif /* CONFIG_DEBUG_FS */
	dev_dbg(kbdev->dev, "r32: reg %pK val %08x",
		gpu_reg, val);
	return val;
}
KBASE_EXPORT_TEST_API(kbase_xr_reg_read32);

/*
 * When using this function to read or write the
 * target register, please first check whether
 * the address space of the register to be read
 * or written is out of bounds.
 */
u64 kbase_xr_reg_read64(struct kbase_device *kbdev, void __iomem *base_addr, u32 offset)
{
	u64 val = 0;
	void __iomem *gpu_reg;

	if (!kbdev)
		return 0;

	if (!base_addr) {
		dev_err(kbdev->dev, "read register failed!");
		return 0;
	}
	gpu_reg = base_addr + offset;
	val = (u64)readl(gpu_reg) |
		((u64)readl(gpu_reg + 4) << 32);
#if IS_ENABLED(CONFIG_DEBUG_FS)
	if (unlikely(kbdev->io_history.enabled)) {
		kbase_io_history_add(&kbdev->io_history, gpu_reg, (u32)val, 0);
		kbase_io_history_add(&kbdev->io_history, gpu_reg + 4,
				(u32)(val >> 32), 0);
	}
#endif /* CONFIG_DEBUG_FS */
	dev_dbg(kbdev->dev, "r64: reg %pK val %016llx",
		gpu_reg, val);
	return val;
}
KBASE_EXPORT_TEST_API(kbase_xr_reg_read64);

/*
 * When using this function to read or write the
 * target register, please first check whether
 * the address space of the register to be read
 * or written is out of bounds.
 */
void kbase_xr_reg_write32(struct kbase_device *kbdev, void __iomem *base_addr,
		u32 offset, u32 value)
{
	void __iomem *gpu_reg;

	if (!kbdev)
		return;

	if (!base_addr) {
		dev_err(kbdev->dev, "write register failed!");
		return;
	}
	gpu_reg = base_addr + offset;
	writel(value, gpu_reg);
#if IS_ENABLED(CONFIG_DEBUG_FS)
	if (unlikely(kbdev->io_history.enabled))
		kbase_io_history_add(&kbdev->io_history, gpu_reg, value, 1);
#endif /* CONFIG_DEBUG_FS */
	dev_dbg(kbdev->dev, "w32: reg %pK val %08x",
		gpu_reg, value);
}
KBASE_EXPORT_TEST_API(kbase_xr_reg_write32);

/*
 * When using this function to read or write the
 * target register, please first check whether
 * the address space of the register to be read
 * or written is out of bounds.
 */
void kbase_xr_reg_write64(struct kbase_device *kbdev, void __iomem *base_addr,
		u32 offset, u64 value)
{
	void __iomem *gpu_reg;

	if (!kbdev)
		return;

	if (!base_addr) {
		dev_err(kbdev->dev, "write register failed!");
		return;
	}
	gpu_reg = base_addr + offset;
	writel(value & 0xFFFFFFFF, gpu_reg);
	writel(value >> 32, gpu_reg + 4);
#if IS_ENABLED(CONFIG_DEBUG_FS)
	if (unlikely(kbdev->io_history.enabled)) {
		kbase_io_history_add(&kbdev->io_history, gpu_reg, (u32)value,
				1);
		kbase_io_history_add(&kbdev->io_history, gpu_reg + 4,
				(u32)(value >> 32), 1);
	}
#endif /* CONFIG_DEBUG_FS */
	dev_dbg(kbdev->dev, "w64: reg %pK val %016llx",
		gpu_reg, value);
}
KBASE_EXPORT_TEST_API(kbase_xr_reg_write64);