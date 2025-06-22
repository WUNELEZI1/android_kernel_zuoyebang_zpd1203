/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_MODULE_CORE_H__
#define __MDR_MODULE_CORE_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

struct mdr_module_ops_s {
	struct list_head s_list;
	u64 s_core_id;
	struct mdr_module_ops s_ops;
};

u64 mdr_get_nve(void);
char *mdr_get_exception_core(u64 coreid);
u64 mdr_get_cur_regcore(void);
u64 mdr_get_dump_result(u32 modid);
void mdr_notify_module_reset(u32 modid, struct mdr_exception_info_s *e_info);
u64 mdr_notify_onemodule_dump(u32 modid, u64 core, u32 type, u64 formcore, char *path);
u64 mdr_notify_module_dump(u32 modid, struct mdr_exception_info_s *e_info, char *path);
void mdr_print_all_ops(void);

#endif
