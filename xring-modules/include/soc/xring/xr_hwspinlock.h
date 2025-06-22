/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __XR_HWSPINLOCK_H__
#define __XR_HWSPINLOCK_H__

#if IS_ENABLED(CONFIG_XRING_HWLOCK)
int xr_get_hwspinlock_mid(uint32_t lockid, uint32_t *mid);
#else
static int xr_get_hwspinlock_mid(uint32_t lockid, uint32_t *mid)
{
	return 0;
}
#endif

#endif /* __XR_HWSPINLOCK_H__ */
