// SPDX-License-Identifier: GPL-2.0 or later
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

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/syscore_ops.h>
#include <linux/types.h>
#include <dt-bindings/xring/platform-specific/dvs.h>
#include <dt-bindings/xring/platform-specific/hss1_crg.h>
#include <dt-bindings/xring/platform-specific/hss2_crg.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/media1_crg.h>
#include <dt-bindings/xring/platform-specific/media2_crg.h>
#include <dt-bindings/xring/platform-specific/lpis_crg.h>
#include <dt-bindings/xring/platform-specific/lms_crg.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
#include <dt-bindings/xring/xr-o1-clock.h>
#include <dt-bindings/xring/xr-clk-vote.h>
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"
#include "dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h"
#include <dt-bindings/xring/platform-specific/ip_regulator_define.h>
#include <soc/xring/xr-clk-provider.h>

#include "clk-o1.h"
#include "clk/xr-dvfs-private.h"
#include "clk/clk.h"
#include "xsp_ffa.h"

DEFINE_SPINLOCK(dpu_ppll2_lock);

static struct clk_pll_cfg_table ppll0_cfg = {
	.pll_id = XR_PPLL0,
	.vote_en = {PERI_CRG_PPLL0_VOTE_EN, PERI_CRG_PPLL0_VOTE_EN_PPLL0_VOTE_EN_SHIFT},
	.lock_ctrl = {PERI_CRG_PLL_LOCK_STATE, PERI_CRG_PLL_LOCK_STATE_PPLL0_LOCK_SHIFT},
};

static struct clk_pll_cfg_table ppll1_cfg = {
	.pll_id = XR_PPLL1,
	.vote_en = {PERI_CRG_PPLL1_VOTE_EN, PERI_CRG_PPLL1_VOTE_EN_PPLL1_VOTE_EN_SHIFT},
	.lock_ctrl = {PERI_CRG_PLL_LOCK_STATE, PERI_CRG_PLL_LOCK_STATE_PPLL1_LOCK_SHIFT},
};

static struct clk_pll_cfg_table ppll2_cfg = {
	.pll_id = XR_PPLL2,
	.vote_en = {PERI_CRG_PPLL2_VOTE_EN, PERI_CRG_PPLL2_VOTE_EN_PPLL2_VOTE_EN_SHIFT},
	.lock_ctrl = {PERI_CRG_PLL_LOCK_STATE, PERI_CRG_PLL_LOCK_STATE_PPLL2_LOCK_SHIFT},
};

static struct clk_pll_cfg_table ppll2_dpu_cfg = {
	.pll_id = XR_PPLL2_DPU,
	.vote_en = {PERI_CRG_PPLL2_VOTE_EN,
		PERI_CRG_PPLL2_VOTE_EN_PPLL2_VOTE_EN_SHIFT + PPLL2_DPU_VOTE_BIT},
	.lock_ctrl = {PERI_CRG_PLL_LOCK_STATE,
		PERI_CRG_PLL_LOCK_STATE_PPLL2_LOCK_SHIFT},
};

static struct clk_pll_cfg_table ppll3_cfg = {
	.pll_id = XR_PPLL3,
	.vote_en = {PERI_CRG_PPLL3_VOTE_EN, PERI_CRG_PPLL3_VOTE_EN_PPLL3_VOTE_EN_SHIFT},
	.lock_ctrl = {PERI_CRG_PLL_LOCK_STATE, PERI_CRG_PLL_LOCK_STATE_PPLL3_LOCK_SHIFT},
};

static struct clk_pll_cfg_table pcie_pll_cfg = {
	.pll_id = XR_PCIE_PLL,
	.vote_en = {HSS2_CRG_PCIE_PLL_VOTE_EN, HSS2_CRG_PCIE_PLL_VOTE_EN_PLL_VOTE_EN_SHIFT},
	.lock_ctrl = {HSS2_CRG_PCIE_PLL_LOCK_STATE, HSS2_CRG_PCIE_PLL_LOCK_STATE_PLL_LOCK_SHIFT},
};

static struct clk_pll_cfg_table dpu_pll_cfg = {
	.pll_id = XR_DPU_PLL,
	.vote_en = {HSS1_CRG_DPU_PLL_VOTE_EN, HSS1_CRG_DPU_PLL_VOTE_EN_DPU_PLL_VOTE_EN_SHIFT},
	.lock_ctrl = {HSS1_CRG_PLL_LOCK_STATE, HSS1_CRG_PLL_LOCK_STATE_DPU_PLL_LOCK_SHIFT},
};

static struct clk_pll_cfg_table dp_pll_cfg = {
	.pll_id = XR_DP_PLL,
	.vote_en = {HSS1_CRG_DP_PLL_VOTE_EN, HSS1_CRG_DP_PLL_VOTE_EN_DP_PLL_VOTE_EN_SHIFT},
	.lock_ctrl = {HSS1_CRG_PLL_LOCK_STATE, HSS1_CRG_PLL_LOCK_STATE_DP_PLL_LOCK_SHIFT},
};

/* list all pcie device compatible string */
static const char * const pcie_device_compt_string[] = {
	"xring,dw-pcie0",
	"xring,dw-pcie1",
};

/* output rate must be decreasing sequence */
static const struct clk_pll_rate_table ppll0_rate_table[] = {
	{
		.output_rate = 1350000000,
		.refdiv = 1,
		.fbdiv = 70,
		.frac = 5242880,
		.postdiv1 = 1,
		.postdiv2 = 0,
	},
};

static const struct clk_pll_rate_table ppll1_rate_table[] = {
	{
		.output_rate = 1200000000,
		.refdiv = 1,
		.fbdiv = 62,
		.frac = 8388608,
		.postdiv1 = 1,
		.postdiv2 = 0,
	},
};

/* output rate must be decreasing sequence */
static const struct clk_pll_rate_table ppll2_rate_table[] = {
	{
		.output_rate = 1450000000,
		.refdiv = 1,
		.fbdiv = 75,
		.frac = 8718733,
		.postdiv1 = 1,
		.postdiv2 = 0
	},
};

static const struct clk_pll_rate_table ppll3_rate_table[] = {
	{
		.output_rate = 960000000,
		.refdiv = 1,
		.fbdiv = 50,
		.frac = 0,
		.postdiv1 = 1,
		.postdiv2 = 0
	},
};

static const struct clk_pll_rate_table pcie_pll_rate_table[] = {
	{
		.output_rate = 100000000,
		.refdiv = 1,
		.fbdiv = 62,
		.frac = 8388608,
		.postdiv1 = 7,
		.postdiv2 = 2
	},
};


/* ======== Generated Mux Sel Array START,Do not modify by hand! ======== */

static const char * const clk_ocm_sw_sels[] = {
	"clk_ocm_gpll_div",	"clk_sys_peri",	"clk_ocm_lppll_div",
};

static const char * const clk_peri_timer0_mux_sels[] = {
	"clk_timer_h_ini",	"clkin_ref_peri",
};

static const char * const clk_peri_timer1_mux_sels[] = {
	"clk_timer_h_ini",	"clkin_ref_peri",
};

static const char * const clk_peri_timer2_mux_sels[] = {
	"clk_timer_h_ini",	"clkin_ref_peri",
};

static const char * const clk_peri_timer3_mux_sels[] = {
	"clk_timer_h_ini",	"clkin_ref_peri",
};

static const char * const clk_csi_sys_gpll_sels[] = {
	"clk_ppll1",	"clk_gpll_peri",	"clk_ppll2",
};

static const char * const clk_csi_sys_sw_sels[] = {
	"clk_csi_sys_gpll",	"clk_sys_peri",	"clk_lppll_peri",
};

static const char * const clk_dpu_veu_sw_sels[] = {
	"clk_ppll1_media1",	"clk_gpll_media1",	"clk_ppll2_media1",	"clk_ppll3_media1",
};

static const char * const clk_vdec_core_sw_sels[] = {
	"clk_ppll1_media1",	"clk_gpll_media1",	"clk_ppll2_media1",	"clk_ppll3_media1",
};

static const char * const clk_dpu_core_sw_sels[] = {
	"clk_ppll1_media1",	"clk_gpll_media1",	"clk_ppll2_dpu_media1",	"clk_ppll3_media1",
};

static const char * const clk_dpu_dsc0_mux_sels[] = {
	"clk_dpu_dsc0_div",	"clk_dpu_core2",
};

static const char * const clk_dpu_dsc1_mux_sels[] = {
	"clk_dpu_dsc1_div",	"clk_dpu_core2",
};

static const char * const aclk_media1_bus_sw_sels[] = {
	"clk_ppll1_media1",	"clk_gpll_media1",	"clk_ppll2_media1",	"clk_ppll3_media1",
};

static const char * const pclk_media1_cfg_sw_sels[] = {
	"clk_ppll1_media1",	"clk_gpll_media1",	"clk_ppll2_media1",	"clk_ppll3_media1",
};

static const char * const clk_isp_func1_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll2_media2",
};

static const char * const clk_isp_func2_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll2_media2",
};

static const char * const clk_isp_func3_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll2_media2",
};

static const char * const clk_isp_mcu_func_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const clk_media2_noc_data_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll2_media2",	"clk_ppll3_media2",
};

static const char * const clk_venc_core_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const aclk_media2_bus_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const pclk_media2_cfg_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const clk_isp_i2c_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const clk_isp_i3c_sw_sels[] = {
	"clk_ppll0_media2",	"clk_gpll_media2",	"clk_ppll1_media2",	"clk_ppll3_media2",
};

static const char * const clk_isp_timer_sw_sels[] = {
	"clk_isp_ref_ini",	"clkin_ref_media2",
};

static const char * const clk_cam_mux_sels[] = {
	"clk_gpll_lpis",	"clk_lppll_lpis",
};

static const char * const clk_cam0_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_cam1_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_cam2_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_cam3_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_cam4_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_cam5_mux_sels[] = {
	"clk_sys_div",	"clk_cam_div",
};

static const char * const clk_codec_mux_sels[] = {
	"clkin_sys",	"clk_gpll_lms",	"clk_lp32kpll",	"clk_lppll",
};

/* ======== Generated Mux Sel Array End,Do not modify by hand! ======== */
/***************** dvfs feature reserved ******************/
/* isp */
static const struct dvfs_cfg ispfunc1_dvfs_cfg = { { 557056, 600000, 725000 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  600000, 1 };
static const struct dvfs_cfg ispfunc2_dvfs_cfg = { { 417800, 600000, 725000 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  600000, 1 };
static const struct dvfs_cfg ispfunc3_dvfs_cfg = { { 417800, 600000, 725000 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  600000, 1 };

/* vdec */
static const struct dvfs_cfg vdec_dvfs_cfg = { { 278530, 417800, 835590 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  417800, 1 };

/* dpu */
static const struct dvfs_cfg dpu_dvfs_cfg = { { 362500, 483400, 557056 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  483400, 1 };
static const struct dvfs_cfg dpu_veu_dvfs_cfg = { { 362500, 483400, 557056 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  483400, 1 };

/* venc */
static const struct dvfs_cfg venc_dvfs_cfg = { { 334240, 600000, 960000 }, { 0, 1, 2 },
						MEDIA_DVFS_VOLT_LEVEL,  600000, 1 };

/* ocm */
static const struct dvfs_cfg ocm_dvfs_cfg = { { 334240, 417800, 557056, 557056}, { 0, 1, 2, 3 },
						PERI_DVFS_VOLT_LEVEL,  557056, 1 };

/***************** dvfs feature reserved ******************/

static struct clk_hw **g_hws;

/* for fast dfs clock */
struct fast_dfs_clock dpu_core_fast_dfs = {
	.profile_freq = { 26112000, 362500000, 483333333, 557056000 },
	.profile_pll  = { 1671168000, 1450000000, 1450000000, 1671168000 },
	.profile_div  = { 64, 4, 3, 3 },
	.profile_sw  = { 1, 2, 2, 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA1_CRG_CLKSW0,
			MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_CORE_SHIFT, 2 },
		.sw_state_cfg = { MEDIA1_CRG_CLKST0,
			MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_CORE_SHIFT, 4 },
		.scgt_cfg = { MEDIA1_CRG_CLKDIV1,
			MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_CORE_SHIFT },
		.div_cfg = { MEDIA1_CRG_CLKDIV1,
			MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_CORE_SHIFT, 6 },
		.div_state_cfg = { MEDIA1_CRG_CLKST1,
			MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_CORE_SHIFT },
	},
	.sw_sels = clk_dpu_core_sw_sels,
	.sw_freq  = { 1200000000, 1671168000, 1450000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_dpu_core_sw_sels),
	.sw_ids = { XR_CLK_PPLL1_MEDIA1, XR_CLK_GPLL_MEDIA1,
		XR_CLK_PPLL2_DPU_MEDIA1, XR_CLK_PPLL3_MEDIA1 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock dpu_veu_fast_dfs = {
	.profile_freq = { 26112000, 362500000, 483333333, 557056000 },
	.profile_pll  = { 1671168000, 1450000000, 1450000000, 1671168000 },
	.profile_div  = { 64, 4, 3, 3 },
	.profile_sw  = { 1, 2, 2, 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA1_CRG_CLKSW0,
			MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_VEU_SHIFT, 2 },
		.sw_state_cfg = { MEDIA1_CRG_CLKST0,
			MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_VEU_SHIFT, 4 },
		.scgt_cfg = { MEDIA1_CRG_CLKDIV1,
			MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_VEU_SHIFT },
		.div_cfg = { MEDIA1_CRG_CLKDIV1,
			MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_VEU_SHIFT, 6 },
		.div_state_cfg = { MEDIA1_CRG_CLKST1,
			MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_VEU_SHIFT },
	},
	.sw_sels = clk_dpu_veu_sw_sels,
	.sw_freq  = { 1200000000, 1671168000, 1450000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_dpu_veu_sw_sels),
	.sw_ids = { XR_CLK_PPLL1_MEDIA1, XR_CLK_GPLL_MEDIA1,
		XR_CLK_PPLL2_MEDIA1, XR_CLK_PPLL3_MEDIA1 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock aclk_media1_bus_fast_dfs = {
	.profile_freq = { 26112000, 417792000, 557056000, 725000000 },
	.profile_pll  = { 1671168000, 1671168000, 1671168000, 1450000000 },
	.profile_div  = { 64, 4, 3, 2 },
	.profile_sw  = { 1, 1, 1, 2 },
	.hw_cfg = {
		.sw_cfg = { MEDIA1_CRG_CLKSW0,
			MEDIA1_CRG_CLKSW0_SEL_ACLK_MEDIA1_BUS_SHIFT, 2 },
		.sw_state_cfg = { MEDIA1_CRG_CLKST0,
			MEDIA1_CRG_CLKST0_SW_ACK_ACLK_MEDIA1_BUS_SHIFT, 4 },
		.scgt_cfg = { 0, 0 },
		.div_cfg = { MEDIA1_CRG_CLKDIV2,
			MEDIA1_CRG_CLKDIV2_DIV_SW_ACLK_MEDIA1_BUS_SHIFT, 6 },
		.div_state_cfg = { MEDIA1_CRG_CLKST1,
			MEDIA1_CRG_CLKST1_DIV_DONE_ACLK_MEDIA1_BUS_SHIFT },
	},
	.sw_sels = aclk_media1_bus_sw_sels,
	.sw_freq  = { 1200000000, 1671168000, 1450000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(aclk_media1_bus_sw_sels),
	.sw_ids = { XR_CLK_PPLL1_MEDIA1, XR_CLK_GPLL_MEDIA1,
		XR_CLK_PPLL2_MEDIA1, XR_CLK_PPLL3_MEDIA1 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock pclk_media1_cfg_fast_dfs = {
	.profile_freq = { 26112000, 139264000, 185685333, 241666666 },
	.profile_pll  = { 1671168000, 1671168000, 1671168000, 1450000000 },
	.profile_div  = { 64, 12, 9, 6 },
	.profile_sw  = { 1, 1, 1, 2 },
	.hw_cfg = {
		.sw_cfg = { MEDIA1_CRG_CLKSW0,
			MEDIA1_CRG_CLKSW0_SEL_PCLK_MEDIA1_CFG_SHIFT, 2 },
		.sw_state_cfg = { MEDIA1_CRG_CLKST0,
			MEDIA1_CRG_CLKST0_SW_ACK_PCLK_MEDIA1_CFG_SHIFT, 4 },
		.scgt_cfg = { 0, 0 },
		.div_cfg = { MEDIA1_CRG_CLKDIV3,
			MEDIA1_CRG_CLKDIV3_DIV_SW_PCLK_MEDIA1_CFG_SHIFT, 6 },
		.div_state_cfg = { MEDIA1_CRG_CLKST1,
			MEDIA1_CRG_CLKST1_DIV_DONE_PCLK_MEDIA1_CFG_SHIFT },
	},
	.sw_sels = pclk_media1_cfg_sw_sels,
	.sw_freq  = { 1200000000, 1671168000, 1450000000, 960000000 },
	.sw_ids = { XR_CLK_PPLL1_MEDIA1, XR_CLK_GPLL_MEDIA1,
		XR_CLK_PPLL2_MEDIA1, XR_CLK_PPLL3_MEDIA1 },
	.sw_sel_num = ARRAY_SIZE(pclk_media1_cfg_sw_sels),
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock ispfunc1_fast_dfs = {
	.profile_freq = { 104448000, 208896000, 557056000, 600000000, 725000000 },
	.profile_pll  = { 1671168000, 1671168000, 1671168000, 1200000000, 1450000000 },
	.profile_div  = { 16, 8, 3, 2, 2 },
	.profile_sw  = { 1, 1, 1, 2, 3 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC1_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC1_SHIFT, 4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV4,
			MEDIA2_CRG_CLKDIV4_SC_GT_CLK_ISP_FUNC1_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV4,
			MEDIA2_CRG_CLKDIV4_DIV_CLK_ISP_FUNC1_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC1_SHIFT },
	},
	.sw_sels = clk_isp_func1_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 1450000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_func1_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL2_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock ispfunc2_fast_dfs = {
	.profile_freq = { 104448000, 208896000, 417792000, 600000000, 725000000 },
	.profile_pll  = { 1671168000, 1671168000, 1671168000, 1200000000, 1450000000 },
	.profile_div  = { 16, 8, 4, 2, 2 },
	.profile_sw  = { 1, 1, 1, 2, 3 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC2_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC2_SHIFT, 4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV5,
			MEDIA2_CRG_CLKDIV5_SC_GT_CLK_ISP_FUNC2_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV5,
			MEDIA2_CRG_CLKDIV5_DIV_CLK_ISP_FUNC2_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC2_SHIFT },
	},
	.sw_sels = clk_isp_func2_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 1450000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_func2_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL2_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock ispfunc3_fast_dfs = {
	.profile_freq = { 104448000, 208896000, 417792000, 600000000, 725000000 },
	.profile_pll  = { 1671168000, 1671168000, 1671168000, 1200000000, 1450000000 },
	.profile_div  = { 16, 8, 4, 2, 2 },
	.profile_sw  = { 1, 1, 1, 2, 3 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC3_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC3_SHIFT, 4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV5,
			MEDIA2_CRG_CLKDIV5_SC_GT_CLK_ISP_FUNC3_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV5,
			MEDIA2_CRG_CLKDIV5_DIV_CLK_ISP_FUNC3_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC3_SHIFT },
	},
	.sw_sels = clk_isp_func3_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 1450000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_func3_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL2_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock isp_mcu_func_fast_dfs = {
	.profile_freq = { 104448000, 557056000, 1200000000 },
	.profile_pll  = { 1671168000, 1671168000, 1200000000 },
	.profile_div  = { 16, 3, 1 },
	.profile_sw  = { 1, 1, 2 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_MCU_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_MCU_SHIFT,  4 },
		.scgt_cfg = {  MEDIA2_CRG_CLKDIV4,
			MEDIA2_CRG_CLKDIV4_SC_GT_CLK_ISP_MCU_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV4,
			MEDIA2_CRG_CLKDIV4_DIV_CLK_ISP_MCU_FUNC_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_MCU_SHIFT },
	},
	.sw_sels = clk_isp_mcu_func_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_mcu_func_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock isp_i2c_fast_dfs = {
	.profile_freq = { 104448000 },
	.profile_pll  = { 1671168000 },
	.profile_div  = { 16 },
	.profile_sw  = { 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_I2C_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_I2C_SHIFT,  4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV7,
			MEDIA2_CRG_CLKDIV7_SC_GT_CLK_ISP_I2C_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV7,
			MEDIA2_CRG_CLKDIV7_DIV_CLK_ISP_I2C_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_I2C_SHIFT },
	},
	.sw_sels = clk_isp_i2c_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_i2c_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock isp_i3c_fast_dfs = {
	.profile_freq = { 104448000, 128551384},
	.profile_pll  = { 1671168000, 1671168000 },
	.profile_div  = { 16, 13 },
	.profile_sw  = { 1, 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW1,
			MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_I3C_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_I3C_SHIFT,  4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV7,
			MEDIA2_CRG_CLKDIV7_SC_GT_CLK_ISP_I3C_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV7,
			MEDIA2_CRG_CLKDIV7_DIV_CLK_ISP_I3C_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_I3C_SHIFT },
	},
	.sw_sels = clk_isp_i3c_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_isp_i3c_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock media2_noc_data_fast_dfs = {
	.profile_freq = { 104448000, 557056000, 725000000, 960000000 },
	.profile_pll  = { 1671168000, 1671168000, 1450000000, 960000000 },
	.profile_div  = { 16, 3, 2, 1 },
	.profile_sw  = { 1, 1, 2, 3 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW2,
			MEDIA2_CRG_CLKSW2_SEL_CLK_MEDIA2_NOC_DATA_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_SW_ACK_CLK_MEDIA2_NOC_DATA_SHIFT, 4 },
		.scgt_cfg = { 0, 0 },
		.div_cfg = { MEDIA2_CRG_CLKDIV8,
			MEDIA2_CRG_CLKDIV8_DIV_CLK_MEDIA2_NOC_DATA_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_CLK_MEDIA2_NOC_DATA_SHIFT },
	},
	.sw_sels = clk_media2_noc_data_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1450000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_media2_noc_data_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL2_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock venc_core_fast_dfs = {
	.profile_freq = { 104448000, 334233600, 600000000, 960000000 },
	.profile_pll  = { 1671168000, 1671168000, 1200000000, 960000000 },
	.profile_div  = { 16, 5, 2, 1 },
	.profile_sw  = { 1, 1, 2, 3 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW0,
			MEDIA2_CRG_CLKSW0_SEL_CLK_VENC_CORE_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST0,
			MEDIA2_CRG_CLKST0_SW_ACK_CLK_VENC_CORE_SHIFT, 4 },
		.scgt_cfg = { MEDIA2_CRG_CLKDIV1,
			MEDIA2_CRG_CLKDIV1_SC_GT_CLK_VENC_CORE_SHIFT },
		.div_cfg = { MEDIA2_CRG_CLKDIV1,
			MEDIA2_CRG_CLKDIV1_DIV_CLK_VENC_CORE_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST1,
			MEDIA2_CRG_CLKST1_DIV_DONE_CLK_VENC_CORE_SHIFT },
	},
	.sw_sels = clk_venc_core_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(clk_venc_core_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_GATE,
};

struct fast_dfs_clock aclk_media2_bus_fast_dfs = {
	.profile_freq = { 104448000, 417792000, 600000000, 835584000 },
	.profile_pll  = { 1671168000, 1671168000, 1200000000, 1671168000 },
	.profile_div  = { 16, 4, 2, 2 },
	.profile_sw  = { 1, 1, 2, 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW2,
			MEDIA2_CRG_CLKSW2_SEL_ACLK_MEDIA2_BUS_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_ACLK_MEDIA2_BUS_SHIFT, 4 },
		.scgt_cfg = { 0, 0 },
		.div_cfg = { MEDIA2_CRG_CLKDIV3,
			MEDIA2_CRG_CLKDIV3_DIV_ACLK_MEDIA2_BUS_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_ACLK_MEDIA2_BUS_SHIFT },
	},
	.sw_sels = aclk_media2_bus_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(aclk_media2_bus_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2,
		XR_CLK_PPLL1_MEDIA2, XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

struct fast_dfs_clock pclk_media2_cfg_fast_dfs = {
	.profile_freq = { 104448000, 139264000, 200000000, 278528000 },
	.profile_pll  = { 1671168000, 1671168000, 1200000000, 1671168000 },
	.profile_div  = { 16, 12, 6, 6 },
	.profile_sw  = { 1, 1, 2, 1 },
	.hw_cfg = {
		.sw_cfg = { MEDIA2_CRG_CLKSW2,
			MEDIA2_CRG_CLKSW2_SEL_PCLK_MEDIA2_BUS_SHIFT, 2 },
		.sw_state_cfg = { MEDIA2_CRG_CLKST3,
			MEDIA2_CRG_CLKST3_SW_ACK_PCLK_MEDIA2_BUS_SHIFT, 4 },
		.scgt_cfg = { 0, 0 },
		.div_cfg = { MEDIA2_CRG_CLKDIV3,
			MEDIA2_CRG_CLKDIV3_DIV_PCLK_MEDIA2_CFG_SHIFT, 6 },
		.div_state_cfg = { MEDIA2_CRG_CLKST4,
			MEDIA2_CRG_CLKST4_DIV_DONE_PCLK_MEDIA2_BUS_SHIFT },
	},
	.sw_sels = pclk_media2_cfg_sw_sels,
	.sw_freq  = { 1350000000, 1671168000, 1200000000, 960000000 },
	.sw_sel_num = ARRAY_SIZE(pclk_media2_cfg_sw_sels),
	.sw_ids = { XR_CLK_PPLL0_MEDIA2, XR_CLK_GPLL_MEDIA2, XR_CLK_PPLL1_MEDIA2,
		XR_CLK_PPLL3_MEDIA2 },
	.gate_flag = FAST_DFS_SCGT_NO_GATE,
};

#ifdef CONFIG_PM_SLEEP
static int o1_clk_suspend(void)
{
	clk_save_context();
	return 0;
}

static void o1_clk_resume(void)
{
	clk_restore_context();
}
#endif
static struct syscore_ops o1_clk_syscore_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = o1_clk_suspend,
	.resume = o1_clk_resume,
#endif
};

static int xring_o1_clocks_probe(struct platform_device *pdev)
{
	struct clk_hw_onecell_data **clk_hw_data = xr_clk_hw_data_get();
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	void __iomem *base;
	int power_stat;
	int ret;

	clkinfo("start o1 clock probe!\n");
	*clk_hw_data = devm_kzalloc(dev, struct_size(*clk_hw_data, hws, XR_CLK_END), GFP_KERNEL);
	if (WARN_ON(!(*clk_hw_data)))
		return -ENOMEM;

	ret = of_clk_add_hw_provider(np, of_clk_hw_onecell_get, *clk_hw_data);
	if (ret < 0) {
		clkerr("failed to register clks for XRing, %d!\n", ret);
		return ret;
	}

	(*clk_hw_data)->num = XR_CLK_END;
	g_hws = (*clk_hw_data)->hws;

	g_hws[XR_CLKIN_REF] = devm_xr_clk_hw_fixed(dev, "clkin_ref", 32764);
	g_hws[XR_CLKIN_SYS] = devm_xr_clk_hw_fixed(dev, "clkin_sys", 38400000);

	/* LMS_CRG */
	g_hws[XR_CLK_LP32KPLL] = devm_xr_clk_hw_fixed(dev, "clk_lp32kpll", 49147000);
	g_hws[XR_CLK_LPPLL] = devm_xr_clk_hw_fixed(dev, "clk_lppll", 491520000);

	/* PERI  */
	g_hws[XR_CLK_GPLL] = devm_xr_clk_hw_fixed(dev, "clk_gpll", 1671168000);
	g_hws[XR_CLK_SYS_PERI] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_peri", "clkin_sys",
		1, 1);

	g_hws[XR_CLK_REF_PERI] = devm_xr_clk_hw_fixed_factor(dev, "clkin_ref_peri", "clkin_ref",
		1, 1);

	g_hws[XR_CLK_LPPLL_PERI] = devm_xr_clk_hw_fixed_factor(dev, "clk_lppll_peri",
		"clk_lppll", 1, 1);

	g_hws[XR_CLK_PPLL0_FAKE] = devm_xr_clk_hw_fixed_factor(dev, "clk_ppll0_fake",
		"clk_sys_peri", 1, 1);

	g_hws[XR_CLK_PPLL1_FAKE] = devm_xr_clk_hw_fixed_factor(dev, "clk_ppll1_fake",
		"clk_sys_peri", 1, 1);

	g_hws[XR_CLK_PPLL2_FAKE] = devm_xr_clk_hw_fixed_factor(dev, "clk_ppll2_fake",
		"clk_sys_peri", 1, 1);

	g_hws[XR_CLK_PPLL3_FAKE] = devm_xr_clk_hw_fixed_factor(dev, "clk_ppll3_fake",
		"clk_sys_peri", 1, 1);

	base = get_xr_clk_base(XR_CLK_CRGCTRL);
	ppll0_cfg.base = base;
	g_hws[XR_CLK_PPLL0] = devm_xr_clk_hw_ppll(dev, "clk_ppll0", "clk_ppll0_fake", &ppll0_cfg,
		ppll0_rate_table, ARRAY_SIZE(ppll0_rate_table), 0, PLL_FLAGS_FIXED_RATE);

	ppll1_cfg.base = base;
	g_hws[XR_CLK_PPLL1] = devm_xr_clk_hw_ppll(dev, "clk_ppll1", "clk_ppll1_fake", &ppll1_cfg,
		ppll1_rate_table, ARRAY_SIZE(ppll1_rate_table), 0, PLL_FLAGS_FIXED_RATE);

	ppll2_cfg.base = base;
	g_hws[XR_CLK_PPLL2] = devm_xr_clk_hw_ppll(dev, "clk_ppll2", "clk_ppll2_fake", &ppll2_cfg,
		ppll2_rate_table, ARRAY_SIZE(ppll2_rate_table), 0, PLL_FLAGS_FIXED_RATE);

	ppll2_dpu_cfg.base = base;
	g_hws[XR_CLK_PPLL2_DPU] = devm_xr_clk_hw_ppll(dev, "clk_ppll2_dpu", "clk_sys_peri",
		&ppll2_dpu_cfg, ppll2_rate_table, ARRAY_SIZE(ppll2_rate_table), 0, PLL_FLAGS_FIXED_RATE);

	ppll3_cfg.base = base;
	g_hws[XR_CLK_PPLL3] = devm_xr_clk_hw_ppll(dev, "clk_ppll3", "clk_ppll3_fake", &ppll3_cfg,
		ppll3_rate_table, ARRAY_SIZE(ppll3_rate_table), 0, PLL_FLAGS_FIXED_RATE);

	/* HSS1 */
	g_hws[XR_CLK_SYS_HSS1] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_hss1", "clkin_sys",
		1, 1);

	g_hws[XR_CLK_REF_HSS1] = devm_xr_clk_hw_fixed_factor(dev, "clkin_ref_hss1", "clkin_sys",
		1, 1);

	g_hws[XR_CLK_SYS_DIV2] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_div2", "clk_sys_hss1",
		1, 2);

	g_hws[XR_CLK_PLL_LOGIC] = devm_xr_clk_hw_fixed_factor(dev, "clk_pll_logic", "clkin_sys",
		1, 32);

	g_hws[XR_CLK_HSS1] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1", "clk_gpll_peri", 1, 4);

	/* HSS2 */
	base = get_xr_clk_base(XR_CLK_HSIF2CRG);
	g_hws[XR_CLK_SYS_HSS2] = devm_xr_clk_hw_fixed(dev, "clk_sys_hss2", 38400000);
	g_hws[XR_CLKIN_REF_HSS2] = devm_xr_clk_hw_fixed_factor(dev, "clkin_ref_hss2", "clkin_ref",
		1, 1);

	pcie_pll_cfg.base = base;

	parse_pciepll_vco_rate(pcie_device_compt_string, ARRAY_SIZE(pcie_device_compt_string));

	g_hws[XR_CLK_PCIE_PLL] = devm_xr_clk_hw_ppll(dev, "clk_pcie_pll", "clk_pcie_pll_logic",
		&pcie_pll_cfg, pcie_pll_rate_table, ARRAY_SIZE(pcie_pll_rate_table),
		SAFE_PLL_PCIE_PLL, PLL_FLAGS_FIXED_RATE | PLL_FLAGS_SOFT_CTRL);

	g_hws[XR_CLK_HSS2] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss2", "clk_gpll_peri", 1, 4);

	/* MEDIA1 */
	base = get_xr_clk_base(XR_CLK_MEDIA1CRG);
	g_hws[XR_CLK_SYS_MEDIA1] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_media1",
		"clkin_sys", 1, 1);

	g_hws[XR_CLKIN_REF_MEDIA1] = devm_xr_clk_hw_fixed_factor(dev, "clkin_ref_media1",
		"clkin_ref", 1, 1);

	/* MEDIA2 */
	base = get_xr_clk_base(XR_CLK_MEDIA2CRG);
	g_hws[XR_CLKIN_REF_MEDIA2] = devm_xr_clk_hw_fixed_factor(dev, "clkin_ref_media2",
		"clkin_ref", 1, 1);

	g_hws[XR_CLK_SYS_MEDIA2] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_media2",
		"clkin_sys", 1, 1);

/* ======== Generated Register Code Start,Do not modify by hand! ======== */

	/*****HSS2_CRG*****/
	base = get_xr_clk_base(XR_CLK_HSIF2CRG);
	g_hws[XR_CLK_HSS2_BUS_CFG_DIV] = devm_xr_clk_hw_divider(dev, "clk_hss2_bus_cfg_div",
		"clk_hss2", base + HSS2_CRG_CLKDIV0,
		HSS2_CRG_CLKDIV0_DIV_CLK_HSS2_BUS_CFG_SHIFT, 6,
		base + HSS2_CRG_CLKST0,
		HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_CFG_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_HSS2_BUS_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_hss2_bus_cfg_ini", "clk_hss2_bus_cfg_div", 1, 1);

	g_hws[XR_CLK_HSS2_BUS_DATA_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_hss2_bus_data_sc_gt",
		"clk_hss2", base + HSS2_CRG_CLKDIV1,
		HSS2_CRG_CLKDIV1_SC_GT_CLK_HSS2_BUS_DATA_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_HSS2_BUS_DATA_DIV] = devm_xr_clk_hw_divider(dev, "clk_hss2_bus_data_div",
		"clk_hss2_bus_data_sc_gt", base + HSS2_CRG_CLKDIV1,
		HSS2_CRG_CLKDIV1_DIV_CLK_HSS2_BUS_DATA_SHIFT, 6,
		base + HSS2_CRG_CLKST0,
		HSS2_CRG_CLKST0_DIV_DONE_CLK_HSS2_BUS_DATA_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_HSS2_BUS_DATA_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_hss2_bus_data_ini", "clk_hss2_bus_data_div", 1, 1);

	g_hws[XR_CLK_HSS2_BUS_DATA] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss2_bus_data",
		"clk_hss2_bus_data_ini", 1, 1);

	g_hws[XR_CLK_CFGBUS_HSS2] = devm_xr_clk_hw_fixed_factor(dev, "clk_cfgbus_hss2",
		"clk_cfgbus_up", 1, 1);

	g_hws[XR_PCLK_PCIE0_APB] = devm_xr_clk_hw_gt(dev, "pclk_pcie0_apb",
		"clk_hss2_bus_cfg_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_PCIE0_APB_SHIFT, 0, 0);

	g_hws[XR_PCLK_PCIE1_APB] = devm_xr_clk_hw_gt(dev, "pclk_pcie1_apb",
		"clk_hss2_bus_cfg_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_PCIE1_APB_SHIFT, 0, 0);

	g_hws[XR_PCLK_EMMC] = devm_xr_clk_hw_gt(dev, "pclk_emmc", "clk_hss2_bus_cfg_ini", NULL,
		base, HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_W1S_GT_PCLK_EMMC_SHIFT, 0, 0);

	g_hws[XR_PCLK_HSS2_TCU_SLV] = devm_xr_clk_hw_gt(dev, "pclk_hss2_tcu_slv",
		"clk_hss2_bus_cfg_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_HSS2_TCU_SLV_SHIFT, 0, 0);

	g_hws[XR_ACLK_PCIE0_AXI_SLV] = devm_xr_clk_hw_gt(dev, "aclk_pcie0_axi_slv",
		"clk_hss2_bus_cfg_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE0_AXI_SLV_SHIFT, 0, 0);

	g_hws[XR_ACLK_PCIE1_AXI_SLV] = devm_xr_clk_hw_gt(dev, "aclk_pcie1_axi_slv",
		"clk_hss2_bus_cfg_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE1_AXI_SLV_SHIFT, 0, 0);

	g_hws[XR_ACLK_HSS2_TCU] = devm_xr_clk_hw_gt(dev, "aclk_hss2_tcu",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TCU_SHIFT, 0, 0);

	g_hws[XR_ACLK_HSS2_TBU0] = devm_xr_clk_hw_gt(dev, "aclk_hss2_tbu0",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TBU0_SHIFT, 0, 0);

	g_hws[XR_ACLK_HSS2_TBU1] = devm_xr_clk_hw_gt(dev, "aclk_hss2_tbu1",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_HSS2_TBU1_SHIFT, 0, 0);

	g_hws[XR_ACLK_PCIE0_AXI_MST] = devm_xr_clk_hw_gt(dev, "aclk_pcie0_axi_mst",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE0_AXI_MST_SHIFT, 0, 0);

	g_hws[XR_ACLK_PCIE1_AXI_MST] = devm_xr_clk_hw_gt(dev, "aclk_pcie1_axi_mst",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PCIE1_AXI_MST_SHIFT, 0, 0);

	g_hws[XR_ACLK_PERFMON_PCIE0] = devm_xr_clk_hw_gt(dev, "aclk_perfmon_pcie0",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_PCIE0_SHIFT, 0, 0);

	g_hws[XR_ACLK_PERFMON_PCIE1] = devm_xr_clk_hw_gt(dev, "aclk_perfmon_pcie1",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_PCIE1_SHIFT, 0, 0);

	g_hws[XR_ACLK_PERFMON_TCU] = devm_xr_clk_hw_gt(dev, "aclk_perfmon_tcu",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_ACLK_PERFMON_TCU_SHIFT, 0, 0);

	g_hws[XR_CLK_PCIE0_FW_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_pcie0_fw_sc_gt",
		"clk_hss2", base + HSS2_CRG_CLKDIV2,
		HSS2_CRG_CLKDIV2_SC_GT_CLK_PCIE0_FW_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_PCIE0_FW_DIV] = devm_xr_clk_hw_divider(dev, "clk_pcie0_fw_div",
		"clk_pcie0_fw_sc_gt", base + HSS2_CRG_CLKDIV2,
		HSS2_CRG_CLKDIV2_DIV_CLK_PCIE0_FW_SHIFT, 6, base + HSS2_CRG_CLKST0,
		HSS2_CRG_CLKST0_DIV_DONE_CLK_PCIE0_FW_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_PCIE0_FW_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_pcie0_fw_ini",
		"clk_pcie0_fw_div", 1, 1);

	g_hws[XR_CLK_PCIE0_FW] = devm_xr_clk_hw_gt(dev, "clk_pcie0_fw",
		"clk_pcie0_fw_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_SW_CLK_PCIE0_FW_SHIFT, 0, 0);

	g_hws[XR_CLK_PCIE1_FW] = devm_xr_clk_hw_gt(dev, "clk_pcie1_fw",
		"clk_pcie0_fw_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_SW_CLK_PCIE1_FW_SHIFT, 0, 0);

	g_hws[XR_HCLK_EMMC_SC_GT] = devm_xr_clk_hw_scgt(dev, "hclk_emmc_sc_gt", "clk_hss2",
		base + HSS2_CRG_CLKDIV3, HSS2_CRG_CLKDIV3_SC_GT_HCLK_EMMC_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_HCLK_EMMC_DIV] = devm_xr_clk_hw_divider(dev, "hclk_emmc_div",
		"hclk_emmc_sc_gt", base + HSS2_CRG_CLKDIV3,
		HSS2_CRG_CLKDIV3_DIV_HCLK_EMMC_SHIFT, 6, base + HSS2_CRG_CLKST0,
		HSS2_CRG_CLKST0_DIV_DONE_HCLK_EMMC_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_HCLK_EMMC_INI] = devm_xr_clk_hw_fixed_factor(dev, "hclk_emmc_ini",
		"hclk_emmc_div", 1, 1);

	g_hws[XR_HCLK_EMMC] = devm_xr_clk_hw_gt(dev, "hclk_emmc", "hclk_emmc_ini", NULL, base,
		HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_W1S_GT_HCLK_EMMC_SHIFT, 0, 0);

	g_hws[XR_HCLK_EMMC_SLV] = devm_xr_clk_hw_fixed_factor(dev, "hclk_emmc_slv",
		"clk_hss2_bus_cfg_ini", 1, 1);

	g_hws[XR_CLK_EMMC_CCLK_BASE_SC_GT] = devm_xr_clk_hw_scgt(dev,
		"clk_emmc_cclk_base_sc_gt", "clk_hss2", base + HSS2_CRG_CLKDIV4,
		HSS2_CRG_CLKDIV4_SC_GT_CLK_EMMC_CCLK_BASE_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_EMMC_CCLK_BASE_DIV] = devm_xr_clk_hw_divider(dev, "clk_emmc_cclk_base_div",
		"clk_emmc_cclk_base_sc_gt", base + HSS2_CRG_CLKDIV4,
		HSS2_CRG_CLKDIV4_DIV_CLK_EMMC_CCLK_BASE_SHIFT, 6, base + HSS2_CRG_CLKST0,
		HSS2_CRG_CLKST0_DIV_DONE_CLK_EMMC_CCLK_BASE_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_EMMC_CCLK_BASE_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_emmc_cclk_base_ini", "clk_emmc_cclk_base_div", 1, 1);

	g_hws[XR_CLK_EMMC_TMCLK_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_emmc_tmclk_ini",
		"clkin_ref_hss2", 1, 1);

	g_hws[XR_CLK_HSS2_BUS_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss2_bus_ref",
		"clk_emmc_tmclk_ini", 1, 1);

	g_hws[XR_CLK_PCIE0_AUX_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_pcie0_aux_ini",
		"clk_pcie_aux", 1, 1);

	g_hws[XR_CLK_PCIE0_AUX] = devm_xr_clk_hw_gt(dev, "clk_pcie0_aux", "clk_pcie0_aux_ini",
		NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE0_AUX_SHIFT, 0, 0);

	g_hws[XR_CLK_PCIE1_AUX] = devm_xr_clk_hw_gt(dev, "clk_pcie1_aux", "clk_pcie0_aux_ini",
		NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE1_AUX_SHIFT, 0, 0);

	g_hws[XR_CLK_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_ref", "clk_sys_hss2", 1, 1);

	g_hws[XR_CLK_PCIE_PLL_LOGIC] = devm_xr_clk_hw_gt(dev, "clk_pcie_pll_logic",
		"clk_pll_logic", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE_PLL_LOGIC_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERFMON_PCIE0] = devm_xr_clk_hw_gt(dev, "pclk_perfmon_pcie0",
		"pclk_perfmon_peri", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PCIE0_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERFMON_PCIE1] = devm_xr_clk_hw_gt(dev, "pclk_perfmon_pcie1",
		"pclk_perfmon_peri", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PCIE1_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERFMON_TCU] = devm_xr_clk_hw_gt(dev, "pclk_perfmon_tcu",
		"pclk_perfmon_peri", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_TCU_SHIFT, 0, 0);

	g_hws[XR_CLK_PCIE0_TPC] = devm_xr_clk_hw_gt(dev, "clk_pcie0_tpc",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE0_TPC_SHIFT, 0, 0);

	g_hws[XR_CLK_PCIE1_TPC] = devm_xr_clk_hw_gt(dev, "clk_pcie1_tpc",
		"clk_hss2_bus_data_ini", NULL, base, HSS2_CRG_CLKGT0_W1S,
		HSS2_CRG_CLKGT0_W1S_GT_CLK_PCIE1_TPC_SHIFT, 0, 0);

	g_hws[XR_CLK_TCU_TPC] = devm_xr_clk_hw_gt(dev, "clk_tcu_tpc", "clk_hss2_bus_data_ini",
		NULL, base, HSS2_CRG_CLKGT0_W1S, HSS2_CRG_CLKGT0_W1S_GT_CLK_TCU_TPC_SHIFT,
		0, 0);

	/*****HSS1_CRG*****/
	base = get_xr_clk_base(XR_CLK_HSIF1CRG);
	dpu_pll_cfg.base = base;
	g_hws[XR_CLK_DPU_PLL] = devm_xr_clk_hw_ppll(dev, "clk_dpu_pll", "clkin_sys",
		&dpu_pll_cfg, NULL, 0, SAFE_PLL_DPU_PLL, PLL_FLAGS_DYNAMIC_RATE);

	dp_pll_cfg.base = base;
	g_hws[XR_CLK_DP_PLL] = devm_xr_clk_hw_ppll(dev, "clk_dp_pll", "clkin_sys",
		&dp_pll_cfg, NULL, 0, SAFE_PLL_DP_PLL, PLL_FLAGS_DYNAMIC_RATE);

	g_hws[XR_CLK_HSS1_BUS_DATA_SC_GT] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1_bus_data_sc_gt",
		"clk_hss1", 1, 1);

	g_hws[XR_CLK_HSS1_BUS_DATA_DIV] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1_bus_data_div",
		"clk_hss1_bus_data_sc_gt", 1, 1);

	g_hws[XR_CLK_HSS1_BUS_DATA_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_hss1_bus_data_ini", "clk_hss1_bus_data_div", 1, 1);

	g_hws[XR_CLK_HSS1_BUS_DATA] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1_bus_data",
		"clk_hss1_bus_data_ini", 1, 1);

	g_hws[XR_CLK_HSS1_BUS_CFG_DIV] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1_bus_cfg_div",
		"clk_hss1", 1, 2);

	g_hws[XR_CLK_HSS1_BUS_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_hss1_bus_cfg_ini", "clk_hss1_bus_cfg_div", 1, 1);

	g_hws[XR_CLK_HSS1_BUS_CFG] = devm_xr_clk_hw_fixed_factor(dev, "clk_hss1_bus_cfg",
		"clk_hss1_bus_cfg_ini", 1, 1);

	g_hws[XR_CLK_BUS_REF_HSS1] = devm_xr_clk_hw_fixed_factor(dev, "clk_bus_ref_hss1",
		"clkin_ref_hss1", 1, 1);

	g_hws[XR_CLK_UFS_CORE_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_ufs_core_sc_gt",
		"clk_hss1", base + HSS1_CRG_CLKDIV1,
		HSS1_CRG_CLKDIV1_SC_GT_CLK_UFS_CORE_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_UFS_CORE_DIV] = devm_xr_clk_hw_divider(dev, "clk_ufs_core_div",
		"clk_ufs_core_sc_gt", base + HSS1_CRG_CLKDIV1,
		HSS1_CRG_CLKDIV1_DIV_CLK_UFS_CORE_SHIFT, 6, base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_CLK_UFS_CORE_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_UFS_CORE_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_ufs_core_ini",
		"clk_ufs_core_div", 1, 1);

	g_hws[XR_CLK_UFS_CORE] = devm_xr_clk_hw_gt(dev, "clk_ufs_core", "clk_ufs_core_ini",
		NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_UFS_CORE_SHIFT, 0, 0);

	g_hws[XR_PCLK_UFS_APB] = devm_xr_clk_hw_gt(dev, "pclk_ufs_apb", "clk_hss1_bus_cfg_ini",
		NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_UFS_APB_SHIFT, 0, 0);

	g_hws[XR_CLK_UFS_MPHY_CFG_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_ufs_mphy_cfg_sc_gt",
		"clk_hss1", base + HSS1_CRG_CLKDIV2,
		HSS1_CRG_CLKDIV2_SC_GT_CLK_UFS_MPHY_CFG_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_UFS_MPHY_CFG_DIV] = devm_xr_clk_hw_divider(dev, "clk_ufs_mphy_cfg_div",
		"clk_ufs_mphy_cfg_sc_gt", base + HSS1_CRG_CLKDIV2,
		HSS1_CRG_CLKDIV2_DIV_CLK_UFS_MPHY_CFG_SHIFT, 6, base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_CLK_UFS_MPHY_CFG_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_UFS_MPHY_CFG] = devm_xr_clk_hw_gt(dev, "clk_ufs_mphy_cfg",
		"clk_ufs_mphy_cfg_div", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_UFS_MPHY_CFG_SHIFT, 0, 0);

	g_hws[XR_PCLK_UFS_PERF_STAT] = devm_xr_clk_hw_gt(dev, "pclk_ufs_perf_stat",
		"pclk_perfmon_peri", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_UFS_PERF_STAT_SHIFT, 0, 0);

	g_hws[XR_ACLK_UFS_PERF_STAT] = devm_xr_clk_hw_gt(dev, "aclk_ufs_perf_stat",
		"clk_ufs_core_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_ACLK_UFS_PERF_STAT_SHIFT, 0, 0);

	g_hws[XR_PCLK_EUSB_ABRG_SLV] = devm_xr_clk_hw_gt(dev, "pclk_eusb_abrg_slv",
		"clk_hss1_bus_cfg_ini", NULL, base,  HSS1_CRG_CLKGT2_W1S,
		HSS1_CRG_CLKGT2_W1S_GT_PCLK_EUSB_ABRG_SLV_SHIFT, 0, 0);

	g_hws[XR_ACLK_UFSCTRL_TPC] = devm_xr_clk_hw_gt(dev, "aclk_ufsctrl_tpc",
		"clk_ufs_core_ini", NULL, base, HSS1_CRG_CLKGT2_W1S,
		HSS1_CRG_CLKGT2_W1S_GT_ACLK_UFSCTRL_TPC_SHIFT, 0, 0);

	g_hws[XR_PCLK_HSS1_SCTRL] = devm_xr_clk_hw_gt(dev, "pclk_hss1_sctrl",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_HSS1_SCTRL_SHIFT, 0, 0);

	g_hws[XR_PCLK_USB_COMBOPHY_APB0] = devm_xr_clk_hw_gt(dev, "pclk_usb_combophy_apb0",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_USB_COMBOPHY_APB0_SHIFT, 0, 0);

	g_hws[XR_PCLK_USB_COMBOPHY_APB1] = devm_xr_clk_hw_gt(dev, "pclk_usb_combophy_apb1",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_USB_COMBOPHY_APB1_SHIFT, 0, 0);

	g_hws[XR_PCLK_USB_COMBOPHY_TCA] = devm_xr_clk_hw_gt(dev, "pclk_usb_combophy_tca",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_USB_COMBOPHY_TCA_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPTX_TRNG_APB_SC_GT] = devm_xr_clk_hw_scgt(dev, "pclk_dptx_trng_apb_sc_gt",
		"clk_hss1_bus_cfg_ini", base + HSS1_CRG_CLKDIV1,
		HSS1_CRG_CLKDIV1_SC_GT_PCLK_DPTX_TRNG_APB_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_PCLK_DPTX_TRNG_APB_DIV] = devm_xr_clk_hw_divider(dev, "pclk_dptx_trng_apb_div",
		"pclk_dptx_trng_apb_sc_gt", base + HSS1_CRG_CLKDIV1,
		HSS1_CRG_CLKDIV1_DIV_PCLK_DPTX_TRNG_APB_SHIFT, 6,
		base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_PCLK_DPTX_TRNG_APB_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_PCLK_USB_EUSB_APB] = devm_xr_clk_hw_gt(dev, "pclk_usb_eusb_apb",
		"pclk_dptx_trng_apb_div", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_USB_EUSB_APB_SHIFT, 0, 0);

	g_hws[XR_PCLK_USB_SUBSYS_SCTRL_APB] = devm_xr_clk_hw_gt(dev,
		"pclk_usb_subsys_sctrl_apb", "clk_hss1_bus_cfg_ini", NULL, base,
		HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_USB_SUBSYS_SCTRL_APB_SHIFT, 0, 0);

	g_hws[XR_CLK_USB_BUS_EARLY_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_usb_bus_early_sc_gt",
		"clk_hss1", base + HSS1_CRG_CLKDIV2,
		HSS1_CRG_CLKDIV2_SC_GT_CLK_USB_BUS_EARLY_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_USB_BUS_EARLY_DIV] = devm_xr_clk_hw_divider(dev, "clk_usb_bus_early_div",
		"clk_usb_bus_early_sc_gt", base + HSS1_CRG_CLKDIV2,
		HSS1_CRG_CLKDIV2_DIV_CLK_USB_BUS_EARLY_SHIFT, 6,
		base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_CLK_USB_BUS_EARLY_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_USB_BUS_EARLY_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_usb_bus_early_ini", "clk_usb_bus_early_div", 1, 1);

	g_hws[XR_CLK_USB_BUS_EARLY] = devm_xr_clk_hw_gt(dev, "clk_usb_bus_early",
		"clk_usb_bus_early_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_USB_BUS_EARLY_SHIFT, 0, 0);

	g_hws[XR_CLK_USB_COMBOPHY_FW] = devm_xr_clk_hw_gt(dev, "clk_usb_combophy_fw",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_USB_COMBOPHY_FW_SHIFT, 0, 0);

	g_hws[XR_CLK_USB_COMPHY_SUSPEND] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_usb_comphy_suspend", "clkin_ref_hss1", 1, 1);

	g_hws[XR_CLK_USB_CONTROLLER_SUSPEND] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_usb_controller_suspend", "clkin_ref_hss1", 1, 1);

	g_hws[XR_CLK_USB_REF_ALT] = devm_xr_clk_hw_gt(dev, "clk_usb_ref_alt", "clk_sys_hss1",
		NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_USB_REF_ALT_SHIFT, 0, 0);

	g_hws[XR_CLK_USB_EUSB_REF] = devm_xr_clk_hw_gt(dev, "clk_usb_eusb_ref", "clk_sys_div2",
		NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_USB_EUSB_REF_SHIFT, 0, 0);

	g_hws[XR_CLK_USB_CONTROLLER_REF_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_usb_controller_ref_ini", "clk_sys_div2", 1, 1);

	g_hws[XR_CLK_USB_CONTROLLER_REF] = devm_xr_clk_hw_gt(dev, "clk_usb_controller_ref",
		"clk_usb_controller_ref_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_USB_CONTROLLER_REF_SHIFT, 0, 0);

	g_hws[XR_ACLK_USBCTRL_TPC] = devm_xr_clk_hw_gt(dev, "aclk_usbctrl_tpc",
		"clk_usb_bus_early_ini", NULL, base, HSS1_CRG_CLKGT2_W1S,
		HSS1_CRG_CLKGT2_W1S_GT_ACLK_USBCTRL_TPC_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPTX_CONTROLLER_APB0] = devm_xr_clk_hw_gt(dev,
		"pclk_dptx_controller_apb0", "clk_hss1_bus_cfg_ini", NULL, base,
		HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_DPTX_CONTROLLER_APB0_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPTX_EXT_SDP_APB1] = devm_xr_clk_hw_gt(dev, "pclk_dptx_ext_sdp_apb1",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_DPTX_EXT_SDP_APB1_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPTX_MESSAGEBUS] = devm_xr_clk_hw_gt(dev, "pclk_dptx_messagebus",
		"clk_hss1_bus_cfg_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_PCLK_DPTX_MESSAGEBUS_SHIFT, 0, 0);

	g_hws[XR_CLK_DPTX_AUX16MHZ_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_dptx_aux16mhz_sc_gt",
		"clk_hss1", base + HSS1_CRG_CLKDIV3,
		HSS1_CRG_CLKDIV3_SC_GT_CLK_DPTX_AUX16MHZ_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_DPTX_AUX16MHZ_DIV1] = devm_xr_clk_hw_divider(dev, "clk_dptx_aux16mhz_div1",
		"clk_dptx_aux16mhz_sc_gt", base + HSS1_CRG_CLKDIV3,
		HSS1_CRG_CLKDIV3_DIV_CLK_DPTX_AUX16MHZ_DIV1_SHIFT, 2,
		base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_CLK_DPTX_AUX16MHZ_DIV1_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_DPTX_AUX16MHZ_DIV] = devm_xr_clk_hw_divider(dev, "clk_dptx_aux16mhz_div",
		"clk_dptx_aux16mhz_div1", base + HSS1_CRG_CLKDIV3,
		HSS1_CRG_CLKDIV3_DIV_CLK_DPTX_AUX16MHZ_SHIFT, 6,
		base + HSS1_CRG_PERISTAT0,
		HSS1_CRG_PERISTAT0_DIV_DONE_CLK_DPTX_AUX16MHZ_DIV1_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_DPTX_AUX16MHZ_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_dptx_aux16mhz_ini", "clk_dptx_aux16mhz_div", 1, 1);

	g_hws[XR_CLK_DPTX_AUX16MHZ] = devm_xr_clk_hw_gt(dev, "clk_dptx_aux16mhz",
		"clk_dptx_aux16mhz_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_DPTX_AUX16MHZ_SHIFT, 0, 0);

	g_hws[XR_CLK_DPTX_ESM_AUX16MHZ] = devm_xr_clk_hw_gt(dev, "clk_dptx_esm_aux16mhz",
		"clk_dptx_aux16mhz_ini", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_CLK_DPTX_ESM_AUX16MHZ_SHIFT, 0, 0);

	g_hws[XR_ACLK_UFSCTRL_ABRG_SLV] = devm_xr_clk_hw_gt(dev, "aclk_ufsctrl_abrg_slv",
		"clk_ufs_core", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_ACLK_UFSCTRL_ABRG_SLV_SHIFT, 0, 0);

	g_hws[XR_ACLK_UFSESI_ABRG_SLV] = devm_xr_clk_hw_gt(dev, "aclk_ufsesi_abrg_slv",
		"clk_ufs_core", NULL, base, HSS1_CRG_CLKGT0_W1S,
		HSS1_CRG_CLKGT0_W1S_GT_ACLK_UFSESI_ABRG_SLV_SHIFT, 0, 0);

	g_hws[XR_CLK_SYS_DSI] = devm_xr_clk_hw_gt(dev, "clk_sys_dsi", "clk_sys_hss1", NULL,
		base, HSS1_CRG_CLKGT0_W1S, HSS1_CRG_CLKGT0_W1S_GT_CLK_SYS_DSI_SHIFT, 0,	0);

	g_hws[XR_PCLK_HSS1_DEBUG] = devm_xr_clk_hw_fixed_factor(dev, "pclk_hss1_debug",
		"clk_hss1_bus_cfg", 1, 1);

	/*****PERI_CRG*****/
	base = get_xr_clk_base(XR_CLK_CRGCTRL);

	g_hws[XR_CLK_GPLL_PERI] = devm_xr_clk_hw_fixed_factor(dev, "clk_gpll_peri", "clk_gpll",
		1, 1);

	g_hws[XR_CLK_GPLL_UP] = devm_xr_clk_hw_fixed_factor(dev, "clk_gpll_up", "clk_gpll_peri",
		1, 1);

	g_hws[XR_CLK_GPLL_MEDIA1] = devm_xr_clk_hw_gt(dev, "clk_gpll_media1",
		"clk_gpll", NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_GPLL_MEDIA1_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_GPLL_MEDIA2] = devm_xr_clk_hw_gt(dev, "clk_gpll_media2",
		"clk_gpll", NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_GPLL_MEDIA2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_CFGBUS_SW] = devm_xr_clk_hw_fixed_factor(dev, "clk_cfgbus_sw", "clk_gpll",
		1, 7);

	g_hws[XR_CLK_CFGBUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_cfgbus_ini",
		"clk_cfgbus_sw", 1, 1);

	g_hws[XR_CLK_CFGBUS_UP] = devm_xr_clk_hw_fixed_factor(dev, "clk_cfgbus_up",
		"clk_cfgbus_ini", 1, 1);

	g_hws[XR_CLK_DEBUG_APB_CORE] = devm_xr_clk_hw_fixed_factor(dev, "clk_debug_apb_core",
		"clk_gpll", 1, 16);

	g_hws[XR_CLK_PERICFG_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_pericfg_bus_ini",
		"clk_cfgbus_sw", 1, 2);

	/* MEDIA2 TO PERI */
	g_hws[XR_CLK_OCM_LPPLL_DIV_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_ocm_lppll_div_sc_gt",
		"clk_lppll_peri", base + PERI_CRG_CLKDIV28,
		PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_LPPLL_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_OCM_LPPLL_DIV] = devm_xr_clk_hw_divider(dev, "clk_ocm_lppll_div",
		"clk_ocm_lppll_div_sc_gt", base + PERI_CRG_CLKDIV28,
		PERI_CRG_CLKDIV28_DIV_CLK_OCM_LPPLL_SHIFT, 6, base + PERI_CRG_PERISTAT2,
		PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_LPPLL_DIV_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_OCM_GPLL_DIV_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_ocm_gpll_div_sc_gt",
		"clk_gpll_peri", base + PERI_CRG_CLKDIV28,
		PERI_CRG_CLKDIV28_SC_GT_CLK_OCM_GPLL_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_OCM_GPLL_DIV] = devm_xr_clk_hw_divider(dev, "clk_ocm_gpll_div",
		"clk_ocm_gpll_div_sc_gt", base + PERI_CRG_CLKDIV28,
		PERI_CRG_CLKDIV28_DIV_CLK_OCM_GPLL_SHIFT, 6, base + PERI_CRG_PERISTAT2,
		PERI_CRG_PERISTAT2_DIV_DONE_CLK_OCM_GPLL_DIV_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_OCM_SW] = devm_xr_clk_hw_mux(dev, "clk_ocm_sw", base + PERI_CRG_CLKDIV4,
		PERI_CRG_CLKDIV4_SEL_CLK_OCM_SHIFT, 2, CLK_MUX_HIWORD_MASK,
		clk_ocm_sw_sels, ARRAY_SIZE(clk_ocm_sw_sels), base + PERI_CRG_PERISTAT3,
		PERI_CRG_PERISTAT3_SW_ACK_CLK_OCM_SHIFT, 3);

	g_hws[XR_CLK_OCM_INI] = devm_xr_clk_hw_dvfs_gt(dev, "clk_ocm_ini",
		"clk_ocm_sw", NULL, base, 0, 0, 0, 0, XR_OCM_DVS_CFG);

	g_hws[XR_CLK_OCM_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ocm_dvfs", "clk_ocm_ini", NULL, base,
		PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_OCM_SHIFT, 0, 0);

	/* peri dvfs clocks */
	g_hws[XR_CLK_OCM] = devm_xr_clk_hw_dvfs(dev, "clk_ocm", &g_hws[XR_CLK_OCM_DVFS],
		CLK_OCM_VOTE_ID, PERI_DVFS_VOTE, &ocm_dvfs_cfg);

	g_hws[XR_PCLK_PERI_TIMER_NS] = devm_xr_clk_hw_gt(dev, "pclk_peri_timer_ns",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_TIMER_NS_SHIFT, 0, 0);

	g_hws[XR_CLK_TIMER_H_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_timer_h_sc_gt",
		"clk_sys_peri", base + PERI_CRG_CLKDIV0,
		PERI_CRG_CLKDIV0_SC_GT_CLK_TIMER_H_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_TIMER_H_DIV] = devm_xr_clk_hw_divider(dev, "clk_timer_h_div",
		"clk_timer_h_sc_gt", base + PERI_CRG_CLKDIV0,
		PERI_CRG_CLKDIV0_DIV_CLK_TIMER_H_SHIFT, 6, base + PERI_CRG_PERISTAT0,
		PERI_CRG_PERISTAT0_DIV_DONE_CLK_TIMER_H_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_TIMER_H_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_timer_h_ini",
		"clk_timer_h_div", 1, 1);

	g_hws[XR_CLK_PERI_TIMER0_MUX] = devm_xr_clk_hw_mux(dev, "clk_peri_timer0_mux",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER0_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_peri_timer0_mux_sels,
		ARRAY_SIZE(clk_peri_timer0_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_PERI_TIMER0] = devm_xr_clk_hw_gt(dev, "clk_peri_timer0",
		"clk_peri_timer0_mux", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER0_SHIFT, 0, 0);

	g_hws[XR_CLK_PERI_TIMER1_MUX] = devm_xr_clk_hw_mux(dev, "clk_peri_timer1_mux",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER1_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_peri_timer1_mux_sels,
		ARRAY_SIZE(clk_peri_timer1_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_PERI_TIMER1] = devm_xr_clk_hw_gt(dev, "clk_peri_timer1",
		"clk_peri_timer1_mux", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER1_SHIFT, 0, 0);

	g_hws[XR_CLK_PERI_TIMER2_MUX] = devm_xr_clk_hw_mux(dev, "clk_peri_timer2_mux",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER2_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_peri_timer2_mux_sels,
		ARRAY_SIZE(clk_peri_timer2_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_PERI_TIMER2] = devm_xr_clk_hw_gt(dev, "clk_peri_timer2",
		"clk_peri_timer2_mux", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER2_SHIFT, 0, 0);

	g_hws[XR_CLK_PERI_TIMER3_MUX] = devm_xr_clk_hw_mux(dev, "clk_peri_timer3_mux",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_PERI_TIMER3_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_peri_timer3_mux_sels,
		ARRAY_SIZE(clk_peri_timer3_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_PERI_TIMER3] = devm_xr_clk_hw_gt(dev, "clk_peri_timer3",
		"clk_peri_timer3_mux", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_TIMER3_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERI_WDT0] = devm_xr_clk_hw_gt(dev, "pclk_peri_wdt0",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_WDT0_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERI_WDT1] = devm_xr_clk_hw_gt(dev, "pclk_peri_wdt1",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_WDT1_SHIFT, 0, 0);

	g_hws[XR_CLK_PERI_WDT0] = devm_xr_clk_hw_gt(dev, "clk_peri_wdt0", "clkin_ref_peri",
		NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_WDT0_SHIFT, 0, 0);

	g_hws[XR_CLK_PERI_WDT1] = devm_xr_clk_hw_gt(dev, "clk_peri_wdt1", "clkin_ref_peri",
		NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PERI_WDT1_SHIFT, 0, 0);

	g_hws[XR_PCLK_SPI4] = devm_xr_clk_hw_gt(dev, "pclk_spi4", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI4_SHIFT, 0, 0);

	g_hws[XR_PCLK_SPI5] = devm_xr_clk_hw_gt(dev, "pclk_spi5", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI5_SHIFT, 0, 0);

	g_hws[XR_PCLK_SPI6] = devm_xr_clk_hw_gt(dev, "pclk_spi6", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_PCLK_SPI6_SHIFT, 0, 0);

	g_hws[XR_CLK_IP_SW] = devm_xr_clk_hw_fixed_factor(dev, "clk_ip_sw", "clk_gpll_peri", 1,
		1);

	g_hws[XR_CLK_SPI4_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_spi4_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV1, PERI_CRG_CLKDIV1_SC_GT_CLK_SPI4_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_SPI4_DIV] = devm_xr_clk_hw_divider(dev, "clk_spi4_div", "clk_spi4_sc_gt",
		base + PERI_CRG_CLKDIV1, PERI_CRG_CLKDIV1_DIV_CLK_SPI4_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI4_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_SPI4_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_spi4_ini",
		"clk_spi4_div", 1, 1);

	g_hws[XR_CLK_SPI4] = devm_xr_clk_doze_gt(dev, "clk_spi4", "clk_spi4_ini", NULL, base,
		PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_SPI4_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_SPI4);

	g_hws[XR_CLK_SPI5_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_spi5_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV1, PERI_CRG_CLKDIV1_SC_GT_CLK_SPI5_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_SPI5_DIV] = devm_xr_clk_hw_divider(dev, "clk_spi5_div", "clk_spi5_sc_gt",
		base + PERI_CRG_CLKDIV1, PERI_CRG_CLKDIV1_DIV_CLK_SPI5_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI5_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_SPI5_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_spi5_ini",
		"clk_spi5_div", 1, 1);

	g_hws[XR_CLK_SPI5] = devm_xr_clk_doze_gt(dev, "clk_spi5", "clk_spi5_ini", NULL, base,
		PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_SPI5_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_SPI5);

	g_hws[XR_CLK_SPI6_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_spi6_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV2, PERI_CRG_CLKDIV2_SC_GT_CLK_SPI6_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_SPI6_DIV] = devm_xr_clk_hw_divider(dev, "clk_spi6_div", "clk_spi6_sc_gt",
		base + PERI_CRG_CLKDIV2, PERI_CRG_CLKDIV2_DIV_CLK_SPI6_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_SPI6_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_SPI6_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_spi6_ini",
		"clk_spi6_div", 1, 1);

	g_hws[XR_CLK_SPI6] = devm_xr_clk_doze_gt(dev, "clk_spi6", "clk_spi6_ini", NULL, base,
		PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_SPI6_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_SPI6);

	g_hws[XR_PCLK_I2C0] = devm_xr_clk_hw_gt(dev, "pclk_i2c0", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C0_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C1] = devm_xr_clk_hw_gt(dev, "pclk_i2c1", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C1_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C2] = devm_xr_clk_hw_gt(dev, "pclk_i2c2", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C2_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C3] = devm_xr_clk_hw_gt(dev, "pclk_i2c3", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C3_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C4] = devm_xr_clk_hw_gt(dev, "pclk_i2c4", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C4_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C5] = devm_xr_clk_hw_gt(dev, "pclk_i2c5", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C5_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C6] = devm_xr_clk_hw_gt(dev, "pclk_i2c6", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C6_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C9] = devm_xr_clk_hw_gt(dev, "pclk_i2c9", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C9_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C10] = devm_xr_clk_hw_gt(dev, "pclk_i2c10", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C10_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C11] = devm_xr_clk_hw_gt(dev, "pclk_i2c11", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C11_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C12] = devm_xr_clk_hw_gt(dev, "pclk_i2c12", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C12_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C13] = devm_xr_clk_hw_gt(dev, "pclk_i2c13", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C13_SHIFT, 0, 0);

	g_hws[XR_PCLK_I2C20] = devm_xr_clk_hw_gt(dev, "pclk_i2c20", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I2C20_SHIFT, 0, 0);

	g_hws[XR_CLK_I2C_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_i2c_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV12, PERI_CRG_CLKDIV12_SC_GT_CLK_I2C_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_I2C_DIV] = devm_xr_clk_hw_divider(dev, "clk_i2c_div", "clk_i2c_sc_gt",
		base + PERI_CRG_CLKDIV12, PERI_CRG_CLKDIV12_DIV_CLK_I2C_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_I2C_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_I2C_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_i2c_ini", "clk_i2c_div",
		1, 1);

	g_hws[XR_CLK_I2C0] = devm_xr_clk_doze_gt(dev, "clk_i2c0", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C0_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C0);

	g_hws[XR_CLK_I2C1] = devm_xr_clk_doze_gt(dev, "clk_i2c1", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C1_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C1);

	g_hws[XR_CLK_I2C2] = devm_xr_clk_doze_gt(dev, "clk_i2c2", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C2_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C2);

	g_hws[XR_CLK_I2C3] = devm_xr_clk_doze_gt(dev, "clk_i2c3", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C3_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C3);

	g_hws[XR_CLK_I2C4] = devm_xr_clk_doze_gt(dev, "clk_i2c4", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C4_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C4);

	g_hws[XR_CLK_I2C5] = devm_xr_clk_doze_gt(dev, "clk_i2c5", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C5_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C5);

	g_hws[XR_CLK_I2C6] = devm_xr_clk_doze_gt(dev, "clk_i2c6", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C6_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C6);

	g_hws[XR_CLK_I2C9] = devm_xr_clk_doze_gt(dev, "clk_i2c9", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C9_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C9);

	g_hws[XR_CLK_I2C10] = devm_xr_clk_doze_gt(dev, "clk_i2c10", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C10_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C10);

	g_hws[XR_CLK_I2C11] = devm_xr_clk_doze_gt(dev, "clk_i2c11", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C11_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C11);

	g_hws[XR_CLK_I2C12] = devm_xr_clk_doze_gt(dev, "clk_i2c12", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C12_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C12);

	g_hws[XR_CLK_I2C13] = devm_xr_clk_doze_gt(dev, "clk_i2c13", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C13_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C13);

	g_hws[XR_CLK_I2C20] = devm_xr_clk_doze_gt(dev, "clk_i2c20", "clk_i2c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I2C20_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I2C20);

	g_hws[XR_PCLK_I3C0] = devm_xr_clk_hw_gt(dev, "pclk_i3c0", "clk_pericfg_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_I3C0_SHIFT, 0, 0);

	g_hws[XR_CLK_I3C_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_i3c_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV15, PERI_CRG_CLKDIV15_SC_GT_CLK_I3C_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_I3C_DIV] = devm_xr_clk_hw_divider(dev, "clk_i3c_div", "clk_i3c_sc_gt",
		base + PERI_CRG_CLKDIV15, PERI_CRG_CLKDIV15_DIV_CLK_I3C_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_I3C_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_I3C_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_i3c_ini", "clk_i3c_div",
		1, 1);

	g_hws[XR_CLK_I3C0] = devm_xr_clk_doze_gt(dev, "clk_i3c0", "clk_i3c_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_I3C0_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_I3C0);

	g_hws[XR_PCLK_PERI_TZPC] = devm_xr_clk_hw_fixed_factor(dev, "pclk_peri_tzpc",
		"clk_pericfg_bus_ini", 1, 1);

	g_hws[XR_PCLK_PERI_SPINLOCK] = devm_xr_clk_hw_gt(dev, "pclk_peri_spinlock",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_SPINLOCK_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERI_IPC1] = devm_xr_clk_hw_gt(dev, "pclk_peri_ipc1",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_IPC1_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERI_IPC2] = devm_xr_clk_hw_gt(dev, "pclk_peri_ipc2",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERI_IPC2_SHIFT, 0, 0);

	g_hws[XR_CLK_BUS_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_bus_ref",
		"clkin_ref_peri", 1, 1);

	g_hws[XR_PCLK_UART3_ABRG] = devm_xr_clk_hw_gt(dev, "pclk_uart3_abrg",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT1_W1S,
		PERI_CRG_CLKGT1_W1S_GT_PCLK_UART3_ABRG_SHIFT, 0, 0);

	g_hws[XR_PCLK_UART6_ABRG] = devm_xr_clk_hw_gt(dev, "pclk_uart6_abrg",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT1_W1S,
		PERI_CRG_CLKGT1_W1S_GT_PCLK_UART6_ABRG_SHIFT, 0, 0);

	g_hws[XR_CLK_UART_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_uart_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV3, PERI_CRG_CLKDIV3_SC_GT_CLK_UART_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_UART_DIV] = devm_xr_clk_hw_divider(dev, "clk_uart_div", "clk_uart_sc_gt",
		base + PERI_CRG_CLKDIV3, PERI_CRG_CLKDIV3_DIV_CLK_UART_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_UART_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_UART_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_uart_ini",
		"clk_uart_div", 1, 1);

	g_hws[XR_CLK_UART3] = devm_xr_clk_hw_gt(dev, "clk_uart3", "clk_uart_ini", NULL, base,
		PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_CLK_UART3_SHIFT, 0, 0);

	g_hws[XR_CLK_UART6] = devm_xr_clk_hw_gt(dev, "clk_uart6", "clk_uart_ini", NULL, base,
		PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_CLK_UART6_SHIFT, 0, 0);

	g_hws[XR_PCLK_PWM0_ABRG] = devm_xr_clk_hw_gt(dev, "pclk_pwm0_abrg",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT1_W1S,
		PERI_CRG_CLKGT1_W1S_GT_PCLK_PWM0_ABRG_SHIFT, 0, 0);

	g_hws[XR_PCLK_PWM1_ABRG] = devm_xr_clk_hw_gt(dev, "pclk_pwm1_abrg",
		"clk_pericfg_bus_ini", NULL, base, PERI_CRG_CLKGT1_W1S,
		PERI_CRG_CLKGT1_W1S_GT_PCLK_PWM1_ABRG_SHIFT, 0, 0);

	g_hws[XR_CLK_PWM_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_pwm_sc_gt", "clk_ip_sw",
		base + PERI_CRG_CLKDIV3, PERI_CRG_CLKDIV3_SC_GT_CLK_PWM_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_PWM_DIV] = devm_xr_clk_hw_divider(dev, "clk_pwm_div", "clk_pwm_sc_gt",
		base + PERI_CRG_CLKDIV3, PERI_CRG_CLKDIV3_DIV_CLK_PWM_SHIFT, 6,
		base + PERI_CRG_PERISTAT0, PERI_CRG_PERISTAT0_DIV_DONE_CLK_PWM_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_PWM_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_pwm_ini", "clk_pwm_div",
		1, 1);

	g_hws[XR_CLK_PWM0] = devm_xr_clk_doze_gt(dev, "clk_pwm0", "clk_pwm_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_PWM0_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_PWM0);

	g_hws[XR_CLK_PWM1] = devm_xr_clk_doze_gt(dev, "clk_pwm1", "clk_pwm_ini", NULL, base,
		PERI_CRG_CLKGT2_W1S, PERI_CRG_CLKGT2_W1S_GT_CLK_PWM1_SHIFT, 0, 0,
		VOTER_DOZAP_CLK_PWM1);

	g_hws[XR_CLK_DMA_BUS_DATA_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_dma_bus_data_ini", "clk_gpll", 1, 4);

	g_hws[XR_PCLK_DMA_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "pclk_dma_bus_ini",
		"clk_gpll", 1, 8);

	g_hws[XR_PCLK_DMA_CFG_ABRG_MST] = devm_xr_clk_hw_fixed_factor(dev,
		"pclk_dma_cfg_abrg_mst", "pclk_dma_bus_ini", 1, 1);

	g_hws[XR_CLK_DMA_S] = devm_xr_clk_hw_gt(dev, "clk_dma_s", "clk_dma_bus_data_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_CLK_DMA_S_SHIFT, 0, 0);

	g_hws[XR_CLK_DMA_NS] = devm_xr_clk_hw_gt(dev, "clk_dma_ns", "clk_dma_bus_data_ini",
		NULL, base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_CLK_DMA_NS_SHIFT,
		0, 0);

	g_hws[XR_PCLK_DMA_S] = devm_xr_clk_hw_gt(dev, "pclk_dma_s", "pclk_dma_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_DMA_S_SHIFT, 0, 0);

	g_hws[XR_PCLK_DMA_NS] = devm_xr_clk_hw_gt(dev, "pclk_dma_ns", "pclk_dma_bus_ini", NULL,
		base, PERI_CRG_CLKGT1_W1S, PERI_CRG_CLKGT1_W1S_GT_PCLK_DMA_NS_SHIFT, 0,
		0);

	g_hws[XR_CLK_PERFMON] = devm_xr_clk_hw_gt(dev, "clk_perfmon", "clk_dma_bus_data_ini",
		NULL, base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_PERFMON_SHIFT,
		0, 0);

	g_hws[XR_PCLK_PERFMON] = devm_xr_clk_hw_gt(dev, "pclk_perfmon", "pclk_dma_bus_ini",
		NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERFMON_PERI] = devm_xr_clk_hw_gt(dev, "pclk_perfmon_peri",
		"pclk_dma_bus_ini", NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFMON_PERI_SHIFT, 0, 0);

	g_hws[XR_PCLK_PERFSTAT] = devm_xr_clk_hw_gt(dev, "pclk_perfstat", "pclk_dma_bus_ini",
		NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_PCLK_PERFSTAT_SHIFT, 0, 0);

	g_hws[XR_CLK_CSI_SYS_GPLL] = devm_xr_clk_hw_mux(dev, "clk_csi_sys_gpll",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_GPLL_SHIFT, 2,
		CLK_MUX_HIWORD_MASK | CLK_DYNAMIC_POFF_AREA, clk_csi_sys_gpll_sels,
		ARRAY_SIZE(clk_csi_sys_gpll_sels), base + PERI_CRG_PERISTAT3,
		PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_GPLL_SHIFT, 3);

	g_hws[XR_CLK_CSI_SYS_SW] = devm_xr_clk_hw_mux(dev, "clk_csi_sys_sw",
		base + PERI_CRG_CLKDIV4, PERI_CRG_CLKDIV4_SEL_CLK_CSI_SYS_SHIFT, 2,
		CLK_MUX_HIWORD_MASK | CLK_DYNAMIC_POFF_AREA, clk_csi_sys_sw_sels, ARRAY_SIZE(clk_csi_sys_sw_sels),
		base + PERI_CRG_PERISTAT3, PERI_CRG_PERISTAT3_SW_ACK_CLK_CSI_SYS_SHIFT, 3);

	g_hws[XR_CLK_CSI_SYS_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_csi_sys_sc_gt",
		"clk_csi_sys_sw", base + PERI_CRG_CLKDIV14,
		PERI_CRG_CLKDIV14_SC_GT_CLK_CSI_SYS_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_CSI_SYS_DIV] = devm_xr_clk_hw_divider(dev, "clk_csi_sys_div",
		"clk_csi_sys_sc_gt", base + PERI_CRG_CLKDIV14,
		PERI_CRG_CLKDIV14_DIV_CLK_CSI_SYS_SHIFT, 6, base + PERI_CRG_PERISTAT1,
		PERI_CRG_PERISTAT1_DIV_DONE_CLK_CSI_SYS_SHIFT,
		CLK_DIVIDER_HIWORD_MASK | CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_CSI_SYS_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_csi_sys_ini",
		"clk_csi_sys_div", 1, 1);

	g_hws[XR_CLK_CSI_SYS] = devm_xr_clk_hw_gt(dev, "clk_csi_sys", "clk_csi_sys_ini", NULL,
		base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_CSI_SYS_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DSI_SYS_SW] = devm_xr_clk_hw_fixed_factor(dev, "clk_dsi_sys_sw",
		"clk_gpll_peri", 1, 1);

	g_hws[XR_CLK_DSI_SYS_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_dsi_sys_sc_gt",
		"clk_dsi_sys_sw", base + PERI_CRG_CLKDIV16,
		PERI_CRG_CLKDIV16_SC_GT_CLK_DSI_SYS_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_DSI_SYS_DIV] = devm_xr_clk_hw_divider(dev, "clk_dsi_sys_div",
		"clk_dsi_sys_sc_gt", base + PERI_CRG_CLKDIV16,
		PERI_CRG_CLKDIV16_DIV_CLK_DSI_SYS_SHIFT, 6, base + PERI_CRG_PERISTAT1,
		PERI_CRG_PERISTAT1_DIV_DONE_CLK_DSI_SYS_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_DSI_SYS] = devm_xr_clk_hw_gt(dev, "clk_dsi_sys", "clk_dsi_sys_div", NULL,
		base, PERI_CRG_CLKGT0_W1S, PERI_CRG_CLKGT0_W1S_GT_CLK_DSI_SYS_SHIFT, 0,
		0);

	g_hws[XR_CLK_PPLL0_MEDIA1] = devm_xr_clk_hw_gt(dev, "clk_ppll0_media1", "clk_ppll0",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL0_MEDIA1_SHIFT, 0, 0);

	g_hws[XR_CLK_PPLL1_MEDIA1] = devm_xr_clk_hw_gt(dev, "clk_ppll1_media1", "clk_ppll1",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL1_MEDIA1_SHIFT, 0, 0);

	g_hws[XR_CLK_PPLL2_MEDIA1] = devm_xr_clk_hw_gt(dev, "clk_ppll2_media1", "clk_ppll2",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL2_MEDIA1_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_PPLL2_DPU_MEDIA1] = devm_xr_clk_hw_fixed_factor(dev, "clk_ppll2_dpu_media1",
		"clk_ppll2_dpu", 1, 1);

	g_hws[XR_CLK_PPLL3_MEDIA1] = devm_xr_clk_hw_gt(dev, "clk_ppll3_media1", "clk_ppll3",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL3_MEDIA1_SHIFT, 0, 0);

	g_hws[XR_CLK_PPLL0_MEDIA2] = devm_xr_clk_hw_gt(dev, "clk_ppll0_media2", "clk_ppll0",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL0_MEDIA2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_PPLL1_MEDIA2] = devm_xr_clk_hw_gt(dev, "clk_ppll1_media2", "clk_ppll1",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL1_MEDIA2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_PPLL2_MEDIA2] = devm_xr_clk_hw_gt(dev, "clk_ppll2_media2", "clk_ppll2",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL2_MEDIA2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_PPLL3_MEDIA2] = devm_xr_clk_hw_gt(dev, "clk_ppll3_media2", "clk_ppll3",
		NULL, base, PERI_CRG_CLKGT2_W1S,
		PERI_CRG_CLKGT2_W1S_GT_CLK_PPLL3_MEDIA2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_PCIE_AUX_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_pcie_aux_sc_gt",
		"clk_lppll_peri", base + PERI_CRG_CLKDIV23,
		PERI_CRG_CLKDIV23_SC_GT_CLK_PCIE_AUX_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_PCIE_AUX_DIV] = devm_xr_clk_hw_divider(dev, "clk_pcie_aux_div",
		"clk_pcie_aux_sc_gt", base + PERI_CRG_CLKDIV23,
		PERI_CRG_CLKDIV23_DIV_CLK_PCIE_AUX_SHIFT, 6, base + PERI_CRG_PERISTAT2,
		PERI_CRG_PERISTAT2_DIV_DONE_CLK_PCIE_AUX_SHIFT, CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_PCIE_AUX] = devm_xr_clk_hw_gt(dev, "clk_pcie_aux", "clk_pcie_aux_div",
		NULL, base, PERI_CRG_CLKGT0_W1S,
		PERI_CRG_CLKGT0_W1S_GT_CLK_PCIE_AUX_SHIFT, 0, 0);

	/*****MEDIA1_CRG*****/
	base = get_xr_clk_base(XR_CLK_MEDIA1CRG);
	if (!base)
		goto media2_init;

	power_stat = get_media_power_status(XR_CLK_MEDIA1CRG);
	if ((power_stat != MEDIA_POWER_OFF) && (power_stat != MEDIA_POWER_ON)) {
		clkerr("get media1 power state failed!\n");
		goto media2_init;
	} else if (power_stat == MEDIA_POWER_OFF)  {
		ret = crg_vote_media_power(XR_CLK_MEDIA1CRG, CLK_VOTE_MEDIA_ON);
		if (ret) {
			clkerr("clk vote media1 subsys power on failed!\n");
			goto media2_init;
		}
	}

	g_hws[XR_CLK_DPUCORE0_DVFS] = devm_xr_clk_hw_gt(dev, "clk_dpucore0_dvfs",
		"clk_dpu_core_ini", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE0_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_CORE0] = devm_xr_clk_hw_dvfs(dev, "clk_dpu_core0",
		&g_hws[XR_CLK_DPUCORE0_DVFS], CLK_DPUCORE0_VOTE_ID, MEDIA_DVFS_VOTE,
		&dpu_dvfs_cfg);

	g_hws[XR_CLK_DPU_CORE1] = devm_xr_clk_hw_gt(dev, "clk_dpu_core1",
		"clk_dpu_core_ini", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE1_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DPU_CORE2] = devm_xr_clk_hw_gt(dev, "clk_dpu_core2",
		"clk_dpu_core_ini", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DPU_DSC0_DIV] = devm_xr_clk_hw_divider(dev, "clk_dpu_dsc0_div",
		"clk_dpu_core2", base + MEDIA1_CRG_CLKDIV5,
		MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC0_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC0_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_DPU_DSC0_MUX] = devm_xr_clk_hw_mux(dev, "clk_dpu_dsc0_mux",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC0_SHIFT, 1,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_dpu_dsc0_mux_sels,
		ARRAY_SIZE(clk_dpu_dsc0_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_DPU_DSC0] = devm_xr_clk_hw_gt(dev, "clk_dpu_dsc0", "clk_dpu_dsc0_mux", NULL,
		base, MEDIA1_CRG_CLKGT1_W1S, MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSC0_SHIFT,
		0, 0);

	g_hws[XR_CLK_DPU_DSC1_DIV] = devm_xr_clk_hw_divider(dev, "clk_dpu_dsc1_div",
		"clk_dpu_core2", base + MEDIA1_CRG_CLKDIV5,
		MEDIA1_CRG_CLKDIV5_DIV_CLK_DPU_DSC1_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_DSC1_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_DPU_DSC1_MUX] = devm_xr_clk_hw_mux(dev, "clk_dpu_dsc1_mux",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_DSC1_SHIFT, 1,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_dpu_dsc1_mux_sels,
		ARRAY_SIZE(clk_dpu_dsc1_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_DPU_DSC1] = devm_xr_clk_hw_gt(dev, "clk_dpu_dsc1", "clk_dpu_dsc1_mux", NULL,
		base, MEDIA1_CRG_CLKGT1_W1S, MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_DSC1_SHIFT,
		0, 0);

	g_hws[XR_CLK_DPU_CORE3] = devm_xr_clk_hw_gt(dev, "clk_dpu_core3",
		"clk_dpu_core_ini", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_CORE3_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DPU_FAKE] = devm_xr_clk_hw_dvfs_gt(dev, "clk_dpu_fake",
			"clkin_sys", NULL, base, 0, 0, 0, 0, XR_DPU_DVS_CFG);

	g_hws[XR_CLK_DPUVEU_DVFS] = devm_xr_clk_hw_gt(dev, "clk_dpuveu_dvfs",
		"clk_dpu_veu_sub", &g_hws[XR_CLK_DPU_FAKE], base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DPU_VEU] = devm_xr_clk_hw_dvfs(dev, "clk_dpu_veu", &g_hws[XR_CLK_DPUVEU_DVFS],
		CLK_DPUVEU_VOTE_ID, MEDIA_DVFS_VOTE, &dpu_veu_dvfs_cfg);

	g_hws[XR_AUTOCLK_DPU_AXI0] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi0", "aclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI0_SHIFT, 0, 0);

	g_hws[XR_AUTOCLK_DPU_AXI1] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi1", "aclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI1_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_AUTOCLK_DPU_AXI2] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi2", "aclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI2_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_AUTOCLK_DPU_AXI3] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi3", "aclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI3_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DPU_VEU_AXI] = devm_xr_clk_hw_gt(dev, "clk_dpu_veu_axi", "aclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_AXI_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_DISP_BUS_DATA] = devm_xr_clk_hw_gt(dev, "clk_disp_bus_data",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DISP_BUS_DATA_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_AXI0_PERF] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi0_perf",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI0_PERF_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_AXI1_PERF] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi1_perf",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI1_PERF_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_AXI2_PERF] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi2_perf",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI2_PERF_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_AXI3_PERF] = devm_xr_clk_hw_gt(dev, "clk_dpu_axi3_perf",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_AXI3_PERF_SHIFT, 0, 0);

	g_hws[XR_CLK_DPU_VEU_AXI_PERF] = devm_xr_clk_hw_gt(dev, "clk_dpu_veu_axi_perf",
		"aclk_dpu_sub", NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DPU_VEU_AXI_PERF_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPU_CFG] = devm_xr_clk_hw_gt(dev, "pclk_dpu_cfg", "pclk_dpu_sub", NULL,
		base, MEDIA1_CRG_CLKGT1_W1S, MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_CFG_SHIFT,
		0, 0);

	g_hws[XR_PCLK_IP_VEU] = devm_xr_clk_hw_gt(dev, "pclk_veu_cfg", "pclk_dpu_sub", NULL,
		base, MEDIA1_CRG_CLKGT1_W1S, MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_VEU_CFG_SHIFT,
		ALWAYS_ON, 0);

	g_hws[XR_CLK_DISP_BUS_CFG] = devm_xr_clk_hw_gt(dev, "clk_disp_bus_cfg", "pclk_dpu_sub",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_CLK_DISP_BUS_CFG_SHIFT, 0, 0);

	g_hws[XR_PCLK_DSI_CFG] = devm_xr_clk_hw_gt(dev, "pclk_dsi_cfg", "pclk_dpu_sub", NULL,
		base, MEDIA1_CRG_CLKGT0_W1S, MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_DSI_CFG_SHIFT,
		0, 0);

	g_hws[XR_PCLK_DPU_PERF0] = devm_xr_clk_hw_gt(dev, "pclk_dpu_perf0", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF0_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPU_PERF1] = devm_xr_clk_hw_gt(dev, "pclk_dpu_perf1", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF1_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPU_PERF2] = devm_xr_clk_hw_gt(dev, "pclk_dpu_perf2", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF2_SHIFT, 0, 0);

	g_hws[XR_PCLK_DPU_PERF3] = devm_xr_clk_hw_gt(dev, "pclk_dpu_perf3", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_DPU_PERF3_SHIFT, 0, 0);

	g_hws[XR_PCLK_VEU_PERF] = devm_xr_clk_hw_gt(dev, "pclk_veu_perf", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT1_W1S,
		MEDIA1_CRG_CLKGT1_W1S_GT_PCLK_VEU_PERF_SHIFT, 0, 0);

#ifndef CONFIG_XRING_CLK_FAST_DFS
	g_hws[XR_CLK_DPU_CORE_SW] = devm_xr_clk_hw_mux(dev, "clk_dpu_core_sw",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_CORE_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_dpu_core_sw_sels,
		ARRAY_SIZE(clk_dpu_core_sw_sels),
		base + MEDIA1_CRG_CLKST0,
		MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_CORE_SHIFT, 4);

	g_hws[XR_CLK_DPU_CORE_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_dpu_core_sc_gt",
		"clk_dpu_core_sw", base + MEDIA1_CRG_CLKDIV1,
		MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_CORE_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_DPU_CORE_DIV] = devm_xr_clk_hw_divider(dev, "clk_dpu_core_div",
		"clk_dpu_core_sc_gt", base + MEDIA1_CRG_CLKDIV1,
		MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_CORE_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_CORE_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_DPU_CORE_INI] = devm_xr_clk_hw_gt(dev, "clk_dpu_core_ini",
		"clk_dpu_core_div", &g_hws[XR_CLK_DPU_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_DPU_VEU_SW] = devm_xr_clk_hw_mux(dev, "clk_dpu_veu_sw",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_CLK_DPU_VEU_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_dpu_veu_sw_sels, ARRAY_SIZE(clk_dpu_veu_sw_sels),
		base + MEDIA1_CRG_CLKST0, MEDIA1_CRG_CLKST0_SW_ACK_CLK_DPU_VEU_SHIFT, 4);

	g_hws[XR_CLK_DPU_VEU_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_dpu_veu_sc_gt",
		"clk_dpu_veu_sw", base + MEDIA1_CRG_CLKDIV1,
		MEDIA1_CRG_CLKDIV1_SC_GT_CLK_DPU_VEU_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_DPU_VEU_DIV] = devm_xr_clk_hw_divider(dev, "clk_dpu_veu_div",
		"clk_dpu_veu_sc_gt", base + MEDIA1_CRG_CLKDIV1,
		MEDIA1_CRG_CLKDIV1_DIV_SW_CLK_DPU_VEU_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_DPU_VEU_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_DPU_VEU_SUB] = devm_xr_clk_hw_fixed_factor(dev, "clk_dpu_veu_sub",
		"clk_dpu_veu_div", 1, 1);

	g_hws[XR_ACLK_MEDIA1_BUS_SW] = devm_xr_clk_hw_mux(dev, "aclk_media1_bus_sw",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_ACLK_MEDIA1_BUS_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, aclk_media1_bus_sw_sels,
		ARRAY_SIZE(aclk_media1_bus_sw_sels), base + MEDIA1_CRG_CLKST0,
		MEDIA1_CRG_CLKST0_SW_ACK_ACLK_MEDIA1_BUS_SHIFT, 4);

	g_hws[XR_ACLK_MEDIA1_BUS_SC_GT] = devm_xr_clk_hw_scgt(dev, "aclk_media1_bus_sc_gt",
		"aclk_media1_bus_sw", base + MEDIA1_CRG_CLKDIV2,
		MEDIA1_CRG_CLKDIV2_SC_GT_ACLK_MEDIA1_BUS_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_ACLK_MEDIA1_BUS_DIV] = devm_xr_clk_hw_divider(dev, "aclk_media1_bus_div",
		"aclk_media1_bus_sc_gt", base + MEDIA1_CRG_CLKDIV2,
		MEDIA1_CRG_CLKDIV2_DIV_SW_ACLK_MEDIA1_BUS_SHIFT, 6,
		base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_ACLK_MEDIA1_BUS_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_ACLK_MEDIA1_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "aclk_media1_bus_ini",
		"aclk_media1_bus_div", 1, 1);

	g_hws[XR_PCLK_MEDIA1_CFG_SW] = devm_xr_clk_hw_mux(dev, "pclk_media1_cfg_sw",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_PCLK_MEDIA1_CFG_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, pclk_media1_cfg_sw_sels,
		ARRAY_SIZE(pclk_media1_cfg_sw_sels), base + MEDIA1_CRG_CLKST0,
		MEDIA1_CRG_CLKST0_SW_ACK_PCLK_MEDIA1_CFG_SHIFT, 4);

	g_hws[XR_PCLK_MEDIA1_CFG_SC_GT] = devm_xr_clk_hw_scgt(dev, "pclk_media1_cfg_sc_gt",
		"pclk_media1_cfg_sw", base + PERI_CRG_CLKDIV3,
		PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA1_CFG_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_PCLK_MEDIA1_CFG_DIV] = devm_xr_clk_hw_divider(dev, "pclk_media1_cfg_div",
		"pclk_media1_cfg_sc_gt", base + MEDIA1_CRG_CLKDIV3,
		MEDIA1_CRG_CLKDIV3_DIV_SW_PCLK_MEDIA1_CFG_SHIFT, 6,
		base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_PCLK_MEDIA1_CFG_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_PCLK_MEDIA1_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev, "pclk_media1_cfg_ini",
		"pclk_media1_cfg_div", 1, 1);
#else
	g_hws[XR_CLK_DPU_CORE_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "clk_dpu_core_dw",
		0, &dpu_core_fast_dfs);

	g_hws[XR_CLK_DPU_CORE_INI] = devm_xr_clk_hw_gt(dev, "clk_dpu_core_ini",
		"clk_dpu_core_dw", &g_hws[XR_CLK_DPU_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_DPU_VEU_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "clk_dpu_veu_dw",
		0, &dpu_veu_fast_dfs);

	g_hws[XR_CLK_DPU_VEU_SUB] = devm_xr_clk_hw_fixed_factor(dev, "clk_dpu_veu_sub",
		"clk_dpu_veu_dw", 1, 1);

	g_hws[XR_ACLK_MEDIA1_BUS_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "aclk_media1_bus_dw",
		0, &aclk_media1_bus_fast_dfs);

	g_hws[XR_ACLK_MEDIA1_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "aclk_media1_bus_ini",
		"aclk_media1_bus_dw", 1, 1);

	g_hws[XR_PCLK_MEDIA1_CFG_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "pclk_media1_cfg_dw",
		0, &pclk_media1_cfg_fast_dfs);

	g_hws[XR_PCLK_MEDIA1_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev, "pclk_media1_cfg_ini",
		"pclk_media1_cfg_dw", 1, 1);
#endif
	g_hws[XR_CLK_VDEC_CORE_SW] = devm_xr_clk_hw_mux(dev, "clk_vdec_core_sw",
		base + MEDIA1_CRG_CLKSW0, MEDIA1_CRG_CLKSW0_SEL_CLK_VDEC_CORE_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_vdec_core_sw_sels,
		ARRAY_SIZE(clk_vdec_core_sw_sels), base + MEDIA1_CRG_CLKST0,
		MEDIA1_CRG_CLKST0_SW_ACK_CLK_VDEC_CORE_SHIFT, 4);

	g_hws[XR_CLK_VDEC_CORE_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_vdec_core_sc_gt",
		"clk_vdec_core_sw", base + MEDIA1_CRG_CLKDIV2,
		MEDIA1_CRG_CLKDIV2_SC_GT_CLK_VDEC_CORE_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_VDEC_CORE_DIV] = devm_xr_clk_hw_divider(dev, "clk_vdec_core_div",
		"clk_vdec_core_sc_gt", base + MEDIA1_CRG_CLKDIV2,
		MEDIA1_CRG_CLKDIV2_DIV_CLK_VDEC_CORE_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_VDEC_CORE_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_VDEC_CORE_INI] = devm_xr_clk_hw_dvfs_gt(dev, "clk_vdec_core_ini",
		"clk_vdec_core_div", NULL, base, 0, 0, 0, 0, XR_VDEC_DVS_CFG);

	g_hws[XR_CLK_VDEC_DVFS] = devm_xr_clk_hw_gt(dev, "clk_vdec_dvfs", "clk_vdec_core_ini",
		NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_CLK_VDEC_CORE_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_CLK_VDEC_CORE] = devm_xr_clk_hw_dvfs(dev, "clk_vdec_core",
		&g_hws[XR_CLK_VDEC_DVFS], CLK_VDEC_VOTE_ID, MEDIA_DVFS_VOTE,
		&vdec_dvfs_cfg);

	g_hws[XR_ACLK_MEDIA1_BUS] = devm_xr_clk_hw_gt(dev, "aclk_media1_bus",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_BUS_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_ACLK_MEDIA1_TCU] = devm_xr_clk_hw_gt(dev, "aclk_media1_tcu",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_TCU_SHIFT, 0, 0);

	g_hws[XR_ACLK_MEDIA1_BIST_SW] = devm_xr_clk_hw_gt(dev, "aclk_media1_bist_sw",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_BIST_SW_SHIFT, 0, 0);

	g_hws[XR_ACLK_MEDIA1_PERF] = devm_xr_clk_hw_gt(dev, "aclk_media1_perf",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_ACLK_MEDIA1_PERF_SHIFT, 0, 0);

	g_hws[XR_ACLK_VDEC_AXI] = devm_xr_clk_hw_gt(dev, "aclk_vdec_axi", "aclk_media1_bus_ini",
		NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_AXI_SHIFT, 0, 0);

	g_hws[XR_ACLK_VDEC_CORE_AXI] = devm_xr_clk_hw_gt(dev, "aclk_vdec_core_axi",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_CORE_AXI_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_ACLK_VDEC_PERF] = devm_xr_clk_hw_gt(dev, "aclk_vdec_perf",
		"aclk_media1_bus_ini", NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_ACLK_VDEC_PERF_SHIFT, 0, 0);

	g_hws[XR_ACLK_DPU_SUB] = devm_xr_clk_hw_fixed_factor(dev, "aclk_dpu_sub",
		"aclk_media1_bus_ini", 1, 1);

	g_hws[XR_PCLK_MEDIA1_TCU] = devm_xr_clk_hw_gt(dev, "pclk_media1_tcu",
		"pclk_media1_cfg_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_TCU_SHIFT, 0, 0);

	g_hws[XR_PCLK_MEDIA1_BIST] = devm_xr_clk_hw_gt(dev, "pclk_media1_bist",
		"pclk_media1_cfg_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_BIST_SHIFT, 0, 0);

	g_hws[XR_PCLK_VDEC_CFG] = devm_xr_clk_hw_gt(dev, "pclk_vdec_cfg", "pclk_media1_cfg_ini",
		NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_CFG_SHIFT, 0, 0);

	g_hws[XR_PCLK_VDEC_CORE_CFG] = devm_xr_clk_hw_gt(dev, "pclk_vdec_core_cfg",
		"pclk_media1_cfg_ini", NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_CORE_CFG_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_PCLK_DPU_SUB] = devm_xr_clk_hw_fixed_factor(dev, "pclk_dpu_sub",
		"pclk_media1_cfg_ini", 1, 1);

	g_hws[XR_CLK_MEDIA1_BUS_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_media1_bus_ref",
		"clkin_ref_media1", 1, 1);

	g_hws[XR_CLK_DISP_BUS_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_disp_bus_ref",
		"clkin_ref_media1", 1, 1);

	g_hws[XR_CLK_VDEC_BUS_REF] = devm_xr_clk_hw_fixed_factor(dev, "clk_vdec_bus_ref",
		"clkin_ref_media1", 1, 1);

	g_hws[XR_CLK_MEDIA1_HPM_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_media1_hpm_sc_gt",
		"clk_sys_media1", base + MEDIA1_CRG_CLKDIV4,
		MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_HPM_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_MEDIA1_HPM_DIV] = devm_xr_clk_hw_divider(dev, "clk_media1_hpm_div",
		"clk_media1_hpm_sc_gt", base + MEDIA1_CRG_CLKDIV4,
		MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_HPM_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_HPM_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_MEDIA1_HPM_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_media1_hpm_ini",
		"clk_media1_hpm_div", 1, 1);

	g_hws[XR_CLK_MEDIA1_HPM] = devm_xr_clk_hw_gt(dev, "clk_media1_hpm",
		"clk_media1_hpm_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_HPM_SHIFT, 0, 0);

	g_hws[XR_CLK_MEDIA1_GPC_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_media1_gpc_sc_gt",
		"clk_sys_media1", base + MEDIA1_CRG_CLKDIV4,
		MEDIA1_CRG_CLKDIV4_SC_GT_CLK_MEDIA1_GPC_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_MEDIA1_GPC_DIV] = devm_xr_clk_hw_divider(dev, "clk_media1_gpc_div",
		"clk_media1_gpc_sc_gt", base + MEDIA1_CRG_CLKDIV4,
		MEDIA1_CRG_CLKDIV4_DIV_CLK_MEDIA1_GPC_SHIFT, 6, base + MEDIA1_CRG_CLKST1,
		MEDIA1_CRG_CLKST1_DIV_DONE_CLK_MEDIA1_GPC_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_MEDIA1_GPC_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_media1_gpc_ini",
		"clk_media1_gpc_div", 1, 1);

	g_hws[XR_CLK_MEDIA1_GPC0] = devm_xr_clk_hw_gt(dev, "clk_media1_gpc0",
		"clk_media1_gpc_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC0_SHIFT, 0, 0);

	g_hws[XR_CLK_MEDIA1_GPC1] = devm_xr_clk_hw_gt(dev, "clk_media1_gpc1",
		"clk_media1_gpc_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC1_SHIFT, 0, 0);

	g_hws[XR_CLK_MEDIA1_GPC2] = devm_xr_clk_hw_gt(dev, "clk_media1_gpc2",
		"clk_media1_gpc_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC2_SHIFT, 0, 0);

	g_hws[XR_CLK_MEDIA1_GPC3] = devm_xr_clk_hw_gt(dev, "clk_media1_gpc3",
		"clk_media1_gpc_ini", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_CLK_MEDIA1_GPC3_SHIFT, 0, 0);

	g_hws[XR_PCLK_MEDIA1_PERF] = devm_xr_clk_hw_gt(dev, "pclk_media1_perf",
		"pclk_perfmon_peri", NULL, base, MEDIA1_CRG_CLKGT0_W1S,
		MEDIA1_CRG_CLKGT0_W1S_GT_PCLK_MEDIA1_PERF_SHIFT, 0, 0);

	g_hws[XR_PCLK_VDEC_PERF] = devm_xr_clk_hw_gt(dev, "pclk_vdec_perf", "pclk_perfmon_peri",
		NULL, base, MEDIA1_CRG_CLKGT2_W1S,
		MEDIA1_CRG_CLKGT2_W1S_GT_PCLK_VDEC_PERF_SHIFT, 0, 0);

	if (power_stat == MEDIA_POWER_OFF) {
		ret = crg_vote_media_power(XR_CLK_MEDIA1CRG, CLK_VOTE_MEDIA_OFF);
		if (ret)
			clkerr("clk vote media1 subsys power off failed!\n");
	}

media2_init:
	/*****MEDIA2_CRG*****/
	base = get_xr_clk_base(XR_CLK_MEDIA2CRG);
	if (!base)
		goto init_out;

	power_stat = get_media_power_status(XR_CLK_MEDIA2CRG);
	if ((power_stat != MEDIA_POWER_OFF) && (power_stat != MEDIA_POWER_ON)) {
		clkerr("clk get media2 subsys power state failed!\n");
		goto init_out;
	} else if (power_stat == MEDIA_POWER_OFF)  {
		ret = crg_vote_media_power(XR_CLK_MEDIA2CRG, CLK_VOTE_MEDIA_ON);
		if (ret) {
			clkerr("clk vote media2 subsys power on failed!\n");
			goto init_out;
		}
	}

	g_hws[XR_CLK_VENC_DVFS] = devm_xr_clk_hw_gt(dev, "clk_venc_dvfs",
		"clk_venc_core_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_CLK_VENC_CORE_SHIFT, 0, 0);

	g_hws[XR_CLK_VENC_CORE] = devm_xr_clk_hw_dvfs(dev, "clk_venc_core",
		&g_hws[XR_CLK_VENC_DVFS], CLK_VENC_VOTE_ID, MEDIA_DVFS_VOTE,
		&venc_dvfs_cfg);

	g_hws[XR_ACLK_VENC_AXI] = devm_xr_clk_hw_gt(dev, "aclk_venc_axi", "aclk_venc_sub", NULL,
		base, MEDIA2_CRG_CLKGT0_W1S, MEDIA2_CRG_CLKGT0_W1S_GT_ACLK_VENC_AXI_SHIFT,
		0, 0);

	g_hws[XR_ACLK_VENC_PERF] = devm_xr_clk_hw_gt(dev, "aclk_venc_perf", "aclk_venc_sub",
		NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_ACLK_VENC_PERF_SHIFT, 0, 0);

	g_hws[XR_PCLK_VENC_CFG] = devm_xr_clk_hw_gt(dev, "pclk_venc_cfg", "pclk_venc_sub", NULL,
		base, MEDIA2_CRG_CLKGT0_W1S, MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_VENC_CFG_SHIFT,
		0, 0);

	g_hws[XR_PCLK_VENC_PERF] = devm_xr_clk_hw_gt(dev, "pclk_venc_perf", "pclk_perfmon_peri",
		NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_VENC_PERF_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_MCU] = devm_xr_clk_hw_gt(dev, "aclk_isp_mcu", "aclk_isp_sub", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_ACLK_ISP_MCU_SHIFT,
		0, 0);

	g_hws[XR_ACLK_ISP_BUS] = devm_xr_clk_hw_gt(dev, "aclk_isp_bus", "aclk_isp_sub", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_ACLK_ISP_BUS_SHIFT,
		0, 0);

	g_hws[XR_ACLK_ISP_SUBSYS_DEBUG] = devm_xr_clk_hw_gt(dev, "aclk_isp_subsys_debug",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_SUBSYS_DEBUG_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_CFG] = devm_xr_clk_hw_gt(dev, "pclk_isp_cfg", "pclk_isp_sub", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_PCLK_ISP_CFG_SHIFT,
		0, 0);

	g_hws[XR_PCLK_ISP2CSI_CFG] = devm_xr_clk_hw_gt(dev, "pclk_isp2csi_cfg", "pclk_isp_sub",
		NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_PCLK_ISP2CSI_CFG_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_BE] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_be",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_BE_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_PE] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_pe",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_PE_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_CVE] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_cve",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_CVE_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_RTBU] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_rtbu",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_RTBU_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_NTBU0] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_ntbu0",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_NTBU0_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_NTBU1] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_ntbu1",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_NTBU1_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_CMD] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_cmd",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_CMD_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_PERF_MCU] = devm_xr_clk_hw_gt(dev, "pclk_isp_perf_mcu",
		"pclk_perfmon_peri", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_PERF_MCU_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_BE] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_be", "aclk_isp_sub",
		NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_BE_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_PE] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_pe", "aclk_isp_sub",
		NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_PE_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_CVE] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_cve",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_CVE_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_RTBU] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_rtbu",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_RTBU_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_NTBU0] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_ntbu0",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_NTBU0_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_NTBU1] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_ntbu1",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_NTBU1_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_CMD] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_cmd",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_CMD_SHIFT, 0, 0);

	g_hws[XR_ACLK_ISP_PERF_MCU] = devm_xr_clk_hw_gt(dev, "aclk_isp_perf_mcu",
		"aclk_isp_sub", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_ACLK_ISP_PERF_MCU_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_MCU_SUB] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_mcu_sub",
		"clk_isp_mcu_func_ini", 1, 1);

	g_hws[XR_CLK_ISP_MCU_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_mcu_ini",
		"clk_isp_mcu_sub", 1, 1);

	g_hws[XR_CLK_ISP_MCU] = devm_xr_clk_hw_gt(dev, "clk_isp_mcu", "clk_isp_mcu_ini", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_MCU_SHIFT,
		0, 0);

	g_hws[XR_CLK_ISP_MCU_DBG_DIV] = devm_xr_clk_hw_divider_with_flags(dev,
		"clk_isp_mcu_dbg_div", "clk_isp_mcu_sub",
		CLK_OPS_PARENT_ENABLE, base + MEDIA2_CRG_CLKDIV9,
		MEDIA2_CRG_CLKDIV9_DIV_CLK_ISP_MCU_DBG_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_MCU_DBG_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_MCU_DBG_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_mcu_dbg_ini",
		"clk_isp_mcu_dbg_div", 1, 1);

	g_hws[XR_PCLK_MM2_ISP_MCU_DEBUG] = devm_xr_clk_hw_gt(dev, "pclk_mm2_isp_mcu_debug",
		"clk_isp_mcu_dbg_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_PCLK_MM2_ISP_MCU_DEBUG_SHIFT, 0, 0);

	g_hws[XR_CLK_MM2_ISP_MCU_DEBUG_ATB] = devm_xr_clk_hw_gt(dev,
		"clk_mm2_isp_mcu_debug_atb", "clk_isp_mcu_dbg_ini", NULL, base,
		MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_MM2_ISP_MCU_DEBUG_ATB_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_R82_PERI] = devm_xr_clk_hw_gt(dev, "clk_isp_r82_peri",
		"clk_isp_mcu_dbg_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_R82_PERI_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_GENERIC_TIMER_DIV] = devm_xr_clk_hw_divider_with_flags(dev,
		"clk_isp_generic_timer_div", "clk_isp_mcu_dbg_ini",
		CLK_OPS_PARENT_ENABLE, base + MEDIA2_CRG_CLKDIV9,
		MEDIA2_CRG_CLKDIV9_DIV_CLK_ISP_GENERIC_TIMER_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_GENERIC_TIMER_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_GENERIC_TIMER_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_isp_generic_timer_ini", "clk_isp_generic_timer_div", 1, 1);

	g_hws[XR_CLK_ISP_GENERIC_TIMER] = devm_xr_clk_hw_gt(dev, "clk_isp_generic_timer",
		"clk_isp_generic_timer_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_GENERIC_TIMER_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_MCU_BUS_DIV] = devm_xr_clk_hw_divider_with_flags(dev,
		"clk_isp_mcu_bus_div", "clk_isp_mcu_sub",
		CLK_OPS_PARENT_ENABLE, base + MEDIA2_CRG_CLKDIV10,
		MEDIA2_CRG_CLKDIV10_DIV_CLK_ISP_MCU_BUS_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_MCU_BUS_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_MCU_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_mcu_bus_ini",
		"clk_isp_mcu_bus_div", 1, 1);

	g_hws[XR_CLK_ISP_MCU_BUS] = devm_xr_clk_hw_gt(dev, "clk_isp_mcu_bus",
		"clk_isp_mcu_bus_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_MCU_BUS_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_MCU_APB_DIV] = devm_xr_clk_hw_divider_with_flags(dev,
		"clk_isp_mcu_apb_div", "clk_isp_mcu_sub",
		CLK_OPS_PARENT_ENABLE, base + MEDIA2_CRG_CLKDIV10,
		MEDIA2_CRG_CLKDIV10_DIV_CLK_ISP_MCU_APB_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_MCU_APB_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_MCU_APB_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_mcu_apb_ini",
		"clk_isp_mcu_apb_div", 1, 1);

	g_hws[XR_CLK_ISP_MCU_APB] = devm_xr_clk_hw_gt(dev, "clk_isp_mcu_apb",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_MCU_APB_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_MCU_DMA] = devm_xr_clk_hw_gt(dev, "clk_isp_mcu_dma",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_CLK_ISP_MCU_DMA_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_UART] = devm_xr_clk_hw_gt(dev, "pclk_isp_uart",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_UART_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_I2C] = devm_xr_clk_hw_gt(dev, "pclk_isp_i2c",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_I2C_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_I3C] = devm_xr_clk_hw_gt(dev, "pclk_isp_i3c",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_I3C_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_TIMER] = devm_xr_clk_hw_gt(dev, "pclk_isp_timer",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_TIMER_SHIFT, 0, 0);

	g_hws[XR_PCLK_ISP_WTD] = devm_xr_clk_hw_gt(dev, "pclk_isp_wtd",
		"clk_isp_mcu_apb_ini", NULL, base, MEDIA2_CRG_CLKGT2_W1S,
		MEDIA2_CRG_CLKGT2_W1S_GT_PCLK_ISP_WTD_SHIFT, 0, 0);

	g_hws[XR_CLK_ISPFUNC_FAKE] = devm_xr_clk_hw_dvfs_gt(dev, "clk_ispfunc_fake",
			"clkin_sys", NULL, base, 0, 0, 0, 0, XR_ISP_DVS_CFG);

#ifndef CONFIG_XRING_CLK_FAST_DFS
	g_hws[XR_CLK_ISP_FUNC1_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_func1_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC1_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_func1_sw_sels,
		ARRAY_SIZE(clk_isp_func1_sw_sels), base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC1_SHIFT, 4);

	g_hws[XR_CLK_ISP_FUNC1_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_func1_sc_gt",
		"clk_isp_func1_sw", base + MEDIA2_CRG_CLKDIV4,
		MEDIA2_CRG_CLKDIV4_SC_GT_CLK_ISP_FUNC1_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_FUNC1_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_func1_div",
		"clk_isp_func1_sc_gt", base + MEDIA2_CRG_CLKDIV4,
		MEDIA2_CRG_CLKDIV4_DIV_CLK_ISP_FUNC1_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC1_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISPFUNC1_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc1_dvfs",
		"clk_isp_func1_div", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_ISP_FUNC2_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_func2_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC2_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_func2_sw_sels,
		ARRAY_SIZE(clk_isp_func2_sw_sels),
		base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC2_SHIFT, 4);

	g_hws[XR_CLK_ISP_FUNC2_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_func2_sc_gt",
		"clk_isp_func2_sw", base + MEDIA2_CRG_CLKDIV5,
		MEDIA2_CRG_CLKDIV5_SC_GT_CLK_ISP_FUNC2_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_FUNC2_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_func2_div",
		"clk_isp_func2_sc_gt", base + MEDIA2_CRG_CLKDIV5,
		MEDIA2_CRG_CLKDIV5_DIV_CLK_ISP_FUNC2_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC2_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISPFUNC2_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc2_dvfs",
		"clk_isp_func2_div", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_ISP_FUNC3_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_func3_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_FUNC3_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_func3_sw_sels,
		ARRAY_SIZE(clk_isp_func3_sw_sels), base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_FUNC3_SHIFT, 4);

	g_hws[XR_CLK_ISP_FUNC3_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_func3_sc_gt",
		"clk_isp_func3_sw", base + MEDIA2_CRG_CLKDIV5,
		MEDIA2_CRG_CLKDIV5_SC_GT_CLK_ISP_FUNC3_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_FUNC3_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_func3_div",
		"clk_isp_func3_sc_gt", base + MEDIA2_CRG_CLKDIV5,
		MEDIA2_CRG_CLKDIV5_DIV_CLK_ISP_FUNC3_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_FUNC3_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISPFUNC3_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc3_dvfs",
		"clk_isp_func3_div", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_SW] = devm_xr_clk_hw_mux(dev, "clk_media2_noc_data_sw",
		base + MEDIA2_CRG_CLKSW2, MEDIA2_CRG_CLKSW2_SEL_CLK_MEDIA2_NOC_DATA_SHIFT,
		2, CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_media2_noc_data_sw_sels,
		ARRAY_SIZE(clk_media2_noc_data_sw_sels), base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_SW_ACK_CLK_MEDIA2_NOC_DATA_SHIFT, 4);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_SC_GT] = devm_xr_clk_hw_scgt(dev,
		"clk_media2_noc_data_sc_gt", "clk_media2_noc_data_sw",
		base + MEDIA2_CRG_CLKDIV8,
		MEDIA2_CRG_CLKDIV8_SC_GT_CLK_MEDIA2_NOC_DATA_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_DIV] = devm_xr_clk_hw_divider(dev,
		"clk_media2_noc_data_div", "clk_media2_noc_data_sc_gt",
		base + MEDIA2_CRG_CLKDIV8,
		MEDIA2_CRG_CLKDIV8_DIV_CLK_MEDIA2_NOC_DATA_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_MEDIA2_NOC_DATA_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_media2_noc_data_ini", "clk_media2_noc_data_div", 1, 1);

	g_hws[XR_CLK_VENC_CORE_SW] = devm_xr_clk_hw_mux(dev, "clk_venc_core_sw",
		base + MEDIA2_CRG_CLKSW0, MEDIA2_CRG_CLKSW0_SEL_CLK_VENC_CORE_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_venc_core_sw_sels,
		ARRAY_SIZE(clk_venc_core_sw_sels), base + MEDIA2_CRG_CLKST0,
		MEDIA2_CRG_CLKST0_SW_ACK_CLK_VENC_CORE_SHIFT, 4);

	g_hws[XR_CLK_VENC_CORE_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_venc_core_sc_gt",
		"clk_venc_core_sw", base + MEDIA2_CRG_CLKDIV1,
		MEDIA2_CRG_CLKDIV1_SC_GT_CLK_VENC_CORE_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_VENC_CORE_DIV] = devm_xr_clk_hw_divider(dev, "clk_venc_core_div",
		"clk_venc_core_sc_gt", base + MEDIA2_CRG_CLKDIV1,
		MEDIA2_CRG_CLKDIV1_DIV_CLK_VENC_CORE_SHIFT, 6, base + MEDIA2_CRG_CLKST1,
		MEDIA2_CRG_CLKST1_DIV_DONE_CLK_VENC_CORE_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_VENC_CORE_INI] = devm_xr_clk_hw_dvfs_gt(dev, "clk_venc_core_ini",
		"clk_venc_core_div", NULL, base, 0, 0, 0, 0, XR_VENC_DVS_CFG);

	g_hws[XR_ACLK_MEDIA2_BUS_SW] = devm_xr_clk_hw_mux(dev, "aclk_media2_bus_sw",
		base + MEDIA2_CRG_CLKSW2, MEDIA2_CRG_CLKSW2_SEL_ACLK_MEDIA2_BUS_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, aclk_media2_bus_sw_sels,
		ARRAY_SIZE(aclk_media2_bus_sw_sels), base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_ACLK_MEDIA2_BUS_SHIFT, 4);

	g_hws[XR_ACLK_MEDIA2_BUS_SC_GT] = devm_xr_clk_hw_scgt(dev, "aclk_media2_bus_sc_gt",
		"aclk_media2_bus_sw", base + MEDIA2_CRG_CLKDIV3,
		MEDIA2_CRG_CLKDIV3_SC_GT_ACLK_MEDIA2_BUS_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_ACLK_MEDIA2_BUS_DIV] = devm_xr_clk_hw_divider(dev, "aclk_media2_bus_div",
		"aclk_media2_bus_sc_gt", base + MEDIA2_CRG_CLKDIV3,
		MEDIA2_CRG_CLKDIV3_DIV_ACLK_MEDIA2_BUS_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_ACLK_MEDIA2_BUS_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_ACLK_MEDIA2_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "aclk_media2_bus_ini",
		"aclk_media2_bus_div", 1, 1);

	g_hws[XR_PCLK_MEDIA2_CFG_SW] = devm_xr_clk_hw_mux(dev, "pclk_media2_cfg_sw",
		base + MEDIA2_CRG_CLKSW2, MEDIA2_CRG_CLKSW2_SEL_PCLK_MEDIA2_BUS_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, pclk_media2_cfg_sw_sels,
		ARRAY_SIZE(pclk_media2_cfg_sw_sels), base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_PCLK_MEDIA2_BUS_SHIFT, 4);

	g_hws[XR_PCLK_MEDIA2_CFG_SC_GT] = devm_xr_clk_hw_scgt(dev, "pclk_media2_cfg_sc_gt",
		"pclk_media2_cfg_sw", base + PERI_CRG_CLKDIV3,
		PERI_CRG_CLKDIV3_SC_GT_PCLK_MEDIA2_CFG_SHIFT,
		CLK_GATE_HIWORD_MASK | CLK_SCGATE_ALWAYS_ON_MASK);

	g_hws[XR_PCLK_MEDIA2_CFG_DIV] = devm_xr_clk_hw_divider(dev, "pclk_media2_cfg_div",
		"pclk_media2_cfg_sc_gt", base + MEDIA2_CRG_CLKDIV3,
		MEDIA2_CRG_CLKDIV3_DIV_PCLK_MEDIA2_CFG_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_PCLK_MEDIA2_BUS_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_PCLK_MEDIA2_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev, "pclk_media2_cfg_ini",
		"pclk_media2_cfg_div", 1, 1);

	g_hws[XR_CLK_ISP_MCU_FUNC_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_mcu_func_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_MCU_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_mcu_func_sw_sels,
		ARRAY_SIZE(clk_isp_mcu_func_sw_sels), base + MEDIA2_CRG_CLKST3,
		MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_MCU_SHIFT, 4);

	g_hws[XR_CLK_ISP_MCU_FUNC_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_mcu_func_sc_gt",
		"clk_isp_mcu_func_sw", base + MEDIA2_CRG_CLKDIV4,
		MEDIA2_CRG_CLKDIV4_SC_GT_CLK_ISP_MCU_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_MCU_FUNC_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_mcu_func_div",
		"clk_isp_mcu_func_sc_gt", base + MEDIA2_CRG_CLKDIV4,
		MEDIA2_CRG_CLKDIV4_DIV_CLK_ISP_MCU_FUNC_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_MCU_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_MCU_FUNC_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_isp_mcu_func_ini", "clk_isp_mcu_func_div", 1, 1);

	g_hws[XR_CLK_ISP_I2C_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_i2c_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_I2C_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_i2c_sw_sels,
		ARRAY_SIZE(clk_isp_i2c_sw_sels),
		base + MEDIA2_CRG_CLKST3, MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_I2C_SHIFT, 4);

	g_hws[XR_CLK_ISP_I2C_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_i2c_sc_gt",
		"clk_isp_i2c_sw", base + MEDIA2_CRG_CLKDIV7,
		MEDIA2_CRG_CLKDIV7_SC_GT_CLK_ISP_I2C_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_I2C_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_i2c_div",
		"clk_isp_i2c_sc_gt", base + MEDIA2_CRG_CLKDIV7,
		MEDIA2_CRG_CLKDIV7_DIV_CLK_ISP_I2C_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_I2C_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_I2C_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_i2c_ini",
		"clk_isp_i2c_div", 1, 1);

	g_hws[XR_CLK_ISP_I3C_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_i3c_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_I3C_SHIFT, 2,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_i3c_sw_sels,
		ARRAY_SIZE(clk_isp_i3c_sw_sels),
		base + MEDIA2_CRG_CLKST3, MEDIA2_CRG_CLKST3_SW_ACK_CLK_ISP_I3C_SHIFT, 4);

	g_hws[XR_CLK_ISP_I3C_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_isp_i3c_sc_gt",
		"clk_isp_i3c_sw", base + MEDIA2_CRG_CLKDIV7,
		MEDIA2_CRG_CLKDIV7_SC_GT_CLK_ISP_I3C_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_ISP_I3C_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_i3c_div",
		"clk_isp_i3c_sc_gt", base + MEDIA2_CRG_CLKDIV7,
		MEDIA2_CRG_CLKDIV7_DIV_CLK_ISP_I3C_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_I3C_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_I3C_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_i3c_ini",
		"clk_isp_i3c_div", 1, 1);

#else
	g_hws[XR_CLK_ISP_FUNC1_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "clk_isp_func1_dw",
		0, &ispfunc1_fast_dfs);

	g_hws[XR_CLK_ISPFUNC1_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc1_dvfs",
		"clk_isp_func1_dw", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_ISP_FUNC2_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "clk_isp_func2_dw",
		0, &ispfunc2_fast_dfs);

	g_hws[XR_CLK_ISPFUNC2_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc2_dvfs",
		"clk_isp_func2_dw", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_ISP_FUNC3_DW] = devm_xr_clk_hw_fast_dfs(dev, base, "clk_isp_func3_dw",
		0, &ispfunc3_fast_dfs);

	g_hws[XR_CLK_ISPFUNC3_DVFS] = devm_xr_clk_hw_gt(dev, "clk_ispfunc3_dvfs",
		"clk_isp_func3_dw", &g_hws[XR_CLK_ISPFUNC_FAKE], base, 0, 0, 0, 0);

	g_hws[XR_CLK_ISP_MCU_FUNC_INI] = devm_xr_clk_hw_fast_dfs(dev, base,
		"clk_isp_mcu_func_ini", 0, &isp_mcu_func_fast_dfs);

	g_hws[XR_CLK_ISP_I2C_INI] = devm_xr_clk_hw_fast_dfs(dev, base,
		"clk_isp_i2c_ini", 0, &isp_i2c_fast_dfs);

	g_hws[XR_CLK_ISP_I3C_INI] = devm_xr_clk_hw_fast_dfs(dev, base,
		"clk_isp_i3c_ini", 0, &isp_i3c_fast_dfs);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_DW] = devm_xr_clk_hw_fast_dfs(dev, base,
		"clk_media2_noc_data_dw", 0, &media2_noc_data_fast_dfs);

	g_hws[XR_CLK_MEDIA2_NOC_DATA_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"clk_media2_noc_data_ini", "clk_media2_noc_data_dw", 1, 1);

	g_hws[XR_CLK_VENC_CORE_DW] = devm_xr_clk_hw_fast_dfs(dev, base,
		"clk_venc_core_dw", 0, &venc_core_fast_dfs);

	g_hws[XR_CLK_VENC_CORE_INI] = devm_xr_clk_hw_dvfs_gt(dev, "clk_venc_core_ini",
		"clk_venc_core_dw", NULL, base, 0, 0, 0, 0, XR_VENC_DVS_CFG);

	g_hws[XR_ACLK_MEDIA2_BUS_DW] = devm_xr_clk_hw_fast_dfs(dev, base,
		"aclk_media2_bus_dw", 0, &aclk_media2_bus_fast_dfs);

	g_hws[XR_ACLK_MEDIA2_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"aclk_media2_bus_ini", "aclk_media2_bus_dw", 1, 1);

	g_hws[XR_PCLK_MEDIA2_CFG_DW] = devm_xr_clk_hw_fast_dfs(dev, base,
		"pclk_media2_cfg_dw", 0, &pclk_media2_cfg_fast_dfs);

	g_hws[XR_PCLK_MEDIA2_CFG_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"pclk_media2_cfg_ini", "pclk_media2_cfg_dw", 1, 1);
#endif
	g_hws[XR_CLK_ISP_FUNC1_INI] = devm_xr_clk_hw_dvfs(dev, "clk_ispfunc1_ini",
		&g_hws[XR_CLK_ISPFUNC1_DVFS], CLK_ISPFUNC1_VOTE_ID, MEDIA_DVFS_VOTE,
		&ispfunc1_dvfs_cfg);

	g_hws[XR_CLK_ISP_FUNC2_INI] = devm_xr_clk_hw_dvfs(dev, "clk_ispfunc2_ini",
		&g_hws[XR_CLK_ISPFUNC2_DVFS], CLK_ISPFUNC2_VOTE_ID, MEDIA_DVFS_VOTE,
		&ispfunc2_dvfs_cfg);

	g_hws[XR_CLK_ISP_FUNC3_INI] = devm_xr_clk_hw_dvfs(dev, "clk_ispfunc3_ini",
		&g_hws[XR_CLK_ISPFUNC3_DVFS], CLK_ISPFUNC3_VOTE_ID, MEDIA_DVFS_VOTE,
		&ispfunc3_dvfs_cfg);


	g_hws[XR_PCLK_MM2_SUBSYS_DEBUG_ATB_DIV] = devm_xr_clk_hw_divider(dev,
		"pclk_mm2_subsys_debug_atb_div", "clk_isp_mcu_func_div",
		base + MEDIA2_CRG_CLKDIV2,
		MEDIA2_CRG_CLKDIV2_DIV_PCLK_MM2_SUBSYS_DEBUG_ATB_SHIFT, 6,
		base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_PCLK_MM2_SUBSYS_DEBUG_ATB_SHIFT,
		CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_PCLK_MM2_SUBSYS_DEBUG_ATB_INI] = devm_xr_clk_hw_fixed_factor(dev,
		"pclk_mm2_subsys_debug_atb_ini", "pclk_mm2_subsys_debug_atb_div", 1, 1);

	g_hws[XR_PCLK_MM2_SUBSYS_DEBUG_ATB] = devm_xr_clk_hw_gt(dev,
		"pclk_mm2_subsys_debug_atb", "pclk_mm2_subsys_debug_atb_ini", NULL, base,
		MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MM2_SUBSYS_DEBUG_ATB_SHIFT, 0, 0);

	g_hws[XR_CLK_MEDIA2_NOC_DATA] = devm_xr_clk_hw_gt(dev, "clk_media2_noc_data",
		"clk_media2_noc_data_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_MEDIA2_NOC_DATA_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_ACLK_MEDIA2_BUS] = devm_xr_clk_hw_gt(dev, "aclk_media2_bus",
		"aclk_media2_bus_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_ACLK_MEDIA2_BUS_SHIFT, ALWAYS_ON, 0);

	g_hws[XR_ACLK_MEDIA2_TCU] = devm_xr_clk_hw_gt(dev, "aclk_media2_tcu",
		"aclk_media2_bus_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_ACLK_MEDIA2_TCU_SHIFT, 0, 0);

	g_hws[XR_ACLK_MEDIA2_BIST_HW] = devm_xr_clk_hw_gt(dev, "aclk_media2_bist",
		"aclk_media2_bus_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_ACLK_MEDIA2_BIST_SW_SHIFT, 0, 0);

	g_hws[XR_ACLK_MEDIA2_PERF] = devm_xr_clk_hw_gt(dev, "aclk_media2_perf",
		"aclk_media2_bus_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_ACLK_MEDIA2_PERF_SHIFT, 0, 0);

	g_hws[XR_ACLK_VENC_SUB] = devm_xr_clk_hw_fixed_factor(dev, "aclk_venc_sub",
		"aclk_media2_bus_ini", 1, 1);

	g_hws[XR_ACLK_ISP_SUB] = devm_xr_clk_hw_fixed_factor(dev, "aclk_isp_sub",
		"aclk_media2_bus_ini", 1, 1);

	g_hws[XR_PCLK_MEDIA2_TPC] = devm_xr_clk_hw_gt(dev, "pclk_media2_tpc",
		"pclk_media2_cfg_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MEDIA2_TPC_SHIFT, 0, 0);

	g_hws[XR_PCLK_MM2_SUBSYS_DEBUG_APB] = devm_xr_clk_hw_gt(dev,
		"pclk_mm2_subsys_debug_apb", "pclk_media2_cfg_ini", NULL, base,
		MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MM2_SUBSYS_DEBUG_APB_SHIFT, 0, 0);

	g_hws[XR_PCLK_MEDIA2_BIST] = devm_xr_clk_hw_gt(dev, "pclk_media2_bist",
		"pclk_media2_cfg_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MEDIA2_BIST_SHIFT, 0, 0);

	g_hws[XR_PCLK_MEDIA2_TCU] = devm_xr_clk_hw_gt(dev, "pclk_media2_tcu",
		"pclk_media2_cfg_ini", NULL, base, MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MEDIA2_TCU_SHIFT, 0, 0);

	g_hws[XR_PCLK_VENC_SUB] = devm_xr_clk_hw_fixed_factor(dev, "pclk_venc_sub",
		"pclk_media2_cfg_ini", 1, 1);

	g_hws[XR_PCLK_ISP_SUB] = devm_xr_clk_hw_fixed_factor(dev, "pclk_isp_sub",
		"pclk_media2_cfg_ini", 1, 1);

	g_hws[XR_CLK_ISP_I2C] = devm_xr_clk_hw_gt(dev, "clk_isp_i2c", "clk_isp_i2c_ini", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_I2C_SHIFT,
		0, 0);

	g_hws[XR_CLK_ISP_I3C] = devm_xr_clk_hw_gt(dev, "clk_isp_i3c", "clk_isp_i3c_ini", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_I3C_SHIFT,
		0, 0);

	g_hws[XR_CLK_MEDIA2_HPM_DIV] = devm_xr_clk_hw_divider(dev, "clk_media2_hpm_div",
		"clk_sys_media2", base + MEDIA2_CRG_CLKDIV2,
		MEDIA2_CRG_CLKDIV2_DIV_CLK_MEIDA2_HPM_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_MEDIA2_HPM_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_MEDIA2_HPM_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_media2_hpm_ini",
		"clk_media2_hpm_div", 1, 1);

	g_hws[XR_CLK_MEDIA2_HPM] = devm_xr_clk_hw_gt(dev, "clk_media2_hpm",
		"clk_media2_hpm_ini", NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_MEDIA2_HPM_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_REF_DIV] = devm_xr_clk_hw_divider(dev, "clk_isp_ref_div",
		"clk_sys_media2", base + MEDIA2_CRG_CLKDIV8,
		MEDIA2_CRG_CLKDIV8_DIV_CLK_ISP_REF_SHIFT, 6, base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_DIV_DONE_CLK_ISP_REF_SHIFT, CLK_DIVIDER_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA);

	g_hws[XR_CLK_ISP_REF_INI] = devm_xr_clk_hw_fixed_factor(dev, "clk_isp_ref_ini",
		"clk_isp_ref_div", 1, 1);

	g_hws[XR_CLK_ISP_REF] = devm_xr_clk_hw_gt(dev, "clk_isp_ref", "clk_isp_ref_ini", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_REF_SHIFT,
		0, 0);

	g_hws[XR_CLK_ISP_TIMER_SW] = devm_xr_clk_hw_mux(dev, "clk_isp_timer_sw",
		base + MEDIA2_CRG_CLKSW1, MEDIA2_CRG_CLKSW1_SEL_CLK_ISP_TIMER_SHIFT, 1,
		CLK_MUX_HIWORD_MASK|CLK_DYNAMIC_POFF_AREA, clk_isp_timer_sw_sels,
		ARRAY_SIZE(clk_isp_timer_sw_sels), base + MEDIA2_CRG_CLKST4,
		MEDIA2_CRG_CLKST4_SW_ACK_CLK_ISP_TIMER_SHIFT, 2);

	g_hws[XR_CLK_ISP_TIMER] = devm_xr_clk_hw_gt(dev, "clk_isp_timer", "clk_isp_timer_sw",
		NULL, base, MEDIA2_CRG_CLKGT1_W1S,
		MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_TIMER_SHIFT, 0, 0);

	g_hws[XR_CLK_ISP_WTD] = devm_xr_clk_hw_gt(dev, "clk_isp_wtd", "clkin_ref_media2", NULL,
		base, MEDIA2_CRG_CLKGT1_W1S, MEDIA2_CRG_CLKGT1_W1S_GT_CLK_ISP_WTD_SHIFT,
		0, 0);

	g_hws[XR_PCLK_MEDIA2_PERF] = devm_xr_clk_hw_gt(dev, "pclk_media2_perf", "pclk_perfmon_peri",
		NULL, base, MEDIA2_CRG_CLKGT0_W1S, MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MEDIA2_PERF_SHIFT,
		0, 0);

	g_hws[XR_PCLK_MM2_SUBSYS_R82_DEBUG_APB] = devm_xr_clk_hw_gt(dev,
		"pclk_mm2_subsys_r82_debug_apb", "clk_debug_apb_core", NULL, base,
		MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MM2_SUBSYS_R82_DEBUG_APB_SHIFT, 0, 0);

	g_hws[XR_PCLK_MM2_SUBCHIP_DEBUG_ATB] = devm_xr_clk_hw_gt(dev,
		"pclk_mm2_subchip_debug_atb", "clk_cfgbus_up", NULL, base,
		MEDIA2_CRG_CLKGT0_W1S,
		MEDIA2_CRG_CLKGT0_W1S_GT_PCLK_MM2_SUBCHIP_DEBUG_ATB_SHIFT, 0, 0);

	if (power_stat == MEDIA_POWER_OFF) {
		ret = crg_vote_media_power(XR_CLK_MEDIA2CRG, CLK_VOTE_MEDIA_OFF);
		if (ret)
			clkerr("clk vote media2 subsys power off failed!\n");
	}

init_out:
	g_hws[XR_CLK_32K_O2] = devm_xr_clk_hw_pmu(dev, "clk_32k_o2", "clkin_ref",
		TOP_REG_CRG_CONFIG3_2, TOP_REG_CRG_CONFIG3_2_SW_CRG_32KO2_EN_SHIFT, 0);
	g_hws[XR_CLK_32K_O3] = devm_xr_clk_hw_pmu(dev, "clk_32k_o3", "clkin_ref",
		TOP_REG_CRG_CONFIG3_2, TOP_REG_CRG_CONFIG3_2_SW_CRG_32KO3_EN_SHIFT, 0);
	g_hws[XR_CLK_38M4_O2] = devm_xr_clk_hw_pmu(dev, "clk_38m4_o2", "clkin_sys",
		TOP_REG_PCTRL_CONFIG29_2, TOP_REG_PCTRL_CONFIG29_2_SW_CLK_BUF2_EN_SHIFT, 0);
	g_hws[XR_CLK_38M4_O3] = devm_xr_clk_hw_pmu(dev, "clk_38m4_o3", "clkin_sys",
		TOP_REG_PCTRL_CONFIG29_2, TOP_REG_PCTRL_CONFIG29_2_SW_CLK_BUF3_EN_SHIFT, 0);
	g_hws[XR_CLK_38M4_O4] = devm_xr_clk_hw_pmu(dev, "clk_38m4_o4", "clkin_sys",
		TOP_REG_PCTRL_CONFIG29_2, TOP_REG_PCTRL_CONFIG29_2_SW_CLK_BUF4_EN_SHIFT, 0);
	g_hws[XR_CLK_38M4_O5] = devm_xr_clk_hw_pmu(dev, "clk_38m4_o5", "clkin_sys",
		TOP_REG_PCTRL_CONFIG29_2, TOP_REG_PCTRL_CONFIG29_2_SW_CLK_BUF5_EN_SHIFT, 0);

	base = get_xr_clk_base(XR_CLK_LPISCRG);
	g_hws[XR_CLK_SYS_LPIS] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_lpis", "clkin_sys",
		1, 1);

	g_hws[XR_CLK_SYS_DIV_SC_GT] = devm_xr_clk_hw_safe_gt(dev, "clk_sys_div_sc_gt",
		"clk_sys_lpis", NULL, SAFE_CLK_SYS_DIV_SC_GT, 0);

	g_hws[XR_CLK_SYS_DIV] = devm_xr_clk_hw_fixed_factor(dev, "clk_sys_div", "clk_sys_div_sc_gt",
		1, 2);

	g_hws[XR_CLK_GPLL_LPIS] = devm_xr_clk_hw_fixed_factor(dev, "clk_gpll_lpis", "clk_gpll",
		1, 1);

	g_hws[XR_CLK_LPLL_LPIS] = devm_xr_clk_hw_fixed_factor(dev, "clk_lppll_lpis", "clk_lppll",
		1, 1);

	g_hws[XR_CLK_CAM_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam_mux", base + LPIS_CRG_CLKDIV14,
		LPIS_CRG_CLKDIV14_SEL_CLK_CAM_MUX_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam_mux_sels, ARRAY_SIZE(clk_cam_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_CAM_DIV_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_cam_div_sc_gt", "clk_cam_mux",
		base + LPIS_CRG_CLKDIV13, LPIS_CRG_CLKDIV13_SC_GT_CLK_CAM_DIV_SHIFT,
		CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_CAM_DIV_PRE] = devm_xr_clk_hw_divider(dev, "clk_cam_div_pre",
		"clk_cam_div_sc_gt", base + LPIS_CRG_CLKDIV13,
		LPIS_CRG_CLKDIV13_DIV_CLK_CAM_DIV_PRE_SHIFT, 6,
		base + LPIS_CRG_DIV_STAT0, LPIS_CRG_DIV_STAT0_DIV_DONE_CLK_CAM_DIV_PRE_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_CAM_DIV] = devm_xr_clk_hw_divider(dev, "clk_cam_div", "clk_cam_div_pre",
		base + LPIS_CRG_CLKDIV13, LPIS_CRG_CLKDIV13_DIV_CLK_CAM_SHIFT, 2,
		base + LPIS_CRG_DIV_STAT0, LPIS_CRG_DIV_STAT0_DIV_DONE_CLK_CAM_DIV_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_CAM0_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam0_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM0_SHIFT,
		1, CLK_MUX_HIWORD_MASK, clk_cam0_mux_sels, ARRAY_SIZE(clk_cam0_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM0] = devm_xr_clk_hw_gt(dev, "clk_cam0", "clk_cam0_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM0_SHIFT,
		0, 0);

	g_hws[XR_CLK_CAM1_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam1_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM1_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam1_mux_sels, ARRAY_SIZE(clk_cam1_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM1] = devm_xr_clk_hw_gt(dev, "clk_cam1", "clk_cam1_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM1_SHIFT,
		0, 0);

	g_hws[XR_CLK_CAM2_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam2_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM2_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam2_mux_sels, ARRAY_SIZE(clk_cam2_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM2] = devm_xr_clk_hw_gt(dev, "clk_cam2", "clk_cam2_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM2_SHIFT,
		0, 0);

	g_hws[XR_CLK_CAM3_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam3_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM3_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam3_mux_sels, ARRAY_SIZE(clk_cam3_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM3] = devm_xr_clk_hw_gt(dev, "clk_cam3", "clk_cam3_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM3_SHIFT,
		0, 0);

	g_hws[XR_CLK_CAM4_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam4_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM4_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam4_mux_sels, ARRAY_SIZE(clk_cam4_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM4] = devm_xr_clk_hw_gt(dev, "clk_cam4", "clk_cam4_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM4_SHIFT,
		0, 0);

	g_hws[XR_CLK_CAM5_MUX] = devm_xr_clk_hw_mux(dev, "clk_cam5_mux",
		base + LPIS_CRG_CLKDIV14, LPIS_CRG_CLKDIV14_SEL_CLK_CAM5_SHIFT, 1,
		CLK_MUX_HIWORD_MASK, clk_cam5_mux_sels, ARRAY_SIZE(clk_cam5_mux_sels),
		NULL, 0, 0);

	g_hws[XR_CLK_CAM5] = devm_xr_clk_hw_gt(dev, "clk_cam5", "clk_cam5_mux",
		NULL, base, LPIS_CRG_CLKGT2_W1S, LPIS_CRG_CLKGT2_W1S_GT_CLK_CAM5_SHIFT,
		0, 0);

	g_hws[XR_PCLK_ONEWIRE0] = devm_xr_clk_hw_safe_gt(dev, "pclk_onewire0",
		"clk_pericfg_bus_ini", NULL, SAFE_PCLK_ONEWIRE0, 0);
	g_hws[XR_PCLK_ONEWIRE1] = devm_xr_clk_hw_safe_gt(dev, "pclk_onewire1",
		"clk_pericfg_bus_ini", NULL, SAFE_PCLK_ONEWIRE1, 0);
	g_hws[XR_PCLK_ONEWIRE2] = devm_xr_clk_hw_safe_gt(dev, "pclk_onewire2",
		"clk_pericfg_bus_ini", NULL, SAFE_PCLK_ONEWIRE2, 0);
	g_hws[XR_CLK_ONEWIRE0] = devm_xr_clk_hw_safe_gt(dev, "clk_onewire0",
		"clk_sys_peri", NULL, SAFE_CLK_ONEWIRE0, 0);
	g_hws[XR_CLK_ONEWIRE1] = devm_xr_clk_hw_safe_gt(dev, "clk_onewire1",
		"clk_sys_peri", NULL, SAFE_CLK_ONEWIRE1, 0);
	g_hws[XR_CLK_ONEWIRE2] = devm_xr_clk_hw_safe_gt(dev, "clk_onewire2",
		"clk_sys_peri", NULL, SAFE_CLK_ONEWIRE2, 0);

	g_hws[XR_PCLK_LPIS_BUS_INI] = devm_xr_clk_hw_fixed_factor(dev, "pclk_lpis_bus_ini",
		"clk_gpll", 1, 4);
	g_hws[XR_PCLK_LPIS_SPI_DMAC] = devm_xr_clk_hw_safe_gt(dev, "pclk_lpis_spi_dmac",
		"pclk_lpis_bus_ini", NULL, SAFE_PCLK_SPI_DMA_GT, 0);
	g_hws[XR_CLK_SPI_DMAC_DIV] = devm_xr_clk_hw_sec_div(dev, "clk_spi_dmac_div",
		"clk_lppll_lpis", SAFE_DIV_CLK_SPI_DMA, 6);
	g_hws[XR_CLK_SPI_DMAC] = devm_xr_clk_hw_safe_gt(dev, "clk_spi_dmac",
		"clk_spi_dmac_div", NULL, SAFE_CLK_SPI_DMA_GT, 0);

	/* LMS_CRG */
	base = get_xr_clk_base(XR_CLK_LMSCRG);

	g_hws[XR_CLK_GPLL_LMS] = devm_xr_clk_hw_fixed_factor(dev, "clk_gpll_lms",
		"clk_gpll", 1, 34);

	g_hws[XR_CLK_CODEC_MUX] = devm_xr_clk_hw_mux(dev, "clk_codec_mux",
		base + LMS_CRG_CLKDIV2, LMS_CRG_CLKDIV2_SEL_CLK_CODEC_SHIFT, 2,
		CLK_MUX_HIWORD_MASK, clk_codec_mux_sels,
		ARRAY_SIZE(clk_codec_mux_sels), NULL, 0, 0);

	g_hws[XR_CLK_CODEC_SC_GT] = devm_xr_clk_hw_scgt(dev, "clk_codec_sc_gt", "clk_codec_mux",
		base + LMS_CRG_CLKDIV1, LMS_CRG_CLKDIV1_SC_GT_CLK_CODEC_SHIFT, CLK_GATE_HIWORD_MASK);

	g_hws[XR_CLK_CODEC_DIV] = devm_xr_clk_hw_divider(dev, "clk_codec_div",
		"clk_codec_sc_gt", base + LMS_CRG_CLKDIV1,
		LMS_CRG_CLKDIV1_DIV_CLK_CODEC_SHIFT, 6,
		base + LMS_CRG_PERISTAT0, LMS_CRG_PERISTAT0_DIV_DONE_CLK_CODEC_SHIFT,
		CLK_DIVIDER_HIWORD_MASK);

	g_hws[XR_CLK_CODEC] = devm_xr_clk_hw_gt(dev, "clk_codec", "clk_codec_div",
		NULL, base, LMS_CRG_CLKGT0_W1S, LMS_CRG_CLKGT0_W1S_GT_CLK_CODEC_SHIFT, 0, 0);


/* ======== Generated Register Code End,Do not modify by hand! ======== */
	register_syscore_ops(&o1_clk_syscore_ops);
	clkinfo("end o1 clock probe!\n");
	return 0;
}

static int xring_o1_clocks_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	of_clk_del_provider(np);
	xr_unregister_hw_clocks(g_hws, XR_CLK_END);

	return 0;
}

static const struct of_device_id xring_o1_clk_of_match[] = {
	{ .compatible = "xring,xr-o1-ccm" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, xring_o1_clk_of_match);

static struct platform_driver xring_o1_clk_driver = {
	.probe = xring_o1_clocks_probe,
	.remove = xring_o1_clocks_remove,
	.driver = {
		.name = "xr-o1-ccm",
		/*
		 * Disable bind attributes: clocks are not removed and
		 * reloading the driver will crash or break devices.
		 */
		.suppress_bind_attrs = true,
		.of_match_table = of_match_ptr(xring_o1_clk_of_match),
	},
};

int xring_clocks_init(void)
{
	int ret;

	clk_feature_state_init();

	ret = platform_driver_register(&xring_o1_clk_driver);
	if (ret) {
		pr_err("xring o1 clocks register failed: %d\n", ret);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(xring_clocks_init);

int xring_clocks_exit(void)
{
	platform_driver_unregister(&xring_o1_clk_driver);
	return 0;
}
EXPORT_SYMBOL_GPL(xring_clocks_exit);

static struct clk_pll_cfg_table *ppll2_dpu_cfg_get(void)
{
	return &ppll2_dpu_cfg;
}

int dpu_ppll2_vote_bypass(unsigned int flag)
{
	struct xsp_ffa_msg msg = { 0 };
	void __iomem *lock_base = NULL;
	struct clk_pll_cfg_table *pll_cfg = NULL;
	int ret = 0;
	unsigned long flags;

	if (flag >= DPU_PPLL2_MAX)
		return -EINVAL;

	spin_lock_irqsave(&dpu_ppll2_lock, flags);
	msg.fid = FFA_MSG_CLK_DPU;
	msg.data0 = flag;
	ret = xrsp_ffa_direct_message(&msg);
	if ((ret != 0) || (msg.ret != 0)) {
		clkerr("dpu ppll2 vote bypass failed: ret=%d, msg.ret=%lu, flag %u\n",
			ret, msg.ret, flag);
		goto byps_out;
	}

	if (flag == DPU_PPLL2_UNBYPASS) {
		lock_base = get_xr_clk_base(XR_CLK_CRGCTRL);
		pll_cfg = ppll2_dpu_cfg_get();
		lock_base += pll_cfg->lock_ctrl[0];
		ret = wait_ap_pll_lock(lock_base, XR_PPLL2_DPU, pll_cfg->lock_ctrl[1]);
	}

byps_out:
	spin_unlock_irqrestore(&dpu_ppll2_lock, flags);
	return ret;
}
EXPORT_SYMBOL_GPL(dpu_ppll2_vote_bypass);
