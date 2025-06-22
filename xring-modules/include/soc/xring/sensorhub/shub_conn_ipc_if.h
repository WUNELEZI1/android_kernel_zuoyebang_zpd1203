/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024 XRing Technologies.Co.Ltd. All rights reserved.
 */

#ifndef __SHUB_CONN_IPC_IF_H__
#define __SHUB_CONN_IPC_IF_H__

#include <dt-bindings/xring/platform-specific/sensorhub_connap.h>

/* Kernel GNSS/BT/WIFI/MDM Driver send 'conn_to_shub_msg'
 * to Sensorhub to notify WCN/MDM MCU has powered up.
 */
struct conn_to_shub_msg {
	enum shub_conn_module_e module;
	enum shub_conn_driver_state_e state;
};

/* Sensorhub will send 'shub_conn_to_ap_msg' to request target MCU power up/down.
 * Then 'shub_conn_ipc_if' will call 'shub_conn_ipc_cb'(if registered) to
 * transmit shub request cmd to target module(e.g. MODULE_GNSS).
 *
 * Module should parse cmd to check target if correct and take data
 * to their context, calling shub_conn_notify_state after on/off done
 * to let shub know whether MCU has powered up successfully.
 */
struct shub_conn_to_ap_msg {
	enum shub_conn_module_e module;
	enum shub_conn_request_cmd_e request_cmd;
};

typedef void (*shub_conn_ipc_cb)(struct shub_conn_to_ap_msg *shub_request_msg);

/* WIFI/BT/GNSS/Modem use this to notify shub their status */
void shub_conn_notify_state(struct conn_to_shub_msg *to_shub_msg);

/* registe callback to recv shub msg */
void shub_conn_regist_callback(enum shub_conn_module_e module, shub_conn_ipc_cb cb_for_shub);

#endif /* __SHUB_CONN_IPC_IF_H__ */
