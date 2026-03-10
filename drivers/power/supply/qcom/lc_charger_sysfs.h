#include <linux/power_supply.h>
#include <linux/qti_power_supply.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <dt-bindings/iio/qti_power_supply_iio.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/pmic-voter.h>

//iio enable config
#define USE_LC_CHG_SYSFS_IIO

#define LC_CHG_TEST_FLAG    777
#define ICL_STORE           "ICL_STORE"

#ifdef  pr_info
#undef  pr_info
#endif

#define pr_info    pr_err

/* select the style for get prop */
enum lc_chg_sysfs_get_prop_style {
    LC_GET_PROP_NULL,
    LC_GET_PROP_QCOM_IIO,
    LC_GET_PROP_MTK_OPS,
};

enum typec_power_role {
	TYPEC_PR_NONE,		/* CC lines in high-Z */
	TYPEC_PR_DUAL,
	TYPEC_PR_SINK,
	TYPEC_PR_SOURCE,
};

struct lc_chg_sysfs {
    struct class            lc_chg_sysfs_class;
    struct power_supply     *psy;
    struct device           *dev;
    struct platform_device  *pdev;

#ifdef USE_LC_CHG_SYSFS_IIO
    struct charger_param    *chg_param;
    struct iio_dev          *indio_dev;
    struct iio_chan_spec    *iio_chan;
    struct iio_channel      *int_iio_chans;
    struct iio_channel      **fg_ext_iio_chans;
    struct iio_channel      **cp_ext_iio_chans;
    struct iio_channel      **main_chg_ext_iio_chans;
    struct iio_channel      **cc_ext_iio_chans;
    struct iio_channel      **ds_ext_iio_chans;
#endif

    int get_prop_style;    //select the style for get prop
    int test_flag;
    int cc_orientation;
    int ui_soc;
    int quick_charge_type;
    int soc_decimal;
    int soc_decimal_rate;
    int shutdown_delay;
    int usb_real_type;
    int input_suspend;
    int ship_mode;
    int mtbf_current;
    int typec_mode;
    bool backlight_status;
    bool audio_cctog;
    int use_shipcount_en;
    int cid_status;
};

static const char * const power_supply_usb_type_text[] = {
	"Unknown", "Battery", "UPS", "Mains", "USB",
	"USB_DCP", "USB_CDP", "USB_ACA", "USB_C",
	"USB_PD", "USB_PD_DRP", "BrickID", "Wireless",
	"USB_HVDCP", "USB_HVDCP_3", "USB_HVDCP_3P5", "USB_FLOAT",
	"USB_PPS",
};

static const char * const power_supply_usbc_text[] = {
    "Nothing attached", "Sink attached", "Powered cable w/ sink",
    "Debug Accessory", "Audio Adapter", "Powered cable w/o sink",
    "Source attached (default current)",
    "Source attached (medium current)",
    "Source attached (high current)",
    "Non compliant",
};

#ifdef USE_LC_CHG_SYSFS_IIO
#define SEC_IIO_CHANNEL_OFFSET     2
enum lc_chg_iio_type {
	LC_MAIN,
	LC_BMS,
	LC_CP_MASTER,
	LC_CC,
	LC_DS,
};

struct lc_chg_sysfs_iio_channels {
	const char *datasheet_name;
	int channel_num;
	enum iio_chan_type type;
	long info_mask;
};

#define LC_CHG_SYSFS_IIO_CHAN(_name, _num, _type, _mask)		\
	{						\
		.datasheet_name = _name,		\
		.channel_num = _num,			\
		.type = _type,				\
		.info_mask = _mask,			\
	},

#define LC_CHG_CHAN_ENERGY(_name, _num)			\
	LC_CHG_SYSFS_IIO_CHAN(_name, _num, IIO_CURRENT,		\
		BIT(IIO_CHAN_INFO_PROCESSED))
static const struct lc_chg_sysfs_iio_channels lc_chg_sysfs_iio_psy_channels[] = {
	LC_CHG_CHAN_ENERGY("lc_test", PSY_IIO_LC_TEST)
	LC_CHG_CHAN_ENERGY("lc_screen_status", PSY_IIO_LC_SCREEN_STA)
	LC_CHG_CHAN_ENERGY("cid_status", PSY_IIO_LC_CID_STA)
	LC_CHG_CHAN_ENERGY("set_ship_mode", PSY_IIO_SHIP_MODE)
	LC_CHG_CHAN_ENERGY("shipmode_count_reset", PSY_IIO_SHIPMODE_COUNT_RESET)
};

enum fg_ext_iio_channels {
    SOC_DECIMAL,
    SOC_DECIMAL_RATE,
    SHUTDOWN_DELAY,
};

static const char * const fg_ext_iio_chan_name[] = {
    [SOC_DECIMAL]      = "soc_decimal",
    [SOC_DECIMAL_RATE] = "soc_decimal_rate",
    [SHUTDOWN_DELAY]   = "shutdown_delay",
};


enum cc_ext_iio_channels {
        TYPEC_CC_ORIENTATION,
        TYPEC_MODE,
        TYPEC_POWER_ROLE,
        FORCE_SOURCE,
};

static const char * const cc_ext_iio_chan_name[] = {
        [TYPEC_CC_ORIENTATION] = "typec_cc_orientation",
        [TYPEC_MODE]           = "typec_mode",
        [TYPEC_POWER_ROLE]     = "typec_power_role",
        [FORCE_SOURCE]         = "force_source",
};

enum main_chg_ext_iio_channels {
        QUICK_CHARGE_TYPE,
        USB_REAL_TYPE,
        INPUT_SUSPEND,
        SET_SHIP_MODE,
        MTBF_CURRENT,
        APDO_CURR,
        APDO_VOLT,
        PD_ACTIVE,
        LPD_CONTROL,
        LPD_CHARGING,
};

static const char * const main_chg_ext_iio_chan_name[] = {
        [QUICK_CHARGE_TYPE] = "quick_charge_type",
        [USB_REAL_TYPE]     = "usb_real_type",
        [INPUT_SUSPEND]     = "input_suspend",
        [SET_SHIP_MODE]     = "set_ship_mode",
        [MTBF_CURRENT]      = "mtbf_current",
        [APDO_CURR]         = "apdo_curr",
        [APDO_VOLT]         = "apdo_volt",
        [PD_ACTIVE]         = "pd_active",
        [LPD_CONTROL]       = "lpd_control",
        [LPD_CHARGING]      = "lpd_charging",
};

#if 0
enum ds_ext_iio_channels {
};

static const char * const ds_ext_iio_chan_name[] = {
};
#endif

enum cp_ext_iio_channels {
        LC_CP_MASTER_SC_BUS_CURRENT,
        LC_CP_MASTER_SC_MANUFACTURER,
        LC_CP_MASTER_BQ_BUS_CURRENT,
        LC_CP_MASTER_BQ_MANUFACTURER,
};

static const char * const cp_ext_iio_chan_name[] = {
	[LC_CP_MASTER_SC_BUS_CURRENT] = "sc_bus_current",
	[LC_CP_MASTER_SC_MANUFACTURER] = "sc_manufacturer",
	[LC_CP_MASTER_BQ_BUS_CURRENT] = "bq_bus_current",
	[LC_CP_MASTER_BQ_MANUFACTURER] = "bq_manufacturer",
};

static const char * const batt_manufacturer_text[] = {
	"Unknown","XM30_COS", "XM30_NVT", "XM81_SWD", "XM81_NVT",
};

static const char * const battery_type_text[] = {
	"Unknown","XM30_COS-7000mAh", "XM30_NVT-7000mAh", "XM81_SWD-7000mAh", "XM81_NVT-7000mAh",
};

static const char * const cp_manufacturer_text[] = {
	"Unknown","sc8531", "bq25960H",
};

extern int lc_chg_ext_init_iio_psy(struct lc_chg_sysfs *lc_chg);
extern int lc_chg_init_iio_psy(struct lc_chg_sysfs *chip);
extern int lc_chg_sysfs_set_iio_channel(struct lc_chg_sysfs *chg, enum lc_chg_iio_type type, int channel, int val);
extern int lc_chg_sysfs_get_iio_channel(struct lc_chg_sysfs *chg, enum lc_chg_iio_type type, int channel, int *val);
#endif

extern int lc_get_uisoc(struct lc_chg_sysfs *lc_chg, int *val);
extern int lc_set_cid_status(struct lc_chg_sysfs *lc_chg);
extern int lc_get_ship_mode(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval);
extern int lc_set_ship_mode(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval);
extern int use_count_reset(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval);
extern int lc_get_pd_active(struct lc_chg_sysfs *lc_chg, union power_supply_propval *pval);