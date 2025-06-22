/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _AMU_REG_
#define _AMU_REG_

#include <linux/io.h>
#include <linux/types.h>
#include <asm/sysreg.h>

#define AMCGCR_EL0			S3_3_C13_C2_2
#define AMU_GROUP0_NUM(reg_val)		(((reg_val) >> 0) & 0xFF)
#define AMU_GROUP1_NUM(reg_val)		(((reg_val) >> 8) & 0xFF)
#define AMEVCNTR20			0x200

static inline u64 read_amcgcr(void)
{
	return read_sysreg(AMCGCR_EL0);
}

u64 amu_group0_cnt_read(int idx);
u64 amu_group1_cnt_read(int idx);

static inline u64 amu_group2_cnt_read(void *base, int idx)
{
	void *addr = base + AMEVCNTR20 + 8 * idx;

	return ioread64(addr);
}

#endif // _AMU_REG_
