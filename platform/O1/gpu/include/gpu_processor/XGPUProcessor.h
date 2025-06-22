// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XGPU_PROCESSOR_H__
#define __XGPU_PROCESSOR_H__
#include "toolkits/XGPUToolkit.h"
#include <XGPUCommon.h>
#include <memory>

class XGPUProcessor {
public:
    XGPUProcessor() : mGPUToolkit(nullptr) {};
    bool createToolkit(ApiType api, OpType op);
    bool run(AHardwareBuffer* src,
             AHardwareBuffer* dst,
             ExtraBufferInfo* pExtraInfo = nullptr);

private:
    std::shared_ptr<XGPUToolkit> mGPUToolkit;
};
#endif
