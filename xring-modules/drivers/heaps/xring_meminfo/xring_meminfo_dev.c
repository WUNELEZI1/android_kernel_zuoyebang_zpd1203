// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/oom.h>
#include <linux/printk.h>
#include <linux/mm.h>

#include "xring_meminfo.h"

long xr_meminfo_ioctl(struct file *f, unsigned int cmd, unsigned long data)
{
	long ret = 0;

	switch (cmd) {
	case XR_MEMINFO_IOC_SHOW_MEMINFO:
		ret = xr_process_show_meminfo((void *)data);
		break;
	case XR_MEMINFO_IOC_LEAK_DETECT:
		ret = xr_process_leak_detect((void *)data);
		break;
	case XR_MEMINFO_IOC_LEAK_READ:
		ret = xr_process_leak_read((void *)data);
		break;
	case XR_MEMINFO_IOC_SET_TH:
		ret = xr_process_set_th((void *)data);
		break;
	case XR_MEMINFO_IOC_GET_TH:
		ret = xr_process_get_val((void *)data);
		break;
	case XR_MEMINFO_IOC_TRANSMIT:
		ret = xr_process_transmit((void *)data);
		break;
	case XR_MEMINFO_IOC_SHOW_KILL:
		ret = xr_process_show_kill((void *)data);
		break;
	default:
		ret = -EINVAL;
		xrmeminfo_info("unsupported cmd %u\n", cmd);
		break;
	}

	return ret;
}

static const struct file_operations xr_meminfo_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = xr_meminfo_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = xr_meminfo_ioctl,
#endif
};

static struct miscdevice xr_meminfo_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "xr_meminfo",
	.fops = &xr_meminfo_fops,
};

static int xr_meminfo_probe(struct platform_device *pdev)
{
	int ret;

	ret = misc_register(&xr_meminfo_dev);

	if (ret < 0) {
		xrmeminfo_err("misc_register failed, ret = %d\n", ret);
		return ret;
	}

	xr_meminfo_dev.parent = &pdev->dev;
	ret = xr_meminfo_parameter_init();
	if (ret) {
		xrmeminfo_info("xr_meminfo_parameter_init failed\n");
		return ret;
	}
	ret = xr_memory_netlink_init();
	if (ret) {
		xrmeminfo_info("xr_memory_netlink_init failed\n");
		return ret;
	}
	ret = register_oom_notifier(&meminfo_oom_nb);
	if (ret) {
		xrmeminfo_info("register_oom_notifier failed\n");
		return ret;
	}
	ret = xr_lowmem_shrinker_init();
	if (ret) {
		xrmeminfo_info("xr_lowmem_shrinker_init failed\n");
		return ret;
	}
	ret = xr_memtrack_init();
	if (ret) {
		xrmeminfo_info("xr_memtrack_init failed\n");
		return ret;
	}
	return 0;
}

static int xr_meminfo_remove(struct platform_device *pdev)
{
	xr_memtrack_exit();
	xr_lowmem_shrinker_exit();
	unregister_oom_notifier(&meminfo_oom_nb);
	xr_memory_netlink_exit();
	xr_meminfo_parameter_exit();
	misc_deregister(&xr_meminfo_dev);
	return 0;
}

static const struct of_device_id xr_meminfo_match_table[] = {
	{.compatible = "xring,meminfo"},
	{}};

MODULE_DEVICE_TABLE(of, xr_meminfo_match_table);

static struct platform_driver xr_meminfo_driver = {
	.driver = {
		.name = "xr_meminfo",
		.of_match_table = xr_meminfo_match_table,
	},
	.probe = xr_meminfo_probe,
	.remove = xr_meminfo_remove,
};

int xr_meminfo_init(void)
{
	return platform_driver_register(&xr_meminfo_driver);
}

void xr_meminfo_exit(void)
{
	platform_driver_unregister(&xr_meminfo_driver);
}
