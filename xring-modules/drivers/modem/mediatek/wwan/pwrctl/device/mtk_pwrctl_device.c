// SPDX-License-Identifier: GPL-2.0 only.
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/sched.h>
#include <linux/syscore_ops.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include "mtk_pwrctl.h"
#include "mtk_pwrctl_common.h"
#include "mtk_pwrctl_interface.h"
#ifdef CONFIG_WWAN_GPIO_PWRCTL_UT
#include "ut_pwrctl_t800_fake.h"
#endif
#include "mdm_mdr.h"
#include <soc/xring/sensorhub/shub_conn_ipc_if.h>

/* Uevent to user */
#define PWRCTL_UEVENT_POWER_ON		"power_on"
#define PWRCTL_UEVENT_POWER_OFF		"power_off"
#define PWRCTL_UEVENT_RESET		"reset"
/* GPIO pin name */
#define PWRCTL_GPIO_PMIC_EN		"pmic-en"
#define PWRCTL_GPIO_MODEM_CTRL		"pmic-rst"
#define PWRCTL_GPIO_REBOOT_INT		"reset-eint"
#define PWRCTL_GPIO_MODOME_DUMP		"md-dump"
#define PWRCTL_GPIO_FAULT_BAR		"pmic-faultb"
#define PWRCTL_GPIO_PCIE_WAKE		"pcie-wake"

#define PWRCTL_REBOOT_TRIGGER_MODE	(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING)
#define PWRCTL_PCIE_WAKE_TRIGGER_MODE	(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING)

static __attribute__ ((weakref("mtk_gpio_sap_ctrl_set_off"))) \
		       int mtk_gpio_set_mode_off(void);
static __attribute__ ((weakref("mtk_gpio_sap_ctrl_set_download"))) \
		       int mtk_gpio_set_mode_preboot(void);
static __attribute__ ((weakref("mtk_gpio_sap_ctrl_set_working"))) \
		       int mtk_gpio_set_mode_working(void);

#define GPIO_SET_MODE_OFF() do{ if (mtk_gpio_set_mode_off) mtk_gpio_set_mode_off(); } while(0)
#define GPIO_SET_MODE_PREBOOT() \
	do{ if (mtk_gpio_set_mode_preboot)  mtk_gpio_set_mode_preboot(); } while(0)
#define GPIO_SET_MODE_WORKING() \
	do{ if (mtk_gpio_set_mode_working) mtk_gpio_set_mode_working(); } while(0)

extern int xring_pcie_probe_port_by_port(int port);
extern int xring_pcie_remove_port_by_port(int port);
extern void xiaomi_modem_power_irq_unregister(void);
extern int xring_pcie_disable_data_trans(int port);

static struct pwrctl_dev_mngr * dev_mngr = NULL;
int xiaomi_pwrctl_power_on(struct pwrctl_mdev *mdev, int nt_rc)
{
	if (!dev_mngr) {
		dev_err(&mdev->pdev->dev, "[Power on] GPIO is not initialized\n");
		return -EINVAL;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return -EINVAL;
	}
	gpio_set_value(dev_mngr->gpio.pmic_en, 0);
	msleep(50);
	dev_info(&mdev->pdev->dev, "Powere on device  1\n");
	gpio_set_value(dev_mngr->gpio.pmic_en, 1);
	dev_info(&mdev->pdev->dev, "power on gpio  2\n");
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);
	msleep(50);
	GPIO_SET_MODE_PREBOOT();

	if (nt_rc)
		xring_pcie_probe_port_by_port(0);

	mutex_unlock(&dev_mngr->op_lock);
	dev_info(&mdev->pdev->dev, "power on device done\n");
	return 0;
}


static void mtk_pwrctl_clear_irq(int irq)
{
	struct irq_desc *desc;
	struct irq_chip *chip;

	desc = irq_to_desc(irq);
	if (!desc)
		return;

	chip = irq_desc_get_chip(desc);
	if (!chip)
		return;

	if (chip->irq_ack) {
		chip->irq_ack(&desc->irq_data);
		pr_info("pwrctl: Clear irq<%d> done\n", irq);
	}
}

void mtk_pwrctl_enable_irqs(void)
{
	mtk_pwrctl_clear_irq(dev_mngr->irq.reboot);
	enable_irq_wake(dev_mngr->irq.reboot);
	enable_irq(dev_mngr->irq.reboot);
}

void mtk_pwrctl_disable_irqs(void)
{
	disable_irq(dev_mngr->irq.reboot);
	disable_irq_wake(dev_mngr->irq.reboot);
}

static void mtk_pwrctl_event_notify(enum pwrctl_evt evt, const char *uevent)
{
	mtk_pwrctl_uevent_notify_user(uevent);

	mtk_pwrctl_cb_notify_user(evt);
}

int mtk_pwrctl_power_on(struct pwrctl_mdev *mdev, int nt_rc)
{
	if (!dev_mngr) {
		dev_err(&mdev->pdev->dev, "[Power on] GPIO is not initialized\n");
		return -EINVAL;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return -EINVAL;
	}
	dev_info(&mdev->pdev->dev, "Powere on device\n");
	gpio_set_value(dev_mngr->gpio.pmic_en, 1);
	dev_info(&mdev->pdev->dev, "power on gpio\n");
	msleep(50);
	GPIO_SET_MODE_PREBOOT();

	if (nt_rc)
		xring_pcie_probe_port_by_port(0);

	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);
	mutex_unlock(&dev_mngr->op_lock);
	dev_info(&mdev->pdev->dev, "power on device done\n");
	return 0;
}

int mtk_pwrctl_power_off(struct pwrctl_mdev *mdev, int nt_rc)
{
	if (!dev_mngr) {
		dev_err(&mdev->pdev->dev, "[Power off] GPIO is not initialized\n");
		return -EINVAL;
	}

	if (mtk_pwrctl_get_power_state(mdev) == PWRCTL_STATE_PWROFF){
		dev_info(&mdev->pdev->dev, "Device is powered off, ignore the repeated operation\n");
		return -EINVAL;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return -EINVAL;
	}
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWROFF);
	if (nt_rc)
		xring_pcie_remove_port_by_port(0);

    GPIO_SET_MODE_OFF();
	gpio_set_value(dev_mngr->gpio.pmic_en, 0);

	mutex_unlock(&dev_mngr->op_lock);
	msleep(100);
	dev_info(&mdev->pdev->dev, "power off device done\n");
	return 0;
}

int mtk_pwrctl_fldr(void)
{
	struct pwrctl_mdev *mdev;
	int state;

	if (!dev_mngr){
		pr_err("pwrctl: No FLDR method!\n");
		return -EPERM;
	}

	mdev = dev_mngr->mdev;
	dev_info(&mdev->pdev->dev, "FLDR in by %ps\n", __builtin_return_address(0));

	state = mtk_pwrctl_get_power_state(mdev);
	if (state == PWRCTL_STATE_PWROFF) {
		dev_info(&mdev->pdev->dev, "Device is powered off, ignore the warm reset\n");
		return 0;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return -EINVAL;
	}
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWROFF);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_OFF);

	gpio_set_value(dev_mngr->gpio.mode_ctrl, 0);
	udelay(2000);
	gpio_set_value(dev_mngr->gpio.mode_ctrl, 1);
	GPIO_SET_MODE_PREBOOT();
	msleep(300);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_ON);
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);
	mutex_unlock(&dev_mngr->op_lock);

	dev_info(&mdev->pdev->dev, "FLDR done\n");
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_fldr);

void mtk_pwrctl_fldr_remove_rescan(struct work_struct *work)
{
	struct pwrctl_mdev *mdev;
	int state;

	if (!dev_mngr){
		pr_err("pwrctl: No FLDR method!\n");
		return;
	}

	mdev = dev_mngr->mdev;
	dev_info(&mdev->pdev->dev, "FLDR in by %ps\n", __builtin_return_address(0));

	state = mtk_pwrctl_get_power_state(mdev);
	if (state == PWRCTL_STATE_PWROFF) {
		dev_info(&mdev->pdev->dev, "Device is powered off, ignore the warm reset\n");
		return;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return;
	}
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWROFF);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_REMOVE_PORT);

	gpio_set_value(dev_mngr->gpio.mode_ctrl, 0);
	udelay(2000);
	gpio_set_value(dev_mngr->gpio.mode_ctrl, 1);
	GPIO_SET_MODE_PREBOOT();
	msleep(300);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SCAN_PORT);
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);
	mutex_unlock(&dev_mngr->op_lock);

	dev_info(&mdev->pdev->dev, "FLDR in linkdown done\n");
	return;
}
EXPORT_SYMBOL(mtk_pwrctl_fldr_remove_rescan);

int mtk_pwrctl_pldr(void)
{
	struct pwrctl_mdev *mdev;

	if (!dev_mngr){
		pr_err("pwrctl: No PLDR method!\n");
		return -EPERM;
	}

	mdev = dev_mngr->mdev;
	dev_info(&mdev->pdev->dev, "PLDR in by %ps\n", __builtin_return_address(0));
	if (mtk_pwrctl_get_power_state(mdev) == PWRCTL_STATE_PWROFF){
		dev_info(&mdev->pdev->dev, "Device is powered off, ignore the coldreset\n");
		return 0;
	}

	//mutex_lock(&dev_mngr->op_lock);
	if (!mutex_trylock(&dev_mngr->op_lock)) {
	    return -EINVAL;
	}
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWROFF);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_OFF);

	GPIO_SET_MODE_OFF();
	gpio_set_value(dev_mngr->gpio.pmic_en, 0);
	msleep(200);
	gpio_set_value(dev_mngr->gpio.pmic_en, 1);
	msleep(50);
	GPIO_SET_MODE_PREBOOT();

	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_ON);
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);

	mutex_unlock(&dev_mngr->op_lock);
	dev_info(&mdev->pdev->dev, "PLDR done\n");
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_pldr);

int mtk_pwrctl_remove_dev(void)
{
	/* for debug */
	return 0;
}
EXPORT_SYMBOL(mtk_pwrctl_remove_dev);

static int mtk_pwrctl_trigger_exception(struct pwrctl_mdev *mdev)
{
	if (!dev_mngr){
		pr_err("pwrctl: GPIO is not initialized\n");
		return -EINVAL;
	}

	gpio_set_value(dev_mngr->gpio.modem_dump, 1);
	mdelay(15);
	gpio_set_value(dev_mngr->gpio.modem_dump, 0);
	dev_info(&dev_mngr->mdev->pdev->dev, "Trigger exception done\n");

	return 0;
}

static bool mtk_pwrctl_gpio_debounce(int pin, bool level)
{
	int glitch_cnt = 0;

	do {
		if (level != gpio_get_value(pin))
			return false;
		udelay(100);
	} while(glitch_cnt++ < 20);

	return true;
}

static irqreturn_t mtk_pwrctl_eint_handler(int irq, void *arg)
{
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	if (irq != dev_mngr->irq.reboot) {
		dev_warn(&mdev->pdev->dev, "The received IRQ<%d> is not expected IRQ<%d>\n",
			 irq, dev_mngr->irq.reboot);
		goto end;
	}

	if (dev_mngr->eint_work.func)
		schedule_work(&dev_mngr->eint_work);

end:
	return IRQ_HANDLED;
}

static irqreturn_t mtk_pwrctl_faultb_handler(int irq, void *arg)
{
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	if (irq != dev_mngr->irq.faultb) {
		dev_warn(&mdev->pdev->dev, "The received IRQ<%d> is not expected IRQ<%d>\n",
			 irq, dev_mngr->irq.faultb);
		return IRQ_HANDLED;
	}

	if (dev_mngr->faultb_work.func)
		schedule_work(&dev_mngr->faultb_work);

	return IRQ_HANDLED;
}

static irqreturn_t mtk_pwrctl_pcie_wake_handler(int irq, void *arg)
{
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	if (irq != dev_mngr->irq.pcie_wake) {
		dev_warn(&mdev->pdev->dev, "The received IRQ<%d> is not expected IRQ<%d>\n",
			 irq, dev_mngr->irq.pcie_wake);
		goto end;
	}

	if (dev_mngr->pcie_wake_work.func)
		schedule_work(&dev_mngr->pcie_wake_work);

end:
	return IRQ_HANDLED;
}

static void mtk_pwrctl_eint_work(struct work_struct * work)
{
	int level = gpio_get_value(dev_mngr->gpio.reboot);
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	if (!mtk_pwrctl_gpio_debounce(dev_mngr->gpio.reboot, level & 0x01)) {
		dev_info(&dev_mngr->mdev->pdev->dev,
			 "Ignore the glitch of reset_out interrupt\n");
		return;
	}

	/* Rising Edge*/
	if (level & 0x01) {
		dev_info(&dev_mngr->mdev->pdev->dev, "Device is booting to preloader\n");
		mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_WORKING);
		//call GPIO_setting, device is normal working;
		GPIO_SET_MODE_WORKING();
	}
	else {/* Falling Edge*/
		/* delay to avoid conflict with faultb, ensure faultb interrupt runs first*/
		udelay(2000);
		if (mtk_pwrctl_get_power_state(mdev) == PWRCTL_STATE_PWROFF) {
			dev_info(&mdev->pdev->dev,
				 "Device is powered off, ignore the reset_out interrupt\n");
			return;
		}

		dev_info(&dev_mngr->mdev->pdev->dev, "Device is rebooting\n");
		// Workaround: Jira-4889, delay 300ms for EP
		// linkdown procedure, RC PCLK may turn off during this period,
		// that could lead to NOC timeout in the following procedure,
		// after that PCLK return back
		msleep(300);
		dev_info(&dev_mngr->mdev->pdev->dev,
				"wait 300ms for device reset before RC register access\n");
		//call GPIO_setting, device is reset before preloader;
		GPIO_SET_MODE_PREBOOT();
		mtk_pwrctl_event_notify(PWRCTL_EVT_RESET, PWRCTL_UEVENT_RESET);
		mutex_lock(&dev_mngr->op_lock);
		mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_OFF);
		mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_ON);
		mutex_unlock(&dev_mngr->op_lock);
	}
}

static void mtk_pwrctl_faultb_int_work(struct work_struct * work)
{
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	if (!mtk_pwrctl_gpio_debounce(dev_mngr->gpio.faultb, 0))
		return;

	if (mtk_pwrctl_get_power_state(mdev) == PWRCTL_STATE_PWROFF) {
		dev_info(&mdev->pdev->dev, "Ignore this FAULTB due to powered off.\n");
		return;
	}

	dev_info(&mdev->pdev->dev, "Device is powered off with PMIC FAULTB\n");
	mutex_lock(&dev_mngr->op_lock);
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWROFF);
	//call GPIO_setting, device is powered off;
	GPIO_SET_MODE_OFF();
	//xring_pcie_disable_data_trans(0);
	//mtk_pwrctl_event_notify(PWRCTL_EVT_PWROFF, PWRCTL_UEVENT_POWER_OFF);
	/*Avoid completion timeout during soft off, wait EP access reg completely*/
	//dev_info(&mdev->pdev->dev, "before delay\n");
	//msleep(300);
	//dev_info(&mdev->pdev->dev, "delay done for waiting access reg completely\n");
	//mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_OFF);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_REMOVE_PORT);

	gpio_set_value(dev_mngr->gpio.pmic_en, 0);
	msleep(200);

	gpio_set_value(dev_mngr->gpio.pmic_en, 1);
	msleep(50);
	GPIO_SET_MODE_PREBOOT();

	//mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SOFT_ON);
	mtk_pwrctl_request_to_controller(mdev, PWRCTL_CONTROLLER_REQUEST_SCAN_PORT);

	//mtk_pwrctl_event_notify(PWRCTL_EVT_PWRON, PWRCTL_UEVENT_POWER_ON);
	mtk_pwrctl_set_power_state(mdev, PWRCTL_STATE_PWRON);
	mutex_unlock(&dev_mngr->op_lock);
	dev_info(&mdev->pdev->dev, "FAULTB int work done\n");

	{
		t800_send_platform_uevent(5/*MTK_UEVENT_MODEM_MONITOR*/, 7/*MODEM_MONITOR_FAULTB*/, 0, 0, 0);
	}
}

static void mtk_pwrctl_pcie_wake_work(struct work_struct * work)
{
	int level = gpio_get_value(dev_mngr->gpio.pcie_wake);
	struct pwrctl_mdev *mdev = dev_mngr->mdev;

	/* Rising Edge*/
	if (level & 0x01) {
		dev_info(&mdev->pdev->dev, "pcie_wake Rising edge\n");
	} else {/* Falling Edge*/
		dev_info(&mdev->pdev->dev, "pcie_wake Falling edge\n");
	}
}

void xiaomi_mdr_trigger_exception(void)
{
	mdm_mdr_trigger_exception();

}

void mdm_notify_sensorhub_state(bool isReady)
{
	struct conn_to_shub_msg to_shub_msg;
	pr_info("[%s] type=[%d]", __func__, isReady);
	to_shub_msg.module = MODULE_MODEM;
	if (isReady) {
		to_shub_msg.state = DRIVER_STATE_ON;
	} else {
		to_shub_msg.state = DRIVER_STATE_OFF;
	}
	shub_conn_notify_state(&to_shub_msg);
}

void xiaomi_notify_sensorhub(bool isReady)
{
	mdm_notify_sensorhub_state(isReady);
}

long mtk_pwrctl_cmd_process(struct pwrctl_mdev *mdev, int cmd)
{
	enum pwrctl_state state = mtk_pwrctl_get_power_state(mdev);

	dev_info(&mdev->pdev->dev, "IOCTL CMD: %d by user[%d:%s]\n", _IOC_NR(cmd),
		 current->pid, current->comm);

	switch(cmd) {
	case PWRCTL_CMD_POWER_ON:
		mtk_pwrctl_power_on(mdev, true);
		mtk_pwrctl_uevent_notify_user(PWRCTL_UEVENT_POWER_ON);
		break;
	case PWRCTL_CMD_POWER_OFF:
		if (state == PWRCTL_STATE_PWROFF){
			dev_info(&mdev->pdev->dev,
				 "Device is powered off, ignore the repeated operation\n");
			return -EINVAL;
		}

		mtk_pwrctl_uevent_notify_user(PWRCTL_UEVENT_POWER_OFF);
		mtk_pwrctl_power_off(mdev, true);
		break;
	case PWRCTL_CMD_COLD_RESET:
		if (state == PWRCTL_STATE_PWROFF){
			dev_info(&mdev->pdev->dev,
				 "Device is powered off, please execute power on command first\n");
			return -EINVAL;
		}

		mtk_pwrctl_event_notify(PWRCTL_EVT_PWROFF, PWRCTL_UEVENT_POWER_OFF);
		mtk_pwrctl_pldr();
		mtk_pwrctl_event_notify(PWRCTL_EVT_PWRON, PWRCTL_UEVENT_POWER_ON);
		break;
	case PWRCTL_CMD_WARM_RESET:
		if(state == PWRCTL_STATE_PWROFF) {
			dev_info(&mdev->pdev->dev,
				"The device is powered off, ignore warm reset operation\n");
			return -EINVAL;
		}

		mtk_pwrctl_event_notify(PWRCTL_EVT_PWROFF, PWRCTL_UEVENT_POWER_OFF);
		mtk_pwrctl_fldr();
		mtk_pwrctl_event_notify(PWRCTL_EVT_PWRON, PWRCTL_UEVENT_POWER_ON);
		break;
	case PWRCTL_CMD_TRIGGER_EXCEPTION:
		if(state == PWRCTL_STATE_PWROFF) {
			dev_info(&mdev->pdev->dev,
				"The device is powered off, ignore dump operation\n");
			return -EINVAL;
		}

		mtk_pwrctl_trigger_exception(mdev);
		break;
	case PWRCTL_CMD_POWERUP:
		xiaomi_pwrctl_power_on(mdev, true);
		break;
	case PWRCTL_CMD_MDR_TRIGGER_EXCEPTION:
		xiaomi_mdr_trigger_exception();
		break;
	case PWRCTL_CMD_NOTIFY_SENSORHUB_READY:
		xiaomi_notify_sensorhub(1);
		break;
	case PWRCTL_CMD_NOTIFY_SENSORHUB_EXCEPTION:
		xiaomi_notify_sensorhub(0);
		break;
	default:
		dev_err(&mdev->pdev->dev, "Invalid command\n");
		return -EINVAL;
	break;
        }
	return 0;
}

static int mtk_pwrctl_dev_gpio_init(struct pwrctl_dev_mngr *dev_mngr)
{
	struct pwrctl_gpio *gpio;
	struct pwrctl_irq *irq;

	gpio = &dev_mngr->gpio;
	irq = &dev_mngr->irq;

	gpio->mode_ctrl = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_MODEM_CTRL);
	gpio->pmic_en = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_PMIC_EN);
	gpio->reboot = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_REBOOT_INT);
#if 0
	irq->reboot = mtk_pwrctl_irq_request(dev_mngr->dev, gpio->reboot,
						  mtk_pwrctl_eint_handler,
						  PWRCTL_REBOOT_TRIGGER_MODE,
						  true);
	INIT_WORK(&dev_mngr->eint_work, mtk_pwrctl_eint_work);
#endif
	gpio->modem_dump = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_MODOME_DUMP);
	gpio->faultb = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_FAULT_BAR);
#if 0
	irq->faultb = mtk_pwrctl_irq_request(dev_mngr->dev, gpio->faultb,
						  mtk_pwrctl_faultb_handler,
						  IRQF_TRIGGER_FALLING,
						  true);
	INIT_WORK(&dev_mngr->faultb_work, mtk_pwrctl_faultb_int_work);
#endif

	gpio->pcie_wake = mtk_pwrctl_gpio_request(dev_mngr->dev, PWRCTL_GPIO_PCIE_WAKE);

	gpio_set_value(gpio->mode_ctrl, 1);
	dev_info(dev_mngr->dev, "gpio init done\n");
	return 0;
}

static void mtk_pwrctl_dev_gpio_uninit(struct pwrctl_dev_mngr *dev_mngr)
{
#if 0
	struct pwrctl_irq *irq = &dev_mngr->irq;

	flush_work(&dev_mngr->eint_work);
	flush_work(&dev_mngr->faultb_work);
	mtk_pwrctl_irq_free(dev_mngr->dev, irq->reboot);
	mtk_pwrctl_irq_free(dev_mngr->dev, irq->faultb);
#endif
	xiaomi_modem_power_irq_unregister();
	dev_info(dev_mngr->dev, "gpio free done\n");
}

static void mtk_pwrctl_dev_param_init(struct pwrctl_mdev *mdev)
{
	dev_mngr = &mdev->dev_mngr;
	dev_mngr->dev = &mdev->pdev->dev;
	dev_mngr->mdev = mdev;

	mutex_init(&dev_mngr->op_lock);

	clear_bit(PWRCTL_SKIP_SUSPEND_OFF, &mdev->pm.flags);
}

static void mtk_pwrctl_dev_param_uninit(struct pwrctl_mdev *mdev)
{
	dev_mngr->dev = NULL;
	dev_mngr->mdev = NULL;
	dev_mngr = NULL;
}

void mtk_power_irq_register(struct pwrctl_dev_mngr *dev_mngr)
{
	struct pwrctl_gpio *gpio;
	struct pwrctl_irq *irq;

	gpio = &dev_mngr->gpio;
	irq = &dev_mngr->irq;
	irq->reboot = mtk_pwrctl_irq_request(dev_mngr->dev, gpio->reboot,
						  mtk_pwrctl_eint_handler,
						  PWRCTL_REBOOT_TRIGGER_MODE,
						  true);
	INIT_WORK(&dev_mngr->eint_work, mtk_pwrctl_eint_work);
	irq->faultb = mtk_pwrctl_irq_request(dev_mngr->dev, gpio->faultb,
						  mtk_pwrctl_faultb_handler,
						  IRQF_TRIGGER_FALLING,
						  true);
	INIT_WORK(&dev_mngr->faultb_work, mtk_pwrctl_faultb_int_work);

	irq->pcie_wake = mtk_pwrctl_irq_request(dev_mngr->dev, gpio->pcie_wake,
						  mtk_pwrctl_pcie_wake_handler,
						  PWRCTL_PCIE_WAKE_TRIGGER_MODE,
						  true);
	INIT_WORK(&dev_mngr->pcie_wake_work, mtk_pwrctl_pcie_wake_work);

	pr_info("%s done!\n", __func__);
}

void mtk_power_irq_unregister(struct pwrctl_dev_mngr *dev_mngr)
{
	struct pwrctl_irq *irq = &dev_mngr->irq;
	pr_info("%s in \n", __func__);
	flush_work(&dev_mngr->eint_work);
	flush_work(&dev_mngr->faultb_work);
	mtk_pwrctl_irq_free(dev_mngr->dev, irq->reboot);
	mtk_pwrctl_irq_free(dev_mngr->dev, irq->faultb);

}

int mtk_pwrctl_dev_init(struct pwrctl_mdev *mdev)
{
	mtk_pwrctl_dev_param_init(mdev);
	mtk_pwrctl_dev_gpio_init(&mdev->dev_mngr);
	mdm_mdr_exception_register();

	return 0;
}

int mtk_pwrctl_dev_uninit(struct pwrctl_mdev *mdev)
{
	mtk_pwrctl_dev_gpio_uninit(&mdev->dev_mngr);
	mtk_pwrctl_dev_param_uninit(mdev);
	mdm_mdr_exception_unregister();
	return 0;
}
