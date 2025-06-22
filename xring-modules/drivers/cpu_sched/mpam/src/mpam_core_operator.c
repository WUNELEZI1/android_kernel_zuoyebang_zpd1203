// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "mpam_internal.h"
#include "mpam_register.h"
#include "mpam_partid_map_mgr.h"
#include "mpam_user_interface.h"
#include <asm/sysreg.h>
#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/platform_device.h>

#define TAG "xring-mpam-core: "

static DEFINE_PER_CPU(bool, mpam_sys_enabled);
static DEFINE_PER_CPU(bool, mpam_sys_supported);

static inline void mpam1_write_core_partid_raw(u16 partid_I, u16 partid_D)
{
	u64 reg = 0;

	if (!this_cpu_read(mpam_sys_enabled))
		return;

	reg = read_sysreg_s(SYS_MPAM1_EL1);

	FIELD_SET(reg, MPAM_SYSREG_PARTID_I, partid_I);
	FIELD_SET(reg, MPAM_SYSREG_PARTID_D, partid_D);

	write_sysreg_s(reg, SYS_MPAM1_EL1);
}

static inline u8 mpam_sys_version(void)
{
	u8 major_version = 0;
	u8 minor_version = 0;
	u8 version = 0;

	major_version = FIELD_GET(MPAM_SYSREG_ARCH_MAJOR_REV, read_sysreg_s(SYS_ID_AA64PFR0_EL1));
	minor_version = FIELD_GET(MPAM_SYSREG_ARCH_MINOR_REV, read_sysreg_s(SYS_ID_AA64PFR1_EL1));

	version = (major_version << 4) | minor_version;

	return version;
}

static inline bool mpam_check_sys_enable(void)
{
	u64 reg = read_sysreg_s(SYS_MPAM1_EL1);

	return FIELD_GET(MPAM_SYSREG_EN, reg);
}

/* The raw MPAM register Write USE ISB at the end */
static inline void mpam0_write_core_partid_raw(u16 partid_I, u16 partid_D)
{
	u64 reg = 0;

	if (!this_cpu_read(mpam_sys_enabled))
		return;

	reg = read_sysreg_s(SYS_MPAM0_EL1);

	FIELD_SET(reg, MPAM_SYSREG_PARTID_I, partid_I);
	FIELD_SET(reg, MPAM_SYSREG_PARTID_D, partid_D);
	write_sysreg_s(reg, SYS_MPAM0_EL1);

}

/* Add the mpam write register before __switch_to memory barrier */
void mpam_write_core_partid(u16 partid_I, u16 partid_D, bool do_recover)
{
	u64 reg_old = 0;
	u64 reg_new = 0;

	if (do_recover) {
		if (!this_cpu_read(mpam_sys_supported))
			return;

		if (mpam_check_sys_enable()) {
			this_cpu_write(mpam_sys_enabled, true);
		} else {
			this_cpu_write(mpam_sys_enabled, false);
			return;
		}
	} else if (!this_cpu_read(mpam_sys_enabled)) {
		return;
	}

	reg_new = reg_old = read_sysreg_s(SYS_MPAM0_EL1);

	FIELD_SET(reg_new, MPAM_SYSREG_PARTID_I, partid_I);
	FIELD_SET(reg_new, MPAM_SYSREG_PARTID_D, partid_D);

	if (reg_new == reg_old)
		return;

	write_sysreg_s(reg_new, SYS_MPAM0_EL1);
}

static DEFINE_PER_CPU(u16, s_percpu_max_partid);

static void mpam_core_check(void *data)
{
	u64 idr = 0;

	if (!mpam_sys_version()) {
		pr_info(TAG "CPU %d not support MPAM\n", smp_processor_id());
		this_cpu_write(mpam_sys_supported, false);
		this_cpu_write(mpam_sys_enabled, false);
		this_cpu_write(s_percpu_max_partid, 0);
		return;
	}
	this_cpu_write(mpam_sys_supported, true);

	idr = read_sysreg_s(SYS_MPAMIDR_EL1);
	this_cpu_write(s_percpu_max_partid, FIELD_GET(MPAMIDR_PARTID_MAX, idr));

	if (mpam_check_sys_enable())
		this_cpu_write(mpam_sys_enabled, true);
	else
		this_cpu_write(mpam_sys_enabled, false);
}

int mpam_core_function_check(void)
{
	int cpu = 0;
	bool has_support_cpu = false;
	u16 cpu_max_partid = 0xffffU;

	on_each_cpu(mpam_core_check, NULL, true);

	for_each_possible_cpu(cpu) {
		if (!per_cpu(mpam_sys_enabled, cpu)) {
			pr_info(TAG "CPU %d is not enabled mpam\n", cpu);
		} else {
			has_support_cpu = true;
			cpu_max_partid =
				min(per_cpu(s_percpu_max_partid, cpu), cpu_max_partid);
		}
	}

	if (!has_support_cpu)
		return -ENODEV;

	mpam_update_max_partid(cpu_max_partid);
	return 0;
}

void mpam_reset_partid(void __always_unused *info)
{
	/* Reset MPAM0_EL1 */
	mpam0_write_core_partid_raw(MPAM_PARTID_RESET, MPAM_PARTID_RESET);
}
