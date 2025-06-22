// SPDX-License-Identifier: GPL-2.0
/*
 *ex_pd_protocol.h
 *
 * pd driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef XM_PD_ADAPTER_H
#define XM_PD_ADAPTER_H

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <mca/protocol/protocol_pd_class.h>
#include "tcpci_config.h"
#include "tcpm.h"

#define PCA_PPS_CMD_RETRY_COUNT 2

struct adapter_properties {
	const char *alias_name;
};

#define USB_PD_MI_SVID			0x2717
#define USBPD_UVDM_VERIFIED_LEN		1
#define USBPD_VDM_REQUEST		0x1

#define VDM_HDR(svid, cmd0, cmd1) \
	   (((svid) << 16) | (0 << 15) | ((cmd0) << 8) \
	   | (cmd1))
#define UVDM_HDR_CMD(hdr)	((hdr) & 0xFF)

struct ext_pd_protocol {
	struct device *dev;
	struct tcpc_device *tcpc;
	struct notifier_block pd_nb;
	struct task_struct *adapter_task;
	const char *adapter_dev_name;
	bool enable_kpoc_shdn;
	bool verify_process;
	bool has_dp;
	uint8_t role;
	uint8_t current_state;
	char reserve[3];
	struct tcpm_svid_list *adapter_svid_list;
	int verified;
	uint32_t adapter_svid;
	uint32_t adapter_id;
	uint32_t adapter_fw_ver;
	uint32_t adapter_hw_ver;
	int  uvdm_state;
	struct	 usbpd_vdm_data   vdm_data;

	int pd_active;
	int pd_cur_max;
	int pd_vol_min;
	int pd_vol_max;
	int pd_in_hard_reset;
	int typec_mode;
	int pd_usb_suspend_supported;
	int pd_apdo_max;
	int typec_accessory_mode;
	int pps_chg_stage;
	/* quirk for udp */
	bool xr_quirk_get_cc_orien_from_tcpm;
	bool otg_plugin_status;
	bool cc_toggle_status;
	struct hrtimer otg_switch_timer;
	struct work_struct otg_switch_work;
	bool otg_switch_timeout_flag;
	bool otg_device_plugin_flag;
};

enum pps_select_pdo_stage {
	MCA_PD_FIXED,
	MCA_PD_APDO,
	MCA_PD_APDO_START,
	MCA_PD_APDO_END,
};

int adapter_check_usb_psy(struct ext_pd_protocol *info);
int adapter_check_battery_psy(struct ext_pd_protocol *info);

#endif /*XM_PD_ADAPTER_H*/

