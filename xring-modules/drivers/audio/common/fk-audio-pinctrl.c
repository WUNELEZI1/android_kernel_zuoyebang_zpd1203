// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
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

#include <linux/of.h>
#include <linux/device.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>

#include "fk-audio-pinctrl.h"
#include "fk-audio-log.h"
#include "../asoc/fk-dai-be.h"

struct audio_pinctrl_pdata {
	struct device *dev;
	struct pinctrl *pinctrl_t;
	struct mutex pinctrl_lock;

	int pinctrl_active_flag[PINCTRL_FUNC_MAX];
	struct mutex active_flag_lock;
	const char *bind_product;
};

struct audio_pinctrl_desc_s {
	char *pinctrl_state;
	u8 func_type;
	u8 action;
};

struct audio_pinctrl_pdata *audio_pinctrl_p;

struct audio_pinctrl_desc_s audio_pinctrl_desc[PINCTRL_FUNC_MAX][PIN_STATE_MAX] = {
	/* pdm pinctrl function */
	{{ "pdm0_init",     PDM0_FUNC, PIN_INIT },
	 { "pdm0_default",  PDM0_FUNC, PIN_DEFAULT },
	 { "pdm0_sleep",    PDM0_FUNC, PIN_SLEEP },},

	{{ "pdm1_init",     PDM1_FUNC, PIN_INIT },
	 { "pdm1_default",  PDM1_FUNC, PIN_DEFAULT },
	 { "pdm1_sleep",    PDM1_FUNC, PIN_SLEEP },},

	{{ "codec_mclk_init",    CDC_CLK_FUNC, PIN_INIT },
	 { "codec_mclk_default", CDC_CLK_FUNC, PIN_DEFAULT },
	 { "codec_mclk_sleep",   CDC_CLK_FUNC, PIN_SLEEP },},

	{{ "i2s0_init",    I2S0_FUNC, PIN_INIT },
	 { "i2s0_default", I2S0_FUNC, PIN_DEFAULT },
	 { "i2s0_sleep",   I2S0_FUNC, PIN_SLEEP },},

	{{ "i2s1_init",    I2S1_FUNC, PIN_INIT },
	 { "i2s1_default", I2S1_FUNC, PIN_DEFAULT },
	 { "i2s1_sleep",   I2S1_FUNC, PIN_SLEEP },},

	{{ "i2s2_init",    I2S2_FUNC, PIN_INIT },
	 { "i2s2_default", I2S2_FUNC, PIN_DEFAULT },
	 { "i2s2_sleep",   I2S2_FUNC, PIN_SLEEP },},

	{{ "i2s3_init",    I2S3_FUNC, PIN_INIT },
	 { "i2s3_default", I2S3_FUNC, PIN_DEFAULT },
	 { "i2s3_sleep",   I2S3_FUNC, PIN_SLEEP },},

	{{ "i2s4_init",     I2S4_FUNC, PIN_INIT },
	 { "i2s4_default",  I2S4_FUNC, PIN_DEFAULT },
	 { "i2s4_sleep",    I2S4_FUNC, PIN_SLEEP },},

	{{ "i2s5_init",    I2S5_FUNC, PIN_INIT },
	 { "i2s5_default", I2S5_FUNC, PIN_DEFAULT },
	 { "i2s5_sleep",   I2S5_FUNC, PIN_SLEEP },},

	{{ "i2s6_init",    I2S6_FUNC, PIN_INIT },
	 { "i2s6_default", I2S6_FUNC, PIN_DEFAULT },
	 { "i2s6_sleep",   I2S6_FUNC, PIN_SLEEP },},

	{{ "i2s7_init",    I2S7_FUNC, PIN_INIT },
	 { "i2s7_default", I2S7_FUNC, PIN_DEFAULT },
	 { "i2s7_sleep",   I2S7_FUNC, PIN_SLEEP },},

	{{ "i2s8_init",    I2S8_FUNC, PIN_INIT },
	 { "i2s8_default", I2S8_FUNC, PIN_DEFAULT },
	 { "i2s8_sleep",   I2S8_FUNC, PIN_SLEEP },},

	{{ "i2s9_init",     I2S9_FUNC, PIN_INIT },
	 { "i2s9_default",  I2S9_FUNC, PIN_DEFAULT },
	 { "i2s9_sleep",    I2S9_FUNC, PIN_SLEEP },},

	{{ "i2s10_init",    I2S10_FUNC, PIN_INIT },
	 { "i2s10_default", I2S10_FUNC, PIN_DEFAULT },
	 { "i2s10_sleep",   I2S10_FUNC, PIN_SLEEP },},

	{{ "device_int_init",    DEVICE_INT_FUNC, PIN_INIT },
	 { "device_int_default", DEVICE_INT_FUNC, PIN_DEFAULT },
	 { "device_int_sleep",   DEVICE_INT_FUNC, PIN_SLEEP },}
};

struct audio_pinctrl_pdata *fk_audio_pinctrl_pdata_get(void)
{
	return audio_pinctrl_p;
}

static int fk_audio_pinctrl_func_set(struct audio_pinctrl_pdata *pinctrl_p,
	char *pinctrl_state)
{
	struct pinctrl_state *pinctrl_st = NULL;
	int ret = 0;

	pinctrl_st =
		pinctrl_lookup_state(pinctrl_p->pinctrl_t, pinctrl_state);
	if (IS_ERR_OR_NULL(pinctrl_st)) {
		AUD_LOG_ERR(AUD_COMM, "find state: %s failed", pinctrl_state);
		return PTR_ERR(pinctrl_st);
	}
	mutex_lock(&pinctrl_p->pinctrl_lock);

	ret = pinctrl_select_state(pinctrl_p->pinctrl_t, pinctrl_st);
	if (ret) {
		AUD_LOG_ERR(AUD_COMM, "select state: %s switch failed",
			pinctrl_state);
		mutex_unlock(&pinctrl_p->pinctrl_lock);
		return ret;
	}
	mutex_unlock(&pinctrl_p->pinctrl_lock);
	AUD_LOG_INFO(AUD_COMM, "pin state %s audio pin set succes",
		pinctrl_state);
	return ret;
}

static int fk_audio_pinctrl_flag_set(u8 func_type, u8 action,
	char *pinctrl_state)
{
	struct audio_pinctrl_pdata *pinctrl_p = fk_audio_pinctrl_pdata_get();
	int ret = 0;

	AUD_LOG_DBG(AUD_COMM, "func_type: %d,action:%d,pinctrl_state: %s.",
		func_type, action, pinctrl_state);

	mutex_lock(&pinctrl_p->active_flag_lock);

	if (action == PIN_DEFAULT) {
		if (func_type == I2S9_FUNC || func_type == I2S10_FUNC) {
			ret = fk_audio_pinctrl_func_set(pinctrl_p, pinctrl_state);
			if (ret)
				goto set_fail;
		} else {
			if (!pinctrl_p->pinctrl_active_flag[func_type]) {
				ret = fk_audio_pinctrl_func_set(pinctrl_p, pinctrl_state);
				if (ret)
					goto set_fail;
			}
		}

		pinctrl_p->pinctrl_active_flag[func_type]++;
		AUD_LOG_DBG(AUD_COMM, "pinctrl_active_flag:%d.",
			pinctrl_p->pinctrl_active_flag[func_type]);
	}

	if (action == PIN_SLEEP) {
		if (!pinctrl_p->pinctrl_active_flag[func_type])
			goto set_fail;

		pinctrl_p->pinctrl_active_flag[func_type]--;
		AUD_LOG_DBG(AUD_COMM, "pinctrl_active_flag:%d.",
			pinctrl_p->pinctrl_active_flag[func_type]);
		if (func_type == I2S9_FUNC || func_type == I2S10_FUNC) {
			if (!pinctrl_p->pinctrl_active_flag[I2S9_FUNC] &&
				!pinctrl_p->pinctrl_active_flag[I2S10_FUNC]) {
				ret = fk_audio_pinctrl_func_set(pinctrl_p, pinctrl_state);
				if (ret) {
					pinctrl_p->pinctrl_active_flag[func_type]++;
					goto set_fail;
				}
			}
		} else {
			if (!pinctrl_p->pinctrl_active_flag[func_type]) {
				ret = fk_audio_pinctrl_func_set(pinctrl_p, pinctrl_state);
				if (ret) {
					pinctrl_p->pinctrl_active_flag[func_type]++;
					goto set_fail;
				}
			}
		}
	}

set_fail:
	mutex_unlock(&pinctrl_p->active_flag_lock);
	return ret;
}

int fk_audio_pinctrl_switch_state(u8 func_type, u8 action)
{
	struct audio_pinctrl_desc_s *p_desc = NULL;
	int ret = 0;
	int i, j;

	for (i = 0; i < PINCTRL_FUNC_MAX; i++) {
		for (j = 0; j < PIN_STATE_MAX; j++) {
			p_desc = &audio_pinctrl_desc[i][j];
			if ((func_type == p_desc->func_type) && (action == p_desc->action)) {
				ret = fk_audio_pinctrl_flag_set(func_type, action,
					p_desc->pinctrl_state);
				if (ret)
					return ret;
			}
		}
	}
	return ret;
}
EXPORT_SYMBOL(fk_audio_pinctrl_switch_state);

int fk_audio_port_pinctrl_func_set(int port_id, bool en)
{
	u8 action;
	int ret = 0;

	if (en == true)
		action = PIN_DEFAULT;
	else
		action = PIN_SLEEP;

	AUD_LOG_DBG(AUD_COMM, "port_id=%d, action=%d", port_id, action);
	switch (port_id) {
	case FK_XR_I2S0_RX:
	case FK_XR_I2S0_TX:
	case FK_XR_TDM0_RX:
	case FK_XR_TDM0_TX:
		break;
	case FK_XR_I2S1_RX:
	case FK_XR_I2S1_TX:
	case FK_XR_TDM1_RX:
	case FK_XR_TDM1_TX:
		ret = fk_audio_pinctrl_switch_state(I2S1_FUNC, action);
		break;
	case FK_XR_I2S2_RX:
	case FK_XR_I2S2_TX:
	case FK_XR_TDM2_RX:
	case FK_XR_TDM2_TX:
		ret = fk_audio_pinctrl_switch_state(I2S2_FUNC, action);
		break;
	case FK_XR_I2S3_RX:
	case FK_XR_I2S3_TX:
	case FK_XR_TDM3_RX:
	case FK_XR_TDM3_TX:
		if (audio_pinctrl_p->bind_product && !strcmp(audio_pinctrl_p->bind_product, "dijun")) {
			ret = fk_audio_pinctrl_switch_state(PDM1_FUNC, action);
		} else {
			ret = fk_audio_pinctrl_switch_state(PDM0_FUNC, action);
			ret = fk_audio_pinctrl_switch_state(PDM1_FUNC, action);
		}
		break;
	case FK_XR_I2S4_RX:
	case FK_XR_I2S4_TX:
	case FK_XR_TDM4_RX:
	case FK_XR_TDM4_TX:
		ret = fk_audio_pinctrl_switch_state(I2S4_FUNC, action);
		break;
	case FK_XR_I2S5_RX:
	case FK_XR_I2S5_TX:
	case FK_XR_TDM5_RX:
	case FK_XR_TDM5_TX:
		ret = fk_audio_pinctrl_switch_state(I2S5_FUNC, action);
		break;
	case FK_XR_I2S6_RX:
	case FK_XR_I2S6_TX:
	case FK_XR_TDM6_RX:
	case FK_XR_TDM6_TX:
		ret = fk_audio_pinctrl_switch_state(I2S6_FUNC, action);
		break;
	case FK_XR_I2S7_RX:
	case FK_XR_I2S7_TX:
	case FK_XR_TDM7_RX:
	case FK_XR_TDM7_TX:
		ret = fk_audio_pinctrl_switch_state(I2S7_FUNC, action);
		break;
	case FK_XR_I2S8_RX:
	case FK_XR_I2S8_TX:
	case FK_XR_TDM8_RX:
	case FK_XR_TDM8_TX:
		ret = fk_audio_pinctrl_switch_state(I2S8_FUNC, action);
		break;
	case FK_XR_I2S9_RX:
	case FK_XR_I2S9_TX:
	case FK_XR_TDM9_RX:
	case FK_XR_TDM9_TX:
		ret = fk_audio_pinctrl_switch_state(I2S9_FUNC, action);
		break;
	case FK_XR_TDM7_8_RX:
	case FK_XR_TDM7_8_TX:
		ret = fk_audio_pinctrl_switch_state(I2S7_FUNC, action);
		ret = fk_audio_pinctrl_switch_state(I2S8_FUNC, action);
		break;
	/*enable vad I2S func*/
	case FK_XR_VAD_I2S_TX:
		ret = fk_audio_pinctrl_switch_state(I2S10_FUNC, action);
		break;
	case FK_XR_AUDIO_USB_RX:
	case FK_XR_AUDIO_USB_TX:
		break;
	case FK_XR_MI2S0_RX:
		break;
	default:
		AUD_LOG_ERR(AUD_COMM, "unused port_id=%d", port_id);
		break;
	};

	if (ret)
		AUD_LOG_ERR(AUD_SOC, "i2s pinctrl set fail");

	return ret;
}
EXPORT_SYMBOL(fk_audio_port_pinctrl_func_set);

static int fk_audio_pinctrl_probe(struct platform_device *pdev)
{
	int rc = 0;

	audio_pinctrl_p =
		devm_kmalloc(&pdev->dev, sizeof(struct audio_pinctrl_pdata), GFP_KERNEL);
	if (audio_pinctrl_p == NULL) {
		AUD_LOG_ERR(AUD_COMM, "alloc audio_pinctrl_p failed, out of memory");
		return -ENOMEM;
	}
	memset(audio_pinctrl_p, 0, sizeof(struct audio_pinctrl_pdata));
	mutex_init(&audio_pinctrl_p->pinctrl_lock);
	mutex_init(&audio_pinctrl_p->active_flag_lock);
	audio_pinctrl_p->dev = &pdev->dev;

	/* get pinctrl handle */
	audio_pinctrl_p->pinctrl_t = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR_OR_NULL(audio_pinctrl_p->pinctrl_t)) {
		rc = PTR_ERR(audio_pinctrl_p->pinctrl_t);
		AUD_LOG_ERR(AUD_COMM, "get audio pinctrl fail, ret = %d.", rc);
	}

	if (audio_pinctrl_p->dev->of_node) {
		if (!of_property_read_string(audio_pinctrl_p->dev->of_node,
				"bind-product", &audio_pinctrl_p->bind_product)) {
			AUD_LOG_INFO(AUD_COMM, "it is %s product.",
				audio_pinctrl_p->bind_product);
		}
	}

	return rc;
}

static int fk_audio_pinctrl_remove(struct platform_device *pdev)
{
	AUD_LOG_INFO(AUD_COMM, "audio_pinctrl_p remove finish.");

	return 0;
}


static const struct of_device_id fk_audio_pinctrl_dt_match[] = {
	{.compatible = "xring,fk-audio-pinctrl"},
	{}
};

static struct platform_driver fk_audio_pinctrl_driver = {
	.probe  = fk_audio_pinctrl_probe,
	.remove = fk_audio_pinctrl_remove,
	.driver = {
		.name = "fk-audio-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = fk_audio_pinctrl_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init fk_audio_pinctrl_init(void)
{
	return platform_driver_register(&fk_audio_pinctrl_driver);
}

void fk_audio_pinctrl_exit(void)
{
	platform_driver_unregister(&fk_audio_pinctrl_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO COMMON PINCTRL");
MODULE_LICENSE("Dual BSD/GPL");
