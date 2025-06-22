// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <trace/hooks/vmscan.h>
#include <trace/hooks/mm.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/printk.h>

#include "../xring_mem_adapter.h"

static u32 abort_madvise_opt;
static u32 abort_compact_opt;

module_param_named(abort_madvise, abort_madvise_opt, uint, 0644);
module_param_named(abort_compact, abort_compact_opt, uint, 0644);

static void should_abort_madvise(void *data, struct vm_area_struct *vma, bool *abort_madvise)
{
	if (abort_madvise_opt) {
		*abort_madvise = true;
		xrmem_info("set abort_madvise_opt success!\n");
	}
}

static void should_abort_compact(void *data, bool *abort_compact)
{
	if (abort_compact_opt) {
		*abort_compact = true;
		xrmem_info("set abort_compact_opt success!\n");
	}
}

static int register_abort_mm_opt_vendor_hooks(void)
{
	int ret = 0;

	ret = register_trace_android_vh_madvise_cold_or_pageout_abort(should_abort_madvise, NULL);
	if (ret != 0) {
		xrmem_err("register_trace_android_vh_madvise_cold_or_pageout_abort failed! ret=%d\n", ret);
		goto out;
	}

	ret = register_trace_android_vh_compact_finished(should_abort_compact, NULL);
	if (ret != 0) {
		unregister_trace_android_vh_madvise_cold_or_pageout_abort(should_abort_madvise, NULL);
		xrmem_err("register_trace_android_vh_compact_finished failed! ret=%d\n", ret);
		goto out;
	}

out:
	return ret;
}

static void unregister_abort_mm_opt_vendor_hooks(void)
{
	unregister_trace_android_vh_compact_finished(should_abort_compact, NULL);
	unregister_trace_android_vh_madvise_cold_or_pageout_abort(should_abort_madvise, NULL);
}

int abort_mm_opt_init(void)
{
	int ret = 0;

	ret = register_abort_mm_opt_vendor_hooks();
	if (ret != 0)
		return ret;

	xrmem_info("init succeed!\n");
	return 0;
}

void abort_mm_opt_exit(void)
{
	unregister_abort_mm_opt_vendor_hooks();
	xrmem_info("exit succeed!\n");
}
