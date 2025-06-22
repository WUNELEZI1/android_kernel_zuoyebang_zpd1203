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
#include "mdump.h"

static DEFINE_RAW_SPINLOCK(g_mdump_lock);
static struct mdump_head *g_mdump_head;
static u64 g_mem_paddr;
static void *g_mem_vaddr;
static void *g_logbuf_vaddr;

static int get_memdump_addr(void)
{
	g_mem_paddr = DFX_MEMDUMP_PARAM_ADDR;
	g_logbuf_vaddr = ioremap(DFX_LOGBUF_ADDR, DFX_LOGBUF_SIZE + DFX_MEMDUMP_PARAM_SIZE);
	if (!g_logbuf_vaddr) {
		pr_err("Unable to map I/O memory!\n");
		return -EINVAL;
	}

	g_mem_vaddr = g_logbuf_vaddr + DFX_LOGBUF_SIZE;
	return 0;
}

void *get_memdump_vaddr(void)
{
	return g_logbuf_vaddr;
}

int memdump_init(void)
{
	int ret;

	ret = get_memdump_addr();
	if (ret) {
		pr_err("get memdump addr failed\n");
		return ret;
	}

	ret = memdump_logbuf_init();
	if (ret) {
		pr_err("memdump logbuf init failed\n");
		return ret;
	}

	/* init head information */
	g_mdump_head = (struct mdump_head *)g_mem_vaddr;
	g_mdump_head->magic = MEMDUNMP_MAGIC;
	g_mdump_head->regs_info[0].mdump_id = MEMDUMP_HEAD;
	g_mdump_head->regs_info[0].size = DFX_MEMDUMP_PARAM_SIZE;
	g_mdump_head->regs_info[0].paddr = g_mem_paddr;
	g_mdump_head->nums = 1;

	pr_info("memdump init success\n");
	return 0;
}

static int check_memdump_id(u8 mdump_id)
{
	int index;

	for (index = 0; index < g_mdump_head->nums; index++) {
		if (mdump_id == g_mdump_head->regs_info[index].mdump_id)
			return -EINVAL;
	}

	return 0;
}

/*
 * size need 4k alignment
 * size_max	2*1024*1024
 * mdump_id_max 10
 * addr_max	0x840000000
 *
 */
int register_mdump(char mdump_id, unsigned int size, unsigned long addr)
{
	int i;

	if (mdump_id > MEMDUMP_MAX) {
		pr_err("mdump_id [%d] is over max_mdump_id\n", mdump_id);
		return -EINVAL;
	}

	if (size > MEMDUMP_MAX_SIZE) {
		pr_err("module is [%d], size is over max_size\n", mdump_id);
		return -EINVAL;
	}

	if (size & (SIZE_4K - 1)) {
		pr_err("module is [%d], size need 4k alignment\n", mdump_id);
		return -EINVAL;
	}

	if (addr > MEMDUMP_ADDR_MAX) {
		pr_err("module is [%d], addr is over max_addr\n", mdump_id);
		return -EINVAL;
	}

	if (addr + size > MEMDUMP_ADDR_MAX) {
		pr_err("module is [%d], addr + size is over max_addr\n", mdump_id);
		return -EINVAL;
	}

	if (check_memdump_id(mdump_id)) {
		pr_err("mdump_id: [%d] exit already\n", mdump_id);
		return -EINVAL;
	}

	raw_spin_lock(&g_mdump_lock);

	i = g_mdump_head->nums;
	if (i >= MEMDUMP_MAX) {
		pr_err("memdump data corruption(nums: %d)\n", i);
		raw_spin_unlock(&g_mdump_lock);
		return -EINVAL;
	}

	g_mdump_head->regs_info[i].mdump_id = mdump_id;
	g_mdump_head->regs_info[i].size = size;
	g_mdump_head->regs_info[i].paddr = addr;
	g_mdump_head->nums += 1;

	raw_spin_unlock(&g_mdump_lock);
	return 0;
}
EXPORT_SYMBOL(register_mdump);

void memdump_exit(void)
{
	pr_info("memdump exit!\n");
}
