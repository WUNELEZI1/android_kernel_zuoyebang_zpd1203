// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "clk", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "clk", __func__, __LINE__

#include <linux/io.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/err.h>
#include "xrisp_clk.h"
#include "xrisp_log.h"
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"

void xrisp_clk_release_devs(struct xrisp_clk_data *priv)
{
	if (priv && priv->clk_info) {
		int i;

		for (i = 0; i < priv->clk_num; i++) {
			struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];

			if (!IS_ERR(clk_info_p->clk_p) && clk_info_p->clk_p != NULL) {
				while (atomic_read(&clk_info_p->clk_enable) > 0) {
					clk_disable_unprepare(clk_info_p->clk_p);
					atomic_dec(&clk_info_p->clk_enable);
				}
				clk_info_p->clk_p = NULL;
			}
			XRISP_PR_INFO("%s disable, clk_name = %s", priv->name, clk_info_p->clk_name);
		}

		priv->clk_info->clk_init_rate = NULL;
		priv->clk_info = NULL;
	}
}

int xrisp_clk_enable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int ret = 0;
	int i;

	for (i = 0; i < priv->clk_num; i++) {
		if ((1 << i) & mask) {
			struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];

			if (clk_info_p->clk_p == NULL)
				clk_info_p->clk_p = devm_clk_get(priv->dev, clk_info_p->clk_name);

			if (IS_ERR(clk_info_p->clk_p)) {
				XRISP_PR_ERROR("%s get clk fail, clk_name = %s, clp_p = %pK",
					priv->name, clk_info_p->clk_name, clk_info_p->clk_p);
				return -ENODEV;
			}

			ret = clk_prepare_enable(clk_info_p->clk_p);
			if (ret) {
				XRISP_PR_ERROR("%s enable clk fail, clk_name = %s, ret = %d", priv->name,
					clk_info_p->clk_name, ret);
				return ret;
			}

			atomic_inc(&clk_info_p->clk_enable);
		}
	}

	return ret;
}

unsigned int xrisp_clk_disable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int i;
	unsigned int enable_mask = 0;

	for (i = 0; i < priv->clk_num; i++) {
		if ((1 << i) & mask) {
			struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];
			int count = 0;

			if (!IS_ERR(clk_info_p->clk_p) && clk_info_p->clk_p != NULL
				&& atomic_read(&clk_info_p->clk_enable) > 0) {
				clk_disable_unprepare(clk_info_p->clk_p);
				atomic_dec(&clk_info_p->clk_enable);
				count = atomic_read(&clk_info_p->clk_enable);
				if (count)
					enable_mask |= 1 << i;
				XRISP_PR_DEBUG("%s disable clk, clk_name = %s, clk cnt = %d", priv->name,
					clk_info_p->clk_name, count);
			}
		}
	}

	return enable_mask;
}

unsigned int xrisp_clk_get_stats(struct xrisp_clk_data *priv)
{

	unsigned int ret = 0;
	int i;

	for (i = 0; i < priv->clk_num; i++) {
		struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];

		if (!IS_ERR(clk_info_p->clk_p) && atomic_read(&clk_info_p->clk_enable))
			ret |= 1 << i;
	}

	XRISP_PR_INFO("%s status = %u", priv->name, ret);

	return ret;
}

int xrisp_clk_set_rate_by_mask(struct xrisp_clk_data *priv, unsigned int mask, int rate_index)
{
	int ret = 0;
	int i;

	for (i = 0; i < priv->clk_num; i++) {
		if ((1 << i) & mask) {
			struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];

			if (IS_ERR(clk_info_p->clk_p) || clk_info_p->clk_p == NULL) {
				ret = -ENODEV;
				XRISP_PR_ERROR("%s clk ptr is error, clk_name = %s",
					priv->name, clk_info_p->clk_name);
				return ret;
			}
			if (atomic_read(&clk_info_p->clk_enable) <= 0) {
				ret = -ENODEV;
				XRISP_PR_ERROR("%s clk is disable, clk_name = %s",
					priv->name, clk_info_p->clk_name);
				return ret;
			}
			if (clk_info_p->clk_init_rate) {
				ret = clk_set_rate(clk_info_p->clk_p,
						   clk_info_p->clk_init_rate[rate_index]);
				if (ret) {
					XRISP_PR_ERROR("%s set clk rate fail, clk_name = %s, ret = %d",
						       priv->name, clk_info_p->clk_name, ret);
					return ret;
				}
				clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
#ifdef XRISP_CLK_RATE_DEBUG
				XRISP_PR_DEBUG("%s clk name = %s, init clk rate = %u", priv->name,
					       clk_info_p->clk_name, clk_info_p->clk_rate);
#endif
			}
		}
	}
	return ret;
}

int xrisp_clk_enable_init_rate(struct xrisp_clk_data *priv, unsigned int mask, int rate_index)
{
	int ret = 0;

	ret = xrisp_clk_enable(priv, mask);

	if (ret) {
		XRISP_PR_ERROR("%s enable clk fail", priv->name);
		return ret;
	}

	ret = xrisp_clk_set_rate_by_mask(priv, mask, rate_index);
	if (ret) {
		XRISP_PR_ERROR("%s set clk rate fail", priv->name);
		return ret;
	}

	return ret;
}

int xrisp_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	int ret = 0;
	struct xrisp_clk_info_t *clk_info_p = NULL;

	if (ch < 0 || ch >= priv->clk_num) {
		ret = -ECHRNG;
		return ret;
	}
	clk_info_p = &priv->clk_info[ch];
	if (IS_ERR(clk_info_p->clk_p) || clk_info_p->clk_p == NULL) {
		ret = -ENODEV;
		XRISP_PR_ERROR("%s clk ptr is error, clk_name = %s", priv->name, clk_info_p->clk_name);
		return ret;
	}
	if (atomic_read(&clk_info_p->clk_enable) <= 0) {
		ret = -ENODEV;
		XRISP_PR_ERROR("%s clk is disable, clk_name = %s", priv->name, clk_info_p->clk_name);
		return ret;
	}
	ret = clk_set_rate(clk_info_p->clk_p, rate);
	if (ret)
		XRISP_PR_ERROR("%s set clk rate fail, clk_name = %s, ret = %d",
			priv->name, clk_info_p->clk_name, ret);
	else {
		clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
#ifdef XRISP_CLK_RATE_DEBUG
		XRISP_PR_DEBUG("%s set clk name = %s, clk rate = %u", priv->name, clk_info_p->clk_name,
				clk_info_p->clk_rate);
#endif
	}
	return ret;
}

unsigned int xrisp_clk_get_rate(struct xrisp_clk_data *priv, int ch)
{
	struct xrisp_clk_info_t *clk_info_p = NULL;

	if (ch < 0 || ch >= priv->clk_num)
		return 0;
	clk_info_p = &priv->clk_info[ch];
	if (IS_ERR(clk_info_p->clk_p)) {
		XRISP_PR_ERROR("%s ptr is null, clk_name = %s", priv->name, clk_info_p->clk_name);
		return 0;
	}
	if (atomic_read(&clk_info_p->clk_enable) <= 0) {
		XRISP_PR_ERROR("%s is disable, clk_name = %s", priv->name, clk_info_p->clk_name);
		return 0;
	}
	clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
	return clk_info_p->clk_rate;
}

void xrisp_clk_deinit_clk(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	struct xrisp_clk_info_t *clk_info_p = NULL;

	if (ch < 0 || ch >= priv->clk_num)
		return;
	clk_info_p = &priv->clk_info[ch];

	if (clk_info_p->clk_p == NULL)
		clk_info_p->clk_p = devm_clk_get(priv->dev, clk_info_p->clk_name);
	if (IS_ERR(clk_info_p->clk_p)) {
		XRISP_PR_ERROR("%s get clk fail, clk_name = %s, clp_p = %pK",
			priv->name, clk_info_p->clk_name, clk_info_p->clk_p);
		return;
	}
	if (clk_set_rate(clk_info_p->clk_p, rate)) {
		XRISP_PR_ERROR("%s set clk rate fail, clk_name = %s", priv->name,
				clk_info_p->clk_name);
		return;
	}
	clk_info_p->clk_rate = clk_get_rate(clk_info_p->clk_p);
#ifdef XRISP_CLK_RATE_DEBUG
	XRISP_PR_DEBUG("%s set clk name = %s, clk rate = %u", priv->name, clk_info_p->clk_name,
		clk_info_p->clk_rate);
#endif
}

int xrisp_clk_init_priv_data(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
			int rate_num, const char **rate_prop_name,
			struct xrisp_clk_ops_t *ops, const char *prop_name)
{
	int i, j;
	struct xrisp_clk_info_t *clk_info;
	struct device_node *np_clk;

	if (priv == NULL) {
		XRISP_PR_ERROR("xrisp clk priv_date ptr is NULL!!!");
		return -EINVAL;
	}

	np_clk = of_get_child_by_name(dev->of_node, prop_name);
	if (!np_clk) {
		XRISP_PR_ERROR("%s node not found\n", prop_name);
		return -EINVAL;
	}

	for (i = 0; i < rate_num; i++) {
		int num = of_property_count_u32_elems(np_clk, rate_prop_name[i]);

		if (clk_num > num) {
			XRISP_PR_ERROR("%s rate number error in dts, rate number = %d, clk num = %d",
				prop_name, num, clk_num);
			return -EINVAL;
		}
	}

	clk_info = devm_kzalloc(dev, sizeof(struct xrisp_clk_info_t) * clk_num, GFP_KERNEL);
	if (clk_info == NULL)
		return -ENOMEM;

	for (i = 0; i < clk_num; i++) {
		if (of_property_read_string_index(np_clk, "clk-names", i, &clk_info[i].clk_name)) {
			XRISP_PR_ERROR("%s clk is NULL", clk_info[i].clk_name);
			return -EINVAL;
		}

		atomic_set(&clk_info[i].clk_enable, 0);
	}

	if (rate_num > 0) {
		for (i = 0; i < clk_num; i++) {
			unsigned int *clk_init_rate =
				devm_kzalloc(dev, sizeof(unsigned int) * rate_num, GFP_KERNEL);

			if (clk_init_rate == NULL)
				return -ENOMEM;

			for (j = 0; j < rate_num; j++)
				if (of_property_read_u32_index(np_clk, rate_prop_name[j], i,
							       &clk_init_rate[j])) {
					XRISP_PR_ERROR("%s rate is NULL", rate_prop_name[j]);
					return -EINVAL;
				}

			clk_info[i].clk_init_rate = clk_init_rate;
		}
	}

	priv->name = prop_name;
	priv->clk_num = clk_num;
	priv->dev = dev;
	priv->clk_info = clk_info;
	priv->ops = ops;

	return 0;
}
