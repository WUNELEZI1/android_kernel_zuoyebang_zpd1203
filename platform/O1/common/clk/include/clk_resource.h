/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __PLATFORM_CLK_RESOURCE_H__
#define __PLATFORM_CLK_RESOURCE_H__

/* target volt exceeds lowtemp limit volt */
#define ELOW_TEMP       0xE333
/* target volt exceeds the fixed volt */
#define EFIXED_VOLT     0xE334

#define PPLL2_ACPU_VOTE_BIT 0
#define PPLL2_DPU_VOTE_BIT  2

#define LMS_TIMER_RATE_HIGH 4095563  /* Hz*/
#define LMS_TIMESTAMP_RATE  2730375  /* Hz*/

#define BL31_GATE_DISABLE    0
#define BL31_GATE_ENABLE     1

/* bl2 ctrl cmd for pll */
#define BL2_CLK_OPS_READ     0
#define BL2_CLK_OPS_WRITE    1

/* ffa msg group and fid definition,same as bl31_smc_id,
 * but we need to know ffa_id is not bl31_smc_id actually.
 * Note: ffa_fid must same as xsp definition in xsp/.../msg_fid /src/lib.rs
 */
#define MAKE_FID(group, id)  ((((0xc3U << 16) + (group)) << 8) + (id))

#define FFA_CLK_GROUP      0x28

#define FFA_MSG_CLK_DVFS    MAKE_FID(FFA_CLK_GROUP, 0x00)
#define FFA_MSG_CLK_DPU     MAKE_FID(FFA_CLK_GROUP, 0x01)
#define FFA_MSG_CLK_PLL     MAKE_FID(FFA_CLK_GROUP, 0x02)
#define FFA_MSG_CLK_SECDIV  MAKE_FID(FFA_CLK_GROUP, 0x03)
#define FFA_MSG_CLK_SECGATE MAKE_FID(FFA_CLK_GROUP, 0x04)

#define FFA_CLK_OPS_READ    0
#define FFA_CLK_OPS_WRITE   1

#define FFA_GATE_DISABLE    0
#define FFA_GATE_ENABLE     1

#define FFA_PLL_DISABLE     0
#define FFA_PLL_ENABLE      1
#define FFA_PLL_SET_RATE    2

#define FFA_DVS_CH_BYPASS   0
#define FFA_DVS_CH_UNBYPASS 1

#define DPU_PPLL2_BYPASS    0
#define DPU_PPLL2_UNBYPASS  1
#define DPU_PPLL2_MAX       2

enum {
	XR_DPU_DVS_CFG = 1,
	XR_VDEC_DVS_CFG,
	XR_ISP_DVS_CFG,
	XR_VENC_DVS_CFG,
	XR_OCM_DVS_CFG,
	XR_MAX_DVS_CFG,
};

enum PCIEPLL_VCO_RATE {
	VCO_RATE_NO_CARE,
	VCO_RATE_2D4G,
	VCO_RATE_4D8G,
};

enum SAFE_GATE_ID {
	SAFE_PCLK_ONEWIRE0,
	SAFE_PCLK_ONEWIRE1,
	SAFE_PCLK_ONEWIRE2,
	SAFE_CLK_ONEWIRE0,
	SAFE_CLK_ONEWIRE1,
	SAFE_CLK_ONEWIRE2,
	SAFE_PCLK_SPI_DMA_GT,
	SAFE_CLK_SPI_DMA_SC_GT,
	SAFE_CLK_SPI_DMA_GT,
	SAFE_CLK_SYS_DIV_SC_GT,
	SAFE_GATE_ID_MAX,
};

enum SAFE_DIVIDER_ID {
	SAFE_DIV_CLK_SPI_DMA,
	SAFE_DIV_ID_MAX,
};

enum SAFE_PLL_ID {
	SAFE_PLL_PCIE_PLL,
	SAFE_PLL_DP_PLL,
	SAFE_PLL_DPU_PLL,
	SAFE_PLL_ID_MAX,
};

enum {
	PERI_DVFS_VOLT_0,
	PERI_DVFS_VOLT_1,
	PERI_DVFS_VOLT_2,
	PERI_DVFS_VOLT_3,
	PERI_DVFS_VOLT_MAX,
};

enum {
	PERI_DVFS_FREQ_0,
	PERI_DVFS_FREQ_1,
	PERI_DVFS_FREQ_2,
	PERI_DVFS_FREQ_3,
	PERI_DVFS_FREQ_MAX,
};

#endif
