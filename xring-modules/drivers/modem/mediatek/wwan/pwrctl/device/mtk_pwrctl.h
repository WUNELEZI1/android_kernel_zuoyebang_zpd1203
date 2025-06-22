// SPDX-License-Identifier: GPL-2.0 only.
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __MTK_PWRCTL_H_
#define __MTK_PWRCTL_H_

/* Power control command magic number */
#define PWRCTL_CMD_MAGIC		'G'
/* Power control command through ioctl of /dev/wwan_pwrctl */
#define PWRCTL_CMD_POWER_ON                     _IOW(PWRCTL_CMD_MAGIC, 0, int)
#define PWRCTL_CMD_POWER_OFF                    _IOW(PWRCTL_CMD_MAGIC, 1, int)
#define PWRCTL_CMD_WARM_RESET                   _IOW(PWRCTL_CMD_MAGIC, 2, int)
#define PWRCTL_CMD_COLD_RESET                   _IOW(PWRCTL_CMD_MAGIC, 3, int)
#define PWRCTL_CMD_TRIGGER_EXCEPTION            _IOW(PWRCTL_CMD_MAGIC, 5, int)

#define PWRCTL_CMD_POWERUP                      _IOW(PWRCTL_CMD_MAGIC, 10, int)
#define PWRCTL_CMD_MDR_TRIGGER_EXCEPTION        _IOW(PWRCTL_CMD_MAGIC, 11, int)
#define PWRCTL_CMD_NOTIFY_SENSORHUB_READY       _IOW(PWRCTL_CMD_MAGIC, 12, int)
#define PWRCTL_CMD_NOTIFY_SENSORHUB_EXCEPTION   _IOW(PWRCTL_CMD_MAGIC, 13, int)
/* Event ID for power status notification.
 * PWRCTL_EVT_RESET: indicate the devcie is reset.
 * PWRCTL_EVT_PWRON: indicate the device is powered on.
 * PWRCTL_EVT_PWROFF: indicate the devcie is powered off.
 * PWRCTL_EVT_PRERST: indicate the device is waiting host give a reset pulse.
 */
enum pwrctl_evt {
	PWRCTL_EVT_RESET = 0,
	PWRCTL_EVT_PWRON,
	PWRCTL_EVT_PWROFF,
	PWRCTL_EVT_PRERST,
	PWRCTL_EVT_MAXNUM = PWRCTL_EVT_PRERST
};

enum pwrctl_flags {
	PWRCTL_SKIP_SUSPEND_OFF,
	PWRCTL_WARM_RESET,
	PWRCTL_SOFT_OFF_FAIL,
	PWRCTL_PMIC_FAULTB,
	PWRCTL_FLAG_MAXNUM = PWRCTL_PMIC_FAULTB
};

/* struct pwrctl_gpio - The GPIO description.
 * @pmic_en: The pin connects to PMIC_EN, for power on/off device.
 * @mode_ctrl: The pin connects to PMIC_RST, for warm reset or soft off.
 * @reboot: The pin connects to REBOOT_INT interrupt.
 * @modem_dump: The pin is used to trigger modem dump.
 * @faultb: The pin is used to monitor FAULTB.
 */
struct pwrctl_gpio {
	int pmic_en;
	int mode_ctrl;
	int reboot;
	int modem_dump;
	int faultb;
	int pcie_wake;
};

struct pwrctl_irq {
	int reboot;
	int faultb;
	int pcie_wake;
};

struct pwrctl_dev_mngr {
	struct pwrctl_mdev *mdev;
	struct device *dev;
	struct pwrctl_gpio gpio;
	struct pwrctl_irq irq;
	struct mutex op_lock;
	struct work_struct eint_work;
	struct work_struct faultb_work;
	struct work_struct pcie_wake_work;
};

/* Founction for register event notify callback for host PCIe (EP)driver */
int mtk_pwrctl_event_register_callback( void (*cb)(enum pwrctl_evt , void *),
					void *data);
int mtk_pwrctl_event_unregister_callback(void (*cb)(enum pwrctl_evt , void *));

/*
 * Function for setting suspend skip flag, called by host PCIe (EP) driver if
 * can't power off device.
 * @flag: 1: don't power off device when suspend.
 *        0: power off device when suspend.
 */
int mtk_pwrctl_set_suspend_skip(bool flag);

/* Function for warm reset device with sysrst of PMIC_RST*/
int mtk_pwrctl_fldr(void);

/* Function for code reset device device */
int mtk_pwrctl_pldr(void);

/* for debug */
int mtk_pwrctl_remove_dev(void);
#endif //__MTK_PWRCTL_H_
