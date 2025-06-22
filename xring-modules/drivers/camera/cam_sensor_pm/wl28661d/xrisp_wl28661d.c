// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)	"XRISP_DRV wl28661d: [%s] " fmt, __func__
#define dev_fmt(fmt)	"XRISP_DRV wl28661d: [%s] " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/mfd/core.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/unaligned.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/pm.h>
#include <linux/math.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_wl28661d.h"

#define volt2regval(wldo_volt_addr, volt)                  \
		(wldo_volt_addr < 0x5 ?                            \
		((volt) - 600 * MV_PER_V) / (6 * MV_PER_V)         \
		: ((volt) - 1200 * MV_PER_V) / 12500)              \

#define regval2volt(wldo_volt_addr, regval)                \
		(wldo_volt_addr < 0x5 ?                            \
		600 * MV_PER_V + 6 * MV_PER_V * (regval)           \
		: 1200 * MV_PER_V + 12500 * (regval))              \

static int xrisp_wl28661d_rgltr_setvolt(struct regulator_dev *rdev, int min_uV,
					int max_uV, unsigned int *selector)
{
	int ret = 0, vsel, uV = 0, uV_regVal;
	bool is_range;
	int lim_min_uV, lim_max_uV;
	int *wldo_volt_addr = NULL;

	if (!rdev) {
		pr_err("rdev is null");
		return -EINVAL;
	}
	if (min_uV > max_uV) {
		pr_err("request v=[%d, %d] invalid\n", min_uV, max_uV);
		return -EINVAL;
	}
	wldo_volt_addr = (int *)rdev_get_drvdata(rdev);
	if (IS_ERR_OR_NULL(wldo_volt_addr)) {
		pr_err("get wldo_volt_addr err");
		return -EINVAL;
	}
	is_range = min_uV != max_uV;
	lim_min_uV = rdev->desc->volt_table[0];
	lim_max_uV = rdev->desc->volt_table[rdev->desc->n_voltages - 1];
	if (max_uV < lim_min_uV || min_uV > lim_max_uV) {
		pr_err("request v=[%d, %d] is outside possible v=[%d, %d]\n",
				min_uV, max_uV, lim_min_uV, lim_max_uV);
		return -EINVAL;
	}
	if (is_range) {
		for (vsel = 0; vsel < rdev->desc->n_voltages; vsel++) {
			uV = rdev->desc->volt_table[vsel];
			if ((min_uV <= uV) && (uV <= max_uV))
				break;
		}
		if (vsel == rdev->desc->n_voltages) {
			pr_err("request v=[%d, %d], no suitable voltage found\n",
					min_uV, max_uV);
			return -EINVAL;
		}
	} else if (lim_min_uV <= max_uV && max_uV <= lim_max_uV) {
		uV = max_uV;
	} else {
		pr_err("request v=[%d, %d], no suitable voltage found\n",
				min_uV, max_uV);
		return -EINVAL;
	}
	uV_regVal = volt2regval(*wldo_volt_addr, uV);
	ret = regmap_write(rdev->regmap, *wldo_volt_addr, uV_regVal);
	if (ret < 0) {
		pr_err("regulator %s set voltage err, ret=%d",
				rdev->constraints->name, ret);
		return -EINVAL;
	}
	return ret;
}

static int xrisp_wl28661d_rgltr_getvolt(struct regulator_dev *rdev)
{
	int ret = 0;
	int *wldo_volt_addr = NULL;
	unsigned int volt_read = 0;

	if (!rdev) {
		pr_err("rdev is null");
		return -EINVAL;
	}
	wldo_volt_addr = (int *)rdev_get_drvdata(rdev);
	if (IS_ERR_OR_NULL(wldo_volt_addr)) {
		pr_err("get wldo_volt_addr err");
		return -EINVAL;
	}
	ret = regmap_read(rdev->regmap, *wldo_volt_addr, &volt_read);
	if (ret < 0) {
		pr_err("regulator %s read  err, ret=%d", rdev->constraints->name, ret);
		return ret;
	}
	return regval2volt(*wldo_volt_addr, volt_read);
}

static int xrisp_wl28661d_rgltr_enable(struct regulator_dev *rdev)
{
	if (!rdev) {
		pr_err("rdev NULL ERR!");
		return -EINVAL;
	}
	return regulator_enable_regmap(rdev);
};

static const struct regulator_ops wl28661d_reg_control_ops = {
	.enable		= xrisp_wl28661d_rgltr_enable,
	.disable	= regulator_disable_regmap,
	.is_enabled	= regulator_is_enabled_regmap,
	.set_voltage = xrisp_wl28661d_rgltr_setvolt,
	.get_voltage = xrisp_wl28661d_rgltr_getvolt,
};

#define WL28661D_REG_DESC(_name, _id, _en_reg, _en_bits, _ops) \
{							                                   \
	.name = _name,			                                   \
	.id = _id,			                                       \
	.n_voltages = 2,				                           \
	.ops = &_ops,					                           \
	.enable_reg = _en_reg,				                       \
	.enable_val = _en_bits,				                       \
	.enable_mask = _en_bits,			                       \
	.type = REGULATOR_VOLTAGE,			                       \
	.owner = THIS_MODULE,				                       \
}

static struct regulator_desc wl28661d_chip_rgltr_desc[] = {
	WL28661D_REG_DESC("WL2_LDO1", 0, WL28661D_EN_ADDR, BIT(0), wl28661d_reg_control_ops),
	WL28661D_REG_DESC("WL2_LDO2", 0, WL28661D_EN_ADDR, BIT(1), wl28661d_reg_control_ops),
	WL28661D_REG_DESC("WL2_LDO3", 0, WL28661D_EN_ADDR, BIT(2), wl28661d_reg_control_ops),
	WL28661D_REG_DESC("WL2_LDO4", 0, WL28661D_EN_ADDR, BIT(3), wl28661d_reg_control_ops),
};

static const struct regmap_config wl28661d_chip_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xFF,
};

static int wl28661d_chipid_compare(struct wl28661d_chip *chip)
{
	int ret = 0;
	unsigned int chip_id = 0;

	if (!chip) {
		pr_err("chip NULL ERR!");
		return -EINVAL;
	}
	ret = regmap_read(chip->rmap, WL28661D_ID_ADDR, &chip_id);
	if (ret) {
		pr_err("%s: read chipid err, ret=%d", chip->name, ret);
		return -EINVAL;
	}
	if (chip_id != WL28661D_ID) {
		pr_err("%s: chipid not match, id=%d, exp=%d!\n", chip->name,
		       chip_id, WL28661D_ID);
		return -EINVAL;
	}
	return ret;
}
#ifdef WL28661D_DEBUG
static void wl28661d_print_volt_reg(struct wl28661d_chip *chip)
{
	int i, ret;
	unsigned int reg_val = 0;

	if (!chip) {
		pr_err("chip NULL ERR!");
		return;
	}
	for (i = 0; i < chip->rgltr_num; i++) {
		ret = regmap_read(chip->rmap, chip->wldo_volt_addr[i], &reg_val);
		if (ret)
			pr_err("%s: read addr 0x%x failed\n", chip->name,
				chip->wldo_volt_addr[i]);
		else
			pr_info("%s: read 0x%x=0x%x", chip->name,
				chip->wldo_volt_addr[i], reg_val);
	}
}
#endif
static int wl28661d_init(struct wl28661d_chip *chip)
{
	int ret = 0;

	if (!chip) {
		pr_err("chip NULL ERR!");
		return -EINVAL;
	}
	ret = wl28661d_chipid_compare(chip);
	if (ret < 0) {
		pr_err("read id failed\n");
		return ret;
	}
#ifdef WL28661D_DEBUG
	wl28661d_print_volt_reg(chip);
#endif
	return ret;
}

static int wl28661d_discharge_init(struct wl28661d_chip *chip, int ldo_id)
{
	int ret = 0;
	unsigned int reg_val = 0;

	if (!chip) {
		pr_err("chip NULL ERR!");
		return -EINVAL;
	}
	ret = regmap_read(chip->rmap, WL28661D_DISCHARGE_ADDR, &reg_val);
	if (ret < 0) {
		pr_err("discharge read register err, ret=%d", ret);
		return -EINVAL;
	}

	reg_val |= WL28661D_DISCHARGE_LDO_EN << ldo_id;
	ret = regmap_write(chip->rmap, WL28661D_DISCHARGE_ADDR, reg_val);
	if (ret < 0) {
		pr_err("discharge write err, ret=%d", ret);
		return -EINVAL;
	}

	return ret;
}

static ssize_t wr_reg_func(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	uint32_t get_val = 0;
	char user_data[60];
	struct wl28661d_chip *chip = NULL;

	if (!user_buf || !file || count >= sizeof(user_data)) {
		pr_err("user_buf or file NULL ERR!");
		return -EINVAL;
	}
	if (copy_from_user(user_data, user_buf, count) != 0) {
		pr_err("failed copying from user");
		return -EFAULT;
	}
	chip = (struct wl28661d_chip *)file->f_inode->i_private;
	if (!chip) {
		pr_err("chip is null!");
		return -EINVAL;
	}
	if (!chip->rmap) {
		pr_err("regmap get failed");
		return -EINVAL;
	}
	if (count <= 1) {
		pr_err("count val err!");
		return -EINVAL;
	}
	if (!strncmp(user_data, "read", count - 1)) {            /* read */
		if (regmap_read(chip->rmap, chip->reg_addr, &get_val)) {
			pr_err("%s: i2c read failed, reg=0x%x, val=0x%x", chip->name,
			       chip->reg_addr, get_val);
			return -EINVAL;
		}
		pr_info("%s: read: 0x%x = 0x%x\n", chip->name, chip->reg_addr, get_val);
	} else if (!strncmp(user_data, "write", count - 1))  {   /* write */
		if (regmap_write(chip->rmap, chip->reg_addr, chip->reg_value)) {
			pr_err("%s: i2c write failed, reg=0x%x, val=0x%x", chip->name,
			       chip->reg_addr, chip->reg_value);
			return -EINVAL;
		}
		pr_info("%s: write: 0x%x = 0x%x\n", chip->name,
				chip->reg_addr, chip->reg_value);
	}
	return count;
}
static ssize_t print_val_func(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[20];
	struct wl28661d_chip *chip = NULL;

	if (!user_buf || !file) {
		pr_err("user_buf or file NULL ERR!");
		return -EINVAL;
	}
	chip = (struct wl28661d_chip *)file->f_inode->i_private;
	if (!chip) {
		pr_err("chip is null!");
		return -EINVAL;
	}
	int len = snprintf(buf, sizeof(buf), "%d\n",
			chip->reg_value);
	pr_info("%s: write: 0x%x = 0x%x\n", chip->name,
				chip->reg_addr, chip->reg_value);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations wr_reg_ops = {
	.owner = THIS_MODULE,
	.write = wr_reg_func,
	.read  = print_val_func,
};

int wl2866_debugfs_create(struct wl28661d_chip *chip)
{
	struct dentry *debugfs_file = NULL;

	if (!chip) {
		pr_err("chip NULL ERR!");
		return -EINVAL;
	}
	chip->debugfs_dir = debugfs_create_dir(chip->name, NULL);
	if (IS_ERR_OR_NULL(chip->debugfs_dir)) {
		pr_info("dir create failed %ld\n", PTR_ERR(chip->debugfs_dir));
		return -EINVAL;
	}
	debugfs_create_u32("addr", 0600, chip->debugfs_dir, &chip->reg_addr);
	debugfs_create_u32("val", 0600, chip->debugfs_dir, &chip->reg_value);
	debugfs_file =
		debugfs_create_file("wr_reg", 0600, chip->debugfs_dir, chip, &wr_reg_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		pr_info("debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		debugfs_remove(chip->debugfs_dir);
		return -EINVAL;
	}
	pr_info("%s: debugfs create success", chip->name);
	return 0;
}


int wl28661d_probe(struct i2c_client *client)
{
	int i = 0;
	int ret = 0;
	int uV_regVal = 0;
	struct wl28661d_chip *chip = NULL;
	struct device_node *np_rgltr = NULL;
	struct regulator_dev *rgltr_dev = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config rgltr_config;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("check i2c functionality failed\n");
		return -EIO;
	}
	if (client->dev.of_node == NULL) {
		pr_err("client dev node is NULL, exit");
		return -EINVAL;
	}
	pr_info("%s: probe start\n", client->dev.of_node->name);
	chip = devm_kzalloc(&client->dev, sizeof(struct wl28661d_chip), GFP_KERNEL);
	if (!chip) {
		pr_err("chip malloc ERR!");
		return -ENOMEM;
	}
	chip->client = client;
	chip->dev = &client->dev;
	chip->name = client->dev.of_node->name;
	chip->reg_value = 0;
	dev_set_drvdata(chip->dev, chip);
	i2c_set_clientdata(chip->client, chip);
	chip->rmap = devm_regmap_init_i2c(client, &wl28661d_chip_regmap_config);
	if (IS_ERR_OR_NULL(chip->rmap)) {
		ret = PTR_ERR(chip->rmap);
		pr_err("%s: regmap_init failed with err: %d\n",
				chip->name, ret);
		goto init_err;
	}
	if (wl28661d_init(chip)) {
		pr_err("%s: init fail!\n", chip->name);
		goto init_err;
	}
	//regulator init
	chip->rgltr_num = of_property_count_strings(chip->dev->of_node, "regulator-names");
	if (chip->rgltr_num <= 0) {
		pr_err("no regulator-names found");
		chip->rgltr_num = 0;
		goto init_err;
	}
	chip->rgltr_name_arr =
		devm_kzalloc(chip->dev, sizeof(char *) * chip->rgltr_num, GFP_KERNEL);
	if (!chip->rgltr_name_arr) {
		pr_err("chip->rgltr_name_arr malloc ERR!");
		goto init_err;
	}
	chip->wldo_volt_addr =
		devm_kzalloc(chip->dev, sizeof(uint32_t) * chip->rgltr_num, GFP_KERNEL);
	if (!chip->wldo_volt_addr) {
		pr_err("chip->wldo_volt_addr malloc ERR!");
		goto init_err;
	}
	for (i = 0; i < chip->rgltr_num; i++) {
		wl28661d_chip_rgltr_desc[i].volt_table =
			devm_kzalloc(chip->dev, sizeof(u32) * 2, GFP_KERNEL);
		if (!wl28661d_chip_rgltr_desc[i].volt_table) {
			pr_err("volt_table malloc ERR!");
			goto init_err;
		}
		if (of_property_read_string_index(chip->dev->of_node,
			"regulator-names", i, (chip->rgltr_name_arr + i))) {
			pr_err("no regulator-names at cnt=%d", i);
			goto init_err;
		}
		np_rgltr = of_get_child_by_name(chip->dev->of_node, *(chip->rgltr_name_arr + i));
		if (!np_rgltr) {
			pr_err("regulators %s node not found\n", *(chip->rgltr_name_arr + i));
			goto init_err;
		}
		initdata = of_get_regulator_init_data(chip->dev, np_rgltr, NULL);
		if (!initdata) {
			pr_err("get regulator %s init_data error !\n", *(chip->rgltr_name_arr + i));
			goto init_err;
		}
		if (of_property_read_u32(np_rgltr, "regulator-id",
			&(wl28661d_chip_rgltr_desc[i].id))) {
			pr_err("No regulator-id found!");
			goto init_err;
		}
		if (of_property_read_u32(np_rgltr, "regulator-min-microvolt",
			(u32 *)(wl28661d_chip_rgltr_desc[i].volt_table))) {
			pr_err("No regulator-min-microvolt found!");
			goto init_err;
		}
		if (of_property_read_u32(np_rgltr, "regulator-max-microvolt",
			(u32 *)(wl28661d_chip_rgltr_desc[i].volt_table + 1))) {
			pr_err("No regulator-max-microvolt found!");
			goto init_err;
		}
		if (of_property_read_u32(np_rgltr, "regulator-regaddr",
			&chip->wldo_volt_addr[i])) {
			pr_err("No regulator-regaddr found!");
			goto init_err;
		}
		uV_regVal = volt2regval(chip->wldo_volt_addr[i],
			wl28661d_chip_rgltr_desc[i].volt_table[0]);
		ret = regmap_write(chip->rmap, chip->wldo_volt_addr[i], uV_regVal);
		if (ret < 0) {
			pr_err("line:%d,regulator %s set default voltage uV_regVal err, ret=%d",
				__LINE__, *(chip->rgltr_name_arr + i), ret);
			goto init_err;
		}

		wl28661d_discharge_init(chip, i);

		rgltr_config.init_data = initdata;
		rgltr_config.dev = chip->dev;
		rgltr_config.of_node = chip->dev->of_node;
		rgltr_config.ena_gpiod = NULL;
		rgltr_config.regmap = chip->rmap;
		rgltr_config.driver_data = (void *)&chip->wldo_volt_addr[i];
		wl28661d_chip_rgltr_desc[i].name = *(chip->rgltr_name_arr + i);

		rgltr_dev = devm_regulator_register(chip->dev, &wl28661d_chip_rgltr_desc[i],
						    &rgltr_config);
		if (IS_ERR(rgltr_dev)) {
			pr_err("regulator %s register error, ret=%ld", *(chip->rgltr_name_arr + i),
				PTR_ERR(rgltr_dev));
			goto init_err;
		}
		pr_debug("regulator %s register successfully", *(chip->rgltr_name_arr + i));
	}

	if (wl2866_debugfs_create(chip))
		pr_err("debugfs create fail");
	pr_info("%s: probe successfully\n", chip->name);
	return 0;
init_err:
	chip = NULL;
	return -EINVAL;
}

void wl28661d_remove(struct i2c_client *client)
{
	struct wl28661d_chip *chip;

	chip = i2c_get_clientdata(client);
	if (IS_ERR_OR_NULL(chip)) {
		pr_err("private data chip is NULL, exit");
		return;
	}
	pr_info("%s: remove", chip->name);
	debugfs_remove(chip->debugfs_dir);
	chip->debugfs_dir = NULL;
	chip = NULL;
}

static const struct i2c_device_id wl28661d_i2c_id_table[] = {
	{"xring,xrisp_wl28661d", 0},
	{} /* NULL terminated */
};
MODULE_DEVICE_TABLE(i2c, wl28661d_i2c_id_table);

static const struct of_device_id wl28661d_i2c_of_match_table[] = {
	{ .compatible = "xring,xrisp_wl28661d" },
	{}
};
MODULE_DEVICE_TABLE(of, wl28661d_i2c_of_match_table);

static struct i2c_driver wl28661d_driver = {
	.id_table = wl28661d_i2c_id_table,
	.probe = wl28661d_probe,
	.remove = wl28661d_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "xring,xrisp_wl28661d",
		.of_match_table = wl28661d_i2c_of_match_table,
		},
};

module_i2c_driver(wl28661d_driver);

MODULE_LICENSE("GPL");
MODULE_SOFTDEP("post: xr_isp");
