/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_RESET_INTERNAL_H__
#define __XR_RESET_INTERNAL_H__

struct xr_reset_config {
	const char *name;
	bool is_secured;
	u32 offset;
	u32 shift;
};

struct xr_reset_desc {
	const struct xr_reset_config *resets;
	size_t num_resets;
};

struct xr_reset_data {
	struct reset_controller_dev rcdev;
	void __iomem *base;
	const struct xr_reset_desc *desc;
	u32 id;
};


int xr_reset_probe(struct platform_device *pdev, const struct xr_reset_desc *desc);

#endif /* __XR_RESET_INTERNAL_H__ */
