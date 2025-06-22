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
#ifndef __PINCTRL_XR_H__
#define __PINCTRL_XR_H__

#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/clk.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt)		"XR_PINCTRL: " fmt

#define PIN_CONFIG_XRING_IRQ_MASK_INTERNEL	(PIN_CONFIG_END + 1)

/**
 * DW_APB_GPIO Definitions
 */
#define DWAPB_DRIVER_NAME	"gpio-dwapb"
#define DWAPB_MAX_GPIOS		32
#define DWAPB_NR_CLOCKS		2

#define MAX_NR_GPIO_CHIP	16
#define MAX_NR_TILES		12
#define GPIO_HW_LOCK_TIMEOUT_MS	10
#define HWSPINLOCK_RETRY_TIMES	5

struct pinctrl_pin_desc;
extern u32 xr_gpio_doze_flag;

enum xr_pad_type {
	XR_PAD_TSMC_1P2 = 0,
	TSMC_IO8D_V = 0,
	TSMC_IO8D_H = 0,
	TSMC_ISD4_V = 0,
	TSMC_PTESTMODE_V = 0,
	XR_PAD_SNSP_1P8 = 1,
	SNSP_IO_V = 1,
	SNSP_IO_H = 1,
};


/**
 * struct xr_function - a pinmux function
 * @name:    Name of the pinmux function.
 * @groups:  List of pingroups for this function.
 * @ngroups: Number of entries in @groups.
 */
struct xr_function {
	const char *name;
	const char * const *groups;
	unsigned int ngroups;
};

/**
 * struct xr_pingroup - XRing pingroup definition
 * @name:                 Name of the pingroup.
 * @pins:	          A list of pins assigned to this pingroup.
 * @npins:	          Number of entries in @pins.
 * @funcs:                A list of pinmux functions that can be selected for
 *                        this group. The index of the selected function is used
 *                        for programming the function selector.
 *                        Entries should be indices into the groups list of the
 *                        struct xr_pinctrl_soc_data.
 */
struct xr_pingroup {
	const char *name;
	unsigned int pad_type;

	const unsigned int *pins;
	unsigned int npins;

	unsigned int *funcs;
	unsigned int nfuncs;

	unsigned int cfg_reg;
	unsigned int mux_reg;

	unsigned int cfg_tile:4;
	unsigned int mux_tile:4;

	unsigned int drv_bit:5;
	unsigned int pull_bit:5;
	unsigned int st_bit:5;
	unsigned int sl_bit:5;
	unsigned int mux_bit:5;
};

/**
 * struct xr_pinctrl_soc_data - XRing pin controller driver configuration
 * @pins:	    An array describing all pins the pin controller affects.
 * @npins:	    The number of entries in @pins.
 * @functions:	    An array describing all mux functions the SoC supports.
 * @nfunctions:	    The number of entries in @functions.
 * @groups:	    An array describing all pin groups the pin SoC supports.
 * @ngroups:	    The numbmer of entries in @groups.
 * @ngpio:	    The number of pingroups the driver should expose as GPIOs.
 * @pull_no_keeper: The SoC does not support keeper bias.
 * @gpio_func: Which function number is GPIO (usually 0).
 */
struct xr_pinctrl_soc_data {
	const struct pinctrl_pin_desc *pins;
	unsigned int npins;
	const struct xr_function *functions;
	unsigned int nfunctions;
	const struct xr_pingroup *groups;
	unsigned int ngroups;
	unsigned int ngpios;
	bool pull_no_keeper;
	const char *const *tiles;
	unsigned int ntiles;
	unsigned int gpio_func;
};

struct dwapb_gpio {
	struct device *dev;
	struct fwnode_handle *fwnode;
	struct xr_pinctrl *xr_pctrl;
	struct hwspinlock *hwlock;
	struct gpio_chip gc;
	struct irq_chip irq_chip;
	int irq;
	unsigned int flags;
	u32 doze_function;
	void __iomem *base_reg;
	void __iomem *extmask_reg;
	unsigned int extmask_enabled;
	struct reset_control *rst;
	struct clk_bulk_data clks[DWAPB_NR_CLOCKS];
};

/**
 * struct xr_pinctrl - state for a pinctrl-xr device
 * @dev:            device handle.
 * @pctrl:          pinctrl handle.
 * @desc:           pin controller descriptor
 * @lock:           Spinlock to protect register resources as well
 *                  as xr_pinctrl data structures.
 * @soc:            Reference to soc_data of platform specific data.
 * @regs:           Base addresses for the TLMM tiles.
 * @phys_base:      Physical base address
 */
struct xr_pinctrl {
	struct device *dev;
	struct pinctrl_dev *pctrl;
	struct pinctrl_desc desc;

	raw_spinlock_t lock;

	struct dwapb_gpio dw_gpios[MAX_NR_GPIO_CHIP];
	unsigned int n_dwgpios;

	const struct xr_pinctrl_soc_data *soc;
	void __iomem *regs[MAX_NR_TILES];
	u32 phys_base[MAX_NR_TILES];
	u32 ioctl_non_secure_size;
	u32 fpga_debug;
};
int xr_readl_cfg(struct xr_pinctrl *pctrl,
			const struct xr_pingroup *g, unsigned int group, u32 *val);
int xr_readl_mux(struct xr_pinctrl *pctrl,
			const struct xr_pingroup *g, unsigned int group, u32 *val);
u32 xr_readl_gpio(struct dwapb_gpio *gc, unsigned int offset);
int xr_gpio_offset_to_pin(struct gpio_chip *chip, unsigned int offset);
int xr_pinctrl_probe(struct platform_device *pdev,
		      const struct xr_pinctrl_soc_data *soc_data);
#endif
