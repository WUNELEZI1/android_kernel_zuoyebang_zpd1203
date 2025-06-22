/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_RPROC_H__
#define __XRING_RPROC_H__

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/firmware.h>
#include <linux/remoteproc.h>
#include <linux/miscdevice.h>
#include <linux/irqreturn.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#include "xrisp_tee_ca_api.h"
#include "xrisp_rproc_api.h"
#include "xrisp_clk_api.h"
#include "dfx_memory_layout.h"

extern pid_t g_provider_pid;
extern uint64_t boot_timeout_ms;
extern const char *xrisp_mcu_rate_prop_name[XRISP_MCU_CLK_RATE_MAX];
extern const char *xrisp_mcu_rate_prop_name[XRISP_MCU_CLK_RATE_MAX];
extern const char *xrisp_rate_prop_name[XRISP_CLK_RATE_MAX];

extern struct task_struct *provider_task;
extern struct task_struct *current_task;
extern struct cam_rproc *g_xrproc;
extern atomic_t rproc_stop;

#define XRISP_MAX_NAME_LEN (32)
#define BL31_SUPPROT_DMA_RESET
#define XRISP_TIME_SYNC_INTERVAL (180000)

#define ISP_RPSMG_WAIT_TIME_MS	    (1000)
#define ISP_SERVICE_WAIT_TIME_MS    (800)

#define SEC_SSMMU_THREAD_MAP_WAIT_TIME_MS  (3000)
#define SEC_SSMMU_THREAD_EXIT_WAIT_TIME_MS (15000)

enum cam_rproc_mem_region {
	RPROC_MEM_REGION_INVAL = 0,
	RPROC_MEM_REGION_START = 1,
	RPROC_MEM_REGION_VDEVBUFFER = RPROC_MEM_REGION_START,
	RPROC_MEM_REGION_VDEVRING0,
	RPROC_MEM_REGION_VDEVRING1,
	RPROC_MEM_REGION_BOOTADDR,
	RPROC_MEM_REGION_FWBUF,
	RPROC_MEM_REGION_IRAM,
	RPROC_MEM_REGION_NPUOCM,
	RPROC_MEM_REGION_RAMLOG,
	RPROC_MEM_REGION_DFXMEM,
	RPROC_MEM_REGION_MAX,
};


enum cam_rproc_mem_type {
	RPROC_MEM_TYPE_RESVERED,
	RPROC_MEM_TYPE_DMABUF,
	RPROC_MEM_TYPE_EMBEDDED,
	RPROC_MEM_TYPE_EXTRA,
	RPROC_MEM_TYPE_CMA,
	RPROC_MEM_TYPE_MAX
};

struct cam_device_address {
	dma_addr_t base;
	size_t size;
	enum cam_rproc_mem_region region;
	enum cam_rproc_mem_type type;
};

struct cam_rproc_mem {
	char name[XRISP_MAX_NAME_LEN];
	bool avail;
	struct cam_device_address da;
	phys_addr_t pa;
	phys_addr_t r82pa;
	dma_addr_t dma_handle;
	void __iomem *va;
	bool is_iomem;
	bool is_iommu;
	struct dentry *dentry;
	struct dma_buf *buf;
	struct sg_table *sgtlb;
	struct dma_buf_attachment *dma_attach;
	enum dma_data_direction dma_dir;
	struct iosys_map kmap;
};

struct cam_rproc_extra_mem {
	char name[XRISP_MAX_NAME_LEN];
	bool is_iommu;
	phys_addr_t pa;
	phys_addr_t r82pa;
	dma_addr_t da;
	size_t size;
};

struct cam_rproc_reserve_cma {
	enum cam_rproc_mem_region region;
	dma_addr_t dma_handle;
	void __iomem *va;
	size_t size;
};

#define XRISP_RPROC_RSC_ADDR		(0x87800000)
#define XRISP_RPROC_RSC_SIZE		(0x1000)
#define XRISP_RPROC_BOOT_ADDR		(0x80000000)
#define XRISP_RPROC_IRAM_ADDR		(0xe50c0000)
#define XRISP_RPROC_IRAM_DADDR		(0xf0000000)
#define XRISP_RPROC_IRAM_SIZE		(0x80000)
#define XRISP_RPROC_NPUOCM_ADDR		(0xe9820000)
#define XRISP_R82_NPUOCM_ADDR		(0x840000000)
#define XRISP_R82_NPUOCM_DEVICE_ADDR	(0xa8000000)
#define XRISP_RPROC_NPUOCM_SIZE		(0x440000)

#define XRISP_RPROC_DFXMEM_ADDR		(DFX_MEM_ISP_ADDR)
#define XRISP_R82_DFXMEM_ADDR		(DFX_MEM_ISP_ADDR)
#define XRISP_RPROC_DFXMEM_SIZE		(DFX_MEM_ISP_SIZE)

#define XRISP_RESERVE_IOVA_BASE1	(0x0)
#define XRISP_RESERVE_IOVA_SIZE1	(0x100000)

#define XRISP_RESERVE_IOVA_BASE2	(0x40000000)
#define XRISP_RESERVE_IOVA_SIZE2	(U64_MAX - XRISP_RESERVE_IOVA_BASE2)

enum xrisp_fw_resource_type {
	XRISP_RSC_VENDOR_START = RSC_VENDOR_START,
	XRISP_RSC_VENDOR_RAMLOG = RSC_VENDOR_START,
	XRISP_RSC_VENDOR_VDEV,
	XRISP_RSC_VENDOR_LAST,
	XRISP_RSC_VENDOR_END = RSC_VENDOR_END,
};

/* fw_rsc_config is not used, just follow nuttx's definition */
struct fw_rsc_config {
	uint32_t h2r_buf_size;
	uint32_t r2h_buf_size;
	uint32_t reserved[14];
} __packed;

struct fw_rsc_vdev_buffer {
	uint32_t da;
	uint32_t pa;
	uint32_t len;
	uint32_t reserved[2];
} __packed;

/* The feature bitmap for virtio rpmsg */
#define VIRTIO_RPMSG_F_NS	0 /* RP supports name service notifications */

struct fw_rsc_bd_info {
	uint32_t mcu_clock;
	uint32_t uart_clock;
	uint32_t timer_clock;
	uint64_t time_offset;
} __packed;

struct xrisp_rproc_rsc_table {
	/* keep `struct xrisp_rproc_rsc_table` same with nuttx */
	struct resource_table rsc_tbl_hdr;
	unsigned int offset[2];
	struct fw_rsc_hdr log_trace_hdr;
	struct fw_rsc_trace log_trace;
	struct fw_rsc_hdr rpmsg_vdev_hdr;
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;
	struct fw_rsc_config config;
	/* xrisp add vendor rsc start */
	struct fw_rsc_vdev_buffer rpmsg_vbuffer;
	/* xrisp add vendor rsc end */
	struct fw_rsc_bd_info bd_info;
} __aligned(8);

extern const struct xrisp_rproc_rsc_table rproc_rsc_table;

struct xrisp_rproc_ramlog {
	char name[XRISP_MAX_NAME_LEN];
	struct rproc_subdev subdev;
	void *priv;
	int (*ramlog_start)(void *priv);
	void (*ramlog_stop)(void *priv, bool crashed);
	bool avail;
	struct rproc *rproc;
	u32 rsc_offset;
	struct rproc_mem_entry *logmem;
};

struct xrisp_rproc_reg {
	char name[XRISP_MAX_NAME_LEN];
	struct rproc_subdev subdev;
	struct miscdevice regdev;
	void *priv;
	int (*regdev_start)(void *priv);
	void (*regdev_stop)(void *priv, bool crashed);
	bool avail;
	struct rproc *rproc;
	u32 rsc_offset;
	struct rproc_mem_entry *logmem;
	struct completion sig_kill;
	struct task_struct *user_task;
	struct wait_queue_head wait_q;
};

struct xrisp_rproc_mpu {
	char name[XRISP_MAX_NAME_LEN];
	struct rproc_subdev subdev;
	void *priv;
	int (*rproc_mpu_start)(void *priv);
	void (*rproc_mpu_stop)(void *priv, bool crashed);
	struct mutex lock;
	bool avail;
	struct rproc *rproc;
	u32 rsc_offset;
};

struct xrisp_rproc_vring {
	void *va;
	int len;
	u32 da;
	u32 align;
	int notifyid;
	struct xrisp_rproc_vdev *rvdev;
	struct virtqueue *vq;
};

struct xrisp_rproc_vdev {
	struct kref refcount;

	struct rproc_subdev subdev;
	struct device dev;

	unsigned int id;
	struct list_head node;
	struct cam_rproc *xrisp_rproc;
	struct xrisp_rproc_vring vring[RVDEV_NUM_VRINGS];
	u32 rsc_offset;
	u32 index;
};

struct rproc_mbox_work {
	struct work_struct work;
	enum xrisp_rproc_msg_type msg_type;
	uint32_t data[MBOX_MAX_DATA_REG];
	void *priv;
	callback_func_t callback;
};

struct rproc_mbox_msg_cb {
	void *priv;
	callback_func_t callback;
};

struct scene_rate_info {
	const char *scene_name;
	uint8_t mcu_rate_id;
	uint8_t crg_rate_id;
	uint32_t mcu_mask;
	uint32_t crg_mask;
};

struct cam_rproc {
	struct device *dev;
	void *mcu_base;
	struct rproc *rproc;
	struct dentry *dentry;
	struct dentry *rpmsg_dentry;
	const struct firmware *fw;
	struct xrisp_rproc_rsc_table rsc_table;
	size_t rsc_table_size;
	struct completion boot_complete;
	struct completion rpmsg_complete;
	struct completion ssmmu_map_th_exit;
	struct task_struct *ssmmu_map_th;
	atomic_t ssmmu_th_stop;
	int ssmmu_map_result;
	bool attach_mode;
	struct iommu_domain *domain;
	struct cam_rproc_mem mem[RPROC_MEM_REGION_MAX];
	struct cam_rproc_reserve_cma *reserve_cma;
	uint32_t cma_region_num;
	struct xrisp_rproc_ramlog ramlog;
	struct xrisp_rproc_reg regdev;
	struct xrisp_rproc_mpu rproc_mpu;
	struct list_head rvdevs;
	int nb_vdev;
	struct idr notifyids;
	int max_notifyid;
	bool skip_first_kick;
	struct workqueue_struct *workqueue;
	ktime_t kick_time;
	struct work_struct kick_work;
	struct regulator *mcu_rgltr;
	struct regulator *media2_rgltr;
	uint32_t scene;
	struct fw_rsc_bd_info bdinfo_list[XRISP_MCU_CLK_RATE_MAX];
	struct scene_rate_info scene_rate_info_list[XRISP_BOOT_SCENE_MAX];
	struct timer_list time_sync_timer;
	struct work_struct time_sync_work;
	struct wakeup_source *xrisp_wakelock;
};

extern struct cam_rproc *g_xrproc;

#define RPROC_STATUS(state)	(state == RPROC_OFFLINE ? "offline" :		\
				(state == RPROC_SUSPENDED) ? "suspend" :	\
				(state == RPROC_RUNNING) ? "running" :		\
				(state == RPROC_CRASHED) ? "crashed" :		\
				(state == RPROC_DELETED) ? "deleted" :		\
				(state == RPROC_ATTACHED) ? "attached" :	\
				(state == RPROC_DETACHED) ? "detached" :	\
				 "unkonw")

struct m2_register {
	uint32_t offset;
	uint32_t val;
#define M2_READ 0x0
#define M2_WRITE 0x1
#define M2_READ_AFTER_WRITE 0x2
#define M2_POLL 0x3
	uint32_t ops;
	uint32_t msdelay;
	char name[32];
};

struct xrisp_reg_map {
	uint32_t addr;
	uint32_t size;
	uint32_t is_atf_ctl;
	void __iomem *va;
	char name[32];
};

struct time_sync_msg {
	uint32_t msg;
	uint64_t time_offset;
};

#define XRISP_RPROC_IPC_ID IPC_VC_PERI_NS_AP_ISP_CAMERA

int rproc_debugfs_init(struct cam_rproc *xrisp_rproc);
void rproc_debugfs_exit(struct cam_rproc *xrisp_rproc);
struct dentry *rpmsg_debugfs_get_root(void);

int rproc_mem_debugfs_install(struct cam_rproc *xrisp_rproc, enum cam_rproc_mem_region region);
void rproc_mem_debugfs_uninstall(struct cam_rproc *xrisp_rproc, enum cam_rproc_mem_region region);

int xrisp_rproc_mbox_init(void);
void xrisp_rproc_mbox_exit(void);
int xrisp_rproc_mbox_send_sync(unsigned int *tx_buffer, unsigned int len);

int xrisp_power_on(struct cam_rproc *xrisp_rproc);
void xrisp_power_off(struct cam_rproc *xrisp_rproc);

int xrisp_rproc_set_attach_mode(bool attach_mode);

int xrisp_rproc_handle_ramlog(struct rproc *rproc, void *ptr,
			      int offset, int avail);

void xrisp_rproc_add_ramlog_subdev(struct cam_rproc *xrisp_rproc);
void xrisp_rproc_remove_ramlog_subdev(struct cam_rproc *xrisp_rproc);
int xrisp_rproc_add_reg_subdev(struct cam_rproc *xrisp_rproc);
void xrisp_rproc_remove_reg_subdev(struct cam_rproc *xrisp_rproc);
void xrisp_rproc_add_mpu_subdev(struct cam_rproc *xrisp_rproc);
void xrisp_rproc_remove_mpu_subdev(struct cam_rproc *xrisp_rproc);

__printf(2, 3)
struct rproc_mem_entry *
xrisp_rproc_find_carveout_by_name(struct rproc *rproc, const char *name, ...);

static inline struct xrisp_rproc_vdev *vdev_to_xrisp_rvdev(struct virtio_device *vdev)
{
	return container_of(vdev->dev.parent, struct xrisp_rproc_vdev, dev);
}

static inline struct cam_rproc *vdev_to_xrisp_rproc(struct virtio_device *vdev)
{
	struct xrisp_rproc_vdev *rvdev = vdev_to_xrisp_rvdev(vdev);

	return rvdev->xrisp_rproc;
}

int xrisp_rproc_handle_vdev(struct rproc *rproc, void *ptr,
			     int offset, int avail);

int xrisp_rproc_add_virtio_dev(struct xrisp_rproc_vdev *rvdev, int id);
int xrisp_rproc_remove_virtio_dev(struct device *dev, void *data);
void xrisp_rproc_free_vring(struct xrisp_rproc_vring *rvring);

irqreturn_t xrisp_rproc_vq_interrupt(struct cam_rproc *xrisp_rproc, int notifyid);
void xrisp_rproc_vdev_release(struct kref *ref);

void xrisp_mcu_dma_dereset(void);
void xrisp_ispmodule_dereset(void);
void xrisp_m2crg_ip_reset(void);
void xrisp_m2crg_ip_dereset(void);

void pr_rsc_table(struct cam_rproc *xrisp_rproc);

int xrisp_rproc_ocm_map(unsigned long pa, size_t len);
void xrisp_rproc_ocm_unmap(unsigned long va, size_t len);
#endif
