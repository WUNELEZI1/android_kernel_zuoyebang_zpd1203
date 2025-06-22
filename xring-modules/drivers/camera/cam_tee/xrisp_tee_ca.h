/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef MITEE_XRISP_CA_H
#define MITEE_XRISP_CA_H

#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>

#define DRIVER_NAME "xrisp-ca"

#define XRISP_CA_DEBUG
#define XRISP_CA_TEST

#define CA_KMALLOC_RETRY_TIMES	  (3)
#define CA_KMALLOC_RETRY_DELAY_MS (3)

#define FIRMWARE_BOOT_ADDR (0x80000000)
#define FIRMWARE_LOAD_SIZE (0x1400000)
#define FIRMWARE_MAX_SIZE  (0xC00000)
#define KZALLOC_MAX_SIZE   (0x400000)
#define FIRMWARE_ARR_CNT   (3)

struct ta_dma_buf_nonsec_info {
	uint32_t mmap_type;
	uint64_t iova;
};

struct ta_xrisp_load_info {
	uint32_t sfd;
	uint32_t size;
	uint64_t iova;
	uint32_t fs_load;
};

struct xrisp_ca_buf_dentry {
	struct debugfs_blob_wrapper blob_buf;
	struct dentry *blob_dentry;
};

struct xrisp_ca_rsv_mem {
	phys_addr_t   mem_phys;
	size_t        size;
	void          *baseaddr;
	struct xrisp_ca_buf_dentry blob_tab[5];
};

/**
 * struct xrisp_ca_private - TEE private data
 * @ctx:	TEE context handler.
 * @session:sample_test TA session identifier.
 * @sfd:	secure memory handle for storing isp firmware
 * @mode:	isp mode, 2-safe, 1-unsafe
 */
struct xrisp_ca_private {
	struct tee_context *ctx;
	atomic_t open_cnt;
	uint32_t session;
	int mode;
	uint32_t sfd;
	uint32_t load_size;
	struct dma_buf *load_buf;
	struct dma_heap *sec_heap;
	struct dentry *dentry;
	struct xrisp_ca_rsv_mem meminfo;
	struct mutex nsbuf_map_mtx;
};

struct device *xrisp_ca_get_dev(void);

int xrisp_ca_debugfs_init(struct xrisp_ca_private *xrisp_ca);
void xrisp_ca_debugfs_exit(struct xrisp_ca_private *xrisp_ca);
#endif // MITEE_XRISP_CA_H
