#ifndef __XM_SMART_CHG_H
#define __XM_SMART_CHG_H

#include <linux/err.h>
#include <linux/power_supply.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/kstrtox.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/pmic-voter.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/notifier.h>
#include <linux/soc/qcom/panel_event_notifier.h>
#include <drm/drm_panel.h>
#include <linux/time64.h>
#include <linux/qti_power_supply.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include "smb5-lib.h"
#include "battery.h"

#define XM_L_SOC_CHG_WORK_MS            1000
#define XM_BAT_PROTECT_MS               1000
#define XM_CHARGE_WORK_MS               5000
#define MAX_THERMAL_LEVELS              15
#define XM_PROTECT_DEC_CUR              1000000

#define SMART_BATT_VOTER                "SMART_BATT_VOTER"
#define CYCLE_COUNT_VOTER               "CYCLE_COUNT_VOTER"
#define NIGHT_CHARGING_VOTER            "NIGHT_CHARGING_VOTER"
#define SMART_CHARGING_VOTER            "SMART_CHARGING_VOTER"
#define LOW_FAST_VOTER                  "LOW_FAST_VOTER"
#define OUTDOOR_FCC_VOTER               "OUTDOOR_FCC_VOTER"
#define OUTDOOR_ICL_VOTER               "OUTDOOR_ICL_VOTER"
#define ENDURANCE_PRO_VOTER             "ENDURANCE_PRO_VOTER"
#define SMART_PROTECT                   "SMART_PROTECT"
#define SMART_INPUT_LIMIT               "SMART_INPUT_LIMIT"
#define FCC_STEPPER_VOTER               "FCC_STEPPER_VOTER"
#define SW_ICL_MAX_VOTER                "SW_ICL_MAX_VOTER"

enum smart_chg_functype{
    SMART_CHG_STATUS_FLAG = 0,
    SMART_CHG_FEATURE_MIN_NUM = 1,
    SMART_CHG_NAVIGATION = 1,
    SMART_CHG_OUTDOOR_CHARGE,
    SMART_CHG_LOW_FAST = 3,
    /* add new func here */
    SMART_CHG_ENDURANCE_PRO = 4,
    SMART_CHG_FEATURE_MAX_NUM = 15,
};

struct smart_chg {
    bool en_ret;
    int active_status;
    int func_val;
};

struct charger_screen_monitor {
    struct notifier_block charger_panel_notifier;
    int screen_state;
};

enum blank_flag{
    NORMAL = 0,
    BLACK_TO_BRIGHT = 1,
    BRIGHT = 2,
    BLACK = 3,
};

struct xm_smart_chg_info {
    struct device           *dev;
    int                     smart_batt;
    int                     fv_ageing;
    int                     real_type;
    bool                    night_charging;
    bool                    night_charging_flag;
    const char              *label;
    int			*pps_thermal_mitigation;
    int			*pps_thermal_mitigation_fast;
    int			thermal_levels;
    int                     screen_state;
    int			thermal_level;
    struct charger_screen_monitor    sm;
    struct votable		*fv_votable;
    struct votable		*fcc_votable;
    struct votable		*fcc_main_votable;
    struct votable		*chg_disable_votable;
    struct votable		*low_fast_votable;
    struct votable		*night_charging_votable;
    struct votable		*usb_icl_votable;
    struct smart_chg        smart_charge[SMART_CHG_FEATURE_MAX_NUM + 1];
    int         smart_chg_cmd;
    int         soc;
    int         raw_soc;
    int 	over_vbat_timer;
    int 	protect_done;
    bool        smart_ctrl_en;
    int                     thermal_board_temp;
    int                     pd_active;
    int                     first_flag;
    bool                    low_fast_plugin_flag;
    bool                    pps_fast_mode;
    enum blank_flag         b_flag;
    /*charger_plugin_event 0:none 1:plugin 2:plugout*/
    int                     charger_plugin_event;
    struct notifier_block   qpnp_smb5_nb;
    struct delayed_work     xm_charge_work;
    struct delayed_work     bat_protect_work;
    struct delayed_work     xm_smart_chg_post_init_work;

    struct power_supply *batt_psy;
    struct iio_channel	*capacity_chan;
    struct iio_channel	*pd_active_chan;
    struct iio_channel	*capacity_raw_chan;
    struct iio_channel	*backlight_status_chan;
    struct iio_channel  *real_type_chan;
    struct iio_channel  *cp_enabled_chan;
    /*contain: iio_info(ops)，iio_chan_spec(data channels)*/
    struct iio_dev		*indio_dev;
    struct iio_chan_spec	*iio_chan;
    struct iio_channel	*int_iio_chans;

    int                     intLabel;
    int                     outdoor_flag;
    bool                    endurance_protect_flag;
};

struct xm_smart_chg_iio_channels {
    const char *datasheet_name;
    int channel_num;
    enum iio_chan_type type;
    long info_mask;
};

#define XM_SMART_CHG_IIO_CHAN(_name, _num, _type, _mask)		\
    {						\
        .datasheet_name = _name,		\
        .channel_num = _num,			\
        .type = _type,				\
        .info_mask = _mask,			\
    },

#define XM_SMART_CHG_CHAN_ENERGY(_name, _num)			\
        XM_SMART_CHG_IIO_CHAN(_name, _num, IIO_ENERGY,		\
                BIT(IIO_CHAN_INFO_PROCESSED))

static const struct xm_smart_chg_iio_channels xm_smart_chg_iio_channels[] = {
    XM_SMART_CHG_CHAN_ENERGY("intLabel", PSY_IIO_INTLABEL)
};

static int cycle_count_conf[] = {
    100, 300, 800, 65535
};
static int dropfv_conf[] = {
    0, 10, 20, 40
};

void xm_charge_work(struct work_struct *work);
void xm_smart_chg_post_init_work(struct work_struct *work);
int xm_smart_chg_register_panel_notifier(void);

void set_error(struct xm_smart_chg_info *manager);
void set_success(struct xm_smart_chg_info *manager);
int smart_chg_is_error(struct xm_smart_chg_info *manager);
void handle_smart_chg_functype(struct xm_smart_chg_info *manager,
	const int func_type, const int en_ret, const int func_val);
int handle_smart_chg_functype_status(struct xm_smart_chg_info *manager);
void monitor_smart_chg(void);
#endif