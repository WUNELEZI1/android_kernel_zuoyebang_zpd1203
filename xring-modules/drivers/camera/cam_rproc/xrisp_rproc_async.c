// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "rproc_async", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/remoteproc.h>
#include <linux/delay.h>

#include "xrisp_rproc.h"
#include "remoteproc_internal.h"
#include "xrisp_rproc_api.h"
#include "xrisp_sensor_dev.h"
#include "xrisp_log.h"

#define ISP_POWERDOWN_MAX_WAIT_TIMEMS (400)  //isp powerdown wait parklens max time
#define ISP_POWERUP_MAX_WAIT_TIMEMS   (2000) //isp powerup wait powerdown_thread max time

enum ISP_ASYNC_POWERDOWN_STATE {
	ISP_POWERDOWN_DONE,	    //init or powerdown done
	ISP_POWERDOWN_DIRECT_START, //direct powerdown start, not interruptible
	ISP_POWERDOWN_ASYNC_START,  //thread powerdown start
	ISP_POWERDOWN_SKIP,	    //powerdown waitting, boot start, skip powerdown
};

struct rproc_async_ctrl {
	wait_queue_head_t powerdown_waitqueue;
	struct mutex isp_mtx;
	atomic_t parklens_wait_cnt;
	atomic_t parklens_done_cnt;
	uint32_t poff_wait_cnt;
	uint32_t power_state;
	struct completion thread_powerdown_done;
	struct task_struct *powerdown_thread;
};

struct rproc_async_ctrl *g_rproc_async_ctl;

static void xrisp_trace_sensor_power_state(void)
{
	struct cam_power_ctrl_record *power_record;
	struct cam_pm_dev *cam_pm;
	int i;

	for (i = 0; i < CAM_SENSOR_NUM_MAX; i++) {
		cam_pm = xrisp_sensor_get_context(i);
		if (!cam_pm)
			continue;

		power_record = &cam_pm->power_info.power_record[POWER_TYPE_SENSOR];
		XRISP_PR_DEBUG("camid:%d power count:%d.", cam_pm->id,
			       atomic_read(&power_record->power_cnt));
	}
}

static struct rproc_async_ctrl *xrisp_rproc_get_async_ctl(void)
{
	return g_rproc_async_ctl;
}

void xrisp_rproc_add_powerdown_wait_cond(uint32_t camid)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();

	if (!rp_async_ctl)
		return;

	XRISP_PR_INFO("camid[%d] need parklens, wait_cnt[%d]", camid,
		      atomic_inc_return(&rp_async_ctl->parklens_wait_cnt));
}

void xrisp_rproc_wake_powerdown_wait_cond(uint32_t camid)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();

	if (!rp_async_ctl)
		return;

	XRISP_PR_INFO("camid[%d] parklens done, done_cnt[%d]", camid,
		      atomic_inc_return(&rp_async_ctl->parklens_done_cnt));

	wake_up_interruptible(&rp_async_ctl->powerdown_waitqueue);
}

static void xrisp_rproc_reset_powerdown_cond(void)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();

	if (!rp_async_ctl)
		return;
	atomic_set(&rp_async_ctl->parklens_done_cnt, 0);
	rp_async_ctl->poff_wait_cnt = 0;
}

static int xrisp_rproc_start(struct rproc *rproc, const struct firmware *fw)
{
	struct rproc_subdev *subdev;
	struct rproc_mem_entry *entry, *tmp;
	struct resource_table *rsc_tab;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct xrisp_rproc_rsc_table *xrisp_rsc_table;
	struct xrisp_rproc_ramlog *ramlog;
	int ret;

	//clean bss
	list_for_each_entry_safe(entry, tmp, &rproc->carveouts, node) {
		struct cam_rproc_mem *cam_mem = entry->priv;

		if (cam_mem->da.region == RPROC_MEM_REGION_BOOTADDR) {
			if (cam_mem->is_iomem)
				memset_io(cam_mem->va, 0, cam_mem->da.size);
			else
				memset(cam_mem->va, 0, cam_mem->da.size);
			XRISP_PR_DEBUG("clean bss");
		}
	}

	rproc->ops->load(rproc, fw);

	rsc_tab = rproc_da_to_va(rproc, XRISP_RPROC_RSC_ADDR, XRISP_RPROC_RSC_SIZE, NULL);
	memcpy(rsc_tab, rproc->cached_table, rproc->table_sz);
	rproc->table_ptr = rsc_tab;

	ramlog = &xrisp_rproc->ramlog;
	ramlog->avail = true;
	xrisp_rsc_table = (struct xrisp_rproc_rsc_table *)rproc->table_ptr;
	ramlog->logmem = xrisp_rproc_find_carveout_by_name(rproc, xrisp_rsc_table->log_trace.name);

	list_for_each_entry(subdev, &rproc->subdevs, node) {
		if (subdev->prepare) {
			ret = subdev->prepare(subdev);
			if (ret)
				goto unroll_sub_unprepare;
		}
	}

	//start
	xrisp_m2crg_ip_dereset();
	xrisp_ispmodule_dereset();
	rproc->ops->start(rproc);

	list_for_each_entry(subdev, &rproc->subdevs, node) {
		if (subdev->start) {
			ret = subdev->start(subdev);
			if (ret)
				goto unroll_sub_stop;
		}
	}

	rproc->state = RPROC_RUNNING;
	XRISP_PR_INFO("remote processor %s is now up", rproc->name);

	return 0;

unroll_sub_stop:
	list_for_each_entry_continue_reverse(subdev, &rproc->subdevs, node) {
		if (subdev->stop)
			subdev->stop(subdev, true);
	}

	rproc->ops->stop(rproc);

unroll_sub_unprepare:
	list_for_each_entry_continue_reverse(subdev, &rproc->subdevs, node) {
		if (subdev->unprepare)
			subdev->unprepare(subdev);
	}

	return ret;
}

static int xrisp_rproc_stop(struct rproc *rproc)
{
	struct rproc_subdev *subdev;
	int ret;

	list_for_each_entry_reverse(subdev, &rproc->subdevs, node) {
		if (subdev->stop)
			subdev->stop(subdev, false);
	}

	ret = rproc->ops->stop(rproc); //isp_crg ip_reset
	if (ret) {
		XRISP_PR_ERROR("can't stop rproc: %d", ret);
		return ret;
	}
	xrisp_m2crg_ip_reset();

	list_for_each_entry_reverse(subdev, &rproc->subdevs, node) {
		if (subdev->unprepare)
			subdev->unprepare(subdev);
	}

	rproc->state = RPROC_OFFLINE;
	XRISP_PR_INFO("rproc %s is offline", rproc->name);
	return 0;
}

__maybe_unused int xrisp_rproc_fast_reboot(struct rproc *rproc)
{
	const struct firmware *fw_p;
	struct device *dev = &rproc->dev;
	int ret;

	XRISP_PR_DEBUG("rproc fast reboot start");

	ret = request_firmware(&fw_p, rproc->firmware, dev);
	if (ret < 0) {
		XRISP_PR_ERROR("request_firmware failed: %d", ret);
		return ret;
	}

	ret = mutex_lock_interruptible(&rproc->lock);
	if (ret)
		goto release_fw;

	ret = xrisp_rproc_stop(rproc);
	if (ret)
		goto unlock_mutex;

	ret = xrisp_rproc_start(rproc, fw_p);
	if (ret)
		goto unlock_mutex;

unlock_mutex:
	mutex_unlock(&rproc->lock);
release_fw:
	release_firmware(fw_p);
	return ret;
}

static int xrisp_async_powerdown_thread_stop(void)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	int ret = 0;

	if (!rp_async_ctl)
		return -EINVAL;

	mutex_lock(&rp_async_ctl->isp_mtx);

	if (rp_async_ctl->power_state == ISP_POWERDOWN_ASYNC_START) {
		rp_async_ctl->power_state = ISP_POWERDOWN_DIRECT_START;
		XRISP_PR_INFO("isp into direct powerdown, uninterruptible");
	} else if (rp_async_ctl->power_state == ISP_POWERDOWN_SKIP) {
		ret = true;
		XRISP_PR_INFO("isp fast reboot, skip powerdown");
	} else {
		rp_async_ctl->power_state = ISP_POWERDOWN_DIRECT_START;
		XRISP_PR_ERROR("isp unknown state=%d, into direct_powerdown",
			       rp_async_ctl->power_state);
	}

	mutex_unlock(&rp_async_ctl->isp_mtx);

	return ret;
}

static bool xrisp_rproc_boot_skip_powerdown(uint32_t scene)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	bool skip_powerdown = false;

	if (!rp_async_ctl)
		return -EINVAL;

	if (scene != XRISP_NORMAL_BOOT)
		return false;

	mutex_lock(&rp_async_ctl->isp_mtx);

	if (rp_async_ctl->power_state == ISP_POWERDOWN_ASYNC_START) {
		rp_async_ctl->power_state = ISP_POWERDOWN_SKIP;
		skip_powerdown = true;
	}

	mutex_unlock(&rp_async_ctl->isp_mtx);

	return skip_powerdown;
}

void xrisp_rproc_async_state_release(void)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();

	if (!rp_async_ctl)
		return;

	atomic_set(&rp_async_ctl->parklens_done_cnt, 0);
	atomic_set(&rp_async_ctl->parklens_wait_cnt, 0);
	reinit_completion(&rp_async_ctl->thread_powerdown_done);
	rp_async_ctl->power_state = ISP_POWERDOWN_DONE;
	rp_async_ctl->poff_wait_cnt = 0;
	rp_async_ctl->powerdown_thread = NULL;
}

int xrisp_rproc_shutdown_wait_parklens_done(void)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	int need_wait_cnt;
	int ret;

	if (!rp_async_ctl)
		return -EINVAL;

	need_wait_cnt = rp_async_ctl->poff_wait_cnt - atomic_read(&rp_async_ctl->parklens_done_cnt);
	if (need_wait_cnt == 0) {
		xrisp_rproc_reset_powerdown_cond();
		XRISP_PR_DEBUG("all parklens done");
		return 0;
	}

	XRISP_PR_INFO("need wait parklens cnt=%d", need_wait_cnt);
	ret = wait_event_interruptible_timeout(rp_async_ctl->powerdown_waitqueue,
					       (atomic_read(&rp_async_ctl->parklens_done_cnt) ==
						rp_async_ctl->poff_wait_cnt),
					       msecs_to_jiffies(ISP_POWERDOWN_MAX_WAIT_TIMEMS));
	if (ret > 0)
		XRISP_PR_DEBUG("all parklens done, wait %d ms",
			       ISP_POWERDOWN_MAX_WAIT_TIMEMS - jiffies_to_msecs(ret));
	else {
		XRISP_PR_WARN("wait all parklens done timeout");
		xrisp_trace_sensor_power_state();
	}

	xrisp_rproc_reset_powerdown_cond();

	return 0;
}

static int xrisp_async_powerdown_thread(void *data)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	int ret = 0;

	if (xrisp_rproc_shutdown_wait_parklens_done())
		return -EINVAL;

	if (xrisp_async_powerdown_thread_stop())
		goto thread_stop;

	ret = xrisp_rproc_shutdown();

thread_stop:
	mutex_lock(&rp_async_ctl->isp_mtx);
	rp_async_ctl->powerdown_thread = NULL;
	rp_async_ctl->power_state = ISP_POWERDOWN_DONE;
	mutex_unlock(&rp_async_ctl->isp_mtx);

	complete(&rp_async_ctl->thread_powerdown_done);
	return ret;
}

__maybe_unused int xrisp_rproc_async_shutdown(bool direct_powerdown)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	int ret = 0;

	if (!rp_async_ctl || !g_xrproc)
		return -EINVAL;

	if (atomic_read(&g_xrproc->rproc->power) == 0)
		return -EBUSY;

	if (g_xrproc->scene != XRISP_NORMAL_BOOT || direct_powerdown)
		goto direct_powerdown;

	rp_async_ctl->poff_wait_cnt = atomic_read(&rp_async_ctl->parklens_wait_cnt);
	atomic_set(&rp_async_ctl->parklens_wait_cnt, 0);

	if (rp_async_ctl->poff_wait_cnt == 0 ||
	    (rp_async_ctl->poff_wait_cnt == atomic_read(&rp_async_ctl->parklens_done_cnt)))
		goto direct_powerdown;

	reinit_completion(&rp_async_ctl->thread_powerdown_done);
	rp_async_ctl->power_state = ISP_POWERDOWN_ASYNC_START;

	rp_async_ctl->powerdown_thread =
		kthread_run(xrisp_async_powerdown_thread, NULL, "isp_powerdown_th");
	if (IS_ERR(rp_async_ctl->powerdown_thread)) {
		XRISP_PR_ERROR(
			"create isp_powerdown thread failed, waitting parklens, goto direct powerdown");
		xrisp_rproc_shutdown_wait_parklens_done();
	} else {
		XRISP_PR_DEBUG("isp powerdown thread start");
		return 0;
	}

direct_powerdown:
	XRISP_PR_DEBUG("isp direct powerdown start");
	xrisp_rproc_reset_powerdown_cond();
	rp_async_ctl->powerdown_thread = NULL;
	rp_async_ctl->power_state = ISP_POWERDOWN_DIRECT_START;
	ret = xrisp_rproc_shutdown();
	rp_async_ctl->power_state = ISP_POWERDOWN_DONE;
	return ret;
}

int xrisp_rproc_async_boot(uint32_t scene)
{
	struct rproc_async_ctrl *rp_async_ctl = xrisp_rproc_get_async_ctl();
	bool skip_powerdown = false;
	unsigned long timeout;

	if (!rp_async_ctl || !g_xrproc)
		return -EINVAL;

	mutex_lock(&rp_async_ctl->isp_mtx);
	if (rp_async_ctl->power_state == ISP_POWERDOWN_DONE) {
		mutex_unlock(&rp_async_ctl->isp_mtx);
		XRISP_PR_INFO("isp direct boot");
		goto direct_boot;
	}
	mutex_unlock(&rp_async_ctl->isp_mtx);

	skip_powerdown = xrisp_rproc_boot_skip_powerdown(scene);

	timeout = wait_for_completion_timeout(&rp_async_ctl->thread_powerdown_done,
					      msecs_to_jiffies(ISP_POWERUP_MAX_WAIT_TIMEMS));
	if (timeout)
		XRISP_PR_INFO("isp powerdown_thread complete, wait %d ms, boot start",
			      ISP_POWERUP_MAX_WAIT_TIMEMS - jiffies_to_msecs(timeout));
	else {
		XRISP_PR_ERROR("wait isp powerdown_thread timeout, boot failed");
		return -ETIMEDOUT;
	}

	if (skip_powerdown) {
		XRISP_PR_INFO("skip powerdown, fast reboot");
		return xrisp_rproc_fast_reboot(g_xrproc->rproc);
	}

direct_boot:
	return xrisp_rproc_boot(scene);
}

int xrisp_rproc_async_ctl_init(void)
{
	g_rproc_async_ctl = kzalloc(sizeof(struct rproc_async_ctrl), GFP_KERNEL);
	if (!g_rproc_async_ctl) {
		XRISP_PR_ERROR("g_rproc_async_ctl alloc failed\n");
		return -ENOMEM;
	}

	mutex_init(&g_rproc_async_ctl->isp_mtx);
	init_completion(&g_rproc_async_ctl->thread_powerdown_done);
	init_waitqueue_head(&g_rproc_async_ctl->powerdown_waitqueue);
	atomic_set(&g_rproc_async_ctl->parklens_wait_cnt, 0);
	atomic_set(&g_rproc_async_ctl->parklens_done_cnt, 0);
	g_rproc_async_ctl->poff_wait_cnt = 0;
	g_rproc_async_ctl->power_state = ISP_POWERDOWN_DONE;
	g_rproc_async_ctl->powerdown_thread = NULL;

	return 0;
}

void xrisp_rproc_async_ctl_exit(void)
{
	if (!g_rproc_async_ctl)
		return;

	mutex_destroy(&g_rproc_async_ctl->isp_mtx);
	kfree(g_rproc_async_ctl);
	g_rproc_async_ctl = NULL;
}
