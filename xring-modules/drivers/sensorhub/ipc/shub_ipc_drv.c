// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/list.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/genalloc.h>
#include <linux/mutex.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <soc/xring/sensorhub/shub_notifier.h>

#define IPC_SHM_BUSY				   1
#define IPC_SHM_FREE				   0

#define BYTES_PER_WORD				   4
#define BITS_PER_BYTE				   8
#define ALIGN4_MASK					   0x03ul

#define IPC_SHM_POOL_FREE_THRESHOLD	   (IPC_SHM_AP2SH_SIZE >> 1)
#define IPC_RECV_SHM_POOL_START_ADDR   ((uintptr_t)g_shmem_addr_base.recv_addr_base)
#define IPC_SEND_SHM_POOL_START_ADDR   ((uintptr_t)g_shmem_addr_base.send_addr_base)
#define IPC_SHM_POOL_MIN_ALLOC_ORDER   3
#define IPC_SHM_POOL_NID			   -1
#define IPC_SHM_NOTIFY_NUM			   3

struct ipc_busy_list_node {
	struct list_head node;
	uintptr_t shm;
	size_t shm_size;
};

struct shmem_addr_base {
	void __iomem *recv_addr_base;
	void __iomem *send_addr_base;
};

static struct shmem_addr_base g_shmem_addr_base;
static struct list_head g_busy_list_head;
static struct gen_pool *g_shm_pool;
static struct mutex g_ipcshm_lock;
static atomic_t g_sh_ok_status = ATOMIC_INIT(1);

static int sh_ipc_event_cb(struct notifier_block *nb, unsigned long action, void *data)
{
	switch (action) {
	case ACTION_SH_RESET:
		atomic_set_release(&g_sh_ok_status, 0);
		break;
	case ACTION_SH_RECOVERY_OS_READY:
		atomic_set_release(&g_sh_ok_status, 1);
		break;
	default:
		pr_err("%s unknown action[%lu]\n", __func__, action);
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block sh_ok_notifier = {
	.notifier_call = sh_ipc_event_cb,
};

static void shm_set_bf_to_free(uintptr_t shm)
{
	u32 ioval;

	ioval = readl((u32 *)shm);
	((struct ipc_shm *)(&ioval))->bf = IPC_SHM_FREE;
	writel(ioval, (u32 *)shm);
}

static bool shm_is_no_busy(struct ipc_busy_list_node *node)
{
	u32 ioval;
	struct ipc_shm *shm = (struct ipc_shm *)node->shm;

	ioval = readl(shm);
	return ((struct ipc_shm *)(&ioval))->bf == IPC_SHM_FREE;
}

static void shm_free_no_busy_blk(void)
{
	bool no_busy;
	struct ipc_busy_list_node *pos = NULL;
	struct ipc_busy_list_node *tmp = NULL;

	mutex_lock(&g_ipcshm_lock);
	list_for_each_entry_safe(pos, tmp, &g_busy_list_head, node) {
		no_busy = shm_is_no_busy(pos);
		if (no_busy) {
			gen_pool_free(g_shm_pool, (unsigned long)pos->shm, pos->shm_size);
			list_del(&pos->node);
			kfree(pos);
			pos = NULL;
		}
	}

	mutex_unlock(&g_ipcshm_lock);
}

static uintptr_t sh_ipc_shm_alloc(size_t size)
{
	uintptr_t shm;
	struct ipc_busy_list_node *busy_list_node = NULL;

	busy_list_node = kzalloc(sizeof(struct ipc_busy_list_node), GFP_KERNEL);
	if (!busy_list_node)
		return 0;

	shm_free_no_busy_blk();
	shm = (uintptr_t)gen_pool_alloc(g_shm_pool, size);
	if (shm == 0) {
		pr_err("shm alloc failed\n");
		goto free_busy_node;
	}

	busy_list_node->shm = shm;
	busy_list_node->shm_size = size;

	mutex_lock(&g_ipcshm_lock);
	list_add_tail(&busy_list_node->node, &g_busy_list_head);
	mutex_unlock(&g_ipcshm_lock);

	return shm;

free_busy_node:
	kfree(busy_list_node);
	busy_list_node = NULL;

	return 0;
}

static void sh_ipc_copy_data_to_shm(struct ipc_shm *shm, char *data, size_t data_len)
{
	shm->bf = IPC_SHM_BUSY;
	shm->magic = IPC_SHM_MAGIC;
	shm->data_len = data_len;

	memcpy(shm->data, data, data_len);
}

static int sh_ipc_check_pkt(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	u32 ioval;
	u32 shm_addr_offset;
	struct ipc_shm *shm_header = NULL;
	struct ipc_shm_pkt *shm_pkt = NULL;
	struct ipc_shm *shm = NULL;
	u8 pkt_type = pkt->header.control.type;

	if (pkt_type != IPC_SHM_PKT)
		return 0;

	shm_pkt = (struct ipc_shm_pkt *)pkt;
	shm_addr_offset = shm_pkt->shm_addr_offset;

	if (shm_addr_offset + shm_pkt->shm_len > AP_IPC_SHM_SH2AP_SIZE) {
		pr_err("shm_addr_offset:0x%x, shm_len:%d",
				shm_addr_offset, shm_pkt->shm_len);
		return -EINVAL;
	}

	shm = (struct ipc_shm *)(IPC_RECV_SHM_POOL_START_ADDR + shm_addr_offset);
	ioval = readl(shm);
	shm_header = (struct ipc_shm *)&ioval;
	if (shm_header->bf != IPC_SHM_BUSY || shm_header->magic != IPC_SHM_MAGIC) {
		pr_err("shm format error");
		return -EINVAL;
	}

	return 0;
}

static int sh_ipc_release_shm(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	u32 ioval;
	u8 pkt_type = pkt->header.control.type;
	struct ipc_shm *shm = NULL;
	struct ipc_shm_pkt *shm_pkt = (struct ipc_shm_pkt *)pkt;
	u32 shm_addr_offset = shm_pkt->shm_addr_offset;

	if (pkt_type != IPC_SHM_PKT)
		return 0;

	shm = (struct ipc_shm *)(IPC_RECV_SHM_POOL_START_ADDR + shm_addr_offset);
	ioval = readl(shm);
	((struct ipc_shm *)&ioval)->bf = IPC_SHM_FREE;
	writel(ioval, shm);

	return 0;
}

static int __sh_ipc_send_msg(bool sync, u8 vc_id, u16 tag, u8 cmd, void *msg,
	u16 msg_size, ack_callback_t cb, void *arg)
{
	int ret;
	u32 shm_offset_addr;
	u16 shm_len;
	uintptr_t shm;

	if ((msg == NULL && msg_size > 0) || msg_size > IPC_SHM_PKT_DATA_SIZE) {
		pr_err("msg size error");
		return -EMSGSIZE;
	}

	if (msg_size <= IPC_LONG_PKT_DATA_SIZE) {
		if (sync)
			ret = ap_ipc_send_sync(vc_id, tag, cmd, msg, msg_size);
		else
			ret = ap_ipc_send_async(vc_id, tag, cmd, msg, msg_size, cb, arg);
		return ret;
	}

	shm = sh_ipc_shm_alloc(msg_size + IPC_SHM_PKT_DATA_HEADER_SIZE);
	if (shm == 0) {
		pr_err("shm alloc failed");
		return -ENOMEM;
	}

	sh_ipc_copy_data_to_shm((struct ipc_shm *)shm, msg, msg_size);
	shm_offset_addr = (u32)(shm - IPC_SEND_SHM_POOL_START_ADDR);
	shm_len = msg_size + IPC_SHM_PKT_DATA_HEADER_SIZE;

	ret = ipc_send_shm_pkt(vc_id, tag, cmd, shm_offset_addr, shm_len);
	if (ret < 0)
		shm_set_bf_to_free(shm);

	return ret;
}

int sh_ipc_send_msg_sync(u8 vc_id, u16 tag, u8 cmd, void *msg, u16 msg_size)
{
	int ret;

	atomic_read_acquire(&g_sh_ok_status);
	if (unlikely(g_sh_ok_status.counter == 0)) {
		pr_err("shub is not ok, try again");
		return -EAGAIN;
	}

	ret = __sh_ipc_send_msg(true, vc_id, tag, cmd, msg, msg_size, NULL, NULL);
	return ret;
}
EXPORT_SYMBOL(sh_ipc_send_msg_sync);

int sh_ipc_send_msg_async(u8 vc_id, u16 tag, u8 cmd, void *msg, u16 msg_size,
	void (*complete)(int result, void *arg), void *arg)
{
	int ret;

	atomic_read_acquire(&g_sh_ok_status);
	if (unlikely(g_sh_ok_status.counter == 0)) {
		pr_err("shub is not ok, try again");
		return -EAGAIN;
	}

	ret = __sh_ipc_send_msg(false, vc_id, tag, cmd, msg, msg_size, complete, arg);
	return ret;
}
EXPORT_SYMBOL(sh_ipc_send_msg_async);

int sh_ipc_send_with_resp_msg(u8 vc_id, u16 tag, u8 cmd, void *tx_data,
			u16 tx_data_len, void *rx_buff, u16 rx_len)
{
	int ret;

	if ((tx_data == NULL && tx_data_len > 0) || tx_data_len > IPC_SHM_PKT_DATA_SIZE) {
		pr_err("msg size error");
		return -EMSGSIZE;
	}

	atomic_read_acquire(&g_sh_ok_status);
	if (unlikely(g_sh_ok_status.counter == 0)) {
		pr_err("shub is not ok, try again");
		return -EAGAIN;
	}

	ret = ap_ipc_send_recv(vc_id, tag, cmd, tx_data, tx_data_len, rx_buff, rx_len);
	return ret;
}
EXPORT_SYMBOL(sh_ipc_send_with_resp_msg);

struct ipc_shm *sh_ipc_get_shm(struct ipc_shm_pkt *shm_pkt)
{
	struct ipc_shm *shm = NULL;

	if (shm_pkt == NULL) {
		pr_err("invalid shm_pkt\n");
		return NULL;
	}

	shm = (struct ipc_shm *)(IPC_RECV_SHM_POOL_START_ADDR + shm_pkt->shm_addr_offset);
	if (shm->bf != IPC_SHM_BUSY || shm->magic != IPC_SHM_MAGIC) {
		pr_err("invalid shm\n");
		return NULL;
	}

	return shm;
}
EXPORT_SYMBOL(sh_ipc_get_shm);

int sh_register_recv_notifier(u8 vc_id, u16 tag, struct ipc_notifier_info *notify_info)
{
	int ret;

	if (notify_info == NULL) {
		pr_err("notify_info is NULL");
		return -EFAULT;
	}

	if (notify_info->callback == NULL) {
		pr_err("notify_info.callback is NULL");
		return -EINVAL;
	}

	ret = ap_ipc_recv_register(vc_id, tag, sh_ipc_check_pkt, NULL);
	if (ret < 0) {
		pr_err("sh_ipc_check_pkt callback register failed, ret:%d", ret);
		return ret;
	}

	ret = ap_ipc_recv_register(vc_id, tag, notify_info->callback, notify_info->arg);
	if (ret < 0) {
		pr_err("user's callback register failed, ret:%d", ret);
		goto unreg_check_pkt_cb;
	}

	ret = ap_ipc_recv_register(vc_id, tag, sh_ipc_release_shm, NULL);
	if (ret < 0) {
		pr_err("sh_ipc_release_shm callback register failed, ret:%d", ret);
		goto unreg_cb;
	}

	return 0;

unreg_cb:
	ap_ipc_recv_unregister(vc_id, tag, notify_info->callback);

unreg_check_pkt_cb:
	ap_ipc_recv_unregister(vc_id, tag, sh_ipc_check_pkt);

	return ret;
}
EXPORT_SYMBOL(sh_register_recv_notifier);

int sh_unregister_recv_notifier(u8 vc_id, u16 tag, struct ipc_notifier_info *notify_info)
{
	if (notify_info == NULL) {
		pr_err("notify_info is NULL");
		return -EFAULT;
	}

	if (notify_info->callback == NULL) {
		pr_err("notify_info.callback is NULL");
		return -EINVAL;
	}

	ap_ipc_recv_unregister(vc_id, tag, notify_info->callback);
	ap_ipc_recv_unregister(vc_id, tag, sh_ipc_check_pkt);
	ap_ipc_recv_unregister(vc_id, tag, sh_ipc_release_shm);

	return 0;
}
EXPORT_SYMBOL(sh_unregister_recv_notifier);

static int __init sh_ipc_protocol_drv_init(void)
{
	int ret;

	g_shmem_addr_base.send_addr_base = ioremap_wc((ssize_t)AP_IPC_SHM_AP2SH_ADDR,
		(unsigned long)AP_IPC_SHM_AP2SH_SIZE);
	if (IS_ERR_OR_NULL(g_shmem_addr_base.send_addr_base)) {
		pr_err("[%s] send_addr_base err\n", __func__);
		return -ENOMEM;
	}

	g_shmem_addr_base.recv_addr_base = ioremap_wc((ssize_t)AP_IPC_SHM_SH2AP_ADDR,
		(unsigned long)AP_IPC_SHM_SH2AP_SIZE);
	if (IS_ERR_OR_NULL(g_shmem_addr_base.recv_addr_base)) {
		pr_err("[%s] recv_addr_base err\n", __func__);
		ret = -ENOMEM;
		goto send_unmap;
	}

	g_shm_pool = gen_pool_create(IPC_SHM_POOL_MIN_ALLOC_ORDER, IPC_SHM_POOL_NID);
	if (IS_ERR_OR_NULL(g_shm_pool)) {
		pr_err("gen_pool_create failed\n");
		ret = -ENOMEM;
		goto recv_unmap;
	}

	ret = gen_pool_add(g_shm_pool, IPC_SEND_SHM_POOL_START_ADDR, AP_IPC_SHM_AP2SH_SIZE, -1);
	if (ret != 0) {
		ret = -ENOMEM;
		pr_err("gen_pool_add failed\n");
		goto destroy_pool;
	}

	mutex_init(&g_ipcshm_lock);
	INIT_LIST_HEAD(&g_busy_list_head);

	ret = sh_reset_notifier_register(&sh_ok_notifier);
	if (ret != 0) {
		pr_err("register sh_reset notifier failed\n");
		goto destroy_pool;
	}

	ret = sh_recovery_notifier_register(&sh_ok_notifier);
	if (ret != 0) {
		pr_err("register sh_recovery notifier failed\n");
		goto unreg_sh_reset;
	}

	pr_info("shub_ipc protocol drv init done.\n");
	return 0;

unreg_sh_reset:
	sh_reset_notifier_unregister(&sh_ok_notifier);
destroy_pool:
	gen_pool_destroy(g_shm_pool);
	g_shm_pool = NULL;
recv_unmap:
	iounmap(g_shmem_addr_base.recv_addr_base);
send_unmap:
	iounmap(g_shmem_addr_base.send_addr_base);
	pr_err("shub_ipc protocol drv init failed, ret:%d\n", ret);

	return ret;
}

static void __exit sh_ipc_protocol_drv_exit(void)
{
	sh_recovery_notifier_unregister(&sh_ok_notifier);
	sh_reset_notifier_unregister(&sh_ok_notifier);
	gen_pool_destroy(g_shm_pool);
	g_shm_pool = NULL;
	iounmap(g_shmem_addr_base.recv_addr_base);
	iounmap(g_shmem_addr_base.send_addr_base);
	g_shmem_addr_base.recv_addr_base = 0;
	g_shmem_addr_base.send_addr_base = 0;
}

module_init(sh_ipc_protocol_drv_init);
module_exit(sh_ipc_protocol_drv_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ipc protocol driver for sensorhub");
MODULE_AUTHOR("xiaomi.com");
