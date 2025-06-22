// SPDX-License-Identifier: GPL-2.0-only
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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#include "dpu_log.h"
#include "dpu_dt_util.h"
#include "dpu_hw_parser.h"

enum {
	SELF_TEST_BOOL,
	SELF_TEST_U32,
	SELF_TEST_U32_ARRAY,
	SELF_TEST_STRING,
	SELF_TEST_STRING_ARRAY,
	SELF_TEST_MAX
};

static struct prop_node self_test_nodes[] = {
	{SELF_TEST_BOOL, "test-bool", false, PROP_TYPE_BOOL},
	{SELF_TEST_U32, "test-u32", false, PROP_TYPE_U32},
	{SELF_TEST_U32_ARRAY, "test-u32-array", false, PROP_TYPE_U32_ARRAY},
	{SELF_TEST_STRING, "test-string", false, PROP_TYPE_STRING},
	{SELF_TEST_STRING_ARRAY, "test-string-array", false, PROP_TYPE_STRING_ARRAY}
};

enum {
	DPU_TOP_OFFSET,
	DPU_TOP_LEN,
	DPU_TOP_PROP_MAX
};

/* the device tree node group definition of dpu top block */
static struct prop_node dpu_top_nodes[] = {
	{DPU_TOP_OFFSET, "dpu-top-offset", true, PROP_TYPE_U32_ARRAY},
	{DPU_TOP_LEN, "dpu-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	CTL_TOP_OFFSET,
	CTL_TOP_LEN,
	CTL_TOP_PROP_MAX
};

/* the device tree node group definition of ctl top block */
static struct prop_node ctl_top_nodes[] = {
	{CTL_TOP_OFFSET, "ctl-top-offset", true, PROP_TYPE_U32_ARRAY},
	{CTL_TOP_LEN, "ctl-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	SCENE_CTL_OFFSET,
	SCENE_CTL_LEN,
	SCENE_CTL_PROP_MAX
};

/* the device tree node group definition of scene ctl block */
static struct prop_node scene_ctl_nodes[] = {
	{SCENE_CTL_OFFSET, "scene-ctl-offset", true, PROP_TYPE_U32_ARRAY},
	{SCENE_CTL_LEN, "scene-ctl-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	RCH_TOP_OFFSET,
	RCH_TOP_LEN,
	RCH_TOP_PROP_MAX
};

/* the device tree node group definition of r-channel top block */
static struct prop_node rch_top_nodes[] = {
	{RCH_TOP_OFFSET, "rch-top-offset", true, PROP_TYPE_U32_ARRAY},
	{RCH_TOP_LEN, "rch-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	RCH_OFFSET,
	RCH_LEN,
	RCH_TYPE,
	RCH_PROP_MAX
};

/* the device tree node group definition of r-channel block */
static struct prop_node rch_nodes[] = {
	{RCH_OFFSET, "rch-offset", true, PROP_TYPE_U32_ARRAY},
	{RCH_LEN, "rch-len", true, PROP_TYPE_U32_ARRAY},
	/* rch-type: rch, rch_v_lite, rch_g */
	{RCH_TYPE, "rch-type", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	PREPIPE_TOP_OFFSET,
	PREPIPE_TOP_LEN,
	PREPIPE_TOP_TYPE,
	PREPIPE_TOP_PROP_MAX
};

/* the device tree node group definition of prepipe top block */
static struct prop_node prepipe_top_nodes[] = {
	{PREPIPE_TOP_OFFSET, "prepipe-top-offset", true, PROP_TYPE_U32_ARRAY},
	{PREPIPE_TOP_LEN, "prepipe-top-len", true, PROP_TYPE_U32_ARRAY},
	/* prepipe-top-type: prepipe_top_lite, prepipe_top_full */
	{PREPIPE_TOP_TYPE, "prepipe-top-type", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	PRE_SCALER_OFFSET,
	PRE_SCALER_LEN,
	PRE_SCALER_BOUND_RCH,
	PRE_SCALER_PROP_MAX
};

/* the device tree node group definition of pre scaler block */
static struct prop_node pre_scaler_nodes[] = {
	{PRE_SCALER_OFFSET, "pre-scaler-offset", true, PROP_TYPE_U32_ARRAY},
	{PRE_SCALER_LEN, "pre-scaler-len", true, PROP_TYPE_U32_ARRAY},
	/* pre-scaler-bound-rch: rch_v_0 ~ 3 */
	{PRE_SCALER_BOUND_RCH, "pre-scaler-bound-rch", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	TM_OFFSET,
	TM_LEN,
	TM_TYPE,
	TM_PROP_MAX
};

/* the device tree node group definition of tone mapping block */
static struct prop_node tm_nodes[] = {
	{TM_OFFSET, "tm-offset", true, PROP_TYPE_U32_ARRAY},
	{TM_LEN, "tm-len", true, PROP_TYPE_U32_ARRAY},
	/* tm-type: tm, tm_lite */
	{TM_TYPE, "tm-type", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	PRE_LUT3D_OFFSET,
	PRE_LUT3D_LEN,
	PRE_LUT3D_BOUND_RCH,
	PRE_LUT3D_PROP_MAX
};

/* the device tree node group definition of pre_lut3d block */
static struct prop_node pre_lut3d_nodes[] = {
	{PRE_LUT3D_OFFSET, "pre-lut3d-offset", true, PROP_TYPE_U32_ARRAY},
	{PRE_LUT3D_LEN, "pre-lut3d-len", true, PROP_TYPE_U32_ARRAY},
	/* pre-lut3d-bound-rch: rch_v_0 */
	{PRE_LUT3D_BOUND_RCH, "pre-lut3d-bound-rch", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	PRE_HIST_OFFSET,
	PRE_HIST_LEN,
	PRE_HIST_BOUND_RCH,
	PRE_HIST_PROP_MAX
};

/* the device tree node group definition of pre_hist block */
static struct prop_node pre_hist_nodes[] = {
	{PRE_HIST_OFFSET, "pre-hist-offset", true, PROP_TYPE_U32_ARRAY},
	{PRE_HIST_LEN, "pre-hist-len", true, PROP_TYPE_U32_ARRAY},
	/* pre-hist-bound-rch: rch_v_0 */
	{PRE_HIST_BOUND_RCH, "pre-hist-bound-rch", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	MIXER_OFFSET,
	MIXER_LEN,
	MIXER_TYPE,
	MIXER_PROP_MAX
};

/* the device tree node group definition of mixer block */
static struct prop_node mixer_nodes[] = {
	{MIXER_OFFSET, "mixer-offset", true, PROP_TYPE_U32_ARRAY},
	{MIXER_LEN, "mixer-len", true, PROP_TYPE_U32_ARRAY},
	/* mixer-type: "primary", "secondary", "writeback" */
	{MIXER_TYPE, "mixer-type", true, PROP_TYPE_STRING_ARRAY},
};

enum {
	POST_PIPE_TOP_OFFSET,
	POST_PIPE_TOP_LEN,
	POST_PIPE_TOP_PROP_MAX
};

static struct prop_node post_pipe_top_nodes[] = {
	{POST_PIPE_TOP_OFFSET, "post-pipe-top-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PIPE_TOP_LEN, "post-pipe-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_PQ_TOP_OFFSET,
	POST_PQ_TOP_LEN,
	POST_PQ_TOP_PROP_MAX
};

static struct prop_node post_pq_top_nodes[] = {
	{POST_PQ_TOP_OFFSET, "post-pq-top-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PQ_TOP_LEN, "post-pq-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_PRE_PROC_OFFSET,
	POST_PRE_PROC_LEN,
	POST_PRE_PROC_PROP_MAX
};

static struct prop_node post_pre_proc_nodes[] = {
	{POST_PRE_PROC_OFFSET, "post-pre-proc-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PRE_PROC_LEN, "post-pre-proc-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	ACAD_OFFSET,
	ACAD_LEN,
	ACAD_PROP_MAX
};

static struct prop_node acad_nodes[] = {
	{ACAD_OFFSET, "acad-offset", true, PROP_TYPE_U32_ARRAY},
	{ACAD_LEN, "acad-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_3DLUT_OFFSET,
	POST_3DLUT_LEN,
	POST_3DLUT_PROP_MAX
};

static struct prop_node post_3dlut_nodes[] = {
	{POST_3DLUT_OFFSET, "post-lut-3d-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_3DLUT_LEN, "post-lut-3d-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_GAMMA_OFFSET,
	POST_GAMMA_LEN,
	POST_GAMMA_PROP_MAX
};

static struct prop_node post_gamma_nodes[] = {
	{POST_GAMMA_OFFSET, "post-gamma-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_GAMMA_LEN, "post-gamma-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_HIST_OFFSET,
	POST_HIST_LEN,
	POST_HIST_PROP_MAX
};

static struct prop_node post_hist_nodes[] = {
	{POST_HIST_OFFSET, "post-histogram-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_HIST_LEN, "post-histogram-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_SCALER_OFFSET,
	POST_SCALER_LEN,
	POST_SCALER_PROP_MAX
};

static struct prop_node post_scale_nodes[] = {
	{POST_SCALER_OFFSET, "post-scale-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_SCALER_LEN, "post-scale-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DSC_OFFSET,
	DSC_LEN,
	DSC_PROP_MAX
};

/* the device tree node group definition of dsc block */
static struct prop_node dsc_nodes[] = {
	{DSC_OFFSET, "dsc-offset", true, PROP_TYPE_U32_ARRAY},
	{DSC_LEN, "dsc-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DSC_WDMA_OFFSET,
	DSC_WDMA_LEN,
	DSC_WDMA_PROP_MAX
};

/* the device tree node group definition of dsc wdma_ block */
static struct prop_node dsc_wdma_nodes[] = {
	{DSC_WDMA_OFFSET, "dsc-wdma-offset", true, PROP_TYPE_U32_ARRAY},
	{DSC_WDMA_LEN, "dsc-wdma-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DSC_RDMA_OFFSET,
	DSC_RDMA_LEN,
	DSC_RDMA_PROP_MAX
};

/* the device tree node group definition of dsc rdma block */
static struct prop_node dsc_rdma_nodes[] = {
	{DSC_RDMA_OFFSET, "dsc-rdma-offset", true, PROP_TYPE_U32_ARRAY},
	{DSC_RDMA_LEN, "dsc-rdma-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	WB_TOP_OFFSET,
	WB_TOP_LEN,
	WB_TOP_PROP_MAX
};

/* the device tree node group definition of wb top block */
static struct prop_node wb_top_nodes[] = {
	{WB_TOP_OFFSET, "wb-top-offset", true, PROP_TYPE_U32_ARRAY},
	{WB_TOP_LEN, "wb-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	WB_SCALER_OFFSET,
	WB_SCALER_LEN,
	WB_SCALER_PROP_MAX
};

/* the device tree node group definition of wb scaler block */
static struct prop_node wb_scaler_nodes[] = {
	{WB_SCALER_OFFSET, "wb-scaler-offset", true, PROP_TYPE_U32_ARRAY},
	{WB_SCALER_LEN, "wb-scaler-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	WB_CORE_OFFSET,
	WB_CORE_LEN,
	WB_CORE_OFFLINE_ID,
	WB_CORE_PROP_MAX
};

/* offline mixer id for wb core */
enum {
	WB_CORE_OFFLINE_MIXER_1 = 1,
	WB_CORE_OFFLINE_MIXER_2 = 2
};

/* the device tree node group definition of wb core block */
static struct prop_node wb_core_nodes[] = {
	{WB_CORE_OFFSET, "wb-core-offset", true, PROP_TYPE_U32_ARRAY},
	{WB_CORE_LEN, "wb-core-len", true, PROP_TYPE_U32_ARRAY},
	/* offline mixer id for wb offline mode */
	{WB_CORE_OFFLINE_ID, "wb-core-offline-id", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DPU_INTR_OFFSET,
	DPU_INTR_LEN,
	DPU_INTR_PROP_MAX
};

/* the device tree node group definition of dpu interrupt block */
static struct prop_node dpu_intr_nodes[] = {
	{DPU_INTR_OFFSET, "intr-offset", true, PROP_TYPE_U32_ARRAY},
	{DPU_INTR_LEN, "intr-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DPU_GLB_OFFSET,
	DPU_GLB_LEN,
	DPU_GLB_PROP_MAX
};

/* the device tree node group definition of dpu glb block */
static struct prop_node dpu_glb_nodes[] = {
	{DPU_GLB_OFFSET, "glb-offset", true, PROP_TYPE_U32_ARRAY},
	{DPU_GLB_LEN, "glb-len", true, PROP_TYPE_U32_ARRAY},
};

static int __maybe_unused dpu_parse_self_test_nodes(struct device_node *np)
{
	struct prop_data *prop_data;
	int ret;
	u32 i;

	if (!np) {
		DPU_ERROR("invalid parameters, %pK\n", np);
		return -EINVAL;
	}

	DPU_DEBUG("self-test start\n");

	prop_data = kcalloc(SELF_TEST_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, self_test_nodes, SELF_TEST_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse self test nodes %d\n", ret);
		goto error;
	}

	DPU_DEBUG("SELF_TEST_BOOL: %s\n",
			prop_data[SELF_TEST_BOOL].value[0] ? "true" : "false");

	if (prop_data[SELF_TEST_U32].counts)
		DPU_DEBUG("SELF_TEST_U32: %x\n",
				prop_data[SELF_TEST_U32].value[0]);

	for (i = 0; i < prop_data[SELF_TEST_U32_ARRAY].counts; i++)
		DPU_DEBUG("SELF_TEST_U32_ARRAY: %x\n",
				prop_data[SELF_TEST_U32_ARRAY].value[i]);

	if (prop_data[SELF_TEST_STRING].counts)
		DPU_DEBUG("SELF_TEST_STRING: %s\n",
				prop_data[SELF_TEST_STRING].string[0]);

	for (i = 0; i < prop_data[SELF_TEST_STRING_ARRAY].counts; i++)
		DPU_DEBUG("SELF_TEST_STRING_ARRAY: %s",
				prop_data[SELF_TEST_STRING_ARRAY].string[i]);

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_base_iomem(struct platform_device *pdev,
		struct dpu_iomem *dpu_mem)
{
	static const char *base_name = "dpu_base";
	void __iomem *iomem_ptr;
	struct resource *res;
	u32 size;

	iomem_ptr = dpu_iomem_remap(pdev, base_name);
	if (IS_ERR_OR_NULL(iomem_ptr))
		return -ENXIO;

	size = dpu_iomem_size(pdev, base_name);
	if (!size)
		return -ENXIO;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, base_name);

	dpu_mem->base = iomem_ptr;
	dpu_mem->addr = (u32)res->start;
	dpu_mem->len = size;

	return 0;
}

static int dpu_parse_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dpu_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DPU_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dpu_top_nodes, DPU_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dpu top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DPU_TOP_OFFSET].counts;
	if (prop_count > DPU_TOP_MAX) {
		DPU_ERROR("wrong dpu top property count %d/%d\n",
				prop_count, DPU_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dpu_top_base = &dpu_cap->dpu_top[i].base;

		snprintf(dpu_top_base->name, MAX_NAME_LEN, "dpu_top_%d", i);
		dpu_top_base->id = DPU_TOP_0 + i;
		dpu_top_base->addr = prop_data[DPU_TOP_OFFSET].value[i];
		dpu_top_base->len = prop_data[DPU_TOP_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				dpu_top_base->name,
				dpu_top_base->id,
				dpu_top_base->addr,
				dpu_top_base->len);
	}

	dpu_cap->dpu_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_ctl_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *ctl_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(CTL_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, ctl_top_nodes, CTL_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse ctl top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[CTL_TOP_OFFSET].counts;
	if (prop_count > CTL_TOP_MAX) {
		DPU_ERROR("wrong ctl top property count %d/%d\n",
				prop_count, CTL_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		ctl_top_base = &dpu_cap->ctl_top[i].base;

		snprintf(ctl_top_base->name, MAX_NAME_LEN, "ctl_top_%d", i);
		ctl_top_base->id = CTL_TOP_0 + i;
		ctl_top_base->addr = prop_data[CTL_TOP_OFFSET].value[i];
		ctl_top_base->len = prop_data[CTL_TOP_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				ctl_top_base->name,
				ctl_top_base->id,
				ctl_top_base->addr,
				ctl_top_base->len);
	}

	dpu_cap->ctl_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_scene_ctl(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *scene_ctl_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(SCENE_CTL_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, scene_ctl_nodes,
			SCENE_CTL_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse scene ctl nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[SCENE_CTL_OFFSET].counts;
	if (prop_count > SCENE_CTL_MAX) {
		DPU_ERROR("wrong scene ctl property count %d/%d\n",
				prop_count, SCENE_CTL_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		scene_ctl_base = &dpu_cap->scene_ctl[i].base;

		snprintf(scene_ctl_base->name, MAX_NAME_LEN, "scene_ctl_%d", i);
		scene_ctl_base->id = SCENE_CTL_0 + i;
		scene_ctl_base->addr = prop_data[SCENE_CTL_OFFSET].value[i];
		scene_ctl_base->len = prop_data[SCENE_CTL_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				scene_ctl_base->name,
				scene_ctl_base->id,
				scene_ctl_base->addr,
				scene_ctl_base->len);
	}

	dpu_cap->scene_ctl_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_intr(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dpu_intr_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DPU_INTR_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dpu_intr_nodes,
			DPU_INTR_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dpu intr nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DPU_INTR_OFFSET].counts;
	if (prop_count > DPU_INTR_MAX) {
		DPU_ERROR("wrong dpu intr property count %d/%d\n",
				prop_count, DPU_INTR_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dpu_intr_base = &dpu_cap->dpu_intr[i].base;

		snprintf(dpu_intr_base->name, MAX_NAME_LEN, "dpu_intr_%d", i);
		dpu_intr_base->id = DPU_INTR_0 + i;
		dpu_intr_base->addr = prop_data[DPU_INTR_OFFSET].value[i];
		dpu_intr_base->len = prop_data[DPU_INTR_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				dpu_intr_base->name,
				dpu_intr_base->id,
				dpu_intr_base->addr,
				dpu_intr_base->len);
	}

	dpu_cap->dpu_intr_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_rch_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *rch_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(RCH_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, rch_top_nodes, RCH_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse rch top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[RCH_TOP_OFFSET].counts;
	if (prop_count > RCH_TOP_MAX) {
		DPU_ERROR("wrong rch top property count %d/%d\n",
				prop_count, RCH_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		rch_top_base = &dpu_cap->rch_top[i].base;

		snprintf(rch_top_base->name, MAX_NAME_LEN, "rch_top_%d", i);
		rch_top_base->id = RCH_TOP_0 + i;
		rch_top_base->addr = prop_data[RCH_TOP_OFFSET].value[i];
		rch_top_base->len = prop_data[RCH_TOP_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				rch_top_base->name,
				rch_top_base->id,
				rch_top_base->addr,
				rch_top_base->len);
	}

	dpu_cap->rch_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_rch(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *rch_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(RCH_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, rch_nodes, RCH_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse rch nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[RCH_OFFSET].counts;
	if (prop_count > RCH_MAX) {
		DPU_ERROR("wrong rch property count %d/%d\n",
				prop_count, RCH_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		rch_base = &dpu_cap->rch[i].base;

		if (!strcmp(prop_data[RCH_TYPE].string[i], "rch_g")) {
			rch_base->id = RCH_G_0 + i;
			set_bit(RCH_G, &rch_base->features);
			snprintf(rch_base->name, MAX_NAME_LEN, "%s_%d",
					prop_data[RCH_TYPE].string[i], i);
		} else if (!strcmp(prop_data[RCH_TYPE].string[i], "rch_v_lite")) {
			rch_base->id = RCH_G_0 + i;
			set_bit(RCH_V_LITE, &rch_base->features);
			snprintf(rch_base->name, MAX_NAME_LEN, "%s_%d",
					prop_data[RCH_TYPE].string[i], i);
		} else if (!strcmp(prop_data[RCH_TYPE].string[i], "rch_v_full")) {
			rch_base->id = RCH_G_0 + i;
			set_bit(RCH_V_FULL, &rch_base->features);
			snprintf(rch_base->name, MAX_NAME_LEN, "%s_%d",
					prop_data[RCH_TYPE].string[i], i);
		} else {
			DPU_ERROR("unsupported rch type: %s\n",
					prop_data[RCH_TYPE].string[i]);
			ret = -EINVAL;
			goto error;
		}

		rch_base->addr = prop_data[RCH_OFFSET].value[i];
		rch_base->len = prop_data[RCH_LEN].value[i];

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				rch_base->name, rch_base->id,
				rch_base->addr, rch_base->len);
		DPU_DEBUG("name: %s, features: 0x%lx\n",
				rch_base->name, rch_base->features);
	}

	dpu_cap->rch_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_prepipe_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(PREPIPE_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, prepipe_top_nodes, PREPIPE_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse prepipe_top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PREPIPE_TOP_OFFSET].counts;
	if (prop_count > PREPIPE_TOP_MAX) {
		DPU_ERROR("wrong prepipe top property count %d/%d\n",
				prop_count, PREPIPE_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->prepipe_top[i].base;

		if (!strcmp(prop_data[PREPIPE_TOP_TYPE].string[i], "prepipe_top_lite")) {
			set_bit(PREPIPE_TOP_LITE, &base->features);
		} else if (!strcmp(prop_data[PREPIPE_TOP_TYPE].string[i], "prepipe_top_full")) {
			set_bit(PREPIPE_TOP_FULL, &base->features);
		} else {
			DPU_ERROR("unsupported prepipe top type: %s\n",
					prop_data[PREPIPE_TOP_TYPE].string[i]);
			ret = -EINVAL;
			goto error;
		}
		base->id = RCH_G_0 + i;
		base->addr = prop_data[PREPIPE_TOP_OFFSET].value[i];
		base->len = prop_data[PREPIPE_TOP_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				prop_data[PREPIPE_TOP_TYPE].string[i], i);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->prepipe_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_pre_scaler(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(PRE_SCALER_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, pre_scaler_nodes, PRE_SCALER_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse pre scaler nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_SCALER_OFFSET].counts;
	if (prop_count > PRE_SCALER_MAX) {
		DPU_ERROR("wrong pre scaler property count %d/%d\n",
				prop_count, PRE_SCALER_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->pre_scaler[i].base;
		if (!strcmp(prop_data[PRE_SCALER_BOUND_RCH].string[i], "rch_v_0")) {
			base->id = RCH_V_0;
		} else if (!strcmp(prop_data[PRE_SCALER_BOUND_RCH].string[i], "rch_v_1")) {
			base->id = RCH_V_1;
		} else if (!strcmp(prop_data[PRE_SCALER_BOUND_RCH].string[i], "rch_v_2")) {
			base->id = RCH_V_2;
		} else if (!strcmp(prop_data[PRE_SCALER_BOUND_RCH].string[i], "rch_v_3")) {
			base->id = RCH_V_3;
		} else {
			DPU_ERROR("unsupported pre scaler belongs to %s\n",
					prop_data[PRE_SCALER_BOUND_RCH].string[i]);
			ret = -EINVAL;
			goto error;
		}
		base->addr = prop_data[PRE_SCALER_OFFSET].value[i];
		base->len = prop_data[PRE_SCALER_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_scaler",
				prop_data[PRE_SCALER_BOUND_RCH].string[i]);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->pre_scaler_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_tm(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(TM_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, tm_nodes, TM_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse tm nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[TM_OFFSET].counts;
	if (prop_count > TM_MAX) {
		DPU_ERROR("wrong tm property count %d/%d\n",
				prop_count, TM_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->tm[i].base;
		if (!strcmp(prop_data[TM_TYPE].string[i], "tm")) {
			set_bit(FEATURE_TM, &base->features);
		} else if (!strcmp(prop_data[TM_TYPE].string[i], "tm_lite")) {
			set_bit(FEATURE_TM_LITE, &base->features);
		} else {
			DPU_ERROR("unsupported tm type: %s\n",
					prop_data[TM_TYPE].string[i]);
			ret = -EINVAL;
			goto error;
		}
		base->id = RCH_G_0 + i;
		base->addr = prop_data[TM_OFFSET].value[i];
		base->len = prop_data[TM_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				prop_data[TM_TYPE].string[i], i);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->tm_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_pre_lut3d(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(PRE_LUT3D_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, pre_lut3d_nodes, PRE_LUT3D_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse pre lut3d nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_LUT3D_OFFSET].counts;
	if (prop_count > PRE_LUT3D_MAX) {
		DPU_ERROR("wrong pre lut3d property count %d/%d\n",
				prop_count, PRE_LUT3D_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->pre_lut3d[i].base;
		if (!strcmp(prop_data[PRE_LUT3D_BOUND_RCH].string[i], "rch_v_0")) {
			base->id = RCH_V_0;
		} else {
			DPU_ERROR("unsupported pre lut3d belongs to %s\n",
					prop_data[PRE_LUT3D_BOUND_RCH].string[i]);
			ret = -EINVAL;
			goto error;
		}
		base->addr = prop_data[PRE_LUT3D_OFFSET].value[i];
		base->len = prop_data[PRE_LUT3D_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s-lut3d",
				prop_data[PRE_LUT3D_BOUND_RCH].string[i]);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->pre_lut3d_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_pre_hist(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(PRE_HIST_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, pre_hist_nodes, PRE_HIST_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse pre hist nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_HIST_OFFSET].counts;
	if (prop_count > PRE_HIST_MAX) {
		DPU_ERROR("wrong pre hist property count %d/%d\n",
				prop_count, PRE_HIST_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->pre_hist[i].base;
		if (!strcmp(prop_data[PRE_HIST_BOUND_RCH].string[i], "rch_v_0")) {
			base->id = RCH_V_0;
		} else {
			DPU_ERROR("unsupported pre hist type: %s\n",
					prop_data[PRE_HIST_BOUND_RCH].string[0]);
			ret = -EINVAL;
			goto error;
		}

		base->addr = prop_data[PRE_HIST_OFFSET].value[i];
		base->len = prop_data[PRE_HIST_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s-0",
				prop_data[PRE_HIST_BOUND_RCH].string[i]);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}
	dpu_cap->pre_hist_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_mixer(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *mixer_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(MIXER_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, mixer_nodes, MIXER_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse mixer nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[MIXER_OFFSET].counts;
	if (prop_count > MIXER_MAX) {
		DPU_ERROR("wrong mixer property count %d/%d\n",
				prop_count, MIXER_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		mixer_base = &dpu_cap->mixer[i].base;

		if (!strcmp(prop_data[MIXER_TYPE].string[i], "primary")) {
			mixer_base->id = MIXER_0 + i;
			set_bit(MIXER_PRIMARY, &mixer_base->features);
			snprintf(mixer_base->name, MAX_NAME_LEN, "%s_%d",
					"mixer", mixer_base->id);
		} else if (!strcmp(prop_data[MIXER_TYPE].string[i], "writeback")) {
			mixer_base->id = MIXER_0 + i;
			set_bit(MIXER_WB, &mixer_base->features);
			snprintf(mixer_base->name, MAX_NAME_LEN, "%s_%d",
					"mixer", mixer_base->id);
		} else {
			mixer_base->id = MIXER_0 + i;
			set_bit(MIXER_SECONDARY, &mixer_base->features);
			snprintf(mixer_base->name, MAX_NAME_LEN, "%s_%d",
					"mixer", mixer_base->id);
		}

		mixer_base->addr = prop_data[MIXER_OFFSET].value[i];
		mixer_base->len = prop_data[MIXER_LEN].value[i];

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				mixer_base->name, mixer_base->id,
				mixer_base->addr, mixer_base->len);
		DPU_DEBUG("name: %s, features: 0x%lx\n",
				mixer_base->name, mixer_base->features);
	}

	dpu_cap->mixer_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_pipe_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_PIPE_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_pipe_top_nodes,
			POST_PIPE_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_pipe_top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PIPE_TOP_OFFSET].counts;
	if (prop_count > POST_PIPE_TOP_MAX) {
		DPU_ERROR("wrong post_pipe_top property count %d/%d\n",
				prop_count, POST_PIPE_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pipe_top[i].base;

		base->addr = prop_data[POST_PIPE_TOP_OFFSET].value[i];
		base->len = prop_data[POST_PIPE_TOP_LEN].value[i];
		base->id = POST_PIPE_TOP_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pipe_top", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pipe_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_pq_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_PQ_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_pq_top_nodes,
			POST_PQ_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_pq_top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PQ_TOP_OFFSET].counts;
	if (prop_count > POST_PQ_TOP_MAX) {
		DPU_ERROR("wrong post_pq_top property count %d/%d\n",
				prop_count, POST_PQ_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pq_top[i].base;

		base->addr = prop_data[POST_PQ_TOP_OFFSET].value[i];
		base->len = prop_data[POST_PQ_TOP_LEN].value[i];
		base->id = POST_PQ_TOP_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pq_top", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pq_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_pre_proc(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_PRE_PROC_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_pre_proc_nodes,
			POST_PRE_PROC_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_pre_proc nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PRE_PROC_OFFSET].counts;
	if (prop_count > POST_PRE_PROC_MAX) {
		DPU_ERROR("wrong post_pre_proc property count %d/%d\n",
				prop_count, POST_PRE_PROC_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pre_proc[i].base;

		base->addr = prop_data[POST_PRE_PROC_OFFSET].value[i];
		base->len = prop_data[POST_PRE_PROC_LEN].value[i];
		base->id = POST_PRE_PROC_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pre_proc", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pre_proc_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_acad(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(ACAD_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, acad_nodes, ACAD_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse acad nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[ACAD_OFFSET].counts;
	if (prop_count > ACAD_MAX) {
		DPU_ERROR("wrong acad property count %d/%d\n",
				prop_count, ACAD_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->acad[i].base;

		base->addr = prop_data[ACAD_OFFSET].value[i];
		base->len = prop_data[ACAD_LEN].value[i];
		base->id = ACAD_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"acad", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->acad_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_3dlut(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_3DLUT_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_3dlut_nodes,
			POST_3DLUT_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_3dlut nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_3DLUT_OFFSET].counts;
	if (prop_count > POST_3DLUT_MAX) {
		DPU_ERROR("wrong post_3dlut property count %d/%d\n",
				prop_count, POST_3DLUT_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_3dlut[i].base;

		base->addr = prop_data[POST_3DLUT_OFFSET].value[i];
		base->len = prop_data[POST_3DLUT_LEN].value[i];
		base->id = POST_3DLUT_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_3dlut", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_3dlut_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_gamma(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_GAMMA_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_gamma_nodes,
			POST_GAMMA_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_gamma nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_GAMMA_OFFSET].counts;
	if (prop_count > POST_GAMMA_MAX) {
		DPU_ERROR("wrong post_gamma property count %d/%d\n",
				prop_count, POST_GAMMA_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_gamma[i].base;

		base->addr = prop_data[POST_GAMMA_OFFSET].value[i];
		base->len = prop_data[POST_GAMMA_LEN].value[i];
		base->id = POST_GAMMA_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_gamma", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_gamma_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_hist(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_HIST_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_hist_nodes,
			POST_HIST_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_hist nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_HIST_OFFSET].counts;
	if (prop_count > POST_HIST_MAX) {
		DPU_ERROR("wrong post_hist property count %d/%d\n",
				prop_count, POST_HIST_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_hist[i].base;

		base->addr = prop_data[POST_HIST_OFFSET].value[i];
		base->len = prop_data[POST_HIST_LEN].value[i];
		base->id = POST_HIST_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_hist", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_hist_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_post_scaler(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = kcalloc(POST_SCALER_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, post_scale_nodes,
			POST_SCALER_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse post_scale nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_SCALER_OFFSET].counts;
	if (prop_count > POST_SCALER_MAX) {
		DPU_ERROR("wrong post_scale property count %d/%d\n",
				prop_count, POST_SCALER_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_scaler[i].base;

		base->addr = prop_data[POST_SCALER_OFFSET].value[i];
		base->len = prop_data[POST_SCALER_LEN].value[i];
		base->id = POST_SCALER_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_scaler", base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_scaler_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_postpq(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	int ret;

	DPU_DEBUG("enter\n");

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	ret = dpu_parse_post_pipe_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post_pipe_top\n");
		return ret;
	}

	ret = dpu_parse_post_pq_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post_pq_top\n");
		return ret;
	}

	ret = dpu_parse_post_pre_proc(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post_pre_proc\n");
		return ret;
	}

	ret = dpu_parse_acad(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse acad\n");
		return ret;
	}

	ret = dpu_parse_post_3dlut(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post 3dlut\n");
		return ret;
	}

	ret = dpu_parse_post_gamma(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post gamma\n");
		return ret;
	}

	ret = dpu_parse_post_hist(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post hist\n");
		return ret;
	}

	ret = dpu_parse_post_scaler(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse post scaler\n");
		return ret;
	}

	DPU_DEBUG("exit\n");
	return ret;
}

static int dpu_parse_dsc(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dsc_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DSC_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dsc_nodes, DSC_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dsc nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DSC_OFFSET].counts;
	if (prop_count > DSC_MAX) {
		DPU_ERROR("wrong dsc property count %d/%d\n",
				prop_count, DSC_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dsc_base = &dpu_cap->dsc[i].base;

		dsc_base->addr = prop_data[DSC_OFFSET].value[i];
		dsc_base->len = prop_data[DSC_LEN].value[i];
		dsc_base->id = DSC_0 + i;
		snprintf(dsc_base->name, MAX_NAME_LEN, "%s_%d",
				"dsc", dsc_base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				dsc_base->name, dsc_base->id,
				dsc_base->addr, dsc_base->len);
	}

	dpu_cap->dsc_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_dsc_wdma(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dsc_wdma_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DSC_WDMA_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dsc_wdma_nodes, DSC_WDMA_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dsc wdma nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DSC_WDMA_OFFSET].counts;
	if (prop_count > DSC_WDMA_MAX) {
		DPU_ERROR("wrong dsc wdma property count %d/%d\n",
				prop_count, DSC_WDMA_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dsc_wdma_base = &dpu_cap->dsc_wdma[i].base;

		dsc_wdma_base->addr = prop_data[DSC_WDMA_OFFSET].value[i];
		dsc_wdma_base->len = prop_data[DSC_WDMA_LEN].value[i];
		dsc_wdma_base->id = DSC_WDMA_0 + i;
		snprintf(dsc_wdma_base->name, MAX_NAME_LEN, "%s_%d", "dscw", dsc_wdma_base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				dsc_wdma_base->name, dsc_wdma_base->id,
				dsc_wdma_base->addr, dsc_wdma_base->len);
	}

	dpu_cap->dsc_wdma_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_dsc_rdma(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dsc_rdma_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DSC_RDMA_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dsc_rdma_nodes, DSC_RDMA_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dsc rdma nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DSC_RDMA_OFFSET].counts;
	if (prop_count > DSC_RDMA_MAX) {
		DPU_ERROR("wrong dsc rdma property count %d/%d\n",
				prop_count, DSC_RDMA_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dsc_rdma_base = &dpu_cap->dsc_rdma[i].base;

		dsc_rdma_base->addr = prop_data[DSC_RDMA_OFFSET].value[i];
		dsc_rdma_base->len = prop_data[DSC_RDMA_LEN].value[i];
		dsc_rdma_base->id = DSC_RDMA_0 + i;
		snprintf(dsc_rdma_base->name, MAX_NAME_LEN, "%s_%d", "dscr", dsc_rdma_base->id);

		DPU_DEBUG("name: %s, id: %d, addr: 0x%08x, len: %d\n",
				dsc_rdma_base->name, dsc_rdma_base->id,
				dsc_rdma_base->addr, dsc_rdma_base->len);
	}

	dpu_cap->dsc_rdma_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_wb_top(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *wb_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(WB_TOP_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, wb_top_nodes, WB_TOP_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse wb top nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_TOP_OFFSET].counts;
	if (prop_count > WB_TOP_MAX) {
		DPU_ERROR("wrong wb top property count %d/%d\n",
				prop_count, WB_TOP_MAX);
		ret = -EINVAL;
		goto error;
	}


	for (i = 0; i < prop_count; i++) {
		wb_top_base = &dpu_cap->wb_top[i].base;

		snprintf(wb_top_base->name, MAX_NAME_LEN, "wb_top_%d", i);
		wb_top_base->id = WB_TOP_0 + i;
		wb_top_base->addr = prop_data[WB_TOP_OFFSET].value[i];
		wb_top_base->len = prop_data[WB_TOP_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				wb_top_base->name,
				wb_top_base->id,
				wb_top_base->addr,
				wb_top_base->len);
	}

	dpu_cap->wb_top_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_wb_core(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *wb_core_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	u32 offline_id;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(WB_CORE_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, wb_core_nodes, WB_CORE_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse wb core nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_CORE_OFFSET].counts;
	if (prop_count > WB_CORE_MAX) {
		DPU_ERROR("wrong wb core property count %d/%d\n",
				prop_count, WB_CORE_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		wb_core_base = &dpu_cap->wb_core[i].base;
		offline_id = prop_data[WB_CORE_OFFLINE_ID].value[i];

		if (offline_id == WB_CORE_OFFLINE_MIXER_1) {
			set_bit(WB_CORE_OFFLINE_0, &wb_core_base->features);
		} else if (offline_id == WB_CORE_OFFLINE_MIXER_2) {
			set_bit(WB_CORE_OFFLINE_1, &wb_core_base->features);
		} else {
			DPU_ERROR("unsupported offline id for wb core, %u\n",
					offline_id);
			ret = -EINVAL;
			goto error;
		}

		snprintf(wb_core_base->name, MAX_NAME_LEN, "wb_core_%d", i);
		wb_core_base->id = WB_CORE_0 + i;
		wb_core_base->addr = prop_data[WB_CORE_OFFSET].value[i];
		wb_core_base->len = prop_data[WB_CORE_LEN].value[i];

		DPU_DEBUG("name: %s, id: %d, addr: %x, len: %x, feature %lx\n",
				wb_core_base->name,
				wb_core_base->id,
				wb_core_base->addr,
				wb_core_base->len,
				wb_core_base->features);
	}

	dpu_cap->wb_core_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_wb_scaler(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *wb_scaler_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(WB_SCALER_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(prop_data))
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, wb_scaler_nodes, WB_SCALER_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse wb scaler nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_SCALER_OFFSET].counts;
	if (prop_count > WB_SCALER_MAX) {
		DPU_ERROR("wrong wb scaler property count %d/%d\n",
				prop_count, WB_SCALER_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		wb_scaler_base = &dpu_cap->wb_scaler[i].base;

		snprintf(wb_scaler_base->name, MAX_NAME_LEN, "wb_scaler_%d", i);
		wb_scaler_base->id = WB_SCALER_0 + i;
		wb_scaler_base->addr = prop_data[WB_SCALER_OFFSET].value[i];
		wb_scaler_base->len = prop_data[WB_SCALER_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				wb_scaler_base->name,
				wb_scaler_base->id,
				wb_scaler_base->addr,
				wb_scaler_base->len);
	}

	dpu_cap->wb_scaler_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

static int dpu_parse_dpu_glb(struct device_node *np,
		struct dpu_cap *dpu_cap)
{
	struct dpu_hw_base *dpu_glb_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	if (!np || !dpu_cap) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", np, dpu_cap);
		return -EINVAL;
	}

	prop_data = kcalloc(DPU_GLB_PROP_MAX, sizeof(*prop_data), GFP_KERNEL);
	if (!prop_data)
		return -ENOMEM;

	ret = dpu_parse_dt_nodes(np, dpu_glb_nodes, DPU_GLB_PROP_MAX, prop_data);
	if (ret) {
		DPU_ERROR("failed to parse dpu glb nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DPU_GLB_OFFSET].counts;
	if (prop_count > DPU_GLB_MAX) {
		DPU_ERROR("wrong dpu glb property count %d/%d\n",
				prop_count, DPU_GLB_MAX);
		ret = -EINVAL;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dpu_glb_base = &dpu_cap->dpu_glb[i].base;

		snprintf(dpu_glb_base->name, MAX_NAME_LEN, "dpu_glb_%d", i);
		dpu_glb_base->id = DPU_GLB_0 + i;
		dpu_glb_base->addr = prop_data[DPU_GLB_OFFSET].value[i];
		dpu_glb_base->len = prop_data[DPU_GLB_LEN].value[i];

		DPU_DEBUG("name :%s, id: %d, addr: %x, len: %x\n",
				dpu_glb_base->name,
				dpu_glb_base->id,
				dpu_glb_base->addr,
				dpu_glb_base->len);
	}

	dpu_cap->dpu_glb_count = prop_count;

error:
	kfree(prop_data);
	return ret;
}

int dpu_cap_init(struct drm_device *dev,
		struct dpu_cap *dpu_cap)
{
	struct platform_device *pdev;
	struct device_node *np;
	int ret;

	if (!dev || !dpu_cap) {
		DPU_ERROR("invalid parameters: %pK, %pK\n", dev, dpu_cap);
		return -EINVAL;
	}

	pdev = to_platform_device(dev->dev);
	np = pdev->dev.of_node;

	ret = dpu_parse_base_iomem(pdev, &dpu_cap->dpu_mem);
	if (ret) {
		DPU_ERROR("failed to init dpu iomem\n");
		return ret;
	}

	ret = dpu_parse_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dpu_top\n");
		return ret;
	}

	ret = dpu_parse_ctl_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse ctl_top\n");
		return ret;
	}

	ret = dpu_parse_scene_ctl(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse scene_ctl\n");
		return ret;
	}

	ret = dpu_parse_intr(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dpu intr\n");
		return ret;
	}

	ret = dpu_parse_rch_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse rch_top\n");
		return ret;
	}

	ret = dpu_parse_rch(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse rch\n");
		return ret;
	}

	ret = dpu_parse_prepipe_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse prepipe top\n");
		return ret;
	}

	ret = dpu_parse_pre_scaler(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse pre scaler\n");
		return ret;
	}

	ret = dpu_parse_tm(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse tm\n");
		return ret;
	}

	ret = dpu_parse_pre_lut3d(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse pre lut3d\n");
		return ret;
	}

	ret = dpu_parse_pre_hist(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse pre histogram\n");
		return ret;
	}

	ret = dpu_parse_mixer(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse mixer\n");
		return ret;
	}

	ret = dpu_parse_postpq(np, dpu_cap);
	if (ret)
		DPU_ERROR("failed to parse postpq\n");

	ret = dpu_parse_dsc(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dsc\n");
		return ret;
	}

	ret = dpu_parse_dsc_wdma(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dsc wdma\n");
		return ret;
	}

	ret = dpu_parse_dsc_rdma(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dsc rdma\n");
		return ret;
	}

	ret = dpu_parse_wb_top(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse wb top\n");
		return ret;
	}

	ret = dpu_parse_wb_core(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse wb core\n");
		return ret;
	}

	ret = dpu_parse_wb_scaler(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse wb scaler\n");
		return ret;
	}

	ret = dpu_parse_dpu_glb(np, dpu_cap);
	if (ret) {
		DPU_ERROR("failed to parse dpu glb\n");
		return ret;
	}

	return 0;
}
