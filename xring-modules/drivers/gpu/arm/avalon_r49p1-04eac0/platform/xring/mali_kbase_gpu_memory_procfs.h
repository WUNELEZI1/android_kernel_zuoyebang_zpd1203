// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _KBASE_GPU_MEMORY_PROCFS_H
#define _KBASE_GPU_MEMORY_PROCFS_H

struct proc_dir_entry *kbasep_gpu_memory_procfs_init(struct kbase_device *kbdev);
void kbasep_gpu_memory_procfs_exit(void);

#endif
