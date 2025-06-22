// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#pragma once

#ifndef _UNIFIED_COMMON_H_
#define _UNIFIED_COMMON_H_

#include <string>
#include <functional>
#include <vector>
#include "xnn/graph/places.h"

typedef unsigned char       mnne_uint8_t;
typedef unsigned short      mnne_uint16_t;
typedef unsigned int        mnne_uint32_t;
typedef unsigned long long  mnne_uint64_t;
typedef signed char         mnne_int8_t;
typedef signed short        mnne_int16_t;
typedef signed int          mnne_int32_t;
typedef signed long long    mnne_int64_t;
typedef char                mnne_char_t;
typedef float               mnne_float_t;

namespace mnne {
namespace lite {

enum status_t
{
    XNN_RUNTIME_SUCCESS                       = 0x0,
    XNN_ERROR_OPEN_DEV_FAIL                   = 0x1,
    XNN_ERROR_PARA_SET_ILLEGAL                = 0x2,
    XNN_ERROR_CREATE_NETWORK_FAILED           = 0x3,
    XNN_ERROR_RUN_TASK_FAILED                 = 0x4,
    XNN_ERROR_ASYNC_RUN_TASK_FAILED           = 0x5,
    XNN_ERROR_GET_NETWORK_IO_INFO_ERROR       = 0x6,
    XNN_ERROR_BUFFER_MAP_FAIL                 = 0x7,
    XNN_ERROR_CREATE_BUFFER_FAIL              = 0x8,
    XNN_ERROR_SET_BUFFER_FAIL                 = 0x9,
    XNN_ERROR_BUFFER_UNMAP_FAIL               = 0xA,
    XNN_ERROR_BUFFER_CACHE_SYNC_FAIL          = 0xB,
    XNN_ERROR_GET_PROFILING_DATA_FAIL         = 0xC,
    XNN_ERROR_GET_LAYER_DUMP_DATA_FAIL        = 0xD,
    XNN_ERROR_PARA_SET_FAIL                   = 0xE,
    XNN_ERROR_STREAM_RECORD_FAIL              = 0xF,
    XNN_ERROR_STREAM_SYNCHRONIZE_FAIL         = 0x10,
    XNN_ERROR_INIT_FAIL                       = 0x11,
    XNN_ERROR_FREE_NETWORK_FAILED             = 0x12,
    XNN_ERROR_GET_TASK_FAILED                 = 0x13,
    XNN_ERROR_CLOSE_DEV_FAIL                  = 0x14,
    XNN_ERROR_SET_BUFFER_OFFSET_FAIL          = 0x15,
    XNN_ERROR_GET_EVENT_FAIL                  = 0x16,
    XNN_ERROR_EVENT_SYNC_FAIL                 = 0x17,
    XNN_ERROR_STREAM_SUBMIT_TASK_FAIL         = 0x18,
    XNN_ERROR_TASK_CANCEL_FAIL                = 0x19,
};

enum XnnCreateNetworkType
{
    XNN_CREATE_NETWORK_FROM_NONE             = 0x00,
    XNN_CREATE_NETWORK_FROM_FILE             = 0x01,
    XNN_CREATE_NETWORK_FROM_MEMORY           = 0x02, 
    XNN_CREATE_NETWORK_MODE_SHARE_COMMAND    = 0x100,
    XNN_CREATE_NETWORK_MODE_SHARE_WEIGHTS    = 0x400,
    XNN_CREATE_NETWORK_MAX,
};

enum XnnShareMode {
    XNN_SHARED_NONE        = 0x00,
    XNN_SHARED_WEAK        = 0x01,
    XNN_SHARED_WEIGHT      = 0x02,
    XNN_SHARED_CMD         = 0x03,
    XNN_SHARED_ALL         = 0x04,
    XNN_SHARED_NUM         = 0x05
};

enum TaskWorkMode {
    TASK_WORK_MODE_DEFAULT  = 0,
    TASK_WORK_MODE_MAX
};

enum TaskPriority {
    TASK_PRIORITY_PREEMPT   = 0,
    TASK_PRIORITY_HIGH      = 1,
    TASK_PRIORITY_MID       = 2,
    TASK_PRIORITY_LOW       = 3,
};

enum XnnMemMapType {
    XNN_CPU_MAP   = 0x1,
    XNN_MCU_MAP   = 0x2,
    XNN_NPU_MAP   = 0x4,
    XNN_GPU_MAP   = 0x8,
};

enum XnnMemType
{
    XNN_MEM_TYPE_DDR_DMA_BUF    = 0,
    XNN_MEM_TYPE_OCM            = 1,
};

enum BufferType
{
    BUFFER_TYPE_INPUT  = 0x1,
    BUFFER_TYPE_OUTPUT = 0x2
};

enum XnnProfilingLevel {
    XNN_PROFILING_DISABLE = 0,
    XNN_PROFILING_NORMAL  = 1,
    XNN_PROFILING_DETAIL  = 2
};

struct ProgramTaskPara
{
    PowerMode mPowerMode;
    TargetType target;
    XnnProfilingLevel profileLevel;
    bool mDdrVoteClose;
};

struct NPUTaskInfo {
    mnne_uint32_t mUsingCoreCount;
    mnne_uint32_t mNetworkCount;
};

#define NETWORK_IO_MAX_DIM 6
#define NETWORK_IO_MAX_NAME_LENGTH 64

struct IOInfo
{
    mnne_uint32_t dim_count;
    mnne_uint32_t dim_size[NETWORK_IO_MAX_DIM];
    PrecisionType data_format;
    mnne_uint32_t quan_format;
    mnne_int32_t  fixed_pos;
    mnne_float_t  tf_scale;
    mnne_int32_t  tf_zero_point;
    mnne_char_t   name[NETWORK_IO_MAX_NAME_LENGTH];
    mnne_uint32_t  size;
};

struct PredictorProfileData
{
    float programInitTime {0};
    float programRunTime {0};
    float setInputTime {0};
    float setOutputTime {0};
    float creatTaskTime {0};
    float flushInputCacheTime {0};
    float flushOutputCacheTime {0};
    float quantizeTime {0};
    float deQuantizeTime {0};
};

struct ProgramProfileData
{
    float npuRTInitTime {0};
    float programInitTime {0};
    std::vector<float> npuRTRunTime;
    std::vector<float> programRunTime;
    std::vector<float> setInputTime;
    std::vector<float> setOutputTime;
    std::vector<float> creatTaskTime;
    std::vector<float> flushInputCacheTime;
    std::vector<float> flushOutputCacheTime;
    std::vector<float> quantizeTime;
    std::vector<float> deQuantizeTime;
};

struct XnnProfileConfig
{
    bool layerDump {false};
    XnnProfilingLevel profileLevel {XNN_PROFILING_DISABLE};
    std::string outputPath;
};

typedef enum {
    XNN_LOG_INFO = 0,
    XNN_LOG_WARNING,
    XNN_LOG_DEBUG,
    XNN_LOG_ERROR,
    XNN_LOG_CLOSE
} XNNLogLevel;

class NPUBuffer;
typedef struct XnnNetworksIOConfig
{
    std::vector<NPUBuffer*> mInputs;
    std::vector<NPUBuffer*> mOutputs;
} XnnNetworksIOConfig_t;

typedef struct XnnTaskIOConfig
{
    std::vector<XnnNetworksIOConfig_t> mNetworks;
} XnnTaskIOConfig_t;

typedef std::function<void(status_t)> XnnAsyncRunCallback;

}
}
#endif
