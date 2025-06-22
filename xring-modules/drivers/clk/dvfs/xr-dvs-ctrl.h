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
#ifndef __XR_DVS_CTRL_H__
#define __XR_DVS_CTRL_H__

#include <dt-bindings/xring/platform-specific/common/pmu/include/dvs_resource.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include "clk/xr-dvfs-vote.h"

int dvs_write_wait_ack(struct dvs_hw *hw, uint8_t data);
int dvs_write_no_wait_ack(struct dvs_hw *hw, uint8_t data);
int dvs_get_cur_volt(struct dvs_hw *hw, uint8_t *data);
int dvs_wait_ack(struct dvs_hw *hw);
irqreturn_t dvs_timeout_interrupt_handle(int irq, void *dev);

#endif /* __XR_DVS_CTRL_H__ */
