/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __INCLUDE_MPAM_BASE_H_
#define __INCLUDE_MPAM_BASE_H_

#include "soc/xring/securelib/securec.h"
#include <linux/bitfield.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#define MPAM_PARTID_RESET 0

/* Depend on how long the cgroup path file length */
#define MPAM_STRING_MAX 50U

/* Bits for mpam_features_t */
enum mpam_device_features {
	mpam_feat_ccap_part = 0,
	mpam_feat_cpor_part,
	mpam_feat_mbw_part,
	mpam_feat_mbw_min,
	mpam_feat_mbw_max,
	mpam_feat_mbw_prop,
	mpam_feat_intpri_part,
	mpam_feat_intpri_part_0_low,
	mpam_feat_dspri_part,
	mpam_feat_dspri_part_0_low,
	mpam_feat_msmon,
	mpam_feat_msmon_csu,
	mpam_feat_msmon_csu_capture,
	/*
	 * Having mpam_feat_msmon_mbwu set doesn't mean the regular 31 bit MBWU
	 * counter would be used. The exact counter used is decided based on the
	 * status of mpam_feat_msmon_mbwu_l/mpam_feat_msmon_mbwu_lwd as well.
	 */
	mpam_feat_msmon_mbwu,
	mpam_feat_msmon_mbwu_44counter,
	mpam_feat_msmon_mbwu_63counter,
	mpam_feat_msmon_mbwu_capture,
	mpam_feat_msmon_mbwu_rwbw,
	mpam_feat_msmon_capt,
	mpam_feat_partid_nrw,
	MPAM_FEATURE_LAST,
};

#define mpam_has_feature(_feat, x) (test_bit(_feat, (x)->features))
#define mpam_set_feature(_feat, x) (set_bit(_feat, (x)->features))
#define mpam_clear_feature(x) (bitmap_zero((x)->features, MPAM_FEATURE_LAST))

struct mpam_props {
	DECLARE_BITMAP(features, MPAM_FEATURE_LAST);

	u16 cpbm_wd;
	u16 cpbm_reg_count;
	u16 mbw_pbm_bits;
	u8 bwa_wd;
	u16 cmax_wd;
	u16 intpri_wd;
	u16 dspri_wd;
	u16 num_csu_mon;
	u16 num_mbwu_mon;
};

union mpam_error_state {
	struct error_info {
		u64 parid_mon : 16;
		u64 pmg : 8;
		u64 errcode : 4;
		u64: 3;
		u64 ovrwr : 1;
		u64 ris : 4;
		u64: 28;
	} info;
	u64 reg;
};

struct mpam_msc {
	int id;
	struct platform_device *pdev;
	struct mutex lock;

	bool has_extd_esr;
	u16 partid_max;
	u8 pmg_max;

	u64 idr;

	/* MPAM Error State Register */
	union mpam_error_state error_state;

	struct mpam_props prop;

	/*
	 * part_sel_lock protects access to the MSC hardware registers that are
	 * affected by MPAMCFG_PART_SEL. (including the ID registers)
	 */
	spinlock_t part_sel_lock;

	void __iomem *mapped_hwpage;
	size_t mapped_hwpage_sz;

	bool probed;
	bool config_lock;
};

#define mpam_msc_dev(x) (&x->pdev->dev)

#define FIELD_SET(reg, field, val)                                             \
	(reg = (reg & ~field) | FIELD_PREP(field, val))

#endif /* __INCLUDE_MPAM_BASE_H_ */
