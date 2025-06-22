// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRSE_PLATFORM_H__
#define __XRSE_PLATFORM_H__

#include "lpis_actrl.h"

#define XRSE_USED_STATUS_REG                   LPIS_ACTRL_SC_RSV_S_0
#define XRSE_BACKUP_FLAG_REG                   LPIS_ACTRL_SC_RSV_S_2
#define XRSE_VOTE_S_REG                        0x900
#define XRSE_VOTE_RESULT_S_REG                 0x914
#define XRSE_VOTE_NS_REG                       0x700
#define XRSE_VOTE_RESULT_NS_REG                0x704
#define XRSE_READY_STATUS_SHIFT                0U
#define XRSE_READY_STATUS_MASK_SHIFT           (XRSE_READY_STATUS_SHIFT + 16)
#define XRSE_USED_STATUS_LPCTRL_SHIFT          1U
#define XRSE_USED_STATUS_LPCTRL_MASK_SHIFT     (XRSE_USED_STATUS_LPCTRL_SHIFT + 16)
#define XRSE_USED_STATUS_ATF_SHIFT             2U
#define XRSE_USED_STATUS_ATF_MASK_SHIFT        (XRSE_USED_STATUS_ATF_SHIFT + 16)
#define XRSE_USED_STATUS_MITEE_SHIFT           3U
#define XRSE_USED_STATUS_MITEE_MASK_SHIFT      (XRSE_USED_STATUS_MITEE_SHIFT + 16)
#define XRSE_USED_STATUS_SENSORHUB_SHIFT       4U
#define XRSE_USED_STATUS_SENSORHUB_MASK_SHIFT  (XRSE_USED_STATUS_SENSORHUB_SHIFT + 16)
#define XRSE_USED_STATUS_XSPS_SHIFT            5U
#define XRSE_USED_STATUS_XSPS_MASK_SHIFT       (XRSE_USED_STATUS_XSPS_SHIFT + 16)
#define XRSE_USED_STATUS_MASK                  0xFFU
#define XRSE_VOTE_MNG_MASK                     0xFFU
#define XRSE_WAIT_READY_TIMEOUT_US             1000000U
#define XRSE_WAIT_IDLE_TIMEOUT_US              1000U
#define XRSE_WAIT_VOTE_PD_TIMEOUT_US           100000U
#define XRSE_WAIT_EXCEPT_PD_TIMEOUT_US         2500000U
#define XRSE_WAIT_WFI_TIMEOUT_US               10000U
#define XRSE_VOTE_CNT_MAX                      0xFFFFFFFFU
#define XRSE_VOTE_CNT_MIN                      0U
#define XRSE_VOTE_POWERON_CNT                  1U
#define XRSE_VOTE_POWERDOWN_CNT                0U
#define XRSE_BACKUP_FLAG                       0x00003C3CU
#define XRSE_BACKUP_FLAG_RW_MASK               0xFFFF0000U
#define XRSE_BOOT_TYPE_SHIFT                   0U
#define XRSE_BOOT_TYPE_COLD                    0U
#define XRSE_BOOT_TYPE_WARM                    1U
#define XRSE_WARM_BOOT_TYPE_SHIFT              1U
#define XRSE_WARM_BOOT_TYPE_RAM                0U
#define XRSE_WARM_BOOT_TYPE_DDR                1U


#define TZPC_SECURE                     0x3CU
#define TZPC_NONSECURE                  0xC3U

#define TZPC_LOCK                       0x5AU
#define TZPC_UNLOCK                     0xA5U

#define MREGION_LOCK                    0x5AU
#define MREGION_UNLOCK                  0xA5U

#define PMPU_SET                        0x5AU
#define PMPU_UNSET                      0xA5U

#define MREGION_SET                     0x5AU
#define MREGION_UNSET                   0xA5U
#define MREGION_REGION_INIT             0x3CU
#define MREGION_REGION_DEINIT           0xC3U
#define MREGION_REGION_INSERT           0x1BU

#define MREGION_GPU_RESUME_DONE         0xA5A5A5A5U

#define TZPC_SET                        0x5AU
#define TZPC_BOOT_SET                   0xA5U

#define DDR_SET                         0x5AU


#define EFUSE_WRITE_NVCOUNTER           0x3CU
#define EFUSE_WRITE_CFG_SWITCH          0xC3U
#define EFUSE_WRITE_MAGIC_NUM           0xDEADAB1EU


#define DICE_CMD_GET_CDI_ATTEST         0x5AU
#define DICE_CMD_GET_CDI_SEAL           0xA5U
#define DICE_CMD_GET_BCC_CHAIN          0x3CU
#define DICE_CMD_GET_PVMFW_HANDOVER     0xC3U

#define IPC_RESPONSE_DICE_DATA_ERR      0xFBADADDFU



#define SERVICE_REQ_SUCCESS             0x5A5A5A5AU
#define SERVICE_REQ_FAILED              0xA5A5A5A5U


#define IPC_SHM_SIZE_64K                     (64 * 1024)
#define IPC_SHM_SIZE_16K                     (16 * 1024)
#define IPC_SHM_SIZE_TOTAL                   (2 * 176 * 1024)
#define XRSE_IPC_SHM_OFFSET                  (1664 * 1024)


#define XRSE_SHM_IPC_TEE_SEND_OFFSET         0
#define XRSE_SHM_IPC_TEE_SEND_SIZE           IPC_SHM_SIZE_64K
#define XRSE_SHM_IPC_TEE_RECV_OFFSET         (XRSE_SHM_IPC_TEE_SEND_OFFSET + XRSE_SHM_IPC_TEE_SEND_SIZE)
#define XRSE_SHM_IPC_TEE_RECV_SIZE           XRSE_SHM_IPC_TEE_SEND_SIZE


#define XRSE_SHM_IPC_ATF_SEND_OFFSET         XRSE_SHM_IPC_TEE_RECV_OFFSET + XRSE_SHM_IPC_TEE_RECV_SIZE
#define XRSE_SHM_IPC_ATF_SEND_SIZE           IPC_SHM_SIZE_64K
#define XRSE_SHM_IPC_ATF_RECV_OFFSET         (XRSE_SHM_IPC_ATF_SEND_OFFSET + XRSE_SHM_IPC_ATF_SEND_SIZE)
#define XRSE_SHM_IPC_ATF_RECV_SIZE           XRSE_SHM_IPC_ATF_SEND_SIZE


#define XRSE_SHM_IPC_LP_SEND_OFFSET          (XRSE_SHM_IPC_ATF_RECV_OFFSET + XRSE_SHM_IPC_ATF_RECV_SIZE)
#define XRSE_SHM_IPC_LP_SEND_SIZE            IPC_SHM_SIZE_16K
#define XRSE_SHM_IPC_LP_RECV_OFFSET          (XRSE_SHM_IPC_LP_SEND_OFFSET + XRSE_SHM_IPC_LP_SEND_SIZE)
#define XRSE_SHM_IPC_LP_RECV_SIZE            XRSE_SHM_IPC_LP_SEND_SIZE


#define XRSE_SHM_IPC_SH_SEND_OFFSET          (XRSE_SHM_IPC_LP_RECV_OFFSET + XRSE_SHM_IPC_LP_RECV_SIZE)
#define XRSE_SHM_IPC_SH_SEND_SIZE            IPC_SHM_SIZE_16K
#define XRSE_SHM_IPC_SH_RECV_OFFSET          (XRSE_SHM_IPC_SH_SEND_OFFSET + XRSE_SHM_IPC_SH_SEND_SIZE)
#define XRSE_SHM_IPC_SH_RECV_SIZE            XRSE_SHM_IPC_SH_SEND_SIZE


#define XRSE_SHM_IPC_AP_SEND_OFFSET          (XRSE_SHM_IPC_SH_RECV_OFFSET + XRSE_SHM_IPC_SH_RECV_SIZE)
#define XRSE_SHM_IPC_AP_SEND_SIZE            IPC_SHM_SIZE_16K
#define XRSE_SHM_IPC_AP_RECV_OFFSET          (XRSE_SHM_IPC_AP_SEND_OFFSET + XRSE_SHM_IPC_AP_SEND_SIZE)
#define XRSE_SHM_IPC_AP_RECV_SIZE            XRSE_SHM_IPC_AP_SEND_SIZE



enum xrse_status {
	XRSE_ROM_UNREADY                = 0xC33C3CC3U,
	XRSE_ROM_READY                  = 0x3CC33CC3U,
	XRSE_ROM_EXCEPTION              = 0x3C3CC3C3U,
	XRSE_FIRMWARE_UNREADY           = 0xA55AA55AU,
	XRSE_FIRMWARE_READY             = 0x5AA55AA5U,
};

enum xrse_ipc_service {
	IPC_EFUSE              = 0,
	IPC_CRYPTO,
	IPC_TRNG,
	IPC_SEC_CFG,
	IPC_SEC_DEBUG,
	IPC_SEC_BOOT           = 5,
	IPC_DICE,
	IPC_MREGION,
	IPC_TZPC,
	IPC_PMPU,
	IPC_DDR                = 10,
	IPC_DTB,
	IPC_XRSE_AGENT,
	IPC_DFX,
	IPC_RIP,
	IPC_EXAMPLE,
	IPC_BM,
	IPC_ATE,
	IPC_SLT,
	IPC_SERVICE_MAX,
};

enum xrse_agent_ipc_index {
	XRSE_AGENT_CHIPTEST = 0,
	XRSE_AGENT_GETLOG = 1,
};

enum tzpc_case_index {
	TZPC_USB_CFG_CASE            = 0,
	TZPC_UFS_CFG_CASE,
	TZPC_EMMC_CFG_CASE,
	TZPC_CSI_CFG_CASE,
	TZPC_ISP_CFG_CASE            = 4,
	TZPC_SPI_DMA_CASE,
	TZPC_CSI5_MASTER_CASE,
	TZPC_SHUB_SPI10_CASE,
	TZPC_MAX_CFG_CASE,
};

enum mregion_case_index{
	MREGION_DDR_HOLE0_CASE      =0,
	MREGION_DDR_HOLE1_CASE,
	MREGION_DDR_HOLE2_CASE,
	MREGION_DDR_HOLE3_CASE,
	MREGION_XRSE_DYM_CASE,
	MREGION_NPU_DYM_CASE        =5,
	MREGION_ISP_FACEID_CASE,
	MREGION_TUI_DISPLAY_CASE,
	MREGION_TUI_FONT_CASE,
	MREGION_DFX_CASE,
	MREGION_TEE_DATA1_CASE     =10,
	MREGION_TEE_CODE_CASE,
	MREGION_TEE_DATA2_CASE,
	MREGION_BL2_IMG_CASE,
	MREGION_PVMFW_IMG_CASE,
	MREGION_KERNEL_IMG_CASE,
	MREGION_UFS_DFX_CASE,
	MREGION_MAX_CASE,
};

struct xrse_tzpc_data {
    uint16_t index;
    uint8_t attr;
    uint8_t lock;
};

#pragma pack(1)
struct xrse_dice_nsec_img_info {
    uint64_t addr;
    uint32_t size;
    uint64_t version;
};
#pragma pack()

enum xrse_efuse_field {
	EFUSE_PVT                  = 0,
	EFUSE_HPM_CPU,
	EFUSE_HPM_GPU,
	EFUSE_HPM_NPU,
	EFUSE_HPM_MEDIA,
	EFUSE_HPM_DDR              = 5,
	EFUSE_HPM_PERI,
	EFUSE_DJTAG_SHM_DIS,
	EFUSE_LPCORE_CFG,
	EFUSE_DEBUG_AUTH_MODE,
	EFUSE_UEFI_GET_DIE_ID      = 10,
	EFUSE_UEFI_GET_CPU_STATUS,
	EFUSE_UFS_CALIBRATION_N,
	EFUSE_UFS_CALIBRATION_E,
	EFUSE_CHIP_LCS,
	EFUSE_LOT_INFO             = 15,
	EFUSE_WAFER_INFO,
	EFUSE_DIEX_INFO,
	EFUSE_DIEY_INFO,
	EFUSE_ATE_FREQ_INFO,
	EFUSE_SLT_FREQ_INFO        = 20,
	EFUSE_PARTICAL_GOOD,
	EFUSE_SECDEBUG_ENABLE,
	EFUSE_OEM_SECBOOT_ENABLE,
	EFUSE_IMG_NVC,
	EFUSE_SYS_SWITCH           = 25,
	EFUSE_NEW_SLT_FREQ_INFO,
	EFUSE_NEW_ATE_FREQ_INFO,
	EFUSE_CSI_VPH_SELECT,
	EFUSE_OVERCLOCKING_INFO,
};

enum xrse_vote_channel {
	XRSE_VOTE_CH_XRSE = 0,
	XRSE_VOTE_CH_LPCTRL,
	XRSE_VOTE_CH_ATF,
	XRSE_VOTE_CH_MITEE,
	XRSE_VOTE_CH_SENSORHUB,
	XRSE_VOTE_CH_XSPS,
};


#define RIP_MEASURE_VALUE_LEN              32
#define RIP_MEASURE_SUCCESS                0x5a
#define RIP_MEASURE_FAIL                   0xa5
#define RIP_MEASURE_NO_INIT                0x3c
#define RIP_MEASURE_UNKNOWN                0xc3

#define RIP_MAGIC_NUMBER                   0x12345678

enum rip_cmd {

	RIP_BASELINE_MEASURE = 0,
	RIP_TIMING_MEASURE,
	RIP_REALTIME_MEASURE,
	RIP_MEASURE_CMD_MAX,

	RIP_STATUS_GET = 0x10,
	RIP_TIMING_MEASURE_NOTIFY,
	RIP_CMD_MAX,
};

enum rip_trust_level {
	TRUST_WORTHY = 0,
	SLIGHT_RISK,
	MEDIUM_RISK,
	CRITICAL_RISK,
	DEV_UNREADY,
};


enum rip_module_id {

	RIP_BL31_RODATA,
	RIP_BL31_TEXT,
	RIP_XSPM_RODATA,
	RIP_XSPM_TEXT,
	CRITICAL_RISK_BOUNDARY = RIP_XSPM_TEXT,

	RIP_MITEE,
	RIP_XSPS_RODATA,
	RIP_XSPS_TEXT,
	MEDIUM_RISK_BOUNDARY = RIP_XSPS_TEXT,

	RIP_XHEE_RODATA,
	RIP_XHEE_TEXT,
	RIP_KERNEL,
	SLIGHTT_RISK_BOUNDARY = RIP_KERNEL,
	RIP_MAX,
};

struct rip_measure_strategy_str {
	uint32_t alarm_time;
};

struct rip_service_data_str {
	uint32_t module_id;
	uint64_t measure_addr;
	uint32_t measure_len;
	struct rip_measure_strategy_str rip_measure_strategy;
};

struct rip_measure_status_str {
	uint8_t measure_status[RIP_MAX];
};




#endif
