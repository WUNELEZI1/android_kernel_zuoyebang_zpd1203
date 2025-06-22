// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */


#ifndef __UAPI_XRISP_H__
#define __UAPI_XRISP_H__

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/media.h>

#define XRISP_DEVICE_TYPE_BASE	    (MEDIA_ENT_F_OLD_BASE + 500)
#define XRISP_VIDEO_DEVICE_TYPE	    (XRISP_DEVICE_TYPE_BASE + 1)

#define FENCE_SYNC_V4L_EVENT_TYPE   (V4L2_EVENT_PRIVATE_START + 1)
#define RING_CAMERA_V4L2_EVENT_TYPE (V4L2_EVENT_PRIVATE_START + 2)
#define RING_SYSFS_V4L2_EVENT_TYPE  (V4L2_EVENT_PRIVATE_START + 3)
#define RING_CAMERA_V4L2_RPC_EVENT_TYPE (V4L2_EVENT_PRIVATE_START + 4)

#define FENCE_SYNC_V4L_EVENT_ID	    (0)
#define ISP_MSG_V4L2_EVENT_ID	    (1)
#define ISP_WATCHDOG_V4L2_EVENT_ID  (2)
#define ISP_SYSFS_V4L2_EVENT_ID	    (3)

#define XRING_CMD_NR		    (0x99)
#define XRING_INIT		    _IOWR('X', XRING_CMD_NR, struct xrisp_control_arg)
#define XRING_FENCE		    _IOWR('X', XRING_CMD_NR + 1, struct xrisp_control_arg)
#define XRING_IOC_CBM_BUF	    _IOWR('X', XRING_CMD_NR + 2, struct xrisp_control_arg)
#define XRING_IOC_CAMERA_CMD	    _IOWR('X', XRING_CMD_NR + 3, struct xrisp_control_arg)
#define XRING_IOCTL_CAM_TOF	    _IOWR('X', XRING_CMD_NR + 4, struct xrisp_control_arg)


#define FENCE_STATE_UNCREATE	     (0)
#define FENCE_STATE_CREATE_SUCCESS   (1)
#define FENCE_STATE_SIGNALED_SUCCESS (2)
#define FENCE_STATE_SIGNALED_ERROR   (3)
#define FENCE_STATE_SIGNALED_FLUSH   (4)


#define FENCE_SYNC_EVENT_START	     (0)
#define FENCE_SYNC_EVENT_SUCCESS     (FENCE_SYNC_EVENT_START + 1)
#define FENCE_SYNC_EVENT_FLUSH	     (FENCE_SYNC_EVENT_START + 2)
#define FENCE_SYNC_EVENT_STOP	     (FENCE_SYNC_EVENT_START + 3)
#define FENCE_SYNC_REG_PAYLOAD_EVENT (FENCE_SYNC_EVENT_START + 4)
#define FENCE_SYNC_SIGNAL_EVENT	     (FENCE_SYNC_EVENT_START + 5)
#define FENCE_SYNC_RELEASE_EVENT     (FENCE_SYNC_EVENT_START + 6)


#define CAM_BUF_HW_READ_ONLY	(1 << 0)
#define CAM_BUF_HW_WRITE_ONLY	(1 << 1)
#define CAM_BUF_HW_READ_WRITE	(1 << 2)
#define CAM_BUF_USER_READ_ONLY	(1 << 3)
#define CAM_BUF_USER_WRITE_ONLY (1 << 4)
#define CAM_BUF_USER_READ_WRITE (1 << 5)
#define CAM_BUF_KERNEL_ACCESS	(1 << 6)

#define CAM_BUF_HAL_UNCACHED    (1 << 7)
#define CAM_BUF_SAFE_SHAREUSE	(1 << 8)
#define SEC_BUF_TYPE_PROTECT	(1 << 9)

#define XRISP_AOC_POWER_SYNC_BIT_MAX        (6)
#define XRISP_AP_CAM_POWER_STATUS_BUSY      (1)
#define XRISP_AP_CAM_POWER_STATUS_IDLE      (0)
#define XRISP_AP_CAM_POWER_STATUS_MASK      BIT(0)
#define XRISP_SH_CAM_POWER_STATUS_MASK      BIT(1)
#define XRISP_AP_CAM_FRONT_STATUS_MASK      BIT(3)
#define XRISP_AOC_AP_POWER_MASKBIT          (0)
#define XRISP_AOC_SH_POWER_MASKBIT          (1)
#define XRISP_AOC_AP_FRONT_MASKBIT          (3)
#define XRISP_AP_CAM_POWER_STATUS_SIZE      (4)
#define XRISP_AP_CAM_ACTRL_BMRW_SHIFT       (16)
#define XRISP_AP_CAM_POWER_STATUS_BASE      (ACPU_LPIS_ACTRL + LPIS_ACTRL_SC_RSV_NS_17)

struct xrisp_control_arg {
	__u32 op_code;
	__u32 size;
	__u64 handle;
};


struct xring_fence_event_head {
	__u32 fence_id;
	__u32 status;
	__u32 event_cause;
	__u64 payload_data[2];
};

struct fence_create_para {
	__u32 fence_id;
	char name[64];
};
struct fence_register_para {
	__u32 fence_id;
	__u64 payload_data[2];
};
struct fence_signal_para {
	__u32 fence_id;
	__u32 status;
	__u32 event_cause;
};

enum {
	XRING_FENCE_CREATE = 1,
	XRING_FENCE_DESTORY,
	XRING_FENCE_REGISTER,
	XRING_FENCE_DEREGISTER,
	XRING_FENCE_SIGNAL,
};



enum xr_camera_power_seq_type {
	SENSOR_DVDD = 0,
	SENSOR_AVDD,
	SENSOR_BOB1,
	SENSOR_AF,
	SENSOR_OIS,
	SENSOR_CAMIO,
	SENSOR_BOB2,
	SENSOR_CLK_VDD,
	SENSOR_RESET,
	SENSOR_MCLK,
	SENSOR_STANDBY,
	SENSOR_VAF_PWDM,
	SENSOR_APERT,
	SENSOR_CUSTOM_GPIO1,
	SENSOR_CUSTOM_GPIO2,
	SENSOR_SEQ_TYPE_MAX,
};

enum cam_power_type {
	POWER_TYPE_SENSOR,
	POWER_TYPE_AF,
	POWER_TYPE_OIS,
	POWER_TYPE_EEPROM,
	POWER_TYPE_APERT,
	POWER_TYPE_MAX,
};

struct xrisp_sensor_power_setting {
	__u8 power_seq_type;
	__u16 delay_milli_seconds;
	__u64 power_seq_value;
};

struct xrisp_sensor_power_ctrl {
	__u8 sensor_id;
	__u8 count;
	__u8 ois_delay;
	__u8 need_parklens;
	__u8 aon_boot_scene;
	__u8 is_probe_event;
	enum cam_power_type power_type;
	struct xrisp_sensor_power_setting power_settings[];
};



enum xrisp_cbm_ops {
	XRISP_CBM_BUF_ALLOC_MAP,
	XRISP_CBM_BUF_MAP,
	XRISP_CBM_BUF_RELEASE,
	XRISP_CBM_BUF_CPU_ACCESS_BEGIN,
	XRISP_CBM_BUF_CPU_ACCESS_END,
	XRISP_CBM_BUF_ALLOC_IOVA_REGION,
	XRISP_CBM_BUF_FREE_IOVA_REGION,
	XRISP_CBM_BUF_MAX
};


#define CAM_SHM_REGION_NUM		(18)
#define CAM_SHM_STATIC_REGION_IDX_START	(1)
#define CAM_SHM_STATIC_REGION_NUM	(1)
#define CAM_SHM_DYNAMIC_REGION_64MB_IDX_START	\
		(CAM_SHM_STATIC_REGION_IDX_START + CAM_SHM_STATIC_REGION_NUM)
#define CAM_SHM_DYNAMIC_REGION_64MB_NUM		(14)
#define CAM_SHM_DYNAMIC_REGION_128MB_IDX_START	\
		(CAM_SHM_DYNAMIC_REGION_64MB_IDX_START + CAM_SHM_DYNAMIC_REGION_64MB_NUM)
#define CAM_SHM_DYNAMIC_REGION_128MB_NUM	(2)
#define CAM_SHM_DYNAMIC_REGION_64MB_SIZE	(0x2e00000)
#define CAM_SHM_DYNAMIC_REGION_128MB_SIZE	(0x5000000)

struct xrisp_cam_buf_desc {
	__u64 buf_handle;
	__s32 fd;
	__u64 hwaddr;
};

enum xrisp_cam_buf_region_type {
	XRISP_CAM_SHM_REGION_64MB,
	XRISP_CAM_SHM_REGION_128MB,
};

struct xrisp_cam_buf_region {
	enum xrisp_cam_buf_region_type region_type;
	__u32 region;
};

struct xrisp_cam_buf_req {
	__u32 buf_region;

	__u32 size;
	__u32 flags;
	struct xrisp_cam_buf_desc desc;
};

struct xrisp_cam_buf_map {
	__s32 fd;
	__u32 buf_region;

	__u32 flags;
	struct xrisp_cam_buf_desc desc;
};

struct xrisp_cam_buf_cache {
	__u64 buf_handle;
	__u32 is_partial;
	__u32 offset;
	__u32 size;
};

struct xrisp_cam_buf_release {
	__u64 buf_handle;
};



enum xrisp_power_domain {
	XRISP_FE_CORE_0 = 0,
	XRISP_FE_CORE_1 = 1,
	XRISP_FE_CORE_2 = 2,
	XRISP_BE = 3,
	XRISP_PE = 4,
	MAX_XISP_DOMAIN_NUM,
};

enum xrisp_boot_scene {
	XRISP_NORMAL_BOOT = 0,
	XRISP_SAFE_FACE_BOOT,
	XRISP_SAFE_AON_BOOT,
	XRISP_SHAKE_BOOT,
	XRISP_NSAFE_FACE_BOOT,
	XRISP_BOOT_SCENE_MAX,
};

struct xrisp_power_up {
	__u32 scene;
	__u8 domain[MAX_XISP_DOMAIN_NUM];
};

struct xrisp_power_down {
	__u8 direct_powerdown;
	__u8 domain[MAX_XISP_DOMAIN_NUM];
};

struct xrisp_dvfs {
	__u8 dvfs_set;
};

struct xrisp_msg_buffer_register {
	__u64 buf_handle;
};

struct xrisp_msg_buffer_unregister {
	__u64 buf_handle;
};

struct xrisp_buf_idx_release {
	__u32 idx;
};

struct xrisp_msg_send {
	__u64 msg_id;
	__u32 size;
	void *buffer;
};

struct xrisp_timestamp {
	__u64 time_ns;
};

struct xrisp_ocm_buf_info {
	__s32 fd;
	__u64 hwaddr;
	__u32 size;
};

enum ioc_camera_cmd {
	CAMERA_IOC_CMD_INVALID = 0,
	CAMERA_IOC_CMD_SENSOR_POWER_UP,
	CAMERA_IOC_CMD_SENSOR_POWER_DOWN,
	CAMERA_IOC_CMD_ISP_POWER_UP,
	CAMERA_IOC_CMD_ISP_POWER_DOWN,
	CAMERA_IOC_CMD_DVFS,
	CAMERA_IOC_CMD_ISP_MSG_SEND,
	CAMERA_IOC_CMD_MSG_BUFFER_REGISTER,
	CAMERA_IOC_CMD_MSG_BUFFER_UNREGISTER,
	CAMERA_IOC_CMD_GET_TIMESTAMP,
	CAMERA_IOC_CMD_FLASH,
	CAMERA_IOC_CMD_GET_OCM_BUF,
	CAMERA_IOC_CMD_PUT_OCM_BUF,
	CAMERA_IOC_CMD_OCM_LINK,
	CAMERA_IOC_CMD_OCM_UNLINK,
	CAMERA_IOC_CMD_ISP_RPC_SEND,
	CAMERA_IOC_CMD_MSG_BUFFER_IDX_RELEASE,
	CAMERA_IOC_CMD_MAX,
};

enum xrisp_cam_tof {
	XRISP_CAM_TOF_IOCTL_POWER_OFF = 0,
	XRISP_CAM_TOF_IOCTL_POWER_ON,
	XRISP_CAM_TOF_IOCTL_CHIP_INIT,
	XRISP_CAM_TOF_IOCTL_XTALK_CALIB,
	XRISP_CAM_TOF_IOCTL_XTALK_CONFIG,
	XRISP_CAM_TOF_IOCTL_OFFSET_CALIB,
	XRISP_CAM_TOF_IOCTL_OFFSET_CONFIG,
	XRISP_CAM_TOF_IOCTL_SINGLE_RANGE,
	XRISP_CAM_TOF_IOCTL_START_RANGE,
	XRISP_CAM_TOF_IOCTL_STOP_RANGE,
	XRISP_CAM_TOF_IOCTL_RANGE_DATA,
	XRISP_CAM_TOF_IOCTL_PERIOD,
	XRISP_CAM_TOF_IOCTL_READ_REG,
	XRISP_CAM_TOF_IOCTL_WRITE_REG,
	XRISP_CAM_TOF_IOCTL_POWER_MODE,
	XRISP_CAM_TOF_IOCTL_GET_INFO,
	XRISP_CAM_TOF_IOCTL_SET_SPEED_HZ,
};

struct xrisp_cam_tof_range_data {
	__s16 distance;
	__u32 peak;
	__u8  confidence;
	__u32 noise;
	__u32 ambient_rate;
	__u32 integral_times;
	__u64 time_ns;
};

struct xrisp_cam_tof_calib_data {
	__s8  xtalk_cal;
	__u16 xtalk_peak;
	__u8  maxratio;
	__s16 offset_cal;
	__s16 offset_mili;
};


#endif
