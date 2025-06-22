
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

#ifndef _HANTRO_VCMD_PRIV_H
#define _HANTRO_VCMD_PRIV_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __FREERTOS__
#include "basetype.h"
#include "dev_common_freertos.h"
#elif defined(__linux__)
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/types.h>
#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/kfifo.h>
#include <linux/version.h>
#include <linux/ktime.h>
#endif

#include "bidirect_list.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "vcmdswhwregisters.h"
#else
#include <dt-bindings/xring/platform-specific/vpu/vdec/vcmdswhwregisters.h>
#endif
#include "subsys.h"
#include <linux/version.h>
#include <display/xring_xplayer_uapi.h>
#include "vdec_log.h"

#define MAX_VCMD_NUM			(MAX_SUBSYS_NUM)

#define REG_ID_CMDBUF_EXE_CNT	(3)
#define REG_ID_CMDBUF_EXE_ID	(26)
/* read vcmd swreg26 (CMDBUF_EXE_ID) to reg-buf[REG_ID2_CMDBUF_EXE_ID] at
	beginning of each cmdbuf to indicate which cmdbuf is executing in reg-buf*/
#define REG_ID2_CMDBUF_EXE_ID	(REG_ID_CMDBUF_EXE_ID + 1)

#define VCMD_REG_NUM_READ			(27)

#define _ALIGN(X, Factor) ((((X) + ((Factor)-1)) / (Factor)) * (Factor))
#define ALIGN_4K(X) _ALIGN(X, 4096)

#define VCMD_STATE_POWER_OFF	0
#define VCMD_STATE_POWER_ON		1
#define VCMD_STATE_IDLE			2
#define VCMD_STATE_WORKING		3

#define HW_WORK_STATE_IDLE		0
#define HW_WORK_STATE_WORK		1
#define HW_WORK_STATE_STALL		2
#define HW_WORK_STATE_PEND		3
#define HW_WORK_STATE_ABORT		4

#define _GET_PO(filp)	((struct proc_obj *)((struct vcmd_priv_ctx *)filp->private_data)->po)

#define CMDBUF_OFF(id)		((id) * SLOT_SIZE_CMDBUF)
#define STATUSBUF_OFF(id)	((id) * SLOT_SIZE_STATUSBUF)
#define CMDBUF_OFF_32(id)		(((id) * SLOT_SIZE_CMDBUF) / sizeof(u32))
#define STATUSBUF_OFF_32(id)	(((id) * SLOT_SIZE_STATUSBUF) / sizeof(u32))

/* VCE: 0; VCD: 8 */
#define ENC_INTR_SRC_BIT                         0
#define DEC_INTR_SRC_BIT                         8
#define ABNORMAL_IRQ_BIT_MASK(A)                 ((A)+16)
#define DEC_ABNORMAL_IRQ_MASK                                      \
	(1 << ABNORMAL_IRQ_BIT_MASK(DEC_INTR_SRC_BIT))
/* abnormal irq schedule */
#define ABN_IRQ_SCHEDULE_BUFFER_EMPTY (0x1)

/*offset of vcmd register*/
enum {
	VCMD_INTERRUPT_SRC = 2 * 4,			//whether vcmd has received an interrupt
	VCMD_WORK_STATE = 15 * 4,			//bit 0-2: vcmd work state
	VCMD_ABORT_STATE = 16 * 4, 			//bit 0: start trigger, bit 3: abort mode
	VCMD_INTERRUPT_TRIGGER = 17 * 4,	//whether vcmd has triggered an interrupt
	VCMD_INTERRUPT_ENABLE = 18 * 4,		//whether vcmd interrupt can be sent to cpu
	VCMD_INTERRUPT_GATE = 2 * 4,		//whether IPs interrupt can be sent to cpu
};

/*offset of vc9000d register*/
enum {
	VC9000D_INTERRUPT = 0x4,
	VC9000D_LUMA_BUFFER_MSB = 0x108,
	VC9000D_LUMA_BUFFER_LSB = 0x104,
	VC9000D_CHROMA_BUFFER_MSB = 0x188,
	VC9000D_CHROMA_BUFFER_LSB = 0x18C,
	VC9000D_STREAM_BUFFER_MSB = 0x2A0,
	VC9000D_STREAM_BUFFER_LSB = 0x2A4,
	VC9000D_SLICE_TEXT_1_BUFFER_MSB = 0x688, //bits 16:23
	VC9000D_SLICE_TEXT_1_BUFFER_LSB = 0x68C,
	VC9000D_SLICE_TEXT_2_BUFFER_MSB = 0x6A4, //bits 16:23
	VC9000D_SLICE_TEXT_2_BUFFER_LSB = 0x6A8,
	VC9000D_SLICE_TEXT_3_BUFFER_MSB = 0x6C0, //bits 16:23
	VC9000D_SLICE_TEXT_3_BUFFER_LSB = 0x6C4,
	VC9000D_SLICE_TEXT_4_BUFFER_MSB = 0x6DC, //bits 16:23
	VC9000D_SLICE_TEXT_4_BUFFER_LSB = 0x6E0,
};

/*brief dump register tag*/
enum {
	DUMP_VCMD_REG = 1,
	DUMP_CORE_REG = 1 << 1,
	DUMP_AXIFE_REG = 1 << 2,
	DUMP_STATE_REG = 1 << 3,
	DUMP_INTERRUPT_REG = 1 << 4,
	DUMP_BASE_INFO = 1 << 5,
};

/* declarations */
extern int vsi_dumplvl;

#define vcmd_ftrace(fmt, ...)
#define vcmd_wfd_trace(fmt, ...) vcmd_klog(LOGLVL_DEBUG, "WFD_SLICE: " fmt, ##__VA_ARGS__)


#define SEMA_RET_NONE
#define _sema_down(sem, ret) {               \
	if (down_interruptible(sem)) {     \
		vcmd_klog(LOGLVL_ERROR, "%s: sema-down is interrupted!", __func__); \
		return ret;                                                         \
	}                                                                       \
}
#define _sema_up(sem) up(sem)
#define CMDBUF_ID_CHECK(id) \
	({ \
		int valid = 0; \
		if ((id) >= SLOT_NUM_CMDBUF) { \
			vcmd_klog(LOGLVL_ERROR, "%s: error cmdbuf id!\n", __func__); \
			valid = -1; \
		} \
		valid; \
	})

/* ------------------------------------------------------
 * watch-dog state and feed mode definition
 -------------------------------------------------------*/
#ifdef SUPPORT_WATCHDOG
enum WD_STATE {
	WD_STATE_NONE, // a init state
	WD_STATE_ACTIVE, // the watch-dog has been activated
	WD_STATE_PAUSE, // the watch-dog has been paused
	WD_STATE_STOPPED // the watch-dog has been stopped
};

enum WD_FEED_MODE {
	WD_FEED_ACTIVE, // feed watch-dog
	WD_FEED_APPEND, // feed a active or stopped state watch-dog
	WD_FEED_RESUME // feed a pause state watch-dog
};
#endif

/* ------------------------------------------------------
 * vcmd timer definition
 -------------------------------------------------------*/
enum vcmd_timer_id {
	VCMD_TIMER_TIMEOUT = 0x0,
	VCMD_TIMER_BUFFER_EMPTY,
	VCMD_TIMER_WATCHDOG,

	VCMD_TIMER_MAX
};

struct vcmd_timer {
	struct timer_list timer;
	enum vcmd_timer_id id;
	u8 active;
	u8 expired;
	void *owner;

	unsigned long period;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	void (*func)(unsigned long );
#else
	void (*func)(struct timer_list *);
#endif
};

/* ------------------------------------------------------
*  single linked list
--------------------------------------------------------*/
typedef struct si_linked_node {
	struct si_linked_node *next;
	void *data;
}si_linked_node;

struct si_linked_list {
	spinlock_t spinlock;
	struct si_linked_node *head;
	struct si_linked_node *tail;
	struct si_linked_node nodes[SLOT_NUM_CMDBUF];
};

struct proc_obj {
	u64 total_workload;
	spinlock_t spinlock;
	wait_queue_head_t resource_waitq;
	wait_queue_head_t job_waitq;
	u32 module_type; 	/* correspond to filp, indicates subsys type */

	spinlock_t job_lock;
	struct file *filp;
	struct bi_list job_done_list;
	u32 in_wait;		/* user is waiting for a specified cmdbuf run done */
	struct xplayer_cmdlist_addr xplayer_info;
	dma_addr_t xplayer_iova[4];
	u32 vcmd_get_slice_info_flags;
};


struct noncache_mem {
	u32 *va;
	u32 size;
	dma_addr_t pa;		/* buffer physical address */
	phys_addr_t mmap_pa;
	size_t mmu_ba; 	/* buffer bus address in MMU*/
};


struct dec_map_info{
	unsigned long iova;
	unsigned long offset;
	u32 *va;
	int len;
	struct dma_buf *dma_buf;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
	struct dma_buf_map map;
#else
	struct iosys_map map;
#endif
};

struct cmdbuf_obj {
	/* current CMDBUF type: input vce=0,
	 * IM=1,vcd=2, pege=3, jpegd=4
	 */
	u32 module_type;
	/* current CMDBUFpriority: normal=0, high=1 */
	u32 priority;
	/* workload = encoded_image_size*(rdoLevel+1)*(rdoq+1);*/
	u64 workload;
	u64 interrupt_ctrl;
	/* current CMDBUF size, change to payload after linked  */
	u32 cmdbuf_size;
	/* current CMDBUF virtual address.*/
	u32 *cmd_va;
	/*current status CMDBUF virtual address.*/
	u32 *status_va;
	/*current status CMDBUF size*/
	u32 status_size;
	/*current CMDBUF executing status.*/
	u32 executing_status;
	/* which vcmd core is assigned to.*/
	u16 core_id;
	/* used to manage CMDBUF in driver.It is a handle
	 * to identify cmdbuf.also is an interrupt vector.
	 * position in pool,same as status position.
	 */
	u16 cmdbuf_id;
	/* cmdbuf node is in dev->work_list (1) or not (0) */
	u8 cmdbuf_linked;
	/* if 0, waiting for CMDBUF finish;
	 * if 1, op code in CMDBUF has finished one by one.
	 * HANTRO_VCMD_IOCH_WAIT_CMDBUF will check this variable.
	 */
	u8 cmdbuf_run_done;
	/* if 0, wait slice ready
	 * if 1, slice has ready, but the job has not been added to job_done_list.
	 * if 2, slice has ready, and the job has been added to job_done_list.
	 */
	u8 slice_run_done;
	/* if 0, not need to remove CMDBUF;
	 * if 1, CMDBUF can be removed if it is not used by vcmd hw;
	 */
	u8 cmdbuf_need_remove;
	/* cmdbuf node has been processed (1) or not (0) */
	u8 cmdbuf_processed;
	/* The last command is JMP (1) or END (0). */
	u8 has_jmp_cmd;
	/* JMP command will trigger JMP interrupt (1) or not. */
	u8 jmp_ie;
	/* the cmdbuf is occupied (1) or not (in free_obj_list). */
	u8 is_occupied;
	/* current CMDBUF physical address. */
	addr_t cmd_pa;
	/*current status CMDBUF physical address.*/
	addr_t status_pa;

	/* which cores can be uesd.*/
	u32 core_mask;
	/*file pointer occupying this obj.*/
	struct file *filp;
	/* pointer of process object occupying this obj*/
	struct proc_obj *po;
	/* pointer of the instance of process */
	void *owner;
	//current status CMDBUF mmu mapping address.
	size_t mmu_status_ba;
	//current CMDBUF mmu mapping address.
	size_t mmu_cmd_ba;
	// when cmdbufs aborted, whether to need restart vcmd
	atomic_t need_restart;

	u32 xring_slice_mode;
	struct dec_map_info slice_outbuf;

	//hw frequency for this cmdbuf
	u64 clk_rate;
};

enum subsys_module_id {
	SUB_MOD_VCMD = 0,
	SUB_MOD_MAIN,
	SUB_MOD_MMU,
	SUB_MOD_MMU_WR,
	SUB_MOD_DEC400,
	SUB_MOD_AXIFE,
	SUB_MOD_UFBC,

	SUB_MOD_MAX
};

enum vcmd_sw_init_mode {
	VCMD_INIT_NORMAL = 0,		//Not run init commands
	VCMD_INIT_TRIGGER = 1,		//Run init commands only when it is triggered
	VCMD_INIT_EXE_CMDBUF = 2,	//Run init commands before execution of each CMDBUF
};

enum vcmd_slice_config_id {
	VCMD_PP0_OUTPUT_AFBC_ENABLE = 401,			/* bit 26 of swreg 384 */
	VCMD_PP0_OUTPUT_BUFFER_MSB = 402,			/* Base address MSB for output buffer */
	VCMD_PP0_OUTPUT_BUFFER_LSB = 403,			/* Base address LSB for output buffer */
	VCMD_PP0_OUTPUT_BUFFER_PLAN2_MSB = 404,		/* Base address MSB for chroma or G */
	VCMD_PP0_OUTPUT_BUFFER_PLAN2_LSB = 405,		/* Base address LSB for chroma or G chanl*/
	VCMD_PP0_OUTPUT_BUFFER_PLAN3_MSB = 406,		/* Base address MSB for B */
	VCMD_PP0_OUTPUT_BUFFER_PLAN3_LSB = 407,		/* Base address LSB for B */
	VCMD_SLICE_LINE_CNT_CTRL = 429,			/* PP0 output pixel line count control */
	VCMD_SLICE0_ADDR_MSB = 435,				/* Base address MSB for slice0 address (bit[31:16]) */
											/* and line cnt (bit[15:0]) */
	VCMD_SLICE0_ADDR_LSB = 436,				/* Base address LSB for slice0 address */
	VCMD_SLICE0_TEXT0 = 438,				/* Text0 of slice 0 to be written to external memory */
};

#define DEFAULT_VCMD_INIT_MODE VCMD_INIT_TRIGGER

struct vcmd_subsys_info {
	addr_t reg_base;		/* physical reg base addr of subsys */
	int irq;				/* irq of vcmd */

	/*input vce=0,IM=1,vcd=2,jpege=3, jpegd=4*/
	u32 sub_module_type;	/* type of main module */

	u16 reg_off[SUB_MOD_MAX];	/* reg offset of each module */
	u16 io_size[SUB_MOD_MAX]; /* reg size of each module, in bytes*/

	/* for dbgfs */
	volatile u8 *hwregs[SUB_MOD_MAX];
};

struct vcmd_hw_features {
	u8 has_arbiter;	//hw support APB arbiter (1) or not (0)
	u8 has_init_mode;	//hw support init-mode (1) or not (0)
};

struct hantrovcmd_dev {
	//config of each core,such as base addr, irq,etc
	struct vcmd_subsys_info *subsys_info;
	u32 hw_version_id;
	//vcmd core id for driver and sw internal use
	u32 core_id;
	u32 cmd_buf_id;
	struct vcmd_hw_features hw_feature;
	volatile u8 *hwregs; /* registers IO mem base */
	u32 reg_mirror[ASIC_VCMD_SWREG_AMOUNT];

	spinlock_t owner_lock_vcmd;     // mainly for HW related operation
	spinlock_t *spinlock;           //point to owner_lock_vcmd
	spinlock_t owner_po_lock;       //protect po release
	spinlock_t *po_spinlock;        //point to owner_po_lock

	struct semaphore work_sem;
	bi_list work_list;

	u32 sw_cmdbuf_rdy_num;

	//number of cmdbufs without interrupt.
	u64 duration;
	volatile u32 state;
	u64 total_workload;

	//vcmd registers memory
	u32 *reg_mem_va;
	u32 reg_mem_sz;
	addr_t reg_mem_ba;
	size_t mmu_reg_mem_ba;

	//translation offset from bus addr (ba) to physical addr (pa).
	addr_t pa_trans_offset;
	wait_queue_head_t abort_queue_vcmd;

	wait_queue_head_t *abort_waitq;	//point to abort_queue_vcmd

	u32 abort_mode;		//0: stop when JMP/END; 1: stop curr-cmd (imediately);
	enum vcmd_sw_init_mode init_mode;

	u32 init_cmd_idx;	//for setting init cmd

	u32 aborted_cmdbuf_id;

	u8 mmu_enable;

	void *handler;		// point to vcmd_mgr

	// abnormal interrupt related
	u32 abn_irq_mask; 	// abnormal irq mask
	u32 intr_gate_mask;
	spinlock_t abn_irq_lock;
	u8 abn_irq_schedule;

	// vcmd timer[VCMD_TIMER_xxx]: use it by vcmd_timer_id
	struct vcmd_timer vcmd_timer[VCMD_TIMER_MAX];
#ifdef SUPPORT_WATCHDOG
	enum WD_STATE watchdog_state;
#endif
	// a flag to indicate arbiter reset irq received
	u32 arb_reset_irq;
	// a flag to indicate arbiter err irq received
	u32 arb_err_irq;

	/* waiting to be sent more slices to decode */
	u8 waiting_sent_slice;

	// Statistics of top and bottom half execution time
	ktime_t top_half_start, bottom_half_start;
	ktime_t top_half_abort_start, bottom_half_abort_start;

	// workqueue: work item for interrupt bottom half
	struct workqueue_struct *wq;
	struct work_struct _irq_work;
	u32 slice_send_outbuf_flag;
	struct kfifo send_slice_buffer_queue;

	// re-power vdec sub-system
	u32 reset_count;
	u32 wdt_trigger_count;
	struct device *device;

	u32 error_mask;

#ifdef SUPPORT_DBGFS
	void *dbgfs_info;
#endif
};

struct vcmd_module_mgr {
	u32 num;
	struct hantrovcmd_dev *dev[MAX_VCMD_NUM];
	struct semaphore sem; //for reserve cmdbuf
};

typedef struct {
	/* input configuration to vcmd manager */
	int subsys_num;
	struct vcmd_subsys_info core_array[MAX_SUBSYS_NUM];
	struct platform_device *platformdev;

	/* three linear buffers: vcmd buffer / status / registers */
	struct noncache_mem mem_vcmd;
	struct noncache_mem mem_status;
	struct noncache_mem mem_regs;

	//translation offset from bus addr (ba) to physical addr (pa).
	addr_t pa_trans_offset;

	struct vcmd_module_mgr module_mgr[MAX_VCMD_TYPE];

	/* cmdbuf/obj/node management*/
	struct cmdbuf_obj objs[SLOT_NUM_CMDBUF];

	struct bi_list_node nodes[SLOT_NUM_CMDBUF];		//nodes for dev work-list
	struct bi_list_node po_jobs[SLOT_NUM_CMDBUF];	//node for po job-list

	struct semaphore free_obj_sema;
	struct si_linked_list free_obj_list;

	struct hantrovcmd_dev *dev_ctx;

	u8 mmu_enable;
	u8 vcmd_irq_enabled;

	//PCIE_EN defined
	struct noncache_mem pcie_pool;	//pool for mem_vcmd, mem_status and mem_regs
	struct pci_dev *pcie_dev;

	struct proc_obj *init_po;

	struct semaphore isr_polling_sema;

	struct task_struct *kthread;
	u8 stop_kthread;
	wait_queue_head_t kthread_waitq;

#ifdef SUPPORT_DBGFS
	void *dbgfs_ctx;
#endif
} vcmd_mgr_t;

/* -----------------------------------------------------
*  debug func
* ------------------------------------------------------
*/

void dump_brief_regs(struct hantrovcmd_dev *dev, u32 dump);
static void vcmd_start(struct hantrovcmd_dev *dev);


/******************************************************************************
 * VCMD Command Definitions
 ******************************************************************************/
	#define OPCODE_MASK                         (0x1F << 27)
	#define OPCODE_WREG                         (0x01 << 27)
	#define OPCODE_END                          (0x02 << 27)
	#define OPCODE_NOP                          (0x03 << 27)
	#define OPCODE_RREG                         (0x16 << 27)
	#define OPCODE_INT                          (0x18 << 27)
	#define OPCODE_JMP                          (0x19 << 27)
	#define OPCODE_STALL                        (0x09 << 27)
	#define OPCODE_CLRINT                       (0x1a << 27)
	#define OPCODE_M2M                          (0x1b << 27)
	#define OPCODE_MSET                         (0x1c << 27)
	#define OPCODE_M2MP                         (0x1d << 27)


#define RREG_ADDR_INC			(0)
#define RREG_ADDR_FIX			(1)
#define RREG_MODE(A)			(((A) & 1) << 26)
#define RREG_G_MODE(A)			(((A) >> 26) & 1)
#define RREG_LEN(A)				(((A) & 0x3FF) << 16)
#define RREG_G_LEN(A)			((((((A) >> 16) & 0x3FF) - 1) & 0x3FF) + 1)
#define RREG_START_ADDR(A)		((A) & 0xFFFF)
struct cmd_rreg_t {
	u32 opcode;					//refer to register spec.
	u32 lsb;					//reg value buf addr [31..0]
	u32 msb;					//reg value buf addr [63..32]
	u32 padding;
};

#define JMP_RDY(A)			(((A) & 1) << 26)
#define JMP_G_RDY(A)		(((A) >> 26) & 1)
#define JMP_IE(A)			(((A) & 1) << 25)
#define JMP_G_IE(A)			(((A) >> 25) & 1)
#define JMP_NEXT_LEN(A)		((A) & 0xFFFF)
struct cmd_jmp_t {
	u32 opcode;					//refer to register spec.
	u32 lsb;					//next cmd-buf addr [31..0]
	u32 msb;					//next cmd-buf addr [63..32]
	u32 id;						//next cmd-buf id
};

#define WREG_ADDR_INC			(0)
#define WREG_ADDR_FIX			(1)
#define WREG_MODE(A)			(((A) & 1) << 26)
#define WREG_G_MODE(A)			(((A) >> 26) & 1)
#define WREG_LEN(A)				(((A) & 0x3FF) << 16)
#define WREG_G_LEN(A)			((((((A) >> 16) & 0x3FF) - 1) & 0x3FF) + 1)
#define WREG_START_ADDR(A)		((A) & 0xFFFF)
struct cmd_wreg_t {
	u32 opcode;					//refer to register spec.
	u32 data[0];
};

struct cmd_end_t {
	u32 opcode;					//refer to register spec.
	u32 padding;
};

#define CMD_SET_ADDR(cmd, ba) do { \
	(cmd)->lsb = (u32)(ba); \
	(cmd)->msb = (sizeof(addr_t) == 4) ? 0 : ((u32)((u64)(ba) >> 32)); \
  } while (0)



#ifdef __cplusplus
}
#endif
#endif
