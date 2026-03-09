#ifndef __MTDOOPS_H__
#define __MTDOOPS_H__

#include <linux/types.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/sched/clock.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/pagemap.h>
#include <linux/backing-dev.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/major.h>
#include <linux/writeback.h>
#include <linux/export.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/stat.h>
#include <linux/console.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/timekeeping.h>
#include <linux/mtd/mtd.h>
#include <linux/kmsg_dump.h>
#include <linux/reboot.h>
#include <linux/platform_device.h>
#include <linux/panic_notifier.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/memblock.h>
#include <linux/pstore_ram.h>
#include <linux/rslib.h>
#include <linux/slab.h>
#include <asm/page.h>
#include <linux/nmi.h>
#include <linux/sched/debug.h>
#include <linux/miscdevice.h>
#include <linux/atomic.h>
#include <linux/ctype.h>
#include <linux/kexec.h>
#include <linux/kmod.h>
#include <linux/suspend.h>
#include <linux/syscalls.h>
#include <linux/syscore_ops.h>

#if defined __GNUC__
#define PACK(x)                           __attribute__((packed)) x
#elif defined __GNUG__
#define PACK(x)                           __attribute__((packed)) x
#elif defined __arm
#define PACK(x)                           __attribute__((packed)) x
#elif defined _WIN32
#define PACK(x)                           x
#else
#define PACK(x)                           __attribute__((packed)) x
#endif

typedef unsigned int                      uint32;
typedef unsigned char                     uint8;
typedef unsigned short                    uint16;
typedef unsigned long                     uint64;

#define MTN_PRINT_ERR(args...)            pr_err("<monitor fail>"args);
#define MTN_PRINT_INFO(args...)           pr_info("<monitor info>"args);
#define MTN_PRINT_START(args...)          pr_info(">>>enter  %s: line: %d.\n", __func__, __LINE__);
#define MTN_PRINT_END(args...)            pr_info("<<<exit  %s: line %d.\n", __func__, __LINE__);
#define countof(arr)                      (sizeof(arr) / sizeof(arr[0]))

#define LOG_TABLE_HEADER_MAGIC            0x56775AF41BCDE0F0

#define MTDOOPS_HEADER_SIZE               512
#define MAX_CMDLINE_PARAM_LEN             128
#define DEFAULT_TIMEOUT	                  5

#define MTDOOPS_START_ADDR	          0x0       //start
#define MTDOOPS_HEADER   	          0x400     //header size
#define PARTITION_KMSG                    0x80000
#define PARTITION_KMSG_SIZE               PARTITION_KMSG - MTDOOPS_HEADER  //512k - 1k
#define PARTITION_PART_SIZE               0x200000  //2 M
#define PARTITION_PMSG_SIZE               PARTITION_PART_SIZE - PARTITION_KMSG - MTDOOPS_HEADER //2 M - 512K 1k

#define DEVICE_NAME "mtd0"
#define MTD_SIZE (16 * 1024 * 1024) // 16MB

extern struct raw_notifier_head pwrkey_irq_notifier_list;
extern char build_fingerprint[MAX_CMDLINE_PARAM_LEN];

struct boot_platform_data {
	unsigned long	mem_size;
	phys_addr_t	mem_address;
	unsigned long	console_size;
	unsigned long	pmsg_size;
};

struct bootmonitor_context {
	struct kmsg_dumper dump;
	struct boot_platform_data monitor_data;
	void *pmsg_buf;
	void *dmesg_buf;
	struct notifier_block reboot_nb;
	struct notifier_block panic_nb;
	struct notifier_block pwrkey_long_press_nb;
};

struct pmsg_buffer_hdr {
	uint32_t    sig;
	atomic_t    start;
	atomic_t    size;
	uint8_t     data[0];
};

enum mtd_dump_reason {
	MTD_DUMP_UNDEF,
	MTD_DUMP_PANIC,
	MTD_DUMP_OOPS,
	MTD_DUMP_EMERG,
	MTD_DUMP_SHUTDOWN,
	MTD_DUMP_RESTART,
	MTD_DUMP_POWEROFF,
	MTD_DUMP_LONG_PRESS,
	MTD_DUMP_MAX
};

static char *kdump_reason[8] = {
	"Unknown",
	"Kernel Panic",
	"Oops!",
	"Emerg",
	"Shut Down",
	"Restart",
	"PowerOff",
	"Long Press"
};

enum mtdoops_log_type {
	MTDOOPS_TYPE_UNDEF,
	MTDOOPS_TYPE_DMESG,
	MTDOOPS_TYPE_PMSG,
};

static char *log_type[4] = {
	"Unknown",
	"LAST KMSG",
	"LAST LOGCAT"
};

struct PACK(mtdoops_header) {
	/* LOG_TABLE_MAGIC */
	uint64 magic;
	uint32 flags;;
};

#endif
