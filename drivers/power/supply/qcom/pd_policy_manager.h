#ifndef SRC_PDLIB_USB_PD_POLICY_MANAGER_H_
#define SRC_PDLIB_USB_PD_POLICY_MANAGER_H_

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/usb/usbpd.h>
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
#include "smb5-lib.h"
#endif

#define TAPER_FCC_VOTER                 "TAPER_FCC_VOTER"
#define JEITA_FCC_TAPER_VOTER           "JEITA_FCC_TAPER_VOTER"
#define PD_FFC_TAPER_MAX_CURRENT        3000000  //uA
#define MAX_THERMAL_LEVEL               13
#define JEITA_HYSTERESIS                20
#define JEITA_WARM_THR                  450
#define JEITA_COOL_NOT_ALLOW_CP_THR     100
#define COOL_HYS_THRESHOLDS             180
#define WARM_HYS_THRESHOLDS             450
#define PDO_MAX_NUM                     7
#define LOW_POWER_PPS_CURR_THR          2000
#define XIAOMI_LOW_POWER_PPS_CURR_MAX   1500
#define MAX_POWER_PPS_CURR_THR          3000  //mA
#define STEP_MV                         20
#define TAPER_VOL_HYS                   20
#define TAPER_HYS_MV                    40
#define TAPER_DECREASE_STEP_MA          100
#define QUICK_RAISE_VOLT_INTERVAL_S     10 * MSEC_PER_SEC
#define TAPER_TIMEOUT                   3
#define IBUS_CHANGE_TIMEOUT             2
#define TUNE_VBUS_RETRY_MAX             33
#define PD_SRC_PDO_TYPE_AUGMENTED       3
#define BATT_MAX_CHG_VOLT               4650 //mV
#define BATT_FAST_CHG_CURR              6000
#define BAT_VOLT_LOOP_LMT               BATT_MAX_CHG_VOLT
#define BAT_CURR_LOOP_LMT               BATT_FAST_CHG_CURR
#define BATT_WARM_CHG_VOLT              4100
#define BUS_VOLT_INIT_UP                400
#define PM_WORK_RUN_FAST                200
#define PM_WORK_RUN_STABLE              500
#define IBUS_RISE_HYS_MA                100
#define ITERM_RISE_HYS_MA               400

struct usbpd_pdo {
	bool pps;
	int type;
	int max_volt_mv;
	int min_volt_mv;
	int curr_ma;
	int pos;
};

enum pm_state {
	PD_PM_STATE_ENTRY,
	PD_PM_STATE_FC2_ENTRY,
	PD_PM_STATE_FC2_ENTRY_1,
	PD_PM_STATE_FC2_ENTRY_2,
	PD_PM_STATE_FC2_ENTRY_3,
	PD_PM_STATE_FC2_TUNE,
#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	PD_PM_STATE_HOLD,
#endif
	PD_PM_STATE_FC2_EXIT,
};

struct usbpd_pm {
	struct device *dev;
	enum pm_state state;

	struct work_struct cp_psy_change_work;
	struct work_struct usb_psy_change_work;
	struct delayed_work pm_work;
	struct notifier_block nb;
	struct votable *fcc_votable;
	struct power_supply *batt_psy;
	struct power_supply *fg_psy;
	struct power_supply *usb_psy;
	struct iio_channel **cp_iio;
	struct iio_channel **smb_iio;

	int vbat;
	int ibat;
	int iterm;//mA
	int fg_cv;
	int fg_vbat;
	int fg_rsoc;
	int vbus;
	int ibus;
	int soc;
	int batt_temp;
	int thermal_level;
	int request_vbus;
	int request_ibus;
	int select_fix_pdo;

	bool jeita_triggered;
	bool is_temp_out_fc2_range;
	bool cool_warm_done;
	bool cp_charging;
	bool sw_charging;
	bool pd_active;
	bool pps_supported;
	bool psy_change_running;
	bool bq25960_flag;
	int pm_work_time_ms;
	spinlock_t psy_change_lock;
	ktime_t entry_cv_time;

	struct usbpd *pd;
	struct usbpd_pdo pdo[7];
	int src_cap_id;
	int apdo_max_volt;
	int apdo_max_curr;
	int apdo_selected_pdo;

#if IS_ENABLED(CONFIG_XIAOMI_SMART_CHG)
	struct votable	*chg_disable;
	bool	ctrl_flag;
#endif
	struct votable  *pdo_5v_votable;
};

struct pdpm_config {
	int bat_volt_lp_lmt; /*bat volt loop limit*/
	int bat_curr_lp_lmt;
	int fc2_taper_current;
	int fc2_steps;
	int min_adapter_volt_required;
	int min_adapter_curr_required;
	int min_vbat_for_cp;
};

enum pd_select_pdo_flag {
    PD_SELECT_DEFAULT,
    PD_SELECT_FIXED_9V,
    PD_SELECT_FIXED_5V,
    PD_SELECT_MAX,
};

extern int usbpd_fetch_pdo(struct usbpd *pd, struct usbpd_pdo *pdos);
extern int usbpd_select_pdo(struct usbpd *pd, int pdo, int uv, int ua);
extern int usbpd_get_src_cap_id(struct usbpd *pd);
extern struct usbpd *usbpd_get_pd_lobal(void);

#endif /* SRC_PDLIB_USB_PD_POLICY_MANAGER_H_ */
