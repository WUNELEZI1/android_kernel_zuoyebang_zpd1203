/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __HPC_INTERNEL__
#define __HPC_INTERNEL__

#include <linux/types.h>
#include <linux/virtio.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/irqreturn.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/virtio.h>
#include <linux/regulator/consumer.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/tee_drv.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <linux/version.h>

#ifndef KERNEL_VERSION_LESS_THAN_6_6_0
#define KERNEL_VERSION_LESS_THAN_6_6_0 \
	(LINUX_VERSION_MAJOR < 6 || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL < 6))
#endif

#ifndef KERNEL_VERSION_GREATER_THAN_6_6_0
#define KERNEL_VERSION_GREATER_THAN_6_6_0 \
(LINUX_VERSION_MAJOR > 6 || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL > 6) || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL == 6 && LINUX_VERSION_SUBLEVEL > 0))
#endif

#if KERNEL_VERSION_GREATER_THAN_6_6_0
#include <linux/iosys-map.h>
#endif

#define HPC_VRING_NUM_MAX          (2)
#define LOG_HEADER_ADDR_OFFSET (0x400UL)

#if IS_ENABLED(CONFIG_XRING_DEBUG)
#define HPC_LOG_LEVEL_ASSERT		0
#define HPC_LOG_LEVEL_ERROR		1
#define HPC_LOG_LEVEL_WARN		2
#define HPC_LOG_LEVEL_INFO		3
#define HPC_LOG_LEVEL_DEBUG		4
#define HPC_LOG_LEVEL_MAX		5
#define HPC_LOG_LEVEL_DEF		HPC_LOG_LEVEL_INFO

extern int hpc_log_level;
#define hpcerr(fmt, ...)                                                 \
do {                                                                     \
	if (hpc_log_level >= (int)HPC_LOG_LEVEL_ERROR)                          \
		pr_err("HPC-DEV [%s][E]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcwarn(fmt, ...)                                                \
do {                                                                     \
	if (hpc_log_level >= (int)HPC_LOG_LEVEL_WARN)                           \
		pr_warn("HPC-DEV [%s][W]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcinfo(fmt, ...)                                                \
do {                                                                     \
	if (hpc_log_level >= (int)HPC_LOG_LEVEL_INFO)                           \
		pr_info("HPC-DEV [%s][I]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcdbg(fmt, ...)                                                \
do {                                                                     \
	if (hpc_log_level >= (int)HPC_LOG_LEVEL_DEBUG)                           \
		pr_debug("HPC-DEV [%s][D]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)
#else
#define hpcerr(fmt, ...)                                                 \
do {                                                                     \
	pr_err("HPC-DEV [%s][E]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcwarn(fmt, ...)                                                \
do {                                                                     \
	pr_warn("HPC-DEV [%s][W]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcinfo(fmt, ...)                                                \
do {                                                                     \
	pr_info("HPC-DEV [%s][I]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define hpcdbg(fmt, ...)                                                \
do {                                                                     \
	pr_debug("HPC-DEV [%s][D]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)
#endif

#define HPC_IOMMU_NAME_SIZE        32

#define HEAP_IS_NOT_ENOUGH         ((void *)-1)

typedef int (*hpc_notifier_cb)(void *arg, void *data);

struct hpc_virtio_device;

/*
 * This is the hpc device boot memory type.
 * HBM_IMG_TYPE : hpc boot image memory
 * HBM_RVQUEUE_TYPE: hpc ree vqueue memory
 * HBM_RVRING_TYPE : hpc ree vring memory
 * HBM_RS_TYPE : hpc boot ree share memory
 * HBM_NCAL_TYPE : hpc vip core auto link memory
 */
enum hpc_boot_mem_type_e {
	HBM_IMG_TYPE = 0,
	HBM_RVQUEUE_TYPE,
	HBM_RVRING_TYPE,
	HBM_RS_TYPE,
	HBM_NCAL_TYPE,
	HBM_HMDR_TYPE,
	HBM_TYPE_MAX,
};

enum hpc_event_type_e {
	HE_LOG_FLUSH               = 0x0,
	HE_LOG_LEVEL_SET           = 0x1,
	HE_AINR_DUMP_REF           = 0x2,
	HE_CANCEL_TPID_ALL         = 0x3,
	HE_TYPE_MAX                = 0x100,
};

enum hpc_hmdr_status_e {
	HMDR_NV_DISENABLE          = 0x0,
	HMDR_NV_ENABLE             = 0x1,
};

enum hpc_aon_status_e {
	HAON_DISENABLE             = 0x0,
	HAON_ENABLE                = 0x1,
};

//hpc exception/normal log disk drop
enum hpc_log_drop_e {
	HMDR_NORM_LOG_DROP          = 0x0,
	HMDR_EXCE_LOG_DROP          = 0x1,
};

struct hpc_log_flush_msg {
	u32 readp_offset;
	u32 wirtep_offset;
};

struct hpc_log_level_msg {
	u32 log_level;
};

struct hpc_cancel_pidall_msg {
	u32 tpid;
	u32 status;
};

struct hpc_log_info {
	enum hpc_hmdr_status_e hmdr_enable;
	uint32_t log_level;
	uint32_t writep;
	uint32_t readp;
	enum hpc_hmdr_status_e wdt_enable;
	enum hpc_log_drop_e log_drop;
};

struct haon_log_info {
	u32 check_code1;
	u32 check_code2;
	u32 log_level;
	u32 writep;
	u32 readp;
};

struct hpc_ainr_dump_ref_msg {
	u32 frame_num;
	u32 buf_id;
};

struct hpc_event_msg {
	u32 len;
	enum hpc_event_type_e type;
	union {
		struct hpc_log_flush_msg hlfm;
		struct hpc_log_level_msg hllm;
		struct hpc_ainr_dump_ref_msg hadfm;
		struct hpc_cancel_pidall_msg hcpm;
	} msg;
};

struct hpc_vdev_share_para {
	u32 id;
	u32 notifyid;
	u32 dfeatures;
	u32 gfeatures;
	u32 config_len;
	u8 status;
	u8 num_vrings;
	u8 reserve[2];
};

struct hpc_vring_share_para {
	u32 align;
	u32 num;
	u32 notifyid;
	u32 size;
	u32 da;
	u32 pa;
	u64 va;
};

struct hpc_vq_share_para {
	u32 size;
	u32 da;
	u32 pa;
	u64 va;
};

struct hpc_virtio_share_para {
	struct hpc_vdev_share_para hvsp;
	struct hpc_vring_share_para hvrsp[HPC_VRING_NUM_MAX];
	struct hpc_vq_share_para hvqsp;
};

struct hpc_vring {
	u32 notifyid;
	void *priv;
	struct virtqueue *vq;
};

struct hpc_virtio_ops {
	bool (*kick)(struct hpc_virtio_device *hvdev, int vqid);
};

struct hpc_virtio_device {
	void *priv;
	struct hpc_virtio_share_para *hpc_vsp;
	struct hpc_virtio_ops *ops;
	struct virtio_device *vdev;
	const char *name;
	int id;
	atomic_t power;
	struct mutex lock;
	struct ida notifyids;
	struct device dev;
	struct hpc_vring vring[HPC_VRING_NUM_MAX];
};

struct hpc_mem_info {
	int fd;
	u32 size;
	int prot;
	u32 da;
	u64 pa;
	unsigned long iova;
	void *va;
};

struct hpc_dma_buf {
	int fd;
	size_t size;
	void *vaddr;
	struct dma_buf *dmabuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
#if KERNEL_VERSION_LESS_THAN_6_6_0
	struct dma_buf_map map;
#else
	struct iosys_map map;
#endif
	struct list_head node;
};

struct hpc_iova_region {
	u32 iova;
	size_t size;
};

enum hpc_mem_buf_type_e {
	HPC_MEM_BUF_DMA = 0,
	HPC_MEM_BUF_OCM,
	HPC_MEM_BUF_TYPE_MAX,
};

struct hpc_rpmsg_mem_map_info {
	bool flag;
	bool flag4k;
	int prot;
	int sid;
	int ssid;
	u32 da;
};
struct hpc_rpmsg_mem_info {
	int heap_status;   //0-heap size is enought，1-heap size is not enought
	int fd;
	u32 size;
	enum hpc_mem_buf_type_e type;
	int idr;
	struct hpc_rpmsg_mem_map_info mcu;
	struct hpc_rpmsg_mem_map_info vip;
};

struct hpc_rpmsg_mem_node {
	struct hpc_dma_buf *buf;
	struct ocm_buffer *ob;
	struct hpc_rpmsg_mem_info info;
	struct list_head node;
};
struct hpc_mem_heap {
	struct platform_device *pdev;
	struct gen_pool *pool;
	struct iommu_domain *domain;
	u32 ref;
	u32 map_ref;
	u32 al_ref;
	u32 sid;
	u32 ssid;
	u32 dynamic;
	pid_t pid;
	struct hpc_iova_region region;
	struct mutex mutex;
	struct list_head node;
};

struct hpc_mem_heap_ctrl {
	struct mutex dyn_heap_lock;
	struct platform_device *pdev;
	struct miscdevice miscdev;
	struct idr idr_head;
};

struct hpc_mdr_device {
	struct hpc_device *hdev;
	struct mbox_client cl;
	struct mbox_chan *tx_ch;
	struct mbox_chan *rx_ch;
	struct miscdevice miscdev;
	struct device dev;
	struct list_head nh;
	struct mdr_module_ops mdr_npu_ops;
	struct mdr_register_module_result mdr_retinfo;
	struct fasync_struct *fasync_q;
	struct hpc_ainr_dump_ref_msg ainr_dump_ref_msg;
	wait_queue_head_t wait;
	u32    ainr_dump_msg_ready;
	struct hpc_log_info *p_hli; // pointer to rs share mem, -> rs_mem_addr + LOG_HEADER_ADDR_OFFSET
	struct hpc_log_info g_hli;
};

struct hpc_aon_device {
	struct hpc_device *hdev;
	enum hpc_aon_status_e aon_enable;
	struct dma_buf *imgbuf_src;
	struct dma_buf *imgbuf_run;
	struct dma_heap *dma_sysheap;
	struct dma_buf *imgshm_dmabuf;
#if KERNEL_VERSION_LESS_THAN_6_6_0
	struct dma_buf_map imgshm_map;
#else
	struct iosys_map imgshm_map;
#endif
	uint32_t imgsrc_sfd;
	uint32_t cipherimg_size;
	uint32_t imgrun_sfd;
	uint32_t imgrun_size;
	void *imgshm_va;
	uint32_t imgshm_size;
	struct tee_shm *imgshm_tee;
	struct miscdevice miscdev;
	struct device dev;
};

struct hpc_device {
	struct hpc_mem_info hbm[HBM_TYPE_MAX];
	struct dma_heap *system_heap;
	struct dma_buf *img_buf;
	struct platform_device *pdev;
	struct hpc_virtio_device *hvdev;
	struct hpc_mdr_device *hmdev;
	struct hpc_aon_device *hadev;
	struct platform_device *hcdev_pdev;
	struct platform_device *hmp_pdev;
	struct regulator *regulator;
	struct mbox_chan *tx_ch;
	struct mbox_chan *rx_ch;
	struct workqueue_struct *wq;
	struct mbox_client cl;
	struct work_struct rpc_rx_work;
	u32 hbmsz;
	u32 hbm_info_size;
	struct hpc_virtio_share_para table;
	atomic_t power;
	struct mutex lock;
	struct completion boot;
	dma_addr_t dma_handle;
	void *dma_va;
	wait_queue_head_t cancel_all_wait;
	bool cancel_all_flag;
};

/* hpc_virtio.c */
struct hpc_virtio_device *hpc_vdev_alloc(struct device *dev, const char *name,
	struct hpc_virtio_ops *ops, void *priv);
void hpc_vdev_free(struct hpc_virtio_device *hvdev);
int hpc_vdev_boot(struct hpc_virtio_device *hvdev);
void hpc_vdev_shutdown(struct hpc_virtio_device *hvdev);
irqreturn_t hpc_vq_interrupt(struct hpc_virtio_device *hvdev, int notifyid);

/* hpc_boot.c */
int hdev_boot(struct hpc_device *hdev);
int hdev_shutdown(struct hpc_device *hdev);
int hdev_shutdown_atomic(struct hpc_device *hdev);
int hdev_resource_release(struct hpc_device *hdev);
bool hdev_booted(struct hpc_device *hdev);
static inline void hdev_rpmsg_complete(struct hpc_device *hdev)
{
	complete_all(&hdev->boot);
}
static inline void hdev_boot_completion_reinit(struct hpc_device *hdev)
{
	reinit_completion(&hdev->boot);
}

/* hpc_atf.c */
int hpc_smc_set_boot_info(struct hpc_mem_info *hbm, int num);
int hpc_smc_boot(void);
int hpc_smc_shutdown(void);

/* hpc_mem.c */
int hpc_dma_buf_map_vaddr(struct hpc_dma_buf *buf);
void hpc_dma_buf_unmap_vaddr(struct hpc_dma_buf *buf);
int hpc_dma_buf_clean(struct hpc_dma_buf *buf);
int hpc_mem_unmap(struct hpc_mem_heap *heap, unsigned long iova, size_t size);
int hpc_mem_map_al_iova(u64 pa, size_t size, unsigned long iova, int prot);
int hpc_mem_unmap_al_iova(unsigned long iova, size_t size);
void hpc_dma_free(struct hpc_device *hdev);
int hpc_dma_alloc(struct hpc_device *hdev);
void hpc_mem_heap_check_and_free(void);
int hrpdev_dma_buf_map(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem);
int hrpdev_dma_buf_unmap(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem);
int hrpdev_ocm_buf_map(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem);
int hrpdev_ocm_buf_unmap(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem);

void set_npu_excp_true(void);
void set_npu_excp_false(void);
bool is_npu_excp_true(void);

/* hpc_mdr.c */
int hmdr_init(struct hpc_device *hdev);
int hmdr_exit(struct hpc_device *hdev);
int hpc_event_notifier_register(struct hpc_device *hdev, u32 event,
		void *arg, hpc_notifier_cb cb);
int hpc_event_notifier_unregister(struct hpc_device *hdev, u32 event);
int hpc_send_event(struct hpc_device *hdev, void *msg);

/* hpc tee */
#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
int hree_ca_init(struct hpc_mem_info *mem, u32 size);
int hree_ca_exit(void);
int hree_ca_boot(int boot_flag);
int hree_ca_shutdown(void);
int hree_ca_load_aonfw(struct hpc_aon_device *hadev);
#else
static inline int hree_ca_init(struct hpc_mem_info *mem, u32 size)
{
	return 0;
}
static inline int hree_ca_exit(void)
{
	return 0;
}
static inline int hree_ca_boot(int boot_flag)
{
	return -ENODEV;
}
static inline int hree_ca_shutdown(void)
{
	return -ENODEV;
}
static inline int hree_ca_load_aonfw(struct hpc_aon_device *hadev)
{
	return -ENODEV;
}
#endif

/* hpc aon */
int haon_init(struct hpc_device *hdev);
int haon_exit(struct hpc_device *hdev);

#endif
