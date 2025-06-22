/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __DW_WDT__
#define __DW_WDT__

#include <linux/watchdog.h>

#define WDOG_CONTROL_REG_WDT_EN_MASK	0x01
#define WDOG_CONTROL_REG_RESP_MODE_MASK	0x02
#define WDOG_COUNTER_RESTART_KICK_VALUE	0x76
#define WDT_TORR_MASK	0xf0
#define WDOG_PAUSE	1
#define PAUSE_MASK	0xffff0000
#define WDOG_UNPAUSE	0
#define WDOG_MASK	0
#define WDOG_MASK_VALUE	0xffbf
#define WDOG_UNMASK	1
#define WDOG_UNMASK_SHIFT	6
#define PERI_CRG_SIZE	0x200
#define WDOG_COMP_ENABLE	0
#define RESET	0
#define NORESET	1
#define RESET_MASK	0x2800000
#define CLK_MASK	0x3000000
#define CPU0	0
#define CPU1	1
#define CRG_SHITF	0x100
#define W1C_SHIFT	0x4

#define to_dw_wdt(wdd)    container_of(wdd, struct dw_wdt, wdd)

enum dw_wdt_rmod {
	DW_WDT_RMOD_RESET = 1,
	DW_WDT_RMOD_IRQ = 2
};

enum dw_wdt_timeout {
	C256 = 0,
	C512,
	C1K,
	C2K,
	C4K,
	C8K,
	C16K,
	C32K,
	C64K,
	C128K,
	C256K,
	C512K,
	C1M,
	C2M,
	C4M,
	C8M
};

struct dw_wdt {
	void __iomem	*regs;
	void __iomem	*wdt_pause;
	void __iomem	*wdt_rst_mask;
	void __iomem	*wdt_rst_crg;
	void __iomem	*wdt_clk_crg;
	struct device	*dev;
	u32	peri_wd0_mask;
	u32	pctrl_wdt_debug;
	u32	peri_wdt_clk;
	u32	top_time;
	u32	top_init_time;
	u32	restart_time;
	struct clk	*tclk;
	struct clk	*pclk;
	enum dw_wdt_rmod	rmod;
	struct watchdog_device	wdd;
	struct delayed_work	wdt_delay_wq;
	struct workqueue_struct *wdt_wq;
};

int dw_wdt_set_timeout_ctl(unsigned int timeout);
int dw_wdt_update_mode_ctl(enum dw_wdt_rmod rmod);
int dw_wdt_top_init(u32 timeout);
int dw_wdt_pause(u32 val);
int dw_wdt_rst_set(u32 val);
u32 dw_wdt_ccvr(void);

#endif
