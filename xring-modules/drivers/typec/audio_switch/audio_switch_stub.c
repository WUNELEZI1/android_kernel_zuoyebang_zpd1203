// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/usb.h>

#define AUDIO_SWITCH_MODULE_NAME "audio_switch_stub"

enum audio_switch_mode {
	AUDIO_SW_MODE_NOT_CONNECTED = 0,
	AUDIO_SW_MODE_USB,
	AUDIO_SW_MODE_USBDP,
	AUDIO_SW_MODE_HEADSET,
	AUDIO_SW_MODE_MAX,
};

enum audio_switch_orientation {
	AUDIO_SW_ORIEN_NORMAL = 0,
	AUDIO_SW_ORIEN_FLIP,
	AUDIO_SW_ORIEN_MAX,
};

struct audio_switch_stub_priv {
	struct device *dev;
	struct regmap *regmap;
	struct dentry *debug_root;
	u64 mode;
	u64 orientation;
};

struct audio_switch_reg_config {
	u32 addr;
	u32 val;
	u32 usleep_time;
};

#define AUDIO_SW_REG_ID		     0x00
#define AUDIO_SW_REG_SWITCH_SETTINGS 0x04
#define AUDIO_SW_REG_SWITCH_CONTROL  0x05
#define AUDIO_SW_REG_SWITCH_STATUS0  0x06
#define AUDIO_SW_REG_SWITCH_STATUS1  0x07
#define AUDIO_SW_REG_SLOW_L	     0x08
#define AUDIO_SW_REG_SLOW_R	     0x09
#define AUDIO_SW_REG_SLOW_MIC	     0x0A
#define AUDIO_SW_REG_SLOW_SENSE	     0x0B
#define AUDIO_SW_REG_SLOW_GND	     0x0C
#define AUDIO_SW_REG_DELAY_L_R	     0x0D
#define AUDIO_SW_REG_DELAY_L_MIC     0x0E
#define AUDIO_SW_REG_DELAY_L_SENSE   0x0F
#define AUDIO_SW_REG_DELAY_L_AGND    0x10
#define AUDIO_SW_REG_FUNCTION_ENABLE 0x12
#define AUDIO_SW_REG_JACK_STATUS     0x17
#define AUDIO_SW_REG_DETECTION_INT   0x18
#define AUDIO_SW_REG_RESET	     0x1E
#define AUDIO_SW_REG_CURRENT_SOURCE  0x1F
#define AUDIO_SW_REG_INVALID	     0xFFFFFFFF

#ifdef CONFIG_DEBUG_FS
static const struct audio_switch_reg_config audio_sw_config_disconnect[] = {
	{ AUDIO_SW_REG_RESET, 0x1, 1000 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x18, 0 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x98, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config audio_sw_config_usb[] = {
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x80, 0 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x18, 50 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x98, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config audio_sw_config_normal_usbdp[] = {
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x80, 0 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x18, 50 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0xF8, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config audio_sw_config_normal_headset[] = {
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x80, 0 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x00, 50 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x9F, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config audio_sw_config_flip_usbdp[] = {
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x80, 0 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x78, 50 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0xF8, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config audio_sw_config_flip_headset[] = {
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x80, 0 },
	{ AUDIO_SW_REG_SWITCH_CONTROL, 0x07, 50 },
	{ AUDIO_SW_REG_SWITCH_SETTINGS, 0x9F, 0 },
	{ AUDIO_SW_REG_INVALID, 0, 0 } /* terminate entry */
};

static const struct audio_switch_reg_config *audio_sw_normal_configs[] = {
	[AUDIO_SW_MODE_NOT_CONNECTED] = audio_sw_config_disconnect,
	[AUDIO_SW_MODE_USB] = audio_sw_config_usb,
	[AUDIO_SW_MODE_USBDP] = audio_sw_config_normal_usbdp,
	[AUDIO_SW_MODE_HEADSET] = audio_sw_config_normal_headset,
};

static const struct audio_switch_reg_config *audio_sw_flip_configs[] = {
	[AUDIO_SW_MODE_NOT_CONNECTED] = audio_sw_config_disconnect,
	[AUDIO_SW_MODE_USB] = audio_sw_config_usb,
	[AUDIO_SW_MODE_USBDP] = audio_sw_config_flip_usbdp,
	[AUDIO_SW_MODE_HEADSET] = audio_sw_config_flip_headset,
};

static const struct audio_switch_reg_config **audio_sw_mode_configs[] = {
	[AUDIO_SW_ORIEN_NORMAL] = audio_sw_normal_configs,
	[AUDIO_SW_ORIEN_FLIP] = audio_sw_flip_configs,
};

static int audio_sw_config_regs(struct audio_switch_stub_priv *priv)
{
	int ret = -EINVAL;
	const struct audio_switch_reg_config *config = NULL;

	config = audio_sw_mode_configs[priv->orientation][priv->mode];

	while (config->addr != AUDIO_SW_REG_INVALID) {
		ret = regmap_write(priv->regmap, config->addr, config->val);
		if (ret) {
			dev_err(priv->dev, "failed to config %x to %x",
				config->addr, config->val);
			break;
		}
		if (config->usleep_time)
			usleep_range(config->usleep_time,
				     config->usleep_time + 5);
		config++;
	}

	return ret;
}

static int audio_sw_mode_set(void *data, u64 val)
{
	struct audio_switch_stub_priv *priv = data;

	if (val >= AUDIO_SW_MODE_MAX) {
		dev_err(priv->dev, "invalid mode number %llu\n", val);
		return -EINVAL;
	}

	priv->mode = val;

	return audio_sw_config_regs(priv);
}

static int audio_sw_mode_get(void *data, u64 *val)
{
	struct audio_switch_stub_priv *priv = data;

	*val = priv->mode;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_mode_fops, audio_sw_mode_get,
			 audio_sw_mode_set, "%llu\n");

static int audio_sw_orientation_set(void *data, u64 val)
{
	struct audio_switch_stub_priv *priv = data;

	if (val >= AUDIO_SW_ORIEN_MAX) {
		dev_err(priv->dev, "invalid orientation number %llu\n", val);
		return -EINVAL;
	}

	priv->orientation = val;

	return audio_sw_config_regs(priv);
}

static int audio_sw_orientation_get(void *data, u64 *val)
{
	struct audio_switch_stub_priv *priv = data;

	*val = priv->orientation;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(audio_sw_orientation_fops, audio_sw_orientation_get,
			 audio_sw_orientation_set, "%llu\n");

static void audio_switch_debugfs_create(struct audio_switch_stub_priv *priv)
{
	struct dentry *root = NULL;

	root = debugfs_create_dir(AUDIO_SWITCH_MODULE_NAME, usb_debug_root);
	if (!root) {
		dev_err(priv->dev, "failed to create dir\n");
		return;
	}

	debugfs_create_file("mode", 0644, root, priv, &audio_sw_mode_fops);
	debugfs_create_file("orientation", 0644, root, priv,
			    &audio_sw_orientation_fops);
	priv->debug_root = root;
}

static void audio_switch_debugfs_destroy(struct audio_switch_stub_priv *priv)
{
	debugfs_remove(priv->debug_root);
	priv->debug_root = NULL;
}
#else
static inline void audio_switch_debugfs_create(struct audio_switch_stub_priv *priv)
{
}
static inline void audio_switch_debugfs_destroy(struct audio_switch_stub_priv *priv)
{
}
#endif

static const struct regmap_config audio_switch_stub_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = AUDIO_SW_REG_CURRENT_SOURCE,
};

static int audio_switch_stub_probe(struct i2c_client *client)
{
	struct audio_switch_stub_priv *priv;

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = &client->dev;
	priv->mode = AUDIO_SW_MODE_USB;
	priv->orientation = AUDIO_SW_ORIEN_NORMAL;

	priv->regmap =
		devm_regmap_init_i2c(client, &audio_switch_stub_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	audio_switch_debugfs_create(priv);
	i2c_set_clientdata(client, priv);

	return 0;
}

static void audio_switch_stub_remove(struct i2c_client *client)
{
	struct audio_switch_stub_priv *priv;

	priv = i2c_get_clientdata(client);
	audio_switch_debugfs_destroy(priv);
	i2c_set_clientdata(client, NULL);
}

static const struct of_device_id audio_switch_stub_dt_match[] = {
	{
		.compatible = "audio_switch_stub,HL5281",
	},
	{}
};
MODULE_DEVICE_TABLE(of, audio_switch_stub_dt_match);

static const struct i2c_device_id audio_switch_stub_id[] = {
	{ "HL5281", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, audio_switch_stub_id);

static struct i2c_driver audio_switch_stub_driver = {
	.driver		= {
		.name	= AUDIO_SWITCH_MODULE_NAME,
		.of_match_table = audio_switch_stub_dt_match,
	},
	.probe		= audio_switch_stub_probe,
	.remove		= audio_switch_stub_remove,
	.id_table = audio_switch_stub_id,
};
module_i2c_driver(audio_switch_stub_driver);

MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_DESCRIPTION("Audio Switch Driver for SOC Verify");
MODULE_LICENSE("GPL v2");
