// SPDX-License-Identifier: GPL-2.0-only
/*
 * XRing PMIC key driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#include <linux/export.h>
#include <linux/notifier.h>
#include <soc/xring/xr_pmic_key_event.h>

static ATOMIC_NOTIFIER_HEAD(xr_pmic_key_notifier_list);

int xr_pmic_key_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&xr_pmic_key_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(xr_pmic_key_register_notifier);

int xr_pmic_key_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(
		&xr_pmic_key_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(xr_pmic_key_unregister_notifier);

int xr_call_pmic_key_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&xr_pmic_key_notifier_list,
		val, v);
}
EXPORT_SYMBOL_GPL(xr_call_pmic_key_notifiers);
