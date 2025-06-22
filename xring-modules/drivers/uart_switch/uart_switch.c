// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: uart switch driver
 */

#include <linux/debugfs.h>
#include <linux/dev_printk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/sys_ctrl_por_reg.h>

#include "uart_switch.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": %s:%d: " fmt, __func__, __LINE__

#define SYS_CTRL_POR_REG_USEC0	(ACPU_LMS_SYS_CTRL + SYS_CTRL_POR_REG_SC_REG_USEC0)

#define MAX_ARGS_LEN		64
#define MAX_CONFIG_INFO_LEN	128

struct iomux_channel_drv_cfg {
	void __iomem *rx_reg;
	void __iomem *tx_reg;
	u32 rx_addr;
	u32 tx_addr;
	u32 rx_val;
	u32 tx_val;
};

struct uart_switch_data {
	void __iomem		*sctrl_reg;
	void __iomem		*peri_ioctrl;
	void __iomem		*lpis_ioc;
	u32			switch_mode;
	u32			chan_num;
	u32			*is_secure_reg;
	char			**options;
	u32			*chans;
	u32			*sctrl_val;
	u32			*sctrl_mask;
	u32			*config_val;
	u32			option_count;
	char			**current_option;
	struct device		*dev;
	struct device_node	*np;
	struct iomux_channel_drv_cfg *iomux_drv_cfg;
};

static struct uart_switch_data *g_uart_switch_data;
static struct dentry *g_debugfs_file;

static int uart_switch_parse_dts(struct uart_switch_data *dev_data)
{
	int ret;
	int i;
	char *label;
	struct device *dev = dev_data->dev;
	struct device_node *np = dev_data->np;

	dev_data->sctrl_reg = ioremap(SYS_CTRL_POR_REG_USEC0, sizeof(u32));
	if (IS_ERR(dev_data->sctrl_reg)) {
		dev_err(dev, "Failed to ioremap sctrl_reg\n");
		return PTR_ERR(dev_data->sctrl_reg);
	}

	label = "switch_mode";
	ret = of_property_read_u32(np, label, &dev_data->switch_mode);
	if (ret) {
		dev_err(dev, "Failed to parse %s from dts\n", label);
		return ret;
	}
	if (dev_data->switch_mode == SWITCH_MODE_IOMUX) {
		dev_data->peri_ioctrl = ioremap(ACPU_PERI_IOCTRL, ACPU_PERI_IOCTRL_SIZE);
		if (IS_ERR(dev_data->peri_ioctrl)) {
			dev_err(dev, "Failed to ioremap peri_ioctrl\n");
			return PTR_ERR(dev_data->peri_ioctrl);
		}
		dev_data->lpis_ioc = ioremap(ACPU_LPIS_IOC, ACPU_LPIS_IOC_SIZE);
		if (IS_ERR(dev_data->lpis_ioc)) {
			dev_err(dev, "Failed to ioremap lpis_ioc\n");
			return PTR_ERR(dev_data->lpis_ioc);
		}
	}
	label = "uart_chan_num";
	ret = of_property_read_u32(np, label, &dev_data->chan_num);
	if (ret) {
		dev_err(dev, "Failed to parse %s from dts\n", label);
		return ret;
	}
	dev_data->current_option = devm_kzalloc(dev, sizeof(char *) * dev_data->chan_num,
						GFP_KERNEL);
	if (!dev_data->current_option)
		return -ENOMEM;
	dev_data->is_secure_reg = devm_kzalloc(dev, sizeof(u32) * dev_data->chan_num, GFP_KERNEL);
	if (!dev_data->is_secure_reg)
		return -ENOMEM;
	label = "is_secure_reg";
	for (i = 0; i < dev_data->chan_num; i++) {
		ret = of_property_read_u32_index(np, label, i, &dev_data->is_secure_reg[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	label = "uart_chans";
	ret = of_property_count_u32_elems(np, label);
	if (ret <= 0) {
		dev_err(dev, "Failed to find %s in dts\n", label);
		return -EINVAL;
	}
	dev_data->option_count = ret;
	dev_data->chans = devm_kzalloc(dev, sizeof(u32) * dev_data->option_count, GFP_KERNEL);
	if (!dev_data->chans)
		return -ENOMEM;
	for (i = 0; i < dev_data->option_count; i++) {
		ret = of_property_read_u32_index(np, label, i, &dev_data->chans[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	dev_data->options = devm_kzalloc(dev, sizeof(char *) * dev_data->option_count, GFP_KERNEL);
	if (!dev_data->options)
		return -ENOMEM;
	label = "uart_chan_options";
	for (i = 0; i < dev_data->option_count; i++) {
		ret = of_property_read_string_index(np, label, i,
						    (const char **)&dev_data->options[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	dev_data->sctrl_val = devm_kzalloc(dev, sizeof(u32) * dev_data->option_count, GFP_KERNEL);
	if (!dev_data->sctrl_val)
		return -ENOMEM;
	label = "sctrl_val";
	for (i = 0; i < dev_data->option_count; i++) {
		ret = of_property_read_u32_index(np, label, i, &dev_data->sctrl_val[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	dev_data->sctrl_mask = devm_kzalloc(dev, sizeof(u32) * dev_data->option_count, GFP_KERNEL);
	if (!dev_data->sctrl_mask)
		return -ENOMEM;
	label = "sctrl_mask";
	for (i = 0; i < dev_data->option_count; i++) {
		ret = of_property_read_u32_index(np, label, i, &dev_data->sctrl_mask[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	dev_data->config_val = devm_kzalloc(dev, sizeof(u32) * dev_data->option_count, GFP_KERNEL);
	if (!dev_data->config_val)
		return -ENOMEM;
	label = "config_val";
	for (i = 0; i < dev_data->option_count; i++) {
		ret = of_property_read_u32_index(np, label, i, &dev_data->config_val[i]);
		if (ret) {
			dev_err(dev, "Failed to parse %s index %d from dts\n", label, i);
			return ret;
		}
	}
	if (dev_data->switch_mode == SWITCH_MODE_IOMUX) {
		dev_data->iomux_drv_cfg = devm_kzalloc(dev,
			sizeof(struct iomux_channel_drv_cfg) * dev_data->chan_num, GFP_KERNEL);
		if (!dev_data->iomux_drv_cfg)
			return -ENOMEM;
		for (i = 0; i < dev_data->chan_num; i++) {
			ret = of_property_read_u32_index(np, "iomux_drv_cfg", 4 * i,
				&dev_data->iomux_drv_cfg[i].rx_addr);
			if (ret) {
				dev_err(dev, "Failed to parse iomux_drv_cfg %d\n", 4 * i);
				return ret;
			}
			if (dev_data->iomux_drv_cfg[i].rx_addr) {
				dev_data->iomux_drv_cfg[i].rx_reg =
					ioremap(dev_data->iomux_drv_cfg[i].rx_addr, sizeof(u32));
				if (IS_ERR(dev_data->iomux_drv_cfg[i].rx_reg)) {
					dev_err(dev, "Failed to ioremap chan %d rx_addr\n", i);
					return PTR_ERR(dev_data->iomux_drv_cfg[i].rx_reg);
				}
			}
			ret = of_property_read_u32_index(np, "iomux_drv_cfg", 4 * i + 1,
				&dev_data->iomux_drv_cfg[i].tx_addr);
			if (ret) {
				dev_err(dev, "Failed to parse iomux_drv_cfg %d\n", 4 * i + 1);
				return ret;
			}
			if (dev_data->iomux_drv_cfg[i].tx_addr) {
				dev_data->iomux_drv_cfg[i].tx_reg =
					ioremap(dev_data->iomux_drv_cfg[i].tx_addr, sizeof(u32));
				if (IS_ERR(dev_data->iomux_drv_cfg[i].tx_reg)) {
					dev_err(dev, "Failed to ioremap chan %d tx_addr\n", i);
					return PTR_ERR(dev_data->iomux_drv_cfg[i].tx_reg);
				}
			}
			ret = of_property_read_u32_index(np, "iomux_drv_cfg", 4 * i + 2,
				&dev_data->iomux_drv_cfg[i].rx_val);
			if (ret) {
				dev_err(dev, "Failed to parse iomux_drv_cfg %d\n", 4 * i + 2);
				return ret;
			}
			ret = of_property_read_u32_index(np, "iomux_drv_cfg", 4 * i + 3,
				&dev_data->iomux_drv_cfg[i].tx_val);
			if (ret) {
				dev_err(dev, "Failed to parse iomux_drv_cfg %d\n", 4 * i + 3);
				return ret;
			}
		}
	}

	return 0;
}

static void config_iomux_driver(u32 channel)
{
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;
	struct iomux_channel_drv_cfg *drv_cfg;

	if (dev_data->switch_mode != SWITCH_MODE_IOMUX)
		return;

	if (channel < 1 || channel > dev_data->chan_num) {
		dev_err(dev, "invalid channel %u\n", channel);
		return;
	}

	drv_cfg = &dev_data->iomux_drv_cfg[channel - 1];
	if (drv_cfg->rx_addr)
		writel(drv_cfg->rx_val, drv_cfg->rx_reg);

	if (drv_cfg->tx_addr)
		writel(drv_cfg->tx_val, drv_cfg->tx_reg);
}

static int uart_switch_set_secure_reg(u32 mode, u32 channel, u32 value)
{
	struct arm_smccc_res res = { 0 };
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;

	arm_smccc_smc(FID_BL31_UART_SWITCH, mode, channel, value,
		      0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		dev_err(dev, "%s failed\n", __func__);
		return -EFAULT;
	}

	return 0;
}

static int uart_switch_set_unsecure_reg(u32 mode, u32 channel, u32 value)
{
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;

	if (mode != SWITCH_MODE_IOMUX)
		return 0;

	dev_info(dev, "uart_switch_mode=%u, channel=%u\n", mode, channel);
	dev_info(dev, "value to be set: 0x%x\n", value);
	switch (channel) {
	case 2: /* channel2 */
		writel(value, (dev_data->peri_ioctrl + LPCTRL_PERI_IOCTRL_CHAN2_TX_OFFSET));
		writel(value, (dev_data->peri_ioctrl + LPCTRL_PERI_IOCTRL_CHAN2_RX_OFFSET));
		break;
	case 4: /* channel4 */
		writel(value, (dev_data->lpis_ioc + LPCTRL_LPIS_IOC_CHAN4_TX_OFFSET));
		writel(value, (dev_data->lpis_ioc + LPCTRL_LPIS_IOC_CHAN4_RX_OFFSET));
		break;
	default:
		dev_err(dev, "uart_channel (%u) invalid\n", channel);
		return -EINVAL;
	}
	return 0;
}

static void uart_switch_set_sctrl_reg(u32 channel, u32 index)
{
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;
	int i;
	u32 value;

	/* close other uarts in this channel */
	for (i = 0; i < dev_data->option_count; i++) {
		if ((dev_data->chans[i] == channel) && (i != index)) {
			value = SCTRL_ALL_UART_MASK | dev_data->sctrl_mask[i];
			writel(value, dev_data->sctrl_reg);
		}
	}

	/* set bit for this option */
	value = ((dev_data->sctrl_val[index]) | (dev_data->sctrl_mask[index]));
	dev_info(dev, "sctrl value to be set: 0x%x\n", value);
	writel(value, dev_data->sctrl_reg);
	dev_info(dev, "current sctrl value: 0x%x\n", readl(dev_data->sctrl_reg));
}

static int find_index_by_chan_and_option(u32 channel, char *option, u32 *index)
{
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;
	int i;

	for (i = 0; i < dev_data->option_count; i++) {
		if ((dev_data->chans[i] == channel) &&
			!strncmp(option, dev_data->options[i], strlen(dev_data->options[i]))) {
			*index = i;
			return 0;
		}
	}
	dev_info(dev, "%s: not found\n", __func__);
	return -EFAULT;
}

static ssize_t uart_switch_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;
	char current_config[MAX_CONFIG_INFO_LEN + 1] = { 0 };
	int ret;

	if (*ppos >= MAX_CONFIG_INFO_LEN) {
		dev_info(dev, "stop reading\n");
		return 0;
	}

	ret = snprintf(current_config, MAX_CONFIG_INFO_LEN,
		       "chan1:%s\nchan2:%s\nchan3:%s\nchan4:%s\n",
		       dev_data->current_option[0], dev_data->current_option[1],
		       dev_data->current_option[2], dev_data->current_option[3]);
	if (ret < 0) {
		dev_err(dev, "snprintf failed, ret = %d\n", ret);
		return 0;
	}
	count = strlen(current_config) + 1;
	ret = copy_to_user(buf, current_config, count);
	if (ret) {
		dev_err(dev, "copy_to_user failed, ret = %d\n", ret);
		/* set count to zero since copied error */
		return 0;
	}
	*ppos = MAX_CONFIG_INFO_LEN;
	return count;
}

static ssize_t uart_switch_write(struct file *file, const char __user *buf,
				 size_t count, loff_t *ppos)
{
	char args[MAX_ARGS_LEN + 1] = { 0 };
	char *arg1;
	char *arg2;
	struct uart_switch_data *dev_data = g_uart_switch_data;
	struct device *dev = dev_data->dev;
	int ret;
	u32 channel;
	u32 index;
	char *delim = " ";
	char *buffer;

	if (count > MAX_ARGS_LEN) {
		dev_err(dev, "count(%zu) out of range\n", count);
		return -EINVAL;
	}
	ret = copy_from_user(args, buf, count);
	if (ret) {
		dev_err(dev, "copy args from user failed, ret = %d\n", ret);
		return -EFAULT;
	}
	dev_info(dev, "args = %s\n", args);
	buffer = (char *)args;
	arg1 = strsep(&buffer, delim);
	if (!arg1 || strncmp(arg1, "channel", strlen("channel"))) {
		dev_err(dev, "arg1 is invalid\n");
		return -EINVAL;
	}
	arg2 = strsep(&buffer, delim);
	if (!arg2) {
		dev_err(dev, "arg2 is invalid\n");
		return -EINVAL;
	}
	arg1 += strlen("channel");
	ret = kstrtouint(arg1, 10, &channel);
	if (ret) {
		dev_err(dev, "kstrtouint failed, ret = %d\n", ret);
		return -EFAULT;
	}
	if ((channel < 1) || (channel > dev_data->chan_num)) {
		dev_err(dev, "channel(%d) out of range", channel);
		return -EINVAL;
	}
	if (dev_data->current_option[channel - 1] &&
		!strncmp(arg2, dev_data->current_option[channel - 1],
			 strlen(dev_data->current_option[channel - 1]))) {
		dev_info(dev, "channel%d is already %s\n", channel, arg2);
		return count;
	}
	ret = find_index_by_chan_and_option(channel, arg2, &index);
	if (ret) {
		dev_err(dev, "Invalid argument\n");
		return -EFAULT;
	}
	if (dev_data->is_secure_reg[channel - 1])
		ret = uart_switch_set_secure_reg(dev_data->switch_mode, channel,
						 dev_data->config_val[index]);
	else
		ret = uart_switch_set_unsecure_reg(dev_data->switch_mode, channel,
						   dev_data->config_val[index]);
	if (ret)
		return ret;
	config_iomux_driver(channel);
	uart_switch_set_sctrl_reg(channel, index);
	dev_data->current_option[channel - 1] = dev_data->options[index];

	return count;
}

static const struct file_operations uart_switch_fops = {
	.owner   = THIS_MODULE,
	.read    = uart_switch_read,
	.write   = uart_switch_write,
	.open    = simple_open,
};

static int uart_switch_init_debugfs(struct uart_switch_data *dev_data)
{
	struct device *dev = dev_data->dev;

	g_debugfs_file = debugfs_create_file("uart_switch", 0644,
					     NULL, NULL, &uart_switch_fops);
	if (!g_debugfs_file) {
		dev_err(dev, "Failed to create /sys/kernel/debug/uart_switch\n");
		return -EFAULT;
	}

	return 0;
}

static int uart_switch_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct uart_switch_data *dev_data = NULL;

	dev_info(dev, "uart_switch driver probe\n");

	dev_data = devm_kzalloc(dev, sizeof(struct uart_switch_data), GFP_KERNEL);
	if (!dev_data)
		return -ENOMEM;
	dev_info(dev, "success to allocate dev_data\n");

	dev_data->dev = dev;
	dev_data->np = np;
	g_uart_switch_data = dev_data;

	ret = uart_switch_parse_dts(dev_data);
	if (ret)
		return ret;

	ret = uart_switch_init_debugfs(dev_data);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, dev_data);

	return 0;
}

static int uart_switch_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "uart_switch driver remove\n");

	debugfs_remove(g_debugfs_file);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int uart_switch_suspend(struct device *dev)
{
	dev_info(dev, "uart_switch driver suspend\n");

	return 0;
}

static int uart_switch_resume(struct device *dev)
{
	struct uart_switch_data *data = dev_get_drvdata(dev);
	int i;
	int ret;
	u32 index;

	dev_info(dev, "uart_switch driver resume++\n");
	for (i = 0; i < data->chan_num; i++) {
		if (!data->current_option[i])
			continue;
		dev_info(dev, "channel %d option %s\n",
			 i + 1, data->current_option[i]);
		ret = find_index_by_chan_and_option(
			i + 1, data->current_option[i], &index);
		if (ret) {
			dev_err(dev, "find index by chan(%d) option(%s) failed\n",
				i + 1, data->current_option[i]);
			continue;
		}
		if (data->is_secure_reg[i])
			ret = uart_switch_set_secure_reg(data->switch_mode, i + 1,
							 data->config_val[index]);
		else
			ret = uart_switch_set_unsecure_reg(data->switch_mode, i + 1,
							   data->config_val[index]);
		if (ret) {
			dev_err(dev, "set reg for chan(%d) option(%s) failed\n",
				i + 1, data->current_option[i]);
			continue;
		}
		config_iomux_driver(i + 1);
		uart_switch_set_sctrl_reg(i + 1, index);
	}
	dev_info(dev, "uart_switch driver resume--\n");

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops uart_switch_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(uart_switch_suspend, uart_switch_resume)
};

static const struct of_device_id uart_switch_of_match[] = {
	{ .compatible = "xring,uart-switch", },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, uart_switch_of_match);

static struct platform_driver uart_switch_driver = {
	.probe = uart_switch_probe,
	.remove = uart_switch_remove,
	.driver = {
		.name = "uart_switch",
		.of_match_table = of_match_ptr(uart_switch_of_match),
		.pm = &uart_switch_pm_ops,
	},
};

module_platform_driver(uart_switch_driver);

MODULE_DESCRIPTION("Xring Uart Switch Driver");
MODULE_LICENSE("GPL v2");
