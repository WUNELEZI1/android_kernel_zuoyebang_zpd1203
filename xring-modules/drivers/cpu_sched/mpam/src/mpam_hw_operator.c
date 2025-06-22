// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mpam_internal.h"
#include "mpam_partid_map_mgr.h"
#include "mpam_register.h"
#include <linux/cpuidle.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>

#define TAG "xring-mpam-msc: "

static const char *mpam_errcode_name[16] = { [0] = "No",
					     [1] = "PARTID_SEL_Range",
					     [2] = "Req_PARTID_Range",
					     [3] = "MSMONCFG_ID_Range",
					     [4] = "Req_PMG_Range",
					     [5] = "Monitor_Range",
					     [6] = "intPARTID_Range",
					     [7] = "Unexpected_INTERNAL",
					     [8] = "Undefined_RIS_PART_SEL",
					     [9] = "RIS_No_Control",
					     [10] = "Undefined_RIS_MON_SEL",
					     [11] = "RIS_No_Monitor",
					     [12 ... 15] = "Reserved" };
struct mpam_config {
	struct mpam_msc *msc;
	u16 reg;
	u32 val;
};

static inline u32 __mpam_read_reg(struct mpam_msc *msc, u16 reg)
{
	WARN_ON_ONCE((reg + sizeof(u32)) > msc->mapped_hwpage_sz);
	return readl(msc->mapped_hwpage + reg);
}

static inline void __mpam_write_reg(struct mpam_msc *msc, u16 reg, u32 val)
{
	WARN_ON_ONCE((reg + sizeof(u32)) > msc->mapped_hwpage_sz);
	writel(val, msc->mapped_hwpage + reg);
}

u64 mpam_msc_read_idr(struct mpam_msc *msc)
{
	u64 idr_high = 0;
	u64 idr_low = 0;

	idr_low = __mpam_read_reg(msc, MPAMF_IDR);
	if (FIELD_GET(MPAMF_IDR_HAS_EXT, idr_low))
		idr_high = __mpam_read_reg(msc, MPAMF_IDR + 4);

	return (idr_high << 32) | idr_low;
}

static void mpam_msc_zero_esr(struct mpam_msc *msc)
{
	__mpam_write_reg(msc, MPAMF_ESR, 0);
	if (msc->has_extd_esr)
		__mpam_write_reg(msc, MPAMF_ESR + 4, 0);
}

static inline void mpam_msc_read_esr(struct mpam_msc *msc)
{
	u64 esr_high = 0;
	u32 esr_low = 0;

	esr_low = __mpam_read_reg(msc, MPAMF_ESR);
	if (msc->has_extd_esr)
		esr_high = __mpam_read_reg(msc, MPAMF_ESR + 4);
	msc->error_state.reg = (esr_high << 32) | esr_low;
}

static void mpam_enable_msc_ecr(struct mpam_msc *msc)
{
	__mpam_write_reg(msc, MPAMF_ECR, 1U);
}

static void mpam_disable_msc_ecr(struct mpam_msc *msc)
{
	__mpam_write_reg(msc, MPAMF_ECR, 0);
}

static void __mpam_msc_disable_irq(struct mpam_msc *msc)
{
	mpam_disable_msc_ecr(msc);
	mpam_msc_zero_esr(msc);
}

static irqreturn_t mpam_irq_handler(int irq, void *dev_id)
{
	struct mpam_msc *msc = dev_id;

	if (IS_ERR_OR_NULL(dev_id))
		return IRQ_NONE;

	/* Only Get MPAMF_ESR register */
	mpam_msc_read_esr(msc);
	__mpam_msc_disable_irq(msc);
	return IRQ_WAKE_THREAD;
}

/*
 * Called in response to an error IRQ.
 * All of MPAMs errors indicate a software bug
 * close irq and turn off MPAM function
 */
static irqreturn_t mpam_disable_thread(int irq, void *dev_id)
{
	struct mpam_msc *msc = dev_id;
	struct error_info *info = &msc->error_state.info;

	mpam_msc_unregister_irq(msc);

	dev_err(mpam_msc_dev(msc), "MPAM Error: error code = %u %s\n",
		info->errcode, mpam_errcode_name[info->errcode]);
	dev_err(mpam_msc_dev(msc),
		"MPAM Error: pmg = %u, partid_mon = %u, ris = %u\n", info->pmg,
		info->parid_mon, info->ris);

	if (info->ovrwr)
		dev_err(mpam_msc_dev(msc),
			"MPAM Error: Multiple errors Occurred\n");

	if (info->errcode == MPAM_ERRCODE_NONE) {
		mpam_enable_msc_ecr(msc);
		/* NO Error Occurred */
		return IRQ_NONE;
	}

	/* lock DSU User Interface */
	msc->config_lock = true;

	/* Turn off CPU function */
	mpam_set_cpu_enable(false);

	return IRQ_HANDLED;
}

int mpam_msc_register_irqs(struct mpam_msc *msc)
{
	int err = 0;
	int irq = 0;
	struct device *dev = mpam_msc_dev(msc);

	lockdep_assert_held(&msc->lock);

	irq = platform_get_irq_byname_optional(msc->pdev, "error");
	if (irq <= 0) {
		dev_info(dev, "Do not get irq by name, errcode=%d\n", irq);
		return 0;
	}

	err = devm_request_threaded_irq(dev, irq, &mpam_irq_handler,
					&mpam_disable_thread, IRQF_SHARED,
					dev_name(dev), msc);
	if (err) {
		dev_err(dev, "Request IRQ Error, errcode=%d\n",
			err);
		return err;
	}

	mpam_enable_msc_ecr(msc);

	return 0;
}

void mpam_msc_unregister_irq(struct mpam_msc *msc)
{
	if (IS_ERR_OR_NULL(msc))
		pr_err(TAG "Input Error, %ld\n", PTR_ERR(msc));
	else
		__mpam_msc_disable_irq(msc);
}

static inline void __mpam_set_cfgs(struct mpam_msc *msc, const u16 regs,
				   const u32 val)
{
	cpuidle_pause_and_lock();

	spin_lock(&msc->part_sel_lock);
	if (!msc->config_lock)
		__mpam_write_reg(msc, regs, val);
	else
		dev_warn(mpam_msc_dev(msc), "Write after lock\n");

	spin_unlock(&msc->part_sel_lock);

	cpuidle_resume_and_unlock();
}

static inline u32 __mpam_get_cfgs(struct mpam_msc *msc, const u16 regs)
{
	u32 ret = 0;

	cpuidle_pause_and_lock();

	spin_lock(&msc->part_sel_lock);
	ret = msc->config_lock ? 0 : __mpam_read_reg(msc, regs);
	spin_unlock(&msc->part_sel_lock);

	cpuidle_resume_and_unlock();

	return ret;
}

int mpam_set_partid_sel(struct mpam_msc *msc, const u16 partid)
{
	u32 reg = 0;

	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "Input ptr error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}

	if (!mpam_partid_check(partid)) {
		dev_err(mpam_msc_dev(msc), "Input partid Err, max=%d, input=%u\n",
			mpam_get_max_partid(), partid);
		return -EINVAL;
	}

	cpuidle_pause_and_lock();

	spin_lock(&msc->part_sel_lock);
	if (!msc->config_lock) {
		reg = __mpam_read_reg(msc, MPAMCFG_PART_SEL);
		FIELD_SET(reg, MPAMCFG_PART_SEL_PARTID_SEL, partid);
		__mpam_write_reg(msc, MPAMCFG_PART_SEL, reg);
	}
	spin_unlock(&msc->part_sel_lock);

	cpuidle_resume_and_unlock();

	return 0;
}

int mpam_get_partid_sel(struct mpam_msc *msc, u16 *partid)
{
	u32 reg = 0;

	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "MSC ptr Error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}

	reg = __mpam_get_cfgs(msc, MPAMCFG_PART_SEL);
	*partid = FIELD_GET(MPAMCFG_PART_SEL_PARTID_SEL, reg);

	return 0;
}

static inline int mpam_cpor_part_check(struct mpam_msc *msc, const u32 val)
{
	u32 reg_mask = 0;

	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "MSC ptr Error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}
	reg_mask = GENMASK(msc->prop.cpbm_wd - 1, 0);
	if (val & (~reg_mask)) {
		dev_err(mpam_msc_dev(msc),
			"CPBM Input value error, input=0x%08x, mask=0x%08x\n",
			val, reg_mask);
		return -EINVAL;
	}

	return 0;
}

int mpam_set_cpor_part_config(struct mpam_msc *msc, const u32 val)
{
	int ret = 0;

	ret = mpam_cpor_part_check(msc, val);
	if (ret)
		return ret;

	__mpam_set_cfgs(msc, MPAMCFG_CPBM, val);

	return 0;
}

int mpam_get_cpor_part_config(struct mpam_msc *msc, u32 *reg)
{
	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "Input ptr Error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}

	*reg = __mpam_get_cfgs(msc, MPAMCFG_CPBM);

	return 0;
}

static inline int mpam_mbw_part_check(struct mpam_msc *msc, const u32 val)
{
	u32 reg_mask = 0;

	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "MSC ptr Error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}

	reg_mask = MPAMCFG_MBW_PROP_EN | GENMASK(msc->prop.bwa_wd - 1, 0);
	if (val & (~reg_mask)) {
		dev_err(mpam_msc_dev(msc),
			"MBW Part Input value error, input=0x%08x, mask=0x%08x\n",
			val, reg_mask);
		return -EINVAL;
	}

	return 0;
}

int mpam_set_mbw_part_config(struct mpam_msc *msc, u32 val)
{
	int ret = 0;

	ret = mpam_mbw_part_check(msc, val);
	if (ret)
		return ret;

	__mpam_set_cfgs(msc, MPAMCFG_MBW_PROP, val);

	return 0;
}

int mpam_get_mbw_part_config(struct mpam_msc *msc, u32 *reg)
{
	if (IS_ERR_OR_NULL(msc)) {
		pr_err(TAG "MSC ptr Error, ptr = %ld\n", (long)msc);
		return -ENODEV;
	}

	*reg = __mpam_get_cfgs(msc, MPAMCFG_MBW_PROP);

	return 0;
}

static void mpam_feature_config(struct mpam_msc *msc)
{
	struct mpam_props *props = &msc->prop;

	lockdep_assert_held(&msc->lock);

	mpam_clear_feature(props);
	/* Cache Capacity Partitioning */
	if (FIELD_GET(MPAMF_IDR_HAS_CCAP_PART, msc->idr)) {
		u32 ccap_features = __mpam_read_reg(msc, MPAMF_CCAP_IDR);

		props->cmax_wd =
			FIELD_GET(MPAMF_CCAP_IDR_CMAX_WD, ccap_features);
		if (props->cmax_wd)
			mpam_set_feature(mpam_feat_ccap_part, props);
	}

	/* Cache Portion partitioning */
	if (FIELD_GET(MPAMF_IDR_HAS_CPOR_PART, msc->idr)) {
		u32 cpor_features = __mpam_read_reg(msc, MPAMF_CPOR_IDR);

		props->cpbm_wd =
			FIELD_GET(MPAMF_CPOR_IDR_CPBM_WD, cpor_features);
		if (props->cpbm_wd) {
			mpam_set_feature(mpam_feat_cpor_part, props);
			/* Calculate How many cpbm regs the MPAM has */
			props->cpbm_reg_count = (props->cpbm_wd + 31U) / 32U;
		}
	}

	/* Memory bandwidth partitioning */
	if (FIELD_GET(MPAMF_IDR_HAS_MBW_PART, msc->idr)) {
		u32 mbw_features = __mpam_read_reg(msc, MPAMF_MBW_IDR);

		/* portion bitmap resolution */
		props->mbw_pbm_bits =
			FIELD_GET(MPAMF_MBW_IDR_BWPBM_WD, mbw_features);
		if (props->mbw_pbm_bits &&
		    FIELD_GET(MPAMF_MBW_IDR_HAS_PBM, mbw_features))
			mpam_set_feature(mpam_feat_mbw_part, props);

		props->bwa_wd = FIELD_GET(MPAMF_MBW_IDR_BWA_WD, mbw_features);
		if (props->bwa_wd &&
		    FIELD_GET(MPAMF_MBW_IDR_HAS_MAX, mbw_features))
			mpam_set_feature(mpam_feat_mbw_max, props);

		if (props->bwa_wd &&
		    FIELD_GET(MPAMF_MBW_IDR_HAS_MIN, mbw_features))
			mpam_set_feature(mpam_feat_mbw_min, props);

		if (props->bwa_wd &&
		    FIELD_GET(MPAMF_MBW_IDR_HAS_PROP, mbw_features))
			mpam_set_feature(mpam_feat_mbw_prop, props);
	}

	/* Priority partitioning */
	if (FIELD_GET(MPAMF_IDR_HAS_PRI_PART, msc->idr)) {
		u32 pri_features = __mpam_read_reg(msc, MPAMF_PRI_IDR);

		props->intpri_wd =
			FIELD_GET(MPAMF_PRI_IDR_INTPRI_WD, pri_features);
		if (props->intpri_wd &&
		    FIELD_GET(MPAMF_PRI_IDR_HAS_INTPRI, pri_features)) {
			mpam_set_feature(mpam_feat_intpri_part, props);
			if (FIELD_GET(MPAMF_PRI_IDR_INTPRI_0_IS_LOW,
				      pri_features))
				mpam_set_feature(mpam_feat_intpri_part_0_low,
						 props);
		}

		props->dspri_wd =
			FIELD_GET(MPAMF_PRI_IDR_DSPRI_WD, pri_features);
		if (props->dspri_wd &&
		    FIELD_GET(MPAMF_PRI_IDR_HAS_DSPRI, pri_features)) {
			mpam_set_feature(mpam_feat_dspri_part, props);
			if (FIELD_GET(MPAMF_PRI_IDR_DSPRI_0_IS_LOW,
				      pri_features))
				mpam_set_feature(mpam_feat_dspri_part_0_low,
						 props);
		}
	}
}

int mpam_msc_hw_probe(struct mpam_msc *msc)
{
	u64 aidr = 0;
	u64 idr = 0;
	u8 version = 0;

	lockdep_assert_held(&msc->lock);

	aidr = __mpam_read_reg(msc, MPAMF_AIDR);
	version =
		aidr & (MPAMF_AIDR_ARCH_MAJOR_REV | MPAMF_AIDR_ARCH_MINOR_REV);
	if (version == 0) {
		dev_err(mpam_msc_dev(msc),
			"MSC does not match MPAM architecture\n");
		return -ENODEV;
	}

	idr = mpam_msc_read_idr(msc);
	msc->partid_max = FIELD_GET(MPAMF_IDR_PARTID_MAX, idr);
	msc->pmg_max = FIELD_GET(MPAMF_IDR_PMG_MAX, idr);
	msc->idr = idr;

	mpam_feature_config(msc);

	msc->probed = true;

	mpam_update_max_partid(msc->partid_max);

	return 0;
}

int mpam_set_msc_lock(struct mpam_msc *msc)
{
	if (IS_ERR_OR_NULL(msc))
		return -EINVAL;
	if (!msc->config_lock) {
		msc->config_lock = true;
		dev_info(mpam_msc_dev(msc), "MSC Locked\n");
	}
	return 0;
}

int mpam_get_msc_lock(struct mpam_msc *msc)
{
	int lock_status = 0;

	if (IS_ERR_OR_NULL(msc))
		return -EINVAL;
	lock_status = msc->config_lock;
	return lock_status;
}
