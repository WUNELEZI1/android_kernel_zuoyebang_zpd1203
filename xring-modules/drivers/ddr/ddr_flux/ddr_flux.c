// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/units.h>
#include <linux/sysfs.h>
#include <linux/arm-smccc.h>
#include <soc/xring/ddr_flux.h>
#include <soc/xring/perf_actuator.h>
#include <ddr_spec_define.h>
#include <fcm_acpu_address_map.h>
#include <bl31_smc_id.h>
#include <ddr_clock_gate_define.h>

#define CNT_MAX			0x100000000
#define CHANNEL_OFFSET(ch)	((ch) * (ACPU_DDRB_PHY - ACPU_DDRA_PHY))
#define SAFE_SUB_U32(a, b)	(((a) >= (b)) ? ((a) - (b)) : (0xFFFFFFFF - (b) + (a) + 1))
#define CMD_GET_DDR_FLUX	PERF_R_CMD(GET_DDR_FLUX, struct ddr_throughput)
#undef pr_fmt
#define pr_fmt(fmt)		"ddr_flux: " fmt

struct ddr_flux {
	void __iomem *ddr_flux_rd_addr[CHANNEL_NUM_MAX];
	void __iomem *ddr_flux_wr_addr[CHANNEL_NUM_MAX];
	unsigned int ddr_flux_rd_cnt[CHANNEL_NUM_MAX];
	unsigned int ddr_flux_wr_cnt[CHANNEL_NUM_MAX];
	unsigned long long ddr_flux_rd_bytes;
	unsigned long long ddr_flux_wr_bytes;
	unsigned int ddr_cnt_to_bytes;
	struct mutex flux_mutex;
	bool flux_initialized;
};

static struct ddr_flux g_ddr_flux;

static int ddr_flux_dts_proccess(struct device_node *ddr_flux_node)
{
	int ret;
	unsigned int ch;
	unsigned int ddr_flux_rd_reg;
	unsigned int ddr_flux_wr_reg;

	ret = of_property_read_u32_array(ddr_flux_node, "ddr_flux_rd_reg", &ddr_flux_rd_reg, 1);
	if (ret) {
		pr_err("Failed to get ddr_flux_rd_reg!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_flux_node, "ddr_flux_wr_reg", &ddr_flux_wr_reg, 1);
	if (ret) {
		pr_err("Failed to get ddr_flux_wr_reg!\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(ddr_flux_node,
			"ddr_cnt_to_bytes", &g_ddr_flux.ddr_cnt_to_bytes, 1);
	if (ret) {
		pr_err("Failed to get ddr_cnt_to_bytes!\n");
		return -EINVAL;
	}

	for (ch = 0; ch < CHANNEL_NUM_MAX; ch++) {
		g_ddr_flux.ddr_flux_rd_addr[ch] =
			ioremap(ddr_flux_rd_reg +  CHANNEL_OFFSET(ch), sizeof(unsigned int));
		g_ddr_flux.ddr_flux_wr_addr[ch] =
			ioremap(ddr_flux_wr_reg +  CHANNEL_OFFSET(ch), sizeof(unsigned int));
		if (g_ddr_flux.ddr_flux_rd_addr[ch] == NULL ||
				g_ddr_flux.ddr_flux_wr_addr[ch] == NULL) {
			pr_err("Failed to ioremap ddr_flux_addr!\n");
			return -EINVAL;
		}
	}

	return 0;
}

int get_ddr_throughput(struct ddr_throughput *throughput)
{
	unsigned int ch;
	unsigned int rd_cnt;
	unsigned int wr_cnt;
	unsigned int rd_cnt_add;
	unsigned int wr_cnt_add;
	struct arm_smccc_res res = { 0 };

	if (!g_ddr_flux.flux_initialized) {
		pr_err("ddr flux no probe!\n");
		return -EINVAL;
	}

	if (throughput == NULL) {
		pr_err("get ddr throughput input is NULL!\n");
		return -EINVAL;
	}

	mutex_lock(&g_ddr_flux.flux_mutex);

	arm_smccc_smc(FID_BL31_DDR_CLOCK_GATE, true, PERF_TOP_CLK_GT,
		      GT_VOTE_FEATURE_FLUX, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("perf top clk enable err, ret=0x%lx!\n", res.a0);
		return -EINVAL;
	}

	for (ch = 0; ch < CHANNEL_NUM_MAX; ch++) {
		rd_cnt = readl(g_ddr_flux.ddr_flux_rd_addr[ch]);
		wr_cnt = readl(g_ddr_flux.ddr_flux_wr_addr[ch]);

		rd_cnt_add = SAFE_SUB_U32(rd_cnt, g_ddr_flux.ddr_flux_rd_cnt[ch]);
		if (rd_cnt_add == 0)
			pr_warn("warning ch%d rd ddr flux cnt no increase!\n", ch);
		g_ddr_flux.ddr_flux_rd_cnt[ch] = rd_cnt;

		wr_cnt_add = SAFE_SUB_U32(wr_cnt, g_ddr_flux.ddr_flux_wr_cnt[ch]);
		if (wr_cnt_add == 0)
			pr_warn("warning ch%d wr ddr flux cnt no increase!\n", ch);
		g_ddr_flux.ddr_flux_wr_cnt[ch] = wr_cnt;

		g_ddr_flux.ddr_flux_rd_bytes +=
			(unsigned long long)rd_cnt_add * g_ddr_flux.ddr_cnt_to_bytes;
		g_ddr_flux.ddr_flux_wr_bytes +=
			(unsigned long long)wr_cnt_add * g_ddr_flux.ddr_cnt_to_bytes;
	}

	throughput->rd_bytes = g_ddr_flux.ddr_flux_rd_bytes;
	throughput->wr_bytes = g_ddr_flux.ddr_flux_wr_bytes;

	arm_smccc_smc(FID_BL31_DDR_CLOCK_GATE, false, PERF_TOP_CLK_GT,
		GT_VOTE_FEATURE_FLUX, 0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("perf top clk disable err, ret=0x%lx!\n", res.a0);
		return -EINVAL;
	}

	mutex_unlock(&g_ddr_flux.flux_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(get_ddr_throughput);

static int perf_actuator_get_ddr_flux(void __user *uarg)
{
	int ret;
	struct ddr_throughput throughput;

	if (uarg == NULL)
		return -EINVAL;

	ret = get_ddr_throughput(&throughput);
	if (ret != 0)
		return ret;

	if (copy_to_user(uarg, &throughput, sizeof(struct ddr_throughput)))
		return -EFAULT;

	return 0;
}

static int ddr_flux_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;

	ret = ddr_flux_dts_proccess(dev->of_node);
	if (ret) {
		pr_err("failed to ddr_flux_dts_proccess!\n");
		return -EINVAL;
	}



	mutex_init(&g_ddr_flux.flux_mutex);
	g_ddr_flux.flux_initialized = true;

	register_perf_actuator(CMD_GET_DDR_FLUX, perf_actuator_get_ddr_flux);

	pr_info("ddr flux probe success!\n");

	return 0;
}

static int ddr_flux_remove(struct platform_device *pdev)
{
	unsigned int ch;

	mutex_lock(&g_ddr_flux.flux_mutex);

	unregister_perf_actuator(CMD_GET_DDR_FLUX);



	for (ch = 0; ch < CHANNEL_NUM_MAX; ch++) {
		if (g_ddr_flux.ddr_flux_rd_addr[ch] != NULL)
			iounmap(g_ddr_flux.ddr_flux_rd_addr[ch]);

		if (g_ddr_flux.ddr_flux_wr_addr[ch] != NULL)
			iounmap(g_ddr_flux.ddr_flux_wr_addr[ch]);
	}

	mutex_unlock(&g_ddr_flux.flux_mutex);
	mutex_destroy(&g_ddr_flux.flux_mutex);
	pr_info("ddr flux remove success!\n");

	return 0;
}

static const struct of_device_id g_ddr_flux_table[] = {
	{.compatible = "xring,ddr_flux"},
	{},
};

MODULE_DEVICE_TABLE(of, g_ddr_flux_table);

static struct platform_driver g_ddr_flux_driver = {
	.probe = ddr_flux_probe,
	.remove = ddr_flux_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ddr_flux",
		.of_match_table = g_ddr_flux_table,
	},
};

int ddr_flux_init(void)
{
	return platform_driver_register(&g_ddr_flux_driver);
}

void ddr_flux_exit(void)
{
	platform_driver_unregister(&g_ddr_flux_driver);
}
