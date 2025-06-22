// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <dt-bindings/xring/xr_thermal_cdev.h>

struct gpu_hotplug_cdev {
	struct device_node *np;
	unsigned int *hp_index_table;
	int cur_state;
	int max_state;
	char cdev_name[THERMAL_NAME_LENGTH];
	struct thermal_cooling_device *cdev;
};
static struct gpu_hotplug_cdev *gpu_hp_cdev;
struct xr_gpuhp_cdev_ops *gpuhp_cdev_ops;

int xr_gpuhp_cdev_ops_register(struct xr_gpuhp_cdev_ops *ops)
{
	if (!ops) {
		pr_err("gpuhp cdev register:ops is null\n");
		return -ENOMEM;
	}

	gpuhp_cdev_ops = ops;

	return 0;
}
EXPORT_SYMBOL(xr_gpuhp_cdev_ops_register);

void xr_gpuhp_cdev_ops_unregister(void)
{
	gpuhp_cdev_ops = NULL;
}
EXPORT_SYMBOL(xr_gpuhp_cdev_ops_unregister);

static int gpu_thermal_cores_control(unsigned int core_mask_index)
{
	int ret;

	if (!gpuhp_cdev_ops) {
		pr_err("gpuhp ops not init!\n");
		return -ENOMEM;
	}

	pr_debug("gpuhp cdev: coremask %u!\n", core_mask_index);
	ret = gpuhp_cdev_ops->update_hotplug_coremask(gpuhp_cdev_ops->data, core_mask_index);
	if (ret)
		pr_err("gpuhp update coremask fail %d!\n", ret);

	return ret;
}

static int gpu_hotplug_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	struct gpu_hotplug_cdev *hp_cdev = cdev->devdata;
	unsigned int core_mask_index;

	/* Request state should be less than max_level */
	if (state > hp_cdev->max_state)
		return -EINVAL;

	/* Check if the old cooling action is same as new cooling action */
	if (state == hp_cdev->cur_state)
		return 0;

	core_mask_index = hp_cdev->hp_index_table[state];
	pr_debug("cdev:%s Limit:%u\n", cdev->type, core_mask_index);

	gpu_thermal_cores_control(core_mask_index);

	hp_cdev->cur_state = state;

	return 0;
}

static int gpu_hotplug_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct gpu_hotplug_cdev *hp_cdev = cdev->devdata;

	*state = hp_cdev->cur_state;

	return 0;
}

static int gpu_hotplug_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct gpu_hotplug_cdev *hp_cdev = cdev->devdata;

	*state =  hp_cdev->max_state;

	return 0;
}

static struct thermal_cooling_device_ops gpu_hot_cooling_ops = {
	.get_max_state = gpu_hotplug_get_max_state,
	.get_cur_state = gpu_hotplug_get_cur_state,
	.set_cur_state = gpu_hotplug_set_cur_state,
};

static void gpu_hotplug_register_cdev(struct gpu_hotplug_cdev *hp_cdev)
{
	unsigned int *hp_index_table = NULL;
	const char *alias = NULL;
	unsigned int hot_ct;
	int ret, i;

	ret = of_property_read_string(hp_cdev->np,
			"xring,cdev-alias", &alias);
	if (ret) {
		pr_err("dev hotplug missing cdev-alias:%d\n", ret);
		return;
	}
	strscpy(hp_cdev->cdev_name, alias,
				THERMAL_NAME_LENGTH);

	hot_ct = of_property_count_u32_elems(hp_cdev->np, "xring,hot-index-table");
	hp_index_table = kcalloc(hot_ct, sizeof(*hp_index_table), GFP_KERNEL);
	if (!hp_index_table) {
		pr_err("kcalloc hot index table fail: no-mem!\n");
		return;
	}
	ret = of_property_read_u32_array(hp_cdev->np, "xring,hot-index-table",
				   hp_index_table, hot_ct);
	if (ret != 0) {
		pr_err("get hot index table err:%d!\n", ret);
		goto exit;
	}
	hp_cdev->cur_state = 0;
	hp_cdev->max_state = hot_ct-1;
	hp_cdev->hp_index_table = hp_index_table;
	for (i = 0; i < hot_ct; i++)
		pr_debug("%d. hot index table:%d\n", i, hp_index_table[i]);

	hp_cdev->cdev = thermal_of_cooling_device_register(
					hp_cdev->np,
					hp_cdev->cdev_name,
					hp_cdev,
					&gpu_hot_cooling_ops);
	if (IS_ERR(hp_cdev->cdev)) {
		pr_err("Cooling register failed for %s, ret:%ld\n",
			hp_cdev->cdev_name, PTR_ERR(hp_cdev->cdev));
		hp_cdev->cdev = NULL;
		goto exit;
	}
	pr_debug("[%s] cdev registered.\n", hp_cdev->cdev_name);
	return;
exit:
	kfree(hp_cdev->hp_index_table);
}

static int gpu_hotplug_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	gpu_hp_cdev = devm_kzalloc(dev, sizeof(*gpu_hp_cdev), GFP_KERNEL);
	if (!gpu_hp_cdev)
		return -ENOMEM;

	gpu_hp_cdev->cdev = NULL;
	gpu_hp_cdev->np = pdev->dev.of_node;

	gpu_hotplug_register_cdev(gpu_hp_cdev);

	return 0;
}

static int gpu_hotplug_remove(struct platform_device *pdev)
{
	if (gpu_hp_cdev->cdev) {
		thermal_cooling_device_unregister(gpu_hp_cdev->cdev);
		kfree(gpu_hp_cdev->hp_index_table);
		gpu_hp_cdev->cdev = NULL;
	}

	return 0;
}

static const struct of_device_id gpu_hotplug_match[] = {
	{ .compatible = "xring,gpu-hotplug", },
	{},
};

static struct platform_driver gpu_hotplug_driver = {
	.probe		= gpu_hotplug_probe,
	.remove         = gpu_hotplug_remove,
	.driver		= {
		.name = KBUILD_MODNAME,
		.of_match_table = gpu_hotplug_match,
	},
};
module_platform_driver(gpu_hotplug_driver);
MODULE_DESCRIPTION("GPU Hotplug cooling device driver");
MODULE_LICENSE("GPL");
