// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: memory dump driver
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <trace/hooks/logbuf.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#include "printk_ringbuffer.h"
#include "mdump.h"

static void *g_mem_addr;
static char *g_logbuf_addr;
static size_t g_logbuf_size;

u64 get_logbuf_addr(void)
{
	return (u64)g_logbuf_addr;
}

size_t get_logbuf_size(void)
{
	return g_logbuf_size;
}

static void android_vh_logbuf_handler(void *unused, struct printk_ringbuffer *rb,
		struct printk_record *r)
{
	u64 phys_addr;

	if (g_logbuf_addr)
		return;

	if (!rb->text_data_ring.data) {
		pr_err("logbuf addr is null\n");
		return;
	}

	g_logbuf_addr = rb->text_data_ring.data;
	g_logbuf_size = _DATA_SIZE(rb->text_data_ring.size_bits);
	phys_addr = virt_to_phys(g_logbuf_addr);
	if (!phys_addr) {
		pr_err("Unable to get phy address\n");
		return;
	}

	memcpy(g_mem_addr, &phys_addr, sizeof(phys_addr));
}

int memdump_logbuf_init(void)
{
	int ret;

	g_mem_addr = get_memdump_vaddr();

	ret = register_trace_android_vh_logbuf(&android_vh_logbuf_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_vh_logbuf failed (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}
