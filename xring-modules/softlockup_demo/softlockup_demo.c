// SPDX-License-Identifier: GPL-2.0-only
/*
 * msleep trigger softlockup demo
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

DEFINE_SPINLOCK(mylock);

static void spinlock_trigger_softlockup(void)
{
	spin_lock(&mylock);
	spin_lock(&mylock);
}


static int __init softlockup_demo_init(void)
{
	spinlock_trigger_softlockup();
	return 0;
}

static void __exit softlockup_demo_exit(void)
{
}

module_init(softlockup_demo_init);
module_exit(softlockup_demo_exit);

MODULE_LICENSE("GPL");
