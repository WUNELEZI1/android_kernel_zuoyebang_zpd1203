/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAM_CTRL_H_
#define _XRISP_CAM_CTRL_H_

#include <linux/mutex.h>
#include <linux/completion.h>
#include <media/v4l2-event.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#define MSG_BUF_BITMAP_NUM   (4)
#define MSG_BUF_SIZE	     (512)
#define MAX_RGLTR_SET_NUM    (5)
#define ISP_POWER_ON	     (true)
#define ISP_POWER_OFF	     (false)

#define CTRL_MBOX_CH	     (IPC_VC_PERI_NS_AP_ISP1)
#define FAST_MBOX_CH	     (IPC_VC_PERI_NS_AP_ISP0)
#define SHUB_MBOX_CH	     (IPC_VC_AO_NS_SH_AP_OIS)
#define RPMSG_BUF_MAXLEN     (488)
#define MBOX_BUF_MAXLEN	     (60)

#define SENSOR_ID_MAX	     (5)
#define POWER_SET_SIZE_MAX   (320)

struct xrisp_msg_bitmap {
	unsigned long bitmap[MSG_BUF_BITMAP_NUM];
};

struct xrisp_msg_buf {
	__u8 buf[MSG_BUF_SIZE];
};

struct ocm_buf_list {
	struct list_head node;
	struct xrisp_ocm_buf_info buf_info;
};

struct xrisp_cam_ctrl {
	struct video_device *vdev;
	struct xrisp_msg_bitmap *p_msg_bitmap;
	struct xrisp_msg_buf *p_msg_buf;
	uint64_t buf_handle;
	uint32_t msg_buf_num;
	bool is_msgbuf_register;
	struct mutex ctrl_lock;
	spinlock_t event_lock;

	struct completion isp_service_online;

	struct list_head ocm_buf_list;
	struct mutex ocm_list_mutex;
	bool ois_mbox_reg_done;
	bool mbox_reg_done;
};

enum xrisp_drv_cmd_list {
	DRV_DUMMY,
	DRV_ISP_PIPE_POWERON,
	DRV_ISP_PIPE_POWEROFF,
	DRV_ISP_DVFS,
	DRV_ISP_SERVICE_ONLINE,
	DRV_ISP_PARKLENS_COMPLETE,
};

struct cpc_msg_data {
	uint64_t msg_id;
	uint8_t type;
	uint32_t size;
	unsigned char data[];
};

int xrisp_cam_ctrl_init(struct video_device *vdev);
void xrisp_cam_ctrl_release(void);

void xrisp_sensor_power_resource_clear(void);

int xrisp_send_event_to_hal(void *buffer, uint32_t len, uint32_t event_type);
void xrisp_ipc_msgbuf_reset(void);

int xrisp_cam_cmd_ioctl(void *arg);

uint64_t xrisp_read_msg_id(void *addr);
void camctrl_pr_msg(void *data, unsigned int len);

void xrisp_rpmsg_register(void);
int xrisp_ctrl_mbox_register(bool *register_done);
int xrisp_ctrl_mbox_unregister(bool *need_unregister);

int xrisp_ois_mbox_register(bool *register_done);
int xrisp_ois_mbox_unregister(bool *need_unregister);

int xrisp_camctrl_debugfs_init(void);
void xrisp_camctrl_debugfs_exit(void);
int xrisp_ipc_cpc_handler(void *data, unsigned int len);
void xrisp_ocmlist_release(void);
#endif
