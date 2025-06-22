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
#include <soc/xring/securelib/securec.h>
#include "venc_clk.h"
#include "venc_power_manager.h"
#include "linux/clk-provider.h"
#include "linux/clk.h"
#include "linux/err.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/regulator/consumer.h"
#include "linux/slab.h"

extern u32 always_on;

int hantroenc_get_vcmdmgr(struct device *dev, vcmd_mgr_t **vcmd_mgr) {
	struct hantroenc_dev *enc_dev = dev_get_drvdata(dev);
	if (enc_dev == NULL) {
		venc_pm_klog(LOGLVL_ERROR, "dev get drvdata is NULL!\n");
		return -EINVAL;
	}

	*vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);
	if (*vcmd_mgr == NULL) {
		venc_pm_klog(LOGLVL_ERROR, "vcmd_mgr is NULL!\n");
		return -EINVAL;
	}
	return 0;
}

venc_power_mgr *hantroenc_power_mgr_init(struct device *dev) {
	int ret = 0;
	venc_power_mgr *pm = NULL;
	int i;

	if (!dev) {
		venc_pm_klog(LOGLVL_ERROR, "dev is NULL, invalid para!\n");
		return NULL;
	}

	pm = kzalloc(sizeof(venc_power_mgr), GFP_KERNEL);
	if (!pm) {
		venc_pm_klog(LOGLVL_ERROR, "power_mgr kzalloc failed!\n");
		return NULL;
	}

	pm->dev = dev;
	pm->venc_clk = NULL;
	pm->always_on_curr_state = 0;
	pm->is_suspended = false;
	for (i = 0; i < POWER_DOMAIN_MAX; i++){
		atomic_set(&pm->power_count[i], 0);
	}

	pm->media2_rg = devm_regulator_get(dev, "media2_subsys");
	if (IS_ERR_OR_NULL(pm->media2_rg)) {
		venc_pm_klog(LOGLVL_ERROR, "failed to get media2 supply!\n");
		goto err;
	}

	pm->venc_rg = devm_regulator_get(dev, "venc_subsys");
	if (IS_ERR_OR_NULL(pm->venc_rg)) {
		venc_pm_klog(LOGLVL_ERROR, "failed to get venc_subsys supply!\n");
		goto err;
	}

	ret = venc_clk_init(pm);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "venc_clk_init failed:%d!\n", ret);
		goto err;
	}

	mutex_init(&pm->pm_lock);

	return pm;
err:
	kfree(pm);
	return NULL;
}

void hantroenc_power_mgr_deinit(venc_power_mgr *power_mgr) {
	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "input para is NULL!\n");
		return;
	}
	mutex_destroy(&power_mgr->pm_lock);
	venc_clk_deinit(power_mgr->venc_clk);
	kfree(power_mgr);
}

int hantroenc_pm_hw_init(struct device *dev)
{
	int ret = 0;
	vcmd_mgr_t *vcmd_mgr = NULL;

	vcmd_klog(LOGLVL_INFO, "called\n");

	ret = hantroenc_get_vcmdmgr(dev, &vcmd_mgr);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to get venc vcmdmgr\n");
		return ret;
	}

	ret = venc_qos_cfg();
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to venc qos cfg\n");
		return ret;
	}

	vcmd_config_modules(vcmd_mgr);
	vce_reset_asic(vcmd_mgr);
	vcmd_reset_asic(vcmd_mgr);
	return 0;
}

static inline int power_off_state_check(venc_power_mgr *pm, venc_power_domain_e pd) {
	if (!pm) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}

	if (pd > POWER_MEDIA2_SUBSYS) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:pd!\n");
		return -EINVAL;
	}

	if (!pm->venc_rg || !pm->media2_rg) {
		venc_pm_klog(LOGLVL_ERROR, "venc regulator or media2 regulator shouldn't be NULL\n");
		return -EINVAL;
	}

	if (!atomic_read(&pm->power_count[pd])) {
		venc_pm_klog(LOGLVL_INFO, "power_count is zero, no need do domain[%d] power off!\n", pd);
		return 1;
	}

	return 0;
}

void hantroenc_disable_tbu(struct device *dev) {
	int ret = 0;

	ret = xring_smmu_tbu_ctrl(dev, "media2_smmu_tbu4", XRING_SMMU_TBU_POWER_OFF);
	if (ret) {
		WARN_ON(ret);
		venc_pm_klog(LOGLVL_ERROR, "Failed dec XRING_SMMU_TBU_POWER_OFF smmu tbu: %d\n", ret);
	}
}

void hantroenc_disable_tcu(struct device *dev) {
	int ret = 0;

	ret = xring_smmu_tcu_ctrl(dev, XRING_SMMU_TCU_POWER_OFF);
	if (ret) {
		WARN_ON(ret);
		venc_pm_klog(LOGLVL_ERROR, "Failed disable smmu tcu: %d\n", ret);
	}
}

int hantroenc_enable_tbu(struct device *dev) {
	int ret = 0;

	ret = xring_smmu_tbu_ctrl(dev, "media2_smmu_tbu4", XRING_SMMU_TBU_POWER_ON);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed enable smmu tbu: %d\n", ret);
		WARN_ON(ret);
		return ret;
	}

	ret = xring_smmu_sid_set(dev, "media2_smmu_sid3", 4, 8, 1);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to set venc sid: %d\n", ret);
		goto smmu_sid_set_fail;
	}
	return 0;

smmu_sid_set_fail:
	xring_smmu_tbu_ctrl(dev, "media2_smmu_tbu4", XRING_SMMU_TBU_POWER_OFF);
	return ret;
}

int hantroenc_enable_tcu(struct device *dev) {
	int ret = 0;

	ret = xring_smmu_tcu_ctrl(dev, XRING_SMMU_TCU_POWER_ON);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed enable smmu tcu: %d\n", ret);
		WARN_ON(ret);
	}
	return ret;
}

void venc_power_off(venc_power_mgr *power_mgr, venc_power_domain_e pd) {
	int ret;
	int i = 0;
	unsigned long flags;
	struct hantrovcmd_dev *vdev;
	struct hantroenc_dev *enc_dev = NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;

	ret = power_off_state_check(power_mgr, pd);
	if (ret) {
		venc_pm_klog(LOGLVL_INFO, "power_off_state_check not passed, result: %d !\n", ret);
		return;
	}

	ret = hantroenc_get_vcmdmgr(power_mgr->dev, &vcmd_mgr);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to get venc vcmdmgr\n");
		return;
	}

	if (pd == POWER_VENC_SUBSYS &&
		(atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]) == 1) &&
		!power_mgr->is_suspended) {
		for (i = 0; i < vcmd_mgr->subsys_num; i++) {
			vdev = &vcmd_mgr->dev_ctx[i];
			spin_lock_irqsave(vdev->spinlock, flags);
			vdev->state = VCMD_STATE_POWER_OFF;
			spin_unlock_irqrestore(vdev->spinlock, flags);
		}
		venc_clk_rate_config(power_mgr, venc_get_default_rate(power_mgr->venc_clk));
		hantroenc_disable_tbu(power_mgr->dev);
		regulator_disable(power_mgr->venc_rg);
		venc_clk_disable(power_mgr);
		venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys power off\n");
	} else if (pd == POWER_MEDIA2_SUBSYS &&
		(atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]) == 1) &&
		!power_mgr->is_suspended) {
		hantroenc_disable_tcu(power_mgr->dev);
		regulator_disable(power_mgr->media2_rg);
		venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys power off\n");
	}
	atomic_dec(&power_mgr->power_count[pd]);

	venc_pm_klog(LOGLVL_INFO, "domain power down finish! m2 power count: %d, venc power count: %d\n",
			atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]));
}

void venc_force_power_off(venc_power_mgr *power_mgr, venc_power_domain_e pd) {
	int ret;
	int i = 0;
	unsigned long flags;
	struct hantrovcmd_dev *vdev;
	struct hantroenc_dev *enc_dev = NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;

	ret = power_off_state_check(power_mgr, pd);
	if (ret) {
		venc_pm_klog(LOGLVL_INFO, "power_off_state_check not passed, result: %d !\n", ret);
		return;
	}

	ret = hantroenc_get_vcmdmgr(power_mgr->dev, &vcmd_mgr);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to get venc vcmdmgr\n");
		return;
	}

	if (pd == POWER_VENC_SUBSYS) {
		if(regulator_is_enabled(power_mgr->venc_rg)) {
			for (i = 0; i < vcmd_mgr->subsys_num; i++) {
				vdev = &vcmd_mgr->dev_ctx[i];
				spin_lock_irqsave(vdev->spinlock, flags);
				vdev->state = VCMD_STATE_POWER_OFF;
				spin_unlock_irqrestore(vdev->spinlock, flags);
			}
			venc_clk_rate_config(power_mgr, venc_get_default_rate(power_mgr->venc_clk));
			hantroenc_disable_tbu(power_mgr->dev);
			regulator_disable(power_mgr->venc_rg);
			venc_clk_disable(power_mgr);
			venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys power off\n");
		} else {
			venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys is already powered off\n");
		}
	} else if (pd == POWER_MEDIA2_SUBSYS && (atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]))) {
		if(regulator_is_enabled(power_mgr->media2_rg)) {
			hantroenc_disable_tcu(power_mgr->dev);
			regulator_disable(power_mgr->media2_rg);
			venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys power off\n");
		} else {
			venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys is already powered off\n");
		}
	}

	power_mgr->is_suspended = true;
	venc_pm_klog(LOGLVL_INFO, "venc_force_power_off finish! m2 power count: %d, venc power count: %d\n",
			atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]));
}

int venc_power_on(venc_power_mgr *power_mgr, venc_power_domain_e pd) {
	int ret = 0;
	int i = 0;
	unsigned long flags;
	struct hantrovcmd_dev *vdev;
	struct hantroenc_dev *enc_dev = NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;

	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}

	ret = hantroenc_get_vcmdmgr(power_mgr->dev, &vcmd_mgr);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to get venc vcmdmgr\n");
		return ret;
	}

	if (!power_mgr->venc_rg || !power_mgr->media2_rg) {
		venc_pm_klog(LOGLVL_ERROR, "venc regulator or media2 regulator shouldn't be NULL\n");
		return -EINVAL;
	}


	if (pd == POWER_VENC_SUBSYS &&
		!atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]) &&
		!power_mgr->is_suspended) {
		venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys power on\n");
		ret = venc_clk_enable(power_mgr);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to venc clk enable: %d\n", ret);
			return ret;
		}
		ret = regulator_enable(power_mgr->venc_rg);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc_rg regulator: %d\n", ret);
			BUG_ON(ret);
			goto enable_venc_subsys_fail;
		}
		venc_clk_rate_config(power_mgr, venc_get_work_rate(power_mgr->venc_clk));

		ret = hantroenc_enable_tbu(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
			goto enable_tbu_fail;
		}
		ret = hantroenc_pm_hw_init(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "resume_hw_init failed\n");
			BUG_ON(ret);
			goto enable_tbu_fail;
		}
		for (i = 0; i < vcmd_mgr->subsys_num; i++) {
			vdev = &vcmd_mgr->dev_ctx[i];
			spin_lock_irqsave(vdev->spinlock, flags);
			vdev->state = VCMD_STATE_POWER_ON;
			spin_unlock_irqrestore(vdev->spinlock, flags);
		}
	} else if (pd == POWER_MEDIA2_SUBSYS &&
		!atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]) &&
		!power_mgr->is_suspended) {
		venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys power on\n");
		ret = regulator_enable(power_mgr->media2_rg);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable media2_rg regulator: %d\n", ret);
			BUG_ON(ret);
			return ret;
		}
		ret = hantroenc_enable_tcu(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
			goto enable_tcu_fail;
		}
	}

	atomic_inc(&power_mgr->power_count[pd]);
	venc_pm_klog(LOGLVL_INFO, "power on finished, m2 power count: %d, venc power count: %d!\n",
			atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]));

	return 0;

enable_tbu_fail:
	venc_clk_rate_config(power_mgr, venc_get_default_rate(power_mgr->venc_clk));
	regulator_disable(power_mgr->venc_rg);
enable_venc_subsys_fail:
	venc_clk_disable(power_mgr);
	hantroenc_disable_tcu(power_mgr->dev);
enable_tcu_fail:
	regulator_disable(power_mgr->media2_rg);
	return ret;
}

int venc_force_power_on(venc_power_mgr *power_mgr, venc_power_domain_e pd) {
	int ret = 0;
	unsigned long flags;
	struct hantrovcmd_dev *vdev = NULL;
	struct hantroenc_dev *enc_dev = NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;
	int i = 0;

	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}

	if (!power_mgr->venc_rg || !power_mgr->media2_rg) {
		venc_pm_klog(LOGLVL_ERROR, "venc regulator or media2 regulator shouldn't be NULL\n");
		return -EINVAL;
	}

	ret = hantroenc_get_vcmdmgr(power_mgr->dev, &vcmd_mgr);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to get venc vcmdmgr\n");
		return ret;
	}

	if ((pd == POWER_VENC_SUBSYS && atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]))) {
		if(!regulator_is_enabled(power_mgr->venc_rg)) {
			venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys power on\n");
			ret = venc_clk_enable(power_mgr);
			if (ret) {
				venc_pm_klog(LOGLVL_ERROR, "Failed to venc clk enable: %d\n", ret);
				return ret;
			}
			ret = regulator_enable(power_mgr->venc_rg);
			if (ret) {
				venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc_rg regulator: %d\n", ret);
				BUG_ON(ret);
				goto enable_venc_subsys_fail;
			}
			venc_clk_rate_config(power_mgr, venc_get_work_rate(power_mgr->venc_clk));
			ret = hantroenc_enable_tbu(power_mgr->dev);
			if (ret) {
				venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
				goto enable_tbu_fail;
			}
			ret = hantroenc_pm_hw_init(power_mgr->dev);
			if (ret) {
				venc_pm_klog(LOGLVL_ERROR, "resume_hw_init failed\n");
				BUG_ON(ret);
				goto enable_tbu_fail;
			}
			for (i = 0; i < vcmd_mgr->subsys_num; i++) {
				vdev = &vcmd_mgr->dev_ctx[i];
				spin_lock_irqsave(vdev->spinlock, flags);
				vdev->state = VCMD_STATE_POWER_ON;
				spin_unlock_irqrestore(vdev->spinlock, flags);
			}
		}
	} else if ((pd == POWER_MEDIA2_SUBSYS && atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]))) {
		venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys power on\n");
		ret = regulator_enable(power_mgr->media2_rg);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable media2_rg regulator: %d\n", ret);
			BUG_ON(ret);
			return ret;
		}
		ret = hantroenc_enable_tcu(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
			goto enable_tcu_fail;
		}
	}
	power_mgr->is_suspended = false;
	venc_pm_klog(LOGLVL_INFO, "venc_force_power_on finished, m2 power count: %d, venc power count: %d!\n",
			atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]));

	return 0;

enable_tbu_fail:
	venc_clk_rate_config(power_mgr, venc_get_default_rate(power_mgr->venc_clk));
	regulator_disable(power_mgr->venc_rg);
enable_venc_subsys_fail:
	venc_clk_disable(power_mgr);
	hantroenc_disable_tcu(power_mgr->dev);
enable_tcu_fail:
	regulator_disable(power_mgr->media2_rg);
	return ret;
}

int venc_probe_power_on(venc_power_mgr *power_mgr, venc_power_domain_e pd) {
	int ret = 0;

	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}

	if (!power_mgr->venc_rg || !power_mgr->media2_rg) {
		venc_pm_klog(LOGLVL_ERROR, "venc regulator or media2 regulator shouldn't be NULL\n");
		return -EINVAL;
	}

	if (pd == POWER_VENC_SUBSYS &&
		!atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]) &&
		!power_mgr->is_suspended) {
		venc_pm_klog(LOGLVL_INFO, "domain_venc_subsys power on\n");
		ret = venc_clk_enable(power_mgr);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to venc clk enable: %d\n", ret);
			return ret;
		}
		ret = regulator_enable(power_mgr->venc_rg);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc_rg regulator: %d\n", ret);
			BUG_ON(ret);
			goto enable_venc_subsys_fail;
		}
		venc_clk_rate_config(power_mgr, venc_get_work_rate(power_mgr->venc_clk));

		ret = hantroenc_enable_tbu(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
			goto enable_tbu_fail;
		}
	} else if (pd == POWER_MEDIA2_SUBSYS &&
		!atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]) &&
		!power_mgr->is_suspended) {
		venc_pm_klog(LOGLVL_INFO, "domain_media2_subsys power on\n");
		ret = regulator_enable(power_mgr->media2_rg);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable media2_rg regulator: %d\n", ret);
			BUG_ON(ret);
			return ret;
		}
		ret = hantroenc_enable_tcu(power_mgr->dev);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "Failed to enable venc tbu: %d\n", ret);
			goto enable_tcu_fail;
		}
	}

	atomic_inc(&power_mgr->power_count[pd]);

	venc_pm_klog(LOGLVL_INFO, "power on finished, m2 power count: %d, venc power count: %d!\n",
			atomic_read(&power_mgr->power_count[POWER_MEDIA2_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VENC_SUBSYS]));

	return 0;
enable_tbu_fail:
	venc_clk_rate_config(power_mgr, venc_get_default_rate(power_mgr->venc_clk));
	regulator_disable(power_mgr->venc_rg);
enable_venc_subsys_fail:
	venc_clk_disable(power_mgr);
	hantroenc_disable_tcu(power_mgr->dev);
enable_tcu_fail:
	regulator_disable(power_mgr->media2_rg);
	return ret;
}

/* put on power, clock */
int hantroenc_power_on(venc_power_mgr *pm) {
	int ret;
	WARN_ON(!pm);

	mutex_lock(&pm->pm_lock);
	ret = venc_probe_power_on(pm, POWER_MEDIA2_SUBSYS);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on media2_subsys: %d\n", ret);
		return ret;
	}

	ret = venc_probe_power_on(pm, POWER_VENC_SUBSYS);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
		goto power_on_venc_subsys_fail;
	}

	mutex_unlock(&pm->pm_lock);
	return ret;
power_on_venc_subsys_fail:
	venc_power_off(pm, POWER_MEDIA2_SUBSYS);
	mutex_unlock(&pm->pm_lock);
	return ret;
}

void hantroenc_power_off(venc_power_mgr *pm) {
	WARN_ON(!pm);
	mutex_lock(&pm->pm_lock);
	venc_power_off(pm, POWER_VENC_SUBSYS);
	venc_power_off(pm, POWER_MEDIA2_SUBSYS);
	mutex_unlock(&pm->pm_lock);
}
