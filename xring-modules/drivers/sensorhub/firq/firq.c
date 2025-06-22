// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/workqueue.h>
#include <linux/devm-helpers.h>
#include <asm/io.h>
#include "firq.h"
#include <soc/xring/sensorhub/shub_boot_prepare.h>

#define BMRW_REGISTER_SHIFT 16

struct firq_dev_s {
	int channel;
	uint32_t data_length;
	uint32_t data_addr;
	firq_recv_callback_t recv_callback;
	struct work_struct work;
};

struct firq_dev_chan {
	int channel_num;
	uint32_t firq_trigger_reg;
	uint32_t firq_status_reg;
	uint32_t firq_mask_reg;
	uint32_t firq_data_length_reg;
	uint32_t firq_data_addr_reg;
	void __iomem	*base;
	struct firq_dev_s *devs;
	struct workqueue_struct *wq;
};

static struct firq_dev_chan *g_firq_dev_chans;

static inline void firq_intr_clear(int chan)
{
	u32 reg = (1 << (chan + BMRW_REGISTER_SHIFT));

	writel(reg, g_firq_dev_chans->base + g_firq_dev_chans->firq_trigger_reg);
}

static inline uint32_t firq_data_length(int chan)
{
	u32 reg_length = readl(g_firq_dev_chans->base + g_firq_dev_chans->firq_data_length_reg + (chan * 0x4));
	return reg_length;
}

static inline uint32_t firq_data_addr(int chan)
{
	u32 reg_data_addr = readl(g_firq_dev_chans->base + g_firq_dev_chans->firq_data_addr_reg + (chan * 0x4));
	return reg_data_addr;
}

static void firq_recv_message_worker(struct work_struct *work)
{
	struct firq_dev_s *firq_dev = container_of(work, struct firq_dev_s, work);

	if (firq_dev->recv_callback != NULL)
		firq_dev->recv_callback(firq_dev->data_addr, firq_dev->data_length);
}

static irqreturn_t firq_recv_data_isr(int irq, void *devid)
{
	struct firq_dev_s *firq_dev = (struct firq_dev_s *)devid;

	firq_intr_clear(firq_dev->channel);
	firq_dev->data_addr = firq_data_addr(firq_dev->channel);
	firq_dev->data_length = firq_data_length(firq_dev->channel);
	queue_work(g_firq_dev_chans->wq, &firq_dev->work);

	return IRQ_HANDLED;
}

static int firq_dev_chans_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;

	g_firq_dev_chans = devm_kzalloc(dev, sizeof(*g_firq_dev_chans), GFP_KERNEL);
	if (!g_firq_dev_chans)
		return -ENOMEM;

	ret = of_property_read_u32(node, "firq_channel", &g_firq_dev_chans->channel_num);
	if (ret) {
		pr_err("unable to get firq_channel\n");
		return ret;
	}

	ret = of_property_read_u32(node, "reg_trigger", &g_firq_dev_chans->firq_trigger_reg);
	if (ret) {
		pr_err("unable to find reg_trigger\n");
		return ret;
	}

	ret = of_property_read_u32(node, "reg_mask", &g_firq_dev_chans->firq_mask_reg);
	if (ret) {
		pr_err("unable to find reg_mask\n");
		return ret;
	}

	ret = of_property_read_u32(node, "reg_status", &g_firq_dev_chans->firq_status_reg);
	if (ret) {
		pr_err("unable to find reg_status\n");
		return ret;
	}

	ret = of_property_read_u32(node, "reg_data_length", &g_firq_dev_chans->firq_data_length_reg);
	if (ret) {
		pr_err("unable to find data_length_offset\n");
		return ret;
	}

	ret = of_property_read_u32(node, "reg_data_addr", &g_firq_dev_chans->firq_data_addr_reg);
	if (ret) {
		pr_err("unable to find data_addr_offset\n");
		return ret;
	}

	g_firq_dev_chans->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(g_firq_dev_chans->base))
		return PTR_ERR(g_firq_dev_chans->base);

	g_firq_dev_chans->wq = create_workqueue("firq_rx_wq");
	if (!g_firq_dev_chans->wq) {
		pr_err("create firq_rx_wq failed\n");
		return -ENOMEM;
	}

	g_firq_dev_chans->devs = NULL;
	return 0;
}

static int firq_dev_init(struct platform_device *pdev)
{
	int ret = 0;
	int irq;
	struct device *dev = &pdev->dev;
	struct firq_dev_s *firq_devs;

	firq_devs = devm_kcalloc(dev, g_firq_dev_chans->channel_num, sizeof(*firq_devs), GFP_KERNEL);
	if (!firq_devs)
		return -ENOMEM;

	g_firq_dev_chans->devs = firq_devs;

	for (int chan = 0; chan < g_firq_dev_chans->channel_num; chan++) {
		firq_devs[chan].channel = chan;

		irq = platform_get_irq_optional(pdev, chan);
		if (irq > 0) {
			INIT_WORK(&firq_devs[chan].work, firq_recv_message_worker);
			ret = devm_request_irq(dev, irq, firq_recv_data_isr,
								   0, "firq_isr", &firq_devs[chan]);
			if (ret) {
				pr_err("fail to request irq[%d]\n", irq);
				return ret;
			}
		} else {
			pr_err("platform get irq <= 0\n");
			return -EFAULT;
		}
	}

	return ret;
}

int firq_recv_cb_register(int chan, firq_recv_callback_t recv_callback)
{
	int chan_num = g_firq_dev_chans->channel_num;

	if (chan < 0 || chan >= chan_num) {
		pr_err("chan number[%d] is invalid\n", chan);
		return -ERANGE;
	}

	if (recv_callback == NULL) {
		pr_err("recv_callback is null\n");
		return -EINVAL;
	}

	if (g_firq_dev_chans->devs[chan].recv_callback != NULL)
		pr_info("This channel[%d] has already registered callback function\n", chan);

	g_firq_dev_chans->devs[chan].recv_callback = recv_callback;
	return 0;
}
EXPORT_SYMBOL(firq_recv_cb_register);

int firq_recv_cb_unregister(int chan)
{
	int chan_num = g_firq_dev_chans->channel_num;

	if (chan < 0 || chan >= chan_num) {
		pr_err("chan number[%d] is invalid\n", chan);
		return -ERANGE;
	}

	g_firq_dev_chans->devs[chan].recv_callback = NULL;
	return 0;
}
EXPORT_SYMBOL(firq_recv_cb_unregister);

static int firq_drv_probe(struct platform_device *pdev)
{
	int ret;

	pr_info("firq probe start!\n");
	if (!is_shub_dts_enable()) {
		pr_err("firq: sensorhub dts not okay\n");
		return -EOPNOTSUPP;
	}

	ret = firq_dev_chans_init(pdev);
	if (ret) {
		pr_err("firq_dev_chans_init failed!\n");
		return ret;
	}

	ret = firq_dev_init(pdev);
	if (ret) {
		pr_err("firq_dev_init failed!\n");
		goto err_out;
	}

	pr_info("firq probe done!\n");
	return 0;

err_out:
	destroy_workqueue(g_firq_dev_chans->wq);
	pr_err("firq probe failed!\n");
	return ret;
}


static int firq_drv_remove(struct platform_device *pdev)
{
	destroy_workqueue(g_firq_dev_chans->wq);
	g_firq_dev_chans = NULL;
	pr_info("firq removed!\n");
	return 0;
}

static const struct of_device_id firq_of_match[] = {
	{ .compatible = "xring,firq" },
	{}
};
MODULE_DEVICE_TABLE(of, firq_of_match);

static struct platform_driver firq_driver = {
	.probe = firq_drv_probe,
	.remove = firq_drv_remove,
	.driver = {
		.name = "firq",
		.of_match_table =
		of_match_ptr(firq_of_match),
	},
};
module_platform_driver(firq_driver);

MODULE_DESCRIPTION("FIRQ driver");
MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_LICENSE("GPL v2");
