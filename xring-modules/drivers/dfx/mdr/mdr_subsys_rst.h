/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_SUBSYS_RST_H__
#define __MDR_SUBSYS_RST_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

void mdr_subsys_reset(struct mdr_exception_info_s *e_info);
bool check_subsys_sysrst_enable(struct mdr_exception_info_s *e_info);

#endif
