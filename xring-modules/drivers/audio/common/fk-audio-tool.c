// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>

#include "fk-audio-log.h"
#include "fk-audio-tool-core.h"

struct tool_drvdata tool_data_p;

/************************************************************************
 * function: tool_set_cals
 * description: set tool param.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: set success
 *		<0: set fail
 ************************************************************************/
static int tool_set_cals(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_set_cal(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "tool_set_cal failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: tool_set_dsp_log_param
 * description: set dsp log param.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: get success
 *		<0: get fail
 ************************************************************************/
static int tool_set_dsp_log_param(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_set_log_param(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "log param set  failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: tool_set_dsp_pcm_param
 * description: set dsp pcm param.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: get success
 *		<0: get fail
 ************************************************************************/
static int tool_set_dsp_pcm_param(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_set_pcm_param(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "pcm param set failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: tool_dsp_reg_value_read
 * description: dsp reg value read.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: get success
 *		<0: get fail
 ************************************************************************/
static int tool_dsp_reg_value_read(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_reg_value_read(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "reg param set failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: tool_dsp_reg_value_write
 * description: dsp reg value write.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: get success
 *		<0: get fail
 ************************************************************************/
static int tool_dsp_reg_value_write(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_reg_value_write(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "reg param set failed.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: tool_dsp_uncache_log
 * description: dsp uncache log en.
 * parameter:
 *		int32_t tool_type : tool type.
 *		size_t tool_type_size : tool param size.
 *		void *data : tool param.
 * return:
 *		0: get success
 *		<0: get fail
 ************************************************************************/
static int tool_dsp_uncache_log(int32_t tool_type,
				size_t tool_type_size, void *data)
{
	int ret = 0;

	ret = fk_audio_tool_dsp_uncache_log(tool_type, tool_type_size, data);
	if (ret < 0) {
		ret = -1;
		AUD_LOG_ERR(AUD_COMM, "uncached log set failed.");
		return ret;
	}

	return ret;
}

static int audio_tool_fasync(int fd, struct file *file, int on)
{
	struct tool_drvdata *drvdata = fk_audio_tool_drvdata_get();

	AUD_LOG_INFO(AUD_COMM, "enter.");
	return fasync_helper(fd, file, on, &drvdata->async_queue);
}

static int audio_tool_open(struct inode *inode, struct file *f)
{
	int ret = 0;

	fk_audio_init_tool_dump_data();
	AUD_LOG_DBG(AUD_COMM, "enter.");

	return ret;
}

static int audio_tool_release(struct inode *inode, struct file *f)
{
	struct tool_drvdata *drvdata = fk_audio_tool_drvdata_get();
	int ret = 0;

	fk_audio_tool_close_msg_send();
	fk_audio_init_tool_dump_data();

	if (drvdata->async_queue)
		ret = fasync_helper(-1, f, 0, &drvdata->async_queue);

	AUD_LOG_DBG(AUD_COMM, "release enter.");
	return ret;
}

static ssize_t audio_tool_read(struct file *file, char __user *buf,
			size_t len, loff_t *ppos)
{
	int ret = 0;

	ret = fk_audio_tool_dump_read(buf, len);

	return ret;
}

static long audio_tool_shared_ioctl(struct file *file, unsigned int cmd,
							void __user *arg)
{
	int ret = 0;
	int32_t size;
	struct aud_tool_context *data = NULL;

	AUD_LOG_DBG(AUD_COMM, "audio tool io ctl enter.");

	switch (cmd) {
	case AUDIO_SET_CALIBRATION:
	case AUDIO_GET_CALIBRATION:
	case AUDIO_SET_DSP_LOG_PARAM:
	case AUDIO_DUMP_DSP_LOG:
	case AUDIO_SET_DSP_PCM_PARAM:
	case AUDIO_DUMP_DSP_PCM:
	case AUDIO_DEBUG_REG_VALUE:
	case AUDIO_DSP_UNCACHE_LOG:
		break;
	default:
		AUD_LOG_ERR(AUD_COMM, "ioctl not found.");
		return -EINVAL;
	}

	if (!arg) {
		AUD_LOG_ERR(AUD_COMM, "Invalid data pointer.");
		return -EINVAL;
	}

	if (copy_from_user(&size, arg, sizeof(size))) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		return -EINVAL;
	}

	if (size != sizeof(struct aud_tool_context)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size sent to driver: %d, min size is %lu.",
			size, sizeof(struct aud_tool_context));
		return -EINVAL;
	}

	data = kmalloc(size, GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;

	if (copy_from_user(data, arg, size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy data from user.");
		ret = -EFAULT;
		goto done;
	}

	if ((data->tool_meta.tool_type < 0) ||
		(data->tool_meta.tool_type >= TOOL_TYPE_MAX)) {
		AUD_LOG_ERR(AUD_COMM, "cal type %d is Invalid!",
			data->tool_meta.tool_type);
		ret = -EINVAL;
		goto done;
	}

	AUD_LOG_DBG(AUD_COMM, "size:%d,cmd:%d,type:%d,in size:%d,out size::%d.",
		size, cmd, data->tool_meta.tool_type, data->tool_data_in.tool_size,
		data->tool_data_out.tool_size);

	switch (cmd) {
	case AUDIO_SET_CALIBRATION:
		ret = tool_set_cals(data->tool_meta.tool_type,
			data->tool_data_in.tool_size, data->tool_data_in.tool_data);
		break;
	case AUDIO_SET_DSP_LOG_PARAM:
		ret = tool_set_dsp_log_param(data->tool_meta.tool_type,
			data->tool_data_in.tool_size, data->tool_data_in.tool_data);
		break;
	case AUDIO_SET_DSP_PCM_PARAM:
		ret = tool_set_dsp_pcm_param(data->tool_meta.tool_type,
			data->tool_data_in.tool_size, data->tool_data_in.tool_data);
		break;
	case AUDIO_DEBUG_REG_VALUE:
		if (data->tool_data_in.tool_size)
			ret = tool_dsp_reg_value_write(data->tool_meta.tool_type,
				data->tool_data_in.tool_size, data->tool_data_in.tool_data);
		else if (data->tool_data_out.tool_size)
			ret = tool_dsp_reg_value_read(data->tool_meta.tool_type,
				data->tool_data_out.tool_size, data->tool_data_out.tool_data);
		break;
	case AUDIO_DSP_UNCACHE_LOG:
		ret = tool_dsp_uncache_log(data->tool_meta.tool_type,
			data->tool_data_in.tool_size, data->tool_data_in.tool_data);
		break;
	}

done:
	kfree(data);
	return ret;
}


static long audio_tool_ioctl(struct file *f,
		unsigned int cmd, unsigned long arg)
{
	return audio_tool_shared_ioctl(f, cmd, (void __user *)arg);
}

static const struct file_operations audio_tool_fops = {
	.owner = THIS_MODULE,
	.open = audio_tool_open,
	.release = audio_tool_release,
	.read = audio_tool_read,
	.fasync = audio_tool_fasync,
	.unlocked_ioctl = audio_tool_ioctl,
};

struct miscdevice audio_tool_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "xring_audio_tool",
	.fops	= &audio_tool_fops,
};

struct tool_drvdata *fk_audio_tool_drvdata_get(void)
{
	return &tool_data_p;
}

static int fk_audio_tool_probe(struct platform_device *pdev)
{
	int ret = 0;

	tool_data_p.dev = &pdev->dev;
	dev_set_drvdata(tool_data_p.dev, &tool_data_p);
	tool_data_p.miscdev = audio_tool_misc;

	ret = misc_register(&audio_tool_misc);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "audio_cal: misc_register fail, ret=%d.", ret);
		return -1;
	}

	audio_tool_misc.parent = &pdev->dev;

	fk_audio_ipc_recv_register(AP_AUDIO_TOOL, fk_audio_tool_msg_handle, NULL);

	return ret;
}

static int fk_audio_tool_remove(struct platform_device *pdev)
{
	misc_deregister(&audio_tool_misc);
	return 0;
}


static const struct of_device_id fk_audio_tool_dt_match[] = {
	{.compatible = "xring,fk-audio-tool"},
	{}
};

static struct platform_driver fk_audio_tool_driver = {
	.probe  = fk_audio_tool_probe,
	.remove = fk_audio_tool_remove,
	.driver = {
		.name = "fk-audio-tool",
		.owner = THIS_MODULE,
		.of_match_table = fk_audio_tool_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init fk_audio_tool_init(void)
{
	fk_audio_init_tool_pdata();
	return platform_driver_register(&fk_audio_tool_driver);
}

void fk_audio_tool_exit(void)
{
	platform_driver_unregister(&fk_audio_tool_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO TOOL DRIVER");
MODULE_LICENSE("Dual BSD/GPL");
