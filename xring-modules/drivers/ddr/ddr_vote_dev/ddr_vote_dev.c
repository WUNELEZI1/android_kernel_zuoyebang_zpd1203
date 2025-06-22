// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/units.h>
#include <linux/miscdevice.h>
#include <linux/compiler_attributes.h>
#include <trace/events/power.h>
#include <soc/xring/vote_mng.h>
#include <soc/xring/ddr_vote_dev.h>
#include <soc/xring/perf_actuator.h>
#include <ddr_dvfs_vote.h>
#include <ddr_spec_define.h>

#define VOTE_DATA_WORD_SIZE	2
#define DDR_BD_PERCENT		100
#define TRACE_NAME_LEN		30
#define CMD_GET_DDR_VOTE_DEV	PERF_R_CMD(GET_DDR_VOTE_DEV, struct ddr_vote_dev_info)
#undef pr_fmt
#define pr_fmt(fmt)		"ddr_vote_dev: " fmt

struct ddr_vote_dev_data {
	unsigned int bytes_per_sec_per_bps;
	unsigned int bd_utilization;
	struct vote_mng *vote_mng;
	bool is_initialized;
};

struct ddr_vote_dev_info {
	unsigned int vote_value;
};

static struct ddr_vote_dev_data g_ddr_vote_dev_data;

static void trace_ddr_vote_dev(char *type, unsigned int ch, unsigned int freq_mbps)
{
	char name[TRACE_NAME_LEN] = {0};

	snprintf(name, TRACE_NAME_LEN, "ddr_vote_dev_%s_ch%u", type, ch);
	trace_clock_set_rate(name, freq_mbps, 0);
}

int ddr_vote_dev_request(unsigned int vote_ch, unsigned int vote_type, unsigned int vote_val)
{
	int ret;
	unsigned int freq_mbps;

	if (!g_ddr_vote_dev_data.is_initialized) {
		pr_err("not initialized,ch=%u,type=%u,val=%u\n", vote_ch, vote_type, vote_val);
		return -EINVAL;
	}

	if (vote_ch >= DDR_DVFS_HW_VOTE_MAX) {
		pr_err("ch is wrong, should be smaller than %u,ch=%u,type=%u,val=%u\n",
			DDR_DVFS_HW_VOTE_MAX, vote_ch, vote_type, vote_val);
		return -EINVAL;
	}
	if (vote_type >= VOTE_TYPE_END || vote_type <= VOTE_TYPE_START) {
		pr_err("type is wrong,ch=%u,type=%u,val=%u\n", vote_ch, vote_type, vote_val);
		return -EINVAL;
	}

	if (vote_type >= VOTE_MIN_BANDWIDTH_MB_S && vote_type <= VOTE_MAX_BANDWIDTH_MB_S)
		freq_mbps = DIV_ROUND_UP(DIV_ROUND_UP((vote_val * DDR_BD_PERCENT),
			g_ddr_vote_dev_data.bd_utilization),
			g_ddr_vote_dev_data.bytes_per_sec_per_bps);
	else if (vote_type >= VOTE_MIN_FREQ_MHZ && vote_type <= VOTE_MAX_FREQ_MHZ)
		freq_mbps = DDR_FREQ_TO_RATE(vote_val);
	else
		freq_mbps = vote_val;

	if (freq_mbps > VOTE_MAX_VALUE) {
		pr_err("val is too large,ch=%u,type=%u,val=%u\n", vote_ch, vote_type, vote_val);
		return -EINVAL;
	}

	switch (vote_type) {
	case VOTE_MIN_BANDWIDTH_MB_S:
	case VOTE_MIN_FREQ_MHZ:
	case VOTE_MIN_DATA_RATE_MBPS:
		ret = vote_mng_vote_min(g_ddr_vote_dev_data.vote_mng, vote_ch, freq_mbps);
		trace_ddr_vote_dev("min", vote_ch, freq_mbps);
		break;
	case VOTE_DMD_BANDWIDTH_MB_S:
	case VOTE_DMD_FREQ_MHZ:
	case VOTE_DMD_DATA_RATE_MBPS:
		ret = vote_mng_vote_dmd(g_ddr_vote_dev_data.vote_mng, vote_ch, freq_mbps);
		trace_ddr_vote_dev("dmd", vote_ch, freq_mbps);
		break;
	case VOTE_MAX_BANDWIDTH_MB_S:
	case VOTE_MAX_FREQ_MHZ:
	case VOTE_MAX_DATA_RATE_MBPS:
		ret = vote_mng_vote_max(g_ddr_vote_dev_data.vote_mng, vote_ch, freq_mbps);
		trace_ddr_vote_dev("max", vote_ch, freq_mbps);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(ddr_vote_dev_request);

/*
 * value: output value of vote result, unit is "mbps"
 *
 * return: 0: success, <0: error
 */
static int get_ddr_vote_for_schedgenius(unsigned int *value)
{
	int ret;
	unsigned int ch, val;
	unsigned int min = 0;
	unsigned int dmd = 0;

	if (!g_ddr_vote_dev_data.is_initialized) {
		pr_err("not initialized\n");
		return -EINVAL;
	}

	if (value == NULL) {
		pr_err("value ptr is NULL\n");
		return -EFAULT;
	}

	for (ch = 0; ch < DDR_DVFS_HW_VOTE_MAX; ch++) {
		if (ch == DDR_DVFS_HW_VOTE_SCHEDGENIUS)
			continue;

		ret = vote_mng_mode2_get_min(g_ddr_vote_dev_data.vote_mng, ch, &val);
		if (ret != 0) {
			pr_err("get min err:%d\n", ret);
			return ret;
		}
		min = min > val ? min : val;

		ret = vote_mng_mode2_get_dmd(g_ddr_vote_dev_data.vote_mng, ch, &val);
		if (ret != 0) {
			pr_err("get dmd err:%d\n", ret);
			return ret;
		}
		dmd += val;
	}

	*value = min > dmd ? min : dmd;
	return 0;
}

static int perf_actuator_get_ddr_vote_dev(void __user *uarg)
{
	int ret;
	unsigned int value;
	struct ddr_vote_dev_info vote_dev_info;

	if (uarg == NULL)
		return -EINVAL;

	rcu_read_lock();
	ret = get_ddr_vote_for_schedgenius(&value);
	if (ret != 0) {
		rcu_read_unlock();
		return ret;
	}
	/* vote frequency is 1000 times the return value */
	vote_dev_info.vote_value = value * 1000;
	rcu_read_unlock();

	if (copy_to_user(uarg, &vote_dev_info, sizeof(struct ddr_vote_dev_info)))
		return -EFAULT;

	return 0;
}

static int ddr_vote_dev_open(__maybe_unused struct inode *inode, struct file *file)
{
	union ddr_vote_data *vote_data = NULL;

	if (file == NULL) {
		pr_err("open file is NULL\n");
		return -EFAULT;
	}

	vote_data = kzalloc(sizeof(union ddr_vote_data), GFP_KERNEL);
	if (vote_data == NULL)
		return -ENOMEM;

	file->private_data = vote_data;

	return 0;
}

static int ddr_vote_dev_release(__maybe_unused struct inode *inode, struct file *file)
{
	int ret;
	union ddr_vote_data *vote_data = NULL;

	if (file == NULL) {
		pr_err("release file is NULL\n");
		return -EFAULT;
	}

	vote_data = (union ddr_vote_data *)file->private_data;
	if (vote_data == NULL) {
		pr_err("release private_data is NULL\n");
		return -EFAULT;
	}

	if (vote_data->value == 0)
		return 0;

	switch (vote_data->data.vote_type) {
	case VOTE_MAX_BANDWIDTH_MB_S:
	case VOTE_MAX_FREQ_MHZ:
	case VOTE_MAX_DATA_RATE_MBPS:
		ret = ddr_vote_dev_request(vote_data->data.vote_ch,
			VOTE_MAX_DATA_RATE_MBPS, VOTE_MAX_VALUE);
		break;
	default:
		ret = ddr_vote_dev_request(vote_data->data.vote_ch,
			vote_data->data.vote_type, 0);
		break;
	}

	kfree(vote_data);
	file->private_data = NULL;

	return ret;
}

static long ddr_vote_dev_ioctrl(struct file *file, unsigned int cmd, unsigned long data)
{
	union ddr_vote_data *vote_data = NULL;

	if (file == NULL) {
		pr_err("ioctrl file is NULL\n");
		return -EFAULT;
	}

	vote_data = (union ddr_vote_data *)file->private_data;
	if (vote_data == NULL) {
		pr_err("ioctrl private_data is NULL\n");
		return -EFAULT;
	}

	if (cmd != DDR_DVFS_VOTE_IOCTRL_CMD) {
		pr_err("cmd=0x%x is wrong\n", cmd);
		return -EINVAL;
	}
	vote_data->value = data;
	return ddr_vote_dev_request(vote_data->data.vote_ch,
		vote_data->data.vote_type, vote_data->data.vote_val);
}

static const struct file_operations g_ddr_vote_dev_fops = {
	.owner = THIS_MODULE,
	.open = ddr_vote_dev_open,
	.release = ddr_vote_dev_release,
	.unlocked_ioctl = ddr_vote_dev_ioctrl,
	.llseek = no_llseek,
};

static struct miscdevice g_ddr_vote_dev_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ddr_vote_dev",
	.fops = &g_ddr_vote_dev_fops,
};

static int ddr_vote_dev_probe(struct platform_device *platform_dev)
{
	struct device *dev = &platform_dev->dev;
	struct device_node *dev_node = dev->of_node;
	int ret;

	if (dev_node == NULL) {
		dev_err(dev, "no device node\n");
		return -ENODEV;
	}

	ret = of_property_read_u32_array(dev_node, "vote_data",
		(u32 *)&g_ddr_vote_dev_data, VOTE_DATA_WORD_SIZE);
	if (ret != 0) {
		dev_err(dev, "no vote_data\n");
		return -EINVAL;
	}
	dev_info(dev, "per_bps %d, utilization %d\n",
		g_ddr_vote_dev_data.bytes_per_sec_per_bps,
		g_ddr_vote_dev_data.bd_utilization);

	g_ddr_vote_dev_data.vote_mng = vote_mng_get(dev, "vote_mng_ddr_vote_dev");
	if (g_ddr_vote_dev_data.vote_mng == NULL) {
		dev_err(dev, "get vote mng failed\n");
		return -ENXIO;
	}

	ret = misc_register(&g_ddr_vote_dev_miscdev);
	if (ret < 0) {
		dev_err(dev, "misc register fail\n");
		return ret;
	}

	register_perf_actuator(CMD_GET_DDR_VOTE_DEV, perf_actuator_get_ddr_vote_dev);

	g_ddr_vote_dev_data.is_initialized = true;
	dev_info(dev, "probe succ\n");

	return 0;
}

static int ddr_vote_dev_remove(struct platform_device *platform_dev)
{
	struct device *dev = &platform_dev->dev;

	g_ddr_vote_dev_data.is_initialized = false;
	unregister_perf_actuator(CMD_GET_DDR_VOTE_DEV);
	misc_deregister(&g_ddr_vote_dev_miscdev);
	vote_mng_put(dev, g_ddr_vote_dev_data.vote_mng);

	dev_info(dev, "remove succ\n");

	return 0;
}

static const struct of_device_id g_ddr_vote_dev_of_match[] = {
	{
		.compatible = "xring,ddr_vote_dev",
	},
	{},
};
MODULE_DEVICE_TABLE(of, g_ddr_vote_dev_of_match);

static struct platform_driver g_ddr_vote_dev_driver = {
	.probe = ddr_vote_dev_probe,
	.remove = ddr_vote_dev_remove,
	.driver = {
		.name = "ddr_vote_dev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(g_ddr_vote_dev_of_match),
	},
};

int ddr_vote_init(void)
{
	return platform_driver_register(&g_ddr_vote_dev_driver);
}

void ddr_vote_exit(void)
{
	platform_driver_unregister(&g_ddr_vote_dev_driver);
}
