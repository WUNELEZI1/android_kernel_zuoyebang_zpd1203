/* SPDX-License-Identifier: GPL-2.0 */
/*
 * XRING shrink slabd Feature
 *
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __LINUX_XRING_SHRINK_SLABD_H
#define __LINUX_XRING_SHRINK_SLABD_H

#ifdef CONFIG_XRING_KSHRINK_SLABD
int shrink_slabd_init(void);
void shrink_slabd_exit(void);
#endif

extern unsigned long shrink_slab(gfp_t gfp_mask, int nid,
				struct mem_cgroup *memcg,
				int priority);

#endif /* __LINUX_XRING_SHRINK_SLABD_H */
