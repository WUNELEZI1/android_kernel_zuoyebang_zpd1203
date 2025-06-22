/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_ISP_FENCE_H_
#define _XR_ISP_FENCE_H_
#include <linux/bitmap.h>
#include <media/v4l2-device.h>
#include <linux/mutex.h>
#define FENCE_MAX_NUM	 2048
#define FENCE_NAME_LEN	 64
#define USER_PAYLOAD_LEN 2

enum Fence_status {
	FENCE_CREATE = 1,
	FENCE_DESTORY,
	FENCE_REGISTER,
	FENCE_DEREGISTER,
	FENCE_SIGNAL,
};

struct fence_event_head {
	uint32_t fence_id;
	uint32_t status;
	uint32_t event_cause;
	uint64_t payload_data[USER_PAYLOAD_LEN];
};
//callbacks
struct user_callback {
	struct list_head list;
	uint64_t payload_data[USER_PAYLOAD_LEN];
};
//fence_entity
struct fence_entity {
	char name[FENCE_NAME_LEN];
	int32_t fence_id;
	uint32_t state;
	struct list_head userload_list;
};
//Fence Manager struct
struct fence_manager {
	struct video_device *vdev;
	struct fence_entity fence_table[FENCE_MAX_NUM];
	spinlock_t fence_spinlocks[FENCE_MAX_NUM];
	DECLARE_BITMAP(bitmap, FENCE_MAX_NUM);
	struct mutex fence_ctrl_mutex;
};
int cam_fence_ioctl(void *arg);
int cam_fence_close(struct fence_manager *fence_dev);
int cam_fence_init(struct fence_manager *fence_dev, struct video_device *vdev);
int cam_fence_exit(void);
#endif /* _XR_ISP_FENCE_H_ */
