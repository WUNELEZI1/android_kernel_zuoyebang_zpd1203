// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/relay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/wait.h>
#ifdef CONFIG_SUPPORT_WWAN
#include <linux/wwan.h>
#endif

#include "mtk_port_io.h"
#ifdef CONFIG_TX00_UT_PORT
#include "ut_port_io_fake.h"
#endif

#define TAG			"PORT_IO"
#define MTK_CCCI_CLASS_NAME	"ccci"
#define MTK_DEVID_REGION_NAME	"ccci_port"
#define MTK_TTY_DRV_NAME	"ttyC"
#define MTK_DFLT_READ_TIMEOUT	(1 * HZ)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
#define class_create(owner, name)                              \
({                                                             \
       class_create(name);    \
})

#endif

static const struct file_operations char_dev_fops;
static void *ccci_class;
static int ccci_major;

static DEFINE_IDR(ccci_minors);
unsigned short brom_dl_flag;

static struct mtk_port *mtk_port_get_by_devid_locked(dev_t devt)
{
	struct mtk_port *port;

	mutex_lock(&port_mngr_grp_mtx);
	port = idr_find(&ccci_minors, MINOR(devt));
	if (!port) {
		pr_err_ratelimited("[PORT_IO][get_by_devid_locked] Port does not exist\n");
		goto exit_unlock;
	}

	/* Add port->kref so that port can still be working,
	 * even after user close the port or device are removed.
	 */
	kref_get(&port->kref);
exit_unlock:
	mutex_unlock(&port_mngr_grp_mtx);

	return port;
}

static int mtk_port_get_locked(struct mtk_port *port)
{
	int ret = 0;

	/* Protect the structure not released suddenly during the check */
	mutex_lock(&port_mngr_grp_mtx);
	if (!port) {
		mutex_unlock(&port_mngr_grp_mtx);
		pr_err_ratelimited("[PORT_IO][get_locked] Port does not exist\n");
		return -ENODEV;
	}
	kref_get(&port->kref);
	mutex_unlock(&port_mngr_grp_mtx);

	return ret;
}

/* After calling the mtk_port_put_locked(),
 * do not use the port pointer because the port structure might be freed.
 */
static void mtk_port_put_locked(struct mtk_port *port)
{
	mutex_lock(&port_mngr_grp_mtx);
	kref_put(&port->kref, mtk_port_release);
	mutex_unlock(&port_mngr_grp_mtx);
}

static int mtk_port_cdev_register(void)
{
	dev_t dev;
	int ret;

	ret = alloc_chrdev_region(&dev, MTK_CDEV_MINOR_BASE, MTK_CDEV_MAX_NUM,
				  MTK_DEVID_REGION_NAME);
	if (ret) {
		pr_err("Failed to register cdev, ret=%d\n", ret);
		return ret;
	}

	ccci_major = MAJOR(dev);

	return ret;
}

static void mtk_port_cdev_unregister(void)
{
	if (!ccci_major)
		return;
	unregister_chrdev_region(MKDEV(ccci_major, MTK_CDEV_MINOR_BASE), MTK_CDEV_MAX_NUM);
}

/**
 * mtk_port_io_init() - Function for initialize device driver.
 * Create ccci_class and register each type of device driver into kernel.
 *
 * Return:.
 * * 0:	success
 * * error value if initialization failed
 */
int mtk_port_io_init(void)
{
	int ret;

	ccci_class = class_create(THIS_MODULE, MTK_CCCI_CLASS_NAME);
	if (IS_ERR(ccci_class)) {
		ret = PTR_ERR(ccci_class);
		goto end;
	}

	ret = mtk_port_cdev_register();
	if (unlikely(ret))
		goto destroy_class;

	return ret;

destroy_class:
	class_destroy(ccci_class);
end:
	return ret;
}

/**
 * mtk_port_io_exit() - Function for delete device driver.
 * Unregister each type of device driver from kernel, and destroyccci_class.
 *
 * This function called at driver module exit.
 */
void mtk_port_io_exit(void)
{
	mtk_port_cdev_unregister();
	ccci_major = 0;
	class_destroy(ccci_class);
}

static void mtk_port_struct_init(struct mtk_port *port)
{
	port->tx_seq = 0;
	port->rx_seq = -1;
	clear_bit(PORT_S_ENABLE, &port->status);
	kref_init(&port->kref);
	skb_queue_head_init(&port->rx_skb_list);
	skb_queue_head_init(&port->rx_skb_dump_list);
	port->rx_buf_size = MTK_RX_BUF_SIZE;
	init_waitqueue_head(&port->trb_wq);
	init_waitqueue_head(&port->rx_wq);
	mutex_init(&port->read_buf_lock);
	mutex_init(&port->write_lock);
}

static int mtk_port_copy_data_from(void *to, union user_buf from, unsigned int len,
				   unsigned int offset, bool from_user_space)
{
	int ret = 0;

	if (from_user_space) {
		ret = copy_from_user(to, from.ubuf + offset, len);
		if (ret)
			ret = -EFAULT;
	} else {
		memcpy(to, from.kbuf + offset, len);
	}

	return ret;
}

static int mtk_port_copy_data_to(union user_buf to, void *from, unsigned int len,
				 unsigned int offset, bool to_user_space)
{
	int ret = 0;

	if (to_user_space) {
		ret = copy_to_user(to.ubuf + offset, from, len);
		if (ret)
			ret = -EFAULT;
	} else {
		memcpy(to.kbuf + offset, from, len);
	}

	return ret;
}

static int mtk_port_cdev_init(struct mtk_port *port)
{
	struct mtk_port_mngr *port_mngr;
	struct cdev *cdev;
	dev_t devt;
	int ret;

	mtk_port_struct_init(port);
	port->enable = false;

	port_mngr = port->port_mngr;
	port->minor = idr_alloc(&ccci_minors, port, MTK_CDEV_MINOR_BASE,
				MTK_CDEV_MAX_NUM - 1, GFP_KERNEL);

	cdev = cdev_alloc();
	if (!cdev) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to alloc port(%s) cdev\n",
			port->info.name);
		return -ENOMEM;
	}
	cdev->ops = &char_dev_fops;
	cdev->owner = THIS_MODULE;
	port->priv.c_priv.cdev = cdev;
	devt = MKDEV(ccci_major, port->minor);
	ret = cdev_add(cdev, devt, 1);
	if (ret) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to create port(%s) cdev, ret=%d\n",
			port->info.name, ret);
		kobject_put(&cdev->kobj);
	}

	return ret;
}

static int mtk_port_cdev_exit(struct mtk_port *port)
{
	struct cdev *cdev = NULL;

	if (test_bit(PORT_S_ENABLE, &port->status))
		ports_ops[port->info.type]->disable(port);

	cdev = port->priv.c_priv.cdev;
	cdev_del(cdev);
	skb_queue_purge(&port->rx_skb_list);
	idr_remove(&ccci_minors, port->minor);

	return 0;
}

static int mtk_port_internal_init(struct mtk_port *port)
{
	mtk_port_struct_init(port);
	port->enable = false;

	return 0;
}

static int mtk_port_internal_exit(struct mtk_port *port)
{
	if (test_bit(PORT_S_ENABLE, &port->status))
		ports_ops[port->info.type]->disable(port);

	return 0;
}

static int mtk_port_proprietary_init(struct mtk_port *port)
{
	mtk_port_struct_init(port);
	port->minor = idr_alloc(&ccci_minors, port, MTK_CDEV_MINOR_BASE,
				MTK_CDEV_MAX_NUM - 1, GFP_KERNEL);

	return 0;
}

static int mtk_port_proprietary_exit(struct mtk_port *port)
{
	if (test_bit(PORT_S_ENABLE, &port->status))
		ports_ops[port->info.type]->disable(port);

	/* Use lock to keep skb list from user read*/
	skb_queue_purge(&port->rx_skb_list);

	idr_remove(&ccci_minors, port->minor);

	return 0;
}

static int mtk_port_reset(struct mtk_port *port)
{
	port->tx_seq = 0;
	port->rx_seq = -1;

	return 0;
}

static void mtk_port_dump(struct mtk_port *port)
{
	MTK_INFO(port->port_mngr->ctrl_blk->mdev,
		 "Dump info for port: dev_str:%s; port name:%s;\n",
		 port->port_mngr->ctrl_blk->mdev->dev_str, port->info.name);
	MTK_INFO(port->port_mngr->ctrl_blk->mdev,
		 "Tx_seq:%d; port tx_err_cnt:%d; port rx_seq:%d; port rx_err_cnt:%d\n",
		 port->tx_seq, port->tx_err_cnt, port->rx_seq, port->rx_err_cnt);
	MTK_INFO(port->port_mngr->ctrl_blk->mdev,
		 "Rx_data_len:%d;\n", port->rx_data_len);

	/* Dump rx data header */
	mtk_port_rx_skb_dump(port);
}

static int mtk_port_cdev_enable(struct mtk_port *port)
{
	char full_name[MTK_DFLT_FULL_NAME_LEN];
	struct mtk_port_mngr *port_mngr;
	struct device *dev;
	dev_t dev_n;
	int ret;

	if (test_bit(PORT_S_ENABLE, &port->status))
		goto end;

	ret = mtk_port_vq_enable(port);
	if (ret && ret != -EBUSY)
		goto exit_error;

	port_mngr = port->port_mngr;
	/* Port created in /dev/wwanD(id)(name)
	 * example: /dev/wwanD0port
	 */
	snprintf(full_name, MTK_DFLT_FULL_NAME_LEN, "%s%d%s",
		 MTK_PORT_NAME_HDR, port_mngr->dev_id, port->info.name);

	dev_n = port->priv.c_priv.cdev->dev;
	dev = device_create(ccci_class, port_mngr->ctrl_blk->mdev->dev,
			    dev_n, port, "%s", full_name);

	if (IS_ERR(dev)) {
		ret = PTR_ERR(dev);
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to create cdev port(%s) device, ret=%d\n",
			port->info.name, ret);
		goto exit_error;
	}

	set_bit(PORT_S_RDWR, &port->status);
	set_bit(PORT_S_ENABLE, &port->status);

end:
	return 0;
exit_error:
	return ret;
}

static int mtk_port_cdev_disable(struct mtk_port *port)
{
	struct cdev *cdev;
	dev_t dev_n;

	if (!test_and_clear_bit(PORT_S_ENABLE, &port->status))
		return 0;

	clear_bit(PORT_S_RDWR, &port->status);
	cdev = port->priv.c_priv.cdev;
	dev_n = cdev->dev;
	device_destroy(ccci_class, dev_n);

	mtk_port_vq_disable(port);

	return 0;
}

static int mtk_port_internal_enable(struct mtk_port *port)
{
	int ret;

	if (test_bit(PORT_S_ENABLE, &port->status))
		return 0;

	ret = mtk_port_vq_enable(port);
	if (ret && ret != -EBUSY)
		return ret;

	set_bit(PORT_S_RDWR, &port->status);
	set_bit(PORT_S_ENABLE, &port->status);

	return 0;
}

static int mtk_port_internal_disable(struct mtk_port *port)
{
	if (!test_and_clear_bit(PORT_S_ENABLE, &port->status))
		return 0;

	clear_bit(PORT_S_RDWR, &port->status);
	mtk_port_vq_disable(port);

	return 0;
}

static int mtk_port_proprietary_enable(struct mtk_port *port)
{
	int ret;

	if (test_bit(PORT_S_ENABLE, &port->status))
		return 0;

	ret = mtk_port_vq_enable(port);
	if (ret && ret != -EBUSY)
		return ret;

	set_bit(PORT_S_RDWR, &port->status);
	set_bit(PORT_S_ENABLE, &port->status);

	return 0;
}

static int mtk_port_proprietary_disable(struct mtk_port *port)
{
	if (!test_and_clear_bit(PORT_S_ENABLE, &port->status))
		return 0;

	clear_bit(PORT_S_RDWR, &port->status);
	mtk_port_vq_disable(port);

	return 0;
}

static int mtk_port_recv(struct mtk_port *port, struct sk_buff *skb)
{
	unsigned long flags;
	int ret;

start_recv:
	if (port->port_mngr->in_suspend)
		return -EPIPE;

	if (!test_bit(PORT_S_OPEN, &port->status) &&
	    !(port->info.flags & PORT_F_FORCE_HOLD_DATA)) {
		/* If current port is not opened by any user,
		 * the received data will be dropped
		 */
		ret = -ENXIO;
		goto drop_data;
	}

	if (skb->len + port->rx_data_len > port->rx_buf_size) {
		/* If the rx buffer is not enough, the data will be dropped */
		if (port->info.flags & PORT_F_ALLOW_DROP ||
		    !test_bit(PORT_S_ENABLE, &port->status) ||
		    !test_bit(PORT_S_RDWR, &port->status)) {
			ret = -ENOMEM;
			goto drop_data;
		} else {
			/* Avoid occupying the CPU long time when blocking */
			usleep_range(1000, 2000);
			goto start_recv;
		}
	}

	spin_lock_irqsave(&port->rx_skb_list.lock, flags);
	__skb_queue_tail(&port->rx_skb_list, skb);
	port->rx_data_len += skb->len;
	spin_unlock_irqrestore(&port->rx_skb_list.lock, flags);

	wake_up_interruptible_all(&port->rx_wq);

	return 0;

drop_data:
	port->rx_err_cnt++;
	mtk_port_free_rx_skb(port, skb);
	return ret;
}

static int mtk_port_internal_recv(struct mtk_port *port, struct sk_buff *skb)
{
	struct mtk_internal_port *priv;
	int ret = -ENXIO;

	if (!test_bit(PORT_S_OPEN, &port->status))
		goto drop_data;

	priv = &port->priv.i_priv;
	if (!priv->recv_cb || !priv->arg) {
		MTK_WARN_RATELIMITED(port->port_mngr->ctrl_blk->mdev,
				     "Invalid (%s) recv_cb, drop packet\n", port->info.name);
		goto drop_data;
	}

	ret = priv->recv_cb(priv->arg, skb);
	return ret;

drop_data:
	port->rx_err_cnt++;
	mtk_port_free_rx_skb(port, skb);
	return ret;
}

static int mtk_port_common_open(struct mtk_port *port)
{
	int ret = 0;

	if (!test_bit(PORT_S_ENABLE, &port->status)) {
		ret = -ENODEV;
		goto end;
	}

	if (test_bit(PORT_S_OPEN, &port->status)) {
		ret = -EBUSY;
		goto end;
	}

	MTK_INFO(port->port_mngr->ctrl_blk->mdev, "Open port %s\n", port->info.name);

	if (!(port->info.flags & PORT_F_FORCE_HOLD_DATA)) {
		/* Empty the RX skb list, and ensure there is no skb left */
		skb_queue_purge(&port->rx_skb_list);
		port->rx_data_len = 0;
	}

	set_bit(PORT_S_OPEN, &port->status);
	clear_bit(PORT_S_FLUSH, &port->status);
end:
	return ret;
}

static void mtk_port_common_close(struct mtk_port *port)
{
	pr_err("[PORT_IO][common_close] Close port %s\n", port->info.name);

	clear_bit(PORT_S_OPEN, &port->status);

	skb_queue_purge(&port->rx_skb_list);
	port->rx_data_len = 0;

	set_bit(PORT_S_FLUSH, &port->status);
	wake_up_interruptible_all(&port->rx_wq);
}

static int mtk_port_common_write(struct mtk_port *port, union user_buf buf, unsigned int len,
				 bool from_user_space)
{
	unsigned int tx_cnt, left_cnt = len;
	struct sk_buff *skb;
	int ret;
	if (len == 0)
		return 0;
start_write:
	ret = mtk_port_status_check(port);
	if (ret)
		goto end_write;

	skb = mtk_port_alloc_tx_skb(port);
	if (!skb) {
		MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"Failed to alloc skb for port(%s)\n", port->info.name);
		ret = -ENOMEM;
		goto end_write;
	}

	if (!(port->info.flags & PORT_F_RAW_DATA)) {
		/* Reserve enough buf len for ccci header */
		skb_reserve(skb, sizeof(struct mtk_ccci_header));
	}

	tx_cnt = min(left_cnt, port->tx_mtu);
	ret = mtk_port_copy_data_from(skb_put(skb, tx_cnt), buf, tx_cnt, len - left_cnt,
				      from_user_space);
	if (ret) {
		MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"Failed to copy data for port(%s)\n", port->info.name);
		mtk_port_free_tx_skb(port, skb);
		goto end_write;
	}

	ret = mtk_port_send_data(port, skb);
	if (ret < 0) {
		if (ret == -EINTR) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"PORT_IO : (%s) [common_write] got -EINTR\n", port->info.name);
			left_cnt -= tx_cnt;
		}
		goto end_write;
	}

	left_cnt -= ret;
	if (left_cnt)
		goto start_write;
end_write:
	return (len > left_cnt) ? (len - left_cnt) : ret;
}

static int mtk_port_common_read(struct mtk_port *port, union user_buf buf, unsigned int len,
				bool to_user_space)
{
	struct sk_buff_head *skb_list;
	unsigned int left_len = len;
	unsigned int total_read = 0;
	unsigned int read_len;
	struct sk_buff *skb;
	unsigned long flags;
	int merge_cnt = 0;
	int ret;

	if (len == 0)
		return 0;
	/* Protect read operation for multi-thread */
	mutex_lock(&port->read_buf_lock);

check_perm:
	ret = mtk_port_status_check(port);
	if (ret)
		goto end_read;

/* If check pass and have read part of data, the read thread returns only when skb_list is empty
 * or read enough data. Port kref protects port not released.
 */
start_read:
	skb_list = &port->rx_skb_list;
	/* Dequeue a skb from skb_list. Get NULL if skb_list is empty */
	skb = skb_dequeue(skb_list);
	if (!skb) {
		/* No data in rx buffer right now */
		if (!total_read && port->info.flags & PORT_F_BLOCKING) {
			/*Wait if no data to read in blocking mode*/
			ret = wait_event_interruptible_timeout(port->rx_wq,
							       !skb_queue_empty(skb_list) ||
							       test_bit(PORT_S_FLUSH,
									&port->status),
							       MTK_DFLT_READ_TIMEOUT);
			if (ret == -ERESTARTSYS) {
				ret = -EINTR;
				goto end_read;
			} else {
				/* If timeout or wakeup by signal, check permission again */
				goto check_perm;
			}
		} else if (!total_read && !(port->info.flags & PORT_F_BLOCKING)) {
			ret = -EAGAIN;
			goto end_read;
		} else {
			/* return even it is less than the required length
			 * in both blocking or non-blocking mode
			 */
			ret = total_read;
			goto end_print_trace;
		}
	}

	if (port->info.flags & PORT_F_FORCE_NO_SPILT_PACKET) {
		if (left_len < skb->len) {
			total_read = 0;
			goto end_read;
		} else {
			left_len = skb->len;
		}
	}
	read_len = min(skb->len, left_len);

	ret = mtk_port_copy_data_to(buf, skb->data, read_len, total_read, to_user_space);
	if (ret) {
		MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"Failed to copy data for port(%s)\n", port->info.name);
		goto end_read;
	}

	skb_pull(skb, read_len);

	/* Check if skb empty:free skb to pool or queue back to the rx skb_list */
	if (!skb->len)
		mtk_port_free_rx_skb(port, skb);
	else
		skb_queue_head(skb_list, skb);

	/* Use the skb_list lock to protect the SKB enqueue/dequeue
	 * as well as the read buffer list size.
	 */
	spin_lock_irqsave(&skb_list->lock, flags);
	port->rx_data_len -= read_len;
	spin_unlock_irqrestore(&skb_list->lock, flags);

	/* update read length for next cycle or end read */
	left_len -= read_len;
	total_read += read_len;
	if (left_len) {
		merge_cnt++;
		goto start_read;
	}

end_print_trace:
	MTK_DBG(port->port_mngr->ctrl_blk->mdev, MTK_DBG_CTRL_RX, MTK_MEMLOG_RG_10,
		"Port(%s) read %d bytes, merge_cnt: %d, left %d bytes\n",
		port->info.name, total_read, merge_cnt, port->rx_data_len);
end_read:
	mutex_unlock(&port->read_buf_lock);

	return ret ? ret : total_read;
}

static int mtk_port_common_ioctl(struct mtk_port *port, unsigned int cmd,
				 union user_buf buf, bool user_buf)
{
	unsigned int set_buf_size;
	int ret;

	ret = mtk_port_status_check(port);
	if (ret)
		goto end_ioctl;

	MTK_INFO(port->port_mngr->ctrl_blk->mdev, "Ioctl cmd: 0x%x", cmd);

	switch (cmd) {
	case TCFLSH:
		break;

	case TCGETS:
		ret = mtk_port_copy_data_to(buf, &port->priv.c_priv.termios,
					    sizeof(struct termios), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_to_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}

		break;
	case TCSETS:
	case TCSETSW:
	case TCSETSF:
		ret = mtk_port_copy_data_from(&port->priv.c_priv.termios, buf,
					      sizeof(struct termios), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_from_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}
		break;

#ifdef TCGETS2
	case TCGETS2:
		ret = mtk_port_copy_data_to(buf, &port->priv.c_priv.termios,
					    sizeof(struct termios), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_to_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}
		break;

	case TCSETS2:
	case TCSETSW2:
	case TCSETSF2:
		ret = mtk_port_copy_data_from(&port->priv.c_priv.termios, buf,
					      sizeof(struct termios), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_from_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}
		break;
#endif

	case TIOCMGET:
		ret = mtk_port_copy_data_to(buf, &port->priv.c_priv.mdmbits,
					    sizeof(int), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_to_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}
		break;

	case TIOCMSET:
	case TIOCMBIC:
	case TIOCMBIS: {
		int mdmbits;

		ret = mtk_port_copy_data_from(&mdmbits, buf, sizeof(int), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"Failed to copy data (%s) , copy_from_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
			break;
		}

		if (cmd == TIOCMBIC)
			port->priv.c_priv.mdmbits &= ~mdmbits;
		else if (cmd == TIOCMBIS)
			port->priv.c_priv.mdmbits |= mdmbits;
		else
			port->priv.c_priv.mdmbits = mdmbits;
		break;
	}

	case MTK_IOCTL_WDM_MAX_COMMAND:
		ret = mtk_port_copy_data_to(buf, &port->tx_mtu,
					    sizeof(port->tx_mtu), 0, user_buf);
		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"(IOCTL_WDM_MAX_COMMAND)Failed to copy data (%s) , copy_to_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}

		break;
	case MTK_IOC_SET_RX_BUF_SIZE:
		ret = mtk_port_copy_data_from(&set_buf_size, buf,
					      sizeof(set_buf_size), 0, user_buf);
		if (ret < 0) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"(MTK_IOC_SET_RX_BUF_SIZE)(%s)Failed to copy data from user, ret=%d",
				port->info.name, ret);
			break;
		}
		MTK_INFO(port->port_mngr->ctrl_blk->mdev,
			 "Port(%s) user want to set new rx buffer size(%d), the old size is %d\n",
			 port->info.name, set_buf_size, port->rx_buf_size);
		if (port->rx_buf_size >= set_buf_size) {
			ret = 0;
			break;
		}

		if (set_buf_size > MTK_RX_BUF_MAX_SIZE) {
			MTK_WARN(port->port_mngr->ctrl_blk->mdev,
				 "(MTK_IOC_SET_RX_BUF_SIZE)(%s)Failed to set buffer size(%d): Max buffer is %dB",
				 port->info.name, set_buf_size, MTK_RX_BUF_MAX_SIZE);
			ret = -EBUSY;
			break;
		}

		port->rx_buf_size = set_buf_size;

		if (ret) {
			MTK_ERR(port->port_mngr->ctrl_blk->mdev,
				"(MTK_IOC_SET_RX_BUF_SIZE)Failed to copy data (%s) , copy_from_user ret=%d",
				port->info.name, ret);
			ret = -EFAULT;
		}

		break;
	case MTK_IOC_ALLOW_DROP_PACKET:
		port->info.flags |= PORT_F_ALLOW_DROP;
		MTK_INFO(port->port_mngr->ctrl_blk->mdev, "Port(%s) allow to drop packet",
			 port->info.name);
		break;
	case MTK_IOC_FORBID_DROP_PACKET:
		port->info.flags &= ~PORT_F_ALLOW_DROP;
		MTK_INFO(port->port_mngr->ctrl_blk->mdev, "Port(%s) forbid to drop packet",
			 port->info.name);
		break;
	default:
		ret = -ENOIOCTLCMD;
	}

end_ioctl:

	return ret;
}

static int mtk_port_cdev_open(struct inode *inode, struct file *file)
{
	struct mtk_port *port;
	int ret;

	port = mtk_port_get_by_devid_locked(inode->i_cdev->dev);
	if (!port) {
		ret = -ENODEV;
		goto end;
	}

	ret = mtk_port_common_open(port);
	if (ret) {
		mtk_port_put_locked(port);
		goto end;
	}

	/* Brom port blocking mode should follow fsm state */
	/* This is used for download port to set speed, avoid
	 * data sequence abnormal, don't delete
	 */
	if (port->info.rx_ch != BROM_INDEX_RX) {
		if (file->f_flags & O_NONBLOCK)
			port->info.flags &= ~PORT_F_BLOCKING;
		else
			port->info.flags |= PORT_F_BLOCKING;
	}

	file->private_data = port;

	stream_open(inode, file);
end:
	return ret;
}

static int mtk_port_cdev_close(struct inode *inode, struct file *file)
{
	struct mtk_port *port = file->private_data;

	mtk_port_common_close(port);
	mtk_port_put_locked(port);
	return 0;
}

static ssize_t mtk_port_cdev_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *ppos)
{
	struct mtk_port *port = file->private_data;
	union user_buf user_buf;

	if (port->info.rx_ch != BROM_INDEX_RX) {
		if (file->f_flags & O_NONBLOCK)
			port->info.flags &= ~PORT_F_BLOCKING;
		else
			port->info.flags |= PORT_F_BLOCKING;
	}
	user_buf.ubuf = (void __user *)buf;
	return mtk_port_common_write(port, user_buf, (unsigned int)count, true);
}

static ssize_t mtk_port_cdev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct mtk_port *port = file->private_data;
	union user_buf user_buf;

	if (port->info.rx_ch != BROM_INDEX_RX) {
		if (file->f_flags & O_NONBLOCK)
			port->info.flags &= ~PORT_F_BLOCKING;
		else
			port->info.flags |= PORT_F_BLOCKING;
	}
	user_buf.ubuf = (void __user *)buf;
	return mtk_port_common_read(port, user_buf, (unsigned int)count, true);
}

static long mtk_port_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mtk_port *port = file->private_data;
	union user_buf user_buf;

	user_buf.ubuf = (void __user *)arg;
	return mtk_port_common_ioctl(port, cmd, user_buf, true);
}

#ifdef CONFIG_COMPAT
static long mtk_port_cdev_compat_ioctl(struct file *file, unsigned int cmd,
				       unsigned long arg)
{
	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;

	return file->f_op->unlocked_ioctl(file, cmd,
			(unsigned long)compat_ptr(arg));
}
#endif

static __poll_t mtk_port_cdev_poll(struct file *file, struct poll_table_struct *poll)
{
	struct mtk_port *port = file->private_data;
	struct mtk_ctrl_blk *ctrl_blk;
	__poll_t mask = 0;

	poll_wait(file, &port->trb_wq, poll);
	poll_wait(file, &port->rx_wq, poll);

	if (mtk_port_status_check(port))
		goto end_poll;

	ctrl_blk = port->port_mngr->ctrl_blk;

	if (!VQ_LIST_FULL(ctrl_blk->trans, port->info.vq_id))
		mask |= EPOLLOUT | EPOLLWRNORM;

	if (!skb_queue_empty(&port->rx_skb_list))
		mask |= EPOLLIN | EPOLLRDNORM;

end_poll:
	return mask;
}

/**
 * mtk_port_internal_open() - Function for open internal port.
 * @mdev: pointer to mtk_md_dev.
 * @name: the name of port will be opened.
 * @flag: optional operation type.
 *
 * This function called by FSM. Used to open interal port MDCTRL/SAPCTRL,
 * when need to transer some control message.
 *
 * Return:
 * * mtk_port structure if success.
 * * error valude if fail.
 */
void *mtk_port_internal_open(struct mtk_md_dev *mdev, char *name, int flag)
{
	struct mtk_port_mngr *port_mngr;
	struct mtk_ctrl_blk *ctrl_blk;
	struct mtk_port *port;
	int ret;

	ctrl_blk = mdev->ctrl_blk;
	port_mngr = ctrl_blk->port_mngr;

	port = mtk_port_search_by_name(port_mngr, name);
	if (port && port->info.type != PORT_TYPE_INTERNAL) {
		port = NULL;
		goto end;
	}

	ret = mtk_port_get_locked(port);
	if (ret)
		goto end;

	ret = mtk_port_common_open(port);
	if (ret) {
		mtk_port_put_locked(port);
		goto end;
	}

	if (flag & O_NONBLOCK)
		port->info.flags &= ~PORT_F_BLOCKING;
	else
		port->info.flags |= PORT_F_BLOCKING;
end:
	return port;
}

/**
 * mtk_port_internal_close() - Function for close internal port.
 * @i_port: which port need close.
 *
 * This function called by FSM. Used to close interal port MDCTRL/SAPCTRL.
 *
 * Return:
 * * 0:		success.
 * * -EINVAL:	port is NULL.
 * * -EBADF:	port is not opened.
 */
int mtk_port_internal_close(void *i_port)
{
	struct mtk_port *port = i_port;
	int ret = 0;

	if (!port) {
		ret = -EINVAL;
		goto end;
	}

	/* Avoid close port twice */
	if (!test_bit(PORT_S_OPEN, &port->status)) {
		pr_err("[PORT_IO][internal_close] (%s) has been closed\n", port->info.name);
		ret = -EBADF;
		goto end;
	}

	mtk_port_common_close(port);
	mtk_port_put_locked(port);
end:
	return ret;
}

/**
 * mtk_port_internal_write() - Function for writing interal data.
 * @i_port: pointer to mtk_port, indicate channel for sending data.
 * @skb:    inlude the data to be sent.
 *
 * This function called by FSM. Used to write control message through
 * interal port MDCTRL/SAPCTRL, example of handshake message.
 *
 * Return:
 * * actual sent data length if success.
 * * error value if send failed.
 */
int mtk_port_internal_write(void *i_port, struct sk_buff *skb)
{
	struct mtk_port *port = i_port;

	if (!port || !skb) {
		pr_err_ratelimited("[PORT_IO][internal_write] Input value error\n");
		return -EINVAL;
	}

	return mtk_port_send_data(port, skb);
}

long mtk_port_internal_ioctl(void *i_port, unsigned int cmd, unsigned long arg)
{
	struct mtk_port *port = i_port;
	union user_buf user_buf;

	if (!port) {
		pr_err_ratelimited("[PORT_IO][internal_ioctl] Input value error\n");
		return -EINVAL;
	}

	user_buf.kbuf = (void *)arg;
	return mtk_port_common_ioctl(port, cmd, user_buf, false);
}

/**
 * mtk_port_internal_recv_register() - Function for register receive callback.
 * @i_port: pointer to mtk_port, indicate channel for receiving data.
 * @cb:     callback for receiving data.
 * @arg:    user data which will be transferred in callback function.
 *
 * This function called by FSM. Used to register callback for receiving data.
 *
 * Return: No return valude.
 *
 */
void mtk_port_internal_recv_register(void *i_port,
				     int (*cb)(void *priv, struct sk_buff *skb),
				     void *arg)
{
	struct mtk_port *port = i_port;
	struct mtk_internal_port *priv;

	priv = &port->priv.i_priv;
	priv->arg = arg;
	priv->recv_cb = cb;
}

int mtk_port_search(char *name, dev_t *result, unsigned int max_cnt)
{
	struct mtk_port *port;
	int current_cnt = 0;
	int vnr;

	if (!result || !max_cnt) {
		pr_err_ratelimited("[PORT_IO][search] Input value error\n");
		return -EINVAL;
	}

	mutex_lock(&port_mngr_grp_mtx);
	idr_for_each_entry(&ccci_minors, port, vnr) {
		/* If port is enabled and port name matches, recode the dev id */
		if (test_bit(PORT_S_ENABLE, &port->status) &&
		    !strncmp(port->info.name, name, strlen(name)))
			result[current_cnt++] = MKDEV(0, port->minor);
		if (current_cnt >= max_cnt)
			break;
	}
	mutex_unlock(&port_mngr_grp_mtx);
	return current_cnt;
}
EXPORT_SYMBOL_GPL(mtk_port_search);

int mtk_port_open(dev_t devt, int flag)
{
	struct mtk_port *port;
	int ret;

	port = mtk_port_get_by_devid_locked(devt);
	if (!port) {
		ret = -ENODEV;
		goto err;
	}

	ret = mtk_port_common_open(port);
	if (ret) {
		mtk_port_put_locked(port);
		goto err;
	}

	if (flag & O_NONBLOCK)
		port->info.flags &= ~PORT_F_BLOCKING;
	else
		port->info.flags |= PORT_F_BLOCKING;

err:
	return ret;
}
EXPORT_SYMBOL_GPL(mtk_port_open);

int mtk_port_close(dev_t devt)
{
	struct mtk_port *port;
	int ret = 0;

	/* This lock protect port from scenarios like:
	 * 1. port is moving to or from stale list, while the device is removing.
	 * 2. port will be deleted and may crash if user close twice by multi-thread.
	 * 3. port will be deleted if on stale list. And if user closes twice
	 * by multi-thread, the second close will crash because port is not exist.
	 */
	port = mtk_port_get_by_devid_locked(devt);
	if (!port) {
		ret = -ENODEV;
		goto err;
	}

	/* Avoid close port twice */
	if (!test_bit(PORT_S_OPEN, &port->status)) {
		ret = -EBADF;
		goto end_close;
	}

	mtk_port_common_close(port);

	wake_up_interruptible_all(&port->trb_wq);

	/* This kref_put is for port open */
	mtk_port_put_locked(port);

end_close:
	/* This kref_put is for port get at start of this function */
	mtk_port_put_locked(port);
err:
	return ret;
}
EXPORT_SYMBOL_GPL(mtk_port_close);

int mtk_port_write(dev_t devt, void *buf, unsigned int len)
{
	struct mtk_port *port;
	union user_buf user_buf;
	int ret;

	if (!buf) {
		pr_err_ratelimited("[PORT_IO][write] Input value error\n");
		ret = -EINVAL;
		goto err;
	}

	port = mtk_port_get_by_devid_locked(devt);
	if (!port) {
		ret = -ENODEV;
		goto err;
	}

	user_buf.kbuf = buf;
	ret = mtk_port_common_write(port, user_buf, len, false);

	mtk_port_put_locked(port);
err:
	return ret;
}
EXPORT_SYMBOL_GPL(mtk_port_write);

int mtk_port_read(dev_t devt, void *buf, unsigned int len)
{
	struct mtk_port *port;
	union user_buf user_buf;
	int ret;

	if (!buf) {
		pr_err_ratelimited("[PORT_IO][read] Input value error\n");
		ret = -EINVAL;
		goto err;
	}

	port = mtk_port_get_by_devid_locked(devt);
	if (!port) {
		ret = -ENODEV;
		goto err;
	}

	user_buf.kbuf = buf;
	ret = mtk_port_common_read(port, user_buf, len, false);

	mtk_port_put_locked(port);
err:
	return ret;
}
EXPORT_SYMBOL_GPL(mtk_port_read);

long mtk_port_ioctl(dev_t devt, unsigned int cmd, unsigned long arg)
{
	struct mtk_port *port;
	union user_buf user_buf;
	int ret;

	port = mtk_port_get_by_devid_locked(devt);
	if (!port) {
		ret = -ENODEV;
		goto err;
	}

	user_buf.kbuf = (void *)arg;
	ret = mtk_port_common_ioctl(port, cmd, user_buf, false);

	mtk_port_put_locked(port);
err:
	return ret;
}
EXPORT_SYMBOL_GPL(mtk_port_ioctl);

int mtk_port_send_brom_cmd(struct mtk_port *port, unsigned char cmd)
{
	union user_buf buf;

	buf.kbuf = (void *)&cmd;
	return mtk_port_common_write(port, buf, 1, false);
}

/* sleep_time: number of sleeps, 20ms at a time */
int mtk_port_read_brom_cmd_ack(struct mtk_port *port, unsigned char expected_cmd,
			       unsigned int sleep_time)
{
	unsigned char received_cmd;
	union user_buf user_buf;
	int ret;

	user_buf.kbuf = &received_cmd;
	do {
		ret = mtk_port_common_read(port, user_buf, 1, false);
		if (ret == 1) {
			if (unlikely(expected_cmd != received_cmd)) {
				ret = -EPROTO;
				goto exit;
			}

			return 0;
		}

		if (ret == -EAGAIN) {
			if (sleep_time)
				msleep(20);
		} else {
			goto exit;
		}
	} while (--sleep_time > 0);

	ret = -ETIMEDOUT;

exit:
	MTK_ERR(port->port_mngr->ctrl_blk->mdev,
		"Failed to recv brom cmd ack[0x%x], ret = %d, sleep_time=%d\n",
		expected_cmd, ret, sleep_time);
	return ret;
}

#ifdef CONFIG_SUPPORT_WWAN
static int mtk_port_wwan_open(struct wwan_port *w_port)
{
	struct mtk_port *port;
	int ret;

	port = wwan_port_get_drvdata(w_port);
	ret = mtk_port_get_locked(port);
	if (ret)
		return ret;

	ret = mtk_port_common_open(port);
	if (ret)
		mtk_port_put_locked(port);

	return ret;
}

static void mtk_port_wwan_close(struct wwan_port *w_port)
{
	struct mtk_port *port = wwan_port_get_drvdata(w_port);

	mtk_port_common_close(port);
	mtk_port_put_locked(port);
}

static int mtk_port_wwan_write(struct wwan_port *w_port, struct sk_buff *skb)
{
	struct mtk_port *port = wwan_port_get_drvdata(w_port);
	union user_buf user_buf;

	port->info.flags &= ~PORT_F_BLOCKING;
	user_buf.kbuf = (void *)skb->data;
	return mtk_port_common_write(port, user_buf, skb->len, false);
}

static int mtk_port_wwan_write_blocking(struct wwan_port *w_port, struct sk_buff *skb)
{
	struct mtk_port *port = wwan_port_get_drvdata(w_port);
	union user_buf user_buf;

	port->info.flags |= PORT_F_BLOCKING;
	user_buf.kbuf = (void *)skb->data;
	return mtk_port_common_write(port, user_buf, skb->len, false);
}

static __poll_t mtk_port_wwan_poll(struct wwan_port *w_port, struct file *file,
				   struct poll_table_struct *poll)
{
	struct mtk_port *port = wwan_port_get_drvdata(w_port);
	struct mtk_ctrl_blk *ctrl_blk;
	__poll_t mask = 0;

	if (mtk_port_status_check(port))
		goto end_poll;

	ctrl_blk = port->port_mngr->ctrl_blk;
	poll_wait(file, &port->trb_wq, poll);
	if (!VQ_LIST_FULL(ctrl_blk->trans, port->info.vq_id))
		mask |= EPOLLOUT | EPOLLWRNORM;

end_poll:
	return mask;
}

static const struct wwan_port_ops wwan_ops = {
	.start = mtk_port_wwan_open,
	.stop = mtk_port_wwan_close,
	.tx = mtk_port_wwan_write,
	.tx_blocking = mtk_port_wwan_write_blocking,
	.tx_poll = mtk_port_wwan_poll,
};

static int mtk_port_wwan_init(struct mtk_port *port)
{
	mtk_port_struct_init(port);
	port->enable = false;

	mutex_init(&port->priv.w_priv.w_lock);

	switch (port->info.rx_ch) {
	case CCCI_MBIM_RX:
		port->priv.w_priv.w_type = WWAN_PORT_MBIM;
		break;
	case CCCI_UART2_RX:
		port->priv.w_priv.w_type = WWAN_PORT_AT;
		break;
	case CCCI_SAP_GNSS_RX:
		port->priv.w_priv.w_type = WWAN_PORT_AT;
		break;
	case CCCI_LB_IT_RX:
		/* Loopback port turn to wwan type only for test */
		port->priv.w_priv.w_type = WWAN_PORT_AT;
		break;
	default:
		port->priv.w_priv.w_type = WWAN_PORT_UNKNOWN;
		break;
	}

	return 0;
}

static int mtk_port_wwan_exit(struct mtk_port *port)
{
	if (test_bit(PORT_S_ENABLE, &port->status))
		ports_ops[port->info.type]->disable(port);

	return 0;
}

static int mtk_port_wwan_enable(struct mtk_port *port)
{
	struct mtk_port_mngr *port_mngr;
	int ret = 0;

	port_mngr = port->port_mngr;

	if (test_bit(PORT_S_ENABLE, &port->status))
		goto end;

	ret = mtk_port_vq_enable(port);
	if (ret && ret != -EBUSY)
		goto end;

	port->priv.w_priv.w_port = wwan_create_port(port_mngr->ctrl_blk->mdev->dev,
						    port->priv.w_priv.w_type, &wwan_ops, port);
	if (IS_ERR(port->priv.w_priv.w_port)) {
		MTK_ERR(port_mngr->ctrl_blk->mdev,
			"Failed to create wwan port for (%s)\n", port->info.name);
		ret = PTR_ERR(port->priv.w_priv.w_port);
		goto end;
	}

	set_bit(PORT_S_RDWR, &port->status);
	set_bit(PORT_S_ENABLE, &port->status);

	return 0;
end:
	return ret;
}

static int mtk_port_wwan_disable(struct mtk_port *port)
{
	struct wwan_port *w_port;

	if (!test_and_clear_bit(PORT_S_ENABLE, &port->status))
		return 0;

	clear_bit(PORT_S_RDWR, &port->status);
	w_port = port->priv.w_priv.w_port;
	/* When the port is being disabled, port manager may receive RX data
	 * and try to call wwan_port_rx(). So the w_lock is to protect w_port
	 * from using by disable flow and receive flow at the same time.
	 */
	mutex_lock(&port->priv.w_priv.w_lock);
	port->priv.w_priv.w_port = NULL;
	mutex_unlock(&port->priv.w_priv.w_lock);

	wwan_remove_port(w_port);

	mtk_port_vq_disable(port);

	return 0;
}

static int mtk_port_wwan_recv(struct mtk_port *port, struct sk_buff *skb)
{
	if (!test_bit(PORT_S_OPEN, &port->status))
		goto drop_data;

	/* Protect w_port from using by disable flow and receive flow at the same time. */
	mutex_lock(&port->priv.w_priv.w_lock);
	if (!port->priv.w_priv.w_port) {
		mutex_unlock(&port->priv.w_priv.w_lock);
		MTK_WARN_RATELIMITED(port->port_mngr->ctrl_blk->mdev,
				     "Invalid (%s) wwan_port, drop packet\n", port->info.name);
		goto drop_data;
	}

	wwan_port_rx(port->priv.w_priv.w_port, skb);
	mutex_unlock(&port->priv.w_priv.w_lock);
	return 0;

drop_data:
	port->rx_err_cnt++;
	mtk_port_free_rx_skb(port, skb);
	return -ENXIO;
}

static const struct port_ops port_wwan_ops = {
	.init = mtk_port_wwan_init,
	.exit = mtk_port_wwan_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_wwan_enable,
	.disable = mtk_port_wwan_disable,
	.recv = mtk_port_wwan_recv,
	.dump = mtk_port_dump,
};

#ifdef CONFIG_SUPPORT_RELAYFS
#define MTK_RELAYFS_N_SUB_BUFF		16
#define MTK_RELAYFS_CTRL_FILE_PERM		0600

static ssize_t relayfs_ctrl_file_write(struct file *filp,
				       const char __user *buffer,
				       size_t count, loff_t *ppos)
{
	struct mtk_port *port = filp->private_data;
	union user_buf user_buf;

	user_buf.ubuf = (void __user *)buffer;
	return mtk_port_common_write(port, user_buf, (unsigned int)count, true);
}

static const struct file_operations relayfs_ctrl_file_fops = {
	.open = simple_open,
	.write = relayfs_ctrl_file_write,
};

static struct dentry *trace_create_buf_file_handler(const char *filename, struct dentry *parent,
						    umode_t mode, struct rchan_buf *buf,
						    int *is_global)
{
	*is_global = 1;
	return debugfs_create_file(filename, mode, parent, buf, &relay_file_operations);
}

static int trace_remove_buf_file_handler(struct dentry *dentry)
{
	debugfs_remove_recursive(dentry);
	return 0;
}

static int trace_subbuf_start_handler(struct rchan_buf *buf, void *subbuf,
				      void *prev_subbuf, size_t prev_padding)
{
	struct mtk_port *port = buf->chan->private_data;

	if (relay_buf_full(buf)) {
		pr_err_ratelimited("Failed to write relayfs buffer");
		atomic_set(&port->priv.rf_priv.is_full, 1);
		return 0;
	}
	atomic_set(&port->priv.rf_priv.is_full, 0);
	return 1;
}

static struct rchan_callbacks relay_callbacks = {
	.subbuf_start = trace_subbuf_start_handler,
	.create_buf_file = trace_create_buf_file_handler,
	.remove_buf_file = trace_remove_buf_file_handler,
};

static int mtk_port_relayfs_enable(struct mtk_port *port)
{
	struct dentry *debugfs_pdev;
	int ret;

	if (test_bit(PORT_S_ENABLE, &port->status))
		return 0;

	debugfs_pdev = wwan_get_debugfs_dir(port->port_mngr->ctrl_blk->mdev->dev);
	if (IS_ERR_OR_NULL(debugfs_pdev)) {
		MTK_ERR(port->port_mngr->ctrl_blk->mdev,
			"Failed to get wwan debugfs dentry port(%s)\n", port->info.name);
		return 0;
	}

	port->priv.rf_priv.d_wwan = debugfs_pdev;

	ret = mtk_port_vq_enable(port);
	if (ret && ret != -EBUSY)
		goto put_dbg_dir;

	if (port->info.rx_ch == CCCI_LB_IT_RX) {
		snprintf(port->priv.rf_priv.ctrl_file_name, MTK_DFLT_FULL_NAME_LEN, "%s_ctrl",
			 port->info.name);
		port->priv.rf_priv.ctrl_file =
			debugfs_create_file(port->priv.rf_priv.ctrl_file_name,
					    MTK_RELAYFS_CTRL_FILE_PERM,
					    debugfs_pdev,
					    port, &relayfs_ctrl_file_fops);
	}

	port->priv.rf_priv.rc = relay_open(port->info.name,
					   debugfs_pdev,
					   port->rx_mtu,
					   MTK_RELAYFS_N_SUB_BUFF,
					   &relay_callbacks, port);

	set_bit(PORT_S_RDWR, &port->status);
	set_bit(PORT_S_ENABLE, &port->status);
	/* Open port and allow to receive data */
	ret = mtk_port_common_open(port);
	if (ret)
		goto put_dbg_dir;
	port->info.flags &= ~PORT_F_BLOCKING;
	return 0;

put_dbg_dir:
	wwan_put_debugfs_dir(port->priv.rf_priv.d_wwan);
	return ret;
}

static int mtk_port_relayfs_disable(struct mtk_port *port)
{
	if (!test_and_clear_bit(PORT_S_ENABLE, &port->status))
		goto end;
	clear_bit(PORT_S_RDWR, &port->status);
	mtk_port_common_close(port);

	relay_close(port->priv.rf_priv.rc);
	wwan_put_debugfs_dir(port->priv.rf_priv.d_wwan);
	mtk_port_vq_disable(port);
end:
	return 0;
}

static int mtk_port_relayfs_recv(struct mtk_port *port, struct sk_buff *skb)
{
	struct mtk_relayfs_port *relayfs_port = &port->priv.rf_priv;

	while (test_bit(PORT_S_OPEN, &port->status) && test_bit(PORT_S_ENABLE, &port->status) &&
	       test_bit(PORT_S_RDWR, &port->status)) {
		if (port->port_mngr->in_suspend)
			return -EPIPE;

		__relay_write(relayfs_port->rc, skb->data, skb->len);
		if (atomic_read(&port->priv.rf_priv.is_full)) {
			msleep(20);
			continue;
		} else {
			break;
		}
	}

	mtk_port_free_rx_skb(port, skb);
	return 0;
}

static int mtk_port_relayfs_init(struct mtk_port *port)
{
	mtk_port_struct_init(port);
	port->enable = false;
	atomic_set(&port->priv.rf_priv.is_full, 0);

	return 0;
}

static int mtk_port_relayfs_exit(struct mtk_port *port)
{
	if (test_bit(PORT_S_ENABLE, &port->status))
		ports_ops[port->info.type]->disable(port);

	return 0;
}

static const struct port_ops port_relayfs_ops = {
	.init = mtk_port_relayfs_init,
	.exit = mtk_port_relayfs_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_relayfs_enable,
	.disable = mtk_port_relayfs_disable,
	.recv = mtk_port_relayfs_recv,
	.dump = mtk_port_dump,
};

/*!CONFIG_SUPPORT_RELAYFS*/
#else
/*if relayfs isn't supported, the trace port will be created as cdev port.*/
static const struct port_ops port_relayfs_ops = {
	.init = mtk_port_cdev_init,
	.exit = mtk_port_cdev_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_cdev_enable,
	.disable = mtk_port_cdev_disable,
	.recv = mtk_port_recv,
	.dump = mtk_port_dump,
};

#endif
/*!CONFIG_SUPPORT_WWAN*/
#else
/*if wwan isn't supported, the wwan port will be created as cdev port.*/
static const struct port_ops port_wwan_ops = {
	.init = mtk_port_cdev_init,
	.exit = mtk_port_cdev_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_cdev_enable,
	.disable = mtk_port_cdev_disable,
	.recv = mtk_port_recv,
	.dump = mtk_port_dump,
};

/*if wwan isn't supported, the trace port will be created as cdev port.*/
static const struct port_ops port_relayfs_ops = {
	.init = mtk_port_cdev_init,
	.exit = mtk_port_cdev_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_cdev_enable,
	.disable = mtk_port_cdev_disable,
	.recv = mtk_port_recv,
	.dump = mtk_port_dump,
};
#endif

static const struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.open = mtk_port_cdev_open,
	.read = mtk_port_cdev_read,
	.write = mtk_port_cdev_write,
	.release = mtk_port_cdev_close,
	.unlocked_ioctl = mtk_port_cdev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mtk_port_cdev_compat_ioctl,
#endif
	.poll = mtk_port_cdev_poll,
};

static const struct port_ops port_cdev_ops = {
	.init = mtk_port_cdev_init,
	.exit = mtk_port_cdev_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_cdev_enable,
	.disable = mtk_port_cdev_disable,
	.recv = mtk_port_recv,
	.dump = mtk_port_dump,
};

static const struct port_ops port_internal_ops = {
	.init = mtk_port_internal_init,
	.exit = mtk_port_internal_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_internal_enable,
	.disable = mtk_port_internal_disable,
	.recv = mtk_port_internal_recv,
	.dump = mtk_port_dump,
};

static const struct port_ops port_proprietary_ops = {
	.init = mtk_port_proprietary_init,
	.exit = mtk_port_proprietary_exit,
	.reset = mtk_port_reset,
	.enable = mtk_port_proprietary_enable,
	.disable = mtk_port_proprietary_disable,
	.recv = mtk_port_recv,
	.dump = mtk_port_dump,
};

const struct port_ops *ports_ops[PORT_TYPE_MAX] = {
	&port_cdev_ops,
	&port_internal_ops,
	&port_proprietary_ops,
	&port_wwan_ops,
	&port_relayfs_ops
};

module_param(brom_dl_flag, ushort, 0644);
MODULE_PARM_DESC(brom_dl_flag, "This value is used to control dl behaivor\n");

