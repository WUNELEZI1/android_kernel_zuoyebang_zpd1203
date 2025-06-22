/* SPDX-License-Identifier: GPL-2.0 */
/*
 * XRING_DMABUF_HELPER
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on dma heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef __XRING_DMABUF_HELPER_H__
#define __XRING_DMABUF_HELPER_H__
#include <linux/dma-buf.h>


enum {
	XR_MEM_PROTECT_HEAP = 1,
	XR_MEM_FACEID_HEAP,
	XR_MEM_SYSTEM_DIO_ALLOC,
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

struct xr_heap_buffer_header {
	union {
		long long header_size;	/* header size */
	};
};

struct xr_dmabuf_helper_args {
	int dmabuf_fd;	/* to fd */
	__u32 sfd;
	struct xr_heap_buffer_header header;
	__u32 scene;
	__u32 action;
	__u32 size;
	int file_fd;
	long long offset;
	long long ppos;
};

enum dma_data_direction {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};

struct xr_dmabuf_helper_cpu_access {
	int dmabuf_fd;
	enum dma_data_direction direction;
	unsigned int offset;
	unsigned int len;
};

#endif
