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

#include "venc_power_manager.h"
#include "linux/clk-provider.h"
#include "linux/clk.h"
#include "linux/err.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/slab.h"
#include <soc/xring/xring_smmu_wrapper.h>
#include <linux/regulator/consumer.h>
#include "venc_clk_manager.h"
#include <soc/xring/xr-clk-provider.h>
#define ptr_null_and_return(ptr) {\
	if (ptr == NULL) {   \
		venc_pm_klog(LOGLVL_ERROR, "venc_pm: " fmt, ##__VA_ARGS__);  \
		return; \
	}  \
}
extern u32 always_on;

int venc_clk_init(venc_power_mgr *pm) {
	int ret = 0;
	struct device_node *node = NULL;
	const char *power_always_on = NULL;
	venc_clock *venc_clk;
	u32 work_rate = 0;
	u32 default_rate = 0;
	const char *name = NULL;

	venc_clk = kzalloc(sizeof(venc_clock), GFP_KERNEL);
	if (!venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "venc_clk kzalloc failed!\n");
		return -ENOMEM;
	}

	node = of_find_compatible_node(NULL, NULL, "xring,vpu-enc");
	if(!node) {
		venc_pm_klog(LOGLVL_ERROR, "find xring vpu-enc node fail\n");
		goto err;
	}

	ret = of_property_read_string(node, "clock-names", &name);
	if (!strlen(name)) {
		venc_pm_klog(LOGLVL_ERROR, " %s node's clock name is empty, %d\n", node->name, ret);
		goto err;
	}
	strncpy_s(venc_clk->clk_name, VENC_CLK_NAME_LEN, name, strlen(name));

	ret = of_property_read_u32(node, "work-rate", &work_rate);
	if (!work_rate) {
		venc_pm_klog(LOGLVL_ERROR, " %u node work_rate is empty, %d\n", work_rate, ret);
		goto err;
	}
	ret = of_property_read_u32(node, "default-rate", &default_rate);
	if (!default_rate) {
		venc_pm_klog(LOGLVL_ERROR, " %d node default_rate is empty, %d\n", default_rate, ret);
		goto err;
	}

	if (of_property_read_string(node, "power-always-on",
			&power_always_on)) {
		venc_pm_klog(LOGLVL_INFO, "failed to get string power-always-on\n");
	} else {
		if (!strcmp(power_always_on, "enable")) {
			always_on = 1;
			pm->always_on_curr_state = 1;
		} else if (!strcmp(power_always_on, "disable")) {
			always_on = 0;
			pm->always_on_curr_state = 0;
		} else {
			venc_pm_klog(LOGLVL_INFO, "power-always-on invalid!\n");
		}
	}

	venc_clk->clk = devm_clk_get(pm->dev, venc_clk->clk_name);

	if (IS_ERR_OR_NULL(venc_clk->clk)) {
		venc_pm_klog(LOGLVL_ERROR, "failed to get %s\n", venc_clk->clk_name);
		ret = -EFAULT;
		goto err;
	}
	venc_clk->work_rate = work_rate;
	venc_clk->default_rate = default_rate;
	pm->venc_clk = venc_clk;

	return 0;
err:
	kfree(venc_clk);
	return ret;
}

void venc_clk_deinit(venc_clock *venc_clk) {
	if (!venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "venc_clock NULL, is already free!\n");
		return;
	}
	kfree(venc_clk);
}

static inline struct clk *_venc_get_clk(venc_power_mgr *pm) {
	if (!pm) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter pm\n");
		return NULL;
	}
	if (!pm->venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter venc_clk\n");
		return NULL;
	}
	return pm->venc_clk->clk;
}

static inline char *_venc_get_clk_name(venc_clock *venc_clk) {
	if (!venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter venc_clk\n");
		return NULL;
	}
	return venc_clk->clk_name;
}


void venc_clk_rate_config(venc_power_mgr* power_mgr, unsigned long rate) {
	struct clk *clk;
	int ret = 0;
	int low_temp_state = media_dvs_is_low_temperature();
	unsigned long todo_rate = rate;
#ifdef VENC_CLK_ENABLE
	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL！\n");
		return;
	}

	clk = _venc_get_clk(power_mgr);

	if (clk && __clk_is_enabled(clk)) {
		const char *clk_name = _venc_get_clk_name(power_mgr->venc_clk);
		if (low_temp_state && rate > VENC_CLK_MAX_RATE_LOW_TEMP) {		
			todo_rate = VENC_CLK_MAX_RATE_LOW_TEMP;
			venc_pm_klog(LOGLVL_INFO, "clk low temp protection!");
		}
		ret = clk_set_rate(clk, todo_rate);
		if (ret < 0) {
			venc_pm_klog(LOGLVL_ERROR, "failed to set clk %s to %lu, err:%d!\n",
			clk_name, todo_rate, ret);
		} else {
#ifdef CONFIG_ENC_CM_RUNTIME
			//Change the work frequency for the next power-on.
			if (todo_rate != power_mgr->venc_clk->default_rate) {
				power_mgr->venc_clk->work_rate = todo_rate;
			}
#endif
			venc_pm_klog(LOGLVL_DEBUG, "set rate: %lu Hz\n", todo_rate);
		}
		venc_pm_klog(LOGLVL_DEBUG, "clk_get_rate: %lu Hz\n", clk_get_rate(clk));
	}
#endif
}

/*
*0:success; !0:fail
*/
int venc_clk_enable(venc_power_mgr* power_mgr) {
	struct clk *clk;
	char *clk_name;
	int ret = 0;

#ifdef VENC_CLK_ENABLE
	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL！\n");
		return -EINVAL;;
	}

	clk = _venc_get_clk(power_mgr);
	if (clk) {
		ret = clk_prepare_enable(clk);
		if (ret) {
			venc_pm_klog(LOGLVL_ERROR, "try to enable handrodec clk failed!\n");
		} else {
			clk_name = _venc_get_clk_name(power_mgr->venc_clk);
			venc_pm_klog(LOGLVL_INFO, "enable clk:%s success, to be set work rate\n", clk_name);
		}
	}
#endif
	return ret;
}

void venc_clk_disable(venc_power_mgr* power_mgr) {
	struct clk *clk;
	char *clk_name;
	unsigned long default_rate;
	int ret = 0;

#ifdef VENC_CLK_ENABLE
	if (!power_mgr) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter:power_mgr NULL！\n");
		return;
	}

	clk = _venc_get_clk(power_mgr);

	if (clk) {
		clk_name = _venc_get_clk_name(power_mgr->venc_clk);
		clk_disable_unprepare(clk);
		venc_pm_klog(LOGLVL_INFO, "disable clk:%s, clk already be set default rate!\n", clk_name);
	}
#endif
}
