// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <soc/xring/xr_hwspinlock.h>
#include <linux/hwspinlock.h>
#include "hwspinlock_internal.h"


/* group and bit offset, X is lock id */
#define GROUP_NUM(X)			((X) / 8)
#define BITS_OFFSET(X)			((X) % 8)

/* register address offset, X is lock id */
#define LOCK_REQ_ADDRESS(X)		(GROUP_NUM(X) * 0x0C)
#define LOCK_MID_H_ADDRESS		(0x04)
#define LOCK_MID_L_ADDRESS		(0x08)

/* hwlocks and Acpu master ID */
#define XR_HWLOCKS_BASE_ID		0
#define SHIFT_32			32
#define MAX_HWLOCK_NUM			112
#define LOCK_PER_GROUP			8

/* req and clr bits. and mid req bits */
#define LOCK_REQ_BIT(X)			(0x01 << (BITS_OFFSET(X) + 8))
#define LOCK_CLR_BIT(X)			(0x01 << (BITS_OFFSET(X) + 16))
#define MID_REQ_MASK(X)			(0xffULL << (BITS_OFFSET(X) * 8))
#define MID_REQ_BIT(mid, X)		((u64)mid << (BITS_OFFSET(X) * 8))

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_HWLOCK:%s:%d " fmt, __func__, __LINE__

struct xr_hwlock_info {
	uint32_t hwlock_num;
	void __iomem *io_base;
};
struct xr_hwlock_info **hwlock_info;
struct xr_hwlock_info *hw_dev;
static uint32_t xr_mid = 3;

uint32_t xr_hwlock_mid[MAX_HWLOCK_NUM];

int xr_get_hwspinlock_mid(uint32_t lockid, uint32_t *mid)
{
	if (lockid >= MAX_HWLOCK_NUM)
		return -1;

	*mid = xr_hwlock_mid[lockid];

	return 0;
}
EXPORT_SYMBOL(xr_get_hwspinlock_mid);

/* try to lock the hardware spinlock */
static int xr_hwspinlock_trylock(struct hwspinlock *lock)
{
	void __iomem *lock_addr = lock->priv;
	u64 lock_owner_h;
	u32 lock_owner_l;
	u64 lock_owner;
	int lock_id = hwlock_to_id(lock);
	bool locked;

	if (lock_id >= MAX_HWLOCK_NUM)
		return 0;

	writel((LOCK_REQ_BIT(lock_id) | xr_mid), lock_addr);

	lock_owner_h = readl(lock_addr + LOCK_MID_H_ADDRESS);
	lock_owner_l = readl(lock_addr + LOCK_MID_L_ADDRESS);
	lock_owner = (lock_owner_h << SHIFT_32) | lock_owner_l;

	/* if equal, we have the lock, otherwise
	 * someone else has it.
	 */
	locked = (lock_owner & MID_REQ_MASK(lock_id)) == MID_REQ_BIT(xr_mid, lock_id);

	xr_hwlock_mid[lock_id] =
		(lock_owner & MID_REQ_MASK(lock_id)) >> (BITS_OFFSET(lock_id) * LOCK_PER_GROUP);

	if (!locked)
		return 0;
	return 1;
}

/* unlock the hardware spinlock */
static void xr_hwspinlock_unlock(struct hwspinlock *lock)
{
	void __iomem *lock_addr = lock->priv;
	int lock_id = hwlock_to_id(lock);

	writel(LOCK_CLR_BIT(lock_id) | xr_mid, lock_addr);
}

/* The specs recommended below number as the retry delay time */
static void xr_hwspinlock_relax(struct hwspinlock *lock)
{
	ndelay(10);
}

static const struct hwspinlock_ops xr_hwspinlock_ops = {
	.trylock = xr_hwspinlock_trylock,
	.unlock = xr_hwspinlock_unlock,
	.relax = xr_hwspinlock_relax,
};

static int xr_hwspinlock_probe(struct platform_device *pdev)
{
	struct hwspinlock_device *bank = NULL;
	struct device_node *node = pdev->dev.of_node;
	size_t array_size;
	int i, j, lockid, ret;
	int ip_num = 0;
	uint32_t total_hwlock_num = 0;

	if (!node) {
		pr_err("device_node is NULL\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "ip_num", &ip_num);
	if (ret) {
		pr_err("failed to get of_iomap, ret = %d\n", ret);
		return ret;
	}
	hwlock_info = kcalloc(ip_num, sizeof(*hwlock_info), GFP_KERNEL);
	if (!hwlock_info)
		return -ENOMEM;

	for (i = 0; i < ip_num; i++) {
		hw_dev = kcalloc(1, sizeof(*hw_dev), GFP_KERNEL);
		if (!hw_dev) {
			ret = -ENOMEM;
			goto to_free_hwlock;
		}
		hwlock_info[i] = hw_dev;
		hwlock_info[i]->io_base = of_iomap(node, i);
		if (!hwlock_info[i]->io_base) {
			pr_err("of_iomap error, failed to get io_base\n");
			ret = -1;
			goto to_free_dev;
		}
		ret = of_property_read_u32_index(node, "hwlock_num",
			i, &hwlock_info[i]->hwlock_num);
		if (ret) {
			pr_err("failed to get hwlock_num, ret = %d\n", ret);
			goto to_iounmap;
		}
		total_hwlock_num += hwlock_info[i]->hwlock_num;
	}
	array_size = total_hwlock_num * sizeof(struct hwspinlock);
	bank = devm_kzalloc(&pdev->dev, sizeof(*bank) + array_size, GFP_KERNEL);
	if (!bank) {
		pr_err("bank devm_kzalloc err\n");
		ret = -ENOMEM;
		goto to_iounmap;
	}
	for (i = 0, lockid = 0; i < ip_num; i++) {
		for (j = 0; j < hwlock_info[i]->hwlock_num; j++) {
			bank->lock[lockid].priv = hwlock_info[i]->io_base + LOCK_REQ_ADDRESS(j);
			lockid++;
		}
	}
	platform_set_drvdata(pdev, bank);

	ret = of_property_read_u32(node, "masterid", &xr_mid);
	if (ret)
		pr_warn("get mid from dts fail, use default(%d)\n", xr_mid);

	pr_info("hwspinlock probe succ, mid = %d\n", xr_mid);
	return devm_hwspin_lock_register(&pdev->dev, bank, &xr_hwspinlock_ops,
			XR_HWLOCKS_BASE_ID, total_hwlock_num);

to_iounmap:
	j = 0;
	while (hwlock_info[j]) {
		if (hwlock_info[j]->io_base) {
			iounmap(hwlock_info[j]->io_base);
			hwlock_info[j]->io_base = NULL;
		}
		j++;
	}
to_free_dev:
	while (i--)
		kfree(hwlock_info[i]);
to_free_hwlock:
	kfree(hwlock_info);
	return ret;
}

static int xr_hwspinlock_remove(struct platform_device *pdev)
{
	struct hwspinlock_device *bank = platform_get_drvdata(pdev);
	int ret;
	int i = 0;

	while (hwlock_info[i]) {
		if (hwlock_info[i]->io_base) {
			iounmap(hwlock_info[i]->io_base);
			hwlock_info[i]->io_base = NULL;
		}
		i++;
	}
	kfree(hw_dev);
	kfree(hwlock_info);
	ret = hwspin_lock_unregister(bank);
	if (ret)
		dev_err(&pdev->dev, "%s failed: %d\n", __func__, ret);

	return 0;
}

static const struct of_device_id xr_hwspinlock_of_match[] = {
	{ .compatible = "xring,hwspinlock", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_hwspinlock_of_match);

static struct platform_driver xr_hwspinlock_driver = {
	.probe = xr_hwspinlock_probe,
	.remove = xr_hwspinlock_remove,
	.driver = {
		.name = "xr_hwspinlock",
		.of_match_table = xr_hwspinlock_of_match,
	},
};
module_platform_driver(xr_hwspinlock_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware spinlock driver for Xring");
