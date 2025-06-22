#include "dpu_log.h"
#include "dpu_format.h"
#include "dpu_res_mgr.h"
#include "dpu_pre_color.h"
#include "dpu_color_internal.h"

/* define postpq module cfg object, fill and pass it to lowlevel */
struct prepq_module_payload {
	enum dpu_crtc_postpq_features feature;
	void *data;
	uint32_t len;

	bool is_int_value;
	uint32_t value;
};

/* update this global payload array in
 * dpu_pre_color_cfg and color_prepq_update_cfg
 */
static struct prepq_module_payload g_prepq_payload_array[] = {
	[PLANE_COLOR_SCALER_EN] = {PLANE_COLOR_SCALER_EN, NULL, 0, true, 0}, /* default disable */
	[PLANE_COLOR_SCALER] = {PLANE_COLOR_SCALER, NULL, sizeof(struct dpu_2d_scaler_cfg), false, 0},
	[PLANE_COLOR_PREALPHA_EN] = {PLANE_COLOR_PREALPHA_EN, NULL, 0, true, 0},
	[PLANE_COLOR_NON_PREALPHA_EN] = {PLANE_COLOR_NON_PREALPHA_EN, NULL, 0, true, 0},
	[PLANE_COLOR_NON_PREALPHA] = {PLANE_COLOR_NON_PREALPHA, NULL, 0, false, 0},
	[PLANE_COLOR_R2Y_EN] = {PLANE_COLOR_R2Y_EN, NULL, 0, true, 0},
	[PLANE_COLOR_R2Y] = {PLANE_COLOR_R2Y, NULL, sizeof(struct dpu_csc_matrix_cfg), false, 0},
	[PLANE_COLOR_Y2R_EN] = {PLANE_COLOR_Y2R_EN, NULL, 0, true, 0},
	[PLANE_COLOR_Y2R] = {PLANE_COLOR_Y2R, NULL, sizeof(struct dpu_csc_matrix_cfg), false, 0},
	[PLANE_COLOR_ALPHA_EN] = {PLANE_COLOR_ALPHA_EN, NULL, 0, true, 0},
	[PLANE_COLOR_TM_EN] = {PLANE_COLOR_TM_EN, NULL, 0, true, 0},
	[PLANE_COLOR_TM] = {PLANE_COLOR_TM, NULL, sizeof(struct dpu_tm_cfg), false, 0},
	[PLANE_COLOR_LUT3D_EN] = {PLANE_COLOR_LUT3D_EN, NULL, 0, true, 0},
	[PLANE_COLOR_LUT3D] = {PLANE_COLOR_LUT3D, NULL, sizeof(struct dpu_lut_3d_cfg), false, 0},
	[PLANE_COLOR_HIST_READ_MODE] = {PLANE_COLOR_HIST_READ_MODE, NULL, 0, true, 0},
	[PLANE_COLOR_HIST] = {PLANE_COLOR_HIST, NULL, sizeof(struct dpu_hist_cfg), false, 0},
};

static void dpu_pre_scaler_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_pre_scaler *hw_pre_scaler;
	bool scaler_enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PRE_SCALER, layer->dma_id);
	if (!hw_blk) {
		dpu_pr_debug("not v-chn or get dpu_hw_blk failed\n");
		return;
	}

	hw_pre_scaler = to_dpu_hw_pre_scaler(hw_blk);
	dpu_check_and_no_retval(!hw_pre_scaler, "get dpu_hw_pre_scaler failed\n");

	scaler_enable = g_prepq_payload_array[PLANE_COLOR_SCALER_EN].value;
	hw_pre_scaler->ops.enable(hw_blk, scaler_enable);

	if (scaler_enable) {
		hw_cfg->data = g_prepq_payload_array[PLANE_COLOR_SCALER].data;;
		hw_cfg->len = g_prepq_payload_array[PLANE_COLOR_SCALER].len;

		hw_pre_scaler->ops.set(hw_blk, hw_cfg);
	}

	dpu_pr_debug("-\n");
}

static void dpu_pre_r2y_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_prepipe_top *hw_prepipe_top;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PREPIPE_TOP, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);
	dpu_check_and_no_retval(!hw_prepipe_top, "get dpu_hw_prepipe_top failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_R2Y_EN].value;

	if (enable) {
		hw_cfg->data = g_prepq_payload_array[PLANE_COLOR_R2Y].data;;
		hw_cfg->len = g_prepq_payload_array[PLANE_COLOR_R2Y].len;
		layer->enabled_features |= BIT(PLANE_COLOR_R2Y_EN);
		hw_prepipe_top->ops.set_r2y(hw_blk, hw_cfg);
	} else {
		layer->enabled_features &= ~BIT(PLANE_COLOR_R2Y_EN);
	}

	dpu_pr_debug("-\n");
}

static void dpu_pre_y2r_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_prepipe_top *hw_prepipe_top;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PREPIPE_TOP, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);
	dpu_check_and_no_retval(!hw_prepipe_top, "get dpu_hw_prepipe_top failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_Y2R_EN].value;

	if (enable) {
		hw_cfg->data = g_prepq_payload_array[PLANE_COLOR_Y2R].data;;
		hw_cfg->len = g_prepq_payload_array[PLANE_COLOR_Y2R].len;
		layer->enabled_features |= BIT(PLANE_COLOR_Y2R_EN);
		hw_prepipe_top->ops.set_y2r(hw_blk, hw_cfg);
	} else {
		layer->enabled_features &= ~BIT(PLANE_COLOR_Y2R_EN);
	}

	dpu_pr_debug("-\n");
}

static void dpu_pre_alpha_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_prepipe_top *hw_prepipe_top;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PREPIPE_TOP, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);
	dpu_check_and_no_retval(!hw_prepipe_top, "get dpu_hw_prepipe_top failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_PREALPHA_EN].value;
	if (enable) {
		layer->enabled_features |= BIT(PLANE_COLOR_PREALPHA_EN);
	} else {
		layer->enabled_features &= ~BIT(PLANE_COLOR_PREALPHA_EN);
	}
	dpu_pr_debug("-\n");
}

static void dpu_non_pre_alpha_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_prepipe_top *hw_prepipe_top;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PREPIPE_TOP, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);
	dpu_check_and_no_retval(!hw_prepipe_top, "get dpu_hw_prepipe_top failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_NON_PREALPHA_EN].value;

	if (enable) {
		struct dpu_none_prealpha_offset payload = {
			.offset0 = 16,
			.offset1 = 128,
			.offset2 = 128,
		};
		hw_cfg->data = &payload;
		hw_cfg->len = sizeof(struct dpu_none_prealpha_offset);
		layer->enabled_features |= BIT(PLANE_COLOR_NON_PREALPHA_EN);
		hw_prepipe_top->ops.set_nonpre_alpha(hw_blk, hw_cfg);
	} else {
		layer->enabled_features &= ~BIT(PLANE_COLOR_NON_PREALPHA_EN);
	}

	dpu_pr_debug("-\n");
}

static void dpu_tm_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_tm *hw_tm;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_TM, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_tm = to_dpu_hw_tm(hw_blk);
	dpu_check_and_no_retval(!hw_tm, "get dpu_hw_tm failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_TM_EN].value;
	hw_tm->ops.enable(hw_blk, enable);

	if (enable) {
		hw_cfg->data = g_prepq_payload_array[PLANE_COLOR_TM].data;
		hw_cfg->len = g_prepq_payload_array[PLANE_COLOR_TM].len;
		hw_tm->ops.set_tm(hw_blk, hw_cfg);
	}

	dpu_pr_debug("-\n");
}

static void dpu_pre_3dlut_commit(struct prepq_hw_cfg *hw_cfg, disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_pre_3dlut *hw_3dlut;
	bool enable;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PRE_3DLUT, RCH_V_0);
	if (!hw_blk) {
		dpu_pr_debug("not v0 or get dpu_hw_blk failed\n");
		return;
	}

	hw_3dlut = to_dpu_hw_pre_3dlut(hw_blk);
	dpu_check_and_no_retval(!hw_3dlut, "get dpu_hw_pre_3dlut failed\n");

	enable = g_prepq_payload_array[PLANE_COLOR_LUT3D_EN].value;
	hw_3dlut->ops.enable(hw_blk, enable);

	if (enable) {
		hw_cfg->data = g_prepq_payload_array[PLANE_COLOR_LUT3D].data;
		hw_cfg->len = g_prepq_payload_array[PLANE_COLOR_LUT3D].len;
		hw_3dlut->ops.set_3dlut(hw_blk, hw_cfg);
		hw_3dlut->ops.flush(hw_blk, false);
	}

	dpu_pr_debug("-\n");
}

static void color_prepq_update_cfg(layer_color_t *color)
{
	/* if layer color enabled, will update payload array first before commit */
	g_prepq_payload_array[PLANE_COLOR_PREALPHA_EN].value = color->pre_alpha_en;
	g_prepq_payload_array[PLANE_COLOR_NON_PREALPHA_EN].value = color->non_pre_alpha_en;

	g_prepq_payload_array[PLANE_COLOR_R2Y_EN].value = color->r2y.enable;
	g_prepq_payload_array[PLANE_COLOR_R2Y].data = &color->r2y;
	g_prepq_payload_array[PLANE_COLOR_Y2R_EN].value = color->y2r.enable;
	g_prepq_payload_array[PLANE_COLOR_Y2R].data = &color->y2r;
	g_prepq_payload_array[PLANE_COLOR_SCALER_EN].value = color->scaler.enable;
	g_prepq_payload_array[PLANE_COLOR_SCALER].data = &color->scaler;
	g_prepq_payload_array[PLANE_COLOR_TM_EN].value = color->tm.enable;
	g_prepq_payload_array[PLANE_COLOR_TM].data = &color->tm;
	g_prepq_payload_array[PLANE_COLOR_LUT3D_EN].value = color->lut3d.enable;
	g_prepq_payload_array[PLANE_COLOR_LUT3D].data = &color->lut3d;
	g_prepq_payload_array[PLANE_COLOR_HIST_READ_MODE].value = color->hist.enable;
	g_prepq_payload_array[PLANE_COLOR_HIST].data = &color->hist;
}

static void dpu_prepq_top_enable(disp_layer_t *layer)
{
	struct dpu_hw_blk *hw_blk;
	struct prepq_feature_ctrl ctrl_info;
	struct dpu_hw_prepipe_top *hw_prepipe_top;

	dpu_pr_debug("+\n");

	hw_blk = get_hw_blk(BLK_PREPIPE_TOP, layer->chn_idx);
	dpu_check_and_no_retval(!hw_blk, "get dpu_hw_blk failed\n");

	hw_prepipe_top = to_dpu_hw_prepipe_top(hw_blk);
	dpu_check_and_no_retval(!hw_prepipe_top, "get dpu_hw_prepipe_top failed\n");

	ctrl_info.enabled_features = layer->enabled_features;
	ctrl_info.left_right_reuse = false;
	hw_prepipe_top->ops.feature_enable(hw_blk, &ctrl_info);

	dpu_pr_debug("-\n");
}

static void color_prepq_commit_feature(disp_layer_t *layer)
{
	struct prepq_hw_cfg hw_cfg;

	dpu_mem_set(&hw_cfg, 0, sizeof(hw_cfg));
	dpu_pre_r2y_commit(&hw_cfg, layer);
	dpu_pre_scaler_commit(&hw_cfg, layer);
	dpu_pre_y2r_commit(&hw_cfg, layer);
	dpu_pre_alpha_commit(&hw_cfg, layer);
	dpu_non_pre_alpha_commit(&hw_cfg, layer);
	dpu_tm_commit(&hw_cfg, layer);
	dpu_pre_3dlut_commit(&hw_cfg, layer);
	dpu_prepq_top_enable(layer);
}

int dpu_pre_color_cfg(disp_layer_t *layer)
{
	dpu_check_and_return(!layer, -1, "layer is null\n");

	if (!layer->prepq_enable)
		return 0;

	dpu_check_and_return(!layer->layer_color, -1,
			"layer->layer_color is null\n");

	color_prepq_update_cfg(layer->layer_color);
	color_prepq_commit_feature(layer);

	return 0;
}