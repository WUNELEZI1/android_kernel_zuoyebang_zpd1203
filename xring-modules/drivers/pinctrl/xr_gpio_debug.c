// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/syscore_ops.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include "dt-bindings/xring/platform-specific/xr_gpio_pri_data.h"
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>
#include "xsp_ffa.h"
#include "xr_gpio_debug.h"

#include "dt-bindings/xring/platform-specific/DW_apb_gpio_header.h"

#ifndef XR_MUX_GPIO
#define XR_MUX_GPIO 0
#endif

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1
#define BITMASK_DATABITS 16
#define XRSE_GPIO_MIN 171
#define XRSE_GPIO_MAX 176
#define DEBUGFS_INPUT_VAL_SIZE_MAX 4096
#define DECIMALISM 10
#define HEXADECIMAL 16

#define IS_SLEEP_NAME(name) (strcmp((name + strlen(name) - strlen("sleep")), "sleep") == 0)

/**
 * Private struct difinition
 */

enum xr_gpio_in_out_e {
	XR_GPIO_NULL,
	XR_GPIO_IN,
	XR_GPIO_OUT_HIGH,
	XR_GPIO_OUT_LOW,
};

struct xr_gpio_in_out_params {
	const char * const property;
	enum xr_gpio_in_out_e param;
};

static struct xr_gpio_in_out_params inout_params[] = {
	{ "not-gpio", XR_GPIO_NULL},
	{ "gpio-input", XR_GPIO_IN},
	{ "gpio-output-high", XR_GPIO_OUT_HIGH},
	{ "gpio-output-low", XR_GPIO_OUT_LOW},
};

static u32 current_gpio_num;

static const char *const xr_gpio_in_out_name[] = {
	" - ",
	" in ",
	"out-1",
	"out-0",
};

union pinconf_u {
	struct {
		u32 slew_rate : 1;
		bool input_schmitt_en : 1;
		bool pull_down : 1;
		bool pull_up : 1;
		u32 drive_strength : 4;
		u32 reserved : 24;
	} prop;
	u32 reg_val;
};

struct one_gpio_info {
	bool writed;
	union pinconf_u pinconf;
	u32 mux;
	u32 in_out;
};

struct gpio_debug_info {
	struct device *dev;
	struct xr_pinctrl *xr_pctrl;
	struct one_gpio_info *dtb_info;
	u32 dtb_node_cnt;
	struct dentry *root;
	struct dentry *gpios;
	bool sr_debug_en;
};

struct gpio_debug_info *gpio_dbg;

static void xr_writel_mux(u32 val, struct xr_pinctrl *pctrl,
				const struct xr_pingroup *g, unsigned int group)
{
	struct xsp_ffa_msg msg;
	int ret;

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

static void xr_writel_cfg(u32 val, struct xr_pinctrl *pctrl,
				const struct xr_pingroup *g, unsigned int group)
{
	struct xsp_ffa_msg msg;
	int ret;

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

static inline void xr_gpio_dbg_print_line(struct one_gpio_info *gpio_reg_info, u32 pin,
						const char *msg)
{
	const struct xr_pinctrl_soc_data *pctrl_data = gpio_dbg->xr_pctrl->soc;

	/*  PAD_GPIO_xxx drive pull_up pull_down slew schmitt func_name  gpio  other */
	pr_info(" %s %-5u %-7u %-9u %-4u %-7u %-10s %-5s %s\n", pctrl_data->pins[pin].name,
		gpio_reg_info->pinconf.prop.drive_strength, gpio_reg_info->pinconf.prop.pull_up,
		gpio_reg_info->pinconf.prop.pull_down, gpio_reg_info->pinconf.prop.slew_rate,
		gpio_reg_info->pinconf.prop.input_schmitt_en,
		pctrl_data->functions[pctrl_data->groups[pin].funcs[gpio_reg_info->mux]].name,
		xr_gpio_in_out_name[gpio_reg_info->in_out], msg);
}

static void xr_get_gpio_in_out(struct gpio_chip *chip, u32 offset, struct one_gpio_info *reg_info)
{
	struct dwapb_gpio *dw_gc = gpiochip_get_data(chip);
	u32 ext_reg = 0;
	u32 ddr_reg = 0;
	u32 dr0_reg = 0;
	u32 dr1_reg = 0;
	int is_out, val;

	if (!gpiochip_line_is_valid(chip, offset))
		return;

	ext_reg = xr_readl_gpio(dw_gc, GPIO_EXT_PORTA);
	ddr_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DDR);
	dr0_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DR0);
	dr1_reg = xr_readl_gpio(dw_gc, GPIO_SWPORTA_DR1);
	is_out = !!(ddr_reg & BIT(offset));
	pr_debug("is_out:%d, ext_reg:0x%x, ddr_reg:0x%x, dr0_reg:0x%x, dr1_reg:0x%x,offset:%d\n",
		is_out, ext_reg, ddr_reg, dr0_reg, dr1_reg, offset);
	if (is_out) {
		if (offset >= BITMASK_DATABITS)
			val = !!(dr1_reg & BIT(offset - BITMASK_DATABITS));
		else
			val = !!(dr0_reg & BIT(offset));
	} else {
		val = !!(ext_reg & BIT(offset));
	}

	if (is_out)
		reg_info->in_out = val ? XR_GPIO_OUT_HIGH : XR_GPIO_OUT_LOW;
	else
		reg_info->in_out = XR_GPIO_IN;
}

static void xr_gpio_dbg_show_one(struct gpio_chip *chip, u32 offset)
{
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	struct one_gpio_info *gpios_info = gpio_dbg->dtb_info;
	const struct xr_pingroup *g;
	struct one_gpio_info gpio_reg_info = {0};
	bool is_io_err;
	int pin;
	int ret;

	if (!gpiochip_line_is_valid(chip, offset))
		return;

	pin = xr_gpio_offset_to_pin(chip, offset);
	if (pin < 0 || pin >= gpio_dbg->dtb_node_cnt)
		return;

	if (gpio_dbg->dtb_info[pin].writed == false)
		return;

	g = &pctrl->soc->groups[pin];
	ret = xr_readl_cfg(pctrl, g, pin, &gpio_reg_info.pinconf.reg_val);
	if (ret != 0)
		return;
	ret = xr_readl_mux(pctrl, g, pin, &gpio_reg_info.mux);
	if (ret != 0)
		return;
	gpio_reg_info.writed = true;

	if (pctrl->soc->groups[pin].funcs[gpio_reg_info.mux] == XR_MUX_GPIO)
		xr_get_gpio_in_out(chip, offset, &gpio_reg_info);

	is_io_err = (bool)memcmp(&gpio_reg_info, &gpios_info[pin], sizeof(gpio_reg_info));

	xr_gpio_dbg_print_line(&gpio_reg_info, pin, is_io_err ? "[io_err]" : "");
	xr_gpio_dbg_print_line(&gpios_info[pin], pin, "[from-dtb]");
	pr_info("\n");
}

static int xr_pin_get_from_name(const char *name, u32 *pin)
{
	u32 i;
	const struct xr_pinctrl_soc_data *pctrl_data = gpio_dbg->xr_pctrl->soc;

	for (i = 0; i < pctrl_data->npins; i++) {
		if (!strcmp(name, pctrl_data->pins[i].name)) {
			*pin = pctrl_data->pins[i].number;
			return 0;
		}
	}

	return -EINVAL;
}

static int xr_mux_get_from_func_name(u32 pin, const char *func_name, u32 *mux)
{
	u32 func_id, mux_index;
	const struct xr_pinctrl_soc_data *pctrl_data = gpio_dbg->xr_pctrl->soc;

	for (func_id = 0; func_id < pctrl_data->npins; func_id++) {
		if (!strcmp(func_name, pctrl_data->functions[func_id].name))
			break;
	}

	for (mux_index = 0; mux_index < pctrl_data->groups[pin].nfuncs; mux_index++) {
		if (pctrl_data->groups[pin].funcs[mux_index] == func_id) {
			*mux = mux_index;
			return 0;
		}
	}

	return -EINVAL;
}

static int xr_gpio_parse_pinconf(struct device_node *pinconf_node, union pinconf_u *pinconf)
{
	int ret;
	u32 buffer;

	if (of_find_property(pinconf_node, "bias-disable", NULL)) {
		pinconf->prop.pull_down = false;
		pinconf->prop.pull_up = false;
	}
	if (of_find_property(pinconf_node, "bias-bus-hold", NULL)) {
		pinconf->prop.pull_down = true;
		pinconf->prop.pull_up = true;
	}
	if (of_find_property(pinconf_node, "bias-pull-up", NULL)) {
		pinconf->prop.pull_down = false;
		pinconf->prop.pull_up = true;
	}
	if (of_find_property(pinconf_node, "bias-pull-down", NULL)) {
		pinconf->prop.pull_down = true;
		pinconf->prop.pull_up = false;
	}

	if (of_find_property(pinconf_node, "input-schmitt-enable", NULL))
		pinconf->prop.input_schmitt_en = true;

	ret = of_property_read_u32(pinconf_node, "drive-strength", &buffer);
	if (ret != 0) {
		dev_warn(gpio_dbg->dev, "read pinconf drive failed(%d), set to 0\n", ret);
		buffer = 0;
	}
	pinconf->prop.drive_strength = buffer;

	ret = of_property_read_u32(pinconf_node, "slew-rate", &buffer);
	if (ret != 0) {
		dev_warn(gpio_dbg->dev, "read pinconf slew-rate failed(%d), set to 0\n", ret);
		buffer = 0;
	}
	pinconf->prop.slew_rate = buffer;

	return 0;
}

static void xr_gpio_parse_in_out(struct device_node *pinconf_node, u32 *in_out)
{
	if (of_find_property(pinconf_node, "input-enable", NULL)) {
		*in_out = XR_GPIO_IN;
		return;
	}
	if (of_find_property(pinconf_node, "output-high", NULL)) {
		*in_out = XR_GPIO_OUT_HIGH;
		return;
	}
	if (of_find_property(pinconf_node, "output-low", NULL)) {
		*in_out = XR_GPIO_OUT_LOW;
		return;
	}
}

static int xr_gpio_parse_mux(struct device_node *mux_node, int pin_index, u32 *pin, u32 *mux)
{
	const char *pin_msg, *function_name;
	int ret;

	ret = of_property_read_string_index(mux_node, "pins", pin_index, &pin_msg);
	if (ret != 0) {
		dev_err(gpio_dbg->dev, "get pins failed(%d)\n", ret);
		return -ENXIO;
	}

	ret = xr_pin_get_from_name(pin_msg, pin);
	if (ret != 0) {
		dev_err(gpio_dbg->dev, "get pin num failed(%d)\n", ret);
		return -ENXIO;
	}

	ret = of_property_read_string(mux_node, "function", &function_name);
	if (ret != 0) {
		dev_err(gpio_dbg->dev, "read mux function failed(%d)\n", ret);
		return -ENXIO;
	}

	ret = xr_mux_get_from_func_name(*pin, function_name, mux);
	if (ret != 0) {
		dev_err(gpio_dbg->dev, "get mux failed(%d)\n", ret);
		return -ENXIO;
	}

	return 0;
}

static void xr_gpio_write_dtb_info(struct device_node *pin_node, u32 pin,
					struct one_gpio_info dtb_info)
{
	if (gpio_dbg->dtb_info[pin].writed == true) {
		if (gpio_dbg->dtb_info[pin].pinconf.reg_val != dtb_info.pinconf.reg_val) {
			dev_warn(gpio_dbg->dev, "node_name is %s, pin[%u] pinconf rewrite\n",
				 pin_node->name, pin);
		}
		if (gpio_dbg->dtb_info[pin].mux != dtb_info.mux) {
			dev_warn(gpio_dbg->dev, "node_name is %s, pin[%u] mux rewrite\n",
				 pin_node->name, pin);
		}
		if (gpio_dbg->dtb_info[pin].in_out != dtb_info.in_out) {
			dev_warn(gpio_dbg->dev, "node_name is %s, pin[%u] in_out rewrite\n",
				 pin_node->name, pin);
		}
	} else {
		gpio_dbg->dtb_info[pin] = dtb_info;
		gpio_dbg->dtb_info[pin].writed = true;
	}
}

static int xr_gpio_parse_child_node(struct device_node *pin_node)
{
	struct device_node *mux_node, *pinconf_node;
	struct one_gpio_info dtb_info = { 0 };
	int ret, pin_index, pin_count;
	u32 pin, mux;

	pinconf_node = of_get_child_by_name(pin_node, "pinconf");
	mux_node = of_get_child_by_name(pin_node, "mux");
	if (!pinconf_node || !mux_node) {
		dev_err(gpio_dbg->dev, "node_name[%s]: pinconf or mux node no found\n",
			pin_node->name);
		return -ENXIO;
	}

	ret = xr_gpio_parse_pinconf(pinconf_node, &dtb_info.pinconf);
	if (ret != 0) {
		dev_err(gpio_dbg->dev, "node_name[%s]: parse pinconf fail\n", pin_node->name);
		return -ENXIO;
	}

	pin_count = of_property_count_strings(mux_node, "pins");
	if (pin_count <= 0) {
		dev_err(gpio_dbg->dev, "node_name[%s]: read mux pins failed(%d)\n", pin_node->name,
			pin_count);
		return -ENXIO;
	}

	for (pin_index = 0; pin_index < pin_count; pin_index++) {
		ret = xr_gpio_parse_mux(mux_node, pin_index, &pin, &mux);
		if (ret != 0) {
			dev_err(gpio_dbg->dev, "node_name[%s]: parse mux fail\n", pin_node->name);
			return -ENXIO;
		}

		if (gpio_dbg->xr_pctrl->soc->groups[pin].funcs[mux] == XR_MUX_GPIO)
			xr_gpio_parse_in_out(pinconf_node, &dtb_info.in_out);
		else
			dtb_info.in_out = XR_GPIO_NULL;

		dtb_info.mux = mux;
		xr_gpio_write_dtb_info(pin_node, pin, dtb_info);
	}
	return 0;
}

static int xr_gpio_parse_dtb(void)
{
	struct device_node *root, *parent, *child;

	root = of_node_get(of_root);
	if (!root) {
		dev_err(gpio_dbg->dev, "can not found root node.\n");
		return -ENXIO;
	}

	parent = of_find_node_by_name(root, "pinctrl-configs");
	if (!parent) {
		dev_err(gpio_dbg->dev, "can not found pinctrl-configs node.\n");
		return -ENXIO;
	}

	for_each_child_of_node(parent, child) {
		if (IS_SLEEP_NAME(child->name))
			xr_gpio_parse_child_node(child);
	}
	return 0;
}

static int xr_gpio_sr_debug_suspend(void)
{
	struct gpio_chip *chip;
	int i = 0;

	if (!gpio_dbg->sr_debug_en)
		return 0;

	pr_info("\n\n");
	pr_info("------------------------------ GPIO SR DEBUG ------------------------------\n");
	pr_info(" PAD_GPIO_xxx drive pull_up pull_down slew schmitt func_name  gpio  other\n");
	pr_info("---------------------------------------------------------------------------\n");
	pr_info("\n");

	for (i = 0; i < gpio_dbg->xr_pctrl->n_dwgpios; i++) {
		chip = &gpio_dbg->xr_pctrl->dw_gpios[i].gc;
		for (int offset = 0; offset < chip->ngpio; offset++)
			xr_gpio_dbg_show_one(chip, offset);
	}

	pr_info("\n\n");

	return 0;
}

static struct syscore_ops gpio_sr_debug_syscore_ops = {
	.suspend = xr_gpio_sr_debug_suspend,
};

static u32 cal_io_soft_num(int gpio)
{
	int i;
	u32 controller_id = 0;
	u32 offset = 0;
	u32 current_gpio_soft_num = GPIO_DYNAMIC_BASE;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;

	CONVERT_GPIO_AND_IP(current_gpio_num, controller_id, offset);
	for (i = 0; i < controller_id; i++)
		current_gpio_soft_num += pctrl->dw_gpios[i].gc.ngpio;

	current_gpio_soft_num += offset;
	pr_debug("io_num:%d, controller_id:%d, offset:%d, current_gpio_soft_num:%d,i:%d\n",
		current_gpio_num, controller_id, offset, current_gpio_soft_num, i);
	return current_gpio_soft_num;
}

static ssize_t xr_gpios_val_write(struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf;
	u32 value;
	u32 io_soft_num;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0';

	if (kstrtou32(buf, DECIMALISM, &value) != 0) {
		pr_err("Invalid integer value received\n");
		kfree(buf);
		return -EINVAL;
	}

	io_soft_num = cal_io_soft_num(current_gpio_num);

	gpio_set_value(io_soft_num, value);
	kfree(buf);
	return count;
}

static ssize_t xr_gpios_val_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	int cnt;
	u32 r_val = 0;
	char buf[30];
	u32 io_soft_num;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	io_soft_num = cal_io_soft_num(current_gpio_num);
	r_val = gpio_get_value(io_soft_num);
	pr_debug("%s r_val:0x%x, soft_num:%d\n",
		 __func__, r_val, io_soft_num);
	cnt = snprintf(buf, sizeof(buf), "r_val(io=%d):0x%x\n", current_gpio_num, r_val);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static ssize_t xr_gpios_direction_write(struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf;
	u32 dir;
	int val_set;
	int ret;
	u32 io_soft_num;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;
	struct one_gpio_info gpio_reg_info = {0};

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0';

	if (kstrtou32(buf, DECIMALISM, &dir) != 0) {
		pr_err("Invalid integer dir received\n");
		kfree(buf);
		return -EINVAL;
	}

	g = &pctrl->soc->groups[current_gpio_num];
	ret = xr_readl_mux(pctrl, g, current_gpio_num, &gpio_reg_info.mux);
	if (ret != 0) {
		pr_err("r_mux fail, ret: %d\n", ret);
		kfree(buf);
		return -EFAULT;
	}
	if (pctrl->soc->groups[current_gpio_num].funcs[gpio_reg_info.mux] != XR_MUX_GPIO) {
		pr_err("current_io_func is: %s(not gpio), please config mux to gpio first!\n",
			inout_params[gpio_reg_info.in_out].property);
		kfree(buf);
		return -EFAULT;
	}

	io_soft_num = cal_io_soft_num(current_gpio_num);
	if (dir == GPIO_DIR_IN) {
		gpio_direction_input(io_soft_num);
	} else {
		val_set = gpio_get_value(io_soft_num);
		ret = gpio_direction_output(io_soft_num, val_set);
	}

	kfree(buf);
	return count;
}

static ssize_t xr_gpios_direction_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;
	int ret;
	int cnt;
	char buf[50];
	struct one_gpio_info gpio_reg_info = {0};
	struct gpio_chip *chip;
	u32 controller_id = 0;
	u32 offset = 0;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	CONVERT_GPIO_AND_IP(current_gpio_num, controller_id, offset);
	pr_debug("io_num:%d, controller_id:%d, offset:%d",
		current_gpio_num, controller_id, offset);

	if (controller_id >= pctrl->n_dwgpios || controller_id >= MAX_NR_GPIO_CHIP) {
		pr_err("convert contro_id(%d) invalid, max:n_dwgpios(%d) or MAXGPIO_CHIP(%d)\n",
			controller_id, pctrl->n_dwgpios, MAX_NR_GPIO_CHIP);
		cnt = snprintf(buf, sizeof(buf), "r_dir fail,invalid contro_id\n");
		return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
	}
	chip = &gpio_dbg->xr_pctrl->dw_gpios[controller_id].gc;
	g = &pctrl->soc->groups[current_gpio_num];
	ret = xr_readl_mux(pctrl, g, current_gpio_num, &gpio_reg_info.mux);
	if (ret != 0) {
		cnt = snprintf(buf, sizeof(buf), "r_mux fail,ret%d\n", ret);
		return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
	}
	if (pctrl->soc->groups[current_gpio_num].funcs[gpio_reg_info.mux] == XR_MUX_GPIO)
		xr_get_gpio_in_out(chip, offset, &gpio_reg_info);

	cnt = snprintf(buf, sizeof(buf), "r_dir(io=%d):%s\n",
		current_gpio_num, inout_params[gpio_reg_info.in_out].property);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static ssize_t xr_gpios_config_write(struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf;
	u32 value;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0';

	if (kstrtou32(buf, HEXADECIMAL, &value) != 0) {
		pr_err("Invalid integer value received\n");
		kfree(buf);
		return -EINVAL;
	}

	g = &pctrl->soc->groups[current_gpio_num];

	xr_writel_cfg(value, pctrl, g, current_gpio_num);
	kfree(buf);
	return count;
}

static ssize_t xr_gpios_config_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;
	int ret;
	int cnt;
	u32 r_config = 0;
	char buf[30];

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	g = &pctrl->soc->groups[current_gpio_num];
	ret = xr_readl_cfg(pctrl, g, current_gpio_num, &r_config);
	if (ret != 0) {
		pr_err("xr_readl_cfg fail, ret:%d, r_config:%d\n", ret, r_config);
		return -EIO;
	}

	cnt = snprintf(buf, sizeof(buf), "r_config(io=%d):0x%x\n",
		current_gpio_num, r_config);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static ssize_t xr_gpios_mux_write(struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf;
	u32 value;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0';

	if (kstrtou32(buf, DECIMALISM, &value) != 0) {
		pr_err("Invalid integer value received\n");
		kfree(buf);
		return -EINVAL;
	}

	g = &pctrl->soc->groups[current_gpio_num];

	xr_writel_mux(value, pctrl, g, current_gpio_num);
	kfree(buf);
	return count;
}

static ssize_t xr_gpios_mux_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;
	const struct xr_pingroup *g;
	int ret;
	int cnt;
	u32 r_mux = 0;
	char buf[30];

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	g = &pctrl->soc->groups[current_gpio_num];
	ret = xr_readl_mux(pctrl, g, current_gpio_num, &r_mux);
	if (ret != 0) {
		pr_err("xr_readl_mux fail, ret:%d, r_mux:%d\n", ret, r_mux);
		return -EIO;
	}

	cnt = snprintf(buf, sizeof(buf), "r_mux(io=%d):0x%x\n", current_gpio_num, r_mux);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static ssize_t xr_gpios_ionum_write(struct file *filp,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf;
	u32 io_num;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	buf = kmalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	buf[count] = '\0';

	if (kstrtou32(buf, DECIMALISM, &io_num) != 0) {
		pr_err("Invalid integer io_num received\n");
		kfree(buf);
		return -EINVAL;
	}

	if (io_num >= pctrl->soc->ngpios) {
		pr_err("This io_num(%d) is invalid(max_io_num:%d)\n",
			io_num, pctrl->soc->ngpios);
		kfree(buf);
		return -EINVAL;
	}
	if (io_num >= XRSE_GPIO_MIN && io_num <= XRSE_GPIO_MAX) {
		pr_err("This io_num(%d) belongs to xrse[%d,%d], Retry it\n",
			io_num, XRSE_GPIO_MIN, XRSE_GPIO_MAX);
		kfree(buf);
		return -EINVAL;
	}

	current_gpio_num = io_num;
	kfree(buf);
	return count;
}

static ssize_t xr_gpios_ionum_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	int cnt;
	char buf[30];

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	cnt = snprintf(buf, sizeof(buf), "gpio_num=%d\n", current_gpio_num);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static ssize_t xr_gpios_info_read(struct file *filp,
		char __user *buffer, size_t count, loff_t *ppos)
{
	int cnt;
	char buf[50];
	struct gpio_chip *chip;
	u32 controller_id = 0;
	u32 offset = 0;
	struct xr_pinctrl *pctrl = gpio_dbg->xr_pctrl;

	if (count > DEBUGFS_INPUT_VAL_SIZE_MAX) {
		pr_err("Input val size(%zu) invalid!\n", count);
		return -EFAULT;
	}
	CONVERT_GPIO_AND_IP(current_gpio_num, controller_id, offset);

	if (controller_id >= pctrl->n_dwgpios || controller_id >= MAX_NR_GPIO_CHIP) {
		pr_err("convert contro_id(%d) invalid, max:n_dwgpios(%d) or MAXGPIO_CHIP(%d)\n",
			controller_id, pctrl->n_dwgpios, MAX_NR_GPIO_CHIP);
		cnt = snprintf(buf, sizeof(buf), "r_info fail,invalid contro_id\n");
		return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
	}
	chip = &gpio_dbg->xr_pctrl->dw_gpios[controller_id].gc;
	cnt = snprintf(buf, sizeof(buf), "This gpio=%d info showed at acpu console:\n",
		current_gpio_num);
	pr_info("\n\n");
	pr_info("---------------------------------------------------------------------------\n");
	pr_info(" PAD_GPIO_xxx drive pull_up pull_down slew schmitt func_name  gpio  other\n");
	pr_info("---------------------------------------------------------------------------\n");
	pr_info("\n");
	xr_gpio_dbg_show_one(chip, offset);
	return simple_read_from_buffer(buffer, count, ppos, buf, cnt);
}

static const struct file_operations ionum_fops = {
	.write = xr_gpios_ionum_write,
	.read = xr_gpios_ionum_read,
};

static const struct file_operations mux_fops = {
	.write = xr_gpios_mux_write,
	.read = xr_gpios_mux_read,
};

static const struct file_operations config_fops = {
	.write = xr_gpios_config_write,
	.read = xr_gpios_config_read,
};

static const struct file_operations dir_fops = {
	.write = xr_gpios_direction_write,
	.read = xr_gpios_direction_read,
};

static const struct file_operations val_fops = {
	.write = xr_gpios_val_write,
	.read = xr_gpios_val_read,
};

static const struct file_operations io_info_fops = {
	.read = xr_gpios_info_read,
};

int xr_gpio_debug_init(struct xr_pinctrl *pctrl)
{
	int ret;

	if (!pctrl) {
		pr_err("pctrl is null_ptr\n");
		return PTR_ERR(pctrl);
	}

	gpio_dbg = devm_kzalloc(pctrl->dev, sizeof(*gpio_dbg), GFP_KERNEL);
	if (!gpio_dbg)
		return -ENOMEM;

	gpio_dbg->xr_pctrl = pctrl;
	gpio_dbg->dev = pctrl->dev;

	gpio_dbg->dtb_info = devm_kzalloc(
		pctrl->dev, sizeof(struct one_gpio_info) * pctrl->soc->npins, GFP_KERNEL);
	if (!gpio_dbg->dtb_info)
		return -ENOMEM;

	gpio_dbg->dtb_node_cnt = pctrl->soc->npins;

	ret = xr_gpio_parse_dtb();
	if (ret < 0) {
		dev_err(pctrl->dev, "parse dtb failed\n");
		return -ENXIO;
	}

	gpio_dbg->root = debugfs_create_dir("xr_gpio_debug", NULL);
	if (!gpio_dbg->root) {
		dev_err(pctrl->dev, "create debugfs dir failed\n");
		return -ENOMEM;
	}

	debugfs_create_file("io_num", 0644, gpio_dbg->root, NULL, &ionum_fops);
	debugfs_create_file("mux", 0644, gpio_dbg->root, NULL, &mux_fops);
	debugfs_create_file("config", 0644, gpio_dbg->root, NULL, &config_fops);
	debugfs_create_file("direction", 0644, gpio_dbg->root, NULL, &dir_fops);
	debugfs_create_file("value", 0644, gpio_dbg->root, NULL, &val_fops);
	debugfs_create_file("io_info", 0444, gpio_dbg->root, NULL, &io_info_fops);

	debugfs_create_bool("sr_debug_en", 0644, gpio_dbg->root, &gpio_dbg->sr_debug_en);
	register_syscore_ops(&gpio_sr_debug_syscore_ops);
	return 0;
}

void xr_gpio_debug_exit(void)
{
	unregister_syscore_ops(&gpio_sr_debug_syscore_ops);

	if (!gpio_dbg)
		return;

	/* debugfs_remove_recursive(NULL) is safe and this check is probably not required */
	debugfs_remove_recursive(gpio_dbg->root);

	devm_kfree(gpio_dbg->dev, gpio_dbg->dtb_info);
	devm_kfree(gpio_dbg->dev, gpio_dbg);
}
