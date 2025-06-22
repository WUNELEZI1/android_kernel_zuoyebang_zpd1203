// SPDX-License-Identifier: GPL-2.0-only
/*
 * UFS Host driver for Xring-UFS's FPGA-M_PHY
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 */
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/nls.h>
#include <linux/platform_device.h>
#include <linux/bitfield.h>
#include <ufs_sys_ctrl.h>
#include <ufs/ufshcd.h>
#include "unipro.h"
#include "ufshci.h"
#include "ufs_quirks.h"
#include "ufs-xring.h"
#include "ufs-xring-fpga.h"

#define MPHY_RATEA_CMN   35
#define MPHY_RATEB       27

#define UFSHCD_ENABLE_NEED_INTRS	(UTP_TRANSFER_REQ_COMPL |\
				 UTP_TASK_REQ_COMPL |\
				 UFSHCD_ERROR_MASK)

static struct ufs_xring_dme_attr_val ufs_xring_mphy_apb_ratea_cmn[MPHY_RATEA_CMN];
static struct ufs_xring_dme_attr_val ufs_xring_mphy_apb_rateb[MPHY_RATEB];

struct ufs_xring_mphy_i2c_val {
	u8 i2c_reg;
	u8 i2c_val;
	u8 i2c_slv_addr;
};


static const uint ufs_xring_mphy_apb_ratea_cmn_attrid[MPHY_RATEA_CMN] = {
	0x0142, //RATEA
	0x3021,
	0x3022,
	0x3023,
	0x3039,
	0x311c, //CMN
	0x311d,
	0x311e,
	0x311f,
	0x3135, //RATEA
	0x3136,
	0x3137,
	0x3138,
	0x3139,
	0x313a,
	0x313b,
	0x313c,
	0x313d,
	0x3221,
	0x3222,
	0x3223,
	0x3239,
	0x331c, //CMN
	0x331d,
	0x331e,
	0x331f,
	0x3335, //RATEA
	0x3336,
	0x3337,
	0x3338,
	0x3339,
	0x333a,
	0x333b,
	0x333c,
	0x333d,
};

static const uint ufs_xring_mphy_apb_rateb_attrid[MPHY_RATEB] = {
	0x0143, //RATEB
	0x3024,
	0x3025,
	0x3026,
	0x303a,
	0x313e,
	0x313f,
	0x3140,
	0x3141,
	0x3142,
	0x3143,
	0x3144,
	0x3145,
	0x3146,
	0x3224,
	0x3225,
	0x3226,
	0x323a,
	0x333e,
	0x333f,
	0x3340,
	0x3341,
	0x3342,
	0x3343,
	0x3344,
	0x3345,
	0x3346,
};



/**
 * ufs_xring_mphy_dme_set()
 * @hba: Pointer to drivers structure
 * @ufs_xring_dme_attr_val : dme addt val
 * @mphy_dme_type: mphy dme cmd type
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_dme_set(struct ufs_hba *hba,
				const struct ufs_xring_dme_attr_val *v, u8 mphy_dme_type)
{
	int ret = 0;
	uint reg_val = 0;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (hba == NULL) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	switch (mphy_dme_type) {
	case IS_SFR_BLOCK_SEL:
		ufs_sctrl_rmwl(host, 0xF1, UFS_SYS_CTRL_UFS_FPGA_FPGA_SFR_SEL_MASK,
			UFS_SYS_CTRL_UFS_FPGA);
		break;
	case IS_TX0_BLOCK_SEL:
	case IS_TX1_BLOCK_SEL:
	case IS_RX0_BLOCK_SEL:
	case IS_RX1_BLOCK_SEL:
		reg_val = UFS_SYS_CTRL_UFS_FPGA_FPGA_BLOCK_SEL_OVRD_EN_MASK;
		reg_val = reg_val | (mphy_dme_type << UFS_SYS_CTRL_UFS_FPGA_FPGA_BLOCK_SEL_SHIFT);
		ufs_sctrl_rmwl(host, 0xF1, reg_val, UFS_SYS_CTRL_UFS_FPGA);
		break;
	default:
		ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);
		break;
	}

	ret = ufshcd_dme_set_attr(hba, UIC_ARG_MIB(v->attr_sel), ATTR_SET_NOR,
		v->mib_val, DME_LOCAL);
	if (ret)
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);

	return ret;
}

/**
 * ufs_xring_mphy_dme_get()
 * @hba: Pointer to drivers structure
 * @ufs_xring_dme_attr_val : dme addt val
 * @mphy_dme_type: mphy dme cmd type
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_dme_get(struct ufs_hba *hba,
				struct ufs_xring_dme_attr_val *v, u8 mphy_dme_type)
{
	int ret = 0;
	uint reg_val = 0;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (hba == NULL) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	switch (mphy_dme_type) {
	case IS_SFR_BLOCK_SEL:
		ufs_sctrl_rmwl(host, 0xF1, UFS_SYS_CTRL_UFS_FPGA_FPGA_SFR_SEL_MASK,
			UFS_SYS_CTRL_UFS_FPGA);
		break;
	case IS_TX0_BLOCK_SEL:
	case IS_TX1_BLOCK_SEL:
	case IS_RX0_BLOCK_SEL:
	case IS_RX1_BLOCK_SEL:
		reg_val = UFS_SYS_CTRL_UFS_FPGA_FPGA_BLOCK_SEL_OVRD_EN_MASK;
		reg_val = reg_val | (mphy_dme_type << UFS_SYS_CTRL_UFS_FPGA_FPGA_BLOCK_SEL_SHIFT);
		ufs_sctrl_rmwl(host, 0xF1, reg_val, UFS_SYS_CTRL_UFS_FPGA);
		break;
	default:
		ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);
		break;
	}

	ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB(v->attr_sel),
		&v->mib_val, DME_LOCAL);
	if (ret)
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);

	return ret;
}

/**
 * ufs_xring_mphy_dme_wait_for - wait for m_phy_dme value to change
 * @hba: per-adapter interface
 * @reg: DME ATTRID
 * @mask: mask to apply to the read value
 * @val: value to wait for
 * @interval_us: polling interval in microseconds
 * @timeout_ms: timeout in milliseconds
 * @mphy_dme_type: mphy ops mode

 * Return:
 * -ETIMEDOUT on error, zero on success.
 */
int ufs_xring_mphy_dme_wait_for(struct ufs_hba *hba, u32 attrid, u32 mask,
				u32 val, unsigned long interval_us,
				unsigned long timeout_ms,  u8 mphy_dme_type)
{
	int ret = 0;
	struct ufs_xring_dme_attr_val phy_cmd;
	unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);

	if (IS_ERR(hba)) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return PTR_ERR(hba);
	}

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	phy_cmd.attr_sel = attrid;
	phy_cmd.mib_val = ~val;

	while ((phy_cmd.mib_val & mask) != val) {
		ret = ufs_xring_mphy_dme_get(hba, &phy_cmd, mphy_dme_type);
		if (ret) {
			dev_err(hba->dev, "%s mphy_dme_get fail.!\n", __func__);
			goto out;
		}
		usleep_range(interval_us, interval_us + 50);
		if (time_after(jiffies, timeout)) {
			ret = ufs_xring_mphy_dme_get(hba, &phy_cmd, mphy_dme_type);
			if (ret) {
				dev_err(hba->dev, "%s mphy_dme_get fail.!\n", __func__);
				goto out;
			}
			if ((phy_cmd.mib_val & mask) != val)
				ret = -ETIMEDOUT;
			goto out;
		}
	}

out:
	return ret;
}

/**
 * ufs_xring_mphy_cb_set()
 * @hba: Pointer to drivers structure
 * @ufs_xring_dme_attr_val : dme addt val
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_cb_set(struct ufs_hba *hba, const struct ufs_xring_dme_attr_val *v)
{
	int ret = -ETIMEDOUT;
	struct ufs_xring_dme_attr_val cb_cmd;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (hba == NULL) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	/* CB write addr */
	ret = ufs_xring_mphy_dme_set(hba, v, IS_CB_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0xD085;
	cb_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &cb_cmd, IS_TX0_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0xD085;
	cb_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &cb_cmd, IS_TX1_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0xD085;
	cb_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &cb_cmd, IS_RX0_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0xD085;
	cb_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &cb_cmd, IS_RX1_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

	cb_cmd.attr_sel = 0x21;
	cb_cmd.mib_val = 0x00;
	ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB(cb_cmd.attr_sel),
		&cb_cmd.mib_val, DME_LOCAL);
	if (ret)
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);

out:
	return ret;
}

/**
 * ufs_xring_mphy_apb_set()
 * @hba: Pointer to drivers structure
 * @ufs_xring_dme_attr_val : dme addt val
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_apb_set(struct ufs_hba *hba, const struct ufs_xring_dme_attr_val *v)
{
	int ret = -ETIMEDOUT;
	ktime_t timeout;
	u32 reg_val = 0;
	struct ufs_xring_dme_attr_val cb_cmd;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	if (hba == NULL) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return -EINVAL;
	}

	/* CB write addr */
	cb_cmd.attr_sel = 0x810E;
	cb_cmd.mib_val = (u8)v->attr_sel;
	ret = ufs_xring_mphy_cb_set(hba, &cb_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0x810F;
	cb_cmd.mib_val = (u8)(v->attr_sel >> 8);
	ret = ufs_xring_mphy_cb_set(hba, &cb_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* CB write data */
	cb_cmd.attr_sel = 0x8110;
	cb_cmd.mib_val = (u8)v->mib_val;
	ret = ufs_xring_mphy_cb_set(hba, &cb_cmd);
	if  (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	cb_cmd.attr_sel = 0x8111;
	cb_cmd.mib_val = (u8)(v->mib_val >> 8);
	ret = ufs_xring_mphy_cb_set(hba, &cb_cmd);
	if (ret) {
		dev_err(hba->dev,  "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* CB write addr 0x14 vak = 0x01 */
	cb_cmd.attr_sel = 0x8114;
	cb_cmd.mib_val = 1;
	ret = ufs_xring_mphy_cb_set(hba, &cb_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* Poll tx_busy_0 flag to be de-asserted(0 is de-assert). */
	timeout = ktime_add_ms(ktime_get(), 100);
	do {
		reg_val = ufs_sctrl_readl(host, UFS_SYS_CTRL_DWC_MPHY_STAT);
		if (!(reg_val & UFS_SYS_CTRL_DWC_MPHY_STAT_TX_BUSY_0_MASK)) {
			ret = 0;
			break;
		}
		usleep_range(100, 2000);
	} while (ktime_before(ktime_get(), timeout));

	if (ret) {
		dev_err(hba->dev,
				"%s reading tx_busy_0 flag to be de-asserted timeout!\n", __func__);
	}

out:
	return ret;
}

/**
 * ufs_xring_mphy_apb_set_attrs()
 * @hba: Pointer to drivers structure
 * @ufs_xring_dme_attr_val : dme addt val
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_apb_set_attrs(struct ufs_hba *hba,
				const struct ufs_xring_dme_attr_val *v, int n)
{
	int ret = 0;
	int attr_node = 0;

	if (IS_ERR(hba)) {
		dev_err(hba->dev, "%s: %d, Error, hba == NULL.\n", __func__, __LINE__);
		return PTR_ERR(hba);
	}

	for (attr_node = 0; attr_node < n; attr_node++) {
		ret = ufs_xring_mphy_apb_set(hba, &v[attr_node]);

		if (ret)
			return ret;

		usleep_range(5000, 10000);
	}

	return 0;
}

/**
 * ufs_xring_mphy_setup_set_status_reg()
 * @hba: Pointer to drivers structure
 *
 * Returns 0 on success or non-zero value on failure
 */

static int ufs_xring_mphy_setup_set_status_reg(struct ufs_hba *hba)
{
	static const struct ufs_xring_dme_attr_val setup_attrs[] = {
		{0x015c, 0x02, DME_LOCAL },
		{0x3122, 0x0C, DME_LOCAL },
		{0x3101, 0x02, DME_LOCAL },
		{0x3322, 0x0C, DME_LOCAL },
		{0x3301, 0x02, DME_LOCAL },
	};

	return ufs_xring_mphy_apb_set_attrs(hba, setup_attrs, ARRAY_SIZE(setup_attrs));
}

/**
 * ufs_xring_mphy_assert_reset()
 * @hba: Pointer to drivers structure
 *
 * Returns 0 on success or non-zero value on failure
 */

int ufs_xring_mphy_assert_reset(struct ufs_hba *hba)
{
	int ret = 0;
	struct ufs_xring_dme_attr_val phy_cmd;

	phy_cmd.attr_sel = 0x87;
	phy_cmd.mib_val = 0x30;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret)
		dev_err(hba->dev,
			"%s: mphy assert reset fail, ret is = %d.\n",
			__func__, ret);

	return ret;
}

/**
 * ufs_xring_mphy_deassert_reset()
 * @hba: Pointer to drivers structure
 *
 * Returns 0 on success or non-zero value on failure
 */

int ufs_xring_mphy_deassert_reset(struct ufs_hba *hba)
{
	int ret = 0;
	struct ufs_xring_dme_attr_val phy_cmd;

	phy_cmd.attr_sel = 0x87;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret)
		dev_err(hba->dev,
			"%s: mphy deassert reset fail, ret is = %d.\n",
			__func__, ret);

	return ret;
}

/**
 * ufs_xring_subsystem_powerup()
 * @hba: Pointer to drivers structure
 *
 * Returns 0 on success or non-zero value on failure
 */

int ufs_xring_subsystem_powerup(struct ufs_hba *hba)
{
	int ret = 0;

	ufs_xring_subsys_reset(hba);

	ufs_xring_mphy_clk_cfg_fpga(hba);

	ufs_host_controller_init(hba);

	return ret;
}


/**
 * ufs_xring_setup_mphy_pwr_up_seq()
 * @hba: Pointer to drivers structure
 *
 * Returns 0 on success or non-zero value on failure
 */

int ufs_xring_setup_mphy_pwr_up_seq(struct ufs_hba *hba)
{
	int ret = 0;
	ktime_t timeout;
	bool write_calibration_banks;
	struct ufs_xring_dme_attr_val phy_cmd;
	struct ufs_xring_host *host = ufshcd_get_variant(hba);

	dev_err(hba->dev, "%s: %d, hba->dev_ref_clk_freq = %d.\n",
		__func__, __LINE__, hba->dev_ref_clk_freq);

	if (hba->dev_ref_clk_freq == REF_CLK_FREQ_INVAL) {
		dev_err(hba->dev, "dev_ref_clk not configured.\n");
		ret = -EINVAL;
		goto out;
	}

	switch (host->mphy_pro_calibration_status) {
	case MPHY_PRO_CALIBRATION_STEP1:
	case MPHY_PRO_CALIBRATION_STEP2:
		write_calibration_banks = false;
		break;
	default:
		host->mphy_pro_calibration_status = MPHY_PRO_CALIBRATIONED;
		write_calibration_banks = true;
		break;
	}

	/* step0:*/
	ret = ufs_xring_subsystem_powerup(hba);

	/* step5: M-PHY init */
	/* (1) SFR: sel phy reset, tc_reset all is 1 */
	phy_cmd.attr_sel = 0x87;
	phy_cmd.mib_val = 0x30;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x8B;
	phy_cmd.mib_val = 0x03;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (2) SFR: sel phy mode */
	phy_cmd.attr_sel = 0x86;
	phy_cmd.mib_val = 0x03;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* step2: M-PHY's ocs cfg */
	/* step2 - 5: cfg osc freq 38.4MHz(0x02) */
	phy_cmd.attr_sel = 0x85;
	phy_cmd.mib_val = hba->dev_ref_clk_freq;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (4) SFR: sel cfg_clock_sel 19.2M */
	phy_cmd.attr_sel = 0x84;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (5) CB: pulse on cb reg */
	phy_cmd.attr_sel = 0x811A;
	phy_cmd.mib_val = 0x02;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}
	usleep_range(5000, 10000);
	phy_cmd.attr_sel = 0x811A;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* add: new,20230710-11 */
	phy_cmd.attr_sel = 0x8106;
	phy_cmd.mib_val = 0x12;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (6) SFR: sel firmware run mode, ROM mode */
	phy_cmd.attr_sel = 0xAA;
	phy_cmd.mib_val = 0x00;/* by sram */
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xAB;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (7) SFR: De-assert tc_reset */
	phy_cmd.attr_sel = 0x87;
	phy_cmd.mib_val = 0x10;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (8) SFR: Generate jtag controller rst */
	phy_cmd.attr_sel = 0x90;
	phy_cmd.mib_val = 0x10;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev,  "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	usleep_range(5000, 10000);

	phy_cmd.attr_sel = 0x90;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (9) SFR: Generate jtag_reset */
	phy_cmd.attr_sel = 0x95;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x95;
	phy_cmd.mib_val = 0x04;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x95;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	usleep_range(100, 1000);

	phy_cmd.attr_sel = 0x95;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x95;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (10) SFR: SRL reset */
	phy_cmd.attr_sel = 0xA7;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xA7;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (11) SFR: wait for tc_ready */
	ret = ufs_xring_mphy_dme_wait_for(hba, 0x12, BIT_MASK(0), 0x01,
			1000, 1000, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for tc_ready_1 failed\n", __func__);
		goto out;
	}

	/* (12) SFR: enable TC configuration */
	phy_cmd.attr_sel = 0xA6;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xA6;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (13) SFR: wait for SFR: tc_ready */
	ret = ufs_xring_mphy_dme_wait_for(hba, 0x12, BIT_MASK(0), 0x01,
			100, 1000, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for tc_ready_2 failed\n", __func__);
		goto out;
	}

	/* (14) UFS_SCTRL: wait for SYS_CTRL: tx_cfgrdyn_N, rx_cfgrdyn_N is 0 */
	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
			0x3C0,
			0x00 << UFS_SYS_CTRL_DWC_MPHY_STAT_TX_CFGRDYN_0_SHIFT,
			10, 1000);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for tx_cfgrdyn_N failed\n", __func__);
		goto out;
	}

	/* (15) CB: Cfg RMMI ATTR/Setup PHY primary ports */
	phy_cmd.attr_sel = 0x8101;
	phy_cmd.mib_val = 0x05;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	usleep_range(5000, 10000);
	phy_cmd.attr_sel = 0x8118;
	phy_cmd.mib_val = 0x04;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}
	usleep_range(5000, 10000);
	phy_cmd.attr_sel = 0x8115;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_cb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}
	usleep_range(5000, 10000);


	/* (16) DME_SET: rend a txN_cfgupt pulse, rxN_cfgupt pulse */
	phy_cmd.attr_sel = 0xD085;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_TX0_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xD085;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_TX1_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xD085;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_RX0_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xD085;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_RX1_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (17) SFR: de-assert phy_reset */
	phy_cmd.attr_sel = 0x87;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* step4: device deassert-rst */
	/* UFS_SCTRL: devicec reset de-assert, is set 1 */
	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_UFS_DEVC_RST_N_UFS_RST_N_MASK,
			0 << UFS_SYS_CTRL_UFS_DEVC_RST_N_UFS_RST_N_SHIFT,
			UFS_SYS_CTRL_UFS_DEVC_RST_N);

	usleep_range(10, 50);

	ufs_sctrl_rmwl(host, UFS_SYS_CTRL_UFS_DEVC_RST_N_UFS_RST_N_MASK,
			1 << UFS_SYS_CTRL_UFS_DEVC_RST_N_UFS_RST_N_SHIFT,
			UFS_SYS_CTRL_UFS_DEVC_RST_N);


	/* (18) SFR: wait for sram_init_done */
	ret = ufs_xring_mphy_dme_wait_for(hba, 0x19, BIT_MASK(0), 0x01,
			1000, 5000, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for sram_init_done failed\n", __func__);
		goto out;
	}

	/* (19) APB: load re-calibration banks */
	if (write_calibration_banks) {
		ret = ufs_xring_mphy_apb_set_attrs(hba,
			ufs_xring_mphy_apb_ratea_cmn, MPHY_RATEA_CMN);
		if (ret) {
			dev_err(hba->dev, "%s: set mphy_apb_ratea_cmn failed\n", __func__);
			goto out;
		}

		ret = ufs_xring_mphy_apb_set_attrs(hba, ufs_xring_mphy_apb_rateb, MPHY_RATEB);
		if (ret) {
			dev_err(hba->dev, "%s: set mphy_apb_rateb failed\n", __func__);
			goto out;
		}

		ret = ufs_xring_mphy_setup_set_status_reg(hba);
		if (ret) {
			dev_err(hba->dev, "%s: set mphy_status_reg failed\n", __func__);
			goto out;
		}
	}

	/* (20) APB: write on reg RX_CDR_DETECTOR_EN */
	phy_cmd.attr_sel = 0x31B6;
	phy_cmd.mib_val = 0x0004;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x33B6;
	phy_cmd.mib_val = 0x0004;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x3103;
	phy_cmd.mib_val = 0x0020;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0x3303;
	phy_cmd.mib_val = 0x0020;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (21) SFR: enable testchip cfg */
	phy_cmd.attr_sel = 0xA6;
	phy_cmd.mib_val = 0x02;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	phy_cmd.attr_sel = 0xA6;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (22) SFR: wait for tc_ready */
	ret = ufs_xring_mphy_dme_wait_for(hba, 0x12, BIT_MASK(0), 0x01,
			10, 10, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for tc_ready_3 failed\n", __func__);
		goto out;
	}

	/* (add) samsung exit h8 and switch speed buf fix */
	phy_cmd.attr_sel = 0x10BB;
	phy_cmd.mib_val = 0x3C00;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}
	phy_cmd.attr_sel = 0x12BB;
	phy_cmd.mib_val = 0x3C00;
	ret = ufs_xring_mphy_apb_set(hba, &phy_cmd);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (23) SFR: set sram_ext_ld_done */
	phy_cmd.attr_sel = 0xAB;
	phy_cmd.mib_val = 0x01;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_SFR_BLOCK_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (24) DME_SET: mphy_disbale = 0 */
	phy_cmd.attr_sel = 0xD0C1;
	phy_cmd.mib_val = 0x00;
	ret = ufs_xring_mphy_dme_set(hba, &phy_cmd, IS_NORMAL_SEL);
	if (ret) {
		dev_err(hba->dev, "%s: %d, ops ret is = %d.\n", __func__, __LINE__, ret);
		goto out;
	}

	/* (25) UFS_SCTRL: wait for SYS_CTRL: tx_busy_N, rx_busy_N is 0*/
	ret = ufs_xring_wait_for_sctrl_reg(hba, UFS_SYS_CTRL_DWC_MPHY_STAT,
			0x3C,
			0x00 << UFS_SYS_CTRL_DWC_MPHY_STAT_TX_BUSY_0_SHIFT,
			100, 5000);
	if (ret) {
		dev_err(hba->dev, "%s: Wait for tx_cfgrdyn_N failed\n", __func__);
		goto out;
	}


	/* (26) DME_GET: wait for M-PHY FSM state is Hibern8 */
	timeout = ktime_add_ms(ktime_get(), 10000);
	ret = -ETIMEDOUT;
	phy_cmd.attr_sel = 0x41;
	phy_cmd.mib_val = 0x00;/* tx lane0 */

	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

	do {
		ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB_SEL(phy_cmd.attr_sel, phy_cmd.mib_val),
			&phy_cmd.mib_val, DME_LOCAL);
		if (ret) {
			dev_err(hba->dev, "%s,%d: ufshcd_dme_get_attr_1 failed\n",
				__func__, __LINE__);
			goto out;
		}

		if (phy_cmd.mib_val & BIT_MASK(0)) {
			ret = 0;
			break;
		}
		usleep_range(1000, 2000);
	} while (ktime_before(ktime_get(), timeout));

	if (ret) {
		dev_err(hba->dev,
			"%s,%d: t0,wait for M-PHY FSM_1 timeout!\n", __func__, __LINE__);
		goto out;
	}

	timeout = ktime_add_ms(ktime_get(), 10000);
	ret = -ETIMEDOUT;
	phy_cmd.attr_sel = 0x41;
	phy_cmd.mib_val = 0x01;/* tx lane1 */
	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

	do {
		ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB_SEL(phy_cmd.attr_sel, phy_cmd.mib_val),
			&phy_cmd.mib_val, DME_LOCAL);
		if (ret) {
			dev_err(hba->dev, "%s: ufshcd_dme_get_attr_1 failed\n", __func__);
			goto out;
		}

		if (phy_cmd.mib_val & BIT_MASK(0)) {
			ret = 0;
			break;
		}
		usleep_range(1000, 2000);
	} while (ktime_before(ktime_get(), timeout));

	if (ret) {
		dev_err(hba->dev,
			"%s,%d:t1, wait for M-PHY FSM_1 timeout!\n", __func__, __LINE__);
		goto out;
	}

	timeout = ktime_add_ms(ktime_get(), 10000);
	ret = -ETIMEDOUT;
	phy_cmd.attr_sel = 0xC1;
	phy_cmd.mib_val = 0x04;/* rx lane0 */
	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

	do {
		ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB_SEL(phy_cmd.attr_sel, phy_cmd.mib_val),
			&phy_cmd.mib_val, DME_LOCAL);
		if (ret) {
			dev_err(hba->dev, "%s,%d: ufshcd_dme_get_attr_1 failed\n",
				__func__, __LINE__);
			goto out;
		}

		if (phy_cmd.mib_val & BIT_MASK(0)) {
			ret = 0;
			break;
		}
		usleep_range(1000, 2000);
	} while (ktime_before(ktime_get(), timeout));

	if (ret) {
		dev_err(hba->dev,
			"%s,%d: r0 wait for M-PHY FSM_1 timeout!\n", __func__, __LINE__);
		goto out;
	}

	timeout = ktime_add_ms(ktime_get(), 10000);
	ret = -ETIMEDOUT;
	phy_cmd.attr_sel = 0xC1;
	phy_cmd.mib_val = 0x05;/* rx lane1 */
	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

	do {
		ret = ufshcd_dme_get_attr(hba, UIC_ARG_MIB_SEL(phy_cmd.attr_sel, phy_cmd.mib_val),
			&phy_cmd.mib_val, DME_LOCAL);
		if (ret) {
			dev_err(hba->dev, "%s,%d: ufshcd_dme_get_attr_1 failed\n",
				__func__, __LINE__);
			goto out;
		}

		if (phy_cmd.mib_val & BIT_MASK(0)) {
			ret = 0;
			break;
		}
		usleep_range(1000, 2000);
	} while (ktime_before(ktime_get(), timeout));

	if (ret) {
		dev_err(hba->dev,
			"%s,%d:r1 wait for M-PHY FSM_1 timeout!\n", __func__, __LINE__);
		goto out;
	}

	/* clear UE(WC)&UIC E(ROC): */
	ufshcd_writel(hba, 0xFFFFFFFF, REG_INTERRUPT_STATUS);
	ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER);
	ufshcd_readl(hba, REG_UIC_ERROR_CODE_DATA_LINK_LAYER);
	ufshcd_readl(hba, REG_UIC_ERROR_CODE_NETWORK_LAYER);
	ufshcd_readl(hba, REG_UIC_ERROR_CODE_TRANSPORT_LAYER);
	ufshcd_readl(hba, REG_UIC_ERROR_CODE_DME);

	ufs_sctrl_rmwl(host, 0xF1, 0x0, UFS_SYS_CTRL_UFS_FPGA);

out:
	return ret;
}


/**
 * ufs_xring_setup_mphy_pro_calibration()
 * @hba: Pointer to drivers structure.
 * used in function: ufshcd_tune_unipro_parems()->apply_dev_quirks
 * Returns 0 on success or non-zero value on failure
 */

int ufs_xring_setup_mphy_pro_calibration(struct ufs_hba *hba)
{
	struct ufs_xring_host *host = ufshcd_get_variant(hba);
#ifdef TEST_DONT_CAL
	host->mphy_pro_calibration_status = MPHY_PRO_CALIBRATION_STEP1;
	return 0;
#endif
}

MODULE_LICENSE("GPL v2");
