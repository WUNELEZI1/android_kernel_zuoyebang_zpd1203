/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __MPAM_USER_INTERFACE_H_
#define __MPAM_USER_INTERFACE_H_

#include "mpam_internal.h"

int mpam_msc_sysfs_interface_init(struct mpam_msc *msc);

int mpam_sysfs_interface_init(void);

#endif /* __MPAM_USER_INTERFACE_H_ */
