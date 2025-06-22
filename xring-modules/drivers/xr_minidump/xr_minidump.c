// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xr-minidump driver
 */

#define pr_fmt(fmt) "[xr_minidump]:%s:%d " fmt, __func__, __LINE__

#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <linux/android_debug_symbols.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/mmzone.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <linux/slab.h>
#include <soc/xring/dfx_switch.h>
#include <soc/xring/securelib/securec.h>
#include <linux/bits.h>
#include <linux/sched/prio.h>
#include <linux/seq_buf.h>
#include <linux/panic_notifier.h>
#include <trace/events/sched.h>
#include <soc/xring/xr_timestamp.h>
#include <trace/hooks/logbuf.h>
#include "printk_ringbuffer.h"

#define CRC_SIZE 4
#define MAX_MINIDUMP_REGION_CNT 100
#define MAX_NAME_LENGTH 16
#define PER_CPU_REGION_NAME "MD_PERCPU"
#define TASK_INFO_REGION_NAME "MD_TASK_INFO"
#define RECORD_NAME			"record"
#define CONSOLE_NAME		"console"
#define FTRACE_NAME			"ftrace"
#define PMSG_NAME			"pmsg"
#define MINIDUMP_PREFIX		"MD_"
#define SIZE_SUFFIX			"-size"
#define KSTACK_REGION_NAME "MD_KSTACK"
#define XR_MINIDUMP_MAGIC 0xDEADBEEFDEADBEEF
#define TASK_INFO_PAGES 150
#define CHECK_SUM_WIDTH 32
#define LOW_32_BIT_MASK 0xffffffff
#define TASK_INFO_PAGES			150
#define STACK_NUM_PAGES (THREAD_SIZE / PAGE_SIZE)

enum dump_status {
	DSTATUS_INVALID = 1,
	DSTATUS_REGION_INFO,
	DSTATUS_DUMP_REGION,
	DSTATUS_READ_REGION,
};

#pragma pack(4)
struct minidump_region {
	u8 name[MAX_NAME_LENGTH];
	u64 pbase_addr;
	u64 vbase_addr;
	u64 size;
};

struct minidump_header {
	u64 magic;
	u32 status;
	u32 region_cnt;
	struct minidump_region md_regions[MAX_MINIDUMP_REGION_CNT];
	u32 crc;
};
#pragma pack()

struct xrmd_cpu_stack_st {
	ktime_t timestamp;
	struct minidump_region stack_mdr[STACK_NUM_PAGES];
} ____cacheline_aligned_in_smp;

static DEFINE_PER_CPU_SHARED_ALIGNED(struct xrmd_cpu_stack_st, g_xrmd_percpu_stack);
static struct seq_buf g_xrmd_task_info;
static struct minidump_header *g_xrmd_header;
static bool is_xrmd_in_oops_handler;

static void xrmd_init_header(struct minidump_header *header)
{
	header->magic = XR_MINIDUMP_MAGIC;
	header->region_cnt = 0;
	header->crc = 0;
}

void xrmd_print_header(struct minidump_header *header)
{
	pr_info("magic: %llx\n", header->magic);
	pr_info("status: %x\n", header->status);
	pr_info("region_cnt: %x\n", header->region_cnt);
	pr_info("crc: %x\n", header->crc);
}

static int xrmd_register_region(const struct minidump_region *region, struct minidump_header *header)
{
	struct minidump_region *cur_region;
	int ret;
	int region_idx = header->region_cnt;

	if (region_idx >= MAX_MINIDUMP_REGION_CNT) {
		pr_err("minidump header region count too large\n");
		return -1;
	}

	cur_region = &header->md_regions[region_idx];
	header->region_cnt++;
	cur_region->pbase_addr = region->pbase_addr;
	cur_region->vbase_addr = region->vbase_addr;
	cur_region->size = region->size;
	ret = strcpy_s(cur_region->name, sizeof(cur_region->name), region->name);
	if (ret != EOK) {
		pr_err("strcpy_s in %s return error:%d\n", __func__, ret);
		return -1;
	}
	return region_idx;
}

static unsigned int xrmd_get_pstore_size(struct device_node *node, unsigned char *name)
{
	int ret;
	unsigned int size;
	unsigned char node_name[MAX_NAME_LENGTH] = {0};

	scnprintf(node_name, MAX_NAME_LENGTH, "%s%s", name, SIZE_SUFFIX);
	ret = of_property_read_u32(node, node_name, &size);
	if (!ret && size > 0)
		return size;
	else
		return 0;
}

static unsigned int xrmd_register_pstore_region(struct device_node *node, unsigned char *name,
	phys_addr_t start_phy)
{
	unsigned int size;
	struct minidump_region region;

	size = xrmd_get_pstore_size(node, name);
	if (size > 0) {
		scnprintf(region.name, MAX_NAME_LENGTH, "%s%s", MINIDUMP_PREFIX, name);
		region.vbase_addr = (uintptr_t)phys_to_virt(start_phy);
		region.pbase_addr = start_phy;
		region.size = size;
		if (xrmd_register_region(&region, g_xrmd_header) < 0)
			pr_err("Failed to add percpu sections in Minidump\n");
		return size;
	}
	return 0;
}


static void xrmd_register_pstore_info(void)
{
	int ret;
	struct device_node *node;
	struct resource resource;
	struct reserved_mem *rmem = NULL;
	phys_addr_t paddr;

	node = of_find_compatible_node(NULL, NULL, "ramoops");
	if (IS_ERR_OR_NULL(node)) {
		pr_err("Failed to get pstore node\n");
		return;
	}

	ret = of_address_to_resource(node, 0, &resource);
	if (ret) {
		rmem = of_reserved_mem_lookup(node);
		if (rmem) {
			paddr = rmem->base;
		} else {
			pr_err("Failed to get pstore mem\n");
			return;
		}
	} else {
		paddr = resource.start;
	}

	paddr += xrmd_register_pstore_region(node, RECORD_NAME, paddr);
	paddr += xrmd_register_pstore_region(node, CONSOLE_NAME, paddr);
	paddr += xrmd_register_pstore_region(node, FTRACE_NAME, paddr);
	paddr += xrmd_register_pstore_region(node, PMSG_NAME, paddr);
}

static void xrmd_register_percpu_region(struct minidump_header *header)
{
	struct minidump_region region;
	size_t static_size;
	void __percpu *base;
	unsigned int cpu;

	base = android_debug_symbol(ADS_PER_CPU_START);
	static_size = (size_t)(android_debug_symbol(ADS_PER_CPU_END) - base);

	for_each_possible_cpu(cpu) {
		void *start = per_cpu_ptr(base, cpu);

		scnprintf(region.name, sizeof(region.name), "%s%d", PER_CPU_REGION_NAME, cpu);
		region.vbase_addr = (uintptr_t)start;
		region.pbase_addr = per_cpu_ptr_to_phys(start);
		region.size = static_size;
		if (xrmd_register_region(&region, header) < 0)
			pr_err("Failed to add percpu sections in Minidump\n");
	}
}

static int xrmd_init_task_info(struct minidump_header *header, int num_pages)
{
	struct minidump_region region;
	char *buf;

	buf = kzalloc(num_pages * PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -EINVAL;

	scnprintf(region.name, sizeof(region.name), "%s", TASK_INFO_REGION_NAME);
	region.vbase_addr = (uintptr_t)buf;
	region.pbase_addr = virt_to_phys(buf);
	region.size = num_pages * PAGE_SIZE;
	if (xrmd_register_region(&region, header) < 0) {
		pr_err("Failed to add percpu sections in Minidump\n");
		kfree(buf);
		return -1;
	}

	seq_buf_init(&g_xrmd_task_info, buf, num_pages * PAGE_SIZE);
	return 0;
}

static void xrmd_dump_tasks_info(void)
{
	struct task_struct *p, *t;
#if IS_ENABLED(CONFIG_SCHED_WALT)
	struct walt_task_struct *wts;
#endif

	seq_buf_printf(&g_xrmd_task_info, "%-15s", "Task name");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 6, "PID");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 16, "Exec_started_at");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 16, "Last_queued_at");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 16, "Total_wait_time");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 12, "Exec_times");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 4, "CPU");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 5, "Prio");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 6, "State");
#if IS_ENABLED(CONFIG_SCHED_WALT)
	seq_buf_printf(&g_xrmd_task_info, "%*s", 17, "Last_enqueued_ts");
	seq_buf_printf(&g_xrmd_task_info, "%*s", 16, "Last_sleep_ts");
#endif
	seq_buf_printf(&g_xrmd_task_info, "\n");

	for_each_process_thread(p, t) {
		seq_buf_printf(&g_xrmd_task_info, "%-15s", t->comm);
		seq_buf_printf(&g_xrmd_task_info, "%6d", t->pid);
		seq_buf_printf(&g_xrmd_task_info, "%16lld", t->sched_info.last_arrival);
		seq_buf_printf(&g_xrmd_task_info, "%16lld", t->sched_info.last_queued);
		seq_buf_printf(&g_xrmd_task_info, "%16lld", t->sched_info.run_delay);
		seq_buf_printf(&g_xrmd_task_info, "%12ld", t->sched_info.pcount);
		seq_buf_printf(&g_xrmd_task_info, "%4d", task_cpu(t));
		seq_buf_printf(&g_xrmd_task_info, "%5d", t->prio);
		seq_buf_printf(&g_xrmd_task_info, "%*c", 6, task_state_to_char(t));
#if IS_ENABLED(CONFIG_SCHED_WALT)
		wts = (struct walt_task_struct *) t->android_vendor_data1;
		seq_buf_printf(&g_xrmd_task_info, "%17ld", wts->last_enqueued_ts);
		seq_buf_printf(&g_xrmd_task_info, "%16ld", wts->last_sleep_ts);
#endif
		seq_buf_printf(&g_xrmd_task_info, "\n");
	}
}

static void xrmd_register_cpu_stack(void)
{
	int cpu;
	int i;
	struct minidump_region *region;
	struct xrmd_cpu_stack_st *xrmd_cpu_stack;

	for_each_possible_cpu(cpu) {
		xrmd_cpu_stack = &per_cpu(g_xrmd_percpu_stack, cpu);
		region = xrmd_cpu_stack->stack_mdr;
		seq_buf_printf(&g_xrmd_task_info, "%d time:%lld.%09llds\n", cpu, xrmd_cpu_stack->timestamp / NSEC_PER_SEC,
		xrmd_cpu_stack->timestamp - xrmd_cpu_stack->timestamp / NSEC_PER_SEC * NSEC_PER_SEC);
		for (i = 0; i < STACK_NUM_PAGES; i++) {
			if (xrmd_register_region(region, g_xrmd_header) < 0)
				pr_err("Failed to add cpu stack sections in Minidump\n");
			region++;
		}
	}
}

static u32 checksum32(u32 *addr, u32 count)
{
	u64 sum = 0;
	u32 left = 0;
	u32 i;

	while (count > (sizeof(u32) - 1)) {
		sum += *(addr++);
		count -= sizeof(u32);
	}

	if (count > 0) {
		left = 0;
		i = 0;
		while (i <= count) {
			*((u8 *)&left + i) = *((u8 *)addr + i);
			i++;
		}
		sum += left;
	}

	while (sum >> CHECK_SUM_WIDTH)
		sum = (sum & LOW_32_BIT_MASK) + (sum >> CHECK_SUM_WIDTH);

	return (~sum);
}

void xrmd_panic_dump(void)
{
	if (is_xrmd_in_oops_handler)
		return;
	is_xrmd_in_oops_handler = true;
	xrmd_dump_tasks_info();
	xrmd_register_cpu_stack();
	g_xrmd_header->status = DSTATUS_REGION_INFO;
	g_xrmd_header->crc = checksum32((u32 *)g_xrmd_header, sizeof(struct minidump_header) - CRC_SIZE);
	xrmd_print_header(g_xrmd_header);
}

static int xrmd_panic_handler(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	xrmd_panic_dump();
	return NOTIFY_DONE;
}

static struct notifier_block xrmd_panic_block = {
	.notifier_call = xrmd_panic_handler,
	.priority = INT_MAX - 2, //reserve two high priority panic notifiers
};

static void xrmd_update_stack_region(struct minidump_region *region, u64 sp)
{
	struct page *sp_page;

	region->vbase_addr = sp;
	sp_page = vmalloc_to_page((const void *) sp);
	region->pbase_addr = page_to_phys(sp_page);
}

static void xrmd_register_stack_region(struct minidump_region *region, u64 sp, u64 size)
{
	struct page *sp_page;

	region->vbase_addr = sp;
	sp_page = vmalloc_to_page((const void *) sp);
	region->pbase_addr = page_to_phys(sp_page);
	region->size = size;
}

static void xrmd_update_vmap_stack(struct minidump_region *region, u64 sp)
{
	int i;

	sp &= ~(PAGE_SIZE - 1);
	for (i = 0; i < STACK_NUM_PAGES; i++) {
		xrmd_update_stack_region(region, sp);
		sp += PAGE_SIZE;
		region++;
	}
}

static void xrmd_update_cpu_stack(struct task_struct *tsk, u32 cpu, u64 sp)
{
	struct xrmd_cpu_stack_st *xrmd_cpu_stack = &per_cpu(g_xrmd_percpu_stack, cpu);

	if (is_idle_task(tsk))
		return;
	xrmd_cpu_stack->timestamp = xr_timestamp_gettime();
	xrmd_update_vmap_stack(xrmd_cpu_stack->stack_mdr, sp);
}

void xrmd_update_stack_notifer(void *ignore, bool preempt,
		struct task_struct *prev, struct task_struct *next, unsigned int prev_state)
{
	u32 cpu = task_cpu(next);
	u64 sp = (u64)next->stack;

	xrmd_update_cpu_stack(next, cpu, sp);
}

void xrmd_update_stack_ipi_handler(void *data)
{
	u32 cpu = smp_processor_id();
	struct vm_struct *stack_vm_area;
	u64 sp = current_stack_pointer;

	if (is_idle_task(current))
		return;

	stack_vm_area = task_stack_vm_area(current);
	sp = (u64)stack_vm_area->addr;
	xrmd_update_cpu_stack(current, cpu, sp);
}

static void xrmd_register_vmap_stack(struct minidump_region *region, u64 sp, int cpu)
{
	int i;

	sp &= ~(PAGE_SIZE - 1);
	for (i = 0; i < STACK_NUM_PAGES; i++) {
		scnprintf(region->name, sizeof(region->name), "%s%d_%d", KSTACK_REGION_NAME, cpu, i);
		xrmd_register_stack_region(region, sp, PAGE_SIZE);
		sp += PAGE_SIZE;
		region++;
	}
}

static void xrmd_init_cpu_stack(void)
{
	int cpu;
	u64 sp = current_stack_pointer;
	struct xrmd_cpu_stack_st *xrmd_cpu_stack;
	struct vm_struct *stack_vm_area;

	stack_vm_area = task_stack_vm_area(current);
	sp = (u64)stack_vm_area->addr;
	for_each_possible_cpu(cpu) {
		xrmd_cpu_stack = &per_cpu(g_xrmd_percpu_stack, cpu);
		xrmd_cpu_stack->timestamp = xr_timestamp_gettime();
		xrmd_register_vmap_stack(xrmd_cpu_stack->stack_mdr, sp, cpu);
	}

	register_trace_sched_switch(xrmd_update_stack_notifer, NULL);
	smp_call_function(xrmd_update_stack_ipi_handler, NULL, 1);
}


static void xrmd_register_printk_buffer(struct printk_ringbuffer *prb)
{
	struct prb_desc_ring desc_ring;
	struct prb_data_ring data_ring;
	struct prb_desc *descs_addr;
	struct printk_info *infos_addr;
	struct minidump_region region;
	static bool is_logbuf_register;
	int ret;

	if (is_logbuf_register || !prb)
		return;

	is_logbuf_register = true;
	desc_ring = prb->desc_ring;
	data_ring = prb->text_data_ring;
	descs_addr = desc_ring.descs;
	infos_addr = desc_ring.infos;
	scnprintf(region.name, MAX_NAME_LENGTH, "%s%s", MINIDUMP_PREFIX, "KLOGBUF");
	region.vbase_addr = (uintptr_t)data_ring.data;
	region.pbase_addr = virt_to_phys(data_ring.data);
	region.size = _DATA_SIZE(data_ring.size_bits);
	ret = xrmd_register_region(&region, g_xrmd_header);
	if (ret < 0)
		pr_err("Failed to add log_text in minidump\n");

	scnprintf(region.name, MAX_NAME_LENGTH, "%s%s", MINIDUMP_PREFIX, "LOG_DESC");
	region.vbase_addr = (uintptr_t)descs_addr;
	region.pbase_addr = virt_to_phys(descs_addr);
	region.size = sizeof(struct prb_desc) * _DESCS_COUNT(desc_ring.count_bits);
	ret = xrmd_register_region(&region, g_xrmd_header);
	if (ret < 0)
		pr_err("Failed to add log_desc in minidump\n");

	scnprintf(region.name, MAX_NAME_LENGTH, "%s%s", MINIDUMP_PREFIX, "LOG_INFO");
	region.vbase_addr = (uintptr_t)infos_addr;
	region.pbase_addr = virt_to_phys(infos_addr);
	region.size = sizeof(struct printk_info) * _DESCS_COUNT(desc_ring.count_bits);
	ret = xrmd_register_region(&region, g_xrmd_header);
	if (ret < 0)
		pr_err("Failed to add log_info in minidump\n");
}


static void logbuf_vh_handler(void *unused, struct printk_ringbuffer *rb,
		struct printk_record *r)
{
	xrmd_register_printk_buffer(rb);
}

static int xrmd_register_kernel_info(void)
{
	g_xrmd_header = (struct minidump_header *)memremap(DFX_MEM_MINIDUMP_ADDR,
	DFX_MEM_MINIDUMP_SIZE, MEMREMAP_WC);
	if (g_xrmd_header == NULL) {
		pr_err("map minidump reserve mem failed!\n");
		return -ENOMEM;
	}
	memset_s(g_xrmd_header, DFX_MEM_MINIDUMP_SIZE, 0, DFX_MEM_MINIDUMP_SIZE);
	g_xrmd_header->status = DSTATUS_INVALID;
	xrmd_init_header(g_xrmd_header);
	xrmd_register_percpu_region(g_xrmd_header);
	xrmd_register_pstore_info();
	xrmd_init_task_info(g_xrmd_header, TASK_INFO_PAGES);
	xrmd_init_cpu_stack();
	atomic_notifier_chain_register(&panic_notifier_list, &xrmd_panic_block);
	register_trace_android_vh_logbuf(logbuf_vh_handler, NULL);
	return 0;
}

static int __init xr_minidump_init(void)
{
	int ret = 0;

	pr_info("enter xr_minidump init\n");

	ret = xrmd_register_kernel_info();
	if (ret < 0) {
		pr_err("minidump register kernel info failed.\n");
		return ret;
	}

	return ret;
}

static void __exit xr_minidump_exit(void)
{
	pr_info("mini dump exit\n");
	atomic_notifier_chain_unregister(&panic_notifier_list, &xrmd_panic_block);
	unregister_trace_sched_switch(xrmd_update_stack_notifer, NULL);
	unregister_trace_android_vh_logbuf(logbuf_vh_handler, NULL);
	memunmap(g_xrmd_header);
}

module_init(xr_minidump_init);
module_exit(xr_minidump_exit);
MODULE_IMPORT_NS(MINIDUMP);
MODULE_DESCRIPTION("XRing minidmup driver");
MODULE_LICENSE("GPL v2");
