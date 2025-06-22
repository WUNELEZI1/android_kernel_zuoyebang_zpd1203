/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/memdump.h>

#ifndef _MDUMP_H_
#define _MDUMP_H_

#define MEMDUMP_MAX		10
#define MEMDUMP_MAX_SIZE	(2*1024*1024)
#define MEMDUMP_HEAD		0
#define MEMDUMP_ADDR_MAX	0x840000000
#define SIZE_4K			4096

int memdump_logbuf_init(void);
void *get_memdump_vaddr(void);

#endif
