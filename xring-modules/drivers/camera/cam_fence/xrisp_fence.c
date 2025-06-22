// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "fence", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "fence", __func__, __LINE__

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <media/v4l2-event.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "cam_cdm/xrisp_cdm.h"
#include "xrisp_debug.h"
#include "xrisp_log.h"

struct fence_manager *pfence_dev;
struct dentry *fence_debugfs;

static void cam_fence_send_v4l2event(uint32_t fence_id, uint32_t status,
				     uint32_t event_cause, void *payload)
{
	struct v4l2_event event;
	struct fence_event_head *ev_head = NULL;

	event.id = FENCE_SYNC_V4L_EVENT_ID;
	event.type = FENCE_SYNC_V4L_EVENT_TYPE;
	ev_head = (struct fence_event_head *)event.u.data;
	ev_head->fence_id = fence_id;
	ev_head->status = status;
	ev_head->event_cause = event_cause;
	memcpy(ev_head->payload_data, payload, USER_PAYLOAD_LEN * sizeof(uint64_t));

	v4l2_event_queue(pfence_dev->vdev, &event);

	XRISP_PR_INFO("v4l2_event send success fence id[%d], status: %d", fence_id,
		status);
}

static void fence_entity_init(uint32_t fence_id, char *fence_name)
{
	struct fence_entity *entity = &pfence_dev->fence_table[fence_id];

	entity->fence_id = fence_id;
	strscpy(entity->name, fence_name, FENCE_NAME_LEN);
	entity->state = FENCE_STATE_CREATE_SUCCESS;
	INIT_LIST_HEAD(&entity->userload_list);
}

static int fence_create(int32_t *fence_id, char *fence_name)
{
	uint32_t id;

	if (fence_id == NULL || fence_name == NULL) {
		XRISP_PR_ERROR("input fence_id or fence_name invalid");
		return -EINVAL;
	}
	id = find_first_zero_bit(pfence_dev->bitmap, FENCE_MAX_NUM);
	if (id < 1 || FENCE_MAX_NUM <= id) {
		XRISP_PR_ERROR("unable create fence, no free fence_id, fence_id[%d] name: %s\n",
		       id, fence_name);
		return -ENOMEM;
	}

	set_bit(id, pfence_dev->bitmap);
	spin_lock_bh(&pfence_dev->fence_spinlocks[id]);
	fence_entity_init(id, fence_name);
	spin_unlock_bh(&pfence_dev->fence_spinlocks[id]);
	*fence_id = id;

	XRISP_PR_INFO("fence id[%d], name: %s created success", id, fence_name);

	return 0;
}
static int fence_destroy(uint32_t fence_id)
{
	struct fence_entity *entity = NULL;
	struct user_callback *pos = NULL;
	struct user_callback *temp = NULL;

	if (fence_id < 1 || FENCE_MAX_NUM <= fence_id) {
		XRISP_PR_ERROR("input fence id out of range, input id: %d, range: 1 ~ %d",
		       fence_id, FENCE_MAX_NUM - 1);
		return -EINVAL;
	}

	entity = &pfence_dev->fence_table[fence_id];

	spin_lock_bh(&pfence_dev->fence_spinlocks[fence_id]);
	if (entity->state == FENCE_STATE_UNCREATE) {
		XRISP_PR_ERROR("destroy uninitialized fence entity id[%d], name: %s", fence_id,
		       entity->name);
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		return -EINVAL;
	}

	if (entity->state == FENCE_STATE_CREATE_SUCCESS)
		XRISP_PR_INFO("destroy active fence entity id[%d], name: %s", fence_id,
			entity->name);

	entity->state = FENCE_STATE_UNCREATE;
	list_for_each_entry_safe(pos, temp, &entity->userload_list, list) {
		list_del_init(&pos->list);
		kfree(pos);
	}
	memset(entity, 0, sizeof(struct fence_entity));
	clear_bit(fence_id, pfence_dev->bitmap);
	INIT_LIST_HEAD(&entity->userload_list);
	spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);

	XRISP_PR_INFO("fence id[%d] destroy success", fence_id);
	return 0;
}

static int fence_register(uint32_t fence_id, void *payload)
{
	struct fence_entity *entity = NULL;
	struct user_callback *user_payload = NULL;
	struct user_callback *pos = NULL;
	struct user_callback *temp = NULL;

	if (fence_id < 1 || FENCE_MAX_NUM <= fence_id) {
		XRISP_PR_ERROR("input fence id out of range, id[%d] range: 1 ~ %d", fence_id,
		       FENCE_MAX_NUM - 1);
		return -EINVAL;
	}

	entity = &pfence_dev->fence_table[fence_id];
	spin_lock_bh(&pfence_dev->fence_spinlocks[fence_id]);

	if (entity->state == FENCE_STATE_UNCREATE) {
		XRISP_PR_ERROR("fence id[%d] uninitialized, register fail", fence_id);
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		return -EINVAL;
	}

	user_payload = kzalloc(sizeof(*user_payload), GFP_KERNEL);
	if (user_payload == NULL) {
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		XRISP_PR_ERROR("failed to allocate memory for user payload");
		return -ENOMEM;
	}
	memcpy(user_payload->payload_data, payload, USER_PAYLOAD_LEN * sizeof(uint64_t));

	if (entity->state == FENCE_STATE_SIGNALED_SUCCESS ||
	    entity->state == FENCE_STATE_SIGNALED_FLUSH ||
	    entity->state == FENCE_STATE_SIGNALED_ERROR) {
		cam_fence_send_v4l2event(fence_id, entity->state,
					 FENCE_SYNC_REG_PAYLOAD_EVENT,
					 user_payload->payload_data);

		XRISP_PR_INFO("fence id[%d] name: %s alreadly signal, register user_load send event",
			fence_id, entity->name);

		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		kfree(user_payload);
		return 0;
	}
	//unable add same callback
	list_for_each_entry_safe(pos, temp, &entity->userload_list, list) {
		if ((user_payload->payload_data[1] == pos->payload_data[1]) &&
		    (user_payload->payload_data[0] == pos->payload_data[0])) {
			spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
			kfree(user_payload);
			return -EALREADY;
		}
	}

	list_add_tail(&user_payload->list, &entity->userload_list);
	spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);

	XRISP_PR_INFO("fence id[%d] name: %s register user_load success", fence_id,
		entity->name);
	return 0;
}

static int fence_deregister(uint32_t fence_id, void *payload)
{
	struct fence_entity *entity = NULL;
	struct user_callback *pos = NULL;
	struct user_callback *temp = NULL;
	struct user_callback user_in;
	bool found = false;

	if (fence_id < 1 || FENCE_MAX_NUM <= fence_id) {
		XRISP_PR_ERROR("input fence id out of range, id[%d] range: 1 ~ %d", fence_id,
		       FENCE_MAX_NUM - 1);
		return -EINVAL;
	}

	entity = &pfence_dev->fence_table[fence_id];

	spin_lock_bh(&pfence_dev->fence_spinlocks[fence_id]);
	memcpy(&user_in.payload_data, payload, USER_PAYLOAD_LEN * sizeof(uint64_t));

	if (entity->state == FENCE_STATE_UNCREATE) {
		XRISP_PR_ERROR("fence id[%d] uninitialized, deregister fail", fence_id);
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		return -EINVAL;
	}

	list_for_each_entry_safe(pos, temp, &entity->userload_list, list) {
		if (pos->payload_data[0] == user_in.payload_data[0] &&
		    pos->payload_data[1] == user_in.payload_data[1]) {
			list_del_init(&pos->list);
			kfree(pos);
			XRISP_PR_INFO("fence id[%d] name: %s deregister user_load success",
				fence_id, entity->name);
			found = true;
		}
	}

	spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
	return found ? 0 : -ENOENT;
}

static int fence_signal(uint32_t fence_id, uint32_t status, uint32_t event_cause)
{
	struct fence_entity *entity = NULL;
	struct user_callback *pos = NULL;
	struct user_callback *temp = NULL;
	bool send = false;

	if (fence_id < 1 || FENCE_MAX_NUM <= fence_id) {
		XRISP_PR_ERROR("input fence id out of range, id[%d] range: 1 ~ %d", fence_id,
		       FENCE_MAX_NUM - 1);
		return -EINVAL;
	}

	if (status != FENCE_STATE_SIGNALED_SUCCESS &&
	    status != FENCE_STATE_SIGNALED_ERROR &&
	    status != FENCE_STATE_SIGNALED_FLUSH) {
		XRISP_PR_ERROR("undefined status , fence id[%d] signal fail", fence_id);
		return -EINVAL;
	}

	entity = &pfence_dev->fence_table[fence_id];
	spin_lock_bh(&pfence_dev->fence_spinlocks[fence_id]);
	if (entity->state == FENCE_STATE_UNCREATE) {
		XRISP_PR_ERROR("fence_entity uninitialized, fence id[%d] signal fail", fence_id);
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		return -EINVAL;
	}

	if (entity->state != FENCE_STATE_CREATE_SUCCESS) {
		XRISP_PR_ERROR("fence id[%d] name: %s already signal", entity->fence_id,
		       entity->name);
		spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);
		return -EALREADY;
	}

	list_for_each_entry_safe(pos, temp, &entity->userload_list, list) {
		cam_fence_send_v4l2event(fence_id, status, event_cause,
					 pos->payload_data);
		list_del_init(&pos->list);
		kfree(pos);
		send = true;
	}
	entity->state = status;
	spin_unlock_bh(&pfence_dev->fence_spinlocks[fence_id]);

	if (send)
		XRISP_PR_INFO("fence id[%d] name: %s status: %d, signal success",
			entity->fence_id, entity->name, entity->state);
	else
		XRISP_PR_INFO("fence id[%d] name: %s status: %d, signal success and callback list is empty",
			entity->fence_id, entity->name, entity->state);

	return 0;
}

int cam_fence_ioctl(void *arg)
{
	int ret = 0;
	struct xrisp_control_arg *cmd = NULL;

	if (!arg) {
		XRISP_PR_ERROR("input ioctl arg invalid");
		return -EINVAL;
	}

	cmd = (struct xrisp_control_arg *)arg;
	mutex_lock(&pfence_dev->fence_ctrl_mutex);
	switch (cmd->op_code) {
	case XRING_FENCE_CREATE: {
		struct fence_create_para create_para;

		if (cmd->size != sizeof(create_para)) {
			ret = -EINVAL;
			goto release_mutex;
		}
		ret = copy_from_user(&create_para, u64_to_user_ptr(cmd->handle),
				     sizeof(create_para));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying from user");
			ret = -EFAULT;
			goto release_mutex;
		}

		create_para.name[FENCE_NAME_LEN - 1] = '\0';
		ret = fence_create(&create_para.fence_id, create_para.name);
		if (ret != 0) {
			XRISP_PR_ERROR("fence create failed");
			ret = -EINVAL;
			goto release_mutex;
		}
		ret = copy_to_user(u64_to_user_ptr(cmd->handle), &create_para,
				   sizeof(create_para));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying to user");
			ret = -EFAULT;
			goto release_mutex;
		}
	} break;

	case XRING_FENCE_DESTORY: {
		uint32_t fence_id;

		ret = copy_from_user(&fence_id, u64_to_user_ptr(cmd->handle),
				     sizeof(fence_id));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying from user");
			ret = -EFAULT;
			goto release_mutex;
		}

		ret = fence_destroy(fence_id);
		if (ret != 0) {
			XRISP_PR_ERROR("fence destroy failed");
			ret = -EINVAL;
			goto release_mutex;
		}
	} break;

	case XRING_FENCE_REGISTER: {
		struct fence_register_para register_para;

		if (cmd->size != sizeof(register_para)) {
			ret = -EINVAL;
			goto release_mutex;
		}
		ret = copy_from_user(&register_para, u64_to_user_ptr(cmd->handle),
				     sizeof(register_para));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying to user");
			ret = -EFAULT;
			goto release_mutex;
		}
		ret = fence_register(register_para.fence_id, register_para.payload_data);
		if (ret != 0) {
			XRISP_PR_ERROR("fence register failed");
			ret = -EINVAL;
			goto release_mutex;
		}
	} break;

	case XRING_FENCE_DEREGISTER: {
		struct fence_register_para deregister_para;

		if (cmd->size != sizeof(deregister_para)) {
			ret = -EINVAL;
			goto release_mutex;
		}
		ret = copy_from_user(&deregister_para, u64_to_user_ptr(cmd->handle),
				     sizeof(deregister_para));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying to user");
			ret = -EFAULT;
			goto release_mutex;
		}
		ret = fence_deregister(deregister_para.fence_id,
				       (void *)deregister_para.payload_data);
		if (ret != 0) {
			XRISP_PR_ERROR("fence deregister failed");
			ret = -EINVAL;
			goto release_mutex;
		}
	} break;

	case XRING_FENCE_SIGNAL: {
		struct fence_signal_para signal_para;

		if (cmd->size != sizeof(signal_para)) {
			ret = -EINVAL;
			goto release_mutex;
		}
		ret = copy_from_user(&signal_para, u64_to_user_ptr(cmd->handle),
				     sizeof(signal_para));
		if (ret < 0) {
			XRISP_PR_ERROR("failed copying to user");
			ret = -EFAULT;
			goto release_mutex;
		}
		ret = fence_signal(signal_para.fence_id, signal_para.status,
				   signal_para.event_cause);
		if (ret != 0) {
			XRISP_PR_ERROR("fence signal failed");
			ret = -EINVAL;
			goto release_mutex;
		}
	} break;

	default:
		XRISP_PR_ERROR("Invalid Opcode: %d", cmd->op_code);
		ret = -EINVAL;
		goto release_mutex;
	}
release_mutex:
	mutex_unlock(&pfence_dev->fence_ctrl_mutex);
	return ret;
}

static ssize_t debug_print_fence(struct file *file, char __user *user_buf, size_t count,
				 loff_t *ppos)
{
	char buf[30];
	int id;
	uint32_t fence_num = 0;
	struct fence_entity *entity = NULL;

	if (*ppos >= sizeof(buf))
		return 0;

	if (pfence_dev != NULL) {
		for (id = 1; id < FENCE_MAX_NUM; id++) {
			entity = &pfence_dev->fence_table[id];
			spin_lock_bh(&pfence_dev->fence_spinlocks[id]);
			if (entity->state != FENCE_STATE_UNCREATE) {
				XRISP_PR_INFO("debug fence info: id[%d], name: %s, state: %d",
					id, entity->name, entity->state);
				fence_num++;
			}
			spin_unlock_bh(&pfence_dev->fence_spinlocks[id]);
		}
	} else
		return -EFAULT;

	if (scnprintf(buf, sizeof(buf), "create fence num=%d\n", fence_num))
		return simple_read_from_buffer(user_buf, count, ppos, buf, sizeof(buf));

	return -EINVAL;
}

static ssize_t debug_print_available_events(struct file *file, char __user *user_buf,
					    size_t count, loff_t *ppos)
{
	char buf[30];
	int num_events = 0;
	struct xrisp_cdm_data *priv;
	struct v4l2_kevent *kev = NULL;
	struct v4l2_kevent *temp = NULL;
	struct fence_event_head *ev_head = NULL;

	if (*ppos >= sizeof(buf))
		return 0;

	priv = container_of(pfence_dev, struct xrisp_cdm_data, fence_dev);
	if (priv == NULL || priv->eventq == NULL) {
		XRISP_PR_ERROR("v4l2_fh not open");
		return -EFAULT;
	}

	num_events = priv->eventq->navailable;
	if (num_events > 0) {
		list_for_each_entry_safe(kev, temp, &priv->eventq->available, list) {
			ev_head = (struct fence_event_head *)kev->event.u.data;
			XRISP_PR_INFO("debug available info: id = %d , status = %d , event_cause = %d",
				ev_head->fence_id, ev_head->status, ev_head->event_cause);
		}
	}
	if (scnprintf(buf, sizeof(buf), "available event num=%d\n", num_events))
		return simple_read_from_buffer(user_buf, count, ppos, buf, sizeof(buf));

	return -EINVAL;
}

static const struct file_operations print_ops = {
	.open = simple_open,
	.read = debug_print_fence,
};
static const struct file_operations num_events_ops = {
	.open = simple_open,
	.read = debug_print_available_events,
};

__maybe_unused int cam_fence_debugfs(void)
{
	struct dentry *debugfs_file;
	int ret = 0;

	fence_debugfs = debugfs_create_dir("fence", xrisp_debugfs_get_root());
	if (!IS_ERR_OR_NULL(fence_debugfs)) {
		debugfs_file = debugfs_create_file("print_fence", 0400, fence_debugfs,
						   NULL, &print_ops);
		if (IS_ERR_OR_NULL(debugfs_file)) {
			XRISP_PR_INFO("print debugfs init failed %ld\n", PTR_ERR(debugfs_file));
			debugfs_remove(fence_debugfs);
			ret = -EINVAL;
			goto out;
		}

		debugfs_file = debugfs_create_file("available_event", 0400, fence_debugfs,
						   NULL, &num_events_ops);
		if (IS_ERR_OR_NULL(debugfs_file)) {
			XRISP_PR_INFO("num_events debugfs init failed %ld\n",
				PTR_ERR(debugfs_file));
			debugfs_remove(fence_debugfs);
			ret = -EINVAL;
			goto out;
		}
	} else {
		XRISP_PR_INFO("fence_debugfs create failed %ld\n", PTR_ERR(fence_debugfs));
		ret = -EINVAL;
	}
out:
	return ret;
}

int cam_fence_close(struct fence_manager *fence_dev)
{
	int fence_id;
	struct fence_entity *entity = NULL;
	int ret;

	mutex_lock(&fence_dev->fence_ctrl_mutex);
	for (fence_id = 1; fence_id < FENCE_MAX_NUM; fence_id++) {
		entity = &(fence_dev->fence_table[fence_id]);
		if (entity->state == FENCE_STATE_CREATE_SUCCESS) {
			ret = fence_signal(fence_id, FENCE_STATE_SIGNALED_ERROR,
					   FENCE_SYNC_RELEASE_EVENT);
			if (ret) {
				XRISP_PR_ERROR("Into fence close, fence id[%d] name:%s signal fail",
				       fence_id, entity->name);
			}
		}
	}

	for (fence_id = 1; fence_id < FENCE_MAX_NUM; fence_id++) {
		entity = &(fence_dev->fence_table[fence_id]);
		if (entity->state != FENCE_STATE_UNCREATE) {
			ret = fence_destroy(fence_id);
			if (ret) {
				XRISP_PR_ERROR("Into fence close, fence id[%d] name:%s clean fail",
				       fence_id, entity->name);
			}
		}
	}

	mutex_unlock(&fence_dev->fence_ctrl_mutex);
	return 0;
}

int cam_fence_init(struct fence_manager *fence_dev, struct video_device *vdev)
{
	int fence_id;
	int ret = 0;

	for (fence_id = 0; fence_id < FENCE_MAX_NUM; fence_id++)
		spin_lock_init(&(fence_dev->fence_spinlocks[fence_id]));

	/* bitmap range: 1 ~ FENCE_MAX_NUM-1 */
	bitmap_zero(fence_dev->bitmap, FENCE_MAX_NUM);
	set_bit(0, fence_dev->bitmap);
	mutex_init(&fence_dev->fence_ctrl_mutex);
	fence_dev->vdev = vdev;

	pfence_dev = fence_dev;

	ret = cam_fence_debugfs();
	XRISP_PR_INFO("fence init finished");
	return ret;
}

int cam_fence_exit(void)
{
	pfence_dev = NULL;
	if (!IS_ERR_OR_NULL(fence_debugfs))
		debugfs_remove(fence_debugfs);
	return 0;
}
