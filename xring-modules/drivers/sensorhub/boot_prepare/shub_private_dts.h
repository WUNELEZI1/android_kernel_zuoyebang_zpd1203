// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SHUB_PRIVATE_DTS__
#define __SHUB_PRIVATE_DTS__

#include <dt-bindings/xring/platform-specific/sensorhub_dts.h>

int shub_run_bsp_dts_cb(void);
void shub_shm_addr_map(void);
void shub_shm_addr_unmap(void);
int shub_parse_aoc_dts(struct device_node *parent, struct sh_bsp_dts *dts_mem_block);

#endif
