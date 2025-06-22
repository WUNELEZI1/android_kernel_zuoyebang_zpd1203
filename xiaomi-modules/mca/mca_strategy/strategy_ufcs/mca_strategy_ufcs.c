
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/subpmic/subpmic_class.h>
#include "inc/xm_subpmic_protocol.h"

static int xm_subpmic_protocol_probe(struct platform_device *pdev)
{
	pr_err("%s: Start\n", __func__);

	pr_err("%s: End\n", __func__);

	return 0;
}

static int xm_subpmic_protocol_remove(struct platform_device *pdev)
{
	return 0;
}

static void xm_subpmic_protocol_shutdown(struct platform_device *pdev)
{
	return;
}

static const struct of_device_id match_table[] = {
	{.compatible = "xiaomi,subpmic-protocol"},
	{},
};

static struct platform_driver xm_subpmic_protocol_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "xm_subpmic_protocol",
		.of_match_table = match_table,
	},
	.probe = xm_subpmic_protocol_probe,
	.remove = xm_subpmic_protocol_remove,
	.shutdown = xm_subpmic_protocol_shutdown,
};

static int __init xm_subpmic_protocol_init(void)
{
	return platform_driver_register(&xm_subpmic_protocol_driver);
}
module_init(xm_subpmic_protocol_init);

static void __exit xm_subpmic_protocol_exit(void)
{
	platform_driver_unregister(&xm_subpmic_protocol_driver);
}
module_exit(xm_subpmic_protocol_exit);

MODULE_DESCRIPTION("Xiaomi Sub Pmic Protocol");
MODULE_AUTHOR("getian@xiaomi.com");
MODULE_LICENSE("GPL v2");
