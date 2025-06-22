// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/of_gpio.h>
#include "dpu_backlight.h"
#include "dsi_panel.h"
#include "dsi_host.h"
#include "dpu_log.h"
#include "backlight_ktz8866.h"

#define NORMAL_MAX_DVB 1630

static struct list_head ktz8866_dev_list;
static struct mutex ktz8866_dev_list_mutex;

static struct regmap_config ktz8866_regmap_i2c = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int ktz8866_write(struct ktz8866_led *ktz,
		unsigned int reg, unsigned int data)
{
	int ret;

	if (ktz) {
		ret = regmap_write(ktz->regmap, reg, data);
		DSI_DEBUG("i2c write, reg:0x%x, val:0x%x", reg, data);
		if (ret < 0)
			DSI_ERROR("ktz write failed to access reg = 0x%x, data = 0x%x\n", reg, data);
	} else {
		DSI_ERROR("write missing ktz8866\n");
		ret = -EINVAL;
	}
	return ret;
}

static int ktz8866_read(struct ktz8866_led *ktz,
		unsigned int reg, unsigned int *data)
{
	int ret;

	if (ktz) {
		ret = regmap_read(ktz->regmap, reg, data);
		DSI_DEBUG("i2c write, reg:0x%x, val:0x%x", reg, *data);
		if (ret < 0)
			DSI_ERROR("ktz read failed to access reg = 0x%x, data = 0x%x\n", reg, *data);
	} else {
		DSI_ERROR("read missing ktz8866\n");
		ret = -EINVAL;
	}
	return ret;
}

static int ktz_update_status(struct ktz8866_led *ktz, unsigned int level)
{
	int exponential_bl = level;
	int brightness = 0;
	unsigned int v[2];

	if (!ktz) {
		DSI_ERROR("ktz8866 not exit, return!\n ");
		return -EINVAL;
	}

	if (exponential_bl <= BL_LEVEL_MAX) {
		exponential_bl = (exponential_bl * NORMAL_MAX_DVB) / 2047;
	} else if (exponential_bl <= BL_LEVEL_MAX_HBM) {
		exponential_bl = ((exponential_bl - 2048) * (2047 - NORMAL_MAX_DVB)) / 2047 + NORMAL_MAX_DVB;
	} else {
		DSI_ERROR("ktz8866 backlight out of 4095 too large!!!\n");
		return -EINVAL;
	}

	brightness = mi_bl_level_remap[exponential_bl];
	if (brightness < 0 || brightness > BL_LEVEL_MAX) {
		DSI_ERROR("ktz8866 backlight out of 2047 too large!!!\n");
		return -EINVAL;
	}

	if (!ktz->ktz8866_status && brightness > 0) {
		ktz8866_write(ktz, KTZ8866_DISP_BL_ENABLE, 0x7f);
		ktz->ktz8866_status = 1;
		DSI_DEBUG("ktz8866 backlight enable,dimming close");
	} else if (brightness == 0) {
		ktz8866_write(ktz, KTZ8866_DISP_BL_ENABLE, 0x3f);
		ktz->ktz8866_status = 0;
		usleep_range((10 * 1000), (10 * 1000) + 10);
		DSI_DEBUG("ktz8866 backlight disable,dimming close");
	}
	v[0] = (brightness >> 3) & 0xff;
	v[1] = brightness & 0x7;
	DSI_DEBUG("ktz8866 get level: %d, exponential_bl: %d, reg: %d ,MSB: 0x%02x, LSB: 0x%02x\n",
			level, exponential_bl, brightness, v[0], v[1]);
	ktz8866_write(ktz, KTZ8866_DISP_BB_LSB, v[1]);
	ktz8866_write(ktz, KTZ8866_DISP_BB_MSB, v[0]);
	ktz->level = brightness;

	return 0;
}

static void ktz_power_on(struct ktz8866_led *ktz)
{
	if (!ktz)
		DSI_ERROR("ktz8866 not exit!!\n");

	DSI_DEBUG("%s!!\n", __func__);
	ktz8866_write(ktz, KTZ8866_DISP_BC1, 0x52);
	ktz8866_write(ktz, KTZ8866_DISP_FULL_CURRENT, 0x91);
	ktz8866_write(ktz, KTZ8866_DISP_BC2, 0xCD);
	/* set the voltage of vsp/vsn to 6v */
	ktz8866_write(ktz, KTZ8866_DISP_BIAS_VPOS, 0x28);
	ktz8866_write(ktz, KTZ8866_DISP_BIAS_VNEG, 0x28);
	ktz8866_write(ktz, KTZ8866_DISP_BIAS_CONF1, 0x9F);
	/* set bl as 0 */
	ktz8866_write(ktz, KTZ8866_DISP_BB_LSB, 0);
	ktz8866_write(ktz, KTZ8866_DISP_BB_MSB, 0);
	/* enable the bl device */
	ktz8866_write(ktz, KTZ8866_DISP_BL_ENABLE, 0x7F);
}

static void ktz_power_on_except_vsp_vsn(struct ktz8866_led *ktz)
{
	if (!ktz)
		DSI_ERROR("ktz8866 not exit!!\n");

	DSI_DEBUG("%s!!\n", __func__);
	ktz8866_write(ktz, KTZ8866_DISP_BC1, 0x42);
	ktz8866_write(ktz, KTZ8866_DISP_FULL_CURRENT, 0xF9);
	ktz8866_write(ktz, KTZ8866_DISP_BC2, 0xCD);
	/* set bl as 0 */
	ktz8866_write(ktz, KTZ8866_DISP_BB_LSB, 0);
	ktz8866_write(ktz, KTZ8866_DISP_BB_MSB, 0);
	/* enable the bl device */
	ktz8866_write(ktz, KTZ8866_DISP_BL_ENABLE, 0x7F);
}

static void ktz_power_off(struct ktz8866_led *ktz)
{
	if (!ktz)
		DSI_ERROR("ktz8866 not exit!!\n ");

	DSI_DEBUG("%s!!\n", __func__);
	ktz8866_write(ktz, KTZ8866_DISP_BL_ENABLE, 0x0);
}

static int ktz_get_brightness(struct ktz8866_led *ktz,
		unsigned int reg, unsigned int *data)
{
	return ktz8866_read(ktz, reg, data);
}

static const struct ktz_ops ops = {
	.power_on = ktz_power_on,
	.power_on_except_vsp_vsn = ktz_power_on_except_vsp_vsn,
	.power_off = ktz_power_off,
	.update_status	= ktz_update_status,
	.get_brightness	= ktz_get_brightness,
};

int ktz8866_backlight_update_status(unsigned int level)
{
	int ret = 0;
	static struct ktz8866_led *ktz;
	unsigned int read;

	DSI_DEBUG("ktz8866 backlight level:%d", level);

	mutex_lock(&ktz8866_dev_list_mutex);
	list_for_each_entry(ktz, &ktz8866_dev_list, entry) {
		ret = ktz->ops->update_status(ktz, level);
		if (ret)
			DSI_ERROR("failed to update ktz8866 bl status\n");
		ret = ktz->ops->get_brightness(ktz, KTZ8866_DISP_BB_MSB, &read);
		if (ret)
			DSI_ERROR("failed to get ktz8866 bl value\n");
	}
	mutex_unlock(&ktz8866_dev_list_mutex);

	return ret;
}

void ktz8866_power_on(void)
{
	static struct ktz8866_led *ktz;

	mutex_lock(&ktz8866_dev_list_mutex);
	list_for_each_entry(ktz, &ktz8866_dev_list, entry) {
		ktz->ops->power_on(ktz);
	}
	mutex_unlock(&ktz8866_dev_list_mutex);
}

void ktz8866_power_on_except_vsp_vsn(void)
{
	static struct ktz8866_led *ktz;

	mutex_lock(&ktz8866_dev_list_mutex);
	list_for_each_entry(ktz, &ktz8866_dev_list, entry) {
		ktz->ops->power_on_except_vsp_vsn(ktz);
	}
	mutex_unlock(&ktz8866_dev_list_mutex);
}

void ktz8866_power_off(void)
{
	static struct ktz8866_led *ktz;

	mutex_lock(&ktz8866_dev_list_mutex);
	list_for_each_entry(ktz, &ktz8866_dev_list, entry) {
		ktz->ops->power_off(ktz);
	}
	mutex_unlock(&ktz8866_dev_list_mutex);
}

static int ktz8866_probe(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	struct ktz8866_led *pdata;
	int ret = 0;

	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		DSI_ERROR("ktz8866 I2C adapter doesn't support I2C_FUNC_SMBUS_BYTE\n");
		return -EIO;
	}

	pdata = devm_kzalloc(dev, sizeof(struct ktz8866_led), GFP_KERNEL);
	if (!pdata) {
		DSI_ERROR("failed: out of memory\n");
		return -ENOMEM;
	}

	pdata->regmap = devm_regmap_init_i2c(i2c, &ktz8866_regmap_i2c);
	if (IS_ERR(pdata->regmap)) {
		ret = PTR_ERR(pdata->regmap);
		DSI_ERROR("init regmap failed\n");
		return ret;
	}

	pdata->ops = &ops;
	list_add(&pdata->entry, &ktz8866_dev_list);

	return ret;
}

static void ktz8866_remove(struct i2c_client *i2c)
{
}

static const struct of_device_id ktz8866_match_table[] = {
	{ .compatible = "ktz,ktz8866",},
	{ },
};

static struct i2c_driver ktz8866_driver = {
	.driver = {
		.name = "ktz8866",
		.of_match_table = ktz8866_match_table,
	},
	.probe = ktz8866_probe,
	.remove = ktz8866_remove,
};

int backlight_ktz8866_init(void)
{
	int ret;

	INIT_LIST_HEAD(&ktz8866_dev_list);
	mutex_init(&ktz8866_dev_list_mutex);

	ret = i2c_add_driver(&ktz8866_driver);

	DSI_DEBUG("i2c add driver, ret %d\n", ret);

	return ret;
}
