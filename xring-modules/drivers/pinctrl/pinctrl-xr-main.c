// SPDX-License-Identifier: GPL-2.0
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

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/slab.h>
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <linux/log2.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <soc/xring/xr_hwspinlock.h>

#include "xr_gpio_debug.h"

// from kernel/driver/gpio
#include "gpiolib.h"
#include "gpiolib-acpi.h"

// from kernel/driver/pinctrl
#include "core.h"
#include "pinconf.h"
#include "pinctrl-utils.h"
#include "pinctrl-xr.h"

#include "dt-bindings/xring/platform-specific/DW_apb_gpio_header.h"
#include "dt-bindings/xring/platform-specific/ioc_reg.h"
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>
#include "xsp_ffa.h"

#define BITMASK_LO_16BIT	0xffff
#define BITMASK_HI_16BIT	0xffff0000
#define BITMASK_DATABITS	16
#define BITMASK_OFFSET		16
#define INVALID_LOCKID		0xffffffff

#define XR_IOC_NO_PULL		0
#define XR_IOC_PULL_DOWN	BIT(0)
#define XR_IOC_PULL_UP		BIT(1)
#define XR_IOC_BUS_KEEPER	(BIT(0) | BIT(1))

#define XR_DEBUG_FPGA_VERSION	1

struct xr_pinctrl;
u32 xr_gpio_doze_flag;
EXPORT_SYMBOL(xr_gpio_doze_flag);

static int xr_gpio_lock_get(struct dwapb_gpio *dw_gc, unsigned long *flags)
{
	int lock_id;
	uint32_t cur_mid_r;
	int ret = 0;
	int i;

	if (dw_gc->hwlock) {
		/* Request hwspinlock may fail. Add retries. */
		for (i = 0; i < HWSPINLOCK_RETRY_TIMES; i++) {
			ret = hwspin_lock_timeout_irqsave(dw_gc->hwlock, GPIO_HW_LOCK_TIMEOUT_MS,
								flags);
			if (ret) {
				lock_id = hwspin_lock_get_id(dw_gc->hwlock);
				if (xr_get_hwspinlock_mid(lock_id, &cur_mid_r)) {
					dev_err(dw_gc->dev, "Get hwspinlock(%d) mid failed\n",
						lock_id);
					return -EINVAL;
				}
				dev_err(dw_gc->dev,
					"Get hwspinlock(%d) timeout(%d ms) (%d/%d), used by %u\n",
					lock_id, GPIO_HW_LOCK_TIMEOUT_MS, i + 1,
					HWSPINLOCK_RETRY_TIMES, cur_mid_r);
			} else {
				break;
			}
		}
	} else {
		raw_spin_lock_irqsave(&dw_gc->gc.bgpio_lock, *flags);
	}

	return ret;
}

static void xr_gpio_lock_put(struct dwapb_gpio *dw_gc, unsigned long *flags)
{
	if (dw_gc->hwlock)
		hwspin_unlock_irqrestore(dw_gc->hwlock, flags);
	else
		raw_spin_unlock_irqrestore(&dw_gc->gc.bgpio_lock, *flags);
}

int xr_readl_cfg(struct xr_pinctrl *pctrl,
			const struct xr_pingroup *g, unsigned int group, u32 *val)
{
	struct xsp_ffa_msg msg;
	int ret;

	if (pctrl->fpga_debug == XR_DEBUG_FPGA_VERSION)
		return -EPERM;

	if (g->cfg_reg >= pctrl->ioctl_non_secure_size) {
		memset(&msg, 0, sizeof(msg));
		msg.fid = FID_IOC_CONFIG_GET;
		msg.data0 = group;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0) {
			dev_err(pctrl->dev, "ffa message not arrive in respondent, ret = %d\n", ret);
			return ret;
		}
		if (msg.ret != 0) {
			dev_err(pctrl->dev, "pinctrl read sec_gpio cfg_smc fail!\n");
			return msg.ret;
		}
		*val = msg.data1;
	} else {
		*val = readl(pctrl->regs[g->cfg_tile] + g->cfg_reg);
	}

	return 0;
}

static void xr_writel_cfg(u32 val, struct xr_pinctrl *pctrl,
				const struct xr_pingroup *g, unsigned int group)
{
	struct xsp_ffa_msg msg;
	int ret;

	if (pctrl->fpga_debug == XR_DEBUG_FPGA_VERSION)
		return;

	if (g->cfg_reg >= pctrl->ioctl_non_secure_size) {
		memset(&msg, 0, sizeof(msg));
		msg.fid = FID_IOC_CONFIG_SET;
		msg.data0 = group;
		msg.data1 = val;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0)
			dev_err(pctrl->dev, "ffa message not arrive in respondent, ret = %d\n", ret);
		if (msg.ret != 0)
			dev_err(pctrl->dev, "pinctrl write sec_gpio cfg_smc fail!\n");
	} else {
		writel(val, pctrl->regs[g->cfg_tile] + g->cfg_reg);
	}
}

int xr_readl_mux(struct xr_pinctrl *pctrl,
			const struct xr_pingroup *g, unsigned int group, u32 *val)
{
	struct xsp_ffa_msg msg;
	int ret;

	if (pctrl->fpga_debug == XR_DEBUG_FPGA_VERSION)
		return -EPERM;

	if (g->mux_reg >= pctrl->ioctl_non_secure_size) {
		memset(&msg, 0, sizeof(msg));
		msg.fid = FID_IOC_FUNC_GET;
		msg.data0 = group;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0) {
			dev_err(pctrl->dev, "ffa message not arrive in respondent, ret = %d\n", ret);
			return ret;
		}
		if (msg.ret != 0) {
			dev_err(pctrl->dev, "pinctrl read sec_gpio mux_smc fail!\n");
			return msg.ret;
		}
		*val = msg.data1;
	} else {
		*val = readl(pctrl->regs[g->mux_tile] + g->mux_reg);
	}

	return 0;
}

static void xr_writel_mux(u32 val, struct xr_pinctrl *pctrl,
				const struct xr_pingroup *g, unsigned int group)
{
	struct xsp_ffa_msg msg;
	int ret;

	if (pctrl->fpga_debug == XR_DEBUG_FPGA_VERSION)
		return;

	if (g->mux_reg >= pctrl->ioctl_non_secure_size) {
		memset(&msg, 0, sizeof(msg));
		msg.fid = FID_IOC_FUNC_SET;
		msg.data0 = group;
		msg.data1 = val;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0)
			dev_err(pctrl->dev, "ffa message not arrive in respondent, ret = %d\n", ret);
		if (msg.ret != 0)
			dev_err(pctrl->dev, "pinctrl write sec_gpio mux_smc fail!\n");
	} else {
		writel(val, pctrl->regs[g->mux_tile] + g->mux_reg);
	}
}

u32 xr_readl_gpio(struct dwapb_gpio *gc, unsigned int offset)
{
	return readl(gc->base_reg + offset);
}

static void xr_writel_gpio(u32 val, struct dwapb_gpio *gc, unsigned int offset)
{
	writel(val, gc->base_reg + offset);
}

static int xr_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->ngroups;
}

static const char *xr_get_group_name(struct pinctrl_dev *pctldev,
				      unsigned int group)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->groups[group].name;
}

static int xr_get_group_pins(struct pinctrl_dev *pctldev,
			      unsigned int group,
			      const unsigned int **pins,
			      unsigned int *num_pins)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	*pins = pctrl->soc->groups[group].pins;
	*num_pins = pctrl->soc->groups[group].npins;

	return 0;
}

static const struct pinctrl_ops xr_pinctrl_ops = {
	.get_groups_count	= xr_get_groups_count,
	.get_group_name		= xr_get_group_name,
	.get_group_pins		= xr_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_group,
	.dt_free_map		= pinctrl_utils_free_map,
};

static int xr_pinmux_request(struct pinctrl_dev *pctldev, unsigned int pin)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	struct pinctrl_gpio_range *range;
	unsigned int offset;

	range = pinctrl_find_gpio_range_from_pin(pctrl->pctrl, pin);
	if (!range || range->pins) {
		dev_err(pctrl->dev, "Invalid gpio range, pin=%d\n", pin);
		return -EINVAL;
	}

	offset = range->id + pin - range->pin_base;

	return gpiochip_line_is_valid(range->gc, offset) ? 0 : -EINVAL;
}

static int xr_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->nfunctions;
}

static const char *xr_get_function_name(struct pinctrl_dev *pctldev,
					 unsigned int function)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->functions[function].name;
}

static int xr_get_function_groups(struct pinctrl_dev *pctldev,
				   unsigned int function,
				   const char * const **groups,
				   unsigned int * const num_groups)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	*groups = pctrl->soc->functions[function].groups;
	*num_groups = pctrl->soc->functions[function].ngroups;

	return 0;
}

static int xr_pinmux_set_mux(struct pinctrl_dev *pctldev,
			      unsigned int function,
			      unsigned int group)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	const struct xr_pingroup *g;
	unsigned long flags;
	u32 val, mask;
	int i;
	int ret;

	g = &pctrl->soc->groups[group];
	mask = GENMASK(g->mux_bit + order_base_2(g->nfuncs) - 1, g->mux_bit);

	for (i = 0; i < g->nfuncs; i++) {
		if (g->funcs[i] == function)
			break;
	}

	if (WARN_ON(i == g->nfuncs)) {
		dev_err(pctrl->dev, "Invalid function: %d\n", function);
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&pctrl->lock, flags);

	ret = xr_readl_mux(pctrl, g, group, &val);
	if (ret != 0) {
		raw_spin_unlock_irqrestore(&pctrl->lock, flags);
		dev_err(pctrl->dev, "ioc read mux failed!\n");
		return -EIO;
	}
	val &= ~mask;
	val |= i << g->mux_bit;
	xr_writel_mux(val, pctrl, g, group);

	raw_spin_unlock_irqrestore(&pctrl->lock, flags);

	return 0;
}

static int xr_pinmux_request_gpio(struct pinctrl_dev *pctldev,
				   struct pinctrl_gpio_range *range,
				   unsigned int offset)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	const struct xr_pingroup *g = &pctrl->soc->groups[offset];

	/* No funcs? Probably ACPI so can't do anything here */
	if (!g->nfuncs)
		return 0;

	return xr_pinmux_set_mux(pctldev, g->funcs[pctrl->soc->gpio_func], offset);
}

static const struct pinmux_ops xr_pinmux_ops = {
	.request		= xr_pinmux_request,
	.get_functions_count	= xr_get_functions_count,
	.get_function_name	= xr_get_function_name,
	.get_function_groups	= xr_get_function_groups,
	.gpio_request_enable	= xr_pinmux_request_gpio,
	.set_mux		= xr_pinmux_set_mux,
};

#define DEFAULT_DRIVE_LEVEL	5
#define MAX_DRIVE_LEVEL		8

static int pad_tsmc_drive_ma_tbl[MAX_DRIVE_LEVEL] = {
	3, 9, 13, 18, 23, 29, 33, 38,
};

static int pad_snsp_drive_ma_tbl[MAX_DRIVE_LEVEL] = {
	1, 3, 4, 5, 6, 8, 9, 10,
};

static int xr_regval_to_drive_ma(enum xr_pad_type type, u32 val)
{
	if (val >= MAX_DRIVE_LEVEL)
		return -EINVAL;

	if (type == XR_PAD_TSMC_1P2)
		return pad_tsmc_drive_ma_tbl[val];

	if (type == XR_PAD_SNSP_1P8)
		return pad_snsp_drive_ma_tbl[val];

	return -EINVAL;
}

static bool xr_pinconf_is_gpio_param(unsigned int param)
{
	return (param == PIN_CONFIG_INPUT_ENABLE)
		|| (param == PIN_CONFIG_INPUT_DEBOUNCE)
		|| (param == PIN_CONFIG_OUTPUT);
}

static int xr_config_reg(const struct xr_pingroup *g,
			  unsigned int param,
			  unsigned int *mask,
			  unsigned int *bit)
{
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_BUS_HOLD:
		*bit = g->pull_bit;
		*mask = (1 << CFG_PAD_PULL_BITS) - 1;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		*bit = g->drv_bit;
		*mask = (1 << CFG_PAD_DS_BITS) - 1;
		break;
	case PIN_CONFIG_SLEW_RATE:
		*bit = g->sl_bit;
		*mask = (1 << CFG_PAD_SL_BITS) - 1;
		break;
	case PIN_CONFIG_INPUT_SCHMITT:
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		*bit = g->st_bit;
		*mask = (1 << CFG_PAD_ST_BITS) - 1;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int xr_pinconf_ioc_set(struct xr_pinctrl *pctrl, unsigned int group,
			      unsigned int param, unsigned int arg)
{
	const struct xr_pingroup *g;
	unsigned long flags;
	unsigned int mask, bit;
	u32 val;
	int ret;

	g = &pctrl->soc->groups[group];

	ret = xr_config_reg(g, param, &mask, &bit);
	if (ret < 0)
		return ret;

	/* Convert pinconf values to register values */
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		arg = XR_IOC_NO_PULL;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		arg = XR_IOC_PULL_DOWN;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		arg = XR_IOC_PULL_UP;
		break;
	case PIN_CONFIG_BIAS_BUS_HOLD:
		arg = XR_IOC_BUS_KEEPER;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		break;
	case PIN_CONFIG_SLEW_RATE:
		arg = !!arg;
		break;
	case PIN_CONFIG_INPUT_SCHMITT:
		arg = 1;
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		arg = !!arg;
		break;
	default:
		dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
			param);
		return -EINVAL;
	}

	/* Range-check user-supplied value */
	if (arg & ~mask) {
		dev_err(pctrl->dev, "config %x: %x is invalid\n", param, arg);
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&pctrl->lock, flags);
	ret = xr_readl_cfg(pctrl, g, group, &val);
	if (ret != 0) {
		raw_spin_unlock_irqrestore(&pctrl->lock, flags);
		dev_err(pctrl->dev, "ioc read cfg failed!\n");
		return -EIO;
	}
	val &= ~(mask << bit);
	val |= arg << bit;
	xr_writel_cfg(val, pctrl, g, group);
	raw_spin_unlock_irqrestore(&pctrl->lock, flags);

	return 0;
}

static int xr_pinconf_gpio_set(struct xr_pinctrl *pctrl, unsigned int pin,
			       unsigned int param, unsigned int arg)
{
	struct pinctrl_gpio_range *range;
	unsigned int offset;
	int ret = 0;

	range = pinctrl_find_gpio_range_from_pin(pctrl->pctrl, pin);
	if (!range || range->pins) {
		dev_err(pctrl->dev, "Invalid gpio range, pin=%d\n", pin);
		return -EINVAL;
	}

	offset = range->id + pin - range->pin_base;

	switch (param) {
	case PIN_CONFIG_INPUT_DEBOUNCE:
		if (range->gc->set_config) {
			ret = range->gc->set_config(range->gc, offset,
					pinconf_to_config_packed(param, arg));
		} else {
			ret = -EOPNOTSUPP;
		}
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		ret = range->gc->direction_input(range->gc, offset);
		break;
	case PIN_CONFIG_OUTPUT:
		ret = range->gc->direction_output(range->gc, offset, arg);
		break;
	default:
		dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
			param);
		return -EOPNOTSUPP;
	}

	return ret;
}

static int xr_config_group_get(struct pinctrl_dev *pctldev,
				unsigned int group,
				unsigned long *config)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	const struct xr_pingroup *g;
	int ret;
	unsigned int mask, bit;
	unsigned int param, arg, val;

	g = &pctrl->soc->groups[group];
	param = pinconf_to_config_param(*config);

	ret = xr_config_reg(g, param, &mask, &bit);
	if (ret < 0) {
		dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
			param);
		return ret;
	}

	ret = xr_readl_cfg(pctrl, g, group, &val);
	if (ret != 0) {
		dev_err(pctrl->dev, "ioc read cfg failed!\n");
		return -EIO;
	}
	arg = (val >> bit) & mask;
	/* Convert register value to pinconf value */
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		arg = arg ? 1 : 0;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (arg & XR_IOC_PULL_DOWN)
			arg = 1;
		else
			arg = 0;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		if (arg & XR_IOC_PULL_UP)
			arg = 1;
		else
			arg = 0;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		break;
	case PIN_CONFIG_SLEW_RATE:
		arg = !!arg;
		break;
	case PIN_CONFIG_INPUT_SCHMITT:
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		arg = !!arg;
		break;
	default:
		dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
			param);
		return -EOPNOTSUPP;
	}

	*config = pinconf_to_config_packed(param, arg);

	return 0;
}

static int xr_config_group_set(struct pinctrl_dev *pctldev,
				unsigned int group,
				unsigned long *configs,
				unsigned int num_configs)
{
	struct xr_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	unsigned int param, arg;
	int ret;
	int i;

	for (i = 0; i < num_configs; i++) {
		param = pinconf_to_config_param(configs[i]);
		arg = pinconf_to_config_argument(configs[i]);

		if (xr_pinconf_is_gpio_param(param))
			ret = xr_pinconf_gpio_set(pctrl, group, param, arg);
		else
			ret = xr_pinconf_ioc_set(pctrl, group, param, arg);

		if (ret < 0) {
			pr_err("%s: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

static const struct pinconf_ops xr_pinconf_ops = {
	.is_generic		= true,
	.pin_config_group_get	= xr_config_group_get,
	.pin_config_group_set	= xr_config_group_set,
};

static int xr_gpio_get_direction(struct gpio_chip *chip, unsigned int offset)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	u32 val;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}
	val = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DDR);

	return val & BIT(offset) ? GPIO_LINE_DIRECTION_OUT :
				      GPIO_LINE_DIRECTION_IN;
}

static int xr_gpio_direction_input(struct gpio_chip *chip, unsigned int offset)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	int ret;
	unsigned long flags;
	u32 val;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}

	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return ret;

	val = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DDR);
	val &= ~BIT(offset);
	xr_writel_gpio(val, dw_gc, GPIO_SWPORTA_DDR);

	xr_gpio_lock_put(dw_gc, &flags);

	return ret;
}

static void xr_gpio_set_do(struct dwapb_gpio *dw_gc,
				unsigned int offset, int value)
{
	u32 mask_bit;
	u32 dr_reg;
	u32 val;

	dr_reg = GPIO_SWPORTA_DR0;
	if (offset >= BITMASK_DATABITS) {
		dr_reg = GPIO_SWPORTA_DR1;
		offset -= BITMASK_DATABITS;
	}

	mask_bit = BIT(offset) << BITMASK_OFFSET;
	val = (!!value) << offset;
	xr_writel_gpio(mask_bit | val, dw_gc, dr_reg);
}

static int xr_gpio_direction_output(struct gpio_chip *chip,
				    unsigned int offset, int value)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	int ret;
	unsigned long flags;
	u32 val;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}

	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return ret;

	xr_gpio_set_do(dw_gc, offset, value);

	val = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DDR);
	val |= BIT(offset);
	xr_writel_gpio(val, dw_gc, GPIO_SWPORTA_DDR);

	xr_gpio_lock_put(dw_gc, &flags);

	return ret;
}

static int xr_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	u32 val;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}
	val = xr_readl_gpio(dw_gc, GPIO_EXT_PORTA);
	return !!(val & BIT(offset));
}

static void xr_gpio_set(struct gpio_chip *chip, unsigned int offset, int value)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	unsigned long flags;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}

	raw_spin_lock_irqsave(&dw_gc->gc.bgpio_lock, flags);
	xr_gpio_set_do(dw_gc, offset, value);
	raw_spin_unlock_irqrestore(&dw_gc->gc.bgpio_lock, flags);
}

static void xr_gpio_set_external_mask(struct gpio_chip *chip, u32 offset,
				u32 is_masked)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	void __iomem *reg_base;
	unsigned long flags;
	u32 val;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}

	reg_base = dw_gc->extmask_reg;

	/* a gpiochip have most 2 external mask register */
	if (offset >= BITMASK_DATABITS) {
		reg_base += sizeof(u32);
		offset -= BITMASK_DATABITS;
	}

	is_masked = is_masked ? 1 : 0;

	/* bit-mask | irq_mask */
	val = (BIT(offset) << BITMASK_OFFSET) | (is_masked << offset);

	raw_spin_lock_irqsave(&dw_gc->gc.bgpio_lock, flags);
	writel(val, reg_base);
	raw_spin_unlock_irqrestore(&dw_gc->gc.bgpio_lock, flags);
}

static void xr_gpio_set_internal_mask(struct gpio_chip *chip, u32 offset,
				unsigned int is_masked)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	unsigned long flags;
	u32 val;
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}

	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return;
	val = xr_readl_gpio(dw_gc, GPIO_INTMASK);
	if (is_masked)
		val |= BIT(offset);
	else
		val &= ~BIT(offset);
	xr_writel_gpio(val, dw_gc, GPIO_INTMASK);

	xr_gpio_lock_put(dw_gc, &flags);
}

static int xr_gpio_set_debounce(struct gpio_chip *chip, u32 offset,
				unsigned int debounce)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	unsigned int val_deb;
	unsigned long flags;
	unsigned long mask = BIT(offset);
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}

	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return ret;

	val_deb = xr_readl_gpio(dw_gc, GPIO_DEBOUNCE);
	if (debounce)
		val_deb |= mask;
	else
		val_deb &= ~mask;
	xr_writel_gpio(val_deb, dw_gc, GPIO_DEBOUNCE);

	xr_gpio_lock_put(dw_gc, &flags);

	return ret;

}

static int xr_gpio_set_config(struct gpio_chip *gc, unsigned int offset,
				 unsigned long config)
{
	u32 debounce, mask;
	unsigned long config_param;
	int ret = 0;

	config_param = pinconf_to_config_param(config);
	switch (config_param) {
	case PIN_CONFIG_INPUT_DEBOUNCE:
		debounce = pinconf_to_config_argument(config);
		ret = xr_gpio_set_debounce(gc, offset, debounce);
		dev_dbg(&gc->gpiodev->dev, "set debounce=%d set ok\n", debounce);
		break;

	case PIN_CONFIG_XRING_IRQ_MASK_INTERNEL:
		mask = pinconf_to_config_argument(config);
		xr_gpio_set_internal_mask(gc, offset, mask);
		dev_dbg(&gc->gpiodev->dev, "set internal irq mask=%d ok\n", mask);
		break;
	default:
		/* gpiolib will check this error if not support */
		break;
	}

	return ret;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/seq_file.h>
static const char * const pulls_keeper[] = {
	"no pull",
	"pull down",
	"keeper",
	"pull up"
};

static const char * const pulls_no_keeper[] = {
	"no pull",
	"pull down",
	"pull up",
	"up/down?"
};

int xr_gpio_offset_to_pin(struct gpio_chip *chip, unsigned int offset)
{

	struct gpio_device *gdev = chip->gpiodev;
	struct gpio_pin_range *pin_range = NULL, *tmp = NULL;
	struct pinctrl_gpio_range *range;
	unsigned int npins = 0;

	list_for_each_entry_safe(pin_range, tmp, &gdev->pin_ranges, node) {
		range = &pin_range->range;
		npins += range->npins;
		if (npins > offset)
			return range->pin_base + offset - range->id;
	}

	dev_err(chip->parent, "convert offset from gpio to pin failed\n");
	return -EINVAL;
}

static void xr_gpio_dbg_show_one(struct seq_file *s,
				  struct pinctrl_dev *pctldev,
				  struct gpio_chip *chip,
				  unsigned int offset,
				  unsigned int gpio)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	struct xr_pinctrl *pctrl;
	const struct xr_pingroup *g;
	int func, pin;
	int is_out, drive, pull, val;
	u32 ctl_reg = 0;
	u32 mux_reg = 0;
	u32 ext_reg = 0;
	u32 ddr_reg = 0;
	u32 dr0_reg = 0;
	u32 dr1_reg = 0;
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}

	pctrl = dw_gc->xr_pctrl;

	if (!gpiochip_line_is_valid(chip, offset))
		return;

	pin = xr_gpio_offset_to_pin(chip, offset);
	if (pin < 0)
		return;

	g = &pctrl->soc->groups[pin];
	ext_reg = xr_readl_gpio(dw_gc, GPIO_EXT_PORTA);
	ddr_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DDR);
	dr0_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DR0);
	dr1_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DR1);
	if (pctrl->fpga_debug != XR_DEBUG_FPGA_VERSION) {
		ret = xr_readl_cfg(pctrl, g, pin, &ctl_reg);
		if (ret != 0) {
			dev_err(pctrl->dev, "ioc read cfg failed!\n");
			return;
		}
		ret = xr_readl_mux(pctrl, g, pin, &mux_reg);
		if (ret != 0) {
			dev_err(pctrl->dev, "ioc read mux failed!\n");
			return;
		}
	}

	is_out = !!(ddr_reg & BIT(offset));
	if (is_out) {
		if (offset >= BITMASK_DATABITS)
			val = !!(dr1_reg & BIT(offset - BITMASK_DATABITS));
		else
			val = !!(dr0_reg & BIT(offset));
	} else {
		val = !!(ext_reg & BIT(offset));
	}

	func = (mux_reg & MUX_PAD_SEL_MASK) >> g->mux_bit;
	drive = (ctl_reg & CFG_PAD_DS_MASK) >> g->drv_bit;
	pull = (ctl_reg & CFG_PAD_PULL_MASK) >> g->pull_bit;

	seq_printf(s, " %-8s: %-3s", g->name, is_out ? "out" : "in");
	seq_printf(s, " %-4s func%d", val ? "high" : "low", func);
	seq_printf(s, " %dmA", xr_regval_to_drive_ma(g->pad_type, drive));
	seq_printf(s, " mux:0x%x, iopad:0x%x", mux_reg, ctl_reg);
	seq_printf(s, " %s", pulls_no_keeper[pull]);

	seq_puts(s, "\n");
}

static void xr_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip)
{
	unsigned int gpio = chip->base;
	unsigned int i;

	for (i = 0; i < chip->ngpio; i++, gpio++)
		xr_gpio_dbg_show_one(s, NULL, chip, i, gpio);
}

#else
#define xr_gpio_dbg_show NULL
#endif

static const struct gpio_chip xr_gpio_template = {
	.direction_input  = xr_gpio_direction_input,
	.direction_output = xr_gpio_direction_output,
	.get_direction    = xr_gpio_get_direction,
	.get              = xr_gpio_get,
	.set              = xr_gpio_set,
	.request          = gpiochip_generic_request,
	.free             = gpiochip_generic_free,
	.dbg_show         = xr_gpio_dbg_show,
};

static void xr_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);
	u32 gc_offset = irqd_to_hwirq(d);
	u32 val = BIT(gc_offset);
	unsigned long flags;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}

	raw_spin_lock_irqsave(&gc->bgpio_lock, flags);
	xr_writel_gpio(val, dw_gc, GPIO_PORTA_EOI);
	raw_spin_unlock_irqrestore(&gc->bgpio_lock, flags);
}

static void xr_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);
	u32 gc_offset = irqd_to_hwirq(d);

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}
	if (dw_gc->extmask_enabled)
		xr_gpio_set_external_mask(gc, gc_offset, 1);
	else
		xr_gpio_set_internal_mask(gc, gc_offset, 1);
}

static void xr_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	u32 gc_offset = irqd_to_hwirq(d);

	xr_gpio_set_internal_mask(gc, gc_offset, 0);
	xr_gpio_set_external_mask(gc, gc_offset, 0);
}

static void xr_gpio_irq_enable(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);
	u32 gc_offset = irqd_to_hwirq(d);
	unsigned long flags;
	u32 val;
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return;
	}
	if (xr_gpio_doze_flag && dw_gc->doze_function)
		sys_state_doz2nor_vote(VOTER_DOZAP_PERI_GPIO);

	xr_gpio_irq_unmask(d);


	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return;

	val = xr_readl_gpio(dw_gc, GPIO_INTEN);
	val |= BIT(gc_offset);
	xr_writel_gpio(val, dw_gc, GPIO_INTEN);

	xr_gpio_lock_put(dw_gc, &flags);
}

static void xr_gpio_irq_disable(struct irq_data *d)
{
	xr_gpio_irq_mask(d);
}

static int xr_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);
	irq_hw_number_t bit = irqd_to_hwirq(d);
	unsigned long level, polarity, int_bothedge;
	unsigned long flags;
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}

	ret = xr_gpio_lock_get(dw_gc, &flags);
	if (ret)
		return ret;

	level = xr_readl_gpio(dw_gc, GPIO_INTTYPE_LEVEL);
	polarity = xr_readl_gpio(dw_gc, GPIO_INT_POLARITY);

	switch (type) {
	case IRQ_TYPE_EDGE_BOTH:
		/* set int_bothedge will ignore level and polarity register */
		int_bothedge = xr_readl_gpio(dw_gc, GPIO_INT_BOTHEDGE);
		xr_writel_gpio(int_bothedge | BIT(bit), dw_gc, GPIO_INT_BOTHEDGE);
		break;
	case IRQ_TYPE_EDGE_RISING:
		level |= BIT(bit);
		polarity |= BIT(bit);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		level |= BIT(bit);
		polarity &= ~BIT(bit);
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		level &= ~BIT(bit);
		polarity |= BIT(bit);
		break;
	case IRQ_TYPE_LEVEL_LOW:
		level &= ~BIT(bit);
		polarity &= ~BIT(bit);
		break;
	}

	if (type & IRQ_TYPE_LEVEL_MASK)
		irq_set_handler_locked(d, handle_level_irq);
	else if (type & IRQ_TYPE_EDGE_BOTH)
		irq_set_handler_locked(d, handle_edge_irq);

	if (type != IRQ_TYPE_EDGE_BOTH) {
		xr_writel_gpio(level, dw_gc, GPIO_INTTYPE_LEVEL);
		xr_writel_gpio(polarity, dw_gc, GPIO_INT_POLARITY);
		/* disable bothedge to enable the normal interrupt */
		int_bothedge = xr_readl_gpio(dw_gc, GPIO_INT_BOTHEDGE);
		int_bothedge &= ~BIT(bit);
		xr_writel_gpio(int_bothedge, dw_gc, GPIO_INT_BOTHEDGE);
	}

	xr_gpio_lock_put(dw_gc, &flags);

	return ret;
}

static int xr_gpio_irq_set_wake(struct irq_data *d, unsigned int on)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}
	/*
	 * While they may not wake up when the TLMM is powered off,
	 * some GPIOs would like to wakeup the system from suspend
	 * when TLMM is powered on. To allow that, enable the GPIO
	 * summary line to be wakeup capable at GIC.
	 */
	if (d->parent_data)
		return irq_chip_set_wake_parent(d, on);

	return irq_set_irq_wake(dw_gc->irq, on);
}

static int xr_gpio_irq_reqres(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct dwapb_gpio *dw_gc = gpiochip_get_data(gc);
	struct xr_pinctrl *pctrl;
	int offset, pin;
	int ret;

	if (!dw_gc) {
		pr_err("get dw_gc handle fail\n");
		return -EINVAL;
	}

	pctrl = dw_gc->xr_pctrl;

	if (!try_module_get(gc->owner))
		return -ENODEV;

	offset = irqd_to_hwirq(d);
	pin = xr_gpio_offset_to_pin(gc, offset);
	if (pin < 0) {
		ret = pin;
		goto out;
	}

	ret = xr_pinmux_request_gpio(pctrl->pctrl, NULL, pin);
	if (ret)
		goto out;
	xr_gpio_direction_input(gc, offset);

	if (gpiochip_lock_as_irq(gc, offset)) {
		dev_err(gc->parent,
			"unable to lock HW IRQ %lu for IRQ\n",
			d->hwirq);
		ret = -EINVAL;
		goto out;
	}

	/*
	 * The disable / clear-enable workaround we do in xr_pinmux_set_mux()
	 * only works if disable is not lazy since we only clear any bogus
	 * interrupt in hardware. Explicitly mark the interrupt as UNLAZY.
	 */
	irq_set_status_flags(d->irq, IRQ_DISABLE_UNLAZY);

	return 0;
out:
	module_put(gc->owner);
	return ret;
}

static void xr_gpio_irq_relres(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);

	gpiochip_unlock_as_irq(gc, d->hwirq);
	module_put(gc->owner);
}

static void xr_gpio_irq_handler(struct irq_desc *desc)
{
	struct dwapb_gpio *dw_gc = irq_desc_get_handler_data(desc);
	struct gpio_chip *gc = &dw_gc->gc;
	struct irq_chip *chip = irq_desc_get_chip(desc);
	unsigned long irq_status;
	irq_hw_number_t hwirq = 0;
	int handled = 0;
	u32 ext_mask_val;
	u32 ext_mask_val_hi;

	chained_irq_enter(chip, desc);

	irq_status = xr_readl_gpio(dw_gc, GPIO_INTSTATUS);

	/*
	 *	a gpiochip have most 2 external mask register.
	 *	bit-mask | irq_mask, bit[31:16] | bit[15:0]
	 *	GPIO0 ~ 15, gpio_ext_mask_reg_0
	 *	GPIO16 ~ 31, gpio_ext_mask_reg_1
	 *	GPIO32 ~ 47, gpio_ext_mask_reg_3
	 *	...
	 */
	ext_mask_val = readl(dw_gc->extmask_reg) & 0xFFFF;
	ext_mask_val_hi = readl(dw_gc->extmask_reg + sizeof(u32)) & 0xFFFF;

	for_each_set_bit(hwirq, &irq_status, DWAPB_MAX_GPIOS) {
		if (hwirq >= BITMASK_DATABITS)
			ext_mask_val = ext_mask_val_hi;

		if (!(BIT(hwirq) & ext_mask_val)) {
			int gpio_irq = irq_find_mapping(gc->irq.domain, hwirq);

			generic_handle_irq(gpio_irq);
			handled++;
		}
	}

	/* No interrupts were flagged */
	if (handled == 0)
		handle_bad_irq(desc);

	chained_irq_exit(chip, desc);
}

static int xr_gpio_parse_dts_data(struct dwapb_gpio *dw_gc)
{
	u32 tmp[2];
	u32 lock_id;
	const struct fwnode_handle *fwnode;
	struct device *dev = dw_gc->dev;

	fwnode = dw_gc->fwnode;

	dw_gc->gc.label = fwnode_get_name(fwnode);

	if (fwnode_property_read_u32(fwnode, "doze_function", &dw_gc->doze_function)) {
		dev_err(dev, "failed to get doze_function property\n");
		dw_gc->doze_function = 0;
	}

	if (fwnode_property_read_u32(fwnode, "lockid", &lock_id)) {
		dev_warn(dev, "no lockid property\n");
		lock_id = INVALID_LOCKID;
	}
	dw_gc->hwlock = hwspin_lock_request_specific(lock_id);
	if (!dw_gc->hwlock)
		dev_warn(dev, "invalid or used hwspinlock(%d)\n", lock_id);

	/* read base addr and size to remap */
	if (fwnode_property_read_u32_array(fwnode, "xring,reg", tmp, 2)) {
		dev_err(dev, "gc [%s]: missing/invalid base addr\n",
			fwnode_get_name(fwnode));
		return -EINVAL;
	}
	dev_dbg(dev, "gc [%s]: base_phyaddr: 0x%08x, size: 0x%x\n",
		 fwnode_get_name(fwnode), (u32)tmp[0], (u32)tmp[1]);
	dw_gc->base_reg = devm_ioremap(dev, tmp[0], tmp[1]);
	if (!dw_gc->base_reg) {
		dev_err(dev, "failed to of_iomap\n");
		return -ENOMEM;
	}

	if (fwnode_property_read_u32(fwnode, "xring,extmask-enable", tmp)) {
		dev_dbg(dev, "[%s]: extmask-enable enabled default\n",
			  fwnode_get_name(fwnode));
		dw_gc->extmask_enabled = 1;
	} else {
		dw_gc->extmask_enabled = tmp[0] ? 1 : 0;
		dev_dbg(dev, "[%s]: extmask-enable = %d\n",
			fwnode_get_name(fwnode), dw_gc->extmask_enabled);
	}

	/* read register for external interrupt mask */
	if (fwnode_property_read_u32_array(fwnode, "xring,extmask-reg", tmp, 2)) {
		dev_err(dev, "gc [%s]: missing/invalid extmask-reg addr\n",
			fwnode_get_name(fwnode));
		return -EINVAL;
	}

	dev_dbg(dev, "gc [%s]: extmask-reg phyaddr: 0x%08x, size: 0x%x\n",
		 fwnode_get_name(fwnode), tmp[0], tmp[1]);
	dw_gc->extmask_reg = devm_ioremap(dev, tmp[0], tmp[1]);
	if (!dw_gc->extmask_reg) {
		dev_err(dev, "failed to of_iomap\n");
		return -ENOMEM;
	}

	if (fwnode_property_read_u32(fwnode, "xring,nr-gpios", tmp)) {
		dev_err(dev, "gc [%s]: failed to get nr-gpios, set default 32\n",
			 fwnode_get_name(fwnode));
		dw_gc->gc.ngpio = DWAPB_MAX_GPIOS;
	} else {
		dw_gc->gc.ngpio = tmp[0];
	}

	if (dw_gc->gc.ngpio > DWAPB_MAX_GPIOS) {
		dev_err(dev, "gc [%s]: invalid number of gpio\n",
			fwnode_get_name(fwnode));
		return -EINVAL;
	}

	dev_dbg(dev, "gc [%s]: ngpio: %d\n", fwnode_get_name(fwnode),
		 dw_gc->gc.ngpio);

	dw_gc->irq = fwnode_irq_get(fwnode, 0);
	if (dw_gc->irq < 0) {
		dev_err(dev, "gc [%s]: missing/invalid irq, ret=%d\n",
			fwnode_get_name(fwnode), dw_gc->irq);
		return -EINVAL;
	}
	dev_dbg(dev, "gc [%s]: irq: %d\n", fwnode_get_name(fwnode),
		 dw_gc->irq);

	return 0;
}

static void xr_gpio_disable_clks(void *data)
{
	struct dwapb_gpio *gpio = data;

	clk_bulk_disable_unprepare(DWAPB_NR_CLOCKS, gpio->clks);
}

static int xr_gpio_get_clks(struct dwapb_gpio *gpio)
{
	int err;

	/* Optional bus and debounce clocks */
	gpio->clks[0].id = "bus";
	gpio->clks[1].id = "db";
	err = devm_clk_bulk_get_optional(gpio->dev, DWAPB_NR_CLOCKS,
					 gpio->clks);
	if (err)
		return dev_err_probe(gpio->dev, err,
				     "Cannot get APB/Debounce clocks\n");

	err = clk_bulk_prepare_enable(DWAPB_NR_CLOCKS, gpio->clks);
	if (err) {
		dev_err(gpio->dev, "Cannot enable APB/Debounce clocks\n");
		return err;
	}

	return devm_add_action_or_reset(gpio->dev, xr_gpio_disable_clks, gpio);
}

static int xr_gpio_configure_irq(struct dwapb_gpio *dw_gpio)
{
	struct irq_chip *irq_chip = &dw_gpio->irq_chip;
	struct gpio_irq_chip *girq;

	irq_chip->name = "xr-dwapb-gpio";
	irq_chip->irq_enable = xr_gpio_irq_enable;
	irq_chip->irq_disable = xr_gpio_irq_disable;
	irq_chip->irq_mask = xr_gpio_irq_mask;
	irq_chip->irq_unmask = xr_gpio_irq_unmask;
	irq_chip->irq_ack = xr_gpio_irq_ack;
	irq_chip->irq_set_type = xr_gpio_irq_set_type;
	irq_chip->irq_set_wake = xr_gpio_irq_set_wake;
	irq_chip->irq_request_resources = xr_gpio_irq_reqres;
	irq_chip->irq_release_resources = xr_gpio_irq_relres;

	girq = &dw_gpio->gc.irq;
	girq->chip = irq_chip;
	girq->parent_handler = xr_gpio_irq_handler;
	girq->parent_handler_data = dw_gpio;
	girq->fwnode = dw_gpio->fwnode;
	girq->num_parents = 1;
	girq->parents = devm_kcalloc(dw_gpio->dev, 1, sizeof(*girq->parents),
				     GFP_KERNEL);
	if (!girq->parents)
		return -ENOMEM;

	girq->default_type = IRQ_TYPE_NONE;
	girq->handler = handle_bad_irq;
	girq->parents[0] = dw_gpio->irq;

	return 0;
}

static int xr_dwgpio_init(struct dwapb_gpio *dw_gpio)
{
	int ret;
	struct gpio_chip *chip;
	struct device *dev = dw_gpio->dev;
	const struct fwnode_handle *fwnode = NULL;

	chip = &dw_gpio->gc;

	/* allocated by gpiolib */
	chip->base = -1;
	chip->parent = dev;
	chip->owner = THIS_MODULE;

	chip->fwnode = dw_gpio->fwnode;

	chip->direction_input = xr_gpio_direction_input;
	chip->direction_output = xr_gpio_direction_output;
	chip->get_direction = xr_gpio_get_direction;
	chip->get = xr_gpio_get;
	chip->set = xr_gpio_set;
	chip->set_config = xr_gpio_set_config;
	chip->request = gpiochip_generic_request;
	chip->free = gpiochip_generic_free;
	chip->dbg_show = xr_gpio_dbg_show;

	raw_spin_lock_init(&chip->bgpio_lock);
	ret = xr_gpio_get_clks(dw_gpio);
	if (ret)
		return ret;

	ret = xr_gpio_configure_irq(dw_gpio);
	if (ret) {
		dev_err(dev, "Config irq failed, ret=%d", ret);
		return ret;
	}

	ret = devm_gpiochip_add_data(dev, chip, dw_gpio);
	if (ret) {
		dev_err(dev, "Failed to register gpiochip for %s\n",
			fwnode_get_name(fwnode));
		return ret;
	}

	return 0;
}

static int xr_gpio_init(struct xr_pinctrl *pctrl)
{
	int i, ret;
	struct device *dev = pctrl->dev;
	struct fwnode_handle *fwnode = NULL;
	struct fwnode_handle *child_fn = NULL;
	struct dwapb_gpio *dw_gpio = NULL;
	int nr_gpiochips = 0;

	fwnode = device_get_named_child_node(dev, "gpiochips");
	if (!fwnode) {
		dev_err(dev, "gpiochips not found\n");
		return -ENODEV;
	}

	i = 0;
	fwnode_for_each_child_node(fwnode, child_fn) {
		dw_gpio = &pctrl->dw_gpios[i++];

		dw_gpio->dev = dev;
		dw_gpio->fwnode = child_fn;
		dw_gpio->xr_pctrl = pctrl;

		ret = xr_gpio_parse_dts_data(dw_gpio);
		if (ret)
			return ret;

		ret = xr_dwgpio_init(dw_gpio);
		if (ret)
			return ret;

		nr_gpiochips += 1;

		if (nr_gpiochips > MAX_NR_GPIO_CHIP) {
			dev_err(dev, "exceeded max gpiochips, nr=%d, max=%d\n",
				nr_gpiochips, MAX_NR_GPIO_CHIP);
			return -EINVAL;
		}
	}

	if (nr_gpiochips == 0) {
		dev_err(dev, "gpio chips not found\n");
		return -ENODEV;
	}

	pctrl->n_dwgpios = nr_gpiochips;

	return 0;
}

static __maybe_unused int xr_pinctrl_suspend(struct device *dev)
{
	struct xr_pinctrl *pctrl = dev_get_drvdata(dev);

	return pinctrl_force_sleep(pctrl->pctrl);
}

static __maybe_unused int xr_pinctrl_resume(struct device *dev)
{
	struct xr_pinctrl *pctrl = dev_get_drvdata(dev);

	return pinctrl_force_default(pctrl->pctrl);
}

const struct dev_pm_ops __maybe_unused xr_pinctrl_dev_pm_ops = {
	.suspend = xr_pinctrl_suspend,
	.resume = xr_pinctrl_resume,
};
EXPORT_SYMBOL(xr_pinctrl_dev_pm_ops);

int xr_pinctrl_probe(struct platform_device *pdev,
		      const struct xr_pinctrl_soc_data *soc_data)
{
	struct xr_pinctrl *pctrl;
	struct resource *res;
	int ret;
	int i;

	if (!soc_data) {
		dev_err(&pdev->dev, "invalid soc_data\n");
		return -EINVAL;
	}

	pctrl = devm_kzalloc(&pdev->dev, sizeof(*pctrl), GFP_KERNEL);
	if (!pctrl)
		return -ENOMEM;

	pctrl->dev = &pdev->dev;
	pctrl->soc = soc_data;

	raw_spin_lock_init(&pctrl->lock);

	if (soc_data->tiles) {
		for (i = 0; i < soc_data->ntiles; i++) {
			res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							   soc_data->tiles[i]);
			pctrl->regs[i] = devm_ioremap_resource(&pdev->dev, res);
			if (IS_ERR(pctrl->regs[i])) {
				dev_err(&pdev->dev, "invalid reg addr\n");
				return PTR_ERR(pctrl->regs[i]);
			}
		}
	} else {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		pctrl->regs[0] = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(pctrl->regs[0])) {
			dev_err(&pdev->dev, "invalid reg addr\n");
			return PTR_ERR(pctrl->regs[0]);
		}

		pctrl->phys_base[0] = res->start;
	}

	ret = device_property_read_u32(&pdev->dev, "ioc-non-secure-size",
					&pctrl->ioctl_non_secure_size);
	if (ret)
		return ret;

	pctrl->fpga_debug = !XR_DEBUG_FPGA_VERSION;
	ret = device_property_read_bool(&pdev->dev, "fpga-debug-iomux");
	if (ret) {
		/* ret == true, fpga version */
		pctrl->fpga_debug = XR_DEBUG_FPGA_VERSION;
	}

	pctrl->desc.owner = THIS_MODULE;
	pctrl->desc.pctlops = &xr_pinctrl_ops;
	pctrl->desc.pmxops = &xr_pinmux_ops;
	pctrl->desc.confops = &xr_pinconf_ops;
	pctrl->desc.name = dev_name(&pdev->dev);
	pctrl->desc.pins = pctrl->soc->pins;
	pctrl->desc.npins = pctrl->soc->npins;

	pctrl->pctrl = devm_pinctrl_register(&pdev->dev, &pctrl->desc, pctrl);
	if (IS_ERR(pctrl->pctrl)) {
		dev_err(&pdev->dev, "Couldn't register pinctrl driver\n");
		return PTR_ERR(pctrl->pctrl);
	}

	ret = xr_gpio_init(pctrl);
	if (ret) {
		dev_err(&pdev->dev, "xring gpio init failed\n");
		return ret;
	}

	ret = xr_gpio_debug_init(pctrl);
	if (ret)
		dev_err(&pdev->dev, "xring gpio debug init failed\n");

	platform_set_drvdata(pdev, pctrl);

	dev_info(&pdev->dev, "Probed XRing pinctrl driver, %d gpiochip found\n",
			pctrl->n_dwgpios);

	return 0;
}
EXPORT_SYMBOL(xr_pinctrl_probe);

MODULE_DESCRIPTION("XRing Pin controller driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xsp1");
