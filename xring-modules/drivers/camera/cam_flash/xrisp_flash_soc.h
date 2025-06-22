/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_FLASH_SOC_H_
#define _XRISP_FLASH_SOC_H_

#include <linux/pinctrl/consumer.h>
#include "xrisp_flash_common.h"

int xrisp_pinctrl_state_set(struct pinctrl *pinctrl, char *pinctrl_name);
int xrisp_flash_parse_dts(struct xrisp_flash_data *flash_data);

#endif
