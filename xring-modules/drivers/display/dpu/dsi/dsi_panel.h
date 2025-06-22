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

#ifndef _DSI_PANEL_H_
#define _DSI_PANEL_H_

#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <display/xring_dpu_color.h>
#include <drm/drm_panel.h>
#include <drm/drm_modes.h>
#include "drm/drm_mipi_dsi.h"
#include "dpu_hw_dsc_ops.h"
#include "dsi_common.h"
#include "dpu_hw_dsi.h"
#include "dsi_ctrl.h"

#ifdef MI_DISPLAY_MODIFY
#include "mi_dsi_panel.h"
#endif

#define DSI_TIMING_MAX 32
#define DSI_CMDLIST_WRITE true
#define DSI_BL_LEVEL_MAX 65535
#define DSI_MIPI_DYN_FREQ_MAX (10)
#define DSI_ESD_GPIO_MAX 5

#define PANEL_TIMING_V_TOTAL(timing) (((timing)->vfp) + ((timing)->vpw) + \
		((timing)->vbp) + ((timing)->vdisplay))
#define PANEL_TIMING_H_TOTAL(timing) (((timing)->hfp) + ((timing)->hpw) + \
		((timing)->hbp) + ((timing)->hdisplay))

struct dsi_display;

struct dsi_panel_power_mgr {
	u32 count;
	struct mutex lock;
};

/**
 * dsi_panel_power_mgr_init - init dsi panel power mgr
 * @dsi_panel_power_mgr: pointer of dsi panel power mgr
 *
 * Returns: 0: success, other values failure
 */
int dsi_panel_power_mgr_init(struct dsi_panel_power_mgr **power_mgr);

/**
 * dsi_panel_power_mgr_deinit - deinit dsi panel power mgr
 * @dsi_panel_power_mgr: pointer of dsi panel power mgr
 */
void dsi_panel_power_mgr_deinit(struct dsi_panel_power_mgr *power_mgr);

/**
 * dsi_panel_power_get - power on dsi panel
 * @connector: the drm connector pointer for power control
 */
void dsi_panel_power_get(struct drm_connector *connector);

/**
 * dsi_panel_power_put - power off dsi panel
 * @connector: the drm connector pointer for power control
 */
void dsi_panel_power_put(struct drm_connector *connector);

/**
 * is_dsi_panel_powered_on - whether the dsi panel is powered on
 * @dsi_panel_power_mgr: pointer of dsi panel power mgr
 *
 * Return: true on powered on, false on powered off
 */
bool is_dsi_panel_powered_on(struct dsi_panel_power_mgr *power_mgr);

/**
 * dsi_panel_bl_update_type - backlight setting method
 * @DSI_PANEL_BL_UPDATE_METHOD_CMD: setting backlight by cmd
 * @DSI_PANEL_BL_UPDATE_METHOD_I2C: setting backlight by i2c
 * @DSI_PANEL_BL_UPDATE_METHOD_MAX:  invalid backlight setting method
 */
enum dsi_panel_bl_update_type {
	DSI_PANEL_BL_UPDATE_METHOD_CMD = 0,
	DSI_PANEL_BL_UPDATE_METHOD_I2C,
	DSI_PANEL_BL_UPDATE_METHOD_MAX,
};

/**
 * dsi_panel_type - panel physical type
 * @DSI_PANEL_TYPE_LCD:  LED panel
 * @DSI_PANEL_TYPE_LTPS: LTPS panel
 * @DSI_PANEL_TYPE_LTPO: LTPO panel
 * @DSI_PANEL_TYPE_MAX:  invalid panel physical type
 */
enum dsi_panel_type {
	DSI_PANEL_TYPE_LCD = 0,
	DSI_PANEL_TYPE_LTPS,
	DSI_PANEL_TYPE_LTPO,
	DSI_PANEL_TYPE_MAX,
};

/**
 * dsi_panel_timing - the struct of panel timing
 * @framerate: vertical refresh rate
 * @te_rate: ddic te rate
 * @base_rate: ddic base rate
 * @hdisplay: horizontal display size
 * @hfp: horizontal fron porch in pixels
 * @hpw: horizontal pulse width
 * @hbp: horizontal back porch in pixels
 * @h_skew: horizontal skew
 * @vdisplay: vertical display size
 * @vfp: vertical fron porch in pixels
 * @vpw: vertical pulse width
 * @vbp: vertical back porch in pixels.
 */
struct dsi_panel_timing {
	u32 framerate;
	u32 te_rate;
	u32 base_rate;

	u32 hdisplay;
	u32 hfp;
	u32 hpw;
	u32 hbp;
	u32 h_skew;

	u32 vdisplay;
	u32 vfp;
	u32 vpw;
	u32 vbp;
};

struct dsi_panel_flat_mode_info {
	u32 mipi_addr;
	u32 flat_on_gamma;
	u32 flat_off_gamma;
};

/**
 * dsi_panel_mode - display mode for dsi display
 * @index: the mode index
 * @cur_timing: pointer to the timing being used
 * @timing: timing parameters for the panel.
 * @timing_baks_num: the size of timing_baks array
 * @timing_baks: timing parameters array for MIPI dynamic frequency
 * @cmd_sets: array of cmd set
 * @flat_mode_infos: the gamma cfg for flat mode
 * @vrr_type: the vrr type of this mode
 * @skip_frame_num: the skip frame num of vrr, only valid in skip-te mode
 * @frame_duration_ms: duration time per frame in ms
 * @frame_timeout_ms: the maximium frame duration in ms
 * @te_duration_ms: the te duration in ms
 * @is_flattened_mode: the flattened mode share cmd_sets memory
 * with the source mode
 */
struct dsi_panel_mode {
	u32 index;
	struct dsi_panel_timing *cur_timing;
	struct dsi_panel_timing timing;
	u32 timing_baks_num;
	struct dsi_panel_timing timing_baks[DSI_MIPI_DYN_FREQ_MAX];
	struct dsi_cmd_set cmd_sets[DSI_CMD_SET_MAX];
	struct dsi_panel_flat_mode_info *flat_mode_infos;
	u32 flat_mode_info_num;
	enum dsi_vrr_type vrr_type;
	u32 skip_frame_num;
	u32 frame_duration_ms;
	u32 frame_timeout_ms;
	u32 te_duration_ms;
	bool is_flattened_mode;
#ifdef MI_DISPLAY_MODIFY
	struct mi_mode_info mi_timing;
#endif
};

/**
 * dsi_panel_power_state - the node structure of dsi gpio sequence
 * @level: 0-pull down  1-pull up
 * @delay_ms: the delay after this state
 */
struct dsi_panel_power_state {
	u32 level;
	u32 delay_ms;
};

/**
 * reset_ctrl - the structure of gpio ctrl
 * @gpio: the gpio number
 * @state_sequence: the gpio sequence
 * @state_num: the state number of gpio sequence
 * @delay_ms_before_reset_off: the delay before panel power off
 */
struct reset_ctrl {
	int gpio;
	struct dsi_panel_power_state *state_sequence;
	u32 state_num;
	u32 delay_ms_before_reset_off;
	bool reset_after_lp11;
};

struct dsi_panel_pinctrl_info {
	struct pinctrl *pinctrl_ptr;
	struct pinctrl_state *work_state;
	struct pinctrl_state *sleep_state;
	struct pinctrl_state *te_work_state;
	struct pinctrl_state *te_gpio_state;
};

enum dsi_panel_power_type {
	DSI_PANEL_POWER_TYPE_INVALID = 0,
	DSI_PANEL_POWER_TYPE_GPIO,
	DSI_PANEL_POWER_TYPE_LDO,
};

struct dsi_panel_power_supply {
	const char *name;
	enum dsi_panel_power_type power_type;
	u32 gpio_id;
	struct regulator *regulator;
	struct dsi_panel_power_state on_state;
	struct dsi_panel_power_state off_state;
};

/**
 * dsi_panel_power_seq - the panel power sequence
 * @supply_info_num: num of panel power supply info
 * @power_supply_infos: panel power supply info
 */
struct dsi_panel_power_seq {
	u32 supply_info_num;
	struct dsi_panel_power_supply *power_supply_infos;
};

/**
 * phy_info - the parameters of dsi controller and phy
 * @phy_type: panel c-phy or d-phy selsect
 * @clk_type: phy hs clock is continuous or non-continuous
 * @lane_num: dsi lane number
 * @cur_lane_rate: the lane_rate being used
 * @lane_rate: hs speed, bps
 * @lane_rate_baks_num: the size of hs speed bps baks array
 * @lane_rate_baks: hs speed bps array for MIPI dynamic requency
 * @sys_clk_rate: mipi system input clock
 * @phy_lp_speed: phy send speed by lp mode
 * @ppi_width: phy ppi width
 * @phy_amplitude: set phy amplitude
 * @phy_oa_setr: for phy eq adjust reference parameters
 * @phy_eqa: for phy eq adjust
 * @phy_eqb: for phy eq adjust
 */
struct phy_info {
	enum dsi_phy_type phy_type;
	enum dsi_phy_clk_type clk_type;
	enum dsi_phy_lane lane_num;
	u32 cur_lane_rate;
	u32 lane_rate;
	u32 lane_rate_baks_num;
	u32 lane_rate_baks[DSI_MIPI_DYN_FREQ_MAX];
	u32 phy_lp_speed;
	u32 sys_clk_rate;
	u32 ppi_width;
	u32 phy_amplitude;
	u32 phy_oa_setr;
	u32 phy_eqa;
	u32 phy_eqb;
};

/**
 * dsi_panel_host_info - the parameters for dsi host
 * @dsc_cfg: panle dsc config
 * @phy_info: panel phy
 * @burst_mode_type: only for video mode: burst or non-burst pulses/events
 * @color_fmt: destination color format
 * @ctrl_mode: select video mode or commond mode
 * @virtual_channel: virtual channel
 * @eotp_en: eotp function enable
 * @bta_en: bta function enable
 * @cur_pixel_clk_rate: the pixel clock rate being used
 * @pixel_clk_rate: the pixel clock rate
 * @pixel_clk_div: pixel clock divide parameter
 * @pixel_clk_rate_baks_num: the size of the pixel clock rate baks array
 * @pixel_clk_rate_baks: the pixel clock rate for MIPI dynamic frequency
 * @ipi_pll_sel: the pixel clock mode : main or backup
 * @bpp: dsi bit per pixel
 * @bpp: bit per pixel
 * @last_refresh_rate: last frame refresh rate
 * @te_sel: the host TE source
 * @port_mask: mask of panel required dsi port
 * @sbs_en: side-by-side
 */
struct dsi_panel_host_info {
	struct dsc_parms dsc_cfg;
	struct phy_info phy_info;
	enum video_mode_type burst_mode_type;
	enum dsi_pixel_format color_fmt;
	enum dsi_ctrl_mode ctrl_mode;
	u32 virtual_channel;
	u32 eotp_en;
	u32 bta_en;
	u32 cur_pixel_clk_rate;
	u32 pixel_clk_rate;
	u32 pixel_clk_div;
	u32 pixel_clk_rate_baks_num;
	u32 pixel_clk_rate_baks[DSI_MIPI_DYN_FREQ_MAX];
	u32 ipi_pll_sel;
	u32 bpp;
	u32 last_refresh_rate;
	u8 te_sel;
	u8 port_mask;
	bool sbs_en;
};

/**
 * dsi_panel_physical_info - physical characteristics of the screen
 * @type: panel physical type
 * @width_mm: panel physical width
 * @height_mm: panel physical height
 */
struct dsi_panel_physical_info {
	enum dsi_panel_type type;
	u32 width_mm;
	u32 height_mm;
};

/**
 * dsi_backlight_config - backlight configure info
 * @brightness_min_level: min brightness level
 * @brightness_max_level: max brightness level
 * @brightness_init_level: init brightness level
 * @brightness: panel brightness value cache, when the panel is power on,
 * the cached brightness value will be used to restore the panel brightness state.
 * @current_brightness: the current brightness in panel
 */
struct dsi_backlight_config {
	u32 brightness_min_level;
	u32 brightness_max_level;
	u32 brightness_init_level;
	u32 brightness;
	u32 current_brightness;
#ifdef MI_DISPLAY_MODIFY
	bool bl_inverted_dbv;
	u32 actual_brightness_max_level;
#endif
};

/**
 * dsi_panel_esd_mode - the esd mode type
 * @DSI_PANEL_ESD_MODE_ERR_INT: esd check by gpio interruption
 * @DSI_PANEL_ESD_MODE_REG_READ: esd check by read panel regist
 * @DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ: esd check by gpio interruption & read regist
 * @DSI_PANEL_ESD_MODE_PANEL_TE: esd check by panel te irq
 * @DSI_PANEL_ESD_MODE_SW_SUCCESS: for debugfs, always return esd check success
 * @DSI_PANEL_ESD_MODE_SW_FAILURE: for debugfs, always return esd check falied
 */
enum dsi_panel_esd_mode {
	DSI_PANEL_ESD_MODE_ERR_INT,
	DSI_PANEL_ESD_MODE_REG_READ,
	DSI_PANEL_ESD_MODE_ERR_INT_AND_REG_READ,
	DSI_PANEL_ESD_MODE_PANEL_TE,
	DSI_PANEL_ESD_MODE_SW_SUCCESS,
	DSI_PANEL_ESD_MODE_SW_FAILURE,
	DSI_PANEL_ESD_MODE_MAX,
};

/**
 * dsi_panel_esd_info - the panel esd info
 * @enable: esd check enable
 * @esd_mode: esd check mode
 * @err_irq_gpio_name: name of the err gpio
 * @err_irq_gpio: the err irq gpio id
 * @err_irq_gpio_flag: the flag of err irq gpio
 * @err_irq_gpio_num: the number of err irq gpio
 * @err_irq: the esd check err irq id
 * @err_irq_enabled: err irq enabled
 * @dsi_cmd_set: the cmd set of read panel reg
 * @status_length: the read back panel status lenght
 * @status_length_total: the total read back status length
 * @status_value:  the normal panel status value
 * @status_buf: the read back result buffer
 * @period: the period of read panel status reg
 * @te_gpio: the panel te gpio
 */
struct dsi_panel_esd_info {
	bool enable;
	enum dsi_panel_esd_mode esd_mode;
	/* err irq */
	const char *err_irq_gpio_name[DSI_ESD_GPIO_MAX];
	int err_irq_gpio[DSI_ESD_GPIO_MAX];
	int err_irq_gpio_flag[DSI_ESD_GPIO_MAX];
	int err_irq_gpio_num;
	int err_irq[DSI_ESD_GPIO_MAX];
	bool err_irq_enabled;
	/* status read */
	struct dsi_cmd_set panel_status_cmd;
	u32 *status_length;
	u32 status_length_total;
	u32 *status_value;
	u8 *status_buf;
	u32 period;
	/* check te */
	int te_gpio;
};


/**
 * dsi_cmd_queue_node - the dsi cmd queue node
 * @cmd_set: the pointer of cmd set
 * @transfer_type: USE_CPU or USE_CMDLIST
 * @num_delayed_frame: number of frame that this cmd set
 * detained in priority queue
 */
struct dsi_cmd_queue_node {
	struct dsi_cmd_set *cmd_set;
	u8 transfer_type;
	u8 num_delayed_frame;
};

/**
 * dsi_cmd_mgr - the dsi cmd manager
 * @num_cmd_set: number cmd set stashed in cmd mgr queue
 * @header_size: the total header size in queue
 * @payload_size: the total payload size in queue
 * @transfered_payload_size: the transfered payload size in this frame
 * @transfered_header_size: the transfered header size in this frame
 * @queue: the buffer for stashed cmd set
 * @front: the front pointer for queue
 * @rear: the rear pointer for queue
 * @panel: the phandle of panel
 * @cmd_mgr_lock: mutex lock for cmd mgr
 */
struct dsi_cmd_mgr {
	u32 num_cmd_set;
	u32 header_size;
	u32 payload_size;
	u32 transfered_payload_size;
	u32 transfered_header_size;
	struct dsi_cmd_queue_node queue[DSI_HDR_MEM_MAX];
	u8 front;
	u8 rear;
	struct dsi_panel *panel;
	struct mutex cmd_mgr_lock;
};

/**
 * dsi_panel_info - panel info from panel dtsi file
 * @name: the panel name
 * @panel_id: for panel and mipi connector
 * @boot_up_profile: the dpu boot up profile level
 * @lowpower_ctrl: the panel low power caps, bits define see enum DPU_LP_CTRL
 * @idle_policy: bit mask of idle policy
 * @frame_power_mask: the panel supported frame power ctrl partition mask
 * @frame_power_max_refresh_rate: the max refresh rate enabling frame power ctrl
 * @hw_ulps_max_refresh_rate: max refresh rate of hw ulps
 * @is_fake_panel: flag for fake panel
 * @is_asic: fpga or asic
 * @is_primary: true for primary panel, otherwise for secdonary panel
 * @support_aod: support aod mode
 * @partial_update_enable: partial update enabled
 * @num_timings: the number of the timing node in this panel
 * @modes: array of panel modes
 * @num_modes: total number of panel modes
 * @current_mode: the pointer of current panel mode
 * @max_fps_mode: mode with the highest fps
 * @physical_info: panel physical info
 * @gpio_info: the gpio number info of this panel
 * @regulator_info: the regulator info of this panel
 * @pinctrl_info: the pinctrl info of this panel
 * @host_info: dsi host config info from panel dtsi
 * @phy_info: phy config info from panel dtsi
 * @bl_config: backlight configure info
 * @power_seq: the panel power on/off sequence
 * @reset_ctrl: the panel reset sequence
 * @esd_info: panel esd status info
 * @reset_after_lp11: reset panel after lp11
 */
struct dsi_panel_info {
	const char *name;
	u32 panel_id;
	u32 boot_up_profile;
	u32 lowpower_ctrl;
	u32 idle_policy;
	u32 frame_power_mask;
	u32 frame_power_max_refresh_rate;
	u32 hw_ulps_max_refresh_rate;

	bool is_fake_panel;
	bool is_asic;
	bool vrr_enable;
	bool is_primary;
	bool support_aod;
	bool partial_update_enable;

	u32 num_timings;
	u32 num_modes;
	struct dsi_panel_mode *modes;
	struct dsi_panel_mode *current_mode;
	struct dsi_panel_mode *old_mode;
	struct dsi_panel_mode *max_fps_mode;
	struct dsi_panel_physical_info physical_info;
	struct dsi_panel_pinctrl_info pinctrl_info;
	struct dsi_panel_host_info host_info;
	enum dsi_panel_bl_update_type bl_setting_type;
	struct dsi_panel_hdr_caps hdr_info;
	struct dsi_backlight_config bl_config;
	struct dsi_panel_power_seq power_seq;
	struct reset_ctrl reset_ctrl;
	struct dsi_panel_esd_info esd_info;
	int te_scanline;
};

/**
 * dsi_panel - dsi panel struct
 * @drm_panel: drm panel
 * @mipi_device: mipi device
 * @panel_lock: panel lock
 * @host: the mipi dsi host hanle
 * @bl_device: backlight device pointer
 * @panel_info: info from panel dtsi file
 * @initialized: the panel initialize status
 * @panel_dead: indicate the panel still alive
 * @panel_recovery_pending: panel in recovery process
 */
struct dsi_panel {
	struct drm_panel drm_panel;
	struct mipi_dsi_device mipi_device;

	struct mutex panel_lock;
	struct mipi_dsi_host *host;

	struct backlight_device *bl_device;

	struct dsi_panel_info panel_info;
	bool initialized;
	bool panel_dead;
	bool cmd_mgr_enabled;
	atomic_t panel_recovery_pending;

#ifdef MI_DISPLAY_MODIFY
	struct mi_dsi_panel mi_panel;
#endif
	struct dsi_display *display;
};

inline bool dsi_cmd_type_is_read(u8 type);
u32 dsi_basic_cmd_flag_get(struct dsi_cmd_desc *cmd, u8 transfer_type, u32 cur_index, u32 num_cmds);

struct dsi_cmd_set *dsi_cmd_set_duplicate(struct dsi_cmd_set *cmd_set);
void dsi_cmd_set_delete(struct dsi_cmd_set *cmd_set);

int dsi_cmd_mgr_push(struct dsi_cmd_set *cmd_set, u8 transfer_type);
int dsi_cmd_mgr_flush(void);
int dsi_cmd_mgr_quota_update(struct dsi_cmd_set *cmd_set);
void dsi_cmd_mgr_quota_reset(void);

const char *dsi_panel_cmd_set_name_get(int type);

/**
 * dsi_panel_initialized - Check if the screen has been initialized.
 * @panel: The dsi panel phandle.
 *
 * Return: true on initialized or a false on uninitialized.
 */
bool dsi_panel_initialized(struct dsi_panel *panel);

/**
 * dsi_panel_timing_switch - Switch the panel timing.
 * @panel: The dsi panel phandle.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_timing_switch(struct dsi_panel *panel);

/**
 * dsi_panel_post_timing_switch - Post switch the panel timing.
 * @panel: The dsi panel phandle.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_post_timing_switch(struct dsi_panel *panel);

/**
 * dsi_panel_cmd_set_update - Update the cmd set tx buf by cmd set update info.
 * @cmd_set: The cmd set pointer
 * @info_index: The index of update info
 * @data_buf: Data buffer
 * @data_len: The length of data buffer
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_cmd_set_update(struct dsi_cmd_set *cmd_set,
		u8 info_index, u8 *data_buf, u32 data_len);

/**
 * dsi_tmg_create - create a dsi panel instance
 * @dev_node: the phandle of dsi panel device node
 *
 * Return: dsi panel pointer on success, error pointer on failure.
 */
struct dsi_panel *dsi_panel_create(struct device_node *dev_node, bool is_asic, const char *type);

/**
 * dsi_panel_destroy - destroy a panel instance and release reserved resources.
 * @dsi_panel: the dsi panel phandle
 */
void dsi_panel_destroy(struct dsi_panel *dsi_panel);

/**
 * dsi_panel_get_drm_mode - get the drm mode by panel mode
 * @panel: the dsi panel phandle
 * @index: the panel mode index
 * @drm_mode: the output result
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_get_drm_mode(struct dsi_panel *panel, u32 index,
		struct drm_display_mode *drm_mode);

/**
 * dsi_panel_modes_destroy - destroy panel modes
 * @panel: the dsi panel phandle
 */
void dsi_panel_modes_destroy(struct dsi_panel *panel);

/**
 * dsi_panel_mode_validate - validate the drm mode
 * @panel: the dsi panel phandle
 * @drm_mode: the drm mode tobe validated
 *
 * Return: 0 on valid or a negative number on invalid.
 */
int dsi_panel_mode_validate(struct dsi_panel *panel,
		struct drm_display_mode *drm_mode);

/**
 * dsi_panel_mode_search - search panel mode by drm mode
 * @panel: the dsi panel phandle
 * @drm_mode: the drm mode pointer
 * @result: the searched result
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_mode_search(struct dsi_panel *panel,
		struct drm_display_mode *drm_mode,
		struct dsi_panel_mode **result);

/**
 * dsi_panel_timing_dump - dump panel timging
 * @panel_timing: the panel timing tobe dumped.
 */
void dsi_panel_timing_dump(struct dsi_panel_timing *panel_timing);

/**
 * dsi_panel_cmd_set_send - send specific cmd set to panel
 * @panel: the dsi panel phandle
 * @type: send cmd set according to the given type,
 * only invalid when the cmd_set pointer is null.
 * @cmd_set: send this cmd set
 * @transfer_type: use cpu or cmdlist, only valid in cmd send.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_cmd_set_send(struct dsi_panel *panel, enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set, u8 transfer_type);

/**
 * dsi_panel_enable - enable the panel by sending initial code
 * @panel: the dsi panel phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_enable(struct dsi_panel *panel);

/**
 * dsi_panel_disable - disable the panel
 * @panel: the dsi panel phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_disable(struct dsi_panel *panel);

/**
 * dsi_panel_backlight_set_locked - setting the panel backlight level
 * @panel: the dsi panel phandle
 * @brightness: the backlight brightness
 * @transfer_type: use cpu or cmdlist, only valid in cmd send.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_backlight_set_locked(struct dsi_panel *panel,
		u32 brightness, u8 transfer_type);

/**
 * dsi_panel_doze - set panel enter doze.
 * @panel: the dsi panel phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_doze(struct dsi_panel *panel);

/**
 * dsi_panel_doze_suspend - set panel enter doze suspend.
 * @panel: the dsi panel phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_doze_suspend(struct dsi_panel *panel);

/**
 * dsi_panel_exit_lp - set panel exit doze or doze suspend.
 * @panel: the dsi panel phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_exit_lp(struct dsi_panel *panel);

/**
 * dsi_panel_always_on_display - set panel always on
 * @panel: the dsi panel phandle
 * @enable: true - enable aod, false - disable aod
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_always_on_display(struct dsi_panel *panel, bool enable);

/**
 * dsi_panel_reset - reset the dsi panel
 * @panel_info: the dsi panel information
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_reset(struct dsi_panel_info *panel_info);

/**
 * dsi_panel_mipi_freq_update - update mipi frequence parameters
 * @panel: the phandle of dsi panel
 * @freq_id: 0 means master timing;
 * when freq_id >= 1, freq_id - 1 == the index of backup timing
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_mipi_freq_update(struct dsi_panel *panel, u8 freq_id);

/**
 * dsi_panel_esd_irq_ctrl - the switcher of esd irq
 * @panel: the phandle of dsi panel
 * @enabel: the esd irq status that we are expected
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_esd_irq_ctrl(struct dsi_panel *panel, bool enable);

/**
 * dsi_panel_esd_irq_parse - parse the esd irq info
 * @esd_info: the phandle of esd info
 * @np: the phandle of panel device node
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_esd_irq_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np);

/**
 * dsi_panel_esd_reg_read_parse - parse the sed reg read info
 * @esd_info: the phandle of esd info
 * @np: the phandle of panel device node
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_esd_reg_read_parse(struct dsi_panel_esd_info *esd_info,
		struct device_node *np);

/**
 * dsi_panel_esd_attack_trigger - trugger esd attack by pull down the reset GPIO
 * @panel: the phandle of dsi panel
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_esd_attack_trigger(struct dsi_panel *panel);

/**
 * dsi_panel_cmd_single_parse - trigger esd attack by pull down the reset GPIO
 * @cmd_set: the phandle of cmd set
 * @cmd_type: the type of cmd set
 * @np: the phandle of panel device node
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_cmd_single_parse(struct dsi_cmd_set *cmd_set,
		enum dsi_cmd_set_type cmd_type, struct device_node *np);

/**
 * dsi_panel_status_reg_read - read the panel status reg
 * @panel: the phandle of dsi panel
 * @port_index: check panel status in this port
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_panel_status_reg_read(struct dsi_panel *panel, int port_index);

/**
 * dsi_panel_status_value_validate - validate panel status
 * @panel: the phandle of dsi panel
 *
 * Return: true on success or false on failure.
 */
bool dsi_panel_status_value_validate(struct dsi_panel *panel);

inline void dsi_panel_lock(struct dsi_panel *panel);

inline void dsi_panel_unlock(struct dsi_panel *panel);

void dsi_panel_mode_update(struct dsi_panel *panel,
		struct dsi_panel_mode *new_mode);

char *dsi_panel_esd_mode_name_get(int esd_mode);

#ifdef MI_DISPLAY_MODIFY
int dsi_panel_get_cmd_pkt_count(const char *cmd_data, u32 length,
		u32 *pkt_count);

int dsi_panel_create_cmd_packets(const char *data, u32 length,
		u32 count, struct dsi_cmd_desc *cmd);

int dsi_panel_cmds_parse(struct dsi_cmd_set *cmd_set,
		struct device_node *np, const char *cmd_name);
#endif

#endif
