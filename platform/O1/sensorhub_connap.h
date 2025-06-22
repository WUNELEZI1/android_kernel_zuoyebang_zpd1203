// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __SENSORHUB_CONNAP_H__
#define __SENSORHUB_CONNAP_H__

enum shub_conn_module_e : unsigned char {
	MODULE_GNSS = 0,
	MODULE_BT,
	MODULE_WIFI,
	MODULE_MODEM,
	MODULE_CONNSYS,
	MODULE_NUM_MAX
};

enum shub_conn_driver_state_e : unsigned char {
	DRIVER_STATE_OFF,
	DRIVER_STATE_ON,
	DRIVER_STATE_NUM_MAX
};

enum shub_conn_request_cmd_e : unsigned char {
	SHUB_REQUEST_OFF,
	SHUB_REQUEST_ON,
	SHUB_REQUEST_STATUS,
	SHUB_REQUEST_CMD_MAX
};

#endif
