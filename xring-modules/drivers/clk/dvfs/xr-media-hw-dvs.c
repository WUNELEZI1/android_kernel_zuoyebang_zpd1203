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
 * GNU General Public License for more details.
 */
#include "xr-media-hw-dvs.h"
#include "clk/xr-clk-common.h"
#include "clk/xr-dvfs-vote.h"
#include "xr-common-hw-dvs.h"
#include "xr-dvs-ctrl.h"
#include <dt-bindings/xring/xr-clk-vote.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/hwspinlock.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <dt-bindings/xring/platform-specific/common/pmu/include/pmu_common_resource.h>
#include <soc/xring/xr-clk-provider.h>

#define media_volt2code(voltage) ((uint8_t)DIV_ROUND_UP((voltage) - \
		CPMIC_DAC_CODE_BASE_MEDIA, CPMIC_DAC_CODE_STEP_MEDIA))

enum media_dvs_dev {
	DVS_FEA,
	DVS_CTRL,
	DVS_MEDIA1BUS,
	DVS_MEDIA2BUS,
	DVS_CHANNEL,
	DVS_VOTE,
	DVS_MNTN,
};

static unsigned int g_max_chan_num;
static struct dvs_config *g_dvs_cfg;
static struct clock_mediabus *g_mediabus_cfg[MEDIABUS_MAX];
static struct dvs_channel *g_dvs_channels[MAX_CHANNEL_NUM];
static struct dvs_mntn *g_dvs_mntn;

static unsigned int g_media_volt_table[CLK_MAX_VOTE_ID][MEDIA_DVFS_VOLT_LEVEL] = {
	[CLK_DPUCORE0_VOTE_ID] = { 600, 650, 750 }, /* dpucore0 mv */
	[CLK_DPUVEU_VOTE_ID] = { 600, 650, 750 },   /* dpuveu mv */
	[CLK_VDEC_VOTE_ID] = { 600, 650, 750 },     /* vdec mv */
	[CLK_ISPFUNC1_VOTE_ID] = { 600, 650, 750 }, /* isp1 mv */
	[CLK_ISPFUNC2_VOTE_ID] = { 600, 650, 750 }, /* isp2 mv */
	[CLK_ISPFUNC3_VOTE_ID] = { 600, 650, 750 }, /* isp3 mv */
	[CLK_VENC_VOTE_ID] = { 600, 650, 750 },     /* venc mv */
};

int media_dvs_is_low_temperature(void)
{
	return dvs_is_low_temperature(g_dvs_cfg);
}
EXPORT_SYMBOL_GPL(media_dvs_is_low_temperature);

unsigned int get_volt_by_volt_level(unsigned int dev_id,
					   unsigned int level)
{
	return g_media_volt_table[dev_id][level];
}

int get_dpu_avs_volt(unsigned int volt_level, uint8_t *dac_code)
{
	unsigned int voltage;

	if (volt_level >= MEDIA_DVFS_VOLT_LEVEL) {
		clkerr("dpu input level %u err!\n", volt_level);
		return -EINVAL;
	}

	if (!dac_code) {
		clkerr("dpu input dac code null!\n");
		return -EINVAL;
	}

	/* volt level to voltage value */
	voltage = get_volt_by_volt_level(CLK_DPUCORE0_VOTE_ID, volt_level);

	*dac_code = media_volt2code(voltage);

	return 0;
}
EXPORT_SYMBOL_GPL(get_dpu_avs_volt);


static int media_dvs_wait_completed(struct dvfs_volt_vote *vote_inst)
{
	struct dvs_hw hw = { 0 };

	if (vote_inst->channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote channel id %u illegal!\n", vote_inst->channel_id);
		return -EINVAL;
	}

	hw.base = XR_CLK_DVSMEDIA;
	hw.ch = vote_inst->channel_id;
	return dvs_wait_ack(&hw);
}

static unsigned int media_dvs_get_volt(struct dvfs_volt_vote *vote_inst)
{
	struct dvs_hw hw = { 0 };
	uint8_t dac_code;
	unsigned int volt;
	int ret;

	if (unlikely(!vote_inst || !g_dvs_mntn))
		return -EINVAL;

	if (unlikely(g_dvs_mntn->volt_level_fixed != MEDIA_FIXED_VOLT_NONE))
		return g_dvs_mntn->volt_level_fixed - 1;

	if (vote_inst->channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote channel id %u illegal!\n", vote_inst->channel_id);
		return -EINVAL;
	}

	hw.base = XR_CLK_DVSMEDIA;
	hw.ch = vote_inst->channel_id;
	ret = dvs_get_cur_volt(&hw, &dac_code);
	if (ret) {
		clkerr("media dvs get volt fail %d!", ret);
		return 0;
	}

	volt = dac_code;

	return ((volt * CPMIC_DAC_CODE_STEP_MEDIA) + CPMIC_DAC_CODE_BASE_MEDIA);
}

static int mediabus_freq_update(unsigned int mediabus_id)
{
	struct clock_mediabus *mediabus = NULL;
	struct dvfs_volt_vote *vote_inst = NULL;
	unsigned int target_volt_level = MEDIA_VOLT_0;
	unsigned long target_rate;
	int low_temp, ret, i;

	/* ignore invalid mediabus type */
	if (mediabus_id >= MEDIABUS_MAX || !g_mediabus_cfg[mediabus_id])
		return -EINVAL;

	mediabus = g_mediabus_cfg[mediabus_id];
	list_for_each_entry(vote_inst, &mediabus->mediabus_list, mediabus_node)
		target_volt_level = max_value(target_volt_level, vote_inst->volt_level);

	/* low temp case */
	low_temp = media_dvs_is_low_temperature();
	if ((low_temp == LOW_TEMPRATURE) && (target_volt_level > mediabus->lowtemp_volt_level))
		target_volt_level = mediabus->lowtemp_volt_level;

	if (target_volt_level >= MEDIA_VOLT_MAX ||
	    target_volt_level >= DVFS_MAX_FREQ_NUM) {
		clkerr("invalid media volt level %u\n", target_volt_level);
		return -EINVAL;
	}

	for (i = 0; i < mediabus->link_clk_num; i++) {
		target_rate = mediabus->sensitive_freq[i][target_volt_level];
		ret = clk_set_rate(mediabus->bus_clk[i], target_rate);
		if (ret) {
			clkerr("mediabus %d set rate %lu fail %d!\n", i, target_rate, ret);
			return ret;
		}
	}
	mediabus->volt_level = target_volt_level;
	return ret;
}

static int dvs_set_volt_hw_cfg(struct dvs_channel *chan,
			       struct dvfs_volt_vote *vote_inst, unsigned int volt_level)
{
	struct dvs_hw hw = { 0 };
	unsigned int voltage;
	uint8_t dac_code;
	int ret;

	/* volt level to voltage value */
	voltage = get_volt_by_volt_level(vote_inst->dev_id, volt_level);

	dac_code = (uint8_t)DIV_ROUND_UP(voltage - CPMIC_DAC_CODE_BASE_MEDIA,
		CPMIC_DAC_CODE_STEP_MEDIA);

	hw.base = XR_CLK_DVSMEDIA;
	hw.ch = vote_inst->channel_id;

	ret = dvs_write_no_wait_ack(&hw, dac_code);
	if (ret) {
		clkerr("dvs write async fail %d\n", ret);
		return ret;
	}

	return 0;
}

static int __media_dvs_set_volt(struct dvfs_volt_vote *vote_inst)
{
	struct dvfs_volt_vote *inst = NULL;
	struct dvs_channel *dvs_channel = NULL;
	unsigned int channel_id, cur_volt_level;
	unsigned int target_volt_level = MEDIA_VOLT_0;
	int ret;

	channel_id = vote_inst->channel_id;
	dvs_channel = g_dvs_channels[channel_id];
	cur_volt_level = dvs_channel->volt_level;

	/* gather all vote inst on the same channel */
	list_for_each_entry(inst, &dvs_channel->chan_list, chan_node)
		target_volt_level = max_value(target_volt_level, inst->volt_level);

	if (cur_volt_level >= target_volt_level) {
		/* mediabus freq down before volt down */
		ret = mediabus_freq_update(vote_inst->mediabus_id);
		if (ret)
			return ret;
		/* media volt vote flow */
		ret = dvs_set_volt_hw_cfg(dvs_channel, vote_inst, target_volt_level);
		if (ret)
			return ret;
		/*
		 * When the system enters doze, the dvs will not have a clock, causing it
		 * to work abnormally. Therefore, it is necessary to ensure that
		 * the system has a chance to enter doze after the dvs execution is completed.
		 */
#if IS_ENABLED(CONFIG_XRING_DOZE)
		/* wait dvs_media set volt done */
		ret = media_dvs_wait_completed(vote_inst);
		if (ret)
			return ret;
#endif
	} else {
		/* media volt vote flow */
		ret = dvs_set_volt_hw_cfg(dvs_channel, vote_inst, target_volt_level);
		if (ret)
			return ret;
		/* wait dvs_media set volt done */
		ret = media_dvs_wait_completed(vote_inst);
		if (ret)
			return ret;
		/* mediabus freq up after volt up */
		ret = mediabus_freq_update(vote_inst->mediabus_id);
		if (ret)
			return ret;
	}

	/* update channel volt level */
	dvs_channel->volt_level = target_volt_level;

	return 0;
}

static void media_dvs_per_vote_record(const struct dvfs_volt_vote *vote_inst)
{
	unsigned int channel_id;
	struct dvs_channel *dvs_channel = NULL;

	channel_id = vote_inst->channel_id;
	dvs_channel = g_dvs_channels[channel_id];
	dvs_per_vote_record(dvs_channel, g_dvs_cfg);
}

static int media_dvs_set_volt(struct dvfs_volt_vote *vote_inst,
			      unsigned int volt_level)
{
	unsigned int old_volt_level;
	int ret;

	if (unlikely(!vote_inst || !g_dvs_mntn))
		return -EINVAL;

	/* fixed volt case */
	if ((g_dvs_mntn->volt_level_fixed != MEDIA_FIXED_VOLT_NONE) &&
	    (volt_level + 1 > g_dvs_mntn->volt_level_fixed)) {
		clkerr("can not set %u bigger than volt_level_fixed %u\n",
		       volt_level, g_dvs_mntn->volt_level_fixed - 1);
		return -EFIXED_VOLT;
	}

	old_volt_level = vote_inst->volt_level;
	vote_inst->volt_level = volt_level;
	ret = __media_dvs_set_volt(vote_inst);
	if (ret) {
		clkerr("media dvs set volt_level %u fail, ret %d!\n",
		       volt_level, ret);
		vote_inst->volt_level = old_volt_level;
		return ret;
	}

	media_dvs_per_vote_record(vote_inst);

	return ret;
}

static struct dvfs_vote_ops dvs_ops = {
	.get_volt = media_dvs_get_volt,
	.set_volt = media_dvs_set_volt,
	.wait_completed = media_dvs_wait_completed,
	.is_low_temperature = media_dvs_is_low_temperature,
	.get_volt_by_level = get_volt_by_volt_level,
};

static int dvfs_vote_dt_parse(const struct device_node *np,
			      struct dvfs_volt_vote *vote_inst)
{
	if (of_property_read_string(np, "mediavote-output-names",
				    &vote_inst->name)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "mediavote-output-names");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "mediavote-poll-id",
				 &vote_inst->dev_id)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "mediavote-poll-id");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "mediavote-poll-channel",
				 &vote_inst->channel_id)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "mediavote-poll-channel");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "mediavote-poll-mediabus",
				 &vote_inst->mediabus_id)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "mediavote-poll-mediabus");
		return -EINVAL;
	}

	return 0;
}

static int dvfs_vote_node_init(struct device *dev,
			       const struct device_node *np)
{
	struct dvfs_volt_vote *vote_inst = NULL;
	unsigned int channel_id, mediabus_id;
	int ret;

	vote_inst = devm_kzalloc(dev, sizeof(*vote_inst), GFP_KERNEL);
	if (vote_inst == NULL)
		return -ENOMEM;

	ret = dvfs_vote_dt_parse(np, vote_inst);
	if (ret)
		return ret;

	vote_inst->volt_level = MEDIA_VOLT_0;
	vote_inst->ops = &dvs_ops;

	channel_id = vote_inst->channel_id;
	if (channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote have a invalid channel id %u!\n", channel_id);
		return -EINVAL;
	}
	list_add(&vote_inst->chan_node,
		 &(g_dvs_channels[channel_id]->chan_list));

	mediabus_id = vote_inst->mediabus_id;
	if (mediabus_id >= MEDIABUS_MAX) {
		clkerr("dvfs vote have a invalid mediabus id %u!\n", mediabus_id);
		return -EINVAL;
	}

	if (!g_mediabus_cfg[mediabus_id]) {
		clkerr("mediabus%u do not init, skip register %s!\n", mediabus_id + 1,
		       vote_inst->name);
		return 0;
	}

	list_add(&vote_inst->mediabus_node,
		 &(g_mediabus_cfg[mediabus_id]->mediabus_list));

	ret = dvfs_vote_register(vote_inst);
	if (ret)
		clkerr("dvfs vote register faild, %d!\n", ret);

	return ret;
}

static void media_dvs_debug_init(void);

static int media_dvs_vote_init(struct device *dev)
{
	const struct device_node *np = dev->of_node;
	struct device_node *node = NULL;
	int ret = 0;

	for_each_child_of_node(np, node) {
		ret = dvfs_vote_node_init(dev, node);
		if (ret < 0)
			return ret;
	}

	media_dvs_debug_init();

	return ret;
}

static int media_dvs_per_channel_init(struct device *dev,
				      void __iomem *reg_base, unsigned int chan)
{
	struct device_node *np = dev->of_node;
	struct dvs_channel *dvs_channel = NULL;
	unsigned int dvs_cfg[ADDR_CFG_MAX] = { 0 };
	char prot[MAX_DVS_PROT_STRLEN] = { 0 };
	int ret;

	dvs_channel = devm_kzalloc(dev, sizeof(*dvs_channel), GFP_KERNEL);
	if (!dvs_channel)
		return -ENOMEM;

	ret = snprintf(prot, sizeof(prot), "xring,ch%u-record-reg", chan);
	if (ret <= 0) {
		clkerr("snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	if (of_property_read_u32_array(np, prot, &dvs_cfg[0],
				       ADDR_CFG_MAX)) {
		clkerr("%s node doesn't have %s\n", np->name, prot);
		return -EINVAL;
	}

	dvs_channel->record_reg_offset = dvs_cfg[ADDR_OFFSET];
	dvs_channel->record_mask = dvs_cfg[ADDR_MASK];
	dvs_channel->record_shift = dvs_cfg[ADDR_SHIFT];

	dvs_channel->channel_id = chan;
	dvs_channel->reg_base = reg_base;

	g_dvs_channels[chan] = dvs_channel;
	INIT_LIST_HEAD(&dvs_channel->chan_list);

	dvs_channel->volt_level = MEDIA_VOLT_0;
	return 0;
}

static int media_dvs_channel_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	unsigned int i;
	void __iomem *reg_base = NULL;
	int ret = 0;

	ret = dvs_get_base(np, &reg_base);
	if (ret)
		return ret;

	if (of_property_read_u32(np, "xring,channel-num", &g_max_chan_num)) {
		clkerr("%s node doesn't have %s\n", np->name, "xring,channel-num");
		return -EINVAL;
	}
	if (g_max_chan_num > MAX_CHANNEL_NUM) {
		clkerr("chan num illegal %u > max_channel_num %u\n", g_max_chan_num,
		       MAX_CHANNEL_NUM);
		return -EINVAL;
	}

	for (i = 0; i < g_max_chan_num; i++) {
		ret = media_dvs_per_channel_init(dev, reg_base, i);
		if (ret < 0) {
			clkerr("media dvs channel init fail, chan id %u\n", i);
			return ret;
		}
	}

	return ret;
}

static int link_clock_per_init(struct device *dev, struct clock_mediabus *mediabus,
			       unsigned int clk_id)
{
	const struct device_node *np = dev->of_node;
	char prot[MAX_DVS_PROT_STRLEN] = { 0 };
	unsigned int freq[MEDIA_DVFS_VOLT_LEVEL];
	int ret, i;

	ret = snprintf(prot, sizeof(prot), "xring,mediabus-clock%u-freq-tbl", clk_id);
	if (ret <= 0) {
		clkerr("snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	if (of_property_read_u32_array(np, prot, &(freq[0]), MEDIA_DVFS_VOLT_LEVEL)) {
		clkerr("%s node doesn't have %s\n", np->name, prot);
		return -EINVAL;
	}

	for (i = 0; i < MEDIA_DVFS_VOLT_LEVEL; i++) {
		if (freq[i] > (ULONG_MAX / FREQ_CONVERSION_COEFF)) {
			clkerr("mediabus rate %u too large!\n", freq[i]);
			return -EINVAL;
		}
		mediabus->sensitive_freq[clk_id][i] =
		    ((unsigned long)freq[i]) * FREQ_CONVERSION_COEFF;
	}

	ret = snprintf(prot, sizeof(prot), "dm%u_clock", clk_id);
	if (ret <= 0) {
		clkerr("snprintf failed! ret=%d\n", ret);
		return -EINVAL;
	}

	mediabus->bus_clk[clk_id] = devm_clk_get(dev, prot);
	if (IS_ERR_OR_NULL(mediabus->bus_clk[clk_id])) {
		clkerr("dev %s can not get %s!\n", dev_name(dev), prot);
		return -EINVAL;
	}

	return 0;
}

static int clock_mediabus_init(struct device *dev)
{
	const struct device_node *np = dev->of_node;
	struct clock_mediabus *mediabus = NULL;
	unsigned int mediabus_id, i;
	int ret;

	mediabus = devm_kzalloc(dev, sizeof(*mediabus), GFP_KERNEL);
	if (!mediabus)
		return -ENOMEM;

	if (of_property_read_u32(np, "xring,mediabus_id", &mediabus_id)) {
		clkerr("%s node doesn't have %s\n", np->name, "xring,mediabus_id");
		return -EINVAL;
	}
	if (mediabus_id >= MEDIABUS_MAX) {
		clkerr("mediabus id illegal %u\n", mediabus_id);
		return -EINVAL;
	}

	if (of_property_read_u32(np, "xring,link_clock_num", &(mediabus->link_clk_num))) {
		clkerr("%s node doesn't have %s\n", np->name, "xring,link_clock_num");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "xring,mediabus-lowtemp-volt-level",
				 &mediabus->lowtemp_volt_level)) {
		clkerr("%s node doesn't have %s\n", np->name,
		       "xring,mediabus-lowtemp-freq");
		return -EINVAL;
	}

	for (i = 0; i < mediabus->link_clk_num; i++) {
		ret = link_clock_per_init(dev, mediabus, i);
		if (ret)
			return ret;
	}

	mediabus->bus_type = mediabus_id;
	mediabus->volt_level = MEDIA_VOLT_0;
	INIT_LIST_HEAD(&(mediabus->mediabus_list));
	g_mediabus_cfg[mediabus_id] = mediabus;

	return 0;
}

static int media_dvs_irq_register(struct device *dev)
{
	struct device_node *np = dev->of_node;
	int ret;

	g_dvs_cfg->dvs_timeout_irq = irq_of_parse_and_map(np, 0);

	ret = devm_request_irq(dev, g_dvs_cfg->dvs_timeout_irq, dvs_timeout_interrupt_handle,
		IRQF_SHARED, "dvs_media_intr", (void *)(XR_CLK_DVSMEDIA));
	if (ret)
		clkerr("request media irq failed, ret %d\n", ret);

	return ret;
}

static int media_dvs_config_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	unsigned int dvs_cfg[ADDR_CFG_MAX] = { 0 };
	int ret;

	g_dvs_cfg = devm_kzalloc(dev, sizeof(*g_dvs_cfg), GFP_KERNEL);
	if (!g_dvs_cfg)
		return -ENOMEM;

	ret = dvs_get_base(np, &(g_dvs_cfg->reg_base));
	if (ret)
		return ret;

	if (of_property_read_u32_array(np, "xring,lowtemp-reg", &dvs_cfg[0],
				       ADDR_CFG_MAX)) {
		clkerr("%s node doesn't have %s\n", np->name,
		       "xring,lowtemp-reg");
		return -EINVAL;
	}
	g_dvs_cfg->lowtemp_offset = dvs_cfg[ADDR_OFFSET];
	g_dvs_cfg->lowtemp_mask = dvs_cfg[ADDR_MASK];
	g_dvs_cfg->lowtemp_shift = dvs_cfg[ADDR_SHIFT];

	ret = media_dvs_irq_register(dev);

	return ret;
}

static unsigned int media_dvs_get_fixed_volt(void)
{
	unsigned int fix_volt = MEDIA_FIXED_VOLT_NONE;

	if (!IS_ENABLED(CONFIG_XRING_CLK_DEBUG))
		return fix_volt;

	fix_volt = readl(g_dvs_mntn->reg_base + g_dvs_mntn->fixed_volt_offset);
	fix_volt &= g_dvs_mntn->fixed_volt_mask;
	fix_volt >>= g_dvs_mntn->fixed_volt_shift;

	return fix_volt;
}

static int media_dvs_mntn_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	unsigned int dvs_cfg[ADDR_CFG_MAX] = { 0 };
	int ret;

	g_dvs_mntn = devm_kzalloc(dev, sizeof(*g_dvs_mntn), GFP_KERNEL);
	if (!g_dvs_mntn)
		return -ENOMEM;

	ret = dvs_get_base(np, &(g_dvs_mntn->reg_base));
	if (ret)
		return ret;

	if (of_property_read_u32_array(np, "xring,fixed-volt-reg", &dvs_cfg[0],
				       ADDR_CFG_MAX)) {
		clkerr("%s node doesn't have xring,fixed-volt-reg\n", np->name);
		return -EINVAL;
	}

	g_dvs_mntn->fixed_volt_offset = dvs_cfg[ADDR_OFFSET];
	g_dvs_mntn->fixed_volt_mask = dvs_cfg[ADDR_MASK];
	g_dvs_mntn->fixed_volt_shift = dvs_cfg[ADDR_SHIFT];
	g_dvs_mntn->volt_level_fixed = media_dvs_get_fixed_volt();

	return 0;
}

static int media_volt_table_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	unsigned int array[MEDIA_VOLT_TABLE_ITEM_NUM] = { 0 };
	unsigned int bias[MEDIA_DVFS_VOLT_LEVEL] = { 0 };
	unsigned int i, j, temp;

	if (of_property_read_u32_array(np, "opp-millivolt", &array[0],
				       MEDIA_VOLT_TABLE_ITEM_NUM)) {
		clkerr("media dvfs missing opp-millivolt!\n");
		return 0;
	}

	if (of_property_read_u32_array(np, "opp-millivolt-bias", &bias[0],
				       MEDIA_DVFS_VOLT_LEVEL))
		clkwarn("media dvfs missing opp-millivolt-bias!\n");

	for (i = MEDIA_MIN_VOTE_ID; i <= MEDIA_MAX_VOTE_ID; i++) {
		for (j = 0; j < MEDIA_DVFS_VOLT_LEVEL; j++) {
			temp = (i - MEDIA_MIN_VOTE_ID) * MEDIA_DVFS_VOLT_LEVEL + j;
			if (temp >= MEDIA_VOLT_TABLE_ITEM_NUM) {
				clkerr("error media volt, temp %u, item num %u\n",
					temp, MEDIA_VOLT_TABLE_ITEM_NUM);
				continue;
			}
			g_media_volt_table[i][j] = array[temp] + bias[j];
		}
	}

	clkinfo("media volt table init succ!\n");
	return 0;
}

static int media_dvs_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	enum media_dvs_dev type = (enum media_dvs_dev)of_device_get_match_data(dev);

	switch (type) {
	case DVS_FEA:
		ret = media_volt_table_init(dev);
		break;
	case DVS_CTRL:
		ret = media_dvs_config_init(dev);
		break;
	case DVS_MEDIA1BUS:
	case DVS_MEDIA2BUS:
		ret = clock_mediabus_init(dev);
		break;
	case DVS_CHANNEL:
		ret = media_dvs_channel_init(dev);
		break;
	case DVS_MNTN:
		ret = media_dvs_mntn_init(dev);
		break;
	case DVS_VOTE:
		ret = media_dvs_vote_init(dev);
		break;
	default:
		clkerr("invalid dvs type, %u\n", type);
		return -EINVAL;
	}
	return ret;
}

static int media_dvs_remove(struct platform_device *pdev)
{
	clkinfo("Media DVS Remove!\n");

	return 0;
}

static void media_dvs_global_config_show(struct seq_file *s)
{
	seq_puts(s, "\n<---- Show Global MediaDVS Configure START --->\n");
	seq_printf(s, "  Media DVS ON/OFF     : %u\n", is_media_dvfs_ft_on());
	seq_printf(s, "  Low Temp Mask        : 0x%x shift: %u offset: 0x%x\n",
		   g_dvs_cfg->lowtemp_mask, g_dvs_cfg->lowtemp_shift,
		   g_dvs_cfg->lowtemp_offset);
	seq_printf(s, "  Low Temp Flag        : %u\n", media_dvs_is_low_temperature());
	seq_printf(s, "  Fixed Volt Flag      : %u\n",
		   g_dvs_mntn->volt_level_fixed);
	seq_puts(s, "<---- Show Global MediaDVS Configure END --->\n");
}

static void media_dvs_vote_show(struct seq_file *s, struct dvfs_volt_vote *dvfs_vote)
{
	if (!dvfs_vote)
		return;

	seq_puts(s, "    ------- MDV --------\n");
	seq_printf(s, "    Name         : %s\n", dvfs_vote->name);
	seq_printf(s, "    dev_id       : %u\n", dvfs_vote->dev_id);
	seq_printf(s, "    channel      : %u\n", dvfs_vote->channel_id);
	seq_printf(s, "    Volt         : %u\n", dvfs_vote->volt_level);
	seq_printf(s, "    Bus id       : %u\n", dvfs_vote->mediabus_id);
}

static void media_dvs_mediabus_show(struct seq_file *s, unsigned int bus_id)
{
	struct dvfs_volt_vote *dvfs_vote = NULL;
	struct clock_mediabus *bus = NULL;
	int i, j;

	if (bus_id >= MEDIABUS_MAX)
		return;
	bus = g_mediabus_cfg[bus_id];
	if (!bus) {
		seq_printf(s, "ERROR: MediaDVS Mediabus[%u] is NONE!\n", bus_id);
		return;
	}

	seq_printf(s, "\n<---- Show MediaDVS Mediabus[%u] State START --->\n",
		   bus->bus_type);
	seq_printf(s, "  Mediabus Vote Level  : %u\n", bus->volt_level);
	seq_printf(s, "  Mediabus LowTemp     : %u\n", bus->lowtemp_volt_level);
	for (i = 0; i < bus->link_clk_num; i++)
		seq_printf(s, "  Mediabus%u Rate       : %lu\n", i, clk_get_rate(bus->bus_clk[i]));

	seq_puts(s, "  Mediabus    Sensitive Rate:\n");
	for (i = 0; i < bus->link_clk_num; i++) {
		for (j = 0; j < MEDIA_DVFS_VOLT_LEVEL; j++)
			seq_printf(s, "    [Level_%d] %12lu", j, bus->sensitive_freq[i][j]);
		seq_puts(s, "\n");
	}
	seq_puts(s, "  Mediabus MDV lists     :\n");
	list_for_each_entry(dvfs_vote, &bus->mediabus_list, mediabus_node)
		media_dvs_vote_show(s, dvfs_vote);
	seq_printf(s, "<---- Show MediaDVS Mediabus[%u] State END --->\n",
		   bus->bus_type);
}

static void media_dvs_channel_state_show(struct seq_file *s,
					 unsigned int chan)
{
	struct dvs_channel *dvs_chan = NULL;
	struct dvfs_volt_vote *dvfs_vote = NULL;

	if (chan >= g_max_chan_num)
		return;
	dvs_chan = g_dvs_channels[chan];
	if (!dvs_chan) {
		seq_printf(s, "ERROR: MediaDVS Channel[%u] is NONE!\n", chan);
		return;
	}

	seq_printf(s, "\n<---- Show MediaDVS Channel[%u] State START --->\n",
		   dvs_chan->channel_id);
	seq_printf(s, "  volt Lvl          : %u\n", dvs_chan->volt_level);
	seq_puts(s, "  Channel MDV lists   :\n");
	list_for_each_entry(dvfs_vote, &dvs_chan->chan_list, chan_node)
		media_dvs_vote_show(s, dvfs_vote);
	seq_printf(s, "<---- Show MediaDVS Channel[%u] State END --->\n",
		   dvs_chan->channel_id);
}

static int media_dvs_summary_show(struct seq_file *s, void *data)
{
	unsigned int i;

	if (!s || !g_dvs_cfg)
		return -EINVAL;

	media_dvs_global_config_show(s);
	for (i = 0; i < MEDIABUS_MAX; i++)
		media_dvs_mediabus_show(s, i);
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
		media_dvs_channel_state_show(s, i);

	return 0;
}

static int media_dvs_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, media_dvs_summary_show, inode->i_private);
}

static const struct file_operations media_dvs_summary_fops = {
	.open = media_dvs_summary_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void media_dvs_debug_init(void)
{
	struct dentry *dvs_parent = NULL;
	struct dentry *dvs_info = NULL;

	if (!IS_ENABLED(CONFIG_XRING_CLK_DEBUG)) {
		clkinfo("Clock Debug Close!\n");
		return;
	}

	dvs_parent = debugfs_create_dir("media_dvs", NULL);
	if (!dvs_parent) {
		clkerr("create media_dvs debugfs dir failed\n");
		return;
	}

	dvs_info = debugfs_create_file("dvs_info", 0640, dvs_parent,
				       NULL, &media_dvs_summary_fops);
	if (!dvs_info) {
		clkerr("create dvs_info debugfs file failed\n");
		return;
	}
}

static const struct of_device_id media_dvs_of_match[] = {
	{ .compatible = "xring,media-dvfs-fea", .data = (void *)DVS_FEA },
	{ .compatible = "xring,media-dvs-ctrl", .data = (void *)DVS_CTRL },
	{ .compatible = "xring,media-dvs-media1bus", .data = (void *)DVS_MEDIA1BUS },
	{ .compatible = "xring,media-dvs-media2bus", .data = (void *)DVS_MEDIA2BUS },
	{ .compatible = "xring,media-dvs-channel", .data = (void *)DVS_CHANNEL },
	{ .compatible = "xring,media-dvs-mntn", .data = (void *)DVS_MNTN },
	{ .compatible = "xring,media-dvs-vote", .data = (void *)DVS_VOTE },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, media_dvs_of_match);

static struct platform_driver media_hw_dvs_driver = {
	.probe = media_dvs_probe,
	.remove = media_dvs_remove,
	.driver = {
		.name = "mediadvs",
		.of_match_table = of_match_ptr(media_dvs_of_match),
		.suppress_bind_attrs = true,
	},
};
module_platform_driver(media_hw_dvs_driver);

MODULE_SOFTDEP("pre: xr-clk");
MODULE_AUTHOR("Shaobo Zheng <zhengshaobo1@xiaomi.com>");
MODULE_DESCRIPTION("XRing Media HW DVS driver");
MODULE_LICENSE("GPL v2");
