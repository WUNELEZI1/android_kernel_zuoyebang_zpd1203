/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: tee mrmory monitor Kernel CA header file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#ifndef XRING_MITEE_MEMORY_MONITOR_H
#define XRING_MITEE_MEMORY_MONITOR_H

#include <linux/types.h>

void mitee_memory_monitor_init(void);
void mitee_memory_monitor_exit(void);

#endif
