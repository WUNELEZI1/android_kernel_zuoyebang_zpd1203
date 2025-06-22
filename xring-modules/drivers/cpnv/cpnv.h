/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#ifndef __CPNV_H__
#define __CPNV_H__

#include <linux/types.h>
#include <linux/module.h>
#include "dt-bindings/xring/platform-specific/cpnv/cpnv_plat.h"
#include "dt-bindings/xring/platform-specific/common/cpnv/cpnv_hw.h"
#include "dt-bindings/xring/platform-specific/common/cpnv/cpnv_interface.h"

#define CPNV_DIY_NOT_ENABLE		1
#define CPNV_BASE_PROFILE_INVALID	(-1)

/* cpufreq use kHz */
#define DTB_CPU_FREQ_SCALE 1000
#define DTB_CPU_VOLT_SCALE 1000
/* devfreq use Hz */
#define DTB_GPU_FREQ_SCALE 1000
#define DTB_GPU_VOLT_SCALE 1000

struct cpnv_system_status {
	bool overclock_enable;
	uint8_t test_status;
	uint8_t efuse_sim;
	bool need_reboot;
	uint32_t volt_min_step;
};

struct cpnv_profile_full {
	struct device_profile profile;
	bool is_modified;
	bool need_test;
	bool is_turbo;
	int32_t base_profile_id;
	uint32_t base_volt;
};

struct cpnv_device_profile_list {
	uint8_t device_id;
	bool is_enabled;
	const char *device_name;
	uint32_t diy_new_profile_cnt;
	uint32_t base_profile_cnt;
	/* all_profile_cnt = base_profile_cnt + diy_new_profile_cnt */
	uint32_t all_profile_cnt;
	uint32_t min_freq;
	uint32_t max_freq;
	/* profiles after add diy profiles */
	struct cpnv_profile_full real_profiles[CPNV_MAX_PROFILE_NUM];
	/* system base profiles */
	struct device_profile base_profiles[CPNV_MAX_PROFILE_NUM];
};

struct cpnv_parse_ops {
	int (*nv_write)(uint8_t *data, uint32_t len);
	int (*nv_read)(uint8_t *data, uint32_t len);
};
struct cpnv_system {
	struct cpnv_system_status status;
	struct cpnv_parse_ops nv_ops;
	struct cpnv_device_profile_list *profile_lists;
	uint8_t	device_cnt;
};

static inline bool profile_is_added(struct cpnv_profile_full *profile)
{
	return profile->base_profile_id == CPNV_BASE_PROFILE_INVALID;
}

/*
 * Before calling this function
 * MAKE SURE Real profile list has been initialized
 * as DTB profile list
 */
int cpnv_parse_recover(struct cpnv_system *cpnv);
/*
 * Only Call in Reboot process
 */
int cpnv_update(struct cpnv_system *cpnv);

#ifndef CPNV_ERROR
#define CPNV_ERROR(args...) pr_err(args)
#endif

#ifndef CPNV_INFO
#define CPNV_INFO(args...) pr_info(args)
#endif

#ifndef CPNV_DEBUG
#define CPNV_DEBUG(args...) pr_devel(args)
#endif

#ifndef E_CPNV_NV_FORMAT
#define E_CPNV_NV_FORMAT EINVAL
#endif

#ifndef E_CPNV_NO_SPACE
#define E_CPNV_NO_SPACE ENOSPC
#endif

#endif /* __CPNV_H__ */
