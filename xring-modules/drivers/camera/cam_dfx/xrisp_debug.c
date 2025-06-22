// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xrisp_debug.h"

static struct dentry *xrisp_debugfs_root;
uint64_t trigger_edr_report = -1;

int xrisp_debugfs_init(void)
{
	xrisp_debugfs_root = debugfs_create_dir(XRISP_DEBUGFS_ROOT_NAME, NULL);
	if (IS_ERR_OR_NULL(xrisp_debugfs_root)) {
		xrisp_debugfs_root = NULL;
		return PTR_ERR(xrisp_debugfs_root);
	}
	debugfs_create_u64("trigger_edr_report", 0600, xrisp_debugfs_root, &trigger_edr_report);
	return 0;
}

void xrisp_debugfs_exit(void)
{
	debugfs_remove(xrisp_debugfs_root);
	xrisp_debugfs_root = NULL;
}


struct dentry *xrisp_debugfs_get_root(void)
{
	return xrisp_debugfs_root;
}
