// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _VENC_MDR_H_
#define _VENC_MDR_H_

#include "mdr_pub.h"
/**
 * mdr_venc_exception_register - Register venc exception to mdr.
 *
 * @dev: The device structure for device (must be a valid pointer)
 */
int mdr_venc_exception_register(struct device *dev);
/**
 * mdr_venc_exception_unregister - Unregister venc exception to mdr.
 *
 * @hdev: The device structure for the device (must be a valid pointer)
 */
int mdr_venc_exception_unregister(struct device *dev);
#endif