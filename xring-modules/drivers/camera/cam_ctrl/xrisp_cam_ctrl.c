// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "cam_ctrl", __func__, __LINE__

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/bitops.h>
#include <linux/remoteproc.h>
#include <linux/string.h>
#include <soc/xring/xr_timestamp.h>
#include <soc/xring/ocm_wrapper.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#include "xrisp_debug.h"
#include "cam_cbm/xrisp_cbm.h"
#include "cam_cdm/xrisp_cdm.h"
#include "cam_rproc/xrisp_rproc.h"
#include "xrisp_clk_api.h"
#include "xrisp_cam_ctrl.h"
#include "xrisp_camctrl_ipcm.h"
#include "xrisp_cbm_api.h"
#include "xrisp_ipcm_api.h"
#include "xrisp_pm_api.h"
#include "xrisp_rproc_api.h"
#include "cam_flash/xrisp_flash_core.h"
#include "cam_rpc/xrisp_cam_rpc_service.h"
#include "xrisp_sensor_ctrl.h"
#include "xrisp_log.h"
#include "xrisp_common.h"

struct xrisp_cam_ctrl g_cam_ctrl;

int xrisp_copy_cmdop(struct xrisp_control_arg *cmd, void *xrisp_op, size_t size)
{
	if (cmd == NULL || xrisp_op == NULL) {
		XRISP_PR_ERROR("input parameter addr is NULL");
		return -EINVAL;
	}

	if (cmd->size != size) {
		XRISP_PR_ERROR("op_code %d: cmd size not match", cmd->op_code);
		return -EINVAL;
	}

	if (copy_from_user(xrisp_op, u64_to_user_ptr(cmd->handle), size) != 0) {
		XRISP_PR_ERROR("op_code %d: failed copying from user", cmd->op_code);
		return -EFAULT;
	}

	return 0;
}

int xrisp_drv_hal_sharebuf_register(__u64 buf_handle)
{
	uintptr_t kva;
	struct cam_buf_desc desc;
	size_t size = 0;
	uint32_t msgbuf_num = 0;

	if (g_cam_ctrl.is_msgbuf_register == true) {
		XRISP_PR_ERROR("sharebuf has been registered, exit");
		return -EINVAL;
	}

	if (cbm_buf_get_by_handle(buf_handle, &desc, &size)) {
		XRISP_PR_ERROR("get sharebuf handle failed");
		return -EINVAL;
	}

	if (desc.kvaddr == 0) {
		XRISP_PR_ERROR("get msgbuf kva is null");
		return -EINVAL;
	}
	kva = desc.kvaddr;
	XRISP_PR_DEBUG("get sharebuf kva 0x%pK", (void *)desc.kvaddr);

	/* get buf num */
	msgbuf_num = size - (sizeof(struct xrisp_msg_bitmap));
	msgbuf_num = msgbuf_num / MSG_BUF_SIZE;
	g_cam_ctrl.msg_buf_num = msgbuf_num;
	XRISP_PR_DEBUG("drv_hal sharebuf num = %d", msgbuf_num);

	/* bitmap init*/
	g_cam_ctrl.buf_handle = buf_handle;
	g_cam_ctrl.p_msg_bitmap = (struct xrisp_msg_bitmap *)kva;
	g_cam_ctrl.p_msg_buf = (struct xrisp_msg_buf *)(kva + sizeof(struct xrisp_msg_bitmap));

	bitmap_zero(g_cam_ctrl.p_msg_bitmap->bitmap, msgbuf_num);
	g_cam_ctrl.is_msgbuf_register = true;

	return 0;
}

void pr_msgbuf_status(struct xrisp_msg_bitmap *p_map)
{
	int no_return_num = 0;
	int i = 0;

	if (p_map) {
		for (i = 0; i < MSG_BUF_BITMAP_NUM; i++)
			no_return_num += hweight_long(p_map->bitmap[i]);
		XRISP_PR_INFO("unreport sharebuf num = %d", no_return_num);
	}
}

int xrisp_send_event_to_hal(void *buffer, uint32_t len, uint32_t event_type)
{
	int idx = 0;
	struct v4l2_event event;
	struct xrisp_msg_buf *point_buf;
	static uint32_t full_err_cnt;

	if (!xrisp_rproc_is_running())
		return 0;

	if (len >= RPMSG_BUF_MAXLEN || !buffer) {
		XRISP_PR_ERROR("input buffer is invalid");
		return -EINVAL;
	}

	spin_lock(&g_cam_ctrl.event_lock);
	if (unlikely(!g_cam_ctrl.is_msgbuf_register)) {
		spin_unlock(&g_cam_ctrl.event_lock);
		XRISP_PR_INFO("msgbuf unregister, send event cancel");
		return -EINVAL;
	}

	idx = find_first_zero_bit(g_cam_ctrl.p_msg_bitmap->bitmap, g_cam_ctrl.msg_buf_num);
	if (idx >= g_cam_ctrl.msg_buf_num) {
		spin_unlock(&g_cam_ctrl.event_lock);

		if ((full_err_cnt++ % 20) == 0)
			XRISP_PR_ERROR("no free msg_buf to store, FW2AP failed, err_cnt=%d, idx=%d",
				       full_err_cnt, idx);

		return -ENOMEM;
	}

	set_bit(idx, g_cam_ctrl.p_msg_bitmap->bitmap);

	/* fw2hal msg are copied to shared msg_memory */
	point_buf = g_cam_ctrl.p_msg_buf + idx;
	memcpy(point_buf, buffer, len);
	spin_unlock(&g_cam_ctrl.event_lock);

	full_err_cnt = 0;

	/* send cam v4l2 event, no free remove oldest*/
	event.id = ISP_MSG_V4L2_EVENT_ID;
	event.u.data[0] = idx;
	event.type = event_type;

	v4l2_event_queue(g_cam_ctrl.vdev, &event);

	return 0;
}

static int xrisp_wait_isp_service_online(uint32_t msecs)
{
	unsigned long timeout;

	timeout = wait_for_completion_timeout(&g_cam_ctrl.isp_service_online,
					      msecs_to_jiffies(msecs));
	if (timeout)
		XRISP_PR_DEBUG("isp service completion use %d ms",
			       msecs - jiffies_to_msecs(timeout));
	else {
		XRISP_PR_ERROR("wait isp service online timeout");
		return -EINVAL;
	}
	return 0;
}

static int xrisp_isp_power_up(struct xrisp_power_up *powerup_set)
{
	int ret = 0;
	KTIME_DEBUG_DEFINE_START();

	if (powerup_set->scene >= XRISP_BOOT_SCENE_MAX) {
		XRISP_PR_ERROR("scene %d is invalid", powerup_set->scene);
		return -EINVAL;
	}

	ipc_trace_cnt_init();
	reinit_completion(&g_cam_ctrl.isp_service_online);
	xrisp_store_pipergltr_set(powerup_set->domain, MAX_XISP_DOMAIN_NUM);
	ret = xrisp_rproc_async_boot(powerup_set->scene);
	if (ret)
		return ret;

	ret = xrisp_rpmsg_wait_complete(ISP_RPSMG_WAIT_TIME_MS);
	if (ret)
		goto isp_powerdown;

	xrisp_rpmsg_register();

	ret = xrisp_wait_isp_service_online(ISP_SERVICE_WAIT_TIME_MS);
	if (ret)
		goto isp_powerdown;

	KTIME_DEBUG_TIMEOUT_CHECK(ISP_POWER_ON_TIME_MAX);
	return 0;

isp_powerdown:
	if (xrisp_rproc_is_running())
		xrisp_rproc_shutdown();
	return ret;
}

void xrisp_sensor_power_resource_clear(void)
{
	int id;
	enum cam_power_type type;

	for (id = 0; id < CAM_SENSOR_NUM_MAX; id++) {
		for (type = POWER_TYPE_SENSOR; type < POWER_TYPE_MAX; type++)
			xrisp_sensor_power_down_force(id, type);
	}
}

int xrisp_cam_cmd_ioctl(void *arg)
{
	int ret = 0;
	struct xrisp_control_arg *cmd = NULL;
	uint32_t log_info = EDR_RAMLOG | EDR_LOGCAT | EDR_KERNEL_KMSG | EDR_OFFLINELOG;
	ktime_t ioctl_start, ioctl_use;

	if (arg == NULL) {
		XRISP_PR_ERROR("input cmd is invalid");
		return -EINVAL;
	}

	ioctl_start = ktime_get();
	cmd = (struct xrisp_control_arg *)arg;
	switch (cmd->op_code) {
	case CAMERA_IOC_CMD_SENSOR_POWER_UP: {
		void *sensor_set = NULL;

		if (cmd->size > POWER_SET_SIZE_MAX) {
			XRISP_PR_ERROR("sensor power up cmd->size > %d err\n",
			       POWER_SET_SIZE_MAX);
			return -EINVAL;
		}
		sensor_set = kzalloc(cmd->size, GFP_KERNEL);
		ret = xrisp_copy_cmdop(cmd, sensor_set, cmd->size);
		if (ret < 0) {
			XRISP_PR_ERROR("copy_cmdop data from user error!");
			kfree(sensor_set);
			return ret;
		}
		if (((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id >
				SENSOR_ID_MAX
			|| ((struct xrisp_sensor_power_ctrl *)sensor_set)->count <=
				0
			|| ((struct xrisp_sensor_power_ctrl *)sensor_set)->count >
			    SENSOR_SEQ_TYPE_MAX) {
			XRISP_PR_ERROR("copy_cmdop data from user error!");
			kfree(sensor_set);
			return -EINVAL;
		}
		XRISP_PR_DEBUG("sensor_power_up, sensor id = %d, count = %d type = %d\n",
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id,
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->count,
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->power_type);
		if (trigger_edr_report == EDR_SENSOR_POWER_UP_ERROR) {
			edr_drv_submit_api(EDR_MCU, EDR_SENSOR_POWER_UP_ERROR, log_info, 1, 0);
			trigger_edr_report = 0;
			XRISP_PR_ERROR("CAMERA_IOC_CMD_SENSOR_POWER_UP fail, sensor_id=%d",
			       ((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id);
			kfree(sensor_set);
			return -ENOMEM;
		}
		ret = xrisp_sensor_power_up((struct xrisp_sensor_power_ctrl *)sensor_set);
		if (ret < 0) {
			edr_drv_submit_api(EDR_MCU, EDR_SENSOR_POWER_UP_ERROR, log_info, 1, 0);
			XRISP_PR_ERROR("CAMERA_IOC_CMD_SENSOR_POWER_UP fail, sensor_id=%d",
			       ((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id);
			kfree(sensor_set);
			ret = -EFAULT;
			goto cam_ioctl_out;
		}

		XRISP_PR_INFO("CAMERA_IOC_CMD_SENSOR_POWER_UP success, sensor_id=%d",
			((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id);
		kfree(sensor_set);
	} break;

	case CAMERA_IOC_CMD_SENSOR_POWER_DOWN: {
		void *sensor_set = NULL;

		if (cmd->size > POWER_SET_SIZE_MAX) {
			XRISP_PR_ERROR("sensor power down cmd->size > %d err\n",
			       POWER_SET_SIZE_MAX);
			return -EINVAL;
		}
		sensor_set = kzalloc(cmd->size, GFP_KERNEL);
		ret = xrisp_copy_cmdop(cmd, sensor_set, cmd->size);
		if (ret < 0) {
			XRISP_PR_ERROR("copy_cmdop data from user error!");
			kfree(sensor_set);
			return ret;
		}
		if (((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id >
				SENSOR_ID_MAX
			|| ((struct xrisp_sensor_power_ctrl *)sensor_set)->count <=
				0
			|| ((struct xrisp_sensor_power_ctrl *)sensor_set)->count >
			    SENSOR_SEQ_TYPE_MAX) {
			XRISP_PR_ERROR("copy_cmdop data from user error!");
			kfree(sensor_set);
			return -EINVAL;
		}
		XRISP_PR_DEBUG("sensor_power_down, sensor id = %d, count = %d type = %d\n",
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id,
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->count,
			 ((struct xrisp_sensor_power_ctrl *)sensor_set)->power_type);
		ret = xrisp_sensor_power_down((struct xrisp_sensor_power_ctrl *)sensor_set);
		if (ret < 0) {
			XRISP_PR_ERROR("CAMERA_IOC_CMD_SENSOR_POWER_DOWN fail, sensor_id=%d",
			       ((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id);
			kfree(sensor_set);
			ret = -EFAULT;
			goto cam_ioctl_out;
		}
		XRISP_PR_INFO("CAMERA_IOC_CMD_SENSOR_POWER_DOWN success, sensor_id=%d",
			((struct xrisp_sensor_power_ctrl *)sensor_set)->sensor_id);
		kfree(sensor_set);
	} break;

	case CAMERA_IOC_CMD_ISP_POWER_UP: {
		struct xrisp_power_up isp_powerup_set;

		ret = xrisp_copy_cmdop(cmd, &isp_powerup_set, sizeof(struct xrisp_power_up));
		if (ret)
			return ret;

		XRISP_PR_INFO("CAMERA_IOC_CMD_ISP_POWER_UP start");
		if (trigger_edr_report == EDR_FW_BOOT_ERROR) {
			edr_drv_submit_api(EDR_MCU, EDR_FW_BOOT_ERROR, log_info, 1, 0);
			trigger_edr_report = 0;
			XRISP_PR_ERROR("CAMERA_IOC_CMD_ISP_POWER_UP failed");
			return -ENOMEM;
		}

		mutex_lock(&g_cam_ctrl.ctrl_lock);
		ret = xrisp_isp_power_up(&isp_powerup_set);
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret) {
			edr_drv_submit_api(EDR_MCU, EDR_FW_BOOT_ERROR, log_info, 1, 0);
			XRISP_PR_ERROR("CAMERA_IOC_CMD_ISP_POWER_UP failed");
		} else
			XRISP_PR_INFO("CAMERA_IOC_CMD_ISP_POWER_UP success");
	} break;

	case CAMERA_IOC_CMD_ISP_POWER_DOWN: {
		struct xrisp_power_down isp_powerdown_set;

		ret = xrisp_copy_cmdop(cmd, &isp_powerdown_set, sizeof(struct xrisp_power_down));
		if (ret)
			return ret;

		XRISP_PR_INFO("CAMERA_IOC_CMD_ISP_POWER_DOWN start");
		mutex_lock(&g_cam_ctrl.ctrl_lock);
		ret = xrisp_rproc_async_shutdown(!!(isp_powerdown_set.direct_powerdown));
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret)
			XRISP_PR_ERROR("CAMERA_IOC_CMD_ISP_POWER_DOWN failed");
		else
			XRISP_PR_INFO("CAMERA_IOC_CMD_ISP_POWER_DOWN success");
	} break;

	case CAMERA_IOC_CMD_DVFS: {
		static const char *const dvfs_level[] = { "high", "medium", "low", "aon" };
		struct xrisp_dvfs dvfs;

		ret = xrisp_copy_cmdop(cmd, &dvfs, sizeof(struct xrisp_dvfs));
		if (ret)
			return ret;

		if (dvfs.dvfs_set >= XRISP_CLK_RATE_MAX) {
			XRISP_PR_ERROR("input dvfs_set invalid");
			return -EINVAL;
		}

		mutex_lock(&g_cam_ctrl.ctrl_lock);
		if (unlikely(!xrisp_rproc_is_running())) {
			XRISP_PR_ERROR("dvfs set failed, dvfs setting requires isp power on");
			ret = -ENODEV;
		} else
			ret = xrisp_clk_api_set_rate_by_mask(
				XRISP_CRG_CLK, (1 << CLK_ISP_CRG_OUT_MAX) - 1, dvfs.dvfs_set);
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret)
			XRISP_PR_ERROR("CAMERA_IOC_CMD_DVFS set dvfs level %s failed, ret = %d",
				       dvfs_level[dvfs.dvfs_set], ret);
		else
			XRISP_PR_DEBUG("CAMERA_IOC_CMD_DVFS set dvfs level %s success",
				       dvfs_level[dvfs.dvfs_set]);
	} break;

	case CAMERA_IOC_CMD_ISP_RPC_SEND:
	case CAMERA_IOC_CMD_ISP_MSG_SEND: {
		struct xrisp_msg_send msg;
		u8 topic;

		ret = xrisp_copy_cmdop(cmd, &msg, sizeof(struct xrisp_msg_send));
		if (ret)
			return ret;

		if (msg.size == 0 | msg.size >= RPMSG_BUF_MAXLEN) {
			XRISP_PR_ERROR("input msg size is err");
			return -EFAULT;
		}

		if (cmd->op_code == CAMERA_IOC_CMD_ISP_RPC_SEND)
			topic = TOPIC_ISP_RPC;
		else
			topic = TOPIC_PPM;

		mutex_lock(&g_cam_ctrl.ctrl_lock);
		ret = xrisp_msg_send(msg.buffer, msg.size, topic);
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret)
			XRISP_PR_ERROR(
				"CAMERA_IOC_CMD_ISP_MSG_SEND fail, AP2FW send ppm-msg failed, msg_id=%llu, msg_size=%d",
				msg.msg_id, msg.size);
	} break;

	case CAMERA_IOC_CMD_MSG_BUFFER_REGISTER: {
		struct xrisp_msg_buffer_register buf_reg;

		ret = xrisp_copy_cmdop(cmd, &buf_reg, sizeof(struct xrisp_msg_buffer_register));
		if (ret)
			return ret;

		mutex_lock(&g_cam_ctrl.ctrl_lock);
		ret = xrisp_drv_hal_sharebuf_register(buf_reg.buf_handle);
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret)
			XRISP_PR_INFO("CAMERA_IOC_CMD_MSG_BUFFER_REGISTER failed");
		else
			XRISP_PR_INFO("CAMERA_IOC_CMD_MSG_BUFFER_REGISTER success, buf_handle=%llx",
				      buf_reg.buf_handle);
	} break;

	case CAMERA_IOC_CMD_MSG_BUFFER_UNREGISTER: {
		struct xrisp_msg_buffer_unregister buf_reg;

		ret = xrisp_copy_cmdop(cmd, &buf_reg, sizeof(struct xrisp_msg_buffer_unregister));
		if (ret)
			return ret;

		mutex_lock(&g_cam_ctrl.ctrl_lock);
		if (buf_reg.buf_handle != g_cam_ctrl.buf_handle) {
			XRISP_PR_ERROR("intput handle not match register handle, unregister failed, %llx != %llx",
				buf_reg.buf_handle, g_cam_ctrl.buf_handle);
			ret = -EINVAL;
			goto buf_unregister_out;
		}

		if (g_cam_ctrl.is_msgbuf_register != true) {
			XRISP_PR_ERROR("share msgbuf not register, unregister failed");
			ret = -EINVAL;
			goto buf_unregister_out;
		}

		pr_msgbuf_status(g_cam_ctrl.p_msg_bitmap);
		xrisp_ipc_msgbuf_reset();

buf_unregister_out:
		mutex_unlock(&g_cam_ctrl.ctrl_lock);

		if (ret)
			XRISP_PR_ERROR("CAMERA_IOC_CMD_MSG_BUFFER_UNREGISTER failed");
		else
			XRISP_PR_INFO("CAMERA_IOC_CMD_MSG_BUFFER_UNREGISTER success");
	} break;

	case CAMERA_IOC_CMD_GET_TIMESTAMP: {
		struct xrisp_timestamp timestamp;

		if (cmd->size != sizeof(timestamp)) {
			XRISP_PR_ERROR("failed check cmd size %d", cmd->size);
			return -EINVAL;
		}

		timestamp.time_ns = xr_timestamp_gettime();

		if (timestamp.time_ns == 0) {
			XRISP_PR_ERROR("failed get timestamp");
			return -EFAULT;
		}

		if (copy_to_user(u64_to_user_ptr(cmd->handle), &timestamp, cmd->size) != 0) {
			XRISP_PR_ERROR("failed copying to user");
			ret = -EFAULT;
		}
	} break;

	case CAMERA_IOC_CMD_FLASH: {
		struct xrisp_flash_connect_cmd flash_cmd;

		XRISP_PR_DEBUG("into flash start\n");
		if ((cmd->size) > sizeof(struct xrisp_flash_connect_cmd)) {
			XRISP_PR_ERROR("invalid arg size\n");
			return -EOVERFLOW;
		}
		ret = xrisp_copy_cmdop(cmd, &flash_cmd, cmd->size);
		if (ret)
			return -EINVAL;

		ret = xrisp_flash_parse_command(&flash_cmd);
		if (ret < 0) {
			XRISP_PR_ERROR("flash_cmd error");
			ret = -EFAULT;
			goto cam_ioctl_out;
		}

		if (flash_cmd.header.cmdType == FlashCmdTypeQueryCurrent) {
			if (copy_to_user(u64_to_user_ptr(cmd->handle), &flash_cmd,
					 cmd->size) != 0) {
				XRISP_PR_ERROR("fail copy to user");
				ret = -EFAULT;
				goto cam_ioctl_out;
			}
			XRISP_PR_INFO("len1_current = %d, len2_current = %d\n",
				      flash_cmd.upayload.querry_current.currentMilliampere[0],
				      flash_cmd.upayload.querry_current.currentMilliampere[1]);
		}
		XRISP_PR_INFO("CAMERA_IOC_CMD_FLASH success, sensor_id = %d\n",
			      flash_cmd.sensor_id);
	} break;

	case CAMERA_IOC_CMD_GET_OCM_BUF: {
		struct ocm_buffer *buf;
		struct ocm_buf_list *buf_list;

		if (cmd->size != sizeof(struct xrisp_ocm_buf_info)) {
			XRISP_PR_ERROR("failed check cmd size %d", cmd->size);
			return -EINVAL;
		}

		buf_list = kmalloc(sizeof(*buf_list), GFP_KERNEL);
		if (!buf_list)
			return -ENOMEM;

		if (copy_from_user(&buf_list->buf_info, u64_to_user_ptr(cmd->handle), cmd->size) !=
		    0) {
			XRISP_PR_ERROR("failed copying from user");
			kfree(buf_list);
			return -EFAULT;
		}

		mutex_lock(&g_cam_ctrl.ocm_list_mutex);
		buf = ocm_buf_get(buf_list->buf_info.fd);
		if (!buf) {
			XRISP_PR_ERROR("fail get ocm buf for fd= %d", buf_list->buf_info.fd);
			kfree(buf_list);
			mutex_unlock(&g_cam_ctrl.ocm_list_mutex);
			return -EINVAL;
		}

		buf_list->buf_info.hwaddr = xrisp_rproc_ocm_pa_to_da(buf->addr, buf->size);
		buf_list->buf_info.size = buf->size;

		ret = xrisp_rproc_ocm_map(buf->addr, buf->size);
		if (ret) {
			kfree(buf_list);
			mutex_unlock(&g_cam_ctrl.ocm_list_mutex);
			XRISP_PR_ERROR("xrisp_rproc_ocm_map failed");
			return -EFAULT;
		}

		list_add_tail(&buf_list->node, &g_cam_ctrl.ocm_buf_list);
		mutex_unlock(&g_cam_ctrl.ocm_list_mutex);

		xrisp_be_ocm_link();

		if (copy_to_user(u64_to_user_ptr(cmd->handle), &buf_list->buf_info, cmd->size) !=
		    0) {
			XRISP_PR_ERROR("failed copying to user");
			return -EFAULT;
		}
		XRISP_PR_INFO("Got ocm fd:%d, hwaddr:0x%llx, size:%d",
			buf_list->buf_info.fd, buf_list->buf_info.hwaddr, buf_list->buf_info.size);
	} break;

	case CAMERA_IOC_CMD_PUT_OCM_BUF: {
		struct xrisp_ocm_buf_info buf_info;
		struct ocm_buf_list *buf_list_pos = NULL, *buf_list_t = NULL;
		bool find_ocm_fd = false;

		if (cmd->size != sizeof(buf_info)) {
			XRISP_PR_ERROR("failed check cmd size %d", cmd->size);
			return -EINVAL;
		}
		if (copy_from_user(&buf_info, u64_to_user_ptr(cmd->handle), cmd->size) !=
		    0) {
			XRISP_PR_ERROR("failed copying from user");
			return -EFAULT;
		}

		mutex_lock(&g_cam_ctrl.ocm_list_mutex);
		list_for_each_entry_safe(buf_list_pos, buf_list_t,
					 &g_cam_ctrl.ocm_buf_list, node) {
			if (!memcmp(&buf_list_pos->buf_info, &buf_info, sizeof(buf_info))) {
				find_ocm_fd = true;
				list_del(&buf_list_pos->node);
				kfree(buf_list_pos);
				break;
			}
		}

		if (!find_ocm_fd) {
			mutex_unlock(&g_cam_ctrl.ocm_list_mutex);
			XRISP_PR_ERROR("fail to find ocm fd, fd:%d, hwaddr:0x%llx, size:%d",
				       buf_info.fd, buf_info.hwaddr, buf_info.size);
			return -EINVAL;
		}

		xrisp_rproc_ocm_unmap(buf_info.hwaddr, buf_info.size);
		(void)ocm_buf_put(buf_info.fd);
		mutex_unlock(&g_cam_ctrl.ocm_list_mutex);

		xrisp_be_ocm_unlink();

		XRISP_PR_INFO("Put ocm fd:%d, hwaddr:0x%llx, size:%d",
			buf_info.fd, buf_info.hwaddr, buf_info.size);

	} break;

	case CAMERA_IOC_CMD_OCM_LINK: {
		ret = xrisp_be_ocm_link();
		if (ret)
			XRISP_PR_ERROR("ocm link fail");
		XRISP_PR_INFO("ocm link should skip");
	} break;

	case CAMERA_IOC_CMD_OCM_UNLINK: {
		ret = xrisp_be_ocm_unlink();
		if (ret)
			XRISP_PR_ERROR("ocm unlink fail");
		XRISP_PR_INFO("ocm unlink should skip");
	} break;

	case CAMERA_IOC_CMD_MSG_BUFFER_IDX_RELEASE: {
		struct xrisp_buf_idx_release buf_id;

		ret = xrisp_copy_cmdop(cmd, &buf_id,
				       sizeof(struct xrisp_buf_idx_release));
		if (ret)
			return ret;

		if (g_cam_ctrl.is_msgbuf_register == false) {
			XRISP_PR_ERROR("msg_buf not been registered, exit");
			return -EINVAL;
		}

		if (buf_id.idx >= g_cam_ctrl.msg_buf_num) {
			XRISP_PR_ERROR("invalid idx=%d", buf_id.idx);
			return -EINVAL;
		}

		spin_lock(&g_cam_ctrl.event_lock);
		if (test_bit(buf_id.idx, g_cam_ctrl.p_msg_bitmap->bitmap))
			clear_bit(buf_id.idx, g_cam_ctrl.p_msg_bitmap->bitmap);
		spin_unlock(&g_cam_ctrl.event_lock);
	} break;

	default: {
		XRISP_PR_ERROR("invalid Opcode: %d", cmd->op_code);
		return -EINVAL;
	} break;
	}

cam_ioctl_out:
	ioctl_use = ktime_ms_delta(ktime_get(), ioctl_start);
	if (ioctl_use > ISP_IOCTL_TIMEMS_MAX)
		XRISP_PR_INFO("cam_ctrl ioctl %d use long time: %lldms", cmd->op_code, ioctl_use);
	return ret;
}

int xrisp_cpc_ack(struct cpc_msg_data *msg, u8 is_success)
{
	int ret;
	uint32_t size = sizeof(struct cpc_msg_data);
	struct cpc_msg_data *ackmsg = kzalloc(size + 1, GFP_ATOMIC);

	if (!ackmsg)
		return -ENOMEM;

	ackmsg->type = msg->type;
	ackmsg->msg_id = msg->msg_id;
	ackmsg->size = 1;
	*ackmsg->data = is_success;
	ret = xrisp_msg_send(ackmsg, size + 1, TOPIC_DRV);
	kfree(ackmsg);
	return ret;
}

int xrisp_ipc_cpc_handler(void *data, unsigned int len)
{
	struct cpc_msg_data *msg = data;
	int ret = 0;

	if (IS_ERR_OR_NULL(msg) || len < sizeof(*msg)) {
		XRISP_PR_ERROR("invalid cpc, exit");
		return -EINVAL;
	}

	XRISP_PR_DEBUG("recv cpc msg id=%llu, type=%d, size=%d", msg->msg_id, msg->type, msg->size);
	switch (msg->type) {
	case DRV_DUMMY: {
		XRISP_PR_DEBUG("into dummy process");
	} break;

	case DRV_ISP_PIPE_POWERON: {
		struct xrisp_power_up *pon_set;

		if (sizeof(*pon_set) != msg->size) {
			XRISP_PR_ERROR("data size not match, get=%d, exp=%lu", msg->size, sizeof(*pon_set));
			return -EINVAL;
		}
		pon_set = (struct xrisp_power_up *)msg->data;
		ret = xrisp_pipe_rgltr_enable(pon_set->domain, 5);
		if (ret)
			xrisp_cpc_ack(msg, false);
		else
			xrisp_cpc_ack(msg, true);
	} break;

	case DRV_ISP_PIPE_POWEROFF: {
		struct xrisp_power_down *poff_set;

		if (sizeof(*poff_set) != msg->size) {
			XRISP_PR_ERROR("data size not match, get=%d, exp=%lu", msg->size,
				       sizeof(*poff_set));
			return -EINVAL;
		}

		poff_set = (struct xrisp_power_down *)msg->data;
		ret = xrisp_pipe_rgltr_disable(poff_set->domain, 5);
		if (ret)
			xrisp_cpc_ack(msg, false);
		else
			xrisp_cpc_ack(msg, true);
	} break;

	case DRV_ISP_DVFS: {
		struct xrisp_dvfs *dvfs;

		if (xrisp_rproc_is_running() != true) {
			XRISP_PR_ERROR("cpc set dvfs failed, need to isp running");
			return -ENODEV;
		}

		if (sizeof(*dvfs) != msg->size) {
			XRISP_PR_ERROR("data size not match, get=%d, exp=%lu", msg->size, sizeof(*dvfs));
			return -EINVAL;
		}

		dvfs = (struct xrisp_dvfs *)msg->data;
		if (dvfs->dvfs_set < XRISP_CLK_RATE_MAX) {
			ret = xrisp_clk_api_set_rate_by_mask(
				XRISP_CRG_CLK, (1 << CLK_ISP_CRG_OUT_MAX) - 1, dvfs->dvfs_set);
			if (ret) {
				XRISP_PR_ERROR("cpc set dvfs failed, ret = %d, dvfs = %d", ret,
					       dvfs->dvfs_set);
				return -EINVAL;
			}
			XRISP_PR_INFO("cpc set dvfs success, dvfs = %d", dvfs->dvfs_set);

		} else {
			XRISP_PR_ERROR("input dvfs invalid, dvfs = %d", dvfs->dvfs_set);
			ret = -EINVAL;
		}
	} break;

	case DRV_ISP_SERVICE_ONLINE: {
		complete(&g_cam_ctrl.isp_service_online);
	} break;

	case DRV_ISP_PARKLENS_COMPLETE: {
		uint32_t sensor_id = *(uint32_t *)(msg->data);

		xrisp_parklens_complete(sensor_id);
	} break;

	default:
		XRISP_PR_ERROR("unknown cpc cmd, type=%d", msg->type);
		ret = -EINVAL;
	}
	return ret;
}

void xrisp_ipc_msgbuf_reset(void)
{
	spin_lock(&g_cam_ctrl.event_lock);
	g_cam_ctrl.p_msg_bitmap = NULL;
	g_cam_ctrl.p_msg_buf = NULL;
	g_cam_ctrl.buf_handle = 0;
	g_cam_ctrl.msg_buf_num = 0;
	g_cam_ctrl.is_msgbuf_register = false;
	spin_unlock(&g_cam_ctrl.event_lock);
	XRISP_PR_INFO("xrisp cam_ctrl share msgbuf reset finish");
}

void xrisp_ocmlist_init(struct xrisp_cam_ctrl *pri)
{
	if (!pri)
		return;

	mutex_init(&pri->ocm_list_mutex);
	INIT_LIST_HEAD(&pri->ocm_buf_list);
}

void xrisp_ocmlist_deinit(struct xrisp_cam_ctrl *pri)
{
	struct ocm_buf_list *buf_list_pos, *buf_list_t;

	if (!pri)
		return;

	mutex_lock(&pri->ocm_list_mutex);
	list_for_each_entry_safe(buf_list_pos, buf_list_t,
				 &pri->ocm_buf_list, node) {
		(void)ocm_buf_put(buf_list_pos->buf_info.fd);
		list_del(&buf_list_pos->node);
		kfree(buf_list_pos);
	}
	mutex_unlock(&pri->ocm_list_mutex);
}

void xrisp_ocmlist_release(void)
{
	xrisp_ocmlist_deinit(&g_cam_ctrl);
}

int xrisp_cam_ctrl_init(struct video_device *vdev)
{
	if (vdev)
		g_cam_ctrl.vdev = vdev;
	else {
		XRISP_PR_ERROR("cam_ctrl get cbm vdev failed");
		return -EINVAL;
	}
	mutex_init(&g_cam_ctrl.ctrl_lock);
	spin_lock_init(&g_cam_ctrl.event_lock);
	init_completion(&g_cam_ctrl.isp_service_online);

	xrisp_ipc_msgbuf_reset();

	if (xrisp_ctrl_mbox_register(&g_cam_ctrl.mbox_reg_done)) {
		XRISP_PR_ERROR("ipc mbox register fail, cam_ctrl init failed");
		return -EINVAL;
	}

	if (xrisp_ois_mbox_register(&g_cam_ctrl.ois_mbox_reg_done)) {
		XRISP_PR_ERROR("ois mbox register fail, cam_ctrl init failed");
		return -EINVAL;
	}
	if (xrisp_camctrl_debugfs_init()) {
		XRISP_PR_ERROR("debugfs init error, cam_ctrl init failed");
		return -EINVAL;
	}

	rpc_service_init();

	xrisp_ocmlist_init(&g_cam_ctrl);

	XRISP_PR_INFO("xrisp cam_ctrl init finish");
	return 0;
}

void xrisp_cam_ctrl_release(void)
{
	xrisp_ocmlist_deinit(&g_cam_ctrl);
	rpc_service_uninit();
	xrisp_camctrl_debugfs_exit();
	xrisp_ois_mbox_unregister(&g_cam_ctrl.ois_mbox_reg_done);
	xrisp_ctrl_mbox_unregister(&g_cam_ctrl.mbox_reg_done);
	mutex_destroy(&g_cam_ctrl.ctrl_lock);
	XRISP_PR_INFO("xrisp cam_ctrl release finish");
}
