/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_CBM_H__
#define __XRING_CBM_H__

#include <linux/dcache.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/completion.h>
#include <linux/dma-direction.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#define CBM_MAX_TABLE_LEN		(1024)
#define CBM_SHM_IOVA_BASE		(0x40000000)
#define CBM_SHM_IOVA_SIZE		(0x40000000)

enum cam_buf_status {
	CAM_BUF_INVALID,
	CAM_BUF_ACTIVE,
	CAM_BUF_ALLOCATED,
	CAM_BUF_MAPED,
	CAM_BUF_PUTTED,
};

enum cam_buf_action {
	CAM_BUF_ALLOC,
	CAM_BUF_RELEASE,
	CAM_BUF_MAP,
};

enum iomap_safe_map_type {
	BUF_NOT_SSMMU_MAPED = 0,
	CAMBUF_SSMMU_MAPED,
	SECBUF_SSMMU_MAPED,
};

#define BUF_ACTION_STR(action)	(action == CAM_BUF_ALLOC ? "alloc" :		\
				(action == CAM_BUF_RELEASE) ? "release" :	\
				(action == CAM_BUF_MAP) ? "map" :		\
				 "debug")

struct cam_buf_handle {
	uint64_t fd : 32;
	uint64_t idx : 16;
	uint64_t reserved1 : 8;
	uint64_t reserved0 : 8;
};

struct cam_buf_iomap {
	dma_addr_t iovaddr;
	dma_addr_t iommuaddr;
	struct cam_share_buf_region *region;
	int iommu_port;
	size_t size;
	ssize_t iommu_size;
	struct dma_buf *buf;
	struct sg_table *sgl;
	bool dynamic_ssmmu_map;
	struct dma_buf_attachment *attach;
	enum dma_data_direction direction;
	enum iomap_safe_map_type map_type;
};

struct cam_share_buf_region {
	uint32_t region_id;
	dma_addr_t addr_base;
	size_t max_size;
	size_t cur_size;
	struct list_head buf_elem;
	struct mutex lock;
};

struct xrisp_set_mpu_region {
	uint32_t msg;
	uint32_t region_id;
	uint64_t base;
	uint32_t size;
};

struct cam_buf_element {
	union {
	uint64_t buf_handle;
	struct cam_buf_handle hdl;
	};

	struct list_head region_head;
	uint32_t region_id;

	int32_t fd;
	unsigned long i_ino;
	size_t size;
	struct dma_buf *buf;
	bool mapio;
	dma_addr_t iova;
	bool mapk;
	uintptr_t kva;
	uint32_t sfd;

	struct cam_buf_iomap iomap;

	struct mutex lock;
	enum cam_buf_status status;
	bool is_internal;
	bool is_imported;
	struct timespec64 timestamp;
	bool uncached;

	struct dentry *entry;
};

struct cam_buf_manager {
	struct device *dev;
	struct dma_heap *system_heap;
	struct dma_heap *system_uncached_heap;
	struct dma_heap *sec_heap;
	struct iommu_domain *i_domain;
	struct iommu_group *i_group;
	struct iommu_device *i_dev;

	bool avalid;

	unsigned long *bitmap;
	size_t bits;
	struct cam_buf_element buf_tbl[CBM_MAX_TABLE_LEN];
	struct mutex lock_tbl;

	struct cam_share_buf_region sh_buf_region[CAM_SHM_REGION_NUM];
	unsigned long *bitmap_region;
	size_t region_num;
	struct completion set_region_complet;
	struct mutex lock_region;

	struct dentry *debugfs;
	struct dentry *cbt_entry;
};

int cbm_cbt_init(void);
void cbm_cbt_exit(void);

int cbm_iova_region_init(void);
void cbm_iova_region_exit(void);

bool cbm_avlid(void);

int cbm_buf_iovmap(struct cam_buf_iomap *obj);
void cbm_buf_iovunmap(struct cam_buf_iomap *obj);
int sec_buf_iovmap(struct cam_buf_iomap *iomap, uint32_t sfd, uint32_t flags);
void sec_buf_iovunmap(struct cam_buf_iomap *iomap, uint32_t sfd);

inline void cbm_buf_lock(struct cam_buf_element *cbm_buf);
inline void cbm_buf_unlock(struct cam_buf_element *cbm_buf);

void pr_cam_buf(struct cam_buf_element *cbm_buf, enum cam_buf_action action);

int cbm_debugfs_init(void);
void cbm_debugfs_exit(void);
int cbt_debugfs_install(struct dentry *entry, struct cam_buf_element *cbm_buf);
void cbt_debugfs_uninstall(struct cam_buf_element *cbm_buf);

int cbm_iova_region_alloc(enum xrisp_cam_buf_region_type type);
int cbm_iova_region_free(uint32_t region_id);
struct cam_share_buf_region *cbm_get_iova_region(uint32_t region_id);
void cbm_buf_add_to_region(struct cam_share_buf_region *region, struct cam_buf_element *buf);
void cbm_buf_del_form_region(struct cam_buf_element *buf);
void cbm_iova_region_lock(struct cam_share_buf_region *region);
void cbm_iova_region_unlock(struct cam_share_buf_region *region);

#endif
