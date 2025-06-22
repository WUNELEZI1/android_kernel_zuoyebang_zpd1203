// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XGPU_COMMON_H__
#define __XGPU_COMMON_H__

struct ExtraBufferInfo {

    int dstWidth;
    int dstHeight;
};

enum ApiType : uint8_t {
    OPENGLES = 0,
    OPENCL = 1,
    VULKAN = 2,
};

enum OpType : uint8_t {
    YUV420SP_TO_YUV420P = 0,
    RGBA_SCALING = 1,
    YUV_SCALING = 2,
    YUV420SP_TO_RGBA = 3,
};
#endif
