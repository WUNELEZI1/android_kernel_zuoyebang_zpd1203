// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * regulator manager
 *
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#ifndef __XR_REGULATOR_MNG_H__
#define __XR_REGULATOR_MNG_H__

#include <linux/mutex.h>

struct regulator_dev;
struct xr_regulator_manager;

/**
 * struct xr_regulator_manager - manage the sub regulators under supply_rdev.
 *
 * @on_sub_regulator_enabling: sub regulator call this ops to notify it's supply
 *                             before really enable the sub regulator.
 */
struct xr_regulator_manager {
	int (*on_sub_regulator_enabling)(struct xr_regulator_manager *mng,
			struct regulator_dev *sub_rdev);

	u64 last_enable_time; // us
	struct mutex   lock;
};

#endif /* __XR_REGULATOR_MNG_H__ */
