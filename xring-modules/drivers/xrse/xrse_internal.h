/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRSE_INTERNEL__
#define __XRSE_INTERNEL__

#include <linux/types.h>
#include <linux/printk.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>

typedef int (*cmd_handle_t)(int argc, char **argv);

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)  (sizeof(x) / sizeof((x)[0]))
#endif

#define xrse_err(fmt, args...)           pr_err("[xrse]" fmt, ##args)
#define xrse_warn(fmt, args...)          pr_warn("[xrse]" fmt, ##args)
#define xrse_info(fmt, args...)          pr_info("[xrse]" fmt, ##args)
#define xrse_debug(fmt, args...)         pr_debug("[xrse]" fmt, ##args)

struct module_case {
	char *name;
	cmd_handle_t store_handle;
};

struct xrse_module_cmd {
	char *module_name;
	struct module_case *cases;
	int num_cases;
};

#if IS_ENABLED(CONFIG_XRING_XRSE_RIP)
int kernel_rodata_rip_request(void);
void xrse_rip_unpack_status(struct rip_measure_status_str *rip_sta, u64 *reg_arr);
#else
static inline int kernel_rodata_rip_request(void) { return 0; }
static inline void xrse_rip_unpack_status(struct rip_measure_status_str *rip_sta, u64 *reg_arr) {};
#endif

bool is_xrse_ready(void);

#endif
