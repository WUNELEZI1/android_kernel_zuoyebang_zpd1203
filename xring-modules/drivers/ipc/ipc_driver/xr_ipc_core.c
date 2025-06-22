// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/semaphore.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>
#include <linux/errno.h>
#include <linux/list.h>

#include <soc/xring/trace_hook_set.h>
#include <soc/xring/ipc/xr_ipc.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/securelib/securectype.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>

#include "xr_ipc_trace_hook.h"
#include "xr_mbox.h"

#define IPC_EOK				(0)
#define IPC_CORE_PR_ERROR		(-1)

#define IPC_TIMEOUT_CNT_MAX		(200)

#define ipc_core_pr_err(fmt, args...)	pr_err("[xr_ipc]" fmt "\n", ##args)
#define ipc_core_pr_info(fmt, args...)	pr_info("[xr_ipc]" fmt "\n", ##args)
#ifdef IPC_DEBUG_ENABLED
#define ipc_core_pr_dbg(fmt, args...)	pr_debug("[xr_ipc]" fmt "\n", ##args)
#else
#define ipc_core_pr_dbg(fmt, args...) \
	do { } while (0)
#endif

#define TX_FIFO_CELL_SIZE		(sizeof(struct xr_mbox_task))
#define MAILBOX_MAX_TX_FIFO		(256)
/* tx_thread warn level to bug_on when tx_thread is blocked by some reasons */
#define TX_THREAD_BUFFER_WARN_LEVEL	(156 * TX_FIFO_CELL_SIZE)

#define WAIT_FOR_WAIT_SYNC_SEND		(20)

#define MBOX_USED_REG_NUM(len)		((len + 3) >> 2)

#define IPC_WAIT_RESP			(0)
#define IPC_WAIT_ACK			(1)

#define IPC_TASK_SEND_MSG		(0)
#define IPC_TASK_SEND_RESP		(1)

#define IPC_ASYNC_SEND_CONTINUE		(0)
#define IPC_ASYNC_SEND_OVER		(1)

enum {
	NOCOMPLETION = 0,
	COMPLETING,
	COMPLETED
};

enum {
	TX_TASK = 0,
	RX_TASK
};

enum  {
	RECV_ACK = 0,
	RECV_MSG,
	RECV_RESP
};

enum ipc_send_mode {
	SEND_SYNC = 0,
	SEND_RECV,
	SEND_ASYNC
};

/* mailbox device resource pool */
static LIST_HEAD(mdevices);

#define CONFIG_XR_IPC_PERFORMANCE_DEBUG
static void ipc_dbg_recv_msg_pr(struct xr_mbox_device *mdev, unsigned int *rx_buffer)
{
#ifdef CONFIG_XR_IPC_PERFORMANCE_DEBUG
	int i;

	for (i = 0; i < mdev->capability; i++)
		ipc_core_pr_dbg("rx_buffer[%d] = 0x%08x\n", i, rx_buffer[i]);
#endif
}

static void ipc_dbg_send_time_pr(u64 start_time)
{
#ifdef CONFIG_XR_IPC_PERFORMANCE_DEBUG
	u64 end_time;
	u64 diff;

	ipc_core_pr_dbg("send start time: %llu", start_time);

	end_time = ktime_get_ns();
	diff = end_time - start_time;
	ipc_core_pr_dbg("send used time: %llu", diff);
#endif
}

static struct xr_mbox_device *mbox_device_get(unsigned int vc_id)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_device *_mdev = NULL;
	struct xr_mbox_device_priv *_priv = NULL;
	struct list_head *list = &mdevices;

	list_for_each_entry(_mdev, list, node) {
		_priv = (struct xr_mbox_device_priv *)_mdev->priv;
		if (_priv->vc_id == vc_id) {
			mdev = _mdev;
			break;
		}
	}

	return mdev;
}

struct xr_ipc *xr_ipc_get_xdev(void)
{
	struct xr_mbox_device_priv *priv = NULL;
	struct xr_mbox_device *mdev = NULL;
	struct xr_ipc_device *idev = NULL;

	mdev = mbox_device_get(0);
	if (mdev == NULL) {
		ipc_core_pr_err("get mdev failed\n");
		return NULL;
	}

	priv = (struct xr_mbox_device_priv *)mdev->priv;
	if (priv == NULL) {
		ipc_core_pr_err("get priv failed\n");
		return NULL;
	}

	idev = (struct xr_ipc_device *)priv->idev;
	if (idev == NULL) {
		ipc_core_pr_err("get idev failed\n");
		return NULL;
	}

	return idev->xdev;
}

static unsigned int list_size(struct list_head *head)
{
	unsigned int count = 0;
	struct list_head *pos;

	list_for_each(pos, head)
		count++;
	return count;
}

struct rproc_list *vc_id_get(void)
{
	struct xr_mbox_device_priv *priv = NULL;
	struct xr_mbox_device *temp_mdev = NULL;
	struct xr_mbox_device *mdev = NULL;
	struct list_head *list = &mdevices;
	struct rproc_list *vc;
	u32 size;
	int i = 0;

	vc = kcalloc(1, sizeof(struct rproc_list), GFP_KERNEL);
	if (vc == NULL) {
		ipc_core_pr_err("vc memory allocation failed\n");
		return NULL;
	}

	size = list_size(list);
	vc->vc_list = kcalloc(size, sizeof(int), GFP_KERNEL);
	if (vc->vc_list == NULL) {
		ipc_core_pr_err("vc list memory allocation failed\n");
		kfree(vc);
		return NULL;
	}

	list_for_each_entry_safe(mdev, temp_mdev, list, node) {
		priv = (struct xr_mbox_device_priv *)mdev->priv;
		vc->vc_list[i] = priv->vc_id;
		i++;
	}
	vc->vc_num = i;

	return vc;
}

void vc_id_free(struct rproc_list *list)
{
	if (!list) {
		ipc_core_pr_err("invalid list");
		return;
	}

	if (!list->vc_list) {
		ipc_core_pr_err("invalid vc_list");
		return;
	}

	kfree(list->vc_list);
	list->vc_list = NULL;

	kfree(list);
}

unsigned int xr_ipc_capa_get(void)
{
	struct xr_mbox_device *mdev = NULL;
	struct list_head *list = &mdevices;
	int capa = 0;

	list_for_each_entry(mdev, list, node) {
		capa = mdev->capability;
		break;
	}

	return capa;
}

static int set_status(struct xr_mbox_device *mdev, int status)
{
	int ret = 0;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->status_lock, flags);
	if ((MDEV_DEACTIVATED & mdev->status)) {
		spin_unlock_irqrestore(&mdev->status_lock, flags);
		ipc_core_pr_info("an unexpected ipc caused by %s", mdev->name);
		ret = -ENODEV;
		goto out;
	} else if ((MDEV_DEACTIVATED & (unsigned int)status)) {
		mdev->status |= (unsigned int)status;

		while ((MDEV_SEND_RECV & mdev->status) ||
			(MDEV_SYNC_SENDING & mdev->status) ||
			(MDEV_ASYNC_ENQUEUE  & mdev->status)) {
			spin_unlock_irqrestore(&mdev->status_lock, flags);
			msleep(WAIT_FOR_WAIT_SYNC_SEND); /* wait for sync_send */
			spin_lock_irqsave(&mdev->status_lock, flags);
		}
	} else {
		mdev->status |= (unsigned int)status;
	}

	spin_unlock_irqrestore(&mdev->status_lock, flags);
out:
	return ret;
}

static inline void clr_status(struct xr_mbox_device *mdev, int status)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->status_lock, flags);
	mdev->status &= ~(unsigned int)status;
	spin_unlock_irqrestore(&mdev->status_lock, flags);
}

static int ipc_wait(struct xr_mbox_device *mdev, int is_ack)
{
	u64 start_time;
	u64 end_time;
	u64 diff;
	unsigned long timeout_jiff;
	long timeout;
	unsigned int mdev_timeout;
	struct completion *complete;
	unsigned long flags = 0;

	if (is_ack) {
		mdev_timeout = mdev->ops->get_ack_timeout(mdev);
		complete = &mdev->ack_complete;
	} else {
		mdev_timeout = mdev->ops->get_resp_timeout(mdev);
		complete = &mdev->resp_complete;
	}

	timeout_jiff = msecs_to_jiffies(mdev_timeout);
	start_time = ktime_get_ns();
	timeout = wait_for_completion_timeout(complete, timeout_jiff);
	ipc_dbg_send_time_pr(start_time);
	if (unlikely(timeout == 0)) {
		xr_ipc_pr_ack_timeout(mdev);

		end_time = ktime_get_ns();
		diff = end_time - start_time;
		ipc_core_pr_err("%s wait %s timeout\n"
				"MSG[0]: 0x%08x  MSG[1]: 0x%08x\n"
				"mntn_diff_time : %llu",
				mdev->name, is_ack ? "ack" : "resp",
				mdev->tx_task->tx_buffer[0],
				mdev->tx_task->tx_buffer[1],
				diff);

		/* Force complete status when timeout */
		if (is_ack) {
			spin_lock_irqsave(&mdev->ack_complete_lock, flags);
			mdev->ops->release(mdev);
			mdev->ack_completed = COMPLETED;
			mdev->tx_task = NULL;
			spin_unlock_irqrestore(&mdev->ack_complete_lock, flags);
		} else {
			spin_lock_irqsave(&mdev->resp_complete_lock, flags);
			mdev->resp_completed = COMPLETED;
			mdev->tx_task = NULL;
			spin_unlock_irqrestore(&mdev->resp_complete_lock, flags);
		}

		mdev->timeout_num++;

		if (mdev->ops->dump_status && (mdev->timeout_num >= IPC_TIMEOUT_CNT_MAX))
			mdev->ops->dump_status(mdev);

		return -EDOM;
	}

	return 0;
}

static int ipc_wait_ack(struct xr_mbox_device *mdev)
{
	return ipc_wait(mdev, IPC_WAIT_ACK);
}

static int ipc_wait_resp(struct xr_mbox_device *mdev)
{
	return ipc_wait(mdev, IPC_WAIT_RESP);
}

static int ipc_task_send(struct xr_mbox_device *mdev,
			struct xr_mbox_task *tx_task, int is_resp)
{
	int ret;
	unsigned long flags = 0;

	ret = mdev->ops->ensure_channel(mdev);
	if (ret) {
		ipc_core_pr_err("mdev %s SOURCE write failed", mdev->name);
		return ret;
	}
	mdev->tx_task = tx_task;
	mdev->tx_task->send_len = 0;

	spin_lock_irqsave(&mdev->ack_complete_lock, flags);
	mdev->ack_complete.done = 0;
	mdev->ack_completed = NOCOMPLETION;
	spin_unlock_irqrestore(&mdev->ack_complete_lock, flags);

	if (is_resp)
		ret = mdev->ops->resp(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len);
	else
		ret = mdev->ops->send(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len);
	if (ret) {
		ipc_core_pr_err("mdev %s can not be sent", mdev->name);
		goto out;
	}

	ret = ipc_wait_ack(mdev);

out:
	mdev->tx_task = NULL;
	/* completion */
	mdev->ops->release(mdev);

	return ret;
}

static int ipc_task_send_recv(struct xr_mbox_device *mdev, struct xr_mbox_task *tx_task)
{
	int ret;
	unsigned long flags = 0;

	ret = mdev->ops->ensure_channel(mdev);
	if (ret) {
		ipc_core_pr_err("mdev %s SOURCE write failed", mdev->name);
		return ret;
	}

	mdev->tx_task = tx_task;

	spin_lock_irqsave(&mdev->ack_complete_lock, flags);
	mdev->ack_complete.done = 0;
	mdev->ack_completed = NOCOMPLETION;
	spin_unlock_irqrestore(&mdev->ack_complete_lock, flags);

	spin_lock_irqsave(&mdev->resp_complete_lock, flags);
	mdev->resp_complete.done = 0;
	mdev->resp_completed = NOCOMPLETION;
	spin_unlock_irqrestore(&mdev->resp_complete_lock, flags);

	ret = mdev->ops->send(mdev, tx_task->tx_buffer, tx_task->tx_buffer_len);
	if (ret) {
		ipc_core_pr_err("mdev %s can not be sent", mdev->name);
		goto out;
	}

	ret = ipc_wait_ack(mdev);
	if (ret)
		goto out;
	mdev->ops->release(mdev);

	ret = ipc_wait_resp(mdev);

out:
	mdev->tx_task = NULL;
	/* completion */
	ipc_core_pr_dbg("mdev %s completion", mdev->name);

	return ret;
}

static int ipc_dequeue_task(struct xr_mbox_device *mdev, struct xr_mbox_task *tx_task)
{
	int ret = 0;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->fifo_lock, flags);
	if (kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) {
		if (!kfifo_out(&mdev->fifo, tx_task, TX_FIFO_CELL_SIZE))
			ret = IPC_CORE_PR_ERROR;
	} else {
		ret = IPC_CORE_PR_ERROR;
	}

	spin_unlock_irqrestore(&mdev->fifo_lock, flags);

	return ret;
}

int xr_mbox_fastack_sta(unsigned int ipc_id)
{
	struct xr_mbox_device *mdev = mbox_device_get(ipc_id);
	struct xr_mbox_device_priv *priv = NULL;

	if (!mdev)
		return IPC_GENE_ACK;
	priv = mdev->priv;

	return priv->fast_ack;
}

static int ipc_tx_thread(void *context)
{
	struct xr_mbox_device *mdev = (struct xr_mbox_device *)context;
	struct xr_mbox_task tx_task;
	int ret = 0;

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(mdev->tx_wait,
			(kfifo_len(&mdev->fifo) >= TX_FIFO_CELL_SIZE) ||
			kthread_should_stop());

		if (kthread_should_stop())
			break;

		mutex_lock(&mdev->dev_lock);
		/*
		 * kick out the async send request from  mdev's kfifo one by one
		 * and send it out
		 */
		ret = ipc_dequeue_task(mdev, &tx_task);
		if (!ret) {
			ret = ipc_task_send(mdev, &tx_task, IPC_TASK_SEND_MSG);
			if (tx_task.ack_cb)
				tx_task.ack_cb(ret, tx_task.ack_arg);
			atomic_inc(&mdev->async_trigger_num);

			/* current task unlinked */
			mdev->tx_task = NULL;

			/* Check if we need to clear the FIFO */
			if (atomic_read(&mdev->async_reset_flag) == IPC_ASYNC_SEND_OVER) {
				kfifo_reset(&mdev->fifo);
				ipc_core_pr_info("%s FIFO is cleared after task send.\n",
						mdev->name);
				atomic_set(&mdev->async_reset_flag, IPC_ASYNC_SEND_CONTINUE);
			}
		}
		mutex_unlock(&mdev->dev_lock);
	}
	return ret;
}

static bool ipc_rx_fifo_write(struct xr_mbox_device *mdev, int mode)
{
	struct xr_mbox_rx_fifo *fifo = &mdev->rx_fifo;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->rx_lock, flags);
	if (fifo->count >= IPC_RX_FIFO_SIZE) {
		spin_unlock_irqrestore(&mdev->rx_lock, flags);
		ipc_core_pr_err("mdev %s rx fifo is full", mdev->name);
		return false;
	}

	mdev->ops->read(mdev, fifo->rx_msg[fifo->write_index].rx_buff);
	fifo->rx_msg[fifo->write_index].cur_task = IPC_MASK(mode);
	fifo->write_index = (fifo->write_index + 1) % IPC_RX_FIFO_SIZE;
	fifo->count++;
	spin_unlock_irqrestore(&mdev->rx_lock, flags);

	mdev->ops->ack(mdev);

	return true;
}

static bool ipc_rx_fifo_read(struct xr_mbox_device *mdev,
		unsigned int *rx_buff, unsigned int *task)
{
	struct xr_mbox_rx_fifo *fifo = &mdev->rx_fifo;
	unsigned long flags = 0;
	int ret = 0;

	spin_lock_irqsave(&mdev->rx_lock, flags);
	if (fifo->count <= 0) {
		spin_unlock_irqrestore(&mdev->rx_lock, flags);
		return false;
	}

	ret = memcpy_s((void *)rx_buff,
			MBOX_CHAN_DATA_SIZE,
			(void *)fifo->rx_msg[fifo->read_index].rx_buff,
			MBOX_CHAN_DATA_SIZE);
	if (ret) {
		spin_unlock_irqrestore(&mdev->rx_lock, flags);
		ipc_core_pr_err("mdev %s rx fifo memcpy_s failed", mdev->name);
		return false;
	}
	*task = __ffs(fifo->rx_msg[fifo->read_index].cur_task);
	fifo->read_index = (fifo->read_index + 1) % IPC_RX_FIFO_SIZE;
	fifo->count--;
	spin_unlock_irqrestore(&mdev->rx_lock, flags);

	return true;
}

static void ipc_rx_bh(unsigned long context)
{
	struct xr_mbox_device *mdev = (struct xr_mbox_device *)(uintptr_t)context;
	struct xr_mbox_device_priv *priv = mdev->priv;
	unsigned int rx_buffer[MBOX_MAX_DATA_REG];
	unsigned long flags = 0;
	unsigned int task = 0;
	int ret = 0;
#ifdef CONFIG_XR_IPC_PERFORMANCE_DEBUG
	unsigned int pr_buffer[MBOX_MAX_DATA_REG];
#endif

	ipc_core_pr_dbg("mdev %s rx_bh enter", mdev->name);

	ipc_trace_hook_call_chain(priv->vc_id, IPC_IRQ_LOW, IPC_DIRECTION_IN);

	while (ipc_rx_fifo_read(mdev, rx_buffer, &task) == true) {
		switch (task) {
		case RECV_MSG:
			atomic_notifier_call_chain(
				&mdev->notifier, priv->vc_id, (void *)rx_buffer);

			ipc_dbg_recv_msg_pr(mdev, rx_buffer);
			atomic_inc(&mdev->recv_num);
			break;

		case RECV_RESP:
			spin_lock_irqsave(&mdev->resp_complete_lock, flags);
			if ((mdev->resp_completed == COMPLETED) || (!mdev->tx_task)) {
				ret = -EBUSY;
			} else {
				ret = memcpy_s((void *)mdev->tx_task->rx_buffer,
						MBOX_CHAN_DATA_SIZE,
						(void *)rx_buffer,
						mdev->tx_task->rx_buffer_len);
#ifdef CONFIG_XR_IPC_PERFORMANCE_DEBUG
				ret = memcpy_s((void *)pr_buffer,
						MBOX_CHAN_DATA_SIZE,
						(void *)rx_buffer,
						mdev->tx_task->rx_buffer_len);
#endif

				mdev->resp_completed = COMPLETED;
				complete(&mdev->resp_complete);
			}
			spin_unlock_irqrestore(&mdev->resp_complete_lock, flags);

			if (ret) {
				ipc_core_pr_err("mdev %s tx_task rx_buffer memcpy_s failed\n"
						"or response timeout, ret = %d",
						mdev->name, ret);
				break;
			}
			ipc_dbg_recv_msg_pr(mdev, pr_buffer);

			break;

		default:
			ipc_core_pr_err("mdev %s has invalied cur_task", mdev->name);
			break;
		}
	}

	mdev->ops->enable_irq(mdev);
	ipc_core_pr_dbg("mdev %s rx_bh leave", mdev->name);

	ipc_trace_hook_call_chain(priv->vc_id, IPC_IRQ_LOW, IPC_DIRECTION_OUT);
}

static bool ipc_ack_handler(struct xr_mbox_device *mdev)
{
	bool has_completed = false;
	unsigned long flags = 0;

	spin_lock_irqsave(&mdev->ack_complete_lock, flags);
	if (mdev->ack_completed == COMPLETED) {
		has_completed = true;
	} else if (mdev->tx_task->tx_buffer_len > 0) {
		xr_mdev_send_set(mdev, IPC_MBOX_SEND_MSG);
		xr_mdev_hw_send(mdev, mdev->tx_task->tx_buffer,
				mdev->tx_task->tx_buffer_len);
	} else {
		mdev->ops->clr_ack(mdev);

		mdev->ack_completed = COMPLETED;
		complete(&mdev->ack_complete);

		atomic_inc(&mdev->async_ack_num);
	}
	spin_unlock_irqrestore(&mdev->ack_complete_lock, flags);

	return has_completed;
}

irqreturn_t ipc_interrupt(int irq, void *p)
{
	struct xr_mbox_device *mdev = (struct xr_mbox_device *)p;
	struct xr_mbox_device_priv *priv = mdev->priv;
	unsigned int *rx_buffer = NULL;
	bool has_completed = false;
	bool bh_flag = false;
	int irq_sta;
	int ret = IRQ_HANDLED;

	if (!mdev->ops) {
		ipc_core_pr_err("an unexpected interrupt");
		return IRQ_NONE;
	}

	if (!mdev->configured) {
		ipc_core_pr_err("mdev %s has not startup yet", mdev->name);
		return IRQ_NONE;
	}

	ipc_trace_hook_call_chain(priv->vc_id, IPC_IRQ_TOP, IPC_DIRECTION_IN);

	irq_sta = mdev->ops->irq_sta(mdev);

	if (!irq_sta) {
		ipc_core_pr_dbg("an unexpected interrupt %d occurred", irq);
		ret = IRQ_NONE;
		goto out;
	}

	if (irq_sta & IPC_MASK(ACK_IRQ)) {
		ipc_core_pr_dbg("mdev %s receive ACK_IRQ", mdev->name);
		if (mdev->ops->msta(mdev)) {
			ipc_core_pr_dbg("The mailbox[%s] is currently used by another vc_id",
					mdev->name);
		} else {
			has_completed = ipc_ack_handler(mdev);
		}

		if (has_completed == true) {
			mdev->ops->dump_status(mdev);
			ipc_core_pr_info("%s has been handled", mdev->name);
		}
	}

	if (irq_sta & IPC_MASK(RECV_IRQ)) {
		ipc_core_pr_dbg("mdev %s receive RECV_IRQ", mdev->name);

		if ((priv->fast_ack == IPC_FAST_ACK) && (mdev->recv_cb)) {
			mdev->ops->recv(mdev, &rx_buffer);
			mdev->recv_cb((void *)rx_buffer, mdev->recv_arg);
			mdev->ops->ack(mdev);

			atomic_inc(&mdev->recv_num);
		} else {
			bh_flag = ipc_rx_fifo_write(mdev, RECV_MSG);
		}
	} else if (irq_sta & IPC_MASK(RESP_IRQ)) {
		ipc_core_pr_dbg("mdev %s receive RESP_IRQ", mdev->name);
		bh_flag = ipc_rx_fifo_write(mdev, RECV_RESP);
	}

	if (bh_flag == true) {
		mdev->ops->disable_irq(mdev);
		tasklet_schedule(&mdev->rx_bh);
	}

	ipc_core_pr_dbg("mdev %s interrupt leave, irq %d, sta %d",
			mdev->name, irq, irq_sta);

out:
	ipc_trace_hook_call_chain(priv->vc_id, IPC_IRQ_TOP, IPC_DIRECTION_OUT);

	return ret;
}

static void ipc_shutdown(struct xr_mbox_device *mdev)
{
	mutex_lock(&mdev->dev_lock);
	if (!--mdev->configured) {
		ipc_core_pr_dbg("%s shutdown", mdev->name);

		mdev->ops->free_irq(mdev);
		free_irq(mdev->cur_irq, mdev);

		kthread_stop(mdev->tx_kthread);
		mdev->tx_kthread = NULL;
		kfifo_free(&mdev->fifo);
		tasklet_kill(&mdev->rx_bh);
	}
	mutex_unlock(&mdev->dev_lock);
}

static void ipc_rx_fifo_init(struct xr_mbox_device *mdev)
{
	mdev->rx_fifo.count = 0;
	mdev->rx_fifo.read_index = 0;
	mdev->rx_fifo.write_index = 0;

	spin_lock_init(&mdev->rx_lock);
}

static int ipc_startup(struct xr_mbox_device *mdev)
{
	int ret = 0;
	unsigned int tx_buff;

	mutex_lock(&mdev->dev_lock);
	if (!mdev->configured++) {
		tx_buff = mdev->ops->get_fifo_size(mdev) * TX_FIFO_CELL_SIZE;
		ipc_core_pr_dbg("mdev fifo_size is %u", tx_buff);
		ret = kfifo_alloc(&mdev->fifo, tx_buff, GFP_KERNEL);

		if (ret) {
			ipc_core_pr_err("mdev %s alloc kfifo failed", mdev->name);
			ret = -ENOMEM;
			goto deconfig;
		}

		init_waitqueue_head(&mdev->tx_wait);
		/* create the async tx thread */
		mdev->tx_kthread = kthread_create(ipc_tx_thread,
						(void *)mdev, "%s", mdev->name);
		if (IS_ERR(mdev->tx_kthread)) {
			ipc_core_pr_err("create kthread tx_kthread failed!");
			ret = -EINVAL;
			goto deinit_work;
		} else {
			wake_up_process(mdev->tx_kthread);
		}

		tasklet_init(&mdev->rx_bh, ipc_rx_bh, (uintptr_t)mdev);
	}
	ipc_core_pr_dbg("mdev->configured = %d", mdev->configured);
	mutex_unlock(&mdev->dev_lock);
	return ret;

deinit_work:
	kfifo_free(&mdev->fifo);
deconfig:
	mdev->configured--;
	mutex_unlock(&mdev->dev_lock);
	return ret;
}

static void timeout_statistics(struct xr_mbox_device *mdev,
				u64 start_time, int mode)
{
	u64 send_time;
	u64 end_time;
	u64 ave_to;

	end_time = ktime_get_ns();
	send_time = end_time - start_time;

	switch (mode) {
	case SEND_SYNC:
		mdev->max_send_sync_time = (send_time > mdev->max_send_sync_time) ?
					send_time : mdev->max_send_sync_time;
		ave_to = (mdev->ave_send_sync_num * mdev->ave_send_sync_time + send_time)
			/ (mdev->ave_send_sync_num + 1);
		mdev->ave_send_sync_time = ave_to;
		mdev->ave_send_sync_num++;
		break;
	case SEND_RECV:
		mdev->max_send_recv_time = (send_time > mdev->max_send_recv_time) ?
				send_time : mdev->max_send_recv_time;
		ave_to = (mdev->ave_send_recv_num * mdev->ave_send_recv_time + send_time)
			/ (mdev->ave_send_recv_num + 1);
		mdev->ave_send_recv_time = ave_to;
		mdev->ave_send_recv_num++;
		break;
	default:
		ipc_core_pr_err("%s: get timeoutstatistics failed\n", mdev->name);
		break;
	}
}

int xr_ipc_send_recv(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
		unsigned int *rx_buffer, unsigned int rx_buffer_len)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_task tx_task;
	u64 start_time;
	int tx_len;
	int ret;

	start_time = ktime_get_ns();

	if (!tx_buffer) {
		ipc_core_pr_err("ipc-%d no tx_buffer", ipc_id);
		ret = -EINVAL;
		goto out;
	}

	if (!rx_buffer) {
		ipc_core_pr_err("ipc-%d no rx_buffer", ipc_id);
		ret = -EINVAL;
		goto out;
	}

	tx_len = MBOX_USED_REG_NUM(len);
	tx_task.tx_buffer_len = tx_len;

	ret = memcpy_s((void *)tx_task.tx_buffer, IPC_MBOX_B2B_DATA_SIZE,
			(void *)tx_buffer, len);
	if (ret != IPC_EOK) {
		ipc_core_pr_err("%s: memcpy_s failed", __func__);
		ret = -EINVAL;
		goto out;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		ret = -ENODEV;
		goto out;
	}

	/* MDEV_SEND_RECV start */
	ret = set_status(mdev, MDEV_SEND_RECV);
	if (ret) {
		ipc_core_pr_err("MSG{0x%08x, 0x%08x}", tx_task.tx_buffer[0],
				tx_task.tx_buffer[1]);
		goto out;
	}

	/* send */
	mutex_lock(&mdev->dev_lock);
	tx_task.rx_buffer_len = rx_buffer_len;
	tx_task.rx_buffer = rx_buffer;
	ret = ipc_task_send_recv(mdev, &tx_task);

	ipc_dbg_recv_msg_pr(mdev, rx_buffer);

	timeout_statistics(mdev, start_time, SEND_RECV);
	mutex_unlock(&mdev->dev_lock);

	/* MDEV_SEND_RECV end */
	clr_status(mdev, MDEV_SEND_RECV);

out:
	return ret;
}

int xr_ipc_send_sync(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_task tx_task;
	u64 start_time;
	int tx_len;
	int ret;

	start_time = ktime_get_ns();

	if (!tx_buffer) {
		ipc_core_pr_err("ipc-%d no tx_buffer", ipc_id);
		ret = -EINVAL;
		goto out;
	}

	tx_len = MBOX_USED_REG_NUM(len);
	tx_task.tx_buffer_len = tx_len;

	ret = memcpy_s((void *)tx_task.tx_buffer, IPC_MBOX_B2B_DATA_SIZE,
			(void *)tx_buffer, len);
	if (ret != IPC_EOK) {
		ipc_core_pr_err("%s: memcpy_s failed", __func__);
		ret = -EINVAL;
		goto out;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		ret = -ENODEV;
		goto out;
	}

	/* SYNC_SENDING start */
	ret = set_status(mdev, MDEV_SYNC_SENDING);
	if (ret) {
		ipc_core_pr_err("MSG{0x%08x, 0x%08x}", tx_task.tx_buffer[0],
				tx_task.tx_buffer[1]);
		goto out;
	}

	/* send */
	mutex_lock(&mdev->dev_lock);
	ret = ipc_task_send(mdev, &tx_task, IPC_TASK_SEND_MSG);
	timeout_statistics(mdev, start_time, SEND_SYNC);
	mutex_unlock(&mdev->dev_lock);

	/* SYNC_SENDING end */
	clr_status(mdev, MDEV_SYNC_SENDING);

out:
	return ret;
}

int xr_ipc_send_async(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
			ack_callback_t cb, void *arg)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_task *tx_task;
	unsigned long flags = 0;
	int tx_len;
	int ret;

	if (!tx_buffer) {
		ipc_core_pr_err("ipc-%d no tx_buffer", ipc_id);
		ret = -EINVAL;
		goto over;
	}

	tx_task = kcalloc(1, sizeof(*tx_task), GFP_ATOMIC);
	if (!tx_task) {
		ipc_core_pr_err("failed to alloc tx_task");
		ret = -ENOMEM;
		goto over;
	}

	tx_len = MBOX_USED_REG_NUM(len);
	tx_task->tx_buffer_len = tx_len;

	ret = memcpy_s((void *)tx_task->tx_buffer, IPC_MBOX_B2B_DATA_SIZE,
			(void *)tx_buffer, len);
	if (ret != IPC_EOK) {
		ipc_core_pr_err("%s: memcpy_s failed", __func__);
		goto out;
	}

	if ((cb != NULL) && (arg != NULL)) {
		tx_task->ack_cb = cb;
		tx_task->ack_arg = arg;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		ret = -ENODEV;
		goto out;
	}

	/* ASYNC_ENQUEUE start */
	ret = set_status(mdev, MDEV_ASYNC_ENQUEUE);
	if (ret) {
		ipc_core_pr_err("MSG{0x%08x, 0x%08x}",
				tx_task->tx_buffer[0], tx_task->tx_buffer[1]);
		goto out;
	}

	/* enqueue */
	spin_lock_irqsave(&mdev->fifo_lock, flags);
	if (kfifo_avail(&mdev->fifo) < TX_FIFO_CELL_SIZE) {
		spin_unlock_irqrestore(&mdev->fifo_lock, flags);
		ipc_core_pr_err("%s: no enough kfifo", __func__);
		ret = -ENOMEM;
		goto clearstatus;
	}

	ret = kfifo_in(&mdev->fifo, tx_task,
			TX_FIFO_CELL_SIZE) != TX_FIFO_CELL_SIZE;
	spin_unlock_irqrestore(&mdev->fifo_lock, flags);
	if (ret)
		ipc_core_pr_err("%s: no enough kfifo", __func__);

	atomic_inc(&mdev->async_send_num);

	wake_up_interruptible(&mdev->tx_wait);

clearstatus:
	/* ASYNC_ENQUEUE end */
	clr_status(mdev, MDEV_ASYNC_ENQUEUE);
out:
	kfree(tx_task);
over:
	return ret;
}

int xr_ipc_async_fifo_reset(unsigned int ipc_id)
{
	struct xr_mbox_device *mdev = NULL;

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return -ENODEV;
	}

	atomic_set(&mdev->async_reset_flag, IPC_ASYNC_SEND_OVER);

	return 0;
}

int xr_ipc_send_resp(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_task tx_task;
	int tx_len;
	int ret;

	if (!tx_buffer) {
		ipc_core_pr_err("ipc-%d no tx_buffer", ipc_id);
		ret = -EINVAL;
		goto out;
	}

	tx_len = MBOX_USED_REG_NUM(len);
	tx_task.tx_buffer_len = tx_len;

	ret = memcpy_s((void *)tx_task.tx_buffer, IPC_MBOX_B2B_DATA_SIZE,
			(void *)tx_buffer, len);
	if (ret != IPC_EOK) {
		ipc_core_pr_err("%s: memcpy_s failed", __func__);
		ret = -EINVAL;
		goto out;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		ret = -ENODEV;
		goto out;
	}

	/* RESP_SENDING start */
	ret = set_status(mdev, MDEV_RESP_SENDING);
	if (ret) {
		ipc_core_pr_err("MSG{0x%08x, 0x%08x}", tx_task.tx_buffer[0],
				tx_task.tx_buffer[1]);
		goto out;
	}

	/* send */
	mutex_lock(&mdev->dev_lock);
	ret = ipc_task_send(mdev, &tx_task, IPC_TASK_SEND_RESP);
	if (ret)
		ipc_core_pr_err("%s: send failed", __func__);

	mutex_unlock(&mdev->dev_lock);

	/* RESP_SENDING end */
	clr_status(mdev, MDEV_RESP_SENDING);

out:
	return ret;
}

int xr_ipc_recv_register(unsigned int ipc_id, struct notifier_block *nb)
{
	struct xr_mbox_device *mdev = NULL;

	if (!nb) {
		ipc_core_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return -ENODEV;
	}

	atomic_notifier_chain_register(&mdev->notifier, nb);

	return 0;
}

int xr_ipc_recv_unregister(unsigned int ipc_id, struct notifier_block *nb)
{
	struct xr_mbox_device *mdev = NULL;

	if (!nb) {
		ipc_core_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return -ENODEV;
	}

	atomic_notifier_chain_unregister(&mdev->notifier, nb);

	return 0;
}

int xr_ipc_fast_ack_recv_register(unsigned int ipc_id,
		fast_ack_callback_t cb, void *arg)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_device_priv *priv = NULL;

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return -ENODEV;
	}
	priv = mdev->priv;

	if (mdev->recv_cb) {
		ipc_core_pr_err("this vc_id(%d) has registered the callback", ipc_id);
		return -EINVAL;
	}

	mdev->recv_cb = cb;
	mdev->recv_arg = arg;

	return 0;
}

int xr_ipc_fast_ack_recv_unregister(unsigned int ipc_id)
{
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_device_priv *priv = NULL;

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return -ENODEV;
	}
	priv = mdev->priv;

	if (!mdev->recv_cb) {
		ipc_core_pr_err("this vc_id(%d) has not registered the callback", ipc_id);
		return -EINVAL;
	}

	mdev->recv_cb = NULL;
	mdev->recv_arg = NULL;

	return 0;
}

int xr_mdev_list_add(struct xr_mbox_device *mdev)
{
	if (!mdev) {
		ipc_core_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	list_add_tail(&mdev->node, &mdevices);

	return 0;
}

void xr_mbox_device_deactivate(struct xr_mbox_device **list)
{
	struct xr_mbox_device_priv *priv = NULL;
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_task tx_task;
	int need_wakeup;
	int ret;
	int i;

	if (!list) {
		ipc_core_pr_err("no mboxes registered");
		return;
	}

	for (i = 0; (mdev = list[i]); i++) {
		ret = set_status(mdev, MDEV_DEACTIVATED);
		if (ret)
			continue;
		mutex_lock(&mdev->dev_lock);
		if (!mdev->configured) {
			mutex_unlock(&mdev->dev_lock);
			continue;
		}

		ret = ipc_dequeue_task(mdev, &tx_task);
		while (!ret) {
			ret = ipc_task_send(mdev, &tx_task, IPC_TASK_SEND_MSG);
			ret = ipc_dequeue_task(mdev, &tx_task);
		}
		mutex_unlock(&mdev->dev_lock);

		priv = mdev->priv;
		need_wakeup = priv->idev->need_wakeup;
		if (need_wakeup == IPC_LPIS)
			enable_irq_wake(mdev->cur_irq);
	}
}

void xr_mbox_device_activate(struct xr_mbox_device **list)
{
	struct xr_mbox_device_priv *priv = NULL;
	struct xr_mbox_device *mdev = NULL;
	int need_wakeup;
	int i;

	if (!list) {
		ipc_core_pr_err("no mboxes registered");
		return;
	}

	for (i = 0; (mdev = list[i]); i++) {
		clr_status(mdev, MDEV_DEACTIVATED);

		priv = mdev->priv;
		need_wakeup = priv->idev->need_wakeup;
		if (need_wakeup == IPC_LPIS)
			disable_irq_wake(mdev->cur_irq);
	}
}

int xr_mbox_device_unregister(struct xr_mbox_device **list)
{
	struct xr_mbox_device *mdev = NULL;
	int i;

	if (!list) {
		ipc_core_pr_err("no mboxes registered");
		return -EINVAL;
	}

	ipc_trace_hook_exit();

	for (i = 0; (mdev = list[i]); i++) {
		ipc_shutdown(mdev);
		mutex_destroy(&mdev->dev_lock);
		list_del(&mdev->node);
	}

	return 0;
}

static int mbox_device_is_valid(struct xr_mbox_device *mdev)
{
	if (WARN_ON(!mdev->ops->send || !mdev->ops->resp ||
			!mdev->ops->ack || !mdev->ops->release ||
			!mdev->ops->recv || !mdev->ops->ensure_channel ||
			!mdev->ops->dump_status || !mdev->ops->get_fifo_size ||
			!mdev->ops->get_ack_timeout ||
			!mdev->ops->get_resp_timeout ||
			!mdev->ops->request_irq || !mdev->ops->free_irq ||
			!mdev->ops->enable_irq || !mdev->ops->disable_irq ||
			!mdev->ops->irq_sta || !mdev->ops->is_msg ||
			!mdev->ops->is_ack || !mdev->ops->clr_ack))
		return 0;

	return 1;
}

static int ipc_init(void)
{
	int ret;

	ret = ipc_trace_hook_init();

	return ret;
}

int xr_mbox_device_init(struct device *parent, struct xr_mbox_device **list)
{
	struct xr_mbox_device *mdev = NULL;
	int ret;
	int i;

	if (!parent) {
		ipc_core_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	if (!list) {
		ipc_core_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	ret = ipc_init();
	if (ret) {
		ipc_core_pr_err("ipc init IPC_CORE_PR_ERROR");
		goto out;
	}

	for (i = 0; (mdev = list[i]); i++) {
		if (!mbox_device_is_valid(mdev)) {
			ipc_core_pr_err("invalid mdev");
			ret = -EINVAL;
			goto err_out;
		}

		spin_lock_init(&mdev->fifo_lock);
		spin_lock_init(&mdev->status_lock);
		spin_lock_init(&mdev->ack_complete_lock);
		spin_lock_init(&mdev->resp_complete_lock);
		mutex_init(&mdev->dev_lock);

		init_completion(&mdev->ack_complete);
		init_completion(&mdev->resp_complete);

		atomic_set(&mdev->async_reset_flag, IPC_ASYNC_SEND_CONTINUE);

		atomic_set(&mdev->recv_num, 0);
		atomic_set(&mdev->async_ack_num, 0);
		atomic_set(&mdev->async_send_num, 0);
		atomic_set(&mdev->async_trigger_num, 0);

		mdev->status = 0;
		mdev->timeout_num = 0;

		ATOMIC_INIT_NOTIFIER_HEAD(&mdev->notifier);
		mdev->recv_cb = NULL;
		mdev->recv_arg = NULL;

		ipc_rx_fifo_init(mdev);

		ipc_startup(mdev);

		ret = request_irq(mdev->cur_irq, ipc_interrupt,
				0, mdev->name, (void *)mdev);
		if (ret) {
			ipc_core_pr_err("request irq %s failed, ret = %d",
				mdev->name, ret);
			goto err_out;
		}
	}

	return 0;

err_out:
	while (i--) {
		mdev = list[i];
		list_del(&mdev->node);
		mutex_destroy(&mdev->dev_lock);
	}
out:
	return ret;
}

void xr_ipc_dump_async_cnt(unsigned int ipc_id)
{
	struct xr_mbox_device *mdev = NULL;

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return;
	}

	ipc_core_pr_info("vc_id[%s] number of messages received = %d",
			mdev->name, atomic_read(&mdev->recv_num));
	ipc_core_pr_info("vc_id[%s] number of messages async_send = %d",
			mdev->name, atomic_read(&mdev->async_send_num));
	ipc_core_pr_info("vc_id[%s] number of messages async_ack = %d",
			mdev->name, atomic_read(&mdev->async_ack_num));
	ipc_core_pr_info("vc_id[%s] number of messages async_trigger = %d",
			mdev->name, atomic_read(&mdev->async_trigger_num));
}

void xr_ipc_reset_async_cnt(unsigned int ipc_id)
{
	struct xr_mbox_device *mdev = NULL;

	mdev = mbox_device_get(ipc_id);
	if (!mdev) {
		ipc_core_pr_err("%s: mbox_device_get failed", __func__);
		return;
	}

	atomic_set(&mdev->async_ack_num, 0);
	atomic_set(&mdev->async_send_num, 0);
	atomic_set(&mdev->async_trigger_num, 0);
}
