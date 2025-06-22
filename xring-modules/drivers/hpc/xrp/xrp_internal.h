/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */



#ifndef XRP_INTERNAL_H
#define XRP_INTERNAL_H

#include <linux/completion.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/iommu.h>
#include "soc/xring/xring_smmu_wrapper.h"
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <uapi/linux/dma-heap.h>
#include "xrp_kernel_defs.h"
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <linux/mailbox_client.h>
#include <linux/platform_device.h>
#include <linux/irqreturn.h>
#include "xrp_kernel_defs.h"
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <linux/mailbox_client.h>
#include <linux/platform_device.h>
#include <linux/irqreturn.h>
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

#define xrperr(fmt, ...)         \
	pr_err("XRP-DEV [%s][E]: " fmt, __func__, ##__VA_ARGS__)

#define xrpwarn(fmt, ...)        \
	pr_err("XRP-DEV [%s][W]: " fmt, __func__, ##__VA_ARGS__)

#define xrpinfo(fmt, ...)        \
	pr_err("XRP-DEV [%s][I]: " fmt, __func__, ##__VA_ARGS__)

#define xrpdbg(fmt, ...)        \
	pr_debug("XRP-DEV [%s][D]: " fmt, __func__, ##__VA_ARGS__)

#define VDSP_BOOT_ADDR (0xE9200000)
#define VDSP_SRAM_ADDR (0x10000000)
#define VDSP_SROM_ADDR (0x50000000)
#define VDSP_SRAM_SIZE (2*1024*1024)
#define VDSP_SROM_SIZE (3*1024*1024)

#define VDSP_MDR_ADDR (0x102B0000)
#define VDSP_COMM (0x10200000)
#define VDSP_SRAM (0x10000000)
#define VDSP_SROM (0x50000000)
#define SHAKEHAND_OFFSET 0x2000

#define OPEN_MDR_FRAME 1

struct device;
struct firmware;
struct xrp_hw_ops;
struct xrp_allocation_pool;

struct xrp_comm {
	struct mutex lock;
	void __iomem *comm;
	struct completion completion;
	u32 priority;
};

struct xvp {
	struct device *dev;
	const char *firmware_name;
	const struct firmware *firmware;
	struct miscdevice miscdev;
	const struct xrp_hw_ops *hw_ops;
	void *hw_arg;
	unsigned int n_queues;

	u32 *queue_priority;
	struct xrp_comm *queue;
	struct xrp_comm **queue_ordered;
	void __iomem *comm;
	phys_addr_t pmem;
	phys_addr_t comm_phys;
	phys_addr_t shared_size;
	atomic_t reboot_cycle;
	atomic_t reboot_cycle_complete;

	bool host_irq_mode;
	bool if_need_sync;
	struct xrp_allocation_pool *pool;
	bool off;
	int nodeid;
	bool direct_mapping;
	struct iommu_domain *domain;
	void *vdsp_sysram_vaddr;
	void *vdsp_sysrom_vaddr;
};

//xrp_task.c结构体
enum mailbox_msg_type {
	DEFAULT_MSG_TYPE,
	TASK_RUN_SIT,
	SET_LOG_LEV_SIT,
	FLUSH_LOG_SIT,
	QUERY_STATUS_SIT,
	MSG_TYPE_MAX,
};

struct xrp_task_info {
	struct xrp_task_info_l task;
	struct completion done;
};

struct mbx_send_statu {
	struct completion mbx_done;
};

struct queue_node {
	struct xrp_task_info task_info;
	struct list_head list;
	struct mbx_send_statu mbx_send_info;
};

struct user_wait_data {
	uint32_t msg_id;
	uint32_t priority;
	uint32_t wait_time_ms;
};

struct vdsp_mbox {
	struct platform_device *pdev;
	struct mbox_client cl;
	struct mbox_chan *tx_ch;
	struct mbox_chan *rx_ch;
};

struct hpc_iova_region {
	u32 iova;
	size_t size;
};


struct vdsp_task_priority_queue {
	struct mutex lock;
	struct list_head queue;
	struct list_head *cursor;
	int priority;
};


struct xrp_dma_buf {
	int fd;
	size_t size;
	int len;
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
struct xrp_mem_node {
	pid_t mem_pid;
	struct xrp_dma_buf *buf;
	struct vdsp_memmap_info info;
	struct list_head node;
};


struct xrp_task_device {
	struct cdev cdev;
	struct device *dev;
	struct miscdevice miscdev;
	int num_queues;
	struct vdsp_task_priority_queue *vdsp_taskq;
	int *priorities;
	struct task_struct *thread;
	void *vdsp_comm_vaddr_base;
	dma_addr_t comm_dma_handle;
	size_t comm_size;
	bool if_use_mbox;
	struct vdsp_mbox *vmbox;
	struct iommu_domain *domain;
	void *vdsp_sysram_mem_buf;
	void *vdsp_sysrom_mem_buf;
	struct gen_pool *pool;
	struct hpc_iova_region region;
	bool is_first_open;
	int open_count;
	struct mutex open_mutex;
	struct mutex lock;
	struct list_head mem;
	struct kmem_cache *queue_node_cache;
	struct xrp_device *xdev;
	struct xboot_dev *xbdev;
	const struct xrp_hw_ops *hw_ops;
	void *hw_arg;
	void *xmdr_log_info;

	struct dma_heap *vdsp_system_uncached_heap;
	struct list_head uncache_heap_list;
	struct mutex dma_buf_lock;
};

void *allocate_and_map_comm_memory(struct xrp_task_device *xrp_task_dev,
										size_t size,
										unsigned long iova,
										dma_addr_t *dma_pa_addr,
										gfp_t gfp);


void *allocate_and_map_vdsp_memory(struct xrp_task_device *xrp_task_dev,
										size_t size,
										unsigned long iova,
										dma_addr_t *dma_pa_addr,
										gfp_t gfp,
										bool if_alloc,
										bool if_map);

void free_and_unmap_comm_memory(struct xrp_task_device *xrp_task_dev, unsigned long iova, void *va, dma_addr_t *dma_pa_addr);


void free_and_unmap_special_memory(struct xrp_task_device *xrp_task_dev,
	size_t size, unsigned long iova, void *va, dma_addr_t *dma_pa_addr, bool if_free, bool if_unmap);

//xrp_mdr.h结构体
enum xrp_hmdr_status_e {
	HMDR_NV_DISENABLE          = 0x0,
	HMDR_NV_ENABLE             = 0x1,
};

//xrp exception/normal log disk drop
enum xrp_log_drop_e {
	HMDR_NORM_LOG_DROP          = 0x0,
	HMDR_EXCE_LOG_DROP          = 0x1,
};

struct xrp_log_info {
	enum xrp_hmdr_status_e hmdr_enable;
	uint32_t log_level;
	uint32_t writep;
	uint32_t readp;
	enum xrp_log_drop_e log_drop;
};

enum vdsp_status_type {
	DEFAULT,
	POWEROFF,
	WORK,
	IDLE,
	HANG,
	STATUS_MAX,
};


enum xrp_event_type_e {
	HE_LOG_FLUSH               = 0x0,
	HE_LOG_LEVEL_SET           = 0x1,
	HE_TYPE_MAX                = 0x100,
};

//malibox send msg struct
struct mbox_msg_t {
	uint32_t msg_len;
	uint32_t msg_type;
	uint32_t cv_type;
	uint32_t msg_id;
	uint32_t priority;
};//task

struct xrp_log_flush_msg {
	uint32_t readp_offset;
	uint32_t wirtep_offset;
};

struct xrp_log_level_msg {
	uint32_t log_level;
};

struct xrp_event_msg_s {
	uint32_t len;
	enum xrp_event_type_e type;
	union {
		struct xrp_log_flush_msg hlfm;
		struct xrp_log_level_msg hllm;
	} msg;
};

struct xrp_mdr_mbox_send_setlog_msg_s {
	uint32_t msg_len;
	uint32_t msg_type;
	struct xrp_event_msg_s event_msg;
};// setlog_level


struct vdsp_status_data {
	enum vdsp_status_type status;
};

struct xrp_mdr_mbox_send_query_msg_s {
	uint32_t msg_len;
	uint32_t msg_type;
	struct vdsp_status_data status_msg;
};//query status
//malibox send msg struct





//malibox receive msg struct
struct mbox_rec_vdsp_task_msg_s {
	uint32_t msg_len;
	uint32_t msg_type;
	uint32_t cv_type;
	uint32_t msg_id;
	uint32_t priority;
	uint32_t total_cycle;
};//task


struct mbox_rec_vdsp_flushlog_msg_s {
	uint32_t msg_len;
	uint32_t msg_type;
	struct xrp_event_msg_s event_msg;
};//flushlog

struct mbox_rec_vdsp_query_msg_s {
	uint32_t msg_len;
	uint32_t msg_type;
	struct vdsp_status_data status_msg;
};//query status
//malibox receive msg struct


typedef int (*xrp_notifier_cb)(void *arg, void *data);

struct xrp_notifer_block {
	uint32_t event;
	void *arg;
	xrp_notifier_cb cb;
	struct list_head node;
};

struct xrp_mdr_mem_info {
	int fd;
	u32 size;
	int prot;
	u32 da;
	u64 pa;
	unsigned long iova;
	void *va;
};

struct mdr_mbox {
	struct platform_device *pdev;
	struct mbox_client cl;
	struct mbox_chan *tx_ch;
	struct mbox_chan *rx_ch;
};

struct xrp_mdr_device {
	struct xrp_device *xdev;
	struct miscdevice miscdev;
	u32 log_level;
	struct fasync_struct *fasync_q;
	struct list_head nh;
	// struct vdsp_mbox *vmbox;
	struct mdr_register_module_result mdr_retinfo;
	struct mdr_module_ops mdr_vdsp_ops;
	void *vdsp_mdr_vaddr_base;
	bool is_first_open_mdr;
	int open_count;
};

struct xrp_device {
	struct platform_device *pdev;
	struct xrp_mdr_device *xmdev;
	struct xrp_task_device *xtdev;
	struct xrp_mdr_mem_info xmm;
};

struct xboot_dev {
	struct platform_device *pdev;
	struct regulator *vdsp_regulator;
	struct regulator *npu_subsys_regulator;
	atomic_t power;
	struct miscdevice miscdev;
};

enum heap_flags_t {
	SRAM_HEAP = 0,
	SROM_HEAP,
	COMMON_HEAP
};

struct sys_nocache_heap_data {
	int addr;
	int size;
	enum heap_flags_t data_heap_flag;
};

struct sys_nocache_dma_buf {
	struct list_head list;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
#if KERNEL_VERSION_LESS_THAN_6_6_0
	struct dma_buf_map map;
#else
	struct iosys_map map;
#endif
	struct sg_table *sgt;
	char *buf;
	int len;
	int fd;
	enum heap_flags_t heap_flag;
	int act_size;
};

void print_xrp_notifier_block(struct xrp_notifer_block *block);
void print_mbox_send_task_msg_t(struct mbox_msg_t msg);
void print_xrp_mdr_mbox_send_setlog_msg_s(struct xrp_mdr_mbox_send_setlog_msg_s msg);
void print_xrp_mdr_mbox_send_query_status(struct xrp_mdr_mbox_send_query_msg_s msg);

void print_mbox_rec_vdsp_task_msg_s(struct mbox_rec_vdsp_task_msg_s msg);
void print_mbox_rec_vdsp_flushlog_msg_s(struct mbox_rec_vdsp_flushlog_msg_s msg);
void print_mbox_rec_vdsp_query_msg_s(struct mbox_rec_vdsp_query_msg_s msg);

int xrp_poweron_map_on_process(struct xrp_task_device *xrp_task_dev);
int xrp_poweroff_unmap_on_process(struct xrp_task_device *xrp_task_dev);

int xmdr_init(struct xrp_device *xdev);
int xmdr_exit(struct xrp_device *xdev);

int alloc_and_map_special_mdr_mem(struct xrp_task_device *xtdev, struct xrp_mdr_mem_info *mem, bool if_alloc, bool if_map);
void free_and_unmap_special_mdr_mem(struct xrp_task_device *xtdev, bool if_free, bool if_unmap);
int vdsp_exception_resource_release(struct xrp_task_device *xrp_task_dev);
void clear_priority_queues(struct xrp_task_device *xrp_task_dev, size_t count);

void set_vdsp_pw_true(void);
void set_vdsp_pw_false(void);
bool is_vdsp_pw_false(void);

void set_vdsp_excp_true(void);
void set_vdsp_excp_false(void);
bool is_vdsp_excp_true(void);

int get_log_level(void);

struct sys_nocache_dma_buf *sys_nocache_map(struct xrp_task_device *xrp_task_dev,
										struct sys_nocache_heap_data *data);
int sys_nocache_unmap(struct xrp_task_device *xrp_task_dev,
						struct sys_nocache_heap_data *heap_data);

int rqs_sysncached_heap(struct xrp_task_device *xrp_task_dev);
void rls_sysncached_heap(struct xrp_task_device *xrp_task_dev);
#endif
