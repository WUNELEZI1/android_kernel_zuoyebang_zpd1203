// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _VDEC_MDR_H_
#define _VDEC_MDR_H_

#include "mdr_pub.h"
/**
 * mdr_vdec_exception_register - Register video decoder exception to mdr.
 *
 * @dev: The device structure for the device (must be a valid pointer)
 */
int mdr_vdec_exception_register(struct device *dev);
/**
 * xring_vdec_mdr_exception_unregister - Unregister video decoder exception to mdr.
 *
 * @dev: The device structure for the device (must be a valid pointer)
 */
int mdr_vdec_exception_unregister(struct device *dev);
#endif