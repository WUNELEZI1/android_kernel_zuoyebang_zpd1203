// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2010, 2011, 2016 The Linux Foundation. All rights reserved.
 */
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/regmap.h>
#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
#include <dt-bindings/xring/platform-specific/pmic/rgb_resource.h>
#include <linux/compiler_attributes.h>

#define XR_PMIC_MAX_LEDS 3
#define XR_MAX_PERIOD 8000
#define FREQ_FACTOR 1000
#define FREQ_NUM 8
#define FREQ_MUTATION 7
#define FREQ_STEP_SIZE 2
#define XR_DEFAULT_PERIOD 500
#define XR_BASE_PERIOD 125
#define XR_BASE2_PERIOD 64000
#define XR_BREATH_PERIOD 128000
#define DUTY_SIZE 128
#define MS_PER_S 1000
#define BREATH_PARA_NUM 6

#define RGB_CURRENT_01_mA_MIN LED_ON
#define RGB_CURRENT_01_mA_MAX 36
#define RGB_CURRENT_03_mA_MIN 37
#define RGB_CURRENT_03_mA_MAX 72
#define RGB_CURRENT_05_mA_MIN 73
#define RGB_CURRENT_05_mA_MAX 107
#define RGB_CURRENT_07_mA_MIN 108
#define RGB_CURRENT_07_mA_MAX 142
#define RGB_CURRENT_09_mA_MIN 143
#define RGB_CURRENT_09_mA_MAX 177
#define RGB_CURRENT_11_mA_MIN 178
#define RGB_CURRENT_11_mA_MAX 212
#define RGB_CURRENT_13_mA_MIN 213
#define RGB_CURRENT_13_mA_MAX LED_FULL

#define GET_RGB_CTRL_EN_ADDR TOP_REG_PWM_CTRL_EN_REG

#define RGB_DISABLE 0
#define RGB_ENABLE 1
#define RGB_ID_RED 0
#define RGB_ID_GREEN 1
#define XR_MAX_BRIGHTNESS 13

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_PMICRGB:%s:%d " fmt, __func__, __LINE__

struct xr_pmic_leds;
char *rgb_name[XR_PMIC_MAX_LEDS] = {"led0", "led1", "led2"};

/**
 * struct xr_pmic_led - state container for the LED device
 * @id:			the identifier in xr_pmic LED device
 * @parent:		the pointer to xr_pmic LED controller
 * @cdev:		LED class device for this LED device
 * @current_brightness: current state of the LED device
 * @regmap:		Pointer to the register map for the LED
 * @pwm_freq:	PWM frequency for the LED
 * @toff:		Time for the LED to be off
 * @tr1:		Time for rising edge of the first pulse
 * @tr2:		Time for rising edge of the second pulse
 * @ton:		Time for the LED to be on
 * @tf1:		Time for falling edge of the first pulse
 * @tf2:		Time for falling edge of the second pulse
 * @mode:		Mode of operation for the LED
 * @dimming:	Dimming level for the LED
 * @base:		Base address for the LED
 */
struct xr_pmic_led {
	int			id;
	struct xr_pmic_leds	*parent;
	struct led_classdev	cdev;
	enum led_brightness	current_brightness;
	struct regmap	*regmap;
	enum rgb_pwm_freq pwm_freq;
	enum rgb_toff toff;
	enum rgb_ton_tr_tf tr1;
	enum rgb_ton_tr_tf tr2;
	enum rgb_ton_tr_tf ton;
	enum rgb_ton_tr_tf tf1;
	enum rgb_ton_tr_tf tf2;
	enum rgb_mode mode;
	uint32_t dimming;
	uint32_t base;
};

/**
 * struct xr_pmic_leds -	state container for holding LED controller
 *			of the driver
 * @dev:		the device pointer
 * @lock:		the lock among process context
 * @led:		the array that contains the state of individual LED device
 */
struct xr_pmic_leds {
	struct device		*dev;
	/* protect among process context */
	struct mutex		lock;
	struct xr_pmic_led	*led[XR_PMIC_MAX_LEDS];
};

static int xr_pmic_rgb_read(struct xr_pmic_led *led,
				uint16_t offset, uint8_t *data, int len)
{
	int ret;

	ret = regmap_bulk_read(led->regmap, led->base + offset, data, len);
	pr_debug("base:%x,offset:%x,data:%x,len:%x\n",
			led->base, offset, *data, len);
	return ret;
}

static int xr_pmic_rgb_write(struct xr_pmic_led *led,
				uint16_t offset, uint8_t *data, int len)
{
	int ret;

	pr_debug("base:%x,offset:%x,data:%x,len:%x\n",
		led->base, offset, *data, len);
	ret = regmap_bulk_write(led->regmap, led->base + offset, data, len);
	return ret;
}

static int pmic_rgb_set_tf1_tf2(struct xr_pmic_led *led)
{
	bool is_valid_input;
	uint8_t value;
	int ret = 0;

	is_valid_input = CHECK_RGB_ID(led->id);
	if (!is_valid_input) {
		pr_err("invalid rgb_id :%d\n", led->id);
		return -EINVAL;
	}
	is_valid_input = CHECK_RGB_TON_TR_TF(led->tf1) && CHECK_RGB_TON_TR_TF(led->tf2);
	if (!is_valid_input) {
		pr_err("invalid tf1 :%d, tf2: %d\n", led->tf1, led->tf2);
		return -EINVAL;
	}
	ret = xr_pmic_rgb_read(led, GET_RGB_TF1_TF2_ADDR(led->id), &value, sizeof(value));
	if (ret)
		return ret;
	value = (uint8_t)(led->tf1 << TOP_REG_RGBL_TF1_TF2_REG_SW_RGB1_TF1_SHIFT) |
			(uint8_t)(led->tf2 << TOP_REG_RGBL_TF1_TF2_REG_SW_RGB1_TF2_SHIFT);
	ret = xr_pmic_rgb_write(led, GET_RGB_TF1_TF2_ADDR(led->id), &value, sizeof(value));
	return ret;
}

static int pmic_rgb_set_tr1_tr2(struct xr_pmic_led *led)
{
	bool is_valid_input;
	uint8_t value;
	int ret = 0;

	is_valid_input = CHECK_RGB_ID(led->id);
	if (!is_valid_input) {
		pr_err("invalid rgb_id :%d\n", led->id);
		return -EINVAL;
	}
	is_valid_input = CHECK_RGB_TON_TR_TF(led->tr1) && CHECK_RGB_TON_TR_TF(led->tr2);
	if (!is_valid_input) {
		pr_err("invalid tf1 :%d, tf2: %d\n", led->tr1, led->tr2);
		return -EINVAL;
	}
	ret = xr_pmic_rgb_read(led, GET_RGB_TR1_TR2_ADDR(led->id), &value, sizeof(value));
	if (ret)
		return ret;
	value = (uint8_t)(led->tr1 << TOP_REG_RGBL_TR1_TR2_REG_SW_RGB1_TR1_SHIFT) |
			(uint8_t)(led->tr2 << TOP_REG_RGBL_TR1_TR2_REG_SW_RGB1_TR2_SHIFT);
	ret = xr_pmic_rgb_write(led, GET_RGB_TR1_TR2_ADDR(led->id), &value, sizeof(value));
	return ret;
}

static int pmic_rgb_set_ton_tff(struct xr_pmic_led *led)
{
	bool is_valid_input;
	uint8_t value;
	int ret = 0;

	is_valid_input = CHECK_RGB_ID(led->id);
	if (!is_valid_input) {
		pr_err("invalid rgb_id :%d\n", led->id);
		return -EINVAL;
	}
	is_valid_input = CHECK_RGB_TON_TR_TF(led->ton) && CHECK_RGB_TON_TR_TF(led->toff);
	if (!is_valid_input) {
		pr_err("invalid ton :%d, toff: %d\n", led->ton, led->toff);
		return -EINVAL;
	}
	ret = xr_pmic_rgb_read(led, GET_RGB_TON_TOFF_ADDR(led->id), &value, sizeof(value));
	if (ret)
		return ret;
	value = (uint8_t)(led->ton << TOP_REG_RGBL_TON_TOFF_REG_SW_RGB1_TON_SHIFT) |
			(uint8_t)(led->toff << TOP_REG_RGBL_TON_TOFF_REG_SW_RGB1_TOFF_SHIFT);
	ret = xr_pmic_rgb_write(led, GET_RGB_TON_TOFF_ADDR(led->id), &value, sizeof(value));
	return ret;
}

static int pmic_rgb_set_pwm(struct xr_pmic_led *led)
{
	uint8_t value;
	int ret = 0;

	if (!CHECK_RGB_ID(led->id)) {
		pr_err("invalid rgb_id :%d\n", led->id);
		return -EINVAL;
	}

	if (!CHECK_RGB_PWM(led->pwm_freq)) {
		pr_err("invalid pwm_freq: %d\n", led->pwm_freq);
		return -EINVAL;
	}
	ret = xr_pmic_rgb_read(led, GET_RGB_PWM_ADDR(led->id), &value, sizeof(value));
	if (ret)
		return ret;
	value = (uint8_t)(led->pwm_freq << TOP_REG_RGB1_PWM_FRE_REG_SW_RGB1_PWM_FRE_SHIFT);
	ret = xr_pmic_rgb_write(led, GET_RGB_PWM_ADDR(led->id), &value, sizeof(value));
	return ret;
}

static int pmic_rgb_set_dimming(struct xr_pmic_led *led)
{
	uint8_t value;
	int ret = 0;

	if (!CHECK_RGB_ID(led->id)) {
		pr_err("invalid rgb_id :%d\n", led->id);
		return -EINVAL;
	}

	if (!CHECK_RGB_DIMMING(led->dimming)) {
		pr_err("invalid dimming: %d\n", led->dimming);
		return -EINVAL;
	}
	ret = xr_pmic_rgb_read(led, GET_RGB_DIMMING_ADDR(led->id), &value, sizeof(value));
	if (ret)
		return ret;
	value = (uint8_t)(led->dimming << TOP_REG_RGB1_DIMMING_DUTY_REG_SW_RGB1_DIMMING_DUTY_SHIFT);
	ret = xr_pmic_rgb_write(led, GET_RGB_DIMMING_ADDR(led->id), &value, sizeof(value));
	return ret;
}

static int xr_pmic_led_hw_brightness(struct led_classdev *cdev,
				    enum led_brightness brightness)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	int ret = 0;
	uint8_t current_count;

	if (RGB_CURRENT_01_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_01_mA_MAX) {
		current_count = RGB_CURRENT_01_mA;
	} else if (RGB_CURRENT_03_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_03_mA_MAX) {
		current_count = RGB_CURRENT_03_mA;
	} else if (RGB_CURRENT_05_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_05_mA_MAX) {
		current_count = RGB_CURRENT_05_mA;
	} else if (RGB_CURRENT_07_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_07_mA_MAX) {
		current_count = RGB_CURRENT_07_mA;
	} else if (RGB_CURRENT_09_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_09_mA_MAX) {
		current_count = RGB_CURRENT_09_mA;
	} else if (RGB_CURRENT_11_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_11_mA_MAX) {
		current_count = RGB_CURRENT_11_mA;
	} else if (RGB_CURRENT_13_mA_MIN <= brightness &&  brightness <= RGB_CURRENT_13_mA_MAX) {
		current_count = RGB_CURRENT_13_mA;
	} else {
		pr_err("invaild brightness(%d) input!\n", brightness);
		return -EINVAL;
	}
	pr_debug("brightness is :0x%x!\n", brightness);

	/*
	 * Setup current output for the corresponding
	 * brightness level.
	 */
	ret = xr_pmic_rgb_write(led, GET_RGB_CURRENT_ADDR(led->id),
				&current_count, sizeof(current_count));
	return ret;
}

static int xr_pmic_led_hw_off(struct led_classdev *cdev)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	uint8_t en_reg;
	int ret = 0;

	ret = xr_pmic_rgb_read(led, TOP_REG_PWM_CTRL_EN_REG, &en_reg, sizeof(en_reg));
	if (ret)
		return ret;

	switch (led->id) {
	case RGB_ID_RED:
		en_reg &= (~TOP_REG_PWM_CTRL_EN_REG_SW_RGB1_EN_MASK);
		break;
	case RGB_ID_GREEN:
		en_reg &= (~TOP_REG_PWM_CTRL_EN_REG_SW_RGB2_EN_MASK);
		break;
	default:
		en_reg &= (~TOP_REG_PWM_CTRL_EN_REG_SW_RGB3_EN_MASK);
	}
	ret = xr_pmic_rgb_write(led, TOP_REG_PWM_CTRL_EN_REG, &en_reg, sizeof(en_reg));
	return ret;
}

static enum led_brightness
xr_pmic_get_led_hw_brightness(struct led_classdev *cdev)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	uint8_t status;
	int ret = 0;

	ret = xr_pmic_rgb_read(led, TOP_REG_PWM_CTRL_EN_REG, &status, sizeof(status));
	if (ret < 0)
		return ret;
	switch (led->id) {
	case RGB_ID_RED:
		status &= TOP_REG_PWM_CTRL_EN_REG_SW_RGB1_EN_MASK;
		break;
	case RGB_ID_GREEN:
		status &= TOP_REG_PWM_CTRL_EN_REG_SW_RGB2_EN_MASK;
		break;
	default:
		status &= TOP_REG_PWM_CTRL_EN_REG_SW_RGB3_EN_MASK;
	}
	pr_debug("%s, status:%x\n", __func__, status);
	if (!status)
		return LED_OFF;

	return led->current_brightness;
}

static int xr_pmic_led_hw_on(struct led_classdev *cdev,
			    enum led_brightness brightness)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	uint8_t en_reg;
	uint8_t mode_reg;
	int ret = 0;

	ret = xr_pmic_led_hw_brightness(cdev, brightness);
	if (ret < 0)
		return ret;

	ret = xr_pmic_rgb_read(led, TOP_REG_PWM_CTRL_EN_REG, &en_reg, sizeof(en_reg));
	if (ret)
		return ret;
	ret = xr_pmic_rgb_read(led, TOP_REG_PWM_CTRL_MODE_REG, &mode_reg, sizeof(mode_reg));
	if (ret)
		return ret;
	switch (led->id) {
	case RGB_ID_RED:
		en_reg |= TOP_REG_PWM_CTRL_EN_REG_SW_RGB1_EN_MASK;
		mode_reg &= (~TOP_REG_PWM_CTRL_MODE_REG_SW_RGB1_MODE_MASK);
		mode_reg |= (led->mode << TOP_REG_PWM_CTRL_MODE_REG_SW_RGB1_MODE_SHIFT);
		break;
	case RGB_ID_GREEN:
		en_reg |= TOP_REG_PWM_CTRL_EN_REG_SW_RGB2_EN_MASK;
		mode_reg &= (~TOP_REG_PWM_CTRL_MODE_REG_SW_RGB2_MODE_MASK);
		mode_reg |= (led->mode << TOP_REG_PWM_CTRL_MODE_REG_SW_RGB2_MODE_SHIFT);
		break;
	default:
		en_reg |= TOP_REG_PWM_CTRL_EN_REG_SW_RGB3_EN_MASK;
		mode_reg &= (~TOP_REG_PWM_CTRL_MODE_REG_SW_RGB3_MODE_MASK);
		mode_reg |= (led->mode << TOP_REG_PWM_CTRL_MODE_REG_SW_RGB3_MODE_SHIFT);
	}
	ret = xr_pmic_rgb_write(led, TOP_REG_PWM_CTRL_MODE_REG, &mode_reg, sizeof(mode_reg));
	if (ret)
		return ret;
	ret = xr_pmic_rgb_write(led, TOP_REG_PWM_CTRL_EN_REG, &en_reg, sizeof(en_reg));
	return ret;
}

/* flash */
static int xr_pmic_led_set_blink(struct led_classdev *cdev,
				unsigned long *delay_on,
				unsigned long *delay_off)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	struct xr_pmic_leds *leds = led->parent;
	int ret, i, current_brightness = LED_ON;
	unsigned int pwm_freq;
	unsigned long period;
	unsigned long freq;

	/*
	 * LED subsystem requires a default user
	 * friendly blink pattern for the LED so using
	 * 1Hz duty cycle 50% here if without specific
	 * value delay_on and delay off being assigned.
	 */
	if (!*delay_on && !*delay_off) {
		pr_debug("use default delay num\n");
		*delay_on = XR_DEFAULT_PERIOD;
		*delay_off = XR_DEFAULT_PERIOD;
	}
	if (!*delay_off)
		led->mode = RGB_REGISTER_MODE;
	else
		led->mode = RGB_FLASH_MODE;
	/*
	 * Units are in ms, if over the hardware able
	 * to support, fallback into software blink
	 */
	pr_debug("delay_on:%lu, off:%lu\n", *delay_on, *delay_off);
	period = (*delay_on) + (*delay_off);
	if (period > XR_MAX_PERIOD || period == 0) {
		pr_err("period:%lu is invalid! check it!\n", period);
		return -EINVAL;
	}

	freq = MS_PER_S * FREQ_FACTOR / period;

	/* Calculate the corresponding PWM frequency according to the frequency */
	for (i = 1, pwm_freq = XR_BASE_PERIOD; i <= FREQ_NUM; i++) {
		if (freq <= pwm_freq) {
			led->pwm_freq = i - 1;
			break;
		}
		if (i >= FREQ_MUTATION)
			pwm_freq = XR_BASE2_PERIOD;
		pwm_freq *= FREQ_STEP_SIZE;
	}

	pr_debug("period:%lu, freq:%lu,pwm_freq:%x, led->pwm_freq:%x\n",
			period, freq, pwm_freq, led->pwm_freq);
	mutex_lock(&leds->lock);
	if (led->current_brightness) {
		ret = xr_pmic_led_hw_off(cdev);
		if (ret < 0) {
			pr_err("xr_pmic_led_hw_off fail\n");
			goto out;
		}
		current_brightness = led->current_brightness;
		led->current_brightness = LED_OFF;
	}
	ret = pmic_rgb_set_pwm(led);
	if (ret < 0)
		goto out;

	/* Calculate the dark duty cycle */
	led->dimming = (*delay_off * DUTY_SIZE) / period;
	ret = pmic_rgb_set_dimming(led);
	if (ret < 0)
		goto out;
	/*
	 * Set max_brightness as the software blink behavior
	 * when no blink brightness.
	 */
	if (!led->current_brightness) {
		ret = xr_pmic_led_hw_on(cdev, current_brightness);
		if (ret < 0)
			goto out;
		led->current_brightness = current_brightness;
	}
out:
	mutex_unlock(&leds->lock);
	return ret;
}

static int xr_pmic_led_set_brightness(struct led_classdev *cdev,
				     enum led_brightness brightness)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	struct xr_pmic_leds *leds = led->parent;
	int ret = 0;

	mutex_lock(&leds->lock);
	if (!led->current_brightness && brightness) {
		ret = xr_pmic_led_hw_on(cdev, brightness);
		if (ret < 0)
			goto out;
	} else if (brightness) {
		ret = xr_pmic_led_hw_brightness(cdev, brightness);
		if (ret < 0)
			goto out;
	} else {
		ret = xr_pmic_led_hw_off(cdev);
		if (ret < 0)
			goto out;
	}

	led->current_brightness = brightness;
out:
	mutex_unlock(&leds->lock);

	return ret;
}

static int xr_pmic_led_pattern_clear(struct led_classdev *cdev)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	struct xr_pmic_leds *leds = led->parent;
	int ret = 0;

	mutex_lock(&leds->lock);
	if (led->current_brightness) {
		ret = xr_pmic_led_hw_off(cdev);
		if (ret < 0) {
			pr_err("xr_pmic_led_hw_off fail\n");
			mutex_unlock(&leds->lock);
			return ret;
		}
	}
	led->current_brightness = LED_OFF;
	mutex_unlock(&leds->lock);
	return ret;
}

static void xr_pmic_led_breath_init(struct xr_pmic_led *led)
{
	led->toff = RGB_TOFF_1250_ms;
	led->tr1 = RGB_TON_TR_TF_1625_ms;
	led->tr2 = RGB_TON_TR_TF_1875_ms;
	led->ton = RGB_TON_TR_TF_2375_ms;
	led->tf1 = RGB_TON_TR_TF_1625_ms;
	led->tf2 = RGB_TON_TR_TF_2375_ms;
}

static int xr_pmic_led_pattern_set(struct led_classdev *cdev,
				  struct led_pattern *pattern,
				  uint32_t len, int repeat)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	struct xr_pmic_leds *leds = led->parent;
	int ret, i;
	int current_brightness = LED_ON;

	/*
	 * Must contain 4 tuples to configure the rise time, high time, fall
	 * time and low time to enable the breathing mode.
	 */
	if (len > BREATH_PARA_NUM) {
		pr_err("the len(%d) inputed shouldn't larger than %d\n",
						len, BREATH_PARA_NUM);
		return -EINVAL;
	}
	for (i = 0; i < len; i++) {
		if (pattern[i].delta_t > RGB_TOFF_CNT) {
			pr_err("The time_val(%d: %d) configured is incorrect\n",
							i, pattern[i].delta_t);
			return -EINVAL;
		}
	}
	xr_pmic_led_breath_init(led);
	switch (len) {
	case 6:
		led->tf2 = pattern[5].delta_t;
		fallthrough;
	case 5:
		led->tf1 = pattern[4].delta_t;
		fallthrough;
	case 4:
		led->ton = pattern[3].delta_t;
		fallthrough;
	case 3:
		led->tr2 = pattern[2].delta_t;
		fallthrough;
	case 2:
		led->tr1 = pattern[1].delta_t;
		fallthrough;
	case 1:
		led->toff = pattern[0].delta_t;
		break;
	default:
		pr_err("the len(%d) inputed is invalid!\n", len);
	}
	led->mode = RGB_BREATH_MODE;

	mutex_lock(&leds->lock);
	if (led->current_brightness) {
		ret = xr_pmic_led_hw_off(cdev);
		if (ret < 0) {
			pr_err("xr_pmic_led_hw_off fail\n");
			goto out;
		}
		current_brightness = led->current_brightness;
		led->current_brightness = LED_OFF;
	}
	ret = pmic_rgb_set_ton_tff(led);
	if (ret < 0)
		goto out;
	ret = pmic_rgb_set_tr1_tr2(led);
	if (ret < 0)
		goto out;
	ret = pmic_rgb_set_tf1_tf2(led);
	if (ret < 0)
		goto out;
	if (!led->current_brightness) {
		ret = xr_pmic_led_hw_on(cdev, current_brightness);
		if (ret < 0)
			goto out;
		led->current_brightness = current_brightness;
	}
out:
	mutex_unlock(&leds->lock);
	return ret;
}

static int xr_pmic_led_set_dt_default(struct led_classdev *cdev,
				     struct device_node *np)
{
	struct xr_pmic_led *led = container_of(cdev, struct xr_pmic_led, cdev);
	const char *state;
	int ret = 0;

	state = of_get_property(np, "default-state", NULL);
	if (state) {
		if (!strcmp(state, "keep")) {
			ret = xr_pmic_get_led_hw_brightness(cdev);
			if (ret < 0)
				return ret;
			led->current_brightness = ret;
			ret = 0;
		} else if (!strcmp(state, "on")) {
			ret =
			xr_pmic_led_set_brightness(cdev, cdev->max_brightness);
		} else  {
			ret = xr_pmic_led_set_brightness(cdev, LED_OFF);
		}
	}

	return ret;
}

static int xr_pmic_led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev_of_node(dev);
	struct device_node *child = NULL;
	struct xr_pmic_leds *leds = NULL;
	struct xr_pmic_led *led = NULL;
	int ret = 0;
	uint32_t id = 0;
	uint32_t base_addr = 0;

	leds = devm_kzalloc(dev, sizeof(*leds), GFP_KERNEL);
	if (!leds)
		return -ENOMEM;

	platform_set_drvdata(pdev, leds);
	leds->dev = dev;

	mutex_init(&leds->lock);

	for_each_available_child_of_node(np, child) {
		struct led_init_data init_data = {};

		ret = of_property_read_u32(child, "base_addr", &base_addr);
		if (ret) {
			dev_err(dev, "Failed to read led 'base_addr' property\n");
			goto put_child_node;
		}
		ret = of_property_read_u32(child, "id", &id);
		if (ret) {
			dev_err(dev, "Failed to read led 'id' property\n");
			goto put_child_node;
		}

		if (id >= XR_PMIC_MAX_LEDS || leds->led[id]) {
			dev_err(dev, "Invalid led id %u\n", id);
			ret = -EINVAL;
			goto put_child_node;
		}

		led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
		if (!led) {
			ret = -ENOMEM;
			goto put_child_node;
		}

		leds->led[id] = led;
		leds->led[id]->id = id;
		leds->led[id]->cdev.max_brightness = LED_FULL;
		leds->led[id]->cdev.brightness = LED_HALF;
		leds->led[id]->cdev.name = rgb_name[id];
		leds->led[id]->cdev.brightness_set_blocking =
					xr_pmic_led_set_brightness;
		leds->led[id]->cdev.blink_set = xr_pmic_led_set_blink;
		leds->led[id]->cdev.pattern_set = xr_pmic_led_pattern_set;
		leds->led[id]->cdev.pattern_clear = xr_pmic_led_pattern_clear;
		leds->led[id]->cdev.brightness_get =
					xr_pmic_get_led_hw_brightness;
		leds->led[id]->parent = leds;
		leds->led[id]->base = base_addr;
		leds->led[id]->regmap = dev_get_regmap(dev->parent, NULL);
		if (!leds->led[id]->regmap) {
			dev_err(dev, "Parent regmap unavailable.\n");
			return -ENXIO;
		}
		ret = xr_pmic_led_set_dt_default(&leds->led[id]->cdev, child);
		if (ret < 0) {
			dev_err(dev, "Failed to LED set default from devicetree\n");
			goto put_child_node;
		}
		init_data.fwnode = of_fwnode_handle(child);
		ret = devm_led_classdev_register_ext(dev, &leds->led[id]->cdev,
							 &init_data);
		if (ret) {
			dev_err(dev, "Failed to register LED: %d\n", ret);
			goto put_child_node;
		}
	}
	dev_info(dev, "%s succ!\n", __func__);
	return 0;
put_child_node:
	of_node_put(child);
	return ret;
}

static int xr_pmic_led_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s succ!\n", __func__);
	return 0;
}

static const struct of_device_id xr_pmic_leds_id_table[] = {
	{ .compatible = "xring,pmic_led"},
	{}
};
MODULE_DEVICE_TABLE(of, xr_pmic_leds_id_table);

static struct platform_driver xr_pmic_led_driver = {
	.probe		= xr_pmic_led_probe,
	.remove = xr_pmic_led_remove,
	.driver		= {
		.name	= "xr_pmic_leds",
		.of_match_table = xr_pmic_leds_id_table,
	},
};

int xr_pmic_led_init(void)
{
	return platform_driver_register(&xr_pmic_led_driver);
}

void xr_pmic_led_exit(void)
{
	platform_driver_unregister(&xr_pmic_led_driver);
}
