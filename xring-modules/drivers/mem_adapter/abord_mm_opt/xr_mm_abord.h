/* SPDX-License-Identifier: GPL-2.0 */
/*
 * XRING MM ABORD Feature
 *
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __LINUX_XRING_MM_ABORD_H
#define __LINUX_XRING_MM_ABORD_H

#ifdef CONFIG_XRING_MM_ABORD
int abort_mm_opt_init(void);
void abort_mm_opt_exit(void);
#endif

#endif /* __LINUX_XRING_MM_ABORD_H */
