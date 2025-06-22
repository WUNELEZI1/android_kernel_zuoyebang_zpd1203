// SPDX-License-Identifier: GPL-2.0
/*
 *protocol_pd_manager.c
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

#include <linux/of.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb/typec.h>
#include <linux/extcon-provider.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include "inc/tcpci_typec.h"

#include <mca/platform/platform_buckchg_class.h>
#include <mca/protocol/protocol_class.h>
#include <mca/protocol/protocol_pd_class.h>
#include <mca/common/mca_charge_interface.h>
#include <soc/xring/xr_usbdp_event.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/hardware/mca_xring_usb.h>
#include <mca/strategy/strategy_class.h>
#include <mca/common/mca_event.h>
#include <mca/protocol/protocol_pd_class.h>
#include "inc/tcpm.h"
#include <mca/common/mca_log.h>

#define PROBE_CNT_MAX			50

/* 10ms * 100 = 1000ms = 1s */
#define USB_TYPE_POLLING_INTERVAL	20
#define USB_TYPE_POLLING_CNT_MAX	200

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "sc6601a_pd_protocal_manager"
#endif

enum dr {
	DR_IDLE,
	DR_DEVICE,
	DR_HOST,
	DR_DEVICE_TO_HOST,
	DR_HOST_TO_DEVICE,
	DR_MAX,
};

static char *dr_names[DR_MAX] = {
	"Idle", "Device", "Host", "Device to Host", "Host to Device",
};

struct protocol_pd_manager {
	struct device *dev;
	struct extcon_dev *extcon;
	bool shutdown_flag;
	bool quirk_pr_limit;
	struct delayed_work usb_dwork;
	struct tcpc_device *tcpc;
	struct notifier_block pd_nb;
	enum dr usb_dr;
	int usb_type_polling_cnt;
	int sink_mv_pd;
	int sink_ma_pd;
	uint8_t pd_connect_state;

	struct typec_capability typec_caps;
	struct typec_port *typec_port;
	struct typec_partner *partner;
	struct typec_partner_desc partner_desc;
	struct usb_pd_identity partner_identity;
	struct mca_protocol_class *protocol_core;
};

#define POWER_SUPPLY_TYPE_USB_FLOAT	QTI_POWER_SUPPLY_TYPE_USB_FLOAT
#define POWER_SUPPLY_PD_INACTIVE	QTI_POWER_SUPPLY_PD_INACTIVE
#define POWER_SUPPLY_PD_ACTIVE		QTI_POWER_SUPPLY_PD_ACTIVE
#define POWER_SUPPLY_PD_PPS_ACTIVE	QTI_POWER_SUPPLY_PD_PPS_ACTIVE
#define POWER_SUPPLY_PD_DUMMY_ACTIVE	(POWER_SUPPLY_PD_PPS_ACTIVE + 10)

static const unsigned int rpm_extcon_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_NONE,
};

static int extcon_init(struct protocol_pd_manager *ppm)
{
	int ret = 0;

	/*
	 * associate extcon with the dev as it could have a DT
	 * node which will be useful for extcon_get_edev_by_phandle()
	 */
	ppm->extcon = devm_extcon_dev_allocate(ppm->dev, rpm_extcon_cable);
	if (IS_ERR(ppm->extcon)) {
		ret = PTR_ERR(ppm->extcon);
		mca_log_err("%s extcon dev alloc fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	ret = devm_extcon_dev_register(ppm->dev, ppm->extcon);
	if (ret) {
		mca_log_err("%s extcon dev reg fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	/* Support reporting polarity and speed via properties */
	extcon_set_property_capability(ppm->extcon, EXTCON_USB,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(ppm->extcon, EXTCON_USB,
				       EXTCON_PROP_USB_SS);
	extcon_set_property_capability(ppm->extcon, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(ppm->extcon, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_SS);
out:
	return ret;
}

static inline void stop_usb_host(struct protocol_pd_manager *ppm)
{
	extcon_set_state_sync(ppm->extcon, EXTCON_USB_HOST, false);
}

static inline void start_usb_host(struct protocol_pd_manager *ppm)
{
	union extcon_property_value val = {.intval = 0};

	val.intval = tcpm_inquire_cc_polarity(ppm->tcpc);
	extcon_set_property(ppm->extcon, EXTCON_USB_HOST,
			    EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = 1;
	extcon_set_property(ppm->extcon, EXTCON_USB_HOST,
			    EXTCON_PROP_USB_SS, val);

	extcon_set_state_sync(ppm->extcon, EXTCON_USB_HOST, true);
}

static inline void stop_usb_peripheral(struct protocol_pd_manager *ppm)
{
	extcon_set_state_sync(ppm->extcon, EXTCON_USB, false);
}

static inline void start_usb_peripheral(struct protocol_pd_manager *ppm)
{
	union extcon_property_value val = {.intval = 0};

	val.intval = tcpm_inquire_cc_polarity(ppm->tcpc);
	extcon_set_property(ppm->extcon, EXTCON_USB,
			    EXTCON_PROP_USB_TYPEC_POLARITY, val);

	val.intval = 1;
	extcon_set_property(ppm->extcon, EXTCON_USB, EXTCON_PROP_USB_SS, val);
	extcon_set_state_sync(ppm->extcon, EXTCON_USB, true);
}

static void usb_dwork_handler(struct work_struct *work)
{
	int ret = 0;
	int pd_active = 0;
	struct delayed_work *dwork = to_delayed_work(work);
	struct protocol_pd_manager *ppm =
		container_of(dwork, struct protocol_pd_manager, usb_dwork);
	enum dr usb_dr = ppm->usb_dr;
	union power_supply_propval val = {.intval = 0};

	if (usb_dr < DR_IDLE || usb_dr >= DR_MAX) {
		mca_log_err("%s invalid usb_dr = %d\n",
				   __func__, usb_dr);
		return;
	}

	if (likely(!ppm->quirk_pr_limit))
		mca_log_info("%s %s\n", __func__, dr_names[usb_dr]);
	else
		dev_info_ratelimited(ppm->dev, "%s %s\n", __func__, dr_names[usb_dr]);

	switch (usb_dr) {
	case DR_IDLE:
	case DR_MAX:
		stop_usb_peripheral(ppm);
		stop_usb_host(ppm);
		break;
	case DR_DEVICE:
		ret = protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
		if (pd_active) {
			if (pd_active == QTI_POWER_SUPPLY_PD_ACTIVE)
				val.intval = POWER_SUPPLY_USB_TYPE_PD;
			else if (pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE)
				val.intval = POWER_SUPPLY_USB_TYPE_PD_PPS;
		} else {
			//ret = mca_business_read_iio_prop(g_business_iio, MCA_CHARGER,
			//		MCA_CHARGER_USB_TYPE, &val.intval);
			ret = protocol_class_get_adapter_type(ADAPTER_PROTOCOL_BC12, &val.intval);
		}

		if (likely(!ppm->quirk_pr_limit)) {
			mca_log_info("%s polling_cnt = %d, ret = %d type = %d\n",
				 __func__, ++ppm->usb_type_polling_cnt, ret,
				 val.intval);
		} else {
			dev_info_ratelimited(
				ppm->dev,
				"%s polling_cnt = %d, ret = %d type = %d\n",
				__func__, ++ppm->usb_type_polling_cnt, ret,
				val.intval);
		}

		if (ret < 0 || val.intval == XM_CHARGER_TYPE_UNKNOW) {
			if (ppm->usb_type_polling_cnt <
			    USB_TYPE_POLLING_CNT_MAX) {
				schedule_delayed_work(&ppm->usb_dwork,
						msecs_to_jiffies(
						USB_TYPE_POLLING_INTERVAL));
				break;
			}
		} else if (val.intval != XM_CHARGER_TYPE_SDP  &&
			   val.intval != XM_CHARGER_TYPE_CDP  &&
			   val.intval != POWER_SUPPLY_USB_TYPE_PD)
			break;
		fallthrough;
	case DR_HOST_TO_DEVICE:
		stop_usb_host(ppm);
		start_usb_peripheral(ppm);
		break;
	case DR_HOST:
	case DR_DEVICE_TO_HOST:
		stop_usb_peripheral(ppm);
		start_usb_host(ppm);
		break;
	}
}

static void pd_sink_set_vol_and_cur(struct protocol_pd_manager *ppm,
				    int mv, int ma, uint8_t type)
{
	const int micro_5v = 5000000;
	unsigned long sel = 0;
	union power_supply_propval pd_active = {.intval = 0};
	u32 val = 0;

	ppm->sink_mv_pd = mv;
	ppm->sink_ma_pd = ma;

	if (ppm->pd_connect_state == PD_CONNECT_PE_READY_SNK_APDO)
		pd_active.intval = QTI_POWER_SUPPLY_PD_PPS_ACTIVE;
	else
		pd_active.intval = QTI_POWER_SUPPLY_PD_ACTIVE;
	protocol_class_pd_set_pd_active(TYPEC_PORT_0, pd_active.intval);
    mca_log_err("%s:set pd_active = %d\n", __func__, pd_active.intval);

	switch (type) {
	case TCP_VBUS_CTRL_PD_HRESET:
	case TCP_VBUS_CTRL_PD_PR_SWAP:
	case TCP_VBUS_CTRL_PD_REQUEST:
		set_bit(0, &sel);
		set_bit(1, &sel);
		val = mv * 1000;
		break;
	case TCP_VBUS_CTRL_PD_STANDBY_UP:
		set_bit(1, &sel);
		val = mv * 1000;
		break;
	case TCP_VBUS_CTRL_PD_STANDBY_DOWN:
		set_bit(0, &sel);
		val = mv * 1000;
		break;
	default:
		break;
	}
	if (val < micro_5v)
		val = micro_5v;

	if (test_bit(0, &sel))
		protocol_class_pd_set_pps_min_volt(TYPEC_PORT_0, val);
	if (test_bit(1, &sel))
		protocol_class_pd_set_pps_max_volt(TYPEC_PORT_0, val);

	val = ma * 1000;
	protocol_class_pd_set_pps_max_cur(TYPEC_PORT_0, val);
}

static int pd_tcp_notifier_call(struct notifier_block *nb,
				unsigned long event, void *data)
{
	int ret = 0;
	struct tcp_notify *noti = data;
	struct protocol_pd_manager *ppm =
		container_of(nb, struct protocol_pd_manager, pd_nb);
	uint8_t old_state = TYPEC_UNATTACHED, new_state = TYPEC_UNATTACHED;
	enum typec_pwr_opmode opmode = TYPEC_PWR_MODE_USB;
	uint32_t partner_vdos[VDO_MAX_NR];
	union power_supply_propval val = {.intval = 0};
	bool otg_present;

	switch (event) {
	case TCP_NOTIFY_SINK_VBUS:
		mca_log_info("%s sink vbus %dmV %dmA type(0x%02X)\n",
					 __func__, noti->vbus_state.mv,
					 noti->vbus_state.ma, noti->vbus_state.type);

		if (noti->vbus_state.type & TCP_VBUS_CTRL_PD_DETECT)
			pd_sink_set_vol_and_cur(ppm, noti->vbus_state.mv,
					noti->vbus_state.ma,
					noti->vbus_state.type);
		break;
	case TCP_NOTIFY_SOURCE_VBUS:
		mca_log_info("%s source vbus %dmV\n",
				    __func__, noti->vbus_state.mv);
		/**** todo***/
		if (noti->vbus_state.mv) {
			mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_OTG_CONNECT, NULL);
			platform_class_buckchg_set_otg_en(MAIN_BUCK_CHARGER, true, noti->vbus_state.mv);
			protocol_class_pd_set_otg_plugin(TYPEC_PORT_0,true);
			val.intval = 1;
		} else {
			platform_class_buckchg_set_otg_en(MAIN_BUCK_CHARGER, false, noti->vbus_state.mv);
			protocol_class_pd_set_otg_plugin(TYPEC_PORT_0,false);
			mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_OTG_DISCONNECT, NULL);
			val.intval = 0;
		}
		//mca_business_write_iio_prop(g_business_iio, MCA_CHARGER,
		//		MCA_CHARGER_OTG_ENABLE, val.intval);
		//if (strategy_class_noti) {
		//	mutex_lock(&strategy_class_noti->notify_lock);
		//	strategy_class_noti->charger_msg.otg_enable = val.intval;
		//	strategy_class_noti->charger_msg.msg_type = business_MSG_MAINCHG_OTG_ENABLE;
		//	business_notifier_call_chain(business_EVENT_MAINCHG, strategy_class_noti);
		//	mutex_unlock(&strategy_class_noti->notify_lock);
		//}
		/**** todo ****/
		/* enable/disable OTG power output */
		break;
	case TCP_NOTIFY_TYPEC_STATE:
		old_state = noti->typec_state.old_state;
		new_state = noti->typec_state.new_state;
		if (old_state == TYPEC_UNATTACHED &&
		    (new_state == TYPEC_ATTACHED_SNK ||
		     new_state == TYPEC_ATTACHED_NORP_SRC ||
		     new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
		     new_state == TYPEC_ATTACHED_DBGACC_SNK)) {
			mca_log_info("%s Charger plug in, polarity = %d\n",
				 __func__, noti->typec_state.polarity);
			/*
			 * start charger type detection,
			 * and enable device connection
			 */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_DEVICE;
			ppm->usb_type_polling_cnt = 0;
			schedule_delayed_work(&ppm->usb_dwork,
					      msecs_to_jiffies(
					      USB_TYPE_POLLING_INTERVAL));
			typec_set_data_role(ppm->typec_port, TYPEC_DEVICE);
			typec_set_pwr_role(ppm->typec_port, TYPEC_SINK);
			if ((noti->typec_state.rp_level >= TYPEC_CC_VOLT_SNK_DFT) &&
				(noti->typec_state.rp_level <= (TYPEC_CC_VOLT_SNK_3_0 + 1)))
				typec_set_pwr_opmode(ppm->typec_port,
					noti->typec_state.rp_level - TYPEC_CC_VOLT_SNK_DFT);
			else
				typec_set_pwr_opmode(ppm->typec_port, TYPEC_PWR_MODE_USB);

			typec_set_vconn_role(ppm->typec_port, TYPEC_SINK);
		} else if ((old_state == TYPEC_ATTACHED_SNK ||
			    old_state == TYPEC_ATTACHED_NORP_SRC ||
			    old_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			    old_state == TYPEC_ATTACHED_DBGACC_SNK) &&
			    new_state == TYPEC_UNATTACHED) {
			mca_log_info("%s Charger plug out\n", __func__);
			/*
			 * report charger plug-out,
			 * and disable device connection
			 */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_IDLE;
			schedule_delayed_work(&ppm->usb_dwork, 0);
		} else if (old_state == TYPEC_UNATTACHED &&
			   (new_state == TYPEC_ATTACHED_SRC ||
			    new_state == TYPEC_ATTACHED_DEBUG)) {
			mca_log_info("%s OTG plug in, polarity = %d\n",
				 __func__, noti->typec_state.polarity);
			otg_present = true;
			mca_event_block_notify(MCA_EVENT_TYPE_SUBPMIC_INFO, MCA_EVENT_OTG_DEVICE_PRESENT, &otg_present);
			/* enable host connection */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_HOST;
			schedule_delayed_work(&ppm->usb_dwork, 0);
			typec_set_data_role(ppm->typec_port, TYPEC_HOST);
			typec_set_pwr_role(ppm->typec_port, TYPEC_SOURCE);
			switch (noti->typec_state.local_rp_level) {
			case TYPEC_RP_3_0:
				opmode = TYPEC_PWR_MODE_3_0A;
				break;
			case TYPEC_RP_1_5:
				opmode = TYPEC_PWR_MODE_1_5A;
				break;
			case TYPEC_RP_DFT:
			default:
				opmode = TYPEC_PWR_MODE_USB;
				break;
			}
			typec_set_pwr_opmode(ppm->typec_port, opmode);
			typec_set_vconn_role(ppm->typec_port, TYPEC_SOURCE);
			mca_xring_usb_host_on(ppm->usb_dr);
		} else if ((old_state == TYPEC_ATTACHED_SRC ||
			    old_state == TYPEC_ATTACHED_DEBUG) &&
			    new_state == TYPEC_UNATTACHED) {
			mca_log_info("%s OTG plug out\n", __func__);
			otg_present = false;
			mca_event_block_notify(MCA_EVENT_TYPE_SUBPMIC_INFO, MCA_EVENT_OTG_DEVICE_PRESENT, &otg_present);
			/* disable host connection */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_IDLE;
			schedule_delayed_work(&ppm->usb_dwork, 0);
			mca_xring_usb_host_off(ppm->usb_dr);
			protocol_class_pd_set_has_dp(TYPEC_PORT_0, false);
		} else if (old_state == TYPEC_UNATTACHED &&
			   new_state == TYPEC_ATTACHED_AUDIO) {
			mca_log_info("%s Audio plug in\n", __func__);
			/* enable AudioAccessory connection */
		} else if (old_state == TYPEC_ATTACHED_AUDIO &&
			   new_state == TYPEC_UNATTACHED) {
			mca_log_info("%s Audio plug out\n", __func__);
			/* disable AudioAccessory connection */
		}

		if (new_state != TYPEC_UNATTACHED) {
			val.intval = noti->typec_state.polarity + 1;
		} else {
			val.intval = 0;
		}

		mca_log_info("%s USB plug. val.intval=%d\n", __func__, val.intval);

		if (new_state == TYPEC_ATTACHED_SRC) {
			val.intval = TYPEC_ATTACHED_SRC;
			mca_log_err("%s TYPEC_ATTACHED_SRC(%d)\n",__func__, val.intval);
		} else if (new_state == TYPEC_UNATTACHED) {
			val.intval = TYPEC_UNATTACHED;
			mca_log_err("%s TYPEC_UNATTACHED(%d)\n",__func__, val.intval);
		} else if (new_state == TYPEC_ATTACHED_SNK) {
			val.intval = TYPEC_ATTACHED_SNK;
			mca_log_err("%s TYPEC_ATTACHED_SNK(%d)\n",__func__, val.intval);
		} else if (new_state == TYPEC_ATTACHED_AUDIO) {
			val.intval = TYPEC_ATTACHED_AUDIO;
			mca_log_err("%s TYPEC_ATTACHED_AUDIO(%d)\n",__func__, val.intval);
		}

		if (new_state == TYPEC_UNATTACHED) {
			val.intval = 0;
			protocol_class_pd_set_usb_suspend_supported(TYPEC_PORT_0, val.intval);
			protocol_class_pd_set_pd_active(TYPEC_PORT_0, val.intval);
			protocol_class_pd_set_typec_mode(TYPEC_PORT_0, val.intval);
			protocol_class_pd_set_pd_typec_accessory_mode(TYPEC_PORT_0, val.intval);
			typec_unregister_partner(ppm->partner);
			ppm->partner = NULL;
			if (ppm->typec_caps.prefer_role == TYPEC_SOURCE) {
				typec_set_data_role(ppm->typec_port,
						    TYPEC_HOST);
				typec_set_pwr_role(ppm->typec_port,
						   TYPEC_SOURCE);
				typec_set_pwr_opmode(ppm->typec_port,
						     TYPEC_PWR_MODE_USB);
				typec_set_vconn_role(ppm->typec_port,
						     TYPEC_SOURCE);
			} else {
				typec_set_data_role(ppm->typec_port,
						    TYPEC_DEVICE);
				typec_set_pwr_role(ppm->typec_port,
						   TYPEC_SINK);
				typec_set_pwr_opmode(ppm->typec_port,
						     TYPEC_PWR_MODE_USB);
				typec_set_vconn_role(ppm->typec_port,
						     TYPEC_SINK);
			}
		} else if (!ppm->partner) {
			memset(&ppm->partner_identity, 0,
			       sizeof(ppm->partner_identity));
			ppm->partner_desc.usb_pd = false;
			switch (new_state) {
			case TYPEC_ATTACHED_AUDIO:
				ppm->partner_desc.accessory =
					TYPEC_ACCESSORY_AUDIO;
				break;
			case TYPEC_ATTACHED_DEBUG:
			case TYPEC_ATTACHED_DBGACC_SNK:
			case TYPEC_ATTACHED_CUSTOM_SRC:
				ppm->partner_desc.accessory =
					TYPEC_ACCESSORY_DEBUG;
				break;
			default:
				ppm->partner_desc.accessory =
					TYPEC_ACCESSORY_NONE;
				break;
			}
			protocol_class_pd_set_pd_typec_accessory_mode(TYPEC_PORT_0, ppm->partner_desc.accessory);
			mca_log_err("%s TYPEC_ACCESSORY_MODE(%d)\n",__func__,
						ppm->partner_desc.accessory);

			ppm->partner = typec_register_partner(ppm->typec_port,
					&ppm->partner_desc);
			if (IS_ERR(ppm->partner)) {
				ret = PTR_ERR(ppm->partner);
				mca_log_err("%s typec register partner fail(%d)\n",
					   __func__, ret);
			}
		}

		if (new_state == TYPEC_ATTACHED_SNK) {
			switch (noti->typec_state.rp_level) {
				/* SNK_RP_3P0 */
				case TYPEC_CC_VOLT_SNK_3_0:
					break;
				/* SNK_RP_1P5 */
				case TYPEC_CC_VOLT_SNK_1_5:
					break;
				/* SNK_RP_STD */
				case TYPEC_CC_VOLT_SNK_DFT:
				default:
					break;
			}
		} else if (new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			   new_state == TYPEC_ATTACHED_DBGACC_SNK) {
			switch (noti->typec_state.rp_level) {
				/* DAM_3000 */
				case TYPEC_CC_VOLT_SNK_3_0:
					break;
				/* DAM_1500 */
				case TYPEC_CC_VOLT_SNK_1_5:
					break;
				/* DAM_500 */
				case TYPEC_CC_VOLT_SNK_DFT:
				default:
					break;
			}
		} else if (new_state == TYPEC_ATTACHED_NORP_SRC) {
			/* Both CCs are open */
		}
		break;
	case TCP_NOTIFY_PR_SWAP:
		mca_log_info("%s power role swap, new role = %d\n",
				    __func__, noti->swap_state.new_role);
		if (noti->swap_state.new_role == PD_ROLE_SINK) {
			mca_log_info("%s swap power role to sink\n",
					    __func__);
			/*
			 * report charger plug-in without charger type detection
			 * to not interfering with USB2.0 communication
			 */

			/* toggle chg->pd_active to clean up the effect of
			 * smblib_uusb_removal() */
			val.intval = POWER_SUPPLY_PD_DUMMY_ACTIVE;
			protocol_class_pd_set_pd_active(TYPEC_PORT_0, val.intval);
			typec_set_pwr_role(ppm->typec_port, TYPEC_SINK);
		} else if (noti->swap_state.new_role == PD_ROLE_SOURCE) {
			mca_log_info("%s swap power role to source\n",
					    __func__);
			/* report charger plug-out */

			typec_set_pwr_role(ppm->typec_port, TYPEC_SOURCE);
		}
		break;
	case TCP_NOTIFY_DR_SWAP:
		mca_log_info("%s data role swap, new role = %d\n",
				    __func__, noti->swap_state.new_role);
		if (noti->swap_state.new_role == PD_ROLE_UFP) {
			mca_log_info("%s swap data role to device\n",
					    __func__);
			/*
			 * disable host connection,
			 * and enable device connection
			 */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_HOST_TO_DEVICE;
			schedule_delayed_work(&ppm->usb_dwork, 0);
			typec_set_data_role(ppm->typec_port, TYPEC_DEVICE);
			mca_xr_usb_data_role_to_device();
		} else if (noti->swap_state.new_role == PD_ROLE_DFP) {
			mca_log_info("%s swap data role to host\n",
					    __func__);
			/*
			 * disable device connection,
			 * and enable host connection
			 */
			cancel_delayed_work_sync(&ppm->usb_dwork);
			ppm->usb_dr = DR_DEVICE_TO_HOST;
			schedule_delayed_work(&ppm->usb_dwork, 0);
			typec_set_data_role(ppm->typec_port, TYPEC_HOST);
			mca_xr_usb_data_role_to_host();
		}
		break;
	case TCP_NOTIFY_VCONN_SWAP:
		mca_log_info("%s vconn role swap, new role = %d\n",
				    __func__, noti->swap_state.new_role);
		if (noti->swap_state.new_role) {
			mca_log_info("%s swap vconn role to on\n",
					    __func__);
			typec_set_vconn_role(ppm->typec_port, TYPEC_SOURCE);
		} else {
			mca_log_info("%s swap vconn role to off\n",
					    __func__);
			typec_set_vconn_role(ppm->typec_port, TYPEC_SINK);
		}
		break;
	case TCP_NOTIFY_EXT_DISCHARGE:
		mca_log_info("%s ext discharge = %d\n",
				    __func__, noti->en_state.en);
		/* enable/disable VBUS discharge */
		break;
	case TCP_NOTIFY_PD_STATE:
		ppm->pd_connect_state = noti->pd_state.connected;
		mca_log_info("%s pd state = %d\n",
					__func__, ppm->pd_connect_state);
		switch (ppm->pd_connect_state) {
		case PD_CONNECT_NONE:
			break;
		case PD_CONNECT_HARD_RESET:
			break;
		case PD_CONNECT_PE_READY_SNK_APDO:
			pd_sink_set_vol_and_cur(ppm, ppm->sink_mv_pd,
					ppm->sink_ma_pd,
					TCP_VBUS_CTRL_PD_STANDBY);
			fallthrough;
		case PD_CONNECT_PE_READY_SNK:
		case PD_CONNECT_PE_READY_SNK_PD30:
			ret = tcpm_inquire_dpm_flags(ppm->tcpc);
			val.intval = ret & DPM_FLAGS_PARTNER_USB_SUSPEND ? 1 : 0;
			protocol_class_pd_set_usb_suspend_supported(TYPEC_PORT_0, val.intval);
			fallthrough;
		case PD_CONNECT_PE_READY_SRC:
		case PD_CONNECT_PE_READY_SRC_PD30:
			typec_set_pwr_opmode(ppm->typec_port,
					     TYPEC_PWR_MODE_PD);
			if (!ppm->partner)
				break;
			ret = tcpm_inquire_pd_partner_inform(ppm->tcpc,
							     partner_vdos);
			if (ret != TCPM_SUCCESS)
				break;
			ppm->partner_identity.id_header = partner_vdos[0];
			ppm->partner_identity.cert_stat = partner_vdos[1];
			ppm->partner_identity.product = partner_vdos[2];
			typec_partner_set_identity(ppm->partner);
			break;
		};
		break;
	case TCP_NOTIFY_HARD_RESET_STATE:
		switch (noti->hreset_state.state) {
		case TCP_HRESET_SIGNAL_SEND:
		case TCP_HRESET_SIGNAL_RECV:
			val.intval = 1;
			break;
		default:
			val.intval = 0;
			break;
		}
		protocol_class_pd_set_pd_in_hard_reset(TYPEC_PORT_0, val.intval);
		break;
	case TCP_NOTIFY_AMA_DP_STATE:
		mca_log_info("%s dp_state 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\n",
			__func__, noti->ama_dp_state.sel_config,
			noti->ama_dp_state.signal,
			noti->ama_dp_state.pin_assignment,
			noti->ama_dp_state.polarity,
			noti->ama_dp_state.active);
		if (noti->ama_dp_state.active == 1) {
			unsigned int dp_mode = DP_MODE;

			protocol_class_pd_set_has_dp(TYPEC_PORT_0, true);

			if (noti->ama_dp_state.pin_assignment == MODE_DP_PIN_D)
				dp_mode = USBDP_MODE;

			mca_xr_usb_queue_host_on(noti->ama_dp_state.polarity,
											dp_mode, DP_HPD_EVENT_PLUG_OUT);
		}
		break;
	case TCP_NOTIFY_AMA_DP_ATTENTION:
		mca_log_info("%s dp_attention state = 0x%02x\n",
				    __func__, noti->ama_dp_attention.state);

		break;
	case TCP_NOTIFY_AMA_DP_HPD_STATE:
		{
			enum dp_hpd_event hpd_event = DP_HPD_EVENT_PLUG_OUT;

			mca_log_info("%s dp_hpd irq = 0x%02x state = 0x%02x\n",
				__func__, noti->ama_dp_hpd_state.irq, noti->ama_dp_hpd_state.state);

			if (noti->ama_dp_hpd_state.state)
				hpd_event = DP_HPD_EVENT_PLUG_IN;
			if (noti->ama_dp_hpd_state.irq)
				hpd_event = DP_HPD_EVENT_IRQ;

			mca_xr_usb_queue_dp_hpd(hpd_event);
		}
		break;
	case TCP_NOTIFY_RECV_SRC_CAP:
		if (ppm->pd_connect_state == PD_CONNECT_PE_READY_SNK_APDO)
			val.intval = QTI_POWER_SUPPLY_PD_PPS_ACTIVE;
		else
			val.intval = QTI_POWER_SUPPLY_PD_ACTIVE;
		protocol_class_pd_set_pd_active(TYPEC_PORT_0, val.intval);
		mca_event_block_notify(MCA_EVENT_TYPE_CHARGE_TYPE,
			MCA_EVENT_CHARGE_CAP_CHANGE, NULL);
		break;
	default:
		break;
	};
	return NOTIFY_OK;
}

static int tcpc_typec_try_role(struct typec_port *port, int role)
{
	struct protocol_pd_manager *ppm = typec_get_drvdata(port);
	uint8_t typec_role = TYPEC_ROLE_UNKNOWN;

	mca_log_info("%s role = %d\n", __func__, role);

	switch (role) {
	case TYPEC_NO_PREFERRED_ROLE:
		typec_role = TYPEC_ROLE_DRP;
		break;
	case TYPEC_SINK:
		typec_role = TYPEC_ROLE_TRY_SNK;
		break;
	case TYPEC_SOURCE:
		typec_role = TYPEC_ROLE_TRY_SRC;
		break;
	default:
		return 0;
	}

	return tcpm_typec_change_role_postpone(ppm->tcpc, typec_role, true);
}

static int tcpc_typec_dr_set(struct typec_port *port, enum typec_data_role role)
{
	struct protocol_pd_manager *ppm = typec_get_drvdata(port);
	int ret = 0;
	uint8_t data_role = tcpm_inquire_pd_data_role(ppm->tcpc);
	bool do_swap = false;

	mca_log_info("%s role = %d\n", __func__, role);

	if (role == TYPEC_HOST) {
		if (data_role == PD_ROLE_UFP) {
			do_swap = true;
			data_role = PD_ROLE_DFP;
		}
	} else if (role == TYPEC_DEVICE) {
		if (data_role == PD_ROLE_DFP) {
			do_swap = true;
			data_role = PD_ROLE_UFP;
		}
	} else {
		mca_log_err("%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_data_swap(ppm->tcpc, data_role, NULL);
		if (ret != TCPM_SUCCESS) {
			mca_log_err("%s data role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

static int tcpc_typec_pr_set(struct typec_port *port, enum typec_role role)
{
	struct protocol_pd_manager *ppm = typec_get_drvdata(port);
	int ret = 0;
	uint8_t power_role = tcpm_inquire_pd_power_role(ppm->tcpc);
	bool do_swap = false;

	mca_log_info("%s role = %d\n", __func__, role);

	if (role == TYPEC_SOURCE) {
		if (power_role == PD_ROLE_SINK) {
			do_swap = true;
			power_role = PD_ROLE_SOURCE;
		}
	} else if (role == TYPEC_SINK) {
		if (power_role == PD_ROLE_SOURCE) {
			do_swap = true;
			power_role = PD_ROLE_SINK;
		}
	} else {
		mca_log_err("%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_power_swap(ppm->tcpc, power_role, NULL);
		if (ret == TCPM_ERROR_NO_PD_CONNECTED)
			ret = tcpm_typec_role_swap(ppm->tcpc);
		if (ret != TCPM_SUCCESS) {
			mca_log_err("%s power role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

static int tcpc_typec_vconn_set(struct typec_port *port, enum typec_role role)
{
	struct protocol_pd_manager *ppm = typec_get_drvdata(port);
	int ret = 0;
	uint8_t vconn_role = tcpm_inquire_pd_vconn_role(ppm->tcpc);
	bool do_swap = false;

	mca_log_info("%s role = %d\n", __func__, role);

	if (role == TYPEC_SOURCE) {
		if (vconn_role == PD_ROLE_VCONN_OFF) {
			do_swap = true;
			vconn_role = PD_ROLE_VCONN_ON;
		}
	} else if (role == TYPEC_SINK) {
		if (vconn_role == PD_ROLE_VCONN_ON) {
			do_swap = true;
			vconn_role = PD_ROLE_VCONN_OFF;
		}
	} else {
		mca_log_err("%s invalid role\n", __func__);
		return -EINVAL;
	}

	if (do_swap) {
		ret = tcpm_dpm_pd_vconn_swap(ppm->tcpc, vconn_role, NULL);
		if (ret != TCPM_SUCCESS) {
			mca_log_err("%s vconn role swap fail(%d)\n",
					   __func__, ret);
			return -EPERM;
		}
	}

	return 0;
}

static int tcpc_typec_port_type_set(struct typec_port *port,
				    enum typec_port_type type)
{
	struct protocol_pd_manager *ppm = typec_get_drvdata(port);
	const struct typec_capability *cap = &ppm->typec_caps;
	bool as_sink = tcpc_typec_is_act_as_sink_role(ppm->tcpc);
	uint8_t typec_role = TYPEC_ROLE_UNKNOWN;

	mca_log_info("%s type = %d, as_sink = %d\n",
			    __func__, type, as_sink);

	switch (type) {
	case TYPEC_PORT_SNK:
		if (as_sink)
			return 0;
		break;
	case TYPEC_PORT_SRC:
		if (!as_sink)
			return 0;
		break;
	case TYPEC_PORT_DRP:
		if (cap->prefer_role == TYPEC_SOURCE)
			typec_role = TYPEC_ROLE_TRY_SRC;
		else if (cap->prefer_role == TYPEC_SINK)
			typec_role = TYPEC_ROLE_TRY_SNK;
		else
			typec_role = TYPEC_ROLE_DRP;
		return tcpm_typec_change_role(ppm->tcpc, typec_role);
	default:
		return 0;
	}

	return tcpm_typec_role_swap(ppm->tcpc);
}

const struct typec_operations tcpc_typec_ops = {
	.try_role = tcpc_typec_try_role,
	.dr_set = tcpc_typec_dr_set,
	.pr_set = tcpc_typec_pr_set,
	.vconn_set = tcpc_typec_vconn_set,
	.port_type_set = tcpc_typec_port_type_set,
};

static int typec_init(struct protocol_pd_manager *ppm)
{
	int ret = 0;

	ppm->typec_caps.type = TYPEC_PORT_DRP;
	ppm->typec_caps.data = TYPEC_PORT_DRD;
	ppm->typec_caps.revision = 0x0120;
	ppm->typec_caps.pd_revision = 0x0300;
	switch (ppm->tcpc->desc.role_def) {
	case TYPEC_ROLE_SRC:
	case TYPEC_ROLE_TRY_SRC:
		ppm->typec_caps.prefer_role = TYPEC_SOURCE;
		break;
	case TYPEC_ROLE_SNK:
	case TYPEC_ROLE_TRY_SNK:
		ppm->typec_caps.prefer_role = TYPEC_SINK;
		break;
	default:
		ppm->typec_caps.prefer_role = TYPEC_NO_PREFERRED_ROLE;
		break;
	}
	ppm->typec_caps.driver_data = ppm;
	ppm->typec_caps.ops = &tcpc_typec_ops;
	ppm->typec_port = typec_register_port(ppm->dev, &ppm->typec_caps);
	if (IS_ERR(ppm->typec_port)) {
		ret = PTR_ERR(ppm->typec_port);
		mca_log_err("%s typec register port fail(%d)\n",
				   __func__, ret);
		goto out;
	}

	ppm->partner_desc.identity = &ppm->partner_identity;
out:
	return ret;
}

#ifdef CONFIG_TCPC_NOTIFIER_LATE_SYNC
#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
static int fg_bat_notifier_call(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct pd_port *pd_port = container_of(nb, struct pd_port, fg_bat_nb);
	struct tcpc_device *tcpc = pd_port->tcpc;

	switch (event) {
	case EVENT_BATTERY_PLUG_OUT:
		mca_log_info("%s: fg battery absent\n", __func__);
		schedule_work(&pd_port->fg_bat_work);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */

static int __tcpc_class_complete_work(struct device *dev, void *data)
{
	struct tcpc_device *tcpc = dev_get_drvdata(dev);
#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
	struct notifier_block *fg_bat_nb = &tcpc->pd_port.fg_bat_nb;
	int ret = 0;
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */

	if (tcpc != NULL) {
		mca_log_info("%s = %s\n", __func__, dev_name(dev));
#if 1
		tcpc_device_irq_enable(tcpc);
#else
		schedule_delayed_work(&tcpc->init_work,
			msecs_to_jiffies(1000));
#endif

#ifdef CONFIG_RECV_BAT_ABSENT_NOTIFY
		fg_bat_nb->notifier_call = fg_bat_notifier_call;
		ret = register_battery_notifier(fg_bat_nb);
		if (ret < 0) {
			mca_log_err("%s: register bat notifier fail\n", __func__);
			return -EINVAL;
		}
#endif /* CONFIG_RECV_BAT_ABSENT_NOTIFY */
	}
	return 0;
}

static int tcpc_class_complete_init(void)
{
	if (!IS_ERR(tcpc_class)) {
		class_for_each_device(tcpc_class, NULL, NULL,
			__tcpc_class_complete_work);
	}
	return 0;
}
#endif /* CONFIG_TCPC_NOTIFIER_LATE_SYNC */

static int protocol_pd_manager_probe(struct platform_device *pdev)
{
	int ret = 0;
	static int probe_cnt = 0;
	struct protocol_pd_manager *ppm = NULL;

	mca_log_err("%s probe_cnt = %d\n", __func__, ++probe_cnt);

	if (probe_cnt <= 1) {
		mca_log_err("%s delay probe\n", __func__);
		msleep(100);
		ret = -EPROBE_DEFER;
		goto err_init_extcon;
	}

	ppm = devm_kzalloc(&pdev->dev, sizeof(*ppm), GFP_KERNEL);
	if (!ppm)
		return -ENOMEM;

	ppm->dev = &pdev->dev;
	ppm->quirk_pr_limit = of_property_present(pdev->dev.of_node, "xr,quirk-print-limit");

	ret = extcon_init(ppm);
	if (ret) {
		mca_log_err("%s init extcon fail(%d)\n", __func__, ret);
		ret = -EPROBE_DEFER;
		msleep(100);
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_init_extcon;
	}

	ppm->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (!ppm->tcpc) {
		mca_log_err("%s get tcpc dev fail\n", __func__);
		ret = -EPROBE_DEFER;
		msleep(100);
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_get_tcpc_dev;
	}

	INIT_DELAYED_WORK(&ppm->usb_dwork, usb_dwork_handler);
	ppm->usb_dr = DR_IDLE;
	ppm->usb_type_polling_cnt = 0;
	ppm->pd_connect_state = PD_CONNECT_NONE;
	ppm->shutdown_flag = false;

	ret = typec_init(ppm);
	if (ret < 0) {
		mca_log_err("%s init typec fail(%d)\n", __func__, ret);
		ret = -EPROBE_DEFER;
		msleep(100);
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_init_typec;
	}

	ppm->pd_nb.notifier_call = pd_tcp_notifier_call;
	ret = register_tcp_dev_notifier(ppm->tcpc, &ppm->pd_nb,
					TCP_NOTIFY_TYPE_ALL);
	if (ret < 0) {
		mca_log_err("%s register tcpc notifier fail(%d)\n",
				   __func__, ret);
		ret = -EPROBE_DEFER;
		msleep(100);
		if (probe_cnt >= PROBE_CNT_MAX)
			goto out;
		else
			goto err_reg_tcpc_notifier;
	}
#ifdef CONFIG_TCPC_NOTIFIER_LATE_SYNC
	tcpc_class_complete_init();
#endif /* CONFIG_TCPC_NOTIFIER_LATE_SYNC */
	typec_set_pwr_opmode(ppm->typec_port, TYPEC_PWR_MODE_USB);
	mca_log_err("%s: End!\n", __func__);

out:
	platform_set_drvdata(pdev, ppm);
	mca_log_err("%s %s!!\n", __func__, ret == -EPROBE_DEFER ?
			    "Over probe cnt max" : "OK");
	return 0;

err_reg_tcpc_notifier:
	typec_unregister_port(ppm->typec_port);
err_init_typec:
err_get_tcpc_dev:
err_init_extcon:
	return ret;
}

static int protocol_pd_manager_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct protocol_pd_manager *ppm = platform_get_drvdata(pdev);

	if (!ppm)
		return -EINVAL;

	ret = unregister_tcp_dev_notifier(ppm->tcpc, &ppm->pd_nb,
					  TCP_NOTIFY_TYPE_ALL);
	if (ret < 0)
		mca_log_err("%s unregister tcpc notifier fail(%d)\n",
				   __func__, ret);
	typec_unregister_port(ppm->typec_port);

	return ret;
}

static void protocol_pd_manager_shutdown(struct platform_device *pdev)
{
	struct protocol_pd_manager *ppm = platform_get_drvdata(pdev);

	mca_log_info("%s protocol_pd_manager_shutdown 11\n",
				   __func__);
	if (!ppm)
		return;

	ppm->shutdown_flag = true;

	return;
}

static const struct of_device_id protocol_pd_manager_of_match[] = {
	{ .compatible = "protocol,pd_manager" },
	{ }
};
MODULE_DEVICE_TABLE(of, protocol_pd_manager_of_match);

static struct platform_driver protocol_pd_manager_driver = {
	.driver = {
		.name = "protocol_pd_manager",
		.of_match_table = of_match_ptr(protocol_pd_manager_of_match),
	},
	.probe = protocol_pd_manager_probe,
	.remove = protocol_pd_manager_remove,
	.shutdown = protocol_pd_manager_shutdown,
};

static int __init protocol_pd_manager_init(void)
{
	return platform_driver_register(&protocol_pd_manager_driver);
}
late_initcall(protocol_pd_manager_init);

static void __exit protocol_pd_manager_exit(void)
{
	platform_driver_unregister(&protocol_pd_manager_driver);
}
module_exit(protocol_pd_manager_exit);

MODULE_DESCRIPTION("protocol pd manager");
MODULE_AUTHOR("getian@xiaomi.com");
MODULE_LICENSE("GPL v2");

