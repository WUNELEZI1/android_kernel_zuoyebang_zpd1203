/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_PRINTER_H__
#define __MDR_PRINTER_H__

extern int g_mdr_debug_level;

#define pr_fmt(fmt)	"[xr_dfx][mdr]:%s:%d " fmt, __func__, __LINE__

#define MDR_PRINT_ERR(args...)    pr_err("<mdr fail>"args)
#define MDR_PRINT_START(args...)  pr_info(">>>enter\n")
#define MDR_PRINT_END(args...)    pr_info("<<<exit\n")

#define mdr_debug(level, x...)			 \
	do {				\
		if (g_mdr_debug_level >= (level)) \
			pr_info(x);	\
	} while (0)

#endif
