// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 *
 * Description: bootup track stage2 driver
 */

#define pr_fmt(fmt) "[xr_bootup][bootup track]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <soc/xring/xr_timestamp.h>

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif

#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <dt-bindings/xring/platform-specific/common/bootup_track/bootup_track_common.h>

#define BOOTUP_TRACK_PROC_NAME "bootup_track"
#define BOOTUP_TRACK_ARG_MAX 1
#define U64_STR_MAX_LEN 20
#define BOOTUP_TRACK_TRIGGER_VAL 1

#define BOOTUP_TRACK_NO (1000)
#define BOOTUP_TRACK_APPID "31000402203"
#define BOOTUP_TRACK_AUTH_EVT_NAME "secAuthed"
#define BOOTUP_TRACK_AUTH_STATUS_NAME "authStatus"
#define BOOTUP_TRACK_TIMEOUT_EVT_NAME "bootTimeout"
#define BOOTUP_TRACK_TIMEOUT_DDR_FULLBOOT "ddrIsFullBoot"

static void *g_bootup_track_mem_addr;
static bool g_reported;
static struct proc_dir_entry *g_proc_file;
static u64 g_boot_time_limit[STAGE_MAX] = {
	BOOTROM_THRESHOLD,
	XLOADER_THRESHOLD,
	UEFI_THRESHOLD,
	KERNEL_THRESHOLD,
	ANDROID_THRESHOLD,
};
#if IS_ENABLED(CONFIG_MIEV)
static const char *g_boot_evt_para_name[STAGE_MAX] = {
	"bootromTime",
	"xloaderTime",
	"uefiTime",
	"kernelTime",
	"androidTime",
};
#endif
static void record_android_boot_time(void)
{
	struct bootup_track_data *track_data;
	ktime_t android_boot_timestamp;

	track_data = (struct bootup_track_data *)g_bootup_track_mem_addr;
	android_boot_timestamp = xr_timestamp_gettime();

	track_data->boot_time[STAGE_ANDROID] =
		(u64)android_boot_timestamp / NSEC_PER_MSEC;
	pr_info("record android time: %llu\n",
		track_data->boot_time[STAGE_ANDROID]);
}

static bool is_boot_time_abnormal(const struct bootup_track_data *track_data)
{
	size_t i;

	/* ignore booting from fastboot */
	if (track_data->stayed_in_fastboot == 1)
		return false;

	if (track_data->ddr_fullboot == 1)
		g_boot_time_limit[STAGE_XLOADER] += DDR_FULL_BOOT_TIME;

	/* time should be increasing and not exceed limit*/
	if (track_data->boot_time[STAGE_BOOTROM] >
	    g_boot_time_limit[STAGE_BOOTROM])
		return true;

	for (i = STAGE_BOOTROM; i < STAGE_MAX - 1; ++i) {
		if (track_data->boot_time[i] >= track_data->boot_time[i + 1])
			return true;
		if (track_data->boot_time[i + 1] >
		    track_data->boot_time[i] + g_boot_time_limit[i + 1])
			return true;
	}

	return false;
}

static void report_miev_event(enum bootup_track_event event_type,
			      const struct bootup_track_data *track_data)
{
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *event;
	size_t i;

	event = cdev_tevent_alloc(BOOTUP_TRACK_NO);
	if (!event) {
		pr_err("misight event allocate fail\n");
		return;
	}

	cdev_tevent_add_str(event, "appId", BOOTUP_TRACK_APPID);

	switch (event_type) {
	case BOOTUP_TRACK_EVENT_AUTH:
		cdev_tevent_add_str(event, "eventName",
				    BOOTUP_TRACK_AUTH_EVT_NAME);
		cdev_tevent_add_int(event, BOOTUP_TRACK_AUTH_STATUS_NAME,
				    (long)track_data->auth_state);
		cdev_tevent_write(event);
		break;
	case BOOTUP_TRACK_EVENT_BOOT_TIMEOUT:
		cdev_tevent_add_str(event, "eventName",
				    BOOTUP_TRACK_TIMEOUT_EVT_NAME);

		for (i = STAGE_BOOTROM; i < STAGE_MAX; ++i) {
			if (track_data->boot_time[i] > LONG_MAX)
				cdev_tevent_add_int(event,
						    g_boot_evt_para_name[i], LONG_MAX);
			else
				cdev_tevent_add_int(event,
						    g_boot_evt_para_name[i],
						    (long)track_data->boot_time[i]);
		}

		cdev_tevent_add_int(event, BOOTUP_TRACK_TIMEOUT_DDR_FULLBOOT,
					(long)track_data->ddr_fullboot);

		cdev_tevent_write(event);
		break;
	default:
		break;
	}
	cdev_tevent_destroy(event);
#else
	return;
#endif
}

static void report_event(void)
{
	const struct bootup_track_data *track_data =
		(const struct bootup_track_data *)g_bootup_track_mem_addr;

	if (track_data->magic != BOOTUP_TRACK_DATA_HDR_MAGIC) {
		pr_err("invalid track data magic, will not report event\n");
		return;
	}

	if (track_data->auth_state != 0)
		report_miev_event(BOOTUP_TRACK_EVENT_AUTH, track_data);

	if (is_boot_time_abnormal(track_data))
		report_miev_event(BOOTUP_TRACK_EVENT_BOOT_TIMEOUT, track_data);
}

static ssize_t bootup_track_proc_write(struct file *file,
				       const char __user *buffer, size_t count,
				       loff_t *f_pos)
{
	u32 data;

	if (count > BOOTUP_TRACK_ARG_MAX) {
		pr_err("count: %lu too large", count);
		return -EINVAL;
	}

	if (kstrtouint_from_user(buffer, count, 10, &data) < 0) {
		pr_err("Fail to get uint from user");
		return -EFAULT;
	}

	if (data == BOOTUP_TRACK_TRIGGER_VAL && g_reported == false) {
		record_android_boot_time();
		report_event();
		g_reported = true;
	}

	return count;
}

static const struct proc_ops bootup_track_proc_fops = {
	.proc_write = bootup_track_proc_write,
};

static int bootup_track_proc_init(void)
{
	g_proc_file = proc_create(BOOTUP_TRACK_PROC_NAME, 0640, NULL,
				  &bootup_track_proc_fops);
	if (!g_proc_file) {
		pr_err("proc_create failed!\n");
		return -ENOMEM;
	}
	pr_info(" ==> success\n");
	return 0;
}

static int __init bootup_track_stage2_init(void)
{
	int ret;

	g_reported = false;
	g_proc_file = NULL;
	g_bootup_track_mem_addr =
		ioremap_wc(DFX_MEM_BOOT_INFO_ADDR, DFX_MEM_BOOT_INFO_SIZE);

	if (!g_bootup_track_mem_addr) {
		pr_err("bootup_track_mem ioremap error\n");
		return -ENOMEM;
	}

	ret = bootup_track_proc_init();
	if (ret) {
		pr_err("init proc failed!\n");
		iounmap(g_bootup_track_mem_addr);
		g_bootup_track_mem_addr = NULL;
		return ret;
	}

	return 0;
}

static void __exit bootup_track_stage2_exit(void)
{
	if (g_proc_file) {
		proc_remove(g_proc_file);
		g_proc_file = NULL;
	}

	iounmap(g_bootup_track_mem_addr);
	g_bootup_track_mem_addr = NULL;
	pr_info("remove bootup_stage2.ko");
}

module_init(bootup_track_stage2_init);
module_exit(bootup_track_stage2_exit);

MODULE_LICENSE("GPL");
