// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef MODEL_ZOO_PUB_H
#define MODEL_ZOO_PUB_H

#include "model_zoo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

model_zoo_status_e model_zoo_preload(void** handle,
        model_zoo_algo_type_t type, void* preload_params);

model_zoo_status_e model_zoo_init(void** handle,
        model_zoo_algo_type_t type, void* env_params);

model_zoo_status_e model_zoo_sync_run(void* handle,
        void* in_buf, void* out_buf);

model_zoo_status_e model_zoo_para_udpate(
        void* handle, void* env_params);

model_zoo_status_e model_zoo_deinit(void* handle);

model_zoo_status_e model_zoo_async_init(
        void** handle, model_zoo_algo_type_t type,
        void* env_params, model_zoo_async_callback callback);

model_zoo_status_e model_zoo_async_run(void* handle,
        void* in_buf, void* out_buf, model_zoo_async_callback callback);

model_zoo_status_e model_zoo_trigger(void* handle,
        void* in_buf, void* out_buf,model_zoo_async_event* event);
model_zoo_status_e model_zoo_wait(void* handle, model_zoo_async_event event);

#ifdef __cplusplus
}
#endif

#endif
