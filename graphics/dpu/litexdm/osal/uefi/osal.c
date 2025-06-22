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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <ApbGpio.h>
#include <Library/BaseMemoryLib.h>

#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePathUtilities.h>

#include <Protocol/FdtProtocol.h>
#include <Library/UpdateDeviceTree.h>
#include <libfdt.h>

#include "XRRegulator.h"
#include "Clk.h"
#include "dpu_log.h"
#include "dpu_hw_init_module_ops.h"
#include "dpu_hw_tpc_module_ops.h"
#include "dpu_hw_power_ops.h"
#include "dpu_common_info.h"
#include "litexdm.h"
#include "Protocol/I2cProtocol.h"
#include <ApbGpio.h>
#include "Protocol/Ioc.h"
#include <Library/UefiBootServicesTableLib.h>
#include "PeriMediaDvfs.h"
#include "mipi_dsi_dev.h"

static bool dts_debug_en;
STATIC BOOLEAN PowerProtocolInited = FALSE;
STATIC EFI_XR_REGULATOR_PROTOCOL *RegulatorProtocol = NULL;
STATIC EFI_XR_CLK_PROTOCOL *ClkProtocol = NULL;
struct Regulator *AppRegulator[] = {
	[DPU0_ID] = NULL,
	[DPU1_ID] = NULL,
	[DPU2_ID] = NULL,
};

enum mpmic_hldo {
	MPMIC_MLDO7 = 27,
	MPMIC_MLDO9 = 37,
};
struct Regulator *PanelRegulator[] = {
	[MPMIC_MLDO7] = NULL,
	[MPMIC_MLDO9] = NULL,
};

#define dpu_dts_debug(msg, ...) \
	do { \
		if (dts_debug_en) \
			dpu_pr_debug(msg, ##__VA_ARGS__);\
	} while (0)

void gpio_set_direction_output_value(GPIO_PINS gpio, int32_t value, uint32_t delay_ms)
{
	EFI_XR_APB_GPIO_PROTOCOL *GpioProtocol;

	if (gpio == 0)
		return;

	EFI_STATUS Status = gBS->LocateProtocol (
				&gEfiApbGpioProtocolGuid,
				NULL,
				(VOID **)&GpioProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiApbGpioProtocolGuid error\n");
		return;
	}

	GpioProtocol->GpioSetDirection(gpio, OUTPUT);
	GpioProtocol->GpioSetValue(gpio, value);
	dpu_pr_debug("Set GPIO %d Output value %d\n", gpio, value);

	dpu_mdelay(delay_ms);
}

void gpio_get_direction_input_value(GPIO_PINS gpio, int32_t *value)
{
	EFI_XR_APB_GPIO_PROTOCOL *GpioProtocol;

	EFI_STATUS Status = gBS->LocateProtocol (
				&gEfiApbGpioProtocolGuid,
				NULL,
				(VOID **)&GpioProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiApbGpioProtocolGuid error\n");
		return;
	}

	GpioProtocol->GpioSetDirection(gpio, INPUT);
	dpu_udelay(2000);
	GpioProtocol->GpioGetValue(gpio, (GPIO_PIN_VALUE *)value);
	dpu_pr_debug("Get GPIO %d input value: %d\n", gpio, *value);
}

void gpio_set_direction_input_value(GPIO_PINS gpio, int32_t value)
{
	EFI_XR_APB_GPIO_PROTOCOL *GpioProtocol;

	EFI_STATUS Status = gBS->LocateProtocol (
				&gEfiApbGpioProtocolGuid,
				NULL,
				(VOID **)&GpioProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiApbGpioProtocolGuid error\n");
		return;
	}

	GpioProtocol->GpioSetDirection(gpio, INPUT);
	dpu_udelay(2000);
	GpioProtocol->GpioSetValue(gpio, value);

	dpu_pr_info("Get GPIO %d input value: %d\n", gpio, value);
}

int ldo_set_direction_output_value(uint32_t ldo_num, enum IO_OPS ops, uint32_t delay_ms)
{
	EFI_STATUS Status;
	static EFI_XR_REGULATOR_PROTOCOL *RegulatorProtocol = NULL;
	int Ret;

	Status = gBS->LocateProtocol(&gEfiXRRegulatorProtocolGuid, NULL,
			(void **)&RegulatorProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Failed to open gEfiXRRegulatorProtocolGuid\n");
		return -1;
	}

	if (!PanelRegulator[ldo_num]) {
		PanelRegulator[ldo_num] = RegulatorProtocol->XRRegulatorGet(ldo_num,
				REGULATOR_TYPE_PMIC);
		if (!PanelRegulator[ldo_num]) {
			dpu_pr_err("Failed to get regulator, ldo-%d\n", ldo_num);
			return -1;
		}
	}

	if (ops == PULL_DOWN) {
		Status = RegulatorProtocol->XRRegulatorDisable(PanelRegulator[ldo_num]);
		Ret = RegulatorProtocol->XRRegulatorIsEnabled(PanelRegulator[ldo_num]);
		dpu_pr_info("XRRegulatorIsEnabled ret %d\n", Ret);
		RegulatorProtocol->XRRegulatorPut(PanelRegulator[ldo_num]);
		PanelRegulator[ldo_num] = NULL;
	} else {
		Status = RegulatorProtocol->XRRegulatorEnable(PanelRegulator[ldo_num]);
	}
	if (EFI_ERROR(Status)) {
		dpu_pr_err("%a regulator error, ldo-%d\n", ops ? "enable" : "disable", ldo_num);
		goto error;
	}

	dpu_mdelay(delay_ms);
	dpu_pr_info("set LDO %d value %d\n", ldo_num, ops);
	return 0;

error:
	if (RegulatorProtocol->XRRegulatorIsEnabled(PanelRegulator[ldo_num])) {
		dpu_pr_err("Power Get failed(id=%d), Status:%d\n", ldo_num, Status);
		RegulatorProtocol->XRRegulatorPut(PanelRegulator[ldo_num]);
	}

	PanelRegulator[ldo_num] = NULL;
	return -1;
}

static uint64_t dpu_power_protocol_get()
{
	EFI_STATUS Status;

	if (PowerProtocolInited)
		return 0;

	Status = gBS->LocateProtocol (
				&gEfiXRRegulatorProtocolGuid,
				NULL,
				(VOID **)&RegulatorProtocol
				);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiXRRegulatorProtocolGuid error");
		return EFI_ABORTED;
	}

	Status = gBS->LocateProtocol(&gEfiXRClkProtocolGuid, NULL,
				(VOID **)&ClkProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiXRClkProtocolGuid failed!\n");
		return -EFI_ABORTED;
	}

	PowerProtocolInited = TRUE;

	dpu_pr_debug("-\n");
	return EFI_SUCCESS;
}

static uint64_t dpu_clk_set_by_id(int clk_id, int rate_level)
{
	EFI_STATUS Status;
	UINT64 Rate;

	if (!ClkProtocol) {
		dpu_pr_err("Open protocol gEfiXRClkProtocolGuid error!\n");
		return -EFI_UNSUPPORTED;
	}

	Status = ClkProtocol->ClkSetRate(clk_id, rate_level);
	if (EFI_ERROR(Status))
		dpu_pr_err("Set Clk[%d] to level %d failed!\n", clk_id, rate_level);

	Rate = ClkProtocol->ClkGetRate(clk_id);
	dpu_pr_debug("Clk[%d] Rate:%lu!\n", clk_id, Rate);

	dpu_pr_debug("-\n");
	return EFI_SUCCESS;
}

static uint64_t dpu_pll_clk_set_by_id(int clk_id, uint64_t rate)
{
	EFI_STATUS Status;
	UINT64 Rate;

	if (!ClkProtocol) {
		dpu_pr_err("Open protocol gEfiXRClkProtocolGuid error!\n");
		return -EFI_UNSUPPORTED;
	}

	Status = ClkProtocol->ClkSetPllRate(clk_id, rate);
	if (EFI_ERROR(Status))
		dpu_pr_err("Set Clk[%d] to level %d failed!\n", clk_id, rate);

	Rate = ClkProtocol->ClkGetRate(clk_id);
	dpu_pr_debug("Clk[%d] Rate:%lu!\n", clk_id, Rate);

	return EFI_SUCCESS;
}

static uint64_t dpu_clk_ctrl_by_id(bool enable, int clk_id)
{
	EFI_STATUS Status;

	if (enable) {
		Status = ClkProtocol->ClkEnable(clk_id);
	} else {
		Status = ClkProtocol->ClkDisable(clk_id);
	}

	if (EFI_ERROR(Status)) {
		dpu_pr_err("Clk[%d] %a failed!\n",
			clk_id, enable ? "enable" : "disable");
	} else {
		dpu_pr_debug("Clk[%d] %a success!\n",
			clk_id, enable ? "enable" : "disable");
	}

	return EFI_SUCCESS;
}

static uint64_t dpu_power_up_by_id(int regulator_id)
{
	EFI_STATUS Status;

	AppRegulator[regulator_id] = RegulatorProtocol->XRRegulatorGet(regulator_id,
					REGULATOR_TYPE_IP);
	if (!AppRegulator[regulator_id]) {
		dpu_pr_err("Protocol XRRegulatorGet error");
		return EFI_ABORTED;
	}

	Status = RegulatorProtocol->XRRegulatorEnable(AppRegulator[regulator_id]);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("%d power on error\n",regulator_id);
		goto error;
	}

	dpu_pr_debug("-\n");
	return EFI_SUCCESS;
error:
	if (RegulatorProtocol->XRRegulatorIsEnabled(AppRegulator[regulator_id]))
		RegulatorProtocol->XRRegulatorPut(AppRegulator[regulator_id]);

	AppRegulator[regulator_id] = NULL;
	return EFI_ABORTED;
}

static uint64_t dpu_power_down_by_id(int regulator_id)
{
	dpu_pr_info("regulator_id = %d +\n", regulator_id);

	if (!AppRegulator[regulator_id]) {
		dpu_pr_err("Protocol XRRegulatorGet error");
		return EFI_ABORTED;
	}

	if (RegulatorProtocol->XRRegulatorDisable(AppRegulator[regulator_id])) {
		dpu_pr_err("regulator_id %d power down error\n", regulator_id);
		return EFI_ABORTED;
	}

	RegulatorProtocol->XRRegulatorPut(AppRegulator[regulator_id]);

	AppRegulator[regulator_id] = NULL;

	dpu_pr_info("-\n");
	return EFI_SUCCESS;
}

static void dpu_partition_clk_ctrl(uint32_t part_id, bool enable)
{
	dpu_pr_debug("+\n");
	switch (part_id) {
	case DPU_PARTITION_0:
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_CORE0))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_AXI0))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DISP_BUS_DATA))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DISP_BUS_CFG))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, PCLK_DPU_CFG))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, PCLK_DSI_CFG))
			goto error;
		break;
	case DPU_PARTITION_1:
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_CORE1))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_CORE2))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_AXI1))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_AXI2))
			goto error;
		break;
	case DPU_PARTITION_2:
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_CORE3))
			goto error;
		if (dpu_clk_ctrl_by_id(enable, CLK_DPU_AXI3))
			goto error;
		break;
	}

	dpu_pr_debug("-\n");
	return;
error:
	dpu_pr_err("p%d clk %a failed\n", part_id, enable ? "enable" : "disable");
	return;
}

static int32_t dpu_dsc_clk_ctrl(bool enable)
{
	if (dpu_clk_ctrl_by_id(enable, CLK_DPU_DSC0)) {
		dpu_pr_err("dsc clk %a failed\n", enable ? "enable" : "disable");
		return -1;
	}

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_dsc_clk_set(struct dpu_power_mgr *mgr)
{
	int profile_id = mgr->profile_id;

	dpu_pr_debug("+\n");

	if (dpu_clk_set_by_id(CLK_DPU_DSC0, profile_id)) {
		dpu_pr_err("dsc0 clk set to profile%d failed\n", profile_id);
		return -1;
	}

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_clk_set(uint32_t profile_id)
{
	dpu_pr_debug("+\n");
	if (dpu_clk_set_by_id(CLK_DPU_CORE0, profile_id)) {
		dpu_pr_err("core0 clk set to profile%d failed\n", profile_id);
		return -1;
	}

	dpu_pr_debug("-\n");
	return 0;
}

static void dpu_do_hw_init(struct dpu_power_mgr *mgr, uint32_t part_id)
{
	struct dpu_hw_init_cfg cfg;
	uint32_t lowpower_ctrl;

	if (!mgr || !mgr->pinfo) {
        	dpu_pr_err("invalid parameters\n");
		return;
        }
	lowpower_ctrl = mgr->pinfo->lowpower_ctrl;
	cfg.auto_cg_cfg = DISABLE_AUTO_CG;
	cfg.sram_lp_cfg = DISABLE_SRAM_LP;

	if (lowpower_ctrl & DPU_LP_AUTO_CG_ENABLE)
		cfg.auto_cg_cfg = ENABLE_AUTO_CG;

	if ((lowpower_ctrl & DPU_LP_AUTO_CG_ENABLE) &&
			(lowpower_ctrl & DPU_LP_SRAM_LP_ENABLE))
		cfg.sram_lp_cfg = ENABLE_SRAM_LP;

	dpu_hw_do_hw_init(part_id, &cfg);

	/* disable auto cg for tpc config */
	dpu_hw_auto_cg_disable(part_id);
	dpu_hw_tpc_module_cfg(part_id);

	/* recover auto cg status */
	if (lowpower_ctrl & DPU_LP_AUTO_CG_ENABLE)
		dpu_hw_auto_cg_enable(part_id);

	dpu_pr_debug("-\n");
}

static int dpu_volt_set(uint32_t volt_level)
{
	EFI_PERI_MEDIA_DVFS_PROTOCOL *PMDProtocol = NULL;
	EFI_STATUS Status;

	Status = gBS->LocateProtocol(&gEfiPeriMediaDvfsProtocolGuid, NULL,
				(VOID **)&PMDProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Open protocol gEfiPeriMediaDvfsProtocolGuid failed!\n");
		return -1;
	}

	Status = PMDProtocol->DPUVoltSet(volt_level);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("dpu volt set [%d] failed!\n", volt_level);
		return -1;
	}
	dpu_pr_info("dpu volt set [%d]!\n", volt_level);

	/* media freq init to level 0 */
	Status = PMDProtocol->MediaFreqSet(PMD_DPU_VOTE, PMD_PROFILE_LEVEL_0);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("Media Dvfs[%d] set [%d] failed!\n", PMD_DPU_VOTE, PMD_PROFILE_LEVEL_0);
		return -1;
	}
	dpu_pr_info("Media Dvfs[%d] set [%d]!\n",
			PMD_DPU_VOTE, PMD_PROFILE_LEVEL_0);

	return 0;
}

static int32_t dpu_p0_power_up(struct dpu_power_mgr *mgr)
{
	dpu_pr_debug("+\n");

	if (dpu_power_up_by_id(MEDIA1_SUBSYS_ID)) {
		dpu_pr_err("Media1 power on failed\n");
		return -1;
	}

	/* if wants to vote other level, refer to enum UEFI_PERI_MEDIA_DVFS_LEVEL */
	if (dpu_volt_set(mgr->volt_level)) {
		dpu_pr_err("media bus clk set failed\n");
		return -1;
	}

	dpu_partition_clk_ctrl(DPU_PARTITION_0, true);

	if (dpu_power_up_by_id(DPU0_ID)) {
		dpu_pr_err("dpu0 power on failed\n");
		return -1;
	}

	if (dpu_clk_set(mgr->profile_id)) {
		dpu_pr_err("dpu clk set failed\n");
		return -1;
	}

	dpu_do_hw_init(mgr, DPU_PARTITION_0);

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_p1_power_up(struct dpu_power_mgr *mgr)
{
	dpu_pr_debug("+\n");

	dpu_partition_clk_ctrl(DPU_PARTITION_1, true);

	if (mgr->pinfo && mgr->pinfo->dsc_cfg.dsc_en) {
		dpu_dsc_clk_ctrl(true);
		dpu_dsc_clk_set(mgr);
	}

	if (dpu_power_up_by_id(DPU1_ID)) {
		dpu_pr_err("dpu part1 power on failed\n");
		return -1;
	}

	dpu_do_hw_init(mgr, DPU_PARTITION_1);

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_p2_power_up(struct dpu_power_mgr *mgr)
{
	dpu_pr_debug("+\n");

	dpu_partition_clk_ctrl(DPU_PARTITION_2, true);

	if (dpu_power_up_by_id(DPU2_ID)) {
		dpu_pr_err("dpu part2 power on failed\n");
		return -1;
	}

	dpu_do_hw_init(mgr, DPU_PARTITION_2);

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_p0_power_down(void)
{
	dpu_pr_debug("+\n");

	if (dpu_power_down_by_id(DPU0_ID)) {
		dpu_pr_err("dpu part0 power off failed\n");
		return -1;
	}

	dpu_partition_clk_ctrl(DPU_PARTITION_0, false);

	if (dpu_power_down_by_id(MEDIA1_SUBSYS_ID)) {
		dpu_pr_err("Media1 power off failed\n");
		return -1;
	}

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_p1_power_down(struct dpu_power_mgr *mgr)
{
	dpu_pr_debug("+\n");

	if (dpu_power_down_by_id(DPU1_ID)) {
		dpu_pr_err("dpu part1 power off failed\n");
		return -1;
	}

	dpu_partition_clk_ctrl(DPU_PARTITION_1, false);
	if (mgr->pinfo && mgr->pinfo->dsc_cfg.dsc_en) {
		dpu_dsc_clk_ctrl(false);
	}

	dpu_hw_auto_cg_reset(DPU_PARTITION_1);

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t dpu_p2_power_down(void)
{
	dpu_pr_debug("+\n");

	if (dpu_power_down_by_id(DPU2_ID)) {
		dpu_pr_err("dpu part2 power off failed\n");
		return -1;
	}

	dpu_partition_clk_ctrl(DPU_PARTITION_2, false);

	dpu_hw_auto_cg_reset(DPU_PARTITION_2);

	dpu_pr_debug("-\n");
	return 0;
}

int32_t dpu_power_on(struct dpu_power_mgr *mgr)
{
	int32_t ret;
	dpu_pr_debug("+\n");

	dpu_check_and_return(!mgr, -1, "pwr_mgr is null\n");

	ret = dpu_power_protocol_get();
	dpu_check_and_return(ret != 0, -1, "get power protocol failed\n");

	ret = dpu_p0_power_up(mgr);
	dpu_check_and_return(ret != 0, -1, "dpu p0 power on failed\n");

	ret = dpu_p1_power_up(mgr);
	dpu_check_and_return(ret != 0, -1, "dpu p1 power on failed\n");

	ret = dpu_p2_power_up(mgr);
	dpu_check_and_return(ret != 0, -1, "dpu p2 power on failed\n");

	dpu_pr_debug("-\n");
	return ret;
}

int32_t dpu_power_off(struct dpu_power_mgr *mgr)
{
	int32_t ret;
	dpu_pr_info("+\n");

	dpu_check_and_return(!mgr, -1, "pwr_mgr is null\n");

	ret = dpu_p2_power_down();
	dpu_check_and_return(ret != 0, -1, "dpu p2 power off failed\n");

	ret = dpu_p1_power_down(mgr);
	dpu_check_and_return(ret != 0, -1, "dpu p1 power off failed\n");

	ret = dpu_p0_power_down();
	dpu_check_and_return(ret != 0, -1, "dpu p0 power off failed\n");

	dpu_pr_info("-\n");
	return ret;
}

static int32_t dpu_dsi_sys_clk_ctrl(struct dpu_power_mgr *mgr, bool enable)
{
	if (dpu_clk_ctrl_by_id(enable, CLK_DSI_SYS)) {
		dpu_pr_err("dsi clk %a failed\n", enable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

static int32_t dpu_dsi_sys_clk_set(struct dpu_power_mgr *mgr)
{
	if (dpu_clk_set_by_id(CLK_DSI_SYS, RATE_LEVEL_PROFILE_MID_HIGH)) {
		dpu_pr_err("dsi sys clk set failed\n");
		return -1;
	}
	return 0;
}

static void dsi_ipi_clk_mux_cfg(struct dpu_panel_info *pinfo)
{
	enum dsi_ipi_clk_mode mode;
	u32 div;
	u64 out_rate;

	mode = dsi_ipi_clk_mode_get(pinfo);
	dpu_pr_info("ipi clk mode:%u\n", mode);

	if (pinfo->ipi_pll_sel == DSI_PIXEL_CLOCK_PHY_PLL) {
		dsi_hw_set_main_ipi_mux(mode);
	} else {
		dsi_clk_div_cal(pinfo->pixel_clk_rate, &div, &out_rate);
		dpu_pr_info("dpu pll: rate:%llu div:%u\n", out_rate, div);
		if (dpu_pll_clk_set_by_id(CLK_DPU_PLL, out_rate)) {
			dpu_pr_err("dpu pll clk set failed\n");
			return;
		}

		if (dpu_clk_ctrl_by_id(true, CLK_DPU_PLL)) {
			dpu_pr_err("dsi ipi clk enable failed\n");
			return;
		}
		dsi_hw_set_backup_ipi_gt_div(div, mode);
		dsi_set_ipi_mux_backup(mode);
	}
}

int32_t dsi_power_on(struct dpu_power_mgr *mgr)
{
	int ret;

	dpu_pr_debug("+\n");

	dpu_check_and_return(!mgr || !mgr->pinfo, -1,
		"pwr_mgr or mgr->pinfo is null\n");

	ret = dpu_power_protocol_get();
	dpu_check_and_return(ret != 0, -1, "get power protocol failed\n");

	ret = dpu_dsi_sys_clk_ctrl(mgr, true);
	dpu_check_and_return(ret != 0, -1, "dpu_dsi_sys_clk_ctrl failed\n");

	dsi_hw_power_up();

	ret = dpu_dsi_sys_clk_set(mgr);
	dpu_check_and_return(ret != 0, -1, "dpu_dsi_sys_clk_set failed\n");

	if (mgr->pinfo->dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_off(0);
		dsi_clk_dpu_dsi_ipi_gt_off(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_off(mgr->pinfo->connector_id);
	}

	dsi_ipi_clk_mux_cfg(mgr->pinfo);

	if (mgr->pinfo->dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_on(0);
		dsi_clk_dpu_dsi_ipi_gt_on(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_on(mgr->pinfo->connector_id);
	}

	dpu_pr_debug("-\n");
	return 0;
}

int32_t dsi_power_off(struct dpu_power_mgr *mgr)
{
	int ret;

	dpu_pr_debug("+\n");

	dpu_check_and_return(!mgr || !mgr->pinfo, -1,
		"pwr_mgr or mgr->pinfo is null\n");

	ret = dpu_dsi_sys_clk_ctrl(mgr, false);
	dpu_check_and_return(ret != 0, -1, "dpu_dsi_sys_clk_ctrl failed\n");

	if (mgr->pinfo->dual_port) {
		dsi_clk_dpu_dsi_ipi_gt_off(0);
		dsi_clk_dpu_dsi_ipi_gt_off(1);
	} else {
		dsi_clk_dpu_dsi_ipi_gt_off(mgr->pinfo->connector_id);
	}

	dsi_hw_power_down();

	dpu_pr_debug("-\n");
	return 0;
}


void dpu_mem_cpy(void *dst, const void *src, uint32_t size)
{
	CopyMem(dst, src, size);
}

void dpu_mem_set(void *buf,  uint32_t value, uint32_t size)
{
	SetMem(buf, size, value);
}

void *dpu_mem_alloc(uint32_t size)
{
	return AllocateZeroPool(size);
}

void *dpu_mem_realloc(uint32_t old_size, uint32_t new_size, void *old_ptr)
{
	return ReallocatePool(old_size, new_size, old_ptr);
}

void dpu_mem_free(void *ptr)
{
	if (ptr)
		FreePool(ptr);
}

void dpu_write_reg(uint64_t addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
	dpu_pr_debug("[W] addr:0x%x value:0x%x\n", addr, value);
}

void dpu_udelay(uint32_t us)
{
	gBS->Stall(us);
}

void dpu_mdelay(uint32_t ms)
{
	dpu_udelay(1000 * ms);
}

void dpu_str_cpy(char* dest, const char* src, uint32_t max_len)
{
	AsciiStrCpyS(dest, max_len, src);
}

int32_t dpu_str_cmp(const char *str1, const char *str2)
{
	return AsciiStrCmp(str1, str2);
}

int32_t dpu_get_subnode_offset(void *fdt, int parent_offset, const char *name)
{
	int offset;

	if (!fdt | !name) {
		dpu_pr_err("can not get fdt address\n");
		return -1;
	}

	offset = fdt_subnode_offset(fdt, parent_offset, name);
	return offset;
}

int32_t dpu_get_fdt_offset(void *fdt, const char *path)
{
	int32_t offset;

	if (!fdt | !path) {
		dpu_pr_err("can not get fdt address\n");
		return -1;
	}

	offset = fdt_path_offset(fdt, path);

	return offset;
}

void *dpu_get_fdt(void)
{
	STATIC EFI_FDT_PROTOCOL *fdt_protocol;
	uint32_t ret = 0;

	ret = gBS->LocateProtocol(&gEfiFdtDxeProtocolGuid, NULL,
			(VOID **)&fdt_protocol);
	if (EFI_ERROR(ret) || fdt_protocol == NULL) {
		dpu_pr_err("can not open Fdt protocal\n");
		return NULL;
	}

	return fdt_protocol->GetDtbAddr();
}

int32_t dpu_str_n_cmp(const char *str1, const char *str2, uint32_t length)
{
	return AsciiStrnCmp(str1, str2, length);
}

bool dpu_check_existence(void)
{
	const char *prop_val;
	int32_t prop_len;
	int32_t offset;

	offset = dpu_get_fdt_offset(dpu_get_fdt(), DPU_DTS_PATH);
	if (offset < 0) {
		dpu_pr_err("get dpu support fdt offset failed!\n");
		return false;
	}

	prop_val = fdt_getprop(dpu_get_fdt(), offset, "status", &prop_len);
	if (prop_val != NULL) {
		return (dpu_str_n_cmp(prop_val, "okay", strlen(prop_val)) == 0);
	}

	return false;
}

int32_t ilog2(int64_t gray)
{
	dpu_pr_info("nothing to do\n");

	return 0;
}

int dpu_dts_parse_u8(void *fdt, int32_t offset, const char *name, uint8_t *out_data)
{
	int32_t prop_len;
	const uint8_t *val;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	*out_data = *val;
	dpu_dts_debug("prop:%a value:%u\n", name, *out_data);
	return 0;
}

int dpu_dts_get_u8_array_size(void *fdt, int32_t offset, const char *name)
{
	int32_t prop_len;
	const uint8_t *val;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	return prop_len / sizeof(uint8_t);
}

int dpu_dts_parse_u8_array(void *fdt, int32_t offset, const char *name, uint8_t *out_data)
{
	int32_t prop_len;
	const uint8_t *val;
	int i;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	uint32_t count = prop_len / sizeof(uint8_t);
	dpu_dts_debug("prop:%a count:%u\n", name, count);
	for (i = 0; i < count; i++) {
		out_data[i] = ((uint8_t *)val)[i];
		dpu_dts_debug("prop:%a idx:%u value:%u\n", name, i, out_data[i]);
	}

	return 0;
}

int dpu_dts_parse_u32(void *fdt, int32_t offset, const char *name, uint32_t *out_data)
{
	int32_t prop_len;
	const fdt32_t *val;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	*out_data = fdt32_to_cpu (*val);
	dpu_dts_debug("prop:%a value:%u\n", name, *out_data);
	return 0;
}

int dpu_dts_get_u32_array_size(void *fdt, int32_t offset, const char *name)
{
	int32_t prop_len;
	const fdt32_t *val;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	return prop_len / sizeof(uint32_t);
}

int dpu_dts_get_string_array_size(void *fdt, int32_t offset, const char *name)
{
	int32_t prop_len;
	const void *prop;
	const char *p;
	int string_count = 0;

	prop = fdt_getprop(fdt, offset, name, &prop_len);
	if ((prop == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	p = prop;
	while (prop_len > 0) {
		const char *next_string = memchr(p, '\0', prop_len);
		if (!next_string) {
			dpu_pr_err("Error: Malformed string property '%a'.\n", name);
			return -1;
		}

		string_count++;
		prop_len -= (next_string - p) + 1;
		p = next_string + 1;
	}

	return string_count;
}

int dpu_dts_parse_string_array(void *fdt, int32_t offset,
		const char *name, const char **out_strs, size_t size)
{
	int32_t prop_len;
	const void *prop;
	const char *p, *end;
	int i = 0, l = 0;

	if (!out_strs) {
		dpu_pr_err("get invalid parameter\n");
		return -1;
	}
	prop = fdt_getprop(fdt, offset, name, &prop_len);
	if ((prop == NULL) || (prop_len < 0)) {
		dpu_pr_err("fdt_getprop failed\n");
		return -1;
	}

	p = prop;
	end = p + prop_len;
	for (i = 0; p < end && i < size; i++, p += l) {
		const char *next_string = memchr(p, '\0', prop_len);
		if (!next_string) {
			dpu_pr_err("Error: Malformed string property '%a'.\n", name);
			return -1;
		}

		l = (next_string - p) + 1;
		*out_strs++ = p;
	}

	return i <= 0 ? -1 : i;
}

int dpu_dts_parse_u32_array(void *fdt, int32_t offset, const char *name, uint32_t *out_data)
{
	int32_t prop_len;
	const fdt32_t *val;
	int i;

	val = fdt_getprop(fdt, offset, name, &prop_len);
	if ((val == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	uint32_t count = prop_len / sizeof(uint32_t);
	for (i = 0; i < count; i++) {
		out_data[i] = fdt32_to_cpu (((uint32_t *)val)[i]);
		dpu_dts_debug("prop:%a idx:%u value:%u\n", name, i, out_data[i]);
	}

	return 0;
}

int dpu_dts_parse_string(void *fdt, int32_t offset, const char *name, const char **out_string)
{
	int32_t prop_len;
	const char *str;

	str = fdt_getprop(fdt, offset, name, &prop_len);
	if ((str == NULL) || (prop_len < 0)) {
		dpu_pr_debug("fdt_getprop parse %a failed\n", name);
		return -1;
	}

	*out_string = str;
	dpu_dts_debug("prop:%a value:%a\n", name, *out_string);
	return 0;
}

bool dpu_dts_read_bool(void *fdt, int32_t offset, const char *name)
{
	int32_t prop_len;
	const char *prop;

	prop = fdt_getprop(fdt, offset, name, &prop_len);
	return prop ? true : false;
}

int dpu_dts_get_offset_by_phandle(void *fdt, uint32_t phandle)
{
	int offset;

	offset = fdt_node_offset_by_phandle(fdt, phandle);
	if (offset <= 0) {
		dpu_pr_debug("get offset failed\n");
		return -1;
	}
	return offset;
}

void dpu_dts_update_string_prop(void *fdt, int32_t offset, const char *prop_name, const char *new_value)
{
	struct fdt_property *blob;
	int32_t prop_len;
	int ret;

	blob = fdt_get_property_w(fdt, offset, prop_name, &prop_len);
	if (blob) {
		ret = fdt_setprop(fdt, offset, prop_name, new_value, strlen(new_value) + 1);
		if (ret) {
			dpu_pr_err("ERROR:fdt_setprop(): %a\n", fdt_strerror(ret));
		} else {
			dpu_pr_debug("update %a as %a\n", prop_name, new_value);
		}
	} else {
		dpu_pr_warn("get null blob for prop:%a, new_value:%a\n", prop_name, new_value);
	}
}

int i2c_write(uint32_t bus_id, uint16_t addr, uint32_t reg, uint32_t reg_len,
		uint8_t *buf, uint32_t data_len)
{
	EFI_I2C_PROTOCOL *i2c = NULL;
	EFI_STATUS status;
	int index;

	status = gBS->LocateProtocol(&gEfiI2cMasterProtocolGuid, NULL,
		(VOID **)&i2c);
	if (EFI_ERROR (status) || !i2c) {
		DEBUG((DEBUG_ERROR, "Open i2c protocol failed\n"));
		return -1;
	}

	i2c->Write(i2c, bus_id, addr, reg, reg_len, buf, data_len);
	for (index = 0; index < data_len; index++)
		dpu_pr_debug("i2c write, reg:0x%X, buf[%d]:0x%x\n",
			reg, index, buf[index]);

	return 0;
}

int i2c_read(uint32_t bus_id, uint16_t addr, uint32_t reg, uint32_t reg_len,
		uint8_t *buf, uint32_t data_len)
{
	EFI_I2C_PROTOCOL *i2c = NULL;
	EFI_STATUS status;
	int index;

	status = gBS->LocateProtocol(&gEfiI2cMasterProtocolGuid, NULL,
		(VOID **)&i2c);
	if (EFI_ERROR (status) || !i2c) {
		DEBUG((DEBUG_ERROR, "Open i2c protocol failed\n"));
		return -1;
	}

	i2c->Read(i2c, bus_id, addr, reg, reg_len, buf, data_len);

	for (index = 0; index < data_len; index++)
		dpu_pr_debug("i2c read, reg:0x%X, buf[%d]:0x%x\n",
			reg, index, buf[index]);

	return 0;
}

int gpio_pull_status_set(uint32_t gpio_id, uint8_t pull_status)
{
	EFI_XR_IOC_PROTOCOL *IocProtocol;
	EFI_STATUS Status;

	Status = gBS->LocateProtocol(&gEfiIocProtocolGuid, NULL,
                      (VOID **)&IocProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("failed to locate IOC protocol, status:%d\n", Status);
		return -1;
	}

	Status = IocProtocol->IocSetPull(gpio_id, pull_status);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("set gpio %d pull down error, status:%d\n",
				gpio_id, Status);
		return -1;
	}

	return 0;
}

int gpio_func_set(uint32_t gpio_id, uint32_t func_id)
{
	EFI_XR_IOC_PROTOCOL *IocProtocol;
	EFI_STATUS Status;


	Status = gBS->LocateProtocol(&gEfiIocProtocolGuid, NULL,
                      (VOID **)&IocProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("failed to locate IOC protocol, status:%d\n", Status);
		return -1;
	}

	Status = IocProtocol->IocSetFunc(gpio_id, func_id);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("set gpio %d as func %d error, status:%d\n",
				gpio_id, func_id, Status);
		return -1;
	}

	return 0;
}

int gpio_ioc_prepare(uint32_t gpio_id, uint32_t func_id, uint8_t pull_status)
{
	EFI_XR_IOC_PROTOCOL *IocProtocol;
	EFI_STATUS Status;


	Status = gBS->LocateProtocol(&gEfiIocProtocolGuid, NULL,
                      (VOID **)&IocProtocol);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("failed to locate IOC protocol, status:%d\n", Status);
		return -1;
	}

	Status = IocProtocol->IocSetPull(gpio_id, pull_status);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("set gpio %d pull down error, status:%d\n",
				gpio_id, Status);
		return -1;
	}

	Status = IocProtocol->IocSetFunc(gpio_id, func_id);
	if (EFI_ERROR(Status)) {
		dpu_pr_err("set gpio %d as func %d error, status:%d\n",
				gpio_id, func_id, Status);
		return -1;
	}

	return 0;
}
