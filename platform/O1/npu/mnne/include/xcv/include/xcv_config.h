// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef XCV_CONFIG_H
#define XCV_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define XCV_API __attribute__ ((visibility ("default")))

typedef float  float32_t;
typedef double float64_t;
typedef double double64_t;

#define XCV_IMAGE_NUM_MAX      8
#define XCV_OP_FUNC_NUM_MAX    8
#define XCV_OP_IO_NUM_MAX      8

typedef enum {
    XCV_PRIORITY_LOW = 0,
    XCV_PRIORITY_MIDDLE,
    XCV_PRIORITY_HIGH,
    XCV_PRIORITY_MAX
}xcvPriority;

typedef enum {
    XCV_DATA_DUMP_OFF = 0,
    XCV_DATA_DUMP_ENQUEUE_CMD,
    XCV_DATA_DUMP_OUTPUT,
    XCV_DATA_DUMP_CMD_AND_OUT
} xcvDataDumpType;

typedef enum {
    XCV_PROFILING_DISABLE = 0,
    XCV_PROFILING_NORMAL,
    XCV_PROFILING_DETAIL
} xcvProfilingType;

typedef enum {
    XCV_POWER_LOW  = 0,
    XCV_POWER_MID  = 1,
    XCV_POWER_HIGH = 2,
    XCV_POWER_FULL = 3
} xcvOpPowerMode;

typedef struct xcvConfig {
    xcvPriority      mPriority{xcvPriority::XCV_PRIORITY_MIDDLE};
    xcvProfilingType mProfilingType{xcvProfilingType::XCV_PROFILING_DETAIL};
    uint32_t         mTimeOut = 0;
    char             mCustomLibPath[XCV_IMAGE_NUM_MAX][256]={{'\0'}};
    char             mCustomOpName[XCV_OP_FUNC_NUM_MAX][256]={{'\0'}};
    uint32_t         mLogClose = 1;
} xcvConfig;

typedef enum {
    XCV_SCALE_TYPE_NEAREST_NEIGHBOR = 0,
    XCV_SCALE_INTERPOLATION_TYPE_BILINEAR,
    XCV_SCALE_INTERPOLATION_TYPE_BICUBIC,
    XCV_SCALE_INTERPOLATION_TYPE_AREA,
    XCV_SCALE_INTERPOLATION_TYPE_LANCOZ
} xcvScaleType;

typedef enum {
    XCV_KERNEL_3x3 = 0,
    XCV_KERNEL_5x5,
    XCV_KERNEL_7x7,
    XCV_KERNEL_9x9,
    XCV_KERNEL_11x11,
    XCV_KERNEL_MAX,
    XCV_KERNEL_1x1
} xcvKernelType;

typedef enum {
    XCV_ZERO_PADDING        = 0,
    XCV_CONSTANT_PADDING    = 1,
    XCV_EDGE_PADDING        = 2,
    XCV_PADDING_REFLECT_101 = 3,
    XCV_PADDING_MAX         = 4
} xcvBorderType;

typedef enum {
    XCV_THRESH_TYPE_TOZERO_INV = 0,
    XCV_THRESH_TYPE_BINARY_INV,
    XCV_THRESH_TYPE_BINARY_INV_TRIANGLE
} xcvThreshType;



typedef struct {
    uint32_t mOpIndex;
    uint32_t mInputNum;
    uint32_t mOutputNum;
    uint32_t mIOSize[XCV_OP_IO_NUM_MAX];
    uint32_t mCycle;
} xcvCommonParam;

#endif
