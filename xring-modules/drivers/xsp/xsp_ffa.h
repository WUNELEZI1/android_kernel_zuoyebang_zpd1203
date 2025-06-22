/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: xring rust secure partition driver header file
 * Modify time: 2024-07-06
 */

#ifndef _XSP_FFA_H
#define _XSP_FFA_H

#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/uuid.h>


struct xsp_ffa_msg {
	union {
		unsigned long fid;
		unsigned long ret;
	};

	unsigned long data0;
	unsigned long data1;
	unsigned long data2;
	unsigned long data3;
};


#if IS_ENABLED(CONFIG_XRING_RSP)
int xrsp_ffa_direct_message(struct xsp_ffa_msg *msg);
#else
static inline int xrsp_ffa_direct_message(struct xsp_ffa_msg *msg)
{
	if (!msg) {
		pr_err("%s not implement, msg is null ptr, error\n", __func__);
		return -EOPNOTSUPP;
	}
	msg->ret = 0;
	pr_err("%s not implement, return 0\n", __func__);
	return 0;
}
#endif

#endif


