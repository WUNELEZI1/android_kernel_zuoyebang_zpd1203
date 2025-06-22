// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */



#ifndef __IPC_CMDS_LPCTRL_AP_H__
#define __IPC_CMDS_LPCTRL_AP_H__

enum ipc_cmds_lpctrl_ap_e
{

    CMD_LP_RESERVED = 0,


    CMD_LP_SH_RST = 0x1,
    CMD_LP_SH_LOG,

    CMD_LP_DFX_AP_VOTE,


    CMD_LP_MDR_SYS_RST,
    CMD_LP_MDR_DUMP,
    CMD_LP_MDR_RST,
    CMD_LP_MDR_INIT_F,
    CMD_LP_MDR_DFX_ADDR,


    CMD_LP_VOTE_GPU_ON = 0x1,
    CMD_LP_VOTE_GPU_OFF,


    CMD_LP_PM_DATA,


    CMD_LP_DMD_PLL_ERR = 0x0,
    CMD_LP_DMD_PMU_ERR,

    CMD_LP_DMD_MAX,



    CMD_LP_MAX_NUM = 0xFF
};

#endif
