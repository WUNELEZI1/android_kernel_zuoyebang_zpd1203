#ifndef __XR_COUL_H__
#define __XR_COUL_H__

/* ===========include begin=========== */
#include <linux/types.h>
#include <linux/ioctl.h>

/* ===========include end=========== */


/* ===========define begin=========== */
#define NV_MAX_TEMPS_SIZE       16
#define MAX_LUT_SIZE            32
#define MAX_PC_SIZE             32
#define MAX_TEMP_SIZE           8
#define MAX_COEF_ROW_SIZE       32
#define MAX_COEF_COL_SIZE       16
#define MAX_IAVG_ARR_NUM        10
#define MAX_SOC_ARR_NUM         3
#define SR_ARR_LEN              100

/* ioctl cmd list */
#define COUL_GET_K2U_DATA_BEGIN _IOWR('C', 0x01, int) //get k2u data start 0:common data 1:k2u data
#define COUL_GET_K2U_DATA_END _IOWR('C', 0x02, int) //get k2u data end, notify kernel read data ok
#define COUL_GET_CC _IOWR('C', 0x03, int) //get_cc
#define COUL_SET_CC _IOWR('C', 0x04, int) //set_cc
#define COUL_RVAL2UV _IOWR('C', 0x05, int) //ocv_regval2uv
#define COUL_SET_OCV _IOWR('C', 0x06, int) //set_ocv
#define COUL_GET_OCV _IOWR('C', 0x07, int) //get_ocv
#define COUL_CLR_OCV _IO('C', 0x08) //clear_ocv
#define COUL_SET_OCV_TEMP _IOWR('C', 0x09, int) //set_ocv_temp
#define COUL_GET_OCV_TEMP _IOWR('C', 0x0a, int) //get_ocv_temp
#define COUL_SET_LOW_VOL_TH _IOWR('C', 0x0b, int) //set_low_vol_alert_thr
#define COUL_GET_TIME _IOWR('C', 0x0c, long long) //get_coul_time
#define COUL_CLR_TIME _IO('C', 0x0d) //clear_coul_time
#define COUL_CLR_CC _IOWR('C', 0x0e, int) //clear_cc
#define COUL_GET_VOL _IOWR('C', 0x0f, int) //get_vol
#define COUL_GET_CUR _IOWR('C', 0x10, int) //get_cur
#define COUL_GET_VOL_FIFO _IOWR('C', 0x11, int *) //get_vol_fifo
#define COUL_GET_CUR_FIFO _IOWR('C', 0x12, int *) //get_cur_fifo
#define COUL_GET_LP_CC _IOWR('C', 0x13, int) //get_lp_compensate_cc
#define COUL_SET_OCV_LVL _IOWR('C', 0x14, int) //set_ocv_level
#define COUL_GET_OCV_LVL _IOWR('C', 0x15, int) //get_ocv_level
#define COUL_GET_TEMP _IOWR('C', 0x16, int) //get_temp
#define COUL_GET_OCV_TYPE _IOWR('C', 0x17, int) //get_ocv_type
#define COUL_GET_FCC_UPDATE_F _IOWR('C', 0x18, int) //get_ocv_type
#define COUL_SET_FCC_UPDATE_F _IOWR('C', 0x19, int) //set_ocv_type
/* ===========define end=========== */


/* ===========enum begin=========== */
enum xr_coul_k2u_event_type {
	XR_COUL_K2U_EVENT_INIT = 0,
	XR_COUL_K2U_EVENT_RESUME,
	XR_COUL_K2U_EVENT_SOC,
	XR_COUL_K2U_EVENT_CHG_START,
	XR_COUL_K2U_EVENT_CHG_STOP,
	XR_COUL_K2U_EVENT_CHG_FULL,
};

enum bat_board_type {
	BAT_BOARD_FPGA = 0,
	BAT_BOARD_UDP,
	BAT_BOARD_PRODUCT,
	BAT_BOARD_MAX,
};

enum charging_state_type {
	CHARGING_STATE_UNKNOW = 0,
	CHARGING_STATE_START,
	CHARGING_STATE_STOP,
	CHARGING_STATE_FULL,
};

enum avg_curr_level {
	AVG_CURR_250MA = 0,
	AVG_CURR_500MA,
	AVG_CURR_MAX,
};

/* ===========enum end=========== */


/* ===========struct begin=========== */

struct linear_lut {
	int x[MAX_LUT_SIZE];
	int y[MAX_LUT_SIZE];
	int cols;
};

struct pc_temp_ocv_lut {
	int rows;
	int cols;
	int temp[MAX_TEMP_SIZE]; //ascending
	int percent[MAX_PC_SIZE]; //descending
	int ocv[MAX_PC_SIZE][MAX_TEMP_SIZE]; //descending
};

struct coef_lut {
	int rows;
	int cols;
	int row_entries[MAX_COEF_COL_SIZE];
	int percent[MAX_COEF_ROW_SIZE];
	int coef[MAX_COEF_ROW_SIZE][MAX_COEF_COL_SIZE];
};

struct coul_battery_data {
	struct linear_lut fcc_temp_lut; /* x:temperature y:fcc */
	struct linear_lut fcc_coef_lut; /* x:charge cycle y:fcc coefficients(percentage) */
	struct pc_temp_ocv_lut pc_temp_ocv_lut;
	struct coef_lut pc_coef_lut;
	struct coef_lut rbatt_coef_lut;
	int default_rbatt_mohm;
	int delta_rbatt_mohm;
	int fcc;
	unsigned int vbatt_max;
	unsigned int vbatt_min;
	unsigned int low_temp_opt_flag;
	unsigned int vbatt_low_temp_soc_three;
};

/* used to calculate latest 10min average curr for multi ocv check */
struct avg_curr2update_ocv {
	int current_ma;
	long long time;
};

struct coul_nv_info {
	int charge_cycles;
	int limit_fcc;
	short qmax;
	short temp[NV_MAX_TEMPS_SIZE];
	short fcc[NV_MAX_TEMPS_SIZE];
	short ocv_voltage_onboot;
	short ocv_current_onboot;
};

struct full_charge_ocv_para {
	int full_cnt;
	int last_cc;
	long long last_cc_time;
	long long ocv_time;
};

struct coul_common_data {
	int event_type; /* add for event from kernel to user */
	int batt_ocv;
	int batt_temp;
	int qmax;
	int batt_fcc;
	int batt_ruc;
	int batt_limit_fcc;
	int pre_charging_state;
	int charging_state;
	int batt_soc;
	int soc_init;
	int ui_soc;
	long long charging_begin_time;
	long long charging_stop_time;
	int is_nv_need_save;
	int batt_changed_flag;
	int soc_limit_flag;
	int batt_delta_rc;
	int batt_pre_delta_rc;
	int rbatt;
	int charging_begin_soc;
	int charging_stop_soc;
	int charging_begin_cc;
	int soc_work_interval;
	int rbatt_ratio;
	int last_fifo_iavg_ma;
	int prev_pc_unusable;
	int batt_rm;
	int batt_soc_real;
	int last_cc;
	long long last_cc_time;
	int batt_soc_est;
	int ready_to_refresh_fcc;
	int batt_ocv_temp;
	int fcc_real_mah;
	int suspend_cc;

	/* calculate average current */
	int iavg_arr[MAX_IAVG_ARR_NUM];
	int iavg_index;
	int iavg_count;

	/* smoothing soc */
	int soc_arr[MAX_SOC_ARR_NUM];
	int soc_index;

	/* calculate sleep/wakeup time ratio */
	long long sr_suspend_time;
	long long sr_resume_time;
	long long sr_sleep_time[SR_ARR_LEN];
	long long sr_wakeup_time[SR_ARR_LEN];
	int sr_sleep_index;
	int sr_wakeup_index;

	int last_ocv_level;
	int charge_cycles;
	struct avg_curr2update_ocv curr2update_ocv[AVG_CURR_MAX];
	struct full_charge_ocv_para full_chg_data;
	struct coul_nv_info nv_info;
};

struct coul_k2u_data {
	int batt_exist;
	int fifo_depth;
	int last_powerdown_soc;
	int multi_ocv_enabled;
	int uuc_zero_vol;
	int soc_at_term;
	int r_pcb;
	struct coul_battery_data batt_data;
};

/* ===========struct end=========== */

/* ===========global begin=========== */
/* ===========global end=========== */


/* ===========func begin=========== */
/* ===========func end=========== */

#endif /* __XR_COUL_H__ */

