/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 * Mdr public header for Kernel
*/
#ifndef __DFX_KERNEL__USER_H__
#define __DFX_KERNEL__USER_H__

#define DMD_EXTERN_CLIENT_MAX_BUF_SIZE	1024
#define MSG_MAX_LEN	DMD_EXTERN_CLIENT_MAX_BUF_SIZE
#define SUBLOGSAVE_OK	"sublogsave_ok"

enum nlmsg_flags
{
	GET_PID_FLAG = 0x5a5a,
	LOGSAVE_FLAG,
	LOGSAVE_FLAG_PRODUCT,
	LOGSAVE_FLAG_SUBSYS,
};

#endif
