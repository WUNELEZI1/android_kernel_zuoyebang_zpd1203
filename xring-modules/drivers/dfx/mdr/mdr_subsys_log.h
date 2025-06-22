/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_SUBSYS_LOG_H__
#define __MDR_SUBSYS_LOG_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

struct mdr_subsys_info_s {
	struct	list_head s_list;
	char    *node_name;
	char    *log_name;
	u32     log_size;
	u64	core_id;
};

void mdr_save_subsys_log(struct mdr_exception_info_s *p_exce_info);

#endif
