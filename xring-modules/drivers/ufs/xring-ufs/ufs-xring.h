/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_H__
#define __UFS_XRING_H__

#include <linux/clk.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/version.h>
#include "ufs-xring-fpga-mphy.h"
#include <ufs/ufshcd.h>

#define UFS_HW_VER_MAJOR_SHIFT	(8)
#define UFS_HW_VER_MAJOR_MASK	(0x00FF << UFS_HW_VER_MAJOR_SHIFT)
#define UFS_HW_VER_MINOR_SHIFT	(4)
#define UFS_HW_VER_MINOR_MASK	(0x000F << UFS_HW_VER_MINOR_SHIFT)
#define UFS_HW_VER_STEP_SHIFT	(0)
#define UFS_HW_VER_STEP_MASK	(0x000F << UFS_HW_VER_STEP_SHIFT)

#define UFS_WAIT_FOR_REG_TIMEOUT 500 /* timeout 500ms */

#define UFS_CLK_DIV_READY (HSS1_CRG_PERISTAT0_DIV_DONE_CLK_HSS1_BUS_CFG_MASK |\
			HSS1_CRG_PERISTAT0_DIV_DONE_CLK_UFS_CORE_MASK |\
			HSS1_CRG_PERISTAT0_DIV_DONE_CLK_UFS_MPHY_CFG_MASK)

#define UFS_HSS1_CRG_RESET_DERESET_VALUE 0x00100081

#if IS_ENABLED(CONFIG_XRING_DEBUG)
// for power_mode sysfs
#define PA_PEERSCRAMBLING 0x155B
#define PA_SCRAMBLING 0x1585
#define PA_SCRAMBLING_UNCHANGE 0x2
#define PA_HS_MODE_UNCHANGE 0x0
#define DOORBELL_CLR_TOUT_US		(10 * 1000 * 1000)		/* 10 sec */
#endif

#define BUSTHRTL_CGE_MASK BIT(12)
#define BUSTHRTL_CGE_SHIFT 12

/* Clock Divider Values: Hex equivalent of frequency in MHz */
enum ufs_clk_div_values {
	DWC_UFS_REG_HCLKDIV_DIV_62_5	= 0x3e,
	DWC_UFS_REG_HCLKDIV_DIV_125	= 0x7d,
	DWC_UFS_REG_HCLKDIV_DIV_200	= 0xc8,
};

/* Selector Index */
enum selector_index {
	SELIND_LN0_TX		= 0x00,
	SELIND_LN1_TX		= 0x01,
	SELIND_LN0_RX		= 0x04,
	SELIND_LN1_RX		= 0x05,
};

/* mphy pro calibration */
enum mphy_pro_status {
	MPHY_PRO_UNCALIBRATION = 0x00,
	MPHY_PRO_CALIBRATION_STEP1 = 0x01,
	MPHY_PRO_CALIBRATION_STEP2 = 0x02,
	MPHY_PRO_CALIBRATIONED = 0x03,
	MPHY_PRO_MAX,
};

enum mphy_refclk {
	REF_CLK_EXTERNAL = 0,
	REF_CLK_INTERNAL = 1,
};

#define CALIBRATION_DATA_COUNT 72
struct ufs_reserved_memory {
	u32 refclk;
	u16 calibration_data[CALIBRATION_DATA_COUNT];
};

struct ufs_xring_clk {
	struct clk *clk_ufs_mphy_cfg;
	struct clk *pclk_ufs_apb;
	struct clk *clk_ufs_core;
	struct clk *aclk_ufsctrl_abrg_slv;
	struct clk *aclk_ufsesi_abrg_slv;
	struct clk *aclk_ufsctrl_tpc;
};

struct ufs_xring_host {
	u32 caps;
	struct ufs_hba *hba;
	struct ufs_pa_layer_attr dev_req_params;
	struct ufs_xring_clk *clk;
	void __iomem *ufs_sctrl_mmio;
	void __iomem *ufs_mphy_mmio;
	void __iomem *dev_ref_clk_ctrl_mmio;
	struct regmap *hss1_regmap;
	struct regmap *peri_crg_regmap;
	struct regmap *lms_crg_regmap;
	struct regmap *lpis_actrl_regmap;
	struct regmap *sys_ctrl_regmap;
	bool is_dev_ref_clk_enabled;
	bool is_fpga;
	bool is_emu;
	bool is_mphy_fw_sram_init;
	struct ufs_reserved_memory *resv_mm;
	u32 dev_ref_clk_en_mask;
	/* Reset control of HCI */
	struct reset_control *core_reset;
	struct reset_controller_dev *rcdev;

	/* FPGA platform M-PHY PRO Calibration */
	u32 mphy_pro_calibration_status;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 17))
	bool esi_supported;
	bool esi_is_requested;
	u32 mcq_queue_depth;
	unsigned int esi_base;
#endif
};


#define ufs_sctrl_writel(host, val, reg)	\
	writel((val), (host)->ufs_sctrl_mmio + (reg))
#define ufs_sctrl_readl(host, reg)	\
	readl((host)->ufs_sctrl_mmio + (reg))


/**
 * ufshcd_rmwl - read modify write into a register
 * @hba - per adapter instance
 * @mask - mask to apply on read value
 * @val - actual value to write
 * @reg - register address
 */
static inline void ufs_sctrl_rmwl(struct ufs_xring_host *host, u32 mask, u32 val, u32 reg)
{
	u32 tmp;

	tmp = ufs_sctrl_readl(host, reg);
	tmp &= ~mask;
	tmp |= (val & mask);
	ufs_sctrl_writel(host, tmp, reg);
}

struct ufs_xring_dme_attr_val {
	u32 attr_sel;
	u32 mib_val;
	u8 peer;
};

#define ufs_xring_is_link_off(hba) ufshcd_is_link_off(hba)
#define ufs_xring_is_link_active(hba) ufshcd_is_link_active(hba)
#define ufs_xring_is_link_hibern8(hba) ufshcd_is_link_hibern8(hba)

int ufs_xring_wait_for_sctrl_reg(struct ufs_hba *hba, u32 reg, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms);

int ufs_xring_subsys_reset(struct ufs_hba *hba);
int ufs_host_controller_init(struct ufs_hba *hba);

enum UFS_MPHY_ATTR {
	CBATTR2APBCTRL				= 0x8115,
	RX_CDR_ACTIVE_LATENCY_ADJUST		= 0x8019,
	VS_MPHYRST				= 0xd0c3,
	VS_MPHYDISABLE				= 0xd0c1,
	MPHY_RX0_ASYNC_FILTER		= 0x8013,
	VS_MPHYCBULPH8				= 0x8118,
};
#define MPHY_LATENCY_ADJUST_RX0_SELECTOR	0x4
#define MPHY_LATENCY_ADJUST_RX1_SELECTOR	0x5
#define UFS_PHY_DEASSERT_READY (UFS_SYS_CTRL_DWC_MPHY_STAT_TX_CFGRDYN_0_MASK |\
			UFS_SYS_CTRL_DWC_MPHY_STAT_TX_CFGRDYN_1_MASK |\
			UFS_SYS_CTRL_DWC_MPHY_STAT_RX_CFGRDYN_0_MASK |\
			UFS_SYS_CTRL_DWC_MPHY_STAT_RX_CFGRDYN_1_MASK)

#define UFS_MPHY_INIT_DONE (UFS_SYS_CTRL_DWC_MPHY_STAT_TX_BUSY_0_MASK |\
					UFS_SYS_CTRL_DWC_MPHY_STAT_TX_BUSY_1_MASK |\
					UFS_SYS_CTRL_DWC_MPHY_STAT_RX_BUSY_0_MASK |\
					UFS_SYS_CTRL_DWC_MPHY_STAT_RX_BUSY_1_MASK)

/* mphy calibration */
#define MPHY_CBAPBPADDRLSB			0x810e
#define MPHY_CBAPBPADDRMSB			0x810f
#define MPHY_CBAPBPWDATALSB			0x8110
#define MPHY_CBAPBPWDATAMSB			0x8111
#define MPHY_CBAPBPWRITESEL			0x8114
#define MPHY_CALI_DONE_TIMEOUT_MS		50

#define MPHY_RAWCMN_DIG_AON_CMNCAL_MPLL_STATUS				0x017c
#define MPHY_RAWLANEAON0_DIG_RX_AFE_DFE_CAL_DONE			0x3128
#define MPHY_RAWLANEAON1_DIG_RX_AFE_DFE_CAL_DONE			0x3328
#define MPHY_RAWLANEAON0_DIG_RX_STARTUP_CAL_ALGO_CTL_1			0x3101
#define MPHY_RAWLANEAON1_DIG_RX_STARTUP_CAL_ALGO_CTL_1			0x3301
#define MPHY_RAWLANEAON0_DIG_RX_CONT_ALGO_CTL				0x3103
#define MPHY_RAWLANEAON1_DIG_RX_CONT_ALGO_CTL				0x3303

#define MPHY_RAWCMN_DIG_AON_MPLLA_TUNE_BANK_0				0x0162
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_0		0x3021
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_0		0x3221
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_FULL_BANK_0			0x3022
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_FULL_BANK_0			0x3222
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_HALF_BANK_0			0x3023
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_HALF_BANK_0			0x3223
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_CAL_DONE_BANK_0			0x3039
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_CAL_DONE_BANK_0			0x3239
#define MPHY_RAWLANEAON0_DIG_RX_DCC_CTRL_RANGE_BANK_0			0x313c
#define MPHY_RAWLANEAON1_DIG_RX_DCC_CTRL_RANGE_BANK_0			0x333c
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_DATA_BANK_0			0x313d
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_DATA_BANK_0			0x333d
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_BYP_BANK_0			0x313e
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_BYP_BANK_0			0x333e
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_PHASE_BANK_0			0x313f
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_PHASE_BANK_0			0x333f
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_DATA_BANK_0			0x3140
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_DATA_BANK_0			0x3340
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_BYP_BANK_0			0x3141
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_BYP_BANK_0			0x3341
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_PHASE_BANK_0			0x3142
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_PHASE_BANK_0			0x3342
#define MPHY_RAWLANEAON0_DIG_RX_BYPASS_IQ_CAL_BANK_0			0x3143
#define MPHY_RAWLANEAON1_DIG_RX_BYPASS_IQ_CAL_BANK_0			0x3343
#define MPHY_RAWLANEAON0_DIG_RX_DATA_IQ_CAL_BANK_0			0x3144
#define MPHY_RAWLANEAON1_DIG_RX_DATA_IQ_CAL_BANK_0			0x3344
#define MPHY_RAWLANEAON0_DIG_RX_BYPASS_CAL_DONE_BANK_0			0x3145
#define MPHY_RAWLANEAON1_DIG_RX_BYPASS_CAL_DONE_BANK_0			0x3345
#define MPHY_RAWLANEAON0_DIG_RX_DATA_CAL_DONE_BANK_0			0x3146
#define MPHY_RAWLANEAON1_DIG_RX_DATA_CAL_DONE_BANK_0			0x3346
#define MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_0		0x311e
#define MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_0		0x331e
#define MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_1		0x3120
#define MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_1		0x3320
#define MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_2		0x3122
#define MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_2		0x3322

#define MPHY_RAWCMN_DIG_AON_MPLLA_TUNE_BANK_1				0x0163
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_1		0x3024
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_1		0x3224
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_FULL_BANK_1			0x3025
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_FULL_BANK_1			0x3225
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_HALF_BANK_1			0x3026
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_HALF_BANK_1			0x3226
#define MPHY_RAWLANEAON0_DIG_TX_MPLLA_CAL_DONE_BANK_1			0x303a
#define MPHY_RAWLANEAON1_DIG_TX_MPLLA_CAL_DONE_BANK_1			0x323a
#define MPHY_RAWLANEAON0_DIG_RX_DCC_CTRL_RANGE_BANK_1			0x3147
#define MPHY_RAWLANEAON1_DIG_RX_DCC_CTRL_RANGE_BANK_1			0x3347
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_DATA_BANK_1			0x3148
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_DATA_BANK_1			0x3348
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_BYP_BANK_1			0x3149
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_BYP_BANK_1			0x3349
#define MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_PHASE_BANK_1			0x314a
#define MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_PHASE_BANK_1			0x334a
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_DATA_BANK_1			0x314b
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_DATA_BANK_1			0x334b
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_BYP_BANK_1			0x314c
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_BYP_BANK_1			0x334c
#define MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_PHASE_BANK_1			0x314d
#define MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_PHASE_BANK_1			0x334d
#define MPHY_RAWLANEAON0_DIG_RX_BYPASS_IQ_CAL_BANK_1			0x314e
#define MPHY_RAWLANEAON1_DIG_RX_BYPASS_IQ_CAL_BANK_1			0x334e
#define MPHY_RAWLANEAON0_DIG_RX_DATA_IQ_CAL_BANK_1			0x314f
#define MPHY_RAWLANEAON1_DIG_RX_DATA_IQ_CAL_BANK_1			0x334f
#define MPHY_RAWLANEAON0_DIG_RX_BYPASS_CAL_DONE_BANK_1			0x3150
#define MPHY_RAWLANEAON1_DIG_RX_BYPASS_CAL_DONE_BANK_1			0x3350
#define MPHY_RAWLANEAON0_DIG_RX_DATA_CAL_DONE_BANK_1			0x3151
#define MPHY_RAWLANEAON1_DIG_RX_DATA_CAL_DONE_BANK_1			0x3351
#define MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_3		0x3124
#define MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_3		0x3324
#define MPHY_RAWLANEAON0_DIG_RX_SCOPRE_VDAC_OFST			0x3129
#define MPHY_RAWLANEAON1_DIG_RX_SCOPRE_VDAC_OFST			0x3329

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 17))
#define MCQ_QCFGPTR_MASK	GENMASK(23, 16)
#define MCQ_QCFGPTR_UNIT	0x200
#define MCQ_QCFG_SIZE		0x40

#define ESI_NR_IRQS 96
#define ESI_IRQ_TYPE 6
#define CQ_TEPS_OFFSET 4

#define UFS_ESI_DEVICE_ID_MASK GENMASK(31, 16)


/* Attribute idn for Query requests */
#define QUERY_ATTR_IDN_THROTTING_STATUS  0x1B

/* ufs_sys_ctrl register */
enum {
	UFS_ESI_MASK_0_OFFSET = 0x70,
	UFS_ESI_MASK_1_OFFSET = 0x74,
	UFS_ESI_MASK_2_OFFSET = 0x78,
	UFS_ESI_DEVICE_ID_OFFSET = 0x8c,
};
#endif

static inline void ufs_xring_msi_lock_descs(struct ufs_hba *hba)
{
	mutex_lock(&hba->dev->msi.data->mutex);
}

static inline void ufs_xring_msi_unlock_descs(struct ufs_hba *hba)
{
	mutex_unlock(&hba->dev->msi.data->mutex);
}
#endif
