/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */
#ifndef __DDR_DEVFREQ_H__
#define __DDR_DEVFREQ_H__

#define DVFS_OK_BUF_LEN		120

int ddr_get_cur_bandwidth(unsigned long *bandwidth_mbytes_ps);
int ddr_get_max_bandwidth(unsigned long *bandwidth_mbytes_ps);

/**
 * @brief Get ISP or DPU dvfs_ok timeout information
 * @param buf Buffer for storing the string of timeout information
 * @param buf_size Size of the buffer, should be at least DVFS_OK_BUF_MAX_LEN
 * @return Returns the number of characters on success, or a negative number on failure
 */
int get_dvfs_ok_tout_info(char *buf, unsigned int buf_size);

#endif /* __DDR_DEVFREQ_H__ */
