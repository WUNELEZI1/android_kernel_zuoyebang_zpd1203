/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DP_HDCP_H_
#define _DP_HDCP_H_

#include <linux/version.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include "dp_display.h"

struct dp_hdcp;

enum hdcp_version {
	HDCP_VERSION_NONE = 0,
	HDCP_VERSION_13 = 13,
	HDCP_VERSION_22 = 22,
	HDCP_VERSION_MAX
};

/* the TA command id for invoking to tee */
enum hdcp_ta_cmd_id {
	TA_CMD_HDCP_CHECK_AUTH_STATE,
	TA_CMD_HDCP_SET_CP_IRQ,
	TA_CMD_HDCP_AUTHENTICATE,
	TA_CMD_HDCP_REAUTHENTICATE,
	TA_CMD_HDCP_ENABLE_ENCRYPTION,
	TA_CMD_HDCP_SET_FORCED_VER,
	TA_CMD_HDCP_ON,
	TA_CMD_HDCP_OFF,
	TA_CMD_HDCP_FW_UPDATE,
	TA_CMD_HDCP_FW_RELEASE,
	TA_CMD_HDCP_SET_MODE,
	TA_CMD_HDCP_AUTHENTICATION_COUNT_DUMP,
	TA_CMD_HDCP_IS_DP_CONNECTED = 12,
};

/**
 * struct hdcp_tee_private - TEE private data
 * @ctx: TEE context handler
 * @session: hdcp TA session identifier
 * @shm_fw: the shared memory for updating firmware
 */
struct hdcp_tee_private {
	struct tee_context *ctx;
	u32 session;

	struct tee_shm *shm_fw;
};

struct hdcp_ops {
	/* check hdcp authentication state */
	int (*check_auth_state)(struct dp_hdcp *hdcp);
	/* notify ta the cp irq is coming */
	int (*set_cp_irq)(struct dp_hdcp *hdcp);
	/* start authentication */
	int (*authenticate)(struct dp_hdcp *hdcp);
	/* do re-authentication when encounter abnormal behavior */
	int (*reauthenticate)(struct dp_hdcp *hdcp);
	/* enable encryption for esm */
	int (*enable_encryption)(struct dp_hdcp *hdcp, bool enable);
	/* set the version which used in authentication stage */
	int (*set_forced_version)(struct dp_hdcp *hdcp, enum hdcp_version ver,
			bool has_repeater, bool is_dpcd_12p);
	/* hdcp power on */
	int (*on)(struct dp_hdcp *hdcp);
	/* hdcp power off */
	int (*off)(struct dp_hdcp *hdcp);
	/* set hdcp encryption mode */
	int (*set_mode)(struct dp_hdcp *hdcp, u8 mode);
	/* dump authentication result */
	int (*dump_authenticate_count)(struct dp_hdcp *hdcp, u64 *value);
	int (*is_dp_connected)(struct dp_hdcp *hdcp, bool enable);
};

/* internal run info state machine */
enum hdcp_run_info {
	HDCP_READY = 0,
	HDCP_OFF,
	HDCP_CHECK_CAPS,
	HDCP_ON,
	HDCP_AUTHENTICATING,
	HDCP_AUTHENTICATE_FAIL,
	HDCP_AUTHENTICATED,
};

/* the internal hdcp state */
struct dp_hdcp_state {
	/* control or state flag */
	bool task_started;
	bool is_encryption_enabled;
	enum hdcp_version forced_hdcp_ver;

	/* internal state machine */
	enum hdcp_version curr_version;
	enum hdcp_run_info run_info;
};

/* dp_hdcp - the hdcp module structure */
struct dp_hdcp {
	struct dp_display *display;

	/* tee client data */
	struct hdcp_tee_private *tee_priv;

	/* monitor task data */
	struct task_struct *monitor_task;
	struct kthread_worker monitor_worker;
	struct kthread_work monitor_work;

	/* the internal hdcp state */
	struct dp_hdcp_state state;

	struct mutex lock;

	struct hdcp_ops *ops;
};

int dp_hdcp_monitor_start(struct dp_hdcp *hdcp);
void dp_hdcp_monitor_stop(struct dp_hdcp *hdcp);
int dp_hdcp_encryption_enable(struct dp_hdcp *hdcp, bool enable);
int dp_hdcp_init(struct dp_display *display, struct dp_hdcp **hdcp);
void dp_hdcp_deinit(struct dp_hdcp *hdcp);

#endif /* _DP_HDCP_H_ */
