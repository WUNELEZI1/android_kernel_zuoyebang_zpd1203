/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __DDR_CURRENT_FREQ_INTERNAL_H__
#define __DDR_CURRENT_FREQ_INTERNAL_H__

unsigned long ddr_get_current_freq(void);
int ddr_current_freq_remap(struct device *dev);

#endif /* __DDR_CURRENT_FREQ_H__ */
