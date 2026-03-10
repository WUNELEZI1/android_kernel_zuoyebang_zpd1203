#include <linux/reboot.h>
#include "lc_charger_sysfs.h"
#include "bq28z610.h"

int g_shipmode_en = 0;
static struct notifier_block		reboot_notifier;
extern int smblib_enable_ship_mode(int val);

static void lc_chg_sysfs_release(struct class *pcls)
{
    pr_err("%s enter\n", __func__);
    if (NULL != pcls) {
        kfree(pcls);
    }
    return;
}

/* ================== */
/*      Functions     */
/* ================== */

int lc_get_uisoc(struct lc_chg_sysfs *lc_chg, int *val)
{
    union power_supply_propval pval = {0, };
    struct power_supply *batt_psy;
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    batt_psy = power_supply_get_by_name("battery");
    if (IS_ERR_OR_NULL(batt_psy)) {
        pr_err("%s failed to get batt_psy", __func__);
        return ret;
    }

    ret = power_supply_get_property(batt_psy, POWER_SUPPLY_PROP_CAPACITY, &pval);
    if (ret < 0) {
        pr_err("%s failed to get soc_prop", __func__);
        return ret;
    }

    *val = pval.intval;
    pr_info("%s intval:%d val:%d\n", __func__, pval.intval, *val);
    return ret;
}
EXPORT_SYMBOL(lc_get_uisoc);

static const char *get_usbc_text_name(u32 usb_type)
{
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(power_supply_usbc_text); i++) {
		if (i == usb_type)
			return power_supply_usbc_text[i];
	}
	return "Unknown";
}

int lc_get_typec_cc_orientation(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_CC, TYPEC_CC_ORIENTATION, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get typec_cc_orientation, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_debug("%s %d\n", __func__, pval->intval);
    return ret;

_err:
return ret;
}

#if 0
int lc_get_quick_charge_type(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, QUICK_CHARGE_TYPE, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get typec_cc_orientation, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_soc_decimal(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_BMS, SOC_DECIMAL, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get soc_decimal, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_soc_decimal_rate(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_BMS, SOC_DECIMAL_RATE, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get soc_decimal_rate, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_shutdown_delay(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_BMS, SHUTDOWN_DELAY, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get shutdown_delay, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}
#endif
int lc_get_usb_real_type(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, USB_REAL_TYPE, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get usb_real_type, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_debug("%s %d\n", __func__, pval->intval);
    return ret;

_err:
    return ret;
}

int lc_get_mtbf_current(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, MTBF_CURRENT, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get mtbf_current, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_info("%s %d\n", __func__, pval->intval);
    return ret;

_err:
    return ret;
}

int lc_set_mtbf_current(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_set_iio_channel(lc_chg, LC_MAIN, MTBF_CURRENT, pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get mtbf_current, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_info("%s %d\n", __func__, pval->intval);
    return ret;

_err:
    return ret;
}

int lc_get_input_suspend(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, INPUT_SUSPEND, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get input_suspend, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_info("%s %d\n", __func__, pval->intval);
    return ret;

_err:
return ret;
}

int lc_set_input_suspend(struct lc_chg_sysfs *lc_chg, union power_supply_propval pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_set_iio_channel(lc_chg, LC_MAIN, INPUT_SUSPEND, pval.intval);
            if (ret < 0) {
                pr_err("%s failed to get input_suspend, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_info("%s %d\n", __func__, pval.intval);
    return ret;

_err:
    return ret;
}

int lc_get_ship_mode(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, SET_SHIP_MODE, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get SET_SHIP_MODE, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_set_ship_mode(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		pr_info("%s LC_SET_PROP_QCOM_IIO\n", __func__);
		ret = lc_chg_sysfs_set_iio_channel(lc_chg, LC_MAIN, SET_SHIP_MODE, pval->intval);
		if (ret < 0) {
			pr_err("%s failed to set SHIP_MODE, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s set_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_info("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_apdo_curr(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, APDO_CURR, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get apdo_curr, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_debug("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_apdo_volt(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
	int ret = -EINVAL;

	if (IS_ERR_OR_NULL(lc_chg)) {
		pr_err("%s lc_chg is err or null\n", __func__);
		return ret;
	}

	switch (lc_chg->get_prop_style) {
	case LC_GET_PROP_QCOM_IIO:
		ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, APDO_VOLT, &pval->intval);
		if (ret < 0) {
			pr_err("%s failed to get apdo_volt, ret: %d\n", __func__, ret);
			goto _err;
		}
		break;
	case LC_GET_PROP_MTK_OPS:
		break;
	default:
		pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
		return ret;
	}

	pr_debug("%s %d\n", __func__, pval->intval);
	return ret;

_err:
	return ret;
}

int lc_get_typec_mode(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_CC, TYPEC_MODE, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get typec_mode, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_debug("%s %d\n", __func__, pval->intval);
    return ret;

_err:
return ret;
}

__maybe_unused static bool is_need_set_drp_to_sink(struct lc_chg_sysfs *lc_chg)
{
    union power_supply_propval pval = {0, };
    int ret = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_info("%s point lc_chg is err or null\n", __func__);
        return false;
    }

    ret = lc_get_typec_cc_orientation(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get typec_cc_orientation fail, %d\n", __func__, ret);
        return ret;
    }
    lc_chg->cc_orientation = pval.intval;

    pr_info("%s backlight_status: %d, audio_cctog: %d, cc_orientation: %d\n",
                __func__,
                lc_chg->backlight_status,
                lc_chg->audio_cctog,
                lc_chg->cc_orientation);

    if ((lc_chg->backlight_status) && (!lc_chg->audio_cctog) && (lc_chg->cc_orientation <= 0)) {
        return true;
    } else {
        return false;
    }
}

__maybe_unused static int lc_set_drp_status(struct lc_chg_sysfs *lc_chg, bool en)
{
    int ret = -EINVAL;
    int typec_role = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_info("%s point lc_chg is err or null\n", __func__);
        return ret;
    }

    typec_role = en ? TYPEC_PR_DUAL : TYPEC_PR_SINK;
    switch (lc_chg->get_prop_style) {
        case LC_GET_PROP_QCOM_IIO:
                ret = lc_chg_sysfs_set_iio_channel(lc_chg, LC_CC, TYPEC_POWER_ROLE, typec_role);
                if (ret < 0) {
                    pr_err("%s failed to set typec_cc_orientation, ret: %d\n", __func__, ret);
                    goto _err;
                }
            break;
        case LC_GET_PROP_MTK_OPS:
                break;
        default:
                pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
                return ret;
    }

    return ret;

_err:
return ret;
}

int lc_set_cid_status(struct lc_chg_sysfs *lc_chg)
{
#if IS_ENABLED(CONFIG_FACTORY_BUILD)
    pr_info("%s factory version needn't handle\n", __func__);
    return 0;
#endif
    union power_supply_propval pval = {0, };
    int ret = -EINVAL;
    int need_change = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s point lc_chg is err or null\n", __func__);
        return ret;
    }

    ret = lc_get_pd_active(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get pd_active fail\n", __func__);
    }

    need_change = is_need_set_drp_to_sink(lc_chg);
    if (need_change < 0) {
        pr_err("%s check cid status fail, %d\n", __func__, need_change);
        return need_change;
    }

    if (pval.intval == 1 || pval.intval == 2 || need_change == lc_chg->cid_status) {
        pr_info("%s needn't handle, pd_active: %d, cid_status: %d\n", __func__, pval.intval, lc_chg->cid_status);
        return ret;
    } else if (need_change) {
        pr_info("%s set drp status to sink\n", __func__);
        ret = lc_set_drp_status(lc_chg, false);
        if (ret < 0) {
            pr_err("%s lc_set_drp_status fail\n", __func__);
            return -ret;
        }
        lc_chg->cid_status = need_change;
    } else {
        pr_info("%s set drp status to try_sink\n", __func__);
        ret = lc_set_drp_status(lc_chg, true);
        if (ret < 0) {
            pr_err("%s lc_set_drp_status fail\n", __func__);
            return -ret;
        }
        lc_chg->cid_status = need_change;
    }

    pr_info("%s successful\n", __func__);
    return ret;
}
EXPORT_SYMBOL(lc_set_cid_status);

int lc_get_pd_active(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return ret;
    }

    switch (lc_chg->get_prop_style) {
    case LC_GET_PROP_QCOM_IIO:
            ret = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, PD_ACTIVE, &pval->intval);
            if (ret < 0) {
                pr_err("%s failed to get pd_active, ret: %d\n", __func__, ret);
                goto _err;
            }
            break;
    case LC_GET_PROP_MTK_OPS:
            break;
    default:
            pr_err("%s get_prop_style %d is not supported\n",__func__, lc_chg->get_prop_style);
            return ret;
    }

    pr_debug("%s %d\n", __func__, pval->intval);
    return ret;

_err:
    return ret;
}

/* ================= */
/*      Policy       */
/* ================= */

static ssize_t test_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);

    pr_info("%s test_flag: %d\n", __func__, lc_chg->test_flag);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->test_flag);
}

static ssize_t test_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->test_flag = pval.intval;
    pr_info("%s test_flag: %d %d\n", __func__, lc_chg->test_flag, pval.intval);
    return count;
}
static CLASS_ATTR_RW(test);

static ssize_t typec_cc_orientation_show(struct class *c,
                        struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = 0;

    ret = lc_get_typec_cc_orientation(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get typec_cc_orientation fail\n", __func__);
        return ret;
    }

    lc_chg->cc_orientation = pval.intval;
    pr_info("%s cc_orientation: %d\n", __func__, lc_chg->cc_orientation);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->cc_orientation);
}
static CLASS_ATTR_RO(typec_cc_orientation);

static const char *get_usb_type_name(u32 usb_type)
{
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(power_supply_usb_type_text); i++) {
		if (i == usb_type)
			return power_supply_usb_type_text[i];
	}
	return "Unknown";
}

static ssize_t real_type_show(struct class *c,
                        struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = 0;

    ret = lc_get_usb_real_type(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get usb_real_type fail\n", __func__);
        return ret;
    }

    lc_chg->usb_real_type = pval.intval;

    ret = lc_get_pd_active(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get pd_active fail\n", __func__);
        return ret;
    }

    if (lc_chg->usb_real_type == POWER_SUPPLY_TYPE_USB_PD && pval.intval == 2)
        lc_chg->usb_real_type = 17;

    pr_info("%s:usb_real_type %s \n", __func__, get_usb_type_name(lc_chg->usb_real_type));
    return scnprintf(buf, PAGE_SIZE, "%s\n", get_usb_type_name(lc_chg->usb_real_type));
}
static CLASS_ATTR_RO(real_type);

static ssize_t input_suspend_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    int ret = 0;
    union power_supply_propval pval = {0, };
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);


    ret = lc_get_input_suspend(lc_chg, &pval);
    if (ret < 0) {
        pr_err("%s get input_suspend fail\n", __func__);
        return ret;
    }

    lc_chg->input_suspend = pval.intval;

    pr_info("%s input_suspend: %d\n", __func__, lc_chg->input_suspend);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->input_suspend);
}

static ssize_t input_suspend_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->input_suspend = pval.intval;
    ret = lc_set_input_suspend(lc_chg, pval);
    if (ret < 0) {
        pr_err("%s get input_suspend fail\n", __func__);
        return ret;
    }

    pr_info("%s input_suspend: %d %d\n", __func__, lc_chg->input_suspend, pval.intval);
    return count;
}
static CLASS_ATTR_RW(input_suspend);

static ssize_t soc_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = 0;

    ret = lc_get_uisoc(lc_chg, &pval.intval);
    if (ret < 0) {
        pr_err("%s get uisoc fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->ui_soc = pval.intval;
    pr_info("%s ui_soc: %d\n", __func__, lc_chg->ui_soc);

    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->ui_soc);
}
static CLASS_ATTR_RO(soc);

static ssize_t prop_style_show(struct class *c,
                    struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);

    pr_info("%s get_prop_style: %d\n", __func__, lc_chg->get_prop_style);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->get_prop_style);
}

static ssize_t prop_style_store(struct class *c, struct class_attribute *attr,
                            const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->get_prop_style = pval.intval;

    pr_info("%s get_prop_style: %d %d\n", __func__, lc_chg->get_prop_style, pval.intval);
    return count;
}
static CLASS_ATTR_RW(prop_style);

static ssize_t batt_id_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	int batt_id = 0;
	struct bq_fg_chip *gm;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		gm = power_supply_get_drvdata(psy_fg);
		if (!IS_ERR_OR_NULL(gm))
			batt_id = gm->cell_supplier;
		else
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

	return scnprintf(buf, PAGE_SIZE, "%d\n", batt_id);
}
static CLASS_ATTR_RO(batt_id);

static ssize_t resistance_id_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", 100000);
}
static CLASS_ATTR_RO(resistance_id);

static ssize_t auth_manufacturer_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", "mpc8011b");
}
static CLASS_ATTR_RO(auth_manufacturer);

static ssize_t chip_ok_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	struct bq_fg_chip *bq;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		bq = power_supply_get_drvdata(psy_fg);
		if (!IS_ERR_OR_NULL(bq)) {
			return snprintf(buf, PAGE_SIZE, "%d\n", bq->authenticate);
		} else
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

	return -EINVAL;
}
static CLASS_ATTR_RO(chip_ok);

static ssize_t authentic_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	struct bq_fg_chip *bq;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		bq = power_supply_get_drvdata(psy_fg);
		if (!IS_ERR_OR_NULL(bq)) {
			return snprintf(buf, PAGE_SIZE, "%d\n", bq->authenticate);
		} else
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

	return -EINVAL;
}
static CLASS_ATTR_RO(authentic);

static ssize_t cp_bus_current_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    union power_supply_propval pval = {0, };
    struct power_supply *cp_psy;
    int ret = -EINVAL;

    cp_psy = power_supply_get_by_name("ti-cp-standalone");
    if (IS_ERR_OR_NULL(cp_psy)) {
        cp_psy = power_supply_get_by_name("sc-cp-standalone");
        if (IS_ERR_OR_NULL(cp_psy)) {
            pr_err("%s failed to get cp_psy\n", __func__);
            return ret;
        }
    }

    ret = power_supply_get_property(cp_psy, POWER_SUPPLY_PROP_CURRENT_NOW, &pval);
    if (ret < 0) {
        pr_err("%s failed to get cp bus current, ret:%d", __func__, ret);
        return ret;
    }

    return scnprintf(buf, PAGE_SIZE, "%d\n", pval.intval);
}
static CLASS_ATTR_RO(cp_bus_current);

static ssize_t fcc_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *fcc_votable;
    int fcc_val = 0;

    fcc_votable = find_votable("FCC");
    if (!fcc_votable) {
        pr_err("%s failed to get fcc_votable\n", __func__);
        return -EINVAL;
    }

    fcc_val = get_effective_result_locked(fcc_votable);

    return scnprintf(buf, PAGE_SIZE, "%d\n", fcc_val);
}
static CLASS_ATTR_RO(fcc);

static ssize_t fcc_voter_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *fcc_votable;
    const char *fcc_client;

    fcc_votable = find_votable("FCC");
    if (!fcc_votable) {
        pr_err("%s failed to get fcc_votable\n", __func__);
        return -EINVAL;
    }

    fcc_client = get_effective_client_locked(fcc_votable);
    if (!fcc_client) {
        return scnprintf(buf, PAGE_SIZE, "Unknown\n");
    }

    return scnprintf(buf, PAGE_SIZE, "%s\n", fcc_client);
}
static CLASS_ATTR_RO(fcc_voter);

static ssize_t icl_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *icl_votable;
    int icl_val = 0;

    icl_votable = find_votable("USB_ICL");
    if (!icl_votable) {
        pr_err("%s failed to get icl_votable\n", __func__);
        return -EINVAL;
    }

    icl_val = get_effective_result_locked(icl_votable);

    return scnprintf(buf, PAGE_SIZE, "%d\n", icl_val);
}

static ssize_t icl_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    union power_supply_propval pval = {0, };
    struct votable *icl_votable;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    icl_votable = find_votable("USB_ICL");
    if (!icl_votable) {
        pr_err("%s failed to get icl_votable\n", __func__);
        return -EINVAL;
    }
    if (pval.intval >= 0) {
        vote(icl_votable, ICL_STORE, true, pval.intval);
        pr_info("%s set icl_vote: %d\n", __func__, pval.intval);
    } else {
        vote(icl_votable, ICL_STORE, false, 0);
        pr_info("%s cancel icl_vote\n", __func__);
    }
    return count;
}
static CLASS_ATTR_RW(icl);

static ssize_t icl_voter_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *icl_votable;
    const char *icl_client;

    icl_votable = find_votable("USB_ICL");
    if (!icl_votable) {
        pr_err("%s failed to get icl_votable\n", __func__);
        return -EINVAL;
    }

    icl_client = get_effective_client_locked(icl_votable);
    if (!icl_client) {
        return scnprintf(buf, PAGE_SIZE, "Unknown\n");
    }

    return scnprintf(buf, PAGE_SIZE, "%s\n", icl_client);
}
static CLASS_ATTR_RO(icl_voter);

static ssize_t fv_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *fv_votable;
    int fv_val = 0;

    fv_votable = find_votable("FV");
    if (!fv_votable) {
        pr_err("%s failed to get fv_votable\n", __func__);
        return -EINVAL;
    }

    fv_val = get_effective_result_locked(fv_votable);

    return scnprintf(buf, PAGE_SIZE, "%d\n", fv_val);
}
static CLASS_ATTR_RO(fv);

static ssize_t fv_voter_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *fv_votable;
    const char *fv_client;

    fv_votable = find_votable("FV");
    if (!fv_votable) {
        pr_err("%s failed to get fv_votable\n", __func__);
        return -EINVAL;
    }

    fv_client = get_effective_client_locked(fv_votable);
    if (!fv_client) {
        return scnprintf(buf, PAGE_SIZE, "Unknown\n");
    }

    return scnprintf(buf, PAGE_SIZE, "%s\n", fv_client);
}
static CLASS_ATTR_RO(fv_voter);

static ssize_t awake_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *awake_votable;
    int awake_val = 0;

    awake_votable = find_votable("AWAKE");
    if (IS_ERR_OR_NULL(awake_votable)) {
        pr_err("%s failed to get awake_votable\n", __func__);
        return -EINVAL;
    }

    awake_val = get_effective_result_locked(awake_votable);

    return scnprintf(buf, PAGE_SIZE, "%d\n", awake_val);
}
static CLASS_ATTR_RO(awake);

static ssize_t awake_voter_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
    struct votable *awake_votable;
    const char *awake_client;

    awake_votable = find_votable("AWAKE");
    if (!awake_votable) {
        pr_err("%s failed to get awake_votable\n", __func__);
        return -EINVAL;
    }

    awake_client = get_effective_client_locked(awake_votable);
    if (!awake_client) {
        return scnprintf(buf, PAGE_SIZE, "Unknown\n");
    }

    return scnprintf(buf, PAGE_SIZE, "%s\n", awake_client);
}
static CLASS_ATTR_RO(awake_voter);

static ssize_t batt_manufacturer_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	int batt_id = 0;
	struct bq_fg_chip *gm;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		gm = power_supply_get_drvdata(psy_fg);
		if (!IS_ERR_OR_NULL(gm))
			batt_id = gm->cell_supplier;
		else
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

    return scnprintf(buf, PAGE_SIZE, "%s\n", batt_manufacturer_text[batt_id]);
}
static CLASS_ATTR_RO(batt_manufacturer);

static ssize_t battery_type_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	int batt_id = 0;
	struct bq_fg_chip *gm;
	struct power_supply *psy_fg = NULL;

	psy_fg = power_supply_get_by_name("bq28z610");
	if (!IS_ERR_OR_NULL(psy_fg)) {
		gm = power_supply_get_drvdata(psy_fg);
		if (!IS_ERR_OR_NULL(gm))
			batt_id = gm->cell_supplier;
		else
			pr_err("%s: get fg psy drv data failed\n", __func__);
	} else
		pr_err("%s: get fg psy failed\n", __func__);

    return scnprintf(buf, PAGE_SIZE, "%s\n", battery_type_text[batt_id]);
}
static CLASS_ATTR_RO(battery_type);

static ssize_t cp_manufacturer_show(struct class *c,
				struct class_attribute *attr, char *buf)
{
	int rc;
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };

	if (!lc_chg) {
		pr_err("Failed to get lc_chg\n");
		return 0;
        }
	rc = lc_chg_sysfs_get_iio_channel(lc_chg, LC_CP_MASTER, LC_CP_MASTER_SC_MANUFACTURER, &pval.intval);
	if (rc < 0) {
		pval.intval = 0;
		pr_err("Failed to get sc_manufacturer rc=%d\n", rc);
	}else{
		pval.intval = pval.intval +1;
	}

	return scnprintf(buf, PAGE_SIZE, "%s\n",
            pval.intval < ARRAY_SIZE(cp_manufacturer_text) ? cp_manufacturer_text[pval.intval] : "Unknown");
}
static CLASS_ATTR_RO(cp_manufacturer);

static ssize_t manufacturer_show(struct class *c, struct class_attribute *attr,
                            char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", "qcom-PM7250b");
}
static CLASS_ATTR_RO(manufacturer);

static ssize_t fg_vendor_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", "mpc8011b");
}
static CLASS_ATTR_RO(fg_vendor);

static ssize_t batt_model_name_show(struct class *cls,
    struct class_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", 7000*1000);
}
static CLASS_ATTR_RO(batt_model_name);
#if 0
static ssize_t quick_charge_type_show(struct class *c,
	struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_quick_charge_type(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get quick_charge_type fail\n", __func__);
		return ret;
	}

	lc_chg->quick_charge_type = pval.intval;
	pr_info("%s quick_charge_type: %d\n", __func__, lc_chg->quick_charge_type);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->quick_charge_type);
}
static CLASS_ATTR_RO(quick_charge_type);

static ssize_t soc_decimal_show(struct class *c,
	struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_soc_decimal(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get soc_decimal fail\n", __func__);
		return ret;
	}

	lc_chg->soc_decimal = pval.intval;
	pr_info("%s soc_decimal: %d\n", __func__, lc_chg->soc_decimal);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->soc_decimal);
}
static CLASS_ATTR_RO(soc_decimal);

static ssize_t soc_decimal_rate_show(struct class *c,
	struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_soc_decimal_rate(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get soc_decimal_rate fail\n", __func__);
		return ret;
	}

	lc_chg->soc_decimal_rate = pval.intval;
	pr_info("%s soc_decimal_rate: %d\n", __func__, lc_chg->soc_decimal_rate);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->soc_decimal_rate);
}
static CLASS_ATTR_RO(soc_decimal_rate);

static ssize_t shutdown_delay_show(struct class *c,
	struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_shutdown_delay(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get shutdown_delay fail\n", __func__);
		return ret;
	}

	lc_chg->shutdown_delay = pval.intval;
	//pr_info("%s shutdown_delay: %d\n", __func__, lc_chg->shutdown_delay);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->shutdown_delay);
}
static CLASS_ATTR_RO(shutdown_delay);
#endif

static ssize_t set_ship_mode_show(struct class *c,
                    struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

    ret = lc_get_ship_mode(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get ship_mode fail\n", __func__);
		return ret;
	}

	lc_chg->ship_mode = pval.intval;
	pr_info("%s ship_mode: %d\n", __func__, lc_chg->ship_mode);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->ship_mode);
}

static ssize_t set_ship_mode_store(struct class *c, struct class_attribute *attr,
                            const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->ship_mode = pval.intval;

    ret = lc_set_ship_mode(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get ship_mode fail\n", __func__);
		return ret;
	}

    pr_info("%s set ship_mode: %d %d\n", __func__, lc_chg->ship_mode, pval.intval);
    return count;
}
static CLASS_ATTR_RW(set_ship_mode);

static ssize_t shipmode_count_reset_show(struct class *c,
                    struct class_attribute *attr, char *buf)
{
    int val = 0;
    val = g_shipmode_en;

    pr_info("%s shipmode_count_reset_show: %d\n", __func__, g_shipmode_en);
    return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}

#define CQR_VBAT_MIM       3750000
#define CQR_VBAT_MAX       4300000
#define CQR_READ_VBAT_TIMES 5
#define CQR_VBAT_OUT_RANGE_CNT 2
static ssize_t shipmode_count_reset_store(struct class *c, struct class_attribute *attr,
                            const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    struct power_supply *batt_psy;
    int ret = 0;
    int i = 0;
    int shipmode_cnt = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    g_shipmode_en = pval.intval;
    lc_chg->use_shipcount_en = g_shipmode_en;

    pr_info("%s get_prop_style: %d %d\n", __func__, g_shipmode_en, pval.intval);
    batt_psy = power_supply_get_by_name("battery");
    if (batt_psy) {
		for (i = 0; i < CQR_READ_VBAT_TIMES; i++) {
			ret = power_supply_get_property(batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &pval);
			if (ret) {
				pr_err("get vbat fail\n");
				continue;
			}
			if (pval.intval < CQR_VBAT_MIM || pval.intval > CQR_VBAT_MAX) {
				shipmode_cnt++;
				pr_info("%s %d %d %d %d %d\n",
					__func__, g_shipmode_en, pval.intval, CQR_VBAT_MAX, CQR_VBAT_MIM, shipmode_cnt);
			}
		}
	}

    if (shipmode_cnt >= CQR_VBAT_OUT_RANGE_CNT) {
		g_shipmode_en = false;
		lc_chg->use_shipcount_en = g_shipmode_en;
		pr_info("%s after %d\n", __func__, g_shipmode_en);
		return -EINVAL;
	}

    return count;
}
static CLASS_ATTR_RW(shipmode_count_reset);

int use_count_reset(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval)
{
    struct power_supply *batt_psy;
    int ret = 0;
    int i = 0;
    int shipmode_cnt = 0;
    union power_supply_propval val = {0, };

    g_shipmode_en = pval->intval;
    lc_chg->use_shipcount_en = g_shipmode_en;

    pr_err("%s get_prop_style: %d %d\n", __func__, g_shipmode_en, pval->intval);
    batt_psy = power_supply_get_by_name("battery");
    if (batt_psy) {
		for (i = 0; i < CQR_READ_VBAT_TIMES; i++) {
			ret = power_supply_get_property(batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
			if (ret) {
				pr_err("get vbat fail\n");
				continue;
			}
			if (val.intval < CQR_VBAT_MIM || val.intval > CQR_VBAT_MAX) {
				shipmode_cnt++;
				pr_info("%s %d %d %d %d %d\n",
					__func__, g_shipmode_en, val.intval, CQR_VBAT_MAX, CQR_VBAT_MIM, shipmode_cnt);
			}
		}
	}

    if (shipmode_cnt >= CQR_VBAT_OUT_RANGE_CNT) {
		g_shipmode_en = false;
		lc_chg->use_shipcount_en = g_shipmode_en;
		pr_info("%s after %d\n", __func__, g_shipmode_en);
		return -EINVAL;
	}

	return 0;
}

static ssize_t mtbf_current_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_mtbf_current(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get mtbf_current fail\n", __func__);
		return ret;
	}

	lc_chg->mtbf_current = pval.intval;

	pr_info("%s mtbf_current: %d\n", __func__, lc_chg->mtbf_current);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->mtbf_current);
}

static ssize_t mtbf_current_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	if (kstrtoint(buf, 10, &pval.intval)) {
		pr_err("%s kstrtoint fail\n", __func__);
		return -EINVAL;
	}

	ret = lc_set_mtbf_current(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get mtbf_current fail\n", __func__);
		return ret;
	}

	pr_info("%s mtbf_current: %d %d\n", __func__, lc_chg->mtbf_current, pval.intval);
	return count;
}
static CLASS_ATTR_RW(mtbf_current);

static ssize_t apdo_max_show(struct class *c,
	struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs, lc_chg_sysfs_class);
	int ret = 0;
	union power_supply_propval pval = {0, };
	int apdo_curr = 0;
	int apdo_volt = 0;
	int apdo_max = 0;

	ret = lc_get_apdo_curr(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get apdo_curr fail\n", __func__);
		return ret;
	}
	apdo_curr = pval.intval;

	ret = lc_get_apdo_volt(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get apdo_curr fail\n", __func__);
		return ret;
	}
	apdo_volt = pval.intval;

	apdo_max = (apdo_curr * apdo_volt) / 1000000;

	pr_info("%s apdo_max: %d\n", __func__, apdo_max);
	return scnprintf(buf, PAGE_SIZE, "%d\n", apdo_max);
}
static CLASS_ATTR_RO(apdo_max);

static ssize_t typec_mode_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
	struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,lc_chg_sysfs_class);
	union power_supply_propval pval = {0, };
	int ret = 0;

	ret = lc_get_typec_mode(lc_chg, &pval);
	if (ret < 0) {
		pr_err("%s get ypec_mode fail\n", __func__);
		return ret;
	}

	lc_chg->typec_mode = pval.intval;

	pr_info("%s typec_mode: %d, %s\n", __func__, lc_chg->typec_mode, get_usbc_text_name(lc_chg->typec_mode));
	return scnprintf(buf, PAGE_SIZE, "%s\n", get_usbc_text_name(lc_chg->typec_mode));
}
static CLASS_ATTR_RO(typec_mode);

static ssize_t backlight_status_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);

    pr_info("%s backlight_status: %d\n", __func__, lc_chg->backlight_status);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->backlight_status);
}

static ssize_t backlight_status_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s point lc_chg is err or null\n", __func__);
        return -EINVAL;
    }

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->backlight_status = !!pval.intval;
    pr_info("%s backlight_status: %d\n", __func__, lc_chg->backlight_status);

    ret = lc_set_cid_status(lc_chg);
    if (ret < 0) {
        pr_err("%s set cid status fail\n", __func__);
        return -EINVAL;
    }

    return count;
}
static CLASS_ATTR_RW(backlight_status);

static ssize_t audio_cctog_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);

    pr_info("%s audio_cctog: %d\n", __func__, lc_chg->audio_cctog);
    return scnprintf(buf, PAGE_SIZE, "%d\n", lc_chg->audio_cctog);
}

static ssize_t audio_cctog_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int ret = -EINVAL;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s point lc_chg is err or null\n", __func__);
        return -EINVAL;
    }

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    lc_chg->audio_cctog = !!pval.intval;
    pr_info("%s audio_cctog: %d\n", __func__, lc_chg->audio_cctog);

    ret = lc_set_cid_status(lc_chg);
    if (ret < 0) {
        pr_err("%s set cid status fail, ret: %d\n", __func__, ret);
        return ret;
    }

    return count;
}
static CLASS_ATTR_RW(audio_cctog);

static ssize_t source_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_get_iio_channel(lc_chg, LC_CC, FORCE_SOURCE, &pval.intval);
    if (rc < 0) {
        pr_err("Couldn't get source");
    }

    pr_info("%s get val: %d\n", __func__, pval.intval);
    return scnprintf(buf, PAGE_SIZE, "0x%x\n", pval.intval);
}

static ssize_t source_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_set_iio_channel(lc_chg, LC_CC, FORCE_SOURCE, pval.intval);
    if (rc < 0) {
        pr_err("Couldn't set source");
    }

    pr_info("%s set val: %d successful\n", __func__, pval.intval);
    return count;
}
static CLASS_ATTR_RW(source);

static ssize_t lpd_charging_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, LPD_CHARGING, &pval.intval);
    if (rc < 0) {
        pr_err("Couldn't get lpd_charging");
    }

    pr_info("%s get val: %d\n", __func__, pval.intval);
    return scnprintf(buf, PAGE_SIZE, "%d\n", pval.intval);
}

static ssize_t lpd_charging_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_set_iio_channel(lc_chg, LC_MAIN, LPD_CHARGING, pval.intval);
    if (rc < 0) {
        pr_err("Couldn't set lpd_charging");
    }

    pr_info("%s set val: %d successful\n", __func__, pval.intval);
    return count;
}
static CLASS_ATTR_RW(lpd_charging);

static ssize_t lpd_control_show(struct class *c,
                struct class_attribute *attr, char *buf)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_get_iio_channel(lc_chg, LC_MAIN, LPD_CONTROL, &pval.intval);
    if (rc < 0) {
        pr_err("Couldn't get lpd_control");
    }

    pr_info("%s get val: %d\n", __func__, pval.intval);
    return scnprintf(buf, PAGE_SIZE, "%d\n", pval.intval);
}

static ssize_t lpd_control_store(struct class *c, struct class_attribute *attr,
                        const char *buf, size_t count)
{
    struct lc_chg_sysfs *lc_chg = container_of(c, struct lc_chg_sysfs,
                                        lc_chg_sysfs_class);
    union power_supply_propval pval = {0, };
    int rc = 0;

    if (kstrtoint(buf, 10, &pval.intval)) {
        pr_err("%s kstrtoint fail\n", __func__);
        return -EINVAL;
    }

    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("Failed to get lc_chg\n");
        return -EINVAL;
    }

    rc = lc_chg_sysfs_set_iio_channel(lc_chg, LC_MAIN, LPD_CONTROL, pval.intval);
    if (rc < 0) {
        pr_err("Couldn't set lpd_control");
    }

    pr_info("%s set val: %d successful\n", __func__, pval.intval);
    return count;
}
static CLASS_ATTR_RW(lpd_control);

/* ================= */
/*       Node        */
/* ================= */

static struct attribute *lc_chg_sysfs_attrs[] = {
    &class_attr_test.attr,
    &class_attr_typec_cc_orientation.attr,
    &class_attr_soc.attr,
    &class_attr_prop_style.attr,
    &class_attr_batt_manufacturer.attr,
    &class_attr_battery_type.attr,
    &class_attr_set_ship_mode.attr,
    &class_attr_shipmode_count_reset.attr,
    // battery secret ic info
    &class_attr_batt_id.attr,
    &class_attr_resistance_id.attr,
    &class_attr_auth_manufacturer.attr,
    &class_attr_chip_ok.attr,
    &class_attr_authentic.attr,
    // third charge pump
    &class_attr_cp_bus_current.attr,
    &class_attr_cp_manufacturer.attr,
    // charge ic
    &class_attr_manufacturer.attr,
    // vote node
    &class_attr_fcc.attr,
    &class_attr_fcc_voter.attr,
    &class_attr_icl.attr,
    &class_attr_icl_voter.attr,
    &class_attr_fv.attr,
    &class_attr_fv_voter.attr,
    &class_attr_awake.attr,
    &class_attr_awake_voter.attr,
    //other
#if 0
    &class_attr_quick_charge_type.attr,
    &class_attr_soc_decimal.attr,
    &class_attr_soc_decimal_rate.attr,
    &class_attr_shutdown_delay.attr,
#endif
    &class_attr_real_type.attr,
    &class_attr_input_suspend.attr,
    &class_attr_mtbf_current.attr,
    &class_attr_apdo_max.attr,
    &class_attr_typec_mode.attr,
    &class_attr_backlight_status.attr,
    &class_attr_audio_cctog.attr,
    &class_attr_source.attr,
    &class_attr_lpd_charging.attr,
    &class_attr_lpd_control.attr,
    &class_attr_fg_vendor.attr,
    &class_attr_batt_model_name.attr,
    NULL,
};

ATTRIBUTE_GROUPS(lc_chg_sysfs);

static int reboot_ship_mode(struct notifier_block *nb, unsigned long code, void *unused)
{
	int rc;
	if (g_shipmode_en == 1) {
		rc = smblib_enable_ship_mode(g_shipmode_en);
		if (rc < 0)
			pr_emerg("Failed to write ship mode: %d\n", rc);
	}
	return NOTIFY_DONE;
}

static int shipmode_register_notifier(void)
{
	int rc = 0;

	reboot_notifier.notifier_call = reboot_ship_mode;
	reboot_notifier.priority = 255;
	rc = register_reboot_notifier(&reboot_notifier);
	if (rc < 0) {
            pr_err("Couldn't register reboot notifier rc = %d\n", rc);
            return rc;
        }

        return 0;
}

static int lc_chg_sysfs_probe(struct platform_device *pdev)
{
    struct lc_chg_sysfs *lc_chg = NULL;
    struct iio_dev *indio_dev =  NULL;
    int ret = 0;

    pr_info("%s enter\n", __func__);

#ifndef USE_LC_CHG_SYSFS_IIO
    lc_chg = kzalloc(sizeof(*lc_chg), GFP_KERNEL);
    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s: kzalloc lc_chg fail\n", __func__);
        return -ENOMEM;
    }

    lc_chg->dev = &pdev->dev;
    lc_chg->pdev = pdev;
#else
    indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*lc_chg));
    if (!indio_dev) {
        pr_err("Failed to allocate memory\n");
        return -ENOMEM;
    }

    lc_chg = iio_priv(indio_dev);
    lc_chg->indio_dev = indio_dev;
    lc_chg->dev = &pdev->dev;
    lc_chg->pdev = pdev;

    ret = lc_chg_ext_init_iio_psy(lc_chg);
    if (ret < 0) {
        pr_err("%s: Failed to init ext iio ret = %d\n", __func__, ret);
        return ret;
    }

    ret = lc_chg_init_iio_psy(lc_chg);
    if (ret < 0) {
        pr_err("%s: Failed to init lc chg iio ret = %d\n", __func__, ret);
        return ret;
    }
#endif

    // init data
    lc_chg->test_flag = LC_CHG_TEST_FLAG;
    lc_chg->get_prop_style = LC_GET_PROP_QCOM_IIO;
    lc_chg->backlight_status = false;
    lc_chg->audio_cctog = false;
    lc_chg->cc_orientation = 0;
    lc_chg->cid_status = -EINVAL;

    lc_chg->lc_chg_sysfs_class.name = "lc_charger";
    lc_chg->lc_chg_sysfs_class.class_groups = lc_chg_sysfs_groups;
    lc_chg->lc_chg_sysfs_class.class_release = lc_chg_sysfs_release;
    ret = class_register(&lc_chg->lc_chg_sysfs_class);
	if (ret < 0) {
        pr_err("%s: Failed to create class_register ret = %d\n", __func__, ret);
        return ret;
    }

    platform_set_drvdata(pdev, lc_chg);

    ret = shipmode_register_notifier();
	if (ret < 0) {
		pr_err("Couldn't register reboot notifier rc = %d\n", ret);
		return ret;
	}

    pr_info("%s: successful\n", __func__);
    return 0;
}

static int lc_chg_sysfs_remove(struct platform_device *pdev)
{
    struct lc_chg_sysfs *lc_chg = platform_get_drvdata(pdev);

    pr_info("%s enter\n", __func__);
    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return -ENODEV;
    }
    sysfs_remove_groups(&lc_chg->dev->kobj, lc_chg_sysfs_groups);
    class_destroy(&lc_chg->lc_chg_sysfs_class);
	return 0;
}

static void lc_chg_sysfs_shutdown(struct platform_device *pdev)
{
    struct lc_chg_sysfs *lc_chg = platform_get_drvdata(pdev);

    pr_info("%s enter\n", __func__);
    if (IS_ERR_OR_NULL(lc_chg)) {
        pr_err("%s lc_chg is err or null\n", __func__);
        return;
    }
    return;
}

static const struct of_device_id lc_chg_sysfs_of_match[] = {
    {.compatible = "lc,chg_sysfs",},
    {},
};

MODULE_DEVICE_TABLE(of, lc_chg_sysfs_of_match);

static struct platform_driver lc_chg_sysfs_driver = {
    .driver = {
            .name = "lc_chg_sysfs",
            .owner = THIS_MODULE,
            .of_match_table = lc_chg_sysfs_of_match,
    },
    .probe = lc_chg_sysfs_probe,
    .remove = lc_chg_sysfs_remove,
    .shutdown = lc_chg_sysfs_shutdown,
};

static int __init lc_chg_sysfs_init(void)
{
    return platform_driver_register(&lc_chg_sysfs_driver);
}
late_initcall_sync(lc_chg_sysfs_init);

static void __exit lc_chg_sysfs_exit(void)
{
    platform_driver_unregister(&lc_chg_sysfs_driver);
}
module_exit(lc_chg_sysfs_exit);

MODULE_LICENSE("GPL");
