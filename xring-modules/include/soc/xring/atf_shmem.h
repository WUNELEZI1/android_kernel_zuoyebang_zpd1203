/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 * Description: atf shmem driver
 * Modify time: 2023-03-13
 */

#pragma once

#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

/* maybe can use NR_CPUS or set bigger value */
#define ATF_SHM_MAX_BUF_CNT  32

enum shmem_type {
	TYPE_IN = 0x5F171A06,
	TYPE_OUT = 0x00204081,
	TYPE_INOUT = TYPE_IN | TYPE_OUT,
};

enum shmem_status {
	STATUS_IDLE = 0x0,
	STATUS_BUSY = 0x1,
};

struct shm_info_t {
	void *vaddr;
	phys_addr_t paddr;
	u64 size;
	u64 buff_nums;
	u64 inited;
	/* the followings are per buff info */
	u64 per_size;
	u64 buff_status[ATF_SHM_MAX_BUF_CNT];
	spinlock_t buff_lock;
	/* for shm lock */
	spinlock_t init_lock;
} __aligned(8);

extern s32 smc_shm_mode(u64 fid, char *addr, size_t size, enum shmem_type shmem_type, u64 x4);
extern s32 get_atf_shm_size(u64 *size);
