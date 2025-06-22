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

#ifndef _DPU_COLOR_INTERNAL_H_
#define _DPU_COLOR_INTERNAL_H_

#include <linux/list.h>
#include <display/xring_dpu_color.h>
#include "dpu_hw_color.h"

/* post pq hw modules */
enum postpq_modules {
	POSTPQ_PRE_PROC,
	POSTPQ_ACAD,
	POSTPQ_TOP,
	POSTPQ_DPP_3DLUT,
	POSTPQ_DPP_GAMMA,
	POSTPQ_HIST,
	POSTPQ_SCALER,
	POSTPQ_RC,
	POSTPQ_MODULE_MAX,
};

/**
 * postpq_property_node - the property node data structure
 * @feature: the color feature in post pq
 * @prop_id: property id
 * @prop_flags: property flags
 * @prop_val: property value
 * @blob_ptr: point to blob property
 * @blob_size: blob size
 * @feature_head: list head of features
 * @dirty_head: dirty list head of features
 * @history_head: store list head of features
 * @is_stored: record feature whether stored
 */
struct postpq_property_node {
	u32 feature;
	u32 prop_id;
	u32 prop_flags;
	uint64_t prop_val;
	void *blob_ptr;
	u32 blob_size;

	struct list_head feature_head;
	struct list_head dirty_head;
	struct list_head history_head;
	bool is_stored;
};

/**
 * color_hist_event - the histogram event data structure
 * @enable: enable or disable hist statistics
 * @hist_read_mode: 0 for reading hist data on each eof, 1 for reading once
 * @read_cnt: used in read mode 1
 * @hist_blob: hist data blob
 */
struct color_hist_event {
	bool enable;
	u32 hist_read_mode;
	atomic_t read_cnt;
	struct drm_property_blob *hist_blob;
};

#endif /* _DPU_COLOR_INTERNAL_H_ */
