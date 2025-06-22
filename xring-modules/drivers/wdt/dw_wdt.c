// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2010-2011 Picochip Ltd., Jamie Iles
 * https://www.picochip.com
 *
 * This file implements a driver for the Synopsys DesignWare watchdog device
 * in the many subsystems. The watchdog has 16 different timeout periods
 * and these are a function of the input clock frequency.
 *
 * The DesignWare watchdog cannot be stopped once it has been started so we
 * do not implement a stop function. The watchdog core will continue to send
 * heartbeat requests after the watchdog device has been closed.
 */

#define pr_fmt(fmt)	"[xr_dfx][wdt]:%s:%d " fmt, __func__, __LINE__

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/reset.h>
#include <linux/workqueue.h>
#include <linux/devm-helpers.h>
#include <linux/sched/debug.h>
#include <linux/kmsg_dump.h>
#include <linux/cpumask.h>
#include <linux/reboot.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/syscore_ops.h>
#include <trace/hooks/gic.h>
#include <soc/xring/dfx_switch.h>
#include <dt-bindings/xring/platform-specific/DW_apb_wdt_header.h>
#include <dt-bindings/xring/platform-specific/peri_crg.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <soc/xring/flush_cache.h>
#include <soc/xring/dfx_bootup.h>

#include "dw_wdt.h"

static struct dw_wdt *old_wdt;
static struct syscore_ops wdt_syscore_ops;

/*******************************************************************************
 * Function:  dw_wdt_update_mode
 *
 * Description:
 * Set the wdt mode, which has two modes: RESET and INTERRUPT
 *
 * Input:
 * @dw_wdt: struct dw_wdt
 * @rmod:   mode selection
 ********************************************************************************/
static void dw_wdt_update_mode(struct dw_wdt *dw_wdt, enum dw_wdt_rmod rmod)
{
	u32 val;

	val = readl(dw_wdt->regs + DW_APB_WDT_CR);
	if (rmod == DW_WDT_RMOD_IRQ)
		val |= WDOG_CONTROL_REG_RESP_MODE_MASK;
	else
		val &= ~WDOG_CONTROL_REG_RESP_MODE_MASK;
	writel(val, dw_wdt->regs + DW_APB_WDT_CR);

	dw_wdt->rmod = rmod;
}

int dw_wdt_update_mode_ctl(enum dw_wdt_rmod rmod)
{
	struct dw_wdt *dw_wdt = old_wdt;

	dev_info(dw_wdt->dev, "%s : rmodval = 0x%x\n", __func__, rmod);

	if (DW_WDT_RMOD_RESET != rmod && DW_WDT_RMOD_IRQ != rmod) {
		dev_err(dw_wdt->dev, "rmod value is invalid\n");
		return -EINVAL;
	}

	dw_wdt_update_mode(dw_wdt, rmod);

	return 0;
}
EXPORT_SYMBOL(dw_wdt_update_mode_ctl);

/*******************************************************************************
 * Function:  dw_wdt_restart
 *
 * Description:
 *   Feed dog function
 *
 * Input:
 *   @wdd: struct watchdog_device
 ********************************************************************************/
static void dw_wdt_restart(struct watchdog_device *wdd)
{
	struct dw_wdt *dw_wdt = to_dw_wdt(wdd);

	writel(WDOG_COUNTER_RESTART_KICK_VALUE, dw_wdt->regs + DW_APB_WDT_CRR);
	dev_err(dw_wdt->dev, "feed wdt\n");
}

/*******************************************************************************
 * Function: dw_wdt_set_timeout
 *
 * Description:
 *   Set timeout
 *
 * Input:
 *   @wdd: struct watchdog_device
 *   @timeout
 ********************************************************************************/
/*
 * WDT Timeout Range Register
 *  - timeout period define:
 *     --> 8/9/a/b:  64K/128K/256K/512K cnt
 *     --> c/d/e/f:  1M/2M/4M/8M cnt
 *     --> 7/8/9/a:  1s/2s/4s/8s
 *     --> b/c/d/e:  16s/32s/64s/128s
 */
static int dw_wdt_set_timeout(struct dw_wdt *dw_wdt, enum dw_wdt_timeout timeout)
{
	u32 val;

	if (timeout < C256 || timeout > C8M) {
		dev_err(dw_wdt->dev, "top value is invalid\n");
		return -EINVAL;
	}

	val = readl(dw_wdt->regs + DW_APB_WDT_TORR);
	val &= WDT_TORR_MASK;
	val |= timeout;

	writel(val, dw_wdt->regs + DW_APB_WDT_TORR);

	return 0;
}

int dw_wdt_top_init(enum dw_wdt_timeout timeout)
{
	struct dw_wdt *dw_wdt = old_wdt;
	u32 val;

	dev_info(dw_wdt->dev, "%s : timeoutval = 0x%x\n", __func__, timeout);
	if (timeout < C256 || timeout > C8M) {
		dev_err(dw_wdt->dev, "top init value is invalid\n");
		return -EINVAL;
	}

	val = readl(dw_wdt->regs + DW_APB_WDT_TORR);
	val |= (timeout << DW_APB_WDT_TORR_TOP_INIT_BITADDRESSOFFSET);
	writel(val, dw_wdt->regs + DW_APB_WDT_TORR);

	return 0;
}
EXPORT_SYMBOL(dw_wdt_top_init);

/*******************************************************************************
 * Function: dw_wdt_set_timeout_ctl
 *
 * Description:
 *   Set timeout external interface
 *
 * Input:
 *   @timeout
 ********************************************************************************/
int dw_wdt_set_timeout_ctl(enum dw_wdt_timeout timeout)
{
	struct dw_wdt *dw_wdt = old_wdt;

	if (timeout < C256 || timeout > C8M) {
		dev_err(dw_wdt->dev, "timeout value is invalid\n");
		return -EINVAL;
	}

	dw_wdt_set_timeout(dw_wdt, timeout);

	return 0;
}
EXPORT_SYMBOL(dw_wdt_set_timeout_ctl);

/*******************************************************************************
 * Function: restart_work_fn
 *
 * Description:
 *   Feeding dogs in the work queue
 *
 * Input:
 *   @work: struct work_struct
 ********************************************************************************/
static void restart_work_fn(struct work_struct *work)
{
	struct dw_wdt *dw_wdt = container_of(work, struct dw_wdt, wdt_delay_wq.work);

	if (cpu_online(CPU0))
		queue_delayed_work_on(CPU0, dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));
	else
		queue_delayed_work(dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));
	dw_wdt_restart(&dw_wdt->wdd);
}

/*******************************************************************************
 * Function: dw_wdt_enable
 *
 * Description:
 *   Enable wdt
 *
 * Input:
 *   @dw_wdt: struct dw_wdt
 ********************************************************************************/
static void dw_wdt_enable(struct dw_wdt *dw_wdt)
{
	u32 val = readl(dw_wdt->regs + DW_APB_WDT_CR);

	/* Enable watchdog. */
	val |= WDOG_CONTROL_REG_WDT_EN_MASK;
	writel(val, dw_wdt->regs + DW_APB_WDT_CR);
}

/*******************************************************************************
 * Function: dw_wdt_start
 *
 * Description:
 *   Start wdt
 *
 * Input:
 *   @wdd: struct watchdog_device
 ********************************************************************************/
static void dw_wdt_start(struct watchdog_device *wdd)
{
	struct dw_wdt *dw_wdt = to_dw_wdt(wdd);

	dw_wdt_enable(dw_wdt);
	dw_wdt_pause(WDOG_UNPAUSE);
}

void dw_wdt_start_ctl(void)
{
	struct dw_wdt *dw_wdt = old_wdt;

	dw_wdt_start(&dw_wdt->wdd);
}
EXPORT_SYMBOL(dw_wdt_start_ctl);

void dw_wdt_clear_irq(void)
{
	struct dw_wdt *dw_wdt = old_wdt;

	readl(dw_wdt->regs + DW_APB_WDT_EOI);
}

int dw_wdt_pause(u32 val)
{
	struct dw_wdt *dw_wdt = old_wdt;
	u32 wdt_debug_val;

	if (val != WDOG_PAUSE && val != WDOG_UNPAUSE) {
		dev_err(dw_wdt->dev, "pause value is invalid\n");
		return -EINVAL;
	}

	dev_info(dw_wdt->dev, "val = 0x%x\n", val);
	wdt_debug_val = readl(dw_wdt->wdt_pause);
	wdt_debug_val |= PAUSE_MASK;

	if (val == WDOG_PAUSE)
		wdt_debug_val |= val;

	if (val == WDOG_UNPAUSE)
		wdt_debug_val &= PAUSE_MASK;

	dev_info(dw_wdt->dev, "wdt_debug_val = 0x%x\n", wdt_debug_val);
	writel(wdt_debug_val, dw_wdt->wdt_pause);

	return 0;
}
EXPORT_SYMBOL(dw_wdt_pause);

/* wdt ip reset */
int dw_wdt_reset_crg(u32 reset_val)
{
	struct dw_wdt *dw_wdt = old_wdt;

	dev_info(dw_wdt->dev, "%s : resetval = 0x%x\n", __func__, reset_val);

	if (reset_val != RESET && reset_val != NORESET) {
		dev_err(dw_wdt->dev, "reset value is invalid\n");
		return -EINVAL;
	}

	if (reset_val == RESET)
		writel(RESET_MASK, dw_wdt->wdt_rst_crg + PERI_CRG_RST0_W1C);

	if (reset_val == NORESET)
		writel(RESET_MASK, dw_wdt->wdt_rst_crg + PERI_CRG_RST0_W1S);

	return 0;
}

static void dw_wdt_clk(struct dw_wdt *dw_wdt)
{
	writel(CLK_MASK, dw_wdt->wdt_clk_crg + W1C_SHIFT);
	writel(CLK_MASK, dw_wdt->wdt_clk_crg);
}

u32 dw_wdt_ccvr(void)
{
	struct dw_wdt *dw_wdt = old_wdt;

	return readl(dw_wdt->regs + DW_APB_WDT_CCVR);
}
EXPORT_SYMBOL(dw_wdt_ccvr);

/*******************************************************************************
 * Function: dw_wdt_irq
 *
 * Description:
 * Interrupt handling function
 *
 * Input:
 *   @irq: interrupt number
 *   @devid: dw_wdt
 ********************************************************************************/
static irqreturn_t dw_wdt_irq(int irq, void *devid)
{
	struct dw_wdt *dw_wdt = devid;
	u32 val;

	dev_err(dw_wdt->dev, "wdt timeout !!!\n");
	dw_wdt_restart(&dw_wdt->wdd);
	/*
	 * We don't clear the IRQ status. It's supposed to be done by the
	 * following ping operations.
	 */
	val = readl(dw_wdt->regs + DW_APB_WDT_STAT);
	if (!val)
		return IRQ_NONE;

	return IRQ_HANDLED;
}

static int wdt_dt_parse(struct device_node *np, struct dw_wdt *dw_wdt)
{
	int ret;

	ret = of_property_read_u32(np, "pctrl_wdt_debug", &dw_wdt->pctrl_wdt_debug);
	if (ret) {
		dev_err(dw_wdt->dev, "Failed to get pctrl_wdt_debug.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "peri_wdt_clk", &dw_wdt->peri_wdt_clk);
	if (ret) {
		dev_err(dw_wdt->dev, "Failed to get peri_wdt_clk.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "top_time", &dw_wdt->top_time);
	if (ret) {
		dev_err(dw_wdt->dev, "Failed to get top_time.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "top_init_time", &dw_wdt->top_init_time);
	if (ret) {
		dev_err(dw_wdt->dev, "Failed to get top_init_time.\n");
		return ret;
	}

	ret = of_property_read_u32(np, "restart_time", &dw_wdt->restart_time);
	if (ret) {
		dev_err(dw_wdt->dev, "Failed to get restart_time.\n");
		return ret;
	}

	return 0;
}

void show_state_filter_wdt(unsigned long state_filter)
{
	struct task_struct *g, *p;

	rcu_read_lock();
	for_each_process_thread(g, p) {
		/*
		 * reset the NMI-timeout, listing all files on a slow
		 * console might take a lot of time:
		 * Also, reset softlockup watchdogs on all CPUs, because
		 * another CPU might be blocked waiting for us to process
		 * an IPI.
		 */
		//touch_nmi_watchdog();
		if (p->__state == state_filter)
			sched_show_task(p);
	}
	rcu_read_unlock();
}

void regs_dump(void)
{
	pr_err("wdt fiq regs dump  !!!\n");
}

static void android_rvh_fiq_dump_handler(void *unused, struct pt_regs *regs)
{
	u8 reboot_reason;

	pr_err("wdt fiq dump start!!!\n");

	reboot_reason = get_reboot_reason();
	pr_err("reboot reason is 0x%x\n", reboot_reason);

	set_subtype_exception(APWDT_AP, true);

	regs_dump();
	show_regs(regs);
	dump_stack();
	show_state_filter_wdt(TASK_UNINTERRUPTIBLE);
	flush_cache_all_by_setway();
	flush_dfx_cache();

	panic("acpu wdt fiq");
}

/*******************************************************************************
 * Function: dw_wdt_drv_probe
 *
 * Description:
 *   Probe function
 *
 * Input:
 *   @pdev: struct platform_device
 ********************************************************************************/
static int dw_wdt_drv_probe(struct platform_device *pdev)
{
	struct dw_wdt *dw_wdt = NULL;
	int ret;

	dev_info(&pdev->dev, "probe\n");

	dw_wdt = devm_kzalloc(&pdev->dev, sizeof(*dw_wdt), GFP_KERNEL);
	if (!dw_wdt)
		return -ENOMEM;

	dw_wdt->dev = &pdev->dev;

	old_wdt = dw_wdt;

	ret = wdt_dt_parse(pdev->dev.of_node, dw_wdt);
	if (ret) {
		dev_err(&pdev->dev, "failed to assign device tree parsing\n");
		return ret;
	}

	dw_wdt->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(dw_wdt->regs)) {
		dev_err(&pdev->dev, "failed to claim register region\n");
		ret = PTR_ERR(dw_wdt->regs);
		return ret;
	}

	dw_wdt->wdt_pause = devm_ioremap(&pdev->dev, dw_wdt->pctrl_wdt_debug, sizeof(u32));
	if (!dw_wdt->wdt_pause) {
		dev_err(&pdev->dev, "Cannot map device memory\n");
		return -ENOMEM;
	}

	dw_wdt->wdt_clk_crg = devm_ioremap(&pdev->dev, dw_wdt->peri_wdt_clk, sizeof(u32));
	if (!dw_wdt->wdt_clk_crg) {
		dev_err(&pdev->dev, "Cannot map device memory\n");
		return -ENOMEM;
	}

	dw_wdt->wdt_rst_crg = devm_ioremap(&pdev->dev, ACPU_PERI_CRG, PERI_CRG_SIZE);
	if (!dw_wdt->wdt_rst_crg) {
		dev_err(&pdev->dev, "Cannot map device memory\n");
		return -ENOMEM;
	}

	ret = dw_wdt_reset_crg(RESET);
	if (ret) {
		dev_err(&pdev->dev, "reset value is invalid\n");
		return -EINVAL;
	}

	udelay(200);

	ret = dw_wdt_reset_crg(NORESET);
	if (ret) {
		dev_err(&pdev->dev, "reset value is invalid\n");
		return -EINVAL;
	}

	dw_wdt_clk(dw_wdt);

	ret = dw_wdt_top_init(dw_wdt->top_init_time);
	if (ret) {
		dev_err(&pdev->dev, "top init value is invalid\n");
		return -EINVAL;
	}

	ret = dw_wdt_set_timeout(dw_wdt, dw_wdt->top_time);
	if (ret) {
		dev_err(&pdev->dev, "top value is invalid\n");
		return -EINVAL;
	}

	dw_wdt_update_mode(dw_wdt, DW_WDT_RMOD_IRQ);

	ret = platform_get_irq_optional(pdev, 0);

	if (ret > 0) {
		ret = devm_request_irq(&pdev->dev, ret, dw_wdt_irq, IRQF_SHARED, pdev->name,
		      dw_wdt);

		if (ret) {
			dev_err(&pdev->dev, "failed to request irq\n");
			return ret;
		}
	} else {
		dev_err(&pdev->dev, "No IRQ for wdt\n");
		return ret;
	}

	ret = register_trace_android_rvh_fiq_dump(android_rvh_fiq_dump_handler, NULL);
	if (ret) {
		dev_err(&pdev->dev, "register_trace_android_vh_fiq_dump filed(ret = %d)\n", ret);
		return ret;
	}

	INIT_DELAYED_WORK(&dw_wdt->wdt_delay_wq, restart_work_fn);

#ifndef CONFIG_XRING_WDT_DISABLE
	if (is_switch_on(SWITCH_AP_WDT))
		dw_wdt_start(&dw_wdt->wdd);
#endif

	dw_wdt->wdt_wq = alloc_workqueue("wdt_wq", WQ_MEM_RECLAIM | WQ_HIGHPRI, 1);
	if (dw_wdt->wdt_wq == NULL) {
		dev_err(&pdev->dev, "alloc workqueue failed\n");
		ret = -ENOMEM;
		return ret;
	}

	if (cpu_online(CPU0))
		queue_delayed_work_on(CPU0, dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));
	else
		queue_delayed_work(dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));

	register_syscore_ops(&wdt_syscore_ops);

	platform_set_drvdata(pdev, dw_wdt);

	ret = set_bootup_status(STAGE_KERNEL_WDT_INIT_OK);
	if (ret)
		dev_err(&pdev->dev, "set bootup keypoint[%u] failed\n", STAGE_KERNEL_WDT_INIT_OK);

	return 0;

}

/*******************************************************************************
 * Function: dw_wdt_drv_remove
 *
 * Description:
 *   remove function
 *
 * Input:
 *   @pdev: struct platform_device
 ********************************************************************************/
static int dw_wdt_drv_remove(struct platform_device *pdev)
{
	struct dw_wdt *dw_wdt = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "remove\n");

	unregister_syscore_ops(&wdt_syscore_ops);
	cancel_delayed_work_sync(&dw_wdt->wdt_delay_wq);
	devm_kfree(&pdev->dev, dw_wdt);
	old_wdt = NULL;

	return 0;
}

void wdt_cancel_work(void)
{
	struct dw_wdt *dw_wdt = old_wdt;

	cancel_delayed_work(&dw_wdt->wdt_delay_wq);
}

static int wdt_suspend(void)
{
	pr_info("%s +\n", __func__);

	wdt_cancel_work();

	pr_info("%s -\n", __func__);

	return 0;
}

static void wdt_resume(void)
{
	struct dw_wdt *dw_wdt = old_wdt;

	pr_info("%s +\n", __func__);

	INIT_DELAYED_WORK(&dw_wdt->wdt_delay_wq, restart_work_fn);

	if (cpu_online(CPU0))
		queue_delayed_work_on(CPU0, dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));
	else
		queue_delayed_work(dw_wdt->wdt_wq, &dw_wdt->wdt_delay_wq,
			msecs_to_jiffies(dw_wdt->restart_time));

	pr_info("%s -\n", __func__);
}

static struct syscore_ops wdt_syscore_ops = {
	.suspend = wdt_suspend,
	.resume = wdt_resume
};

static const struct of_device_id dw_wdt_of_match[] = {
	{ .compatible = "snps,dw-wdt", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dw_wdt_of_match);

static struct platform_driver dw_wdt_driver = {
	.probe     = dw_wdt_drv_probe,
	.remove    = dw_wdt_drv_remove,
	.driver    = {
		.name  = "dw_wdt",
		.of_match_table = of_match_ptr(dw_wdt_of_match),
	},
};

module_platform_driver(dw_wdt_driver);

MODULE_AUTHOR("Ren Jie");
MODULE_DESCRIPTION("Synopsys DesignWare Watchdog Driver");
MODULE_LICENSE("GPL");
