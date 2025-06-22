/* SPDX-License-Identifier: GPL-2.0 */
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

#include "dpu_hw_power_ops.h"
#include "dpu_hw_init_module_ops.h"

struct dpu_power_reg_base {
	DPU_IOMEM peri_crg_base;
	DPU_IOMEM media1_lpctrl_ft_base;
	DPU_IOMEM pctrl_base;
	DPU_IOMEM media1_crg_base;
	DPU_IOMEM lpis_actrl_base;
	DPU_IOMEM media1_ctrl_base;
	DPU_IOMEM dpu_glb_base;
	DPU_IOMEM media1_sys_base;
	DPU_IOMEM dsi_sctrl_base;
	DPU_IOMEM hss1_crg_base;
};

static struct dpu_power_reg_base g_reg_base;

static u32 g_m1_bisr;
static bool g_first_power_on = true;

static struct dpu_power_reg_base *get_power_reg_base(void)
{
	REG_MAP(g_reg_base.peri_crg_base, 0xec001000, 0x1000);
	REG_MAP(g_reg_base.media1_lpctrl_ft_base, 0xe7c05000, 0x1000);
	REG_MAP(g_reg_base.pctrl_base, 0xec002000, 0x2000);
	REG_MAP(g_reg_base.media1_crg_base, 0xe7a00000, 0x1000);
	REG_MAP(g_reg_base.lpis_actrl_base, 0xe1508000, 0x1000);
	REG_MAP(g_reg_base.media1_ctrl_base, 0xe7c04000, 0x1000);
	REG_MAP(g_reg_base.dpu_glb_base, 0xE7058000, 0x1000);
	REG_MAP(g_reg_base.hss1_crg_base, 0xE1A83000, 0x1000);
	REG_MAP(g_reg_base.dsi_sctrl_base, 0xE8180000, 0x1000);

	return &g_reg_base;
}

static void put_power_reg_base(struct dpu_power_reg_base *base)
{
	REG_UNMAP(base->hss1_crg_base);
	REG_UNMAP(base->dsi_sctrl_base);
	REG_UNMAP(base->media1_sys_base);
	REG_UNMAP(base->dpu_glb_base);
	REG_UNMAP(base->media1_ctrl_base);
	REG_UNMAP(base->lpis_actrl_base);
	REG_UNMAP(base->media1_crg_base);
	REG_UNMAP(base->pctrl_base);
	REG_UNMAP(base->media1_lpctrl_ft_base);
	REG_UNMAP(base->peri_crg_base);
}

static void set_media1_clk_profile(struct dpu_power_reg_base *base)
{
	/* set media1 to 0.6v profile */
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x000, 0x0fc30d42);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x008, 0x007f0007);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x00c, 0x3f800380);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x010, 0x003f000b);
}

static void media1_subsys_power_up(struct dpu_power_reg_base *base)
{
	u32 value;

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x614, 0x00020002);
	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x840, 0x08000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x060, 0x0000000C);
	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x844, 0x08000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x064, 0x0000000C);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x8D0, 0x40000000);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x840, 0x08000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x060, 0x0000000C);

	DPU_REG_WRITE_BARE(base->lpis_actrl_base + 0x0464, 0x00040000);
	DPU_UDELAY(100);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x020, 0x0C000000);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x020, 0x01800000);
	DPU_UDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x020, 0x10000000);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x020, 0x00400000);

	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x08D0, 0x00010000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0850, 0x01000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0860, 0x10000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0020, 0x02400000);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0870, 0x10000000);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0060, 0x00000003);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0874, 0x10000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0064, 0x00000003);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x0804, 0x00010000);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x08d0, 0x00000040);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0080, 0x40000000);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0870, 0x10000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0060, 0x00000003);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0020, 0x00080018);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x04000000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0100, 0x00140014);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->lpis_actrl_base + 0x04B0, 0x00040000);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x614, 0x00020000);

	if (!g_first_power_on) {
		DPU_INFO("after power off, need restore bisr 0x%x\n", g_m1_bisr);
		value = DPU_REG_READ_BARE(base->media1_ctrl_base + 0x0C04);

		DPU_INFO("curr bisr 0x%x\n", value);
		value = value & ~BIT(0);
		value |= g_m1_bisr & BIT(0);

		DPU_INFO("restore bisr 0x%x\n", value);
		DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0X0C04, value);
	}
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x114, 0x00010000);
}

static void dpu_p0_power_up(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0840, 0x10000000);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0020, 0x00000208);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x01E82101);
	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0844, 0x10000000);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0044, 0x01E82101);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0080, 0x9C7117C0);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0090, 0x00010010);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0840, 0x10000000);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0110, 0xFFFFFFFF);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0020, 0x00000208);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x01E82101);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0128, 0x00010001);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x02000000);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x00220000);
	DPU_MDELAY(1);
}

static void dpu_p1_power_up(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0904,
			DPU_REG_READ_BARE(base->media1_crg_base + 0x0904) | BIT(1));
	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0000, 0x00020002);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0024, 0x6DB00000);
	DPU_UDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0024, 0x12480000);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->dpu_glb_base + 0x0028, 0x00000000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x00000666);
	DPU_UDELAY(5);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0044, 0x00000666);
	DPU_UDELAY(5);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0010, 0x00020000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0080, 0x000CC000);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0090, 0x0000000C);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x00000666);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x01000000);
	DPU_MDELAY(1);
}

static void dpu_p2_power_up(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0934,
			DPU_REG_READ_BARE(base->media1_crg_base + 0x0934) | BIT(1));

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x000, 0x00010001);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0020, 0x00300000);
	DPU_UDELAY(5);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0020, 0x00080000);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x00000808);
	DPU_UDELAY(5);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0044, 0x00000808);
	DPU_UDELAY(5);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0010, 0x00010000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0080, 0x00020000);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0090, 0x00000002);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x0040, 0x00000808);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x00400000);
	DPU_MDELAY(1);
}

static void __maybe_unused media1_subsys_power_down(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0614, 0x00020002);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->lpis_actrl_base + 0x4B0, 0x00040004);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x100, 0x00140000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0, 0x04000400);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x24, 0x00080018);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x064, 0x00000003);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x874, 0x10000000);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x84, 0x40000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x8d4, 0x00000040);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x804, 0x00010001);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x854, 0x01000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x864, 0x10000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x24, 0x02400000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x8D4, 0x00010000);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x10001000);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x00400040);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x24, 0x02080208);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x0C000C00);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x01800180);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x24, 0x0C300C30);

	DPU_REG_WRITE_BARE(base->lpis_actrl_base + 0x464, 0x00040004);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x844, 0x08000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x64, 0x0000000C);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x08D4, 0x40000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0614, 0x00020000);
}

static void dpu_p0_power_down(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0128, 0x00010000);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0000, 0x02000200);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0, 0x00220022);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x844, 0x10000000);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x110, 0xFFFFF0FF);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x24, 0x200);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x44, 0x01E82101);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x84, 0x9C7FFFC0);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x94, 0x000187DE);
}

static void dpu_p1_power_down(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x904,
			DPU_REG_READ_BARE(base->media1_crg_base + 0x0904) | BIT(1));
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0, 0x01000100);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x44, 0x00000666);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x84, 0x000CC000);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x94, 0x0000000C);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x10, 0x00020002);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x24, 0x10401040);
	DPU_MDELAY(1);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x24, 0x61806180);

	DPU_REG_WRITE_BARE(base->dpu_glb_base + 0x0028, 0x000000a0);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x24, 0x0C300410);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0, 0x00020000);
}

static void dpu_p2_power_down(struct dpu_power_reg_base *base)
{
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x934,
			DPU_REG_READ_BARE(base->media1_crg_base + 0x0934) | BIT(1));
	DPU_REG_WRITE_BARE(base->media1_ctrl_base + 0x0, 0x00400040);
	DPU_MDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x44, 0x00000808);
	DPU_UDELAY(1);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x84, 0x00020000);
	DPU_REG_WRITE_BARE(base->media1_crg_base + 0x94, 0x00000002);

	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x10, 0x00010001);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x00080008);
	DPU_MDELAY(10);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x20, 0x00300030);
	DPU_REG_WRITE_BARE(base->media1_lpctrl_ft_base + 0x0, 0x00010000);
}

void dpu_hw_set_media1_clk(void)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	set_media1_clk_profile(base);
	put_power_reg_base(base);
}

void dpu_hw_power_up(struct dpu_hw_init_cfg *cfg)
{
	struct dpu_power_reg_base *base = get_power_reg_base();

	if (!cfg) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	media1_subsys_power_up(base);
	dpu_p0_power_up(base);
	dpu_hw_do_hw_init(DPU_PARTITION_0, cfg);
	dpu_p1_power_up(base);
	dpu_hw_do_hw_init(DPU_PARTITION_1, cfg);
	dpu_p2_power_up(base);
	dpu_hw_do_hw_init(DPU_PARTITION_2, cfg);
	set_media1_clk_profile(base);

	put_power_reg_base(base);
}

void dpu_hw_power_down(void)
{
	struct dpu_power_reg_base *base = get_power_reg_base();

	g_first_power_on = false;
	g_m1_bisr = DPU_REG_READ_BARE(base->media1_ctrl_base + 0x0C04);

	dpu_p2_power_down(base);
	dpu_hw_auto_cg_reset(DPU_PARTITION_2);
	dpu_p1_power_down(base);
	dpu_hw_auto_cg_reset(DPU_PARTITION_1);
	dpu_p0_power_down(base);
	media1_subsys_power_down(base);

	put_power_reg_base(base);
}

void dsi_hw_power_up(void)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	u32 mem_lpstatus_3, mem_lpstatus_4, mrc_ack_1;
#ifdef __KERNEL__
	struct arm_smccc_res res = {0};
#endif

	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1688, 0x00010000);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1690, 0x00010000);

	DPU_UDELAY(100);

	mem_lpstatus_3 = DPU_REG_READ_BARE(base->pctrl_base + 0x168C);

	mem_lpstatus_4 = DPU_REG_READ_BARE(base->pctrl_base + 0x1694);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0100, 0x00002000);

#ifdef __KERNEL__
	arm_smccc_smc(FID_BL31_DPU_PERI_DSI_ON_REG, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0)
		DPU_ERROR("dpu dsi power up failed!\n");
#else

	(VOID)ArmCallSmc3(FID_BL2_DPU_PERI_DSI_ON_REG, NULL, NULL, NULL);
#endif

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0010, 0x04000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0000, 0x00004000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0000, 0x00000800);
	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x0000, 0x02C00000);

	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0014, 0x04000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0004, 0x00004000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0004, 0x00000800);
	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x0004, 0x02C00000);

	DPU_UDELAY(1);

	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1604, 0x00010000);

	DPU_UDELAY(100);

	mrc_ack_1 = DPU_REG_READ_BARE(base->lpis_actrl_base + 0x0714);
	DPU_INFO("MEM_LPSTATUS3:0x%x, MEM_LPSTATUS4:0x%x, MRC_ACK_1:0x%x\n",
			mem_lpstatus_3, mem_lpstatus_4, mrc_ack_1);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0100, 0x00001000);

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0010, 0x04000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0000, 0x00004000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0000, 0x00000800);
	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x0000, 0x02C00000);

	put_power_reg_base(base);
}

void dsi_hw_power_down(void)
{
	u32 value1, value2;

	struct dpu_power_reg_base *base = get_power_reg_base();

	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0014, 0x04000000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0004, 0x00004000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0004, 0x00000800);
	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x0004, 0x02C00000);

	DPU_UDELAY(1);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x104, 0x00001000);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1604, 0x00010001);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x104, 0x00002000);
	DPU_REG_WRITE_BARE(base->peri_crg_base + 0x0010, 0x04000000);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1688, 0x00010001);
	DPU_REG_WRITE_BARE(base->pctrl_base + 0x1690, 0x00010001);

	DPU_UDELAY(100);

	value1 = DPU_REG_READ_BARE(base->pctrl_base + 0x168C);
	value2 = DPU_REG_READ_BARE(base->pctrl_base + 0x1694);

	DPU_INFO("MEM_LPSTATUS3 0x%x, MEM_LPSTATUS4 0x%x\n", value1, value2);

	put_power_reg_base(base);
}

void dpu_dsi_ipi_gt_off(u8 port)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	/* media1_crg: [23]-dsi1, [22]-dsi0 */
	if (port == 0)
		DPU_REG_WRITE_BARE(base->media1_crg_base + 0x44, 0x400000);
	if (port == 1)
		DPU_REG_WRITE_BARE(base->media1_crg_base + 0x44, 0x800000);

	DPU_DEBUG("dpu dsi ipi port(%d) gt off\n", port);

	put_power_reg_base(base);
}

void dpu_dsi_ipi_gt_on(u8 port)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	/* media1_crg: [23]-dsi1, [22]-dsi0 */
	if (port == 0)
		DPU_REG_WRITE_BARE(base->media1_crg_base + 0x40, 0x400000);
	if (port == 1)
		DPU_REG_WRITE_BARE(base->media1_crg_base + 0x40, 0x800000);

	DPU_DEBUG("dpu dsi ipi port(%d) gt on\n", port);

	put_power_reg_base(base);
}

void dsi_clk_dpu_dsi_ipi_gt_off(u8 port)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	/* hss1_crg: [23]-dsi1, [22]-dsi0  */
	if (port == 0)
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x04, 0x400000);
	if (port == 1)
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x04, 0x800000);

	DPU_DEBUG("dsi ipi port(%d) gt off\n", port);

	put_power_reg_base(base);
}

void dsi_clk_dpu_dsi_ipi_gt_on(u8 port)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	/* hss1_crg: [23]-dsi1, [22]-dsi0 */
	if (port == 0)
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x00, 0x400000);
	if (port == 1)
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x00, 0x800000);

	DPU_DEBUG("dsi ipi port(%d) gt on\n", port);

	put_power_reg_base(base);
}

void dsi_hw_set_main_ipi_mux(enum dsi_ipi_clk_mode mode)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	u32 value, mask;

	value = DPU_REG_READ_BARE(base->hss1_crg_base + 0x80);

	if (mode == DSI_IPI_MAIN_PORT0) {
		mask = 0x1 <<16;
		value = 0x0 << 0;
		value |= mask;
	} else if (mode == DSI_IPI_MAIN_PORT1) {
		mask = 0x6 <<16;
		value = 0x1 << 1;
		value |= mask;
	} else if (mode == DSI_IPI_MAIN_DUAL_PORT) {
		mask = 0x7 <<16;
		value = (0x0 << 1) | (0x0 << 0);
		value |= mask;
	} else {
		value = 0;
		DPU_DEBUG("Unsupported mode(main ipi): %d\n", mode);
	}

	DPU_DEBUG("Set IPI main clock, mode: %d\n", mode);

	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x80, value);

	put_power_reg_base(base);
}

void dsi_hw_set_backup_ipi_gt_div(u8 div, enum dsi_ipi_clk_mode mode)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	u32 clk5_mask = 0;
	u32 clk5_val = 0;
	u32 clk6_mask = 0;
	u32 clk6_val = 0;

	/**
	 * CLKDIV5(0x54): [13] - clk_dpu_dsi_ipi0 div pre gt
	 *                [11:6] - div_clk_dpu_dsi_ipi0, div ratio
	 * CLKDIV6(0x58): [12] - clk_dpu_dsi_ipi1 div pre gt
	 *                [5:0] - div_clk_dpu_dsi_ipi0, div ratio
	 */

	DPU_DEBUG("clk_dpu_dsi_ipi0/1 gt and div set, mode:%d, div:%d\n",
		mode, div);

	if (mode == DSI_IPI_BACKUP_PORT0) {
		clk5_mask = ((0x1 << 13) | (0x3F << 6)) << 16;
		clk5_val = (0x1 << 13) | (div << 6);
		clk5_val |= clk5_mask;
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x54, clk5_val);
	} else if (mode == DSI_IPI_BACKUP_PORT1) {
		clk6_mask = ((0x1 << 12) | (0x3F << 0)) << 16;
		clk6_val = (0x1 << 12) | (div << 0);
		clk6_val |= clk6_mask;
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x58, clk6_val);

	} else if (mode == DSI_IPI_BACKUP_DUAL_PORT) {
		clk5_mask = ((0x1 << 13) | (0x3F << 6)) << 16;
		clk5_val = (0x1 << 13) | (div << 6);
		clk5_val |= clk5_mask;
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x54, clk5_val);

		clk6_mask = ((0x1 << 12) | (0x3F << 0)) << 16;
		clk6_val = (0x1 << 12) | (div << 0);
		clk6_val |= clk6_mask;
		DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x58, clk6_val);
	} else {
		DPU_DEBUG("Backup ipi does not support this mode\n");
	}

	DPU_DEBUG("backup ipi div: CLKDIV5 0x%x 0x%x, CLKDIV6 0x%x 0x%x\n",
		clk5_val, clk5_mask, clk6_val, clk6_mask);

	put_power_reg_base(base);
}

static void __maybe_unused dsi_hw_set_backup_ipi1_dp(u8 div)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	u32 value, mask;

	mask = (0x2FC0 << 16);
	value = (0x1 << 13) | (div << 6);
	value |= mask;
	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x58, value);

	put_power_reg_base(base);
}

void dsi_set_ipi_mux_backup(enum dsi_ipi_clk_mode mode)
{
	struct dpu_power_reg_base *base = get_power_reg_base();
	u32 value = 0;
	u32 mask = 0;

	if (mode == DSI_IPI_BACKUP_PORT0) {
		mask = 0x1 <<16;
		value = 0x1;
		value |= mask;
	} else if (mode == DSI_IPI_BACKUP_PORT1) {
		mask = 0x6 <<16;
		value = 0x2 << 1;
		value |= mask;
	} else if (mode == DSI_IPI_BACKUP_DUAL_PORT) {
		mask = 0x7 <<16;
		value = (0x2 << 1) | (0x1 << 0);
		value |= mask;
	} else {
		DPU_DEBUG("Backup ipi does not support this mode: %d\n", mode);
	}

	DPU_DEBUG("backup ipi select: mode %d, PERICTRL0 0x%x 0x%x\n",
		mode, value, mask);

	DPU_REG_WRITE_BARE(base->hss1_crg_base + 0x80, value);

	put_power_reg_base(base);
}

void dpu_dsi_clk_ctrl(u32 dsi_idx, bool enable)
{
	if (enable)
		dpu_dsi_ipi_gt_on(dsi_idx);
	else
		dpu_dsi_ipi_gt_off(dsi_idx);
}
