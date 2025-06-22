// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-combophy.c - 3.0 & DP Phy Driver for XRing USB&DP.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-generic-phy.h"
#include "xr-usb-util.h"
#include "combophy_fw.h"

#include <soc/xring/xr_usbdp_event.h>
#include <dt-bindings/xring/platform-specific/usb_sctrl.h>
#include <dt-bindings/xring/xr-usb-combophy.h>
#include <asm-generic/errno-base.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>

#undef pr_fmt
#undef dev_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#define dev_fmt(fmt) ": %s:%d: " fmt, __func__, __LINE__

#define PHY_PWR_EN                                              \
	(BIT(USB_PHY0_PCS_PWR_EN_PHY0_PCS_PWR_EN_SHIFT) |       \
	 BIT(USB_PHY0_PCS_PWR_EN_PHY0_PMA_PWR_EN_SHIFT) |       \
	 BIT(USB_PHY0_PCS_PWR_EN_UPCS_PWR_EN_SHIFT))

#define HSS1_BUSIDLE_WAIT_INTERVAL	(50ul)
#define HSS1_BUSIDLE_LOOP_NUM		(60ul)
#define SRAM_INIT_WAIT_INTERVAL		(20ul)
#define SRAM_INIT_LOOP_NUM		(150ul)
#define ROM_MODE			(3u)
#define SRAM_MODE			(0u)

#define TCA_INTR_EN			0x04
#define XA_ACK_EVT_EN			0
#define XA_TIMEOUT_EVT_EN		1
#define TCA_INTR_STS			0x08
#define XA_ACK_EVT			0
#define XA_TIMEOUT_EVT			1
#define TCA_GCFG			0x10
#define AUTO_MODE_EN			12
#define TCA_TCPC			0x14
#define TCPC_MUX_CONTROL_SHIFT		0
#define TCPC_MUX_CONTROL_MASK		(0x07u)
#define TCPC_CONN_ORIENTAION_SHIFT	3
#define TCPC_CONN_ORIENTAION_MASK	(0x08u)
#define TCPC_VALID_SHIFT		4
#define TCPC_VALID_MASK			(0x10u)
#define TCA_EVT_STATUS			(BIT(XA_ACK_EVT) | BIT(XA_TIMEOUT_EVT))
#define TCA_EVT_WAIT_INTERVAL		(1000ul)
#define TCA_EVT_LOOP_NUM		(500ul)
#define TCA_PSTATE_1			0x54
#define USB32_LANE0_PD_MASK		0xf
#define USB32_LANE0_PD_SHIFT		0
#define USB32_LANE1_PD_MASK		0xf0000
#define USB32_LANE1_PD_SHIFT		16
#define PD_WAIT_INTERVAL		(500ul)
#define PD_LOOP_NUM			(100ul)
#define P4PG_STATE			0x0c

#define TCA_CTRLSSYNC_CFG1		0x24
#define XA_TIMEOUT_VAL_MASK		(0xfffff)

#define TCA_CTRLSSYNC_DBG0		0x28
#define DPALT_DIS			BIT(1)
#define TCA_DP_INTERVAL			(1000ul)
#define TCA_DP_LOOP			(500ul)

#define TCA_CTRLSSYNC_DBG2		0x30
#define DPALT_DIS_CTRL_EN		BIT(2)
#define DPALT_DIS_CTRL_VAL		BIT(18)

#define TCA_PSTATE_0			0x50
#define UPCS_LANE0_PHYSTATUS		8
#define UPCS_LANE1_PHYSTATUS		24
#define UPCS_PHY_STATUS			(BIT(UPCS_LANE0_PHYSTATUS) | BIT(UPCS_LANE1_PHYSTATUS))
#define PHYSTATUS_WAIT_INTERVAL		(1000ul)
#define PHYSTATUS_LOOP_NUM		(30ul)

#define COMBOPHY_FW_START               0x8000U
#define COMBOPHY_APB_OFFSET(n)          ((n) << 2)
/* combophy ssc & eye reg */
/*
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 * | Index | Prop                  | BitSet  | Shift |     Mask | PerLane | RegConfig |
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 * |     0 | MPLLA_SSC_PEAK        | [19:0]  |     0 |  0xFFFFF | N       | 6:[15:0]  |
 * |       |                       |         |       |          |         | 5:[3:0]   |
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 * |     1 | MPLLA_SSC_STEP_SIZE   | [20:0]  |     0 | 0x1FFFFF | N       | 4:[15:0]  |
 * |       |                       |         |       |          |         | 3:[9:5]   |
 * |       | SSC_OVRD_EN           | [30]    |    30 |      0x1 | N       | 3:[4]     |
 * |       | SSC_EN                | [31]    |    31 |      0x1 | N       | 3:[2]     |
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 * |     2 | TX_PRE_CURSOR         | [6:0]   |     0 |     0x7F | Y       | 2:[6:0]   |
 * |       | TX_MAIN_CURSOR        | [13:7]  |     7 |     0x7F | Y       | 1:[13:7]  |
 * |       | TX_POST_CURSOR        | [20:14] |    14 |     0x7F | Y       | 1:[6:0]   |
 * |       | TX_EQ_OVRD_EN         | [31]    |    31 |      0x1 | Y       | 1:[14]    |
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 * |     3 | TX_VBOOST_LVL         | [2:0]   |     0 |      0x7 | N       | 7:[8:6]   |
 * |       | TX_IBOOST_LVL         | [6:3]   |     3 |      0xF | Y       | 0:[6:3]   |
 * |       | TX_VBOOST_LVL_OVRD_EN | [29]    |    29 |      0x1 | N       | 7:[9]     |
 * |       | TX_VBOOST_EN          | [30]    |    30 |      0x1 | Y       | 0:[7]     |
 * |       | TX_OVRD_EN            | [31]    |    31 |      0x1 | Y       | 0:[8]     |
 * |-------+-----------------------+---------+-------+----------+---------+-----------|
 *
 * |-------+-----------------------------+----------------+---------|
 * | Index | RegName                     | Offset         | PerLane |
 * |-------+-----------------------------+----------------+---------|
 * |     0 | LANEN_DIG_ASIC_TX_OVRD_IN_2 | 0x1003+N*0x200 | Y       |
 * |     1 | LANEN_DIG_ASIC_TX_OVRD_IN_3 | 0x1004+N*0x200 | Y       |
 * |     2 | LANEN_DIG_ASIC_TX_OVRD_IN_4 | 0x1005+N*0x200 | Y       |
 * |     3 | SUP_DIG_MPLLA_SSC_OVRD_IN_0 | 0x10           | N       |
 * |     4 | SUP_DIG_MPLLA_SSC_OVRD_IN_1 | 0x11           | N       |
 * |     5 | SUP_DIG_MPLLA_SSC_OVRD_IN_2 | 0x12           | N       |
 * |     6 | SUP_DIG_MPLLA_SSC_OVRD_IN_3 | 0x13           | N       |
 * |     7 | SUP_DIG_LVL_OVRD_IN         | 0x29           | N       |
 * |-------+-----------------------------+----------------+---------|
 */

#define LANEN_DIG_ASIC_TX_OVRD_IN_2(n)		((0x1003 + 0x200 * (n)) << 2)
#define TX_IBOOST_LVL_MASK			(0x78u)
#define TX_IBOOST_LVL_SHIFT			3
#define TX_VBOOST_EN_MASK			(0x80u)
#define TX_VBOOST_EN_SHIFT			7
#define TX_OVRD_EN_MASK				(0x100u)
#define TX_OVRD_EN_SHIFT			8

#define LANEN_DIG_ASIC_TX_OVRD_IN_3(n)		((0x1004 + 0x200 * (n)) << 2)
#define TX_MAIN_CURSOR_MASK			(0x3F80u)
#define TX_MAIN_CURSOR_SHIFT			7
#define TX_POST_CURSOR_MASK			(0x7Fu)
#define TX_POST_CURSOR_SHIFT			0
#define TX_EQ_OVRD_EN_MASK			(0x4000u)
#define TX_EQ_OVRD_EN_SHIFT			14

#define LANEN_DIG_ASIC_TX_OVRD_IN_4(n)		((0x1005 + 0x200 * (n)) << 2)
#define TX_PRE_CURSOR_MASK			(0x7Fu)
#define TX_PRE_CURSOR_SHIFT			0

#define LANEN_DIG_ANA_XF_TX_STAT_EQ_OVRD_0(n)	((0x106f + 0x200 * (n)) << 2)
#define TX_ANA_EQ_LFPS_OVRD_EN_SHIFT		12
#define TX_ANA_EQ_LFPS_OVRD_EN_MASK		BIT(12)

#define SUP_DIG_MPLLA_SSC_OVRD_IN_0		(0x10u << 2)
#define MPLLA_SSC_STEP_SIZE_20_16_MASK		(0x3E0u)
#define MPLLA_SSC_STEP_SIZE_20_16_SHIFT		5
#define SSC_OVRD_EN_MASK			(0x10u)
#define SSC_OVRD_EN_SHIFT			4
#define SSC_EN_MASK				(0x4u)
#define SSC_EN_SHIFT				2

#define SUP_DIG_MPLLA_SSC_OVRD_IN_1		(0x11u << 2)
#define MPLLA_SSC_STEP_SIZE_15_0_MASK		(0xFFFFu)
#define MPLLA_SSC_STEP_SIZE_15_0_SHIFT		0

#define SUP_DIG_MPLLA_SSC_OVRD_IN_2		(0x12u << 2)
#define MPLLA_SSC_PEAK_19_16_MASK		(0xFu)
#define MPLLA_SSC_PEAK_19_16_SHIFT		0

#define SUP_DIG_MPLLA_SSC_OVRD_IN_3		(0x13u << 2)
#define MPLLA_SSC_PEAK_15_0_MASK		(0xFFFFu)
#define MPLLA_SSC_PEAK_15_0_SHIFT		0

#define SUP_DIG_LVL_OVRD_IN			(0x29u << 2)
#define TX_VBOOST_LVL_MASK			0x1C0u
#define TX_VBOOST_LVL_SHIFT			6
#define TX_VBOOST_LVL_OVRD_EN_MASK		0x200u
#define TX_VBOOST_LVL_OVRD_EN_SHIFT		9

#define PARAM_SET_LANE0				0
#define PARAM_SET_LANE1				1
#define PARAM_SET_LANE2				2
#define PARAM_SET_LANE3				3

#pragma pack(4)
struct ssc_peak_param {
	unsigned int mplla_ssc_peak_15_0 : 16;
	unsigned int mplla_ssc_peak_19_16 : 4;
	unsigned int reserved : 12;
};

struct ssc_step_size_param {
	unsigned int mplla_ssc_step_size_15_0 : 16;
	unsigned int mplla_ssc_step_size_20_16 : 5;
	unsigned int reserved : 9;
	unsigned int ssc_ovrd_en : 1;
	unsigned int ssc_en : 1;
};

struct cursor_eye_param {
	unsigned int tx_pre_cusor : 7;
	unsigned int tx_main_cusor : 7;
	unsigned int tx_post_cusor : 7;
	unsigned int reserved : 10;
	unsigned int tx_eq_ovrd_en : 1;
};

struct boost_eye_param {
	unsigned int tx_vboost_lvl : 3;
	unsigned int tx_iboost_lvl : 4;
	unsigned int reserved : 22;
	unsigned int tx_vboost_lvl_ovrd_en : 1;
	unsigned int tx_vboost_en : 1;
	unsigned int tx_ovrd_en : 1;
};
#pragma pack()

struct xr_usb_combophy_priv {
	struct device *dev;
	void __iomem *usbctrl;
	void __iomem *ssphy;
	void __iomem *tca;

	struct phy *phy;
	struct xr_usb_combophy combophy;

	unsigned int is_fpga : 1;
};

static inline bool is_invert_set(struct xr_usb_combophy *combophy_data)
{
	return combophy_data->submode.plat_submode.invert;
}

static inline bool orientation_get(struct xr_usb_combophy_priv *priv)
{
	return priv->combophy.submode.plat_submode.flip;
}

void ssc_peak_set(struct xr_usb_combophy_priv *priv, const void *param)
{
	const struct ssc_peak_param *p = param;
	unsigned int val = 0;
	unsigned int reg;

	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_0);
	reg &= ~SSC_EN_MASK;
	writel(reg, priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_0);

	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_3);
	reg &= ~MPLLA_SSC_PEAK_15_0_MASK;
	val = p->mplla_ssc_peak_15_0 << MPLLA_SSC_PEAK_15_0_SHIFT;
	reg |= val;
	writel(reg, priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_3);

	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_2);
	reg &= ~MPLLA_SSC_PEAK_19_16_MASK;
	val = p->mplla_ssc_peak_19_16 << MPLLA_SSC_PEAK_19_16_SHIFT;
	reg |= val;
	writel(reg, priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_2);
}

unsigned int ssc_peak_get(struct xr_usb_combophy_priv *priv)
{
	struct ssc_peak_param *p;
	unsigned int val = 0;
	unsigned int reg;

	p = (struct ssc_peak_param *)&val;
	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_2);
	p->mplla_ssc_peak_19_16 = (reg & MPLLA_SSC_PEAK_19_16_MASK) >>
				MPLLA_SSC_PEAK_19_16_SHIFT;
	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_3);
	p->mplla_ssc_peak_15_0 = (reg & MPLLA_SSC_PEAK_15_0_MASK) >>
				MPLLA_SSC_PEAK_15_0_SHIFT;

	return val;
}

void ssc_stepsize_set(struct xr_usb_combophy_priv *priv, const void *param)
{
	const struct ssc_step_size_param *p = param;
	unsigned int val = 0;
	unsigned int reg;

	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_1);
	reg &= ~MPLLA_SSC_STEP_SIZE_15_0_MASK;
	val = p->mplla_ssc_step_size_15_0 << MPLLA_SSC_STEP_SIZE_15_0_SHIFT;
	reg |= val;
	writel(reg, priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_1);

	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_0);
	reg &= ~(MPLLA_SSC_STEP_SIZE_20_16_MASK |
		 SSC_OVRD_EN_MASK |
		 SSC_EN_MASK);
	val = (p->mplla_ssc_step_size_20_16 << MPLLA_SSC_STEP_SIZE_20_16_SHIFT) |
	      (p->ssc_ovrd_en << SSC_OVRD_EN_SHIFT) |
	      (p->ssc_en << SSC_EN_SHIFT);
	reg |= val;
	writel(reg, priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_0);
}

unsigned int ssc_stepsize_get(struct xr_usb_combophy_priv *priv)
{
	struct ssc_step_size_param *p;
	unsigned int val = 0;
	unsigned int reg;

	p = (struct ssc_step_size_param *)&val;
	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_1);
	p->mplla_ssc_step_size_15_0 = (reg & MPLLA_SSC_STEP_SIZE_15_0_MASK) >>
				MPLLA_SSC_STEP_SIZE_15_0_SHIFT;
	reg = readl(priv->ssphy + SUP_DIG_MPLLA_SSC_OVRD_IN_0);
	p->mplla_ssc_step_size_20_16 = (reg & MPLLA_SSC_STEP_SIZE_20_16_MASK) >>
				MPLLA_SSC_STEP_SIZE_20_16_SHIFT;
	p->ssc_ovrd_en = (reg & SSC_OVRD_EN_MASK) >> SSC_OVRD_EN_SHIFT;
	p->ssc_en = (reg & SSC_EN_MASK) >> SSC_EN_SHIFT;

	return val;
}

void eye_cusor_set(struct xr_usb_combophy_priv *priv, const void *param)
{
	const struct cursor_eye_param *p = param;
	int lane;
	unsigned int val = 0;
	unsigned int reg;
	bool orientation;

	orientation = orientation_get(priv);

	if (orientation == ORIG_NORMAL)
		lane = PARAM_SET_LANE0;
	else
		lane = PARAM_SET_LANE3;

	reg = readl(priv->ssphy + LANEN_DIG_ANA_XF_TX_STAT_EQ_OVRD_0(lane));
	reg &= ~TX_ANA_EQ_LFPS_OVRD_EN_MASK;
	writel(reg, priv->ssphy + LANEN_DIG_ANA_XF_TX_STAT_EQ_OVRD_0(lane));

	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_4(lane));
	reg &= ~TX_PRE_CURSOR_MASK;
	val = p->tx_pre_cusor << TX_PRE_CURSOR_SHIFT;
	reg |= val;
	writel(reg, priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_4(lane));

	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_3(lane));
	reg &= ~(TX_MAIN_CURSOR_MASK | TX_POST_CURSOR_MASK | TX_EQ_OVRD_EN_MASK);
	val = (p->tx_main_cusor << TX_MAIN_CURSOR_SHIFT) |
	      (p->tx_post_cusor << TX_POST_CURSOR_SHIFT) |
	      (p->tx_eq_ovrd_en << TX_EQ_OVRD_EN_SHIFT);
	reg |= val;
	writel(reg, priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_3(lane));
}

unsigned int eye_cusor_get(struct xr_usb_combophy_priv *priv)
{
	struct cursor_eye_param *p;
	int lane;
	unsigned int val = 0;
	unsigned int reg;
	bool orientation;

	orientation = orientation_get(priv);

	if (orientation == ORIG_NORMAL)
		lane = PARAM_SET_LANE0;
	else
		lane = PARAM_SET_LANE3;

	p = (struct cursor_eye_param *)&val;
	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_4(lane));
	p->tx_pre_cusor = (reg & TX_PRE_CURSOR_MASK) >> TX_PRE_CURSOR_SHIFT;

	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_3(lane));
	p->tx_main_cusor = (reg & TX_MAIN_CURSOR_MASK) >> TX_MAIN_CURSOR_SHIFT;
	p->tx_post_cusor = (reg & TX_POST_CURSOR_MASK) >> TX_POST_CURSOR_SHIFT;
	p->tx_eq_ovrd_en = (reg & TX_EQ_OVRD_EN_MASK) >> TX_EQ_OVRD_EN_SHIFT;

	return val;
}

void eye_boost_set(struct xr_usb_combophy_priv *priv, const void *param)
{
	const struct boost_eye_param *p = param;
	int lane;
	unsigned int val = 0;
	unsigned int reg;
	bool orientation;

	orientation = orientation_get(priv);

	if (orientation == ORIG_NORMAL)
		lane = PARAM_SET_LANE0;
	else
		lane = PARAM_SET_LANE3;

	reg = readl(priv->ssphy + SUP_DIG_LVL_OVRD_IN);
	reg &= ~(TX_VBOOST_LVL_MASK | TX_VBOOST_LVL_OVRD_EN_MASK);
	val = (p->tx_vboost_lvl << TX_VBOOST_LVL_SHIFT) |
	      (p->tx_vboost_lvl_ovrd_en << TX_VBOOST_LVL_OVRD_EN_SHIFT);
	reg |= val;
	writel(reg, priv->ssphy + SUP_DIG_LVL_OVRD_IN);

	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_2(lane));
	reg &= ~(TX_IBOOST_LVL_MASK | TX_VBOOST_EN_MASK | TX_OVRD_EN_MASK);
	val = (p->tx_iboost_lvl << TX_IBOOST_LVL_SHIFT) |
	      (p->tx_vboost_en << TX_VBOOST_EN_SHIFT) |
	      (p->tx_ovrd_en << TX_OVRD_EN_SHIFT);
	reg |= val;
	writel(reg, priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_2(lane));
}

unsigned int eye_boost_get(struct xr_usb_combophy_priv *priv)
{
	struct boost_eye_param *p;
	int lane;
	unsigned int val = 0;
	unsigned int reg;
	bool orientation;

	orientation = orientation_get(priv);

	if (orientation == ORIG_NORMAL)
		lane = PARAM_SET_LANE0;
	else
		lane = PARAM_SET_LANE3;

	p = (struct boost_eye_param *)&val;
	reg = readl(priv->ssphy + SUP_DIG_LVL_OVRD_IN);
	p->tx_vboost_lvl = (reg & TX_VBOOST_LVL_MASK) >> TX_VBOOST_LVL_SHIFT;
	p->tx_vboost_lvl_ovrd_en = (reg & TX_VBOOST_LVL_OVRD_EN_MASK) >>
				TX_VBOOST_LVL_OVRD_EN_SHIFT;

	reg = readl(priv->ssphy + LANEN_DIG_ASIC_TX_OVRD_IN_2(lane));
	p->tx_iboost_lvl = (reg & TX_IBOOST_LVL_MASK) >> TX_IBOOST_LVL_SHIFT;
	p->tx_vboost_en = (reg & TX_VBOOST_EN_MASK) >> TX_VBOOST_EN_SHIFT;
	p->tx_ovrd_en = (reg & TX_OVRD_EN_MASK) >> TX_OVRD_EN_SHIFT;

	return val;
}

void (*combophy_eye_set_func[])(struct xr_usb_combophy_priv *, const void *) = {
	ssc_peak_set,
	ssc_stepsize_set,
	eye_cusor_set,
	eye_boost_set,
};

unsigned int (*combophy_eye_get_func[])(struct xr_usb_combophy_priv *) = {
	ssc_peak_get,
	ssc_stepsize_get,
	eye_cusor_get,
	eye_boost_get,
};

static const char * const ssc_peak_regname[] = {
	"[15:0]MPLLA_SSC_PEAK_15_0",
	"[19:16]MPLLA_SSC_PEAK_19_16",
	NULL,
};

static const char * const ssc_stepsize_regname[] = {
	"[15:0]MPLLA_SSC_STEP_SIZE_15_0",
	"[20:16]MPLLA_SSC_STEP_SIZE_20_16",
	"[30]MPLLA_SSC_OVRD_EN",
	"[31]MPLLA_SSC_EN",
	NULL,
};

static const char * const cursor_eye_param_regname[] = {
	"[6:0]LANE_TX_PRE_CURSOR",
	"[13:7]LANE_TX_MAIN_CURSOR",
	"[20:14]LANE_TX_POST_CURSOR",
	"[31]LANE_TX_EQ_QVRD_EN",
	NULL,
};

static const char * const boost_eye_param_regname[] = {
	"[2:0]TX_VBOOST_LVL",
	"[6:3]LANE_TX_IBOOST_LVL",
	"[29]TX_VBOOST_LVL_OVRD_EN",
	"[30]TX_VBOOST_EN",
	"[31]TX_OVRD_EN",
	NULL,
};

static const char *const *xr_usb_combophy_eye_param_name_array[] = {
	&ssc_peak_regname[0],
	&ssc_stepsize_regname[0],
	&cursor_eye_param_regname[0],
	&boost_eye_param_regname[0],
};

static void combophy_param_form(struct seq_file *s)
{
	int index;
	const char *const *src = NULL;

	seq_puts(s, "***************************************\n");
	for (index = 0; index < COMBOPHY_EYE_PARAM_MAX; index++) {
		seq_printf(s, "parameter group[%d]\n", index);
		src = xr_usb_combophy_eye_param_name_array[index];

		for (; *src != NULL; src++)
			seq_printf(s, "    %s\n", *src);
	}
	seq_puts(s, "***************************************\n");
}

static void write_combophy_eye_param(struct xr_usb_combophy_priv *priv,
		int index, const unsigned int *value)
{
	if (priv->is_fpga) {
		dev_info(priv->dev, "combophy set eye param[%d]:0x%x\n", index, *value);
		return;
	}

	combophy_eye_set_func[index](priv, value);
}

static int read_combophy_eye_param(struct xr_usb_combophy_priv *priv,
		int index, unsigned int *val)
{
	if (priv->is_fpga) {
		dev_info(priv->dev, "fpga no phy to get!\n");
		return -ENOENT;
	}

	*val = combophy_eye_get_func[index](priv);
	return 0;
}

static int combophy_set_eye_param(struct xr_usb_combophy *combophy_data,
		int index, const unsigned int *value)
{
	struct xr_usb_combophy_priv *priv =
		container_of(combophy_data, struct xr_usb_combophy_priv, combophy);
	struct phy *phy = NULL;

	if (index >= COMBOPHY_EYE_PARAM_MAX) {
		dev_err(priv->dev, "no valid string matched to set\n");
		return -EINVAL;
	}

	phy = priv->combophy.phy;
	if (!phy)
		return -EFAULT;

	mutex_lock(&phy->mutex);
	if (phy->power_count > 0)
		write_combophy_eye_param(priv, index, value);

	mutex_unlock(&phy->mutex);

	return 0;
}

static int combophy_get_eye_param(struct xr_usb_combophy *combophy_data,
		int index, unsigned int *val)
{
	struct xr_usb_combophy_priv *priv =
		container_of(combophy_data, struct xr_usb_combophy_priv, combophy);
	struct phy *phy = NULL;
	int ret = 0;

	if (index >= COMBOPHY_EYE_PARAM_MAX) {
		dev_err(priv->dev, "no valid string matched to get\n");
		return -EINVAL;
	}

	phy = priv->combophy.phy;
	if (!phy)
		return -EINVAL;

	mutex_lock(&phy->mutex);
	if (phy->power_count <= 0)
		ret = -ENOENT;
	else
		ret = read_combophy_eye_param(priv, index, val);

	mutex_unlock(&phy->mutex);

	return ret;
}

static int xr_usb_combophy_core_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	struct xr_usb_combophy_priv *priv = phy_get_drvdata(phy);

	dev_info(priv->dev, "combophy_core_set_mode\n");
	return 0;
}

static const struct phy_ops xr_usb_combophy_core_ops = {
	.set_mode = xr_usb_combophy_core_set_mode,
	.owner = THIS_MODULE,
};

static int wait_phy_status_init(struct xr_usb_combophy_priv *priv)
{
	unsigned int val;

	return read_poll_timeout(readl, val, (val & UPCS_PHY_STATUS) !=
				 UPCS_PHY_STATUS, PHYSTATUS_WAIT_INTERVAL,
				 PHYSTATUS_WAIT_INTERVAL * PHYSTATUS_LOOP_NUM,
				 false, priv->tca + TCA_PSTATE_0);
}

static int phy_toggle_mode(struct xr_usb_combophy *combophy_data,
	unsigned int usbdp_mode)
{
	struct xr_usb_combophy_priv *priv =
		container_of(combophy_data, struct xr_usb_combophy_priv, combophy);

	unsigned int val;
	int ret;

	dev_info(priv->dev, "phy toggle mode\n");
	if (usbdp_mode > USBDP_MODE) {
		dev_err(priv->dev, "unknown combophy mode\n");
		return -EINVAL;
	}

	if (priv->is_fpga)
		return 0;

	/* wait combophy init done */
	ret = wait_phy_status_init(priv);
	if (ret) {
		dev_err(priv->dev, "phy status init done timeout\n");
		return ret;
	}

	/* TCA NC mode settting  */
	val = readl(priv->tca + TCA_INTR_EN);
	val |= (BIT(XA_ACK_EVT_EN) | BIT(XA_TIMEOUT_EVT_EN));
	writel(val, (priv->tca + TCA_INTR_EN));

	ret = read_poll_timeout(readl, val, (val & TCA_EVT_STATUS),
				TCA_EVT_WAIT_INTERVAL, TCA_EVT_WAIT_INTERVAL *
				TCA_EVT_LOOP_NUM, false, priv->tca +
				TCA_INTR_STS);
	if (ret) {
		dev_err(priv->dev, "TCA NC mode software timeout\n");
		return ret;
	} else if (val & BIT(XA_TIMEOUT_EVT)) {
		dev_err(priv->dev, "TCA NC mode event timeout\n");
		val = BIT(XA_TIMEOUT_EVT);
		writel(val, priv->tca + TCA_INTR_STS);
		return -ETIMEDOUT;
	}

	val = BIT(XA_ACK_EVT);
	writel(val, priv->tca + TCA_INTR_STS);
	dev_info(priv->dev, "phy toggle to NC\n");

	if (usbdp_mode == USB_MODE) {
		val = readl(priv->tca + TCA_CTRLSSYNC_DBG2);
		val |= (DPALT_DIS_CTRL_VAL | DPALT_DIS_CTRL_EN);
		writel(val, priv->tca + TCA_CTRLSSYNC_DBG2);
	}

	/* Toggle to disired mode */
	val = readl(priv->tca + TCA_TCPC);
	val &= ~(TCPC_MUX_CONTROL_MASK |
		 TCPC_CONN_ORIENTAION_MASK |
		 TCPC_VALID_MASK);
	if (is_invert_set(combophy_data) || orientation_get(priv))
		val |= BIT(TCPC_CONN_ORIENTAION_SHIFT);
	val |= (usbdp_mode << TCPC_MUX_CONTROL_SHIFT) |
		BIT(TCPC_VALID_SHIFT);
	writel(val, priv->tca + TCA_TCPC);

	if (usbdp_mode >= DP_MODE) {
		ret = read_poll_timeout(readl, val, !(val & DPALT_DIS),
					TCA_DP_INTERVAL, TCA_DP_INTERVAL *
					TCA_DP_LOOP, false, priv->tca +
					TCA_CTRLSSYNC_DBG0);
		if (ret)
			dev_err(priv->dev, "DP loop timeout\n");

		val = readl(priv->tca + TCA_CTRLSSYNC_DBG2);
		val &= ~DPALT_DIS_CTRL_VAL;
		val |= DPALT_DIS_CTRL_EN;
		writel(val, priv->tca + TCA_CTRLSSYNC_DBG2);
	}

	ret = read_poll_timeout(readl, val, (val & TCA_EVT_STATUS),
				TCA_EVT_WAIT_INTERVAL, TCA_EVT_WAIT_INTERVAL *
				TCA_EVT_LOOP_NUM, false, priv->tca +
				TCA_INTR_STS);
	if (ret) {
		dev_err(priv->dev, "TCA event software timeout\n");
		return ret;
	} else if (val & BIT(XA_TIMEOUT_EVT)) {
		dev_err(priv->dev, "TCA event timeout\n");
		val = BIT(XA_TIMEOUT_EVT);
		writel(val, priv->tca + TCA_INTR_STS);
		return -ETIMEDOUT;
	}

	val = BIT(XA_ACK_EVT);
	writel(val, priv->tca + TCA_INTR_STS);

	dev_info(priv->dev, "set combophy mode: %d\n", usbdp_mode);

	dev_info(priv->dev, "phy toggle success\n");
	return 0;
}

static int phy_state_lpcheck(struct xr_usb_combophy *combophy_data)
{
	struct xr_usb_combophy_priv *priv =
		container_of(combophy_data, struct xr_usb_combophy_priv, combophy);
	unsigned int val;
	int ret;

	ret = read_poll_timeout(readl, val,
		(((val & USB32_LANE0_PD_MASK) >> USB32_LANE0_PD_SHIFT == P4PG_STATE) &&
		((val & USB32_LANE1_PD_MASK) >> USB32_LANE1_PD_SHIFT == P4PG_STATE)),
		PD_WAIT_INTERVAL, PD_WAIT_INTERVAL *
		PD_LOOP_NUM, false, priv->tca +
		TCA_PSTATE_1);
	if (ret)
		dev_err(priv->dev, "combophy enter low power timeout\n");

	return ret;
}

int combophy_release(struct xr_usb_combophy_priv *priv)
{
	unsigned int val;

	val = readl(priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);
	val &= ~(unsigned int)USB_MODE_PHY0_SRAM_BYPASS_MODE_MASK;
	val |= (ROM_MODE << USB_MODE_PHY0_SRAM_BYPASS_MODE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);

	val = readl(priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);
	val &= ~(unsigned int)USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK;
	val |= (ROM_MODE << USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);

	val = BIT(USB_W1C_IP_RST_COMBOPHY_REG_SHIFT);
	writel(val, priv->usbctrl + USB_W1C);

	return 0;
}

int combophy_release_updatefw(struct xr_usb_combophy_priv *priv)
{
	unsigned int val;
	int ret;
	int i;
	spinlock_t lock;
	u64 t1, t2, t3, t4, d0, d1;

	spin_lock_init(&lock);
	val = readl(priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);
	val &= ~(unsigned int)USB_MODE_PHY0_SRAM_BYPASS_MODE_MASK;
	val |= (SRAM_MODE << USB_MODE_PHY0_SRAM_BYPASS_MODE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);

	val = readl(priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);
	val &= ~(unsigned int)USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK;
	val |= (SRAM_MODE << USB_MODE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);

	t1 = ktime_get_real_ns();
	spin_lock(&lock);
	/* release combophy */
	val = BIT(USB_W1C_IP_RST_COMBOPHY_REG_SHIFT);
	writel(val, priv->usbctrl + USB_W1C);

	/* set combophy timeout to 0xfffff */
	val = readl(priv->tca + TCA_CTRLSSYNC_CFG1);
	val |= XA_TIMEOUT_VAL_MASK;
	writel(val, priv->tca + TCA_CTRLSSYNC_CFG1);

	ret = read_poll_timeout_atomic(readl, val, (val &
			BIT(USB_PHY_SRAM_INIT_DONE_PHY_SRAM_INIT_DONE_SHIFT)),
			SRAM_INIT_WAIT_INTERVAL, SRAM_INIT_WAIT_INTERVAL *
			SRAM_INIT_LOOP_NUM, false, priv->usbctrl +
			USB_PHY_SRAM_INIT_DONE);
	if (ret) {
		spin_unlock(&lock);
		dev_err(priv->dev, "combophy sram init timeout\n");
		return ret;
	}
	spin_unlock(&lock);

	t3 = ktime_get_real_ns();
	/* update firmware */
	for (i = 0; i < ARRAY_SIZE(g_combophy_firmware); i++) {
		writel_relaxed(g_combophy_firmware[i],
		       priv->ssphy + COMBOPHY_APB_OFFSET(COMBOPHY_FW_START + i));
	}
	/* ensure write firmware completed */
	wmb();
	dev_info(priv->dev, "use writel_relaxed to update firmware\n");
	t4 = ktime_get_real_ns();

	udelay(1);
	spin_lock(&lock);
	val = readl(priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);
	val |= BIT(USB_MODE_PHY_SRAM_EXT_LD_DONE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_SRAM_EXT_DONE_BYPASS_MODE);

	/* set vbus valid */
	val = readl(priv->usbctrl + USB_USB_VBUS_VALID_CC_SEL);
	val |= BIT(USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT);
	writel(val, priv->usbctrl + USB_USB_VBUS_VALID_CC_SEL);

	/* release controller */
	val = BIT(USB_IP_W1S_IP_RST_CONTROLLER_REG_N_SHIFT);
	writel(val, priv->usbctrl + USB_IP_RST_CONTROLLER_REG_N_W1S);
	spin_unlock(&lock);
	t2 = ktime_get_real_ns();

	d0 = (t2 - t1);
	d1 = (t4 - t3);
	dev_info(priv->dev, "usb phy init total time: %lld ns\n", d0);
	dev_info(priv->dev, "usb update firmware time: %lld ns\n", d1);
	return ret;
}

static int combophy_enable_setting(struct xr_usb_combophy_priv *priv)
{
	int ret;
	unsigned int val;

	if (priv->is_fpga)
		return 0;

	/* wait combophy power enable */
	ret = read_poll_timeout(readl, val, (val & PHY_PWR_EN) == PHY_PWR_EN,
				HSS1_BUSIDLE_WAIT_INTERVAL, HSS1_BUSIDLE_WAIT_INTERVAL *
				HSS1_BUSIDLE_LOOP_NUM, false, priv->usbctrl +
				USB_PHY0_PCS_PWR_EN);
	if (ret) {
		dev_err(priv->dev, "combophy power status error\n");
		return ret;
	}

	/* set combophy power stable */
	val = readl(priv->usbctrl + USB_PHY0_PCS_PWR_STABLE);
	val |= (BIT(USB_STABLE_PHY0_PCS_PWR_STABLE_SHIFT) |
		BIT(USB_STABLE_UPCS_PWR_STABLE_SHIFT));
	writel(val, priv->usbctrl + USB_PHY0_PCS_PWR_STABLE);

	usleep_range(50, 100);

	/* set phy power present */
	val = readl(priv->usbctrl + USB_PHY_LANE0_POWER_PRESENT);
	val |= BIT(USB_PHY_LANE0_POWER_PRESENT_SHIFT);
	writel(val, priv->usbctrl + USB_PHY_LANE0_POWER_PRESENT);

	/* set phy apb0 if mode */
	val = readl(priv->usbctrl + USB_PHY0_APB0_IF_MODE);
	val &= ~(unsigned int)USB_PHY0_APB0_IF_MODE_PHY0_APB0_IF_MODE_MASK;
	val |= BIT(USB_PHY0_APB0_IF_MODE_PHY0_APB0_IF_MODE_SHIFT);
	writel(val, priv->usbctrl + USB_PHY0_APB0_IF_MODE);

	ret = combophy_release_updatefw(priv);
	if (ret) {
		dev_err(priv->dev, "combophy release error\n");
		return ret;
	}

	return 0;
}

static int combophy_init(struct phy *phy)
{
	struct xr_usb_combophy *data_combophy = phy_get_drvdata(phy);
	struct xr_usb_combophy_priv *priv =
		container_of(data_combophy, struct xr_usb_combophy_priv, combophy);
	unsigned int val;
	int ret;

	dev_info(priv->dev, "enter\n");

	if (!orientation_get(priv) && is_invert_set(data_combophy)) {
		val = readl(priv->usbctrl + USB_TYPEC_FLIP_INVERT);
		val &= ~USB_TYPEC_FLIP_INVERT_TYPEC_FLIP_INVERT_MASK;
		val |= BIT(USB_TYPEC_FLIP_INVERT_TYPEC_FLIP_INVERT_SHIFT);
		writel(val, priv->usbctrl + USB_TYPEC_FLIP_INVERT);
	}

	ret = combophy_enable_setting(priv);
	if (ret) {
		dev_err(priv->dev, "combophy enable setting fail\n");
		return ret;
	}

	return 0;
}

static void combophy_disable_setting(struct xr_usb_combophy_priv *priv)
{
	unsigned int val;

	if (priv->is_fpga)
		return;

	val = BIT(USB_W1S_IP_RST_COMBOPHY_REG_SHIFT);
	writel(val, priv->usbctrl + USB_IP_RST_COMBOPHY_REG_W1S);
}

static int combophy_exit(struct phy *phy)
{
	struct xr_usb_combophy *data_combophy = phy_get_drvdata(phy);
	struct xr_usb_combophy_priv *priv =
		container_of(data_combophy, struct xr_usb_combophy_priv, combophy);

	dev_info(priv->dev, "enter\n");

	combophy_disable_setting(priv);
	data_combophy->mode = PHY_MODE_INVALID;
	data_combophy->submode.data = 0;

	return 0;
}

static int combophy_poweron(struct phy *phy)
{
	dev_info(&phy->dev, "power on\n");
	return 0;
}

static int combophy_poweroff(struct phy *phy)
{
	dev_info(&phy->dev, "power off\n");
	return 0;
}

static int combophy_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	struct xr_usb_combophy *data_combophy = phy_get_drvdata(phy);

	dev_info(&phy->dev, "enter\n");
	data_combophy->mode = mode;
	data_combophy->submode.data = (unsigned int)submode;

	return 0;
}

static const struct phy_ops xr_usb_combophy_ops = {
	.owner = THIS_MODULE,
	.init = combophy_init,
	.exit = combophy_exit,
	.power_on = combophy_poweron,
	.power_off = combophy_poweroff,
	.set_mode = combophy_set_mode,
};

static struct phy *xr_usb_combophy_of_xlate(struct device *dev,
					 struct of_phandle_args *args)
{
	struct xr_usb_combophy_priv *priv = dev_get_drvdata(dev);
	unsigned int phy_id;
	struct phy *phy = NULL;

	if (!priv)
		return ERR_PTR(-ENOENT);

	if (args->args_count != XR_USB_COMBOPHY_CELL_SIZE)
		return ERR_PTR(-EINVAL);

	phy_id = args->args[XR_USB_COMBOPHY_CELL_ID];
	if (phy_id >= USB_COMBOPHY_IDX_MAX)
		return ERR_PTR(-EINVAL);

	switch (phy_id) {
	case USB_COMBOPHY_IDX_DWC3:
		if (priv->phy)
			phy = priv->phy;
		break;
	case USB_COMBOPHY_IDX_XR_USB:
		if (priv->combophy.phy)
			phy = priv->combophy.phy;
		break;
	default:
		dev_err(dev, "unknown combophy\n");
		break;
	}

	if (!phy) {
		dev_err(dev, "combophy register fail\n");
		return ERR_PTR(-EINVAL);
	}

	return phy;
}

static int xr_usb_combophy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phy *phy = NULL;
	struct phy_provider *phy_provider = NULL;
	struct xr_usb_combophy_priv *priv = NULL;

	dev_info(dev, "probe enter\n");
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;

	priv->usbctrl = devm_ioremap_resource_byname_no_req_region(pdev, "usbctrl");
	if (!priv->usbctrl) {
		dev_err(dev, "unable to remap \"usbctrl\" memory\n");
		return -ENOMEM;
	}

	priv->ssphy = devm_ioremap_resource_byname_no_req_region(pdev, "combo_phy");
	if (!priv->ssphy) {
		dev_err(dev, "unable to remap \"combo_phy\" memory\n");
		return -ENOMEM;
	}

	priv->tca = devm_ioremap_resource_byname_no_req_region(pdev, "tca");
	if (!priv->tca) {
		dev_err(dev, "unable to remap \"tca\" memory\n");
		return -ENOMEM;
	}

	/* get property */
	priv->is_fpga = device_property_read_bool(dev, "fpga");

	/* combophy for dwc3 core */
	phy = devm_phy_create(dev, NULL, &xr_usb_combophy_core_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);
	priv->phy = phy;
	phy_set_drvdata(phy, priv);

	/* combophy for xr-usb */
	phy = devm_phy_create(dev, NULL, &xr_usb_combophy_ops);
	if (IS_ERR(phy))
		return PTR_ERR(phy);
	priv->combophy.phy = phy;
	phy_set_drvdata(phy, &priv->combophy);

	priv->combophy.set_eye_param = combophy_set_eye_param;
	priv->combophy.get_eye_param = combophy_get_eye_param;
	priv->combophy.param_form = combophy_param_form;
	priv->combophy.toggle_mode = phy_toggle_mode;
	priv->combophy.lpcheck = phy_state_lpcheck;
	priv->combophy.mode = PHY_MODE_INVALID;
	priv->combophy.submode.data = 0;

	dev_set_drvdata(dev, priv);
	phy_provider = devm_of_phy_provider_register(dev, xr_usb_combophy_of_xlate);
	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id xr_usb_combophy_of_match[] = {
	{ .compatible = "xring,o1,usb-combophy" },
	{},
};
MODULE_DEVICE_TABLE(of, xr_usb_combophy_of_match);

static struct platform_driver
	xr_usb_combophy_driver = { .probe = xr_usb_combophy_probe,
				.driver = {
					.name = "xr-usb-combophy",
					.of_match_table = xr_usb_combophy_of_match,
				} };

int __init usb_combophy_init(void)
{
	int ret;

	ret = platform_driver_register(&xr_usb_combophy_driver);
	if (ret)
		pr_err("fail to register xr_usb_combophy");

	return ret;
}

void usb_combophy_exit(void)
{
	platform_driver_unregister(&xr_usb_combophy_driver);
}
