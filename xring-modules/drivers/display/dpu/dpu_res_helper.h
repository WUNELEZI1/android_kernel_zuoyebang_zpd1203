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

#include "dpu_cap.h"
#include "dpu_crtc.h"

struct dpu_kms;

/**
 * dpu_connector_info - the dpu connector resource info structure
 * @name: the connector type id
 * @blk_id: the resource block index
 */
struct dpu_connector_info {
	int type;
	int blk_id;
};

/**
 * dpu_res_reservation_table - dpu resource reservation table structure
 * @type: the crtc type value
 * @connectors: the available connector info
 * @clone_wbs: the available clone wb info
 */
struct dpu_res_reservation_table {
	enum dpu_crtc_type crtc_type;
	struct dpu_connector_info connectors[MAX_CRTC_COUNT];
	struct dpu_connector_info clone_wbs[MAX_CLONE_WB_COUNT];
};

/**
 * dpu_res_helper_resource_reserve - reserve dpu resources
 * @dpu_kms: the dpu kms pointer
 * @state: the atomic state pointer
 * @test_only: test only flag
 *
 * Return: zero on success, -error number on failure
 */
int dpu_res_helper_resource_reserve(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state, bool test_only);

/**
 * dpu_res_helper_update_crtc_dsc_cfg - update dsc cfg
 * @state: the atomic state pointer
 * @crtc: the drm crtc pointer
 * @new_state: the crtc new state pointer
 */
void dpu_res_helper_update_crtc_dsc_cfg(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *new_state);

/**
 * dpu_res_helper_resource_release - release dpu resources
 * @dpu_kms: the dpu kms pointer
 * @state: the atomic state pointer
 * @finish: finish flag
 */
void dpu_res_helper_resource_release(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state, bool finish);

