// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "cdm", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "cdm", __func__, __LINE__

#include <linux/module.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "cam_ctrl/xrisp_cam_ctrl.h"
#include "xrisp_rproc_api.h"
#include "xrisp_cbm_api.h"
#include "xrisp_cdm.h"
#include "xrisp_log.h"
#include "xrisp_sensor_ctrl.h"
#include "cam_priv_led/xrisp_cam_privacy_led.h"
#include "cam_flash/xrisp_flash_core.h"

pid_t g_provider_pid;

static int xrisp_cdm_open(struct file *filep)
{
	int ret = 0;
	struct xrisp_cdm_data *priv = video_drvdata(filep);

	if (!priv)
		return -ENODEV;

	xrisp_sys_set_user_task();

	mutex_lock(&priv->dev_lock);
	ret = atomic_cmpxchg(&priv->dev_opened, 0, 1);
	if (ret != 0) {
		XRISP_PR_ERROR("xrisp_cdm busy, alreadly open, exit");
		goto out;
	}

	ret = v4l2_fh_open(filep);
	if (ret) {
		atomic_set(&priv->dev_opened, 0);
		XRISP_PR_ERROR("xrisp cdm fh open fail");
	} else {
		priv->eventq = filep->private_data;
		filep->private_data = priv->eventq;
		g_provider_pid = current->pid;
	}

out:
	mutex_unlock(&priv->dev_lock);
	XRISP_PR_INFO("xrisp cdm opened by %s(pid: %d), total_vm:%ld, rss:%ld", current->comm,
		      current->pid, current->mm->total_vm, get_mm_rss(current->mm));
	return ret;
}

static void xrisp_cdm_res_close(struct xrisp_cdm_data *priv)
{
	if (!priv)
		return;

	xrisp_sys_clear_user_task();
	xrisp_ipc_msgbuf_reset();
	cam_fence_close(&priv->fence_dev);

	//if isp running, hal crash
	xrisp_rproc_async_state_release();
	if (xrisp_rproc_is_running()) {
		XRISP_PR_ERROR("cdm abnormal close, hal crash");
		xrisp_rproc_shutdown();
	}
	xrisp_ocmlist_release();
	xrisp_flash_close();

	//notify shub ois power_down
	xrisp_notify_shub_ois_power_down();

	if (xrisp_cbm_buf_need_release())
		cbm_buf_release_all();
	xrisp_sensor_power_resource_clear();
	//cam_pm_resource_close();
}

static int xrisp_cdm_close(struct file *filep)
{
	int ret = 0;
	struct xrisp_cdm_data *priv = video_drvdata(filep);

	if (!priv)
		return -ENODEV;

	mutex_lock(&priv->dev_lock);
	ret = atomic_cmpxchg(&priv->dev_opened, 1, 0);
	if (ret != 1) {
		XRISP_PR_WARN("cdm close failed, open_cnt=%d", ret);
		goto out;
	}

	xrisp_cdm_res_close(priv);
	v4l2_fh_release(filep);
	filep->private_data = NULL;
	g_provider_pid = 0;
	xrisp_privacy_reminder_led_disable();

out:
	mutex_unlock(&priv->dev_lock);
	XRISP_PR_INFO("xrisp cdm closed by %s(pid: %d), total_vm:%ld, rss:%ld", current->comm,
		      current->pid, current->mm ? current->mm->total_vm : 0,
		      current->mm ? get_mm_rss(current->mm) : 0);
	return 0;
}

static unsigned int xrisp_cdm_poll(struct file *filep,
				  struct poll_table_struct *pll_table)
{
	int ret = 0;
	struct v4l2_fh *eventq = filep->private_data;

	if (!eventq)
		return -EINVAL;
	poll_wait(filep, &eventq->wait, pll_table);
	if (v4l2_event_pending(eventq))
		ret = POLLPRI;
	return ret;
}

int xrisp_cdm_subscribe_event(struct v4l2_fh *fh,
			      const struct v4l2_event_subscription *sub)
{
	XRISP_PR_INFO("xrisp subscribe event id: %d, type: %d", sub->id, sub->type);
	return v4l2_event_subscribe(fh, sub, XRING_EVENT_QUEUE_LEN, NULL);
}

int xrisp_cdm_unsubscribe_event(struct v4l2_fh *fh,
				const struct v4l2_event_subscription *sub)
{
	XRISP_PR_INFO("xrisp unsubscribe event id: %d, type: %d", sub->id, sub->type);
	return v4l2_event_unsubscribe(fh, sub);
}

static long xrisp_cdm_dev_ioctl(struct file *filep, void *fh, bool valid_prio,
				unsigned int cmd, void *arg)
{
	int ret = 0;
	struct xrisp_cdm_data *priv = video_drvdata(filep);
	struct xrisp_control_arg *cmd_arg;

	if (!priv)
		return -ENODEV;

	cmd_arg = (struct xrisp_control_arg *)arg;

	switch (cmd) {
	case XRING_INIT:
		XRISP_PR_INFO("xrisp cdm init finished");
		ret = 0;
		break;

	case XRING_FENCE: {
		XRISP_PR_INFO("xrisp_cbm_ioctl XRING_FENCE");
		ret = cam_fence_ioctl(cmd_arg);
		if (ret)
			XRISP_PR_ERROR("cam_fence_ioctl fail ret = %d", ret);
	}
	break;

	case XRING_IOC_CBM_BUF: {
		ret = xrisp_cbm_ioctl(cmd_arg);
		if (ret)
			XRISP_PR_ERROR("xrisp_cbm_ioctl fail ret = %d", ret);
	}
	break;

	case XRING_IOC_CAMERA_CMD: {
		ret = xrisp_cam_cmd_ioctl(cmd_arg);
		if (ret)
			XRISP_PR_ERROR("xrisp_cam_cmd_ioctl fail ret = %d", ret);
	} break;

	default:
		XRISP_PR_ERROR("invalid input cmd");
		return -ENOIOCTLCMD;
	}

	return ret;
}

const struct v4l2_file_operations xrisp_cdm_fops = {
	.owner  = THIS_MODULE,
	.open   = xrisp_cdm_open,
	.poll   = xrisp_cdm_poll,
	.release = xrisp_cdm_close,
	.unlocked_ioctl   = video_ioctl2,
};

const struct v4l2_ioctl_ops xrisp_cdm_ioctl_ops = {
	.vidioc_subscribe_event = xrisp_cdm_subscribe_event,
	.vidioc_unsubscribe_event = xrisp_cdm_unsubscribe_event,
	.vidioc_default = xrisp_cdm_dev_ioctl,
};

void xrisp_cdm_release(struct video_device *vdev)
{

}

static int xrisp_cdm_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct xrisp_cdm_data *priv;

	XRISP_PR_INFO("cdm probe start");

	priv = devm_kzalloc(&pdev->dev,
			    sizeof(struct xrisp_cdm_data),
			    GFP_KERNEL);
	if (priv == NULL) {
		XRISP_PR_ERROR("%s Out of memory", __func__);
		return -ENOMEM;
	}

	priv->v4l2_dev = devm_kzalloc(&pdev->dev,
			sizeof(struct v4l2_device),
			GFP_KERNEL);
	if (priv->v4l2_dev == NULL) {
		XRISP_PR_ERROR("%s Out of memory", __func__);
		return -ENOMEM;
	}

	ret = v4l2_device_register(&pdev->dev, priv->v4l2_dev);
	if (ret) {
		XRISP_PR_ERROR("v4l2 dev register fail ret = %d", ret);
		return ret;
	}

	priv->v4l2_dev->mdev = devm_kzalloc(&pdev->dev,
					    sizeof(struct media_device),
					    GFP_KERNEL);
	if (!priv->v4l2_dev->mdev) {
		XRISP_PR_ERROR("media device alloc fail");
		goto v4l_unregister;
	}
	media_device_init(priv->v4l2_dev->mdev);
	priv->v4l2_dev->mdev->dev = &pdev->dev;

	ret = media_device_register(priv->v4l2_dev->mdev);
	if (ret) {
		XRISP_PR_ERROR("media device register fail ret = %d", ret);
		goto v4l_unregister;
	}

	priv->vdev = video_device_alloc();
	if (!priv->vdev) {
		XRISP_PR_ERROR("video device register fail ret = %d", ret);
		goto media_unregister;
	}

	priv->vdev->v4l2_dev = priv->v4l2_dev;
	strscpy(priv->vdev->name, "xrisp-manager",
		sizeof(priv->vdev->name));
	priv->vdev->release = xrisp_cdm_release;
	priv->vdev->fops = &xrisp_cdm_fops;
	priv->vdev->ioctl_ops = &xrisp_cdm_ioctl_ops;
	priv->vdev->minor = -1;
	priv->vdev->vfl_type = VFL_TYPE_VIDEO;
	priv->vdev->device_caps |= V4L2_CAP_VIDEO_CAPTURE;

	video_set_drvdata(priv->vdev, priv);

	ret = video_register_device(priv->vdev, VFL_TYPE_VIDEO, -1);
	if (ret) {
		XRISP_PR_ERROR("video register fail ret = %d", ret);
		goto video_release;
	}
	priv->vdev->entity.name = video_device_node_name(priv->vdev);
	priv->vdev->entity.function = XRISP_VIDEO_DEVICE_TYPE;

	priv->dev = &pdev->dev;
	mutex_init(&priv->dev_lock);

	platform_set_drvdata(pdev, priv);

	ret = cam_fence_init(&priv->fence_dev, priv->vdev);
	if (ret) {
		XRISP_PR_ERROR("fence init failed, ret = %d", ret);
		goto video_unregister_device;
	}

	ret = xrisp_cam_ctrl_init(priv->vdev);
	if (ret) {
		XRISP_PR_ERROR("cam_ctrl init failed");
		goto video_unregister_device;
	}

	ret = cam_sysfs_init(&pdev->dev, priv->vdev);
	if (ret) {
		XRISP_PR_ERROR("cam_sysfs init failed");
		goto video_unregister_device;
	}

	ret = cam_log_init(&pdev->dev);
	if (ret) {
		XRISP_PR_ERROR("cam_sysfs init failed");
		goto video_unregister_device;
	}

	XRISP_PR_INFO("xrisp cdm probe success.");
	return 0;

video_unregister_device:
	video_unregister_device(priv->vdev);
video_release:
	video_device_release(priv->vdev);
media_unregister:
	media_device_unregister(priv->v4l2_dev->mdev);
	priv->v4l2_dev->mdev = NULL;
v4l_unregister:
	v4l2_device_unregister(priv->v4l2_dev);
	priv->v4l2_dev = NULL;

	return ret;
}

static int xrisp_cdm_remove(struct platform_device *pdev)
{
	struct xrisp_cdm_data *priv = platform_get_drvdata(pdev);

	cam_log_exit();
	cam_sysfs_exit();
	xrisp_cam_ctrl_release();
	cam_fence_exit();
	video_unregister_device(priv->vdev);
	video_device_release(priv->vdev);
	priv->vdev = NULL;
	media_device_unregister(priv->v4l2_dev->mdev);
	priv->v4l2_dev->mdev = NULL;
	v4l2_device_unregister(priv->v4l2_dev);

	XRISP_PR_INFO("xrisp cdm remove finish");
	return 0;
}

static const struct of_device_id xrisp_cdm_of_match[] = {
	{
		.compatible = "xring,cam-device-manager",
	},
	{},
};

static struct platform_driver xrisp_cdm_driver = {
	.probe	= xrisp_cdm_probe,
	.remove	= xrisp_cdm_remove,
	.driver	= {
		.name		= "cam-device-manager",
		.of_match_table	= xrisp_cdm_of_match,
	},
};

int xrisp_cdm_init(void)
{
	return platform_driver_register(&xrisp_cdm_driver);
}

void xrisp_cdm_exit(void)
{
	platform_driver_unregister(&xrisp_cdm_driver);
}

MODULE_AUTHOR("Zhangjiawei <zhangjiawei1@xiaomi.com>");
MODULE_AUTHOR("Chenhonglin <Chenhonglin@xiaomi.com>");
MODULE_AUTHOR("zhenbin <zhenbin@xiaomi.com>");
MODULE_AUTHOR("lizexin <lizexin@xiaomi.com>");
MODULE_LICENSE("GPL v2");
