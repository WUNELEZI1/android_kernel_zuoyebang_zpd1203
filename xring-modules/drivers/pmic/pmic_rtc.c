// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
 */
#include <linux/of.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
#include <linux/delay.h>

#define NUM_8_BIT_RTC_REGS		0x4
#define PMIC_RTC_CNT_EN		BIT(0)
#define PMIC_RTC_RELOAD_EN	BIT(0)
#define PMIC_RTC_ALARM_EN	BIT(0)
#define PMIC_CLEAR_INTR_EN	BIT(0)
#define PMIC_RTC_ALARM_DIS	0
#define PMIC_RTC_RELOAD_DIS 0
#define PMIC_RTC_INTR		BIT(3)
#define PMIC_RTC_ENABLE		1
#define PMIC_RTC_DIS		0
#define PMIC_BIT_WIDTH		8
#define PMIC_BIT_MASK		0xff
#define RELOAD_RTC_TIME_US	31
#define ALARM_DEFAULT_VALUE	0xffffffff

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_PMICRTC:%s:%d " fmt, __func__, __LINE__

typedef void (*pmic_rtc_callback_t)(void);
/**
 * struct pmic_rtc -  rtc driver internal structure
 * @rtc:		rtc device for this driver.
 * @regmap:		regmap used to access RTC registers
 * @allow_set_time:	indicates whether writing to the RTC is allowed
 * @rtc_alarm_irq:	rtc alarm irq number.
 * @rtc_dev:		device structure.
 * @ctrl_reg_lock:	spinlock protecting access to ctrl_reg.
 */
struct pmic_rtc {
	struct rtc_device *rtc;
	struct regmap *regmap;
	bool allow_set_time;
	int rtc_alarm_irq;
	struct device *rtc_dev;
	spinlock_t ctrl_reg_lock;
	uint32_t base;
	pmic_rtc_callback_t alarm_cb;
};
struct pmic_rtc *g_pmic_rtc_dev;

static int pmic_rtc_readl(struct pmic_rtc *rtc, uint16_t offset, uint32_t *data, int len)
{
	int ret;

	ret = regmap_bulk_read(rtc->regmap, rtc->base + offset, data, len);
	pr_debug("base:0x%x,offset:0x%x,data:0x%x,len:0x%x\n",
			rtc->base, offset, *data, len);
	return ret;
}

static int pmic_rtc_writel(struct pmic_rtc *rtc, uint16_t offset, uint32_t *data, int len)
{
	int ret;

	pr_debug("base:0x%x,offset:0x%x,data:0x%x,len:0x%x\n",
			rtc->base, offset, *data, len);
	ret = regmap_bulk_write(rtc->regmap, rtc->base + offset, data, len);
	return ret;
}

static int pmic_rtc_read(struct pmic_rtc *rtc, uint16_t offset, uint8_t *data, int len)
{
	int ret;

	ret = regmap_bulk_read(rtc->regmap, rtc->base + offset, data, len);
	pr_debug("base:0x%x,offset:0x%x,data:0x%x,len:0x%x\n",
			rtc->base, offset, *data, len);
	return ret;
}

static int pmic_rtc_write(struct pmic_rtc *rtc, uint16_t offset, uint8_t *data, int len)
{
	int ret;

	pr_debug("base:0x%x,offset:0x%x,data:0x%x,len:0x%x\n",
			rtc->base, offset, *data, len);
	ret = regmap_bulk_write(rtc->regmap, rtc->base + offset, data, len);
	return ret;
}

static irqreturn_t pmic_alarm_trigger(int irq, void *dev_id)
{
	struct pmic_rtc *xr_pmic_rtc = dev_id;

	if (xr_pmic_rtc->alarm_cb)
		xr_pmic_rtc->alarm_cb();

	return IRQ_HANDLED;
}

void pmic_register_alarm_callback(pmic_rtc_callback_t fn)
{
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	rtc_dev->alarm_cb = fn;
}
EXPORT_SYMBOL(pmic_register_alarm_callback);

static int pmic_rtc_alarm_irq_enable(bool enable)
{
	int ret;
	uint8_t reg = 0;
	unsigned long irq_flags;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	spin_lock_irqsave(&rtc_dev->ctrl_reg_lock, irq_flags);

	ret = pmic_rtc_read(rtc_dev, TOP_REG_PMIC_LVL2INT_O14_IRQ_INT_MASK, &reg, sizeof(reg));
	if (ret)
		goto rtc_rw_fail;

	if (enable)
		reg &= ~PMIC_RTC_INTR;
	else
		reg |= PMIC_RTC_INTR;

	ret = pmic_rtc_write(rtc_dev, TOP_REG_PMIC_LVL2INT_O14_IRQ_INT_MASK, &reg, sizeof(reg));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to rtc_intr_mask register failed\n");
		goto rtc_rw_fail;
	}
rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dev->ctrl_reg_lock, irq_flags);
	return ret;
}

/*******************************************************************************
 * Function: pmic_rtc_getalarm
 *
 * Description:
 *   Set the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_getalarm(struct rtc_wkalrm *alarm)
{
	int ret;
	uint32_t secs = 0;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	ret = pmic_rtc_readl(rtc_dev, TOP_REG_SW_ALARM_T_CFG1_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "RTC alarm time read failed\n");
		return ret;
	}

	pr_debug(" secs:0x%x\n", secs);
	rtc_time64_to_tm(secs, &alarm->time);
	dev_info(rtc_dev->rtc_dev, "Alarm set for - h:m:s=%ptRt, y-m-d=%ptRdr\n",
					&alarm->time, &alarm->time);
	return 0;
}
EXPORT_SYMBOL(pmic_rtc_getalarm);

int pmic_rtc_poweroffalarm_enable(bool enabled)
{
	int ret;
	uint32_t secs = 0;
	uint8_t reg = 0;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	if (enabled) {
		reg = PMIC_RTC_ALARM_EN;
	} else {
		reg = PMIC_RTC_ALARM_DIS;
		secs = ALARM_DEFAULT_VALUE;
		dev_info(rtc_dev->rtc_dev, "Disabling RTC Alarm, reset it's value(0x%x)\n", secs);
		ret = pmic_rtc_writel(rtc_dev, TOP_REG_SW_ALARM_T_CFG2_0, &secs, sizeof(secs));
		if (ret) {
			dev_err(rtc_dev->rtc_dev, "Write to alarm_cfg2_0 register failed\n");
			return ret;
		}
	}

	ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_ALARM_CFG2_EN, &reg, sizeof(reg));
	if (ret)
		dev_err(rtc_dev->rtc_dev, "Write to rtc_intr_mask register failed\n");
	return ret;
}
EXPORT_SYMBOL(pmic_rtc_poweroffalarm_enable);

/*******************************************************************************
 * Function: pmic_rtc_setalarm
 *
 * Description:
 *   Set the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_setalarm(struct rtc_wkalrm *alarm)
{
	int ret;
	uint32_t secs;
	uint8_t reg;
	unsigned long irq_flags;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	secs = rtc_tm_to_time64(&alarm->time);
	dev_info(rtc_dev->rtc_dev, "Seconds alarm value to be written to RTC = %u\n", secs);
	pmic_rtc_alarm_irq_enable(PMIC_RTC_ENABLE);
	spin_lock_irqsave(&rtc_dev->ctrl_reg_lock, irq_flags);
	ret = pmic_rtc_read(rtc_dev, TOP_REG_SW_ALARM_CFG1_EN, &reg, sizeof(reg));
	if (ret < 0) {
		dev_err(rtc_dev->rtc_dev, "read alarm_cfg1_en register failed\n");
		return ret;
	}
	if (reg) {
		reg &= PMIC_RTC_RELOAD_DIS;
		ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_ALARM_CFG1_EN, &reg, sizeof(reg));
		if (ret) {
			dev_err(rtc_dev->rtc_dev, "Write to alarm_cfg1_en register failed\n");
			goto rtc_rw_fail;
		}
	}
	ret = pmic_rtc_writel(rtc_dev, TOP_REG_SW_ALARM_T_CFG1_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to alarm_cfg1_en register failed\n");
		goto rtc_rw_fail;
	}

	reg = PMIC_RTC_ALARM_EN;
	ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_ALARM_CFG1_EN, &reg, sizeof(reg));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to RTC alarm_en register failed\n");
		goto rtc_rw_fail;
	}

rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dev->ctrl_reg_lock, irq_flags);
	return ret;
}
EXPORT_SYMBOL(pmic_rtc_setalarm);

/*******************************************************************************
 * Function: pmic_rtc_getwkalarm
 *
 * Description:
 *   Set the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_getwkalarm(struct rtc_wkalrm *alarm)
{
	int ret;
	uint32_t secs = 0;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	ret = pmic_rtc_readl(rtc_dev, TOP_REG_SW_ALARM_T_CFG2_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "RTC alarm time read failed\n");
		return ret;
	}

	rtc_time64_to_tm(secs, &alarm->time);
	dev_info(rtc_dev->rtc_dev, "Alarm set for - h:m:s=%ptRt, y-m-d=%ptRdr\n",
					&alarm->time, &alarm->time);
	return 0;
}
EXPORT_SYMBOL(pmic_rtc_getwkalarm);

/*******************************************************************************
 * Function: pmic_rtc_setwkalarm
 *
 * Description:
 *   Set the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_setwkalarm(struct rtc_wkalrm *alarm)
{
	int ret;
	uint32_t secs;
	uint8_t reg;
	unsigned long irq_flags;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	secs = rtc_tm_to_time64(&alarm->time);
	dev_info(rtc_dev->rtc_dev, "Seconds alarm value to be written to RTC = %u\n", secs);

	pmic_rtc_alarm_irq_enable(PMIC_RTC_ENABLE);
	spin_lock_irqsave(&rtc_dev->ctrl_reg_lock, irq_flags);
	ret = pmic_rtc_read(rtc_dev, TOP_REG_SW_ALARM_CFG2_EN, &reg, sizeof(reg));
	if (ret < 0) {
		dev_err(rtc_dev->rtc_dev, "read alarm_cfg2_en register failed\n");
		return ret;
	}
	if (reg) {
		reg &= PMIC_RTC_RELOAD_DIS;
		ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_ALARM_CFG2_EN, &reg, sizeof(reg));
		if (ret) {
			dev_err(rtc_dev->rtc_dev, "Write to alarm_cfg2_en register failed\n");
			goto rtc_rw_fail;
		}
	}
	ret = pmic_rtc_writel(rtc_dev, TOP_REG_SW_ALARM_T_CFG2_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to alarm_cfg2_0 register failed\n");
		goto rtc_rw_fail;
	}
	reg = PMIC_RTC_ALARM_EN;
	ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_ALARM_CFG2_EN, &reg, sizeof(reg));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to RTC alarm_en register failed\n");
		goto rtc_rw_fail;
	}

rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dev->ctrl_reg_lock, irq_flags);
	return ret;
}
EXPORT_SYMBOL(pmic_rtc_setwkalarm);

/*******************************************************************************
 * Function: pmic_rtc_settime
 *
 * Description:
 *   Set the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_settime(struct rtc_time *tm)
{
	int ret;
	uint8_t reg;
	uint32_t secs;
	unsigned long irq_flags;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	if (!rtc_dev->allow_set_time)
		return -EACCES;
	secs = rtc_tm_to_time64(tm);
	dev_info(rtc_dev->rtc_dev, "Seconds value to be written to RTC = 0x%x\n", secs);
	spin_lock_irqsave(&rtc_dev->ctrl_reg_lock, irq_flags);
	ret = pmic_rtc_writel(rtc_dev, TOP_REG_SW_RELOAD_RTC_CFG_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to RTC write data register failed\n");
		goto rtc_rw_fail;
	}
	reg = PMIC_RTC_RELOAD_EN;
	ret = pmic_rtc_write(rtc_dev, TOP_REG_SW_RELOAD_RTC_EN, &reg, sizeof(reg));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "Write to RTC reload_en register failed\n");
		goto rtc_rw_fail;
	}
	udelay(RELOAD_RTC_TIME_US);
rtc_rw_fail:
	spin_unlock_irqrestore(&rtc_dev->ctrl_reg_lock, irq_flags);
	return ret;
}
EXPORT_SYMBOL(pmic_rtc_settime);

/*******************************************************************************
 * Function: pmic_rtc_gettime
 *
 * Description:
 *   Get the count value of the rtc register inside the pmic
 *
 * Input:
 *   null
 ********************************************************************************/
int pmic_rtc_gettime(struct rtc_time *tm)
{
	int ret;
	uint32_t secs = 0;
	struct pmic_rtc *rtc_dev = g_pmic_rtc_dev;

	ret = pmic_rtc_readl(rtc_dev, TOP_REG_RTC_TIME_0, &secs, sizeof(secs));
	if (ret) {
		dev_err(rtc_dev->rtc_dev, "RTC read data register failed\n");
		return ret;
	}
	pr_debug("[gettime], secs:0x%x\n", secs);
	rtc_time64_to_tm(secs, tm);
	dev_info(rtc_dev->rtc_dev, "secs = %u, h:m:s == %ptRt, y-m-d = %ptRdr\n", secs, tm, tm);
	return 0;
}
EXPORT_SYMBOL(pmic_rtc_gettime);

static int pmic_rtc_enable(struct pmic_rtc *xr_pmic_rtc)
{
	uint8_t cnt_en_reg;
	int ret;

	/* Check if the RTC is on, else turn it on */
	ret = pmic_rtc_read(xr_pmic_rtc, TOP_REG_SW_RTC_CNT_EN, &cnt_en_reg, sizeof(cnt_en_reg));
	if (ret)
		return ret;

	if (!(cnt_en_reg & PMIC_RTC_CNT_EN)) {
		cnt_en_reg |= PMIC_RTC_CNT_EN;
		ret = pmic_rtc_write(xr_pmic_rtc, TOP_REG_SW_RTC_CNT_EN,
						&cnt_en_reg, sizeof(cnt_en_reg));
		if (ret)
			return ret;
	} else {
		pr_info("pmic_rtc is enabled as expected!\n");
	}
	return 0;
}

/*
 * Hardcoded RTC bases until IORESOURCE_REG mapping is figured out
 */
static const struct of_device_id pmic_id_table[] = {
	{ .compatible = "xring,pmic-rtc"},
	{ },
};
MODULE_DEVICE_TABLE(of, pmic_id_table);

static int pmic_rtc_probe(struct platform_device *pdev)
{
	int ret;
	uint32_t base_addr;
	struct pmic_rtc *xr_pmic_rtc = NULL;
	const struct of_device_id *match = NULL;

	match = of_match_node(pmic_id_table, pdev->dev.of_node);
	if (!match)
		return -ENXIO;

	xr_pmic_rtc = devm_kzalloc(&pdev->dev, sizeof(*xr_pmic_rtc), GFP_KERNEL);
	if (xr_pmic_rtc == NULL)
		return -ENOMEM;

	g_pmic_rtc_dev = xr_pmic_rtc;
	/* Initialise spinlock to protect RTC control register */
	spin_lock_init(&xr_pmic_rtc->ctrl_reg_lock);

	xr_pmic_rtc->regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!xr_pmic_rtc->regmap) {
		dev_err(&pdev->dev, "Parent regmap unavailable.\n");
		return -ENXIO;
	}

	xr_pmic_rtc->rtc_alarm_irq = platform_get_irq(pdev, 0);
	if (xr_pmic_rtc->rtc_alarm_irq < 0)
		return -ENXIO;

	xr_pmic_rtc->allow_set_time = of_property_read_bool(pdev->dev.of_node,
						      "allow-set-time");
	ret = of_property_read_u32(pdev->dev.of_node, "base_addr", &base_addr);
	if (ret) {
		dev_err(&pdev->dev, "invalid rtc base_addr\n");
		return ret;
	}
	xr_pmic_rtc->base = base_addr;
	xr_pmic_rtc->rtc_dev = &pdev->dev;
	ret = pmic_rtc_enable(xr_pmic_rtc);
	if (ret)
		return ret;

	/* Request the alarm IRQ */
	ret = devm_request_any_context_irq(&pdev->dev, xr_pmic_rtc->rtc_alarm_irq,
					  pmic_alarm_trigger,
					  IRQF_TRIGGER_RISING,
					  "pmic_rtc_alarm", xr_pmic_rtc);
	if (ret < 0) {
		dev_err(&pdev->dev, "Request IRQ failed (%d)\n", ret);
		return ret;
	}
	dev_info(&pdev->dev, "%s succ!\n", __func__);
	return 0;
}

static int pmic_rtc_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s succ!\n", __func__);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int pmic_rtc_resume(struct device *dev)
{
	struct pmic_rtc *xr_pmic_rtc = dev_get_drvdata(dev);

	if (device_may_wakeup(dev))
		disable_irq_wake(xr_pmic_rtc->rtc_alarm_irq);

	return 0;
}

static int pmic_rtc_suspend(struct device *dev)
{
	struct pmic_rtc *xr_pmic_rtc = dev_get_drvdata(dev);

	if (device_may_wakeup(dev))
		enable_irq_wake(xr_pmic_rtc->rtc_alarm_irq);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(pmic_rtc_pm_ops,
			 pmic_rtc_suspend,
			 pmic_rtc_resume);

static struct platform_driver pmic_rtc_driver = {
	.probe		= pmic_rtc_probe,
	.remove = pmic_rtc_remove,
	.driver	= {
		.name		= "rtc-pmic",
		.pm		= &pmic_rtc_pm_ops,
		.of_match_table	= pmic_id_table,
	},
};

int xr_pmic_rtc_init(void)
{
	return platform_driver_register(&pmic_rtc_driver);
}

void xr_pmic_rtc_exit(void)
{
	platform_driver_unregister(&pmic_rtc_driver);
}
