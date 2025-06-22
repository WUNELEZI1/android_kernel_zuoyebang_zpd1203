/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#ifndef __DOZE_H__
#define __DOZE_H__

#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>

/* vote interface */
#if IS_ENABLED(CONFIG_XRING_DOZE)
int sys_state_doz2nor_unvote(unsigned int voter);
int sys_state_doz2nor_vote(unsigned int voter);
#else
static inline int sys_state_doz2nor_unvote(unsigned int voter) { return 0; }
static inline int sys_state_doz2nor_vote(unsigned int voter) { return 0; }
#endif
#endif /* __DOZE_H__ */
