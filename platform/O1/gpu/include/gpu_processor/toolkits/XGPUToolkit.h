// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XGPU_TOOLKIT_H__
#define __XGPU_TOOLKIT_H__

#include <vndk/hardware_buffer.h>
#include <XGPUCommon.h>

class XGPUToolkit {
public:
    XGPUToolkit() {};
    virtual ~XGPUToolkit() {};

    virtual bool prepare(AHardwareBuffer* src,
                         AHardwareBuffer* dst,
                         ExtraBufferInfo* pExtraInfo = nullptr) = 0;
    virtual void draw() = 0;
};
#endif
