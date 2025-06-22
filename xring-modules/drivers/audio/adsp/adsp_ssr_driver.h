/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _ADSP_SSR_DRIVER_H_
#define _ADSP_SSR_DRIVER_H_

#include <asm/bitops.h>
#include <asm/io.h>
#include <linux/types.h>

#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/audio_sys.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include <dt-bindings/xring/platform-specific/DW_apb_wdt_header.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <dt-bindings/xring/platform-specific/memory_layout.h>

#define ADSP_MARK_REG		(ACPU_LPIS_ACTRL + LPIS_ACTRL_AUDSYS_RESERVED2)
#define AUDIO_SYS_M_INTR_FROM_ACPU_REG		(ACPU_ASP_CFG + AUDIO_SYS_M_INTR_FROM_ACPU)
#define AUDIO_SYS_M_INTR_FROM_ACPU_MASK		(ACPU_ASP_CFG + \
	AUDIO_SYS_INTR_DEBUG_FROM_ACPU_IRQ_INT_MASK)
#define AUDIO_SYS_WDT_CRR_REG				(ACPU_ASP_WDG + DW_APB_WDT_CRR)
#define AUDIO_DSP_UNCACHELOG_ADDR			(XR_ADSP_RUN_MEM_BASE + 0x9E04D0)
#define AUDIO_DSP_UNCACHELOG_SIZE			(0x8018)
#define AUDIO_RESERVED_MEM					(XR_ADSP_RUN_MEM_BASE + 0x400000)
#define AUDIO_RESERVED_SIZE					(0x900000)
#define ADSP_TCM_MEM							0xE1400000
#define ADSP_TCM_SIZE							0x84000

#define IOREMAP_REGISTER_SIZE				0x4

/* clear wdt timeout intr */
#define WDOG_COUNTER_RESTART_KICK_VALUE		0x76

/* close AUDIO_SYS_INTR_DEBUG_FROM_ACPU_IRQ_INT_MASK */
#define AUDIO_SYS_INTR_DEBUG_FROM_ACPU_MASK_OFF		0x0

/* open INTR_FROM_ACPU W1P */
#define AUDIO_SYS_M_INTR_FROM_ACPU_VALUE			0x1

#define ADSP_SSR_UEVENT_BUF_MAXLEN  (128)
#define ADSP_UEVENT_RESET_OPEN		"reset_open"
#define ADSP_UEVENT_RESET_CLOSE		"reset_close"

#define MODID_ADSP_ILLEGAL_INSTRUCTION      0x84000001
#define MODID_ADSP_FETCH_ERR                0x84000002
#define MODID_ADSP_LOAD_ERR                 0x84000003
#define MODID_ADSP_ILLEGAL_NEXT_PC_VALUE    0x84000004
#define MODID_ADSP_DIVIDE_BY_ZERO           0x84000005
#define MODID_ADSP_UNALIGNED_LOAD           0x84000006
#define MODID_ADSP_MEM_NOT_FETCH            0x84000007
#define MODID_ADSP_MEM_NOT_LOAD             0x84000008
#define MODID_ADSP_MEM_NOT_STORE            0x84000009
#define MODID_ADSP_WRITE_ERR                0x8400000a
#define MODID_ADSP_WDT                      0x8400000b
#define MODID_ADSP_IPC_TIMEOUT              0x8400000c
#define MODID_ADSP_VOTEMSG_ERROR            0x8400000d
#define MODID_ADSP_OTHER                    0x8400000e

#define ADSP_MDR_EXP_TYPE_MAX               0xe

static struct mdr_exception_info_s g_adsp_exp[ADSP_MDR_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_ADSP_ILLEGAL_INSTRUCTION,
		.e_modid_end        = (u32)MODID_ADSP_ILLEGAL_INSTRUCTION,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_ILLEGAL_INSTRUCTION,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "ILLEGAL INSTRUCTION",
	},
	[1] = {
		.e_modid            = (u32)MODID_ADSP_FETCH_ERR,
		.e_modid_end        = (u32)MODID_ADSP_FETCH_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_FETCH_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "FETCH_ERR",
	},
	[2] = {
		.e_modid            = (u32)MODID_ADSP_LOAD_ERR,
		.e_modid_end        = (u32)MODID_ADSP_LOAD_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_LOAD_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "LOAD ERROR",
	},
	[3] = {
		.e_modid            = (u32)MODID_ADSP_ILLEGAL_NEXT_PC_VALUE,
		.e_modid_end        = (u32)MODID_ADSP_ILLEGAL_NEXT_PC_VALUE,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_ILLEGAL_NEXT_PC_VALUE,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "ILLEGAL NEXT PC VALUE",
	},
	[4] = {
		.e_modid            = (u32)MODID_ADSP_DIVIDE_BY_ZERO,
		.e_modid_end        = (u32)MODID_ADSP_DIVIDE_BY_ZERO,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_DIVIDE_BY_ZERO,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "DIVIDE BY ZERO",
	},
	[5] = {
		.e_modid            = (u32)MODID_ADSP_UNALIGNED_LOAD,
		.e_modid_end        = (u32)MODID_ADSP_UNALIGNED_LOAD,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_UNALIGNED_LOAD,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "UNALIGNED LOAD",
	},
	[6] = {
		.e_modid            = (u32)MODID_ADSP_MEM_NOT_FETCH,
		.e_modid_end        = (u32)MODID_ADSP_MEM_NOT_FETCH,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_MEM_NOT_FETCH,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "MEM NOT FETCH",
	},
	[7] = {
		.e_modid            = (u32)MODID_ADSP_MEM_NOT_LOAD,
		.e_modid_end        = (u32)MODID_ADSP_MEM_NOT_LOAD,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_MEM_NOT_LOAD,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "MEM NOT LOAD",
	},
	[8] = {
		.e_modid            = (u32)MODID_ADSP_MEM_NOT_STORE,
		.e_modid_end        = (u32)MODID_ADSP_MEM_NOT_STORE,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_MEM_NOT_STORE,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "MEM NOT STORE",
	},
	[9] = {
		.e_modid            = (u32)MODID_ADSP_WRITE_ERR,
		.e_modid_end        = (u32)MODID_ADSP_WRITE_ERR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_WRITE_ERR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "WRITE ERROR",
	},
	[10] = {
		.e_modid            = (u32)MODID_ADSP_WDT,
		.e_modid_end        = (u32)MODID_ADSP_WDT,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_WDT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "ADSP WDT",
	},
	[11] = {
		.e_modid            = (u32)MODID_ADSP_IPC_TIMEOUT,
		.e_modid_end        = (u32)MODID_ADSP_IPC_TIMEOUT,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_IPC_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "ADSP IPC TIMEOUT",
	},
	[12] = {
		.e_modid            = (u32)MODID_ADSP_VOTEMSG_ERROR,
		.e_modid_end        = (u32)MODID_ADSP_VOTEMSG_ERROR,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_VOTEMSG_ERROR,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "VOTEMSG ERROR",
	},
	[13] = {
		.e_modid            = (u32)MODID_ADSP_OTHER,
		.e_modid_end        = (u32)MODID_ADSP_OTHER,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_AUDIO,
		.e_reset_core_mask  = MDR_AUDIO,
		.e_from_core        = MDR_AUDIO,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = AUDIO_S_EXCEPTION,
		.e_exce_subtype		= ASP_S_OTHER,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "ADSP",
		.e_desc             = "ADSP OTHER",
	},
};

struct adsp_mdr_item {
	int val;
	uint32_t mdr_exception_type;
};

struct recovery_ops {
	void (*pre_recovery)(void *priv);
	void (*post_recovery)(void *priv);
};

struct adsp_ssr_action {
	struct list_head action_list;
	struct recovery_ops *ops;
	void *priv;
};

struct adsp_ssr_client {
	atomic_t ac_num;
	struct list_head ac_list;
	struct mutex list_lock;
};

struct xr_adsp_ssr_dev {
	struct platform_device *pdev;
	/* audio sys wdt timeout intr restart reg */
	void __iomem *audio_sys_wdt_restart_reg;

	/* ADSP_A mark reg */
	void __iomem *adsp_mark_reg;

	/* audio sys debug intr from acpu trigger reg */
	void __iomem *adsp_debug_intr_trigger_reg;

	/* audio sys debug intr mask */
	void __iomem *adsp_debug_intr_mask;

	/* audsys power status */
	void __iomem *audsys_power_state;

	/* adsp tineout intr */
	int irqno_wdt;

	/* adsp reset open ctrl */
	bool reset_switch;

	struct mdr_module_ops mdr_adsp_ops;
	struct mdr_register_module_result mdr_retinfo;
	struct adsp_ssr_client ssr_client;

	bool adsp_reset_process;
};

static inline unsigned int adsp_reg_read32(const void __iomem *reg)
{
	return __raw_readl(reg);
}

static inline void adsp_reg_write32(void __iomem *reg, int val)
{
	__raw_writel(val, reg);
}

int adsp_reset(int subtype);
int get_adsp_reset_switch(void);
void set_adsp_reset_switch(bool enable);
bool get_adsp_reset_process(void);
int adsp_send_debug_intr_reset(void);
int adsp_wdt_exception_injection(void);
int adsp_panic_exception_injection(void);
struct adsp_ssr_action *fk_adsp_ssr_register(struct recovery_ops *ops, void *priv);
int fk_adsp_ssr_unregister(struct adsp_ssr_action *action);

#endif //_ADSP_SSR_DRIVER_H_

