// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <soc/xring/display/panel_event_notifier.h>
#include "dsi_panel_event_notifier.h"
#include "dpu_log.h"

struct event_list {
	struct list_head head;
};

struct xring_panel_event_entry {
	enum xring_panel_event_tag tag;
	enum xring_panel_event_client client;
	enum xring_panel_event_type type;
	struct device_node *panel_node;
	xring_panel_event_handler handler;
	void *priv;

	struct list_head node;
};

static struct event_list event_list[DRM_PANEL_EVENT_MAX];
static struct mutex event_lock[DRM_PANEL_EVENT_MAX];

#define check_and_return(condition, ret, msg, ...) \
	do { \
		if (condition) { \
			DSI_ERROR(msg, ##__VA_ARGS__); \
			return ret; \
		} \
	} while (0)

#define check_and_return_void(condition, msg, ...) \
	do { \
		if (condition) { \
			DSI_ERROR(msg, ##__VA_ARGS__); \
			return; \
		} \
	} while (0)

static inline bool is_tag_valid(enum xring_panel_event_tag tag)
{
	return tag < XRING_PANEL_EVENT_TAG_MAX && tag >= 0;
}

static inline bool is_type_valid(enum xring_panel_event_type type)
{
	return type < DRM_PANEL_EVENT_MAX && type >= 0;
}

static inline bool is_client_valid(enum xring_panel_event_client client)
{
	return client < XRING_PANEL_EVENT_CLIENT_MAX && client >= 0;
}

static char *event_type_str_table[] = {
	"DRM_PANEL_EVENT_ALL",
	"DRM_PANEL_EVENT_BLANK",
	"DRM_PANEL_EVENT_UNBLANK",
	"DRM_PANEL_EVENT_BLANK_LP",
	"DRM_PANEL_EVENT_FPS_CHANGE",
	"DRM_PANEL_EVENT_HBM_ON",
	"DRM_PANEL_EVENT_HBM_OFF",
	"DRM_PANEL_EVENT_MAX",
};

static char *event_tag_str_table[] = {
	"XRING_PANEL_EVENT_TAG_NONE",
	"XRING_PANEL_EVENT_TAG_PRIMARY",
	"XRING_PANEL_EVENT_TAG_SECONDARY",
	"XRING_PANEL_EVENT_TAG_MAX",
};

static char *event_client_str_table[] = {
	"XRING_PANEL_EVENT_CLIENT_PRIMARY_TOUCH",
	"XRING_PANEL_EVENT_CLIENT_SECONDARY_TOUCH",
	"XRING_PANEL_EVENT_CLIENT_PRIMARY_FINGERPRINT",
	"XRING_PANEL_EVENT_CLIENT_SECONDARY_FINGERPRINT",
	"XRING_PANEL_EVENT_CLIENT_PRIMARY_CHARGER",
	"XRING_PANEL_EVENT_CLIENT_SECONDARY_CHARGER",
	"XRING_PANEL_EVENT_CLIENT_PRIMARY_THERMAL",
	"XRING_PANEL_EVENT_CLIENT_SECONDARY_THERMAL",
	"XRING_PANEL_EVENT_NOTIFIER_CLIENT_KEYBOARD",
	"XRING_PANEL_EVENT_CLIENT_PRIMARY_WIFI",
	"XRING_PANEL_EVENT_CLIENT_SECONDARY_WIFI",
	"XRING_PANEL_EVENT_CLIENT_MAX",
};

static void *_register(
		enum xring_panel_event_tag tag,
		enum xring_panel_event_client client,
		enum xring_panel_event_type type,
		struct device_node *panel_node,
		xring_panel_event_handler handler,
		void *priv)
{
	struct xring_panel_event_entry *entry;

	check_and_return(!is_tag_valid(tag), NULL,
			"invalid xring_panel_event_tag: %d\n", tag);
	check_and_return(!is_client_valid(client), NULL,
			"invalid xring_panel_event_client: %d\n", client);
	check_and_return(!is_type_valid(type), NULL,
			"invalid xring_panel_event_type: %d\n", type);
	check_and_return(panel_node == NULL, NULL,
			"invalid panel_node: %p\n", panel_node);
	check_and_return(handler == NULL, NULL,
			"invalid callback function: %p\n", handler);

	entry = kzalloc(sizeof(struct xring_panel_event_entry), GFP_KERNEL);
	check_and_return(entry == NULL, NULL, "failed to alloc memory\n");

	entry->tag = tag;
	entry->client = client;
	entry->type = type;
	entry->panel_node = panel_node;
	entry->handler = handler;
	entry->priv = priv;

	DSI_INFO("tag: %s\n", event_tag_str_table[entry->tag]);
	DSI_DEBUG("client: %s\n", event_client_str_table[entry->client]);
	DSI_DEBUG("event type: %s\n", event_type_str_table[entry->type]);

	mutex_lock(&event_lock[type]);
	list_add_tail(&entry->node, &event_list[type].head);
	mutex_unlock(&event_lock[type]);

	DSI_DEBUG("register panel event done, cookie = %p\n", entry);

	return entry;
}

void *xring_panel_event_notifier_register(
		enum xring_panel_event_tag tag,
		enum xring_panel_event_client client,
		struct device_node *panel_node,
		xring_panel_event_handler handler,
		void *priv)
{
	return _register(tag, client, DRM_PANEL_EVENT_ALL, panel_node, handler, priv);
}
EXPORT_SYMBOL(xring_panel_event_notifier_register);

void *xring_panel_event_notifier_register_with_type(
		enum xring_panel_event_tag tag,
		enum xring_panel_event_client client,
		enum xring_panel_event_type type,
		struct device_node *panel_node,
		xring_panel_event_handler handler,
		void *priv)
{
	return _register(tag, client, type, panel_node, handler, priv);
}
EXPORT_SYMBOL(xring_panel_event_notifier_register_with_type);

void xring_panel_event_notifier_unregister(void *cookie)
{
	struct xring_panel_event_entry *entry;
	enum xring_panel_event_type type;

	check_and_return_void(cookie == NULL, "invalid cookie:%p", cookie);

	entry = (struct xring_panel_event_entry *)cookie;
	type = entry->type;

	DSI_INFO("unregister panel event, cookie = %p\n", cookie);

	mutex_lock(&event_lock[type]);
	list_del(&entry->node);
	kfree(entry);
	mutex_unlock(&event_lock[type]);

	DSI_INFO("unregister panel event done\n");
}
EXPORT_SYMBOL(xring_panel_event_notifier_unregister);

static void _trigger(enum xring_panel_event_type type,
		enum xring_panel_event_tag tag,
		struct xring_panel_event_notification *notification)
{
	struct xring_panel_event_entry *entry = NULL;

	list_for_each_entry(entry, &event_list[type].head, node) {
		/* @tag is ignored */
		if (entry->panel_node &&
				entry->panel_node == notification->panel_node) {
			DSI_DEBUG("panel event trigger\n");
			DSI_DEBUG("tag: %s\n", event_tag_str_table[entry->tag]);
			DSI_DEBUG("client: %s\n", event_client_str_table[entry->client]);
			DSI_DEBUG("event type: %s\n", event_type_str_table[notification->type]);
			if (entry->handler)
				entry->handler(tag, notification, entry->priv);
			else
				DSI_ERROR("panel event callback func is NULL\n");
		}
	}
}

void xring_panel_event_notifier_trigger(
		enum xring_panel_event_tag tag,
		struct xring_panel_event_notification *notification)
{
	ktime_t start_ktime;
	s64 elapsed_us;

	start_ktime = ktime_get();
	check_and_return_void(!is_tag_valid(tag),
			"invalid xring_panel_event_tag: %d\n", tag);
	check_and_return_void(notification == NULL,
			"invalid notification: %p\n", notification);
	check_and_return_void(!is_type_valid(notification->type),
			"invalid xring_panel_event_type: %d\n", notification->type);
	check_and_return_void(notification->panel_node == NULL,
			"invalid panel: %p\n", notification->panel_node);

	_trigger(notification->type, tag, notification);
	_trigger(DRM_PANEL_EVENT_ALL, tag, notification);
	elapsed_us = ktime_us_delta(ktime_get(), start_ktime);
	DSI_INFO("notify event %s cost %d.%d(ms)\n", event_type_str_table[notification->type],
			(int)(elapsed_us / 1000), (int)(elapsed_us % 1000));
}
EXPORT_SYMBOL(xring_panel_event_notifier_trigger);

void dsi_panel_event_notifier_init(void)
{
	int i;

	for (i = 0; i < DRM_PANEL_EVENT_MAX; i++) {
		INIT_LIST_HEAD(&event_list[i].head);
		mutex_init(&event_lock[i]);
	}
}

void dsi_panel_event_notifier_deinit(void)
{
	struct xring_panel_event_entry *pos;
	int i;

	for (i = 0; i < DRM_PANEL_EVENT_MAX; i++) {
		mutex_destroy(&event_lock[i]);
		while (event_list[i].head.next != &event_list[i].head) {
			pos = list_entry(event_list[i].head.next,
					struct xring_panel_event_entry, node);
			list_del(&pos->node);
			kfree(pos);
		}
	}
}

void dsi_panel_event_notifier_trigger(enum xring_panel_event_type type,
		struct dsi_panel *panel)
{
	struct xring_panel_event_notification notification = {0};
	enum xring_panel_event_tag tag;

	notification.type = type;
	notification.data.early_trigger = 0;
	notification.panel_node = panel->drm_panel.dev->of_node;
	tag = panel->panel_info.is_primary ? XRING_PANEL_EVENT_TAG_PRIMARY :
			XRING_PANEL_EVENT_TAG_SECONDARY;
	xring_panel_event_notifier_trigger(tag, &notification);
	DSI_DEBUG("dsi panel event %s trigger\n", dsi_panel_event_name_get(type));
}

char *dsi_panel_event_name_get(enum xring_panel_event_type event_type)
{
	switch (event_type) {
	case DRM_PANEL_EVENT_ALL:
		return "ALL";
	case DRM_PANEL_EVENT_BLANK:
		return "BLANK";
	case DRM_PANEL_EVENT_UNBLANK:
		return "UNBLANK";
	case DRM_PANEL_EVENT_BLANK_LP:
		return "BLANK_LP";
	case DRM_PANEL_EVENT_FPS_CHANGE:
		return "BLANK_LP";
	case DRM_PANEL_EVENT_HBM_ON:
		return "BLANK_LP";
	case DRM_PANEL_EVENT_HBM_OFF:
		return "BLANK_LP";
	default:
		return "INVALID";
	}
}
