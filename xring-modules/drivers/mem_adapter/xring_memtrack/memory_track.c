// SPDX-License-Identifier: GPL-2.0
#include <linux/err.h>
#include <linux/mmzone.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>

#include "../xring_mem_adapter.h"
#include "memory_track.h"

#define SLUB_NAME_LEN	64

enum {
	SLUB_TRACK,
	VMALLOC_TRACK,
};

struct page_stack_trace {
	int (*page_stack_on)(char *name);
	int (*page_stack_off)(char *name);
	int (*page_stack_open)(int type);
	int (*page_stack_close)(void);
	size_t (*page_stack_read)(struct xr_stack_info *stack_info_list, size_t len, int type);
};

struct mem_trace {
	int type;
	size_t (*get_mem_stats)(void);
	size_t (*get_mem_detail)(void *buf, size_t size);
	struct page_stack_trace *stack_trace;
};

struct caller_item_count {
	unsigned long caller;
	unsigned long cnt;
};

struct page_stack_trace slub_page_stack = {
	.page_stack_on = xr_slub_track_on,
	.page_stack_off = xr_slub_track_off,
	.page_stack_open = xr_slub_stack_open,
	.page_stack_close = xr_slub_stack_close,
	.page_stack_read = xr_slub_stack_read,
};

struct page_stack_trace vmalloc_page_stack = {
	.page_stack_on = xr_vmalloc_track_on,
	.page_stack_off = xr_vmalloc_track_off,
	.page_stack_open = xr_vmalloc_stack_open,
	.page_stack_close = xr_vmalloc_stack_close,
	.page_stack_read = xr_vmalloc_stack_read,
};

struct mem_trace mem_trace[] = {
	{SLUB_TRACK, NULL, NULL, &slub_page_stack},
	{VMALLOC_TRACK, NULL, NULL, &vmalloc_page_stack},
};

static unsigned int memtrack_init_state = XR_MEM_INIT_IDLE;
static char top_slub_name[SLUB_NAME_LEN];
static int slub_type[] = { SLUB_ALLOC, SLUB_FREE };
static const char *const slub_text[] = {"SLUB_ALLOC", "SLUB_FREE"};

void memcheck_save_top_slub(const char *name)
{
	memcpy(top_slub_name, name, sizeof(top_slub_name));
}

size_t get_mem_stats(int type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].get_mem_stats)
			return mem_trace[i].get_mem_stats();
	return 0;
}

size_t get_mem_detail(int type, void *buf, size_t len)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].get_mem_detail)
			return mem_trace[i].get_mem_detail(buf, len);
	return 0;
}

int page_trace_on(int type, char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].stack_trace)
			return mem_trace[i].stack_trace->page_stack_on(name);
	return -EINVAL;
}

int page_trace_off(int type, char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].stack_trace)
			return mem_trace[i].stack_trace->page_stack_off(name);
	return -EINVAL;
}

int page_trace_open(int type, int subtype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].stack_trace)
			return mem_trace[i].stack_trace->page_stack_open(subtype);
	return -EINVAL;
}

int page_trace_close(int type, int subtype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].stack_trace)
			return mem_trace[i].stack_trace->page_stack_close();
	return -EINVAL;
}

size_t page_trace_read(int type, struct xr_stack_info *info,
				size_t len, int subtype)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(mem_trace); i++)
		if (type == mem_trace[i].type && mem_trace[i].stack_trace)
			return mem_trace[i].stack_trace->page_stack_read(info, len, subtype);
	return 0;
}

static size_t memcheck_stack_to_str(struct xr_stack_info *list, size_t num,
				    char *user_payload, size_t user_total, size_t used)
{
	int i;
	int len;

	for (i = 0; i < num; i++) {
		unsigned long *entries;
		int nr_entries, j;

		nr_entries = stack_depot_fetch(list[i].caller, &entries);
		len = snprintf(user_payload + used, user_total - used, "[0x%lx c:(%d/%lu) (p:%ld-%ld) %s]\n",
			list[i].caller, atomic_read(&list[i].ref), list[i].same_caller_cnt,
			list[i].min_pid, list[i].max_pid, list[i].comm);
		if (len < 0)
			goto buf_done;

		used += min((size_t)len, user_total - used - 1);

		for (j = 0; j < nr_entries; j++) {
			len = snprintf(user_payload + used, user_total - used, "  %pS\n", (void *)entries[j]);
			if (len < 0)
				goto buf_done;
			used += min((size_t)len, user_total - used - 1);
			if (used >= user_total - 1)
				break;
		}
	}
buf_done:
	return used;
}

static size_t memcheck_append_str(const char *str, char *user_payload, size_t user_total,
				  size_t used)
{
	int len;

	len = snprintf(user_payload + used, user_total - used, "@@@@@%s %s@@@@@\n", top_slub_name, str);
	if (len < 0)
		return used;
	used += min((size_t)len, user_total - used - 1); /* when snprintf size is not enough, real size is user_total-used-1 */
	return used;
}


static size_t memcheck_get_slub_stack(struct xr_stack_info *stack_info_list, size_t num,
		char *user_payload, size_t user_total)
{
	unsigned int i;
	int ret;
	size_t used = 0; // dont copy to userspace
	size_t ret_num;

	for (i = 0; i < ARRAY_SIZE(slub_type); i++) {
		ret = page_trace_open(SLUB_TRACK, slub_type[i]);
		if (ret) {
			xrmem_err("open slub stack failed\n");
			return used;
		}
		ret_num = page_trace_read(SLUB_TRACK, stack_info_list, num, slub_type[i]);
		/* only use top SLUB_LEAK_TOP_NUM */
		ret_num = min(ret_num, SLUB_LEAK_TOP_NUM);
		if (ret_num == 0) {
			xrmem_info("empty %s stack record\n", slub_text[i]);
			page_trace_close(SLUB_TRACK, slub_type[i]);
			continue;
		}
		ret = page_trace_close(SLUB_TRACK, slub_type[i]);
		if (ret) {
			xrmem_err("close slub track failed\n");
			return used;
		}
		used = memcheck_append_str(slub_text[i], user_payload, user_total, used);
		if (used >= (user_total - 1))
			return used;
		used = memcheck_stack_to_str(stack_info_list, ret_num, user_payload, user_total, used);
		if ((ret_num >= num) || (used >= (user_total - 1))) {
			xrmem_err("memcheck_stack_to_str fail ret_num:%lu num:%lu used:%lu total:%lu\n",
				ret_num, num, used, user_total);
			return used;
		}
	}
	return used;
}

static size_t memcheck_get_vmalloc_stack(struct xr_stack_info *stack_info_list, size_t num,
		char *user_payload, size_t user_total)
{
	int ret;
	size_t used = 0; // dont copy to userspace
	size_t ret_num;

	ret = page_trace_open(VMALLOC_TRACK, 0);
	ret_num = page_trace_read(VMALLOC_TRACK, stack_info_list, num, 0);
	ret = page_trace_close(VMALLOC_TRACK, 0);
	return used;
}

static size_t memcheck_get_stack_items(size_t num, char *user_payload, size_t user_total, struct stack_data *info)
{
	size_t used = 0;
	struct xr_stack_info *stack_info_list = NULL;

	if (info->type == MTYPE_KERN_SLUB) {
		/* use max num rather than num */
		stack_info_list = vzalloc(num * sizeof(*stack_info_list));
		used = memcheck_get_slub_stack(stack_info_list, num, user_payload, user_total);
		vfree(stack_info_list);
	} else if (info->type == MTYPE_KERN_VMALLOC) {
		used = memcheck_get_vmalloc_stack(stack_info_list, num, user_payload, user_total);
	}
	return used;
}

static int memcheck_kernel_stack_read(void *arg, struct stack_data *info)
{
	size_t num;
	size_t len;
	int ret = 0;
	char *payload = NULL;

	num = info->size;
	if (!num) {
		info->size = 0;
		if (copy_to_user(arg, info, sizeof(*info))) {
			xrmem_err("copy to user failed\n");
			return -EFAULT;
		}
		xrmem_err("info size is too small to contain stack data\n");
		return 0;
	}
	payload = vzalloc(info->size + 1); /* extra 1 byte for '\0' */
	if (!payload)
		return -ENOMEM;

	len = memcheck_get_stack_items(SLUB_RINGBUF_LEN, payload, info->size, info); /* use SLUB_RINGBUF_LEN */
	info->size = len; /* store detail stack info to payload */
	if (copy_to_user(arg, info, sizeof(*info))) {
		xrmem_err("copy to user failed\n");
		ret = -EFAULT;
		goto err_buf;
	}
	if (len && copy_to_user((void __user *)(arg + sizeof(struct stack_data)), payload, len)) {
		xrmem_err("payload copy to user failed\n");
		ret = -EFAULT;
		goto err_buf;
	}

err_buf:
	vfree(payload);
	return ret;
}

int memcheck_stack_read(void *arg, struct stack_data *info)
{
	if (info->type & MTYPE_KERNEL)
		return memcheck_kernel_stack_read(arg, info);
	xrmem_err("unsupported operation\n");
	return -EFAULT;
}

int xr_process_leak_read(void *arg)
{
	struct stack_data info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		xrmem_err("copy from_user failed\n");
		return -EFAULT;
	}
	if (!info.size || info.size > MEMCHECK_STACKINFO_MAXSIZE) {
		xrmem_err("wrong size %llu\n", info.size);
		return -EINVAL;
	}
	if (!(info.type & MTYPE_KERNEL)) {
		xrmem_err("wrong type %d\n", info.type);
		return -EINVAL;
	}
	return memcheck_stack_read(arg, &info);
}
EXPORT_SYMBOL(xr_process_leak_read);

int memcheck_do_kernel_command(const struct track_cmd *cmd)
{
	int ret = -EINVAL;

	switch (cmd->cmd) {
	case MEMCMD_ENABLE:
		if (cmd->type == MTYPE_KERN_SLUB)
			ret = page_trace_on(SLUB_TRACK, top_slub_name);
		else if (cmd->type == MTYPE_KERN_VMALLOC)
			ret = page_trace_on(VMALLOC_TRACK, "");

		if (ret)
			xrmem_err("trace on failed, memtype=%d\n", cmd->type);
		break;

	case MEMCMD_DISABLE:
		if (cmd->type == MTYPE_KERN_SLUB)
			ret = page_trace_off(SLUB_TRACK, top_slub_name);
		else if (cmd->type == MTYPE_KERN_VMALLOC)
			ret = page_trace_off(VMALLOC_TRACK, "");

		if (ret)
			xrmem_err("trace off failed, memtype=%d\n", cmd->type);
		break;
	case MEMCMD_PREPARE:
		if (cmd->type == MTYPE_KERN_SLUB)
			ret = get_max_slub();
		else if (cmd->type == MTYPE_KERN_VMALLOC)
			ret = 0;
		if (ret)
			xrmem_err("trace prepare failed, memtype=%d\n", cmd->type);
		break;
	default:
		xrmem_err("Unsupported cmd=%d\n", cmd->cmd);
		break;
	}
	return ret;
}

int memcheck_do_command(const struct track_cmd *cmd)
{
	if (cmd->type & MTYPE_KERNEL)
		return memcheck_do_kernel_command(cmd);
	xrmem_err("unsupported operation\n");
	return 0;
}

int xr_process_leak_detect(const void *arg)
{
	struct track_cmd cmd;

	if (copy_from_user(&cmd, arg, sizeof(cmd))) {
		xrmem_err("copy_from_user failed\n");
		return -EFAULT;
	}
	return memcheck_do_command(&cmd);
}
EXPORT_SYMBOL(xr_process_leak_detect);

int xr_memtrack_init(void)
{
	int ret;

	ret = slub_track_init();
	if (ret) {
		xrmem_err("slub_track_init fail\n");
		return ret;
	}
	ret = vmalloc_track_init();
	if (ret) {
		xrmem_err("vmalloc_track_init fail\n");
		return ret;
	}
	memtrack_init_state = XR_MEM_INIT_DONE;
	return 0;
}
EXPORT_SYMBOL(xr_memtrack_init);

void xr_memtrack_exit(void)
{
	vmalloc_track_exit();
	slub_track_exit();
}
EXPORT_SYMBOL(xr_memtrack_exit);

module_param(memtrack_init_state, uint, 0444);
