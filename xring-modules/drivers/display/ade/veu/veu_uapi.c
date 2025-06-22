// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include "veu_uapi.h"
#include "veu_defs.h"
#include "veu_utils.h"
#include "veu_dmmu.h"
#include "veu_stripe.h"
#include "veu_enum.h"
#include "veu_format.h"
#include "include/veu_base_addr.h"
#include "include/veu_hw_scene_ctl_reg.h"
#include "cmdlist/veu_cmdlist_frame_mgr.h"
#define CREATE_TRACE_POINTS
#include "veu_trace.h"
#undef CREATE_TRACE_POINTS

/*
 * VEU need at least 4200MB for 4k resolution layer
 * 4200MB can raise ddr rate profile by 1 step to ensure
 * VEU get enough ddr bandwidth
 */
#define VEU_MIN_DDR_RATE_VOTE 4200

static void veu_reg_config(struct veu_data *veu_dev)
{
	VEU_DBG("enter");

	veu_rdma_set_reg(veu_dev);
	veu_pipe_set_reg(veu_dev);
	veu_wdma_set_reg(veu_dev);
	veu_2dscaler_set_reg(veu_dev);

	VEU_DBG("exit");
}

int veu_format_check(struct veu_layer *input_layer, struct veu_layer *output_layer)
{
	const struct veu_format_map *format_map;

	format_map = veu_format_get(MODULE_RDMA, input_layer->format, input_layer->afbc_used);
	veu_check_and_return(!format_map, -1, "rch format unsupported");

	format_map = veu_format_get(MODULE_WDMA, output_layer->format, output_layer->afbc_used);
	veu_check_and_return(!format_map, -1, "wb format unsupported");

	return 0;
}

static int veu_config(struct veu_data *veu_dev, struct veu_task *task)
{
	bool is_last_block = false;
	bool is_first_block = true;
	int ret;
	int i;

	VEU_DBG("enter");

	ret = veu_format_check(&task->input_layer, &task->output_layer);
	veu_check_and_return(ret != 0, -EINVAL, "format check fail");

	for (i = 0; i < veu_dev->veu_stripe->count; i++) {
		if (i == (veu_dev->veu_stripe->count - 1)) {
			VEU_DBG("is last_block");
			is_last_block = true;
		}
		veu_dev->cmdlist->funcs->prepare(veu_dev, i, is_first_block, is_last_block);
		is_first_block = false;
		VEU_DBG("one block config end");
	}

	ret = veu_rdma_config(veu_dev, &(task->input_layer));
	veu_check_and_return(ret != 0, ret, "rdma config fail");

	ret = veu_pipe_config(veu_dev,
			&(task->input_layer), &(task->output_layer));
	veu_check_and_return(ret != 0, ret, "pipe config fail");

	ret = veu_wdma_config(veu_dev, &(task->output_layer));
	veu_check_and_return(ret != 0, ret, "wdma config fail");

	veu_reg_config(veu_dev);

	if (veu_dev->veu_stripe->count > 1) {
		ret = veu_stripe_config(veu_dev);
		veu_check_and_return(ret != 0, ret, "veu stripe config fail");
	}

	veu_dev->cmdlist->funcs->commit(veu_dev);

	veu_ctrl_set_reg(veu_dev);

	VEU_DBG("exit");

	return 0;
}

static void veu_writeback_timeout_handle(struct veu_data *veu_dev)
{
	uint32_t clear_status;
	uint32_t clear_st_mask;
	int ret;

	cpu_bit_write(veu_dev, VEU_SCENE_CTL0 + BOTH_CFG_RDY_OFFSET, 0x1, SW_CLR_SHIFT, SW_CLR_LENGTH);

	clear_st_mask = BITS_MASK(SW_CLR_ST_SHIFT, SW_CLR_ST_LENGTH);

	ret = VEU_READ_POLL_TIMEOUT(veu_dev, VEU_SCENE_CTL0 + BOTH_CFG_RDY_OFFSET, clear_status,
		(clear_status & clear_st_mask) == 0, 1000, 16000);
	if (ret)
		VEU_ERR("wait ctl clear timeout! value 0x%x\n", clear_status);
}

static int veu_postprocess(struct veu_data *veu_dev)
{
	ktime_t k_begin;
	s64 timecost_us;
	uint32_t value;
	int ret;

	trace_veu_postprocess("veu postprocess start");

	k_begin = veu_get_timestamp();

	ret = wait_event_interruptible_timeout(veu_dev->frm_done_wq, veu_dev->frm_done_flag,
		msecs_to_jiffies(100));
	if (ret <= 0) {
		veu_writeback_timeout_handle(veu_dev);
		VEU_ERR("wait for frm done irq timeout, sw clear begin");
		value = inp32(veu_dev->base + VEU_INT);
		VEU_INFO("veu irq status 0x%x", value);
	}

	timecost_us = veu_timestamp_diff(k_begin);
	VEU_TIMECOST("veu hardware process cost %d.%dms",
		(u32)(timecost_us / 1000), (u32)(timecost_us % 1000));

	veu_dev->frm_done_flag = 0;

	trace_veu_postprocess("veu postprocess end");

	return ret <= 0 ? -1 : 0;
}

extern int veu_ddr_bandwidth_update(u32 veu_expect_bandwidth);

void veu_ddr_vote(struct veu_task *task)
{
	struct veu_layer input_layer = task->input_layer;
	int width = input_layer.src_rect.right - input_layer.src_rect.left + 1;
	int height = input_layer.src_rect.bottom - input_layer.src_rect.top + 1;
	uint64_t ddr_freq;
	uint64_t fps = 120;
	uint32_t bpp = 32;

	ddr_freq = width * height * fps * bpp / 8 / 1000000;
	VEU_DBG("veu expected ddr_freq %lluMB", ddr_freq);
	ddr_freq = ddr_freq < VEU_MIN_DDR_RATE_VOTE ? VEU_MIN_DDR_RATE_VOTE : ddr_freq;
	veu_ddr_bandwidth_update(ddr_freq);
}

int veu_process(struct file *filp, unsigned long arg)
{
	const void __user *argp = (void __user *)(uintptr_t)arg;
	struct veu_data *priv = NULL;
	struct veu_task task;
	struct veu_data_copy data_copy;
	int ret;

	VEU_DBG("enter");

	veu_check_and_return(!filp, -1, "filp is null");
	priv = (struct veu_data *)(filp->private_data);
	veu_check_and_return(!priv, -1, "priv is null");

	ret = copy_from_user(&data_copy, argp, sizeof(struct veu_data_copy));
	if (ret) {
		VEU_ERR("copy from user fail: data copy");
		goto get_data_copy_fail;
	}

	task = data_copy.veu_layer_task;

	priv->veu_stripe = vmalloc(sizeof(struct veu_stripe));
	if (!priv->veu_stripe) {
		VEU_ERR("alloc priv->veu_stripe fail");
		goto get_data_copy_fail;
	}
	memset(priv->veu_stripe, 0, sizeof(struct veu_stripe));

	if ((struct veu_stripe *)(uintptr_t)data_copy.veu_stripe_ptr == NULL) {
		VEU_ERR("veu_stripe ptr is null");
		goto get_stripe_ptr_fail;
	}

	ret = copy_from_user(priv->veu_stripe, (struct veu_stripe *)(uintptr_t)data_copy.veu_stripe_ptr, sizeof(struct veu_stripe));
	if (ret) {
		VEU_ERR("copy from user fail: veu stripe");
		goto get_stripe_ptr_fail;
	}

	if (priv->veu_stripe->count < MIN_STRIPE_COUNT || priv->veu_stripe->count > MAX_STRIPE_COUNT) {
		VEU_ERR("stripe count %d error, ioctl failed", priv->veu_stripe->count);
		goto get_stripe_ptr_fail;
	}

	VEU_DBG("stripe count: %d", priv->veu_stripe->count);

	veu_ddr_vote(&task);

	priv->set_core_clk_rate(priv, priv->clk_rate_level);

	ret = veu_config(priv, &task);
	if (ret) {
		VEU_ERR("veu config fail, return");
		goto get_stripe_ptr_fail;
	}

	ret = veu_postprocess(priv);
	if (ret) {
		priv->off(priv);
		priv->on(priv);
	}

	veu_ddr_bandwidth_update(0);

get_stripe_ptr_fail:
	vfree(priv->veu_stripe);
	priv->veu_stripe = NULL;

get_data_copy_fail:
	if (priv->rdma_obj->iova)
		veu_mmu_unmap_layer_buffer(priv->rdma_obj);
	if (priv->wdma_obj->iova)
		veu_mmu_unmap_layer_buffer(priv->wdma_obj);

	return ret;
}

int veu_set_core_clk(struct file *filp, unsigned long arg)
{
	const void __user *argp = (void __user *)(uintptr_t)arg;
	struct veu_data *priv = NULL;
	uint32_t level;
	int ret;

	veu_check_and_return(!filp, -1, "filp is null");

	priv = (struct veu_data *)(filp->private_data);
	veu_check_and_return(!priv, -1, "priv is null");

	ret = copy_from_user(&level, argp, sizeof(uint32_t));
	veu_check_and_return(ret, -1, "copy from user fail");

	if (level > PROFILE_LVL_MAX) {
		VEU_ERR("invalid clk rate level %u", level);
		return -1;
	}
	priv->clk_rate_level = level;

	return 0;
}
