// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/panic_notifier.h>
#include <dt-bindings/xring/platform-specific/isp_bus.h>
#include <dt-bindings/xring/platform-specific/main_bus.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#include "noc.h"
#include "noc_exception.h"

#define MAX_ADDR	0x83fffffff
#define CPU_MID_MIN	77
#define CPU_MID_MAX	82

static struct noc_dev *g_noc;
static struct noc_errlog *g_errlog;
static struct statistic *g_noc_statis;

static char *noc_get_desc(struct device_node *np,
		const char *desc_str,
		unsigned int desc_num)
{
	int ret;
	const char *desc_ret = NULL;

	ret = of_property_read_string_index(np, desc_str, desc_num, (const char **)&desc_ret);
	if (ret < 0) {
		pr_err("[xr_dfx][noc]: Cannot parse data\n");
		return NULL;
	}

	return (char *)desc_ret;
}

static int noc_name_parse(struct noc_dev *noc_dev)
{
	u32 i;
	struct noc_platdata *parseddata = noc_dev->parseddata;

	for (i = 0; i < noc_dev->bus_array; ++i) {
		parseddata->bus_name[i] = noc_get_desc(noc_dev->dev->of_node, "bus-name", i);
		if (parseddata->bus_name[i] == NULL) {
			dev_err(noc_dev->dev, "Cannot parse bus_name data\n");
			return -ENXIO;
		}
	}

	noc_dev->mid_max = of_property_count_strings(noc_dev->dev->of_node, "mid-name");
	if (noc_dev->mid_max < 0) {
		dev_err(noc_dev->dev, "mid-name property found\n");
		return -ENXIO;
	}

	for (i = 0; i < noc_dev->mid_max; ++i) {
		parseddata->mid_name[i] = noc_get_desc(noc_dev->dev->of_node, "mid-name", i);
		if (parseddata->mid_name[i] == NULL) {
			dev_err(noc_dev->dev, "Cannot parse mid_name data\n");
			return -ENXIO;
		}
	}

	for (i = 0; i <= ERRCODE_MAX; ++i) {
		parseddata->errcode[i] = noc_get_desc(noc_dev->dev->of_node, "errcode", i);
		if (parseddata->errcode[i] == NULL) {
			dev_err(noc_dev->dev, "Cannot parse errcode\n");
			return -ENXIO;
		}
	}

	return 0;
}

static u64 read_xrse_idle(struct noc_dev *noc_dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_MNTN_XRSE_IDLE_REG, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != SMC_OK) {
		dev_err(noc_dev->dev, "Read xrse idle fail\n");
		return -EINVAL;
	}
	return (u64)res.a1;
}

long get_targ_addr(struct noc_dev *noc_dev, int bus_num, struct routeid_s *routeid,
		u32 *bus_info_index)
{
	struct noc_platdata *parseddata = NULL;
	u64 init_addr = 0;
	long targ_addr = 0;
	u32 i = 0;
	u32 j = 0;

	if (noc_dev == NULL) {
		pr_err("noc_dev is null\n");
		return -EINVAL;
	}

	if (routeid == NULL) {
		dev_err(noc_dev->dev, "routeid is null\n");
		return -EINVAL;
	}

	if (bus_info_index == NULL) {
		dev_err(noc_dev->dev, "bus_info_index is null\n");
		return -EINVAL;
	}

	parseddata = noc_dev->parseddata;

	if (parseddata == NULL) {
		dev_err(noc_dev->dev, "parseddata is null\n");
		return -EINVAL;
	}

	for (i = 0; i < routeid->bus_info_max; ++i) {
		if (bus_info[bus_num][i].init_flow == routeid->init_id &&
		    bus_info[bus_num][i].targ_flow == routeid->target_id &&
		    bus_info[bus_num][i].targ_sub == routeid->sub_id) {
			init_addr = bus_info[bus_num][i].init_addr & INITLOCAL_MASK;
			targ_addr = bus_info[bus_num][i].targ_addr;

			for (j = 0; j < noc_dev->main_bus_info_count; j++) {
				if (bus_num == noc_dev->main_bus_info[j] && init_addr < noc_dev->remap_ddrend
						&& init_addr > noc_dev->remap_ddrstart)
					targ_addr = init_addr;
			}
			*bus_info_index = i;
			break;
		}

		/*
		 * If no matching entry is found after traversing bus_info,
		 * an exception will be printed.
		 */
		if (i == g_info_size[bus_num] - 1) {
			dev_err(noc_dev->dev, "g_info_size[bus_num] = %d\n", g_info_size[bus_num]);
			dev_err(noc_dev->dev, "bus_info is wrong.\n");
			parseddata->init_flow_string = "N/A";
			parseddata->targ_flow_string = "N/A";
			parseddata->mid_num = MID_MIN;
			return -EINVAL;
		}
	}

	return targ_addr;
}

static int noc_logging_parse_route(struct noc_dev *noc_dev, int bus_num)
{
	u32 mid_num, errlog1, errlog0, errlog5, errlog3;
	u32 bus_info_index = 0;
	u64 errlog4;
	u64 init_addr = 0;
	long targ_addr = 0;
	u32 bus_idle;
	u32 cpuid = 0;
	struct routeid_s routeid = {0};

	struct noc_platdata *parseddata = noc_dev->parseddata;

	if (bus_num == XRSE_NUM)
		bus_idle = read_xrse_idle(noc_dev) & noc_dev->bus_idle_mask[bus_num];
	else
		bus_idle = readl(noc_dev->idle_regs[bus_num]) & noc_dev->bus_idle_mask[bus_num];
	if (bus_idle != BUS_NOIDLE) {
		dev_err(noc_dev->dev, "%s is idle", parseddata->bus_name[bus_num]);
		return -ENXIO;
	}

	errlog0 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG0);
	errlog1 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG1);
	errlog3 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG3);
	errlog4 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG4);
	errlog5 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG5);

	/* parsing init_id、target_id、sub_id from errlog1 */
	routeid.init_id = (errlog1 & init_info[bus_num].init_mask) >> init_info[bus_num].init_shift;
	routeid.target_id = (errlog1 & target_info[bus_num].target_mask) >>
		target_info[bus_num].target_shift;
	routeid.sub_id = (errlog1 & sub_info[bus_num].sub_mask) >> sub_info[bus_num].sub_shift;

	routeid.bus_info_max = noc_dev->bus_info_max;

	/* parsing mid_num */
	if (bus_num != noc_dev->errlog6_bus[ERRLOG6_MAIN_BUS] &&
	    bus_num != noc_dev->errlog6_bus[ERRLOG6_MAIN_BUS_CPU] &&
	    bus_num != noc_dev->errlog6_bus[ERRLOG6_MAIN_BUS_GPU])
		mid_num = errlog5 & MID_MASK;
	else
		mid_num = (errlog5 & MAINBUS_MID_MASK) >> MAINBUS_MID_SHIFT;

	if (mid_num > noc_dev->mid_max) {
		dev_err(noc_dev->dev, "chip error, mid num %d is error\n", mid_num);
		parseddata->init_flow_string = "N/A";
		parseddata->targ_flow_string = "N/A";
		parseddata->mid_num = MID_MIN;
		parseddata->mid_name[parseddata->mid_num - 1] = "N/A";
		return -EINVAL;
	}

	if (bus_num == noc_dev->errlog6_bus[ERRLOG6_CPU_BUS])
		cpuid = (errlog5 & SRCATTR_MASK) >> SRCATTR_SHIFT;

	/* parsing errcode and r/w */
	parseddata->errcode_num = (errlog0 & MAIN_BUS_ERR_MAIN_0_ERRLOG0_ERRCODE_MASK) >>
					MAIN_BUS_ERR_MAIN_0_ERRLOG0_ERRCODE_SHIFT;
	if (parseddata->errcode_num > ERRCODE_MAX) {
		dev_err(noc_dev->dev, "errcode num is error\n");
		return -EINVAL;
	}

	parseddata->opc = (errlog0 & MAIN_BUS_ERR_MAIN_0_ERRLOG0_OPC_MASK) >>
				       MAIN_BUS_ERR_MAIN_0_ERRLOG0_OPC_SHIFT;

	if (parseddata->opc > OPC_MAX) {
		dev_err(noc_dev->dev, "opc num is error\n");
		return -EINVAL;
	}

	/* match through init_flow、targ_flow、targ_sub to obtain init_addr, targ_addr */
	targ_addr = get_targ_addr(noc_dev, bus_num, &routeid, &bus_info_index);
	if (targ_addr < 0) {
		dev_err(noc_dev->dev, "get targ addr failed\n");
		return -EINVAL;
	}

	parseddata->init_flow_string = bus_info[bus_num][bus_info_index].init_flow_string;
	parseddata->targ_flow_string = bus_info[bus_num][bus_info_index].targ_flow_string;
	parseddata->init_addr = init_addr | errlog3 + (errlog4 ? (errlog4 << 32) : 0);
	parseddata->target_addr = targ_addr | errlog3 + (errlog4 ? (errlog4 << 32) : 0);

	parseddata->subrange = routeid.sub_id;

	parseddata->mid_num = mid_num;
	parseddata->cpuid = cpuid;

	return 0;
}

static void noc_setdata(struct noc_errlog *errlog_reg)
{
	g_errlog = errlog_reg;
}

struct noc_errlog *noc_getdata(void)
{
	return g_errlog;
}
EXPORT_SYMBOL(noc_getdata);

static void pr_nocerrlog(struct noc_dev *noc_dev, u32 bus_num, struct noc_errlog *errlog_reg)
{
	struct noc_platdata *parseddata = noc_dev->parseddata;


	dev_err(noc_dev->dev, "\n=============================================\n"
		" bus_name:%s  Errlog Raw Registers\n"
		"\tErrvld   : 0x%x\n",
		parseddata->bus_name[bus_num], errlog_reg->errvld);

	pr_err("\tErrLog0  : 0x%x, Errcode : 0x%x : %s, r/w(r:0, 1, 2, 3) : %d\n",
		errlog_reg->errlog0, parseddata->errcode_num,
		parseddata->errcode[parseddata->errcode_num], parseddata->opc);

	pr_err("\tErrLog1  : 0x%x\n"
		"\t	   initflow:%s\n"
		"\t	   targetflow:%s\n"
		"\t	   targetaddr:0x%llx\n",
		errlog_reg->errlog1, parseddata->init_flow_string,
		parseddata->targ_flow_string, parseddata->target_addr);

	pr_err("\tErrLog2  : 0x%x\n"
		"\tErrLog3  : 0x%x\n"
		"\tErrLog4  : 0x%x\n",
		errlog_reg->errlog2, errlog_reg->errlog3, errlog_reg->errlog4);

	pr_err("\tErrLog5  : 0x%x, mid : 0x%x : %s, cpuid : 0x%x\n"
		"\tErrLog6  : 0x%x\n"
		"\tErrLog7  : (0: Security, 1:NonSecurity)  : 0x%x\n"
		"=============================================\n",
		errlog_reg->errlog5, parseddata->mid_num,
		parseddata->mid_name[parseddata->mid_num], parseddata->cpuid,
		errlog_reg->errlog6, errlog_reg->errlog7);

	set_noc_exception_list(parseddata->mid_num, parseddata->errcode_num);
	noc_dma_exception(&noc_dev->noc_dma_s, parseddata->mid_name[parseddata->mid_num],
			parseddata->mid_num, parseddata->target_addr, parseddata->opc);
}

static void noc_logging_dump_raw(struct noc_dev *noc_dev, u32 bus_num)
{
	struct noc_errlog *errlog_reg = noc_dev->errlog_reg;

	errlog_reg->errvld = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRVLD);
	errlog_reg->errlog0 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG0);
	errlog_reg->errlog1 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG1);
	errlog_reg->errlog3 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG3);
	errlog_reg->errlog4 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG4);
	errlog_reg->errlog5 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG5);
	errlog_reg->errlog6 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG6);
	errlog_reg->errlog7 = readl(noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRLOG7);

	noc_setdata(errlog_reg);

	pr_nocerrlog(noc_dev, bus_num, errlog_reg);

	writel(ERRCLR, noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRCLR);
}

static void noc_logging_dump(struct noc_dev *noc_dev, u32 bus_num)
{
	int ret = 0;

	ret = noc_logging_parse_route(noc_dev, bus_num);

	if (ret) {
		dev_err(noc_dev->dev, "noc logging parse failed\n");
		writel(ERRCLR, noc_dev->regs[bus_num] + MAIN_BUS_ERR_MAIN_0_ERRCLR);
		return;
	}

	noc_logging_dump_raw(noc_dev, bus_num);
}

static void noc_bus_error(struct noc_dev *noc_dev, bool is_panic)
{
	int i = 0;
	u32 index;
	u32 intr_state;
	u32 id = 0;
	u32 index_max = (1 << noc_dev->bus_array) - 1;

	/* readl interrupt aggregation register */
	index = readl(noc_dev->err_irq_reg + EXC0_RAW_ST);
	intr_state = index;

	if (index > index_max) {
		dev_err(noc_dev->dev, "index is out of range.\n");
		return;
	}

	dev_err(noc_dev->dev, "noc intr = 0x%x\n", index);

	/* Traverse each bit of the index. If the current bit is 1,
	 * print the register information. Otherwise, determine whether
	 * the value of the next bit in the segment is 1.
	 */
	while (index > 0) {
		id = index & 0x1;
		if (id == IRQ_VALID)
			noc_logging_dump(noc_dev, i);
		i++;
		index = index >> 1;
	}

	/* no noc intr or in panic process, do not run noc_exception_process */
	if (intr_state && !is_panic) {
		noc_dma_process(&noc_dev->noc_dma_s);
		noc_exception_process();
	}
}

static int noc_bus_error_notify(struct notifier_block *nb, unsigned long event, void *buf)
{
	struct noc_dev *noc_dev = get_noc();

	noc_bus_error(noc_dev, true);

	return 0;
}

static struct notifier_block noc_bus_error_block = {
	.notifier_call = noc_bus_error_notify,
	.priority = INT_MAX,
};

static irqreturn_t noc_error_irq(int irq, void *data)
{
	struct noc_dev *noc_dev = (struct noc_dev *)data;

	noc_bus_error(noc_dev, false);

	return IRQ_HANDLED;
}

static void set_noc(struct noc_dev *noc_dev)
{
	g_noc = noc_dev;
}

struct noc_dev *get_noc(void)
{
	return g_noc;
}

void *get_isp_bus_base(void)
{
	return g_noc_statis->isp_bus_base;
}

void *get_venc_bus_base(void)
{
	return g_noc_statis->venc_bus_base;
}

void *get_dpu_bus_base(void)
{
	return g_noc_statis->dpu_bus_base;
}

static int noc_dt_parse(struct device_node *np,
		struct noc_dev *noc_dev)
{
	int ret;

	if (!np || !noc_dev)
		return -EINVAL;
	/* obtain the maximum number of interrupts and the maximum number of bus infos */
	ret = of_property_read_u32(np, "bus-array", &noc_dev->bus_array);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus-array.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "bus-info-max", &noc_dev->bus_info_max);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus-info-max.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "bus-intr-exc0", &noc_dev->bus_intr_exc0);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus-intr-exc0.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "log6_num", &noc_dev->log6_num);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get log6_num.\n");
		return ret;
	}

	ret = of_property_read_u32_array(np, "errlog6_bus", noc_dev->errlog6_bus,
					 noc_dev->log6_num);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get errlog6_bus.\n");
		return ret;
	}

	noc_dev->main_bus_info_count = of_property_count_u32_elems(np, "main_bus_info");
	if (noc_dev->main_bus_info_count < 0) {
		dev_err(noc_dev->dev, "No main bus info property found\n");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "main_bus_info", noc_dev->main_bus_info,
			noc_dev->main_bus_info_count);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus_idle_mask.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "remap_ddrstartl", &noc_dev->remap_ddrstartl);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get remap_ddrstartl.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "remap_ddrstarth", &noc_dev->remap_ddrstarth);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get remap_ddrstarth.\n");
		return ret;
	}

	noc_dev->remap_ddrstart = noc_dev->remap_ddrstartl + ((u64)noc_dev->remap_ddrstarth << 32);

	ret = of_property_read_u32(np, "remap_ddrendl", &noc_dev->remap_ddrendl);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get remap_ddrendl.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "remap_ddrendh", &noc_dev->remap_ddrendh);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get remap_ddrendh.\n");
		return ret;
	}

	noc_dev->remap_ddrend = noc_dev->remap_ddrendl + ((u64)noc_dev->remap_ddrendh << 32);

	ret = of_property_read_u32(np, "ddr_maxl", &noc_dev->ddr_maxl);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get ddr_maxl.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "ddr_maxh", &noc_dev->ddr_maxh);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get ddr_maxh.\n");
		return ret;
	}

	noc_dev->ddr_max = noc_dev->ddr_maxl + ((u64)noc_dev->ddr_maxh << 32);

	pr_err("[xr_dfx][noc]: remap_ddrstart = 0x%llx", noc_dev->remap_ddrstart);
	pr_err("[xr_dfx][noc]: remap_ddrend = 0x%llx", noc_dev->remap_ddrend);
	pr_err("[xr_dfx][noc]: ddrmax = 0x%llx", noc_dev->ddr_max);

	ret = of_property_read_u32(np, "main_bus_routeidmask", &noc_dev->main_bus_routeidmask);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get main_bus_routeidmask.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "lpis_bus_routeidmask", &noc_dev->lpis_bus_routeidmask);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get lpis_bus_routeidmask.\n");
		return ret;
	}

	ret = of_property_read_u32_array(np, "bus_idle_addr", noc_dev->bus_idle_addr,
					 noc_dev->bus_array);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus_idle_addr.\n");
		return ret;
	}

	ret = of_property_read_u32_array(np, "bus_idle_mask", noc_dev->bus_idle_mask,
					 noc_dev->bus_array);
	if (ret) {
		dev_err(noc_dev->dev, "Failed to get bus_idle_mask.\n");
		return ret;
	}

	return 0;
}

static int noc_poweridle_parse(struct platform_device *pdev, struct noc_dev *noc_dev)
{
	int i;

	for (i = 0; i < noc_dev->bus_array; ++i) {
		noc_dev->idle_regs[i] = devm_ioremap(&pdev->dev, noc_dev->bus_idle_addr[i],
				sizeof(u32));
		if (!noc_dev->idle_regs[i]) {
			dev_err(&pdev->dev, "Cannot map device memory\n");
			return -ENOMEM;
		}
	}

	return 0;
}

static int noc_irq_parse(struct platform_device *pdev, struct noc_dev *noc_dev)
{
	int ret;

	/* remap interrupt aggregation register */
	noc_dev->err_irq_reg = devm_ioremap(&pdev->dev, noc_dev->bus_intr_exc0, MASK_SIZE);
	if (!noc_dev->err_irq_reg) {
		dev_err(&pdev->dev, "Cannot map device memory\n");
		return -ENOMEM;
	}

	writel(NOMASK, noc_dev->err_irq_reg + MASK_L);
	writel(NOMASK, noc_dev->err_irq_reg + MASK_H);

	ret = platform_get_irq_optional(pdev, 0);

	if (ret > 0) {
		ret = devm_request_irq(&pdev->dev, ret, noc_error_irq, IRQF_SHARED, pdev->name,
				noc_dev);

		if (ret) {
			dev_err(&pdev->dev, "failed to request irq\n");
			return ret;
		}
	} else {
		dev_err(&pdev->dev, "No IRQ for noc\n");
		return ret;
	}

	return 0;
}

static int noc_probe(struct platform_device *pdev)
{
	int ret;
	int i;
	struct noc_dev *noc_dev = NULL;
	struct noc_platdata *parseddata = NULL;
	struct noc_errlog *errlog_reg = NULL;

	/* resource request */
	noc_dev = devm_kzalloc(&pdev->dev, sizeof(struct noc_dev), GFP_KERNEL);
	if (!noc_dev) {
		dev_err(&pdev->dev, "failed to allocate memory for driver's noc_dev data\n");
		return -ENOMEM;
	}
	noc_dev->dev = &pdev->dev;

	errlog_reg = devm_kzalloc(&pdev->dev, sizeof(struct noc_errlog), GFP_KERNEL);
	if (!errlog_reg) {
		dev_err(&pdev->dev, "failed to allocate memory for errlog_reg\n");
		return -ENOMEM;
	}
	noc_dev->errlog_reg = errlog_reg;

	parseddata = devm_kzalloc(&pdev->dev, sizeof(struct noc_platdata), GFP_KERNEL);
	if (!parseddata) {
		dev_err(&pdev->dev, "failed to allocate memory for driver's parseddata\n");
		return -ENOMEM;
	}
	noc_dev->parseddata = parseddata;

	ret = noc_dt_parse(pdev->dev.of_node, noc_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to assign device tree parsing\n");
		return ret;
	}

	ret = noc_poweridle_parse(pdev, noc_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse poweridle\n");
		return ret;
	}

	ret = noc_name_parse(noc_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse noc name\n");
		return ret;
	}

	/* noc bus has 29 errlog base adds */
	for (i = 0; i < noc_dev->bus_array; ++i) {
		noc_dev->regs[i] = devm_platform_ioremap_resource(pdev, i);

		if (noc_dev->regs[i] == NULL) {
			dev_err(&pdev->dev, "failed to claim register region\n");
			return -ENOENT;
		}
	}

	ret = atomic_notifier_chain_register(&panic_notifier_list, &noc_bus_error_block);
	if (ret) {
		dev_err(&pdev->dev, "Fail to register noc notifier\n");
		return ret;
	}

	ret = noc_irq_parse(pdev, noc_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to parse irq\n");
		return ret;
	}

	platform_set_drvdata(pdev, noc_dev);

	set_noc(noc_dev);

	noc_exception_wq_init();

	ret = noc_dma_process_init(&noc_dev->noc_dma_s);
	if (ret)
		dev_err(&pdev->dev, "noc_dma_process_init failed\n");

	pr_info("[xr_dfx][noc]: %s ==> probe\n", __func__);

	return 0;
}

static int noc_remove(struct platform_device *pdev)
{
	struct noc_dev *noc_dev = platform_get_drvdata(pdev);
	int ret = 0;

	ret = noc_dma_process_exit(&noc_dev->noc_dma_s);
	if (ret)
		dev_err(&pdev->dev, "noc_dma_process_exit failed\n");

	atomic_notifier_chain_unregister(&panic_notifier_list, &noc_bus_error_block);
	devm_kfree(&pdev->dev, noc_dev);

	return ret;
}

static const struct of_device_id noc_of_match[] = {
	{ .compatible = "xring,arteris-noc",
	  .data = NULL, },
	{},
};

static struct platform_driver noc_driver = {
	.probe     = noc_probe,
	.remove    = noc_remove,
	.driver    = {
		.name  = "arteris-noc",
		.of_match_table = of_match_ptr(noc_of_match),
	},
};

module_platform_driver(noc_driver);

MODULE_AUTHOR("Ren Jie");
MODULE_DESCRIPTION("Arteris noc Driver");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
