// SPDX-License-Identifier: GPL-2.0-or-later
//
// Copyright (c) 2024 Xring.

#include <linux/module.h>
#include <linux/init.h>
#include "xr_ipc_main.h"

static int __init xr_ipc_init(void)
{
	int ret;

	ret = xr_ipc_dev_init();
	if (ret)
		pr_err("xr_ipc_dev_init fail, ret=%d\n", ret);

	ret = xr_ipc_prot_init();
	if (ret)
		pr_err("xr_ipc_prot_init fail, ret=%d\n", ret);

	return ret;
}

static void __exit xr_ipc_exit(void)
{
	xr_ipc_dev_exit();

	xr_ipc_prot_exit();
}

module_init(xr_ipc_init);
module_exit(xr_ipc_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring IPC driver");
MODULE_LICENSE("GPL v2");
