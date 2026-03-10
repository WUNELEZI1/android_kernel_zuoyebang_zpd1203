#include "lc_charger_sysfs.h"
#include "middleware/battery_secret_manager/inc/battery_secret_class.h"
#include "charger_partition.h"

static int last_usb_online;
static struct sysfs_desc *lc_chg_sysfs_get_desc(void)
{
	struct power_supply *usb_psy = NULL;
	static struct sysfs_desc *desc = NULL;

	if (IS_ERR_OR_NULL(desc)) {
		usb_psy = power_supply_get_by_name("usb");
		if (IS_ERR_OR_NULL(usb_psy)) {
			sysfs_err("%s: get the usb_psy failed \n", __func__);
			return NULL;
		}

		desc = power_supply_get_drvdata(usb_psy);
		if (IS_ERR_OR_NULL(desc)) {
			sysfs_err("%s: get the desc failed \n", __func__);
			return NULL;
		}
	}

	return desc;
}
static int get_uisoc_decimal_rate(struct sysfs_desc *desc, int *val)
{
	static int mtk_soc_decimal_rate[24] = {0,32,10,30,20,28,30,28,40,28,50,28,60,28,70,28,80,28,90,26,95,10,99,5};
	static int *dec_rate_seq = &mtk_soc_decimal_rate[0];
	static int dec_rate_len = 24;
	int i, soc = 0;

	for (i = 0; i < dec_rate_len; i += 2) {
		if (soc < dec_rate_seq[i]) {
			*val = dec_rate_seq[i - 1];
			return soc;
		}
	}
	*val = dec_rate_seq[dec_rate_len - 1];
	return soc;
}

static void get_uisoc_decimal(struct sysfs_desc *desc, int *val)
{
	int dec_rate, soc_dec, soc, hal_soc;
	static int last_val = 0, last_soc_dec = 0, last_hal_soc = 0;

	hal_soc = desc->gm->ui_soc ;

	soc_dec = desc->gm->fg_cust_data.ui_old_soc % 100;
	soc = get_uisoc_decimal_rate(desc, &dec_rate);

	if (soc_dec >= 0 && soc_dec < (50 - dec_rate))
		*val = soc_dec + 50;
	else if (soc_dec >= (50 - dec_rate) && soc_dec < 50)
		*val = soc_dec + 50 - dec_rate;
	else
		*val = soc_dec -50;
	if (last_hal_soc == hal_soc) {
		if ((last_val > *val && hal_soc != soc) || (last_soc_dec == soc_dec && hal_soc == soc)) {
			if (last_val > 50)
				*val = last_val + (100 - last_val - dec_rate) / 2;
			else
				*val = last_val + dec_rate / 4;
		} else if (last_val > *val) {
			*val = last_val;
		}
	}
	if (last_val != *val)
		last_val = *val;
	if (last_soc_dec != soc_dec)
		last_soc_dec = soc_dec;
	if (last_hal_soc != hal_soc)
		last_hal_soc = hal_soc;
}

static int soc_decimal_threadfn(void *param)
{
	int ret;
	int soc_decimal;
	int soc_decimal_rate;
	char soc_decimal_str[64];
	char soc_decimal_rate_str[64];
	char *envp[] = {soc_decimal_str,
					soc_decimal_rate_str, NULL};
	struct sysfs_desc *desc = (struct sysfs_desc *)param;

	while(!kthread_should_stop()) {
			wait_event_interruptible(desc->wq, desc->wakeup_thread);

			get_uisoc_decimal(desc, &soc_decimal);
			get_uisoc_decimal_rate(desc, &soc_decimal_rate);

			sprintf( envp[0],"POWER_SUPPLY_SOC_DECIMAL=%d", soc_decimal);
			sprintf( envp[1],"POWER_SUPPLY_SOC_DECIMAL_RATE=%d", soc_decimal_rate);

			ret = kobject_uevent_env(&desc->bat_psy->dev.kobj,
					KOBJ_CHANGE, envp);
			if (ret < 0){
					sysfs_err("send uevent fail");

					return -1;
			}

			msleep(100);
	}
	return 0;
}

static int lc_sysfs_get_shutdown_delay(struct sysfs_desc *desc)
{
#ifdef KERNEL_6
	struct mtk_battery_manager *bm;
#else
	struct mtk_battery *bm;
#endif
	static int count = 0;
	int usb_online;
	char sd_str[32];
	char *envp[] = { sd_str, NULL };
	static bool last_shutdown_delay;

	bm = desc->gm;
	if(IS_ERR_OR_NULL(desc->gm)){
		sysfs_err("%s: get the sysfs_desc failed \n", __func__);
		return -EINVAL;
	}

	usb_online = charger_dev_get_online(desc);
	if (bm->bs_data.bat_capacity < 0) {
		pr_err("%s: get batt capacity failed! \n", __func__);
		return -EINVAL;
	} else if (bm->bs_data.bat_capacity <= 1) {
		if (bm->bs_data.bat_batt_vol <= SHUTDOWN_DELAY_VOL_MAX && bm->bs_data.bat_batt_vol >= SHUTDOWN_DELAY_VOL_MIN) {
			if (!usb_online)
				desc->shutdown_delay = true;
			else if (desc->shutdown_delay)
				desc->shutdown_delay = false;
		} else {
			desc->shutdown_delay = false;
		}
	}

	if (desc->shutdown_delay != last_shutdown_delay){
		sprintf(envp[0], "POWER_SUPPLY_SHUTDOWN_DELAY=%d", desc->shutdown_delay);
		kobject_uevent_env(&desc->bat_psy->dev.kobj, KOBJ_CHANGE, envp);
		last_shutdown_delay = desc->shutdown_delay;
	}

	if (desc->shutdown_delay && count <= 60) {
		count ++;
		kobject_uevent_env(&desc->bat_psy->dev.kobj, KOBJ_CHANGE, envp);
	}

	if (!desc->shutdown_delay){
		count = 0;
	}
	sysfs_info("capacity(%d), vbat(%d), batt_status(%d), shutdown_delay(%d), count(%d) \n",
		bm->bs_data.bat_capacity, bm->bs_data.bat_batt_vol, bm->bs_data.bat_status, desc->shutdown_delay, count);

	return 0;
}

static ssize_t shutdown_delay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->shutdown_delay);
}

static struct device_attribute shutdown_delay_attr =
__ATTR(shutdown_delay, 0444, shutdown_delay_show, NULL);

static void wake_up_soc_decimal_task(struct sysfs_desc *desc)
{
	desc->wakeup_thread = true;
	wake_up_interruptible(&desc->wq);
}

int lc_sysfs_get_charge_type(struct sysfs_desc *desc, int *real_chr_type, int *chr_type)
{
	struct power_supply *chg_psy;
	union power_supply_propval val = {0};
	union power_supply_propval pval = {0};
	int ret = 0;
	int pd_type = 0;

	if (IS_ERR_OR_NULL(desc)){
		sysfs_err("%s: get the sysfs_desc failed \n",__func__);
		return -EINVAL;
	}

	chg_psy = desc->chg_psy;
	if (IS_ERR_OR_NULL(chg_psy)) {
		sysfs_err("%s Couldn't get chg_psy \n", __func__);
		chg_psy = power_supply_get_by_name("primary_chg");
		if (IS_ERR_OR_NULL(chg_psy)) {
			sysfs_err("%s Couldn't get chg_psy still fail\n", __func__);
			return -EINVAL;
		}
	}


	if (charger_dev_get_online(desc) == 0) {
		pval.intval = CHARGER_UNKNOWN;
		val.intval = POWER_SUPPLY_TYPE_UNKNOWN;
		sysfs_info("%s chg offline, set type unknown \n", __func__);
	} else if (desc->info->ta_status[PD] == TA_ATTACH) {
		pd_type = adapter_dev_get_property(desc->info->adapter_dev[PD],
						   CAP_TYPE);
		if (pd_type == MTK_PD_APDO) {
			*real_chr_type = PPS_CAHRGER;
		} else {
			*real_chr_type = PD_CAHRGER;
		}
	} else {
		ret = power_supply_get_property(desc->chg_psy,
						POWER_SUPPLY_PROP_TYPE, &val);
		if (ret) {
			sysfs_err("%s Couldn't get charge_type \n", __func__);
			return ret;
		}
		*chr_type = val.intval;
		switch (val.intval) {
		case POWER_SUPPLY_TYPE_UNKNOWN:
			*real_chr_type = CHARGER_UNKNOWN;
			break;
		case POWER_SUPPLY_TYPE_USB:
			ret = power_supply_get_property(
				desc->chg_psy, POWER_SUPPLY_PROP_USB_TYPE,
				&pval);
			if (ret) {
				sysfs_err("%s Couldn't get usb type \n",
					  __func__);
				return ret;
			} else {
				desc->info->usb_type = pval.intval;
			}
			if (desc->info->usb_type == POWER_SUPPLY_USB_TYPE_DCP) {
				*real_chr_type = NONSTANDARD_CHARGER;
			} else {
				*real_chr_type = STANDARD_HOST;
			}
			break;
		case POWER_SUPPLY_TYPE_USB_CDP:
			*real_chr_type = CHARGING_HOST;
			break;
		case POWER_SUPPLY_TYPE_USB_DCP:
			*real_chr_type = STANDARD_CHARGER;
			break;
		case POWER_SUPPLY_TYPE_USB_ACA:
			*real_chr_type = HVDCP_CHARGER;
			break;
		default:
			*real_chr_type = NONSTANDARD_CHARGER;
		}
	}

	if ((*real_chr_type != CHARGER_UNKNOWN) && desc->info->cmd_discharging) {
		sysfs_info("input suspend set input current 100mA\n");
		charger_dev_set_input_current(desc->info->chg1_dev, INPUT_SUSPEND_CURRENT);
	}

	desc->real_type = *real_chr_type;
	desc->chr_type = *chr_type;

	sysfs_info("%s: chr_type(%d), usb_type(%d), real_type(%d) \n", __func__, *chr_type, desc->info->usb_type, *real_chr_type);
	return 0;
}

static int quick_chr_type;
static int last_quick_chr_type;
static int lc_sysfs_get_quick_charge_type(struct sysfs_desc *desc,int *chr_type)
{
	int ret;
	char chr_type_str[64];
	char *envp[] = { chr_type_str, NULL };
	int pd_type = 0;
	int temp_flag = 0;

	if (IS_ERR_OR_NULL(desc)){
		sysfs_err("%s: get the sysfs_desc failed \n",__func__);
		return -EINVAL;
	}

	if (desc->info->battery_temp < 5 || desc->info->battery_temp > 48) {
		quick_chr_type = QUICK_CHARGE_NORMAL;
		temp_flag = 1;
	} else if (desc->info->ta_status[PD] == TA_ATTACH) {
		pd_type = adapter_dev_get_property(desc->info->adapter_dev[PD], CAP_TYPE);
		if (pd_type == MTK_PD_APDO) {
			quick_chr_type = QUICK_CHARGE_TURBE;
		} else {
			quick_chr_type = QUICK_CHARGE_FAST;
		}
	} else {
		switch (*chr_type) {
			case POWER_SUPPLY_TYPE_UNKNOWN:
			case POWER_SUPPLY_TYPE_USB:
			case POWER_SUPPLY_TYPE_USB_CDP:
			case POWER_SUPPLY_TYPE_USB_DCP:
				quick_chr_type = QUICK_CHARGE_NORMAL;
				break;
			case POWER_SUPPLY_TYPE_USB_ACA:
				quick_chr_type = QUICK_CHARGE_FAST;
				break;
			default:
				break;
			}
	}

	if (temp_flag != 1 && quick_chr_type == QUICK_CHARGE_NORMAL && *chr_type != POWER_SUPPLY_TYPE_USB_DCP) {
		desc->this_desc.type = POWER_SUPPLY_TYPE_USB;
	} else
		desc->this_desc.type = POWER_SUPPLY_TYPE_USB_DCP;

	sysfs_info("%s chr_type=%d, pd_type=%d, temp=%d, quick_chr_type:%d\n", __func__,
		*chr_type, pd_type, desc->info->battery_temp, quick_chr_type);

	if (quick_chr_type != last_quick_chr_type) {
		sysfs_info("%s chr_type(%d), batt_temp(%d), quick_charge_type(%d), last_quick_charge_type(%d) \n",
			__func__, *chr_type, desc->info->battery_temp, quick_chr_type, last_quick_chr_type);
		if (pd_type == MTK_PD_APDO)
			wake_up_soc_decimal_task(desc);
		else
			desc->wakeup_thread = false;

		sprintf(envp[0], "POWER_SUPPLY_QUICK_CHARGE_TYPE=%d", quick_chr_type);
		ret = kobject_uevent_env(&(desc->info->chg_psy->dev.kobj), KOBJ_CHANGE, envp);
		if (ret)
			sysfs_err("%s send uevent fail(%d)\n", __func__, ret);
		last_quick_chr_type = quick_chr_type;
	}

	return 0;

}

/*Fixed the issue where the quick_charge_type is 0 not 1 when Apple charger plugged-in start*/
static int psy_change_noti(struct notifier_block *nb,
	unsigned long event, void *v)
{
	struct power_supply *psy = v;
	struct sysfs_desc *desc;
	int ret = 0;
	int real_chr_type = CHARGER_UNKNOWN;
	int chr_type = POWER_SUPPLY_TYPE_UNKNOWN;

	desc = container_of(nb, struct sysfs_desc, psy_nb);
	if (event != PSY_EVENT_PROP_CHANGED || !desc || !psy)
		return NOTIFY_OK;

	if (strcmp(psy->desc->name, "usb") == 0)
		return NOTIFY_OK;

	sysfs_err("%s: psy name=%s, charge_type(%d, %d)\n",
		__func__, psy->desc->name, desc->real_type, desc->chr_type);

	ret = lc_sysfs_get_charge_type(desc, &real_chr_type, &chr_type);
	ret |= lc_sysfs_get_quick_charge_type(desc, &chr_type);
	ret |= lc_sysfs_get_shutdown_delay(desc);
	if(ret < 0){
		sysfs_err("%s: get shutdown_delay failed \n",__func__);
	}

	if (last_usb_online) {
		schedule_delayed_work(&desc->monitor_work, 0);
	} else {
		power_supply_changed(desc->usb_psy);
	}

	return 0;
}
/*Fixed the issue where the quick_charge_type is 0 not 1 when Apple charger plugged-in end*/

static ssize_t quick_charge_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	sysfs_info("%s quick_charge_type:%d\n", __func__, quick_chr_type);
	return sprintf(buf, "%d\n", quick_chr_type);
}

static struct device_attribute quick_charge_type_attr =
__ATTR(quick_charge_type, 0444, quick_charge_type_show, NULL);

static ssize_t real_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	static int real_type = 0, chr_type = 0;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (real_type != desc->real_type || chr_type != desc->chr_type) {
		sysfs_info("%s: real_type is %s ,chr_type:%d\n", __func__, real_type_name[desc->real_type], desc->chr_type);
		real_type = desc->real_type;
		chr_type = desc->chr_type;
	}
	return sprintf(buf, "%s\n", real_type_name[desc->real_type]);
}

static struct device_attribute real_type_attr =
__ATTR(real_type, 0444, real_type_show, NULL);

static ssize_t usb_otg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int res = 0;
#if IS_ENABLED(CONFIG_TCPC_CLASS)
	struct tcpc_device *tcpc;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	tcpc = desc->tcpc;

	if (tcpc->typec_attach_new == TYPEC_ATTACHED_SRC)
		res = 1;
#endif

	return sprintf(buf, "%d\n", res);
}

static struct device_attribute usb_otg_attr =
__ATTR(usb_otg, 0444, usb_otg_show, NULL);

int set_ship_mode(struct sysfs_desc *desc)
{
	int i, ret;
	int shipmode_cnt = 0;
	union power_supply_propval val;
	union charger_propval val1;

	if (!IS_ERR_OR_NULL(desc->bat_psy)) {
		for (i = 0; i < CQR_READ_VBAT_TIMES; i++) {
			ret = power_supply_get_property(desc->bat_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
			if (ret) {
				sysfs_err("get vbat fail\n");
				continue;
			}

			msleep(1);

			if (val.intval < CQR_VBAT_MIM || val.intval > CQR_VBAT_MAX) {
				shipmode_cnt++;
				sysfs_info("%s %d %d %d %d %d\n",
					__func__, desc->ship_mode, val.intval, CQR_VBAT_MAX, CQR_VBAT_MIM, shipmode_cnt);
			}
		}
	}

	if (shipmode_cnt >= CQR_VBAT_OUT_RANGE_CNT) {
		desc->ship_mode = false;
		sysfs_info("%s after %d\n", __func__, desc->ship_mode);
	}

	if (desc->ship_mode)
		val1.intval = 1;
	else
		val1.intval = 0;

	ret = charger_dev_set_property(desc->chg_dev, CHARGER_PROP_SHIP_MODE, &val1);
	if (ret) {
		sysfs_err("set ship mode fail\n");
		return ret;
	}

	return 0;
}

static ssize_t shipmode_count_reset_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->ship_mode);
}

static ssize_t shipmode_count_reset_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtobool(buf, &desc->ship_mode)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	sysfs_info("%s before %d\n", __func__, desc->ship_mode);

	ret = set_ship_mode(desc);
	if (ret) {
		sysfs_err("set ship mode fail\n");
		return ret;
	}

	sysfs_info("%s %d\n", __func__, desc->ship_mode);

	return count;
}

static struct device_attribute shipmode_count_reset_attr =
__ATTR(shipmode_count_reset, 0644, shipmode_count_reset_show, shipmode_count_reset_store);

static ssize_t set_ship_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->ship_mode);
}

static ssize_t set_ship_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtobool(buf, &desc->ship_mode)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	sysfs_info("%s before %d\n", __func__, desc->ship_mode);

	ret = set_ship_mode(desc);
	if (ret) {
		sysfs_err("set ship mode fail\n");
		return ret;
	}

	sysfs_info("%s %d\n", __func__, desc->ship_mode);

	return count;
}

static struct device_attribute set_ship_mode_attr =
__ATTR(set_ship_mode, 0644, set_ship_mode_show, set_ship_mode_store);

/* add ata_test_flag start */
static ssize_t ata_test_flag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->ata_test_flag);
}

static ssize_t ata_test_flag_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtobool(buf, &desc->ata_test_flag)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}
	sysfs_info("%s %d\n", __func__, desc->ata_test_flag);

	if (desc->ata_test_flag) {
		charger_dev_set_mivr(desc->info->chg1_dev, 4800000);
	} else {
		charger_dev_set_mivr(desc->info->chg1_dev, 4600000);
	}
	return count;
}

static struct device_attribute ata_test_flag_attr =
__ATTR(ata_test_flag, 0644, ata_test_flag_show, ata_test_flag_store);
/* add ata_test_flag end */

void set_input_suspend(struct sysfs_desc *desc, bool enable)
{
    struct mtk_charger *info = desc->info;

	if (enable == 1) {
		desc->info->cmd_discharging = true;
		charger_dev_enable(desc->info->chg1_dev, false);
		charger_dev_set_input_current(desc->info->chg1_dev, INPUT_SUSPEND_CURRENT);
		charger_dev_do_event(desc->info->chg1_dev,
			EVENT_DISCHARGE, 0);
	} else if (enable == 0) {
		desc->info->cmd_discharging = false;
		charger_dev_enable(desc->info->chg1_dev, true);
		charger_dev_do_event(desc->info->chg1_dev,
			EVENT_RECHARGE, 0);
	}
     if (enable) {
            charger_dev_enable_safety_timer(info->chg1_dev, 1);
     } else {
            charger_dev_enable_safety_timer(info->chg1_dev, 0);
            if (info->bat.uisoc < 50) {
                    charger_dev_dump_registers(info->chg1_dev);
                    charger_dev_dump_registers(info->dvchg1_dev);
            }
     }
}

static ssize_t input_suspend_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	bool enable = false;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	enable = desc->info->cmd_discharging;

	return sprintf(buf, "%d\n", enable);
}

static ssize_t input_suspend_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	bool enable = false;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtobool(buf, &enable)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	if (desc->info->cmd_discharging != enable) {
		set_input_suspend(desc, enable);

		sysfs_err("input_suspend_store = %d\n", desc->info->cmd_discharging);

		/* notifier to mtk battery */
		power_supply_changed(desc->info->psy1);
	}

	return count;
}
static struct device_attribute input_suspend_attr =
__ATTR(input_suspend, 0644, input_suspend_show, input_suspend_store);

static ssize_t batt_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 id = 0;
	int ret = 0;
	ret = lc_get_battery_id();
	if (ret < 0) {
		sysfs_err("%s get batt_id fail\n", __func__);
		id = 0xff;
	} else {
		sysfs_err("%s get batt_id succ\n", __func__);
		id = ret;
	}

	return sprintf(buf, "%d\n", id);
}

static struct device_attribute batt_id_attr =
__ATTR(batt_id, 0444, batt_id_show, NULL);

static ssize_t model_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", MODEL_NAME);
}

static struct device_attribute model_name_attr =
__ATTR(model_name, 0444, model_name_show, NULL);

static ssize_t battery_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 id = 0;
	int ret = 0;

	ret = lc_get_battery_id();
	if (ret < 0) {
		sysfs_err("%s get batt_id fail\n", __func__);
		id = 0xff;
	} else {
		sysfs_err("%s get batt_id succ\n", __func__);
		id = ret;
	}
	if (id < 1 || id > 3)
		return sprintf(buf, "%s\n", "Unknown");

	return sprintf(buf, "%s\n", batt_type_name[id]);
}
static struct device_attribute battery_type_attr =
__ATTR(battery_type, 0444, battery_type_show, NULL);

static ssize_t apdo_max_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

    int apdo_max = 0;

    apdo_max = desc->info->adapter_dev[0]->apdo_max;

	return sprintf(buf, "%d\n", apdo_max);// desc->info->pd_adapter->apdo_max / 1000000);
}
static struct device_attribute apdo_max_attr =
__ATTR(apdo_max, 0444, apdo_max_show, NULL);

static ssize_t pd_verifed_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	//struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", 0); //desc->info->pd_adapter->verifed);
}
static struct device_attribute pd_verifed_attr =
__ATTR(pd_verifed, 0444, pd_verifed_show, NULL);

static ssize_t fake_temp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}

	desc->gm->fake_temp = val;

	sysfs_info("set fake_temp = %d \n", val);

	/* notifier to mtk battery */
	power_supply_changed(desc->info->psy1);

	return count;
}

static ssize_t fake_temp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->gm->fake_temp);
}
static struct device_attribute fake_temp_attr =
__ATTR(fake_temp, 0644, fake_temp_show, fake_temp_store);

static ssize_t fake_cycle_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}

	if (val > 0) {
		desc->gm->fake_cycle = val;
		sysfs_info("set fake_cycle = %d \n", val);
	} else {
		sysfs_info("set fake_cycle failed, val=%d \n", val);
	}

	return count;
}

static ssize_t fake_cycle_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	return sprintf(buf, "%d\n", desc->gm->fake_cycle);
}
static struct device_attribute fake_cycle_attr =
__ATTR(fake_cycle, 0644, fake_cycle_show, fake_cycle_store);


static ssize_t RSOC_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rsoc;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	rsoc = desc->gm->soc;
	if (rsoc > 100) {
		rsoc = 100;
	} else if (rsoc < 0) {
		rsoc = 0;
	}
	sysfs_info("%s: %d\n", __func__, rsoc);
	return sprintf(buf, "%d\n", rsoc);
}

static struct device_attribute RSOC_attr =
__ATTR(RSOC, 0444, RSOC_show, NULL);

static ssize_t soh_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int soh;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	soh = lc_get_rawsoh();
	if (soh < 0) {
		soh = ((desc->gm->aging_factor + 99) / 100);
		pr_err("%s aging_factor=%d\n", __func__, desc->gm->aging_factor);
		if (soh > 100) {
			soh = 100;
		} else if (soh < 0) {
			soh = 0;
		}
	}
	sysfs_err("%s:soh: %d\n", __func__, soh);
	desc->gm->soh = soh;
	return sprintf(buf, "%d\n", soh);
}

static struct device_attribute soh_attr =
__ATTR(soh, 0444, soh_show, NULL);

static ssize_t soh_new_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int soh;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	soh = desc->gm->soh;

	sysfs_info("%s:soh new: %d\n", __func__, soh);

	return sprintf(buf, "%d\n", soh);
}
static struct device_attribute soh_new_attr =
__ATTR(soh_new, 0444, soh_new_show, NULL);

static ssize_t authentic_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int is_auth;

	is_auth = lc_is_battery_auth_success();
	if (is_auth)
		sysfs_err("%s get battery_auth succ\n", __func__);
	else
		sysfs_err("%s get battery_auth fail\n", __func__);
	return sprintf(buf, "%d\n", is_auth);
}
static struct device_attribute authentic_attr =
__ATTR(authentic, 0444, authentic_show, NULL);

static ssize_t soh_sn_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const char *soh_sn;

	soh_sn = lc_get_battery_sn();
	if (soh_sn) {
		sysfs_err("%s get sn succ\n", __func__);
		return sprintf(buf, "%s\n", soh_sn);
	} else {
		sysfs_err("%s get sn fail\n", __func__);
		return -EINVAL;
	}
}
static struct device_attribute soh_sn_attr =
__ATTR(soh_sn, 0444, soh_sn_show, NULL);

static ssize_t manufacturing_date_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	const char *manufacturing_date;

	manufacturing_date = lc_get_battery_manufacture_date();
	if (manufacturing_date) {
		sysfs_err("%s get battery_manufacture_date succ\n", __func__);
		return sprintf(buf, "%s\n", manufacturing_date);
	} else {
		sysfs_err("%s get battery_manufacture_date fail\n", __func__);
		manufacturing_date = "00000000";
		return sprintf(buf, "%s\n", manufacturing_date);
	}
}
static struct device_attribute manufacturing_date_attr =
__ATTR(manufacturing_date, 0444, manufacturing_date_show, NULL);

static ssize_t first_usage_date_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	const char *first_usage_time;
	char first_usage_date[9] = {'0','0','0','0','0','0','0','0','\0'};

	first_usage_time = lc_get_battery_first_use_time();
	if (!first_usage_time) {
		memcpy(first_usage_date, "99999999", 8);
		sysfs_err("%s get first_usage_date error\n", __func__);
		return sprintf(buf, "%s\n", first_usage_date);
	}
	memcpy(&first_usage_date[2], first_usage_time, 6);
	if (strncmp(&first_usage_date[2], "000000", 6)) //read date != 00000000, show date
		first_usage_date[0] = '2';
	return sprintf(buf, "%s\n", first_usage_date);
}

static ssize_t first_usage_date_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	char first_usage_date[7] = {0};

	sysfs_err("first_usage_date:%s\n", buf);
	memcpy(first_usage_date, &buf[2], 6);
	first_usage_date[6] = '\0';
	ret = lc_set_battery_first_use_time(first_usage_date);

	return count;
}

static struct device_attribute first_usage_date_attr =
		__ATTR(first_usage_date, 0644, first_usage_date_show, first_usage_date_store);

static ssize_t clear_cycle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	const char* key = "clrcls";
	int ret = 0;

	ret = strncmp(key, buf, 6);
	if (ret == 0) {
		ret = lc_clear_cycle_count();
			if (ret == 0) {
				sysfs_err("clear_cycle succ\n");
				return count;
			} else {
				sysfs_err("clear_cycle fail\n");
				return -EINVAL;
			}
	} else {
		sysfs_err("clear_cycle_key error=%d\n", ret);
		return -EINVAL;
	}
}
static struct device_attribute clear_cycle_attr =
		__ATTR(clear_cycle, 0644, NULL, clear_cycle_store);

static ssize_t ui_soh_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u8 ui_soh_data[16] = {0};
	int ret = 0;
	ret = lc_get_uisoh(ui_soh_data, 11);
	ret = snprintf(buf, PAGE_SIZE, "%d %d %d %d %d %d %d %d %d %d %d \n",
		ui_soh_data[0],ui_soh_data[1],ui_soh_data[2],ui_soh_data[3],ui_soh_data[4],ui_soh_data[5],
		ui_soh_data[6],ui_soh_data[7],ui_soh_data[8],ui_soh_data[9],ui_soh_data[10]);
	pr_err("%s: latest_ui_soh = %d \n", __func__, ui_soh_data[0]);
	return ret;
}

static ssize_t ui_soh_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	char t_data[70] = {0};
	char *pchar = NULL, *qchar = NULL;
	u8 ui_soh_data[40] = {0,};
	int ret = 0, i = 0;
	u8 val = 0;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();
	memset(t_data, 0, sizeof(t_data));
	strncpy(t_data, buf, count);
	pr_err("%s t_data : %s\n", __func__, t_data);
	qchar = t_data;
	while ((pchar = strsep(&qchar, " ")))
	{
		ret = kstrtou8(pchar, 10, &val);
		if (ret < 0) {
			pr_err("kstrtou8 error return %d \n", ret);
			return count;
		}
		ui_soh_data[i] = val;
		val = 0;
		pr_err("%s ui_soh_data[%d]: %d \n", __func__ ,i, ui_soh_data[i]);
		i++;
	}
	ret = lc_set_uisoh(ui_soh_data, 11);
	desc->gm->ui_soh = ui_soh_data[0];
	pr_err("%s: ui_soh = %d\n", __func__, desc->gm->ui_soh);
	return count;
}

static struct device_attribute ui_soh_attr =
		__ATTR(ui_soh, 0644, ui_soh_show, ui_soh_store);

static ssize_t charger_partition_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	static charger_partition_info_1 *info_1 = NULL;

	/* 分配缓冲区 */
    info_1 = kzalloc(CHARGER_CONFIG_SIZE, GFP_KERNEL);
    if (!info_1) {
        pr_err("Failed to allocate config buffer\n");
        return -EINVAL;
    }

    /* 读取分区数据 */
    ret = charger_partition_read(CHARGER_PARTITION_NAME, info_1, CHARGER_CONFIG_SIZE, (CHARGER_PARTITION_INFO_1 * CHARGER_PARTITION_RWSIZE));
    if (ret < 0) {
        pr_err("Failed to read charger config\n");
        return -EINVAL;
    }
	pr_err("[charger] %s ret: %d, info_1->power_off_mode: %u\n", __func__, ret, info_1->test);
	return sprintf(buf, "%d\n", info_1->test);
}

static ssize_t charger_partition_test_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	uint64_t offset;
	static charger_partition_info_1 *info_1 = NULL;
	info_1 = kzalloc(CHARGER_CONFIG_SIZE, GFP_KERNEL);
    if (!info_1) {
        pr_err("Failed to allocate config buffer\n");
        return -EINVAL;
    }

	if (kstrtoint(buf, 10, &info_1->test)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}
	offset = CHARGER_PARTITION_INFO_1 * CHARGER_PARTITION_RWSIZE;
	/* 写入分区数据 */
    ret = charger_partition_write(CHARGER_PARTITION_NAME, info_1, CHARGER_PARTITION_RWSIZE, offset);
    if (ret < 0) {
        pr_err("Failed to read charger config\n");
        return -EINVAL;
    }


	return count;
}
static struct device_attribute charger_partition_test_attr =
		__ATTR(charger_partition_test, 0644, charger_partition_test_show, charger_partition_test_store);

static ssize_t charger_partition_poweroffmode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	static charger_partition_info_1 *info_1 = NULL;

	/* 分配缓冲区 */
    info_1 = kzalloc(CHARGER_CONFIG_SIZE, GFP_KERNEL);
    if (!info_1) {
        pr_err("Failed to allocate config buffer\n");
        return -EINVAL;
    }

    /* 读取分区数据 */
    ret = charger_partition_read(CHARGER_PARTITION_NAME, info_1, CHARGER_CONFIG_SIZE, (CHARGER_PARTITION_INFO_1 * CHARGER_PARTITION_RWSIZE));
    if (ret < 0) {
        pr_err("Failed to read charger config\n");
        return -EINVAL;
    }
	pr_err("[charger] %s ret: %d, info_1->power_off_mode: %u\n", __func__, ret, info_1->power_off_mode);
	return sprintf(buf, "%d\n", info_1->power_off_mode);
}

static ssize_t charger_partition_poweroffmode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	uint64_t offset;
	charger_partition_info_1 *info_1 = kzalloc(CHARGER_CONFIG_SIZE, GFP_KERNEL);
	if (!info_1) {
		pr_err("Failed to allocate config buffer\n");
		return -EINVAL;
	}
	info_1->power_off_mode = 2;
	info_1->zero_speed_mode = 2;
	info_1->test = 0x34567890;
	info_1->reserved = 0;

	if (kstrtoint(buf, 10, &info_1->power_off_mode)) {
		sysfs_err("parsing number fail\n");
		ret = -EINVAL;
		goto out_free;
	}
	offset = CHARGER_PARTITION_INFO_1 * CHARGER_PARTITION_RWSIZE;
	/* 写入分区数据 */
	ret = charger_partition_write(CHARGER_PARTITION_NAME, info_1, CHARGER_PARTITION_RWSIZE, offset);
	if (ret < 0) {
		pr_err("Failed to read charger config\n");
		ret = -EINVAL;
		goto out_free;
	}
	ret  = count;
out_free:
	kfree(info_1);
	return ret;
}
static struct device_attribute charger_partition_poweroffmode_attr =
		__ATTR(charger_partition_poweroffmode, 0644, charger_partition_poweroffmode_show, charger_partition_poweroffmode_store);

static ssize_t is_eu_model_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int val = 0;

	charger_partition_get_eu_prop(&val);
	pr_err("%s:get eu_mode is %d\n", __func__, val);
	return sprintf(buf, "%d\n", val);
}

static ssize_t is_eu_model_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0;
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();
	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}
	pr_err("%s:set eu_mode is %d\n", __func__, val);
	charger_partition_set_eu_prop(val);
	desc->info->is_eu_model = val;
	desc->gm->is_eu_model = val;
	return count;
}
static struct device_attribute is_eu_model_attr =
		__ATTR(is_eu_model, 0644, is_eu_model_show, is_eu_model_store);


static struct attribute *bms_psy_attrs[] = {
	&soh_attr.attr,
	&soh_new_attr.attr,
	&authentic_attr.attr,
	NULL,
};

static struct attribute *usb_psy_attrs[] = {
	&quick_charge_type_attr.attr,
	&real_type_attr.attr,
	&usb_otg_attr.attr,
	&apdo_max_attr.attr,
	&pd_verifed_attr.attr,
	&ata_test_flag_attr.attr,
	NULL,
};

static struct attribute *bat_psy_attrs[] = {
	&shutdown_delay_attr.attr,
	&input_suspend_attr.attr,
	&batt_id_attr.attr,
	&shipmode_count_reset_attr.attr,
	&set_ship_mode_attr.attr,
	&fake_cycle_attr.attr,
	&fake_temp_attr.attr,
	&battery_type_attr.attr,
	&model_name_attr.attr,
	&RSOC_attr.attr,
	&soh_sn_attr.attr,
	&manufacturing_date_attr.attr,
	&first_usage_date_attr.attr,
	&clear_cycle_attr.attr,
	&charger_partition_test_attr.attr,
	&charger_partition_poweroffmode_attr.attr,
	&soh_attr.attr,
	&authentic_attr.attr,
	&ui_soh_attr.attr,
        &is_eu_model_attr.attr,
	NULL,
};

static const struct attribute_group usb_psy_group = {
	.attrs = usb_psy_attrs,
};

static const struct attribute_group bat_psy_group = {
	.attrs = bat_psy_attrs,
};

static const struct attribute_group bms_psy_group = {
	.attrs = bms_psy_attrs,
};

static int sysfs_setup_files(struct sysfs_desc *desc)
{
	int ret;

	if (!desc->usb_psy || !desc->bat_psy || !desc->info) {
		sysfs_err("%s find psy fail\n", __func__);
		ret = -EINVAL;
		goto _out;
	}

	ret = sysfs_create_group(&(desc->bat_psy->dev.kobj),
		 &bat_psy_group);
	if (ret) {
		sysfs_err("%s create battery node fail(%d)\n",
			  __func__, ret);
		goto _out;
	}

	ret = sysfs_create_group(&(desc->usb_psy->dev.kobj),
		&usb_psy_group);
	if (ret) {
		sysfs_err("%s create usb node fail(%d)\n", __func__, ret);
		goto _out;
	}

	ret = sysfs_create_group(&(desc->bms_psy->dev.kobj),
				 &bms_psy_group);
	if (ret) {
		sysfs_err("%s create bms node fail(%d)\n", __func__, ret);
		goto _out;
	}

	return 0;

_out:
	return ret;
}

int charger_dev_get_online(struct sysfs_desc *desc)
{
	struct power_supply *chg_psy;
	union power_supply_propval val = {0};
	int ret;

	if (IS_ERR_OR_NULL(desc) || IS_ERR_OR_NULL(desc->chg_psy)){
		sysfs_err("%s Couldn't get desc \n", __func__);
		return -EINVAL;
	}

	chg_psy = desc->chg_psy;

	ret = power_supply_get_property(chg_psy, POWER_SUPPLY_PROP_ONLINE, &val);
	if (ret){
		sysfs_err("%s Couldn't get usb_online \n", __func__);
		return ret;
	}

	if (last_usb_online != val.intval) {
		sysfs_err("%s usb_online is %d, last_online is %d \n", __func__, val.intval, last_usb_online);
		power_supply_changed(chg_psy);
		last_usb_online = val.intval;
	}

	return val.intval;
}

static int get_usb_vbus(struct sysfs_desc *desc)
{
	int vbus = 0;
	int val = 0;
	int ret = 0;

	if (IS_ERR_OR_NULL(desc) || IS_ERR_OR_NULL(desc->cp_dev)){
		sysfs_err("%s Couldn't get desc \n", __func__);
		return 0;
	}

	ret = charger_dev_get_adc(desc->cp_dev, ADC_CHANNEL_VBUS, &val, &val);
	if (ret)
		sysfs_err("%s Couldn't get vbus\n", __func__);
	else
		vbus = val / 1000;

	return vbus;
}

static enum power_supply_property usb_psy_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static int psy_usb_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct sysfs_desc *desc = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = charger_dev_get_online(desc);
		break;
	case POWER_SUPPLY_PROP_TYPE:
		val->intval = desc->this_desc.type;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = get_usb_vbus(desc);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int usb_psy_register(struct sysfs_desc *desc)
{
	struct power_supply_config psy_cfg = {
		.drv_data = desc,
	};

	desc->this_desc.name = "usb";
	desc->this_desc.type = POWER_SUPPLY_TYPE_USB;
	desc->this_desc.properties = usb_psy_properties;
	desc->this_desc.num_properties = ARRAY_SIZE(usb_psy_properties);
	desc->this_desc.get_property = psy_usb_get_property;

	desc->usb_psy = devm_power_supply_register(
		&(desc->info->pdev->dev), &desc->this_desc, &psy_cfg);
	if (IS_ERR(desc->usb_psy)) {
		pr_err("failed to register charger_psy\n");
		return PTR_ERR(desc->usb_psy);
	}

	pr_err("%s power supply register successfully\n",
	       desc->this_desc.name);

	return 0;
}

static enum power_supply_property bms_psy_properties[] = {
	POWER_SUPPLY_PROP_TYPE,
	POWER_SUPPLY_PROP_TEMP,
};

static int psy_bms_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	switch (psp) {
	case POWER_SUPPLY_PROP_TYPE:
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = desc->board_temp;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int psy_bms_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	struct sysfs_desc *desc = lc_chg_sysfs_get_desc();

	switch (psp) {
	case POWER_SUPPLY_PROP_TEMP:
		desc->board_temp = val->intval;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

struct power_supply_desc bms_psy_desc = {
	.name = "bms",
	.properties = bms_psy_properties,
	.num_properties = ARRAY_SIZE(bms_psy_properties),
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.get_property = psy_bms_get_property,
	.set_property = psy_bms_set_property,
};

static int init_psy_bms(struct sysfs_desc *desc)
{
	struct power_supply_config cfg = {
		.drv_data = desc,
	};

	desc->bms_psy = power_supply_register(&(desc->info->pdev->dev),
		&bms_psy_desc, &cfg);
	if (IS_ERR(desc->bms_psy)) {
		sysfs_err("%s register bms psy fail(%ld)\n", __func__,
			PTR_ERR(desc->bms_psy));
		return -PTR_ERR(desc->bms_psy);
	}
	return 0;
}

static int init_psy_tcpc(struct sysfs_desc *desc)
{
	//struct power_supply_config cfg = {
		//.drv_data = desc,
	//};

	//struct mtk_battery_manager *bm;

	usb_psy_register(desc);
	if (IS_ERR_OR_NULL(desc->usb_psy)) {
		sysfs_err("%s register usb psy fail(%ld)\n", __func__,
			PTR_ERR(desc->usb_psy));
		return -EINVAL;
	}

	desc->bat_psy = power_supply_get_by_name("battery");
	if (IS_ERR_OR_NULL(desc->bat_psy)) {
		sysfs_err("%s get battery psy fail\n", __func__);
		return -EINVAL;
	}

	desc->cp_dev = get_charger_by_name("primary_dvchg");
	if (IS_ERR_OR_NULL(desc->cp_dev)) {
		sysfs_err("%s get cp psy fail\n", __func__);
		//return -EINVAL;
	}

	desc->batt_verify_psy = power_supply_get_by_name("batt_verify");
	if (IS_ERR_OR_NULL(desc->batt_verify_psy)) {
		sysfs_err("%s get batt verify psy fail\n", __func__);
		//return -EINVAL;
	}

	desc->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (IS_ERR_OR_NULL(desc->tcpc)) {
		sysfs_err("%s get typec device fail\n", __func__);
		return -EINVAL;
	}

	desc->chg_dev = get_charger_by_name("primary_chg");
	if (IS_ERR_OR_NULL(desc->chg_dev)) {
		sysfs_err("%s get charger dev fail\n", __func__);
		return -EINVAL;
	}

#ifdef KERNEL_6
	desc->bm = power_supply_get_drvdata(desc->bat_psy);
	if (IS_ERR_OR_NULL(desc->bm)) {
		sysfs_err("%s get battery info fail\n", __func__);
		return -EINVAL;
	}
	desc->gm = bm->gm1;
	desc->bm = bm;
#else
	desc->gm = power_supply_get_drvdata(desc->bat_psy);
	if (IS_ERR_OR_NULL(desc->gm)) {
		sysfs_err("%s get battery info fail\n", __func__);
		return -EINVAL;
	}
#endif

	return 0;
}

static void monitor_work(struct work_struct *work)
{
	int real_type = 0, chg_type = 0;
	struct sysfs_desc *desc = container_of(work, struct sysfs_desc, monitor_work.work);

	if (desc->real_type == PD_CAHRGER || desc->real_type == CHARGER_UNKNOWN) {//notify to batterysecret
		power_supply_changed(desc->usb_psy);
		lc_sysfs_get_charge_type(desc, &real_type, &chg_type);
	}

	if (last_usb_online) {
		schedule_delayed_work(&desc->monitor_work, msecs_to_jiffies(2000));
	} else { //notify to batterysecret
		if (!IS_ERR_OR_NULL(desc->usb_psy))
			power_supply_changed(desc->usb_psy);
	}
}

/*static int __init lc_charger_sysfs_init(void) */
static int lc_charger_sysfs_probe(struct platform_device *pdev)
{
	int ret;
	struct sysfs_desc *desc;
	struct power_supply *main_psy;
	struct power_supply *chg_psy;
	static int probe_cnt = 0;

	probe_cnt++;

	sysfs_info("%s: starts, probe_cnt = %d\n", __func__, probe_cnt);

	chg_psy = power_supply_get_by_name("primary_chg");
	if (IS_ERR_OR_NULL(chg_psy)) {
		sysfs_err("%s get primary_chg psy fail\n", __func__);
		return -EPROBE_DEFER;
	}

	main_psy = power_supply_get_by_name("mtk-master-charger");
	if (!main_psy) {
		sysfs_err("%s get main charger psy fail\n", __func__);
		return -EPROBE_DEFER;
	}

	desc = kzalloc(sizeof(struct sysfs_desc), GFP_KERNEL);
	if (!desc) {
		sysfs_err("%s alloc desc mem fail\n", __func__);
		ret = -ENOMEM;
		goto alloc_err;
	}

	desc->info = power_supply_get_drvdata(main_psy);
	desc->chg_psy = chg_psy;

	ret = init_psy_tcpc(desc);
	if (ret){
		goto psy_tcpc_err;
	}

	ret = init_psy_bms(desc);
	if (ret)
		goto psy_bms_err;

	ret = sysfs_setup_files(desc);
	if (ret){
		goto sysfs_setup_err;
	}

	lc_charger_node_init(desc);

	desc->psy_nb.notifier_call = psy_change_noti;
	ret = power_supply_reg_notifier(&desc->psy_nb);

	desc->disp_nb.notifier_call = screen_noti_for_cid;
	ret |= mtk_disp_notifier_register("SW_CID", &desc->disp_nb);
	if (ret < 0) {
		sysfs_err("%s register psy notifier or disp notifier fail(%d)\n", __func__, ret);
		ret = -EINVAL;
		goto main_psy_err;
	}

	desc->wakeup_thread = false;
	init_waitqueue_head(&desc->wq);
	desc->soc_decimal_task = kthread_run(soc_decimal_threadfn, desc,
							"soc_decimal_task");
	if (IS_ERR(desc->soc_decimal_task)) {
			ret = -PTR_ERR(desc->soc_decimal_task);
			sysfs_err("%s run task fail(%d)\n", __func__, ret);
	}
	INIT_DELAYED_WORK(&desc->monitor_work, monitor_work);

	g_desc = desc;
#if IS_ENABLED(CONFIG_FACTORY_BUILD)
	desc->cid_enable = 0;
#else
	desc->cid_enable = 1;
#endif

	sysfs_err("%s init success !\n", __func__);
	charger_partition_init();
	return 0;


main_psy_err:
sysfs_setup_err:
psy_bms_err:
	power_supply_unregister(desc->bms_psy);
psy_tcpc_err:
	power_supply_unregister(desc->usb_psy);
	kfree(desc);
alloc_err:
	sysfs_err("%s init failed !\n", __func__);
	return ret;
}

static int lc_charger_sysfs_remove(struct platform_device *dev)
{
	charger_partition_exit();
	return 0;
}

static void lc_charger_sysfs_shutdown(struct platform_device *dev)
{
}

static const struct of_device_id lc_charger_sysfs_of_match[] = {
	{.compatible = "lc,charger_sysfs",},
	{},
};

MODULE_DEVICE_TABLE(of, lc_charger_sysfs_of_match);

static struct platform_driver lc_charger_sysfs_driver = {
	.probe = lc_charger_sysfs_probe,
	.remove = lc_charger_sysfs_remove,
	.shutdown = lc_charger_sysfs_shutdown,
	.driver = {
		   .name = "charger_sysfs",
		   .of_match_table = lc_charger_sysfs_of_match,
	},
};
static int __init lc_charger_sysfs_init(void)
{
	return platform_driver_register(&lc_charger_sysfs_driver);
}
late_initcall_sync(lc_charger_sysfs_init);

static void __exit lc_charger_sysfs_exit(void)
{
	platform_driver_unregister(&lc_charger_sysfs_driver);
}
module_exit(lc_charger_sysfs_exit);

MODULE_LICENSE("GPL");
