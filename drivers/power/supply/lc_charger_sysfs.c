#include "lc_charger_sysfs.h"
#include "middleware/battery_secret_manager/inc/battery_secret_class.h"

/*add SW-CID start*/
static int set_drp_mode_for_cid(struct sysfs_desc *desc);
int screen_noti_for_cid(struct notifier_block *nb, unsigned long event, void *data)
{
	int *blank = (int *)data;
	struct sysfs_desc *desc;
	int ret;

	desc = container_of(nb, struct sysfs_desc, disp_nb);
	if (!desc || !blank){
		return NOTIFY_OK;
	}

	if (!desc->cid_enable) {
		return 0;
	}

	if (event == MTK_DISP_EARLY_EVENT_BLANK) {
		if (*blank == MTK_DISP_BLANK_POWERDOWN) {
			sysfs_info("%s: screen off\n", __func__);
			desc->screen_on = false;
		}
	} else if (event == MTK_DISP_EVENT_BLANK) {
		if(*blank == MTK_DISP_BLANK_UNBLANK) {
			sysfs_info("%s: screen on\n", __func__);
			desc->screen_on = true;
		}
	}

	sysfs_info("%s: audio_on:%d, screen_on:%d \n", __func__, desc->audio_on, desc->screen_on);
	if (!desc->audio_on){
		ret = set_drp_mode_for_cid(desc);
		if(ret < 0){
			sysfs_err("%s: set drp_mode_for_cid failed, ret = %d \n",__func__, ret);
			return -EINVAL;
		}
	}

	return 0;
}

static int get_status_for_cid(struct sysfs_desc *desc, bool *cc_status)
{
	int ret = 0;

	desc->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (IS_ERR_OR_NULL(desc->tcpc)) {
		sysfs_err("%s: get tcpc device fail\n",__func__);
		return -ENODEV;
	}
	sysfs_info("%s: audio_on:%d, screen_on:%d \n", __func__, desc->audio_on, desc->screen_on);
	if (desc->screen_on || desc->audio_on)
		desc->tcpc->screen_and_audio_off = false;
	else
		desc->tcpc->screen_and_audio_off = true;

	ret = tcpm_inquire_typec_attach_state(desc->tcpc);
	if (ret < 0) {
		sysfs_err("%s: get typec_attach_state fail\n",__func__);
		return -EINVAL;
	} else if (ret == TYPEC_UNATTACHED) {
		*cc_status = false;
		sysfs_info("%s: typec Unattached !\n", __func__);
	} else {
		*cc_status = true;
		sysfs_info("%s: typec Attached !\n", __func__);
	}

	return 0;
}

static int set_drp_mode_for_cid(struct sysfs_desc *desc)
{
	int ret = 0;
	uint8_t typec_role = TYPEC_ROLE_UNKNOWN;

	if (!desc->cid_enable){
		pr_err("%s: sw_cid is not enbaled! \n", __func__);
		return 0;
	}

	ret = get_status_for_cid(desc, &desc->typec_attached);
	if (ret < 0){
		sysfs_err("%s:get_status_for_cid failed \n",__func__);
		return 0;
	}
	sysfs_info("%s: typec_attached:%d, screen_and_audio_off:%d \n",__func__, desc->typec_attached, desc->tcpc->screen_and_audio_off);
	if (!desc->typec_attached) {
		if (desc->tcpc->screen_and_audio_off == false)
			typec_role = TYPEC_ROLE_TRY_SNK;
		else
			typec_role = TYPEC_ROLE_SNK;

		ret = tcpm_typec_change_role_postpone(desc->tcpc, typec_role, true);
		if(ret < 0) {
			sysfs_err("%s set typec_role failed\n", __func__);
			return -EINVAL;
		}
	}
	sysfs_info("%s: typec_attached:%d, screen_and_audio_off:%d, typec_role:%d \n",__func__, desc->typec_attached, desc->tcpc->screen_and_audio_off, typec_role);
	return 0;
}
/*add SW-CID end*/

#ifdef KERNEL_6
static ssize_t real_type_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t real_type_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	static int real_type = 0, chr_type = 0;
	struct sysfs_desc *desc = container_of(class, struct sysfs_desc, lc_charger_class);

	if (real_type != desc->real_type || chr_type != desc->chr_type) {
		sysfs_info("%s: real_type is %s ,chr_type:%d\n", __func__, real_type_name[desc->real_type], desc->chr_type);
		real_type = desc->real_type;
		chr_type = desc->chr_type;
	}

	return sprintf(buf, "%s\n", real_type_name[desc->real_type]);
}
static CLASS_ATTR_RO(real_type);

#if IS_ENABLED(CONFIG_TCPC_CLASS)
static int typec_cc_orientation_handler(struct tcpc_device *tcpc)
{
	int typec_cc_orientation = 0;

	tcpci_get_cc(tcpc);

	if (typec_get_cc1() == 0 && typec_get_cc2() == 0)
		typec_cc_orientation = 0;
	else if (typec_get_cc2() == 0)
		typec_cc_orientation = 1;
	else if (typec_get_cc1() == 0)
		typec_cc_orientation = 2;
	sysfs_info("%s: typec_cc_orientation:%d, cc1:%d, cc2:%d \n", __func__, typec_cc_orientation, typec_get_cc1(), typec_get_cc2());
	return typec_cc_orientation;
}

#ifdef KERNEL_6
static ssize_t typec_cc_orientation_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t typec_cc_orientation_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", typec_cc_orientation_handler(desc->tcpc));
}
static CLASS_ATTR_RO(typec_cc_orientation);

#ifdef KERNEL_6
static ssize_t typec_mode_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t typec_mode_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct tcpc_device *tcpc;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	tcpc = desc->tcpc;
	if (tcpc->typec_attach_new > ARRAY_SIZE(typec_mode_name))
		return sprintf(buf, "%s\n", "Unknown");
	return sprintf(buf, "%s\n", typec_mode_name[tcpc->typec_attach_new]);
}
static CLASS_ATTR_RO(typec_mode);
#endif /* CONFIG_TCPC_CLASS */

#ifdef KERNEL_6
static ssize_t chip_ok_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t chip_ok_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	int ret = 0;
	bool chip_ok_status = false;
	union power_supply_propval val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (IS_ERR_OR_NULL(desc->batt_verify_psy)) {
		desc->batt_verify_psy = power_supply_get_by_name("batt_verify");
	}
	if (desc->batt_verify_psy) {
		ret = power_supply_get_property(desc->batt_verify_psy, POWER_SUPPLY_PROP_AUTHENTIC, &val);
		if (ret < 0)
			chip_ok_status = false;

		if (val.intval)
			chip_ok_status = true;
		else
			chip_ok_status = false;
	} else {
		chip_ok_status = false;
	}

	return sprintf(buf, "%d\n", chip_ok_status);
}
#ifdef KERNEL_6
static ssize_t chip_ok_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t chip_ok_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int ret = 0;
	int chip_ok_status;
	union power_supply_propval val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (IS_ERR_OR_NULL(desc->batt_verify_psy)) {
		desc->batt_verify_psy = power_supply_get_by_name("batt_verify");
	}

	if (kstrtoint(buf, 10, &chip_ok_status)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}
	val.intval = chip_ok_status;
	if (desc->batt_verify_psy) {
		ret = power_supply_set_property(desc->batt_verify_psy, POWER_SUPPLY_PROP_AUTHENTIC, &val);
		if (ret < 0)
			sysfs_err("set chip status fail by set_property\n");
	} else {
		sysfs_err("set chip status fail by batt_verify_psy\n");
	}
	sysfs_info("set chip_ok = %d\n", chip_ok_status);
	return count;
}
static CLASS_ATTR_RW(chip_ok);

#define HAS_BATTERY_SECRET_IC 1
#ifdef KERNEL_6
static ssize_t resistance_id_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t resistance_id_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	u8 id = 0;
	int ret = 0;
	union power_supply_propval val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (IS_ERR_OR_NULL(desc->batt_verify_psy)) {
		desc->batt_verify_psy = power_supply_get_by_name("batt_verify");
	}
	if (desc->batt_verify_psy) {
		ret = power_supply_get_property(desc->batt_verify_psy, POWER_SUPPLY_PROP_SCOPE, &val);
		if (ret) {
			id = 0;
		}
		id = val.intval;
	} else {
		id = 0;
	}
#ifdef HAS_BATTERY_SECRET_IC
	return sprintf(buf, "%s\n", "500000");
#endif

	return sprintf(buf, "%s\n",resistance_id_name[id]);
}
static CLASS_ATTR_RO(resistance_id);

#ifdef KERNEL_6
static ssize_t cp_manufacturer_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t cp_manufacturer_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
#if 0
	int ret = 0;
	union power_supply_propval val;
	struct sysfs_desc *desc = container_of(dev,
		struct sysfs_desc, dev);

	if (!IS_ERR_OR_NULL(desc->cp_dev)) {
		ret = power_supply_get_property(desc->cp_dev, POWER_SUPPLY_PROP_MODEL_NAME, &val);
		if (ret)
			sysfs_err("get cp name fail, ret=%d \n", ret);
	}
#endif
	return sprintf(buf, "%s\n", "Unknown");
}
static CLASS_ATTR_RO(cp_manufacturer);

#ifdef KERNEL_6
static ssize_t sw_manufacturer_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t sw_manufacturer_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);
	struct charger_properties *props = &desc->chg_dev->props;

	return sprintf(buf, "%s\n", props->alias_name);
}
static CLASS_ATTR_RO(sw_manufacturer);

#ifdef KERNEL_6
static ssize_t batt_manufacturer_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t batt_manufacturer_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	u8 id = 0;
	int ret = 0;

	ret = lc_get_battery_id();
	if (ret < 0)
		id = 0;
	else
		id = ret;

    return scnprintf(buf, PAGE_SIZE, "%s\n", batt_manufacturer_name[id]);
}
static CLASS_ATTR_RO(batt_manufacturer);

#ifdef KERNEL_6
static ssize_t cp_bus_voltage_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t cp_bus_voltage_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif

{
	int ret = 0;
	int vbus_total = 0;
	int val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (!IS_ERR_OR_NULL(desc->cp_dev)) {
		ret = charger_dev_get_adc(desc->cp_dev, ADC_CHANNEL_VBUS, &val, &val); //mv
		if (ret)
			sysfs_err("get current fail, ret=%d \n", ret);
		vbus_total = val / 1000;
	} else {
		vbus_total = 0;
	}

	return sprintf(buf, "%d\n", vbus_total);
}
static CLASS_ATTR_RO(cp_bus_voltage);

#ifdef KERNEL_6
static ssize_t cp_bus_current_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t cp_bus_current_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	int ret = 0;
	int cp_ibus = 0;
	int val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (!IS_ERR_OR_NULL(desc->cp_dev)) {
		ret = charger_dev_get_adc(desc->cp_dev, ADC_CHANNEL_IBUS, &val, &val); //ma
		if (ret) {
			sysfs_err("get current fail, ret=%d \n", ret);
		} else {
			cp_ibus = val/1000;
		}
	}
	return sprintf(buf, "%d\n", cp_ibus);
}
static CLASS_ATTR_RO(cp_bus_current);

#ifdef KERNEL_6
static ssize_t mtbf_current_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t mtbf_current_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}

	desc->info->mtbf_current = val;

    if (desc->info->mtbf_current) {
            charger_dev_enable_safety_timer(desc->info->chg1_dev, 0);
    } else {
            charger_dev_enable_safety_timer(desc->info->chg1_dev, 1);
    }

	sysfs_info("set mtbf_current = %d \n", val);

	return count;
}

#ifdef KERNEL_6
static ssize_t mtbf_current_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t mtbf_current_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{

	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->info->mtbf_current);
}
static CLASS_ATTR_RW(mtbf_current);

#ifdef KERNEL_6
static ssize_t input_suspend_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t input_suspend_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	bool enable = false;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	enable = desc->info->cmd_discharging;

	return sprintf(buf, "%d\n", enable);
}
#ifdef KERNEL_6
static ssize_t fake_batt_full_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t fake_batt_full_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int val;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}

	desc->info->fake_batt_full = val;

	sysfs_info("set fake_batt_full = %d \n", val);

	return count;
}

#ifdef KERNEL_6
static ssize_t fake_batt_full_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t fake_batt_full_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{

	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->info->fake_batt_full);
}
static CLASS_ATTR_RW(fake_batt_full);

#ifdef KERNEL_6
static ssize_t input_suspend_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t input_suspend_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	bool enable = false;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

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
static CLASS_ATTR_RW(input_suspend);

#ifdef KERNEL_6
static ssize_t set_ship_mode_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t set_ship_mode_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->ship_mode);
}

#ifdef KERNEL_6
static ssize_t set_ship_mode_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t set_ship_mode_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int ret;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

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
static CLASS_ATTR_RW(set_ship_mode);

#ifdef KERNEL_6
static ssize_t shipmode_count_reset_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t shipmode_count_reset_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->ship_mode);
}

#ifdef KERNEL_6
static ssize_t shipmode_count_reset_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t shipmode_count_reset_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int ret;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

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
static CLASS_ATTR_RW(shipmode_count_reset);

#ifdef KERNEL_6
static ssize_t ffc_disable_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t ffc_disable_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int val, i = 0;
	struct chg_alg_device *alg = NULL;

	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtoint(buf, 10, &val)) {
		sysfs_err("get buf error %s\n", buf);
		return -EINVAL;
	}

	if (val) {
		desc->info->bat.ffc_disable = true;
		 for (i = 0; i < MAX_ALG_NO; i++) {
			 alg = desc->info->alg[i];
			 if (alg == NULL)
				 continue;
			 chg_alg_stop_algo(alg);
		 }
	} else {
		desc->info->bat.ffc_disable = false;
	}
	sysfs_info("set ffc_disable = %d\n", val);

	return count;
}
#ifdef KERNEL_6
static ssize_t ffc_disable_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t ffc_disable_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->info->bat.ffc_disable);
}
static CLASS_ATTR_RW(ffc_disable);

/*add for SW-CID start*/
#ifdef KERNEL_6
static ssize_t sw_cid_enable_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t sw_cid_enable_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	sysfs_info("%s %d\n", __func__, desc->cid_enable);
	return sprintf(buf, "%d\n", desc->cid_enable);
}

#ifdef KERNEL_6
static ssize_t sw_cid_enable_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t sw_cid_enable_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtobool(buf, &desc->cid_enable)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	sysfs_info("%s %d\n", __func__, desc->cid_enable);

	return count;
}
CLASS_ATTR_RW(sw_cid_enable);

#ifdef KERNEL_6
static ssize_t screen_on_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t screen_on_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->screen_on);
}

#ifdef KERNEL_6
static ssize_t screen_on_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t screen_on_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int ret;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtobool(buf, &desc->screen_on)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	if(!desc->audio_on){
		ret = set_drp_mode_for_cid(desc);
		if(ret < 0){
			sysfs_err("%s: set drp_mode_for_cid failed, ret = %d \n",__func__, ret);
			return -EINVAL;
		}
	}

	return count;
}
static CLASS_ATTR_RW(screen_on);

#ifdef KERNEL_6
static ssize_t audio_on_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t audio_on_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->audio_on);
}

#ifdef KERNEL_6
static ssize_t audio_on_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
#else
static ssize_t audio_on_store(struct class *class,
	struct class_attribute *attr, const char *buf, size_t count)
#endif
{
	int ret;
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	if (kstrtobool(buf, &desc->audio_on)) {
		sysfs_err("parsing number fail\n");
		return -EINVAL;
	}

	if (!desc->screen_on) {
		ret = set_drp_mode_for_cid(desc);
		if(ret < 0){
			sysfs_err("%s: set drp_mode_for_cid failed, ret = %d \n",__func__, ret);
			return -EINVAL;
		}
	}

	return count;
}
static CLASS_ATTR_RW(audio_on);
/*add for SW-CID end*/
#ifdef KERNEL_6
static ssize_t jeita_cv_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t jeita_cv_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);
	return sprintf(buf, "%d\n", desc->info->sw_jeita.cv);
}
static CLASS_ATTR_RO(jeita_cv);

#ifdef KERNEL_6
static ssize_t ffc_mode_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
#else
static ssize_t ffc_mode_show(struct class *class,
	struct class_attribute *attr, char *buf)
#endif
{
	struct sysfs_desc *desc = container_of(class,
		struct sysfs_desc, lc_charger_class);

	return sprintf(buf, "%d\n", desc->info->bat.ffc);
}
static CLASS_ATTR_RO(ffc_mode);

static struct attribute *lc_charger_attrs[] = {
	&class_attr_real_type.attr,
#if IS_ENABLED(CONFIG_TCPC_CLASS)
	&class_attr_typec_cc_orientation.attr,
	&class_attr_typec_mode.attr,
#endif /* CONFIG_TCPC_CLASS */
	&class_attr_chip_ok.attr,
	&class_attr_resistance_id.attr,
	&class_attr_sw_manufacturer.attr,
	&class_attr_cp_manufacturer.attr,
	&class_attr_cp_bus_voltage.attr,
	&class_attr_cp_bus_current.attr,
	&class_attr_mtbf_current.attr,
	&class_attr_input_suspend.attr,
	&class_attr_shipmode_count_reset.attr,
	&class_attr_set_ship_mode.attr,
	&class_attr_ffc_disable.attr,
	&class_attr_sw_cid_enable.attr,
	&class_attr_screen_on.attr,
	&class_attr_audio_on.attr,
	&class_attr_batt_manufacturer.attr,
	&class_attr_jeita_cv.attr,
	&class_attr_ffc_mode.attr,
	&class_attr_fake_batt_full.attr,
	NULL,
};

ATTRIBUTE_GROUPS(lc_charger);

int lc_charger_node_init(struct sysfs_desc *desc)
{
	int ret = 0;

	desc->lc_charger_class.name = "lc_charger";
	desc->lc_charger_class.class_groups = lc_charger_groups;
	ret = class_register(&desc->lc_charger_class);
	if (ret < 0) {
		sysfs_err("Failed to create lc_charger_class\n");
		goto out;
	}
out:
	return ret;
}
