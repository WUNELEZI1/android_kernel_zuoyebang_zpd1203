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

#include <stdio.h>
#include <Protocol/FdtProtocol.h>
#include <libfdt.h>

#include "dpu_hw_res_parser.h"
#include "dpu_log.h"

#define TEST_DTS_PARSE 2

/* max property counts per node */
#define MAX_PROP_ARRAY_SIZE 10

/* the dpu supported device tree node type */
enum prop_type {
	PROP_TYPE_BOOL,
	PROP_TYPE_U32,
	PROP_TYPE_U32_ARRAY,
	PROP_TYPE_STRING,
	PROP_TYPE_STRING_ARRAY,
};

/**
 * prop_item - property of one device tree node
 * @id: the array index of this property item
 * @name: property name of device tree property
 * @is_mandatory: the property is mandatory or optional
 * @type: property type
 */
struct prop_item {
	u32 id;
	char *name;
	bool is_mandatory;
	enum prop_type type;
};

/**
 * prop_data - to store the parsed property data
 * @counts: the property data counts
 * @value: the property value for integer type
 * @string: the property value for string type
 */
struct prop_data {
	u32 counts;
	u32 value[MAX_PROP_ARRAY_SIZE];
	const char *string[MAX_PROP_ARRAY_SIZE];
};


enum {
	DPU_TOP_OFFSET,
	DPU_TOP_LEN,
	DPU_TOP_PROP_MAX
};

/* the device tree node group definition of dpu top block */
static struct prop_item dpu_top_props[] = {
	{DPU_TOP_OFFSET, "dpu-top-offset", true, PROP_TYPE_U32_ARRAY},
	{DPU_TOP_LEN, "dpu-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	CTL_TOP_OFFSET,
	CTL_TOP_LEN,
	CTL_TOP_PROP_MAX
};

/* the device tree node group definition of ctl top block */
static struct prop_item ctl_top_props[] = {
	{CTL_TOP_OFFSET, "ctl-top-offset", true, PROP_TYPE_U32_ARRAY},
	{CTL_TOP_LEN, "ctl-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	SCENE_CTL_OFFSET,
	SCENE_CTL_LEN,
	SCENE_CTL_PROP_MAX
};

/* the device tree node group definition of scene ctl block */
static struct prop_item scene_ctl_props[] = {
	{SCENE_CTL_OFFSET, "scene-ctl-offset", true, PROP_TYPE_U32_ARRAY},
	{SCENE_CTL_LEN, "scene-ctl-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	RCH_TOP_OFFSET,
	RCH_TOP_LEN,
	RCH_TOP_PROP_MAX
};

/* the device tree node group definition of r-channel top block */
static struct prop_item rch_top_props[] = {
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
static struct prop_item rch_props[] = {
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
static struct prop_item prepipe_top_props[] = {
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
static struct prop_item pre_scaler_props[] = {
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
static struct prop_item tm_props[] = {
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
static struct prop_item pre_lut3d_props[] = {
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
static struct prop_item pre_hist_props[] = {
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
static struct prop_item mixer_props[] = {
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

static struct prop_item post_pipe_top_props[] = {
	{POST_PIPE_TOP_OFFSET, "post-pipe-top-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PIPE_TOP_LEN, "post-pipe-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_PQ_TOP_OFFSET,
	POST_PQ_TOP_LEN,
	POST_PQ_TOP_PROP_MAX
};

static struct prop_item post_pq_top_props[] = {
	{POST_PQ_TOP_OFFSET, "post-pq-top-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PQ_TOP_LEN, "post-pq-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_PRE_PROC_OFFSET,
	POST_PRE_PROC_LEN,
	POST_PRE_PROC_PROP_MAX
};

static struct prop_item post_pre_proc_props[] = {
	{POST_PRE_PROC_OFFSET, "post-pre-proc-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_PRE_PROC_LEN, "post-pre-proc-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	ACAD_OFFSET,
	ACAD_LEN,
	ACAD_PROP_MAX
};

static struct prop_item acad_props[] = {
	{ACAD_OFFSET, "acad-offset", true, PROP_TYPE_U32_ARRAY},
	{ACAD_LEN, "acad-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_3DLUT_OFFSET,
	POST_3DLUT_LEN,
	POST_3DLUT_PROP_MAX
};

static struct prop_item post_3dlut_props[] = {
	{POST_3DLUT_OFFSET, "post-lut-3d-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_3DLUT_LEN, "post-lut-3d-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_GAMMA_OFFSET,
	POST_GAMMA_LEN,
	POST_GAMMA_PROP_MAX
};

static struct prop_item post_gamma_props[] = {
	{POST_GAMMA_OFFSET, "post-gamma-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_GAMMA_LEN, "post-gamma-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_HIST_OFFSET,
	POST_HIST_LEN,
	POST_HIST_PROP_MAX
};

static struct prop_item post_hist_props[] = {
	{POST_HIST_OFFSET, "post-histogram-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_HIST_LEN, "post-histogram-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	POST_SCALER_OFFSET,
	POST_SCALER_LEN,
	POST_SCALER_PROP_MAX
};

static struct prop_item post_scale_props[] = {
	{POST_SCALER_OFFSET, "post-scale-offset", true, PROP_TYPE_U32_ARRAY},
	{POST_SCALER_LEN, "post-scale-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	DSC_OFFSET,
	DSC_LEN,
	DSC_PROP_MAX
};

/* the device tree node group definition of dsc block */
static struct prop_item dsc_props[] = {
	{DSC_OFFSET, "dsc-offset", true, PROP_TYPE_U32_ARRAY},
	{DSC_LEN, "dsc-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	WB_TOP_OFFSET,
	WB_TOP_LEN,
	WB_TOP_PROP_MAX
};

/* the device tree node group definition of wb top block */
static struct prop_item wb_top_props[] = {
	{WB_TOP_OFFSET, "wb-top-offset", true, PROP_TYPE_U32_ARRAY},
	{WB_TOP_LEN, "wb-top-len", true, PROP_TYPE_U32_ARRAY},
};

enum {
	WB_SCALER_OFFSET,
	WB_SCALER_LEN,
	WB_SCALER_PROP_MAX
};

/* the device tree node group definition of wb scaler block */
static struct prop_item wb_scaler_props[] = {
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
static struct prop_item wb_core_props[] = {
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
static struct prop_item dpu_intr_props[] = {
	{DPU_INTR_OFFSET, "intr-offset", true, PROP_TYPE_U32_ARRAY},
	{DPU_INTR_LEN, "intr-len", true, PROP_TYPE_U32_ARRAY},
};

static int _dpu_parse_dt_u32(void *fdt, int32_t offset,
		struct prop_item *prop_item,
		struct prop_data *prop_data)
{
	int32_t prop_len;
	const fdt32_t *val;

	val = fdt_getprop(fdt, offset, prop_item->name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	prop_data->value[0] = fdt32_to_cpu (*val);
	prop_data->counts = 1;

	return 0;
}

static int _dpu_parse_dt_u32_array(void *fdt, int32_t offset,
		struct prop_item *prop_item,
		struct prop_data *prop_data)
{
	int32_t prop_len;
	const fdt32_t *val;
	int i;

	val = fdt_getprop(fdt, offset, prop_item->name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	uint32_t count = prop_len / sizeof(uint32_t);
	for (i = 0; i < count; i++) {
		prop_data->value[i] = fdt32_to_cpu (((UINT32 *)val)[i]);
	}

	prop_data->counts = count;
	return 0;
}

static int _dpu_parse_dt_string(void *fdt, int32_t offset,
		struct prop_item *prop_item,
		struct prop_data *prop_data)
{
	int32_t prop_len;
	const char *str;

	str = fdt_getprop(fdt, offset, prop_item->name, &prop_len);
	if ((str == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	prop_data->string[0] = str;
	prop_data->counts = 1;

	return 0;
}

static int _dpu_parse_dt_string_array(void *fdt, int32_t offset,
		struct prop_item *prop_item,
		struct prop_data *prop_data)
{
	int32_t prop_len;
	const void *prop;
	const char *p;
	int string_count = 0;

	prop = fdt_getprop(fdt, offset, prop_item->name, &prop_len);
	if ((prop == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	p = prop;
	while (prop_len > 0) {
		const char *next_string = memchr(p, '\0', prop_len);
		if (!next_string) {
			dpu_pr_err("Error: Malformed string property '%a'.\n", prop_item->name);
			return -1;
		}

		prop_data->string[string_count] = p;

		string_count++;
		prop_len -= (next_string - p) + 1;
		p = next_string + 1;
	}

	prop_data->counts = string_count;
	return 0;
}

static int32_t dpu_parse_dt_props(void *fdt, int32_t offset,
		struct prop_item *prop_items,
		u32 prop_count,
		struct prop_data *prop_data)
{
	struct prop_item *cur_item;
	struct prop_data *cur_data;
	int i;
	int ret = 0;

	if (prop_count > MAX_PROP_ARRAY_SIZE) {
		dpu_pr_err("reach the maximum prop array size %u/%u\n",
				prop_count, MAX_PROP_ARRAY_SIZE);
		return -1;
	}

	for (i = 0; i < prop_count; i++) {
		cur_item = &prop_items[i];
		cur_data = &prop_data[i];

		switch (cur_item->type) {
		case PROP_TYPE_BOOL:
		case PROP_TYPE_U32:
			ret = _dpu_parse_dt_u32(fdt, offset, cur_item, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_U32_ARRAY:
			ret = _dpu_parse_dt_u32_array(fdt, offset, cur_item, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_STRING:
			ret = _dpu_parse_dt_string(fdt, offset, cur_item, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_STRING_ARRAY:
			ret = _dpu_parse_dt_string_array(fdt, offset, cur_item, cur_data);
			if (ret)
				return ret;
			break;
		default:
			dpu_pr_err("unsupported prop type %d!\n", prop_items[i].type);
		}
	}

	return ret;
}

//TODO:
static int32_t dpu_parse_base_iomem(struct dpu_iomem *dpu_mem)
{
	dpu_mem->base = 0xE7000000;
	dpu_mem->addr = 0xE7000000;
	dpu_mem->len = 0x5A000;
	return 0;
}

static int32_t dpu_parse_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *dpu_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(sizeof(*prop_data) * DPU_TOP_PROP_MAX);
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, dpu_top_props, DPU_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse dpu top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DPU_TOP_OFFSET].counts;
	if (prop_count > DPU_TOP_MAX) {
		dpu_pr_err("wrong dpu top property count %d/%d\n",
				prop_count, DPU_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dpu_top_base = &dpu_cap->dpu_top[i].base;

		snprintf(dpu_top_base->name, MAX_NAME_LEN, "dpu_top_%d", i);
		dpu_top_base->id = DPU_TOP_0 + i;
		dpu_top_base->addr = prop_data[DPU_TOP_OFFSET].value[i];
		dpu_top_base->len = prop_data[DPU_TOP_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				dpu_top_base->name,
				dpu_top_base->id,
				dpu_top_base->addr,
				dpu_top_base->len);
	}

	dpu_cap->dpu_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_ctl_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *ctl_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(CTL_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, ctl_top_props, CTL_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse ctl top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[CTL_TOP_OFFSET].counts;
	if (prop_count > CTL_TOP_MAX) {
		dpu_pr_err("wrong ctl top property count %d/%d\n",
				prop_count, CTL_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		ctl_top_base = &dpu_cap->ctl_top[i].base;

		snprintf(ctl_top_base->name, MAX_NAME_LEN, "ctl_top_%d", i);
		ctl_top_base->id = CTL_TOP_0 + i;
		ctl_top_base->addr = prop_data[CTL_TOP_OFFSET].value[i];
		ctl_top_base->len = prop_data[CTL_TOP_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				ctl_top_base->name,
				ctl_top_base->id,
				ctl_top_base->addr,
				ctl_top_base->len);
	}
	dpu_cap->ctl_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_scene_ctl(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *scene_ctl_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(SCENE_CTL_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, scene_ctl_props,
			SCENE_CTL_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse scene ctl props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[SCENE_CTL_OFFSET].counts;
	if (prop_count > SCENE_CTL_MAX) {
		dpu_pr_err("wrong scene ctl property count %d/%d\n",
				prop_count, SCENE_CTL_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		scene_ctl_base = &dpu_cap->scene_ctl[i].base;

		snprintf(scene_ctl_base->name, MAX_NAME_LEN, "scene_ctl_%d", i);
		scene_ctl_base->id = SCENE_CTL_0 + i;
		scene_ctl_base->addr = prop_data[SCENE_CTL_OFFSET].value[i];
		scene_ctl_base->len = prop_data[SCENE_CTL_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				scene_ctl_base->name,
				scene_ctl_base->id,
				scene_ctl_base->addr,
				scene_ctl_base->len);
	}
	dpu_cap->scene_ctl_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_intr(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *dpu_intr_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(DPU_INTR_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, dpu_intr_props,
			DPU_INTR_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse dpu intr props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DPU_INTR_OFFSET].counts;
	if (prop_count > DPU_INTR_MAX) {
		dpu_pr_err("wrong dpu intr property count %d/%d\n",
				prop_count, DPU_INTR_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dpu_intr_base = &dpu_cap->dpu_intr[i].base;

		snprintf(dpu_intr_base->name, MAX_NAME_LEN, "dpu_intr_%d", i);
		dpu_intr_base->id = DPU_INTR_0 + i;
		dpu_intr_base->addr = prop_data[DPU_INTR_OFFSET].value[i];
		dpu_intr_base->len = prop_data[DPU_INTR_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				dpu_intr_base->name,
				dpu_intr_base->id,
				dpu_intr_base->addr,
				dpu_intr_base->len);
	}

	dpu_cap->dpu_intr_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_rch_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *rch_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(RCH_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, rch_top_props, RCH_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse rch top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[RCH_TOP_OFFSET].counts;
	if (prop_count > RCH_TOP_MAX) {
		dpu_pr_err("wrong rch top property count %d/%d\n",
				prop_count, RCH_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		rch_top_base = &dpu_cap->rch_top[i].base;

		snprintf(rch_top_base->name, MAX_NAME_LEN, "rch_top_%d", i);
		rch_top_base->id = RCH_TOP_0 + i;
		rch_top_base->addr = prop_data[RCH_TOP_OFFSET].value[i];
		rch_top_base->len = prop_data[RCH_TOP_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				rch_top_base->name,
				rch_top_base->id,
				rch_top_base->addr,
				rch_top_base->len);
	}

	dpu_cap->rch_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;

}

static int32_t dpu_parse_rch(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *rch_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(RCH_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL) {
		dpu_pr_err("alloc prop_data failed\n");
		return -1;
	}

	ret = dpu_parse_dt_props(fdt, offset, rch_props, RCH_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse rch props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[RCH_OFFSET].counts;
	if (prop_count > RCH_MAX) {
		dpu_pr_err("wrong rch property count %d/%d\n",	prop_count, RCH_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		if (dpu_str_cmp(prop_data[RCH_TYPE].string[i], "rch_g") &&
			dpu_str_cmp(prop_data[RCH_TYPE].string[i], "rch_v_lite") &&
			dpu_str_cmp(prop_data[RCH_TYPE].string[i], "rch_v_full")) {
			dpu_pr_err("unsupported rch type: %a\n",
					prop_data[RCH_TYPE].string[i]);
			ret = -1;
			goto error;
		}

		rch_base = &dpu_cap->rch[i].base;
		rch_base->id = RCH_G_0 + i;
		snprintf(rch_base->name, MAX_NAME_LEN, "%s_%d",
				prop_data[RCH_TYPE].string[i], i);
		rch_base->addr = prop_data[RCH_OFFSET].value[i];
		rch_base->len = prop_data[RCH_LEN].value[i];

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				rch_base->name, rch_base->id, rch_base->addr, rch_base->len);
		dpu_pr_debug("name: %a, features: 0x%lx\n", rch_base->name);
	}

	dpu_cap->rch_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_prepipe_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(PREPIPE_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, prepipe_top_props, PREPIPE_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse prepipe_top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PREPIPE_TOP_OFFSET].counts;
	if (prop_count > PREPIPE_TOP_MAX) {
		dpu_pr_err("wrong prepipe top property count %d/%d\n",
				prop_count, PREPIPE_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		if (strcmp(prop_data[PREPIPE_TOP_TYPE].string[i], "prepipe_top_lite") &&
			strcmp(prop_data[PREPIPE_TOP_TYPE].string[i], "prepipe_top_full")) {
			dpu_pr_err("unsupported prepipe top type: %a\n",
					prop_data[PREPIPE_TOP_TYPE].string[i]);
			ret = -1;
			goto error;
		}

		base = &dpu_cap->prepipe_top[i].base;
		base->id = RCH_G_0 + i;
		base->addr = prop_data[PREPIPE_TOP_OFFSET].value[i];
		base->len = prop_data[PREPIPE_TOP_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				prop_data[PREPIPE_TOP_TYPE].string[i], i);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->prepipe_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_pre_scaler(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(PRE_SCALER_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, pre_scaler_props, PRE_SCALER_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse pre scaler props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_SCALER_OFFSET].counts;
	if (prop_count > PRE_SCALER_MAX) {
		dpu_pr_err("wrong pre scaler property count %d/%d\n",
				prop_count, PRE_SCALER_MAX);
		ret = -1;
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
			dpu_pr_err("unsupported pre scaler belongs to %a\n",
					prop_data[PRE_SCALER_BOUND_RCH].string[i]);
			ret = -1;
			goto error;
		}
		base->addr = prop_data[PRE_SCALER_OFFSET].value[i];
		base->len = prop_data[PRE_SCALER_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_scaler",
				prop_data[PRE_SCALER_BOUND_RCH].string[i]);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->pre_scaler_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_tm(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(TM_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, tm_props, TM_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse tm props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[TM_OFFSET].counts;
	if (prop_count > TM_MAX) {
		dpu_pr_err("wrong tm property count %d/%d\n",
				prop_count, TM_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->tm[i].base;
		if (strcmp(prop_data[TM_TYPE].string[i], "tm") &&
			strcmp(prop_data[TM_TYPE].string[i], "tm_lite")) {
			dpu_pr_err("unsupported tm type: %a\n",
					prop_data[TM_TYPE].string[i]);
			ret = -1;
			goto error;
		}
		base->id = RCH_G_0 + i;
		base->addr = prop_data[TM_OFFSET].value[i];
		base->len = prop_data[TM_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				prop_data[TM_TYPE].string[i], i);
		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->tm_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_pre_lut3d(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(PRE_LUT3D_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, pre_lut3d_props, PRE_LUT3D_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse pre lut3d props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_LUT3D_OFFSET].counts;
	if (prop_count > PRE_LUT3D_MAX) {
		dpu_pr_err("wrong pre lut3d property count %d/%d\n",
				prop_count, PRE_LUT3D_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->pre_lut3d[i].base;
		if (!strcmp(prop_data[PRE_LUT3D_BOUND_RCH].string[i], "rch_v_0")) {
			base->id = RCH_V_0;
		} else {
			dpu_pr_err("unsupported pre lut3d belongs to %a\n",
					prop_data[PRE_LUT3D_BOUND_RCH].string[i]);
			ret = -1;
			goto error;
		}
		base->addr = prop_data[PRE_LUT3D_OFFSET].value[i];
		base->len = prop_data[PRE_LUT3D_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s-lut3d",
				prop_data[PRE_LUT3D_BOUND_RCH].string[i]);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}

	dpu_cap->pre_lut3d_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;

}

static int dpu_parse_pre_hist(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(PRE_HIST_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, pre_hist_props, PRE_HIST_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse pre hist props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[PRE_HIST_OFFSET].counts;
	if (prop_count > PRE_HIST_MAX) {
		dpu_pr_err("wrong pre hist property count %d/%d\n",
				prop_count, PRE_HIST_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->pre_hist[i].base;
		if (!strcmp(prop_data[PRE_HIST_BOUND_RCH].string[i], "rch_v_0")) {
			base->id = RCH_V_0;
		} else {
			dpu_pr_err("unsupported pre hist type: %a\n",
					prop_data[PRE_HIST_BOUND_RCH].string[0]);
			ret = -1;
			goto error;
		}

		base->addr = prop_data[PRE_HIST_OFFSET].value[i];
		base->len = prop_data[PRE_HIST_LEN].value[i];
		snprintf(base->name, MAX_NAME_LEN, "%s-0",
				prop_data[PRE_HIST_BOUND_RCH].string[i]);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x, features: 0x%lx\n",
				base->name, base->id, base->addr, base->len, base->features);
	}
	dpu_cap->pre_hist_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;

}

static int32_t dpu_parse_mixer(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *mixer_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(MIXER_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, mixer_props, MIXER_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse mixer props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[MIXER_OFFSET].counts;
	if (prop_count > MIXER_MAX) {
		dpu_pr_err("wrong mixer property count %d/%d\n",
				prop_count, MIXER_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		mixer_base = &dpu_cap->mixer[i].base;

		mixer_base->id = MIXER_0 + i;
		mixer_base->addr = prop_data[MIXER_OFFSET].value[i];
		mixer_base->len = prop_data[MIXER_LEN].value[i];
		snprintf(mixer_base->name, MAX_NAME_LEN, "%s_%d",
					"mixer", mixer_base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				mixer_base->name, mixer_base->id,
				mixer_base->addr, mixer_base->len);
	}

	dpu_cap->mixer_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_post_pipe_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_PIPE_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_pipe_top_props,
			POST_PIPE_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_pipe_top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PIPE_TOP_OFFSET].counts;
	if (prop_count > POST_PIPE_TOP_MAX) {
		dpu_pr_err("wrong post_pipe_top property count %d/%d\n",
				prop_count, POST_PIPE_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pipe_top[i].base;

		base->addr = prop_data[POST_PIPE_TOP_OFFSET].value[i];
		base->len = prop_data[POST_PIPE_TOP_LEN].value[i];
		base->id = POST_PIPE_TOP_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pipe_top", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pipe_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_post_pq_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_PQ_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_pq_top_props,
			POST_PQ_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_pq_top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PQ_TOP_OFFSET].counts;
	if (prop_count > POST_PQ_TOP_MAX) {
		dpu_pr_err("wrong post_pq_top property count %d/%d\n",
				prop_count, POST_PQ_TOP_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pq_top[i].base;

		base->addr = prop_data[POST_PQ_TOP_OFFSET].value[i];
		base->len = prop_data[POST_PQ_TOP_LEN].value[i];
		base->id = POST_PQ_TOP_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pq_top", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pq_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_post_pre_proc(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_PRE_PROC_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_pre_proc_props,
			POST_PRE_PROC_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_pre_proc props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_PRE_PROC_OFFSET].counts;
	if (prop_count > POST_PRE_PROC_MAX) {
		dpu_pr_err("wrong post_pre_proc property count %d/%d\n",
				prop_count, POST_PRE_PROC_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_pre_proc[i].base;

		base->addr = prop_data[POST_PRE_PROC_OFFSET].value[i];
		base->len = prop_data[POST_PRE_PROC_LEN].value[i];
		base->id = POST_PRE_PROC_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_pre_proc", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_pre_proc_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;

}

static int32_t dpu_parse_acad(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(ACAD_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, acad_props, ACAD_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse acad props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[ACAD_OFFSET].counts;
	if (prop_count > ACAD_MAX) {
		dpu_pr_err("wrong acad property count %d/%d\n",
				prop_count, ACAD_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->acad[i].base;

		base->addr = prop_data[ACAD_OFFSET].value[i];
		base->len = prop_data[ACAD_LEN].value[i];
		base->id = ACAD_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"acad", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->acad_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_post_3dlut(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_3DLUT_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_3dlut_props,
			POST_3DLUT_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_3dlut props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_3DLUT_OFFSET].counts;
	if (prop_count > POST_3DLUT_MAX) {
		dpu_pr_err("wrong post_3dlut property count %d/%d\n",
				prop_count, POST_3DLUT_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_3dlut[i].base;

		base->addr = prop_data[POST_3DLUT_OFFSET].value[i];
		base->len = prop_data[POST_3DLUT_LEN].value[i];
		base->id = POST_3DLUT_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_3dlut", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_3dlut_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_post_gamma(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_GAMMA_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_gamma_props,
			POST_GAMMA_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_gamma props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_GAMMA_OFFSET].counts;
	if (prop_count > POST_GAMMA_MAX) {
		dpu_pr_err("wrong post_gamma property count %d/%d\n",
				prop_count, POST_GAMMA_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_gamma[i].base;

		base->addr = prop_data[POST_GAMMA_OFFSET].value[i];
		base->len = prop_data[POST_GAMMA_LEN].value[i];
		base->id = POST_GAMMA_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_gamma", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_gamma_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;

}

static int32_t dpu_parse_post_hist(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_HIST_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_hist_props,
			POST_HIST_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_hist props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_HIST_OFFSET].counts;
	if (prop_count > POST_HIST_MAX) {
		dpu_pr_err("wrong post_hist property count %d/%d\n",
				prop_count, POST_HIST_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_hist[i].base;

		base->addr = prop_data[POST_HIST_OFFSET].value[i];
		base->len = prop_data[POST_HIST_LEN].value[i];
		base->id = POST_HIST_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_hist", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_hist_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_post_scaler(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(POST_SCALER_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, post_scale_props,
			POST_SCALER_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse post_scale props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[POST_SCALER_OFFSET].counts;
	if (prop_count > POST_SCALER_MAX) {
		dpu_pr_err("wrong post_scale property count %d/%d\n",
				prop_count, POST_SCALER_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		base = &dpu_cap->post_scaler[i].base;

		base->addr = prop_data[POST_SCALER_OFFSET].value[i];
		base->len = prop_data[POST_SCALER_LEN].value[i];
		base->id = POST_SCALER_0 + i;
		snprintf(base->name, MAX_NAME_LEN, "%s_%d",
				"post_scaler", base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				base->name, base->id, base->addr, base->len);
	}

	dpu_cap->post_scaler_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_postpq(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	int ret;

	ret = dpu_parse_post_pipe_top(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post_pipe_top\n");
		return ret;
	}

	ret = dpu_parse_post_pq_top(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post_pq_top\n");
		return ret;
	}

	ret = dpu_parse_post_pre_proc(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post_pre_proc\n");
		return ret;
	}

	ret = dpu_parse_acad(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse acad\n");
		return ret;
	}

	ret = dpu_parse_post_3dlut(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post 3dlut\n");
		return ret;
	}

	ret = dpu_parse_post_gamma(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post gamma\n");
		return ret;
	}

	ret = dpu_parse_post_hist(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post hist\n");
		return ret;
	}

	ret = dpu_parse_post_scaler(dpu_cap, fdt, offset);
	if (ret) {
		dpu_pr_err("failed to parse post scaler\n");
		return ret;
	}

	return ret;
}

static int dpu_parse_dsc(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *dsc_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(DSC_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, dsc_props, DSC_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse dsc props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[DSC_OFFSET].counts;
	if (prop_count > DSC_MAX) {
		dpu_pr_err("wrong dsc property count %d/%d\n",
				prop_count, DSC_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		dsc_base = &dpu_cap->dsc[i].base;

		dsc_base->addr = prop_data[DSC_OFFSET].value[i];
		dsc_base->len = prop_data[DSC_LEN].value[i];
		dsc_base->id = DSC_0 + i;
		snprintf(dsc_base->name, MAX_NAME_LEN, "%s_%d",
				"dsc", dsc_base->id);

		dpu_pr_debug("name: %a, id: %d, addr: 0x%08x, len: 0x%x\n",
				dsc_base->name, dsc_base->id,
				dsc_base->addr, dsc_base->len);
	}

	dpu_cap->dsc_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_wb_top(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *wb_top_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(WB_TOP_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, wb_top_props, WB_TOP_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse wb top props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_TOP_OFFSET].counts;
	if (prop_count > WB_TOP_MAX) {
		dpu_pr_err("wrong wb top property count %d/%d\n",
				prop_count, WB_TOP_MAX);
		ret = -1;
		goto error;
	}


	for (i = 0; i < prop_count; i++) {
		wb_top_base = &dpu_cap->wb_top[i].base;

		snprintf(wb_top_base->name, MAX_NAME_LEN, "wb_top_%d", i);
		wb_top_base->id = WB_TOP_0 + i;
		wb_top_base->addr = prop_data[WB_TOP_OFFSET].value[i];
		wb_top_base->len = prop_data[WB_TOP_LEN].value[i];

		dpu_pr_debug("name :%a, id: %d, addr: 0x%x, len: 0x%x\n",
				wb_top_base->name,
				wb_top_base->id,
				wb_top_base->addr,
				wb_top_base->len);
	}

	dpu_cap->wb_top_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int32_t dpu_parse_wb_core(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *wb_core_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	u32 offline_id;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(WB_CORE_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, wb_core_props, WB_CORE_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse wb core props %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_CORE_OFFSET].counts;
	if (prop_count > WB_CORE_MAX) {
		dpu_pr_err("wrong wb core property count %d/%d\n",
				prop_count, WB_CORE_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		wb_core_base = &dpu_cap->wb_core[i].base;
		offline_id = prop_data[WB_CORE_OFFLINE_ID].value[i];

		if ((offline_id != WB_CORE_OFFLINE_MIXER_1) &&
			(offline_id != WB_CORE_OFFLINE_MIXER_2)) {
			dpu_pr_err("unsupported offline id for wb core, %u\n",
					offline_id);
			return -1;
		}

		snprintf(wb_core_base->name, MAX_NAME_LEN, "wb_core_%d", i);
		wb_core_base->id = WB_CORE_0 + i;
		wb_core_base->addr = prop_data[WB_CORE_OFFSET].value[i];
		wb_core_base->len = prop_data[WB_CORE_LEN].value[i];

		dpu_pr_debug("name: %a, id: %d, addr: 0x%x, len: 0x%x, feature %lx\n",
				wb_core_base->name,
				wb_core_base->id,
				wb_core_base->addr,
				wb_core_base->len,
				wb_core_base->features);
	}

	dpu_cap->wb_core_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

static int dpu_parse_wb_scaler(struct dpu_cap *dpu_cap, void *fdt, int32_t offset)
{
	struct dpu_hw_base *wb_scaler_base;
	struct prop_data *prop_data;
	u32 prop_count = 0;
	int i;
	int ret;

	prop_data = dpu_mem_alloc(WB_SCALER_PROP_MAX * sizeof(*prop_data));
	if (prop_data == NULL)
		return -1;

	ret = dpu_parse_dt_props(fdt, offset, wb_scaler_props, WB_SCALER_PROP_MAX, prop_data);
	if (ret) {
		dpu_pr_err("failed to parse wb scaler nodes %d\n", ret);
		goto error;
	}

	prop_count = prop_data[WB_SCALER_OFFSET].counts;
	if (prop_count > WB_SCALER_MAX) {
		dpu_pr_err("wrong wb scaler property count %d/%d\n",
				prop_count, WB_SCALER_MAX);
		ret = -1;
		goto error;
	}

	for (i = 0; i < prop_count; i++) {
		wb_scaler_base = &dpu_cap->wb_scaler[i].base;

		snprintf(wb_scaler_base->name, MAX_NAME_LEN, "wb_scaler_%d", i);
		wb_scaler_base->id = WB_SCALER_0 + i;
		wb_scaler_base->addr = prop_data[WB_SCALER_OFFSET].value[i];
		wb_scaler_base->len = prop_data[WB_SCALER_LEN].value[i];

		dpu_pr_debug("name :%s, id: %d, addr: %x, len: %x\n",
				wb_scaler_base->name,
				wb_scaler_base->id,
				wb_scaler_base->addr,
				wb_scaler_base->len);
	}

	dpu_cap->wb_scaler_count = prop_count;

error:
	dpu_mem_free(prop_data);
	return ret;
}

int32_t dpu_cap_init(struct dpu_cap *dpu_cap)
{
	void *fdt;
	int32_t offset;

	fdt = dpu_get_fdt();
	if (!fdt) {
		dpu_pr_err("dpu_get_fdt fail\n");
		goto err;
	}

	offset = dpu_get_fdt_offset(fdt, DPU_DTS_PATH);
	if (offset < 0) {
		dpu_pr_err("get invalid offset\n");
		goto err;
	}

	dpu_parse_base_iomem(&dpu_cap->dpu_mem);
	dpu_parse_top(dpu_cap, fdt, offset);
	dpu_parse_ctl_top(dpu_cap, fdt, offset);
	dpu_parse_scene_ctl(dpu_cap, fdt, offset);
	dpu_parse_intr(dpu_cap, fdt, offset);
	dpu_parse_rch_top(dpu_cap, fdt, offset);
	dpu_parse_rch(dpu_cap, fdt, offset);
	dpu_parse_prepipe_top(dpu_cap, fdt, offset);
	dpu_parse_pre_scaler(dpu_cap, fdt, offset);
	dpu_parse_tm(dpu_cap, fdt, offset);
	dpu_parse_pre_lut3d(dpu_cap, fdt, offset);
	dpu_parse_pre_hist(dpu_cap, fdt, offset);
	dpu_parse_mixer(dpu_cap, fdt, offset);
	dpu_parse_postpq(dpu_cap, fdt, offset);
	dpu_parse_dsc(dpu_cap, fdt, offset);
	dpu_parse_wb_top(dpu_cap, fdt, offset);
	dpu_parse_wb_core(dpu_cap, fdt, offset);
	dpu_parse_wb_scaler(dpu_cap, fdt, offset);

err:
	return 0;
}
