// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 *
 * Description: bootup track stage1 driver
 */

#define pr_fmt(fmt) "[xr_bootup][bootup track]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/ktime.h>
#include <soc/xring/xr_timestamp.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <dt-bindings/xring/platform-specific/common/bootup_track/bootup_track_common.h>

static void *g_bootup_track_mem_addr;

static int __init bootup_track_stage1_init(void)
{
	struct bootup_track_data *track_data;
	ktime_t kernel_init_timestamp;

	g_bootup_track_mem_addr =
		ioremap_wc(DFX_MEM_BOOT_INFO_ADDR, DFX_MEM_BOOT_INFO_SIZE);

	if (!g_bootup_track_mem_addr) {
		pr_err("bootup_track_mem ioremap error\n");
		return -ENOMEM;
	}

	track_data = (struct bootup_track_data *)g_bootup_track_mem_addr;
	kernel_init_timestamp = xr_timestamp_gettime();

	/* timestamp is nsec, convert to msec */
	track_data->boot_time[STAGE_KERNEL] =
		(u64)kernel_init_timestamp / NSEC_PER_MSEC;
	pr_info("record kernel time: %llu\n",
		track_data->boot_time[STAGE_KERNEL]);

	return 0;
}

static void __exit bootup_track_stage1_exit(void)
{
	iounmap(g_bootup_track_mem_addr);
	g_bootup_track_mem_addr = NULL;
	pr_info("remove bootup_stage1.ko");
}

module_init(bootup_track_stage1_init);
module_exit(bootup_track_stage1_exit);

MODULE_LICENSE("GPL");
