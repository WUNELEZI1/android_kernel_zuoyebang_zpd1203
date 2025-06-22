// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/ktime.h>
#include <linux/sched/clock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include "xrisp_wdt.h"
#include "xrisp_common.h"


#define XRING_EDR_WDT_ID 32

#define EDR_WDT_STEP_SIZE     12
#define PROC_NAME             "xrisp_fw_step"

//panic: 00
//fw_wdt_hang: f1
//fw_timer_hang: f2
#define FW_WDT_HANG          0xf1
#define FW_TIMER_HANG        0xf2

#define MAX_WAIT_TIME        50 //1s

static int wdt_irq = -1;

static atomic_t count = ATOMIC_INIT(0);

void *wdt_buf;


void xring_wdt_work_notify(struct work_struct *work)
{
	uint16_t type;
	uint16_t subtype;
	uint32_t log_info;
	int i;
	u8 *pr_data = NULL;

	if (atomic_read(&rproc_stop) == 1) {
		edr_info("rproc stop, return");
		return;
	}
	if (!wdt_buf) {
		edr_info("ramlog not init, yet");
		return;
	}
	pr_data = wdt_buf;
	edr_info("wdt notify panictype [0x%x] process[0x%x]", *pr_data, *(pr_data+4));
	if (!(*pr_data)) {
		type = EDR_FW_PANIC;
		//subtype = EDR_DATA_ERROR;
		if (*(pr_data+4) == WDT_STAGE_KICK0) {
			type = FW_WDT_HANG;
			edr_err("ISP FW Deadlock or hung up, step_core0=%u step_core1=%u", *(pr_data + 4), *(pr_data + 8));
		}

	} else {
		type = *pr_data;
		if (type == FW_WDT_HANG)
			type = EDR_FW_WDT_HANG;
		else if (type == FW_TIMER_HANG)
			type = EDR_FW_TIMER_HANG;
		else
			type = EDR_FW_PANIC;
	}
	for (i = 0; i < MAX_WAIT_TIME; i++) {
		if (atomic_read(&rproc_stop) == 1) {
			edr_info("rproc stop, exit buffer get");
			break;
		}
		if (*(pr_data + 4) == WDT_STAGE_COREDUMP_END)
			break;
		msleep(20);
	}
	if (i == MAX_WAIT_TIME)
		edr_info("Timeout while waiting for coredump over[0x%x]", *(pr_data + 4));
	else
		edr_info("coredump is complete[0x%x]", *(pr_data + 4));
#if (0)
	if (atomic_read(&rproc_stop) == 1)
		subtype = EDR_DATA_ERROR;
	else
		subtype = *(pr_data + 4);
#endif
	log_info = EDR_RAMLOG | EDR_FW_COREDUMP | EDR_LOGCAT | EDR_KERNEL_KMSG | EDR_OFFLINELOG | EDR_DDR_STATUS;
	//must catch coredump, even zero
	subtype = 8;

	edr_drv_submit_api(type, subtype, log_info, 1, 0);
}

DECLARE_WORK(xring_wdt_work, xring_wdt_work_notify);

irqreturn_t xring_wdt_irq(int irq, void *dev_id)
{
	atomic_inc(&count);
	edr_info("wdt irq %d handle; number[%d]", irq, atomic_read(&count));
	disable_irq_nosync(irq);
	schedule_work(&xring_wdt_work);
	enable_irq(irq);
	return IRQ_HANDLED;
}

static int wdt_proc_show(struct seq_file *m, void *v)
{
	u8 *pr_data = wdt_buf;
	int rowcount = 0;
	int i;

	rowcount = (EDR_WDT_STEP_SIZE / 4) * 4;
	for (i = 0; i < rowcount; i += 4) {
		edr_info("send data: %02x %02x %02x %02x",
			*(pr_data + i), *(pr_data + i + 1), *(pr_data + i + 2),
			*(pr_data + i + 3));
	}
	seq_printf(m, "%pK\n", wdt_buf);
	return 0;
}

static int wdt_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wdt_proc_show, NULL);
}

static const struct proc_ops wdt_proc_ops = {
	.proc_flags = PROC_ENTRY_PERMANENT,
	.proc_read = seq_read,
	.proc_open = wdt_proc_open,
	.proc_release = single_release,
	.proc_lseek = seq_lseek,
};


int xring_edr_wdt_init(void)
{
	struct device_node *np;
	unsigned long irqflags;
	int ret = -1;

	edr_info("wdt init");
	edr_info("wdt addr:0x%pK, size:0x%x\n",
				wdt_buf, EDR_WDT_STEP_SIZE);
	//parse dts node
	np = of_find_node_by_path("/soc/xrisp_cdm");
	if (!np) {
		edr_err("wdt node not found");
		return -EINVAL;
	}
	//get wdt irq id
	wdt_irq = irq_of_parse_and_map(np, XRING_EDR_WDT_ID);
	if (wdt_irq)
		edr_info("get wdt irq num %d", wdt_irq);
	else {
		edr_err("get wdt irq failed, %d", wdt_irq);
		return -EINVAL;
	}
	//register wdt irq
	irqflags = IRQF_TRIGGER_RISING | IRQF_ONESHOT;
	ret = request_threaded_irq(wdt_irq, NULL, xring_wdt_irq,
				   irqflags, "xring_edr_wdt", NULL);
	if (ret) {
		edr_err("register irq[%d] error, ret=%d", wdt_irq, ret);
		return -EINVAL;
	}
	proc_create(PROC_NAME, 0, NULL, &wdt_proc_ops);
	edr_info("xring wdt irq %d register successd", wdt_irq);
	return 0;
}
void xring_edr_wdt_exit(void)
{
	edr_info("wdt exit");
	remove_proc_entry(PROC_NAME, NULL);
	//free wdt irq
	if (wdt_irq != -1)
		free_irq(wdt_irq, NULL);
}

MODULE_AUTHOR("fang jun<fangjun3@xiaomi.com>");
MODULE_DESCRIPTION("x-ring edr wdt");
MODULE_LICENSE("GPL v2");
