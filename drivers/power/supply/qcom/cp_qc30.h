/*
 * cp_qc30.h
 *
 */

#ifndef SRC_PDLIB_USB_PD_POLICY_MANAGER_H_
#define SRC_PDLIB_USB_PD_POLICY_MANAGER_H_
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

typedef enum  {
	CP_STATE_ENTRY,
	CP_STATE_DISCONNECT,
	CP_STATE_SW_ENTRY,
	CP_STATE_SW_ENTRY_2,
	CP_STATE_SW_LOOP,
	CP_STATE_FLASH2_ENTRY,
	CP_STATE_FLASH2_ENTRY_1,
	CP_STATE_FLASH2_ENTRY_3,
	CP_STATE_FLASH2_TUNE,
	CP_STATE_FLASH2_DELAY,
	CP_STATE_STOP_CHARGE,
} pm_sm_state_t;

#define	BAT_OVP_FAULT_SHIFT			0
#define	BAT_OCP_FAULT_SHIFT			1
#define	BUS_OVP_FAULT_SHIFT			2
#define	BUS_OCP_FAULT_SHIFT			3
#define	BAT_THERM_FAULT_SHIFT			4
#define	BUS_THERM_FAULT_SHIFT			5
#define	DIE_THERM_FAULT_SHIFT			6

#define	BAT_OVP_FAULT_MASK		(1 << BAT_OVP_FAULT_SHIFT)
#define	BAT_OCP_FAULT_MASK		(1 << BAT_OCP_FAULT_SHIFT)
#define	BUS_OVP_FAULT_MASK		(1 << BUS_OVP_FAULT_SHIFT)
#define	BUS_OCP_FAULT_MASK		(1 << BUS_OCP_FAULT_SHIFT)
#define	BAT_THERM_FAULT_MASK		(1 << BAT_THERM_FAULT_SHIFT)
#define	BUS_THERM_FAULT_MASK		(1 << BUS_THERM_FAULT_SHIFT)
#define	DIE_THERM_FAULT_MASK		(1 << DIE_THERM_FAULT_SHIFT)

#define	BAT_OVP_ALARM_SHIFT			0
#define	BAT_OCP_ALARM_SHIFT			1
#define	BUS_OVP_ALARM_SHIFT			2
#define	BUS_OCP_ALARM_SHIFT			3
#define	BAT_THERM_ALARM_SHIFT			4
#define	BUS_THERM_ALARM_SHIFT			5
#define	DIE_THERM_ALARM_SHIFT			6
#define BAT_UCP_ALARM_SHIFT			7

#define	BAT_OVP_ALARM_MASK		(1 << BAT_OVP_ALARM_SHIFT)
#define	BAT_OCP_ALARM_MASK		(1 << BAT_OCP_ALARM_SHIFT)
#define	BUS_OVP_ALARM_MASK		(1 << BUS_OVP_ALARM_SHIFT)
#define	BUS_OCP_ALARM_MASK		(1 << BUS_OCP_ALARM_SHIFT)
#define	BAT_THERM_ALARM_MASK		(1 << BAT_THERM_ALARM_SHIFT)
#define	BUS_THERM_ALARM_MASK		(1 << BUS_THERM_ALARM_SHIFT)
#define	DIE_THERM_ALARM_MASK		(1 << DIE_THERM_ALARM_SHIFT)
#define	BAT_UCP_ALARM_MASK		(1 << BAT_UCP_ALARM_SHIFT)

#define VBAT_REG_STATUS_SHIFT			0
#define IBAT_REG_STATUS_SHIFT			1

#define VBAT_REG_STATUS_MASK		(1 << VBAT_REG_STATUS_SHIFT)
#define IBAT_REG_STATUS_MASK		(1 << VBAT_REG_STATUS_SHIFT)

enum qc_hvdcp3_type {
	QC_HVDCP3_NONE = 0,
	QC_HVDCP3_CLASSA_18W,
	QC_HVDCP3_CLASSB_27W,
};

#define MAIN_CHG_VOTER                  "MAIN_CHG_VOTER"
#define QC3_MAIN_CHARGER_ICL            2000000
#define QC3_CHARGER_ICL                 500000

#define QC3P5_BQ_TAPER_FCC_VOTER       "QC3P5_BQ_TAPER_FCC_VOTER"
#define QC3P5_BQ_TAPER_HYS_MV                   30
#define QC3P5_BQ_TAPER_DECREASE_STEP_MA         200

#define HVDCP3_CLASS_B_BAT_CURRENT_MA           5400
#define HVDCP3_CLASS_B_BUS_CURRENT_MA           2700
#define HVDCP3_CLASS_A_BAT_CURRENT_MA           3600
#define HVDCP3_CLASS_A_BUS_CURRENT_MA           2100
#define HVDCP3P5_BUS_CURRENT_MA                 2400
#define HVDCP3P5_BAT_CURRENT_MA                 4500
#define HVDCP3_TAPER_HYS_MV                     40
#define MAX_THERMAL_LEVEL                       13
#define HVDCP3P5_BAT_VOLT_MV                    4100
/* jeita related */
#define JEITA_WARM_THR                          450
#define JEITA_COOL_NOT_ALLOW_CP_THR             100
/*
 * add hysteresis for warm threshold to avoid flash
 * charge and normal charge switch frequently at
 * the warm threshold
 */
#define JEITA_HYSTERESIS                        20
#define HIGH_CAPACITY_TRH                       90


struct flash2_policy {
	int down_steps;
	uint16_t volt_hysteresis;
};

struct charge_pump {
	bool charge_enabled;

	bool batt_pres;
	bool vbus_pres;

	/* alarm/fault status */
	bool bat_ovp_fault;
	bool bat_ocp_fault;
	bool bus_ovp_fault;
	bool bus_ocp_fault;

	bool bat_ovp_alarm;
	bool bat_ocp_alarm;
	bool bus_ovp_alarm;
	bool bus_ocp_alarm;

	bool bat_ucp_alarm;

	bool bat_therm_alarm;
	bool bus_therm_alarm;
	bool die_therm_alarm;

	bool bat_therm_fault;
	bool bus_therm_fault;
	bool die_therm_fault;

	bool therm_shutdown_flag;
	bool therm_shutdown_stat;

	bool vbat_reg;
	bool ibat_reg;

	int  vout_volt;
	int  vbat_volt;
	int  vbus_volt;
	int  ibat_curr;
	int  ibus_curr;

	int  bat_temp;
	int  bus_temp;
	int  die_temp;
};

struct sw_charger {
	bool charge_enabled;
	bool charge_limited;
};

#define PM_STATE_LOG_MAX    32
typedef struct {
	bool        sw_is_charging;
	bool        flash2_is_charging;
	bool        sw_near_cv;
	bool		sw_fc2_init_fail;
	bool		bms_fastcharge_mode;

	uint16_t		ibus_lmt_curr;
	pm_sm_state_t     state; //state machine
	pm_sm_state_t state_log[PM_STATE_LOG_MAX];
	uint8_t     log_idx;

	int			usb_type;
	int			hvdcp3_type;
	int			usb_present;
	int 		usb_online;
	int			ibat_now;
	int			vbat_volt;
	int			capacity;
	struct charge_pump	 charge_pump;
	struct sw_charger	 sw_chager;
	struct votable		 *fcc_votable;
	struct votable		 *usb_icl_votable;

	struct power_supply *fc_psy;
	struct power_supply *sw_psy;
	struct power_supply *usb_psy;
	/* jeita or thermal related */
	bool			jeita_triggered;
	bool			is_temp_out_fc2_range;
} pm_t;


struct sys_config {
	uint16_t bat_volt_lp_lmt; /*bat volt loop limit*/
	uint16_t ffc_bat_volt_lmt;
	uint16_t bat_curr_lp_lmt;
	uint16_t bus_volt_lp_lmt;
	uint16_t bus_curr_lp_lmt;
	uint16_t ibus_plus_deviation_val;
	uint16_t ibus_minus_deviation_val;
	uint16_t ibat_plus_deviation_val;
	uint16_t ibat_minus_deviation_val;
	int32_t fc2_taper_current;

	struct flash2_policy flash2_policy;

	uint16_t min_vbat_start_flash2;
	bool	cp_sec_enable;
};

#endif /* CP_QC30_H_ */
