// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020-2022 Oplus. All rights reserved.
 */
#define pr_fmt(fmt) "dynamic_readahead: " fmt
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/mm_inline.h>
#include <linux/mm_types.h>
#include <linux/mmzone.h>
#include <linux/sched.h>
#include <linux/vmstat.h>
#include <linux/fs.h>
#include <trace/hooks/mm.h>
#include <linux/cgroup.h>
#include <soc/xring/qos_inherit.h>

static unsigned long long high_wm = 0;
/* true by default, false when dynamic_readahead=N in cmdline */
static bool enable = true;

static struct zone *next_zone_(struct zone *zone)
{
	pg_data_t *pgdat = zone->zone_pgdat;
	if (zone < pgdat->node_zones + MAX_NR_ZONES - 1)
		zone++;
	else
		zone = NULL;
	return zone;
}

#define for_each_zone_(zone)			        \
	for (zone = (NODE_DATA(numa_node_id()))->node_zones; \
	     zone;					\
	     zone = next_zone_(zone))

static inline bool is_lowmem(void)
{
	return global_zone_page_state(NR_FREE_PAGES) < high_wm;
}

static bool current_is_key_task(void)
{
	bool key_task  = true;

	key_task = get_qos_lvl(current) < QOS_CRITICAL ? false : true;
	return key_task || rt_task(current);
  	//return strncmp(task_css(current, cpuset_cgrp_id)->cgroup->kn->name, "background", 11);
}

static void adjust_readaround(void *ignore, unsigned int ra_pages, pgoff_t pgoff,
				pgoff_t *start, unsigned int *size, unsigned int *async_size)
{
	unsigned int dy_ra_pages = ra_pages / 2;

	if (enable && !current_is_key_task() && is_lowmem()) {
		*start = max_t(long, 0, pgoff - dy_ra_pages / 2);
		*size = dy_ra_pages;
		*async_size = dy_ra_pages / 4;
	}

}

static void adjust_readahead(void *ignore, struct readahead_control *ractl, 
                             				unsigned long *max_pages)
{
	struct file_ra_state *ra = ractl->ra;

	if (enable && !current_is_key_task() && is_lowmem()) {
		*max_pages = min_t(long, *max_pages, ra->ra_pages / 2);
	}
}

static int register_dymamic_readahead_hooks(void)
{
	int ret = 0;
	ret = register_trace_android_vh_ra_tuning_max_page(adjust_readahead, NULL);
	if (ret != 0) {
		pr_err("register_trace_android_vh_ra_tuning_max_page failed! ret=%d\n", ret);
		goto out;
	}
	ret = register_trace_android_vh_tune_mmap_readaround(adjust_readaround, NULL);
	if (ret != 0) {
		unregister_trace_android_vh_ra_tuning_max_page(adjust_readahead, NULL);
		pr_err("register_trace_android_vh_tune_mmap_readaround failed! ret=%d\n", ret);
		goto out;
	}
out:
	return ret;
}

static void unregister_dymamic_readahead_hooks(void)
{
	unregister_trace_android_vh_ra_tuning_max_page(adjust_readahead, NULL);
	unregister_trace_android_vh_tune_mmap_readaround(adjust_readaround, NULL);
}

static int __init dynamic_readahead_init(void)
{
	struct zone *zone;
	int ret = 0;
	ret = register_dymamic_readahead_hooks();
	if (ret != 0)
		return ret;
	for_each_zone_(zone) {
		high_wm += high_wmark_pages(zone);
	}
	pr_info("dynamic_readahead_init high_wm: %llu\n", high_wm);
	return 0;
}

static void __exit dynamic_readahead_exit(void)
{
	unregister_dymamic_readahead_hooks();
}

module_init(dynamic_readahead_init);
module_exit(dynamic_readahead_exit);

module_param(enable, bool, 0600);
MODULE_PARM_DESC(enable, "dynamic_readahead.enable=1 to indicate supporting dynamic readahead or not ");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xr-qi");
