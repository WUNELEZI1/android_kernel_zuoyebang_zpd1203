// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <soc/xring/vote_mng.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/vote_mng_define.h>
#include "vote_mng_internal.h"

#define VOTE_MNG_REGS_NUM	3

#define MODE2_XX_VAL_VOTE_CH_N(MODE2_XX_RESULT, CH_N) \
	(MODE2_XX_RESULT + (CH_N) * 0xc)

enum {
	VOTE_MNG_MODE1,
	VOTE_MNG_MODE2,
	VOTE_MNG_MODE3,
	VOTE_MNG_MODE_MAX
};

struct of_vote_mng_arbiter {
	struct list_head link;

	struct device_node *node;
	uint32_t type;
	struct mutex vote_mng_mutex;
	void *data;
};

struct vote_mng {
	struct device *dev;
	struct of_vote_mng_arbiter *arbiter;
	uint32_t arb;
};

static LIST_HEAD(of_vote_mng_arbiters);
DEFINE_SPINLOCK(g_vote_mng_spinlock);

int vote_mng_msg_send(struct vote_mng *vote_mng,
		      uint32_t *data,
		      uint32_t size,
		      uint32_t sync)
{
	int ret;
	struct of_vote_mng_arbiter *arbiter;

	if (!vote_mng || !data)
		return -EINVAL;

	arbiter = vote_mng->arbiter;
	if (!arbiter)
		return -ECHILD;

	if (arbiter->type != VOTE_MNG_MODE1)
		return -EPERM;

	mutex_lock(&arbiter->vote_mng_mutex);
	ret = vote_mng_mode1_msg_send(arbiter->data, vote_mng->arb, data, size, sync);
	mutex_unlock(&arbiter->vote_mng_mutex);

	return ret;
}
EXPORT_SYMBOL(vote_mng_msg_send);

static int vote_mng_mode2_request(struct vote_mng *vote_mng,
				  uint32_t ch,
				  uint32_t data,
				  uint32_t type)
{
	struct of_vote_mng_arbiter *arbiter;

	if (!vote_mng)
		return -EINVAL;

	arbiter = vote_mng->arbiter;
	if (!arbiter)
		return -ECHILD;

	if (arbiter->type != VOTE_MNG_MODE2)
		return -EPERM;

	return vote_mng_mode2_vote(arbiter->data, vote_mng->arb, ch, data, type);
}

int vote_mng_vote_dmd(struct vote_mng *vote_mng, uint32_t ch, uint32_t dmd)
{
	return vote_mng_mode2_request(vote_mng, ch, dmd, VOTE_MNG_MODE2_T_DMD);
}
EXPORT_SYMBOL(vote_mng_vote_dmd);

int vote_mng_vote_min(struct vote_mng *vote_mng, uint32_t ch, uint32_t min)
{
	return vote_mng_mode2_request(vote_mng, ch, min, VOTE_MNG_MODE2_T_MIN);
}
EXPORT_SYMBOL(vote_mng_vote_min);

int vote_mng_vote_max(struct vote_mng *vote_mng, uint32_t ch, uint32_t max)
{
	return vote_mng_mode2_request(vote_mng, ch, max, VOTE_MNG_MODE2_T_MAX);
}
EXPORT_SYMBOL(vote_mng_vote_max);

int vote_mng_vote_onoff(struct vote_mng *vote_mng, uint32_t ch, uint32_t flag)
{
	struct of_vote_mng_arbiter *arbiter;

	if (!vote_mng || (flag > VOTE_MNG_OFF_SYNC))
		return -EINVAL;

	arbiter = vote_mng->arbiter;
	if (!arbiter)
		return -ECHILD;

	if (arbiter->type != VOTE_MNG_MODE3)
		return -EPERM;

	return vote_mng_mode3_vote(arbiter->data, vote_mng->arb, ch, flag);
}
EXPORT_SYMBOL(vote_mng_vote_onoff);

static void *vote_mng_mode_hw_create(struct device *dev,
				    struct device_node *np,
				    void __iomem *base)
{
	struct vote_mng_mode1_hw *hw;
	int ret;
	u32 array[VOTE_MNG_REGS_NUM];
	void __iomem **regs;
	u32 i;

	hw = devm_kzalloc(dev, sizeof(*hw), GFP_KERNEL);
	if (!hw)
		return NULL;

	ret = of_property_read_u32_array(np, "regs", array, VOTE_MNG_REGS_NUM);
	if (ret < 0) {
		dev_err(dev, "bad np for regs\n");
		return NULL;
	}

	ret = of_property_read_u32(np, "arb_num", &hw->arb_num);
	if (ret) {
		dev_err(dev, "bad arb_num\n");
		return NULL;
	}

	ret = of_property_read_u32(np, "ch_num", &hw->ch_num);
	if (ret) {
		dev_err(dev, "bad arb_num\n");
		return NULL;
	}

	ret = of_property_read_u32(np, "core", &hw->core);
	if (ret) {
		dev_err(dev, "bad core name\n");
		return NULL;
	}

	regs = (void __iomem *)hw;
	for (i = 0; i < VOTE_MNG_REGS_NUM; i++)
		regs[i] = base + array[i];

	return (void *)hw;
}

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>

static struct dentry *rootdir;

static int vote_mng_file_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return nonseekable_open(inode, file);
}

static ssize_t vote_mng_file_read(struct file *file,
				  char __user *user_buf,
				  size_t count,
				  loff_t *ppos)
{
	char *buf;
	char *info;
	ssize_t ret;
	uint32_t arb_num;
	uint32_t i;
	struct vote_mng_mode1_hw *m1_hw;
	struct vote_mng_mode2_hw *m2_hw;
	struct vote_mng_mode3_hw *m3_hw;
	struct of_vote_mng_arbiter *arbiter = file->private_data;
	uint32_t type_id[] = {
		[VOTE_MNG_MODE1] = 1,
		[VOTE_MNG_MODE2] = 2,
		[VOTE_MNG_MODE3] = 3,
	};

	buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	info = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!info) {
		kfree(buf);
		return -ENOMEM;
	}

	switch (arbiter->type) {
	case VOTE_MNG_MODE1:
		m1_hw = arbiter->data;
		arb_num = m1_hw->arb_num;
		for (i = 0; i < arb_num; ++i) {
			snprintf(info, PAGE_SIZE, "arbiter %d message: 0x%x 0x%x 0x%x 0x%x\n", i,
				 readl(m1_hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(i) + 0x0),
				 readl(m1_hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(i) + 0x4),
				 readl(m1_hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(i) + 0x8),
				 readl(m1_hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(i) + 0xC));
			memcpy_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), info, strlen(info));
		}
		break;
	case VOTE_MNG_MODE2:
		m2_hw = arbiter->data;
		arb_num = m2_hw->arb_num;
		for (i = 0; i < arb_num; ++i) {
			snprintf(info, PAGE_SIZE,
				 "arbiter %d: dmd result 0x%x, max result 0x%x, min result 0x%x\n",
				 i,
				 readl(m2_hw->dmd + VOTE_MNG_MODE2_HW_OFFSET(i)),
				 readl(m2_hw->max + VOTE_MNG_MODE2_HW_OFFSET(i)),
				 readl(m2_hw->min + VOTE_MNG_MODE2_HW_OFFSET(i)));
			memcpy_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), info, strlen(info));
		}
		break;
	case VOTE_MNG_MODE3:
		m3_hw = arbiter->data;
		arb_num = m3_hw->arb_num;
		snprintf(info, PAGE_SIZE, "arbiters ack state: 0x%x\n", readl(m3_hw->ack));
		memcpy_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), info, strlen(info));
		break;
	default:
		kfree(buf);
		kfree(info);
		return -ENOMEM;
	}

	ret = snprintf(info, PAGE_SIZE,
		       "name: %s\n"
		       "type: vote mng mode%u\n"
		       "arb num: %u\n",
		       arbiter->node->name,
		       type_id[arbiter->type],
		       arb_num);
	ret = memcpy_s(buf + strlen(buf), PAGE_SIZE - strlen(buf), info, strlen(info));

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
	kfree(buf);
	kfree(info);

	return ret;
}

static ssize_t vote_mng_file_write(struct file *file,
				   const char __user *user_buf,
				   size_t count,
				   loff_t *ppos)
{
	struct vote_mng *mng;
	char *buf;
	ssize_t ret;
	uint32_t id = 0;
	uint32_t ops_type = 0;
	uint32_t para0 = 0;
	uint32_t para1 = 0;
	struct of_vote_mng_arbiter *arbiter = file->private_data;

	buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = simple_write_to_buffer(buf, PAGE_SIZE, ppos, user_buf, count);
	if (ret <= 0) {
		kfree(buf);
		return ret;
	}

	ret = sscanf(buf, "%u %u %u %u", &ops_type, &id, &para0, &para1);
	kfree(buf);

	pr_info("ops_type=%u, id=%u, para0=%u, para1=%u\n",
		ops_type, id, para0, para1);
	if (ret != 4) {
		pr_err("para parse fail\n");
		return -ENOMEM;
	}

	mng = kzalloc(sizeof(struct vote_mng), GFP_KERNEL);
	if (!mng)
		return -ENOMEM;

	mng->dev = NULL;
	mng->arbiter = arbiter;
	mng->arb = id;

	switch (ops_type) {
	case 0:
		ret = vote_mng_msg_send(mng, &para0, 1, para1);
		if (ret)
			pr_err("send fail %zd\n", ret);
		break;
	case 1:
		vote_mng_vote_dmd(mng, para0, para1);
		break;
	case 2:
		vote_mng_vote_min(mng, para0, para1);
		break;
	case 3:
		vote_mng_vote_max(mng, para0, para1);
		break;
	case 4:
		ret = vote_mng_vote_onoff(mng, para0, para1);
		if (ret)
			pr_err("send fail %zd\n", ret);
		break;
	default:
		kfree(mng);
		return -ENOMEM;
	};

	kfree(mng);

	if (ret != 0)
		return ret;
	return count;
}

static const struct file_operations vote_mng_fops = {
	.open = vote_mng_file_open,
	.read = vote_mng_file_read,
	.write = vote_mng_file_write,
	.owner = THIS_MODULE,
	.llseek = no_llseek,
};
#endif

static const struct of_device_id g_vote_mng_arbiter_matches[] = {
	{
		.compatible = "xring,vote_mng_m1_arbiter",
		.data = (void *)VOTE_MNG_MODE1,
	},
	{
		.compatible = "xring,vote_mng_m2_arbiter",
		.data = (void *)VOTE_MNG_MODE2,
	},
	{
		.compatible = "xring,vote_mng_m3_arbiter",
		.data = (void *)VOTE_MNG_MODE3,
	},
	{},
};

static int vote_mng_probe(struct platform_device *pdev)
{
	void *hw;
	void __iomem *base;
	struct resource *res;
	unsigned long flags;
	struct device_node *child = NULL;
	const struct of_device_id *match;
	struct of_vote_mng_arbiter *arbiter;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;
	base = devm_ioremap(dev, res->start, resource_size(res));
	if (!base)
		return -ENOMEM;

#ifdef CONFIG_DEBUG_FS
	if (!rootdir)
		rootdir = debugfs_create_dir(np->name, NULL);
#endif

	for_each_child_of_node(np, child) {
		match = of_match_node(g_vote_mng_arbiter_matches, child);
		if (!match) {
			dev_err(dev, "match vote mng[%s] hw fail\n", child->name);
			continue;
		}

		hw = vote_mng_mode_hw_create(dev, child, base);
		if (!hw) {
			dev_err(dev, "create vote mng[%s] hw fail\n", child->name);
			continue;
		}

		arbiter = devm_kzalloc(dev, sizeof(*arbiter), GFP_KERNEL);
		if (!arbiter)
			continue;
		arbiter->node = child;
		arbiter->type = (u32)(uintptr_t)match->data;
		arbiter->data = hw;

		mutex_init(&arbiter->vote_mng_mutex);

		spin_lock_irqsave(&g_vote_mng_spinlock, flags);
		list_add(&arbiter->link, &of_vote_mng_arbiters);
		spin_unlock_irqrestore(&g_vote_mng_spinlock, flags);

#ifdef CONFIG_DEBUG_FS
		debugfs_create_file(child->name, 0660,
				    rootdir, arbiter, &vote_mng_fops);
#endif
	}

	return 0;
}

static struct of_vote_mng_arbiter *
of_vote_mng_get_arbiter_from_spec(struct of_phandle_args *spec)
{
	unsigned long flags;
	struct of_vote_mng_arbiter *arbiter = ERR_PTR(-EPROBE_DEFER);

	if (!spec)
		return ERR_PTR(-EINVAL);

	spin_lock_irqsave(&g_vote_mng_spinlock, flags);
	list_for_each_entry(arbiter, &of_vote_mng_arbiters, link) {
		if (arbiter->node == spec->np)
			break;
	}
	spin_unlock_irqrestore(&g_vote_mng_spinlock, flags);

	return arbiter;
}

static int of_parse_vote_mng_spec(const struct device_node *np,
				  const char *name,
				  struct of_phandle_args *out_args)
{
	int index = 0;
	int ret = -ENOENT;

	/* Walk up the tree of devices looking for a vote mng property that matches */
	while (np) {
		if (name)
			index = of_property_match_string(np, "vote-mng-names", name);
		ret = of_parse_phandle_with_fixed_args(np, "vote-mngs", 1, index, out_args);
		if (!ret)
			break;
		if (name && index >= 0)
			break;

		/*
		 * No matching vote mng found on this node. If the parent node
		 * has a "vote-mng-ranges" property, then we can try one of its
		 * vote-mng.
		 */
		np = np->parent;
		if (np && !of_get_property(np, "vote-mng-ranges", NULL))
			break;
		index = 0;
	}

	return ret;
}

static struct vote_mng *
of_vote_mng_get_arbiter(struct device *dev, const char *con_id)
{
	int ret;
	struct of_vote_mng_arbiter *arbiter;
	struct vote_mng *core;
	struct of_phandle_args spec;
	uint32_t idx = 0;
	struct device_node *np = dev->of_node;

	if (!np)
		return ERR_PTR(-EINVAL);

	ret = of_parse_vote_mng_spec(np, con_id, &spec);
	if (ret)
		return ERR_PTR(ret);

	arbiter = of_vote_mng_get_arbiter_from_spec(&spec);
	if (spec.args_count > 0)
		idx = spec.args[0];
	of_node_put(spec.np);

	core = devm_kzalloc(dev, sizeof(*core), GFP_KERNEL);
	if (!core)
		return ERR_PTR(-ENOMEM);
	core->arbiter = arbiter;
	core->arb = idx;
	core->dev = dev;

	return core;
}

struct vote_mng *vote_mng_get(struct device *dev, const char *con_id)
{
	if (!dev)
		return NULL;

	return of_vote_mng_get_arbiter(dev, con_id);
}
EXPORT_SYMBOL(vote_mng_get);

int vote_mng_put(struct device *dev, struct vote_mng *core)
{
	return 0;
}
EXPORT_SYMBOL(vote_mng_put);

int vote_mng_mode2_get_dmd(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val)
{
	struct vote_mng_mode2_hw *mode2_hw;

	if (vote_mng == NULL)
		return -EINVAL;
	if (vote_mng->arbiter->type != VOTE_MNG_MODE2)
		return -EINVAL;

	mode2_hw = (struct vote_mng_mode2_hw *)vote_mng->arbiter->data;
	if (ch >= mode2_hw->ch_num)
		return -EINVAL;

	*val = readl(MODE2_XX_VAL_VOTE_CH_N(mode2_hw->dmd + VOTE_MNG_MODE2_HW_OFFSET(vote_mng->arb), ch));
	return 0;
}
EXPORT_SYMBOL(vote_mng_mode2_get_dmd);

int vote_mng_mode2_get_max(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val)
{
	struct vote_mng_mode2_hw *mode2_hw;

	if (vote_mng == NULL)
		return -EINVAL;
	if (vote_mng->arbiter->type != VOTE_MNG_MODE2)
		return -EINVAL;

	mode2_hw = (struct vote_mng_mode2_hw *)vote_mng->arbiter->data;
	if (ch >= mode2_hw->ch_num)
		return -EINVAL;

	*val = readl(MODE2_XX_VAL_VOTE_CH_N(mode2_hw->max + VOTE_MNG_MODE2_HW_OFFSET(vote_mng->arb), ch));
	return 0;
}
EXPORT_SYMBOL(vote_mng_mode2_get_max);

int vote_mng_mode2_get_min(struct vote_mng *vote_mng, uint32_t ch, uint32_t *val)
{
	struct vote_mng_mode2_hw *mode2_hw;

	if (vote_mng == NULL)
		return -EINVAL;
	if (vote_mng->arbiter->type != VOTE_MNG_MODE2)
		return -EINVAL;

	mode2_hw = (struct vote_mng_mode2_hw *)vote_mng->arbiter->data;
	if (ch >= mode2_hw->ch_num)
		return -EINVAL;

	*val = readl(MODE2_XX_VAL_VOTE_CH_N(mode2_hw->min + VOTE_MNG_MODE2_HW_OFFSET(vote_mng->arb), ch));
	return 0;
}
EXPORT_SYMBOL(vote_mng_mode2_get_min);

int vote_mng_mode2_get_ch_num(struct vote_mng *vote_mng)
{
	struct vote_mng_mode2_hw *mode2_hw;

	if (vote_mng == NULL)
		return -EINVAL;
	if (vote_mng->arbiter->type != VOTE_MNG_MODE2)
		return -EINVAL;

	mode2_hw = (struct vote_mng_mode2_hw *)vote_mng->arbiter->data;
	return mode2_hw->ch_num;
}
EXPORT_SYMBOL(vote_mng_mode2_get_ch_num);

int vote_mng_mode1_result_dump(struct vote_mng *votemng, struct vote_mng_mode1_result *res)
{
	uint32_t i;
	uint32_t arb_ch;
	struct vote_mng_mode1_hw *mode1_hw;

	if (IS_ERR_OR_NULL(votemng) || IS_ERR_OR_NULL(res))
		return -EINVAL;

	arb_ch = votemng->arb;

	res->arb_ch = arb_ch;

	mode1_hw = (struct vote_mng_mode1_hw *)votemng->arbiter->data;
	res->ack = vote_mng_mode1_ack_get(mode1_hw, arb_ch);
	for (i = 0; i < VOTE_MNG_MSG_SIZE; i++)
		res->msg[i] = readl(mode1_hw->msg + VOTE_MNG_MODE1_MSG_OFFSET(arb_ch) + i * 0x4);

	return 0;
}
EXPORT_SYMBOL(vote_mng_mode1_result_dump);

int vote_mng_mode2_result_dump(struct vote_mng *votemng, struct vote_mng_mode2_result *res)
{
	uint32_t arb_ch;
	struct vote_mng_mode2_hw *mode2_hw;

	if (IS_ERR_OR_NULL(votemng) || IS_ERR_OR_NULL(res))
		return -EINVAL;

	arb_ch = votemng->arb;

	res->arb_ch = arb_ch;

	mode2_hw = (struct vote_mng_mode2_hw *)votemng->arbiter->data;
	res->dmd = vote_mng_mode2_dmd_result(mode2_hw, arb_ch);
	res->max = vote_mng_mode2_max_result(mode2_hw, arb_ch);
	res->min = vote_mng_mode2_min_result(mode2_hw, arb_ch);

	return 0;
}
EXPORT_SYMBOL(vote_mng_mode2_result_dump);

int vote_mng_mode3_result_dump(struct vote_mng *votemng, struct vote_mng_mode3_result *res)
{
	uint32_t arb_ch;
	struct vote_mng_mode3_hw *mode3_hw;

	if (IS_ERR_OR_NULL(votemng) || IS_ERR_OR_NULL(res))
		return -EINVAL;

	arb_ch = votemng->arb;

	res->arb_ch = arb_ch;

	mode3_hw = (struct vote_mng_mode3_hw *)votemng->arbiter->data;
	res->ack = readl(mode3_hw->ack + VOTE_MNG_MODE3_HW_OFFSET(arb_ch));

	return 0;
}
EXPORT_SYMBOL(vote_mng_mode3_result_dump);

static int vote_mng_remove(struct platform_device *pdev)
{
	unsigned long flags;
	struct of_vote_mng_arbiter *arbiter = NULL;
	struct of_vote_mng_arbiter *tmp = NULL;

#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(rootdir);
#endif

	spin_lock_irqsave(&g_vote_mng_spinlock, flags);
	list_for_each_entry_safe(arbiter, tmp, &of_vote_mng_arbiters, link) {
		list_del(&arbiter->link);
	}
	spin_unlock_irqrestore(&g_vote_mng_spinlock, flags);

	return 0;
}

static const struct of_device_id vote_mng_dt_match[] = {
	{.compatible = "xring,vote_mng",},
	{}
};

static struct platform_driver vote_mng_driver = {
	.driver = {
		.name = "vote_mng",
		.of_match_table = of_match_ptr(vote_mng_dt_match),
	},
	.probe  = vote_mng_probe,
	.remove = vote_mng_remove,
};

module_platform_driver(vote_mng_driver);

MODULE_LICENSE("GPL");
