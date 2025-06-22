/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRSE_CMD_H__
#define __XRSE_CMD_H__

#define XRSE_CMD_MAX_NUM (12)
#define XRSE_PARAM_MAX_NUM (10)
#define XRSE_PER_CMD_LENGTH_MAX (32)
#define XRSE_RES_BUF_LENGTH_MAX (1024)

int xrse_parse_cmd_store(const char *buff, unsigned int buff_size);
int xrse_get_module_show(char *buf);

int xrse_cmd_res_store(char *buf, int buf_size);
int xrse_cmd_res_show(char *buf);

#endif
