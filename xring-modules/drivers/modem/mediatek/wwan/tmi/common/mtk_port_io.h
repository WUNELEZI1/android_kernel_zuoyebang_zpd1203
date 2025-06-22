/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_PORT_IO_H__
#define __MTK_PORT_IO_H__

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include "mtk_bm.h"
#include "mtk_debug.h"
#include "mtk_port.h"

/* Default rx buffer */
#define MTK_RX_BUF_SIZE			(1024 * 1024)
#define MTK_RX_BUF_MAX_SIZE		(2 * 1024 * 1024)

/* CDEV and Proprietary port minor region:0~127
 */
#define MTK_CDEV_MINOR_BASE			(0)
#define MTK_CDEV_MAX_NUM			(128)

#define MTK_DFLT_DUMP_RX_BUDGET			(16)

#define MTK_IOC_MAGIC					'M'
#define MTK_IOC_SET_RX_BUF_SIZE			_IOW(MTK_IOC_MAGIC, 0xA0, unsigned int)
#define MTK_IOC_ALLOW_DROP_PACKET			_IO(MTK_IOC_MAGIC, 0xA1)
#define MTK_IOC_FORBID_DROP_PACKET			_IO(MTK_IOC_MAGIC, 0xA2)
/* This IOCTL is used to retrieve the wMaxCommand for the device,
 * defining the message limit for both reading and writing.
 */
#define MTK_IOCTL_WDM_MAX_COMMAND			_IOR('H', 0xA0, unsigned short)

extern struct mutex port_mngr_grp_mtx;

struct port_ops {
	int (*init)(struct mtk_port *port);
	int (*exit)(struct mtk_port *port);
	int (*reset)(struct mtk_port *port);
	int (*enable)(struct mtk_port *port);
	int (*disable)(struct mtk_port *port);
	int (*recv)(struct mtk_port *port, struct sk_buff *skb);
	int (*match)(struct mtk_port *port, struct sk_buff *skb);
	void (*dump)(struct mtk_port *port);
};

union user_buf {
	void __user *ubuf;
	void *kbuf;
};

int mtk_port_io_init(void);
void mtk_port_io_exit(void);

int mtk_port_search(char *name, dev_t *result, unsigned int max_cnt);
int mtk_port_open(dev_t devt, int flag);
int mtk_port_close(dev_t devt);
int mtk_port_write(dev_t devt, void *buf, unsigned int len);
int mtk_port_read(dev_t devt, void *buf, unsigned int len);
long mtk_port_ioctl(dev_t devt, unsigned int cmd, unsigned long arg);

void *mtk_port_internal_open(struct mtk_md_dev *mdev, char *name, int flag);
int mtk_port_internal_close(void *i_port);
int mtk_port_internal_write(void *i_port, struct sk_buff *skb);
long mtk_port_internal_ioctl(void *i_port, unsigned int cmd, unsigned long arg);
void mtk_port_internal_recv_register(void *i_port,
				     int (*cb)(void *priv, struct sk_buff *skb),
				     void *arg);

int mtk_port_send_brom_cmd(struct mtk_port *port, unsigned char cmd);
int mtk_port_read_brom_cmd_ack(struct mtk_port *port, unsigned char expected_cmd,
			       unsigned int sleep_time);

static inline struct sk_buff *mtk_port_alloc_tx_skb(struct mtk_port *port)
{
	struct mtk_ctrl_blk *ctrl_blk = port->port_mngr->ctrl_blk;
	struct sk_buff *skb;

	if (port->tx_mtu > VQ_MTU_3_5K)
		skb = mtk_bm_alloc(ctrl_blk->bm_pool_63K);
	else
		skb = mtk_bm_alloc(ctrl_blk->bm_pool);

	return skb;
}

static inline void mtk_port_free_tx_skb(struct mtk_port *port, struct sk_buff *skb)
{
	switch (port->info.type) {
	case PORT_TYPE_CHAR:
	case PORT_TYPE_PROPRIETARY:
	case PORT_TYPE_WWAN:
	case PORT_TYPE_RELAYFS:
		if (port->tx_mtu > VQ_MTU_3_5K)
			mtk_bm_free(port->port_mngr->ctrl_blk->bm_pool_63K, skb);
		else
			mtk_bm_free(port->port_mngr->ctrl_blk->bm_pool, skb);
		break;
	case PORT_TYPE_INTERNAL:
		dev_kfree_skb_any(skb);
		break;
	default:
		dev_kfree_skb_any(skb);
		break;
	}
}

static inline void mtk_port_free_rx_skb(struct mtk_port *port, struct sk_buff *skb)
{
	if (!port)
		dev_kfree_skb_any(skb);
	else if (port->rx_mtu > VQ_MTU_3_5K)
		mtk_bm_free(port->port_mngr->ctrl_blk->bm_pool_63K, skb);
	else
		mtk_bm_free(port->port_mngr->ctrl_blk->bm_pool, skb);
}

static inline void mtk_port_rx_skb_dump(struct mtk_port *port)
{
	struct sk_buff *skb;
	unsigned long flags;

	if (!(port->info.flags & PORT_F_RAW_DATA)) {
		spin_lock_irqsave(&port->rx_skb_dump_list.lock, flags);
		while ((skb = __skb_dequeue(&port->rx_skb_dump_list))) {
			MTK_HEX_DUMP(port->port_mngr->ctrl_blk->mdev,
					 MTK_DBG_PORT, MTK_MEMLOG_RG_9,
				     "Dumping rx data header which user has read:",
				     skb->data - sizeof(struct mtk_ccci_header),
				     sizeof(struct mtk_ccci_header));
			mtk_port_free_rx_skb(port, skb);
		}
		port->dump_rx_budget = 0;
		spin_unlock_irqrestore(&port->rx_skb_dump_list.lock, flags);
	}
}

#endif /* __MTK_PORT_IO_H__ */
