/* SPDX-License-Identifier: GPL-2.0 */
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
#ifndef __XR_DVFS_VOLT_H__
#define __XR_DVFS_VOLT_H__

#include "xr-clk-common.h"

#define DVFS_WAIT_LOOP_MAX        400

struct dvfs_vote_ops;

struct dvfs_volt_vote {
	const char *name;
	unsigned int dev_id;
	unsigned int channel_id;
	unsigned int mediabus_id;
	unsigned int volt_level;
	struct list_head node;
	struct list_head chan_node;
	struct list_head mediabus_node;
	const struct dvfs_vote_ops *ops;
};

struct dvfs_vote_ops {
	unsigned int (*get_volt)(struct dvfs_volt_vote *vote_inst);
	int (*set_volt)(struct dvfs_volt_vote *vote_inst, unsigned int volt_level);
	int (*wait_completed)(struct dvfs_volt_vote *vote_inst);
	int (*is_low_temperature)(void);
	unsigned int (*get_volt_by_level)(unsigned int dev_id, unsigned int level);
};

struct dvfs_volt_vote *dvfs_vote_get(unsigned int dev_id, const char *name);
unsigned int dvfs_get_volt_by_level(struct dvfs_volt_vote *vote_inst,
		unsigned int volt_level);
int dvfs_set_volt(struct dvfs_volt_vote *vote_inst, unsigned int volt_level);
int dvfs_wait_completed(struct dvfs_volt_vote *vote_inst);

int dvfs_is_low_temperature(struct dvfs_volt_vote *vote_inst);
int dvfs_vote_register(struct dvfs_volt_vote *vote_inst);
#endif /* __XR_DVFS_VOLT_H__ */
