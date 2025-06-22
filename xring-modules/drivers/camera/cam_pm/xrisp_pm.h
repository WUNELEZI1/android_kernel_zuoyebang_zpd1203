/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_PM_H_
#define _XRISP_PM_H_

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/bits.h>
#include <asm/io.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include "xrisp_pm_api.h"
#include "xrisp_log.h"

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif

#define RGLTR_NAME_LEN (30)
#define REG_BIT(x)     (1 << x)
#define BMRW_SET(x)    ((1 << (x + 16)) | (1 << x))
#define BMRW_CLEAR(x)    ((1 << (x + 16)) | (0 << x))

#define CLK_ON		 (true)
#define CLK_OFF		 (false)

struct rgltr_s {
	const char *name;
	struct regulator_dev *rgltr_dev;
	const struct regulator_ops *ops;
	atomic_t rgltr_cnt;
	struct regulator_desc rgltr_desc;
};

struct xrisp_pm_mgnt {
	struct device *pm_dev;
	struct regulator *isp_rgltrs[ISP_REGULATOR_MAX];
	atomic_t rgltr_open_cnt[ISP_REGULATOR_MAX];
	struct mutex rgltr_lock;
	struct mutex ocm_lock;
	bool ocm_link_flag;
	unsigned int ocm_link_count;
	u8 pipe_rgltr_on_set[ISP_REGULATOR_MAX];
};

struct rgltr_reg {
	void __iomem *va;
	uint32_t base;
	uint32_t size;
	char name[32];
};

enum cam_clk_e {
	XR_CLK_CAMF,
	XR_CLK_CAMFU,
	XR_CLK_CAMT,
	XR_CLK_CAMU,
	XR_CLK_CAMUT,
	XR_CLK_CAMW,
	CAM_CLK_NUM_MAX,
};

static inline uint32_t readl_io(void *base_va, uint32_t base_pa, uint32_t reg)
{
	uint32_t val = 0;

	if (likely(base_va))
		val = readl(base_va + reg);
	else {
		XRISP_PR_ERROR("input base_va error\n");
		return 0;
	}

	XRISP_PR_DEBUG("read 0x%x = 0x%x\n", base_pa + reg, val);
	return val;
}

static inline void writel_io(void *base_va, uint32_t base_pa, uint32_t reg, uint32_t val)
{
	if (likely(base_va))
		writel(val, base_va + reg);
	else {
		XRISP_PR_ERROR("input base_va error\n");
		return;
	}

	XRISP_PR_DEBUG("write 0x%x = 0x%x\n", base_pa + reg, val);
}

static inline uint32_t read_field(void *base_va, uint32_t base_pa, uint32_t reg,
				  uint32_t shift, uint32_t width)
{
	return ((readl_io(base_va, base_pa, reg) & (((1 << (width)) - 1) << (shift))) >>
		(shift));
}

static inline int read_field_cmp(void *base_va, uint32_t base_pa, uint32_t expected_val,
				 uint32_t reg, uint32_t shift, uint32_t width)
{
	uint32_t get_val = 0;

	get_val =
		((readl_io(base_va, base_pa, reg) & (((1 << (width)) - 1) << (shift))) >>
		 (shift));

	if (get_val != expected_val) {
		XRISP_PR_ERROR("0x%x read value not match, bit[%d:%d] get 0x%x , exp 0x%x",
		       base_pa + reg, shift, shift + width, get_val, expected_val);
	}
	return 0;
}

static inline void write_bits(void *base_va, uint32_t base_pa, uint32_t reg,
			      uint32_t mask, uint32_t data)
{
	uint32_t tmp;

	tmp = readl_io(base_va, base_pa, reg);
	tmp &= ~mask;
	tmp |= data & mask;
	writel_io(base_va, base_pa, reg, tmp);
}

static inline void xr_delay_us(uint32_t times)
{
	if (times >= 1000)
		mdelay(times / 1000);
	else
		udelay(times);
}

#endif
