#ifndef _XR_MEMORY_TRACK_H
#define _XR_MEMORY_TRACK_H

#define IDX_KERN_START			(0)
#define IDX_KERN_SLUB			(IDX_KERN_START + 1)
#define IDX_KERN_VMALLOC		(IDX_KERN_START + 2)
#define MTYPE_KERN_SLUB			(1 << IDX_KERN_SLUB)
#define MTYPE_KERN_VMALLOC		(1 << IDX_KERN_VMALLOC)
#define MTYPE_KERNEL			(MTYPE_KERN_SLUB | MTYPE_KERN_VMALLOC)
#define MEMCHECK_STACKINFO_MAXSIZE	(5 * 1024 * 1024)

#define SLUB_LEAK_TOP_NUM		25
#define SLUB_STACK_ENTRY_NUM	7

#define VM_CALLER_HAS_STACK		0x12345678ull
#define VM_STACK_MARKER_STR		"vmalloc_stack "

enum {
	SLUB_ALLOC = 0,
	SLUB_FREE,
	NR_SLUB_ID,
};

enum memcmd {
	MEMCMD_PREPARE = 0,
	MEMCMD_ENABLE,
	MEMCMD_DISABLE,
	MEMCMD_MAX
};

struct track_cmd {
	unsigned int id;
	unsigned int type;
	unsigned long long timestamp;
	enum memcmd cmd;
};

struct stack_data {
	int type;
	__u64 size;
	char data[0];
};

#endif // _XR_MEMORY_TRACK_H