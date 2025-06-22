/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023-2025 XRing Technologies Co., Ltd.
 */
#ifndef __XR_DSE_H__
#define __XR_DSE_H__

#define SYSREG_ID(reg)  SYSREG_ID##reg
enum sysreg_id {
	SYSREG_ID(CPUPPMCR),
	SYSREG_ID(CPUMPMMCR),
	SYSREG_ID(CPUPPMPDPCR),
	SYSREG_ID(CPUMPMMTUNE),
	SYSREG_ID(CPUPDPTUNE),
	SYSREG_ID(CPUPDPTUNE2),
	SYSREG_ID(CPUECTLR),
	SYSREG_ID(CPUECTLR2),
	SYSREG_ID(CMPXECTLR),
	SYSREG_NUM,
};

u64 read_dse_sysreg(int cpu_id, enum sysreg_id id);
void write_dse_sysreg(int cpu_id, enum sysreg_id id, u64 val);

#endif
