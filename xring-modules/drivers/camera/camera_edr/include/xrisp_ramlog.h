/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_RAMLOG_H_
#define _XRISP_RAMLOG_H_

#define RAMLOG_ACTION_CLOSE          0
#define RAMLOG_ACTION_OPEN           1
#define RAMLOG_ACTION_READ           2

/* Return number of unread characters in the log buffer */
#define RAMLOG_ACTION_SIZE_UNREAD    9

#define RAMLOG_FROM_READER           0
#define RAMLOG_FROM_PROC             1

#define CONSOLE_EXT_LOG_MAX     8192

/*
 * descs/infos/log data buffer used in AP side
 * cannot use buffer addr read from lrb because it's addr of FW
 */
extern struct lrb_desc    *ap_descs;
extern struct lrb_info    *ap_infos;
extern char *ap_data_rb;
extern void *ramtrace_buf;
extern void *wdt_buf;
extern struct tm edr_time;

int do_xrisp_ramlog(int type, char __user *buf, int len, int source);
int ramlog_set_default_cfg(void *vaddr);
void ramlog_loglevel_set(int level);


#endif
