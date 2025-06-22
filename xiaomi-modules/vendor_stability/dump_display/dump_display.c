#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "dfx_memory_layout.h"
#include <linux/io.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/stacktrace.h>
#include <linux/sizes.h>
#include <linux/panic_notifier.h>
#include <linux/module.h>
#include <asm/stacktrace.h>

#define DUMP_DISPALY_MEM_START DFX_MEM_RSV_ADDR
#define DUMP_DISPALY_MEM_SIZE 0x800

#define CRASH_RECORD_MAGIC 0x12345678
#define TRACE_INFO_SIZE 2044

struct crash_info {
	unsigned int magic;
	char back_trace[TRACE_INFO_SIZE];
};

static char dump_fingerprint[128];

struct crash_info __iomem *crash_info_region_base;
int trace_snprint(char *buf, size_t size, const unsigned long *entries,
			unsigned int nr_entries)
{
	unsigned int generated, i, total = 0;

	if (!entries)
		return 0;

	for (i = 0; i < nr_entries && size; i++) {
		generated = snprintf(buf, size, "%pS\n",
				     (void *)entries[i]);

		total += generated;
		if (generated >= size) {
			buf += size;
			size = 0;
		} else {
			buf += generated;
			size -= generated;
		}
	}
	generated = snprintf(buf, size, "\ncomm:%s", current->comm);
	size = size - generated;
	
	snprintf(buf, size, "\nVersion:[%s]", dump_fingerprint);
	
	return total;
}

static int  set_backtrace_msg(struct notifier_block *self, unsigned long v, void *p)
{
	unsigned int nr_entries;
	unsigned long entries[30] = {0};
	char *trace_record;

	nr_entries = stack_trace_save(entries,  ARRAY_SIZE(entries), 2);
	if (!nr_entries)
		return 0;

	trace_record = kmalloc(TRACE_INFO_SIZE, GFP_ATOMIC);
	if (!trace_record)
		return 0;

	trace_snprint(trace_record, TRACE_INFO_SIZE, entries, nr_entries);
	memcpy_toio(crash_info_region_base->back_trace, trace_record, TRACE_INFO_SIZE);

	kfree(trace_record);
	return 0;
}

static struct notifier_block  panic_block_to_dump = {
	.notifier_call = set_backtrace_msg
};

static int dump_display_init(void)
{
	char unknow_info[256] = {0};
	int size = 0;
	int unknow_info_size  = 0;

	crash_info_region_base = ioremap(DUMP_DISPALY_MEM_START, DUMP_DISPALY_MEM_SIZE);
	if (!crash_info_region_base) {
		pr_err("error to ioremap crash_record base\n");
		return -EIO;
	}

	memset_io(crash_info_region_base, 0, DUMP_DISPALY_MEM_SIZE);
	crash_info_region_base->magic = CRASH_RECORD_MAGIC;
	size = snprintf(unknow_info, sizeof("UNKNOWN"), "%s\n", "UNKNOWN");
	snprintf(unknow_info+size-1, sizeof(dump_fingerprint), "\nVersion:[%s]", dump_fingerprint);
	unknow_info_size = sizeof(unknow_info);
	memcpy_toio(crash_info_region_base->back_trace, unknow_info, unknow_info_size );

	atomic_notifier_chain_register(&panic_notifier_list,
				&panic_block_to_dump);
	return 0;
}

static void dump_display_exit(void)
{
    iounmap(crash_info_region_base);
    pr_info("dump_display_exit\n");
}
module_param_string(fingerprint, dump_fingerprint, 128, 0);
module_init(dump_display_init);
module_exit(dump_display_exit);
MODULE_LICENSE("GPL v2");
