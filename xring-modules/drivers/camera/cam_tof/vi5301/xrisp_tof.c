// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tof_vi5301", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tof_vi5301", __func__, __LINE__

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/pinctrl/consumer.h>
#include "xrisp_tof.h"
#include "xrisp_log.h"

#define XRISP_TOF_DRIVER_NAME "vi530x"
#define XRISP_TOF_INPUT_DISTANCE         ABS_HAT0X
#define XRISP_TOF_INPUT_CONFIDENCE       ABS_HAT0Y
#define XRISP_TOF_INPUT_PEAK             ABS_HAT1X
#define XRISP_TOF_INPUT_NOISE            ABS_HAT1Y
#define XRISP_TOF_INPUT_INTEGRAL_TIMES   ABS_HAT2X

static int tof_regulator_power(struct regulator *dev_reg, bool enable)
{
	int ret = 0;

	if (dev_reg == NULL) {
		XRISP_PR_WARN("regulator 'avdd' is not found.");
		return ret;
	}

	ret = regulator_set_voltage(dev_reg, 3000000, 3000000);
	if (ret) {
		XRISP_PR_ERROR("set regulator 'avdd' voltage failed.");
		return ret;
	}
	ret = enable ? regulator_enable(dev_reg) : regulator_disable(dev_reg);
	if (ret) {
		XRISP_PR_ERROR("regulator 'avdd' %s failed.", enable ? "enable" : "disable");
		return ret;
	}

	return ret;
}

static int __power_on(struct xrisp_tof_data *dev)
{
	int ret = 0;

	if (!dev) {
		XRISP_PR_ERROR("dev is null.");
		return -1;
	}

	ret = tof_regulator_power(dev->dev_regulator, true);
	if (ret)
		return ret;
	ret = gpio_direction_output(dev->xshut_gpio, 0);
	if (ret)
		return ret;
	mdelay(5);
	ret = gpio_direction_output(dev->xshut_gpio, 1);
	if (ret)
		return ret;
	mdelay(5);
	if (dev->irq > 0)
		enable_irq(dev->irq);
	return ret;
}

static int __power_off(struct xrisp_tof_data *dev)
{
	int ret = 0;

	if (!dev) {
		XRISP_PR_ERROR("dev is null.");
		return -1;
	}

	ret = gpio_direction_output(dev->xshut_gpio, 0);
	if (ret)
		return ret;
	ret = tof_regulator_power(dev->dev_regulator, false);
	if (ret)
		return ret;
	if (dev->irq > 0) {
		disable_irq(dev->irq);
		cancel_work_sync(&dev->tof_work);
	}

	dev->fw_loaded = false;
	return ret;
}

static int tof_power_on(struct xrisp_tof_data *dev)
{
	int ret = 0;

	if (!dev) {
		XRISP_PR_ERROR("dev is null.");
		return -1;
	}

	if (dev->power_refs != 0) {
		dev->power_refs++;
		XRISP_PR_INFO("tof has been powered on, power_refs=%d.", dev->power_refs);
		return ret;
	}

	ret = __power_on(dev);
	if (ret) {
		XRISP_PR_ERROR("power on failed, ret=%d.", ret);
		return ret;
	}

	dev->power_refs++;
	return ret;
}

static int tof_power_off(struct xrisp_tof_data *dev)
{
	int ret = 0;

	if (!dev) {
		XRISP_PR_ERROR("dev is null.");
		return -1;
	}

	if (dev->power_refs == 0) {
		XRISP_PR_INFO("tof has been powered off.");
		return ret;
	}

	if (--(dev->power_refs) != 0) {
		XRISP_PR_INFO("power off, power_refs=%d.", dev->power_refs);
		return ret;
	}

	ret = __power_off(dev);
	if (ret) {
		XRISP_PR_ERROR("power off failed, ret=%d.", ret);
		return ret;
	}

	return ret;
}

static ssize_t chip_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	return scnprintf(buf, PAGE_SIZE, "%u\n", data->chip_enable);
}

static ssize_t chip_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	mutex_lock(&data->work_mutex);
	if ((val == 1) && (data->chip_enable == 0)) {
		ret = tof_power_on(data);
		if (ret)
			goto error;
		data->chip_enable = 1;
	} else if ((val == 0) && (data->chip_enable == 1)) {
		ret = tof_power_off(data);
		if (ret)
			goto error;
		data->chip_enable = 0;
	}

	mutex_unlock(&data->work_mutex);
	XRISP_PR_ERROR("Chip enable succeed, value=%u.", val);
	return count;

error:
	mutex_unlock(&data->work_mutex);
	XRISP_PR_ERROR("Chip enable failed, value=%u.", val);
	return -EINVAL;
}

static DEVICE_ATTR_RW(chip_enable);

/* for debug */
static ssize_t enable_debug_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;
	return snprintf(buf, PAGE_SIZE, "%u\n", data->enable_debug);
}

static ssize_t enable_debug_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);
	unsigned int on = 0;

	if (!data)
		return -EINVAL;
	if (sscanf(buf, "%u\n", &on) != 1)
		return -EINVAL;

	if ((on != 0) &&  (on != 1)) {
		XRISP_PR_ERROR("invalid parameter, on=%d.", on);
		return -EINVAL;
	}

	mutex_lock(&data->work_mutex);
	data->enable_debug = on;
	mutex_unlock(&data->work_mutex);
	return count;
}

static DEVICE_ATTR_RW(enable_debug);

static ssize_t chip_init_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t chip_init_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	mutex_lock(&data->work_mutex);
	if (val == 1) {
		ret = xrisp_tof_chip_init(data);
		if (ret)
			goto error;
		data->fw_loaded = true;
	}
	mutex_unlock(&data->work_mutex);
	return count;

error:
	mutex_unlock(&data->work_mutex);
	return -EINVAL;
}

static DEVICE_ATTR_RW(chip_init);

static ssize_t period_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;
	return scnprintf(buf, PAGE_SIZE, "%u\n", data->period);
}

static ssize_t period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	mutex_lock(&data->work_mutex);
	data->period = val;
	if (xrisp_tof_set_period(data, data->period))
		XRISP_PR_ERROR("Failed to set period.");

	mutex_unlock(&data->work_mutex);
	return count;
}

static DEVICE_ATTR_RW(period);

static ssize_t capture_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t capture_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	if (val != 0 && val != 1) {
		XRISP_PR_ERROR("capture store unvalid value=%u", val);
		return -EINVAL;
	}

	mutex_lock(&data->work_mutex);
	if (val == 1)
		ret = xrisp_tof_start_continuous_measure(data);
	else
		ret = xrisp_tof_stop_continuous_measure(data);
	if (ret)
		XRISP_PR_ERROR("start/stop continuous measure failed.");

	mutex_unlock(&data->work_mutex);
	return ret ? -EINVAL : count;
}

static DEVICE_ATTR_RW(capture);

static ssize_t xtalk_calib_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;
	return scnprintf(buf, PAGE_SIZE, "%d.%u\n", data->calib_data.xtalk_cal,
			data->calib_data.xtalk_peak);
}

static ssize_t xtalk_calib_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%u\n", &val) != 1)
		return -EINVAL;

	if (val != 1) {
		XRISP_PR_ERROR("xtalk calibration store unvalid value=%u", val);
		return -EINVAL;
	}

	mutex_lock(&data->work_mutex);
	data->xtalk_mark = 1;
	ret = xrisp_tof_xtalk_calibration(data);
	if (ret) {
		XRISP_PR_ERROR("xtalk calibration failed.");
		data->xtalk_mark = 0;
		mutex_unlock(&data->work_mutex);
		return -EINVAL;
	}

	msleep(600);
	data->xtalk_mark = 0;
	mutex_unlock(&data->work_mutex);
	return count;
}

static DEVICE_ATTR_RW(xtalk_calib);

static ssize_t offset_calib_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;
	return scnprintf(buf, PAGE_SIZE, "%d\n", data->calib_data.offset_cal);
}

static ssize_t offset_calib_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	int val = 0;
	struct xrisp_tof_data *data = dev_get_drvdata(dev);

	if (!data)
		return -EINVAL;

	if (sscanf(buf, "%d\n", &val) != 1)
		return -EINVAL;

	if (val <= 0) {
		XRISP_PR_ERROR("offset calibration store unvalid value=%u", val);
		return -EINVAL;
	}

	mutex_lock(&data->work_mutex);
	data->calib_data.offset_mili = val;
	data->offset_mark = 1;
	ret = xrisp_tof_offset_calibration(data);
	if (ret) {
		XRISP_PR_ERROR("offset calibration failed.");
		data->offset_mark = 0;
		mutex_unlock(&data->work_mutex);
		return -EINVAL;
	}

	data->offset_mark = 0;
	mutex_unlock(&data->work_mutex);
	return count;
}

static DEVICE_ATTR_RW(offset_calib);

static ssize_t tof_xtalk_data_read(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct xrisp_tof_data *data = dev_get_drvdata(dev);
	void *src = (void *) &(data->calib_data);

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		XRISP_PR_ERROR("can't set calib data while disable sensor");
		mutex_unlock(&data->work_mutex);
		return -EBUSY;
	}

	if (count > sizeof(struct xrisp_cam_tof_calib_data))
		count = sizeof(struct xrisp_cam_tof_calib_data);

	memcpy(buf, src, count);
	mutex_unlock(&data->work_mutex);
	return count;
}

static ssize_t tof_xtalk_data_write(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	int ret = 0;
	struct device *dev = container_of(kobj, struct device, kobj);
	struct xrisp_tof_data *data = dev_get_drvdata(dev);
	struct xrisp_cam_tof_calib_data *calib_data =
				(struct xrisp_cam_tof_calib_data *)buf;

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		ret = -EBUSY;
		XRISP_PR_ERROR("can't set calib data while disable sensor");
		goto error;
	}

	if (count != sizeof(struct xrisp_cam_tof_calib_data))
		goto invalid;

	if (data->enable_debug) {
		XRISP_PR_INFO("xtalk_cal: %d", calib_data->xtalk_cal);
		XRISP_PR_INFO("xtalk_maxratio: %d", calib_data->maxratio);
	}

	data->calib_data.xtalk_cal = calib_data->xtalk_cal;
	data->calib_data.maxratio = calib_data->maxratio;
	ret = xrisp_tof_set_xtalk_parameter(data);
	if (ret) {
		XRISP_PR_ERROR("config xtalk calibration data failed, ret=%d.", ret);
		goto error;
	}
	mutex_unlock(&data->work_mutex);
	return count;

invalid:
	XRISP_PR_ERROR("invalid syntax");
	ret = -EINVAL;
error:
	mutex_unlock(&data->work_mutex);
	return ret;
}

static ssize_t tof_offset_data_read(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct xrisp_tof_data *data = dev_get_drvdata(dev);
	void *src = (void *) &(data->calib_data);

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		XRISP_PR_ERROR("can't set calib data while disable sensor");
		mutex_unlock(&data->work_mutex);
		return -EBUSY;
	}

	if (count > sizeof(struct xrisp_cam_tof_calib_data))
		count = sizeof(struct xrisp_cam_tof_calib_data);

	memcpy(buf, src, count);
	mutex_unlock(&data->work_mutex);
	return count;
}

static ssize_t tof_offset_data_write(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	int ret = 0;
	struct device *dev = container_of(kobj, struct device, kobj);
	struct xrisp_tof_data *data = dev_get_drvdata(dev);
	struct xrisp_cam_tof_calib_data *calib_data =
				(struct xrisp_cam_tof_calib_data *)buf;

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		ret = -EBUSY;
		XRISP_PR_ERROR("can't set calib data while disable sensor");
		goto error;
	}

	if (count != sizeof(struct xrisp_cam_tof_calib_data))
		goto invalid;

	if (data->enable_debug)
		XRISP_PR_INFO("offset config: %d", calib_data->offset_cal);

	data->calib_data.offset_cal = calib_data->offset_cal;
	mutex_unlock(&data->work_mutex);
	return count;

invalid:
	XRISP_PR_ERROR("invalid syntax");
	ret = -EINVAL;
error:
	mutex_unlock(&data->work_mutex);
	return ret;
}

static struct attribute *tof_attributes[] = {
	&dev_attr_chip_enable.attr,
	&dev_attr_enable_debug.attr,
	&dev_attr_chip_init.attr,
	&dev_attr_period.attr,
	&dev_attr_capture.attr,
	&dev_attr_xtalk_calib.attr,
	&dev_attr_offset_calib.attr,
	NULL,
};

static const struct attribute_group tof_attr_group = {
	.name = NULL,
	.attrs = tof_attributes,
};

static struct bin_attribute tof_xtalk_data_attr = {
	.attr = {
		.name = "xtalk_calib_data",
		.mode = 0644/*S_IWUGO | S_IRUGO*/,
	},
	.size = sizeof(struct xrisp_cam_tof_calib_data),
	.read = tof_xtalk_data_read,
	.write = tof_xtalk_data_write,
};

static struct bin_attribute tof_offset_data_attr = {
	.attr = {
		.name = "offset_calib_data",
		.mode = 0644/*S_IWUGO | S_IRUGO*/,
	},
	.size = sizeof(struct xrisp_cam_tof_calib_data),
	.read = tof_offset_data_read,
	.write = tof_offset_data_write,
};

static int tof_debug_sys(struct  xrisp_tof_data *data)
{
	int ret = 0;

	if (!data)
		return -EINVAL;

	data->input_dev = input_allocate_device();
	if (data->input_dev == NULL) {
		XRISP_PR_ERROR("Error allocating input_dev.");
		return -EFAULT;
	}

	data->input_dev->name = "vi530x";
	data->input_dev->id.bustype = BUS_I2C;
	input_set_drvdata(data->input_dev, data);
	set_bit(EV_ABS, data->input_dev->evbit);
	input_set_abs_params(data->input_dev, XRISP_TOF_INPUT_DISTANCE, 0, 0xffffffff, 0, 0);
	input_set_abs_params(data->input_dev, XRISP_TOF_INPUT_CONFIDENCE, 0, 0xff, 0, 0);
	input_set_abs_params(data->input_dev, XRISP_TOF_INPUT_PEAK, 0, 0xffffffff, 0, 0);
	input_set_abs_params(data->input_dev, XRISP_TOF_INPUT_NOISE, 0, 0xffffffff, 0, 0);
	input_set_abs_params(data->input_dev, XRISP_TOF_INPUT_INTEGRAL_TIMES, 0, 0xffffffff, 0, 0);

	ret = input_register_device(data->input_dev);
	if (ret) {
		XRISP_PR_ERROR("Error registering input_dev.");
		goto input_reg_err;
	}

	ret = sysfs_create_group(&data->input_dev->dev.kobj, &tof_attr_group);
	if (ret) {
		XRISP_PR_ERROR("Error creating sysfs attribute group.");
		goto sysfs_group_err;
	}

	ret = sysfs_create_bin_file(&data->input_dev->dev.kobj, &tof_xtalk_data_attr);
	if (ret) {
		ret = -ENOMEM;
		XRISP_PR_ERROR("%d error:%d", __LINE__, ret);
		goto sysfs_bin_err1;
	}

	ret = sysfs_create_bin_file(&data->input_dev->dev.kobj, &tof_offset_data_attr);
	if (ret) {
		ret = -ENOMEM;
		XRISP_PR_ERROR("%d error:%d", __LINE__, ret);
		goto sysfs_bin_err2;
	}

	return 0;

sysfs_bin_err2:
	sysfs_remove_bin_file(&data->input_dev->dev.kobj,
		&tof_xtalk_data_attr);
sysfs_bin_err1:
	sysfs_remove_group(&data->input_dev->dev.kobj,
		&tof_attr_group);
sysfs_group_err:
	input_unregister_device(data->input_dev);
input_reg_err:
	input_free_device(data->input_dev);
	return ret;
}

static void tof_work_handle(struct work_struct *work)
{
	int ret = 0;
	struct xrisp_tof_data *data = container_of(work,
		struct xrisp_tof_data, tof_work);

	if (data->xtalk_mark) {
		// XRISP_PR_INFO("Interrupt: Get Xtalk Data!");
		ret = xrisp_tof_get_xtalk_parameter(data);
		if (ret) {
			XRISP_PR_ERROR("Get xtalk data failed.");
			return;
		}
	}

	if (!data->xtalk_mark && !data->offset_mark) {
		// XRISP_PR_INFO("Interrupt: Get Range Data!");
		ret = xrisp_tof_get_measure_data(data);
		if (ret) {
			XRISP_PR_ERROR("Get Measure data failed.");
			return;
		}
		complete(&data->is_ready);
		input_report_abs(data->input_dev, XRISP_TOF_INPUT_DISTANCE,
				data->range_data.distance);
		input_report_abs(data->input_dev, XRISP_TOF_INPUT_CONFIDENCE,
				data->range_data.confidence);
		input_report_abs(data->input_dev, XRISP_TOF_INPUT_PEAK,
				data->range_data.peak);
		input_report_abs(data->input_dev, XRISP_TOF_INPUT_NOISE,
				data->range_data.noise);
		input_report_abs(data->input_dev, XRISP_TOF_INPUT_INTEGRAL_TIMES,
				data->range_data.integral_times);
		input_sync(data->input_dev);
	}
}

static irqreturn_t tof_irq_handler(int vec, void *info)
{
	struct xrisp_tof_data *data = (struct xrisp_tof_data *)info;
	// XRISP_PR_INFO("Enter Tof Interrupt!");

	if (!data || !data->fw_loaded)
		return IRQ_HANDLED;

	if (data->irq == vec)
		schedule_work(&data->tof_work);

	return IRQ_HANDLED;
}

static long tof_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	uint8_t reg_buf[2] = {0};
	struct xrisp_tof_data *data = container_of(file->private_data,
		struct xrisp_tof_data, miscdev);

	struct xrisp_control_arg argp = {0};

	if (copy_from_user(&argp, (struct xrisp_control_arg __user *)arg,
			sizeof(struct xrisp_control_arg))) {
		XRISP_PR_ERROR("Copy xrisp_control_arg failed.");
		return -EFAULT;
	}

	mutex_lock(&data->work_mutex);
	switch (argp.op_code) {
	case XRISP_CAM_TOF_IOCTL_POWER_OFF:
		ret = tof_power_off(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_POWER_ON:
		ret = tof_power_on(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_CHIP_INIT:
		ret = xrisp_tof_chip_init(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		data->fw_loaded = true;
		break;
	case XRISP_CAM_TOF_IOCTL_XTALK_CALIB:
		data->xtalk_mark = 1;
		ret = xrisp_tof_xtalk_calibration(data);
		if (ret) {
			data->xtalk_mark = 0;
			ret = -EINVAL;
			goto exit;
		}
		msleep(800);
		if (copy_to_user(u64_to_user_ptr(argp.handle), &(data->calib_data),
			sizeof(struct xrisp_cam_tof_calib_data))) {
			XRISP_PR_ERROR("Copy Xtalk calib data failed.");
			data->xtalk_mark = 0;
			ret = -EFAULT;
			goto exit;
		}
		data->xtalk_mark = 0;
		break;
	case XRISP_CAM_TOF_IOCTL_XTALK_CONFIG:
		if (copy_from_user(&(data->calib_data), u64_to_user_ptr(argp.handle),
			sizeof(struct xrisp_cam_tof_calib_data))) {
			XRISP_PR_ERROR("Copy xtalk calib data failed.");
			ret = -EFAULT;
			goto exit;
		}
		if (data->enable_debug) {
			XRISP_PR_INFO("xtalk_cal: %d", data->calib_data.xtalk_cal);
			XRISP_PR_INFO("xtalk_maxratio: %d", data->calib_data.maxratio);
		}
		ret = xrisp_tof_set_xtalk_parameter(data);
		if (ret) {
			ret = -EINVAL;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_OFFSET_CALIB:
		if (copy_from_user(&(data->calib_data.offset_mili), &(((struct xrisp_cam_tof_calib_data *)
			(u64_to_user_ptr(argp.handle)))->offset_mili), sizeof(data->calib_data.offset_mili))) {
			XRISP_PR_ERROR("Copy offset_mili failed.");
			ret = -EFAULT;
			goto exit;
		}
		data->offset_mark = 1;
		ret = xrisp_tof_offset_calibration(data);
		if (ret) {
			data->offset_mark = 0;
			ret = -EINVAL;
			goto exit;
		}
		if (copy_to_user(&(((struct xrisp_cam_tof_calib_data *)
			(u64_to_user_ptr(argp.handle)))->offset_cal), &(data->calib_data.offset_cal),
			sizeof(data->calib_data.offset_cal))) {
			XRISP_PR_ERROR("Copy offset calib data failed.");
			data->offset_mark = 0;
			ret = -EFAULT;
			goto exit;
		}
		data->offset_mark = 0;
		break;
	case XRISP_CAM_TOF_IOCTL_OFFSET_CONFIG:
		if (copy_from_user(&(data->calib_data.offset_cal),
			&(((struct xrisp_cam_tof_calib_data *)(u64_to_user_ptr(argp.handle)))->offset_cal),
			sizeof(data->calib_data.offset_cal))) {
			XRISP_PR_ERROR("Copy offset calib data failed.");
			ret = -EFAULT;
			goto exit;
		}
		if (data->enable_debug)
			XRISP_PR_ERROR("offset config: %d", data->calib_data.offset_cal);
		break;
	case XRISP_CAM_TOF_IOCTL_SINGLE_RANGE:
		reinit_completion(&data->is_ready);
		ret = xrisp_tof_single_measure(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_START_RANGE:
		reinit_completion(&data->is_ready);
		ret = xrisp_tof_start_continuous_measure(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_STOP_RANGE:
		ret = xrisp_tof_stop_continuous_measure(data);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_RANGE_DATA:
		ret = wait_for_completion_timeout(&data->is_ready,
			msecs_to_jiffies(1000 / data->period * 5));
		if (ret) {
			XRISP_PR_DEBUG("wait time= %d ms", 1000 / data->period * 5 - jiffies_to_msecs(ret));
			reinit_completion(&data->is_ready);
			ret = 0;
		} else {
			XRISP_PR_WARN("Get range data timeout.");
			reinit_completion(&data->is_ready);
			(void)debug_dump_reg(data);
			ret = -EFAULT;
			goto exit;
		}

		mutex_lock(&data->range_mutex);
		if (copy_to_user(u64_to_user_ptr(argp.handle), &(data->range_data),
			sizeof(struct xrisp_cam_tof_range_data))) {
			XRISP_PR_ERROR("Copy range data failed.");
			mutex_unlock(&data->range_mutex);
			ret = -EFAULT;
			goto exit;
		}
		mutex_unlock(&data->range_mutex);
		break;
	case XRISP_CAM_TOF_IOCTL_PERIOD:
		if (copy_from_user(&(data->period),
			u64_to_user_ptr(argp.handle), sizeof(uint32_t))) {
			XRISP_PR_ERROR("Copy period data failed.");
			ret = -EFAULT;
			goto exit;
		}
		if (data->enable_debug)
			XRISP_PR_INFO("period setting: %d", data->period);
		ret = xrisp_tof_set_period(data, data->period);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_READ_REG:
		if (copy_from_user(reg_buf, u64_to_user_ptr(argp.handle), sizeof(uint8_t) * 2)) {
			XRISP_PR_ERROR("Copy register's info failed.");
			ret = -EFAULT;
			goto exit;
		}
		ret = xrisp_tof_read_byte(data, reg_buf[0], &reg_buf[1]);
		if (ret) {
			ret = -EIO;
			goto exit;
		}
		if (copy_to_user(u64_to_user_ptr(argp.handle), reg_buf, sizeof(uint8_t) * 2)) {
			XRISP_PR_ERROR("Copy register's info failed.");
			ret = -EFAULT;
			goto exit;
		}
		break;
	case XRISP_CAM_TOF_IOCTL_WRITE_REG:
		if (copy_from_user(reg_buf, u64_to_user_ptr(argp.handle), sizeof(uint8_t) * 2)) {
			XRISP_PR_ERROR("Copy register's info failed.");
			ret = -EFAULT;
			goto exit;
		}
		ret = xrisp_tof_write_byte(data, reg_buf[0], reg_buf[1]);
		if (ret) {
			XRISP_PR_ERROR("Copy register's info failed.");
			ret = -EIO;
			goto exit;
		}
		break;
	default:
		ret = -EFAULT;
	}

exit:
	mutex_unlock(&data->work_mutex);
	return ret;
}

static int tof_open(struct inode *inode, struct file *file)
{
	XRISP_PR_INFO("open!");
	return 0;
}

static int tof_release(struct inode *inode, struct file *file)
{
	struct xrisp_tof_data *data = container_of(file->private_data,
		struct xrisp_tof_data, miscdev);

	if (data->power_refs > 0) {
		if (__power_off(data)) {
			XRISP_PR_ERROR("power off failed, power_refs=%d.", data->power_refs);
			return -1;
		}
		data->power_refs = 0;
	}

	XRISP_PR_INFO("release!");
	return 0;
}

static const struct file_operations tof_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = tof_ioctl,
	.open = tof_open,
	.release = tof_release,
};

static int tof_parse_dt(struct  xrisp_tof_data *data)
{
	struct device_node *node = NULL;

	if (!data || !data->client->dev.of_node)
		return -EINVAL;
	node = data->client->dev.of_node;

	data->inter_gpio = of_get_named_gpio(node, "inter-gpio", 0);
	if (!gpio_is_valid(data->inter_gpio)) {
		XRISP_PR_ERROR("get inter gpio: %d is invalid.", data->inter_gpio);
		return -ENODEV;
	}
	XRISP_PR_DEBUG("inter gpio: %d", data->inter_gpio);

	data->xshut_gpio = of_get_named_gpio(node, "xshut-gpio", 0);
	if (!gpio_is_valid(data->xshut_gpio)) {
		XRISP_PR_ERROR("get xshut gpio: %d is invalid.", data->xshut_gpio);
		return -ENODEV;
	}
	XRISP_PR_DEBUG("xshut gpio: %d", data->xshut_gpio);

	if (of_parse_phandle(node, "avdd-supply", 0)) {
		data->dev_regulator = devm_regulator_get(&data->client->dev, "avdd");
		if (IS_ERR_OR_NULL(data->dev_regulator)) {
			XRISP_PR_ERROR("regulator get 'avdd' failed.");
			return -ENODEV;
		}
		XRISP_PR_DEBUG("regulator get 'avdd' success.");
	} else {
		XRISP_PR_INFO("regulator 'avdd' is not found.");
		data->dev_regulator = NULL;
	}

	return  0;
}

static int tof_setup(struct  xrisp_tof_data *data)
{
	int ret = 0;
	int irq = 0;
	uint8_t buf = 0;
	uint8_t chipid[3] = {0};
	uint32_t ChipVersion = 0;

	if (!data)
		return -EINVAL;

	if (!gpio_is_valid(data->inter_gpio) || !gpio_is_valid(data->xshut_gpio))
		return -ENODEV;

	ret = gpio_request(data->xshut_gpio, "xshut gpio");
	if (ret < 0) {
		XRISP_PR_ERROR("xshut gpio request failed.");
		return -EINVAL;
	}

	ret = gpio_request(data->inter_gpio, "inter gpio");
	if (ret < 0) {
		XRISP_PR_ERROR("inter gpio request failed.");
		ret = -EINVAL;
		goto exit_free_xshut;
	}

	ret = gpio_direction_input(data->inter_gpio);
	if (ret) {
		XRISP_PR_ERROR("inter gpio direction input failed.");
		ret = -EINVAL;
		goto exit_free_inter;
	}

	irq = gpio_to_irq(data->inter_gpio);
	if (irq < 0) {
		XRISP_PR_ERROR("fail to map gpio=%d to irq=%d.", data->inter_gpio, irq);
		ret = -EINVAL;
		goto exit_free_inter;
	} else {
		XRISP_PR_INFO("request irq: %d.", irq);
		ret = request_threaded_irq(irq,  NULL, tof_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "vi530x_interrupt", (void *)data);
		if (ret) {
			XRISP_PR_ERROR("Register interrupt failed, ret=%d.", ret);
			goto exit_free_inter;
		}
	}

	data->irq = irq;
	disable_irq(data->irq);
	data->fw_loaded = false;
	init_completion(&data->is_ready);
	data->power_refs = 0;

	if (tof_power_on(data)) {
		ret = -EINVAL;
		goto exit_free_irq;
	}

	if (xrisp_tof_read_multibytes(data, XRISP_TOF_REG_CHIPID_BASE, chipid, 3)) {
		ret = tof_power_off(data);
		ret = -EINVAL;
		goto exit_free_irq;
	}
	ChipVersion = (chipid[1] << 16) + (chipid[0] << 8) + chipid[2];

	if (xrisp_tof_read_byte(data, XRISP_TOF_REG_DEV_ADDR, &buf)) {
		ret = tof_power_off(data);
		ret = -EINVAL;
		goto exit_free_irq;
	}

	if (tof_power_off(data)) {
		ret = -EINVAL;
		goto exit_free_irq;
	}

	if (buf != XRISP_TOF_CHIP_ADDR) {
		XRISP_PR_ERROR("Read chip version failed, ChipVersion=0x%x, ChipAddr = 0x%x",
				ChipVersion, buf);
		ret = -EFAULT;
		goto exit_free_irq;
	}
	XRISP_PR_INFO("Read chip version successed, ChipVersion=0x%x, ChipAddr = 0x%x",
			ChipVersion, buf);

	data->miscdev.minor = MISC_DYNAMIC_MINOR;
	data->miscdev.name = "vi530x";
	data->miscdev.fops = &tof_fops;

	if (misc_register(&data->miscdev) != 0) {
		XRISP_PR_ERROR("Could not register miscdev for vi5301 sensor.");
		ret = -ENOMEM;
		goto exit_free_irq;
	}
	data->period = 30;
	data->enable_debug = 0;

	struct pinctrl *tof_pinctrl = devm_pinctrl_get(&data->client->dev);

	if (IS_ERR_OR_NULL(tof_pinctrl)) {
		XRISP_PR_ERROR("get pinctrl fail, ret = %ld", PTR_ERR(tof_pinctrl));
		ret = -EFAULT;
		goto exit_misc_dereg;
	}

	struct pinctrl_state *pinctrl_intr = pinctrl_lookup_state(tof_pinctrl,
			"interrupt_default");
	if (IS_ERR_OR_NULL(pinctrl_intr)) {
		XRISP_PR_ERROR("lookup pinctrl state error!");
		ret = -EFAULT;
		goto exit_misc_dereg;
	}

	if (pinctrl_select_state(tof_pinctrl, pinctrl_intr)) {
		XRISP_PR_ERROR("select state switch failed");
		ret = -EFAULT;
		goto exit_misc_dereg;
	}

	return 0;

exit_misc_dereg:
	misc_deregister(&data->miscdev);
exit_free_irq:
	free_irq(data->irq, data);
	data->irq = -1;
exit_free_inter:
	gpio_free(data->inter_gpio);
	data->inter_gpio = -1;
exit_free_xshut:
	gpio_free(data->xshut_gpio);
	data->xshut_gpio = -1;

	return ret;
}


static int xrisp_tof_probe(struct i2c_client *client)
{
	int ret  = 0;
	struct xrisp_tof_data *vi530x_data = NULL;

	XRISP_PR_DEBUG("probe start!");
	vi530x_data = kzalloc(sizeof(struct xrisp_tof_data), GFP_KERNEL);
	if (!vi530x_data) {
		XRISP_PR_ERROR("Allocat memory failed.");
		return -ENOMEM;
	}
	if (!client->dev.of_node) {
		kfree(vi530x_data);
		return -EINVAL;
	}
	/* setup device data */
	vi530x_data->dev_name = dev_name(&client->dev);
	vi530x_data->client = client;
	i2c_set_clientdata(client, vi530x_data);
	mutex_init(&vi530x_data->work_mutex);
	mutex_init(&vi530x_data->range_mutex);
	INIT_WORK(&vi530x_data->tof_work, tof_work_handle);

	ret = tof_parse_dt(vi530x_data);
	if (ret) {
		XRISP_PR_ERROR("vi5301 parse dts failed.");
		goto exit_error;
	}

	ret = tof_setup(vi530x_data);
	if (ret) {
		XRISP_PR_ERROR("vi5301 setup failed.");
		goto exit_error;
	}

	ret = tof_debug_sys(vi530x_data);
	if (ret)
		XRISP_PR_WARN("vi5301 debug sysfs create failed.");

	return 0;

exit_error:
	mutex_destroy(&vi530x_data->work_mutex);
	mutex_destroy(&vi530x_data->range_mutex);
	i2c_set_clientdata(client, NULL);
	kfree(vi530x_data);
	return ret;
}

static void xrisp_tof_remove(struct i2c_client *client)
{
	int ret = 0;
	struct xrisp_tof_data *data = i2c_get_clientdata(client);

	if (!data)
		return;

	if (tof_power_off(data))
		XRISP_PR_ERROR("power off failed.");

	if (data->input_dev) {
		XRISP_PR_DEBUG("unregister sysfs dev.");
		sysfs_remove_group(&data->input_dev->dev.kobj, &tof_attr_group);
		sysfs_remove_bin_file(&data->input_dev->dev.kobj, &tof_xtalk_data_attr);
		sysfs_remove_bin_file(&data->input_dev->dev.kobj, &tof_offset_data_attr);
		input_unregister_device(data->input_dev);
		input_free_device(data->input_dev);
	}

	if (!IS_ERR(data->miscdev.this_device) && data->miscdev.this_device != NULL) {
		XRISP_PR_DEBUG("unregister misc dev.");
		misc_deregister(&data->miscdev);
	}

	if (data->xshut_gpio > 0) {
		ret = gpio_direction_output(data->xshut_gpio, 0);
		if (ret)
			XRISP_PR_INFO("fail to set xshut_gpio=0");
		gpio_free(data->xshut_gpio);
	}

	if (data->inter_gpio > 0)
		gpio_free(data->inter_gpio);

	if (data->irq > 0)
		free_irq(data->irq, data);

	i2c_set_clientdata(client, NULL);
	mutex_destroy(&data->work_mutex);
	mutex_destroy(&data->range_mutex);
	kfree(data);
}

static const struct i2c_device_id tof_dev_id[] = {
	{ XRISP_TOF_DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, tof_dev_id);

static const struct of_device_id tof_dt_match[] = {
	{.compatible = "xring,xrisp_vi530x",},
	{},
};
MODULE_DEVICE_TABLE(of, tof_dt_match);

struct i2c_driver xrisp_tof_driver = {
	.driver  = {
		.name = XRISP_TOF_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = tof_dt_match,
	},
	.probe = xrisp_tof_probe,
	.remove = xrisp_tof_remove,
	.id_table = tof_dev_id,
};

int xrisp_tof_init(void)
{
	return i2c_add_driver(&xrisp_tof_driver);
}

void xrisp_tof_exit(void)
{
	i2c_del_driver(&xrisp_tof_driver);
}

MODULE_AUTHOR("zhanghongyi <zhanghongyi@xiaomi.com>");
MODULE_DESCRIPTION("TOF Sensor Driver: vi5301");
MODULE_LICENSE("GPL v2");
