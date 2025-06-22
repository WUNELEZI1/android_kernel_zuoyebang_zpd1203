// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <tee_drv.h>
#include <linux/uuid.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <drm/display/drm_dp_helper.h>

#include "dp_hdcp.h"
#include "dp_display.h"

#define DPCD_HDCP13_BCAPS                             0x68028
#define DPCD_HDCP13_CAPABLE_MASK                      0x1
#define DPCD_HDCP13_REPEATER_MASK                     0x2
#define DPCD_HDCP13_BSTATUS                           0x68029
#define DPCD_HDCP22_RXCAPS                            0x6921D
#define DPCD_HDCP22_REPEATER_MASK                     0x1
#define DPCD_HDCP22_HDCP_CAPABLE_MASK                 0x2
#define DPCD_HDCP22_RX_STATUS                         0x69493
#define DPCD_HDCP22_H_AVALIABLE                       0x2
#define DPCD_HDCP22_PAIRING_AVALIABLE                 0x4
#define DPCD_VERSION_OFFSET                           0
#define DPCD_VERSION_MAJOR_SHIFT                      4
#define DPCD_HDCP22_H_PRIME                           0x692c0
#define DPCD_HDCP22_KM                                0x692e0
#define DPCD_IRQ_VECTOR_1                             0x201
#define DPCD_IRQ_VECTOR_2                             0x2003

#define HDCP_MONITOR_TIMEOUT                          (5 * HZ)
#define HDCP_REAUTH_COUNT                             3
#define HDCP_FW_SIZE                                  (256 * 1024)

static int dp_hdcp_1p3_caps_check(struct dp_display *display,
		bool *is_supported, bool *has_repeater, bool *is_dpcd_12p)
{
	struct drm_dp_aux *aux = &display->aux->base;
	u8 caps;
	int ret;

	ret = drm_dp_dpcd_readb(aux, DPCD_HDCP13_BCAPS, &caps);
	if (ret <= 0) {
		DP_ERROR("failed to get hdcp 1.3 bcaps\n");
		return ret;
	}

	if (caps & DPCD_HDCP13_CAPABLE_MASK)
		*is_supported = true;
	else
		*is_supported = false;

	if (caps & DPCD_HDCP13_REPEATER_MASK)
		*has_repeater = true;
	else
		*has_repeater = false;

	ret = drm_dp_dpcd_readb(aux, DPCD_VERSION_OFFSET, &caps);
	if (ret <= 0) {
		DP_ERROR("failed to get version\n");
		return ret;
	}

	*is_dpcd_12p = (caps >= 0x12);

	return 0;
}

static int dp_hdcp_2p2_caps_check(struct dp_display *display,
		bool *is_supported, bool *has_repeater, bool *is_dpcd_12p)
{
	struct drm_dp_aux *aux = &display->aux->base;
	u8 caps[3];
	int ret;

	ret = drm_dp_dpcd_read(aux, DPCD_HDCP22_RXCAPS, caps, 3);
	if (ret != 3) {
		DP_ERROR("failed to get hdcp 2.2 bcaps, %d\n", ret);
		return -EINVAL;
	}

	if (caps[2] != 0x2) {
		DP_WARN("don't have hdcp 2.2 caps, value: %x\n", caps[2]);
		return -EINVAL;
	}

	if (caps[0] & DPCD_HDCP22_HDCP_CAPABLE_MASK)
		*is_supported = true;
	else
		*is_supported = false;

	if (caps[0] & DPCD_HDCP22_REPEATER_MASK)
		*has_repeater = true;
	else
		*has_repeater = false;

	ret = drm_dp_dpcd_readb(aux, DPCD_VERSION_OFFSET, &caps[0]);
	if (ret <= 0) {
		DP_ERROR("failed to get version\n");
		return ret;
	}

	*is_dpcd_12p = (caps[0] >= 0x12);

	return 0;
}

static int dp_hdcp_ca_exec(struct dp_hdcp *hdcp, enum hdcp_ta_cmd_id cmd)
{
	struct tee_ioctl_invoke_arg invoke_arg = {0};
	struct tee_param param[4] = {0};
	int ret = 0;

	invoke_arg.func = cmd;
	invoke_arg.session = hdcp->tee_priv->session;
	invoke_arg.num_params = 4;

	ret = tee_client_invoke_func(hdcp->tee_priv->ctx, &invoke_arg, param);
	if ((ret < 0) || ((invoke_arg.ret != 0))) {
		DP_ERROR("hdcp ca exec error: %x, %x\n", ret, invoke_arg.ret);
		return -EINVAL;
	}

	return ret;
}

static int dp_hdcp_ca_write(struct dp_hdcp *hdcp, enum hdcp_ta_cmd_id cmd,
		u64 value[4], u8 arg_count)
{
	struct tee_ioctl_invoke_arg invoke_arg = {0};
	struct tee_param param[4] = {0};
	int ret = 0;
	int i;

	invoke_arg.func = cmd;
	invoke_arg.session = hdcp->tee_priv->session;
	invoke_arg.num_params = 4;

	for (i = 0; i < arg_count; i++) {
		param[i].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[i].u.value.a = value[i];
	}

	ret = tee_client_invoke_func(hdcp->tee_priv->ctx, &invoke_arg, param);
	if ((ret < 0) || ((invoke_arg.ret != 0))) {
		DP_ERROR("hdcp ca write error: %x, %x\n", ret, invoke_arg.ret);
		return -EINVAL;
	}

	return ret;
}

static int dp_hdcp_ca_read(struct dp_hdcp *hdcp, enum hdcp_ta_cmd_id cmd,
		u64 *value, u8 arg_count)
{
	struct tee_ioctl_invoke_arg invoke_arg = {0};
	struct tee_param param[4] = {0};
	int ret = 0;
	int i;

	invoke_arg.func = cmd;
	invoke_arg.session = hdcp->tee_priv->session;
	invoke_arg.num_params = 4;

	for (i = 0; i < arg_count; i++) {
		param[i].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
		param[i].u.value.a = value[i];
	}

	ret = tee_client_invoke_func(hdcp->tee_priv->ctx, &invoke_arg, param);
	if ((ret < 0) || ((invoke_arg.ret != 0))) {
		DP_ERROR("hdcp ca read error: %x, %x\n", ret, invoke_arg.ret);
		return -EINVAL;
	}

	for (i = 0; i < arg_count; i++)
		value[i] = param[i].u.value.a;

	return ret;
}

static int dp_hdcp_check_auth_state(struct dp_hdcp *hdcp)
{
	u64 value;
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_read(hdcp, TA_CMD_HDCP_CHECK_AUTH_STATE, &value, 1);
	if (ret) {
		DP_ERROR("failed to check authentication state, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_dump_authenticate_count(struct dp_hdcp *hdcp, u64 *value)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_read(hdcp, TA_CMD_HDCP_AUTHENTICATION_COUNT_DUMP, value, 2);
	if (ret) {
		DP_ERROR("failed to check authentication state, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_authenticate(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_AUTHENTICATE);
	if (ret) {
		DP_ERROR("failed to do authentication and try to do reauthenticate, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_reauthenticate(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_ON);
	if (ret) {
		DP_ERROR("failed to do hdcp on, ret %d\n", ret);
		return ret;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_REAUTHENTICATE);
	if (ret) {
		DP_ERROR("failed to do re-authentication, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_set_cp_irq(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_SET_CP_IRQ);
	if (ret) {
		DP_ERROR("failed to set cp irq, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_is_connected(struct dp_hdcp *hdcp, bool enable)
{
	u64 value[4] = {0};
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	value[0] = enable;

	ret = dp_hdcp_ca_write(hdcp, TA_CMD_HDCP_IS_DP_CONNECTED, value, 1);
	if (ret) {
		DP_ERROR("failed to enable encryption, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_enable_encryption(struct dp_hdcp *hdcp, bool enable)
{
	u64 value[4] = {0};
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	value[0] = enable;

	ret = dp_hdcp_ca_write(hdcp, TA_CMD_HDCP_ENABLE_ENCRYPTION, value, 1);
	if (ret) {
		DP_ERROR("failed to enable encryption, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_set_forced_version(struct dp_hdcp *hdcp,
		enum hdcp_version ver, bool has_repeater, bool is_dpcd_12p)
{
	u64 value[4] = {0};
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	value[0] = ver;
	value[1] = has_repeater;
	value[2] = is_dpcd_12p;

	ret = dp_hdcp_ca_write(hdcp, TA_CMD_HDCP_SET_FORCED_VER, value, 3);
	if (ret) {
		DP_ERROR("failed to set forced version, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_on(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_ON);
	if (ret) {
		DP_ERROR("failed to set hdcp on, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_off(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	ret = dp_hdcp_ca_exec(hdcp, TA_CMD_HDCP_OFF);
	if (ret) {
		DP_ERROR("failed to set hdcp off, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static int dp_hdcp_set_mode(struct dp_hdcp *hdcp, u8 mode)
{
	u64 value[4] = {0};
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	value[0] = mode;

	ret = dp_hdcp_ca_write(hdcp, TA_CMD_HDCP_SET_MODE, value, 1);
	if (ret) {
		DP_ERROR("failed to set mode, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static struct hdcp_ops hdcp_ops = {
	.check_auth_state           = dp_hdcp_check_auth_state,
	.authenticate               = dp_hdcp_authenticate,
	.reauthenticate             = dp_hdcp_reauthenticate,
	.set_cp_irq                 = dp_hdcp_set_cp_irq,
	.enable_encryption          = dp_hdcp_enable_encryption,
	.set_forced_version         = dp_hdcp_set_forced_version,
	.on                         = dp_hdcp_on,
	.off                        = dp_hdcp_off,
	.set_mode                   = dp_hdcp_set_mode,
	.dump_authenticate_count    = dp_hdcp_dump_authenticate_count,
	.is_dp_connected            = dp_hdcp_is_connected,
};

#define HDCP_TA_UUID UUID_INIT(0xa72aaa20, 0x375d, 0x2729, 0x10, 0xf1, 0xd2, \
	0xf3, 0xc9, 0xb4, 0x93, 0x15)

static int dp_hdcp_tee_client_open(struct hdcp_tee_private *tee_priv);
static void dp_hdcp_tee_client_close(struct hdcp_tee_private *tee_priv);

static void dp_hdcp_cp_state_reset(struct dp_hdcp *hdcp)
{
	struct drm_dp_aux *aux = &hdcp->display->aux->base;
	enum hdcp_version version;
	u8 state;

	version = hdcp->state.curr_version;

	if (version == HDCP_VERSION_22) {
		drm_dp_dpcd_readb(aux, DPCD_HDCP22_H_PRIME, &state);
		drm_dp_dpcd_readb(aux, DPCD_HDCP22_KM, &state);
	}

	drm_dp_dpcd_writeb(aux, DPCD_IRQ_VECTOR_1, BIT(2));
	drm_dp_dpcd_writeb(aux, DPCD_IRQ_VECTOR_2, BIT(2));
}

static void dp_hdcp_monitor(struct kthread_work *work)
{
	struct dp_hdcp *hdcp = container_of(work, struct dp_hdcp, monitor_work);
	enum hdcp_version hdcp_ver;
	bool is_supported;
	bool has_repeater;
	bool is_dpcd_12p;
	int retry_count;
	int ret;

	DP_DEBUG("Enter\n");

	hdcp_ver = hdcp->state.forced_hdcp_ver;
	hdcp->state.run_info = HDCP_CHECK_CAPS;

repeat:
	retry_count = HDCP_REAUTH_COUNT;

	switch (hdcp_ver) {
	case HDCP_VERSION_NONE:
		hdcp_ver = HDCP_VERSION_22;
		goto repeat;
	case HDCP_VERSION_13:
		ret = dp_hdcp_1p3_caps_check(hdcp->display, &is_supported,
				&has_repeater, &is_dpcd_12p);
		if (ret) {
			DP_ERROR("cannot get hdcp 1p3 caps, %d\n", ret);
			goto err;
		}
		break;
	case HDCP_VERSION_22:
		ret = dp_hdcp_2p2_caps_check(hdcp->display, &is_supported,
				&has_repeater, &is_dpcd_12p);
		if ((ret || !is_supported) && !hdcp->state.forced_hdcp_ver) {
			hdcp_ver = HDCP_VERSION_13;
			goto repeat;
		} else if (ret) {
			DP_ERROR("cannot get hdcp 2p2 caps, %d\n", ret);
			goto err;
		}
		break;
	default:
		DP_ERROR("unsupported hdcp version %d\n", hdcp_ver);
		goto err;
	}

	if (is_supported) {
		hdcp->state.curr_version = hdcp_ver;
		hdcp->ops->set_forced_version(hdcp, hdcp_ver,
				has_repeater, is_dpcd_12p);
		DP_INFO("hdcp version: %d, repeater support: %d\n",
				hdcp->state.curr_version, has_repeater);
	} else {
		DP_ERROR("sink does not support hdcp %d\n", hdcp_ver);
		goto err;
	}

	hdcp->state.run_info = HDCP_ON;
	ret = hdcp->ops->on(hdcp);
	if (ret) {
		DP_ERROR("failed to do hdcp on, %d\n", ret);
		goto on_err;
	}

	hdcp->state.is_encryption_enabled = false;
	hdcp->display->status.is_hdcp_encryption_enabled = false;
	hdcp->state.run_info = HDCP_AUTHENTICATING;

	while (1) {
		if (hdcp->state.run_info == HDCP_AUTHENTICATING) {
			dp_hdcp_cp_state_reset(hdcp);
			ret = hdcp->ops->authenticate(hdcp);
			if (ret) {
				hdcp->state.run_info = HDCP_AUTHENTICATE_FAIL;
				hdcp->ops->off(hdcp);
				continue;
			} else {
				hdcp->state.run_info = HDCP_AUTHENTICATED;
				hdcp->display->status.hdcp_availabled = true;
				DP_INFO("HDCP %d authenticate successfully\n", hdcp->state.curr_version);
			}
		} else if (hdcp->state.run_info == HDCP_AUTHENTICATE_FAIL) {
			retry_count--;
			if (retry_count <= 0) {
				if (hdcp_ver == HDCP_VERSION_22) {
					DP_ERROR("hdcp2.2 authernticate failed, try to hdcp1.3 task\n");
					hdcp_ver = HDCP_VERSION_13;
					goto repeat;
				}
				DP_ERROR("failed to do authentication\n");
				break;
			}

			ret = hdcp->ops->reauthenticate(hdcp);
			if (ret) {
				DP_DEBUG("HDCP %d reauthenticate failed, retry %d", hdcp->state.curr_version,
					HDCP_REAUTH_COUNT - retry_count);
				hdcp->ops->off(hdcp);
				continue;
			} else {
				hdcp->state.run_info = HDCP_AUTHENTICATED;
				hdcp->display->status.hdcp_availabled = true;
				DP_INFO("HDCP %d authenticate successfully\n", hdcp->state.curr_version);
			}
		}

		if (kthread_should_stop())
			break;

		ret = hdcp->ops->check_auth_state(hdcp);
		if (ret)
			hdcp->state.run_info = HDCP_AUTHENTICATE_FAIL;
		else
			schedule_timeout_uninterruptible(HDCP_MONITOR_TIMEOUT);
	}

on_err:
	ret = hdcp->ops->off(hdcp);
	hdcp->state.run_info = HDCP_OFF;
	hdcp->state.is_encryption_enabled = false;
	hdcp->display->status.is_hdcp_encryption_enabled = false;

err:
	hdcp->display->status.hdcp_availabled = false;

	DP_DEBUG("Exit\n");
}

int dp_hdcp_monitor_start(struct dp_hdcp *hdcp)
{
	int ret;

	if (!hdcp) {
		DP_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	mutex_lock(&hdcp->lock);
	if (hdcp->state.task_started) {
		mutex_unlock(&hdcp->lock);
		return 0;
	}

	ret = dp_hdcp_tee_client_open(hdcp->tee_priv);
	if (ret) {
		DP_ERROR("failed to open tee client, ret %d\n", ret);
		goto err_tee;
	}

	hdcp->state.task_started = true;
	/* send dp connected state for widewine get hdcp level */
	hdcp->ops->is_dp_connected(hdcp, true);

#ifdef F1_HDCP_SUPPORT
	hdcp->monitor_task = kthread_run(kthread_worker_fn,
			&hdcp->monitor_worker, "hdcp_monitor");
	if (IS_ERR(hdcp->monitor_task)) {
		DP_ERROR("failed to run monitor thread\n");
		ret = PTR_ERR(hdcp->monitor_task);
		goto err_task;
	}

	sched_set_fifo(hdcp->monitor_task);

	kthread_queue_work(&hdcp->monitor_worker, &hdcp->monitor_work);
#endif
	mutex_unlock(&hdcp->lock);

	return 0;

#ifdef F1_HDCP_SUPPORT
err_task:
	dp_hdcp_tee_client_close(hdcp->tee_priv);
#endif
err_tee:
	hdcp->state.task_started = false;
	mutex_unlock(&hdcp->lock);
	return ret;
}

void dp_hdcp_monitor_stop(struct dp_hdcp *hdcp)
{
	mutex_lock(&hdcp->lock);

	if (hdcp->state.task_started) {
		/* send dp connected state for widewine get hdcp level */
		hdcp->ops->is_dp_connected(hdcp, false);

#ifdef F1_HDCP_SUPPORT
		kthread_stop(hdcp->monitor_task);
#endif
		dp_hdcp_tee_client_close(hdcp->tee_priv);
		hdcp->state.task_started = false;
	}

	mutex_unlock(&hdcp->lock);
}

int dp_hdcp_encryption_enable(struct dp_hdcp *hdcp, bool enable)
{
	int ret = 0;

	mutex_lock(&hdcp->lock);
	if ((hdcp->state.is_encryption_enabled != enable) &&
			hdcp->state.task_started) {
		ret = hdcp->ops->enable_encryption(hdcp, enable);
		if (!ret) {
			hdcp->state.is_encryption_enabled = enable;
			hdcp->display->status.is_hdcp_encryption_enabled = enable;
		}
	}
	mutex_unlock(&hdcp->lock);

	return ret;
}

static int mitee_ctx_match(struct tee_ioctl_version_data *ver,
		const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int dp_hdcp_tee_client_open(struct hdcp_tee_private *tee_priv)
{
	struct tee_ioctl_open_session_arg sess_arg = {0};
	int ret = 0;

	if (!tee_priv) {
		DP_ERROR("invalid tee client device\n");
		return -EAGAIN;
	}

	tee_priv->ctx = tee_client_open_context(NULL,
			mitee_ctx_match, NULL, NULL);
	if (IS_ERR(tee_priv->ctx)) {
		DP_ERROR("tee_client_open_context error\n");
		return -ENODEV;
	}

	export_uuid(sess_arg.uuid, &HDCP_TA_UUID);
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(tee_priv->ctx, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		DP_ERROR("tee_client_open_session error: %x, %x\n",
				ret, sess_arg.ret);
		tee_client_close_context(tee_priv->ctx);
		return -EINVAL;
	}

	tee_priv->session = sess_arg.session;

	return 0;
}

static void dp_hdcp_tee_client_close(struct hdcp_tee_private *tee_priv)
{
	tee_client_close_session(tee_priv->ctx, tee_priv->session);
	tee_client_close_context(tee_priv->ctx);

	tee_priv->session = 0;
	tee_priv->ctx = NULL;
}

static int dp_hdcp_tee_init(struct dp_hdcp *hdcp)
{
	struct hdcp_tee_private *tee_priv;

	tee_priv = kzalloc(sizeof(*tee_priv), GFP_KERNEL);
	if (!tee_priv)
		return -ENOMEM;

	hdcp->tee_priv = tee_priv;

	return 0;
}

static void dp_hdcp_tee_deinit(struct dp_hdcp *hdcp)
{
	dp_hdcp_monitor_stop(hdcp);
	kfree(hdcp->tee_priv);
	hdcp->tee_priv = NULL;
}

int dp_hdcp_init(struct dp_display *display, struct dp_hdcp **hdcp)
{
	struct dp_hdcp *dp_hdcp;
	int ret;

	dp_hdcp = kzalloc(sizeof(*dp_hdcp), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(dp_hdcp))
		return -ENOMEM;

	mutex_init(&dp_hdcp->lock);

	ret = dp_hdcp_tee_init(dp_hdcp);
	if (ret) {
		DP_ERROR("failed to init hdcp tee\n");
		goto tee_init_err;
	}

	kthread_init_worker(&dp_hdcp->monitor_worker);
	kthread_init_work(&dp_hdcp->monitor_work, &dp_hdcp_monitor);

	dp_hdcp->ops = &hdcp_ops;
	dp_hdcp->display = display;
	*hdcp = dp_hdcp;

	return 0;

tee_init_err:
	kfree(dp_hdcp);
	return ret;
}

void dp_hdcp_deinit(struct dp_hdcp *hdcp)
{
	dp_hdcp_tee_deinit(hdcp);
	kfree(hdcp);
}
#endif
