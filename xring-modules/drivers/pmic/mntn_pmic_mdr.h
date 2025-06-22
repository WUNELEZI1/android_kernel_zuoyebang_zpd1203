/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MPMIC MDR(Maintenance Data Record)
 *
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#ifndef __MNTN_PMIC_MDR_H__
#define __MNTN_PMIC_MDR_H__

#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

extern int ocp_reset_system;

/*
 * return 0 on fail
 * return the MDR modid on success
 */
u32 mpmic_mdr_get_e_modid_by_ocp_irq_name(const char *irq_name);

int mpmic_mdr_exception_register(void);
void mpmic_mdr_exception_unregister(void);

#endif /* __MNTN_PMIC_MDR_H__ */
