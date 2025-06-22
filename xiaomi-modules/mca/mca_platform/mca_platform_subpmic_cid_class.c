#include <linux/module.h>
#include <linux/platform_device.h>
#include <mca/platform/platform_sc6601a_cid_class.h>
#include <mca/common/mca_log.h>
#include <mca/platform/platform_buckchg_class.h>
struct platform_class_buckchg_sc6601a_cid_data {
	void *data;
	struct platform_class_buckchg_sc6601a_cid_ops *ops;
};

#define platform_class_buckchg_cid_invalid_ops(data, name) (!data || !(data->ops) || !(data->ops->name))

static struct platform_class_buckchg_sc6601a_cid_data platform_buckchg_cid_ops_data[MAX_BUCK_CHARGER];

static struct platform_class_buckchg_sc6601a_cid_data *platform_class_buckchg_cid_get_ops_data(unsigned int role)
{
	if (role >= MAX_BUCK_CHARGER)
		return NULL;

	return &platform_buckchg_cid_ops_data[role];
}

bool platform_class_buckchg_cid_is_init_ok(void)
{
	return (platform_buckchg_cid_ops_data[MAIN_BUCK_CHARGER].ops != NULL);
}
EXPORT_SYMBOL(platform_class_buckchg_cid_is_init_ok);

int platform_class_buckchg_ops_cid_register(unsigned int role, void *data, struct platform_class_buckchg_sc6601a_cid_ops *ops)
{
	if (role >= MAX_BUCK_CHARGER)
		return -1;

	platform_buckchg_cid_ops_data[role].data = data;
	platform_buckchg_cid_ops_data[role].ops = ops;

	return 0;
}
EXPORT_SYMBOL(platform_class_buckchg_ops_cid_register);

int platform_class_buckchg_ops_get_cid_status(unsigned int role, bool *status)
{
	struct platform_class_buckchg_sc6601a_cid_data *temp_data = platform_class_buckchg_cid_get_ops_data(role);

	if (platform_class_buckchg_cid_invalid_ops(temp_data, get_cid_status))
		return -1;

	return temp_data->ops->get_cid_status(status, temp_data->data);
}
EXPORT_SYMBOL(platform_class_buckchg_ops_get_cid_status);

static struct platform_driver platform_class_buckchg_sc6601a_cid_driver = {
	.driver	= {
		.name = "sc6601a_cid_class",
	},
};

module_platform_driver(platform_class_buckchg_sc6601a_cid_driver);
MODULE_DESCRIPTION("mca sc6601a cid class");
MODULE_AUTHOR("tianye9@xiaomi.com");
MODULE_LICENSE("GPL v2");