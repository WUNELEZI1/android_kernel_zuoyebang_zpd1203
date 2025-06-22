// SPDX-License-Identifier: GPL-2.0-only
/*
 * UFS Host driver for Xring-UFS
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 */
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/bits.h>
#include <linux/pm_runtime.h>
#include <linux/cpuhotplug.h>
#include <linux/tracepoint.h>
#include <peri_crg.h>
#include <hss1_crg.h>
#include <ufs_sys_ctrl.h>
#include <DWC_ufshc_header.h>
#include <lms_crg.h>
#include <lpis_actrl.h>
#include <sys_ctrl.h>
#include <trace/hooks/ufshcd.h>

#include <ufs/ufshcd.h>
#include "ufshcd-pltfrm.h"
#include "unipro.h"
#include "ufshci.h"
#include "ufs_quirks.h"
#include "ufs-xring.h"
#include "ufs-xring-fpga.h"
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
#include "ufs-xring-mcq.h"
#endif
#include "ufs-xring-rpmb.h"
#include "ufs-xring-crypto.h"
#include "ufs-xring-debug.h"
#if IS_ENABLED(CONFIG_XRING_CPU_TEST_LPI)
#include "soc/xring/test_lpi.h"
#endif

static u32 hs_mode = PA_HS_MODE_B;
static u32 work_mode = UFS_HS_MODE;
static struct ufs_xring_host *s_ufs_xring_host = NULL;

static u16 mphy_attr[] = {
	MPHY_RAWCMN_DIG_AON_MPLLA_TUNE_BANK_0,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_0,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_0,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_FULL_BANK_0,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_FULL_BANK_0,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_HALF_BANK_0,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_HALF_BANK_0,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_CTRL_RANGE_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_CTRL_RANGE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_DATA_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_DATA_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_BYP_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_BYP_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_PHASE_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_PHASE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_DATA_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_DATA_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_BYP_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_BYP_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_PHASE_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_PHASE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_BYPASS_IQ_CAL_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_BYPASS_IQ_CAL_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DATA_IQ_CAL_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DATA_IQ_CAL_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_BYPASS_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_BYPASS_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_DATA_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_DATA_CAL_DONE_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_0,
	MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_0,
	MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_2,
	MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_2,
	MPHY_RAWCMN_DIG_AON_MPLLA_TUNE_BANK_1,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_1,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_CTRL_RANGE_BANK_1,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_FULL_BANK_1,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_FULL_BANK_1,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_DCC_HALF_BANK_1,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_DCC_HALF_BANK_1,
	MPHY_RAWLANEAON0_DIG_TX_MPLLA_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON1_DIG_TX_MPLLA_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_CTRL_RANGE_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_CTRL_RANGE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_DATA_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_DATA_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_BYP_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_BYP_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_FULL_PHASE_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_FULL_PHASE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_DATA_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_DATA_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_BYP_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_BYP_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DCC_HALF_PHASE_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DCC_HALF_PHASE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_BYPASS_IQ_CAL_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_BYPASS_IQ_CAL_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DATA_IQ_CAL_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DATA_IQ_CAL_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_BYPASS_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_BYPASS_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_DATA_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON1_DIG_RX_DATA_CAL_DONE_BANK_1,
	MPHY_RAWLANEAON0_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_3,
	MPHY_RAWLANEAON1_DIG_RX_AFE_CTLE_IDAC_OFST_BANK_3,
	MPHY_RAWLANEAON0_DIG_RX_SCOPRE_VDAC_OFST,
	MPHY_RAWLANEAON1_DIG_RX_SCOPRE_VDAC_OFST,
};

#define ufs_xring_hex_dump(prefix_str, buf, len) do {                  \
	size_t __len = (len);                                            \
	print_hex_dump(KERN_ERR, prefix_str,                             \
		       __len > 4 ? DUMP_PREFIX_OFFSET : DUMP_PREFIX_NONE,\
		       16, 4, buf, __len, false);                        \
} while (0)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
static struct ufs_dev_fix ufs_xring_dev_fixups[] = {
	UFS_FIX(UFS_VENDOR_MICRON, UFS_ANY_MODEL,
		UFS_DEVICE_QUIRK_DELAY_AFTER_LPM),
	UFS_FIX(UFS_VENDOR_SAMSUNG, UFS_ANY_MODEL,
		UFS_DEVICE_QUIRK_DELAY_AFTER_LPM),
	UFS_FIX(UFS_VENDOR_TOSHIBA, UFS_ANY_MODEL,
		UFS_DEVICE_QUIRK_DELAY_AFTER_LPM),
	UFS_FIX(UFS_VENDOR_SKHYNIX, UFS_ANY_MODEL,
		UFS_DEVICE_QUIRK_DELAY_AFTER_LPM),

	END_FIX
};
#else
static struct ufs_dev_quirk ufs_xring_dev_fixups[] = {
	{ .wmanufacturerid = UFS_VENDOR_MICRON,
	  .model = UFS_ANY_MODEL,
	  .quirk = UFS_DEVICE_QUIRK_DELAY_AFTER_LPM},
	{ .wmanufacturerid = UFS_VENDOR_SAMSUNG,
	  .model = UFS_ANY_MODEL,
	  .quirk = UFS_DEVICE_QUIRK_DELAY_AFTER_LPM},
	{ .wmanufacturerid = UFS_VENDOR_TOSHIBA,
	  .model = UFS_ANY_MODEL,
	  .quirk = UFS_DEVICE_QUIRK_DELAY_AFTER_LPM},
	{ .wmanufacturerid = UFS_VENDOR_SKHYNIX,
	  .model = UFS_ANY_MODEL,
	  .quirk = UFS_DEVICE_QUIRK_DELAY_AFTER_LPM},
	{}
};
#endif

#define NONE_CPU_ID 0xFFFF
static void ufs_xring_irq_set_affinity(struct ufs_hba *hba,  unsigned int cpuid);
static int ufs_xring_cpu_online(unsigned int cpu);

static struct ufs_hba *ufs_host_to_ufs_hba(void *private)
{
	return container_of(&private, struct ufs_hba, priv);
}

/**
 * ufshcd_xring_mphy_wait_for_register - wait for register value to change
 * @hba: per-adapter interface
 * @reg: mmio register offset
 * @mask: mask to apply to the read register value
 * @val: value to wait for
 * @interval_us: polling interval in microseconds
 * @timeout_ms: timeout in milliseconds
 *
 * Return:
 * -ETIMEDOUT on error, zero on 0.
 */
static int ufs_xring_wait_for_hcd_reg(struct ufs_hba *hba, u32 reg, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms)
{
	int ret = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	while ((ufshcd_readl(hba, reg) & mask) != val) {
		usleep_range(interval_us, interval_us + 50);
		if (time_after(jiffies, timeout)) {
			if ((ufshcd_readl(hba, reg) & mask) != val)
				ret = -ETIMEDOUT;
			break;
		}
	}

	return ret;
}

static int peri_crg_readl(struct ufs_xring_host *host, u32 reg, u32 *val)
{
	int ret;
	struct ufs_hba *hba = ufs_host_to_ufs_hba((void *)host);
	struct device *dev = hba->dev;

	ret = regmap_read(host->peri_crg_regmap, reg, val);
	if (ret < 0)
		dev_err_once(dev, "failed to syscon read address:0x%x\n", reg);

	return ret;
}

/**
 * ufs_xring_wait_for_sctrl_reg - wait for ufs_sys_ctrl register value to change
 * @hba: per-adapter interface
 * @reg: mmio register offset
 * @mask: mask to apply to the read register value
 * @val: value to wait for
 * @interval_us: polling interval in microseconds
 * @timeout_ms: timeout in milliseconds
 *
 * Return:
 * -ETIMEDOUT on error, zero on 0.
 */
int ufs_xring_wait_for_sctrl_reg(struct ufs_hba *hba, u32 reg, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms)
{
	int ret = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	while ((ufs_sctrl_readl(host, reg) & mask) != val) {
		usleep_range(interval_us, interval_us + 50);
		if (time_after(jiffies, timeout)) {
			if ((ufs_sctrl_readl(host, reg) & mask) != val)
				ret = -ETIMEDOUT;
			break;
		}
	}

	return ret;
}
void hss1_crg_writel(struct ufs_xring_host *host, u32 val, u32 reg)
{
	int ret;
	struct ufs_hba *hba = ufs_host_to_ufs_hba((void *)host);
	struct device *dev = hba->dev;

	ret = regmap_write(host->hss1_regmap, reg, val);
	if (ret < 0)
		dev_err_once(dev, "failed to syscon write address:0x%x\n", reg);
}

u32 hss1_crg_readl(struct ufs_xring_host *host, u32 reg)
{
	u32 val;
	int ret;
	struct ufs_hba *hba = ufs_host_to_ufs_hba((void *)host);
	struct device *dev = hba->dev;

	ret = regmap_read(host->hss1_regmap, reg, &val);
	if (ret < 0)
		dev_err_once(dev, "failed to syscon read address:0x%x\n", reg);

	return val;
}
/**
 * ufshcd_wait_for_hss1_crg_register - wait for register value to change
 * @hba: per-adapter interface
 * @reg: mmio register offset
 * @mask: mask to apply to the read register value
 * @val: value to wait for
 * @interval_us: polling interval in microseconds
 * @timeout_ms: timeout in milliseconds
 *
 * Return:
 * -ETIMEDOUT on error, zero on success.
 */
int ufs_xring_wait_for_hss1_crg_register(struct ufs_hba *hba, u32 reg, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms)
{
	int err = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	while ((hss1_crg_readl(host, reg) & mask) != val) {
		usleep_range(interval_us, interval_us + 50);
		if (time_after(jiffies, timeout)) {
			if ((hss1_crg_readl(host, reg) & mask) != val)
				err = -ETIMEDOUT;
			break;
		}
	}

	return err;
}

int ufs_sysctrl_dump_regs(struct ufs_hba *hba, size_t offset, size_t len,
		     const char *prefix)
{
	u32 *regs;
	size_t pos;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (offset % 4 != 0 || len % 4 != 0) /* keep readl happy */
		return -EINVAL;

	regs = kzalloc(len, GFP_ATOMIC);
	if (!regs)
		return -ENOMEM;

	for (pos = 0; pos < len; pos += 4)
		regs[pos / 4] = ufs_sctrl_readl(host, offset + pos);

	ufs_xring_hex_dump(prefix, regs, len);
	kfree(regs);

	return 0;
}

static int ufs_xring_open_hss1_clk(struct ufs_hba *hba)
{
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct ufs_xring_clk *clk = host->clk;

	ret = clk_prepare_enable(clk->clk_ufs_mphy_cfg);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk clk_ufs_mphy_cfg\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->pclk_ufs_apb);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk pclk_ufs_apb\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->clk_ufs_core);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk clk_ufs_core\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->aclk_ufsctrl_abrg_slv);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk aclk_ufsctrl_abrg_slv\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->aclk_ufsesi_abrg_slv);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk aclk_ufsesi_abrg_slv\n");
		return ret;
	}

	ret = clk_prepare_enable(clk->aclk_ufsctrl_tpc);
	if (ret) {
		dev_err(hba->dev, "failed to enable clk aclk_ufsctrl_tpc\n");
		return ret;
	}

	return 0;
}

static void ufs_xring_close_hss1_clk(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct ufs_xring_clk *clk = host->clk;

	clk_disable_unprepare(clk->clk_ufs_mphy_cfg);
	clk_disable_unprepare(clk->pclk_ufs_apb);
	clk_disable_unprepare(clk->clk_ufs_core);
	clk_disable_unprepare(clk->aclk_ufsctrl_abrg_slv);
	clk_disable_unprepare(clk->aclk_ufsesi_abrg_slv);
	clk_disable_unprepare(clk->aclk_ufsctrl_tpc);
}

int ufs_xring_subsys_reset(struct ufs_hba *hba)
{
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	hss1_crg_writel(host, UFS_HSS1_CRG_RESET_DERESET_VALUE, HSS1_CRG_RST0_W1C);

	usleep_range(5, 10);

	ufs_xring_close_hss1_clk(hba);

	hss1_crg_writel(host, UFS_HSS1_CRG_RESET_DERESET_VALUE, HSS1_CRG_RST0_W1S);

	usleep_range(5, 10);

	ret = ufs_xring_open_hss1_clk(hba);
	if (ret) {
		dev_err(hba->dev, "ufs open hss1_clk fail\n");
		return ret;
	}

	return 0;
}

/**
 * ufs_xring_program_clk_div()
 * This function programs the clk divider value.
 * @hba: Private Structure pointer
 *
 */
static int ufs_xring_program_div_clk(struct ufs_hba *hba)
{
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct ufs_xring_clk *clk = host->clk;

	/* config div_ufs_clk_core: 1 */
	ret = clk_set_rate(clk->clk_ufs_core, 417792000);
	if (ret) {
		dev_err(hba->dev, "config div_ufs_clk_core fail:%d\n", ret);
		return ret;
	}

	/* config div_clk_ufs_mphy_cfg: 4 */
	ret = clk_set_rate(clk->clk_ufs_mphy_cfg, 104450000);
	if (ret) {
		dev_err(hba->dev, "config div_ufs_clk_core fail:%d\n", ret);
		return ret;
	}

	/* plk_ufs_apb :2 */
	ret = clk_set_rate(clk->pclk_ufs_apb, 208900000);
	if (ret) {
		dev_err(hba->dev, "config pclk_ufs_apb fail:%d\n", ret);
		return ret;
	}

	return ret;
}

static void ufs_xring_mphy_fw_init(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* mphy fw init mode select */
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_BYPASS_MODE_MASK,
				0x0, UFS_SYS_CTRL_DWC_MPHY_CTRL);
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_EXT_LD_DONE_MASK,
				0x0, UFS_SYS_CTRL_DWC_MPHY_CTRL);

	if (host->is_mphy_fw_sram_init) {
		/* sram mode select */
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_BYPASS_MODE_MASK,
					0x0, UFS_SYS_CTRL_DWC_MPHY_CTRL);
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_EXT_LD_DONE_MASK,
					0x0, UFS_SYS_CTRL_DWC_MPHY_CTRL);
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_BOOTLOAD_BYPASS_MODE_MASK,
					0x3 << UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT,
					UFS_SYS_CTRL_DWC_MPHY_CTRL);
		/* ufs mphy sram sel */
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_UFS_MISC_CTRL_UFS_MPHY_SRAM_EXT_SEL_MASK,
					0x1 << UFS_SYS_CTRL_UFS_MISC_CTRL_UFS_MPHY_SRAM_EXT_SEL_SHIFT,
					UFS_SYS_CTRL_UFS_MISC_CTRL);
		/* sram clk enable:1 */
		ufs_sctrl_writel(host, 0x100, UFS_SYS_CTRL_UFS_SYS_RST_SWGT_W1S);

		usleep_range(1, 5);

		/* sram clk clear:0 */
		ufs_sctrl_writel(host, 0x100, UFS_SYS_CTRL_UFS_SYS_RST_SWGT_W1C);

		usleep_range(1, 5);
		/* set dwc_mphy access mphy sram */
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_UFS_MISC_CTRL_UFS_MPHY_SRAM_EXT_SEL_MASK,
					0x0, UFS_SYS_CTRL_UFS_MISC_CTRL);
	}
}

static void ufs_xring_mphy_clk_cfg(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* ufs mphy CG tick config */
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_UFS_MISC_CTRL_CG_TICK1US_DIV_MASK,
				0x72, UFS_SYS_CTRL_UFS_MISC_CTRL);

	/* ufs mphy cfg clock sel */
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_CFG_CLOCK_SEL_MASK,
				0x4, UFS_SYS_CTRL_DWC_MPHY_CTRL);

	/* off-chip mphy ref clk config */
	if (!IS_ERR_OR_NULL(host->resv_mm) && host->resv_mm->refclk == REF_CLK_EXTERNAL)
		ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_MPHY_CFG_MASK,
				0x3 << UFS_SYS_CTRL_DWC_MPHY_CTRL_MPHY_CFG_SHIFT,
				UFS_SYS_CTRL_DWC_MPHY_CTRL);
}

int ufs_host_controller_init(struct ufs_hba *hba)
{
	int ret = 0;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* step: UFS HC init,do de-assert reset */
	/* UFS_SCTRL: ufshc reset de-assert */
	ufs_sctrl_writel(host, 0x1, UFS_SYS_CTRL_UFS_SYS_RST_SWGT_W1S);

	if (host->is_fpga) {
		/* UFS_CTRL: HCE set 1 */
		ufshcd_writel(hba, CONTROLLER_ENABLE, REG_CONTROLLER_ENABLE);

		/* UFS_CTRL: HCE completion is 1 */
		ret = ufs_xring_wait_for_hcd_reg(hba, REG_CONTROLLER_ENABLE,
			CONTROLLER_ENABLE, CONTROLLER_ENABLE,
			10, UFS_WAIT_FOR_REG_TIMEOUT);
		if (ret) {
			dev_err(hba->dev, "%s: %d, REG_CONTROLLER_ENABLE = 0x%x.\n",
				__func__, __LINE__, ufshcd_readl(hba, REG_CONTROLLER_ENABLE));
			dev_err(hba->dev, "%s: Controller enable failed\n", __func__);
			return ret;
		}

		/* add enable_intc ops : enable UIC intc*/
		ufshcd_writel(hba, ufshcd_readl(hba,
			REG_INTERRUPT_ENABLE) | UFSHCD_UIC_MASK, REG_INTERRUPT_ENABLE);
	}

	return ret;
}

static void ufshcd_write_mphy_register(struct ufs_hba *hba, u16 addr, u16 data)
{
	int ret;

	(void)ufshcd_dme_set(hba, UIC_ARG_MIB(MPHY_CBAPBPADDRLSB), (addr & 0xff));
	(void)ufshcd_dme_set(hba, UIC_ARG_MIB(MPHY_CBAPBPADDRMSB), ((addr >> 8) & 0xff));

	(void)ufshcd_dme_set(hba, UIC_ARG_MIB(MPHY_CBAPBPWDATALSB), (data & 0xff));
	(void)ufshcd_dme_set(hba, UIC_ARG_MIB(MPHY_CBAPBPWDATAMSB), ((data >> 8) & 0xff));

	(void)ufshcd_dme_set(hba, UIC_ARG_MIB(MPHY_CBAPBPWRITESEL), 0x1);

	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
					UFS_SYS_CTRL_DWC_MPHY_STAT_TX_BUSY_0_MASK, 0x0, 10,
					MPHY_CALI_DONE_TIMEOUT_MS);
	if (ret)
		dev_err(hba->dev, "wait for mphy calibration write timeout\n");
}

static void ufs_xring_write_calibration_data(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	u16 *val = host->resv_mm->calibration_data;
	u8 i;

	for (i = 0; i < CALIBRATION_DATA_COUNT; i++)
		ufshcd_write_mphy_register(hba, mphy_attr[i], val[i]);

	/* value is provided from synopsys, config mphy calibration done attr */
	ufshcd_write_mphy_register(hba, MPHY_RAWCMN_DIG_AON_CMNCAL_MPLL_STATUS, 0x0002);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON0_DIG_RX_AFE_DFE_CAL_DONE, 0x000c);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON1_DIG_RX_AFE_DFE_CAL_DONE, 0x000c);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON0_DIG_RX_STARTUP_CAL_ALGO_CTL_1, 0x0002);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON1_DIG_RX_STARTUP_CAL_ALGO_CTL_1, 0x0002);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON0_DIG_RX_CONT_ALGO_CTL, 0x0060);
	ufshcd_write_mphy_register(hba, MPHY_RAWLANEAON1_DIG_RX_CONT_ALGO_CTL, 0x0060);
}

static int ufs_get_reserved_memory(struct ufs_hba *hba)
{
	int ret;
	unsigned long long reg[2];
	unsigned long long base_addr = 0;
	unsigned long long rmem_size = 0;
	struct device_node *np = hba->dev->of_node;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	np = of_find_node_by_path("/reserved-memory/rsv_mem_ufs_param@70001000");
	if (IS_ERR(np)) {
		dev_err(hba->dev, "failed to find reserved memory node!\n");
		return PTR_ERR(np);
	}

	ret = of_property_read_u64_array(np, "reg", reg, 2);
	if (ret) {
		dev_err(hba->dev, " Failed to get reserve memory reg!\n");
		return ret;
	}

	base_addr = reg[0];
	rmem_size = reg[1];

	/* ufs reserved memory IO remap */
	host->resv_mm = (struct ufs_reserved_memory *)ioremap_wc(base_addr, rmem_size);

	if (IS_ERR_OR_NULL(host->resv_mm)) {
		dev_err(hba->dev, "%s host reserved mm ioremap fail\n", __func__);
		return PTR_ERR(host->resv_mm);
	}

	return 0;
}

__maybe_unused static int ufs_xring_init_mphy(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	int ret;

	/* wait for phy deassert reset complete */
	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
								UFS_PHY_DEASSERT_READY, 0x0,
								10,
								UFS_WAIT_FOR_REG_TIMEOUT);
	if (ret) {
		dev_err(hba->dev, "%s wait for phy deassert reset fail ret=%d.\n", __func__, ret);
		goto out;
	}

	/* set apb access ufs mphy */
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBATTR2APBCTRL), 0x01);

	/* set phy rx implementation attr */
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_CDR_ACTIVE_LATENCY_ADJUST,
									MPHY_LATENCY_ADJUST_RX0_SELECTOR), 0x15);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(RX_CDR_ACTIVE_LATENCY_ADJUST,
									MPHY_LATENCY_ADJUST_RX1_SELECTOR), 0x15);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MPHY_RX0_ASYNC_FILTER,
									MPHY_LATENCY_ADJUST_RX0_SELECTOR), 0x1);
	ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(MPHY_RX0_ASYNC_FILTER,
									MPHY_LATENCY_ADJUST_RX1_SELECTOR), 0x1);
	/* tx_cfgupdt_0/1 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYCFGUPDT), 0x01);

	/* phy dereset */
	ufs_sctrl_writel(host, 0x0, UFS_SYS_CTRL_DWC_MPHY_RST_OVRD);

	/* ufs device RST_N */
	ufs_sctrl_writel(host, 0x1 << UFS_SYS_CTRL_UFS_DEVC_RST_N_UFS_RST_N_SHIFT,
				UFS_SYS_CTRL_UFS_DEVC_RST_N);

	/* poll until sram init done */
	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
						UFS_SYS_CTRL_DWC_MPHY_STAT_SRAM_INIT_DONE_MASK, 1,
						10,
						UFS_WAIT_FOR_REG_TIMEOUT);
	if (ret) {
		dev_err(hba->dev, "%s UFS mphy sram init fail ret=%d.\n", __func__, ret);
		goto out;
	}

	if (IS_ERR_OR_NULL(host->resv_mm)) {
		dev_err(hba->dev, "%s host reserved mm is err or fail no need to write calibration data\n", __func__);
		return PTR_ERR(host->resv_mm);
	}

	/* write calibration data */
	ufs_xring_write_calibration_data(hba);

	/* write sram_ext_ld_done */
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_EXT_LD_DONE_MASK,
			0x1 << UFS_SYS_CTRL_DWC_MPHY_CTRL_SRAM_EXT_LD_DONE_SHIFT,
			UFS_SYS_CTRL_DWC_MPHY_CTRL);

	/* deassert mphy disable */
	ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYDISABLE), 0x0);

	/* polling until phy init done */
	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
						UFS_MPHY_INIT_DONE, 0,
						10,
						UFS_WAIT_FOR_REG_TIMEOUT);
	if (ret) {
		dev_err(hba->dev, "%s UFS mphy init fail ret=%d.\n", __func__, ret);
		goto out;
	}

out:
	return ret;
}

static bool ufs_xring_crypto_enable(struct ufs_hba *hba)
{
	if (!(hba->caps & UFSHCD_CAP_CRYPTO))
		return false;

	/* Reset might clear all keys, so reprogram all the keys. */
	blk_crypto_reprogram_all_keys(&hba->crypto_profile);

	if (hba->android_quirks & UFSHCD_ANDROID_QUIRK_BROKEN_CRYPTO_ENABLE)
		return false;

	return true;
}

static int ufs_xring_hce_enable_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	int ret = 0;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	switch (status) {
	case PRE_CHANGE:
		if (host->is_fpga) {
			/* excute m-phy pro calibration */
			ret = ufs_xring_setup_mphy_pro_calibration(hba);
			if (ret)
				dev_err(hba->dev, "setup mphy pro calibration failed (%d)\n", ret);

			/* excute m-phy power up seq */
			ret = ufs_xring_setup_mphy_pwr_up_seq(hba);
			if (ret)
				dev_err(hba->dev, "setup mphy pwrup seq failed (%d)\n", ret);

			/* Clear IS and UEC err status. ROC */
			ufshcd_writel(hba, 0xFFFFFFFF, REG_INTERRUPT_STATUS);
			ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER);
			ufshcd_readl(hba, REG_UIC_ERROR_CODE_DATA_LINK_LAYER);
			ufshcd_readl(hba, REG_UIC_ERROR_CODE_NETWORK_LAYER);
			ufshcd_readl(hba, REG_UIC_ERROR_CODE_TRANSPORT_LAYER);
			ufshcd_readl(hba, REG_UIC_ERROR_CODE_DME);
		} else {
			/* xring power up */
			ret = ufs_xring_subsys_reset(hba);
			if (ret) {
				dev_err(hba->dev, "%s ufs subsystem reset fail:%d\n", __func__, ret);
				return ret;
			}
			ret = ufs_xring_program_div_clk(hba);
			if (ret) {
				dev_err(hba->dev, "%s: div clk fail\n", __func__);
				return ret;
			}
			ufs_xring_mphy_clk_cfg(hba);
			ufs_xring_mphy_fw_init(hba);
			ret = ufs_host_controller_init(hba);
			if (ret) {
				dev_err(hba->dev, "%s : %d, ufs_host_controller_init fail, ret is = %d.\n",
					__func__, __LINE__, ret);
				return ret;
			}
		}
		break;

	case POST_CHANGE:
		if (host->is_fpga)
			break;

		/* disable AH8 Lower power gating */
		ufshcd_rmwl(hba, GENMASK(17, 16), 0x0, BUSTHRTL);
		/* setting axi */
		ufshcd_rmwl(hba, GENMASK(15, 13), 0x3 << 13, BUSTHRTL);

		if (host->is_emu)
			break;

		if (IS_ENABLED(CONFIG_XRING_UFS_CRYPTO)) {
			if (!ufs_xring_crypto_enable(hba)) {
				ret = -EOPNOTSUPP;
				dev_err(hba->dev, "%s ufs hw key reprogram fail %d\n", __func__, ret);
				return ret;
			}
		}
		ret = ufs_xring_init_mphy(hba);
		if (ret)
			return ret;

		/* enable mphy ULP */
		ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYCBULPH8), 0x01);

		/* tx_cfgupdt_0/1 */
		ufshcd_dme_set(hba, UIC_ARG_MIB(VS_MPHYCFGUPDT), 0x01);
		break;
	default:
		dev_err(hba->dev, "%s: invalid status %d\n", __func__, status);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void ufs_xring_dev_ref_clk_ctrl(struct ufs_xring_host *host, bool enable)
{
	u32 temp;

	if (host->dev_ref_clk_ctrl_mmio &&
		(enable ^ host->is_dev_ref_clk_enabled)) {
		temp = readl(host->dev_ref_clk_ctrl_mmio);

		if (enable)
			temp |= host->dev_ref_clk_en_mask;
		else
			temp &= ~host->dev_ref_clk_en_mask;

		/*
		 * If we are here to disable this clock it might be immediately
		 * after entering into hibern8 in which case we need to make
		 * sure that device ref_clk is active for specific time after
		 * hibern8 enter.
		 */
		if (!enable) {
			unsigned long gating_wait;

			gating_wait = host->hba->dev_info.clk_gating_wait_us;
			if (!gating_wait) {
				udelay(1);
			} else {
				/*
				 * bRefClkGatingWaitTime defines the minimum
				 * time for which the reference clock is
				 * required by device during transition from
				 * HS-MODE to LS-MODE or HIBERN8 state. Give it
				 * more delay to be on the safe side.
				 */
				gating_wait += 10;
				usleep_range(gating_wait, gating_wait + 10);
			}
		}

		writel(temp, host->dev_ref_clk_ctrl_mmio);

		/*
		 * Make sure the write to ref_clk reaches the destination and
		 * not stored in a Write Buffer (WB).
		 */
		readl(host->dev_ref_clk_ctrl_mmio);

		/*
		 * If we call hibern8 exit after this, we need to make sure that
		 * device ref_clk is stable for at least 1us before the hibern8
		 * exit command.
		 */
		if (enable)
			udelay(1);

		host->is_dev_ref_clk_enabled = enable;
	}
}

static void ufs_xring_init_pwr_dev_param(struct ufs_xring_host *host,
	struct ufs_dev_params *dev_param)
{
	dev_param->tx_lanes = 2;
	dev_param->rx_lanes = 2;
	dev_param->hs_rx_gear = UFS_HS_G5;
	dev_param->hs_tx_gear = UFS_HS_G5;
	dev_param->pwm_rx_gear = UFS_PWM_G1;
	dev_param->pwm_tx_gear = UFS_PWM_G1;
	dev_param->rx_pwr_pwm = SLOW_MODE;
	dev_param->tx_pwr_pwm = SLOW_MODE;
	dev_param->rx_pwr_hs = FAST_MODE;
	dev_param->tx_pwr_hs = FAST_MODE;
	dev_param->hs_rate = hs_mode;
	dev_param->desired_working_mode = work_mode;
}

static int ufs_xring_pwr_change_notify(struct ufs_hba *hba,
				enum ufs_notify_change_status status,
				struct ufs_pa_layer_attr *dev_max_params,
				struct ufs_pa_layer_attr *dev_req_params)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct ufs_dev_params ufs_xring_cap;
	int ret = 0;

	if (!dev_req_params) {
		dev_err(hba->dev, "%s: incoming dev_req_params is NULL\n", __func__);
		ret = -EINVAL;
		return ret;
	}

	switch (status) {
	case PRE_CHANGE:
		ufshcd_writel(hba, (ufshcd_readl(hba, REG_INTERRUPT_STATUS)), REG_INTERRUPT_STATUS);

		ufs_xring_init_pwr_dev_param(host, &ufs_xring_cap);

		ret = ufshcd_get_pwr_dev_param(&ufs_xring_cap,
			dev_max_params,
			dev_req_params);
		if (ret) {
			dev_info(hba->dev, "%s: failed to determine capabilities\n", __func__);
			break;
		}

		/* enable the device ref clock before changing to HS mode */
		if (!ufshcd_is_hs_mode(&hba->pwr_info) &&
			ufshcd_is_hs_mode(dev_req_params))
			ufs_xring_dev_ref_clk_ctrl(host, true);

		ufshcd_dme_configure_adapt(hba,
			dev_req_params->gear_tx,
			PA_INITIAL_ADAPT);

		break;
	case POST_CHANGE:
		/* cache the power mode parameters to use internally */
		memcpy(&host->dev_req_params,
			dev_req_params, sizeof(*dev_req_params));

		dev_info(hba->dev, "%s,hba->dev_quirks = 0x%x\n", __func__, hba->dev_quirks);

		/* disable the device ref clock if entered PWM mode */
		if (ufshcd_is_hs_mode(&hba->pwr_info) &&
			!ufshcd_is_hs_mode(dev_req_params))
			ufs_xring_dev_ref_clk_ctrl(host, false);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


/**
 * ufs_xring_setup_clocks - enables/disable clocks
 * @hba: host controller instance
 * @on: If true, enable clocks else disable them.
 * @status: PRE_CHANGE or POST_CHANGE notify
 *
 * Returns 0 on success, non-zero on failure.
 */
static int ufs_xring_setup_clocks(struct ufs_hba *hba, bool on,
				enum ufs_notify_change_status status)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	int ret = 0;

	/*
	 * In case ufs_xring_init() is not yet done, simply ignore.
	 * This ufs_xring_setup_clocks() shall be called from
	 * ufs_xring_init() after init is done.
	 */
	if (!host)
		return -EINVAL;

	switch (status) {
	case PRE_CHANGE:
		if (!on) {
			if (!ufs_xring_is_link_active(hba)) {
				/* disable device ref_clk */
				ufs_xring_dev_ref_clk_ctrl(host, false);
			}
		}
		break;
	case POST_CHANGE:
		if (on) {
			/* enable the device ref clock for HS mode*/
			if (ufshcd_is_hs_mode(&hba->pwr_info))
				ufs_xring_dev_ref_clk_ctrl(host, true);
		}
		break;
	default:
		dev_err(hba->dev, "ufs xring setup clocks failed\n");
		break;
	}

	return ret;
}

#if IS_ENABLED(CONFIG_XRING_DEBUG)
static int get_current_scrambling(struct ufs_hba *hba,
					u32 *scrambling, u32 *peer_scrambling)
{
	int ret = 0;

	down(&hba->host_sem);
	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_PEERSCRAMBLING),
				  peer_scrambling);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to get peer scrambling status, ret=%d!\n",
				__func__, ret);
	}

	if (*peer_scrambling == 0x1) {
		dev_info(hba->dev, "%s: Peer device supports scrambling.\n", __func__);
		ret = ufshcd_dme_get(hba, UIC_ARG_MIB(PA_SCRAMBLING), scrambling);
		if (ret) {
			dev_err(hba->dev, "%s: Failed to get UFS HC scrambling status, ret=%d!\n",
				__func__, ret);
		}
	}

	pm_runtime_put_sync(hba->dev);
	up(&hba->host_sem);

	return ret;
}
#endif

static ssize_t power_mode_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	static const char * const names[] = {
		"INVALIDMODE",
		"FASTMODE",
		"SLOWMODE",
		"INVALIDMODE",
		"FASTAUTOMODE",
		"SLOWAUTOMODE",
		"INVALIDMODE",
	};
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	static const char * const scrambles[] = {
		"OFF",
		"ON",
		"PEER_UNSUPPORT",
	};
	u32 scrambling = 0xFFFFFFFF;
	u32 peer_scrambling = 0xFFFFFFFF;
	u32 scrambling_index = 0x0;
	int ret = 0;

	ret = get_current_scrambling(hba, &scrambling, &peer_scrambling);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to get UFS HC scrambling status, ret=%d!\n",
					__func__, ret);
		return -EINVAL;
	}

	dev_info(hba->dev, "%s: peer_scrambling=%u, scrambling=%u.\n",
					__func__, peer_scrambling, scrambling);
	if (peer_scrambling == 0x0) {
		scrambling_index = 2;
	} else if (peer_scrambling == 0x1) {
		if (scrambling == 0x0 || scrambling == 0x1) {
			scrambling_index = scrambling;
		} else {
			dev_err(hba->dev, "%s: Unknown UFS HC scrambling state, scrambling=%u.\n",
					__func__, scrambling);
		}
	} else {
		dev_err(hba->dev, "%s: Unknown peer scrambling state, peer_scrambling=%u.\n",
					__func__, peer_scrambling);
		return -EINVAL;
	}

	/* Print current power info */
	return sysfs_emit(buf,
		"[Rx,Tx]: Gear[%u,%u], Lane[%u,%u], PWR[%s ,%s ], Rate-%c , Scrambling-%s\n",
		hba->pwr_info.gear_rx, hba->pwr_info.gear_tx,
		hba->pwr_info.lane_rx, hba->pwr_info.lane_tx,
		names[hba->pwr_info.pwr_rx],
		names[hba->pwr_info.pwr_tx],
		hba->pwr_info.hs_rate == PA_HS_MODE_B ? 'B' : 'A',
		scrambles[scrambling_index]);
#else
	return sysfs_emit(buf,
		"[Rx,Tx]: Gear[%d,%d], Lane[%d,%d], PWR[%s,%s], Rate-%c\n",
		hba->pwr_info.gear_rx, hba->pwr_info.gear_tx,
		hba->pwr_info.lane_rx, hba->pwr_info.lane_tx,
		names[hba->pwr_info.pwr_rx],
		names[hba->pwr_info.pwr_tx],
		hba->pwr_info.hs_rate == PA_HS_MODE_B ? 'B' : 'A');
#endif
}

#if IS_ENABLED(CONFIG_XRING_DEBUG)
static bool convert_to_u32(const char *source,
			const char * const attr_list[], int num_attr_list, u32 *target)
{
	int index;

	for (index = 0; index < num_attr_list; index++) {
		if (strcmp(source, attr_list[index]) == 0) {
			*target = index;
			return true;
		}
	}

	return false;
}

static int get_power_mode_pwr_rx_tx(struct ufs_hba *hba,
					const char *source, u32 *target, bool is_rx)
{
	static const char * const names[] = {
		"INVALIDMODE",
		"FASTMODE",
		"SLOWMODE",
		"INVALIDMODE",
		"FASTAUTOMODE",
		"SLOWAUTOMODE",
		"INVALIDMODE",
		"DONTCHANGE",
	};

	if (!convert_to_u32(source, names,
		sizeof(names) / sizeof(names[0]), target)) {
		dev_err(hba->dev, "%s: Invalid value for pwr_rx!\n", __func__);
		return -EINVAL;
	}

	if (*target == UNCHANGED) {
		if (is_rx)
			*target = hba->pwr_info.pwr_rx;
		else
			*target = hba->pwr_info.pwr_tx;
	}

	return 0;
}

static int get_power_mode_hs_rate(struct ufs_hba *hba,
					const char *source, u32 *target)
{
	static const char * const rates[] = {
		"DONTCHANGE",
		"A",
		"B",
	};

	if (!convert_to_u32(source, rates,
		sizeof(rates) / sizeof(rates[0]), target)) {
		dev_err(hba->dev, "%s: Invalid value for hs_rate!\n", __func__);
		return -EINVAL;
	}

	if (*target == PA_HS_MODE_UNCHANGE)
		*target = hba->pwr_info.hs_rate;

	return 0;
}

static int get_power_mode_scrambling(struct ufs_hba *hba,
					const char *source, u32 *target)
{
	static const char * const scrambles[] = {
		"OFF",
		"ON",
		"DONTCHANGE",
	};
	int ret = 0;
	u32 peer_scrambling = 0xFFFFFFFF;

	if (!convert_to_u32(source, scrambles,
		sizeof(scrambles) / sizeof(scrambles[0]), target)) {
		dev_err(hba->dev, "%s: Invalid value for scrambling!\n", __func__);
		return -EINVAL;
	}

	if (*target == PA_SCRAMBLING_UNCHANGE) {
		ret = get_current_scrambling(hba, target, &peer_scrambling);
		if (ret) {
			dev_err(hba->dev, "%s: Failed to get UFS HC scrambling status, ret=%d!\n",
				__func__, ret);
			return -EINVAL;
		}
	}

	return 0;
}

static int get_power_mode_gear_rx_tx(struct ufs_hba *hba,
							struct ufs_pa_layer_attr *pwr_info)
{
	if (pwr_info->pwr_rx == FAST_MODE || pwr_info->pwr_rx == FASTAUTO_MODE) {
		if (pwr_info->gear_rx == UFS_HS_DONT_CHANGE) {
			pwr_info->gear_rx = hba->pwr_info.gear_rx;
		} else if (pwr_info->gear_rx < UFS_HS_G1 || pwr_info->gear_rx > UFS_HS_G5) {
			dev_err(hba->dev, "%s: Invalid value for gear_rx:%d, pwr_rx=%u!!\n",
				__func__, pwr_info->gear_rx, pwr_info->pwr_tx);
			return -EINVAL;
		}
	} else if (pwr_info->pwr_rx == SLOW_MODE || pwr_info->pwr_rx == SLOWAUTO_MODE) {
		if (pwr_info->gear_rx == UFS_PWM_DONT_CHANGE) {
			pwr_info->gear_rx = hba->pwr_info.gear_rx;
		} else if (pwr_info->gear_rx < UFS_PWM_G1 || pwr_info->gear_rx > UFS_PWM_G7) {
			dev_err(hba->dev, "%s: Invalid value for gear_rx:%d, pwr_rx=%u!!\n",
				__func__, pwr_info->gear_rx, pwr_info->pwr_rx);
			return -EINVAL;
		}
	} else {
		dev_err(hba->dev, "%s: Invalid value for pwr_rx: %d!\n",
			__func__, pwr_info->pwr_rx);
		return -EINVAL;
	}

	if (pwr_info->pwr_tx == FAST_MODE || pwr_info->pwr_tx == FASTAUTO_MODE) {
		if (pwr_info->gear_tx == UFS_HS_DONT_CHANGE) {
			pwr_info->gear_tx = hba->pwr_info.gear_tx;
		} else if (pwr_info->gear_tx < UFS_HS_G1 || pwr_info->gear_tx > UFS_HS_G5) {
			dev_err(hba->dev, "%s: Invalid value for gear_tx:%d, pwr_tx=%u!\n",
				__func__, pwr_info->gear_tx, pwr_info->pwr_tx);
			return -EINVAL;
		}
	} else if (pwr_info->pwr_tx == SLOW_MODE || pwr_info->pwr_tx == SLOWAUTO_MODE) {
		if (pwr_info->gear_tx == UFS_PWM_DONT_CHANGE) {
			pwr_info->gear_tx = hba->pwr_info.gear_tx;
		} else if (pwr_info->gear_tx < UFS_PWM_G1 || pwr_info->gear_tx > UFS_PWM_G7) {
			dev_err(hba->dev, "%s: Invalid value for gear_tx:%d, pwr_tx=%u!\n",
				__func__, pwr_info->gear_tx, pwr_info->pwr_tx);
			return -EINVAL;
		}
	} else {
		dev_err(hba->dev, "%s: Invalid value for pwr_tx: %d!\n",
			__func__, pwr_info->pwr_tx);
		return -EINVAL;
	}

	return 0;
}

static int get_power_mode_lane_rx_tx(struct ufs_hba *hba,
							struct ufs_pa_layer_attr *pwr_info)
{
	if (pwr_info->lane_rx == UFS_LANE_DONT_CHANGE) {
		pwr_info->lane_rx = hba->pwr_info.lane_rx;
	} else if (pwr_info->lane_rx < UFS_LANE_1 || pwr_info->lane_rx > UFS_LANE_2) {
		dev_err(hba->dev, "%s: Invalid value for lane_rx: %d!\n",
			__func__, pwr_info->lane_rx);
		return -EINVAL;
	}

	if (pwr_info->lane_tx == UFS_LANE_DONT_CHANGE) {
		pwr_info->lane_tx = hba->pwr_info.lane_tx;
	} else if (pwr_info->lane_tx < UFS_LANE_1 || pwr_info->lane_tx > UFS_LANE_2) {
		dev_err(hba->dev, "%s: Invalid value for lane_tx: %d!\n",
			__func__, pwr_info->lane_tx);
		return -EINVAL;
	}

	return 0;
}

static int get_power_mode_attr(struct ufs_hba *hba, const char *buf,
					struct ufs_pa_layer_attr *pwr_info, u32 *scrambling)
{
	char pwr_rx[50] = { 0 };
	char pwr_tx[50] = { 0 };
	char hs_rate[50] = { 0 };
	char scramble[50] = { 0 };
	int ret = 0;

	ret = sscanf(buf, "[Rx,Tx]: Gear[%u,%u], Lane[%u,%u], PWR[%20s ,%20s ], Rate-%10s , Scrambling-%10s",
		&pwr_info->gear_rx, &pwr_info->gear_tx, &pwr_info->lane_rx,
		&pwr_info->lane_tx, pwr_rx, pwr_tx, hs_rate, scramble);
	if (ret != 8) {
		dev_err(hba->dev, "%s: Invalid input format for UFS power_mode:%s, ret = %d!\n",
			__func__, buf, ret);
		return -EINVAL;
	}

	dev_info(hba->dev,
		"%s:[Rx,Tx]: Gear[%u,%u], Lane[%u,%u], PWR[%s ,%s ], Rate-%s , Scrambling-%s\n",
		__func__, pwr_info->gear_rx, pwr_info->gear_tx,
		pwr_info->lane_rx, pwr_info->lane_tx,
		pwr_rx, pwr_tx,
		hs_rate, scramble);

	if (get_power_mode_pwr_rx_tx(hba, pwr_rx, &pwr_info->pwr_rx, true)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for pwr_rx=%s!\n",
			__func__, pwr_rx);
		return -EINVAL;
	}

	if (get_power_mode_pwr_rx_tx(hba, pwr_tx, &pwr_info->pwr_tx, false)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for pwr_tx=%s!\n",
			__func__, pwr_tx);
		return -EINVAL;
	}

	if (get_power_mode_hs_rate(hba, hs_rate, &pwr_info->hs_rate)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for hs_rate=%s!\n",
			__func__, hs_rate);
		return -EINVAL;
	}

	if (get_power_mode_scrambling(hba, scramble, scrambling)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for scrambling=%s!\n",
			__func__, scramble);
		return -EINVAL;
	}

	if (get_power_mode_gear_rx_tx(hba, pwr_info)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for Gear[%d,%d]!\n",
			__func__, pwr_info->gear_rx, pwr_info->gear_tx);
		return -EINVAL;
	}

	if (get_power_mode_lane_rx_tx(hba, pwr_info)) {
		dev_err(hba->dev, "%s: Invalid input attribute value for Lane[%d,%d]!\n",
			__func__, pwr_info->lane_rx, pwr_info->lane_tx);
		return -EINVAL;
	}

	return 0;
}

/*
 * Determine the number of pending commands by counting the bits in the SCSI
 * device budget maps. This approach has been selected because a bit is set in
 * the budget map before scsi_host_queue_ready() checks the host_self_blocked
 * flag. The host_self_blocked flag can be modified by calling
 * scsi_block_requests() or scsi_unblock_requests().
 */
static u32 xring_ufshcd_pending_cmds(struct ufs_hba *hba)
{
	const struct scsi_device *sdev;
	u32 pending = 0;

	lockdep_assert_held(hba->host->host_lock);
	__shost_for_each_device(sdev, hba->host)
		pending += sbitmap_weight(&sdev->budget_map);

	return pending;
}

/*
 * Wait until all pending SCSI commands and TMFs have finished or the timeout
 * has expired.
 *
 * Return: 0 upon success; -EBUSY upon timeout.
 */
static int xring_ufshcd_wait_for_doorbell_clr(struct ufs_hba *hba,
					u64 wait_timeout_us)
{
	unsigned long flags;
	int ret = 0;
	u32 tm_doorbell;
	u32 tr_pending;
	bool timeout = false, do_last_check = false;
	ktime_t start;

	ufshcd_hold(hba);
	spin_lock_irqsave(hba->host->host_lock, flags);
	/*
	 * Wait for all the outstanding tasks/transfer requests.
	 * Verify by checking the doorbell registers are clear.
	 */
	start = ktime_get();
	do {
		if (hba->ufshcd_state != UFSHCD_STATE_OPERATIONAL) {
			ret = -EBUSY;
			goto out;
		}

		tm_doorbell = ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL);
		tr_pending = xring_ufshcd_pending_cmds(hba);
		if (!tm_doorbell && !tr_pending) {
			timeout = false;
			break;
		} else if (do_last_check) {
			break;
		}

		spin_unlock_irqrestore(hba->host->host_lock, flags);
		io_schedule_timeout(msecs_to_jiffies(20));
		if (ktime_to_us(ktime_sub(ktime_get(), start)) >
		    wait_timeout_us) {
			dev_info(hba->dev, "%s: Failed to wait for doorbell clr, tr_pending = %u !\n",
				__func__, tr_pending);
			timeout = true;
			/*
			 * We might have scheduled out for long time so make
			 * sure to check if doorbells are cleared by this time
			 * or not.
			 */
			do_last_check = true;
		}
		spin_lock_irqsave(hba->host->host_lock, flags);
	} while (tm_doorbell || tr_pending);

	if (timeout) {
		dev_err(hba->dev,
			"%s: timedout waiting for doorbell to clear (tm=0x%x, tr=0x%x)\n",
			__func__, tm_doorbell, tr_pending);
		ret = -EBUSY;
	}
out:
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	ufshcd_release(hba);
	return ret;
}

static void xring_ufshcd_scsi_unblock_requests(struct ufs_hba *hba)
{
	if (atomic_dec_and_test(&hba->scsi_block_reqs_cnt))
		scsi_unblock_requests(hba->host);
}

static void xring_ufshcd_scsi_block_requests(struct ufs_hba *hba)
{
	if (atomic_inc_return(&hba->scsi_block_reqs_cnt) == 1)
		scsi_block_requests(hba->host);
}

/*
 * Wait until all pending SCSI commands and TMFs have finished or the timeout
 * has expired.
 *
 * Return: 0 upon success; -EBUSY upon timeout.
 */
static int ufshcd_power_mode_change_prepare(struct ufs_hba *hba, u64 timeout_us)
{
	int ret = 0;

	xring_ufshcd_scsi_block_requests(hba);

	if (xring_ufshcd_wait_for_doorbell_clr(hba, timeout_us)) {
		ret = -EBUSY;
		xring_ufshcd_scsi_unblock_requests(hba);
		goto out;
	}

	/* let's not get into low power until clock scaling is completed */
	ufshcd_hold(hba);

out:
	return ret;
}

static void ufshcd_power_mode_change_unprepare(struct ufs_hba *hba)
{
	xring_ufshcd_scsi_unblock_requests(hba);
	ufshcd_release(hba);
}

static int ufshcd_power_mode_change(struct ufs_hba *hba,
					struct ufs_pa_layer_attr *pwr_info, u32 scrambling)
{
	int ret = 0;
	u32 peer_scrambling = 0xFFFFFFFF;

	ret =  ufshcd_power_mode_change_prepare(hba, DOORBELL_CLR_TOUT_US);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to prepare for power mode change,ret= %d\n",
			__func__, ret);
		return ret;
	}

	down(&hba->host_sem);
	pm_runtime_get_sync(hba->dev);

	// Check whether the peer supports scrambling
	ret = ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_PEERSCRAMBLING),
				  &peer_scrambling);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to get peer scrambling status, ret=%d!\n",
			__func__, ret);
	}

	if (peer_scrambling == 0x1) {
		ret = ufshcd_dme_set(hba, UIC_ARG_MIB(PA_SCRAMBLING), scrambling);
		if (ret)
			dev_err(hba->dev, "%s: Failed to set scrambling!,ret= %d\n", __func__, ret);

		hba->force_pmc = 1;
	} else if (peer_scrambling == 0x0) {
		dev_info(hba->dev, "%s: Not set scrambling because peer don't support scrambling!\n",
			__func__);
	} else {
		dev_err(hba->dev, "%s: Unknown peer scrambling state, peer_scrambling=%u\n",
			__func__, peer_scrambling);
	}

	ret = ufshcd_config_pwr_mode(hba, pwr_info);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to switch power mode to target,ret= %d\n",
			__func__, ret);
	}
	pm_runtime_put_sync(hba->dev);
	up(&hba->host_sem);

	ufshcd_power_mode_change_unprepare(hba);
	return ret;
}

static int switch_to_pwm_g1(struct ufs_hba *hba,
			struct ufs_pa_layer_attr *pwr_info, u32 scrambling)
{
	int ret = 0;
	struct ufs_pa_layer_attr slow_mode_pwr_info = { 0 };

	slow_mode_pwr_info.gear_rx = UFS_PWM_G1;
	slow_mode_pwr_info.gear_tx = UFS_PWM_G1;
	slow_mode_pwr_info.lane_rx = pwr_info->lane_rx;
	slow_mode_pwr_info.lane_tx = pwr_info->lane_tx;
	slow_mode_pwr_info.pwr_rx = SLOW_MODE;
	slow_mode_pwr_info.pwr_tx = SLOW_MODE;

	ret = ufshcd_power_mode_change(hba, &slow_mode_pwr_info, scrambling);
	return ret;
}

static ssize_t power_mode_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	struct ufs_pa_layer_attr pwr_info = { 0 };
	u32 scrambling = 0;
	int ret = 0;

	ret = get_power_mode_attr(hba, buf, &pwr_info, &scrambling);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to get power mode attribute (ret = %d)\n",
			__func__, ret);
		return ret;
	}

	dev_info(hba->dev,
		"%s:[Rx,Tx]: Gear[%u,%u], Lane[%u,%u], PWR[%u,%u], Rate-%u, Scrambling-%u\n",
		__func__, pwr_info.gear_rx, pwr_info.gear_tx,
		pwr_info.lane_rx, pwr_info.lane_tx,
		pwr_info.pwr_rx, pwr_info.pwr_tx,
		pwr_info.hs_rate, scrambling);

	/*
	 * Switch between different rate types, we first cut back to the lower speed:
	 * Rate-A  --> Rate-B
	 * Rate-B  --> Rate-A
	 */
	if ((pwr_info.pwr_rx == FAST_MODE || pwr_info.pwr_rx == FASTAUTO_MODE ||
			pwr_info.pwr_tx == FAST_MODE || pwr_info.pwr_tx == FASTAUTO_MODE) &&
			pwr_info.hs_rate != hba->pwr_info.hs_rate) {
		hs_mode = pwr_info.hs_rate;
		work_mode = UFS_PWM_MODE;
		dev_info(hba->dev, "%s: Switch to different rate, need to switch slow mode:PWM_G1.\n",
			__func__);
		ret = switch_to_pwm_g1(hba, &pwr_info, scrambling);
		if (ret) {
			dev_err(hba->dev, "%s: Failed switch to SLOW_MODE (ret = %d)\n",
				__func__, ret);
			return ret;
		}
	}

	work_mode = UFS_HS_MODE;
	if (pwr_info.pwr_rx == SLOW_MODE || pwr_info.pwr_rx == SLOWAUTO_MODE ||
			pwr_info.pwr_tx == SLOW_MODE || pwr_info.pwr_tx == SLOWAUTO_MODE) {
		work_mode = UFS_PWM_MODE;
	}
	dev_info(hba->dev, "%s: Start switching to target power mode.\n", __func__);
	ret = ufshcd_power_mode_change(hba, &pwr_info, scrambling);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to change power mode (ret = %d)\n",
			__func__, ret);
	}

	return ret < 0 ? (ssize_t)ret : count;
}
static DEVICE_ATTR_RW(power_mode);
#else
static DEVICE_ATTR_RO(power_mode);
#endif

static ssize_t bootlu_sel_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int ret;
	uint value;
	struct ufs_hba *hba = dev_get_drvdata(dev);

	down(&hba->host_sem);
	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
		QUERY_ATTR_IDN_BOOT_LU_EN, 0, 0, &value);
	pm_runtime_put_sync(hba->dev);
	up(&hba->host_sem);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to read bootlu attr (ret = %d)\n", __func__, ret);
		goto out;
	}
	ret = sysfs_emit(buf, "%d\n", value);

out:
	return ret;
}

static ssize_t bootlu_sel_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	unsigned int bootlu_sel;
	ssize_t ret;

	if (kstrtouint(buf, 0, &bootlu_sel))
		return -EINVAL;

	if (bootlu_sel != 1 && bootlu_sel != 2)
		return -EINVAL;

	down(&hba->host_sem);
	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
		QUERY_ATTR_IDN_BOOT_LU_EN, 0, 0, &bootlu_sel);
	pm_runtime_put_sync(hba->dev);
	up(&hba->host_sem);
	return ret < 0 ? ret : count;
}
static DEVICE_ATTR_RW(bootlu_sel);

static ssize_t hibern8_stat_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	u32 stat;
	int ret;
	struct ufs_hba *hba = dev_get_drvdata(dev);
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	down(&hba->host_sem);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	ufshcd_hold(hba, false);
#else
	ufshcd_hold(hba);
#endif
	stat = ufs_sctrl_readl(host, UFS_SYS_CTRL_DWC_UFSHC_STAT);
	ufshcd_release(hba);
	up(&hba->host_sem);
	if (stat & BIT(5))
		ret = sysfs_emit(buf, "h8 state.\n");
	else
		ret = sysfs_emit(buf, "work state.\n");

	return ret;
}
static DEVICE_ATTR_RO(hibern8_stat);

static ssize_t device_temp_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	uint value = 0;
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (hba->caps & UFSHCD_CAP_TEMP_NOTIF) {
		down(&hba->host_sem);
		pm_runtime_get_sync(hba->dev);
		ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
			QUERY_ATTR_IDN_CASE_ROUGH_TEMP, 0, 0, &value);
		pm_runtime_put_sync(hba->dev);
		up(&hba->host_sem);
		if (ret) {
			dev_err(hba->dev, "%s: Failed to read temp attr (ret = %d)\n", __func__, ret);
			goto out;
		}
	}
	ret = sysfs_emit(buf, "%ld\n", ((long)value - 80) * 1000);

out:
	return ret;
}
static DEVICE_ATTR_RO(device_temp);

static ssize_t throtting_stat_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	uint value;
	struct ufs_hba *hba = dev_get_drvdata(dev);

	down(&hba->host_sem);
	pm_runtime_get_sync(hba->dev);
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
		QUERY_ATTR_IDN_THROTTING_STATUS, 0, 0, &value);
	pm_runtime_put_sync(hba->dev);
	up(&hba->host_sem);
	if (ret) {
		dev_err(hba->dev, "%s: Failed to read throtting_stat attr (ret = %d)\n", __func__, ret);
		goto out;
	}

	ret = sysfs_emit(buf, "%d\n", value);

out:
	return ret;
}
static DEVICE_ATTR_RO(throtting_stat);


static struct attribute *ufs_xring_sysfs_attrs[] = {
	&dev_attr_power_mode.attr,
	&dev_attr_bootlu_sel.attr,
	&dev_attr_hibern8_stat.attr,
	&dev_attr_device_temp.attr,
	&dev_attr_throtting_stat.attr,
	NULL
};

static const struct attribute_group ufs_xring_sysfs_group = {
	.name = "xring-ufs",
	.attrs = ufs_xring_sysfs_attrs,
};

static int ufs_xring_init_sysfs(struct ufs_hba *hba)
{
	int ret;

	ret = sysfs_create_group(&hba->dev->kobj, &ufs_xring_sysfs_group);
	if (ret)
		dev_err(hba->dev, "%s: Failed to create xring sysfs group (ret = %d)\n",
				__func__, ret);

	return ret;
}


static void ufs_xring_fixup_dev_quirks(struct ufs_hba *hba)
{
	ufshcd_fixup_dev_quirks(hba, ufs_xring_dev_fixups);
}

static int ufs_xring_apply_dev_quirks(struct ufs_hba *hba)
{
	unsigned long flags;
	int ret;

	if (is_mcq_enabled(hba)) {
		ufs_xring_irq_set_affinity(hba, NONE_CPU_ID);
		ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
				"ufs_xring:online", ufs_xring_cpu_online, NULL);
		if (ret < 0)
			dev_err(hba->dev, "%s: Failed to set cpuhp hook (ret = %d)\n",
				__func__, ret);
	}

	spin_lock_irqsave(hba->host->host_lock, flags);
	/* Set the default auto-hiberate idle timer value to 5ms */
	hba->ahit = FIELD_PREP(UFSHCI_AHIBERN8_TIMER_MASK, 5) |
		FIELD_PREP(UFSHCI_AHIBERN8_SCALE_MASK, 3);
	spin_unlock_irqrestore(hba->host->host_lock, flags);

	return 0;
}

/**
 * ufs_xring_advertise_quirks - advertise the known XRING UFS controller quirks
 * @hba: host controller instance
 *
 * XRING UFS host controller might have some non standard behaviours (quirks)
 * than what is specified by UFSHCI specification. Advertise all such
 * quirks to standard UFS host controller driver so standard takes them into
 * account.
 */
static void ufs_xring_advertise_quirks(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* Legacy UniPro mode still need following quirks */
	hba->quirks |= UFSHCD_QUIRK_DELAY_BEFORE_DME_CMDS;

	if (host->is_emu)
		hba->quirks &= ~UFSHCD_QUIRK_DELAY_BEFORE_DME_CMDS;

	/* For external PHY in FPGA_validation_platform, Do not use the AH8 function for now */
	if (host->is_fpga)
		hba->quirks |= UFSHCD_QUIRK_BROKEN_AUTO_HIBERN8;

if (IS_ENABLED(CONFIG_XRING_UFS_CRYPTO)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	hba->quirks |= UFSHCD_QUIRK_CUSTOM_KEYSLOT_MANAGER;
#else
	hba->android_quirks |= UFSHCD_ANDROID_QUIRK_CUSTOM_CRYPTO_PROFILE;
#endif
}

	dev_dbg(hba->dev, "%s,%d: hba->quirks = 0x%x\n", __func__, __LINE__, hba->quirks);
}

/**
 * ufs_xring_set_caps() - set UFS host cap.
 * @hba: per-adapter instance
 *
 */
static void ufs_xring_set_caps(struct ufs_hba *hba)
{
	hba->caps |= UFSHCD_CAP_AUTO_BKOPS_SUSPEND;
	/* Enable WriteBooster */
	hba->caps |= UFSHCD_CAP_WB_EN;

	/* Enable temperature notify */
	hba->caps |= UFSHCD_CAP_TEMP_NOTIF;

	/* enable ufs crypt */
	if (IS_ENABLED(CONFIG_XRING_UFS_CRYPTO))
		hba->caps |= UFSHCD_CAP_CRYPTO;
}

static int xring_ufs_parse_clk(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct device *dev = hba->dev;
	struct ufs_xring_clk *clk = host->clk;

	clk->clk_ufs_core = devm_clk_get(dev, "clk_ufs_core");
	if (IS_ERR(clk->clk_ufs_core)) {
		dev_err(dev, "failed to parse clk clk_ufs_core\n");
		return PTR_ERR(clk->clk_ufs_core);
	}

	clk->clk_ufs_mphy_cfg = devm_clk_get(dev, "clk_ufs_mphy_cfg");
	if (IS_ERR(clk->clk_ufs_mphy_cfg)) {
		dev_err(dev, "failed to parse clk clk_ufs_mphy_cfg\n");
		return PTR_ERR(clk->clk_ufs_mphy_cfg);
	}

	clk->pclk_ufs_apb = devm_clk_get(dev, "pclk_ufs_apb");
	if (IS_ERR(clk->pclk_ufs_apb)) {
		dev_err(dev, "failed to parse clk pclk_ufs_apb\n");
		return PTR_ERR(clk->pclk_ufs_apb);
	}

	clk->aclk_ufsctrl_abrg_slv = devm_clk_get(dev, "aclk_ufsctrl_abrg_slv");
	if (IS_ERR(clk->aclk_ufsctrl_abrg_slv)) {
		dev_err(dev, "failed to parse clk aclk_ufsctrl_abrg_slv\n");
		return PTR_ERR(clk->aclk_ufsctrl_abrg_slv);
	}

	clk->aclk_ufsesi_abrg_slv = devm_clk_get(dev, "aclk_ufsesi_abrg_slv");
	if (IS_ERR(clk->aclk_ufsesi_abrg_slv)) {
		dev_err(dev, "failed to parse clk aclk_ufsesi_abrg_slv\n");
		return PTR_ERR(clk->aclk_ufsesi_abrg_slv);
	}

	clk->aclk_ufsctrl_tpc = devm_clk_get(dev, "aclk_ufsctrl_tpc");
	if (IS_ERR(clk->aclk_ufsctrl_tpc)) {
		dev_err(dev, "failed to parse clk aclk_ufsctrl_tpc\n");
		return PTR_ERR(clk->aclk_ufsctrl_tpc);
	}

	return 0;
}

static void set_ufs_device_ref_clk(struct ufs_hba *hba, u32 clk)
{
	switch (clk) {
	case REF_CLK_FREQ_19_2_MHZ:
	case REF_CLK_FREQ_26_MHZ:
	case REF_CLK_FREQ_38_4_MHZ:
	case REF_CLK_FREQ_52_MHZ:
		hba->dev_ref_clk_freq = clk;
		break;
	default:
		dev_err(hba->dev, "unsupport ufs device ref clk\n");
		break;
	}

	return;
}

/**
 * ufs_xring_init - bind phy with controller
 * @hba: host controller instance
 *
 * Binds PHY with controller and powers up PHY enabling clocks
 * and regulators.
 *
 * Returns -EPROBE_DEFER if binding fails, returns negative error
 * on phy power up failure and returns zero on success.
 */
static int ufs_xring_init(struct ufs_hba *hba)
{
	int ret;
	struct resource *res;
	struct ufs_xring_host *host;
	struct device *dev = hba->dev;
	struct platform_device *pdev = to_platform_device(dev);
	struct device_node *np = dev->of_node;
	struct ufs_xring_clk *clk;
	u32 dev_ref_clk;

	host = devm_kzalloc(dev, sizeof(*host), GFP_KERNEL);
	if (IS_ERR(host)) {
		dev_err(dev, "%s: no memory for xring ufs host\n", __func__);
		return PTR_ERR(host);
	}

	clk = devm_kzalloc(dev, sizeof(*clk), GFP_KERNEL);
	if (IS_ERR(clk)) {
		dev_err(dev, "%s: no memory for xring ufs clk\n", __func__);
		return PTR_ERR(clk);
	}
	host->clk = clk;

	/* Make a two way bind between the xring host and the hba */
	host->hba = hba;
	s_ufs_xring_host = host;
	ufshcd_set_variant(hba, host);

	/* esi irq status init */
	host->esi_is_requested = false;

	/* Setup the UFS_SCTRL ioremap*/
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ufs_sctrl_mem");
	if (IS_ERR(res)) {
		ret = PTR_ERR(res);
		dev_warn(dev, "UFS_SCTRL registers not found\n");
		goto out_variant_clear;
	}

	host->ufs_sctrl_mmio = devm_ioremap_resource(dev, res);
	if (IS_ERR(host->ufs_sctrl_mmio)) {
		ret = PTR_ERR(host->ufs_sctrl_mmio);
		dev_err(dev, "Failed to map UFS_SCTRL registers; ret=%d\n", ret);
		goto out_variant_clear;
	}

	/* Setup the HSS1_CRG ioremap*/
	host->hss1_regmap = syscon_regmap_lookup_by_phandle(dev->of_node,
				"hss1-crg-syscon");
	if (IS_ERR(host->hss1_regmap)) {
		dev_err(dev, "unable to find hsscrg syscon registers\n");
		ret = PTR_ERR(host->hss1_regmap);
		goto out_variant_clear;
	}

	/* Setup the PERI_CRG ioremap*/
	host->peri_crg_regmap = syscon_regmap_lookup_by_phandle(dev->of_node,
				"peri-crg-syscon");
	if (IS_ERR(host->peri_crg_regmap)) {
		dev_err(dev, "unable to find peri crg syscon registers\n");
		ret = PTR_ERR(host->peri_crg_regmap);
		goto out_variant_clear;
	}

	/* Setup ufs device ref_clk */
	ret = of_property_read_u32(dev->of_node, "ufs-dev-ref-clk", &dev_ref_clk);
	if (ret) {
		dev_err(dev, "get ufs ref clk fail\n");
		goto out_variant_clear;
	}
	set_ufs_device_ref_clk(hba, dev_ref_clk);

	host->is_mphy_fw_sram_init = of_property_read_bool(np, "xring,ufs-mphy-fw-sram-init");

	host->is_fpga = of_property_read_bool(np, "is_fpga");
	host->is_emu = of_property_read_bool(np, "is_emu");
	dev_info(hba->dev, "host->is_fpga:%s host->is_emu:%s dev_ref_clk=0x%x\n",
			host->is_fpga ? "yes" : "no", host->is_emu ? "yes" : "no",
			hba->dev_ref_clk_freq);

	/* Setup the lms_crg ioremap*/
	host->lms_crg_regmap = syscon_regmap_lookup_by_phandle(dev->of_node,
				"lms-crg-syscon");
	if (IS_ERR(host->lms_crg_regmap)) {
		dev_err(dev, "unable to find lms_crg syscon registers\n");
		ret = PTR_ERR(host->lms_crg_regmap);
		return ret;
	}

	/* Setup the lms_crg ioremap*/
	host->lpis_actrl_regmap = syscon_regmap_lookup_by_phandle(dev->of_node,
				"lpis-actrl-syscon");
	if (IS_ERR(host->lpis_actrl_regmap)) {
		dev_err(dev, "unable to find lpis_actrl syscon registers\n");
		ret = PTR_ERR(host->lpis_actrl_regmap);
		return ret;
	}

	/* setup the lms sys_ctrl ioremap */
	host->sys_ctrl_regmap = syscon_regmap_lookup_by_phandle(dev->of_node,
				"lms-sysctrl-syscon");
	if (IS_ERR(host->sys_ctrl_regmap)) {
		dev_err(dev, "unable to find host->sys_ctrl_regmap syscon registers\n");
		ret = PTR_ERR(host->sys_ctrl_regmap);
		return ret;
	}

	if (host->is_fpga) {
		ret = ufs_xring_fpga_ioremap(hba);
		if (ret)
			goto out_variant_clear;
	}

	if (host->is_fpga)
		goto skip_rsv_mem;

	ret = ufs_get_reserved_memory(hba);
	if (ret) {
		dev_err(hba->dev, "%s host get reserved mm fail not write calibration\n", __func__);
		return ret;
	}

skip_rsv_mem:
	ret = xring_ufs_parse_clk(hba);
	if (ret)
		goto out_variant_clear;

	/* match clk hardware status */
	ret = ufs_xring_open_hss1_clk(hba);
	if (ret) {
		dev_err(hba->dev, "%s match clk hw status fail:%d\n", __func__, ret);
		return ret;
	}

	/*
	 * ufs subsystem reset
	 */
	ret = ufs_xring_subsys_reset(hba);
	if (ret) {
		dev_err(hba->dev, "ufs subsystem reset fail:%d\n", ret);
		goto out_variant_clear;
	}

	ret = ufs_xring_program_div_clk(hba);
	if (ret) {
		dev_err(dev, "%s: div clk fail\n", __func__);
		goto out_variant_clear;
	}

	if (host->is_fpga) {
		/* Register TAG version-dependent subfunctions */
		ufs_xring_mphy_clk_cfg_fpga(hba);
	} else {
		ufs_xring_mphy_clk_cfg(hba);
	}

	ufs_xring_mphy_fw_init(hba);
	ret = ufs_host_controller_init(hba);
	if (ret) {
		dev_err(hba->dev, "%s : %d, ufs_host_controller_init fail, ret is = %d.\n",
			__func__, __LINE__, ret);
		goto out_variant_clear;
	}

	/*
	 * for newer controllers, device reference clock control bit has
	 * moved inside UFS controller register address space itself.
	 */
	host->dev_ref_clk_ctrl_mmio = NULL;

	ufs_xring_set_caps(hba);
	ufs_xring_advertise_quirks(hba);

	ufs_xring_setup_clocks(hba, true, POST_CHANGE);

	/* init crypto capbilities for wrapped key */
	ret = ufshcd_xring_hba_init_crypto_capabilities(hba);
	if (ret) {
		dev_err(hba->dev, "Ufs init crypto capblities fail,err=%d\n", ret);
		goto out_variant_clear;
	}

	host->mphy_pro_calibration_status = MPHY_PRO_UNCALIBRATION;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	ufs_xring_mcq_alloc_priv(hba);
	ufs_xring_mcq_get_dts_info(hba);
	ufs_xring_mcq_install_tracepoints(hba);
#endif
	ufs_xring_init_sysfs(hba);

	if (ufs_xring_dbg_register(hba))
		dev_warn(hba->dev, "ufs register tracepoint func fail\n");

	dev_info(dev, "HC capabilities = 0x%x hba->caps = 0x%x\n", ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES), hba->caps);
out_variant_clear:
	if (ret) {
		ufshcd_set_variant(hba, NULL);
		s_ufs_xring_host = NULL;
	}

	return ret;
}

static void __iomem *mcq_opr_base(struct ufs_hba *hba,
					 enum ufshcd_mcq_opr n, int i)
{
	struct ufshcd_mcq_opr_info_t *opr = &hba->mcq_opr[n];

	return opr->base + opr->stride * i;
}

int ufs_xring_mcq_dump_regs(struct ufs_hba *hba, size_t offset, size_t len,
		     const char *prefix)
{
	u32 *regs;
	size_t pos;

	if (offset % 4 != 0 || len % 4 != 0) /* keep readl happy */
		return -EINVAL;
	regs = kzalloc(len, GFP_ATOMIC);
	if (!regs)
		return -ENOMEM;
	for (pos = 0; pos < len; pos += 4) {
		if (offset == 0 &&
		    pos >= REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER &&
		    pos <= REG_UIC_ERROR_CODE_DME)
			continue;
		regs[pos / 4] = ufsmcq_readl(hba, offset + pos);
	}
	ufs_xring_hex_dump(prefix, regs, len);
	kfree(regs);
	return 0;
}

static void ufs_xring_mcq_dump_register(struct ufs_hba *hba)
{
	u32 i;
	u32 offset;
	/* dump mcq sq/cq registers */
	for (i = 0; i < hba->nr_hw_queues; i++) {
		offset = MCQ_QCFG_SIZE * i;
		dev_err(hba->dev, "Dump mcq related register, queue id: %d, offset: 0x%x\n",
						i, offset);
		ufs_xring_mcq_dump_regs(hba, offset, MCQ_QCFG_SIZE, "mcq_related_regs: ");
	}
	/* dump mcq operation&runtime registers */
	for (i = 0; i < hba->nr_hw_queues; i++) {
		dev_err(hba->dev, "Dump mcq opr register, queue id: %d\n", i);
		ufs_xring_hex_dump("OPR_SQD: ", mcq_opr_base(hba, OPR_SQD, i), 0x14);
		ufs_xring_hex_dump("OPR_SQIS: ", mcq_opr_base(hba, OPR_SQIS, i), 0x14);
		ufs_xring_hex_dump("OPR_CQD: ", mcq_opr_base(hba, OPR_CQD, i), 0x14);
		ufs_xring_hex_dump("OPR_CQIS: ", mcq_opr_base(hba, OPR_CQIS, i), 0x14);
	}
}

/**
 * ufs_xring_dump_dbg_regs() - dump debug reg
 * @hba: per-adapter instance
 *
 */

static void ufs_xring_dump_dbg_regs(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	u32 gpll_val;

	ufs_sysctrl_dump_regs(hba, UFS_SYS_CTRL_UFS_SYS_RST_SWGT_W1S, UFS_SYS_CTRL_UFS_FPGA,
		"UFS SysCtrl, ");

	/* read UFS debug_fsm */
	ufs_sctrl_rmwl(host, 0xFF, 0xFF, 0xA8);
	ufs_sysctrl_dump_regs(hba, 0xA8, 0x30, "UFS SysCtrl Debug_FSM, ");
	ufs_sctrl_rmwl(host, 0xFF, 0x00, 0xA8);

	/* GPLL */
	if (peri_crg_readl(host, PERI_CRG_PLL_LOCK_STATE, &gpll_val) < 0) {
		dev_err(hba->dev, "read gpll lock state fail\n");
		return;
	}

	dev_err(hba->dev, "PERI_CRG_PLL_LOCK_STATE:0x%08x bit[0]=0x%x\n", gpll_val,
		gpll_val & PERI_CRG_PLL_LOCK_STATE_GPLL_LOCK_MASK);

	/* hss1 crg offset:0x8 */
	dev_err(hba->dev, "HSS1_CRG_CLKGT0:0x%08x bit[2]=0x%x, bit[3]=0x%x,bit[5]=0x%x,\n",
			hss1_crg_readl(host, HSS1_CRG_CLKGT0_RO),
		(hss1_crg_readl(host, HSS1_CRG_CLKGT0_RO) & HSS1_CRG_CLKGT0_RO_GT_CLK_UFS_MPHY_CFG_MASK)
			>> HSS1_CRG_CLKGT0_RO_GT_CLK_UFS_MPHY_CFG_SHIFT,
		(hss1_crg_readl(host, HSS1_CRG_CLKGT0_RO) & HSS1_CRG_CLKGT0_RO_GT_PCLK_UFS_APB_MASK)
			>> HSS1_CRG_CLKGT0_RO_GT_PCLK_UFS_APB_SHIFT,
		(hss1_crg_readl(host, HSS1_CRG_CLKGT0_RO) & HSS1_CRG_CLKGT0_RO_GT_CLK_UFS_CORE_MASK)
			>> HSS1_CRG_CLKGT0_RO_GT_CLK_UFS_CORE_SHIFT);

	/* hss1 crg offset:0x40~0x48 */
	dev_err(hba->dev, "HSS1_CRG_CLKDIV0:0x%08x bit[5:0]=0x%x\n",
			hss1_crg_readl(host, HSS1_CRG_CLKDIV0),
			hss1_crg_readl(host, HSS1_CRG_CLKDIV0) & HSS1_CRG_CLKDIV0_DIV_CLK_HSS1_BUS_CFG_MASK);

	dev_err(hba->dev, "HSS1_CRG_CLKDIV1:0x%08x bit[13]=0x%x, bit[11:6]=0x%x\n",
			hss1_crg_readl(host, HSS1_CRG_CLKDIV1),
		(hss1_crg_readl(host, HSS1_CRG_CLKDIV1) & HSS1_CRG_CLKDIV1_SC_GT_CLK_UFS_CORE_MASK)
			>> HSS1_CRG_CLKDIV1_SC_GT_CLK_UFS_CORE_SHIFT,
		(hss1_crg_readl(host, HSS1_CRG_CLKDIV1) & HSS1_CRG_CLKDIV1_DIV_CLK_UFS_CORE_MASK)
			>> HSS1_CRG_CLKDIV1_DIV_CLK_UFS_CORE_SHIFT);

	dev_err(hba->dev, "HSS1_CRG_CLKDIV2:0x%08x bit[12]=0x%x bit[5:0]=0x%x\n",
			hss1_crg_readl(host, HSS1_CRG_CLKDIV2),
		(hss1_crg_readl(host, HSS1_CRG_CLKDIV2) & HSS1_CRG_CLKDIV2_SC_GT_CLK_UFS_MPHY_CFG_MASK)
			>> HSS1_CRG_CLKDIV2_SC_GT_CLK_UFS_MPHY_CFG_SHIFT,
		hss1_crg_readl(host, HSS1_CRG_CLKDIV2) & HSS1_CRG_CLKDIV2_DIV_CLK_UFS_MPHY_CFG_MASK);

	/* ufs hc vendor special reg */
	dev_err(hba->dev, "UFSHC Vendor special BUSTHRTL:0x%08x bit[12]=0x%lx\n",
		ufshcd_readl(hba, BUSTHRTL),
		(ufshcd_readl(hba, BUSTHRTL) & BUSTHRTL_CGE_MASK) >> BUSTHRTL_CGE_SHIFT);

	/* hss1 crg offset:0x28 */
	dev_err(hba->dev, "HSS1_CRG_RST0:0x%08x bit[0]=0x%x bit[7]=0x%x bit[20]=0x%x\n",
			hss1_crg_readl(host, HSS1_CRG_RST0_RO),
		hss1_crg_readl(host, HSS1_CRG_RST0_RO) & HSS1_CRG_RST0_RO_IP_RST_UFS_SYS_N_MASK,
		(hss1_crg_readl(host, HSS1_CRG_RST0_RO) & HSS1_CRG_RST0_RO_IP_PRST_UFS_APB_N_MASK)
			>> HSS1_CRG_RST0_RO_IP_PRST_UFS_APB_N_SHIFT,
		(hss1_crg_readl(host, HSS1_CRG_RST0_RO) & HSS1_CRG_RST0_RO_IP_RST_UFSCTRL_TPC_N_MASK)
			>> HSS1_CRG_RST0_RO_IP_RST_UFSCTRL_TPC_N_SHIFT);

	ufs_xring_mcq_dump_register(hba);

#if IS_ENABLED(CONFIG_XRING_CPU_TEST_LPI)
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		dev_err(hba->dev, "gic cpu %d lpi is %s\n", cpu,
					lpi_is_enabled(cpu) ? "enabled" : "disabled");
	}
#endif
}

static void ufs_xring_hibern8_notify(struct ufs_hba *hba, enum uic_cmd_dme uic_cmd,
					enum ufs_notify_change_status status)
{
	if (status == PRE_CHANGE) {
		if (uic_cmd == UIC_CMD_DME_HIBER_ENTER) {
			if (!ufshcd_is_auto_hibern8_supported(hba))
				return;
			/* close AH8 */
			ufshcd_rmwl(hba, UFSHCI_AHIBERN8_TIMER_MASK, 0x0,
						REG_AUTO_HIBERNATE_IDLE_TIMER);
		}
	}
}

static int ufs_xring_link_startup_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	int ret;
	u32 pa_tactivate;

	switch (status) {
	case PRE_CHANGE:
		break;
	case POST_CHANGE:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
		ret = ufs_xring_register_esi(hba);
		if (ret)
			ufs_xring_mcq_enable_intr(hba, UFSHCD_ENABLE_MCQ_INTRS);
		else
			ufs_xring_mcq_enable_intr(hba, UFSHCD_ENABLE_MCQ_INTRS_ESI);
		dev_info(hba->dev, "ESI %sconfigured\n", ret ? "is not " : "");
#endif
		ret = ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TACTIVATE), &pa_tactivate);
		if (!ret) {
			ret = ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TACTIVATE), pa_tactivate + 1);
			if (ret)
				dev_err(hba->dev, "%s: Failed to set PA_TACTIVATE, ret= %d\n",
					__func__, ret);
			else
				dev_info(hba->dev, "%s: PA_TACTIVATE=%d\n", __func__, pa_tactivate + 1);
		} else {
			dev_err(hba->dev, "%s: Failed to get PA_TACTIVATE, ret= %d\n",
				__func__, ret);
		}

		break;
	default:
		break;
	}

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
/* Resources */
static const struct ufshcd_res_info ufs_res_info[RES_MAX] = {
	{.name = "ufs_ctrl_mem",},
	{.name = "mcq",},
	/* Submission Queue DAO */
	{.name = "mcq_sqd",},
	/* Submission Queue Interrupt Status */
	{.name = "mcq_sqis",},
	/* Completion Queue DAO */
	{.name = "mcq_cqd",},
	/* Completion Queue Interrupt Status */
	{.name = "mcq_cqis",},
	/* MCQ vendor specific */
	{.name = "mcq_vs",},
};

static int ufs_xring_get_hba_mac(struct ufs_hba *hba)
{
	struct device_node *np = hba->dev->of_node;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	struct ufs_dev_params dev_param;

	ufs_xring_init_pwr_dev_param(host, &dev_param);

	if (of_property_read_u32(np, "xring,ufs-mcq-q-depth",
		&host->mcq_queue_depth))
		host->mcq_queue_depth = 32;

	/* Micron UFS device limit: only HS_G5 support QD64, otherwise QD is 32 */
	if ((hba->dev_info.wmanufacturerid == UFS_VENDOR_MICRON) &&
		((dev_param.hs_rx_gear < UFS_HS_G5) ||
		(dev_param.hs_tx_gear < UFS_HS_G5)))
		host->mcq_queue_depth = 32;

	return host->mcq_queue_depth;
}

static int ufs_xring_op_runtime_config(struct ufs_hba *hba)
{
	struct ufshcd_res_info *mem_res, *sqdao_res;
	struct ufshcd_mcq_opr_info_t *opr;
	int i;

	mem_res = &hba->res[RES_UFS];
	sqdao_res = &hba->res[RES_MCQ_SQD];
	if (!mem_res->base || !sqdao_res->base)
		return -EINVAL;

	/* per type size: 0x40, per queue size: 0x100 */
	for (i = 0; i < OPR_MAX; i++) {
		opr = &hba->mcq_opr[i];
		opr->offset = sqdao_res->resource->start -
			      mem_res->resource->start + 0x40 * i;
		opr->stride = 0x100;
		opr->base = sqdao_res->base + 0x40 * i;
	}
	return 0;
}

static void ufs_xring_mcq_get_dts_info(struct ufs_hba *hba)
{
	struct device_node *np = hba->dev->of_node;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	host->esi_supported = false;
	if (of_property_read_bool(np, "xring,ufs-esi-enabled"))
		host->esi_supported = true;
	else
		host->esi_supported = false;

	if (of_property_read_u32(np, "xring,ufs-mcq-nr-hwq",
		&hba->nr_hw_queues))
		return;
}

static int ufs_xring_mcq_config_resource(struct ufs_hba *hba)
{
	struct platform_device *pdev = to_platform_device(hba->dev);
	struct ufshcd_res_info *res;
	struct resource *res_mem, *res_mcq;
	int i, ret;

	ufs_xring_mcq_get_dts_info(hba);
	memcpy(hba->res, ufs_res_info, sizeof(ufs_res_info));

	for (i = 0; i < RES_MAX; i++) {
		res = &hba->res[i];
		res->resource = platform_get_resource_byname(pdev,
								IORESOURCE_MEM,
								res->name);
		if (!res->resource) {
			dev_info(hba->dev, "Resource %s not provided\n",
								res->name);
			if (i == RES_UFS)
				return -ENOMEM;
			continue;
		} else if (i == RES_UFS) {
			res_mem = res->resource;
			res->base = hba->mmio_base;
			continue;
		}
		res->base = devm_ioremap_resource(hba->dev, res->resource);
		if (IS_ERR(res->base)) {
			dev_err(hba->dev, "Failed to map res %s, err=%d\n",
					 res->name, (int)PTR_ERR(res->base));
			ret = PTR_ERR(res->base);
			res->base = NULL;
			return ret;
		}
	}
	/* MCQ resource provided in DT */
	res = &hba->res[RES_MCQ];
	/* Bail if MCQ resource is provided */
	if (res->base)
		goto out;
	/* Explicitly allocate MCQ resource from ufs_mem */
	res_mcq = devm_kzalloc(hba->dev, sizeof(*res_mcq), GFP_KERNEL);
	if (!res_mcq)
		return -ENOMEM;
	res_mcq->start = res_mem->start + FIELD_GET(MCQ_QCFGPTR_MASK,
					hba->mcq_capabilities) * MCQ_QCFGPTR_UNIT;
	res_mcq->end = res_mcq->start + hba->nr_hw_queues *
						MCQ_QCFG_SIZE - 1;
	res_mcq->flags = res_mem->flags;
	res_mcq->name = "mcq";
	ret = insert_resource(&iomem_resource, res_mcq);
	if (ret) {
		dev_err(hba->dev, "Failed to insert MCQ resource, err=%d\n",
			ret);
		return ret;
	}
	res->base = devm_ioremap_resource(hba->dev, res_mcq);
	if (IS_ERR(res->base)) {
		dev_err(hba->dev, "MCQ registers mapping failed, err=%d\n",
			(int)PTR_ERR(res->base));
		ret = PTR_ERR(res->base);
		goto ioremap_err;
	}
out:
	hba->mcq_base = res->base;
	return 0;
ioremap_err:
	res->base = NULL;
	remove_resource(res_mcq);
	return ret;
}

static void ufs_xring_irq_set_affinity(struct ufs_hba *hba, unsigned int cpuid)
{
	u32 cpu, irq, _irq, queue_id, queue_map;
	int ret, i;
	struct blk_mq_tag_set *tag_set = &hba->host->tag_set;
	struct blk_mq_queue_map *qmap;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	u32 target_cpu;

	if (!host->esi_supported)
		return;

	for (queue_map = 0; queue_map < HCTX_MAX_TYPES; queue_map++) {
		qmap = &tag_set->map[queue_map];
		if (queue_map == HCTX_TYPE_POLL)
			continue;
		if (!qmap->mq_map || !qmap->nr_queues)
			continue;
		for_each_possible_cpu(cpu) {
			queue_id = qmap->mq_map[cpu];
			irq = queue_id * ESI_IRQ_TYPE + host->esi_base;
			if (cpu == 0)
				target_cpu = 1;
			else
				target_cpu = cpu;
			if (cpuid != NONE_CPU_ID && cpuid != target_cpu)
				continue;

			for (i = 0; i < ESI_IRQ_TYPE; i++) {
				_irq = irq + i;
				ret = irq_set_affinity(_irq, cpumask_of(target_cpu));
				if (ret)
					dev_err(hba->dev, "ufs: irq_set_affinity irq %d on CPU %d failed\n",
						_irq, target_cpu);
			}
		}
	}
}

static int ufs_xring_cpu_online(unsigned int cpu)
{
	if (s_ufs_xring_host)
		ufs_xring_irq_set_affinity(s_ufs_xring_host->hba, cpu);

	return 0;
}

static void ufs_xring_enable_esi(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	unsigned int ufs_esi_reg[3];
	unsigned int ufs_esi_mask[4];
	int i;

	ufs_esi_reg[0] = UFS_ESI_MASK_2_OFFSET;
	ufs_esi_reg[1] = UFS_ESI_MASK_1_OFFSET;
	ufs_esi_reg[2] = UFS_ESI_MASK_0_OFFSET;
	ufs_esi_mask[0] = 0xffffffff;
	ufs_esi_mask[1] = 0xffffffff;
	ufs_esi_mask[2] = 0xffffffff;
	ufs_sctrl_rmwl(host, UFS_ESI_DEVICE_ID_MASK, (0xff << 16),
				UFS_ESI_DEVICE_ID_OFFSET);

	for (i = 0; i < ESI_NR_IRQS; i++) {
		if (i % ESI_IRQ_TYPE == CQ_TEPS_OFFSET)
			__clear_bit(i % 32, (unsigned long *)(&(ufs_esi_mask[i / 32])));
	}
	for (i = 0; i < 3; i++)
		ufs_sctrl_writel(host, ufs_esi_mask[i], ufs_esi_reg[i]);

	ufshcd_writel(hba, ufshcd_readl(hba, REG_UFS_MEM_CFG) | 0x2,
		      REG_UFS_MEM_CFG);
}

static unsigned int ufs_xring_irq_to_queue_id(unsigned int base,
						unsigned int irq)
{
	return ((irq - base) / ESI_IRQ_TYPE);
}

static irqreturn_t ufs_xring_esi_handler(int irq, void *__hba)
{
	struct ufs_hba *hba = __hba;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
	u32 id = ufs_xring_irq_to_queue_id(host->esi_base, irq);
	struct ufs_hw_queue *hwq = &hba->uhq[id];

	ufshcd_mcq_write_cqis(hba, 0x1, id);
	ufshcd_mcq_poll_cqe_lock(hba, hwq);
	return IRQ_HANDLED;
}

static void ufs_xring_write_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	/* hw config, sw do nothing here */
	return;
}

static int ufs_xring_request_irq(struct ufs_hba *hba)
{
	struct msi_desc *desc = NULL;
	struct msi_desc *failed_desc = NULL;
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	ret = platform_msi_domain_alloc_irqs(hba->dev, ESI_NR_IRQS,
					     ufs_xring_write_msi_msg);
	if (ret) {
		pr_err("%s alloc esi irq fail ret=%d\n", __func__, ret);
		host->esi_supported = 0;
		return ret;
	}

	ufs_xring_msi_lock_descs(hba);
	msi_for_each_desc(desc, hba->dev, MSI_DESC_ALL) {
		if (!desc->msi_index)
			host->esi_base = desc->irq;
		ret = devm_request_irq(hba->dev, desc->irq,
						ufs_xring_esi_handler,
						IRQF_SHARED, "xring-mcq-esi", hba);
		if (ret) {
			dev_err(hba->dev, "%s: Fail to request IRQ for %d, err = %d\n",
				__func__, desc->irq, ret);
			failed_desc = desc;
			host->esi_supported = 0;
			break;
		}
	}
	ufs_xring_msi_unlock_descs(hba);
	if (ret) {
		ufs_xring_msi_lock_descs(hba);
		msi_for_each_desc(desc, hba->dev, MSI_DESC_ALL) {
			if (desc == failed_desc)
				break;
			devm_free_irq(hba->dev, desc->irq, hba);
		}
		ufs_xring_msi_unlock_descs(hba);
		host->esi_supported = false;
		host->esi_is_requested = false;
		dev_warn(hba->dev, "Failed to request Platform MSI %d\n", ret);
	} else {
		host->esi_is_requested = true;
	}

	return ret;
}

static int ufs_xring_config_esi(struct ufs_hba *hba)
{
	int ret = 0;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (!host->esi_supported) {
		dev_notice(hba->dev, "%s: ESI mode is not enabled\n", __func__);
		return -EOPNOTSUPP;
	}

	if (!host->esi_is_requested) {
		ret = ufs_xring_request_irq(hba);
		if (ret)
			return ret;
	}

	ufs_xring_enable_esi(hba);

	return ret;
}
#endif

u32 lpis_actrl_readl(struct ufs_xring_host *host, u32 reg)
{
	u32 val;
	int ret;
	struct ufs_hba *hba = ufs_host_to_ufs_hba((void *)host);
	struct device *dev = hba->dev;

	ret = regmap_read(host->lpis_actrl_regmap, reg, &val);
	if (ret < 0)
		dev_err_once(dev, "failed to syscon read address:0x%x\n", reg);

	return val;
}
/**
 * ufs_xring_wait_for_lpis_actrl_register - wait for register value to change
 * @hba: per-adapter interface
 * @reg: mmio register offset
 * @mask: mask to apply to the read register value
 * @val: value to wait for
 * @interval_us: polling interval in microseconds
 * @timeout_ms: timeout in milliseconds
 *
 * Return:
 * -ETIMEDOUT on error, zero on success.
 */
int ufs_xring_wait_for_lpis_actrl_register(struct ufs_hba *hba, u32 reg, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms)
{
	int ret = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	while ((lpis_actrl_readl(host, reg) & mask) != val) {
		usleep_range(interval_us, interval_us + 50);
		if (time_after(jiffies, timeout)) {
			if ((lpis_actrl_readl(host, reg) & mask) != val)
				ret = -ETIMEDOUT;
			break;
		}
	}

	return ret;
}

static int ufs_subsys_power_off(struct ufs_hba *hba)
{
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	/* sel ufs reset iso value */
	ret = regmap_write(host->lms_crg_regmap, LMS_CRG_CLKDIV2, 0x1000100);
	if (ret) {
		dev_err(hba->dev, "write LMS_CRG_CLKDIV2 reg fail\n");
		return ret;
	}

	/* HSS1 bus enter idle */
	ret = regmap_write(host->lpis_actrl_regmap, LPIS_ACTRL_HSS1_BUS_PDOM_REQ, 0x180018);
	if (ret) {
		dev_err(hba->dev, "write LPIS_ACTRL_HSS1_BUS_PDOM_REQ reg fail\n");
		return ret;
	}

	ret = ufs_xring_wait_for_lpis_actrl_register(hba, LPIS_ACTRL_HSS1_BUS_PDOM_IDLE,
			GENMASK(4, 3) | GENMASK(10, 9),
			GENMASK(4, 3) | GENMASK(10, 9),
			10, 50);
	if (ret) {
		dev_err(hba->dev, "Failed to entering idle status\n");
		return ret;
	}

	ufs_xring_close_hss1_clk(hba);

	/* ufs iso en */
	ret = regmap_write(host->sys_ctrl_regmap, SYS_CTRL_ISOLATION_GROUP_USEC, 0x10001);
	if (ret) {
		dev_err(hba->dev, "write SYS_CTRL_ISOLATION_GROUP_USEC reg fail\n");
		return ret;
	}

	return 0;
}

static int ufs_xr_hibernate_confirm(struct ufs_hba *hba)
{
	int ret;

	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_UFSHC_STAT,
						UFS_SYS_CTRL_DWC_UFSHC_STAT_UFS_HIBERNATE_STATE_MASK,
						UFS_SYS_CTRL_DWC_UFSHC_STAT_UFS_HIBERNATE_STATE_MASK,
						10, UFS_WAIT_FOR_REG_TIMEOUT);
	if (ret) {
		dev_err(hba->dev, "ufs wait for hibetnate state fail\n");
		return ret;
	}

	return 0;
}

static int ufs_xring_suspend(struct ufs_hba *hba, enum ufs_pm_op op,
				enum ufs_notify_change_status status)
{
	int ret = 0;

	if (status == PRE_CHANGE)
		return 0;

	dev_info(hba->dev, "%s enter\n", __func__);
	ret = ufs_xr_hibernate_confirm(hba);
	if (ret) {
		dev_err(hba->dev, "ufs_xr_hibernate_confirm fail:%d", ret);
		return ret;
	}

	ret = ufs_subsys_power_off(hba);
	if (ret) {
		dev_err(hba->dev, "Ufs Subsys PowerOff fail: %d\n", ret);
		return ret;
	}
	dev_info(hba->dev, "%s end ret=%d ufs_pm_op=%d hba->rpm_lvl=%d hba->spm_lvl=%d hba->uic_link_state=%d hba->curr_dev_pwr_mode=%d\n",
			__func__, ret, op, hba->rpm_lvl, hba->spm_lvl, hba->uic_link_state, hba->curr_dev_pwr_mode);

	return ret;
}

static int ufs_subsys_power_on(struct ufs_hba *hba)
{
	int ret;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	ret = ufs_xring_open_hss1_clk(hba);
	if (ret) {
		dev_err(hba->dev, "ufs open hss1_clk fail:%d\n", ret);
		return ret;
	}

	ufs_xring_close_hss1_clk(hba);

	/* ufs iso disable */
	ret = regmap_write(host->sys_ctrl_regmap, SYS_CTRL_ISOLATION_GROUP_USEC, 0x10000);
	if (ret) {
		dev_err(hba->dev, "write SYS_CTRL_ISOLATION_GROUP_USEC reg fail\n");
		return ret;
	}

	ret = ufs_xring_wait_for_lpis_actrl_register(hba, LPIS_ACTRL_MRC_ACK_1,
			BIT(15), BIT(15), 10, 100);
	if (ret) {
		dev_err(hba->dev, "ufs mem repair fail\n");
		return ret;
	}

	ret = ufs_xring_open_hss1_clk(hba);
	if (ret) {
		dev_err(hba->dev, "ufs open hss1_clk fail:%d\n", ret);
		return ret;
	}

	/* HSS1 bus exit idle */
	ret = regmap_write(host->lpis_actrl_regmap, LPIS_ACTRL_HSS1_BUS_PDOM_REQ, 0x180000);
	if (ret) {
		dev_err(hba->dev, "write LPIS_ACTRL_HSS1_BUS_PDOM_REQ reg fail\n");
		return ret;
	}

	ret = ufs_xring_wait_for_lpis_actrl_register(hba, LPIS_ACTRL_HSS1_BUS_PDOM_IDLE,
			GENMASK(4, 3), 0, 10, 50);
	if (ret) {
		dev_err(hba->dev, "Failed to exit idle status\n");
		return ret;
	}

	return 0;
}

static int ufs_xring_resume(struct ufs_hba *hba, enum ufs_pm_op op)
{
	int ret;

	dev_info(hba->dev, "%s enter\n", __func__);
	ret = ufs_subsys_power_on(hba);
	if (ret) {
		dev_err(hba->dev, "Ufs Subsys PowerOn fail: %d\n", ret);
		return ret;
	}

	dev_info(hba->dev, "%s end ret=%d ufs_pm_op=%d hba->rpm_lvl=%d hba->spm_lvl=%d hba->uic_link_state=%d hba->curr_dev_pwr_mode=%d\n",
			__func__, ret, op, hba->rpm_lvl, hba->spm_lvl, hba->uic_link_state, hba->curr_dev_pwr_mode);

	return 0;
}

static void ufs_xring_update_sdev(void *param, struct scsi_device *sdev)
{
	sdev->broken_fua = 1;
}

/*
 * struct ufs_hba_vops - UFS XRING specific variant operations
 * The variant operations configure the necessary controller and PHY
 * handshake during initialization.
 */
static const struct ufs_hba_variant_ops ufs_hba_vops = {
	.name                   = "xring-ufs",
	.init                   = ufs_xring_init,
	.setup_clocks           = ufs_xring_setup_clocks,
	.hce_enable_notify      = ufs_xring_hce_enable_notify,
	.pwr_change_notify      = ufs_xring_pwr_change_notify,
	.apply_dev_quirks       = ufs_xring_apply_dev_quirks,
	.dbg_register_dump      = ufs_xring_dump_dbg_regs,
	.fixup_dev_quirks       = ufs_xring_fixup_dev_quirks,
	.link_startup_notify    = ufs_xring_link_startup_notify,
	.hibern8_notify         = ufs_xring_hibern8_notify,
	.suspend                = ufs_xring_suspend,
	.resume                 = ufs_xring_resume,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 17))
	/* mcq vops */
	.get_hba_mac         = ufs_xring_get_hba_mac,
	.op_runtime_config   = ufs_xring_op_runtime_config,
	.mcq_config_resource = ufs_xring_mcq_config_resource,
	.config_esi          = ufs_xring_config_esi,
#endif
};

static const struct of_device_id ufs_xring_of_match[] = {
	{ .compatible = "xring,ufs", .data = &ufs_hba_vops},
	{},
};
MODULE_DEVICE_TABLE(of, ufs_xring_of_match);

/**
 * ufs_xring_probe - probe routine of the driver
 * @pdev: pointer to Platform device handle
 *
 * Return zero for success and non-zero for failure
 */
static int ufs_xring_probe(struct platform_device *pdev)
{
	int ret;
	const struct of_device_id *of_id;

	of_id = of_match_node(ufs_xring_of_match, pdev->dev.of_node);

	/* Perform generic probe */
	ret = ufshcd_pltfrm_init(pdev, of_id->data);
	if (ret) {
		dev_err(&pdev->dev, "ufshcd_pltfrm_init() failed %d\n", ret);
		return ret;
	}

	ufs_xring_rpmb_probe(platform_get_drvdata(pdev));

	register_trace_android_vh_ufs_update_sdev(ufs_xring_update_sdev, NULL);

	return ret;
}

/**
 * ufs_xring_remove - set driver_data of the device to NULL
 * @pdev: pointer to platform device handle
 *
 * Always returns 0
 */
static int ufs_xring_remove(struct platform_device *pdev)
{
	struct ufs_hba *hba =  platform_get_drvdata(pdev);
	int ret = 0;

	ufs_xring_rpmb_remove(hba);
	ret = ufs_xring_dbg_unregister();
	pm_runtime_get_sync(&(pdev)->dev);
	ufshcd_remove(hba);

	return ret;
}

static const struct dev_pm_ops ufs_xring_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ufshcd_system_suspend, ufshcd_system_resume)
	SET_RUNTIME_PM_OPS(ufshcd_runtime_suspend, ufshcd_runtime_resume, NULL)
	.prepare	 = ufshcd_suspend_prepare,
	.complete	 = ufshcd_resume_complete,
};

static struct platform_driver ufs_xring_pltform = {
	.probe	= ufs_xring_probe,
	.remove	= ufs_xring_remove,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	.shutdown = ufshcd_pltfrm_shutdown,
#endif
	.driver	= {
		.name	= "ufshcd-xring",
		.pm	= &ufs_xring_pm_ops,
		.of_match_table = of_match_ptr(ufs_xring_of_match),
	},
};
module_platform_driver(ufs_xring_pltform);
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr_regulator xr-clk xr_rpmb fbe trace_hook xr_timestamp");
