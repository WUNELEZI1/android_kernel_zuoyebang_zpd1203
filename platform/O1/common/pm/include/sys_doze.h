/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __SYS_DOZE_H__
#define __SYS_DOZE_H__

#if defined(__KERNEL__) && !defined(__NuttX__)
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#else
#include "pm/include/sys_doze_plat.h"
#endif

enum voter_id {
	DOZE_VOTER_AP = 0,
	DOZE_VOTER_LPCORE = 1,
	DOZE_VOTER_XCTRL_CPU = 2,
	DOZE_VOTER_TEE_SPI0 = 3,
	DOZE_VOTER_TEE_SPI1 = 4,
	DOZE_VOTER_TEE_SPI2 = 5,
	DOZE_VOTER_MAX = 6,
};

static const int sys_doze_voter[] = {SYS_CTRL_VOTER_BIT_AP,
				     SYS_CTRL_VOTER_BIT_LPCORE,
				     SYS_CTRL_VOTER_BIT_XCTRL_CPU,
				     SYS_CTRL_VOTER_BIT_TEE_SPI0,
				     SYS_CTRL_VOTER_BIT_TEE_SPI1,
				     SYS_CTRL_VOTER_BIT_TEE_SPI2,
				     SYS_CTRL_VOTER_BIT_RSV6,
				     SYS_CTRL_VOTER_BIT_RSV7,
				     SYS_CTRL_VOTER_BIT_RSV8,
				     SYS_CTRL_VOTER_BIT_RSV9,
				     SYS_CTRL_VOTER_BIT_RSV10,
				     SYS_CTRL_VOTER_BIT_RSV11,
				     SYS_CTRL_VOTER_BIT_RSV12,
				     SYS_CTRL_VOTER_BIT_RSV13,
				     SYS_CTRL_VOTER_BIT_RSV14,
				     SYS_CTRL_VOTER_BIT_RSV15};


#define DOZE_VOTER_AP_STR "acpu"
#define DOZE_VOTER_LPCORE_STR "lpcore"
#define DOZE_VOTER_XCTRL_CPU_STR "xcrl_cpu"
#define DOZE_VOTER_TEE_SPI0_STR "tee_spi1"
#define DOZE_VOTER_TEE_SPI1_STR "tee_spi2"
#define DOZE_VOTER_TEE_SPI2_STR "tee_spi3"

/* state and info save cnt */
#define SYS_DOZE_SAVE_CNT 4
#define SYS_DOZE_TIMEOUT 500

/* soft voter master in lpctrl */
/* the first voter lpctrl only use in sr */
#define VOTER_DOZLP_LPCTRL_SR 0
#define VOTER_DOZLP_SENSORHUB 1
#define VOTER_DOZLP_PERI_DVFS 2
#define VOTER_DOZLP_ADSP 3
#define VOTER_DOZLP_XRSE 4
#define VOTER_DOZLP_LPCTRL_WFI 5
#define VOTER_DOZLP_LPDC 6
#define VOTER_DOZLP_LPCORE_MAX 7

#define VOTER_DOZLP_STR_LPCTRL_SR "lpctrl_sr"
#define VOTER_DOZLP_STR_LPCTRL_WFI "lpctrl_wfi"
#define VOTER_DOZLP_STR_SENSORHUB "sensorhub"
#define VOTER_DOZLP_STR_PERI_DVFS "peri_dvfs"
#define VOTER_DOZLP_STR_ADSP "adsp"
#define VOTER_DOZLP_STR_XRSE "xrse"
#define VOTER_DOZLP_STR_LPDC "lpdc"
#define VOTER_DOZLP_STR_LP_MAX "voter_lp_max"

/* soft voter master in xctrl_cpu */
#define VOTER_DOZXC_CPU 0
#define VOTER_DOZXC_CPU_PROFILE 1
#define VOTER_DOZXC_NPU 2
#define VOTER_DOZXC_GPU 3
#define VOTER_DOZXC_XCTRL_MAX 4

#define VOTER_DOZXC_STR_CPU "cpu"
#define VOTER_DOZXC_STR_CPU_PROFILE "cpu_profile"
#define VOTER_DOZXC_STR_NPU "npu"
#define VOTER_DOZXC_STR_GPU "gpu"
#define VOTER_DOZXC_STR_XC_MAX "voter_xc_max"

/* soft voter master in acpu */
#define VOTER_DOZAP_USB  0
#define VOTER_DOZAP_BT  1
/* clk master in acpu */
#define VOTER_DOZAP_CLK_SPI4   2
#define VOTER_DOZAP_CLK_SPI5   3
#define VOTER_DOZAP_CLK_SPI6   4
#define VOTER_DOZAP_CLK_I2C0   5
#define VOTER_DOZAP_CLK_I2C1   6
#define VOTER_DOZAP_CLK_I2C2   7
#define VOTER_DOZAP_CLK_I2C3   8
#define VOTER_DOZAP_CLK_I2C4   9
#define VOTER_DOZAP_CLK_I2C5   10
#define VOTER_DOZAP_CLK_I2C6   11
#define VOTER_DOZAP_CLK_I2C9   12
#define VOTER_DOZAP_CLK_I2C10  13
#define VOTER_DOZAP_CLK_I2C11  14
#define VOTER_DOZAP_CLK_I2C12  15
#define VOTER_DOZAP_CLK_I2C13  16
#define VOTER_DOZAP_CLK_I2C20  17
#define VOTER_DOZAP_CLK_I3C0   18
#define VOTER_DOZAP_SWITCH     19
#define VOTER_DOZAP_CLK_PWM0   20
#define VOTER_DOZAP_CLK_PWM1   21
#define VOTER_DOZAP_TP_GPIO    22
#define VOTER_DOZAP_PERI_GPIO  23
#define VOTER_DOZAP_DP         24
#define VOTER_DOZAP_VEU        25
#define VOTER_DOZAP_DPU        26
#define VOTER_DOZAP_UART6      27
#define VOTER_DOZAP_AP_MAX     28

#define VOTER_DOZAP_STR_USB "USB"
#define VOTER_DOZAP_STR_BT "BT"
/* clk master str in acpu */
#define VOTER_DOZAP_STR_CLK_SPI4   "clk_spi4"
#define VOTER_DOZAP_STR_CLK_SPI5   "clk_spi5"
#define VOTER_DOZAP_STR_CLK_SPI6   "clk_spi6"
#define VOTER_DOZAP_STR_CLK_I2C0   "clk_i2c0"
#define VOTER_DOZAP_STR_CLK_I2C1   "clk_i2c1"
#define VOTER_DOZAP_STR_CLK_I2C2   "clk_i2c2"
#define VOTER_DOZAP_STR_CLK_I2C3   "clk_i2c3"
#define VOTER_DOZAP_STR_CLK_I2C4   "clk_i2c4"
#define VOTER_DOZAP_STR_CLK_I2C5   "clk_i2c5"
#define VOTER_DOZAP_STR_CLK_I2C6   "clk_i2c6"
#define VOTER_DOZAP_STR_CLK_I2C9   "clk_i2c9"
#define VOTER_DOZAP_STR_CLK_I2C10  "clk_i2c10"
#define VOTER_DOZAP_STR_CLK_I2C11  "clk_i2c11"
#define VOTER_DOZAP_STR_CLK_I2C12  "clk_i2c12"
#define VOTER_DOZAP_STR_CLK_I2C13  "clk_i2c13"
#define VOTER_DOZAP_STR_CLK_I2C20  "clk_i2c20"
#define VOTER_DOZAP_STR_CLK_I3C0   "clk_i3c0"
#define VOTER_DOZAP_STR_SWITCH     "switch"
#define VOTER_DOZAP_STR_CLK_PWM0   "clk_pwm0"
#define VOTER_DOZAP_STR_CLK_PWM1   "clk_pwm1"
#define VOTER_DOZAP_STR_TP_GPIO    "tp_gpio"
#define VOTER_DOZAP_STR_PERI_GPIO  "peri_gpio"
#define VOTER_DOZAP_STR_DP         "dp"
#define VOTER_DOZAP_STR_VEU        "veu"
#define VOTER_DOZAP_STR_DPU        "dpu"
#define VOTER_DOZAP_STR_UART6      "uart6"
#define VOTER_DOZAP_STR_AP_MAX "voter_ap_max"

struct doze_lpctrl_vote_info {
	/* soft voter cnt */
	unsigned int vote_cnt[VOTER_DOZLP_LPCORE_MAX];
	unsigned long long vote_time[VOTER_DOZLP_LPCORE_MAX][SYS_DOZE_SAVE_CNT];
	unsigned int hw_state[SYS_DOZE_SAVE_CNT];
	unsigned int enter_cnt[SYS_DOZE_SAVE_CNT];
	/* soft voter stat */
	unsigned char cur_voter[VOTER_DOZLP_LPCORE_MAX];
	/* hard voter */
	unsigned char lpctrl_vote;
};

struct doze_xctrl_vote_info {
	/* soft voter cnt */
	unsigned int vote_cnt[VOTER_DOZXC_XCTRL_MAX];
	unsigned long long vote_time[VOTER_DOZXC_XCTRL_MAX][SYS_DOZE_SAVE_CNT];
	unsigned int hw_state[SYS_DOZE_SAVE_CNT];
	unsigned int enter_cnt[SYS_DOZE_SAVE_CNT];
	/* soft voter stat */
	unsigned char cur_voter[VOTER_DOZXC_XCTRL_MAX];
	/* hard voter */
	unsigned char xctrl_vote;
};

struct doze_ap_vote_info {
	/* soft voter cnt */
	unsigned int vote_cnt[VOTER_DOZAP_AP_MAX];
	unsigned long long vote_time[VOTER_DOZAP_AP_MAX][SYS_DOZE_SAVE_CNT];
	unsigned int hw_state[SYS_DOZE_SAVE_CNT];
	unsigned int enter_cnt[SYS_DOZE_SAVE_CNT];
	/* soft voter stat */
	unsigned char cur_voter[VOTER_DOZAP_AP_MAX];
	/* hard voter */
	unsigned char ap_vote;
};
#endif
