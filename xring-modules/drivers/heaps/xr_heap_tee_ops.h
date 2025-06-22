/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Xring Secure operater
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef _XR_HEAP_TEE_OPS_H
#define _XR_HEAP_TEE_OPS_H

#include <linux/tee.h>
#include <linux/types.h>
#include <linux/dma-buf.h>
#include <dt-bindings/xring/xr-heaps-dts.h>
#include <linux/version.h>
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
#include <linux/tee_drv.h>
#else
#include <tee_drv.h>
#endif
#include "xring_heaps.h"

#define TEE_SECMEM_NAME "xr-heap-ca"

#define TA_UUID \
	UUID_INIT(0xa734eed9, 0xd6a1, 0x4244, 0xaa, 0x50, 0x7c, 0x99, 0x71, 0x9e, 0x6b, 0x6b)

#define HEAP_TEE (HEAP_SECURE | HEAP_MIX)

struct tee_info {
	struct tee_context *context;
	u32 session;
};

enum xring_ta_tag {
	XRING_SEC_CMD_PGATBLE_INIT = 0,
	XRING_SEC_CMD_ALLOC = 0x1,
	XRING_SEC_CMD_FREE = 0x10,
	XRING_SEC_CMD_ALLOC_TEE = 0x100,
	XRING_SEC_CMD_FREE_TEE = 0x1000,
	XRING_SEC_CMD_TABLE_SET = 0x10000,
	XRING_SEC_CMD_TABLE_CLEAN = 0x100000,
	XRING_SEC_CMD_SEND_HEAPINFO = 0x1000000,
};

#define XRING_TEE_ALLOC (XRING_SEC_CMD_ALLOC | XRING_SEC_CMD_ALLOC_TEE | XRING_SEC_CMD_TABLE_SET)
#define XRING_TEE_FREE (XRING_SEC_CMD_FREE | XRING_SEC_CMD_FREE_TEE | XRING_SEC_CMD_TABLE_CLEAN)
#define XRING_SET_PROTECT_OR_SAFE (XRING_SEC_CMD_ALLOC | XRING_SEC_CMD_TABLE_SET)
#define XRING_CLEAR_PROTECT_OR_SAFE (XRING_SEC_CMD_FREE | XRING_SEC_CMD_TABLE_CLEAN)

/**
 * @brief this struct transfer from ree to tee
 * @buff_id: secure/protect file descriptor
 * @nents: scatterlist numbers
 * @size: memor size
 * @prot: memory property
 * @type: heap type
 * @phys_addr: memory physical address
 * @smmuid: identify which smmu
 * @sid: identify which master
 * @ssid: identify which core in master
 *
 */
struct mem_chunk_list {
	u32 buff_id;
	u32 nents;
	u32 size;
	int prot;
	u32 type;
	void *phys_addr;
	u32 smmuid;
	u32 sid;
	u32 ssid;
};

/**
 * @brief this struct describe all pages in a scatterlist
 * @addr: page phys address
 * @nr_pages: page count in a scatterlist
 *
 */
struct tz_pageinfo {
	u64 addr;
	u32 nr_pages;
} __aligned(8);

/**
 * @brief this struct describe all pages in a sg table
 * @pageinfo: a pointer arry represent all pages in scatterlist
 * @sfd: secure fd return from tee
 * @size: memory alloc size of pageinfo
 * @nents: number of scatterlist in sg table
 *
 */
struct tz_info {
	struct tz_pageinfo *pageinfo;
	u32 sfd;
	u32 size;
	u32 nents;
	u32 type;
};

int xr_secmem_tee_init(void);
void xr_secmem_tee_destroy(void);
int xr_change_mem_prot_tee(struct tz_info *info, u32 cmd);
int xr_send_heapinfo(struct xring_secure_heap_data *xring_secure_heap_list, int nents);
int xr_xhee_action(struct tz_info *info, int xhee_cmd);
#endif
