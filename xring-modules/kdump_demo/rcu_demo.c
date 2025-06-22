// SPDX-License-Identifier: GPL-2.0-only
/*
 * spinlock trigger rcu demo
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

DEFINE_SPINLOCK(mylock);

static void spinlock_trigger_rcu(void)
{
	unsigned long flags;

	pr_info("hello softlockup\n");
	spin_lock_irqsave(&mylock, flags);
	spin_lock_irqsave(&mylock, flags);
}


static int __init rcu_demo_init(void)
{
	spinlock_trigger_rcu();
	return 0;
}

static void __exit rcu_demo_exit(void)
{
}

module_init(rcu_demo_init);
module_exit(rcu_demo_exit);

MODULE_LICENSE("GPL");
