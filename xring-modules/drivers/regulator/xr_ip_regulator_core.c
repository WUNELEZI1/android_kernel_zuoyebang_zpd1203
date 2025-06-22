// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/of_address.h>
#include <linux/pm.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/vote_mng.h>
#include <linux/clk.h>
#include <soc/xring/flowctrl.h>
#include "xr_regulator_internal.h"
#include "regulator/internal.h"
#include <dt-bindings/xring/platform-specific/hwlock/ap_spin_lock_id.h>
#include <linux/hwspinlock.h>
#include <soc/xring/xr_hwspinlock.h>

#define XR_IP_REGULATOR_GPC_DEBUG

#define VOTE_MODE1 (1)
#define VOTE_MODE3 (3)

volatile int xr_ip_regulator_status[RELULATOR_MAX_COUNT] = { 0 };

static int regulator_clock_enable(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int idx = 0;
	int ret = 0;

	if (!sreg) {
		dev_err(dev, "[%s]xring_regulator_ip is err!\n", __func__);
		return -EINVAL;
	}
	for (idx = 0; idx < sreg->clk_cnt; idx++) {
		if (!IS_ERR_OR_NULL(sreg->dm_clk[idx])) {
			ret = clk_prepare_enable(sreg->dm_clk[idx]);
			if (ret)
				dev_err(dev, "%s clock enable failed!\n", sreg->name);
		} else {
			dev_err(dev, "%s can not get and init clock! idx=%d,clk_cnt=%d\n",
				sreg->name, idx, sreg->clk_cnt);
		}
	}
	for (idx = 0; idx < sreg->clk_cnt; idx++) {
		if (!IS_ERR_OR_NULL(sreg->dm_clk[idx])) {
			if (sreg->rate_en[idx]) { /* Init to specific rate_en after enable */
				if (clk_set_rate(sreg->dm_clk[idx], sreg->rate_en[idx]))
					dev_err(dev, "%s clock set rate failed! idx=%d,rate_en=%lu\n",
						sreg->name, idx, clk_get_rate(sreg->dm_clk[idx]));
			}
		} else {
			dev_err(dev, "%s can not get and init clock! idx=%d,clk_cnt=%d\n",
				sreg->name, idx, sreg->clk_cnt);
		}
	}
	return 0;
}

static int regulator_clock_disable(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int idx = 0;

	if (!sreg) {
		pr_err("[%s]xring_regulator_ip is err!\n", __func__);
		return -EINVAL;
	}

	for (idx = 0; idx < sreg->clk_cnt; idx++) {
		if (!IS_ERR_OR_NULL(sreg->dm_clk[idx])) {
			if (sreg->rate_dis) { /* Reset to specific rate_dis before disable */
				if (clk_set_rate(sreg->dm_clk[idx], sreg->rate_dis))
					dev_err(dev, "%s clock set rate failed! idx=%d,rate_dis=%lu\n",
						sreg->name, idx, clk_get_rate(sreg->dm_clk[idx]));
			}
			clk_disable_unprepare(sreg->dm_clk[idx]);
		} else {
			dev_err(dev, "%s can not get clock! idx=%d,clk_cnt=%d\n",
				sreg->name, idx, sreg->clk_cnt);
		}
	}

	return 0;
}

static int of_regulator_properties(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	int ret;
	int id = 0;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_property_read_u32_array(np, "regulator-id", &id, 1);
	if (ret) {
		dev_err(dev, "%s:get regulator-id failed\n", sreg->name);
		return ret;
	}
	sreg->regulator_id = id;
	sreg->rdesc.id = id;

	ret = device_property_read_u32(dev, "flow-ctrl", &sreg->flowctrl.param);
	if (ret)
		sreg->flowctrl.enable = 0;
	else
		sreg->flowctrl.enable = 1;

	return 0;
}

static int of_regulator_clock_properties(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	int ret = 0;
	unsigned int idx = 0;
	unsigned int rate_dis = 0;
	unsigned int rate_en = 0;
	const char *clk_name = NULL;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para error!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_property_read_u32_array(np, "clock-rate-dis", &rate_dis, 1);
	if (!ret)
		sreg->rate_dis = rate_dis;

	sreg->clk_cnt = of_property_count_strings(np, "clock-names");
	if ((sreg->clk_cnt < MAX_CLK_NAME_NUM) && (sreg->clk_cnt > 0)) {
		for (idx = 0; idx < sreg->clk_cnt; idx++) {
			ret = of_property_read_string_index(np, "clock-names", idx, &clk_name);
			if (ret) {
				dev_err(dev, "get clock-names failed!\n");
				sreg->clk_cnt = 0;
				return ret;
			}
			sreg->dm_clk[idx] = devm_clk_get(dev, clk_name);

			ret = of_property_read_u32_index(np, "clock-rate-en", idx, &rate_en);
			if (!ret)
				sreg->rate_en[idx] = rate_en;
			else
				sreg->rate_en[idx] = 0;
		}
	} else {
		sreg->clk_cnt = 0;
	}

	return 0;
}

static int of_regulator_vote_properties(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	int ret;
	int vote_ch = 0;
	int vote_mode = 0;
	int off_sync = 0;
	const char *vote_name = NULL;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_property_read_string(np, "vote-mng-names", &vote_name);
	if (ret) {
		sreg->vote_cfg.vote_mng = NULL;
		dev_err(dev, "get vote-mng-names failed\n");
		return -EINVAL;
	}
	sreg->vote_cfg.vote_mng = vote_mng_get(dev, vote_name);
	if (!sreg->vote_cfg.vote_mng) {
		dev_err(dev, "regulator get vote mng failed\n");
		return -EPERM;
	}

	ret = of_property_read_u32_array(np, "vote-mng-mode", &vote_mode, 1);
	if (ret) {
		dev_warn(dev, "no vote-mng-mode config\n");
		sreg->vote_cfg.mode = -1;
	} else {
		sreg->vote_cfg.mode = vote_mode;
	}

	ret = of_property_read_u32_array(np, "vote-mng-ch", &vote_ch, 1);
	if (ret) {
		if (sreg->vote_cfg.mode == VOTE_MODE3) {
			dev_err(dev, "no vote-mng-ch config for mode3\n");
			return -EINVAL;
		}
	} else {
		sreg->vote_cfg.ch = vote_ch;
	}

	ret = of_property_read_u32_array(np, "vote-off-sync", &off_sync, 1);
	if (ret)
		sreg->vote_cfg.off_sync = 0;
	else
		sreg->vote_cfg.off_sync = off_sync;

	return 0;
}

static int xring_dt_parse_ip_xctrl_cpu(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_regulator_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}
	ret = of_regulator_vote_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}
	ret = of_regulator_clock_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	return ret;
}

static int xring_dt_parse_ip_lpcore(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_regulator_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}
	ret = of_regulator_vote_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}
	ret = of_regulator_clock_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	return ret;
}

static int xring_dt_parse_ip_atf(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_regulator_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}
	ret = of_regulator_clock_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	return ret;
}

static int xring_dt_parse_ip_gpc(struct xring_regulator_ip *sreg,
	struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!sreg || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_regulator_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	ret = xr_regulator_gpc_init(dev, sreg);
	if (ret) {
		dev_err(dev, "gpc init failed\n");
		return ret;
	}

	ret = of_regulator_clock_properties(sreg, pdev);
	if (ret) {
		dev_err(dev, "get static properties failed\n");
		return ret;
	}

	sreg->last_off_us = 0;
	ret = device_property_read_u32(dev, "off-on-delay", &sreg->off_on_delay);
	if (ret)
		sreg->off_on_delay = 0;

	return 0;
}

static int xring_ip_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;
	return sreg->regulator_enalbe_flag;
}

static inline void xring_ip_regulator_dfx(struct xring_regulator_ip *sreg, bool is_enable)
{
	if (!sreg || (sreg->regulator_id >= RELULATOR_MAX_COUNT)) {
		pr_debug("%s power dfx failed\n", sreg->name);
		return;
	}

	xr_ip_regulator_status[sreg->regulator_id] = is_enable ? 1 : 0;
}

static int xring_ip_to_xctrl_cpu_enable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->vote_cfg.vote_mng == NULL) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.vote_mng is null!\n", __func__, __LINE__);
		return -ENODEV;
	}

	if (sreg->vote_cfg.mode < 0) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.mode not set!\n", __func__, __LINE__);
		return -ENODEV;
	} else if ((sreg->vote_cfg.mode == VOTE_MODE3) && (sreg->vote_cfg.ch < 0)) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.ch not set!\n", __func__, __LINE__);
		return -ENODEV;
	}

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_PRE_ENABLE, NULL);

	if (sreg->vote_cfg.mode == VOTE_MODE3) {
		ret = vote_mng_vote_onoff(sreg->vote_cfg.vote_mng, sreg->vote_cfg.ch,
				VOTE_MNG_ON);
		if (ret) {
			dev_err(&rdev->dev, "%s,%d, %s power on failed ret=%d\n",
					__func__, __LINE__, rdev->desc->name, ret);
			return ret;
		}
	} else { /* use mode1 */
		value[0] = rdev->desc->id;
		value[1] = MEDIA_POWER_ON;
		ret = vote_mng_msg_send(sreg->vote_cfg.vote_mng, value, ARRAY_SIZE(value),
				VOTE_MNG_MSG_SYNC);
		if ((ret) || (value[2])) {
			dev_err(&rdev->dev, "%s power on failed (ret=%d,value[2]=%d)!\n",
					rdev->desc->name, ret, value[2]);
			return -EAGAIN;
		}
	}

	sreg->regulator_enalbe_flag = 1;

	xring_ip_regulator_dfx(sreg, true);

	if (sreg->flowctrl.enable)
		xring_flowctrl_regulator_cfg(sreg->flowctrl.param);

	return 0;
}

static int xring_ip_to_xctrl_cpu_disable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};
	int ret = 0;
	int off_sync;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->vote_cfg.vote_mng == NULL) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.vote_mng is null!\n", __func__, __LINE__);
		return -ENODEV;
	}

	if (sreg->vote_cfg.mode < 0) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.mode not set!\n", __func__, __LINE__);
		return -ENODEV;
	} else if ((sreg->vote_cfg.mode == VOTE_MODE3) && (sreg->vote_cfg.ch < 0)) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.ch not set!\n", __func__, __LINE__);
		return -ENODEV;
	}

	off_sync = sreg->vote_cfg.off_sync ? VOTE_MNG_OFF_SYNC : VOTE_MNG_OFF;
	if (sreg->vote_cfg.mode == VOTE_MODE3) {
		ret = vote_mng_vote_onoff(sreg->vote_cfg.vote_mng, sreg->vote_cfg.ch,
				off_sync);
	} else {
		value[0] = rdev->desc->id;
		value[1] = MEDIA_POWER_OFF;
		ret = vote_mng_msg_send(sreg->vote_cfg.vote_mng, value, ARRAY_SIZE(value),
				VOTE_MNG_MSG_ASYNC);
		if (ret) {
			dev_err(&rdev->dev, "%s power off failed (ret=%d,value[2]=%d)!\n",
					rdev->desc->name, ret, value[2]);
			return ret;
		}
	}

	if (ret) {
		dev_err(&rdev->dev, "%s,%d, %s power off failed ret=%d\n",
			__func__, __LINE__, rdev->desc->name, ret);
		return ret;
	}

	sreg->regulator_enalbe_flag = 0;

	xring_ip_regulator_dfx(sreg, false);

	return 0;
}

static int xring_ip_to_lpcore_enable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	int ret = 0;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (!sreg->vote_cfg.vote_mng) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.vote_mng is null!\n", __func__, __LINE__);
		return -ENODEV;
	}

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_PRE_ENABLE, NULL);

	value[0] = rdev->desc->id;
	value[1] = MEDIA_POWER_ON;
	ret = vote_mng_msg_send(sreg->vote_cfg.vote_mng, value, ARRAY_SIZE(value),
			VOTE_MNG_MSG_SYNC);
	if ((ret) || (value[2])) {
		dev_err(&rdev->dev, "%s power on failed (ret=%d,value[2]=%d)!\n",
			rdev->desc->name, ret, value[2]);
		return -EAGAIN;
	}
	sreg->regulator_enalbe_flag = 1;

	xring_ip_regulator_dfx(sreg, true);

	if (sreg->clk_cnt > 0)
		regulator_clock_enable(&rdev->dev, sreg);

	if (sreg->flowctrl.enable)
		xring_flowctrl_regulator_cfg(sreg->flowctrl.param);

	return 0;
}

static int xring_ip_to_lpcore_disable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	int ret = 0;
	uint32_t value[4] = {0xff, 0xff, 0xff, 0xff};

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->clk_cnt > 0)
		regulator_clock_disable(&rdev->dev, sreg);

	if (sreg->vote_cfg.vote_mng == NULL) {
		dev_err(&rdev->dev, "%s,%d, vote_cfg.vote_mng is null!\n", __func__, __LINE__);
		return -ENODEV;
	}

	value[0] = rdev->desc->id;
	value[1] = MEDIA_POWER_OFF;
	ret = vote_mng_msg_send(sreg->vote_cfg.vote_mng, value, ARRAY_SIZE(value),
		VOTE_MNG_MSG_ASYNC);
	if (ret) {
		dev_err(&rdev->dev, "%s power off failed (ret=%d,value[2]=%d)!\n",
			rdev->desc->name, ret, value[2]);
		return ret;
	}
	sreg->regulator_enalbe_flag = 0;

	xring_ip_regulator_dfx(sreg, false);

	return 0;
}

static int xring_ip_to_atf_enable(struct regulator_dev *rdev)
{
	struct raw_notifier_head rnh;
	struct xring_regulator_ip *sreg = NULL;
	unsigned long flags;
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_PRE_ENABLE, NULL);

	spin_lock_irqsave(&sreg->lock, flags);

	rnh.head = rdev->notifier.head;
	raw_notifier_call_chain(&rnh, REGULATOR_EVENT_PRE_HW_ENABLE, NULL);
	ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_IP_REGULATOR_ON,
			rdev->desc->id, 0, NULL);
	if (ret) {
		spin_unlock_irqrestore(&sreg->lock, flags);
		return -EAGAIN;
	}

	raw_notifier_call_chain(&rnh, REGULATOR_EVENT_POST_HW_ENABLE, NULL);

	spin_unlock_irqrestore(&sreg->lock, flags);

	sreg->regulator_enalbe_flag = 1;

	xring_ip_regulator_dfx(sreg, true);

	if (sreg->clk_cnt > 0)
		regulator_clock_enable(&rdev->dev, sreg);

	if (sreg->flowctrl.enable)
		xring_flowctrl_regulator_cfg(sreg->flowctrl.param);

	return 0;
}

static int xring_ip_to_atf_disable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->clk_cnt > 0)
		regulator_clock_disable(&rdev->dev, sreg);

	ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_IP_REGULATOR_OFF,
			rdev->desc->id, 0, NULL);
	if (ret)
		return -EAGAIN;

	sreg->regulator_enalbe_flag = 0;

	xring_ip_regulator_dfx(sreg, false);

	return 0;
}

static int xring_ip_to_gpc_enable(struct regulator_dev *rdev)
{
	struct raw_notifier_head rnh;
	struct xring_regulator_ip *sreg = NULL;
	unsigned long flags;
	int ret = 0;
	uint32_t cur_mid_r = 0;
	int lock_id = 0;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_PRE_ENABLE, NULL);

	if (sreg->hwlock) {
		ret = hwspin_lock_timeout_irqsave(sreg->hwlock, HWSPINLOCK_TIMEOUT_MS, &flags);
		if (ret) {
			lock_id = hwspin_lock_get_id(sreg->hwlock);
			ret = xr_get_hwspinlock_mid(lock_id, &cur_mid_r);
			if (ret) {
				dev_err(&rdev->dev, "%s get hwspinlock(%d) mid failed\n",
						sreg->name, lock_id);
				return -EINVAL;
			}
			dev_err(&rdev->dev, "%s get hwspin_lock(%d) timeout! used by %d\n",
					sreg->name, lock_id, cur_mid_r);
			return -EBUSY;
		}
	} else {
		spin_lock_irqsave(&sreg->lock, flags);
	}

	rnh.head = rdev->notifier.head;
	raw_notifier_call_chain(&rnh, REGULATOR_EVENT_PRE_HW_ENABLE, NULL);

	ret = xr_regulator_gpc_enable(&rdev->dev, sreg);
	if (ret)
		goto error_out;

	raw_notifier_call_chain(&rnh, REGULATOR_EVENT_POST_HW_ENABLE, NULL);

	if (sreg->hwlock)
		hwspin_unlock_irqrestore(sreg->hwlock, &flags);
	else
		spin_unlock_irqrestore(&sreg->lock, flags);

	sreg->regulator_enalbe_flag = 1;

	xring_ip_regulator_dfx(sreg, true);

	if (sreg->flowctrl.enable)
		xring_flowctrl_regulator_cfg(sreg->flowctrl.param);

	return 0;

error_out:
	if (sreg->hwlock)
		hwspin_unlock_irqrestore(sreg->hwlock, &flags);
	else
		spin_unlock_irqrestore(&sreg->lock, flags);

#ifdef XR_IP_REGULATOR_GPC_DEBUG
	if (ret == -EAGAIN)
		xr_regulator_gpc_debug(&rdev->dev, sreg);
#endif

	return ret;
}

static int xring_ip_to_gpc_disable(struct regulator_dev *rdev)
{
	struct xring_regulator_ip *sreg = NULL;
	int ret = 0;
	unsigned long flags;
	uint32_t cur_mid_r = 0;
	int lock_id = 0;

	if (!rdev)
		return -ENODEV;

	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->hwlock) {
		ret = hwspin_lock_timeout_irqsave(sreg->hwlock, HWSPINLOCK_TIMEOUT_MS, &flags);
		if (ret) {
			lock_id = hwspin_lock_get_id(sreg->hwlock);
			ret = xr_get_hwspinlock_mid(lock_id, &cur_mid_r);
			if (ret) {
				dev_err(&rdev->dev, "%s get hwspinlock(%d) mid failed\n",
						sreg->name, lock_id);
				return -EINVAL;
			}
			dev_err(&rdev->dev, "%s get hwspin_lock(%d) timeout! used by %d\n",
					sreg->name, lock_id, cur_mid_r);
			return -EBUSY;
		}
	}

	ret = xr_regulator_gpc_disable(&rdev->dev, sreg);
	if (ret)
		goto error_out;

	if (sreg->hwlock)
		hwspin_unlock_irqrestore(sreg->hwlock, &flags);

	sreg->regulator_enalbe_flag = 0;

	xring_ip_regulator_dfx(sreg, false);

	return 0;

error_out:
	if (sreg->hwlock)
		hwspin_unlock_irqrestore(sreg->hwlock, &flags);
	return ret;
}

int xr_ip_regulator_get_status(int *status, int count)
{
	int i;

	if ((status == NULL) || (count > RELULATOR_MAX_COUNT))
		return -EINVAL;

	for (i = 0; i < count; i++)
		status[i] = xr_ip_regulator_status[i];

	return 0;
}

const struct regulator_ops xring_ip_xctrl_cpu_rops = {
	.is_enabled = xring_ip_regulator_is_enabled,
	.enable = xring_ip_to_xctrl_cpu_enable,
	.disable = xring_ip_to_xctrl_cpu_disable,
};

const struct regulator_ops xring_ip_lpcore_rops = {
	.is_enabled = xring_ip_regulator_is_enabled,
	.enable = xring_ip_to_lpcore_enable,
	.disable = xring_ip_to_lpcore_disable,
};

const struct regulator_ops xring_ip_atf_rops = {
	.is_enabled = xring_ip_regulator_is_enabled,
	.enable = xring_ip_to_atf_enable,
	.disable = xring_ip_to_atf_disable,
};

const struct regulator_ops xring_ip_gpc_rops = {
	.is_enabled = xring_ip_regulator_is_enabled,
	.enable = xring_ip_to_gpc_enable,
	.disable = xring_ip_to_gpc_disable,
};

static const struct xring_regulator_ip xring_regulator_ip_xctrl_cpu = {
	.rdesc = {
		.ops = &xring_ip_xctrl_cpu_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = xring_dt_parse_ip_xctrl_cpu,
};

static const struct xring_regulator_ip xring_regulator_ip_lpcore = {
	.rdesc = {
		.ops = &xring_ip_lpcore_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = xring_dt_parse_ip_lpcore,
};

static const struct xring_regulator_ip xring_regulator_ip_atf = {
	.rdesc = {
		.ops = &xring_ip_atf_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = xring_dt_parse_ip_atf,
};

static const struct xring_regulator_ip xring_regulator_ip_gpc = {
	.rdesc = {
		.ops = &xring_ip_gpc_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = xring_dt_parse_ip_gpc,
};

static const struct of_device_id xring_of_match_tbl[] = {
	{
		.compatible = "xring,regulator_xctrl_cpu",
		.data = &xring_regulator_ip_xctrl_cpu,
	},
	{
		.compatible = "xring,regulator_lpcore",
		.data = &xring_regulator_ip_lpcore,
	},
	{
		.compatible = "xring,regulator_atf",
		.data = &xring_regulator_ip_atf,
	},
	{
		.compatible = "xring,regulator_gpc",
		.data = &xring_regulator_ip_gpc,
	},
	{ /* end */ }

};

static int of_xring_regulator_ip(struct device *dev,
	struct xring_regulator_ip **sreg)
{
	const struct of_device_id *match = NULL;
	const struct xring_regulator_ip *temp = NULL;

	/* to check which type of regulator this is */
	match = of_match_device(xring_of_match_tbl, dev);
	if (!match) {
		dev_err(dev, "get xring regulator fail!\n\r");
		return -EINVAL;
	}

	temp = match->data;

	*sreg = kmemdup(temp, sizeof(struct xring_regulator_ip), GFP_KERNEL);
	if (!(*sreg))
		return -ENOMEM;

	return 0;
}

static int ip_regulator_suspend(struct device *dev)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct regulator *consumer;
	const char *consumer_name;
	const struct regulator_state *rstate;

	dev_dbg(dev, "%s ++\n", __func__);

	if (!rdev->constraints->always_on && rdev->desc->ops->is_enabled(rdev)) {
		rstate = &rdev->constraints->state_mem;
		if (rstate->enabled == ENABLE_IN_SUSPEND) {
			dev_dbg(dev, "%s -- ENABLE_IN_SUSPEND\n", __func__);
			return 0;
		}

		dev_err(dev, "ERROR: %s is not disabled, rdev use_count:%d\n", rdev->constraints->name, rdev->use_count);

		list_for_each_entry(consumer, &rdev->consumer_list, list) {
			if (consumer->enable_count) {
				consumer_name = consumer->supply_name ? consumer->supply_name :
					consumer->dev ? dev_name(consumer->dev) : "deviceless";
				dev_err(dev, "consumer %s is not disabled, enable_count:%d\n",
						consumer_name, consumer->enable_count);
			}
		}
		if (rdev->desc->id == MEDIA1_SUBSYS_ID || rdev->desc->id == MEDIA2_SUBSYS_ID) {
			WARN_ON(1);
			return -EPERM;
		}
	}

	dev_dbg(dev, "%s --\n", __func__);

	return 0;
}

static int ip_regulator_resume(struct device *dev)
{
	dev_dbg(dev, "%s ++\n", __func__);

	dev_dbg(dev, "%s --\n", __func__);

	return 0;
}

static const struct dev_pm_ops xring_regulator_ip_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ip_regulator_suspend, ip_regulator_resume)
};

static int of_parse_dts(struct platform_device *pdev,
		struct xring_regulator_ip *sreg,
		struct regulator_init_data *initdata)
{
	const char *supplyname = NULL;
	struct device_node *np = pdev->dev.of_node;
	u32 hwlock_id;
	int ret;

	/* common params */
	supplyname = of_get_property(np, "xring,supply_name", NULL);
	if (supplyname != NULL)
		initdata->supply_regulator = supplyname;

	/* hwlock_id is optional */
	ret = of_property_read_u32(np, "xring,hwlock_id", &hwlock_id);
	if (ret == 0) {
		dev_info(&pdev->dev, "Regulator(%s) hwlock_id=%d\n", sreg->name, hwlock_id);

		sreg->hwlock = hwspin_lock_request_specific(hwlock_id);
		if (!sreg->hwlock) {
			dev_err(&pdev->dev, "Regulator(%s) request hwspinlock[%d] fail!\n",
					sreg->name, hwlock_id);
			return -EBUSY;
		}
	}

	/* to parse device tree data for regulator specific */
	ret = sreg->dt_parse(sreg, pdev);
	if (ret) {
		pr_err("parse regulator specific parameter error(%d)!\n", ret);
		return ret;
	}

	return 0;
}

static int xring_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	struct regulator_desc *rdesc = NULL;
	struct regulator_dev *rdev = NULL;
	struct xring_regulator_ip *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config = {};
	int ret;

	if (!pdev) {
		pr_err("[%s]regulator get  platform device para is err!\n",
			__func__);
		return -EINVAL;
	}

	dev = &pdev->dev;
	np = dev->of_node;

	dev_dbg(dev, "xirng_ip_regulator probe start\n");

	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (!initdata) {
		dev_err(dev, "get regulator init data error !\n");
		return -EINVAL;
	}

	ret = of_xring_regulator_ip(dev, &sreg);
	if (ret) {
		dev_err(dev, "allocate xring_regulator_ip fail!\n");
		goto xring_ip_probe_end;
	}

	sreg->name = initdata->constraints.name;
	rdesc = &sreg->rdesc;
	rdesc->name = sreg->name;

	ret = of_parse_dts(pdev, sreg, initdata);
	if (ret)
		goto xring_ip_probe_end;

	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;
	sreg->np = np;
	sreg->regulator_enalbe_flag = 0;

	/* register regulator */
	rdev = regulator_register(&pdev->dev, rdesc, &config);
	if (IS_ERR(rdev)) {
		dev_err(dev, "failed to register %s\n", rdesc->name);
		ret = PTR_ERR(rdev);
		goto xring_ip_probe_end;
	}

	platform_set_drvdata(pdev, rdev);

	spin_lock_init(&sreg->lock);

	dev_dbg(dev, "xirng_ip_regulator probe end\n");

	return 0;

xring_ip_probe_end:
	if (sreg->hwlock) {
		hwspin_lock_free(sreg->hwlock);
		sreg->hwlock = NULL;
	}
	kfree(sreg);
	return ret;
}

static int xring_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = NULL;
	struct xring_regulator_ip *sreg = NULL;
	struct device *dev = &pdev->dev;

	if (!pdev)
		return -ENODEV;

	rdev = platform_get_drvdata(pdev);
	if (!rdev)
		return -ENODEV;

	regulator_unregister(rdev);
	sreg = rdev_get_drvdata(rdev);
	if (!sreg)
		return -EINVAL;

	if (sreg->vote_cfg.vote_mng) {
		vote_mng_put(dev, sreg->vote_cfg.vote_mng);
		sreg->vote_cfg.vote_mng = NULL;
	}
	if (sreg->hwlock) {
		hwspin_lock_free(sreg->hwlock);
		sreg->hwlock = NULL;
	}

	kfree(sreg);
	return 0;
}

static struct platform_driver xring_ip_regulator_driver = {
	.driver = {
		.name = "xring_ip_regulator",
		.owner = THIS_MODULE,
		.of_match_table = xring_of_match_tbl,
		.pm = &xring_regulator_ip_dev_pm_ops,
	},
	.probe = xring_regulator_probe,
	.remove = xring_regulator_remove,
};

int xring_regulator_ip_init(void)
{
	return platform_driver_register(&xring_ip_regulator_driver);
}

void xring_regulator_ip_exit(void)
{
	platform_driver_unregister(&xring_ip_regulator_driver);
}
