// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__

#include <linux/gpio/consumer.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>

#include "connv3_hw.h"
#include "connv3_pinctrl_mng.h"

#include "consys_reg_util.h"

/*
 * NOTE: f1 use MPMIC for ext32k clock clk_32k_o2.
 * #define CONNSYS_PIN_NAME_EXT_32K_EN_DEFAULT	"connsys-pin-ext32k-en-default"
 * #define CONNSYS_PIN_NAME_EXT_32K_EN_SET		"connsys-pin-ext32k-en-set"
 * #define CONNSYS_PIN_NAME_EXT_32K_EN_CLR		"connsys-pin-ext32k-en-clr"
 */

/*
 * NOTE: f1 uart for sensorhub is configured by scp owner
 * #define CONNSYS_PIN_NAME_UART_INIT		"connsys-combo-gpio-init"
 * #define CONNSYS_PIN_NAME_UART_PRE_ON		"connsys-combo-gpio-pre-on"
 * #define CONNSYS_PIN_NAME_UART_ON		"connsys-combo-gpio-on"
 */

static struct clk *g_clk_32k_o2;
static int fmd_flag = false;
/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static int connv3_plt_pinctrl_init_f1(struct platform_device *pdev);
static int connv3_plt_pinctrl_deinit_f1(void);
static int connv3_plt_pinctrl_setup_pre_f1(void);
static int connv3_plt_pinctrl_setup_done_f1(void);
static int connv3_plt_pinctrl_remove_f1(void);
static int connv3_plt_pinctrl_ext_32k_ctrl_f1(bool on);
static int connv3_plt_pinctrl_fmd_setting_f1(u32 enable);

const struct connv3_platform_pinctrl_ops g_connv3_platform_pinctrl_ops_f1 = {
	.pinctrl_init = connv3_plt_pinctrl_init_f1,
	.pinctrl_deinit = connv3_plt_pinctrl_deinit_f1,
	.pinctrl_setup_pre = connv3_plt_pinctrl_setup_pre_f1,
	.pinctrl_setup_done = connv3_plt_pinctrl_setup_done_f1,
	.pinctrl_remove = connv3_plt_pinctrl_remove_f1,
	.pinctrl_ext_32k_ctrl = connv3_plt_pinctrl_ext_32k_ctrl_f1,
	.pinctrl_fmd_setting = connv3_plt_pinctrl_fmd_setting_f1,
};

static int connv3_plt_pinctrl_initial_state(void)
{
	return 0;
}

static int connv3_plt_pinctrl_init_f1(struct platform_device *pdev)
{
	g_clk_32k_o2 = devm_clk_get(&pdev->dev, "clk_32k_o2");

	if (IS_ERR_OR_NULL(g_clk_32k_o2)) {
		pr_err("[%s] fail to get g_clk_32k_o2 clk", __func__);
		return -1;
	}
	return 0;
}

static int connv3_plt_pinctrl_deinit_f1(void)
{
	pr_info("[%s] do nothing", __func__);

	return 0;
}

static int connv3_plt_pinctrl_setup_pre_f1(void)
{
	pr_info("[%s] do nothing", __func__);

	return 0;
}

static int connv3_plt_pinctrl_setup_done_f1(void)
{
	pr_info("[%s] do nothing", __func__);

	return 0;
}

static int connv3_plt_pinctrl_remove_f1(void)
{
	connv3_plt_pinctrl_initial_state();
	return 0;
}
static int connv3_plt_pinctrl_fmd_setting_f1(u32 enable) {
	fmd_flag = true;
	return 0;
}
static int connv3_plt_pinctrl_ext_32k_ctrl_f1(bool on)
{
	int ret = 0;

	static enum {CLK_OFF, CLK_ON} clk = CLK_OFF;

	if (IS_ERR_OR_NULL(g_clk_32k_o2)) {
		pr_info("%s: no ext32k g_clk_32k_o2 configured\n", __func__);
		return 0;
	}

	if (on) {
		if (clk == CLK_ON) {
			pr_info("%s: pmic clk_32k_o2 already ON\n", __func__);
			goto exit;
		}
		ret = clk_prepare_enable(g_clk_32k_o2);
		if (!ret)
			clk = CLK_ON;
	} else {
		if (fmd_flag == true) {
			pr_info("%s[%d] Don't turn off PMIC_EN0 in co-clock FMD device\n", __func__, __LINE__);
			return 0;
		}
          	if (clk == CLK_OFF) {
			pr_info("%s: pmic clk_32k_o2 already OFF\n", __func__);
			goto exit;
		}
		clk_disable_unprepare(g_clk_32k_o2);
		clk = CLK_OFF;
		mdelay(1);
	}
	pr_info("[%s][%s] ext32k control: ret=[%d]",
			__func__, on ? "ON" : "OFF", ret);
exit:
	return ret;
}
