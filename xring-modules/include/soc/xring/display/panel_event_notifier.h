/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
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

#ifndef _XRING_PANEL_EVENT_NOTIFIER_H_
#define _XRING_PANEL_EVENT_NOTIFIER_H_

#include <linux/types.h>
#include <linux/device.h>

enum xring_panel_event_type {
	DRM_PANEL_EVENT_ALL = 0,
	DRM_PANEL_EVENT_BLANK,
	DRM_PANEL_EVENT_UNBLANK,
	DRM_PANEL_EVENT_BLANK_LP,
	DRM_PANEL_EVENT_FPS_CHANGE,
	DRM_PANEL_EVENT_HBM_ON,
	DRM_PANEL_EVENT_HBM_OFF,
	DRM_PANEL_EVENT_MAX,
};

enum xring_panel_event_tag{
	XRING_PANEL_EVENT_TAG_NONE = 0,
	XRING_PANEL_EVENT_TAG_PRIMARY,
	XRING_PANEL_EVENT_TAG_SECONDARY,
	XRING_PANEL_EVENT_TAG_MAX,
};

enum xring_panel_event_client {
	XRING_PANEL_EVENT_CLIENT_PRIMARY_TOUCH = 0,
	XRING_PANEL_EVENT_CLIENT_SECONDARY_TOUCH,
	XRING_PANEL_EVENT_CLIENT_PRIMARY_FINGERPRINT,
	XRING_PANEL_EVENT_CLIENT_SECONDARY_FINGERPRINT,
	XRING_PANEL_EVENT_CLIENT_PRIMARY_CHARGER,
	XRING_PANEL_EVENT_CLIENT_SECONDARY_CHARGER,
	XRING_PANEL_EVENT_CLIENT_PRIMARY_THERMAL,
	XRING_PANEL_EVENT_CLIENT_SECONDARY_THERMAL,
	XRING_PANEL_EVENT_NOTIFIER_CLIENT_KEYBOARD,
	XRING_PANEL_EVENT_CLIENT_PRIMARY_WIFI,
	XRING_PANEL_EVENT_CLIENT_SECONDARY_WIFI,
	XRING_PANEL_EVENT_CLIENT_MAX
};

struct xring_panel_event_notification_data {
	u32 old_fps;
	u32 new_fps;
	bool early_trigger;
};

struct xring_panel_event_notification {
	enum xring_panel_event_type type;
	struct device_node *panel_node;

	struct xring_panel_event_notification_data data;
};

/**
 * xring_panel_event_handler - callback function registered by client
 *
 * @tag: passed in by panel, indicate panel type
 * @notification: passed in by panel, including panel data
 * @priv: private data of client, passed in by client when calling
 *     xring_panel_event_register()
 *
 * client that needed to align with panel event could define its own callback
 * function in this format.
 */
typedef void (*xring_panel_event_handler)(
		enum xring_panel_event_tag tag,
		struct xring_panel_event_notification *notification,
		void *priv);

/**
 * xring_panel_event_notifier_register - register a callback function for
 *     all kinds of panel event
 *
 * @tag: indicate panel type
 * @client: indicate client type
 * @panel: indicate which panel should trigger the handler function
 * @handler: callback function defined by client
 * @priv: private data of client, will be passed to @handler when panel event is
 *     triggered
 *
 * clients that needed to alien with panel event could define its own callback
 * function, than call this function to register it.
 */
void *xring_panel_event_notifier_register(
		enum xring_panel_event_tag tag,
		enum xring_panel_event_client client,
		struct device_node *panel_node,
		xring_panel_event_handler handler,
		void *priv);

/**
 * xring_panel_event_notifier_register_with_type - register a callback function
 *     for specified kind of panel event
 */
void *xring_panel_event_notifier_register_with_type(
		enum xring_panel_event_tag tag,
		enum xring_panel_event_client client,
		enum xring_panel_event_type type,
		struct device_node *panel_node,
		xring_panel_event_handler handler,
		void *priv);

/**
 * xring_panel_event_notifier_unregister - unregister a callback function for
 *     panel event
 *
 * @cookie: cookie is obtained from the return of
 *     xring_panel_event_notifier_register()
 */
void xring_panel_event_notifier_unregister(void *cookie);

/**
 * xring_panel_event_notifier_trigger - trigger the callback function registered
 *     by clients
 * @tag: panel type
 * @notification: notification sended to clients
 */
void xring_panel_event_notifier_trigger(
		enum xring_panel_event_tag tag,
		struct xring_panel_event_notification *notification);

#endif /* _XRING_PANEL_EVENT_NOTIFIER_H_ */
