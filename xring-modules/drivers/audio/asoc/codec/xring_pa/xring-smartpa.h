/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef XRING_CODEC_H
#define XRING_CODEC_H

#define PA_INIT_MAX_COUNT    5

enum xring_smartpa_type {
	/* 0 is reserved for indicating failure to identify */
	XR_CS35L41B = 1,
	XR_CS35L43 = 2,
	XR_CS40L26 = 3,
	XR_CS35L45 = 4,
	XR_FS3001 = 5,
};

struct xring_smartpa_priv {
	struct device *dev;
	enum xring_smartpa_type smartpa_type;
	const struct snd_soc_component_driver *component;
	struct snd_soc_dai_driver *dai_driver;
	struct device_node *rst_np;
	unsigned int reset_gpio;
	struct regmap *regmap;
	struct regmap *regmap_32bit;

	int num_dai;
};

extern int __init cs40l26_codec_driver_init(void);
extern void cs40l26_codec_driver_exit(void);

#endif
