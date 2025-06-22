// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <trace/hooks/logbuf.h>
#include <trace/hooks/printk.h>
#include <linux/rtc.h>
#include <soc/xring/xr_timestamp.h>

#include "printk_ringbuffer.h"

#if IS_ENABLED(CONFIG_XRING_DEBUG)
static struct print_overflow_data *printk_data;
#endif

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)     "XR_VH_PRINTK: " fmt


#ifdef XRING_PREFIX
/****************************************************************************/
/* Definition from kernel source file: kernel/printk/printk.c */


#ifdef CONFIG_PRINTK_CALLER
#define PREFIX_MAX      48
#else
#define PREFIX_MAX      32
#endif

/* the maximum size of a formatted record (i.e. with prefix added per line) */
#define CONSOLE_LOG_MAX     1024

/* the maximum size allowed to be reserved for a record */
#define LOG_LINE_MAX        (CONSOLE_LOG_MAX - PREFIX_MAX)
/****************************************************************************/

#define MAX_CALLER_LEN	32
#define XRING_PREFIX
static void xring_add_prefix(struct printk_ringbuffer *rb, struct printk_record *r)
{
	const u32 caller_id = r->info->caller_id;
	char caller[MAX_CALLER_LEN];
	struct prb_reserved_entry e;
	size_t text_len = r->info->text_len;
	char *text = r->text_buf;
	size_t len;

	/*
	 * Add prefix format:	[cpu_id,pid,process_name]
	 */
	len = snprintf(caller, sizeof(caller), "[cpu%u,pid%d,%s", smp_processor_id(), current->pid,
			in_irq() ? "in irq" : current->comm);

	/*
	 * len is the length of the formatted string, not the length of the copied string,
	 * so when len is greater than the buffer size (sizeof(caller)), we need to use the
	 * buffer size to represent the length of the copied string.
	 * Because buffer size includes the terminating '\0', so we need to minus 1
	 */
	len = min(len, sizeof(caller) - 1);
	caller[len++] = ']'; /* Replace '\0' with ']' */

	prb_rec_init_wr(r, len);
	if (prb_reserve_in_last(&e, rb, r, caller_id, LOG_LINE_MAX)) {
		memmove(&text[len], text, text_len);
		memcpy(text, caller, len);

		r->info->text_len += len;

		prb_final_commit(&e);
	}
}
#endif

static void _trace_android_rvh_logbuf_handler(void *unused, struct printk_ringbuffer *rb,
		struct printk_record *r)
{
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	u64 ts_diff_ms = 0;
	unsigned long rem_nsec_start;
	unsigned long rem_nsec_end;
#endif

	/* Replace the origin timestamp */
	r->info->ts_nsec = xr_timestamp_gettime();

#if IS_ENABLED(CONFIG_XRING_DEBUG)
	printk_data->char_count += r->info->text_len + PREFIX_LEN;
	printk_data->ts_end = r->info->ts_nsec;
	ts_diff_ms = (printk_data->ts_end - printk_data->ts_start) / NSEC_PER_MSEC;
	if (ts_diff_ms > 1000) {
		printk_data->required_baud = printk_data->char_count * (8 + 2) * 1000 / ts_diff_ms;
		if (unlikely(printk_data->required_baud > STANDARD_BAUDRATE)) {
			rem_nsec_start = do_div(printk_data->ts_start, NSEC_PER_SEC);
			rem_nsec_end = do_div(printk_data->ts_end, NSEC_PER_SEC);
			printk_deferred(KERN_ERR "Too many logs are printed from %lu.%06lu to %lu.%06lu, required_baud = %d.\n",
							(unsigned long)printk_data->ts_start,
							rem_nsec_start / 1000,
							(unsigned long)printk_data->ts_end,
							rem_nsec_end / 1000,
							printk_data->required_baud
							);
		}
		printk_data->ts_start = printk_data->ts_end;
		printk_data->char_count = 0;
	}
#endif

#ifdef XRING_PREFIX
	xring_add_prefix(rb, r);
#endif
}

#define CORE_ID_WIDTH	(4)
#define CORE_ID_MASK	(0xf)
static void _trace_android_vh_printk_caller_handler(void *unused, char *caller, size_t size, u32 id,
		int *ret)
{
	if (id & 0x80000000)
		snprintf(caller, size, "C%u", id & ~0x80000000);
	else
		snprintf(caller, size, "T%u-C%u", id >> CORE_ID_WIDTH, id & CORE_ID_MASK);

	*ret = 1;
}

static void _trace_android_vh_printk_caller_id_handler(void *unused, u32 *caller_id)
{
	*caller_id = in_task() ?
		((task_pid_nr(current) << CORE_ID_WIDTH) | (smp_processor_id() & CORE_ID_MASK)) :
		0x80000000 + smp_processor_id();
}

static int xring_hook_printk_init(void)
{
	int ret;
	u64 ts_time;

	ts_time = xr_timestamp_gettime();

	pr_info("xr_vh_printk entry (timestamp:%lld.%09llds)\n",
			ts_time / NSEC_PER_SEC,
			ts_time - ts_time / NSEC_PER_SEC * NSEC_PER_SEC);

#if IS_ENABLED(CONFIG_XRING_DEBUG)
	printk_data = kzalloc(sizeof(struct print_overflow_data), GFP_KERNEL);
	if (printk_data == NULL) {
		pr_err("Failed to allocate memory!\n");
		return -ENOMEM;
	}
#endif

	ret = register_trace_android_rvh_logbuf(&_trace_android_rvh_logbuf_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_rvh_logbuf failed (ret=%d)\n", ret);
		return ret;
	}

	ret = register_trace_android_vh_printk_caller(
			&_trace_android_vh_printk_caller_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_vh_printk_caller failed (ret=%d)\n", ret);
		return ret;
	}

	ret = register_trace_android_vh_printk_caller_id(
			&_trace_android_vh_printk_caller_id_handler, NULL);
	if (ret) {
		pr_err("register_trace_android_vh_printk_caller_id failed (ret=%d)\n", ret);
		return ret;
	}

	pr_info("xr_vh_printk init success\n");

	return ret;
}

static void xring_hook_printk_exit(void)
{
	unregister_trace_android_vh_printk_caller_id(
			&_trace_android_vh_printk_caller_id_handler, NULL);
	unregister_trace_android_vh_printk_caller(&_trace_android_vh_printk_caller_handler, NULL);
#if IS_ENABLED(CONFIG_XRING_DEBUG)
	kfree(printk_data);
#endif
	pr_info("xr_vh_printk exit\n");
}

module_init(xring_hook_printk_init);
module_exit(xring_hook_printk_exit);
MODULE_DESCRIPTION("X-Ring Printk Vendor Hooks Driver");
MODULE_LICENSE("GPL");
