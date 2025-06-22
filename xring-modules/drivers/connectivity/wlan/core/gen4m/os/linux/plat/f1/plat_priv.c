/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
#include "gl_os.h"

#if KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
#include <uapi/linux/sched/types.h>
#include <linux/sched/task.h>
#include <linux/cpufreq.h>
#endif
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>

#include <linux/pm_qos.h>
#include "precomp.h"

#if (KERNEL_VERSION(5, 10, 0) <= CFG80211_VERSION_CODE)
#include <linux/regulator/consumer.h>
#endif
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include "wlan_pinctrl.h"

#include <linux/interconnect.h>


#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#define RST_PIN_MIN_WAIT_TIME		10 /* ms */
static struct pinctrl *pinctrl_ptr;
#ifdef CFG_XRING_PLAT
static unsigned int g_wakeup_irq_num;
static int g_wakeup_n_gpio;
#endif
static int8_t last_wf_rst_pin_state;
static OS_SYSTIME last_toggle_time;

static int32_t f1_wlan_pinctrl_init(struct mt66xx_chip_info *chip_info);
static int32_t f1_wlan_pinctrl_action(struct mt66xx_chip_info *chip_info,
	enum WLAN_PINCTRL_MSG msg);

struct WLAN_PINCTRL_OPS f1_pinctrl_ops = {
	.init = f1_wlan_pinctrl_init,
	.action = f1_wlan_pinctrl_action,
};
#endif

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#ifdef CFG_XRING_PLAT
static irqreturn_t pcie_wake_n_isr(int irq, void *arg)
{
	DBGLOG(INIT, INFO, "arg: [0x%px], irqnum:[%d]", arg, irq);
	return IRQ_HANDLED;
}

static void wf_wakeup_init(struct platform_device *pdev)
{
	DBGLOG(INIT, INFO, "Wakeup irq : use mtk\n");
	return;
}

static void wf_wakeup_deinit(struct platform_device *pdev)
{
	DBGLOG(INIT, INFO, "Wakeup un-irq : use mtk\n");
	return;
}
#endif

static int32_t f1_wlan_pinctrl_init(struct mt66xx_chip_info *chip_info)
{
	struct platform_device *pdev;
	int32_t ret = 0;
	kalGetPlatDev(&pdev);

	if (!pdev) {
		DBGLOG(INIT, ERROR,
			"NULL platform_device\n",
			ret);
		ret = -EINVAL;
		goto exit;
	}
#ifdef CFG_XRING_PLAT
	g_wakeup_n_gpio = of_get_named_gpio(pdev->dev.of_node, "wf-wk-gpio", 0);
	if (!gpio_is_valid(g_wakeup_n_gpio)) {
		DBGLOG(INIT, ERROR, "Failed to get wf wk-gpio\n");
		return -1;
	}
	g_wakeup_irq_num = gpio_to_irq(g_wakeup_n_gpio);
	DBGLOG(INIT, INFO, "wf wakeup host irq = %d\n", g_wakeup_irq_num);
#endif
	pinctrl_ptr = devm_pinctrl_get(&pdev->dev);

	if (KAL_IS_ERR(pinctrl_ptr)) {
		ret = PTR_ERR(pinctrl_ptr);
		DBGLOG(INIT, ERROR,
			"devm_pinctrl_get failed, ret=%d.\n",
			ret);
		goto exit;
	}

	last_toggle_time = 0;
	last_wf_rst_pin_state = -1;

	//wlan_pinctrl_action(chip_info, WLAN_PINCTRL_MSG_FUNC_PTA_UART_INIT);

exit:
	return ret;
}


static void ensure_rst_pin_min_wait_time(int8_t state)
{
	OS_SYSTIME current_time = 0;
	uint32_t retry = 0;

	if (last_wf_rst_pin_state == state)
		return;

	while (TRUE) {
		GET_CURRENT_SYSTIME(&current_time);

		if (last_toggle_time == 0)
			break;
		else if (CHECK_FOR_TIMEOUT(current_time,
					   last_toggle_time,
					   RST_PIN_MIN_WAIT_TIME))
			break;

		DBGLOG_LIMITED(INIT, INFO, "retry: %d.\n", retry);
		retry++;
		kalMdelay(1);
	}
	GET_CURRENT_SYSTIME(&last_toggle_time);
	last_wf_rst_pin_state = state;
}

static int32_t f1_wlan_pinctrl_action(struct mt66xx_chip_info *chip_info,
	enum WLAN_PINCTRL_MSG msg)
{
	struct pinctrl_state *pinctrl_state;
	uint8_t *name;
	int32_t ret = 0;
#ifdef CFG_XRING_PLAT
	struct platform_device *pdev;
        kalGetPlatDev(&pdev);
        if (!pdev) {
                DBGLOG(INIT, ERROR, "pdev is NULL\n");
                ret = -EINVAL;
		goto exit;
	}
	enum {WK_HOST_NONE, WK_HOST_ON, WK_HOST_OFF} wk_host = WK_HOST_NONE;
#endif

	if (KAL_IS_ERR(pinctrl_ptr)) {
		DBGLOG(INIT, ERROR, "pinctrl: not inited!\n");
		ret = -EINVAL;
		goto exit;
	}

	switch (msg) {
	case WLAN_PINCTRL_MSG_FUNC_ON:
		name = "wf_rst_on";
		ensure_rst_pin_min_wait_time(1);
		break;
	case WLAN_PINCTRL_MSG_FUNC_OFF:
		name = "wf_rst_off";
		ensure_rst_pin_min_wait_time(0);
		break;
	case WLAN_PINCTRL_MSG_FUNC_PTA_UART_INIT:
		name = "wf_rst_pta_uart_init";
		break;
	case WLAN_PINCTRL_MSG_FUNC_PTA_UART_ON:
		name = "wf_rst_pta_uart_on";
		break;
	case WLAN_PINCTRL_MSG_FUNC_PTA_UART_OFF:
		name = "wf_rst_pta_uart_off";
		break;
#ifdef CFG_XRING_PLAT
	case WLAN_PINCTRL_MSG_FUNC_WK_HOST_ON:
		name = "wf_wk_host_on";
		wk_host = WK_HOST_ON;
		break;
	case WLAN_PINCTRL_MSG_FUNC_WK_HOST_OFF:
		name = "wf_wk_host_off";
		wk_host = WK_HOST_OFF;
		break;
#endif
	default:
		DBGLOG(INIT, ERROR,
			"Unknown msg: %d.\n",
			msg);
		ret = -EINVAL;
		goto exit;
	}

	pinctrl_state = pinctrl_lookup_state(pinctrl_ptr, name);

	if (KAL_IS_ERR(pinctrl_state)) {
		ret = PTR_ERR(pinctrl_state);
		DBGLOG(INIT, ERROR,
			"pinctrl_lookup_state %s, ret=%d.\n",
			name,
			ret);
		goto exit;
	}

	ret = pinctrl_select_state(pinctrl_ptr, pinctrl_state);
	DBGLOG(INIT, INFO,
		"pinctrl_select_state msg[%d][%s], ret: %d.\n",
		msg, name, ret);

#ifdef CFG_XRING_PLAT
	if (wk_host == WK_HOST_ON)
		wf_wakeup_init(pdev);
	if (wk_host == WK_HOST_OFF)
		wf_wakeup_deinit(pdev);
#endif

exit:
	return ret;
}
#endif
