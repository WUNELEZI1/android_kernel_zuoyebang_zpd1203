/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef __XR_CLK_H__
#define __XR_CLK_H__

#include <linux/clk-provider.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include "xr-clk-common.h"
#include "xr-clk-dvfs.h"
#include "xr-clk-fast-dfs.h"
#include "xr-clk-fixed-rate.h"
#include "xr-clk-gate.h"
#include "xr-clk-pll.h"
#include "xr-clk-scgate.h"
#include "xr-clk-divider.h"
#include "xr-clk-mux.h"
#include "xr-clk-pmu.h"

#define ALWAYS_ON                    1
#define CLK_SCGATE_ALWAYS_ON_MASK    BIT(3)
#define HIMASK_OFFSET                16

/* must set dvs bypass after clock close */
#define CLK_DVS_BYPASS_SET      BIT(0)

#define CLK_GATE_DOZE_VOTE      BIT(2)
#define CLK_GATE_SAFE_SET       BIT(3)
#define CLK_DIVIDER_SAFE_SET    BIT(8)
#define CLK_DYNAMIC_POFF_AREA   BIT(9)

#define get_friend_clk(_hw)     (_hw ? ((*_hw)->clk) : NULL)
#define bitmask(width)          ((1 << (width)) - 1)

#define himask_enable(shift)    (0x10001 << (shift))
#define himask_disable(shift)   (0x10000 << (shift))
#define himask_set(shift, width, val) \
		((bitmask(width) << (16 + (shift))) | ((val) << (shift)))


static inline struct clk_hw *devm_xr_clk_hw_fixed(struct device *dev, const char *name, int rate)
{
	return devm_xr_clk_hw_fixed_rate(dev, NULL, name, NULL, NULL, 0, rate, 0);
}

static inline struct clk_hw *devm_xr_clk_hw_ppll(struct device *dev, const char *name,
						const char *parent_name,
						const struct clk_pll_cfg_table *cfg,
						const struct clk_pll_rate_table *freq_table,
						unsigned int num_entry,
						unsigned char safe_id,
						unsigned char pll_flags)
{
	return devm_xr_clk_hw_pll(dev, name, parent_name, cfg, freq_table,
				num_entry, safe_id, pll_flags);
}

static inline struct clk_hw *devm_xr_clk_hw_mux(struct device *dev, const char *name,
						void __iomem *reg, u8 shift, u8 width,
						u16 clk_mux_flags,
						const char * const *parents,
						int num_parents, void __iomem *ack_reg,
						u8 ack_shift, u8 ack_width)
{
	spinlock_t *lock = get_xr_clk_lock();

	/* CLK_OPS_PARENT_ENABLE: switch need parents enabled before change source clk */
	return devm_xr_clk_hw_register_mux(dev, name, num_parents,
				parents, CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE, reg, shift,
				bitmask(width), clk_mux_flags,
				ack_reg, ack_shift, ack_width, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_fixed_factor(struct device *dev, const char *name,
						const char *parent,
						unsigned int mult,
						unsigned int div)
{
	return devm_clk_hw_register_fixed_factor(dev, name, parent,
				CLK_SET_RATE_PARENT, mult, div);
}

static inline struct clk_hw *devm_xr_clk_hw_divider(struct device *dev, const char *name,
						const char *parent,
						void __iomem *reg, u8 shift, u8 width,
						void __iomem *ack_reg, u8 ack_shift,
						u32 clk_divider_flags)
{
	spinlock_t *lock = get_xr_clk_lock();

	/* CLK_OPS_PARENT_ENABLE for div_done signal */
	return	devm_xr_clk_hw_register_divider(dev, name, parent,
				CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
				reg, shift, width, clk_divider_flags,
				ack_reg, ack_shift, 0, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_divider_with_flags(struct device *dev, const char *name,
						const char *parent, u32 flags,
						void __iomem *reg, u8 shift, u8 width,
						void __iomem *ack_reg, u8 ack_shift,
						u32 clk_divider_flags)
{
	spinlock_t *lock = get_xr_clk_lock();

	return	devm_xr_clk_hw_register_divider(dev, name, parent,
				flags, reg, shift, width, clk_divider_flags,
				ack_reg, ack_shift, 0, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_sec_div(struct device *dev, const char *name,
						const char *parent, u8 safe_id, u8 width)
{
	spinlock_t *lock = get_xr_clk_lock();

	/* CLK_OPS_PARENT_ENABLE for div_done signal */
	return	devm_xr_clk_hw_register_divider(dev, name, parent,
				CLK_SET_RATE_PARENT | CLK_OPS_PARENT_ENABLE,
				NULL, 0, width, CLK_DIVIDER_SAFE_SET, NULL, 0, safe_id, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_scgt(struct device *dev, const char *name,
						const char *parent,
						void __iomem *reg, u8 bit_idx,
						u8 clk_gate_flags)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_scgate(dev, NULL, name, parent, NULL, NULL,
				CLK_SET_RATE_PARENT, reg, bit_idx, clk_gate_flags,
				lock);
}

static inline struct clk_hw *devm_xr_clk_hw_gt(struct device *dev, const char *name,
				const char *parent, struct clk_hw **friend_hw,
				void __iomem *reg, unsigned int offset,
				unsigned int bit_shift, unsigned int always_on,
				unsigned int sync_time)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_gate(dev, name, parent, friend_hw, CLK_SET_RATE_PARENT,
				reg, offset, (1UL << bit_shift), always_on, sync_time,
				lock, 0, 0, 0, 0);
}

static inline struct clk_hw *devm_xr_clk_hw_safe_gt(struct device *dev, const char *name,
				const char *parent, struct clk_hw **friend_hw,
				unsigned int safe_id, unsigned int always_on)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_gate(dev, name, parent, friend_hw, CLK_SET_RATE_PARENT,
				NULL, 0, 0, always_on, 0, lock, CLK_GATE_SAFE_SET, safe_id, 0, 0);
}

static inline struct clk_hw *devm_xr_clk_doze_gt(struct device *dev, const char *name,
				const char *parent, struct clk_hw **friend_hw,
				void __iomem *reg, unsigned int offset,
				unsigned int bit_shift, unsigned int always_on,
				unsigned int sync_time, unsigned int vote_id)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_gate(dev, name, parent, friend_hw, CLK_SET_RATE_PARENT,
				reg, offset, (1UL << bit_shift), always_on, sync_time,
				lock, CLK_GATE_DOZE_VOTE, 0, vote_id, 0);
}

static inline struct clk_hw *devm_xr_clk_hw_dvfs_gt(struct device *dev, const char *name,
				const char *parent, struct clk_hw **friend_hw,
				void __iomem *reg, unsigned int offset,
				unsigned int bit_shift, unsigned int always_on,
				unsigned int sync_time, unsigned int dvs_flag)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_gate(dev, name, parent, friend_hw, CLK_SET_RATE_PARENT,
				reg, offset, 0, always_on, sync_time,
				lock, CLK_DVS_BYPASS_SET, 0, 0, dvs_flag);
}

static inline struct clk_hw *devm_xr_clk_hw_pmu(struct device *dev, const char *name,
				const char *parent, unsigned int offset, unsigned int bit_shift,
				unsigned int always_on)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_clk_hw_pmu_clock(dev, name, parent, 0,
				offset, bit_shift, always_on, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_fast_dfs(struct device *dev, void __iomem *base,
				const char *name, unsigned int always_on,
				struct fast_dfs_clock *fast_dfs_cfg)
{
	spinlock_t *lock = get_xr_clk_lock();

	return devm_xr_hw_fast_dfs_clock(dev, base, name, fast_dfs_cfg, always_on, lock);
}

static inline struct clk_hw *devm_xr_clk_hw_dvfs(struct device *dev, const char *name,
		struct clk_hw **friend_hw, unsigned int vote_id,
		unsigned int vote_type, const struct dvfs_cfg *dvfs_cfg)
{
	return devm_xr_clk_hw_dvfs_clock(dev, name, friend_hw, vote_id,
				vote_type, dvfs_cfg);
}

#endif /* __XR_CLK_H__ */
