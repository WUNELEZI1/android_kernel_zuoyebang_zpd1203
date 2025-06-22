/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_CLK_API_H__
#define __XRISP_CLK_API_H__

#include <linux/types.h>
#include <linux/clk.h>

enum xrisp_crg_clk_in_e {
	CLK_ISP_FUNC1,
	CLK_ISP_FUNC2,
	CLK_ISP_FUNC3,
	CLK_ISP_CRG_IN_MAX,
};

enum xrisp_clk_out_e {
	CLK_ISP2CSI_FE,
	CLK_ISP_CVE,
	CLK_ISP_PE,
	CLK_ISP_BE,
	CLK_ISP_FE_CORE2,
	CLK_ISP_FE_CORE1,
	CLK_ISP_FE_CORE0,
	CLK_ISP_FE_ROUTER,
	CLK_ISP_CRG_OUT_MAX,
};

enum xrisp_mcu_clk_out_e {
	CLK_ISP_MCU,
	CLK_ISP_MCU_BUS,
	CLK_ISP_MCU_APB,
	PCLK_MM2_ISP_MCU_DEBUG,
	XR_CLK_MM2_ISP_MCU_DEBUG_ATB,
	CLK_ISP_R82_PERI,
	CLK_ISP_GENERIC_TIMER,
	CLK_ISP_MCU_DMA,
	PCLK_ISP_UART,
	PCLK_ISP_I2C,
	PCLK_ISP_I3C,
	PCLK_ISP_TIMER,
	PCLK_ISP_WTD,
	CLK_ISP_REF,
	CLK_ISP_I2C,
	CLK_ISP_I3C,
	CLK_ISP_TIMER,
	CLK_ISP_WTD,
	ISP_MCU_OUT_MAX,
};

enum xrisp_debug_clk_out_e {
	PCLK_MM2_SUBSYS_R82_DEBUG_APB,
	PCLK_MM2_SUBCHIP_DEBUG_ATB,
	ISP_DEBUG_OUT_MAX,
};

enum xrisp_perf_clk_out_e {
	PCLK_ISP_PERF_BE,
	PCLK_ISP_PERF_PE,
	PCLK_ISP_PERF_CVE,
	PCLK_ISP_PERF_RTBU,
	PCLK_ISP_PERF_NTBU0,
	PCLK_ISP_PERF_NTBU1,
	PCLK_ISP_PERF_CMD,
	PCLK_ISP_PERF_MCU,
	ISP_PERF_OUT_MAX,
};

enum xrisp_clk_rate_e {
	XRISP_CLK_RATE_075V,
	XRISP_CLK_RATE_065V,
	XRISP_CLK_RATE_060V,
	XRISP_CLK_RATE_AON,
	XRISP_CLK_RATE_MAX,
};

enum xrisp_clk_peri_volt_e {
	XRISP_CLK_PERI_VOLT_080V,
	XRISP_CLK_PERI_VOLT_070V,
	XRISP_CLK_PERI_VOLT_065V,
	XRISP_CLK_PERI_VOLT_060V,
	XRISP_CLK_PERI_VOLT_MAX,
};

enum xrisp_mcu_clk_rate_e {
	XRISP_MCU_CLK_RATE_1200M,
	XRISP_MCU_CLK_RATE_557M,
	XRISP_MCU_CLK_RATE_MAX,
};

enum xrisp_clk_e {
	XRISP_CRG_CLK,
	XRISP_MCU_CLK,
	XRISP_DEBUG_CLK,
	XRISP_PERF_CLK,
	XRISP_CLK_MAX,
};

int xrisp_clk_api_enable(enum xrisp_clk_e clk, unsigned int mask);
void xrisp_clk_api_disable(enum xrisp_clk_e clk, unsigned int mask);
unsigned int xrisp_clk_api_get_stats(enum xrisp_clk_e clk);
int xrisp_clk_api_set_rate_by_mask(enum xrisp_clk_e clk, unsigned int mask, int rate_index);
int xrisp_clk_api_enable_init_rate(enum xrisp_clk_e clk, unsigned int mask, int rate_index);
int xrisp_clk_api_set_rate(enum xrisp_clk_e clk, int ch, unsigned int rate);
unsigned int xrisp_clk_api_get_rate(enum xrisp_clk_e clk, int ch);
void xrisp_clk_api_deinit_clk(enum xrisp_clk_e clk);

#endif
