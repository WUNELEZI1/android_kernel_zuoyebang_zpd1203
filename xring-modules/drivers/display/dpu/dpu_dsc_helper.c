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

#include "dpu_dsc_helper.h"

#define DSC_PPS_TABLE0                                         (0)
#define DSC_CONVERT_RGB                                        (1)

#define DSC_DISABLE                                            (0)
#define DSC_ENABLE                                             (1)

#define DSC_MUX_WORD_SIZE                                      (48)

#define DSC_BITS_PER_COMPONENT                                 (0x8)

#define DSC_INITIAL_XMIT_DELAY                                 (0x200)
#define DSC_RC_MODE_SIZE                                       (0x2000)
#define DSC_RCTO_LIMIT                                         (0x33)
#define DSC_RCEF_LIMIT                                         (0x6)
#define DSC_INITIAL_OFFSET                                     (6144)

#define DSC_RCRP_MAX                                           (15)

#define BPC_IS_8BIT(bpc)                                       (((bpc) == (0x8)) ? (1) : (0))

struct dsc_parms_private {
	struct dsc_parms *parms;
	u8 line_buffer_depth;
	u8 slice_count;
	bool block_predication_en;
};

int minqp444_8b[] = {0, 0, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 5, 9, 12};
int maxqp444_8b[] = {4, 4, 5, 6, 7, 7, 7, 8, 9, 10, 10, 11, 11, 12, 13};
u8 offset_rgb_und08[] = {2, 0, 0, 62, 60, 58, 56, 56, 56, 54, 54, 52, 52, 52, 52};

int minqp444_10b[] = {0, 4, 5, 5, 7, 7, 7, 7, 7, 8, 9, 9, 9, 12, 16};
int maxqp444_10b[] = {8, 8, 9, 10, 11, 11, 11, 12, 13, 14, 14, 15, 15, 16, 17};

/* Rate Control range parameters rgb */
static void dpu_dsc_rc_range_parameter_rgb(struct dpu_dsc_config *dsc_config)
{
	int *minqp444;
	int *maxqp444;
	int bpc;
	int i;

	bpc = dsc_config->bits_per_component;
	/* Only support BPP(bits_per_pixel) = 8 */
	minqp444 = (BPC_IS_8BIT(bpc)) ? minqp444_8b : minqp444_10b;
	maxqp444 = (BPC_IS_8BIT(bpc)) ? maxqp444_8b : maxqp444_10b;

	for (i = 0; i < DSC_RCRP_MAX; ++i) {
		dsc_config->rc_range_params[i].range_min_qp = minqp444[i];
		dsc_config->rc_range_params[i].range_max_qp = maxqp444[i];
		/**
		 * if (bitsPerPixel <= 8.0)
		 *     rc_range_parameters[i].range_bpg_offset =
		 *     ofs_und6[i] + (int)(0.5 * (bitsPerPixel - 6.0) *
		 *     (ofs_und8[i] - ofs_und6[i]) + 0.5);
		 * bitsPerPixel = 8
		 */
		dsc_config->rc_range_params[i].range_bpg_offset = offset_rgb_und08[i];
	}
}

static u32 dpu_dsc_num_extra_mux_bits(struct dpu_dsc_config *dsc_config)
{
	u32 num_extra_mux_bits;
	int slice_bits;
	u32 bpc;

	bpc = dsc_config->bits_per_component;

	/**
	 * native_422 disable , = 0  -> num_ssps = 3;
	 * if (pps->native_422 == DSC_ENABLE) num_ssps = 4;
	 * else num_ssps = 3;
	 *
	 * mux_word_size = (bpc >= 12) ? 64 : 48;
	 * only support 10-BPC(0xA) and 8-BPC(0x8)
	 * mux_word_size = 64
	 */
	if (dsc_config->convert_rgb)
		/* (numSsps * (muxWordSize + (4 * BPC + 4) - 2)) */
		num_extra_mux_bits = 3 * (DSC_MUX_WORD_SIZE + (4 * bpc) + 2);
	else if (!dsc_config->native_422)
		/* (numSsps * muxWordSize + (4 * BPC + 4) + 2 * (4 * BPC ) - 2) */
		num_extra_mux_bits = 3 * DSC_MUX_WORD_SIZE + (12 * bpc) + 2;
	else
		/* (numSsps * muxWordSize + (4 * BPC + 4) + 3 * (4 * BPC ) - 2) */
		num_extra_mux_bits = 3 * DSC_MUX_WORD_SIZE + (16 * bpc) + 2;

	slice_bits = 8 * dsc_config->slice_chunk_size * dsc_config->slice_height;

	while ((num_extra_mux_bits > 0) &&
			((slice_bits - num_extra_mux_bits) % DSC_MUX_WORD_SIZE))
		num_extra_mux_bits--;

	return num_extra_mux_bits;
}

static u32 dpu_dsc_final_offset(struct dpu_dsc_config *dsc_config,
		u32 num_extra_mux_bits)
{
	u32 final_offset;

	/**
	 * final_offset = rc_model_size -
	 *                ((initial_xmit_delay * BPP + 8) >> 4) +
	 *                num_extra_mux_bits;
	 * initial_xmit_delay = 0x200 (512), BPP = 0x80(128)
	 * ((initial_xmit_delay * BPP + 8) >> 4) = (512 * 128 + 8) >> 4 = 69632
	 * DSC_RC_MODE_SIZE(0x2000) - (4096) = 4096
	 */
	final_offset = (u32)(4096 + num_extra_mux_bits);

	return final_offset;
}

/* sbo: slice bpg offset */
static u32 dpu_dsc_slice_bpg_offset(struct dpu_dsc_config *dsc_config,
		u32 num_extra_mux_bits)
{
	int tmp;
	u32 slice_bpg_offset;

	/* initial_offset = 6144 */
	/* DSC_RC_MODE_SIZE - dsc_config->initial_offset + num_extra_mux_bits */
	tmp = DSC_RC_MODE_SIZE - DSC_INITIAL_OFFSET + num_extra_mux_bits;

	/**
	 * slice_bpg_offset = (int)ceil((double)(1 << 11) *
	 *	(rc_model_size - initial_offset + num_extra_mux_bits) /
	 *	(groups_total));
	 */

	slice_bpg_offset = DIV_ROUND_UP((1 << 11) * tmp,
			(dsc_config->slice_width + 2) / 3 * dsc_config->slice_height);

	return slice_bpg_offset;
}

/* INOL, INOH : initial offset */
static u32 dpu_dsc_initial_offset(struct dpu_dsc_config *dsc_config)
{
	u32 initial_offset;

	/* only support bpp = DSC_BITS_PER_COMPONENT(0x8) */
	/* initial_offset = 6144 - (int)((DSC_BITS_PER_COMPONENT - 8) * 256 + 0.5); */
	initial_offset = 6144;

	return initial_offset;
}

/* isv: initial scale value */
static u32 dpu_dsc_initial_scale_value(struct dpu_dsc_config *dsc_config)
{
	u32 initial_scale_value;
	/* group per line */
	int group_per_line;

	initial_scale_value = 8 * DSC_RC_MODE_SIZE /
			(DSC_RC_MODE_SIZE - dsc_config->initial_offset);
	group_per_line = (dsc_config->slice_width + 2) / 3;

	if (group_per_line < (initial_scale_value - 8))
		initial_scale_value = (u32)(group_per_line + 8);

	return initial_scale_value;
}

/* FBO: first line BPG offset */
static u32 dpu_dsc_first_line_bpg_offset(struct dpu_dsc_config *dsc_config)
{
	u32 first_line_bpg_offset;
	int slice_height;
	int tmp;

	slice_height = dsc_config->slice_height - 8;

	tmp = (slice_height > 34) ? 34 : slice_height;

	if (slice_height >= 8)
		/**
		 * first_line_bpg_offset =
		 * (int)(0.09 * ((slice_height > 34) ? 34 : slice_height));
		 */
		first_line_bpg_offset = 12 + (u32)(90 * tmp / 1000);
	else
		first_line_bpg_offset = (u32)(2 * (slice_height + 8 - 1));

	return first_line_bpg_offset;
}

/* NFBOL, NFBOH: nfl_bpg_offset */
static u32 dpu_dsc_nfl_bpg_offset(struct dpu_dsc_config *dsc_config)
{
	u32 nfl_bpg_offset;
	int tmp;

	/**
	 * nfl_bpg_offset = (int)ceil((double)(first_line_bpg_ofs << 11) /
	 *                  (slice_height - 1));
	 */
	tmp = DIV_ROUND_UP(dsc_config->first_line_bpg_offset << 11,
			dsc_config->slice_height - 1);

	if (dsc_config->slice_height > 1)
		nfl_bpg_offset = tmp;
	else
		nfl_bpg_offset = 0;

	return nfl_bpg_offset;
}

/* second line bpg offset */
static u32 dpu_dsc_nsl_bpg_offset(struct dpu_dsc_config *dsc_config)
{
	u32 nsl_bpg_offset;
	int tmp;

	/**
	 * nsl_bpg_offset = (int)ceil((double)(second_line_bpg_ofs << 11) /
	 *                  (slice_height - 1));
	 */
	tmp = DIV_ROUND_UP(dsc_config->second_line_offset_adj << 11,
			dsc_config->slice_height - 1);

	if (dsc_config->slice_height > 2)
		nsl_bpg_offset = tmp;
	else
		nsl_bpg_offset = 0;

	return nsl_bpg_offset;
}

/* return value is in units of bits */
static u32 dpu_dsc_min_rate_buffer_size(struct dpu_dsc_config *dsc_config)
{
	u32 min_rate_buffer_size;
	u32 group_per_line;

	/* only support s444 */
	group_per_line = DIV_ROUND_UP(dsc_config->slice_width, 3);

	min_rate_buffer_size = dsc_config->rc_model_size - dsc_config->initial_offset
			+ DIV_ROUND_UP(dsc_config->initial_xmit_delay *
			dsc_config->bits_per_pixel, 16)
			+ group_per_line * dsc_config->first_line_bpg_offset;

	return min_rate_buffer_size;
}

/* idd: initial_dec_delay */
static int dpu_dsc_initial_dec_delay(struct dpu_dsc_config *dsc_config)
{
	int min_rate_buffer_size, hrd_delay;
	int initial_dec_delay;

	min_rate_buffer_size = dpu_dsc_min_rate_buffer_size(dsc_config);

	/* hrdDelay = (int)(ceil((double)rbsMin / bitsPerPixel)); */
	hrd_delay = DIV_ROUND_UP(min_rate_buffer_size, DSC_BITS_PER_COMPONENT);
	/* initial_dec_delay = hrdDelay - initial_xmit_delay; */
	initial_dec_delay = hrd_delay - DSC_INITIAL_XMIT_DELAY;

	return initial_dec_delay;
}

/* SDIL, SDIH: scale decrement interval */
static u32 dpu_dsc_scale_decrement_interval(struct dpu_dsc_config *dsc_config)
{
	int scale_decrement_interval;
	/* group per line */
	int group_per_line;

	group_per_line = (dsc_config->slice_width + 2) / 3;

	if (dsc_config->initial_scale_value > 8)
		/**
		 * scale_decrement_interval =
		 * groupsPerLine / (initial_scale_value - 8);
		 */
		scale_decrement_interval = group_per_line /
				(dsc_config->initial_scale_value - 8);
	else
		scale_decrement_interval = 4095;

	return (u32)scale_decrement_interval;
}

/* SIIL, SIIH: scale increment interval */
static u32 dpu_dsc_scale_increment_interval(struct dpu_dsc_config *dsc_config)
{
	int final_scale;
	u32 scale_increment_interval;
	int tmp;

	tmp = dsc_config->nfl_bpg_offset + dsc_config->slice_bpg_offset +
		dsc_config->nsl_bpg_offset;

	final_scale = 8 * (DSC_RC_MODE_SIZE /
		(DSC_RC_MODE_SIZE - dsc_config->final_offset));

	if (final_scale > 9)
		/**
		 * 2048 = 1 << 11
		 * (int)((double)(2048) * dsc_config->final_offset /
		 * ((double)(final_scale - 9) * tmp));
		 */
		scale_increment_interval = (int)(2048 * dsc_config->final_offset /
			((final_scale - 9) * tmp));
	else
		scale_increment_interval = 0;

	return scale_increment_interval;
}

static void dpu_dsc_populate_dsc_config(struct dsc_parms_private *dsc_params_priv,
		struct dpu_dsc_config *dsc_config)
{
	struct dsc_parms *dsc_params = dsc_params_priv->parms;
	u32 num_extra_mux_bits;

	/* PPS3: set bits_per_component, only support 8BPC and 10BPC */
	/* NO.9 PPS37: FMXQ, flatness max QP */
	/* NO.11 PPS42: RCQIL1, rc quantization increment limit 1 */
	/* NO.12 PPS41: RCQIL0, rc quantization increment limit 0 */
	if (dsc_params->bits_per_component == DSC_INPUT_8BPC) {
		dsc_config->bits_per_component = 0x8;
		dsc_config->flatness_max_qp = 12;
		dsc_config->flatness_min_qp = 3;
		dsc_config->rc_quant_incr_limit0 = 11;
		dsc_config->rc_quant_incr_limit1 = 11;
	} else if (dsc_params->bits_per_component == DSC_INPUT_10BPC) {
		dsc_config->bits_per_component = 0xA;
		dsc_config->flatness_max_qp = 16;
		dsc_config->flatness_min_qp = 7;
		dsc_config->rc_quant_incr_limit0 = 15;
		dsc_config->rc_quant_incr_limit1 = 15;
	} else if (dsc_params->bits_per_component == DSC_INPUT_12BPC) {
		dsc_config->bits_per_component = 0xC;
		dsc_config->flatness_max_qp = 20;
		dsc_config->flatness_min_qp = 11;
		dsc_config->rc_quant_incr_limit0 = 19;
		dsc_config->rc_quant_incr_limit1 = 19;
	} else {
		dsc_config->bits_per_component = 0x8;
		dsc_config->flatness_max_qp = 16;
		dsc_config->flatness_min_qp = 3;
		dsc_config->rc_quant_incr_limit0 = 11;
		dsc_config->rc_quant_incr_limit1 = 11;
	}

	/* PPS3: set Line buffer bit depth: 10 + 1 */
	dsc_config->line_buf_depth = dsc_params_priv->line_buffer_depth;

	/* PPS1: PPS Identifier: only support 1 pps table, default 0x0 */
	dsc_config->pps_identifier = DSC_PPS_TABLE0;

	/* PPS0: MJV, 0x1, RO */
	/* PPS0: MNV, Read from Dislay parameter */
	dsc_config->dsc_version_major = dsc_params->dsc_version_major;
	dsc_config->dsc_version_minor = dsc_params->dsc_version_minor;

	// priv_info->dsc.config.dsc_version_major = 0x1;
	//	priv_info->dsc.config.dsc_version_minor = 0x1;

	/* PPS6, PPS7: MNV, Read from Dislay parameter */
	dsc_config->pic_height = dsc_params->pic_height;
	/* PPS8, PPS9: PCWL, PCWH */
	dsc_config->pic_width = dsc_params->pic_width;

	/**
	 * PPS5(BPPL), PPS7(BPPH): bit_per_pixel,
	 * Only support 8bit output, default 0x80
	 */
	dsc_config->bits_per_pixel = dsc_params->bits_per_pixel << 4;

	/* PPS4: BPE(block_pred_enable), not sport*/
	dsc_config->block_pred_enable = dsc_params_priv->block_predication_en;

	/* PPS4: CRGB, color space, only support RGB, default: 0X1 */
	dsc_config->convert_rgb = DSC_CONVERT_RGB;

	/* PPS4: S422 is not support */
	dsc_config->simple_422 = DSC_DISABLE;
	/* PPS4: VBR field RO */
	dsc_config->vbr_enable = DSC_DISABLE;
	/* PPS10, PPS11: SLHL, SLHH, slice heigh, range 63 ~ 255 */
	dsc_config->slice_height = dsc_params->slice_height;
	/* PPS12, PPS13: SLWL, SLWH, slice width */
	dsc_config->slice_width = dsc_params->slice_width;

	dsc_config->slice_count = dsc_params_priv->slice_count;

	/* PPS14, PPS15: CHSL, CHSH, chunck size */
	dsc_config->slice_chunk_size = dsc_params->slice_width * (DSC_BITS_PER_COMPONENT) / 8;

	/**
	 * NO.1: PPS16,PPS17, IDDL, IDDH, initial_xmit_delay
	 * (int)(4096 / (DSC_BITS_PER_COMPONENT) + 0.5)
	 * static value : 512 = 0x200
	 */
	dsc_config->initial_xmit_delay = DSC_INITIAL_XMIT_DELAY;

	/* PPS27: FBO, first line BPG offset  range: 6 ~ 15 */
	dsc_config->first_line_bpg_offset = dpu_dsc_first_line_bpg_offset(dsc_config);

	/* PPS38, PPS39: RCMSL, RCMSH, rc mode size, static value 0x2000 */
	dsc_config->rc_model_size = DSC_RC_MODE_SIZE;

	/* PPS43: RCTOH, RCTOL, rc target offset, range 1 ~ 15 */
	dsc_config->rc_tgt_offset_high = (DSC_RCTO_LIMIT >> 4) & 0xf;
	dsc_config->rc_tgt_offset_low = DSC_RCTO_LIMIT & 0xf;

	/* PPS40: RCEF, rc edge factor */
	dsc_config->rc_edge_factor = DSC_RCEF_LIMIT;
	/* NO. PPS44~PPS57:RCBT0~13, rc buffer threshold */
	dsc_config->rc_buf_thresh[0] = 14;   /* 896 / 64 */
	dsc_config->rc_buf_thresh[1] = 28;   /* 1792 / 64 */
	dsc_config->rc_buf_thresh[2] = 42;   /* 2688 / 64 */
	dsc_config->rc_buf_thresh[3] = 56;   /* 3584 / 64 */
	dsc_config->rc_buf_thresh[4] = 70;   /* 4480 / 64 */
	dsc_config->rc_buf_thresh[5] = 84;   /* 5376 / 64 */
	dsc_config->rc_buf_thresh[6] = 98;   /* 6272 / 64 */
	dsc_config->rc_buf_thresh[7] = 105;  /* 6720 / 64 */
	dsc_config->rc_buf_thresh[8] = 112;  /* 7168 / 64 */
	dsc_config->rc_buf_thresh[9] = 119;  /* 7616 / 64 */
	dsc_config->rc_buf_thresh[10] = 121; /* 7744 / 64 */
	dsc_config->rc_buf_thresh[11] = 123; /* 7872 / 64 */
	dsc_config->rc_buf_thresh[12] = 125; /* 8000 / 64 */
	dsc_config->rc_buf_thresh[13] = 126; /* 8064 / 64 */

	dsc_config->native_420 = DSC_DISABLE;
	dsc_config->native_422 = DSC_DISABLE;

	/* NO.13: PPS89~PPS95, RO
	 * second_line_bpg_offset Value shall be 0 if either of the following
	 * conditions exist:
	 * • dsc_version_minor = 1
	 * • native_420 = 0
	 */
	dsc_config->second_line_bpg_offset = 0;
	if (dsc_config->native_420)
		dsc_config->second_line_offset_adj = 512;
	else
		dsc_config->second_line_offset_adj = 0;

	/* NO.2: PPS58~PPS87: rcrp_rgbpo, rc_range_params[].range_max_qp/range_min_qp */
	/* only support rgb */
	dpu_dsc_rc_range_parameter_rgb(dsc_config);

	/* NO.3 PPS34, PPS35: FNOL, FNOH, final offset */
	num_extra_mux_bits = dpu_dsc_num_extra_mux_bits(dsc_config);
	dsc_config->final_offset = dpu_dsc_final_offset(dsc_config,
			num_extra_mux_bits);

	/* NO.4 PPS30, PPS31: SBOL, SBOH, sclice BPG offset */
	dsc_config->slice_bpg_offset = dpu_dsc_slice_bpg_offset(dsc_config,
			num_extra_mux_bits);

	/* NO.5 PPS32, PPS33: INOL, INOH, initial offset */
	dsc_config->initial_offset = dpu_dsc_initial_offset(dsc_config);

	/* NO.7 PPS28, PPS29: NFBOL, NFBOH, non-first lien bpg offset */
	dsc_config->nfl_bpg_offset = dpu_dsc_nfl_bpg_offset(dsc_config);

	/* NO.8: PPS90, PPS91, non-second line bpg offset RO */
	dsc_config->nsl_bpg_offset = dpu_dsc_nsl_bpg_offset(dsc_config);

	/* NO.13: PPS21, initial scale value */
	dsc_config->initial_scale_value = dpu_dsc_initial_scale_value(dsc_config);

	/* NO.14 PPS18, PPS19: IDDL, IDDH */
	dsc_config->initial_dec_delay = dpu_dsc_initial_dec_delay(dsc_config);

	/* NO.15 PPS24, PP25: SDIL, SDIH, scale decrement interval */
	dsc_config->scale_decrement_interval = dpu_dsc_scale_decrement_interval(dsc_config);

	/* NO.16 PPS22, PP21: SIIL, SIIH, scale increment interval */
	dsc_config->scale_increment_interval = dpu_dsc_scale_increment_interval(dsc_config);

	DPU_DEBUG("pps parameters:\n");
	DPU_DEBUG("dsc_version_minor: %d\n", dsc_config->dsc_version_minor);
	DPU_DEBUG("bits_per_component: %d\n", dsc_config->bits_per_component);
	DPU_DEBUG("pps_identifier: %d\n", dsc_config->pps_identifier);
	DPU_DEBUG("line_buf_depth: %d\n", dsc_config->line_buf_depth);
	DPU_DEBUG("block_pred_enable: %d\n", dsc_config->block_pred_enable);
	DPU_DEBUG("bits_per_pixel: %d\n", dsc_config->bits_per_pixel);
	DPU_DEBUG("pic_height: %d, pic_width:%d\n",
			dsc_config->pic_height, dsc_config->pic_width);
	DPU_DEBUG("slice height:%d, slice width:%d\n",
			dsc_config->slice_height, dsc_config->slice_width);
	DPU_DEBUG("slice_chunk_size: %d\n", dsc_config->slice_chunk_size);
	DPU_DEBUG("initial_xmit_delay: %d\n", dsc_config->initial_xmit_delay);
	DPU_DEBUG("initial_decoding_delay: %d\n", dsc_config->initial_dec_delay);
	DPU_DEBUG("initial_scale_value:%d\n", dsc_config->initial_scale_value);
	DPU_DEBUG("scale_increment_interval: %d\n", dsc_config->scale_increment_interval);
	DPU_DEBUG("scale_decrement_interval: %d\n", dsc_config->scale_decrement_interval);
	DPU_DEBUG("first_line_bpg_offset: %d\n", dsc_config->first_line_bpg_offset);
	DPU_DEBUG("nfl_bpg_offset: %d\n", dsc_config->nfl_bpg_offset);
	DPU_DEBUG("slice_bpg_offset:%d\n", dsc_config->slice_bpg_offset);
	DPU_DEBUG("initial_offset:%d\n", dsc_config->initial_offset);
	DPU_DEBUG("final_offset:%d\n", dsc_config->final_offset);
	DPU_DEBUG("flatness_min_qp %d\n", dsc_config->flatness_min_qp);
	DPU_DEBUG("flatness_max_qp %d\n", dsc_config->flatness_max_qp);
	DPU_DEBUG("convert_rgb:%d, simple_422:%d, vbr_enable:%d\n",
			dsc_config->convert_rgb, dsc_config->simple_422, dsc_config->vbr_enable);
	DPU_DEBUG("nsl_bpg_offset:%d\n", dsc_config->nsl_bpg_offset);
	DPU_DEBUG("rc_model_size:%d\n", dsc_config->rc_model_size);
	DPU_DEBUG("rc_quant_incr_limit0:%d\n", dsc_config->rc_quant_incr_limit0);
	DPU_DEBUG("rc_quant_incr_limit1:%d\n", dsc_config->rc_quant_incr_limit1);
	DPU_DEBUG("rc_edge_factor:%d, second_line_bpg_offset:%d\n",
			dsc_config->rc_edge_factor, dsc_config->second_line_bpg_offset);
	DPU_DEBUG("native_420:%d, native_422:%d\n",
			dsc_config->native_420, dsc_config->native_422);
}

static u8 dpu_dsc_max_slice_cnt(u8 *sink_slice_cnts, u8 sink_slice_cnts_size,
		u8 src_max_slice_cnt)
{
	u8 i;

	for (i = 0; i < sink_slice_cnts_size; i++)
		if (sink_slice_cnts[i] <= src_max_slice_cnt)
			return sink_slice_cnts[i];

	return 1;
}

static bool dpu_dsc_bpc_supported(u8 bits_per_component,
		u8 *sink_bpcs, u8 sink_bpcs_size, u8 *src_bpcs, u8 src_bpcs_size)
{
	u8 i;

	for (i = 0; i < sink_bpcs_size; i++)
		if (bits_per_component == sink_bpcs[i])
			break;
	if (i == sink_bpcs_size)
		return false;

	for (i = 0; i < src_bpcs_size; i++)
		if (bits_per_component == src_bpcs[i])
			break;
	if (i == src_bpcs_size)
		return false;

	return true;
}

static void dpu_dsc_dump_caps(struct dsc_caps_src *src_cap)
{
	u8 i;

	DPU_DEBUG("========== DPU DSC CAPABILITIES ==========");
	DPU_DEBUG("dsc src caps:\n");
	DPU_DEBUG("dsc_version_minor: %d\n", src_cap->dsc_version_minor);
	DPU_DEBUG("dsc_version_major: %d\n", src_cap->dsc_version_major);

	DPU_DEBUG("max_line_buffer_depth: %d\n", src_cap->max_line_buffer_depth);
	DPU_DEBUG("max_pic_height: %d\n", src_cap->max_pic_height);
	DPU_DEBUG("max_pic_width: %d\n", src_cap->max_pic_width);
	DPU_DEBUG("max_slice_height: %d\n", src_cap->max_slice_height);
	DPU_DEBUG("max_slice_width: %d\n", src_cap->max_slice_width);
	DPU_DEBUG("max_slice_cnt: %d\n", src_cap->max_slice_cnt);

	DPU_DEBUG("native_422_supported: %d\n", src_cap->native_422_supported);
	DPU_DEBUG("native_420_supported: %d\n", src_cap->native_420_supported);
	DPU_DEBUG("bpcs_size: %d\n", src_cap->bpcs_size);
	for (i = 0; i < src_cap->bpcs_size; i++)
		DPU_DEBUG("bpcs: %d\n", src_cap->bpcs[i]);
	DPU_DEBUG("var_bit_rate_supported: %d\n", src_cap->var_bit_rate_supported);
	DPU_DEBUG("block_predication_supported: %d\n", src_cap->block_predication_supported);

	DPU_DEBUG("rate_buffer_size_bytes: %d\n", src_cap->rate_buffer_size_bytes);
	DPU_DEBUG("==========================================");
}

static int dpu_dsc_params_correct(struct dsc_parms_private *in_params_priv,
		struct dsc_caps_sink *sink_cap, struct dsc_caps_src *src_cap)
{
	struct dsc_parms *in_params = in_params_priv->parms;
	u8 line_buffer_depth;
	u32 slice_width;
	u8 slice_cnt;
	u8 i;

	dpu_dsc_dump_caps(src_cap);

	if (sink_cap == NULL) {
		/* version major */
		if (in_params->dsc_version_major != DSC_VERSION_MAJOR_V1) {
			DPU_ERROR("unsupported dsc major version %d\n",
					in_params->dsc_version_major);
			return -EINVAL;
		}

		/* version minor */
		if (in_params->dsc_version_minor != DSC_VERSION_MINOR_V1 &&
				in_params->dsc_version_minor != DSC_VERSION_MINOR_V2A) {
			DPU_ERROR("unsupported dsc minor version %d\n",
					in_params->dsc_version_minor);
			return -EINVAL;
		}

		/* picture height */
		if (in_params->pic_height > src_cap->max_pic_height) {
			DPU_ERROR("unsupported pic height %d, max:%d\n",
					in_params->pic_height, src_cap->max_pic_height);
			return -EINVAL;
		}

		/* picture width */
		if (in_params->pic_width > src_cap->max_pic_width) {
			DPU_ERROR("unsupported pic width %d, max:%d\n",
					in_params->pic_width, src_cap->max_pic_width);
			return -EINVAL;
		}

		/* slice height */
		if (in_params->slice_height > src_cap->max_slice_height) {
			DPU_WARN("unsupported slice_height %d, reset to:%d\n",
					in_params->slice_height, src_cap->max_slice_height);
			in_params->slice_height = src_cap->max_slice_height;
		}

		/* slice width */
		if (in_params->slice_width > src_cap->max_slice_width) {
			DPU_ERROR("unsupported slice width %d, max:%d\n",
					in_params->slice_width, src_cap->max_slice_width);
			return -EINVAL;
		}

		/* bpc */
		for (i = 0; i < src_cap->bpcs_size; i++)
			if (in_params->bits_per_component == src_cap->bpcs[i])
				break;
		if (i == src_cap->bpcs_size) {
			DPU_ERROR("unsupported bits_per_component %d\n",
					in_params->bits_per_component);
			return -EINVAL;
		}

		/* bpp */
		if (in_params->bits_per_pixel != 0x8) {
			DPU_WARN("unsupported compressed bpp: %d, reset to %d",
					in_params->bits_per_pixel, 0x8);
			in_params->bits_per_pixel = 0x8;
		}

		/* line buffer depth */
		line_buffer_depth = in_params->bits_per_component + 1;
		if (line_buffer_depth > src_cap->max_line_buffer_depth) {
			DPU_WARN("unsupported line buffer depth (bpc + 1): %d, reset to:%d\n",
					line_buffer_depth, src_cap->max_line_buffer_depth);
			line_buffer_depth = src_cap->max_line_buffer_depth;
		}
		in_params_priv->line_buffer_depth = line_buffer_depth;

		/* slice count */
		slice_cnt = DIV_ROUND_UP(in_params->pic_width, in_params->slice_width);
		if (slice_cnt > src_cap->max_slice_cnt) {
			DPU_WARN("unsupported slice count: %d, reset to %d\n",
					slice_cnt, src_cap->max_slice_cnt);
			slice_cnt = src_cap->max_slice_cnt;
			slice_width = DIV_ROUND_UP(in_params->pic_width, slice_cnt);
			DPU_WARN("reset slice width to:%d", slice_width);
			if (slice_width > src_cap->max_slice_width) {
				DPU_ERROR("unsupported slice width, max:%d\n",
						src_cap->max_slice_width);
				return -EINVAL;
			}
			in_params->slice_width = slice_width;
		}
		in_params_priv->slice_count = slice_cnt;

		/* bpe */
		in_params_priv->block_predication_en = true;

	} else {
		if (!sink_cap->dsc_supported) {
			DPU_ERROR("sink doesn`t support dsc\n");
			return -EINVAL;
		}

		in_params->dsc_version_major = DSC_VERSION_MAJOR_V1;

		in_params->dsc_version_minor = min_t(u8,
				sink_cap->dsc_version_minor, src_cap->dsc_version_minor);

		/* picture width */
		if (in_params->pic_width > src_cap->max_pic_width) {
			DPU_ERROR("unsupported pic width %d, max:%d\n",
					in_params->pic_width, src_cap->max_pic_width);
			return -EINVAL;
		}

		/* picture height */
		if (in_params->pic_height > src_cap->max_pic_height) {
			DPU_ERROR("unsupported pic height %d, max:%d\n",
					in_params->pic_height, src_cap->max_pic_height);
			return -EINVAL;
		}

		/* slice count */
		in_params_priv->slice_count = dpu_dsc_max_slice_cnt(sink_cap->slice_counts,
				sink_cap->slice_counts_size, src_cap->max_slice_cnt);
		/* slice width */
		in_params->slice_width = DIV_ROUND_UP(in_params->pic_width,
				in_params_priv->slice_count);
		if (in_params->slice_width > sink_cap->max_slice_width ||
				in_params->slice_width > src_cap->max_slice_width) {
			DPU_ERROR("unsupported slice width %d, src caps:%d, sink caps:%d\n",
					in_params->slice_width,
					src_cap->max_slice_width,
					sink_cap->max_slice_width);
			return -EINVAL;
		}

		/* slice height */
		in_params->slice_height = (in_params->pic_height >= src_cap->max_slice_height)
				? in_params->pic_height / 2 : in_params->pic_height;

		/* bpc */
		if (!dpu_dsc_bpc_supported(in_params->bits_per_component,
				sink_cap->bpcs, sink_cap->bpcs_size,
				src_cap->bpcs, src_cap->bpcs_size)) {
			DPU_ERROR("unsupported bpc: %d\n", in_params->bits_per_component);
			return -EINVAL;
		}

		/* bpp */
		in_params->bits_per_pixel = 8;

		/* line buffer depth */
		line_buffer_depth = in_params->bits_per_component + 1;
		if (line_buffer_depth > sink_cap->max_line_buffer_depth ||
				line_buffer_depth > src_cap->max_line_buffer_depth) {
			line_buffer_depth = min_t(u8, sink_cap->max_line_buffer_depth,
					src_cap->max_line_buffer_depth);
			DPU_WARN("unsupported line buffer depth %d, reset to:%d\n",
					(in_params->bits_per_component + 1), line_buffer_depth);
		}
		in_params_priv->line_buffer_depth = line_buffer_depth;

		/* bpe */
		in_params_priv->block_predication_en =
				sink_cap->block_predication_supported &&
				src_cap->block_predication_supported;
	}

	return 0;
}

static void dpu_dsc_customized_config_get(struct dsc_parms_private *dsc_params_priv,
		struct dpu_dsc_config *dsc_config)
{
	struct dsc_parms *dsc_params = dsc_params_priv->parms;
	u8 *customized_pps_table;

	dsc_config->customized_pps_table = dsc_params->customized_pps_table;
	customized_pps_table = dsc_config->customized_pps_table;
	/* slice count for DSC custom config */
	dsc_config->slice_count = dsc_params_priv->slice_count;
	/* parameters for min rate buffer size */
	dsc_config->slice_width = dsc_params->slice_width;
	dsc_config->rc_model_size = DSC_RC_MODE_SIZE;
	dsc_config->initial_offset = dpu_dsc_initial_offset(dsc_config);
	dsc_config->initial_xmit_delay = DSC_INITIAL_XMIT_DELAY;
	dsc_config->bits_per_pixel = dsc_params->bits_per_pixel << 4;
	dsc_config->first_line_bpg_offset = customized_pps_table[27];

	dsc_config->dsc_version_major = dsc_params->dsc_version_major;
	dsc_config->dsc_version_minor = dsc_params->dsc_version_minor;
	DPU_DEBUG("slice_count %d\n", dsc_config->slice_count);
	DPU_DEBUG("rc_model_size %d\n", dsc_config->rc_model_size);
	DPU_DEBUG("initial_offset %d\n", dsc_config->initial_offset);
	DPU_DEBUG("initial_xmit_delay %d\n", dsc_config->initial_xmit_delay);
	DPU_DEBUG("bits_per_pixel %d\n", dsc_config->bits_per_pixel);
	DPU_DEBUG("first_line_bpg_offset %d\n", dsc_config->first_line_bpg_offset);
	DPU_DEBUG("dsc_version_minor %d\n", dsc_config->dsc_version_minor);
	DPU_DEBUG("dsc_version_minor %d\n", dsc_config->dsc_version_minor);
}

void dpu_dsc_get_config(struct dsc_parms *in_params,
		struct dsc_caps_sink *sink_cap, struct dsc_caps_src *src_cap,
		struct dpu_dsc_config *dsc_config)
{
	struct dsc_parms_private parms_priv;
	u32 max_rate_buffer_size;
	u32 rate_buffer_size;

	parms_priv.parms = in_params;

	if (dpu_dsc_params_correct(&parms_priv, sink_cap, src_cap)) {
		DP_ERROR("invalid value for dsc caps\n");
		goto error;
	}

	max_rate_buffer_size = sink_cap ? min_t(u32,
			sink_cap->rate_buffer_size_kbytes << 10,
			src_cap->rate_buffer_size_bytes) :
			src_cap->rate_buffer_size_bytes;

	/* Calculate the rate_buffer_size for customized_pps_table */
	dsc_config->customized_pps_table = in_params->customized_pps_table;
	if (dsc_config->customized_pps_table) {
		dpu_dsc_customized_config_get(&parms_priv, dsc_config);
		rate_buffer_size = DIV_ROUND_UP(
				dpu_dsc_min_rate_buffer_size(dsc_config), 8);
		DPU_DEBUG("rate_buffer_size: %d\n", rate_buffer_size);
		/* Adjust the rate_buffer_size for customized_pps_table is unnecessary */
		if (rate_buffer_size > max_rate_buffer_size) {
			DPU_ERROR("invalid rate buffer size from " \
					"customized_pps_table: %d Byte\n",
					rate_buffer_size);
			goto error;
		}

		goto exit;
	}

	do {
		dpu_dsc_populate_dsc_config(&parms_priv, dsc_config);
		rate_buffer_size = DIV_ROUND_UP(
				dpu_dsc_min_rate_buffer_size(dsc_config), 8);

		if (rate_buffer_size <= max_rate_buffer_size)
			break;

		if (dsc_config->slice_height == 1) {
			DPU_ERROR("unsupported rate buffer size: %d Byte\n",
					rate_buffer_size);
			goto error;
		}

		dsc_config->slice_height /= 2;
		DPU_WARN("unsupported rate buffer size: %d Byte, reset slice height to %d\n",
				rate_buffer_size, dsc_config->slice_height);
	} while (1);

exit:
	dsc_config->valid = true;
	return;

error:
	dsc_config->valid = false;
}
