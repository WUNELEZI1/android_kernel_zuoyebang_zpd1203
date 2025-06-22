// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2010-2011 Picochip Ltd., Jamie Iles
 * https://www.picochip.com
 *
 * This file implements a driver for the Synopsys DesignWare watchdog device
 * in the many subsystems. The watchdog has 16 different timeout periods
 * and these are a function of the input clock frequency.
 *
 * The DesignWare watchdog cannot be stopped once it has been started so we
 * do not implement a stop function. The watchdog core will continue to send
 * heartbeat requests after the watchdog device has been closed.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include "adsp_boot_driver.h"
#include "adsp_ssr_driver.h"
#include "../common/fk-audio-log.h"
#include "../common/fk-common-ctrl.h"

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif

static struct adsp_boot_dev g_adsp_boot_dev;

static void adsp_vote_poweron(void)
{
	int ret;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	AUD_LOG_INFO(AUD_DSP, "adsp vote poweron, current vote_status: %s",
		(g_adsp_boot_dev.vote_status == true) ? "true" : "false");
	if (g_adsp_boot_dev.vote_status == false) {
		AUD_LOG_INFO(AUD_DSP, "adsp vote poweron");
		ret = regulator_enable(g_adsp_boot_dev.adsp_boot_reg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_DSP, "failed to enable adsp power regulator");
#if IS_ENABLED(CONFIG_MIEV)
			mievent  = cdev_tevent_alloc(906001501);
			cdev_tevent_add_str(mievent, "Keyword", "adsp vote poweron fail");
			cdev_tevent_write(mievent);
			cdev_tevent_destroy(mievent);
#endif
			return;
		}
		g_adsp_boot_dev.vote_status = true;
	}
}

static void adsp_vote_poweroff(void)
{
	int ret;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	AUD_LOG_INFO(AUD_DSP, "adsp vote poweroff, current vote_status: %s",
		(g_adsp_boot_dev.vote_status == true) ? "true" : "false");
	if (g_adsp_boot_dev.vote_status == true) {
		AUD_LOG_INFO(AUD_DSP, "adsp vote poweroff");
		ret = regulator_disable(g_adsp_boot_dev.adsp_boot_reg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_DSP, "failed to disable adsp power regulator: %d", ret);
#if IS_ENABLED(CONFIG_MIEV)
			mievent  = cdev_tevent_alloc(906001501);
			cdev_tevent_add_str(mievent, "Keyword", "adsp vote poweroff fail");
			cdev_tevent_write(mievent);
			cdev_tevent_destroy(mievent);
#endif
			return;
		}
		g_adsp_boot_dev.vote_status = false;
	}
}

/************************************************************************
 * function: get_adsp_power_status
 * description:acpu vote status
 * parameter:
 * return:
 *		1 : vote adsp poweron
 *		0 : vote adsp poweroff
 ************************************************************************/
int get_adsp_power_status(void)
{
	return g_adsp_boot_dev.power_status;
}

/************************************************************************
 * function: copy_adsp_img_to_running_memory
 * description:copy adsp img from load mem to run mem
 * parameter:
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int copy_adsp_img_to_running_memory(void)
{
	if (memcpy(g_adsp_boot_dev.adsp_run_addr, g_adsp_boot_dev.adsp_load_addr,
		g_adsp_boot_dev.adsp_image_size) == NULL) {
		AUD_LOG_ERR(AUD_DSP, "copy adsp img to running memory error");
		return -ENOMEM;
	}

	AUD_LOG_INFO(AUD_DSP, "copy adspimg to running memory finished");
	return 0;
}

/************************************************************************
 * function: check_audsys_power_flow_done
 * description:wait audsys_poweroff_flow_down = 0
 * parameter:
 *		timwout : timeout msec
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int check_audsys_power_off_flow_done(int timwout)
{
	int power_off_flow_done;
	int time_cnt = 0;

	power_off_flow_done = adsp_reg_read32(g_adsp_boot_dev.audsys_poweroff_flow_down);
	while (power_off_flow_done != 0) {
		usleep_range(1000, 2000);
		time_cnt++;
		if (time_cnt > timwout) {
			/* adsp exception */
			return -EFAULT;
		}
		power_off_flow_done = adsp_reg_read32(g_adsp_boot_dev.audsys_poweroff_flow_down);
	}

	return 0;
}

/************************************************************************
 * function: check_audsys_power_on_flow_done
 * description:wait power_on_flow_done = 1 || power_on_flow_done = 2
 * parameter:
 *		timwout : timeout msec
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int check_audsys_power_on_flow_done(int timwout)
{
	int power_on_flow_done;
	int time_cnt = 0;

	power_on_flow_done = adsp_reg_read32(g_adsp_boot_dev.audsys_power_state);
	while ((power_on_flow_done != AUDSYS_POWERON_STATE) &&
		(power_on_flow_done != AUDSYS_PREPARE_POWERDOWN)) {
		usleep_range(1000, 2000);
		time_cnt++;
		if (time_cnt > timwout) {
			/* adsp exception */
			return -EFAULT;
		}
		power_on_flow_done = adsp_reg_read32(g_adsp_boot_dev.audsys_power_state);
	}

	return 0;
}

/************************************************************************
 * function: adsp_poweron_flow
 * description:adsp poweron flow
 * parameter:
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int adsp_poweron_flow(void)
{
	int ret = 0;
	int audsys_power_status;

	adsp_reg_write32(g_adsp_boot_dev.acpu2adsp_interupt_lock_reg, 0x1);

	/* if audsys_power_status == 0x3 then wait adsp poweroff flow done */
	audsys_power_status = adsp_reg_read32(g_adsp_boot_dev.audsys_power_state);

	if ((audsys_power_status == AUDSYS_POWERON_STATE) ||
		(audsys_power_status == AUDSYS_PREPARE_POWERDOWN)) {
		AUD_LOG_INFO(AUD_DSP, "adsp status is %d, not need to vote poweron.", audsys_power_status);
		g_adsp_boot_dev.power_status = true;
		return 0;
	}

	if (audsys_power_status == AUDSYS_ADSP_VOTE_SHUTDOWN) {
		ret = check_audsys_power_off_flow_done(WAIT_AUDSYS_POWEROFF_FLOW_DOWN_TIMEOUT);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_DSP, "check adsp power off flow done failed.");
			return ret;
		}
	}

	adsp_vote_poweron();
	g_adsp_boot_dev.power_status = true;

	/* wait adsp poweron flow done & adsp initialization done */
	ret = check_audsys_power_on_flow_done(WAIT_AUDSYS_POWERON_DONE_TIMEOUT);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "wait audsys poweron done failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: adsp_poweron
 * description:adsp power on interface
 * parameter:
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int adsp_poweron(void)
{
	int index = 0;

	if (g_adsp_boot_dev.power_status == true) {
		AUD_LOG_ERR(AUD_DSP, "adsp already poweron, please do not poweron repeatedly");
		return -EINVAL;
	}

	if (g_adsp_boot_dev.adsp_boot_reg == NULL) {
		AUD_LOG_ERR(AUD_DSP, "adsp boot regulator is NULL");
		return -EINVAL;
	}

	for (; index < ADSP_BOOT_RETRY_TIME; index++) {
		if (adsp_poweron_flow() == 0)
			break;
	}

	if (index == ADSP_BOOT_RETRY_TIME) {
		AUD_LOG_ERR(AUD_DSP, "adsp poweron failed");
		return -EFAULT;
	}

	AUD_LOG_INFO(AUD_DSP, "adsp poweron success");

	return 0;
}

/************************************************************************
 * function: adsp_poweroff
 * description:adsp power off interface
 * parameter:
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int adsp_poweroff(void)
{
	if (g_adsp_boot_dev.power_status == false) {
		AUD_LOG_ERR(AUD_DSP, "adsp already poweroff, please do not poweroff repeatedly");
		return -EINVAL;
	}

	if (g_adsp_boot_dev.adsp_boot_reg == NULL) {
		AUD_LOG_ERR(AUD_DSP, "adsp boot regulator is NULL");
		return -EINVAL;
	}

	adsp_reg_write32(g_adsp_boot_dev.acpu2adsp_interupt_lock_reg, 0x0);

	adsp_vote_poweroff();
	g_adsp_boot_dev.power_status = false;

	AUD_LOG_INFO(AUD_DSP, "adsp poweroff success");
	return 0;
}

static int get_device_type(struct device *dev)
{
	int ret = 0;
	u32 device_type = 0;

	ret = of_property_read_u32(dev->of_node, "device-type", &device_type);
	if (ret)
		AUD_LOG_ERR(AUD_DSP, "No DT match device-type\n");

	return (device_type == 0) ? 0 : 1;
}

static int adsp_boot_drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *image_mem = NULL;
	struct resource *run_mem = NULL;
	struct adsp_boot_dev *adsp_boot_device = NULL;
	int device_type = 0;

	adsp_boot_device = &g_adsp_boot_dev;

	g_adsp_boot_dev.adsp_boot_reg = regulator_get(&pdev->dev, "regulator_asp");
	if (g_adsp_boot_dev.adsp_boot_reg == NULL) {
		AUD_LOG_ERR(AUD_DSP, "failed to get adsp-boot-reg");
		ret = -ENXIO;
		goto err;
	}

	adsp_boot_device->audsys_poweroff_flow_down = ioremap(AUDSYS_POWEROFF_FLOW_DOWN,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_boot_device->audsys_poweroff_flow_down) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDSYS_POWEROFF_FLOW_DOWN");
		ret = -EINVAL;
		goto err;
	}

	adsp_boot_device->audsys_power_state = ioremap(AUDSYS_POWER_STATE,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_boot_device->audsys_power_state) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDSYS_POWEROFF_STATE");
		iounmap(adsp_boot_device->audsys_poweroff_flow_down);
		ret = -EINVAL;
		goto err;
	}

	adsp_boot_device->acpu2adsp_interupt_lock_reg = ioremap(ACPU2ADSP_INTERRPUT_LOCK,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_boot_device->acpu2adsp_interupt_lock_reg) {
		AUD_LOG_ERR(AUD_DSP, "can't remap ACPU2ADSP_INTERRPUT_LOCK");
		iounmap(adsp_boot_device->audsys_poweroff_flow_down);
		iounmap(adsp_boot_device->audsys_power_state);
		ret = -EINVAL;
		goto err;
	}

	adsp_boot_device->adsp_load_addr = devm_platform_get_and_ioremap_resource(pdev, 0, &image_mem);

	AUD_LOG_INFO(AUD_DSP, "adsp_load_addr = 0x%llx",
		(uint64_t)adsp_boot_device->adsp_load_addr);

	if (!adsp_boot_device->adsp_load_addr) {
		iounmap(adsp_boot_device->audsys_poweroff_flow_down);
		iounmap(adsp_boot_device->audsys_power_state);
		iounmap(adsp_boot_device->acpu2adsp_interupt_lock_reg);
		AUD_LOG_ERR(AUD_DSP,  "iomap adsp load addr error");
		ret = -ENODEV;
		goto err;
	}

	adsp_boot_device->adsp_run_addr = devm_platform_get_and_ioremap_resource(pdev, 1, &run_mem);

	AUD_LOG_INFO(AUD_DSP, "adsp_run_addr = 0x%llx",
		(uint64_t)adsp_boot_device->adsp_run_addr);

	if (!adsp_boot_device->adsp_run_addr) {
		iounmap(adsp_boot_device->audsys_poweroff_flow_down);
		iounmap(adsp_boot_device->audsys_power_state);
		iounmap(adsp_boot_device->acpu2adsp_interupt_lock_reg);
		iounmap(adsp_boot_device->adsp_load_addr);
		AUD_LOG_ERR(AUD_DSP, "iomap adsp load addr error");
		ret = -ENODEV;
		goto err;
	}

	adsp_boot_device->adsp_image_size = resource_size(image_mem);
	AUD_LOG_INFO(AUD_DSP, "adsp_image_size = 0x%x",
		adsp_boot_device->adsp_image_size);

	ret = copy_adsp_img_to_running_memory();
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "copy adsp img to running memory is failed");
		goto err;
	}

	adsp_boot_device->vote_status = false;
	adsp_boot_device->power_status = false;

	device_type = get_device_type(&pdev->dev);
	fk_device_type_set(device_type);

err:
	return ret;
}

static int adsp_boot_drv_remove(struct platform_device *pdev)
{
	if (g_adsp_boot_dev.audsys_poweroff_flow_down)
		iounmap(g_adsp_boot_dev.audsys_poweroff_flow_down);
	if (g_adsp_boot_dev.audsys_power_state)
		iounmap(g_adsp_boot_dev.audsys_power_state);
	if (g_adsp_boot_dev.acpu2adsp_interupt_lock_reg)
		iounmap(g_adsp_boot_dev.acpu2adsp_interupt_lock_reg);
	if (g_adsp_boot_dev.adsp_load_addr)
		iounmap(g_adsp_boot_dev.adsp_load_addr);
	if (g_adsp_boot_dev.adsp_run_addr)
		iounmap(g_adsp_boot_dev.adsp_run_addr);

	return 0;
}

static const struct of_device_id adsp_boot_of_match[] = {
	{ .compatible = "xring,aduio-adsp-boot", },
	{ /* sentinel */ }
};

static struct platform_driver adsp_boot_driver = {
	.probe     = adsp_boot_drv_probe,
	.remove    = adsp_boot_drv_remove,
	.driver    = {
		.name  = "adsp_boot",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_boot_of_match),
	},
};

int __init adsp_boot_init(void)
{
	return platform_driver_register(&adsp_boot_driver);
}

void adsp_boot_exit(void)
{
	platform_driver_unregister(&adsp_boot_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING ADSP BOOT driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);

