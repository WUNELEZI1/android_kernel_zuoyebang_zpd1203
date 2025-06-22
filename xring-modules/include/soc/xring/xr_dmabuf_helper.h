/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on dma heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef __XRING_DMABUF_HELPER_H__
#define __XRING_DMABUF_HELPER_H__

#include <linux/dma-buf.h>
#include <linux/dma-direction.h>
#include <dt-bindings/xring/xr-heaps-dts.h>

enum {
	XR_MEM_PROTECT_HEAP = HEAP_PROTECT_DRM,
	XR_MEM_FACEID_HEAP = HEAP_SECURE_FACEID,
	XR_MEM_SYSTEM_DIO_ALLOC = HEAP_SYSTEM_DIO_ALLOC,
};

enum {
	XR_ACTION_PREALLOC_START = 1,
	XR_ACTION_PREALLOC_END,
};

#define XR_HELPER_IOC_MAGIC 'X'
#define XR_HELPER_IOCTL_HEADERSIZE		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x0, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_GETSFD		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x1, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_MEMPROT		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x2, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_PREALLOC		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x3, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_BEGIN_CPU_ACCESS_PARTIAL	\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x4, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_END_CPU_ACCESS_PARTIAL		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x5, struct xr_dmabuf_helper_args)
#define XR_HELPER_IOCTL_DMABUF_DIO_ALLOC		\
	_IOWR(XR_HELPER_IOC_MAGIC, 0x6, struct xr_dmabuf_helper_args)

enum xr_dmabuf_kernel_type {
	XR_DMABUF_KERNEL_ISP_RPROC,
	XR_DMABUF_KERNEL_ISP_CA,
	XR_DMABUF_KERNEL_GPU,
	XR_DMABUF_KERNEL_NPU,
	XR_DMABUF_KERNEL_MITEE_TUI,
	XR_DMABUF_KERNEL_TYPE,
};

struct xr_heap_buffer_header {
	long long header_size;	/* header size */
};

struct xr_dmabuf_helper_args {
	int dmabuf_fd;	/* to fd */
	u32 sfd;
	struct xr_heap_buffer_header header;
	u32 scene;
	u32 action;
	u32 size;
	int file_fd;
	loff_t offset;
	loff_t ppos;
};

struct xr_dmabuf_helper_cpu_access {
	int dmabuf_fd;
	enum dma_data_direction direction;
	unsigned int offset;
	unsigned int len;
};

long long xr_dmabuf_get_headersize(struct dma_buf *dmabuf);
u32 xr_dmabuf_helper_get_sfd(struct dma_buf *dmabuf);
void show_dmabuf_detail(bool verbose);
int xr_dmabuf_pre_alloc(const char *name, unsigned int size, int mode);
int xr_dmabuf_kernel_account(struct dma_buf *dmabuf, enum xr_dmabuf_kernel_type type);
int xr_dmabuf_kernel_unaccount(struct dma_buf *dmabuf, enum xr_dmabuf_kernel_type type);
#endif
