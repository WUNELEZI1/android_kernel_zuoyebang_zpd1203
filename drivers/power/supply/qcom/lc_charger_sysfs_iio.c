/* lc charger sysfs iio */
#include "lc_charger_sysfs.h"

bool is_chan_valid(struct lc_chg_sysfs *chip,
            enum lc_chg_iio_type type, int chan)
{
	int rc = 0;

    switch(type) {
    case LC_MAIN:
            if (IS_ERR(chip->main_chg_ext_iio_chans[chan])) {
                pr_err("%s LC_MAIN chans is err\n", __func__);
                goto _err;
            }
            if (!chip->main_chg_ext_iio_chans[chan]) {
                chip->main_chg_ext_iio_chans[chan] = iio_channel_get(chip->dev,
                                                        main_chg_ext_iio_chan_name[chan]);
                if (IS_ERR(chip->main_chg_ext_iio_chans[chan])) {
                    rc = PTR_ERR(chip->main_chg_ext_iio_chans[chan]);
                    if (rc == -EPROBE_DEFER)
                        chip->main_chg_ext_iio_chans[chan] = NULL;
                    pr_err("Failed to get IIO channel %s, rc=%d\n",
                                main_chg_ext_iio_chan_name[chan], rc);
                    goto _err;
                }
            }
            break;

    case LC_BMS:
            if (IS_ERR(chip->fg_ext_iio_chans[chan])) {
                pr_err("%s LC_MAIN chans is err\n", __func__);
                goto _err;
            }
            if (!chip->fg_ext_iio_chans[chan]) {
                chip->fg_ext_iio_chans[chan] = iio_channel_get(chip->dev,
                                                        fg_ext_iio_chan_name[chan]);
                if (IS_ERR(chip->fg_ext_iio_chans[chan])) {
                    rc = PTR_ERR(chip->fg_ext_iio_chans[chan]);
                    if (rc == -EPROBE_DEFER)
                        chip->fg_ext_iio_chans[chan] = NULL;
                    pr_err("Failed to get IIO channel %s, rc=%d\n",
                                fg_ext_iio_chan_name[chan], rc);
                    goto _err;
                }
            }
            break;
    case LC_CP_MASTER:
            if (IS_ERR(chip->cp_ext_iio_chans[chan])) {
                pr_err("%s LC_MAIN chans is err\n", __func__);
                goto _err;
            }
            if (!chip->cp_ext_iio_chans[chan]) {
                chip->cp_ext_iio_chans[chan] = iio_channel_get(chip->dev,
                                                        cp_ext_iio_chan_name[chan]);
                if (IS_ERR(chip->cp_ext_iio_chans[chan])) {
                    rc = PTR_ERR(chip->cp_ext_iio_chans[chan]);
                    if (rc == -EPROBE_DEFER)
                        chip->cp_ext_iio_chans[chan] = NULL;
                    pr_err("Failed to get IIO channel %s, rc=%d\n",
                            cp_ext_iio_chan_name[chan], rc);
                    goto _err;
                }
            }
            break;
    case LC_CC:
            if (IS_ERR(chip->cc_ext_iio_chans[chan])) {
                pr_err("%s LC_MAIN chans is err\n", __func__);
                goto _err;
            }
            if (!chip->cc_ext_iio_chans[chan]) {
                chip->cc_ext_iio_chans[chan] = iio_channel_get(chip->dev,
                                                        cc_ext_iio_chan_name[chan]);
                if (IS_ERR(chip->cc_ext_iio_chans[chan])) {
                    rc = PTR_ERR(chip->cc_ext_iio_chans[chan]);
                    if (rc == -EPROBE_DEFER)
                        chip->cc_ext_iio_chans[chan] = NULL;
                    pr_err("%s Failed to get IIO channel %s, rc: %d\n",
                            __func__, cc_ext_iio_chan_name[chan], rc);
                    goto _err;
                }
            }
            break;
#if 0
    case LC_DS:
            if (IS_ERR(chip->ds_ext_iio_chans[chan])) {
                pr_err("%s LC_MAIN chans is err\n", __func__);
                goto _err;
            }
            if (!chip->ds_ext_iio_chans[chan]) {
                chip->ds_ext_iio_chans[chan] = iio_channel_get(chip->dev,
                                                        ds_ext_iio_chan_name[chan]);
                if (IS_ERR(chip->ds_ext_iio_chans[chan])) {
                    rc = PTR_ERR(chip->ds_ext_iio_chans[chan]);
                    if (rc == -EPROBE_DEFER)
                        chip->ds_ext_iio_chans[chan] = NULL;
                    pr_err("Failed to get IIO channel %s, rc=%d\n",
                            ds_ext_iio_chan_name[chan], rc);
                    goto _err;
                }
            }
            break;
#endif
    default:
            pr_err("%s Invalid IIO channel type:%d\n", __func__, type);
                    goto _err;
    }

	return true;
_err:
return false;
}

int lc_chg_sysfs_get_iio_channel(struct lc_chg_sysfs *lc_chg,
                enum lc_chg_iio_type type, int channel, int *val)
{
    struct iio_channel *iio_chan_list = NULL;
    int rc = 0;

    switch (type) {
    case LC_MAIN:
        if (!is_chan_valid(lc_chg, LC_MAIN, channel))
            return -ENODEV;
        iio_chan_list = lc_chg->main_chg_ext_iio_chans[channel];
        break;
    case LC_BMS:
        if (!is_chan_valid(lc_chg, LC_BMS, channel))
            return -ENODEV;
        iio_chan_list = lc_chg->fg_ext_iio_chans[channel];
        break;
    case LC_CP_MASTER:
        if (is_chan_valid(lc_chg, LC_CP_MASTER, channel)) {
            iio_chan_list = lc_chg->cp_ext_iio_chans[channel];
        } else if (is_chan_valid(lc_chg, LC_CP_MASTER, channel+SEC_IIO_CHANNEL_OFFSET)) {
            iio_chan_list = lc_chg->cp_ext_iio_chans[channel+SEC_IIO_CHANNEL_OFFSET];
        } else {
            pr_err("There is no vaild cp channel!\n");
            return -ENODEV;
        }
        break;
    case LC_CC:
        if (!is_chan_valid(lc_chg, LC_CC, channel)) {
            pr_err("%s LC_CC channel: %d is envaild\n", __func__, channel);
            return -ENODEV;
        }
        iio_chan_list = lc_chg->cc_ext_iio_chans[channel];
        break;
#if 0
    case LC_DS:
        if (!is_chan_valid(lc_chg, LC_DS, channel))
            return -ENODEV;
        iio_chan_list = lc_chg->ds_ext_iio_chans[channel];
        break;
#endif
    default:
        pr_err_ratelimited("iio_type %d is not supported\n", type);
        return -EINVAL;
    }

    rc = iio_read_channel_processed(iio_chan_list, val);
    pr_debug("%s type: %d, channel: %d, val:%d, rc:%d\n", __func__, type, channel, *val, rc);
    return rc < 0 ? rc : 0;
}
EXPORT_SYMBOL(lc_chg_sysfs_get_iio_channel);

int lc_chg_sysfs_set_iio_channel(struct lc_chg_sysfs *lc_chg,
			enum lc_chg_iio_type type, int channel, int val)
{
    struct iio_channel *iio_chan_list = NULL;
    int rc = 0;

    switch (type) {
    case LC_MAIN:
        if (!is_chan_valid(lc_chg, LC_MAIN, channel)) {
            pr_err("%s LC_MAIN channel is invalid\n", __func__);
            return -ENODEV;
        }
        iio_chan_list = lc_chg->main_chg_ext_iio_chans[channel];
        break;

    case LC_BMS:
        if (!is_chan_valid(lc_chg, LC_BMS, channel)) {
            pr_err("%s LC_BMS channel is invalid\n", __func__);
            return -ENODEV;
        }
        iio_chan_list = lc_chg->fg_ext_iio_chans[channel];
        break;
    case LC_CP_MASTER:
        if (is_chan_valid(lc_chg, LC_CP_MASTER, channel)) {
            iio_chan_list = lc_chg->cp_ext_iio_chans[channel];
        } else if (is_chan_valid(lc_chg, LC_CP_MASTER, channel+SEC_IIO_CHANNEL_OFFSET)) {
            iio_chan_list = lc_chg->cp_ext_iio_chans[channel+SEC_IIO_CHANNEL_OFFSET];
        } else {
            pr_err("There is no vaild cp channel!\n");
            return -ENODEV;
        }
        break;
    case LC_CC:
        if (!is_chan_valid(lc_chg, LC_CC, channel)) {
            pr_err("%s LC_CC channel: %d is envaild\n", __func__, channel);
            return -ENODEV;
        }
        iio_chan_list = lc_chg->cc_ext_iio_chans[channel];
        break;

    default:
        pr_err_ratelimited("iio_type %d is not supported\n", type);
        return -EINVAL;
    }

    rc = iio_write_channel_raw(iio_chan_list, val);
    pr_err("%s type: %d, channel: %d, val:%d, rc:%d\n", __func__, type, channel, val, rc);
    return rc < 0 ? rc : 0;
}
EXPORT_SYMBOL(lc_chg_sysfs_set_iio_channel);

static int lc_chg_sysfs_iio_read_raw(struct iio_dev *indio_dev,
    struct iio_chan_spec const *chan, int *val1, int *val2, long mask)
{
    struct lc_chg_sysfs *chip = NULL;
    union power_supply_propval pval = {0, };
    int rc = 0;
    *val1 = 0;

    if (IS_ERR_OR_NULL(indio_dev) || IS_ERR_OR_NULL(chan)) {
        pr_err("%s iio point is err or null\n", __func__);
        return -EINVAL;
    }

    chip = iio_priv(indio_dev);
    if (IS_ERR_OR_NULL(chip)) {
        pr_err("%s chip is err or null\n", __func__);
        return -EINVAL;
    }

	switch (chan->channel) {
    case PSY_IIO_LC_TEST:
        *val1 = chip->test_flag;
        pr_info("%s read tese_flag: %d\n", __func__, chip->test_flag);
        break;
    case PSY_IIO_LC_SCREEN_STA:
        *val1 = chip->backlight_status;
        break;
    case PSY_IIO_SHIP_MODE:
        rc = lc_get_ship_mode(chip, &pval);
        *val1 = pval.intval;
        break;
    case PSY_IIO_SHIPMODE_COUNT_RESET:
        *val1 = chip->use_shipcount_en;
        break;
    default:
        pr_err("Unsupported battery IIO chan %d\n", chan->channel);
        rc = -EINVAL;
        break;
    }
    if (rc < 0) {
        pr_err_ratelimited("Couldn't read IIO channel %d, rc = %d\n",
            chan->channel, rc);
        return rc;
    }
    return IIO_VAL_INT;
}

static int lc_chg_sysfs_iio_write_raw(struct iio_dev *indio_dev,
    struct iio_chan_spec const *chan, int val1, int val2, long mask)
{
    struct lc_chg_sysfs *chip = NULL;
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (IS_ERR_OR_NULL(indio_dev) || IS_ERR_OR_NULL(chan)) {
        pr_err("%s iio point is err or null\n", __func__);
        return -EINVAL;
    }

    chip = iio_priv(indio_dev);
    if (IS_ERR_OR_NULL(chip)) {
        pr_err("%s chip is err or null\n", __func__);
        return -EINVAL;
    }

    switch (chan->channel) {
    case PSY_IIO_LC_TEST:
        chip->test_flag = val1;
        pr_info("%s set tese_flag: %d\n", __func__, chip->test_flag);
        break;
    case PSY_IIO_LC_SCREEN_STA:
        chip->backlight_status = !!val1;
        rc = lc_set_cid_status(chip);
        break;
    case PSY_IIO_LC_CID_STA:
        rc = lc_set_cid_status(chip);
        break;
    case PSY_IIO_SHIP_MODE:
        pval.intval = val1;
        rc = lc_set_ship_mode(chip, &pval);
        break;
    case PSY_IIO_SHIPMODE_COUNT_RESET:
        pval.intval = val1;
        rc = use_count_reset(chip, &pval);
        break;
    default:
        pr_err("Unsupported battery IIO chan %d\n", chan->channel);
        rc = -EINVAL;
        break;
    }

    if (rc < 0) {
        pr_err_ratelimited("Couldn't write IIO channel %d, rc = %d\n",
        chan->channel, rc);
        return rc;
    }
    return IIO_VAL_INT;
}

static int lc_chg_sysfs_iio_of_xlate(struct iio_dev *indio_dev,
                                const struct fwnode_reference_args *iiospec)
{
	struct lc_chg_sysfs *chip = NULL;
	struct iio_chan_spec *iio_chan = NULL;
	int i = 0;

    if (IS_ERR_OR_NULL(indio_dev)) {
        pr_err("%s iio point is err or null\n", __func__);
        return -EINVAL;
    }

    chip = iio_priv(indio_dev);
    if (IS_ERR_OR_NULL(chip) || IS_ERR_OR_NULL(chip->iio_chan)) {
        pr_err("%s chip is err or null\n", __func__);
        return -EINVAL;
    }

    iio_chan = chip->iio_chan;
	for (i = 0; i < ARRAY_SIZE(lc_chg_sysfs_iio_psy_channels);
					i++, iio_chan++)
		if (iio_chan->channel == iiospec->args[0])
			return i;

	return -EINVAL;
}

static const struct iio_info lc_chg_sysfs_iio_info = {
	.read_raw   = lc_chg_sysfs_iio_read_raw,
	.write_raw  = lc_chg_sysfs_iio_write_raw,
	.fwnode_xlate   = lc_chg_sysfs_iio_of_xlate,
};

int lc_chg_init_iio_psy(struct lc_chg_sysfs *chip)
{
    struct iio_dev *indio_dev =  NULL;
    struct iio_chan_spec *chan = NULL;
    int num_iio_channels = ARRAY_SIZE(lc_chg_sysfs_iio_psy_channels);
    int rc = 0, i = 0;

    pr_info("%s start\n", __func__);
    if (IS_ERR_OR_NULL(chip)) {
        pr_err("%s chip point is err or null\n", __func__);
        return -ENOMEM;
    }

    indio_dev = chip->indio_dev;
    chip->iio_chan = devm_kcalloc(chip->dev, num_iio_channels,
                            sizeof(*chip->iio_chan), GFP_KERNEL);
    if (IS_ERR_OR_NULL(chip->iio_chan)) {
        pr_err("%s failed to alloc memory for iio_chan_spec\n", __func__);
        return -ENOMEM;
    }

    chip->int_iio_chans = devm_kcalloc(chip->dev, num_iio_channels,
                            sizeof(*chip->int_iio_chans), GFP_KERNEL);
    if (IS_ERR_OR_NULL(chip->int_iio_chans)) {
        pr_err("%s is err or null int_iio_chans\n", __func__);
        return -ENOMEM;
    }

    indio_dev->info = &lc_chg_sysfs_iio_info;
    indio_dev->dev.parent = chip->dev;
    indio_dev->dev.of_node = chip->dev->of_node;
    indio_dev->modes = INDIO_DIRECT_MODE;
    indio_dev->channels = chip->iio_chan;
    indio_dev->num_channels = num_iio_channels;
    indio_dev->name = "lc_chg_sysfs";

    for (i = 0; i < num_iio_channels; i++) {
        pr_info("%s num_iio_channels init number: %d\n", __func__, i);
        chip->int_iio_chans[i].indio_dev = indio_dev;
        chan = &chip->iio_chan[i];
        chip->int_iio_chans[i].channel = chan;
        chan->address = i;
        chan->channel = lc_chg_sysfs_iio_psy_channels[i].channel_num;
        chan->type = lc_chg_sysfs_iio_psy_channels[i].type;
        chan->datasheet_name = lc_chg_sysfs_iio_psy_channels[i].datasheet_name;
        chan->extend_name = lc_chg_sysfs_iio_psy_channels[i].datasheet_name;
        chan->info_mask_separate = lc_chg_sysfs_iio_psy_channels[i].info_mask;
    }

    rc = devm_iio_device_register(chip->dev, indio_dev);
    if (rc) {
        pr_err("Failed to register nopmi chg IIO device, rc=%d\n", rc);
    }

    pr_info("%s nopmi chg IIO device, rc=%d\n", __func__, rc);
    return 0;
}
EXPORT_SYMBOL(lc_chg_init_iio_psy);

int lc_chg_ext_init_iio_psy(struct lc_chg_sysfs *lc_chg)
{
    pr_info("%s enter\n", __func__);
    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s is err or null lc_chg\n",__func__);
        return -ENOMEM;
    }

    lc_chg->main_chg_ext_iio_chans = devm_kcalloc(lc_chg->dev,
                                ARRAY_SIZE(main_chg_ext_iio_chan_name),
                                sizeof(*lc_chg->main_chg_ext_iio_chans),
                                GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg->main_chg_ext_iio_chans)) {
        pr_err("%s is err or null main_chg_ext_iio_chans\n",__func__);
        return -ENOMEM;
    }


    lc_chg->fg_ext_iio_chans = devm_kcalloc(lc_chg->dev,
                                ARRAY_SIZE(fg_ext_iio_chan_name),
                                sizeof(*lc_chg->fg_ext_iio_chans),
                                GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg->fg_ext_iio_chans)) {
        pr_err("%s is err or null fg_ext_iio_chans\n",__func__);
        return -ENOMEM;
    }

    lc_chg->cp_ext_iio_chans = devm_kcalloc(lc_chg->dev,
                                ARRAY_SIZE(cp_ext_iio_chan_name),
                                sizeof(*lc_chg->cp_ext_iio_chans),
                                GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg->cp_ext_iio_chans)) {
        pr_err("%s is err or null cp_ext_iio_chans\n",__func__);
        return -ENOMEM;
    }

    lc_chg->cc_ext_iio_chans = devm_kcalloc(lc_chg->dev,
                                ARRAY_SIZE(cc_ext_iio_chan_name),
                                sizeof(*lc_chg->cc_ext_iio_chans),
                                GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg->cc_ext_iio_chans)) {
        pr_err("%s is err or null cc_ext_iio_chans\n",__func__);
        return -ENOMEM;
    }
#if 0
    lc_chg->ds_ext_iio_chans = devm_kcalloc(lc_chg->dev,
                                ARRAY_SIZE(ds_ext_iio_chan_name),
                                sizeof(*lc_chg->ds_ext_iio_chans),
                                GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg->ds_ext_iio_chans)) {
        pr_err("%s is err or null ds_ext_iio_chans\n",__func__);
        return -ENOMEM;
    }
#endif

    pr_info("%s successful\n", __func__);
    return 0;
}
EXPORT_SYMBOL(lc_chg_ext_init_iio_psy);