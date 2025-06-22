// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-repeater.c - Repeater Driver for XRing USB.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-repeater.h"

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pinctrl/consumer.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/seq_file.h>
#include <linux/types.h>

/* eUSB2 repeater registers */
#define RESET_CONTROL			0x01
#define LINK_CONTROL1			0x02
#define LINK_CONTROL2			0x03
#define eUSB2_RX_CONTROL		0x04
#define eUSB2_TX_CONTROL		0x05
#define USB2_RX_CONTROL			0x06
#define USB2_TX_CONTROL1		0x07
#define USB2_TX_CONTROL2		0x08
#define USB2_HS_TERMINATION		0x09
#define RAP_SIGNATURE			0x0D
#define VDX_CONTROL			0x0E
#define DEVICE_STATUS			0x0F
#define LINK_STATUS			0x10
#define REVISION_ID			0x13
#define CHIP_ID_0			0x14
#define CHIP_ID_1			0x15
#define CHIP_ID_2			0x16
#define MAX_REG_ADDR			0x17

struct xr_usb_repeater {
	struct device			*dev;
	struct usb_repeater		ur;
	struct regmap			*regmap;

	u32				*repeter_device_set;
	int				device_param_cnt;
	u32				*repeter_host_set;
	int				host_param_cnt;

	struct dentry			*root;
	struct pinctrl			*reset_pinctrl;
	struct pinctrl_state		*pinctrl_reset_state;
	struct pinctrl_state		*pinctrl_release_state;

};

static const struct regmap_config xrusb_repeater_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = MAX_REG_ADDR,
};

static int xr_usb_i2c_read_reg(struct xr_usb_repeater *er, u8 reg, u8 *val)
{
	int ret;
	unsigned int reg_val;

	ret = regmap_read(er->regmap, reg, &reg_val);
	if (ret < 0) {
		dev_err(er->dev, "Failed to read reg:0x%02x ret=%d\n", reg, ret);
		return ret;
	}

	*val = reg_val;
	dev_dbg(er->dev, "read reg:0x%02x val:0x%02x\n", reg, *val);

	return 0;
}

static int xr_usb_i2c_write_reg(struct xr_usb_repeater *er, u8 reg, u32 val)
{
	int ret;

	ret = regmap_write(er->regmap, reg, val);
	if (ret < 0) {
		dev_err(er->dev, "failed to write 0x%02x to reg: 0x%02x ret=%d\n", val, reg, ret);
		return ret;
	}

	dev_dbg(er->dev, "write reg:0x%02x val:0x%02x\n", reg, val);

	return 0;
}

static void eusb2_repeater_param_set(struct xr_usb_repeater *er, u32 *seq, u8 cnt)
{
	int i;

	for (i = 0; i < cnt; i = i+2) {
		dev_dbg(er->ur.dev, "write 0x%02x to 0x%02x\n", seq[i], seq[i+1]);
		xr_usb_i2c_write_reg(er, seq[i+1], seq[i]);
	}
}

static int xr_usb_repeater_init(struct usb_repeater *ur, enum phy_mode mode)
{
	struct xr_usb_repeater *er =
			container_of(ur, struct xr_usb_repeater, ur);
	u8 param_cnt;
	u32 *repeater_param;

	switch (mode) {
	case PHY_MODE_USB_HOST:
		param_cnt = er->host_param_cnt;
		repeater_param = er->repeter_host_set;
		break;
	case PHY_MODE_USB_DEVICE:
		param_cnt = er->device_param_cnt;
		repeater_param = er->repeter_device_set;
		break;
	default:
		dev_err(ur->dev, "mode invalid\n");
		return -EINVAL;
	}
	/* set repeater value when init */
	eusb2_repeater_param_set(er, repeater_param, param_cnt);

	return 0;
}

static int repeater_get_pinctrl(struct xr_usb_repeater *er)
{
	struct device *dev = er->dev;
	int ret;

	/* get pinctrl handle */
	er->reset_pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(er->reset_pinctrl)) {
		ret = -ENODEV;
		dev_err(dev, "%s: Target does not use pinctrl %d\n", __func__, ret);
		goto err_get_pinctrl_handle;
	}

	/* find pinctrl state by name */
	er->pinctrl_reset_state = pinctrl_lookup_state(er->reset_pinctrl, "reset");
	if (IS_ERR_OR_NULL(er->pinctrl_reset_state)) {
		ret = -ENODEV;
		dev_err(dev, "%s: Can not lookup default pinstate %d\n", __func__, ret);
		goto err_find_pinctrl_reset;
	}

	er->pinctrl_release_state = pinctrl_lookup_state(er->reset_pinctrl, "release");
	if (IS_ERR_OR_NULL(er->pinctrl_release_state)) {
		ret = -ENODEV;
		dev_err(dev, "%s: Can not lookup default pinstate %d\n", __func__, ret);
		goto err_find_pinctrl_release;
	}

	return 0;

err_find_pinctrl_release:
	er->pinctrl_reset_state = NULL;
err_find_pinctrl_reset:
	devm_pinctrl_put(er->reset_pinctrl);
err_get_pinctrl_handle:
	return ret;
}

static int xr_usb_repeater_reset(struct usb_repeater *ur, bool set)
{
	struct xr_usb_repeater *er =
			container_of(ur, struct xr_usb_repeater, ur);
	int ret = -EFAULT;

	dev_info(ur->dev, "reset gpio:%s\n",
			set ? "assert" : "deassert");

	if (set)
		ret = pinctrl_select_state(er->reset_pinctrl, er->pinctrl_reset_state);
	else
		ret = pinctrl_select_state(er->reset_pinctrl, er->pinctrl_release_state);

	if (ret)
		dev_err(ur->dev, "repeater set %s fail\n",
			set ? "assert" : "deassert");

	return ret;
}

static const struct i2c_device_id usb_repeater_id[] = {
	{ "PTN3222", 0 },
	{}
};

static const struct of_device_id xr_usb_repeater_id_table[] = {
	{
		.compatible = "usb-repeater,PTN3222",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, xr_usb_repeater_id_table);

struct i2c_client *xr_usb_repeater_get_client(struct device_node *node)
{
	struct i2c_client *client;

	client = of_find_i2c_device_by_node(node);
	if (!client)
		return NULL;

	return client;
}

struct usb_repeater *usb_get_repeater_by_phandle(struct device *dev,
	const char *phandle, u8 index)
{
	struct device_node *node;
	struct xr_usb_repeater *repeater;
	struct i2c_client *client;

	if (!dev || !phandle)
		return ERR_PTR(-EINVAL);

	if (!dev->of_node) {
		dev_dbg(dev, "device does not have a device node entry\n");
		return ERR_PTR(-EINVAL);
	}

	node = of_parse_phandle(dev->of_node, phandle, index);
	if (!node) {
		dev_dbg(dev, "failed to get %s phandle in %pOF node\n", phandle,
			dev->of_node);
		return ERR_PTR(-ENODEV);
	}

	client = xr_usb_repeater_get_client(node);
	of_node_put(node);
	if (!client)
		return ERR_PTR(-EPROBE_DEFER);

	repeater = i2c_get_clientdata(client);
	if (!repeater) {
		put_device(&client->dev);
		return ERR_PTR(-EPROBE_DEFER);
	}

	return &repeater->ur;
}
EXPORT_SYMBOL_GPL(usb_get_repeater_by_phandle);

void usb_put_repeater(struct usb_repeater *repeater)
{
	if (!IS_ERR_OR_NULL(repeater))
		put_device(repeater->dev);
}
EXPORT_SYMBOL_GPL(usb_put_repeater);

static int regmap_map_show(struct seq_file *s, void *unused)
{
	struct xr_usb_repeater *repeater = NULL;
	unsigned int i = 0;
	u8 val = 0;
	int ret;

	repeater = s->private;
	if (!repeater) {
		pr_err("repeater is NULL, show fail\n");
		return -EFAULT;
	}

	for (i = 0; i < MAX_REG_ADDR; i++) {
		/* Format the register */
		seq_printf(s, "%.*x: ", (int)(2 * sizeof(u8)), i);

		/* Format the value, write all X if we can't read */
		ret = xr_usb_i2c_read_reg(repeater, i, &val);
		if (ret == 0)
			seq_printf(s, "%.*x\n", (int)(2 * sizeof(u8)), val);
		else
			seq_puts(s, "XX\n");
	}

	seq_puts(s, "repeater param get done\n");
	return 0;
}

static int regmap_map_open_file(struct inode *inode, struct file *file)
{
	return single_open(file, regmap_map_show, inode->i_private);
}

static ssize_t regmap_map_write_file(struct file *file,
				     const char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct seq_file *s = NULL;
	struct xr_usb_repeater *repeater = NULL;
	char buf[32];
	size_t buf_size = min(count, (sizeof(buf) - 1));
	char *args, *field;
	char *delim = " ";
	unsigned int reg, value;
	int ret;

	s = file->private_data;
	if (!s) {
		pr_err("private data is NULL\n");
		return -EINVAL;
	}

	repeater = s->private;
	if (!repeater) {
		pr_err("repeater is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = 0;
	args = (char *)buf;
	field = strsep(&args, delim);
	if (!field || !args) {
		pr_err("command is invalid\n");
		return -EINVAL;
	}

	ret = kstrtouint(field, 16, &reg);
	if (ret) {
		pr_err("invalid scan reg\n");
		return -EINVAL;
	}

	ret = kstrtouint(args, 16, &value);
	if (ret) {
		pr_err("invalid scan value\n");
		return -EINVAL;
	}

	if (reg >= MAX_REG_ADDR) {
		pr_err("scan address over max\n");
		return -EINVAL;
	}

	ret = xr_usb_i2c_write_reg(repeater, reg, value);
	if (ret < 0)
		return ret;

	return buf_size;
}

static const struct file_operations regmap_map_fops = {
	.open = regmap_map_open_file,
	.read = seq_read,
	.write = regmap_map_write_file,
	.llseek = seq_lseek,
};

static int repeater_reset_show(struct seq_file *s, void *unused)
{
	seq_puts(s, "echo 0/1 > reset\n");
	return 0;
}

static int repeater_reset_open(struct inode *inode, struct file *file)
{
	return single_open(file, repeater_reset_show, inode->i_private);
}

static ssize_t repeater_reset_write(struct file *file, const char __user *buff,
				     size_t len, loff_t *ppos)
{
	struct seq_file *s = NULL;
	char buf[32];
	size_t len_copy = min_t(size_t, sizeof(buf) - 1, len);
	struct xr_usb_repeater *repeater = NULL;
	int ret = -EINVAL;

	s = file->private_data;
	if (!s) {
		pr_err("private data is NULL\n");
		return -EINVAL;
	}

	repeater = s->private;
	if (!repeater) {
		pr_err("repeater is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(&buf, buff, len_copy))
		return -EFAULT;

	if (!strncmp(buf, "1", 1) || !strncmp(buf, "reset", strlen("reset")))
		ret = pinctrl_select_state(repeater->reset_pinctrl, repeater->pinctrl_reset_state);
	else if (!strncmp(buf, "0", 1) || !strncmp(buf, "release", strlen("release")))
		ret = pinctrl_select_state(repeater->reset_pinctrl, repeater->pinctrl_release_state);

	if (ret)
		pr_err("failed to set reset - %d\n", ret);
	return len_copy;
}

static const struct file_operations reset_fops = {
	.open = repeater_reset_open,
	.read = seq_read,
	.write = repeater_reset_write,
	.llseek = seq_lseek,
};

#if IS_ENABLED(CONFIG_XRING_USB_DEBUG)
static int usb_repeater_debugfs_init(struct xr_usb_repeater *repeater)
{
	struct dentry *root;
	struct dentry *file = NULL;

	root = debugfs_create_dir("usb_repeater", NULL);
	if (!root) {
		dev_err(repeater->dev, "failed to create usb_repeater dir\n");
		return -EFAULT;
	}

	file = debugfs_create_file("repeater", 0644, root,
				repeater, &regmap_map_fops);
	if (!file)
		dev_err(repeater->dev, "Failed to create repeater debugfs\n");

	file = debugfs_create_file("reset", 0644, root,
				repeater, &reset_fops);
	if (!file)
		dev_err(repeater->dev, "Failed to create reset debugfs\n");

	repeater->root = root;

	return 0;
}

void usb_repeater_debugfs_exit(struct dentry **root)
{
	if (!root)
		return;

	debugfs_remove(*root);
	*root = NULL;
}
#else
static int usb_repeater_debugfs_init(struct xr_usb_repeater *repeater)
{
	return 0;
}

void usb_repeater_debugfs_exit(struct dentry **root)
{
}
#endif

static int repeater_get_properties(struct xr_usb_repeater *repeater)
{
	struct device *dev = repeater->dev;
	int num_elem;
	int ret = 0;

	num_elem = of_property_count_elems_of_size(dev->of_node, "device-repeater",
				sizeof(*repeater->repeter_device_set));
	if (num_elem > 0) {
		if (num_elem % 2) {
			dev_err(dev, "invalid device repeater param\n");
			ret = -EINVAL;
			goto properties_err;
		}

		repeater->device_param_cnt = num_elem;
		repeater->repeter_device_set = devm_kcalloc(dev,
				repeater->device_param_cnt,
				sizeof(*repeater->repeter_device_set), GFP_KERNEL);
		if (!repeater->repeter_device_set) {
			ret = -ENOMEM;
			goto properties_err;
		}

		ret = of_property_read_u32_array(dev->of_node,
				"device-repeater",
				repeater->repeter_device_set,
				repeater->device_param_cnt);
		if (ret) {
			dev_err(dev, "device repeater param get failed:%d\n", ret);
			goto properties_err;
		}
	}

	num_elem = of_property_count_elems_of_size(dev->of_node, "host-repeater",
				sizeof(*repeater->repeter_host_set));
	if (num_elem > 0) {
		if (num_elem % 2) {
			dev_err(dev, "invalid host repeater param\n");
			ret = -EINVAL;
			goto properties_err;
		}

		repeater->host_param_cnt = num_elem;
		repeater->repeter_host_set = devm_kcalloc(dev,
				repeater->host_param_cnt,
				sizeof(*repeater->repeter_host_set), GFP_KERNEL);
		if (!repeater->repeter_host_set) {
			ret = -ENOMEM;
			goto properties_err;
		}

		ret = of_property_read_u32_array(dev->of_node,
				"host-repeater",
				repeater->repeter_host_set,
				repeater->host_param_cnt);
		if (ret) {
			dev_err(dev, "host repeater param get failed:%d\n", ret);
			goto properties_err;
		}
	}

	return 0;

properties_err:
	return ret;
}

static int xr_usb_repeater_probe(struct i2c_client *client)
{
	struct xr_usb_repeater *repeater;
	struct device *dev = &client->dev;
	int ret = 0;

	repeater = devm_kzalloc(dev, sizeof(*repeater), GFP_KERNEL);
	if (!repeater) {
		ret = -ENOMEM;
		goto err_probe;
	}

	repeater->dev = dev;

	repeater->regmap = devm_regmap_init_i2c(client, &xrusb_repeater_regmap);
	if (!repeater->regmap) {
		dev_err(dev, "failed to allocate register map\n");
		ret = -EINVAL;
		goto err_probe;
	}

	ret = repeater_get_properties(repeater);
	if (ret) {
		dev_err(dev, "repeater get properties error\n");
		goto err_probe;
	}

	ret = repeater_get_pinctrl(repeater);
	if (ret) {
		dev_err(dev, "repeater get reset pintctrl error\n");
		goto err_probe;
	}

	repeater->ur.dev = dev;
	repeater->ur.init = xr_usb_repeater_init;
	repeater->ur.reset = xr_usb_repeater_reset;

	ret = usb_repeater_debugfs_init(repeater);
	if (ret) {
		dev_err(dev, "failed to init repeater debugfs\n");
		goto err_probe;
	}

	i2c_set_clientdata(client, repeater);

	dev_info(dev, "%s success.\n", __func__);

	return 0;

err_probe:
	return ret;
}

static void xr_usb_repeater_remove(struct i2c_client *client)
{
	struct xr_usb_repeater *repeater = i2c_get_clientdata(client);

	if (!repeater)
		return;

	usb_repeater_debugfs_exit(&repeater->root);
}

static struct i2c_driver xr_usb_repeater_driver = {
	.probe	= xr_usb_repeater_probe,
	.remove = xr_usb_repeater_remove,
	.driver = {
		.name	= "xring_usb_repeater",
		.of_match_table = of_match_ptr(xr_usb_repeater_id_table),
	},
	.id_table = usb_repeater_id,
};

module_i2c_driver(xr_usb_repeater_driver);

MODULE_SOFTDEP("pre: xring_i2c");
MODULE_SOFTDEP("post: xring_usb_hsphy");
MODULE_DESCRIPTION("xr usb repeater driver");
MODULE_LICENSE("GPL v2");
