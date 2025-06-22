/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */
#ifndef _MI_DISP_PARSE_H_
#define _MI_DISP_PARSE_H_


#include "dsi_panel.h"

int mi_dsi_panel_parse_esd_gpio_config(struct dsi_panel *panel);
int mi_dsi_panel_parse_config(struct dsi_panel *panel);

#endif /* _MI_DISP_PARSE_H_ */
