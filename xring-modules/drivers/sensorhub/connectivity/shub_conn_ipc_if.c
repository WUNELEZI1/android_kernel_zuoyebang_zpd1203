// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <dt-bindings/xring/platform-specific/ipc_protocol_sh.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/sensorhub/shub_conn_ipc_if.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <soc/xring/sensorhub/shub_notifier.h>

#define AP_SHUB_CONN_IPC_VCID (IPC_VC_AO_NS_SH_AP_DATA_TRANS)
#define AP_SHUB_CONN_IPC_TAG (TAG_SH_CONNECTIVITY)
#define CONN_MSG_TO_CMD(module, state) ((u8)((((u8)module) << 4) | (((u8)state) & 0xF)))
#define CONN_CMD_TO_MSG(cmd, module, state) do {	\
	*module = (((u8)cmd) >> 4);						\
	*state = (((u8)cmd) & 0xF);						\
} while (0)

static struct conn_to_shub_msg g_conn_state_backup[MODULE_NUM_MAX] = {};
static shub_conn_ipc_cb g_conn_ipc_cb[MODULE_NUM_MAX] = {};

static void shub_conn_ipc_notify_on(struct conn_to_shub_msg *to_shub_msg)
{
	int ret = 0;
	u8 cmd = 0;
	int retry_count = 20;

	if (to_shub_msg->state != DRIVER_STATE_ON)
		return;

	pr_info("shub_conn recover notify [%u] on.\n", to_shub_msg->module);
	cmd = CONN_MSG_TO_CMD(to_shub_msg->module, DRIVER_STATE_ON);
	do {
		ret = sh_ipc_send_msg_sync(AP_SHUB_CONN_IPC_VCID, AP_SHUB_CONN_IPC_TAG, cmd, NULL, 0);
		if (ret == -EBUSY || ret == -EAGAIN) {
			pr_err("shub_conn cmd[%u] ipc send failed, ret:%d, retry:%d\n", cmd, ret, retry_count);
			msleep(1);
		} else if (ret < 0) {
			pr_err("shub_conn cmd[%u] ipc send failed, ret:%d\n", cmd, ret);
			return;
		} else {
			return;
		}
		retry_count--;
	} while (retry_count > 0);
}

static int shub_conn_event_cb(struct notifier_block *nb, unsigned long action, void *data)
{
	if (action == ACTION_SH_RECOVERY_ALL_READY) {
		for (int i = MODULE_BT; i <= MODULE_MODEM; i++)
			shub_conn_ipc_notify_on(&g_conn_state_backup[i]);
		pr_info("%s done.\n", __func__);
	}
	return NOTIFY_DONE;
}

static struct notifier_block shub_conn_notifier = {
	.notifier_call = shub_conn_event_cb,
};

static int _conn_msg_from_shub_cb(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	if (tag != AP_SHUB_CONN_IPC_TAG) {
		pr_err("not shub conn ipc tag : 0x%x\n", tag);
		return 0;
	}

	struct shub_conn_to_ap_msg shub_request_msg;

	CONN_CMD_TO_MSG(cmd, &(shub_request_msg.module), &(shub_request_msg.request_cmd));

	if ((shub_request_msg.module >= MODULE_NUM_MAX) ||
			(shub_request_msg.request_cmd >= SHUB_REQUEST_CMD_MAX)) {
		pr_err("CONN cmd not match, module = 0x%x, request_cmd = 0x%x, ipc_cmd = 0x%x\n",
					shub_request_msg.module, shub_request_msg.request_cmd, cmd);
		return 0;
	}

	if (g_conn_ipc_cb[shub_request_msg.module] != NULL)
		g_conn_ipc_cb[shub_request_msg.module](&shub_request_msg);
	else
		pr_err("cb not registed\n");

	return 0;
}

static void _regist_shub_ipc_msg_cb(void)
{
	int ret;
	struct ipc_notifier_info notify_info;

	notify_info.callback = _conn_msg_from_shub_cb;
	notify_info.arg = NULL;
	ret = sh_register_recv_notifier(AP_SHUB_CONN_IPC_VCID, AP_SHUB_CONN_IPC_TAG, &notify_info);
	if (ret < 0)
		pr_err("conn ipc cb failed, ret:%d\n", ret);
}

void shub_conn_notify_state(struct conn_to_shub_msg *to_shub_msg)
{
	if (to_shub_msg == NULL) {
		pr_err("to_shub_msg is NULL\n");
		return;
	}
	int ret = 0;

	g_conn_state_backup[to_shub_msg->module].module = to_shub_msg->module;
	g_conn_state_backup[to_shub_msg->module].state = to_shub_msg->state;
	u8 cmd = CONN_MSG_TO_CMD(to_shub_msg->module, to_shub_msg->state);

	ret = sh_ipc_send_msg_sync(AP_SHUB_CONN_IPC_VCID, AP_SHUB_CONN_IPC_TAG, cmd, NULL, 0);
	if (ret < 0)
		pr_err("send_sync failed, ret:%d\n", ret);
}
EXPORT_SYMBOL(shub_conn_notify_state);

void shub_conn_regist_callback(enum shub_conn_module_e module,
								 shub_conn_ipc_cb cb_for_shub)
{
	if (module >= MODULE_NUM_MAX || module < MODULE_GNSS)
		return;

	if (cb_for_shub != NULL)
		g_conn_ipc_cb[module] = cb_for_shub;

	static bool is_registered = false;

	if (is_registered == false) {
		_regist_shub_ipc_msg_cb();
		is_registered = true;
	}
}
EXPORT_SYMBOL(shub_conn_regist_callback);

static int __init shub_conn_ipc_drv_init(void)
{
	int ret;

	ret = sh_recovery_notifier_register(&shub_conn_notifier);
	if (ret != 0)
		pr_err("register shub_conn_notifier failed!\n");
	pr_info("shub_conn_ipc_drv init done.\n");
	return ret;
}

static void __exit shub_conn_ipc_drv_exit(void)
{
	int ret;

	ret = sh_recovery_notifier_unregister(&shub_conn_notifier);
	if (ret != 0)
		pr_err("unregister shub_conn_notifier failed!\n");
	pr_info("shub_conn_ipc_drv exit.\n");
}

module_init(shub_conn_ipc_drv_init);
module_exit(shub_conn_ipc_drv_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("conn ipc interface between sensorhub and kernel");
MODULE_AUTHOR("xiaomi.com");
