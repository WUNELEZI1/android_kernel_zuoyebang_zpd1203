/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024, X-Ring Technologies, Inc.  All Rights Reserved
 * Description: coresight etr config header
 */

#ifndef __CORESIGHT_ETR_CONFIG_H__
#define __CORESIGHT_ETR_CONFIG_H__


int toggle_coresight_atb_sh_clk_gate(bool);
void set_mainbus_autogt(bool);

#endif /* __CORESIGHT_ETR_CONFIG_H__ */