// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/notifier.h>

BLOCKING_NOTIFIER_HEAD(g_sh_reboot_notifier_list);
BLOCKING_NOTIFIER_HEAD(g_sh_recovery_notifier_list);
ATOMIC_NOTIFIER_HEAD(g_shub_boot_notifier_list);

int sh_reset_notifier_register(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = blocking_notifier_chain_register(&g_sh_reboot_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(sh_reset_notifier_register);

int sh_reset_notifier_unregister(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = blocking_notifier_chain_unregister(&g_sh_reboot_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(sh_reset_notifier_unregister);

int sh_recovery_notifier_register(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = blocking_notifier_chain_register(&g_sh_recovery_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(sh_recovery_notifier_register);

int sh_recovery_notifier_unregister(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = blocking_notifier_chain_unregister(&g_sh_recovery_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(sh_recovery_notifier_unregister);

int shub_boot_atomic_notifier_register(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = atomic_notifier_chain_register(&g_shub_boot_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(shub_boot_atomic_notifier_register);

int shub_boot_atomic_notifier_unregister(struct notifier_block *nb)
{
	int ret;

	if (nb == NULL || nb->notifier_call == NULL)
		ret = -EINVAL;
	else
		ret = atomic_notifier_chain_unregister(&g_shub_boot_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(shub_boot_atomic_notifier_unregister);

int sh_reset_notifier_call_chain(unsigned long value)
{
	int ret = blocking_notifier_call_chain(&g_sh_reboot_notifier_list, value, NULL);
	return ret;
}
EXPORT_SYMBOL(sh_reset_notifier_call_chain);

int sh_recovery_notifier_call_chain(unsigned long value)
{
	int ret = blocking_notifier_call_chain(&g_sh_recovery_notifier_list, value, NULL);
	return ret;
}
EXPORT_SYMBOL(sh_recovery_notifier_call_chain);

int shub_boot_atomic_notifier_call_chain(unsigned long value)
{
	int ret = atomic_notifier_call_chain(&g_shub_boot_notifier_list, value, NULL);
	return ret;
}
EXPORT_SYMBOL(shub_boot_atomic_notifier_call_chain);
