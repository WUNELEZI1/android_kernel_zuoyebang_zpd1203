#include <linux/module.h>
#include "lc_notify.h"

static BLOCKING_NOTIFIER_HEAD(charger_notifier);

int lc_charger_notifier_register(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&charger_notifier, nb);
}
EXPORT_SYMBOL_GPL(lc_charger_notifier_register);

int lc_charger_notifier_unregister(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&charger_notifier, nb);
}
EXPORT_SYMBOL_GPL(lc_charger_notifier_unregister);

int lc_charger_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&charger_notifier, val, v);
}
