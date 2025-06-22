// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SHUB_DFX_PRIV_H__
#define __SHUB_DFX_PRIV_H__

#include <linux/kernel.h>
#include <dt-bindings/xring/platform-specific/ipc_protocol.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>

enum sh_dfx_save_type {
	SH_SAVE_TYPE_LOG = 0,
	SH_SAVE_TYPE_ALL = 1
};

int sh_mdr_register(void);
void sh_mdr_unregister(void);
void sh_trigger_loop_dump(char *path);
void sh_notify_to_reset(void);
void sh_notify_hal_savelogs(enum sh_dfx_save_type type, char *path);
int sh_log_dev_init(void);
void sh_log_dev_exit(void);
void sh_mdr_error_notify(modid_sensorhub modid);
int sh_log_notify_control(enum ipc_cmds_sh_ap_e dfx_cmd);
int recovery_msg_handler(unsigned long action);
void sh_log_ddr_rd_ptr_init(void);

int sh_dmd_register(void);
int sh_dmd_unregister(void);
void sh_dmd_report(struct ipc_pkt *pkt);

#endif
