#include "xm_smart_chg.h"
#include <linux/printk.h>

#define TAG                     "[xm_smart_chg]"
#define xm_err(fmt, ...)        pr_err(TAG "[%s]:" fmt, __func__, ##__VA_ARGS__)
#define xm_warn(fmt, ...)       pr_warn(TAG "[%s]:" fmt, __func__, ##__VA_ARGS__)
#define xm_notice(fmt, ...)     pr_notice(TAG "[%s]:" fmt, __func__, ##__VA_ARGS__)
#define xm_info(fmt, ...)       pr_info(TAG "[%s]:" fmt, __func__, ##__VA_ARGS__)
#define xm_debug(fmt, ...)      pr_debug(TAG "[%s]:" fmt, __func__, ##__VA_ARGS__)

struct xm_smart_chg_info *info;
static void *cookie = NULL;
//extern void set_fv_ageing(int val);

/*smart chg all fun*/

static int input_current_limit(bool en)
{
	if(en) {
		vote(info->usb_icl_votable, SMART_INPUT_LIMIT, true, 1000000);
		vote(info->fcc_main_votable, FCC_STEPPER_VOTER, false, 0);
		vote(info->fcc_main_votable, SMART_INPUT_LIMIT, true, 0);
		vote(info->fcc_votable, SMART_INPUT_LIMIT, true, 0);
	} else {
		vote(info->usb_icl_votable, SMART_INPUT_LIMIT, false, 1000000);
		vote(info->fcc_main_votable, SMART_INPUT_LIMIT, false, 0);
		vote(info->fcc_votable, SMART_INPUT_LIMIT, false, 0);
	}

	return 0;
}

void set_error(struct xm_smart_chg_info *manager)
{
	manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret = 1;
	xm_err("en_ret=%d\n", manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret);
}

void set_success(struct xm_smart_chg_info *manager)
{
	manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret = 0;
	xm_info("en_ret=%d\n", manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret);
}

int smart_chg_is_error(struct xm_smart_chg_info *manager)
{
	return manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret? true : false;
}

void handle_smart_chg_functype(struct xm_smart_chg_info *manager,
	const int func_type, const int en_ret, const int func_val)
{
	switch (func_type)
	{
	case SMART_CHG_FEATURE_MIN_NUM ... SMART_CHG_FEATURE_MAX_NUM:
		manager->smart_charge[func_type].en_ret = en_ret;
		manager->smart_charge[func_type].active_status = false;
		manager->smart_charge[func_type].func_val = func_val;
		set_success(manager);
		xm_err("set func_type:%d, en_ret = %d\n", func_type, en_ret);
		break;
	default:
		xm_err( "ERROR: Not supported func type: %d\n", func_type);
		set_error(manager);
		break;
	}
}

int handle_smart_chg_functype_status(struct xm_smart_chg_info *manager)
{
	int i;
	int all_func_status = 0;
	all_func_status |= !!manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret;	//handle bit0

	xm_info("all_func_status =%#X, en_ret=%d\n",all_func_status, manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret);

	/* save functype[i] enable status in all_func_status bit[i] */
	for(i = SMART_CHG_FEATURE_MIN_NUM; i <= SMART_CHG_FEATURE_MAX_NUM; i++){  //handle bit1 ~ bit SMART_CHG_FEATURE_MAX_NUM
		if(manager->smart_charge[i].en_ret)
			all_func_status |= BIT_MASK(i);
		else
			all_func_status &= ~BIT_MASK(i);
		xm_info("type:%d, en_ret=%d, active_status=%d,func_val=%d, all_func_status=%#X\n",
			i, manager->smart_charge[i].en_ret, manager->smart_charge[i].active_status, manager->smart_charge[i].func_val,all_func_status);
	}

	return all_func_status;
}

static void xm_smart_chg_endurance_protect(void)
{
    struct xm_smart_chg_info *manager = NULL;
    struct power_supply *psy = NULL;
    struct votable *CHG_DISABLE = NULL;
    static int g_smart_soc = 80;
    union power_supply_propval pval = {0,};
    int ret = 0;

    if (IS_ERR_OR_NULL(info)) {
        pr_err("%s point info is err or null\n", __func__);
        return;
    }

    manager = info;
    psy = power_supply_get_by_name("bq28z610");
    if (IS_ERR_OR_NULL(psy)) {
        pr_err("%s get battery fail\n", __func__);
        return;
    }
    manager->batt_psy = psy;
    ret = power_supply_get_property(manager->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &pval);
    if (ret < 0) {
        pr_err("%s get battery soc error.\n", __func__);
        return;
    } else {
        manager->soc = pval.intval;
    }

    CHG_DISABLE = find_votable("CHG_DISABLE");
    if (!CHG_DISABLE) {
        pr_err("%s find vote CHG_DISABLE fail\n", __func__);
        return;
    }

    if(manager->smart_charge[SMART_CHG_ENDURANCE_PRO].en_ret &&
                manager->soc >= manager->smart_charge[SMART_CHG_ENDURANCE_PRO].func_val) {
        xm_info("disable charger, en_ret = %d, fun_val = %d, active_status = %d, smart_soc = %d, ui_soc =%d, smart_ctrl_en = %d\n",
            manager->smart_charge[SMART_CHG_ENDURANCE_PRO].en_ret,
            manager->smart_charge[SMART_CHG_ENDURANCE_PRO].func_val,
            manager->smart_charge[SMART_CHG_ENDURANCE_PRO].active_status,
            g_smart_soc,
            manager->soc,
            manager->smart_ctrl_en);

        manager->smart_charge[SMART_CHG_ENDURANCE_PRO].active_status = true;
        manager->endurance_protect_flag = true;
        info->endurance_protect_flag = true;
        g_smart_soc = manager->smart_charge[SMART_CHG_ENDURANCE_PRO].func_val;
        ret = vote(CHG_DISABLE, ENDURANCE_PRO_VOTER, true, 0);
        if (ret < 0) {
                pr_err("%s vote error.\n", __func__);
        }
    } else if (manager->endurance_protect_flag == true  &&
                (!manager->smart_charge[SMART_CHG_ENDURANCE_PRO].en_ret || (manager->soc <= 75))) {
        manager->smart_charge[SMART_CHG_ENDURANCE_PRO].active_status = false;
        manager->endurance_protect_flag = false;
        info->endurance_protect_flag = false;
        ret = vote(CHG_DISABLE, ENDURANCE_PRO_VOTER, false, 0);
        if (ret < 0) {
                pr_err("%s vote error.\n", __func__);
        }
        xm_info("enable charger, uisoc = %d\n", manager->soc);
    }

    return;
}

void monitor_smart_chg(void)
{
    struct power_supply *psy = NULL;
    struct votable	*CHG_DISABLE = NULL;
    struct xm_smart_chg_info *manager;
    union power_supply_propval pval = {0,};
    int ret = 0;
    static int g_smart_soc = 50;

    manager = info;
    psy = power_supply_get_by_name("battery");

    if (psy == NULL) {
        xm_info("call monitor_smart_chg failed\n");
        return;
    }

    CHG_DISABLE = find_votable("CHG_DISABLE");
    if (!CHG_DISABLE)
        return;
    manager->batt_psy = psy;
    ret = power_supply_get_property(manager->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &pval);
    if (ret < 0)
        xm_info("get battery soc error.\n");
    else
        manager->soc = pval.intval;

    if(manager->smart_charge[SMART_CHG_NAVIGATION].en_ret && manager->soc >= manager->smart_charge[SMART_CHG_NAVIGATION].func_val && !manager->smart_charge[SMART_CHG_NAVIGATION].active_status) {
        xm_info("monitor disable charger, monitor_smart_chg, en_ret = %d, fun_val = %d, active_status = %d, smart_soc = %d, ui_soc =%d, smart_ctrl_en = %d\n",
            manager->smart_charge[SMART_CHG_NAVIGATION].en_ret,
            manager->smart_charge[SMART_CHG_NAVIGATION].func_val,
            manager->smart_charge[SMART_CHG_NAVIGATION].active_status,
            g_smart_soc,
            manager->soc,
            manager->smart_ctrl_en);

        manager->smart_charge[SMART_CHG_NAVIGATION].active_status = true;
        g_smart_soc = manager->smart_charge[SMART_CHG_NAVIGATION].func_val;
        vote(CHG_DISABLE, SMART_CHARGING_VOTER, true, 0);
        manager->smart_ctrl_en = true;
    } else if ((((!manager->smart_charge[SMART_CHG_NAVIGATION].en_ret || manager->soc <= (manager->smart_charge[SMART_CHG_NAVIGATION].func_val - 5)) && manager->smart_charge[SMART_CHG_NAVIGATION].active_status) ||
                (!manager->smart_charge[SMART_CHG_NAVIGATION].en_ret && manager->soc >= manager->smart_charge[SMART_CHG_NAVIGATION].func_val && !manager->smart_charge[SMART_CHG_NAVIGATION].active_status)) && manager->smart_ctrl_en) {
        manager->smart_charge[SMART_CHG_NAVIGATION].active_status = false;
        vote(CHG_DISABLE, SMART_CHARGING_VOTER, false, 0);
        manager->smart_ctrl_en = false;
        xm_info("monitor enable charger, soc <= %d\n", (manager->smart_charge[SMART_CHG_NAVIGATION].func_val - 5));
    }

    if((manager->smart_charge[SMART_CHG_LOW_FAST].en_ret) && (!manager->smart_charge[SMART_CHG_LOW_FAST].active_status)) {
        manager->smart_charge[SMART_CHG_LOW_FAST].active_status = true;
        xm_info("set smart_charge[SMART_CHG_LOW_FAST].en_ret = %d, enable low_fast\n", manager->smart_charge[SMART_CHG_LOW_FAST].en_ret);
    } else if ((!manager->smart_charge[SMART_CHG_LOW_FAST].en_ret) && (manager->smart_charge[SMART_CHG_LOW_FAST].active_status)) {
        manager->smart_charge[SMART_CHG_LOW_FAST].active_status = false;
        xm_info("set smart_charge[SMART_CHG_LOW_FAST].en_ret = %d, disable low_fast\n", manager->smart_charge[SMART_CHG_LOW_FAST].en_ret);
    }
}
#if 0
static ssize_t smart_chg_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	// DECLEAR_BITMAP(func_type, SMART_CHG_FEATURE_MAX_NUM);
	int val;
	bool en_ret;
	unsigned long func_type;
	int func_val;
	int bit_pos;
	int all_func_status;
	struct xm_smart_chg_info *manager = NULL;

	manager = info;

	if (IS_ERR_OR_NULL(manager))
			return PTR_ERR(manager);

	if (kstrtoint(buf, 16, &val))
			return -EINVAL;

	en_ret = val & 0x1;
	func_type = (val & 0xFFFE) >> 1;
	func_val = val >> 16;

	xm_info("O19:get val:%#X, func_type:%#X, en_ret:%d, func_val:%d\n",
		val, func_type, en_ret, func_val);
	bit_pos = find_first_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM);

   if(bit_pos == SMART_CHG_FEATURE_MAX_NUM || find_next_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM , bit_pos + 1) != SMART_CHG_FEATURE_MAX_NUM){
           xm_err("ERROR: zero or more than one func type!\n");
           xm_err("find_next_bit = %d, bit_pos = %d\n",
                   find_next_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM , bit_pos + 1), bit_pos);
           set_error(manager);
   } else
           set_success(manager);

  // if func_type bit0 is 1, bit_pos = 0, not 1. so ++bit_pos.
   if(!smart_chg_is_error(manager))
           handle_smart_chg_functype(manager, ++bit_pos, en_ret, func_val);

   /* update smart_chg[0] status */
   all_func_status = handle_smart_chg_functype_status(manager);
   manager->smart_chg_cmd = all_func_status;
   manager->smart_charge[SMART_CHG_STATUS_FLAG].en_ret = all_func_status & 0x1;
   manager->smart_charge[SMART_CHG_STATUS_FLAG].active_status = (all_func_status & 0xFFFE) >> 1;

   return count;
}
#endif
void handle_smart_chg_value(int val)
{
	bool en_ret;
	unsigned long func_type;
	int func_val;
	int bit_pos;
	int all_func_status;
        int outdoor;

        en_ret = val & 0x1;
	func_type = (val & 0xFFFE) >> 1;
	func_val = val >> 16;
        outdoor = (val >> 2) & 0x1;

	xm_info("O19:get val:%#X, func_type:%#X, en_ret:%d, func_val:%d, outdoor:%d\n",
		val, func_type, en_ret, func_val, outdoor);
	bit_pos = find_first_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM);

        if(bit_pos == SMART_CHG_FEATURE_MAX_NUM || find_next_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM , bit_pos + 1) != SMART_CHG_FEATURE_MAX_NUM){
                xm_err("ERROR: zero or more than one func type!\n");
                xm_err("find_next_bit = %d, bit_pos = %d\n",
                        find_next_bit(&func_type, SMART_CHG_FEATURE_MAX_NUM , bit_pos + 1), bit_pos);
                set_error(info);
        } else
                set_success(info);

        // if func_type bit0 is 1, bit_pos = 0, not 1. so ++bit_pos.
        if(!smart_chg_is_error(info))
                handle_smart_chg_functype(info, ++bit_pos, en_ret, func_val);

        /* update smart_chg[0] status */
        all_func_status = handle_smart_chg_functype_status(info);
        info->smart_chg_cmd = all_func_status;
        info->smart_charge[SMART_CHG_STATUS_FLAG].en_ret = all_func_status & 0x1;
        info->smart_charge[SMART_CHG_STATUS_FLAG].active_status = (all_func_status & 0xFFFE) >> 1;
        info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].en_ret = en_ret;
        info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].active_status = outdoor;
}
#if 0
static ssize_t smart_chg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct xm_smart_chg_info *manager = NULL;

	manager = info;

	if (IS_ERR_OR_NULL(manager))
			return PTR_ERR(manager);

	return sprintf(buf, "%d\n", manager->smart_chg_cmd);
}

static struct device_attribute smart_chg_attr =
		__ATTR(smart_chg, 0644, smart_chg_show, smart_chg_store);

static ssize_t night_charging_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
        xm_err("night_charging = %d", info->night_charging);
	return sprintf(buf, "%d\n", info->night_charging) + 1;
}
static ssize_t night_charging_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	if (kstrtobool(buf, &info->night_charging))
		return -EINVAL;
        xm_err("night_charging = %d", info->night_charging);

	return size;
}
static struct device_attribute night_charging_attrs =
	__ATTR(night_charging, 0644, night_charging_show, night_charging_store);

static ssize_t raw_soc_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
        xm_err("raw_soc = %d", info->raw_soc);
	return sprintf(buf, "%d\n", info->raw_soc) + 1;
}
static ssize_t raw_soc_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
        if (kstrtoint(buf, 0, &info->raw_soc))
		return -EINVAL;
        xm_err("raw_soc = %d", info->raw_soc);

	return size;
}
static struct device_attribute raw_soc_attrs =
	__ATTR(raw_soc, 0644, raw_soc_show, raw_soc_store);
#endif

static ssize_t smart_batt_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
        xm_err("smart_batt = %d", info->smart_batt);
	return sprintf(buf, "%d\n", info->smart_batt) + 1;
}
static ssize_t smart_batt_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	if (kstrtoint(buf, 0, &info->smart_batt))
		return -EINVAL;
        xm_err("smart_batt = %d", info->smart_batt);

	return size;
}
static struct device_attribute smart_batt_attrs =
	__ATTR(smart_batt, 0644, smart_batt_show, smart_batt_store);

static struct attribute *batt_psy_attrs[] = {
	&smart_batt_attrs.attr,
//	&night_charging_attrs.attr,
//	&smart_chg_attr.attr,
//	&raw_soc_attrs.attr,
	NULL,
};

static const struct attribute_group batt_psy_attrs_group = {
	.attrs = batt_psy_attrs,
};

int xm_smart_charge_add_to_batt_psy(void)
{
        struct power_supply *batt_psy;
        int ret = 0;

        batt_psy = power_supply_get_by_name("battery");
        if(!batt_psy){
                xm_err("failed to get batt_psy");
                return -EPROBE_DEFER;
        }

        ret = sysfs_create_group(&batt_psy->dev.kobj, &batt_psy_attrs_group);
        if(ret){
                xm_err("failed to creat batt_psy group, ret = %d\n", ret);
                return -EPROBE_DEFER;
        }

        return ret;
}

void get_fv_ageing(int cyclecount)
{
        int i = 0;

        while (cyclecount > cycle_count_conf[i])
                i++;
        info->fv_ageing = dropfv_conf[i];
        //set_fv_ageing(info->fv_ageing);
        xm_debug("i = %d, fv_ageing = %d\n", i, info->fv_ageing);
        return;
}

void get_drop_floatvolatge(void)
{
        int ret = 0;
        struct power_supply   *batt_psy;
        union power_supply_propval pval;
        int cycle_count = 0;
        static int last_fv_ageing = 0;

        batt_psy = power_supply_get_by_name("battery");
        if (!batt_psy){
                xm_err("failed to get batt_psy, use default fv_aging: 0");
                info->fv_ageing = 0;
		return;
        }
        ret = power_supply_get_property(batt_psy, POWER_SUPPLY_PROP_CYCLE_COUNT, &pval);
        if (ret < 0) {
                xm_err("failed to get cycle_count_prop, use default fv_aging: 0");
                info->fv_ageing = 0;
                return;
        }
        cycle_count = pval.intval;
        xm_debug("cycle_count = %d", cycle_count);

        get_fv_ageing(cycle_count);
        if(info->fv_ageing != last_fv_ageing){
                if (info->fv_votable) {
                        rerun_election(info->fv_votable);
                }
        }
        last_fv_ageing = info->fv_ageing;
}

void monitor_night_charging(void)
{
        int ret = 0;

        if (info->night_charging && info->soc >= 80) {
                xm_info("soc = %d, night_charging = %d, disable charging", info->soc, info->night_charging);
                info->night_charging_flag = true;
                ret = vote(info->chg_disable_votable, NIGHT_CHARGING_VOTER, true, 0);
                if (ret < 0) {
                        xm_err("Couldn't vote to chg_disable_votable, ret = %d\n", ret);
                        return;
                }
                vote(info->night_charging_votable, NIGHT_CHARGING_VOTER, true, 0);
        } else if((info->night_charging_flag) && (!info->night_charging || info->soc <= 75)) {
                xm_info("soc = %d, night_charging = %d, enable charging", info->soc, info->night_charging);
                info->night_charging_flag = false;
                ret = vote(info->chg_disable_votable, NIGHT_CHARGING_VOTER, false, 0);
                if (ret < 0) {
                        xm_err("Couldn't vote to chg_disable_votable, ret = %d\n", ret);
                        return;
                }
                vote(info->night_charging_votable, NIGHT_CHARGING_VOTER, false, 0);
        }
}

void monitor_low_fast_strategy(void)
{
        bool fast_flag = false;
        time64_t time_now = 0, delta_time = 0;
        static time64_t time_last = 0;
        int thermal_vote_current;
        static int last_level = 0;
        static bool hot_flag = false;
        static int count = 0;

        if(count++ == 5){
                xm_err("soc = %d, low_fast_plugin_flag = %d, low_fast_enable = %d, thermal_level = %d, thermal_board_temp = %d, pd_active = %d, screen_state = %d, b_flag = %d\n",
                                info->soc,
                                info->low_fast_plugin_flag,
                                info->smart_charge[SMART_CHG_LOW_FAST].active_status,
                                info->thermal_level,
                                info->thermal_board_temp,
                                info->pd_active,
                                info->screen_state,
                                info->b_flag);
                count = 0;
        }

        //if (info->thermal_level <= 0)
	//	goto err;
        if ((info->pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE)
                        && (info->soc <= 40)
                        && (info->low_fast_plugin_flag)
                        && (info->smart_charge[SMART_CHG_LOW_FAST].active_status)) {
		if (info->thermal_level > MAX_THERMAL_LEVELS) {
			xm_err("thermal_level is invalid\n");
			goto err;
		}

                //info->screen_state 0:bright, 1:black
                if(info->b_flag == NORMAL && !info->screen_state) {  //black to bright
                        info->b_flag = BLACK_TO_BRIGHT;
                        time_last = ktime_get_seconds();
                        fast_flag = true;
                        xm_err("switch to bright time_last = %d\n", time_last);
                } else if((info->b_flag == BLACK_TO_BRIGHT || info->b_flag == BRIGHT) && !info->screen_state) {  //still bright
                        info->b_flag = BRIGHT;
                        time_now = ktime_get_seconds();
                        delta_time = time_now - time_last;
                        if(delta_time <= 15) {
                                fast_flag = true;
                                xm_err("still_bright time_now = %d, time_last = %d, delta_time = %d, stay fast\n",
                                                time_now, time_last, delta_time);
                        }
                        else {
                                fast_flag = false;
                                xm_err("still_bright time_now = %d, time_last = %d, exit fast = %d, stay fast\n",
                                                time_now, time_last, delta_time);
                        }
                } else { //black
                        info->b_flag = BLACK;
                        fast_flag = true;
                        xm_err("black stay fast\n");
                }

                /*avoid thermal_board_temp raise too fast*/
                if((last_level == 8) && (info->thermal_level == 5) && (info->thermal_board_temp > 415)){
                        hot_flag = true;
                        fast_flag = false;
                        xm_err("avoid thermal_board_temp raise too fast, exit fast mode\n");
                }
                else if((last_level == 5) && ((info->thermal_level == 5) || (info->thermal_level == 8)) && hot_flag && (info->thermal_board_temp > 415)){
                        fast_flag = false;
                }
                else{
                        hot_flag = false;
                }

                if((info->thermal_board_temp > 420) || (info->soc > 38)){
                        fast_flag = false;
                }

                if(fast_flag) {  //stay fast strategy
                        info->pps_fast_mode = true;
                        thermal_vote_current = info->pps_thermal_mitigation_fast[info->thermal_level];

                        if((info->soc > 38) && (info->thermal_board_temp > 380)){
                                if(thermal_vote_current >= 5450000){
                                        thermal_vote_current -= 3300000;
                                }
                                else{
                                        thermal_vote_current = 2150000;
                                }
                                xm_err("stay fast but decrease 3.3, thermal_board_temp = %d, thermal_vote_current = %d\n", info->thermal_board_temp, thermal_vote_current);
                        }
                        else if(info->thermal_board_temp > 410){
                                if(thermal_vote_current >= 5150000){
                                        thermal_vote_current -= 3000000;
                                }
                                else{
                                        thermal_vote_current = 2150000;
                                }
                                xm_err("stay fast but decrease 3, thermal_board_temp = %d, thermal_vote_current = %d\n", info->thermal_board_temp, thermal_vote_current);
                        }
                        else if((info->soc > 30) && (info->soc < 39) && (info->thermal_board_temp < 390)){
                                if(thermal_vote_current <= 3950000){
                                        thermal_vote_current += 2000000;
                                }
                                else{
                                        thermal_vote_current = 5950000;
                                }
                                xm_err("stay fast but add 2, thermal_board_temp = %d, thermal_vote_current = %d\n", info->thermal_board_temp, thermal_vote_current);
                        }
                        else if((info->soc >= 15) && ((info->soc < 38)) && (info->thermal_board_temp < 400)){
                                if(thermal_vote_current <= 2950000){
                                        thermal_vote_current += 3000000;
                                }
                                else{
                                        thermal_vote_current = 5950000;
                                }
                                xm_err("stay fast but add 3, thermal_board_temp = %d, thermal_vote_current = %d\n", info->thermal_board_temp, thermal_vote_current);
                        }
                        vote(info->low_fast_votable, LOW_FAST_VOTER, true, 0);
                        vote(info->fcc_votable, THERMAL_DAEMON_VOTER, true, thermal_vote_current);
                        xm_err("stay fast, thermal_vote_current = %d\n", thermal_vote_current);
                }
                else { //exit fast strategy
                        info->pps_fast_mode = false;
                        vote(info->low_fast_votable, LOW_FAST_VOTER, false, 0);
                        thermal_vote_current = info->pps_thermal_mitigation[info->thermal_level];
                        vote(info->fcc_votable, THERMAL_DAEMON_VOTER, true, thermal_vote_current);
                        xm_err("exit fast, thermal_vote_current = %d\n", thermal_vote_current);
                }
	}
        last_level = info->thermal_level;

	return;
err:
        vote(info->low_fast_votable, LOW_FAST_VOTER, false, 0);
        vote(info->fcc_votable, THERMAL_DAEMON_VOTER, true, info->pps_thermal_mitigation[info->thermal_level]);
        last_level = info->thermal_level;
	return;
}

void xm_charge_update_date(void)
{
        int ret = 0;

        info->fv_votable = find_votable("FV");
        if (!info->fv_votable) {
                xm_err("failed to get fv_votable\n");
                return;
        }
        info->fcc_votable = find_votable("FCC");
        if (!info->fcc_votable) {
                xm_err("failed to get fcc_votable\n");
                return;
        }
        info->fcc_main_votable = find_votable("FCC_MAIN");
        if (!info->fcc_main_votable) {
        	xm_err("failed to get fcc_main_votable\n");
        	return;
        }
        info->chg_disable_votable = find_votable("CHG_DISABLE");
        if (!info->chg_disable_votable) {
                xm_err("failed to get chg_disable_votable\n");
                return;
        }
        info->low_fast_votable = find_votable("LOW_FAST");
        if (!info->low_fast_votable) {
                xm_err("failed to get low_fast_votable\n");
                return;
        }
        info->night_charging_votable = find_votable("NIGHT_CHARGING");
        if (!info->night_charging_votable) {
                xm_err("failed to get night_charging_votable\n");
                return;
        }
        info->usb_icl_votable = find_votable("USB_ICL");
	if (info->usb_icl_votable == NULL) {
		xm_err("failed to get USB_ICL votable");
		return;
	}

        ret = iio_read_channel_processed(info->capacity_raw_chan, &info->raw_soc);
	if (ret < 0) {
		xm_err("raw_soc channel read failed, ret=%d\n", ret);
	}
        ret = iio_read_channel_processed(info->capacity_chan, &info->soc);
	if (ret < 0) {
		xm_err("capacity channel read failed, ret=%d\n", ret);
		return;
	}
        ret = iio_read_channel_processed(info->pd_active_chan, &info->pd_active);
	if (ret < 0) {
		xm_err("pd_active channel read failed, ret=%d\n", ret);
	}
        ret = iio_read_channel_processed(info->real_type_chan, &info->real_type);
	if (ret < 0) {
		xm_err("real_type channel read failed, ret=%d\n", ret);
	}

        get_drop_floatvolatge();
}
#define USB_DCP  5
#define DCP_CURRENT 1800000
#define OUTDOOR_CURR 1900000
void monitor_outdoor_charging(void)
{
    if(info->real_type == USB_DCP){
        xm_err("smart_chg en = %d, outdoor_charging = %d",
                    info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].en_ret,
                    info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].active_status);

        if(info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].en_ret && info->smart_charge[SMART_CHG_OUTDOOR_CHARGE].active_status){
            xm_err("enter enable outdoor charging...\n");
            if(info->outdoor_flag != 1){
                vote(info->fcc_votable, OUTDOOR_FCC_VOTER, false, 0);
                vote(info->fcc_votable, OUTDOOR_FCC_VOTER, true, OUTDOOR_CURR);
                vote(info->usb_icl_votable, OUTDOOR_ICL_VOTER, false, 0);
                vote(info->usb_icl_votable, SW_ICL_MAX_VOTER, false, 0);
                vote(info->usb_icl_votable, OUTDOOR_ICL_VOTER, true, OUTDOOR_CURR);
                info->outdoor_flag = 1;
                xm_err("vote outdoor charging 1900ma...\n");
            }
        }else{
                info->outdoor_flag = 0;
                xm_err("exit outdoor charging...\n");
                vote(info->fcc_votable, OUTDOOR_FCC_VOTER, true, DCP_CURRENT);
                vote(info->usb_icl_votable, SW_ICL_MAX_VOTER, false, 0);
                vote(info->usb_icl_votable, OUTDOOR_ICL_VOTER, true, DCP_CURRENT);
                vote(info->usb_icl_votable, SW_ICL_MAX_VOTER, true, DCP_CURRENT);
        }
    }else{
        info->outdoor_flag = 2;
        vote(info->fcc_votable, OUTDOOR_FCC_VOTER, false, 0);
        vote(info->usb_icl_votable, OUTDOOR_ICL_VOTER, false, 0);
    }
}

#define MAX_CHECK_POST_INIT_READY_COUNT 1000
void xm_charge_work(struct work_struct *work)
{
    xm_charge_update_date();
    monitor_night_charging();
    monitor_smart_chg();
    monitor_low_fast_strategy();
    monitor_outdoor_charging();
    xm_smart_chg_endurance_protect();

    xm_info("night_charging_flag = %d, endurance_protect_flag = %d\n",
            info->night_charging_flag,
            info->endurance_protect_flag);
    if(info->night_charging_flag || info->endurance_protect_flag) {
        input_current_limit(true);
    } else {
        input_current_limit(false);
    }

    /*
    * Move to qpnp-smb5-main
    * monitor_smart_batt();
    * monitor_cycle_count();
    **/
    if (info->soc <= 40) {
        schedule_delayed_work(&info->xm_charge_work, msecs_to_jiffies(XM_L_SOC_CHG_WORK_MS));
    } else {
        schedule_delayed_work(&info->xm_charge_work, msecs_to_jiffies(XM_CHARGE_WORK_MS));
    }
}
static struct drm_panel *prim_panel;

void xm_bat_protect_work(struct work_struct *work)
{
    int ret;
    int cp_enable;
    int fv;
    int vol_now;
    int cur_now;
    struct power_supply *fg_psy = NULL;
    union power_supply_propval val = {0,};

    if(IS_ERR_OR_NULL(info) || IS_ERR_OR_NULL(info->pd_active_chan)){
        pr_err("%s info is err or null\n", __func__);
        return;
    }
    ret = iio_read_channel_processed(info->pd_active_chan, &info->pd_active);
    if (ret < 0) {
        xm_err("pd_active channel read failed, ret=%d\n", ret);
        goto done;
    }

    if(IS_ERR_OR_NULL(info->cp_enabled_chan)){
        pr_err("%s cp_enabled_chan is err or null\n", __func__);
        return;
    }
    ret = iio_read_channel_processed(info->cp_enabled_chan, &cp_enable);
    if (ret < 0) {
        xm_err("cp_enabled channel read failed, ret=%d\n", ret);
        goto done;
    }

    if(info->pd_active) {
        xm_info("pd_active = %d, cp_enable = %d\n", info->pd_active, cp_enable);

        fg_psy = power_supply_get_by_name("bq28z610");
        if (IS_ERR_OR_NULL(fg_psy)) {
            xm_err("power supply get fg is fail\n");
            goto done;
        }
        ret = power_supply_get_property(fg_psy,
                POWER_SUPPLY_PROP_VOLTAGE_MAX, &val);
        if (ret < 0) {
            xm_err("Failed to get fv\n");
            goto done;
        }
        fv = val.intval;

        ret = power_supply_get_property(info->batt_psy,
                POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
        if (ret < 0) {
            xm_err("Failed to get voltage_now\n");
            goto done;
        }
        vol_now = val.intval;

        if (info->protect_done)
            return;

        if (vol_now > fv - info->smart_batt * 1000) {
            info->over_vbat_timer += 1;
            if (info->over_vbat_timer >= 3) {
                ret = power_supply_get_property(info->batt_psy,
                        POWER_SUPPLY_PROP_CURRENT_NOW, &val);
                if (ret < 0){
                    xm_err("Failed to get current_now\n");
                }
                cur_now = val.intval;

                if (cur_now > 2000000)
                    vote(info->fcc_votable, SMART_PROTECT, true, cur_now - XM_PROTECT_DEC_CUR);
                xm_info("vol_now = %d, FV = %d, cur_now = %d, smart_batt = %d\n", vol_now, fv, cur_now, info->smart_batt);
                info->protect_done = 1;
            }
        }else {
            info->over_vbat_timer = 0;
        }
    }else {
        info->over_vbat_timer = 0;
    }

done:
    schedule_delayed_work(&info->bat_protect_work, msecs_to_jiffies(XM_BAT_PROTECT_MS));
}

static int qpnp_smb5_notifier_event_callback(struct notifier_block *notifier,
			unsigned long chg_event, void *val)
{
    int ret = 0;

    switch (chg_event) {
        /*charger_plugin_event 0:none 1:plugin 2:plugout*/
        case QPNP_SMB5_USB_PLUGIN_EVENT:
                info->charger_plugin_event = *(int *)val;
                //xm_info("get charger_plugin_event: %d\n", info->charger_plugin_event);

                ret = iio_read_channel_processed(info->capacity_chan, &info->soc);
                if (ret < 0) {
                        xm_err("capacity channel read failed, ret=%d\n", ret);
                        return ret;
                }
                //xm_info("soc = %d\n", info->soc);
                if(info->charger_plugin_event == 1){
                        if (info->batt_psy && info->first_flag) {
                                xm_err("batt_psy = %d\n", info->batt_psy);
                                info->first_flag = 0;
                                schedule_delayed_work(&info->bat_protect_work, msecs_to_jiffies(0));
                        }
                }else if(info->charger_plugin_event == 2){
                        info->protect_done = 0;
                        info->over_vbat_timer = 0;
                        vote(info->fcc_votable, SMART_PROTECT, false, 0);
                        info->first_flag = 1;
                        cancel_delayed_work(&info->bat_protect_work);
                        //xm_err("delay cancel\n");
                }

                if((info->charger_plugin_event == 1) && (info->soc <= 40) && (info->thermal_board_temp <= 390))
                {
                        info->low_fast_plugin_flag = 1;
                }
                else if(info->charger_plugin_event == 2)
                {
                        info->low_fast_plugin_flag = 0;
                }
                //info->smart_ctrl_en = false;
                info->pps_fast_mode = false;
                info->b_flag = NORMAL;
                vote(info->low_fast_votable, LOW_FAST_VOTER, false, 0);
                break;
        case QPNP_SMB5_SMART_CHG_EVENT:
                handle_smart_chg_value(*(int *)val);
                xm_info("get smart_chg_event, analyzing...\n");
                break;
        case QPNP_SMB5_SMART_BATT_EVENT:
                info->smart_batt = *(int *)val;
                xm_info("get smart_batt_event: %d\n", info->smart_batt);
                break;
        case QPNP_SMB5_NIGHT_CHARGING_EVENT:
                info->night_charging = *(int *)val;
                xm_info("get night_charging_event = %d\n", info->night_charging);
                break;
        default:
                xm_debug("not supported charger notifier event: %d\n", chg_event);
                break;
    }

	return NOTIFY_DONE;
}

int xm_smart_chg_post_init(void)
{
    int ret = 0;

    xm_err("++\n");
    /*add sysfs node to battery psy*/
    ret = xm_smart_charge_add_to_batt_psy();
    if(ret){
        xm_err("xm_smart_charge_add_to_batt_psy failed(%d)\n", ret);
        return ret;
    }

    /*register display panel*/
    ret = xm_smart_chg_register_panel_notifier();
    if (ret < 0) {
        xm_err("xm_smart_chg_register_panel_notifier failed(%d)\n", ret);
    }

    /*register mi_thermal_notifier*/
    info->thermal_board_temp = 250;
    info->thermal_level = 0;

    /*register qpnp_smb5_notifier*/
    info->qpnp_smb5_nb.notifier_call = qpnp_smb5_notifier_event_callback;
    qpnp_smb5_reg_notifier(&info->qpnp_smb5_nb);

    return 0;
}


#define MAX_CHECK_DISPLAY_READY_COUNT 1000
static int xm_smart_chg_check_panel(void)
{
    struct device_node *charger_screen_node, *panel_node;
    struct drm_panel *panel;
    int retry_num = 0;
    int count;
    int i;

    charger_screen_node = of_find_node_by_name(NULL, "charger-screen");
    if (!charger_screen_node) {
        xm_err("ERROR: Cannot find charger_screen_node with panel!");
        return -ENODEV;
    }

    count = of_count_phandle_with_args(charger_screen_node, "panel", NULL);
    xm_err("count of panel in node is: %d\n" ,count);
    if (count <= 0){
        return -ENODEV;
    }

    do {
        for (i = 0; i < count; i++) {
            panel_node = of_parse_phandle(charger_screen_node, "panel", i);
            xm_info("retry_num: %d, try to add of node panel: %s\n", retry_num, panel_node);
            panel = of_drm_find_panel(panel_node);
            of_node_put(panel_node);
            if (!IS_ERR(panel)) {
                prim_panel = panel;
                retry_num = 3;
                break;
            } else {
                prim_panel = NULL;
                msleep(500);
            }
        }
        ++retry_num;
    } while (retry_num < 3);

    if (PTR_ERR(prim_panel) == -EPROBE_DEFER) {
        xm_err("ERROR: Cannot find prim_panel of panel_node!");
        return -EPROBE_DEFER;
    }

    xm_info("count of panel in panel_node PTR_ERR_prim_panel  is: %d\n", PTR_ERR(prim_panel));
    return 0;
}

static void screen_state_for_xm_smart_chg_callback(enum panel_event_notifier_tag tag,
		struct panel_event_notification *notification, void *client_data)
{
        int ret = 0;

	if (!notification) {
		xm_err("Invalid notification\n");
		return;
	}

	if(notification->notif_data.early_trigger) {
		return;
	}
	if(tag == PANEL_EVENT_NOTIFICATION_PRIMARY){
		switch (notification->notif_type) {
			case DRM_PANEL_EVENT_UNBLANK:
				info->screen_state = 0;//bright
				break;
			case DRM_PANEL_EVENT_BLANK:
			case DRM_PANEL_EVENT_BLANK_LP:
				info->screen_state = 1;//black
				break;
			case DRM_PANEL_EVENT_FPS_CHANGE:
				return;
			default:
				return;
		}
		xm_err("screen_state = %d\n", info->screen_state);
	}

        if (IS_ERR_OR_NULL(info->backlight_status_chan)) {
                pr_err("%s backlight_status_chan is err, set screen_state fail\n", __func__);
        } else {
                ret = iio_write_channel_raw(info->backlight_status_chan, info->screen_state);
                if (ret < 0) {
                        pr_err("%s backlight_status_chan write fail\n", __func__);
                }
                pr_info("%s screen_state: %d\n", __func__, info->screen_state);
        }
}

#define MAX_XM_SMART_CHG_CHECK_PANEL_COUNT 10
int xm_smart_chg_register_panel_notifier(void)
{
    int retval = 0;
    void *pvt_data = NULL;
    int i = 0;

    for(i = 0; i < MAX_XM_SMART_CHG_CHECK_PANEL_COUNT; i++){
        retval = xm_smart_chg_check_panel();
        if (retval < 0) {
            xm_err("check panel fail(%d), i = %d\n", retval, i);
            if (retval == -EPROBE_DEFER) {
                return retval;
            }
        }
        if (prim_panel) {
            xm_err("success to check panel\n");
            break;
        }
    }

    if (prim_panel) {
        if (!cookie) {
            cookie = panel_event_notifier_register(PANEL_EVENT_NOTIFICATION_PRIMARY,
                                PANEL_EVENT_NOTIFIER_CLIENT_XM_SMART_CHG, prim_panel,
                                screen_state_for_xm_smart_chg_callback, pvt_data);
            if (IS_ERR(cookie)){
                xm_err("Failed to register for prim_panel events\n");
                retval = -EPROBE_DEFER;
                return retval;
            } else {
                xm_err("prim_panel_event_notifier_register register succeed\n");
                retval = 0;
            }
        }
    } else {
        xm_err("Failed to get prim_panel\n");
        retval = -ENODEV;
    }

    return retval;
}

int xm_smart_chg_parse_dt(struct platform_device *pdev)
{
        int ret = 0, byte_len = 0, i = 0;

        ret = of_property_read_string(pdev->dev.of_node,
		"label", &info->label);
	if (ret < 0) {
		xm_err("failed to read label(%d), use default label\n", ret);
		info->label = "smart_charger";
	}
        xm_info("label: %s\n", info->label);

        if (of_find_property(pdev->dev.of_node, "xiaomi,pd-thermal-mitigation-fast", &byte_len)) {
		info->pps_thermal_mitigation_fast = kzalloc(byte_len, GFP_KERNEL);
		if (info->pps_thermal_mitigation_fast == NULL){
                        xm_err("failed to alloc mem for pps_thermal_mitigation_fast\n");
			return -ENOMEM;
                }
		info->thermal_levels = byte_len / sizeof(u32);
		ret = of_property_read_u32_array(pdev->dev.of_node,
				"xiaomi,pd-thermal-mitigation-fast",
				info->pps_thermal_mitigation_fast,
				info->thermal_levels);
		if (ret < 0) {
			xm_err("failed read pps_thermal_mitigation_fast from dtsi, ret = %d\n", ret);
			return ret;
		}
                for (i = 0; i < info->thermal_levels; i++){
                        xm_debug("pps_thermal_mitigation_fast[%d] = %d\n", i, info->pps_thermal_mitigation_fast[i]);
                }
	}

        if (of_find_property(pdev->dev.of_node, "xiaomi,pd-thermal-mitigation", &byte_len)) {
		info->pps_thermal_mitigation = kzalloc(byte_len, GFP_KERNEL);
		if (info->pps_thermal_mitigation == NULL){
                        xm_err("failed to alloc mem for pps_thermal_mitigation\n");
			return -ENOMEM;
                }
		info->thermal_levels = byte_len / sizeof(u32);
		ret = of_property_read_u32_array(pdev->dev.of_node,
				"xiaomi,pd-thermal-mitigation",
				info->pps_thermal_mitigation,
				info->thermal_levels);
		if (ret < 0) {
			xm_err("failed read pps_thermal_mitigation from dtsi, ret = %d\n", ret);
			return ret;
		}
                for (i = 0; i < info->thermal_levels; i++){
                        xm_info("pps_thermal_mitigation[%d] = %d\n", i, info->pps_thermal_mitigation[i]);
                }
	}

        return 0;
}

int xm_smart_chg_info_init(struct platform_device *pdev)
{
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*info));
    if (!indio_dev){
        xm_err("failed to alloc memory for indio_dev");
        return -ENOMEM;
    }
    info = iio_priv(indio_dev);
    info->indio_dev = indio_dev;
    info->first_flag = 1;
    info->smart_batt = 0;
    info->night_charging_flag = false;
    info->endurance_protect_flag = false;
    info->smart_ctrl_en = false;
    info->over_vbat_timer = 0;
    info->protect_done = 0;
    info->intLabel = 33;

    INIT_DELAYED_WORK(&info->xm_charge_work, xm_charge_work);
    INIT_DELAYED_WORK(&info->bat_protect_work, xm_bat_protect_work);
    return 0;
}

/*************************iio******************/
static int xm_smart_chg_iio_write_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan, int val1,
		int val2, long mask)
{
	int rc = 0;

	switch (chan->channel) {
	case PSY_IIO_INTLABEL:
		info->intLabel = val1;
                xm_err("intLabel = %d\n", info->intLabel);
		break;
	default:
		xm_err("Unsupported xm_smart_chg IIO chan %d\n", chan->channel);
		rc = -EINVAL;
		break;
	}

	if (rc < 0)
		xm_err("Couldn't write IIO channel %d, rc = %d\n",
			chan->channel, rc);
	return rc;
}

static int xm_smart_chg_iio_read_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan, int *val1,
		int *val2, long mask)
{
	int rc = 0;
	*val1 = 0;

	switch (chan->channel) {
	case PSY_IIO_INTLABEL:
		*val1 = info->intLabel;
                xm_err("intLabel = %d\n", *val1);
		break;
	default:
		xm_err("Unsupported QG IIO chan %d\n", chan->channel);
		rc = -EINVAL;
		break;
	}

	if (rc < 0) {
		xm_err("Couldn't read IIO channel %d, rc = %d\n",
			chan->channel, rc);
		return rc;
	}
	return IIO_VAL_INT;
}

static int xm_smart_chg_iio_fwnode_xlate(struct iio_dev *indio_dev,
				const struct fwnode_reference_args *iiospec)
{
	struct iio_chan_spec *iio_chan = info->iio_chan;
	int i;

	for (i = 0; i < ARRAY_SIZE(xm_smart_chg_iio_channels);
					i++, iio_chan++){
		if (iio_chan->channel == iiospec->args[0])
			return i;
    }
	return -EINVAL;
}

static const struct iio_info xm_smart_chg_iio_info = {
	.read_raw	= xm_smart_chg_iio_read_raw,
	.write_raw	= xm_smart_chg_iio_write_raw,
	.fwnode_xlate	= xm_smart_chg_iio_fwnode_xlate,
};

static int xm_smart_chg_init_iio(struct platform_device *pdev)
{
        struct iio_dev *indio_dev = info->indio_dev;
	struct iio_chan_spec *chan;
	int xm_smart_chg_num_iio_channels = ARRAY_SIZE(xm_smart_chg_iio_channels);
	int rc = 0, i;

        info->capacity_chan = iio_channel_get(&pdev->dev, "capacity");
	if (IS_ERR(info->capacity_chan)) {
		rc = PTR_ERR(info->capacity_chan);
		if (rc != -EPROBE_DEFER)
			xm_err("capacity channel unavailable, rc=%d\n", rc);
		info->capacity_chan = NULL;
		return rc;
	}
        info->pd_active_chan = iio_channel_get(&pdev->dev, "pd_active");
	if (IS_ERR(info->pd_active_chan)) {
		rc = PTR_ERR(info->pd_active_chan);
		if (rc != -EPROBE_DEFER)
			xm_err("pd_active channel unavailable, rc=%d\n", rc);
		info->pd_active_chan = NULL;
		return rc;
	}
        info->capacity_raw_chan = iio_channel_get(&pdev->dev, "capacity_raw");
	if (IS_ERR(info->capacity_raw_chan)) {
		rc = PTR_ERR(info->capacity_raw_chan);
		if (rc != -EPROBE_DEFER)
			xm_err("capacity_raw channel unavailable, rc=%d\n", rc);
		info->capacity_raw_chan = NULL;
		return rc;
	}

	info->backlight_status_chan = iio_channel_get(&pdev->dev, "lc_screen_status");
	if (IS_ERR(info->backlight_status_chan)) {
		rc = PTR_ERR(info->backlight_status_chan);
		if (rc != -EPROBE_DEFER)
			xm_err("backlight_status channel unavailable, rc=%d\n", rc);
		info->backlight_status_chan = NULL;
		return rc;
	}

        info->real_type_chan = iio_channel_get(&pdev->dev, "usb_real_type");
	if (IS_ERR(info->real_type_chan)) {
		rc = PTR_ERR(info->real_type_chan);
		if (rc != -EPROBE_DEFER)
			xm_err("real_type channel unavailable, rc=%d\n", rc);
		info->real_type_chan = NULL;
		return rc;
	}

        if(!IS_ERR(iio_channel_get(&pdev->dev, "sc_charging_enabled"))) {
                info->cp_enabled_chan = iio_channel_get(&pdev->dev, "sc_charging_enabled");
        }
        else if (!IS_ERR(iio_channel_get(&pdev->dev, "bq_charging_enabled"))) {
                info->cp_enabled_chan = iio_channel_get(&pdev->dev, "bq_charging_enabled");
        }
        else {
                rc = PTR_ERR(info->cp_enabled_chan);
                if (rc != -EPROBE_DEFER)
                        xm_err("cp_enabled channel unavailable, rc=%d\n", rc);
                info->cp_enabled_chan = NULL;
                return rc;
        }

        info->iio_chan = devm_kcalloc(info->dev, xm_smart_chg_num_iio_channels,
				sizeof(*info->iio_chan), GFP_KERNEL);
	if (!info->iio_chan){
                xm_err("failed to alloc memory for iio_chan_spec, rc=%d\n", rc);
		return -ENOMEM;
        }

        info->int_iio_chans = devm_kcalloc(info->dev,
				xm_smart_chg_num_iio_channels,
				sizeof(*info->int_iio_chans),
				GFP_KERNEL);
	if (!info->int_iio_chans){
                xm_err("failed to alloc memory for iio_channel, rc=%d\n", rc);
		return -ENOMEM;
        }

        indio_dev->info = &xm_smart_chg_iio_info;
	indio_dev->dev.parent = info->dev;
	indio_dev->dev.of_node = info->dev->of_node;
	indio_dev->name = "xm_smart_chg";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = info->iio_chan;
	indio_dev->num_channels = xm_smart_chg_num_iio_channels;

        for (i = 0; i < xm_smart_chg_num_iio_channels; i++) {
		info->int_iio_chans[i].indio_dev = indio_dev;
		chan = &info->iio_chan[i];
		info->int_iio_chans[i].channel = chan;
		chan->address = i;
		chan->channel = xm_smart_chg_iio_channels[i].channel_num;
		chan->type = xm_smart_chg_iio_channels[i].type;
		chan->datasheet_name =
			xm_smart_chg_iio_channels[i].datasheet_name;
		chan->extend_name =
			xm_smart_chg_iio_channels[i].datasheet_name;
		chan->info_mask_separate =
			xm_smart_chg_iio_channels[i].info_mask;
	}

        rc = devm_iio_device_register(info->dev, indio_dev);
	if (rc)
		xm_err("failed to register xm_smart_chg IIO device, rc=%d\n", rc);
        else
                xm_err("success to register xm_smart_chg IIO device, rc=%d\n", rc);
        return rc;
}

/******************* Module driver ***********************************/
static int xm_smart_chg_probe(struct platform_device *pdev)
{
        int ret = 0;

        ret = xm_smart_chg_info_init(pdev);
        if(ret < 0){
                xm_err("xm_smart_chg_info_init failed, xm_smart_chg driver exit\n");
                return ret;
        }
        info->dev = &pdev->dev;

        ret = xm_smart_chg_parse_dt(pdev);
        if(ret < 0){
                xm_err("xm_smart_chg_parse_dt failed, xm_smart_chg driver exit\n");
                return ret;
        }

        ret = xm_smart_chg_init_iio(pdev);
        if(ret < 0){
                xm_err("xm_smart_chg_init_iio failed, xm_smart_chg driver exit\n");
                return -EPROBE_DEFER;
        }else{
                xm_err("xm_smart_chg_init_iio success\n");
        }

        ret = xm_smart_chg_post_init();
        if(ret < 0){
                xm_err("xm_smart_chg_post_init failed, xm_smart_chg driver exit\n");
                return ret;
        }else{
                xm_err("xm_smart_chg_post_init success\n");
        }

        schedule_delayed_work(&info->xm_charge_work, msecs_to_jiffies(XM_CHARGE_WORK_MS*3));
        info->batt_psy = power_supply_get_by_name("battery");
        if (!info->batt_psy)
                xm_err("Failed to get battery psy\n");

        info->outdoor_flag = -1;
        xm_err("Successful\n");
	return ret;
}

static int xm_smart_chg_remove(struct platform_device *pdev)
{
	xm_err("++\n");
	return 0;
}

static const struct of_device_id xm_smart_chg_of_ids[] = {
	{
		.compatible = "xiaomi,xm_smart_chg",
	},
	{},
};
MODULE_DEVICE_TABLE(of, xm_smart_chg_of_ids);
static struct platform_driver xm_smart_chg_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "xm_smart_chg",
                   .of_match_table = xm_smart_chg_of_ids,
	},
	.probe = xm_smart_chg_probe,
	.remove = xm_smart_chg_remove,
};

/******************* Module Init ***********************************/
static int __init xm_smart_chg_init(void)
{
        return platform_driver_register(&xm_smart_chg_driver);
}
static void __exit xm_smart_chg_exit(void)
{
        struct power_supply *batt_psy;
        cancel_delayed_work(&info->xm_charge_work);
        cancel_delayed_work(&info->bat_protect_work);
        if (prim_panel && !IS_ERR(cookie)) {
		panel_event_notifier_unregister(cookie);
	} else {
		xm_err("prim_panel_event_notifier_unregister falt\n");
	}
        batt_psy = power_supply_get_by_name("battery");
        if(!batt_psy){
                xm_err("failed to get batt_psy");
                return;
        }
        sysfs_remove_group(&batt_psy->dev.kobj, &batt_psy_attrs_group);
        platform_driver_unregister(&xm_smart_chg_driver);
}
late_initcall(xm_smart_chg_init);
module_exit(xm_smart_chg_exit);
MODULE_DESCRIPTION("XM SMART CHARGE Driver");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: qpnp-smb5-main");
