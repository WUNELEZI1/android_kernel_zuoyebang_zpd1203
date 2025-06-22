/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024, X-Ring Technologies, Inc.  All Rights Reserved
 * Description: bootup keypoint header
 */

#ifndef __BOOTUP_KEYPOINT_H__
#define __BOOTUP_KEYPOINT_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h"

int set_bootup_status(u32 value);
u32 get_bootup_status(void);
int bootup_keypoint_addr_init(void);

#endif /* __BOOTUP_KEYPOINT_H__ */
