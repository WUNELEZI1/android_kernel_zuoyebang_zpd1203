/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SECBOOT_STATUS_CHECK__
#define __SECBOOT_STATUS_CHECK__

#include <linux/types.h>

int xr_secboot_status_show(char *buf);
int xr_secboot_status_store(int argc, char *argv[]);
#endif
