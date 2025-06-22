/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_MBOX_H__
#define __XR_MBOX_H__

#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/atomic.h>
#include <linux/ktime.h>

#include <soc/xring/ipc/xr_ipc.h>

#define MDEV_SYNC_SENDING	(1 << 0)
#define MDEV_ASYNC_ENQUEUE	(1 << 1)
#define MDEV_RESP_SENDING	(1 << 2)
#define MDEV_SEND_RECV		(1 << 3)
#define MDEV_DEACTIVATED	(1 << 4)

#define IPC_VC_NUM_PER_IP	(32)
#define GET_IP_ID(ipc_vc)	(ipc_vc / IPC_VC_NUM_PER_IP)

#define IPC_PRINT_LOG_SIZE	(256)
#define IPC_PER_LOG_SIZE	(32)

#define IPC_MASK(n)		(1U << (n))

#define IPC_MBOX_MANUAL_ACK	(0)
#define IPC_MBOX_AUTO_ACK	(1)
#define IPC_MBOX_SEND_MSG	IPC_MBOX_MANUAL_ACK
#define IPC_MBOX_SEND_RESP	IPC_MBOX_AUTO_ACK

#define IPC_RX_FIFO_SIZE	(1)

enum ipc_irq_status {
	ACK_IRQ = 0,
	RECV_IRQ,
	RESP_IRQ,
	INVALID_IRQ,
};

struct xr_mbox {
	int				mdev_index;
	struct xr_mbox_device		*mbox;
	struct notifier_block		*nb;
};

struct xr_ipc {
	int				total_vc_num;
	int				ip_num;

	struct xr_ipc_device		**idev;
	struct xr_mbox_device		**mdev_res;

	struct atomic_notifier_head	hook_nb;
};

struct xr_mbox_rx_msg {
	unsigned int rx_buff[MBOX_MAX_DATA_REG];
	int cur_task;
};

struct xr_mbox_rx_fifo {
	struct xr_mbox_rx_msg rx_msg[IPC_RX_FIFO_SIZE];
	int read_index;
	int write_index;
	int count;
};

struct xr_mbox_device {
	const char		*name;
	struct list_head	node;
	int			configured;
	unsigned int		ipc_id;
	void			*priv;
	struct xr_mbox_dev_ops	*ops;
	int			cur_irq;
	unsigned int		status;
	spinlock_t		status_lock;
	mbox_irq_handler_t	irq_handler;
	unsigned int		capability;

	struct device		*dev;

	/* tx attributes */
	spinlock_t		fifo_lock;
	struct kfifo		fifo;
	struct mutex		dev_lock;
	struct completion	ack_complete;
	spinlock_t		ack_complete_lock;
	int			ack_completed;
	struct completion	resp_complete;
	spinlock_t		resp_complete_lock;
	int			resp_completed;
	struct xr_mbox_task	*tx_task;
	atomic_t		async_reset_flag;
	spinlock_t		timeout_num_lock;
	unsigned int		timeout_num;

	/* rx attributes */
	unsigned int		*rx_buffer;
	unsigned int		*resp_buffer;
	struct xr_mbox_rx_fifo	rx_fifo;
	spinlock_t		rx_lock;
	struct tasklet_struct	rx_bh;
	struct task_struct	*tx_kthread;
	struct atomic_notifier_head notifier;
	fast_ack_callback_t	recv_cb;
	void			*recv_arg;
	ktime_t			fast_ack_time;

	wait_queue_head_t	tx_wait;

	/* timeout statistics */
	u64			max_send_sync_time;
	u64			ave_send_sync_time;
	u64			ave_send_sync_num;
	u64			max_send_recv_time;
	u64			ave_send_recv_time;
	u64			ave_send_recv_num;
	atomic_t		recv_num;
	atomic_t		async_ack_num;
	atomic_t		async_send_num;
	atomic_t		async_trigger_num;
};

struct xr_ipc_device {
	void __iomem *base;
	void __iomem *crg_addr;

	unsigned int *buf_pool;
	unsigned int capability;
	struct xr_ipc *xdev;
	int ip_id;
	int vc_num;
	char *ip_name;
	int need_wakeup;

	int ack_timeout;
	int resp_timeout;

	int irq;

	/* Used for scenarios where multiple channels use the same mailbox */
	struct mutex mbox_lock;
};

struct xr_mbox_device_priv {
	int vc_id;
	int ip_id;
	int src;
	int dst;
	int ack_timeout;
	int resp_timeout;
	int fast_ack;
	int com_mbox;
	/* tx attributes */
	char *dst_chn_name;
	int dst_chn_id;
	int tx_mbox_id;

	/* rx attributes */
	char *src_chn_name;
	int src_chn_id;
	int rx_mbox_id;

	int capability;
	int used;
	unsigned int fifo_size;

	struct xr_ipc_device *idev;
};

struct xr_mbox_dev_ops {
	/* communication */
	int	(*send)(struct xr_mbox_device *mdev,
			unsigned int *msg,
			unsigned int len);
	int	(*resp)(struct xr_mbox_device *mdev,
			unsigned int *msg,
			unsigned int len);
	void	(*ack)(struct xr_mbox_device *mdev);
	void	(*release)(struct xr_mbox_device *mdev);
	int	(*recv)(struct xr_mbox_device *mdev, unsigned int **msg);
	void	(*read)(struct xr_mbox_device *mdev, unsigned int *msg);
	unsigned int	(*get_ack_timeout)(struct xr_mbox_device *mdev);
	unsigned int	(*get_resp_timeout)(struct xr_mbox_device *mdev);
	unsigned int	(*get_fifo_size)(struct xr_mbox_device *mdev);
	unsigned int	(*msta)(struct xr_mbox_device *mdev);

	/* irq */
	int	(*request_irq)(struct xr_mbox_device *mdev,
				irq_handler_t handler, void *p);
	void	(*free_irq)(struct xr_mbox_device *mdev);
	void	(*enable_irq)(struct xr_mbox_device *mdev);
	void	(*disable_irq)(struct xr_mbox_device *mdev);


	int	(*is_msg)(struct xr_mbox_device *mdev);
	int	(*is_ack)(struct xr_mbox_device *mdev);
	void	(*clr_ack)(struct xr_mbox_device *mdev);
	int	(*ensure_channel)(struct xr_mbox_device *mdev);
	int	(*irq_sta)(struct xr_mbox_device *mdev);

	// /* mntn */
	void	(*dump_status)(struct xr_mbox_device *mdev);
};

extern int xr_mdev_list_add(struct xr_mbox_device *mdev);

extern void xr_mbox_device_activate(struct xr_mbox_device **mdevs);
extern void xr_mbox_device_deactivate(struct xr_mbox_device **mdevs);

extern int xr_mbox_device_init(struct device *parent, struct xr_mbox_device **mdevs);
extern int xr_mbox_device_unregister(struct xr_mbox_device **list);

extern irqreturn_t ipc_interrupt(int irq, void *p);

struct rproc_list *vc_id_get(void);
void vc_id_free(struct rproc_list *list);

unsigned int xr_ipc_capa_get(void);
void xr_ipc_pr_ack_timeout(struct xr_mbox_device *mdev);

void xr_mdev_hw_send(struct xr_mbox_device *mdev, u32 *msg, u32 len);
void xr_mdev_send_set(struct xr_mbox_device *mdev, int ack_mode);

int xr_ipc_send_async(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
			ack_callback_t cb, void *arg);
int xr_ipc_async_fifo_reset(unsigned int ipc_id);
int xr_ipc_send_sync(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len);
int xr_ipc_send_resp(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len);
int xr_ipc_send_recv(unsigned int ipc_id, unsigned int *tx_buffer, unsigned int len,
			unsigned int *rx_buffer, unsigned int rx_buffer_len);
int xr_ipc_recv_register(unsigned int ipc_id, struct notifier_block *nb);
int xr_ipc_recv_unregister(unsigned int ipc_id, struct notifier_block *nb);
int xr_ipc_fast_ack_recv_register(unsigned int ipc_id, fast_ack_callback_t cb, void *arg);
int xr_ipc_fast_ack_recv_unregister(unsigned int ipc_id);

void xr_ipc_dump_async_cnt(unsigned int ipc_id);
void xr_ipc_reset_async_cnt(unsigned int ipc_id);

struct xr_ipc *xr_ipc_get_xdev(void);

#endif /* __XR_MBOX_H__ */
