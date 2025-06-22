/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */

#ifndef _MI_DSI_PANEL_H_
#define _MI_DSI_PANEL_H_
#include <linux/types.h>
#include <linux/pm_wakeup.h>

#include "dsi_common.h"
#include "dsi_panel.h"
#include "mi_disp_feature.h"
#include "mi_disp.h"
#include "mi_disp_config.h"

/* ---------------------------- */
/* | 15 - 14 | 13 - 7 | 6 - 0 | */
/* ---------------------------- */
/* |   mode  | sf fps | min fps| */
/* ---------------------------- -*/
/* mode: 1 idle, 2 auto, 3 qsync */
/*   1 << 14 | 120 << 7 | 24     */
/* ---------------------------- */
#define FPS_NORMAL           0
#define FPS_VALUE_MASK       0x7F
#define FPS_SF_FPS_OFFSET    7
#define FPS_MODE_OFFSET      14
#define FPS_MODE_VALUE_MASK  0x3
#define FPS_MODE_IDLE        1
#define FPS_MODE_AUTO        2
#define FPS_MODE_QSYNC       3
#define FPS_COUNT(DDIC_MODE, DDIC_FPS, DDIC_MIN_FPS) ((DDIC_MODE * 100000) + (DDIC_FPS * 100) + DDIC_MIN_FPS)

#define ROUND_CORNER_CMD_DUAL_PORT   0x00
#define ROUND_CORNER_CMD_PORT0       0x01
#define ROUND_CORNER_CMD_PORT1       0x02

#define PMIC_PWRKEY_BARK_TRIGGER 1
#define PMIC_PWRKEY_TRIGGER 2
#define DISPLAY_DELAY_SHUTDOWN_TIME_MS 1800

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

struct dsi_panel;
struct dsi_panel_mode;

enum dbv_Interval {
	DBV_IN19_27 = 0x0,
	DBV_IN28_4B,
	DBV_IN4C_83,
	DBV_MAX
};

enum backlight_dimming_state {
	STATE_NONE,
	STATE_DIM_BLOCK,
	STATE_DIM_RESTORE,
	STATE_ALL
};

enum panel_state {
	PANEL_STATE_OFF = 0,
	PANEL_STATE_ON,
	PANEL_STATE_DOZE_HIGH,
	PANEL_STATE_DOZE_LOW,
	PANEL_STATE_MAX,
};

enum hal_power_state {
	HAL_POWERMODE_OFF = 0,       /* hal::PowerMode::OFF */
	HAL_POWERMODE_ON,            /* hal::PowerMode::ON */
	HAL_POWERMODE_DOZE,          /* hal::PowerMode::DOZE */
	HAL_POWERMODE_DOZE_SUSPEND,  /* hal::PowerMode::DOZE_SUSPEND */
	HAL_POWERMODE_MAX,
};

enum dc_lut_state {
	DC_LUT_60HZ,
	DC_LUT_120HZ,
	DC_LUT_MAX
};

enum dc_feature_type {
	TYPE_NONE = 0,
	TYPE_CRC_SKIP_BL
};

/* Enter/Exit DC_LUT info */
struct dc_lut_cfg {
	bool update_done;
	u8 enter_dc_lut[DC_LUT_MAX][75];
	u8 exit_dc_lut[DC_LUT_MAX][75];
};

struct flat_mode_cfg {
	bool update_done;
	int cur_flat_state; /*only use when flat cmd need sync with te*/
	u8 flat_on_data[4];
	u8 flat_off_data[4];
};

struct panel_demura_info {
	enum dsi_cmd_set_type demura_type; /*ddic demura type*/
	int demura_change_dbv_threshold;
};

struct mi_dsi_panel_cfg {
	struct dsi_panel *dsi_panel;

	/* xiaomi panel id */
	u64 mi_panel_id;

	/* xiaomi feature values */
	int feature_val[DISP_FEATURE_MAX];

	/* indicate esd check gpio and config irq */
	int esd_err_irq_gpio;
	int esd_err_irq;
	int esd_err_irq_flags;
	bool esd_err_enabled;
	bool panel_build_id_read_needed;

	/* brightness control */
	atomic_t last_bl_level;
	u32 last_no_zero_bl_level;
	atomic_t brightness_clone;
	u32 max_brightness_clone;
	u32 bl_normal_max;
	bool need_restore_last_bl;

	/* AOD control */
	u32 doze_brightness;
	u32 last_doze_brightness;
	u32 fullscreen_aod_status;
	struct mutex doze_lock;
	struct wakeup_source *disp_wakelock;
	int doze_hbm_dbv_level;
	int doze_lbm_dbv_level;
	bool aod_status;
	bool hwc_set_doze_brightness;
	bool xeq_enabled;

	/* DDIC round corner */
	bool ddic_round_corner_enabled;
	/* DDIC round corner port mode */
	int ddic_round_corner_port_mode;

	/* DC */
	bool dc_feature_enable;
	bool dc_update_flag;
	enum dc_feature_type dc_type;
	u32 dc_threshold;
	struct dc_lut_cfg dc_cfg;
	u32 real_dc_state;
	/*dbi*/
	int real_dbi_state;

	/* flat mode */
	bool flatmode_default_on_enabled;
	bool flat_sync_te;
	bool flat_update_flag;
	struct flat_mode_cfg flat_cfg;

	/* peak hdr */
	bool is_peak_hdr;

    /* record the last refresh_rate */
	u32 last_refresh_rate;
	u32 last_fps_mode;

	/* Dimming */
	u32 panel_on_dimming_delay;
	u32 dimming_state;
	bool disable_ic_dimming;
	int ic_dimming_dbv_threshold;
	u32 ic_dimming_by_feature;

	/* Panel status */
	int panel_state;

	u8 panel_batch_number;
	bool panel_batch_number_read_done;

	u32 hbm_backlight_threshold;
	bool count_hbm_by_backlight;
	int pmic_pwrkey_status;

	/*DDIC ADD CMD Type*/
	enum dsi_cmd_set_type skip_source_type;
	enum dsi_cmd_set_type dbi_bwg_type;

	/*hbm gamma read*/
	bool aod_enter_flags;
	bool aod_exit_flags;

	struct panel_demura_info demura_info;

	/* addtional panel timming setting */
	u32 reset_post_off_delay;

	/* fps change need notify clients flag */
	bool fps_change_notify_enabled;

	/* fp unlock status */
	u32 fp_unlock_value;

	/* timestamp for DFS backlight zero */
	u64 timestamp_backlight_zero;
	bool whether_backlight_zero;

	/* sf power mode */
	int sf_transfer_power_mode;
};

struct panel_batch_info {
	u8 batch_number;       /* Panel batch number */
	char *batch_name;      /* Panel batch name */
};

struct drm_panel_build_id_config {
	struct dsi_cmd_set id_cmd;
	struct dsi_cmd_set sub_id_cmd;
	u32 id_cmds_rlen;
	u8 build_id;
};

struct drm_panel_wp_config {
	struct dsi_cmd_set pre_tx_cmd;
	struct dsi_cmd_set wp_cmd;
	u32 wp_read_info_index;
	u32 wp_cmds_rlen;
	u8 *return_buf;
};

struct drm_panel_cell_id_config {
	struct dsi_cmd_set pre_tx_cmd;
	struct dsi_cmd_set cell_id_cmd;
	struct dsi_cmd_set after_tx_cmd;
	u32 cell_id_read_info_index;
	u32 cell_id_cmds_rlen;
	u8 *return_buf;
};

enum dsi_cmd_set_upate_type {
	DSI_CMD_UPDATE_MAX
};

struct mi_dsi_panel {
	const char *type;
	struct mi_dsi_panel_cfg mi_cfg;
	bool pending_backlight_by_qsync;
	struct drm_panel_build_id_config id_config;
	struct drm_panel_wp_config wp_config;
	struct drm_panel_cell_id_config cell_id_config;
};

extern const char *cmd_set_names[DSI_CMD_SET_MAX];

int mi_dsi_panel_init(struct dsi_panel *panel);
int mi_dsi_panel_deinit(struct dsi_panel *panel);

int mi_dsi_acquire_wakelock(struct dsi_panel *panel);
int mi_dsi_release_wakelock(struct dsi_panel *panel);

bool is_aod_and_panel_initialized(struct dsi_panel *panel);

bool is_partial_aod_and_panel_initialized(struct dsi_panel *panel);

bool is_backlight_set_skip(struct dsi_panel *panel, u32 bl_lvl);

void mi_dsi_panel_update_last_bl_level(struct dsi_panel *panel,
			int brightness);

int mi_dsi_panel_backlight_prepare(struct dsi_panel *panel,
		u32 brightness);

int mi_dsi_panel_esd_irq_ctrl(struct dsi_panel *panel,
			bool enable);

int mi_dsi_panel_esd_irq_ctrl_locked(struct dsi_panel *panel,
			bool enable);

int mi_dsi_print_51_backlight_log(struct dsi_panel *panel,
			struct dsi_cmd_desc *cmd);

int mi_dsi_panel_parse_sub_timing(struct mi_mode_info *mode,
			struct device_node *np);

int mi_dsi_panel_write_cmd_set(struct dsi_panel *panel,
			struct dsi_cmd_set *cmd_sets);

int mi_dsi_panel_read_batch_number(struct dsi_panel *panel);

bool mi_dsi_panel_need_tx_or_rx_cmd(u32 feature_id);

int mi_dsi_panel_set_disp_param(struct dsi_panel *panel,
			struct disp_feature_ctl *ctl);

int mi_dsi_panel_get_disp_param(struct dsi_panel *panel,
			struct disp_feature_ctl *ctl);

ssize_t mi_dsi_panel_show_disp_param(struct dsi_panel *panel,
			char *buf, size_t size);

int mi_dsi_panel_set_doze_brightness(struct dsi_panel *panel,
			u32 doze_brightness, u8 transfer_type);

int mi_dsi_panel_set_doze_brightness_lock(struct dsi_panel *panel,
			u32 doze_brightness, u8 transfer_type);

int mi_dsi_panel_get_doze_brightness(struct dsi_panel *panel,
			u32 *doze_brightness);

int mi_dsi_panel_get_brightness(struct dsi_panel *panel,
			u32 *brightness);

int mi_dsi_panel_write_dsi_cmd(struct dsi_panel *panel,
			struct dsi_cmd_rw_ctl *ctl);

int mi_dsi_panel_write_dsi_cmd_set(struct dsi_panel *panel, int type);

ssize_t mi_dsi_panel_show_dsi_cmd_set_type(struct dsi_panel *panel,
			char *buf, size_t size);

int mi_dsi_panel_set_brightness_clone(struct dsi_panel *panel,
			u32 brightness_clone);

int mi_dsi_panel_get_brightness_clone(struct dsi_panel *panel,
			u32 *brightness_clone);

int mi_dsi_panel_get_max_brightness_clone(struct dsi_panel *panel,
			u32 *max_brightness_clone);

int mi_dsi_panel_set_dc_mode(struct dsi_panel *panel, bool enable, bool is_possible_in_suspend);

int mi_dsi_panel_set_dc_mode_locked(struct dsi_panel *panel, bool enable, bool is_possible_in_suspend);

int mi_dsi_panel_set_ltmp_cmpst_locked(struct dsi_panel *panel, bool enable);

int mi_dsi_panel_set_round_corner_locked(struct dsi_panel *panel,
			bool enable);

int mi_dsi_panel_set_round_corner(struct dsi_panel *panel,
			bool enable);

int mi_dsi_panel_set_fp_unlock_state(struct dsi_panel *panel,
			u32 fp_unlock_value);

//int mi_dsi_update_flat_mode_on_cmd(struct dsi_panel *panel, enum dsi_cmd_set_type type);

int mi_dsi_update_timing_switch_and_flat_mode_cmd(struct dsi_panel *panel, enum dsi_cmd_set_type type);

int dsi_panel_parse_build_id_read_config(struct dsi_panel *panel);

int mi_dsi_update_51_mipi_cmd(struct dsi_panel *panel, enum dsi_cmd_set_type type, int bl_lvl);

int dsi_panel_parse_wp_reg_read_config(struct dsi_panel *panel);

int dsi_panel_parse_cell_id_read_config(struct dsi_panel *panel);

int mi_dsi_set_switch_cmd_before(struct dsi_panel *panel, int fps_mode);

int mi_dsi_panel_set_doze(struct dsi_panel *panel);

int mi_dsi_panel_set_doze_suspend(struct dsi_panel *panel);

int mi_dsi_panel_exit_lp_locked(struct dsi_panel *panel);

int mi_dsi_panel_set_nolp_locked(struct dsi_panel *panel);

int mi_dsi_panel_update_gamma_param(struct dsi_panel *panel, u32 cmd_update_index,
			enum dsi_cmd_set_type type);

int mi_dsi_panel_set_count_info(struct dsi_panel *panel, struct disp_count_info *count_info);

int mi_dsi_panel_aod_to_normal_optimize_locked(struct dsi_panel *panel, bool enable);

int mi_dsi_update_switch_cmd_O3(struct dsi_panel *panel, u32 cmd_update_index, u32 index);

int mi_dsi_panel_parse_dc_fps_config(struct dsi_panel *panel, struct dsi_panel_mode *mode);

int mi_dsi_panel_demura_set_by_dbv(struct dsi_panel *panel, u32 bl_lvl);

void dsi_panel_dealloc_cmd_packets(struct dsi_cmd_set *set);

void dsi_panel_destroy_cmd_packets(struct dsi_cmd_set *set);

int dsi_panel_cmd_single_parse(struct dsi_cmd_set *cmd_set,
			enum dsi_cmd_set_type cmd_type, struct device_node *np);

int mi_dsi_panel_enable(struct dsi_panel *panel);

int mi_dsi_panel_disable(struct dsi_panel *panel);

int mi_dsi_panel_timing_switch(struct dsi_panel *panel);

int mi_dsi_panel_csc_by_temper_comp(struct dsi_panel *panel, int temp_val, bool is_possible_in_suspend);

int mi_dsi_panel_csc_by_temper_comp_locked(struct dsi_panel *panel, int temp_val, bool is_possible_in_suspend);

#endif /* _MI_DSI_PANEL_H_ */
