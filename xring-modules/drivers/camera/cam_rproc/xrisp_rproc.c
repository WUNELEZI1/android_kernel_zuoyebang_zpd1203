// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rproc", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rproc", __func__, __LINE__

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/msi.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_device.h>
#include <linux/remoteproc.h>
#include <linux/firmware.h>
#include <linux/printk.h>
#include <linux/iommu.h>
#include <linux/dma-heap.h>
#include <linux/dma-buf.h>
#include <linux/kthread.h>
#include <linux/iova.h>
#include <linux/types.h>
#include <linux/arm-smccc.h>
#include <linux/time.h>
#include <soc/xring/xr_timestamp.h>
#include <soc/xring/xring_smmu_wrapper.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/media2_crg.h>
#include <dt-bindings/xring/platform-specific/isp_crg.h>
#include <dt-bindings/xring/platform-specific/isp_mcu_ctl.h>
#include <dt-bindings/xring/platform-specific/isp_dma.h>

#include "remoteproc_internal.h"
#include "remoteproc_elf_helpers.h"
#include "xrisp_rproc.h"
#include "xrisp_address_map.h"
#include "cam_ctrl/xrisp_cam_ctrl.h"
#include "xrisp_pm_api.h"
#include "xrisp_rproc_api.h"
#include "xrisp_tee_ca_api.h"
#include "xrisp_debug.h"
#include "xrisp_log.h"
#include "xrisp_ramlog.h"
#include "soc/xring/xr_dmabuf_helper.h"

// TODO: Just for temporary solution
#define DDR_DVFS_VOTE_ISP      6
#include <soc/xring/ddr_vote_dev.h>

#define ISP_MCU_DMA_CLK_FLAG 0x1
#define ISP_MCU_DMA_RST_FLAG 0x2

#define CONFIG_XRISP_RPROC_RESERVE_CMA

struct cam_rproc *g_xrproc;

struct task_struct *provider_task;
struct task_struct *current_task;
static unsigned long g_poweroff_rss_count;
/* XRISP_ISP_SC_BASE */
struct m2_register isp_top_smmu_bypass[] = {
	/* isp sc do not have platform-specific file */
	{0xc, 0x0, M2_READ_AFTER_WRITE, 1, "isp smmu bypass"},
};

/* XRISP_M2_CRG_BASE */
struct m2_register m2_crg_r82_dereset[] = {
	{ MEDIA2_CRG_RST1_RO, 0x0, M2_READ, 0, "r82 reset status"},
	{ MEDIA2_CRG_RST1_W1S, 0x80, M2_WRITE, 0, "r82 dereset" },
	{ MEDIA2_CRG_RST1_RO, 0x0, M2_READ, 0, "r82 reset status"},
};

struct m2_register m2_crg_r82_reset[] = {
	{ MEDIA2_CRG_RST1_RO, 0x0, M2_READ, 0, "r82 reset status"},
	{ MEDIA2_CRG_RST1_W1C, 0x80, M2_WRITE, 0, "r82 reset"},
	{ MEDIA2_CRG_RST1_RO, 0x0, M2_READ, 0, "r82 reset status"},
};

struct m2_register m2crg_subsys_debug_dereset[] = {
	{ MEDIA2_CRG_RST1_W1S, 0x4000000, M2_WRITE, 0, "m2 subsys debug dereset"},
	{ MEDIA2_CRG_CLKGT0_W1S, 0x14000, M2_WRITE, 0, "m2 subsys debug clk"},
};

struct m2_register m2crg_subsys_debug_reset[] = {
	{ MEDIA2_CRG_CLKGT0_W1C, 0x14000, M2_WRITE, 0, "m2 subsys debug clk"},
	{ MEDIA2_CRG_RST1_W1C, 0x4000000, M2_WRITE, 0, "m2 subsys debug reset"},
};

struct m2_register m2_crg_mcu_dma_reset[] = {
	{ MEDIA2_CRG_CLKST6, 0, M2_READ, 0, "mcu dma clk status" },
	{ MEDIA2_CRG_RST1_RO, 0, M2_READ, 0, "mcu dma reset status" },
	{ MEDIA2_CRG_RST1_W1C, 0x80000000, M2_WRITE, 0, "mcu dma reset" },
};

struct m2_register m2_crg_mcu_dma_dereset[] = {
	{ MEDIA2_CRG_RST1_W1S, 0x80000000, M2_WRITE, 0, "mcu dma reset"},
};

struct m2_register m2_crg_mcu_ip_reset[] = {
	{ MEDIA2_CRG_RST1_W1C, 0xF0FF017F, M2_WRITE, 0, "m2crg ip reset" },
};

struct m2_register m2_crg_mcu_ip_dereset[] = {
	{ MEDIA2_CRG_RST1_W1S, 0xF0FF017F, M2_WRITE, 0, "m2crg ip dereset" },
};

/* XRISP_ISP_CRG_BASE */
struct m2_register isp_crg_clock_dereset[] = {
	{ ISP_CRG_ISP_CLOCK_RST_0_W1S, 0xffffffff, M2_READ_AFTER_WRITE, 1, "isp crg ip dereset" },
};

struct m2_register isp_crg_clock_reset[] = {
	{ ISP_CRG_ISP_CLOCK_RST_0_W1C, 0xffffffff, M2_READ_AFTER_WRITE, 1, "isp crg ip reset" },
};

struct m2_register isp_aon_crg_clock_reset[] = {
	{ ISP_CRG_ISP_CLOCK_RST_0_W1C, 0xfffe4fc3, M2_READ_AFTER_WRITE, 1, "isp crg ip reset" },
};

struct m2_register isp_shake_crg_clock_reset[] = {
	{ ISP_CRG_ISP_CLOCK_RST_0_W1C, 0xfffc4f83, M2_READ_AFTER_WRITE, 1, "isp crg ip reset" },
};

struct m2_register isp_mcu_ctrl_halt_enable[] = {
	{ ISP_MCU_CTL_CPU_HALT0, 0x1, M2_WRITE, 0, "mcu core0 halt enable"},
	{ ISP_MCU_CTL_CPU_HALT1, 0x1, M2_WRITE, 0, "mcu core1 halt enable"},
};

struct m2_register isp_mcu_ctrl_halt_disable[] = {
	{ ISP_MCU_CTL_CPU_HALT0, 0x0, M2_WRITE, 0, "mcu core0 halt disable"},
	{ ISP_MCU_CTL_CPU_HALT1, 0x0, M2_WRITE, 0, "mcu core1 halt disable"},
};

struct m2_register isp_mcu_ctrl_rv_baraddr[] = {
	{ ISP_MCU_CTL_CFG_RV_BARADDR0_LOW, (0x80000000 >> 2), M2_WRITE, 0,
		"mcu core0 rv baseaddr"},
	{ ISP_MCU_CTL_CFG_RV_BARADDR0_HIGH, 0x0,              M2_WRITE, 0,
		"mcu core0 rv baseaddr"},
	{ ISP_MCU_CTL_CFG_RV_BARADDR1_LOW, (0x80000000 >> 2), M2_WRITE, 0,
		"mcu core1 rv baseaddr"},
	{ ISP_MCU_CTL_CFG_RV_BARADDR1_HIGH, 0x0,              M2_WRITE, 0,
		"mcu core1 rv baseaddr"},
};

enum xrisp_reg_type {
	M2_CRG,
	ISP_CRG,
	ISP_TOP,
	ISP_MCU_CTL,
	ISP_NIC,
	XRISP_REG_MAX,
};

struct xrisp_reg_map reg_map_list[XRISP_REG_MAX] = {
	[M2_CRG]      = { XRISP_M2_CRG_BASE,       XRISP_M2_CRG_SIZE,       0, NULL, "m2_crg"     },
	[ISP_CRG]     = { XRISP_ISP_CRG_BASE,      XRISP_ISP_CRG_SIZE,      1, NULL, "isp_crg"    },
	[ISP_TOP]     = { XRISP_ISP_SC_BASE,       XRISP_ISP_SC_SIZE,       1, NULL, "isp_top"    },
	[ISP_MCU_CTL] = { XRISP_ISP_MCU_CTRL_BASE, XRISP_ISP_MCU_CTRL_SIZE, 1, NULL, "isp_mcu_ctl"},
	[ISP_NIC]     = { XRISP_ISP_NIC_BASE,      XRISP_ISP_NIC_BASE,      1, NULL, "isp_nic"    },
};

static int xrisp_reg_read_smc(uint32_t addr)
{
	struct arm_smccc_res res = {0xff};

	arm_smccc_smc(FID_BL31_ISP_READ_REG, addr, 0, 0, 0, 0, 0, 0, &res);
	//XRISP_PR_INFO("%s: a0=0x%lx, a1=0x%lx, a2=0x%lx\n", __func__,
		//res.a0, res.a1, res.a2);
	if (res.a0) {
		XRISP_PR_ERROR("%s %d arm_smccc_smc error!\n", __func__, __LINE__);
		return -ENODEV;
	}

	return res.a1;
}

static int xrisp_reg_write_smc(uint32_t val, uint32_t addr)
{
	struct arm_smccc_res res = {0xff};

	arm_smccc_smc(FID_BL31_ISP_WRITE_REG, addr, val, 0, 0, 0, 0, 0, &res);
	// XRISP_PR_INFO("%s: a0=0x%lx, a1=0x%lx, a2=0x%lx\n", __func__,
		//res.a0, res.a1, res.a2);
	if (res.a0) {
		XRISP_PR_ERROR("%s %d arm_smccc_smc error!\n", __func__, __LINE__);
		return -ENODEV;
	}

	return 0;
}

static int xring_isp_reg_iomap(struct xrisp_reg_map reg_list[], uint32_t num)
{
	int i;

	for (i = 0; i < num; i++) {
		if (reg_list[i].is_atf_ctl == 1)
			continue;
		reg_list[i].va = ioremap(reg_list[i].addr, reg_list[i].size);
		if (reg_list[i].va == NULL)
			goto xrisp_iomap_err;
	}

	return 0;

xrisp_iomap_err:
	XRISP_PR_ERROR("reg[%s] iomap failed", reg_list[i].name);
	for (i = i - 1; i >= 0; i--) {
		if (reg_list[i].is_atf_ctl == 1)
			continue;
		iounmap(reg_list[i].va);
	}
	return -EINVAL;
}

static void xring_isp_reg_iounmap(struct xrisp_reg_map reg_list[], uint32_t num)
{
	int i;

	for (i = 0; i < num; i++) {
		if (reg_list[i].is_atf_ctl == 1)
			continue;
		if (reg_list[i].va != NULL)
			iounmap(reg_list[i].va);
	}
}

void xrisp_reg_setting(enum xrisp_reg_type type, struct m2_register *m2_reg,
		       uint32_t num)
{
	int           i;
	uint32_t      val;
	void __iomem  *va = reg_map_list[type].va;
	uint32_t      pa = reg_map_list[type].addr;

	for (i = 0; i < num; i++) {
		if (m2_reg[i].ops == M2_WRITE || m2_reg[i].ops == M2_READ_AFTER_WRITE) {
			if (reg_map_list[type].is_atf_ctl == 1)
				xrisp_reg_write_smc(m2_reg[i].val, pa + m2_reg[i].offset);
			else
				writel(m2_reg[i].val, va + m2_reg[i].offset);

			//XRISP_PR_DEBUG("%s write 0x%x = 0x%x", m2_reg[i].name,
			//reg_map_list[type].addr + m2_reg[i].offset,
			//m2_reg[i].val);
		}
		if (m2_reg[i].ops == M2_READ || m2_reg[i].ops == M2_READ_AFTER_WRITE
			|| m2_reg[i].ops == M2_POLL) {
			if (reg_map_list[type].is_atf_ctl == 1)
				val = xrisp_reg_read_smc(pa + m2_reg[i].offset);
			else
				val = readl(va + m2_reg[i].offset);

			m2_reg[i].val = val;
			//XRISP_PR_DEBUG("%s read 0x%x = 0x%x", m2_reg[i].name,
			//reg_map_list[type].addr + m2_reg[i].offset, m2_reg[i].val);
		}
		mdelay(m2_reg[i].msdelay);
	}
}

static void xrisp_halt(uint32_t enable)
{
	if (enable == 1)
		xrisp_reg_setting(ISP_MCU_CTL,
		    isp_mcu_ctrl_halt_enable, ARRAY_SIZE(isp_mcu_ctrl_halt_enable));
	else
		xrisp_reg_setting(ISP_MCU_CTL,
		    isp_mcu_ctrl_halt_disable, ARRAY_SIZE(isp_mcu_ctrl_halt_disable));
}

static void xrisp_set_rv_baraddr(uint64_t bootaddr)
{
	/* Update bootaddr */
	isp_mcu_ctrl_rv_baraddr[0].val = ((uint32_t)bootaddr) >> 2;
	isp_mcu_ctrl_rv_baraddr[1].val = (uint32_t)(bootaddr >> 32);
	isp_mcu_ctrl_rv_baraddr[2].val = ((uint32_t)bootaddr) >> 2;
	isp_mcu_ctrl_rv_baraddr[3].val = (uint32_t)(bootaddr >> 32);

	xrisp_reg_setting(ISP_MCU_CTL,
	    isp_mcu_ctrl_rv_baraddr, ARRAY_SIZE(isp_mcu_ctrl_rv_baraddr));
}

void xrisp_r82_reset(void)
{
	xrisp_reg_setting(M2_CRG, m2_crg_r82_reset, ARRAY_SIZE(m2_crg_r82_reset));
}

static void xrisp_r82_dereset(void)
{
	xrisp_reg_setting(M2_CRG, m2_crg_r82_dereset, ARRAY_SIZE(m2_crg_r82_dereset));
}

static void xrisp_trace_debug_open(void)
{
	xrisp_reg_setting(M2_CRG, m2crg_subsys_debug_dereset,
			  ARRAY_SIZE(m2crg_subsys_debug_dereset));
}

static void xrisp_trace_debug_close(void)
{
	xrisp_reg_setting(M2_CRG, m2crg_subsys_debug_reset,
			  ARRAY_SIZE(m2crg_subsys_debug_reset));
}

__maybe_unused void xrisp_ispmodule_reset(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;

	if (xrisp_rproc->scene == XRISP_SAFE_AON_BOOT)
		xrisp_reg_setting(ISP_CRG, isp_aon_crg_clock_reset,
				  ARRAY_SIZE(isp_aon_crg_clock_reset));
	else if (xrisp_rproc->scene == XRISP_SHAKE_BOOT)
		xrisp_reg_setting(ISP_CRG, isp_shake_crg_clock_reset,
				  ARRAY_SIZE(isp_shake_crg_clock_reset));
	else
		xrisp_reg_setting(ISP_CRG, isp_crg_clock_reset, ARRAY_SIZE(isp_crg_clock_reset));
}

__maybe_unused void xrisp_ispmodule_dereset(void)
{
	xrisp_reg_setting(ISP_CRG, isp_crg_clock_dereset,
			  ARRAY_SIZE(isp_crg_clock_dereset));
}

__maybe_unused void xrisp_m2crg_ip_reset(void)
{
	xrisp_reg_setting(M2_CRG, m2_crg_mcu_ip_reset, ARRAY_SIZE(m2_crg_mcu_ip_reset));
}

__maybe_unused void xrisp_m2crg_ip_dereset(void)
{
	xrisp_reg_setting(M2_CRG, m2_crg_mcu_ip_dereset, ARRAY_SIZE(m2_crg_mcu_ip_dereset));
}

static void xrisp_smmu_bypass_disable(void)
{
	xrisp_reg_setting(ISP_TOP, isp_top_smmu_bypass,
			  ARRAY_SIZE(isp_top_smmu_bypass));
}

static int xrisp_mcu_dma_status_check(void)
{
	uint32_t clk_status;
	uint32_t rst_status;
	int status = 0;

	xrisp_reg_setting(M2_CRG, &m2_crg_mcu_dma_reset[0], 1);
	clk_status = (m2_crg_mcu_dma_reset[0].val & MEDIA2_CRG_CLKST6_ST_CLK_ISP_MCU_DMA_MASK);
	if (clk_status)
		status |= ISP_MCU_DMA_CLK_FLAG;

	xrisp_reg_setting(M2_CRG, &m2_crg_mcu_dma_reset[1], 1);
	rst_status = (m2_crg_mcu_dma_reset[1].val & MEDIA2_CRG_RST1_RO_IP_RST_ISP_MCU_DMA_N_MASK);
	if (rst_status)
		status |= ISP_MCU_DMA_RST_FLAG;

	return status;
}

void xrisp_mcu_dma_dereset(void)
{
	xrisp_reg_setting(M2_CRG, &m2_crg_mcu_dma_dereset[0], 1);
}

static void xrisp_mcu_dma_reset(void)
{
	uint32_t status;

	status = xrisp_mcu_dma_status_check();
	XRISP_PR_INFO("mcu dma reset:0x%x.", status);
	if (status == (ISP_MCU_DMA_CLK_FLAG | ISP_MCU_DMA_RST_FLAG))
		xrisp_reg_setting(M2_CRG, &m2_crg_mcu_dma_reset[2], 1);
}

static int xrisp_crg_get_gate_st(uint32_t shift)
{
	struct m2_register isp_reg = { ISP_CRG_ISP_CLOCK_CG_ST_0, 0, M2_POLL, 0,
				       "isp_crg_clk_gate_st" };

	xrisp_reg_setting(ISP_CRG, &isp_reg, 1);
	if ((1 << shift) & isp_reg.val)
		return 1;
	else
		return 0;
}

#ifndef BL31_SUPPROT_DMA_RESET
static int xrisp_fepe_datawdma_reset(uint32_t addr, unsigned int to)
{
	unsigned long expire;
	struct m2_register isp_dma_reg[] = {
		{ addr + 0x2c - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_fepe_dma_stop" },
		{ addr + 0x34 - reg_map_list[ISP_NIC].addr, 0, M2_POLL, 0, "isp_fepe_dma_busy" },
		{ addr + 0x30 - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_fepe_dma_reset" },
		{ addr + 0x30 - reg_map_list[ISP_NIC].addr, 0, M2_WRITE, 0,
		  "isp_fepe_dma_dereset" },
		{ addr + 0x2c - reg_map_list[ISP_NIC].addr, 0, M2_WRITE, 0, "isp_fepe_dma_start" },
	};

	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[0], 2);
	expire = msecs_to_jiffies(to) + jiffies;
	while (isp_dma_reg[1].val & 0x10) { // axi busy
		if (time_is_before_eq_jiffies(expire)) {
			XRISP_PR_ERROR("isp dma wait %s time out", isp_dma_reg[1].name);
			return -ETIMEDOUT;
		}
		xrisp_reg_setting(ISP_NIC, &isp_dma_reg[1], 1);
	}
	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[2], 3);
	return 0;
}

static int xrisp_datadma_reset(uint32_t addr, unsigned int to)
{
	unsigned long expire;
	struct m2_register isp_dma_reg[] = {
		{ addr + 0x44 - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_dma_stop" },
		{ addr + 0x6c - reg_map_list[ISP_NIC].addr, 0, M2_POLL, 0, "isp_dma_busy" },
		{ addr + 0x48 - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_dma_reset" },
		{ addr + 0x48 - reg_map_list[ISP_NIC].addr, 0, M2_WRITE, 0, "isp_dma_dereset" },
		{ addr + 0x44 - reg_map_list[ISP_NIC].addr, 0, M2_WRITE, 0, "isp_dma_start" },
	};

	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[0], 2);
	expire = msecs_to_jiffies(to) + jiffies;
	while (isp_dma_reg[1].val & 0x10) { // axi busy
		if (time_is_before_eq_jiffies(expire)) {
			XRISP_PR_ERROR("isp dma wait %s time out", isp_dma_reg[1].name);
			return -ETIMEDOUT;
		}
		xrisp_reg_setting(ISP_NIC, &isp_dma_reg[1], 1);
	}
	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[2], 3);
	return 0;
}

static int xrisp_cmddma_reset(uint32_t addr, unsigned int to)
{
	unsigned long expire;
	struct m2_register isp_dma_reg[] = {
		{ addr + 0x1c - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_cmddma_stop" },
		{ addr + 0x84 - reg_map_list[ISP_NIC].addr, 0, M2_POLL, 0, "isp_cmddma_busy" },
		{ addr + 0x24 - reg_map_list[ISP_NIC].addr, 1, M2_WRITE, 0, "isp_cmddma_flush" },
		{ addr + 0x84 - reg_map_list[ISP_NIC].addr, 0, M2_POLL, 0, "isp_cmddma_busy" },
		{ addr + 0x1c - reg_map_list[ISP_NIC].addr, 0, M2_WRITE, 0, "isp_cmddma_start" },
	};

	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[0], 2);
	expire = msecs_to_jiffies(to) + jiffies;
	while (isp_dma_reg[1].val & 0xf) { // axi busy
		if (time_is_before_eq_jiffies(expire)) {
			XRISP_PR_ERROR("isp dma wait %s time out", isp_dma_reg[1].name);
			return -ETIMEDOUT;
		}
		xrisp_reg_setting(ISP_NIC, &isp_dma_reg[1], 1);
	}

	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[2], 2);
	expire = msecs_to_jiffies(to) + jiffies;
	while (isp_dma_reg[3].val & 0xf) { // axi busy
		if (time_is_before_eq_jiffies(expire)) {
			XRISP_PR_ERROR("isp dma wait %s time out", isp_dma_reg[3].name);
			return -ETIMEDOUT;
		}
		xrisp_reg_setting(ISP_NIC, &isp_dma_reg[3], 1);
	}
	xrisp_reg_setting(ISP_NIC, &isp_dma_reg[4], 1);
	return 0;
}

struct xrisp_dma_list_t {
	uint32_t start_addr;
	uint32_t end_addr;
	uint32_t addr_size;
	enum isp_rgltr_type power_type;
	uint32_t gate_shift;
	int (*func)(uint32_t addr, unsigned int to);
};

static struct xrisp_dma_list_t xrisp_dma_list[] = {
	{ISP_WDMA_FE_ROUTER_START, ISP_WDMA_FE_ROUTER_END, ISP_DATADMA_SIZE,
		ISP_REGULATOR_MAX, 17, &xrisp_fepe_datawdma_reset},
	{ISP_RDMA_FE_ROUTER_START, ISP_RDMA_FE_ROUTER_END, ISP_DATADMA_SIZE,
		ISP_REGULATOR_MAX, 17, &xrisp_datadma_reset},
	{ISP_RDMA_CVE_START, ISP_RDMA_CVE_END, ISP_DATADMA_SIZE,
		ISP_REGULATOR_MAX, 11, &xrisp_datadma_reset},
	{ISP_WDMA_CVE_START, ISP_WDMA_CVE_END, ISP_DATADMA_SIZE,
		ISP_REGULATOR_MAX, 11, &xrisp_datadma_reset},
	{ISP_WDMA_PE_START, ISP_WDMA_PE_END, ISP_DATADMA_SIZE,
		ISP_PE, 12, &xrisp_fepe_datawdma_reset},
	{ISP_RDMA_PE_START, ISP_RDMA_PE_END, ISP_DATADMA_SIZE,
		ISP_PE, 12, &xrisp_datadma_reset},
	{ISP_RDMA_BEF_START, ISP_RDMA_BEF_END, ISP_DATADMA_SIZE,
		ISP_BE, 13, &xrisp_datadma_reset},
	{ISP_WDMA_BEF_START, ISP_WDMA_BEF_END, ISP_DATADMA_SIZE,
		ISP_BE, 13, &xrisp_datadma_reset},
	{ISP_RDMA_BEB_START, ISP_RDMA_BEB_END, ISP_DATADMA_SIZE,
		ISP_BE, 13, &xrisp_datadma_reset},
	{ISP_WDMA_BEB_START, ISP_WDMA_BEB_END, ISP_DATADMA_SIZE,
		ISP_BE, 13, &xrisp_datadma_reset},
	{ISP_WDMA_FE0_START, ISP_WDMA_FE0_END, ISP_DATADMA_SIZE,
		ISP_FE_CORE0, 16, &xrisp_fepe_datawdma_reset},
	{ISP_WDMA_FE1_START, ISP_WDMA_FE1_END, ISP_DATADMA_SIZE,
		ISP_FE_CORE1, 15, &xrisp_fepe_datawdma_reset},
	{ISP_WDMA_FE2_START, ISP_WDMA_FE2_END, ISP_DATADMA_SIZE,
		ISP_FE_CORE2, 14, &xrisp_fepe_datawdma_reset},
	{ISP_CMDDMA_START, ISP_CMDDMA_END, ISP_CMDDMA_SIZE,
		ISP_REGULATOR_MAX, 0, &xrisp_cmddma_reset},
};

static int xrisp_dma_reset(void)
{
	uint32_t i, j;
	unsigned int to = 1;  // time out 1ms
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(xrisp_dma_list); i++) {
		for (j = xrisp_dma_list[i].start_addr; j <= xrisp_dma_list[i].end_addr;
			j += xrisp_dma_list[i].addr_size) {
			if ((xrisp_dma_list[i].power_type == ISP_REGULATOR_MAX
				|| xrisp_get_rgltr_open_cnt(xrisp_dma_list[i].power_type) > 0)
				&& xrisp_crg_get_gate_st(xrisp_dma_list[i].gate_shift))
				ret |= xrisp_dma_list[i].func(j, to);
		}
	}

	return ret;
}
#else

struct xrisp_dma_list {
	uint32_t module; // enum isp_dma_module
	enum isp_rgltr_type power_type;
	uint32_t gate_shift;
};

static struct xrisp_dma_list g_dma_list[] = {
	{FE_ROUTER_WDMA, ISP_REGULATOR_MAX, 17},
	{FE_ROUTER_RDMA, ISP_REGULATOR_MAX, 17},
	{CVE_RDMA, ISP_REGULATOR_MAX, 11},
	{CVE_WDMA, ISP_REGULATOR_MAX, 11},
	{PE_WDMA, ISP_PE, 12},
	{PE_RDMA, ISP_PE, 12},
	{BEF_RDMA, ISP_BE, 13},
	{BEF_WDMA, ISP_BE, 13},
	{BEB_RDMA, ISP_BE, 13},
	{BEB_WDMA, ISP_BE, 13},
	{FE_CORE0_WDMA, ISP_FE_CORE0, 16},
	{FE_CORE1_WDMA, ISP_FE_CORE1, 15},
	{FE_CORE2_WDMA, ISP_FE_CORE2, 14},
	{CMDDMA, ISP_REGULATOR_MAX, 0},
};

static int xrisp_dma_reset(void)
{
	uint32_t i;
	uint32_t flags = 0;
	struct arm_smccc_res res = {0xff};

	for (i = 0; i < ARRAY_SIZE(g_dma_list); i++) {
		if ((g_dma_list[i].power_type == ISP_REGULATOR_MAX
			|| xrisp_get_rgltr_open_cnt(g_dma_list[i].power_type) > 0)
			&& xrisp_crg_get_gate_st(g_dma_list[i].gate_shift))
			flags |= BIT(g_dma_list[i].module);
	}
	XRISP_PR_INFO("isp dma reset flags=0x%x", flags);

	arm_smccc_smc(FID_BL31_ISP_DMA_RESET, flags, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		XRISP_PR_ERROR("dma reset failed!");
		return -ENODEV;
	}

	return 0;
}
#endif

__maybe_unused
static struct cam_rproc_extra_mem xrisp_extra_mem[RPROC_MEM_REGION_MAX] = {
	[RPROC_MEM_REGION_VDEVBUFFER] = {
		.name = "vdev0buffer",
	},
	[RPROC_MEM_REGION_VDEVRING0] = {
		.name = "vdev0vring0",
	},
	[RPROC_MEM_REGION_VDEVRING1] = {
		.name = "vdev0vring1",
	},
	[RPROC_MEM_REGION_BOOTADDR] = {
		.name = "boot_buf",
	},
	[RPROC_MEM_REGION_FWBUF] = {
		.name = "fw_buf",
	},
	[RPROC_MEM_REGION_NPUOCM] = {
		.name = "npu_ocm",
		.is_iommu = true,
		.pa = XRISP_RPROC_NPUOCM_ADDR,
		.r82pa = XRISP_R82_NPUOCM_ADDR,
		.size = XRISP_RPROC_NPUOCM_SIZE
	},
	[RPROC_MEM_REGION_RAMLOG] = {
		.name = "ramlog",
	},
	[RPROC_MEM_REGION_DFXMEM] = {
		.name = "dfx_mem",
		.is_iommu = true,
		.pa = XRISP_RPROC_DFXMEM_ADDR,
		.r82pa = XRISP_R82_DFXMEM_ADDR,
		.size = XRISP_RPROC_DFXMEM_SIZE
	},
};

int xrisp_rproc_state_get(void)
{
	if (g_xrproc == NULL)
		return -EINVAL;
	XRISP_PR_INFO("isp state=%s", RPROC_STATUS(g_xrproc->rproc->state));
	return g_xrproc->rproc->state;
}

bool xrisp_rproc_is_running(void)
{
	if (!g_xrproc)
		return false;

	if (atomic_read(&g_xrproc->rproc->power) > 0)
		return true;

	return false;
}

static inline bool check_sec_scene(uint32_t scene)
{
	return ((scene == XRISP_SAFE_FACE_BOOT) || (scene == XRISP_SAFE_AON_BOOT));
}

static bool check_isp_attach_boot(uint32_t scene)
{
	return check_sec_scene(scene);
}

static bool check_isp_norm_boot(uint32_t scene)
{
	return ((scene == XRISP_NORMAL_BOOT) ||
		 (scene == XRISP_SHAKE_BOOT) ||
		(scene == XRISP_NSAFE_FACE_BOOT));
}

bool xrisp_rproc_is_sec_running(void)
{
	if (!g_xrproc)
		return false;
	return !!(atomic_read(&g_xrproc->rproc->power) && check_sec_scene(g_xrproc->scene));
}

__maybe_unused static int xrisp_rproc_into_wfi(void)
{
	uint32_t wfi_msg = XRISP_RPROC_SET_WFI;
	int ret = 0;

	ret = xrisp_rproc_mbox_send_sync(&wfi_msg, sizeof(wfi_msg));
	if (ret) {
		XRISP_PR_ERROR("failed to set isp wfi");
		return -EINVAL;
	}
	mdelay(1);
	return 0;
}

static int xrisp_smmu_power_on(struct cam_rproc *xrisp_rproc)
{
	int ret = 0;

	ret = xring_smmu_tcu_ctrl(xrisp_rproc->dev, XRING_SMMU_TCU_POWER_ON);
	if (ret) {
		XRISP_PR_ERROR("power on tcu fail:%d\n", ret);
		return ret;
	}

	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu0",
				  XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		XRISP_PR_ERROR("power on tbu0 fail");
		goto smmu_tcu_power_off;
	}
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu1",
				  XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		XRISP_PR_ERROR("power on tbu1 fail");
		goto smmu_tbu0_power_off;
	}
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu2",
				  XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		XRISP_PR_ERROR("power on tbu2 fail");
		goto smmu_tbu1_power_off;
	}
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu3",
				  XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		XRISP_PR_ERROR("power on tbu3 fail");
		goto smmu_tbu2_power_off;
	}

	//secboot not need to set sid
	if (check_sec_scene(xrisp_rproc->scene))
		return 0;

	/*
	 * Set sid to media2_smmu_ctrl in order to recovery in power outage
	 * sid0:tbu_rt sid1:tbu_nm0&tbu_nm1 sid2:tbu_ll
	 **/
	ret = xring_smmu_sid_set(xrisp_rproc->dev, "media2_smmu_sid0",
				 0x3, 0x5, 0x1);
	if (ret) {
		XRISP_PR_ERROR("set sid0 fail");
		goto smmu_tbu3_power_off;
	}
	ret = xring_smmu_sid_set(xrisp_rproc->dev, "media2_smmu_sid1",
				 0x3, 0x6, 0x1);
	if (ret) {
		XRISP_PR_ERROR("set sid1 fail");
		goto smmu_tbu3_power_off;
	}
	ret = xring_smmu_sid_set(xrisp_rproc->dev, "media2_smmu_sid2",
				 0x3, 0x7, 0x1);
	if (ret) {
		XRISP_PR_ERROR("set sid2 fail");
		goto smmu_tbu3_power_off;
	}
	return 0;

smmu_tbu3_power_off:
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu3",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu3 fail");
smmu_tbu2_power_off:
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu2",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu2 fail");
smmu_tbu1_power_off:
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu1",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu1 fail");
smmu_tbu0_power_off:
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu0",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu0 fail");
smmu_tcu_power_off:
	ret = xring_smmu_tcu_ctrl(xrisp_rproc->dev, XRING_SMMU_TCU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tcu fail.");

	return -EINVAL;
}

static void xrisp_smmu_power_off(struct cam_rproc *xrisp_rproc)
{
	int ret = 0;

	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu0",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu0 fail");
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu1",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu1 fail");
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu2",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu2 fail");
	ret = xring_smmu_tbu_ctrl(xrisp_rproc->dev, "media2_smmu_tbu3",
				  XRING_SMMU_TBU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tbu3 fail");

	ret = xring_smmu_tcu_ctrl(xrisp_rproc->dev, XRING_SMMU_TCU_POWER_OFF);
	if (ret)
		XRISP_PR_ERROR("power off tcu fail: %d\n", ret);
}

__maybe_unused static int media2_reg_setting(uint32_t addr, size_t size,
					     struct m2_register *m2_reg, uint32_t num)
{
	int ret = 0;
	int i = 0;
	void *va = NULL;
	uint32_t val;

	va = ioremap(addr, size);
	if (!va) {
		XRISP_PR_ERROR("ioremap 0x%x fail", addr);
		ret = -ENOMEM;
		goto exit;
	}

	for (i = 0; i < num; i++) {
		if (m2_reg[i].ops == M2_WRITE || m2_reg[i].ops == M2_READ_AFTER_WRITE) {
			writel(m2_reg[i].val, va + m2_reg[i].offset);
			XRISP_PR_INFO("%s write 0x%x 0x%x", m2_reg[i].name,
				addr + m2_reg[i].offset, m2_reg[i].val);
		}
		XRISP_PR_INFO("%s delay %d ms", m2_reg[i].name, m2_reg[i].msdelay);
		mdelay(m2_reg[i].msdelay);
		if (m2_reg[i].ops == M2_READ || m2_reg[i].ops == M2_READ_AFTER_WRITE) {
			val = readl(va + m2_reg[i].offset);
			XRISP_PR_INFO("%s read 0x%x 0x%x", m2_reg[i].name,
				addr + m2_reg[i].offset, val);
		}
	}

	iounmap(va);
	return 0;
exit:
	return ret;
}

int xrisp_rproc_handle_bdinfo(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct xrisp_rproc_rsc_table *rsc_ptr;
	enum xrisp_mcu_clk_rate_e mcu_rate_id;

	if (!xrisp_rproc)
		return -EINVAL;

	rsc_ptr = (struct xrisp_rproc_rsc_table *)rproc->table_ptr;
	mcu_rate_id = xrisp_rproc->scene_rate_info_list[xrisp_rproc->scene].mcu_rate_id;

	rsc_ptr->bd_info.mcu_clock = xrisp_rproc->bdinfo_list[mcu_rate_id].mcu_clock;
	rsc_ptr->bd_info.uart_clock = xrisp_rproc->bdinfo_list[mcu_rate_id].uart_clock;
	rsc_ptr->bd_info.timer_clock = xrisp_rproc->bdinfo_list[mcu_rate_id].timer_clock;

	//XRISP_PR_DEBUG("mcu clk=%d, uart clk=%d, timer clk=%d", rsc_ptr->bd_info.mcu_clock,
	//rsc_ptr->bd_info.uart_clock, rsc_ptr->bd_info.timer_clock);
	return RSC_HANDLED;
}

static ktime_t xrisp_get_time_offset(void)
{
	ktime_t timestamp, ap_syscount;

	timestamp = xr_timestamp_gettime();
	ap_syscount = ktime_get_boottime();
	return ktime_sub(timestamp, ap_syscount);
}

static void xrisp_rproc_time_sync_work(struct work_struct *work)
{
	struct time_sync_msg sync_msg;
	int ret = 0;

	sync_msg.msg = XRISP_RPROC_TIME_SYNC;
	sync_msg.time_offset = xrisp_get_time_offset();
	ret = xrisp_rproc_mbox_send_sync((unsigned int *)&sync_msg, sizeof(sync_msg));
	if (ret) {
		XRISP_PR_ERROR("time sync failed");
		return;
	}

	XRISP_PR_INFO("sync time offset=%llu", sync_msg.time_offset);
}

static void xrisp_time_sync_handler(struct timer_list *timer)
{
	queue_work(system_wq, &g_xrproc->time_sync_work);
	mod_timer(&g_xrproc->time_sync_timer, jiffies + msecs_to_jiffies(XRISP_TIME_SYNC_INTERVAL));
}

static void xrisp_time_sync_init(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct xrisp_rproc_rsc_table *rsc_ptr;

	if (!xrisp_rproc)
		return;

	rsc_ptr = (struct xrisp_rproc_rsc_table *)rproc->table_ptr;
	rsc_ptr->bd_info.time_offset = xrisp_get_time_offset();

	timer_setup(&xrisp_rproc->time_sync_timer, xrisp_time_sync_handler, 0);
	mod_timer(&xrisp_rproc->time_sync_timer,
		  jiffies + msecs_to_jiffies(XRISP_TIME_SYNC_INTERVAL));

	XRISP_PR_INFO("sync time offset=%llu", rsc_ptr->bd_info.time_offset);
}

static void xrisp_time_sync_delete(struct cam_rproc *xrisp_rproc)
{
	if (!xrisp_rproc)
		return;
	cancel_work_sync(&xrisp_rproc->time_sync_work);
	del_timer_sync(&xrisp_rproc->time_sync_timer);
}

void xrisp_wakeup_lock_acquire(struct wakeup_source *wakelock)
{
	if (wakelock == NULL)
		return;

	__pm_stay_awake(wakelock);
	XRISP_PR_INFO("wakeup lock acquire success");
}

void xrisp_wakeup_lock_release(struct wakeup_source *wakelock)
{
	if (wakelock == NULL)
		return;

	if (wakelock->active) {
		__pm_relax(wakelock);
		XRISP_PR_INFO("wakeup lock release success");
	} else
		XRISP_PR_INFO("wakeup lock is not taken");

}

#ifdef PR_TABLE
void pr_rsc_table(struct cam_rproc *xrisp_rproc)
{
	XRISP_PR_ERROR("table_ptr 0x%pK cached_table 0x%pK clean_table 0x%pK",
		xrisp_rproc->rproc->table_ptr, xrisp_rproc->rproc->cached_table,
		xrisp_rproc->rproc->clean_table);

	if (xrisp_rproc->rproc->table_ptr)
		print_hex_dump_debug("rproc", DUMP_PREFIX_OFFSET, 16, 1,
			xrisp_rproc->rproc->table_ptr, 0xd0, true);
}
#else
void pr_rsc_table(struct cam_rproc *xrisp_rproc)
{

}
#endif

int xrisp_power_on(struct cam_rproc *xrisp_rproc)
{
	struct scene_rate_info *scene_rate;
	int status;

	if (check_sec_scene(xrisp_rproc->scene)) {
		if (xrisp_ca_context_build()) {
			XRISP_PR_ERROR("isp ca context build failed");
			return -EINVAL;
		}

		if (xrisp_ca_switch_mode(TA_SECMODE_ISP, TA_SECMODE_SAFE)) {
			XRISP_PR_ERROR("isp switch safe_mode failed");
			xrisp_ca_context_unbuild();
			return -EINVAL;
		}
	}

	xrisp_wakeup_lock_acquire(xrisp_rproc->xrisp_wakelock);

	if (regulator_enable(xrisp_rproc->media2_rgltr)) {
		XRISP_PR_ERROR("media2 regulator enable failed");
		goto power_on_fail;
	}

	if (regulator_enable(xrisp_rproc->mcu_rgltr)) {
		XRISP_PR_ERROR("mcu regulator enable failed");
		goto media2_rgltr_release;
	}

	if (xrisp_boot_pipe_rgltr_enable()) {
		XRISP_PR_ERROR("pipe rgltr enable failed");
		goto mcu_rgltr_release;
	}

	scene_rate = &xrisp_rproc->scene_rate_info_list[xrisp_rproc->scene];
	if (xrisp_clk_api_enable_init_rate(XRISP_CRG_CLK, scene_rate->crg_mask,
					   scene_rate->crg_rate_id)) {
		XRISP_PR_ERROR("isp crg clk init failed");
		goto pipe_rgltr_release;
	}

	if (xrisp_clk_api_enable_init_rate(XRISP_MCU_CLK, scene_rate->mcu_mask,
					   scene_rate->mcu_rate_id)) {
		XRISP_PR_ERROR("isp mcu clk init failed");
		goto isp_crg_clk_deinit;
	}

	status = xrisp_mcu_dma_status_check();
	if (status != (ISP_MCU_DMA_CLK_FLAG | ISP_MCU_DMA_RST_FLAG)) {
		XRISP_PR_ERROR("isp dma status:0x%x error", status);
		goto isp_mcu_clk_deinit;
	}

	if (xrisp_smmu_power_on(xrisp_rproc)) {
		XRISP_PR_ERROR("isp smmu power on failed");
		goto isp_mcu_clk_deinit;
	}

	if (check_sec_scene(xrisp_rproc->scene)) {
		if (xrisp_ca_attach(true)) {
			XRISP_PR_ERROR("isp ca smmu_attach failed");
			goto isp_smmu_power_off;
		}
	}

	if (g_provider_pid)
		provider_task = find_task_by_vpid(g_provider_pid);

	current_task = current;
	XRISP_PR_DEBUG("power on finish");
	return 0;

isp_smmu_power_off:
	xrisp_smmu_power_off(xrisp_rproc);

isp_mcu_clk_deinit:
	xrisp_clk_api_deinit_clk(XRISP_MCU_CLK);
	XRISP_PR_ERROR("isp mcu clk deinit");
isp_crg_clk_deinit:
	xrisp_clk_api_deinit_clk(XRISP_CRG_CLK);
	XRISP_PR_ERROR("isp crg clk deinit");
pipe_rgltr_release:
	xrisp_pipe_rgltr_release();
	XRISP_PR_ERROR("isp pipe rgltr release");
mcu_rgltr_release:
	regulator_disable(xrisp_rproc->mcu_rgltr);
	XRISP_PR_ERROR("mcu rgltr release");
media2_rgltr_release:
	regulator_disable(xrisp_rproc->media2_rgltr);
	XRISP_PR_ERROR("media2 rgltr release");
power_on_fail:
	xrisp_wakeup_lock_release(xrisp_rproc->xrisp_wakelock);
	if (check_sec_scene(xrisp_rproc->scene)) {
		xrisp_ca_switch_mode(TA_SECMODE_ISP, TA_SECMODE_UNSAFE);
		xrisp_ca_context_unbuild();
	}
	XRISP_PR_ERROR("isp power on failed");
	return -EINVAL;
}

void xrisp_power_off(struct cam_rproc *xrisp_rproc)
{
	unsigned long rsstotal;
	struct task_struct *task;

	if (check_sec_scene(xrisp_rproc->scene))
		xrisp_ca_attach(false);
	xrisp_smmu_power_off(xrisp_rproc);
	xrisp_be_ocm_unlink_reap();
	xrisp_pipe_rgltr_release();
	regulator_disable(xrisp_rproc->mcu_rgltr);
	xrisp_clk_api_deinit_clk(XRISP_MCU_CLK);
	xrisp_clk_api_deinit_clk(XRISP_CRG_CLK);
	regulator_disable(xrisp_rproc->media2_rgltr);

	xrisp_wakeup_lock_release(xrisp_rproc->xrisp_wakelock);

	if (check_sec_scene(xrisp_rproc->scene)) {
		xrisp_ca_switch_mode(TA_SECMODE_ISP, TA_SECMODE_UNSAFE);
		xrisp_ca_context_unbuild();
	}

	if (g_provider_pid)
		task = find_task_by_vpid(g_provider_pid);
	else
		task = current;

	if (task && (task->mm)) {
		rsstotal = get_mm_rss(task->mm);
		XRISP_PR_INFO("[%s][pid:%d]xrisp provider: total_vm:%ld, rss:%ld[power_off], encreased:%ld",
			 task->comm, task->pid, task->mm->total_vm,
			 rsstotal, rsstotal-g_poweroff_rss_count);
		g_poweroff_rss_count = rsstotal;
	}
}

static int xrisp_rproc_stop(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;

	XRISP_PR_INFO("rproc stop");
	xrisp_time_sync_delete(xrisp_rproc);
	xrisp_rproc_into_wfi();

	xrisp_trace_debug_close();
	xrisp_mcu_dma_reset();
	xrisp_r82_reset();

	if (xrisp_rproc->scene != XRISP_SHAKE_BOOT)
		xrisp_dma_reset();

	xrisp_ispmodule_reset(rproc);

	pr_rsc_table(rproc->priv);
	return 0;
}

static int xrisp_rproc_start(struct rproc *rproc)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;

	XRISP_PR_INFO("rproc start");

	xrisp_smmu_bypass_disable();

	xrisp_set_rv_baraddr(rproc->bootaddr);

	// Set fw ramlog cfg to last config
	if (ramlog_set_default_cfg(xrisp_rproc->ramlog.logmem->va))
		XRISP_PR_ERROR("set ramlog default cfg failed");

	xrisp_r82_dereset();
	xrisp_trace_debug_open();

	xrisp_rproc->skip_first_kick = false;
	reinit_completion(&xrisp_rproc->boot_complete);
	reinit_completion(&xrisp_rproc->rpmsg_complete);
	xrisp_rproc_handle_bdinfo(rproc);
	xrisp_time_sync_init(rproc);
	xrisp_halt(0);

	if (xrisp_rproc->skip_first_kick)
		return ret;

	XRISP_PR_INFO("waitting boot completion");

	ret = wait_for_completion_timeout(&xrisp_rproc->boot_complete,
					  msecs_to_jiffies(boot_timeout_ms));
	if (ret) {
		XRISP_PR_INFO("boot completion wait %llu ms",
			      boot_timeout_ms - jiffies_to_msecs(ret));
	} else {
		XRISP_PR_ERROR("boot timeout");
		ret = -ETIMEDOUT;
		boot_timeout_ms = 1000;
		goto rproc_timeout_err;
	}

	pr_rsc_table(rproc->priv);
	return 0;

rproc_timeout_err:
	xrisp_halt(1);
	xrisp_time_sync_delete(rproc->priv);
	xrisp_trace_debug_close();
	xrisp_r82_reset();

	return ret;
}

static int xrisp_rproc_rmem_alloc(struct rproc *rproc,
			       struct rproc_mem_entry *mem)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct cam_rproc_mem *cam_mem;
	int port;
	void *va = NULL;

	cam_mem = mem->priv;

	if (cam_mem->da.region < RPROC_MEM_REGION_FWBUF ||
	    cam_mem->da.region == RPROC_MEM_REGION_RAMLOG) {

		va = ioremap_wc(cam_mem->pa, cam_mem->da.size);
		if (IS_ERR_OR_NULL(va)) {
			XRISP_PR_ERROR("unable to ioremap memory region: 0x%pK+0x%zx",
				       (void *)cam_mem->pa, cam_mem->da.size);
			return -ENOMEM;
		}
		XRISP_PR_INFO("memset %s", cam_mem->name);
		memset(va, 0, cam_mem->da.size);
	}

	port = IOMMU_READ | IOMMU_WRITE;

	if (cam_mem->da.region != RPROC_MEM_REGION_BOOTADDR)
		port |= IOMMU_NOEXEC;

	if (cam_mem->is_iommu) {
		//if (cam_mem->da.region == RPROC_MEM_REGION_FWBUF)
		//	cam_mem->r82pa = cam_mem->r82pa | (1UL << 38);

		if (check_sec_scene(xrisp_rproc->scene))
			ret = xrisp_ns_cmabuf_ssmmu_map(cam_mem->pa, cam_mem->da.size,
						     &cam_mem->da.base);
		else {
			ret = iommu_map(xrisp_rproc->domain, cam_mem->da.base, cam_mem->r82pa,
					cam_mem->da.size, port, GFP_KERNEL);
		}

		if (ret < 0) {
			XRISP_PR_ERROR("iommu map fail r82pa 0x%pK+0x%zx", (void *)cam_mem->r82pa,
				       cam_mem->da.size);
			if (va)
				iounmap(va);
			return -ENOMEM;
		}

		XRISP_PR_INFO("iommu map: port 0x%x r82pa 0x%pK+0x%zx to da 0x%llx+0x%zx", port,
			      (void *)cam_mem->r82pa, cam_mem->da.size, cam_mem->da.base,
			      cam_mem->da.size);
	}


	cam_mem->va = va;
	cam_mem->is_iomem = false;
	/* Update memory entry */
	mem->is_iomem =  false;
	mem->va = va;
	mem->da = cam_mem->da.base;
	mem->dma = cam_mem->da.base;
	//smmu bypass use  da == pa
	// mem->da = cam_mem->pa;
	// cam_mem->da.base = cam_mem->pa;

	mem->len = cam_mem->da.size;
	cam_mem->avail = true;

	XRISP_PR_INFO("rproc mem %s: pa 0x%pK+0x%zx va 0x%pK+0x%zx da 0x%x+0x%zx", mem->name,
		      (void *)cam_mem->pa, mem->len, mem->va, mem->len, mem->da, mem->len);

	rproc_mem_debugfs_install(xrisp_rproc, cam_mem->da.region);

	return 0;
}

static int xrisp_rproc_rmem_release(struct rproc *rproc,
				 struct rproc_mem_entry *mem)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct cam_rproc_mem *cam_mem;

	cam_mem = mem->priv;

	if (!cam_mem->avail)
		return -EINVAL;
	cam_mem->avail = false;

	XRISP_PR_DEBUG("unmap memory %s ", cam_mem->name);

	rproc_mem_debugfs_uninstall(xrisp_rproc, cam_mem->da.region);
	if (mem->va)
		iounmap(mem->va);
	if (cam_mem->is_iommu) {
		if (check_sec_scene(xrisp_rproc->scene))
			xrisp_ns_cmabuf_ssmmu_unmap(cam_mem->pa, cam_mem->da.size,
						 &cam_mem->da.base);
		else
			iommu_unmap(xrisp_rproc->domain, cam_mem->da.base,
				    cam_mem->da.size);
	}

	cam_mem->va = NULL;
	cam_mem->pa = 0;
	cam_mem->avail = false;

	mem->va = NULL;
	mem->da = 0;
	mem->dma = 0;
	mem->len = 0;
	return 0;
}

static int xrisp_rproc_buf_release(struct rproc *rproc,
				   struct rproc_mem_entry *mem)
{
	struct cam_rproc_mem *cam_mem;
	struct cam_rproc *xrisp_rproc = rproc->priv;

	XRISP_PR_DEBUG("rproc buf release %s", mem->name);

	cam_mem = mem->priv;

	if (!cam_mem->avail)
		return -EINVAL;

	if (WARN_ON(!cam_mem->dma_attach || !cam_mem->buf ||
	    !cam_mem->sgtlb))
		return -EINVAL;

	rproc_mem_debugfs_uninstall(xrisp_rproc, cam_mem->da.region);

	dma_buf_vunmap(cam_mem->buf, &cam_mem->kmap);

	if (check_sec_scene(xrisp_rproc->scene)) {
		if (xrisp_ns_dmabuf_ssmmu_unmap(cam_mem->sgtlb, &cam_mem->da.base,
						TA_SMMU_MMAP_TYPE_FIX_IOVA))
			XRISP_PR_ERROR("dmabuf safe_smmu unmap failed");
	} else
		iommu_unmap(xrisp_rproc->domain, cam_mem->da.base, cam_mem->da.size);

	dma_buf_unmap_attachment(cam_mem->dma_attach,
				 cam_mem->sgtlb,
				 cam_mem->dma_dir);
	dma_buf_detach(cam_mem->buf, cam_mem->dma_attach);
	xr_dmabuf_kernel_unaccount(cam_mem->buf, XR_DMABUF_KERNEL_ISP_RPROC);
	dma_heap_buffer_free(cam_mem->buf);

	cam_mem->buf = NULL;
	cam_mem->dma_attach = NULL;
	cam_mem->sgtlb = NULL;
	cam_mem->va = NULL;
	cam_mem->pa = 0;
	cam_mem->avail = false;

	mem->is_iomem = false;
	mem->va = NULL;
	mem->da = 0;
	mem->len = 0;

	return 0;
}

static int xrisp_rproc_buf_alloc(struct rproc *rproc,
				 struct rproc_mem_entry *mem)
{
	int ret = 0;
	struct dma_heap *system_heap;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct cam_rproc_mem *cam_mem;
	size_t iommu_size = 0;
	int port;
	KTIME_DEBUG_DEFINE_START();

	XRISP_PR_DEBUG("%s buf alloc start\n", mem->name);
	cam_mem = mem->priv;
	system_heap = dma_heap_find("system-uncached");
	if (IS_ERR_OR_NULL(system_heap)) {
		ret = PTR_ERR(system_heap);
		XRISP_PR_ERROR("system head not find, ret = %d(%pe)",
			      ret, system_heap);
		return -EINVAL;
	}
	cam_mem->buf = dma_heap_buffer_alloc(system_heap,
			cam_mem->da.size, O_RDWR, 0);
	if (IS_ERR(cam_mem->buf)) {
		XRISP_PR_ERROR("dma heap alloc buf fail, ret = %ld(%pe)",
			PTR_ERR(cam_mem->buf), cam_mem->buf);
		ret = -ENOMEM;
		goto out;
	}
	xr_dmabuf_kernel_account(cam_mem->buf, XR_DMABUF_KERNEL_ISP_RPROC);

	cam_mem->dma_attach = dma_buf_attach(cam_mem->buf, xrisp_rproc->dev);
	if (IS_ERR_OR_NULL(cam_mem->dma_attach)) {
		ret = PTR_ERR(cam_mem->dma_attach);
		XRISP_PR_ERROR("dma buf attach failed %d", ret);
		goto dam_buf_free;
	}
	cam_mem->dma_dir = DMA_BIDIRECTIONAL;
	cam_mem->sgtlb = dma_buf_map_attachment(cam_mem->dma_attach,
						cam_mem->dma_dir);
	if (IS_ERR_OR_NULL(cam_mem->sgtlb)) {
		ret = PTR_ERR(cam_mem->sgtlb);
		XRISP_PR_ERROR("dma buf attachment map failed %d", ret);
		goto dma_detach;
	}

	//iova map
	if (check_sec_scene(xrisp_rproc->scene)) {
		ret = xrisp_ns_dmabuf_ssmmu_map(cam_mem->sgtlb, &cam_mem->da.base,
						TA_SMMU_MMAP_TYPE_FIX_IOVA);
		if (ret) {
			XRISP_PR_ERROR("dmabuf safe_smmu map failed");
			goto unmap_attachment;
		}
	} else {
		port = IOMMU_READ | IOMMU_WRITE;

		if (cam_mem->da.region != RPROC_MEM_REGION_BOOTADDR)
			port |= IOMMU_NOEXEC;

		iommu_size = iommu_map_sgtable(xrisp_rproc->domain, cam_mem->da.base,
					       cam_mem->sgtlb, port);
		if (iommu_size <= 0) {
			XRISP_PR_ERROR("iommu map fail ret = %zu", iommu_size);
			ret = -ENOMEM;
			goto unmap_attachment;
		}
		cam_mem->da.size = iommu_size;
	}

	ret = dma_buf_vmap(cam_mem->buf, &cam_mem->kmap);
	if (ret) {
		XRISP_PR_ERROR("dma buf vmap fail ret = %d", ret);
		goto dam_buf_free;
	}
	// XRISP_PR_DEBUG("vmap vaddr 0x%pK, vaddr iomem 0x%pK, iomem %s.",
	//	cam_mem->kmap.vaddr,
	//	cam_mem->kmap.vaddr_iomem,
	//	cam_mem->kmap.is_iomem ? "Y" : "N");

	dma_heap_put(system_heap);

	cam_mem->va = cam_mem->kmap.vaddr;
	cam_mem->is_iomem = cam_mem->kmap.is_iomem;
	cam_mem->avail = true;
	/* Update memory entry */
	mem->va = cam_mem->va;
	mem->is_iomem = cam_mem->is_iomem;

	if (cam_mem->da.region == RPROC_MEM_REGION_BOOTADDR) {
		if (cam_mem->is_iomem)
			memset_io(cam_mem->va, 0, cam_mem->da.size);
		else
			memset(cam_mem->va, 0, cam_mem->da.size);
	}

	rproc_mem_debugfs_install(xrisp_rproc, cam_mem->da.region);

	XRISP_PR_INFO("%s iommu map pa 0x%pK+0x%zx  to da 0x%llx+0x%zx  va 0x%pK  iomem %s.",
		      mem->name, (void *)cam_mem->pa, cam_mem->da.size, cam_mem->da.base,
		      cam_mem->da.size, cam_mem->va, mem->is_iomem ? "Y" : "N");
	KTIME_DEBUG_TIMEOUT_CHECK(ISP_RPROC_ALLOC_TIME_MAX);
	return 0;

unmap_attachment:
	dma_buf_unmap_attachment(cam_mem->dma_attach, cam_mem->sgtlb,
				 cam_mem->dma_dir);
dma_detach:
	dma_buf_detach(cam_mem->buf, cam_mem->dma_attach);
dam_buf_free:
	xr_dmabuf_kernel_unaccount(cam_mem->buf, XR_DMABUF_KERNEL_ISP_RPROC);
	dma_heap_buffer_free(cam_mem->buf);
out:
	dma_heap_put(system_heap);
	return ret;
}

__maybe_unused static struct cam_rproc_reserve_cma *
xrisp_rproc_reserve_cma_get(struct cam_rproc *xrisp_rproc, uint32_t iova_region)
{
	if (!xrisp_rproc->reserve_cma)
		return NULL;

	for (int i = 0; i < xrisp_rproc->cma_region_num; i++) {
		if (xrisp_rproc->reserve_cma[i].region == iova_region)
			return &xrisp_rproc->reserve_cma[i];
	}

	return NULL;
}

static int xrisp_rproc_cma_release(struct rproc *rproc, struct rproc_mem_entry *mem)
{
	struct cam_rproc_mem *cam_mem;
	struct cam_rproc *xrisp_rproc = rproc->priv;

	XRISP_PR_DEBUG("rproc cma release %s", mem->name);

	cam_mem = mem->priv;

	if (!cam_mem->avail)
		return -EINVAL;

	rproc_mem_debugfs_uninstall(xrisp_rproc, cam_mem->da.region);

	if (check_sec_scene(xrisp_rproc->scene))
		xrisp_ns_cmabuf_ssmmu_unmap(cam_mem->pa, cam_mem->da.size,
					 &cam_mem->da.base);
	else
		iommu_unmap(xrisp_rproc->domain, cam_mem->da.base, cam_mem->da.size);

#ifndef CONFIG_XRISP_RPROC_RESERVE_CMA
	dma_free_attrs(xrisp_rproc->dev, cam_mem->da.size, cam_mem->va, cam_mem->dma_handle,
		       DMA_ATTR_FORCE_CONTIGUOUS);
#endif

	cam_mem->va = NULL;
	cam_mem->pa = 0;
	cam_mem->avail = false;

	mem->is_iomem = false;
	mem->va = NULL;
	mem->da = 0;
	mem->len = 0;

	return 0;
}

static int xrisp_rproc_cma_alloc(struct rproc *rproc, struct rproc_mem_entry *mem)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct cam_rproc_mem *cam_mem;
	int port;
	__maybe_unused struct cam_rproc_reserve_cma *rcma;
	KTIME_DEBUG_DEFINE_START();

	XRISP_PR_DEBUG("%s cma alloc start\n", mem->name);
	cam_mem = mem->priv;

#ifdef CONFIG_XRISP_RPROC_RESERVE_CMA
	rcma = xrisp_rproc_reserve_cma_get(xrisp_rproc, cam_mem->da.region);
	if (!rcma) {
		XRISP_PR_ERROR("reserve cma region %d not found", cam_mem->da.region);
		return -ENOMEM;
	}

	if (rcma->size != cam_mem->da.size) {
		XRISP_PR_ERROR("reserve cma region %d size not match %zu != %zu",
			       cam_mem->da.region, rcma->size, cam_mem->da.size);
		return -EINVAL;
	}

	cam_mem->va = rcma->va;
	cam_mem->dma_handle = rcma->dma_handle;
#else
	cam_mem->va = dma_alloc_attrs(xrisp_rproc->dev, cam_mem->da.size, &cam_mem->dma_handle,
				      GFP_KERNEL, DMA_ATTR_FORCE_CONTIGUOUS);
#endif

	if (!cam_mem->va) {
		XRISP_PR_ERROR("dma alloc coherent fail ret = %ld", PTR_ERR(cam_mem->va));
		return -ENOMEM;
	}

	cam_mem->pa = cam_mem->dma_handle;

	port = IOMMU_READ | IOMMU_WRITE;

	if (cam_mem->da.region != RPROC_MEM_REGION_BOOTADDR)
		port |= IOMMU_NOEXEC;

	if (check_sec_scene(xrisp_rproc->scene))
		ret = xrisp_ns_cmabuf_ssmmu_map(cam_mem->pa, cam_mem->da.size,
					     &cam_mem->da.base);
	else {
		ret = iommu_map(xrisp_rproc->domain, cam_mem->da.base, cam_mem->pa,
				cam_mem->da.size, port, GFP_KERNEL);
	}

	if (ret < 0) {
		XRISP_PR_ERROR("iommu map fail ret = %d", ret);
		ret = -ENOMEM;
		goto dma_free;
	}

	cam_mem->is_iomem = false;
	cam_mem->avail = true;
	/* Update memory entry */
	mem->va = cam_mem->va;
	mem->is_iomem = cam_mem->is_iomem;

	rproc_mem_debugfs_install(xrisp_rproc, cam_mem->da.region);

	XRISP_PR_INFO("%s: iommu map pa 0x%pK+%zx  to da 0x%llx+%zx  va 0x%pK", mem->name,
		      (void *)cam_mem->pa, cam_mem->da.size, cam_mem->da.base, cam_mem->da.size,
		      cam_mem->va);
	KTIME_DEBUG_TIMEOUT_CHECK(ISP_RPROC_ALLOC_TIME_MAX);
	return 0;

dma_free:
#ifndef CONFIG_XRISP_RPROC_RESERVE_CMA
	dma_free_attrs(xrisp_rproc->dev, cam_mem->da.size, cam_mem->va, cam_mem->dma_handle,
		       DMA_ATTR_FORCE_CONTIGUOUS);
#endif
	return ret;
}

static int xrisp_of_parse_device_addres(struct device_node *np, struct cam_device_address *dev_addr)
{
	int ret = 0;
	int len;
	const __be32 *cell;

	cell = of_get_property(np, "reg", &len);
	if (!cell) {
		XRISP_PR_ERROR("%pOF: missing reg property", np);
		return -EINVAL;
	}
	if (len / 4 != of_n_addr_cells(np) + of_n_size_cells(np)) {
		XRISP_PR_ERROR("%pOF: missing reg cells (%d)", np, len);
		return -EINVAL;
	}

	dev_addr->base = of_read_number(cell, of_n_addr_cells(np));
	cell += of_n_addr_cells(np);
	dev_addr->size = of_read_number(cell, of_n_size_cells(np));

	ret = of_property_read_u32(np, "iova-region", &dev_addr->region);
	if (ret || dev_addr->region < RPROC_MEM_REGION_START ||
		dev_addr->region >= RPROC_MEM_REGION_MAX) {
		XRISP_PR_ERROR("%pOF: has no or invalid iova-region %d", np,
			dev_addr->region);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "mem-type", &dev_addr->type);
	if (ret || dev_addr->type < 0 || dev_addr->type > RPROC_MEM_TYPE_MAX - 1) {
		XRISP_PR_ERROR("%pOF: has no or invalid mem type %d", np, dev_addr->type);
		return -EINVAL;
	}

	XRISP_PR_DEBUG("%s devices address base 0x%llx , size 0x%zx, region %d, type %d",
		 np->name, dev_addr->base, dev_addr->size, dev_addr->region, dev_addr->type);

	return 0;
}

static int xrisp_provider_buf_ssmmu_map_thread(void *data)
{
	g_xrproc->ssmmu_map_result = xrisp_provider_buf_ssmmu_map_all(&g_xrproc->ssmmu_th_stop);
	complete(&g_xrproc->ssmmu_map_th_exit);
	return 0;
}

static int xrisp_provider_buf_ssmmu_map_thread_stop(struct task_struct *thread, int timeout)
{
	if (!thread || thread->__state == TASK_DEAD) {
		XRISP_PR_WARN("thread is not running");
		return -EINVAL;
	}

	atomic_set(&g_xrproc->ssmmu_th_stop, 1);

	if (timeout) {
		unsigned long wait;

		wait = wait_for_completion_timeout(&g_xrproc->ssmmu_map_th_exit,
						   msecs_to_jiffies(timeout));
		if (!wait) {
			XRISP_PR_WARN("wait thread stop timeout");
			return -ETIMEDOUT;
		}
	} else
		wait_for_completion_interruptible(&g_xrproc->ssmmu_map_th_exit);

	XRISP_PR_INFO("thread stop success");
	return 0;
}

static int xrisp_rproc_prepare(struct rproc *rproc)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct device_node *np = xrisp_rproc->dev->of_node;
	struct of_phandle_iterator it;
	struct cam_device_address dev_addr;
	struct rproc_mem_entry *mem = NULL;
	struct reserved_mem *rmem = NULL;
	struct cam_rproc_mem *cam_mem = NULL;
	struct device_node *rmemnp = NULL;
	phys_addr_t mem_phys = 0;
	size_t mem_size = 0;
	u32 da = 0;

	XRISP_PR_DEBUG("rproc prepare\n");
	xrisp_rproc->nb_vdev = 0;
	xrisp_rproc->max_notifyid = -1;
	xrisp_rproc->rsc_table_size = sizeof(rproc_rsc_table);
	memcpy(&xrisp_rproc->rsc_table, &rproc_rsc_table, sizeof(rproc_rsc_table));

	ret = xrisp_power_on(xrisp_rproc);
	if (ret)
		return ret;

	if (check_sec_scene(xrisp_rproc->scene)) {
		reinit_completion(&xrisp_rproc->ssmmu_map_th_exit);
		atomic_set(&xrisp_rproc->ssmmu_th_stop, 0);
		xrisp_rproc->ssmmu_map_result = 0;

		xrisp_rproc->ssmmu_map_th =
			kthread_run(xrisp_provider_buf_ssmmu_map_thread, NULL, "ssmmu_map");
		if (IS_ERR(xrisp_rproc->ssmmu_map_th)) {
			XRISP_PR_ERROR("provider_buf ssmmu_map_thread run failed");
			ret = -EINVAL;
			goto power_off;
		}

		ret = xrisp_sec_fw_load();
		if (ret) {
			XRISP_PR_ERROR("sec fw load failed, ret=%d", ret);
			goto ssmmu_unmap_buf;
		}
	}

	/* Register memory regions */
	rmemnp = of_parse_phandle(np, "memory-region", 0);
	if (!rmemnp)
		XRISP_PR_INFO("OF: has no memory-region node");
	else {
		XRISP_PR_DEBUG("OF: parse reserved mem %s", rmemnp->name);

		rmem = of_reserved_mem_lookup(rmemnp);
		if (!rmem) {
			XRISP_PR_ERROR("%s unable to acquire memory-region\n", rmemnp->name);
			ret = -EINVAL;
			goto unload_fw;
		}

		mem_phys = rmem->base;
		mem_size = rmem->size;
		XRISP_PR_INFO("start: 0x%llx, size: 0x%zx\n", mem_phys, mem_size);
	}

	of_phandle_iterator_init(&it, np, "xring,device-address", NULL, 0);
	while (of_phandle_iterator_next(&it) == 0) {
		ret = xrisp_of_parse_device_addres(it.node, &dev_addr);
		if (ret) {
			XRISP_PR_ERROR("%pOF: parse fail", it.node);
			goto unload_fw;
		}

		cam_mem = &xrisp_rproc->mem[dev_addr.region];
		memset(cam_mem, 0, sizeof(*cam_mem));
		memcpy(&cam_mem->da, &dev_addr, sizeof(dev_addr));

		if (dev_addr.type == RPROC_MEM_TYPE_RESVERED) {
			if (!rmemnp) {
				XRISP_PR_ERROR("no reserved memory-region, alloc reserved memory failed");
				ret = -EINVAL;
				goto unload_fw;
			}

			snprintf(cam_mem->name, sizeof(cam_mem->name), "%s",
				 xrisp_extra_mem[cam_mem->da.region].name);
			cam_mem->pa = mem_phys;
			cam_mem->r82pa = cam_mem->pa;
			if (cam_mem->da.size > mem_size) {
				XRISP_PR_WARN("%pOF: update device addess size", it.node);
				cam_mem->da.size = mem_size;
			}
			cam_mem->is_iommu = true;
			XRISP_PR_INFO("%s pa: 0x%llx, size: 0x%zx", cam_mem->name, cam_mem->pa,
				      cam_mem->da.size);
			mem_phys += cam_mem->da.size;
			mem_size -= cam_mem->da.size;
			/* Register memory region */
			da = cam_mem->da.base;
			mem = rproc_mem_entry_init(xrisp_rproc->dev, NULL, 0,
						   cam_mem->da.size, da,
						   xrisp_rproc_rmem_alloc,
						   xrisp_rproc_rmem_release,
						   cam_mem->name);
		} else if (dev_addr.type == RPROC_MEM_TYPE_DMABUF) {
			// sec_boot no alloc RPROC_MEM_REGION_BOOTADDR
			if (check_sec_scene(xrisp_rproc->scene) &&
			    dev_addr.region == RPROC_MEM_REGION_BOOTADDR)
				continue;

			cam_mem->is_iommu = true;
			snprintf(cam_mem->name, sizeof(cam_mem->name), "%s",
				 xrisp_extra_mem[cam_mem->da.region].name);
			/* Register memory region */
			da = cam_mem->da.base;
			mem = rproc_mem_entry_init(xrisp_rproc->dev, NULL, 0,
						   cam_mem->da.size, da,
						   xrisp_rproc_buf_alloc,
						   xrisp_rproc_buf_release,
						   cam_mem->name);
		} else if (dev_addr.type == RPROC_MEM_TYPE_CMA) {
			cam_mem->is_iommu = true;
			snprintf(cam_mem->name, sizeof(cam_mem->name), "%s",
				 xrisp_extra_mem[cam_mem->da.region].name);
			/* Register memory region */
			da = cam_mem->da.base;
			mem = rproc_mem_entry_init(xrisp_rproc->dev, NULL, 0,
						   cam_mem->da.size, da,
						   xrisp_rproc_cma_alloc,
						   xrisp_rproc_cma_release,
						   cam_mem->name);
		} else if (dev_addr.type == RPROC_MEM_TYPE_EXTRA) {
			cam_mem->is_iommu = xrisp_extra_mem[cam_mem->da.region].is_iommu;
			snprintf(cam_mem->name, sizeof(cam_mem->name), "%s",
				 xrisp_extra_mem[cam_mem->da.region].name);
			cam_mem->pa = xrisp_extra_mem[cam_mem->da.region].pa;
			cam_mem->r82pa = xrisp_extra_mem[cam_mem->da.region].r82pa;
			if (!cam_mem->is_iommu) {
				cam_mem->da.base = xrisp_extra_mem[cam_mem->da.region].da;
				cam_mem->da.size =
					xrisp_extra_mem[cam_mem->da.region].size;
			}
			if (cam_mem->da.size < xrisp_extra_mem[cam_mem->da.region].size) {
				XRISP_PR_ERROR("%pOF: extra mem bigger than device addess",
					       it.node);
				ret = -ENOMEM;
				goto unload_fw;
			} else if (cam_mem->da.size >
				   xrisp_extra_mem[cam_mem->da.region].size) {
				XRISP_PR_WARN("%pOF: update device addess size", it.node);
				cam_mem->da.size =
					xrisp_extra_mem[cam_mem->da.region].size;
			}
			/* Register memory region */
			da = cam_mem->da.base;
			mem = rproc_mem_entry_init(xrisp_rproc->dev, NULL, 0,
						   cam_mem->da.size, da,
						   xrisp_rproc_rmem_alloc,
						   xrisp_rproc_rmem_release,
						   cam_mem->name);

		} else {
			XRISP_PR_ERROR("%pOF: unknown mem type", it.node);
			ret = -EINVAL;
			goto unload_fw;
		}

		if (!mem) {
			ret = -ENOMEM;
			goto unload_fw;
		}

		mem->priv = cam_mem;

		if (cam_mem->da.region == RPROC_MEM_REGION_BOOTADDR)
			rproc_coredump_add_segment(rproc, da, mem->len);

		rproc_add_carveout(rproc, mem);
	}
	XRISP_PR_INFO("rproc prepare finish\n");
	return 0;

unload_fw:
	if (check_sec_scene(xrisp_rproc->scene))
		xrisp_sec_fw_unload();
ssmmu_unmap_buf:
	if (check_sec_scene(xrisp_rproc->scene)) {
		xrisp_provider_buf_ssmmu_map_thread_stop(xrisp_rproc->ssmmu_map_th, 0);

		xrisp_ssmmu_unmap_all();
	}
power_off:
	xrisp_power_off(xrisp_rproc);
	return ret;
}

__printf(2, 3)
struct rproc_mem_entry *
xrisp_rproc_find_carveout_by_name(struct rproc *rproc, const char *name, ...)
{
	va_list args;
	char _name[32];
	struct rproc_mem_entry *carveout, *mem = NULL;

	if (!name)
		return NULL;

	va_start(args, name);
	vsnprintf(_name, sizeof(_name), name, args);
	va_end(args);

	list_for_each_entry(carveout, &rproc->carveouts, node) {
		/* Compare carveout and requested names */
		if (!strcmp(carveout->name, _name)) {
			mem = carveout;
			break;
		}
	}

	return mem;
}

static int xrisp_rproc_unprepare(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct xrisp_rproc_vdev *rvdev, *rvtmp;

	list_for_each_entry_safe(rvdev, rvtmp, &xrisp_rproc->rvdevs, node) {
		kref_put(&rvdev->refcount, xrisp_rproc_vdev_release);
	}

	if (check_sec_scene(xrisp_rproc->scene))
		xrisp_sec_fw_unload();

	xrisp_power_off(xrisp_rproc);

	return 0;
}

static void xrisp_rproc_kick(struct rproc *rproc, int vqid)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	uint32_t msg[2] = {
		XRISP_RPROC_MSG_KICK,
		vqid
	};

	// XRISP_PR_DEBUG("rproc kikc vqid %d", vqid);
	if (xrisp_rproc->skip_first_kick) {
		XRISP_PR_INFO("skip first kick");
		xrisp_rproc->skip_first_kick = false;
		return;
	}

	ret = xrisp_rproc_mbox_send_sync(msg, sizeof(msg));
	if (ret)
		XRISP_PR_ERROR("kick fail ret = %d", ret);

}

static void *xrisp_rproc_da_to_va(struct rproc *rproc, u64 da, size_t len, bool *is_iomem)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	int i = 0;
	void *va = NULL;
	struct cam_rproc_mem *mem = xrisp_rproc->mem;

	if (len == 0)
		return NULL;

	for (i = RPROC_MEM_REGION_START; i < RPROC_MEM_REGION_MAX; i++) {
		if (da >= mem[i].da.base &&
		    da + len < mem[i].da.base + mem[i].da.size) {
			s64 offset = da - mem[i].da.base;

			va = (__force void *)(mem[i].va + offset);
			if (is_iomem)
				*is_iomem = mem[i].is_iomem;
			break;
		}
	}

	XRISP_PR_DEBUG("da 0x%llx  len 0x%zx va %pK iomem %s",
		  da, len, va, is_iomem ? "Y" : "N");

	return va;
}

static int xrisp_rproc_load_rsc_table(struct rproc *rproc)
{
	bool is_iomem = false;
	void *ptr;
	struct cam_rproc *xrisp_rproc = rproc->priv;

	ptr = rproc_da_to_va(rproc, XRISP_RPROC_RSC_ADDR, XRISP_RPROC_RSC_SIZE, &is_iomem);

	if (!ptr)
		return -ENOMEM;

	if (is_iomem)
		memcpy_toio((void __iomem *)ptr, &xrisp_rproc->rsc_table,
			    xrisp_rproc->rsc_table_size);
	else
		memcpy(ptr, &xrisp_rproc->rsc_table, xrisp_rproc->rsc_table_size);

	return 0;
}

static int xrisp_rproc_load(struct rproc *rproc, const struct firmware *fw)
{
	int ret  = 0;
	bool is_iomem = false;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	void *ptr;

	ret = xrisp_rproc_load_rsc_table(rproc);
	if (ret) {
		XRISP_PR_ERROR("rproc load rsc_table fail");
		return ret;
	}
	//sec_boot no load fw
	if (check_sec_scene(xrisp_rproc->scene))
		return 0;

	if (rproc->elf_class != ELFCLASS64) {
		ptr = rproc_da_to_va(rproc, XRISP_RPROC_BOOT_ADDR, fw->size, &is_iomem);
		if (!ptr) {
			XRISP_PR_ERROR("bad da 0x%x mem 0x%zx", XRISP_RPROC_RSC_ADDR, fw->size);
			return -EINVAL;
		}
		if (fw->size) {
			if (is_iomem)
				memcpy_toio((void __iomem *)ptr, fw->data, fw->size);
			else
				memcpy(ptr, fw->data, fw->size);
		}
	} else {
		ret = rproc_elf_load_segments(rproc, fw);
	}

	return ret;
}

static int xrisp_rproc_parse_fw(struct rproc *rproc, const struct firmware *fw)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;

	rproc->cached_table = kmemdup(&xrisp_rproc->rsc_table,
				      xrisp_rproc->rsc_table_size,
				      GFP_KERNEL);
	if (!rproc->cached_table)
		return -ENOMEM;

	rproc->table_ptr = rproc->cached_table;
	rproc->table_sz = xrisp_rproc->rsc_table_size;

	return ret;
}

static int xrisp_rproc_attach(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	unsigned long timeout;
	int ret;

	XRISP_PR_DEBUG("rproc attach");

	rproc->elf_class = ELFCLASSNONE;
	rproc->elf_machine = EM_NONE;

	rproc->bootaddr = XRISP_RPROC_BOOT_ADDR;
	xrisp_rproc_load(rproc, NULL); //load rsc_table, not need load fw
	rproc->table_ptr = rproc_da_to_va(rproc, XRISP_RPROC_RSC_ADDR, XRISP_RPROC_RSC_SIZE, NULL);

	ret = xrisp_rproc_start(rproc);
	if (ret) {
		XRISP_PR_ERROR("rproc start failed.");
		goto reset_table_ptr;
	}

	XRISP_PR_DEBUG("wait provider_buf ssmmu_map_thread start");
	timeout = wait_for_completion_timeout(&g_xrproc->ssmmu_map_th_exit,
					      msecs_to_jiffies(SEC_SSMMU_THREAD_MAP_WAIT_TIME_MS));
	if (timeout)
		XRISP_PR_INFO("provider_buf ssmmu_map_thread use %u ms",
			      SEC_SSMMU_THREAD_MAP_WAIT_TIME_MS - jiffies_to_msecs(timeout));
	else {
		xrisp_provider_buf_ssmmu_map_thread_stop(g_xrproc->ssmmu_map_th, 0);
		xrisp_ssmmu_unmap_all();

		XRISP_PR_ERROR("wait provider_buf ssmmu_map_thread timeout, stop thread");
		ret = -ETIMEDOUT;
		goto stop_rproc;
	}

	if (xrisp_rproc->ssmmu_map_result) {
		ret = xrisp_rproc->ssmmu_map_result;
		xrisp_ssmmu_unmap_all();
		XRISP_PR_ERROR("provider_buf ssmmu_map_thread fail, ret %d",
			       xrisp_rproc->ssmmu_map_result);
		goto stop_rproc;
	}

	return 0;

stop_rproc:
	xrisp_rproc_stop(rproc);

reset_table_ptr:
	rproc->table_ptr = (struct resource_table *)(&xrisp_rproc->rsc_table);

	return ret;
}

static int xrisp_rproc_detach(struct rproc *rproc)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;

	XRISP_PR_DEBUG("rproc detach");
	xrisp_rproc_stop(rproc);

	if (check_sec_scene(xrisp_rproc->scene))
		xrisp_ssmmu_unmap_all();
	return 0;
}

typedef int (*handle_vendor_resource_t)(struct rproc *rproc,
				 void *, int offset, int avail);

static handle_vendor_resource_t vendor_rsc_handlers[XRISP_RSC_VENDOR_LAST] = {
	[XRISP_RSC_VENDOR_RAMLOG] = xrisp_rproc_handle_ramlog,
	[XRISP_RSC_VENDOR_VDEV] = xrisp_rproc_handle_vdev,
};

static int xrisp_rproc_handle_rsc(struct rproc *rproc, u32 rsc_type, void *rsc,
				  int offset, int avail)
{
	handle_vendor_resource_t handler;

	if (rsc_type < XRISP_RSC_VENDOR_START ||
	    rsc_type >= XRISP_RSC_VENDOR_LAST) {
		XRISP_PR_WARN("unknown vendor rsc type");
		return RSC_IGNORED;
	}
	handler = vendor_rsc_handlers[rsc_type];
	if (!handler) {
		XRISP_PR_WARN("no handler to this vendor rsc type %d", rsc_type);
		return RSC_IGNORED;
	}

	return handler(rproc, rsc, offset, avail);
}

struct resource_table *xrisp_rproc_find_loaded_rsc_table(struct rproc *rproc,
						       const struct firmware *fw)
{
	struct resource_table *rsc_tab;

	pr_rsc_table(rproc->priv);

	rsc_tab = rproc_da_to_va(rproc, XRISP_RPROC_RSC_ADDR,
			XRISP_RPROC_RSC_SIZE, NULL);

	XRISP_PR_DEBUG("rsc table 0x%pK", rsc_tab);
	return rsc_tab;
}

static struct resource_table *xrisp_rproc_get_loaded_rsc_table(struct rproc *rproc,
							       size_t *table_sz)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;

	*table_sz = xrisp_rproc->rsc_table_size;

	return (struct resource_table *)(&xrisp_rproc->rsc_table);
}

static int xrisp_rproc_fw_sanity_check(struct cam_rproc *xrisp_rproc, const struct firmware *fw)
{
	int ret = 0;
	return ret;
}

static int xrisp_rproc_sanity_check(struct rproc *rproc, const struct firmware *fw)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct elf32_hdr *ehdr;

	ret = xrisp_rproc_fw_sanity_check(xrisp_rproc, fw);
	if (ret) {
		XRISP_PR_ERROR("xrisp fw check fail ret = %d", ret);
		return ret;
	}
	ehdr = (struct elf32_hdr *)fw->data;
	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
		XRISP_PR_WARN("load bin fmt fw");
		rproc->elf_class = ELFCLASSNONE;
		rproc->elf_machine = EM_NONE;
	} else {
		rproc->elf_class = ELFCLASS64;
		rproc->elf_machine = EM_AARCH64;
		ret = rproc_elf_sanity_check(rproc, fw);
		if (ret)
			XRISP_PR_ERROR("fw elf fmt cheak fail ret = %d", ret);
	}
	XRISP_PR_DEBUG("elf_class %d elf_machine %d", rproc->elf_class, rproc->elf_machine);
	return ret;
}

u64 xrisp_rproc_get_boot_addr(struct rproc *rproc, const struct firmware *fw)
{
	u64 boot_addr;

	if (rproc->elf_class != ELFCLASS64) {
		boot_addr = XRISP_RPROC_BOOT_ADDR;
		XRISP_PR_DEBUG("bin fmt boot_addr 0x%llx", boot_addr);
	} else {
		boot_addr = rproc_elf_get_boot_addr(rproc, fw);
		XRISP_PR_DEBUG("elf fmt boot_addr 0x%llx", boot_addr);
	}
	return boot_addr;
}

static void xrisp_rproc_coredump(struct rproc *rproc)
{
	if (rproc->dump_conf == RPROC_COREDUMP_DISABLED)
		return;
	rproc_coredump(rproc);
}

static const struct rproc_ops ipcm_rporc_ops = {
	.prepare = xrisp_rproc_prepare,
	.unprepare = xrisp_rproc_unprepare,
	.start = xrisp_rproc_start,
	.stop = xrisp_rproc_stop,
	.attach = xrisp_rproc_attach,
	.detach = xrisp_rproc_detach,
	.kick = xrisp_rproc_kick,
	.da_to_va = xrisp_rproc_da_to_va,
	.parse_fw = xrisp_rproc_parse_fw,
	.handle_rsc = xrisp_rproc_handle_rsc,
	.find_loaded_rsc_table = xrisp_rproc_find_loaded_rsc_table,
	.get_loaded_rsc_table = xrisp_rproc_get_loaded_rsc_table,
	.load = xrisp_rproc_load,
	.sanity_check = xrisp_rproc_sanity_check,
	.get_boot_addr = xrisp_rproc_get_boot_addr,
	.coredump = xrisp_rproc_coredump,
};

#define ISP_OCM_START_PA_ADDR 0x840000000
#define ISP_OCM_START_VA_ADDR 0xA8000000
#define ISP_OCM_SIZE          0x440000

int xrisp_rproc_ocm_map(unsigned long pa, size_t len)
{

	int ret = 0;
	long offset = pa - ISP_OCM_START_PA_ADDR;

	if (!g_xrproc) {
		XRISP_PR_ERROR("g_rproc is NULL");
		return -1;
	}

	if (offset < 0 || offset + len > ISP_OCM_SIZE) {
		XRISP_PR_ERROR("offset is invaild");
		return -1;
	}

	ret = iommu_map(g_xrproc->domain, ISP_OCM_START_VA_ADDR + offset, pa, len,
			IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
	return ret;
}

void xrisp_rproc_ocm_unmap(unsigned long va, size_t len)
{

	iommu_unmap(g_xrproc->domain, va, len);
}

unsigned long xrisp_rproc_ocm_pa_to_da(unsigned long pa, size_t len)
{
	if (pa >= XRISP_R82_NPUOCM_ADDR &&
		pa + len <=  XRISP_R82_NPUOCM_ADDR + XRISP_RPROC_NPUOCM_SIZE)
		return pa - XRISP_R82_NPUOCM_ADDR + XRISP_R82_NPUOCM_DEVICE_ADDR;
	return 0;
}

void xrisp_rproc_rpmsg_work(struct work_struct *work)
{
	ktime_t kick_use;

	if (!g_xrproc)
		return;
	if (!xrisp_rproc_is_running()) {
		XRISP_PR_INFO("rproc has already shutdown");
		return;
	}

	xrisp_rproc_vq_interrupt(g_xrproc, 0);
	xrisp_rproc_vq_interrupt(g_xrproc, 1);
	kick_use = ktime_ms_delta(ktime_get(), g_xrproc->kick_time);
	if (kick_use > 10)
		XRISP_PR_INFO("rpmsg work use long time %lld ms", kick_use);
}

void xrisp_rproc_boot_complete(void *priv, void *data)
{
	if (!g_xrproc)
		return;

	XRISP_PR_INFO("rproc boot complete");
	complete(&g_xrproc->boot_complete);
}

int xrisp_rpmsg_wait_complete(uint32_t times)
{
	unsigned long timeout;

	if (!g_xrproc)
		return -ENODEV;

	XRISP_PR_INFO("wait rpmsg completion");
	timeout = wait_for_completion_timeout(&g_xrproc->rpmsg_complete,
					      msecs_to_jiffies(times));
	if (timeout)
		XRISP_PR_DEBUG("rpmsg complete use time %d ms", times - jiffies_to_msecs(timeout));
	else {
		XRISP_PR_ERROR("rpmsg ept timeout");
		return -EINVAL;
	}
	return 0;
}

void xrisp_rproc_rpmsg_complete(void)
{
	if (!g_xrproc)
		return;
	complete(&g_xrproc->rpmsg_complete);
}

int xrisp_rproc_boot(uint32_t scene)
{
	int ret = 0;
	int power_cnt = 0;

	if (!g_xrproc)
		return -ENODEV;

	power_cnt = atomic_read(&g_xrproc->rproc->power);
	XRISP_PR_INFO("xrisp rproc scene = %d, state = %d, power_cnt=%d, attach_mode = %d", scene,
		      g_xrproc->rproc->state, power_cnt, g_xrproc->attach_mode);

	if (power_cnt > 0) {
		XRISP_PR_WARN("isp already power on, exit");
		return -EBUSY;
	}

	if (check_isp_attach_boot(scene)) {
		if (xrisp_rproc_set_attach_mode(true))
			return -EINVAL;
	} else if (check_isp_norm_boot(scene)) {
		if (xrisp_rproc_set_attach_mode(false))
			return -EINVAL;
	} else
		XRISP_PR_ERROR("unknown scene");

	g_xrproc->scene = scene;
	ret = rproc_boot(g_xrproc->rproc);
	if (ret) {
		g_xrproc->scene = XRISP_NORMAL_BOOT;
		return ret;
	}

	return 0;
}

int xrisp_rproc_shutdown(void)
{
	int power_cnt = 0;
	int ret = 0;

	if (!g_xrproc)
		return -ENODEV;

	power_cnt = atomic_read(&g_xrproc->rproc->power);
	if (power_cnt <= 0) {
		XRISP_PR_WARN("isp already power off, exit");
		return -EBUSY;
	}

	XRISP_PR_INFO("xrisp rproc scene =%d, state = %d, power_cnt=%d, attach_mode = %d",
		      g_xrproc->scene, g_xrproc->rproc->state, power_cnt, g_xrproc->attach_mode);

	if (check_isp_attach_boot(g_xrproc->scene) && g_xrproc->rproc->state == RPROC_ATTACHED) {
		ret = rproc_detach(g_xrproc->rproc);
	} else if (check_isp_norm_boot(g_xrproc->scene) && g_xrproc->rproc->state == RPROC_RUNNING)
		ret = rproc_shutdown(g_xrproc->rproc);
	else {
		XRISP_PR_ERROR("unknown state, scene=%d, attach mode=%d, state=%s", g_xrproc->scene,
			       g_xrproc->attach_mode, RPROC_STATUS(g_xrproc->rproc->state));
		ret = -EINVAL;
	}
	g_xrproc->scene = XRISP_NORMAL_BOOT;
	return ret;
}

int xrisp_rproc_set_attach_mode(bool attach_mode)
{
	if (!g_xrproc)
		return -ENODEV;

	if (g_xrproc->rproc->state == RPROC_OFFLINE ||
	    g_xrproc->rproc->state == RPROC_DETACHED) {
		g_xrproc->attach_mode = attach_mode;
		if (attach_mode)
			g_xrproc->rproc->state = RPROC_DETACHED;
		else
			g_xrproc->rproc->state = RPROC_OFFLINE;

		return 0;
	}

	XRISP_PR_ERROR("can not change mode when rproc running or attach");
	return -EINVAL;
}

bool xrisp_mcu_smmu_is_poweron(void)
{
	if (!g_xrproc)
		return false;
	return regulator_is_enabled(g_xrproc->mcu_rgltr);
}

int xrisp_mcu_smmu_power_on(void)
{
	if (!g_xrproc)
		return -ENODEV;
	return xrisp_power_on(g_xrproc);
}

int xrisp_mcu_smmu_power_off(void)
{
	if (!g_xrproc)
		return -ENODEV;
	xrisp_power_off(g_xrproc);
	return 0;
}

int xrisp_mcu_set_rate(int rate)
{
	if (rate != XRISP_MCU_CLK_RATE_1200M && rate != XRISP_MCU_CLK_RATE_557M) {
		XRISP_PR_ERROR("input mcu rate invalid, in_rate=%d", rate);
		return -EINVAL;
	}

	if (g_xrproc->mcu_rgltr == NULL ||
	    regulator_is_enabled(g_xrproc->mcu_rgltr) == 0) {
		XRISP_PR_ERROR("mcu rate set need to rgltr_isp_mcu enable, exit");
		return -EINVAL;
	}

	if (xrisp_clk_api_set_rate_by_mask(XRISP_MCU_CLK, (1 << ISP_MCU_OUT_MAX) - 1, rate)) {
		XRISP_PR_ERROR("mcu rate set fail, rate=%d", rate);
		return -EINVAL;
	}
	if (rate == XRISP_MCU_CLK_RATE_1200M)
		XRISP_PR_INFO("mcu rate set 1200M success");
	else
		XRISP_PR_INFO("mcu rate set 557M success");

	return 0;
}

static int xrisp_bdinfo_dts_parse(struct cam_rproc *xrisp_rproc)
{
	static const char *const mcu_clk_name[] = { "clk_isp_mcu", "pclk_isp_uart",
						    "clk_isp_timer" };
	struct fw_rsc_bd_info *bdinfo_list;
	struct device_node *np;
	int match_idx;
	int ret = 0;
	int i, j;

	if (!xrisp_rproc)
		return -EINVAL;

	np = of_find_node_by_name(NULL, "xrisp_mcu_clk");
	if (!np) {
		XRISP_PR_ERROR("failed to find mcu_clk node\n");
		return -EINVAL;
	}

	bdinfo_list = xrisp_rproc->bdinfo_list;

	for (i = 0; i < ARRAY_SIZE(mcu_clk_name); i++) {
		match_idx = of_property_match_string(np, "clk-names", mcu_clk_name[i]);
		if (match_idx < 0) {
			XRISP_PR_ERROR("failed to find clk name %s\n", mcu_clk_name[i]);
			return -EINVAL;
		}

		for (j = 0; j < XRISP_MCU_CLK_RATE_MAX; j++) {
			ret = of_property_read_u32_index(np, xrisp_mcu_rate_prop_name[j], match_idx,
							 (uint32_t *)(bdinfo_list + j) + i);
			if (ret) {
				XRISP_PR_ERROR("failed to read %s %s\n", xrisp_mcu_rate_prop_name[j],
				       mcu_clk_name[i]);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int xrisp_clk_name_to_id(const char *name, const char *clk_arr_name[], int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (!strcmp(name, clk_arr_name[i]))
			return i;
	}
	return -EINVAL;
}

static void xrisp_scene_set_rate_mask(uint32_t scene, struct scene_rate_info *scene_info)
{
	switch (scene) {
	case XRISP_NORMAL_BOOT:
	case XRISP_SAFE_FACE_BOOT:
	case XRISP_NSAFE_FACE_BOOT:
		scene_info->crg_mask = (1 << CLK_ISP_CRG_OUT_MAX) - 1;
		break;

	case XRISP_SAFE_AON_BOOT:
		scene_info->crg_mask = ((1 << CLK_ISP2CSI_FE) | (1 << CLK_ISP_CVE) |
					(1 << CLK_ISP_FE_CORE0) | (1 << CLK_ISP_FE_ROUTER));
		break;

	case XRISP_SHAKE_BOOT:
		scene_info->crg_mask = 0;
		break;

	default:
		scene_info->crg_mask = 0;
	}

	scene_info->mcu_mask = (1 << ISP_MCU_OUT_MAX) - 1;
}

static int xrisp_scene_info_dts_parse(struct cam_rproc *xrisp_rproc)
{
	const char *mcu_rate_name, *crg_rate_name;
	struct scene_rate_info *scene_info;
	struct device_node *np_scene;
	struct device_node *np;
	int scene_num = 0;
	int ret = 0;
	int i = 0;

	if (!xrisp_rproc)
		return -EINVAL;

	np = xrisp_rproc->dev->of_node;
	np_scene = of_get_child_by_name(np, "scene-info");
	if (!np_scene) {
		XRISP_PR_ERROR("failed to get scene info node\n");
		return -EINVAL;
	}

	scene_num = of_property_count_strings(np_scene, "scene_names");
	if (scene_num < 0 || scene_num != XRISP_BOOT_SCENE_MAX) {
		XRISP_PR_ERROR("failed to get scene_names num, scene_num=%d\n", scene_num);
		return -EINVAL;
	}

	scene_info = xrisp_rproc->scene_rate_info_list;

	for (i = 0; i < scene_num; i++) {
		ret = of_property_read_string_index(np_scene, "scene_names", i,
						    &scene_info[i].scene_name);
		if (ret) {
			XRISP_PR_ERROR("failed to get idx=%d scene name\n", i);
			return -EINVAL;
		}
	}

	for (i = 0; i < scene_num; i++) {
		ret = of_property_read_string_index(np_scene, scene_info[i].scene_name, 0,
						    &mcu_rate_name);
		if (ret) {
			XRISP_PR_ERROR("failed to get scene:%s mcu_rate_name, ret = %d\n",
				       scene_info[i].scene_name, ret);
			return -EINVAL;
		}
		ret = xrisp_clk_name_to_id(mcu_rate_name, xrisp_mcu_rate_prop_name,
					   XRISP_MCU_CLK_RATE_MAX);
		if (ret < 0) {
			XRISP_PR_ERROR("failed to get scene:%s mcu_rate_id(%s)\n",
				       scene_info[i].scene_name, mcu_rate_name);
			return -EINVAL;
		}
		scene_info[i].mcu_rate_id = ret;
	}

	for (i = 0; i < scene_num; i++) {
		ret = of_property_read_string_index(np_scene, scene_info[i].scene_name, 1,
						    &crg_rate_name);
		if (ret) {
			XRISP_PR_ERROR("failed to get scene:%s crg_rate_name, ret = %d\n",
				       scene_info[i].scene_name, ret);
			return -EINVAL;
		}
		ret = xrisp_clk_name_to_id(crg_rate_name, xrisp_rate_prop_name, XRISP_CLK_RATE_MAX);
		if (ret < 0) {
			XRISP_PR_ERROR("failed to get scene:%s crg_rate_id(%s)\n",
				       scene_info[i].scene_name, crg_rate_name);
			return -EINVAL;
		}
		scene_info[i].crg_rate_id = ret;
	}

	for (i = 0; i < scene_num; i++)
		xrisp_scene_set_rate_mask(i, &scene_info[i]);

	XRISP_PR_INFO("get scene info success\n");
	return 0;
}

__maybe_unused static int xrisp_rproc_reserve_cma_alloc(struct cam_rproc *xrisp_rproc)
{
	struct cam_rproc_reserve_cma rcam_arr[RPROC_MEM_REGION_MAX] = { 0 };
	struct cam_rproc_reserve_cma *rcma;
	struct device_node *np, *sub_np;
	struct of_phandle_iterator it;
	uint32_t mem_type;
	const __be32 *cell;
	int len;
	int ret;
	int i;

	if (!xrisp_rproc)
		return -EINVAL;

	np = xrisp_rproc->dev->of_node;
	xrisp_rproc->cma_region_num = 0;

	ret = of_phandle_iterator_init(&it, np, "xring,device-address", NULL, 0);
	if (ret) {
		XRISP_PR_ERROR("failed to get xring,device-address\n");
		return -ENODEV;
	}

	while (of_phandle_iterator_next(&it) == 0) {
		sub_np = it.node;
		ret = of_property_read_u32(sub_np, "mem-type", &mem_type);
		if (ret) {
			XRISP_PR_ERROR("failed to get mem-type\n");
			goto free_rcma;
		}

		if (mem_type == RPROC_MEM_TYPE_CMA &&
		    xrisp_rproc->cma_region_num < RPROC_MEM_REGION_MAX) {
			rcma = &rcam_arr[xrisp_rproc->cma_region_num];

			cell = of_get_property(sub_np, "reg", &len);
			if (!cell) {
				XRISP_PR_ERROR("%pOF: missing reg property", sub_np);
				goto free_rcma;
			}
			if (len / 4 != of_n_addr_cells(sub_np) + of_n_size_cells(sub_np)) {
				XRISP_PR_ERROR("%pOF: missing reg cells (%d)", sub_np, len);
				goto free_rcma;
			}

			cell += of_n_addr_cells(sub_np);
			rcma->size = of_read_number(cell, of_n_size_cells(sub_np));

			ret = of_property_read_u32(sub_np, "iova-region", &rcma->region);
			if (ret || rcma->region < RPROC_MEM_REGION_START ||
			    rcma->region >= RPROC_MEM_REGION_MAX) {
				XRISP_PR_ERROR("%pOF: has no or invalid iova-region %d", sub_np,
					       rcma->region);
				goto free_rcma;
			}

			rcma->va = dma_alloc_attrs(xrisp_rproc->dev, rcma->size, &rcma->dma_handle,
						   GFP_KERNEL, DMA_ATTR_FORCE_CONTIGUOUS);
			if (!rcma->va) {
				XRISP_PR_ERROR("region %d failed to alloc cma, size %zu",
					       rcma->region, rcma->size);
				ret = -ENOMEM;
				goto free_rcma;
			}
			XRISP_PR_DEBUG(
				"region %d alloc cma success, va=0x%pK, pa=0x%pK, size=%zu\n",
				rcma->region, rcma->va, (void *)rcma->dma_handle, rcma->size);
			xrisp_rproc->cma_region_num++;
		}
	}

	xrisp_rproc->reserve_cma = kzalloc(
		sizeof(struct cam_rproc_reserve_cma) * xrisp_rproc->cma_region_num, GFP_KERNEL);
	if (!xrisp_rproc->reserve_cma) {
		XRISP_PR_ERROR("failed to alloc reserve_cma");
		ret = -ENOMEM;
		goto free_rcma;
	}

	memcpy(xrisp_rproc->reserve_cma, rcam_arr,
	       sizeof(struct cam_rproc_reserve_cma) * xrisp_rproc->cma_region_num);

	XRISP_PR_INFO("alloc reserve cma success\n");

	return 0;

free_rcma:
	for (i = 0; i < xrisp_rproc->cma_region_num; i++) {
		if (rcam_arr[i].va)
			dma_free_attrs(xrisp_rproc->dev, rcam_arr[i].size, rcam_arr[i].va,
				       rcam_arr[i].dma_handle, DMA_ATTR_FORCE_CONTIGUOUS);
	}
	xrisp_rproc->reserve_cma = NULL;
	xrisp_rproc->cma_region_num = 0;

	return ret;
}

__maybe_unused static void xrisp_rproc_reserve_cma_free(struct cam_rproc *xrisp_rproc)
{
	int i;

	if (!xrisp_rproc->reserve_cma || !xrisp_rproc->cma_region_num)
		return;

	for (i = 0; i < xrisp_rproc->cma_region_num; i++) {
		if (xrisp_rproc->reserve_cma[i].va)
			dma_free_attrs(xrisp_rproc->dev, xrisp_rproc->reserve_cma[i].size,
				       xrisp_rproc->reserve_cma[i].va,
				       xrisp_rproc->reserve_cma[i].dma_handle,
				       DMA_ATTR_FORCE_CONTIGUOUS);
	}

	kfree(xrisp_rproc->reserve_cma);
	xrisp_rproc->reserve_cma = NULL;
	xrisp_rproc->cma_region_num = 0;
}

static int xrisp_rproc_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct cam_rproc *xrisp_rproc;
	struct rproc *rproc;
	const char *fw_name;

	XRISP_PR_INFO("probe start");

	ret = xring_isp_reg_iomap(reg_map_list, ARRAY_SIZE(reg_map_list));
	if (ret)
		return ret;

	ret = rproc_of_parse_firmware(&pdev->dev, 0, &fw_name);
	if (ret) {
		XRISP_DEV_ERROR(&pdev->dev, "failed to parse firmware-name property, ret = %d\n",
			ret);
		return ret;
	}

	ret = xrisp_rproc_async_ctl_init();
	if (ret)
		return ret;

	rproc = devm_rproc_alloc(&pdev->dev, pdev->name, &ipcm_rporc_ops,
				  fw_name, sizeof(*xrisp_rproc));
	if (!rproc) {
		XRISP_DEV_ERROR(&pdev->dev, "rproc alloc fail");
		return -ENOMEM;
	}

	XRISP_DEV_INFO(&pdev->dev, "rproc index %d", rproc->index);

	rproc->auto_boot = false;
	rproc->dump_conf = RPROC_COREDUMP_ENABLED;

	xrisp_rproc = (struct cam_rproc *)rproc->priv;
	g_xrproc = xrisp_rproc;
	dev_set_name(&rproc->dev, "xrisp-rproc");
	xrisp_rproc->dev = &pdev->dev;
	xrisp_rproc->rproc = rproc;
	platform_set_drvdata(pdev, xrisp_rproc);

	xrisp_rproc->workqueue = alloc_workqueue(
		"%s", __WQ_LEGACY | WQ_MEM_RECLAIM | WQ_HIGHPRI | WQ_UNBOUND, 1, "xrisp-rproc-wq");
	if (!xrisp_rproc->workqueue) {
		XRISP_PR_ERROR("create workqueue fail");
		goto free_rproc;
	}

	INIT_WORK(&xrisp_rproc->kick_work, xrisp_rproc_rpmsg_work);
	INIT_WORK(&xrisp_rproc->time_sync_work, xrisp_rproc_time_sync_work);

	xrisp_rproc->domain = iommu_get_domain_for_dev(xrisp_rproc->dev);
	if (!xrisp_rproc->domain) {
		XRISP_DEV_ERROR(&pdev->dev, "get iommu domain fail");
		ret = -ENODEV;
		goto destroy_workqueue;
	}

	xrisp_rproc->mcu_rgltr = devm_regulator_get(xrisp_rproc->dev, "isp_mcu_top");
	if (IS_ERR_OR_NULL(xrisp_rproc->mcu_rgltr)) {
		ret = -ENODEV;
		goto destroy_workqueue;
	}

	xrisp_rproc->media2_rgltr = devm_regulator_get(xrisp_rproc->dev, "media2_subsys");
	if (IS_ERR_OR_NULL(xrisp_rproc->media2_rgltr)) {
		ret = -ENODEV;
		goto destroy_workqueue;
	}

	if (xrisp_scene_info_dts_parse(xrisp_rproc))
		goto destroy_workqueue;
	if (xrisp_bdinfo_dts_parse(xrisp_rproc))
		goto destroy_workqueue;

#ifdef CONFIG_XRISP_RPROC_RESERVE_CMA
	ret = xrisp_rproc_reserve_cma_alloc(xrisp_rproc);
	if (ret)
		goto destroy_workqueue;
#endif

	init_completion(&xrisp_rproc->boot_complete);
	init_completion(&xrisp_rproc->rpmsg_complete);
	init_completion(&xrisp_rproc->ssmmu_map_th_exit);

	xrisp_rproc->xrisp_wakelock = wakeup_source_register(NULL, "xrisp_wakelock");
	if (!xrisp_rproc->xrisp_wakelock) {
		pr_err("wakelock register fail");
		goto free_reserve_cma;
	}

	xrisp_rproc->fw = NULL;
	xrisp_rproc->attach_mode = false;

	ret = xrisp_rproc_mbox_init();
	if (ret)
		goto free_wakelock;

	xrisp_rproc_msg_register(XRISP_RPROC_MSG_BOOT_DONE,
				xrisp_rproc_boot_complete, xrisp_rproc);
	ret = devm_rproc_add(&pdev->dev, rproc);
	if (ret) {
		XRISP_DEV_ERROR(&pdev->dev, "rproc add fail ret = %d", ret);
		goto mbox_exit;
	}

	ret = xrisp_rproc_add_reg_subdev(xrisp_rproc);
	if (ret)
		goto mbox_exit;

	xrisp_rproc_add_mpu_subdev(xrisp_rproc);
	xrisp_rproc_add_ramlog_subdev(xrisp_rproc);

	idr_init(&xrisp_rproc->notifyids);
	INIT_LIST_HEAD(&xrisp_rproc->rvdevs);

	rproc_debugfs_init(xrisp_rproc);

	XRISP_PR_INFO("probe done");
	return 0;
mbox_exit:
	xrisp_rproc_mbox_exit();
free_wakelock:
	wakeup_source_unregister(xrisp_rproc->xrisp_wakelock);
free_reserve_cma:
#ifdef CONFIG_XRISP_RPROC_RESERVE_CMA
	xrisp_rproc_reserve_cma_free(xrisp_rproc);
#endif
destroy_workqueue:
	flush_workqueue(xrisp_rproc->workqueue);
	destroy_workqueue(xrisp_rproc->workqueue);
free_rproc:
	g_xrproc = NULL;
	XRISP_PR_ERROR("probe fail ret = %d", ret);

	return ret;
}

static int xrisp_rproc_remove(struct platform_device *pdev)
{
	struct cam_rproc *xrisp_rproc = NULL;

	XRISP_PR_INFO("remove start");
	xrisp_rproc = platform_get_drvdata(pdev);
	if (!xrisp_rproc)
		return -ENODEV;

	xrisp_rproc_shutdown();

	if (xrisp_rproc->fw)
		release_firmware(xrisp_rproc->fw);
	rproc_debugfs_exit(xrisp_rproc);

	wakeup_source_unregister(xrisp_rproc->xrisp_wakelock);
#ifdef CONFIG_XRISP_RPROC_RESERVE_CMA
	xrisp_rproc_reserve_cma_free(xrisp_rproc);
#endif

	idr_destroy(&xrisp_rproc->notifyids);

	xrisp_rproc_mbox_exit();
	cancel_work_sync(&xrisp_rproc->kick_work);
	flush_workqueue(xrisp_rproc->workqueue);
	destroy_workqueue(xrisp_rproc->workqueue);
	xrisp_rproc_remove_mpu_subdev(xrisp_rproc);
	xrisp_rproc_remove_reg_subdev(xrisp_rproc);
	xrisp_rproc_remove_ramlog_subdev(xrisp_rproc);

	xring_isp_reg_iounmap(reg_map_list, ARRAY_SIZE(reg_map_list));

	xrisp_rproc_async_ctl_exit();
	XRISP_PR_INFO("remove done");
	return 0;
}
#ifdef CONFIG_PM_SLEEP
static int xrisp_runtime_suspend(struct device *dev)
{
	int ret = 0;

	pr_info("%s +\n", __func__);

	if (!xrisp_rproc_is_running())
		XRISP_PR_INFO("%s isp already closed\n", __func__);
	else
		XRISP_PR_INFO("%s isp is running\n", __func__);

	pr_info("%s -\n", __func__);

	return ret;
}

static int xrisp_runtime_resume(struct device *dev)
{
	int ret = 0;

	pr_info("%s +\n", __func__);

	pr_info("%s -\n", __func__);
	return ret;
}
#else
static int xrisp_runtime_suspend(struct device *dev)
{
	return 0;
}
static int xrisp_runtime_resume(struct device *dev)
{
	return 0;
}
#endif /* !CONFIG_PM_SLEEP */

static const struct dev_pm_ops xrisp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xrisp_runtime_suspend, xrisp_runtime_resume)
};

static const struct of_device_id rproc_match_table[] = {

	{ .compatible = "xring,cam-remote-proc", },
	{}
};

static struct platform_driver xrisp_rproc_driver = {
	.probe = xrisp_rproc_probe,
	.remove = xrisp_rproc_remove,
	.driver = {
		.name = "cam-remote-proc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rproc_match_table),
		.pm = &xrisp_pm_ops,
	},
};

int xrisp_rproc_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&xrisp_rproc_driver);
	if (ret)
		XRISP_PR_ERROR("register driver failed");

	return ret;
}

void xrisp_rproc_exit(void)
{
	platform_driver_unregister(&xrisp_rproc_driver);
}

MODULE_AUTHOR("Zhen Bin<zhenbin@xiaomi.com>");
MODULE_DESCRIPTION("x-ring camera rproc");
MODULE_LICENSE("GPL v2");
