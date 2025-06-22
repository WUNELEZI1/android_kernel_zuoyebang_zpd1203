// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef XNN_XNNTENSOR_H
#define XNN_XNNTENSOR_H

#include "xnn/c_api/XnnCommon.h"
#include "xnn/c_api/XnnConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    XNN_TENSOR_ERROR_MIN_ERROR             = XNN_MIN_ERROR_TENSOR,
    XNN_TENSOR_ERROR_INVALID_HANDLE        = XNN_MIN_ERROR_TENSOR + 0,
    XNN_TENSOR_ERROR_INVALID_CONFIG        = XNN_MIN_ERROR_TENSOR + 1,
    XNN_TENSOR_ERROR_CREATE_FAIL           = XNN_MIN_ERROR_TENSOR + 2,
    XNN_TENSOR_ERROR_EXTERNAL_MEMORY_NULL  = XNN_MIN_ERROR_TENSOR + 3,
    XNN_TENSOR_ERROR_BUFFER_OWN_DATA_ERROR = XNN_MIN_ERROR_TENSOR + 4,
    XNN_TENSOR_ERROR_SHAPE_SIZE            = XNN_MIN_ERROR_TENSOR + 5,
    XNN_TENSOR_ERROR_CACHE_SYNC            = XNN_MIN_ERROR_TENSOR + 6,
    XNN_TENSOR_NO_ERROR                    = XNN_SUCCESS,

    XNN_TENSOR_MAX_ERROR                   = XNN_MAX_ERROR_TENSOR,
}XNNTensor_Error_t;

XNN_API
XNNTensor_Error_t XNNTensor_create(XNN_TensorHandle_t* tensorHandle,
                                   XnnTargetType target);

XNN_API
XNNTensor_Error_t XNNTensor_setShape(XNN_TensorHandle_t tensorHandle, int64_t* shape, int shapeNum);

XNN_API
XNN_BufferHandle_t XNNTensor_mutableData(XNN_TensorHandle_t tensorHandle, XnnPrecisionType precisionType);

XNN_API
XNN_BufferHandle_t XNNTensor_mutableDataWithSize(XNN_TensorHandle_t tensorHandle, size_t size);

XNN_API
void* XNNTensor_getAddr(XNN_TensorHandle_t tensorHandle);

XNN_API
XnnPrecisionType XNNTensor_getPrecision(XNN_TensorHandle_t tensorHandle);

XNN_API
XNNTensor_Error_t XNNTensor_getShape(XNN_TensorHandle_t tensorHandle, int64_t* shape, int* shapeSize);

XNN_API
XNNTensor_Error_t XNNTensor_shareExternalMemory(XNN_TensorHandle_t tensorHandle, XnnTargetType target, uint32_t size, uint8_t *addr,
                                                int32_t mem_handle);

XNN_API
XNNTensor_Error_t XNNTensor_shareExternalMemoryWithOffset(XNN_TensorHandle_t tensorHandle, XnnTargetType target, uint32_t size, uint8_t *addr,
                                                          int32_t mem_handle, uint32_t offset, uint32_t offsetSize);

XNN_API
XNNTensor_Error_t XNNTensor_cacheStart(XNN_TensorHandle_t tensorHandle);

XNN_API
XNNTensor_Error_t XNNTensor_cacheEnd(XNN_TensorHandle_t tensorHandle);

XNN_API
XNNTensor_Error_t XNNTensor_free(XNN_TensorHandle_t tensorHandle);

XNN_API
XNNTensor_Error_t XNNTensor_unmapShareMemory(XNN_TensorHandle_t tensorHandle, int32_t mem_handle);

#ifdef __cplusplus
}
#endif

#endif
