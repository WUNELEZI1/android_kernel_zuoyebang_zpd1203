/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */


#ifndef _CPUFREQ_H
#define _CPUFREQ_H
int xr_cpufreq_qos_init(struct device_node *xr_node);
void xr_cpufreq_qos_exit(void);

#endif
