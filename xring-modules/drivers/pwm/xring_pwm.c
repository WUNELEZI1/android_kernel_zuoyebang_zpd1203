// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2007 Ben Dooks
 * Copyright (c) 2008 Simtec Electronics
 *     Ben Dooks <ben@simtec.co.uk>, <ben-linux@fluff.org>
 * Copyright (c) 2013 Tomasz Figa <tomasz.figa@gmail.com>
 * Copyright (c) 2017 xring Electronics Co., Ltd.
 *
 * PWM driver for xring SoCs
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/export.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/reset.h>
#include <dt-bindings/xring/platform-specific/pwm_reg.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>

#define PWM_CR_POLARITY_HIGH	1
#define PWM_CR_ENABLE			1

#define ENLARGEMENT_FACTOR		100
#define CLK_FREQ			278530000
#define PWM_FREQ_MIN			100
#define PWM_FREQ_MAX			100000
#define PWM_FREQ_LIMIT			60000
#define PWM_STEP_ACCURACY_MIN	6
#define PWM_STEP_DEFAULT		8
#define PWM_STEP_ACCURACY_MAX	12
#define PWM_STEP_ACCURACY_LIMIT	11
#define PWM_NUM					1

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_PWM:%s:%d " fmt, __func__, __LINE__

/**
 * struct xring_pwm_chip - private data of PWM chip
 * @chip:		generic PWM chip
 * @base:		base address of mapped PWM registers
 * @base_clk:		base clock used to drive the timers
 */
struct xring_pwm_chip {
	struct pwm_chip chip;
	void __iomem *base;
	struct clk *base_clk;
	struct clk *base_pclk;
	spinlock_t lock;
	struct reset_control *rst;
};

/**
 * struct xring_pwm_channel - private data of PWM channel
 * @period_ns:	current period in nanoseconds programmed to the hardware
 * @duty_ns:	current duty time in nanoseconds programmed to the hardware
 * @step_accuracy:	it affects the accuracy of the pwm
 */
struct xring_pwm_channel {
	int period_ns;
	int duty_ns;
	int step_accuracy;
};

/**
 * struct xring_pwm_register_data - data of pwm register
 * @pwm_period:  the value of pwm_period register
 * @step_period:  the value of step_period register
 * @pulse_width_value:  the value of pulse_width_value register
 * @pwm_cr:  the value of pwm_cr register
 */
struct xring_pwm_register_data {
	int pwm_period;
	int step_period;
	int pulse_width_value;
	int pwm_cr;
};

struct xring_pwm_register_data *reg_data;
static int my_pow(int x, int y)
{
	int result = 1;
	int i;

	for (i = 0; i < y; i++)
		result *= x;

	return result;
}

static inline
struct xring_pwm_chip *to_xring_pwm_chip(struct pwm_chip *chip)
{
	return container_of(chip, struct xring_pwm_chip, chip);
}

static int xring_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct xring_pwm_chip *xring_chip;
	unsigned long flags;
	u32 tcon;

	if (chip == NULL) {
		pr_err("Structure pointer pwm_chip should not be empty\n");
		return -EINVAL;
	}
	if (pwm == NULL) {
		pr_err("Structure pointer pwm_device should not be empty\n");
		return -EINVAL;
	}

	xring_chip = to_xring_pwm_chip(chip);
	if (xring_chip == NULL) {
		pr_err("Failed to obtain the chip of the pwm\n");
		return -ENODEV;
	}
	spin_lock_irqsave(&xring_chip->lock, flags);
	tcon = readl(xring_chip->base + PWM_PWM_CR);
	tcon |= BIT(PWM_CR_ENABLE);
	writel(tcon, xring_chip->base + PWM_PWM_CR);
	spin_unlock_irqrestore(&xring_chip->lock, flags);

	return 0;
}

static void xring_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct xring_pwm_chip *xring_chip;
	unsigned long flags;
	u32 tcon;

	if (chip == NULL) {
		pr_err("Structure pointer pwm_chip should not be empty\n");
		return;
	}
	if (pwm == NULL) {
		pr_err("Structure pointer pwm_device should not be empty\n");
		return;
	}

	xring_chip = to_xring_pwm_chip(chip);
	if (xring_chip == NULL) {
		pr_err("Failed to obtain the chip of the pwm\n");
		return;
	}
	spin_lock_irqsave(&xring_chip->lock, flags);
	tcon = readl(xring_chip->base + PWM_PWM_CR);
	tcon &= ~BIT(PWM_CR_ENABLE);
	writel(tcon, xring_chip->base + PWM_PWM_CR);
	spin_unlock_irqrestore(&xring_chip->lock, flags);
}

/*Calculated PWM period*/
static int calc_pwm_period(unsigned long clk_freq, unsigned long pwm_freq)
{
	if (pwm_freq == 0) {
		pr_err("The dividend cannot be 0\n");
		return -EINVAL;
	} else {
		return (clk_freq / pwm_freq);
	}
}

/*Calculate the period value of the stepping precision*/
static int calc_step_period(unsigned long clk_freq, unsigned long pwm_freq, int step_accuracy)
{
	unsigned long temp;

	temp = pwm_freq * (my_pow(2, step_accuracy) - 1);
	if (temp == 0) {
		pr_err("The dividend cannot be 0\n");
		return -EINVAL;
	} else {
		return (clk_freq / temp);
	}
}

/*Calculate the PWM pulse width period value*/
static int calc_pulse_width_value(int pwm_period, int step_period, int ratio)
{
	if (step_period == 0 || ENLARGEMENT_FACTOR == 0) {
		pr_err("The dividend cannot be 0\n");
		return -EINVAL;
	} else {
		return (ratio * pwm_period / step_period / ENLARGEMENT_FACTOR);
	}
}

static int xring_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
			      int duty_ns, int period_ns)
{
	unsigned long clk_rate;
	unsigned long pwm_freq;
	unsigned long flags;
	int pwm_period, step_period, pulse_width_value, ratio;
	struct xring_pwm_chip *xring_chip;
	struct xring_pwm_channel xring_chan;

	if (chip == NULL) {
		pr_err("Structure pointer pwm_chip should not be empty\n");
		return -EINVAL;
	}
	if (pwm == NULL) {
		pr_err("Structure pointer pwm_device should not be empty\n");
		return -EINVAL;
	}

	xring_chip = to_xring_pwm_chip(chip);
	if (xring_chip == NULL) {
		pr_err("Failed to obtain the chip of the pwm\n");
		return -ENODEV;
	}

	if (pwm_get_chip_data(pwm) == NULL)
		xring_chan.step_accuracy = PWM_STEP_DEFAULT;
	else
		xring_chan.step_accuracy = *(int *)pwm_get_chip_data(pwm);
	if (period_ns > NSEC_PER_SEC || duty_ns > period_ns) {
		dev_err(xring_chip->chip.dev, "Invalid period_ns(%d) or duty_ns(%d), please check it\n"
			, period_ns, duty_ns);
		return -EINVAL;
	}
	if (xring_chan.step_accuracy < PWM_STEP_ACCURACY_MIN
		|| xring_chan.step_accuracy > PWM_STEP_ACCURACY_MAX) {
		dev_err(xring_chip->chip.dev, "Invalid step_accuracy(%d), please check it\n"
			, xring_chan.step_accuracy);
		return -EINVAL;
	}
	xring_chan.duty_ns = duty_ns;
	xring_chan.period_ns = period_ns;

	clk_rate = clk_get_rate(xring_chip->base_clk);
	pwm_freq = NSEC_PER_SEC / period_ns;
	ratio = ENLARGEMENT_FACTOR * duty_ns / period_ns;
	if (pwm_freq < PWM_FREQ_MIN || pwm_freq > PWM_FREQ_MAX) {
		dev_err(xring_chip->chip.dev, "Invalid pwm_freq(%lx), please check it\n", pwm_freq);
		return -EINVAL;
	}
	if (xring_chan.step_accuracy == PWM_STEP_ACCURACY_MAX && pwm_freq > PWM_FREQ_LIMIT) {
		dev_err(xring_chip->chip.dev,
			"When the duty cycle accuracy is %dbit, the PWM frequency cannot be greater than %dHz\n",
				xring_chan.step_accuracy, PWM_FREQ_LIMIT);
		return -EINVAL;
	}
	if (pwm_freq >= PWM_FREQ_LIMIT && pwm_freq <= PWM_FREQ_MAX
		&& xring_chan.step_accuracy > PWM_STEP_ACCURACY_LIMIT) {
		dev_err(xring_chip->chip.dev,
			"When the PWM frequency is %dHz-%dHz, duty cycle accuracy cannot exceed %dbit\n",
				PWM_FREQ_LIMIT, PWM_FREQ_MAX, PWM_STEP_ACCURACY_LIMIT);
		return -EINVAL;
	}
	pwm_period = calc_pwm_period(clk_rate, pwm_freq);
	step_period = calc_step_period(clk_rate, pwm_freq, xring_chan.step_accuracy);
	pulse_width_value = calc_pulse_width_value(pwm_period, step_period, ratio);
	dev_dbg(xring_chip->chip.dev, "clk_rate:%lu, pwm_freq:%lu, step_accuracy:%d, ratio:%d\n",
		clk_rate, pwm_freq, xring_chan.step_accuracy, ratio);
	dev_dbg(xring_chip->chip.dev, "pwm_period=%d, step_period=%d, pulse_width_value=%d\n",
		pwm_period, step_period, pulse_width_value);

	spin_lock_irqsave(&xring_chip->lock, flags);
	writel(pwm_period - 1, xring_chip->base + PWM_PWM_PCVR);
	writel(step_period - 1, xring_chip->base + PWM_PWM_SCVR);
	writel(pulse_width_value, xring_chip->base + PWM_PWM_PWPCVR);
	spin_unlock_irqrestore(&xring_chip->lock, flags);
	return 0;
}

static int xring_pwm_set_polarity(struct pwm_chip *chip,
				    struct pwm_device *pwm,
				    enum pwm_polarity polarity)
{
	struct xring_pwm_chip *xring_chip;
	u32 val;
	unsigned long flags;

	if (chip == NULL) {
		pr_err("Structure pointer pwm_chip should not be empty\n");
		return -EINVAL;
	}
	if (pwm == NULL) {
		pr_err("Structure pointer pwm_device should not be empty\n");
		return -EINVAL;
	}

	xring_chip = to_xring_pwm_chip(chip);
	if (xring_chip == NULL) {
		pr_err("Failed to obtain the chip of the pwm\n");
		return -ENODEV;
	}
	spin_lock_irqsave(&xring_chip->lock, flags);
	val = readl(xring_chip->base + PWM_PWM_CR);
	if (polarity != PWM_POLARITY_INVERSED)
		val &= ~PWM_CR_POLARITY_HIGH;
	else
		val |= PWM_CR_POLARITY_HIGH;
	writel(val, xring_chip->base + PWM_PWM_CR);
	spin_unlock_irqrestore(&xring_chip->lock, flags);
	return 0;
}

static int xring_pwm_apply(struct pwm_chip *chip, struct pwm_device *pwm,
			      const struct pwm_state *state)
{
	int err;

	if (state->polarity != pwm->state.polarity) {
		if (pwm->state.enabled) {
			xring_pwm_disable(chip, pwm);
			pwm->state.enabled = false;
		}

		err = xring_pwm_set_polarity(chip, pwm, state->polarity);
		if (err)
			return err;

		pwm->state.polarity = state->polarity;
	}
	if (state->period != pwm->state.period ||
		state->duty_cycle != pwm->state.duty_cycle) {
		err = xring_pwm_config(pwm->chip, pwm,
					state->duty_cycle,
					state->period);
		if (err)
			return err;

		pwm->state.duty_cycle = state->duty_cycle;
		pwm->state.period = state->period;
	}
	if (state->enabled != pwm->state.enabled) {
		if (state->enabled) {
			err = xring_pwm_enable(chip, pwm);
			if (err)
				return err;
		} else {
			xring_pwm_disable(chip, pwm);
		}
		pwm->state.enabled = state->enabled;
	}
	return 0;
}

static const struct pwm_ops xring_pwm_ops = {
	.apply			= xring_pwm_apply,
	.owner			= THIS_MODULE,
};

static int xring_pwm_crg_config(struct device *dev)
{
	int ret;
	struct xring_pwm_chip *chip = dev_get_drvdata(dev);

	ret = reset_control_deassert(chip->rst);
	if (ret < 0) {
		dev_err(dev, "%s Couldn't deassert rst control: %d\n", __func__, ret);
		return ret;
	}

	ret = clk_prepare_enable(chip->base_pclk);
	if (ret < 0) {
		dev_err(dev, "failed to enable pwm_pclk\n");
		return ret;
	}

	ret = clk_prepare_enable(chip->base_clk);
	if (ret < 0)
		dev_err(dev, "failed to enable pwm_clk\n");

	ret = clk_set_rate(chip->base_clk, CLK_FREQ);
	if (ret < 0)
		dev_err(dev, "failed to set pwm_clk rate\n");

	return ret;
}

static struct pwm_device *xring_pwm_xlate(struct pwm_chip *chip,
					     const struct of_phandle_args *args)
{
	if (args->args[0] >= chip->npwm)
		return ERR_PTR(-EINVAL);

	return pwm_request_from_chip(chip, args->args[0], NULL);
}

static int xring_pwm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xring_pwm_chip *chip;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;
	reg_data = devm_kcalloc(&pdev->dev, 1, sizeof(*reg_data), GFP_KERNEL);
	if (!reg_data)
		return -ENOMEM;
	spin_lock_init(&chip->lock);
	chip->chip.dev = &pdev->dev;
	chip->chip.ops = &xring_pwm_ops;
	chip->chip.npwm = PWM_NUM;
	chip->chip.of_xlate = xring_pwm_xlate;

	chip->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(chip->base))
		return PTR_ERR(chip->base);
	chip->base_pclk = devm_clk_get(&pdev->dev, "pclk_pwm1_abrg");
	if (IS_ERR(chip->base_pclk)) {
		dev_err(dev, "failed to get pwm pclk\n");
		return PTR_ERR(chip->base_pclk);
	}
	chip->base_clk = devm_clk_get(&pdev->dev, "clk_pwm1");
	if (IS_ERR(chip->base_clk)) {
		dev_err(dev, "failed to get pwm clk\n");
		return PTR_ERR(chip->base_clk);
	}

	chip->rst = devm_reset_control_get_exclusive(&pdev->dev, "rst");
	if (IS_ERR(chip->rst))  {
		dev_err(&pdev->dev, "Couldn't get rst control\n");
		return PTR_ERR(chip->rst);
	}
	platform_set_drvdata(pdev, chip);
	xring_pwm_crg_config(dev);
	ret = pwmchip_add(&chip->chip);
	if (ret < 0) {
		dev_err(dev, "failed to register PWM chip\n");
		clk_disable_unprepare(chip->base_clk);
		clk_disable_unprepare(chip->base_pclk);
		return ret;
	}

	dev_dbg(dev, "base_clk at %lu\n", clk_get_rate(chip->base_clk));
	return 0;
}

static int xring_pwm_remove(struct platform_device *pdev)
{
	struct xring_pwm_chip *chip = platform_get_drvdata(pdev);

	pwmchip_remove(&chip->chip);
	if (!IS_ERR(chip->base_pclk))
		clk_disable_unprepare(chip->base_pclk);
	if (!IS_ERR(chip->base_clk))
		clk_disable_unprepare(chip->base_clk);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int xring_pwm_suspend(struct device *dev)
{
	struct xring_pwm_chip *our_chip = dev_get_drvdata(dev);

	dev_info(dev, "%s ++\n", __func__);

	reg_data->pwm_cr = readl(our_chip->base + PWM_PWM_CR);
	reg_data->pwm_period = readl(our_chip->base + PWM_PWM_PCVR);
	reg_data->step_period = readl(our_chip->base + PWM_PWM_SCVR);
	reg_data->pulse_width_value = readl(our_chip->base + PWM_PWM_PWPCVR);

	if (!IS_ERR(our_chip->base_pclk))
		clk_disable_unprepare(our_chip->base_pclk);
	if (!IS_ERR(our_chip->base_clk))
		clk_disable_unprepare(our_chip->base_clk);

	dev_info(dev, "%s --\n", __func__);
	return 0;
}

static int xring_pwm_resume(struct device *dev)
{
	unsigned long flags;
	struct xring_pwm_chip *our_chip = dev_get_drvdata(dev);

	dev_info(dev, "%s ++\n", __func__);

	xring_pwm_crg_config(dev);

	spin_lock_irqsave(&our_chip->lock, flags);
	writel(reg_data->pwm_cr, our_chip->base + PWM_PWM_CR);
	writel(reg_data->pwm_period, our_chip->base + PWM_PWM_PCVR);
	writel(reg_data->step_period, our_chip->base + PWM_PWM_SCVR);
	writel(reg_data->pulse_width_value, our_chip->base + PWM_PWM_PWPCVR);
	spin_unlock_irqrestore(&our_chip->lock, flags);
	dev_info(dev, "%s --\n", __func__);
	return 0;
}
#endif
static SIMPLE_DEV_PM_OPS(xring_pwm_pm_ops, xring_pwm_suspend, xring_pwm_resume);

#ifdef CONFIG_OF
static const struct of_device_id xring_pwm_matches[] = {
	{ .compatible = "xring,pwm",},
	{},
};
MODULE_DEVICE_TABLE(of, xring_pwm_matches);
#endif

static struct platform_driver xring_pwm_driver = {
	.driver		= {
		.name	= "xring-pwm",
		.pm	= &xring_pwm_pm_ops,
		.of_match_table = of_match_ptr(xring_pwm_matches),
	},
	.probe		= xring_pwm_probe,
	.remove		= xring_pwm_remove,
};
module_platform_driver(xring_pwm_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PWM driver");
MODULE_ALIAS("platform:xring-pwm");
