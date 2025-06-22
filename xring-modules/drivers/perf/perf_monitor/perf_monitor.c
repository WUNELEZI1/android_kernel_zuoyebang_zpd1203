// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: perf monitor driver
 */

#include <linux/arm-smccc.h>
#include <linux/dev_printk.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/moduleparam.h>
#include <linux/namei.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/wait.h>
#include <linux/of_reserved_mem.h>
#include <linux/notifier.h>
#include <linux/regulator/consumer.h>
#include <dt-bindings/xring/platform-specific/perf_monitor.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>

#include "perf_monitor.h"

#undef pr_fmt
#define pr_fmt(fmt) "[perf_monitor]:%s:%d " fmt, __func__, __LINE__

static struct perf_mon *g_perf_mon_dev;
static int g_startflag;
static int g_initflag;

struct perf_regulator_notifier {
	struct regulator *my_regulator;
	struct notifier_block my_notifier;
};

struct perf_regulator_func_s {
	char subsys_name[20];
	int (*notifier_callback)(struct notifier_block *nb, unsigned long event, void *data);
};

enum {
	PERF_REGULAOTOR_DPU0,
	PERF_REGULAOTOR_M1,
	PERF_REGULAOTOR_VEDC,
	PERF_REGULAOTOR_ISP,
	PERF_REGULAOTOR_VENC,
	PERF_REGULAOTOR_M2,
	PERF_REGULAOTOR_NPU,
	PERF_REGULAOTOR_GPU,
	REGULATOR_MAX
};

static struct perf_regulator_notifier perf_regulator_cb[REGULATOR_MAX];

static int ddr_channel_set(void);

static struct perf_mon *get_perf_monitor(void)
{
	return g_perf_mon_dev;
}

static void set_perf_monitor(struct perf_mon *perf_struct)
{
	if (perf_struct != NULL)
		g_perf_mon_dev = perf_struct;
}

static void perf_crg(u32 reset_addr, u32 clk_addr, u32 reset_val, u32 clk_val)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	void __iomem *perf_reset_addr;
	void __iomem *perf_clk_addr;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return;
	}

	perf_reset_addr = devm_ioremap(perf_mon_dev->dev, reset_addr, CRG_LONGTH);
	perf_clk_addr = devm_ioremap(perf_mon_dev->dev, clk_addr, CRG_LONGTH);

	writel(reset_val, perf_reset_addr + CRG_SHIFT);
	writel(reset_val, perf_reset_addr);
	writel(clk_val, perf_clk_addr + CRG_SHIFT);
	writel(clk_val, perf_clk_addr);
}

static int dpu0_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	void __iomem *dpu_reset;
	void __iomem *dpu_clk;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}

	dpu_reset = devm_ioremap(perf_mon_dev->dev,
		ACPU_MEDIA1_CRG + Media1_CRG_SOFT_RESETS, CRG_LONGTH);
	dpu_clk = devm_ioremap(perf_mon_dev->dev,
		ACPU_MEDIA1_CRG + Media1_CRG_CLK_OFFSET, CRG_LONGTH);

	pr_info("Dpu0 notifier callback\n");
	if (event == REGULATOR_EVENT_ENABLE) {
		writel(Media1_CRG_RESET_VALUE, dpu_reset);
		writel(Media1_CRG_CLK_VALUE, dpu_clk);
	}
	return 0;
}

static int vdec_subsys_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_MEDIA1_CRG + Media1_CRG_SOFT_RESETS, ACPU_MEDIA1_CRG +
			Media1_VCRG_CLK_OFFSET, VDEC_TBU_RESET_VALUE, VDEC_TBU_CLK_VALUE);
	return 0;
}

static int isp_subsys_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_M2_CRG + ISP_CRG_SOFT_RESETS, ACPU_M2_CRG + ISP_CRG_CLK_OFFSET,
			ISP_CRG_RESET_VALUE, ISP_CRG_CLK_VALUE);
	return 0;
}

static int media1_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_MEDIA1_CRG + Media1_CRG_SOFT_RESETS, ACPU_MEDIA1_CRG +
			VDEC_TCU_CLK_OFFSET, VDEC_TCU_RESET_VALUE, VDEC_TCU_CLK_VALUE);
	return 0;
}

static int venc_subsys_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_M2_CRG + VENC_CRG_SOFT_RESETS, ACPU_M2_CRG + VENC_CRG_CLK_OFFSET,
			VENC_CRG_RESET_VALUE, VENC_CRG_CLK_VALUE);
	return 0;
}

static int media2_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_M2_CRG + VENC_CRG_SOFT_RESETS, ACPU_M2_CRG + VENC_CRG_CLK_OFFSET,
			Media2_CRG_RESET_VALUE, Media2_CRG_CLK_VALUE);
	return 0;
}

static int npu_subsys_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct arm_smccc_res res;

	if (event == REGULATOR_EVENT_ENABLE)
		arm_smccc_smc(FID_BL31_PERF_MONITOR_CFG, X1_NPU, 0, 0, 0, 0, 0, 0, &res);
	return 0;
}

static int gpu_subsys_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == REGULATOR_EVENT_ENABLE)
		perf_crg(ACPU_GPU_CRG + GPU_CRG_CLK_OFFSETS, ACPU_GPU_CRG + GPU_CRG_SOFT_RESETS,
			GPU_CRG_RESET_VALUE, GPU_CRG_CLK_VALUE);
	return 0;
}

static int perf_regulator(struct device *dev)
{
	int ret = 0;
	int i;

	static struct perf_regulator_func_s perf_regulator_funcs[REGULATOR_MAX] = {
		{"dpu0", dpu0_notifier_callback},
		{"media1_subsys", media1_notifier_callback},
		{"vdec_subsys", vdec_subsys_notifier_callback},
		{"isp_subsys", isp_subsys_notifier_callback},
		{"venc_subsys", venc_subsys_notifier_callback},
		{"media2_subsys", media2_notifier_callback},
		{"npu_subsys", npu_subsys_notifier_callback},
		{"gpu_subsys", gpu_subsys_notifier_callback},
	};

	for (i = 0; i < REGULATOR_MAX; ++i) {
		perf_regulator_cb[i].my_regulator = regulator_get(dev,
			perf_regulator_funcs[i].subsys_name);
		if (IS_ERR(perf_regulator_cb[i].my_regulator)) {
			ret = PTR_ERR(perf_regulator_cb[i].my_regulator);
			pr_err("Failed to get regulator: %d\n", ret);
			return ret;
		}
		perf_regulator_cb[i].my_notifier.notifier_call =
			perf_regulator_funcs[i].notifier_callback;
		ret = regulator_register_notifier(perf_regulator_cb[i].my_regulator,
			&(perf_regulator_cb[i].my_notifier));
		if (ret) {
			pr_err("Failed to register notifier: %d\n", ret);
			regulator_put(perf_regulator_cb[i].my_regulator);
			return ret;
		}
	}

	return ret;
}

static int args_judgment(u32 accuracy, enum pattern_type pattern, u32 time)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}

	if (accuracy > MAX_ACCU || accuracy < MIN_ACCU) {
		dev_err(perf_mon_dev->dev, "Perf start failed because of wrong accuracy!\n");
		return -EINVAL;
	}

	if (pattern == LOOP_MODE) {
		if (time != 0) {
			dev_err(perf_mon_dev->dev, "Time should be 0 in LOOP mode!\n");
			return -EINVAL;
		}
	}

	if (pattern == ONE_SHOT_MODE) {
		if (time <= 0 || time > MAX_TIME) {
			dev_err(perf_mon_dev->dev, "The value of time is out of range!\n");
			return -EINVAL;
		}
	}

	if (pattern != LOOP_MODE && pattern != ONE_SHOT_MODE) {
		dev_err(perf_mon_dev->dev, "Perf start failed because of wrong pattern!\n");
		return -EINVAL;
	}

	return 0;
}

static int perf_monitor_cfg(struct perf_mon *perf_struct, u64 num_of_chn, enum pattern_type pattern,
			u32 accuracy, u32 time)
{
	int args_ret = 0;
	u32 val = 0;
	u32 chn_l = 0;
	u32 chn_h = 0;
	u32 addr_l = 0;
	u32 addr_h = 0;
	u32 chn_number = 0;
	u32 datacount = 0;
	u32 datasize = 0;
	u64 base_addr = perf_struct->base_addr;
	u64 mem_size = perf_struct->mem_size;
	u32 time_cnt = accuracy * ACCU_CONVERT;
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}

	args_ret = args_judgment(accuracy, pattern, time);
	if (args_ret) {
		dev_err(perf_struct->dev, "args are wrong!\n");
		return -EINVAL;
	}

	/* ----------open channels cfg---------- */
	chn_l = (num_of_chn & MSK_L);
	chn_h = ((num_of_chn & MSK_H) >> BIT_NUM);

	writel(chn_l, perf_struct->regs + PERF_MONITOR_P_MASK1);
	writel(chn_h, perf_struct->regs + PERF_MONITOR_P_MASK2);

	while (num_of_chn != 0) {
		if ((num_of_chn & MARK_ONE) == CHANNEL_OPEN)
			chn_number += 1;
		num_of_chn >>= MARK_ONE;
	}

	/* ----------total number of open channels cfg---------- */
	val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
	val |= (chn_number << PERF_MONITOR_P_COMMON_REG_CHN_NUM_SHIFT);
	writel(val, perf_struct->regs + PERF_MONITOR_P_COMMON);

	/* ----------times of looping, only in one-shot mode---------- */
	if (pattern == ONE_SHOT_MODE) {
		datacount = time * perf_mon_dev->time_shift / accuracy;
		writel(datacount, perf_struct->regs + PERF_MONITOR_WIN_LEN);
	}

	/* ----------pattern cfg---------- */
	val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
	val |= (pattern << PERF_MONITOR_P_COMMON_REG_MODE_SEL_SHIFT);
	writel(val, perf_struct->regs + PERF_MONITOR_P_COMMON);

	/* ----------accuracy cfg---------- */
	val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
	val |= (time_cnt << PERF_MONITOR_P_COMMON_REG_WIN_LEN_SHIFT);
	writel(val, perf_struct->regs + PERF_MONITOR_P_COMMON);
	memset(perf_struct->mem, 0, RESERVE_MEM_SIZE);
	writel(time_cnt, perf_struct->mem);
	// flush cache
	dma_sync_single_for_device(perf_struct->dev, base_addr,
		FLUSH_CACHE_SIZE, DMA_TO_DEVICE);
	pr_info("wr time_cnt = 0x%x to mem, flush cache; win_loop_num = 0x%x for oneshot:%d\n",
		time_cnt, datacount, pattern);

	/* ----------mem addr & size cfg---------- */
	addr_l = (base_addr & MSK_L) + RESERVE_MEM_SIZE; // reserve pre 32B for saving timecnt
	addr_h = ((base_addr & MSK_H) >> BIT_NUM);

	writel(addr_l, perf_struct->regs + PERF_MONITOR_P_BASE_ADDR_L);
	writel(addr_h, perf_struct->regs + PERF_MONITOR_P_BASE_ADDR_H);
	writel(mem_size, perf_struct->regs + PERF_MONITOR_P_ADDR_SIZE);
	pr_info("P_COMMON = 0x%x, addr_h = 0x%x, addr_l = 0x%x, mem_size = 0x%llx\n",
		readl(perf_struct->regs + PERF_MONITOR_P_COMMON), addr_h, addr_l, mem_size);

	datasize = datacount * chn_number * DATA_SIZE;
	perf_struct->datasize = datasize;
	perf_struct->pat = pattern;

	return 0;
}

static void mid_set(struct perf_mon *perf_struct)
{
	uint32_t filter_mid_en;
	uint32_t cpu_l_mid_num = CPU_L_MID_NUM  << PERF_MONITOR_P_MID_CPU_REG_CHN41_MID_SHIFT;
	uint32_t cpu_m0_mid_num = CPU_M0_MID_NUM << PERF_MONITOR_P_MID_CPU_REG_CHN40_MID_SHIFT;
	uint32_t cpu_m1_mid_num = CPU_M1_MID_NUM  << PERF_MONITOR_P_MID_CPU_REG_CHN39_MID_SHIFT;
	uint32_t cpu_b_mid_num  = CPU_B_MID_NUM << PERF_MONITOR_P_MID_CPU_REG_CHN38_MID_SHIFT;
	uint32_t cpu_mid = cpu_l_mid_num | cpu_m0_mid_num | cpu_m1_mid_num | cpu_b_mid_num;

	writel(cpu_mid, perf_struct->regs + PERF_MONITOR_P_MID_CPU);

	//mid cfg
	writel(CPU_WB_MID, perf_struct->regs + PERF_MONITOR_P_MID_MAINBUS);

	//filter cfg
	filter_mid_en = readl(perf_struct->regs + PERF_MONITOR_P_MID_MAINBUS);
	writel((filter_mid_en | FILTER_CPU), perf_struct->regs + PERF_MONITOR_P_MID_MAINBUS);

	pr_info("REG_MID_CPU = 0x%x, REG_MAINBUS_MID = 0x%x\n",
		readl(perf_struct->regs + PERF_MONITOR_P_MID_CPU),
		readl(perf_struct->regs + PERF_MONITOR_P_MID_MAINBUS));
}

static void perf_monitor_enable(struct perf_mon *perf_struct)
{
	u32 val;

	val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
	val |= REG_WIN_EN;  // cfg reg_win_en
	writel(val, perf_struct->regs + PERF_MONITOR_P_COMMON);
	pr_info("PERF_MONITOR_P_COMMON = 0x%x\n", readl(perf_struct->regs + PERF_MONITOR_P_COMMON));
}

static void perf_monitor_disable(struct perf_mon *perf_struct)
{
	u32 val;

	val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
	val |= REG_CFG_STOP;  // cfg stop
	writel(val, perf_struct->regs + PERF_MONITOR_P_COMMON);
}

static int perf_save_file_cfg(void)
{
	struct perf_save_file *pfile;
	u64 size = 0;
	int pattern;
	loff_t pos = 0;
	const char *path;
	u64 mem_size;
	u64 loop_no_cover_size;
	u64 loop_cover_size;
	u64 loop_bottom_size;
	int cover_flag;
	struct file *file = NULL;
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}
	pfile = perf_mon_dev->pfile;
	pattern = perf_mon_dev->pat;
	path = perf_mon_dev->fpath;
	mem_size = perf_mon_dev->mem_size;
	loop_no_cover_size = readl(perf_mon_dev->regs + PERF_MONITOR_LOOP_RD_BACK_END_ADDR);
	loop_cover_size = readl(perf_mon_dev->regs + PERF_MONITOR_LOOP_RD_BACK_ADDR);
	loop_bottom_size = mem_size - loop_cover_size;
	cover_flag = readl(perf_mon_dev->regs + PERF_MONITOR_LOOP_RD_BACK_ADDR_VALID);

	if (pattern == ONE_SHOT_MODE && perf_mon_dev->datasize != 0) {
		size = perf_mon_dev->datasize;
		dev_info(perf_mon_dev->dev, "pattern is %d, address is 0x%llx, size is 0x%llx\n",
				pattern, perf_mon_dev->base_addr, size);
	}
	if (pattern == LOOP_MODE) {
		if (!cover_flag)
			size = loop_no_cover_size;
		else
			size = loop_cover_size;

		dev_info(perf_mon_dev->dev, "pattern is %d, size is 0x%llx, cover_flag is %d\n",
				pattern, size, cover_flag);
	}

	file = filp_open(path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(file)) {
		dev_err(perf_mon_dev->dev, "Create file error!/n");
		return PTR_ERR(file);
	}

	pos = file->f_pos;
	filp_close(file, NULL);

	pfile->size = size;
	pfile->loop_cover_offset = loop_cover_size;
	pfile->loop_bottom_size = loop_bottom_size;
	pfile->pos = pos;

	return 0;
}

/* Write data from base address */
static int file_create(void *buf, u64 size, loff_t pos)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	struct perf_save_file *pfile;
	struct file *file = NULL;
	const char *path;
	ssize_t ret;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}
	pfile = perf_mon_dev->pfile;
	path = perf_mon_dev->fpath;

	pr_info("size = 0x%llx", size);

	file = filp_open(path, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(file)) {
		dev_err(perf_mon_dev->dev, "Create file top error!/n");
		return PTR_ERR(file);
	}

	ret = __kernel_write(file, buf, size, &pos);
	if (ret != size) {
		dev_err(perf_mon_dev->dev, "Failed to write to file!\n");
		filp_close(file, NULL);
		return -EINVAL;
	}

	filp_close(file, NULL);
	dev_info(perf_mon_dev->dev, "Data file has been created!\n");

	return 0;
}

static int perf_monitor_save_to_file(void *buf)
{
	struct perf_save_file *pfile;
	loff_t pos;
	int pattern;
	int filetop;
	int filebottom;
	int fileheader;
	u64 loop_cover_addr;
	u64 loop_bottom_size;
	u64 loop_bottom_offset;
	u64 size;
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}
	pfile = perf_mon_dev->pfile;
	pos = pfile->pos;
	size = pfile->size;
	loop_cover_addr = pfile->loop_cover_offset;
	loop_bottom_offset = pfile->loop_bottom_size;
	pattern = perf_mon_dev->pat;

	pr_info("size = 0x%llx, loop_cover_addr = 0x%llx, loop_bottom_offset = 0x%llx",
			size, loop_cover_addr, loop_bottom_offset);

	/*For one-shot mode and loop mode when data is not covered*/
	if (pattern == ONE_SHOT_MODE || (pattern == LOOP_MODE && loop_cover_addr == 0)) {
		filetop = file_create(buf, size, pos);
		if (filetop)
			goto file_top_create_err;
	}

	/*For loop mode when data is covered*/
	if (pattern == LOOP_MODE && loop_cover_addr != 0) {
		fileheader = file_create(buf, RESERVE_MEM_SIZE, pos);
		if (fileheader)
			goto file_header_create_err;

		pos += (loff_t)RESERVE_MEM_SIZE;
		loop_bottom_size = loop_bottom_offset + ONE_FRAME_SIZE;
		filebottom = file_create(buf + size + ONE_FRAME_SIZE, loop_bottom_size, pos);
		if (filebottom)
			goto file_bottom_create_err;

		pos += (loff_t)loop_bottom_size;
		filetop = file_create(buf + RESERVE_MEM_SIZE, size, pos);
		if (filetop)
			goto file_top_create_err;
	}
	return 0;

file_top_create_err:
	dev_err(perf_mon_dev->dev, "failed to get top file!\n");
	return -EINVAL;
file_bottom_create_err:
	dev_err(perf_mon_dev->dev, "failed to get bottom file!\n");
	return -EINVAL;
file_header_create_err:
	dev_err(perf_mon_dev->dev, "failed to get header file!\n");
	return -EINVAL;
}

void perf_save_file(void)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	char *mem;
	int file_cfg_ret = 0;
	int save_to_file_ret = 0;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return;
	}
	mem = perf_mon_dev->mem;

	// before cpu read, invalid cacheline range data
	dma_sync_single_for_cpu(perf_mon_dev->dev, perf_mon_dev->base_addr,
		FLUSH_CACHE_SIZE, DMA_FROM_DEVICE);

	file_cfg_ret = perf_save_file_cfg();
	if (file_cfg_ret) {
		dev_err(perf_mon_dev->dev, "failed to finish save file config!\n");
		return;
	}
	pr_info("-----save file config finished-----\n");

	save_to_file_ret = perf_monitor_save_to_file(mem);
	if (save_to_file_ret) {
		dev_err(perf_mon_dev->dev, "failed to save data to file!\n");
		return;
	}
	pr_info("Successfully save data file!\n");
}

static void perf_save_work(struct work_struct *work)
{
	perf_save_file();
}

static void interrupt_mask(void)
{
	u32 mask = 0;
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return;
	}

	mask = readl(perf_mon_dev->regs + PERF_MONITOR_IRQ_IRQ_INT_MASK);
	mask &= ~(DATA_IRQ);
	writel(mask, perf_mon_dev->regs + PERF_MONITOR_IRQ_IRQ_INT_MASK);
}

static irqreturn_t perf_monitor_interrupt(int irq, void *dev_id)
{
	u32 irq_val;
	u32 common_val;
	struct perf_mon *perf_struct = dev_id;
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return 0;
	}

	irq_val = readl(perf_struct->regs + PERF_MONITOR_IRQ_IRQ_INT_STATUS);

	pr_info("intr start, irq_val = 0x%08x\n", irq_val);

	writel(CLEAR_INT, perf_struct->regs + PERF_MONITOR_IRQ_IRQ_INT_RAW);

	if (irq_val & PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_ERROR_STATUS_MASK)
		dev_info(perf_mon_dev->dev, "Perf data error interrupt occurs\n");

	if (irq_val & PERF_MONITOR_IRQ_IRQ_INT_STATUS_STAT_PD_FLAG_STATUS_MASK)
		dev_info(perf_mon_dev->dev, "Perf pd flag interrupt occurs\n");

	if (irq_val & PERF_MONITOR_IRQ_IRQ_INT_STATUS_BRESP_PULSE_STATUS_MASK)
		dev_info(perf_mon_dev->dev, "Perf bresp pulse interrupt occurs\n");

	if (irq_val & PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_INT_STATUS_MASK) {
		common_val = readl(perf_struct->regs + PERF_MONITOR_P_COMMON);
		common_val &= ~REG_WIN_EN;  // clear reg_win_en
		writel(common_val, perf_struct->regs + PERF_MONITOR_P_COMMON);
	}

	g_startflag = 0;

	// oneshot or loop mode
	if (irq_val & PERF_MONITOR_IRQ_IRQ_INT_STATUS_PERF_DATA_INT_STATUS_MASK) {
		dev_info(perf_mon_dev->dev, "Perf data %d mode occurs.(0:loop,4:oneshot)\n",
				(readl(perf_struct->regs + PERF_MONITOR_P_COMMON) & LOOP_MODE_BIT));

		schedule_work(&perf_struct->work);
	}

	return IRQ_HANDLED;
}

static void perf_delete_raw_file(void)
{
	struct path path;
	int err;
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	struct dentry *parent;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return;
	}

	err = kern_path(perf_mon_dev->fpath, 0, &path);
	if (err) {
		pr_info("Failed to get path for %s: %d\n", perf_mon_dev->fpath, err);
		return;
	}

	parent = dget_parent(path.dentry);
	inode_lock_nested(d_inode(parent), I_MUTEX_PARENT);
	err = vfs_unlink(mnt_idmap(path.mnt), d_inode(parent), path.dentry, NULL);
	if (err) {
		pr_err("Failed to delete %s: %d\n", perf_mon_dev->fpath, err);
		goto out_err;
	}
	pr_info("deleted file %s: %d\n", perf_mon_dev->fpath, err);

out_err:
	inode_unlock(d_inode(parent));
	dput(parent);
	path_put(&path);
}

/*******************************************************************************
 * Function:		perf_start
 * Description:		set perf monitor config and enable it
 * Input:		num_of_chn, pattern, accuracy, time
 * Output:		NA
 * Return:		0
 ********************************************************************************/
int perf_start(u32 num_of_chn_l, u64 num_of_chn_h, enum pattern_type pattern, uint32_t accuracy,
		uint32_t time)
{
	u64 num_of_chn;
	int set_ddr_channel = 0;

	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}

	num_of_chn = (num_of_chn_h << BIT_NUM) | num_of_chn_l;

	pr_info("num_of_chn = %llx\n", num_of_chn);

	if (!g_initflag) {
		dev_err(perf_mon_dev->dev, "perf probe init failed!\n");
		return -EINVAL;
	}

	if (g_startflag == 1) {
		dev_err(perf_mon_dev->dev, "failed to start perf is running...\n");
		return -EINVAL;
	}

	g_startflag = 1;

	perf_delete_raw_file();

	writel(perf_mon_dev->perf_clk_value, perf_mon_dev->perf_clk_reg);

	interrupt_mask();

	mid_set(perf_mon_dev);

	set_ddr_channel = ddr_channel_set();
	if (set_ddr_channel) {
		dev_err(perf_mon_dev->dev, "failed to set ddr channel!\n");
		return -EINVAL;
	}

	// first clear common cfg
	writel(0, perf_mon_dev->regs + PERF_MONITOR_P_COMMON);

	perf_monitor_cfg(perf_mon_dev, num_of_chn, pattern, accuracy, time);

	perf_monitor_enable(perf_mon_dev);

	return 0;
}

/*******************************************************************************
 * Function:		perf_stop
 * Description:		stop perf monitor, this is only used in loop mode
 * Input:		NA
 * Output:		NA
 * Return:		0
 ********************************************************************************/
int perf_stop(void)
{
	struct perf_mon *perf_mon_dev = get_perf_monitor();

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}

	perf_monitor_disable(perf_mon_dev);

	g_startflag = 0;

	dev_info(perf_mon_dev->dev, "Perf stop!\n");

	return 0;
}

static int stat_set(void)
{
	int i = 0;
	int ret = 0;
	int crg_count_elems = 0;
	u32 reg_addr;
	u32 reg_val;
	u32 clk_reg_addr;
	u32 clk_reg_value;
	u32 time_shift;
	void __iomem *controller_crg_reg;
	void __iomem *perf_clk_reg;
	struct perf_mon *perf_mon_dev = get_perf_monitor();
	struct device_node *np = NULL;

	if (perf_mon_dev == NULL) {
		pr_err("failed to get perf_monitor!\n");
		return -ENOMEM;
	}
	np = perf_mon_dev->np;

	crg_count_elems = of_property_count_u32_elems(np, "controller-reset-reg");
	if (crg_count_elems < 0) {
		dev_err(perf_mon_dev->dev, "No %s property found\n", "reset-reg");
		return -EINVAL;
	}

	for (i = 0; i < (crg_count_elems / 2); i++) {
		ret = of_property_read_u32_index(np, "controller-reset-reg", i * 2, &reg_addr);
		if (ret < 0) {
			dev_err(perf_mon_dev->dev, "failed to get reg address!\n");
			return ret;
		}

		ret = of_property_read_u32_index(np, "controller-reset-reg", i * 2 + 1, &reg_val);
		if (ret < 0) {
			dev_err(perf_mon_dev->dev, "failed to get reg value!\n");
			return ret;
		}

		dev_info(perf_mon_dev->dev, "i is %d, crg address is 0x%x, crg value is 0x%x\n",
			i, reg_addr, reg_val);

		controller_crg_reg = devm_ioremap(perf_mon_dev->dev, reg_addr, 0x100);
		if (!controller_crg_reg) {
			dev_err(perf_mon_dev->dev, "failed to map reset register\n");
			return -ENOMEM;
		}

		writel(reg_val, controller_crg_reg);
	}

	ret = of_property_read_u32_index(np, "controller-clk", 0, &clk_reg_addr);
	perf_clk_reg = devm_ioremap(perf_mon_dev->dev, clk_reg_addr, 0x100);
	ret = of_property_read_u32_index(np, "controller-clk", 1, &clk_reg_value);
	perf_mon_dev->perf_clk_reg = perf_clk_reg;
	perf_mon_dev->perf_clk_value = clk_reg_value;

	ret = of_property_read_u32_index(np, "time-shift", 0, &time_shift);
	perf_mon_dev->time_shift = time_shift;

	return 0;
}

static int ddr_channel_set(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_PERF_MONITOR_CFG, X1_DDR, 0, 0, 0, 0, 0, 0, &res);

	return 0;
}

static int perf_alloc_pages(struct device *dev, struct perf_mon *perf_struct)
{
	struct device_node *node;
	phys_addr_t mem_phys;
	size_t mem_size;
	struct reserved_mem *rsv_mem = NULL;

	node = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!node) {
		dev_err(dev, "no memory-region specified\n");
		return -EINVAL;
	}

	rsv_mem = of_reserved_mem_lookup(node);
	if (!rsv_mem) {
		pr_err("Xring heap: %s is not find in reserved memory list\n", node->name);
		return -ENOMEM;
	}

	mem_phys = rsv_mem->base;
	mem_size = rsv_mem->size;

	pr_info("start: 0x%llx, size: 0x%lx\n", mem_phys, mem_size);

	perf_struct->base_addr = mem_phys;
	perf_struct->mem_size = mem_size;

	return 0;
}

static int perf_monitor_probe(struct platform_device *pdev)
{
	int ret = 0;
	int set_reset_reg = 0;
	const char *fpath = NULL;
	void *mem = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct perf_mon *perf_struct = NULL;
	void *actrl_perf_disable_addr = NULL;

	dev_info(dev, "Perf monitor loaded!\n");

	actrl_perf_disable_addr = ioremap((ACPU_LPIS_ACTRL + LPIS_ACTRL_SC_RSV_NS_16), 0x100);
	if (readl(actrl_perf_disable_addr) &
		LPIS_ACTRL_PERFMONITOR_REG_PERFMONITOR_DIS_MASK) {
		dev_info(dev, "perf should close.\n");
		return -EINVAL;
	}

	perf_struct = devm_kzalloc(dev, sizeof(struct perf_mon), GFP_KERNEL);
	if (!perf_struct)
		return -ENOMEM;
	dev_info(dev, "success to allocate perf_struct memory!\n");

	ret = perf_alloc_pages(dev, perf_struct);
	if (ret) {
		dev_err(dev, "failed to alloc pages:%d!\n", ret);
		return ret;
	}

	mem = memremap(perf_struct->base_addr, perf_struct->mem_size, MEMREMAP_WB);
	if (!mem) {
		dev_err(dev, "failed to map device memory!\n");
		return -ENOMEM;
	}

	fpath = of_get_property(np, "file-path", NULL);
	if (!fpath) {
		dev_err(dev, "No %s specified!\n", "file-path");
		return -EINVAL;
	}

	perf_struct->regs = devm_platform_ioremap_resource(pdev, 0);
	if (perf_struct->regs == NULL) {
		dev_err(dev, "Failed to ioremap device!\n");
		return -ENOMEM;
	}

	perf_struct->pfile = devm_kzalloc(dev, sizeof(struct perf_save_file), GFP_KERNEL);

	perf_struct->np = np;
	perf_struct->mem = mem;
	perf_struct->dev = dev;
	perf_struct->fpath = fpath;

	set_perf_monitor(perf_struct);

	set_reset_reg = stat_set();
	if (set_reset_reg) {
		dev_err(perf_struct->dev, "failed to set reset register!\n");
		return -EINVAL;
	}

	ret = perf_regulator(dev);
	if (ret) {
		dev_err(perf_struct->dev, "failed to register regulator!\n");
		return -EINVAL;
	}

	ret = platform_get_irq_optional(pdev, 0);
	if (!ret) {
		dev_err(perf_struct->dev, "failed to get irq!\n");
		return -EINVAL;
	}

	if (ret > 0) {
		ret = request_irq(ret, perf_monitor_interrupt, IRQF_SHARED, pdev->name,
				perf_struct);
		if (ret) {
			dev_err(perf_struct->dev, "failed to request irq!\n");
			return ret;
		}
	} else {
		dev_err(perf_struct->dev, "No IRQ for perf monitor!\n");
		return ret;
	}
	INIT_WORK(&perf_struct->work, perf_save_work);

	platform_set_drvdata(pdev, perf_struct);

	g_initflag = 1;
	dev_info(dev, "perf monitor driver init success!\n");

	return 0;
}

static int perf_monitor_remove(struct platform_device *pdev)
{
	int ret = 0;
	int i;

	struct perf_mon *perf_struct = platform_get_drvdata(pdev);

	ret = platform_get_irq_optional(pdev, 0);
	if (!ret) {
		dev_err(perf_struct->dev, "failed to get irq!\n");
		return -EINVAL;
	}

	for (i = 0; i < REGULATOR_MAX; ++i) {
		ret = regulator_unregister_notifier(perf_regulator_cb[i].my_regulator,
			&(perf_regulator_cb[i].my_notifier));
		if (ret)
			pr_err("Failed to unregister notifier: %d\n", ret);

		regulator_put(perf_regulator_cb[i].my_regulator);
	}

	devm_kfree(&pdev->dev, perf_struct);
	free_irq(ret, perf_struct);
	dev_info(perf_struct->dev, "perf monitor removed!\n");

	return 0;
}

static int perf_runtime_suspend(struct device *dev)
{
	return 0;
}

static int perf_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops perf_monitor_pm_ops = {
	SET_RUNTIME_PM_OPS(
		perf_runtime_suspend,
		perf_runtime_resume,
		NULL)
};

static const struct of_device_id perf_of_match[] = {
	{ .compatible = "xring,perf_monitor", },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, perf_of_match);

static struct platform_driver dts_perf_driver = {
	.probe = perf_monitor_probe,
	.remove = perf_monitor_remove,
	.driver = {
		.name = DEV_NAME,
		.of_match_table = of_match_ptr(perf_of_match),
		.pm = &perf_monitor_pm_ops,
	},
};

module_platform_driver(dts_perf_driver);

MODULE_SOFTDEP("pre: xr_regulator");

MODULE_AUTHOR("Wang Jiawei");
MODULE_DESCRIPTION("Perf driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
