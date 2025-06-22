#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>

enum
{
	HALL_LOG_INFO_LEVEL = 0,
	HALL_LOG_ERROR_LEVEL = 1
};

#define HALL_LOG(level, format, args...)                                                                                              \
	do                                                                                                                                \
	{                                                                                                                                 \
		struct timespec64 ts;                                                                                                         \
		unsigned long local_time;                                                                                                     \
		struct rtc_time tm;                                                                                                           \
		ktime_get_real_ts64(&ts);                                                                                                     \
		local_time = (u32)(ts.tv_sec - (sys_tz.tz_minuteswest * 60));                                                                 \
		rtc_time64_to_tm(local_time, &tm);                                                                                            \
		switch (level)                                                                                                                \
		{                                                                                                                             \
		case HALL_LOG_INFO_LEVEL:                                                                                                     \
			pr_info("[%02d:%02d:%02d.%03zu] [xiaomi-hall]: " format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, ##args); \
			break;                                                                                                                    \
		case HALL_LOG_ERROR_LEVEL:                                                                                                    \
			pr_err("[%02d:%02d:%02d.%03zu] [xiaomi-hall]: " format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, ##args);  \
			break;                                                                                                                    \
		}                                                                                                                             \
	} while (0)

#define XIAOMI_HALL_NAME "xiaomi-hall"
#define MAX_LEN 32

struct xiaomi_hall_data
{
	const char *desc;
	int gpio;
	int gpio_irq;
	unsigned int code; // linux,code
	unsigned int type; // linux,input-type
	unsigned int flags;
	struct input_dev *input;
};

struct xiaomi_hall_drv_data
{
	int hall_algo_disable;
	int hall_count;
	struct xiaomi_hall_data *data;
	struct input_dev *input;
};

struct xiaomi_hall_cdev
{
	struct cdev chrdev;
	struct device *dev;
	struct class *class;
	dev_t devid;
};

static struct xiaomi_hall_cdev xiaomi_hall_dev;

static int xiaomi_hall_fops_open(struct inode *inode, struct file *file)
{
	file->private_data = (void *)(&xiaomi_hall_dev);
	HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_fops_open.\n");
	return 0;
}

static int xiaomi_hall_fops_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_fops_release.\n");
	return 0;
}

static ssize_t xiaomi_hall_fops_read(struct file *file, char __user *buf,
									 size_t count, loff_t *pos)
{
	int i;
	int len = 0;
	int found = 0;
	int gpio_value;
	struct input_event event = {0};
	struct xiaomi_hall_cdev *cdev = file->private_data;
	struct xiaomi_hall_drv_data *drvdata = dev_get_drvdata(cdev->dev);

	for (i = 0; i < drvdata->hall_count; ++i)
	{
		if (!strcmp("lid_hall", drvdata->data[i].desc))
		{
			if (drvdata->data[i].gpio > 0)
			{
				gpio_value = gpio_get_value(drvdata->data[i].gpio);
				event.type = drvdata->data[i].type;
				event.code = drvdata->data[i].code;
				event.value = (drvdata->data[i].flags & 0x1) ? (!gpio_value) : gpio_value;
				HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_fops_read, tyep=%d,code=%d,value=%d\n",
						 event.type, event.code, event.value);
				if (count < sizeof(event))
				{
					HALL_LOG(HALL_LOG_ERROR_LEVEL, "xiaomi_hall_fops_read, buf size < sizeof(input event)\n");
					break;
				}
				else
				{
					if (copy_to_user(buf, &event, sizeof(event)))
					{
						HALL_LOG(HALL_LOG_ERROR_LEVEL, "xiaomi_hall_fops_read, copy_to_user fail.\n");
						break;
					}
					len = sizeof(event);
					found = 1;
				}
			}
			else
			{
				HALL_LOG(HALL_LOG_ERROR_LEVEL, "xiaomi_hall_fops_read, gpio number < 0\n");
			}
		}
	}

	if (!found)
	{
		HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_fops_read, not find lid hall gpio\n");
		len = -1;
	}

	// msleep(100);

	return len;
}

static ssize_t xiaomi_hall_fops_write(struct file *file,
									  const char __user *buf, size_t count, loff_t *pos)
{
	return -ENOSYS;
}

static const struct file_operations xiaomi_hall_fops = {
	.owner = THIS_MODULE,
	.open = xiaomi_hall_fops_open,
	.read = xiaomi_hall_fops_read,
	.write = xiaomi_hall_fops_write,
	.release = xiaomi_hall_fops_release,
};

struct hall_ipc_msg
{
	uint32_t type;
	uint32_t state;
	uint32_t count;
};

static void check_hall_state(struct ipc_pkt *pkt, struct device *dev)
{
	struct hall_ipc_msg *msg = (struct hall_ipc_msg *)pkt->data;
	struct xiaomi_hall_drv_data *drvdata = dev_get_drvdata(dev);

	HALL_LOG(HALL_LOG_INFO_LEVEL, "hall type: %d, state %d, count %d\n", msg->type, msg->state, msg->count);

	if (msg->type == 0)
	{
		input_event(drvdata->data[0].input, drvdata->data[0].type, drvdata->data[0].code,
					(msg->state & 0x1) ? 1 : 0);
		input_sync(drvdata->data[0].input);
	}
	else
	{
		input_event(drvdata->data[1].input, drvdata->data[1].type, drvdata->data[1].code,
					(msg->state & 0x1) ? 1 : 0);
		input_sync(drvdata->data[1].input);
	}
}

static int hall_ipc_recv_cb(struct ipc_pkt *pkt, uint16_t tag, uint8_t cmd, void *arg)
{
	struct device *dev = (struct device *)arg;

	HALL_LOG(HALL_LOG_INFO_LEVEL, "%s line%d, tag %d\n", __func__, __LINE__, tag);
	if (tag != TAG_SH_HALL)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "tag err %d\n", tag);
		return 0;
	}

	if (cmd == CMD_SH_HALL_EVENT)
		check_hall_state(pkt, dev);
	else
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "cmd err %d\n", cmd);

	return 0;
}

int hall_ipc_register(bool *register_done, struct device *dev)
{
	int ret = 0;
	struct ipc_notifier_info hall_notifier_info;

	hall_notifier_info.callback = hall_ipc_recv_cb;
	hall_notifier_info.arg = dev;

	if (IS_ERR_OR_NULL(register_done))
		return -EINVAL;

	ret = sh_register_recv_notifier(IPC_VC_AO_NS_SH_AP_DEFAULT, TAG_SH_HALL, &hall_notifier_info);
	if (ret)
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "hall register recv fail\n");
	else
		*register_done = true;

	HALL_LOG(HALL_LOG_INFO_LEVEL, "hall register %d\n", ret);
	return ret;
}

int hall_ipc_unregister(bool *need_unregister, struct device *dev)
{
	int ret = 0;
	struct ipc_notifier_info hall_notifier_info;

	hall_notifier_info.callback = hall_ipc_recv_cb;
	hall_notifier_info.arg = dev;

	if (IS_ERR_OR_NULL(need_unregister))
		return -EINVAL;

	if (*need_unregister)
	{
		ret = sh_unregister_recv_notifier(IPC_VC_AO_NS_SH_AP_DEFAULT, TAG_SH_HALL,
										  &hall_notifier_info);
		if (ret)
			HALL_LOG(HALL_LOG_ERROR_LEVEL, "hall unregister recv fail\n");
	}

	*need_unregister = false;
	return ret;
}

irqreturn_t
lid_hall_gpio_irq_handler(int irq, void *dev_id)
{
	int gpio_value;
	struct xiaomi_hall_data *data = dev_id;

	gpio_value = gpio_get_value(data->gpio);
	input_event(data->input, data->type, data->code,
				(data->flags & 0x1) ? (!gpio_value) : gpio_value);
	HALL_LOG(HALL_LOG_INFO_LEVEL, "lid_hall_gpio_irq_handler, input type=%d, code=%d, val=%d\n",
			 data->type, data->code, (data->flags & 0x1) ? (!gpio_value) : gpio_value);
	input_sync(data->input);

	return IRQ_HANDLED;
}

static ssize_t
disable_algo_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct xiaomi_hall_drv_data *drvdata = dev_get_drvdata(dev);
	return sprintf(buf, drvdata->hall_algo_disable ? "hall algo is disable.\n" : "hall algo is enable.\n");
}

static ssize_t
disable_algo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int err = 0;
	int found = 0;
	int val;
	int i;
	struct xiaomi_hall_drv_data *drvdata = dev_get_drvdata(dev);
	bool need_unregister = false;

	hall_ipc_register(&need_unregister, dev);
	err = kstrtoint(buf, 16, &val);
	if (err)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, kstrtoint fail, err=%d.\n", err);
		return 0;
	}

	for (i = 0; i < drvdata->hall_count; ++i)
	{
		if (!strcmp("lid_hall", drvdata->data[i].desc))
		{
			found = 1;
			if (val == 1)
			{
				// Register irq for lid hall gpio (without algo).
				if (drvdata->data[i].gpio > 0)
				{
					err = devm_gpio_request(dev, drvdata->data[i].gpio, NULL);
					if (err)
					{
						HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, gpio_request fail, err=%d.\n", err);
						return 0;
					}
					drvdata->data[i].gpio_irq = gpio_to_irq(drvdata->data[i].gpio);
					err = devm_request_threaded_irq(dev, drvdata->data[i].gpio_irq, NULL,
													lid_hall_gpio_irq_handler,
													IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
													"8030_io_wakeup", &drvdata->data[i]);
					if (err < 0)
					{
						HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, devm_request_threaded_irq fail, err=%d.\n", err);
						goto err_register_irq;
					}
					enable_irq_wake(drvdata->data[i].gpio_irq);
				}
				//// Disable lid hall open_ir and close_irq (with hall algo).
				// if (drvdata->data[i].open_irq > 0 && drvdata->data[i].close_irq > 0) {
				//     disable_irq(drvdata->data[i].open_irq);
				//     disable_irq(drvdata->data[i].close_irq);
				// } else {
				//     HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, invalid open irq or close irq.\n");
				// }
				drvdata->hall_algo_disable = 1;
				HALL_LOG(HALL_LOG_INFO_LEVEL, "disable_algo_store, xiaomi_hall changes to gpio mode.\n");
			}
			else if (val == 0)
			{
				//// Enable lid hall open_ir and close_irq (with hall algo).
				// if (drvdata->data[i].open_irq > 0 && drvdata->data[i].close_irq > 0) {
				//     enable_irq(drvdata->data[i].open_irq);
				//     enable_irq(drvdata->data[i].close_irq);
				// }
				//  Unregister irq for lid hall gpio (without algo).
				if (drvdata->data[i].gpio > 0 && drvdata->data[i].gpio_irq)
				{
					disable_irq_wake(drvdata->data[i].gpio_irq);
					devm_free_irq(dev, drvdata->data[i].gpio_irq, &drvdata->data[i]);
					drvdata->data[i].gpio_irq = 0;
				}
				drvdata->hall_algo_disable = 0;
				HALL_LOG(HALL_LOG_INFO_LEVEL, "disable_algo_store, xiaomi_hall changes to hall algo mode.\n");
			}
			else
			{
				HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, unknown command.\n");
				return 0;
			}
		}
	}

	if (!found)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "disable_algo_store, lid hall not found.\n");
		return 0;
	}
	return count;

err_register_irq:

	return 0;
}

static DEVICE_ATTR(disable_algo, 0664, disable_algo_show, disable_algo_store);

static struct attribute *xiaomi_hall_attrs[] = {
	&dev_attr_disable_algo.attr,
	NULL,
};

static struct attribute_group xiaomi_hall_attrs_group = {
	.attrs = xiaomi_hall_attrs,
};

static int xiaomi_hall_chardev_init(struct xiaomi_hall_drv_data *drvdata)
{
	int ret;

	ret = alloc_chrdev_region(&xiaomi_hall_dev.devid, 0, 1, "hall");
	if (ret < 0)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "alloc_chrdev_region fail, err=%d.\n", ret);
		return ret;
	}

	cdev_init(&xiaomi_hall_dev.chrdev, &xiaomi_hall_fops);

	ret = cdev_add(&xiaomi_hall_dev.chrdev, xiaomi_hall_dev.devid, 1);
	if (ret < 0)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "cdev add fail, err=%d.\n", ret);
		goto err_cdev_add;
	}

	xiaomi_hall_dev.class = class_create("hall");
	if (IS_ERR(xiaomi_hall_dev.class))
	{
		ret = PTR_ERR(xiaomi_hall_dev.class);
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "class_create fail, err=%d.\n", ret);
		goto err_class_create;
	}

	xiaomi_hall_dev.dev = device_create(xiaomi_hall_dev.class,
										NULL,
										MKDEV(MAJOR(xiaomi_hall_dev.devid), 0),
										drvdata,
										"xiaomi_hall");
	if (IS_ERR(xiaomi_hall_dev.dev))
	{
		ret = PTR_ERR(xiaomi_hall_dev.dev);
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "device_create fail, err=%d.\n", ret);
		goto err_device_create;
	}

	ret = sysfs_create_group(&xiaomi_hall_dev.dev->kobj, &xiaomi_hall_attrs_group);
	if (ret < 0)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "sysfs_create_group fail, err=%d.\n", ret);
		goto err_sysfs_create;
	}

	return ret;

err_sysfs_create:
	device_destroy(xiaomi_hall_dev.class, MKDEV(MAJOR(xiaomi_hall_dev.devid), 0));
err_device_create:
	class_destroy(xiaomi_hall_dev.class);
err_class_create:
	cdev_del(&xiaomi_hall_dev.chrdev);
err_cdev_add:
	unregister_chrdev_region(xiaomi_hall_dev.devid, 1);
	return ret;
}

static int xiaomi_hall_open(struct input_dev *input)
{
	int i;
	int value;
	struct xiaomi_hall_data data;
	struct xiaomi_hall_drv_data *drvdata = input_get_drvdata(input);

	for (i = 0; i < drvdata->hall_count; ++i)
	{
		data = drvdata->data[i];
		value = gpio_get_value(data.gpio);
		value = (data.flags & 0x1) ? (!value) : value;
		HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_open, %s(%d) flag %d value %d\n",
				 data.desc, data.gpio, data.flags, value);
		input_event(input, data.type, data.code, value);
		input_sync(input);
	}

	return 0;
}

static void xiaomi_hall_close(struct input_dev *input)
{
	struct xiaomi_hall_drv_data *drvdata = input_get_drvdata(input);
	HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_close, hall_count=%d\n", drvdata->hall_count);
}

static int xiaomi_hall_get_devicetree(struct device *dev)
{
	int child_count = 0;
	int i = 1;
	int ret;
	struct fwnode_handle *child;
	struct xiaomi_hall_drv_data *drvdata;
	struct xiaomi_hall_data *hall_data;

	drvdata = devm_kzalloc(dev, sizeof(struct xiaomi_hall_drv_data), GFP_KERNEL);
	if (!drvdata)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "No memory for xiaomi_hall_drv_data.\n");
		return -ENOMEM;
	}

	child_count = device_get_child_node_count(dev);
	HALL_LOG(HALL_LOG_INFO_LEVEL, "devicetree has %d child %s.\n",
			 child_count, child_count > 1 ? "nodes" : "node");

	hall_data = devm_kzalloc(dev, sizeof(struct xiaomi_hall_data) * child_count, GFP_KERNEL);
	if (!hall_data)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "No memory for xiaomi_hall_data.\n");
		ret = -ENOMEM;
		goto err_hall_data;
	}

	drvdata->data = hall_data;
	drvdata->hall_count = child_count;

	device_for_each_child_node(dev, child)
	{
		ret = fwnode_property_read_string(child, "label", &hall_data->desc);
		if (ret == 0)
		{
			HALL_LOG(HALL_LOG_INFO_LEVEL, "node%d lable = %s.\n", i, hall_data->desc);
		}
		else
		{
			HALL_LOG(HALL_LOG_ERROR_LEVEL, "cant read node%d lable, err = %d.\n", i, ret);
			goto err_parse_tree;
		}

		ret = fwnode_property_read_u32(child, "linux,code", &hall_data->code);
		if (ret == 0)
		{
			HALL_LOG(HALL_LOG_INFO_LEVEL, "node%d linux,code = %u.\n", i, hall_data->code);
		}
		else
		{
			HALL_LOG(HALL_LOG_ERROR_LEVEL, "cant read node%d code, err = %d.\n", i, ret);
			goto err_parse_tree;
		}

		ret = fwnode_property_read_u32(child, "linux,input-type", &hall_data->type);
		if (ret == 0)
		{
			HALL_LOG(HALL_LOG_INFO_LEVEL, "node%d linux,type = %u.\n", i, hall_data->type);
		}
		else
		{
			HALL_LOG(HALL_LOG_ERROR_LEVEL, "cant read node%d type, err = %d.\n", i, ret);
			goto err_parse_tree;
		}

		if (is_of_node(child))
		{
			hall_data->gpio = of_get_named_gpio(to_of_node(child), "hall_pin", 0);
			uint32_t gpio_config[3];

			ret = of_property_read_u32_array(to_of_node(child), "hall_pin", gpio_config, 3);
			if (ret)
			{
				HALL_LOG(HALL_LOG_ERROR_LEVEL, "Failed to get ddr_info_reg!\n");
				return -EINVAL;
			}
			hall_data->flags = gpio_config[2];
			HALL_LOG(HALL_LOG_INFO_LEVEL, "hall_data->gpio %d flags %d\n", hall_data->gpio, hall_data->flags);
		}

		hall_data++;
		i++;
	}

	drvdata->hall_algo_disable = 0;
	dev_set_drvdata(dev, drvdata);
	return 0;

err_parse_tree:
	devm_kfree(dev, hall_data);
err_hall_data:
	devm_kfree(dev, drvdata);

	return ret;
}

static int xiaomi_hall_input_init(struct device *dev)
{
	int i;
	int ret;
	unsigned int *keycode;
	struct input_dev *input;
	struct xiaomi_hall_drv_data *drvdata = dev_get_drvdata(dev);

	input = devm_input_allocate_device(dev);
	if (!input)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "alloc input memory fail.\n");
		return -ENOMEM;
	}

	input->name = XIAOMI_HALL_NAME;
	input->phys = "xiaomi-hall/input0";
	input->dev.parent = dev;
	input->open = xiaomi_hall_open;
	input->close = xiaomi_hall_close;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0EFA;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	keycode = devm_kcalloc(dev, drvdata->hall_count, sizeof(unsigned int), GFP_KERNEL);
	if (!keycode)
		return -ENOMEM;

	for (i = 0; i < drvdata->hall_count; ++i)
	{
		input_set_capability(input, drvdata->data[i].type, drvdata->data[i].code);
		keycode[i] = drvdata->data[i].code;
	}

	input->keycode = keycode;
	input->keycodesize = sizeof(keycode[0]);
	input->keycodemax = drvdata->hall_count;

	drvdata->input = input;
	input_set_drvdata(input, drvdata);
	for (i = 0; i < drvdata->hall_count; ++i)
	{
		drvdata->data[i].input = input;
	}

	ret = input_register_device(input);
	if (ret)
	{
		HALL_LOG(HALL_LOG_ERROR_LEVEL, "register input device fail, err=%d.\n", ret);
		return ret;
	}
	HALL_LOG(HALL_LOG_INFO_LEVEL, "register input device done.\n");

	return 0;
}

static int xiaomi_hall_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;

	HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_probe start.\n");

	ret = xiaomi_hall_get_devicetree(dev);
	if (ret < 0)
		return ret;

	ret = xiaomi_hall_input_init(dev);
	if (ret < 0)
		return ret;

	ret = xiaomi_hall_chardev_init(dev_get_drvdata(dev));
	if (ret < 0)
		return ret;

	bool need_unregister = false;

	HALL_LOG(HALL_LOG_INFO_LEVEL, "%s hall_ipc_register.\n", __func__);
	hall_ipc_register(&need_unregister, dev);

	return 0;
}

static int xiaomi_hall_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;

	HALL_LOG(HALL_LOG_INFO_LEVEL, "xiaomi_hall_remove start.\n");

	device_destroy(xiaomi_hall_dev.class, MKDEV(MAJOR(xiaomi_hall_dev.devid), 0));
	class_destroy(xiaomi_hall_dev.class);
	cdev_del(&xiaomi_hall_dev.chrdev);
	unregister_chrdev_region(xiaomi_hall_dev.devid, 1);
	bool need_unregister = false;
	HALL_LOG(HALL_LOG_INFO_LEVEL, "%s hall_ipc_register.\n", __func__);
	hall_ipc_unregister(&need_unregister, dev);

	return ret;
}

static const struct of_device_id xiaomi_hall_of_match[] = {
	{
		.compatible = XIAOMI_HALL_NAME,
	},
	{},
};

static struct platform_driver xiaomi_hall_driver = {
	.probe = xiaomi_hall_probe,
	.remove = xiaomi_hall_remove,
	.driver = {
		.name = XIAOMI_HALL_NAME,
		.of_match_table = of_match_ptr(xiaomi_hall_of_match),
	}};

static int __init xiaomi_hall_init(void)
{
	return platform_driver_register(&xiaomi_hall_driver);
}

static void __exit xiaomi_hall_exit(void)
{
	platform_driver_unregister(&xiaomi_hall_driver);
}

MODULE_LICENSE("GPL");

module_init(xiaomi_hall_init);
module_exit(xiaomi_hall_exit);