// SPDX-License-Identifier: GPL-2.0-only
/*
 * msleep trigger hungtask demo
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>

DEFINE_MUTEX(my_lock);

static int __init hungtask_init(void)
{
	mutex_lock(&my_lock);
	mutex_lock(&my_lock);

	return 0;
}

static void __exit hungtask_exit(void)
{
}

module_init(hungtask_init);
module_exit(hungtask_exit);

MODULE_LICENSE("GPL");
