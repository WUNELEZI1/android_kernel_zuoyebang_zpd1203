/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: dfx switch header
 */

#ifndef __DFX_SWITCH_H__
#define __DFX_SWITCH_H__

#include <linux/types.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>

bool is_switch_on(u32 switch_name);

#endif /* __DFX_SWITCH_H__ */
