// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include <linux/clk.h>
#include <soc/xring/flowctrl.h>
#include <soc/xring/doze.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>

#include "veu_res.h"
#include "veu_defs.h"
#include "veu_utils.h"
#include "veu_uapi.h"
#include "veu_trace.h"

#define IRQ_VEU_NAME "veu_irq"

#define VEU_RESET_CLK_DPU_CORE0 26112000UL
#define VEU_CORE_MCLK_RATE_060 362500000UL
#define VEU_CORE_MCLK_RATE_065 483400000UL
#define VEU_CORE_MCLK_RATE_075 557056000UL

#define VEU_CORE_ACLK_RATE_060 417800000UL
#define VEU_CORE_ACLK_RATE_065 557060000UL
#define VEU_CORE_ACLK_RATE_075 725000000UL

enum VEU_CORE_RATE {
	CORE_RATE_LVL1 = 0,
	CORE_RATE_LVL2,
	CORE_RATE_LVL3,
	CORE_RATE_MAX,
};

static const uint64_t g_mclk_rate[CORE_RATE_MAX] = {
	VEU_CORE_MCLK_RATE_060,
	VEU_CORE_MCLK_RATE_065,
	VEU_CORE_MCLK_RATE_075,
};

static const uint64_t g_aclk_rate[CORE_RATE_MAX] = {
	VEU_CORE_ACLK_RATE_060,
	VEU_CORE_ACLK_RATE_065,
	VEU_CORE_ACLK_RATE_075,
};

extern int dpu_hw_dvfs_vote(bool enable);

static const char *g_veu_clk_names[VEU_CLK_NUM_MAX] = {
	"clk_dpu_veu",
	"clk_dpu_veu_axi",
	"pclk_veu_cfg",
	"clk_disp_bus_data",
	"clk_disp_bus_cfg",
	"pclk_dpu_cfg",
};

irqreturn_t veu_isr_handler(int irq, void *ptr)
{
	struct veu_data *veu_dev = NULL;
	uint32_t isr;

	veu_dev = (struct veu_data *)ptr;
	veu_check_and_return(!veu_dev, IRQ_NONE, "veu device is null");

	isr = inp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_STATUS);
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_STATUS, isr);

	if (isr & BIT_WB_FRM_DONE) {
		veu_dev->frm_done_flag = 1;
		wake_up_interruptible_all(&veu_dev->frm_done_wq);
	}

	return IRQ_HANDLED;
}

int veu_init_isr(struct veu_data *veu_dev)
{
	int ret;

	veu_check_and_return(!veu_dev, -1, "veu_dev is null");
	veu_check_and_return(veu_dev->veu_irq == 0,
		-1, "invalid irq_no %d", veu_dev->veu_irq);

	veu_dev->veu_irq_info.isr_fnc = veu_isr_handler;

	ret = request_irq(veu_dev->veu_irq,
		veu_dev->veu_irq_info.isr_fnc, 0, IRQ_VEU_NAME, veu_dev);
	veu_check_and_return(ret, -1,
		"veu request_irq failed, irq_no=%d err_no=%d", veu_dev->veu_irq, ret);

	disable_irq(veu_dev->veu_irq);

	init_waitqueue_head(&veu_dev->frm_done_wq);
	veu_dev->frm_done_flag = 0;

	return ret;
}

void veu_free_isr(struct veu_data *veu_dev)
{
	veu_check_and_void_return(!veu_dev, "veu_dev is null");

	disable_irq(veu_dev->veu_irq);
	free_irq(veu_dev->veu_irq, veu_dev);
}

static int media1subsys_regulator_enable(struct veu_data *veu_dev)
{
	int ret;

	ret = regulator_enable(veu_dev->power_mgr->veu_supply[MEDIA1_SUBSYS]);
	if (ret)
		VEU_ERR("fail to enable media1subsys regulator: err %d",  ret);
	return ret;
}

static int dpu0_regulator_enable(struct veu_data *veu_dev)
{
	int ret;

	ret = regulator_enable(veu_dev->power_mgr->veu_supply[DPU0]);
	if (ret)
		VEU_ERR("fail to enable dpu0 regulator: err %d",  ret);
	return ret;
}

static int veusubsys_regulator_enable(struct veu_data *veu_dev)
{
	int ret;

	ret = regulator_enable(veu_dev->power_mgr->veu_supply[VEU_TOP_SUBSYS]);
	if (ret)
		VEU_ERR("fail to enable veu regulator: err %d",  ret);
	return ret;
}

static void media1subsys_regulator_disable(struct veu_data *veu_dev)
{
	regulator_disable(veu_dev->power_mgr->veu_supply[MEDIA1_SUBSYS]);
}

static void dpu0_regulator_disable(struct veu_data *veu_dev)
{
	regulator_disable(veu_dev->power_mgr->veu_supply[DPU0]);
}

static void veusubsys_regulator_disable(struct veu_data *veu_dev)
{
	regulator_disable(veu_dev->power_mgr->veu_supply[VEU_TOP_SUBSYS]);
}

static int veu_reset_core_clk(struct veu_data *veu_dev)
{
	struct clk *mclk = veu_dev->clk_mgr->veu_clk[VEU_MCLK];
	int ret;

	ret = clk_set_rate(mclk, VEU_RESET_CLK_DPU_CORE0);
	if (ret < 0) {
		VEU_ERR("failed to set core clk rate to %lu, err %d", VEU_RESET_CLK_DPU_CORE0, ret);
		return -1;
	}

	VEU_DBG("core clk rate set to %lu", clk_get_rate(mclk));

	return 0;
}

static int veu_set_core_clk_rate(struct veu_data *veu_dev, uint32_t level)
{
	struct clk *mclk = veu_dev->clk_mgr->veu_clk[VEU_MCLK];
	int ret;

	mutex_lock(&veu_dev->clk_mgr->lock);
	level = (veu_dev->clk_rate_lock_level != PROFILE_LVL_INVALID) ? veu_dev->clk_rate_lock_level : level;

	ret = clk_set_rate(mclk, g_mclk_rate[level]);
	if (ret < 0) {
		VEU_ERR("failed to set core clk rate to %llu, err %d", g_mclk_rate[level], ret);
		mutex_unlock(&veu_dev->clk_mgr->lock);
		return -1;
	}

	VEU_DBG("core clk rate set to %lu", clk_get_rate(mclk));

	mutex_unlock(&veu_dev->clk_mgr->lock);
	return 0;
}

static int veu_clk_enable(struct veu_data *veu_dev)
{
	int i;
	int ret;

	for (i = 0; i < VEU_CLK_NUM_MAX; i++) {
		ret = clk_prepare_enable(veu_dev->clk_mgr->veu_clk[i]);
		if (ret) {
			VEU_ERR("%s clk enable fail", g_veu_clk_names[i]);
			goto end;
		} else {
			VEU_DBG("%s clk enable succ", g_veu_clk_names[i]);
		}
	}

	return 0;
end:
	i--;
	for (; i >= 0; i--)
		clk_disable_unprepare(veu_dev->clk_mgr->veu_clk[i]);

	return ret;
}

static void veu_clk_disable(struct veu_data *veu_dev)
{
	int i;

	for (i = 0; i < VEU_CLK_NUM_MAX; i++) {
		clk_disable_unprepare(veu_dev->clk_mgr->veu_clk[i]);
		VEU_DBG("%s clk disable", g_veu_clk_names[i]);
	}
	veu_dev->clk_rate_lock_level = PROFILE_LVL_INVALID;
}

static void veu_irq_disable(struct veu_data *veu_dev)
{
	uint32_t mask;

	mask = 0;
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_MASK, mask);
	mask = 0xffffffff;
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_STATUS, mask);

	disable_irq(veu_dev->veu_irq);
}

static int veu_irq_setup_and_enable(struct veu_data *veu_dev)
{
	uint32_t mask;
	uint32_t val;

	VEU_DBG("enter");
	// mask
	mask = 0;
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_MASK, mask);
	val = inp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_MASK);
	VEU_DBG("0x%x : 0x%x", veu_dev->addr + VEU_INT + VEU_FUNC_IRQ_MASK, val);
	// clear
	mask = 0xffffffff;
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_STATUS, mask);
	val = inp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_STATUS);
	VEU_DBG("0x%x : 0x%x", veu_dev->addr + VEU_INT + VEU_FUNC_IRQ_STATUS, val);
	// enable
	enable_irq(veu_dev->veu_irq);
	// unmask
	mask = 0x30;
	outp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_MASK, mask);
	val = inp32(veu_dev->base + VEU_INT + VEU_FUNC_IRQ_MASK);
	VEU_DBG("0x%x : 0x%x", veu_dev->addr + VEU_INT + VEU_FUNC_IRQ_MASK, val);

	VEU_DBG("exit");

	return 0;
}

static int veu_on(struct veu_data *veu_dev)
{
	int ret;

	VEU_INFO("enter");
	trace_veu_on("veu on start");

	if (!veu_dev) {
		VEU_ERR("veu dev null");
		return -1;
	}

	/* vote disable dpu hw dvfs */
	ret = dpu_hw_dvfs_vote(false);
	if (ret)
		VEU_ERR("vote dpu hw dvfs disable failed\n");

	/* vote disable soc doze */
	sys_state_doz2nor_vote(VOTER_DOZAP_VEU);

	ret = media1subsys_regulator_enable(veu_dev);
	veu_check_and_return(ret, -1, "media1subsys regulator enable fail");

	ret = veu_clk_enable(veu_dev);
	if (ret) {
		VEU_ERR("veu clk enable fail");
		media1subsys_regulator_disable(veu_dev);
		return -1;
	}

	ret = dpu0_regulator_enable(veu_dev);
	if (ret) {
		VEU_ERR("dpu0 regulator enable fail");
		media1subsys_regulator_disable(veu_dev);
		return -1;
	}

	ret = veusubsys_regulator_enable(veu_dev);
	if (ret) {
		VEU_ERR("veusubsys regulator enable fail");
		dpu0_regulator_disable(veu_dev);
		media1subsys_regulator_disable(veu_dev);
		veu_clk_disable(veu_dev);
		return -1;
	}

	xring_flowctrl_veu_top_subsys_cfg();

	veu_set_core_clk_rate(veu_dev, CORE_RATE_LVL1);

	veu_hw_init(veu_dev);

	veu_irq_setup_and_enable(veu_dev);

	trace_veu_on("veu on end");
	VEU_INFO("exit");

	return 0;
}

extern int veu_ddr_bandwidth_update(u32 veu_expect_bandwidth);

static int veu_off(struct veu_data *veu_dev)
{
	int ret;

	trace_veu_process("veu off start");
	VEU_INFO("enter");

	if (!veu_dev) {
		VEU_ERR("veu dev null");
		return -1;
	}

	veu_irq_disable(veu_dev);

	veu_reset_core_clk(veu_dev);

	veusubsys_regulator_disable(veu_dev);

	dpu0_regulator_disable(veu_dev);

	veu_clk_disable(veu_dev);

	media1subsys_regulator_disable(veu_dev);

	/* vote enable dpu hw dvfs */
	ret = dpu_hw_dvfs_vote(true);
	if (ret)
		VEU_ERR("vote dpu hw dvfs enable failed\n");

	veu_ddr_bandwidth_update(0);

	/* vote enable soc doze */
	sys_state_doz2nor_unvote(VOTER_DOZAP_VEU);

	trace_veu_process("veu off end");
	VEU_INFO("exit");

	return 0;
}

int veu_clk_mgr_init(struct device *dev, struct veu_clk_mgr **mgr)
{
	int i;
	struct veu_clk_mgr *clk_mgr = NULL;

	if (!dev) {
		VEU_ERR("dev is NULL");
		return -EINVAL;
	}

	clk_mgr = kzalloc(sizeof(*clk_mgr), GFP_KERNEL);
	if (!clk_mgr)
		return -ENOMEM;

	for (i = 0; i < VEU_CLK_NUM_MAX; i++) {
		clk_mgr->veu_clk[i] = devm_clk_get(dev, g_veu_clk_names[i]);
		if (IS_ERR(clk_mgr->veu_clk[i])) {
			VEU_ERR("fail to get %s", g_veu_clk_names[i]);
			goto err;
		}
	}

	mutex_init(&clk_mgr->lock);
	*mgr = clk_mgr;
	return 0;
err:
	kfree(clk_mgr);
	clk_mgr = NULL;
	return -EINVAL;
}

void veu_clk_mgr_deinit(struct veu_clk_mgr *clk_mgr)
{
	veu_check_and_void_return(!clk_mgr, "clk_mgr is null");
	mutex_destroy(&clk_mgr->lock);
	kfree(clk_mgr);
	clk_mgr = NULL;
}

int veu_power_mgr_init(struct device *dev, struct veu_power_mgr **mgr)
{
	struct veu_power_mgr *power_mgr = NULL;
	char target_supply[20];
	int ret;

	if (!dev) {
		VEU_ERR("dev is NULL");
		return -EINVAL;
	}

	veu_check_and_return(*mgr, -1, "mgr has been initialized");

	power_mgr = kzalloc(sizeof(*power_mgr), GFP_KERNEL);
	if (!power_mgr)
		return -ENOMEM;

	ret = snprintf(target_supply, sizeof(target_supply), "media1_subsys");
	power_mgr->veu_supply[MEDIA1_SUBSYS] = devm_regulator_get(dev, target_supply);
	if (IS_ERR(power_mgr->veu_supply[MEDIA1_SUBSYS])) {
		VEU_ERR("failed to get %s supply\n", target_supply);
		goto err_regulator_get;
	}

	ret = snprintf(target_supply, sizeof(target_supply), "dpu0");
	power_mgr->veu_supply[DPU0] = devm_regulator_get(dev, target_supply);
	if (IS_ERR(power_mgr->veu_supply[DPU0])) {
		VEU_ERR("failed to get %s supply\n", target_supply);
		goto err_regulator_get;
	}

	ret = snprintf(target_supply, sizeof(target_supply), "veu_top_subsys");
	power_mgr->veu_supply[VEU_TOP_SUBSYS] = devm_regulator_get(dev, target_supply);
	if (IS_ERR(power_mgr->veu_supply[VEU_TOP_SUBSYS])) {
		VEU_ERR("failed to get %s supply", target_supply);
		goto err_regulator_get;
	}

	atomic_set(&power_mgr->power_count, 0);
	*mgr = power_mgr;

	return 0;

err_regulator_get:
	kfree(power_mgr);
	power_mgr = NULL;
	return -EINVAL;
}

void veu_power_mgr_deinit(struct veu_power_mgr *power_mgr)
{
	veu_check_and_void_return(!power_mgr, "power_mgr is null");
	kfree(power_mgr);
	power_mgr = NULL;
}

void veu_callback_register(struct veu_data *veu_dev)
{
	veu_check_and_void_return(!veu_dev, "veu_dev is null");

	veu_dev->on = veu_on;
	veu_dev->off = veu_off;
	veu_dev->set_core_clk_rate = veu_set_core_clk_rate;
	veu_dev->set_reg = cmdlist_write;
}
