/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_BOB_REGULATOR_H__
#define __XR_BOB_REGULATOR_H__

#include <linux/regulator/consumer.h>

#if IS_ENABLED(CONFIG_XRING_BOB_REGULATOR)
int xring_regulator_bob_init(void);
void xring_regulator_bob_exit(void);
#else
static int xring_regulator_bob_init(void)
{
	return 0;
}
static void xring_regulator_bob_exit(void) {}
#endif

#endif /* __XR_BOB_REGULATOR_H__ */
