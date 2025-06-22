/*
 * SPDX_license-Identifier: GPL-2.0  OR BSD-3-Clause
 * Copyright (c) 2015, Verisilicon Inc. - All Rights Reserved
 * Copyright (c) 2011-2014, Google Inc. - All Rights Reserved
 *
 ********************************************************************************
 *
 * GPL-2.0
 *
 ********************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ********************************************************************************
 *
 * Alternatively, This software may be distributed under the terms of
 * BSD-3-Clause, in which case the following provisions apply instead of the ones
 * mentioned above :
 *
 ********************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 */

#include <linux/ioctl.h>
#include <linux/types.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "vcx_abnormal_irq.h"

#include "venc_trace.h"

extern void vcmd_start(struct hantrovcmd_dev *dev);
extern int vcmd_abort(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
					u32 *aborted_id);
extern void _vcmd_kthread_wakeup(vcmd_mgr_t *vcmd_mgr);

/**
 * @brief read vce slice info for outputing encoded slice stream
 */
static void read_vce_slice_info(struct hantrovcmd_dev *dev, volatile void *hwregs)
{
	u32 reg_val;
	struct slice_stream_info *info = &dev->slice_strm_info;

	/* output stream base lsb: reg8 */
	info->output_strm_base_lsb = (u32)ioread32((void __iomem *)(hwregs + 8 * 4));
	/* output stream base msb: reg59 */
	info->output_strm_base_msb = (u32)ioread32((void __iomem *)(hwregs + 59 * 4));
	/* read out slice ready num: reg7 */
	reg_val = (u32)ioread32((void __iomem *)(hwregs + 7 * 4));
	info->slice_rdy_num = (reg_val >> 17) & 0xFF;
	/* size tbl base lsb, which is used for storing slice size: reg10 */
	info->size_tbl_base_lsb = (u32)ioread32((void __iomem *)(hwregs + 10 * 4));
	/* size tbl base msb, which is used for storing slice size: reg58 */
	info->size_tbl_base_msb = (u32)ioread32((void __iomem *)(hwregs + 58 * 4));
}

/**
 * @brief used in interrupt bottom half if need process slice info further
 */
void hook_process_vce_slice_info(struct hantrovcmd_dev *dev)
{
	// customer to do further process if needed.
	// dev->slice_strm_info;
}

/**
 * @brief vce multi-slice abnormal irq
 */
static void process_vce_slice_irq(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, volatile void *hwregs,
								  struct cmdbuf_obj *obj, u32 irq)
{
	u32 slice_rdy_num;
	u32 *status_va;

	trace_process_vce_slice_irq("[enc] slice ready: cmdbuf ", obj->cmdbuf_id);
	vcmd_wfd_trace("slice ready: cmdbuf[%d]\n", obj->cmdbuf_id);

	if (obj->slice_run_done == 0) {
		/* status_main_addr is 0 */
		status_va = obj->status_va + 0;
		// clear VCE slice_rdy irq (bit 8 of reg1)
		irq &= 0xffff903;
		iowrite32(irq, (void __iomem *)(hwregs + 0x4));

		// read out & backup slice irq related info
		slice_rdy_num = (u32)ioread32((void __iomem *)(hwregs + 0x1C));

		*(status_va + VCMD_SLICE_RDY_INTERRUPT) = irq;
		*(status_va + VCMD_SLICE_RDY_NUM) = slice_rdy_num;
	}
	// more info need to be read for outputing encoded slice stream.
	//read_vce_slice_info(&vcmd_mgr->dev_ctx[obj->core_id], hwregs);

	/* xiaomi add*/
	process_vce_slice_size(vcmd_mgr, dev, obj, SEND_SLICE_SLICE_READY);

}

/**
 * @brief proccess vce input segment polling timeout irq
 */
static void proccess_vce_segment_polling_timeout_irq(vcmd_mgr_t *vcmd_mgr,
					struct hantrovcmd_dev *dev, volatile void *hwregs,
					struct cmdbuf_obj *obj, u32 irq)
{
	trace_proccess_vce_segment_polling_timeout_irq("[enc] slice info poll timeout: cmdbuf ", obj->cmdbuf_id);
	vcmd_wfd_trace("slice info poll timeout: cmdbuf[%d]\n", obj->cmdbuf_id);

	/* clear segment polling timeout err irq */
	iowrite32(irq & 0x1, (void __iomem *)(hwregs + 118 * 4));
	/* mark this cmdbuf as input segment polling timeout err */
	obj->segment_polling_timeout = 1;
	dev->segment_polling_timeout = 1;
	_vcmd_kthread_wakeup(vcmd_mgr);
}

void process_vce_abn_irq(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
						struct cmdbuf_obj *obj)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;

	volatile void *hwregs;
	unsigned long flags;
	u32 irq;

	hwregs = subsys->hwregs[SUB_MOD_MAIN];

	spin_lock_irqsave(&dev->abn_irq_lock, flags);
	// read VCE int_status
	irq = (u32)ioread32((void __iomem *)(hwregs + 0x04));
	if (irq & ASIC_STATUS_SLICE_READY) {
		/* slice rdy process*/
		process_vce_slice_irq(vcmd_mgr, dev, hwregs, obj, irq);
		dev->abn_irq_schedule = ABN_IRQ_SCHEDULE_SLICE_RDY;
	}
	// read VCE input segment poll timeout irq status
	irq = (u32)ioread32((void __iomem *)(hwregs + 118 * 4)); // vce reg118
	if (irq & 0x1)
		proccess_vce_segment_polling_timeout_irq(vcmd_mgr, dev, hwregs, obj, irq);
	spin_unlock_irqrestore(&dev->abn_irq_lock, flags);

}

#define REGS_OFFSET_IN_CMDBUF         422 //422 for xiaomi side
#define CMDBUF_DATA(va, off)          ((va) + (off) - REGS_OFFSET_IN_CMDBUF)

static void change_cmdbuf_for_ufbc_dec_err(struct cmdbuf_obj *obj)
{
	//change ufbcEnable to 0.
	*CMDBUF_DATA(obj->cmd_va, 435) = *CMDBUF_DATA(obj->cmd_va, 435) & 0xe;
	//change format to NV12.
	*CMDBUF_DATA(obj->cmd_va, 482) = *CMDBUF_DATA(obj->cmd_va, 482) & 0x1fffffff;
	//change input cb address to y address.
	*CMDBUF_DATA(obj->cmd_va, 457) = *CMDBUF_DATA(obj->cmd_va, 456);
	//change input cr address to y address.
	*CMDBUF_DATA(obj->cmd_va, 458) = *CMDBUF_DATA(obj->cmd_va, 456);
	//change input cb address to y address.(MSB)
	*CMDBUF_DATA(obj->cmd_va, 498) = *CMDBUF_DATA(obj->cmd_va, 497);
	//change input cr address to y address.(MSB)
	*CMDBUF_DATA(obj->cmd_va, 499) = *CMDBUF_DATA(obj->cmd_va, 497);
}

static void change_cmdbuf_for_segment_polling_timeout(struct cmdbuf_obj *obj)
{
	// disable input segment poll(bit3)
	/* 445 is the position of vce reg1, 562 is vce reg118,
	 * should set it according to your cmdbuf WREG.
	 */
	*CMDBUF_DATA(obj->cmd_va, 562) = *CMDBUF_DATA(obj->cmd_va, 562) & (~0x8);
}

/**
 * @brief process flow of abnormal err irq
 *   when dev->ufbc_decoder_err = 1, process for ufbc decoder err
 *   when dev->segmen_polling_timeout = 1, process for vce input
 *      segmen polling timeout.
 */

void process_abnormal_err_irq(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev)
{
	u32 aborted_id;
	u32 mode;
	u32 curr_id;
	struct cmdbuf_obj *obj;
	unsigned long flags;

	spin_lock_irqsave(dev->spinlock, flags);
	mode = dev->abort_mode;
	curr_id = vcmd_get_register_value((const void *)dev->hwregs,
							dev->reg_mirror,
							HWIF_VCMD_CMDBUF_EXE_ID);
	obj = &vcmd_mgr->objs[curr_id];

	/* abort vcmd by immediate mode */
	dev->abort_mode = 1;
	vcmd_set_reg_mirror(dev->reg_mirror, HWIF_VCMD_ABORT_MODE, 0x1);
	spin_unlock_irqrestore(dev->spinlock, flags);

	/* catch abnormal error irq trace */
	trace_process_abnormal_err_irq("[enc] abnormal error irq: cmdbuf ", curr_id);

	vcmd_abort(vcmd_mgr, dev, &aborted_id);
	spin_lock_irqsave(dev->spinlock, flags);
	/* recover the abort mode */
	dev->abort_mode = mode;
	if (dev->state == VCMD_STATE_IDLE) {
		/* changing cmdbuf content and start this cmdbuf again */
		if (dev->ufbc_decoder_err) {
			dev->ufbc_decoder_err = 0;
			change_cmdbuf_for_ufbc_dec_err(obj);
		}
		if (dev->segment_polling_timeout) {
			dev->segment_polling_timeout = 0;
			change_cmdbuf_for_segment_polling_timeout(obj);
		}
		vcmd_start(dev);
	} else {
		vcmd_klog(LOGLVL_ERROR, "hantrovcmd: abort vcmd failed!\n");
	}
	spin_unlock_irqrestore(dev->spinlock, flags);
}

/**
 * @brief process ufbc abnormal irq
 */
void process_ufbc_abn_irq(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
						struct cmdbuf_obj *obj)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;

	volatile void *hwregs;
	unsigned long flags;
	u32 irq;

	trace_process_ufbc_abn_irq("[enc] ufbc decode error: cmdbuf ", obj->cmdbuf_id);

	hwregs = subsys->hwregs[SUB_MOD_UFBC];

	spin_lock_irqsave(&dev->abn_irq_lock, flags);
	irq = (u32)ioread32((void __iomem *)(hwregs + 0x20)); //ufbc reg8
	/* clear ufbc decoder err irq */
	iowrite32(irq, (void __iomem *)(hwregs + 0x20));
	if (irq & 0x1) {
		/* mark this cmdbuf as ufbc decoder err */
		obj->ufbc_decoder_err = 1;
		dev->ufbc_decoder_err = 1;
		_vcmd_kthread_wakeup(vcmd_mgr);
	}
	spin_unlock_irqrestore(&dev->abn_irq_lock, flags);
}