// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kern_levels.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/kernel.h>
#include <linux/amba/bus.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/version.h>

#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_reg.h>
#include <soc/xring/ipc/xr_ipc.h>
#include "../xr_ipc_main.h"
#include "xr_mbox.h"

#define XR_IPC_ID		(0x00041320)
#define XR_IPC_MASK		(0x000fffff)

#define VC_UNUSED		(0)
#define VC_USED			(1)

#define FIFO_SIZE		(64)

#define IPC_MBOX_MANUAL_ACK	(0)
#define IPC_MBOX_AUTO_ACK	(1)
#define IPC_MBOX_SEND_MSG	IPC_MBOX_MANUAL_ACK
#define IPC_MBOX_SEND_RESP	IPC_MBOX_AUTO_ACK

#define IPC_MBOX_RAW_DATA_REG	(7)
#define IPC_MBOX_NEW_DATA_REG	(8)

/* IPC send message direction */
#define IPC_MBOX_SEND_INACTIVE	(0x00UL)
#define IPC_MBOX_SEND_MSG2DST	(0x01UL)
#define IPC_MBOX_SEND_MSG2SRC	(0x02UL)

#define IPC_MBOX_SOURCE_CLEAR	(0UL)

#define IPCMxSOURCE_WRITE_COUNT	(10)

#define IPC_RESOURCE_SIZE	(4096)

#define XR_IPC_MAX_ANALYSIS_BUFFER	(512)

#define ipc_err(fmt, args...)		pr_err("[xr_ipc]" fmt "\n", ##args)
#define ipc_info(fmt, args...)		pr_info("[xr_ipc]" fmt "\n", ##args)
#define ipc_debug(fmt, args...)		pr_debug("[xr_ipc]" fmt "\n", ##args)

enum {
	RX_BUFFER_TYPE = 0,
	RESP_BUFFER_TYPE,
	MBOX_BUFFER_TYPE_MAX
};

static int ipc_get_ch_id(int word)
{
	int i = 0;

	while ((word & 1) == 0) {
		word >>= 1;
		i++;
	}

	return i - 1;
}

static inline unsigned int mbox_source_get(
		const void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxSOURCE(mbox_id));
}

static inline void mbox_source_set(
		void __iomem *base, int mbox_id, int src_id)
{
	__raw_writel(IPC_MASK(src_id),
				base + IPCMxSOURCE(mbox_id));
}

static inline unsigned int mbox_get_send_st(
		const void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxSEND((unsigned int)mbox_id));
}

static inline void mbox_mode_set(
		void __iomem *base, int mbox_id, int mode)
{
	__raw_writel(mode, base + IPCMxMODE(mbox_id));
}

static inline unsigned int mbox_ack_st(
		const void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxMODE((unsigned int)mbox_id));
}

static inline unsigned int mbox_msg_read(
		const void __iomem *base, int mbox_id, int index)
{
	if (index < IPC_MBOX_RAW_DATA_REG) {
		return __raw_readl(base + IPCMxDR((unsigned int)mbox_id,
				(unsigned int)index));
	} else {
		return __raw_readl(base + IPCMxDR_NEW((unsigned int)mbox_id,
				(unsigned int)index - IPC_MBOX_RAW_DATA_REG));
	}
}

static inline void mbox_set_des(
		void __iomem *base, int mbox_id, int dst)
{
	__raw_writel(IPC_MASK(dst), base + IPCMxDSET(mbox_id));
}

static inline void mbox_set_mask(
		void __iomem *base, int mbox_id, int dst, int src)
{
	__raw_writel(IPC_MASK(dst) | IPC_MASK(src),
		base + IPCMxMSET(mbox_id));
}

/* mask current mailbox interrupt to current core */
static inline void mbox_mask_recv_irq(
		void __iomem *base, int mbox_id)
{
	__raw_writel(mbox_id, base + IPCMxMCLEAR(mbox_id));
}

/* unmask current mailbox interrupt to current core */
static inline void mbox_unmask_recv_irq(
		void __iomem *base, int mbox_id, int src_id)
{
	__raw_writel(IPC_MASK(src_id),
		base + IPCMxMSET(mbox_id));
}

static inline void mbox_clr_intr(
		void __iomem *base, int mbox_id, int src_id)
{
	__raw_writel(IPC_MASK(src_id),
		base + IPCMxDCLEAR(mbox_id));
}

static inline void mbox_clear_ack(
		void __iomem *base, int mbox_id)
{
	__raw_writel(IPC_MBOX_SEND_INACTIVE,
		base + IPCMxSEND(mbox_id));
}

static inline void mbox_manual_ack(
		void __iomem *base, int mbox_id)
{
	__raw_writel(IPC_MBOX_SEND_MSG2SRC,
		base + IPCMxSEND(mbox_id));
}

static inline void mbox_send_set(
		void __iomem *base, int mbox_id)
{
	__raw_writel(IPC_MBOX_SEND_MSG2DST,
		base + IPCMxSEND(mbox_id));
}

static inline unsigned int mbox_send_read(
		void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxSEND((unsigned int)mbox_id));
}

static inline void mbox_dr_write(
		void __iomem *base, int mbox_id,
		int index, unsigned int msg)
{
	if (index < IPC_MBOX_RAW_DATA_REG) {
		__raw_writel(((unsigned int)msg), base +
			IPCMxDR((unsigned int)mbox_id, (unsigned int)index));
	} else {
		__raw_writel(((unsigned int)msg), base +
			IPCMxDR_NEW((unsigned int)mbox_id,
				(unsigned int)index - IPC_MBOX_RAW_DATA_REG));
	}
}

static inline void mbox_realease(
		void __iomem *base, int mbox_id)
{
	__raw_writel(IPC_MBOX_SOURCE_CLEAR,
		base + IPCMxSOURCE(mbox_id));
}

static inline unsigned int mbox_dsta_get(
		const void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxDSTATUS(mbox_id));
}

static inline unsigned int mbox_msta_get(
		const void __iomem *base, int mbox_id)
{
	return __raw_readl(base + IPCMxMSTATUS(mbox_id));
}

static inline unsigned int mbox_mmis_get(
		const void __iomem *base, int chan_id)
{
	return __raw_readl(base + IPCMMIS(chan_id));
}

static inline unsigned int mbox_mris_get(
		const void __iomem *base, int chan_id)
{
	return __raw_readl(base + IPCMRIS(chan_id));
}

static inline int  mbox_irq_to(
		void __iomem *base, int irq)
{
	return __raw_readl(base + IPCMRIS(irq));
}

static unsigned int xr_mdev_msta(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	unsigned int reg_sta = mbox_msta_get(priv->idev->base, priv->tx_mbox_id);
	unsigned int ch_sta = IPC_MASK(priv->src_chn_id) | IPC_MASK(priv->dst_chn_id);

	return (ch_sta != reg_sta);
}

static int xr_mdev_is_ack(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int is_ack = 0;
	int send_st;

	send_st = mbox_get_send_st(priv->idev->base, priv->tx_mbox_id);
	if (send_st & IPC_MBOX_SEND_MSG2SRC)
		is_ack = 1;

	return is_ack;
}

static int xr_mdev_is_msg(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int is_msg = 0;
	int ack_st;

	ack_st = mbox_ack_st(priv->idev->base, priv->rx_mbox_id);
	if (ack_st == IPC_MBOX_SEND_MSG)
		is_msg = 1;

	return is_msg;
}

static int xr_mdev_is_send(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int is_send = 0;
	int send_st;

	send_st = mbox_get_send_st(priv->idev->base, priv->rx_mbox_id);
	if (send_st == IPC_MBOX_SEND_MSG2DST)
		is_send = 1;

	return is_send;
}

static unsigned int xr_mdev_fifo_size(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	priv->fifo_size = FIFO_SIZE;

	return priv->fifo_size;
}

static int xr_irq_sta(struct xr_mbox_device *mdev)
{
	int sta = 0;
	int irq = 0;

	if (xr_mdev_is_send(mdev)) {
		sta = xr_mdev_is_msg(mdev);
		if (sta)
			irq |= IPC_MASK(RECV_IRQ);
		else
			irq |= IPC_MASK(RESP_IRQ);
	}

	sta = xr_mdev_is_ack(mdev);
	if (sta)
		irq |= IPC_MASK(ACK_IRQ);

	return irq;
}

static void xr_mdev_hw_read(struct xr_mbox_device *mdev,
			unsigned int *msg)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int i;

	for (i = 0; i < priv->capability; i++) {
		msg[i] = mbox_msg_read(priv->idev->base,
					priv->rx_mbox_id, i);
	}
}

static void xr_mdev_clr_irq(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	/* unmask irq */
	mbox_unmask_recv_irq(priv->idev->base,
			priv->rx_mbox_id, priv->src);

	/* clear IPCMxDSTATUS REGs to clear interrupt */
	mbox_clr_intr(priv->idev->base,
		priv->rx_mbox_id, priv->src);
}

static int xr_mdev_recv_msg(struct xr_mbox_device *mdev,
			unsigned int **buf)
{
	unsigned int *_buf = NULL;

	if (!buf)
		return -EINVAL;

	if (xr_mdev_is_msg(mdev))
		_buf = mdev->rx_buffer;
	else
		_buf = mdev->resp_buffer;

	if (_buf)
		xr_mdev_hw_read(mdev, _buf);

	*buf = _buf;

	xr_mdev_clr_irq(mdev);
	return 0;
}

static void xr_mdev_read_msg(struct xr_mbox_device *mdev,
			unsigned int *rx_buff)
{
	xr_mdev_hw_read(mdev, rx_buff);

	xr_mdev_clr_irq(mdev);
}

void xr_mdev_send_set(struct xr_mbox_device *mdev, int ack_mode)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	/* set mode to differentiate send message or send response */
	mbox_mode_set(priv->idev->base, priv->tx_mbox_id, ack_mode);
	mbox_set_des(priv->idev->base, priv->tx_mbox_id, priv->dst_chn_id);
	mbox_set_mask(priv->idev->base, priv->tx_mbox_id,
		priv->dst_chn_id, priv->src_chn_id);
}

void xr_mdev_hw_send(struct xr_mbox_device *mdev, u32 *msg, u32 len)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int msg_base = mdev->tx_task->send_len;
	int msg_size = len;
	int i;

	if (msg_size > MBOX_MAX_DATA_REG)
		msg_size = MBOX_MAX_DATA_REG;

	/* fill data register */
	for (i = 0; i < msg_size; i++) {
		mbox_dr_write(priv->idev->base, priv->tx_mbox_id,
				i, msg[i + msg_base]);
	}

	/* write the others register data to 0 */
	for (; i < priv->capability; i++)
		mbox_dr_write(priv->idev->base, priv->tx_mbox_id, i, 0);

	mdev->tx_task->tx_buffer_len -= msg_size;
	mdev->tx_task->send_len += msg_size;

	/* trigger destination core's interrupt */
	mbox_send_set(priv->idev->base, priv->tx_mbox_id);
}

void xr_ipc_pr_ack_timeout(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int send;

	send = mbox_send_read(priv->idev->base, priv->tx_mbox_id);
	if (send == IPC_MBOX_SEND_MSG2DST)
		ipc_err("The destination core did not reply with an ack, send = %d", send);
	else if (send == IPC_MBOX_SEND_MSG2SRC)
		ipc_err("The destination core has replied with an ack, send = %d", send);
	else
		ipc_err("The mailbox has been released, send = %d", send);
}

static int xr_mdev_send_msg(struct xr_mbox_device *mdev, unsigned int *msg,
			unsigned int len)
{
	if (mdev->status == MDEV_DEACTIVATED)
		return -EINVAL;

	if (!msg) {
		ipc_err("ipc-%d no message", mdev->ipc_id);
		return -EINVAL;
	}

	xr_mdev_send_set(mdev, IPC_MBOX_SEND_MSG);
	xr_mdev_hw_send(mdev, msg, len);
	return 0;
}

static int xr_mdev_send_resp(struct xr_mbox_device *mdev, unsigned int *msg,
			unsigned int len)
{
	if (mdev->status == MDEV_DEACTIVATED)
		return -EINVAL;

	if (!msg) {
		ipc_err("ipc-%d no message", mdev->ipc_id);
		return -EINVAL;
	}

	xr_mdev_send_set(mdev, IPC_MBOX_SEND_RESP);
	xr_mdev_hw_send(mdev, msg, len);
	return 0;
}

static int xr_mdev_ensure_channel(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	struct xr_ipc_device *idev = priv->idev;
	int ipc_src = 0;
	int ret = -EAGAIN;
	int i;

	/* Use a mutex to protect the mailbox when the vc_id uses a public mailbox */
	if (priv->com_mbox == IPC_COM_MAILBOX)
		mutex_lock(&idev->mbox_lock);

	/* Maximum wait time for last send */
	for (i = 0; i < priv->ack_timeout; i++) {
		ipc_src = mbox_source_get(priv->idev->base, priv->tx_mbox_id);
		if (!ipc_src)
			break;

		ipc_err("mailbox has used by channel %d", ipc_get_ch_id(ipc_src));
		msleep(1);
	}

	/* After checks, the mailbox source is still used and an error is returned */
	if (ipc_src) {
		ret = -EBUSY;
		goto unlock;
	}

	for (i = 0; i < IPCMxSOURCE_WRITE_COUNT; i++) {
		mbox_source_set(priv->idev->base, priv->tx_mbox_id, priv->src_chn_id);

		ipc_src = mbox_source_get(priv->idev->base, priv->tx_mbox_id);
		if (ipc_src == IPC_MASK(priv->src_chn_id)) {
			ret = 0;
			goto unlock;
		}

		udelay(10);
	}

unlock:
	if (priv->com_mbox == IPC_COM_MAILBOX)
		mutex_unlock(&idev->mbox_lock);

	return ret;
}

static inline void mbox_ack_to_src(
		void __iomem *base, int mbox_id, int src)
{
	/* clear IPCMxDSTATUS REGs to clear interrupt */
	__raw_writel(IPC_MASK(src), base + IPCMxDCLEAR(mbox_id));
}

static void xr_mdev_ack(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	int st;

	mbox_ack_to_src(priv->idev->base, priv->rx_mbox_id,
			priv->src_chn_id);

	st = mbox_ack_st(priv->idev->base, priv->rx_mbox_id);

	/* if manual ack, then ack to src core */
	if (st == IPC_MBOX_MANUAL_ACK)
		mbox_manual_ack(priv->idev->base, priv->rx_mbox_id);
}

static void xr_mdev_release(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	mbox_realease(priv->idev->base, priv->tx_mbox_id);
}

static int xr_mdev_gic_request(struct xr_mbox_device *mdev,
	irq_handler_t handler, void *dev_id)
{
	int ret;

	ret = request_irq(mdev->cur_irq, handler,
		IRQF_NO_SUSPEND, mdev->name, dev_id);

	return ret;
}

static int xr_mdev_irq_request(
	struct xr_mbox_device *mdev, irq_handler_t handler, void *dev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	int ret = 0;

	ret = xr_mdev_gic_request(mdev, handler, dev);
	if (ret) {
		ipc_err("fast desitnation %s request irq %d failed",
			mdev->name, priv->dst_chn_id);
	}

	return ret;
}

static void xr_mdev_irq_enable(struct xr_mbox_device *mdev)
{
	enable_irq((unsigned int)mdev->cur_irq);
}

static void xr_mdev_irq_disable(struct xr_mbox_device *mdev)
{
	disable_irq_nosync((unsigned int)mdev->cur_irq);
}

static void xr_mdev_clr_ack(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	mbox_clear_ack(priv->idev->base, priv->tx_mbox_id);
}

unsigned char rproc_find_index(unsigned int pro_code)
{
	unsigned char index = 0;

	while (pro_code) {
		index++;
		pro_code >>= 1;
	}
	return index;
}

static const char *rproc_analysis(
	struct xr_mbox_device *mdev, unsigned int pro_code)
{
	const char *mdev_name = mdev->name;
	struct xr_mbox_device_priv *priv = mdev->priv;
	unsigned char index;

	index = rproc_find_index(pro_code);
	if (!index)
		return "RPROC = 0";
	index--;
	if (likely(index < priv->idev->vc_num))
		return mdev_name;
	else
		return "ERR_RPROC";
}

static void xr_mdev_dump_status(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;
	unsigned int mbox_tx_src;
	unsigned int mbox_tx_dsta;
	unsigned int mbox_tx_msta;
	unsigned int mbox_tx_send;
	unsigned int mbox_tx_mmis;
	unsigned int mbox_tx_mris;

	unsigned int mbox_rx_src;
	unsigned int mbox_rx_dsta;
	unsigned int mbox_rx_msta;
	unsigned int mbox_rx_send;
	unsigned int mbox_rx_mmis;
	unsigned int mbox_rx_mris;
	unsigned int mbox_src = 0;
	const char *vc_name;

	mbox_tx_src = mbox_source_get(priv->idev->base, priv->tx_mbox_id);
	mbox_rx_src = mbox_source_get(priv->idev->base, priv->rx_mbox_id);
	mbox_tx_dsta = mbox_dsta_get(priv->idev->base, priv->tx_mbox_id);
	mbox_rx_dsta = mbox_dsta_get(priv->idev->base, priv->rx_mbox_id);
	mbox_tx_msta = mbox_msta_get(priv->idev->base, priv->tx_mbox_id);
	mbox_rx_msta = mbox_msta_get(priv->idev->base, priv->rx_mbox_id);
	mbox_tx_send = mbox_send_read(priv->idev->base, priv->tx_mbox_id);
	mbox_rx_send = mbox_send_read(priv->idev->base, priv->rx_mbox_id);
	mbox_tx_mmis = mbox_mmis_get(priv->idev->base, priv->dst_chn_id);
	mbox_rx_mmis = mbox_mmis_get(priv->idev->base, priv->src_chn_id);
	mbox_tx_mris = mbox_mmis_get(priv->idev->base, priv->dst_chn_id);
	mbox_rx_mris = mbox_mmis_get(priv->idev->base, priv->src_chn_id);
	vc_name = rproc_analysis(mdev, mbox_src);

	ipc_err("[ipc_vc]: [%s] timeout", vc_name);
	ipc_err("mbox_tx_src = 0x%x\n"
		"mbox_tx_dsta = 0x%x\n"
		"mbox_tx_msta = 0x%x\n"
		"mbox_tx_send = 0x%x\n"
		"mbox_tx_mmis = 0x%x\n"
		"mbox_tx_mris = 0x%x\n"
		"mbox_rx_src = 0x%x\n"
		"mbox_rx_dsta = 0x%x\n"
		"mbox_rx_msta = 0x%x\n"
		"mbox_rx_send = 0x%x\n"
		"mbox_rx_mmis = 0x%x\n"
		"mbox_rx_mris = 0x%x\n",
		mbox_tx_src, mbox_tx_dsta, mbox_tx_msta,
		mbox_tx_send, mbox_tx_mmis, mbox_tx_mris,
		mbox_rx_src, mbox_rx_dsta, mbox_rx_msta,
		mbox_rx_send, mbox_rx_mmis, mbox_rx_mris);
}

static unsigned int xr_ack_timeout(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	return priv->ack_timeout;
}

static unsigned int xr_resp_timeout(struct xr_mbox_device *mdev)
{
	struct xr_mbox_device_priv *priv = mdev->priv;

	return priv->resp_timeout;
}

static ssize_t ipc_time_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct amba_device *adev = container_of(dev, struct amba_device, dev);
	struct xr_ipc *xdev = amba_get_drvdata(adev);
	struct xr_mbox_device **mdev_res = xdev->mdev_res;
	struct xr_mbox_device *mdev = NULL;
	int mbox_num = xdev->total_vc_num;
	u64 max_send_sync_time;
	u64 ave_send_sync_time;
	u64 max_send_recv_time;
	u64 ave_send_recv_time;
	int i;

	for (i = 0; i < mbox_num; i++) {
		mdev = mdev_res[i];

		max_send_sync_time = mdev->max_send_sync_time;
		ave_send_sync_time = mdev->ave_send_sync_time;
		max_send_recv_time = mdev->max_send_recv_time;
		ave_send_recv_time = mdev->ave_send_recv_time;

		dev_info(dev, "vc_id[%s] max sync send time = %llu ns\n",
			mdev->name, max_send_sync_time);
		dev_info(dev, "vc_id[%s] average sync send time = %llu ns\n",
			mdev->name, ave_send_sync_time);
		dev_info(dev, "vc_id[%s] max sync send and recv time = %llu ns\n",
			mdev->name, max_send_recv_time);
		dev_info(dev, "vc_id[%s] average sync and recv send time = %llu ns\n",
			mdev->name, ave_send_recv_time);
		dev_info(dev, "vc_id[%s] number of messages received = %d",
			mdev->name, atomic_read(&mdev->recv_num));
		dev_info(dev, "vc_id[%s] number of messages async_send = %d",
			mdev->name, atomic_read(&mdev->async_send_num));
	}

	return 0;
}
static DEVICE_ATTR_RO(ipc_time);

static void ipc_creat_sysfs(struct device *pdev)
{
	if (device_create_file(pdev, &dev_attr_ipc_time) < 0)
		dev_err(pdev, "Failed to create file dev_attr_ipc_time\n");
}

static void ipc_remove_sysfs(struct device *pdev)
{
	device_remove_file(pdev, &dev_attr_ipc_time);
}

struct xr_mbox_dev_ops xr_mdev_ops = {
	.send = xr_mdev_send_msg,
	.resp = xr_mdev_send_resp,
	.recv = xr_mdev_recv_msg,
	.read = xr_mdev_read_msg,
	.ack = xr_mdev_ack,
	.msta = xr_mdev_msta,

	.request_irq = xr_mdev_irq_request,
	.free_irq = xr_mdev_clr_irq,
	.enable_irq = xr_mdev_irq_enable,
	.disable_irq = xr_mdev_irq_disable,

	.release = xr_mdev_release,
	.get_ack_timeout = xr_ack_timeout,
	.get_resp_timeout = xr_resp_timeout,
	.is_ack = xr_mdev_is_ack,
	.is_msg = xr_mdev_is_msg,
	.irq_sta = xr_irq_sta,
	.clr_ack = xr_mdev_clr_ack,
	.ensure_channel = xr_mdev_ensure_channel,
	.get_fifo_size = xr_mdev_fifo_size,

	.dump_status = xr_mdev_dump_status,
};

static int xr_ipc_mdev_suspend(struct device *dev)
{
	struct amba_device *adev =
		container_of(dev, struct amba_device, dev);
	struct xr_ipc *xdev = amba_get_drvdata(adev);

	if (xdev)
		xr_mbox_device_deactivate(xdev->mdev_res);

	return 0;
}

static int xr_ipc_mdev_resume(struct device *dev)
{
	struct amba_device *adev =
		container_of(dev, struct amba_device, dev);
	struct xr_ipc *xdev = amba_get_drvdata(adev);

	if (xdev)
		xr_mbox_device_activate(xdev->mdev_res);

	return 0;
}

static void xr_mbox_put(struct xr_mbox_device **list)
{
	struct xr_mbox_device *mdev = NULL;
	int i;

	i = 0;
	mdev = list[0];
	while (mdev) {
		i++;
		kfree(mdev);
		mdev = list[i];
	}
}

static void xr_mdev_free(struct xr_ipc *xdev)
{
	struct xr_mbox_device **list = xdev->mdev_res;

	xr_mbox_put(list);
}

static void idev_mbox_free(struct xr_ipc_device **idev)
{
	int i = 0;

	while (idev[i]) {
		kfree(idev[i]->buf_pool);
		if (idev[i]->buf_pool)
			idev[i]->buf_pool = NULL;

		if (idev[i]->base) {
			iounmap(idev[i]->base);
			idev[i]->base = NULL;
		}

		mutex_destroy(&idev[i]->mbox_lock);

		i++;
	}
}

static void xr_ipc_dev_remove(struct amba_device *adev)
{
	struct xr_ipc *xdev = amba_get_drvdata(adev);

	amba_set_drvdata(adev, NULL);
	ipc_remove_sysfs(&adev->dev);

	if (!xdev)
		return;

	if (xdev->mdev_res) {
		xr_mbox_device_unregister(xdev->mdev_res);
		xr_mdev_free(xdev);
	}

	kfree(xdev);
}

static void ipc_info_print(struct xr_ipc_device *idev,
	unsigned int vc_num)
{
	ipc_debug("ipc_name: %s", idev->ip_name);
	ipc_debug("ipc_id: %u", idev->ip_id);
	ipc_debug("capability: %u", idev->capability);
	ipc_debug("mailboxes: %u", vc_num);
}

static void mbox_info_print(struct xr_mbox_device_priv *priv)
{
	ipc_debug("vc_id: %d", priv->vc_id);
	ipc_debug("src_chn_id: %d", priv->src_chn_id);
	ipc_debug("tx_mbox_id: %u", priv->tx_mbox_id);
	ipc_debug("dst_chn_id: %d", priv->dst_chn_id);
	ipc_debug("rx_mbox_id: %u", priv->rx_mbox_id);
}

static int mdev_idev_init(struct xr_ipc_device *idev,
	struct device_node *node)
{
	unsigned int *buf_pool = NULL;
	unsigned int buf_pool_len;
	unsigned int vc_num;
	int id;

	id = idev->ip_id;
	idev->base = of_iomap(node, id);
	if (!idev->base) {
		ipc_err("iomap error");
		goto out;
	}

	vc_num = idev->vc_num;

	buf_pool_len = idev->capability * vc_num * MBOX_BUFFER_TYPE_MAX;
	buf_pool = kcalloc(buf_pool_len, sizeof(*buf_pool), GFP_KERNEL);
	if (!buf_pool)
		goto to_iounmap;

	idev->buf_pool = buf_pool;

	mutex_init(&idev->mbox_lock);

	ipc_info_print(idev, vc_num);

	return 0;

to_iounmap:
	iounmap(idev->base);
out:
	return -ENODEV;
}

static struct xr_mbox_device_priv *mdev_priv_init(struct device_node *child,
	struct xr_ipc_device *idev, struct xr_mbox_device *mdev, unsigned int used)
{
	int ret;
	struct xr_mbox_device_priv *priv = NULL;

	priv = kcalloc(1, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return NULL;

	ret = of_property_read_u32(child, "vc_id", (u32 *)&(priv->vc_id));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "dst_chn_name", (u32 *)&(priv->dst_chn_name));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "src_chn_id", (u32 *)&(priv->src_chn_id));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "tx_mbox_id", (u32 *)&(priv->tx_mbox_id));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "src_chn_name", (u32 *)&(priv->src_chn_name));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "dst_chn_id", (u32 *)&(priv->dst_chn_id));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "rx_mbox_id", (u32 *)&(priv->rx_mbox_id));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "wait_ack_timeout", (u32 *)&(priv->ack_timeout));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "wait_resp_timeout", (u32 *)&(priv->resp_timeout));
	if (ret)
		goto free_priv;

	ret = of_property_read_u32(child, "fast_ack", (u32 *)&(priv->fast_ack));

	ret = of_property_read_u32(child, "com_mbox", (u32 *)&(priv->com_mbox));

	priv->capability = idev->capability;
	priv->idev = idev;
	priv->used = used;
	mbox_info_print(priv);
	return priv;

free_priv:
	kfree(priv);
	return NULL;
}

static int xr_mdev_get(struct xr_ipc_device **idev,
	struct xr_mbox_device **mdevs, struct device_node *node)
{
	struct device_node *child = NULL;
	struct xr_mbox_device *mdev = NULL;
	struct xr_mbox_device_priv *priv = NULL;
	unsigned int *rx_buffer = NULL;
	unsigned int *resp_buffer = NULL;

	int child_num = 0;
	int mdev_id = 0;
	int idev_id = 0;
	int capa;
	int used;
	int ret = 0;
	int i;

	for (i = 0; (child = of_get_next_child(node, child)); i++) {
		if (mdev_id >= (child_num + idev[idev_id]->vc_num)) {
			child_num += idev[idev_id]->vc_num;
			idev_id++;
		}

		used = 0;
		ret = of_property_read_u32(child, "used", &used);
		if (ret) {
			ipc_err("VC-%d has no tag <used>",
				mdev_id);
			goto to_break;
		}
		if (used == VC_UNUSED) {
			ipc_debug("mailbox node %s is not used", child->name);
			continue;
		}

		mdev = kcalloc(1, sizeof(*mdev), GFP_KERNEL);
		if (!mdev) {
			ret = -ENOMEM;
			goto to_break;
		}

		priv = mdev_priv_init(child, idev[idev_id], mdev, used);
		if (!priv) {
			ret = -ENOMEM;
			goto free_mdev;
		}

		capa = idev[idev_id]->capability;
		ipc_debug("vc node: %s", child->name);

		rx_buffer = idev[idev_id]->buf_pool + capa * RX_BUFFER_TYPE;
		resp_buffer = idev[idev_id]->buf_pool + capa * RESP_BUFFER_TYPE;
		idev[idev_id]->buf_pool = idev[idev_id]->buf_pool +
					capa * MBOX_BUFFER_TYPE_MAX;

		mdev->name = child->name;
		mdev->priv = priv;
		mdev->rx_buffer = rx_buffer;
		mdev->resp_buffer = resp_buffer;
		mdev->ops = &xr_mdev_ops;
		mdev->capability = capa;
		mdev->cur_irq = irq_of_parse_and_map(child, 0);

		if (mdev->cur_irq <= 0) {
			ipc_err("Failed to parse and map IRQ %s\n", mdev->name);
			ret = -EINVAL;
			goto free_mdev;
		}

		mdevs[mdev_id] = mdev;
		mdev_id++;

		continue;

free_mdev:
		kfree(mdev);
to_break:
		break;
	}

	if (!ret)
		return ret;

	while (i--)
		kfree(mdevs[i]->priv);

	idev_mbox_free(idev);
	return ret;
}

static int xr_ipc_mbox_get(struct device_node *node,
			struct xr_ipc_device **idev,
			struct xr_mbox_device **mdev_res)
{
	int ret;

	ret = xr_mdev_get(idev, mdev_res, node);
	if (ret) {
		ipc_err("dts[%s] can not get ipc resource", node->name);
		ret = -ENODEV;
	}

	return ret;
}

static int xr_mbox_is_valid(struct xr_mbox_device *mdev)
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

static int xr_mbox_dev_reg(struct xr_mbox_device **list)
{
	struct xr_mbox_device *mdev = NULL;
	int ret;
	int i;

	if (!list) {
		ipc_err("null pointer");
		return -EINVAL;
	}

	for (i = 0; list[i]; i++) {
		mdev = list[i];
		if (!xr_mbox_is_valid(mdev)) {
			ipc_err("invalid mdev handler");
			ret = -EINVAL;
			goto err_out;
		}

		if (xr_mdev_list_add(mdev)) {
			ipc_err("invalid mdev list");
			ret = -EINVAL;
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

	return ret;
}

static int xr_ipc_idev_init(struct device_node *node,
			struct xr_ipc *xdev,
			struct xr_ipc_device **idev)
{
	struct xr_ipc_device *dev;
	int ret = 0;
	int i;

	for (i = 0; i < xdev->ip_num; i++) {
		dev = kcalloc(1, sizeof(*dev), GFP_KERNEL);
		if (!dev) {
			ret = -ENOMEM;
			goto to_break;
		}

		dev->xdev = xdev;

		ret = of_property_read_u32_index(node, "max_ipc_vc_num",
					i, (u32 *)&(dev->vc_num));
		if (ret) {
			ipc_err("dts[%s-%d] no max_ipc_vc_num resources", node->name, i);
			ret = -ENODEV;
			goto free_dev;
		}

		xdev->total_vc_num += dev->vc_num;

		ret = of_property_read_string_index(node, "ip_name",
					i, (const char **)&dev->ip_name);
		if (ret) {
			ipc_err("dts[%s-%d] no ip_name resources", node->name, i);
			ret = -ENODEV;
			goto free_dev;
		}

		ret = of_property_read_u32(node, "capability",
					&(dev->capability));
		if (ret) {
			ipc_err("prop \"capability\" error %d", ret);
			ret = -ENODEV;
			goto free_dev;
		}

		ret = of_property_read_u32_index(node, "is_lpis",
					i, (u32 *)&(dev->need_wakeup));
		if (ret) {
			ipc_err("dts[%s-%d] no is_lpis resources", node->name, i);
			ret = -ENODEV;
			goto free_dev;
		}

		dev->ip_id = i;
		idev[i] = dev;
		continue;

free_dev:
		kfree(dev);
to_break:
		break;
	}

	if (!ret)
		return ret;

	while (i--)
		kfree(idev[i]);
	idev = NULL;

	return ret;
}

static const struct of_device_id xr_ipc_of_match[] = {
	{ .compatible = "xring,ipc", },
	{ },
};
MODULE_DEVICE_TABLE(of, xr_ipc_of_match);

static int xr_ipc_dev_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct device_node *node = adev->dev.of_node;
	struct xr_mbox_device **mdev_res = NULL;
	struct xr_ipc_device **idev = NULL;
	struct xr_ipc *xdev = NULL;
	struct device *pdev = NULL;
	int ret = 0;
	int i;

	pdev = &adev->dev;

	if (!node) {
		ipc_err("invalied ipc node");
		ret = -ENOENT;
		goto out;
	}

	if (!node->name) {
		ipc_err("invalied ipc node name");
		ret = -ENOENT;
		goto out;
	}

	xdev = kcalloc(1, sizeof(*xdev), GFP_KERNEL);
	if (!xdev) {
		ret = -ENOMEM;
		goto out;
	}

	ret = of_property_read_u32(node, "ipc_ip_num", &(xdev->ip_num));
	if (ret) {
		ipc_err("prop \"ip_num\" error %d", ret);
		ret = -ENODEV;
		goto free_xdev;
	}

	idev = kcalloc((xdev->ip_num + 1), sizeof(*idev), GFP_KERNEL);
	if (!idev) {
		ret = -ENOMEM;
		goto free_xdev;
	}
	idev[xdev->ip_num] = NULL;

	ret = xr_ipc_idev_init(node, xdev, idev);
	xdev->idev = idev;

	ipc_debug("ipc get %s", node->name);
	for (i = 0; i < xdev->ip_num; i++) {
		ret = mdev_idev_init(idev[i], node);
		if (ret)
			goto free_idev;
		idev[i]->xdev = xdev;
	}

	mdev_res = kcalloc((xdev->total_vc_num + 1),
			sizeof(*mdev_res), GFP_KERNEL);
	if (!mdev_res) {
		ret = -ENOMEM;
		goto free_idev;
	}
	mdev_res[xdev->total_vc_num] = NULL;

	ret = xr_ipc_mbox_get(node, idev, mdev_res);
	if (ret)
		goto free_mdevs;
	xdev->mdev_res = mdev_res;

	ret = xr_mbox_dev_reg(mdev_res);
	if (ret) {
		ipc_err("mdevs register failed");
		ret = -ENODEV;
		goto put_res;
	}

	ret = xr_mbox_device_init(&adev->dev, mdev_res);
	if (ret) {
		ipc_err("mdevs init failed");
		ret = -ENODEV;
		goto put_res;
	}

	ipc_creat_sysfs(pdev);
	amba_set_drvdata(adev, xdev);

	ipc_debug("xdev->total_vc_num = %d", xdev->total_vc_num);
	ipc_info("X-Ring [%s] are ready", node->name);

	return 0;

put_res:
	idev_mbox_free(idev);
	xr_mbox_put(mdev_res);
free_mdevs:
	kfree(mdev_res);
	xdev->mdev_res = NULL;
free_idev:
	kfree(idev);
free_xdev:
	kfree(xdev);
out:
	return ret;
}

static const struct dev_pm_ops xr_ipc_dev_pm_ops = {
	.suspend_late = xr_ipc_mdev_suspend,
	.resume_early = xr_ipc_mdev_resume,
};

static struct amba_id xr_ipc_ids[] = {
	{
		.id	= XR_IPC_ID,
		.mask	= XR_IPC_MASK,
	},
	{ 0, 0 },
};
MODULE_DEVICE_TABLE(amba, xr_ipc_ids);

static struct amba_driver xr_ipc_dev_driver = {
	.drv = {
		.name	= "xr_ipc",
		.owner	= THIS_MODULE,
		.pm	= &xr_ipc_dev_pm_ops,
		.of_match_table = xr_ipc_of_match,
	},
	.id_table = xr_ipc_ids,
	.probe	= xr_ipc_dev_probe,
	.remove	= xr_ipc_dev_remove,
};

int xr_ipc_dev_init(void)
{
	ipc_info("xr ipc device init!");

	return amba_driver_register(&xr_ipc_dev_driver);
}

void xr_ipc_dev_exit(void)
{
	amba_driver_unregister(&xr_ipc_dev_driver);
}
