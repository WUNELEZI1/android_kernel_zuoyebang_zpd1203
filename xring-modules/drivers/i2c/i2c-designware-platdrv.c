// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * XRing I2C adapter driver.
 *
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/units.h>

#include "i2c-designware-core.h"

#define PM_RUNTIME_AUTO_SUSPEND_DELAY	100	/* ms */

static u32 i2c_dw_get_clk_rate_khz(struct dw_i2c_dev *dev)
{
	return clk_get_rate(dev->clk) / KILO;
}

static const struct of_device_id dw_i2c_of_match[] = {
	{ .compatible = "snps,designware-i2c", },
	{},
};
MODULE_DEVICE_TABLE(of, dw_i2c_of_match);

static int dw_i2c_plat_request_regs(struct dw_i2c_dev *dev)
{
	struct platform_device *pdev = to_platform_device(dev->dev);
	int ret;

#if defined CONFIG_XRING_I2C_DMA
	struct resource *mem = NULL;

	dev->base = devm_platform_get_and_ioremap_resource(pdev, 0, &mem);
	ret = IS_ERR(dev->base);
	if (ret)
		return ret;

	dev->ip_paddr = mem->start;
	dev->fifo_dma_addr = (dma_addr_t)(dev->ip_paddr + DW_IC_DATA_CMD);
#else
	dev->base = devm_platform_ioremap_resource(pdev, 0);
	ret = PTR_ERR_OR_ZERO(dev->base);
#endif

	return ret;
}

static void i2c_parse_timing(struct device *dev, u32 *scl_cur_val_p,
		u32 *sda_cur_val_p, struct xr_i2c_timing_info *timing_info)
{
	int ret;
	u32 val[XR_SUPPORT_SPEED_MODE] = {0}; /* 4 modes: 100KHz, 400KHz, 1MHz, 3.4MHz */

	ret = device_property_read_u32_array(dev, "i2c-scl-falling-time-ns",
					val, XR_SUPPORT_SPEED_MODE);
	if (ret && timing_info->using_default)
		*scl_cur_val_p = timing_info->scl_def_val;
	else
		*scl_cur_val_p = val[timing_info->speed_mode];

	ret = device_property_read_u32_array(dev, "i2c-sda-falling-time-ns",
					val, XR_SUPPORT_SPEED_MODE);
	if (ret && timing_info->using_default)
		*sda_cur_val_p = timing_info->sda_def_val;
	else
		*sda_cur_val_p = val[timing_info->speed_mode];
}

void xr_i2c_parse_fw_timings(struct device *dev, struct i2c_timings *t, bool use_defaults)
{
	int ret;
	u32 val[XR_SUPPORT_SPEED_MODE] = {0}; /* 4 modes: 100KHz, 400KHz, 1MHz, 3.4MHz */
	struct xr_i2c_timing_info timing_info = {
			.using_default = use_defaults,
		};

	ret = device_property_read_u32(dev, "clock-frequency", &t->bus_freq_hz);
	if (ret && use_defaults) {
		t->bus_freq_hz = I2C_MAX_FAST_MODE_FREQ; /* default 400KHz mode */
		timing_info.speed_mode = XR_FAST_MODE;
		dev_dbg(dev, "clock-frequency: %u\n", t->bus_freq_hz);
	}

	switch (t->bus_freq_hz) {
	case I2C_MAX_STANDARD_MODE_FREQ:  /* 100KHz */
		timing_info.scl_def_val = SCL_SS_FALLING_TIME;
		timing_info.sda_def_val = SDA_SS_FALLING_TIME;
		timing_info.speed_mode = XR_STANDARD_MODE;
		break;
	case I2C_MAX_FAST_MODE_FREQ:  /* 400KHz */
		timing_info.scl_def_val = SCL_FS_FALLING_TIME;
		timing_info.sda_def_val = SDA_FS_FALLING_TIME;
		timing_info.speed_mode = XR_FAST_MODE;
		break;
	case I2C_MAX_FAST_MODE_PLUS_FREQ:  /* 1MHz */
		timing_info.scl_def_val = SCL_FP_FALLING_TIME;
		timing_info.sda_def_val = SDA_FP_FALLING_TIME;
		timing_info.speed_mode = XR_FAST_MODE_PLUS;
		break;
	case I2C_MAX_HIGH_SPEED_MODE_FREQ:  /* 3.4MHz */
		timing_info.scl_def_val = SCL_HS_FALLING_TIME;
		timing_info.sda_def_val = SDA_HS_FALLING_TIME;
		timing_info.speed_mode = XR_HIGH_SPEED_MODE;
		break;
	default:
		return;
	}

	ret = device_property_read_u32_array(dev, "tHD,DAT",
					val, XR_SUPPORT_SPEED_MODE);
	if (ret && use_defaults)
		t->sda_hold_ns = 0; /* use chip default, i2c_dw_set_sda_hold */
	else
		t->sda_hold_ns = val[timing_info.speed_mode];

	dev_dbg(dev, "from dts tHD,DAT: %u\n", t->sda_hold_ns);

	i2c_parse_timing(dev, &t->scl_fall_ns, &t->sda_fall_ns, &timing_info);

	dev_dbg(dev, "clock-frequency: %u, i2c-scl-falling-time-ns: %u, i2c-sda-falling-time-ns: %u\n",
			t->bus_freq_hz, t->scl_fall_ns, t->sda_fall_ns);
}

static int dw_i2c_plat_probe(struct platform_device *pdev)
{
	struct i2c_adapter *adap = NULL;
	struct dw_i2c_dev *dev = NULL;
	struct i2c_timings *t = NULL;
	const char *clk_name = NULL;
	int irq;
	int ret;
	u64 clk_khz;
	int i;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	dev = devm_kzalloc(&pdev->dev, sizeof(struct dw_i2c_dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->dev = &pdev->dev;
	dev->irq = irq;
	platform_set_drvdata(pdev, dev);

	ret = dw_i2c_plat_request_regs(dev);
	if (ret)
		return ret;

	dev->prst = devm_reset_control_get_exclusive(&pdev->dev, "prst");
	if (IS_ERR(dev->prst)) {
		dev_err(dev->dev, "Couldn't get prst control: %d\n", ret);
		return PTR_ERR(dev->prst);
	}

	dev->rst = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(dev->rst))  {
		dev_err(dev->dev, "Couldn't get rst control: %d\n", ret);
		return PTR_ERR(dev->rst);
	}

	ret = reset_control_deassert(dev->prst);
	if (ret < 0) {
		dev_err(dev->dev, "Couldn't deassert prst control: %d\n", ret);
		return ret;
	}

	ret = reset_control_deassert(dev->rst);
	if (ret < 0) {
		dev_err(dev->dev, "Couldn't deassert rst control: %d\n", ret);
		return ret;
	}

	t = &dev->timings;
	xr_i2c_parse_fw_timings(&pdev->dev, t, true);

	ret = i2c_dw_validate_speed(dev);
	if (ret)
		goto exit_reset;

	ret = i2c_dw_probe_lock_support(dev);
	if (ret)
		goto exit_reset;

	i2c_dw_configure_master(dev);

	for (i = 0; i < XR_I2C_CLK_NUMS; i++) {
		ret = of_property_read_string_index(
					(&pdev->dev)->of_node,
					"clock-names",
					i,
					&clk_name);
		if (ret < 0) {
			dev_err(dev->dev, "clock-names get failed.\n");
			return ret;
		}

		if (memcmp(clk_name, XR_PCLK_PREFIX_NAME, XR_PCLK_PREFIX_LEN) == 0) {
			dev->pclk = devm_clk_get(&pdev->dev, clk_name);
			if (IS_ERR(dev->pclk)) {
				ret = PTR_ERR(dev->pclk);
				goto exit_reset;
			}
		} else {
			dev->clk = devm_clk_get(&pdev->dev, clk_name);
			if (IS_ERR(dev->clk)) {
				ret = PTR_ERR(dev->clk);
				goto exit_reset;
			}
		}
	}

	ret = i2c_dw_prepare_clk(dev, I2C_CLK_PREPARE_ENABLE);
	if (ret)
		goto exit_reset;

	if (dev->clk) {
		dev->get_clk_rate_khz = i2c_dw_get_clk_rate_khz;
		clk_khz = dev->get_clk_rate_khz(dev);

		if (!dev->sda_hold_time && t->sda_hold_ns)
			dev->sda_hold_time =
				DIV_S64_ROUND_CLOSEST(clk_khz * t->sda_hold_ns, MICRO);
	}

	adap = &dev->adapter;
	adap->owner = THIS_MODULE;
	adap->class = dmi_check_system(NULL) ?
				I2C_CLASS_HWMON : I2C_CLASS_DEPRECATED;

	ACPI_COMPANION_SET(&adap->dev, ACPI_COMPANION(&pdev->dev));
	adap->dev.of_node = pdev->dev.of_node;
	adap->nr = XR_I2C_DEFAULT_NR;

	dev->suspended = true;
	/* The code below assumes runtime PM to be disabled. */
	WARN_ON(pm_runtime_enabled(&pdev->dev));

	ret = i2c_dw_probe_master(dev);
	if (ret)
		goto exit_probe;

	i2c_dw_prepare_clk(dev, I2C_CLK_DISABLE);

	pr_info("Synopsys DesignWare I2C adapter %d init OK\n", adap->nr);

	return ret;

exit_probe:
	i2c_dw_prepare_clk(dev, I2C_CLK_DISABLE_UNPREPARE);
exit_reset:
	reset_control_assert(dev->rst);
	reset_control_assert(dev->prst);
	return ret;
}

static int dw_i2c_plat_remove(struct platform_device *pdev)
{
	struct dw_i2c_dev *dev = platform_get_drvdata(pdev);

#if defined CONFIG_XRING_I2C_DMA
	xr_i2c_release_dma(dev);
#endif

#if defined CONFIG_XRING_I2C_DEBUGFS
	xr_i2c_cleanup_debugfs(dev);
#endif

	i2c_del_adapter(&dev->adapter);

	dev->disable(dev);

	i2c_dw_prepare_clk(dev, I2C_CLK_DISABLE_UNPREPARE);
	reset_control_assert(dev->rst);
	reset_control_assert(dev->prst);

	if (dev->hwspin_lock)
		hwspin_lock_free(dev->hwspin_lock);

	return 0;
}

int xr_i2c_runtime_suspend(struct device *dev)
{
	struct dw_i2c_dev *i_dev = dev_get_drvdata(dev);
	int ret;

	i_dev->suspended = true;
	i_dev->disable(i_dev);
	ret = i2c_irq_lock_clk_disable(i_dev, XR_I2C_TRANS_DONE); /* Until the next transfer. */
	if (ret < 0) {
		dev_warn(dev, "failed to i2c_irq_lock_clk_disable, ret = %d\n", ret);
		return ret;
	}

	ret = i2c_pinsctrl(i_dev, PINCTRL_STATE_SLEEP);
	if (ret < 0)
		dev_warn(dev, "pins are not configured to sleep, ret = %d\n", ret);

	return 0;
}

int xr_i2c_runtime_resume(struct device *dev)
{
	struct dw_i2c_dev *i_dev = dev_get_drvdata(dev);
	int ret;

	if (i_dev->hwspin_lock) {
		ret = reset_control_deassert(i_dev->prst);
		if (ret < 0) {
			dev_err(dev, "Couldn't deassert prst control: %d\n", ret);
			return ret;
		}

		ret = reset_control_deassert(i_dev->rst);
		if (ret < 0) {
			dev_err(dev, "Couldn't deassert rst control: %d\n", ret);
			return ret;
		}
	}

	ret = i2c_dw_prepare_clk(i_dev, I2C_CLK_EBABLE);
	if (ret) {
		dev_err(dev, "prepare clk failed, ret = %d\n", ret);
		return ret;
	}

	if (i_dev->hwspin_lock)
		i_dev->init(i_dev);

	ret = i2c_pinsctrl(i_dev, PINCTRL_STATE_DEFAULT);
	if (ret < 0)
		dev_warn(dev, "pins are not configured to default, ret = %d\n", ret);

	i_dev->suspended = false;

	return 0;
}

int xr_i2c_suspend(struct device *dev)
{
	struct dw_i2c_dev *i_dev = dev_get_drvdata(dev);
	unsigned long time, timeout;
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	i2c_mark_adapter_suspended(&i_dev->adapter);

	timeout = jiffies + msecs_to_jiffies(GET_DEV_LOCK_TIMEOUT);
	while (!mutex_trylock(&i_dev->lock)) {
		time = jiffies;
		if (time_after(time, timeout)) {
			dev_warn(dev, "%s: mutex_trylock timeout fail.\n", __func__);
			i2c_mark_adapter_resumed(&i_dev->adapter);
			return -EAGAIN;
		}

		/* 1 ~ 2ms */
		usleep_range(1000, 2000);
	}

	if (i_dev->hwspin_lock) {
		ret = i2c_hardware_spinlock(i_dev);
		if (ret < 0) {
			dev_err(dev, "request hwspinlock failed\n");
			return -EAGAIN;
		}
	}

	if (i_dev->suspended != true)
		dev_info(dev, "%s: force suspend.\n", __func__);

	dev_info(dev, "%s --\n", __func__);

	if (i_dev->hwspin_lock)
		hwspin_unlock_raw(i_dev->hwspin_lock);

	return 0;
}

int xr_i2c_resume(struct device *dev)
{
	struct dw_i2c_dev *i_dev = dev_get_drvdata(dev);
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	if (i_dev->hwspin_lock) {
		ret = i2c_hardware_spinlock(i_dev);
		if (ret < 0) {
			dev_err(dev, "request hwspinlock failed: %d\n", ret);
			return ret;
		}
	}

	ret = reset_control_deassert(i_dev->prst);
	if (ret < 0) {
		dev_err(dev, "Couldn't deassert prst control: %d\n", ret);
		goto err_exit;
	}

	ret = reset_control_deassert(i_dev->rst);
	if (ret < 0) {
		dev_err(dev, "Couldn't deassert rst control: %d\n", ret);
		goto err_exit;
	}

	ret = i2c_dw_prepare_clk(i_dev, I2C_CLK_EBABLE);
	if (ret) {
		dev_err(dev, "prepare clk failed, ret = %d\n", ret);
		goto err_exit;
	}

	i_dev->init(i_dev);

	ret = i2c_dw_prepare_clk(i_dev, I2C_CLK_DISABLE);
	if (ret) {
		dev_err(dev, "unprepare clk failed, ret = %d\n", ret);
		goto err_exit;
	}

	mutex_unlock(&i_dev->lock);
	i2c_mark_adapter_resumed(&i_dev->adapter);

err_exit:
	if (i_dev->hwspin_lock)
		hwspin_unlock_raw(i_dev->hwspin_lock);

	dev_info(dev, "%s --\n", __func__);

	return ret;
}

static const struct dev_pm_ops xr_i2c_dev_pm_ops = {
	NOIRQ_SYSTEM_SLEEP_PM_OPS(xr_i2c_suspend, xr_i2c_resume)
};


/* Work with hotplug and coldplug */
MODULE_ALIAS("platform:i2c_designware");

static struct platform_driver dw_i2c_driver = {
	.probe = dw_i2c_plat_probe,
	.remove = dw_i2c_plat_remove,
	.driver		= {
		.name	= "i2c_designware",
		.of_match_table = of_match_ptr(dw_i2c_of_match),
		.pm	= &xr_i2c_dev_pm_ops,
	},
};

static int __init dw_i2c_init_driver(void)
{
	return platform_driver_register(&dw_i2c_driver);
}
subsys_initcall(dw_i2c_init_driver);

static void __exit dw_i2c_exit_driver(void)
{
	platform_driver_unregister(&dw_i2c_driver);
}
module_exit(dw_i2c_exit_driver);

MODULE_SOFTDEP("pre: xr_ip_reset");
MODULE_SOFTDEP("pre: pinctrl-xr");
MODULE_SOFTDEP("pre: xring_axi_dmac");
MODULE_SOFTDEP("pre: xring_hwspinlock");
MODULE_AUTHOR("Baruch Siach <baruch@tkos.co.il>");
MODULE_AUTHOR("Hu Wei <huwei19@xiaomi.com>");
MODULE_DESCRIPTION("Synopsys DesignWare I2C bus adapter");
MODULE_LICENSE("GPL");
