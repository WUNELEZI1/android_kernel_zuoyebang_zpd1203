/* SPDX-License-Identifier: GPL-2.0-only
 *
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

#ifndef __PINCTRL_UTILS_H__
#define __PINCTRL_UTILS_H__

int pinctrl_utils_reserve_map(struct pinctrl_dev *pctldev,
		struct pinctrl_map **map, unsigned int *reserved_maps,
		unsigned int *num_maps, unsigned int reserve);
int pinctrl_utils_add_map_mux(struct pinctrl_dev *pctldev,
		struct pinctrl_map **map, unsigned int *reserved_maps,
		unsigned int *num_maps, const char *group,
		const char *function);
int pinctrl_utils_add_map_configs(struct pinctrl_dev *pctldev,
		struct pinctrl_map **map, unsigned int *reserved_maps,
		unsigned int *num_maps, const char *group,
		unsigned long *configs, unsigned int num_configs,
		enum pinctrl_map_type type);
int pinctrl_utils_add_config(struct pinctrl_dev *pctldev,
		unsigned long **configs, unsigned int *num_configs,
		unsigned long config);
void pinctrl_utils_free_map(struct pinctrl_dev *pctldev,
		struct pinctrl_map *map, unsigned int num_maps);

#endif /* __PINCTRL_UTILS_H__ */
