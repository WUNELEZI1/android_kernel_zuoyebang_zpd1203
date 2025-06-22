// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_stmvl53l8", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_stmvl53l8", __func__, __LINE__

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/atomic.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include "xrisp_tof.h"

#define XRISP_TOF_VER_MAJOR 0x06
#define XRISP_TOF_VER_MINOR 0x01
#define XRISP_TOF_VER_BUILD 0x00
#define XRISP_TOF_VER_REVISION 0x00
#define SPI_DEFAULT_TRANSFER_SPEED_HZ 100000
#define XRISP_TOF_DRIVER_NAME "stmvl53l8"

static int tof_regulator_power(struct regulator *reg, uint32_t voltage,
		uint32_t current_val, bool on)
{
	int ret = 0;

	if (!reg) {
		XRISP_PR_WARN("regulator is not found.");
		goto exit;
	}

	ret = on ? regulator_set_voltage(reg, voltage, voltage) :
		regulator_set_voltage(reg, 0, voltage);
	if (ret) {
		XRISP_PR_ERROR("set regulator's voltage failed.");
		goto exit;
	}
	ret = regulator_set_load(reg, current_val);
	if (ret) {
		XRISP_PR_ERROR("set regulator's current failed.");
		goto exit;
	}
	ret = on ? regulator_enable(reg) : regulator_disable(reg);
	if (ret) {
		XRISP_PR_ERROR("regulator enable or disable failed.");
		goto exit;
	}

exit:
	return ret;
}

static int tof_power_on(struct xrisp_tof_data *tof_data)
{
	int ret = 0;

	if (!tof_data) {
		ret = -EINVAL;
		goto exit;
	}
	ret = tof_regulator_power(tof_data->regulator_3V3, 3300000, 85000, true);
	if (ret) {
		ret = -EFAULT;
		goto exit;
	}
	ret = tof_regulator_power(tof_data->regulator_1V8, 1800000, 85000, true);
	if (ret) {
		ret = -EFAULT;
		goto off_3v3;
	}
	ret = tof_regulator_power(tof_data->regulator_1V2, 1200000, 85000, true);
	if (ret) {
		ret = -EFAULT;
		goto off_1v8;
	}

	return ret;

off_1v8:
	(void)tof_regulator_power(tof_data->regulator_1V8, 1800000, 0, false);
off_3v3:
	(void)tof_regulator_power(tof_data->regulator_3V3, 3300000, 0, false);
exit:
	return ret;
}

static int tof_power_off(struct xrisp_tof_data *tof_data)
{
	int ret = 0;

	if (!tof_data) {
		ret = -EINVAL;
		goto exit;
	}

	ret = tof_regulator_power(tof_data->regulator_1V2, 1200000, 0, false);
	if (ret) {
		ret = -EFAULT;
		goto exit;
	}
	ret = tof_regulator_power(tof_data->regulator_1V8, 1800000, 0, false);
	if (ret) {
		ret = -EFAULT;
		goto on_1v2;
	}
	ret = tof_regulator_power(tof_data->regulator_3V3, 3300000, 0, false);
	if (ret) {
		ret = -EFAULT;
		goto on_1v8;
	}

	return ret;

on_1v8:
	(void)tof_regulator_power(tof_data->regulator_1V8, 1800000, 85000, true);
on_1v2:
	(void)tof_regulator_power(tof_data->regulator_1V2, 1200000, 85000, true);
exit:
	return ret;
}

static int tof_open(struct inode *inode, struct file *file)
{
	XRISP_PR_INFO("Open multi_tof driver: stmvl53l8.");
	return 0;
}

static int tof_release(struct inode *inode, struct file *file)
{
	XRISP_PR_INFO("Close multi_tof driver: stmvl53l8.");
	struct xrisp_tof_data *tof_dev = container_of(file->private_data,
		struct xrisp_tof_data, miscdev);

	if (tof_dev->power_refs > 0) {
		(void)tof_power_off(tof_dev);
		tof_dev->power_refs = 0;
	}

	return 0;
}

static long tof_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	uint8_t reg_buf[3] = {0};
	uint8_t info[8] = { XRISP_TOF_VER_MAJOR, XRISP_TOF_VER_MINOR,
			XRISP_TOF_VER_BUILD, XRISP_TOF_VER_REVISION };
	struct xrisp_control_arg karg = {0};
	enum xrisp_tof_power_mode power_mode;
	struct xrisp_tof_data *tof_dev = container_of(file->private_data,
		struct xrisp_tof_data, miscdev);

	if (copy_from_user(&karg, (struct xrisp_control_arg __user *)arg,
			sizeof(struct xrisp_control_arg))) {
		XRISP_PR_ERROR("Failed to get parameters from userspace.");
		return -EFAULT;
	}

	mutex_lock(&tof_dev->tof_mutex);
	switch (karg.op_code) {
	case XRISP_CAM_TOF_IOCTL_POWER_OFF:
		if (tof_dev->power_refs == 0) {
			XRISP_PR_INFO("tof has been powered off.");
			break;
		}
		if (--tof_dev->power_refs > 0) {
			XRISP_PR_INFO("power_refs=%d.", tof_dev->power_refs);
			break;
		}
		ret = tof_power_off(tof_dev);
		if (ret) {
			XRISP_PR_ERROR("Failed to power off.");
			tof_dev->power_refs++;
			ret = -EFAULT;
			goto exit;
		}
		tof_dev->power_refs = 0;
		break;
	case XRISP_CAM_TOF_IOCTL_POWER_ON:
		if (tof_dev->power_refs > 0) {
			XRISP_PR_INFO("tof has powered on, power_refs=%d.", tof_dev->power_refs);
			tof_dev->power_refs++;
			break;
		}
		ret = tof_power_on(tof_dev);
		if (ret) {
			XRISP_PR_ERROR("Failed to power on.");
			ret = -EFAULT;
			goto exit;
		}
		tof_dev->power_refs++;
		break;
	case XRISP_CAM_TOF_IOCTL_POWER_MODE:
		if (tof_dev->power_refs <= 0) {
			XRISP_PR_ERROR("tof need to be powered on.");
			ret = -EFAULT;
			goto exit;
		}
		if (copy_from_user(&power_mode, u64_to_user_ptr(karg.handle),
			sizeof(power_mode))) {
			XRISP_PR_ERROR("Failed to get power_mode from userspace.");
			ret = -EFAULT;
			goto exit;
		}
		ret = tof_set_power_mode(tof_dev, power_mode);
		if (ret) {
			XRISP_PR_ERROR("Set power mode failed.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_CHIP_INIT:
		ret = check_rom_firmware_boot(tof_dev);
		if (ret) {
			XRISP_PR_ERROR("Failed to check rom firmware boot.");
			ret = -EFAULT;
			goto exit;
		}
		ret = tof_load_firmware(tof_dev);
		if (ret) {
			XRISP_PR_ERROR("Failed to load firmware.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_SET_SPEED_HZ:
		if (copy_from_user(&tof_dev->transfer_speed_hz, u64_to_user_ptr(karg.handle),
			sizeof(tof_dev->transfer_speed_hz))) {
			XRISP_PR_ERROR("Failed to get speed_hz from userspace.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_WRITE_REG:
		if (copy_from_user(reg_buf, u64_to_user_ptr(karg.handle), sizeof(reg_buf))) {
			XRISP_PR_ERROR("Failed to get reg info from userspace.");
			ret = -EFAULT;
			goto exit;
		}
		ret = tof_write_byte(tof_dev, (reg_buf[1] << 8) | reg_buf[0], reg_buf[2]);
		if (ret) {
			XRISP_PR_ERROR("Failed to write reg_addr=0x%x, reg_val=0x%x.",
				(reg_buf[1] << 8) | reg_buf[0], reg_buf[2]);
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_READ_REG:
		if (copy_from_user(reg_buf, u64_to_user_ptr(karg.handle), sizeof(reg_buf))) {
			XRISP_PR_ERROR("Failed to get reg info from userspace.");
			ret = -EFAULT;
			goto exit;
		}
		ret = tof_read_byte(tof_dev, (reg_buf[1] << 8) | reg_buf[0], &reg_buf[2]);
		if (ret) {
			XRISP_PR_ERROR("Failed to read reg_addr=0x%x, reg_val=0x%x.",
				(reg_buf[1] << 8) | reg_buf[0], reg_buf[2]);
			ret = -EFAULT;
			goto exit;
		}
		if (copy_to_user(u64_to_user_ptr(karg.handle), reg_buf, sizeof(reg_buf))) {
			XRISP_PR_ERROR("Failed to copy to userspace.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_GET_INFO:
		info[4] = tof_dev->firmware.device_id;
		info[5] = tof_dev->firmware.revision_id;
		info[6] = tof_dev->firmware.device_booted;
		info[7]	= tof_dev->firmware.fw_loaded;

		if (copy_to_user(u64_to_user_ptr(karg.handle), info, sizeof(info))) {
			XRISP_PR_ERROR("Failed to copy to userspace.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	default:
		XRISP_PR_ERROR("The option is invalid, karg.op_code=%d.", karg.op_code);
		ret = -EFAULT;
		break;
	}

exit:
	mutex_unlock(&tof_dev->tof_mutex);
	return ret;
}

static const struct file_operations tof_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= tof_ioctl,
	.open			= tof_open,
	.release		= tof_release,
};

static irqreturn_t tof_irq_handler(int irq, void *dev)
{
	XRISP_PR_DEBUG("Enter interrupt.");
	return IRQ_HANDLED;
}

static int tof_parse_dt(struct xrisp_tof_data *tof_data)
{
	int ret = 0;
	struct device_node *node = tof_data->device->dev.of_node;

	if (of_parse_phandle(node, "power_1V2-supply", 0)) {
		tof_data->regulator_1V2 = devm_regulator_get(&tof_data->device->dev, "power_1V2");
		if (IS_ERR_OR_NULL(tof_data->regulator_1V2)) {
			XRISP_PR_ERROR("regulator get 'power_1V2' failed.");
			return -ENODEV;
		}
		XRISP_PR_INFO("regulator get 'power_1V2' success.");
	} else {
		XRISP_PR_INFO("regulator 'power_1V2' is not found.");
		tof_data->regulator_1V2 = NULL;
	}

	if (of_parse_phandle(node, "power_1V8-supply", 0)) {
		tof_data->regulator_1V8 = devm_regulator_get(&tof_data->device->dev, "power_1V8");
		if (IS_ERR_OR_NULL(tof_data->regulator_1V8)) {
			XRISP_PR_ERROR("regulator get 'power_1V8' failed.");
			return -ENODEV;
		}
		XRISP_PR_INFO("regulator get 'power_1V8' success.");
	} else {
		XRISP_PR_INFO("regulator 'power_1V8' is not found.");
		tof_data->regulator_1V8 = NULL;
	}

	if (of_parse_phandle(node, "power_3V3-supply", 0)) {
		tof_data->regulator_3V3 = devm_regulator_get(&tof_data->device->dev, "power_3V3");
		if (IS_ERR_OR_NULL(tof_data->regulator_3V3)) {
			XRISP_PR_ERROR("regulator get 'power_3V3' failed.");
			return -ENODEV;
		}
		XRISP_PR_INFO("regulator get 'power_3V3' success.");
	} else {
		XRISP_PR_INFO("regulator 'power_3V3' is not found.");
		tof_data->regulator_3V3 = NULL;
	}

#ifdef XRISP_TOF_SPI_MODE
	if (of_find_property(node, "spi-cpha", NULL))
		tof_data->device->mode |= SPI_CPHA;
	if (of_find_property(node, "spi-cpol", NULL))
		tof_data->device->mode |= SPI_CPOL;
	XRISP_PR_INFO("stmvl53l8 spi mode: %d", tof_data->device->mode);
#endif

	if (of_property_read_string(node, "xring,firmware_name",
		&tof_data->firmware.fw_name)) {
		XRISP_PR_ERROR("Couldn't find 'xring,firmware_name'.");
		return -EINVAL;
	}

	tof_data->inter_gpio = of_get_named_gpio(node, "inter-gpio", 0);
	if (gpio_is_valid(tof_data->inter_gpio)) {
		ret = devm_gpio_request(&tof_data->device->dev, tof_data->inter_gpio, "stmvl53l8 inter gpio");
		if (ret) {
			XRISP_PR_ERROR("inter gpio request failed.");
			return -EINVAL;
		}

		ret = gpio_direction_input(tof_data->inter_gpio);
		if (ret) {
			XRISP_PR_ERROR("inter gpio request failed.");
			goto exit;
		}

		tof_data->inter_irq = gpio_to_irq(tof_data->inter_gpio);
		if (tof_data->inter_irq < 0) {
			XRISP_PR_ERROR("get irq failed.");
			ret = -EINVAL;
			goto exit;
		}

		ret = devm_request_threaded_irq(&tof_data->device->dev, tof_data->inter_irq, NULL, tof_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "stmvl53l8_interrupt", NULL);
		if (ret) {
			XRISP_PR_ERROR("request threaded irq failed.");
			goto exit;
		}

		XRISP_PR_INFO("inter_gpio: %d, inter_irq=%d",
			tof_data->inter_gpio, tof_data->inter_irq);
	}

	return ret;

exit:
	return ret;
}

#ifdef XRISP_TOF_SPI_MODE
static int tof_probe(struct spi_device *dev)
#else
static int tof_probe(struct i2c_client *dev)
#endif
{
	int ret = 0;
	struct xrisp_tof_data *tof_dev = NULL;

	XRISP_PR_INFO("Enter spi probe.");

	tof_dev = kzalloc(sizeof(struct xrisp_tof_data), GFP_KERNEL);
	if (tof_dev == NULL) {
		XRISP_PR_ERROR("Failed to allocate memory");
		return -ENOMEM;
	}

	tof_dev->device = dev;
	tof_dev->firmware.fw_name = NULL;
	tof_dev->transfer_speed_hz = SPI_DEFAULT_TRANSFER_SPEED_HZ;
	tof_dev->power_mode = XRISP_TOF_POWER_MODE_HP;
	tof_dev->power_refs = 0;
	mutex_init(&tof_dev->tof_mutex);

	ret = tof_parse_dt(tof_dev);
	if (ret) {
		XRISP_PR_ERROR("Failed to parse device tree");
		goto exit;
	}

	ret = tof_power_on(tof_dev);
	if (ret) {
		XRISP_PR_ERROR("Regulator failed to power on.");
		goto exit;
	}

	ret = tof_read_chip_info(tof_dev);
	if (ret) {
		XRISP_PR_ERROR("Failed to read chip information.");
		goto power_off;
	}

	tof_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	tof_dev->miscdev.name = XRISP_TOF_DRIVER_NAME;
	tof_dev->miscdev.fops = &tof_fops;
	tof_dev->miscdev.mode = 0666;
	ret = misc_register(&tof_dev->miscdev);
	if (ret) {
		XRISP_PR_ERROR("Failed to create misc device");
		goto power_off;
	}

	ret = check_rom_firmware_boot(tof_dev);
	if (ret) {
		XRISP_PR_ERROR("Failed to check rom firmware boot.");
		goto unreg_misc;
	}

	// ret = tof_load_firmware(tof_dev);
	// if (ret) {
	//	XRISP_PR_ERROR("Failed to load firmware.");
	//	goto unreg_misc;
	// }

	ret = tof_set_power_mode(tof_dev, XRISP_TOF_POWER_MODE_LP);
	if (ret) {
		XRISP_PR_ERROR("Set power mode to LP failed.");
		goto unreg_misc;
	}
	XRISP_PR_INFO("Exit spi probe.");

	return ret;

unreg_misc:
	misc_deregister(&tof_dev->miscdev);
power_off:
	(void)tof_power_off(tof_dev);
exit:
	mutex_destroy(&tof_dev->tof_mutex);
	kfree(tof_dev);
	tof_dev = NULL;
	return ret;
}

#ifdef XRISP_TOF_SPI_MODE
static void tof_remove(struct spi_device *dev)
{
	struct xrisp_tof_data *tof_dev = spi_get_drvdata(dev);
#else
static void tof_remove(struct i2c_client *dev)
{
	struct xrisp_tof_data *tof_dev = i2c_get_clientdata(dev);
#endif
	XRISP_PR_INFO("Remove spi device");
	if (!tof_dev)
		return;

	if (!IS_ERR(tof_dev->miscdev.this_device) &&
			tof_dev->miscdev.this_device != NULL) {
		misc_deregister(&tof_dev->miscdev);
	}

	(void)tof_power_off(tof_dev);
	mutex_destroy(&tof_dev->tof_mutex);
	kfree(tof_dev);
	tof_dev = NULL;
}

static const struct of_device_id dev_id[] = {
	{
		.compatible = "xring,stmvl53l8",
	},
	{},
};

#ifdef XRISP_TOF_SPI_MODE
static const struct spi_device_id spi_id[] = {
	{ XRISP_TOF_DRIVER_NAME, 0 },
	{},
};

static struct spi_driver tof_spi_driver = {
	.driver = {
		.name   = XRISP_TOF_DRIVER_NAME,
		.of_match_table = dev_id,
		.owner  = THIS_MODULE,
		},
	.probe  = tof_probe,
	.remove = tof_remove,
	.id_table = spi_id,
};
#else
static const struct i2c_device_id i2c_id[] = {
	{ XRISP_TOF_DRIVER_NAME, 0 },
	{},
};

static struct i2c_driver i2c_driver = {
	.driver = {
		.name = XRISP_TOF_DRIVER_NAME,
		.of_match_table = dev_id,
		.owner = THIS_MODULE,
	},
	.probe = tof_probe,
	.remove = tof_remove,
	.id_table = i2c_id,
};
#endif

int xrisp_multi_tof_init(void)
{
#ifdef XRISP_TOF_SPI_MODE
	return spi_register_driver(&tof_spi_driver);
#else
	return i2c_add_driver(&i2c_driver);
#endif
}

void xrisp_multi_tof_exit(void)
{
#ifdef XRISP_TOF_SPI_MODE
	spi_unregister_driver(&tof_spi_driver);
#else
	i2c_del_driver(&i2c_driver);
#endif
}

MODULE_AUTHOR("zhanghongyi <zhanghongyi@xiaomi.com>");
MODULE_DESCRIPTION("TOF Sensor Driver: stmvl53l8");
MODULE_LICENSE("GPL v2");
