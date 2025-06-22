// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/pwm.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/random.h>
#include <linux/types.h>
#include <linux/of_device.h>

#define PWM_DUTY_CONFIG	_IOW('L', 0, int)
#define PWM_FREQ_CONFIG _IOW('L', 1, int)
#define PWM_STEP_CONFIG _IOW('L', 2, int)
#define PWM_RATIO_CONFIG _IOW('L', 3, int)
#define PWM_POLARITY_CONFIG _IOW('L', 4, int)
#define PWM_DEVICE_INFO _IOR('L', 5, int)

/*step:6bit,duty:50%,period:100hz*/
#define PWM_FREQ_DEFAULT (100)
#define PWM_PERIOD_NS_DEFAULT	(NSEC_PER_SEC / PWM_FREQ_DEFAULT)
#define M_BIT_DEFAULT			(6)
#define RATIO_DEFAULT			(50)
#define POLARITY_DEFAULT		(0)
#define ENLARGEMENT_FACTOR	(100)

struct rw_reg_data {
	__be32 reg[4];
	int num;
	int value;
};

struct my_pwm_state {
	int duty_cycle;
	int period;
	int step_accuracy;
	int ratio;
	bool polarity;
	bool is_default;
};

struct pwm_device *pwm_dev;

static long pwm_test_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

const struct file_operations xring_pwm_test_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = pwm_test_ioctl
};
struct miscdevice xring_pwm_test = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "xring_pwm_test",
	.fops = &xring_pwm_test_fops
};

static int pwm_polarity_config(struct my_pwm_state *user_state,
			struct pwm_device *pwm_dev, struct pwm_state *pwm_state)
{
	int error;

	if (user_state->is_default) {
		pwm_state->period = PWM_PERIOD_NS_DEFAULT;
		pwm_state->duty_cycle = RATIO_DEFAULT * pwm_state->period / ENLARGEMENT_FACTOR;
		pwm_state->polarity = user_state->polarity;
		user_state->step_accuracy = M_BIT_DEFAULT;
	} else {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = user_state->polarity;
	}
	pwm_dev->chip_data = &user_state->step_accuracy;
	pwm_state->enabled = true;
	error = pwm_apply_state(pwm_dev, pwm_state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	return 0;
}

static int pwm_duty_config(struct my_pwm_state *user_state,
			struct pwm_device *pwm_dev, struct pwm_state *pwm_state)
{
	int error;

	if (user_state->is_default) {
		pwm_state->period = PWM_PERIOD_NS_DEFAULT;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = POLARITY_DEFAULT;
		user_state->step_accuracy = M_BIT_DEFAULT;
	} else {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = user_state->polarity;
	}
	pwm_dev->chip_data = &user_state->step_accuracy;
	pwm_state->enabled = true;
	error = pwm_apply_state(pwm_dev, pwm_state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	return 0;
}

static int pwm_freq_config(struct my_pwm_state *user_state,
			struct pwm_device *pwm_dev, struct pwm_state *pwm_state)
{
	int error;

	if (user_state->is_default) {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = RATIO_DEFAULT * pwm_state->period / ENLARGEMENT_FACTOR;
		pwm_state->polarity = POLARITY_DEFAULT;
		user_state->step_accuracy = M_BIT_DEFAULT;
	} else {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = user_state->polarity;
	}
	pwm_dev->chip_data = &user_state->step_accuracy;
	pwm_state->enabled = true;
	error = pwm_apply_state(pwm_dev, pwm_state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	return 0;
}

static int pwm_step_config(struct my_pwm_state *user_state,
			struct pwm_device *pwm_dev, struct pwm_state *pwm_state)
{
	int error;

	if (user_state->is_default) {
		pwm_state->period = PWM_PERIOD_NS_DEFAULT;
		pwm_state->duty_cycle = RATIO_DEFAULT * pwm_state->period / ENLARGEMENT_FACTOR;
		pwm_state->polarity = POLARITY_DEFAULT;
	} else {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = user_state->polarity;
	}
	pwm_dev->chip_data = &user_state->step_accuracy;
	pwm_state->enabled = true;
	error = pwm_apply_state(pwm_dev, pwm_state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	return 0;
}

static int pwm_ratio_config(struct my_pwm_state *user_state,
			struct pwm_device *pwm_dev, struct pwm_state *pwm_state)
{
	int error;

	if (user_state->is_default) {
		pwm_state->period = PWM_PERIOD_NS_DEFAULT;
		pwm_state->duty_cycle = user_state->ratio * pwm_state->period / ENLARGEMENT_FACTOR;
		pwm_state->polarity = POLARITY_DEFAULT;
		user_state->step_accuracy = M_BIT_DEFAULT;
	} else {
		pwm_state->period = user_state->period;
		pwm_state->duty_cycle = user_state->duty_cycle;
		pwm_state->polarity = user_state->polarity;
	}
	pwm_dev->chip_data = &user_state->step_accuracy;
	pwm_state->enabled = true;
	error = pwm_apply_state(pwm_dev, pwm_state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	return 0;
}

static void pwm_state_initialize(struct pwm_state *state)
{
	state->period = PWM_PERIOD_NS_DEFAULT;
	state->duty_cycle = 0;
	state->enabled = false;
}

static long pwm_test_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	int error;
	struct my_pwm_state user_state;
	struct pwm_state state;
	struct rw_reg_data reg_data;
	struct device_node *node;

	/* Sync up PWM state and ensure it is off.*/
	pwm_init_state(pwm_dev, &state);
	pwm_state_initialize(&state);
	user_state.step_accuracy = M_BIT_DEFAULT;
	pwm_dev->chip_data = &user_state.step_accuracy;
	error = pwm_apply_state(pwm_dev, &state);
	if (error) {
		pr_err("failed to apply initial PWM state:%d\n", error);
		return error;
	}
	switch (cmd) {
	case PWM_DUTY_CONFIG:
		pr_info("test mode is pwm_duty_config\n");
		if (copy_from_user(&user_state, (struct my_pwm_state *)arg,
			sizeof(struct my_pwm_state))) {
			ret = -EFAULT;
			break;
		}
		ret = pwm_duty_config(&user_state, pwm_dev, &state);
		if (ret) {
			pr_err("pwm_duty_config test failed\n");
			return -1;
		}
		pr_info("pwm_duty_config test succeed\n");
		break;
	case PWM_FREQ_CONFIG:
		pr_info("test mode is pwm_freq_config\n");
		if (copy_from_user(&user_state, (struct my_pwm_state *)arg,
			sizeof(struct my_pwm_state))) {
			ret = -EFAULT;
			break;
		}
		ret = pwm_freq_config(&user_state, pwm_dev, &state);
		if (ret) {
			pr_err("pwm_freq_config test failed\n");
			return -1;
		}
		pr_info("pwm_freq_config test succeed\n");
		break;
	case PWM_STEP_CONFIG:
		pr_info("test mode is pwm_step_config\n");
		if (copy_from_user(&user_state, (struct my_pwm_state *)arg,
			sizeof(struct my_pwm_state))) {
			ret = -EFAULT;
			break;
		}
		ret = pwm_step_config(&user_state, pwm_dev, &state);
		if (ret) {
			pr_err("pwm_step_config test failed\n");
			return -1;
		}
		pr_info("pwm_step_config test succeed\n");
		break;
	case PWM_RATIO_CONFIG:
		pr_info("test mode is pwm_ratio_config\n");
		if (copy_from_user(&user_state, (struct my_pwm_state *)arg,
			sizeof(struct my_pwm_state))) {
			ret = -EFAULT;
			break;
		}
		ret = pwm_ratio_config(&user_state, pwm_dev, &state);
		if (ret) {
			pr_err("pwm_ratio_config test failed\n");
			return -1;
		}
		pr_info("pwm_ratio_config test succeed\n");
		break;
	case PWM_POLARITY_CONFIG:
		pr_info("test mode is pwm_polarity_config\n");
		if (copy_from_user(&user_state, (struct my_pwm_state *)arg,
			sizeof(struct my_pwm_state))) {
			ret = -EFAULT;
			break;
		}
		ret = pwm_polarity_config(&user_state, pwm_dev, &state);
		if (ret) {
			pr_err("pwm_polarity_config test failed\n");
			return -1;
		}
		pr_info("pwm_polarity_config test succeed\n");
		break;
	case PWM_DEVICE_INFO:
		node = of_find_node_by_name(NULL, "pwm");
		if (!node) {
			pr_err("no dev\n");
			return -ENODEV;
		}
		if (of_property_read_u32_array(node, "reg", reg_data.reg, 4) != 0) {
			pr_err("get reg_data fail\n");
			return -EINVAL;
		}
		if (copy_to_user((unsigned int __user *)arg, &reg_data.reg,
			sizeof(unsigned int) * 4)) {
			pr_err("copy_to_user dev_base fail\n");
			ret = -EFAULT;
			break;
		}
		break;
	default:
		pr_err("Select test mode please.\n");
		break;
	}
	return 0;
}

static int pwm_test_probe(struct platform_device *pdev)
{
	int ret;

	pwm_dev = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pwm_dev))
		return PTR_ERR(pwm_dev);
	ret = misc_register(&xring_pwm_test);
	if (ret < 0) {
		pr_err("pwm test device register error.\n");
		return ret;
	}
	pr_info("pwm test device registered\n");
	return 0;
}

static int pwm_test_remove(struct platform_device *pdev)
{
	misc_deregister(&xring_pwm_test);
	pr_info("pwm test device unregistered\n");
	return 0;
}

static const struct of_device_id pwm_test_of_match[] = {
	{ .compatible = "xring,pwm_test", },
	{ },
};
MODULE_DEVICE_TABLE(of, pwm_test_of_match);

static struct platform_driver pwm_test_driver = {
	.probe = pwm_test_probe,
	.remove = pwm_test_remove,
	.driver = {
		.name	= "xring-pwm-test",
		.of_match_table = pwm_test_of_match,
	},
};
module_platform_driver(pwm_test_driver);

MODULE_SOFTDEP("pre: xring_pwm");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware pwm test for Xring");
