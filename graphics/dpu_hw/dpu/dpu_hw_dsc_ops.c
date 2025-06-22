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

#include "dpu_hw_dsc_reg.h"
#include "dpu_hw_dsc_ops.h"

#define DSC0_CUSTOM_OFFSET                             (0x100)
#define DSC1_CUSTOM_OFFSET                             (0x80)

static const u8 slice_cnt_lut[] = {1, 2, 4, 8, 12, 16, 20, 24};
static const u32 DSC_CUSTOM_OFFSET[] = {DSC0_CUSTOM_OFFSET, DSC1_CUSTOM_OFFSET};

static u8 dpu_hw_dsc_supported_bpcs(u8 max_bpc, u8 bpcs[SRC_DSC_CAP_BPCS_SIZE])
{
	u8 idx = 0;

	switch (max_bpc) {
	case 0x0:
		bpcs[idx++] = 16;
		fallthrough;
	case 0xe:
		bpcs[idx++] = 14;
		fallthrough;
	case 0xc:
		bpcs[idx++] = 12;
		fallthrough;
	case 0xa:
		bpcs[idx++] = 10;
		fallthrough;
	default:
		bpcs[idx++] = 8;
	}

	return idx;
}

void dpu_hw_dsc_get_caps(struct dpu_hw_blk *hw, struct dsc_caps_src *caps_src)
{
	u8 line_buf_depth;
	u8 max_cnt_code;
	u8 bpc;
	u32 reg;

	reg = DPU_REG_READ(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X04_DSC_CFG_REG0);
	caps_src->dsc_version_minor = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_MNV_ABL_SHIFT,
			DSC_0X04_DSC_CFG_REG0_MNV_ABL_LEN);
	caps_src->dsc_version_major = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_MJV_ABL_SHIFT,
			DSC_0X04_DSC_CFG_REG0_MJV_ABL_LEN);
	line_buf_depth = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_LBFD_SHIFT,
			DSC_0X04_DSC_CFG_REG0_LBFD_LEN);
	caps_src->max_line_buffer_depth = (line_buf_depth == 0) ? 16 : line_buf_depth;
	max_cnt_code = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_MNSLC_SHIFT,
			DSC_0X04_DSC_CFG_REG0_MNSLC_LEN);
	caps_src->max_slice_cnt = slice_cnt_lut[max_cnt_code];
	bpc = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_NBPC_SHIFT,
			DSC_0X04_DSC_CFG_REG0_NBPC_LEN);
	caps_src->bpcs_size = dpu_hw_dsc_supported_bpcs(bpc, caps_src->bpcs);
	caps_src->block_predication_supported = GET_BITS_VAL(reg,
			DSC_0X04_DSC_CFG_REG0_BPA_SHIFT,
			DSC_0X04_DSC_CFG_REG0_BPA_LEN);

	reg = DPU_REG_READ(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X08_DSC_CFG_REG1);
	caps_src->max_pic_height = GET_BITS_VAL(reg,
			DSC_0X08_DSC_CFG_REG1_MPCH_SHIFT,
			DSC_0X08_DSC_CFG_REG1_MPCH_LEN);
	caps_src->max_pic_width = GET_BITS_VAL(reg,
			DSC_0X08_DSC_CFG_REG1_MPCW_SHIFT,
			DSC_0X08_DSC_CFG_REG1_MPCW_LEN);

	reg = DPU_REG_READ(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X0C_DSC_CFG_REG2);
	caps_src->rate_buffer_size_bytes = GET_BITS_VAL(reg,
			DSC_0X0C_DSC_CFG_REG2_RBS_SHIFT,
			DSC_0X0C_DSC_CFG_REG2_RBS_LEN);

	reg = DPU_REG_READ(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X10_DSC_CFG_REG3);
	caps_src->max_slice_height = GET_BITS_VAL(reg,
			DSC_0X10_DSC_CFG_REG3_MSLH_SHIFT,
			DSC_0X10_DSC_CFG_REG3_MSLH_LEN);
	caps_src->max_slice_width = GET_BITS_VAL(reg,
			DSC_0X10_DSC_CFG_REG3_MSLW_SHIFT,
			DSC_0X10_DSC_CFG_REG3_MSLW_LEN);

	caps_src->native_422_supported = false;
	caps_src->native_420_supported = false;
	caps_src->var_bit_rate_supported = false;
}

void dpu_hw_dsc_height_set(struct dpu_hw_blk *hw, u16 height, bool dual_port)
{
	struct dpu_hw_blk hw_p1;
	u16 pps_6_7;
	u32 pps_4_7;

	pps_6_7 = ((height >> 8) & 0xff) | (height << 8);
	pps_4_7 = DPU_REG_READ(hw, DSC_0X04_PPS4_7);
	pps_4_7 = MERGE_MASK_BITS(pps_4_7, pps_6_7, DSC_0X04_PPS4_7_PCHH_SHIFT,
			DSC_0X04_PPS4_7_PCHH_MASK | DSC_0X04_PPS4_7_PCHL_MASK);
	DPU_REG_WRITE(hw, DSC_0X04_PPS4_7, pps_4_7, DIRECT_WRITE);

	if (!dual_port)
		return;

	dpu_mem_cpy(&hw_p1, hw, sizeof(struct dpu_hw_blk));
	hw_p1.blk_offset += DSC_0_PORT1_OFFSET;
	pps_4_7 = DPU_REG_READ(&hw_p1, DSC_0X04_PPS4_7);
	pps_4_7 = MERGE_MASK_BITS(pps_4_7, pps_6_7, DSC_0X04_PPS4_7_PCHH_SHIFT,
			DSC_0X04_PPS4_7_PCHH_MASK | DSC_0X04_PPS4_7_PCHL_MASK);
	DPU_REG_WRITE(&hw_p1, DSC_0X04_PPS4_7, pps_4_7, DIRECT_WRITE);
}

/* nslc: number of slices per line */
static u8 dpu_hw_dsc_slice_cnt_to_code(u8 slice_cnt)
{
	u8 idx;

	for (idx = 0; idx < ARRAY_SIZE(slice_cnt_lut); idx++) {
		if (slice_cnt_lut[idx] == slice_cnt)
			return idx;
	}

	return 0;
}

static void dpu_hw_dsc_custom_enable(struct dpu_hw_blk *hw,
		struct dpu_dsc_config *cfg)
{
	u32 value = 0;
	u8 custom_ifep;
	u8 mnr_ver = cfg->dsc_version_minor;
	u8 slice_count = cfg->slice_count;

	if (mnr_ver == DSC_VERSION_MINOR_V2A)
		custom_ifep = 1;
	else
		custom_ifep = 0;

	value = MERGE_BITS(value, custom_ifep,
			DSC_0X20_DSC_CTRL0_IFEP_SHIFT, DSC_0X20_DSC_CTRL0_IFEP_LEN);
	value = MERGE_BITS(value, 1,
			DSC_0X20_DSC_CTRL0_EPL_SHIFT, DSC_0X20_DSC_CTRL0_EPL_LEN);
	value = MERGE_BITS(value, 1,
			DSC_0X20_DSC_CTRL0_FLAL_SHIFT, DSC_0X20_DSC_CTRL0_FLAL_LEN);
	value = MERGE_BITS(value, 1,
			DSC_0X20_DSC_CTRL0_RBIT_SHIFT, DSC_0X20_DSC_CTRL0_RBIT_LEN);
	value = MERGE_BITS(value, 1,
			DSC_0X20_DSC_CTRL0_RBYT_SHIFT, DSC_0X20_DSC_CTRL0_RBYT_LEN);
	value = MERGE_BITS(value, dpu_hw_dsc_slice_cnt_to_code(slice_count),
			DSC_0X20_DSC_CTRL0_NSLC_SHIFT, DSC_0X20_DSC_CTRL0_NSLC_LEN);
	value = MERGE_BITS(value, 1,
			DSC_0X20_DSC_CTRL0_EN_SHIFT, DSC_0X20_DSC_CTRL0_EN_LEN);
	DPU_REG_WRITE(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X20_DSC_CTRL0, value, CMDLIST_WRITE);
}

void dpu_hw_dsc_convert_to_pps(struct dpu_dsc_config *dsc_cfg,
		u8 pps[DSC_PPS_TABLE_BYTES_SIZE])
{
	u32 idx;
	u32 tmp;
	u32 i;

	idx = 0;
	for (i = 0; i < DSC_PPS_TABLE_BYTES_SIZE; i++)
		pps[i] = 0;

	/* PPS 0 */
	pps[idx++] = dsc_cfg->dsc_version_minor |
		dsc_cfg->dsc_version_major << DSC_PPS_VERSION_MAJOR_SHIFT;

	/* PPS 1, 2 is 0 */
	pps[idx++] = 0;
	pps[idx++] = 0;

	/* PPS 3 */
	pps[idx++] = dsc_cfg->line_buf_depth |
		dsc_cfg->bits_per_component << DSC_PPS_BPC_SHIFT;

	/* PPS 4 */
	pps[idx++] = ((dsc_cfg->bits_per_pixel & DSC_PPS_BPP_HIGH_MASK) >>
		 DSC_PPS_MSB_SHIFT) |
		dsc_cfg->vbr_enable << DSC_PPS_VBR_EN_SHIFT |
		dsc_cfg->simple_422 << DSC_PPS_SIMPLE422_SHIFT |
		dsc_cfg->convert_rgb << DSC_PPS_CONVERT_RGB_SHIFT |
		dsc_cfg->block_pred_enable << DSC_PPS_BLOCK_PRED_EN_SHIFT;

	/* PPS 5 */
	pps[idx++] = (dsc_cfg->bits_per_pixel & DSC_PPS_LSB_MASK);

	/*
	 * The DSC panel expects the PPS packet to have big endian format
	 * for data spanning 2 bytes. Use a macro cpu_to_be16() to convert
	 * to big endian format. If format is little endian, it will swap
	 * bytes to convert to Big endian else keep it unchanged.
	 */

	/* PPS 6, 7 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->pic_height, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->pic_height, 0, 8);

	/* PPS 8, 9 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->pic_width, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->pic_width, 0, 8);

	/* PPS 10, 11 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_height, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_height, 0, 8);

	/* PPS 12, 13 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_width, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_width, 0, 8);

	/* PPS 14, 15 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_chunk_size, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_chunk_size, 0, 8);

	/* PPS 16 */
	pps[idx++] = ((dsc_cfg->initial_xmit_delay &
			DSC_PPS_INIT_XMIT_DELAY_HIGH_MASK) >> DSC_PPS_MSB_SHIFT);

	/* PPS 17 */
	pps[idx++] = (dsc_cfg->initial_xmit_delay & DSC_PPS_LSB_MASK);

	/* PPS 18, 19 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->initial_dec_delay, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->initial_dec_delay, 0, 8);

	/* PPS 20 is 0 */
	pps[idx++] = 0;

	/* PPS 21 */
	pps[idx++] = dsc_cfg->initial_scale_value;

	/* PPS 22, 23 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->scale_increment_interval, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->scale_increment_interval, 0, 8);

	/* PPS 24 */
	pps[idx++] = ((dsc_cfg->scale_decrement_interval &
			DSC_PPS_SCALE_DEC_INT_HIGH_MASK) >> DSC_PPS_MSB_SHIFT);

	/* PPS 25 */
	pps[idx++] = (dsc_cfg->scale_decrement_interval & DSC_PPS_LSB_MASK);

	/* PPS 26[7:0], PPS 27[7:5] RESERVED */
	pps[idx++] = 0;

	/* PPS 27 */
	pps[idx++] = dsc_cfg->first_line_bpg_offset;

	/* PPS 28, 29 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->nfl_bpg_offset, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->nfl_bpg_offset, 0, 8);

	/* PPS 30, 31 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_bpg_offset, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->slice_bpg_offset, 0, 8);

	/* PPS 32, 33 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->initial_offset, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->initial_offset, 0, 8);

	/* PPS 34, 35 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->final_offset, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->final_offset, 0, 8);

	/* PPS 36 */
	pps[idx++] = dsc_cfg->flatness_min_qp;

	/* PPS 37 */
	pps[idx++] = dsc_cfg->flatness_max_qp;

	/* PPS 38, 39 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->rc_model_size, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->rc_model_size, 0, 8);

	/* PPS 40 */
	pps[idx++] = DSC_RC_EDGE_FACTOR_CONST;

	/* PPS 41 */
	pps[idx++] = dsc_cfg->rc_quant_incr_limit0;

	/* PPS 42 */
	pps[idx++] = dsc_cfg->rc_quant_incr_limit1;

	/* PPS 43 */
	pps[idx++] = DSC_RC_TGT_OFFSET_LO_CONST |
		DSC_RC_TGT_OFFSET_HI_CONST << DSC_PPS_RC_TGT_OFFSET_HI_SHIFT;

	/* PPS 44 - 57 */
	for (i = 0; i < DSC_NUM_BUF_RANGES - 1; i++)
		pps[idx++] = dsc_cfg->rc_buf_thresh[i];

	/* PPS 58 - 87 */
	/*
	 * For DSC sink programming the RC Range parameter fields
	 * are as follows: Min_qp[15:11], max_qp[10:6], offset[5:0]
	 */
	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		tmp = (dsc_cfg->rc_range_params[i].range_min_qp <<
				DSC_PPS_RC_RANGE_MINQP_SHIFT) |
				(dsc_cfg->rc_range_params[i].range_max_qp <<
				DSC_PPS_RC_RANGE_MAXQP_SHIFT) |
				(dsc_cfg->rc_range_params[i].range_bpg_offset);

		pps[idx++] = GET_BITS_VAL(tmp, 8, 8);
		pps[idx++] = GET_BITS_VAL(tmp, 0, 8);
	}

	/* PPS 88 */
	pps[idx++] = dsc_cfg->native_422 | dsc_cfg->native_420 <<
			DSC_PPS_NATIVE_420_SHIFT;

	/* PPS 89 */
	pps[idx++] = dsc_cfg->second_line_bpg_offset;

	/* PPS 90, 91 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->nsl_bpg_offset, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->nsl_bpg_offset, 0, 8);

	/* PPS 92, 93 */
	pps[idx++] = GET_BITS_VAL(dsc_cfg->second_line_offset_adj, 8, 8);
	pps[idx++] = GET_BITS_VAL(dsc_cfg->second_line_offset_adj, 0, 8);

	/* PPS 94 - 127 are O */
}

static void dpu_hw_dsc_rc_range_parameters_update(struct dpu_dsc_config *dsc_config, u8 *pps)
{
	int i;
	u32 tmp, index;
	u8 range_min_qp, range_max_qp, range_bpg_offset;

	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		tmp = 0;
		index = 58 + (i << 1);
		tmp = MERGE_BITS(tmp, pps[index], 8, 8);
		tmp = MERGE_BITS(tmp, pps[index + 1], 0, 8);
		range_min_qp = GET_BITS_VAL(tmp,
				DSC_PPS_RC_RANGE_MINQP_SHIFT,
				DSC_PPS_RC_RANGE_MINQP_LEN);
		range_max_qp = GET_BITS_VAL(tmp,
				DSC_PPS_RC_RANGE_MAXQP_SHIFT,
				DSC_PPS_RC_RANGE_MAXQP_LEN);
		range_bpg_offset = GET_BITS_VAL(tmp,
				DSC_PPS_RC_RANGE_BPG_OFFSET_SHIFT,
				DSC_PPS_RC_RANGE_BPG_OFFSET_LEN);

		dsc_config->rc_range_params[i].range_min_qp = range_min_qp;
		dsc_config->rc_range_params[i].range_max_qp = range_max_qp;
		dsc_config->rc_range_params[i].range_bpg_offset = range_bpg_offset;
	}
}

static void dpu_hw_dsc_pps_set(struct dpu_hw_blk *hw,
		struct dpu_dsc_config *dsc_config)
{
	u8 pps[DSC_PPS_TABLE_BYTES_SIZE];
	u32 reg;
	u32 i;

	if (dsc_config->customized_pps_table) {
		COMPOSER_DEBUG("get customized dsc pps table\n");
		dpu_mem_cpy(pps, dsc_config->customized_pps_table,
				DSC_PPS_TABLE_BYTES_SIZE);
		dpu_hw_dsc_rc_range_parameters_update(dsc_config, pps);
	} else {
		dpu_hw_dsc_convert_to_pps(dsc_config, pps);
	}

	/* PPS 0~55 */
	for (i = 0; i < 56; i += 4) {
		reg = 0;
		reg = MERGE_BITS(reg, pps[i],     0,  8);
		reg = MERGE_BITS(reg, pps[i + 1], 8,  8);
		reg = MERGE_BITS(reg, pps[i + 2], 16, 8);
		reg = MERGE_BITS(reg, pps[i + 3], 24, 8);
		DPU_REG_WRITE(hw, DSC_0X00_PPS0_3 + (i / 4) * 0x4, reg, CMDLIST_WRITE);
	}

	/* register configuration of is not followed pps table sequence from here */
	/* PPS 56~59 */
	reg = 0;
	reg = MERGE_BITS(reg, pps[i++], 0, 8);
	reg = MERGE_BITS(reg, pps[i++], 8, 8);
	reg = MERGE_BITS(reg, dsc_config->rc_range_params[0].range_bpg_offset,
			DSC_0X38_PPS56_59_RCRP0_RGBPO_SHIFT,
			DSC_0X38_PPS56_59_RCRP0_RGBPO_LEN);
	reg = MERGE_BITS(reg, dsc_config->rc_range_params[0].range_max_qp,
			DSC_0X38_PPS56_59_RCRP0_MXQRG_SHIFT,
			DSC_0X38_PPS56_59_RCRP0_MXQRG_LEN);
	reg = MERGE_BITS(reg, dsc_config->rc_range_params[0].range_min_qp,
			DSC_0X38_PPS56_59_RCRP0_MNQRG_SHIFT,
			DSC_0X38_PPS56_59_RCRP0_MNQRG_LEN);
	DPU_REG_WRITE(hw, DSC_0X38_PPS56_59, reg, CMDLIST_WRITE);

	/* PPS 60~87 */
	for (i = 1; i < DSC_NUM_BUF_RANGES; i += 2) {
		reg = 0;
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i + 1].range_bpg_offset,
				DSC_0X3C_PPS60_63_RCRP2_RGBPO_SHIFT,
				DSC_0X3C_PPS60_63_RCRP2_RGBPO_LEN);
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i + 1].range_max_qp,
				DSC_0X3C_PPS60_63_RCRP2_MXQRG_SHIFT,
				DSC_0X3C_PPS60_63_RCRP2_MXQRG_LEN);
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i + 1].range_min_qp,
				DSC_0X3C_PPS60_63_RCRP2_MNQRG_SHIFT,
				DSC_0X3C_PPS60_63_RCRP2_MNQRG_LEN);
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i].range_bpg_offset,
				DSC_0X3C_PPS60_63_RCRP1_RGBPO_SHIFT,
				DSC_0X3C_PPS60_63_RCRP1_RGBPO_LEN);
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i].range_max_qp,
				DSC_0X3C_PPS60_63_RCRP1_MXQRG_SHIFT,
				DSC_0X3C_PPS60_63_RCRP1_MXQRG_LEN);
		reg = MERGE_BITS(reg, dsc_config->rc_range_params[i].range_min_qp,
				DSC_0X3C_PPS60_63_RCRP1_MNQRG_SHIFT,
				DSC_0X3C_PPS60_63_RCRP1_MNQRG_LEN);
		DPU_REG_WRITE(hw, DSC_0X3C_PPS60_63 + (i / 2) * 0x4, reg, CMDLIST_WRITE);
	}
}

void dpu_hw_dsc_enable(struct dpu_hw_blk *hw, struct dpu_dsc_config *dsc_config)
{
	struct dpu_hw_blk hw_p0;
	struct dpu_hw_blk hw_p1;

	dpu_mem_cpy(&hw_p0, hw, sizeof(struct dpu_hw_blk));
	dpu_mem_cpy(&hw_p1, hw, sizeof(struct dpu_hw_blk));
	hw_p1.blk_offset += DSC_0_PORT1_OFFSET;

	if (dsc_config->dual_port == 1) {
		dpu_hw_dsc_custom_enable(&hw_p0, dsc_config);
		dpu_hw_dsc_pps_set(&hw_p0, dsc_config);
		dpu_hw_dsc_custom_enable(&hw_p1, dsc_config);
		dpu_hw_dsc_pps_set(&hw_p1, dsc_config);
	} else {
		dpu_hw_dsc_custom_enable(hw, dsc_config);
		dpu_hw_dsc_pps_set(hw, dsc_config);
	}
}

void dpu_hw_dsc_disable(struct dpu_hw_blk *hw)
{
	DPU_REG_WRITE(hw, DSC_CUSTOM_OFFSET[hw->blk_id] + DSC_0X20_DSC_CTRL0, 0, DIRECT_WRITE);
}
