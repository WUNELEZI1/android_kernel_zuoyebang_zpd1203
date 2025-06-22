#ifndef _XR_PMIC_KEY_EVENT_H
#define _XR_PMIC_KEY_EVENT_H
#include  <linux/notifier.h>


typedef  enum {
	XR_POWERKEY_PRESS = 0,
	XR_POWERKEY_RELEASE,
	XR_POWERKEY_PRESS_HOLD_1S,
	XR_POWERKEY_PRESS_HOLD_6S,
	XR_VOLUMEDOWN_PRESS,
	XR_VOLUMEDOWN_RELEASE,
	XR_VOLUMEDOWN_PRESS_HOLD_1S,
	XR_VOLUMEUP_PRESS,
	XR_VOLUMEUP_RELEASE,
	XR_VOLUMEUP_PRESS_HOLD_1S,
}XR_POWERKEY_EVENT_T;

int xr_pmic_key_register_notifier(struct notifier_block *nb);
int xr_pmic_key_unregister_notifier(struct notifier_block *nb);
int xr_call_pmic_key_notifiers(unsigned long val,void *v);

#endif