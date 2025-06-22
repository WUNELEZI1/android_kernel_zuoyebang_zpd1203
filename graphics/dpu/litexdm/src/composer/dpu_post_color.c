#include "litexdm.h"
#include "dpu_log.h"
#include "dpu_format.h"
#include "dpu_res_mgr.h"
#include "dpu_post_color.h"
#include "dpu_comp_mgr.h"

/* define postpq module cfg object, fill and pass it to lowlevel */
struct postpq_module_payload {
	enum dpu_crtc_postpq_features feature;
	void *data;
	uint32_t len;

	bool is_int_value;
	uint32_t value;
};

/* update this global payload struct in
 * dpu_mixer_color_postpq_init and color_postpq_update_cfg
 */
struct postpq_module_payload g_postpq_payload_array[] = {
	[DPU_CRTC_POSTPQ_LTM_EN] = {DPU_CRTC_POSTPQ_LTM_EN, NULL, 0, true, 0}, /* default disable */
	[DPU_CRTC_POSTPQ_LTM] = {DPU_CRTC_POSTPQ_LTM, NULL, sizeof(struct dpu_ltm_cfg), false, 0},
	[DPU_CRTC_POSTPQ_R2Y_EN] = {DPU_CRTC_POSTPQ_R2Y_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_R2Y] = {DPU_CRTC_POSTPQ_R2Y, NULL, sizeof(struct dpu_csc_matrix_cfg), false, 0},
	[DPU_CRTC_POSTPQ_SCALER_EN] = {DPU_CRTC_POSTPQ_SCALER_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_SCALER] = {DPU_CRTC_POSTPQ_SCALER, NULL, sizeof(struct dpu_2d_scaler_cfg), false, 0},
	[DPU_CRTC_POSTPQ_DITHER_LITE_EN] = {DPU_CRTC_POSTPQ_DITHER_LITE_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_Y2R_EN] = {DPU_CRTC_POSTPQ_Y2R_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_Y2R] = {DPU_CRTC_POSTPQ_Y2R, NULL, sizeof(struct dpu_csc_matrix_cfg), false, 0},
	[DPU_CRTC_POSTPQ_MUL_MATRIX_EN] = {DPU_CRTC_POSTPQ_MUL_MATRIX_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_MUL_MATRIX] = {DPU_CRTC_POSTPQ_MUL_MATRIX, NULL, sizeof(struct dpu_post_matrix_3x4_cfg), false, 0},
	[DPU_CRTC_POSTPQ_3DLUT_EN] = {DPU_CRTC_POSTPQ_3DLUT_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_3DLUT] = {DPU_CRTC_POSTPQ_3DLUT, NULL, sizeof(struct dpu_lut_3d_cfg), false, 0},
	[DPU_CRTC_POSTPQ_CSC_MATRIX_EN] = {DPU_CRTC_POSTPQ_CSC_MATRIX_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_CSC_MATRIX] = {DPU_CRTC_POSTPQ_CSC_MATRIX, NULL, sizeof(struct dpu_post_matrix_3x3_cfg), false, 0},
	[DPU_CRTC_POSTPQ_USR_GAMMA_EN] = {DPU_CRTC_POSTPQ_USR_GAMMA_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_USR_GAMMA] = {DPU_CRTC_POSTPQ_USR_GAMMA, NULL, sizeof(struct dpu_gamma_cfg), false, 0},
	[DPU_CRTC_POSTPQ_DITHER_EN] = {DPU_CRTC_POSTPQ_DITHER_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_DITHER] = {DPU_CRTC_POSTPQ_DITHER, NULL, sizeof(struct dpu_dither_cfg), false, 0},
	[DPU_CRTC_POSTPQ_ACAD_EN] = {DPU_CRTC_POSTPQ_ACAD_EN, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_ACAD] = {DPU_CRTC_POSTPQ_ACAD, NULL, sizeof(struct dpu_acad_cfg), false, 0},
	[DPU_CRTC_POSTPQ_HIST_READ_MODE] = {DPU_CRTC_POSTPQ_HIST_READ_MODE, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_HIST] = {DPU_CRTC_POSTPQ_HIST, NULL, sizeof(struct dpu_hist_cfg), false, 0},
	[DPU_CRTC_POSTPQ_RC_REGION] = {DPU_CRTC_POSTPQ_RC_REGION, NULL, 0, true, 0},
	[DPU_CRTC_POSTPQ_RC] = {DPU_CRTC_POSTPQ_RC, NULL, sizeof(struct dpu_rc_cfg), false, 0},
};

/* module flush funcs defined here */
static void postpq_usr_gamma_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;

	if (!gamma)
		return;

	gamma->ops.flush(&gamma->hw, false);
}

static void postpq_3dlut_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;

	if (!lut_3d)
		return;

	lut_3d->ops.flush(&lut_3d->hw, false);
}

static void postpq_top_flush(struct color_postpq *postpq)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top)
		return;

	postpq_top->ops.flush(&postpq_top->hw);
}

static void postpq_pre_proc_flush(struct color_postpq *postpq)
{
	struct dpu_hw_postpre_proc *postpre_proc = postpq->hw_pre_proc;

	if (!postpre_proc)
		return;

	postpre_proc->ops.flush(&postpre_proc->hw);
}

static void postpq_acad_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;

	if (!acad)
		return;

	acad->ops.flush(&acad->hw);
}

static void postpq_hist_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;

	if (!hist)
		return;

	hist->ops.flush(&hist->hw);
}

static void postpq_scaler_flush(struct color_postpq *postpq)
{
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;

	if (!scaler)
		return;

	scaler->ops.flush(&scaler->hw);
}

typedef void (*postpq_module_flush_func_t)(struct color_postpq *postpq);
static postpq_module_flush_func_t
	postpq_module_flush_func[POSTPQ_MODULE_MAX];

#define postpq_module_flush_funcs_init(func) \
do { \
	func[POSTPQ_DPP_GAMMA] = postpq_usr_gamma_flush; \
	func[POSTPQ_DPP_3DLUT] = postpq_3dlut_flush; \
	func[POSTPQ_TOP] = postpq_top_flush; \
	func[POSTPQ_PRE_PROC] = postpq_pre_proc_flush; \
	func[POSTPQ_ACAD] = postpq_acad_flush; \
	func[POSTPQ_HIST] = postpq_hist_flush; \
	func[POSTPQ_SCALER] = postpq_scaler_flush;\
} while (0)


/* module commit funcs defined here */
static void postpq_usr_gamma_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;

	if (!gamma || !postpq->gma->enable)
		return;

	gamma->ops.set_gamma(&gamma->hw, hw_cfg);

	dpu_pr_debug("-\n");
}

static void postpq_usr_gamma_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_gamma *gamma = postpq->hw_gamma;
	u32 feature = BIT(DPU_CRTC_POSTPQ_USR_GAMMA_EN);

	if (!gamma)
		return;

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		gamma->ops.enable(&gamma->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		gamma->ops.enable(&gamma->hw, false);
	}
	dpu_pr_debug("-\n");
}

static void postpq_3dlut_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;

	if (!lut_3d || !postpq->lut3d->enable)
		return;

	lut_3d->ops.set_3dlut(&lut_3d->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_3dlut_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_3dlut *lut_3d = postpq->hw_3dlut;
	u32 feature = BIT(DPU_CRTC_POSTPQ_3DLUT_EN);

	if (!lut_3d)
		return;

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		lut_3d->ops.enable(&lut_3d->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		lut_3d->ops.enable(&lut_3d->hw, false);
	}
	dpu_pr_debug("-\n");
}

static void postpq_r2y_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top || !postpq->r2y->enable)
		return;

	postpq_top->ops.set_r2y(&postpq_top->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_r2y_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_R2Y_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_scaler_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!scaler || !postpq_top || !postpq->scaler->enable)
		return;

	scaler->ops.set(&scaler->hw, hw_cfg);
	postpq_top->ops.set_scaler_output_size(&postpq_top->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_scaler_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;
	struct dpu_hw_post_scaler *scaler = postpq->hw_scaler;
	u32 feature = BIT(DPU_CRTC_POSTPQ_SCALER_EN);

	if (!postpq_top || !scaler)
		return;

	postpq_top->ops.set_scaler_output_size(&postpq_top->hw, hw_cfg);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
		scaler->ops.enable(&scaler->hw, true);
	} else {
		postpq->enabled_features &= ~feature;
		scaler->ops.enable(&scaler->hw, false);
	}

	dpu_pr_debug("-\n");
}

static void postpq_dither_lite_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_DITHER_LITE_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_y2r_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top || !postpq->y2r->enable)
		return;

	postpq_top->ops.set_y2r(&postpq_top->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_y2r_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_Y2R_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_csc_matrix_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top || !postpq->cscmatrix->enable)
		return;

	postpq_top->ops.set_csc_matrix(&postpq_top->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_csc_matrix_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_CSC_MATRIX_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_dither_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top || !postpq->dither->enable)
		return;

	postpq_top->ops.set_dither(&postpq_top->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_dither_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_DITHER_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_ltm_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;

	if (!pre_proc || !postpq->ltm->enable)
		return;

	pre_proc->ops.set_eotf_oetf(&pre_proc->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_ltm_enable(struct color_postpq *postpq,
	struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_LTM_EN);

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_acad_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;

	if (!acad || !postpq->acad->enable)
		return;

	acad->ops.set_acad(&acad->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_acad_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_acad *acad = postpq->hw_acad;
	u32 feature = BIT(DPU_CRTC_POSTPQ_ACAD_EN);

	if (!acad)
		return;

	if (hw_cfg->value > 0) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
		acad->ops.enable(&acad->hw, false);
	}
	dpu_pr_debug("-\n");
}

static void postpq_mul_matrix_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;

	if (!pre_proc || !postpq->mulmatrix->enable)
		return;

	pre_proc->ops.set_mul_matrix(&pre_proc->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_mul_matrix_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 feature = BIT(DPU_CRTC_POSTPQ_MUL_MATRIX_EN);

	if (hw_cfg->value) {
		postpq->enabled_features |= feature;
	} else {
		postpq->enabled_features &= ~feature;
	}
	dpu_pr_debug("-\n");
}

static void postpq_hist_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;

	if (!hist || !postpq->hist->enable)
		return;

	hist->ops.set_hist(&hist->hw, hw_cfg);
	dpu_pr_debug("-\n");
}

static void postpq_hist_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_hist *hist = postpq->hw_hist;
	u32 feature = BIT(DPU_CRTC_POSTPQ_HIST_READ_MODE);
	u32 read_mode;

	if (!hist)
		return;

	read_mode = hw_cfg->value;

	if (postpq->hist_event.hist_read_mode != read_mode)
		hist->ops.enable_update(&hist->hw);

	postpq->hist_event.hist_read_mode = read_mode;

	if (read_mode == HIST_READ_DISABLE) {
		postpq->enabled_features &= ~feature;
		hist->ops.enable(&hist->hw, false);
		postpq->hist_event.enable = false;
	} else {
		postpq->enabled_features |= feature;
		hist->ops.enable(&hist->hw, true);
		postpq->hist_event.enable = true;
		if (postpq->hist_event.hist_read_mode == HIST_READ_ONCE)
			postpq->hist_event.read_cnt++;
	}
	dpu_pr_debug("-\n");
}

static void postpq_rc_commit(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top = postpq->hw_post_pipe_top;

	if (!hw_post_pipe_top || !postpq->rc->enable)
		return;

	hw_post_pipe_top->ops.rc_stream_config(&hw_post_pipe_top->hw,
			hw_cfg->data);
	hw_post_pipe_top->ops.rc_config(&hw_post_pipe_top->hw,
			hw_cfg->data);
	dpu_pr_debug("-\n");
}

/* config rc enable flag and dpu_region(uefi not use now) */
static void postpq_rc_enable(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg)
{
	if (hw_cfg->value > 0)
		postpq->enabled_features |= BIT(DPU_CRTC_POSTPQ_RC_REGION);
	else
		postpq->enabled_features &= ~BIT(DPU_CRTC_POSTPQ_RC_REGION);
}

typedef void (*postpq_feature_commit_func_t)(struct color_postpq *postpq,
		struct postpq_hw_cfg *hw_cfg);
static postpq_feature_commit_func_t
	postpq_feature_commit_func[DPU_CRTC_POSTPQ_FEATURE_MAX];

#define postpq_feature_commit_funcs_init(func) \
do { \
	func[DPU_CRTC_POSTPQ_USR_GAMMA_EN] = postpq_usr_gamma_enable; \
	func[DPU_CRTC_POSTPQ_USR_GAMMA] = postpq_usr_gamma_commit; \
	func[DPU_CRTC_POSTPQ_3DLUT_EN] = postpq_3dlut_enable; \
	func[DPU_CRTC_POSTPQ_3DLUT] = postpq_3dlut_commit; \
	func[DPU_CRTC_POSTPQ_R2Y_EN] = postpq_r2y_enable; \
	func[DPU_CRTC_POSTPQ_R2Y] = postpq_r2y_commit; \
	func[DPU_CRTC_POSTPQ_SCALER_EN] = postpq_scaler_enable; \
	func[DPU_CRTC_POSTPQ_SCALER] = postpq_scaler_commit; \
	func[DPU_CRTC_POSTPQ_DITHER_LITE_EN] = postpq_dither_lite_enable; \
	func[DPU_CRTC_POSTPQ_Y2R_EN] = postpq_y2r_enable; \
	func[DPU_CRTC_POSTPQ_Y2R] = postpq_y2r_commit; \
	func[DPU_CRTC_POSTPQ_CSC_MATRIX_EN] = postpq_csc_matrix_enable; \
	func[DPU_CRTC_POSTPQ_CSC_MATRIX] = postpq_csc_matrix_commit; \
	func[DPU_CRTC_POSTPQ_DITHER_EN] = postpq_dither_enable; \
	func[DPU_CRTC_POSTPQ_DITHER] = postpq_dither_commit; \
	func[DPU_CRTC_POSTPQ_LTM_EN] = postpq_ltm_enable; \
	func[DPU_CRTC_POSTPQ_LTM] = postpq_ltm_commit; \
	func[DPU_CRTC_POSTPQ_MUL_MATRIX_EN] = postpq_mul_matrix_enable; \
	func[DPU_CRTC_POSTPQ_MUL_MATRIX] = postpq_mul_matrix_commit; \
	func[DPU_CRTC_POSTPQ_ACAD_EN] = postpq_acad_enable; \
	func[DPU_CRTC_POSTPQ_ACAD] = postpq_acad_commit; \
	func[DPU_CRTC_POSTPQ_HIST_READ_MODE] = postpq_hist_enable; \
	func[DPU_CRTC_POSTPQ_HIST] = postpq_hist_commit; \
	func[DPU_CRTC_POSTPQ_RC_REGION] = postpq_rc_enable; \
	func[DPU_CRTC_POSTPQ_RC] = postpq_rc_commit; \
} while (0)

static void dpu_get_ltm_cfg(dpu_ltm_param_t *ltm_param)
{
	dpu_check_and_no_retval(!ltm_param, "ltm_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_LTM].data = &ltm_param->ltm;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_LTM_EN].value = ltm_param->enable;
}

static void dpu_get_acad_cfg(dpu_acad_param_t *acad_param)
{
	dpu_check_and_no_retval(!acad_param, "acad_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_ACAD].data = &acad_param->acad;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_ACAD_EN].value = acad_param->enable;
}

static void dpu_get_mulmatrix_cfg(dpu_matrix_3x4_param_t *mulmatrix_param)
{
	dpu_check_and_no_retval(!mulmatrix_param, "mulmatrix_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_MUL_MATRIX].data = &mulmatrix_param->matrix;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_MUL_MATRIX_EN].value = mulmatrix_param->enable;
}

static void dpu_get_r2y_cfg(dpu_csc_matrix_param_t *csc_matrix)
{
	dpu_check_and_no_retval(!csc_matrix, "csc_matrix is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_R2Y_EN].value = csc_matrix->enable;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_R2Y].data = &csc_matrix->matrix;

}

static void dpu_get_post_scaler_cfg(dpu_scl2d_param_t *scaler_param)
{
	dpu_check_and_no_retval(!scaler_param, "scaler_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_SCALER_EN].value = scaler_param->enable;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_SCALER].data = &scaler_param->scaler;
}

static void dpu_get_y2r_cfg(dpu_csc_matrix_param_t *csc_matrix)
{
	dpu_check_and_no_retval(!csc_matrix, "csc_matrix is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_Y2R_EN].value = csc_matrix->enable;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_Y2R].data = &csc_matrix->matrix;
}

static void dpu_get_lut3d_cfg(dpu_lut3d_param_t *lut3d_param)
{
	dpu_check_and_no_retval(!lut3d_param, "lut3d_param is null!\n");

	dpu_pr_info("lut3d_param->enable = %d\n", lut3d_param->enable);
	g_postpq_payload_array[DPU_CRTC_POSTPQ_3DLUT].data = &lut3d_param->lut3d;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_3DLUT_EN].value = lut3d_param->enable;
}

static void dpu_get_cscmatrix_cfg(dpu_matrix_3x3_param_t *cscmatrix_param)
{
	dpu_check_and_no_retval(!cscmatrix_param, "cscmatrix_param is null!\n");

	dpu_pr_info("cscmatrix_param->enable = %d\n", cscmatrix_param->enable);
	g_postpq_payload_array[DPU_CRTC_POSTPQ_CSC_MATRIX].data = &cscmatrix_param->matrix;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_CSC_MATRIX_EN].value = cscmatrix_param->enable;
}

static void dpu_get_gamma_cfg(dpu_gamma_param_t *gamma_param)
{
	dpu_check_and_no_retval(!gamma_param, "gamma_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_USR_GAMMA].data = &gamma_param->gamma;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_USR_GAMMA_EN].value = gamma_param->enable;
}

static void dpu_get_dither_cfg(dpu_dither_param_t *dither_param)
{
	dpu_check_and_no_retval(!dither_param, "dither_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_DITHER].data = &dither_param->dither;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_DITHER_EN].value = dither_param->enable;
}

static void dpu_get_hist_cfg(dpu_hist_param_t *hist_param)
{
	dpu_check_and_no_retval(!hist_param, "hist_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_HIST].data = &hist_param->hist;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_HIST_READ_MODE].value = hist_param->enable;
}

static void dpu_get_post_rc_cfg(dpu_rc_param_t *rc_param)
{
	dpu_check_and_no_retval(!rc_param, "rc_param is null!\n");

	g_postpq_payload_array[DPU_CRTC_POSTPQ_RC].data = &rc_param->rc;
	g_postpq_payload_array[DPU_CRTC_POSTPQ_RC_REGION].value = rc_param->enable;
}


static void postpq_feature_enable(struct composer *comp, struct color_postpq *postpq)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top = postpq->hw_post_pipe_top;
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;
	struct dpu_hw_postpre_proc *pre_proc = postpq->hw_pre_proc;
	struct postpq_feature_ctrl ctrl_info;

	ctrl_info.enabled_features = postpq->enabled_features;
	ctrl_info.lp_enable = false;

	if (postpq_top)
		postpq_top->ops.feature_enable(&postpq_top->hw, &ctrl_info);

	if (pre_proc)
		pre_proc->ops.feature_enable(&pre_proc->hw, &ctrl_info);

	if (hw_post_pipe_top) {
		if (postpq->rc && postpq->rc->enable) {
			hw_post_pipe_top->ops.rc_enable(&hw_post_pipe_top->hw,
					comp->pinfo->xres, comp->pinfo->yres, true);
		} else {
			hw_post_pipe_top->ops.rc_enable(&hw_post_pipe_top->hw,
					comp->pinfo->xres, comp->pinfo->yres, false);
		}
	}
}

static void color_postpq_commit_feature(struct composer *comp,
		struct color_postpq *postpq)
{
	struct postpq_hw_cfg hw_cfg;

	for (int i = 0; i < ARRAY_SIZE(g_postpq_payload_array); i++) {
		dpu_mem_set(&hw_cfg, 0, sizeof(struct postpq_hw_cfg));

		if (!g_postpq_payload_array[i].is_int_value) {
			hw_cfg.len = g_postpq_payload_array[i].len;
			hw_cfg.data = g_postpq_payload_array[i].data;
		} else {
			hw_cfg.value = g_postpq_payload_array[i].value;
		}

		hw_cfg.lm_width = comp->lm_width;
		hw_cfg.lm_height = comp->lm_height;

		hw_cfg.panel_width = comp->pinfo->xres;
		hw_cfg.panel_height = comp->pinfo->yres;

		if (postpq_feature_commit_func[g_postpq_payload_array[i].feature])
			postpq_feature_commit_func[g_postpq_payload_array[i].feature](
					postpq, &hw_cfg);
	}

	postpq_feature_enable(comp, postpq);
}

/* module init funcs define */
static void postpq_top_init(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg)
{
	struct dpu_hw_postpq_top *postpq_top = postpq->hw_pq_top;

	if (!postpq_top) {
		dpu_pr_debug("postpq_top is null\n");
		return;
	}

	postpq_top->ops.init(&postpq_top->hw, cfg);
}

typedef void (*postpq_module_init_func_t)(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg);
static postpq_module_init_func_t postpq_module_init_func[POSTPQ_MODULE_MAX];

#define postpq_module_init_funcs_init(func) \
do { \
	func[POSTPQ_TOP] = postpq_top_init; \
} while (0)

static void color_postpq_module_init(struct color_postpq *postpq,
		struct postpq_hw_init_cfg *cfg)
{
	int i = 0;

	for (i = 0; i < POSTPQ_MODULE_MAX; i++) {
		if (postpq_module_init_func[i])
			postpq_module_init_func[i](postpq, cfg);
	}
}

static void color_postpq_flush(struct color_postpq *postpq)
{
	int i = 0;

	for (i = 0; i < POSTPQ_MODULE_MAX; i++) {
		if (postpq_module_flush_func[i])
			postpq_module_flush_func[i](postpq);
	}
}

int color_postpq_pipe_init(struct composer *comp)
{
	struct color_postpq *postpq = NULL;
	struct postpq_hw_init_cfg cfg;

	dpu_check_and_return(!comp || !comp->pinfo, -1,
			"comp or comp->pinfo is null\n");
	postpq = &comp->postpq;

	cfg.hdisplay = comp->lm_width;
	cfg.vdisplay = comp->lm_height;

	color_postpq_module_init(postpq, &cfg);
	color_postpq_flush(postpq);

	dpu_pr_debug("postpq pipe init, input display wxh = %d x %d\n",
			cfg.hdisplay, cfg.vdisplay);
	return 0;
}

static void color_postpq_update_cfg(struct color_postpq *postpq)
{
	g_postpq_payload_array[DPU_CRTC_POSTPQ_DITHER_LITE_EN].value = postpq->dither_lite_en;

	dpu_get_ltm_cfg(postpq->ltm);
	dpu_get_acad_cfg(postpq->acad);
	dpu_get_mulmatrix_cfg(postpq->mulmatrix);
	dpu_get_r2y_cfg(postpq->r2y);
	dpu_get_post_scaler_cfg(postpq->scaler);
	dpu_get_y2r_cfg(postpq->y2r);
	dpu_get_lut3d_cfg(postpq->lut3d);
	dpu_get_cscmatrix_cfg(postpq->cscmatrix);
	dpu_get_gamma_cfg(postpq->gma);
	dpu_get_dither_cfg(postpq->dither);
	dpu_get_hist_cfg(postpq->hist);
	dpu_get_post_rc_cfg(postpq->rc);
}

static int color_postpq_commit(struct composer *comp)
{
	struct color_postpq *postpq = NULL;

	dpu_check_and_return(!comp, -1, "comp is null\n");
	postpq = &comp->postpq;

	/* update postpq config in uefi */
	color_postpq_update_cfg(postpq);

	color_postpq_commit_feature(comp, postpq);

	color_postpq_flush(postpq);

	return 0;
}

struct color_postpq_funcs postpq_funcs = {
	.commit = color_postpq_commit,
	.pipe_init = color_postpq_pipe_init,
};

void dpu_mixer_color_postpq_init(struct color_postpq *postpq)
{
	dpu_check_and_no_retval(!postpq, "postpq is null\n");

	postpq->funcs = &postpq_funcs;

	postpq_feature_commit_funcs_init(postpq_feature_commit_func);
	postpq_module_flush_funcs_init(postpq_module_flush_func);
	postpq_module_init_funcs_init(postpq_module_init_func);
}
