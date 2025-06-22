/* SPDX-License-Identifier: GPL-2.0 */
/**
 * xring dec support smmu
 *
 */


#ifndef X_V4L2_H
#define X_V4L2_H

#include "vsi-v4l2.h"
#include "vsi-v4l2-priv.h"

struct v4l2_plane_info {
	s32 fd;
	u32 offset;
	u32 length;
	u32 bytes_used;
};

typedef enum {
	VSI_V4L2_BUFFER_TYPE_INPUT,
	VSI_V4L2_BUFFER_TYPE_OUTPUT,
	VSI_V4L2_BUFFER_TYPE_META,
	VSI_V4L2_BUFFER_TYPE_PP,
} vsi_v4l2_dma_buffer_info_type;

struct vsi_v4l2_dma_buffers_info
{
	unsigned long instance_id;
	vsi_v4l2_dma_buffer_info_type type;
	uint32_t index;
	uint32_t plane_count;
	struct v4l2_plane_info plane_infos[4];
	unsigned long dma_buf;
};

int vsi_v4l2_get_buffer_info(struct vsi_v4l2_device *vpu, struct vsi_v4l2_dma_buffers_info *info);

#endif

