/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_CBM_API_H__
#define __XRING_CBM_API_H__
#include <linux/types.h>
#include <linux/dma-buf.h>

extern struct cam_buf_manager g_cam_buf_mgr;

struct cam_buf_request {
	size_t size;
	uint32_t buf_region;
	unsigned int flags;
};

struct cam_buf_map {
	int fd;
	uint32_t buf_region;
	unsigned int flags;
};

struct cam_buf_desc {
	uint64_t buf_handle;
	uintptr_t kvaddr;
	dma_addr_t iovaddr;
};

enum cam_buf_cache_ops {
	CAM_BUF_CPU_ACCESS_BEGIN,
	CAM_BUF_CPU_ACCESS_END
};

struct cam_buf_cache {
	uint64_t buf_handle;
	uint32_t is_partial;
	uint32_t offset;
	uint32_t size;
	enum cam_buf_cache_ops ops;
};

size_t sg_table_len(struct sg_table *sg_tlb);

int cbm_buf_get(struct cam_buf_request *req, struct cam_buf_desc *desc);
int cbm_buf_put(struct cam_buf_desc *desc);

int cbm_buf_map(struct cam_buf_map *map, struct cam_buf_desc *desc);

int cbm_dma_buf_cache_ops(struct cam_buf_cache *cache);

int cbm_buf_get_by_handle(uint64_t buf_handle, struct cam_buf_desc *desc, size_t *size);

int xrisp_cbm_ioctl(void *cmd);
void cbm_buf_release_all(void);
int xrisp_cbm_buf_need_release(void);
#endif
