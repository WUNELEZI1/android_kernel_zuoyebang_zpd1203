// SPDX-License-Identifier: GPL-2.0-or-later
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/of_platform.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/genalloc.h>
#include <linux/dma-buf.h>
#include "xrp_hw.h"
#include <linux/iommu.h>
#include "soc/xring/xring_smmu_wrapper.h"
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <linux/dma-mapping.h>
#include <linux/mailbox_client.h>
#include "xrp_internal.h"
#include "xrp_kernel_defs.h"
#include <soc/xring/dfx_switch.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>

#define VDSP_LOG_LEVEL_ASSERT				0x01
#define VDSP_LOG_LEVEL_ERROR				0x02
#define VDSP_LOG_LEVEL_WARN					0x04
#define VDSP_LOG_LEVEL_INFO					0x08
#define VDSP_LOG_LEVEL_DEBUG				0x10
#define VDSP_LOG_LEVEL_MAX					0x20



#if IS_ENABLED(CONFIG_XRING_DEBUG)
int log_level = LOG_LEVEL_INFO;
module_param(log_level, int, 0644);
MODULE_PARM_DESC(log_level, "Control log level (0=none, 1=info, ...)");

static int vdsp_log_level = 20;
module_param(vdsp_log_level, int, 0644);
MODULE_PARM_DESC(vdsp_log_level, "Control log level (0=none, 1=assert, ...)");
#else
int log_level = LOG_LEVEL_INFO;
static int vdsp_log_level = 2;
#endif

#if KERNEL_VERSION_GREATER_THAN_6_6_0
MODULE_IMPORT_NS(DMA_BUF);
#endif

#define MAX_WAIT_TIME 100000 // Maximum wait time in milliseconds

#define DRIVER_NAME "priority_queue"
#define MAX_QUEUES 10
#define MAX_RETRIES 30
#define RETRY_DELAY_MS 1

struct mutex print_func_lock;
static bool is_mbx_send_ok;

/**************** wait queue define begin***************/
static DECLARE_WAIT_QUEUE_HEAD(wq_vdsp_poweron_done);
static bool vdsp_poweron_done;
static DECLARE_WAIT_QUEUE_HEAD(wq_vdsp_poweroff_done);
static bool vdsp_poweroff_done;
static DECLARE_WAIT_QUEUE_HEAD(wq_task_first_open);
static bool task_first_open;
static DECLARE_WAIT_QUEUE_HEAD(wq_task_last_close);
static bool task_last_close;
static DECLARE_WAIT_QUEUE_HEAD(wq_task_ready);
static bool task_ready;

static struct sys_nocache_heap_data common_data = {
								VDSP_COMM,
								sizeof(struct xrp_vdsp_cmds) + SHAKEHAND_OFFSET,
								COMMON_HEAP
								};


int get_log_level(void)
{
	return log_level;
}

int task_ready_action(bool should_wake_up)
{
	int ret = 0;

	if (should_wake_up) {
		xrp_print(LOG_LEVEL_DEBUG,
			"task_ready: Preparing to wake up waiting processes.\n");
		task_ready = true;
		wake_up_interruptible(&wq_task_ready);
		return 0;
	}
	xrp_print(LOG_LEVEL_DEBUG, "task_ready: Waiting for task_ready condition.\n");
	ret = wait_event_interruptible(wq_task_ready, task_ready || kthread_should_stop());
	if (ret == 0)
		task_ready = false;
	return ret;
}

int vdsp_poweron_done_action(bool should_wake_up)
{
	int ret = 0;
	//unsigned long timeout = msecs_to_jiffies(100000);

	if (should_wake_up) {
		xrp_print(LOG_LEVEL_DEBUG,
			"%s VDSP power-on: Preparing to wake up waiting processes.\n", __func__);
		vdsp_poweron_done = true;
		wake_up_interruptible(&wq_vdsp_poweron_done);
		return 0;
	}
	xrp_print(LOG_LEVEL_DEBUG, "VDSP power-on: Waiting for power-on condition.\n");
	ret = wait_event_interruptible(wq_vdsp_poweron_done, vdsp_poweron_done);
	// ret = wait_event_interruptible_timeout(wq_vdsp_poweron_done, vdsp_poweron_done, timeout);

	if (ret != 0) {
		vdsp_poweron_done = false;
		xrp_print(LOG_LEVEL_DEBUG, "VDSP power-on: fail, ret is != 0, ret =%d\n", ret);
		return ret;
	}
	if (ret == 0) {
		vdsp_poweron_done = false;
		xrp_print(LOG_LEVEL_DEBUG, "VDSP power-on: ok, ret is %d\n", ret);
	}

	return ret;
}
EXPORT_SYMBOL(vdsp_poweron_done_action);

int vdsp_poweroff_done_action(bool should_wake_up)
{
	int ret = 0;
	unsigned long timeout = msecs_to_jiffies(10000);

	if (should_wake_up) {
		xrp_print(LOG_LEVEL_DEBUG,
			"%s VDSP power-off: Preparing to wake up waiting processes.\n", __func__);
		vdsp_poweroff_done = true;
		wake_up_interruptible(&wq_vdsp_poweroff_done);
		return 0;
	}
	xrp_print(LOG_LEVEL_DEBUG, "VDSP power-off: Waiting for power-off condition.\n");
	ret = wait_event_interruptible_timeout(wq_vdsp_poweroff_done, vdsp_poweroff_done, timeout);
	if (ret >= 0)
		vdsp_poweroff_done = false;
	return ret;
}
EXPORT_SYMBOL(vdsp_poweroff_done_action);

int task_first_open_action(bool should_wake_up)
{
	int ret = 0;

	if (should_wake_up) {
		xrp_print(LOG_LEVEL_DEBUG,
			"%s Task first open: Preparing to wake up waiting processes.\n", __func__);
		task_first_open = true;
		wake_up_interruptible(&wq_task_first_open);
		return 0;
	}
	xrp_print(LOG_LEVEL_DEBUG, "Task first open: Waiting for first open condition.\n");
	ret = wait_event_interruptible(wq_task_first_open, task_first_open);
	if (ret == 0)
		task_first_open = false;
	return ret;
}
EXPORT_SYMBOL(task_first_open_action);

int task_last_close_action(bool should_wake_up)
{
	int ret = 0;

	if (should_wake_up) {
		xrp_print(LOG_LEVEL_DEBUG,
			"%s Task last close: Preparing to wake up waiting processes.\n", __func__);
		task_last_close = true;
		wake_up_interruptible(&wq_task_last_close);
		return 0;
	}
	xrp_print(LOG_LEVEL_DEBUG, "Task last close: Waiting for last close condition.\n");
	ret = wait_event_interruptible(wq_task_last_close, task_last_close);
	if (ret == 0)
		task_last_close = false;
	return ret;
}
EXPORT_SYMBOL(task_last_close_action);

/**************** wait queue define end***************/
static atomic_t vdsp_if_exception = ATOMIC_INIT(0);
static atomic_t vdsp_if_poweron = ATOMIC_INIT(0);
static atomic_t msg_id_counter = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(vdsp_wq);
static DECLARE_WAIT_QUEUE_HEAD(mbx_wq);
static bool interrupt_occurred;
static struct wakeup_source *g_vdsp_sys_pm_wksrc;

static bool is_wksrc_active(void)
{
	if ((g_vdsp_sys_pm_wksrc != NULL) && (g_vdsp_sys_pm_wksrc->active))
		return true;

	xrp_print(LOG_LEVEL_DEBUG, "g_vdsp_sys_pm_wksrc->active is false\n");
	return false;
}

void set_vdsp_excp_true(void)
{
	atomic_set(&vdsp_if_exception, 1);
}

void set_vdsp_excp_false(void)
{
	atomic_set(&vdsp_if_exception, 0);
}

bool is_vdsp_excp_true(void)
{
	return atomic_read(&vdsp_if_exception) != 0;
}

void set_vdsp_pw_true(void)
{
	atomic_set(&vdsp_if_poweron, 1);
}
EXPORT_SYMBOL(set_vdsp_pw_true);


void set_vdsp_pw_false(void)
{
	atomic_set(&vdsp_if_poweron, 0);
	wake_up(&vdsp_wq);
}
EXPORT_SYMBOL(set_vdsp_pw_false);

bool is_vdsp_pw_false(void)
{
	return atomic_read(&vdsp_if_poweron) != 1;
}

void print_queue_info(struct vdsp_task_priority_queue *queue)
{
	struct queue_node *node;
	int i = 0;

	if (log_level < LOG_LEVEL_DEBUG)
		return;

	mutex_lock(&queue->lock);

	xrp_print(LOG_LEVEL_DEBUG, "=== Queue %d Information ===\n", queue->priority);
	xrp_print(LOG_LEVEL_DEBUG,
		"Queue Status: %s\n", list_empty(&queue->queue) ? "Empty" : "Contains Tasks");

	list_for_each_entry(node, &queue->queue, list) {
		xrp_print(LOG_LEVEL_DEBUG, "Node [%d]:\n", i++);
		xrp_print(LOG_LEVEL_DEBUG, "  Msg ID: %u\n", node->task_info.task.msg_id);
		xrp_print(LOG_LEVEL_DEBUG,
			"  Completion Status: %s\n",
			completion_done(&node->task_info.done) ? "Completed" : "Pending");
	}

	xrp_print(LOG_LEVEL_DEBUG, "===========  END  ==========\n");

	mutex_unlock(&queue->lock);
}

void print_mbox_msg(struct mbox_msg_t *msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "len: %d\n", msg->msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %d\n", msg->msg_type);
	xrp_print(LOG_LEVEL_DEBUG, "cv_type: %u\n", msg->cv_type);
	xrp_print(LOG_LEVEL_DEBUG, "msg_id: %u\n", msg->msg_id);
	xrp_print(LOG_LEVEL_DEBUG, "priority: %u\n", msg->priority);
}

int check_vdsp_status(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;
	int retries = 0;
	int max_retrys = 2000;
	int single_time = 10;
	struct xrp_shakehand_cmd *ptr;

	ptr = (struct xrp_shakehand_cmd *)((char *)xrp_task_dev->vdsp_comm_vaddr_base + SHAKEHAND_OFFSET);
	do {
		if (is_vdsp_pw_false()) {
			xrp_print(LOG_LEVEL_ERROR, "is vdsp_pw_false true in poweroff");
			return -1;
		}
		if (!xrp_task_dev->vdsp_comm_vaddr_base) {
			xrp_print(LOG_LEVEL_ERROR, "Invalid xrp_task_dev or vdsp_comm_vaddr_base");
			return -1;
		}

		if (ptr->vdsp_status != VDSP_BEGIN_WAITI) {
			retries++;
			xrp_print(LOG_LEVEL_DEBUG, "vdsp status is %d in %d try, delay send mailbox",
							ptr->vdsp_status, retries);
			msleep(single_time);
		} else {
			xrp_print(LOG_LEVEL_DEBUG, "vdsp status is %d in %d try, send mailbox",
							ptr->vdsp_status, retries);
			return ret;
		}
	} while (retries < max_retrys);
	xrp_print(LOG_LEVEL_INFO, "query %d ms, vdsp still not in WAITI, return -1",
					single_time*max_retrys);
	return -1;
}

static int vdsp_mailbox_send(struct mbox_chan *chan, void *msg, struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;
	int retries = 0;

	if (msg == NULL)
		return -EINVAL;

	if (is_vdsp_excp_true()) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp enter is_vdsp_excp_true");
		return -EIO;
	}
	xrp_print(LOG_LEVEL_DEBUG, "enter %s, begin check vdsp status", __func__);
	ret = check_vdsp_status(xrp_task_dev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "please dont send mbox msg\n");
		return ret;
	}

	do {
		ret = mbox_send_message(chan, (void *)msg);
		if (ret < 0) {
			xrp_print(LOG_LEVEL_ERROR, "mbox_send_message failed, retrying %d/%d\n",
						retries + 1, MAX_RETRIES);
			retries++;
			msleep(RETRY_DELAY_MS);
		} else {
			break;
		}
	} while (retries < MAX_RETRIES);

	if (ret < 0)
		xrp_print(LOG_LEVEL_ERROR, "mbox_send_message failed after %d retries\n", MAX_RETRIES);

	return ret;
}

void print_xrp_task_info_l(const struct xrp_task_info_l *task_info);

void save_task_result(struct xrp_task_info_l *task_info, struct xrp_task_device *xrp_task_dev)
{
	size_t size = sizeof(struct xrp_vdsp_cmds);

	memcpy((void *)&task_info->cmds, xrp_task_dev->vdsp_comm_vaddr_base, size);
	print_xrp_task_info_l(task_info);
}

static void receive_mailbox_msg(struct xrp_task_device *xrp_task_dev,
							int msg_id,
							int queue_priority)
{
	struct queue_node *node;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;

	mutex_lock(&taskq[queue_priority].lock);
	if (&taskq[queue_priority].queue == taskq[queue_priority].cursor) {
		mutex_unlock(&taskq[queue_priority].lock);
		xrp_print(LOG_LEVEL_ERROR,
			"node msg_id %u has already been deleted.\n", msg_id);
		return;
	}

	node = list_entry(taskq[queue_priority].cursor, struct queue_node, list);
	if (node->task_info.task.msg_id == msg_id) {
		complete(&node->task_info.done);
		taskq[queue_priority].cursor = node->list.next;
		xrp_print(LOG_LEVEL_DEBUG,
			"%s: Task with msg_id %u completed.\n", __func__, msg_id);
		save_task_result(&node->task_info.task, xrp_task_dev);
	}

	mutex_unlock(&taskq[queue_priority].lock);
}


static void cancel_mbx_fail_task(struct xrp_task_device *xrp_task_dev,
							int msg_id,
							int queue_priority)
{
	struct queue_node *node;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;

	mutex_lock(&taskq[queue_priority].lock);
	if (&taskq[queue_priority].queue == taskq[queue_priority].cursor) {
		mutex_unlock(&taskq[queue_priority].lock);
		xrp_print(LOG_LEVEL_ERROR,
			"node msg_id %u has already been deleted.\n", msg_id);
		return;
	}

	node = list_entry(taskq[queue_priority].cursor, struct queue_node, list);
	if (node->task_info.task.msg_id == msg_id) {
		complete(&node->mbx_send_info.mbx_done);
		taskq[queue_priority].cursor = node->list.next;
		xrp_print(LOG_LEVEL_DEBUG,
			"%s: Task with msg_id %u cancle.\n", __func__, msg_id);
	}

	mutex_unlock(&taskq[queue_priority].lock);
}

void print_mbox_rec_vdsp_task_msg_s(struct mbox_rec_vdsp_task_msg_s msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);
	xrp_print(LOG_LEVEL_DEBUG, "cv_type: %u\n", msg.cv_type);
	xrp_print(LOG_LEVEL_DEBUG, "msg_id: %u\n", msg.msg_id);
	xrp_print(LOG_LEVEL_DEBUG, "priority: %u\n", msg.priority);
	xrp_print(LOG_LEVEL_DEBUG, "total_cycle: %u\n", msg.total_cycle);
}

void print_mbox_rec_vdsp_flushlog_msg_s(struct mbox_rec_vdsp_flushlog_msg_s msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);
	xrp_print(LOG_LEVEL_DEBUG, "event_msg len: %u\n", msg.event_msg.len);

	if (msg.event_msg.type == HE_LOG_FLUSH) {
		xrp_print(LOG_LEVEL_DEBUG, "event_msg type: HE_LOG_FLUSH\n");
		xrp_print(LOG_LEVEL_DEBUG, "readp_offset: %u\n", msg.event_msg.msg.hlfm.readp_offset);
		xrp_print(LOG_LEVEL_DEBUG, "wirtep_offset: %u\n", msg.event_msg.msg.hlfm.wirtep_offset);
	} else if (msg.event_msg.type == HE_LOG_LEVEL_SET) {
		xrp_print(LOG_LEVEL_DEBUG, "event_msg type: HE_LOG_LEVEL_SET\n");
		xrp_print(LOG_LEVEL_DEBUG, "log_level: %u\n", msg.event_msg.msg.hllm.log_level);
	}
}

void print_mbox_rec_vdsp_query_msg_s(struct mbox_rec_vdsp_query_msg_s msg)
{
	xrp_print(LOG_LEVEL_DEBUG, "msg_len: %u\n", msg.msg_len);
	xrp_print(LOG_LEVEL_DEBUG, "msg_type: %u\n", msg.msg_type);
	switch (msg.status_msg.status) {
	case DEFAULT:
		xrp_print(LOG_LEVEL_DEBUG, "Status: DEFAULT\n");
		break;
	case POWEROFF:
		xrp_print(LOG_LEVEL_DEBUG, "Status: POWEROFF\n");
		break;
	case WORK:
		xrp_print(LOG_LEVEL_DEBUG, "Status: WORK\n");
		break;
	case IDLE:
		xrp_print(LOG_LEVEL_DEBUG, "Status: IDLE\n");
		break;
	case HANG:
		xrp_print(LOG_LEVEL_DEBUG, "Status: HANG\n");
		break;
	case STATUS_MAX:
		xrp_print(LOG_LEVEL_DEBUG, "Status: STATUS_MAX\n");
		break;
	default:
		xrp_print(LOG_LEVEL_ERROR, "Error: Unknown status value\n");
		break;
	}
}

static void xmdr_mbox_flushlog_rx_callback(struct mbox_client *cl, void *msg)
{
	struct xrp_task_device *xtdev = dev_get_drvdata(cl->dev);
	struct xrp_mdr_device *xmdev = xtdev->xdev->xmdev;
	struct mbox_rec_vdsp_flushlog_msg_s *em = (struct mbox_rec_vdsp_flushlog_msg_s *)msg;
	struct xrp_notifer_block *nb = NULL, *tmp = NULL;
	int ret;

	list_for_each_entry_safe(nb, tmp, &xmdev->nh, node) {
		if (nb->event != em->event_msg.type) {
			xrp_print(LOG_LEVEL_DEBUG, "nb->event != em->event_msg.type %d\n", em->event_msg.type);
			continue;
		}
		if (!nb->cb) {
			xrp_print(LOG_LEVEL_ERROR, "nb->cb %pK cb is NULL\n", nb->cb);
			return;
		}

		print_xrp_notifier_block(nb);
		ret = nb->cb(nb->arg, (void *)&em->event_msg);

		if (ret != 0)
			xrp_print(LOG_LEVEL_ERROR, "event cb process failed: %d\n", ret);

		return;
	}

	xrp_print(LOG_LEVEL_DEBUG, "%s mbox flush log end\n", __func__);
}


static void vdsp_rpc_rx_callback(struct mbox_client *cl, void *msg)
{
	uint32_t *msg_type_tmp = (uint32_t *)msg;

	switch (*(msg_type_tmp + 1)) {
	case TASK_RUN_SIT:
	{
		interrupt_occurred = true;
		wake_up(&vdsp_wq);

		break;
	}
	case SET_LOG_LEV_SIT:
	{
		xrp_print(LOG_LEVEL_ERROR, "%s Received SET_LOG_LEV_SIT message, but fw will not send this, this is error\n", __func__);
		break;
	}
	case FLUSH_LOG_SIT:
	{
		struct mbox_rec_vdsp_flushlog_msg_s msg_local;

		memcpy(&msg_local, msg, sizeof(struct mbox_rec_vdsp_flushlog_msg_s));

		xrp_print(LOG_LEVEL_DEBUG, "%s Received FLUSH_LOG_SIT message\n", __func__);
		//print_mbox_rec_vdsp_flushlog_msg_s(msg_local);
		xmdr_mbox_flushlog_rx_callback(cl, &msg_local);
		xrp_print(LOG_LEVEL_DEBUG, "%s xmdr_mbox_flushlog_rx_callback end\n", __func__);
		break;
	}
	case QUERY_STATUS_SIT:
	{
		struct mbox_rec_vdsp_query_msg_s msg_local;

		memcpy(&msg_local, msg, sizeof(struct mbox_rec_vdsp_query_msg_s));

		xrp_print(LOG_LEVEL_DEBUG, "%s Received QUERY_STATUS_SIT message\n", __func__);
		print_mbox_rec_vdsp_query_msg_s(msg_local);
		break;
	}
	default:
		xrp_print(LOG_LEVEL_ERROR, "%s Unknown message type %d\n", __func__, *msg_type_tmp);
	}

}

static int vdsp_mbox_ch_init(struct platform_device *pdev, struct xrp_task_device *xrp_task_dev)
{
	struct vdsp_mbox *mbox;
	struct device *dev = &pdev->dev;
	struct mbox_client *cl;

	if (!of_get_property(dev->of_node, "mbox-names", NULL))
		return -EFAULT;

	mbox = devm_kzalloc(&pdev->dev, sizeof(*mbox), GFP_KERNEL);
	if (!mbox)
		return -ENOMEM;

	mbox->pdev = pdev;
	xrp_task_dev->vmbox = mbox;

	cl = &mbox->cl;
	cl->dev = dev;
	cl->tx_block = true;
	cl->tx_tout = 100;
	cl->knows_txdone = false;
	cl->rx_callback = vdsp_rpc_rx_callback;
	mbox->tx_ch = mbox_request_channel_byname(cl, "tx");
	if (IS_ERR(mbox->tx_ch)) {
		dev_err(&pdev->dev, "mailbox tx channel request failed\n");
		return PTR_ERR(mbox->tx_ch);
	}

	mbox->rx_ch = mbox_request_channel_byname(cl, "rx");
	if (IS_ERR(mbox->rx_ch)) {
		dev_err(&pdev->dev, "mailbox rx channel request failed\n");
		mbox_free_channel(mbox->tx_ch);
		return PTR_ERR(mbox->rx_ch);
	}

	return 0;
}

static void vdsp_mbox_ch_deinit(struct vdsp_mbox *mbox)
{
	mbox_free_channel(mbox->tx_ch);
	mbox_free_channel(mbox->rx_ch);
	mbox->tx_ch = NULL;
	mbox->rx_ch = NULL;
}

static int wait_for_completed_msg(struct file *file, unsigned long arg)
{
	struct queue_node *node, *tmp;
	long time_left;
	struct user_wait_data wait_data;
	int result = -EINVAL;
	uint32_t priority, wait_time_ms;
	struct xrp_task_device *xrp_task_dev = file->private_data;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;



	if (copy_from_user(&wait_data,
					(struct user_wait_data __user *)arg,
					sizeof(struct user_wait_data)) != 0)
		return -EFAULT;
	if (wait_data.priority > 3)
		return -EINVAL;

	xrp_print(LOG_LEVEL_PERF, "wait msgid %u", wait_data.msg_id);
	priority = wait_data.priority;
	wait_time_ms = wait_data.wait_time_ms;
	xrp_print(LOG_LEVEL_DEBUG,
		"%s, msg_id: %u, priority: %u, wait_time_ms: %u\n",
		__func__, wait_data.msg_id, wait_data.priority, wait_data.wait_time_ms);

	mutex_lock(&taskq[priority].lock);
	list_for_each_entry_safe(node, tmp, &taskq[priority].queue, list) {
		if (node->task_info.task.msg_id == wait_data.msg_id) {
			mutex_unlock(&taskq[priority].lock);

			if (completion_done(&node->mbx_send_info.mbx_done)) {
				mutex_lock(&taskq[priority].lock);
				list_del(&node->list);
				kmem_cache_free(xrp_task_dev->queue_node_cache, node);
				mutex_unlock(&taskq[priority].lock);
				xrp_print(LOG_LEVEL_INFO, "because msg_id %u send mbx fail, cancel task\n", wait_data.msg_id);
				return -ENOENT;
			}

			if (completion_done(&node->task_info.done)) {
				result = 0;
			} else {
				time_left = wait_for_completion_interruptible_timeout(
								&node->task_info.done,
								msecs_to_jiffies(wait_time_ms));
				if (time_left > 0)
					result = 0;
				else if (time_left == 0)
					result = -ETIMEDOUT;
				else
					result = time_left;
			}

			if (result == 0) {
				xrp_print(LOG_LEVEL_DEBUG,
					"wait task id: %u, priority: %u, ok!\n",
					wait_data.msg_id, wait_data.priority);
				mutex_lock(&taskq[priority].lock);
				if (copy_to_user((struct xrp_task_info_l __user *)arg,
					&node->task_info.task, sizeof(struct xrp_task_info_l))) {
					mutex_unlock(&taskq[priority].lock);
					xrp_print(LOG_LEVEL_ERROR,
						"Failed to copy msg_id to user\n");
					return -EFAULT;
				}
				list_del(&node->list);
				kmem_cache_free(xrp_task_dev->queue_node_cache, node);
				mutex_unlock(&taskq[priority].lock);
			}
			xrp_print(LOG_LEVEL_PERF, "wait done");
			xrp_print(LOG_LEVEL_DEBUG,
				"msg_id: %u, result is %d\n", wait_data.msg_id, result);
			return result;
		}
	}
	mutex_unlock(&taskq[priority].lock);

	xrp_print(LOG_LEVEL_ERROR,
		"wait task id: %u, priority: %u, failed!\n", wait_data.msg_id, wait_data.priority);
	return -ENOENT; // Message not found
}

static int queue_thread(void *data);

void clear_priority_queues(struct xrp_task_device *xrp_task_dev, size_t count)
{
	struct queue_node *node, *tmp;
	size_t i;
	struct vdsp_task_priority_queue *queues = xrp_task_dev->vdsp_taskq;

	if (!queues)
		return;

	for (i = 0; i < count; i++) {
		queues[i].cursor = &queues[i].queue;
		list_for_each_entry_safe(node, tmp, &queues[i].queue, list) {
			list_del(&node->list);
			kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		}
	}
}

static int xvp_dma_buf_unmap_helper(struct xrp_task_device *xrp_task_dev,
		struct xrp_mem_node *mem);

static int xvp_dma_buf_unmap_all(struct xrp_task_device *xrp_task_dev)
{
	struct xrp_mem_node *mem = NULL, *tmp = NULL;
	int ret = 0;

	mutex_lock(&xrp_task_dev->lock);
	list_for_each_entry_safe(mem, tmp, &xrp_task_dev->mem, node) {
		xrp_print(LOG_LEVEL_TRACE,
			"%s MEM UNMAP: pid is %d, fd is %d\n", __func__, mem->mem_pid, mem->info.fd);

		ret = xvp_dma_buf_unmap_helper(xrp_task_dev, mem);
		if (ret != 0)
			xrp_print(LOG_LEVEL_ERROR,
				"hrpdev dma buf unmap mem failed: %d\n", ret);

		list_del(&mem->node);
		kfree(mem);
	}
	mutex_unlock(&xrp_task_dev->lock);

	return ret;
}

void print_free_and_unmap_memory(const char *func_name, size_t size, unsigned long iova,
	void *va, dma_addr_t *dma_pa_addr, struct xrp_task_device *xrp_task_dev)
{
	xrp_print(LOG_LEVEL_DEBUG, "%s begin\n", func_name);
	xrp_print(LOG_LEVEL_DEBUG, "size: %lx\n", size);
	xrp_print(LOG_LEVEL_DEBUG, "iova: %pK\n", (void *)iova);
	xrp_print(LOG_LEVEL_DEBUG, "va: %pK\n", va);
	xrp_print(LOG_LEVEL_DEBUG, "dma_pa_addr: %pK\n", dma_pa_addr);
	xrp_print(LOG_LEVEL_DEBUG, "%s xrp_task_dev->vdsp_comm_vaddr_base: %pK\n", func_name,
		xrp_task_dev->vdsp_comm_vaddr_base);
	xrp_print(LOG_LEVEL_DEBUG, "%s xrp_task_dev->comm_size: %lx\n", func_name,
		xrp_task_dev->comm_size);
}

void free_and_unmap_comm_memory(struct xrp_task_device *xrp_task_dev,
	unsigned long iova, void *va, dma_addr_t *dma_pa_addr)
{
	dma_addr_t dma_handle = *dma_pa_addr;
	size_t size = xrp_task_dev->comm_size;

	print_free_and_unmap_memory(__func__, size, iova, va, dma_pa_addr, xrp_task_dev);

	iommu_unmap(xrp_task_dev->domain, iova, size);

	dma_free_coherent(xrp_task_dev->dev, size, va, (dma_addr_t)dma_handle);

	xrp_task_dev->vdsp_comm_vaddr_base = NULL;
	xrp_task_dev->comm_dma_handle = 0;
	xrp_task_dev->comm_size = 0;
	xrp_print(LOG_LEVEL_DEBUG, "%s free unmap comm mem end\n", __func__);
}

void free_and_unmap_special_memory(struct xrp_task_device *xrp_task_dev,
	size_t size, unsigned long iova, void *va, dma_addr_t *dma_pa_addr, bool if_free, bool if_unmap)
{
	print_free_and_unmap_memory(__func__, size, iova, va, dma_pa_addr, xrp_task_dev);

	if (if_unmap)
		iommu_unmap(xrp_task_dev->domain, iova, size);

	if (if_free)
		dma_free_coherent(xrp_task_dev->dev, size, va, (dma_addr_t)*dma_pa_addr);
	else
		mdr_unmap(va);

	xrp_print(LOG_LEVEL_DEBUG, "%s free unmap special mem end\n", __func__);
}
EXPORT_SYMBOL(free_and_unmap_special_memory);


void *allocate_and_map_comm_memory(struct xrp_task_device *xrp_task_dev,
										size_t size,
										unsigned long iova,
										dma_addr_t *dma_pa_addr,
										gfp_t gfp)
{
	dma_addr_t dma_handle;
	void *va;
	int ret;
	struct device *dev = NULL;

	if (!xrp_task_dev || !xrp_task_dev->dev || !xrp_task_dev->domain) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid xrp_task_device structure or missing device/domain\n");
		return NULL;
	}

	dev = xrp_task_dev->dev;
	size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	xrp_task_dev->comm_size = size;
	xrp_print(LOG_LEVEL_DEBUG,
		"%s Allocating DMA coherent memory of size %zu, %lx\n", __func__, size, size);

	va = dma_alloc_coherent(dev, size, (dma_addr_t *)&dma_handle, gfp);
	if (!va)
		return NULL;

	xrp_print(LOG_LEVEL_DEBUG,
		"%s DMA memory allocated, VA: %pK, PA: 0x%pK\n", __func__, va, (void *)dma_handle);

	*dma_pa_addr = dma_handle;
	xrp_print(LOG_LEVEL_DEBUG,
		"%s IOVA: 0x%lx, DMA handle: 0x%pK\n", __func__, iova, (void *)dma_handle);

#if KERNEL_VERSION_LESS_THAN_6_6_0
	ret = iommu_map(xrp_task_dev->domain, iova, dma_handle, size, IOMMU_READ | IOMMU_WRITE);
#else
	ret = iommu_map(xrp_task_dev->domain, iova, dma_handle, size, IOMMU_READ | IOMMU_WRITE, gfp);
#endif
	if (ret) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to map memory to IOMMU (IOVA: 0x%pK), error: %d\n", (void *)iova, ret);
		dma_free_coherent(dev, size, va, dma_handle);
		return NULL;
	}

	xrp_print(LOG_LEVEL_DEBUG, "%s size: %lx\n", __func__, size);
	xrp_print(LOG_LEVEL_DEBUG, "iova: %pK\n", (void *)iova);
	xrp_print(LOG_LEVEL_DEBUG, "va: %pK\n", va);
	xrp_print(LOG_LEVEL_DEBUG, "dma_pa_addr: %pK\n", dma_pa_addr);
	xrp_print(LOG_LEVEL_DEBUG, "%s xrp_task_dev->comm_size: %lx\n", __func__, xrp_task_dev->comm_size);
	xrp_print(LOG_LEVEL_DEBUG, "Memory mapped successfully\n");

	return va;
}

void *allocate_and_map_vdsp_memory(struct xrp_task_device *xrp_task_dev,
										size_t size,
										unsigned long iova,
										dma_addr_t *dma_pa_addr,
										gfp_t gfp,
										bool if_alloc,
										bool if_map)
{
	int ret;
	dma_addr_t dma_handle;
	struct device *dev = xrp_task_dev->dev;
	void *va = NULL;

	if (!xrp_task_dev->dev || !xrp_task_dev->domain) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid xrp_task_device structure or missing device/domain\n");
		return NULL;
	}

	size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	xrp_print(LOG_LEVEL_DEBUG,
		"%s Allocating DMA coherent memory of size %zu, %lx\n", __func__, size, size);

	if (if_alloc) {
		va = dma_alloc_coherent(dev, size, (dma_addr_t *)&dma_handle, gfp);
		if (!va)
			return NULL;

		*dma_pa_addr = dma_handle;
	} else {
		*dma_pa_addr = DFX_MEM_VDSP_ADDR;
		size = DFX_MEM_VDSP_SIZE;
		va = mdr_map(*dma_pa_addr, size);
	}

	if (if_map) {
#if KERNEL_VERSION_LESS_THAN_6_6_0
		ret = iommu_map(xrp_task_dev->domain, iova, *dma_pa_addr, size, IOMMU_READ | IOMMU_WRITE);
#else
		ret = iommu_map(xrp_task_dev->domain, iova, *dma_pa_addr, size, IOMMU_READ | IOMMU_WRITE, gfp);
#endif
		if (ret) {
			xrp_print(LOG_LEVEL_ERROR, "Failed to map memory to IOMMU (IOVA: 0x%pK), error: %d\n", (void *)iova, ret);
			if (if_alloc)
				dma_free_coherent(dev, size, va, *dma_pa_addr);
			else
				mdr_unmap(va);
			return NULL;
		}
	}
	xrp_print(LOG_LEVEL_DEBUG, "%s size: %lx\n", __func__, size);
	xrp_print(LOG_LEVEL_DEBUG, "iova: %pK\n", (void *)iova);
	xrp_print(LOG_LEVEL_DEBUG, "dma_pa_addr: %pK\n", dma_pa_addr);
	xrp_print(LOG_LEVEL_DEBUG, "%s xrp_task_dev->comm_size: %lx\n", __func__, xrp_task_dev->comm_size);
	xrp_print(LOG_LEVEL_DEBUG, "Memory mapped successfully\n");

	return va;
}
EXPORT_SYMBOL(allocate_and_map_vdsp_memory);




int xrp_poweron_map_on_process(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;

	xrp_print(LOG_LEVEL_DEBUG, "%s power on map begin\n", __func__);
	if (!xrp_task_dev || !xrp_task_dev->hw_ops) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid function pointers detected\n");
		return -EINVAL;
	}

	if (!xrp_task_dev->hw_ops->enable) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer enable are not properly initialized\n");
		return -EINVAL;
	}
	ret = xrp_task_dev->hw_ops->enable(xrp_task_dev->hw_arg);
	if (ret) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp failed to enable, ret is %d\n", ret);
		return ret;
	}

	if (!xrp_task_dev->hw_ops->halt) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer halt are not properly initialized\n");
		return -EINVAL;
	}
	xrp_task_dev->hw_ops->halt(xrp_task_dev->hw_arg);

	if (!xrp_task_dev->hw_ops->disreset) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer disreset are not properly initialized\n");
		return -EINVAL;
	}
	xrp_task_dev->hw_ops->disreset(xrp_task_dev->hw_arg);

	if (!xrp_task_dev->hw_ops->load_fw_segment) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer load_fw_segment are not properly initialized\n");
		return -EINVAL;
	}
	ret = xrp_task_dev->hw_ops->load_fw_segment(xrp_task_dev->hw_arg, NULL, NULL);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp failed to map and load fw images, ret is %d\n", ret);
		return ret;
	}

	if (!xrp_task_dev->hw_ops->release) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer release are not properly initialized\n");
		return -EINVAL;
	}
	xrp_task_dev->hw_ops->release(xrp_task_dev->hw_arg);

	xrp_print(LOG_LEVEL_DEBUG, "%s power on map end\n", __func__);
	return ret;
}


static int xrp_alloc_and_map_mdr_mem(struct xrp_task_device *xtdev, bool if_alloc, bool if_map)
{
	int ret = 0;
	struct xrp_device *xdev = NULL;
	struct xrp_mdr_device *xmdev = NULL;
	struct xrp_mdr_mem_info *mem = NULL;
	struct xrp_log_info *log_info = NULL;

	xdev = xtdev->xdev;
	xmdev = xtdev->xdev->xmdev;
	mem = &xdev->xmm;
	ret = alloc_and_map_special_mdr_mem(xtdev, mem, if_alloc, if_map);
	if (ret) {
		xrp_print(LOG_LEVEL_ERROR, "xrp_malloc_and_map_mdr Failed\n");
		return ret;
	}

	xrp_print(LOG_LEVEL_DEBUG, "@xmdr mem memory va %pK, pa 0x%pK\n", mem->va, (void *)mem->pa);
	xmdev->mdr_retinfo.log_addr = mem->pa;
	xmdev->mdr_retinfo.log_len = mem->size;
	log_info = (struct xrp_log_info *)mem->va;
	log_info->writep = 0;
	log_info->readp = 0;

	xrp_print(LOG_LEVEL_DEBUG, "mem->pa:0x%pK, mem->va %pK, mem->iova 0x%pK\n, mem->size:0x%x",
		(void *)xdev->xmm.pa, (void *)mem->va, (void *)mem->iova, mem->size);

	return ret;
}

int vdsp_exception_resource_release(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;

	if (mutex_lock_interruptible(&xrp_task_dev->open_mutex))
		return -EINTR;

	//xrp_task_dev->is_first_open = true;
	clear_priority_queues(xrp_task_dev, xrp_task_dev->num_queues);

	ret = xvp_dma_buf_unmap_all(xrp_task_dev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xvp_dma_buf_unmap_all fail, ret is %d\n", ret);
		mutex_unlock(&xrp_task_dev->open_mutex);
		return ret;
	}

	sys_nocache_unmap(xrp_task_dev, &common_data);
	xrp_task_dev->vdsp_comm_vaddr_base = NULL;
	xrp_task_dev->comm_size = 0;
	free_and_unmap_special_mdr_mem(xrp_task_dev, false, true);
	xrp_print(LOG_LEVEL_INFO, "free_and_unmap_mdr mem\n");

	if (xrp_task_dev->if_use_mbox && xrp_task_dev->vmbox != NULL) {
		xrp_print(LOG_LEVEL_INFO, "if_use_mbox = %d, vmbox =%pK\n",
			xrp_task_dev->if_use_mbox, xrp_task_dev->vmbox);
		vdsp_mbox_ch_deinit(xrp_task_dev->vmbox);
	}

	ret = xrp_poweroff_unmap_on_process(xrp_task_dev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xrp_poweroff_unmap_on_process fail, ret is %d\n", ret);
		mutex_unlock(&xrp_task_dev->open_mutex);
		return ret;
	}

	if (is_wksrc_active()) {
		__pm_relax(g_vdsp_sys_pm_wksrc);
		xrp_print(LOG_LEVEL_DEBUG, "__pm_relax.g_vdsp_sys_pm_wksrc end\n");
	}

	// vdsp_poweron_done = false;
	// xrp_task_dev->open_count--;
	xrp_task_dev->is_first_open = true;
	xrp_print(LOG_LEVEL_DEBUG, "queue_release xrp_task_dev->open_count is %d  is_first_open %d\n", xrp_task_dev->open_count, xrp_task_dev->is_first_open);

	mutex_unlock(&xrp_task_dev->open_mutex);

	return ret;
}

void unmap_dfx_mem(struct xrp_task_device *xrp_task_dev)
{
	unsigned long iova = VDSP_MDR_ADDR;
	size_t size = (size_t)DFX_MEM_VDSP_SIZE;

	if (iommu_unmap(xrp_task_dev->domain, iova, size) == 0)
		xrp_print(LOG_LEVEL_ERROR, "Error: iommu_unmap failed\n");

	mdr_unmap(xrp_task_dev->xmdr_log_info);
}

static void vdsp_dfx_disable(struct xrp_task_device *xrp_task_dev)
{
	int ret;
	unsigned long iova = VDSP_MDR_ADDR;
	dma_addr_t dma_pa_addr = (dma_addr_t)DFX_MEM_VDSP_ADDR;
	size_t size = (size_t)DFX_MEM_VDSP_SIZE;
	struct xrp_log_info *log_info;
	void *log_infoll = NULL;

	log_infoll = mdr_map(dma_pa_addr, size);
	if (!log_infoll) {
		xrp_print(LOG_LEVEL_ERROR, "Error: mdr_map failed\n");
		return;
	}

	xrp_task_dev->xmdr_log_info = log_infoll;

#if KERNEL_VERSION_LESS_THAN_6_6_0
	ret = iommu_map(xrp_task_dev->domain, iova, dma_pa_addr, size, IOMMU_READ | IOMMU_WRITE);
#else
	ret = iommu_map(xrp_task_dev->domain, iova, dma_pa_addr, size, IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#endif
	if (ret) {
		xrp_print(LOG_LEVEL_ERROR, "Error: iommu_map failed with error code %d\n", ret);
		mdr_unmap(log_infoll);
		return;
	}
	log_info = (struct xrp_log_info *)log_infoll;

	log_info->hmdr_enable = HMDR_NV_DISENABLE;
	log_info->log_drop = HMDR_NORM_LOG_DROP;
}

static int queue_open(struct inode *inode, struct file *file)
{
	struct xrp_task_device *xrp_task_dev;
	struct platform_device *pdev;
	struct xrp_log_info *log_info;
	struct sys_nocache_dma_buf *xrp_sys_dma_buf = NULL;
	int ret = 0;

	xrp_print(LOG_LEVEL_DEBUG, "enter %s, line =%d", __func__, __LINE__);
	xrp_task_dev = container_of(file->private_data,
							struct xrp_task_device,
							miscdev);

	file->private_data = xrp_task_dev;
	if (mutex_lock_interruptible(&xrp_task_dev->open_mutex))
		return -EINTR;
	xrp_print(LOG_LEVEL_DEBUG, "qopen xrp_task_dev->open_count is %d  is_first_open %d\n",
		xrp_task_dev->open_count, xrp_task_dev->is_first_open);
	if (xrp_task_dev->is_first_open) {
		xrp_print(LOG_LEVEL_DEBUG, "begin malloc and map comm memory, addr is %pK, sizeof(struct xrp_vdsp_cmds) is %ld\n",
			(void *)VDSP_COMM, sizeof(struct xrp_vdsp_cmds));
		xrp_sys_dma_buf = sys_nocache_map(xrp_task_dev, &common_data);
		xrp_task_dev->vdsp_comm_vaddr_base = xrp_sys_dma_buf->buf;
		xrp_task_dev->comm_size = xrp_sys_dma_buf->dma_buf->size;
		xrp_print(LOG_LEVEL_DEBUG, "xrp_task_dev->comm_size = %zu\n", xrp_task_dev->comm_size);
		if (!xrp_task_dev->vdsp_comm_vaddr_base) {
			xrp_print(LOG_LEVEL_ERROR, "Failed to allocate and map communication memory\n");
			mutex_unlock(&xrp_task_dev->open_mutex);
			return -EINTR;
		}
		xrp_print(LOG_LEVEL_DEBUG, "sizeof struct xrp_vdsp_cmds is %ld\n", sizeof(struct xrp_vdsp_cmds));

		if (is_switch_on(SWITCH_HMDR)) {
			xrp_alloc_and_map_mdr_mem(xrp_task_dev, false, true);

			log_info = (struct xrp_log_info *)xrp_task_dev->xdev->xmm.va;
			if (vdsp_log_level != 20) {
				xrp_task_dev->xdev->xmdev->log_level = vdsp_log_level;
				xrp_print(LOG_LEVEL_DEBUG, "dynamic modify log_level, value is %d\n", xrp_task_dev->xdev->xmdev->log_level);
			}

			log_info->hmdr_enable = HMDR_NV_ENABLE;
			log_info->log_level = xrp_task_dev->xdev->xmdev->log_level;
			log_info->log_drop = HMDR_NORM_LOG_DROP;
			xrp_print(LOG_LEVEL_DEBUG, "log_info->log_level %08x\n", log_info->log_level);

		} else
			vdsp_dfx_disable(xrp_task_dev);

		pdev = to_platform_device(xrp_task_dev->dev);
		xrp_task_dev->if_use_mbox = true;
		if (xrp_task_dev->if_use_mbox) {
			ret = vdsp_mbox_ch_init(pdev, xrp_task_dev);
			if (ret < 0) {
				xrp_print(LOG_LEVEL_ERROR, "vdsp_mbox_ch_init fail\n");
				mutex_unlock(&xrp_task_dev->open_mutex);
				return ret;
			}
		}

		if (!is_wksrc_active()) {
			__pm_stay_awake(g_vdsp_sys_pm_wksrc);
			xrp_print(LOG_LEVEL_DEBUG, "__pm_stay_awake.g_vdsp_sys_pm_wksrc end\n");
		}

		ret = xrp_poweron_map_on_process(xrp_task_dev);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "xrp_poweron_map_on_process fail, ret is %d\n", ret);
			mutex_unlock(&xrp_task_dev->open_mutex);
			__pm_relax(g_vdsp_sys_pm_wksrc);
			return ret;
		}

		xrp_task_dev->is_first_open = false;
		xrp_print(LOG_LEVEL_DEBUG, "queue_open_first——open xrp_task_dev->open_count is %d  is_first_open %d\n",
			xrp_task_dev->open_count, xrp_task_dev->is_first_open);

		set_vdsp_excp_false();
	}
	xrp_task_dev->open_count++;
	xrp_print(LOG_LEVEL_DEBUG, "queue_open_end xrp_task_dev->open_count is %d  is_first_open %d\n",
		xrp_task_dev->open_count, xrp_task_dev->is_first_open);

	xrp_print(LOG_LEVEL_DEBUG,
		"%s, device open count is %d\n", __func__, xrp_task_dev->open_count);
	mutex_unlock(&xrp_task_dev->open_mutex);

	return 0;
}

int xrp_poweroff_unmap_on_process(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;

	xrp_print(LOG_LEVEL_DEBUG, "%s power off unmap begin\n", __func__);

	if (!xrp_task_dev || !xrp_task_dev->hw_ops) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid function pointers detected\n");
		return -EINVAL;
	}

	if (!xrp_task_dev->hw_ops->free_fw_segment) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer free_fw_segment are not properly initialized\n");
		return -EINVAL;
	}
	ret = xrp_task_dev->hw_ops->free_fw_segment(xrp_task_dev->hw_arg, NULL, NULL);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "vdsp failed to unmap and remove fw images, ret is %d\n", ret);
		return ret;
	}

	if (!xrp_task_dev->hw_ops->disable) {
		xrp_print(LOG_LEVEL_ERROR, "Function pointer disable are not properly initialized\n");
		return -EINVAL;
	}
	xrp_task_dev->hw_ops->disable(xrp_task_dev->hw_arg);

	xrp_print(LOG_LEVEL_DEBUG, "%s power off unmap end\n", __func__);
	return ret;
}

static int xrp_flush_log_in_poweroff(struct xrp_task_device *xrp_task_dev)
{
	struct xrp_mdr_device *xmdev;
	struct xrp_log_info *info;

	xrp_print(LOG_LEVEL_DEBUG, "flush log in power off entry %s\n", __func__);
	xmdev = (struct xrp_mdr_device *)xrp_task_dev->xdev->xmdev;
	info = xmdev->xdev->xmm.va;

	kill_fasync(&xmdev->fasync_q, SIGUSR2, POLL_IN);
	xrp_print(LOG_LEVEL_DEBUG, "send exception fasync_q SIGUSR2");

	xrp_print(LOG_LEVEL_DEBUG, "log_level: %u\n", info->log_level);
	xrp_print(LOG_LEVEL_DEBUG, "writep: %u\n", info->writep);
	xrp_print(LOG_LEVEL_DEBUG, "readp: %u\n", info->readp);

	return 0;
}


static int queue_release(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct xrp_task_device *xrp_task_dev = file->private_data;

	if (mutex_lock_interruptible(&xrp_task_dev->open_mutex))
		return -EINTR;

	xrp_task_dev->open_count--;
	xrp_print(LOG_LEVEL_DEBUG, "%s, device open count is %d\n", __func__, xrp_task_dev->open_count);

	xrp_print(LOG_LEVEL_DEBUG, "qrelease xrp_task_dev->open_count is %d  is_first_open %d\n", xrp_task_dev->open_count, xrp_task_dev->is_first_open);

	if (xrp_task_dev->open_count == 0) {
		xrp_task_dev->is_first_open = true;

		if (is_vdsp_excp_true()) {
			xrp_print(LOG_LEVEL_DEBUG, "enter %s, vdsp_excp_true happen %d\n", __func__, xrp_task_dev->open_count);
			mutex_unlock(&xrp_task_dev->open_mutex);
			return 0;
		}

		clear_priority_queues(xrp_task_dev, xrp_task_dev->num_queues);
		xrp_task_dev->hw_ops->reset(xrp_task_dev->hw_arg);
		xvp_dma_buf_unmap_all(xrp_task_dev);
		sys_nocache_unmap(xrp_task_dev, &common_data);
		xrp_task_dev->vdsp_comm_vaddr_base = NULL;
		xrp_task_dev->comm_size = 0;

		if (is_switch_on(SWITCH_HMDR)) {
			xrp_flush_log_in_poweroff(xrp_task_dev);

			free_and_unmap_special_mdr_mem(xrp_task_dev, false, true);
			xrp_print(LOG_LEVEL_DEBUG, "free_and_unmap_mdr mem end\n");
		} else
			unmap_dfx_mem(xrp_task_dev);

		if (xrp_task_dev->if_use_mbox && xrp_task_dev->vmbox != NULL) {
			xrp_print(LOG_LEVEL_DEBUG, "if_use_mbox = %d, vmbox =%pK\n",
				xrp_task_dev->if_use_mbox, xrp_task_dev->vmbox);
			vdsp_mbox_ch_deinit(xrp_task_dev->vmbox);
		}

		ret = xrp_poweroff_unmap_on_process(xrp_task_dev);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR, "xrp_poweroff_unmap_on_process fail, ret is %d\n", ret);
			mutex_unlock(&xrp_task_dev->open_mutex);
			return ret;
		}

		if (is_wksrc_active()) {
			__pm_relax(g_vdsp_sys_pm_wksrc);
			xrp_print(LOG_LEVEL_DEBUG, "__pm_relax.g_vdsp_sys_pm_wksrc end\n");
		}

	}
	mutex_unlock(&xrp_task_dev->open_mutex);

	return 0;
}

void print_all_addrs_on_same_line(struct xrp_vdsp_cmds *cmds)
{
	struct xrp_vdsp_cmd *cmd;
	size_t remaining = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	int written;

	char line[1024] = {0};
	size_t line_length = 0;

	if (log_level < LOG_LEVEL_DEBUG)
		return;

	if (!cmds) {
		xrp_print(LOG_LEVEL_DEBUG, "Commands structure pointer is null\n");
		return;
	}

	for (i = 0; i < cmds->cmd_num; ++i) {
		cmd = &cmds->dsp_cmds[i];
		for (j = 0; j < cmd->buffer_num; ++j) {
			remaining = sizeof(line) - line_length;
			written = snprintf(line + line_length,
							remaining,
							"0x%08x ",
							cmd->buffer[j].addr);
			if (written > 0 && written < remaining) {
				line_length += written;
			} else {
				xrp_print(LOG_LEVEL_DEBUG,
					"Error: Not enough space in the line buffer or snpr_info error\n");
				return;
			}
		}
	}

	if (line_length > 0) {
		line[line_length - 1] = '\0';
		xrp_print(LOG_LEVEL_DEBUG, "VADDR:%s\n", line);
	}
}


void print_xrp_vdsp_buffer(const struct xrp_vdsp_buffer *buffer)
{
	xrp_print(LOG_LEVEL_DEBUG, "Buffer:\n");
	xrp_print(LOG_LEVEL_DEBUG, "  Flag: %u\n", buffer->flag);
	xrp_print(LOG_LEVEL_DEBUG, "  Size: %u\n", buffer->size);
	xrp_print(LOG_LEVEL_DEBUG, "  Addr: %u\n", buffer->addr);
}

void print_xrp_vdsp_cmd(const struct xrp_vdsp_cmd *cmd)
{
	int i;

	if (log_level < LOG_LEVEL_DEBUG)
		return;

	xrp_print(LOG_LEVEL_DEBUG, "VDSP Command:\n");
	xrp_print(LOG_LEVEL_DEBUG, "  In Data Size: %u\n", cmd->in_data_size);
	xrp_print(LOG_LEVEL_DEBUG, "  Out Data Size: %u\n", cmd->out_data_size);
	xrp_print(LOG_LEVEL_DEBUG, "  Buffer Num: %u\n", cmd->buffer_num);
	xrp_print(LOG_LEVEL_DEBUG, "  Status: %u\n", cmd->status);

	xrp_print(LOG_LEVEL_DEBUG, "  Buffers:\n");
	for (i = 0; i < cmd->buffer_num; i++)
		print_xrp_vdsp_buffer(&cmd->buffer[i]);
}

void print_xrp_vdsp_cmds(const struct xrp_vdsp_cmds *cmds)
{
	uint32_t i;

	xrp_print(LOG_LEVEL_DEBUG,
		"VDSP Commands: Cmd Type: %u, Cmd Num: %u\n", cmds->cmd_type, cmds->cmd_num);
	for (i = 0; i < cmds->cmd_num; ++i)
		print_xrp_vdsp_cmd(&cmds->dsp_cmds[i]);
}

void print_xrp_task_info_l(const struct xrp_task_info_l *task_info)
{
	if (log_level < LOG_LEVEL_DEBUG)
		return;
	mutex_lock(&print_func_lock);
	xrp_print(LOG_LEVEL_DEBUG,
		"Task Info: MsgID: %u, Priority: %u\n", task_info->msg_id, task_info->priority);
	print_xrp_vdsp_cmds(&task_info->cmds);
	mutex_unlock(&print_func_lock);
}

bool is_valid_xrp_task_info_l(struct xrp_task_info_l *task)
{
	struct xrp_vdsp_cmd *cmd;

	if (task->priority > 3 || task->cmds.cmd_num > VDSP_CMD_MAX_NUM)
		return false;
	for (uint32_t i = 0; i < task->cmds.cmd_num; i++) {
		cmd = &task->cmds.dsp_cmds[i];
		if (cmd->in_data_size > XRP_DSP_CMD_INLINE_INDATA_SIZE ||
			cmd->out_data_size > XRP_DSP_CMD_INLINE_OUTDATA_SIZE ||
			cmd->buffer_num > XRP_DSP_CMD_INLINE_BUFFER_SIZE) {
			return false;
		}
	}
	return true;
}

static int enqueue_task(struct file *file, unsigned long arg)
{
	int ret = 0;
	uint32_t priority, msg_id;
	struct queue_node *node;
	struct xrp_task_device *xrp_task_dev = file->private_data;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;

	xrp_print(LOG_LEVEL_PERF, "enqueue");

	node = kmem_cache_alloc(xrp_task_dev->queue_node_cache, GFP_KERNEL);
	if (!node) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to allocate memory for queue node\n");
		return -ENOMEM;
	}
	xrp_print(LOG_LEVEL_PERF, "alloc node done");
	if (copy_from_user(&node->task_info.task,
					(struct xrp_task_info_l __user *)arg,
					sizeof(struct xrp_task_info_l))) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to copy xrp_task_info from user\n");
		kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		return -EFAULT;
	}
	if (!is_valid_xrp_task_info_l(&node->task_info.task)) {
		kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		return -EINVAL;
	}

	xrp_print(LOG_LEVEL_PERF, "cp_f_u done");
	priority = node->task_info.task.priority;

	if (priority >= xrp_task_dev->num_queues) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid priority\n");
		kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		return -EINVAL;
	}

	msg_id = atomic_inc_return(&msg_id_counter); // Generate unique msg_id
	node->task_info.task.msg_id = msg_id;
	init_completion(&node->task_info.done);
	init_completion(&node->mbx_send_info.mbx_done);

	mutex_lock(&taskq[priority].lock);
	(void)task_ready_action(true);
	list_add_tail(&node->list, &taskq[priority].queue);
	if (&taskq[priority].queue == taskq[priority].cursor)
		taskq[priority].cursor = &node->list;

	mutex_unlock(&taskq[priority].lock);
	xrp_print(LOG_LEVEL_PERF, "enqueue done: %u", msg_id);

	if (copy_to_user(&((struct xrp_task_info_l __user *)arg)->msg_id,
		&msg_id, sizeof(uint32_t))) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to copy msg_id to user\n");
		kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		return -EFAULT;
	}

	return ret;
}

static int dequeue_task(struct file *file, unsigned long arg)
{
	int ret = -ENOENT;
	uint32_t priority, status = 0;
	struct queue_node *node, *tmp;
	struct xrp_task_cancel task_cancel;
	struct xrp_task_device *xrp_task_dev = file->private_data;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;

	if (copy_from_user(&task_cancel, (struct xrp_task_cancel __user *)arg,
		sizeof(struct xrp_task_cancel)) != 0) {
		xrp_print(LOG_LEVEL_ERROR, "copy_from_user error\n");
		return -EFAULT;
	}
	xrp_print(LOG_LEVEL_DEBUG, "enter dequeue task\n");
	priority = task_cancel.priority;

	if (priority >= xrp_task_dev->num_queues) {
		xrp_print(LOG_LEVEL_ERROR, "Invalid priority\n");
		return -EINVAL;
	}

	mutex_lock(&taskq[priority].lock);

	if (list_empty(&taskq[priority].queue)) {
		mutex_unlock(&taskq[priority].lock);
		return -ENOENT;
	}

	list_for_each_entry_safe(node, tmp, &taskq[priority].queue, list) {
		if (node->task_info.task.msg_id == task_cancel.msg_id) {
			if (taskq[priority].cursor == &node->list) {
				mutex_unlock(&taskq[priority].lock);
				xrp_print(LOG_LEVEL_DEBUG,
					"%s: Task with msg_id %u is being processed, ret is %d\n",
					__func__, task_cancel.msg_id, -EBUSY);
				status = 1;
				if (copy_to_user(&((struct xrp_task_cancel __user *)arg)->status,
								&status, sizeof(uint32_t))) {
					xrp_print(LOG_LEVEL_ERROR,
						"Failed to copy status to user\n");
					return -EFAULT;
				}
				return -EBUSY;
			}
			list_del(&node->list);
			kmem_cache_free(xrp_task_dev->queue_node_cache, node);
			ret = 0;
			break;
		}
	}

	mutex_unlock(&taskq[priority].lock);

	if (ret == 0) {
		xrp_print(LOG_LEVEL_DEBUG,
			"%s: Task with msg_id %u removed from priority %u queue\n",
			__func__, task_cancel.msg_id, priority);
	} else {
		xrp_print(LOG_LEVEL_ERROR,
			"%s: Task with msg_id %u not found in priority %u queue\n",
			__func__, task_cancel.msg_id, priority);
		ret = -ENOENT;
	}

	if (copy_to_user(&((struct xrp_task_cancel  *)arg)->status, &status, sizeof(uint32_t))) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to copy status to user\n");
		return -EFAULT;
	}

	return ret;
}


void print_vdsp_memmap_info(const struct vdsp_memmap_info *info)
{
	if (log_level < LOG_LEVEL_DEBUG)
		return;

	if (info == NULL) {
		xrp_print(LOG_LEVEL_DEBUG, "%s: Passed NULL pointer\n", __func__);
		return;
	}

	xrp_print(LOG_LEVEL_DEBUG, "vdsp_memmap_info values:\n");
	xrp_print(LOG_LEVEL_DEBUG, "fd: %d\n", info->fd);
	xrp_print(LOG_LEVEL_DEBUG, "size: %d\n", info->size);
	xrp_print(LOG_LEVEL_DEBUG, "type: %d\n", info->type);
	xrp_print(LOG_LEVEL_DEBUG, "flag: %s\n", info->flag ? "true" : "false");
	xrp_print(LOG_LEVEL_DEBUG, "prot: %d\n", info->prot);
}

void print_xrp_mem_list(struct list_head *head)
{
	struct xrp_mem_node *current_node;

	if (log_level < LOG_LEVEL_DEBUG)
		return;

	mutex_lock(&print_func_lock);
	xrp_print(LOG_LEVEL_DEBUG, "--------%s--------\n", __func__);

	list_for_each_entry(current_node, head, node) {
		xrp_print(LOG_LEVEL_DEBUG, "PID: %d\n", current_node->mem_pid);
		xrp_print(LOG_LEVEL_DEBUG, "Buffer Pointer: %pK\n", (void *)current_node->buf);

		print_vdsp_memmap_info(&current_node->info);

		xrp_print(LOG_LEVEL_DEBUG, "----------------\n");
	}
	mutex_unlock(&print_func_lock);
}

int xvp_mem_unmap_iova(struct xrp_task_device *xrp_task_dev, struct xrp_dma_buf *buf,
		unsigned long iova)
{
	int ret = 0;
	size_t size = 0;

	if (buf->dmabuf->size != ALIGN(buf->size, PAGE_SIZE)) {
		xrp_print(LOG_LEVEL_ERROR,
			"dma buf size 0x%lx not match size 0x%lx", buf->dmabuf->size, buf->size);
		return -EINVAL;
	}

	size = iommu_unmap(xrp_task_dev->domain, iova, buf->dmabuf->size);
	if (size != buf->dmabuf->size) {
		xrp_print(LOG_LEVEL_ERROR,
			"iommu unmap failed, map size 0x%lx, sgt size 0x%lx\n", size, buf->size);
		ret = -ENOMEM;
		return ret;
	}

	if (iova != VDSP_SRAM && iova != VDSP_SROM)
		gen_pool_free(xrp_task_dev->pool, iova, buf->size);

	return ret;
}

int xvp_dma_buf_clean(struct device *dev, struct xrp_dma_buf *buf)
{
	if (!buf->attach || !buf->sgt) {
		xrp_print(LOG_LEVEL_ERROR, "no attachmemt or no buf, fd %d\n", buf->fd);
		return -EINVAL;
	}

	dma_buf_vunmap(buf->dmabuf, &buf->map);

	dma_buf_unmap_attachment(buf->attach, buf->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf->dmabuf, buf->attach);
	dma_buf_put(buf->dmabuf);
	kfree(buf);

	return 0;
}

static int xvp_dma_buf_unmap_helper(struct xrp_task_device *xrp_task_dev,
		struct xrp_mem_node *mem)
{
	struct device *dev = xrp_task_dev->dev;
	int ret;

	xrp_print(LOG_LEVEL_TRACE, "vdsp memory unmap start\n");
	print_vdsp_memmap_info(&mem->info);

	ret = xvp_dma_buf_clean(dev, mem->buf);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "hdev dma buf clean failed\n");
		return ret;
	}

	xrp_print(LOG_LEVEL_TRACE, "vdsp memory unmap end\n");

	return 0;
}

static int xvp_dma_buf_unmap(struct file *filp, void __user *argp)
{
	struct vdsp_memmap_info info;
	struct xrp_mem_node *mem = NULL, *tmp = NULL;
	int ret;
	struct xrp_task_device *xrp_task_dev = filp->private_data;
	pid_t pid = current->pid;

	ret = copy_from_user(&info, argp, sizeof(info));
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR,
			"copy from user for mem info failed: %d\n", ret);
		return ret;
	}

	mutex_lock(&xrp_task_dev->lock);
	list_for_each_entry_safe(mem, tmp, &xrp_task_dev->mem, node) {
		if ((mem->mem_pid != pid) ||
			(mem->info.fd != info.fd) ||
			(mem->info.size != info.size))
			continue;

		xrp_print(LOG_LEVEL_DEBUG,
			"%s MEM UNMAP: pid is %d, fd is %d\n", __func__, pid, mem->info.fd);

		ret = xvp_dma_buf_unmap_helper(xrp_task_dev, mem);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR,
				"hrpdev dma buf unmap mem failed: %d\n", ret);
			return ret;
		}

		list_del(&mem->node);
		kfree(mem);
		break;
	}
	print_xrp_mem_list(&xrp_task_dev->mem);
	mutex_unlock(&xrp_task_dev->lock);

	return ret;
}

struct xrp_dma_buf *xrp_dma_buf_fill(struct device *dev,
		int fd, size_t size)
{
	struct xrp_dma_buf *buf;
	int ret = 0;

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf) {
		xrp_print(LOG_LEVEL_ERROR, "zalloc hpc dma buf failed, fd %d\n", fd);
		return NULL;
	}

	buf->fd = fd;
	buf->size = size;
	buf->dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf->dmabuf)) {
		xrp_print(LOG_LEVEL_ERROR, "dma buf get failed, fd %d\n", fd);
		goto err_put_zalloc;
	}

	if (buf->dmabuf->size != ALIGN(size, PAGE_SIZE)) {
		xrp_print(LOG_LEVEL_ERROR,
			"dma buf size 0x%lx not match size 0x%lx", buf->dmabuf->size, size);
		goto err_put_dma_buf;
	}

	buf->attach = dma_buf_attach(buf->dmabuf, dev);
	if (IS_ERR_OR_NULL(buf->attach)) {
		xrp_print(LOG_LEVEL_ERROR, "dma buf attach failed, fd %d\n", fd);
		goto err_put_dma_buf;
	}

	buf->sgt = dma_buf_map_attachment(buf->attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(buf->sgt)) {
		xrp_print(LOG_LEVEL_ERROR, "dma buf map attachment failed, fd %d\n", fd);
		goto err_put_dma_attach;
	}

	/* map to kernel and access the mem */
	ret = dma_buf_vmap(buf->dmabuf, &buf->map);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "heap_test: dma_buf_vmap fail\n");
		goto err_dma_buf_vmap;
	}

	buf->vaddr = (void *)buf->map.vaddr;
	buf->len = 0;

	return buf;

err_dma_buf_vmap:
	dma_buf_unmap_attachment(buf->attach, buf->sgt, DMA_BIDIRECTIONAL);
err_put_dma_attach:
	dma_buf_detach(buf->dmabuf, buf->attach);
err_put_dma_buf:
	dma_buf_put(buf->dmabuf);
err_put_zalloc:
	kfree(buf);

	return NULL;
}

int xvp_mem_map_iova(struct xrp_task_device *xrp_task_dev, struct xrp_dma_buf *buf,
		unsigned long *iova, int prot)
{
	size_t size = 0;
	int ret = 0;
	bool if_genpol_alloc = false;

	if (buf->dmabuf->size != ALIGN(buf->size, PAGE_SIZE)) {
		xrp_print(LOG_LEVEL_ERROR,
			"dma buf size 0x%lx not match size 0x%lx", buf->dmabuf->size, buf->size);
		return -EINVAL;
	}

	if (*iova == 0) {
		*iova = gen_pool_alloc(xrp_task_dev->pool, buf->size);
		if (*iova == 0) {
			xrp_print(LOG_LEVEL_ERROR, "gen pool alloc iova failed\n");
			ret = -ENOMEM;
			return ret;
		}
		if_genpol_alloc = true;
	}

	size = iommu_map_sgtable(xrp_task_dev->domain, *iova, buf->sgt, prot);
	if (size != buf->dmabuf->size) {
		xrp_print(LOG_LEVEL_ERROR,
			"map sgtable failed, map size 0x%lx, sgt size 0x%lx\n", size, buf->size);
		if (if_genpol_alloc)
			gen_pool_free(xrp_task_dev->pool, *iova, buf->size);
		return -ENOMEM;
	}

	return ret;
}

static int xvp_dma_buf_map_helper(struct xrp_task_device *xrp_task_dev,
		struct xrp_mem_node *mem)
{
	struct device *dev = xrp_task_dev->dev;

	print_vdsp_memmap_info(&mem->info);

	mem->buf = xrp_dma_buf_fill(dev, mem->info.fd, mem->info.size);
	if (!mem->buf) {
		xrp_print(LOG_LEVEL_ERROR, "hdev dma buf fill failed\n");
		return -ENOMEM;
	}

	if (mem->info.type == 0)
		xrp_task_dev->vdsp_sysram_mem_buf = (void *)mem->buf;

	if (mem->info.type == 1)
		xrp_task_dev->vdsp_sysrom_mem_buf = (void *)mem->buf;

	mem->info.da = (unsigned int)mem->buf->sgt->sgl->dma_address;

	return 0;
}

static int xvp_dma_buf_map(struct file *filp, void __user *argp)
{
	struct xrp_mem_node *mem = NULL;
	int ret;
	struct xrp_task_device *xrp_task_dev = filp->private_data;
	pid_t pid = current->pid;

	mem = kzalloc(sizeof(*mem), GFP_KERNEL);
	if (!mem) {
		xrp_print(LOG_LEVEL_ERROR, "malloc rpmsg mem node failed\n");
		return -ENOMEM;
	}

	mem->mem_pid = pid;
	ret = copy_from_user(&mem->info, argp, sizeof(mem->info));
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR,
			"copy from user for mem info failed or heap not exist: %d\n", ret);
		goto put_dma_buf_fill;
	}

	xrp_print(LOG_LEVEL_DEBUG, "MEMMAP: pid is %d, fd is %d\n", pid, mem->info.fd);
	ret = xvp_dma_buf_map_helper(xrp_task_dev, mem);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "dma buf map mem failed: %d\n", ret);
		goto put_dma_buf_fill;
	}

	ret = copy_to_user(argp, &mem->info, sizeof(mem->info));
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "copy_to_user failed: %d\n", ret);
		goto put_copy_to_user;
	}

	mutex_lock(&xrp_task_dev->lock);
	list_add_tail(&mem->node, &xrp_task_dev->mem);
	print_xrp_mem_list(&xrp_task_dev->mem);
	mutex_unlock(&xrp_task_dev->lock);

	return 0;

put_copy_to_user:
	(void)xvp_dma_buf_unmap_helper(xrp_task_dev, mem);

put_dma_buf_fill:
	kfree(mem);

	return ret;
}

void set_hw_ops(void *dev, const void *hw_ops, void *hw_arg)
{
	struct device *parent_dev;
	struct platform_device *parent_pdev;
	struct device *child_dev = (struct device *)dev;
	struct xrp_task_device *xrp_task_dev;

	parent_dev = child_dev->parent;
	parent_pdev = to_platform_device(parent_dev);
	xrp_task_dev = platform_get_drvdata(parent_pdev);
	xrp_task_dev->hw_ops = (const struct xrp_hw_ops *)hw_ops;
	xrp_task_dev->hw_arg = hw_arg;
}
EXPORT_SYMBOL(set_hw_ops);

int vdsp_task_memmap(void *dev, struct boot_kernel_mem_map_info *mem_map_info)
{
	int ret;
	struct xrp_mem_node *mem = NULL;
	struct device *parent_dev;
	struct platform_device *parent_pdev;
	struct device *child_dev = (struct device *)dev;
	struct xrp_task_device *xrp_task_dev;
	pid_t pid = current->pid;

	parent_dev = child_dev->parent;

	parent_pdev = to_platform_device(parent_dev);
	xrp_task_dev = platform_get_drvdata(parent_pdev);

	mem = kzalloc(sizeof(*mem), GFP_KERNEL);
	if (!mem) {
		xrp_print(LOG_LEVEL_ERROR, "malloc rpmsg mem node failed\n");
		return -ENOMEM;
	}

	mem->mem_pid = pid;
	memcpy((void *)&mem->info, (void *)&mem_map_info->map_info,
			sizeof(struct vdsp_memmap_info));

	xrp_print(LOG_LEVEL_DEBUG, "MEMMAP: pid is %d, fd is %d\n", pid, mem->info.fd);

	ret = xvp_dma_buf_map_helper(xrp_task_dev, mem);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "dma buf map mem failed: %d\n", ret);
		kfree(mem);
		return ret;
	}

	mem_map_info->dmabuf = (void *)mem->buf->dmabuf;
	mem_map_info->kernel_vaddr = mem->buf->vaddr;

	mutex_lock(&xrp_task_dev->lock);
	list_add_tail(&mem->node, &xrp_task_dev->mem);
	print_xrp_mem_list(&xrp_task_dev->mem);
	mutex_unlock(&xrp_task_dev->lock);

	return ret;
}
EXPORT_SYMBOL(vdsp_task_memmap);

int vdsp_task_memunmap(void *dev, struct vdsp_memmap_info *mem_map_info)
{
	int ret = 0;
	struct device *parent_dev;
	struct platform_device *parent_pdev;
	struct device *child_dev = (struct device *)dev;
	struct xrp_mem_node *mem = NULL, *tmp = NULL;
	struct vdsp_memmap_info info;
	struct xrp_task_device *xrp_task_dev;
	pid_t pid = current->pid;

	parent_dev = child_dev->parent;

	parent_pdev = to_platform_device(parent_dev);
	xrp_task_dev = platform_get_drvdata(parent_pdev);

	memcpy((void *)&info, (void *)mem_map_info, sizeof(struct vdsp_memmap_info));

	mutex_lock(&xrp_task_dev->lock);
	list_for_each_entry_safe(mem, tmp, &xrp_task_dev->mem, node) {
		if ((mem->mem_pid != pid) ||
			(mem->info.fd != info.fd) ||
			(mem->info.size != info.size))
			continue;

		xrp_print(LOG_LEVEL_DEBUG,
			"%s MEM UNMAP: pid is %d, fd is %d\n", __func__, pid, mem->info.fd);

		ret = xvp_dma_buf_unmap_helper(xrp_task_dev, mem);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR,
				"hrpdev dma buf unmap mem failed: %d\n", ret);
			return ret;
		}

		list_del(&mem->node);
		kfree(mem);
		break;
	}
	print_xrp_mem_list(&xrp_task_dev->mem);
	mutex_unlock(&xrp_task_dev->lock);

	return ret;
}
EXPORT_SYMBOL(vdsp_task_memunmap);

static void print_ioctl_command(unsigned int cmd)
{
	if (log_level < LOG_LEVEL_DEBUG)
		return;
	switch (cmd) {
	case XRP_IOCTL_ENQUEUE:
		xrp_print(LOG_LEVEL_DEBUG, "Command: XRP_IOCTL_ENQUEUE\n");
		break;
	case XRP_IOCTL_CANCEL:
		xrp_print(LOG_LEVEL_DEBUG, "Command: XRP_IOCTL_CANCEL\n");
		break;
	case XRP_IOCTL_WAIT:
		xrp_print(LOG_LEVEL_DEBUG, "Command: XRP_IOCTL_WAIT\n");
		break;
	case VDSP_IOCTL_MAP:
		xrp_print(LOG_LEVEL_DEBUG, "Command: VDSP_IOCTL_MAP\n");
		break;
	case VDSP_IOCTL_UNMAP:
		xrp_print(LOG_LEVEL_DEBUG, "Command: VDSP_IOCTL_UNMAP\n");
		break;
	default:
		xrp_print(LOG_LEVEL_DEBUG, "Command: Unknown\n");
		break;
	}
}

static long queue_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	// boot vdsp_exception

	print_ioctl_command(cmd);

	if (is_vdsp_excp_true())
		return -EIO;

	switch (cmd) {
	case XRP_IOCTL_ENQUEUE:
		ret = enqueue_task(file, arg);
		break;
	case XRP_IOCTL_CANCEL:
		ret = dequeue_task(file, arg);
		break;
	case XRP_IOCTL_WAIT:
		ret = wait_for_completed_msg(file, arg);
		break;
	case VDSP_IOCTL_MAP:
		ret = xvp_dma_buf_map(file, (void __user *)arg);
		break;
	case VDSP_IOCTL_UNMAP:
		ret = xvp_dma_buf_unmap(file, (void __user *)arg);
		break;
	default:
		xrp_print(LOG_LEVEL_ERROR, "%s: Unknown command\n", __func__);
		ret = -ENOTTY;
		break;
	}

	return ret;
}


static unsigned int queue_poll(struct file *file, struct poll_table_struct *wait)
{
	return 0;
}

static const struct file_operations queue_fops = {
	.owner = THIS_MODULE,
	.open = queue_open,
	.release = queue_release,
	.unlocked_ioctl = queue_ioctl,
	.poll = queue_poll,
};

//	void process_task(struct xrp_task_info_l *task_info, struct xrp_task_device *xrp_task_dev)
//	{
//		size_t size = sizeof(struct xrp_vdsp_cmds);
//		struct mbox_msg_t test_msg;
//		int ret;
//		struct xrp_dma_buf *xrp_dma_buf_l = (struct xrp_dma_buf *)xrp_task_dev->vdsp_sysram_mem_buf;

//		xrp_print(LOG_LEVEL_DEBUG,
//			"Begin processing task with msg_id: %u, priority: %u\n",
//			task_info->msg_id, task_info->priority);

//		memcpy(xrp_task_dev->vdsp_comm_vaddr_base, (void *)&task_info->cmds, size);
//			xrp_print(LOG_LEVEL_DEBUG,
//			"Commands copied to comm vaddr %llx\n", xrp_task_dev->vdsp_comm_vaddr_base);

//		dma_buf_end_cpu_access(xrp_dma_buf_l->dmabuf, DMA_TO_DEVICE);

//		test_msg.len = 3;
//		test_msg.cv_type = 0x55;
//		test_msg.msg_id = task_info->msg_id;
//		test_msg.priority = task_info->priority;
//		ret = vdsp_mailbox_send(xrp_task_dev->vmbox->tx_ch, (void *)&test_msg);
//		xrp_print(LOG_LEVEL_ERROR, "Failed to vdsp_mailbox_send\n");

//		xrp_print(LOG_LEVEL_DEBUG,
//			"Task processing completed for msg_id: %u\n", task_info->msg_id);
//	};

static uint32_t find_highest_priority_task(struct xrp_task_device *xrp_task_dev,
									int *highest_priority)
{
	struct queue_node *node = NULL;
	struct vdsp_task_priority_queue *taskq = xrp_task_dev->vdsp_taskq;
	int i = 0;
	size_t size = sizeof(struct xrp_vdsp_cmds);

	for (i = xrp_task_dev->num_queues - 1; i >= 0; --i) {
		print_queue_info(&taskq[i]);
		mutex_lock(&taskq[i].lock);
		if (!list_empty(&taskq[i].queue)) {
			if (&taskq[i].queue != taskq[i].cursor) {
				node = list_entry(taskq[i].cursor, struct queue_node, list);
				*highest_priority = xrp_task_dev->priorities[i];
				xrp_print(LOG_LEVEL_PERF, "cp2shm");
				xrp_print(LOG_LEVEL_DEBUG,
					"Commands copied to vdsp_comm_vaddr_base %pK, from %pK\n",
					xrp_task_dev->vdsp_comm_vaddr_base,
					&node->task_info.task.cmds);
				memcpy(xrp_task_dev->vdsp_comm_vaddr_base,
						(void *)&node->task_info.task.cmds,
						size);
				print_all_addrs_on_same_line(&node->task_info.task.cmds);
				mutex_unlock(&taskq[i].lock);
				xrp_print(LOG_LEVEL_PERF, "cp2shm done");
				return node->task_info.task.msg_id;
			}
			xrp_print(LOG_LEVEL_DEBUG,
				"All Task completed for queue: priorities %d\n",
				taskq[i].priority);
		}
		mutex_unlock(&taskq[i].lock);
	}

	*highest_priority = -1;
	return 0;
}

static int queue_thread(void *data)
{
	struct xrp_task_device *xrp_task_dev = (struct xrp_task_device *)data;
	int highest_priority = -1;
	uint32_t msg_id = 0;
	int ret;
	struct mbox_msg_t test_msg = {
		.msg_len = sizeof(struct mbox_msg_t)/4,
		.msg_type = TASK_RUN_SIT,
		.cv_type = 1000};
	struct task_struct *task = current;
	struct sched_param param = { .sched_priority = 92 };

	if (sched_setscheduler(task, SCHED_RR, &param) < 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to set SCHED_RR, use nice=-10\n");
		set_user_nice(task, -10);
	} else {
		xrp_print(LOG_LEVEL_ERROR, "Sched policy: SCHED_RR/30\n");
	}

	xrp_print(LOG_LEVEL_INFO,
		"Queue thread started for device: %s\n",
		dev_name(xrp_task_dev->dev));
	wait_event_interruptible(wq_task_ready, task_ready || kthread_should_stop());
	task_ready = false;

	while (!kthread_should_stop()) {
		while (true) {
			xrp_print(LOG_LEVEL_PERF, "find...");

			msg_id = find_highest_priority_task(xrp_task_dev, &highest_priority);
			if (msg_id) {
				xrp_print(LOG_LEVEL_DEBUG,
					"Processing task with priority: %d, msg_id: %u\n",
						highest_priority, msg_id);
				test_msg.msg_id = msg_id;
				test_msg.priority = highest_priority;
				//print_mbox_msg(&test_msg);
				xrp_print(LOG_LEVEL_PERF, "mbox2v");
				ret = vdsp_mailbox_send(xrp_task_dev->vmbox->tx_ch,
										(void *)&test_msg, xrp_task_dev);
				if (ret < 0) {
					is_mbx_send_ok = false;
					xrp_print(LOG_LEVEL_ERROR,
						"Failed to vdsp_mailbox_send, please cancel cur msg_id!\n");
					break;
				}
				is_mbx_send_ok = true;
				xrp_print(LOG_LEVEL_PERF, "mbox2v done");
				break;
			}

			xrp_print(LOG_LEVEL_DEBUG,
						"No task found, sleeping and wait event\n");
			wait_event_interruptible(wq_task_ready,
									task_ready ||
									kthread_should_stop());
			if (kthread_should_stop())
				return 0;
			task_ready = false;
		}

		if (is_mbx_send_ok) {
			xrp_print(LOG_LEVEL_DEBUG,
							"Queue thread waiting for event or interrupt...\n");
			wait_event_interruptible(vdsp_wq, interrupt_occurred || is_vdsp_pw_false() || kthread_should_stop());
			if (kthread_should_stop())
				return 0;
			interrupt_occurred = false;
			receive_mailbox_msg(xrp_task_dev, msg_id, highest_priority);
		} else {
			cancel_mbx_fail_task(xrp_task_dev, msg_id, highest_priority);
			xrp_print(LOG_LEVEL_ERROR, "send mbx fail, cancel msg_id %u\n", msg_id);
		}
		cond_resched();
	}

	return 0;
}

static void xrp_print_dt_data(const struct device *dev, const struct xrp_task_device *data)
{
	int i;

	xrp_print(LOG_LEVEL_INFO, "%s: Number of queues: %u\n",
					dev_name(dev), data->num_queues);

	if (data->priorities) {
		xrp_print(LOG_LEVEL_INFO, "%s: Queue priorities:\n", dev_name(dev));
		for (i = 0; i < data->num_queues; i++)
			xrp_print(LOG_LEVEL_INFO, "%s:  Priority of queue %d: %u\n",
							dev_name(dev), i, data->priorities[i]);
	}
}


static int xrp_parse_dt(struct device *dev, struct xrp_task_device *data)
{
	struct device_node *node = dev->of_node;
	int length, ret;

	ret = of_property_read_u32(node, "num-queues", &data->num_queues);
	if (ret) {
		dev_err(dev, "Failed to read 'num-queues' property\n");
		return ret;
	}

	if (of_find_property(node, "queue-priorities", &length)) {
		if ((length / sizeof(u32)) != data->num_queues) {
			dev_err(dev, "Invalid 'queue-priorities' length\n");
			return -EINVAL;
		}

		data->priorities = devm_kzalloc(dev, length, GFP_KERNEL);
		if (!data->priorities)
			return -ENOMEM;

		ret = of_property_read_u32_array(node,
							"queue-priorities",
							data->priorities,
							length / sizeof(u32));
		if (ret) {
			dev_err(dev, "Failed to read 'queue-priorities' property\n");
			return ret;
		}
	} else {
		dev_err(dev, "'queue-priorities' property not found\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(node, "iova-start", &data->region.iova);
	if (ret < 0) {
		xrp_print(LOG_LEVEL_ERROR,
							"%s get iova failed: %d\n",
							dev_name(dev), ret);
		return ret;
	}

	ret = of_property_read_u32(node, "iova-len", (u32 *)&data->region.size);
	if (ret < 0) {
		xrp_print(LOG_LEVEL_ERROR,
							"%s get iova len failed: %d\n",
							dev_name(dev), ret);
		return ret;
	}

	xrp_print_dt_data(dev, data);

	return 0;
}

struct vdsp_task_priority_queue *create_vdsp_task_priority_queues(size_t count)
{
	struct vdsp_task_priority_queue *queues;
	size_t i;

	queues = kmalloc_array(count, sizeof(struct vdsp_task_priority_queue), GFP_KERNEL);
	if (!queues)
		return NULL;

	for (i = 0; i < count; i++) {
		mutex_init(&queues[i].lock);
		INIT_LIST_HEAD(&queues[i].queue);
		queues[i].cursor = &queues[i].queue;
		queues[i].priority = i;
	}

	return queues;
}

#define ALIGN_SIZE_TO_4K(size) ((size) & ~0xFFF)

static int add_address_ranges_to_pool(struct gen_pool *pool)
{
	struct {
		unsigned long start;
		unsigned long size;
	} address_ranges[] = {
		{0x00001000, ALIGN_SIZE_TO_4K(0x0fffffff - 0x00001000 + 1)},
		{0x10300000, ALIGN_SIZE_TO_4K(0x1ffc0000 - 0x10300000 + 1)},
		{0x20000000, ALIGN_SIZE_TO_4K(0x3ffc0000 - 0x20000000 + 1)},
		{0x40000000, ALIGN_SIZE_TO_4K(0x4fffffff - 0x40000000 + 1)},
		{0x51000000, ALIGN_SIZE_TO_4K(0x5ffc0000 - 0x51000000 + 1)},
		{0x60000000, ALIGN_SIZE_TO_4K(0x7ffc0000 - 0x60000000 + 1)},
		{0x80000000, ALIGN_SIZE_TO_4K(0x9ffc0000 - 0x80000000 + 1)},
		{0xa0000000, ALIGN_SIZE_TO_4K(0xbff00000 - 0xa0000000 + 1)},
		{0xc0000000, ALIGN_SIZE_TO_4K(0xcfffffff - 0xc0000000 + 1)},
		{0xd1000000, ALIGN_SIZE_TO_4K(0xdffc0000 - 0xd1000000 + 1)},
		{0xe0000000, ALIGN_SIZE_TO_4K(0xe9000000 - 0xe0000000 + 1)},
		{0xea000000, ALIGN_SIZE_TO_4K(0xfffc0000 - 0xea000000 + 1)},
	};

	int ret;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(address_ranges); i++) {
		ret = gen_pool_add(pool, address_ranges[i].start, address_ranges[i].size, -1);
		if (ret != 0) {
			xrp_print(LOG_LEVEL_ERROR,
					"gen_pool_add failed for range %lx - %lx: %d\n",
					address_ranges[i].start,
					address_ranges[i].start + address_ranges[i].size - 1,
					ret);
			return ret;
		}
	}

	return 0;
}


void destroy_vdsp_task_priority_queues(struct xrp_task_device *xrp_task_dev, size_t count)
{
	struct queue_node *node, *tmp;
	size_t i;
	struct vdsp_task_priority_queue *queues = xrp_task_dev->vdsp_taskq;

	if (!queues)
		return;

	for (i = 0; i < count; i++) {
		list_for_each_entry_safe(node, tmp, &queues[i].queue, list) {
			list_del(&node->list);
			kmem_cache_free(xrp_task_dev->queue_node_cache, node);
		}
	}

	kfree(queues);
}


static int xrp_register_mdr_frame(struct platform_device *pdev, struct xrp_task_device *xtdev)
{
	struct xrp_device *xdev;
	int ret = 0;

	xrp_print(LOG_LEVEL_INFO, "register mdr frame entry %s\n", __func__);
	xdev = devm_kzalloc(&pdev->dev, sizeof(*xdev), GFP_KERNEL);
	if (!xdev) {
		xrp_print(LOG_LEVEL_ERROR, "xrp xdev zalloc failed\n");
		return -ENOMEM;
	}

	xdev->pdev = pdev;
	xdev->xtdev = xtdev;

	xtdev->xdev = xdev;
	ret = xmdr_init(xdev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xmdr_init failed: %d\n", ret);
		return ret;
	}

	xrp_print(LOG_LEVEL_INFO, "register mdr frame end %s\n", __func__);
	return ret;
}

static int xrp_unregister_mdr_frame(struct platform_device *pdev)
{
	struct xrp_task_device *xtdev;
	struct xrp_device *xdev;
	int ret = 0;

	xrp_print(LOG_LEVEL_INFO, "unregister mdr frame entry %s\n", __func__);
	xtdev = dev_get_drvdata(&pdev->dev);

	xdev = xtdev->xdev;

	ret = xmdr_exit(xdev);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "xmdr_exit failed: %d\n", ret);
		return ret;
	}

	xrp_print(LOG_LEVEL_INFO, "unregister mdr frame end %s\n", __func__);
	return ret;
}

static int priority_queue_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct xrp_task_device *queue_dev_local;

	queue_dev_local = devm_kzalloc(dev, sizeof(*queue_dev_local), GFP_KERNEL);
	if (!queue_dev_local)
		return -ENOMEM;

	platform_set_drvdata(pdev, queue_dev_local);
	queue_dev_local->dev = dev;

	ret = xrp_parse_dt(dev, queue_dev_local);
	if (ret) {
		dev_err(dev, "Failed to parse device tree\n");
		return ret;
	}

	queue_dev_local->vdsp_taskq = create_vdsp_task_priority_queues(queue_dev_local->num_queues);
	if (!queue_dev_local->vdsp_taskq) {
		dev_err(dev, "Failed to allocate memory for device\n");
		return -ENOMEM;
	}

	queue_dev_local->thread = kthread_run(queue_thread, queue_dev_local, "queue_thread");
	if (IS_ERR(queue_dev_local->thread)) {
		xrp_print(LOG_LEVEL_ERROR,
			"Failed to create thread: %ld\n",
			PTR_ERR(queue_dev_local->thread));
		return PTR_ERR(queue_dev_local->thread);
	}

	queue_dev_local->queue_node_cache = kmem_cache_create("queue_node_cache",
									sizeof(struct queue_node),
									0,
									SLAB_HWCACHE_ALIGN,
									NULL);
	if (!queue_dev_local->queue_node_cache) {
		dev_err(dev, "Failed to create queue_node cache\n");
		return -ENOMEM;
	}

	queue_dev_local->pool = gen_pool_create(12, -1);
	if (!queue_dev_local->pool) {
		xrp_print(LOG_LEVEL_ERROR, "%s gen pool create failed\n", dev_name(dev));
		ret = -ENOMEM;
		goto err_alloc;
	}

	ret = add_address_ranges_to_pool(queue_dev_local->pool);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR,
							"%s: Adding address ranges to pool failed: %d\n",
							dev_name(dev), ret);
		goto err_gen_pool;
	}

	queue_dev_local->domain = iommu_get_domain_for_dev(dev);
	if (!queue_dev_local->domain) {
		xrp_print(LOG_LEVEL_ERROR, "%s get iommu domain failed\n", dev_name(dev));
		ret = -ENODEV;
		goto err_gen_pool_add;
	}


	INIT_LIST_HEAD(&queue_dev_local->mem);

	queue_dev_local->open_count = 0;
	queue_dev_local->is_first_open = true;
	mutex_init(&queue_dev_local->open_mutex);
	mutex_init(&queue_dev_local->lock);
	mutex_init(&print_func_lock);

	queue_dev_local->miscdev = (struct miscdevice){
		.minor = MISC_DYNAMIC_MINOR,
		.name = "xvp_queue",
		.fops = &queue_fops,
	};

	ret = misc_register(&queue_dev_local->miscdev);
	if (ret < 0) {
		xrp_print(LOG_LEVEL_ERROR, "Failed to register misc device\n");
		goto err_mbox_init;
	}

	ret = of_platform_populate(dev->of_node, NULL, NULL, dev);
	if (ret) {
		dev_err(dev, "Failed to populate child devices\n");
		goto err_mbox_init;
	}

	xrp_print(LOG_LEVEL_INFO, "Queue driver initialized\n");

#if OPEN_MDR_FRAME
	if (is_switch_on(SWITCH_HMDR)) {
		xrp_print(LOG_LEVEL_INFO, "hmdr_nv is enable, then init xmdr\n");
		ret = xrp_register_mdr_frame(pdev, queue_dev_local);
		if (ret) {
			xrp_print(LOG_LEVEL_ERROR, "xrp_register_mdr_frame Failed\n");
			goto err_mbox_init;
		}
	} else {
		xrp_print(LOG_LEVEL_INFO, "hmdr_nv is disenable, will not init xmdr\n");
	}
#endif

	g_vdsp_sys_pm_wksrc = wakeup_source_register(NULL, "vdsp_sys_pm_wksrc");
	if (g_vdsp_sys_pm_wksrc == NULL) {
		xrp_print(LOG_LEVEL_ERROR, "%s:%d g_vdsp_sys_pm_wksrc create err!\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto err_mbox_init;
	}

	ret = rqs_sysncached_heap(queue_dev_local);
	if (ret) {
		xrp_print(LOG_LEVEL_ERROR, "rqs_sysncached_heap Failed\n");
		goto err_mbox_init;
	}
	INIT_LIST_HEAD(&queue_dev_local->uncache_heap_list);
	mutex_init(&queue_dev_local->dma_buf_lock);
	xrp_print(LOG_LEVEL_INFO, "uncache_heap and list initialize done\n");
	return 0;

err_mbox_init:
	if (queue_dev_local->if_use_mbox)
		vdsp_mbox_ch_deinit(queue_dev_local->vmbox);

err_gen_pool_add:
	gen_pool_destroy(queue_dev_local->pool);

err_gen_pool:

err_alloc:

	return ret;

}

static int priority_queue_remove(struct platform_device *pdev)
{
	struct xrp_task_device *queue_dev_local = platform_get_drvdata(pdev);

	of_platform_depopulate(&pdev->dev);

	if (queue_dev_local) {
		if (queue_dev_local->thread && !IS_ERR(queue_dev_local->thread)) {
			kthread_stop(queue_dev_local->thread);
			queue_dev_local->thread = NULL;
		}

		destroy_vdsp_task_priority_queues(queue_dev_local, queue_dev_local->num_queues);

		if (queue_dev_local->pool)
			gen_pool_destroy(queue_dev_local->pool);

		kmem_cache_destroy(queue_dev_local->queue_node_cache);

		mutex_destroy(&queue_dev_local->lock);
		mutex_destroy(&queue_dev_local->open_mutex);

		misc_deregister(&queue_dev_local->miscdev);
		rls_sysncached_heap(queue_dev_local);
		mutex_destroy(&queue_dev_local->dma_buf_lock);
	}

#if OPEN_MDR_FRAME
	if (is_switch_on(SWITCH_HMDR)) {
		xrp_print(LOG_LEVEL_INFO, "hmdr_nv is enable, then de-init xmdr\n");
		int ret = 0;

		ret = xrp_unregister_mdr_frame(pdev);
		if (ret)
			xrp_print(LOG_LEVEL_ERROR, "xrp_register_mdr_frame Failed\n");
	} else {
		xrp_print(LOG_LEVEL_INFO, "hmdr_nv is disenable, will not de-init xmdr\n");
	}
#endif
	xrp_print(LOG_LEVEL_INFO, "Queue driver exited\n");
	return 0;

}

static const struct of_device_id xrp_priority_queue_of_match[] = {
	{ .compatible = "xring,vdsp-task-queue" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, xrp_priority_queue_of_match);

static struct platform_driver xrp_priority_queue_driver = {
	.probe = priority_queue_probe,
	.remove = priority_queue_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(xrp_priority_queue_of_match),
		.owner = THIS_MODULE,
	},
};
module_platform_driver(xrp_priority_queue_driver);


MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("xrp task queue Driver");
MODULE_LICENSE("GPL v2");
