// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __SENSORHUB_DFX_H__
#define __SENSORHUB_DFX_H__

#include "ipc_resource.h"

#define SH_DFX_AP_LP_VC_ID     (IPC_VC_PERI_NS_AP_LP_COMMON_1)
#define SH_DFX_AP_SH_VC_ID     (IPC_VC_AO_NS_SH_AP_DEFAULT)
#define SH_DFX_FIRQ_CHANNEL    (0)

#define SH_FLAG_LOGS_SHIFT      (30)
#define SH_FLAG_LOGS_MASK       (0xC0000000)
#define SH_LOGS_DONE_SH_VALUE   (0x1)
#define SH_LOGS_DONE_LP_VALUE   (0x2)
#define SH_LOGS_DONE_SH         (SH_LOGS_DONE_SH_VALUE << SH_FLAG_LOGS_SHIFT)
#define SH_LOGS_DONE_LP         (SH_LOGS_DONE_LP_VALUE << SH_FLAG_LOGS_SHIFT)

#define SH_FLAG_RESET_DOT_SHIFT (9)
#define SH_FLAG_IRQ_NUM_MASK    (0x1FF)

enum {
	SH_RESET_BOARD_LOGS    = 1    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_BOARD_IRQ     = 2    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_BOARD_WD      = 3    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_ENTER     = 4    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_PAUSE_WD  = 5    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_REG       = 6    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_BACKTRACE = 7    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_LOGS      = 8    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_NMI_DUMP      = 9    << SH_FLAG_RESET_DOT_SHIFT,
	SH_RESET_STEPS_MAX     = 0xF  << SH_FLAG_RESET_DOT_SHIFT
};

#define FLAG_SH_ALL_READY           (0x1)
#define FLAG_SH_RESET_POWER_DOWN    (0x10)

#define SH_LOG_HEAD_WR_OFFSET     (0x100000 - 0x10)

#define SH_DFX_NETLINK_UNIT         (25)
#define SH_DFX_MSG_MAX_LEN          (60)
#define SH_DFX_MSG_HEAD_KERNEL_LOG  "SH_K:LOG"
#define SH_DFX_MSG_HEAD_KERNEL_ALL  "SH_K:ALL"

#define SH_DFX_NODE_DEV_NAME        "shub_log"
#define SH_DFX_MAP_MAX_SIZE         (0x480000)

#define SH_RECOVERY_MESSAGE_PART_SHIFT    (24)
#define SH_BOOT_MESSAGE_PART_MASK         (0xFFFF)

#define SH_XRISP_AP_CAM_POWER_STATUS_BASE      (0xE15087c4)
#define SH_XRISP_AP_CAM_POWER_STATUS_SIZE      (4)
#define SH_XRISP_AP_CAM_ACTRL_BMRW_SHIFT       (16)
#define SH_XRISP_SH_CAM_POWER_STATUS_MASK      BIT(1)
#define SH_XRISP_SH_CAM_CRASH_STATUS_MASK      BIT(2)
#define SH_XRISP_SH_CAM_POWER_STATUS_IDLE      (0)


enum SHUB_DFX_ALARM_FAULT_ID {
	SHUB_ID_DFX_TEST = 921003000,
};

struct shub_dfx_alarm_head {
	uint32_t fault_id;
	uint32_t time_low;
	uint32_t time_high;
};
#endif
