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

#include <linux/version.h>
#include "linux/err.h"
#include "linux/kstrtox.h"
#include "linux/mutex.h"
#include "vdec_power_common.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "hantrodec.h"
#include "vcmdswhwregisters.h"
#include "hantrodec_defs.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/hantrodec_defs.h>
#include <dt-bindings/xring/platform-specific/vpu/vdec/vcmdswhwregisters.h>
#include <dt-bindings/xring/platform-specific/vpu/vdec/hantrodec.h>
#endif
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/mod_devicetable.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#if (KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE)
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/dma-buf.h>
#include <linux/of_device.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/of_reserved_mem.h>
#include <linux/pm_runtime.h>

#include "subsys.h"
#include "hantroaxife.h"
#include "hantroafbc.h"
#include "hantrovcmd_priv.h"
#ifdef SUPPORT_DBGFS
#include <linux/debugfs.h>
#endif
#include <linux/time.h>

#include <linux/iommu.h>
#ifdef FPGA_POWER_ON
#include "power_on_vdec.h"
#endif

#include "vdec_power_base.h"
#include "vdec_power_manager.h"

#include <linux/regulator/consumer.h>
#include <soc/xring/xring_smmu_wrapper.h>
#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>

#include "vdec_mdr.h"
#include "vdec_clk_manager.h"
#include "vdec_log.h"

#undef PDEBUG
#ifdef HANTRODEC_DEBUG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) vdec_klog(LOGLVL_DEBUG, fmt, ##args)
#else
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...)
#endif

/* hantro VC8000D reg config */
#define HANTRO_VCD_REGS                 MAX_REG_COUNT /*VCD total regs*/
#define HANTRO_VCD_FIRST_REG            0
#define HANTRO_VCD_LAST_REG             (HANTRO_VCD_REGS - 1)
#define HANTRODEC_HWBUILD_ID_OFF        (309 * 4)

/* Logic module IRQs */
#define HXDEC_NO_IRQ                    -1

#define MAX(a, b)                       (((a) > (b)) ? (a) : (b))

#define DEC_IO_SIZE_MAX                 (HANTRO_VCD_REGS * 4)

/* User should modify these configuration if do porting to own platform. */
/* Please guarantee the base_addr, io_size, dec_irq belong to same core. */

/* Defines use kernel clk cfg or not**/
//#define CLK_CFG
#ifdef CLK_CFG
/*this id should conform with platform define*/
#define CLK_ID   "hantrodec_clk"
#endif

/* Logic module base address */

#define DEC_IO_SIZE_0                   DEC_IO_SIZE_MAX /* bytes */
#define DEC_IO_SIZE_1                   DEC_IO_SIZE_MAX /* bytes */

#define DEC_IRQ_0                       HXDEC_NO_IRQ
#define DEC_IRQ_1                       HXDEC_NO_IRQ

#define IS_G1(hw_id)                    (((hw_id) == 0x6731) ? 1 : 0)
#define IS_G2(hw_id)                    (((hw_id) == 0x6732) ? 1 : 0)
#define IS_VCD(hw_id)               (((hw_id) == 0x9001) ? 1 : 0)
#define IS_BIGOCEAN(hw_id)              (((hw_id) == 0xB16D) ? 1 : 0)

/* Some IPs HW configuration parameters for APB Filter */
/* Because now such information can't be
 * read from APB filter configuration registers
 */
/* The fixed value have to be used */
#define VCD_NUM_MASK_REG                336
#define VCD_NUM_MODE                    4
#define VCD_MASK_REG_OFFSET             4096
#define VCD_MASK_BITS_PER_REG           1

#define VCDJ_NUM_MASK_REG               332
#define VCDJ_NUM_MODE                   1
#define VCDJ_MASK_REG_OFFSET            4096
#define VCDJ_MASK_BITS_PER_REG          1

#define AV1_NUM_MASK_REG                303
#define AV1_NUM_MODE                    1
#define AV1_MASK_REG_OFFSET             4096
#define AV1_MASK_BITS_PER_REG           1

#define AXIFE_NUM_MASK_REG              144
#define AXIFE_NUM_MODE                  1
#define AXIFE_MASK_REG_OFFSET           4096
#define AXIFE_MASK_BITS_PER_REG         1

#define CORE_MASK_CLIENTTYPE_BITMAP     16

#define DRIVER_NAME                     "xring_vdec"

#define XRING_VPU_DEC                   (1 << 1)

#define VDEC_DEFAULT_SUSPEND_DELAY_MS   (1)
#define UINT32_MAX_BITS                 (32)
/***********local variable declaration********/

static const int DecHwId[] = {
	0x6731, /* G1 */
	0x6732, /* G2 */
	0xB16D, /* BigOcean */
	0x9001 /* VC8000D */
};

/**
 * module param:
 *  base_port        - base address for single core legacy mode
 *  reg_access_opt   - register access option:
 *                     0: access all registers
 *                     1: access specified registers
 *  alloc_size       - alloc memory size
 *  alloc_base       - alloc memory base address
 *  dec_dev_n        - device name
 *  vcmd_isr_polling - the mode to wait for device being aborted
 *                     0: use IRQ mode
 *                     1: use polling ISR mode
 *  vsi_kloglvl      - kernel driver log level, default is LOGLVL_ERROR
 *  vsi_klogMask     - kernel driver log mask, default is VDEC_LOGMASK_DEFAULT
 *  vsi_dumplvl      - dump register log level, default is DUMP_REGISTER_LEVEL_BRIEF_ONERROR
 *  arbiter_urgent      - 0: normal priority
 *                        1: urgent priority
 *  arbiter_weight      - normal priority, it indicates required bandwidth
 *                        urgent priority, it indicates urgent level
 *  arbiter_timewindow  - time window, 2^n cycles
 *  arbiter_bw_overflow - 0: can't overflow
 *                        1: can overflow
 */
static unsigned long base_port = -1;
static unsigned int reg_access_opt;
static unsigned int vcmd;
unsigned long alloc_size = -1;
unsigned long alloc_base = -1;
unsigned int  has_apb_arbiter;
char *dec_dev_n = DRIVER_NAME;
unsigned long vcmd_isr_polling;
int vsi_kloglvl = LOGLVL_WARNING;
ulong vsi_klogMask = VDEC_LOGMASK_DEFAULT;
int vsi_dumplvl = DUMP_REGISTER_LEVEL_BRIEF_ONERROR;
u32 arbiter_weight = 0x1d;
u32 arbiter_urgent;
u32 arbiter_timewindow = 0x1d;
u32 arbiter_bw_overflow;
#ifdef CONFIG_DEC_PM
int always_on;
#endif
#ifdef CLK_CFG
struct clk *clk_cfg;
int is_clk_on;
struct timer_list timer;
#endif
#ifdef CONFIG_DEC_CM_RUNTIME
u32 runtime_cm_enable = 1;
#endif
extern struct vcmd_config vcmd_core_array[MAX_SUBSYS_NUM];

#ifdef SUPPORT_DBGFS
/*debugfs for performance statistics*/
static struct dentry *debug_root;
static int N_reserved = 50;
#define MAX_RESERVED_TIME 256
#define IDLE 0
#define RESERVED 1
#define DECODING 2
#define DONE 3
//static char *subsys_name[MAX_SUBSYS_NUM] = {"reg_subsys0", "reg_subsys1", "reg_subsys2", "reg_subsys3"};
#endif

/* for match platform driver.
 */
static const struct platform_device_info hantro_platform_info = {
	.name = DRIVER_NAME,
	.id = -1,
	.dma_mask = DMA_BIT_MASK(32),
};

static const struct of_device_id vdec_of_match[] = {
	{
		.compatible = "xring,vpu-dec",
	},
	{ /* sentinel */ },
};

static const struct platform_device_id hantro_ids[] = {
	{
		.name = DRIVER_NAME,
	},
	{/* sentinel */},
};

/* module_param(name, type, perm) */
module_param(base_port, ulong, 0);
module_param(reg_access_opt, uint, 0);
module_param(alloc_base, ulong, 0);
module_param(alloc_size, ulong, 0);
module_param(dec_dev_n, charp, 0644);
module_param(vcmd_isr_polling, ulong, 0);
module_param(vsi_kloglvl, int, 0644);
module_param(vsi_klogMask, ulong, 0644);
module_param(vsi_dumplvl, int, 0644);
module_param(arbiter_urgent, uint, 0);
module_param(arbiter_weight, uint, 0);
module_param(arbiter_timewindow, uint, 0);
module_param(arbiter_bw_overflow, uint, 0);
#ifdef CONFIG_DEC_CM_RUNTIME
module_param(runtime_cm_enable, uint, 0644);
#endif
/* here's all the must remember stuff */

/* a watchdog which belongs to each device */
typedef struct {
	struct timer_list timer; //a timer to check if there is no response from hardware.
	u8 active; // 1: watchdog is start
	u8 triggered; // 1: watchdog is triggered
	u32 core_id; // corresponding to which device
} watchdog_t;

typedef struct {
	char *buffer;

	volatile unsigned int iosize[HXDEC_MAX_CORES];
	/* mapped address to different HW cores regs*/
	volatile u8 *hwregs[HXDEC_MAX_CORES][HW_CORE_MAX];
	/* mapped address to different HW cores regs*/
	volatile u8 *apbfilter_hwregs[HXDEC_MAX_CORES][HW_CORE_MAX];
	volatile int irq[HXDEC_MAX_CORES];
	int hw_id[HXDEC_MAX_CORES][HW_CORE_MAX];
	/* Requested client type for given core,
	 * used when a subsys has multiple
	 * decoders, e.g., VCD+VCDJ+BigOcean
	 */
	int cores;
	struct fasync_struct *async_queue_dec;
	struct fasync_struct *async_queue_pp;
	watchdog_t watchdog[HXDEC_MAX_CORES];
#ifdef CONFIG_DEC_PM
	int hw_active[HXDEC_MAX_CORES];
#endif
} hantrodec_t;

struct core_cfg {
	u32 cfg[HXDEC_MAX_CORES];              /* indicate the supported format */
};

static struct SubsysMgr {
	struct platform_device *platformdev;
	/* for non-vcmd */
	unsigned long multicorebase[HXDEC_MAX_CORES];
	int irq[HXDEC_MAX_CORES];
	int iosize[HXDEC_MAX_CORES];
	struct subsys_config vpu_subsys[MAX_SUBSYS_NUM];
	struct apbfilter_cfg apbfilter_cfg[MAX_SUBSYS_NUM][HW_CORE_MAX];
	struct axife_cfg axife_cfg[MAX_SUBSYS_NUM];
	int elements;
	hantrodec_t hantrodec_data; /* dynamic allocation? */
	int hantrodec_major; /* dynamic allocation */

	u32 dec_regs[HXDEC_MAX_CORES][DEC_IO_SIZE_MAX / 4];
	u32 apbfilter_regs[HXDEC_MAX_CORES][DEC_IO_SIZE_MAX / 4 + 1];
	/* shadow_regs used to compare whether it's necessary to write to registers */
	u32 shadow_dec_regs[HXDEC_MAX_CORES][DEC_IO_SIZE_MAX / 4];

	struct semaphore dec_core_sem;
	struct semaphore pp_core_sem;

	int dec_irq;
	int pp_irq;

	atomic_t irq_rx;
	atomic_t irq_tx;

	struct file *dec_owner[HXDEC_MAX_CORES];
	struct file *pp_owner[HXDEC_MAX_CORES];
	struct core_cfg config;

	spinlock_t owner_lock;

	wait_queue_head_t dec_wait_queue;
	wait_queue_head_t pp_wait_queue;
	wait_queue_head_t hw_queue;

	struct task_struct *kthread;
	u8 stop_kthread;
	wait_queue_head_t kthread_waitq;

	/* for VCMD */
	void *vcmd_mgr;   /* allocated in hantro_vcmd.c */
#ifdef SUPPORT_DBGFS
	/* for debugfs */
	unsigned long hw_return_time[HXDEC_MAX_CORES][MAX_RESERVED_TIME];
	unsigned long reserved_time[HXDEC_MAX_CORES][MAX_RESERVED_TIME];
	unsigned long start_hw_time[HXDEC_MAX_CORES][MAX_RESERVED_TIME];
	unsigned long release_time[HXDEC_MAX_CORES][MAX_RESERVED_TIME];
	int r_index[HXDEC_MAX_CORES], hw_r_index[HXDEC_MAX_CORES], start_hw_index[HXDEC_MAX_CORES], release_index[HXDEC_MAX_CORES];
	int decode_state[HXDEC_MAX_CORES];
#endif
} subsys_mgr;

static int ReserveIO(void);
static void ReleaseIO(void);
static void ResetAsic(hantrodec_t *dev);

#ifdef HANTRODEC_DEBUG
static void dump_regs(hantrodec_t *dev);
#endif

/* IRQ handler */
static irqreturn_t hantrodec_isr(int irq, void *dev_id);

static int CoreHasFormat(const u32 *cfg, int core, u32 format);
extern int memmap_set_device(struct device *dev, int flag);

/**
 * @brief stop vcd normall
 */
int abort_vcd(volatile u8 *reg_base)
{
	u32 status;

	status = (u32)ioread32((void __iomem *)(reg_base + HANTRODEC_IRQ_STAT_DEC_OFF));
	vcmd_klog(LOGLVL_INFO, "abort vcd, status=0x%x\n", status);
	if (status & 0x1) {
		//abort vcd
		status |= HANTRODEC_DEC_ABORT;
		iowrite32(status, (void __iomem *)(reg_base + HANTRODEC_IRQ_STAT_DEC_OFF));

		return 1;
	}


	return 0;
}

#ifdef SUPPORT_WATCHDOG
/**
 * @brief hook function for system-driver to do further process
 *  for tiggered watchdog.
 */
static void hook_watchdog(void *_dev, int succeed)
{
	if (succeed) {
		vdec_klog(LOGLVL_ERROR, "hantrodec: flush succeed!!");
	} else {
		vdec_klog(LOGLVL_ERROR, "hantrodec: flush failed, need to re-power sub-system");
	}
}

/**
 * @brief stop vcd when watchdog triggered
 */
int watchdog_stop_vcd(volatile u8 *reg_base)
{
	unsigned long flags;
	u32 status;

	if (abort_vcd(reg_base)) {
		mdelay(10); //delay 10ms
		status = (u32)ioread32((void __iomem *)(reg_base + HANTRODEC_IRQ_STAT_DEC_OFF));
		//Stop VCD by setting reg1 bit0 to 0.
		if ((status & 0x1) == 0)
			iowrite32(0, (void __iomem *)(reg_base + HANTRODEC_IRQ_STAT_DEC_OFF));
	}

	return 0;
}

/**
 * @brief process when watchdog triggered.
 */
static void watchdog_process(watchdog_t *watchdog)
{
	hantrodec_t *dev = &subsys_mgr.hantrodec_data;
	u32 core_id = watchdog->core_id;
	int succeed = 1;
	unsigned long flags;

	if (core_id >= subsys_mgr.hantrodec_data.cores)
		return;
	spin_lock_irqsave(&subsys_mgr.owner_lock, flags);
	watchdog_stop_vcd(dev->hwregs[core_id][HW_VCD]);
	if (AXIFEFlush(dev->hwregs[core_id][HW_AXIFE]) == -1)
		succeed = 0;
	hook_watchdog(dev, succeed);
	spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);
}
#endif //SUPPORT_WATCHDOG

/**
 * @brief To check subsys_mgr/dev's actions which need kthread to process
 * @return int: 0: no actions; 1: have actions
 */
static int _kthread_actions(watchdog_t **watchdog)
{
	int ret = 0, i;

	if (subsys_mgr.stop_kthread == 1) {
		return 1;
	}

	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		*watchdog = &subsys_mgr.hantrodec_data.watchdog[i];
		if ((*watchdog)->triggered == 1) {
			ret = 1;
			break;
		} else {
			ret = 0;
			/*TODO*/
		}
	}

	return ret;
}

/**
 * @brief hantrodec kernel thread main function
 */
static int _kthread_fn(void *data)
{
	watchdog_t *watchdog = NULL;

	while (!kthread_should_stop()) {
		if (wait_event_interruptible(subsys_mgr.kthread_waitq,
				_kthread_actions(&watchdog))) {
			vdec_klog(LOGLVL_ERROR, "hantrodec: signaled!!!\n");
			return -ERESTARTSYS;
		}

		if (watchdog == NULL) {
			continue;
		}
#ifdef SUPPORT_WATCHDOG
		if (watchdog->triggered == 1) {
			watchdog->triggered = 0;
			watchdog_process(watchdog);
			continue;
		}
#endif
	}

	return 0;
}

/**
 * @brief wake up hantrodec kernel thread
 */
static void hantrodec_kthread_wakeup(void)
{

	if (IS_ERR(subsys_mgr.kthread))
		return;

	wake_up_interruptible_all(&subsys_mgr.kthread_waitq);
}

/**
 * @brief create kernel thread for hantrodec driver
 */
static void hantrodec_kthread_create(void)
{
	subsys_mgr.stop_kthread = 0;
	init_waitqueue_head(&subsys_mgr.kthread_waitq);
	subsys_mgr.kthread =
		kthread_run(_kthread_fn, NULL, "hantrodec_kthread");
	if (IS_ERR(subsys_mgr.kthread)) {
		vdec_klog(LOGLVL_ERROR, "hantrodec: create hantrodec kthread failed\n");
		return;
	}
}

/**
 * @brief stop kernel thread hantrodec driver
 */
static void hantrodec_kthread_stop(void)
{
	if (!IS_ERR(subsys_mgr.kthread)) {
		subsys_mgr.stop_kthread = 1;
		kthread_stop(subsys_mgr.kthread);
		subsys_mgr.kthread = NULL;
	}
}

#ifdef SUPPORT_WATCHDOG
/**
 * @brief timer callback function of watchdog
 */
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
static void _watchdog_cb(unsigned long arg)
#else
static void _watchdog_cb(struct timer_list *timer)
#endif
{
	struct timer_list *watchdog_timer;
	watchdog_t *watchdog;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	watchdog_timer = (struct timer_list *)arg;
#else
	watchdog_timer = timer;
#endif

	watchdog = container_of(watchdog_timer, watchdog_t, timer);

	watchdog->triggered = 1;
	hantrodec_kthread_wakeup();
}

/**
 * @brief start watchdog
 */
static void _watchdog_start(watchdog_t *watchdog)
{
	struct timer_list *timer = &watchdog->timer;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	init_timer(timer);
	timer->function = _watchdog_cb;
	timer->data = (unsigned long)timer;
#else
	timer_setup(timer, _watchdog_cb, 0);
#endif
	watchdog->active = 1;
}

/**
 * stop watchdog
 */
static void _watchdog_stop(watchdog_t *watchdog)
{
	if (watchdog->active) {
		del_timer(&watchdog->timer);
		watchdog->active = 0;
	}
}

/**
 * feed and start watchdog
 */
static void _watchdog_feed(watchdog_t *watchdog)
{
	if (watchdog->active == 0) {
		_watchdog_start(watchdog);
	}
	mod_timer(&watchdog->timer,
		(jiffies + HZ * ONE_JOB_WAIT_TIME / 1000));
}
#endif // SUPPORT_WATCHDOG

static int vdec_qos_cfg(void)
{
	struct vpu_qos_map *qos_priv = NULL;
	void __iomem *vdec_axi_vaddr = NULL;
	unsigned int qos_temp;
	int i = 0;

	qos_priv = (struct vpu_qos_map *)xring_flowctrl_get_cfg_table(MST_VPU_QOS_ID);
	if (qos_priv == NULL) {
		vdec_klog(LOGLVL_ERROR, "vdec qos_cfg_table get failed\n");
		return -EINVAL;
	}

	vdec_axi_vaddr = ioremap(ACPU_AXIFE_VDEC, ACPU_AXIFE_VDEC_SIZE);
	if (vdec_axi_vaddr == NULL) {
		vdec_klog(LOGLVL_ERROR, "vdec axi ioremap failed\n");
		return -EINVAL;
	}

	for (i = VDEC_AXI_REM_RQOS_ID; i <= VDEC_RDCH1_WQOS_ID; i++) {
		writel(qos_priv[i].val, vdec_axi_vaddr + qos_priv[i].offset);
		if ((unsigned int)readl(vdec_axi_vaddr + qos_priv[i].offset) != qos_priv[i].val) {
			vdec_klog(LOGLVL_ERROR, "failed to write qos\n");
			iounmap((void __iomem *)vdec_axi_vaddr);
			return -EINVAL;
		}
	}

	iounmap((void __iomem *)vdec_axi_vaddr);
	vdec_klog(LOGLVL_VERBOSE, "success to configure vdec qos\n");
	return 0;
}

int vdec_core_hw_config(vdec_power_mgr *pm)
{
	int ret = 0;
	struct hantrodec_dev *dec_dev;
	struct hantrovcmd_dev *vcmd_dev = NULL;
	vcmd_mgr_t *vcmd_mgr = NULL;
	unsigned long flag;
	int i;

	if (!pm->runtime_init) {
		return ret;
	}

	dec_dev = dev_get_drvdata(pm->dev);
	WARN_ON(!dec_dev);

	vcmd_mgr = (vcmd_mgr_t *)dec_dev->priv_data;
	WARN_ON(!vcmd_mgr);

	ret = vdec_qos_cfg();
	if (ret) {
		vdec_klog(LOGLVL_ERROR, "Failed to vdec qos cfg\n");
		return ret;
	}
	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++)
		AXIFEEnable(subsys_mgr.hantrodec_data.hwregs[i][HW_AXIFE]);
	vcmd_reset_asic(dec_dev->priv_data);
	for (i = 0; i < vcmd_mgr->subsys_num; i++) {
		vcmd_dev = &vcmd_mgr->dev_ctx[i];
		WARN_ON(!vcmd_dev);

		spin_lock_irqsave(vcmd_dev->spinlock, flag);
		vcmd_dev->state = VCMD_STATE_POWER_ON;
		spin_unlock_irqrestore(vcmd_dev->spinlock, flag);
	}
	return ret;
}

static int runtime_power_get(vdec_power_mgr *pm)
{
	int ret = 0;
	unsigned long flags;
	vdec_sr_status_e sr_status;

	if (!pm) {
		vdec_klog(LOGLVL_ERROR, "invalid param:%p\n", pm);
		return -EINVAL;
	}

	ret = vdec_core_do_power_on(pm);
	if (ret) {
		vdec_klog(LOGLVL_ERROR, "vdec_core_do_power_on failed:%d\n", ret);
		return ret;
	}
	return ret;
}

static void runtime_power_put(vdec_power_mgr *pm)
{
	if (!pm) {
		vdec_klog(LOGLVL_ERROR, "invalid param:%p\n", pm);
		return;
	}

	vdec_core_do_power_off(pm);
}

static int inst_power_get(vdec_power_mgr *pm)
{
	int ret;
	unsigned long flags;
	vdec_sr_status_e sr_status;

	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid param:%p\n", pm);
		return -EINVAL;
	}

	spin_lock_irqsave(&pm->pm_sr_lock, flags);
	sr_status = pm->sr_status;
	spin_unlock_irqrestore(&pm->pm_sr_lock, flags);

	vdec_pm_klog(LOGLVL_DEBUG, "inst POWER GET SR STATUS %d, pm reset %d\n", sr_status, pm->reset);

	if (sr_status == POWER_IS_RESUMING || pm->reset)
		ret = vdec_force_power_on(pm, POWER_VDEC_SUBSYS);
	else
		ret = vdec_power_on(pm, POWER_VDEC_SUBSYS);

	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed to power on vdec_subsys: %d\n", ret);
	}

	return ret;
}

static void inst_power_put(vdec_power_mgr *pm)
{
	unsigned long flags;
	vdec_sr_status_e sr_status;

	if (!pm) {
		vdec_pm_klog(LOGLVL_ERROR, "invalid param:%p\n", pm);
		return;
	}

	spin_lock_irqsave(&pm->pm_sr_lock, flags);
	sr_status = pm->sr_status;
	spin_unlock_irqrestore(&pm->pm_sr_lock, flags);

	vdec_pm_klog(LOGLVL_DEBUG, "inst POWER PUT SR STATUS %d, pm reset %d\n",
			sr_status, pm->reset);

	if (sr_status == POWER_IS_SUSPENDING || pm->reset)
		vdec_force_power_off(pm, POWER_VDEC_SUBSYS);
	else
		vdec_power_off(pm, POWER_VDEC_SUBSYS);
}

/*
* if user haven't close device yet before rmmod ko, then some domain power is enable,
* so we need force cleanup the pm->power_count to zero, and do power down for
* specified domain.
*/
static void release_power_at_remove(vdec_power_mgr *pm, vdec_power_domain_e pd)
{

	mutex_lock(&pm->pm_lock);

	int tmp = atomic_read(&pm->power_count[pd]);
	while (tmp > 0) {
		vdec_pm_klog(LOGLVL_WARNING, "regulator domain[%d] not disabled here, do power put!\n", pd);
		if (pd == POWER_VDEC_CORE)
			vdec_core_do_power_off(pm);
		else if (pd == POWER_VDEC_SUBSYS)
			inst_power_put(pm);
		// break for avoid busy loop when power_count is not decreased.
		if (atomic_read(&pm->power_count[pd]) == tmp)
			break;
		tmp = atomic_read(&pm->power_count[pd]);
	}

	mutex_unlock(&pm->pm_lock);
}

static int _power_always_on_all(vdec_power_mgr *pm)
{
	int ret = 0;
	WARN_ON(!pm);

	/* power on subsys and smmu, count++ */
	ret = vdec_power_on(pm, POWER_VDEC_SUBSYS);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed to power on vdec_subsys: %d\n", ret);
		return ret;
	}
	/* force power on vdec core */
	ret = vdec_force_power_on(pm, POWER_VDEC_CORE);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Failed to power on vdec_core: %d\n", ret);
		return ret;
	}
	return ret;
}

#if defined(CONFIG_DEC_PM) && defined(CONFIG_DEC_PM_DEBUG)
static int _always_on_enable(vdec_power_mgr *pm)
{
	int ret = 0;
	WARN_ON(!pm);

	if (!atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		!atomic_read(&pm->power_count[POWER_VDEC_SUBSYS]) &&
		!regulator_is_enabled(pm->vdec_rg)) {
		/* power on all vdec hardware resource */
		ret = _power_always_on_all(pm);
		if (ret) {
			vdec_pm_klog(LOGLVL_ERROR, "Failed to _power_always_on_all: %d\n", ret);
			return ret;
		}
	} else if (!atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_WARNING, "the vdec regulator status[enable] abnormal. enable_count:%d\n",
			regulator_count_voltages(pm->vdec_rg));
	} else if (!atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		atomic_read(&pm->power_count[POWER_VDEC_SUBSYS])) {
		vdec_pm_klog(LOGLVL_DEBUG, "the count of core/subsys[0/1]. core will be power on by runtime!\n");
	} else if (atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		!regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_WARNING, "the vdec regulator status[disable] abnormal. enable count:%d\n",
			regulator_count_voltages(pm->vdec_rg));
	} else if (atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_DEBUG, "power already status on!\n");
	}

	return ret;
}

static void _always_on_disable(vdec_power_mgr *pm)
{
	WARN_ON(!pm);

	if (atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_DEBUG, "power status on!\n");
	} else if (atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		!regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_WARNING, "the vdec regulator status[disable] abnormal. enable_count:%d\n",
			regulator_count_voltages(pm->vdec_rg));
	} else if (!atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		regulator_is_enabled(pm->vdec_rg)) {
		/* power off subsys and smmu */
		vdec_force_power_off(pm, POWER_VDEC_CORE);
		/* power off subsys and smmu if rg_is_enable, count-- */
		vdec_power_off(pm, POWER_VDEC_SUBSYS);
	} else if (!atomic_read(&pm->power_count[POWER_VDEC_CORE]) &&
		!regulator_is_enabled(pm->vdec_rg)) {
		vdec_pm_klog(LOGLVL_WARNING, "the vdec regulator status[disable] abnormal in always_on. enable_count:%d\n",
			regulator_count_voltages(pm->vdec_rg));
	}
}

static int always_on_param_set(const char *val, const struct kernel_param *kp)
{
	int ret;
	int *aon_val = (int *)kp->arg;

	WARN_ON(!subsys_mgr.platformdev);
	struct hantrodec_dev *dec_dev = platform_get_drvdata(subsys_mgr.platformdev);

	if (!dec_dev || !dec_dev->pm) {
		vdec_pm_klog(LOGLVL_ERROR, "pm or dec_dev[%p] is NULL!\n", dec_dev);
		return -EINVAL;
	}
	mutex_lock(&dec_dev->pm->pm_lock);
	ret = kstrtoint(val, 10, aon_val);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "Invalid parameter value:%s\n", val);
		mutex_unlock(&dec_dev->pm->pm_lock);
		return -EINVAL;
	}

	if (*aon_val != 0 && *aon_val != 1) {
		vdec_pm_klog(LOGLVL_ERROR, "set value must be zero or one, now: %d\n", *aon_val);
		mutex_unlock(&dec_dev->pm->pm_lock);
		return -EINVAL;
	}

	vdec_pm_klog(LOGLVL_INFO, "set vdec power always on[%d][%d] to: %d\n", always_on, dec_dev->pm->cur_always_on, *aon_val);
	if (!dec_dev->pm->cur_always_on) {
		/* 0=>1 */
		if (*aon_val) {
			vdec_pm_klog(LOGLVL_DEBUG, "always_on will be enabled. core count now:%d\n",
				atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]));

			ret = _always_on_enable(dec_dev->pm);
			if (ret) {
				vdec_pm_klog(LOGLVL_ERROR, "Failed to enable always_on: %d\n", ret);
				mutex_unlock(&dec_dev->pm->pm_lock);
				return ret;
			}
		} else {
			/* 0=>0 */
			vdec_pm_klog(LOGLVL_DEBUG, "always on[0] unchanged.\n");
		}
	} else {
		/* 1=>0 */
		if (!*aon_val) {
			vdec_pm_klog(LOGLVL_DEBUG, "always_on will be disabled. core count now:%d\n",
				atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]));

			_always_on_disable(dec_dev->pm);
		} else {
			/* 1=>1 */
			vdec_pm_klog(LOGLVL_DEBUG, "always on[1] unchanged.\n");
		}
	}
	dec_dev->pm->cur_always_on = always_on;

	mutex_unlock(&dec_dev->pm->pm_lock);
	return 0;
}

static struct kernel_param_ops always_on_param_cb = {
	.set = always_on_param_set,
	.get = param_get_int
};

module_param_cb(always_on, &always_on_param_cb, &always_on, 0644);
#endif

#ifdef CONFIG_DEC_PM
static long DecRestoreRegs(hantrodec_t *dev, u32 id)
{
	long i;

	/* write all regs to hardware */
	for (i = 1; i < HANTRO_VCD_REGS; i++)
		iowrite32(subsys_mgr.shadow_dec_regs[id][i],
			(void __iomem *)(dev->hwregs[id][HW_VCD] + i*4));

	return 0;
}

static long DecStoreRegs(hantrodec_t *dev, u32 id)
{
	long i;

	/* read all registers from hardware */
	for (i = 0; i < HANTRO_VCD_REGS; i++)
		subsys_mgr.shadow_dec_regs[id][i] =
			ioread32((void __iomem *)(dev->hwregs[id][HW_VCD] + i*4));

	return 0;
}

static int hantrodec_pm_runtime_init(struct platform_device *pdev, vdec_power_mgr *pm)
{
	int result = 0;

	pm_runtime_set_autosuspend_delay(&pdev->dev, VDEC_DEFAULT_SUSPEND_DELAY_MS); // 1 ms
	pm_runtime_use_autosuspend(&pdev->dev);
	//pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	if (!pm_runtime_enabled(&pdev->dev)) {
		vdec_klog(LOGLVL_ERROR, "pm_runtime not enabled\n");
		result = -EINVAL;
	} else if (atomic_read(&pdev->dev.power.usage_count)) {
		vdec_klog(LOGLVL_ERROR, "vdec runtime usage count unexpectedly non zero %d",
				atomic_read(&pdev->dev.power.usage_count));
		result = -EINVAL;
	}
	pm->runtime_init = true;
	return result;
}

static void hantrodec_pm_runtime_deinit(struct platform_device *pdev)
{
	if (atomic_read(&pdev->dev.power.usage_count))
		vdec_klog(LOGLVL_ERROR, "vdec runtime usage count unexpectedly non zero %d",
				atomic_read(&pdev->dev.power.usage_count));

	pm_runtime_disable(&pdev->dev);
}

static int dec_pm_suspend(void *handler)
{
	u32 i;
	hantrodec_t *dev = (hantrodec_t *)handler;

	for (i = 0; i < dev->cores; i++) {
		if (subsys_mgr.dec_owner[i]) {
			/* polling until hw is idle */
			while (dev->hw_active[i]) {
				usleep_range(5000, 10000);
			}

			/* let's backup all registers from H/W to shadow register to support suspend */
			DecStoreRegs(dev, i);
		}
	}

	return 0;
}

int dec_pm_resume(void *handler)
{
	int i;
	hantrodec_t *dev = (hantrodec_t *)handler;

	for (i = 0; i < dev->cores; i++) {
		if (subsys_mgr.dec_owner[i]) {
			/* let's restore registers from shadow register to H/W to support resume */
			DecRestoreRegs(dev, i);
		}
	}

	return 0;
}

static int hantrodec_pm_suspend(struct device *dev)
{
	struct hantrodec_dev *dec_dev;
	int ret = 0;
	unsigned long flags;

	dec_dev = dev_get_drvdata(dev);
	WARN_ON(!dec_dev);
	vdec_pm_klog(LOGLVL_INFO, "device suspend enter!\n");

	mutex_lock(&dec_dev->pm->pm_lock);

	spin_lock_irqsave(&dec_dev->pm->pm_sr_lock, flags);
	dec_dev->pm->sr_status = POWER_IS_SUSPENDING;
	spin_unlock_irqrestore(&dec_dev->pm->pm_sr_lock, flags);

	if ((atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]) ||
		dec_dev->pm->cur_always_on)) {
		if (vcmd)
			ret = vcmd_pm_suspend(dec_dev->priv_data);
		else
			ret = dec_pm_suspend(dec_dev->priv_data);

		vdec_core_do_power_off(dec_dev->pm);
	}
	/* force subsys and smmu power off when the instance is power on */
	if (atomic_read(&dec_dev->pm->power_count[POWER_VDEC_SUBSYS]))
		inst_power_put(dec_dev->pm);

	spin_lock_irqsave(&dec_dev->pm->pm_sr_lock, flags);
	dec_dev->pm->sr_status = POWER_IS_SUSPENDED;
	spin_unlock_irqrestore(&dec_dev->pm->pm_sr_lock, flags);

	mutex_unlock(&dec_dev->pm->pm_lock);

	vdec_pm_klog(LOGLVL_WARNING, "device suspend done!\n");
	return ret;
}

static int hantrodec_pm_resume(struct device *dev)
{
	struct hantrodec_dev *dec_dev;
	int ret = 0;
	unsigned long flags;

	dec_dev = dev_get_drvdata(dev);
	WARN_ON(!dec_dev);
	vdec_pm_klog(LOGLVL_INFO, "device resume enter!\n");
	/* SR resume power up */
	mutex_lock(&dec_dev->pm->pm_lock);

	spin_lock_irqsave(&dec_dev->pm->pm_sr_lock, flags);
	dec_dev->pm->sr_status = POWER_IS_RESUMING;
	spin_unlock_irqrestore(&dec_dev->pm->pm_sr_lock, flags);

	/* if power count is non-zero before pm_suspend, then force power on to recover state */
	if (atomic_read(&dec_dev->pm->power_count[POWER_VDEC_SUBSYS]) > 0) {
		ret = inst_power_get(dec_dev->pm);
		if (ret) {
			mutex_unlock(&dec_dev->pm->pm_lock);
			vdec_pm_klog(LOGLVL_ERROR, "SR resume inst power up failed!\n");
			return ret;
		}
	}
	if (dec_dev->pm->cur_always_on || atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]) > 0) {
		ret = vdec_core_do_power_on(dec_dev->pm);
		if (ret) {
			mutex_unlock(&dec_dev->pm->pm_lock);
			vdec_pm_klog(LOGLVL_ERROR, "SR resume vdec_core power up failed!\n");
			return ret;
		}

		if (vcmd)
			ret = vcmd_pm_resume(dec_dev->priv_data);
		else
			ret = dec_pm_resume(dec_dev->priv_data);
	}

	spin_lock_irqsave(&dec_dev->pm->pm_sr_lock, flags);
	dec_dev->pm->sr_status = POWER_IS_NONSR;
	spin_unlock_irqrestore(&dec_dev->pm->pm_sr_lock, flags);

	mutex_unlock(&dec_dev->pm->pm_lock);

	vdec_pm_klog(LOGLVL_WARNING, "device resume done!\n");
	return ret;
}

static int hantrodec_pm_runtime_suspend(struct device *dev)
{
	struct hantrodec_dev *dec_dev = dev_get_drvdata(dev);

	WARN_ON(!dec_dev);
	mutex_lock(&dec_dev->pm->pm_lock);
	if (atomic_read(&dev->power.usage_count) > 0 || dev->power.runtime_status != RPM_SUSPENDING) {
		vdec_pm_klog(LOGLVL_INFO, "abort suspending! usage_count %d, runtime_status %d\n",
				atomic_read(&dev->power.usage_count), dev->power.runtime_status);
		mutex_unlock(&dec_dev->pm->pm_lock);
		return -EBUSY;
	}
	runtime_power_put(dec_dev->pm);
	mutex_unlock(&dec_dev->pm->pm_lock);
	vdec_pm_klog(LOGLVL_DEBUG, "vdec runtime suspend done!\n");
	return 0;
}

static int hantrodec_pm_runtime_resume(struct device *dev)
{
	int ret = 0;
	struct hantrodec_dev *dec_dev = dev_get_drvdata(dev);

	WARN_ON(!dec_dev);
	mutex_lock(&dec_dev->pm->pm_lock);
	if (atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]) != 0 ||
			atomic_read(&dec_dev->pm->power_count[POWER_VDEC_SUBSYS]) <= 0) {
		vdec_pm_klog(LOGLVL_INFO, "abort resuming! usage_count %d, runtime_status %d\n",
				atomic_read(&dev->power.usage_count), dev->power.runtime_status);
		mutex_unlock(&dec_dev->pm->pm_lock);
		return 0;
	}

	ret = runtime_power_get(dec_dev->pm);
	if (ret) {
		vdec_pm_klog(LOGLVL_ERROR, "runtime_power_get failed: %d!\n", ret);
		mutex_unlock(&dec_dev->pm->pm_lock);
		return ret;
	}
	mutex_unlock(&dec_dev->pm->pm_lock);
	vdec_pm_klog(LOGLVL_DEBUG, "vdec runtime resume done!\n");
	return 0;
}

static const struct dev_pm_ops hantrodec_pm_ops = {
	SET_RUNTIME_PM_OPS(hantrodec_pm_runtime_suspend, hantrodec_pm_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(hantrodec_pm_suspend, hantrodec_pm_resume)
};
#endif

#ifdef SUPPORT_DBGFS
static char *subsys_state_read(int i)
{
	if (subsys_mgr.decode_state[i] == RESERVED)
		return "reserved\n";
	else if (subsys_mgr.decode_state[i] ==  IDLE)
		return "idle\n";
	else if (subsys_mgr.decode_state[i] == DECODING)
		return "decoding\n";
	else
		return "done\n";
}

static ssize_t subsys_state(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char *val, *v;
	int ret, i;

	val = kmalloc(80, GFP_KERNEL);
	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		sprintf(val, "core[%d] work state\n", i);
		v = subsys_state_read(0);
		strcat(val, v);
	}
	ret = simple_read_from_buffer(user_buf, count, ppos, val, strlen(val));
	kfree(val);
	return ret;
}

const struct file_operations fileop_subsys_state = {
	.read = subsys_state,
};

static ssize_t Hw_Register_Print(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char *v0, v1[30];
	int num_regs = 0, NUM_REGS = 0;
	int swreg_mes, j, i, k, ret = 0;
	hantrodec_t hantrodec_data;

	hantrodec_data = subsys_mgr.hantrodec_data;
	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		for (j = HW_VCD; j < HW_CORE_MAX - 1; j++)
			num_regs += subsys_mgr.vpu_subsys[i].submodule_iosize[j] / 4;
	}
	v0 = kzalloc(num_regs * 100, GFP_KERNEL);
	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		sprintf(v1, "core[%d]\n", i);
		strcat(v0, v1);
		for (k = HW_VCD; k < HW_CORE_MAX - 1; k++) {
			if (k == HW_VCD && hantrodec_data.hwregs[i][HW_VCD]) {
				sprintf(v1, "[VCD]\n");
				strcat(v0, v1);
				for (j = 0; j < MAX_REG_COUNT;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCD] + j * 4));
					sprintf(v1, "%d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < MAX_REG_COUNT) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCD] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < MAX_REG_COUNT) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCD] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < MAX_REG_COUNT) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCD] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
						j++;
					}
				}
			}
			if (k == HW_VCDJ && hantrodec_data.hwregs[i][HW_VCDJ]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "[VCDJ]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCDJ] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCDJ] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCDJ] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_VCDJ] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
					}
					j++;
				}
			}
			if (k == HW_BIGOCEAN && hantrodec_data.hwregs[i][HW_BIGOCEAN]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[BIGOCEAN]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_BIGOCEAN] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_BIGOCEAN] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_BIGOCEAN] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_BIGOCEAN] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
					}
					j++;
				}
			}
			if (k == HW_MMU && hantrodec_data.hwregs[i][HW_MMU]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[MMU]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_MMU] + j * 4));
					sprintf(v1, "%d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_MMU] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_MMU] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][HW_MMU] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
						j++;
					}
				}
			}
			if (k == HW_MMU_WR && hantrodec_data.hwregs[i][HW_MMU_WR]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[MMU_WR]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
					}
					j++;
				}
			}
			if (k == HW_DEC400 && hantrodec_data.hwregs[i][HW_DEC400]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[DEC400]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
						j++;
					}
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
						j++;
					}
				}
			}
			if (k == HW_AXIFE && hantrodec_data.hwregs[i][HW_AXIFE]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[AXIFE]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
					}
					j++;
				}
			}
			if (k == HW_AFBC && hantrodec_data.hwregs[i][HW_AFBC]) {
				NUM_REGS = subsys_mgr.vpu_subsys[i].submodule_iosize[k] / 4;
				sprintf(v1, "\n[AFBC]\n");
				strcat(v0, v1);
				for (j = 0; j < NUM_REGS;) {
					swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
					sprintf(v1, "%3d: %08x ", j, swreg_mes);
					strcat(v0, v1);
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x ", swreg_mes);
						strcat(v0, v1);
					}
					j++;
					if (j < NUM_REGS) {
						swreg_mes = ioread32((void __iomem *)(hantrodec_data.hwregs[i][k] + j * 4));
						sprintf(v1, "%08x\n", swreg_mes);
						strcat(v0, v1);
					}
					j++;
				}
			}
		}
	}
	ret = simple_read_from_buffer(user_buf, count, ppos, v0, strlen(v0));
	kfree(v0);
	return ret;
}

const struct file_operations fileop_hw_reg_print = {
	.read = Hw_Register_Print,
};

static ssize_t hw_cycles(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char *v0;
	char v1[30];
	u32 swreg_mes = 0;
	int i;
	int ret;

	v0 = kzalloc(subsys_mgr.hantrodec_data.cores * 30, GFP_KERNEL);
	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		if (subsys_mgr.hantrodec_data.hwregs[i][HW_VCD])
			swreg_mes = ioread32((void __iomem *)(subsys_mgr.hantrodec_data.hwregs[i][HW_VCD] + 63 * 4));
		if (subsys_mgr.hantrodec_data.hwregs[i][HW_VCDJ])
			swreg_mes = ioread32((void __iomem *)(subsys_mgr.hantrodec_data.hwregs[i][HW_VCDJ] + 63 * 4));
		sprintf(v1, "hw cycles(swreg 63) = %u", swreg_mes);
		strcat(v0, v1);
	}
	ret = simple_read_from_buffer(user_buf, count, ppos, v0, strlen(v0));
	kfree(v0);
	return ret;
}

const struct file_operations fileop_cycles = {
	.read = hw_cycles,
};

static ssize_t perf_statistic_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
	char In_fo[10];
	int bytes_to_read = 4;

	simple_write_to_buffer(In_fo, count, ppos, user_buf, bytes_to_read);
	kstrtoint(In_fo, 10, &N_reserved);
	//sscanf(In_fo, "%d", &N_reserved);

	return count;
}

static ssize_t perf_statistic_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	char v[200], *t;
	int i, j, index, ret;

	t = kmalloc(N_reserved * 60 * subsys_mgr.hantrodec_data.cores, GFP_KERNEL);
	for (j = 0; j < subsys_mgr.hantrodec_data.cores; j++) {
		sprintf(v, "core[%d]\n", j);
		strcat(t, v);
		strcat(t, "[reserved time, enable time - reserved time, done_time - reserved time, release_time - reserved time] /ms\n");
		index = subsys_mgr.r_index[j];
		if (index >= N_reserved) {
			for (i = 0; i < N_reserved; i++) {
				sprintf(v, "[%lu, ", subsys_mgr.reserved_time[j][i + index - N_reserved]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.start_hw_time[j][i + index - N_reserved] - subsys_mgr.reserved_time[j][i + index - N_reserved]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.hw_return_time[j][i + index - N_reserved] - subsys_mgr.reserved_time[j][i + index - N_reserved]);
				strcat(t, v);
				sprintf(v, "%lu]\n ", subsys_mgr.release_time[j][i + index - N_reserved] - subsys_mgr.reserved_time[j][i + index - N_reserved]);
				strcat(t, v);
			}
		} else {
			for (i = 0; i < index; i++) {
				sprintf(v, "[%lu, ", subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.start_hw_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.hw_return_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu]\n", subsys_mgr.release_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
			}

			for (i = MAX_RESERVED_TIME - 1; i > MAX_RESERVED_TIME - 1 - (N_reserved - index); i--) {
				sprintf(v, "[%lu, ", subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.start_hw_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu, ", subsys_mgr.hw_return_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
				sprintf(v, "%lu]\n", subsys_mgr.release_time[j][i] - subsys_mgr.reserved_time[j][i]);
				strcat(t, v);
			}
		}
	}

	ret = simple_read_from_buffer(user_buf, count, ppos, t, strlen(t));
	kfree(t);
	return ret;
}

const struct file_operations fileop_perfor_statistic = {
	.write = perf_statistic_write,
	.read = perf_statistic_read,
};
#endif

#ifdef CLK_CFG
DEFINE_SPINLOCK(clk_lock);
#endif

#define DWL_CLIENT_TYPE_H264_DEC        1U
#define DWL_CLIENT_TYPE_MPEG4_DEC       2U
#define DWL_CLIENT_TYPE_JPEG_DEC        3U
#define DWL_CLIENT_TYPE_PP              4U
#define DWL_CLIENT_TYPE_VC1_DEC         5U
#define DWL_CLIENT_TYPE_MPEG2_DEC       6U
#define DWL_CLIENT_TYPE_VP6_DEC         7U
#define DWL_CLIENT_TYPE_AVS_DEC         8U
#define DWL_CLIENT_TYPE_RV_DEC          9U
#define DWL_CLIENT_TYPE_VP8_DEC         10U
#define DWL_CLIENT_TYPE_VP9_DEC         11U
#define DWL_CLIENT_TYPE_HEVC_DEC        12U
#define DWL_CLIENT_TYPE_ST_PP           14U
#define DWL_CLIENT_TYPE_H264_MAIN10     15U
#define DWL_CLIENT_TYPE_AVS2_DEC        16U
#define DWL_CLIENT_TYPE_AV1_DEC         17U

#define CORE_TYPE_STR_CASE(ct) { case (ct): return(#ct + 3); }

static char *CoreTypeStr(enum CoreType ct)
{
	switch (ct) {
		CORE_TYPE_STR_CASE(HW_VCD)
		CORE_TYPE_STR_CASE(HW_VCDJ)
		CORE_TYPE_STR_CASE(HW_VCMD)
		CORE_TYPE_STR_CASE(HW_MMU)
		CORE_TYPE_STR_CASE(HW_MMU_WR)
		CORE_TYPE_STR_CASE(HW_DEC400)
		CORE_TYPE_STR_CASE(HW_AXIFE)
		CORE_TYPE_STR_CASE(HW_AFBC)
	default :
		return "Invalid core type";
	}
}

#ifdef HANTRODEC_DEBUG

#define IOCTL_CMD_STR_CASE(cmd) { case (cmd): return(#cmd); }

static char *IoctlCmdStr(unsigned int cmd)
{
	switch (cmd) {
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_MC_CORES)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCGHWOFFSET)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCGHWIOSIZE)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_MC_OFFSETS)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_CLI)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_STI)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCS_DEC_PUSH_REG)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCS_DEC_PULL_REG)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCH_DEC_RESERVE)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCT_DEC_RELEASE)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCX_DEC_WAIT)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCG_CORE_WAIT)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOX_ASIC_ID)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCG_CORE_ID)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCS_DEC_WRITE_REG)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCS_DEC_READ_REG)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOX_ASIC_BUILD_ID)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCX_POLL)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOX_SUBSYS)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOCS_DEC_WRITE_APBFILTER_REG)
		IOCTL_CMD_STR_CASE(HANTRODEC_DEBUG_STATUS)
		/* MMU */
		IOCTL_CMD_STR_CASE(HANTRO_IOCS_MMU_MEM_MAP)
		IOCTL_CMD_STR_CASE(HANTRO_IOCS_MMU_MEM_UNMAP)
		IOCTL_CMD_STR_CASE(HANTRO_IOCS_MMU_FLUSH)
		/* VCMD */
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_GET_CMDBUF_PARAMETER)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_GET_CMDBUF_POOL_SIZE)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_SET_CMDBUF_POOL_BASE)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_GET_VCMD_PARAMETER)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_RESERVE_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_LINK_RUN_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_WAIT_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_RELEASE_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_POLLING_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_DROP_OWNER)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_PUSH_SLICE_REG)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_ABORT_CMDBUF)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_IOCH_WAIT_OWNER_DONE)
		IOCTL_CMD_STR_CASE(HANTRO_VCMD_TOCH_GET_CMDBUF_STATUS)
		/* AXI FE / APB filter */
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_APBFILTER_CONFIG)
		IOCTL_CMD_STR_CASE(HANTRODEC_IOC_AXIFE_CONFIG)
	default :
		return "Invalid ioctl cmd";
	}
}
#endif

static void ReadCoreConfig(hantrodec_t *dev)
{
	int c, j;
	u32 reg, tmp, mask;

	memset(subsys_mgr.config.cfg, 0, sizeof(subsys_mgr.config.cfg));

	for (c = 0; c < dev->cores; c++) {
		for (j = 0; j < HW_CORE_MAX; j++) {
			if (j != HW_VCD && j != HW_VCDJ)
				continue;
			/* NOT defined core type */
			if (!dev->hwregs[c][j])
				continue;
			/* Decoder configuration */
			if (IS_VCD(dev->hw_id[c][j])) {
				reg = ioread32((void __iomem *)(dev->hwregs[c][j] +
					HANTRODEC_SYNTH_CFG * 4));

				vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] swreg[%d] = 0x%08x\n",
					c, HANTRODEC_SYNTH_CFG, reg);

				tmp = (reg >> DWL_H264_E) & 0x3U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has H264\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_H264_DEC : 0;

				tmp = (reg >> DWL_H264HIGH10_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has H264HIGH10\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_H264_DEC : 0;

				tmp = (reg >> DWL_AVS2_E) & 0x03U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has AVS2\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_AVS2_DEC : 0;

				tmp = (reg >> DWL_AV1_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has AV1\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_AV1_DEC : 0;

				tmp = (reg >> DWL_JPEG_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has JPEG\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;

				tmp = (reg >> DWL_HJPEG_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has HJPEG\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_JPEG_DEC : 0;

				tmp = (reg >> DWL_MPEG4_E) & 0x3U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has MPEG4\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_MPEG4_DEC : 0;

				tmp = (reg >> DWL_VC1_E) & 0x3U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has VC1\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_VC1_DEC : 0;

				tmp = (reg >> DWL_MPEG2_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has MPEG2\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_MPEG2_DEC : 0;

				tmp = (reg >> DWL_VP6_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has VP6\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_VP6_DEC : 0;

				reg = ioread32((void __iomem *)
					(dev->hwregs[c][j] +
					HANTRODEC_SYNTH_CFG_2 * 4));

				vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] swreg[%d] = 0x%08x\n",
					c, HANTRODEC_SYNTH_CFG_2, reg);

				if (ioread32((void __iomem *)
					(dev->hwregs[c][j] +
					HANTRODEC_HWBUILD_ID_OFF)) !=
					0x1F70) {
					/* VP7 and WEBP is part of VP8 */
					mask = (1 << DWL_VP8_E) |
						(1 << DWL_VP7_E) |
						(1 << DWL_WEBP_E);
					tmp = (reg & mask);
					if (tmp & (1 << DWL_VP8_E))
						vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has VP8\n", c);
					if (tmp & (1 << DWL_VP7_E))
						vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has VP7\n", c);
					if (tmp & (1 << DWL_WEBP_E))
						vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has WebP\n", c);
					subsys_mgr.config.cfg[c] |=
					  tmp ? 1 << DWL_CLIENT_TYPE_VP8_DEC :
					  0;
				}

				tmp = (reg >> DWL_AVS_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has AVS\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_AVS_DEC : 0;

				tmp = (reg >> DWL_RV_E) & 0x03U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has RV\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_RV_DEC : 0;

				reg = ioread32((void __iomem *)
					  (dev->hwregs[c][j] +
					  HANTRODEC_SYNTH_CFG_3 * 4));
				vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] swreg[%d] = 0x%08x\n",
					c, HANTRODEC_SYNTH_CFG_3, reg);

				tmp = (reg >> DWL_HEVC_E) & 0x07U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has HEVC\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_HEVC_DEC : 0;

				tmp = (reg >> DWL_VP9_E) & 0x07U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has VP9\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_VP9_DEC : 0;

				/* Post-processor configuration */
				reg = ioread32((void __iomem *)
					  (dev->hwregs[c][j] +
					  HANTRODECPP_CFG_STAT * 4));

				tmp = (reg >> DWL_PP_E) & 0x01U;
				if (tmp)
					vdec_klog(LOGLVL_INFO, "hantrodec: subsys[%d] has PP\n", c);
				subsys_mgr.config.cfg[c] |=
					tmp ? 1 << DWL_CLIENT_TYPE_PP : 0;

				subsys_mgr.config.cfg[c] |= 1 << DWL_CLIENT_TYPE_ST_PP;
			}
		}
	}
}

static int CoreHasFormat(const u32 *cfg, int core, u32 format)
{
	return (cfg[core] & (1 << format)) ? 1 : 0;
}

static int IsValideCore(long core, unsigned long format)
{
	u32 core_mask = format & (0xffffffff >>
							CORE_MASK_CLIENTTYPE_BITMAP);

	if ((core_mask >> core) & 0x01)
		return 1;
	return 0;
}

static int GetDecCore(long core, hantrodec_t *dev, struct file *filp,
		      unsigned long format)
{
	int success = 0;
	unsigned long flags;
	#ifdef SUPPORT_DBGFS
		u64 time_num;
		struct timeval time_now;
	#endif

	spin_lock_irqsave(&subsys_mgr.owner_lock, flags);
	if (IsValideCore(core, format) && !subsys_mgr.dec_owner[core]) {
		subsys_mgr.dec_owner[core] = filp;
		success = 1;
#ifdef SUPPORT_DBGFS

		subsys_mgr.decode_state[core] = RESERVED;
#if (KERNEL_VERSION(4, 19, 94) > LINUX_VERSION_CODE)

	do_gettimeofday(&time_now);
		time_num = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
#else
	struct timespec64 time_now;

	ktime_get_real_ts64(&time_now);
		time_num = time_now.tv_sec * 1000 + time_now.tv_nsec / 1000000;
#endif
	subsys_mgr.reserved_time[core][subsys_mgr.r_index[core]] = time_num;
		subsys_mgr.r_index[core] = subsys_mgr.r_index[core] + 1;
	if (subsys_mgr.r_index[core] == MAX_RESERVED_TIME)
		subsys_mgr.r_index[core] = 0;
#endif
	}

	spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);

	return success;
}

static int GetDecCoreAny(long *core, hantrodec_t *dev,
			 struct file *filp,
			 unsigned long format)
{
	int success = 0;
	long c;

	*core = -1;

	for (c = 0; c < dev->cores; c++) {
		/* a free core that has format */
		if (GetDecCore(c, dev, filp, format)) {
			success = 1;
			*core = c;
			break;
		}
	}

	return success;
}

static int GetDecCoreID(hantrodec_t *dev,
			struct file *filp,
			unsigned long format)
{
	long c;
	unsigned long flags;

	int core_id = -1;

	/* fuzz report */
	if (dev->cores >= HXDEC_MAX_CORES) {
		vdec_klog(LOGLVL_ERROR, "dev->cores(%u) out of bounds\n", dev->cores);
		return -EINVAL;
	}

	for (c = 0; c < dev->cores; c++) {
		/* a core that has format */
		spin_lock_irqsave(&subsys_mgr.owner_lock, flags);
		if (CoreHasFormat(subsys_mgr.config.cfg, c, format)) {
			core_id = c;
			spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);
			break;
		}
		spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);
	}
	return core_id;
}

static long ReserveDecoder(hantrodec_t *dev,
			   struct file *filp,
	 unsigned long format)
{
	long core = -1;

	/* fuzz report */
	if (!subsys_mgr.dec_core_sem.wait_list.prev &&
		!subsys_mgr.dec_core_sem.wait_list.next) {
		vdec_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}

	/* reserve a core */
	if (down_interruptible(&subsys_mgr.dec_core_sem)) {
		vcmd_klog(LOGLVL_WARNING, "down_interruptible is interrupted.\n");
		return -ERESTARTSYS;
	}

	/* lock a core that has specific format*/
	if (wait_event_interruptible(subsys_mgr.hw_queue,
				     GetDecCoreAny(&core, dev,
						       filp, format) != 0)) {
		vcmd_klog(LOGLVL_WARNING, "wait_event_interruptible is interrupted.\n");
		return -ERESTARTSYS;
	}

	return core;
}

static void ReleaseDecoder(hantrodec_t *dev, long core)
{
	u32 status;
	unsigned long flags;

	#ifdef SUPPORT_DBGFS
		u64 time_num;
		struct timeval time_now;
	#endif

	PDEBUG("%s %ld\n", __func__, core);

	status = ioread32((void __iomem *)
			(dev->hwregs[core][HW_VCD] +
			HANTRODEC_IRQ_STAT_DEC_OFF));

	/* make sure HW is disabled */
	if (status & HANTRODEC_DEC_E) {
		vcmd_klog(LOGLVL_DEBUG, "hantrodec: DEC[%li] still enabled -> reset\n",
			core);

		/* abort decoder */
		status |= HANTRODEC_DEC_ABORT | HANTRODEC_DEC_IRQ_DISABLE;
		iowrite32(status, (void __iomem *)
		  (dev->hwregs[core][HW_VCD] +
		  HANTRODEC_IRQ_STAT_DEC_OFF));
	}
#ifdef SUPPORT_WATCHDOG
	/* if interrupt uses polling mode */
	_watchdog_stop(&dev->watchdog[core]);
#endif

	spin_lock_irqsave(&subsys_mgr.owner_lock, flags);

	subsys_mgr.dec_owner[core] = NULL;
#ifdef SUPPORT_DBGFS
#if (KERNEL_VERSION(4, 19, 94) > LINUX_VERSION_CODE)

	do_gettimeofday(&time_now);
	time_num = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
#else
	struct timespec64 time_now;

	ktime_get_real_ts64(&time_now);
	time_num = time_now.tv_sec * 1000 + time_now.tv_nsec / 1000000;
#endif
	subsys_mgr.release_time[core][subsys_mgr.release_index[core]] = time_num;
	subsys_mgr.release_index[core] = subsys_mgr.release_index[core] + 1;
	if (subsys_mgr.release_index[core] == MAX_RESERVED_TIME)
		subsys_mgr.release_index[core] = 0;
	subsys_mgr.decode_state[core] = IDLE;
#endif
	subsys_mgr.dec_irq &= ~(1 << core);

	spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);

	up(&subsys_mgr.dec_core_sem);

	wake_up_interruptible_all(&subsys_mgr.hw_queue);
}

#ifdef HANTRODEC_DEBUG
static u32 flush_count; /* times of calling of DecFlushRegs */
static u32 flush_regs; /* total number of registers flushed */
#endif

static int DecFlushRegs(hantrodec_t *dev, struct core_desc *core)
{
	long ret = 0, i;
#ifdef HANTRODEC_DEBUG
	int reg_wr = 2;
#endif
#ifdef SUPPORT_DBGFS
		u64 time_num;
		struct timeval time_now;
#endif
	u32 id = core->id;
	u32 type = core->type;

	PDEBUG("hantrodec: %S\n", __func__);
	PDEBUG("hantrodec: id = %d, type = %d [ %s ], size = %d, reg_id = %d\n",
	       core->id, core->type, CoreTypeStr(core->type), core->size,
		   core->reg_id);

	/* fuzz report */
	if (id >= MAX_SUBSYS_NUM) {
		vdec_klog(LOGLVL_ERROR, "id(%u) > MAX_SUBSYS_NUM\n", id);
		return -EINVAL;
	}

	if (type == HW_VCD && !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		type = HW_VCDJ;

	if (!subsys_mgr.vpu_subsys[id].base_addr ||
	    core->type >= HW_CORE_MAX || !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		return -EINVAL;

	PDEBUG("hantrodec: submodule_iosize = %d\n",
	       subsys_mgr.vpu_subsys[id].submodule_iosize[type]);
	PDEBUG("hantrodec: reg count = %d\n", reg_count[id]);

	ret = copy_from_user(subsys_mgr.dec_regs[id], (__u32 __user *)core->regs,
			     subsys_mgr.vpu_subsys[id].submodule_iosize[type]);
	if (ret) {
		PDEBUG("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	if (type == HW_VCD || type == HW_BIGOCEAN || type == HW_VCDJ) {
		/* write all regs but the status reg[1] to hardware */
		if (reg_access_opt) {
			for (i = 3;
				 i < subsys_mgr.vpu_subsys[id].submodule_iosize[type] / 4;
				 i++) {
				/* check whether register value is updated. */
				if (subsys_mgr.dec_regs[id][i] != subsys_mgr.shadow_dec_regs[id][i]) {
					iowrite32(subsys_mgr.dec_regs[id][i], (void __iomem *)
					  (dev->hwregs[id][type] + i * 4));
					subsys_mgr.shadow_dec_regs[id][i] = subsys_mgr.dec_regs[id][i];

#ifdef HANTRODEC_DEBUG
					reg_wr++;
#endif
				}
			}
		} else {
			for (i = 3;
				 i < subsys_mgr.vpu_subsys[id].submodule_iosize[type] / 4;
				 i++) {
				iowrite32(subsys_mgr.dec_regs[id][i], (void __iomem *)
						  (dev->hwregs[id][type] + i * 4));

#ifdef VALIDATE_REGS_WRITE
	if (subsys_mgr.dec_regs[id][i] !=
			ioread32((void *)(dev->hwregs[id][type] +
			  i * 4)))
		vcmd_klog(LOGLVL_DEBUG, "hantrodec: swreg[%ld]: read %08x != write %08x *\n",
				i,
				ioread32((void *)(dev->hwregs[id][type] + i * 4)),
				subsys_mgr.dec_regs[id][i]);
#endif
			}
#ifdef HANTRODEC_DEBUG
			reg_wr = subsys_mgr.vpu_subsys[id].submodule_iosize[type] / 4 - 1;
#endif
		}

		/* write swreg2 for AV1, in which bit0 is the start bit */
		iowrite32(subsys_mgr.dec_regs[id][2],
			  (void __iomem *)(dev->hwregs[id][type] + 8));
		subsys_mgr.shadow_dec_regs[id][2] = subsys_mgr.dec_regs[id][2];

#ifdef CONFIG_DEC_PM
		if (subsys_mgr.dec_regs[id][1] & 0x1)
			dev->hw_active[id] = 1;
#endif

		/* write the status register, which may start the decoder */
		iowrite32(subsys_mgr.dec_regs[id][1],
			  (void __iomem *)(dev->hwregs[id][type] + 4));
#ifdef SUPPORT_DBGFS

	if ((subsys_mgr.dec_regs[id][1] & 0x1) == 1) {
		subsys_mgr.decode_state[id] = DECODING;
#if (KERNEL_VERSION(4, 19, 94) > LINUX_VERSION_CODE)

		do_gettimeofday(&time_now);
		time_num = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
#else
		struct timespec64 time_now;

		ktime_get_real_ts64(&time_now);
		time_num = time_now.tv_sec * 1000 + time_now.tv_nsec / 1000000;
#endif
		subsys_mgr.start_hw_time[id][subsys_mgr.start_hw_index[id]] = time_num;
		subsys_mgr.start_hw_index[id] = subsys_mgr.start_hw_index[id] + 1;
		if (subsys_mgr.start_hw_index[id] == MAX_RESERVED_TIME)
			subsys_mgr.start_hw_index[id] = 0;
	}
#endif
		subsys_mgr.shadow_dec_regs[id][1] = subsys_mgr.dec_regs[id][1];

#ifdef HANTRODEC_DEBUG
		flush_count++;
		flush_regs += reg_wr;
#endif

		PDEBUG("flushed registers on core %d\n", id);
		PDEBUG("%d %s: flushed %d/%d registers (dec_mode = %d, avg %d regs per flush)\n",
		       flush_count, __func__,
			   reg_wr, flush_regs,
			   subsys_mgr.dec_regs[id][3] >> 27,
			   flush_regs / flush_count);
	} else {
		/* write all regs but the status reg[1] to hardware */
		for (i = 0; i < subsys_mgr.vpu_subsys[id].submodule_iosize[type] / 4;
			 i++) {
			iowrite32(subsys_mgr.dec_regs[id][i],
				  (void __iomem *)(dev->hwregs[id][type] +
				  i * 4));
#ifdef VALIDATE_REGS_WRITE
			if (subsys_mgr.dec_regs[id][i] !=
				ioread32((void *)(dev->hwregs[id][type] + i * 4)))
				vdec_klog(LOGLVL_INFO,
					   "hantrodec: swreg[%ld]: read %08x != write %08x *\n",
					   i,
					   ioread32((void *)(dev->hwregs[id][type] +
					   i * 4)),
					   subsys_mgr.dec_regs[id][i]);
#endif
		}
	}
#ifdef SUPPORT_WATCHDOG
	//feed the watchdog
	_watchdog_feed(&dev->watchdog[id]);
#endif


	return 0;
}

static int DecWriteRegs(hantrodec_t *dev, struct core_desc *core)
{
	long ret = 0;
	u32 i = core->reg_id;
	u32 id = core->id;
	u32 type = core->type;

	PDEBUG("hantrodec: %s\n", __func__);
	PDEBUG("hantrodec: id = %d, type = %d [ %s ], size = %d, reg_id = %d\n",
	       core->id, core->type, CoreTypeStr(core->type), core->size,
		   core->reg_id);

	/* fuzz report */
	if (id >= MAX_SUBSYS_NUM) {
		vdec_klog(LOGLVL_ERROR, "id(%u) > MAX_SUBSYS_NUM\n", id);
		return -EINVAL;
	}

	if (type == HW_VCD && !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		type = HW_VCDJ;

	if (!subsys_mgr.vpu_subsys[id].base_addr ||
	    type >= HW_CORE_MAX || !subsys_mgr.vpu_subsys[id].submodule_hwregs[type] ||
		(core->size & 0x3) || core->reg_id * 4 + core->size >
		subsys_mgr.vpu_subsys[id].submodule_iosize[type])

		return -EINVAL;

	ret = copy_from_user(subsys_mgr.dec_regs[id], (__u32 __user *)core->regs,
			     core->size);
	if (ret) {
		PDEBUG("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++) {
		PDEBUG("hantrodec: write %08x to reg[%d] core %d\n",
		       subsys_mgr.dec_regs[id][i - core->reg_id], i, id);
		iowrite32(subsys_mgr.dec_regs[id][i - core->reg_id],
			  (void __iomem *)(dev->hwregs[id][type] + i * 4));
		if (type == HW_VCD)
			subsys_mgr.shadow_dec_regs[id][i] = subsys_mgr.dec_regs[id][i - core->reg_id];
	}
	return 0;
}

static int DecWriteApbFilterRegs(hantrodec_t *dev, struct core_desc *core)
{
	long ret = 0;
	u32 i = core->reg_id;
	u32 id = core->id;

	PDEBUG("hantrodec: %s\n", __func__);
	PDEBUG("hantrodec: id = %d, type = %d, size = %d, reg_id = %d\n",
	       core->id, core->type, core->size, core->reg_id);

	if (id >= MAX_SUBSYS_NUM || !subsys_mgr.vpu_subsys[id].base_addr ||
	    core->type >= HW_CORE_MAX ||
		!subsys_mgr.vpu_subsys[id].submodule_hwregs[core->type] ||
		(core->size & 0x3) ||
		core->reg_id * 4 + core->size >
		subsys_mgr.vpu_subsys[id].submodule_iosize[core->type] + 4)

		return -EINVAL;

	ret = copy_from_user(subsys_mgr.apbfilter_regs[id], (__u32 __user *)core->regs,
			     core->size);
	if (ret) {
		PDEBUG("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}

	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++) {
		PDEBUG("hantrodec: write %08x to reg[%d] core %d\n",
		       subsys_mgr.dec_regs[id][i - core->reg_id], i, id);
		iowrite32(subsys_mgr.apbfilter_regs[id][i - core->reg_id],
			  (void __iomem *)
				  (dev->apbfilter_hwregs[id][core->type] +
				  i * 4));
	}
	return 0;
}

static long DecReadRegs(hantrodec_t *dev, struct core_desc *core)
{
	struct hantrodec_dev *dec_dev;
	vdec_power_mgr *pm = NULL;
	struct SubsysMgr *subsys;
	long ret;
	int ret_val;
	u32 id = core->id;
	u32 i = core->reg_id;
	u32 type = core->type;
	#ifdef SUPPORT_DBGFS
		u64 time_num;
		struct timeval time_now;
	#endif
	PDEBUG("hantrodec: %s\n", __func__);
	PDEBUG("hantrodec: id = %d, type = %d [ %s ], size = %d, reg_id = %d\n",
	       core->id, core->type, CoreTypeStr(core->type), core->size,
	 core->reg_id);

	/* fuzz report */
	if (id >= MAX_SUBSYS_NUM) {
		vdec_klog(LOGLVL_ERROR, "id(%u) > MAX_SUBSYS_NUM\n", id);
		return -EINVAL;
	}

	if (type == HW_VCD && !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		type = HW_VCDJ;

	if (!subsys_mgr.vpu_subsys[id].base_addr ||
	    type >= HW_CORE_MAX || !subsys_mgr.vpu_subsys[id].submodule_hwregs[type] ||
			(core->size & 0x3) ||
			core->reg_id * 4 + core->size >
			subsys_mgr.vpu_subsys[id].submodule_iosize[type])
		return -EINVAL;

	/* fuzz report */
	subsys = container_of(dev, struct SubsysMgr, hantrodec_data);
	dec_dev = platform_get_drvdata(subsys->platformdev);
	if (!dec_dev || !dec_dev->pm) {
		vdec_pm_klog(LOGLVL_ERROR, "pm or dec_dev[%p] is NULL!\n", dec_dev);
		return -EINVAL;
	}
	pm = dec_dev->pm;
	mutex_lock(&pm->pm_lock);
	ret_val = regulator_is_enabled(pm->vdec_rg);
	if (ret_val <= 0) {
		vcmd_klog(LOGLVL_WARNING, "vdec core has been powered off. ret_val=%d\n", ret_val);
		mutex_unlock(&pm->pm_lock);
		return -EBUSY;
	}

	/* read specific registers from hardware */
	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++) {
		subsys_mgr.dec_regs[id][i] = ioread32(
			(void __iomem *)(dev->hwregs[id][type] + i * 4));
#ifdef SUPPORT_DBGFS
		if ((i == 1) && ((subsys_mgr.dec_regs[id][1] & 0x1) == 0)) {
			subsys_mgr.decode_state[id] = DONE;
#if (KERNEL_VERSION(4, 19, 94) > LINUX_VERSION_CODE)

			do_gettimeofday(&time_now);
			time_num = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
#else
			struct timespec64 time_now;

			ktime_get_real_ts64(&time_now);
			time_num = time_now.tv_sec * 1000 + time_now.tv_nsec / 1000000;
#endif
			subsys_mgr.hw_return_time[id][subsys_mgr.hw_r_index[id]] = time_num;
			subsys_mgr.hw_r_index[id] = subsys_mgr.hw_r_index[id] + 1;
			if (subsys_mgr.hw_r_index[id] == MAX_RESERVED_TIME)
				subsys_mgr.hw_r_index[id] = 0;
		}
#endif
		PDEBUG("hantrodec: read %08x from reg[%d] core %d\n",
		       subsys_mgr.dec_regs[id][i], i, id);
		if (type == HW_VCD)
			subsys_mgr.shadow_dec_regs[id][i] = subsys_mgr.dec_regs[id][i];
	}
	mutex_unlock(&pm->pm_lock);

	/* put registers to user space*/
	ret = copy_to_user((__u32 __user *)core->regs, &subsys_mgr.dec_regs[id][core->reg_id],
			   core->size);
	if (ret) {
		PDEBUG("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	return 0;
}

static long DecRefreshRegs(hantrodec_t *dev, struct core_desc *core)
{
	vdec_power_mgr *pm = NULL;
	struct SubsysMgr *subsys;
	struct hantrodec_dev *dec_dev;
	long ret, i;
	int ret32;
	u32 id = core->id;
	u32 type = core->type;

	PDEBUG("hantrodec: %s\n", __func__);
	PDEBUG("hantrodec: id = %d, type = %d [ %s ], size = %d, reg_id = %d\n",
	       core->id, core->type, CoreTypeStr(core->type), core->size,
	 core->reg_id);

	/* MC check report */
	if (id >= MAX_SUBSYS_NUM) {
		vdec_klog(LOGLVL_ERROR, "id(%u) > MAX_SUBSYS_NUM\n", id);
		return -EINVAL;
	}

	if (type == HW_VCD && !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		type = HW_VCDJ;

	if (!subsys_mgr.vpu_subsys[id].base_addr ||
	    type >= HW_CORE_MAX || !subsys_mgr.vpu_subsys[id].submodule_hwregs[type])
		return -EINVAL;

	PDEBUG("hantrodec: submodule_iosize = %d\n",
	       vpu_subsys[id].submodule_iosize[type]);

	subsys = container_of(dev, struct SubsysMgr, hantrodec_data);
	dec_dev = platform_get_drvdata(subsys->platformdev);
	if (!dec_dev || !dec_dev->pm) {
		vdec_pm_klog(LOGLVL_ERROR, "pm or dec_dev[%p] is NULL!\n", dec_dev);
		return -EINVAL;
	}
	pm = dec_dev->pm;
	/* check if device is suspended */
	mutex_lock(&pm->pm_lock);
	ret32 = regulator_is_enabled(pm->vdec_rg);
	if (ret32 <= 0) {
		vcmd_klog(LOGLVL_WARNING, "vdec core has been powered off. ret=%d\n", ret32);
		mutex_unlock(&pm->pm_lock);
		return -EINVAL;
	}
	if (!reg_access_opt) {
		for (i = 0; i < subsys_mgr.vpu_subsys[id].submodule_iosize[type] / 4; i++) {
			subsys_mgr.dec_regs[id][i] = ioread32((void __iomem *)(dev->hwregs[id][type] + i * 4));
		}
	} else {
		// only need to read swreg1,62(?),63,168,169
#define REFRESH_REG(idx)                                                       \
	do {                                                                   \
		i = (idx);                                                     \
		subsys_mgr.shadow_dec_regs[id][i] = subsys_mgr.dec_regs[id][i] = ioread32(           \
			(void __iomem *)(dev->hwregs[id][type] + i * 4));      \
	} while (0)
		REFRESH_REG(0);
		REFRESH_REG(1);
		REFRESH_REG(62);
		REFRESH_REG(63);
		REFRESH_REG(168);
		REFRESH_REG(169);
#undef REFRESH_REG
	}
	mutex_unlock(&pm->pm_lock);

	ret = copy_to_user((__u32 __user *)core->regs, subsys_mgr.dec_regs[id],
			   subsys_mgr.vpu_subsys[id].submodule_iosize[type]);
	if (ret) {
		PDEBUG("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	return 0;
}

static int CheckDecIrq(hantrodec_t *dev, u32 id)
{
	unsigned long flags;
	int rdy = 0;

	const u32 irq_mask = (1U << id);

	spin_lock_irqsave(&subsys_mgr.owner_lock, flags);

	if (subsys_mgr.dec_irq & irq_mask) {
		/* reset the wait condition(s) */
		subsys_mgr.dec_irq &= ~irq_mask;
		rdy = 1;
	}

	spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);

	return rdy;
}

static long WaitDecReadyAndRefreshRegs(hantrodec_t *dev, struct core_desc *core)
{
	u32 id = core->id;
	long ret;

	PDEBUG("wait_event_interruptible DEC[%d]\n", id);

	/* fuzz report */
	if (id >= UINT32_MAX_BITS) {
		vdec_klog(LOGLVL_WARNING, "id(%u) out of bounds\n", id)
		return -EINVAL;
	}

	/* fuzz report */
	if (!subsys_mgr.dec_wait_queue.head.prev &&
		!subsys_mgr.dec_wait_queue.head.next) {
		vdec_klog(LOGLVL_ERROR, "dec_wait_queue has not been initialized\n");
		return -EINVAL;
	}

	ret = wait_event_interruptible(subsys_mgr.dec_wait_queue, CheckDecIrq(dev, id));
	if (ret) {
		PDEBUG("DEC[%d]  wait_event_interruptible interrupted\n", id);
		return -ERESTARTSYS;
	}

	atomic_inc(&subsys_mgr.irq_tx);

	/* refresh registers */
	return DecRefreshRegs(dev, core);
}

static int CheckCoreIrq(hantrodec_t *dev, const struct file *filp, int *id)
{
	unsigned long flags;
	int rdy = 0, n = 0;

	do {
		u32 irq_mask = (1 << n);

		spin_lock_irqsave(&subsys_mgr.owner_lock, flags);

		if (subsys_mgr.dec_irq & irq_mask) {
			if (subsys_mgr.dec_owner[n] == filp) {
				/* we have an IRQ for our client */

				/* reset the wait condition(s) */
				subsys_mgr.dec_irq &= ~irq_mask;

				/* signal ready core no. for our client */
				*id = n;

				rdy = 1;

				spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);
				break;
			} else if (!subsys_mgr.dec_owner[n]) {
				/* zombie IRQ */
				vcmd_klog(LOGLVL_VERBOSE, "IRQ on core[%d], but no owner!!!\n",
					n);

				/* reset the wait condition(s) */
				subsys_mgr.dec_irq &= ~irq_mask;
			}
		}

		spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);

		n++; /* next core */
	} while (n < dev->cores);

	return rdy;
}

static long WaitCoreReady(hantrodec_t *dev, const struct file *filp, int *id)
{
	long ret;

	PDEBUG("wait_event_interruptible CORE\n");

	/* fuzz report */
	if (!subsys_mgr.dec_wait_queue.head.prev &&
		!subsys_mgr.dec_wait_queue.head.next) {
		vdec_klog(LOGLVL_ERROR, "semaphore has not been initialized\n");
		return -EINVAL;
	}

	ret = wait_event_interruptible(subsys_mgr.dec_wait_queue,
				       CheckCoreIrq(dev, filp, id));
	if (ret) {
		PDEBUG("CORE[%d] wait_event_interruptible interrupted with 0x%x\n",
		       *id, ret);
		return -ERESTARTSYS;
	}

	atomic_inc(&subsys_mgr.irq_tx);

	return 0;
}

/*---------------------------------------------------------
 * Function name : hantrodec_ioctl
 * Description   : communication method to/from the user space
 * Return type   : long
 *----------------------------------------------------------
 */
static long hantrodec_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long arg)
{
	struct hantrodec_dev *dec_dev;
	vdec_power_mgr *pm = NULL;
	int err = 0;
	long tmp;
	u32 i = 0;
	int ret;
	int ret_func = 0;
#ifdef CLK_CFG
	unsigned long flags;
#endif

#ifdef HW_PERFORMANCE
	struct timeval *end_time_arg;
#endif

	PDEBUG("ioctl cmd 0x%08x [ %s ]\n", cmd, IoctlCmdStr(cmd));
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl)
	 * before access_ok()
	 */
	if (_IOC_TYPE(cmd) != HANTRODEC_IOC_MAGIC &&
	    _IOC_TYPE(cmd) != HANTRO_IOC_MMU &&
		_IOC_TYPE(cmd) != HANTRO_VCMD_IOC_MAGIC)

		return -ENOTTY;
	if ((_IOC_TYPE(cmd) == HANTRODEC_IOC_MAGIC &&
	     _IOC_NR(cmd) > HANTRODEC_IOC_MAXNR) ||
		 (_IOC_TYPE(cmd) == HANTRO_IOC_MMU &&
		 _IOC_NR(cmd) > HANTRO_IOC_MMU_MAXNR) ||
		 (_IOC_TYPE(cmd) == HANTRO_VCMD_IOC_MAGIC &&
		 _IOC_NR(cmd) > HANTRO_VCMD_IOC_MAXNR))

		return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok((void *)arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

#ifdef CLK_CFG
	spin_lock_irqsave(&clk_lock, flags);
	if (clk_cfg && !IS_ERR(clk_cfg) && (is_clk_on == 0)) {
		vdec_klog(LOGLVL_VERBOSE, "turn on clock by user\n");
		if (clk_enable(clk_cfg)) {
			spin_unlock_irqrestore(&clk_lock, flags);
			return -EFAULT;
		}
		is_clk_on = 1;
	}

	spin_unlock_irqrestore(&clk_lock, flags);
	/*the interval is 10s*/
	mod_timer(&timer, jiffies + 10 * HZ);
#endif

	switch (cmd) {
	case HANTRODEC_IOC_CLI: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOC_CLI.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOC_STI: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOC_STI.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCGHWOFFSET: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCGHWOFFSET.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCGHWIOSIZE: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCGHWIOSIZE.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOC_MC_OFFSETS: {
		tmp = copy_to_user((unsigned long __user *)arg,
				   subsys_mgr.multicorebase,
		 sizeof(subsys_mgr.multicorebase));
		if (tmp) {
			PDEBUG("copy_to_user failed, returned %li\n", tmp);
			return -EFAULT;
		}
		break;
	}
	case HANTRODEC_IOC_MC_CORES:
		__put_user(subsys_mgr.hantrodec_data.cores, (unsigned int __user *)arg);
		PDEBUG("subsys_mgr.hantrodec_data.cores=%d\n", subsys_mgr.hantrodec_data.cores);
		break;
	case HANTRODEC_IOCS_DEC_PUSH_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_DEC_PUSH_REG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCS_DEC_WRITE_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_DEC_WRITE_REG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCS_DEC_WRITE_APBFILTER_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_DEC_WRITE_APBFILTER_REG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCS_PP_PUSH_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_PP_PUSH_REG.\n");
		return -EINVAL;
	}
	case HANTRODEC_IOCS_DEC_PULL_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_DEC_PULL_REG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCS_DEC_READ_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_DEC_READ_REG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCS_PP_PULL_REG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCS_PP_PULL_REG.\n");
		return -EINVAL;
	}
	case HANTRODEC_IOCH_DEC_RESERVE: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCH_DEC_RESERVE.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCT_DEC_RELEASE: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCT_DEC_RELEASE.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCQ_PP_RESERVE: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCQ_PP_RESERVE.\n");
		return -EINVAL;
	}
	case HANTRODEC_IOCT_PP_RELEASE: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCT_PP_RELEASE.\n");
		return -EINVAL;
	}
	case HANTRODEC_IOCX_DEC_WAIT: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCX_DEC_WAIT.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCX_PP_WAIT: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCX_PP_WAIT.\n");
		return -EINVAL;
	}
	case HANTRODEC_IOCG_CORE_WAIT: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCG_CORE_WAIT.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOX_ASIC_ID: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOX_ASIC_ID.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOCG_CORE_ID: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCG_CORE_ID.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOX_ASIC_BUILD_ID: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOX_ASIC_BUILD_ID.\n");
		return -EFAULT;
	}
	case HANTRODEC_DEBUG_STATUS: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_DEBUG_STATUS.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOX_SUBSYS: {
		struct subsys_desc subsys = { 0 };
		/* TODO(min): check all the subsys */
		if (vcmd) {
			subsys.subsys_vcmd_num = 1;
			subsys.subsys_num = subsys.subsys_vcmd_num;
		} else {
			subsys.subsys_num = subsys_mgr.hantrodec_data.cores;
			subsys.subsys_vcmd_num = 0;
		}
		if (copy_to_user((u32 __user *)arg, &subsys,
			     sizeof(struct subsys_desc)))
			return -EFAULT;
		return 0;
	}
	case HANTRODEC_IOCX_POLL: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOCX_POLL.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOC_APBFILTER_CONFIG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOC_APBFILTER_CONFIG.\n");
		return -EFAULT;
	}
	case HANTRODEC_IOC_AXIFE_CONFIG: {
		vdec_klog(LOGLVL_ERROR, "vcmd mode not use HANTRODEC_IOC_AXIFE_CONFIG.\n");
		return -EFAULT;
	}
	default: {
		if (_IOC_TYPE(cmd) == HANTRO_VCMD_IOC_MAGIC) {
			return hantrovcmd_ioctl(filp, cmd, arg);
		}
		return -ENOTTY;
	}
	}

	return 0;
}


/*
 * Function name   : hantrodec_open
 * Description     : open method

 * Return type     : int
 */
static int hantrodec_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct hantrodec_dev *dec_dev = platform_get_drvdata(subsys_mgr.platformdev);

	WARN_ON(!dec_dev);

	/* excute vdec_subsys power on and hw config here. */
	mutex_lock(&dec_dev->pm->pm_lock);
	vdec_pm_klog(LOGLVL_INFO, "hantrodec dev opening!\n");
	ret = inst_power_get(dec_dev->pm);
	mutex_unlock(&dec_dev->pm->pm_lock);
	if (ret) {
		vdec_klog(LOGLVL_ERROR, "vdec instance power get failed: %d!\n", ret);
		return ret;
	}

	if (vcmd) {
		struct vcmd_priv_ctx *vcmd_priv_ctx = NULL;

		vcmd_priv_ctx = vmalloc(sizeof(struct vcmd_priv_ctx));
		if (!vcmd_priv_ctx) {
			vdec_klog(LOGLVL_ERROR, "Create vcmd private context failed!\n");
			return -EINVAL;
		}
		memset(vcmd_priv_ctx, 0, sizeof(struct vcmd_priv_ctx));

		filp->private_data = (void *)vcmd_priv_ctx;
		vcmd_priv_ctx->vcmd_mgr = (void *)subsys_mgr.vcmd_mgr;
		hantrovcmd_open(inode, filp);
	}

	return 0;
}

/*
 * Function name   : hantrodec_release
 * Description     : Release driver

 * Return type     : int
 */

static int hantrodec_release(struct inode *inode,
			     struct file *filp)
{
	int n;
	hantrodec_t *dev = &subsys_mgr.hantrodec_data;
	struct vcmd_priv_ctx *ctx = (struct vcmd_priv_ctx *)filp->private_data;
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)ctx->vcmd_mgr;
	struct hantrovcmd_dev *vcmd_dev = &vcmd_mgr->dev_ctx[0];
	struct hantrodec_dev *dec_dev = platform_get_drvdata(subsys_mgr.platformdev);
	struct device *device = &vcmd_mgr->platformdev->dev;

	WARN_ON(!dec_dev);

	PDEBUG("closing ...\n");

	if (vcmd) {
		vdec_pm_klog(LOGLVL_INFO, "hantrodec dev closing!\n");
		hantrovcmd_release(inode, filp);
		mutex_lock(&dec_dev->pm->pm_runtime_lock);
		mutex_lock(&dec_dev->pm->pm_lock);
		/*
		 * if daemon exit by signal abnormally, the state of core maybe power_on,
		 * in this situation, we must power off vdec core to avoid leakage after
		 * release the last one decoder instance.
		*/
		inst_power_put(dec_dev->pm);
		if (atomic_read(&dec_dev->pm->power_count[POWER_VDEC_SUBSYS]) == 1 &&
			atomic_read(&dec_dev->pm->power_count[POWER_VDEC_CORE]) == 1 &&
			vcmd_dev->state != VCMD_STATE_WORKING) {
			vdec_pm_klog(LOGLVL_INFO, "vdec quit, cleanup power count here! "
					"usage_count %d, vcmd state %d, runtime_status %d\n",
					atomic_read(&device->power.usage_count), vcmd_dev->state,
					device->power.runtime_status);
			if (!dec_dev->pm->runtime_idled) {
				hantrodec_pm_runtime_put(device);
				dec_dev->pm->runtime_idled = true;
				WARN_ON(atomic_read(&device->power.usage_count) != 0);
			}
		}
		mutex_unlock(&dec_dev->pm->pm_lock);
		mutex_unlock(&dec_dev->pm->pm_runtime_lock);
		return 0;
	}

	for (n = 0; n < dev->cores; n++) {
		if (subsys_mgr.dec_owner[n] == filp) {
			PDEBUG("releasing dec core %i lock\n", n);
			ReleaseDecoder(dev, n);
		}
	}
	PDEBUG("closed\n");
	return 0;
}

#ifdef CLK_CFG
void hantrodec_disable_clk(unsigned long value)
{
	unsigned long flags;
	/* entering this function means decoder
	 * is idle over expiry.So disable clk
	 */
	if (clk_cfg && !IS_ERR(clk_cfg)) {
		spin_lock_irqsave(&clk_lock, flags);
		if (is_clk_on == 1) {
			clk_disable(clk_cfg);
			is_clk_on = 0;
			vdec_klog(LOGLVL_VERBOSE, "turned off hantrodec clk\n");
		}
		spin_unlock_irqrestore(&clk_lock, flags);
	}
}
#endif

#ifdef SUPPORT_SMMU
static int hantrodec_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	dma_addr_t vaddr_start;
	size_t size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	vcmd_klog(LOGLVL_DEBUG, "size = 0x%lx, offset = 0x%lX\n", size, offset);

	/* check if offset validity */
	if (offset + size < offset || offset + size < size || offset + size > VCMDBUF_TOTAL_SIZE) {
		vdec_klog(LOGLVL_ERROR, "invalid offset or size parameters.\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	ret = remap_pfn_range(vma, vma->vm_start,
		(((vcmd_mgr_t *)(subsys_mgr.vcmd_mgr))->mem_vcmd.mmap_pa >> PAGE_SHIFT) +
			vma->vm_pgoff, size, vma->vm_page_prot);
	if (ret != 0) {
		vdec_klog(LOGLVL_ERROR, "remap_pfn_range() failed.\n");
		return ret;
	}

	vcmd_klog(LOGLVL_DEBUG, "remap_pfn_range() success.\n");

	return ret;
}
#endif

/* VFS methods */
static const struct file_operations hantrodec_fops = {
	.owner = THIS_MODULE,
	.open = hantrodec_open,
	.release = hantrodec_release,
	.unlocked_ioctl = hantrodec_ioctl,
#ifdef SUPPORT_SMMU
	.mmap = hantrodec_mmap,
#endif
	.fasync = NULL
};

static struct miscdevice vdec_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRIVER_NAME,
	.fops = &hantrodec_fops,
};

/*
 *Function name   : hantro_probe
 *Description     : Initialize the driver

 *Return type     : int
 */
static int hantro_probe(struct platform_device *pdev)
{
	int result = 0, i, j;
	enum MMUStatus status = 0;
	enum MMUStatus mmu_status = MMU_STATUS_FALSE;
	volatile u8 *mmu_hwregs[MAX_SUBSYS_NUM][2];
	struct hantrodec_dev *dec_dev;
	int subsys_num;
	vdec_power_mgr *pm = NULL;
	vdec_clk_mgr *cm = NULL;
	dec_dev = devm_kzalloc(&pdev->dev, sizeof(*dec_dev), GFP_KERNEL);
	if (!dec_dev)
		return -ENOMEM;

	dec_dev->dev = &pdev->dev;

	/* initialize power_mgr instance, GET power, clock resource. */
	pm = vdec_power_mgr_init(dec_dev->dev);
	if (IS_ERR_OR_NULL(pm)) {
		vdec_init_klog(LOGLVL_ERROR, "Failed to init power manager: %p!\n", pm);
		return -EFAULT;
	}
	dec_dev->pm = pm;

	cm = vdec_clk_mgr_init(pm->vdec_clk->clk);
	if (IS_ERR_OR_NULL(cm)) {
		vdec_init_klog(LOGLVL_ERROR, "Failed to init clock manager: %p!\n", cm);
		goto err0;
	}
	dec_dev->cm = cm;

	/* vdec subsys and smmu power on */
	mutex_lock(&pm->pm_lock);
	result = inst_power_get(pm);
	if (result) {
		vdec_init_klog(LOGLVL_ERROR, "Failed to init other vdec_subsys hw resource: %d\n", result);
		mutex_unlock(&pm->pm_lock);
		goto err1;
	}

	/* vdec_core power on, clk enable and which freq set to work rate */
	result = vdec_core_do_power_on(pm);
	if (result) {
		vdec_init_klog(LOGLVL_ERROR, "vdec_core_do_power_on failed:%d\n", result);
		mutex_unlock(&pm->pm_lock);
		goto err2;
	}
	mutex_unlock(&pm->pm_lock);

	result = vdec_qos_cfg();
	if (result) {
		vdec_init_klog(LOGLVL_ERROR, "Failed to vdec qos cfg\n");
		goto err3;
	}

	vdec_init_klog(LOGLVL_INFO, "module start\n");

	result = memmap_set_device(&pdev->dev, XRING_VPU_DEC);
	if (result < 0) {
		vdec_init_klog(LOGLVL_ERROR, "vdecmem init failed!");
		goto err3;
	}

	if (!ParseVpuDtsiInfo(subsys_mgr.vpu_subsys, &subsys_num, &vcmd, pdev)) {
		vdec_init_klog(LOGLVL_INFO, "parse dec dts info success.");
	} else {
		vdec_init_klog(LOGLVL_WARNING, "parse dec dts info fail.");
	}

	result = mdr_vdec_exception_register(&pdev->dev);
	if (result)
		vdec_init_klog(LOGLVL_ERROR, "Failed to register mdr exception of vdec, %d\n", result);

	subsys_mgr.platformdev = pdev;
	of_dma_configure(&subsys_mgr.platformdev->dev,
		subsys_mgr.platformdev->dev.of_node, true);

	if (dma_set_mask_and_coherent(&subsys_mgr.platformdev->dev,
		DMA_BIT_MASK(64)))
		vdec_init_klog(LOGLVL_ERROR, "64bit hantrodma dev: No suitable DMA available\n");

	if (dma_set_coherent_mask(&subsys_mgr.platformdev->dev,
		DMA_BIT_MASK(64)))
		vdec_klog(LOGLVL_ERROR, "64bit hantrodma dev: No suitable DMA available\n");

	vdec_init_klog(LOGLVL_INFO, "hantrodec: dec/pp kernel module.\n");

	/* If base_port is set when insmod,
	 * use that for single core legacy mode.
	 */
	if (base_port != -1) {
		subsys_mgr.multicorebase[0] = base_port;
		subsys_mgr.elements = 1;
		subsys_mgr.vpu_subsys[0].base_addr = base_port;
		vdec_init_klog(LOGLVL_INFO, "hantrodec: Init single core at 0x%08lx IRQ=%i\n",
			subsys_mgr.multicorebase[0], subsys_mgr.irq[0]);
	} else {
		int i;

		memset(subsys_mgr.multicorebase, 0,
			sizeof(subsys_mgr.multicorebase[0]) * HXDEC_MAX_CORES);
		for (i = 0; i < subsys_num; i++) {
			subsys_mgr.multicorebase[i] = subsys_mgr.vpu_subsys[i].base_addr +
					subsys_mgr.vpu_subsys[i].submodule_offset[HW_VCD];
			subsys_mgr.irq[i] = subsys_mgr.vpu_subsys[i].irq;
			subsys_mgr.iosize[i] = subsys_mgr.vpu_subsys[i].submodule_iosize[HW_VCD];
			vdec_init_klog(LOGLVL_INFO, "hantrodec: [%d] multicorebase 0x%08lx, iosize %d\n",
					i, subsys_mgr.multicorebase[i], subsys_mgr.iosize[i]);
		}
		vdec_init_klog(LOGLVL_INFO, "hantrodec: Init multi :\n"
				" core[0] at 0x%16lx\n"
				" core[1] at 0x%16lx\n"
				" core[2] at 0x%16lx\n"
				" core[3] at 0x%16lx\n"
				" IRQ_0=%i\n"
				" IRQ_1=%i\n",
				subsys_mgr.multicorebase[0],
				subsys_mgr.multicorebase[1],
				subsys_mgr.multicorebase[2],
				subsys_mgr.multicorebase[3],
				subsys_mgr.irq[0],
				subsys_mgr.irq[1]);
	}

	subsys_mgr.hantrodec_data.cores = 0;

	subsys_mgr.hantrodec_data.iosize[0] = DEC_IO_SIZE_0;
	subsys_mgr.hantrodec_data.irq[0] = subsys_mgr.irq[0];
	subsys_mgr.hantrodec_data.iosize[1] = DEC_IO_SIZE_1;
	subsys_mgr.hantrodec_data.irq[1] = subsys_mgr.irq[1];

	for (i = 0; i < HXDEC_MAX_CORES; i++) {
		int j;

		for (j = 0; j < HW_CORE_MAX; j++)
			subsys_mgr.hantrodec_data.hwregs[i][j] = NULL;
		/* If user gave less core bases that we have
		 * by default,invalidate default bases
		 */
		if (subsys_mgr.elements && i >= subsys_mgr.elements)
			subsys_mgr.multicorebase[i] = 0;
	}

	subsys_mgr.hantrodec_data.async_queue_dec = NULL;
	subsys_mgr.hantrodec_data.async_queue_pp = NULL;

#ifdef CLK_CFG
	/* first get clk instance pointer */
	clk_cfg = clk_get(NULL, CLK_ID);
	if (!clk_cfg || IS_ERR(clk_cfg)) {
		vdec_init_klog(LOGLVL_ERROR, "get handrodec clk failed!\n");
		goto err3;
	}

	/* prepare and enable clk */
	if (clk_prepare_enable(clk_cfg)) {
		vdec_init_klog(LOGLVL_ERROR, "try to enable handrodec clk failed!\n");
		goto err3;
	}
	is_clk_on = 1;

	/* init a timer to disable clk */
	init_timer(&timer);
	timer.function = &hantrodec_disable_clk;
	/* the expires time is 100s */
	timer.expires = jiffies + 100 * HZ;
	add_timer(&timer);
#endif

	result = ReserveIO();
	if (result < 0)
		goto err3;

	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++)
		AXIFEEnable(subsys_mgr.hantrodec_data.hwregs[i][HW_AXIFE]);

	spin_lock_init(&subsys_mgr.owner_lock);
	/* read configuration fo all cores */
	ReadCoreConfig(&subsys_mgr.hantrodec_data);

	if (vcmd) {
		/* unmap and release mem region for VCMD,
		 * since it will be mapped and
		 * reserved again in hantro_vcmd.c
		 */
		for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
			if (subsys_mgr.hantrodec_data.hwregs[i][HW_VCD])
				vcmd_core_array[i].submodule_vcd_virtual_address =
					subsys_mgr.hantrodec_data.hwregs[i][HW_VCD];
			if (subsys_mgr.hantrodec_data.hwregs[i][HW_DEC400])
				vcmd_core_array[i].submodule_dec400_virtual_address =
					subsys_mgr.hantrodec_data.hwregs[i][HW_DEC400];
			if (subsys_mgr.hantrodec_data.hwregs[i][HW_MMU])
				vcmd_core_array[i].submodule_MMU_virtual_address =
					subsys_mgr.hantrodec_data.hwregs[i][HW_MMU];
			if (subsys_mgr.hantrodec_data.hwregs[i][HW_MMU_WR])
				vcmd_core_array[i].submodule_MMUWrite_virtual_address =
					subsys_mgr.hantrodec_data.hwregs[i][HW_MMU_WR];
			if (subsys_mgr.hantrodec_data.hwregs[i][HW_AXIFE])
				vcmd_core_array[i].submodule_axife_virtual_address =
					subsys_mgr.hantrodec_data.hwregs[i][HW_AXIFE];

			if (subsys_mgr.hantrodec_data.hwregs[i][HW_VCMD]) {
				iounmap((void __iomem *)subsys_mgr.hantrodec_data.hwregs[i][HW_VCMD]);
				release_mem_region(
				  subsys_mgr.vpu_subsys[i].base_addr +
				  subsys_mgr.vpu_subsys[i].submodule_offset[HW_VCMD],
				subsys_mgr.vpu_subsys[i].submodule_iosize[HW_VCMD]);
				subsys_mgr.hantrodec_data.hwregs[i][HW_VCMD] = NULL;
			}
		}
		platform_set_drvdata(pdev, dec_dev);
		subsys_mgr.vcmd_mgr = hantrovcmd_init(subsys_mgr.vpu_subsys,
				subsys_num, subsys_mgr.platformdev);
		if (!subsys_mgr.vcmd_mgr)
			goto err4;

		/* make sure vcmd_mgr data can be obtained by pm.suspend or pm.resume */
		dec_dev->priv_data = subsys_mgr.vcmd_mgr;
#ifdef CONFIG_DEC_PM
		if (hantrodec_pm_runtime_init(pdev, pm))
			vdec_pm_klog(LOGLVL_ERROR, "hantrodec pm runtime init failed!\n");
		vdec_init_klog(LOGLVL_INFO, "probe always_on[%d]!\n", always_on);
#endif
		/* power down VDEC_CORE/vdec_subsys when probe end */
		mutex_lock(&pm->pm_lock);
		pm->cur_always_on = always_on;
		vdec_core_do_power_off(pm);
		inst_power_put(pm);

		if (always_on) {
			result = _power_always_on_all(pm);
			if (result) {
				vdec_pm_klog(LOGLVL_ERROR, "always_on enable at failed");
				mutex_unlock(&pm->pm_lock);
				goto err4;
			}
			vdec_pm_klog(LOGLVL_ERROR, "always_on enable at success");
		}
		mutex_unlock(&pm->pm_lock);

		result = misc_register(&vdec_misc);
		if (result < 0) {
			vdec_init_klog(LOGLVL_ERROR, "hantrodec: misc_register fail, ret=%d\n", result);
			goto err4;
		}

		vdec_init_klog(LOGLVL_INFO, "hantrodec process OK!");
		return 0;
	} else {
		result = misc_register(&vdec_misc);
		if (result < 0) {
			vdec_klog(LOGLVL_ERROR, "hantrodec: %s: misc_register fail, ret=%d\n", __func__, result);
			goto err4;
		}
	}

	vdec_init_klog(LOGLVL_WARNING, "hantrodec process shouldn't go here !!!");


err4:
	ReleaseIO();
err3:
	mutex_lock(&pm->pm_lock);
	vdec_core_do_power_off(pm);
	mutex_unlock(&pm->pm_lock);
err2:
	mutex_lock(&pm->pm_lock);
	inst_power_put(pm);
	mutex_unlock(&pm->pm_lock);
err1:
	vdec_clk_mgr_deinit(cm);
	dec_dev->cm = NULL;
err0:
	vdec_power_mgr_deinit(pm);
	dec_dev->pm = NULL;
	vdec_init_klog(LOGLVL_ERROR, "hantrodec: module not inserted\n");
	return result;
}

static int hantro_remove(struct platform_device *pdev)
{
	hantrodec_t *dev = &subsys_mgr.hantrodec_data;
	int i, n = 0, result = 0;
	u8 *mmu_hwregs[MAX_SUBSYS_NUM][2];
	int has_mmu = 0;
	struct hantrodec_dev *dec_dev;

	dec_dev = platform_get_drvdata(subsys_mgr.platformdev);

	if (!dec_dev) {
		vdec_klog(LOGLVL_ERROR, "hantrodec remove: dec_dev is NULL!\n");
		return -EINVAL;
	}
	WARN_ON(!dec_dev->pm);

	for (i = 0; i < MAX_SUBSYS_NUM; i++) {
		mmu_hwregs[i][0] = dev->hwregs[i][HW_MMU];
		mmu_hwregs[i][1] = dev->hwregs[i][HW_MMU_WR];
		if (dev->hwregs[i][HW_DEC400]) {
			/* disable dec400 when rmmod driver. */
#ifndef CONFIG_DEC_PM
			iowrite32(0x00810002,
				  (void __iomem *)(dev->hwregs[i][HW_DEC400] +
				 0x800));
#endif
		}
		if (dev->hwregs[i][HW_MMU])
			has_mmu = 1;
	}

	if (vcmd) {
		if (subsys_mgr.vcmd_mgr) {
			hantrovcmd_cleanup(subsys_mgr.vcmd_mgr);
			subsys_mgr.vcmd_mgr = NULL;
			dec_dev->priv_data = NULL;
		}
	} else {
		/* reset hardware */
		ResetAsic(dev);

		/* free the IRQ */
		for (n = 0; n < dev->cores; n++) {
			if (dev->irq[n] != -1)
				free_irq(dev->irq[n], (void *)dev);
		}
	}

	ReleaseIO();
#ifdef SUPPORT_DBGFS
	debugfs_remove_recursive(debug_root);
	debug_root = NULL;
#endif

#ifdef CLK_CFG
	if (clk_cfg && !IS_ERR(clk_cfg)) {
		clk_disable_unprepare(clk_cfg);
		is_clk_on = 0;
		vdec_klog(LOGLVL_INFO, "turned off hantrodec clk\n");
	}

	/*delete timer*/
	del_timer(&timer);
#endif

	/* must be poweroff after rmmod */
	always_on = 0;
	/* process abnormal power state(on), force power off if power is on. */
	vdec_force_power_off(dec_dev->pm, POWER_VDEC_CORE);
	vdec_force_power_off(dec_dev->pm, POWER_VDEC_SUBSYS);
	/* power count will be clear to zero here. */
	vdec_power_mgr_deinit(dec_dev->pm);
	dec_dev->pm = NULL;
	vdec_clk_mgr_deinit(dec_dev->cm);
	dec_dev->cm = NULL;
	misc_deregister(&vdec_misc);

	result = mdr_vdec_exception_unregister(&pdev->dev);
	if (result) {
		vdec_klog(LOGLVL_ERROR, "Failed to unregister mdr exception of vdec, %d\n", result);
	}
	return 0;
}

static struct platform_driver hantro_driver = {
	.probe  = hantro_probe,
	.remove  = hantro_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner  = THIS_MODULE,
		.of_match_table = vdec_of_match,
#ifdef CONFIG_DEC_PM
		.pm = &hantrodec_pm_ops,
#endif
	},
	.id_table = hantro_ids,
};

/*
 *Function name   : hantrodec_init
 *Description     : Initialize the driver

 *Return type     : int
 */

static int __init hantrodec_init(void)
{
	int ret;
	/*
	 * @Xring:
	 * pm_runtime_init is excuted in platform_device_register_full, Will platform_driver_register also do this?
	 * maybe we need do platform_device_register_full before platform_driver_register.
	 * subsys_mgr.platformdev = platform_device_register_full(&hantro_platform_info);
	*/
	ret = platform_driver_register(&hantro_driver);
	if (ret) {
		vdec_klog(LOGLVL_INFO, "hantrodec_init failed, ret[%d]\n", ret);
		return ret;
	}
	return 0;

}

/*
 * Function name   : hantrodec_cleanup
 * Description     : clean up
 * Return type     : int
 */
static void __exit hantrodec_cleanup(void)
{
#ifdef CONFIG_DEC_PM
	hantrodec_pm_runtime_deinit(subsys_mgr.platformdev);
#endif
	/* platform_device_unregister */
	platform_driver_unregister(&hantro_driver);
	vdec_klog(LOGLVL_INFO, "vpu hantrodec: module removed\n");

}

/*
 *Function name   : CheckHwId
 *Return type     : int
 */
static int CheckHwId(hantrodec_t *dev)
{
	int hwid;
	int i, j;
	size_t num_hw = sizeof(DecHwId) / sizeof(*DecHwId);

	int found = 0;

	for (i = 0; i < dev->cores; i++) {
		for (j = 0; j < HW_CORE_MAX; j++) {
			if ((j == HW_VCD || j == HW_VCDJ) &&
			    dev->hwregs[i][j]) {
				hwid = readl((volatile void __iomem *)
					dev->hwregs[i][j]);
				vdec_klog(LOGLVL_INFO, "hantrodec: core %d:%d HW ID=0x%08x [ %s ]\n",
					i, j, hwid, CoreTypeStr(j));
				/* product version only */
				hwid = (hwid >> 16) & 0xFFFF;
				while (num_hw--) {
					if (hwid == DecHwId[num_hw]) {
						vdec_klog(LOGLVL_INFO, "hantrodec: Supported HW found at 0x%16lx\n",
							subsys_mgr.vpu_subsys[i].base_addr +
							subsys_mgr.vpu_subsys[i].submodule_offset[j]);
						found++;
						dev->hw_id[i][j] = hwid;
						break;
					}
				}

				if (!found) {
					vdec_klog(LOGLVL_INFO, "hantrodec: Unknown HW found at 0x%16lx\n",
						subsys_mgr.multicorebase[i]);
					return 0;
				}

				found = 0;
				num_hw = sizeof(DecHwId) / sizeof(*DecHwId);
			}
		}
	}

	return 1;
}

/*
 * Function name   : ReserveIO
 * Description     : IO reserve
 * Return type     : int
 */
static int ReserveIO(void)
{
	int i, j;
	long hwid;
	u32 axife_config;
	u32 vcmd_offset;
	u32 vcmd_iosize;
	volatile u8 *vcmd_hwregs;

	memcpy((unsigned int *)(subsys_mgr.hantrodec_data.iosize), subsys_mgr.iosize,
	       HXDEC_MAX_CORES * sizeof(unsigned int));
	memcpy((unsigned int *)(subsys_mgr.hantrodec_data.irq), subsys_mgr.irq,
	       HXDEC_MAX_CORES * sizeof(int));
	for (i = 0; i < MAX_SUBSYS_NUM; i++) {
		if (!subsys_mgr.vpu_subsys[i].base_addr)
			continue;

		// Check if APB arbiter is existed

		if (vcmd) {
			vcmd_offset = subsys_mgr.vpu_subsys[i].submodule_offset[HW_VCMD];
			vcmd_iosize = subsys_mgr.vpu_subsys[i].submodule_iosize[HW_VCMD];
		} else {
			vcmd_offset = 0;
			vcmd_iosize = 30 * 4;
		}
		if (!request_mem_region(subsys_mgr.vpu_subsys[i].base_addr + vcmd_offset, vcmd_iosize, "vcmd_arbiter_driver")) {
			vdec_klog(LOGLVL_INFO, "hantrodec: failed to reserve VCMD regs\n");
			return -EBUSY;
		}

		vcmd_hwregs = (volatile u8 __force *)ioremap(subsys_mgr.vpu_subsys[i].base_addr + vcmd_offset, vcmd_iosize);

		if (ioread32((void __iomem *)(vcmd_hwregs + 0x3C)) & 0x400)
			has_apb_arbiter = 1;
		if (has_apb_arbiter) {
			vcmd_online_for_arbiter(vcmd_hwregs);
			vcmd_request_arbiter(vcmd_hwregs);
		}
		if (vcmd_hwregs) {
			iounmap((void __iomem *)vcmd_hwregs);
			release_mem_region(subsys_mgr.vpu_subsys[i].base_addr + vcmd_offset, vcmd_iosize);
			vcmd_hwregs = NULL;
		}

		for (j = 0; j < HW_CORE_MAX; j++) {
			if (subsys_mgr.vpu_subsys[i].submodule_iosize[j]) {
				vdec_klog(LOGLVL_INFO, "hantrodec: base=0x%16lx, iosize=%d\n",
					subsys_mgr.vpu_subsys[i].base_addr +
					subsys_mgr.vpu_subsys[i].submodule_offset[j],
					subsys_mgr.vpu_subsys[i].submodule_iosize[j]);

				if (!request_mem_region(subsys_mgr.vpu_subsys[i].base_addr +
					subsys_mgr.vpu_subsys[i].submodule_offset[j],
					subsys_mgr.vpu_subsys[i].submodule_iosize[j],
						"hantrodec0")) {
					vdec_klog(LOGLVL_INFO, "hantrodec: failed to reserve HW %d regs\n", j);
					return -EBUSY;
				}
				/* Why do we need two structures to describe the same object? */
				subsys_mgr.vpu_subsys[i].submodule_hwregs[j] =
					subsys_mgr.hantrodec_data.hwregs[i][j] =
						(volatile u8 *)ioremap(subsys_mgr.vpu_subsys[i].base_addr +
							subsys_mgr.vpu_subsys[i].submodule_offset[j],
							subsys_mgr.vpu_subsys[i].submodule_iosize[j]);

				if (!subsys_mgr.hantrodec_data.hwregs[i][j]) {
					vdec_klog(LOGLVL_INFO, "hantrodec: failed to ioremap HW %d regs\n", j);
					release_mem_region(
						subsys_mgr.vpu_subsys[i].base_addr +
						subsys_mgr.vpu_subsys[i].submodule_offset[j],
						subsys_mgr.vpu_subsys[i].submodule_iosize[j]);
					return -EBUSY;
				}
				if (j == HW_AXIFE) {
					hwid = ioread32((void __iomem *)
						(subsys_mgr.hantrodec_data.hwregs[i][HW_VCD] +
						HANTRODEC_HW_BUILD_ID_OFF));
					axife_config = ioread32((void __iomem *)
						(subsys_mgr.hantrodec_data.hwregs[i][j]));
					subsys_mgr.axife_cfg[i].axi_rd_chn_num = axife_config & 0x7F;
					subsys_mgr.axife_cfg[i].axi_wr_chn_num = (axife_config >> 7) & 0x7F;
					subsys_mgr.axife_cfg[i].axi_rd_burst_length = (axife_config >> 14) & 0x1F;
					subsys_mgr.axife_cfg[i].axi_wr_burst_length = (axife_config >> 22) & 0x1F;
					subsys_mgr.axife_cfg[i].fe_mode = 0; /*need to read from reg in furture*/
					if (hwid == 0x1F66)
						subsys_mgr.axife_cfg[i].fe_mode = 1;
				}

				vdec_klog(LOGLVL_INFO, "hantrodec: HW %d reg[0]=0x%08x [ %s ]\n",
					j,
					readl((volatile void __iomem *) subsys_mgr.hantrodec_data.hwregs[i][j]),
					CoreTypeStr(j));
			} else {
				subsys_mgr.hantrodec_data.hwregs[i][j] = NULL;
			}
		}
		subsys_mgr.hantrodec_data.cores++;
	}

	/* check for correct HW */
	if (!CheckHwId(&subsys_mgr.hantrodec_data)) {
		ReleaseIO();
		return -EBUSY;
	}

	return 0;
}

/*
 * Function name   : releaseIO
 * Description     : release
 * Return type     : void
 */

static void ReleaseIO(void)
{
	int i, j;

	for (i = 0; i < subsys_mgr.hantrodec_data.cores; i++) {
		for (j = 0; j < HW_CORE_MAX; j++) {
			if (subsys_mgr.hantrodec_data.hwregs[i][j]) {
				iounmap((void __iomem *)
				subsys_mgr.hantrodec_data.hwregs[i][j]);
				release_mem_region(subsys_mgr.vpu_subsys[i].base_addr +
				  subsys_mgr.vpu_subsys[i].submodule_offset[j],
				  subsys_mgr.vpu_subsys[i].submodule_iosize[j]);
				subsys_mgr.hantrodec_data.hwregs[i][j] = NULL;
			}
		}
	}
}

/*
 * Function name   : hantrodec_isr
 * Description     : interrupt handler

 * Return type     : irqreturn_t
 */
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t hantrodec_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t hantrodec_isr(int irq, void *dev_id)
#endif
{
	unsigned long flags;
	unsigned int handled = 0;
	int i;
	volatile u8 *hwregs;
	#ifdef SUPPORT_DBGFS
		u64 time_num;
		struct timeval time_now;
	#endif
	hantrodec_t *dev = (hantrodec_t *)dev_id;
	u32 irq_status_dec;

	spin_lock_irqsave(&subsys_mgr.owner_lock, flags);

	for (i = 0; i < dev->cores; i++) {
		volatile u8 *hwregs = dev->hwregs[i][HW_VCD];

		/* interrupt status register read */
		irq_status_dec = ioread32((void __iomem *)
			(hwregs + HANTRODEC_IRQ_STAT_DEC_OFF));

		if ((irq_status_dec >> 22) == 1) {
		    irq_status_dec &=  0xFFBFFFFF;
		    irq_status_dec &= (~HANTRODEC_DEC_IRQ);
		    iowrite32(irq_status_dec,
				  (void __iomem *)(hwregs +
				 HANTRODEC_IRQ_STAT_DEC_OFF));
		    wake_up_interruptible_all(&subsys_mgr.dec_wait_queue);
			handled++;
		} else if (irq_status_dec & HANTRODEC_DEC_IRQ) {
#ifdef SUPPORT_WATCHDOG
			//if there is an interrupt from VCD, should stop the watchdog.
			_watchdog_stop(&dev->watchdog[i]);
#endif
#ifdef SUPPORT_DBGFS
			subsys_mgr.decode_state[i] = DONE;
#if (KERNEL_VERSION(4, 19, 94) > LINUX_VERSION_CODE)

			do_gettimeofday(&time_now);
			time_num = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
#else
			struct timespec64 time_now;

			ktime_get_real_ts64(&time_now);
			time_num = time_now.tv_sec * 1000 + time_now.tv_nsec / 1000000;
#endif
			subsys_mgr.hw_return_time[i][subsys_mgr.hw_r_index[i]] = time_num;
			subsys_mgr.hw_r_index[i] = subsys_mgr.hw_r_index[i] + 1;
			if (subsys_mgr.hw_r_index[i] == MAX_RESERVED_TIME)
				subsys_mgr.hw_r_index[i] = 0;
#endif
		/* clear dec IRQ */

			irq_status_dec &= (~HANTRODEC_DEC_IRQ);
			iowrite32(irq_status_dec,
				  (void __iomem *)(hwregs +
				 HANTRODEC_IRQ_STAT_DEC_OFF));

			PDEBUG("decoder IRQ received! core %d\n", i);
#ifdef CONFIG_DEC_PM
			dev->hw_active[i] = 0;
#endif
			atomic_inc(&subsys_mgr.irq_rx);

			subsys_mgr.dec_irq |= (1 << i);

			wake_up_interruptible_all(&subsys_mgr.dec_wait_queue);
			handled++;
		}
	}

	spin_unlock_irqrestore(&subsys_mgr.owner_lock, flags);

	if (!handled)
		PDEBUG("IRQ received, but not hantrodec's!\n");

	(void)hwregs;
	return IRQ_RETVAL(handled);
}

/*
 * Function name   : ResetAsic
 * Description     : reset asic (only VC8000D supports reset)

 * Return type     :
 */
static void ResetAsic(hantrodec_t *dev)
{
	int i, j;
	u32 status;

	for (j = 0; j < dev->cores; j++) {
		if (!dev->hwregs[j][HW_VCD])
			continue;

		status = ioread32((void __iomem *)(dev->hwregs[j][HW_VCD] +
						  HANTRODEC_IRQ_STAT_DEC_OFF));

		if (status & HANTRODEC_DEC_E) {
			/* abort with IRQ disabled */
			status = HANTRODEC_DEC_ABORT |
					 HANTRODEC_DEC_IRQ_DISABLE;
			iowrite32(status, (void __iomem *)
					  (dev->hwregs[j][HW_VCD] +
					  HANTRODEC_IRQ_STAT_DEC_OFF));
		}

		if (IS_G1(dev->hw_id[j][HW_VCD]))
			/* reset PP */
			iowrite32(0, (void __iomem *)
					  (dev->hwregs[j][HW_VCD] +
					  HANTRO_IRQ_STAT_PP_OFF));

		for (i = 4; i < dev->iosize[j]; i += 4)
			iowrite32(0, (void __iomem *)
					  (dev->hwregs[j][HW_VCD] +
					  i));
	}
}

/*
 * Function name   : dump_regs
 * Description     : Dump registers

 * Return type     :
 */
#ifdef HANTRODEC_DEBUG
void dump_regs(hantrodec_t *dev)
{
	int i, c;

	PDEBUG("Reg Dump Start\n");
	for (c = 0; c < dev->cores; c++) {
		for (i = 0; i < dev->iosize[c]; i += 4 * 4) {
			PDEBUG("\toffset %04X: %08X  %08X  %08X  %08X\n", i,
			       ioread32(dev->hwregs[c][HW_VCD] + i),
	ioread32(dev->hwregs[c][HW_VCD] + i + 4),
	ioread32(dev->hwregs[c][HW_VCD] + i + 16),
	ioread32(dev->hwregs[c][HW_VCD] + i + 24));
		}
	}
	PDEBUG("Reg Dump End\n");
}
#endif

/**
 * @brief reset for hantro dec
 */
int hantrodec_reset(struct device *dev, bool vcmd_dec)
{
	int ret = 0;
	unsigned long flags;
	struct hantrodec_dev *dec_dev = dev_get_drvdata(dev);
	vcmd_mgr_t *vcmd_mgr = (vcmd_mgr_t *)dec_dev->priv_data;
	struct hantrovcmd_dev *vdev = &vcmd_mgr->dev_ctx[0];
	vdec_power_mgr *pm = dec_dev->pm;

	WARN_ON(!dec_dev);
	vdec_klog(LOGLVL_WARNING, "vdec reset start\n");

	if ((atomic_read(&pm->power_count[POWER_VDEC_CORE]) || pm->cur_always_on) &&
			regulator_is_enabled(pm->vdec_rg)) {
		pm->reset = 1;
		/* power off vdec core & subsys */
		vdec_core_do_power_off(pm);
		inst_power_put(pm);

		/* power on vdec core & subsys */
		ret = inst_power_get(pm);
		if (ret) {
			vdec_klog(LOGLVL_ERROR, "vdec_subsys inst power up failed:%d\n", ret);
			goto err;
		}

		ret = vdec_core_do_power_on(pm);
		if (ret) {
			vdec_klog(LOGLVL_ERROR, "vdec_core_do_power_on failed:%d\n", ret);
			goto err;
		}

		if (vcmd_dec)
			ret = vcmd_pm_resume(dec_dev->priv_data);
		else
			ret = dec_pm_resume(dec_dev->priv_data);

		pm->reset = 0;
		vdec_klog(LOGLVL_WARNING, "vdec reset finish, vdec_subsys power count: %d, "
			"vdec power count: %d, dev state %d\n",
			atomic_read(&pm->power_count[POWER_VDEC_SUBSYS]),
			atomic_read(&pm->power_count[POWER_VDEC_CORE]),
			vdev->state);
	} else {
		vdec_klog(LOGLVL_WARNING, "vdec has been powered off, no need to reset\n");
	}
	return ret;
err:
	pm->reset = 0;
	vdec_klog(LOGLVL_ERROR, "vdec reset error");
	mdr_system_error(MODID_VPU_DEC_HANG_EXCEPTION, 0, 0);
	return ret;
}


module_init(hantrodec_init);
module_exit(hantrodec_cleanup);

/* module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VeriSilicon Microelectronics ");
MODULE_DESCRIPTION("driver module for Hantro video decoder VCD");
