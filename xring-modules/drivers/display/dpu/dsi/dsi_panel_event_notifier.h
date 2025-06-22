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

#ifndef _DSI_PANEL_EVENT_NOTIFIER_H_
#define _DSI_PANEL_EVENT_NOTIFIER_H_

#include <soc/xring/display/panel_event_notifier.h>
#include "dsi_panel.h"

void dsi_panel_event_notifier_init(void);

void dsi_panel_event_notifier_deinit(void);

void dsi_panel_event_notifier_trigger(enum xring_panel_event_type type,
		struct dsi_panel *panel);

char *dsi_panel_event_name_get(enum xring_panel_event_type event_type);

#endif /* _DSI_PANEL_EVENT_NOTIFIER_H_ */
