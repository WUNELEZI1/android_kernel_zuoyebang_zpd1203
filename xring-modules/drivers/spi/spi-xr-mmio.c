// SPDX-License-Identifier: GPL-2.0-only
/*
 * XRing SPI adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/acpi.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/reset.h>

#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>

#include "spi-xr.h"

#define DRIVER_NAME "xr_spi_mmio"

static int dw_spi_dw_apb_init(struct platform_device *pdev,
			      struct dw_spi_mmio *dwsmmio)
{
	dw_spi_dma_setup_generic(&dwsmmio->dws);

	return 0;
}

static int spi_dma_init(struct platform_device *pdev,
			    struct dw_spi_mmio *dwsmmio)
{
	xr_spi_dma_setup_generic(&dwsmmio->dws);
	dwsmmio->dws.is_spi_dma = true;
	return 0;
}

static int dw_spi_suspend(struct device *dev, struct dw_spi *dws);
static int dw_spi_resume(struct device *dev, struct dw_spi *dws);
static int dw_spi_runtime_suspend(struct device *dev, struct dw_spi *dws);
static int dw_spi_runtime_resume(struct device *dev, struct dw_spi *dws);
static int spi_dma_suspend(struct device *dev, struct dw_spi *dws);
static int spi_dma_resume(struct device *dev, struct dw_spi *dws);
static int spi_dma_runtime_suspend(struct device *dev, struct dw_spi *dws);
static int spi_dma_runtime_resume(struct device *dev, struct dw_spi *dws);

static int spi_dma_set_qos(struct dw_spi *dws)
{
	struct sys_qos_map *sys_qos_table_dma = NULL;
	int default_val = 0;
	int i;
	int mask;

	sys_qos_table_dma = xring_flowctrl_get_cfg_table(SYS_QOS_TABLE_ID);
	if (!sys_qos_table_dma) {
		dev_err(dws->dev, "get flowctrl cfg_table failed\n");
		return -EINVAL;
	}

	for (i = 0; i < SYS_QOS_REG_NUM; i++) {
		if (sys_qos_table_dma[i].mst_id == QOS_SPI_DMA_ID) {
			mask = (1 << (sys_qos_table_dma[i].end_bit -
				   sys_qos_table_dma[i].start_bit + 1)) - 1;
			default_val = dw_readl(dws, sys_qos_table_dma[i].offset);
			default_val &= ~(mask << sys_qos_table_dma[i].start_bit);
			default_val |= sys_qos_table_dma[i].val << sys_qos_table_dma[i].start_bit;
			dw_writel(dws, sys_qos_table_dma[i].offset, default_val);
			break;
		}
	}

	if (i < SYS_QOS_REG_NUM)
		return 0;
	else
		return -EINVAL;
}

static int dw_spi_pinctrl_init(struct device *dev, struct dw_spi *dws)
{
	int ret;

	dws->spi_pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(dws->spi_pinctrl)) {
		dev_err(dev, "No pinctrl config specified!\n");
		return PTR_ERR(dws->spi_pinctrl);
	}

	dws->pinctrl_state_default =
		pinctrl_lookup_state(dws->spi_pinctrl, "default");
	if (IS_ERR_OR_NULL(dws->pinctrl_state_default)) {
		dev_err(dev, "No default config specified!\n");
		return PTR_ERR(dws->pinctrl_state_default);
	}

	dws->pinctrl_state_sleep =
		pinctrl_lookup_state(dws->spi_pinctrl, "sleep");
	if (IS_ERR_OR_NULL(dws->pinctrl_state_sleep)) {
		dev_err(dev, "No sleep config specified!\n");
		return PTR_ERR(dws->pinctrl_state_sleep);
	}

	ret = pinctrl_select_state(dws->spi_pinctrl,
				dws->pinctrl_state_sleep);
	if (ret) {
		dev_err(dev, "failed to set sleep configuration\n");
		return ret;
	}

	return 0;
}

static void spi_low_power_ops_init(struct dw_spi *dws)
{
	if (dws->is_spi_dma) {
		dws->resume = spi_dma_resume;
		dws->suspend = spi_dma_suspend;
		dws->runtime_resume = spi_dma_runtime_resume;
		dws->runtime_suspend = spi_dma_runtime_suspend;
	} else {
		dws->resume = dw_spi_resume;
		dws->suspend = dw_spi_suspend;
		dws->runtime_resume = dw_spi_runtime_resume;
		dws->runtime_suspend = dw_spi_runtime_suspend;
	}
}

static int dw_spi_mmio_probe(struct platform_device *pdev)
{
	int (*init_func)(struct platform_device *pdev,
			 struct dw_spi_mmio *dwsmmio);
	struct dw_spi_mmio *dwsmmio = NULL;
	struct resource *mem = NULL;
	struct dw_spi *dws = NULL;
	int ret;
	int num_cs;

	dwsmmio = devm_kzalloc(&pdev->dev, sizeof(struct dw_spi_mmio),
			GFP_KERNEL);
	if (!dwsmmio)
		return -ENOMEM;

	dws = &dwsmmio->dws;

	/* Get basic io resource and map it */
	dws->regs = devm_platform_get_and_ioremap_resource(pdev, 0, &mem);
	if (IS_ERR(dws->regs))
		return PTR_ERR(dws->regs);

	dws->paddr = mem->start;
	dws->dev = &pdev->dev;
	dws->irq = platform_get_irq(pdev, 0);
	if (dws->irq < 0) {
		dev_err(&pdev->dev, "failed to get SPI IRQ\n");
		return dws->irq;
	}

	dws->is_fpga = device_property_read_bool(&pdev->dev, "fpga");
	if (!dws->is_fpga) {
		ret = dw_spi_pinctrl_init(&pdev->dev, dws);
		if (ret) {
			dev_err(&pdev->dev, "failed to init spi pinctrl\n");
			return ret;
		}
	}

	dwsmmio->clk_div = devm_clk_get(&pdev->dev, "clk_div");
	if (IS_ERR(dwsmmio->clk_div)) {
		dev_err(&pdev->dev, "failed to get spi clk_div\n");
		return PTR_ERR(dwsmmio->clk_div);
	}

	ret = device_property_read_u32(&pdev->dev, "crg-div-max", &dws->crg_div_max);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to get spi crg-div-max\n");
		return ret;
	}

	ret = device_property_read_u32(&pdev->dev, "crg-div-min", &dws->crg_div_min);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to get spi crg-div-min\n");
		return ret;
	}

	dwsmmio->clk = devm_clk_get(&pdev->dev, "clk");
	if (IS_ERR(dwsmmio->clk)) {
		dev_err(&pdev->dev, "failed to get spi clk\n");
		return PTR_ERR(dwsmmio->clk);
	}

	ret = clk_prepare_enable(dwsmmio->clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable spi clk\n");
		return ret;
	}

	dwsmmio->pclk = devm_clk_get(&pdev->dev, "pclk");
	if (IS_ERR(dwsmmio->pclk)) {
		dev_err(&pdev->dev, "failed to get spi pclk\n");
		ret = PTR_ERR(dwsmmio->pclk);
		goto err_spi_no_pclk;
	}

	ret = clk_prepare_enable(dwsmmio->pclk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable spi pclk\n");
		goto err_spi_pclk_en;
	}

	dwsmmio->rstc = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(dwsmmio->rstc)) {
		dev_err(&pdev->dev, "failed to get spi rstc\n");
		ret = PTR_ERR(dwsmmio->rstc);
		goto err_spi_no_rstc;
	}

	ret = reset_control_deassert(dwsmmio->rstc);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed deassert rst control\n");
		goto err_spi_rstc_deassert;
	}

	dwsmmio->prstc = devm_reset_control_get_exclusive(&pdev->dev, "prst");
	if (IS_ERR(dwsmmio->prstc)) {
		dev_err(&pdev->dev, "failed to get spi prstc\n");
		ret = PTR_ERR(dwsmmio->prstc);
		goto err_spi_no_prstc;
	}

	ret = reset_control_deassert(dwsmmio->prstc);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed deassert prst control\n");
		goto err_spi_prstc_deassert;
	}

	dws->bus_num = pdev->id;

	ret = device_property_read_u32(&pdev->dev, "reg-io-width", &dws->reg_io_width);
	if (ret < 0)
		dws->reg_io_width = 4;

	ret = device_property_read_u32(&pdev->dev, "num-cs", &num_cs);
	if (ret < 0)
		num_cs = 4;
	dws->num_cs = num_cs;

	ret = device_property_read_u32(&pdev->dev, "hardware-lock", &dws->hardware_lock);
	if (!ret) {
		dws->spi_hwspin_lock = hwspin_lock_request_specific(
								dws->hardware_lock);
		if (!dws->spi_hwspin_lock) {
			dev_err(dws->dev, "spi_hwspin_lock request error\n");
			ret = -EBUSY;
			goto err_hwspin_lock_request;
		}
	}

	init_func = device_get_match_data(&pdev->dev);
	if (init_func) {
		ret = init_func(pdev, dwsmmio);
		if (ret)
			goto out;
	}

	if (dws->is_spi_dma) {
		ret = spi_dma_set_qos(dws);
		if (ret) {
			dev_err(&pdev->dev, "failed to set spi_dma qos\n");
			goto out;
		}

		dws->dma_complete_irq = platform_get_irq(pdev, 1);
		if (dws->dma_complete_irq < 0) {
			dev_err(&pdev->dev, "failed to get DMA IRQ\n");
			ret = dws->dma_complete_irq;
			goto out;
		}

		ret = device_property_read_u32(&pdev->dev, "dma-select", &dws->dma_type);
		if (ret < 0)
			dws->dma_type = 0;
	}

	spin_lock_init(&dws->lock);
	platform_set_drvdata(pdev, dwsmmio);
	spi_low_power_ops_init(dws);
	if (dws->is_spi_dma)
		ret = xr_spi_add_host(&pdev->dev, dws);
	else
		ret = dw_spi_add_host(&pdev->dev, dws);
	if (ret)
		goto out;
	clk_disable(dwsmmio->pclk);
	clk_disable(dwsmmio->clk);
	return 0;

out:
	if (dws->spi_hwspin_lock)
		hwspin_lock_free(dws->spi_hwspin_lock);
err_hwspin_lock_request:
	reset_control_assert(dwsmmio->prstc);
err_spi_prstc_deassert:
err_spi_no_prstc:
	reset_control_assert(dwsmmio->rstc);
err_spi_rstc_deassert:
err_spi_no_rstc:
	clk_disable(dwsmmio->pclk);
err_spi_pclk_en:
err_spi_no_pclk:
	clk_disable_unprepare(dwsmmio->clk);

	return ret;
}

static int dw_spi_mmio_remove(struct platform_device *pdev)
{
	struct dw_spi_mmio *dwsmmio = platform_get_drvdata(pdev);
	int ret;

	if (dwsmmio->dws.spi_hwspin_lock)
		hwspin_lock_free(dwsmmio->dws.spi_hwspin_lock);

	ret = clk_enable(dwsmmio->pclk);
	if (ret < 0) {
		dev_err(dwsmmio->dws.dev, "failed to enable spi_pclk (%d)\n", ret);
		return ret;
	}
	ret = clk_enable(dwsmmio->clk);
	if (ret < 0) {
		clk_disable(dwsmmio->pclk);
		dev_err(dwsmmio->dws.dev, "failed to enable spi_clk (%d)\n", ret);
		return ret;
	}

	dw_spi_remove_host(&dwsmmio->dws);
	clk_disable_unprepare(dwsmmio->pclk);
	clk_disable_unprepare(dwsmmio->clk);
	reset_control_assert(dwsmmio->rstc);
	reset_control_assert(dwsmmio->prstc);

	return 0;
}

static int dw_spi_suspend(struct device *dev, struct dw_spi *dws)
{
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	ret = spi_controller_suspend(dws->master);
	if (ret) {
		dev_err(dev, "cannot suspend master\n");
		return ret;
	}

	dws->cur_rx_sample_dly = 0;
	dws->current_freq = 0;

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static int dw_spi_resume(struct device *dev, struct dw_spi *dws)
{
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	ret = reset_control_deassert(dwsmmio->rstc);
	if (ret < 0) {
		dev_err(dev, "failed to reset_control_deassert spi_rstc\n");
		return ret;
	}

	ret = reset_control_deassert(dwsmmio->prstc);
	if (ret < 0) {
		dev_err(dev, "failed to reset_control_deassert spi_prstc\n");
		reset_control_assert(dwsmmio->rstc);
		return ret;
	}

	ret = clk_enable(dwsmmio->pclk);
	if (ret < 0) {
		dev_err(dws->dev, "failed to enable spi_pclk (%d)\n", ret);
		return ret;
	}

	ret = clk_enable(dwsmmio->clk);
	if (ret < 0) {
		clk_disable(dwsmmio->pclk);
		dev_err(dws->dev, "failed to enable spi_clk (%d)\n", ret);
		return ret;
	}

	spi_hw_init(dev, dws);
	dw_writel(dws, DW_SPI_DMARDLR, dws->rxburst - 1);
	dw_writel(dws, DW_SPI_DMATDLR, dws->txburst);

	clk_disable(dwsmmio->pclk);
	clk_disable(dwsmmio->clk);

	ret = spi_controller_resume(dws->master);
	if (ret)
		dev_err(dev, "problem starting queue (%d)\n", ret);

	dev_info(dev, "%s --\n", __func__);

	return ret;
}

static int dw_spi_runtime_suspend(struct device *dev, struct dw_spi *dws)
{
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&dws->lock, flags);
	dws->transfer_state = SPI_TRANSFER_COMPLETE;
	clk_disable(dwsmmio->pclk);
	clk_disable(dwsmmio->clk);
	spin_unlock_irqrestore(&dws->lock, flags);

	ret = pinctrl_select_state(dws->spi_pinctrl, dws->pinctrl_state_sleep);
	if (ret) {
		dev_err(dev, "failed to set sleep configuration\n");
		return ret;
	}

	return 0;
}

static int dw_spi_runtime_resume(struct device *dev, struct dw_spi *dws)
{
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret = 0;

	ret = pinctrl_select_state(dws->spi_pinctrl, dws->pinctrl_state_default);
	if (ret) {
		dev_err(&dws->master->dev, "failed to set active configuration\n");
		return ret;
	}

	ret = clk_enable(dwsmmio->pclk);
	if (ret < 0) {
		dev_err(&dws->master->dev, "failed to enable spi_pclk (%d)\n", ret);
		return ret;
	}

	ret = clk_enable(dwsmmio->clk);
	if (ret < 0) {
		clk_disable(dwsmmio->pclk);
		dev_err(&dws->master->dev, "failed to enable spi_clk (%d)\n", ret);
		return ret;
	}

	dws->transfer_state = SPI_TRANSFER_PREPARED;

	return 0;
}

static int spi_dma_suspend(struct device *dev, struct dw_spi *dws)
{
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	ret = spi_controller_suspend(dws->master);
	if (ret) {
		dev_err(dev, "cannot suspend master\n");
		return ret;
	}

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static int spi_dma_resume(struct device *dev, struct dw_spi *dws)
{
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	ret = spi_controller_resume(dws->master);
	if (ret)
		dev_err(dev, "problem starting queue (%d)\n", ret);

	dev_info(dev, "%s --\n", __func__);

	return ret;
}

static int pinctrl_select_spi_dma(struct device *dev,
				enum spi_pinctrl_state pinctrl_state)
{
	struct arm_smccc_res res;

	memset(&res, 0, sizeof(res));
	arm_smccc_smc(FID_SPI_MISC_PINCTRL, pinctrl_state,
				0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		dev_err(dev, "failed to select pinctrl state: %d via smc!\n",
			pinctrl_state);
		return res.a0;
	}

	return 0;
}

static int spi_dma_runtime_suspend(struct device *dev, struct dw_spi *dws)
{
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&dws->lock, flags);
	dws->transfer_state = SPI_TRANSFER_COMPLETE;
	clk_disable(dwsmmio->pclk);
	clk_disable(dwsmmio->clk);
	spin_unlock_irqrestore(&dws->lock, flags);

	ret = pinctrl_select_spi_dma(dev, PINCTRL_SLEEP);
	if (ret)
		return ret;
	ret = gpiod_direction_input(dws->master->cs_gpiods[0]);
	if (ret)
		return ret;

	return 0;
}

static int spi_dma_runtime_resume(struct device *dev, struct dw_spi *dws)
{
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret;

	ret = gpiod_direction_output(dws->master->cs_gpiods[0], 0);
	if (ret)
		return ret;

	ret = pinctrl_select_spi_dma(dev, PINCTRL_DEFAULT);
	if (ret)
		return ret;

	ret = clk_enable(dwsmmio->pclk);
	if (ret < 0) {
		dev_err(dev, "failed to enable spi_pclk (%d)\n", ret);
		return ret;
	}
	ret = clk_enable(dwsmmio->clk);
	if (ret < 0) {
		clk_disable(dwsmmio->pclk);
		dev_err(dev, "failed to enable spi_clk (%d)\n", ret);
		return ret;
	}

	dws->transfer_state = SPI_TRANSFER_PREPARED;

	return 0;
}

int xr_tui_resource_release(struct spi_device *spidev)
{
	struct dw_spi *dws = spi_master_get_devdata(spidev->master);
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret = 0;

	clk_disable(dwsmmio->pclk);
	clk_disable(dwsmmio->clk);

	ret = pinctrl_select_state(dws->spi_pinctrl,
			dws->pinctrl_state_sleep);
	if (ret) {
		dev_err(&dws->master->dev, "failed to set sleep configuration\n");
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL(xr_tui_resource_release);

int xr_tui_resource_request(struct spi_device *spidev)
{
	struct dw_spi *dws = spi_master_get_devdata(spidev->master);
	struct dw_spi_mmio *dwsmmio = container_of(dws, struct dw_spi_mmio, dws);
	int ret = 0;

	ret = pinctrl_select_state(dws->spi_pinctrl,
				dws->pinctrl_state_default);
	if (ret) {
		dev_err(&dws->master->dev, "failed to set active configuration\n");
		return ret;
	}

	ret = clk_enable(dwsmmio->pclk);
	if (ret < 0) {
		dev_err(dws->dev, "failed to enable spi_pclk (%d)\n", ret);
		return ret;
	}

	ret = clk_enable(dwsmmio->clk);
	if (ret < 0) {
		dev_err(dws->dev, "failed to enable spi_clk (%d)\n", ret);
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(xr_tui_resource_request);

static int spi_resume(struct device *dev)
{
	struct dw_spi *dws = dev_get_drvdata(dev);

	return dws->resume(dev, dws);
}

static int spi_suspend(struct device *dev)
{
	struct dw_spi *dws = dev_get_drvdata(dev);

	return dws->suspend(dev, dws);
}

int spi_runtime_resume(struct device *dev)
{
	struct dw_spi *dws = dev_get_drvdata(dev);

	return dws->runtime_resume(dev, dws);
}

int spi_runtime_suspend(struct device *dev)
{
	struct dw_spi *dws = dev_get_drvdata(dev);

	return dws->runtime_suspend(dev, dws);
}

static const struct dev_pm_ops dw_spi_pm_ops = {
	NOIRQ_SYSTEM_SLEEP_PM_OPS(spi_suspend, spi_resume)
};

static const struct of_device_id xr_spi_mmio_of_match[] = {
	{ .compatible = "xring,dw-apb-ssi", .data = spi_dma_init},
	{ .compatible = "snps,dw-apb-ssi", .data = dw_spi_dw_apb_init},
	{ /* end of table */}
};
MODULE_DEVICE_TABLE(of, xr_spi_mmio_of_match);

static struct platform_driver xr_spi_mmio_driver = {
	.probe		= dw_spi_mmio_probe,
	.remove		= dw_spi_mmio_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.of_match_table = xr_spi_mmio_of_match,
		.pm     = &dw_spi_pm_ops,
	},
};
module_platform_driver(xr_spi_mmio_driver);

MODULE_DESCRIPTION("Memory-mapped I/O interface driver for DW SPI Core");
MODULE_LICENSE("GPL v2");
