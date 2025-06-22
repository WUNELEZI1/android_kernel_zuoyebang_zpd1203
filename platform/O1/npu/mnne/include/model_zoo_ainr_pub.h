// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef MODEL_ZOO_AINR_PUB_H
#define MODEL_ZOO_AINR_PUB_H

#include "model_zoo_types.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32 task_id;
} model_zoo_get_taskid_t;


typedef struct {
    int16 frame_buf_type;
    uint64 i2n_mem_handle;
    uint32 i2n_mem_size;
    uint64 n2i_mem_handle;
    uint32 n2i_mem_size;



#ifdef AINR_ISP_EMU_TEST


    uint64 in_ref_mem_handle;
    uint32 in_ref_size;
    uint64 out_ref_mem_handle;
    uint32 out_ref_size;
    uint8* i2n_cpu_va;
    uint8* in_ref_cpu_va;
    uint8* out_ref_cpu_va;
#endif
} model_zoo_get_iobuf_t;

model_zoo_status_e model_zoo_get_taskid(void* handle, model_zoo_get_taskid_t* outParams);

model_zoo_status_e model_zoo_get_iobuf(void* handle, model_zoo_get_iobuf_t* outParams);

#ifdef __cplusplus
}
#endif

#endif
