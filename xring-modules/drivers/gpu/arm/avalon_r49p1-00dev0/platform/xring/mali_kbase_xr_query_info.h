// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _KBASE_QUERY_INFO_H_
#define _KBASE_QUERY_INFO_H_

#define XR_NUM_PERF_COUNTERS 5
/**
 * kbase_xr_query_fence_count - Query number of times the fence has been triggered
 * since the first query.
 *
 * @data:            The dev pointer.
 * @query_pid:       The process id to query.
 * @fence_count:     The number of times the fence has been triggered.
 */
int kbase_xr_query_fence_count(void *data, pid_t query_pid, u64 *fence_count);

/**
 * kbase_xr_query_hwcounter - Query hardware counters.
 *
 * @data:         The dev pointer.
 * @values:       The address to write the counter values.
 * @num_values:   The num of the counters.
 */
int kbase_xr_query_hwcounter(void *data, u64 *values, size_t num_values);

/**
 * kbase_xr_ai_control_register - Register a ai control client.
 *
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 */
int kbase_xr_ai_control_register(struct kbase_device *kbdev);

#endif