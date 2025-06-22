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

#ifndef __XR_CLK_COMON_H__
#define __XR_CLK_COMON_H__

#include <dt-bindings/xring/xr-clock-base.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#ifndef CLK_LOG_TAG
#define CLK_LOG_TAG "clock"
#endif

#define clkerr(fmt, ...)                                                       \
	pr_err("[%s]%s: " fmt, CLK_LOG_TAG, __func__, ##__VA_ARGS__)

#define clkwarn(fmt, ...)                                                      \
	pr_warn("[%s]%s: " fmt, CLK_LOG_TAG, __func__, ##__VA_ARGS__)

#define clkinfo(fmt, ...)                                                      \
	pr_info("[%s]%s: " fmt, CLK_LOG_TAG, __func__, ##__VA_ARGS__)

#define DTS_REG_ATTRIBUTE_NUM	4
#define PLAT_FPGA	1

#define MUX_WAIT_ACK_TIME_OUT_US 100
#define DIV_WAIT_ACK_TIME_OUT_US 100

#define CLK_VOTE_MEDIA_ON  true
#define CLK_VOTE_MEDIA_OFF false

#ifdef CLK_R_W_DEBUG
static inline uint32_t clkread(void __iomem *addr)
{
	uint32_t ret = readl(addr);

	clkerr("R: addr 0x%x  data 0x%x\n", addr, ret);
	return ret;
}

static inline void clkwrite(uint32_t data, void __iomem *addr)
{
	clkerr("W: addr 0x%x  data 0x%x\n", addr, data);
	writel(data, addr);
}
#else
static inline uint32_t clkread(void __iomem *addr)
{
	return readl(addr);
}

static inline void clkwrite(uint32_t data, void __iomem *addr)
{
	writel(data, addr);
}
#endif

/*
 * for dts reg property,like:
 * xring,xxx-reg = <0x10 0x3 5>;
 */
enum {
	ADDR_OFFSET,
	ADDR_MASK,
	ADDR_SHIFT,
	ADDR_CFG_MAX,
};

/*
 * for sw/div/gate array element,like:
 * clksw_cfg[SW_DIV_CFG_CNT];
 */
enum {
	CFG_OFFSET,
	CFG_SHIFT,
	CFG_LENGTH,
};


spinlock_t *get_xr_clk_lock(void);

struct clk_hw_onecell_data **xr_clk_hw_data_get(void);
struct clk_hw *xr_clk_hw_get(unsigned int clk_id);
int xr_check_clk_hws(struct clk_hw *clks[], unsigned int count);
void xr_unregister_hw_clocks(struct clk_hw *hws[], unsigned int count);

int crg_vote_media_power(int crg_idx, bool on_off);
int get_media_power_status(int crg_idx);
void media_power_votemng_init(struct device *dev, int crg_idx);
void media_power_votemng_exit(void);
void clk_feature_state_init(void);
void __iomem *get_xr_clk_base(unsigned int id);
int crg_base_address_init(struct platform_device *pdev, unsigned long type);

int is_fpga(void);
bool is_peri_dvfs_ft_on(void);
bool is_media_dvfs_ft_on(void);
bool is_peri_avs_ft_on(void);
bool is_media_avs_ft_on(void);
void parse_pciepll_vco_rate(const char * const *compt_string, int num);
int get_pciepll_vco_rate(void);
#endif /* __XR_CLK_COMON_H__ */
