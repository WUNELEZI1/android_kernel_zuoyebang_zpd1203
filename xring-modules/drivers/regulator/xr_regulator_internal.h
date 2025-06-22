/* SPDX-License-Identifier: GPL-2.0-or-later */
/**
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_REGULATOR_INTERNAL_H__
#define __XR_REGULATOR_INTERNAL_H__
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <dt-bindings/xring/platform-specific/ip_regulator_define.h>
#include "soc/xring/xr_regulator_mng.h"
#include <linux/hwspinlock.h>

#define MAX_CLK_NAME_NUM                (8)

#define REGULATOR_EVENT_SET_MODE       0x10000000
#define REGULATOR_EVENT_PRE_ENABLE     0x20000000
#define REGULATOR_EVENT_PRE_HW_ENABLE  0x40000000
#define REGULATOR_EVENT_POST_HW_ENABLE 0x80000000
#define HWSPINLOCK_TIMEOUT_MS (5)

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)     "XR_REGULATOR: " fmt

struct flow_ctrl_info {
	int enable;
	int param;
};

enum xring_regulator_type {
	XR_REGL_TYPE_IP,
	XR_REGL_TYPE_PMIC,
	XR_REGL_TYPE_CNT,
};

struct vote_config {
	struct vote_mng *vote_mng;
	int ch;
	int mode;
	int off_sync;
};

struct xring_regulator_ip {
	const char *name;
	struct device_node *np;
	struct regulator_desc rdesc;
	int (*dt_parse)(struct xring_regulator_ip *sreg, struct platform_device *pdev);
	int regulator_id;
	int regulator_enalbe_flag;
	int clk_cnt;
	unsigned int rate_dis;
	unsigned int rate_en[MAX_CLK_NAME_NUM];
	struct clk *dm_clk[MAX_CLK_NAME_NUM];
	struct vote_config vote_cfg;
	struct flow_ctrl_info flowctrl;
	struct dentry *debugfs;
	u32 off_on_delay;
	u64 last_off_us;
	spinlock_t lock;
	void *gpc_priv;
	struct hwspinlock *hwlock;
};

#define REGULATOR_NORMAL_MODE                            0x0
#define REGULATOR_LOW_POWER_MODE                         0x1

struct xring_regulator_pmic {
	struct regulator_desc rdesc;
	struct device_node *np;
	struct regulator_dev        *rdev;
	struct xr_regulator_manager regulator_mng;
	u32                         sub_regulator_on_delay; // unit: us
	struct dentry *debugfs;
	int (*dt_parse)(struct xring_regulator_pmic *regl, struct platform_device *dev);
	uint32_t step_uV;
	uint32_t max_uV;
	uint32_t min_uV;
	/*
	 * enable parent buckboost fpwm mode when enable this regulator
	 * disable parent buckboost fpwm mode when disable this regulator
	 */
	bool     parent_bob_fpwm_mode_en;
};

int xr_ip_regulator_get_status(int *status, int count);
int xr_regulator_gpc_init(struct device *dev, struct xring_regulator_ip *sreg);
int xr_regulator_gpc_disable(struct device *dev, struct xring_regulator_ip *sreg);
int xr_regulator_gpc_enable(struct device *dev, struct xring_regulator_ip *sreg);
void xr_regulator_gpc_debug(struct device *dev, struct xring_regulator_ip *sreg);
int xr_regulator_ffa_direct_message(struct device *dev, unsigned long fid,
		unsigned long id, unsigned long para, unsigned long *data);

#if IS_ENABLED(CONFIG_XRING_IP_REGULATOR)
int xring_regulator_ip_init(void);
void xring_regulator_ip_exit(void);
#else
static int xring_regulator_ip_init(void) { return 0; }
static void xring_regulator_ip_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_PMIC_REGULATOR)
int xring_regulator_pmic_init(void);
void xring_regulator_pmic_exit(void);
#else
static int xring_regulator_pmic_init(void) { return 0; }
static void xring_regulator_pmic_exit(void) { }
#endif

#endif /* __XR_REGULATOR_INTERNAL_H__ */
