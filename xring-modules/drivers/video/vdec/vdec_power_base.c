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

#include "hantrovcmd_priv.h"
#include "linux/spinlock.h"
#include "vdec_power_manager.h"
#include "vdec_power_base.h"
#include "linux/clk-provider.h"
#include "linux/clk.h"
#include "linux/err.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/regulator/consumer.h"
#include "linux/slab.h"
#include "vdec_clk_manager.h"
#include <soc/xring/xr-clk-provider.h>

static inline struct clk *_vdec_get_clk(vdec_power_mgr *pm) {
	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter pm\n");
		return NULL;
	}

	if (!pm->vdec_clk) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter vdec_clk\n");
		return NULL;
	}

	return pm->vdec_clk->clk;
}

static inline char *_vdec_get_clk_name(vdec_clock *vdec_clk) {
	if (!vdec_clk) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter vdec_clk\n");
		return NULL;
	}

	return vdec_clk->clk_name;
}

static bool _domain_needs_to_power_on(int pwr_cnt,
								vdec_sr_status_e sr_status) {
	if (!pwr_cnt &&
		sr_status != POWER_IS_SUSPENDING &&
		sr_status != POWER_IS_SUSPENDED) {
		return true;
	} else {
		return false;
	}
}

void vdec_clk_rate_config(vdec_power_mgr* power_mgr, unsigned long rate) {
	struct clk *clk;
	int ret = 0;
	int low_temp_state = media_dvs_is_low_temperature();
	unsigned long todo_rate = rate;
#ifdef VDEC_CLK_ENABLE
	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return;
	}

	clk = _vdec_get_clk(power_mgr);
	if (clk && __clk_is_enabled(clk)) {
		const char *clk_name = _vdec_get_clk_name(power_mgr->vdec_clk);

		if (low_temp_state && rate > VDEC_CLK_MAX_RATE_LOW_TEMP) {
			todo_rate = VDEC_CLK_MAX_RATE_LOW_TEMP;
			vdec_pm_klog(LOGLVL_DEBUG, "clk low temp protection!");
		}
		ret = clk_set_rate(clk, todo_rate);
		if (ret < 0) {
			vdec_pm_klog(LOGLVL_ERROR, "failed to set clk %s to %lu, err:%d!\n",
				clk_name, todo_rate, ret);
		} else {
#ifdef CONFIG_DEC_CM_RUNTIME
			//Change the work frequency for the next power-on.
			if (todo_rate != power_mgr->vdec_clk->default_rate) {
				power_mgr->vdec_clk->work_rate = todo_rate;
			}
#endif
			vdec_pm_klog(LOGLVL_DEBUG, "set rate: %lu Hz\n", todo_rate);
		}
		vdec_pm_klog(LOGLVL_DEBUG, "clk_get_rate: %lu Hz\n", clk_get_rate(clk));
	}
#endif
}

/*
*0:success; !0:fail
*/
int vdec_clk_enable(vdec_power_mgr* power_mgr) {
	struct clk *clk;
	char *clk_name;
	int ret = 0;

#ifdef VDEC_CLK_ENABLE
	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}
	if (!regulator_is_enabled(power_mgr->subsys_rg)) {
		WARN_ON(!regulator_is_enabled(power_mgr->subsys_rg));
		return -EINVAL;
	}

	clk = _vdec_get_clk(power_mgr);
	if (clk) {
		ret = clk_prepare_enable(clk);
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "try to enable handrodec clk failed!\n");
		} else {
			clk_name = _vdec_get_clk_name(power_mgr->vdec_clk);
			vdec_pm_klog(LOGLVL_DEBUG, "enable clk:%s success, to be set work rate\n", clk_name);
		}
	} else {
		vdec_pm_klog(LOGLVL_ERROR, "clk is NULL!\n");
		return -EINVAL;
	}
#endif
	return ret;
}

void vdec_clk_disable(vdec_power_mgr* power_mgr) {
	struct clk *clk;
	char *clk_name;

#ifdef VDEC_CLK_ENABLE
	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return;
	}
	if (!regulator_is_enabled(power_mgr->subsys_rg)) {
		WARN_ON(!regulator_is_enabled(power_mgr->subsys_rg));
		return;
	}

	clk = _vdec_get_clk(power_mgr);
	if (clk) {
		clk_name = _vdec_get_clk_name(power_mgr->vdec_clk);
		clk_disable_unprepare(clk);
		vdec_pm_klog(LOGLVL_DEBUG, "disable clk:%s, clk already be set default rate!\n",
			clk_name);
	}
#endif
}

int vdec_power_on(vdec_power_mgr *power_mgr, vdec_power_domain_e pd) {
	int ret = 0;
	unsigned long flags;
	vdec_sr_status_e sr_status;

	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		goto err;
	}

	if (pd > POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_ERROR, "pd is invalid: %d!\n", pd);
		goto err;
	}

	if (!power_mgr->vdec_rg || !power_mgr->subsys_rg) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec regulator or vdec subsys regulator shouldn't be NULL\n");
		goto err;
	}

	spin_lock_irqsave(&power_mgr->pm_sr_lock, flags);
	sr_status = power_mgr->sr_status;
	spin_unlock_irqrestore(&power_mgr->pm_sr_lock, flags);

	if (pd == POWER_VDEC_CORE &&
		_domain_needs_to_power_on(atomic_read(&power_mgr->power_count[pd]), sr_status)) {
		vdec_pm_klog(LOGLVL_INFO, "domain_vdec_core power on\n");
		if (!regulator_is_enabled(power_mgr->vdec_rg)) {
			ret = vdec_clk_enable(power_mgr);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to vdec clk enable: %d\n", ret);
				goto out;
			}
			ret = regulator_enable(power_mgr->vdec_rg);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to enable vdec_rg regulator: %d\n", ret);
				vdec_clk_disable(power_mgr);
				BUG_ON(1);
				goto out;
			}
			vdec_clk_rate_config(power_mgr, vdec_get_work_rate(power_mgr->vdec_clk));

			ret = vdec_core_hw_config(power_mgr);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "vdec_core_hw_config failed:%d\n", ret);
				BUG_ON(1);
				goto out;
			}
			atomic_inc(&power_mgr->power_count[pd]);
			atomic_inc(&power_mgr->power_count[POWER_VDEC_SUBSYS]);
		}
	} else if (pd == POWER_VDEC_SUBSYS) {
		if (_domain_needs_to_power_on(atomic_read(&power_mgr->power_count[pd]), sr_status)) {
			vdec_pm_klog(LOGLVL_DEBUG, "domain_vdec_subsys power on\n");
			vdec_pm_klog(LOGLVL_DEBUG, "vdec_subsys rg enbale:%d\n", regulator_is_enabled(power_mgr->subsys_rg));
			ret = regulator_enable(power_mgr->subsys_rg);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to enable vdec_subsys_rg regulator: %d\n", ret);
				BUG_ON(1);
				goto out;
			}
			ret = vdec_subsys_hw_init(power_mgr);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR,
					"Failed to init vdec_subsys smmu resource, do regulator disable: %d\n", ret);
				WARN_ON(regulator_disable(power_mgr->subsys_rg));
				goto out;
			}
		}
		atomic_inc(&power_mgr->power_count[pd]);
	}

	vdec_pm_klog(LOGLVL_INFO, "power on finished, vdec_subsys power count: %d, vdec power count: %d!\n",
			atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VDEC_CORE]));

out:
	return ret;
err:
	return -EINVAL;
}

static inline int power_off_state_check(vdec_power_mgr *pm, vdec_power_domain_e pd) {
	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return -EINVAL;
	}

	if (pd > POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_ERROR, "pd is invalid: %d!\n", pd);
		return -EINVAL;
	}

	if (!pm->vdec_rg || !pm->subsys_rg) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec regulator or vdec_subsys regulator shouldn't be NULL\n");
		return -EINVAL;
	}

	if (!atomic_read(&pm->power_count[pd])) {
		vdec_pm_klog(LOGLVL_INFO, "power_count is zero, no need do domain[%d] power off!\n", pd);
		return 1;
	}
	return 0;
}

void vdec_power_off(vdec_power_mgr *power_mgr, vdec_power_domain_e pd)
{
	u32 count;
	unsigned long flags;
	int ret;
	vcmd_mgr_t *vcmd_mgr = NULL;
	struct hantrovcmd_dev *dev = NULL;
	WARN_ON(!power_mgr->dev);
	struct hantrodec_dev *dec_dev =  dev_get_drvdata(power_mgr->dev);
	if (dec_dev == NULL) {
		vdec_pm_klog(LOGLVL_ERROR, "dev get drvdata is NULL!\n");
		return;
	}

	ret = power_off_state_check(power_mgr, pd);
	if (ret) {
		vdec_pm_klog(LOGLVL_INFO, "power_off_state_check not passed, result: %d !\n", ret);
		return;
	}

	vcmd_mgr = (vcmd_mgr_t *)dec_dev->priv_data;
	if (vcmd_mgr == NULL) {
		vdec_pm_klog(LOGLVL_ERROR, "vcmd_mgr is NULL!\n");
		return;
	}

	dev = &vcmd_mgr->dev_ctx[0];
	WARN_ON(!dev);

	if (pd == POWER_VDEC_CORE && (atomic_read(&power_mgr->power_count[pd]) == 1) && !power_mgr->cur_always_on) {
		vdec_pm_klog(LOGLVL_INFO, "domain_vdec_core power off, vcmd state is %d\n",dev->state);
		if (regulator_is_enabled(power_mgr->vdec_rg)) {
			spin_lock_irqsave(dev->spinlock, flags);
			dev->state = VCMD_STATE_POWER_OFF;
			spin_unlock_irqrestore(dev->spinlock, flags);
			vdec_clk_rate_config(power_mgr, vdec_get_default_rate(power_mgr->vdec_clk));
			WARN_ON(regulator_disable(power_mgr->vdec_rg));
			vdec_clk_disable(power_mgr);
			atomic_dec(&power_mgr->power_count[pd]);
			atomic_dec(&power_mgr->power_count[POWER_VDEC_SUBSYS]);
		}
	} else if (pd == POWER_VDEC_SUBSYS) {
		atomic_dec(&power_mgr->power_count[pd]);
	}

	if (atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]) == 0) {
		vdec_pm_klog(LOGLVL_INFO, "domain_vdec_subsys power off\n");
		if (regulator_is_enabled(power_mgr->subsys_rg)) {
			vdec_subsys_hw_deinit(power_mgr);
			WARN_ON(regulator_disable(power_mgr->subsys_rg));
		}
		vdec_pm_klog(LOGLVL_INFO, "All power off, vdec_subsys: power count %d, regulator status:%d. "
				"vdec_core: power count %d, regulator status:%d\n",
				atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]),
				regulator_is_enabled(power_mgr->subsys_rg),
				atomic_read(&power_mgr->power_count[POWER_VDEC_CORE]),
				regulator_is_enabled(power_mgr->vdec_rg));
	}

	vdec_pm_klog(LOGLVL_INFO, "domain power down finish! vdec_subsys power count: %d, vdec power count: %d\n",
			atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VDEC_CORE]));
    return;
}

int vdec_force_power_on(vdec_power_mgr *power_mgr, vdec_power_domain_e pd)
{
	int ret = 0;

	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		goto err;
	}

	if (pd > POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_ERROR, "pd is invalid: %d!\n", pd);
		goto err;
	}

	if (!power_mgr->vdec_rg || !power_mgr->subsys_rg) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec regulator or vdec subsys regulator shouldn't be NULL\n");
		goto err;
	}

	if (pd == POWER_VDEC_CORE) {
		vdec_pm_klog(LOGLVL_DEBUG, "force domain_vdec_core power on\n");
		if (!regulator_is_enabled(power_mgr->vdec_rg)) {
			ret = vdec_clk_enable(power_mgr);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to vdec clk enable: %d\n", ret);
				goto out;
			}
			ret = regulator_enable(power_mgr->vdec_rg);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to enable vdec_rg regulator: %d\n", ret);
				vdec_clk_disable(power_mgr);
				BUG_ON(1);
				goto out;
			}
			vdec_clk_rate_config(power_mgr, vdec_get_work_rate(power_mgr->vdec_clk));

			ret = vdec_core_hw_config(power_mgr);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "vdec_core_hw_config failed:%d\n", ret);
				BUG_ON(1);
				goto out;
			}
		}
	} else if (pd == POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_DEBUG, "force domain_vdec_subsys power on\n");
		vdec_pm_klog(LOGLVL_INFO, "force vdec_subsys rg enbale:%d\n", regulator_is_enabled(power_mgr->subsys_rg));
		ret = regulator_enable(power_mgr->subsys_rg);
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "Failed to enable vdec_subsys_rg regulator: %d\n", ret);
			BUG_ON(1);
			goto out;
		}
		ret = vdec_subsys_hw_init(power_mgr);
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR,
				"Failed to init vdec_subsys smmu resource, do regulator disable: %d\n", ret);
			WARN_ON(regulator_disable(power_mgr->subsys_rg));
			goto out;
		}
	}

	vdec_pm_klog(LOGLVL_INFO, "force power on finished, vdec_subsys power count: %d, vdec power count: %d!\n",
			atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VDEC_CORE]));

out:
	return ret;
err:
	return -EINVAL;
}

void vdec_force_power_off(vdec_power_mgr *power_mgr, vdec_power_domain_e pd)
{
	int ret;
	unsigned long flags;
	struct hantrodec_dev *dec_dev =  NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;
	struct hantrovcmd_dev *dev = NULL;

	if (!power_mgr) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL!\n");
		return;
	}

	if (pd > POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_ERROR, "pd is invalid: %d!\n", pd);
		return;
	}

	if (!power_mgr->vdec_rg || !power_mgr->subsys_rg) {
		vdec_pm_klog(LOGLVL_ERROR, "vdec regulator or vdec_subsys regulator shouldn't be NULL\n");
		return;
	}

	if (!power_mgr->cur_always_on && !atomic_read(&power_mgr->power_count[pd])) {
		vdec_pm_klog(LOGLVL_INFO, "power_count is zero, no need do domain[%d] power off!\n", pd);
		return;
	}

	WARN_ON(!power_mgr->dev);
	dec_dev =  dev_get_drvdata(power_mgr->dev);
	if (dec_dev == NULL) {
		vdec_pm_klog(LOGLVL_ERROR, "dev_get_drvdata is NULL\n");
		return;
	}

	if (pd == POWER_VDEC_CORE) {
		vdec_pm_klog(LOGLVL_DEBUG, "force domain_vdec_core power off\n");
		if (regulator_is_enabled(power_mgr->vdec_rg)) {
			vcmd_mgr = (vcmd_mgr_t *)dec_dev->priv_data;
			if (vcmd_mgr != NULL) {
				dev = &vcmd_mgr->dev_ctx[0];
				WARN_ON(!dev);
				spin_lock_irqsave(dev->spinlock, flags);
				dev->state = VCMD_STATE_POWER_OFF;
				spin_unlock_irqrestore(dev->spinlock, flags);
			}

			vdec_clk_rate_config(power_mgr, vdec_get_default_rate(power_mgr->vdec_clk));
			WARN_ON(regulator_disable(power_mgr->vdec_rg));
			vdec_clk_disable(power_mgr);
		}
	} else if (pd == POWER_VDEC_SUBSYS) {
		vdec_pm_klog(LOGLVL_DEBUG, "force domain_vdec_subsys power off\n");
		if (regulator_is_enabled(power_mgr->subsys_rg)) {
			vdec_subsys_hw_deinit(power_mgr);
			WARN_ON(regulator_disable(power_mgr->subsys_rg));
		}
		vdec_pm_klog(LOGLVL_DEBUG, "force vdec_subsys/core rg enable status:%d/%d\n",
			regulator_is_enabled(power_mgr->subsys_rg), regulator_is_enabled(power_mgr->vdec_rg));
	}

	vdec_pm_klog(LOGLVL_DEBUG, "force domain power down finish! vdec_subsys power count: %d, vdec power count: %d\n",
			atomic_read(&power_mgr->power_count[POWER_VDEC_SUBSYS]),
			atomic_read(&power_mgr->power_count[POWER_VDEC_CORE]));

    return;
}
