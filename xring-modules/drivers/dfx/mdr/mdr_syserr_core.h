/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MDR_SYSERR_CORE_H__
#define __MDR_SYSERR_CORE_H__

#define PATH_MAXLEN         128

struct mdr_syserr_param_s {
	struct list_head syserr_list;
	u32 modid;
	u32 arg1;
	u32 arg2;
};

bool mdr_syserr_list_empty(void);
int mdr_syserr_init(void);
void mdr_syserr_process_for_ap(u32 modid, u64 arg1, u64 arg2);
void mdr_syserr_process_for_ap_callback(u32 modid, u64 arg1, u64 arg2);

#endif
