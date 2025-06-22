// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _XRING_VPU_LOG_H_
#define _XRING_VPU_LOG_H_

enum {
	LOGLVL_ERROR    = 0,		// log error
	LOGLVL_WARNING  = 1,		// log warning msg
	LOGLVL_INFO     = 2,		// log info msg
	LOGLVL_DEBUG    = 3,		// log all debug msg
	LOGLVL_VERBOSE  = 4,		// log all
	LOGLVL_NUM,
};

static char *kloglevel[LOGLVL_NUM] = {"E", "W", "I", "D","V"};

#define vpu_klog_base(tag, lvl, fmt, ...) {	\
	if (lvl == LOGLVL_ERROR)	\
		pr_err(tag "[%s:%d]: " fmt, kloglevel[lvl], __func__, __LINE__, ##__VA_ARGS__);	\
	else if (lvl == LOGLVL_WARNING)	\
		pr_warn(tag "[%s:%d]: " fmt, kloglevel[lvl], __func__, __LINE__, ##__VA_ARGS__);	\
	else if (lvl >= LOGLVL_INFO)	\
		pr_info(tag "[%s:%d]: " fmt, kloglevel[lvl], __func__, __LINE__, ##__VA_ARGS__);	\
}
#endif


