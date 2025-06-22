// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/component.h>
#include <linux/version.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_gem_dma_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_drv.h>
#include <drm/drm_vblank.h>
#include <drm/drm_atomic_uapi.h>
#include <drm/drm_print.h>

#include "dpu_module_drv.h"
#include "dpu_drv.h"
#include "dpu_kms.h"
#include "dpu_cap.h"
#include "dpu_gem.h"
#include "dpu_format.h"
#include "dpu_power_helper.h"
#include "dpu_cont_display.h"
#include "dpu_exception.h"
#include "dpu_core_perf.h"
#include "dpu_trace.h"
#include "dpu_crtc.h"

#define DRIVER_NAME         "xring_drm"
#define DRIVER_DESC         "Xring DRM Driver"
#define DRIVER_DATE         "20230309"
#define DRIVER_MAJOR        1
#define DRIVER_MINOR        0

#define DPU_COMMIT_WAIT_TIMEOUT (5 * HZ)

void dpu_drm_atomic_commit_tail(struct drm_atomic_state *old_state)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_device *drm_dev;
	struct dpu_kms *dpu_kms;

	drm_dev = old_state->dev;
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	trace_dpu_drm_atomic_commit_tail("commit start");

	dpu_kms->funcs->prepare(dpu_kms, old_state);

	drm_atomic_helper_commit_modeset_disables(drm_dev, old_state);

	drm_atomic_helper_commit_modeset_enables(drm_dev, old_state);

	dpu_kms->funcs->pre_commit(dpu_kms, old_state);

	drm_atomic_helper_commit_planes(drm_dev, old_state,
			DRM_PLANE_COMMIT_ACTIVE_ONLY);

	dpu_kms->funcs->post_commit(dpu_kms, old_state);

	drm_atomic_helper_commit_hw_done(old_state);

	drm_atomic_helper_cleanup_planes(drm_dev, old_state);

	trace_dpu_drm_atomic_commit_tail("before wait cfg done");

	dpu_kms->funcs->wait_for_hw_cfg_done(dpu_kms, old_state);

	dpu_kms->funcs->finish(dpu_kms, old_state);

	trace_dpu_drm_atomic_commit_tail("commit end");
}

static void release_crtc_commit(struct completion *completion)
{
	struct drm_crtc_commit *commit = container_of(completion,
			typeof(*commit), flip_done);

	drm_crtc_commit_put(commit);
}

static struct drm_crtc_commit *alloc_crtc_commit(struct drm_crtc *crtc)
{
	struct drm_crtc_commit *commit;

	commit = kzalloc(sizeof(*commit), GFP_KERNEL);
	if (!commit)
		return NULL;

	init_completion(&commit->flip_done);
	init_completion(&commit->hw_done);
	init_completion(&commit->cleanup_done);
	INIT_LIST_HEAD(&commit->commit_entry);
	kref_init(&commit->ref);
	commit->crtc = crtc;

	return commit;
}

static struct drm_crtc_commit *dpu_drm_atomic_wait_and_get_commit(
		struct drm_atomic_state *state,
		struct drm_crtc_commit *commit,
		struct drm_crtc *crtc,
		bool nonblock)
{
	struct drm_crtc_state *new_crtc_state;
	struct drm_crtc_commit *commit_ptr;
	struct drm_crtc *crtc_ptr;
	int ret;

	if (commit && nonblock) {
		ret = wait_for_completion_timeout(&commit->flip_done,
				DPU_COMMIT_WAIT_TIMEOUT);
		if (ret <= 0) {
			DPU_ERROR("failed to wait flip done, ret %d\n", ret);
			return ERR_PTR(-EBUSY);
		}
	}

	if (crtc) {
		new_crtc_state = drm_atomic_get_new_crtc_state(state, crtc);
		commit_ptr = new_crtc_state->commit;
	} else {
		if (!state->fake_commit) {
			crtc_ptr = NULL;
			state->fake_commit = alloc_crtc_commit(crtc_ptr);
			if (!state->fake_commit)
				return ERR_PTR(-ENOMEM);
		}

		commit_ptr = state->fake_commit;
	}

	return drm_crtc_commit_get(commit_ptr);
}

static int dpu_drm_atomic_crtc_commit_update(struct drm_crtc_state *crtc_state,
		struct drm_crtc_commit *commit)
{
	if (!crtc_state->event) {
		commit->event = kzalloc(sizeof(*commit->event), GFP_KERNEL);
		if (!commit->event)
			return -ENOMEM;

		crtc_state->event = commit->event;
	}

	crtc_state->event->base.completion = &commit->flip_done;
	crtc_state->event->base.completion_release = release_crtc_commit;
	drm_crtc_commit_get(commit);
	commit->abort_completion = true;
	drm_crtc_commit_get(commit);

	return 0;
}

static struct drm_crtc_commit *dpu_drm_atomic_crtc_commit_init(
		struct drm_crtc *crtc,
		struct drm_crtc_state *new_crtc_state,
		bool nonblock)
{
	struct drm_crtc_commit *commit, *commit_flip, *commit_cleanup;
	bool is_valid;
	int ret;

	commit = alloc_crtc_commit(crtc);
	if (!commit)
		return ERR_PTR(-ENOMEM);

	new_crtc_state->commit = commit;

	spin_lock(&crtc->commit_lock);
	commit_flip = list_first_entry_or_null(&crtc->commit_list,
			struct drm_crtc_commit, commit_entry);

	if (!commit_flip) {
		spin_unlock(&crtc->commit_lock);
		return commit;
	}

	drm_crtc_commit_get(commit_flip);

	commit_cleanup = list_next_entry(commit_flip, commit_entry);
	is_valid = !list_entry_is_head(commit_cleanup, &crtc->commit_list, commit_entry);
	if (is_valid)
		commit_cleanup = drm_crtc_commit_get(commit_cleanup);
	spin_unlock(&crtc->commit_lock);

	ret = wait_for_completion_timeout(
			&commit_flip->flip_done,
			DPU_COMMIT_WAIT_TIMEOUT);
	drm_crtc_commit_put(commit_flip);
	if ((ret <= 0) && nonblock) {
		DPU_ERROR("failed to wait flip done, ret %d\n", ret);
		return ERR_PTR(-EBUSY);
	}

	if (is_valid) {
		ret = wait_for_completion_timeout(
				&commit_cleanup->cleanup_done,
				DPU_COMMIT_WAIT_TIMEOUT);
		drm_crtc_commit_put(commit_cleanup);
		if (ret < 0) {
			DPU_ERROR("failed to wait cleanup done, ret %d\n", ret);
			return ERR_PTR(ret);
		}
	}

	return commit;
}

int dpu_drm_atomic_commit_wait_and_update(struct drm_atomic_state *state, bool nonblock)
{
	struct drm_connector_state *old_conn_state, *new_conn_state;
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct drm_crtc_state *old_crtc_state, *new_crtc_state;
	struct drm_crtc_commit *commit, *commit_ptr;
	struct drm_connector *conn;
	struct drm_plane *plane;
	struct drm_crtc *crtc;
	int ret;
	int i;

	for_each_oldnew_crtc_in_state(state, crtc, old_crtc_state, new_crtc_state, i) {
		commit = dpu_drm_atomic_crtc_commit_init(crtc, new_crtc_state, nonblock);
		if (IS_ERR(commit))
			return PTR_ERR(commit);

		if (!old_crtc_state->active && !new_crtc_state->active) {
			complete_all(&commit->flip_done);
			continue;
		}

		ret = dpu_drm_atomic_crtc_commit_update(new_crtc_state, commit);
		if (ret)
			return ret;

		state->crtcs[i].commit = commit;
	}

	for_each_oldnew_connector_in_state(state, conn, old_conn_state, new_conn_state, i) {
		commit_ptr = dpu_drm_atomic_wait_and_get_commit(state, old_conn_state->commit,
				new_conn_state->crtc ?: old_conn_state->crtc, nonblock);
		if (IS_ERR(commit_ptr)) {
			DPU_ERROR("failed to wait connector[%u] done\n", conn->index);
			return PTR_ERR(commit_ptr);
		}

		new_conn_state->commit = commit_ptr;
	}

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i) {
		commit_ptr = dpu_drm_atomic_wait_and_get_commit(state, old_plane_state->commit,
				new_plane_state->crtc ?: old_plane_state->crtc, nonblock);
		if (IS_ERR(commit_ptr)) {
			DPU_ERROR("failed to wait plane[%u] done\n", plane->index);
			return PTR_ERR(commit_ptr);
		}

		new_plane_state->commit = commit_ptr;
	}

	return 0;
}

int dpu_drm_atomic_commit(struct drm_device *dev,
		struct drm_atomic_state *state,
		bool nonblock)
{
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_commit_ctx *commit_ctx;
	struct drm_crtc_state *crtc_state;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	int index;
	int ret;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	ret = dpu_drm_atomic_commit_wait_and_update(state, nonblock);
	if (ret)
		return ret;

	drm_atomic_state_get(state);
	for_each_new_crtc_in_state(state, crtc, crtc_state, i) {
		index = drm_crtc_index(crtc);
		virt_pipeline = &dpu_kms->virt_pipeline[index];
		commit_ctx = &virt_pipeline->commit_ctx;

		dpu_idle_helper_stop();

		flush_workqueue(virt_pipeline->pipe_wq);
		dpu_dbg_flush_workqueue(dpu_kms->dbg_ctx, crtc);

		/* make sure last frame has commited */
		kthread_flush_work(&commit_ctx->commit_work);

		WARN_ON(drm_atomic_helper_swap_state(state, true) != 0);

		mutex_lock(&commit_ctx->commit_lock);

		if (commit_ctx->state)
			drm_atomic_state_put(commit_ctx->state);
		commit_ctx->state = state;

		commit_ctx->commit_flag = nonblock;
		commit_ctx->ctx_frame_no = to_dpu_crtc_state(crtc_state)->frame_no;

		dpu_kms->funcs->process_fences(crtc_state, commit_ctx);

		ret = kthread_queue_work(&commit_ctx->commit_worker,
				&commit_ctx->commit_work);
		if (!ret) {
			DPU_ERROR("queue commit work failed\n");
			dpu_kms_fence_force_signal(crtc, true);
		}
		mutex_unlock(&commit_ctx->commit_lock);

		if (!nonblock)
			kthread_flush_work(&commit_ctx->commit_work);

		break;
	}

	return 0;
}

static void dpu_drm_pending_event_send(struct drm_device *dev,
		struct drm_file *file, struct drm_event *event, u8 *data)
{
	struct dpu_drm_pending_event *pevent;
	int size;
	int ret;

	size = sizeof(struct dpu_drm_pending_event) + event->length;
	pevent = kzalloc(size, GFP_ATOMIC);
	if (!pevent)
		return;

	pevent->base.file_priv = file;
	pevent->base.event = &pevent->event.base;
	pevent->event.base.type = event->type;
	pevent->event.base.length = sizeof(struct dpu_drm_event_res) +
			event->length;

	memcpy(pevent->event.data, data, event->length);

	ret = drm_event_reserve_init_locked(dev, pevent->base.file_priv,
			&pevent->base, &pevent->event.base);
	if (ret) {
		WARN_ONCE(ret != 0, "failed to add pending event 0x%x\n", event->type);
		kfree(pevent);
		return;
	}
	drm_send_event_locked(dev, &pevent->base);
}

void dpu_drm_event_notify(struct drm_device *dev,
		struct drm_event *event, u8 *data)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_drm_event *node;
	unsigned long flag = 0;

	if (!dev || !event || !data) {
		DPU_ERROR("invalid dev %pK, or event %pK, or data %pK\n",
				dev, event, data);
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(dev);

	spin_lock_irqsave(&dev->event_lock, flag);
	list_for_each_entry(node, &dpu_drm_dev->event_list, head) {
		if (node->event.event_type != event->type)
			continue;

		dpu_drm_pending_event_send(dev, node->file, event, data);
	}
	spin_unlock_irqrestore(&dev->event_lock, flag);
}

static int dpu_event_enable(struct drm_device *dev,
		struct dpu_drm_event_req *req,
		struct drm_file *file, bool enable)
{
	struct dpu_kms *dpu_kms;
	struct drm_mode_object *obj;
	int ret;

	dpu_kms = to_dpu_drm_dev(dev)->dpu_kms;
	obj = drm_mode_object_find(dev, file, req->object_id,
			req->object_type);
	if (!obj) {
		DPU_WARN("not find object");
		return -ENOENT;
	}

	ret = dpu_kms->funcs->enable_event(dpu_kms,
			obj, req->event_type, enable);

	drm_mode_object_put(obj);

	return ret;
}

static int dpu_ioctl_vote_bandwidth(struct drm_device *dev,
		void *data, struct drm_file *file)
{
	struct dpu_drm_device *dpu_drm_dev;
	u32 expect_bandwidth_cmd;
	int ret = 0;

	dpu_drm_dev = to_dpu_drm_dev(dev);

	if (!data || !dpu_drm_dev || !dpu_drm_dev->dpu_kms) {
		DPU_ERROR("null ptr\n");
		return -EINVAL;
	}

	expect_bandwidth_cmd = *((u32 *)data);
	ret = dpu_ddr_bandwidth_update(dpu_drm_dev->dpu_kms->core_perf, expect_bandwidth_cmd, DPU_DDR_CHANNEL_NORMAL);

	return ret;
}

static int dpu_ioctl_vote_clk(struct drm_device *dev,
		void *data, struct drm_file *file)
{
	struct dpu_drm_device *dpu_drm_dev;
	u32 clk_profile;
	int ret;

	dpu_drm_dev = to_dpu_drm_dev(dev);
	if (!data || !dpu_drm_dev || !dpu_drm_dev->dpu_kms) {
		DPU_ERROR("null ptr\n");
		return -EINVAL;
	}

	clk_profile = *((u32 *)data);

	dpu_idle_enable_ctrl(false);
	dpu_power_get_helper(BIT(DPU_PARTITION_0));
	ret = dpu_core_perf_update(dpu_drm_dev->dpu_kms->core_perf, DPU_CORE_PERF_CHANNEL_NORMAL,
			clk_profile, true, false);
	dpu_power_put_helper(BIT(DPU_PARTITION_0));
	dpu_idle_enable_ctrl(true);

	return ret;
}

static int dpu_ioctl_drm_debug_mask(struct drm_device *dev,
		void *data, struct drm_file *file)
{
	u32 drm_debug_mask;

	if (!data) {
		DPU_ERROR("null ptr\n");
		return -EINVAL;
	}

	drm_debug_mask = *((u32 *)data);

	/* __drm_debug in <drm/drm_print.h> */
	__drm_debug = drm_debug_mask;

	DPU_INFO("set drm debug mask: 0x%x", drm_debug_mask);
	return 0;
}

static int dpu_check_event_req_valid(struct dpu_drm_event_req *req)
{
	if (!req) {
		DPU_ERROR("invalid event req\n");
		return -EINVAL;
	}

	if (req->event_type <= DRM_PRIVATE_EVENT_TYPE_MIN ||
		req->event_type >= DRM_PRIVATE_EVENT_TYPE_MAX) {
		DPU_ERROR("invalid event type 0x%x\n", req->event_type);
		return -EINVAL;
	}

	return 0;
}

static int dpu_ioctl_register_event(struct drm_device *dev,
		void *data, struct drm_file *file)
{
	struct dpu_drm_device *dpu_drm_dev = to_dpu_drm_dev(dev);
	struct dpu_drm_event_req *req = data;
	struct dpu_drm_event *event, *node;
	bool registered = false;
	unsigned long flag = 0;
	int ret;

	if (dpu_check_event_req_valid(req) != 0)
		return -EINVAL;

	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (!event)
		return -ENOMEM;

	event->file = file;
	memcpy(&event->event, req, sizeof(event->event));

	mutex_lock(&dpu_drm_dev->event_list_lock);
	spin_lock_irqsave(&dev->event_lock, flag);
	list_for_each_entry(node, &dpu_drm_dev->event_list, head) {
		if (node->file != file)
			continue;

		if (node->event.event_type == req->event_type &&
			node->event.object_id == req->object_id) {
			EVENT_DEBUG("event 0x%x object_id %d is already registered\n",
					req->event_type, req->object_id);
			registered = true;
			break;
		}
	}

	if (!registered)
		list_add_tail(&event->head, &dpu_drm_dev->event_list);

	spin_unlock_irqrestore(&dev->event_lock, flag);

	if (registered) {
		kfree(event);
		mutex_unlock(&dpu_drm_dev->event_list_lock);
		return 0;
	}

	ret = dpu_event_enable(dev, req, file, true);
	if (ret) {
		spin_lock_irqsave(&dev->event_lock, flag);
		list_del(&event->head);
		spin_unlock_irqrestore(&dev->event_lock, flag);

		DPU_ERROR("failed to enable event 0x%x object %x id %d\n",
				req->event_type, req->object_type, req->object_id);

		kfree(event);
	} else {
		EVENT_DEBUG("enable event 0x%x object 0x%x id %d\n",
				req->event_type, req->object_type, req->object_id);
	}

	mutex_unlock(&dpu_drm_dev->event_list_lock);

	return ret;
}

static int dpu_ioctl_unregister_event(struct drm_device *dev,
		void *data, struct drm_file *file)
{
	struct dpu_drm_device *dpu_drm_dev = to_dpu_drm_dev(dev);
	struct dpu_drm_event_req *req = data;
	struct dpu_drm_event *node;
	unsigned long flag = 0;
	bool found = false;
	int ret;

	if (dpu_check_event_req_valid(req) != 0)
		return -EINVAL;

	mutex_lock(&dpu_drm_dev->event_list_lock);
	spin_lock_irqsave(&dev->event_lock, flag);
	list_for_each_entry(node, &dpu_drm_dev->event_list, head) {
		if (node->file != file)
			continue;

		if (node->event.event_type == req->event_type &&
			node->event.object_id == req->object_id) {
			found = true;
			break;
		}
	}
	spin_unlock_irqrestore(&dev->event_lock, flag);

	if (!found) {
		DPU_INFO("type %#x, object_id %#x already unregistered\n", req->event_type, req->object_id);
		mutex_unlock(&dpu_drm_dev->event_list_lock);
		return 0;
	}

	ret = dpu_event_enable(dev, req, file, false);
	if (ret) {
		DPU_ERROR("failed to unregister event 0x%x object 0x%x id %d\n",
				req->event_type, req->object_type, req->object_id);
	} else {
		spin_lock_irqsave(&dev->event_lock, flag);
		list_del(&node->head);
		kfree(node);
		spin_unlock_irqrestore(&dev->event_lock, flag);

		EVENT_DEBUG("unregister event 0x%x object 0x%x id %d\n",
				req->event_type, req->object_type, req->object_id);
	}

	mutex_unlock(&dpu_drm_dev->event_list_lock);

	return ret;
}

static void dpu_events_cleanup(struct drm_device *dev,
		struct drm_file *file)
{
	struct dpu_drm_device *dpu_drm_dev = to_dpu_drm_dev(dev);
	struct dpu_drm_event *node, *next;
	unsigned long flag = 0;
	struct list_head free_list;

	if (!dev || !file)
		return;

	INIT_LIST_HEAD(&free_list);

	spin_lock_irqsave(&dev->event_lock, flag);
	list_for_each_entry_safe(node, next, &dpu_drm_dev->event_list, head) {
		if (node->file != file)
			continue;

		list_del(&node->head);
		list_add_tail(&node->head, &free_list);
	}
	spin_unlock_irqrestore(&dev->event_lock, flag);

	list_for_each_entry_safe(node, next, &free_list, head) {
		dpu_event_enable(dev, &node->event, file, false);
		list_del(&node->head);
		kfree(node);
	}
}

static const struct drm_mode_config_funcs dpu_mode_funcs = {
	.fb_create = dpu_drm_gem_fb_create,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = dpu_drm_atomic_commit,
	.get_format_info = dpu_format_info_get,
};

static const struct drm_mode_config_helper_funcs dpu_mode_config_helpers = {
	.atomic_commit_tail = dpu_drm_atomic_commit_tail,
};

static void dpu_drm_mode_config_init(struct drm_device *drm)
{
	drm_mode_config_init(drm);

	drm->mode_config.min_width = MIN_DISPLAY_WIDTH;
	drm->mode_config.min_height = MIN_DISPLAY_HEIGHT;
	drm->mode_config.max_width = MAX_DISPLAY_WIDTH;
	drm->mode_config.max_height = MAX_DISPLAY_HEIGHT;
	drm->mode_config.fb_modifiers_not_supported = false;


	drm->mode_config.funcs = &dpu_mode_funcs;
	drm->mode_config.helper_private = &dpu_mode_config_helpers;
}

int dpu_release(struct inode *inode, struct file *filp)
{
	struct drm_file *file_priv;
	struct drm_minor *minor;
	struct drm_device *dev;

	file_priv = filp->private_data;
	if (!file_priv)
		return -EINVAL;

	minor = file_priv->minor;
	dev = minor->dev;

	dpu_events_cleanup(dev, file_priv);

	return drm_release(inode, filp);
}

void dpu_lastclose(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);

	if (is_cont_display_enabled()) {
		DPU_INFO("continuous display enabled\n");
		dpu_cont_display_res_force_release(dpu_drm_dev->dpu_kms->power_mgr);
	}

	DPU_INFO("shutdown dpu drm device\n");
	drm_atomic_helper_shutdown(drm_dev);
}

static const struct drm_ioctl_desc dpu_ioctls[] = {
	DRM_IOCTL_DEF_DRV(DPU_REGISTER_EVENT, dpu_ioctl_register_event,
			DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DPU_UNREGISTER_EVENT, dpu_ioctl_unregister_event,
			DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DPU_BANDWIDTH, dpu_ioctl_vote_bandwidth,
			DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DPU_CLK_LEVEL, dpu_ioctl_vote_clk,
			DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(DPU_DRM_DEBUG_MASK, dpu_ioctl_drm_debug_mask,
			DRM_UNLOCKED),
};

static const struct file_operations dpu_drm_fops = {
	.owner                = THIS_MODULE,
	.open                 = drm_open,
	.release              = dpu_release,
	.unlocked_ioctl       = drm_ioctl,
	.compat_ioctl         = drm_compat_ioctl,
	.poll                 = drm_poll,
	.read                 = drm_read,
	.llseek               = no_llseek,
	.mmap                 = drm_gem_mmap,
};

static struct drm_driver dpu_drm_drv = {
	.driver_features = DRIVER_MODESET | DRIVER_ATOMIC | DRIVER_GEM,
	.fops = &dpu_drm_fops,
	.lastclose = &dpu_lastclose,

	.dumb_create = dpu_gem_dumb_create,
	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_import_sg_table = dpu_gem_prime_import_sg_table,
	.ioctls             = dpu_ioctls,
	.num_ioctls         = ARRAY_SIZE(dpu_ioctls),

	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
};

#ifdef CONFIG_PM_SLEEP
static char *_get_dpu_pm_string(u32 pm_mode)
{
	char *str;

	switch (pm_mode) {
	case DPU_POWER_MODE_ON:
		str = "ON";
		break;
	case DPU_POWER_MODE_DOZE:
		str = "DOZE";
		break;
	case DPU_POWER_MODE_DOZE_SUSPEND:
		str = "DOZE_SUSPEND";
		break;
	case DPU_POWER_MODE_OFF:
		str = "OFF";
		break;
	default:
		str = "UNKNOW";
		break;
	};

	return str;
}

static int dpu_pm_suspend(struct device *dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_modeset_acquire_ctx ctx;
	struct drm_atomic_state *state = NULL;
	struct drm_connector_list_iter conn_iter;
	struct drm_connector_state *conn_state;
	struct drm_crtc_state *crtc_state;
	struct drm_connector *conn;
	struct dsi_connector *dsi_conn;
	struct drm_device *drm_dev;
	struct dpu_kms *dpu_kms;
	struct drm_plane_state *plane_state;
	struct drm_plane *plane;
	u32 current_power_mode;
	int i, ret = 0, num_crtcs = 0;

	DPU_INFO("Enter\n");

	if (!dev)
		return -EINVAL;

	drm_dev = dev_get_drvdata(dev);
	if (!drm_dev)
		return -EINVAL;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	/* disable dpu idle */
	dpu_idle_enable_ctrl(false);

	drm_kms_helper_poll_disable(drm_dev);

	DRM_MODESET_LOCK_ALL_BEGIN(drm_dev, ctx, 0, ret);

	if (is_cont_display_enabled()) {
		DPU_INFO("continuous display enabled\n");
		dpu_cont_display_res_force_release(dpu_drm_dev->dpu_kms->power_mgr);
	}

	if (dpu_kms->suspend_state)
		drm_atomic_state_put(dpu_kms->suspend_state);
	dpu_kms->suspend_state = drm_atomic_helper_duplicate_state(drm_dev, &ctx);
	if (IS_ERR_OR_NULL(dpu_kms->suspend_state)) {
		ret = PTR_ERR(dpu_kms->suspend_state);
		DPU_ERROR("failed to back up suspend state, %d\n", ret);
		dpu_kms->suspend_state = NULL;
		goto unlock;
	}

	state = drm_atomic_state_alloc(drm_dev);
	if (!state) {
		ret = -ENOMEM;
		DPU_ERROR("failed to allocate state, %d\n", ret);
		goto unlock;
	}

	state->acquire_ctx = &ctx;
	drm_connector_list_iter_begin(drm_dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter) {
		if (conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_conn = to_dsi_connector(conn);
			current_power_mode = dsi_conn->current_power_mode;
			dpu_kms->dsi_pm_mode_backup = current_power_mode;
			DPU_INFO("dsi_curr_pm_mode is %s\n",
					_get_dpu_pm_string(current_power_mode));
		}

		if (!conn->state || !conn->state->crtc ||
				!conn->state->crtc->state ||
				!conn->state->crtc->state->active)
			continue;

		crtc_state = drm_atomic_get_crtc_state(state,
				conn->state->crtc);
		if (IS_ERR_OR_NULL(crtc_state)) {
			DPU_ERROR("failed to get crtc %d state\n",
					conn->state->crtc->base.id);
			drm_connector_list_iter_end(&conn_iter);
			goto unlock;
		}

		if (conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_conn = to_dsi_connector(conn);
			current_power_mode = dsi_conn->current_power_mode;
			if (current_power_mode == DPU_POWER_MODE_DOZE) {
				conn_state = drm_atomic_get_connector_state(state, conn);
				if (IS_ERR_OR_NULL(conn_state)) {
					DPU_ERROR("failed to get dsi connector state\n");
					goto unlock;
				}
				ret = conn->funcs->atomic_set_property(conn, conn_state,
						dsi_conn->power_mode_prop,
						DPU_POWER_MODE_DOZE_SUSPEND);
				if (ret)
					DPU_ERROR("failed to set pm mode prop\n");

				DPU_INFO("dpu in doze state, change to doze suspend\n");
			}
		}

		ret = drm_atomic_add_affected_planes(state, conn->state->crtc);
		if (ret < 0)
			goto unlock;

		conn_state = drm_atomic_get_connector_state(state, conn);
		if (IS_ERR_OR_NULL(conn_state)) {
			DPU_ERROR("failed to get conn state\n");
			goto unlock;
		}
		ret = drm_atomic_set_crtc_for_connector(conn_state, NULL);
		if (ret < 0) {
			DPU_ERROR("failed to unbind connector\n");
			goto unlock;
		}

		ret = drm_atomic_set_mode_for_crtc(crtc_state, NULL);
		if (ret) {
			DPU_ERROR("failed to clear crtc mode info\n");
			goto unlock;
		}

		crtc_state->active = false;
		++num_crtcs;
		DPU_INFO("disable crtc %d\n", drm_crtc_index(conn->state->crtc));
	}
	drm_connector_list_iter_end(&conn_iter);

	if (num_crtcs) {
		for_each_new_plane_in_state(state, plane, plane_state, i) {
			ret = drm_atomic_set_crtc_for_plane(plane_state, NULL);
			if (ret < 0)
				goto unlock;

			drm_atomic_set_fb_for_plane(plane_state, NULL);
		}

		ret = drm_atomic_commit(state);
		if (ret < 0) {
			DPU_ERROR("failed to disable crtcs, %d\n", ret);
			goto unlock;
		}
	}

	dpu_power_suspend();

unlock:
	if (state)
		drm_atomic_state_put(state);

	DRM_MODESET_LOCK_ALL_END(drm_dev, ctx, ret);

	DPU_INFO("Exit\n");
	return ret;
}

static int dpu_pm_resume(struct device *dev)
{
	struct drm_modeset_acquire_ctx ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_device *drm_dev;
	struct drm_atomic_state *state;
	struct dsi_connector *dsi_conn;
	struct drm_connector_state *conn_state;
	struct drm_connector *connector;
	struct drm_connector_state *new_conn_state;
	struct drm_plane *plane;
	struct drm_plane_state *new_plane_state;
	struct drm_crtc *crtc;
	struct drm_crtc_state *new_crtc_state;
	struct dpu_kms *dpu_kms;
	int ret, i, num_crtcs = 0;
	u32 current_power_mode;

	DPU_INFO("Enter\n");

	if (!dev)
		return -EINVAL;

	drm_dev = dev_get_drvdata(dev);
	if (!drm_dev)
		return -EINVAL;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	DRM_MODESET_LOCK_ALL_BEGIN(drm_dev, ctx, 0, ret);

	dpu_power_resume();

	state = dpu_kms->suspend_state;
	if (state) {
		state->acquire_ctx = &ctx;

		for_each_new_plane_in_state(state, plane, new_plane_state, i) {
			state->planes[i].old_state = plane->state;
			ret = drm_atomic_set_crtc_for_plane(new_plane_state, NULL);
			if (ret < 0)
				goto unlock;

			drm_atomic_set_fb_for_plane(new_plane_state, NULL);
		}

		for_each_new_crtc_in_state(state, crtc, new_crtc_state, i) {
			state->crtcs[i].old_state = crtc->state;
			if (new_crtc_state->active) {
				DPU_INFO("need enable crtc %d\n", drm_crtc_index(crtc));
				num_crtcs++;
			}
		}

		for_each_new_connector_in_state(state, connector, new_conn_state, i) {
			state->connectors[i].old_state = connector->state;
			if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
				dsi_conn = to_dsi_connector(connector);
				conn_state = drm_atomic_get_connector_state(state, connector);
				current_power_mode = dsi_conn->current_power_mode;
				if (current_power_mode != dpu_kms->dsi_pm_mode_backup) {
					ret = connector->funcs->atomic_set_property(connector, conn_state,
							dsi_conn->power_mode_prop,
							dpu_kms->dsi_pm_mode_backup);
					if (ret)
						DPU_ERROR("failed to set pm mode prop %d\n",
								dpu_kms->dsi_pm_mode_backup);
					DPU_INFO("restore dsi pm mode %s\n",
							_get_dpu_pm_string(dpu_kms->dsi_pm_mode_backup));
				}
			}
		}
		if (num_crtcs) {
			ret = drm_atomic_commit(state);
			if (ret == -EDEADLK)
				goto unlock;
		}

		state->acquire_ctx = NULL;
		drm_atomic_state_put(dpu_kms->suspend_state);
		dpu_kms->suspend_state = NULL;
	}

unlock:
	DRM_MODESET_LOCK_ALL_END(drm_dev, ctx, ret);

	/* enable hot-plug polling */
	drm_kms_helper_poll_enable(drm_dev);

	/* enable dpu idle */
	dpu_idle_enable_ctrl(true);

	DPU_INFO("Exit\n");
	return 0;
}
#endif

static const struct dev_pm_ops dpu_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dpu_pm_suspend, dpu_pm_resume)
};

static int dpu_drm_bind(struct device *dev)
{
	struct dpu_drm_device *dpu_drm_dev = NULL;
	struct dpu_kms *dpu_kms;
	struct drm_device *drm_dev;
	int ret;

	dpu_drm_dev = devm_drm_dev_alloc(dev, &dpu_drm_drv,
			struct dpu_drm_device, drm_dev);
	if (IS_ERR_OR_NULL(dpu_drm_dev)) {
		ret = PTR_ERR(dpu_drm_dev);
		return ret;
	}

	INIT_LIST_HEAD(&dpu_drm_dev->event_list);
	mutex_init(&dpu_drm_dev->event_list_lock);
	drm_dev = &dpu_drm_dev->drm_dev;
	dev_set_drvdata(dev, drm_dev);

	dpu_drm_mode_config_init(drm_dev);

	/* init dpu kms resource */
	ret = dpu_kms_obj_init(drm_dev);
	if (ret) {
		DPU_ERROR("failed to init dpu kms object\n");
		return ret;
	}

	dpu_kms = dpu_drm_dev->dpu_kms;

	ret = dpu_kms->funcs->init(dpu_kms);
	if (ret) {
		DPU_ERROR("failed to init dpu kms resources\n");
		goto kms_fail;
	}

	/* bind all sub-components */
	ret = component_bind_all(drm_dev->dev, drm_dev);
	if (ret) {
		DPU_ERROR("failed to bind all sub-components, ret %d\n", ret);
		goto bind_fail;
	}

	/* init vblank for all crtcs */
	ret = drm_vblank_init(drm_dev, drm_dev->mode_config.num_crtc);
	if (ret) {
		DPU_ERROR("failed to init vblank, ret %d\n", ret);
		goto vblank_fail;
	}

	drm_mode_config_reset(drm_dev);

	ret = drm_dev_register(drm_dev, 0);
	if (ret) {
		DPU_ERROR("register drm dev failed, ret=%d\n", ret);
		goto vblank_fail;
	}

	/* enable output polling */
	drm_kms_helper_poll_init(drm_dev);

	ret = dpu_idle_ctx_init(dpu_kms);
	if (ret) {
		DPU_ERROR("failed to init dpu idle ctx\n");
		goto err_idle;
	}

	ret = dpu_cont_display_init(drm_dev);
	if (ret) {
		DPU_ERROR("failed to init continuous display, ret %d\n", ret);
		goto err_cont_display;
	}

	ret = dpu_exception_dbg_ctx_init(dpu_kms);
	if (ret) {
		DPU_ERROR("failed to init dbg ctx, ret %d\n", ret);
		goto err_dbg_ctx;
	}

	device_init_wakeup(dev, true);

	DPU_INFO("Exit\n");

	return 0;

err_dbg_ctx:
	if (is_cont_display_enabled()) {
		DPU_INFO("continuous display enabled\n");
		dpu_cont_display_res_force_release(dpu_drm_dev->dpu_kms->power_mgr);
	}
err_cont_display:
	dpu_idle_ctx_deinit();
err_idle:
	drm_kms_helper_poll_fini(drm_dev);
vblank_fail:
	component_unbind_all(drm_dev->dev, drm_dev);
bind_fail:
	drm_mode_config_cleanup(drm_dev);
kms_fail:
	dpu_kms->funcs->deinit(dpu_kms);
	drm_dev_put(drm_dev);
	return ret;
}

static void dpu_drm_unbind(struct device *dev)
{
	struct dpu_drm_device *dpu_drm_dev = NULL;
	struct dpu_kms *dpu_kms;
	struct drm_device *drm_dev;

	drm_dev = dev_get_drvdata(dev);
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	if (is_cont_display_enabled()) {
		DPU_INFO("continuous display enabled\n");
		dpu_cont_display_res_force_release(dpu_drm_dev->dpu_kms->power_mgr);
	}

	device_init_wakeup(dev, false);

	drm_kms_helper_poll_fini(drm_dev);

	drm_dev_unregister(drm_dev);

	drm_atomic_helper_shutdown(drm_dev);

	drm_mode_config_cleanup(drm_dev);

	component_unbind_all(drm_dev->dev, drm_dev);

	dpu_kms->funcs->deinit(dpu_kms);

	dpu_idle_ctx_deinit();

	dpu_exception_dbg_ctx_deinit(dpu_kms);

	dpu_kms_obj_deinit(drm_dev);
}

static const struct component_master_ops dpu_drm_component_ops = {
	.bind = dpu_drm_bind,
	.unbind = dpu_drm_unbind,
};

static int compare_of(struct device *dev, void *data)
{
	struct device_node *np = data;

	return dev->of_node == np;
}

static int dpu_drm_components_add(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct device_node *node;
	struct component_match *match = NULL;
	unsigned int i;

	if (!of_device_is_compatible(dev->of_node, "xring,dpu-kms"))
		return 0;

	for (i = 0; ; i++) {
		node = of_parse_phandle(np, "components", i);
		if (!node)
			break;

		component_match_add(dev, &match, compare_of, node);
	}

	if (!match) {
		DPU_ERROR("cannot find display components\n");
		return -ENODEV;
	}

	return component_master_add_with_match(dev,
			&dpu_drm_component_ops, match);
}

static int dpu_pdev_probe(struct platform_device *pdev)
{
	int ret;

	ret = dma_coerce_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret)
		DPU_ERROR("dma_set_mask_and_coherent failed (%d)\n", ret);

	return dpu_drm_components_add(&pdev->dev);
}

static int dpu_pdev_remove(struct platform_device *pdev)
{
	component_master_del(&pdev->dev, &dpu_drm_component_ops);

	return 0;
}

static const struct of_device_id dt_match[] = {
	{ .compatible = "xring,dpu-kms" },
	{},
};

MODULE_DEVICE_TABLE(of, dt_match);

static struct platform_driver dpu_platform_driver = {
	.probe = dpu_pdev_probe,
	.remove = dpu_pdev_remove,
	.driver = {
		.name = "xring_drm",
		.of_match_table = dt_match,
		.pm = &dpu_pm_ops,
		.suppress_bind_attrs = true,
	},
};

static int __init dpu_drm_register(void)
{
	int ret;

	ret = dp_drv_register();
	if (ret)
		return ret;

	ret = dpu_wb_register();
	if (ret)
		return ret;

	ret = dsi_display_drv_register();
	if (ret)
		return ret;

	return platform_driver_register(&dpu_platform_driver);
}

static void __exit dpu_drm_unregister(void)
{
	platform_driver_unregister(&dpu_platform_driver);
	dsi_display_drv_unregister();
	dpu_wb_unregister();
	dp_drv_unregister();
}

module_init(dpu_drm_register);
module_exit(dpu_drm_unregister);

MODULE_AUTHOR("Zhiqiang Liu <liuzhiqiang6@xiaomi.com>");
MODULE_DESCRIPTION("X-RingTek DRM driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xring_usb");
MODULE_SOFTDEP("pre: xr_regulator");
MODULE_SOFTDEP("pre: pinctrl-xr");
MODULE_SOFTDEP("pre: perf_monitor");
