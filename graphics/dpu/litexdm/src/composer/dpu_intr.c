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

#include "dpu_hw_common.h"
#include "dpu_format.h"
#include "dpu_hw_cap.h"
#include "dpu_common_info.h"
#include "dpu_res_mgr.h"
#include "dpu_comp_mgr.h"
#include "dpu_log.h"

static inline enum dpu_intr_type dpu_interrupt_id_to_type(u32 intr_id)
{
	if (intr_id < INTR_ONLINE0_MAX)
		return DPU_INTR_ONLINE0;
	else if ((intr_id > INTR_ONLINE0_MAX) && (intr_id < INTR_COMBO_MAX))
		return DPU_INTR_ONLINE1;
	else if ((intr_id > INTR_COMBO_MAX) && (intr_id < INTR_OFFLINE0_MAX))
		return DPU_INTR_OFFLINE0;
	else
		return DPU_INTR_TYPE_MAX;
}

static int dpu_interrupt_register(struct composer *comp, u32 intr_id)
{
	enum dpu_intr_type intr_type;
	struct dpu_hw_intr *hw_intr;

	DPU_DEBUG("intr_id = %d\n", intr_id);

	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("failed to get interrupt type\n");
		return -1;
	}

	hw_intr = comp->hw_intr;
	dpu_check_and_return(!hw_intr, -1, "hw_intr is null\n");

	hw_intr->ops.status_clear(&hw_intr->hw, intr_id);
	hw_intr->ops.enable(&hw_intr->hw, intr_id, true);

	return 0;
}

static void dpu_interrupt_unregister(struct composer *comp, u32 intr_id)
{
	enum dpu_intr_type intr_type;
	struct dpu_hw_intr *hw_intr;

	DPU_DEBUG("intr_id = %d\n", intr_id);
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("failed to get interrupt type\n");
		return;
	}

	hw_intr = comp->hw_intr;
	dpu_check_and_no_retval(!hw_intr, "hw_intr is null\n");

	hw_intr->ops.enable(&hw_intr->hw, intr_id, false);
}

static int dsi_interrupt_register(struct composer *comp, u32 intr_type)
{
	struct dpu_panel_info *pinfo;
	struct dsi_intr_state *intr_state;
	bool is_online0;
	int ret;

	pinfo = comp->pinfo;
	dpu_check_and_return(!pinfo, -1, "pinfo is null\n");

	intr_state = dpu_mem_alloc(sizeof(struct dsi_intr_state));
	dpu_check_and_return(!intr_state, -1, "dsi intr_state is null\n");
	pinfo->dsi_intr_ids = intr_state;

	is_online0 = !!(intr_type == DPU_INTR_ONLINE0);
	intr_state->eof_id = is_online0 ? INTR_ONLINE0_FRM_TIMING_EOF :
			INTR_COMBO_FRM_TIMING_EOF;
	intr_state->vsync_id = is_online0 ? INTR_ONLINE0_FRM_TIMING_VSYNC :
			INTR_COMBO_FRM_TIMING_VSYNC;
	intr_state->vstart_id = is_online0 ? INTR_ONLINE0_TMG_VACTIVE_START :
			INTR_COMBO_TMG_VACTIVE_START;
	intr_state->underflow_id = is_online0 ? INTR_ONLINE0_FRM_TIMING_UNDERFLOW :
			INTR_COMBO_FRM_TIMING_UNDERFLOW;
	intr_state->cfg_rdy_clr_id = is_online0 ? INTR_ONLINE0_CFG_RDY :
			INTR_COMBO_CFG_RDY;

	ret = dpu_interrupt_register(comp, intr_state->cfg_rdy_clr_id);
	if (ret) {
		DPU_ERROR("failed to register cfg_rdy_clr, ret %d\n", ret);
		goto err_cfg_rdy_clr;
	}

	ret = dpu_interrupt_register(comp, intr_state->eof_id);
	if (ret) {
		DPU_ERROR("failed to register eof, ret %d\n", ret);
		goto err_eof;
	}

	ret = dpu_interrupt_register(comp, intr_state->vsync_id);
	if (ret) {
		DPU_ERROR("failed to register vsync, ret %d\n", ret);
		goto err_vsync;
	}

	ret = dpu_interrupt_register(comp, intr_state->vstart_id);
	if (ret) {
		DPU_ERROR("failed to register vactive start, ret %d\n", ret);
		goto err_vact_start;
	}

	ret = dpu_interrupt_register(comp, intr_state->underflow_id);
	if (ret) {
		DPU_ERROR("failed to register timing underflow, ret %d\n", ret);
		goto err_frm_timing_underflow;
	}

	return 0;

err_frm_timing_underflow:
	dpu_interrupt_unregister(comp, intr_state->vstart_id);
err_vact_start:
	dpu_interrupt_unregister(comp, intr_state->vsync_id);
err_vsync:
	dpu_interrupt_unregister(comp, intr_state->eof_id);
err_eof:
	dpu_interrupt_unregister(comp, intr_state->cfg_rdy_clr_id);
err_cfg_rdy_clr:
	return ret;
}

static void dsi_interrupt_unregister(struct composer *comp, u32 intr_type)
{
	struct dpu_panel_info *pinfo;
	struct dsi_intr_state *intr_state;

	pinfo = comp->pinfo;
	dpu_check_and_no_retval(!pinfo, "pinfo is null\n");

	intr_state = pinfo->dsi_intr_ids;
	dpu_check_and_no_retval(!intr_state, "intr_state is null\n");

	dpu_interrupt_unregister(comp, intr_state->underflow_id);
	dpu_interrupt_unregister(comp, intr_state->vstart_id);
	dpu_interrupt_unregister(comp, intr_state->vsync_id);
	dpu_interrupt_unregister(comp, intr_state->eof_id);
	dpu_interrupt_unregister(comp, intr_state->cfg_rdy_clr_id);

	dpu_mem_free(intr_state);
}

static int dpu_intr_type_get(struct composer *comp, u32 *type_out)
{
	enum dpu_intr_type intr_type;

	if (!comp->hw_mixer) {
		DPU_ERROR("hw_mixer is null\n");
		goto error_get;
	}

	/* refer to mixer features defined in dtsi */
	switch (comp->hw_mixer->hw.blk_id) {
	case MIXER_0:
		intr_type = DPU_INTR_ONLINE0;
		break;
	case MIXER_1:
		intr_type = DPU_INTR_ONLINE1;
		break;
	case MIXER_2:
		intr_type = DPU_INTR_OFFLINE0;
		break;
	default:
		DPU_ERROR("incorrect hw_mixer blk_id %d\n", comp->hw_mixer->hw.blk_id);
		goto error_get;
	}

	*type_out = intr_type;
	return 0;

error_get:
	*type_out = DPU_INTR_TYPE_MAX;
	return -1;
}

static int dpu_wb_interrupt_id_get(struct composer *comp, u32 type,
		enum dpu_intr_id *wb_intr_id_out)
{
	bool is_wb0;

	if (!comp->hw_wb) {
		DPU_ERROR("hw_wb is null\n");
		goto error_get;
	}

	is_wb0 = (comp->hw_wb->hw.blk_id == WB0) ? true : false;

	switch (type) {
	case DPU_INTR_ONLINE0:
		DPU_ERROR("litexdm not support dpu interrupt type %d\n", type);
		break;
	case DPU_INTR_ONLINE1:
		*wb_intr_id_out = (is_wb0) ? INTR_COMBO_WB0_DONE : INTR_COMBO_WB1_DONE;
		break;
	case DPU_INTR_OFFLINE0:
		*wb_intr_id_out = (is_wb0) ? INTR_OFFLINE0_WB0_DONE : INTR_OFFLINE0_WB1_DONE;
		break;
	default:
		DPU_ERROR("incorrect dpu interrupt type %d\n", type);
		goto error_get;
	}

	return 0;

error_get:
	*wb_intr_id_out = INTR_OFFLINE0_MAX;
	return -1;
}

static int dpu_wb_interrupt_register(struct composer *comp, u32 type)
{
	struct dpu_panel_info *pinfo;
	struct wb_intr_state *intr_state;
	enum dpu_intr_id wb_intr_id;
	int ret;

	pinfo = comp->pinfo;
	dpu_check_and_return(!pinfo, -1, "pinfo is null\n");

	intr_state = dpu_mem_alloc(sizeof(struct wb_intr_state));
	dpu_check_and_return(!intr_state, -1, "wb intr_state is null\n");
	pinfo->wb_intr_ids = intr_state;

	ret = dpu_wb_interrupt_id_get(comp, type, &intr_state->wb_intr_id);
	if (ret) {
		DPU_ERROR("failed to get wb interrupt id\n");
		return -1;
	}

	ret = dpu_interrupt_register(comp, intr_state->wb_intr_id);
	if (ret) {
		DPU_ERROR("failed to register wb-done-%d interrupt, ret %d\n",
				wb_intr_id, ret);
		goto err_wb_done;
	}

	return 0;

err_wb_done:
	return ret;
}

static void dpu_wb_interrupt_unregister(struct composer *comp, u32 type)
{
	struct dpu_panel_info *pinfo;
	struct wb_intr_state *intr_state;

	pinfo = comp->pinfo;
	dpu_check_and_no_retval(!pinfo, "pinfo is null\n");
	dpu_check_and_no_retval(!pinfo->wb_intr_ids,
			"wb pinfo->wb_intr_ids is null\n");

	intr_state = pinfo->wb_intr_ids;

	dpu_interrupt_unregister(comp, intr_state->wb_intr_id);
	dpu_mem_free(intr_state);
}

void dpu_irq_init(struct composer *comp)
{
	u32 intr_type;

	DPU_DEBUG("+\n");
	dpu_check_and_no_retval(!comp, "comp is null\n");

	dpu_intr_type_get(comp, &intr_type);
	if (is_dsi_connector(comp->scene_id)) {
		if (intr_type > DPU_INTR_ONLINE1) {
			DPU_ERROR("invalid interrupt type(%d) for dsi\n",
					intr_type);
			return;
		}
		dsi_interrupt_register(comp, intr_type);
	} else {
		if (intr_type > DPU_INTR_OFFLINE0) {
			DPU_ERROR("invalid interrupt type(%d) for wb\n",
					intr_type);
			return;
		}
		dpu_wb_interrupt_register(comp, intr_type);
	}

	DPU_DEBUG("-\n");
}

void dpu_irq_deinit(struct composer *comp)
{
	u32 intr_type;

	DPU_DEBUG("+\n");
	dpu_check_and_no_retval(!comp, "comp is null\n");

	dpu_intr_type_get(comp, &intr_type);
	if (is_dsi_connector(comp->scene_id)) {
		if (intr_type > DPU_INTR_ONLINE1) {
			DPU_ERROR("invalid interrupt type(%d) for dsi\n",
					intr_type);
			return;
		}
		dsi_interrupt_unregister(comp, intr_type);
	} else {
		if (intr_type > DPU_INTR_OFFLINE0) {
			DPU_ERROR("invalid interrupt type(%d) for wb\n",
					intr_type);
			return;
		}
		dpu_wb_interrupt_unregister(comp, intr_type);
	}

	DPU_DEBUG("-\n");
}
