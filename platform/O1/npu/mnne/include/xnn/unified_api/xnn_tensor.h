// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#pragma once
#include <vector>
#include "xnn/unified_api/unified_common.h"
#include <iostream>

namespace mnne {
namespace lite {

class XNNTensor {
public:
    virtual ~XNNTensor(){};

    virtual void* mutable_data(PrecisionType type) = 0;
    virtual void* mutable_data(size_t memory_size) = 0;

    virtual void* data() const = 0;

    virtual PrecisionType precision() const = 0;

    virtual status_t shareExternalMemory(TargetType target, size_t size, mnne_uint8_t *addr,
                                         mnne_uint64_t mem_handle) = 0;
    virtual void setShape(const std::vector<int64_t> &shape) = 0;

    virtual std::vector<int64_t> shape() const = 0;

    virtual void* getAddr() const = 0;

    virtual void set_IOtensor_precision(PrecisionType precision) = 0;

    virtual status_t shareExternalMemoryWithOffset(TargetType target, size_t size, mnne_uint8_t *addr,
                                                   mnne_uint64_t mem_handle, mnne_uint32_t offset, mnne_uint32_t offsetSize) = 0;

    virtual status_t cacheStart() = 0;
    virtual status_t cacheEnd() = 0;
    virtual status_t unmapShareMemory(int32_t mem_handle) = 0;
};

}
}

mnne::lite::XNNTensor* createXNNTensor(mnne::lite::TargetType target);
void releaseXNNTensor(mnne::lite::XNNTensor* tensor);