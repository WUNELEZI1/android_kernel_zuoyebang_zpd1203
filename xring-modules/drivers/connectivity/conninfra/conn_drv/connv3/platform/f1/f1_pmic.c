// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__

#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/jiffies.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/timer.h>
#include <linux/vmalloc.h>

#include "connv3.h"
#include "conn_adaptor.h"
#include "connv3_hw.h"
#include "connv3_pmic_mng.h"
#include "consys_reg_util.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

static struct connv3_dev_cb *g_dev_cb;
static struct pinctrl *g_pinctrl_ptr;
int fmd_flag = false;

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static int connv3_plt_pmic_initial_setting_f1(struct platform_device *pdev, struct connv3_dev_cb *dev_cb);
static int connv3_plt_pmic_common_power_ctrl_f1(u32 enable);
static int connv3_plt_pmic_parse_state_f1(char *buffer, int buf_sz);
static int connv3_plt_pmic_antenna_power_ctrl_f1(u32 radio, u32 enable);
static int connv3_plt_pmic_fmd_setting_f1(u32 enable);

const struct connv3_platform_pmic_ops g_connv3_platform_pmic_ops_f1 = {
	.pmic_initial_setting = connv3_plt_pmic_initial_setting_f1,
	.pmic_common_power_ctrl = connv3_plt_pmic_common_power_ctrl_f1,
	.pmic_parse_state = connv3_plt_pmic_parse_state_f1,
	.pmic_fmd_setting = connv3_plt_pmic_fmd_setting_f1,
	.pmic_antenna_power_ctrl = connv3_plt_pmic_antenna_power_ctrl_f1,
};

static struct work_struct g_pmic_faultb_work;
static unsigned int g_pmic_excep_irq_num;
static unsigned int g_spurious_pmic_exception = 1;
static unsigned int faultb_immidately;
static int g_faultb_gpio, g_pmic_en_gpio;
static irqreturn_t pmic_fault_handler(int irq, void *arg)
{
	if (g_spurious_pmic_exception) {
		pr_info("%s, g_spurious_pmic_exception\n", __func__);
		return IRQ_HANDLED;
	}

	pr_err("%s, Get PMIC FaultB interrupt\n", __func__);
	schedule_work(&g_pmic_faultb_work);

	return IRQ_HANDLED;
}

static void check_faultb_status(struct work_struct *work)
{
	unsigned int faultb_level;

	if (faultb_immidately == 0) {
		mdelay(10);

		faultb_level = gpio_get_value(g_faultb_gpio);
		pr_info("%s, PMIC_EN=%d, faultb=%d\n",
			__func__, gpio_get_value(g_pmic_en_gpio), faultb_level);
		if (faultb_level == 1)
			return;
	}

	if (g_dev_cb != NULL && g_dev_cb->connv3_pmic_event_notifier != NULL)
		g_dev_cb->connv3_pmic_event_notifier(0, 1);
}

static void _dump_pmic_gpio_state(void)
{
	pr_info("F1: donot use this _dump_pmic_gpio_state\n");
}

static int connv3_plt_pmic_initial_setting_f1(struct platform_device *pdev, struct connv3_dev_cb *dev_cb)
{
	struct pinctrl_state *pinctrl_faultb_init;
	struct pinctrl_state *pinctrl_pmic_en_init;
	int ret = 0;
	unsigned int irq_num = 0;

	if (dev_cb == NULL)
		return -1;

	g_dev_cb = dev_cb;

	g_faultb_gpio = of_get_named_gpio(pdev->dev.of_node, "pmic-gpio", 0);
	g_pmic_en_gpio = of_get_named_gpio(pdev->dev.of_node, "pmic-gpio", 1);

	if (!gpio_is_valid(g_faultb_gpio) || !gpio_is_valid(g_pmic_en_gpio)) {
		pr_err("cannot get pmic-gpio");
		return -1;
	}

	INIT_WORK(&g_pmic_faultb_work, check_faultb_status);

	g_pinctrl_ptr = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(g_pinctrl_ptr)) {
		pr_err("[%s] get pinctrl fail\n", __func__);
		return -1;
	}

	_dump_pmic_gpio_state();

	/* config faultb to low */
	pinctrl_faultb_init = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-pmic-faultb-default");
	if (!IS_ERR(pinctrl_faultb_init)) {
		ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_faultb_init);
		if (ret) {
			pr_err("[%s] faultb init fail, %d", __func__, ret);
			return -1;
		}
	} else {
		pr_err("[%s] fail to get \"connsys-pin-pmic-faultb-default\"",  __func__);
		return -1;
	}

	/* config pmic-en to low */
	pinctrl_pmic_en_init = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-pmic-en-default");
	if (!IS_ERR(pinctrl_pmic_en_init)) {
		ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_pmic_en_init);
		if (ret) {
			pr_err("[%s] pmic-en init fail, %d", __func__, ret);
			return -1;
		}
	} else {
		pr_err("[%s] fail to get \"connsys-pin-pmic-en-default\"",  __func__);
		return -1;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "faultb-immediately", &faultb_immidately);
	if (ret)
		pr_info("%s[%d], there is no faultb-immediately node, ret=%d, faultb_immidately=%d\n", __func__, __LINE__, ret, faultb_immidately);
	else
		pr_info("%s[%d], faultb-immediately=%d\n", __func__, __LINE__, faultb_immidately);

	irq_num = gpio_to_irq(g_faultb_gpio);

	ret = devm_request_threaded_irq(&pdev->dev, irq_num, NULL,
				pmic_fault_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				"MT6376_FAULT", platform_get_drvdata(pdev));
	if (ret) {
		pr_err("%s[%d], request faultb irq fail with irq_num=%d\n", __func__, __LINE__, irq_num);
		return -1;
	}
	g_pmic_excep_irq_num = irq_num;

	pr_info("%s[%d], request faultb irq OK with irq_num=%d\n", __func__, __LINE__, irq_num);

	return 0;
}

static int connv3_uart_pre(void)
{
	struct pinctrl_state *pinctrl_txd;
	struct pinctrl_state *pinctrl_rxd;
	int ret = 0;

	pinctrl_txd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-txd-pre");
	pinctrl_rxd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-rxd-pre");
	if (IS_ERR(pinctrl_txd) || IS_ERR(pinctrl_rxd)) {
		pr_err("[%s] fail to get \"connsys-pin-trxd-pre\"",  __func__);
		return -1;
	}
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_txd);
	if (ret)
		pr_err("[%s] pinctrl_txd pre fail, %d", __func__, ret);
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_rxd);
	if (ret)
		pr_err("[%s] pinctrl_rxd pre fail, %d", __func__, ret);
	pr_info("[%s] uart config %s", __func__, ret == 0 ? "OK":"FAIL");
	return ret;
}

static int connv3_uart_init(void)
{
	struct pinctrl_state *pinctrl_txd;
	struct pinctrl_state *pinctrl_rxd;
	int ret = 0;

	pinctrl_txd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-txd-init");
	pinctrl_rxd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-rxd-init");
	if (IS_ERR(pinctrl_txd) || IS_ERR(pinctrl_rxd)) {
		pr_err("[%s] fail to get \"connsys-pin-trxd\"",  __func__);
		return -1;
	}
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_txd);
	if (ret)
		pr_err("[%s] pinctrl_txd on fail, %d", __func__, ret);
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_rxd);
	if (ret)
		pr_err("[%s] pinctrl_rxd on fail, %d", __func__, ret);
	pr_info("[%s] uart config %s", __func__, ret == 0 ? "OK":"FAIL");
	return ret;
}

static int connv3_uart_default(void)
{
	struct pinctrl_state *pinctrl_txd;
	struct pinctrl_state *pinctrl_rxd;
	int ret = 0;

	pinctrl_txd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-txd-default");
	pinctrl_rxd = pinctrl_lookup_state(
			g_pinctrl_ptr, "connsys-pin-uart-rxd-default");
	if (IS_ERR(pinctrl_txd) || IS_ERR(pinctrl_rxd)) {
		pr_err("[%s] fail to get \"connsys-pin-trxd\"",  __func__);
		return -1;
	}
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_txd);
	if (ret)
		pr_err("[%s] pinctrl_txd on fail, %d", __func__, ret);
	ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_rxd);
	if (ret)
		pr_err("[%s] pinctrl_rxd on fail, %d", __func__, ret);
	pr_info("[%s] uart config %s", __func__, ret == 0 ? "OK":"FAIL");
	return ret;
}

static int connv3_plt_pmic_common_power_ctrl_f1(u32 enable)
{
	struct pinctrl_state *pinctrl_set;
	struct pinctrl_state *faultb_set;
	int ret = 0;
	pr_info("[%s] enable=[%d]", __func__, enable);

	if (enable) {
		/* config SCP UART1-4 to GPIO output high before pullup PMIC-EN
		 * donot check errors
		 */
		connv3_uart_pre();

		pinctrl_set = pinctrl_lookup_state(
				g_pinctrl_ptr, "connsys-pin-pmic-en-set");
		if (!IS_ERR(pinctrl_set)) {
			mdelay(30);
			ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_set);
			if (ret)
				pr_err("[%s] pinctrl on fail, %d", __func__, ret);
		} else {
			pr_err("[%s] fail to get \"connsys-pin-pmic-en-set\"",  __func__);
		}
		mdelay(20);

		faultb_set = pinctrl_lookup_state(
				g_pinctrl_ptr, "connsys-pin-pmic-faultb-enable");
		if (!IS_ERR(faultb_set)) {
			ret = pinctrl_select_state(g_pinctrl_ptr, faultb_set);
			if (ret)
				pr_err("[%s] faultb on fail, %d", __func__, ret);
		} else {
			pr_err("[%s] fail to get \"connsys-pin-pmic-faultb-enable\"",  __func__);
		}

		/* config SCP UART1-4 as default-state after pullup PMIC-EN
		 * donot check errors
		 */
		connv3_uart_default();

		g_spurious_pmic_exception = 0;
	} else {
		if (fmd_flag == true) {
			pr_info("%s[%d] Don't turn off PMIC_EN0 in co-clock FMD device\n", __func__, __LINE__);
			return 0;
		}
		g_spurious_pmic_exception = 1;

		/* config SCP UART1-4 as init-state after pullup PMIC-EN
		 * donot check errors
		 */
		connv3_uart_init();

		faultb_set = pinctrl_lookup_state(
				g_pinctrl_ptr, "connsys-pin-pmic-faultb-default");
		if (!IS_ERR(faultb_set)) {
			ret = pinctrl_select_state(g_pinctrl_ptr, faultb_set);
			if (ret)
				pr_err("[%s] faultb off fail, %d", __func__, ret);
		} else {
			pr_err("[%s] fail to get \"connsys-pin-pmic-faultb-default\"",  __func__);
		}

		pinctrl_set = pinctrl_lookup_state(
				g_pinctrl_ptr, "connsys-pin-pmic-en-clr");
		if (!IS_ERR(pinctrl_set)) {
			/* Add a delay before pmic_en = 0 to make sure reset task is completed.
			 * According to experiment, 1 ms is enough.
			 * Use 20ms because the api is not accurate.
			 */
			mdelay(20);
			ret = pinctrl_select_state(g_pinctrl_ptr, pinctrl_set);
			if (ret)
				pr_err("[%s] pinctrl on fail, %d", __func__, ret);
		} else {
			pr_err("[%s] fail to get \"connsys-pin-pmic-en-clr\"",	__func__);
		}
	}


	return ret;
}


int connv3_plt_pmic_parse_state_f1(char *buffer, int buf_sz)
{
	pr_info("%s: not used yet\n", __func__);
	return 0;
}

int connv3_plt_pmic_fmd_setting_f1(u32 enable)
{
	fmd_flag = true;
	return 0;
}
int connv3_plt_pmic_antenna_power_ctrl_f1(u32 radio, u32 enable)
{
	int ret = 0;
	static bool is_on;

	if (!conn_adaptor_is_internal()) {
		pr_notice("[%s] external project, ignore setting\n", __func__);
		return 0;
	}

	if (radio != CONNV3_DRV_TYPE_WIFI)
		return 0;

	/* Status check
	 * because connv3_plt_pmic_antenna_power_ctrl_mt6985 off may be called multiple times,
	 * we have to maintain the status to avoid incorrect status change.
	 */
	if (is_on && enable == 1) {
		return 0;
	}

	if (!is_on && enable == 0) {
		return 0;
	}

	return ret;
}
