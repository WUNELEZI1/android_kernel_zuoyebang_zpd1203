// SPDX-License-Identifier: GPL-2.0
/****************************************************************************
 *
 *    The MIT License (MIT)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a
 *    copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *    DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 *    The GPL License (GPL)
 *
 *    Copyright (C) 2020  VeriSilicon Microelectronics Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *****************************************************************************
 *
 *    Note: This software is released under dual MIT and GPL licenses. A
 *    recipient may use this file under the terms of either the MIT license or
 *    GPL License. If you wish to use only one license not the other, you can
 *    indicate your decision by deleting one of the above license notices
 *    in your version of this file.
 *
 *****************************************************************************
 */

#include <linux/ioctl.h>
#include <linux/types.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include "hantroabnormalirq.h"
#include "vdec_trace.h"

extern int vsi_dumplvl;

extern void process_vcd_slice_outbuf(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj);

/**
 * process vcd pp line-counter irq
 */
static void process_vcd_line_cnt_irq(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
									struct cmdbuf_obj *obj, u32 irq)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;
	volatile void *hwregs;

	trace_process_vcd_line_cnt_irq("[dec] line-counter interrupt: cmdbuf ", obj->cmdbuf_id);
	vcmd_wfd_trace("line-counter interrupt: cmdbuf[%d]\n", obj->cmdbuf_id);

	hwregs = subsys->hwregs[SUB_MOD_MAIN];
	irq &= IRQ_BIT_MASK(VCD_STATUS_LINE_CNT_INT);
	iowrite32(irq, (void __iomem *)(hwregs + 4*1));
	process_vcd_slice_outbuf(vcmd_mgr, dev, obj);
	vcmd_klog(LOGLVL_INFO, "VCD has a line-counter interrupt.\n");
}

/**
 * process vcd bus-err abnormal irq
 */
static void process_vcd_bus_err_irq(struct hantrovcmd_dev *dev, volatile void *hwregs, u32 irq)
{
	irq &= IRQ_BIT_MASK(VCD_STATUS_BUS_ERR_INT);
	iowrite32(irq, (void __iomem *)(hwregs + 4*1));

	vcmd_klog(LOGLVL_WARNING, "VCD has a bus err interrupt.\n");
	if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
		dump_brief_regs(dev, DUMP_STATE_REG | DUMP_BASE_INFO);
	else
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
}

#ifdef SUPPORT_WATCHDOG
extern void _vcmd_watchdog_pause(struct hantrovcmd_dev *dev);
#endif

extern int _vcmd_add_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);
/**
 * process vcd multi-slice abnormal irq
 */
static void process_vcd_slice_irq(vcmd_mgr_t *vcmd_mgr,
									struct hantrovcmd_dev *dev,
									struct cmdbuf_obj *obj, u32 irq)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;
	u32 *status_va;
	volatile void *hwregs;
	u32 timeout_val;

	trace_process_vcd_slice_irq("[dec] buffer empty: cmdbuf ", obj->cmdbuf_id);
	if (obj->xring_slice_mode)
		vcmd_wfd_trace("buffer empty: cmdbuf[%d]\n", obj->cmdbuf_id);

	if (obj->slice_run_done == 0 && obj->cmdbuf_run_done == 0) {
		status_va = obj->status_va +
					subsys->reg_off[SUB_MOD_MAIN] / 2 / 4 + 1;

		*status_va = irq & (~0x100);	// clean sw_dec_irq (bit 8 of reg1)

		vcmd_klog(LOGLVL_DEBUG, "avoid repeatly BUF_EMPTY to CPU.\n");

		hwregs = subsys->hwregs[SUB_MOD_MAIN];
		//disable sw_timeout_override_e (bit 31 of reg319), to stop decoder timer
		timeout_val = (u32)ioread32((void __iomem *)(hwregs + 0x4 * 319));
		iowrite32((timeout_val & (0x7fffffff)), (void __iomem *)(hwregs + 0x4 * 319));

		//Gate abnormal interrupt from VCD, to avoid repeatly BUF_EMPTY to CPU.
		vcmd_write_reg((const void *)dev->hwregs,
						VCMD_REGISTER_EXT_INT_GATE_OFFSET,
						dev->intr_gate_mask | dev->abn_irq_mask);
		obj->slice_run_done = 1;
		dev->abn_irq_schedule |= ABN_IRQ_SCHEDULE_BUFFER_EMPTY;

#ifdef SUPPORT_WATCHDOG
		_vcmd_watchdog_pause(dev);
#endif
		_vcmd_add_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
	}
}

/**
 * process vcd abnormal irq
 */
void process_vcd_abn_irq(vcmd_mgr_t *vcmd_mgr,
						struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;
	volatile void *hwregs;
	unsigned long flags;
	u32 irq;

	hwregs = subsys->hwregs[SUB_MOD_MAIN];

	// read VCD int_status
	spin_lock_irqsave(&dev->abn_irq_lock, flags);
	irq = (u32)ioread32((void __iomem *)(hwregs + 0x04));
	vcmd_klog(LOGLVL_VERBOSE, "abnormal irq is 0x%x\n", irq);
	if (irq & VCD_STATUS_LINE_CNT_INT) {
		/* pp line counter irq */
		process_vcd_line_cnt_irq(vcmd_mgr, dev, obj, irq);
	}

	if (irq & VCD_STATUS_BUS_ERR_INT) {
		/* bus-error irq */
		process_vcd_bus_err_irq(dev, hwregs, irq);
	}

	if (irq & VCD_STATUS_SLICE_READY_INT) {
		/* slice rdy irq */
		process_vcd_slice_irq(vcmd_mgr, dev, obj, irq);
	}
	spin_unlock_irqrestore(&dev->abn_irq_lock, flags);

}