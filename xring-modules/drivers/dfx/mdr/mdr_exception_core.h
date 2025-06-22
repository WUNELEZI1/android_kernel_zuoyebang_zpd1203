/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_MODULE_EXCEPTION_H__
#define __MDR_MODULE_EXCEPTION_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"


#define MDR_COMMON_CALLBACK 0x1ull
#define MDR_CALLBACK_MASK 0x3ull

void mdr_callback(struct mdr_exception_info_s *p_exce_info, u32 mod_id, char *logpath);
void mdr_print_one_exc(struct mdr_exception_info_s *e);
void mdr_print_all_exc(void);
struct mdr_exception_info_s *mdr_get_exception_info(u32 modid);


#endif
