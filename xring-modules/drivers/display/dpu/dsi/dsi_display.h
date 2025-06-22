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

#ifndef _DSI_DISPLAY_H_
#define _DSI_DISPLAY_H_

#include "dsi_common.h"
#include "dsi_panel.h"
#include "dsi_ctrl.h"
#include "dsi_phy.h"
#include "dsi_tmg.h"
#include "dpu_flow_ctrl.h"

#include <linux/spinlock.h>

#define display_for_each_port(index, display) \
	for (index = 0; (index < display->port_count) &&\
			(index < DSI_PORT_MAX); index++)

#define display_for_each_active_port(index, display) \
	for (index = 0; (index < display->port_count) &&\
			(index < DSI_PORT_MAX); index++) \
		for_each_if(display->port[index].enabled == true)

/**
 * dsi_display_info  - the display info, only initialized once during driver
 * registration and can't be updated.
 * @dsc_en: is dsc enabled
 * @is_dual_port: is dual mipi panel
 * @dsi_id: indicate current used dsi-0 or dsi-1
 * @hdisplay: horizontal resolution
 * @vdisplay: vertical resolution
 * @tmg_mask: the mask of enabled timing engine
 * @pixel_clk_rate: the pixel_clk_rate
 * @ipi_pll_sel: the pixel clock mode : main or backup
 * @ctrl_mode: the dsi ctrl mode
 * @boot_up_profile: the dpu boot up profile level
 * @lowpower_ctrl: the panel low power caps, bits define see enum DPU_LP_CTRL
 * @idle_policy: bit mask of idle policy
 * @frame_power_mask: the panel supported frame power ctrl partition mask
 * @frame_power_max_refresh_rate: the max refresh rate enabling frame power ctrl
 * @hw_ulps_max_refresh_rate: max refresh rate of hw ulps
 */
struct dsi_display_info {
	bool dsc_en;
	bool is_dual_port;
	u16 dsi_id;
	u16 hdisplay;
	u16 vdisplay;
	u16 width_mm;
	u16 height_mm;
	u32 tmg_mask;
	unsigned long pixel_clk_rate;
	u8 ipi_pll_sel;
	enum dsi_ctrl_mode ctrl_mode;

	u32 boot_up_profile;
	u32 lowpower_ctrl;
	u32 idle_policy;
	u32 frame_power_mask;
	u32 frame_power_max_refresh_rate;
	u32 hw_ulps_max_refresh_rate;
};

/**
 * dsi_display_port  - dsi ctrl/phy information for the display
 * @ctrl: Handle to the DSI controller device.
 * @phy: Handle to the DSI PHY device.
 * @tmg: phandle of timing engine
 * @dsi_id: the hardware id of this port.
 * @enabled: the status of this port
 */
struct dsi_display_port {
	struct dsi_ctrl *ctrl;
	struct dsi_phy *phy;
	struct dsi_tmg *tmg;
	u32 dsi_id;
	bool enabled;
};

/**
 * dsi_mipi_dyn_freq_state - MIPI dynamic frequency state
 * @DSI_MIPI_DYN_FREQ_POWEROFF: panel power off
 * @DSI_MIPI_DYN_FREQ_POWERON: panel power on
 * @DSI_MIPI_DYN_FREQ_WAIT: wait for dyn freq request
 * @DSI_MIPI_DYN_FREQ_ACCEPTED: begin to prepare
 * @DSI_MIPI_DYN_FREQ_PREPARE: prepare done
 * @DSI_MIPI_DYN_FREQ_TRIG: be triggered
 * @DSI_MIPI_DYN_FREQ_DOING: begin to run
 */
enum dsi_mipi_dyn_freq_state {
	DSI_MIPI_DYN_FREQ_POWEROFF = 0,
	DSI_MIPI_DYN_FREQ_POWERON,
	DSI_MIPI_DYN_FREQ_WAIT,
	DSI_MIPI_DYN_FREQ_ACCEPTED,
	DSI_MIPI_DYN_FREQ_PREPARE,
	DSI_MIPI_DYN_FREQ_TRIG,
	DSI_MIPI_DYN_FREQ_DOING,
};

/**
 * dsi_mipi_dyn_freq - MIPI dynamic frequency context
 * @enable: whether to enable MIPI dynamic frequency function
 * @request_freq_id: notified frequency id
 * @state: MIPI dynamic frequency state
 * @dyn_freq_waitqueue: MIPI dynamic frequency state wait queue head
 * @commit_waitqueue: sw commit state wait queue head
 * @dyn_freq_work: MIPI dynamic frequency work thread
 * @dyn_freq_workqueue: workqueue for MIPI dynamic frequency
 */
struct dsi_mipi_dyn_freq {
	bool enable;
	u8 request_freq_id;
	struct mutex dyn_freq_lock;
	struct mutex commit_lock;
	atomic_t dyn_freq_state;
	atomic_t commit_state;
	atomic_t force_refresh_flag;
	wait_queue_head_t dyn_freq_waitqueue;
	struct work_struct dyn_freq_work;
	struct workqueue_struct *dyn_freq_workqueue;
};

/**
 * dsi_display - the dsi display structure
 * @pdev: the dsi platform device
 * @connector: drm connector phandle
 * @tmg_cfg: the tmg config
 * @port: the port of display for single/dual MIPI
 * @port_count: number of dsi port
 * @is_asic: the hardware platform is fpga or asic
 * @host: mipi dsi host object
 * @panel: the hanle of dsi panel
 * @bl_device: backlight device pointer
 * @display_lock: mutex lock of display obejct
 * @display_type: indicate the display is primary or external
 * @display_info: display info for dpu
 * @dsi_panel_power_mgr: dsi panel power manager
 * @esd_check_work: the delayed work of panel status reg read
 * @te_completion: the completion for panel err irq
 * @esd_te_irq_enabled: te irq enabled
 * @esd_check_enabled: esd check enabled
 */
struct dsi_display {
	struct platform_device *pdev;
	struct drm_connector *connector;

	struct dsi_display_port port[DSI_PORT_MAX];
	struct dsi_tmg_cfg tmg_cfg;
	u32 port_count;
	bool is_asic;

	struct mipi_dsi_host host;
	struct dsi_panel *panel;
	struct backlight_device *bl_device;

	enum dsi_display_type display_type;
	struct dsi_display_info display_info;

	struct dsi_panel_power_mgr *dsi_panel_power_mgr;

	struct dsi_mipi_dyn_freq mipi_dyn_freq;

	struct delayed_work esd_check_work;
	struct completion te_completion;
	bool esd_te_irq_enabled;

	bool initialized;
	struct mutex display_lock;
	bool esd_check_enabled;
	atomic_t mipi_cmd_state;
	spinlock_t mipi_cmd_lock;
	bool has_cmdlist_cmd;
};

static inline bool dsi_display_initialized_get(struct dsi_display *display)
{
	return display->initialized;
}

static inline void dsi_display_initialized_set(struct dsi_display *display, bool initialized)
{
	display->initialized = initialized;
}

/**
 * dsi_display_cmd_set_send - Send specific cmd set to panel (also work on suspend).
 *
 * @panel: the dsi panel phandle
 * @type: Send cmd set according to the given type.
 * This parameter only invalid when the cmd_set pointer is null.
 * @cmd_set: send this cmd set
 * @transfer_type: USE_CPU or USE_CMDLIST.
 *
 * This func is protected by dsi_display_lock and dsi_panel_lock.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_cmd_set_send(struct dsi_display *display,
		enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set,
		u8 transfer_type);

int dsi_display_cmd_set_send_locked(struct dsi_display *display,
		enum dsi_cmd_set_type type,
		struct dsi_cmd_set *cmd_set,
		u8 transfer_type);

/**
 * dsi_display_cmd_transfer - transfer single cmd to panel (also work on suspend),
 * @display: the dsi display phandle
 * @cmd_desc: the cmd phandle
 * @transfer_flag:  Transfer flags used to control some special behaviors during
 *                  the DSI transfer, defined in dsi_common.h.
 *                  It can be a combination of the following flags:
 *                  DSI_CMD_FLAG_EXIT_ULPS
 *                  DSI_CMD_FLAG_ENTER_ULPS
 *                  DSI_CMD_FLAG_GET_MIPI_STATE
 *                  DSI_CMD_FLAG_CLEAR_MIPI_STATE
 *                  DSI_CMD_FLAG_WAIT_CRI_AVAILABLE
 *                  DSI_CMD_FLAG_CRI_HOLD
 *                  DSI_CMD_FLAG_CRI_UNHOLD
 *
 * this func is protected by dsi_display_lock and dsi_panel_lock.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_cmd_transfer(struct dsi_display *display,
		struct dsi_cmd_desc *cmd_desc, u32 transfer_flag);

int dsi_display_cmd_transfer_locked(struct dsi_display *display,
		struct dsi_cmd_desc *cmd_desc, u32 transfer_flag);
/**
 * dsi_display_get - get dsi display pointer
 * @index: the index of dsi display
 *
 * Return: valid pointer on success, NULL on failure
 */
struct dsi_display *dsi_display_get(u8 index);

/**
 * dsi_display_post_init - post init dsi display
 * @dpu_iomem_ptr: the dpu virtual base addr
 *
 * Return 0 if the display has been successful enable, otherwise for failure.
 */
int dsi_display_post_init(void __iomem *dpu_iomem_ptr);

/**
 * dsi_panel_info_get - get panel info
 * @connector: pointer to drm connector
 *
 * Return: panel info pointer on success, error pointer on failure.
 */
struct dsi_panel_info *dsi_panel_info_get(
		struct drm_connector *connector);

/**
 * dsi_display_info_get - get displlay info
 * @connector: pointer to drm connector
 *
 * Return: display info pointer on success, error pointer on failure.
 */
struct dsi_display_info *dsi_display_info_get(
		struct drm_connector *connector);

/**
 * dsi_display_panel_timing_switch - send timing switch cmd set to panel
 * @display: pointer to dsi display
 *
 * Return 0 if the display has been successful enable, otherwise for failure.
 */
int dsi_display_panel_timing_switch(struct dsi_display *display);

/**
 * dsi_display_skipped_te_cnt_calculate - Calculate the skip te num
 * @src_fps: the base fps
 * @dest_fps: the target fps
 * @skip_te: the result
 *
 * Return 0 if the display has been successful enable, otherwise for failure.
 */
int dsi_display_skipped_te_cnt_calculate(u32 src_fps,
		u32 dest_fps, u32 *skip_te);

/**
 * dsi_display_front_porch_calculate - Calculate the new front porch.
 * Note that this function is only available when src_fps >= dest_fps.
 * @src_fps: the base fps
 * @dest_fps: the target fps
 * @src_total: the src total
 * @src_fp: the src front porch
 * @tgt_fp: the result tgt porch
 *
 * Return 0 if the display has been successful enable, otherwise for failure.
 */
int dsi_display_front_porch_calculate(u32 src_fps, u32 dest_fps,
		u32 src_total, u32 src_fp, u32 *tgt_fp);
/**
 * dsi_display_enable - enable the dsi display
 * @display: pointer to dsi display
 *
 * Return 0 if the display has been successful enable, otherwise for failure.
 */
int dsi_display_enable(struct dsi_display *display);

/**
 * dsi_display_disable - disable the dsi display
 * @display: pointer to dsi display
 *
 * Return 0 if the display has been successful disable, otherwise for failure.
 */
int dsi_display_disable(struct dsi_display *display);

/**
 * dsi_display_modes_get - get drm_display_mode from panel and probe it to drm
 * @connector: drm connector
 * @display: dsi display object pointer
 *
 * Returns the count of successful probed drm mode.
 */
int dsi_display_modes_get(struct drm_connector *connector,
		struct dsi_display *display);

/**
 * dsi_display_modes_put - release the mode resource
 * @display: dsi display object pointer
 */
void dsi_display_modes_put(struct dsi_display *display);

/**
 * dsi_display_mode_set - set current panel mode
 * @display: dsi display object pointer
 * @mode: the drm mode tobe setted
 * @adjusted_mode: the adjusted drm mode tobe setted
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_mode_set(struct dsi_display *display,
		struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode);

int dsi_display_mode_validate(struct dsi_display *display,
		struct drm_display_mode *mode);

/**
 * dsi_display_ctrl_cfg_get - get dsi ctrl configuration from current panel mode
 * @display: dsi display object pointer
 * @ctrl_cfg: the output config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_ctrl_cfg_get(struct dsi_display *display,
		struct dsi_ctrl_cfg *ctrl_cfg);

/**
 * dsi_display_phy_cfg_get - get dsi phy configuration from current panel mode
 * @display: dsi display object pointer
 * @phy_cfg: the output config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_phy_cfg_get(struct dsi_display *display,
		struct dsi_phy_cfg *phy_cfg);

/**
 * dsi_display_tmg_cfg_init - get dsi tmg configuration from current panel mode
 * @display: dsi display object pointer
 * @tmg_cfg: the output config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_tmg_cfg_init(struct dsi_display *display,
		struct dsi_tmg_cfg *tmg_cfg);

/**
 * dsi_display_tmg_cfg_update - update the tmg config of  timing engine
 * @display: dsi display object pointer
 * @transfer_type: USE_CPU or USE_CMDLIST.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_tmg_cfg_update(struct dsi_display *display, u8 transfer_type);

/**
 * dsi_display_backlight_set - set panel backlight level
 * @display: dsi display object pointer
 * @backlight_level: the panel backlight level tobe setted
 *
 * Called in backlight_device_set_brightness and protected by
 * dsi_panel_lock and dsi_display_lock.
 * May blocked in async-backlight senario.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_backlight_set(struct dsi_display *display,
		u32 backlight_level, u8 bl_transfer_type);

/**
 * dsi_display_sleeping_out - set display sleeping out
 * @display: dsi display object pointer
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_sleeping_out(struct dsi_display *display);

int dsi_display_ctrl_mode_get(struct dsi_display *display,
		enum dsi_ctrl_mode *ctrl_mode);

enum {
	EXIT_ULPS = 0,
	ENTER_ULPS = 1,
};

/**
 * dsi_display_ulps_ctrl_by_cmdlist - dsi ulps enter or exit by cmdlist
 * @display: dsi display object pointer
 * @ulps_enable: 0- exit ulps, 1- enter ulps
 * @transfer_type: 0 - ulps enter or exit by cmdlist
 *                1 - ulps enter or exit by cpu
 * @node_id: cmdlist enter or exit node id
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_ulps_ctrl(struct dsi_display *display,
		u8 ulps_enable, u8 transfer_type, s64 node_id);

/**
 * dsi_display_check_ulps_state - check ulps state
 *
 * @display: dsi display object pointer
 * @enter： true - enter, false - exit
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_check_ulps_state(struct dsi_display *display, bool enter);

/**
 * dsi_display_debug - dsi controller and cdphy debug interface
 * @ctrl: dsi ctrl phandle
 * @type: debug function select
 * @parm_cnt: parameters count
 * @parm: debug parameters
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_debug(struct dsi_display *display,
		enum dsi_debug type, u8 parm_cnt, u8 *parm);

/**
 * dsi_display_timestamp_get - get tmg hardware time stamp
 * @display: dsi display object pointer
 *
 * Return: time stamp.
 */
u64 dsi_display_timestamp_get(struct dsi_display *display);

/**
 * dsi_display_damage_area_update - set panel partial update area by dsi
 * @display: dsi display object pointer
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_damage_area_update(struct dsi_display *display);

/**
 * dsi_display_set_power_mode_prepare - update power mode setting to dsi and panel
 * @display: dsi display object pointer
 * @power_mode: One of the following,
 *              DPU_POWER_MODE_ON
 *              DPU_POWER_MODE_DOZE
 *
 * dsi_display_enable: off/suspend -> on/doze
 * dpu_power_prepare_for_encoder: on/doze -> on/doze
 *
 * Returns: 0 - success
 */
int dsi_display_set_power_mode_prepare(struct dsi_display *display, int power_mode);

/**
 * dsi_display_set_power_mode_finish - update power mode setting to dsi and panel
 * @display: dsi display object pointer
 * @power_mode: One of the following,
 *              DPU_POWER_MODE_DOZE_SUSPEND
 *              DPU_POWER_MODE_OFF
 *
 * dsi_display_disable: on/doze -> off/suspend
 * dpu_power_finish_for_encoder: off/suspend -> off/suspend
 *
 * Returns: 0 - success
 */
int dsi_display_set_power_mode_finish(struct dsi_display *display, int new_power_mode);

/**
 * get_dpu_power_mode_name - get power mode string
 * @power_mode: power mode
 *
 * Returns: name string
 */
char *get_dpu_power_mode_name(int power_mode);

/**
 * dsi_display_line_buf_level_cfg - set line buffer level
 * @display: dsi display object pointer
 */
void dsi_display_line_buf_level_cfg(struct dsi_display *display, int port_id);

/**
 * dsi_display_tmg_init - init tmg
 *
 * @display: dsi display object pointer
 * Returns: 0 - success, else for failure
 */
int dsi_display_tmg_init(struct dsi_display *display);

/**
 * dsi_display_tmg_deinit - deinit tmg
 *
 * @display: dsi display object pointer
 * Returns: 0 - success, else for failure
 */
int dsi_display_tmg_deinit(struct dsi_display *display);

/**
 * dsi_display_ctrl_mode_init - enable dsi
 *
 * @display: dsi display object pointer
 * Returns: 0 - success, else for failure
 */
int dsi_display_ctrl_mode_init(struct dsi_display *display);

/*
 * dsi_display_dyn_freq_request - receive mipi dynamic frequence request
 * @display: dsi display object pointer
 * @freq_id: notified frequency id
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_display_dyn_freq_request(struct dsi_display *display, u8 freq_id);

/*
 * dsi_display_dyn_freq_work_trigger - queue mipi dynamic frequence work
 * @display: dsi display object pointer
 */
inline void dsi_display_dyn_freq_work_trigger(struct dsi_display *display);

/*
 * dsi_display_dyn_freq_wait - wait mipi dynamic frequence done
 * @display: dsi display object pointer
 * @msecs: timeout in ms
 */
void dsi_display_dyn_freq_wait(struct drm_crtc_state *crtc_state,
		unsigned int msecs);

/*
 * dsi_display_dyn_freq_task_wakeup - wake sw commit waiting event
 * @display: dsi display object pointer
 */
void dsi_display_dyn_freq_task_wakeup(struct drm_crtc_state *crtc_state);

int dsi_display_esd_init(struct dsi_display *display);

void dsi_display_esd_ctrl_locked(struct dsi_display *display, bool enable);

void dsi_display_esd_ctrl(struct dsi_display *display, bool enable);

int dsi_display_status_check(struct dsi_display *display);

int dsi_display_te_check_timeout(struct dsi_display *display, u32 timeout_ms);

void dsi_display_panel_dead_notify(struct dsi_display *display);

inline void dsi_display_lock(struct dsi_display *display);

inline void dsi_display_unlock(struct dsi_display *display);

static inline void display_frame_timeout_get(struct dsi_display *display,
		u32 *frame_timeout_ms)
{
	struct dsi_panel_mode *old_mode, *new_mode;

	old_mode = display->panel->panel_info.old_mode;
	new_mode = display->panel->panel_info.current_mode;

	*frame_timeout_ms =
			old_mode->frame_timeout_ms > new_mode->frame_timeout_ms ?
			old_mode->frame_timeout_ms : new_mode->frame_timeout_ms;
}

const char *dsi_display_type_name_get(int display_type);

/**
 * dsi_display_cmd_flush - force flush cmd send use cpu
 *
 * @display: dsi display object pointer
 */
void dsi_display_cmd_flush(struct dsi_display *display);

int dsi_display_mipi_cmd_state_get(struct dsi_display *display);

int dsi_display_mipi_cmd_state_clear(struct dsi_display *display);

int dsi_display_finish(struct dsi_display *display);

#endif /* _DSI_DISPLAY_H_ */
