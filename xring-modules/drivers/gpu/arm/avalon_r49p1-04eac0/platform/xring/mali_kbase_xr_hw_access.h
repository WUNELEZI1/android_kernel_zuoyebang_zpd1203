// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_XR_HW_ACCESS_H_
#define _MALI_KBASE_XR_HW_ACCESS_H_

u32 kbase_xr_reg_read32(struct kbase_device *kbdev, void __iomem *base_addr, u32 offset);
u64 kbase_xr_reg_read64(struct kbase_device *kbdev, void __iomem *base_addr, u32 offset);
void kbase_xr_reg_write32(struct kbase_device *kbdev,
	void __iomem *base_addr, u32 offset, u32 value);
void kbase_xr_reg_write64(struct kbase_device *kbdev,
	void __iomem *base_addr, u32 offset, u64 value);
#define kbase_xr_reg_read_poll32(kbdev, base_addr, offset, val, cond, timeout_us) \
	read_poll_timeout_atomic(kbase_xr_reg_read32, val, cond, 0, timeout_us, \
				 0, kbdev, base_addr, offset)
#define kbase_xr_reg_read_poll64(kbdev, base_addr, offset, val, cond, timeout_us) \
	read_poll_timeout_atomic(kbase_xr_reg_read64, val, cond, 0, timeout_us, \
				 0, kbdev, base_addr, offset)
#endif /* _MALI_KBASE_XR_HW_ACCESS_H_ */
