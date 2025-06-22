/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __INCLUDE_MPAM_HW_OPERATOR_H_
#define __INCLUDE_MPAM_HW_OPERATOR_H_

#include "mpam_base.h"

int mpam_set_partid_sel(struct mpam_msc *msc, const u16 partid);
int mpam_get_partid_sel(struct mpam_msc *msc, u16 *partid);
int mpam_set_cpor_part_config(struct mpam_msc *msc, const u32 val);
int mpam_get_cpor_part_config(struct mpam_msc *msc, u32 *reg);
int mpam_set_mbw_part_config(struct mpam_msc *msc, const u32 val);
int mpam_get_mbw_part_config(struct mpam_msc *msc, u32 *reg);

int mpam_set_msc_lock(struct mpam_msc *msc);
int mpam_get_msc_lock(struct mpam_msc *msc);
int mpam_msc_hw_probe(struct mpam_msc *msc);

/* MPAM MSC interrupt(s) control */
int mpam_msc_register_irqs(struct mpam_msc *msc);
void mpam_msc_unregister_irq(struct mpam_msc *msc);

#endif /* __INCLUDE_MPAM_HW_OPERATOR_H_ */
