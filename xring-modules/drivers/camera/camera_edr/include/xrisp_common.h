/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_EDR_COMMON_H__
#define __XRING_EDR_COMMON_H__

#include "xrisp_log.h"

#define edr_err(fmt, ...) \
	XRISP_PR_ERROR("[XRISP_DRV][edr]%s(%d)-<%s>: "fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define edr_warn(fmt, ...) \
	XRISP_PR_WARN("[XRISP_DRV][edr]%s(%d)-<%s>: "fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define edr_info(fmt, ...) \
	XRISP_PR_INFO("[XRISP_DRV][edr]%s(%d)-<%s>: "fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define edr_debug(fmt, ...) \
	XRISP_PR_DEBUG("[XRISP_DRV][edr]%s(%d)-<%s>: "fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

typedef void (*event_callback)(void *arg);

extern char logpath[24];

struct edr_event {
	uint16_t   type;
	uint16_t   sub_type;
	uint32_t   log_info;
	uint64_t   timestamp;
	uint8_t    reset;
	uint64_t   frame_id;
	char       logpath[24];
};

struct edr_node {
	struct edr_event *head;
	spinlock_t lock;
};

//maintype
enum {
	EDR_FW_PANIC = 0,
	EDR_FW_WDT_HANG,
	EDR_FW_TIMER_HANG,
	EDR_MIPI,
	EDR_FE_ROUTER,
	EDR_FE0,
	EDR_FE1,
	EDR_FE2,
	EDR_BE,
	EDR_PE,
	EDR_CVE,
	EDR_POWER,
	EDR_MCU,
	EDR_MAILBOX,
	EDR_IPC,
	EDR_MEM,
	EDR_I2C,
	EDR_I3C,
	EDR_OIS,
	EDR_CIS,
	EDR_EEPROM,
	EDR_ACTUATOR,
	EDR_THERMAL,
	EDR_META,
	EDR_MSS,
	EDR_RCI,
	EDR_CDI,
	EDR_SESSION,
	EDR_PIPELINE,
	EDR_STREAM,
	EDR_APERTURE,
	EDR_DVFS,
	EDR_MAIN_TYPE_MAX,
};

//subtype
enum {
	EDR_DATA_ERROR = 49,
	EDR_CRC_ERROR,
	EDR_TIMEOUT_ERROR,
	EDR_RX_ERROR,
	EDR_HSRX_TO_ERROR,
	EDR_DESKEW,
	EDR_PHY01,
	EDR_PHY02,
	EDR_CSI2,
	EDR_FRAME,
	EDR_LINE,
	EDR_SDI_HD_FIFO_ERR,
	EDR_SDI_PLD_FIFO_ERR,
	EDR_SDI_WC_ERR,
	EDR_BUS_ERR,
	EDR_AXI_ERR,
	EDR_WR_ERR,
	EDR_SDI_BUF_ERROR,
	EDR_SDI0_VBUS_ERROR,
	EDR_SDI1_VBUS_ERROR,
	EDR_SDI2_VBUS_ERROR,
	EDR_SDI0_BUF_FULL,
	EDR_SDI1_BUF_FULL,
	EDR_SDI2_BUF_FULL,
	EDR_IP_ERROR,
	EDR_DATA_DMA_READ_ERROR,
	EDR_DATA_DMA_WRITE_ERROR,
	EDR_CMD_DMA_ERROR,
	EDR_WARPEIS_SYS_ERROR,
	EDR_WAPRTNR_SYS_ERROR,
	EDR_CMDMA_CVE_ALIGN0_ERROR,
	EDR_CMDMA_CVE_ALIGN1_ERROR,
	EDR_CMDMA_CVE_MFNR_ERROR,
	EDR_REQUEST_TIMEOUT_ERROR,
	EDR_BUFFER_TIMEOUT_ERROR,
	EDR_FLUSH_TIMEOUT_ERROR,
	EDR_REQUEST_FLUSH_TIMEOUT_ERROR,
	EDR_MAILBOX_RX_FULL_ERROR,
	EDR_MAILBOX_TX_FULL_ERROR,
	EDR_MAILBOX_ACK_TIMEOUT_ERROR,
	EDR_IPC_RX_ERROR,
	EDR_IPC_TX_ERROR,
	EDR_CAM_RROBE_ERROR,
	EDR_CAM_INIT_ERROR,
	EDR_CAM_SET_DAC_ERROR,
	EDR_FW_BOOT_ERROR,
	EDR_BUF_ALLOC_MAP_ERROR,
	EDR_BUF_MAP_ERROR,
	EDR_IOVA_REGION_ALLOC_ERROR,
	EDR_SENSOR_POWER_UP_ERROR,
	EDR_CAM_COMMUNICATION_ERROR,
	EDR_WLDO_OV_OC_EEROR,
	EDR_HMS_TIMEOUT_ERROR,
	EDR_MSS_TIMEOUT_ERROR,
	EDR_DVFS_TOUT_ERROR,
	EDR_HAL_FATAL_ERROR,
	EDR_TYPE_MAX,
};

enum {
	XRING_S_DPU = 0X100,
	XRING_S_GPU,
	XRING_S_VPU_DEC,
	XRING_S_VPU_ENC,
	XRING_S_VPU_V4L2,
	XRING_S_NPU,
	XRING_S_AUDIO,
};

//subtype
enum {
	/* DPU */
	DPU_START              = 0x200,
	DPU_S_SW_CLEAR_FAULT   = DPU_START,
	DPU_S_MISS_TE_FAULT,
	DPU_S_ESD_FAULT,
	DPU_S_WAIT_FENCE_TMO_FAULT,
	DPU_S_CREATE_FENCE_FAULT,
	DPU_S_IRQ_SCHE_TMO_FAULT,
	DPU_S_DP_UNDERFLOW_FAULT,
	DPU_END,
	/* GPU */
	GPU_START              = 0x300,
	GPU_IRQ_FAULT          = GPU_START,
	GPU_BUS_FAULT,
	GPU_GPU_FAULT,
	GPU_CS_FAULT,
	GPU_UNHANDLE_PAGE_FAULT,
	GPU_MCU_UNHANDLE_PAGE_FAULT,
	GPU_SOFT_RESET_TIME_OUT,
	GPU_HARD_RESET_TIME_OUT,
	GPU_BIT_STUCK,
	GPU_REGULATOR_ON_FAIL,
	GPU_REGULATOR_OFF_FAIL,
	GPU_FENCE_TIMEOUT,
	GPU_INTERRUPT_TIMEOUT,
	GPU_CS_FATAL,
	GPU_END,
	/* VPU DEC */
	VPU_DEC_START          = 0x400,
	VPU_DEC_ABORT_TIMEOUT  = VPU_DEC_START,
	VPU_DEC_VCD_BUS_ERR,
	VPU_DEC_HANG_EXCEPTION,
	VPU_DEC_WDT_EXCEPTION,
	VPU_DEC_NOC_EXCEPTION,
	VPU_DEC_TIMEOUT_EXCEPTION,
	VPU_DEC_END,
	/* VPU ENC */
	VPU_ENC_START          = 0x500,
	VPU_ENC_ABORT_TIMEOUT  = VPU_ENC_START,
	VPU_ENC_HANG_EXCEPTION,
	VPU_ENC_WDT_EXCEPTION,
	VPU_ENC_NOC_EXCEPTION,
	VPU_ENC_TIMEOUT_EXCEPTION,
	VPU_ENC_END,
	/* VPU V4L2 */
	VPU_V4L2_START         = 0x600,
	VPU_V4L2_END,
	/* NPU */
	NPU_START              = 0x700,
	NPU_WDT0               = NPU_START,
	NPU_LOCKUP,
	NPU_SYSRST,
	VDSP_WDT1,
	VDSP_GPIO_NS,
	NPU_END,
	/* AUDIO */
	AUDIO_START            = 0x800,
	AUDIO_PANIC            = AUDIO_START,
	AUDIO_END,
};

//add for edr exit

#define EDR_RAMLOG       (1 << 0)
#define EDR_FW_COREDUMP  (1 << 1)
#define EDR_BANDWIDTH    (1 << 2)
#define EDR_STACKINFO    (1 << 3)
#define EDR_CPUINFO      (1 << 4)
#define EDR_MEMINFO      (1 << 5)
#define EDR_IRQINFO      (1 << 6)
#define EDR_IMAGE        (1 << 7)
#define EDR_PERFMONITOR  (1 << 8)
#define EDR_TRACE        (1 << 9)
#define EDR_LOGCAT       (1 << 10)
#define EDR_KERNEL_KMSG  (1 << 11)
#define EDR_FILE         (1 << 13)
#define EDR_OFFLINELOG   (1 << 14)
#define EDR_DDR_STATUS   (1 << 15)
#define EDR_DUMP_MAX     (1 << 15)

//panictype
#define FW_WDT_HANG          0xf1
#define FW_TIMER_HANG        0xf2
//others type is panic

//process
enum {
	WDT_STAGE_START = 1,
	WDT_STAGE_CREATE,
	WDT_STAGE_KICK0,   //used middle 4bytes
	WDT_STAGE_KICK1,   //used last 4bytes
	WDT_STAGE_TIMEOUT,
	WDT_STAGE_PANIC,
	WDT_STAGE_COREDUMP_START,
	WDT_STAGE_COREDUMP_END,
};

#define EDR_DEBUG      1

void edr_drv_submit_api(uint16_t type, uint16_t subtype, uint32_t loginfo, uint8_t reset, uint64_t frameid);

//for subsystem
int edr_drv_submit_api_sync(uint16_t type, uint16_t subtype, uint32_t loginfo, char *path);

#endif
