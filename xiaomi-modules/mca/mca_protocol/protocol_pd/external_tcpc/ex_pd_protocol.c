// SPDX-License-Identifier: GPL-2.0
/*
 *ex_pd_protocol.c
 *
 *pd protocol driver
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

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <mca/protocol/protocol_class.h>
#include "inc/ex_pd_protocol.h"
#include <linux/power_supply.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/common/mca_charge_interface.h>
#include <mca/common/mca_log.h>
#include <mca/platform/platform_sc6601a_cid_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include "inc/tcpm.h"
#include <mca/common/mca_event.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "sc6601a_pd_protocal"
#endif

#define PROBE_CNT_MAX	50
int get_apdo_regain;
struct ext_pd_protocol *g_ext_pd_protocol;

static const char * const usbpd_state_strings[] = {
    "UNKNOWN",
    /******************* Source *******************/
#ifdef CONFIG_USB_PD_PE_SOURCE
    "SRC_STARTUP",
    "SRC_DISCOVERY",
    "SRC_SEND_CAPABILITIES",
    "SRC_NEGOTIATE_CAPABILITIES",
    "SRC_TRANSITION_SUPPLY",
    "SRC_TRANSITION_SUPPLY2",
    "SRC_Ready",
    "SRC_DISABLED",
    "SRC_CAPABILITY_RESPONSE",
    "SRC_HARD_RESET",
    "SRC_HARD_RESET_RECEIVED",
    "SRC_TRANSITION_TO_DEFAULT",
    "SRC_GET_SINK_CAP",
    "SRC_WAIT_NEW_CAPABILITIES",
    "SRC_SEND_SOFT_RESET",
    "SRC_SOFT_RESET",
    /* Source Startup Discover Cable */
#ifdef CONFIG_USB_PD_SRC_STARTUP_DISCOVER_ID
#ifdef CONFIG_PD_SRC_RESET_CABLE
    "SRC_CBL_SEND_SOFT_RESET",
#endif	/* CONFIG_PD_SRC_RESET_CABLE */
    "SRC_VDM_IDENTITY_REQUEST",
    "SRC_VDM_IDENTITY_ACKED",
    "SRC_VDM_IDENTITY_NAKED",
#endif	/* PD_CAP_PE_SRC_STARTUP_DISCOVER_ID */
    /* Source for PD30 */
#ifdef CONFIG_USB_PD_REV30
    "SRC_SEND_NOT_SUPPORTED",
    "SRC_NOT_SUPPORTED_RECEIVED",
    "SRC_CHUNK_RECEIVED",
#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
    "SRC_SEND_SOURCE_ALERT",
#endif	/* CONFIG_USB_PD_REV30_ALERT_REMOTE */
#ifdef CONFIG_USB_PD_REV30_ALERT_REMOTE
    "SRC_SINK_ALERT_RECEIVED",
#endif	/* CONFIG_USB_PD_REV30_ALERT_REMOTE */
#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_LOCAL
    "SRC_GIVE_SOURCE_CAP_EXT",
#endif	/* CONFIG_USB_PD_REV30_SRC_CAP_EXT_LOCAL */
#ifdef CONFIG_SUPPORT_SOUTHCHIP_PDPHY
	"SRC_GIVE_SINK_CAP_EXT",
#endif
#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL
    "SRC_GIVE_SOURCE_STATUS",
#endif	/* CONFIG_USB_PD_REV30_STATUS_LOCAL */
#ifdef CONFIG_USB_PD_REV30_STATUS_REMOTE
    "SRC_GET_SINK_STATUS",
#endif	/* CONFIG_USB_PD_REV30_STATUS_REMOTE */
#ifdef CONFIG_USB_PD_REV30_PPS_SOURCE
    "SRC_GIVE_PPS_STATUS",
#endif	/* CONFIG_USB_PD_REV30_PPS_SOURCE */
#endif	/* CONFIG_USB_PD_REV30 */
#endif	/* CONFIG_USB_PD_PE_SOURCE */
    /******************* Sink *******************/
#ifdef CONFIG_USB_PD_PE_SINK
    /* Sink Init */
    "SNK_STARTUP",
    "SNK_DISCOVERY",
    "SNK_WAIT_FOR_CAPABILITIES",
    "SNK_EVALUATE_CAPABILITY",
    "SNK_SELECT_CAPABILITY",
    "SNK_TRANSITION_SINK",
    "SNK_Ready",
    "SNK_HARD_RESET",
    "SNK_TRANSITION_TO_DEFAULT",
    "SNK_GIVE_SINK_CAP",
    "SNK_GET_SOURCE_CAP",
    "SNK_SEND_SOFT_RESET",
    "SNK_SOFT_RESET",
    /* Sink for PD30 */
#ifdef CONFIG_USB_PD_REV30
    "SNK_SEND_NOT_SUPPORTED",
    "SNK_NOT_SUPPORTED_RECEIVED",
    "SNK_CHUNK_RECEIVED",
#ifdef CONFIG_USB_PD_REV30_ALERT_REMOTE
    "SNK_SOURCE_ALERT_RECEIVED",
#endif	/* CONFIG_USB_PD_REV30_ALERT_REMOTE */
#ifdef CONFIG_USB_PD_REV30_ALERT_LOCAL
    "SNK_SEND_SINK_ALERT",
#endif	/* CONFIG_USB_PD_REV30_ALERT_LOCAL */
#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE
    "SNK_GET_SOURCE_CAP_EXT",
#endif	/* CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE */
#ifdef CONFIG_USB_PD_REV30_STATUS_REMOTE
    "SNK_GET_SOURCE_STATUS",
#endif	/* CONFIG_USB_PD_REV30_STATUS_REMOTE */
#ifdef CONFIG_USB_PD_REV30_STATUS_LOCAL
    "SNK_GIVE_SINK_STATUS",
#endif	/* CONFIG_USB_PD_REV30_STATUS_LOCAL */
#ifdef CONFIG_USB_PD_REV30_PPS_SINK
    "SNK_GET_PPS_STATUS",
#endif	/* CONFIG_USB_PD_REV30_PPS_SINK */
#ifdef CONFIG_SUPPORT_SOUTHCHIP_PDPHY
	"SNK_GIVE_SINK_CAP_EXT",
#endif /* CONFIG_SUPPORT_SOUTHCHIP_PDPHY */
#endif	/* CONFIG_USB_PD_REV30 */
#endif	/* CONFIG_USB_PD_PE_SINK */
    /******************* DR_SWAP *******************/
#ifdef CONFIG_USB_PD_DR_SWAP
    /* DR_SWAP_DFP */
    "DRS_DFP_UFP_EVALUATE_DR_SWAP",
    "DRS_DFP_UFP_ACCEPT_DR_SWAP",
    "DRS_DFP_UFP_CHANGE_TO_UFP",
    "DRS_DFP_UFP_SEND_DR_SWAP",
    "DRS_DFP_UFP_REJECT_DR_SWAP",
    /* DR_SWAP_UFP */
    "DRS_UFP_DFP_EVALUATE_DR_SWAP",
    "DRS_UFP_DFP_ACCEPT_DR_SWAP",
    "DRS_UFP_DFP_CHANGE_TO_DFP",
    "DRS_UFP_DFP_SEND_DR_SWAP",
    "DRS_UFP_DFP_REJECT_DR_SWAP",
#endif	/* CONFIG_USB_PD_DR_SWAP */
    /******************* PR_SWAP *******************/
#ifdef CONFIG_USB_PD_PR_SWAP
    /* PR_SWAP_SRC */
    "PRS_SRC_SNK_EVALUATE_PR_SWAP",
    "PRS_SRC_SNK_ACCEPT_PR_SWAP",
    "PRS_SRC_SNK_TRANSITION_TO_OFF",
    "PRS_SRC_SNK_ASSERT_RD",
    "PRS_SRC_SNK_WAIT_SOURCE_ON",
    "PRS_SRC_SNK_SEND_SWAP",
    "PRS_SRC_SNK_REJECT_PR_SWAP",
    /* PR_SWAP_SNK */
    "PRS_SNK_SRC_EVALUATE_PR_SWAP",
    "PRS_SNK_SRC_ACCEPT_PR_SWAP",
    "PRS_SNK_SRC_TRANSITION_TO_OFF",
    "PRS_SNK_SRC_ASSERT_RP",
    "PRS_SNK_SRC_SOURCE_ON",
    "PRS_SNK_SRC_SEND_SWAP",
    "PRS_SNK_SRC_REJECT_SWAP",
    /* get same role cap */
    "DR_SRC_GET_SOURCE_CAP",
    "DR_SRC_GIVE_SINK_CAP",
    "DR_SNK_GET_SINK_CAP",
    "DR_SNK_GIVE_SOURCE_CAP",
    /* get same role cap for PD30 */
#ifdef CONFIG_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_LOCAL
    "DR_SNK_GIVE_SOURCE_CAP_EXT",
#endif	/* CONFIG_USB_PD_REV30_SRC_CAP_EXT_LOCAL */
#ifdef CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE
	"DR_SRC_GET_SOURCE_CAP_EXT",
#endif	/* CONFIG_USB_PD_REV30_SRC_CAP_EXT_REMOTE */
#ifdef CONFIG_SUPPORT_SOUTHCHIP_PDPHY
	"DR_SRC_GIVE_SINK_CAP_EXT",
#endif /* CONFIG_SUPPORT_SOUTHCHIP_PDPHY */
#endif	/* CONFIG_USB_PD_REV30 */
#endif	/* CONFIG_USB_PD_PR_SWAP */
    /******************* VCONN_SWAP *******************/
#ifdef CONFIG_USB_PD_VCONN_SWAP
    "VCS_SEND_SWAP",
    "VCS_EVALUATE_SWAP",
    "VCS_ACCEPT_SWAP",
    "VCS_REJECT_VCONN_SWAP",
    "VCS_WAIT_FOR_VCONN",
    "VCS_TURN_OFF_VCONN",
    "VCS_TURN_ON_VCONN",
    "VCS_SEND_PS_RDY",
#endif	/* CONFIG_USB_PD_VCONN_SWAP */
    /******************* UFP_VDM *******************/
    "UFP_VDM_GET_IDENTITY",
    "UFP_VDM_GET_SVIDS",
    "UFP_VDM_GET_MODES",
    "UFP_VDM_EVALUATE_MODE_ENTRY",
    "UFP_VDM_MODE_EXIT",
    "UFP_VDM_ATTENTION_REQUEST",
#ifdef CONFIG_USB_PD_ALT_MODE
    "UFP_VDM_DP_STATUS_UPDATE",
    "UFP_VDM_DP_CONFIGURE",
#endif/* CONFIG_USB_PD_ALT_MODE */
    /******************* DFP_VDM *******************/
    "DFP_UFP_VDM_IDENTITY_REQUEST",
    "DFP_UFP_VDM_IDENTITY_ACKED",
    "DFP_UFP_VDM_IDENTITY_NAKED",
    "DFP_CBL_VDM_IDENTITY_REQUEST",
    "DFP_CBL_VDM_IDENTITY_ACKED",
    "DFP_CBL_VDM_IDENTITY_NAKED",
    "DFP_VDM_SVIDS_REQUEST",
    "DFP_VDM_SVIDS_ACKED",
    "DFP_VDM_SVIDS_NAKED",
    "DFP_VDM_MODES_REQUEST",
    "DFP_VDM_MODES_ACKED",
    "DFP_VDM_MODES_NAKED",
    "DFP_VDM_MODE_ENTRY_REQUEST",
    "DFP_VDM_MODE_ENTRY_ACKED",
    "DFP_VDM_MODE_ENTRY_NAKED",
    "DFP_VDM_MODE_EXIT_REQUEST",
    "DFP_VDM_MODE_EXIT_ACKED",
    "DFP_VDM_ATTENTION_REQUEST",
#ifdef CONFIG_PD_DFP_RESET_CABLE
    "DFP_CBL_SEND_SOFT_RESET",
    "DFP_CBL_SEND_CABLE_RESET",
#endif	/* CONFIG_PD_DFP_RESET_CABLE */
#ifdef CONFIG_USB_PD_ALT_MODE_DFP
    "DFP_VDM_DP_STATUS_UPDATE_REQUEST",
    "DFP_VDM_DP_STATUS_UPDATE_ACKED",
    "DFP_VDM_DP_STATUS_UPDATE_NAKED",
    "DFP_VDM_DP_CONFIGURATION_REQUEST",
    "DFP_VDM_DP_CONFIGURATION_ACKED",
    "DFP_VDM_DP_CONFIGURATION_NAKED",
#endif/* CONFIG_USB_PD_ALT_MODE_DFP */
    /******************* UVDM & SVDM *******************/
#ifdef CONFIG_USB_PD_CUSTOM_VDM
    "UFP_UVDM_RECV",
    "DFP_UVDM_SEND",
    "DFP_UVDM_ACKED",
    "DFP_UVDM_NAKED",
#endif/* CONFIG_USB_PD_CUSTOM_VDM */
    /******************* PD30 Common *******************/
#ifdef CONFIG_USB_PD_REV30
#ifdef CONFIG_USB_PD_REV30_BAT_CAP_REMOTE
    "GET_BATTERY_CAP",
#endif	/* CONFIG_USB_PD_REV30_BAT_CAP_REMOTE */
#ifdef CONFIG_USB_PD_REV30_BAT_CAP_LOCAL
    "GIVE_BATTERY_CAP",
#endif	/* CONFIG_USB_PD_REV30_BAT_CAP_LOCAL */
#ifdef CONFIG_USB_PD_REV30_BAT_STATUS_REMOTE
    "GET_BATTERY_STATUS",
#endif	/* CONFIG_USB_PD_REV30_BAT_STATUS_REMOTE */
#ifdef CONFIG_USB_PD_REV30_BAT_STATUS_LOCAL
    "GIVE_BATTERY_STATUS",
#endif	/* CONFIG_USB_PD_REV30_BAT_STATUS_LOCAL */
#ifdef CONFIG_USB_PD_REV30_MFRS_INFO_REMOTE
    "GET_MANUFACTURER_INFO",
#endif	/* CONFIG_USB_PD_REV30_MFRS_INFO_REMOTE */
#ifdef CONFIG_USB_PD_REV30_MFRS_INFO_LOCAL
    "GIVE_MANUFACTURER_INFO",
#endif	/* CONFIG_USB_PD_REV30_MFRS_INFO_LOCAL */
#ifdef CONFIG_USB_PD_REV30_COUNTRY_CODE_REMOTE
    "GET_COUNTRY_CODES",
#endif	/* CONFIG_USB_PD_REV30_COUNTRY_CODE_REMOTE */
#ifdef CONFIG_USB_PD_REV30_COUNTRY_CODE_LOCAL
    "GIVE_COUNTRY_CODES",
#endif	/* CONFIG_USB_PD_REV30_COUNTRY_CODE_LOCAL */
#ifdef CONFIG_USB_PD_REV30_COUNTRY_INFO_REMOTE
    "GET_COUNTRY_INFO",
#endif	/* CONFIG_USB_PD_REV30_COUNTRY_INFO_REMOTE */
#ifdef CONFIG_USB_PD_REV30_COUNTRY_INFO_LOCAL
    "GIVE_COUNTRY_INFO",
#endif	/* CONFIG_USB_PD_REV30_COUNTRY_INFO_LOCAL */
    "VDM_NOT_SUPPORTED",
#endif /* CONFIG_USB_PD_REV30 */
#ifdef CONFIG_SUPPORT_SOUTHCHIP_PDPHY
		"GIVE_REVISION",
#endif /* CONFIG_SUPPORT_SOUTHCHIP_PDPHY */

    /******************* Others *******************/
#ifdef CONFIG_USB_PD_CUSTOM_DBGACC
    "DBG_READY",
#endif/* CONFIG_USB_PD_CUSTOM_DBGACC */
#ifdef CONFIG_USB_PD_RECV_HRESET_COUNTER
    "OVER_RECV_HRESET_LIMIT",
#endif/* CONFIG_USB_PD_RECV_HRESET_COUNTER */
    "REJECT",
    "WAIT"
    "ERROR_RECOVERY",
#ifdef CONFIG_USB_PD_ERROR_RECOVERY_ONCE
    "ERROR_RECOVERY_ONCE",
#endif	/* CONFIG_USB_PD_ERROR_RECOVERY_ONCE */
    "BIST_TEST_DATA",
    "BIST_CARRIER_MODE_2",
    /* Wait tx finished */
    "IDLE1",
    "IDLE2",
    "PD_NR_PE_STATES",
};

static void usbpd_mi_vdm_received(struct ext_pd_protocol *pinfo, struct tcp_ny_uvdm uvdm)
{
	int i, cmd;

	if (uvdm.uvdm_svid != USB_PD_MI_SVID)
		return;

	cmd = UVDM_HDR_CMD(uvdm.uvdm_data[0]);
	mca_log_info("cmd = %d\n", cmd);

	mca_log_info("uvdm.ack: %d, uvdm.uvdm_cnt: %d, uvdm.uvdm_svid: 0x%04x\n",
			uvdm.ack, uvdm.uvdm_cnt, uvdm.uvdm_svid);

	switch (cmd) {
	case USBPD_UVDM_CHARGER_VERSION:
		pinfo->vdm_data.ta_version = uvdm.uvdm_data[1];
		mca_log_info("ta_version:%x\n", pinfo->vdm_data.ta_version);
		break;
	case USBPD_UVDM_CHARGER_TEMP:
		pinfo->vdm_data.ta_temp = (uvdm.uvdm_data[1] & 0xFFFF) * 10;
		mca_log_info("pinfo->adapter_dev->vdm_data.ta_temp:%d\n", pinfo->vdm_data.ta_temp);
		break;
	case USBPD_UVDM_CHARGER_VOLTAGE:
		pinfo->vdm_data.ta_voltage = (uvdm.uvdm_data[1] & 0xFFFF) * 10;
		pinfo->vdm_data.ta_voltage *= 1000; /*V->mV*/
		mca_log_info("ta_voltage:%d\n", pinfo->vdm_data.ta_voltage);
		break;
	case USBPD_UVDM_SESSION_SEED:
		for (i = 0; i < USBPD_UVDM_SS_LEN; i++) {
			pinfo->vdm_data.s_secert[i] = uvdm.uvdm_data[i+1];
			mca_log_info("usbpd s_secert uvdm.uvdm_data[%d]=0x%x", i+1, uvdm.uvdm_data[i+1]);
		}
		break;
	case USBPD_UVDM_AUTHENTICATION:
		for (i = 0; i < USBPD_UVDM_SS_LEN; i++) {
			pinfo->vdm_data.digest[i] = uvdm.uvdm_data[i+1];
			mca_log_info("usbpd digest[%d]=0x%x", i+1, uvdm.uvdm_data[i+1]);
		}
		break;
	case USBPD_UVDM_REVERSE_AUTHEN:
		pinfo->vdm_data.reauth = (uvdm.uvdm_data[1] & 0xFFFF);
		break;
	default:
		break;
	}
	pinfo->uvdm_state = cmd;
}

static int ex_pd_tcp_notifier_call(struct notifier_block *pnb,
				unsigned long event, void *data)
{
	struct tcp_notify *noti = data;
	struct ext_pd_protocol *pinfo = g_ext_pd_protocol;

	mca_log_err("PD charger event:%d %d\n", (int)event,
		(int)noti->pd_state.connected);
	switch (event) {
	case TCP_NOTIFY_PD_STATE:
		switch (noti->pd_state.connected) {
		case  PD_CONNECT_NONE:
			pinfo->adapter_id = 0;
			pinfo->adapter_svid = 0;
			pinfo->uvdm_state = USBPD_UVDM_DISCONNECT;
			pinfo->verified = 0;
			pinfo->verify_process = 0;
			break;
		case PD_CONNECT_PE_READY_SNK_PD30:
			pinfo->uvdm_state = USBPD_UVDM_CONNECT;
			break;
		case PD_CONNECT_PE_READY_SNK_APDO:
			get_apdo_regain = 1;
			pinfo->uvdm_state = USBPD_UVDM_CONNECT;
			break;
		};
		break;
	case TCP_NOTIFY_UVDM:
		mca_log_info("%s: tcpc received uvdm message.\n", __func__);
		usbpd_mi_vdm_received(pinfo, noti->uvdm_msg);
		break;
	}
	return NOTIFY_OK;
}

int pd_get_adapter_id(uint32_t *adapter_id, void *data)
{
	struct ext_pd_protocol *info = g_ext_pd_protocol;
	struct pd_source_cap_ext cap_ext;
	int ret;
	int i = 0;
	uint32_t pd_vdos[8];

	if (info == NULL)
		return -EINVAL;

	mca_log_info("%s: enter\n", __func__);
#if 0
	if (info->adapter_id != 0)
		return 0;
#endif
	ret = tcpm_inquire_pd_partner_inform(info->tcpc, pd_vdos);
	if (ret == TCPM_SUCCESS) {
		mca_log_info("find adapter id success.\n");
		for (i = 0; i < 8; i++)
			mca_log_info("VDO[%d] : %08x\n", i, pd_vdos[i]);

		info->adapter_id = pd_vdos[2] & 0x0000FFFF;
		mca_log_info("adapter_id = %08x\n", info->adapter_id);
	} else {
		ret = tcpm_dpm_pd_get_source_cap_ext(info->tcpc,
			NULL, &cap_ext);
		if (ret == TCPM_SUCCESS) {
			info->adapter_id = cap_ext.pid & 0x0000FFFF;
			mca_log_info("adapter_id = %08x\n", info->adapter_id);
		} else {
			mca_log_err("[%s] get adapter message failed!\n", __func__);
			return ret;
		}
	}
	*adapter_id = info->adapter_id;

	return 0;
}

int pd_get_adapter_svid(uint32_t *adapter_svid, void *data)
{
	struct ext_pd_protocol *info = g_ext_pd_protocol;
	struct pd_source_cap_ext cap_ext;
	int ret;
	int i = 0;
	uint32_t pd_vdos[8];

	if (info == NULL)
		return -EINVAL;

	mca_log_info("%s: enter\n", __func__);
#if 0
	if (info->adapter_svid != 0)
		return 0;
#endif
	if (info->adapter_svid_list == NULL) {
		if (in_interrupt()) {
			info->adapter_svid_list = kmalloc(sizeof(struct tcpm_svid_list), GFP_ATOMIC);
		} else {
			info->adapter_svid_list = kmalloc(sizeof(struct tcpm_svid_list), GFP_KERNEL);
		}
		if (info->adapter_svid_list == NULL)
			mca_log_err("[%s] adapter_svid_list is still NULL!\n", __func__);
	}

	ret = tcpm_inquire_pd_partner_inform(info->tcpc, pd_vdos);
	if (ret != TCPM_SUCCESS) {
		mca_log_info("tcpm_inquire_pd_partner_inform fail .\n");
		tcpm_dpm_vdm_discover_id(info->tcpc, NULL);
		msleep(50);
	}
	ret = tcpm_inquire_pd_partner_inform(info->tcpc, pd_vdos);
	if (ret == TCPM_SUCCESS) {
		mca_log_info("find adapter id success.\n");
		for (i = 0; i < 8; i++)
			mca_log_info("VDO[%d] : %08x\n", i, pd_vdos[i]);

		info->adapter_svid = pd_vdos[0] & 0x0000FFFF;
		mca_log_info("adapter_svid = %04x\n", info->adapter_svid);

		ret = tcpm_inquire_pd_partner_svids(info->tcpc, info->adapter_svid_list);
		mca_log_info("[%s] tcpm_inquire_pd_partner_svids, ret=%d!\n", __func__, ret);
		if (ret == TCPM_SUCCESS) {
			mca_log_info("discover svid number is %d\n", info->adapter_svid_list->cnt);
			for (i = 0; i < info->adapter_svid_list->cnt; i++) {
				mca_log_info("SVID[%d] : %04x\n", i, info->adapter_svid_list->svids[i]);
				if (info->adapter_svid_list->svids[i] == USB_PD_MI_SVID)
					info->adapter_svid = USB_PD_MI_SVID;
			}
		}
	} else {
		ret = tcpm_dpm_pd_get_source_cap_ext(info->tcpc,
			NULL, &cap_ext);
		if (ret == TCPM_SUCCESS) {
			info->adapter_svid = cap_ext.vid & 0x0000FFFF;
			info->adapter_fw_ver = cap_ext.fw_ver & 0x0000FFFF;
			info->adapter_hw_ver = cap_ext.hw_ver & 0x0000FFFF;
			mca_log_info("adapter_svid = %04x\n", info->adapter_svid);
			mca_log_info("adapter_fw_ver = %08x\n", info->adapter_fw_ver);
			mca_log_info("adapter_hw_ver = %08x\n", info->adapter_hw_ver);
		} else {
			mca_log_err("[%s] get adapter message failed!\n", __func__);
			return ret;
		}
	}
	*adapter_svid = info->adapter_svid;

	return 0;
}

#define BSWAP_32(x) \
	(u32)((((u32)(x) & 0xff000000) >> 24) | \
			(((u32)(x) & 0x00ff0000) >> 8) | \
			(((u32)(x) & 0x0000ff00) << 8) | \
			(((u32)(x) & 0x000000ff) << 24))

static void usbpd_sha256_bitswap32(unsigned int *array, int len)
{
	int i;

	for (i = 0; i < len; i++)
		array[i] = BSWAP_32(array[i]);
}
#if 0
void charToint(char *str, int input_len, unsigned int *out, unsigned int *outlen)
{
	int i;

	if (outlen != NULL)
		*outlen = 0;
	for (i = 0; i < (input_len / 4 + 1); i++) {
		out[i] = ((str[i*4 + 3] * 0x1000000) |
				(str[i*4 + 2] * 0x10000) |
				(str[i*4 + 1] * 0x100) |
				str[i*4]);
		*outlen = *outlen + 1;
	}

	mca_log_info("%s: outlen = %d\n", __func__, *outlen);
	for (i = 0; i < *outlen; i++)
		mca_log_info("%s: out[%d] = %08x\n", __func__, i, out[i]);
	mca_log_info("%s: char to int done.\n", __func__);
}
#endif
static int tcp_dpm_event_cb_uvdm(struct tcpc_device *tcpc, int ret,
				 struct tcp_dpm_event *event)
{
	int i;
	struct tcp_dpm_custom_vdm_data vdm_data = event->tcp_dpm_data.vdm_data;

	mca_log_info("%s: vdm_data.cnt = %d\n", __func__, vdm_data.cnt);
	for (i = 0; i < vdm_data.cnt; i++)
		mca_log_info("%s vdm_data.vdos[%d] = 0x%08x", __func__, i,
			vdm_data.vdos[i]);
	return 0;
}

const struct tcp_dpm_event_cb_data cb_data = {
	.event_cb = tcp_dpm_event_cb_uvdm,
};

static int pd_request_vdm_cmd(enum uvdm_state cmd, unsigned int *cdata, unsigned int data_len, void *data)
{
	u32 vdm_hdr = 0;
	int rc = 0;
	struct tcp_dpm_custom_vdm_data *vdm_data;
	struct ext_pd_protocol *info;
	int i;

	if (in_interrupt()) {
		vdm_data = kmalloc(sizeof(*vdm_data), GFP_ATOMIC);
		mca_log_info("%s: kmalloc atomic ok.\n", __func__);

	} else {
		vdm_data = kmalloc(sizeof(*vdm_data), GFP_KERNEL);
		mca_log_info("%s: kmalloc kernel ok.\n", __func__);
	}

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL) {
		rc = -EINVAL;
		goto release_req;
	}

	vdm_hdr = VDM_HDR(info->adapter_svid, USBPD_VDM_REQUEST, cmd);
	vdm_data->wait_resp = true;
	vdm_data->vdos[0] = vdm_hdr;

	switch (cmd) {
	case USBPD_UVDM_CHARGER_VERSION:
	case USBPD_UVDM_CHARGER_TEMP:
	case USBPD_UVDM_CHARGER_VOLTAGE:
		vdm_data->cnt = 1;
		rc = tcpm_dpm_send_custom_vdm(info->tcpc, vdm_data, &cb_data);//&tcp_dpm_evt_cb_null
		if (rc < 0) {
			mca_log_err("failed to send %d\n", cmd);
			goto release_req;
		}
		break;
	case USBPD_UVDM_VERIFIED:
	case USBPD_UVDM_REMOVE_COMPENSATION:
		vdm_data->cnt = 1 + USBPD_UVDM_VERIFIED_LEN;

		for (i = 0; i < USBPD_UVDM_VERIFIED_LEN; i++)
			vdm_data->vdos[i + 1] = cdata[i];
		mca_log_info("verify-0: %08x\n", vdm_data->vdos[1]);

		rc = tcpm_dpm_send_custom_vdm(info->tcpc, vdm_data, &cb_data);//&tcp_dpm_evt_cb_null
		if (rc < 0) {
			mca_log_err("failed to send %d\n", cmd);
			goto release_req;
		}
		break;
	case USBPD_UVDM_SESSION_SEED:
	case USBPD_UVDM_AUTHENTICATION:
	case USBPD_UVDM_REVERSE_AUTHEN:
		usbpd_sha256_bitswap32(cdata, USBPD_UVDM_SS_LEN);
		vdm_data->cnt = 1 + USBPD_UVDM_SS_LEN;
		for (i = 0; i < USBPD_UVDM_SS_LEN; i++)
			vdm_data->vdos[i + 1] = cdata[i];

		for (i = 0; i < USBPD_UVDM_SS_LEN; i++)
			mca_log_info("%08x\n", vdm_data->vdos[i+1]);

		rc = tcpm_dpm_send_custom_vdm(info->tcpc, vdm_data, &cb_data);//&tcp_dpm_evt_cb_null
		if (rc < 0) {
			mca_log_err("failed to send %d\n", cmd);
			goto release_req;
		}
		break;
	case USBPD_UVDM_RESET_VSAFE0V:
		vdm_data->cnt = 1 + USBPD_UVDM_SS_LEN;
		for (i = 0; i < USBPD_UVDM_VERIFIED_LEN; i++)
			vdm_data->vdos[i + 1] = cdata[i];
		rc = tcpm_dpm_send_custom_vdm(info->tcpc, vdm_data, &cb_data);//&tcp_dpm_evt_cb_null
		if (rc < 0) {
			mca_log_err("antiburn failed to send %d\n", cmd);
			goto release_req;
		}
		tcpci_ops_get_antiburn_status(info->tcpc, true);
		break;
	default:
		mca_log_err("cmd:%d is not support\n", cmd);
		break;
	}

release_req:
	if (vdm_data != NULL)
		kfree(vdm_data);
	return rc;
}

int pd_get_vdm_cmd(int *cmd, struct usbpd_vdm_data *vdm_data, void *data)
{
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*cmd = info->uvdm_state;
	*vdm_data = info->vdm_data;

	return 0;
}

static int pd_get_power_role(unsigned char *role, void *data)
{
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->role = tcpm_inquire_pd_power_role(info->tcpc);
	*role = info->role;
	mca_log_err("[%s] power role is %d\n", __func__, info->role);
	return 0;
}

static void ex_pd_convert_state_to_string(char *state, struct ext_pd_protocol *info)
{
    if (info->current_state >= (sizeof(usbpd_state_strings) / sizeof(usbpd_state_strings[0]))) {
        info->current_state = 0;
        strcpy(state, "UNKNOWN");
        mca_log_err("%s: current_state out of range %s\n", __func__, usbpd_state_strings[0]);
        return;
    } else {
        strcpy(state, usbpd_state_strings[info->current_state]);
        mca_log_err("%s: %s\n", __func__, usbpd_state_strings[info->current_state]);
        return;
    }
}

static int pd_get_current_state(char *current_state, int len, void *data)
{
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL || len < PROTOCOL_PD_MAX_STRING_LEN)
		return -EINVAL;

	info->current_state = tcpm_inquire_pd_state_curr(info->tcpc);
	ex_pd_convert_state_to_string(current_state, info);
	mca_log_err("[%s] current state is %d\n", __func__, info->current_state);
	return 0;
}

static int pd_get_pdos(struct pd_pdo *received_pdos, int count, void *data)
{
	struct ext_pd_protocol *info;
	struct tcpm_power_cap cap;
	int ret, i;
	int pd_wait_cnt = 0;
	struct tcpm_remote_power_cap pd_cap;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	ret = tcpm_inquire_pd_source_cap(info->tcpc, &cap);
	mca_log_err("[%s] test01 tcpm_inquire_pd_source_cap is %d.\n", __func__, ret);
	if (ret < 0) {
		for(pd_wait_cnt = 0; pd_wait_cnt < 25; pd_wait_cnt++){
			msleep(20);
			mca_log_err("retry [%s] tcpm_inquire_pd_source_cap is %d. cnt =%d\n", __func__, ret, pd_wait_cnt);
			ret = tcpm_inquire_pd_source_cap(info->tcpc, &cap);
			if(ret == 0)
				break;
		}
	}

	if (count > 7)
		count = 7;
	for (i = 0; i < count; i++) {
		//received_pdos[i] = cap.pdos[i];
		mca_log_err("[%s]: pdo[%d] { received_pdos is %08x, cap.pdos is %08x}\n",
			__func__, i, cap.pdos[i], cap.pdos[i]);
	}

	tcpm_get_remote_power_cap(info->tcpc, &pd_cap);

	for (i = 0; i < pd_cap.nr; i++) {
			received_pdos[i].max_current = pd_cap.ma[i];
			received_pdos[i].max_volt = pd_cap.max_mv[i];
			received_pdos[i].min_volt = pd_cap.min_mv[i];

			mca_log_err("[%s]:%d mv:[%d,%d] curent:%d\n",
				__func__, i, received_pdos[i].min_volt,
				received_pdos[i].max_volt, received_pdos[i].max_current);
		}

	return 0;
}

static int pd_set_verify_process(int verify_process, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->verify_process = verify_process;
	mca_log_err("[%s] pd verify in process:%d\n",
		__func__, verify_process);

	return ret;
}

static int pd_get_verify_process(int *verify_process, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*verify_process = info->verify_process;
	mca_log_err("[%s] pd verify in process:%d\n",
		__func__, *verify_process);

	return ret;
}

static int pd_get_cap(struct ext_pd_protocol *info,
	enum adapter_cap_type type,
	struct adapter_power_cap *tacap)
{
	int ret;
	int i;
	int timeout = 0;
	struct tcpm_remote_power_cap pd_cap;

	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	if (info->verify_process)
		return -1;

	if (type == XM_PD) {
APDO_REGAIN:
		pd_cap.nr = 0;
		pd_cap.selected_cap_idx = 0;
		tcpm_get_remote_power_cap(info->tcpc, &pd_cap);

		tacap->nr = pd_cap.nr;
		tacap->selected_cap_idx = pd_cap.selected_cap_idx - 1;
		mca_log_err("[%s] nr:%d idx:%d\n",
		__func__, pd_cap.nr, pd_cap.selected_cap_idx - 1);
		for (i = 0; i < pd_cap.nr; i++) {
			tacap->ma[i] = pd_cap.ma[i];
			tacap->max_mv[i] = pd_cap.max_mv[i];
			tacap->min_mv[i] = pd_cap.min_mv[i];
			tacap->maxwatt[i] = tacap->max_mv[i] * tacap->ma[i];
			tacap->type[i] = pd_cap.type[i];
			mca_log_err("[%s]:%d mv:[%d,%d] %d max:%d min:%d type:%d %d\n",
				__func__, i, tacap->min_mv[i],
				tacap->max_mv[i], tacap->ma[i],
				tacap->maxwatt[i], tacap->minwatt[i],
				tacap->type[i], pd_cap.type[i]);
		}
	}  else if (type == XM_PD_APDO_REGAIN) {
		get_apdo_regain = 0;
		ret = tcpm_dpm_pd_get_source_cap(info->tcpc, NULL);
		if (ret == TCPM_SUCCESS) {
			while (timeout < 10) {
				if (get_apdo_regain) {
					mca_log_err("[%s] ready to get pps info!\n", __func__);
					goto APDO_REGAIN;
				} else {
					msleep(100);
					timeout++;
				}
			}
			mca_log_err("[%s] ready to get pps info - for test!\n", __func__);
			goto APDO_REGAIN;
		} else {
			mca_log_err("[%s] tcpm_dpm_pd_get_source_cap failed!\n", __func__);
			return -EINVAL;
		}
	}
	mca_log_err("[%s] tacap->nr is %d\n", __func__, tacap->nr);

	return 0;
}

static int pd_set_usbpd_verifed(int usbpd_verifed, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;
	struct adapter_power_cap cap = {0};

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->verified = usbpd_verifed;
	mca_log_err("[%s] set pd verify:%d\n", __func__, usbpd_verifed);

	if (info->verified)
		pd_get_cap(info, XM_PD_APDO_REGAIN, &cap);

	return ret;
}

static int pd_get_usbpd_verifed(int *usbpd_verifed, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*usbpd_verifed = info->verified;
	mca_log_err("[%s] get pd verify:%d\n", __func__, *usbpd_verifed);

	return ret;
}

static int pd_set_pd_active(int pd_active, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_active = pd_active;
    mca_log_err("[%s] set pd active vaule = %d\n", __func__, pd_active);
	return ret;
}

static int pd_get_pd_active(int *pd_active, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*pd_active = info->pd_active;
    mca_log_err("[%s] get pd active vaule = %d\n", __func__, *pd_active);
	return ret;
}

static int pd_set_pps_max_curr(u32 max_curr, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_cur_max = max_curr;

	return ret;
}

static int pd_get_pps_max_curr(u32 *max_curr, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*max_curr = info->pd_vol_min;

	return ret;
}

static int pd_set_pps_min_volt(u32 volt, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_vol_min = volt;

	return ret;
}

static int pd_get_pps_min_volt(u32 *volt, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*volt = info->pd_vol_min;

	return ret;
}

static int pd_set_pps_max_volt(u32 volt, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_vol_max = volt;

	return ret;
}

static int pd_get_pps_max_volt(u32 *volt, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*volt = info->pd_vol_max;

	return ret;
}

static int pd_set_pps_apdo_max(u32 apdo_max, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_apdo_max = apdo_max;

	return ret;
}

static int pd_get_pps_apdo_max(u32 *apdo_max, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*apdo_max = info->pd_apdo_max;

	return ret;
}

static int pd_set_typec_mode(int typec_mode, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->typec_mode = typec_mode;

	return ret;
}

static int pd_get_typec_mode(int *typec_mode, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*typec_mode = info->typec_mode;

	return ret;
}

static int pd_get_typec_cc_orientation(int *cc_orientation, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*cc_orientation = tcpm_inquire_cc_polarity(info->tcpc)+1;

	return ret;
}

static int pd_set_pd_in_hard_reset(int in_hard_reset, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_in_hard_reset = in_hard_reset;

	return ret;
}

static int pd_get_pd_in_hard_reset(int *in_hard_reset, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*in_hard_reset = info->pd_in_hard_reset;

	return ret;
}

static int pd_set_usb_suspend_supported(int usb_suspend_supported, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->pd_usb_suspend_supported = usb_suspend_supported;

	return ret;
}

static int pd_get_usb_suspend_supported(int *usb_suspend_supported, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*usb_suspend_supported = info->pd_usb_suspend_supported;

	return ret;
}

static int pd_set_typec_accessory_mode(int typec_accessory_mode, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	info->typec_accessory_mode = typec_accessory_mode;

	return ret;
}

static int pd_get_typec_accessory_mode(int *typec_accessory_mode, void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

	*typec_accessory_mode = info->typec_accessory_mode;

	return ret;
}

static int ex_pd_protocol_get_pd_type(int *type, void *data)
{
    struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (info == NULL || info->tcpc == NULL)
		return -EINVAL;

    switch (info->pd_active) {
    case QTI_POWER_SUPPLY_PD_ACTIVE:
        *type = XM_CHARGER_TYPE_PD;
        info->pps_chg_stage = MCA_PD_FIXED;
        break;
    case QTI_POWER_SUPPLY_PD_PPS_ACTIVE:
	if (info->verified) {
		*type = XM_CHARGER_TYPE_PD_VERIFY;
		mca_log_info("subpmic get PD VERIFY successful\n");
	} else {
		*type = XM_CHARGER_TYPE_PPS;
		mca_log_info("subpmic get adapter type is pps\n");
	}
        info->pps_chg_stage = MCA_PD_APDO_START;
        (void)platform_class_cp_enable_adc(CP_ROLE_MASTER, true);
        (void)platform_class_cp_enable_adc(CP_ROLE_SLAVE, true);
        break;
    case QTI_POWER_SUPPLY_PD_INACTIVE:
        *type = QTI_POWER_SUPPLY_PD_INACTIVE;
        info->pps_chg_stage = MCA_PD_APDO_END;
        break;
    default:
        break;
      }
    mca_log_err("%s,pd_active = %d, type = %d\n", __func__, info->pd_active, *type);
    return 0;
}

static int get_pps_chg_stage(unsigned int vbus_mv)
{
    struct ext_pd_protocol *info;
    int stage = MCA_PD_FIXED;

    info = g_ext_pd_protocol;

    if (info->pd_active != QTI_POWER_SUPPLY_PD_PPS_ACTIVE) {
        info->pps_chg_stage = MCA_PD_FIXED;
    }
    else {
        switch (info->pps_chg_stage) {
        case MCA_PD_APDO_START:
            stage = info->pps_chg_stage;
            info->pps_chg_stage = MCA_PD_APDO;
            break;
        case MCA_PD_APDO:
            if (vbus_mv > 5000) {
                stage = info->pps_chg_stage;
                break;
            } else if (vbus_mv == 5000) {
                //fallthrough;
				stage = info->pps_chg_stage;
                break;
            } else
				break;
        case MCA_PD_APDO_END:
            stage = MCA_PD_APDO_END;
            info->pps_chg_stage = MCA_PD_APDO_END;
            break;
        default:
            stage = MCA_PD_FIXED;
            info->pps_chg_stage = MCA_PD_FIXED;
        break;
        }
    }
    mca_log_err("%s: %d, pps_chg_stage = %d\n", __func__, stage, info->pps_chg_stage);
    return stage;
}

static inline int check_typec_attached_snk(struct tcpc_device *tcpc)
{
	if (tcpm_inquire_typec_attach_state(tcpc) != TYPEC_ATTACHED_SNK)
		return -EINVAL;

	return 0;
}

static int ex_pd_protocol_select_pps_pdo(unsigned int vbus_mv, unsigned int ibus_ma, void *data)
{
	struct ext_pd_protocol *info = g_ext_pd_protocol;
	int ret = 0, cnt = 0;
	int chg_stage;

	if (check_typec_attached_snk(info->tcpc) < 0)
		return -EINVAL;
	mca_log_info("%s,request ibus = %d, request vbus = %d\n", __func__, ibus_ma, vbus_mv);

	if (!tcpm_inquire_pd_connected(info->tcpc)) {
		mca_log_err("pd not connected\n");
		return -EINVAL;
	}
	msleep(500);
	chg_stage = get_pps_chg_stage(vbus_mv);
	mca_log_info("%s: get chg_stage is %d\n", __func__, chg_stage);
	do {
		if (chg_stage == MCA_PD_APDO_START) {
			ret = tcpm_set_apdo_charging_policy(info->tcpc,
			DPM_CHARGING_POLICY_PPS, vbus_mv, ibus_ma, NULL);
		} else if (chg_stage == MCA_PD_APDO) {
			ret = tcpm_dpm_pd_request(info->tcpc, vbus_mv, ibus_ma, NULL);
		} else if (chg_stage == MCA_PD_APDO_END) {
			ret = tcpm_set_pd_charging_policy(info->tcpc,
				vbus_mv, NULL);
		}
		cnt++;
	} while (ret != TCP_DPM_RET_SUCCESS && cnt < PCA_PPS_CMD_RETRY_COUNT);

	if (ret != TCP_DPM_RET_SUCCESS)
		mca_log_err("%s:fail(%d)\n", __func__, ret);

	return ret > 0 ? -ret : ret;
}

static int ex_pd_protocol_select_pd_pdo(unsigned int vbus_mv, void *data)
{
	int ret = 0;
	unsigned int default_ibus_ma = 2000;
	struct ext_pd_protocol *info = g_ext_pd_protocol;

	ret = tcpm_set_pd_charging_policy_and_request(info->tcpc, DPM_CHARGING_POLICY_MAX_POWER_LVIC, vbus_mv, default_ibus_ma, NULL);
	ret |= tcpm_dpm_pd_request(info->tcpc, vbus_mv, default_ibus_ma, NULL);
	if (ret)
		mca_log_err("%s: set fix pdo vol failed\n", __func__);
	else
		mca_log_info("%s: set fix pdo vol %d\n", __func__, vbus_mv);
	return ret;
}

static int ex_pd_protocol_set_has_dp(bool has_dp, void *data)
{
	struct ext_pd_protocol *info = (struct ext_pd_protocol *)data;

	if (!data)
		return -1;

	info->has_dp = has_dp;

	return 0;
}

static int ex_pd_protocol_get_has_dp(bool *has_dp, void *data)
{
	struct ext_pd_protocol *info = (struct ext_pd_protocol *)data;

	if (!data || !has_dp)
		return -1;

	*has_dp = info->has_dp;

	return 0;
}

static int ex_pd_protocol_reset_pps_stage(bool en, void *data)
{
	struct ext_pd_protocol *info;

	info = g_ext_pd_protocol;
	if (en) {
		info->pps_chg_stage = MCA_PD_APDO_START;
	}
	return 0;
}

static int ex_pd_protocol_set_otg_plugin_status(bool otg_plugin_status, void *data)
{
	struct ext_pd_protocol *info = (struct ext_pd_protocol *)data;
	if (!data)
		return -1;
	info->otg_plugin_status = otg_plugin_status;
	return 0;
}

static int ex_pd_protocol_get_otg_plugin_status(bool *otg_plugin_status,void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;
	info = g_ext_pd_protocol;
	if (info == NULL || otg_plugin_status == NULL)
		return -EINVAL;
	*otg_plugin_status = info->otg_plugin_status;
	return ret;
}

static enum hrtimer_restart otg_switch_timer_function(struct hrtimer *timer)
{
    struct ext_pd_protocol *info =
		container_of(timer, struct ext_pd_protocol, otg_switch_timer);

    info->otg_switch_timeout_flag = true;

    info->cc_toggle_status = 0;

    mca_event_block_notify(MCA_EVENT_TYPE_SUBPMIC_INFO, MCA_EVENT_CC_TOGGLE_CHANGE, &info->cc_toggle_status);

    mca_log_info("\n");

	return HRTIMER_NORESTART;
}

static void otg_switch_detect_workfunc(struct work_struct *work)
{
    bool status;
    struct ext_pd_protocol *info = container_of(work,
			struct ext_pd_protocol, otg_switch_work);

	mca_log_info("start\n");

	if (1 == info->cc_toggle_status)  {
	    mca_log_info("start timer\n");
	    hrtimer_init(&info->otg_switch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        info->otg_switch_timer.function = otg_switch_timer_function;
        hrtimer_start(&info->otg_switch_timer, ktime_set(10*60, 0), HRTIMER_MODE_REL);

        while(!info->otg_switch_timeout_flag) {
            mca_log_info("detect loop\n");
            // wait if device insert
        	while(!info->otg_switch_timeout_flag) {
        	    mca_log_info("wait if device insert\n");
        		platform_class_buckchg_ops_get_cid_status(MAIN_BUCK_CHARGER, &status);
        		if (status) {
        		    mca_log_info("device plug in\n");
        			hrtimer_cancel(&info->otg_switch_timer);
        			info->otg_device_plugin_flag = true;
        			mca_log_info("cancel timer\n");
        			break;
        		}

        		msleep(1000);
        	}
        	// wait if device plug out
        	while(info->otg_device_plugin_flag) {
        	    mca_log_info("device has plug in\n");
        		platform_class_buckchg_ops_get_cid_status(MAIN_BUCK_CHARGER, &status);
        		if (!status) {
        		    info->otg_device_plugin_flag = false;
        		    mca_log_info("device plug out\n");
        			hrtimer_start(&info->otg_switch_timer, ktime_set(10*60, 0), HRTIMER_MODE_REL);
        			mca_log_info("start timer\n");
        			break;
        		}

        		msleep(1000);
        	}
        }
        info->otg_switch_timeout_flag = false;

	} else if (0 == info->cc_toggle_status) {
	    mca_log_info("cancel timer\n");
	    hrtimer_cancel(&info->otg_switch_timer);
	}

	mca_log_info("end\n");
}

static int ex_pd_protocol_set_cc_toggle_status(bool cc_toggle_status, void *data)
{
	struct ext_pd_protocol *info = (struct ext_pd_protocol *)data;

	if (!data)
		return -1;

	info->cc_toggle_status = cc_toggle_status;

	mca_log_info("begin cc_toggle_status = %d\n", cc_toggle_status);

	if (1 == cc_toggle_status)  {
	    mca_log_info("start work\n");
	    schedule_work(&info->otg_switch_work);
	}

	mca_event_block_notify(MCA_EVENT_TYPE_SUBPMIC_INFO, MCA_EVENT_CC_TOGGLE_CHANGE, &cc_toggle_status);

	mca_log_info("end cc_toggle_status = %d\n", cc_toggle_status);

	return 0;
}

static int ex_pd_protocol_get_cc_status(bool *status, void *data)
{
	return platform_class_buckchg_ops_get_cid_status(MAIN_BUCK_CHARGER, status);
}

static int ex_pd_protocol_get_cc_toggle_status(bool *cc_toggle_status,void *data)
{
	int ret = 0;
	struct ext_pd_protocol *info;
	info = g_ext_pd_protocol;
	if (info == NULL || cc_toggle_status == NULL)
		return -EINVAL;
	*cc_toggle_status = info->cc_toggle_status;
	return ret;
}

static int ex_pd_protocol_get_usb_communication(bool *if_support, void *data)
{
	struct ext_pd_protocol *info;
	struct tcpm_power_cap cap;
	uint32_t val = 0;
	int ret = 0;

	info = g_ext_pd_protocol;
	if ((info == NULL) || (info->tcpc == NULL))
		return -EINVAL;

	ret = tcpm_inquire_pd_source_cap(info->tcpc, &cap);
	if (ret != TCPM_SUCCESS)
		return ret;

	val = cap.pdos[0];
	val &= BIT(26);

	if (val != 0)
		*if_support = true;
	else
		*if_support = false;
	return ret;
}

struct protocol_class_pd_ops g_ex_pd_ops = {
	.protocol_pd_set_pd_active = pd_set_pd_active,
	.protocol_pd_get_pd_active = pd_get_pd_active,
	.protocol_pd_set_pps_max_cur = pd_set_pps_max_curr,
	.protocol_pd_get_pps_max_cur = pd_get_pps_max_curr,
	.protocol_pd_set_pps_min_volt = pd_set_pps_min_volt,
	.protocol_pd_get_pps_min_volt = pd_get_pps_min_volt,
	.protocol_pd_set_pps_max_volt = pd_set_pps_max_volt,
	.protocol_pd_get_pps_max_volt = pd_get_pps_max_volt,
	.protocol_pd_set_pps_apdo_max = pd_set_pps_apdo_max,
	.protocol_pd_get_pps_apdo_max = pd_get_pps_apdo_max,
	.protocol_pd_set_typec_mode = pd_set_typec_mode,
	.protocol_pd_get_typec_mode = pd_get_typec_mode,
	.protocol_pd_get_typec_cc_orientation = pd_get_typec_cc_orientation,
	.protocol_pd_set_in_hard_reset = pd_set_pd_in_hard_reset,
	.protocol_pd_get_in_hard_reset = pd_get_pd_in_hard_reset,
	.protocol_pd_set_usb_suspend_supported = pd_set_usb_suspend_supported,
	.protocol_pd_get_usb_suspend_supported = pd_get_usb_suspend_supported,
	.protocol_pd_set_pd_typec_accessory_mode = pd_set_typec_accessory_mode,
	.protocol_pd_get_pd_typec_accessory_mode = pd_get_typec_accessory_mode,
	.protocol_pd_get_power_role = pd_get_power_role,
	.protocol_pd_get_current_state = pd_get_current_state,
	.protocol_pd_set_verify_process = pd_set_verify_process,
	.protocol_pd_get_verify_process = pd_get_verify_process,
	.protocol_pd_get_adapter_id = pd_get_adapter_id,
	.protocol_pd_get_adapter_svid = pd_get_adapter_svid,
	.protocol_pd_get_pdos = pd_get_pdos,
	.protocol_pd_request_vdm_cmd = pd_request_vdm_cmd,
	.protocol_pd_get_vdm_cmd = pd_get_vdm_cmd,
	.protocol_pd_set_pd_verifed = pd_set_usbpd_verifed,
	.protocol_pd_get_pd_verifed = pd_get_usbpd_verifed,
	.protocol_pd_get_pd_type = ex_pd_protocol_get_pd_type,
	.protocol_pd_pps_pdo_select = ex_pd_protocol_select_pps_pdo,
	.protocol_pd_fixed_pdo_set_vol = ex_pd_protocol_select_pd_pdo,
	.protocol_pd_set_has_dp = ex_pd_protocol_set_has_dp,
	.protocol_pd_get_has_dp = ex_pd_protocol_get_has_dp,
	.protocol_pd_set_otg_plugin_status = ex_pd_protocol_set_otg_plugin_status,
	.protocol_pd_get_otg_plugin_status = ex_pd_protocol_get_otg_plugin_status,
	.protocol_pd_set_cc_toggle = ex_pd_protocol_set_cc_toggle_status,
	.protocol_pd_get_cc_toggle = ex_pd_protocol_get_cc_toggle_status,
	.protocol_pd_reset_pps_stage = ex_pd_protocol_reset_pps_stage,
	.protocol_pd_get_cid_status = ex_pd_protocol_get_cc_status,
	.protocol_pd_get_usb_communication = ex_pd_protocol_get_usb_communication,
};

static int xm_pd_adapter_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct ext_pd_protocol *info = NULL;
	static int probe_cnt = 0;

	mca_log_err("%s probe_cnt = %d\n", __func__, ++probe_cnt);

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->dev = &pdev->dev;
	platform_set_drvdata(pdev, info);

	INIT_WORK(&info->otg_switch_work, otg_switch_detect_workfunc);

	info->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (!info->tcpc) {
		mca_log_info("%s: tcpc device not ready, defer\n", __func__);
		ret = -EPROBE_DEFER;
		msleep(100);
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_get_tcpc_dev;
	}

	info->pd_nb.notifier_call = ex_pd_tcp_notifier_call;
	ret = register_tcp_dev_notifier(info->tcpc, &info->pd_nb,
				TCP_NOTIFY_TYPE_USB | TCP_NOTIFY_TYPE_MISC | TCP_NOTIFY_TYPE_MODE);
	ret = protocol_class_pd_register_ops(TYPEC_PORT_0, &g_ex_pd_ops, info);
	if (ret) {
		mca_log_err("%s reg pd ops fail\n", __func__);
		return ret;
	}
	if (ret < 0) {
		mca_log_err("%s: register tcpc notifer fail\n", __func__);
		return -EINVAL;
	}

	info->xr_quirk_get_cc_orien_from_tcpm =
		of_property_present(pdev->dev.of_node, "mca,xr,quirk-get-cc-orien-from-tcpm");

	g_ext_pd_protocol = info;
out:
	platform_set_drvdata(pdev, info);
	mca_log_err("%s %s!!\n", __func__, ret == -EPROBE_DEFER ?
				"Over probe cnt max" : "OK");
	return 0;

err_get_tcpc_dev:
	return ret;
}

static int xm_pd_adapter_remove(struct platform_device *pdev)
{
	struct ext_pd_protocol *info = platform_get_drvdata(pdev);
	devm_kfree(&pdev->dev, info);

	return 0;
}

static const struct of_device_id xm_pd_adapter_of_match[] = {
	{.compatible = "mca,pd_adapter",},
	{},
};
MODULE_DEVICE_TABLE(of, xm_pd_adapter_of_match);

static struct platform_driver xm_pd_adapter_driver = {
	.probe = xm_pd_adapter_probe,
	.remove = xm_pd_adapter_remove,
	.driver = {
		   .name = "xm_pd_adapter",
		   .of_match_table = xm_pd_adapter_of_match,
	},
};

static int __init xm_pd_adapter_init(void)
{
	return platform_driver_register(&xm_pd_adapter_driver);
}
module_init(xm_pd_adapter_init);

static void __exit xm_pd_adapter_exit(void)
{
	platform_driver_unregister(&xm_pd_adapter_driver);
}
module_exit(xm_pd_adapter_exit);

MODULE_DESCRIPTION("external pd protocol");
MODULE_AUTHOR("yinshunan@xiaomi.com");
MODULE_LICENSE("GPL v2");

