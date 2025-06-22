/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __MPAM_INTERNAL_H_
#define __MPAM_INTERNAL_H_

#include "mpam_base.h"
#include "mpam_hw_operator.h"

void mpam_write_core_partid(u16 partid_I, u16 partid_D, bool do_recover);
void mpam_reset_partid(void __always_unused *info);

int mpam_core_function_check(void);

int mpam_enable_once(void);
void mpam_disable(void);

int mpam_get_max_partid(void);
int mpam_update_max_partid(u16 device_max_partid);

bool mpam_partid_check(u16 partid);

#endif /* __MPAM_INTERNAL_H_ */
