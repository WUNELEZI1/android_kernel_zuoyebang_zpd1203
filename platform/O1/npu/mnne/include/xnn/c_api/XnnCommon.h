// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef XNN_COMMON_H
#define XNN_COMMON_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XNN_API
#define XNN_API
#endif

#define XNN_SUCCESS 0

#define XNN_MIN_ERROR_COMMON              1000
#define XNN_MAX_ERROR_COMMON              1999
#define XNN_MIN_ERROR_CONFIG              2000
#define XNN_MAX_ERROR_CONFIG              2999
#define XNN_MIN_ERROR_PREDICTOR           3000
#define XNN_MAX_ERROR_PREDICTOR           3999
#define XNN_MIN_ERROR_RUNTIME             4000
#define XNN_MAX_ERROR_RUNTIME             4999
#define XNN_MIN_ERROR_TENSOR              5000
#define XNN_MAX_ERROR_TENSOR              5999
#define XNN_MIN_ERROR_BUFFER              6000
#define XNN_MAX_ERROR_BUFFER              6999
#define XNN_MIN_ERROR_GRAPH               7000
#define XNN_MAX_ERROR_GRAPH               7999
#define XNN_MIN_ERROR_MODEL               8000
#define XNN_MAX_ERROR_MODEL               8999

typedef enum {
    XNN_POWER_LOW       = 0,
    XNN_POWER_BALANCE   = 1,
    XNN_POWER_HIGH      = 2,
    XNN_POWER_FULL      = 3,
    XNN_POWER_NUM       = 4
} XnnPowerMode;

typedef enum {
    XNN_UNK    = 0,
    XNN_ARM    = 4,
    XNN_OPENCL = 5,
    XNN_ANY    = 6,
    XNN_NPU    = 8,
    XNN_F1NPU  = 100,
    XNN_NUM    = 101
} XnnTargetType;

typedef enum {
    XNN_PROFILE_LEVEL_DISABLE = 0,
    XNN_PROFILE_LEVEL_BASIC   = 1,
    XNN_PROFILE_LEVEL_DETAIL  = 2,
    XNN_PROFILE_LEVEL_NUM     = 3
} XnnProfileLevel;

typedef enum {
    XNN_PRIORITY_PREEMPT   = 0,
    XNN_PRIORITY_HIGH      = 1,
    XNN_PRIORITY_MID       = 2,
    XNN_PRIORITY_LOW       = 3,
    XNN_PRIORITY_NUM       = 4
} XnnPriority;

typedef enum {
    kFloat  = 1,
    kInt8   = 2,
    kInt32  = 3,
    kAny    = 4,
    kFP16   = 5,
    kBool   = 6,
    kInt64  = 7,
    kInt16  = 8,
    kUInt8  = 9,
    kFP64   = 10,
    kUInt16 = 11,
    kUInt32 = 12,
    kUInt64 = 13,
    kNUM = 15
} XnnPrecisionType;

typedef enum {
    XNN_QUANTIZE_NONE                    = 0,
    XNN_QUANTIZE_DYNAMIC_FIXED_POINT     = 1,
    XNN_QUANTIZE_ASYMM                   = 2,
    XNN_QUANTIZE_MAX,
} XnnQuantFormat;

typedef enum {
    XNN_INFO,
    XNN_WARNING,
    XNN_DEBUG,
    XNN_ERROR,
    XNN_CLOSE
} XnnLogLevel;

typedef enum {
    XNN_CREATE_NETWORK_FROM_NONE             = 0x00,
    XNN_CREATE_NETWORK_FROM_FILE             = 0x01,
    XNN_CREATE_NETWORK_FROM_MEMORY           = 0x02,
    XNN_CREATE_NETWORK_MODE_SHARE_COMMAND    = 0x100,
    XNN_CREATE_NETWORK_MODE_SHARE_WEIGHTS    = 0x400,
    XNN_CREATE_NETWORK_MAX,
} XnnNetworkCreateType;

typedef enum {
    XNN_SHARED_NONE        = 0x00,
    XNN_SHARED_WEAK        = 0x01,
    XNN_SHARED_WEIGHT      = 0x02,
    XNN_SHARED_CMD         = 0x03,
    XNN_SHARED_ALL         = 0x04,
    XNN_SHARED_NUM         = 0x05
} XnnNetworkShareMode;

typedef enum {
    XNN_COMMON_ERROR_MIN_ERROR     = XNN_MIN_ERROR_COMMON,
    XNN_COMMON_ERROR_INVALID_PARAM = XNN_MIN_ERROR_COMMON + 1,
    XNN_COMMON_NO_ERROR            = XNN_SUCCESS,
    XNN_COMMON_ERROR_MAX_ERROR     = XNN_MAX_ERROR_COMMON,
} XNN_Error_t;

typedef void* XNN_Handle_t;

typedef XNN_Handle_t XNN_PredictorHandle_t;

typedef XNN_Handle_t XNN_RuntimeProgramHandle_t;

typedef XNN_Handle_t XNN_TensorHandle_t;

typedef XNN_Handle_t XNN_GraphHandle_t;

typedef XNN_Handle_t XNN_ModelHandle_t;

typedef XNN_Handle_t XNN_BufferHandle_t;

XNN_API
XNN_Error_t XNN_getVersion(char* xnnVersion);

#ifdef __cplusplus
}
#endif

#endif
