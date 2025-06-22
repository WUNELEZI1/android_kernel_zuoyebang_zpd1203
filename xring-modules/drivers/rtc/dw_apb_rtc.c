// SPDX-License-Identifier: GPL-2.0-only
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/reset.h>
#include <dt-bindings/xring/platform-specific/lms_crg.h>
#include <dt-bindings/xring/platform-specific/DW_apb_rtc_header.h>
#include <soc/xring/xr_pmic_rtc.h>

#define RTC_CCR_IE BIT(DW_APB_RTC_CCR_RTC_IEN_BITADDRESSOFFSET)
#define RTC_CCR_MASK BIT(DW_APB_RTC_CCR_RTC_MASK_BITADDRESSOFFSET)

/* convert seconds to 32bit counter value */
#define SEC_TO_CNT(sec, freq) (sec * freq)
/* convert 32bit counter value to seconds */
#define CNT_TO_SEC(cnt, freq) (cnt / freq)

#define IOREMAP_SIZE 4
#define DEFAULT_CPSR_VAL 1024
#define ENABLE 1
#define DISABLE 0
/* This is how many irqs are being reported (usually one) */
#define RTC_IRQS_NUM 1
#define CRG_MAP_SIZE 0x10

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "dw_apb_rtc:%s:%d " fmt, __func__, __LINE__

struct dw_rtc_dev {
	struct rtc_device *rtc;
	struct device_node *np;
	struct device *dev;
	void __iomem *csr_base;
	struct clk *clk;
	unsigned int irq_wake;
	unsigned int irq_enabled;
	spinlock_t slock;
	uint32_t rtc_freq;
	struct reset_control *rst;
	void __iomem *crg_addr;
};

/* Saved data for persistent RTC time */
static int g_rtc_saved;

#ifdef RTC_DEBUG
typedef void (*pmic_rtc_callback_t)(void);
#endif

uint32_t dw_rtc_cnt_get(struct device *dev)
{
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	return readl(pdata->csr_base + DW_APB_RTC_CCVR) + g_rtc_saved;
}

static inline void dw_rtc_cnt_set(struct device *dev, uint32_t count)
{
	uint32_t alarm_count;
	uint32_t current_count;
	int cnt_offset;
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	alarm_count = readl(pdata->csr_base + DW_APB_RTC_CMR);
	current_count = readl(pdata->csr_base + DW_APB_RTC_CCVR);

	/* If the alarm is set, ensure that it is updated */
	if (alarm_count > current_count) {
		cnt_offset = count - dw_rtc_cnt_get(dev);
		alarm_count -= cnt_offset;
		writel(alarm_count, pdata->csr_base + DW_APB_RTC_CMR);
	}
	g_rtc_saved = count - current_count;
}

static inline void dw_rtc_match_cnt_set(struct device *dev, uint32_t value)
{
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	writel(value, pdata->csr_base + DW_APB_RTC_CMR);
}

static int dw_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long flags;
	uint32_t count = 0;
	uint32_t sec = 0;
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	spin_lock_irqsave(&pdata->slock, flags);
	count = dw_rtc_cnt_get(dev);
	sec = CNT_TO_SEC(count, pdata->rtc_freq);
	spin_unlock_irqrestore(&pdata->slock, flags);
	rtc_time64_to_tm(sec, tm);
	return 0;
}

static int dw_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long flags;
	uint32_t count = 0;
	uint32_t sec = 0;
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);
	int ret = 0;

	/* Converts the passed rtc time to a count value */
	sec = rtc_tm_to_time64(tm);
	spin_lock_irqsave(&pdata->slock, flags);
	count = SEC_TO_CNT(sec, pdata->rtc_freq);
	dw_rtc_cnt_set(dev, count);
	spin_unlock_irqrestore(&pdata->slock, flags);
	ret = pmic_rtc_settime(tm);
	if (ret < 0) {
		dev_err(pdata->dev, "pmic_rtc_settime fail!\n");
		return ret;
	}
	return 0;
}

static int dw_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned long flags;
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	spin_lock_irqsave(&pdata->slock, flags);
	rtc_time64_to_tm(readl(pdata->csr_base + DW_APB_RTC_CMR), &alrm->time);
	alrm->pending = readl(pdata->csr_base + DW_APB_RTC_RSTAT) & RTC_CCR_MASK;
	alrm->enabled = readl(pdata->csr_base + DW_APB_RTC_CCR) & RTC_CCR_IE ? 1 : 0;
	spin_unlock_irqrestore(&pdata->slock, flags);
	return 0;
}

static int dw_rtc_alarm_irq_enable(struct device *dev, uint32_t enabled)
{
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);
	uint32_t ccr;
	int ret = 0;

	ccr = readl(pdata->csr_base + DW_APB_RTC_CCR);
	if (enabled) {
		ccr &= ~RTC_CCR_MASK;
		ccr |= RTC_CCR_IE;
	} else {
		ccr &= ~RTC_CCR_IE;
		ccr |= RTC_CCR_MASK;
	}
	writel(ccr, pdata->csr_base + DW_APB_RTC_CCR);
	ret = pmic_rtc_poweroffalarm_enable(enabled);
	if (ret)
		dev_err(pdata->dev, "pmic_rtc_poweroffalarm_enable fail!\n");
	return ret;
}

static int dw_rtc_alarm_irq_enabled(struct device *dev)
{
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);

	return readl(pdata->csr_base + DW_APB_RTC_CCR) & RTC_CCR_IE ? 1 : 0;
}

static int dw_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned long flags;
	uint32_t alarm_sec;
	uint32_t alarm_count;
	uint32_t match_count;
	struct dw_rtc_dev *pdata = dev_get_drvdata(dev);
	int ret = 0;

	/* Converts the passed rtc time to a count value */
	alarm_sec = rtc_tm_to_time64(&alrm->time);
	alarm_count = SEC_TO_CNT(alarm_sec, pdata->rtc_freq);
	spin_lock_irqsave(&pdata->slock, flags);
	/* Set Count's Match Value*/
	match_count = alarm_count - g_rtc_saved;
	dw_rtc_match_cnt_set(dev, match_count);
	/* Enable Interrupt */
	dw_rtc_alarm_irq_enable(dev, 1);
	spin_unlock_irqrestore(&pdata->slock, flags);
	ret = pmic_rtc_setwkalarm(alrm);
	if (ret < 0) {
		dev_err(pdata->dev, "pmic_rtc_setwkalarm fail!\n");
		return ret;
	}
	return 0;
}

static const struct rtc_class_ops dw_rtc_ops = {
	.read_time = dw_rtc_read_time,
	.set_time = dw_rtc_set_time,
	.read_alarm = dw_rtc_read_alarm,
	.set_alarm = dw_rtc_set_alarm,
	.alarm_irq_enable = dw_rtc_alarm_irq_enable,
};

static irqreturn_t dw_rtc_interrupt(int irq, void *id)
{
	struct dw_rtc_dev *pdata = id;

	/* Check whether the interrupt is an RTC interrupt */
	if (readl(pdata->csr_base + DW_APB_RTC_STAT) != 0) {
		writel(0, pdata->csr_base + DW_APB_RTC_CMR);
		readl(pdata->csr_base + DW_APB_RTC_EOI);
		/* Used to update the RTC intr status, intr and alarm flag */
		rtc_update_irq(pdata->rtc, RTC_IRQS_NUM, RTC_IRQF | RTC_AF);
	} else {
		return IRQ_NONE;
	}
	return IRQ_HANDLED;
}

static int get_crg_info(struct dw_rtc_dev *pdata)
{
	int ret;
	uint32_t val;

	pdata->rst = devm_reset_control_get_exclusive(pdata->dev, "rst");
	if (IS_ERR(pdata->rst))  {
		dev_err(pdata->dev, "Couldn't get rst control\n");
		return PTR_ERR(pdata->rst);
	}

	ret = device_property_read_u32(pdata->dev, "crg_base", &val);
	if (ret) {
		dev_err(pdata->dev, "failed to get crg_base, ret = %d\n", ret);
		return ret;
	}

	dev_dbg(pdata->dev, "get crg-base = %x\n", val);
	pdata->crg_addr = devm_ioremap(pdata->dev, val, CRG_MAP_SIZE);
	return ret;
}

#ifdef RTC_DEBUG
static void pmic_rtc_callback(void)
{
	pr_info("%s enter!!!\n", __func__);
}
#endif

static int dw_rtc_probe(struct platform_device *pdev)
{
	struct dw_rtc_dev *pdata;
	struct device_node *node = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	int ret;
	int irq;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;
	spin_lock_init(&pdata->slock);
	platform_set_drvdata(pdev, pdata);

	pdata->dev = dev;
	pdata->csr_base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(pdata->csr_base)) {
		dev_err(dev, "devm_platform_ioremap_resource failed\n");
		return PTR_ERR(pdata->csr_base);
	}

	pdata->rtc = devm_rtc_allocate_device(dev);
	if (IS_ERR(pdata->rtc)) {
		dev_err(dev, "devm_rtc_allocate_device failed\n");
		return PTR_ERR(pdata->rtc);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "platform_get_irq failed\n");
		return irq;
	}

	ret = devm_request_irq(dev, irq, dw_rtc_interrupt, 0,
		dev_name(dev), pdata);
	if (ret) {
		dev_err(dev, "Could not request IRQ\n");
		return ret;
	}

	ret = of_property_read_u32(node, "frequency", &pdata->rtc_freq);
	if (ret) {
		pdata->clk = devm_clk_get(dev, NULL);
		if (IS_ERR(pdata->clk)) {
			dev_err(dev, "Couldn't get the clock for RTC\n");
			return -ENODEV;
		}
		ret = clk_prepare_enable(pdata->clk);
		if (ret)
			return ret;
		pdata->rtc_freq = clk_get_rate(pdata->clk);
	}
	pdata->np = node;

	/* enable rtc alarm irq */
	dw_rtc_alarm_irq_enable(&pdev->dev, ENABLE);
	ret = device_init_wakeup(&pdev->dev, ENABLE);
	if (ret)
		goto out_clk;
	pdata->rtc->ops = &dw_rtc_ops;
	pdata->rtc->range_max = U32_MAX;
#ifdef RTC_DEBUG
	pmic_register_alarm_callback(pmic_rtc_callback);
#endif
	ret = devm_rtc_register_device(pdata->rtc);
	if (ret)
		goto out_clk;
	ret = get_crg_info(pdata);
	if (ret) {
		dev_err(dev, "Couldn't get info of rtc_crg from dts\n");
		return ret;
	}
	dev_info(dev, "enter rtc probe succ,clk_freq:%d\n", pdata->rtc_freq);
	return 0;
out_clk:
	if (pdata->clk)
		clk_disable_unprepare(pdata->clk);
	return ret;
}

static int dw_rtc_remove(struct platform_device *pdev)
{
	struct dw_rtc_dev *pdata = platform_get_drvdata(pdev);

	if (!pdata) {
		dev_err(&pdev->dev, "get drvdata failed\n");
		return -EINVAL;
	}
	dw_rtc_alarm_irq_enable(&pdev->dev, DISABLE);
	device_init_wakeup(&pdev->dev, DISABLE);
	if (pdata->clk)
		clk_disable_unprepare(pdata->clk);
	return 0;
}

static int dw_rtc_cnt_refresh(struct dw_rtc_dev *pdata)
{
	uint32_t current_time;
	uint32_t current_count;
	uint32_t alarm_count;
	uint32_t sec = 0;
	struct rtc_time tm;
	int ret;

	alarm_count = readl(pdata->csr_base + DW_APB_RTC_CMR);
	current_count = readl(pdata->csr_base + DW_APB_RTC_CCVR);
	/* rtc assert */
	ret = reset_control_assert(pdata->rst);
	if (ret < 0) {
		dev_err(pdata->dev, "Couldn't assert rst control: %d\n", ret);
		return ret;
	}

	ret = pmic_rtc_gettime(&tm);
	if (ret) {
		current_time = current_count + g_rtc_saved;
	} else {
		sec = rtc_tm_to_time64(&tm);
		current_time = SEC_TO_CNT(sec, pdata->rtc_freq);
	}

	/* disable rtc clk */
	writel(BIT(LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_RTC1_SHIFT), pdata->crg_addr + LMS_CRG_CLKGT0_W1C);

	/* set info to rtc register */
	if (current_count < alarm_count) {
		alarm_count += g_rtc_saved;
		writel(alarm_count, pdata->csr_base + DW_APB_RTC_CMR);
	}
	g_rtc_saved = 0;
	writel(DEFAULT_CPSR_VAL, pdata->csr_base + DW_APB_RTC_CPSR);
	writel(current_time, pdata->csr_base + DW_APB_RTC_CLR);

	/* enable rtc clk */
	writel(BIT(LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_RTC1_SHIFT), pdata->crg_addr + LMS_CRG_CLKGT0_W1S);
	mdelay(3);

	/* rtc reset deassert */
	ret = reset_control_deassert(pdata->rst);
	if (ret < 0) {
		dev_err(pdata->dev, "Couldn't deassert rst control: %d\n", ret);
		return ret;
	}
	mdelay(3);
	return 0;
}

static int __maybe_unused dw_rtc_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_rtc_dev *pdata = platform_get_drvdata(pdev);
	int irq;
	int ret;

	dev_info(dev, "%s ++\n", __func__);

	if (!pdata) {
		dev_err(dev, "get drvdata failed\n");
		return -EINVAL;
	}
	ret = dw_rtc_cnt_refresh(pdata);
	if (ret) {
		dev_err(dev, "failed to initialize rtc\n");
		return -EFAULT;
	}
	irq = platform_get_irq(pdev, 0);
	if (device_may_wakeup(&pdev->dev)) {
		if (!enable_irq_wake(irq))
			pdata->irq_wake = 1;
	} else {
		pdata->irq_enabled = dw_rtc_alarm_irq_enabled(dev);
		dw_rtc_alarm_irq_enable(dev, 0);
		if (pdata->clk)
			clk_disable_unprepare(pdata->clk);
	}

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static int __maybe_unused dw_rtc_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_rtc_dev *pdata = platform_get_drvdata(pdev);
	int irq;
	int rc;

	dev_info(dev, "%s ++\n", __func__);

	if (!pdata) {
		dev_err(dev, "get drvdata failed\n");
		return -EINVAL;
	}
	irq = platform_get_irq(pdev, 0);
	if (device_may_wakeup(&pdev->dev)) {
		if (pdata->irq_wake) {
			disable_irq_wake(irq);
			pdata->irq_wake = 0;
		}
	} else {
		if (pdata->clk) {
			rc = clk_prepare_enable(pdata->clk);
			if (rc) {
				dev_err(dev, "Unable to enable clock error %d\n", rc);
				return rc;
			}
		}
		dw_rtc_alarm_irq_enable(dev, pdata->irq_enabled);
	}

	dev_info(dev, "%s --\n", __func__);

	return 0;
}

static SIMPLE_DEV_PM_OPS(dw_rtc_pm_ops, dw_rtc_suspend, dw_rtc_resume);
#ifdef CONFIG_OF
static const struct of_device_id dw_rtc_of_match[] = {
	{ .compatible = "xring,dw_apb_rtc" },
	{}
};
MODULE_DEVICE_TABLE(of, dw_rtc_of_match);
#endif
static struct platform_driver dw_rtc_driver = {
	.probe = dw_rtc_probe,
	.remove = dw_rtc_remove,
	.driver = {
		.name = "dw_apb_rtc",
		.pm = &dw_rtc_pm_ops,
		.of_match_table = of_match_ptr(dw_rtc_of_match),
	},
};
module_platform_driver(dw_rtc_driver);

MODULE_SOFTDEP("pre: xr_pmic");
MODULE_DESCRIPTION("DW_APB_RTC RTC driver");
MODULE_AUTHOR("qinyifei <qinyifei@xiaomi.com>");
MODULE_LICENSE("GPL");
