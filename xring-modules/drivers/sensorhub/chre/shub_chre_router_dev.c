// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <soc/xring/securelib/securec.h>

#define DEVICE_NAME "chre_router"

struct chre_buffer_list {
	struct list_head node;
	uint8_t *data;
	uint16_t len;
};

static struct chre_buffer_list g_buffer_list_head;
struct wait_queue_head wait_queue;
static spinlock_t chre_buffer_lock;

static int store_chre_msg_to_buffer(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	int ret = 0;
	uint8_t *data = NULL;
	uint16_t data_len = 0;
	struct ipc_shm *shm_pkt;
	struct chre_buffer_list *msg_buffer;
	uint8_t *buffer_data = NULL;
	uint8_t ipc_pkt_type = pkt->header.control.type;

	switch (ipc_pkt_type) {
	case IPC_SHORT_PKT:
	case IPC_LONG_PKT:
		data = pkt->data;
		data_len = pkt->header.data_len;
		break;
	case IPC_SHM_PKT:
		shm_pkt = sh_ipc_get_shm((struct ipc_shm_pkt *)pkt);
		data = shm_pkt->data;
		data_len = shm_pkt->data_len;
		break;
	default:
		pr_err("chre msg type=%d is invalid.", ipc_pkt_type);
		return -EINVAL;
	}

	buffer_data = kzalloc(sizeof(uint8_t) * data_len, GFP_ATOMIC);
	if (!buffer_data) {
		ret = -ENOMEM;
		pr_err("zalloc chre msg data buffer fail, ret=%d\n", ret);
		return ret;
	}

	memcpy_s((void *)buffer_data, data_len, (void *)data, data_len);

	msg_buffer = kzalloc(sizeof(struct chre_buffer_list), GFP_ATOMIC);
	if (!msg_buffer) {
		kfree(buffer_data);
		ret = -ENOMEM;
		pr_err("zalloc chre msg node buffer fail, ret=%d\n", ret);
		return ret;
	}

	msg_buffer->data = buffer_data;
	msg_buffer->len = data_len;

	list_add_tail(&msg_buffer->node, &g_buffer_list_head.node);

	wake_up_interruptible(&wait_queue);
	return 0;
}



static int chre_router_open(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}


static ssize_t chre_router_read(struct file *filp,
					char __user *buffer,
					size_t read_size, loff_t *pos)
{
	struct chre_buffer_list *first_chre_msg = NULL;
	int ret = 0;

	if (list_empty(&g_buffer_list_head.node)) {
		pr_err("chre_router read: err, no data");
		return -EINVAL;
	}

	first_chre_msg = list_first_entry(&g_buffer_list_head.node, struct chre_buffer_list, node);
	read_size = first_chre_msg->len;

	ret = copy_to_user((void *)buffer, (void *)first_chre_msg->data, first_chre_msg->len);
	if (ret != 0) {
		pr_err("chre_router read: copy fail, ret = %d", ret);
		return ret;
	}

	spin_lock_bh(&chre_buffer_lock);
	list_del(&(first_chre_msg->node));
	spin_unlock_bh(&chre_buffer_lock);
	kfree(first_chre_msg->data);
	kfree(first_chre_msg);
	return read_size;
}


static ssize_t chre_router_write(struct file *filp,
					const char __user *buffer,
					size_t write_size, loff_t *pos)
{
	int ret = 0;

	if (!is_shub_boot_done())
		return -EAGAIN;

	char *msg = kzalloc(sizeof(uint8_t) * write_size, GFP_KERNEL);

	if (!msg) {
		ret = -ENOMEM;
		pr_err("chre_router write: zalloc fail, ret=%d\n", ret);
		return ret;
	}

	ret = copy_from_user(msg, buffer, write_size);
	if (ret != 0) {
		pr_err("chre_router write: copy form user err, ret=%d", ret);
		goto out;
	}

	ret = sh_ipc_send_msg_sync(IPC_VC_AO_NS_SH_AP_CHRE, TAG_SH_CHRE, 0,
				(void *)msg, write_size);
	if (ret != 0) {
		pr_err("chre_router write: send err, ret=%d", ret);
		goto out;
	}

	ret = write_size;

out:
	kfree(msg);
	return ret;
}

static unsigned int chre_router_poll(struct file *fp, poll_table *wait)
{
	if (list_empty(&g_buffer_list_head.node)) {
		poll_wait(fp, &wait_queue, wait);
		return 0;
	}

	return POLLIN;
}


static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = chre_router_open,
	.read = chre_router_read,
	.write = chre_router_write,
	.poll = chre_router_poll,
};

static struct miscdevice chre_router_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};

static int __init chre_router_init(void)
{
	int ret = 0;
	struct ipc_notifier_info cb_info;

	spin_lock_init(&chre_buffer_lock);

	INIT_LIST_HEAD(&g_buffer_list_head.node);

	init_waitqueue_head(&wait_queue);

	cb_info.arg = NULL;
	cb_info.callback = store_chre_msg_to_buffer;
	ret = sh_register_recv_notifier(IPC_VC_AO_NS_SH_AP_CHRE, TAG_SH_CHRE, &cb_info);
	if (ret != 0) {
		pr_err("register store_chre_message_to_buffer fail, ret=%d\n", ret);
		return ret;
	}

	ret = misc_register(&chre_router_misc);
	if (!ret) {
		pr_info("chre router misc register success %d\n", ret);
		return 0;
	}

	return 0;

}
static void __exit chre_router_exit(void) {}
module_init(chre_router_init);
module_exit(chre_router_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("XRing Chre Router");
MODULE_LICENSE("GPL v2");
