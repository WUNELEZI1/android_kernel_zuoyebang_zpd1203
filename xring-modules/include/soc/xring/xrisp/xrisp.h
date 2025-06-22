/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_API_H__
#define __XRISP_API_H__
#include <linux/kernel.h>

bool xrisp_rproc_ramlog_avail(void);
void *xrisp_rproc_ramlog_addr(void);
// phys_addr_t xrisp_rproc_ramlog_pa(void);
size_t xrisp_rproc_ramlog_size(void);
int xrisp_register_ramlog_ops(void *priv, int (*ramlog_start)(void *priv),
			      void (*ramlog_stop)(void *priv, bool crashed));
int xrisp_unregister_ramlog_ops(void);

bool xrisp_rproc_regdev_avail(void);
void *xrisp_rproc_regdev_mem_addr(void);
size_t xrisp_rproc_regdev_mem_size(void);
int xrisp_register_regdev_ops(void *priv, int (*regdev_start)(void *priv),
			      void (*regdev_stop)(void *priv, bool crashed));
int xrisp_unregister_regdev_ops(void);

#endif /* __XRISP_API_H__ */
