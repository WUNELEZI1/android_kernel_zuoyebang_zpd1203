// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)            "[shub_log]:" fmt

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/of.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <asm/page.h>
#include <net/sock.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dfx.h>
#include "shub_dfx_priv.h"

#define SH_DFX_CONTROL_INTEVAL (5000000000)

struct sh_dfx_ddr_dev {
	const char *dev_name;
	dev_t dev_no;
	struct cdev cdev;
	struct class *class;
	struct device *device;
};

static struct sh_dfx_ddr_dev *g_sh_dfx_ddr_dev;
static struct sock *g_sh_dfx_nl_sk;
static uint32_t g_sh_recv_nl_pid;
static uint32_t g_sh_savetimes;

static int sh_dfx_ddr_open(struct inode *inode, struct file *file)
{
	struct sh_dfx_ddr_dev *dev = container_of(inode->i_cdev, struct sh_dfx_ddr_dev, cdev);

	pr_info("call open\n");
	file->private_data = dev;
	return 0;
}

static int sh_dfx_ddr_release(struct inode *inode, struct file *file)
{
	pr_info("call release\n");
	file->private_data = NULL;
	return 0;
}

static int sh_dfx_ddr_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret = 0;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long ddr_phy_addr = (unsigned long)SHUB_NS_DDR_LOG_BUFFER_ADDR;
	unsigned long long pfn = ddr_phy_addr >> PAGE_SHIFT;

	pr_info("call mmap\n");
	if (size > SH_DFX_MAP_MAX_SIZE) {
		pr_err("mmap size=%#lx more than max=%#x error\n", size, SH_DFX_MAP_MAX_SIZE);
		return -EINVAL;
	}

	ret = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	if (ret < 0)
		pr_err("mmap fail, ret:%d.\n", ret);

	return ret;
}

static int sh_dfx_ddr_flush(struct file *file, fl_owner_t id)
{
	pr_info("call flush file_ptr=%pK, id_ptr=%pK\n", file, id);
	return 0;
}

static int sh_dfx_ddr_fsync(struct file *file, loff_t begin, loff_t end, int datasync)
{
	int ret = sh_log_notify_control(CMD_SH_DFX_LOGFLUSH);

	pr_info("call fsync ret=%d, file_ptr=%pK, data=%#x\n", ret, file, datasync);
	return 0;
}

static int sh_dfx_drv_msg_send(void *msg, uint16_t msg_len)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	uint32_t len = NLMSG_SPACE(msg_len);
	ssize_t ret;

	if (msg == NULL && msg_len > 0) {
		pr_err("msg_send input err\n");
		ret = -EINVAL;
		goto _err_input;
	}

	pr_info("msg_send msg_len:%u, total_len:%u\n", msg_len, len);

	skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (skb == NULL) {
		pr_err("msg_send nlmsg_new err\n");
		ret = -ENOMEM;
		goto _err_input;
	}

	nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, msg_len, 0);
	if (nlh == NULL) {
		ret = -ENOMEM;
		goto _err_nlput;
	}
	NETLINK_CB(skb).dst_group = 0;
	ret = memcpy_s(NLMSG_DATA(nlh), msg_len, msg, msg_len);
	if (ret != EOK) {
		pr_err("msg_send memcpy err ret:%zd\n", ret);
		ret = -ENOMEM;
		goto _err_nlput;
	}

	ret = netlink_unicast(g_sh_dfx_nl_sk, skb, g_sh_recv_nl_pid, MSG_DONTWAIT);
	if (ret < 0) {
		pr_err("msg_send failed ret:%zd, pid=%u.\n", ret, g_sh_recv_nl_pid);
		ret = -EFAULT;
		goto _err_input;
	}
	pr_info("msg_send len:%zd\n", ret);
	return 0;

_err_nlput:
	nlmsg_free(skb);
_err_input:
	return ret;
}

static void sh_dfx_drv_msg_recv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	char *msg_ptr;

	pr_info("msg_recv\n");
	if (!skb) {
		pr_err("msg_recv buf null\n");
		return;
	}
	pr_info("msg_recv len=%u, data_len=%u, size=%u\n", skb->len, skb->data_len, skb->truesize);

	nlh = nlmsg_hdr(skb);
	if (!nlh) {
		pr_err("msg_recv nlmsg_hdr err\n");
		return;
	}

	pr_info("msg_recv old_pid=%u\n", g_sh_recv_nl_pid);
	if (nlh->nlmsg_pid == 0)
		pr_warn("nlmsg_pid=0!\n");
	else
		g_sh_recv_nl_pid = nlh->nlmsg_pid;
	pr_info("msg_recv new_pid=%u\n", g_sh_recv_nl_pid);

	msg_ptr = NLMSG_DATA(nlh);
	if (msg_ptr != NULL)
		pr_info("msg_recv msg:%s\n", msg_ptr);
}

static const struct file_operations sh_dfx_ddr_op = {
	.open = sh_dfx_ddr_open,
	.release = sh_dfx_ddr_release,
	.mmap = sh_dfx_ddr_mmap,
	.flush = sh_dfx_ddr_flush,
	.fsync = sh_dfx_ddr_fsync,
};

struct netlink_kernel_cfg sh_dfx_cfg = {
	.input = sh_dfx_drv_msg_recv,
};

static int sh_log_netlink_init(void)
{
	g_sh_dfx_nl_sk = netlink_kernel_create(&init_net, SH_DFX_NETLINK_UNIT, &sh_dfx_cfg);
	if (!g_sh_dfx_nl_sk) {
		pr_err("netlink create err\n");
		return -ENOMEM;
	}
	pr_info("netlink create ok\n");
	return 0;
}

static void sh_log_netlink_exit(void)
{
	if (g_sh_dfx_nl_sk) {
		netlink_kernel_release(g_sh_dfx_nl_sk);
		g_sh_dfx_nl_sk = NULL;
		pr_info("netlink release ok\n");
	}
}

void sh_notify_hal_savelogs(enum sh_dfx_save_type type, char *path)
{
	char msg[SH_DFX_MSG_MAX_LEN] = {0};
	errno_t ret;
	size_t len = 0;

	g_sh_savetimes++;
	pr_info("notify hal savelogs, times=0x%X\n", g_sh_savetimes);

	if (type == SH_SAVE_TYPE_LOG) {
		snprintf(msg, SH_DFX_MSG_MAX_LEN, "%s", SH_DFX_MSG_HEAD_KERNEL_LOG);
		pr_info("nl log msg form=%s.\n", msg);
	} else {
		snprintf(msg, SH_DFX_MSG_MAX_LEN, "%s", SH_DFX_MSG_HEAD_KERNEL_ALL);

		if (path == NULL) {
			pr_err("get NULL path.\n");
			goto _exit;
		}
		len = strlen(path);
		if (len + strlen(SH_DFX_MSG_HEAD_KERNEL_ALL) >= SH_DFX_MSG_MAX_LEN) {
			pr_err("path=%s used len=%zu too long.\n", path, len);
			goto _exit;
		}
		ret = strcat_s(msg, SH_DFX_MSG_MAX_LEN, path);
		if (ret != EOK)
			pr_err("nl msg path strcat err=%d.\n", ret);

		pr_info("nl path msg form=%s.\n", msg);
	}

_exit:
	ret = sh_dfx_drv_msg_send(msg, strlen(msg) + 1);
	pr_info("call send_nl_msg ret=%d\n", ret);
}

int sh_log_notify_control(enum ipc_cmds_sh_ap_e dfx_cmd)
{
	int ret = 0;
	static ktime_t timestamp;

	if (ktime_get() > (timestamp + SH_DFX_CONTROL_INTEVAL)) {
		pr_info("sending log control ipc, cmd=%X\n", dfx_cmd);
		ret = sh_ipc_send_msg_sync(SH_DFX_AP_SH_VC_ID, TAG_SH_DFX, dfx_cmd, NULL, 0);
		if (ret < 0)
			pr_err("send log control ipc err=%d\n", ret);
		timestamp = ktime_get();
	} else
		pr_info("send log control ipc inteval nok, do nothing\n");

	return ret;
}
EXPORT_SYMBOL(sh_log_notify_control);

void sh_log_ddr_rd_ptr_init(void)
{
	uint64_t *ddr_rd_addr = (uint64_t *)ioremap(SHUB_NS_DDR_LOG_BUFFER_ADDR + SH_LOG_HEAD_WR_OFFSET, 8);

	if (ddr_rd_addr == NULL) {
		pr_err("ddr rd ioremap failed\n");
		return;
	}
	writeq(0, ddr_rd_addr);
	iounmap(ddr_rd_addr);
}
EXPORT_SYMBOL(sh_log_ddr_rd_ptr_init);

int sh_log_dev_init(void)
{
	int ret;

	g_sh_dfx_ddr_dev = kzalloc(sizeof(struct sh_dfx_ddr_dev), GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_sh_dfx_ddr_dev)) {
		pr_err("init kzalloc err\n");
		ret = -ENOMEM;
		goto _err_alloc;
	}

	g_sh_dfx_ddr_dev->dev_name = SH_DFX_NODE_DEV_NAME;
	ret = alloc_chrdev_region(&g_sh_dfx_ddr_dev->dev_no, 0, 1, g_sh_dfx_ddr_dev->dev_name);
	if (ret != 0) {
		pr_err("dev_alloc err, ret:%d\n", ret);
		goto _err_reg;
	}

	cdev_init(&g_sh_dfx_ddr_dev->cdev, &sh_dfx_ddr_op);
	g_sh_dfx_ddr_dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&g_sh_dfx_ddr_dev->cdev, g_sh_dfx_ddr_dev->dev_no, 1);
	if (ret != 0) {
		pr_err("dev_add err, ret:%d\n", ret);
		goto _err_add;
	}

	g_sh_dfx_ddr_dev->class = class_create("shub_class");
	if (IS_ERR_OR_NULL(g_sh_dfx_ddr_dev->class)) {
		pr_err("class_create err\n");
		ret = -EINVAL;
		goto _err_class;
	}

	g_sh_dfx_ddr_dev->device = device_create(g_sh_dfx_ddr_dev->class, NULL,
				g_sh_dfx_ddr_dev->dev_no, NULL, g_sh_dfx_ddr_dev->dev_name);
	if (IS_ERR_OR_NULL(g_sh_dfx_ddr_dev->device)) {
		pr_err("device_create err\n");
		ret = -EINVAL;
		goto _err_device;
	}

	sh_log_netlink_init();
	pr_info("init ok\n");
	ret = 0;
	goto _succ;

_err_device:
	class_destroy(g_sh_dfx_ddr_dev->class);
_err_class:
	cdev_del(&g_sh_dfx_ddr_dev->cdev);
_err_add:
	unregister_chrdev_region(g_sh_dfx_ddr_dev->dev_no, 1);
_err_reg:
	kfree(g_sh_dfx_ddr_dev);
	g_sh_dfx_ddr_dev = NULL;
_err_alloc:
	pr_err("init fail\n");
_succ:
	return ret;
}

void sh_log_dev_exit(void)
{
	device_destroy(g_sh_dfx_ddr_dev->class, g_sh_dfx_ddr_dev->dev_no);
	class_destroy(g_sh_dfx_ddr_dev->class);
	cdev_del(&g_sh_dfx_ddr_dev->cdev);
	unregister_chrdev_region(g_sh_dfx_ddr_dev->dev_no, 1);
	kfree(g_sh_dfx_ddr_dev);
	g_sh_dfx_ddr_dev = NULL;
	sh_log_netlink_exit();

	pr_info("dev exit\n");
}
