/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __XSWAPD_GLOBAL_H
#define __XSWAPD_GLOBAL_H

#ifdef CONFIG_XRING_ZRAM_XSWAPD
void wakeup_xswapds(void);
#else
void wakeup_xswapds(void)
{
}
#endif
#endif
