/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_IPC_H
#define _XR_USB_ADSP_IPC_H

#include <acore/fk-acore-define.h>
#include <linux/kernel.h>

#define XR_USB_ADSP_MSG_LEN_MAX 32U
#define XR_USB_ADSP_MSG_DATA_LEN_MAX                                           \
	(XR_USB_ADSP_MSG_LEN_MAX - sizeof(struct xr_usb_adsp_msg_header))

struct xr_usb_adsp_ipc;
struct xring_usb_adsp;

/* Message start from ADSP */
enum xr_usb_adsp_msg_id {
	XR_USB_ADSP_MSG_ID_START = 0x0,
	XR_USB_ADSP_MSG_ID_FIRST_HID_IN,
	XR_USB_ADSP_MSG_ID_COMMON_ACK,
	XR_USB_ADSP_MSG_ID_PORT_CHANGE,
	XR_USB_ADSP_MSG_ID_URB_GIVEBACK,
	XR_USB_ADSP_MSG_ID_MAX,
};

/* Message start from AP */
enum xr_usb_adsp_ap_msg_id {
	XR_USB_ADSP_AP_MSG_ID_STARTINFO = 0x0,
	XR_USB_ADSP_AP_MSG_ID_STOP,
	XR_USB_ADSP_AP_MSG_ID_URB_ENQUEUE,
	XR_USB_ADSP_AP_MSG_ID_URB_DEQUEUE,
	XR_USB_ADSP_AP_MSG_ID_RESET_DEVICE,
	XR_USB_ADSP_AP_MSG_ID_SET_SR_STATUS,
	XR_USB_ADSP_AP_MSG_ID_GET_SR_STATUS,
	XR_USB_ADSP_AP_MSG_ID_DEBUG,
	XR_USB_ADSP_AP_MSG_ID_MAX,
};

struct xr_usb_adsp_msg_header {
	u16 id;
	u16 reserved1;
	u16 len;
	u8 reserved2;
	u8 msg_source;
} __packed;

/* Specific message definitions */
struct xr_usb_adsp_shared_buf_data {
	u32 base;
	u32 len;
};

struct xr_usb_adsp_first_hid_in_data {
	u8 ep_index;
	u32 actual_len;
	int status;
};

struct xr_usb_adsp_common_ack_data {
	u16 id;
	int result;
};

struct xr_usb_adsp_port_data {
	u32 portsc;
};

enum xr_usb_adsp_sr_status {
	ADSP_USB_BUS_SUSPEND = 0,
	ADSP_USB_BUS_RUNNING,
	ADSP_USB_BUS_STATUS_INVALID,
};

struct xr_usb_adsp_sr_status_data {
	u32 status;
};

enum xr_usb_adsp_debug_mode {
	XR_USB_ADSP_DEBUG_TEST_RESET = 0,
	XR_USB_ADSP_DEBUG_TEST_IPCMISS,
	XR_USB_ADSP_DEBUG_TEST_ISOC,
	XR_USB_ADSP_DEBUG_TEST_INVALID,
};

enum xr_usb_adsp_debug_point {
	XR_USB_ADSP_DEBUG_AT_START = 0,
	XR_USB_ADSP_DEBUG_AT_STOP,
	XR_USB_ADSP_DEBUG_AT_HID_DATA,
	XR_USB_ADSP_DEBUG_AT_CTL_DATA,
	XR_USB_ADSP_DEBUG_AT_ISOC_DATA,
	XR_USB_ADSP_DEBUG_AT_INVALID,
};

struct xr_usb_adsp_debug_data {
	u16 mode;
	u16 point;
};

struct xr_usb_adsp_deq_urb_data {
	u32 base;
	u32 len;
	u32 first_hid : 1;
};

struct xr_usb_adsp_msg {
	struct xr_usb_adsp_msg_header header;
	union {
		u8 data[XR_USB_ADSP_MSG_DATA_LEN_MAX];
		struct xr_usb_adsp_shared_buf_data buf_data;
		struct xr_usb_adsp_first_hid_in_data hid_in_data;
		struct xr_usb_adsp_common_ack_data ack_data;
		struct xr_usb_adsp_port_data port_data;
		struct xr_usb_adsp_sr_status_data sr_data;
		struct xr_usb_adsp_debug_data debug_data;
		struct xr_usb_adsp_deq_urb_data deq_urb_data;
	};
};

#define usb_adsp_init_msg_header(msg, x, y)                                    \
	do {                                                                   \
		(msg)->header.id = x;                                          \
		(msg)->header.len = (u16)((y) + sizeof((msg)->header));        \
		(msg)->header.msg_source = AP_USB_MSG;                         \
	} while (0)

/* Definitions must be consistent with those in adsp */
struct adsp_xhci_ep_info {
	u32 ep_index;
	u32 ring_type;
	u32 ring_base;
	u32 ring_seg_num;
	u32 ring_enqueue;
	u32 ring_dequeue;
};

#define ADSP_XHCI_EP_MAX 31
struct adsp_xhci_info {
	u32 cmd_ring_base;
	u32 cmd_ring_enqueue;
	u32 evt_ring_cycle;
	int slot_id;
	u32 out_ctx;
	u32 in_ctx;
	u32 ep_mask;
	struct adsp_xhci_ep_info eps[ADSP_XHCI_EP_MAX];
};

/*
 * Structure of udev audio info
 * -----------------
 * | intf_info     |
 * -----------------
 * | intf_alt_info |
 * -----------------
 * | ep_info       |
 * |    .          |
 * |    .          |
 * -----------------
 * | intf_alt_info |
 * |    .          |
 * |    .          |
 * | intf_info     |
 * |    .          |
 * |    .          |
 * -----------------
 */
struct adsp_udev_ep_info {
	u32 pipe;
	u16 max_packet_size;
	u8 interval;
} __aligned(sizeof(u32));

struct adsp_udev_intf_alt_info {
	u8 alt_num;
	u8 ep_max;
} __aligned(sizeof(u32));

struct adsp_udev_intf_info {
	u8 intf_num;
	u8 alt_max;
} __aligned(sizeof(u32));

struct adsp_udev_audio_info {
	u8 ctrl_intf_max;
	u8 data_intf_max;
} __aligned(sizeof(u32));

#define USB_ADSP_HID_EP_MAX 4
#define USB_AUDIO_DEV_QUIRK_USE_HID1 BIT(1)
struct adsp_udev_hid_info {
	u8 ep_num;
	u8 valid_hid;
	struct adsp_udev_ep_info eps[USB_ADSP_HID_EP_MAX];
} __aligned(sizeof(u32));

#define USB_ADSP_AUDIO_CTRL_NUM 2
#define USB_ADSP_MAX_AUDIO_DATA_NUM 2
#define USB_ADSP_MAX_AUDIO_INTF_NUM                                            \
	(USB_ADSP_AUDIO_CTRL_NUM + USB_ADSP_MAX_AUDIO_DATA_NUM)

#define AUDIO_UDEV_FLGA_HS BIT(0)
#define AUDIO_UDEV_FLGA_HAS_HID BIT(1)
#define AUDIO_UDEV_FLGA_REMOTE_WAKEUP BIT(2)
struct audio_udev_info {
	u32 flags;
	u32 quirks;
	u32 ep_data_pending_mask;
	struct adsp_udev_hid_info hid;
	struct adsp_udev_audio_info audio;
};

struct usb_adsp_start_info {
	struct adsp_xhci_info xhci_info;
	struct audio_udev_info udev_info;
};

struct usb_adsp_stop_info {
	u32 cmd_ring_queue;
	u32 event_ring_queue;
	u32 ep0_ring_queue;
	u32 hid_ep_index;
	u32 hid_ring_queue;
};

#define USB_ADSP_URB_LEN_MAX 256U
struct usb_adsp_urb_data {
	u32 pipe;
	u32 len;
	u32 actual_len;
	int status;
	struct usb_ctrlrequest ctrl;
	u8 data[];
};
/*------------------------------------------------------------*/

struct xr_usb_adsp_ipc *xr_usb_adsp_ipc_create(struct device *dev,
					       struct xring_usb_adsp *usb_adsp);
void xr_usb_adsp_ipc_destroy(struct device *dev, struct xr_usb_adsp_ipc *adsp_ipc);
void xr_usb_adsp_ipc_suspend(struct xr_usb_adsp_ipc *ipc);
void xr_usb_adsp_ipc_resume(struct xr_usb_adsp_ipc *ipc);
int usb_adsp_send_start_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				     u32 addr, u32 len, unsigned long timeout);
int usb_adsp_send_stop_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				    u32 addr, u32 len, unsigned long timeout);
int usb_adsp_send_get_sr_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				      unsigned long timeout);
int usb_adsp_send_set_sr_msg(u32 sr_status);
int usb_adsp_send_urb_msg(u32 addr, u32 len);
int usb_adsp_send_debug_msg(u16 mode, u16 point);
int usb_adsp_send_deq_urb_msg(u32 addr, u32 len, bool first_hid);

#endif /* _XR_USB_ADSP_IPC_H */
