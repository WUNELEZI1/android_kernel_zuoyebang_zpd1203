// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "parklens", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "parklens", __func__, __LINE__

#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

#include "xrisp_rproc_api.h"
#include "xrisp_sensor_dev.h"
#include "xrisp_sensor_ctrl.h"
#include "xrisp_csiphy_ctrl.h"
#include "xrisp_log.h"

#define PARKLENS_MAX_WAIT_TIMEMS (200)

static void xrisp_parklens_lock(struct cam_parklens_ctrl_t *parklens_s)
{
	mutex_lock(&parklens_s->lock);
}

static void xrisp_parklens_unlock(struct cam_parklens_ctrl_t *parklens_s)
{
	mutex_unlock(&parklens_s->lock);
}

static void xrisp_parklens_set_state(struct cam_parklens_ctrl_t *parklens_s, uint32_t state)
{
	if (parklens_s == NULL || state > PARKLENS_STOP)
		return;
	parklens_s->state = state;
}

void xrisp_parklens_init(struct cam_pm_dev *cam_pm)
{
	if (!cam_pm)
		return;

	cam_pm->power_info.parklens.thread = NULL;
	memset(&cam_pm->power_info.parklens.priv, 0, sizeof(struct cam_parklens_priv_t));
	init_completion(&cam_pm->power_info.parklens.complete);
	xrisp_parklens_set_state(&cam_pm->power_info.parklens, PARKLENS_INVALID);
	mutex_init(&cam_pm->power_info.parklens.lock);
}

void xrisp_parklens_state_reset(struct cam_parklens_ctrl_t *parklens_s)
{
	if (!parklens_s)
		return;

	xrisp_parklens_lock(parklens_s);
	parklens_s->thread = NULL;
	kfree(parklens_s->priv.power_settings);
	memset(&parklens_s->priv, 0, sizeof(struct cam_parklens_priv_t));
	reinit_completion(&parklens_s->complete);
	xrisp_parklens_set_state(parklens_s, PARKLENS_INVALID);
	xrisp_parklens_unlock(parklens_s);
}

void xrisp_parklens_exit(struct cam_pm_dev *cam_pm)
{
	if (!cam_pm)
		return;

	xrisp_parklens_state_reset(&cam_pm->power_info.parklens);
	mutex_destroy(&cam_pm->power_info.parklens.lock);
}

static void xrisp_parklens_wait(struct cam_parklens_ctrl_t *parklens_s)
{
	unsigned long timeout = 0;

	if (!parklens_s)
		return;

	timeout = wait_for_completion_timeout(&parklens_s->complete,
					      msecs_to_jiffies(PARKLENS_MAX_WAIT_TIMEMS));
	if (!timeout)
		XRISP_PR_WARN("camid:%d parklens timeout", parklens_s->sensor_id);
	else
		XRISP_PR_INFO("camid:%d parklens complete, wait %d ms", parklens_s->sensor_id,
			      PARKLENS_MAX_WAIT_TIMEMS - jiffies_to_msecs(timeout));
}

int xrisp_parklens_complete(uint32_t sensor_id)
{
	struct cam_pm_dev *cam_pm;

	cam_pm = xrisp_sensor_get_context(sensor_id);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	if (cam_pm->id != sensor_id) {
		XRISP_PR_ERROR("camid:%d mismatch sensor_id:%d!", cam_pm->id, sensor_id);
		return -EINVAL;
	}

	complete(&cam_pm->power_info.parklens.complete);
	XRISP_PR_DEBUG("camid:%d parklens complete", sensor_id);
	return 0;
}

static int32_t xrisp_parklens_thread_func(void *arg)
{
	struct cam_parklens_ctrl_t *parklens_s = arg;
	struct cam_pm_dev *cam_pm = NULL;
	int ret = 0;

	if (!parklens_s) {
		XRISP_PR_ERROR("parklens_s arg is null!");
		return -EINVAL;
	}
	cam_pm = parklens_s->priv.cam_pm;

	XRISP_PR_DEBUG("camid:%d waitting for parklens.", cam_pm->id);

	xrisp_parklens_wait(parklens_s);
	xrisp_rproc_wake_powerdown_wait_cond(cam_pm->id);

	mutex_lock(&cam_pm->mlock);
	xrisp_parklens_lock(parklens_s);
	if (parklens_s->state == PARKLENS_STOP) {
		XRISP_PR_INFO("camid:%d stop powerdown!", cam_pm->id);
		xrisp_parklens_unlock(parklens_s);
		goto skip_power_down;
	} else
		xrisp_parklens_set_state(parklens_s, PARKLENS_POWERDOWN);
	xrisp_parklens_unlock(parklens_s);

	XRISP_PR_DEBUG("camid:%d powerdown start.", cam_pm->id);
	ret = xrisp_sensor_core_power_down(cam_pm->hw_info, &cam_pm->power_info.gpio_seq,
					   parklens_s->priv.power_settings_cnt,
					   parklens_s->priv.power_settings, 0);
	if (ret)
		XRISP_PR_ERROR("camid:%d powerdown failed.", cam_pm->id);
	else
		XRISP_PR_DEBUG("camid:%d powerdown success.", cam_pm->id);

skip_power_down:
	xrisp_parklens_state_reset(parklens_s);
	mutex_unlock(&cam_pm->mlock);

	if (parklens_s->priv.type == POWER_TYPE_SENSOR) {
		xrisp_csiphy_disable();
		if (xrisp_sensor_get_has_aoc_status()) {
			if (cam_pm->hw_info->is_aoc_cam)
				xrisp_power_sync_clear(XRISP_AOC_AP_FRONT_MASKBIT);
			xrisp_power_sync_clear(XRISP_AOC_AP_POWER_MASKBIT);
		}
	}

	return 0;
}

static struct task_struct *xrisp_parklens_thread_run(int32_t (*thread_handler)(void *data),
						     void *data, int sensor_id)
{
	return kthread_run(thread_handler, data, "camid[%d]_parklens", sensor_id);
}

int xrisp_parklens_sensor_power_up(struct cam_parklens_ctrl_t *parklens_s)
{
	int ret = 0;
	int camid = 0;

	if (!parklens_s)
		return -EINVAL;

	camid = parklens_s->sensor_id;
	xrisp_parklens_lock(parklens_s);

	// XRISP_PR_DEBUG("camid:%d parklens state=%d", camid, parklens_s->state);
	if (parklens_s->state == PARKLENS_RUN && parklens_s->thread != NULL) {
		xrisp_parklens_set_state(parklens_s, PARKLENS_STOP);
		XRISP_PR_DEBUG("camid:%d parklens running, stop powerdown", camid);
		ret = PARKLENS_RETURN;
	}
	xrisp_parklens_unlock(parklens_s);
	return ret;
}

int xrisp_parklens_sensor_power_down(struct cam_parklens_ctrl_t *parklens_s,
				     struct xrisp_sensor_power_ctrl *ctrl,
				     struct cam_pm_dev *cam_pm,
				     struct cam_power_setting *power_settings)
{
	int ret = 0;

	if (!parklens_s)
		return -EINVAL;

	xrisp_parklens_lock(parklens_s);

	if (ctrl->need_parklens && (parklens_s->state == PARKLENS_INVALID)) {
		parklens_s->sensor_id = ctrl->sensor_id;
		parklens_s->priv.type = ctrl->power_type;
		parklens_s->priv.power_settings_cnt = ctrl->count;
		parklens_s->priv.power_settings = power_settings;
		parklens_s->priv.cam_pm = cam_pm;

		parklens_s->thread = xrisp_parklens_thread_run(xrisp_parklens_thread_func,
							       parklens_s, ctrl->sensor_id);
		if (IS_ERR(parklens_s->thread)) {
			memset(&parklens_s->priv, 0, sizeof(struct cam_parklens_priv_t));
			reinit_completion(&parklens_s->complete);
			xrisp_parklens_set_state(parklens_s, PARKLENS_INVALID);
			xrisp_rproc_wake_powerdown_wait_cond(cam_pm->id);
			XRISP_PR_ERROR("camid:%d powerdown thread create failed, normal powerdown.",
				       cam_pm->id);
			ret = -EINVAL;
		} else {
			xrisp_parklens_set_state(parklens_s, PARKLENS_RUN);
			XRISP_PR_DEBUG("camid:%d powerdown thread running.", cam_pm->id);
			ret = PARKLENS_RETURN;
		}
	} else
		XRISP_PR_DEBUG("camid:%d normal powerdown.", cam_pm->id);

	xrisp_parklens_unlock(parklens_s);
	return ret;
}
