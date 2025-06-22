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
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_page_range
 * SetPageReserved
 * ClearPageReserved
 */
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>

#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <linux/semaphore.h>
#include <linux/spinlock.h>
/* needed for virt_to_phys() */
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>

#include <asm/irq.h>

#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#include <linux/iommu.h>
#include<linux/ktime.h>

/* our own stuff */
#include <linux/platform_device.h>
#include "subsys.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "hantrodec_defs.h"
#include "hantrovcmd.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/hantrodec_defs.h>
#include <dt-bindings/xring/platform-specific/vpu/vdec/hantrovcmd.h>
#endif
#include "hantrovcmd_priv.h"
#include "hantroaxife.h"
#include "hantrovcmddbgfs.h"
#include "hantroabnormalirq.h"
#include "vdec_power_base.h"
#include "vdec_power_manager.h"
#include "mdr_pub.h"
#include "vdec_log.h"

#define CREATE_TRACE_POINTS
#include "vdec_trace.h"
#undef CREATE_TRACE_POINTS


/****************************************************************
 * Macro definitions
 ***************************************************************/
//#define TIMEOUT_IRQ_TIMER

/*---------------------------------------------------------------
 * Macros related to sub-system config
 --------------------------------------------------------------*/
/* VCMD master_out_clk mode */
#define APB_CLK_ON	(1)
#define APB_CLK_OFF	(0)
#define APB_CLK_MODE	APB_CLK_OFF

/*---------------------------------------------------------------
 * Macros related to debug
 --------------------------------------------------------------*/

//#define VCMD_DEBUG_INTERNAL
//#define IRQ_SIMULATION

/*---------------------------------------------------------------
 * Macros related to PCIe Platform Config
 --------------------------------------------------------------*/
#ifdef EMU
  #define VCMD_BUF_POOL_OFFSET 0x4800000

  #define PCI_VENDOR_ID_HANTRO      0x1d9b//0x16c3
  #define PCI_DEVICE_ID_HANTRO      0xface// 0x7011

  /* Base address got control register */
  #define PCI_H2_BAR              2

  /* Base address DDR register */
  #define PCI_DDR_BAR             4

#else //EMU

  #define VCMD_BUF_POOL_OFFSET 0x800000
#ifdef PLATFORM_GEN7
  #define PCI_VENDOR_ID_HANTRO      0x10ee//0x16c3
  #define PCI_DEVICE_ID_HANTRO      0x9014// 0x7011

  /* Base address got control register */
  #define PCI_H2_BAR              0

  /* Base address DDR register */
  #define PCI_DDR_BAR             2
#else //PLATFORM_GEN7
  #define PCI_VENDOR_ID_HANTRO      0x10ee//0x16c3
  #define PCI_DEVICE_ID_HANTRO      0x8014// 0x7011

  /* Base address got control register */
  #define PCI_H2_BAR              4

  /* Base address DDR register */
  #define PCI_DDR_BAR             0
#endif //PLATFORM_GEN7
#endif //EMU

/*---------------------------------------------------------------
 * Macros related to dev/process workload management
 --------------------------------------------------------------*/
#define VCMD_WORKLOAD_UNIT                 (8192L * 4096L)
#define VCMD_INTR_INTERVAL                 (VCMD_WORKLOAD_UNIT * 8)
#define PROCESS_MAX_WORKLOAD               (VCMD_WORKLOAD_UNIT * 64)

#define CMDBUF_POOL_TOTAL_SIZE             (1 * 1024 * 1024 - 8 * 1024)
#define CMDBUF_VCMD_REGISTER_TOTAL_SIZE    (16 * 1024)

/*---------------------------------------------------------------
 * Slice info
 --------------------------------------------------------------*/
#define VCMD_SLICE_ENABLE_MASK              (0x80010000)
#define VCMD_REG_MSB_MASK                   (0xFFFF0000)
#define VCMD_REG_LSB_MASK                   (0x0000FFFF)
#define XRING_SLICE_IDENTIFIER              (0xFFFFFFFF)
#define SLICE_MAX_LINE_CNT                  (4)
#define SLICE_TXTBUF_CNT                    (4)
/* one slice text buffer use 4 bytes */
#define SLICE_TXTBUF_SIZE                   (4 * SLICE_TXTBUF_CNT)
#define SLICE_TXTBUF_TOTAL_SIZE             (SLICE_TXTBUF_SIZE * SLICE_MAX_LINE_CNT)
#define SLICE_INTERVAL                      (7)
#define THRESHOLD_US                        (100000ULL)
#define VCMD_ABORT_THRESHOLD_US             (500000ULL)

#define VDEC_MAX_RESET_COUNT                3
#define VDEC_MAX_WDT_COUNT                  1

enum link_and_run_stage {
	PROBE_STAGE  = 0,
	NORMAL_STAGE = 1,
};

/****************************************************************
 * define driver parameter
 ***************************************************************/
/* 0:disable 1:enable */
static int enable_dev_list_check ;
module_param(enable_dev_list_check, int, 0644);
MODULE_PARM_DESC(enable_dev_list_check, "enable dev_list check");

/****************************************************************
 * external/global variables declarations
 ***************************************************************/
struct vcmd_config vcmd_core_array[MAX_SUBSYS_NUM];
extern unsigned long vcmd_isr_polling;
extern u32 arbiter_weight;
extern u32 arbiter_urgent;
extern u32 arbiter_timewindow;
extern u32 arbiter_bw_overflow;
extern int vsi_dumplvl;
#ifdef CONFIG_DEC_CM_RUNTIME
extern u32 runtime_cm_enable;
#endif
/****************************************************************
 * local functions declarations
 ***************************************************************/
static struct cmd_jmp_t *_get_jmp_cmd(struct cmdbuf_obj *obj);
static struct cmd_end_t *_get_end_cmd(struct cmdbuf_obj *obj);

#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t hantrovcmd_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t hantrovcmd_isr(int irq, void *dev_id);
#endif

int vcmd_irq_flags;

static void _vcmd_irq_bottom_half(struct work_struct *work);
static void vcmd_start(struct hantrovcmd_dev *dev);
static void vcmd_start2(struct hantrovcmd_dev *dev, u32 reset);
static u32 dev_get_job_num(struct hantrovcmd_dev *dev);
static u32 dev_wait_job_num(struct hantrovcmd_dev *dev);
static bi_list_node *vcmd_get_exec_node(struct hantrovcmd_dev *dev,
		u32 irq_status);
static int isr_process_node(vcmd_mgr_t *vcmd_mgr,
		bi_list_node *node,
		u32 exe_status);
static void vcmd_do_runtime_suspend(vcmd_mgr_t *vcmd_mgr, bool status);
static bool _runtime_context_idled(vcmd_mgr_t *vcmd_mgr);

extern void abort_vcd(volatile u8 *reg_base);

int _vcmd_add_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);
static void _vcmd_del_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);
static void _vcmd_mod_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);

extern int get_dma_buf_by_iova(unsigned long iova, unsigned long *offset, struct dma_buf **dma_buf);
extern void xplayer_set_display_dma_buf(struct dma_buf *buf,
		struct xplayer_cmdlist_config *config,
		struct xplayer_iommu_format_info *iommu_info);
extern void xplayer_get_cmdlist(struct xplayer_cmdlist_addr *info);

/****************************************************************
 * watchdog, irq_simulation, kernel thread and debug functions
 ***************************************************************/

#ifdef IRQ_SIMULATION
struct timer_manager {
	void *vcmd_mgr;
	void *obj;
};

static struct timer_list irq_simul_timer[SLOT_NUM_CMDBUF];
struct timer_manager irq_simul_ctx[SLOT_NUM_CMDBUF];

void get_random_bytes(void *buf, int nbytes);
/**
 * @brief trigger isr processing for irq simulation.
 */
void _irq_simul_trigger_isr(struct timer_list *timer)
{
	struct cmdbuf_obj *obj;
	u32 timer_id = 0;

	timer_id = timer - irq_simul_timer;
	if (irq_simul_ctx[timer_id].obj) {
		obj = (struct cmdbuf_obj *)irq_simul_ctx[timer_id].obj;
	} else {
		vcmd_klog(LOGLVL_ERROR, "tigger isr failed, the cmdbuf obj is NULL\n");
		return;
	}

	vcmd_klog(LOGLVL_DEBUG, "trigger core 0 irq\n");
	hantrovcmd_isr(obj->core_id, irq_simul_ctx[timer_id].vcmd_mgr);
	del_timer(timer);
	irq_simul_ctx[timer_id].obj = NULL;
}

/**
 * @brief add timer for irq simulation.
 */
void _irq_simul_add_timer(struct cmdbuf_obj *obj)
{
	u64 random_num;
	struct timer_list *temp_timer = NULL;

	//get_random_bytes(&random_num, sizeof(u32));
	random_num = (u32)((u64)100 * obj->workload / VCMD_WORKLOAD_UNIT + 50);
	vcmd_klog(LOGLVL_DEBUG, "random_num=%lld\n", random_num);

	temp_timer = &irq_simul_timer[obj->cmdbuf_id];
	irq_simul_ctx[obj->cmdbuf_id].obj = (void *)obj;

	//if (obj->core_id==0)
	if (temp_timer) {
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
		init_timer(temp_timer);
		temp_timer->function = _irq_simul_trigger_isr;
		temp_timer->expires = jiffies + random_num * HZ / 10;
		temp_timer->data = (unsigned long)temp_timer;
		add_timer(temp_timer);
#else
		timer_setup(temp_timer, _irq_simul_trigger_isr, 0);
		//the expires time is 1s
		temp_timer->expires = jiffies + random_num * HZ / 10;
		add_timer(temp_timer);
#endif
	}
}

/**
 * @brief init timers for irq simulation.
 */
void _irq_simul_init(void *vcmd_mgr)
{
	u32 i;
	for (i = 0; i < SLOT_NUM_CMDBUF; i++) {
		irq_simul_ctx[i].obj = NULL;
		irq_simul_ctx[i].vcmd_mgr = vcmd_mgr;
	}
}
#endif //IRQ_SIMULATION

static void hook_subsys_reset(struct hantrovcmd_dev *dev, u32 modid)
{
	if (dev->reset_count >= VDEC_MAX_RESET_COUNT) {
		vcmd_klog(LOGLVL_WARNING, "[dec] system reset: wdt_trigger_count %d, reset_count %d!",
			dev->wdt_trigger_count, dev->reset_count);
		dev->reset_count = 0;
		mdr_system_error(MODID_VPU_DEC_HANG_EXCEPTION, 0, 0);
	} else {
		dev->reset_count++;
		vcmd_klog(LOGLVL_WARNING, "[dec] subsys reset: wdt_trigger_count %d, reset_count %d!",
			dev->wdt_trigger_count, dev->reset_count);
		mdr_system_error(modid, 0, 0);
		hantrodec_reset(dev->device, true);
	}
}

#ifdef SUPPORT_WATCHDOG
static void _vcmd_watchdog_stop(struct hantrovcmd_dev *dev);
/**
 * @brief hook function for system-driver to do further process
 *  for tiggered watchdog.
 */
static void hook_vcmd_watchdog(struct hantrovcmd_dev *dev, int succeed)
{
	if (succeed) {
		vcmd_klog(LOGLVL_WARNING, "axife flush and vcmd Abort succeed, wdt_trigger_count %d, reset_count %d!",
			dev->wdt_trigger_count, dev->reset_count);
		/*
		 * The watchdog process exceeds the maximum number of consecutive
		 * successful processes and requires a subsystem reset
		 */
		if (dev->wdt_trigger_count >= VDEC_MAX_WDT_COUNT) {
			dev->wdt_trigger_count = 0;
			hook_subsys_reset(dev, MODID_VPU_DEC_WDT_EXCEPTION);
			return;
		}
		dev->wdt_trigger_count++;
	} else {
		vcmd_klog(LOGLVL_ERROR, "axife flush or vcmd abort failed, need to re-power subsys, reset_count %d",
				dev->reset_count);
		if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
			dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
		else
			dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);

		// need to do sub-system reset
		hook_subsys_reset(dev, MODID_VPU_DEC_WDT_EXCEPTION);
		dev->wdt_trigger_count = 0;
	}
}

/**
 * @brief reset vcmd arbiter when VCARB hang
 */
static void watchdog_reset_vcmd_arbiter(struct hantrovcmd_dev *dev)
{
	u32 arb = 0;
	int loop_cnt = 0;

	if (dev->hw_version_id == HW_ID_1_5_0)
		return;

	arb = vcmd_read_reg((const void *)dev->hwregs,
						VCMD_REGISTER_ARB_OFFSET);
	arb &= 0xfffffff8;
	vcmd_write_reg((const void *)dev->hwregs, VCMD_REGISTER_ARB_OFFSET, arb);

	do {
		if (dev->arb_reset_irq == 1) {
			dev->arb_reset_irq = 0;
			vcmd_klog(LOGLVL_INFO, "VCARB is hang, reset it!\n");
			return;
		}
		mdelay(10); // wait 10ms
	} while (++loop_cnt < 100);

	vcmd_klog(LOGLVL_ERROR, "Reseted VCARB, but failed!\n");
}

/**
 * @brief watchdog wait vcmd abort done
 */
static int watchdog_wait_vcmd_aborted(struct hantrovcmd_dev *dev)
{
	int loop_cnt = 0;
	u32 state;

	do {
		state = vcmd_get_register_value((const void *)dev->hwregs,
									dev->reg_mirror, HWIF_VCMD_WORK_STATE);
		if (state == HW_WORK_STATE_IDLE) {
			//aborted
			dev->state = VCMD_STATE_IDLE;
			return 0;
		}
		mdelay(10); // wait 10ms
	} while (++loop_cnt < 100);

	vcmd_klog(LOGLVL_ERROR, "can't go to IDLE, need to re-power sub-system!\n");

	return -1;
}

/**
 * @brief stop vcmd when watchdog triggered
 */
static int watchdog_stop_vcmd(struct hantrovcmd_dev *dev)
{
	u32 state;

	state = vcmd_get_register_value((const void *)dev->hwregs,
				dev->reg_mirror, HWIF_VCMD_WORK_STATE);

	if (state == HW_WORK_STATE_IDLE) {
		dev->state = VCMD_STATE_IDLE;
		return 0;
	}

	//if state is not in IDLE/PEND, abort VCMD by sw.
	if (dev->hw_feature.has_arbiter) {
		vcmd_set_reg_mirror(dev->reg_mirror, HWIF_VCMD_ABORT_MODE, 0x0);
	} else {
		dev->abort_mode = 0x1;
		vcmd_set_reg_mirror(dev->reg_mirror, HWIF_VCMD_ABORT_MODE, 0x1);
	}
	vcmd_write_register_value((const void *)dev->hwregs,
					dev->reg_mirror,
					HWIF_VCMD_START_TRIGGER, 0);

	if (dev->hw_feature.has_arbiter)
		return 0;

	//wait vcmd core aborted and vcmd enters IDLE mode.
	if (watchdog_wait_vcmd_aborted(dev) == 0)
		return 0;

	vcmd_klog(LOGLVL_ERROR, "can't go to IDLE\n");
	return -1;
}

extern void watchdog_stop_vcd(volatile u8 *reg_base);
/**
 * @brief process when watchdog triggered.
 */
static void _vcmd_watchdog_process(struct hantrovcmd_dev *dev)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;
	struct cmdbuf_obj *obj;
	vcmd_mgr_t *vcmd_mgr;
	int succeed = 1, ret;
	unsigned long flags;

	vcmd_klog(LOGLVL_WARNING, "dec: trigger watchdog, stop vcmd, wait job num = %u, sw cmdbuf rdy num = %u\n",
		dev_get_job_num(dev), dev->sw_cmdbuf_rdy_num);
	if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
	else
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);

	if (CMDBUF_ID_CHECK(dev->aborted_cmdbuf_id) >= 0) {
		vcmd_mgr = (vcmd_mgr_t *)dev->handler;
		obj = &vcmd_mgr->objs[dev->aborted_cmdbuf_id];
		vcmd_klog(LOGLVL_WARNING, "dec: the cuurent obj workload is %llu\n", obj->workload);
	}

	spin_lock_irqsave(dev->spinlock, flags);
	ret = watchdog_stop_vcmd(dev);
	watchdog_stop_vcd(subsys->hwregs[SUB_MOD_MAIN]);
	spin_unlock_irqrestore(dev->spinlock, flags);
	if (dev->hw_feature.has_arbiter) {
		mdelay(10); // wait 10ms
		/* if not own arbier, it will receive arberr interrupt */
		if (dev->arb_err_irq) {
			dev->arb_err_irq = 0;
			return;
		}
		spin_lock_irqsave(dev->spinlock, flags);
		ret = watchdog_wait_vcmd_aborted(dev);
		if (ret == 0)
			watchdog_reset_vcmd_arbiter(dev);
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	if (ret < 0)
		succeed = 0;

	spin_lock_irqsave(dev->spinlock, flags);
	if (succeed && AXIFEFlush(subsys->hwregs[SUB_MOD_AXIFE]) == -1)
		succeed = 0;
	spin_unlock_irqrestore(dev->spinlock, flags);

	hook_vcmd_watchdog(dev, succeed);
	mdelay(10); // wait 10ms
	spin_lock_irqsave(dev->spinlock, flags);
	if (dev->state != VCMD_STATE_WORKING) {
		if (dev->abort_mode == 1)
			dev->abort_mode = 0;
		vcmd_start(dev);
	}
	spin_unlock_irqrestore(dev->spinlock, flags);
}
#endif //SUPPORT_WATCHDOG
/**
 * @brief process external vcmd timeout.
 */
static void hook_vcmd_external_timeout(void *_dev)
{
	unsigned long flags;
	struct hantrovcmd_dev *dev = (struct hantrovcmd_dev *)_dev;

	spin_lock_irqsave(dev->spinlock, flags);
	dev->state = VCMD_STATE_IDLE;
	spin_unlock_irqrestore(dev->spinlock, flags);

	vcmd_klog(LOGLVL_ERROR, "vcmd abort is not waited, timeout is from external system! reset_count %d\n",
			dev->reset_count);
	if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
	else
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);

	// need to do sub-system reset
	hook_subsys_reset(dev, MODID_VPU_DEC_EXTERNAL_TIMEOUT_EXCEPTION);
}

/**
 * @brief when buffer empty timer timeout, abort slice decoding
 */
static int _vcmd_slice_decoding_abort(struct hantrovcmd_dev *dev)
{
	bi_list_node *node = NULL, *curr_node;
	struct cmdbuf_obj *obj, *curr_obj;
	unsigned long flags;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)dev->handler;
	struct vcmd_timer *tm = &dev->vcmd_timer[VCMD_TIMER_BUFFER_EMPTY];

	if (down_killable(&dev->work_sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		return -EINTR;
	}
	spin_lock_irqsave(dev->spinlock, flags);
	curr_node = vcmd_get_exec_node(dev, 0);
	spin_unlock_irqrestore(dev->spinlock, flags);

	if (!curr_node) {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n",
			__func__);
		up(&dev->work_sem);
		return -1;
	}
	curr_obj = (struct cmdbuf_obj *)curr_node->data;

	node = dev->work_list.head;
	while (node) {
		if (node == curr_node && node->next) {
			obj = (struct cmdbuf_obj *)node->next->data;
			if (obj->owner == curr_obj->owner)
				vcmd_klog(LOGLVL_WARNING, "%s: work_list has same owner job!\n",
					 __func__);
			break;
		}
		node = node->next;
	}

	if (!node) {
		vcmd_klog(LOGLVL_WARNING, "%s buffer empty timer timeout, but work_list has no other job!!\n", __func__);
		spin_lock_irqsave(dev->spinlock, flags);
		_vcmd_mod_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
		spin_unlock_irqrestore(dev->spinlock, flags);
		up(&dev->work_sem);
		return 1;
 	}

	if (tm->active && curr_obj->slice_run_done == 2) {
		spin_lock_irqsave(dev->spinlock, flags);
		_vcmd_watchdog_stop(dev);
		curr_obj->executing_status = CMDBUF_EXE_STATUS_SLICE_DECODING_ABORTED;
		abort_vcd(dev->subsys_info->hwregs[SUB_MOD_MAIN]);
		spin_unlock_irqrestore(dev->spinlock, flags);
		isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_SLICE_DECODING_ABORTED);
	}
	up(&dev->work_sem);

	return 0;
}

/**
 * @brief To check vcmd_mgr/dev's actions which need kthread to process
 * @return int: 0: no actions; 1: have actions
 */
static int _vcmd_kthread_actions(vcmd_mgr_t *vcmd_mgr,
			struct hantrovcmd_dev **dev)
{
	int i, j;

	if (vcmd_mgr->stop_kthread == 1) {
		return 1;
	}

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		*dev = &vcmd_mgr->dev_ctx[i];
		if ((*dev)->slice_send_outbuf_flag == 1) {
			return 1;
		}
 		for (j = 0; j < VCMD_TIMER_MAX; j++) {
 			if ((*dev)->vcmd_timer[j].expired == 1)
 				return 1;
		}
	}

	return 0;
}

static void send_outbuf_to_xplayer(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev)
{
	u16 cmdbuf_id = 0;
	struct cmdbuf_obj *obj;
	u32 *ptr = NULL;
	int i = 0;
	int ret = 0;

	while(1) {
		ret = kfifo_out(&dev->send_slice_buffer_queue, &cmdbuf_id, sizeof(u16));
		if (cmdbuf_id >= SLOT_NUM_CMDBUF) {
			vcmd_klog(LOGLVL_ERROR, "invalid cmdbuf id\n");
			return;
		}
		if (ret != sizeof(u16))
			return;

		obj = &vcmd_mgr->objs[cmdbuf_id];
		if (obj->xring_slice_mode == 0) {
			vcmd_klog(LOGLVL_ERROR, "slice mode disable when slice_to_send triggered\n");
			return;
		}

		trace_send_outbuf_to_xplayer("[dec] send output buffer: cmdbuf ", cmdbuf_id);
		vcmd_wfd_trace("send output buffer: cmdbuf[%d]\n", cmdbuf_id);
		vcmd_klog(LOGLVL_INFO, "slice mode has send output buffer to xplayer\n");
	}
}

/**
 * @brief vcmd kernel thread main function
 */
static int _vcmd_kthread_fn(void *data)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)data;
	struct hantrovcmd_dev *dev = NULL;
	struct hantrodec_dev *dec_dev = NULL;
	vdec_power_mgr *pm = NULL;
	int ret;

	while (!kthread_should_stop()) {
		if (wait_event_interruptible(vcmd_mgr->kthread_waitq,
				_vcmd_kthread_actions(vcmd_mgr, &dev))) {
			vcmd_klog(LOGLVL_ERROR, "wait_event_interruptible signaled!!!\n");
			return -ERESTARTSYS;
		}

		if (dev == NULL) {
			continue;
		}

		dev->device = &vcmd_mgr->platformdev->dev;
		if (dev->slice_send_outbuf_flag == 1) {
			dev->slice_send_outbuf_flag = 0;
			send_outbuf_to_xplayer(vcmd_mgr, dev);
		}

		dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
		if (!dec_dev) {
			vcmd_klog(LOGLVL_INFO, "dec_dev is NULL!\n");
			continue;
		}
		pm = dec_dev->pm;
		if (!pm) {
			vcmd_klog(LOGLVL_INFO, "PowerMgr is NULL!\n");
			continue;
		}
		mutex_lock(&pm->pm_lock);
		ret = regulator_is_enabled(pm->vdec_rg);
		if (ret <= 0) {
			vcmd_klog(LOGLVL_WARNING, "vdec core has been powered off, ret=%d\n",
					ret);
			mutex_unlock(&pm->pm_lock);
			continue;
		}

		if (dev->vcmd_timer[VCMD_TIMER_TIMEOUT].expired == 1) {
			dev->vcmd_timer[VCMD_TIMER_TIMEOUT].expired = 0;
			hook_vcmd_external_timeout(dev);
			mutex_unlock(&pm->pm_lock);
			continue;
		}

		if (dev->vcmd_timer[VCMD_TIMER_BUFFER_EMPTY].expired == 1) {
			dev->vcmd_timer[VCMD_TIMER_BUFFER_EMPTY].expired = 0;
			vcmd_klog(LOGLVL_WARNING, "vdec slice timeout");
			_vcmd_slice_decoding_abort(dev);
			mutex_unlock(&pm->pm_lock);
			continue;
		}

#ifdef SUPPORT_WATCHDOG
		if (dev->vcmd_timer[VCMD_TIMER_WATCHDOG].expired == 1) {
			dev->vcmd_timer[VCMD_TIMER_WATCHDOG].expired = 0;
			_vcmd_watchdog_process(dev);
			mutex_unlock(&pm->pm_lock);
			continue;
		}
#endif
		mutex_unlock(&pm->pm_lock);
	}

	return 0;
}

/**
 * @brief wake up vcmd kernel thread
 */
static void _vcmd_kthread_wakeup(vcmd_mgr_t *vcmd_mgr)
{

	if (IS_ERR(vcmd_mgr->kthread))
		return;

	wake_up_interruptible_all(&vcmd_mgr->kthread_waitq);
}

/**
 * @brief create kernel thread for vcmd driver
 */
static void _vcmd_kthread_create(vcmd_mgr_t *vcmd_mgr)
{
	vcmd_mgr->stop_kthread = 0;
	init_waitqueue_head(&vcmd_mgr->kthread_waitq);
	vcmd_mgr->kthread =
		kthread_run(_vcmd_kthread_fn, (void *)vcmd_mgr, "vcmd_kthread");
	if (IS_ERR(vcmd_mgr->kthread)) {
		vcmd_klog(LOGLVL_ERROR, "create vcmd kthread failed\n");
		return;
	}
}

/**
 * @brief stop kernel thread vcmd driver
 */
static void _vcmd_kthread_stop(vcmd_mgr_t *vcmd_mgr)
{
	if (!IS_ERR(vcmd_mgr->kthread)) {
		vcmd_mgr->stop_kthread = 1;
		kthread_stop(vcmd_mgr->kthread);
		vcmd_mgr->kthread = NULL;
	}
}

#ifdef SUPPORT_WATCHDOG
static void check_list_insert(bi_list *work_list, struct cmdbuf_obj *obj, const char *func_parent)
{
	struct cmdbuf_obj *obj_tmp;
	bi_list_node *node_tmp;
	u32 num = 0, match_cnt = 0;

	if (!work_list || !obj) {
		vcmd_klog(LOGLVL_ERROR, "[%s] work_list=%p obj=%p\n",
			func_parent, work_list, obj);
		return;
	}

	/* Check if the linked list has the same cmdbuf_id. */
	node_tmp = work_list->head;
	while (node_tmp) {
		obj_tmp = (struct cmdbuf_obj *)node_tmp->data;
		if (obj_tmp->cmdbuf_id == obj->cmdbuf_id) {
			vdec_cm_klog(LOGLVL_ERROR, "[%s] the work_list has exised the same cmdbuf_id(%d) match_cnt=%d!\n",
					func_parent, obj->cmdbuf_id, ++match_cnt);
			WARN_ON(1);
		}
		node_tmp = node_tmp->next;
		num++;
		if (num > SLOT_NUM_CMDBUF) {
			vcmd_klog(LOGLVL_ERROR, "[%s] loop=%d\n", func_parent, SLOT_NUM_CMDBUF);
			break;
		}
	}
}

static void hantor_dump_all_cmdbuf_id_by_list(bi_list *work_list, const char *parent_func, int line_no)
{
	struct cmdbuf_obj *obj;
	bi_list_node *node1;
	u32 cmdbuf_sn = 0;
	ktime_t end;

	if (!work_list) {
		vcmd_klog(LOGLVL_ERROR, "work_list=NULL\n");
		return;
	}

	node1 = work_list->head;

	/*
	 * print the values ​​of all cmdbuf_id in the linked list every 10s.
	 */
	end = ktime_get() + ms_to_ktime(10000);
	for (; ;) {
		if (ktime_after(ktime_get(), end)) {
			vcmd_klog(LOGLVL_ERROR, "[%s]%d ktime_get()=%lld\n", parent_func, line_no, ktime_get());
			node1 = work_list->head;
			while (node1) {
				obj = (struct cmdbuf_obj *)node1->data;
				vcmd_klog(LOGLVL_ERROR, "[%s]%d node1=%p sn:%d cmd_buf[%u]\n",
							parent_func, line_no, node1, cmdbuf_sn++, obj->cmdbuf_id);
				node1 = node1->next;
				if (cmdbuf_sn > SLOT_NUM_CMDBUF) {
					/* next loop print */
					break;
				}
			}
			end = ktime_get() + ms_to_ktime(10000);
			cmdbuf_sn = 0;
		}
	}
}

static void hantor_dump_cmdbuf_id_by_node(bi_list_node *node, const char *parent_func, int line_no)
{
	struct cmdbuf_obj *obj;
	bi_list_node *node1;
	u32 cmdbuf_sn = 0;
	ktime_t end;

	if (!node) {
		vcmd_klog(LOGLVL_ERROR, "node=NULL\n");
		return;
	}

	node1 = node;

	/*
	 * print the values ​​of all cmdbuf_id in the linked list every 10s.
	 */
	end = ktime_get() + ms_to_ktime(10000);
	for (; ;) {
		if (ktime_after(ktime_get(), end)) {
			vcmd_klog(LOGLVL_ERROR, "[%s]%d ktime_get()=%lld\n", parent_func, line_no, ktime_get());
			node1 = node;
			while (node1) {
				obj = (struct cmdbuf_obj *)node1->data;
				vcmd_klog(LOGLVL_ERROR, "[%s]%d node1=%p sn:%d cmd_buf[%u]\n",
							parent_func, line_no, node1, cmdbuf_sn++, obj->cmdbuf_id);
				node1 = node1->next;
				if (cmdbuf_sn > SLOT_NUM_CMDBUF) {
					/* next loop print */
					break;
				}
			}
			end = ktime_get() + ms_to_ktime(10000);
			cmdbuf_sn = 0;
		}
	}
}
/**
 * @brief wait jobs count that need to be finished from dev work_list
 */
static u32 dev_wait_job_num(struct hantrovcmd_dev *dev)
{
	struct cmdbuf_obj *obj;
	bi_list_node *node = dev->work_list.head;
	u32 num = 0, num1 = 0;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		if (obj->cmdbuf_run_done == 0) {
			num++;
			if (obj->has_jmp_cmd == 0 || obj->jmp_ie) {
				return num;
			}
		}
		node = node->next;
		num1++;
		if (num1 > SLOT_NUM_CMDBUF) {
			/* this function not return.
			 * and dump all cmdbuf_id.
			 */
			hantor_dump_all_cmdbuf_id_by_list(&dev->work_list, __func__, __LINE__);
		}
	}

	return num;
}

/**
 * @brief init vcmd watchdog
 */
static void _vcmd_watchdog_start(struct hantrovcmd_dev *dev)
{
	if (_vcmd_add_timer(dev, VCMD_TIMER_WATCHDOG) < 0)
		return;
	dev->watchdog_state = WD_STATE_ACTIVE;
}

/**
 * @brief stop vcmd watchdog
 */
static void _vcmd_watchdog_stop(struct hantrovcmd_dev *dev)
{
	if (dev->watchdog_state == WD_STATE_ACTIVE)
		_vcmd_del_timer(dev, VCMD_TIMER_WATCHDOG);
	dev->watchdog_state = WD_STATE_STOPPED;
}

/**
 * @brief pause vcmd watchdog
 */
void _vcmd_watchdog_pause(struct hantrovcmd_dev *dev) {
	if (dev->watchdog_state == WD_STATE_ACTIVE) {
		_vcmd_del_timer(dev, VCMD_TIMER_WATCHDOG);
		dev->watchdog_state = WD_STATE_PAUSE;
	 } else if (dev->watchdog_state == WD_STATE_STOPPED) {
		vcmd_klog(LOGLVL_INFO, "%s: watch-dog pause, but is has been STOPPED!\n", __func__);
	} else {
		vcmd_klog(LOGLVL_WARNING, "%s: watch-dog pause at invalid state %d!\n", __func__, dev->watchdog_state);
	}
}

/**
 * @brief feed and start vcmd watchdog
 * @param enum WD_FEED_MODE mode: watch-dog feed mode
 */
static void _vcmd_watchdog_feed(struct hantrovcmd_dev *dev, enum WD_FEED_MODE mode) {
	u32 num = 0;
	struct vcmd_timer *tm = &dev->vcmd_timer[VCMD_TIMER_WATCHDOG];

	num = dev_wait_job_num(dev);
	if (num == 0) {
		_vcmd_watchdog_stop(dev);
		return;
	}

	if (dev->hw_feature.has_arbiter)
		tm->period = VCMD_ARBITER_RESET_TIME;
	else
		tm->period = num * ONE_JOB_WAIT_TIME;

	switch (mode) {
	case WD_FEED_ACTIVE: {
		if (dev->watchdog_state != WD_STATE_ACTIVE) {
			_vcmd_watchdog_start(dev);
			return;
		}
		break;
	}
	case WD_FEED_APPEND: {
		if (dev->watchdog_state == WD_STATE_PAUSE)
			return;

		if (dev->watchdog_state == WD_STATE_STOPPED) {
			_vcmd_watchdog_start(dev);
			return;
		}
		break;
	}
	case WD_FEED_RESUME: {
		if (dev->watchdog_state == WD_STATE_ACTIVE || dev->watchdog_state == WD_STATE_STOPPED) {
			vcmd_klog(LOGLVL_ERROR, "%s: resume watch-dog, but watch-dog status is %d", __func__, dev->watchdog_state);
			return;
		}
		if (dev->watchdog_state == WD_STATE_PAUSE) {
			_vcmd_watchdog_start(dev);
			return;
		}
			break;
	}
	default: {
		vcmd_klog(LOGLVL_ERROR, "%s: watch-dog feed mode error!\n", __func__);
		return;
	}
	}

	_vcmd_mod_timer(dev, VCMD_TIMER_WATCHDOG);
}
#endif //SUPPORT_WATCHDOG

/*******************************************************************
 *  vcmd timer related functions
 *******************************************************************/
/**
 * @brief timer callback of vcmd timer
 */
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static void _vcmd_timer_cb(unsigned long arg)
#else
static void _vcmd_timer_cb(struct timer_list *timer)
#endif
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	struct timer_list *timer = (struct timer_list *)arg;
#endif
	vcmd_mgr_t *vcmd_mgr;
	struct hantrovcmd_dev *dev;
	struct vcmd_timer *this_timer;
	unsigned long flags;

	this_timer = container_of(timer, struct vcmd_timer, timer);
	dev = (struct hantrovcmd_dev *)this_timer->owner;

	spin_lock_irqsave(dev->spinlock, flags);
	vcmd_mgr = (vcmd_mgr_t *)dev->handler;
	this_timer->expired = 1;
	spin_unlock_irqrestore(dev->spinlock, flags);

	_vcmd_kthread_wakeup(vcmd_mgr);
}

/**
 * @brief init vcmd timer
 */
static void _vcmd_init_timer(struct hantrovcmd_dev *dev)
{
	struct vcmd_timer *tm;
	int i;

	for (i = 0; i < VCMD_TIMER_MAX; i++) {
		tm = &dev->vcmd_timer[i];
		tm->id = i;
		tm->active = 0;
		tm->expired = 0;
		tm->period = 0;
		tm->owner = (void *)dev;
		tm->func = _vcmd_timer_cb;
	}

#ifdef TIMEOUT_IRQ_TIMER
	tm = &dev->vcmd_timer[VCMD_TIMER_TIMEOUT];
	tm->period = VCMD_TIMER_TIMEOUT_INTERVAL;
#endif

	tm = &dev->vcmd_timer[VCMD_TIMER_BUFFER_EMPTY];
	tm->period = VCMD_TIMER_BUFFER_EMPTY_INTERVAL;
}

/**
 * @brief modify vcmd timer by the specific vcmd timer id
 */
static void _vcmd_mod_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id)
{
	struct vcmd_timer *tm;

	if (id >= VCMD_TIMER_MAX) {
		vcmd_klog(LOGLVL_ERROR, "%s: use error id to delete vcmd timer!\n",
				__func__);
		return;
	}

	tm = &dev->vcmd_timer[id];

	if (tm->active)
		mod_timer(&tm->timer, (jiffies + tm->period * HZ / 1000));
}

/**
 * @brief add vcmd timer by the specific vcmd timer id
 * @return 0: added a timer; < 0: add failed
 */
int _vcmd_add_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id)
{
	struct vcmd_timer *tm;
	struct timer_list *timer;

	if (id >= VCMD_TIMER_MAX) {
		vcmd_klog(LOGLVL_ERROR, "%s: use error id to add vcmd timer!\n", __func__);
		return -1;
	}

	tm = &dev->vcmd_timer[id];

	if (tm->period == 0) {
		vcmd_klog(LOGLVL_WARNING, "%s: period of timer %d is zero!\n", __func__, id);
		return -2;
	}

	if (tm->active) {
		_vcmd_mod_timer(dev, id);
		return 0;
	}

	tm->active = 1;

	timer = &tm->timer;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	init_timer(timer);
	timer->function = tm->func;
	timer->expires = jiffies + tm->period * HZ / 1000;
	timer->data = (unsigned long)timer;
#else
	timer_setup(timer, tm->func, 0);
	//the expires time is time_interval ms
	timer->expires = jiffies + tm->period * HZ / 1000;
#endif

	add_timer(timer);

	return 0;
}

/**
 * @brief delete vcmd timer by the specific vcmd timer id
 */
static void _vcmd_del_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id)
{
	struct vcmd_timer *tm;

	if (id >= VCMD_TIMER_MAX) {
		vcmd_klog(LOGLVL_ERROR, "%s: use error id to delete vcmd timer!\n", __func__);
		return;
	}

	tm = &dev->vcmd_timer[id];

	if (tm->active) {
		del_timer(&tm->timer);
		tm->active = 0;
	}
}

void _dbg_log_instr(u32 offset, u32 instr, u32 *size, char *str)
{
	u32 opcode = instr & OPCODE_MASK;

	if (opcode == OPCODE_WREG) {
		int length = ((instr >> 16) & 0x3FF);

		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s %s %d 0x%x]\n",
			offset, instr, "WREG", ((instr >> 26) & 0x1) ? "FIX" : "",
			   length, (instr & 0xFFFF));
		*size = ((length + 2) >> 1) << 1;
	} else if (opcode == OPCODE_END) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s]\n", offset,
			instr, "END");
		*size = 2;
	} else if (opcode == OPCODE_NOP) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s]\n", offset,
			instr, "NOP");
		*size = 2;
	} else if (opcode == OPCODE_RREG) {
		int length = ((instr >> 16) & 0x3FF);

		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s %s %d 0x%x]\n",
			offset, instr, "RREG", ((instr >> 26) & 0x1) ? "FIX" : "",
			   length, (instr & 0xFFFF));
		*size = 4;
	} else if (opcode == OPCODE_JMP) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s %s %s]\n",
			offset, instr, "JMP", ((instr >> 26) & 0x1) ? "RDY" : "",
			   ((instr >> 25) & 0x1) ? "IE" : "");
		*size = 4;
	} else if (opcode == OPCODE_STALL) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s %s 0x%x]\n",
			offset, instr, "STALL", ((instr >> 26) & 0x1) ? "IM" : "",
			   (instr & 0xFFFF));
		*size = 2;
	} else if (opcode == OPCODE_CLRINT) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s %d 0x%x]\n",
			offset, instr, "CLRINT", (instr >> 25) & 0x3,
			   (instr & 0xFFFF));
		*size = 2;
	} else if (opcode == OPCODE_M2M) {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s]\n",
			offset, instr, "M2M");
		*size = 6;
	} else {
		snprintf(str, 512, "current cmdbuf data %d = 0x%08x => [%s]\n",
			offset, instr, "UNKNOWN CMD");
		*size = 1;
	}
}


#ifdef USER_DEBUG
void dump_brief_regs(struct hantrovcmd_dev *dev, u32 dump) {
	u32 i = 0;
	u32 reg_val = 0;
	u64 base_lsb = 0;
	u64 base_msb = 0;
	u64 base = 0;
	if (dev == NULL)
		return;
	if (dev->state == VCMD_STATE_POWER_OFF)
		return;
	volatile void *vcmd_regs = dev->subsys_info->hwregs[SUB_MOD_VCMD];
	volatile void *vc9000d_regs = dev->subsys_info->hwregs[SUB_MOD_MAIN];
	if (vcmd_regs == NULL || vc9000d_regs == NULL)
		return;

	if (dump & DUMP_VCMD_REG) {
		for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
			reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + i * 4));
			vcmd_klog(LOGLVL_ERROR, "vcmd swreg%d: 0x%x\n", i, reg_val);
		}
	}

	if (dump & DUMP_CORE_REG) {
		for (i = 0; i < MAX_REG_COUNT; i++) {
			reg_val = (u32)ioread32((void __iomem *)(vc9000d_regs + i * 4));
			vcmd_klog(LOGLVL_ERROR, "vc9000d swreg%d: 0x%x\n", i, reg_val);
		}
	}

	if (dump & DUMP_AXIFE_REG) {
		if (dev->subsys_info->hwregs[SUB_MOD_AXIFE]) {
			volatile void *axife_regs = dev->subsys_info->hwregs[SUB_MOD_AXIFE];
			for (i = 0; i < 80; i++) { //AXIFE_REGISTER_SIZE too large, only 80 registers exist in our register.xml
				reg_val = (u32)ioread32((void __iomem *)(axife_regs + i * 4));
				vcmd_klog(LOGLVL_ERROR, "axife swreg%d: 0x%x\n", i, reg_val);
			}
		}
	}

	if (dump & DUMP_STATE_REG) {
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_WORK_STATE));
		switch (reg_val & 0x7) {
			case 0:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x (idle)\n", reg_val);
				break;
			case 1:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x (work)\n", reg_val);
				break;
			case 2:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x (stall)\n", reg_val);
				break;
			case 3:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x (pend)\n", reg_val);
				break;
			case 4:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x (abort)\n", reg_val);
				break;
			default:
				vcmd_klog(LOGLVL_ERROR, "vcmd state: 0x%x\n", reg_val);
				break;
		}
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_ABORT_STATE));
		vcmd_klog(LOGLVL_ERROR, "vcmd abort state: %u, start trigger: %u\n",
			(reg_val & 0x8), (reg_val & 0x1));
	}

	if (dump & DUMP_INTERRUPT_REG) {
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_SRC));
		vcmd_klog(LOGLVL_ERROR, "vcmd interrupt source: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_ENABLE));
		vcmd_klog(LOGLVL_ERROR, "vcmd interrupt enbale: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_TRIGGER));
		vcmd_klog(LOGLVL_ERROR, "vcmd interrupt trigger: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_GATE));
		vcmd_klog(LOGLVL_ERROR, "vcmd interrupt gate: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_INTERRUPT));
		vcmd_klog(LOGLVL_ERROR, "vc9000d interrupt: 0x%x", reg_val);
	}

	/*for bus error of vc9000d*/
	if (dump & DUMP_BASE_INFO) {
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_LUMA_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_LUMA_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_ERROR, "luma iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_CHROMA_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_CHROMA_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_ERROR, "chroma iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_STREAM_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_STREAM_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_ERROR, "stream iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_1_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_1_BUFFER_MSB));
		base = base_lsb + (((base_msb >> 16) & 0xFF) << 32); //bit 16:23 of base_msb is msb of iova
		vcmd_klog(LOGLVL_ERROR, "slice text 1 iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_2_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_2_BUFFER_MSB));
		base = base_lsb + (((base_msb >> 16) & 0xFF) << 32); //bit 16:23 of base_msb is msb of iova
		vcmd_klog(LOGLVL_ERROR, "slice text 2 iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_3_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_3_BUFFER_MSB));
		base = base_lsb + (((base_msb >> 16) & 0xFF) << 32); //bit 16:23 of base_msb is msb of iova
		vcmd_klog(LOGLVL_ERROR, "slice text 3 iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_4_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000d_regs + VC9000D_SLICE_TEXT_4_BUFFER_MSB));
		base = base_lsb + (((base_msb >> 16) & 0xFF) << 32); //bit 16:23 of base_msb is msb of iova
		vcmd_klog(LOGLVL_ERROR, "slice text 4 iova: 0x%llx", base);
	}

	return;
}
#else
void dump_brief_regs(struct hantrovcmd_dev *dev, u32 dump) {}
#endif


#ifdef VCMD_DEBUG_INTERNAL
static void _dbg_log_last_cmd(struct cmdbuf_obj *obj) {
	u32 *p, len, offset, size;
	char log_buf[512];

	vcmd_klog(LOGLVL_DEBUG, "last cmdbuf content: \n");
	if (obj->has_jmp_cmd) {
		p = (void *)_get_jmp_cmd(obj);
		len = sizeof(struct cmd_end_t) / sizeof (u32);
	} else {
		p = (void *)_get_end_cmd(obj);
		len = sizeof(struct cmd_end_t) / sizeof (u32);
	}

	offset = p - obj->cmd_va;
	_dbg_log_instr(offset, *p, &size, log_buf);
	vcmd_klog(LOGLVL_DEBUG, "%s", log_buf);
	len--;
	offset++;

	while (len--) {
		vcmd_klog(LOGLVL_DEBUG, "current cmdbuf data %d = 0x%x\n",
						offset, *(obj->cmd_va + offset));
		offset++;
	}
}

static void _dbg_log_cmdbuf(struct cmdbuf_obj *obj) {
	u32 i, instr = 0, size = 0;
	char log_buf[512];

	vcmd_klog(LOGLVL_DEBUG, "vcmd link, current cmdbuf content\n");
	for (i = 0; i < obj->cmdbuf_size / 4; i++) {
		if (i == instr) {
			//memset(log_buf, 0, sizeof(log_buf));
			_dbg_log_instr(i, *(obj->cmd_va + i), &size, log_buf);
			vcmd_klog(LOGLVL_DEBUG, "%s", log_buf);
			instr += size;
		} else {
			vcmd_klog(LOGLVL_DEBUG, "current cmdbuf data %d = 0x%x\n",
					i, *(obj->cmd_va + i));
		}
	}
}

static void _dbg_log_dev_regs(struct hantrovcmd_dev *dev, u32 dump) {
	u32 i, reg_val;

	if (dump) {
		for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
			reg_val = vcmd_read_reg((const void *)dev->hwregs, i * 4);
			vcmd_klog(LOGLVL_DEBUG, "vcmd swreg%d: 0x%x\n", i, reg_val);
		}
	} else {
		for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
			reg_val = *(dev->reg_mem_va + i);
			vcmd_klog(LOGLVL_DEBUG, "ddr vcmd swreg%d: 0x%x\n", i, reg_val);
		}
	}
}
#endif

static void printk_vcmd_register_debug(const void *hwregs, char *info)
{
#ifdef VCMD_DEBUG_INTERNAL
	u32 i, fordebug;

	for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
		fordebug = vcmd_read_reg((const void *)hwregs, i * 4);
		vcmd_klog(LOGLVL_DEBUG, "%s vcmd register %d:0x%x\n", info, i,
			fordebug);
	}
#endif
}

static void _log_ioctl_cmd(unsigned int cmd)
{
	static unsigned int last_cmd = 0;

	if ((cmd == HANTRO_VCMD_IOCH_POLLING_CMDBUF) && (last_cmd == cmd)) {
		return;
	}
	last_cmd = cmd;
	vcmd_klog(LOGLVL_DEBUG, "ioctl cmd 0x%08x\n", cmd);
}

/*======================= cmdbuf object management ================*/

/**
 * @brief initialize all of cmdbuf objs of vcmd driver handler.
 */
static void vcmd_init_objs(vcmd_mgr_t *vcmd_mgr) {
	u32 i;
	struct cmdbuf_obj *obj;
	struct noncache_mem *m0, *m1;

	m0 = &vcmd_mgr->mem_vcmd;
	m1 = &vcmd_mgr->mem_status;
	for(i=0; i<SLOT_NUM_CMDBUF; i++) {
		obj = &vcmd_mgr->objs[i];
		obj->cmdbuf_id = i;
		obj->cmdbuf_size = SLOT_SIZE_CMDBUF;
		obj->cmd_va = m0->va + CMDBUF_OFF_32(i);
		obj->cmd_pa = m0->pa + CMDBUF_OFF(i);
		obj->mmu_cmd_ba = m0->mmu_ba + CMDBUF_OFF(i);
		obj->po = NULL;

		obj->status_size = SLOT_SIZE_STATUSBUF;
		obj->status_va = m1->va + STATUSBUF_OFF_32(i);
		obj->status_pa = m1->pa + STATUSBUF_OFF(i);
		obj->mmu_status_ba = m1->mmu_ba + STATUSBUF_OFF(i);
	}
}

/**
 * @brief initialize all of cmdbuf nodes of vcmd driver handler.
 */
static void vcmd_init_nodes(vcmd_mgr_t *vcmd_mgr) {
	u32 i;

	for(i=0; i<SLOT_NUM_CMDBUF; i++) {
		vcmd_mgr->nodes[i].data = (void *)&vcmd_mgr->objs[i];
	}
}

/**
 * @brief reset flag/status of cmdbuf obj specified by id.
 */
static void reset_cmdbuf_obj(vcmd_mgr_t *vcmd_mgr, u32 id)
{
	struct cmdbuf_obj *obj = &vcmd_mgr->objs[id];

	obj->executing_status = CMDBUF_EXE_STATUS_OK;
	obj->cmdbuf_run_done = 0;
	obj->slice_run_done = 0;
	obj->cmdbuf_linked = 0;
	obj->cmdbuf_need_remove = 0;
	obj->cmdbuf_processed = 0;
	obj->core_id = 0xFFFF;
	obj->cmdbuf_size = SLOT_SIZE_CMDBUF;
	obj->xring_slice_mode = 0;

	obj->has_jmp_cmd = 1;
	obj->jmp_ie = 0;

	obj->clk_rate = 0;

	atomic_set(&obj->need_restart, 0);
}

/**
 * @brief reset pointer of cmdbuf node specified by id.
 */
static void reset_cmdbuf_node(vcmd_mgr_t *vcmd_mgr, u32 id)
{
	struct bi_list_node  *node;

	node = &vcmd_mgr->nodes[id];
	node->next = NULL;
	node->prev = NULL;

	node = &vcmd_mgr->po_jobs[id];
	node->next = NULL;
	node->prev = NULL;
	node->data = NULL;
}

/**
 * @brief put a node to tail of si-list.
 */
static void _si_list_put(struct si_linked_list *list,
							struct si_linked_node *node) {
	if (list->head == NULL) {
		list->head = list->tail = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}
	node->next = NULL;
}

/**
 * @brief get & remove a node from head of si-list.
 * @return struct si_linked_node *: NULL: no node got; other: the node.
 */
static struct si_linked_node * _si_list_get(struct si_linked_list *list) {
	struct si_linked_node *node = list->head;

	if (list->head) {
		list->head = node->next;
		if (list->head == NULL) {
			list->tail = NULL;
		}
	}

	return node;
}

/**
 * @brief init a si-list.
 */
static void vcmd_init_si_list(struct si_linked_list *list) {

	spin_lock_init(&list->spinlock);
	list->head = NULL;
	list->tail = NULL;
}

/**
 * @brief init free obj (cmdbuf) list of vcmd driver handler.
 */
static void vcmd_init_free_obj_list(vcmd_mgr_t *vcmd_mgr) {
	u32 i;
	struct si_linked_list *list = &vcmd_mgr->free_obj_list;

	vcmd_init_si_list(list);
	sema_init(&vcmd_mgr->free_obj_sema, SLOT_NUM_CMDBUF);

	for(i=0; i<SLOT_NUM_CMDBUF; i++) {
		list->nodes[i].data = (void *)&vcmd_mgr->objs[i];
		_si_list_put(list, &list->nodes[i]);
	}
}

/**
 * @brief acquire a free cmdbuf.
 * @param u32 *id: to store acquired free cmdbuf id.
 * @return int: 0: succeed; otherwise: failed.
 */
static int acquire_cmdbuf(vcmd_mgr_t *vcmd_mgr, u32 *id)
{
	struct si_linked_list *list = &vcmd_mgr->free_obj_list;
	struct si_linked_node *node;
	struct cmdbuf_obj *obj;

	if (down_killable(&vcmd_mgr->free_obj_sema)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!");
		return -EINTR;
	}

	spin_lock(&list->spinlock);
	node = _si_list_get(list);
	if (node == NULL) {
		vcmd_klog(LOGLVL_ERROR, "%s: get NULL node!", __func__);
		spin_unlock(&list->spinlock);
		return -EINVAL;
	} else {
		obj = (struct cmdbuf_obj *)node->data;
		obj->is_occupied = 1;
		*id = obj->cmdbuf_id;
	}
	spin_unlock(&list->spinlock);

	return 0;
}

/**
 * @brief return (release) a cmdbuf specified by id.
 */
static void return_cmdbuf(vcmd_mgr_t *vcmd_mgr, u32 id)
{
	struct si_linked_list *list = &vcmd_mgr->free_obj_list;
	struct si_linked_node *node = &list->nodes[id];
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;
	u32 is_free = 0;

	obj->po = NULL;

	spin_lock(&list->spinlock);
	if (obj->is_occupied == 0) {
		is_free = 1;
	} else {
		_si_list_put(list, node);
		obj->is_occupied = 0;
	}
	spin_unlock(&list->spinlock);
	if (is_free == 0)
		up(&vcmd_mgr->free_obj_sema);
}

/**
 * @brief get cmdbuf node by cmd physical addr.
 * @param addr_t pa: the cmd physical addr.
 * @return bi_list_node *: NULL: no cmdbuf found; otherwise: cmdbuf node.
 */
static bi_list_node *get_cmdbuf_node_by_addr(vcmd_mgr_t *vcmd_mgr,
									struct hantrovcmd_dev *dev, addr_t pa)
{
	bi_list_node *node;
	struct cmdbuf_obj *obj;

	u32 id;
	addr_t pa_base = vcmd_mgr->mem_vcmd.pa - vcmd_mgr->pa_trans_offset;

	id = (pa - pa_base) / SLOT_SIZE_CMDBUF;
	if (CMDBUF_ID_CHECK(id) < 0)
		return NULL;

	obj = &vcmd_mgr->objs[id];
	node = &vcmd_mgr->nodes[id];

	if (obj->core_id != dev->core_id) {
		vcmd_klog(LOGLVL_ERROR, "cmdbuf is not in dev[%d] list !!\n", dev->core_id);
		return NULL;
	}

	return node;
}

/**
 * @brief get the va of vcmdbuf obj's JMP cmd.
 */
static struct cmd_jmp_t *_get_jmp_cmd(struct cmdbuf_obj *obj) {
	u8 *p = (u8 *)obj->cmd_va;
	p += obj->cmdbuf_size - sizeof(struct cmd_jmp_t);
	return (struct cmd_jmp_t *)p;
}

/**
 * @brief get the va of vcmdbuf obj's END cmd.
 */
static struct cmd_end_t *_get_end_cmd(struct cmdbuf_obj *obj) {
	u8 *p = (u8 *)obj->cmd_va;
	p += obj->cmdbuf_size - sizeof(struct cmd_end_t);
	return (struct cmd_end_t *)p;
}

/**
 * @brief set jmp cmd with JMP_RDY and obj cmd mem ba and cmdbuf id,
 * the jmp cmd is the 1st cmd of specified cmdbuf.
 */
static void _set_jmp_cmd(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	addr_t cmd_mem_ba;
	struct cmd_jmp_t *cmd_jmp;
	u16 jmp_len = 0;

	cmd_mem_ba = obj->cmd_pa - dev->pa_trans_offset;
	if (dev->mmu_enable)
		cmd_mem_ba = (addr_t)obj->mmu_cmd_ba;

	/* config first jmp cmd:
	 *    JMP_RDY
	 *    next cmdbuf addr
	 *    next cmdbuf id
	 */
	jmp_len = sizeof(struct cmd_jmp_t);
	cmd_mem_ba += jmp_len;
	cmd_jmp = (struct cmd_jmp_t *)obj->cmd_va;
	CMD_SET_ADDR(cmd_jmp, cmd_mem_ba);
	cmd_jmp->opcode = OPCODE_JMP | JMP_RDY(1) |
		JMP_NEXT_LEN((obj->cmdbuf_size + 7) / 8 - jmp_len / 8);
	cmd_jmp->id = obj->cmdbuf_id;
}

/**
 * @brief set bus-address to 2 rreg cmd with dev reg mem ba,
 * the 2 rreg cmds are the 1st/2nd & the last but one cmd of specified cmdbuf.
 */
static void _set_rreg_addr(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj) {
	addr_t reg_mem_ba;
	struct cmd_rreg_t *cmd_rreg;
	u8 *p;
	u16 jmp_len = 0;

	if (dev->hw_version_id <= HW_ID_1_0_C)
		return;

	reg_mem_ba = dev->reg_mem_ba;
	if (dev->mmu_enable) {
		reg_mem_ba = (addr_t)dev->mmu_reg_mem_ba;
	}

	/* first jmp cmd length */
	if (obj->has_jmp_cmd)
		jmp_len = sizeof(struct cmd_jmp_t);
	//read vcmd executing ID register into ddr memory.
	p = (u8 *)obj->cmd_va;
	cmd_rreg = (struct cmd_rreg_t *)(p + jmp_len);
	CMD_SET_ADDR(cmd_rreg, reg_mem_ba + REG_ID2_CMDBUF_EXE_ID * 4);

	//read vcmd all registers into ddr memory.
	if (obj->has_jmp_cmd)
		p = (u8 *)_get_jmp_cmd(obj);
	else
		p = (u8 *)_get_end_cmd(obj);
	cmd_rreg = (struct cmd_rreg_t *)(p - sizeof(struct cmd_rreg_t));
	CMD_SET_ADDR(cmd_rreg, reg_mem_ba);
}

/**
 * @brief set cmds of specified cmdbuf.
 */
static void _set_cmds(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	/* set first jmp cmd */
	if (obj->has_jmp_cmd)
		_set_jmp_cmd(dev, obj);
	/* set 1st/2nd rreg and last cmd bus address */
	_set_rreg_addr(dev, obj);
}

/**
 * @brief update intr-enable bit of JMP cmd in specified cmdbuf.
 */
static void _update_jmp_ie(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj) {
	struct cmd_jmp_t *cmd_jmp;
	if (obj->has_jmp_cmd) {
		//update dev->duration and adjust JMP_IE accordingly
		if (obj->jmp_ie) {
			dev->duration = 0;
		} else {
			dev->duration += obj->workload;
			if (dev->duration >= VCMD_INTR_INTERVAL) {
				cmd_jmp = _get_jmp_cmd(obj);
				obj->jmp_ie = 1;
				cmd_jmp->opcode |= JMP_IE(1);
			}
		}
	}
}

/**
 * @brief link 2 specified cmdbufs' data by JMP cmd of prev comdbuf.
 */
static void dev_link_cmdbuf(struct hantrovcmd_dev *dev,
								bi_list_node *prev_node,
								bi_list_node *next_node) {
	struct cmdbuf_obj *next_obj, *prev_obj;
	struct cmd_jmp_t *cmd_jmp;
	addr_t next_ba;
	u32 op;

	if (!prev_node)
		return;

	prev_obj = (struct cmdbuf_obj *)prev_node->data;

	if (prev_obj->has_jmp_cmd) {
		cmd_jmp = _get_jmp_cmd(prev_obj);
		if (!next_node) {
			// If next cmdbuf is not available, set the RDY to 0.
			u32 op = cmd_jmp->opcode;
			cmd_jmp->opcode = OPCODE_JMP |
								JMP_RDY(0) | JMP_IE(JMP_G_IE(op)) |
								JMP_NEXT_LEN(0);
		} else {
			next_obj = (struct cmdbuf_obj *)next_node->data;
			if (dev->hw_version_id > HW_ID_1_0_C) {
				//set next cmdbuf id
				cmd_jmp->id = next_obj->cmdbuf_id;
			}
			next_ba = next_obj->cmd_pa - dev->pa_trans_offset;
			if (dev->mmu_enable) {
				next_ba = (addr_t)next_obj->mmu_cmd_ba;
			}

			CMD_SET_ADDR(cmd_jmp, next_ba);
			op = cmd_jmp->opcode;
			cmd_jmp->opcode = OPCODE_JMP |
								JMP_RDY(1) | JMP_IE(JMP_G_IE(op)) |
								JMP_NEXT_LEN(sizeof(struct cmd_jmp_t) / 8);
		}
	}

#ifdef VCMD_DEBUG_INTERNAL
	_dbg_log_last_cmd(prev_obj);
#endif
}

/**
 * @brief create a process object
 */
static struct proc_obj *create_process_object(void) {
	struct proc_obj *po;

	po = vmalloc(sizeof(struct proc_obj));
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "vmalloc failed!\n");
		return NULL;
	}

	memset(po, 0, sizeof(struct proc_obj));
	spin_lock_init(&po->spinlock);
	init_waitqueue_head(&po->resource_waitq);
	init_waitqueue_head(&po->job_waitq);

	spin_lock_init(&po->job_lock);
	init_bi_list(&po->job_done_list);
	return po;
}

/**
 * @brief free a process object
 */
static void free_process_object(struct proc_obj *po) {
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "po is NULL!\n");
		return;
	}
	vfree(po);
}

/**
 * @brief add done obj to job_done_list of its po, wake-up job_waitq if needed.
 */
static void proc_add_done_job(vcmd_mgr_t *vcmd_mgr, struct cmdbuf_obj *obj) {
	u16 id = obj->cmdbuf_id;
	struct proc_obj *po;
	struct bi_list *list;
	u32 is_empty, is_wait;
	struct hantrovcmd_dev *dev;

	if (!obj->po) {
		vcmd_klog(LOGLVL_INFO, "%s: the po and cmdbufs of this po has been released!\n",
						__func__);
		return;
	}

	po = obj->po;
	list = &po->job_done_list;
	if (obj->core_id > 0) {
		/* should not happen */
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) > 0!\n",
		    __func__, obj->core_id);
		return;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];

	spin_lock(&po->job_lock);

	if (vcmd_mgr->po_jobs[id].data) {
		//already in job-done list, do nothing
		spin_unlock(&po->job_lock);
		return;
	}

	vcmd_mgr->po_jobs[id].data = (void *)&vcmd_mgr->objs[id];
	is_empty = (list->head == NULL);
	is_wait = po->in_wait;
	po->in_wait = 0;
	bi_list_insert_node_tail(list, &vcmd_mgr->po_jobs[id]);

	spin_unlock(&po->job_lock);

	if (is_empty || is_wait) {
		wake_up_interruptible_all(&po->job_waitq);
	}
}

/**
 * @brief get & remove a done obj from po's job_done_list.
 * @param struct cmdbuf_obj **pobj: *pobj==NULL: get head node from list.
 * 									otherwise, get specified node from list.
 * @return int: 0: no done obj; 1: obj is done.
 */
static int proc_get_done_job(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
									struct cmdbuf_obj **pobj) {

	struct bi_list *list = &po->job_done_list;
	bi_list_node *node;
	struct cmdbuf_obj *obj = NULL;
	int is_done = 0, i;

	spin_lock(&po->job_lock);
	node = list->head;

	if (node == NULL) {
		// job done list is empty
		po->in_wait = 1;
		for(i=0; i<SLOT_NUM_CMDBUF; i++) {
			obj = &vcmd_mgr->objs[i];
			if (obj->po == po && obj->cmdbuf_processed == 0)
				break;
		}
		if (i == SLOT_NUM_CMDBUF) {
			spin_unlock(&po->job_lock);
			return 1; /* used to drop_cmd_ids!=NULL when seek */
		}
		spin_unlock(&po->job_lock);

		return 0;
	}

	if (*pobj == NULL) {
		//any po's cmdbuf ready, return head of job_done_list
		*pobj = (struct cmdbuf_obj *)node->data;
		is_done = 1;
	} else {
		//specified cmdbuf ready?
		obj = *pobj;
		if (obj->cmdbuf_run_done || (obj->slice_run_done == 2)) {
			int loglvl = obj->slice_run_done ? LOGLVL_DEBUG : LOGLVL_INFO;
			// check if the obj is in job done list
			while (node && node->data != (void *)obj) {
				node = node->next;
			}
			if (node == NULL) {
				vcmd_klog(loglvl, "cmdbuf[%d] is done, but not in done-list!\n",
						obj->cmdbuf_id);
			} else {
				vcmd_klog(loglvl, "cmdbuf[%d] is done!\n",
					obj->cmdbuf_id);
				is_done = 1;
			}
		}
	}

	if (is_done) {
		bi_list_remove_node(list, &vcmd_mgr->po_jobs[(*pobj)->cmdbuf_id]);
		vcmd_mgr->po_jobs[(*pobj)->cmdbuf_id].data = NULL;
	} else {
		po->in_wait = 1;
	}

	spin_unlock(&po->job_lock);

	return is_done;
}

/**
 * @brief remove a job (cmdbuf node) from device work_list,
 * and de-link from cmdbuf jmp list
 * @return int: 1: succeed; 0: failed, the node's JMP cmd is still used by hw.
 */
static int dev_delink_job(struct hantrovcmd_dev *dev,
											bi_list_node *node,
											vcmd_mgr_t *vcmd_mgr)
{
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;
	unsigned long flags;

	if (obj->cmdbuf_linked == 0) {
		//already de-linked or not link into work list yet
		return 1;
	}

	spin_lock_irqsave(dev->spinlock, flags);
	if (dev->state != VCMD_STATE_WORKING ||
		obj->has_jmp_cmd == 0 || node->next) {
		spin_unlock_irqrestore(dev->spinlock, flags);
		//delink cmdbuf, and remove node from work list.
		dev_link_cmdbuf(dev, node->prev, node->next);
		bi_list_remove_node(&dev->work_list, node);
		obj->cmdbuf_linked = 0;
#ifdef CONFIG_DEC_CM_RUNTIME
		if (runtime_cm_enable && obj->clk_rate){
			struct hantrodec_dev* dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
			if(!dec_dev){
				vdec_cm_klog(LOGLVL_ERROR,"dec_dev is NULL!\n");
				return 1;
			}
			vdec_clk_mgr *cm = dec_dev->cm;
			if(!cm){
				vdec_cm_klog(LOGLVL_ERROR,"vdec_clk_mgr is NULL!\n");
				return 1;
			}
			vdec_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_POP);
		} 
#endif
		return 1;
	} else {
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	// Not remove the last node, its JMP cmd is needed to link new cmd.
	return 0;
}

/**
 * @brief remove a job (cmdbuf node) from device work_list,
 * and de-link from cmdbuf jmp list
 */
static int dev_delink_job2(struct hantrovcmd_dev *dev,
											bi_list_node *node)
{
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;

	if (obj->cmdbuf_linked == 0) {
		//already de-linked or not link into work list yet
		return 0;
	}

	//delink cmdbuf, and remove node from work list.
	dev_link_cmdbuf(dev, node->prev, node->next);
	bi_list_remove_node(&dev->work_list, node);
	obj->cmdbuf_linked = 0;

	return 0;
}

/**
 * @brief add a job to tail of device work_list,
 *  and link to cmdbuf jmp list if needed.
 */
static int dev_add_job(struct hantrovcmd_dev *dev, bi_list_node *job_node, vcmd_mgr_t *vcmd_mgr) {

	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)job_node->data;
	_set_cmds(dev, obj);

	/* Check if the link list has the same cmdbuf_id. */
	if (enable_dev_list_check) {
		vdec_cm_klog(LOGLVL_DEBUG, "check list enable!\n");
		check_list_insert(&dev->work_list, obj, __func__);
	}

	bi_list_insert_node_tail(&dev->work_list, job_node);
	_update_jmp_ie(dev, obj);

#ifdef SUPPORT_DBGFS
	_dbgfs_record_active_start_time(dev->dbgfs_info);
#endif

	dev_link_cmdbuf(dev, job_node->prev, job_node);
	obj->core_id = dev->core_id;
	obj->cmdbuf_linked = 1;

#ifdef CONFIG_DEC_CM_RUNTIME
	if (runtime_cm_enable && obj->clk_rate){
		struct hantrodec_dev* dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
		if(!dec_dev){
			vdec_cm_klog(LOGLVL_ERROR,"dec_dev is NULL!\n");
			return 0;
		}
		vdec_clk_mgr *cm = dec_dev->cm;
		if(!cm){
			vdec_cm_klog(LOGLVL_ERROR,"vdec_clk_mgr is NULL!\n");
			return 0;
		}
		vdec_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_PUSH);
	}
#endif
	return 0;
}

/**
 * @brief insert job_node prior to base_node of dev work_list,
 *  and insert to cmdbuf jmp list accordingly if needed.
 */
static int dev_insert_job(struct hantrovcmd_dev *dev,
							bi_list_node *base_node,
							bi_list_node *job_node,
							vcmd_mgr_t *vcmd_mgr)
{
	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)job_node->data;
	_set_cmds(dev, obj);

	/* Check if the link list has the same cmdbuf_id. */
	if (enable_dev_list_check) {
		vdec_cm_klog(LOGLVL_DEBUG, "check list enable!\n");
		check_list_insert(&dev->work_list, obj, __func__);
	}

	bi_list_insert_node_before(&dev->work_list, base_node, job_node);

#ifdef SUPPORT_DBGFS
	_dbgfs_record_active_start_time(dev->dbgfs_info);
#endif

	dev_link_cmdbuf(dev, job_node->prev, job_node);
	dev_link_cmdbuf(dev, job_node, job_node->next);
	obj->core_id = dev->core_id;
	obj->cmdbuf_linked = 1;

#ifdef CONFIG_DEC_CM_RUNTIME
	if (runtime_cm_enable && obj->clk_rate){
		struct hantrodec_dev *dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
		if(!dec_dev){
			vdec_cm_klog(LOGLVL_ERROR, "dec_dev is NULL!\n");
			return 0;
		}
		vdec_clk_mgr *cm = dec_dev->cm;
		if(!cm){
			vdec_cm_klog(LOGLVL_ERROR, "vdec_clk_mgr is NULL!\n");
			return 0;
		}
		vdec_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_PUSH);
	}
#endif
	return 0;
}

/**
 * @brief remove a job from dev work_list,
 *  de-link it from cmdbuf jmp list if needed, and return the cmdbuf.
 */
static int dev_remove_job(vcmd_mgr_t *vcmd_mgr,
							struct hantrovcmd_dev *dev, bi_list_node *node) {
	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)node->data;

	vcmd_klog(LOGLVL_DEBUG, "Delink and remove cmdbuf [%d] from dev [%d].\n",
			obj->cmdbuf_id, dev->core_id);
	if (node->prev) {
		vcmd_klog(LOGLVL_DEBUG, "prev cmdbuf [%d].\n",
			   ((struct cmdbuf_obj *)node->prev->data)->cmdbuf_id);
	} else {
		vcmd_klog(LOGLVL_DEBUG, "NO prev cmdbuf.\n");
	}
	if (node->next) {
		vcmd_klog(LOGLVL_DEBUG, "next cmdbuf [%d].\n",
			   ((struct cmdbuf_obj *)node->next->data)->cmdbuf_id);
	} else {
		vcmd_klog(LOGLVL_DEBUG, "NO next cmdbuf.\n");
	}

	if (dev_delink_job(dev, node, vcmd_mgr)) {
		return_cmdbuf(vcmd_mgr, obj->cmdbuf_id);
	}

	return 0;
}

/**
 * @brief remove a job from dev work_list,
 *  de-link it from cmdbuf jmp list, and return the cmdbuf.
 */
static int dev_remove_job2(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, bi_list_node *node)
{
	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)node->data;

	vcmd_klog(LOGLVL_INFO, "Delink and remove cmdbuf [%d] from dev [%d].\n",
			obj->cmdbuf_id, dev->core_id);
	if (node->prev) {
		vcmd_klog(LOGLVL_INFO, "prev cmdbuf [%d].\n",
			   ((struct cmdbuf_obj *)node->prev->data)->cmdbuf_id);
	} else {
		vcmd_klog(LOGLVL_INFO, "NO prev cmdbuf.\n");
	}
	if (node->next) {
		vcmd_klog(LOGLVL_INFO, "next cmdbuf [%d].\n",
			   ((struct cmdbuf_obj *)node->next->data)->cmdbuf_id);
	} else {
		vcmd_klog(LOGLVL_INFO, "NO next cmdbuf.\n");
	}

	dev_delink_job2(dev, node);

	return_cmdbuf(vcmd_mgr, obj->cmdbuf_id);

	return 0;
}

/**
 * @brief get remain (un-do) jobs count from dev work_list,
 */
static u32 dev_get_job_num(struct hantrovcmd_dev *dev) {
	struct cmdbuf_obj *obj;
	bi_list_node *node = dev->work_list.head;
	u32 num = 0, num1 = 0;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		if (obj->cmdbuf_run_done == 0) {
			num++;
		}
		node = node->next;
		num1++;
		if (num1 > SLOT_NUM_CMDBUF) {
			/* this function not return.
			 * and dump all cmdbuf_id.
			 */
			hantor_dump_all_cmdbuf_id_by_list(&dev->work_list, __func__, __LINE__);
		}
	}
	return num;
}

/**
 * @brief calculate workload after specified node in dev work_list.
 */
static u64 calc_workload_after_node(bi_list_node *node)
{
	u64 sum = 0;
	struct cmdbuf_obj *obj;
	u32 num = 0;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		sum += obj->workload;
		node = node->next;
		num++;

		if (num > SLOT_NUM_CMDBUF) {
			hantor_dump_cmdbuf_id_by_node(node, __func__, __LINE__);
		}
	}
	return sum;
}

/**
 * @brief check if specified device is in core_mask
 * @return int: 1: device is in core_mask; 0: not in core_mask.
 */
static int is_supported_core(u32 core_mask, u16 dev_id)
{
	if (core_mask && ((core_mask >> dev_id) & 0x1))
		return 1; //found one supported core

	return 0; //not found the supported core
}

struct sub_ip_init_cfg{
	u32 reg_id;
	u32 reg_val;
};

struct sub_ip_init_cfg axife_init_cfg[] = {
	{AXI_REG10_SW_FRONTEND_EN, 0x02},
	{AXI_REG11_SW_WORK_MODE, 0x00},
	{0xffff, }	//end guard
};

struct sub_ip_init_cfg mmu_init_cfg[] = {
	{MMU_REG_ADDRESS, 0x0000},		//not move, sw will update mmu addr LSB to index0
	{MMU_REG_ADDRESS_MSB, 0x0000},	//not move, sw will update mmu addr MSB to index1
	{MMU_REG_PAGE_TABLE_ID, 0x10000},
	{MMU_REG_PAGE_TABLE_ID, 0x00000},
	{MMU_REG_CONTROL, 0x01},
	{0xffff, }	//end guard
};


#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
/**
 * @brief set reg id/val of specified sub-ip to init-cmd regs.
 */
static void _set_module_init_cmds(struct hantrovcmd_dev *dev, u32 module_id,
									struct sub_ip_init_cfg * module_cfg) {
	u16 reg_off = dev->subsys_info->reg_off[module_id];
	u32 opcode = OPCODE_WREG | WREG_MODE(WREG_ADDR_FIX) | WREG_LEN(1);

	if (reg_off == 0xffff)
		return;

	while (module_cfg->reg_id != 0xffff) {
		dev->reg_mirror[dev->init_cmd_idx++] = opcode |
												(reg_off + module_cfg->reg_id);
		dev->reg_mirror[dev->init_cmd_idx++] = module_cfg->reg_val;
		module_cfg++;
	}
}
#endif

/**
 * @brief set init-cmd regs to init sub-ips if necessary.
 */
static void vcmd_set_init_cmds(struct hantrovcmd_dev *dev)
{
#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
	u32 i;

	dev->init_cmd_idx = VCMD_REG_ID_SW_INIT_CMD0;

	//enable AXIFE by VCMD
	_set_module_init_cmds(dev, SUB_MOD_AXIFE, axife_init_cfg);

	//enable MMU by VCMD
	if (dev->mmu_enable) {
		u64 mmu_addr = GetMMUAddress();
		vcmd_klog(LOGLVL_DEBUG, "mmu address = 0x%llx", mmu_addr);
		mmu_init_cfg[0].reg_val = (u32)mmu_addr;
		mmu_init_cfg[1].reg_val = (u32)(mmu_addr >> 32);
		_set_module_init_cmds(dev, SUB_MOD_MMU, mmu_init_cfg);
		_set_module_init_cmds(dev, SUB_MOD_MMU_WR, mmu_init_cfg);
	}

	//finished with END command
	dev->reg_mirror[dev->init_cmd_idx++] = OPCODE_END;
	dev->reg_mirror[dev->init_cmd_idx++] = 0x00;

	for (i=VCMD_REG_ID_SW_INIT_CMD0; i<dev->init_cmd_idx; i++) {
		vcmd_write_reg((const void *)dev->hwregs, i*4, dev->reg_mirror[i]);
	}
#endif
}

/**
 * @brief reset all vcmd hw devices.
 */
void vcmd_reset_asic(void *_vcmd_mgr)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	int i, n;
	u32 status;
	struct hantrovcmd_dev *dev = vcmd_mgr->dev_ctx;

	for (n = 0; n < vcmd_mgr->subsys_num; n++) {
		if (dev[n].hwregs) {
			//disable interrupt at first
			vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
			//reset core
			vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_CONTROL_OFFSET, 0x0004);
			//read status register
			status = vcmd_read_reg((const void *)dev[n].hwregs,
						   VCMD_REGISTER_INT_STATUS_OFFSET);
			//clean status register
			vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_INT_STATUS_OFFSET, status);
			//when reset core need clear reg[3]
			vcmd_write_reg((const void *)dev[n].hwregs,
							VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET, 0x0000);
			for (i = VCMD_REGISTER_CONTROL_OFFSET;
				 i < dev[n].subsys_info->io_size[SUB_MOD_VCMD]; i += 4) {
				//set all register 0
				vcmd_write_reg((const void *)dev[n].hwregs, i,
						   0x0000);
			}
			// gate all external interrupts
			vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_EXT_INT_GATE_OFFSET,
					   dev[n].intr_gate_mask);

			if (dev[n].hw_feature.has_arbiter) {
				vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_ARB_OFFSET,
					   0);

				vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_ARBITER_CONFIG_OFFSET,
					   VCMD_ARBITER_PARAMS(arbiter_weight,
						arbiter_urgent, arbiter_bw_overflow,
						arbiter_timewindow));
			}
		}
	}
}

/**
 * @brief reset core to the specified vcmd hw device.
 */
static void vcmd_reset_current_asic(struct hantrovcmd_dev *dev)
{
	u32 status;
	volatile u8 *hwregs = dev->hwregs;

	if (hwregs) {
		//disable interrupt at first
		vcmd_write_reg((const void *)hwregs,
				   VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
		//reset core
		vcmd_write_reg((const void *)hwregs,
				   VCMD_REGISTER_CONTROL_OFFSET, 0x0004);
		//read status register
		status = vcmd_read_reg((const void *)hwregs,
					   VCMD_REGISTER_INT_STATUS_OFFSET);
		//clean status register
		vcmd_write_reg((const void *)hwregs,
				   VCMD_REGISTER_INT_STATUS_OFFSET, status);
		//when reset core need clear reg[3]
		vcmd_write_reg((const void *)hwregs,
						VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET, 0x0000);
	}
}

/**
 * @brief vcmd get online for arbiter
 */
void vcmd_online_for_arbiter(const volatile u8 *hwregs)
{
	u32 arbiter_cfg;

	arbiter_cfg = ioread32((void __iomem *)(hwregs +
							VCMD_REGISTER_ARBITER_CONFIG_OFFSET));
	iowrite32((arbiter_cfg | VCMD_ARBITER_ENABLE),
			(void __iomem *)(hwregs +
			VCMD_REGISTER_ARBITER_CONFIG_OFFSET));
}

/**
 * @brief vcmd get offline for arbiter
 */
void vcmd_offline_for_arbiter(const volatile u8 *hwregs)
{
	// clear arb_req
	iowrite32(0x2, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
	// vcmd offline
	iowrite32(0, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARBITER_CONFIG_OFFSET));
}

/**
 * @brief vcmd request arbiter manully
 */
void vcmd_request_arbiter(const volatile u8 *hwregs)
{
	iowrite32(0x0, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
	iowrite32(0x1, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
	while ((ioread32((void __iomem *)(hwregs +
		VCMD_REGISTER_ARB_OFFSET)) & 0x4) == 0)
		schedule();
}

/**
 * @brief start a vcmd hw device to run the 1st not-done job in its work list.
 */
static void vcmd_start(struct hantrovcmd_dev *dev)
{
	struct cmdbuf_obj *obj = NULL;
	const void *hwregs = (const void *)dev->hwregs;
	bi_list_node *node;
	u32 *reg_mirror = dev->reg_mirror;
	addr_t cmd_ba;
	u32 ba_msb = 0;

	if (dev->state == VCMD_STATE_POWER_OFF) {
		vcmd_klog(LOGLVL_WARNING, "%s: vcmd is in power off state!\n", __func__);
		return;
	}
	if (dev->state == VCMD_STATE_WORKING) {
		vcmd_klog(LOGLVL_INFO, "vcmd is already in working state!\n");
		return;
	}

	dev->sw_cmdbuf_rdy_num = dev_get_job_num(dev);
	node = dev->work_list.head;
	while (node && ((struct cmdbuf_obj *)node->data)->cmdbuf_run_done)
		node = node->next;

	if (dev->sw_cmdbuf_rdy_num == 0 || node == NULL) {
		vcmd_klog(LOGLVL_DEBUG, "no cmdbuf to start yet!\n");
#ifdef SUPPORT_WATCHDOG
		_vcmd_watchdog_stop(dev);
#endif
		return;
	}

	obj = (struct cmdbuf_obj *)node->data;

	printk_vcmd_register_debug(hwregs, "vcmd start enters");
	vcmd_klog(LOGLVL_INFO, "vcmd start for cmdbuf id %d, cmdbuf_run_done = %d\n",
							obj->cmdbuf_id, obj->cmdbuf_run_done);

	//init HWIF_VCMD_EXE_CMDBUF_COUNT
	vcmd_write_register_value(hwregs, reg_mirror, HWIF_VCMD_EXE_CMDBUF_COUNT, 0);
	//for old hw version which reg EXE_CMDBUF_COUNT is read-only.
	dev->sw_cmdbuf_rdy_num += vcmd_get_register_value((const void *)hwregs,
									reg_mirror, HWIF_VCMD_EXE_CMDBUF_COUNT);

	vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_RDY_CMDBUF_COUNT,
							dev->sw_cmdbuf_rdy_num);

	if (dev->state == VCMD_STATE_POWER_ON) {
		//0x40
	#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
		//when start vcmd, first vcmd is init mode
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_INIT_ENABLE, dev->init_mode);
	#endif
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_AXI_CLK_GATE_DISABLE, 0);
		vcmd_set_reg_mirror(reg_mirror,
						HWIF_VCMD_MASTER_OUT_CLK_GATE_DISABLE, APB_CLK_MODE);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_CORE_CLK_GATE_DISABLE, 0);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_ABORT_MODE, dev->abort_mode);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_RESET_CORE, 0);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_RESET_ALL, 0);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_START_TRIGGER, 0);
		//0x48
		if (dev->hw_feature.has_arbiter) {
			vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_ARBRST_EN, 1);
			vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_ARBERR_EN, 0);
		}
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_JMP_EN, 1);

		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_ABORT_EN, 1);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_CMDERR_EN, 1);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_TIMEOUT_EN, 1);
		/* not report bus err interrup */
		//vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_BUSERR_EN, 1);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_IRQ_ENDCMD_EN, 1);
		//0x4c
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_TIMEOUT_ENABLE, 1);
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_TIMEOUT_CYCLES, 500000000);

		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_MAX_BURST_LEN, 0x10);
	}

	cmd_ba = obj->cmd_pa - dev->pa_trans_offset;
	if (dev->mmu_enable) {
		cmd_ba = (addr_t)obj->mmu_cmd_ba;
	}
	if (sizeof(addr_t) == 8)
		ba_msb = (u32)(cmd_ba >> 32);


	vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_CMDBUF_EXE_ADDR, (u32)cmd_ba);
	vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_CMDBUF_EXE_ADDR_MSB, ba_msb);
	if (obj->has_jmp_cmd)
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_CMDBUF_EXE_LENGTH,
			sizeof(struct cmd_jmp_t) / 8);
	else
		vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_CMDBUF_EXE_LENGTH,
			(obj->cmdbuf_size + 7) / 8);

	if (dev->hw_version_id > HW_ID_1_0_C)
		vcmd_write_register_value(hwregs, reg_mirror,
									HWIF_VCMD_CMDBUF_EXE_ID,
									(u32)obj->cmdbuf_id);

	if (dev->state == VCMD_STATE_POWER_ON) {
		vcmd_write_reg(hwregs, 0x44, vcmd_read_reg(hwregs, 0x44));
		vcmd_write_reg(hwregs, 0x40, reg_mirror[0x40 / 4]);
		vcmd_write_reg(hwregs, 0x48, reg_mirror[0x48 / 4]);
		vcmd_write_reg(hwregs, 0x4c, reg_mirror[0x4c / 4]);
		vcmd_write_reg(hwregs, 0x5c, reg_mirror[0x5c / 4]);
		if (dev->hw_version_id >= HW_ID_1_2_1)
			vcmd_set_init_cmds(dev);
	}

	vcmd_write_reg(hwregs, 0x50, reg_mirror[0x50 / 4]);
	vcmd_write_reg(hwregs, 0x54, reg_mirror[0x54 / 4]);
	vcmd_write_reg(hwregs, 0x58, reg_mirror[0x58 / 4]);
	vcmd_write_reg(hwregs, 0x60, reg_mirror[0x60 / 4]);

	dev->state = VCMD_STATE_WORKING;

	//start
	vcmd_set_reg_mirror(reg_mirror, HWIF_VCMD_START_TRIGGER, 1);
	vcmd_write_reg(hwregs, 0x40, reg_mirror[0x40 / 4]);

#ifdef SUPPORT_WATCHDOG
	_vcmd_watchdog_feed(dev, WD_FEED_ACTIVE);
#endif

	printk_vcmd_register_debug(hwregs, "vcmd start exits");
}

/**
 * @brief start a vcmd hw device with spinlock.
 * @param u32 reset: whether to reset asic before start
 */
static void vcmd_start2(struct hantrovcmd_dev *dev, u32 reset)
{
	unsigned long flags;

	spin_lock_irqsave(dev->spinlock, flags);

	if (dev->state == VCMD_STATE_POWER_OFF) {
		spin_unlock_irqrestore(dev->spinlock, flags);
		return;
	}

	if (dev->state == VCMD_STATE_WORKING)
		dev->state = VCMD_STATE_IDLE;

	if (reset)
		vcmd_reset_current_asic(dev);

	vcmd_start(dev);
	spin_unlock_irqrestore(dev->spinlock, flags);
}

/**
 * @brief get current executing cmdbuf by vcmd register
 * @param u32 irq_status irq status
 */
static bi_list_node *vcmd_get_exec_node(struct hantrovcmd_dev *dev,
 u32 irq_status)
{
	u32 curr_id;
	addr_t exe_cmdbuf_ba;
	bi_list_node *curr_node = NULL;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)dev->handler;

	if (dev->hw_version_id < HW_ID_1_0_C) {
		exe_cmdbuf_ba = VCMDGetAddrRegisterValue((const void *)dev->hwregs,
		dev->reg_mirror,
		HWIF_VCMD_CMDBUF_EXE_ADDR);
		//get the executing cmdbuf node.
		curr_node = get_cmdbuf_node_by_addr(vcmd_mgr, dev, exe_cmdbuf_ba);
		if (curr_node == NULL)
		vcmd_klog(LOGLVL_ERROR, "%s no node bind to executing cmd ba 0x%lx!!\n", __func__, exe_cmdbuf_ba);
	} else {
		if ((irq_status == 0) || (irq_status & VCMD_IRQ_ERR_MASK)) {
			//if error irq, read curr_id from register directly.
			curr_id = vcmd_get_register_value((const void *)dev->hwregs,
			dev->reg_mirror,
			HWIF_VCMD_CMDBUF_EXE_ID);
		} else {
			//otherwise, read curr_id from vcmd reg_mem
			curr_id = *(dev->reg_mem_va + REG_ID_CMDBUF_EXE_ID);
		}

		if (CMDBUF_ID_CHECK(curr_id) == 0)
			curr_node = &vcmd_mgr->nodes[curr_id];
	}

	return curr_node;
}

/**
 * @brief wait vcmd abort, if aborted or buffer empty, it stop waiting
 * @return 0: continue to wait; 1: aborted; 2: slice decoding; < 0: failed
 */
static int _wait_vcmd_abort(struct hantrovcmd_dev *dev, u32 *aborted_id)
{
	bi_list_node *curr_node;
	struct cmdbuf_obj *obj;

	if ((dev->state == VCMD_STATE_IDLE) ||
		(dev->state == VCMD_STATE_POWER_OFF)) {
		*aborted_id = dev->aborted_cmdbuf_id;
		return 1;
	}

	curr_node = vcmd_get_exec_node(dev, 0);
	if (!curr_node) {
		vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n", __func__);
		return -1;
	}

	obj = (struct cmdbuf_obj *)curr_node->data;
	if (obj->slice_run_done == 2) {
		*aborted_id = obj->cmdbuf_id;
		return 2;
	}

	return 0;
}

static void vcmd_do_runtime_suspend(vcmd_mgr_t *vcmd_mgr, bool status)
{
#ifdef CONFIG_DEC_PM
		struct hantrodec_dev *dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);

		WARN_ON(!dec_dev);

		vdec_pm_klog(LOGLVL_DEBUG, "runtime put DEV_STATE IS %d.\n", vcmd_mgr->dev_ctx->state);
		mutex_lock(&dec_dev->pm->pm_runtime_lock);
		if (!status && !dec_dev->pm->runtime_idled && _runtime_context_idled(vcmd_mgr)) {
			vdec_pm_klog(LOGLVL_DEBUG, "vdec enter idle state, ready to runtime put.\n");
			hantrodec_pm_runtime_put(&vcmd_mgr->platformdev->dev);
			dec_dev->pm->runtime_idled = true;
		}
		mutex_unlock(&dec_dev->pm->pm_runtime_lock);
#endif
}

/**
 * @brief abort a specified vcmd hw device.
 * @param u32 *aborted_id: the id of aborted cmdbuf.
 * @param u32 vcmd_isr_polling: the mode to wait for device being aborted.
 * @return int: 0: succeed; 1: slice decoding aborted; < 0: failed.
 */
static int vcmd_abort(struct hantrovcmd_dev *dev, u32 *aborted_id)
{
	unsigned long flags = 0;
	u32 cnt = 1000, irq;
	int ret = 0, rv = 0;

	*aborted_id = -1;
	spin_lock_irqsave(dev->spinlock, flags);
	_vcmd_del_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
#ifdef SUPPORT_WATCHDOG
	if (dev->watchdog_state != WD_STATE_PAUSE)
		_vcmd_watchdog_stop(dev);
#endif
	if (dev->state == VCMD_STATE_POWER_OFF) {
		spin_unlock_irqrestore(dev->spinlock, flags);
		vcmd_klog(LOGLVL_WARNING, "vcmd is already in idle state!\n");
		return 0;
	} else {
		vcmd_write_register_value((const void *)dev->hwregs,
								dev->reg_mirror,
								HWIF_VCMD_START_TRIGGER, 0);
	}
	spin_unlock_irqrestore(dev->spinlock, flags);
	if (vcmd_isr_polling == 0) {
		if (!wait_event_timeout(*dev->abort_waitq,
					(rv = _wait_vcmd_abort(dev, aborted_id)),
					msecs_to_jiffies(500))) {
			vcmd_klog(LOGLVL_ERROR, "[dec] abort_waitq is timeout!!!\n");

			if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
				dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
			else
				dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
			return -ERESTARTSYS;
		}
	}

	if (rv == 2) {
		// slice decoding aborted
		ret = 1;
	}

	vcmd_klog(LOGLVL_INFO, "%s: vcmd aborted cmdbuf[%u].\n", __func__, *aborted_id);

	return ret;

}

/**
 * @brief select a suitable device, and add/insert a job node to its work_list.
 */
static int select_vcmd(vcmd_mgr_t *vcmd_mgr, bi_list_node *new_node)
{
	struct hantrovcmd_dev *dev, *smallest_dev;
	struct vcmd_module_mgr *module;
	struct cmdbuf_obj *obj, *tmp_obj;
	bi_list_node *curr_node;
	bi_list *list;
	u64 least_workload;
	u32 reg_id_exe;

	u32 cmdbuf_id, i;
	addr_t curr_exe_addr;
	int ret, rv;

	obj = (struct cmdbuf_obj *)new_node->data;
	module = &vcmd_mgr->module_mgr[obj->module_type];

	/* To check if there is free dev, or dev which tail node is run-done. */
	for (i=0; i<module->num; i++) {
		dev = module->dev[i];
		ret = is_supported_core(obj->core_mask, dev->core_id);
		if (ret == 0) {
			continue;
		}

		list = &dev->work_list;
		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!");
			return -EINTR;
		}
		if (!list->tail ||
			((struct cmdbuf_obj *)list->tail->data)->cmdbuf_run_done) {
			dev_add_job(dev, new_node, vcmd_mgr);
			up(&dev->work_sem);
			return 0;
		}
		up(&dev->work_sem);
	}

	// There is no vcmd in free, calculate each workload, select the least one.
	// If low priority, insert to tail.
	// If high priority, abort the dev, and insert to "head".
	reg_id_exe = REG_ID_CMDBUF_EXE_ID;
	if (obj->priority == CMDBUF_PRIORITY_NORMAL) {
		reg_id_exe = REG_ID2_CMDBUF_EXE_ID;
	}

	least_workload = 0xffffffffffffffff;
	smallest_dev = NULL;
	//calculate remain workload of all dev, find the least one
	for (i=0; i<module->num; i++) {
		dev = module->dev[i];
		ret = is_supported_core(obj->core_mask, dev->core_id);
		if (ret == 0) {
			continue;
		}

		list = &dev->work_list;

		//get the executing cmdbuf node.
		if (dev->hw_version_id <= HW_ID_1_0_C) {
			curr_exe_addr = VCMDGetAddrRegisterValue((const void *)dev->hwregs,
												dev->reg_mirror,
												HWIF_VCMD_CMDBUF_EXE_ADDR);

			curr_node = get_cmdbuf_node_by_addr(vcmd_mgr, dev, curr_exe_addr);

		} else {
			//cmdbuf_id = vcmd_get_register_value((const void *)dev->hwregs,
			//dev->reg_mirror,HWIF_VCMD_CMDBUF_EXE_ID);
			cmdbuf_id = *(dev->reg_mem_va + reg_id_exe);
			if (CMDBUF_ID_CHECK(cmdbuf_id) < 0) {
				return -1;
			}

			curr_node = &vcmd_mgr->nodes[cmdbuf_id];
		}

		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!");
			return -EINTR;
		}
		if (!curr_node)
			curr_node = list->head;
		//calculate total workload of this device
		dev->total_workload = calc_workload_after_node(curr_node);
		up(&dev->work_sem);

		if (dev->total_workload <= least_workload) {
			least_workload = dev->total_workload;
			smallest_dev = dev;
		}
	}

	if (smallest_dev == NULL) {
		vcmd_klog(LOGLVL_ERROR, "no dev is available to cmdbuf [%d] with core_mask 0x%x\n",
					obj->cmdbuf_id, obj->core_mask);
		return -EINVAL;
	}

	list = &smallest_dev->work_list;
	if (obj->priority == CMDBUF_PRIORITY_NORMAL) {
		//insert to tail
		if (down_killable(&smallest_dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			return -EINTR;
		}
		dev_add_job(smallest_dev, new_node, vcmd_mgr);
		up(&smallest_dev->work_sem);
		return 0;
	}

	//CMDBUF_PRIORITY_HIGH
	//abort the vcmd and wait
	if ((rv = vcmd_abort(smallest_dev, &cmdbuf_id)) < 0) {
		//abort failed
		return -ERESTARTSYS;
	}
	if (CMDBUF_ID_CHECK(cmdbuf_id) < 0) {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "vcmd abort id fail\n");
		return -EINTR;
	}
	// need to select inserting position again
	// because hw maybe have run to the next node.
	// CMDBUF_PRIORITY_HIGH
	if (down_killable(&smallest_dev->work_sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		return -EINTR;
	}
	curr_node = &vcmd_mgr->nodes[cmdbuf_id];
	if (smallest_dev->abort_mode == 0 || rv == 1)
		curr_node = curr_node->next;
	while (curr_node) {
		tmp_obj = (struct cmdbuf_obj *)curr_node->data;
		//find the 1st node with normal priority, and insert node prior to it
		if (tmp_obj->priority == CMDBUF_PRIORITY_NORMAL)
			break;
		curr_node = curr_node->next;
	}

	//insert to "head" of normal priority nodes
	dev_insert_job(smallest_dev, curr_node, new_node, vcmd_mgr);
	up(&smallest_dev->work_sem);
	return 0;
}

/**
 * @brief acquire workload from process object.
 * @return int: 0: succeed; Others: failed.
 */
static int wait_process_resource_rdy(struct proc_obj *po)
{
	return po->total_workload <= PROCESS_MAX_WORKLOAD;
}

static int acquire_process_resource(struct proc_obj *po, u64 workload) {
	spin_lock(&po->spinlock);
	po->total_workload += workload;
	spin_unlock(&po->spinlock);
	if (wait_event_interruptible(po->resource_waitq,
									wait_process_resource_rdy(po))) {
		vcmd_klog(LOGLVL_ERROR, "wait event is interrupted!");
		return -1;
	}

	return 0;
}

/**
 * @brief return cmdbuf obj's workload to process object.
 */
static void return_process_resource(struct proc_obj *po,
									struct cmdbuf_obj *obj) {
	if (po && obj->workload) {
		spin_lock(&po->spinlock);
		po->total_workload -= obj->workload;
		spin_unlock(&po->spinlock);
		obj->workload = 0;
		wake_up_interruptible_all(&po->resource_waitq);
	}
}

/**
 * @brief reserve a cmdbuf for specified process object.
 * @param struct exchange_parameter *param: the param of cmdbuf to reserve.
 * @return long: 0: succeed; oters: failed.
 */
static long reserve_cmdbuf(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
			   struct exchange_parameter *param)
{
	struct cmdbuf_obj *obj;
	u32 cmdbuf_id = 0;
	u32 workload;

	if (param->cmdbuf_size > SLOT_SIZE_CMDBUF) {
		vcmd_klog(LOGLVL_ERROR, "size is larger than slot size !!\n");
		return -1;
	}

	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "not find process obj!\n");
		return -1;
	}
	vcmd_klog(LOGLVL_DEBUG, "reserve cmdbuf by filp %p\n", (void *)po->filp);

	workload = (param->interrupt_ctrl) & 0x7fffffff; //bit31 for interrupt
	if (acquire_process_resource(po, workload))
		return -1;

	if (acquire_cmdbuf(vcmd_mgr, &cmdbuf_id))
		return -ERESTARTSYS;

	reset_cmdbuf_obj(vcmd_mgr, cmdbuf_id);
	reset_cmdbuf_node(vcmd_mgr, cmdbuf_id);

	obj = &vcmd_mgr->objs[cmdbuf_id];
	obj->module_type = param->module_type;
	obj->priority = param->priority;
	obj->workload = workload;
	obj->filp = po->filp;
	obj->po = po;
	obj->owner = param->owner;
	obj->core_mask = param->core_mask;
	obj->clk_rate = param->frequency;

	param->cmdbuf_size = SLOT_SIZE_CMDBUF;
	param->cmdbuf_id = cmdbuf_id;
	vcmd_klog(LOGLVL_DEBUG, "filp[%p] reserved cmdbuf[%d]: obj %p, node %p of owner %p\n",
			(void *)obj->filp, cmdbuf_id, (void *)obj,
			(void *)&vcmd_mgr->nodes[cmdbuf_id], obj->owner);
#ifdef SUPPORT_DBGFS
	_dbgfs_record_reserved_time(vcmd_mgr->dev_ctx[0].dbgfs_info,
								param->cmdbuf_id);
#endif

	return 0;
}

/**
 * @brief release a specified cmdbuf.
 * @return long: 0: succeed; oters: failed.
 */
static long release_cmdbuf(vcmd_mgr_t *vcmd_mgr,
							struct proc_obj *po, u16 cmdbuf_id)
{
	struct cmdbuf_obj *obj = NULL;
	bi_list_node *curr_node = NULL;
	struct hantrovcmd_dev *dev = NULL;

	if (CMDBUF_ID_CHECK(cmdbuf_id) < 0) {
		//should not happen
		return -1;
	}

	curr_node = &vcmd_mgr->nodes[cmdbuf_id];
	obj = (struct cmdbuf_obj *)curr_node->data;
	if (obj->po != po) {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "cmdbuf[%d] po not match: owned by %p, released by %p!!\n",
						cmdbuf_id, obj->po, po);
		return -1;
	}
	if (obj->core_id > 0) {
		/* should not happen */
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) > 0!\n",
			__func__, obj->core_id);
		return -EINVAL;
	}

	obj->owner = NULL;
	if (obj->cmdbuf_linked == 0) {
		//not link_and_run yet
		obj = (struct cmdbuf_obj *)curr_node->data;
		return_process_resource(obj->po, obj);
		return_cmdbuf(vcmd_mgr, cmdbuf_id);
	} else {
		obj->cmdbuf_need_remove = 1;
		dev = &vcmd_mgr->dev_ctx[obj->core_id];

		return_process_resource(obj->po, obj);
		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			return -EINTR;
		}
		dev_remove_job(vcmd_mgr, dev, curr_node);
		up(&dev->work_sem);
		obj->po = NULL;
	}

	return 0;
}

static void vcmd_set_slice_info(struct cmdbuf_obj *obj) {
	u32 i = 0;
	u32 j = 0;
	u64 pp0_c_base_msb = *(obj->cmd_va + VCMD_PP0_OUTPUT_BUFFER_PLAN2_MSB);
	u64 pp0_c_base_lsb = *(obj->cmd_va + VCMD_PP0_OUTPUT_BUFFER_PLAN2_LSB);
	u64 pp0_c_base = (pp0_c_base_msb << 32) + pp0_c_base_lsb;
	struct xplayer_cmdlist_config cmdlist_config;
	struct xplayer_iommu_format_info iommu_info;

	memset(&iommu_info, 0, sizeof(struct xplayer_iommu_format_info));

	/* only pp0 is supported, so rgb is not supported for wfd now */
	if (*(obj->cmd_va + VCMD_PP0_OUTPUT_AFBC_ENABLE) & (1 << 26)) {
		iommu_info.is_afbc = 1;
	} else {
		iommu_info.is_afbc = 0;
		iommu_info.offsets[0] = (u32)(obj->slice_outbuf.offset);
		iommu_info.offsets[1] = pp0_c_base > obj->slice_outbuf.iova ? (u32)(pp0_c_base - obj->slice_outbuf.iova) : 0;
		iommu_info.offsets[1] += iommu_info.offsets[0];
		/* only two planers for pp0 */
	}
	/* pp1 (rgb) todo ... */
	xplayer_set_display_dma_buf(obj->slice_outbuf.dma_buf, &cmdlist_config, &iommu_info);
	/* slice info circular assignment, each slice interval is 7 */
	for (i = 0; i < SLICE_MAX_LINE_CNT; i++) {
		/* only bit 31 - bit 16 is slice addr msb, bit 15 - bit 0 is slice line count */
		if ((*(obj->cmd_va + VCMD_SLICE0_ADDR_MSB + (SLICE_INTERVAL * i)) & VCMD_REG_LSB_MASK)) {
			*(obj->cmd_va + VCMD_SLICE0_ADDR_MSB + (SLICE_INTERVAL * i)) +=
				(u32)((obj->po->xplayer_iova[i] >> 16) & (VCMD_REG_MSB_MASK));
			*(obj->cmd_va + VCMD_SLICE0_ADDR_LSB + (SLICE_INTERVAL * i)) =
				(u32)(obj->po->xplayer_iova[i]);
			for (j = 0; j < SLICE_TXTBUF_CNT; j++) {
				*(obj->cmd_va + VCMD_SLICE0_TEXT0 + (SLICE_INTERVAL * i) + j) =
					(cmdlist_config.write_value[j]);
			}
		}
	}
}
// map xplayer iova
static int map_slice_buf(struct device *dev, struct proc_obj *po) {
	int i;
	int ret;
	for (i = 0; i < SLICE_MAX_LINE_CNT; i++) {
		po->xplayer_iova[i] = dma_map_single_attrs(dev,
			phys_to_virt(po->xplayer_info.phy_addr[i]),
			SLICE_TXTBUF_SIZE,
			DMA_BIDIRECTIONAL,
			DMA_ATTR_SKIP_CPU_SYNC);
		if (!(po->xplayer_iova[i])) {
			vcmd_klog(LOGLVL_ERROR, "slice mode buf map slice buffe fail, size= %d",
				  SLICE_TXTBUF_SIZE);
			return -1;
		}
	}

	return 0;
}
// unmap xplayer iova
static void unmap_slice_buf(struct device *dev, struct proc_obj *po) {
	int i;

	for (i = 0; i < SLICE_MAX_LINE_CNT; i++) {
		vcmd_klog(LOGLVL_INFO, "slice mode unmap slice buffer[%d]: %llx\n",
			i, po->xplayer_iova[i]);
		dma_unmap_single_attrs(dev,
			(dma_addr_t)po->xplayer_iova[i],
			SLICE_TXTBUF_SIZE,
			DMA_BIDIRECTIONAL,
			DMA_ATTR_SKIP_CPU_SYNC);
		po->xplayer_iova[i] = 0;
	}
}

static int vcmd_set_slice_param(vcmd_mgr_t *vcmd_mgr,struct cmdbuf_obj *obj)
{
	int ret;
	unsigned long iova_lsb = 0;
	unsigned long iova_msb = 0;
	int i;

	if (*(obj->cmd_va + VCMD_SLICE0_ADDR_LSB) == XRING_SLICE_IDENTIFIER) {
		if (!(obj->po->vcmd_get_slice_info_flags)) {
			vcmd_klog(LOGLVL_INFO, "slice mode get xplayer slice buffer!!\n");
			xplayer_get_cmdlist(&obj->po->xplayer_info);
			if(map_slice_buf(&vcmd_mgr->platformdev->dev, obj->po) == -1) {
				vcmd_klog(LOGLVL_ERROR, "ERROR map xplayer phy addr");
				return -1;
			}
			obj->po->vcmd_get_slice_info_flags = 1;
		}
		if (obj->po->xplayer_iova[0] == 0) {
			/* should not happen */
			vcmd_klog(LOGLVL_ERROR, "ERROR slice buffer is NULL");
			return -1;
		}

		iova_lsb = *(obj->cmd_va + VCMD_PP0_OUTPUT_BUFFER_LSB);
		iova_msb = *(obj->cmd_va + VCMD_PP0_OUTPUT_BUFFER_MSB);
		obj->slice_outbuf.iova = (iova_msb << 32) + iova_lsb;
		ret = get_dma_buf_by_iova(obj->slice_outbuf.iova,
			&obj->slice_outbuf.offset, &obj->slice_outbuf.dma_buf);
		if (ret) {
			vcmd_klog(LOGLVL_ERROR, "get output dma buffer fail!");
			return ret;
		}
		vcmd_set_slice_info(obj);
		obj->xring_slice_mode = 1;
	}
	return 0;
}

static int do_normal_stage1(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	vdec_power_mgr *pm = NULL;
	vdec_clk_mgr *cm;
	struct hantrodec_dev *dec_dev = NULL;
	unsigned long flags;
	int ret = 0;

	if (!vcmd_mgr->platformdev) {
		vcmd_klog(LOGLVL_ERROR, "platformdev is NULL!\n");
		return -EINVAL;
	}
	dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
	if (!dec_dev) {
		vcmd_klog(LOGLVL_ERROR, "dec_dev is NULL!\n");
		return -EINVAL;
	}

	pm = dec_dev->pm;

#ifdef CONFIG_DEC_PM
	/* vdec is active, ready to get power up */
	WARN_ON(!pm);
	mutex_lock(&pm->pm_runtime_lock);
	if (pm->runtime_idled) {
		vdec_pm_klog(LOGLVL_DEBUG, "runtime get DEV_STATE IS %d.\n", vcmd_mgr->dev_ctx->state);
		hantrodec_pm_runtime_get(&vcmd_mgr->platformdev->dev, pm);
		pm->runtime_idled = false;
	}
	mutex_lock(&pm->pm_lock);
#endif

#ifdef CONFIG_DEC_CM_RUNTIME
	if (!runtime_cm_enable || !obj->clk_rate) {
		vdec_cm_klog(LOGLVL_INFO,
					"runtime_cm disabled or cur cmdbuf do not need change clk rate!\n");
		return 0;
	}
	//set frequency
	cm = dec_dev->cm;
	if (!cm) {
		vdec_cm_klog(LOGLVL_ERROR, "vdec_clk_mgr is NULL!\n");
		ret = -EINVAL;
		goto err;
	}
	spin_lock_irqsave(dev->spinlock, flags);
	u64 desire_rate = vdec_clk_mgr_get_rate(cm);
	spin_unlock_irqrestore(dev->spinlock, flags);
	if (desire_rate) {
#ifdef CONFIG_DEC_PM
		vdec_clk_rate_config(pm, desire_rate);
#else
		mutex_lock(&pm->pm_lock);
		vdec_clk_rate_config(pm, desire_rate);
		mutex_unlock(&pm->pm_lock);
#endif
	}
#endif
	return 0;
err:
	mutex_unlock(&pm->pm_lock);
	mutex_unlock(&pm->pm_runtime_lock);
	return ret;
}

static void do_normal_stage2(vcmd_mgr_t *vcmd_mgr)
{
	vdec_power_mgr *pm = NULL;
	struct hantrodec_dev *dec_dev = NULL;

	dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
	WARN_ON(!dec_dev);
	pm = dec_dev->pm;
	WARN_ON(!pm);

	mutex_unlock(&pm->pm_lock);
	mutex_unlock(&pm->pm_runtime_lock);
}

/**
 * @brief add/insert a cmdbuf (job) to suitable device,
 *  and start the device hw if it is not working.
 * @param struct exchange_parameter *param: the param of cmdbuf to link & run.
 * @param stage 0:probe stage, 1:normal stage
 * @return long: 0: succeed; oters: failed.
 */
static long link_and_run_cmdbuf(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
				struct exchange_parameter *param, enum link_and_run_stage stage)
{
	struct cmdbuf_obj *obj;
	bi_list_node *curr_node;
	u32 module_type;

	struct hantrovcmd_dev *dev = NULL;
	struct hantrodec_dev *dec_dev = NULL;
	vdec_power_mgr *pm = NULL;
	unsigned long flags;
	int ret;
	u16 cmdbuf_id = param->cmdbuf_id;
	u16 batchcount = ((param->interrupt_ctrl >> 32) & 0xff);

	struct cmd_jmp_t *cmd_jmp;

	if (CMDBUF_ID_CHECK(cmdbuf_id) < 0) {
		//should not happen
		return -EINVAL;
	}
	/* fuzz report */
	if (param->cmdbuf_size > SLOT_SIZE_CMDBUF ||
		    param->cmdbuf_size < SLOT_SIZE_CMDBUF_MIN) {
		vcmd_klog(LOGLVL_ERROR, " cmdbuf_size(%u) is not available!!\n", param->cmdbuf_size);
		return -EINVAL;
	}

	curr_node = &vcmd_mgr->nodes[cmdbuf_id];
	obj = (struct cmdbuf_obj *)curr_node->data;
	if (obj->po != po) {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "cmdbuf[%d] po not match: owned by %p, released by %p!!\n",
						cmdbuf_id, obj->po, po);
		return -EINVAL;
	}

	if (obj->module_type >= MAX_VCMD_TYPE) {
		vcmd_klog(LOGLVL_ERROR, "%s obj's module type is not supported!!\n", __func__);
		return -EINVAL;
	}

	obj->cmdbuf_size = param->cmdbuf_size;
	obj->interrupt_ctrl = param->interrupt_ctrl;
	/* judgment slice  enable bit */
	if ((*(obj->cmd_va + VCMD_SLICE_LINE_CNT_CTRL) & VCMD_SLICE_ENABLE_MASK) ==
		VCMD_SLICE_ENABLE_MASK) {
		vcmd_klog(LOGLVL_INFO, "slice mode cmdbuf[%d] is xring slice vcmd\n",
			cmdbuf_id);
		ret = vcmd_set_slice_param(vcmd_mgr,obj);
		if (ret) {
			vcmd_klog(LOGLVL_ERROR, "ERROR xring vcmd set fail!");
			return ret;
		}
	}
#ifdef VCMD_DEBUG_INTERNAL
	_dbg_log_cmdbuf(obj);
#endif
	//0: has jmp opcode,1 has end code
	obj->has_jmp_cmd = param->has_end_cmd ? 0 : 1;

	if (obj->has_jmp_cmd) {
		//last command is JMP, get its IE value.
		cmd_jmp = _get_jmp_cmd(obj);
		vcmd_klog(LOGLVL_DEBUG,"has jmp cmd and the last cmd is JMP!\n");

		if ((cmd_jmp->opcode & OPCODE_MASK) != OPCODE_JMP) {
			vcmd_klog(LOGLVL_ERROR, "cmdbuf[%d] is not terminated by JMP,"
					"not match with its flag!", obj->cmdbuf_id);
			return -1;
		}
		obj->jmp_ie = 0;
		if (obj->interrupt_ctrl >> 31) {
			obj->jmp_ie = obj->interrupt_ctrl & 1; //force jmp_ie to be 1 or 0
			cmd_jmp->opcode |= JMP_IE(obj->jmp_ie);
		} else if (JMP_G_IE(cmd_jmp->opcode)) {
			obj->jmp_ie = 1;
		}
	}

	module_type = obj->module_type;
	/* fuzz report */
	if (module_type >= MAX_VCMD_TYPE
		|| (!vcmd_mgr->module_mgr[module_type].sem.wait_list.prev
			&& !vcmd_mgr->module_mgr[module_type].sem.wait_list.next)) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}

	if (down_killable(&vcmd_mgr->module_mgr[module_type].sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		return -EINTR;
	}

	ret = select_vcmd(vcmd_mgr, curr_node);
	if (ret) {
		up(&vcmd_mgr->module_mgr[module_type].sem);
		return ret;
	}
	if (obj->core_id > 0) {
		/* should not happen */
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) > 0!\n",
		    __func__, obj->core_id);
		up(&vcmd_mgr->module_mgr[obj->module_type].sem);
		return -EINVAL;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];
	param->core_id = obj->core_id;
	vcmd_klog(LOGLVL_INFO, "Assign cmdbuf[%d] to core[%d]\n",
			  cmdbuf_id, param->core_id);

	if (stage == NORMAL_STAGE) {
		if (do_normal_stage1(vcmd_mgr, dev, obj) < 0) {
			vcmd_klog(LOGLVL_ERROR, "cmdbuf[%d]-core[%d] normal_stage1 fail.\n",
				cmdbuf_id, param->core_id);
			up(&vcmd_mgr->module_mgr[module_type].sem);
			return -EFAULT;
		}
	}

	//start to run
	trace_link_and_run_cmdbuf("[dec] start frame: cmdbuf ", cmdbuf_id);
	if (obj->xring_slice_mode)
		vcmd_wfd_trace("start frame: cmdbuf[%d]\n", cmdbuf_id);
	/* fuzz report */
	if (!dev->work_sem.wait_list.prev && !dev->work_sem.wait_list.next) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}
	if (down_killable(&dev->work_sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
#ifdef CONFIG_DEC_PM
		if (stage == NORMAL_STAGE)
			do_normal_stage2(vcmd_mgr);
#endif
		return -EINTR;
	}
	spin_lock_irqsave(dev->spinlock, flags);
	if (dev->state != VCMD_STATE_WORKING) {
		//start vcmd
		vcmd_start(dev);
	} else {
		dev->sw_cmdbuf_rdy_num++;
		if ((batchcount > 0 && obj->jmp_ie == 1 && vcmd_mgr->module_mgr[VCMD_TYPE_DECODER].num == 1) ||
			batchcount == 0 || vcmd_mgr->module_mgr[VCMD_TYPE_DECODER].num > 1) {
			//just update cmdbuf ready number
			vcmd_write_register_value((const void *)dev->hwregs,
										dev->reg_mirror,
										HWIF_VCMD_RDY_CMDBUF_COUNT,
										dev->sw_cmdbuf_rdy_num);
#ifdef SUPPORT_DBGFS
			_dbgfs_record_link_time(dev->dbgfs_info, cmdbuf_id,
									dev->sw_cmdbuf_rdy_num,
									obj->workload);
#endif
#ifdef SUPPORT_WATCHDOG
			_vcmd_watchdog_feed(dev, WD_FEED_APPEND);
#endif
		}
	}
	spin_unlock_irqrestore(dev->spinlock, flags);

	//new cmdbuf linked, and free the removeable cmdbuf.
	if (curr_node->prev) {
		obj = (struct cmdbuf_obj *)curr_node->prev->data;
		if (obj->cmdbuf_need_remove) {
			//free the job
			dev_remove_job(vcmd_mgr, dev, curr_node->prev);
		}
	}
	up(&dev->work_sem);
#ifdef CONFIG_DEC_PM
	if (stage == NORMAL_STAGE)
		do_normal_stage2(vcmd_mgr);
#endif
	up(&vcmd_mgr->module_mgr[module_type].sem);

	return 0;
}

/**
 * @brief wait a cmdbuf runs done.
 * @param u16 cmdbuf_id: the id of cmdbuf to wait.
 * @param struct wait_cmdbuf *cmdbuf: point to the done cmdbuf.
 * @return long: 0: succeed; oters: failed.
 */
static long wait_cmdbuf_ready(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
								u16 cmdbuf_id, struct wait_cmdbuf *cmdbuf)
{
	struct cmdbuf_obj *obj = NULL;
	unsigned long timeout_jiff;
	int ret;

	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "not find process obj!\n");
		return -ERESTARTSYS;
	}
	/* fuzz report */
	if (cmdbuf_id >= SLOT_NUM_CMDBUF
		&& cmdbuf_id != ANY_CMDBUF_ID)
		    return -EINVAL;

	if (cmdbuf_id != ANY_CMDBUF_ID) {
		vcmd_klog(LOGLVL_DEBUG, "\n");
		obj = &vcmd_mgr->objs[cmdbuf_id];
		if (obj->po != po) {
			//should not happen
			vcmd_klog(LOGLVL_ERROR, "ERROR cmdbuf filp not match!\n");
			return -1;
		}
#ifdef IRQ_SIMULATION
		_irq_simul_add_timer(obj);
#endif
	}

	trace_wait_cmdbuf_ready("[dec] wait cmdbuf ready: cmdbuf ", cmdbuf_id);
	if (obj != NULL && obj->xring_slice_mode)
		vcmd_wfd_trace("wait cmdbuf ready: cmdbuf[%d]\n", cmdbuf_id);
	timeout_jiff = msecs_to_jiffies(1000);
	ret = wait_event_interruptible_timeout(po->job_waitq,
						proc_get_done_job(vcmd_mgr, po, &obj), timeout_jiff);
	if (!ret) {
		vcmd_klog(LOGLVL_WARNING, "vcmd_wait_queue_0 timeout\n");
		return -EFAULT;
	} else if (-ERESTARTSYS == ret) {
		vcmd_klog(LOGLVL_ERROR, "vcmd_wait_queue_0 interrupted\n");
		return -ERESTARTSYS;
	}

	if (obj == NULL) {
		cmdbuf->id = ANY_CMDBUF_ID;
	} else {
		cmdbuf->id = obj->cmdbuf_id;
		cmdbuf->exe_status = obj->executing_status;
	}

	return 0;
}

static int _vcmd_alloc_dma_mem(vcmd_mgr_t *vcmd_mgr)
{
	phys_addr_t phys_addr = 0;
	dma_addr_t dma_addr = 0;
	u32 *cpu_addr = NULL;

	cpu_addr = dma_alloc_attrs(
			&vcmd_mgr->platformdev->dev, VCMDBUF_TOTAL_SIZE,
			&phys_addr, GFP_KERNEL, DMA_ATTR_FORCE_CONTIGUOUS);
	if (!cpu_addr || !phys_addr) {
		 vcmd_klog(LOGLVL_ERROR, "dma_alloc_coherent memory failed\n");
		return -ENOMEM;
	}

	 vcmd_klog(LOGLVL_INFO, "vdec alloc success, cpu_addr = 0x%llx, phys_addr = 0x%llx, size 0x%x\n",
		(unsigned long long)cpu_addr, (unsigned long long)phys_addr, VCMDBUF_TOTAL_SIZE);

#ifdef BYPASS_SMMU_TEST
	dma_addr = phys_addr;
#else
	dma_addr = dma_map_single_attrs(&vcmd_mgr->platformdev->dev,
				phys_to_virt(phys_addr), VCMDBUF_TOTAL_SIZE,
				DMA_BIDIRECTIONAL, DMA_ATTR_SKIP_CPU_SYNC);
#endif
	if (!dma_addr) {
		 vcmd_klog(LOGLVL_ERROR, "no dma_addr, map failed, size= %d", VCMDBUF_TOTAL_SIZE);
		goto fail_map_pg;
	}

	 vcmd_klog(LOGLVL_INFO, "vdec map success dma_addr = 0x%llx, size 0x%x\n",
			(unsigned long long)dma_addr, VCMDBUF_TOTAL_SIZE);

	/* command buffer */
	vcmd_mgr->mem_vcmd.size = CMDBUF_POOL_TOTAL_SIZE;
	vcmd_mgr->mem_vcmd.va = cpu_addr;
	vcmd_mgr->mem_vcmd.pa = dma_addr;
	vcmd_mgr->mem_vcmd.mmap_pa = phys_addr;

	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_vcmd.iova=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_vcmd.pa);
	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_vcmd.va=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_vcmd.va);

	/* status buffer */
	vcmd_mgr->mem_status.size = CMDBUF_POOL_TOTAL_SIZE;
	vcmd_mgr->mem_status.va = cpu_addr +
			CMDBUF_POOL_TOTAL_SIZE / 4;
	vcmd_mgr->mem_status.pa = dma_addr +
			CMDBUF_POOL_TOTAL_SIZE;
	vcmd_mgr->mem_status.mmap_pa = phys_addr +
			CMDBUF_POOL_TOTAL_SIZE;

	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_status.iova=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_status.pa);
	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_status.va=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_status.va);

	/* register buffer */
	vcmd_mgr->mem_regs.size = CMDBUF_VCMD_REGISTER_TOTAL_SIZE;
	vcmd_mgr->mem_regs.va = cpu_addr +
			(2 * CMDBUF_POOL_TOTAL_SIZE) / 4;
	vcmd_mgr->mem_regs.pa = dma_addr +
			2 * CMDBUF_POOL_TOTAL_SIZE;
	vcmd_mgr->mem_regs.mmap_pa = phys_addr +
			2 * CMDBUF_POOL_TOTAL_SIZE;

	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_regs.iova=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_regs.pa);
	 vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_regs.va=0x%llx.\n",
		(unsigned long long)vcmd_mgr->mem_regs.va);

	return 0;
fail_map_pg:
	dma_free_attrs(&vcmd_mgr->platformdev->dev, VCMDBUF_TOTAL_SIZE,
			cpu_addr, phys_addr, DMA_ATTR_FORCE_CONTIGUOUS);
	return -ENOMEM;
}

static void _vcmd_free_dma_mem(vcmd_mgr_t *vcmd_mgr)
{
	dma_unmap_single_attrs(&vcmd_mgr->platformdev->dev,
			(dma_addr_t)vcmd_mgr->mem_vcmd.pa, VCMDBUF_TOTAL_SIZE,
			DMA_BIDIRECTIONAL, DMA_ATTR_SKIP_CPU_SYNC);

	if (vcmd_mgr->mem_vcmd.va) {
		dma_free_attrs(
			&vcmd_mgr->platformdev->dev, VCMDBUF_TOTAL_SIZE,
			vcmd_mgr->mem_vcmd.va,
			(dma_addr_t)vcmd_mgr->mem_vcmd.mmap_pa,
			DMA_ATTR_FORCE_CONTIGUOUS);
	}
}

/**
 * @brief allocate a memory pool, for non-PCIe platform.
 */
static int _vcmd_alloc_mem(vcmd_mgr_t *vcmd_mgr, struct noncache_mem *mem)
{
	struct kernel_addr_desc mmu_addr;
	dma_addr_t dma_handle = 0;

	/* command buffer */
	mem->va = (u32 *)dma_alloc_coherent(&vcmd_mgr->platformdev->dev,
							mem->size, &dma_handle,
							GFP_KERNEL | __GFP_DMA32);
	mem->pa = (unsigned long long)dma_handle;

	vcmd_klog(LOGLVL_DEBUG, "Init: mem pa=0x%llx, va=0x%llx.\n",
			(unsigned long long)mem->pa, (unsigned long long)mem->va);
	if (vcmd_mgr->mmu_enable) {
		mmu_addr.bus_address = mem->pa;
		mmu_addr.size = mem->size;
		if (MMUKernelMemNodeMap(&mmu_addr) != MMU_STATUS_OK) {
			vcmd_klog(LOGLVL_ERROR, "Init mmu map mem failed\n");
			return -1;
		}
		mem->mmu_ba = mmu_addr.mmu_bus_address;
		vcmd_klog(LOGLVL_DEBUG, "Init mem->mmu_ba=0x%llx.\n",
				(unsigned long long)mem->mmu_ba);
	}
	return 0;
}

/**
 * @brief free the specified memory pool, for non-PCIe platform.
 */
static void _vcmd_free_mem(vcmd_mgr_t *vcmd_mgr, struct noncache_mem *mem)
{
	struct kernel_addr_desc mmu_addr;

	if (mem->va) {
		if (vcmd_mgr->mmu_enable && mem->mmu_ba) {
			mmu_addr.bus_address = mem->pa;
			mmu_addr.size = mem->size;
			MMUKernelMemNodeUnmap(&mmu_addr);
			mem->mmu_ba = 0;
		}
		dma_free_coherent(&vcmd_mgr->platformdev->dev,
							mem->size, mem->va, (dma_addr_t)mem->pa);
		mem->va = NULL;
	}
}

/**
 * @brief allocate memory pools, and init PCIe access if it is PCIe device.
 * @return int: 0: succeed; other: failed.
 */
static int vcmd_init(vcmd_mgr_t *vcmd_mgr)
{
	if (_vcmd_alloc_dma_mem(vcmd_mgr))
		return -1;

	return 0;
}

/**
 * @brief initialize context of all vcmd dev
 */
static void dev_ctx_init(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;
	struct hantrovcmd_dev *dev;
	u32 m_type;
	struct vcmd_module_mgr *module;

	memset(vcmd_mgr->dev_ctx,
			0, sizeof(struct hantrovcmd_dev) * vcmd_mgr->subsys_num);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];

		dev->handler = (void *)vcmd_mgr;
		dev->subsys_info = &vcmd_mgr->core_array[i];
		m_type = dev->subsys_info->sub_module_type;

		dev->core_id = i;
		dev->state = VCMD_STATE_POWER_ON;
		dev->sw_cmdbuf_rdy_num = 0;
		/* the abnormal interrupts source from VCD */
		dev->abn_irq_mask = DEC_ABNORMAL_IRQ_MASK;
		dev->intr_gate_mask = 0xFFFFFFFF & (~dev->abn_irq_mask);
		dev->abn_irq_schedule = 0;
		_vcmd_init_timer(dev);

#ifdef SUPPORT_WATCHDOG
		dev->watchdog_state = WD_STATE_NONE;
#endif
		dev->arb_reset_irq = 0;
		dev->arb_err_irq = 0;
		dev->mmu_enable = vcmd_mgr->mmu_enable;
		dev->abort_mode = 0;
		dev->init_mode = VCMD_INIT_NORMAL;

		dev->spinlock = &dev->owner_lock_vcmd;
		dev->po_spinlock = &dev->owner_po_lock;
		spin_lock_init(dev->spinlock);
		spin_lock_init(dev->po_spinlock);
		spin_lock_init(&dev->abn_irq_lock);
		dev->abort_waitq = &dev->abort_queue_vcmd;
		init_waitqueue_head(dev->abort_waitq);

		sema_init(&dev->work_sem, 1);
		init_bi_list(&dev->work_list);

		/* init work: work & work_func */
		dev->wq = alloc_workqueue("vdec_worker", WQ_HIGHPRI | WQ_UNBOUND, 1);
		if (!dev->wq) {
			vcmd_klog(LOGLVL_ERROR, "alloc_workqueue fail.\n");
			BUG_ON(1);
		}
		INIT_WORK(&dev->_irq_work, _vcmd_irq_bottom_half);
		dev->reg_mem_ba = vcmd_mgr->mem_regs.pa +
							i * SLOT_SIZE_REGBUF - vcmd_mgr->pa_trans_offset;
		dev->mmu_reg_mem_ba = vcmd_mgr->mem_regs.mmu_ba +
								i * SLOT_SIZE_REGBUF;
		dev->reg_mem_va = vcmd_mgr->mem_regs.va + i * SLOT_SIZE_REGBUF / 4;
		dev->reg_mem_sz = SLOT_SIZE_REGBUF;
		memset(dev->reg_mem_va, 0, dev->reg_mem_sz);
		dev->pa_trans_offset = vcmd_mgr->pa_trans_offset;
		dev->reset_count = 0;
		dev->wdt_trigger_count = 0;
		dev->error_mask = 0;

		module = &vcmd_mgr->module_mgr[m_type];
		if (module->num == 0)
			sema_init(&module->sem, 1);
		module->dev[module->num++] = dev;

		vcmd_klog(LOGLVL_INFO, "module init - vcmdcore[%d] addr =0x%llx\n",
			i, (unsigned long long)dev->subsys_info->reg_base);
	}
}

/**
 * @brief fill cmdbuf to read main module's all regs.
 */
#define VCMD_READ_CMD(cmd, n, off, addr) \
	do { \
		(cmd)->opcode = OPCODE_RREG | RREG_MODE(RREG_ADDR_INC) | \
						RREG_LEN(n) | \
						RREG_START_ADDR(off); \
		CMD_SET_ADDR(cmd, (addr)+(off)); \
		(cmd)->padding = 0; \
		cmd++; \
	} while (0)

static void create_read_all_registers_cmdbuf(vcmd_mgr_t *vcmd_mgr,
											struct exchange_parameter *param)
{
	struct hantrovcmd_dev *dev;
	struct vcmd_subsys_info *subsys;
	struct cmd_rreg_t *cmd_rreg;
	struct cmd_end_t *cmd_end;
	u8 *p_cmdbuf;
	addr_t reg_ba;

	dev = &vcmd_mgr->dev_ctx[param->core_id];
	subsys = dev->subsys_info;

	reg_ba = dev->reg_mem_ba;
	if (dev->mmu_enable)
		reg_ba = (addr_t)dev->mmu_reg_mem_ba;

	reg_ba += param->core_id * SLOT_SIZE_REGBUF;

	p_cmdbuf = (u8 *)vcmd_mgr->mem_vcmd.va + CMDBUF_OFF(param->cmdbuf_id);

	cmd_rreg = (struct cmd_rreg_t *)(p_cmdbuf + 0);
	if (dev->hw_version_id > HW_ID_1_0_C) {
		//read vcmd executing cmdbuf id registers to ddr for balancing core load.
		VCMD_READ_CMD(cmd_rreg, 1, REG_ID_CMDBUF_EXE_ID * 4, 0);
	}
	//read main IP all registers
	VCMD_READ_CMD(cmd_rreg,
				subsys->io_size[SUB_MOD_MAIN] / 4,
				subsys->reg_off[SUB_MOD_MAIN] + 0,
				reg_ba);
	if (dev->hw_version_id > HW_ID_1_0_C) {
		//read vcmd registers to ddr, to compliant with user cmdbuf
		VCMD_READ_CMD(cmd_rreg, 27, 0, 0);
	}
	//end cmd
	cmd_end = (struct cmd_end_t *)cmd_rreg;
	cmd_end->opcode = OPCODE_END;
	cmd_end->padding = 0;
	cmd_end++;

	param->cmdbuf_size = (u8 *)cmd_end - p_cmdbuf;
}

/**
 * @brief release submodule IO resource for vcmd driver
 */
static void vcmd_release_submodule_IO(struct vcmd_subsys_info *subsys, u32 sub_mod_id)
{
	if (subsys->hwregs[sub_mod_id]) {
		iounmap((volatile u8 __iomem *)subsys->hwregs[sub_mod_id]);
		release_mem_region(subsys->reg_base + subsys->reg_off[sub_mod_id],
			subsys->io_size[sub_mod_id]);
		subsys->hwregs[sub_mod_id] = NULL;
	}
}

/**
 * @brief reserve submodule IO resource for vcmd driver
 */
static u32 vcmd_reserve_submodule_IO(struct vcmd_subsys_info *subsys, u32 sub_mod_id)
{
	addr_t pa = subsys->reg_base + subsys->reg_off[sub_mod_id];
	size_t sz = subsys->io_size[sub_mod_id];

	if (!request_mem_region(pa, sz, "vcx_vcmd_driver")) {
		return -EBUSY;
	}

	subsys->hwregs[sub_mod_id] = (volatile u8 __force *)ioremap(pa, sz);

	return 0;
}

/**
 * @brief reserve IO resources for vcmd driver.
 */
static int vcmd_reserve_IO(vcmd_mgr_t *vcmd_mgr)
{
	u32 hwid;
	int i;
	u32 found_hw = 0;
	struct hantrovcmd_dev *dev;
	struct vcmd_subsys_info *subsys;
	u32 reg_val;
	int ret;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		subsys = dev->subsys_info;
		dev->hwregs = NULL;

		ret = vcmd_reserve_submodule_IO(subsys, SUB_MOD_VCMD);
		if (ret < 0) {
			vcmd_klog(LOGLVL_ERROR, "failed to reserve HW regs for "
					"vcmd[%d]\n", i);
			vcmd_klog(LOGLVL_ERROR,
						"vcmd_base_addr = 0x%08lx, iosize = %d\n",
						subsys->reg_base, subsys->io_size[SUB_MOD_VCMD]);
			continue;
		}
		if (!subsys->hwregs[SUB_MOD_VCMD]) {
			vcmd_klog(LOGLVL_ERROR, "failed to ioremap HW regs\n");
			release_mem_region(
				subsys->reg_base, subsys->io_size[SUB_MOD_VCMD]);
			continue;
		}
		dev->hwregs = subsys->hwregs[SUB_MOD_VCMD];
		vcmd_core_array[i].submodule_vcmd_virtual_address = dev->hwregs;

		/*read hwid and check validness and store it*/
		hwid = (u32)ioread32((void __iomem *)dev->hwregs);
		vcmd_klog(LOGLVL_INFO, "hantrovcmd: vcmd[%d] hwid=0x%08x\n", i, hwid);
		dev->hw_version_id = hwid;

		/* check for vcmd HW ID */
		if (((hwid >> 16) & 0xFFFF) != VCMD_HW_ID) {
			vcmd_klog(LOGLVL_WARNING, "HW not found at 0x%llx\n",
				(unsigned long long)subsys->reg_base);
			iounmap((void __iomem *)dev->hwregs);
			release_mem_region(subsys->reg_base,
								subsys->io_size[SUB_MOD_VCMD]);
			dev->hwregs = NULL;
			continue;
		}

		reg_val = (u32)ioread32((void __iomem *)dev->hwregs +
				   VCMD_REGISTER_HW_APB_ARBITER_MODE_OFFSET);
		dev->hw_feature.has_arbiter = 0;
		reg_val = (u32)ioread32((void __iomem *)dev->hwregs +
				   VCMD_REGISTER_HW_INIT_MODE_OFFSET);
		dev->hw_feature.has_init_mode = (u8)((reg_val >> HW_INIT_MODE_BIT) &
										0x01);
#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
		if (dev->hw_feature.has_init_mode) {
			dev->init_mode = DEFAULT_VCMD_INIT_MODE;
		}
#endif

		found_hw = 1;
		vcmd_klog(LOGLVL_INFO, "HW at base <0x%llx> with ID <0x%08x>\n",
					(unsigned long long)subsys->reg_base, hwid);
	}

	if (found_hw == 0) {
		vcmd_klog(LOGLVL_ERROR, "NO ANY HW found!!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief release IO resources of vcmd driver.
 */
static void vcmd_release_IO(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;
	struct vcmd_subsys_info *subsys;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		subsys = &vcmd_mgr->core_array[i];
		vcmd_release_submodule_IO(subsys, SUB_MOD_VCMD);
		vcmd_mgr->dev_ctx[i].hwregs = NULL;
	}
}

/**
 * @brief reserve irq for vcmd driver.
 */
static int vcmd_reserve_irq(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;
	int result;
	struct hantrovcmd_dev *dev;
	struct vcmd_subsys_info *subsys;


	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		subsys = dev->subsys_info;

		if (!dev->hwregs)
			continue;

		if (subsys->irq == -1) {
			vcmd_klog(LOGLVL_INFO, "vcmd[%d]: IRQ not in use!\n", i);
			continue;
		}

		result = request_irq(subsys->irq,
							hantrovcmd_isr,
							IRQF_SHARED | vcmd_irq_flags,
							VCMD_IRQ_NAME,
							(void *)vcmd_mgr);

		if (result == -EINVAL) {
			vcmd_klog(LOGLVL_ERROR, "vcmd[%d]: Bad vcmd_irq number or handler.\n", i);
			return -1;
		} else if (result == -EBUSY) {
			vcmd_klog(LOGLVL_ERROR, "vcmd[%d]: IRQ <%d> is occupied!!!\n", i, subsys->irq);
			return -1;
		} else {
			vdec_init_klog(LOGLVL_INFO, "vcmd[%d]: request IRQ <%d> succeed\n", i, subsys->irq);
			vcmd_mgr->vcmd_irq_enabled = 1;
		}
	}
	return 0;
}

/**
 * @brief check the vcmd and vce hwid from the vcmd reg buffer
 * @return 0: check success, -1: check failed
 */
static int _check_hwid(vcmd_mgr_t *vcmd_mgr)
{
	int i;
	u32 *regs_va;
	u32 vcmd_hwid, vcd_hwid;
	struct hantrovcmd_dev *dev;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		regs_va = dev->reg_mem_va;
		vcmd_hwid = *(regs_va + dev->subsys_info->reg_off[SUB_MOD_VCMD] / 4 + 0);
		vcd_hwid = *(regs_va + dev->subsys_info->reg_off[SUB_MOD_MAIN] / 4 + 0);

		if ((((vcmd_hwid >> 16) & 0xFFFF) != VCMD_HW_ID) ||
			(((vcd_hwid >> 16) & 0xFFFF) != VCD_HW_ID))
			return -1;
	}
	return 0;
}

/**
 * @brief read main module's all regs for all vcmd hw devices.
 */
void read_main_module_all_registers(void *_vcmd_mgr)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	int ret;
	struct exchange_parameter param[MAX_SUBSYS_NUM];
	struct wait_cmdbuf cmdbuf;
	struct proc_obj *po;
	u32 i, irq;
	struct hantrovcmd_dev *dev;
	struct vcmd_module_mgr *module;
	u32 *main_regs_va;

	po = vcmd_mgr->init_po;

	for (i=0; i<vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		param[i].interrupt_ctrl = 0;
		param[i].priority = CMDBUF_PRIORITY_NORMAL;
		param[i].cmdbuf_size = 0;
		param[i].module_type = dev->subsys_info->sub_module_type;
		param[i].core_mask = 1 << dev->core_id;
		param[i].core_id = dev->core_id;
		param[i].has_end_cmd = 1;
		param[i].frequency = 0;
		param[i].owner = NULL;
		module = &vcmd_mgr->module_mgr[param[i].module_type];

		ret = reserve_cmdbuf(vcmd_mgr, po, &param[i]);
		create_read_all_registers_cmdbuf(vcmd_mgr, &param[i]);
		link_and_run_cmdbuf(vcmd_mgr, po, &param[i], PROBE_STAGE);
	}
	/* make sure vcmd can complete job, and clear irq
	 */
	if (vcmd_mgr->vcmd_irq_enabled == 0)
		msleep(100);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];

		if (vcmd_mgr->vcmd_irq_enabled == 0) {
			irq = dev->core_id;
			hantrovcmd_isr(irq, vcmd_mgr);
		}

		wait_cmdbuf_ready(vcmd_mgr, po, param[i].cmdbuf_id, &cmdbuf);
		main_regs_va = dev->reg_mem_va +
				dev->subsys_info->reg_off[SUB_MOD_MAIN] / 4 + 0;

		vcmd_klog(LOGLVL_INFO, "main module register 0:0x%x\n",
			*main_regs_va);
		vcmd_klog(LOGLVL_INFO, "main module register 50:0x%08x\n",
			*(main_regs_va + 50));
		vcmd_klog(LOGLVL_INFO, "main module register 54:0x%08x\n",
			*(main_regs_va + 54));
		vcmd_klog(LOGLVL_INFO, "main module register 56:0x%08x\n",
			*(main_regs_va + 56));
		vcmd_klog(LOGLVL_INFO, "main module register 309:0x%x\n",
			*(main_regs_va + 309));

		release_cmdbuf(vcmd_mgr, po, param[i].cmdbuf_id);

	}
}

/**
 * @brief convert subsys info to core array info of vcmd driver context.
 */
static void SubsysToVcmdCoreCfg(struct subsys_config *subsys,
								int subsys_num, vcmd_mgr_t *vcmd_mgr)
{
	int i, j = 0;
	struct vcmd_subsys_info *core_array;

	/* To plug into hantro_vcmd.c */
	for (i = 0; i < subsys_num; i++) {
		if (subsys[i].submodule_iosize[HW_VCMD]) {
			core_array = &vcmd_mgr->core_array[j];
			core_array->irq = subsys[i].irq;
			core_array->reg_base = subsys[i].base_addr;
			core_array->sub_module_type = subsys[i].subsys_type;

			core_array->reg_off[SUB_MOD_VCMD] = 0;
			core_array->reg_off[SUB_MOD_MAIN] = subsys[i].submodule_offset[HW_VCD];
			core_array->reg_off[SUB_MOD_DEC400] = subsys[i].submodule_offset[HW_DEC400];
			core_array->reg_off[SUB_MOD_MMU] = subsys[i].submodule_offset[HW_MMU];
			core_array->reg_off[SUB_MOD_MMU_WR] = subsys[i].submodule_offset[HW_MMU_WR];
			core_array->reg_off[SUB_MOD_AXIFE] = subsys[i].submodule_offset[HW_AXIFE];
			core_array->reg_off[SUB_MOD_UFBC] = subsys[i].submodule_offset[HW_AFBC];

			core_array->io_size[SUB_MOD_VCMD] = subsys[i].submodule_iosize[HW_VCMD];
			core_array->io_size[SUB_MOD_MAIN] = subsys[i].submodule_iosize[HW_VCD];
			core_array->io_size[SUB_MOD_DEC400] = subsys[i].submodule_iosize[HW_DEC400];
			core_array->io_size[SUB_MOD_MMU] = subsys[i].submodule_iosize[HW_MMU];
			core_array->io_size[SUB_MOD_MMU_WR] = subsys[i].submodule_iosize[HW_MMU_WR];
			core_array->io_size[SUB_MOD_AXIFE] = subsys[i].submodule_iosize[HW_AXIFE];
			core_array->io_size[SUB_MOD_UFBC] = subsys[i].submodule_iosize[HW_AFBC];

			core_array->hwregs[SUB_MOD_MAIN] = vcmd_core_array[i].submodule_vcd_virtual_address;
			core_array->hwregs[SUB_MOD_DEC400] = vcmd_core_array[i].submodule_dec400_virtual_address;
			core_array->hwregs[SUB_MOD_MMU] = vcmd_core_array[i].submodule_MMU_virtual_address;
			core_array->hwregs[SUB_MOD_MMU_WR] = vcmd_core_array[i].submodule_MMUWrite_virtual_address;
			core_array->hwregs[SUB_MOD_AXIFE] = vcmd_core_array[i].submodule_axife_virtual_address;

			j++;
		}
	}
	vcmd_mgr->subsys_num = j;
	vcmd_klog(LOGLVL_INFO, "%d VCMD cores found\n", j);
}

/**
 * @brief vcmd driver initialization
 * @param struct subsys_config *subsys: info of sub-systems.
 * @param int subsys_num: number of sub-systems.
 * @param void *platformdev: platform device handler
 * @return void *: NULL: failed; other: vcmd driver handler.
 */
void *hantrovcmd_init(struct subsys_config *subsys,
						int subsys_num, void *platformdev)
{
	int result;
	int i;
	int j;
	struct hantrovcmd_dev *dev_ctx;
	vcmd_mgr_t *vcmd_mgr;

	vcmd_mgr = vmalloc(sizeof(vcmd_mgr_t));
	if (!vcmd_mgr)
		return NULL;

	memset(vcmd_mgr, 0, sizeof(vcmd_mgr_t));
	SubsysToVcmdCoreCfg(subsys, subsys_num, vcmd_mgr);

	if (!vcmd_mgr->subsys_num)
		goto err;

	subsys_num = vcmd_mgr->subsys_num;
	vcmd_mgr->platformdev = (struct platform_device *)platformdev;

	vcmd_mgr->mem_vcmd.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_CMDBUF);
	vcmd_mgr->mem_status.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_STATUSBUF);
	vcmd_mgr->mem_regs.size = ALIGN_4K(subsys_num * SLOT_SIZE_REGBUF);
	result = vcmd_init(vcmd_mgr);
	if (result)
		goto err;

	dev_ctx = vmalloc(sizeof(struct hantrovcmd_dev) * subsys_num);
	if (!dev_ctx)
		goto err;

	vcmd_mgr->dev_ctx = dev_ctx;
	dev_ctx_init(vcmd_mgr);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if(kfifo_alloc(&dev_ctx[i].send_slice_buffer_queue, SLOT_NUM_CMDBUF * sizeof(u16), GFP_DMA | GFP_KERNEL)) {
			/* kfifo alloc error, need free release the kfifo that has been applied */
			for (j = 0; j < i; ++j) {
				kfifo_free(&vcmd_mgr->dev_ctx[j].send_slice_buffer_queue);
			}
			goto err;
		}
	}

	sema_init(&vcmd_mgr->isr_polling_sema, 1);

	vcmd_mgr->init_po = create_process_object();
	if (!vcmd_mgr->init_po)
		goto err;

	result = vcmd_reserve_IO(vcmd_mgr);
	if (result < 0)
		goto err0;
	vcmd_reset_asic(vcmd_mgr);

#ifdef SUPPORT_DBGFS
	/* for debugfs */
	if (_dbgfs_init((void *)vcmd_mgr))
		goto err0;
	_dbgfs_init_ctx((void *)vcmd_mgr, 0);
#endif
	/* get the IRQ line */
	result = vcmd_reserve_irq(vcmd_mgr);
	if (result < 0) {
		goto err0;
	}

#ifdef IRQ_SIMULATION
	_irq_simul_init((void *)vcmd_mgr);
#endif

	vcmd_init_objs(vcmd_mgr);
	vcmd_init_nodes(vcmd_mgr);
	vcmd_init_free_obj_list(vcmd_mgr);

	/* create vcmd kthread, which need to be woken up */
	_vcmd_kthread_create(vcmd_mgr);

	/* read all registers of main-module for each dev
	 * for analyzing configuration in cwl
	 */
	read_main_module_all_registers(vcmd_mgr);
	if (_check_hwid(vcmd_mgr)) {
		vcmd_klog(LOGLVL_ERROR, "check hwid from vcmd reg buffer failed!\n");
		usleep_range(1000, 1200);
		read_main_module_all_registers(vcmd_mgr);
	}

	return vcmd_mgr;

err0:
	vcmd_release_IO(vcmd_mgr);

err:
#ifndef SUPPORT_DMA_ALLOC
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_vcmd);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_status);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_regs);
#else
	_vcmd_free_dma_mem(vcmd_mgr);
#endif

	free_process_object(vcmd_mgr->init_po);

	if (vcmd_mgr->dev_ctx)
		vfree(vcmd_mgr->dev_ctx);
	if (vcmd_mgr)
		vfree(vcmd_mgr);
	vdec_init_klog(LOGLVL_ERROR, "module not inserted!\n");
	return NULL;
}

/**
 * @brief de-init vcmd driver.
 */
void hantrovcmd_cleanup(void *_vcmd_mgr)
{
	int i = 0;
	u32 result;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	struct hantrovcmd_dev *dev_ctx = vcmd_mgr->dev_ctx;

	_vcmd_kthread_stop(vcmd_mgr);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if (!dev_ctx[i].hwregs)
			continue;
		//disable interrupt at first
#ifndef CONFIG_DEC_PM
		vcmd_write_reg((const void *)dev_ctx[i].hwregs,
					VCMD_REGISTER_INT_CTL_OFFSET, 0x0000);
		//disable HW
		vcmd_write_reg((const void *)dev_ctx[i].hwregs,
					VCMD_REGISTER_CONTROL_OFFSET, 0x0000);
		//read status register
		result = vcmd_read_reg((const void *)dev_ctx[i].hwregs,
						VCMD_REGISTER_INT_STATUS_OFFSET);
		//clean status register
		vcmd_write_reg((const void *)dev_ctx[i].hwregs,
					VCMD_REGISTER_INT_STATUS_OFFSET, result);
		// reset interrupt gate
		vcmd_write_reg((const void *)dev_ctx[i].hwregs,
					   VCMD_REGISTER_EXT_INT_GATE_OFFSET, 0x0000);
#endif
		/* free the vcmd IRQ */
		if (dev_ctx[i].subsys_info->irq != -1)
			free_irq(dev_ctx[i].subsys_info->irq, (void *)vcmd_mgr);

		kfifo_free(&vcmd_mgr->dev_ctx[i].send_slice_buffer_queue);
		if (dev_ctx[i].wq)
			destroy_workqueue(dev_ctx[i].wq);
	}

#ifdef SUPPORT_DBGFS
	_dbgfs_cleanup((void *)vcmd_mgr);
#endif
	vcmd_release_IO(vcmd_mgr);
	vfree(dev_ctx);

	//release_vcmd_non_cachable_memory();
#ifndef SUPPORT_DMA_ALLOC
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_vcmd);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_status);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_regs);
#else
	_vcmd_free_dma_mem(vcmd_mgr);
#endif

	free_process_object(vcmd_mgr->init_po);
	vfree(vcmd_mgr);
	vcmd_klog(LOGLVL_DEBUG, "module removed\n");
}

/**
 * @brief flush slice regs in vcmd driver
 */
static long flush_slice_regs(vcmd_mgr_t *vcmd_mgr,
			struct subsys_regs_desc *slice_regs)
{
	u32 cmdbuf_id = slice_regs->id;
	struct hantrovcmd_dev *dev = NULL;
	struct cmdbuf_obj *obj = NULL;
	bi_list_node *node = NULL;
	struct hantrodec_dev *dec_dev;
	vdec_power_mgr *pm = NULL;
	volatile u8 *hwregs;
	unsigned long flags, flags2;
	u32 reg_id;
	int i;
	u32 timeout_val;
	int ret;

	/* fuzz report */
	if (cmdbuf_id >= SLOT_NUM_CMDBUF) {
		vcmd_klog(LOGLVL_WARNING, "cmdbuf_id(%u) >= SLOT_NUM_CMDBUF(%u)\n", cmdbuf_id, SLOT_NUM_CMDBUF);
		return -EINVAL;
	}
	node = &vcmd_mgr->nodes[cmdbuf_id];
	obj = (struct cmdbuf_obj *)node->data;

	if (obj->owner == NULL) {
		vcmd_klog(LOGLVL_ERROR, "%s: cmdbuf owner is NONE !!!\n", __func__);
		return 0;
	}

	if (obj->slice_run_done != 2) {
		vcmd_klog(LOGLVL_ERROR, "%s: cmdbuf is not in slice decoding !!!\n", __func__);
		return 0;
	}

	if (obj->core_id >= vcmd_mgr->subsys_num) {
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) >= vcmd_mgr->subsys_num(%d)!\n",
		    __func__, obj->core_id, vcmd_mgr->subsys_num);
		return -EINVAL;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];
	hwregs = dev->subsys_info->hwregs[SUB_MOD_MAIN];

	spin_lock_irqsave(dev->spinlock, flags);
	_vcmd_del_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
	spin_unlock_irqrestore(dev->spinlock, flags);

	trace_flush_slice_regs("[dec] flush slice regs: cmdbuf ", obj->cmdbuf_id);
	if (obj->xring_slice_mode)
		vcmd_wfd_trace("flush slice regs: cmdbuf[%d]\n", obj->cmdbuf_id);

	dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
	if (!dec_dev || !dec_dev->pm) {
		vcmd_klog(LOGLVL_ERROR, "pm or dec_dev[%p] is NULL!\n", dec_dev);
		return -EINVAL;
	}
	pm = dec_dev->pm;
	/* check if device is suspended */
	mutex_lock(&pm->pm_lock);
	ret = regulator_is_enabled(pm->vdec_rg);
	if (ret <= 0) {
		vcmd_klog(LOGLVL_WARNING, "vdec core has been powered off. ret=%d\n", ret);
		mutex_unlock(&pm->pm_lock);
		return -EBUSY;
	}

	spin_lock_irqsave(dev->spinlock, flags);
	if (obj->executing_status == CMDBUF_EXE_STATUS_SLICE_DECODING_ABORTED) {
		vcmd_klog(LOGLVL_WARNING, "buffer empty cmdbuf has been aborted\n");
		spin_unlock_irqrestore(dev->spinlock, flags);
		mutex_unlock(&pm->pm_lock);
		return -EBUSY;
	}

	spin_lock_irqsave(&dev->abn_irq_lock, flags2);
	for (i = 0; i < slice_regs->reg_num - 1; i++) {
		reg_id = slice_regs->regs[i].reg_id;
		iowrite32(slice_regs->regs[i].reg_data, (void __iomem *)(hwregs + 4 * reg_id));
	}

	iowrite32(slice_regs->regs[i].reg_data, (void __iomem *)(hwregs + 0x4));
	//enable sw_timeout_override_e (bit 31 of reg319), to start decoder timer
	timeout_val = (u32)ioread32((void __iomem *)(hwregs + 0x4 * 319));
	iowrite32((timeout_val | (0x80000000)), (void __iomem *)(hwregs + 0x4 * 319));
	obj->slice_run_done = 0;

	vcmd_write_reg((const void *)dev->hwregs,
		VCMD_REGISTER_EXT_INT_GATE_OFFSET,
		dev->intr_gate_mask);

	spin_unlock_irqrestore(&dev->abn_irq_lock, flags2);

#ifdef SUPPORT_WATCHDOG
	_vcmd_watchdog_feed(dev, WD_FEED_RESUME);
#endif
	spin_unlock_irqrestore(dev->spinlock, flags);


	mutex_unlock(&pm->pm_lock);

	return 0;
}

/**
 * @brief abort the decoder of the cmdbuf_id which is waiting more slice.
 */
static long abort_cmdbuf(vcmd_mgr_t *vcmd_mgr, u16 cmdbuf_id)
{
	struct hantrovcmd_dev *dev = NULL;
	struct cmdbuf_obj *obj = NULL;
	bi_list_node *node = NULL;
	volatile u8 *hwregs;
	u16 reg_id_exe;
	unsigned long flags;

	/* fuzz report */
	if (cmdbuf_id >= SLOT_NUM_CMDBUF) {
		vcmd_klog(LOGLVL_ERROR, "cmdbuf_id(%u) >= SLOT_NUM_CMDBUF(%u)\n", cmdbuf_id, SLOT_NUM_CMDBUF);
		return -EINVAL;
	}
	node = &vcmd_mgr->nodes[cmdbuf_id];
	obj = (struct cmdbuf_obj *)node->data;

	if (obj->core_id >= vcmd_mgr->subsys_num) {
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) >= vcmd_mgr->subsys_num(%d)!\n",
		    __func__, obj->core_id, vcmd_mgr->subsys_num);
		return -EINVAL;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];
	hwregs = dev->subsys_info->hwregs[SUB_MOD_MAIN];
	reg_id_exe = (u16)(*(dev->reg_mem_va + REG_ID2_CMDBUF_EXE_ID));
	if (cmdbuf_id == reg_id_exe) {
		spin_lock_irqsave(dev->spinlock, flags);
		abort_vcd(hwregs);
		spin_unlock_irqrestore(dev->spinlock, flags);
	} else {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "ERROR cmdbuf id not match with current dev!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief drop cmdbufs_id or releae cmd from this po
 */
static long drop_release_cmdbufs(vcmd_mgr_t *vcmd_mgr,
								struct proc_obj *po, void *owner) {
	struct hantrovcmd_dev *dev;
	bi_list *list;
	bi_list_node *node;
	struct cmdbuf_obj *obj, *curr_obj = NULL;
	u32 i, handled, to_drop;
	u32 has_work_node;
	unsigned long flags;
	int aborted_cmdbuf_id;
	long dropped_cmdbuf_num = 0;
	int ret = 0;
	u32 num = 0;
	struct hantrodec_dev *dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
	vdec_power_mgr *pm = NULL;

	WARN_ON(!dec_dev);
	WARN_ON(!dec_dev->pm);

	handled = 0;

	pm = dec_dev->pm;
	mutex_lock(&pm->pm_lock);
	//remove nodes in dev->work_list
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		if (dev == NULL || dev->hwregs == NULL)
			continue;

		list = &dev->work_list;

		has_work_node = 0;

		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			mutex_unlock(&pm->pm_lock);
			return -EINTR;
		}
		node = list->head;
		while (node) {
			obj = (struct cmdbuf_obj *)node->data;
			if (obj->po == po) {
				if ((owner == NULL) || (owner == obj->owner)) {
					if (po->vcmd_get_slice_info_flags && obj->xring_slice_mode)
						po->vcmd_get_slice_info_flags = 0;
					has_work_node = 1;
					break;
				}
			}
			node = node->next;
			num++;
			if (num > SLOT_NUM_CMDBUF) {
				/* this function not return.
				* and dump all cmdbuf_id.
				*/
				hantor_dump_all_cmdbuf_id_by_list(&dev->work_list, __func__, __LINE__);
			}
		}
		up(&dev->work_sem);

		if (has_work_node) {
			spin_lock_irqsave(dev->spinlock, flags);
			if (dev->state == VCMD_STATE_WORKING) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				vcmd_klog(LOGLVL_DEBUG, "Abort dev[%d].\n", dev->core_id);
				ret = vcmd_abort(dev, &aborted_cmdbuf_id);
				/* fuzz report */
				if (ret < 0) {
					vcmd_klog(LOGLVL_ERROR, "vcmd_abort failed!\n");
					continue;
				}
				if (CMDBUF_ID_CHECK(aborted_cmdbuf_id) < 0)
					continue;
				curr_obj = &vcmd_mgr->objs[aborted_cmdbuf_id];
				if (ret == 1) {
					vcmd_klog(LOGLVL_WARNING, "%s: Currently it is slice decoding, vcmd abort failed!\n.", __func__);
					if ((curr_obj->owner == owner) ||
						(owner == NULL && curr_obj->po == po)) {
						spin_lock_irqsave(dev->spinlock, flags);
						abort_vcd(dev->subsys_info->hwregs[SUB_MOD_MAIN]);
						spin_unlock_irqrestore(dev->spinlock, flags);

						wait_event_timeout(*dev->abort_waitq,
							(dev->state == VCMD_STATE_IDLE || curr_obj->cmdbuf_run_done),
							msecs_to_jiffies(ONE_SLICE_WAIT_TIME));
						ret = 2;
					} else {
						atomic_set(&curr_obj->need_restart, 0);
					}
				}
				spin_lock_irqsave(dev->spinlock, flags);
				if (ret == 0 || ret == 2) {
					if (dev->state != VCMD_STATE_IDLE &&
						dev->state != VCMD_STATE_POWER_OFF) {
						vcmd_klog(LOGLVL_ERROR, "dev [%d] is not aborted as expected.", dev->core_id);
						spin_unlock_irqrestore(dev->spinlock, flags);
						continue;
					}
				}
			}
			spin_unlock_irqrestore(dev->spinlock, flags);

			if (down_killable(&dev->work_sem)) {
				vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
				mutex_unlock(&pm->pm_lock);
				return -EINTR;
			}
			num = 0;
			node = list->head;
			while (node) {
				obj = (struct cmdbuf_obj *)node->data;
				if (obj->po == po || obj->cmdbuf_need_remove) {
					to_drop = 0;
					if (owner && owner == obj->owner && !obj->cmdbuf_run_done) {
						to_drop = 1;
						handled++;
					}
					if (owner == NULL || to_drop == 1) {
						vcmd_klog(LOGLVL_INFO, "cmdbuf %d of process %p is released or dropped\n",
												obj->cmdbuf_id, obj->filp);
						dev_remove_job2(vcmd_mgr, dev, node);
					}
				}

				node = node->next;
				num++;
				if (num > SLOT_NUM_CMDBUF) {
					/* this function not return.
					* and dump all cmdbuf_id.
					*/
					hantor_dump_cmdbuf_id_by_node(node, __func__, __LINE__);
				}
			}
			if (ret == 1)
				atomic_set(&curr_obj->need_restart, 1);
			up(&dev->work_sem);

			vcmd_klog(LOGLVL_INFO, "Restart dev[%d].\n", dev->core_id);
			if (ret >= 0) {
				if (ret == 1 && atomic_read(&curr_obj->need_restart) &&
					dev->state != VCMD_STATE_WORKING)
					atomic_set(&curr_obj->need_restart, 0);

				spin_lock_irqsave(dev->spinlock, flags);
				vcmd_start(dev);
				spin_unlock_irqrestore(dev->spinlock, flags);
			}
		}
	}

	// remove cmdbuf reserved but not in work_list or has been dropped
	for (i = 0; i < SLOT_NUM_CMDBUF; i++) {
		obj = &vcmd_mgr->objs[i];
		if (obj->po == po) {
			if (owner == NULL ||
				 (owner == obj->owner && !obj->cmdbuf_run_done)) {
				if (owner)
					return_process_resource(obj->po, obj);
				if (obj->core_id >= vcmd_mgr->subsys_num
					  && obj->core_id != ANY_CMDBUF_ID) {
					vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) >= vcmd_mgr->subsys_num(%d)!\n",
						__func__, obj->core_id, vcmd_mgr->subsys_num);
					mutex_unlock(&pm->pm_lock);
					return -EINVAL;
				}
				if (obj->core_id != ANY_CMDBUF_ID) {
					dev = &vcmd_mgr->dev_ctx[obj->core_id];
					spin_lock_irqsave(dev->po_spinlock, flags);
					return_cmdbuf(vcmd_mgr, i);
					spin_unlock_irqrestore(dev->po_spinlock, flags);
				} else {
					return_cmdbuf(vcmd_mgr, i);
				}
			}
		}
	}

	if (owner && handled)
		dropped_cmdbuf_num = handled;
	wake_up_interruptible_all(&po->job_waitq);
	mutex_unlock(&pm->pm_lock);
	return dropped_cmdbuf_num;
}

/**
 * @brief drop owner from current po
 */
static long drop_owner(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
												void *owner)
{
	u32 module_type;
	long dropped_cmdbuf_num = 0;

	module_type = po->module_type;
	/* fuzz report */
	if (module_type >= MAX_VCMD_TYPE) {
		vcmd_klog(LOGLVL_WARNING, "module_type(%u) >= MAX_VCMD_TYPE(%u)\n", module_type, MAX_VCMD_TYPE);
		return -EINVAL;
	}
	if (!vcmd_mgr->module_mgr[module_type].sem.wait_list.prev
		&& !vcmd_mgr->module_mgr[module_type].sem.wait_list.next) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}

	if (down_killable(&vcmd_mgr->module_mgr[module_type].sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		return -EINTR;
	}

	dropped_cmdbuf_num = drop_release_cmdbufs(vcmd_mgr, po, owner);

	up(&vcmd_mgr->module_mgr[module_type].sem);
	return dropped_cmdbuf_num;
}

/**
 * @brief wait the cmdbuf with special owner done
 */
static long wait_owner_done(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po, void *owner)
{
	struct cmdbuf_obj *obj = NULL;
	u32 module_type;
	u32 i, ret;

	ret = 0;
	module_type = po->module_type;
	/* fuzz report */
	if (module_type >= MAX_VCMD_TYPE) {
		vcmd_klog(LOGLVL_WARNING, "module_type(%u) >= MAX_VCMD_TYPE(%u)\n", module_type, MAX_VCMD_TYPE);
		return -EINVAL;
	}
	if (!vcmd_mgr->module_mgr[module_type].sem.wait_list.prev
		&& !vcmd_mgr->module_mgr[module_type].sem.wait_list.next) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}
	if (down_interruptible(&vcmd_mgr->module_mgr[module_type].sem)) {
		vcmd_klog(LOGLVL_WARNING, "down_interruptible is interrupted.\n");
		return -ERESTARTSYS;
	}

	for(i=0; i<SLOT_NUM_CMDBUF; i++) {
		obj = &vcmd_mgr->objs[i];
		if (obj->po == po && obj->owner == owner) {
			ret = 1;
			break;
		}
	}

	up(&vcmd_mgr->module_mgr[module_type].sem);
	return ret;
}

static bool _runtime_context_idled(vcmd_mgr_t *vcmd_mgr)
{
	int i;
	bool is_empty = true;
	unsigned long flags;
	struct hantrovcmd_dev *dev;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		spin_lock_irqsave(dev->spinlock, flags);
		if (dev_wait_job_num(dev)) {
			is_empty = false;
			spin_unlock_irqrestore(dev->spinlock, flags);
			break;
		}
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	return is_empty;
}

/**
 * @brief ioctl function of vcmd driver.
 */
long hantrovcmd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	long tmp;
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;
	u16 cmdbuf_id;
	struct hantrodec_dev *dec_dev;
	vdec_power_mgr *pm = NULL;
	int ret;

	_log_ioctl_cmd(cmd);
	/*
	 * extract the type and number bitfields, and don't encode
	 * wrong cmds: return ENOTTY (inappropriate ioctl)
	 * before access_ok()
	 */
	if (_IOC_TYPE(cmd) != HANTRO_VCMD_IOC_MAGIC)
		return -ENOTTY;
	if ((_IOC_TYPE(cmd) == HANTRO_VCMD_IOC_MAGIC &&
		 _IOC_NR(cmd) > HANTRO_VCMD_IOC_MAXNR))
		return -ENOTTY;

		/*
		 * the direction is a bitmask, and VERIFY_WRITE catches R/W
		 * transfers. `Type' is user-oriented, while
		 * access_ok is kernel-oriented, so the concept of "read" and
		 * "write" is reversed
		 */
#if (KERNEL_VERSION(5, 0, 0) > LINUX_VERSION_CODE)
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg,
				 _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *)arg,
				 _IOC_SIZE(cmd));
#else
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
#endif
	if (err)
		return -EFAULT;

	switch (cmd) {
	case HANTRO_VCMD_IOCH_GET_CMDBUF_PARAMETER: {
		struct cmdbuf_mem_parameter mem;

		vcmd_klog(LOGLVL_DEBUG, "VCMD get cmdbuf parameter\n");
		mem.cmdbuf_unit_size = SLOT_SIZE_CMDBUF;
		mem.status_cmdbuf_unit_size = SLOT_SIZE_STATUSBUF;
		mem.vcmd_regbuf_unit_size = SLOT_SIZE_REGBUF;
		mem.cmdbuf_total_size = vcmd_mgr->mem_vcmd.size;
		mem.phy_cmdbuf_addr = vcmd_mgr->mem_vcmd.pa;
		mem.status_cmdbuf_total_size = vcmd_mgr->mem_status.size;
		mem.phy_status_cmdbuf_addr = vcmd_mgr->mem_status.pa;
		mem.vcmd_regbuf_total_size = vcmd_mgr->mem_regs.size;
		mem.phy_vcmd_regbuf_addr = vcmd_mgr->mem_regs.pa;
		mem.mmu_phy_status_cmdbuf_addr = vcmd_mgr->mem_status.mmu_ba;
		mem.mmu_phy_cmdbuf_addr = vcmd_mgr->mem_vcmd.mmu_ba;
		mem.mmu_phy_vcmd_regbuf_addr = vcmd_mgr->mem_regs.mmu_ba;
		mem.base_ddr_addr = vcmd_mgr->pa_trans_offset;
		mem.virt_cmdbuf_addr = NULL;
		tmp = copy_to_user((struct cmdbuf_mem_parameter __user *)arg,
				 &mem, sizeof(struct cmdbuf_mem_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		break;
	}
	case HANTRO_VCMD_IOCH_GET_VCMD_PARAMETER: {
		struct config_parameter param;
		struct proc_obj *po = NULL;
		u16 m_type;
		struct hantrovcmd_dev *dev;
		struct vcmd_subsys_info *info;

		vcmd_klog(LOGLVL_DEBUG, "VCMD get vcmd config parameter\n");
		tmp = copy_from_user(&param,
				   (struct config_parameter __user *)arg,
				   sizeof(struct config_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		po = (struct proc_obj *)ctx->po;
		if (!po) {
			return -1;
		}

		po->module_type = param.module_type;
		m_type = param.module_type;

		/* fuzz report */
		if (m_type >= MAX_VCMD_TYPE) {
			vcmd_klog(LOGLVL_ERROR, "m_type(%u) >= MAX_VCMD_TYPE(%u)\n", m_type, MAX_VCMD_TYPE);
			return -EINVAL;
		}
		if (vcmd_mgr->module_mgr[m_type].num) {
			dev = vcmd_mgr->module_mgr[m_type].dev[0];
			info = dev->subsys_info;
			param.submodule_main_addr = info->reg_off[SUB_MOD_MAIN];
			param.submodule_dec400_addr = info->reg_off[SUB_MOD_DEC400];
			param.submodule_MMU_addr = info->reg_off[SUB_MOD_MMU];
			param.submodule_MMUWrite_addr = info->reg_off[SUB_MOD_MMU_WR];
			param.submodule_axife_addr = info->reg_off[SUB_MOD_AXIFE];
			param.vcmd_hw_version_id =	dev->hw_version_id;
			param.vcmd_core_num = vcmd_mgr->module_mgr[m_type].num;
		} else {
			param.submodule_main_addr = 0xffff;
			param.submodule_dec400_addr = 0xffff;
			param.submodule_MMU_addr = 0xffff;
			param.submodule_MMUWrite_addr = 0xffff;
			param.submodule_axife_addr = 0xffff;
			param.vcmd_core_num = 0;
			param.vcmd_hw_version_id = HW_ID_1_0_C;
		}
		tmp = copy_to_user((struct config_parameter __user *)arg, &param,
				 sizeof(struct config_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
			return -EFAULT;
		}

		break;
	}
	case HANTRO_VCMD_IOCH_RESERVE_CMDBUF: {
		long ret;
		struct exchange_parameter param;
		struct proc_obj *po = _GET_PO(filp);

		tmp = copy_from_user(&param,
				   (struct exchange_parameter __user *)arg,
				   sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		ret = reserve_cmdbuf(vcmd_mgr, po, &param);
		if (ret == 0) {
			tmp = copy_to_user((struct exchange_parameter __user *)arg,
					 &param,
					 sizeof(struct exchange_parameter));
			if (tmp) {
				vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
				return -EFAULT;
			}
		}
		vcmd_klog(LOGLVL_DEBUG, "VCMD Reserve CMDBUF %d\n", param.cmdbuf_id);
		return ret;
	}

	case HANTRO_VCMD_IOCH_LINK_RUN_CMDBUF: {
		struct exchange_parameter param;
		long retVal;
		struct proc_obj *po = _GET_PO(filp);

		tmp = copy_from_user(&param,
				   (struct exchange_parameter __user *)arg,
				   sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}

		vcmd_klog(LOGLVL_DEBUG, "VCMD link and run CMDBUF %d\n", param.cmdbuf_id);
		retVal = link_and_run_cmdbuf(vcmd_mgr, po, &param, NORMAL_STAGE);
		tmp = copy_to_user((struct exchange_parameter __user *)arg,
				 &param, sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		return retVal;
	}

	case HANTRO_VCMD_IOCH_WAIT_CMDBUF: {
		u16 cmdbuf_id;
		long ret, tmp;
		struct proc_obj *po = _GET_PO(filp);
		struct wait_cmdbuf cmdbuf;

		tmp = copy_from_user(&cmdbuf, (struct wait_cmdbuf __user *)arg,
				   sizeof(struct wait_cmdbuf));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		cmdbuf_id = cmdbuf.id;
		/*high 16 bits are core id, low 16 bits are cmdbuf_id*/

		vcmd_klog(LOGLVL_INFO, "VCMD wait for CMDBUF %d start.\n", cmdbuf.id);
		ret = wait_cmdbuf_ready(vcmd_mgr, po, cmdbuf_id, &cmdbuf);
		vcmd_klog(LOGLVL_INFO, "VCMD wait for CMDBUF %d end. ret=%ld.\n", cmdbuf.id, ret);
		/* vcmd suspend */
		vcmd_do_runtime_suspend(vcmd_mgr, !!ret);
		if (ret == 0) {
			tmp = copy_to_user((struct wait_cmdbuf __user *)arg,
					 &cmdbuf, sizeof(struct wait_cmdbuf));
			if (tmp) {
				vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
				return -EFAULT;
			}
			return ret;
		}
		return -1;

		break;
	}

	case HANTRO_VCMD_TOCH_GET_CMDBUF_STATUS: {
		struct wait_cmdbuf cmdbuf;
		long tmp;

		vcmd_klog(LOGLVL_INFO, "VCMD get cmdbuf execute status.\n");

		tmp = copy_from_user(&cmdbuf, (struct wait_cmdbuf __user *)arg,
		sizeof(struct wait_cmdbuf));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		if (CMDBUF_ID_CHECK(cmdbuf.id) < 0)
			return -1;

		cmdbuf.exe_status = vcmd_mgr->objs[cmdbuf.id].executing_status;
		tmp = copy_to_user((struct wait_cmdbuf __user *)arg,
		&cmdbuf, sizeof(struct wait_cmdbuf));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %li\n", tmp);
			return -EFAULT;
		}

		return 0;
	}

	case HANTRO_VCMD_IOCH_PUSH_SLICE_REG: {
		struct subsys_regs_desc slice_regs;
		vcmd_klog(LOGLVL_DEBUG, "VCMD push slice reg for buffer empty.\n");

		/* get registers from user space*/
		tmp = copy_from_user(&slice_regs, (void __user *)arg,
				     sizeof(struct subsys_regs_desc));
		if (tmp) {
			PDEBUG("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		return flush_slice_regs(vcmd_mgr, &slice_regs);
	}

	case HANTRO_VCMD_IOCH_ABORT_CMDBUF: {

		vcmd_klog(LOGLVL_DEBUG, "VCMD abort slice for seek.\n");
		/* get registers from user space*/
		tmp = __get_user(cmdbuf_id, (u16 __user *)arg);
		if (tmp) {
			PDEBUG("copy_from_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);
		if (!dec_dev || !dec_dev->pm) {
			vcmd_klog(LOGLVL_ERROR, "pm or dec_dev[%p] is NULL!\n", dec_dev);
			return -EINVAL;
		}
		pm = dec_dev->pm;
		/* fuzz report */
		mutex_lock(&pm->pm_lock);
		ret = regulator_is_enabled(pm->vdec_rg);
		if (ret <= 0) {
			vcmd_klog(LOGLVL_WARNING, "vdec core has been powered off. ret=%d\n", ret);
			mutex_unlock(&pm->pm_lock);
			return -EBUSY;
		}

		tmp = abort_cmdbuf(vcmd_mgr, cmdbuf_id);
		if (tmp) {
			PDEBUG("abort_cmdbuf failed, returned %li\n", tmp);
			mutex_unlock(&pm->pm_lock);

			return -EFAULT;
		}
		mutex_unlock(&pm->pm_lock);

		break;
	}

	case HANTRO_VCMD_IOCH_DROP_OWNER: {
		long dropped_cmdbuf_num = 0;
		struct proc_obj *po = _GET_PO(filp);

		vcmd_klog(LOGLVL_DEBUG, "VCMD drop owner %p for seek.\n", (void *)arg);

		tmp = drop_owner(vcmd_mgr, po, (void *)arg);
		if (tmp < 0) {
			vcmd_klog(LOGLVL_ERROR, "drop_owner failed, returned %li\n", tmp);
			return -EFAULT;
		}
		dropped_cmdbuf_num = tmp;

		return dropped_cmdbuf_num;
	}

	case HANTRO_VCMD_IOCH_WAIT_OWNER_DONE: {
		struct proc_obj *po = _GET_PO(filp);

		vcmd_klog(LOGLVL_DEBUG, "VCMD wait owner %p done.\n", (void *)arg);

		tmp = wait_owner_done(vcmd_mgr, po, (void *)arg);
		if (tmp) {
			vcmd_klog(LOGLVL_DEBUG, "wait_cmdbuf_done failed, returned %li\n", tmp);
			return -EFAULT;
		}

		return 0;
	}

	case HANTRO_VCMD_IOCH_RELEASE_CMDBUF: {
		u16 cmdbuf_id;
		struct proc_obj *po = _GET_PO(filp);

		__get_user(cmdbuf_id, (u16 __user *)arg);
		/*16 bits are cmdbuf_id*/

		vcmd_klog(LOGLVL_DEBUG, "VCMD release CMDBUF %d\n", cmdbuf_id);

		release_cmdbuf(vcmd_mgr, po, cmdbuf_id);
		return 0;
		//break;
	}
	case HANTRO_VCMD_IOCH_POLLING_CMDBUF: {
		vcmd_klog(LOGLVL_ERROR, "dwl should not send HANTRO_VCMD_IOCH_POLLING_CMDBUF ioctl.\n");
		return -EINVAL;
	}
	default:
		break;
	}
	return 0;
}

/**
 * @brief open hantro vcmd device.
 */
int hantrovcmd_open(struct inode *inode, struct file *filp)
{
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;
	struct proc_obj *po;
#ifdef SUPPORT_DBGFS
	/* for debugfs */
	_dbgfs_init_ctx(vcmd_mgr, 1);
#endif
	po = create_process_object();
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "Create process object failed!\n");
		return -EINVAL;
	}
	po->filp = filp;
	po->module_type = vcmd_mgr->core_array[0].sub_module_type;
	ctx->po = (void *)po;

	vcmd_klog(LOGLVL_DEBUG, "dev opened\n");
	vcmd_klog(LOGLVL_INFO, "process obj %p for filp opened %p\n", (void *)po, (void *)filp);
	return 0;
}

/**
 * @brief release hantro vcmd device.
 */
int hantrovcmd_release(struct inode *inode, struct file *filp)
{
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;
	struct proc_obj *po;
	int get_down_flag = 0;
	int ret = 0;

	u32 module_type;

	vcmd_klog(LOGLVL_INFO, "process %p start release\n", (void *)filp);
	po = (struct proc_obj *)ctx->po;
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "not find process obj!\n");
		vfree(ctx);
		return -EINVAL;
	}

	if (po->xplayer_iova[0] != 0) {
		unmap_slice_buf(&vcmd_mgr->platformdev->dev, po);
	}

	module_type = po->module_type;
	/* fuzz report */
	if (module_type >= MAX_VCMD_TYPE) {
		vcmd_klog(LOGLVL_WARNING, "module_type(%u) >= MAX_VCMD_TYPE(%u)\n", module_type, MAX_VCMD_TYPE);
		ret = -EINVAL;
		goto out;
	}
	if (!vcmd_mgr->module_mgr[module_type].sem.wait_list.prev
		&& !vcmd_mgr->module_mgr[module_type].sem.wait_list.next) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		ret = -EINVAL;
		goto out;
	}

	down(&vcmd_mgr->module_mgr[module_type].sem);
	get_down_flag = 1;
	ret = drop_release_cmdbufs(vcmd_mgr, po, NULL);

out:
	vcmd_klog(LOGLVL_INFO, "process obj %p for filp to be removed: %p\n",
			(void *)po, (void *)po->filp);
	free_process_object(ctx->po);
	ctx->po = NULL;
	vfree(ctx);
	if (get_down_flag)
		up(&vcmd_mgr->module_mgr[module_type].sem);
	if (ret >= 0)
		return 0;
	else
		return -ERESTARTSYS;
}

/**
 * @brief process abnormal interrupt
 */
static void process_abnormal_irq(vcmd_mgr_t *vcmd_mgr,
									struct hantrovcmd_dev *dev,
									struct cmdbuf_obj *obj)
{
	u32 intr_src;

	dev->reset_count = 0;
	dev->wdt_trigger_count = 0;
	/* check vcmd interrupt source. */
	intr_src = vcmd_read_reg((const void *)dev->hwregs,
								VCMD_REGISTER_SW_EXT_INT_SRC_OFFSET);
	vcmd_klog(LOGLVL_DEBUG, "vcmd interrupt source is 0x%x.\n", intr_src);
	/* abnormal interrupts source from VCD */
	if (intr_src & DEC_ABNORMAL_IRQ_MASK)
		process_vcd_abn_irq(vcmd_mgr, dev, obj);
}

/**
 * @brief mark cmdbuf obj as run_done with specified exe_status,
 * remove it from device work_list, and add it to owner po's job_done_list.
 */
static int isr_process_node(vcmd_mgr_t *vcmd_mgr,
								bi_list_node *node,
								u32 exe_status) {
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;
	struct hantrovcmd_dev *dev;
	unsigned long flags;
	u32 *status_va;

	if (obj->core_id > 0) {
		/* should not happen */
		vcmd_klog(LOGLVL_ERROR, "%s:obj->core_id(%d) > 0!\n",
			__func__, obj->core_id);
		return -EINVAL;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];

	dev->reset_count = 0;
	dev->wdt_trigger_count = 0;
	if (obj->cmdbuf_run_done == 0) {
		obj->cmdbuf_run_done = 1;
		obj->executing_status = exe_status;
		status_va = obj->status_va + dev->subsys_info->reg_off[SUB_MOD_MAIN] / 2 / 4 + 1;
		*status_va &= (~VCD_STATUS_SLICE_READY_INT);
		dev_delink_job(dev, node, vcmd_mgr);
		spin_lock_irqsave(dev->po_spinlock, flags);
		proc_add_done_job(vcmd_mgr, obj);
		obj->cmdbuf_processed = 1;
		spin_unlock_irqrestore(dev->po_spinlock, flags);
		if (obj->po && obj->po->in_wait)
			wake_up_interruptible_all(&obj->po->job_waitq);

#ifdef SUPPORT_DBGFS
		if (exe_status == CMDBUF_EXE_STATUS_OK) {
			_dbgfs_remove_cmdbuf(dev->dbgfs_info, obj->cmdbuf_id);
			_dbgfs_record_vcd_cycles(dev->dbgfs_info, obj->cmdbuf_id,
						obj->module_type, dev->hw_feature.has_arbiter);
		}
#endif
		return 0;
	} else {
		vcmd_klog(LOGLVL_DEBUG, "cmdbuf[%d] is already done!!\n",
				obj->cmdbuf_id);
		return -1;
	}
}

void process_vcd_slice_outbuf(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	if (obj->xring_slice_mode == 0) {
		return;
	}
	vcmd_klog(LOGLVL_DEBUG, "slice mode get slice irq and wakeup thread \n");
	dev->slice_send_outbuf_flag = 1;
	kfifo_in(&dev->send_slice_buffer_queue, &obj->cmdbuf_id, sizeof(u16));
	_vcmd_kthread_wakeup(vcmd_mgr);
}

/**
 * @brief interrupt bottom half of vcmd driver, when a vcmd interrupt is reported,
 *   ISR will schedule the _irq_work, then the bottom half is excuted in a workqueue's
 *   worker thread.
 */
static void _vcmd_irq_bottom_half(struct work_struct *work)
{
	struct hantrovcmd_dev *dev = container_of(work, struct hantrovcmd_dev, _irq_work);
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)dev->handler;
	u32 irq_status = 0;
	bi_list_node *curr_node, *node;
	struct cmdbuf_obj *curr_obj, *obj;
	u32 curr_id = 0;
	unsigned long flags;
	u64 ns, us;

	/* Check the time taken from the top half to the bottom half */
	dev->bottom_half_start = ktime_get();
	ns = ktime_to_ns(ktime_sub(dev->bottom_half_start, dev->top_half_start));
	us = ns / NSEC_PER_USEC;
	if (us >= THRESHOLD_US)
		vcmd_klog(LOGLVL_WARNING, "vdec th->bh take (%llu)us.\n", us);

	while (1) {
		spin_lock_irqsave(dev->spinlock, flags);
		irq_status = dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4];
		dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] = 0;

		/* currently, abn_irq_schedule is just for slice abnormal irq */
		if (dev->abn_irq_schedule & ABN_IRQ_SCHEDULE_BUFFER_EMPTY) {
			dev->abn_irq_schedule &= (~ABN_IRQ_SCHEDULE_BUFFER_EMPTY);
			if (dev->state == VCMD_STATE_POWER_OFF) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				continue;
			}
			curr_node = vcmd_get_exec_node(dev, 0);
			spin_unlock_irqrestore(dev->spinlock, flags);
			if (!curr_node) {
				vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n", __func__);
				return;
			}
			obj = (struct cmdbuf_obj *)curr_node->data;
			spin_lock_irqsave(&dev->abn_irq_lock, flags);
			obj->slice_run_done = 2;
			spin_unlock_irqrestore(&dev->abn_irq_lock, flags);
			proc_add_done_job(vcmd_mgr, obj);

			spin_lock_irqsave(dev->spinlock, flags);
			if (dev->state == VCMD_STATE_POWER_OFF) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				continue;
			}
			if (vcmd_get_register_value((const void *)dev->hwregs,
				dev->reg_mirror,
				HWIF_VCMD_START_TRIGGER) == 0)
				wake_up_all(dev->abort_waitq);
			spin_unlock_irqrestore(dev->spinlock, flags);
			if (irq_status == 0) {
				continue;
			}
		} else {
			spin_unlock_irqrestore(dev->spinlock, flags);
		}
		if (irq_status == 0) {
			break;
		}

		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			continue;
		}

#ifdef SUPPORT_DBGFS
		_dbgfs_reset_exe_cmdbuf_num(dev->dbgfs_info);
		_dbgfs_record_cmdbuf_num(dev->dbgfs_info);
#endif

		node = dev->work_list.head;
		if (node == NULL) {
			//dev is not in use, should not run to here
			vcmd_klog(LOGLVL_WARNING, "received IRQ but core has nothing to do.\n");
			up(&dev->work_sem);
			continue;
		}

		// get current node/obj/id
		spin_lock_irqsave(dev->spinlock, flags);
		if (dev->state == VCMD_STATE_POWER_OFF) {
			spin_unlock_irqrestore(dev->spinlock, flags);
			up(&dev->work_sem);
			continue;
		}
		curr_node = vcmd_get_exec_node(dev, irq_status);
		spin_unlock_irqrestore(dev->spinlock, flags);
		if (!curr_node) {
			vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n", __func__);
			up(&dev->work_sem);
			continue;
		}
		curr_obj = (struct cmdbuf_obj *)curr_node->data;
		curr_id = curr_obj->cmdbuf_id;

#ifdef VCMD_DEBUG_INTERNAL
		_dbg_log_dev_regs(dev, 0);
#endif

		if (curr_obj->core_id != dev->core_id) {
			vcmd_klog(LOGLVL_ERROR, "error cmdbuf_id %d, core_id[%d] is not dev id[%d] !!\n",
						curr_id, curr_obj->core_id, dev->core_id);
			up(&dev->work_sem);
			continue;
		}

		/*
		 * process nodes between head node and curr_node (exclusive):
		 * 1. mark run_done = 1 & executing status = OK,
		 * 2. remove from dev work list.
		 * 3. add to owner's job_done_list
		 */
		if (curr_obj->cmdbuf_run_done == 0) {
			node = dev->work_list.head;
			while (node && node != curr_node) {
				obj = (struct cmdbuf_obj *)node->data;
				isr_process_node(vcmd_mgr, node, CMDBUF_EXE_STATUS_OK);

				node = node->next;
			}

			if (node == NULL) {
				vcmd_klog(LOGLVL_WARNING, "not find node[%d] in dev work_list!!\n",
							curr_obj->cmdbuf_id);
				wake_up_all(dev->abort_waitq);
				up(&dev->work_sem);
				continue;
			}
		} else {
			// only occurs when for error irq
			if (!(irq_status & VCMD_IRQ_ERR_MASK)) {
				vcmd_klog(LOGLVL_INFO, "normal irq trigger to already done cmdbuf[%d]\n",
							curr_obj->cmdbuf_id);
			}
		}

		if (dev->hw_feature.has_arbiter &&
			(irq_status & VCMD_IRQ_ARBITER_RESET)) {
			// vcmd arbiter reset err
			dev->arb_reset_irq = 1;
			up(&dev->work_sem);
			continue;
		}

		if (dev->hw_feature.has_arbiter &&
			(irq_status & VCMD_IRQ_ARBITER_ERR)) {
			// vcmd arbiter err when access vpu core regs but not own arbiter
			dev->arb_err_irq = 1;
			up(&dev->work_sem);
			continue;
		}

		if (dev->error_mask) {
			vcmd_klog(LOGLVL_WARNING, "dev->error_mask = 0x%x\n", dev->error_mask);
			dev->error_mask = 0;
		}

		//curr_node process
		if (irq_status & VCMD_IRQ_ABORT) {
			/*
			* Check the time taken from the abort of top half
			* to the abort of bottom half
			*/
			dev->bottom_half_abort_start = ktime_get();
			ns = ktime_to_ns(ktime_sub(dev->bottom_half_abort_start, dev->top_half_abort_start));
			us = ns / NSEC_PER_USEC;
			if (us >= VCMD_ABORT_THRESHOLD_US)
				vcmd_klog(LOGLVL_WARNING, "vdec th_abort->bh_abort take (%llu)us.\n", us);

			trace_hantrovcmd_isr_finish_abort("[dec] finish frame(hw abort): cmdbuf ", curr_id);
			//abort error
			spin_lock_irqsave(dev->spinlock, flags);
			_vcmd_del_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
#ifdef TIMEOUT_IRQ_TIMER
			/* if vcmd abort waited, del vcmd timeout timer */
			_vcmd_del_timer(dev, VCMD_TIMER_TIMEOUT);
#endif
#ifdef SUPPORT_WATCHDOG
			_vcmd_watchdog_stop(dev);
#endif

			dev->state = VCMD_STATE_IDLE;
			spin_unlock_irqrestore(dev->spinlock, flags);
			dev->aborted_cmdbuf_id = curr_id;

			if (dev->abort_mode == 0) {
				// curr node is done
				isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_OK);
			} else {
				// curr node is aborted
				/* if current obj is aborted imedialy in slice decoding pm suspend, set the
				* executing_status as SLICE_DECODING_SUSPEND.
				*/
				dev->reset_count = 0;
				dev->wdt_trigger_count = 0;
				curr_obj->executing_status = CMDBUF_EXE_STATUS_ABORTED;
			}
		if (atomic_read(&curr_obj->need_restart)) {
			atomic_set(&curr_obj->need_restart, 0);
				spin_lock_irqsave(dev->spinlock, flags);
				vcmd_start(dev);
				spin_unlock_irqrestore(dev->spinlock, flags);
			}
			up(&dev->work_sem);

			//to notify owner which triggered the abort
			wake_up_all(dev->abort_waitq);
			continue;
		}
#if 0
		/* not report bus err interrupt, because it will trigger abort
		* and ensure vcmd is idle.
		*/
		if (irq_status & VCMD_IRQ_BUS_ERR) {
			//bus error, don't need to reset where to record status?
			isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_BUSERR);

			vcmd_start2(dev, 0);


			up(&dev->work_sem);

			continue;
		}
#endif
		if (irq_status & VCMD_IRQ_TIMEOUT) {
			//time out
			trace_hantrovcmd_isr_timeout("[dec] hw timeout: cmdbuf ", curr_id);
			vcmd_klog(LOGLVL_WARNING, "[dec] hw timeout curr_id=%#x end=%s\n",
				curr_id, irq_status & VCMD_IRQ_END ? "yes":"no");
#ifdef TIMEOUT_IRQ_TIMER
			if ((irq_status & VCMD_IRQ_END) == 0) {
				// start a timer to wait abort irq
				spin_lock_irqsave(dev->spinlock, flags);
				_vcmd_add_timer(dev, VCMD_TIMER_TIMEOUT);
				spin_unlock_irqrestore(dev->spinlock, flags);
			}
#else //TIMEOUT_IRQ_TIMER
			//reset dev and re-start from curr node
			vcmd_start2(dev, 1);
#endif //TIMEOUT_IRQ_TIMER
			up(&dev->work_sem);
			continue;
		}
		if (irq_status & VCMD_IRQ_CMD_ERR) {
			trace_hantrovcmd_isr_err("[dec] finish frame(cmd error): cmdbuf ", curr_id);
			dev->state = VCMD_STATE_IDLE;
#ifdef TIMEOUT_IRQ_TIMER
			/* if vcmd cmderr waited, del vcmd timeout timer */
			spin_lock_irqsave(dev->spinlock, flags);
			_vcmd_del_timer(dev, VCMD_TIMER_TIMEOUT);
			spin_unlock_irqrestore(dev->spinlock, flags);

#endif

			isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_CMDERR);
			//command error, re-start from next node
			vcmd_start2(dev, 0);
			up(&dev->work_sem);
			return;
		}

		//JMP or END interrupt
		trace_hantrovcmd_isr_finish_success("[dec] finish frame(success): cmdbuf ", curr_id);
		if (curr_obj->xring_slice_mode)
			vcmd_wfd_trace("finish frame(success): cmdbuf[%u]\n", curr_id);

		isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_OK);

		spin_lock_irqsave(dev->spinlock, flags);
		_vcmd_del_timer(dev, VCMD_TIMER_BUFFER_EMPTY);
		if (irq_status & VCMD_IRQ_END) {
#ifdef TIMEOUT_IRQ_TIMER
			/* if vcmd end waited, del vcmd timeout timer */
			_vcmd_del_timer(dev, VCMD_TIMER_TIMEOUT);
#endif
			//end command interrupt, start next node
			dev->state = VCMD_STATE_IDLE;
			vcmd_start(dev);

		} else {
#ifdef SUPPORT_WATCHDOG
			if (dev->watchdog_state != WD_STATE_PAUSE)
				_vcmd_watchdog_feed(dev, WD_FEED_ACTIVE);
#endif
		}
		spin_unlock_irqrestore(dev->spinlock, flags);

#ifdef SUPPORT_DBGFS
		_dbgfs_update_index(dev->dbgfs_info);
#endif
		up(&dev->work_sem);
	}
}

/**
 * @brief interrupt service routine of vcmd driver.
 */
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t hantrovcmd_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t hantrovcmd_isr(int irq, void *dev_id)
#endif
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)dev_id;
	struct hantrovcmd_dev *dev = NULL;
	u32 irq_status = 0;
	bi_list_node *curr_node;
	struct cmdbuf_obj *curr_obj;
	bool ret;

	u32 i;
	volatile u8 *hwregs;
	unsigned long flags;

	if (vcmd_mgr->vcmd_irq_enabled == 0) {
		/* all vcmd_irq==-1, there is no IRQ, use irq as dev id */
		if (irq < vcmd_mgr->subsys_num)
			dev = &vcmd_mgr->dev_ctx[irq];
	} else { /* there is assigned IRQ */
		for (i = 0; i < vcmd_mgr->subsys_num; i++) {
			if (vcmd_mgr->dev_ctx[i].subsys_info->irq == irq) {
				dev = &vcmd_mgr->dev_ctx[i];
				break;
			}
		}
	}

	if (dev == NULL)
		return IRQ_HANDLED;

	hwregs = dev->hwregs;

	spin_lock_irqsave(dev->spinlock, flags);

	if (dev->state == VCMD_STATE_POWER_OFF) {
		spin_unlock_irqrestore(dev->spinlock, flags);
		return IRQ_HANDLED;
	}

	irq_status = vcmd_read_reg((const void *)hwregs,
				   VCMD_REGISTER_INT_STATUS_OFFSET);

	if (dev->hw_version_id >= HW_ID_1_5_0 && irq_status == 0) {
		curr_node = vcmd_get_exec_node(dev, 0);
		if (!curr_node) {
			vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n", __func__);
			spin_unlock_irqrestore(dev->spinlock, flags);
			return IRQ_HANDLED;
		}
		curr_obj = (struct cmdbuf_obj *)curr_node->data;

		process_abnormal_irq(vcmd_mgr, dev, curr_obj);

		irq_status = vcmd_read_reg((const void *)dev->hwregs,
									VCMD_REGISTER_INT_STATUS_OFFSET);
		if (!irq_status) {
			if (dev->abn_irq_schedule) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				if (queue_work(dev->wq, &dev->_irq_work))
					dev->top_half_start = ktime_get();
			} else {
				spin_unlock_irqrestore(dev->spinlock, flags);
			}
			return IRQ_HANDLED;
		}
	}

#ifdef VCMD_DEBUG_INTERNAL
	_dbg_log_dev_regs(dev, 1);
#endif

	if (irq_status == 0) {
		//no interrupt source from dev
		spin_unlock_irqrestore(dev->spinlock, flags);
		return IRQ_HANDLED;
	}

	if (irq_status & VCMD_IRQ_CMD_ERR || irq_status & VCMD_IRQ_TIMEOUT || irq_status & VCMD_IRQ_BUS_ERR) {
		dev->error_mask |= irq_status;
	}

	vcmd_write_reg((const void *)hwregs,
						VCMD_REGISTER_INT_STATUS_OFFSET, irq_status);
	dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] = irq_status;

	spin_unlock_irqrestore(dev->spinlock, flags);

	/* schedule work to workqueue */
	ret = queue_work(dev->wq, &dev->_irq_work);
	if (ret) {
		dev->top_half_start = ktime_get();
		if (irq_status & VCMD_IRQ_ABORT)
			dev->top_half_abort_start = ktime_get();
	}

	return IRQ_HANDLED;
}

/**
 * @brief suspend for vcmd driver power management
 */
int vcmd_pm_suspend(void *_vcmd_mgr)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	struct hantrovcmd_dev *dev;
	u32 aborted_id;
	int i, ret = 0;
	unsigned long flags;
	struct cmdbuf_obj *obj;
	struct hantrodec_dev *dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);

	WARN_ON(!dec_dev);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];

		spin_lock_irqsave(dev->spinlock, flags);
		if (dev->state == VCMD_STATE_WORKING) {
			spin_unlock_irqrestore(dev->spinlock, flags);

			ret = vcmd_abort(dev, &aborted_id);

			spin_lock_irqsave(dev->spinlock, flags);
			if (ret == 1) {
				vcmd_klog(LOGLVL_WARNING, "%s: Currently it is slice decoding, vcmd abort failed!\n", __func__);
				if (CMDBUF_ID_CHECK(aborted_id) < 0) {
					spin_unlock_irqrestore(dev->spinlock, flags);
					return -1;
				}
				obj = &vcmd_mgr->objs[aborted_id];
				obj->executing_status = CMDBUF_EXE_STATUS_SLICE_DECODING_SUSPEND;
			} else {
				if (dev->state != VCMD_STATE_IDLE) {
					vcmd_klog(LOGLVL_ERROR, "suspend failed for dev [%d].", dev->core_id);
					spin_unlock_irqrestore(dev->spinlock, flags);
					return -EBUSY;
				}
			}
		}
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	return 0;
}

/**
 * @brief resume for vcmd driver power management
 */
int vcmd_pm_resume(void *_vcmd_mgr)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	struct hantrovcmd_dev *dev;
	int i;
	unsigned long flags;
	u32 curr_id;
	struct cmdbuf_obj *obj;
	bi_list_node *node;
	struct hantrodec_dev *dec_dev = platform_get_drvdata(vcmd_mgr->platformdev);

	WARN_ON(!dec_dev);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		vdec_pm_klog(LOGLVL_INFO, "vcmd_pm_resume dev state %d, dec core power_count [%d].",
			dev->state, atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]));

		spin_lock_irqsave(dev->spinlock, flags);
		obj = &vcmd_mgr->objs[dev->aborted_cmdbuf_id];
		if (dev->state == VCMD_STATE_POWER_ON &&
				obj->executing_status != CMDBUF_EXE_STATUS_SLICE_DECODING_SUSPEND)
			vcmd_start(dev);
		spin_unlock_irqrestore(dev->spinlock, flags);
	}

	return 0;
}
