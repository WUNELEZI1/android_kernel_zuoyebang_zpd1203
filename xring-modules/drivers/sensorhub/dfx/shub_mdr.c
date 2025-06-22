// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)            "[shub_mdr]:" fmt

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_subtype_exception.h>
#include "shub_dfx_priv.h"

#define MAX_CHARS_PRINT  (40)

static struct mutex g_sh_mdr_lock;
static struct mdr_exception_info_s g_sh_exception[] = {
	[0] = {
			.e_modid            = (uint32_t)MODID_SHUB_WDT,
			.e_modid_end        = (uint32_t)MODID_SHUB_WDT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_WDT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub watchdog timeout",
	},
	[1] = {
			.e_modid            = (uint32_t)MODID_SHUB_HARDFAULT,
			.e_modid_end        = (uint32_t)MODID_SHUB_HARDFAULT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_HARDFAULT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub core hardfault",
	},
	[2] = {
			.e_modid            = (uint32_t)MODID_SHUB_MEMFAULT,
			.e_modid_end        = (uint32_t)MODID_SHUB_MEMFAULT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_MEMFAULT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub core memfault",
	},
	[3] = {
			.e_modid            = (uint32_t)MODID_SHUB_BUSFAULT,
			.e_modid_end        = (uint32_t)MODID_SHUB_BUSFAULT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_BUSFAULT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub core busfault",
	},
	[4] = {
			.e_modid            = (uint32_t)MODID_SHUB_USGFAULT,
			.e_modid_end        = (uint32_t)MODID_SHUB_USGFAULT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_USGFAULT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub core usagefault",
	},
	[5] = {
			.e_modid            = (uint32_t)MODID_SHUB_LOCKUP,
			.e_modid_end        = (uint32_t)MODID_SHUB_LOCKUP,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_LOCKUP,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub core lockup",
	},
	[6] = {
			.e_modid            = (uint32_t)MODID_SHUB_USER_TRIGGER,
			.e_modid_end        = (uint32_t)MODID_SHUB_USER_TRIGGER,
			.e_process_priority = MDR_WARN,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_USER_TRIGGER,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub user trigger reset",
	},
	[7] = {
			.e_modid            = (uint32_t)MODID_SHUB_SPI_DMAC_ERR,
			.e_modid_end        = (uint32_t)MODID_SHUB_SPI_DMAC_ERR,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_SPI_DMAC_ERR,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub spi_dmac noc error",
	},
	[8] = {
			.e_modid            = (uint32_t)MODID_SHUB_IPC_TIMEOUT,
			.e_modid_end        = (uint32_t)MODID_SHUB_IPC_TIMEOUT,
			.e_process_priority = MDR_ERR,
			.e_reboot_priority  = MDR_REBOOT_NO,
			.e_notify_core_mask = MDR_SHUB | MDR_AP,
			.e_reset_core_mask  = MDR_SHUB,
			.e_from_core        = MDR_SHUB,
			.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
			.e_exce_type        = SHUB_S_EXCEPTION,
			.e_exce_subtype     = SHUB_IPC_TIMEOUT,
			.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
			.e_from_module      = "sensorhub",
			.e_desc             = "sensorhub and ap ipc timeout error",
	},
};

static struct mdr_exception_info_s g_sh_other_master_exception = {
	.e_modid            = (uint32_t)MODID_SHUB_DMA_ERR,
	.e_modid_end        = (uint32_t)MODID_SHUB_DMA_ERR,
	.e_process_priority = MDR_ERR,
	.e_reboot_priority  = MDR_REBOOT_NO,
	.e_notify_core_mask = MDR_SHUB | MDR_AP,
	.e_reset_core_mask  = MDR_SHUB,
	.e_from_core        = MDR_SHUB,
	.e_reentrant        = (uint32_t)MDR_REENTRANT_DISALLOW,
	.e_exce_type        = SHUB_S_EXCEPTION,
	.e_exce_subtype     = SHUB_DMA_ERR,
	.e_upload_flag      = (uint32_t)MDR_UPLOAD_YES,
	.e_from_module      = "sensorhub",
	.e_desc             = "sensorhub dma noc error",
};

void sh_mdr_dump(uint32_t modid, uint32_t etype, uint64_t coreid, char *log_path,
			pfn_cb_dump_done pfn_cb, void *data)
{
	pr_info("dump: modid=0x%x, etype=0x%x, coreid=0x%llx.\n", modid, etype, coreid);
	pr_debug("dump: log_path=%.*s.\n", MAX_CHARS_PRINT, log_path);
	if (!mutex_trylock(&g_sh_mdr_lock))
		return;

	sh_trigger_loop_dump(log_path);
	mutex_unlock(&g_sh_mdr_lock);
}

void sh_mdr_reset(uint32_t modid, uint32_t etype, uint64_t coreid, void *data)
{
	pr_info("reset: modid=0x%x, etype=0x%x, coreid=0x%llx.\n", modid, etype, coreid);
	if (!mutex_trylock(&g_sh_mdr_lock))
		return;

	sh_notify_to_reset();
	mutex_unlock(&g_sh_mdr_lock);
}

static struct mdr_module_ops g_sh_mdr_ops = {
	.ops_dump   = sh_mdr_dump,
	.ops_reset  = sh_mdr_reset,
};

static struct mdr_register_module_result g_sh_mdr_ret;

static void sh_sub_excep_unregister(uint32_t index)
{
	int ret;
	uint32_t i;

	if (index == 0)
		return;

	for (i = 0; i < index; i++) {
		ret = mdr_unregister_exception(g_sh_exception[i].e_modid);
		if (ret != 0)
			pr_warn("exception[%u] unregister fail.\n", i);
	}
}

int sh_mdr_register(void)
{
	int ret;
	uint32_t i;
	uint32_t size = ARRAY_SIZE(g_sh_exception);

	pr_info("g_sh_exception array size = %u.\n", size);
	for (i = 0; i < size; i++) {
		ret = mdr_register_exception(&g_sh_exception[i]);
		if (ret == 0) {
			pr_err("exception[%u] register fail.\n", i);
			sh_sub_excep_unregister(i);
			return -EINVAL;
		}
	}

	ret = noc_register_exception(&g_sh_other_master_exception);
	if (ret != 0) {
		pr_err("other master exception register fail, ret=%d.\n", ret);
		sh_sub_excep_unregister(size);
		return -EINVAL;
	}

	ret = mdr_register_module_ops(MDR_SHUB, &g_sh_mdr_ops, &g_sh_mdr_ret);
	if (ret < 0) {
		pr_err("ops register fail, ret=%d.\n", ret);
		sh_sub_excep_unregister(size);
		noc_unregister_exception(&g_sh_other_master_exception);
		return -EINVAL;
	}
	mutex_init(&g_sh_mdr_lock);

	return 0;
}

void sh_mdr_unregister(void)
{
	int ret;

	ret = mdr_unregister_module_ops(MDR_SHUB);
	pr_info("ops unregister ret= %d.\n", ret);
	sh_sub_excep_unregister(ARRAY_SIZE(g_sh_exception));
	noc_unregister_exception(&g_sh_other_master_exception);
	mutex_destroy(&g_sh_mdr_lock);
}

void sh_mdr_error_notify(modid_sensorhub modid)
{
	pr_info("error notify: errcode = 0x%x.\n", modid);
	mdr_system_error(modid, 0, 0);
}
