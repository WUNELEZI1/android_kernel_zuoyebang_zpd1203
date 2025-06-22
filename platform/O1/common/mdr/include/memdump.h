/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _MEMDUMP_H_
#define _MEMDUMP_H_

#define MEMDUMP_MAX		10
#define MEMDUNMP_MAGIC          0xDEADBEEF

struct mdump_regs_info{
	unsigned long paddr;
	unsigned int size;
	unsigned char mdump_id;
};

struct mdump_head{
	unsigned int magic;
	unsigned int nums;
	struct mdump_regs_info regs_info[MEMDUMP_MAX];
};

struct mdump_info
{
	unsigned char mdump_id;
	char *mdump_name;
};

enum mdump_id
{
	STARTUP = 1,
};

int register_mdump(char mdump_id, unsigned int size, unsigned long addr);

#endif
