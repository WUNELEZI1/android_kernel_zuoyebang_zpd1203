/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MEM_TRACE_H
#define _MEM_TRACE_H

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/page-flags.h>

#include "../xring_mem_adapter.h"

#define IDX_KERN_START			(0)
#define IDX_KERN_SLUB			(IDX_KERN_START + 1)
#define IDX_KERN_VMALLOC		(IDX_KERN_START + 2)
#define MTYPE_KERN_SLUB			(1 << IDX_KERN_SLUB)
#define MTYPE_KERN_VMALLOC		(1 << IDX_KERN_VMALLOC)
#define MTYPE_KERNEL			(MTYPE_KERN_SLUB | MTYPE_KERN_VMALLOC)
#define MEMCHECK_STACKINFO_MAXSIZE	(5 * 1024 * 1024)

#define invalid_slub_type(type) (type != SLUB_ALLOC && type != SLUB_FREE)
#define SLUB_RINGBUF_LEN		SZ_8K
#define SLUB_RINGBUF_HALF		2
#define SLUB_VALID_CALLER_CNT	50
#define SLUB_LEAK_TOP_NUM		25
#define EACH_CALLER_TOP_NUM		3
#define SLUB_STACK_ENTRY_NUM	7
#define SLUB_IGNORED_ENTRY_NUM	5

#define VM_CALLER_HAS_STACK		0x12345678ull
#define VM_STACK_MARKER_STR		"vmalloc_stack "
#define VM_STACK_ENTRY_NUM		7
#define VM_IGNORED_ENTRY_NUM	3

enum {
	SLUB_ALLOC = 0,
	SLUB_FREE,
	NR_SLUB_ID,
};

struct xr_stack_info {
	unsigned long caller;
	unsigned long same_caller_cnt;
	atomic_t ref;
	char comm[2 * sizeof(long)];
	long min_pid;
	long max_pid;
};

enum memcmd {
	MEMCMD_PREPARE = 0,
	MEMCMD_ENABLE,
	MEMCMD_DISABLE,
	MEMCMD_MAX
};

struct track_cmd {
	u32 id;
	u32 type;
	u64 timestamp;
	enum memcmd cmd;
};

struct stack_data {
	int type;
	__u64 size;
	char data[0];
};

bool xr_should_detect_slab_leak(void);
int lmkd_notify(unsigned int value);
void set_alloc_track(unsigned long caller);
void set_free_track(unsigned long caller);
int slub_track_init(void);
void slub_track_exit(void);
int xr_memtrack_init(void);
void xr_memtrack_exit(void);
void memcheck_save_top_slub(const char *name);

int get_max_slub(void);
int xr_slub_track_on(char *name);
int xr_slub_track_off(char *name);
static inline int xr_slub_stack_open(int type)
{
	return 0;
}
static inline int xr_slub_stack_close(void)
{
	return 0;
}
size_t xr_slub_stack_read(struct xr_stack_info *stack_info_list, size_t len, int type);

int xr_vmalloc_track_on(char *name);
int xr_vmalloc_track_off(char *name);
int xr_vmalloc_stack_open(int type);
int xr_vmalloc_stack_close(void);
size_t xr_vmalloc_stack_read(struct xr_stack_info *stack_info_list, size_t len, int type);
int vmalloc_track_init(void);
void vmalloc_track_exit(void);

#endif
