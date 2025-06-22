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

#include <soc/xring/xring_smmu_wrapper.h>
#include "asm/string.h"
#include "linux/mutex.h"
#include <soc/xring/securelib/securec.h>
#include "vdec_power_base.h"
#include "vdec_power_manager.h"
#include "linux/clk-provider.h"
#include "linux/clk.h"
#include "linux/err.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/regulator/consumer.h"
#include "linux/slab.h"
#include <linux/pm_runtime.h>


static int _vdec_smmu_dts_parse(vdec_power_mgr *pm) {
	const char *tbu_name = NULL;
	const char *sid_name = NULL;
	struct device_node *vdec_node = NULL;
	int ret;
	u32 iommus_data[3];

	vdec_node = of_find_compatible_node(NULL, NULL, "xring,vpu-dec");
	if (vdec_node) {
		ret = of_property_read_string(vdec_node, "tbu", &tbu_name);
		if (!strlen(tbu_name)) {
			vdec_pm_klog(LOGLVL_ERROR, " %s node's tbu name is empty, %d\n", vdec_node->name, ret);
			goto out;
		}
		vdec_pm_klog(LOGLVL_INFO, " tbu name is %s\n", tbu_name);

		ret = of_property_read_string(vdec_node, "sid", &sid_name);
		if (!strlen(sid_name)) {
			vdec_pm_klog(LOGLVL_ERROR, " %s node's sid name is empty, %d\n", vdec_node->name, ret);
			goto out;
		}
		vdec_pm_klog(LOGLVL_INFO, " sid name is %s\n", sid_name);

		ret = of_property_read_u32_array(vdec_node, "iommus", &iommus_data, ARRAY_SIZE(iommus_data));
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "%s node's iommus read failed, %d\n", vdec_node->name, ret);
			goto out;
		}
		vdec_pm_klog(LOGLVL_INFO, "iommus[0]: %u, iommus[1]/sid: %u, iommu[2]/ssid: %u\n",
			iommus_data[0], iommus_data[1], iommus_data[2]);

		ret = strncpy_s(pm->tbu_name, TBU_SID_NAME_LEN, tbu_name, strlen(tbu_name));
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "strncpy_s failed, ret: %d\n", ret);
			goto out;
		}
		ret = strncpy_s(pm->sid_name, TBU_SID_NAME_LEN, sid_name, strlen(sid_name));
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "strncpy_s failed, ret: %d\n", ret);
			goto out;
		}

		pm->sid = iommus_data[1];
		pm->ssid = iommus_data[2];
	} else {
		vdec_pm_klog(LOGLVL_ERROR, "xring,vpu-dec node is invalid");
		ret = EINVAL;
		goto out;
	}

	return 0;
out:
	return ret;
}

static int vdec_clk_init(vdec_power_mgr *pm) {
	int ret = 0;
	WARN_ON(!pm);
	WARN_ON(!pm->dev);
	struct device_node *node = pm->dev->of_node;
	vdec_clock *vdec_clk;
	u32 work_rate = 0;
	u32 default_rate = 0;
	const char *name = NULL;
	struct device_node *vdec_node = NULL;

	vdec_clk = kzalloc(sizeof(vdec_clock), GFP_KERNEL);
	if (!vdec_clk) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec_clk kzalloc failed!\n");
		return -ENOMEM;
	}

	vdec_node = of_find_compatible_node(NULL, NULL, "xring,vpu-dec");

	if (vdec_node) {
		ret = of_property_read_string(vdec_node, "clock-names", &name);
		if (!strlen(name)) {
			vdec_pm_klog(LOGLVL_ERROR, " %s node's clock name is empty, %d\n", vdec_node->name, ret);
			goto err;
		}
		ret = strncpy_s(vdec_clk->clk_name, VDEC_CLK_NAME_LEN, name, strlen(name));
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "strncpy_s failed, ret: %d\n", ret);
			goto err;
		}

		ret = of_property_read_u32(vdec_node, "work-rate", &work_rate);
		if (!work_rate) {
			vdec_pm_klog(LOGLVL_ERROR, " %s node work_rate[%d] is empty, %d\n", vdec_node->name, work_rate, ret);
			goto err;
		}
		ret = of_property_read_u32(vdec_node, "default-rate", &default_rate);
		if (!default_rate) {
			vdec_pm_klog(LOGLVL_ERROR, " %s node default_rate[%d] is empty, %d\n", vdec_node->name, default_rate, ret);
			goto err;
		}
	}

	vdec_clk->clk = devm_clk_get(pm->dev, vdec_clk->clk_name);
	if (IS_ERR_OR_NULL(vdec_clk->clk)) {
		vdec_pm_klog(LOGLVL_ERROR, "failed to get %s\n", vdec_clk->clk_name);
		kfree(vdec_clk);
		return -EFAULT;
	}

	vdec_clk->work_rate = work_rate;
	vdec_clk->default_rate = default_rate;

	pm->vdec_clk = vdec_clk;
	vdec_pm_klog(LOGLVL_INFO, "vdec clock got from device tree, name: %s,\
			work-rate: %d, default-rate: %d\n",
			vdec_clk->clk_name,
			work_rate,
			default_rate);
	return 0;

err:
	kfree(vdec_clk);
	return ret;
}

static void vdec_clk_deinit(vdec_clock *vdec_clk) {
	if (!vdec_clk) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec_clock NULL, is already free!\n");
		return;
	}

	kfree(vdec_clk);
}

vdec_power_mgr *vdec_power_mgr_init(struct device *dev) {
	int ret = 0;
	vdec_power_mgr *pm = NULL;
	int i;

	if (!dev) {
		vdec_pm_klog(LOGLVL_ERROR, "dev is NULL, invalid para!\n");
		return NULL;
	}

	pm = kzalloc(sizeof(vdec_power_mgr), GFP_KERNEL);
	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "power_mgr kzalloc failed!\n");
		return NULL;
	}

	for (i = 0; i < TBU_SID_NAME_LEN; i++) {
		pm->tbu_name[i] = '\0';
		pm->sid_name[i] = '\0';
	}
	pm->sid = SMMU_SID_INVALID;
	pm->ssid = SMMU_SSID_INVALID;
	ret = _vdec_smmu_dts_parse(pm);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec smmu dts parse failed:%d\n", ret);
		goto err;
	}
	pm->dev = dev;
	pm->runtime_idled = true;
	pm->sr_status = POWER_IS_NONSR;
	pm->cur_always_on = 0;
	pm->reset = 0;
	pm->runtime_init = false;
	for (i = 0; i < POWER_DOMAIN_MAX; i++)
		atomic_set(&pm->power_count[i], 0);

	pm->subsys_rg = devm_regulator_get(dev, "vsubsys");
	if (IS_ERR_OR_NULL(pm->subsys_rg)) {
		vdec_pm_klog(LOGLVL_ERROR, "failed to get vdec_subsys supply!\n");
		goto err;
	}

	pm->vdec_rg = devm_regulator_get(dev, "vcore");
	if (IS_ERR_OR_NULL(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_ERROR, "failed to get vdec_core supply!\n");
		goto err;
	}

	ret = vdec_clk_init(pm);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec_clk_init failed:%d!\n", ret);
		goto err;
	}

	mutex_init(&pm->pm_lock);
	mutex_init(&pm->pm_runtime_lock);
	spin_lock_init(&pm->pm_sr_lock);

	return pm;
err:
	kfree(pm);
	return NULL;
}

void vdec_power_mgr_deinit(vdec_power_mgr *power_mgr) {
	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "input para is NULL!\n");
		return;
	}
	mutex_destroy(&power_mgr->pm_lock);
	mutex_destroy(&power_mgr->pm_runtime_lock);
	vdec_clk_deinit(power_mgr->vdec_clk);
	power_mgr->vdec_clk = NULL;
	kfree(power_mgr);
}

int vdec_subsys_hw_init(vdec_power_mgr *pm) {
	int ret;
	WARN_ON(!pm);
	WARN_ON(!pm->dev);

	ret = xring_smmu_tcu_ctrl(pm->dev, XRING_SMMU_TCU_POWER_ON);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed enable smmu tcu: %d\n", ret);
		return ret;
	}

	ret = xring_smmu_tbu_ctrl(pm->dev, pm->tbu_name, XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed enable smmu tbu: %d\n", ret);
		goto err1;
	}

	ret = xring_smmu_sid_set(pm->dev, pm->sid_name, pm->sid, pm->ssid, 1);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed to set vdec sid: %d\n", ret);
		goto err2;
	}

	return ret;

err2:
	if (xring_smmu_tbu_ctrl(pm->dev, pm->tbu_name, XRING_SMMU_TBU_POWER_OFF)) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed dec XRING_SMMU_TBU_POWER_OFF smmu tbu: %d\n", ret);
	}
err1:
	if (xring_smmu_tcu_ctrl(pm->dev, XRING_SMMU_TCU_POWER_OFF)) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed disable smmu tcu: %d\n", ret);
	}
	return ret;
}

void vdec_subsys_hw_deinit(vdec_power_mgr *pm) {
	int ret = 0;

	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec_subsys_hw_deinit dev is NULL!\n");
		return;
	}
	WARN_ON(!pm->dev);

	ret = xring_smmu_tbu_ctrl(pm->dev, pm->tbu_name, XRING_SMMU_TBU_POWER_OFF);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed dec XRING_SMMU_TBU_POWER_OFF smmu tbu: %d\n", ret);
		return;
	}

	ret = xring_smmu_tcu_ctrl(pm->dev, XRING_SMMU_TCU_POWER_OFF);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed disable smmu tcu: %d\n", ret);
	}
}

int vdec_core_do_power_on(vdec_power_mgr *pm) {
	int ret;
	unsigned long flags;
	vdec_sr_status_e sr_status;

	WARN_ON(!pm);

	spin_lock_irqsave(&pm->pm_sr_lock, flags);
	sr_status = pm->sr_status;
	spin_unlock_irqrestore(&pm->pm_sr_lock, flags);
	vdec_pm_klog(LOGLVL_DEBUG, "CORE POWER ON SR STATUS %d, pm reset %d\n", sr_status, pm->reset);

	if (sr_status == POWER_IS_RESUMING || pm->reset)
		ret = vdec_force_power_on(pm, POWER_VDEC_CORE);
	else
		ret = vdec_power_on(pm, POWER_VDEC_CORE);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed to power on vdec core: %d\n", ret);
		return ret;
	}

	return ret;
}

void vdec_core_do_power_off(vdec_power_mgr *pm) {
	unsigned long flags;
	vdec_sr_status_e sr_status;

	WARN_ON(!pm);

	spin_lock_irqsave(&pm->pm_sr_lock, flags);
	sr_status = pm->sr_status;
	spin_unlock_irqrestore(&pm->pm_sr_lock, flags);
	vdec_pm_klog(LOGLVL_DEBUG, "CORE POWER OFF SR STATUS %d, pm reset %d\n", sr_status, pm->reset);

	if (sr_status == POWER_IS_SUSPENDING || pm->reset)
		vdec_force_power_off(pm, POWER_VDEC_CORE);
	else
		vdec_power_off(pm, POWER_VDEC_CORE);
}

void hantrodec_pm_runtime_get(struct device *dev, vdec_power_mgr *pm)
{
	int ret;
	vdec_pm_klog(LOGLVL_DEBUG, "runtime_status %d, power_count %d\n",
			dev->power.runtime_status, atomic_read(&pm->power_count[POWER_VDEC_CORE]));

	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		vdec_pm_klog(LOGLVL_WARNING, "warning, returned %d, usage_count %d, "
				"runtime_status %d, vdec_core power_count %d\n",
				ret, atomic_read(&dev->power.usage_count), dev->power.runtime_status,
				atomic_read(&pm->power_count[POWER_VDEC_CORE]));
		return;
	}
	vdec_pm_klog(LOGLVL_DEBUG, "returned %d, usage_count %d, runtime_status %d",
			ret, atomic_read(&dev->power.usage_count), dev->power.runtime_status);
}

void hantrodec_pm_runtime_put(struct device *dev)
{
	int ret;
	vdec_pm_klog(LOGLVL_DEBUG, "runtime_status %d, usage_count %d\n",
			dev->power.runtime_status, atomic_read(&dev->power.usage_count));

	pm_runtime_mark_last_busy(dev);
	ret = pm_runtime_put_autosuspend(dev);
	if (ret < 0) {
		vdec_pm_klog(LOGLVL_WARNING, "warning, returned %d, "
				"usage_count %d, runtime_status %d\n",
				ret, atomic_read(&dev->power.usage_count),
				dev->power.runtime_status);
		return;
	}

	vdec_pm_klog(LOGLVL_DEBUG, "returned %d, usage_count %d, runtime_status %d",
			ret, atomic_read(&dev->power.usage_count), dev->power.runtime_status);
}
