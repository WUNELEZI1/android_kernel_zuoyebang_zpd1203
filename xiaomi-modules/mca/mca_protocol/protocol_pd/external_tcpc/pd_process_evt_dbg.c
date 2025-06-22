// SPDX-License-Identifier: GPL-2.0
/*
 *pd_process_evt_dbg.c
 *
 * pd driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "inc/pd_core.h"
#include "inc/tcpci_event.h"
#include "inc/pd_process_evt.h"

#ifdef CONFIG_USB_PD_CUSTOM_DBGACC

bool pd_process_event_dbg(struct pd_port *pd_port, struct pd_event *pd_event)
{
	/* Don't need to handle any PD message, Keep VBUS 5V, and using VDM */
	return false;
}

#endif /* CONFIG_USB_PD_CUSTOM_DBGACC */
