/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_UART_SWITCH_H__
#define __XRING_UART_SWITCH_H__

#define SCTRL_ALL_UART_MASK			0x3FF
#define LPCTRL_PERI_IOCTRL_CHAN2_TX_OFFSET	0x74
#define LPCTRL_PERI_IOCTRL_CHAN2_RX_OFFSET	0x78
#define LPCTRL_LPIS_IOC_CHAN4_TX_OFFSET		0x3C
#define LPCTRL_LPIS_IOC_CHAN4_RX_OFFSET		0x40

enum switch_mode {
	SWITCH_MODE_APB_REG = 0,
	SWITCH_MODE_IOMUX,
	SWITCH_MODE_MAX
};

#endif /* __XRING_UART_SWITCH_H__ */
