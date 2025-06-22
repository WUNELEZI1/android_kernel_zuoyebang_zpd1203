// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef XNN_CONFIG_H
#define XNN_CONFIG_H

#include "xnn/c_api/XnnCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XNN_MODEL_PATH_MAX_LEN 256
#define NETWORK_IO_MAX_DIM 6
#define NETWORK_IO_MAX_NAME_LENGTH 64

typedef struct XNN_MobileConfig_t {
    XnnPowerMode         mPowerMode{XNN_POWER_BALANCE};
    XnnTargetType        mTarget{XNN_NPU};
    XnnPriority          mPriority{XNN_PRIORITY_HIGH};
    XnnPrecisionType     mPrecision{kUInt8};
    char                 mModelPath[XNN_MODEL_PATH_MAX_LEN];
    char*                mModelBuf{nullptr};
    uint32_t             mModelSize{0};
    XnnLogLevel          mLogLevel{XNN_CLOSE};
    XnnProfileLevel      mProfileLevel{XNN_PROFILE_LEVEL_DISABLE};
    bool                 mDDrVoteClose{false};
    XnnNetworkCreateType mNetCreateType{XNN_CREATE_NETWORK_FROM_MEMORY};
    XnnNetworkShareMode  mNetShareMode{XNN_SHARED_NONE};
    char*                mModelDmaBufName{nullptr};
} XNN_MobileConfig;

typedef struct XNN_TaskPara_t {
    XnnPowerMode     mPowerMode{XNN_POWER_BALANCE};
    XnnTargetType    mTarget{XNN_NPU};
    XnnPriority      mPriority{XNN_PRIORITY_HIGH};
    XnnProfileLevel  mProfileLevel{XNN_PROFILE_LEVEL_DISABLE};
} XNN_TaskPara;

typedef struct XnnIOInfo_t {
    uint32_t          dim_count;
    uint32_t          dim_size[NETWORK_IO_MAX_DIM];
    XnnPrecisionType  data_format;
    XnnQuantFormat    quan_format;
    int32_t           fixed_pos;
    float             scale;
    int32_t           zero_point;
    char              name[NETWORK_IO_MAX_NAME_LENGTH];
    int32_t           size;
} XnnIOInfo;

#ifdef __cplusplus
}
#endif

#endif
