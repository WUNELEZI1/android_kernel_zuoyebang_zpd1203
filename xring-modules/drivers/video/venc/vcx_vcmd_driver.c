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

#include <linux/kernel.h>
#include <linux/module.h>
/* needed for __init,__exit directives */
#include <linux/init.h>
/* needed for remap_page_range
 *   SetPageReserved
 *   ClearPageReserved
 */
#include <linux/mm.h>
/* obviously, for kmalloc */
#include <linux/slab.h>
/* for struct file_operations, register_chrdev() */
#include <linux/fs.h>
/* standard error codes */
#include <linux/errno.h>

#include <linux/version.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/moduleparam.h>
/* request_irq(), free_irq() */
#include <linux/interrupt.h>
#include <linux/sched.h>

#include <linux/semaphore.h>
#include <linux/spinlock.h>
/* needed for virt_to_phys() */
#include <asm/io.h>
#if (KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE)
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif
#include <linux/mod_devicetable.h>
#include <linux/dma-buf.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>

#include <asm/irq.h>

#include <linux/vmalloc.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/of_device.h>
#include <linux/iommu.h>
#include <linux/regulator/consumer.h>
#include <soc/xring/xring_smmu_wrapper.h>
/* our own stuff */
#include <linux/platform_device.h>
#include "vcmdswhwregisters.h"
#include "bidirect_list.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "vcx_driver.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/venc/vcx_driver.h>
#endif
#include "vcx_vcmd_priv.h"
#include "vcx_abnormal_irq.h"
#include "vcx_vcmd_cfg.h"
#ifdef SUPPORT_DBGFS
#include "vcx_vcmd_dbgfs.h"
#endif
#include "venc_power_manager.h"
#include "mdr_pub.h"

#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>

#include "venc_power_common.h"

#define CREATE_TRACE_POINTS
#include "venc_trace.h"
#undef CREATE_TRACE_POINTS

MODULE_IMPORT_NS(DMA_BUF);
/****************************************************************
 * Macro definitions
 ***************************************************************/
//#define TIMEOUT_IRQ_TIMER

/*---------------------------------------------------------------
 * Macros related to sub-system config
 *---------------------------------------------------------------
 */
/* VCMD master_out_clk mode */
#define APB_CLK_ON	(1)
#define APB_CLK_OFF	(0)
#define APB_CLK_MODE	APB_CLK_OFF

/*---------------------------------------------------------------
 * Macros related to debug
 *---------------------------------------------------------------
 */

//#define VCMD_DEBUG_INTERNAL
//#define IRQ_SIMULATION

#define VCMDBUF_TOTAL_SIZE (2 * 1024 * 1024)
#define CMDBUF_POOL_TOTAL_SIZE    (1 * 1024 * 1024 - 8 * 1024)
#define CMDBUF_VCMD_REGISTER_TOTAL_SIZE   (16 * 1024)

/*---------------------------------------------------------------
 * Macros related to dev/process workload management
 *---------------------------------------------------------------
 */
#define VCMD_WORKLOAD_UNIT   (8192L * 4096L)
#define VCMD_INTR_INTERVAL   (VCMD_WORKLOAD_UNIT * 1)
#define PROCESS_MAX_WORKLOAD (VCMD_WORKLOAD_UNIT * 32)

#define THRESHOLD_US                (100000ULL)
#define VCMD_ABORT_THRESHOLD_US     (500000ULL)

#define VENC_MAX_RESET_COUNT        3
#define VENC_MAX_WDT_COUNT          1

/****************************************************************
 * external/global variables declarations
 ***************************************************************/
static vcmd_mgr_t *vcmd_manager;
extern unsigned long vcmd_isr_polling;
#ifdef CONFIG_ENC_PM_RUNTIME
extern u32 runtime_pm_enable;
#endif
#ifdef CONFIG_ENC_CM_RUNTIME
extern u32 runtime_cm_enable;
#endif
extern int vsi_dumplvl;
extern unsigned long ddr_offset;
extern u32 arbiter_weight;
extern u32 arbiter_urgent;
extern u32 arbiter_timewindow;
extern u32 arbiter_bw_overflow;
extern unsigned long sw_timeout_time;
extern u32 always_on;
extern struct platform_device *platformdev;

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
static void _vcmd_irq_bottom_half(struct work_struct *work);
void vcmd_start(struct hantrovcmd_dev *dev);
static void vcmd_start2(struct hantrovcmd_dev *dev, u32 reset);
static u32 dev_wait_job_num(struct hantrovcmd_dev *dev);

static int _vcmd_add_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);
static void _vcmd_del_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);
static void _vcmd_mod_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id);

extern int memmap_set_device(struct device *dev, int flag);

static struct device *platform_dev;
static struct miscdevice venc_misc;


#define REG_IOMEM_NAME "vpu_enc0"
#define IRQ_GET_NAME "venc_irq"
#define DRIVER_NAME "xring_venc"
#define XRING_VPU_ENC                            (1 << 0)
int vcmd_irq_flags;

static int venc_parse_dts(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct device_node *np = pdev->dev.of_node;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, REG_IOMEM_NAME);
	if (!res) {
		venc_klog(LOGLVL_ERROR, "failed to get memory resource !\n");
		return -EINVAL;
	}

	vcmd_core_array[0].vcmd_base_addr = (unsigned long)res->start;
	venc_klog(LOGLVL_INFO, "base addr is 0x%lx.\n", vcmd_core_array[0].vcmd_base_addr);

	vcmd_core_array[0].vcmd_irq = platform_get_irq_byname(pdev, IRQ_GET_NAME);
	if (vcmd_core_array[0].vcmd_irq < 0) {
		venc_klog(LOGLVL_ERROR, "venc vcmd_irq is %d !\n", vcmd_core_array[0].vcmd_irq);
		return -EINVAL;
	}

	vcmd_irq_flags = irqd_get_trigger_type(
				irq_get_irq_data(vcmd_core_array[0].vcmd_irq));

	if (of_property_read_u32(np, "io-size", &vcmd_core_array[0].submodule_cfg[SUB_MOD_VCMD].io_size)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "io-size");
		return -EINVAL;
	}
	vcmd_core_array[0].submodule_cfg[SUB_MOD_VCMD].io_size *= ASIC_VCMD_SWREG_AMOUNT;

	if (of_property_read_u32(np, "moduletype-property", &vcmd_core_array[0].sub_module_type)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "moduletype-property");
		return -EINVAL;
	}

	if (of_property_read_u16(np, "main-offset", &vcmd_core_array[0].submodule_cfg[SUB_MOD_MAIN].io_off)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "main-offset");
		return -EINVAL;
	}
	if (of_property_read_u16(np, "dec400-offset", &vcmd_core_array[0].submodule_cfg[SUB_MOD_DEC400].io_off)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "dec400-offset");
		return -EINVAL;
	}
	if (of_property_read_u16(np, "l2cache-offset",
		&vcmd_core_array[0].submodule_cfg[SUB_MOD_L2CACHE].io_off)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "l2cache-offset");
		return -EINVAL;
	}
	u16 sub_mmu_addr[2] = {0, 0};
	if (of_property_read_u16_array(np, "mmu-offset", &sub_mmu_addr, 2) == 0) {
		vcmd_core_array[0].submodule_cfg[SUB_MOD_MMU0].io_off = sub_mmu_addr[0];
		vcmd_core_array[0].submodule_cfg[SUB_MOD_MMU1].io_off = sub_mmu_addr[1];
	} else {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "mmu-offset");
		return -EINVAL;
	}
	u16 sub_axife_addr[2] = {0, 0};
	if (of_property_read_u16_array(np, "axife-offset", &sub_axife_addr, 2) == 0) {
		vcmd_core_array[0].submodule_cfg[SUB_MOD_AXIFE0].io_off = sub_axife_addr[0];
		vcmd_core_array[0].submodule_cfg[SUB_MOD_AXIFE1].io_off = sub_axife_addr[1];
	} else {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "axife-offset");
		return -EINVAL;
	}
	if (of_property_read_u16(np, "efbc-offset", &vcmd_core_array[0].submodule_cfg[SUB_MOD_UFBC].io_off)) {
		venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "efbc-offset");
		return -EINVAL;
	}
	// if (of_property_read_u16(np, "slice-offset", &vcmd_core_array[0].slice_addr)) {
	// 	venc_klog(LOGLVL_ERROR, "%s node doesn't have %s\n", np->name, "slice-offset");
	// 	return -EINVAL;
	// }
#ifdef HANTRO_DRIVER_DEBUG
	venc_klog(LOGLVL_INFO, "io-size: %d, moduletype-property: 0x%x, main-offset: 0x%x\n",
		vcmd_core_array[0].submodule_cfg[SUB_MOD_VCMD].vcmd_iosize,
		vcmd_core_array[0].sub_module_type,
		vcmd_core_array[0].submodule_cfg[SUB_MOD_MAIN].io_off);
	venc_klog(LOGLVL_INFO, "dec400-offset: 0x%x, l2cache-offset: 0x%x\n",
		vcmd_core_array[0].submodule_cfg[SUB_MOD_DEC400].io_off,
		vcmd_core_array[0].submodule_cfg[SUB_MOD_L2CACHE].io_off);
	venc_klog(LOGLVL_INFO, "mmu-offset[0]: 0x%x, mmu-offset[1]: 0x%x\n",
		vcmd_core_array[0].submodule_cfg[SUB_MOD_MMU0].io_off,
		vcmd_core_array[0].submodule_cfg[SUB_MOD_MMU1].io_off);
	venc_klog(LOGLVL_INFO, "axife-offset[0]: 0x%x, axife-offset[1]: 0x%x,\n",
		vcmd_core_array[0].submodule_cfg[SUB_MOD_AXIFE0].io_off,
		vcmd_core_array[0].submodule_cfg[SUB_MOD_AXIFE1].io_off);
	// venc_klog(LOGLVL_INFO, "efbc-offset: 0x%x, slice-offset: 0x%x\n",
	// 	vcmd_core_array[0].submodule_ufbc_addr, vcmd_core_array[0].slice_addr);
#endif
	return 0;
}

int venc_misc_register(void)
{
	return misc_register(&venc_misc);
}

void venc_misc_deregister(void)
{
	misc_deregister(&venc_misc);
}

int venc_qos_cfg(void)
{
	void __iomem *venc_axi_vaddr;
	struct vpu_qos_map *qos_priv = NULL;
	unsigned int qos_temp;
	int i = 0;

	qos_priv = (struct vpu_qos_map *)xring_flowctrl_get_cfg_table(MST_VPU_QOS_ID);
	if (qos_priv == NULL) {
		venc_klog(LOGLVL_ERROR, "venc qos_cfg_table get failed\n");
		return -EINVAL;
	}

	venc_axi_vaddr = ioremap(ACPU_AXIFE_VENC, ACPU_AXIFE_VENC_SIZE);
	if (venc_axi_vaddr == NULL) {
		venc_klog(LOGLVL_ERROR, "venc axi ioremap failed\n");
		return -EINVAL;
	}

	for (i = VENC_AXI_REM_RQOS_ID; i <= VENC_RDCH1_WQOS_ID; i++) {
		writel(qos_priv[i].val, venc_axi_vaddr + qos_priv[i].offset);
		if ((unsigned int)readl(venc_axi_vaddr + qos_priv[i].offset) != qos_priv[i].val) {
			venc_klog(LOGLVL_ERROR, "failed to write qos\n");
			iounmap((void __iomem *)venc_axi_vaddr);
			return -EINVAL;
		}
	}

	iounmap((void __iomem *)venc_axi_vaddr);
	venc_klog(LOGLVL_INFO, "success to configure venc qos\n");
	return 0;
}

int venc_pdev_init(struct platform_device *pdev, void **_vcmd_mgr)
{
	struct device *dev = &pdev->dev;
	dma_addr_t dma_handle = 0;
	int ret = 0;

	platform_dev = &pdev->dev;

	ret = venc_qos_cfg();
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "Failed to venc qos cfg\n");
		return ret;
	}

	ret = memmap_set_device(&pdev->dev, XRING_VPU_ENC);
	if (ret < 0) {
		venc_init_klog(LOGLVL_ERROR, "Failed to set devices\n");
		return ret;
	}

	ret = venc_parse_dts(pdev);
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "hantrovcmd: parse dts failed !\n");
		return ret;
	}

	of_dma_configure(dev, dev->of_node, true);

	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64));
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "64bit venc dev: No suitable DMA available\n");
		return ret;
	}

	ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(64));
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "64bit venc dev: No suitable DMA available\n");
		return ret;
	}

	ret = venc_driver_init(_vcmd_mgr);
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "venc driver init fail\n");
		return ret;
	}

	ret = venc_misc_register();
	if (ret) {
		venc_init_klog(LOGLVL_ERROR, "register venc device faild.\n");
		return ret;
	}

	venc_init_klog(LOGLVL_INFO, "hantroenc driver module inserted\n");

	return ret;
}

/****************************************************************
 * extern functions declarations
 ***************************************************************/
#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_RUNTIME
extern void hantroenc_pm_runtime_get(struct device *dev);
extern void hantroenc_pm_runtime_put(struct device *dev);
extern void hantroenc_pm_runtime_put_sync_autosuspend(struct device *dev);
#endif
#endif

/****************************************************************
 * watchdog, irq_simulation and debug functions
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
	if (dev->reset_count >= VENC_MAX_RESET_COUNT) {
		vcmd_klog(LOGLVL_WARNING, "[enc] system reset: wdt_trigger_count %d, reset_count %d!",
			dev->wdt_trigger_count, dev->reset_count);
		dev->reset_count = 0;
		mdr_system_error(MODID_VPU_ENC_HANG_EXCEPTION, 0, 0);
	} else {
		dev->reset_count++;
		vcmd_klog(LOGLVL_WARNING, "[enc] subsys reset: wdt_trigger_count %d, reset_count %d!",
			dev->wdt_trigger_count, dev->reset_count);
		mdr_system_error(modid, 0, 0);
		hantroenc_reset(platform_dev, true);
	}
}

#ifdef SUPPORT_WATCHDOG
/**
 * @brief hook function for system-driver to do further process
 *  for tiggered watchdog.
 */
static void hook_vcmd_watchdog(struct hantrovcmd_dev *dev, int succeed)
{
	if (succeed) {
		vcmd_klog(LOGLVL_WARNING, "axife flush and vcmd Abort succeed, wdt_trigger_count %d, reset_count %d!!",
			dev->wdt_trigger_count, dev->reset_count);
		/*
		 * The watchdog process exceeds the maximum number of consecutive
		 * successful processes and requires a subsystem reset
		 */
		if (dev->wdt_trigger_count >= VENC_MAX_WDT_COUNT) {
			dev->wdt_trigger_count = 0;
			hook_subsys_reset(dev, MODID_VPU_DEC_WDT_EXCEPTION);
		}
		dev->wdt_trigger_count++;
	} else {
		/*TODO*/
		vcmd_klog(LOGLVL_ERROR, "axife flush or vcmd abort failed, need to re-power subsys! reset_count %d\n",
				dev->reset_count);
		dev->state = VCMD_STATE_POWER_ON;
		if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
			dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
		else
			dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);

		// need to do sub-system reset
		hook_subsys_reset(dev, MODID_VPU_ENC_WDT_EXCEPTION);
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

	arb = vcmd_read_reg((const void *)dev->hwregs,
						VCMD_REGISTER_ARB_OFFSET);
	arb &= 0xfffffff8;
	vcmd_write_reg((const void *)dev->hwregs, VCMD_REGISTER_ARB_OFFSET, arb);

	do {
		if (dev->arb_reset_irq == 1) {
			dev->arb_reset_irq = 0;
			vcmd_klog(LOGLVL_DEBUG, "VCARB is hang, reset it!\n");
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

	vcmd_klog(LOGLVL_ERROR, "can't go to IDLE!\n");

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
	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0) {
		vcmd_set_reg_mirror(dev->reg_mirror, HWIF_VCMD_ABORT_MODE, 0x0);
	} else {
		dev->abort_mode = 0x1;
		vcmd_set_reg_mirror(dev->reg_mirror, HWIF_VCMD_ABORT_MODE, 0x1);
	}
	vcmd_write_register_value((const void *)dev->hwregs,
					dev->reg_mirror,
					HWIF_VCMD_START_TRIGGER, 0);
	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0)
		return 0;

	//wait vcmd core aborted and vcmd enters IDLE mode.
	if (watchdog_wait_vcmd_aborted(dev) == 0)
		return 0;

	return -1;
}

extern void watchdog_stop_vce(volatile u8 *reg_base);
/**
 * @brief process when watchdog triggered.
 */
static void _vcmd_watchdog_process(struct hantrovcmd_dev *dev)
{
	struct vcmd_subsys_info *subsys = dev->subsys_info;
	struct cmdbuf_obj *obj;
	vcmd_mgr_t *vcmd_mgr;
	int succeed = 1, ret = 0;
	unsigned long flags;

	vcmd_klog(LOGLVL_WARNING, "enc: trigger watchdog, stop vcmd, wait job num = %u, sw cmdbuf rdy num = %u\n",
		dev_wait_job_num(dev), dev->sw_cmdbuf_rdy_num);
	if (vsi_dumplvl >= DUMP_REGISTER_LEVEL_BRIEF_ONERROR)
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);
	else
		dump_brief_regs(dev, DUMP_VCMD_REG | DUMP_CORE_REG | DUMP_AXIFE_REG | DUMP_STATE_REG | DUMP_INTERRUPT_REG);

	if (CMDBUF_ID_CHECK(dev->aborted_cmdbuf_id) >= 0) {
		vcmd_mgr = (vcmd_mgr_t *)dev->handler;
		obj = &vcmd_mgr->objs[dev->aborted_cmdbuf_id];
		vcmd_klog(LOGLVL_WARNING, "enc: the cuurent obj workload is %u\n", obj->workload);
	}

	if (dev->hw_feature.vcarb_ver <= VCARB_VERSION_2_0) {
		spin_lock_irqsave(dev->spinlock, flags);
		ret = watchdog_stop_vcmd(dev);
		watchdog_stop_vce(subsys->hwregs[SUB_MOD_MAIN]);
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0) {
		mdelay(10); // wait 10ms
		/* if not own arbiter, it will receive arberr interrupt */
		if (dev->arb_err_irq) {
			dev->arb_err_irq = 0;
			return;
		}
		spin_lock_irqsave(dev->spinlock, flags);
		ret = watchdog_wait_vcmd_aborted(dev);
		if (ret == 0)
			watchdog_reset_vcmd_arbiter(dev);
		spin_unlock_irqrestore(dev->spinlock, flags);
	} else if (dev->hw_feature.vcarb_ver == VCARB_VERSION_3_0) {
		vcmd_start2(dev, 0);
		return;
	}
	if (ret < 0)
		succeed = 0;

	spin_lock_irqsave(dev->spinlock, flags);
#ifdef HANTROAXIFE_SUPPORT
	if (succeed)
		succeed = (AXIFEFlush(subsys->hwregs[SUB_MOD_AXIFE0]) != -1);
	if (succeed && subsys->reg_off[SUB_MOD_AXIFE1] != 0xffff)
		succeed = (AXIFEFlush(subsys->hwregs[SUB_MOD_AXIFE1]) != -1);
#else
	succeed = 0;
#endif
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
	struct hantrovcmd_dev *dev = (struct hantrovcmd_dev *)_dev;
	unsigned long flags;

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
	hook_subsys_reset(dev, MODID_VPU_ENC_EXTERNAL_TIMEOUT_EXCEPTION);
}

/**
 * @brief To check vcmd_mgr/dev's actions which need kthread to process
 * @return int: 0: no actions; 1: have actions
 */
static int _vcmd_kthread_actions(vcmd_mgr_t *vcmd_mgr,
			struct hantrovcmd_dev **dev)
{
	int i, j;

	if (vcmd_mgr->stop_kthread == 1)
		return 1;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		*dev = &vcmd_mgr->dev_ctx[i];
		for (j = 0; j < VCMD_TIMER_MAX; j++) {
			if ((*dev)->vcmd_timer[j].expired == 1)
				return 1;
		}

		if ((*dev)->ufbc_decoder_err ||
			(*dev)->segment_polling_timeout ||
			(*dev)->slice_to_send)
			return 1;
	}

	return 0;
}

#ifdef WFD_VPU_DEBUG
/**
 * @brief interface function of xplayer, delete when xplayer ready
 */
static void xplayer_slice_enc_done(struct dma_buf *dma_buf, unsigned long offset,
			unsigned long length, int slice_index, u64 timestamp)
{
	vcmd_klog(LOGLVL_INFO,
		"send slice: dma_buf = 0x%p, offset = %lu, slice idx = %u, slice len = %lu, timestmap = %llu\n",
		dma_buf, offset, slice_index, length, timestamp);
}
#else
extern void xplayer_slice_enc_done(struct dma_buf *dma_buf, unsigned long offset,
			unsigned long length, int slice_index, u64 timestamp);
#endif

/**
 * @brief send dma_buf of output buffer, slice offset, slice length, slice index to xplayer
 */
static void send_slice_msg_to_xplayer(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev)
{
	u16 cmdbuf_id = 0;
	struct cmdbuf_obj *obj;
	unsigned int ret = 0;
	u32 slice_idx = 0;
	u32 start_idx = 0;
	u32 end_idx = 0;
	unsigned long flags = 0;

	while(1) {
		ret = kfifo_out(&dev->send_slice_info_queue, &cmdbuf_id, sizeof(u16));
		if (cmdbuf_id >= SLOT_NUM_CMDBUF) {
			vcmd_klog(LOGLVL_ERROR, "invalid cmdbuf id\n");
			return;
		}
		if (ret != sizeof(u16))
			return;

		obj = &vcmd_mgr->objs[cmdbuf_id];
		if (obj->slice_info.slice_mode == 0) {
			vcmd_klog(LOGLVL_ERROR, "slice mode disable when slice_to_send triggered\n");
			return;
		}

		spin_lock_irqsave(dev->spinlock, flags);
		start_idx = obj->slice_info.slice_send_num;
		end_idx = obj->slice_info.slice_rdy_num;
		obj->slice_info.slice_send_num = obj->slice_info.slice_rdy_num;
		spin_unlock_irqrestore(dev->spinlock, flags);

		u32 offset = obj->slice_info.out_buf.offset;
		for (slice_idx = 0; slice_idx < end_idx; slice_idx++) {
			if (slice_idx >= start_idx && obj->frame_error == 0) {
				trace_send_slice_msg_to_xplayer("[enc] send slice: cmdbuf ", cmdbuf_id);
				vcmd_wfd_trace("send slice: cmdbuf[%d], slice[%u], timesamp %llu\n",
					cmdbuf_id, slice_idx, obj->timestamp);
				if (offset + obj->slice_info.slice_size[slice_idx] >
					obj->slice_info.slice_out_size) {
					vcmd_klog(LOGLVL_ERROR, "slice size out of range of output buffer\n");
					break;
				}
				if (slice_idx == 0 && offset > 0) {
#ifndef WFD_VPU_DEBUG
					vcmd_klog(LOGLVL_INFO,
							"send slice: dma_buf = 0x%p, offset = %d, slice idx = %d, slice len = %u\n",
							obj->slice_info.out_buf.dma_buf, 0, -1, offset);
#endif
					xplayer_slice_enc_done(obj->slice_info.out_buf.dma_buf,
						0, offset, -1, -1);
				}
#ifndef WFD_VPU_DEBUG
				vcmd_klog(LOGLVL_INFO,
						"send slice: dma_buf = 0x%p, offset = %u, slice idx = %u, slice len = %u\n",
						obj->slice_info.out_buf.dma_buf, offset, slice_idx, obj->slice_info.slice_size[slice_idx]);
#endif
				xplayer_slice_enc_done(obj->slice_info.out_buf.dma_buf, offset,
					obj->slice_info.slice_size[slice_idx],
					slice_idx, obj->timestamp);
			}
			offset += obj->slice_info.slice_size[slice_idx];
		}
	}
}

/**
 * @brief vcmd kernel thread main function
 */
static int _vcmd_kthread_fn(void *data)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)data;
	struct hantrovcmd_dev *dev = NULL;
	struct hantroenc_dev *enc_dev = NULL;
	int ret;

	while (!kthread_should_stop()) {
		if (wait_event_interruptible(vcmd_mgr->kthread_waitq,
				_vcmd_kthread_actions(vcmd_mgr, &dev))) {
			vcmd_klog(LOGLVL_ERROR, "wait_event_interruptible signaled!!!\n");
			return -ERESTARTSYS;
		}

		if (dev == NULL)
			continue;
		if (dev->slice_to_send == 1) {
			dev->slice_to_send = 0;
			send_slice_msg_to_xplayer(vcmd_mgr, dev);
		}

		enc_dev = dev_get_drvdata(platform_dev);
		if (!enc_dev) {
			vcmd_klog(LOGLVL_INFO, "enc_dev is null!\n");
			continue;
		}
		mutex_lock(&enc_dev->pm->pm_lock);
		ret = regulator_is_enabled(enc_dev->pm->venc_rg);
		if (ret <= 0) {
			vcmd_klog(LOGLVL_WARNING, "venc subsys has been powered off, ret %d,"
					"action trigger (%d:%d)\n",
					ret,
					dev->ufbc_decoder_err, dev->segment_polling_timeout);
			dev->ufbc_decoder_err = 0;
			dev->segment_polling_timeout = 0;
			mutex_unlock(&enc_dev->pm->pm_lock);
			continue;
		}

		if (dev->vcmd_timer[VCMD_TIMER_TIMEOUT].expired == 1) {
			dev->vcmd_timer[VCMD_TIMER_TIMEOUT].expired = 0;
			hook_vcmd_external_timeout(dev);
			mutex_unlock(&enc_dev->pm->pm_lock);
			continue;
		}

		if (dev->ufbc_decoder_err ||
			dev->segment_polling_timeout) {
			process_abnormal_err_irq(vcmd_mgr, dev);
			mutex_unlock(&enc_dev->pm->pm_lock);
			continue;
		}

#ifdef SUPPORT_WATCHDOG
		if (dev->vcmd_timer[VCMD_TIMER_WATCHDOG].expired == 1) {
			dev->vcmd_timer[VCMD_TIMER_WATCHDOG].expired = 0;
			_vcmd_watchdog_process(dev);
			mutex_unlock(&enc_dev->pm->pm_lock);
			continue;
		}
#endif
		mutex_unlock(&enc_dev->pm->pm_lock);
	}

	return 0;
}

/**
 * @brief wake up vcmd kernel thread
 */
void _vcmd_kthread_wakeup(vcmd_mgr_t *vcmd_mgr)
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
/**
 * @brief timer callback of vcmd timeout timer
 * @brief wait jobs count that need to be finished from dev work_list
 */
static u32 dev_wait_job_num(struct hantrovcmd_dev *dev)
{
	struct cmdbuf_obj *obj;
	bi_list_node *node = dev->work_list.head;
	u32 num = 0;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		if (obj->cmdbuf_run_done == 0) {
			num++;
			if (obj->has_jmp_cmd == 0 || obj->jmp_ie)
				return num;
		}
		node = node->next;
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
 * stop vcmd watchdog
 */
static void _vcmd_watchdog_stop(struct hantrovcmd_dev *dev)
{
	if (dev->watchdog_state == WD_STATE_ACTIVE)
		_vcmd_del_timer(dev, VCMD_TIMER_WATCHDOG);

	dev->watchdog_state = WD_STATE_STOPPED;
}

#if 0
/**
 * @brief pause vcmd watchdog
 */
void _vcmd_watchdog_pause(struct hantrovcmd_dev *dev)
{
	if (dev->watchdog_state == WD_STATE_ACTIVE) {
		_vcmd_del_timer(dev, VCMD_TIMER_WATCHDOG);
		dev->watchdog_state = WD_STATE_PAUSE;
	} else if (dev->watchdog_state == WD_STATE_STOPPED) {
		vcmd_klog(LOGLVL_WARNING, "%s: watch-dog pause, but is has been STOPPED!\n",
			__func__);
	} else {
		vcmd_klog(LOGLVL_ERROR, "%s: watch-dog pause at invalid state %d!\n",
			__func__, dev->watchdog_state);
	}
}
#endif
#endif //SUPPORT_WATCHDOG

/**
 * feed and start vcmd watchdog
 */
static void _vcmd_watchdog_feed(struct hantrovcmd_dev *dev, enum WD_FEED_MODE mode)
{
	u32 num = 0;
	struct vcmd_timer *tm = &dev->vcmd_timer[VCMD_TIMER_WATCHDOG];

	num = dev_wait_job_num(dev);
	if (num == 0) {
		_vcmd_watchdog_stop(dev);
		return;
	}

	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0)
		tm->period =VCMD_ARBITER_RESET_TIME;
	else if (dev->hw_feature.vcarb_ver == VCARB_VERSION_3_0)
		tm->period = sw_timeout_time;
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
			if (dev->watchdog_state == WD_STATE_ACTIVE ||
				dev->watchdog_state == WD_STATE_STOPPED) {
				vcmd_klog(LOGLVL_ERROR, "%s: when need to resume watch-dog, "
						"but state is ACTIVE or STOPPED!", __func__);
				return;
			}
			if (dev->watchdog_state == WD_STATE_PAUSE) {
				_vcmd_watchdog_start(dev);
				return;
			}
			break;
		}
		default: {
			vcmd_klog(LOGLVL_ERROR, "%s: watch-dog feed mode error!\n",
						__func__);
			return;
		}
	}
	_vcmd_mod_timer(dev, VCMD_TIMER_WATCHDOG);
}

/*******************************************************************
 *  vcmd timer related functions
 *******************************************************************/
/**
 * @brief timer callback function of vcmd timer
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
static int _vcmd_add_timer(struct hantrovcmd_dev *dev, enum vcmd_timer_id id)
{
	struct vcmd_timer *tm;
	struct timer_list *timer;

	if (id >= VCMD_TIMER_MAX) {
		vcmd_klog(LOGLVL_ERROR, "%s: use error id to add vcmd timer!\n", __func__);
		return -1;
	}

	tm = &dev->vcmd_timer[id];

	if (tm->period == 0) {
		vcmd_klog(LOGLVL_WARNING, "%s: period of timer %d is zero!\n",
				__func__, id);
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
		vcmd_klog(LOGLVL_ERROR, "%s: use error id to delete vcmd timer!\n",
				__func__);
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

		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s %s %d 0x%x]\n",
			offset, instr, "WREG", ((instr >> 26) & 0x1) ? "FIX" : "",
			   length, (instr & 0xFFFF));
		*size = ((length + 2) >> 1) << 1;
	} else if (opcode == OPCODE_END) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s]\n", offset,
			instr, "END");
		*size = 2;
	} else if (opcode == OPCODE_NOP) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s]\n", offset,
			instr, "NOP");
		*size = 2;
	} else if (opcode == OPCODE_RREG) {
		int length = ((instr >> 16) & 0x3FF);

		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s %s %d 0x%x]\n",
			offset, instr, "RREG", ((instr >> 26) & 0x1) ? "FIX" : "",
			   length, (instr & 0xFFFF));
		*size = 4;
	} else if (opcode == OPCODE_JMP) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s %s %s]\n",
			offset, instr, "JMP", ((instr >> 26) & 0x1) ? "RDY" : "",
			   ((instr >> 25) & 0x1) ? "IE" : "");
		*size = 4;
	} else if (opcode == OPCODE_STALL) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s %s 0x%x]\n",
			offset, instr, "STALL", ((instr >> 26) & 0x1) ? "IM" : "",
			   (instr & 0xFFFF));
		*size = 2;
	} else if (opcode == OPCODE_CLRINT) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s %u 0x%x]\n",
			offset, instr, "CLRINT", (instr >> 25) & 0x3,
			   (instr & 0xFFFF));
		*size = 2;
	} else if (opcode == OPCODE_M2M) {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s]\n",
			offset, instr, "M2M");
		*size = 6;
	} else {
		snprintf(str, 512, "current cmdbuf data %u = 0x%08x => [%s]\n",
			offset, instr, "UNKNOWN CMD");
		*size = 1;
	}
}

#ifdef USER_DEBUG
void dump_brief_regs(struct hantrovcmd_dev *dev, u32 dump)
{
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
	volatile void *vc9000e_regs = dev->subsys_info->hwregs[SUB_MOD_MAIN];
	if (vcmd_regs == NULL || vc9000e_regs == NULL)
		return;

	if (dump & DUMP_VCMD_REG) {
		for (i = 0; i < ASIC_VCMD_SWREG_AMOUNT; i++) {
			reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + i * 4));
			vcmd_klog(LOGLVL_WARNING, "vcmd swreg%d: 0x%x\n", i, reg_val);
		}
	}

	if (dump & DUMP_CORE_REG) {
		for (i = 0; i < ASIC_SWREG_AMOUNT; i++) {
			reg_val = (u32)ioread32((void __iomem *)(vc9000e_regs + i * 4));
			vcmd_klog(LOGLVL_WARNING, "vc9000e swreg%d: 0x%x\n", i, reg_val);
		}
	}

	if (dump & DUMP_AXIFE_REG) {
		if (dev->subsys_info->hwregs[SUB_MOD_AXIFE0]) {
			volatile void *axife_regs = dev->subsys_info->hwregs[SUB_MOD_AXIFE0];
			for (i = 0; i < 80; i++) { //AXIFE_REGISTER_SIZE too large, only 80 registers exist in our register.xml
				reg_val = (u32)ioread32((void __iomem *)(axife_regs + i * 4));
				vcmd_klog(LOGLVL_WARNING, "axife swreg%d: 0x%x\n", i, reg_val);
			}
		}
		if (dev->subsys_info->hwregs[SUB_MOD_AXIFE1]) {
			volatile void *axife_regs = dev->subsys_info->hwregs[SUB_MOD_AXIFE1];
			for (i = 0; i < 80; i++) { //AXIFE_REGISTER_SIZE too large, only 80 registers exist in our register.xml
				reg_val = (u32)ioread32((void __iomem *)(axife_regs + i * 4));
				vcmd_klog(LOGLVL_WARNING, "axife swreg%d: 0x%x\n", i, reg_val);
			}
		}
	}

	if (dump & DUMP_STATE_REG) {
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_WORK_STATE));
		switch (reg_val & 0x7) {
			case 0:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x (idle)\n", reg_val);
				break;
			case 1:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x (work)\n", reg_val);
				break;
			case 2:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x (stall)\n", reg_val);
				break;
			case 3:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x (pend)\n", reg_val);
				break;
			case 4:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x (abort)\n", reg_val);
				break;
			default:
				vcmd_klog(LOGLVL_WARNING, "vcmd state: 0x%x\n", reg_val);
				break;
		}
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_ABORT_STATE));
		vcmd_klog(LOGLVL_WARNING, "vcmd abort state: %u, start trigger: %u\n",
			(reg_val & 0x8), (reg_val & 0x1));
	}

	if (dump & DUMP_INTERRUPT_REG) {
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_SRC));
		vcmd_klog(LOGLVL_WARNING, "vcmd interrupt source: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_ENABLE));
		vcmd_klog(LOGLVL_WARNING, "vcmd interrupt enbale: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_TRIGGER));
		vcmd_klog(LOGLVL_WARNING, "vcmd interrupt trigger: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vcmd_regs + VCMD_INTERRUPT_GATE));
		vcmd_klog(LOGLVL_WARNING, "vcmd interrupt gate: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INTERRUPT));
		vcmd_klog(LOGLVL_WARNING, "vc9000e interrupt: 0x%x", reg_val);
		reg_val = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_POLLING_SLICEINFO));
		vcmd_klog(LOGLVL_WARNING, "vc9000e sliceinfo interrupt: 0x%x", reg_val);
	}

	/*for bus error of vc9000e*/
	if (dump & DUMP_BASE_INFO) {
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_OUTPUT_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_OUTPUT_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "output buffer iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_Y_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_Y_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "input y buffer iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_CB_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_CB_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "input cb buffer iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_CR_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_INPUT_CR_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "input cr buffer iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_SIZETBL_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_SIZETBL_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "sizeTbl buffer iova: 0x%llx", base);
		base_lsb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_SLICEINFO_BUFFER_LSB));
		base_msb = (u32)ioread32((void __iomem *)(vc9000e_regs + VC9000E_SLICEINFO_BUFFER_MSB));
		base = base_lsb + (base_msb << 32);
		vcmd_klog(LOGLVL_WARNING, "sliceinfo buffer iova: 0x%llx", base);
	}

	return;
}
#else
void dump_brief_regs(struct hantrovcmd_dev *dev, u32 dump) {}
#endif

#ifdef VCMD_DEBUG_INTERNAL
static void _dbg_log_last_cmd(struct cmdbuf_obj *obj)
{
	u32 *p, len, offset, size;
	char log_buf[512];

	vcmd_klog(LOGLVL_DEBUG, "last cmdbuf content:\n");
	if (obj->has_jmp_cmd) {
		p = (void *)_get_jmp_cmd(obj);
		len = sizeof(struct cmd_jmp_t) / sizeof(u32);
	} else {
		p = (void *)_get_end_cmd(obj);
		len = sizeof(struct cmd_end_t) / sizeof(u32);
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

static void _dbg_log_cmdbuf(struct cmdbuf_obj *obj)
{
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

static void _dbg_log_dev_regs(struct hantrovcmd_dev *dev, u32 dump)
{
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
	static unsigned int last_cmd;

	if ((cmd == HANTRO_IOCH_POLLING_CMDBUF) && (last_cmd == cmd))
		return;
	last_cmd = cmd;
	vcmd_klog(LOGLVL_DEBUG, "ioctl cmd 0x%08x\n", cmd);
}

/*======================= cmdbuf object management ================*/
/**
 * @brief initialize all of cmdbuf objs of vcmd driver handler.
 */
static void vcmd_init_objs(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;
	struct cmdbuf_obj *obj;
	struct noncache_mem *m0, *m1;

	m0 = &vcmd_mgr->mem_vcmd;
	m1 = &vcmd_mgr->mem_status;
	for (i = 0; i < SLOT_NUM_CMDBUF; i++) {
		obj = &vcmd_mgr->objs[i];
		obj->cmdbuf_id = i;
		obj->cmdbuf_size = SLOT_SIZE_CMDBUF;
		obj->cmd_va = m0->va + CMDBUF_OFF_32(i);
		obj->cmd_pa = m0->pa + CMDBUF_OFF(i);
		obj->mmu_cmd_ba = m0->mmu_ba + CMDBUF_OFF(i);

		obj->status_size = SLOT_SIZE_STATUSBUF;
		obj->status_va = m1->va + STATUSBUF_OFF_32(i);
		obj->status_pa = m1->pa + STATUSBUF_OFF(i);
		obj->mmu_status_ba = m1->mmu_ba + STATUSBUF_OFF(i);
	}
}

/**
 * @brief initialize all of cmdbuf nodes of vcmd driver handler.
 */
static void vcmd_init_nodes(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;

	for (i = 0; i < SLOT_NUM_CMDBUF; i++)
		vcmd_mgr->nodes[i].data = (void *)&vcmd_mgr->objs[i];
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
	obj->core_id = 0xFFFF;
	obj->cmdbuf_size = SLOT_SIZE_CMDBUF;
	obj->slice_info.slice_rdy_num = 0;
	obj->slice_info.slice_send_num = 0;
	obj->slice_info.frame_rdy = 0;
	obj->slice_info.slice_mode = 0;
	obj->slice_info.slice_out_size = 0;
	obj->slice_info.sizeTbl_buf.va = NULL;
	obj->frame_error = 0;
	obj->has_jmp_cmd = 1;
	obj->jmp_ie = 0;

	obj->ufbc_decoder_err = 0;
	obj->segment_polling_timeout = 0;

	obj->clk_rate = 0;
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
							struct si_linked_node *node)
{
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
static struct si_linked_node *_si_list_get(struct si_linked_list *list)
{
	struct si_linked_node *node = list->head;

	if (list->head) {
		list->head = node->next;
		if (list->head == NULL)
			list->tail = NULL;
	}

	return node;
}

/**
 * @brief init a si-list.
 */
static void vcmd_init_si_list(struct si_linked_list *list)
{
	spin_lock_init(&list->spinlock);
	list->head = NULL;
	list->tail = NULL;
}

/**
 * @brief init free obj (cmdbuf) list of vcmd driver handler.
 */
static void vcmd_init_free_obj_list(vcmd_mgr_t *vcmd_mgr)
{
	u32 i;
	struct si_linked_list *list = &vcmd_mgr->free_obj_list;

	vcmd_init_si_list(list);
	sema_init(&vcmd_mgr->free_obj_sema, SLOT_NUM_CMDBUF);

	for (i = 0; i < SLOT_NUM_CMDBUF; i++) {
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
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
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
static struct cmd_jmp_t *_get_jmp_cmd(struct cmdbuf_obj *obj)
{
	u8 *p = (u8 *)obj->cmd_va;

	p += obj->cmdbuf_size - sizeof(struct cmd_jmp_t);
	return (struct cmd_jmp_t *)p;
}

/**
 * @brief get the va of vcmdbuf obj's END cmd.
 */
static struct cmd_end_t *_get_end_cmd(struct cmdbuf_obj *obj)
{
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
static void _set_rreg_addr(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
	addr_t reg_mem_ba;
	struct cmd_rreg_t *cmd_rreg;
	u8 *p;
	u16 jmp_len = 0;

	if (dev->hw_version_id <= HW_ID_1_0_C)
		return;

	reg_mem_ba = dev->reg_mem_ba;
	if (dev->mmu_enable)
		reg_mem_ba = (addr_t)dev->mmu_reg_mem_ba;

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
	/* fuzz: check heap overflow */
	if ((void *)cmd_rreg < (void *)obj->cmd_va)
		vcmd_klog(LOGLVL_ERROR, "obj->cmd_va=%#llx cmd_rreg=%#llx\n", (u64)obj->cmd_va,
			(u64)cmd_rreg);

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
static void _update_jmp_ie(struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj)
{
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
								bi_list_node *next_node)
{
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
			if (dev->mmu_enable)
				next_ba = (addr_t)next_obj->mmu_cmd_ba;

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
static struct proc_obj *create_process_object(void)
{
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
static void free_process_object(struct proc_obj *po)
{
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "po is NULL!\n");
		return;
	}
	vfree(po);
}

/**
 * @brief add done obj to job_done_list of its po, wake-up job_waitq if needed.
 */
static void proc_add_done_job(vcmd_mgr_t *vcmd_mgr, struct cmdbuf_obj *obj)
{
	u16 id = obj->cmdbuf_id;
	struct proc_obj *po;
	struct bi_list *list;
	u32 is_empty, is_wait;

	if (!obj->po) {
		vcmd_klog(LOGLVL_INFO, "%s: the po and cmdbufs of this po has been released!\n",
						__func__);
		return;
	}

	po = obj->po;
	list = &po->job_done_list;

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

	if (is_empty || is_wait)
		wake_up_interruptible_all(&po->job_waitq);
}

/**
 * @brief get & remove a done obj from po's job_done_list.
 * @param struct cmdbuf_obj **pobj: *pobj==NULL: get head node from list.
 *									otherwise, get specified node from list.
 * @return int: 0: no done obj; 1: obj is done.
 */
static int proc_get_done_job(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
							struct cmdbuf_obj **pobj)
{

	struct bi_list *list = &po->job_done_list;
	bi_list_node *node;
	struct cmdbuf_obj *obj = NULL;
	int is_done = 0;

	spin_lock(&po->job_lock);
	node = list->head;

	if (node == NULL) {
		// job done list is empty
		po->in_wait = 1;
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
		if (obj->cmdbuf_run_done || obj->slice_run_done) {
			int loglvl = obj->slice_run_done ? LOGLVL_DEBUG : LOGLVL_INFO;
			// check if the obj is in job done list
			while (node && node->data != (void *)obj)
				node = node->next;

			if (node == NULL) {
				vcmd_klog(loglvl, "cmdbuf[%d] is done, but not in done-list!\n",
						obj->cmdbuf_id);
			} else {
				vcmd_klog(loglvl, "cmdbuf[%d] is done!\n", obj->cmdbuf_id);
				is_done = 1;
			}
				obj->slice_run_done = 0;
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
							bi_list_node *node)
{
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;
	unsigned long flags;

	if (obj->cmdbuf_linked == 0) {
		//already de-linked or not link into work list yet
		return 1;
	}
	spin_lock_irqsave(dev->spinlock, flags);
	if ( dev->state != VCMD_STATE_WORKING ||
		obj->has_jmp_cmd == 0 || node->next) {
		spin_unlock_irqrestore(dev->spinlock, flags);
		//delink cmdbuf, and remove node from work list.
		dev_link_cmdbuf(dev, node->prev, node->next);
		bi_list_remove_node(&dev->work_list, node);
		obj->cmdbuf_linked = 0;

#ifdef CONFIG_ENC_CM_RUNTIME
		if (runtime_cm_enable && obj->clk_rate) {
			struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
			if(!enc_dev){
				venc_cm_klog(LOGLVL_ERROR, "enc_dev is NULL!\n");
				return 1;
			}
			venc_clk_mgr *cm = enc_dev->cm;
			if(!cm){
				venc_cm_klog(LOGLVL_ERROR, "venc_clk_mgr is NULL!\n");
				return 1;
			}
			venc_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_POP);
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
 * @brief add a job to tail of device work_list,
 *  and link to cmdbuf jmp list if needed.
 */
static int dev_add_job(struct hantrovcmd_dev *dev, bi_list_node *job_node)
{

	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)job_node->data;
	_set_cmds(dev, obj);

	bi_list_insert_node_tail(&dev->work_list, job_node);
	_update_jmp_ie(dev, obj);

#ifdef SUPPORT_DBGFS
	_dbgfs_record_active_start_time(dev->dbgfs_info);
#endif

	dev_link_cmdbuf(dev, job_node->prev, job_node);
	obj->core_id = dev->core_id;
	obj->cmdbuf_linked = 1;

#ifdef CONFIG_ENC_CM_RUNTIME
	if (runtime_cm_enable && obj->clk_rate) {
		struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
		if(!enc_dev){
			venc_cm_klog(LOGLVL_ERROR, "enc_dev is NULL!\n");
			return 0;
		}
		venc_clk_mgr *cm = enc_dev->cm;
		if(!cm){
			venc_cm_klog(LOGLVL_ERROR, "venc_clk_mgr is NULL!\n");
			return 0;
		}
		venc_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_PUSH);
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
							bi_list_node *job_node)
{
	struct cmdbuf_obj *obj;

	obj = (struct cmdbuf_obj *)job_node->data;
	_set_cmds(dev, obj);

	bi_list_insert_node_before(&dev->work_list, base_node, job_node);

#ifdef SUPPORT_DBGFS
	_dbgfs_record_active_start_time(dev->dbgfs_info);
#endif

	dev_link_cmdbuf(dev, job_node->prev, job_node);
	dev_link_cmdbuf(dev, job_node, job_node->next);
	obj->core_id = dev->core_id;
	obj->cmdbuf_linked = 1;
#ifdef CONFIG_ENC_CM_RUNTIME
	if (runtime_cm_enable && obj->clk_rate) {
		struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
		if(!enc_dev){
			venc_cm_klog(LOGLVL_ERROR, "enc_dev is NULL!\n");
			return 0;
		}
		venc_clk_mgr *cm = enc_dev->cm;
		if(!cm){
			venc_cm_klog(LOGLVL_ERROR, "venc_clk_mgr is NULL!\n");
			return 0;
		}
		venc_clk_mgr_update(cm, obj->clk_rate, CLK_MGR_PUSH);
	}
#endif
	return 0;
}

/**
 * @brief remove a job from dev work_list,
 *  de-link it from cmdbuf jmp list if needed, and return the cmdbuf.
 */
static int dev_remove_job(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
						bi_list_node *node)
{
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

	if (dev_delink_job(dev, node))
		return_cmdbuf(vcmd_mgr, obj->cmdbuf_id);

	return 0;
}

/**
 * @brief get remain (un-do) jobs count from dev work_list,
 */
static u32 dev_get_job_num(struct hantrovcmd_dev *dev)
{
	struct cmdbuf_obj *obj;
	bi_list_node *node = dev->work_list.head;
	u32 num = 0;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		if (obj->cmdbuf_run_done == 0)
			num++;
		node = node->next;
	}
	return num;
}

/**
 * @brief calculate workload after specified node in dev work_list.
 */
static u32 calc_workload_after_node(bi_list_node *node)
{
	u32 sum = 0;
	struct cmdbuf_obj *obj;

	while (node) {
		obj = (struct cmdbuf_obj *)node->data;
		sum += obj->workload;
		node = node->next;
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

struct sub_ip_init_cfg {
	u32 reg_id;
	u32 reg_val;
};

#ifdef HANTROAXIFE_SUPPORT
struct sub_ip_init_cfg axife_init_cfg[] = {
	{AXI_REG10_SW_FRONTEND_EN, 0x02},
	{AXI_REG11_SW_WORK_MODE, 0x00},
	{0xffff, }	//end guard
};
#endif

#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
/**
 * @brief set reg id/val of specified sub-ip to init-cmd regs.
 */
static void _set_module_init_cmds(struct hantrovcmd_dev *dev, u32 module_id,
									struct sub_ip_init_cfg *module_cfg)
{
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

#ifdef HANTROAXIFE_SUPPORT
	//enable AXIFE by VCMD
	_set_module_init_cmds(dev, SUB_MOD_AXIFE0, axife_init_cfg);
	_set_module_init_cmds(dev, SUB_MOD_AXIFE1, axife_init_cfg);
#endif

	//finished with END command
	dev->reg_mirror[dev->init_cmd_idx++] = OPCODE_END;
	dev->reg_mirror[dev->init_cmd_idx++] = 0x00;

	for (i = VCMD_REG_ID_SW_INIT_CMD0; i < dev->init_cmd_idx; i++)
		vcmd_write_reg((const void *)dev->hwregs, i*4, dev->reg_mirror[i]);
#endif
}

/**
 * @brief reset all vcmd hw devices.
 */
void vcmd_reset_asic(vcmd_mgr_t *vcmd_mgr)
{
	int n;
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
			// gate external interrupts
			vcmd_write_reg((const void *)dev[n].hwregs,
					   VCMD_REGISTER_EXT_INT_GATE_OFFSET,
					   dev[n].intr_gate_mask);

			if (dev[n].hw_feature.vcarb_ver == VCARB_VERSION_2_0) {
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

#ifdef VCARB_REQUEST
/**
 * @brief vcmd get online for arbiter
 */
static void vcmd_online_for_arbiter(const volatile u8 *hwregs)
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
static void vcmd_offline_for_arbiter(const volatile u8 *hwregs)
{
	// vcmd offline
	iowrite32(0, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARBITER_CONFIG_OFFSET));
}

/**
 * @brief vcmd request arbiter manully
 */
static void vcmd_request_arbiter(void *_vcmd_mgr, u32 subsys_id)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	struct hantrovcmd_dev *dev = &vcmd_mgr->dev_ctx[subsys_id];
	volatile u8 *hwregs = dev->hwregs;

	if (dev->hw_feature.vcarb_ver == 0)
		return;
	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0)
		vcmd_online_for_arbiter(hwregs);

	iowrite32(0x0, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
	iowrite32(0x1, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
	while ((ioread32((void __iomem *)(hwregs +
		VCMD_REGISTER_ARB_OFFSET)) & 0x4) == 0)
		schedule();
}

/**
 * @brief release arbiter from keep serving vcmd
 */
static void vcmd_release_arbiter(void *_vcmd_mgr, u32 subsys_id)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)_vcmd_mgr;
	struct hantrovcmd_dev *dev = &vcmd_mgr->dev_ctx[subsys_id];
	volatile u8 *hwregs = dev->hwregs;

	if (dev->hw_feature.vcarb_ver == 0)
		return;
	if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0)
		vcmd_offline_for_arbiter(hwregs);

	// write arb_fe to clear arb_req
	iowrite32(0x2, (void __iomem *)(hwregs +
			VCMD_REGISTER_ARB_OFFSET));
}
#endif //VCARB_REQUEST

/**
 * @brief start a vcmd hw device to run the 1st not-done job in its work list.
 */
void vcmd_start(struct hantrovcmd_dev *dev)
{
	struct cmdbuf_obj *obj = NULL;
	const void *hwregs = (const void *)dev->hwregs;
	bi_list_node *node;
	u32 *reg_mirror = dev->reg_mirror;
	addr_t cmd_ba;
	u32 ba_msb = 0;

	WARN_ON(dev->state == VCMD_STATE_POWER_OFF);
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
		if (dev->hw_feature.vcarb_ver) {
			if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0)
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
	if (dev->mmu_enable)
		cmd_ba = (addr_t)obj->mmu_cmd_ba;
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
			vcmd_klog(LOGLVL_ERROR, "%s no node bind to executing cmd ba 0x%lx!!\n",
											__func__, exe_cmdbuf_ba);
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
 * @brief abort a specified vcmd hw device.
 * @param u32 *aborted_id: the id of aborted cmdbuf.
 * @param u32 vcmd_isr_polling: the mode to wait for device being aborted.
 */
int vcmd_abort(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
					u32 *aborted_id)
{
	u32 cnt = 100000, irq;
	unsigned long flags;

	spin_lock_irqsave(dev->spinlock, flags);
#ifdef SUPPORT_WATCHDOG
	_vcmd_watchdog_stop(dev);
#endif
	vcmd_write_register_value((const void *)dev->hwregs,
								dev->reg_mirror,
								HWIF_VCMD_START_TRIGGER, 0);
	spin_unlock_irqrestore(dev->spinlock, flags);
	if (vcmd_isr_polling == 0) {
		if (!wait_event_timeout(*dev->abort_waitq,
						(dev->state == VCMD_STATE_IDLE) ||
						(dev->state == VCMD_STATE_POWER_OFF),
						msecs_to_jiffies(500))) {
			vcmd_klog(LOGLVL_ERROR, "[enc] abort_waitq timeout.\n");
			return -ERESTARTSYS;
		}
	}

	if (aborted_id)
		*aborted_id = dev->aborted_cmdbuf_id;

	vcmd_klog(LOGLVL_INFO, "%s: vcmd aborted cmdbuf[%u].\n", __func__, dev->aborted_cmdbuf_id);

	return 0;

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
	u32 least_workload;
	u32 reg_id_exe;

	u32 cmdbuf_id, i;
	addr_t curr_exe_addr;
	int ret;

	obj = (struct cmdbuf_obj *)new_node->data;
	module = &vcmd_mgr->module_mgr[obj->module_type];

	/* To check if there is free dev, or dev which tail node is run-done. */
	for (i = 0; i < module->num; i++) {
		dev = module->dev[i];
		ret = is_supported_core(obj->core_mask, dev->id_in_type);
		if (ret == 0)
			continue;

		list = &dev->work_list;
		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			return -EINTR;
		}
		if (!list->tail ||
			((struct cmdbuf_obj *)list->tail->data)->cmdbuf_run_done) {
			dev_add_job(dev, new_node);
			up(&dev->work_sem);
			return 0;
		}
		up(&dev->work_sem);
	}

	// There is no vcmd in free, calculate each workload, select the least one.
	// If low priority, insert to tail.
	// If high priority, abort the dev, and insert to "head".
	reg_id_exe = REG_ID_CMDBUF_EXE_ID;
	if (obj->priority == CMDBUF_PRIORITY_NORMAL)
		reg_id_exe = REG_ID2_CMDBUF_EXE_ID;

	least_workload = 0xffffffff;
	smallest_dev = NULL;

	//calculate remain workload of all dev, find the least one
	for (i = 0; i < module->num; i++) {
		dev = module->dev[i];
		ret = is_supported_core(obj->core_mask, dev->id_in_type);
		if (ret == 0)
			continue;

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
			if (CMDBUF_ID_CHECK(cmdbuf_id) < 0)
				return -1;

			curr_node = &vcmd_mgr->nodes[cmdbuf_id];
		}

		if (down_killable(&dev->work_sem)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
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
		dev_add_job(smallest_dev, new_node);
		up(&smallest_dev->work_sem);
		return 0;
	}

	//CMDBUF_PRIORITY_HIGH
	//abort the vcmd and wait
	if (vcmd_abort(vcmd_mgr, smallest_dev, &cmdbuf_id))
		return -ERESTARTSYS; //abort failed

	// need to select inserting position again
	// because hw maybe have run to the next node.
	// CMDBUF_PRIORITY_HIGH
	if (down_killable(&smallest_dev->work_sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		return -EINTR;
	}
	curr_node = &vcmd_mgr->nodes[cmdbuf_id];
	if (smallest_dev->abort_mode == 0)
		curr_node = curr_node->next;
	while (curr_node) {
		tmp_obj = (struct cmdbuf_obj *)curr_node->data;
		//find the 1st node with normal priority, and insert node prior to it
		if (tmp_obj->priority == CMDBUF_PRIORITY_NORMAL)
			break;
		curr_node = curr_node->next;
	}

	//insert to "head" of normal priority nodes
	dev_insert_job(smallest_dev, curr_node, new_node);
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

static int acquire_process_resource(struct proc_obj *po, u32 workload)
{
	spin_lock(&po->spinlock);
	po->total_workload += workload;
	spin_unlock(&po->spinlock);
	if (wait_event_interruptible(po->resource_waitq,
									wait_process_resource_rdy(po))) {
		vcmd_klog(LOGLVL_ERROR, "%s: wait event is interrupted!", __func__);
		return -1;
	}

	return 0;
}

/**
 * @brief return cmdbuf obj's workload to process object.
 */
static void return_process_resource(struct proc_obj *po,
									struct cmdbuf_obj *obj)
{
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
	obj->interrupt_ctrl = param->interrupt_ctrl;
	obj->filp = po->filp;
	obj->po = po;
	obj->core_mask = param->core_mask;
	obj->clk_rate = param->frequency;
	obj->timestamp = param->timestamp;

	param->cmdbuf_size = SLOT_SIZE_CMDBUF;
	param->cmdbuf_id = cmdbuf_id;
	vcmd_klog(LOGLVL_DEBUG, "filp[%p] reserved cmdbuf[%d]: obj %p, node %p\n",
			(void *)obj->filp, cmdbuf_id, (void *)obj,
			(void *)&vcmd_mgr->nodes[cmdbuf_id]);

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
	if (obj->slice_info.slice_mode && obj->slice_info.sizeTbl_buf.va != NULL) {
		dma_buf_vunmap(obj->slice_info.sizeTbl_buf.dma_buf, &obj->slice_info.sizeTbl_buf.map);
		obj->slice_info.sizeTbl_buf.va = NULL;
	}

	return 0;
}

static int process_slice_buffer_info(struct cmdbuf_obj *obj)
{
	int ret;
	int i;
	unsigned long iova_lsb = 0;
	unsigned long iova_msb = 0;

	/*Get dma_buf of output offer by iova in cmd_buf*/
	iova_lsb = *(obj->cmd_va + OUTPUT_BUFFER_BASE_LSB);
	iova_msb = *(obj->cmd_va + OUTPUT_BUFFER_BASE_MSB);
	obj->slice_info.out_buf.iova = iova_lsb + (iova_msb << 32);
	ret = get_dma_buf_by_iova(obj->slice_info.out_buf.iova, &obj->slice_info.out_buf.offset,
		&obj->slice_info.out_buf.dma_buf);
	if (ret) {
		vcmd_klog(LOGLVL_ERROR, "fail to get sg_table of output buffer!!\n");
		goto err_wfd;
	}
	obj->slice_info.slice_out_size = obj->slice_info.out_buf.dma_buf->size;
	vcmd_klog(LOGLVL_DEBUG, "out buf of cmdbuf[%d]: iova = 0x%lx, offset = 0x%lx, dma_buf = 0x%p, output size = %zu\n",
		obj->cmdbuf_id, obj->slice_info.out_buf.iova, obj->slice_info.out_buf.offset,
		obj->slice_info.out_buf.dma_buf, obj->slice_info.slice_out_size);

	/*Map virtual address of sizeTbl offer by iova in cmd_buf*/
	iova_lsb = *(obj->cmd_va + SIZETBL_BUFFER_BASE_LSB);
	iova_msb = *(obj->cmd_va + SIZETBL_BUFFER_BASE_MSB);
	obj->slice_info.sizeTbl_buf.iova = iova_lsb + (iova_msb << 32);
	ret = get_dma_buf_by_iova(obj->slice_info.sizeTbl_buf.iova, &obj->slice_info.sizeTbl_buf.offset,
		&obj->slice_info.sizeTbl_buf.dma_buf);
	if (ret) {
		vcmd_klog(LOGLVL_ERROR, "fail to get sg_table of sizeTbl buffer!!\n");
		goto err_wfd;
	}
	if (dma_buf_vmap(obj->slice_info.sizeTbl_buf.dma_buf, &obj->slice_info.sizeTbl_buf.map)) {
		vcmd_klog(LOGLVL_ERROR, "fail to map sizeTbl buffer!!\n");
		goto err_wfd;
	}
	obj->slice_info.sizeTbl_buf.va = (u32 *)(obj->slice_info.sizeTbl_buf.map.vaddr + obj->slice_info.sizeTbl_buf.offset);
	vcmd_klog(LOGLVL_DEBUG, "sizeTbl buf of cmdbuf[%d]: iova = 0x%lx, offset = 0x%lx, dma_buf = 0x%p, va = 0x%p\n",
		obj->cmdbuf_id, obj->slice_info.sizeTbl_buf.iova, obj->slice_info.sizeTbl_buf.offset,
		obj->slice_info.sizeTbl_buf.dma_buf, obj->slice_info.sizeTbl_buf.va);
	for (i = 0; i < MAX_SLICE_NUM; ++i) {
		*(obj->slice_info.sizeTbl_buf.va + i) = 0;
	}

	return 0;
err_wfd:
	obj->slice_info.out_buf.dma_buf = NULL;
	obj->slice_info.sizeTbl_buf.dma_buf = NULL;
	obj->slice_info.sizeTbl_buf.va = NULL;
	obj->slice_info.slice_mode = 0;
	obj->slice_info.slice_out_size = 0;
	return -1;
}

/**
 * @brief add/insert a cmdbuf (job) to suitable device,
 *  and start the device hw if it is not working.
 * @param struct exchange_parameter *param: the param of cmdbuf to link & run.
 * @return long: 0: succeed; oters: failed.
 */
static long link_and_run_cmdbuf(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
								struct exchange_parameter *param)
{
	struct cmdbuf_obj *obj;
	bi_list_node *curr_node;
	unsigned long flags;
	u32 module_type;

	struct hantrovcmd_dev *dev = NULL;
	int ret;
	u16 cmdbuf_id = param->cmdbuf_id;
	u16 batchcount = ((param->interrupt_ctrl >> 32) & 0xff);

	struct cmd_jmp_t *cmd_jmp;

	if (CMDBUF_ID_CHECK(cmdbuf_id) < 0) {
		//should not happen
		vcmd_klog(LOGLVL_ERROR, "cmdbuf_id(%u) is not available!!\n", cmdbuf_id);
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

	obj->cmdbuf_size = param->cmdbuf_size;
	obj->interrupt_ctrl = param->interrupt_ctrl;

	u32 sliceinfo_polling_config = *(obj->cmd_va + SLICE_INFO_POLL_CFG);
	//enable slice polling timeout -- bit 3 of swreg 118 of vc9000e
	obj->slice_info.slice_mode = param->slice_en;
	if (obj->slice_info.slice_mode) {
		obj->slice_info.slice_num = param->slice_num;
		process_slice_buffer_info(obj);
	}

#ifdef VCMD_DEBUG_INTERNAL
	_dbg_log_cmdbuf(obj);
#endif

	//0: has jmp opcode,1 has end code
	obj->has_jmp_cmd = param->has_end_cmd ? 0 : 1;

	if (obj->has_jmp_cmd) {
		//last command is JMP, get its IE value.
		/* fuzz report */
		if (obj->cmdbuf_size < sizeof(struct cmd_jmp_t)) {
			vcmd_klog(LOGLVL_ERROR, "cmdbuf size is not enough for JMP cmd !!\n");
			return -EINVAL;
		}
		cmd_jmp = _get_jmp_cmd(obj);
		vcmd_klog(LOGLVL_DEBUG, "has jmp cmd and the last cmd is JMP!\n");

		if ((cmd_jmp->opcode & OPCODE_MASK) != OPCODE_JMP) {
			vcmd_klog(LOGLVL_ERROR, "cmdbuf[%d] is not terminated by JMP, not match with its flag!",
					obj->cmdbuf_id);
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

	dev = &vcmd_mgr->dev_ctx[obj->core_id];
	param->core_id = obj->core_id;
	vcmd_klog(LOGLVL_INFO, "Assign cmdbuf[%d] to core[%d]\n",
			  cmdbuf_id, param->core_id);

#ifdef CONFIG_ENC_CM_RUNTIME
	if(!runtime_cm_enable || !obj->clk_rate){
		venc_cm_klog(LOGLVL_INFO,
					"runtime_cm disabled or cur cmdbuf do not need change clk rate!\n");
		goto next;
	}
	//set frequency
	if(!platformdev){
		venc_cm_klog(LOGLVL_ERROR, "platformdev is NULL!\n");
		goto next;
	}
	struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
	if(!enc_dev){
		venc_cm_klog(LOGLVL_ERROR, "enc_dev is NULL!\n");
		goto next;
	}
	venc_power_mgr *pm = enc_dev->pm;
	venc_clk_mgr *cm = enc_dev->cm;
	if(!cm || !pm){
		venc_cm_klog(LOGLVL_ERROR, "venc_power_mgr or venc_clk_mgr is NULL!\n");
		goto next;
	}
	spin_lock_irqsave(dev->spinlock, flags);
	u64 desire_rate = venc_clk_mgr_get_rate(cm);
	spin_unlock_irqrestore(dev->spinlock, flags);
	if (desire_rate) {
		mutex_lock(&pm->pm_lock);
		venc_clk_rate_config(pm, desire_rate);
		mutex_unlock(&pm->pm_lock);
	}
next:
#endif

	/* fuzz report */
	if (!dev->work_sem.wait_list.prev && !dev->work_sem.wait_list.next) {
		vcmd_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		up(&vcmd_mgr->module_mgr[module_type].sem);
		return -EINVAL;
	}
	//start to run
	if (down_killable(&dev->work_sem)) {
		vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
		up(&vcmd_mgr->module_mgr[module_type].sem);
		return -EINTR;
	}

	trace_link_and_run_cmdbuf("[enc] start frame: cmdbuf ", cmdbuf_id);
	if (obj->slice_info.slice_mode)
		vcmd_wfd_trace("start frame: cmdbuf[%d]\n", cmdbuf_id);
	spin_lock_irqsave(dev->spinlock, flags);
	if (dev->state != VCMD_STATE_WORKING) {
		//start vcmd
		/* if occured ufbc_decoder_err or segment_polling_timeout,
		 * vcmd start after cmdbuf changed in kthread, but not here.
		 */
		if (dev->ufbc_decoder_err == 0 &&
			dev->segment_polling_timeout == 0)
			vcmd_start(dev);
	} else {
		dev->sw_cmdbuf_rdy_num++;
		if ((batchcount > 0 && obj->jmp_ie == 1 && vcmd_mgr->module_mgr[VCMD_TYPE_ENCODER].num == 1) ||
			batchcount == 0 || vcmd_mgr->module_mgr[VCMD_TYPE_ENCODER].num > 1) {
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
	up(&vcmd_mgr->module_mgr[module_type].sem);

	return 0;
}

/**
 * @brief wait a cmdbuf runs done.
 * @param u16 cmdbuf_id: the id of cmdbuf to wait.
 * @param u16 *done_id: point to the id of done cmdbuf.
 * @return long: 0: succeed; oters: failed.
 */
static long wait_cmdbuf_ready(vcmd_mgr_t *vcmd_mgr, struct proc_obj *po,
								u16 cmdbuf_id, u16 *done_id)
{
	struct cmdbuf_obj *obj = NULL;
	unsigned long timeout_jiff;
	int ret;

	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "not find process obj!\n");
		return -ERESTARTSYS;
	}
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

	trace_wait_cmdbuf_ready("[enc] wait cmdbuf ready: cmdbuf ", cmdbuf_id);
	if (obj != NULL && obj->slice_info.slice_mode)
		vcmd_wfd_trace("wait cmdbuf ready: cmdbuf[%d]\n", cmdbuf_id);
	timeout_jiff = msecs_to_jiffies(1000);
	ret = wait_event_interruptible_timeout(po->job_waitq,
						proc_get_done_job(vcmd_mgr, po, &obj), timeout_jiff);
	if (!ret) {
		vcmd_klog(LOGLVL_ERROR, "vcmd_wait_queue_0 timeout\n");
		return -EFAULT;
	} else if (-ERESTARTSYS == ret) {
		vcmd_klog(LOGLVL_ERROR, "vcmd_wait_queue_0 interrupted\n");
		return -ERESTARTSYS;
	}

	*done_id = obj->cmdbuf_id;
	if (obj->cmdbuf_run_done == 1)
		return 0;
	else
		return 1;
}

int _vcmd_alloc_dma_mem(void)
{
	phys_addr_t phys_addr = 0;
	dma_addr_t dma_addr = 0;
	u32 *cpu_addr = NULL;

	cpu_addr = dma_alloc_attrs(
		platform_dev, VCMDBUF_TOTAL_SIZE, &phys_addr,
		GFP_KERNEL, DMA_ATTR_FORCE_CONTIGUOUS);

	if (!cpu_addr || !phys_addr) {
		venc_klog(LOGLVL_ERROR, "venc dma_alloc_coherent memory failed\n");
		return -ENOMEM;
	}
	vcmd_klog(LOGLVL_INFO,
		"venc alloc success, cpu_addr = 0x%llx, phys_addr = 0x%llx, size 0x%x\n",
		(unsigned long long)cpu_addr, (unsigned long long)phys_addr, VCMDBUF_TOTAL_SIZE);

#ifdef BYPASS_SMMU_TEST
	dma_addr = phys_addr;
#else
	dma_addr = dma_map_page(platform_dev, phys_to_page(phys_addr), 0,
				VCMDBUF_TOTAL_SIZE, DMA_BIDIRECTIONAL);
#endif
	if (!dma_addr) {
		venc_klog(LOGLVL_ERROR, "no dma_addr, map failed, size= %d", VCMDBUF_TOTAL_SIZE);
		goto fail_map_pg;
	}

	vcmd_klog(LOGLVL_INFO, "venc map success dma_addr = 0x%llx, size 0x%x\n",
			(unsigned long long)dma_addr, VCMDBUF_TOTAL_SIZE);

	/* command buffer */
	vcmd_manager->mem_vcmd.va = cpu_addr;
	vcmd_manager->mem_vcmd.size = CMDBUF_POOL_TOTAL_SIZE;
	vcmd_manager->mem_vcmd.pa = dma_addr;
	vcmd_manager->mem_vcmd.mmap_pa = phys_addr;
	vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_vcmd.iova=0x%llx va=0x%llx.\n",
		(unsigned long long)vcmd_manager->mem_vcmd.pa,
		(unsigned long long)vcmd_manager->mem_vcmd.va);

	/* status buffer */
	vcmd_manager->mem_status.size = CMDBUF_POOL_TOTAL_SIZE;
	vcmd_manager->mem_status.va = cpu_addr +
							CMDBUF_POOL_TOTAL_SIZE / 4;
	vcmd_manager->mem_status.pa = dma_addr +
							CMDBUF_POOL_TOTAL_SIZE;
	vcmd_manager->mem_status.mmap_pa = phys_addr +
							CMDBUF_POOL_TOTAL_SIZE;

	vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_status.iova=0x%llx va=0x%llx.\n",
		(unsigned long long)vcmd_manager->mem_status.pa,
		(unsigned long long)vcmd_manager->mem_status.va);

	/* register buffer */
	vcmd_manager->mem_regs.size = CMDBUF_VCMD_REGISTER_TOTAL_SIZE;
	vcmd_manager->mem_regs.va = cpu_addr +
						(2 * CMDBUF_POOL_TOTAL_SIZE) / 4;
	vcmd_manager->mem_regs.pa = dma_addr +
						2 * CMDBUF_POOL_TOTAL_SIZE;
	vcmd_manager->mem_regs.mmap_pa = phys_addr +
						2 * CMDBUF_POOL_TOTAL_SIZE;

	vcmd_klog(LOGLVL_INFO, "Init: vcmd_mgr->mem_regs.iova=0x%llx va=0x%llx.\n",
		(unsigned long long)vcmd_manager->mem_regs.pa,
		(unsigned long long)vcmd_manager->mem_regs.va);
	return 0;

fail_map_pg:
	dma_free_attrs(platform_dev, VCMDBUF_TOTAL_SIZE, cpu_addr,
			phys_addr, DMA_ATTR_FORCE_CONTIGUOUS);
	return -ENOMEM;
}

void _vcmd_free_dma_mem(void)
{
	dma_unmap_page(platform_dev, (dma_addr_t)vcmd_manager->mem_vcmd.pa,
			VCMDBUF_TOTAL_SIZE, DMA_BIDIRECTIONAL);

	if (vcmd_manager->mem_vcmd.va) {
		dma_free_attrs(
			platform_dev, VCMDBUF_TOTAL_SIZE,
			vcmd_manager->mem_vcmd.va,
			(dma_addr_t)vcmd_manager->mem_vcmd.mmap_pa,
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
	mem->va = (u32 *)dma_alloc_coherent(&platformdev->dev,
							mem->size, &dma_handle,
							GFP_KERNEL | __GFP_DMA32);
	mem->pa = (unsigned long long)dma_handle;

	if (!mem->va || !mem->pa)
		return -1;
	vcmd_klog(LOGLVL_DEBUG, "Init: mem pa=0x%llx, va=0x%llx.\n",
			(unsigned long long)mem->pa, (unsigned long long)mem->va);
	return 0;
}

/**
 * @brief free the specified memory pool, for non-PCIe platform.
 */
static void _vcmd_free_mem(vcmd_mgr_t *vcmd_mgr, struct noncache_mem *mem)
{
	struct kernel_addr_desc mmu_addr;

	if (mem->va) {
		dma_free_coherent(&platformdev->dev,
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
	if (_vcmd_alloc_dma_mem())
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
		/* previously, reg_base not add reg_base_offset */
		dev->subsys_info->reg_base += vcmd_mgr->reg_base_offset;
		m_type = dev->subsys_info->sub_module_type;

		dev->core_id = i;
		dev->state = VCMD_STATE_POWER_ON;
		dev->sw_cmdbuf_rdy_num = 0;
		/* the abnormal interrupts source from VCE */
		dev->abn_irq_mask = ENC_ABNORMAL_IRQ_MASK;
		/* the abnormal interrupts source from UFBC */
		dev->abn_irq_mask |= UFBC_ABNORMAL_IRQ_MASK;
		dev->intr_gate_mask = 0xFFFFFFFF & (~dev->abn_irq_mask);
		dev->slice_to_send = 0;

		dev->arb_reset_irq = 0;
		dev->arb_err_irq = 0;
		dev->ufbc_decoder_err = 0;
		dev->segment_polling_timeout = 0;
		dev->abn_irq_schedule = 0;

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

		_vcmd_init_timer(dev);

		/* init work: work & work_func */
		dev->wq = alloc_workqueue("venc_worker", WQ_HIGHPRI | WQ_UNBOUND, 1);
		if (!dev->wq) {
			vcmd_klog(LOGLVL_ERROR, "alloc_workqueue fail.\n");
			BUG_ON(1);
		}
		INIT_WORK(&dev->_irq_work, _vcmd_irq_bottom_half);

		dev->reg_mem_ba = vcmd_mgr->mem_regs.pa +
							i * SLOT_SIZE_REGBUF - vcmd_mgr->pa_trans_offset;

		dev->reg_mem_va = vcmd_mgr->mem_regs.va + i * SLOT_SIZE_REGBUF / 4;
		dev->reg_mem_sz = SLOT_SIZE_REGBUF;
		memset(dev->reg_mem_va, 0, dev->reg_mem_sz);
		dev->pa_trans_offset = vcmd_mgr->pa_trans_offset;
		dev->reset_count = 0;
		dev->wdt_trigger_count = 0;

		module = &vcmd_mgr->module_mgr[m_type];
		if (module->num == 0)
			sema_init(&module->sem, 1);
		dev->id_in_type = module->num;
		module->dev[module->num++] = dev;
		vcmd_klog(LOGLVL_INFO, "module init - vcmdcore[%d] addr =0x%llx\n",
			i, (unsigned long long)dev->subsys_info->reg_base);
	}
}

/**
 * @brief fill cmdbuf to read sub-module's all regs.
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
	int i;
	u16 reg_off;

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

	/* read submodule register to ddr */
	for (i = 0; i < SUB_MOD_MAX; i++) {
		if (subsys->reg_off[i] != 0xffff && subsys->rreg_id[i] != 0xffff) {
			reg_off = subsys->reg_off[i] + subsys->rreg_id[i] * 4;
			VCMD_READ_CMD(cmd_rreg,
							subsys->rreg_num[i],
							reg_off,
							reg_ba);
		}
	}

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
static void vcmd_release_submodule_IO(struct vcmd_subsys_info *subsys,
										u32 sub_mod_id)
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
static u32 vcmd_reserve_submodule_IO(struct vcmd_subsys_info *subsys,
										u32 sub_mod_id)
{
	addr_t pa = subsys->reg_base + subsys->reg_off[sub_mod_id];
	size_t sz = subsys->io_size[sub_mod_id];

	if (!request_mem_region(pa, sz, "vcx_vcmd_driver"))
		return -EBUSY;
#if (KERNEL_VERSION(4, 17, 0) > LINUX_VERSION_CODE)
	subsys->hwregs[sub_mod_id] = (volatile u8 __force *)ioremap_nocache(pa, sz);
#else
	subsys->hwregs[sub_mod_id] = (volatile u8 __force *)ioremap(pa, sz);
#endif

	return 0;
}

/**
 * @brief config modules for vcmd driver
 */
int vcmd_config_modules(vcmd_mgr_t *vcmd_mgr)
{
	int i;
	struct vcmd_subsys_info *subsys;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		subsys = &vcmd_mgr->core_array[i];
		/* config AXIFE*/
#ifdef HANTROAXIFE_SUPPORT
		if (subsys->hwregs[SUB_MOD_AXIFE0])
			AXIFEEnable(subsys->hwregs[SUB_MOD_AXIFE0], 1);
		if (subsys->hwregs[SUB_MOD_AXIFE1])
			AXIFEEnable(subsys->hwregs[SUB_MOD_AXIFE1], 1);
#endif
	}

	return 0;
}

/**
 * @brief reserve IO resources for vcmd driver.
 */
static int vcmd_reserve_IO(vcmd_mgr_t *vcmd_mgr)
{
	u32 hwid;
	int i, j;
	u32 found_hw = 0;
	struct hantrovcmd_dev *dev;
	struct vcmd_subsys_info *subsys;
	u32 reg_val;
	int ret, has_arbiter;

	vcmd_klog(LOGLVL_INFO, "total_vcmd_core_num is %d\n",
		vcmd_mgr->subsys_num);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		subsys = dev->subsys_info;
		dev->hwregs = NULL;

		for (j = SUB_MOD_VCMD; j < SUB_MOD_MAX; j++) {
			if (subsys->reg_off[j] == 0xffff)
				continue;
			ret = vcmd_reserve_submodule_IO(subsys, j);
			if (ret < 0) {
				vcmd_klog(LOGLVL_ERROR,
						  "failed to reserve submodule[%d] HW regs for vcmd %d\n",
						  j, i);
				vcmd_klog(LOGLVL_ERROR,
							"vcmd_base_addr = 0x%08lx, iosize = %d\n",
							subsys->reg_base, subsys->io_size[j]);
				continue;
			}
			if (!subsys->hwregs[j]) {
				vcmd_klog(LOGLVL_ERROR,
							"failed to ioremap submodule[%d] HW regs\n",
							j);
				release_mem_region(
					subsys->reg_base, subsys->io_size[j]);
				continue;
			}
		}

		dev->hwregs = subsys->hwregs[SUB_MOD_VCMD];
		vcmd_mgr->mmu_hwregs[i][0] = subsys->hwregs[SUB_MOD_MMU0];
		vcmd_mgr->mmu_hwregs[i][1] = subsys->hwregs[SUB_MOD_MMU1];

		/*read hwid and check validness and store it*/
		hwid = (u32)ioread32((void __iomem *)dev->hwregs + 0x0);
		vcmd_klog(LOGLVL_INFO, "hantrovcmd_data[%d].hwregs=0x%p\n", i, dev->hwregs);
		vcmd_klog(LOGLVL_INFO, "hwid=0x%08x\n", hwid);
		/* check for vcmd HW ID */
		if (((hwid >> 16) & 0xFFFF) != VCMD_HW_ID) {
			vcmd_klog(LOGLVL_ERROR, "HW not found at 0x%lx\n",
						subsys->reg_base);
			iounmap((void __iomem *)dev->hwregs);
			release_mem_region(
				subsys->reg_base, subsys->io_size[SUB_MOD_VCMD]);
			dev->hwregs = NULL;
			continue;
		}

		dev->hw_version_id = hwid;
		reg_val = (u32)ioread32((void __iomem *)dev->hwregs +
									VCMD_REGISTER_HW_APB_ARBITER_MODE_OFFSET);
		has_arbiter = (u8)((reg_val >> HW_APB_ARBITER_MODE_BIT) & 0x01);
		dev->hw_feature.vcarb_ver = 0;
		if (has_arbiter)
			dev->hw_feature.vcarb_ver = hwid < HW_ID_1_5_9 ? VCARB_VERSION_2_0 :
										VCARB_VERSION_3_0;
		reg_val = (u32)ioread32((void __iomem *)dev->hwregs +
									VCMD_REGISTER_HW_INIT_MODE_OFFSET);
		dev->hw_feature.has_init_mode = (u8)((reg_val >> HW_INIT_MODE_BIT) &
										0x01);
#ifdef HANTROVCMD_ENABLE_IP_SUPPORT
		if (dev->hw_feature.has_init_mode)
			dev->init_mode = DEFAULT_VCMD_INIT_MODE;
#endif //HANTROVCMD_ENABLE_IP_SUPPORT

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
	u32 i, j;
	struct vcmd_subsys_info *subsys;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		subsys = &vcmd_mgr->core_array[i];
		for (j = SUB_MOD_VCMD; j < SUB_MOD_MAX; j++)
			vcmd_release_submodule_IO(subsys, j);
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
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
							SA_INTERRUPT | SA_SHIRQ,
#else
							IRQF_SHARED,
#endif
							"venc_irq",
							(void *)vcmd_mgr);

		if (result == -EINVAL) {
			vcmd_klog(LOGLVL_ERROR, "vcmd[%d]: Bad vcmd_irq number or handler.\n", i);
			return -1;
		} else if (result == -EBUSY) {
			vcmd_klog(LOGLVL_ERROR, "vcmd[%d]: IRQ <%d> is occupied!!!\n", i, subsys->irq);
			return -1;
		} else {
			venc_init_klog(LOGLVL_INFO, "vcmd[%d]: request IRQ <%d> succeed\n", i, subsys->irq);
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
	u32 vcmd_hwid, vce_hwid;
	struct hantrovcmd_dev *dev;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		regs_va = dev->reg_mem_va;
		vcmd_hwid = *(regs_va + dev->subsys_info->reg_off[SUB_MOD_VCMD] / 4 + 0);
		vce_hwid = *(regs_va + dev->subsys_info->reg_off[SUB_MOD_MAIN] / 4 + 0);

		if ((((vcmd_hwid >> 16) & 0xFFFF) != VCMD_HW_ID) ||
			(((vce_hwid >> 16) & 0xFFFF) != VCE_HW_ID))
			return -1;
	}
	return 0;
}

/**
 * @brief read main module's all regs for all vcmd hw devices.
 */
static void read_main_module_all_registers(vcmd_mgr_t *vcmd_mgr)
{
	int ret;
	struct exchange_parameter param[MAX_SUBSYS_NUM];
	u16 done_id = 0;
	struct proc_obj *po;
	u32 i, irq;
	struct hantrovcmd_dev *dev;
	struct vcmd_module_mgr *module;
	u32 *main_regs_va;

	po = vcmd_mgr->init_po;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		param[i].interrupt_ctrl = 0;
		param[i].priority = CMDBUF_PRIORITY_NORMAL;
		param[i].cmdbuf_size = 0;
		param[i].module_type = dev->subsys_info->sub_module_type;
		param[i].core_mask = 1 << dev->id_in_type;
		param[i].core_id = dev->core_id;
		param[i].has_end_cmd = 1;
		param[i].slice_num = 1;
		param[i].slice_en = 0;
		param[i].frequency = 0;
		param[i].timestamp = 0;
		module = &vcmd_mgr->module_mgr[param[i].module_type];

		ret = reserve_cmdbuf(vcmd_mgr, po, &param[i]);
		create_read_all_registers_cmdbuf(vcmd_mgr, &param[i]);
		link_and_run_cmdbuf(vcmd_mgr, po, &param[i]);
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

		wait_cmdbuf_ready(vcmd_mgr, po, param[i].cmdbuf_id, &done_id);
		main_regs_va = dev->reg_mem_va +
					dev->subsys_info->reg_off[SUB_MOD_MAIN] / 4;
		vcmd_klog(LOGLVL_INFO, "main module register 0:0x%x\n",
			*main_regs_va + 0);
		vcmd_klog(LOGLVL_INFO, "main module register 80:0x%08x\n",
			*(main_regs_va + 80));
		vcmd_klog(LOGLVL_INFO, "main module register 214:0x%08x\n",
			*(main_regs_va + 214));
		vcmd_klog(LOGLVL_INFO, "main module register 226:0x%08x\n",
			*(main_regs_va + 226));
		vcmd_klog(LOGLVL_INFO, "main module register 287:0x%x\n",
			*(main_regs_va + 287));

		release_cmdbuf(vcmd_mgr, po, param[i].cmdbuf_id);
	}
}

/**
 * @brief convert subsys info to core array info of vcmd driver context.
 */
static void SubsysToVcmdCoreCfg(vcmd_mgr_t *vcmd_mgr)
{
	int i, k, array_sz;
	struct vcmd_subsys_info *subsys;
	struct sub_mod_cfg *mod_cfg;
	enum subsys_module_id mod_id;

	array_sz = ARRAY_SIZE(vcmd_core_array);

	/* To plug into vcx_vcmd_driver.c */
	for (i = 0; i < array_sz; i++) {
		if (vcmd_core_array[i].vcmd_base_addr) {
			subsys = &vcmd_mgr->core_array[vcmd_mgr->subsys_num];
			subsys->irq = vcmd_core_array[i].vcmd_irq;
			/* reg_base = vcmd_base_addr + reg_base_offset,
			 * but now reg_base_offset is 0, will be added later
			 */
			subsys->reg_base = vcmd_core_array[i].vcmd_base_addr +
							   /* vcmd_mgr->reg_base_offset */ 0;
			subsys->sub_module_type = vcmd_core_array[i].sub_module_type;
			subsys->vcmd_priority = vcmd_core_array[i].priority;

			for (k = 0; k < SUB_MOD_MAX; k++) {
				mod_cfg = &vcmd_core_array[i].submodule_cfg[k];
				mod_id = mod_cfg->sub_mod_id;
				if (mod_id < SUB_MOD_MAX) {
					subsys->reg_off[mod_id] = mod_cfg->io_off;
					if (mod_cfg->io_off != 0xffff) {
						subsys->io_size[mod_id] = mod_cfg->io_size;
						subsys->rreg_id[mod_id] = mod_cfg->rreg_id;
						subsys->rreg_num[mod_id] = mod_cfg->rreg_num;
					}
				} else {
					vcmd_klog(LOGLVL_ERROR, "wrong sub-module id in vcmd_core_array[%d].submodule_cfg[%d]\n",
							  i, k);
				}
			}

			vcmd_mgr->subsys_num++;
		}
	}
	vcmd_klog(LOGLVL_INFO, "%d VCMD cores found\n", vcmd_mgr->subsys_num);
}

/**
 * @brief get submodule offset in status buffer
 */
static void get_submodule_offset_in_status_buf(struct vcmd_subsys_info *subsys,
			struct config_parameter *param)
{
	u16 offset;
	u32 size;

	param->status_main_addr = 0;
	size = ALIGN_64(subsys->io_size[SUB_MOD_MAIN]);
	offset = param->status_main_addr + size;

	if (param->submodule_L2Cache_addr != 0xffff) {
		param->status_L2Cache_addr = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_L2CACHE]);
		offset += size;
	}
	if (param->submodule_dec400_addr != 0xffff) {
		param->status_dec400_addr = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_DEC400]);
		offset += size;
	}
	if (param->submodule_MMU_addr[0] != 0xffff) {
		param->status_MMU_addr[0] = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_MMU0]);
		offset += size;
	}
	if (param->submodule_MMU_addr[1] != 0xffff) {
		param->status_MMU_addr[1] = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_MMU0]);
		offset += size;
	}
	if (param->submodule_axife_addr[0] != 0xffff) {
		param->status_axife_addr[0] = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_AXIFE0]);
		offset += size;
	}
	if (param->submodule_axife_addr[1] != 0xffff) {
		param->status_axife_addr[0] = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_AXIFE1]);
		offset += size;
	}
	if (param->submodule_ufbc_addr != 0xffff) {
		param->status_ufbc_addr = offset;
		size = ALIGN_64(subsys->io_size[SUB_MOD_UFBC]);
		offset += size;
	}

}

int  venc_pdev_deinit(struct platform_device *pdev)
{
	venc_driver_deinit(&pdev->dev);
	// todo regulator disable
	// venc_rg = regulator_get(platform_dev, "venc_subsys");
	// ret = regulator_enable(venc_rg);
	// if (ret) {
	// 	venc_klog(LOGLVL_ERROR, "Failed to enable venc_rg regulator: %d\n", ret);
	// 	regulator_put(venc_rg);
	// 	return ret;
	// }
	venc_klog(LOGLVL_INFO, "hantroenc removed success.");
	return 0;
}

int venc_driver_deinit(struct device *dev)
{
	int i = 0;
	u32 result;
	vcmd_mgr_t *vcmd_mgr = vcmd_manager;
	struct hantrovcmd_dev *dev_ctx = vcmd_mgr->dev_ctx;
	struct hantroenc_dev *enc_dev = dev_get_drvdata(dev);
	venc_power_mgr *pm = enc_dev->pm;
	venc_clk_mgr *cm = enc_dev->cm;

	_vcmd_kthread_stop(vcmd_mgr);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if (!dev_ctx[i].hwregs)
			continue;

		/* free the vcmd IRQ */
		if (dev_ctx[i].subsys_info->irq != -1)
			free_irq(dev_ctx[i].subsys_info->irq, (void *)vcmd_mgr);
		if (dev_ctx[i].wq)
			destroy_workqueue(dev_ctx[i].wq);
	}

	_vcmd_free_dma_mem();
	venc_misc_deregister();

	free_process_object(vcmd_mgr->init_po);
	vcmd_release_IO(vcmd_mgr);

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		kfifo_free(&vcmd_mgr->dev_ctx[i].send_slice_info_queue);
	}
#ifndef CONFIG_ENC_PM
	hantroenc_power_off(pm);
#else
	if(always_on)
		hantroenc_power_off(pm);
#endif
	hantroenc_power_mgr_deinit(pm);
	enc_dev->pm = NULL;
	venc_clk_mgr_deinit(cm);
	enc_dev->cm = NULL;
	vfree(vcmd_mgr);
	vfree(dev_ctx);
	return 0;
}

int venc_driver_init(void **_vcmd_mgr)
{
	int result = 0;
	vcmd_mgr_t *vcmd_mgr;
	struct hantrovcmd_dev *dev_ctx = NULL;
	int i = 0;
	int j = 0;

	vcmd_mgr = vmalloc(sizeof(vcmd_mgr_t));
	if (!vcmd_mgr)
		return -1;
	memset(vcmd_mgr, 0, sizeof(vcmd_mgr_t));
	vcmd_manager = vcmd_mgr;

	SubsysToVcmdCoreCfg(vcmd_mgr);

	vcmd_mgr->mem_vcmd.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_CMDBUF);
	vcmd_mgr->mem_status.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_STATUSBUF);
	vcmd_mgr->mem_regs.size = ALIGN_4K(vcmd_mgr->subsys_num * SLOT_SIZE_REGBUF);
	result = vcmd_init(vcmd_mgr);
	if (result)
		goto err;

	dev_ctx = vmalloc(sizeof(struct hantrovcmd_dev) * vcmd_mgr->subsys_num);
	if (!dev_ctx)
		goto err1;

	memset(dev_ctx, 0, sizeof(struct hantrovcmd_dev) * vcmd_mgr->subsys_num);

	vcmd_mgr->dev_ctx = dev_ctx;
	dev_ctx_init(vcmd_mgr);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if(kfifo_alloc(&dev_ctx[i].send_slice_info_queue, SLOT_NUM_CMDBUF * sizeof(u16), GFP_DMA | GFP_KERNEL)) {
			for (j = 0; j < i; ++j)
				kfifo_free(&vcmd_mgr->dev_ctx[j].send_slice_info_queue);
			goto err1;
		}
	}

	sema_init(&vcmd_mgr->isr_polling_sema, 1);

	vcmd_mgr->init_po = create_process_object();
	if (!vcmd_mgr->init_po)
		goto err1;

	result = vcmd_reserve_IO(vcmd_mgr);
	if (result < 0)
		goto err2;

	result = vcmd_config_modules(vcmd_mgr);
	if (result < 0)
		goto err3;

	vce_reset_asic(vcmd_mgr);
	vcmd_reset_asic(vcmd_mgr);

#ifdef SUPPORT_DBGFS
	/* for debugfs */
	if (_dbgfs_init((void *)vcmd_mgr))
		goto err3;
	_dbgfs_init_ctx((void *)vcmd_mgr, 0);
#endif

	/* get the IRQ line */
	result = vcmd_reserve_irq(vcmd_mgr);
	if (result < 0)
		goto err3;

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

	*_vcmd_mgr = (void *)vcmd_mgr;
	return 0;

err3:
	vcmd_release_IO(vcmd_mgr);
err2:
	free_process_object(vcmd_mgr->init_po);
err1:
	_vcmd_free_dma_mem();
err:
	if (dev_ctx)
		vfree(dev_ctx);
	if (vcmd_mgr)
		vfree(vcmd_mgr);
	vcmd_klog(LOGLVL_ERROR, "module not inserted!\n");

	return result;
}

/**
 * @brief store the abnormal err irq status to status buf when
 * this cmdbuf is marked as abnormal err irq
 */
static void store_abnormal_err_irq(vcmd_mgr_t *vcmd_mgr, u16 cmdbuf_id)
{
	struct cmdbuf_obj *obj = &vcmd_mgr->objs[cmdbuf_id];
	struct hantrovcmd_dev *dev;
	u32 *status_va = obj->status_va;
	struct vcmd_subsys_info *info;
	struct config_parameter param;

	if (obj->core_id > 0) {
		/* should not happen */
		vcmd_klog(LOGLVL_ERROR, "%s: cmdbuf[%d] obj->core_id(%d) > 0!\n",
			__func__, cmdbuf_id, obj->core_id);
		return;
	}
	dev = &vcmd_mgr->dev_ctx[obj->core_id];
	info = dev->subsys_info;
	get_submodule_offset_in_status_buf(info, &param);

	if (obj->segment_polling_timeout) {
		/* store input segment polling timeout err irq */
		*(status_va + 118) |= 0x1;
		obj->segment_polling_timeout = 0;
	}

	if (obj->ufbc_decoder_err) {
		status_va += param.status_ufbc_addr / 4;
		/* store ufbc decoder err irq */
		*(status_va + 9) |= 0x1;
		obj->ufbc_decoder_err = 0;
	}
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
static long hantrovcmd_ioctl(struct file *filp, unsigned int cmd,
				 unsigned long arg)
{
	int err = 0, tmp;
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;

	_log_ioctl_cmd(cmd);

	/*
	 * extract the type and number bitfields, and don't encode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != HANTRO_IOC_MAGIC
	)
		return -ENOTTY;
	if ((_IOC_TYPE(cmd) == HANTRO_IOC_MAGIC &&
		 _IOC_NR(cmd) > HANTRO_IOC_MAXNR)
	)
		return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
#else
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
#endif
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
#else
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
#endif
	if (err)
		return -EFAULT;

	switch (cmd) {
	case HANTRO_IOCH_GET_VCMD_ENABLE: {
		__put_user(1, (unsigned long __user *)arg);
		break;
	}

	case HANTRO_IOCH_GET_MMU_ENABLE: {
		__put_user(vcmd_mgr->mmu_enable, (unsigned int __user  *)arg);
		break;
	}

	case HANTRO_IOCH_SET_FIXED_FREQ: {
#ifdef CONFIG_ENC_CM_RUNTIME
		struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
		u32 fixed_freq_en = 0;
		__get_user(fixed_freq_en, (u32 __user *)arg);
		if (fixed_freq_en) {
			enc_dev->cm->fixed_clk_rate++;
		} else {
			enc_dev->cm->fixed_clk_rate--;
		}
#endif
		break;
	}

	case HANTRO_IOCH_GET_CMDBUF_PARAMETER: {
		struct cmdbuf_mem_parameter mem;

		vcmd_klog(LOGLVL_DEBUG, " VCMD GET_CMDBUF_PARAMETER\n");
		mem.cmd_unit_size = SLOT_SIZE_CMDBUF;
		mem.status_unit_size = SLOT_SIZE_STATUSBUF;
		mem.reg_unit_size = SLOT_SIZE_REGBUF;
		mem.cmd_total_size = vcmd_mgr->mem_vcmd.size;
		mem.reg_total_size = vcmd_mgr->mem_regs.size;
		mem.status_total_size = vcmd_mgr->mem_status.size;
		mem.status_phy_addr = vcmd_mgr->mem_status.pa;
		mem.cmd_phy_addr = vcmd_mgr->mem_vcmd.pa;
		mem.reg_phy_addr = vcmd_mgr->mem_regs.pa;
		mem.cmd_virt_addr = NULL;
		vcmd_klog(LOGLVL_DEBUG, "HANTRO_IOCH_GET_CMDBUF_PARAMETER vcmd_mgr->mem_regs.pa 0x%llx\n", vcmd_mgr->mem_regs.pa);
		if (vcmd_mgr->mmu_enable) {
			mem.status_hw_addr = vcmd_mgr->mem_status.mmu_ba;
			mem.cmd_hw_addr = vcmd_mgr->mem_vcmd.mmu_ba;
			mem.reg_hw_addr = vcmd_mgr->mem_regs.mmu_ba;
		} else {
			mem.status_hw_addr = vcmd_mgr->mem_status.pa - vcmd_mgr->pa_trans_offset;
			mem.cmd_hw_addr = vcmd_mgr->mem_vcmd.pa - vcmd_mgr->pa_trans_offset;
			mem.reg_hw_addr = vcmd_mgr->mem_regs.pa - vcmd_mgr->pa_trans_offset;
		}
		mem.base_ddr_addr = vcmd_mgr->pa_trans_offset;
		tmp = copy_to_user((struct cmdbuf_mem_parameter __user *)arg, &mem,
				 sizeof(struct cmdbuf_mem_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
		break;
	}
	case HANTRO_IOCH_GET_VCMD_PARAMETER: {
		int i;
		struct config_parameter param;
		struct proc_obj *po = NULL;
		u16 m_type;
		struct hantrovcmd_dev *dev;
		struct vcmd_subsys_info *info = NULL;

		vcmd_klog(LOGLVL_DEBUG, " VCMD get vcmd config parameter\n");
		tmp = copy_from_user(&param, (struct config_parameter __user *)arg,
					 sizeof(struct config_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
		po = (struct proc_obj *)ctx->po;
		if (!po) {
			vcmd_klog(LOGLVL_ERROR, "not find process obj!\n");
			return -1;
		}

		po->module_type = param.module_type;
		m_type = param.module_type;

		/* fuzz report */
		if (m_type >= MAX_VCMD_TYPE) {
			venc_klog(LOGLVL_ERROR, "m_type(%u) > MAX_VCMD_TYPE\n", m_type);
			return -EINVAL;
		}

		if (vcmd_mgr->module_mgr[m_type].num) {
			dev = vcmd_mgr->module_mgr[m_type].dev[0];
			info = dev->subsys_info;
			param.submodule_main_addr = info->reg_off[SUB_MOD_MAIN];
			param.submodule_L2Cache_addr = info->reg_off[SUB_MOD_L2CACHE];
			param.submodule_dec400_addr = info->reg_off[SUB_MOD_DEC400];
			param.submodule_MMU_addr[0] = info->reg_off[SUB_MOD_MMU0];
			param.submodule_MMU_addr[1] = info->reg_off[SUB_MOD_MMU1];
			param.submodule_axife_addr[0] = info->reg_off[SUB_MOD_AXIFE0];
			param.submodule_axife_addr[1] = info->reg_off[SUB_MOD_AXIFE1];
			param.submodule_ufbc_addr = info->reg_off[SUB_MOD_UFBC];
			param.vcmd_hw_version_id =	dev->hw_version_id;
			param.vcmd_core_num = vcmd_mgr->module_mgr[m_type].num;
			//get priority of vcmd
			for (i = 0; i < vcmd_mgr->subsys_num; i++)
				param.vcmd_priority[i] = info->vcmd_priority;
			get_submodule_offset_in_status_buf(info, &param);
		} else {
			param.submodule_main_addr = 0xffff;
			param.submodule_dec400_addr = 0xffff;
			param.submodule_L2Cache_addr = 0xffff;
			param.submodule_MMU_addr[0] = 0xffff;
			param.submodule_MMU_addr[1] = 0xffff;
			param.submodule_axife_addr[0] = 0xffff;
			param.submodule_axife_addr[1] = 0xffff;
			param.submodule_ufbc_addr = 0xffff;
			param.vcmd_core_num = 0;
			param.vcmd_hw_version_id = HW_ID_1_0_C;
			for (i = 0; i < vcmd_mgr->subsys_num; i++)
				param.vcmd_priority[i] = 0;
		}


		tmp = copy_to_user((struct config_parameter __user *)arg, &param,
				 sizeof(struct config_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
		break;
	}
	case HANTRO_IOCH_RESERVE_CMDBUF: {
		long ret;
		struct exchange_parameter param;
		struct proc_obj *po = _GET_PO(filp);

		tmp = copy_from_user(&param, (struct exchange_parameter __user *)arg,
					 sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
		ret = reserve_cmdbuf(vcmd_mgr, po, &param);
		if (ret == 0) {
			tmp = copy_to_user((struct exchange_parameter __user *)arg,
					 &param,
					 sizeof(struct exchange_parameter));
			if (tmp) {
				vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %i\n", tmp);
				return -EFAULT;
			}
		}
		vcmd_klog(LOGLVL_DEBUG, " VCMD Reserve CMDBUF %d\n", param.cmdbuf_id);
		return ret;
	}

	case HANTRO_IOCH_LINK_RUN_CMDBUF: {
		struct exchange_parameter param;
		long retVal;
		struct proc_obj *po = _GET_PO(filp);

		tmp = copy_from_user(&param, (struct exchange_parameter __user *)arg,
					 sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_from_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_RUNTIME
		if(runtime_pm_enable) {
			if(!always_on) {
				struct hantroenc_dev *enc_dev;
				enc_dev = dev_get_drvdata(&platformdev->dev);
				mutex_lock(&enc_dev->pm->pm_lock);
				hantroenc_pm_runtime_get(&platformdev->dev);
				mutex_unlock(&enc_dev->pm->pm_lock);
			}
		}
#endif
#endif
		vcmd_klog(LOGLVL_INFO, "VCMD link and run CMDBUF %d\n", param.cmdbuf_id);
		retVal = link_and_run_cmdbuf(vcmd_mgr, po, &param);
		tmp = copy_to_user((struct exchange_parameter __user *)arg, &param,
				 sizeof(struct exchange_parameter));
		if (tmp) {
			vcmd_klog(LOGLVL_ERROR, "copy_to_user failed, returned %i\n", tmp);
			return -EFAULT;
		}
		return retVal;
	}

	case HANTRO_IOCH_WAIT_CMDBUF: {
		u16 cmdbuf_id;
		long tmp;
		struct proc_obj *po = _GET_PO(filp);


		__get_user(cmdbuf_id, (u16 __user *)arg);
		/*high 16 bits are core id, low 16 bits are cmdbuf_id*/

		//TODO
		tmp = wait_cmdbuf_ready(vcmd_mgr, po, cmdbuf_id, &cmdbuf_id);
		if (tmp == 0 || tmp == 1) {
#ifdef CONFIG_ENC_CM_RUNTIME
			struct hantroenc_dev *enc_dev = platform_get_drvdata(platformdev);
			WARN_ON(!enc_dev);
			//no channel use fixed clk rate, set clk rate to default if hw idle
			if (enc_dev->cm->fixed_clk_rate == 0) {
				mutex_lock(&enc_dev->pm->pm_lock);
				if (runtime_cm_enable && regulator_is_enabled(enc_dev->pm->venc_rg)) {
					if (_runtime_context_idled(vcmd_mgr)) {
						venc_cm_klog(LOGLVL_DEBUG, "venc enter idle state, set clk rate from %ld to %ld\n",
							enc_dev->pm->venc_clk->work_rate, enc_dev->pm->venc_clk->default_rate);
						//default rate set by dts is smaller than VENC_CLK_MIN_RATE
						venc_clk_rate_config(enc_dev->pm, enc_dev->pm->venc_clk->default_rate);
					}
				}
				mutex_unlock(&enc_dev->pm->pm_lock);
			}
#endif
#ifdef CONFIG_ENC_PM
#ifdef CONFIG_ENC_PM_RUNTIME
			if(runtime_pm_enable) {
				if(!always_on) {
					u32 num = 0;
					u32 core_id = 0;
					struct hantrovcmd_dev *vdev;

					for (core_id = 0; core_id < vcmd_mgr->subsys_num; core_id++) {
						vdev = &vcmd_mgr->dev_ctx[core_id];
						num = dev_wait_job_num(vdev);
					}

					if (num == 0) {
						struct hantroenc_dev *enc_dev;
						enc_dev = dev_get_drvdata(&platformdev->dev);
						mutex_lock(&enc_dev->pm->pm_lock);
						hantroenc_pm_runtime_put_sync_autosuspend(&platformdev->dev);
						mutex_unlock(&enc_dev->pm->pm_lock);
					}
				}
			}
#endif
#endif
			/* if occured abnormal err irq, store the irq status to status buf */
			store_abnormal_err_irq(vcmd_mgr, cmdbuf_id);
			__put_user(cmdbuf_id, (u16 __user *)arg);
			vcmd_klog(LOGLVL_INFO, "VCMD wait for CMDBUF %d finishing.\n", cmdbuf_id);
			return tmp; //return core_id
		} else {
			return -1;
		}

		break;
	}
	case HANTRO_IOCH_RELEASE_CMDBUF: {
		u16 cmdbuf_id;
		struct proc_obj *po = _GET_PO(filp);

		__get_user(cmdbuf_id, (u16 __user *)arg);
		/*16 bits are cmdbuf_id*/

		vcmd_klog(LOGLVL_DEBUG, "VCMD release CMDBUF\n");

		release_cmdbuf(vcmd_mgr, po, cmdbuf_id);
		return 0;
		break;
	}
	case HANTRO_IOCH_POLLING_CMDBUF: {
		u16 core_id;
		u32 i;

		__get_user(core_id, (u16 __user *)arg);

		/*16 bits are cmdbuf_id*/
		if ((core_id >= vcmd_mgr->subsys_num) && (core_id != 0xffff))
			return -1;

		if (down_killable(&vcmd_mgr->isr_polling_sema)) {
			vcmd_klog(LOGLVL_ERROR, "sema-down is killable!\n");
			return -EINTR;
		}

		/* TODO: always to schedule work? */
		if (core_id != 0xffff) {
			hantrovcmd_isr(core_id, vcmd_mgr);
		} else {
			for (i = 0; i < vcmd_mgr->subsys_num; i++)
				hantrovcmd_isr(i, vcmd_mgr);
		}
		up(&vcmd_mgr->isr_polling_sema);

		return 0;
		break;

	}
	default: {
		break;
	}
	}
	return 0;
}

/**
 * @brief open hantro vcmd device.
 */
static int hantrovcmd_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct proc_obj *po = NULL;

	struct vcmd_priv_ctx *ctx = NULL;
	struct hantroenc_dev *enc_dev;

	enc_dev = dev_get_drvdata(platform_dev);

	ctx = vmalloc(sizeof(struct vcmd_priv_ctx));
	if (!ctx) {
		vcmd_klog(LOGLVL_ERROR, "Create vcmd private context failed!\n");
		return -EINVAL;
	}
	memset(ctx, 0, sizeof(struct vcmd_priv_ctx));

	filp->private_data = (void *)ctx;
	ctx->vcmd_mgr = (void *)vcmd_manager;

#ifdef SUPPORT_DBGFS
	/* for debugfs */
	_dbgfs_init_ctx(ctx->vcmd_mgr, 1);
#endif

	po = create_process_object();
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "Create process object failed!\n");
		vfree(ctx);
		return -EINVAL;
	}
	po->filp = filp;
	po->module_type = ((vcmd_mgr_t *)ctx->vcmd_mgr)->core_array[0].sub_module_type;
	ctx->po = (void *)po;

#ifdef CONFIG_ENC_PM
	mutex_lock(&enc_dev->pm->pm_lock);
	ret = venc_power_on(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	if (ret) {
		venc_pm_klog(LOGLVL_ERROR, "Failed to power on media2_subsys: %d\n", ret);
		goto media2_subsys_power_on_fail;
	}
#ifdef CONFIG_ENC_PM_RUNTIME
	if(runtime_pm_enable) {
		venc_pm_klog(LOGLVL_ERROR, "media2_subsys power on\n");
	} else
#endif
	{
		vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);

		ret = venc_power_on(enc_dev->pm, POWER_VENC_SUBSYS);
		if (ret) {
		    venc_pm_klog(LOGLVL_ERROR, "Failed to power on venc_subsys: %d\n", ret);
		    goto venc_subsys_power_on_fail;
		}
	}
	mutex_unlock(&enc_dev->pm->pm_lock);
#endif
	vcmd_klog(LOGLVL_DEBUG, "dev opened\n");
	vcmd_klog(LOGLVL_INFO, "process obj %p for filp opened %p\n", (void *)po, (void *)filp);
	return 0;
#ifdef CONFIG_ENC_PM
venc_subsys_power_on_fail:
	venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
media2_subsys_power_on_fail:
	mutex_unlock(&enc_dev->pm->pm_lock);
	free_process_object(ctx->po);
	ctx->po = NULL;
	vfree(ctx);
	return ret;
#endif
}

/**
 * @brief release hantro vcmd device.
 */
static int hantrovcmd_release(struct inode *inode, struct file *filp)
{
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;
	struct proc_obj *po;
	struct hantrovcmd_dev *dev;
	bi_list *list;
	bi_list_node *node;
	struct cmdbuf_obj *obj;
	u32 module_type;
	u32 i;
	u32 has_work_node, vcmd_aborted;
	int abort_cmdbuf_id;
	unsigned long flags;
	int get_down_flag = 0;
	int ret = 0;

	struct hantroenc_dev *enc_dev;
	enc_dev = dev_get_drvdata(platform_dev);

	vcmd_klog(LOGLVL_INFO, "process %p start release\n", (void *)filp);
	po = (struct proc_obj *)ctx->po;
	if (!po) {
		vcmd_klog(LOGLVL_ERROR, "%s: not find process obj!\n", __func__);
		vfree(ctx);
		return -EINVAL;
	}

	module_type = po->module_type;
	/* fuzz report */
	if (module_type >= MAX_VCMD_TYPE) {
		venc_klog(LOGLVL_ERROR, "m_type(%u) > MAX_VCMD_TYPE(%u)\n", module_type, MAX_VCMD_TYPE);
		ret = -EINVAL;
		goto out;
	}

	if (!vcmd_mgr->module_mgr[module_type].sem.wait_list.prev &&
		!vcmd_mgr->module_mgr[module_type].sem.wait_list.next) {
		venc_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		ret = -EINVAL;
		goto out;
	}

	down(&vcmd_mgr->module_mgr[module_type].sem);
	get_down_flag = 1;

	//remove nodes in dev->work_list
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		if (dev == NULL || dev->hwregs == NULL)
			continue;

		list = &dev->work_list;

		has_work_node = 0;
		vcmd_aborted = 0;

		down(&dev->work_sem);
		node = list->head;
		while (node) {
			obj = (struct cmdbuf_obj *)node->data;
			if (obj->po == po) {
				has_work_node = 1;
				break;
			}
			node = node->next;
		}
		up(&dev->work_sem);

		if (has_work_node) {
			spin_lock_irqsave(dev->spinlock, flags);
			if (dev->state == VCMD_STATE_WORKING) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				vcmd_klog(LOGLVL_DEBUG, "Abort dev[%d].\n", dev->core_id);
				vcmd_abort(vcmd_mgr, dev, &abort_cmdbuf_id);
				spin_lock_irqsave(dev->spinlock, flags);
				if (dev->state != VCMD_STATE_IDLE &&
					dev->state != VCMD_STATE_POWER_OFF) {
					vcmd_klog(LOGLVL_ERROR, "dev [%d] is not aborted as expected.", dev->core_id);
					spin_unlock_irqrestore(dev->spinlock, flags);
					continue;
				}
				vcmd_aborted = 1;
			}
			spin_unlock_irqrestore(dev->spinlock, flags);

			down(&dev->work_sem);
			node = list->head;
			while (node) {
				obj = (struct cmdbuf_obj *)node->data;
				vcmd_klog(LOGLVL_VERBOSE, "Process %p release: checking cmdbuf %d of process %p\n",
					filp, obj->cmdbuf_id, obj->filp);
				if (obj->po == po || obj->cmdbuf_need_remove)
					dev_remove_job(vcmd_mgr, dev, node);
				node = node->next;
			}
			up(&dev->work_sem);

			vcmd_klog(LOGLVL_VERBOSE, "Restart dev[%d].\n", dev->core_id);
			if (vcmd_aborted == 1)
				vcmd_start2(dev, 0);
		}
	}

	// remove cmdbuf reserved but not in work_list
	for (i = 0; i < SLOT_NUM_CMDBUF; i++) {
		obj = &vcmd_mgr->objs[i];
		if (obj->po == po) {
			if (obj->core_id == 0xFFFF) {
				return_cmdbuf(vcmd_mgr, i);
			} else {
				dev = &vcmd_mgr->dev_ctx[obj->core_id];
				spin_lock_irqsave(dev->po_spinlock, flags);
				return_cmdbuf(vcmd_mgr, i);
				spin_unlock_irqrestore(dev->po_spinlock, flags);
			}
		}
	}
out:
	vcmd_klog(LOGLVL_DEBUG, "process obj %p for filp to be removed: %p\n",
			(void *)po, (void *)po->filp);
	free_process_object(ctx->po);
	ctx->po = NULL;
	vfree(ctx);
	if (get_down_flag)
		up(&vcmd_mgr->module_mgr[module_type].sem);
#ifdef CONFIG_ENC_PM
	mutex_lock(&enc_dev->pm->pm_lock);
#ifdef CONFIG_ENC_PM_RUNTIME
	if(runtime_pm_enable) {
		venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	} else
#endif
	{
		vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)(enc_dev->priv_data);

		venc_power_off(enc_dev->pm, POWER_VENC_SUBSYS);
		venc_power_off(enc_dev->pm, POWER_MEDIA2_SUBSYS);
	}
	mutex_unlock(&enc_dev->pm->pm_lock);
#endif

	return ret;
}

/**
 * vm_ops
 */
static const struct vm_operations_struct hantrovcmd_vm_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
	.access = generic_access_phys
#endif
};

/**
 * @brief mmap to mem-pool of vcmd device.
 */
#define IN_MEM_RANGE(mem, s, e) ((s) >= (mem).pa && \
								 (e) <= ((mem).pa + (mem).size))
static int hantrovcmd_mmap(struct file *filp, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	int ret = 0;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;

	vcmd_klog(LOGLVL_VERBOSE,"%08lx-%08lx -> %08lx, %s\n",
		   (long)(vma->vm_pgoff << PAGE_SHIFT),
		   (long)(vma->vm_pgoff << PAGE_SHIFT) + (int)(size),
		   (long)vma->vm_start,
		   (filp->f_flags & O_SYNC) ? "uncached" : "cached");

	/* check if offset validity */
	if (offset + size < offset || offset + size < size || offset + size > VCMDBUF_TOTAL_SIZE) {
		venc_klog(LOGLVL_ERROR, "invalid offset or size parameters.\n");
		return -EINVAL;
	}

	// support only uncached mode
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	ret = remap_pfn_range(vma, vma->vm_start,
		(vcmd_mgr->mem_vcmd.mmap_pa >> PAGE_SHIFT) + vma->vm_pgoff,
		size, vma->vm_page_prot);
	if (ret != 0) {
		venc_klog(LOGLVL_ERROR, "remap_pfn_range() failed.\n");
		goto err_out;
	}

	return 0;

err_out:
	return ret;
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
	/* abnormal interrupt source from VCE */
	if (intr_src & ENC_ABNORMAL_IRQ_MASK)
		process_vce_abn_irq(vcmd_mgr, dev, obj);
	/* abnormal interrupt source from UFBC */
	if (intr_src & UFBC_ABNORMAL_IRQ_MASK)
		process_ufbc_abn_irq(vcmd_mgr, dev, obj);
}

/**
 * @brief mark cmdbuf obj as run_done with specified exe_status,
 * remove it from device work_list, and add it to owner po's job_done_list.
 */
static int isr_process_node(vcmd_mgr_t *vcmd_mgr, bi_list_node *node,
							u32 exe_status)
{
	struct cmdbuf_obj *obj = (struct cmdbuf_obj *)node->data;
	struct hantrovcmd_dev *dev;
	unsigned long flags;

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
		dev_delink_job(dev, node);
		spin_lock_irqsave(dev->po_spinlock, flags);
		proc_add_done_job(vcmd_mgr, obj);
		spin_unlock_irqrestore(dev->po_spinlock, flags);
#ifdef SUPPORT_DBGFS
		if (exe_status == CMDBUF_EXE_STATUS_OK) {
			_dbgfs_remove_cmdbuf(dev->dbgfs_info, obj->cmdbuf_id);
			_dbgfs_record_vce_cycles(dev->dbgfs_info, obj->cmdbuf_id,
						obj->module_type, dev->hw_feature.vcarb_ver);
		}
#endif
		return 0;
	} else {
		vcmd_klog(LOGLVL_DEBUG, "cmdbuf[%d] is already done!!\n",
				obj->cmdbuf_id);
		return -1;
	}
}

/**
 * @brief read slice size in slice rdy irq and frame rdy irq.
 */
void process_vce_slice_size(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev, struct cmdbuf_obj *obj, u32 stage)
{
	u32 slice_rdy_num;
	int slice_idx = 0;
	volatile void *hwregs = dev->subsys_info->hwregs[SUB_MOD_MAIN];

	if (obj->slice_info.slice_mode == 0)
		return;

	if (stage == SEND_SLICE_FRAME_READY) {
		obj->slice_info.frame_rdy = 1;
		slice_rdy_num = obj->slice_info.slice_num;
	} else {
		/*slice ready number stored in vc9000e swreg7 bit 17 - bit 24*/
		slice_rdy_num = (u32)ioread32((void __iomem *)(hwregs + 0x1C));
		slice_rdy_num = (slice_rdy_num >> 17) & 0xFF;
		if (slice_rdy_num > 4) {
			vcmd_klog(LOGLVL_ERROR, "slice ready num > 4, only 4 slice will be send\n");
			slice_rdy_num = 4;
		}
	}

	if (obj->slice_info.sizeTbl_buf.va) {
		for (slice_idx = obj->slice_info.slice_send_num; slice_idx < slice_rdy_num; ++slice_idx) {
			obj->slice_info.slice_size[slice_idx] = *(obj->slice_info.sizeTbl_buf.va + slice_idx);
		}
	}

	obj->slice_info.slice_rdy_num = slice_rdy_num;
	kfifo_in(&dev->send_slice_info_queue, &obj->cmdbuf_id, sizeof(u16));
	dev->slice_to_send = 1;
	_vcmd_kthread_wakeup(vcmd_mgr);
}

/**
 * @brief continue to process finished cmdbuf nodes, avoid the cmdbuf finished but won't trigeger ISR
 */
static int _has_finished_node(struct hantrovcmd_dev *dev, u16 prev_id)
{
	u16 id = *(dev->reg_mem_va + REG_ID_CMDBUF_EXE_ID);

	if (id == prev_id)
		return SLOT_NUM_CMDBUF;

	return id;
}

static int isr_process_finished_nodes(vcmd_mgr_t *vcmd_mgr, struct hantrovcmd_dev *dev,
					u16 prev_id)
{
	u16 curr_id;
	bi_list_node *node, *curr_node;
	struct cmdbuf_obj *curr_obj;

	curr_id = _has_finished_node(dev, prev_id);
	if (curr_id >= SLOT_NUM_CMDBUF)
		return -1;

	vcmd_klog(LOGLVL_INFO, "%s current cmdbuf id is %d.\n", __func__, curr_id);
	curr_node = &vcmd_mgr->nodes[curr_id];
	curr_obj = &vcmd_mgr->objs[curr_id];

	if (curr_obj->cmdbuf_run_done == 0) {
		node = dev->work_list.head;
		while (node) {
			isr_process_node(vcmd_mgr, node, CMDBUF_EXE_STATUS_OK);
			if (node == curr_node)
				break;
			node = node->next;
		}
	}
	return 0;
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
		vcmd_klog(LOGLVL_WARNING, "venc th->bh take (%llu)us.\n", us);

	while (1) {
		spin_lock_irqsave(dev->spinlock, flags);

		irq_status = dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4];
		dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] = 0;

		if (dev->abn_irq_schedule & ABN_IRQ_SCHEDULE_SLICE_RDY) {
			dev->abn_irq_schedule &= (~ABN_IRQ_SCHEDULE_SLICE_RDY);
			hook_process_vce_slice_info(dev);
			if (irq_status == 0) {
				spin_unlock_irqrestore(dev->spinlock, flags);
				continue;
			}
		}
		if (irq_status == 0) {
			spin_unlock_irqrestore(dev->spinlock, flags);
			break;
		}
		spin_unlock_irqrestore(dev->spinlock, flags);

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
			vcmd_klog(LOGLVL_WARNING, "%s:received IRQ but core has nothing to do.\n", __func__);
			up(&dev->work_sem);
			return;
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
			vcmd_klog(LOGLVL_ERROR, "%s failed to get executing node!!\n",
					__func__);
			up(&dev->work_sem);
			return;
		}
		curr_obj = (struct cmdbuf_obj *)curr_node->data;
		curr_id = curr_obj->cmdbuf_id;

#ifdef VCMD_DEBUG_INTERNAL
			_dbg_log_dev_regs(dev, 0);
#endif

		if (curr_obj->core_id != dev->core_id) {
			vcmd_klog(LOGLVL_ERROR, "error cmdbuf_id, core_id[%d] is not dev id[%d] irq_status=%#x!!\n",
						curr_id, dev->core_id, irq_status);
			up(&dev->work_sem);
			continue;
		}

		/* process nodes between head node and curr_node (exclusive):
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
				vcmd_klog(LOGLVL_ERROR, "not find node[%d] in dev work_list!!\n",
							curr_obj->cmdbuf_id);
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

		if (dev->hw_feature.vcarb_ver == VCARB_VERSION_2_0 &&
			(irq_status & VCMD_IRQ_ARBITER_RESET)) {
			// vcmd arbiter reset err
			dev->arb_reset_irq = 1;
			up(&dev->work_sem);
			continue;
		}

		if (dev->hw_feature.vcarb_ver &&
			(irq_status & VCMD_IRQ_ARBITER_ERR)) {
			// vcmd arbiter err if not own arbiter
			dev->arb_err_irq = 1;
			up(&dev->work_sem);
			continue;
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
				vcmd_klog(LOGLVL_WARNING, "venc th_abort->bh_abort take (%llu)us.\n", us);

			trace_hantrovcmd_isr_hw_abort("[enc] finish frame(hw abort): cmdbuf ", curr_id);
			//abort error
			spin_lock_irqsave(dev->spinlock, flags);
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
				curr_obj->executing_status = CMDBUF_EXE_STATUS_ABORTED;
				dev->reset_count = 0;
				dev->wdt_trigger_count = 0;
			}

			up(&dev->work_sem);

			//to notify owner which triggered the abort
			wake_up_all(dev->abort_waitq);
			continue;
		}
#if 0
		/* not report bus err interrup, because it will trigger abort
		* and ensure vcmd is idle.
		*/
		if (irq_status & VCMD_IRQ_BUS_ERR) {
			//bus error, don't need to reset where to record status?
			isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_BUSERR);

			vcmd_start2(dev, VCMD_STATE_IDLE, 0);

			up(&dev->work_sem);

			continue;
		}
#endif
		if (irq_status & VCMD_IRQ_TIMEOUT) {
			//time out
			trace_hantrovcmd_isr_hw_timeout("[enc] hw timeout: cmdbuf ", curr_id);
			vcmd_klog(LOGLVL_WARNING, "[enc] hw timeout curr_id=%#x end=%s\n",
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
#ifdef TIMEOUT_IRQ_TIMER
			/* if vcmd cmderr waited, del vcmd timeout timer */
			spin_lock_irqsave(dev->spinlock, flags);
			_vcmd_del_timer(dev, VCMD_TIMER_TIMEOUT);
			spin_unlock_irqrestore(dev->spinlock, flags);
#endif
			trace_hantrovcmd_isr_cmd_err("[enc] finish frame(cmd error): cmdbuf ", curr_id);
			isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_CMDERR);

			//command error, re-start from next node
			vcmd_start2(dev, 0);
			up(&dev->work_sem);

			continue;
		}

		//JMP or END interrupt
		trace_hantrovcmd_isr_finish_success("[enc] finish frame(success): cmdbuf ", curr_id);
		if (curr_obj->slice_info.slice_mode)
			vcmd_wfd_trace("finish frame(success): cmdbuf[%u]\n", curr_id);

		isr_process_node(vcmd_mgr, curr_node, CMDBUF_EXE_STATUS_OK);
		{
			/* if has finished cmdbuf nodes, continue to process them */
			isr_process_finished_nodes(vcmd_mgr, dev, curr_id);
		}
		spin_lock_irqsave(dev->spinlock, flags);
		if (irq_status & VCMD_IRQ_END) {
#ifdef TIMEOUT_IRQ_TIMER	/* if vcmd end waited, del vcmd timeout timer */
			_vcmd_del_timer(dev, VCMD_TIMER_TIMEOUT);
#endif

			//end command interrupt, start next node
			dev->state = VCMD_STATE_IDLE;
			vcmd_start(dev);
		} else {
#ifdef SUPPORT_WATCHDOG
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
	u32 i;
	volatile u8 *hwregs;
	unsigned long flags;
	bool ret;
	u32 curr_id;

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
			} else
				spin_unlock_irqrestore(dev->spinlock, flags);
			return IRQ_HANDLED;
		}
	}

#ifdef VCMD_DEBUG_INTERNAL
	_dbg_log_dev_regs(dev, 1);
#endif

	if (irq_status == 0) {
		//no interrupt source from dev
		//vcmd_klog(LOGLVL_BRIEF, "%s warning, irq_status is zero!\n", __func__);
		spin_unlock_irqrestore(dev->spinlock, flags);
		return IRQ_HANDLED;
	}

	if ((irq_status & VCMD_IRQ_ERR_MASK) == 0 && (irq_status & VCMD_IRQ_JMP || irq_status & VCMD_IRQ_END)) {
		curr_id = *(dev->reg_mem_va + REG_ID_CMDBUF_EXE_ID);
		curr_obj = &vcmd_mgr->objs[curr_id];
		process_vce_slice_size(vcmd_mgr, dev, curr_obj, SEND_SLICE_FRAME_READY);
	}

	vcmd_write_reg((const void *)hwregs,
						VCMD_REGISTER_INT_STATUS_OFFSET, irq_status);
	dev->reg_mirror[VCMD_REGISTER_INT_STATUS_OFFSET / 4] = irq_status;

	spin_unlock_irqrestore(dev->spinlock, flags);

	vcmd_klog(LOGLVL_DEBUG, "%s: received IRQ!\n", __func__);
	vcmd_klog(LOGLVL_DEBUG, "irq_status of core[%u] is: 0x%x\n", dev->core_id, irq_status);

	/* schedule work to workqueue */
	ret = queue_work(dev->wq, &dev->_irq_work);
	if (ret) {
		dev->top_half_start = ktime_get();
		if (irq_status & VCMD_IRQ_ABORT)
			dev->top_half_abort_start = ktime_get();
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_ENC_PM
/**
 * @brief suspend for vcmd driver power management
 */
int vcmd_pm_suspend(void *handler)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)handler;
	struct hantrovcmd_dev *dev;
	struct hantroenc_dev *enc_dev = dev_get_drvdata(&platformdev->dev);
	u32 aborted_id;
	int i;
	unsigned long flags;
	u32 state;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];

		spin_lock_irqsave(dev->spinlock, flags);
		state = dev->state;
		if (state == VCMD_STATE_WORKING && atomic_read(&enc_dev->pm->power_count[POWER_VENC_SUBSYS])) {
			spin_unlock_irqrestore(dev->spinlock, flags);
			vcmd_abort(vcmd_mgr, dev, &aborted_id);
			spin_lock_irqsave(dev->spinlock, flags);
			if (dev->state != VCMD_STATE_IDLE) {
				vcmd_klog(LOGLVL_ERROR, "suspend failed for dev [%d].", dev->core_id);
				spin_unlock_irqrestore(dev->spinlock, flags);
				return -EBUSY;
			}
		}
		spin_unlock_irqrestore(dev->spinlock, flags);
	}

	return 0;
}

/**
 * @brief resume for vcmd driver power management
 */
int vcmd_pm_resume(void *handler)
{
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)handler;
	struct hantrovcmd_dev *dev;
	struct hantroenc_dev *enc_dev = dev_get_drvdata(&platformdev->dev);
	int i;
	unsigned long flags;

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		dev = &vcmd_mgr->dev_ctx[i];
		spin_lock_irqsave(dev->spinlock, flags);
		if (dev->state == VCMD_STATE_POWER_ON && atomic_read(&enc_dev->pm->power_count[POWER_VENC_SUBSYS])) {
			vcmd_start(dev);
		}
		spin_unlock_irqrestore(dev->spinlock, flags);
	}
	return 0;
}
#endif

/**
 * @brief VFS methods and VM operations
 */
static const struct file_operations hantrovcmd_fops = {
	.owner = THIS_MODULE,
	.open = hantrovcmd_open,
	.mmap = hantrovcmd_mmap,
	.release = hantrovcmd_release,
	.unlocked_ioctl = hantrovcmd_ioctl,
	.fasync = NULL,
};

static struct miscdevice venc_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRIVER_NAME,
	.fops = &hantrovcmd_fops,
};

/**
 * @brief vcmd driver initialization
 * @return int __init: 0: successful; other: failed.
 */
int hantroenc_vcmd_init(void **_vcmd_mgr)
{
	int result = 0;
	vcmd_mgr_t *vcmd_mgr;
	struct hantrovcmd_dev *dev_ctx;
	int i = 0;
	int j = 0;

	vcmd_mgr = vmalloc(sizeof(vcmd_mgr_t));
	if (!vcmd_mgr)
		return -1;
	memset(vcmd_mgr, 0, sizeof(vcmd_mgr_t));
	vcmd_manager = vcmd_mgr;

	SubsysToVcmdCoreCfg(vcmd_mgr);

	vcmd_mgr->mem_vcmd.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_CMDBUF);
	vcmd_mgr->mem_status.size = ALIGN_4K(SLOT_NUM_CMDBUF * SLOT_SIZE_STATUSBUF);
	vcmd_mgr->mem_regs.size = ALIGN_4K(vcmd_mgr->subsys_num * SLOT_SIZE_REGBUF);
	result = vcmd_init(vcmd_mgr);
	if (result)
		goto err1;

	dev_ctx = vmalloc(sizeof(struct hantrovcmd_dev) * vcmd_mgr->subsys_num);
	if (!dev_ctx)
		goto err1;
	memset(dev_ctx, 0, sizeof(struct hantrovcmd_dev) * vcmd_mgr->subsys_num);

	vcmd_mgr->dev_ctx = dev_ctx;
	dev_ctx_init(vcmd_mgr);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if(kfifo_alloc(&dev_ctx[i].send_slice_info_queue, SLOT_NUM_CMDBUF * sizeof(u16), GFP_DMA | GFP_KERNEL)) {
			for (j = 0; j < i; ++j)
				kfifo_free(&vcmd_mgr->dev_ctx[j].send_slice_info_queue);
			goto err1;
		}
	}

	sema_init(&vcmd_mgr->isr_polling_sema, 1);

	vcmd_mgr->init_po = create_process_object();
	if (!vcmd_mgr->init_po)
		goto err1;

	result = register_chrdev(vcmd_mgr->hantrovcmd_major, enc_dev_n, &hantrovcmd_fops);
	if (result < 0) {
		venc_init_klog(LOGLVL_ERROR, "vcx_vcmd_driver: unable to get major <%d>\n",
			vcmd_mgr->hantrovcmd_major);
		goto err;
	} else if (result != 0) {
		/* this is for dynamic major */
		vcmd_mgr->hantrovcmd_major = result;
	}
	result = vcmd_reserve_IO(vcmd_mgr);
	if (result < 0)
		goto err;

	result = vcmd_config_modules(vcmd_mgr);
	if (result < 0)
		goto err;

	vce_reset_asic(vcmd_mgr);
	vcmd_reset_asic(vcmd_mgr);

#ifdef SUPPORT_DBGFS
	/* for debugfs */
	if (_dbgfs_init((void *)vcmd_mgr))
		goto err;
	_dbgfs_init_ctx((void *)vcmd_mgr, 0);
#endif

	/* get the IRQ line */
	result = vcmd_reserve_irq(vcmd_mgr);
	if (result < 0) {
		vcmd_release_IO(vcmd_mgr);
		goto err;
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

	venc_init_klog(LOGLVL_INFO, "vcx_vcmd_driver: module inserted. Major <%d>\n",
		vcmd_mgr->hantrovcmd_major);

	*_vcmd_mgr = (void *)vcmd_mgr;

	return 0;

err:
	vcmd_release_IO(vcmd_mgr);
	unregister_chrdev(vcmd_mgr->hantrovcmd_major, enc_dev_n);
err1:
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_vcmd);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_status);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_regs);

	free_process_object(vcmd_mgr->init_po);

	if (vcmd_mgr->dev_ctx)
		vfree(vcmd_mgr->dev_ctx);
	if (vcmd_mgr)
		vfree(vcmd_mgr);
	vcmd_klog(LOGLVL_ERROR, "module not inserted!\n");
	return result;
}

/**
 * @brief de-init vcmd driver.
 */
void hantroenc_vcmd_cleanup(void)
{
	int i = 0;
	u32 result;
	vcmd_mgr_t *vcmd_mgr = vcmd_manager;
	struct hantrovcmd_dev *dev_ctx = vcmd_mgr->dev_ctx;

	_vcmd_kthread_stop(vcmd_mgr);

#ifdef NEED_REQUEST_ARBITER
	// Check if APB arbiter is existed
	if (dev_ctx[0].hw_feature.has_arbiter) {
		vcmd_online_for_arbiter(dev_ctx[0].hwregs);
		vcmd_request_arbiter(dev_ctx[0].hwregs);
	}
#endif

	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		if (!dev_ctx[i].hwregs)
			continue;
		//disable interrupt at first
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

		/* free the vcmd IRQ */
		if (dev_ctx[i].subsys_info->irq != -1)
			free_irq(dev_ctx[i].subsys_info->irq, (void *)vcmd_mgr);
	}

#ifdef SUPPORT_DBGFS
	_dbgfs_cleanup((void *)vcmd_mgr);
#endif
#ifdef NEED_REQUEST_ARBITER
	if (dev_ctx[0].hw_feature.has_arbiter)
		vcmd_offline_for_arbiter(dev_ctx[0].hwregs);
#endif
	vcmd_release_IO(vcmd_mgr);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		kfifo_free(&vcmd_mgr->dev_ctx[i].send_slice_info_queue);
	}
	vfree(dev_ctx);

	//release_vcmd_non_cachable_memory();
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_vcmd);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_status);
	_vcmd_free_mem(vcmd_mgr, &vcmd_mgr->mem_regs);

	unregister_chrdev(vcmd_mgr->hantrovcmd_major, enc_dev_n);

	free_process_object(vcmd_mgr->init_po);
	vfree(vcmd_mgr);
	vcmd_klog(LOGLVL_DEBUG, "module removed\n");
}
