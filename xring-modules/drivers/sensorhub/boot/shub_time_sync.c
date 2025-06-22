// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <soc/xring/xr_timestamp.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>

#define TIMESYNC_DELAY_TIME  360000

struct delayed_work	sh_timesync_delay_wq;

static void sh_timesync_from_kernel(void)
{
	struct sh_dts_kernel_time_sync_info *timesync_info;
	s64 ap_syscount, utc_time;
	ktime_t shub_timestamp;
	unsigned long flags;

	local_irq_save(flags);
	shub_timestamp = xr_timestamp_gettime();
	ap_syscount = ktime_get_boottime();
	utc_time = ktime_get_real();
	local_irq_restore(flags);

	timesync_info = &(shub_get_bsp_dts()->time_sync_info);

	timesync_info->kernel_syscount_ns = (ap_syscount >= 0) ? (uint64_t)ap_syscount : 0;

	timesync_info->shub_timestamp_ns = (uint64_t)shub_timestamp;

	timesync_info->utc_time_ns = (utc_time >= 0) ? (uint64_t)utc_time : 0;

}

static void shub_timesync_ipc_complete(int result, void *arg)
{
	;
}

static void shub_timesync_schedule(struct work_struct *work)
{
	pr_info("shub timesync scheduled task re-schedule");
	sh_timesync_from_kernel();
	sh_ipc_send_msg_async(IPC_VC_AO_NS_SH_AP_DEFAULT, TAG_SH_TIME_SYNC, CMD_SH_TIME_UPDATE, NULL, 0,
				shub_timesync_ipc_complete, NULL);
	queue_delayed_work(system_wq, &sh_timesync_delay_wq, msecs_to_jiffies(TIMESYNC_DELAY_TIME));
}

void shub_timesync_init(void)
{
	INIT_DELAYED_WORK(&sh_timesync_delay_wq, shub_timesync_schedule);
	pr_info("shub timesync scheduled task init done");
}
EXPORT_SYMBOL(shub_timesync_init);

void shub_timesync_start(void)
{
	pr_info("shub timesync scheduled task start");
	sh_timesync_from_kernel();
	queue_delayed_work(system_wq, &sh_timesync_delay_wq, msecs_to_jiffies(TIMESYNC_DELAY_TIME));
}
EXPORT_SYMBOL(shub_timesync_start);

void sh_timesync_cancel(void)
{
	pr_info("shub timesync scheduled task cancel");
	cancel_delayed_work_sync(&sh_timesync_delay_wq);
}
EXPORT_SYMBOL(sh_timesync_cancel);
