// SPDX-License-Identifier: GPL-2.0-only
/*
 * XRing PMIC key driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/pm_wakeirq.h>
#include <linux/ktime.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/regmap.h>
#include <soc/xring/xr_pmic_key_event.h>
#include <soc/xring/trace_hook_set.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>

#define LONG_PRESS_TIME_MS 5000
#define PMIC_KEY_RELEASE 0
#define PMIC_KEY_PRESS 1
#define KEY_PRESS_IRQ_INDEX 1

struct pmic_key_irq_map {
	char *event_name;
	int irq_suspend_cfg;
	irqreturn_t (*irq_handler)(int irq, void *data);
};

struct pmic_key_data {
	const char	*name;
	const char	*phys;
	struct pmic_key_irq_map *key_irq_cfg;
};

static int power_key_state;
static int volume_down_state;
static struct timer_list power_volume_timer;

struct pmic_key_info {
	struct device *dev;
	struct regmap *regmap;
	struct input_dev *input;
	u32 code;
	const struct pmic_key_data *data;
	int irq;
	int irq_count;
};

static irqreturn_t powerkey_press_handle(int irq, void *data);
static irqreturn_t powerkey_release_handle(int irq, void *data);
static irqreturn_t powerkey_press_hold_1s_handle(int irq, void *data);
static irqreturn_t powerkey_press_hold_6s_handle(int irq, void *data);
static irqreturn_t volumedown_press_handle(int irq, void *data);
static irqreturn_t volumedown_release_handle(int irq, void *data);
static irqreturn_t volumedown_press_hold_1s_handle(int irq, void *data);
static irqreturn_t volumeup_press_handle(int irq, void *data);
static irqreturn_t volumeup_release_handle(int irq, void *data);
static irqreturn_t volumeup_press_hold_1s_handle(int irq, void *data);

static struct pmic_key_irq_map g_event_powerkey_cfg[] = {
	{ .event_name = "powerkey_release", .irq_suspend_cfg = IRQF_NO_SUSPEND | IRQF_ONESHOT,
		.irq_handler = powerkey_release_handle },

	{ .event_name = "powerkey_press", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = powerkey_press_handle },

	{ .event_name = "powerkey_press_hold_1s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_press_hold_1s_handle },

	{ .event_name = "powerkey_press_hold_6s", .irq_suspend_cfg = 0,
		.irq_handler = powerkey_press_hold_6s_handle },
};

static struct pmic_key_irq_map g_event_volumedown_cfg[] = {
	{ .event_name = "volumedown_release", .irq_suspend_cfg = IRQF_NO_SUSPEND | IRQF_ONESHOT,
		.irq_handler = volumedown_release_handle },

	{ .event_name = "volumedown_press", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = volumedown_press_handle },

	{ .event_name = "volumedown_press_hold_1s", .irq_suspend_cfg = 0,
		.irq_handler = volumedown_press_hold_1s_handle },
};

static struct pmic_key_irq_map g_event_volumeup_cfg[] = {
	{ .event_name = "volumeup_release", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = volumeup_release_handle },

	{ .event_name = "volumeup_press", .irq_suspend_cfg = IRQF_NO_SUSPEND,
		.irq_handler = volumeup_press_handle },

	{ .event_name = "volumeup_press_hold_1s", .irq_suspend_cfg = 0,
		.irq_handler = volumeup_press_hold_1s_handle },
};

static void power_volume_timeout(struct timer_list *arg)
{
	if (power_key_state && volume_down_state) {
		pr_info("[%s] trigger warm restart!\n", __func__);
	}

	power_key_state = PMIC_KEY_RELEASE;
	volume_down_state = PMIC_KEY_RELEASE;
}

static irqreturn_t powerkey_press_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	power_key_state = PMIC_KEY_PRESS;
	if (volume_down_state)
		mod_timer(&power_volume_timer, jiffies + msecs_to_jiffies(LONG_PRESS_TIME_MS));

	set_sr_keypoint(SR_KEYPOINT_POWERKEY_PRESS);
	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s] power key press interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_POWERKEY_PRESS, data);

	input_report_key(info->input, KEY_POWER, PMIC_KEY_PRESS);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_release_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	power_key_state = PMIC_KEY_RELEASE;
	if (volume_down_state)
		del_timer_sync(&power_volume_timer);

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));
	pr_debug("[%s] power key release interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_POWERKEY_RELEASE, data);

	input_report_key(info->input, KEY_POWER, PMIC_KEY_RELEASE);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_press_hold_1s_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]response long press 1s interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_POWERKEY_PRESS_HOLD_1S, data);

	return IRQ_HANDLED;
}

static irqreturn_t powerkey_press_hold_6s_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]response long press 6s interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_POWERKEY_PRESS_HOLD_6S, data);

	return IRQ_HANDLED;
}

static irqreturn_t volumedown_press_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	volume_down_state = PMIC_KEY_PRESS;
	if (power_key_state)
		mod_timer(&power_volume_timer, jiffies + msecs_to_jiffies(LONG_PRESS_TIME_MS));

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));
	pr_debug("[%s]volumedown key press interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEDOWN_PRESS, data);

	input_report_key(info->input, KEY_VOLUMEDOWN, PMIC_KEY_PRESS);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t volumedown_release_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	volume_down_state = PMIC_KEY_RELEASE;
	if (power_key_state)
		del_timer_sync(&power_volume_timer);

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));
	pr_debug("[%s]volumedown key release interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEDOWN_RELEASE, data);

	input_report_key(info->input, KEY_VOLUMEDOWN, PMIC_KEY_RELEASE);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t volumedown_press_hold_1s_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]response volumedown long press 1s interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEDOWN_PRESS_HOLD_1S, data);

	return IRQ_HANDLED;
}

static irqreturn_t volumeup_press_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]volumeup key press interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEUP_PRESS, data);

	input_report_key(info->input, KEY_VOLUMEUP, PMIC_KEY_PRESS);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t volumeup_release_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]volumeup key release interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEUP_RELEASE, data);

	input_report_key(info->input, KEY_VOLUMEUP, PMIC_KEY_RELEASE);
	input_sync(info->input);

	return IRQ_HANDLED;
}

static irqreturn_t volumeup_press_hold_1s_handle(int irq, void *data)
{
	struct pmic_key_info *info = NULL;

	if (!data)
		return IRQ_NONE;

	info = (struct pmic_key_info *)data;

	pm_wakeup_event(info->dev, jiffies_to_msecs(HZ));

	pr_debug("[%s]response volumeup long press 1s interrupt!\n", __func__);
	xr_call_pmic_key_notifiers(XR_VOLUMEUP_PRESS_HOLD_1S, data);

	return IRQ_HANDLED;
}

static int pmic_key_probe(struct platform_device *pdev)
{
	struct pmic_key_info *info = NULL;
	struct device *dev = &pdev->dev;
	int i;
	int ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;

	info->dev = dev;
	info->data = of_device_get_match_data(dev);

	info->regmap = dev_get_regmap(dev->parent, NULL);
	if (!info->regmap) {
		dev_err(dev, "failed to get regmap for parent\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(dev->of_node, "linux,code",
				     &info->code);
	if (ret) {
		dev_err(dev, "failed to get key type\n");
		return ret;
	}

	info->input = devm_input_allocate_device(dev);
	if (!info->input) {
		dev_err(dev, "Failed to allocate input device\n");
		return -ENOMEM;
	}

	input_set_capability(info->input, EV_KEY, info->code);

	info->input->name = info->data->name;
	info->input->phys = info->data->phys;

	info->irq_count = platform_irq_count(pdev);
	if (info->irq_count < 0) {
		dev_err(&pdev->dev, "failed to get number of irqs\n");
		return info->irq_count;
	}

	device_init_wakeup(dev, true);

	for (i = 0; i < info->irq_count; i++) {
		info->irq = platform_get_irq_byname(pdev, info->data->key_irq_cfg[i].event_name);
		if (info->irq < 0) {
			dev_err(dev, "failed to get %s irq id\n",
				info->data->key_irq_cfg[i].event_name);
			continue;
		}

		if (i == KEY_PRESS_IRQ_INDEX)
			dev_pm_set_wake_irq(dev, info->irq);

		if (!strcmp(info->data->key_irq_cfg[i].event_name, "powerkey_release") ||
			!strcmp(info->data->key_irq_cfg[i].event_name, "volumedown_release")) {
			ret = devm_request_threaded_irq(dev, info->irq, NULL,
				info->data->key_irq_cfg[i].irq_handler,
				info->data->key_irq_cfg[i].irq_suspend_cfg,
				info->data->key_irq_cfg[i].event_name, info);
			if (ret < 0) {
				dev_err(dev, "failed to request %s irq\n",
					info->data->key_irq_cfg[i].event_name);
				return ret;
			}
		} else {
			ret = devm_request_irq(
				dev, info->irq, info->data->key_irq_cfg[i].irq_handler,
				info->data->key_irq_cfg[i].irq_suspend_cfg,
				info->data->key_irq_cfg[i].event_name, info);
			if (ret) {
				dev_err(dev, "failed to request %s irq\n",
					info->data->key_irq_cfg[i].event_name);
				return ret;
			}
		}
	}

	ret = input_register_device(info->input);
	if (ret) {
		dev_err(dev, "Can't register input device: %d\n", ret);
		return ret;
	}

	timer_setup(&power_volume_timer, power_volume_timeout, 0);

	platform_set_drvdata(pdev, info);

	return 0;
}

static int pmic_key_remove(struct platform_device *pdev)
{
	struct pmic_key_info *info = platform_get_drvdata(pdev);

	if (info != NULL)
		input_unregister_device(info->input);
	return 0;
}

static int pmic_key_suspend(struct device *dev)
{

	dev_info(dev, "%s ++\n", __func__);

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static int pmic_key_resume(struct device *dev)
{

	dev_info(dev, "%s ++\n", __func__);

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static const struct pmic_key_data powerkey_data = {
	.name = "pmic_pwrkey",
	.phys = "pmic_pwrkey/input0",
	.key_irq_cfg = g_event_powerkey_cfg,
};

static const struct pmic_key_data volumedown_data = {
	.name = "volumedown",
	.phys = "pmic_volumedown/input0",
	.key_irq_cfg = g_event_volumedown_cfg,
};

static const struct pmic_key_data volumeup_data = {
	.name = "volumeup",
	.phys = "pmic_volumeup/input0",
	.key_irq_cfg = g_event_volumeup_cfg,
};

static SIMPLE_DEV_PM_OPS(pmic_key_pm_ops, pmic_key_suspend, pmic_key_resume);

static const struct of_device_id pmic_key_id_table[] = {
	{ .compatible = "xring,pmic-pwrkey", .data = &powerkey_data },
	{ .compatible = "xring,pmic-volumedown", .data = &volumedown_data },
	{ .compatible = "xring,pmic-volumeup", .data = &volumeup_data },
	{ }
};
MODULE_DEVICE_TABLE(of, pmic_key_id_table);

static struct platform_driver pmic_key_driver = {
	.probe = pmic_key_probe,
	.remove = pmic_key_remove,
	.driver = {
		.name = "xr-pmic-key",
		.pm = &pmic_key_pm_ops,
		.of_match_table = of_match_ptr(pmic_key_id_table),
	},
};

int xr_pmic_key_init(void)
{
	return platform_driver_register(&pmic_key_driver);
}

void xr_pmic_key_exit(void)
{
	platform_driver_unregister(&pmic_key_driver);
}
