// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018, 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/log2.h>
#include <linux/version.h>

#include <dt-bindings/xring/platform-specific/pmic/top_reg.h>
#include "pmic_adc_pri_def.h"

#define ADC_LDO_TO_EN_TIME_US 320
#define ADC_EN_TO_START_TIME_US 1000
#define ADC_TRANS_DONE_STAT			BIT(0)
#define ADC_CONV_TIMEOUT_CNT		26
#define ADC_CONV_CLK_CYCLE_US(freq)	(10000000 / freq)
#define ADC_CONV_TIMEOUT			msecs_to_jiffies(100)
#define ADC_ENABLE_SIG	1
#define ADC_DISABLE_SIG	0
#define OFF_SET_VAL 0x200
#define ADC_REG_FREQ_L 1
#define ADC_FREQ_L_VAL 1200000
#define ADC_FREQ_H_VAL 2400000
#define ADC_MAX_CODE 4096

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)  "XR_PMICADC:%s:%d " fmt, __func__, __LINE__

struct xr_adc_data {
	const struct xr_adc_channels *adc_chans;
	const struct iio_info *info;
};

/**
 * struct xr_pmic_adc_chip - ADC private structure.
 * @regmap: SPMI XR_ADC peripheral register map field.
 * @dev: SPMI XR_ADC device.
 * @base: base address for the ADC peripheral.
 * @nchannels: number of ADC channels.
 * @chan_props: array of ADC channel properties.
 * @iio_chans: array of IIO channels specification.
 * @poll_eoc: use polling instead of interrupt.
 * @complete: ADC result notification after interrupt is received.
 * @lock: ADC lock for access to the peripheral.
 * @data: software configuration data.
 */
struct xr_pmic_adc_chip {
	struct regmap		*regmap;
	struct device		*dev;
	uint16_t			base;
	uint32_t		nchannels;
	struct xr_pmic_adc_channel_prop	*chan_props;
	struct iio_chan_spec	*iio_chans;
	bool			poll_eoc;
	struct completion	complete;
	struct mutex		lock;
	const struct xr_adc_data	*data;
};

/**
 * struct xr_pmic_adc_channel_prop - ADC channel property.
 * @channel: channel number, refer to the channel list.
 * @ctrl_mode: adc method mode.
 * @datasheet_name: Channel name used in device tree.
 */
struct xr_pmic_adc_channel_prop {
	uint32_t channel;
	uint16_t offset_code;
	uint32_t ctrl_mode;
	const char *datasheet_name;
};

static int xr_pmic_adc_read(struct xr_pmic_adc_chip *adc, uint16_t offset, uint8_t *data, int len)
{
	int ret;

	ret = regmap_bulk_read(adc->regmap, adc->base + offset, data, len);
	pr_debug("offset:0x%x, val:0x%x\n", offset, *data);
	return ret;
}

static int xr_pmic_adc_readl(struct xr_pmic_adc_chip *adc, uint16_t offset, uint16_t *data, int len)
{
	int ret;

	ret = regmap_bulk_read(adc->regmap, adc->base + offset, data, len);
	pr_debug("offset:0x%x, val:0x%x\n", offset, *data);
	return ret;
}

static int pmic_adc_write(struct xr_pmic_adc_chip *adc, uint16_t offset, uint8_t *data, int len)
{
	int ret;

	pr_debug(" base:0x%x,offset:0x%x,data:0x%x,len:0x%x\n",
			adc->base, offset, *data, len);
	ret = regmap_bulk_write(adc->regmap, adc->base + offset, data, len);
	return ret;
}

static int xr_pmic_adc_read_voltage_and_thermal_data(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop, uint16_t *data_code)
{
	int ret;
	uint16_t adc_code;

	ret = xr_pmic_adc_readl(adc, TOP_REG_AUXADC_TRANS_DATAL_REG,
				&adc_code, sizeof(adc_code));
	if (ret)
		return ret;

	if (prop->ctrl_mode == ADC_MODE_NORMAL)
		adc_code -= prop->offset_code;
	if (adc_code > ADC_MAX_CODE) {
		dev_err(adc->dev, "adc raw code:0x%x is error!\n", adc_code);
		return -EFAULT;
	}
	dev_dbg(adc->dev, "adc raw code:0x%x\n", adc_code);
	*data_code = adc_code;
	return 0;
}

static int xr_pmic_adc_poll_wait_eoc(struct xr_pmic_adc_chip *adc)
{
	uint32_t count, freq;
	uint8_t status, reg_freq;
	int ret;

	ret = xr_pmic_adc_read(adc, TOP_REG_CRG_CONFIG1, &reg_freq,
						sizeof(reg_freq));
	if (ret)
		return ret;
	freq = ((reg_freq == ADC_REG_FREQ_L) ? ADC_FREQ_L_VAL : ADC_FREQ_H_VAL);
	for (count = 0; count < ADC_CONV_TIMEOUT_CNT; count++) {
		ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_TRANS_DONE_REG, &status,
							sizeof(status));
		if (ret)
			return ret;

		status &= TOP_REG_AUXADC_TRANS_DONE_REG_AUXADC_TRANS_DONE_MASK;
		if (status == ADC_TRANS_DONE_STAT)
			return 0;
		usleep_range(ADC_CONV_CLK_CYCLE_US(freq), ADC_CONV_CLK_CYCLE_US(freq) + 1);
	}
	dev_err(adc->dev, "adc_poll_wait_eoc timeout\n");
	return -ETIMEDOUT;
}

static int xr_pmic_adc_enable(struct xr_pmic_adc_chip *adc, bool enable)
{
	int ret = 0;
	uint8_t reg_buf;

	ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_CTRL_SETUP_REG, &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	if (enable) {
		usleep_range(ADC_LDO_TO_EN_TIME_US, ADC_LDO_TO_EN_TIME_US + 1);
		/* The auxadc function is enabled to 1 */
		reg_buf |= BIT(TOP_REG_AUXADC_CTRL_SETUP_REG_SW_AUXADC_EN_SHIFT);
	} else {
		/* The auxadc function is enabled to 0 */
		reg_buf &= (~BIT(TOP_REG_AUXADC_CTRL_SETUP_REG_SW_AUXADC_EN_SHIFT));
	}
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_CTRL_SETUP_REG, &reg_buf, sizeof(reg_buf));
	return ret;
}

static int xr_pmic_adc_check_err_event(struct xr_pmic_adc_chip *adc)
{
	int ret = 0;
	uint8_t reg_buf;

	ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_TRANS_DONE_REG, &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	if (reg_buf & TOP_REG_AUXADC_TRANS_DONE_REG_AUXADC_CTRL_TIMEOUT_MASK) {
		/* W1C */
		reg_buf = BIT(TOP_REG_AUXADC_TRANS_DONE_REG_AUXADC_CTRL_TIMEOUT_SHIFT);
		ret = pmic_adc_write(adc, TOP_REG_AUXADC_TRANS_DONE_REG, &reg_buf, sizeof(reg_buf));
	}
	return ret;
}

static int xr_pmic_adc_wait_done(struct xr_pmic_adc_chip *adc)
{
	int ret = 0;

	if (adc->poll_eoc) {
		ret = xr_pmic_adc_poll_wait_eoc(adc);
		if (ret) {
			dev_err(adc->dev, "EOC bit not set\n");
			return ret;
		}
	} else {
		ret = wait_for_completion_timeout(&adc->complete,
							ADC_CONV_TIMEOUT);
		if (ret <= 0) {
			dev_err(adc->dev, "get completion timeout\n");
			ret = xr_pmic_adc_poll_wait_eoc(adc);
			if (ret) {
				dev_err(adc->dev, "EOC bit not set\n");
				return ret;
			}
		} else {
			return 0;
		}
	}
	return ret;
}

static int adc_normal_config(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop)
{
	int ret = 0;
	uint8_t trans_data_l, trans_data_h;
	uint16_t offset_code;
	uint8_t reg_buf;

	/* Get offset_code in calibration mode */
	reg_buf = ADC_EN_CAL;
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_CTRL_CAL_SETUP,
				  &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	if (!adc->poll_eoc)
		reinit_completion(&adc->complete);
	ret = xr_pmic_adc_enable(adc, ADC_ENABLE_SIG);
	if (ret)
		return ret;
	/* Configure auxadc，start at least 1ms after the previous step */
	usleep_range(ADC_EN_TO_START_TIME_US, ADC_EN_TO_START_TIME_US + 1);
	reg_buf = ADC_ENABLE_SIG;
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_START_REG,
				 &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	ret = xr_pmic_adc_wait_done(adc);
	if (ret) {
		dev_err(adc->dev, "ADC sampling timeout with %d\n", ret);
		return ret;
	}
	ret = xr_pmic_adc_check_err_event(adc);
	if (ret) {
		dev_err(adc->dev, "xr_pmic_adc_check_err_event failed with %d\n", ret);
		return ret;
	}
	if (!adc->poll_eoc)
		reinit_completion(&adc->complete);

	/* read offset_code from reg */
	ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_CAL_DATAL_REG,
				&trans_data_l, sizeof(trans_data_l));
	if (ret)
		return ret;

	ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_CAL_DATAH_REG,
				&trans_data_h, sizeof(trans_data_l));
	if (ret)
		return ret;
	trans_data_h &= TOP_REG_AUXADC_CAL_DATAH_REG_AUXADC_CAL_DATA_11_8_MASK;
	offset_code = (trans_data_h << 8) | trans_data_l;
	prop->offset_code = offset_code - OFF_SET_VAL;

	reg_buf = prop->channel | BIT(TOP_REG_AUXADC_CTRL_SETUP_REG_SW_AUXADC_EN_SHIFT) |
		(prop->ctrl_mode << TOP_REG_AUXADC_CTRL_SETUP_REG_SW_AUXADC_CTRL_MODE_SHIFT);
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_CTRL_SETUP_REG, &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	reg_buf = ADC_ENABLE_SIG;
	/* Configure auxadc，start at least 1ms after the previous step */
	usleep_range(ADC_EN_TO_START_TIME_US, ADC_EN_TO_START_TIME_US + 1);
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_START_REG,
				  &reg_buf, sizeof(reg_buf));
	return ret;
}

static int adc_chopper_config(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop)
{
	int ret = 0;
	uint8_t reg_buf;

	/* Configure the channel, mode, delay and calibration mode */
	reg_buf = prop->channel | (prop->ctrl_mode << TOP_REG_AUXADC_CTRL_SETUP_REG_SW_AUXADC_CTRL_MODE_SHIFT);
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_CTRL_SETUP_REG, &reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	if (!adc->poll_eoc)
		reinit_completion(&adc->complete);
	ret = xr_pmic_adc_enable(adc, ADC_ENABLE_SIG);
	if (ret)
		return ret;

	/* Configure auxadc，start at least 1ms after the previous step */
	usleep_range(ADC_EN_TO_START_TIME_US, ADC_EN_TO_START_TIME_US + 1);
	reg_buf = ADC_ENABLE_SIG;
	ret = pmic_adc_write(adc, TOP_REG_AUXADC_START_REG, &reg_buf, sizeof(reg_buf));
	return ret;
}

static int xr_pmic_adc_configure(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop)
{
	int ret;
	uint8_t reg_buf;
	int i;

	/* Ensure that auxadc is not enabled for querying coulombmeter */
	for (i = 0; i < COUL_BUSY_DELAY_NUM; i++) {
		ret = xr_pmic_adc_read(adc, TOP_REG_AUXADC_TRANS_DONE_REG, &reg_buf, sizeof(reg_buf));
		if (ret)
			return ret;
		if (reg_buf & BIT(TOP_REG_AUXADC_TRANS_DONE_REG_AUXADC_COUL_SAMPLE_FLAG_SHIFT))
			mdelay(ADC_DELAY_MS);
		else
			break;
	}
	if (i == COUL_BUSY_DELAY_NUM) {
		dev_err(adc->dev, "Current Coulombs actually use auxadc, wait timeout!\n");
		return -EBUSY;
	}
	reg_buf = BIT(TOP_REG_LDO_ADC_PWR_REG_SW_LDO_ADC_PWR_EN_SHIFT);
	/* Open the ldo_adc configuration */
	ret = pmic_adc_write(adc, TOP_REG_LDO_ADC_PWR_REG,
				&reg_buf, sizeof(reg_buf));
	if (ret)
		return ret;
	if (prop->ctrl_mode == ADC_MODE_CHOPPE)
		ret = adc_chopper_config(adc, prop);
	else
		ret = adc_normal_config(adc, prop);
	return ret;
}

static int xr_pmic_adc_do_conversion(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop,
			struct iio_chan_spec const *chan,
			uint16_t *data_code)
{
	int ret;
	uint8_t reg_buf;

	mutex_lock(&adc->lock);

	ret = xr_pmic_adc_configure(adc, prop);
	if (ret) {
		dev_err(adc->dev, "ADC configure failed with %d\n", ret);
		goto unlock;
	}
	ret = xr_pmic_adc_wait_done(adc);
	if (ret) {
		dev_err(adc->dev, "ADC sampling timeout with %d\n", ret);
		goto unlock;
	}
	ret = xr_pmic_adc_check_err_event(adc);
	if (ret) {
		dev_err(adc->dev, "xr_pmic_adc_check_err_event failed with %d\n", ret);
		goto unlock;
	}
	ret = xr_pmic_adc_read_voltage_and_thermal_data(adc, prop, data_code);
	if (ret)
		goto unlock;

	ret = xr_pmic_adc_enable(adc, ADC_DISABLE_SIG);
	if (ret)
		goto unlock;

	/* close the ldo_adc configuration */
	reg_buf = ADC_DISABLE_SIG;
	ret = pmic_adc_write(adc, TOP_REG_LDO_ADC_PWR_REG,
				&reg_buf, sizeof(reg_buf));

unlock:
	mutex_unlock(&adc->lock);

	return ret;
}

typedef int (*adc_do_conversion)(struct xr_pmic_adc_chip *adc,
			struct xr_pmic_adc_channel_prop *prop,
			struct iio_chan_spec const *chan,
			uint16_t *data_code);

static irqreturn_t xr_pmic_adc_isr(int irq, void *dev_id)
{
	struct xr_pmic_adc_chip *adc = dev_id;

	complete(&adc->complete);

	return IRQ_HANDLED;
}

#if KERNEL_VERSION(6, 6, 0) >= LINUX_VERSION_CODE
static int xr_pmic_adc_of_xlate(struct iio_dev *indio_dev,
				const struct of_phandle_args *iiospec)
{
	struct xr_pmic_adc_chip *adc = iio_priv(indio_dev);
	int i;

	for (i = 0; i < adc->nchannels; i++)
		if (adc->chan_props[i].channel == iiospec->args[0])
			return i;

	return -EINVAL;
}
#else
static int xr_pmic_adc_of_xlate(struct iio_dev *indio_dev,
				const struct fwnode_reference_args *iiospec)
{
	struct xr_pmic_adc_chip *adc = iio_priv(indio_dev);
	int i;

	for (i = 0; i < adc->nchannels; i++)
		if (adc->chan_props[i].channel == iiospec->args[0])
			return i;

	return -EINVAL;
}
#endif

static int adc_read_raw_common(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan, int *val, int *val2,
			 long mask, adc_do_conversion do_conv)
{
	struct xr_pmic_adc_chip *adc = iio_priv(indio_dev);
	struct xr_pmic_adc_channel_prop *prop = NULL;
	uint16_t adc_code;
	int ret;

	prop = &adc->chan_props[chan->address];
	pr_debug(" mode:0x%x, channel:0x%x\n",
		adc->chan_props[chan->address].ctrl_mode, adc->chan_props[chan->address].channel);
	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = do_conv(adc, prop, chan, &adc_code);
		if (ret)
			return ret;
		*val = adc_code;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int xr_pmic_adc_read_raw(struct iio_dev *indio_dev,
			 struct iio_chan_spec const *chan, int *val, int *val2,
			 long mask)
{
	return adc_read_raw_common(indio_dev, chan, val, val2,
				mask, xr_pmic_adc_do_conversion);
}

static const struct iio_info xr_pmic_adc_info = {
	.read_raw = xr_pmic_adc_read_raw,
#if KERNEL_VERSION(6, 6, 0) >= LINUX_VERSION_CODE
	.of_xlate = xr_pmic_adc_of_xlate,
#else
	.fwnode_xlate = xr_pmic_adc_of_xlate,
#endif

};

static int xr_pmic_adc_get_dt_channel_data(struct xr_pmic_adc_chip *adc,
					struct xr_pmic_adc_channel_prop *prop,
					struct device_node *node,
					const struct xr_adc_data *data)
{
	const char *name = node->name;
	struct device *dev = adc->dev;
	uint32_t chan;
	int ret;

	ret = of_property_read_u32(node, "chan", &chan);
	if (ret) {
		dev_err(dev, "invalid channel number at %s\n", name);
		return ret;
	}

	if (chan > ADC_MAX_CHANNEL ||
		!data->adc_chans[chan].datasheet_name) {
		dev_err(dev, "%s invalid channel number %d\n", name, chan);
		return -EINVAL;
	}
	/* the channel has DT description */
	prop->channel = chan;
	prop->datasheet_name = data->adc_chans[chan].datasheet_name;
	dev_dbg(dev, "%02x name %s\n", chan, name);
	return 0;
}

static const struct xr_adc_data xr_pmic_adc_data = {
	.adc_chans = adc_chans_pmic,
	.info = &xr_pmic_adc_info,
};

static const struct of_device_id xr_pmic_adc_match_table[] = {
	{
		.compatible = "xring,pmic-adc",
		.data = &xr_pmic_adc_data,
	},
	{}
};
MODULE_DEVICE_TABLE(of, xr_pmic_adc_match_table);

static int xr_pmic_adc_get_dt_data(struct xr_pmic_adc_chip *adc, struct device_node *node)
{
	const struct xr_adc_channels *adc_chan = NULL;
	struct iio_chan_spec *iio_chan = NULL;
	struct xr_pmic_adc_channel_prop prop = {0}, *chan_props = NULL;
	struct device_node *child = NULL;
	uint32_t index = 0;
	int ret;
	uint32_t ctrl_mode;

	adc->nchannels = of_get_available_child_count(node);
	if (!adc->nchannels)
		return -EINVAL;

	adc->iio_chans = devm_kcalloc(adc->dev, adc->nchannels,
					sizeof(*adc->iio_chans), GFP_KERNEL);
	if (!adc->iio_chans)
		return -ENOMEM;

	adc->chan_props = devm_kcalloc(adc->dev, adc->nchannels,
					sizeof(*adc->chan_props), GFP_KERNEL);
	if (!adc->chan_props)
		return -ENOMEM;

	chan_props = adc->chan_props;
	iio_chan = adc->iio_chans;
	adc->data = of_device_get_match_data(adc->dev);
	if (!adc->data)
		adc->data = &xr_pmic_adc_data;

	/* Gets adc mode */
	ret = of_property_read_u32(node, "mode", &ctrl_mode);
	if (ret) {
		dev_err(adc->dev, "invalid mode number\n");
		return ret;
	}
	for_each_available_child_of_node(node, child) {
		ret = xr_pmic_adc_get_dt_channel_data(adc, &prop, child, adc->data);
		if (ret) {
			of_node_put(child);
			return ret;
		}
		prop.ctrl_mode = ctrl_mode;
		*chan_props = prop;
		adc_chan = &adc->data->adc_chans[prop.channel];
		iio_chan->channel = prop.channel;
		pr_debug("channel: %d, iio_chan->channel:%d\n", prop.channel, iio_chan->channel);
		iio_chan->type = adc_chan->type;
		iio_chan->info_mask_separate = adc_chan->info_mask;
		iio_chan->address = index;
		iio_chan->datasheet_name = prop.datasheet_name;
		iio_chan->extend_name = prop.datasheet_name;
		iio_chan++;
		chan_props++;
		index++;
	}

	return 0;
}

static int xr_pmic_adc_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct iio_dev *indio_dev = NULL;
	struct xr_pmic_adc_chip *adc = NULL;
	struct regmap *regmap = NULL;
	int ret = 0, irq_eoc = 0;
	uint32_t base = 0;

	regmap = dev_get_regmap(dev->parent, NULL);
	if (!regmap) {
		dev_err(dev, "adc dev_get_regmap failed\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "base_addr", &base);
	if (ret < 0) {
		dev_err(dev, "invalid adc base_addr\n");
		return ret;
	}

	indio_dev = devm_iio_device_alloc(dev, sizeof(*adc));
	if (!indio_dev)
		return -ENOMEM;

	adc = iio_priv(indio_dev);
	adc->regmap = regmap;
	adc->dev = dev;
	adc->base = base;

	init_completion(&adc->complete);
	mutex_init(&adc->lock);

	ret = xr_pmic_adc_get_dt_data(adc, node);
	if (ret) {
		dev_err(dev, "adc get dt data failed\n");
		return ret;
	}

	irq_eoc = platform_get_irq(pdev, 0);
	if (irq_eoc < 0) {
		if (irq_eoc == -EPROBE_DEFER || irq_eoc == -EINVAL)
			return irq_eoc;
		adc->poll_eoc = true;
	} else {
		ret = devm_request_irq(dev, irq_eoc, xr_pmic_adc_isr, 0,
					"pm-xr_pmic_adc", adc);
		if (ret)
			return ret;
	}

	indio_dev->name = pdev->name;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = adc->data->info;
	indio_dev->channels = adc->iio_chans;
	indio_dev->num_channels = adc->nchannels;
	ret = devm_iio_device_register(dev, indio_dev);
	if (ret) {
		dev_err(dev, "iio device register fail\n");
		return ret;
	}
	dev_info(dev, "%s succ!\n", __func__);
	return ret;
}

static int xr_pmic_adc_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s succ!\n", __func__);
	return 0;
}

static struct platform_driver xr_pmic_adc_driver = {
	.driver = {
		.name = "xr_pmic_adc",
		.of_match_table = xr_pmic_adc_match_table,
	},
	.probe = xr_pmic_adc_probe,
	.remove = xr_pmic_adc_remove,
};

int xr_pmic_adc_init(void)
{
	return platform_driver_register(&xr_pmic_adc_driver);
}

void xr_pmic_adc_exit(void)
{
	platform_driver_unregister(&xr_pmic_adc_driver);
}
