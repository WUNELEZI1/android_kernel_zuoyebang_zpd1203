#ifndef LC_CHARGER_SYSFS_H
#define LC_CHARGER_SYSFS_H

#include <linux/init.h>		/* For init/exit macros */
#include <linux/module.h>	/* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/pm_wakeup.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/scatterlist.h>
#include <linux/suspend.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/reboot.h>

#include <asm/setup.h>
#include "mtk_charger.h"
#include "mtk_battery.h"
#include "tcpci_typec.h"
#include "adapter_class.h"

/*add for SW_CID start*/
#include "tcpci_core.h"
#include "../../gpu/drm/mediatek/mediatek_v2/mtk_panel_ext.h"
#include "../../gpu/drm/mediatek/mediatek_v2/mtk_disp_notify.h"
/*add for SW_CID end*/

static int log_level = 2;

#define sysfs_err(fmt, ...)     \
do {                                    \
    if (log_level > 0)                  \
        printk(KERN_ERR "[lc_charger_sysfs]" fmt, ##__VA_ARGS__);   \
}while (0)

#define sysfs_info(fmt, ...)    \
do {                                    \
    if (log_level > 1)          \
        printk(KERN_ERR "[lc_charger_sysfs]" fmt, ##__VA_ARGS__);    \
}while (0)

#define sysfs_dbg(fmt, ...)    \
do {                                        \
    if (log_level >=2 )          \
        printk(KERN_ERR "[lc_charger_sysfs]" fmt, ##__VA_ARGS__);    \
}while (0)

#define CQR_VBAT_MIM           3750000
#define CQR_VBAT_MAX           4300000
#define CQR_READ_VBAT_TIMES    5
#define CQR_VBAT_OUT_RANGE_CNT 2
#define SHUTDOWN_DELAY_VOL_MAX 3400
#define SHUTDOWN_DELAY_VOL_MIN 3300
#define MODEL_NAME "P15_6000mAh_33w"

struct sysfs_desc {
	struct mtk_charger *info;
	struct mtk_battery *gm;
	//struct mtk_battery_manager *bm;
	struct device dev;

	struct charger_device *chg_dev;
	struct charger_device *cp_dev;
	struct tcpc_device *tcpc;
	struct class lc_charger_class;

	struct power_supply *chg_psy;
	struct power_supply *bat_psy;
	struct power_supply *usb_psy;
	struct power_supply *bms_psy;
	struct power_supply *batt_verify_psy;

	struct notifier_block tcpc_nb;

	struct task_struct *soc_decimal_task;
	bool wakeup_thread;
	wait_queue_head_t wq;
	bool shutdown_delay;

	struct notifier_block psy_nb;

	/* Add for SW_CID start*/
	bool cid_enable;
	bool screen_on;
	bool audio_on;
	struct notifier_block disp_nb;
	bool typec_attached;
	struct delayed_work monitor_work;
	/* Add for SW_CID end*/

	bool ship_mode; /* ship mode */
	bool ata_test_flag;

	int real_type;
	int chr_type;

	int board_temp;
	struct power_supply_desc this_desc;
};

static struct sysfs_desc *g_desc;
int charger_dev_get_online(struct sysfs_desc *desc);

static const char *const real_type_name[] = {
	"Unknown", "USB", "USB_CDP", "USB_FLOAT",
	"USB_DCP", "USB_HVDCP", "USB_PD", "USB_PPS",
};

static const char *const typec_mode_name[] = {
	"Nothing attached", "Source attached", "Sink attached",
	"Audio Adapter", "Debug Accessory",
};

static const char *const batt_type_name[] = {
	"Unknown", "XM81_COS-6000mAh", "XM81_NVT-6000mAh", "XM81_SWD-6000mAh",
};

static const char * const batt_manufacturer_name[] = {
	"Unknown", "XM81_COS", "XM81_NVT", "XM81_SWD",
};

/*
static const char *const cp_vendor_name[] = {
	"Unknown", "sc8541", "nu2115",
};
*/
static const char *const resistance_id_name[] = {
	"UNKONWN", "68000", "100000", "330000",
};

enum chr_type {
	CHARGER_UNKNOWN = 0,
	STANDARD_HOST,		/* USB : 450mA */
	CHARGING_HOST,
	NONSTANDARD_CHARGER,	/* AC : 450mA~1A */
	STANDARD_CHARGER,	/* AC : ~1A */
	HVDCP_CHARGER,		/* AC: QC charger */
	PD_CAHRGER,		/* AC: PD charger */
	PPS_CAHRGER,		/* AC: PPS charger */
	APPLE_2_1A_CHARGER,	/* 2.1A apple charger */
	APPLE_1_0A_CHARGER,	/* 1A apple charger */
	APPLE_0_5A_CHARGER,	/* 0.5A apple charger */
	WIRELESS_CHARGER,
};

enum power_supply_quick_charge_type {
	QUICK_CHARGE_NORMAL = 0,		/* Charging Power <= 10W */
	QUICK_CHARGE_FAST,			/* 10W < Charging Power <= 20W */
	QUICK_CHARGE_FLASH,			/* 20W < Charging Power <= 30W */
	QUICK_CHARGE_TURBE,			/* 30W < Charging Power <= 50W */
	QUICK_CHARGE_SUPER,			/* Charging Power > 50W */
	QUICK_CHARGE_MAX,
};

int set_ship_mode(struct sysfs_desc *desc);
void set_input_suspend(struct sysfs_desc *desc, bool enable);
int lc_sysfs_get_charge_type(struct sysfs_desc *desc, int *real_chr_type, int *chr_type);
int screen_noti_for_cid(struct notifier_block *nb, unsigned long event, void *data);
int lc_charger_node_init(struct sysfs_desc *desc);

#endif /* LC_CHARGER_SYSFS_H */
