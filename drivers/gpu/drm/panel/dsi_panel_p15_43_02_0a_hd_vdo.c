// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 MediaTek Inc.
 */
#include <linux/backlight.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_modes.h>
#include <linux/delay.h>
#include <drm/drm_connector.h>
#include <drm/drm_device.h>
#include "../mediatek/mediatek_v2/drm_notifier_odm.h"
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>
#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_graph.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#if IS_ENABLED(CONFIG_MIEV)
#include "../mediatek/mediatek_v2/mi_disp/mi_disp_event.h"
#endif
#include "lcd_prox_gesture_keep_high.h"
#define CONFIG_MTK_PANEL_EXT
#if defined(CONFIG_MTK_PANEL_EXT)
#include "../mediatek/mediatek_v2/mtk_panel_ext.h"
#include "../mediatek/mediatek_v2/mtk_drm_graphics_base.h"
#include "../mediatek/mediatek_v2/mtk_log.h"
#endif
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
#include "../mediatek/mediatek_v2/mtk_corner_pattern/mtk_data_hw_roundedpattern.h"
#endif
#include <linux/atomic.h>
#define DATA_RATE1       1242
#define MODE2_FPS       120
#define MODE2_HFP       34
#define MODE2_HSA       4
#define MODE2_HBP       24
#define MODE2_VFP       324
#define MODE2_VSA       2
#define MODE2_VBP       20
#define MODE1_FPS       90
#define MODE1_HFP       34
#define MODE1_HSA       4
#define MODE1_HBP       24
#define MODE1_VFP       972
#define MODE1_VSA       2
#define MODE1_VBP       20
#define MODE0_FPS       60
#define MODE0_HFP       34
#define MODE0_HSA       4
#define MODE0_HBP       24
#define MODE0_VFP       2270
#define MODE0_VSA       2
#define MODE0_VBP       20
#define FRAME_WIDTH       (720)
#define	FRAME_HEIGHT       (1600)

extern bool lcd_reset_keep_high_zeroa;
extern bool prox_lcd_reset_keep_high_zeroa;
extern bool esd_panel_dead;
static struct drm_notifier_data z_notify_data;
static int current_fps = 120;
static int curr_bl1;
static int curr_bl2;
static bool panel_inited = false;
extern int lcd_bias_write_bytes(unsigned char addr, unsigned char value);
struct lcm {
	struct device *dev;
	struct drm_panel panel;
	struct backlight_device *backlight;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *enp_gpio;
	struct gpio_desc *enn_gpio;
	struct mutex panel_lock;
	bool prepared;
	bool enabled;
	unsigned int dynamic_fps;
	const char *panel_info;
	int error;
};
static const char *panel_name = "panel_name=dsi_panel_p15_43_02_0a_hd_vdo";
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[200];
};
static struct lcm *panel_ctx;
#define lcm_dcs_write_seq(ctx, seq...) \
({\
	const u8 d[] = { seq };\
	BUILD_BUG_ON_MSG(ARRAY_SIZE(d) > 64, "DCS sequence too big for stack");\
	lcm_dcs_write(ctx, d, ARRAY_SIZE(d));\
})
#define lcm_dcs_write_seq_static(ctx, seq...) \
({\
	static const u8 d[] = { seq };\
	lcm_dcs_write(ctx, d, ARRAY_SIZE(d));\
})
static inline struct lcm *panel_to_lcm(struct drm_panel *panel)
{
	return container_of(panel, struct lcm, panel);
}
static void lcm_dcs_write(struct lcm *ctx, const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;
	char *addr;
	if (ctx->error < 0)
		return;
	addr = (char *)data;
	if ((int)*addr < 0xB0)
		ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
	else
		ret = mipi_dsi_generic_write(dsi, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %zd writing seq: %ph\n", ret, data);
		ctx->error = ret;
	}
}
#ifdef PANEL_SUPPORT_READBACK
static int lcm_dcs_read(struct lcm *ctx, u8 cmd, void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;
	if (ctx->error < 0)
		return 0;
	ret = mipi_dsi_dcs_read(dsi, cmd, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %d reading dcs seq:(%#x)\n", ret, cmd);
		ctx->error = ret;
	}
	return ret;
}
static void lcm_panel_get_data(struct lcm *ctx)
{
	u8 buffer[3] = {0};
	static int ret;
	if (ret == 0) {
		ret = lcm_dcs_read(ctx,  0x0A, buffer, 1);
		dev_info(ctx->dev, "return %d data(0x%08x) to dsi engine\n",
			 ret, buffer[0] | (buffer[1] << 8));
	}
}
#endif
#define REGFLAG_DELAY       0xFFFC
#define REGFLAG_UDELAY  0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW   0xFFFE
#define REGFLAG_RESET_HIGH  0xFFFF
static struct LCM_setting_table init_setting[] = {
	//PD SCR on pixel dot&subpixel dot=03,pixel column 07,subpixel column 0f
	{0xFF, 01, {0x2A}},
	{0xFB, 01, {0x01}},
	{0x1E, 01, {0x0F}},
	//PWM 16Khz
	{0xFF, 01, {0x23}},
	{0xFB, 01, {0x01}},
	{0x00, 01, {0x80}},
	{0x07, 01, {0x00}},
	{0x08, 01, {0x01}},
	{0x09, 01, {0x0D}},
	//CABC CODE
	{0xFF,01,{0x23}},
	{REGFLAG_DELAY, 1, {}},
	{0xFB,01,{0x01}},
	{0x10,01,{0x00}},
	{0x11,01,{0x00}},
	{0x12,01,{0xB4}},
	{0x15,01,{0xE9}},
	{0x16,01,{0x14}},
	{0x1F,01,{0x15}},
	{0x20,01,{0x17}},
	{0x21,01,{0x19}},
	{0x22,01,{0x1A}},
	{0x29,01,{0x10}},
	{0x2A,01,{0x12}},
	{0x2B,01,{0x12}},
	//UI MODE
	{0x30,01,{0xFF}},
	{0x31,01,{0xFD}},
	{0x32,01,{0xFD}},
	{0x33,01,{0xFC}},
	{0x34,01,{0xFB}},
	{0x35,01,{0xFA}},
	{0x36,01,{0xF9}},
	{0x37,01,{0xF8}},
	{0x38,01,{0xF7}},
	{0x39,01,{0xF6}},
	{0x3A,01,{0xF5}},
	{0x3B,01,{0xF4}},
	{0x3D,01,{0xF3}},
	{0x3F,01,{0xF2}},
	{0x40,01,{0xF1}},
	{0x41,01,{0xEF}},
	//STILL MODE
	{0x45,01,{0xFF}},
	{0x46,01,{0xFD}},
	{0x47,01,{0xFA}},
	{0x48,01,{0xF8}},
	{0x49,01,{0xF6}},
	{0x4A,01,{0xF5}},
	{0x4B,01,{0xF3}},
	{0x4C,01,{0xDB}},
	{0x4D,01,{0xD7}},
	{0x4E,01,{0xCE}},
	{0x4F,01,{0xCD}},
	{0x50,01,{0xCB}},
	{0x51,01,{0xC8}},
	{0x52,01,{0xC0}},
	{0x53,01,{0xAC}},
	{0x54,01,{0xA9}},
	//MOVIE MODE
	{0x58,01,{0xFF}},
	{0x59,01,{0xFD}},
	{0x5A,01,{0xFC}},
	{0x5B,01,{0xF9}},
	{0x5C,01,{0xF7}},
	{0x5D,01,{0xF4}},
	{0x5E,01,{0xF3}},
	{0x5F,01,{0xDF}},
	{0x60,01,{0xD8}},
	{0x61,01,{0xD0}},
	{0x62,01,{0xCB}},
	{0x63,01,{0xC0}},
	{0x64,01,{0xBE}},
	{0x65,01,{0xB6}},
	{0x66,01,{0xB1}},
	{0x67,01,{0xAF}},
	//ERR-FLAG FTE Normal : High, Abnormal : Low
	{0xFF, 01,{0x27}},
	{0xFB, 01,{0x01}},
	{0xD0, 01,{0x31}},
	{0xD1, 01,{0x25}},
	{0xD2, 01,{0x30}},
	{0xD4, 01,{0x08}},
	{0xDE, 01,{0xC3}},
	{0xDF, 01,{0x02}},

	{0xFF, 01,{0x24}},
	{0xFB, 01,{0x01}},
	{0xDB, 01,{0x0F}},
	{0xED, 01,{0xF0}},
	{0xF1, 01,{0x0F}},
	{0xC2, 03,{0x86,0x00,0x02}},

	// {0xFF, 01, {0x10}},
	// {0xFF, 01, {0x01}},
	// {0x5E, 02, {0x00,0x0C}},
	//cabc
	{0xFF, 01, {0x10}},
	{0xFB, 01, {0x01}},
	{0x53, 01, {0x2C}},
	{0x55, 01, {0x00}},
	//setting dimming
	{0x68, 02, {0x02,0x01}},

	{0x11, 01, {0x00}},
	{REGFLAG_DELAY, 100, {}},
	{0x29, 01, {0x00}},
};

static void push_table(struct lcm *ctx, struct LCM_setting_table *table, unsigned int count)
{
	unsigned int i, j;
	unsigned char temp[255] = {0};
	for (i = 0; i < count; i++) {
		unsigned int cmd;
		cmd = table[i].cmd;
		memset(temp, 0, sizeof(temp));
		switch (cmd) {
		case REGFLAG_DELAY:
			msleep(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			temp[0] = cmd;
			for (j = 0; j < table[i].count; j++)
				temp[j+1] = table[i].para_list[j];
			lcm_dcs_write(ctx, temp, table[i].count+1);
		}
	}
}

static int lcm_disable(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);
	if (!ctx->enabled)
		return 0;
	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(ctx->backlight);
	}
	ctx->enabled = false;
	return 0;
}

static int lcm_unprepare(struct drm_panel *panel)
{
	int blank;
	struct lcm *ctx = panel_to_lcm(panel);
	if (!ctx->prepared)
		return 0;
	pr_info("%s: +\n", __func__);
	blank = DRM_BLANK_POWERDOWN;
	z_notify_data.data = &blank;
	drm_notifier_call_chain(DRM_EVENT_BLANK, &z_notify_data);
	pr_err("[XMFP] : %s ++++ blank = DRM_BLANK_POWERDOWN ++++", __func__);
	lcm_dcs_write_seq_static(ctx, 0x28);
	msleep(20);
	ctx->error = 0;
	ctx->prepared = false;
	if (prox_lcd_reset_keep_high_zeroa  && esd_panel_dead == false) {
		pr_info("[LCM]%s:prox_lcd_reset_keep_high_zeroa = %d\n",prox_lcd_reset_keep_high_zeroa);
		pr_info("[LCM]%s:tp_promixity_en\n",__func__);
		return 0;
	}
	if (lcd_reset_keep_high_zeroa && esd_panel_dead == false) {
		pr_info("[LCM]%s:lcd_reset_keep_high_zeroa = %d\n",lcd_reset_keep_high_zeroa);
		lcm_dcs_write_seq_static(ctx, 0x10);
		msleep(120);
		pr_info("[LCM]%s:tp_gesture_en\n",__func__);
		return 0;
	}
	lcm_dcs_write_seq_static(ctx, 0x10);
	msleep(100);
	ctx->enn_gpio =
		devm_gpiod_get(ctx->dev, "enn", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enn_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enn_gpio %ld\n",
			__func__, PTR_ERR(ctx->enn_gpio));
		return PTR_ERR(ctx->enn_gpio);
	}
	gpiod_set_value(ctx->enn_gpio, 0);
	devm_gpiod_put(ctx->dev, ctx->enn_gpio);
	udelay(2000);
	ctx->enp_gpio =
		devm_gpiod_get(ctx->dev, "enp", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enp_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enp_gpio %ld\n",
			__func__, PTR_ERR(ctx->enp_gpio));
		return PTR_ERR(ctx->enp_gpio);
	}
	gpiod_set_value(ctx->enp_gpio, 0);
	devm_gpiod_put(ctx->dev, ctx->enp_gpio);
	udelay(5000);
	pr_info("%s: -\n", __func__);
	return 0;
}

static void lcm_panel_init(struct lcm *ctx)
{
	pr_info("%s: +\n", __func__);
	if (ctx->prepared) {
		pr_info("%s: panel has been prepared, nothing to do!\n", __func__);
		goto err;
	}
	ctx->reset_gpio =
		devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get reset_gpio %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		goto err;
	}
	gpiod_set_value(ctx->reset_gpio, 0);
	udelay(5 * 1000);
	gpiod_set_value(ctx->reset_gpio, 1);
	udelay(5 * 1000);
	gpiod_set_value(ctx->reset_gpio, 0);
	udelay(10 * 1000);
	gpiod_set_value(ctx->reset_gpio, 1);
	udelay(15 * 1000);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);
	push_table(ctx, init_setting,
		sizeof(init_setting) / sizeof(struct LCM_setting_table));
	ctx->prepared = true;
err:
	pr_info("%s: -\n", __func__);
}

static int lcm_prepare(struct drm_panel *panel)
{
	int blank;
	struct lcm *ctx = panel_to_lcm(panel);
	int ret;
	unsigned char cmd = 0x00;
	unsigned char data = 0xFF;
	esd_panel_dead = false;
	pr_info("%s\n", __func__);
	if (ctx->prepared)
		return 0;
	ctx->enp_gpio =
		devm_gpiod_get(ctx->dev, "enp", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enp_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enp_gpio %ld\n",
			__func__, PTR_ERR(ctx->enp_gpio));
		return PTR_ERR(ctx->enp_gpio);
	}
	gpiod_set_value(ctx->enp_gpio, 1);
	devm_gpiod_put(ctx->dev, ctx->enp_gpio);
	/* set AVDD 6.0V*/
	cmd = 0x00;
	data = 0x14;
	ret = lcd_bias_write_bytes(cmd, data);
	if (ret < 0)
		pr_info("nt36528a--kernel--bias----cmd=%0x--i2c write error----\n", cmd);
	else
		pr_info("nt36528a--kernel--bias----cmd=%0x--i2c write success----\n", cmd);
	/* set AVEE -6.0V*/
	cmd = 0x01;
	data = 0x14;
	ret = lcd_bias_write_bytes(cmd, data);
	if (ret < 0)
		pr_info("nt36528a--kernel--bias----cmd=%0x--i2c write error----\n", cmd);
	else
		pr_info("nt36528a--kernel--bias----cmd=%0x--i2c write success----\n", cmd);
	udelay(3000);
	ctx->enn_gpio =
		devm_gpiod_get(ctx->dev, "enn", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enn_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enn_gpio %ld\n",
			__func__, PTR_ERR(ctx->enn_gpio));
		return PTR_ERR(ctx->enn_gpio);
	}
	gpiod_set_value(ctx->enn_gpio, 1);
	devm_gpiod_put(ctx->dev, ctx->enn_gpio);
	udelay(12 * 1000);
	lcm_panel_init(ctx);
	ret = ctx->error;
	if (ret < 0){
		lcm_unprepare(panel);
		pr_info("%s error lcm_unprepare\n", __func__);
	}
	ctx->prepared = true;

	blank = DRM_BLANK_UNBLANK;
	z_notify_data.data = &blank;
	drm_notifier_call_chain(DRM_EVENT_BLANK, &z_notify_data);
	pr_err("[XMFP] : %s ++++ blank = DRM_BLANK_UNBLANK ++++", __func__);
#ifdef PANEL_SUPPORT_READBACK
	lcm_panel_get_data(ctx);
#endif
	return ret;
}

static int lcm_enable(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);
	if (ctx->enabled)
		return 0;
	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(ctx->backlight);
	}
	ctx->enabled = true;
	return 0;
}

static const struct drm_display_mode mode_60hz = {
	.clock = (FRAME_WIDTH + MODE0_HFP + MODE0_HSA + MODE0_HBP)*(FRAME_HEIGHT + MODE0_VFP + MODE0_VSA + MODE0_VBP)*60/1000,//h_total * v_total * fps 60hz; unit:KHz
	//.clock = 144840,//h_total * v_total * fps 60hz
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + MODE0_HFP,
	.hsync_end = FRAME_WIDTH + MODE0_HFP + MODE0_HSA,
	.htotal = FRAME_WIDTH + MODE0_HFP + MODE0_HSA + MODE0_HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE0_VFP,
	.vsync_end = FRAME_HEIGHT + MODE0_VFP + MODE0_VSA,
	.vtotal = FRAME_HEIGHT + MODE0_VFP + MODE0_VSA + MODE0_VBP,
};
static const struct drm_display_mode mode_90hz = {
	.clock = (FRAME_WIDTH + MODE1_HFP + MODE1_HSA + MODE1_HBP)*(FRAME_HEIGHT + MODE1_VFP + MODE1_VSA + MODE1_VBP)*90/1000,//h_total * v_total * fps 90hz
	//.clock = 144815,//h_total * v_total * fps 90hz
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + MODE1_HFP,
	.hsync_end = FRAME_WIDTH + MODE1_HFP + MODE1_HSA,
	.htotal = FRAME_WIDTH + MODE1_HFP + MODE1_HSA + MODE1_HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE1_VFP,
	.vsync_end = FRAME_HEIGHT + MODE1_VFP + MODE1_VSA,
	.vtotal = FRAME_HEIGHT + MODE1_VFP + MODE1_VSA + MODE1_VBP,
};
static const struct drm_display_mode default_mode = {
	.clock = (FRAME_WIDTH + MODE2_HFP + MODE2_HSA + MODE2_HBP)*(FRAME_HEIGHT + MODE2_VFP + MODE2_VSA + MODE2_VBP)*120/1000,//h_total * v_total * fps 120hz
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + MODE2_HFP,
	.hsync_end = FRAME_WIDTH + MODE2_HFP + MODE2_HSA,
	.htotal = FRAME_WIDTH + MODE2_HFP + MODE2_HSA + MODE2_HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE2_VFP,
	.vsync_end = FRAME_HEIGHT + MODE2_VFP + MODE2_VSA,
	.vtotal = FRAME_HEIGHT + MODE2_VFP + MODE2_VSA + MODE2_VBP,
};

#if defined(CONFIG_MTK_PANEL_EXT)
static struct mtk_panel_params ext_params = {
	.pll_clk = DATA_RATE1 / 2,
	.cust_esd_check = 0,
	.esd_check_enable = 0,
	.mi_esd_check_enable = 1,
	.lcm_esd_check_table[0] = {
		.cmd = 0x0a,
		.count = 1,
		.para_list[0] = 0x1c,
	},
	//.is_support_od = true,
	.lcm_color_mode = MTK_DRM_COLOR_MODE_DISPLAY_P3,
	.physical_width_um = 71928,
	.physical_height_um = 159840,
	.data_rate = DATA_RATE1,
	.dyn = {
		.switch_en = 1,
		.pll_clk = 647,
		//.vfp_lp_dyn = 2550,
		.hfp = 70,
		.hbp = 32,
		//.vfp = 1314,
	},
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	.round_corner_en = 1,
	.corner_pattern_height = ROUND_CORNER_H_TOP,
	.corner_pattern_height_bot = ROUND_CORNER_H_BOT,
	.corner_pattern_tp_size_l = sizeof(top_rc_pattern_l),
	.corner_pattern_lt_addr_l = (void *)top_rc_pattern_l,
	.corner_pattern_tp_size_r = sizeof(top_rc_pattern_r),
	.corner_pattern_lt_addr_r = (void *)top_rc_pattern_r,
#endif
};
#endif

struct drm_display_mode *get_mode_by_id(struct drm_connector *connector,
	unsigned int mode)
{
	struct drm_display_mode *m;
	unsigned int i = 0;
	list_for_each_entry(m, &connector->modes, head) {
		if (i == mode)
			return m;
		i++;
	}
	return NULL;
}

static int mtk_panel_ext_param_set(struct drm_panel *panel,
			struct drm_connector *connector, unsigned int mode)
{
	struct mtk_panel_ext *ext = find_panel_ext(panel);
	int ret = 0;
	int dst_fps = 0;
	struct drm_display_mode *m_dst = get_mode_by_id(connector, mode);
	dst_fps = m_dst ? drm_mode_vrefresh(m_dst) : -EINVAL;
	if (dst_fps == MODE0_FPS)
		ext->params = &ext_params;
	else if (dst_fps == MODE1_FPS)
		ext->params = &ext_params;
	else if (dst_fps == MODE2_FPS)
		ext->params = &ext_params;
	else {
			pr_err("%s, dst_fps %d\n", __func__, dst_fps);
			ret = -EINVAL;
		}
	if (!ret)
		current_fps = dst_fps;
	return ret;
}

#define MIN_BL			13	//Normal段输入起点
#define NORMAL_MAX_BL		2047	//normal段最高背光值
#define	MAX_BL			4095	//最高背光值

#define	MIN_REG_VALUE_XL	12	//信利最低寄存器亮度值
#define NORMAL_MAX_REG_XL	2508	//信利noraml段最高寄存器亮度值
#define MAX_REG			3145	//12bit背光寄存器最大值 0C49

static bool bl_first = false;
static int lcm_setbacklight_cmdq(void *dsi, dcs_write_gce cb,
	void *handle, unsigned int level)
{
	char bl_tb[] = {0x51, 0x07, 0xff};
	char bl_tb_cabc[] = {0x5E, 0x00, 0x00};
	pr_info("%s:level xl = %d\n",__func__,level);
	panel_inited = (level > 0);
	if (level == 0) {
		pr_info("%s:level= %d\n",__func__,level);
	} else if (level > 0 && level<= MIN_REG_VALUE_XL) {
		level = MIN_REG_VALUE_XL;
	} else if (level <= NORMAL_MAX_BL)  {
		level = (level - MIN_BL) * (NORMAL_MAX_REG_XL - MIN_REG_VALUE_XL) / (NORMAL_MAX_BL - MIN_BL) + MIN_REG_VALUE_XL;
		pr_info("%s:XL_hbm_off,level= %d\n",__func__,level);
	} else if (level <= MAX_BL) {
		level = (level - NORMAL_MAX_BL) * (MAX_REG - NORMAL_MAX_REG_XL) / (MAX_BL - NORMAL_MAX_BL) + NORMAL_MAX_REG_XL;
		pr_info("%s:XL_hbm_on,level = %d\n",__func__,level);
	} else {
		pr_info("%s:set backlight err\n",__func__);
		return -1;
	}
	bl_tb[1] = (level >> 8) & 0xFF;
	bl_tb[2] = level & 0xFF;
	curr_bl1 = bl_tb[1];
	curr_bl2 = bl_tb[2];

	if (!cb)
		return -1;
	pr_info("%s %d %d %d\n", __func__, level, bl_tb[1], bl_tb[2]);
	cb(dsi, handle, bl_tb, ARRAY_SIZE(bl_tb));

	if (level == 0) {
		cb(dsi, handle, bl_tb_cabc, ARRAY_SIZE(bl_tb_cabc));
		bl_first = false;
	} else {
		if (bl_first == false)
		{
			bl_tb_cabc[2] = 0x0C;
			cb(dsi, handle, bl_tb_cabc, ARRAY_SIZE(bl_tb_cabc));
			bl_first = true;
			pr_info("%s bl_first set true\n", __func__);
		}
	}
	return 0;
}

static void mode_switch_to_120(struct drm_panel *panel,
	enum MTK_PANEL_MODE_SWITCH_STAGE stage)
{
	struct lcm *ctx = panel_to_lcm(panel);
	pr_info("%s state = %d\n", __func__, stage);
	if (stage == BEFORE_DSI_POWERDOWN) {
		ctx->dynamic_fps = 120;
	}
}

static void mode_switch_to_90(struct drm_panel *panel,
	enum MTK_PANEL_MODE_SWITCH_STAGE stage)
{
	struct lcm *ctx = panel_to_lcm(panel);
	pr_info("%s state = %d\n", __func__, stage);
	if (stage == BEFORE_DSI_POWERDOWN) {
		ctx->dynamic_fps = 90;
	}
}

static void mode_switch_to_60(struct drm_panel *panel,
	enum MTK_PANEL_MODE_SWITCH_STAGE stage)
{
	struct lcm *ctx = panel_to_lcm(panel);
	pr_info("%s state = %d\n", __func__, stage);
	if (stage == BEFORE_DSI_POWERDOWN) {
		ctx->dynamic_fps = 60;
	}
}

static int mode_switch(struct drm_panel *panel,
		struct drm_connector *connector, unsigned int cur_mode,
		unsigned int dst_mode, enum MTK_PANEL_MODE_SWITCH_STAGE stage)
{
	int ret = 0;
	int dst_fps = 0, cur_fps = 0;
	int dst_vdisplay = 0, dst_hdisplay = 0;
	int cur_vdisplay = 0, cur_hdisplay = 0;
	bool isFpsChange = false;
	struct drm_display_mode *m_dst = get_mode_by_id(connector, dst_mode);
	struct drm_display_mode *m_cur = get_mode_by_id(connector, cur_mode);
	if (cur_mode == dst_mode)
		return ret;
	dst_fps = m_dst ? drm_mode_vrefresh(m_dst) : -EINVAL;
	dst_vdisplay = m_dst ? m_dst->vdisplay : -EINVAL;
	dst_hdisplay = m_dst ? m_dst->hdisplay : -EINVAL;
	cur_fps = m_cur ? drm_mode_vrefresh(m_cur) : -EINVAL;
	cur_vdisplay = m_cur ? m_cur->vdisplay : -EINVAL;
	cur_hdisplay = m_cur ? m_cur->hdisplay : -EINVAL;
	isFpsChange = ((dst_fps == cur_fps) && (dst_fps != -EINVAL)
			&& (cur_fps != -EINVAL)) ? false : true;
	pr_info("%s isFpsChange = %d\n", __func__, isFpsChange);
	pr_info("%s dst_mode vrefresh = %d, vdisplay = %d, vdisplay = %d\n",
		__func__, dst_fps, dst_vdisplay, dst_hdisplay);
	pr_info("%s cur_mode vrefresh = %d, vdisplay = %d, vdisplay = %d\n",
		__func__, cur_fps, cur_vdisplay, cur_hdisplay);
	if (isFpsChange) {
		if (dst_fps == MODE0_FPS)
			mode_switch_to_60(panel, stage);
		else if (dst_fps == MODE1_FPS)
			mode_switch_to_90(panel, stage);
        else if (dst_fps == MODE2_FPS)
			mode_switch_to_120(panel, stage);
		else
			ret = 1;
	}
	return ret;
}

#define CABC_OFF	0
#define	CABC_UI		1
#define	CABC_MOVIE	2
#define	CABC_STILL	3

int lcm_cabc_control_lc(struct drm_panel *panel, int mode)
{

	struct lcm *ctx = panel_to_lcm(panel);

	if ((!ctx->prepared) || (!panel_inited))
	{
		pr_err("%s, panel unprepare, cabc exit \n", __func__);
		return 0;
	}

	switch (mode)
	{
	case CABC_OFF:
		lcm_dcs_write_seq_static(ctx, 0x55,0x00);
		pr_info("%s, CABC_OFF enable\n", __func__);
		break;
	case CABC_UI:
		lcm_dcs_write_seq_static(ctx, 0x55,0x01);
		pr_info("%s, CABC_UI enable\n", __func__);
		break;

	case CABC_MOVIE:
		lcm_dcs_write_seq_static(ctx, 0x55,0x03);
		pr_info("%s, CABC_MOVIE enable\n", __func__);
		break;

	case CABC_STILL:
		lcm_dcs_write_seq_static(ctx, 0x55,0x02);
		pr_info("%s, CABC_STILL enable\n", __func__);
		break;

	default:
		pr_err("%s, noting to do\n", __func__);
		break;
	}

	pr_info("%s, end \n", __func__);

	return 0;
}

static int panel_get_panel_info(struct drm_panel *panel, char *buf)
{
	int count = 0;
	struct lcm *ctx;
	if (!panel) {
		pr_err(": panel is NULL\n", __func__);
		return -EAGAIN;
	}
	ctx = panel_to_lcm(panel);
	count = snprintf(buf, PAGE_SIZE, "%s\n", ctx->panel_info);
	return count;
}

static int panel_get_wp_info(struct drm_panel *panel, char *buf, size_t size)
{
	struct device_node *chosen;
	char *tmp_buf = NULL;
	int tmp_size = 0;
	chosen = of_find_node_by_path("/chosen");
	if (chosen) {
		tmp_buf = (char *)of_get_property(chosen, "wp_info", (int *)&tmp_size);
		if (tmp_size > 0) {
			strncpy(buf, tmp_buf, tmp_size);
			pr_info("[%s]: white_point = %s, size = %d\n", __func__, buf, tmp_size);
		}
	} else {
		pr_info("[%s]:find chosen failed\n", __func__);
	}

	return tmp_size;
}
static struct LCM_setting_table esd_restore_bl[] = {
	{0x51, 02, {0x03,0xFF}},
};
static void lcm_esd_restore_backlight(struct drm_panel *panel)
{
	struct lcm *ctx = panel_to_lcm(panel);
	esd_restore_bl->para_list[0] = curr_bl1;
	esd_restore_bl->para_list[1] = curr_bl2;
	push_table(ctx, esd_restore_bl,
		sizeof(esd_restore_bl) / sizeof(struct LCM_setting_table));
	pr_info("%s, lcm_esd_restore_backlight restore curr_bl1:%x curr_bl2:%x \n", __func__, curr_bl1, curr_bl2);

	pr_info("lcm_esd_restore_backlight \n");

	return;
}

static struct LCM_setting_table hbm_setting[] = {
	{0x51, 02, {0xDC,0x00}},
};

static struct LCM_setting_table hbm_setting_off[] = {
	{0x51, 02, {0xBF,0x0E}},
};

int lcm_hbm_control_lc(struct drm_panel *panel, uint32_t level)
{

	struct lcm *ctx = panel_to_lcm(panel);

	if (!ctx->prepared)
	{
		pr_err("%s, panel unprepare, hbm exit \n", __func__);
		return 0;
	}
	mutex_lock(&ctx->panel_lock);
	if(level == 1){
		push_table(ctx, hbm_setting,
			sizeof(hbm_setting) / sizeof(struct LCM_setting_table));
		pr_info("%s, lcm_hbm_control_lc enable\n", __func__);
	} else if (level == 0) {
		push_table(ctx, hbm_setting_off,
			sizeof(hbm_setting_off) / sizeof(struct LCM_setting_table));
		pr_info("%s, lcm_hbm_control_lc disable\n", __func__);
	}
	mutex_unlock(&ctx->panel_lock);
	return 0;
}

static int lcm_send_ddic_cmd_pack(struct drm_panel *panel,
		void *dsi_drv, dcs_write_gce_pack cb, void *handle)
{
	struct lcm *ctx = panel_to_lcm(panel);

	lcm_dcs_write_seq_static(ctx, 0x28);
	lcm_dcs_write_seq_static(ctx, 0x10);
	return 0;
}

static struct mtk_panel_funcs ext_funcs = {
	.ext_param_set = mtk_panel_ext_param_set,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	.mode_switch = mode_switch,
	.normal_hbm_control = lcm_hbm_control_lc,
	.get_panel_info = panel_get_panel_info,
	.get_wp_info = panel_get_wp_info,
	.esd_restore_backlight = lcm_esd_restore_backlight,
	.cabc_control_lc = lcm_cabc_control_lc,
	.send_ddic_cmd_pack = lcm_send_ddic_cmd_pack,
};

struct panel_desc {
	const struct drm_display_mode *modes;
	unsigned int num_modes;
	unsigned int bpc;
	struct {
		unsigned int width;
		unsigned int height;
	} size;
	struct {
		unsigned int prepare;
		unsigned int enable;
		unsigned int disable;
		unsigned int unprepare;
	} delay;
};

static int lcm_get_modes(struct drm_panel *panel, struct drm_connector *connector)
{
    struct drm_display_mode *mode0, *mode1, *mode2;
	mode0 = drm_mode_duplicate(connector->dev, &mode_60hz);
	if (!mode0) {
		dev_err(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			mode_60hz.hdisplay, mode_60hz.vdisplay,
			drm_mode_vrefresh(&mode_60hz));
		return -ENOMEM;
	}
	drm_mode_set_name(mode0);
	mode0->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode0);
	mode1 = drm_mode_duplicate(connector->dev, &mode_90hz);
	if (!mode1) {
		dev_err(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			mode_90hz.hdisplay, mode_90hz.vdisplay,
			drm_mode_vrefresh(&mode_90hz));
		return -ENOMEM;
	}
	drm_mode_set_name(mode1);
	mode1->type = DRM_MODE_TYPE_DRIVER;
	drm_mode_probed_add(connector, mode1);
	mode2 = drm_mode_duplicate(connector->dev, &default_mode);
	if (!mode2) {
		dev_err(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			default_mode.hdisplay, default_mode.vdisplay,
			drm_mode_vrefresh(&default_mode));
		return -ENOMEM;
	}
		drm_mode_set_name(mode2);
		mode2->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
		drm_mode_probed_add(connector, mode2);
	return 1;
}

static const struct drm_panel_funcs lcm_drm_funcs = {
	.disable = lcm_disable,
	.unprepare = lcm_unprepare,
	.prepare = lcm_prepare,
	.enable = lcm_enable,
	.get_modes = lcm_get_modes,
};

extern int panel_event;
static ssize_t panel_event_show(struct device *device,
			struct device_attribute *attr,
			char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", panel_event);
}

static DEVICE_ATTR_RO(panel_event);
static struct attribute *nt36528_attrs[] = {
	&dev_attr_panel_event.attr,
	NULL,
};
static const struct attribute_group nt36528_attr_group = {
	.attrs = nt36528_attrs,
};

static int lcm_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct device_node *dsi_node, *remote_node = NULL, *endpoint = NULL;
	struct lcm *ctx;
	struct device_node *backlight;
	int ret;
	pr_info("%s dsi_panel_p15_43_02_0a_hd_vdo +\n", __func__);
	dsi_node = of_get_parent(dev->of_node);
	if (dsi_node) {
		endpoint = of_graph_get_next_endpoint(dsi_node, NULL);
		if (endpoint) {
			remote_node = of_graph_get_remote_port_parent(endpoint);
			if (!remote_node) {
				pr_info("No panel connected,skip probe lcm\n");
				return -ENODEV;
			}
			pr_info("device node name:%s\n", remote_node->name);
		}
	}
	if (remote_node != dev->of_node) {
		pr_info("%s+ skip probe due to not current lcm\n", __func__);
		return -ENODEV;
	}
	ctx = devm_kzalloc(dev, sizeof(struct lcm), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;
	panel_ctx = ctx;
	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dev = dev;
	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM |
		MIPI_DSI_MODE_NO_EOT_PACKET | MIPI_DSI_CLOCK_NON_CONTINUOUS;
	backlight = of_parse_phandle(dev->of_node, "backlight", 0);
	if (backlight) {
		ctx->backlight = of_find_backlight_by_node(backlight);
		of_node_put(backlight);
		if (!ctx->backlight)
			return -EPROBE_DEFER;
	}
	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(dev, "%s: cannot get reset-gpios %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	devm_gpiod_put(dev, ctx->reset_gpio);
	ctx->enp_gpio =
		devm_gpiod_get(ctx->dev, "enp", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enp_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enp_gpio %ld\n",
			__func__, PTR_ERR(ctx->enp_gpio));
		return PTR_ERR(ctx->enp_gpio);
	}
	devm_gpiod_put(ctx->dev, ctx->enp_gpio);
	ctx->enn_gpio =
		devm_gpiod_get(ctx->dev, "enn", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->enn_gpio)) {
#if IS_ENABLED(CONFIG_MIEV)
		mi_disp_mievent_str(MI_EVENT_PANEL_HW_RESOURCE_GET_FAILED);
#endif
		dev_err(ctx->dev, "%s: cannot get enn_gpio %ld\n",
			__func__, PTR_ERR(ctx->enn_gpio));
		return PTR_ERR(ctx->enn_gpio);
	}
	devm_gpiod_put(ctx->dev, ctx->enn_gpio);
	ext_params.err_flag_irq_gpio = of_get_named_gpio_flags(
		dev->of_node, "mi,esd-err-irq-gpio",
		0, (enum of_gpio_flags *)&(ext_params.err_flag_irq_flags));
	ctx->prepared = true;
	ctx->enabled = true;
	ctx->dynamic_fps = 120;
	drm_panel_init(&ctx->panel, dev, &lcm_drm_funcs, DRM_MODE_CONNECTOR_DSI);
	drm_panel_add(&ctx->panel);
	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		drm_panel_remove(&ctx->panel);
#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_tch_handle_reg(&ctx->panel);
	ret = mtk_panel_ext_create(dev, &ext_params, &ext_funcs, &ctx->panel);
	if (ret < 0)
		return ret;
#endif
	ctx->panel_info = panel_name;
	ret = sysfs_create_group(&dev->kobj, &nt36528_attr_group);
	if (ret)
		return ret;
	pr_info("%s- dsi_panel_p15_43_02_0a_hd_vdo\n", __func__);
	return ret;
}

static int lcm_remove(struct mipi_dsi_device *dsi)
{
	struct lcm *ctx = mipi_dsi_get_drvdata(dsi);
#if defined(CONFIG_MTK_PANEL_EXT)
	struct mtk_panel_ctx *ext_ctx = find_panel_ctx(&ctx->panel);
#endif
	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_detach(ext_ctx);
	mtk_panel_remove(ext_ctx);
#endif
	return 0;
}

static const struct of_device_id lcm_of_match[] = {
	{ .compatible = "dsi_panel_p15_43_02_0a_hd_vdo,lcm", },
	{ }
};

MODULE_DEVICE_TABLE(of, lcm_of_match);
static struct mipi_dsi_driver lcm_driver = {
	.probe = lcm_probe,
	.remove = lcm_remove,
	.driver = {
		.name = "dsi_panel_p15_43_02_0a_hd_vdo",
		.owner = THIS_MODULE,
		.of_match_table = lcm_of_match,
	},
};
module_mipi_dsi_driver(lcm_driver);
MODULE_AUTHOR("p15");
MODULE_DESCRIPTION("dsi_panel_p15_43_02_0a_hd_vdo panel driver");
MODULE_LICENSE("GPL v2");
