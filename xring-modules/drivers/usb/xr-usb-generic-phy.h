/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_GENERIC_PHY_H
#define _XR_USB_GENERIC_PHY_H

#include <asm-generic/errno-base.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/phy/phy.h>
#include <linux/err.h>
#include <linux/seq_file.h>

#define PHY_ORIENT_FLIP		BIT(0)
#define PHY_FLIP_INVERT		BIT(1)

struct plat_phy_submode {
	unsigned int flip : 1;
	unsigned int invert : 1;
	unsigned int reserved31_2 : 30;
} __packed;

union phy_submode {
	unsigned int data;
	struct plat_phy_submode plat_submode;
};

/* hsphy structure */
struct xr_usb_hsphy {
	struct phy *phy;
	int (*set_eye_param)(struct xr_usb_hsphy *hsphy_data,
		int index, const unsigned int *val);
	int (*get_eye_param)(struct xr_usb_hsphy *hsphy_data,
		int index, unsigned int *val);
	void (*param_form)(struct seq_file *s);
	enum phy_mode mode;
};

struct xr_usb_combophy {
	struct phy *phy;
	int (*set_eye_param)(struct xr_usb_combophy *combophy_data,
		int index, const unsigned int *value);
	int (*get_eye_param)(struct xr_usb_combophy *combophy_data,
		int index, unsigned int *val);
	void (*param_form)(struct seq_file *s);
	int (*toggle_mode)(struct xr_usb_combophy *combophy_data,
		unsigned int usbdp_mode);
	int (*lpcheck)(struct xr_usb_combophy *combophy_data);
	enum phy_mode mode;
	union phy_submode submode;
};

#define PHY_DEVM_GET(type) \
static inline struct xr_usb_##type *xr_usb_devm_##type##_get(struct device *dev, const char *name) \
{ \
	struct phy *phy = NULL; \
	if (!dev) \
		return ERR_PTR(-ENODEV); \
	phy = devm_phy_get(dev, name); \
	if (IS_ERR(phy)) \
		return (struct xr_usb_##type *)phy; \
	return phy_get_drvdata(phy); \
}

#define MAKE_PHY_OPS_WRAP_INT(type, func) \
static inline int xr_usb_##type##_##func(struct xr_usb_##type *xr_phy) \
{ \
	if (!xr_phy) \
		return -EINVAL; \
	if (xr_phy->phy->ops->func) \
		return phy_##func(xr_phy->phy); \
	return -EINVAL; \
}

#define MAKE_PHY_OPS_WRAP_SET_MODE_INT(type, func) \
static inline int xr_usb_##type##_role_##func(struct xr_usb_##type *xr_phy, \
		enum phy_mode mode) \
{ \
	if (!xr_phy) \
		return -EINVAL; \
	if (xr_phy->phy->ops->func) \
		return phy_##func(xr_phy->phy, mode); \
	return -EINVAL; \
}

#define MAKE_PHY_OPS_WRAP_SET_MODE_EXT_INT(type, func) \
static inline int xr_usb_##type##_role_##func##_ext(struct xr_usb_##type *xr_phy, \
		enum phy_mode mode, int submode) \
{ \
	if (!xr_phy) \
		return -EINVAL; \
	if (xr_phy->phy->ops->func) \
		return phy_##func##_ext(xr_phy->phy, mode, submode); \
	return -EINVAL; \
}

#define MAKE_SET_EYEPARAM_PHY_OPS(type) \
static inline int xr_usb_##type##_set_eye_param(struct xr_usb_##type *xr_phy, \
		int index, const unsigned int *val) \
{ \
	if (!xr_phy) \
		return -EINVAL; \
	if (xr_phy->set_eye_param) \
		return xr_phy->set_eye_param(xr_phy, index, val); \
	return -EINVAL; \
}

#define MAKE_GET_EYEPARAM_PHY_OPS(type) \
static inline int xr_usb_##type##_get_eye_param(struct xr_usb_##type *xr_phy, \
		int index, unsigned int *val) \
{ \
	if (!xr_phy) \
		return -EINVAL; \
	if (xr_phy->get_eye_param) \
		return xr_phy->get_eye_param(xr_phy, index, val); \
	dev_err(&xr_phy->phy->dev, "fail\n"); \
	return -EINVAL; \
}

#define MAKE_GET_EYEPARAM_FORM(type) \
static inline void xr_usb_##type##_paramform(struct xr_usb_##type *xr_phy, \
		struct seq_file *s) \
{ \
	if (!xr_phy) \
		return; \
	if (xr_phy->param_form) \
		xr_phy->param_form(s); \
	else \
		seq_puts(s, "fail\n"); \
}

static inline int xr_usb_combophy_mode_toggle(struct xr_usb_combophy *xr_phy,
		unsigned int usbdp_mode)
{
	if (!xr_phy)
		return -EINVAL;

	if (xr_phy->toggle_mode)
		return xr_phy->toggle_mode(xr_phy, usbdp_mode);

	dev_err(&xr_phy->phy->dev, "fail\n");
	return -EINVAL;
}

static inline int xr_usb_combophy_lpcheck(struct xr_usb_combophy *xr_phy)
{
	if (!xr_phy)
		return -EINVAL;

	if (xr_phy->lpcheck)
		return xr_phy->lpcheck(xr_phy);

	dev_err(&xr_phy->phy->dev, "fail\n");
	return -EINVAL;
}

PHY_DEVM_GET(hsphy);
MAKE_PHY_OPS_WRAP_INT(hsphy, init);
MAKE_PHY_OPS_WRAP_INT(hsphy, exit);
MAKE_PHY_OPS_WRAP_INT(hsphy, power_on);
MAKE_PHY_OPS_WRAP_INT(hsphy, power_off);
MAKE_PHY_OPS_WRAP_SET_MODE_INT(hsphy, set_mode);
MAKE_SET_EYEPARAM_PHY_OPS(hsphy);
MAKE_GET_EYEPARAM_PHY_OPS(hsphy);
MAKE_GET_EYEPARAM_FORM(hsphy);

PHY_DEVM_GET(combophy);
MAKE_PHY_OPS_WRAP_INT(combophy, init);
MAKE_PHY_OPS_WRAP_INT(combophy, exit);
MAKE_PHY_OPS_WRAP_INT(combophy, power_on);
MAKE_PHY_OPS_WRAP_INT(combophy, power_off);
MAKE_PHY_OPS_WRAP_SET_MODE_EXT_INT(combophy, set_mode);
MAKE_SET_EYEPARAM_PHY_OPS(combophy);
MAKE_GET_EYEPARAM_PHY_OPS(combophy);
MAKE_GET_EYEPARAM_FORM(combophy);

int __init usb_hsphy_init(void);
void usb_hsphy_exit(void);
int __init usb_combophy_init(void);
void usb_combophy_exit(void);

#endif /* _XR_USB_GENERIC_PHY_H */
