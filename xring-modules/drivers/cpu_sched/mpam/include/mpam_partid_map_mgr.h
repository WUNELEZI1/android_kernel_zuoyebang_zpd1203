/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __MPAM_PARTID_MAP_MGR_H_
#define __MPAM_PARTID_MAP_MGR_H_

#include "mpam_internal.h"
#include "soc/xring/sched.h"
#include <linux/cgroup.h>

#define MPAM_CGROUP_SUBSYS_MAX 50U

#define TASK_PARTID_DATA(p)                                                    \
	(((struct xr_task_struct *)(p->android_vendor_data1))->mpam_partid)

int mpam_set_task_partid(int pid, u16 partid);
int mpam_set_cgroup_partid(int cgroup_id, u16 partid);
int mpam_set_cpu_partid(int cpu_id, u16 partid);
int mpam_get_cgroup_subsys_id(const char *path_str, int *cgroup_id);

int mpam_set_cpu_enable(int enable);
int mpam_get_cpu_enable_status(void);

int mpam_map_mgr_init(void);

#endif /*__MPAM_PARTID_MAP_MGR_H_*/
