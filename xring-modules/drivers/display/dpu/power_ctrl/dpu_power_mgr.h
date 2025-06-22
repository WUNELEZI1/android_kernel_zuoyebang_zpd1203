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

#ifndef _DPU_POWER_MGR_H_
#define _DPU_POWER_MGR_H_

#include <linux/regulator/consumer.h>
#include <drm/drm_device.h>

#include "dpu_hw_common.h"
#include "dpu_kms.h"
#include "dpu_cap.h"
#include "dpu_hw_power_ops.h"
#include "dpu_clk_mgr.h"

struct dpu_power_client;

/* define dpu power status */
enum dpu_power_status {
	DPU_POWER_OFF,
	DPU_POWER_ON,
};

/**
 * dpu_power_ctrl_info - used do dpu power get
 * @part_mask: the dpu partition mask to do power get
 * @scene_ctrl_id: the sense ctrl id used for cmdlist to do dpu hw init configure
 */
struct dpu_power_ctrl_info {
	u32 part_mask;
	int scene_ctrl_id;
};

/**
 * dpu_power_client_info - used for create a new client
 * sense_ctrl_id: the client's sense ctrl id
 */
struct dpu_power_client_info {
	int sense_ctrl_id;
};

/**
 * dpu_power_client_ops - the dpu power client's ops
 * @enable: used to do power enable
 * @disable: used to do power disable
 * @is_powered_on: used to check the client is powered on or not
 */
struct dpu_power_client_ops {
	void (*enable)(struct dpu_power_client *client, u32 part_mask);
	void (*disable)(struct dpu_power_client *client, u32 part_mask);
	bool (*is_powered_on)(struct dpu_power_client *client);
};

/**
 * dpu_power_client - the dpu power client structure
 * @list: used for power mgr manage all clients
 * @power_mask: the client current powered on partition mask
 * @sense_ctrl_id: the client's sense ctrl id
 * @power_status: the client's current power status
 * @lock: the client's lock
 * @ops: the client's ops to do power opeartion
 */
struct dpu_power_client {
	struct list_head list;

	u32 power_mask;
	int sense_ctrl_id;

	enum dpu_power_status power_status;

	struct mutex lock;
	struct dpu_power_client_ops *ops;
};

/**
 * dpu_power_client_alloc - alloc a new client struct
 * @info: the dpu client's info to do init
 *
 * Returns: the new client instance pointer
 */
struct dpu_power_client *dpu_power_client_alloc(
		struct dpu_power_client_info *info);

/**
 * dpu_power_client_free - free a dpu client instance
 * @client: the client need to be freed
 */
void dpu_power_client_free(struct dpu_power_client *client);

/**
 * dpu_power_mgr - dpu power manager structure
 * @drm_dev: pointer of drm device
 * @dpu_kms: pointer for dpu kms ctx
 * @dpu_partitions: all the dpu's partitions
 * @media1: media1's regulator
 * @power_count: count for each dpu partition
 * @depend_mask: the dependencies between DPU partitions
 * @clk_mgr: pointer of dpu clk mgr
 * @suspend_block: whether dpu driver is in suspend state
 * @clients: the list of dpu clients
 * @client_lock: the spinlock to protect clients list
 * @lock: protect power_count
 * @power_task: dpu power task pointer
 * @power_worker: dpu power worker
 * @power_off_cb: callback for each partition hw power off
 * @cb_data: the data used for power_off_cb
 */
struct dpu_power_mgr {
	struct drm_device *drm_dev;
	struct dpu_kms *dpu_kms;

	struct regulator *dpu_partitions[DPU_PARTITION_MAX_NUM];
	struct regulator *media1;

	u32 power_count[DPU_PARTITION_MAX_NUM];
	u32 depend_mask[DPU_PARTITION_MAX_NUM];

	struct dpu_clk_mgr *clk_mgr;

	bool suspend_block;

	struct list_head clients;
	spinlock_t client_lock;

	struct mutex lock;

	struct task_struct *power_task;
	struct kthread_worker power_worker;

	void (*power_off_cb[DPU_PARTITION_MAX_NUM])(void *data);
	void *cb_data[DPU_PARTITION_MAX_NUM];
};

/**
 * dpu_power_get - power get for dpu partitions
 * @ctrl_info: pointer of power ctrl struct
 */
void dpu_power_get(struct dpu_power_ctrl_info *ctrl_info);

/**
 * dpu_power_put - power put for dpu partitions
 * @part_mask: partition mask to power off
 */
void dpu_power_put(u32 part_mask);

/**
 * dpu_power_get_helper - helper function for power get
 * @part_mask: partition mask to power on
 */
void dpu_power_get_helper(u32 part_mask);

/**
 * dpu_power_put_helper - helper function for power put
 * @part_mask: partition mask to power off
 */
void dpu_power_put_helper(u32 part_mask);

/**
 * is_dpu_powered_on - whether the partition is powered on
 * Return: true on powered on, false on powered off
 */
bool is_dpu_powered_on(void);

/**
 * dpu_power_is_in_suspending - whether dpu is in suspending
 *
 * Return: true for dpu is in suspending , false for not
 */
bool dpu_power_is_in_suspending(void);

/**
 * dpu_power_register_power_off_cb - register callback for power off
 * @part_id: the partition id
 * @data: the pointer for callback
 * @cb: the callback pointer
 *
 * Return: 0 for register success, others for failure
 */
int dpu_power_register_power_off_cb(u32 part_id, void *data,
	void (*cb)(void *data));

/**
 * dpu_power_unregister_power_off_cb - unregister callback
 * @part_id: the partition id
 */
void dpu_power_unregister_power_off_cb(u32 part_id);

/**
 * dpu_power_resume - dpu power resume
 */
int dpu_power_resume(void);

/**
 * dpu_power_suspend - dpu power suspend
 */
int dpu_power_suspend(void);

/**
 * dpu_power_mgr_init - init dpu power mgr
 * @drm_dev: pointer of drm_device
 * @mgr: pointer of power mgr
 *
 * Return: zero on success, -ERRNO on failure
 */
int dpu_power_mgr_init(struct dpu_power_mgr **mgr,
		struct drm_device *drm_dev);

/**
 * dpu_power_mgr_deinit - deinit dpu power mgr
 * @power_mgr: pointer of power mgr
 */
void dpu_power_mgr_deinit(struct dpu_power_mgr *power_mgr);
#endif /* _DPU_POWER_H_ */
