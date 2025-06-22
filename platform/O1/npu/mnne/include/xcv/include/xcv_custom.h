// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef XCV_CUSTOM_H
#define XCV_CUSTOM_H

#include <stddef.h>
#include <stdint.h>
#include "xcv/include/xcv_config.h"

int xcvCustomFunc(xcvCommonParam commonParam,
                  void*          specParam,
                  uint32_t       specParamSize,
                  void*          ioBuf[8]);
#endif
