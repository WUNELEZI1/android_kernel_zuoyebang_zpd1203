// SPDX-License-Identifier: GPL-2.0+
/*
 * mi memory leak detect 
 *
 * Copyright (C) 2024 Xiaomi Ltd.
 * 
 * yangshiguang <yangshiguang@xiaomi.com>
 */

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/workqueue.h>
#include <linux/vmstat.h>
#include <linux/sched/clock.h>

#include "memcheck_process_mem.h"
#include "memcheck_ashmem.h"
#include "memcheck_fd_socket.h"
#include "memcheck_slab.h"
#include "memcheck_vmalloc.h"
#include "memcheck_cma.h"
#include "memcheck_fd_pipe.h"
#include "memcheck_fd_fence.h"
#include "memcheck_gpumem.h"
#include "memcheck_ioctl.h"
#include "memcheck_dma-buf.h"
#include "memcheck_account.h"

#define KB_SHIFT	10
#define MB_SHIFT	20

// Reporting threshold > 50 us
static unsigned long pageslow_report_threshold __read_mostly = 50000;
module_param(pageslow_report_threshold, ulong, 0644);

struct memstat_info {
	unsigned long total;
	unsigned long free;
	unsigned long slab_total;
	unsigned long dma_total;
	unsigned long others;
	unsigned long kgsl_total;
};

struct pages_slowpatch_info {
	pid_t pid;
	char comm[TASK_COMM_LEN];
	gfp_t *gfp_mask;
	u64 utime;
	unsigned int order;
	unsigned long pages_reclaimed;
	int retry_loop_count;
};

static struct work_struct show_stats_work;
static struct work_struct memstat_report_work;
static struct work_struct page_slow_report_work;
static struct proc_dir_entry *entry_memcheck = NULL;
struct memstat_info memstat_info;
struct pages_slowpatch_info pages_slowpatch_info;


static void memcheck_stats_show_func(struct work_struct *work)
{
	memcheck_top_pmem_info(5);
	// memcheck_cma_info_show();
	// memcheck_dmabuf_info_show();
	// memcheck_vmalloc_info_show();
	memcheck_gpumem_info_show();
}

void memcheck_warn_alloc_show_mem(void)
{
	// memstate show max 1 time per minute
	static DEFINE_RATELIMIT_STATE(ratelimit_warn_show_mem, 60 * HZ, 1);

	if (!__ratelimit(&ratelimit_warn_show_mem))
		return;

	schedule_work(&show_stats_work);
}

static void memcheck_memstat_report_func(struct work_struct *work)
{
	int ret = 0;
	struct report_event *report_event;

	report_event = hypsys_event_alloc(MEMSTAT_REPORT);
	if (!report_event)
		return;

	//total
	memstat_info.total = totalram_pages();
	memstat_info.total = memstat_info.total << (PAGE_SHIFT - KB_SHIFT);
	//free
	memstat_info.free = global_zone_page_state(NR_FREE_PAGES);
	memstat_info.free = memstat_info.free << (PAGE_SHIFT - KB_SHIFT);
	//slab
	memstat_info.slab_total = global_node_page_state_pages(NR_SLAB_RECLAIMABLE_B) +
								global_node_page_state_pages(NR_SLAB_UNRECLAIMABLE_B);
	memstat_info.slab_total = memstat_info.slab_total << (PAGE_SHIFT - KB_SHIFT);
	//others
	memstat_info.others = global_node_page_state_pages(NR_ANON_MAPPED) +
							global_node_page_state_pages(NR_FILE_PAGES) +
							global_node_page_state_pages(NR_PAGETABLE) +
							global_node_page_state_pages(NR_KERNEL_STACK_KB);
	memstat_info.others = memstat_info.others << (PAGE_SHIFT - KB_SHIFT);
	//dma_buf
	memstat_info.dma_total = memcheck_dmabuf_get_total() >> KB_SHIFT;
	//kgsl memory
	memstat_info.kgsl_total = memcheck_kgsl_get_total() >> KB_SHIFT;

	ret += hypsys_event_add_int(report_event, "total", memstat_info.total);
	ret += hypsys_event_add_int(report_event, "free", memstat_info.free);
	ret += hypsys_event_add_int(report_event, "slab_total", memstat_info.slab_total);
	ret += hypsys_event_add_int(report_event, "dmabuf_total", memstat_info.dma_total);
	ret += hypsys_event_add_int(report_event, "kgsl_total", memstat_info.kgsl_total);
	ret += hypsys_event_add_int(report_event, "others", memstat_info.others);
	if (ret) {
		memcheck_err("add info to cma_event failed, ret=%d\n", ret);
		goto end;
	}

	if (!hypsys_event_report(report_event))
		memcheck_err("report cma_event failed\n");

end:
	hypsys_event_destroy(report_event);
}

void memcheck_memstat_report(void)
{
	schedule_work(&memstat_report_work);
}

static void memcheck_page_slow_report_func(struct work_struct *work)
{
	int ret = 0;
	struct report_event *report_event;

	report_event = hypsys_event_alloc(PAGESLOW_REPORT);
	if (!report_event)
		return;

	ret += hypsys_event_add_int(report_event, "pid", pages_slowpatch_info.pid);
	ret += hypsys_event_add_str(report_event, "comm", pages_slowpatch_info.comm);
	ret += hypsys_event_add_int(report_event, "order", pages_slowpatch_info.order);
	ret += hypsys_event_add_int(report_event, "pages_reclaimed", pages_slowpatch_info.pages_reclaimed);
	ret += hypsys_event_add_int(report_event, "retry_loop_count", pages_slowpatch_info.retry_loop_count);
	ret += hypsys_event_add_int(report_event, "utime_ns", pages_slowpatch_info.utime);

	if (ret) {
		memcheck_err("add info to slab_event failed, ret=%d\n", ret);
		goto end;
	}

	if (!hypsys_event_report(report_event))
		memcheck_err("report pages_slowpatch failed\n");

end:
	hypsys_event_destroy(report_event);
}

void memcheck_page_slow_report(gfp_t *gfp_mask, unsigned int order, u64 stime,
								unsigned long pages_reclaimed, int retry_loop_count)
{
	// report once maximum in 2 mins
	static DEFINE_RATELIMIT_STATE(ratelimit_page_slow_report, 2 * 60 * HZ, 1);

	if (stime < pageslow_report_threshold)
		return;

	if (!__ratelimit(&ratelimit_page_slow_report))
		return;

	pages_slowpatch_info.pid = current->pid;
	strscpy(pages_slowpatch_info.comm, current->comm,
			sizeof(pages_slowpatch_info.comm));
	pages_slowpatch_info.gfp_mask = gfp_mask;
	pages_slowpatch_info.order = order;
	pages_slowpatch_info.utime = local_clock() - stime;
	pages_slowpatch_info.pages_reclaimed = pages_reclaimed;
	pages_slowpatch_info.retry_loop_count = retry_loop_count;

	schedule_work(&page_slow_report_work);
}

int memcheck_createfs(void)
{
	entry_memcheck = proc_mkdir("memcheck", NULL);
	if(entry_memcheck == NULL) {
		memcheck_err("create proc entry memcheck failed\n");
		return -ENOMEM;
	}

	memcheck_pmem_createfs(entry_memcheck);
	memcheck_dmabuf_createfs(entry_memcheck);
	memcheck_ashmem_createfs(entry_memcheck);
	memcheck_gpumem_createfs(entry_memcheck);
	memcheck_cma_createfs(entry_memcheck);
	memcheck_vmalloc_createfs(entry_memcheck);
	memcheck_fd_socket_createfs(entry_memcheck);
	memcheck_fd_pipe_createfs(entry_memcheck);
	memcheck_fd_fence_createfs(entry_memcheck);
	memcheck_slab_createfs(entry_memcheck);

	INIT_WORK(&show_stats_work, memcheck_stats_show_func);
	INIT_WORK(&memstat_report_work, memcheck_memstat_report_func);
	INIT_WORK(&page_slow_report_work, memcheck_page_slow_report_func);

	return 0;
}
