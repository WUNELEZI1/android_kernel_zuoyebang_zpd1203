/* SPDX-License-Identifier: GPL-2.0-only
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _XRING_XPLAYER_UAPI_H_
#define _XRING_XPLAYER_UAPI_H_

#define XPLAYER_MAGIC 'X'

#define XPLAYER_GET_ENCODED_SLICE_INFO    _IOR(XPLAYER_MAGIC, 0x1, struct xplayer_encoded_slice_info)

#define XPLAYER_FLAG_SINK 0x1
#define XPLAYER_FLAG_SOURCE 0x2
#define XPLAYER_FLAG_EVENT 0x4

#define XPLAYER_SLICE_INIT    _IOW(XPLAYER_MAGIC, 0x2, uint32_t)
#define XPLAYER_SLICE_DEINIT    _IO(XPLAYER_MAGIC, 0x3)
#define XPLAYER_SLICE_PRESENT_INDEX    _IOW(XPLAYER_MAGIC, 0x6, uint8_t)

#define WFD_SINK_IOVA_RESERVED 0x8000000ULL

struct xplayer_cmdlist_addr {
	uint64_t phy_addr[4];
};

struct xplayer_cmdlist_config {
	uint32_t write_value[4];
};

struct xplayer_encoded_slice_info {
	int fd;
	int offset;
	int length;
	int slice_index;
	int64_t pts;
};

enum xplayer_info_type {
	XPLAYER_INFO_INVAILD = -1,
	XPLAYER_INFO_ENC_SLICE,
	XPLAYER_INFO_VSYNC_NOTIFY,
	XPLAYER_INFO_FRAME_BEGIN_NOTIFY,
	XPLAYER_INFO_FRAME_DONE_NOTIFY,
	// ...
};

struct xplayer_info {
	enum xplayer_info_type type;
	int rsv;
	union {
		struct xplayer_encoded_slice_info slice_info;
	};
};

enum xplayer_event {
	XPLAYER_EVENT_VSYNC = XPLAYER_INFO_VSYNC_NOTIFY,
	XPLAYER_EVENT_FRAME_BEGIN = XPLAYER_INFO_FRAME_BEGIN_NOTIFY,
	XPLAYER_EVENT_FRAME_DONE = XPLAYER_INFO_FRAME_DONE_NOTIFY,
};

/**
 * struct xplayer_iommu_format_info - plane format info used iommu
 *
 * @offsets: Offset from buffer start in bytes
 * @is_afbc: is afbc mode or not
 */
struct xplayer_iommu_format_info {
	unsigned int offsets[3];
	bool is_afbc;
};

#endif /* _XRING_XPLAYER_UAPI_H_ */
