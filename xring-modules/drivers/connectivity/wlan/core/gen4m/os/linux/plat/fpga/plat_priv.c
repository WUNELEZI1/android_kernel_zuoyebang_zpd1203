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

#ifdef CONFIG_WLAN_MTK_EMI
#if KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
#include <soc/mediatek/emi.h>
#else
#include <memory/mediatek/emi.h>
#endif
#define DOMAIN_AP	0
#define DOMAIN_CONN	2
#endif

#define MAX_CPU_FREQ (3050 * 1000)
#define AUTO_CPU_FREQ (-1)
#define UNDEFINED_CPU_FREQ (-2)
#define CPU_ALL_CORE (0xff)
#define CPU_BIG_CORE (0xf0)
#define CPU_X_CORE (0x80)
#define CPU_HP_CORE (CPU_BIG_CORE - CPU_X_CORE)
#define CPU_LITTLE_CORE (CPU_ALL_CORE - CPU_BIG_CORE)
#define AUTO_PRIORITY 0
#define HIGH_PRIORITY 100

#define RPS_ALL_CORE (CPU_ALL_CORE - 0x11)
#define RPS_BIG_CORE (CPU_BIG_CORE - 0x10)
#define RPS_LITTLE_CORE (CPU_LITTLE_CORE - 0x01)

#if (KERNEL_VERSION(5, 10, 0) <= CFG80211_VERSION_CODE)
#include <linux/regulator/consumer.h>
#endif
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include "wlan_pinctrl.h"

/* for dram boost */
//#include "dvfsrc-exp.h"
#include <linux/interconnect.h>

static uint32_t u4EmiMetOffset = 0x45D400;

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#define RST_PIN_MIN_WAIT_TIME		10 /* ms */

static int8_t last_wf_rst_pin_state;
static OS_SYSTIME last_toggle_time;

static int32_t fpga_wlan_pinctrl_init(struct mt66xx_chip_info *chip_info);
static int32_t fpga_wlan_pinctrl_action(struct mt66xx_chip_info *chip_info,
	enum WLAN_PINCTRL_MSG msg);

struct WLAN_PINCTRL_OPS fpga_pinctrl_ops = {
	.init = fpga_wlan_pinctrl_init,
	.action = fpga_wlan_pinctrl_action,
};
#endif

uint32_t kalGetCpuBoostThreshold(void)
{
	DBGLOG(SW4, TRACE, "enter kalGetCpuBoostThreshold\n");
	/* 5, stands for 250Mbps */
	return 5;
}

int32_t kalCheckTputLoad(struct ADAPTER *prAdapter,
			 uint32_t u4CurrPerfLevel,
			 uint32_t u4TarPerfLevel,
			 int32_t i4Pending,
			 uint32_t u4Used)
{
	uint32_t pendingTh =
		CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD *
		prAdapter->rWifiVar.u4PerfMonPendingTh / 100;
	uint32_t usedTh = (HIF_TX_MSDU_TOKEN_NUM / 2) *
		prAdapter->rWifiVar.u4PerfMonUsedTh / 100;
	return u4TarPerfLevel >= 3 &&
	       u4TarPerfLevel < prAdapter->rWifiVar.u4BoostCpuTh &&
	       i4Pending >= pendingTh &&
	       u4Used >= usedTh ?
	       TRUE : FALSE;
}

#if KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
void kalSetTaskUtilMinPct(int pid, unsigned int min)
{
	int ret = 0;
	unsigned int blc_1024;
	struct task_struct *p;
	struct sched_attr attr = {};

	if (pid < 0)
		return;

	/* Fill in sched_attr */
	attr.sched_policy = -1;
	attr.sched_flags =
		SCHED_FLAG_KEEP_ALL |
		SCHED_FLAG_UTIL_CLAMP |
		SCHED_FLAG_RESET_ON_FORK;

	if (min == 0) {
		attr.sched_util_min = -1;
		attr.sched_util_max = -1;
	} else {
		blc_1024 = (min << 10) / 100U;
		blc_1024 = clamp(blc_1024, 1U, 1024U);
		attr.sched_util_min = (blc_1024 << 10) / 1280;
		attr.sched_util_max = (blc_1024 << 10) / 1280;
	}

	/* get task_struct */
	rcu_read_lock();
	p = find_task_by_vpid(pid);
	if (likely(p))
		get_task_struct(p);
	rcu_read_unlock();

	/* sched_setattr */
	if (likely(p)) {
		ret = sched_setattr(p, &attr);
		put_task_struct(p);
	}
}


/**
 * kalSetCpuBoost() - Set CPU boost parameters
 * @prAdapter: pointer to Adapter
 * @prBoostInfo: pointer to Boost Info to retrieve how to boost cpu
 *
 * This function provides parameters configuration for each boost status.
 *
 * About RPS, the system default value of the rps_cpus file is zero.
 * This disables RPS, so the CPU that handles the network interrupt also
 * processes the packet.
 * To enable RPS, configure the appropriate RPS CPU mask with the CPUs that
 * should process packets from the specified network device and receive queue.
 * If the network interrupt rate is extremely high, excluding the CPU that
 * handles network interrupts may also improve performance.
 */
void kalSetCpuBoost(struct ADAPTER *prAdapter,
		struct BOOST_INFO *prBoostInfo)
{
	DBGLOG(BSS, INFO, "kalSetCpuBoost skip in fpga\n");
}

#endif

uint32_t kalGetEmiMetOffset(void)
{
	return u4EmiMetOffset;
}

void kalSetEmiMetOffset(uint32_t newEmiMetOffset)
{
	u4EmiMetOffset = newEmiMetOffset;
}

void kalSetEmiMpuProtection(phys_addr_t emiPhyBase, bool enable)
{
}

void kalSetDrvEmiMpuProtection(phys_addr_t emiPhyBase, uint32_t offset,
			       uint32_t size)
{
}

int32_t kalCheckVcoreBoost(struct ADAPTER *prAdapter,
		uint8_t uBssIndex)
{
	DBGLOG(BSS, INFO, "Vcore boost checking skip in fpga\n");
	return 0;
}

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
static int32_t fpga_wlan_pinctrl_init(struct mt66xx_chip_info *chip_info)
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

	last_toggle_time = 0;
	last_wf_rst_pin_state = -1;

	wlan_pinctrl_action(chip_info, WLAN_PINCTRL_MSG_FUNC_PTA_UART_INIT);

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

static int32_t fpga_wlan_pinctrl_action(struct mt66xx_chip_info *chip_info,
	enum WLAN_PINCTRL_MSG msg)
{
	//struct pinctrl_state *pinctrl;
	uint8_t *name;
	int32_t ret = 0;
	int gpio = 0;

        struct platform_device *pdev;
        kalGetPlatDev(&pdev);
        if (!pdev) {
                DBGLOG(INIT, ERROR, "pdev is NULL\n");
                ret = -EINVAL;
		goto exit;
	}

	switch (msg) {
	case WLAN_PINCTRL_MSG_FUNC_ON:
		name = "wf_rst_on";
		ensure_rst_pin_min_wait_time(1);
		gpio = of_get_named_gpio(pdev->dev.of_node, "wf-rst-gpio", 0);
		if (!gpio) {
			DBGLOG(INIT, ERROR, "wifi on: get wf-rst-gpio failed\n");
			goto exit;
		}
		gpio_request(gpio, "wf-rst-gpio");
		gpio_set_value(gpio, 1);
		gpio_free(gpio);
		break;
	case WLAN_PINCTRL_MSG_FUNC_OFF:
		name = "wf_rst_off";
		ensure_rst_pin_min_wait_time(0);
		gpio = of_get_named_gpio(pdev->dev.of_node, "wf-rst-gpio", 0);
		if (!gpio) {
			DBGLOG(INIT, ERROR, "wifi off: get wf-rst-gpio failed\n");
			goto exit;
		}
		gpio_request(gpio, "wf-rst-gpio");
		gpio_set_value(gpio, 0);
		gpio_free(gpio);

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
	default:
		DBGLOG(INIT, ERROR,
			"Unknown msg: %d.\n",
			msg);
		ret = -EINVAL;
		goto exit;
	}

	DBGLOG(INIT, INFO,
		"pinctrl_select_state[%s] not support yet, used gpio\n", name);

exit:
	return ret;
}
#endif

#if (CFG_SUPPORT_HOST_OFFLOAD == 1)
u_int8_t kalIsSupportMawd(void)
{
	return TRUE;
}

u_int8_t kalIsSupportSdo(void)
{
	return TRUE;
}

u_int8_t kalIsSupportRro(void)
{
	return TRUE;
}
#endif

uint32_t kalGetLittleCpuMask(void)
{
	return CPU_LITTLE_CORE;
}

uint32_t kalGetBigCpuMask(void)
{
	return CPU_BIG_CORE;
}

int mtk_pcie_remove_port(int port)
{
	DBGLOG(INIT, INFO,
		"mtk_pcie_remove_port fake api\n");
	return 0;
}

int mtk_pcie_probe_port(int port)
{
	DBGLOG(INIT, INFO,
		"mtk_pcie_probe_port fake api\n");
	return 0;
}

int mtk_msi_unmask_to_other_mcu(
	struct irq_data *data, u32 group)
{
	DBGLOG(INIT, INFO,
		"mtk_msi_unmask_to_other_mcu fake api\n");
	return 0;
}
int mtk_pcie_hw_control_vote(
	int port, bool hw_mode_en, u8 who)
{
	DBGLOG(INIT, INFO,
		"mtk_pcie_hw_control_vote fake api\n");
	return 0;
}

u32 mtk_pcie_dump_link_info(int port)
{
	DBGLOG(INIT, INFO,
		"mtk_pcie_dump_link_info fake api\n");
	return 0;

}
u32 mtk_pcie_disable_data_trans(int port)
{
	DBGLOG(INIT, INFO,
		"mtk_pcie_disable_data_trans fake api\n");
	return 0;
}
