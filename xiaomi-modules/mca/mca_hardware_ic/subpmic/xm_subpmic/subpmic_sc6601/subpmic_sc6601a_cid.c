// SPDX-License-Identifier: GPL-2.0
/*
 *subpmic_sc6601a_cid.c
 *
 * subpmic cid driver
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

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/atomic.h>
#include <linux/of.h>
#include <linux/gpio.h>

#include "inc/subpmic_irq.h"
#include "inc/subpmic.h"
#include "../../../../mca_protocol/protocol_pd/external_tcpc/inc/tcpci_typec.h"
#include <mca/common/mca_log.h>
#include <mca/platform/platform_sc6601a_cid_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include "../../../../mca_protocol/protocol_pd/external_tcpc/inc/tcpm.h"
#include <mca/common/mca_event.h>
#include <mca/common/mca_workqueue.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_subpmic_sc6601a_cid"
#endif
#define SUBPMIC_CID_VERSION             "0.0.1"
#define TYPEC_PORT_NAME                 "type_c_port0"

struct subpmic_cid_device {
	struct i2c_client *client;
	struct device *dev;
	struct regmap *rmap;

	struct work_struct cid_work;
	atomic_t cid_pending;
	bool pd_ready;
	// pd contrl
	struct tcpc_device *tcpc;
	bool cid_en;
	struct notifier_block cc_toggle_nb;
	bool otg_switch_en;
	bool otg_device_present;
	int cid_force_src;
};

#define SUBPMIC_CID_FLAG            BIT(7)

static bool sc6601_get_cid_status(bool *status, void *data)
{
	struct subpmic_cid_device *sc = (struct subpmic_cid_device *)data;
	*status = sc->cid_en;
	return sc->cid_en;
}

static struct platform_class_buckchg_sc6601a_cid_ops sc6601a_cid_ops = {
	.get_cid_status = sc6601_get_cid_status,
};

static void cid_detect_workfunc(struct work_struct *work)
{
	struct subpmic_cid_device *sc = container_of(work,
			struct subpmic_cid_device, cid_work);
	int ret = 0;
	u8 state = 0;
	int typec_role = 0;
	// must get tcpc device
	while(!sc->pd_ready) {
		sc->tcpc = tcpc_dev_get_by_name(TYPEC_PORT_NAME);
		if (sc->tcpc) {
			typec_role = tcpm_inquire_typec_role(sc->tcpc);

			if (typec_role > TYPEC_ROLE_UNKNOWN &&
				typec_role < TYPEC_ROLE_NR && sc->tcpc->pd_inited_flag) {
				break;
			}
		}

		msleep(200);
	}

	do {
		ret = regmap_bulk_read(sc->rmap, SUBPMIC_REG_HK_GEN_STATE, &state, 1);
		mca_log_info("%s:cid state = 0x%x\n", __func__, state);
		if (ret) {
			mca_log_err("%s:failed to read irq state\n", __func__);
			return;
		}
		if (state & SUBPMIC_CID_FLAG) {
			mca_log_info("%s:cid detect plug-in\n", __func__);
			if (sc->pd_ready) {
				if (sc->cid_force_src)
					tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_DRP, true);
				else
					tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_TRY_SNK, true);
			} else {
				tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_TRY_SNK, false);
			}
			sc->cid_en = true;
		} else {
			mca_log_info("%s:cid detect plug-out\n", __func__);
			tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_SNK, true);
			sc->cid_en = false;
		}
		atomic_dec_if_positive(&sc->cid_pending);

		sc->pd_ready = true;

		if (!sc->cid_en) {
		    mca_log_info("sc->otg_switch_en = %d\n", sc->otg_switch_en);
    		if (0 == sc->otg_switch_en) { // otg switch close
    			tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_SNK, true);
    		} else if (1 == sc->otg_switch_en) { // otg switch open
    			tcpm_typec_change_role_postpone(sc->tcpc, TYPEC_ROLE_TRY_SNK, true);
    		}

    		mca_log_info("sc->otg_device_present = %d\n", sc->otg_device_present);
            if (1 == sc->otg_device_present) { // otg device in
                sc->cid_en = true;
    		}/* else if (0 == sc->otg_device_present) { // otg device out
    			sc->cid_en = false;
    		}*/
    	}
	} while(atomic_read(&sc->cid_pending));
}

static irqreturn_t subpmic_cid_alert_handler(int irq, void *data)
{
	struct subpmic_cid_device *sc = data;
	atomic_inc(&sc->cid_pending);
	mca_queue_work(&sc->cid_work);
	return IRQ_HANDLED;
}

static int subpmic_set_cc_toggle_cb(struct notifier_block *nb,
			unsigned long event, void *data)
{
	struct subpmic_cid_device *sc =
		container_of(nb, struct subpmic_cid_device, cc_toggle_nb);

	if (!data)
		return NOTIFY_OK;

	switch (event) {
	case MCA_EVENT_CC_TOGGLE_CHANGE:
		//TO DO
		sc->otg_switch_en = *((bool *)data);
		mca_log_err("MCA_EVENT_CC_TOGGLE_CHANGE otg_switch_en = %d\n", sc->otg_switch_en);
		subpmic_cid_alert_handler(0, sc);
		break;
	case MCA_EVENT_OTG_DEVICE_PRESENT:
		//TO DO
		sc->otg_device_present = *((bool *)data);
		mca_log_err("MCA_EVENT_OTG_DEVICE_PRESENT otg_device_present = %d\n", sc->otg_device_present);
		subpmic_cid_alert_handler(0, sc);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int subpmic_cid_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct subpmic_cid_device *sc;
	struct device_node *np = dev->of_node;
	int ret;

	sc = devm_kzalloc(dev, sizeof(*sc), GFP_KERNEL);
	if (!sc)
		return -ENOMEM;
	sc->rmap = dev_get_regmap(dev->parent, NULL);
	if (!sc->rmap) {
		mca_log_err("%s:failed to get regmap\n", __func__);
		return -ENODEV;
	}
	sc->dev = dev;
	platform_set_drvdata(pdev, sc);

	ret = of_property_read_u32(np, "cid-force-source",
		&sc->cid_force_src);
	if (ret) {
		mca_log_err("%s:failed to get cid-force-source\n", __func__);
		sc->cid_force_src = 0;
	}

	INIT_WORK(&sc->cid_work, cid_detect_workfunc);
	atomic_set(&sc->cid_pending, 0);

	sc->pd_ready = false;

	ret = platform_get_irq_byname(to_platform_device(sc->dev), "CID");
	if (ret < 0) {
		mca_log_err("%s:failed to get irq CID\n", __func__);
		return ret;
	}

	ret = devm_request_threaded_irq(sc->dev, ret, NULL,
			subpmic_cid_alert_handler, IRQF_ONESHOT,
			dev_name(sc->dev), sc);
	if (ret < 0) {
		mca_log_err("%s:failed to request irq CID\n", __func__);
		return ret;
	}

	sc->cc_toggle_nb.notifier_call = subpmic_set_cc_toggle_cb;
	ret = mca_event_block_notify_register(MCA_EVENT_TYPE_SUBPMIC_INFO,
		&sc->cc_toggle_nb);
	if (ret)
		mca_log_err("register cp info failed\n");

	platform_class_buckchg_ops_cid_register(MAIN_BUCK_CHARGER, sc, &sc6601a_cid_ops);
	mca_queue_work(&sc->cid_work);
	device_init_wakeup(sc->dev, true);
	mca_log_info("%s:successful\n", __func__);
	return 0;
}

static int subpmic_cid_remove(struct platform_device *pdev)
{
	return 0;
}

static void subpmic_cid_shutdown(struct platform_device *pdev)
{

}

static int subpmic_cid_suspend(struct device *dev)
{
	/* struct i2c_client *i2c = to_i2c_client(dev); */
	/* struct subpmic_cid_device *cid_dev = i2c_get_clientdata(i2c); */

	return 0;
}

static int subpmic_cid_resume(struct device *dev)
{
	struct i2c_client *i2c = to_i2c_client(dev);
	struct subpmic_cid_device *cid_dev = i2c_get_clientdata(i2c);

	/* hold wake lock for about 200ms for resume */
	pm_wakeup_dev_event(cid_dev->dev, 200, true);
	subpmic_cid_alert_handler(0, cid_dev);

	return 0;
}

static const struct dev_pm_ops subpmic_cid_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(subpmic_cid_suspend, subpmic_cid_resume)
};

static const struct of_device_id subpmic_cid_of_match[] = {
	{.compatible = "xm_subpmic_sc6601a,subpmic_cid",},
	{},
};

static struct platform_driver subpmic_cid_driver = {
	.driver = {
		.name = "subpmic_cid",
		.of_match_table = of_match_ptr(subpmic_cid_of_match),
		.pm = &subpmic_cid_pm_ops,
	},
	.probe = subpmic_cid_probe,
	.remove = subpmic_cid_remove,
	.shutdown = subpmic_cid_shutdown,
};

module_platform_driver(subpmic_cid_driver);

MODULE_AUTHOR("tianye9@xiaomi.com>");
MODULE_DESCRIPTION("sc6601A CID driver");
MODULE_LICENSE("GPL v2");
