/* SPDX-License-Identifier: GPL-2.0 */
/*
 * nv driver for Xring
 *
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 *
 */
#ifndef __XRING_NV_DRIVER_H__
#define __XRING_NV_DRIVER_H__

#include <soc/xring/xr_nv_interface.h>

#define NVE_PARTITION_NAME      "/dev/block/by-name/nvmem"
#define NVE_REGION_SIZE         (128 * 1024)
#define NVE_UPDATE_REGION_ID    0
#define NVE_ORIGIN_REGION_ID    1
#define NVE_PRIMARY_REGION_ID   2
#define NVE_BACKUP_REGION_ID    3
#define NVE_HEADER_LEN          NV_ITEM_SIZE
#define NVE_HEADER_NAME_LEN     20
#define NVE_HEADER_REAERVE      (NVE_HEADER_LEN - NVE_HEADER_NAME_LEN - 8)
#define ITEM_ID_MAX_LEN         (128 * 3 + 1)

#define NV_LOG_ERR(fmt, args...) pr_err("[NV][%s][%d]: "fmt, __func__, __LINE__, ##args)

#define NV_LOG_INFO(fmt, args...) pr_info("[NV][%s][%d]: "fmt, __func__, __LINE__, ##args)

struct nve_region_header {
	char vve_region_name[NVE_HEADER_NAME_LEN];
	u32 valid_items;
	u32 crc32;
	char reserved[NVE_HEADER_REAERVE];
};

struct nve_struct {
	struct nve_region_header *nve_header;
	u8 *nve_region_ram;
};

#endif  /* __XRING_NV_DRIVER_H__ */
