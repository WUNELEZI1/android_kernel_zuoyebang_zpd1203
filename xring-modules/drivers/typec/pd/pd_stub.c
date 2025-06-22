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
#include <linux/usb/pd.h>
#include <linux/usb/role.h>
#include <linux/usb/typec.h>

#define PD_MODULE_NAME "pd_stub"

enum pd_vbus_state {
	PD_VBUS_STATE_OFF = 0,
	PD_VBUS_STATE_ON,
	PD_VBUS_STATE_MAX,
};

struct pd_stub_priv {
	struct device *dev;
	struct regmap *regmap;
	struct dentry *debug_root;
	u64 vbus_state;
	struct typec_port *port;
	struct typec_partner *partner;
	struct usb_pd_identity partner_ident;
	struct typec_partner_desc partner_desc;
};

/* Registers */
#define PD_REG_DEVICE_ID      0x00
#define PD_REG_HK_CTRL1	      0x7
#define PD_REG_HK_CTRL2	      0x8
#define PD_REG_PROTECTION_DIS 0x3A
#define PD_REG_VBOOST_CTRL    0x39
#define PD_REG_CHR_CTRL1      0x3C
#define PD_REG_CHG_INT_STAT2  0x42
#define PD_REG_MAX	      0x9D
#define PD_REG_INVALID	      0xFFFFFFFF

/* Bits of PD_REG_HK_CTRL1 */
#define HK_CTRL1_WD_TIMEOUT_MASK 0x7U
#define HK_CTRL1_WD_TIMEOUT(n)	 ((n)&HK_CTRL1_WD_TIMEOUT_MASK)
#define HK_CTRL1_ACDRV_MANUAL_EN BIT(4)
#define HK_CTRL1_ACDRV_EN	 BIT(5)
/* Bits of PD_REG_HK_CTRL2 */
#define HK_CTRL2_TSBUS_TSBAT_FLT_DIS BIT(0)
#define HK_CTRL2_TSHUT_DIS	     BIT(1)
#define HK_CTRL2_CID_EN		     BIT(3)
#define HK_CTRL2_PMID_PD_EN	     BIT(4)
#define HK_CTRL2_VBUS_PD	     BIT(6)
/* Bits of PD_REG_PROTECTION_DIS */
#define PROTECTION_DIS_TSBAT_JEITA_DIS BIT(3)
/* Bits of PD_REG_VBOOST_CTRL */
#define IBOOST_LIMIT_MASK  0x7U
#define IBOOST_2_5_V       0x5U
/* Bits of PD_REG_CHR_CTRL1 */
#define CHR_CTRL1_CHG_EN   BIT(0)
#define CHR_CTRL1_BOOST_EN BIT(1)
#define CHR_CTRL1_QB_EN	   BIT(2)
/* Bits of PD_REG_CHG_INT_STAT2 */
#define CHG_INT_STAT2_BOOST_OK_STAT BIT(4)

#ifdef CONFIG_DEBUG_FS
static int pd_vbus_set(void *data, u64 val)
{
	struct pd_stub_priv *priv = data;
	int timeout = 1000;
	u32 reg = 0;

	if (val >= PD_VBUS_STATE_MAX) {
		dev_err(priv->dev, "invalid state number %llu\n", val);
		return -EINVAL;
	}

	priv->vbus_state = val;

	if (priv->vbus_state) {
		/* Disable protection */
		regmap_read(priv->regmap, PD_REG_HK_CTRL2, &reg);
		reg |= HK_CTRL2_TSBUS_TSBAT_FLT_DIS | HK_CTRL2_TSHUT_DIS |
		       HK_CTRL2_CID_EN;
		reg &= ~(HK_CTRL2_PMID_PD_EN | HK_CTRL2_VBUS_PD);
		regmap_write(priv->regmap, PD_REG_HK_CTRL2, reg);
		regmap_read(priv->regmap, PD_REG_HK_CTRL2, &reg);
		dev_info(priv->dev, "read PD_REG_HK_CTRL2 0x%x\n", reg);

		regmap_read(priv->regmap, PD_REG_PROTECTION_DIS, &reg);
		reg |= PROTECTION_DIS_TSBAT_JEITA_DIS;
		regmap_write(priv->regmap, PD_REG_PROTECTION_DIS, reg);

		/* Disable watchdog timer */
		regmap_read(priv->regmap, PD_REG_HK_CTRL1, &reg);
		reg &= ~HK_CTRL1_WD_TIMEOUT_MASK;
		regmap_write(priv->regmap, PD_REG_HK_CTRL1, reg);

		/* iBoost limit */
		regmap_read(priv->regmap, PD_REG_VBOOST_CTRL, &reg);
		reg &= ~IBOOST_LIMIT_MASK;
		reg |= IBOOST_2_5_V;
		regmap_write(priv->regmap, PD_REG_VBOOST_CTRL, reg);

		/* Enable Boost */
		regmap_read(priv->regmap, PD_REG_CHR_CTRL1, &reg);
		reg &= ~CHR_CTRL1_CHG_EN;
		regmap_write(priv->regmap, PD_REG_CHR_CTRL1, reg);
		usleep_range(1000, 2000);
		reg |= CHR_CTRL1_BOOST_EN;
		regmap_write(priv->regmap, PD_REG_CHR_CTRL1, reg);

		regmap_read(priv->regmap, PD_REG_CHR_CTRL1, &reg);
		dev_info(priv->dev, "read PD_REG_CHR_CTRL1 0x%x\n", reg);
		while (timeout--) {
			regmap_read(priv->regmap, PD_REG_CHG_INT_STAT2, &reg);
			if (reg & CHG_INT_STAT2_BOOST_OK_STAT)
				break;
			usleep_range(2000, 3000);
		}
		if (timeout <= 0) {
			dev_err(priv->dev, "wait BOOST_OK_STATE timeout 0x%x\n",
				reg);
			return -ETIMEDOUT;
		}

		/* Enable QB */
		regmap_read(priv->regmap, PD_REG_CHR_CTRL1, &reg);
		reg |= CHR_CTRL1_QB_EN;
		regmap_write(priv->regmap, PD_REG_CHR_CTRL1, reg);
		usleep_range(2000, 3000);
		regmap_read(priv->regmap, PD_REG_HK_CTRL1, &reg);
		reg |= HK_CTRL1_ACDRV_MANUAL_EN | HK_CTRL1_ACDRV_EN;
		regmap_write(priv->regmap, PD_REG_HK_CTRL1, reg);
	} else {
		/* Enable Protection */
		regmap_read(priv->regmap, PD_REG_HK_CTRL2, &reg);
		reg &= ~(HK_CTRL2_TSBUS_TSBAT_FLT_DIS | HK_CTRL2_TSHUT_DIS |
			 HK_CTRL2_CID_EN);
		regmap_write(priv->regmap, PD_REG_HK_CTRL2, reg);

		regmap_read(priv->regmap, PD_REG_PROTECTION_DIS, &reg);
		reg &= ~PROTECTION_DIS_TSBAT_JEITA_DIS;
		regmap_write(priv->regmap, PD_REG_PROTECTION_DIS, reg);

		/* Disable Boost and Enable Charger */
		regmap_write(priv->regmap, PD_REG_CHR_CTRL1, CHR_CTRL1_CHG_EN);

		regmap_read(priv->regmap, PD_REG_HK_CTRL2, &reg);
		reg |= HK_CTRL2_PMID_PD_EN | HK_CTRL2_VBUS_PD;
		regmap_write(priv->regmap, PD_REG_HK_CTRL2, reg);
		usleep_range(2000, 3000);
		regmap_read(priv->regmap, PD_REG_HK_CTRL1, &reg);
		reg &= ~HK_CTRL1_ACDRV_MANUAL_EN;
		regmap_write(priv->regmap, PD_REG_HK_CTRL1, reg);
	}

	return 0;
}

static int pd_vbus_get(void *data, u64 *val)
{
	struct pd_stub_priv *priv = data;

	*val = priv->vbus_state;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(pd_vbus_fops, pd_vbus_get, pd_vbus_set, "%llu\n");

static void pd_debugfs_create(struct pd_stub_priv *priv)
{
	struct dentry *root = NULL;

	root = debugfs_create_dir(PD_MODULE_NAME, usb_debug_root);
	if (!root) {
		dev_err(priv->dev, "failed to create dir\n");
		return;
	}

	debugfs_create_file("vbus", 0644, root, priv, &pd_vbus_fops);
	priv->debug_root = root;
}

static void pd_debugfs_destroy(struct pd_stub_priv *priv)
{
	debugfs_remove(priv->debug_root);
	priv->debug_root = NULL;
}
#else
static inline void pd_debugfs_create(struct pd_stub_priv *priv)
{
}
static inline void pd_debugfs_destroy(struct pd_stub_priv *priv)
{
}
#endif

static int pd_stub_dr_set(struct typec_port *port, enum typec_data_role role)
{
	struct pd_stub_priv *priv = typec_get_drvdata(port);

	typec_set_data_role(priv->port, role);

	return 0;
}

static const struct typec_operations pd_stub_port_ops = {
	.dr_set = pd_stub_dr_set,
};

static int pd_stub_typec_init(struct pd_stub_priv *priv)
{
	struct fwnode_handle *fwnode;
	struct typec_capability typec_cap = { };
	int ret;

	fwnode = device_get_named_child_node(priv->dev, "connector");
	if (!fwnode)
		return -ENODEV;

	typec_cap.revision = USB_TYPEC_REV_1_2;
	typec_cap.pd_revision = 0x300;	/* USB-PD spec release 3.0 */
	typec_cap.type = TYPEC_PORT_SNK;
	typec_cap.data = TYPEC_PORT_DRD;
	typec_cap.ops = &pd_stub_port_ops;
	typec_cap.fwnode = fwnode;
	typec_cap.driver_data = priv;
	typec_cap.accessory[0] = TYPEC_ACCESSORY_NONE;

	priv->port = typec_register_port(priv->dev, &typec_cap);
	if (IS_ERR(priv->port)) {
		ret = PTR_ERR(priv->port);
		dev_err(priv->dev, "Failed to register typec port (%d)\n", ret);
		return ret;
	}

	priv->partner_desc.identity = &priv->partner_ident;
	priv->partner_desc.usb_pd = true;
	priv->partner_desc.pd_revision = 0x300;
	priv->partner_desc.accessory = TYPEC_ACCESSORY_NONE;
	priv->partner = typec_register_partner(priv->port, &priv->partner_desc);

	typec_set_pwr_opmode(priv->port, TYPEC_PWR_MODE_USB);
	typec_set_data_role(priv->port, TYPEC_DEVICE);

	return 0;
}

static void pd_stub_typec_exit(struct pd_stub_priv *priv)
{
	typec_unregister_partner(priv->partner);
	priv->partner = NULL;
	typec_unregister_port(priv->port);
	priv->port = NULL;
}

static const struct regmap_config pd_stub_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = PD_REG_MAX,
};

static int pd_stub_probe(struct i2c_client *client)
{
	struct pd_stub_priv *priv;

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = &client->dev;

	priv->regmap = devm_regmap_init_i2c(client, &pd_stub_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	pd_debugfs_create(priv);
	i2c_set_clientdata(client, priv);

	return pd_stub_typec_init(priv);
}

static void pd_stub_remove(struct i2c_client *client)
{
	struct pd_stub_priv *priv;

	priv = i2c_get_clientdata(client);
	pd_debugfs_destroy(priv);
	pd_stub_typec_exit(priv);
	i2c_set_clientdata(client, NULL);
}

static const struct of_device_id pd_stub_dt_match[] = {
	{
		.compatible = "pd_stub,SC6601",
	},
	{}
};
MODULE_DEVICE_TABLE(of, pd_stub_dt_match);

static const struct i2c_device_id pd_stub_id[] = {
	{ "SC6601", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, pd_stub_id);

static struct i2c_driver pd_stub_driver = {
	.driver		= {
		.name	= PD_MODULE_NAME,
		.of_match_table = pd_stub_dt_match,
	},
	.probe		= pd_stub_probe,
	.remove		= pd_stub_remove,
	.id_table = pd_stub_id,
};
module_i2c_driver(pd_stub_driver);

MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_DESCRIPTION("PD&Charger Driver for SOC Verify");
MODULE_LICENSE("GPL v2");
