// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#include <linux/delay.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/media1_crg.h>
#include <dt-bindings/xring/platform-specific/media1_ctrl.h>
#include <dt-bindings/xring/platform-specific/media1_lpctrl_ft.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/gpu_xctrl.h>
#include <dt-bindings/xring/platform-specific/lpctrl_gpu.h>
#include <dt-bindings/xring/platform-specific/gpu_subchip_crg.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include "soc/xring/xr_timestamp.h"
#include "xr_regulator_internal.h"

struct addr_info {
	u64          phy_addr;
	void __iomem *virt_addr;
};

struct gpc_regs_info {
	struct addr_info gpc_req;
	struct addr_info gpc_intr;
};

struct gpu_subsys_regs_info {
	struct gpc_regs_info gpc_regs;
	struct addr_info autofsctrl27;
	struct addr_info adb_q_channel_cfg;
	struct addr_info q_channel_state_2;
	struct addr_info gpu_snd_sys_ctrl;
};

struct dpu_part1_regs_info {
	struct gpc_regs_info gpc_regs;
	struct addr_info mem_lpctrl_req_group0;
	struct addr_info dpu_glb_grp_ds;
};

struct vdec_core_regs_info {
	struct gpc_regs_info gpc_regs;
	struct addr_info m1_dpu_dvfs_bypass;
};

static struct gpu_subsys_regs_info gpu_subsys_regs = {
	.gpc_regs = {
		.gpc_req  = { .phy_addr = ACPU_GPU_LPCTRL + LPCTRL_GPU_GPU_TOP_GPC_PU_REQ_POLL, },
		.gpc_intr = { .phy_addr = ACPU_GPU_LPCTRL + LPCTRL_GPU_INTR_PU_GPU_TOP, },
	},
	.autofsctrl27      = { .phy_addr = ACPU_PERI_CRG + PERI_CRG_AUTOFSCTRL27, },
	.adb_q_channel_cfg = { .phy_addr = ACPU_GPU_CTRL + GPU_XCTRL_ADB_Q_CHANNEL_CFG, },
	.q_channel_state_2 = { .phy_addr = ACPU_GPU_CTRL + GPU_XCTRL_Q_CHANNEL_STATE_2, },
	.gpu_snd_sys_ctrl  = { .phy_addr = ACPU_GPU_CTRL + GPU_XCTRL_GPU_SND_SYS_CTRL, },
};

static struct dpu_part1_regs_info dpu_part1_regs = {
	.gpc_regs = {
		.gpc_req  = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART1, },
		.gpc_intr = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_INTR_PU_DPU_PART1, },
	},
	.mem_lpctrl_req_group0 = { .phy_addr = ACPU_MEDIA1_LPCTRL_FT
						+ MEDIA1_LPCTRL_FT_MEM_LPCTRL_REQ_GROUP0, },
	.dpu_glb_grp_ds        = { .phy_addr = ACPU_DPU_GLB + DPU_GLB_GRP_DS, },
};

static struct gpc_regs_info dpu_part2_regs = {
	.gpc_req  = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_GPC_PU_REQ_POLL_DPU_PART2, },
	.gpc_intr = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_INTR_PU_DPU_PART2, },
};

static struct gpc_regs_info veu_top_regs = {
	.gpc_req  = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_GPC_PU_REQ_POLL_VEU, },
	.gpc_intr = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_INTR_PU_VEU, },
};

static struct vdec_core_regs_info vdec_core_regs = {
	.gpc_regs = {
		.gpc_req    = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_GPC_PU_REQ_POLL_VDEC, },
		.gpc_intr   = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_INTR_PU_VDEC, },
	},
	.m1_dpu_dvfs_bypass = { .phy_addr = ACPU_MEDIA1_CRG + MEDIA1_CRG_M1_DPU_DVFS_BYPASS, },
};

/*
 * helper function to ensure when it returns it is at least 'delay_us'
 * after 'since'.
 */
static void ensured_time_after(u64 since_us, u32 delay_us)
{
	u64 now_us;
	u64 elapsed_us;
	u32 actual_us32 = 0;

	now_us = xr_timestamp_gettime();
	do_div(now_us, NSEC_PER_USEC);
	elapsed_us = now_us - since_us;
	if (delay_us > elapsed_us) {
		actual_us32 = (u32)(delay_us - elapsed_us);
		if (actual_us32 >= 1000) {
			msleep(actual_us32 / 1000);
			udelay(actual_us32 % 1000);
		} else if (actual_us32 > 0) {
			udelay(actual_us32);
		}
	}

	pr_debug("GPC: since:%llu, now:%llu, delay %d us",
			since_us, now_us, actual_us32);
}

static int gpc_enable(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int media1_data = 0;
	struct gpc_regs_info *gpc_regs = (struct gpc_regs_info *)sreg->gpc_priv;
	uint32_t timeout = GPC_TIMEOUT;
	int ret = 0;

	ensured_time_after(sreg->last_off_us, sreg->off_on_delay);

	writel(GPC_ENABLE, gpc_regs->gpc_req.virt_addr);

	while (timeout) {
		media1_data = readl(gpc_regs->gpc_intr.virt_addr);
		if ((media1_data & GPC_POWER_ON_SUCESS) == GPC_POWER_ON_SUCESS)
			break;
		timeout--;
		udelay(1);
	}
	if (timeout == 0) {
		dev_err(dev, "%s,gpc enable failed,timeout,sctrl_reg=0x%x!\n",
			sreg->name, media1_data);
		ret = -EAGAIN;
	}

	return ret;
}

static int gpc_disable(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int media1_data = 0;
	uint32_t timeout = GPC_TIMEOUT;
	int ret = 0;
	struct gpc_regs_info *gpc_regs = (struct gpc_regs_info *)sreg->gpc_priv;

	writel(GPC_DISABLE, gpc_regs->gpc_req.virt_addr);

	while (timeout) {
		media1_data = readl(gpc_regs->gpc_intr.virt_addr);
		if ((media1_data & GPC_POWER_OFF_SUCESS) == GPC_POWER_OFF_SUCESS)
			break;
		timeout--;
		udelay(1);
	}
	if (timeout == 0) {
		dev_err(dev, "%s,gpc disenable failed,sctrl_reg=0x%x!\n",
			sreg->name, media1_data);
		ret = -EAGAIN;
	}

	sreg->last_off_us = xr_timestamp_gettime();

	return ret;
}

static int __reg_map(struct device *dev, struct addr_info *reg_info, u32 size)
{
	if (reg_info->virt_addr == NULL) {
		reg_info->virt_addr = devm_ioremap(dev, reg_info->phy_addr, size);
		if (IS_ERR_OR_NULL(reg_info->virt_addr)) {
			dev_err(dev, "Failed to map phy_addr 0x%llx, size 0x%x, err=%ld\n",
					reg_info->phy_addr, size, PTR_ERR(reg_info->virt_addr));
			return -EFAULT;
		}
	}

	return 0;
}

static int _reg_map(struct device *dev, struct addr_info *reg_info)
{
	return __reg_map(dev, reg_info, 4);
}

static int gpu_subsys_power_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	int ret;

	ret = _reg_map(dev, &gpu_subsys_regs.gpc_regs.gpc_req);
	if (ret)
		return ret;

	ret = _reg_map(dev, &gpu_subsys_regs.gpc_regs.gpc_intr);
	if (ret)
		return ret;

	ret = _reg_map(dev, &gpu_subsys_regs.autofsctrl27);
	if (ret)
		return ret;

	ret = _reg_map(dev, &gpu_subsys_regs.adb_q_channel_cfg);
	if (ret)
		return ret;

	ret = _reg_map(dev, &gpu_subsys_regs.q_channel_state_2);
	if (ret)
		return ret;

	ret = _reg_map(dev, &gpu_subsys_regs.gpu_snd_sys_ctrl);
	if (ret)
		return ret;

	sreg->gpc_priv = &gpu_subsys_regs;

	return 0;
}

static int dpu_part1_power_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	int ret;

	ret = _reg_map(dev, &dpu_part1_regs.gpc_regs.gpc_req);
	if (ret)
		return ret;

	ret = _reg_map(dev, &dpu_part1_regs.gpc_regs.gpc_intr);
	if (ret)
		return ret;

	ret = _reg_map(dev, &dpu_part1_regs.mem_lpctrl_req_group0);
	if (ret)
		return ret;

	ret = _reg_map(dev, &dpu_part1_regs.dpu_glb_grp_ds);
	if (ret)
		return ret;

	sreg->gpc_priv = &dpu_part1_regs;

	return 0;
}

static int dpu_part2_power_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	int ret;

	ret = _reg_map(dev, &dpu_part2_regs.gpc_req);
	if (ret)
		return ret;

	ret = _reg_map(dev, &dpu_part2_regs.gpc_intr);
	if (ret)
		return ret;

	sreg->gpc_priv = &dpu_part2_regs;

	return 0;
}

static int veu_top_power_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	int ret;

	ret = _reg_map(dev, &veu_top_regs.gpc_req);
	if (ret)
		return ret;

	ret = _reg_map(dev, &veu_top_regs.gpc_intr);
	if (ret)
		return ret;

	sreg->gpc_priv = &veu_top_regs;

	return 0;
}

static int vdec_core_power_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	int ret;

	ret = _reg_map(dev, &vdec_core_regs.gpc_regs.gpc_req);
	if (ret)
		return ret;

	ret = _reg_map(dev, &vdec_core_regs.gpc_regs.gpc_intr);
	if (ret)
		return ret;

	ret = _reg_map(dev, &vdec_core_regs.m1_dpu_dvfs_bypass);
	if (ret)
		return ret;

	sreg->gpc_priv = &vdec_core_regs;

	return 0;
}

static int gpu_subsys_power_on(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	uint32_t timeout = GPC_TIMEOUT;
	struct gpu_subsys_regs_info *regs = (struct gpu_subsys_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->autofsctrl27.virt_addr == NULL)
			|| (regs->adb_q_channel_cfg.virt_addr == NULL)
			|| (regs->q_channel_state_2.virt_addr == NULL)
			|| (regs->gpu_snd_sys_ctrl.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_ON_DONE_SHIFT))
		return 0;

	writel(RG_BIT(GPU_AUTOFS_ENABLE) | RG_BIT(GPU_AUTOFS_ENABLE_MASK),
			regs->autofsctrl27.virt_addr);

	if (gpc_enable(dev, sreg))
		return -EAGAIN;

	data = readl(regs->adb_q_channel_cfg.virt_addr);
	writel(data | RG_BIT(ADB_PWR_SLV0_CLK) | RG_BIT(ADB_PWR_SLV1_CLK) |
		RG_BIT(ADB_PWR_SLV2_CLK) | RG_BIT(ADB_PWR_SLV3_CLK),
		regs->adb_q_channel_cfg.virt_addr);

	while (timeout) {
		data = readl(regs->q_channel_state_2.virt_addr);
		if ((data & GPU_ADB_PWR_SLV_STATE_ON) == GPU_ADB_PWR_SLV_STATE_ON)
			break;
		timeout--;
		udelay(1);
	}
	if (timeout == 0) {
		dev_err(dev, "%s failed,GPU_XCTRL_Q_CHANNEL_STATE_2=0x%x!\n",
			sreg->name, data);
		return -EAGAIN;
	}

	writel(RG_BIT(GPU_AUTOFS_ENABLE_MASK), regs->autofsctrl27.virt_addr);

	data = readl(regs->gpu_snd_sys_ctrl.virt_addr);
	writel(data | RG_BIT(GPU_XCTRL_GPU_SND_SYS_CTRL_GPU_SND_SYS_CTRL_16_SHIFT),
			regs->gpu_snd_sys_ctrl.virt_addr);

	return 0;
}

static int gpu_subsys_power_off(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	uint32_t timeout = GPC_TIMEOUT;
	struct gpu_subsys_regs_info *regs = (struct gpu_subsys_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->autofsctrl27.virt_addr == NULL)
			|| (regs->adb_q_channel_cfg.virt_addr == NULL)
			|| (regs->q_channel_state_2.virt_addr == NULL)
			|| (regs->gpu_snd_sys_ctrl.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}
	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(LPCTRL_GPU_INTR_PU_GPU_TOP_GPU_TOP_PWR_OFF_DONE_SHIFT)) {
		dev_dbg(dev, "gpu subsys has already power off\n");
		return 0;
	}

	writel(RG_BIT(GPU_AUTOFS_ENABLE) | RG_BIT(GPU_AUTOFS_ENABLE_MASK),
			regs->autofsctrl27.virt_addr);

	data = readl(regs->adb_q_channel_cfg.virt_addr);
	writel(data & (~(RG_BIT(ADB_PWR_SLV0_CLK) | RG_BIT(ADB_PWR_SLV1_CLK) |
		RG_BIT(ADB_PWR_SLV2_CLK) | RG_BIT(ADB_PWR_SLV3_CLK))),
			regs->adb_q_channel_cfg.virt_addr);

	while (timeout) {
		data = readl(regs->q_channel_state_2.virt_addr);
		if (!(data & GPU_ADB_PWR_SLV_STATE_OFF))
			break;
		timeout--;
		udelay(1);
	}
	if (timeout == 0) {
		dev_err(dev, "%s failed,GPU_XCTRL_Q_CHANNEL_STATE_2=0x%x!\n",
			sreg->name, data);
		return -EAGAIN;
	}

	if (gpc_disable(dev, sreg))
		return -EAGAIN;

	writel(RG_BIT(GPU_AUTOFS_ENABLE_MASK), regs->autofsctrl27.virt_addr);
	return 0;
}

static int dpu1_power_on(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct dpu_part1_regs_info *regs = (struct dpu_part1_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->mem_lpctrl_req_group0.virt_addr == NULL)
			|| (regs->dpu_glb_grp_ds.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_ON_DONE_DPU_PART1_SHIFT)) {
		dev_dbg(dev, "dpu1 has already power on\n");
		return 0;
	}

	writel(DPU1_GPC_MEMORY_ON, regs->mem_lpctrl_req_group0.virt_addr);

	data = readl(regs->dpu_glb_grp_ds.virt_addr);
	writel(data & (~(RG_BIT(DPU1_GPC_P1H0) | RG_BIT(DPU1_GPC_P1H2))),
			regs->dpu_glb_grp_ds.virt_addr);

	if (gpc_enable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int dpu1_power_off(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct dpu_part1_regs_info *regs = (struct dpu_part1_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->mem_lpctrl_req_group0.virt_addr == NULL)
			|| (regs->dpu_glb_grp_ds.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_DPU_PART1_PWR_OFF_DONE_DPU_PART1_SHIFT)) {
		dev_dbg(dev, "dpu1 has already power off\n");
		return 0;
	}

	writel(DPU1_GPC_MEMORY_OFF, regs->mem_lpctrl_req_group0.virt_addr);

	data = readl(regs->dpu_glb_grp_ds.virt_addr);
	writel(data | RG_BIT(DPU1_GPC_P1H0) | RG_BIT(DPU1_GPC_P1H2),
			regs->dpu_glb_grp_ds.virt_addr);

	if (gpc_disable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int dpu2_power_on(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct gpc_regs_info *regs = (struct gpc_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_req.virt_addr == NULL)
			|| (regs->gpc_intr.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_ON_DONE_DPU_PART2_SHIFT)) {
		dev_dbg(dev, "dpu2 has already power on\n");
		return 0;
	}

	if (gpc_enable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int dpu2_power_off(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct gpc_regs_info *regs = (struct gpc_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_req.virt_addr == NULL)
			|| (regs->gpc_intr.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_DPU_PART2_PWR_OFF_DONE_DPU_PART2_SHIFT)) {
		dev_dbg(dev, "dpu2 has already power off\n");
		return 0;
	}

	if (gpc_disable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int veu_top_subsys_power_on(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct gpc_regs_info *regs = (struct gpc_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_req.virt_addr == NULL)
			|| (regs->gpc_intr.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_VEU_PWR_ON_DONE_VEU_SHIFT)) {
		dev_dbg(dev, "veu top subsys has already power on\n");
		return 0;
	}

	if (gpc_enable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int veu_top_subsys_power_off(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct gpc_regs_info *regs = (struct gpc_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_req.virt_addr == NULL)
			|| (regs->gpc_intr.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_VEU_PWR_OFF_DONE_VEU_SHIFT)) {
		dev_dbg(dev, "veu top subsys has already power off\n");
		return 0;
	}

	if (gpc_disable(dev, sreg))
		return -EAGAIN;

	return 0;
}

static int vdec_core_power_on(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	int ret = 0;
	struct vdec_core_regs_info *regs = (struct vdec_core_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->m1_dpu_dvfs_bypass.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_VDEC_PWR_ON_DONE_VDEC_SHIFT)) {
		dev_dbg(dev, "vdec core has already power on\n");
		return 0;
	}

	writel(BMRW_MASK(MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_BLOCK_MASK) |
		MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_BLOCK_MASK,
		regs->m1_dpu_dvfs_bypass.virt_addr);

	if (gpc_enable(dev, sreg))
		return -EAGAIN;

	ret = xr_regulator_ffa_direct_message(dev, FID_BL31_IP_REGULATOR_ON,
			sreg->regulator_id, 0, NULL);
	if (ret)
		return -EAGAIN;

	return 0;
}

static int vdec_core_power_off(struct device *dev, struct xring_regulator_ip *sreg)
{
	unsigned int data = 0;
	struct vdec_core_regs_info *regs = (struct vdec_core_regs_info *)sreg->gpc_priv;

	if ((regs->gpc_regs.gpc_req.virt_addr == NULL)
			|| (regs->gpc_regs.gpc_intr.virt_addr == NULL)
			|| (regs->m1_dpu_dvfs_bypass.virt_addr == NULL)) {
		dev_err(dev, "%s not init\n", sreg->name);
		return -EFAULT;
	}

	data = readl(regs->gpc_regs.gpc_intr.virt_addr);
	if (data & RG_BIT(MEDIA1_CRG_INTR_PU_VDEC_PWR_OFF_DONE_VDEC_SHIFT)) {
		dev_dbg(dev, "vdec core has already power off\n");
		return 0;
	}

	if (gpc_disable(dev, sreg))
		return -EAGAIN;

	writel(BMRW_MASK(MEDIA1_CRG_M1_DPU_DVFS_BYPASS_DPU_DVFS_BLOCK_MASK),
		regs->m1_dpu_dvfs_bypass.virt_addr);

	return 0;
}

static void _reg_dump(struct device *dev, u64 phy_addr, u32 size)
{
	void __iomem *virt_addr;
	u32 i = 0;

	if ((size == 0) || (size & 0xf))
		return;

	virt_addr = ioremap(phy_addr, size);
	if (IS_ERR(virt_addr)) {
		dev_err(dev, "failed to map (addr=0x%llx, size=0x%x), err=%ld\n",
				phy_addr, size, PTR_ERR(virt_addr));
		return;
	}

	do {
		dev_err(dev, "0x%08llx: %08x %08x %08x %08x\n",
				phy_addr + i,
				readl(virt_addr + i),
				readl(virt_addr + i + 0x4),
				readl(virt_addr + i + 0x8),
				readl(virt_addr + i + 0xc));
		i += 0x10;
	} while (i < size);

	iounmap(virt_addr);
}

static void gpu_subsys_sec_dump(struct device *dev)
{
	struct arm_smccc_res res = {0};
	u32 sec_loop = 0;
	u32 base = ACPU_GPU_LPCTRL + 0x800;

	for (sec_loop = 0; sec_loop < 2; sec_loop++) {
		arm_smccc_smc(FID_BL31_IP_REGULATOR_ON, GPU_SUBSYS_ID, sec_loop, 0, 0, 0, 0, 0, &res);
		dev_err(dev, "0x%08x: %08lx %08lx %08lx %08lx\n",
				base + sec_loop * 0x10,
				res.a0, res.a1, res.a2, res.a3);
	}
}

static void get_status_from_xctrl_cpu(struct device *dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(FID_BL31_GPU_POWER_GET, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		dev_err(dev, "get gpu power state fail\n");
		return;
	}

	dev_err(dev, ">>>>> %s: gpu_exist=%lu, gpu_subchip_on=%lu, gpu_subsys_on=%lu\n",
			__func__, res.a1, res.a2, res.a3);
}

static void gpu_subsys_debug(struct device *dev, struct xring_regulator_ip *sreg)
{
	u64 phy_addr;
	u32 size;
	int status[RELULATOR_MAX_COUNT];
	int i, len;
	char line[64];

	WARN_ON(1);

	/* Dump lpctrl_gpu gpc registers */
	phy_addr = ACPU_GPU_LPCTRL;
	size = 0x200;
	_reg_dump(dev, phy_addr, size);

	/* Dump lpctrl_gpu gpc sec registers */
	gpu_subsys_sec_dump(dev);

	/* Dump gpu_subchip crg registers */
	phy_addr = ACPU_GPU_CRG + GPU_SUBCHIP_CRG_CLKGT0_W1S;
	size = 0x50;
	_reg_dump(dev, phy_addr, size);

	if (!xr_ip_regulator_get_status(status, RELULATOR_MAX_COUNT)) {
		for (i = 0; (i + 4) <= RELULATOR_MAX_COUNT; i += 4) {
			dev_err(dev, ">>>>> ip_regulator_status[%d..%d]:%d %d %d %d",
					i, i + 3,
					status[i], status[i+1], status[i+2], status[i+3]);
		}

		if (i < RELULATOR_MAX_COUNT) {
			len = snprintf(line, sizeof(line), "[%d..%d] ",
					i, RELULATOR_MAX_COUNT - 1);
			for (; i < RELULATOR_MAX_COUNT; i++)
				len += snprintf(line + len, sizeof(line) - len, "%d ", status[i]);
			dev_err(dev, ">>>>> ip_regulator_status%s", line);
		}
	}

	get_status_from_xctrl_cpu(dev);
}

typedef int (*gpc_func_type)(struct device *dev, struct xring_regulator_ip *sreg);
typedef void (*gpc_func_dbg_type)(struct device *dev, struct xring_regulator_ip *sreg);

const gpc_func_type regulator_gpc_init[RELULATOR_MAX_COUNT] = {
	[GPU_SUBSYS_ID]     = gpu_subsys_power_init,
	[DPU1_ID]           = dpu_part1_power_init,
	[DPU2_ID]           = dpu_part2_power_init,
	[VEU_TOP_SUBSYS_ID] = veu_top_power_init,
	[VDEC_CORE_ID]      = vdec_core_power_init,
};

const gpc_func_type regulator_gpc_enable[RELULATOR_MAX_COUNT] = {
	[GPU_SUBSYS_ID]     = gpu_subsys_power_on,
	[DPU1_ID]           = dpu1_power_on,
	[DPU2_ID]           = dpu2_power_on,
	[VEU_TOP_SUBSYS_ID] = veu_top_subsys_power_on,
	[VDEC_CORE_ID]      = vdec_core_power_on,
};

const gpc_func_type regulator_gpc_disable[RELULATOR_MAX_COUNT] = {
	[GPU_SUBSYS_ID]     = gpu_subsys_power_off,
	[DPU1_ID]           = dpu1_power_off,
	[DPU2_ID]           = dpu2_power_off,
	[VEU_TOP_SUBSYS_ID] = veu_top_subsys_power_off,
	[VDEC_CORE_ID]      = vdec_core_power_off,
};

const gpc_func_dbg_type regulator_gpc_debug[RELULATOR_MAX_COUNT] = {
	[GPU_SUBSYS_ID]     = gpu_subsys_debug,
};

int xr_regulator_gpc_init(struct device *dev, struct xring_regulator_ip *sreg)
{
	if (!sreg || !dev) {
		dev_err(dev, "[%s] is err!\n", __func__);
		return -EINVAL;
	}

	if ((sreg->regulator_id >= RELULATOR_MAX_COUNT) ||
		(!regulator_gpc_init[sreg->regulator_id])) {
		dev_err(dev, "[%s] id is err!\n", __func__);
		return -EINVAL;
	}

	return regulator_gpc_init[sreg->regulator_id](dev, sreg);
}

int xr_regulator_gpc_enable(struct device *dev, struct xring_regulator_ip *sreg)
{
	if (!sreg || !dev) {
		dev_err(dev, "[%s] is err!\n", __func__);
		return -EINVAL;
	}

	if ((sreg->regulator_id >= RELULATOR_MAX_COUNT) ||
		(!regulator_gpc_enable[sreg->regulator_id])) {
		dev_err(dev, "[%s] id is err!\n", __func__);
		return -EINVAL;
	}

	return regulator_gpc_enable[sreg->regulator_id](dev, sreg);
}

int xr_regulator_gpc_disable(struct device *dev, struct xring_regulator_ip *sreg)
{
	if (!sreg || !dev) {
		dev_err(dev, "[%s] is err!\n", __func__);
		return -EINVAL;
	}

	if ((sreg->regulator_id >= RELULATOR_MAX_COUNT) ||
		(!regulator_gpc_disable[sreg->regulator_id])) {
		dev_err(dev, "[%s] id is err!\n", __func__);
		return -EINVAL;
	}

	return regulator_gpc_disable[sreg->regulator_id](dev, sreg);
}

void xr_regulator_gpc_debug(struct device *dev, struct xring_regulator_ip *sreg)
{
	if (!sreg || !dev) {
		dev_err(dev, "[%s] is err!\n", __func__);
		return;
	}

	if ((sreg->regulator_id >= RELULATOR_MAX_COUNT) ||
		(!regulator_gpc_debug[sreg->regulator_id])) {
		dev_err(dev, "[%s] id is err or debug not implemented!\n", __func__);
		return;
	}

	regulator_gpc_debug[sreg->regulator_id](dev, sreg);
}
