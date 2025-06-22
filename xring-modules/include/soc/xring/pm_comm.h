/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#ifndef __PM_COMM_H__
#define __PM_COMM_H__

#include <soc/xring/vote_mng.h>
#include <dt-bindings/xring/vote_mng_define.h>

struct vote_mng_ptr {
	struct vote_mng *xctrl_shmem;
	struct vote_mng *lpctrl_shmem;
};

extern struct vote_mng_ptr g_vote_mng_ptr;

unsigned long sys_state_reg_read(unsigned long addr);
void sys_state_reg_write(unsigned long addr, unsigned long val);

void sys_state_votemng_xctrl_shmem(int flag);
void sys_state_votemng_lpctrl_shmem(int flag);

#endif
