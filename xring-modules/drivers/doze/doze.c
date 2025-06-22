// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/of_platform.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/kstrtox.h>
#include <linux/spinlock.h>
#include <linux/arm-smccc.h>
#include <soc/xring/pm_comm.h>
#include <soc/xring/xr_timestamp.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_pm_comm_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_pm_comm.h>

#define BM_SHIFT 16
#undef pr_fmt
#define pr_fmt(fmt)	"[DOZE]:%s:%d " fmt, __func__, __LINE__
#define xr_doze_print(s, fmt, args ...) \
	do { \
		if (s == NULL) \
			pr_info(fmt, ##args); \
		else \
			seq_printf(s, fmt, ##args); \
	} while (0)

void __iomem *g_vote_usec;

static unsigned int g_doze_vote_cnt[VOTER_DOZAP_AP_MAX] = {0};
static unsigned int g_doze_all_vote_cnt;

DEFINE_SPINLOCK(g_doz_lock);

struct doze_ap_vote_info g_doze_ap_vote_info;

bool doze_voter_check(unsigned int voter_id)
{
	if (voter_id >= DOZE_VOTER_MAX)
		return false;
	return true;
}
EXPORT_SYMBOL(doze_voter_check);

static bool source_security_check_acpu(unsigned int voter_id)
{
	u64 val;

	val = DOZE_VOTE_SEC_SEL;
	if ((val & BIT(sys_doze_voter[voter_id])) == 0)
		return true;
	return false;
}

static inline bool is_all_doze_vote(void)
{
	int i;
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	for (i = VOTER_DOZAP_USB; i < VOTER_DOZAP_AP_MAX; i++) {
		if (vote_info->cur_voter[i] == 0)
			return false;
	}
	return true;
}

static bool has_ap_vote_doze(void)
{
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	return vote_info->ap_vote;
}

static void sys_doze_vote_acpu(unsigned int voter_id)
{
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	if (!doze_voter_check(voter_id)) {
		xr_doze_print(NULL, "invalid voter!\n");
		return;
	}

	if (!is_all_doze_vote() || has_ap_vote_doze())
		return;

	if (g_vote_usec == NULL) {
		xr_doze_print(NULL, "vote addr invalid!\n");
		return;
	}

	if (source_security_check_acpu(voter_id))
		return;
	writel(BIT(sys_doze_voter[voter_id]) << BM_SHIFT |
		BIT(sys_doze_voter[voter_id]), g_vote_usec);
	vote_info->ap_vote = true;
}

static void sys_doze_unvote_acpu(unsigned int voter_id)
{
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	if (!doze_voter_check(voter_id)) {
		xr_doze_print(NULL, "invalid voter!\n");
		return;
	}

	if (vote_info->ap_vote == false)
		return;

	if (g_vote_usec == NULL) {
		xr_doze_print(NULL, "vote addr invalid!\n");
		return;
	}

	if (source_security_check_acpu(voter_id))
		return;
	writel(BIT(sys_doze_voter[voter_id]) << BM_SHIFT, g_vote_usec);
	vote_info->ap_vote = false;
}

bool is_valid_voter(unsigned int voter)
{
	if (voter >= VOTER_DOZAP_AP_MAX)
		return false;
	return true;
}
EXPORT_SYMBOL(is_valid_voter);

bool is_doze_soft_vote(unsigned int voter_bit)
{
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	if (voter_bit >= VOTER_DOZAP_AP_MAX)
		return true;
	if (vote_info->cur_voter[voter_bit])
		return true;
	return false;
}
EXPORT_SYMBOL(is_doze_soft_vote);

void sys_state_timestamp_save(unsigned int voter_id, struct doze_ap_vote_info *vote_info)
{
	ktime_t time;

	time = xr_timestamp_gettime();
	vote_info->vote_time[voter_id][g_doze_vote_cnt[voter_id] % SYS_DOZE_SAVE_CNT] = time;
}

int sys_state_doz2nor_unvote(unsigned int voter)
{
	unsigned long flags;
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	if (!is_valid_voter(voter)) {
		xr_doze_print(NULL, "invalid param!\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&g_doz_lock, flags);

	vote_info->cur_voter[voter] = 1;
	sys_doze_vote_acpu(DOZE_VOTER_AP);
	vote_info->vote_cnt[voter] += 1;
	sys_state_timestamp_save(voter, vote_info);
	g_doze_all_vote_cnt += 1;
	g_doze_vote_cnt[voter] += 1;

	spin_unlock_irqrestore(&g_doz_lock, flags);
	return 0;
}
EXPORT_SYMBOL(sys_state_doz2nor_unvote);

int sys_state_doz2nor_vote(unsigned int voter)
{
	unsigned long flags;
	struct doze_ap_vote_info *vote_info = &g_doze_ap_vote_info;

	if (!is_valid_voter(voter)) {
		xr_doze_print(NULL, "invalid param!\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&g_doz_lock, flags);

	vote_info->cur_voter[voter] = 0;
	sys_doze_unvote_acpu(DOZE_VOTER_AP);

	spin_unlock_irqrestore(&g_doz_lock, flags);
	return 0;
}
EXPORT_SYMBOL(sys_state_doz2nor_vote);

static int doze_vote_probe(struct platform_device *plat_dev)
{
	struct resource *mem = NULL;
	unsigned int index = 0;
	int ret = 0;
	u32 doze_stat;

	g_vote_usec = devm_platform_get_and_ioremap_resource(plat_dev, 0, &mem);
	if (g_vote_usec == NULL) {
		xr_doze_print(NULL, "g_vote_usec ioremap failed!\n");
		return -EINVAL;
	}

	for (index = 0; index < VOTER_DOZAP_AP_MAX; index++)
		sys_state_doz2nor_unvote(index);
	ret = of_property_read_u32((&plat_dev->dev)->of_node, "xring,doze_enable", &doze_stat);
	if (ret) {
		xr_doze_print(NULL, "%s:%d: Failed to read property\n", __func__, __LINE__);
		return ret;
	}
	if (doze_stat == 0)
		sys_state_doz2nor_vote(VOTER_DOZAP_SWITCH);
	return 0;
}

static int doze_vote_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id doze_vote_acpu_match_table[] = {
	{.compatible = "xring,doze_vote"},
	{}};

static struct platform_driver doze_vote_driver = {
	.driver = {
		.name = "doze_vote",
		.of_match_table = doze_vote_acpu_match_table,
	},
	.probe = doze_vote_probe,
	.remove = doze_vote_remove,
};

static int __init doze_vote_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&doze_vote_driver);
	if (ret) {
		xr_doze_print(NULL, "platdrv register failed\n");
		return ret;
	}
	return 0;
}

static void __exit doze_vote_driver_exit(void)
{
	platform_driver_unregister(&doze_vote_driver);
}

module_init(doze_vote_driver_init);
module_exit(doze_vote_driver_exit);
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: xr_pm_comm");
MODULE_SOFTDEP("pre: xr_timestamp");
