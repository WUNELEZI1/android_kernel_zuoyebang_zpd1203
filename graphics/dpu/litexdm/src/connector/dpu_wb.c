#include "dpu_hw_common.h"
#include "dpu_hw_wb.h"
#include "dpu_format.h"
#include "dpu_hw_cap.h"
#include "dpu_common_info.h"
#include "dpu_res_mgr.h"
#include "dpu_log.h"
#include <string.h>

#include "dpu_wb.h"

static int WB_RGB2YUV_MATRIX_8Bit[YUV_SPACE_LIMIT][3][4] = {
  	/* YUVCSC_BT601_WIDE */
	{
		{1224,  2403,  469,    0},
		{-691, -1357, 2048,  128},
		{2048, -1714, -334,  128},
	},
	/* YUVCSC_BT601_NARROW */
	{
		{1051,  2064,  403,   16},
		{-607, -1192, 1799,  128},
		{1799, -1505, -294,  128},
	},
	/* YUVCSC_BT709_WIDE */
	{
		{ 871,  2929,  296,    0},
		{-469, -1579, 2048,  128},
		{2048, -1860, -188,  128},
	},
	/* YUVCSC_BT709_NARROW */
	{
		{ 748, 2516,  254,   16},
		{-412,-1387, 1799,  128},
		{1799,-1634, -165,  128},
	},
	/* YUVCSC_BT2020_WIDE */
	{
		{1076,  2777,  243,    0},
		{-572, -1476, 2048,  128},
		{2048, -1883, -165,  128},
	},
	/* YUVCSC_BT2020_NARROW */
	{
		{ 924,  2385,  209,   16},
		{-502, -1297, 1799,  128},
		{1799, -1654, -145,  128},
	},
	/* YUVCSC_BT2100_WIDE */
	{
	},
	/* YUVCSC_BT2100_NARROW */
	{
	}
};

enum dpu_hw_wb_pos_type {
	WB_POS_TYPE_AFTER_MIXER,
	WB_POS_TYPE_AFTER_POSTPQ,
	WB_POS_TYPE_AFTER_RC,
	WB_POS_TYPE_BEFORE_PREPQ,
	WB_POS_TYPE_AFTER_PREPQ,
	WB_POS_TYPE_INVALID,
	WB_POS_TYPE_MAX = WB_POS_TYPE_INVALID,
};

static bool is_yuv_format(uint32_t format)
{
	bool check_result = false;

	switch (format) {
	case YUV420_P2_8_UVUV:
	case YUV420_P2_8_VUVU:
	case YUV420_P2_10_UVUV:
	case YUV420_P2_10_VUVU:
		check_result = true;
		break;
	default:
		check_result = false;
		break;
	}

	return check_result;
}

static void dpu_wb_get_addr_info(struct wb_frame_cfg *wb_cfg,
		disp_wb_layer_t *layer_info)
{
	switch (wb_cfg->plane_count) {
	case PLANE_ONE:
		wb_cfg->wdma_base_addrs[WB_PLANAR_Y_RGB] = layer_info->buf_addrs[0];
		break;
	case PLANE_TWO:
		wb_cfg->wdma_base_addrs[WB_PLANAR_Y_RGB] = layer_info->buf_addrs[0];
		wb_cfg->wdma_base_addrs[WB_PLANAR_UV] = layer_info->buf_addrs[1];
		break;
	default:
		dpu_pr_err("invalid format wb plane count: %d\n", wb_cfg->plane_count);
		break;
	}

	if (!wb_cfg->is_afbc)
		wb_cfg->raw_format_stride = layer_info->stride; /* fb->picthes */
}

static void dpu_wb_get_wblayer_cfg(struct wb_frame_cfg *wb_cfg, disp_wb_layer_t *layer_info)
{
	int k = 0;

	dpu_pr_debug("+\n");
	wb_cfg->input_width = layer_info->in_width;
	wb_cfg->input_height = layer_info->in_height;
	wb_cfg->is_offline_mode = layer_info->is_offline;

	if (layer_info->in_crop_en) {
		wb_cfg->crop0_en = true;
		wb_cfg->crop0_area.x1 = layer_info->in_crop_rect.x;
		wb_cfg->crop0_area.y1 = layer_info->in_crop_rect.y;
		wb_cfg->crop0_area.x2 = layer_info->in_crop_rect.x + layer_info->in_crop_rect.w - 1;
		wb_cfg->crop0_area.y2 = layer_info->in_crop_rect.y + layer_info->in_crop_rect.h - 1;
	}

	wb_cfg->scl_en = layer_info->scaler_en;
	wb_cfg->output_format = layer_info->format;

	/* dfc */
	wb_cfg->is_yuv = is_yuv_format(layer_info->format);
	wb_cfg->is_uv_swap = layer_info->uv_swap;

	if (wb_cfg->is_yuv) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 4; j++)
				wb_cfg->csc_cfg.matrix[k++] = WB_RGB2YUV_MATRIX_8Bit[layer_info->yuv_flag][i][j];
		}
	}

	wb_cfg->frame_rot_mode = layer_info->rotation;
	wb_cfg->frame_flip_mode = layer_info->flip;

	/* output config */
	if (layer_info->afbc_en) {
		wb_cfg->is_afbc = layer_info->afbc_en;
		wb_cfg->is_afbc_32x8 = layer_info->afbc_wide;
		wb_cfg->is_afbc_split = layer_info->afbc_split;
		wb_cfg->is_afbc_yuv_transform = layer_info->afbc_yuv;

		/**
		 * WB AFBCE copy mode & default color must be configured to be true to
		 * align with the GPU.
		 */
		wb_cfg->is_afbc_copy_mode = true;
		wb_cfg->is_afbc_default_color = true;
	}

	wb_cfg->output_subframe_rect.x = 0;
	wb_cfg->output_subframe_rect.y = 0;
	wb_cfg->output_subframe_rect.w = layer_info->dst_width;
	wb_cfg->output_subframe_rect.h = layer_info->dst_height;
	wb_cfg->output_original_width = layer_info->dst_width;
	wb_cfg->output_original_height = layer_info->dst_height;

	wb_cfg->plane_count = layer_info->plane_num;

	dpu_wb_get_addr_info(wb_cfg, layer_info);
}

int32_t dpu_wb_cfg(struct composer *comp, struct dpu_frame *frame)
{
	struct wb_frame_cfg wb_cfg = {0};
	disp_wb_layer_t *layer_info;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_hw_wb *hw_wb;
	struct dpu_hw_wb_scaler *hw_wb_scaler;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp || !frame, -1, "comp or frame is null\n");

	layer_info = &frame->wb_layer_info;
	dpu_wb_get_wblayer_cfg(&wb_cfg, layer_info);

	hw_wb = comp->hw_wb;
	dpu_check_and_return(!hw_wb, -1, "hw_wb is null\n");

	hw_wb->ops.reset(&hw_wb->hw);
	hw_wb->ops.frame_config(&hw_wb->hw, &wb_cfg);


	hw_wb_scaler = comp->hw_wb_scaler;
	dpu_check_and_return(!hw_wb_scaler, -1, "hw_wb_scaler is null\n");

	hw_wb_scaler->ops.enable(&hw_wb_scaler->hw, layer_info->scaler_en);
	if (layer_info->scaler_en)
		hw_wb_scaler->ops.set(&hw_wb_scaler->hw,
				&layer_info->pq.scaler_param.scaler);

	hw_ctl_top = comp->hw_ctl_top;
	if (hw_ctl_top)
		hw_ctl_top->ops.wb_input_position_config(&hw_ctl_top->hw,
				hw_wb->hw.blk_id, GET_WB_POS_AFTER_MIXER(comp->id));

	dpu_pr_debug("-\n");
	return 0;
}
