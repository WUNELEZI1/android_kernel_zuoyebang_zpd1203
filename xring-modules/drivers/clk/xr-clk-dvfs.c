// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.s_vote
 */
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/hwspinlock.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <dt-bindings/xring/xr-clk-vote.h>
#include <soc/xring/xr-clk-provider.h>
#include "clk.h"
#include "xr-clk-dvfs.h"

#define to_xr_dvfs_clk(_hw) container_of(_hw, struct xr_dvfs_clk, hw)

static int is_low_temp(struct xr_dvfs_clk *dfclk)
{
	struct dvfs_volt_vote *vote_inst = NULL;

	if (!dfclk->lowtemp_property)
		return 0;

	vote_inst = dvfs_vote_get(dfclk->vote_id, NULL);
	if (vote_inst == NULL) {
		clkerr("vote_inst get failed, dev_id = %u!\n", dfclk->vote_id);
		return -EINVAL;
	}

	return dvfs_is_low_temperature(vote_inst);
}

static long dvfs_clk_round_rate(struct clk_hw *hw,
		unsigned long rate, unsigned long *prate)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return rate;
	}

	return clk_get_rate(friend_clk);
}

static int dvfs_clk_determine_rate(struct clk_hw *hw,
		struct clk_rate_request *req)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return -EBADR;
	}

	if (dfclk->vote_type == MEDIA_DVFS_VOTE &&
		(!is_media_dvfs_ft_on()))
		return 0;

	if (dfclk->vote_type == PERI_DVFS_VOTE &&
		(!is_peri_dvfs_ft_on()))
		return 0;

	if (!__clk_is_enabled(friend_clk))
		return 0;

	if (is_low_temp(dfclk) == LOW_TEMPRATURE) {
		if ((req->rate) > (dfclk->lowtemp_max_freq)) {
			clkerr("clk name =%s, cur_freq-%lu > lowtemp_max_freq-%lu!\n",
				clk_hw_get_name(hw), req->rate, dfclk->lowtemp_max_freq);
			return -ELOW_TEMP;
		}
	}

	return 0;
}

static unsigned long dvfs_clk_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return 0;
	}

	return clk_get_rate(friend_clk);
}

static int set_rate_change_volt_async(struct xr_dvfs_clk *dfclk,
		struct clk *friend_clk, struct dvfs_volt_vote *vote_inst,
		unsigned long rate, unsigned int volt_level)
{
	unsigned long freq_old;
	int ret;

	freq_old = clk_get_rate(friend_clk);

	ret = clk_set_rate(friend_clk, rate);
	if (ret < 0) {
		clkerr("%s fail to set rate %lu, ret = %d!\n",
			clk_hw_get_name(*(dfclk->friend_hw)), rate, ret);
		return ret;
	}

	ret = dvfs_set_volt(vote_inst, volt_level);
	if (ret < 0) {
		clkerr("set volt_level %u failed, ret = %d!\n", volt_level, ret);
		ret = clk_set_rate(friend_clk, freq_old);
		if (ret < 0)
			clkerr("fail to reback, set old rate %lu, ret = %d!\n", freq_old, ret);
	}

	return ret;
}

static int set_rate_change_volt_sync(struct xr_dvfs_clk *dfclk,
		struct clk *friend_clk, struct dvfs_volt_vote *vote_inst,
		unsigned long rate, unsigned int volt_level)
{
	int ret;

	ret = dvfs_set_volt(vote_inst, volt_level);
	if (ret < 0) {
		clkerr("set volt_level %u failed ret = %d !\n", volt_level, ret);
		return ret;
	}
	ret = dvfs_wait_completed(vote_inst);
	if (ret != 0)
		return ret;

	ret = clk_set_rate(friend_clk, rate);
	if (ret < 0)
		clkerr("%s fail to set rate %lu, ret = %d!\n",
			clk_hw_get_name(*(dfclk->friend_hw)), rate, ret);

	return ret;
}

static int clk_set_rate_change_volt(struct xr_dvfs_clk *dfclk,
		struct clk *friend_clk,	unsigned long rate,
		unsigned int volt_level)
{
	struct dvfs_volt_vote *vote_inst = NULL;
	unsigned long freq_old;
	int ret = 0;

	vote_inst = dvfs_vote_get(dfclk->vote_id, NULL);
	if (vote_inst == NULL) {
		clkerr("vote_inst get failed, vote_id %u!\n", dfclk->vote_id);
		return -EINVAL;
	}

	freq_old = clk_get_rate(friend_clk);
	if (!freq_old) {
		clkerr("clk %s rate must not be 0, please check!\n",
			clk_hw_get_name(*(dfclk->friend_hw)));
		return -EINVAL;
	}

	/*
	 *      raise frequency: raise voltage, set frequency
	 *      reduce frequency: reduce frequency, set voltage
	 */
	if (rate > freq_old)
		ret = set_rate_change_volt_sync(dfclk, friend_clk,
			vote_inst, rate, volt_level);
	else
		/* Profile down case no need wait AVS and Profile voltage OK */
		ret = set_rate_change_volt_async(dfclk, friend_clk,
			vote_inst, rate, volt_level);

	return ret;
}

static int __dvfs_clk_set_rate(struct xr_dvfs_clk *dfclk,
		struct clk *friend_clk, unsigned long rate)
{
	unsigned int level, i;
	int ret;

	if (dfclk->sensitive_level <= 1) {
		clkerr("input sensitive level err %u\n", dfclk->sensitive_level);
		return -EINVAL;
	}

	level = dfclk->sensitive_level - 1;

	for (i = 0; i < level; i++) {
		if (rate <= dfclk->sensitive_freq[i])
			break;
	}

	ret = clk_set_rate_change_volt(dfclk, friend_clk,
		rate, dfclk->sensitive_volt[i]);
	if (ret < 0)
		clkerr("set volt %u failed ret = %d, rate %lu, i %u!\n",
			dfclk->sensitive_volt[i], ret, rate, i);

	return ret;
}

static int dvfs_clk_set_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);
	int ret;

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return -EBADR;
	}

	if (dfclk->vote_type == MEDIA_DVFS_VOTE &&
		(!is_media_dvfs_ft_on()))
		goto out;

	if (dfclk->vote_type == PERI_DVFS_VOTE &&
		(!is_peri_dvfs_ft_on()))
		goto out;

	if (!__clk_is_enabled(friend_clk))
		goto out;

	ret = __dvfs_clk_set_rate(dfclk, friend_clk, rate);

	return ret;
out:
	ret = clk_set_rate(friend_clk, rate);
	if (ret < 0)
		clkerr("friend clk %s fail to set rate %lu, ret = %d!\n",
			clk_hw_get_name(*(dfclk->friend_hw)), rate, ret);
	return ret;
}

static int prepare_change_volt_sync(unsigned int vote_id,
		unsigned int volt_level)
{
	struct dvfs_volt_vote *vote_inst = NULL;
	int ret;

	vote_inst = dvfs_vote_get(vote_id, NULL);
	if (vote_inst == NULL) {
		clkerr("dvfs vote get failed, vote_id %u!\n", vote_id);
		return -EINVAL;
	}
	ret = dvfs_set_volt(vote_inst, volt_level);
	if (ret < 0) {
		clkerr("vote inst %u set volt level %u failed, ret = %d!\n",
			vote_id, volt_level, ret);
		return ret;
	}
	ret = dvfs_wait_completed(vote_inst);

	return ret;
}

static int clk_prepare_change_volt(struct xr_dvfs_clk *dfclk,
		unsigned long cur_rate)
{
	int ret = 0;
	unsigned int i;
	unsigned int level;

	if (dfclk->sensitive_level <= 1) {
		clkerr("input sensitive level err %u\n", dfclk->sensitive_level);
		return -EINVAL;
	}

	level = dfclk->sensitive_level - 1;

	if (dfclk->sensitive_freq[0] == 0) {
		ret = prepare_change_volt_sync(dfclk->vote_id,
			dfclk->sensitive_volt[level]);
		return ret;
	}

	for (i = 0; i < level; i++) {
		if (cur_rate <= dfclk->sensitive_freq[i])
			break;
	}

	ret = prepare_change_volt_sync(dfclk->vote_id,
		dfclk->sensitive_volt[i]);

	return ret;
}

static int __dvfs_clk_prepare(struct xr_dvfs_clk *dfclk)
{
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);
	unsigned long cur_rate;
	int ret;

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(*(dfclk->friend_hw)));
		return -EBADR;
	}

	cur_rate = clk_get_rate(friend_clk);
	if (!cur_rate) {
		clkerr("clk %s rate must not be 0, please check!\n",
			clk_hw_get_name(*(dfclk->friend_hw)));
		return -EINVAL;
	}

	if (is_low_temp(dfclk) == LOW_TEMPRATURE) {
		if (cur_rate > (dfclk->lowtemp_max_freq)) {
			clkerr("cur_freq-%lu > lowtemp_max_freq-%lu!\n",
				cur_rate, dfclk->lowtemp_max_freq);
			return -ELOW_TEMP;
		}
	}

	ret = clk_prepare_change_volt(dfclk, cur_rate);
	if (ret < 0)
		clkerr("dvfs change volt failed, clk name %s, ret = %d, cur_rate %lu!\n",
			clk_hw_get_name(*(dfclk->friend_hw)), ret, cur_rate);

	return ret;
}

static int dvfs_clk_prepare(struct clk_hw *hw)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);
	int ret;

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return -EBADR;
	}

	ret = clk_prepare(friend_clk);
	if (ret) {
		clkerr("friend clock %s prepare faild, ret %d!\n",
			clk_hw_get_name(*(dfclk->friend_hw)), ret);
		return ret;
	}

	if (dfclk->vote_type == MEDIA_DVFS_VOTE &&
		(!is_media_dvfs_ft_on()))
		return ret;

	if (dfclk->vote_type == PERI_DVFS_VOTE &&
		(!is_peri_dvfs_ft_on()))
		return ret;

	ret = __dvfs_clk_prepare(dfclk);
	if (ret < 0) {
		clkerr("clock %s prepare dvfs faild, ret = %d!\n",
			clk_hw_get_name(hw), ret);
		clk_unprepare(friend_clk);
	}

	return ret;
}

static int dvfs_clk_enable(struct clk_hw *hw)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);
	int ret;

	if (IS_ERR_OR_NULL(friend_clk)) {
		clkerr("get %s friend clk fail!\n", clk_hw_get_name(hw));
		return -EBADR;
	}

	ret = clk_enable(friend_clk);
	if (ret) {
		clkerr("friend clock %s enable faild, ret %d!",
			clk_hw_get_name(*(dfclk->friend_hw)), ret);
		return ret;
	}

	return ret;
}

static void dvfs_clk_disable(struct clk_hw *hw)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);

	/* if friend clk exist, disable it . */
	if (!IS_ERR_OR_NULL(friend_clk))
		clk_disable(friend_clk);
}

static void __dvfs_clk_unprepare(struct xr_dvfs_clk *dfclk)
{
	int ret;

	ret = prepare_change_volt_sync(dfclk->vote_id, VOLT_LEVEL_0);
	if (ret < 0)
		clkerr("volt change to %u failed, ret = %d, id = %u!\n",
			VOLT_LEVEL_0, ret, dfclk->vote_id);
}

static void dvfs_clk_unprepare(struct clk_hw *hw)
{
	struct xr_dvfs_clk *dfclk = to_xr_dvfs_clk(hw);
	struct clk *friend_clk = get_friend_clk(dfclk->friend_hw);

	if (dfclk->vote_type == MEDIA_DVFS_VOTE &&
		(!is_media_dvfs_ft_on()))
		goto out;

	if (dfclk->vote_type == PERI_DVFS_VOTE &&
		(!is_peri_dvfs_ft_on()))
		goto out;

	__dvfs_clk_unprepare(dfclk);

out:
	if (!IS_ERR_OR_NULL(friend_clk))
		clk_unprepare(friend_clk);
}

static const struct clk_ops dvfs_clk_ops = {
	.recalc_rate    = dvfs_clk_recalc_rate,
	.set_rate       = dvfs_clk_set_rate,
	.determine_rate = dvfs_clk_determine_rate,
	.round_rate     = dvfs_clk_round_rate,
	.prepare        = dvfs_clk_prepare,
	.unprepare      = dvfs_clk_unprepare,
	.enable         = dvfs_clk_enable,
	.disable        = dvfs_clk_disable,
};

static int dvfs_input_param_check(struct clk_hw **friend_hw, unsigned int vote_id,
	unsigned int vote_type,	const struct dvfs_cfg *dvfs_cfg)
{
	int i;

	if (vote_type >= MAX_DVFS_VOTE) {
		clkerr("input vote_type err %u!", vote_type);
		goto out;
	}

	if (vote_id >= CLK_MAX_VOTE_ID) {
		clkerr("input vote_id err %u!", vote_id);
		goto out;
	}

	if (!friend_hw) {
		clkerr("friend hw illegal!\n");
		goto out;
	}

	if (dvfs_cfg->lowtemp_max_freq > (ULONG_MAX / FREQ_CONVERSION_COEFF)) {
		clkerr("lowtemp_max_freq %lu too large!\n", dvfs_cfg->lowtemp_max_freq);
		goto out;
	}

	if (dvfs_cfg->sensitive_level > DVFS_MAX_FREQ_NUM ||
		dvfs_cfg->sensitive_level > DVFS_MAX_VOLT_NUM) {
		clkerr("sensitive level %u too large!\n", dvfs_cfg->sensitive_level);
		goto out;
	}

	for (i = 0; i < dvfs_cfg->sensitive_level; i++) {
		if (dvfs_cfg->sensitive_freq[i] > (ULONG_MAX / FREQ_CONVERSION_COEFF)) {
			clkerr("sensitive_freq %lu too large!\n", dvfs_cfg->sensitive_freq[i]);
			goto out;
		}
	}
	return 0;
out:
	return -EINVAL;
}

struct clk_hw *devm_xr_clk_hw_dvfs_clock(struct device *dev, const char *name,
		struct clk_hw **friend_hw, unsigned int vote_id,
		unsigned int vote_type, const struct dvfs_cfg *dvfs_cfg)
{
	int i, ret;
	struct clk_hw *hw = NULL;
	struct clk_init_data init;
	struct xr_dvfs_clk *dvfs_clk = NULL;

	dvfs_clk = devm_kzalloc(dev, sizeof(struct xr_dvfs_clk), GFP_KERNEL);
	if (!dvfs_clk)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &dvfs_clk_ops;
	init.parent_names = NULL;
	init.num_parents = 0;
	init.flags = 0;
	dvfs_clk->hw.init = &init;

	ret = dvfs_input_param_check(friend_hw, vote_id, vote_type, dvfs_cfg);
	if (ret)
		return ERR_PTR(ret);

	dvfs_clk->lowtemp_property = dvfs_cfg->lowtemp_property;
	dvfs_clk->lowtemp_max_freq = dvfs_cfg->lowtemp_max_freq * FREQ_CONVERSION_COEFF;

	dvfs_clk->vote_id = vote_id;
	dvfs_clk->vote_type = vote_type;
	dvfs_clk->friend_hw = friend_hw;

	dvfs_clk->sensitive_level = dvfs_cfg->sensitive_level;

	for (i = 0; i < dvfs_cfg->sensitive_level; i++) {
		dvfs_clk->sensitive_freq[i] =
			dvfs_cfg->sensitive_freq[i] * FREQ_CONVERSION_COEFF;
		dvfs_clk->sensitive_volt[i] = dvfs_cfg->sensitive_volt[i];
	}

	hw = &dvfs_clk->hw;

	ret = clk_hw_register(NULL, hw);
	if (ret)
		return ERR_PTR(ret);

	/* init is local variable, need set NULL before func */
	dvfs_clk->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_dvfs_clock);
