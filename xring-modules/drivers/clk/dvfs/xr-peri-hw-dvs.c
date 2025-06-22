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
#include "xr-peri-hw-dvs.h"
#include "clk/xr-clk-common.h"
#include "clk/xr-dvfs-private.h"
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
#include <soc/xring/vote_mng.h>
#include <soc/xring/xr-clk-provider.h>

enum peri_dvs_dev {
	DVS_FEA,
	DVS_CTRL,
	DVS_CHANNEL,
	DVS_VOTE,
	PERIBUS_PERF,
	DVS_MNTN,
};

static unsigned int g_max_chan_num;
static struct dvs_config *g_dvs_cfg;
static struct dvs_channel *g_dvs_channels[MAX_CHANNEL_NUM];
static struct dvs_mntn *g_dvs_mntn;

static unsigned int g_peri_volt_table[CLK_MAX_VOTE_ID][PERI_DVFS_VOLT_LEVEL] = {
	[CLK_OCM_VOTE_ID] = { 600, 650, 700, 800 }, /* ocm mv */
};

static int peri_dvs_is_low_temperature(void)
{
	return dvs_is_low_temperature(g_dvs_cfg);
}

static unsigned int get_volt_by_volt_level(unsigned int dev_id,
					   unsigned int level)
{
	if (dev_id >= CLK_MAX_VOTE_ID || level >= PERI_DVFS_VOLT_LEVEL) {
		clkerr("input dev_id %u, level %u err!\n", dev_id, level);
		return -EINVAL;
	}

	return g_peri_volt_table[dev_id][level];
}

static int peri_dvs_wait_completed(struct dvfs_volt_vote *vote_inst)
{
	struct dvs_hw hw = { 0 };

	if (vote_inst->channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote channel id %u illegal!\n", vote_inst->channel_id);
		return -EINVAL;
	}

	hw.base = XR_CLK_DVSPERI;
	hw.ch = vote_inst->channel_id;
	return dvs_wait_ack(&hw);
}

static unsigned int peri_dvs_get_volt(struct dvfs_volt_vote *vote_inst)
{
	struct dvs_hw hw = { 0 };
	uint8_t dac_code;
	unsigned int volt;
	int ret;

	if (unlikely(!vote_inst || !g_dvs_mntn))
		return -EINVAL;

	if (unlikely(g_dvs_mntn->volt_level_fixed != PERI_FIXED_VOLT_NONE))
		return g_dvs_mntn->volt_level_fixed - 1;

	if (vote_inst->channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote channel id %u illegal!\n", vote_inst->channel_id);
		return -EINVAL;
	}

	hw.base = XR_CLK_DVSPERI;
	hw.ch = vote_inst->channel_id;
	ret = dvs_get_cur_volt(&hw, &dac_code);
	if (ret) {
		clkerr("peri dvs get volt fail %d!", ret);
		return 0;
	}

	volt = dac_code;

	return ((volt * MPMIC_DAC_CODE_VOLT_STEP) + MPMIC_DAC_CODE_VOLT_BASE);
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

	dac_code = (uint8_t)DIV_ROUND_UP(voltage - MPMIC_DAC_CODE_VOLT_BASE,
		MPMIC_DAC_CODE_VOLT_STEP);

	hw.base = XR_CLK_DVSPERI;
	hw.ch = vote_inst->channel_id;

	ret = dvs_write_no_wait_ack(&hw, dac_code);
	if (ret) {
		clkerr("dvs write async fail %d\n", ret);
		return ret;
	}
	/*
	 * When the system enters doze, the dvs will not have a clock, causing it
	 * to work abnormally. Therefore, it is necessary to ensure that
	 * the system has a chance to enter doze after the dvs execution is completed.
	 */
#if IS_ENABLED(CONFIG_XRING_DOZE)
	ret = dvs_wait_ack(&hw);
	if (ret) {
		clkerr("dvs wait ack fail %d!\n", ret);
		return ret;
	}
#endif
	clkinfo("set peri dvs hw, channel %u set to %u, volt_level %u!\n",
		chan->channel_id, voltage, volt_level);

	return 0;
}

static int __peri_dvs_set_volt(struct dvfs_volt_vote *vote_inst)
{
	struct dvfs_volt_vote *inst = NULL;
	struct dvs_channel *dvs_channel = NULL;
	unsigned int channel_id, cur_volt_level;
	unsigned int target_volt_level = PERI_VOLT_0;
	int ret;

	channel_id = vote_inst->channel_id;
	dvs_channel = g_dvs_channels[channel_id];
	cur_volt_level = dvs_channel->volt_level;

	/* gather all vote inst on the same channel */
	list_for_each_entry(inst, &dvs_channel->chan_list, chan_node)
		target_volt_level = max_value(target_volt_level, inst->volt_level);

	/* peri volt vote flow */
	ret = dvs_set_volt_hw_cfg(dvs_channel, vote_inst, target_volt_level);
	if (ret)
		return ret;

	/* update channel volt level */
	dvs_channel->volt_level = target_volt_level;

	return 0;
}

static void peri_dvs_per_vote_record(const struct dvfs_volt_vote *vote_inst)
{
	unsigned int channel_id;
	struct dvs_channel *dvs_channel = NULL;

	channel_id = vote_inst->channel_id;
	dvs_channel = g_dvs_channels[channel_id];
	dvs_per_vote_record(dvs_channel, g_dvs_cfg);
}

static int peri_dvs_set_volt(struct dvfs_volt_vote *vote_inst,
			     unsigned int volt_level)
{
	unsigned int old_volt_level;
	int ret;

	if (unlikely(!vote_inst || !g_dvs_mntn))
		return -EINVAL;

	/* fixed volt case */
	if ((g_dvs_mntn->volt_level_fixed != PERI_FIXED_VOLT_NONE) &&
	    (volt_level + 1 > g_dvs_mntn->volt_level_fixed)) {
		clkerr("can not set %u bigger than volt_level_fixed %u\n",
		       volt_level, g_dvs_mntn->volt_level_fixed - 1);
		return -EFIXED_VOLT;
	}

	old_volt_level = vote_inst->volt_level;
	vote_inst->volt_level = volt_level;
	ret = __peri_dvs_set_volt(vote_inst);
	if (ret) {
		clkerr("peri dvs set volt_level %u fail, ret %d!\n",
		       volt_level, ret);
		vote_inst->volt_level = old_volt_level;
		return ret;
	}

	peri_dvs_per_vote_record(vote_inst);

	return ret;
}

static struct dvfs_vote_ops dvs_ops = {
	.get_volt = peri_dvs_get_volt,
	.set_volt = peri_dvs_set_volt,
	.wait_completed = peri_dvs_wait_completed,
	.is_low_temperature = peri_dvs_is_low_temperature,
	.get_volt_by_level = get_volt_by_volt_level,
};

static int dvfs_vote_dt_parse(const struct device_node *np,
			      struct dvfs_volt_vote *vote_inst)
{
	if (of_property_read_string(np, "perivote-output-names",
				    &vote_inst->name)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "perivote-output-names");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "perivote-poll-id",
				 &vote_inst->dev_id)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "perivote-poll-id");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "perivote-poll-channel",
				 &vote_inst->channel_id)) {
		clkerr("%s node doesn't have %s\n",
		       np->name, "perivote-poll-channel");
		return -EINVAL;
	}

	return 0;
}

static int dvfs_vote_node_init(struct device *dev,
			       const struct device_node *np)
{
	struct dvfs_volt_vote *vote_inst = NULL;
	unsigned int channel_id;
	int ret;

	vote_inst = devm_kzalloc(dev, sizeof(*vote_inst), GFP_KERNEL);
	if (vote_inst == NULL)
		return -ENOMEM;

	ret = dvfs_vote_dt_parse(np, vote_inst);
	if (ret)
		return ret;

	vote_inst->volt_level = PERI_VOLT_0;
	vote_inst->ops = &dvs_ops;

	channel_id = vote_inst->channel_id;
	if (channel_id >= MAX_CHANNEL_NUM) {
		clkerr("dvfs vote have a invalid channel id %u!\n", channel_id);
		return -EINVAL;
	}
	list_add(&vote_inst->chan_node,
		 &(g_dvs_channels[channel_id]->chan_list));

	ret = dvfs_vote_register(vote_inst);
	if (ret)
		clkerr("dvfs vote register failed, %d!\n", ret);

	return ret;
}

static void peri_dvs_debug_init(void);

static int peri_dvs_vote_init(struct device *dev)
{
	const struct device_node *np = dev->of_node;
	struct device_node *node = NULL;
	int ret = 0;

	for_each_child_of_node(np, node) {
		ret = dvfs_vote_node_init(dev, node);
		if (ret < 0)
			return ret;
	}

	peri_dvs_debug_init();

	return ret;
}

static int peri_dvs_per_channel_init(struct device *dev,
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

	dvs_channel->volt_level = PERI_VOLT_0; // todo
	return 0;
}

static int peri_dvs_channel_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	void __iomem *reg_base = NULL;
	unsigned int i;
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
		ret = peri_dvs_per_channel_init(dev, reg_base, i);
		if (ret < 0) {
			clkerr("peri dvs channel init fail, chan id %u\n", i);
			return ret;
		}
	}

	return ret;
}

static int peri_dvs_irq_register(struct device *dev)
{
	struct device_node *np = dev->of_node;
	int ret;

	g_dvs_cfg->dvs_timeout_irq = irq_of_parse_and_map(np, 0);

	ret = devm_request_irq(dev, g_dvs_cfg->dvs_timeout_irq, dvs_timeout_interrupt_handle,
		IRQF_SHARED, "dvs_peri_intr", (void *)(XR_CLK_DVSPERI));
	if (ret)
		clkerr("request peri irq failed, ret %d\n", ret);

	return ret;
}

static int peri_dvs_config_init(struct device *dev)
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

	ret = peri_dvs_irq_register(dev);

	return ret;
}

static unsigned int peri_dvs_get_fixed_volt(void)
{
	unsigned int fix_volt = PERI_FIXED_VOLT_NONE;

	if (!IS_ENABLED(CONFIG_XRING_CLK_DEBUG))
		return fix_volt;

	fix_volt = readl(g_dvs_mntn->reg_base + g_dvs_mntn->fixed_volt_offset);
	fix_volt &= g_dvs_mntn->fixed_volt_mask;
	fix_volt >>= g_dvs_mntn->fixed_volt_shift;

	return fix_volt;
}

static int peri_dvs_mntn_init(struct device *dev)
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
		clkerr("%s node doesn't have %s\n", np->name,
		       "xring,fixed-volt-reg");
		return -EINVAL;
	}
	g_dvs_mntn->fixed_volt_offset = dvs_cfg[ADDR_OFFSET];
	g_dvs_mntn->fixed_volt_mask = dvs_cfg[ADDR_MASK];
	g_dvs_mntn->fixed_volt_shift = dvs_cfg[ADDR_SHIFT];

	g_dvs_mntn->volt_level_fixed = peri_dvs_get_fixed_volt();

	return 0;
}

static int peri_volt_table_init(struct device *dev)
{
	struct device_node *np = dev->of_node;
	unsigned int array[PERI_VOLT_TABLE_ITEM_NUM] = { 0 };
	unsigned int bias[PERI_VOLT_TABLE_ITEM_NUM] = { 0 };
	unsigned int i, j, temp;

	if (of_property_read_u32_array(np, "opp-millivolt", &array[0],
				       PERI_VOLT_TABLE_ITEM_NUM)) {
		clkerr("peri dvfs missing opp-millivolt!\n");
		return 0;
	}

	if (of_property_read_u32_array(np, "opp-millivolt-bias", &bias[0],
				       PERI_VOLT_TABLE_ITEM_NUM))
		clkwarn("peri dvfs missing opp-millivolt-bias!\n");

	for (i = PERI_MIN_VOTE_ID; i <= PERI_MAX_VOTE_ID; i++) {
		for (j = 0; j < PERI_DVFS_VOLT_LEVEL; j++) {
			temp = (i - PERI_MIN_VOTE_ID) * PERI_DVFS_VOLT_LEVEL + j;
			if (temp >= PERI_VOLT_TABLE_ITEM_NUM) {
				clkerr("error peri volt, temp %u, item num %u\n",
					temp, PERI_VOLT_TABLE_ITEM_NUM);
				continue;
			}
			g_peri_volt_table[i][j] = array[temp] + bias[j];
		}
	}

	clkinfo("peri volt table init succ!\n");
	return 0;
}

struct peribus_desc {
	unsigned int ch;
	struct vote_mng *g_peribus_vote_mng;
};

static struct peribus_desc g_peribus_desc;

static int peribus_perf_vote_init(struct device *dev)
{
	g_peribus_desc.g_peribus_vote_mng = vote_mng_get(dev, "m2_mng_peribus");
	if (!g_peribus_desc.g_peribus_vote_mng) {
		dev_err(dev, "get vote mng failed\n");
		return -ENOMEM;
	}

	if (of_property_read_u32(dev->of_node, "vote-mng-ch", &(g_peribus_desc.ch))) {
		clkerr("%s node doesn't have %s\n", dev->of_node->name, "vote-mng-ch");
		return -EINVAL;
	}
	clkinfo("peribus perf vote init succ!\n");
	return 0;
}

static void peribus_perf_vote_deinit(struct device *dev)
{
	if (g_peribus_desc.g_peribus_vote_mng)
		vote_mng_put(dev, g_peribus_desc.g_peribus_vote_mng);

	g_peribus_desc.g_peribus_vote_mng = NULL;
}

int peribus_set_rate(unsigned int rate)
{
	int ret;

	if (rate >= PERI_MAINBUS_RATE_MAX) {
		clkerr("input param err, %u\n", rate);
		return -EINVAL;
	}

	if (!g_peribus_desc.g_peribus_vote_mng) {
		clkerr("peribus vote_mng2 do not init!");
		return -EINVAL;
	}

	ret = vote_mng_vote_min(g_peribus_desc.g_peribus_vote_mng,
				g_peribus_desc.ch, rate);
	if (ret)
		clkerr("vote mng vote min peribus %u fail, ret %d\n", rate, ret);

	clkinfo("peribus set rate %u succ, ch %u!\n", rate, g_peribus_desc.ch);
	return ret;
}
EXPORT_SYMBOL_GPL(peribus_set_rate);

static int peri_dvs_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	enum peri_dvs_dev type = (enum peri_dvs_dev)of_device_get_match_data(dev);

	switch (type) {
	case DVS_FEA:
		ret = peri_volt_table_init(dev);
		break;
	case DVS_CTRL:
		ret = peri_dvs_config_init(dev);
		break;
	case DVS_CHANNEL:
		ret = peri_dvs_channel_init(dev);
		break;
	case DVS_MNTN:
		ret = peri_dvs_mntn_init(dev);
		break;
	case DVS_VOTE:
		ret = peri_dvs_vote_init(dev);
		break;
	case PERIBUS_PERF:
		ret = peribus_perf_vote_init(dev);
		break;
	default:
		clkerr("invalid dvs type, %u\n", type);
		return -EINVAL;
	}

	return ret;
}

static int peri_dvs_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	enum peri_dvs_dev type = (enum peri_dvs_dev)of_device_get_match_data(dev);

	switch (type) {
	case PERIBUS_PERF:
		peribus_perf_vote_deinit(dev);
		break;
	default:
		clkerr("invalid dvs type, %u\n", type);
		return -EINVAL;
	}

	clkinfo("peri DVS Remove!\n");

	return 0;
}

static void peri_dvs_global_config_show(struct seq_file *s)
{
	seq_puts(s, "\n<---- Show Global PeriDVS Configure START --->\n");
	seq_printf(s, "  Peri DVS ON/OFF     : %u\n", is_peri_dvfs_ft_on());
	seq_printf(s, "  Low Temp Mask        : 0x%x shift: %u offset: 0x%x\n",
		   g_dvs_cfg->lowtemp_mask, g_dvs_cfg->lowtemp_shift,
		   g_dvs_cfg->lowtemp_offset);
	seq_printf(s, "  Low Temp Flag        : %u\n", peri_dvs_is_low_temperature());
	seq_printf(s, "  Lock Volt Mask       : 0x%x shift: %u offset: 0x%x\n",
		   g_dvs_mntn->fixed_volt_mask, g_dvs_mntn->fixed_volt_shift,
		   g_dvs_mntn->fixed_volt_offset);
	seq_printf(s, "  Fixed Volt Flag      : %u\n",
		   g_dvs_mntn->volt_level_fixed);
	seq_puts(s, "<---- Show Global PeriDVS Configure END --->\n");
}

static void peri_dvs_vote_show(struct seq_file *s, struct dvfs_volt_vote *dvfs_vote)
{
	if (!dvfs_vote)
		return;

	seq_puts(s, "    ------- MDV --------\n");
	seq_printf(s, "    Name         : %s\n", dvfs_vote->name);
	seq_printf(s, "    dev_id       : %u\n", dvfs_vote->dev_id);
	seq_printf(s, "    channel      : %u\n", dvfs_vote->channel_id);
	seq_printf(s, "    Volt         : %u\n", dvfs_vote->volt_level);
}

static void peri_dvs_channel_state_show(struct seq_file *s,
					unsigned int chan)
{
	struct dvs_channel *dvs_chan = NULL;
	struct dvfs_volt_vote *dvfs_vote = NULL;

	if (chan >= g_max_chan_num)
		return;
	dvs_chan = g_dvs_channels[chan];
	if (!dvs_chan) {
		seq_printf(s, "ERROR: PeriDVS Channel[%u] is NONE!\n", chan);
		return;
	}

	seq_printf(s, "\n<---- Show PeriDVS Channel[%u] State START --->\n",
		   dvs_chan->channel_id);
	seq_printf(s, "  volt Lvl          : %u\n", dvs_chan->volt_level);
	seq_puts(s, "  Channel MDV lists   :\n");
	list_for_each_entry(dvfs_vote, &dvs_chan->chan_list, chan_node)
		peri_dvs_vote_show(s, dvfs_vote);
	seq_printf(s, "<---- Show PeriDVS Channel[%u] State END --->\n",
		   dvs_chan->channel_id);
}

static int peri_dvs_summary_show(struct seq_file *s, void *data)
{
	unsigned int i;

	if (!s || !g_dvs_cfg)
		return -EINVAL;

	peri_dvs_global_config_show(s);
	for (i = 0; i < MAX_CHANNEL_NUM; i++)
		peri_dvs_channel_state_show(s, i);

	return 0;
}

static int peri_dvs_summary_open(struct inode *inode, struct file *file)
{
	return single_open(file, peri_dvs_summary_show, inode->i_private);
}

ssize_t peri_dvs_summary_write(struct file *filp, const char __user *ubuf,
			       size_t cnt, loff_t *ppos)
{
	char inputstr[10] = { 0 };
	loff_t buff_pos = 0;
	ssize_t byte_writen;
	int val, ret;

	byte_writen = simple_write_to_buffer(inputstr, sizeof(inputstr),
					     &buff_pos, ubuf, cnt);
	if (byte_writen != cnt) {
		clkerr("copy from user fail, byte_writen = %zd\n", byte_writen);
		return -ENOMEM;
	}
	ret = kstrtoint(inputstr, 0, &val);
	if (ret) {
		clkerr("please input clk rate!\n");
		return -EINVAL;
	}
	ret = peribus_set_rate(val);
	if (ret)
		clkerr("peribus set rat %d fail, ret %d!\n", val, ret);

	return cnt;
}

static const struct file_operations peri_dvs_summary_fops = {
	.open = peri_dvs_summary_open,
	.write = peri_dvs_summary_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void peri_dvs_debug_init(void)
{
	struct dentry *dvs_parent = NULL;
	struct dentry *dvs_info = NULL;

	if (!IS_ENABLED(CONFIG_XRING_CLK_DEBUG)) {
		clkinfo("Clock Debug Close!\n");
		return;
	}

	dvs_parent = debugfs_create_dir("peri_dvs", NULL);
	if (!dvs_parent) {
		clkerr("create peri_dvs debugfs dir failed\n");
		return;
	}

	dvs_info = debugfs_create_file("dvs_info", 0640, dvs_parent,
				       NULL, &peri_dvs_summary_fops);
	if (!dvs_info) {
		clkerr("create dvs_info debugfs file failed\n");
		return;
	}
}

static const struct of_device_id peri_dvs_of_match[] = {
	{ .compatible = "xring,peri-dvfs-fea", .data = (void *)DVS_FEA },
	{ .compatible = "xring,peri-dvs-ctrl", .data = (void *)DVS_CTRL },
	{ .compatible = "xring,peri-dvs-channel", .data = (void *)DVS_CHANNEL },
	{ .compatible = "xring,peri-dvs-mntn", .data = (void *)DVS_MNTN },
	{ .compatible = "xring,peri-dvs-vote", .data = (void *)DVS_VOTE },
	{ .compatible = "xring,peribus-perf-vote", .data = (void *)PERIBUS_PERF },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, peri_dvs_of_match);

static struct platform_driver peri_hw_dvs_driver = {
	.probe = peri_dvs_probe,
	.remove = peri_dvs_remove,
	.driver = {
		.name = "peridvs",
		.of_match_table = of_match_ptr(peri_dvs_of_match),
		.suppress_bind_attrs = true,
	},
};

module_platform_driver(peri_hw_dvs_driver);

MODULE_SOFTDEP("pre: xr-clk");
MODULE_AUTHOR("Shaobo Zheng <zhengshaobo1@xiaomi.com>");
MODULE_DESCRIPTION("XRing Peri HW DVS driver");
MODULE_LICENSE("GPL v2");
