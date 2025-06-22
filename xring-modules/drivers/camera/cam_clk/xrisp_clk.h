/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_CLK_H__
#define __XRISP_CLK_H__

#include "xrisp_clk_api.h"

struct xrisp_clk_info_t {
	const char *clk_name;
	unsigned int *clk_init_rate;
	unsigned int clk_rate;
	struct clk *clk_p;
	atomic_t clk_enable;
};

struct xrisp_clk_data;

struct xrisp_clk_ops_t {
	int (*enable)(struct xrisp_clk_data *priv, unsigned int mask);
	void (*disable)(struct xrisp_clk_data *priv, unsigned int mask);
	unsigned int (*get_stats)(struct xrisp_clk_data *priv);
	int (*set_rate_by_mask)(struct xrisp_clk_data *priv, unsigned int mask, int rate);
	int (*enable_init_rate)(struct xrisp_clk_data *priv, unsigned int mask, int rate);
	void (*deinit)(struct xrisp_clk_data *priv);
	int (*set_rate)(struct xrisp_clk_data *priv, int ch, unsigned int rate);
	unsigned int (*get_rate)(struct xrisp_clk_data *priv, int ch);
	void (*release)(struct xrisp_clk_data *priv);
};

struct xrisp_clk_data {
	struct device *dev;
	const char *name;
	int clk_num;
	struct xrisp_clk_info_t *clk_info;
	struct xrisp_clk_ops_t *ops;
};

struct xrisp_clk_init_data {
	enum xrisp_clk_e clk_id;
	const char *prop_name;
	int clk_num;
	int (*clk_init_func)(struct xrisp_clk_data *priv, struct device *dev, int prop_index,
						 const char *prop_name);
};


extern struct xrisp_clk_data *xrisp_clk_priv_p;

int xrisp_clk_init_priv_data(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
			int rate_num, const char **rate_prop_name,
			struct xrisp_clk_ops_t *ops, const char *prop_name);
void xrisp_clk_release_devs(struct xrisp_clk_data *priv);
int xrisp_clk_enable(struct xrisp_clk_data *priv, unsigned int mask);
unsigned int xrisp_clk_disable(struct xrisp_clk_data *priv, unsigned int mask);
unsigned int xrisp_clk_get_stats(struct xrisp_clk_data *priv);
int xrisp_clk_set_rate_by_mask(struct xrisp_clk_data *priv, unsigned int mask, int rate_index);
int xrisp_clk_enable_init_rate(struct xrisp_clk_data *priv, unsigned int mask, int rate_index);
int xrisp_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate);
unsigned int xrisp_clk_get_rate(struct xrisp_clk_data *priv, int ch);
void xrisp_clk_deinit_clk(struct xrisp_clk_data *priv, int ch, unsigned int rate);

int xrisp_crg_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
					   const char *prop_name);
int xrisp_mcu_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
					   const char *prop_name);
int xrisp_debug_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
						 const char *prop_name);
int xrisp_perf_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
						const char *prop_name);
int clk_debugfs_init(struct xrisp_clk_data *priv);
void clk_debugfs_exit(void);

#endif
