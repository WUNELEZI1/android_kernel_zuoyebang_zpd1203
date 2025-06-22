/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Description: smc fid definitions in bl31
 * This is a document file for reference
 *
 * 0xc3000000-0xc300FFFF SMC64: OEM Service Calls
 *
 * 0xc300 0000  ~  0xc300 FFFF +------> sip fid define
 *                        | |
 *                        | |
 *                        | +----------> Fid in module  (0 ~ 255)
 *                        |
 *                        +------------> Module Group   (0 ~ 255)
 *
 * check usage:
 *              FID_USE(fid) == BL31_BASE_GROUP
 *
 * Modify time: 2023-12-04
 * Author: Security-AP
 *
 */

#ifndef __BL31_SMC_ID_H__
#define __BL31_SMC_ID_H__

/* Get the group number of FID */
#define bl31_fid_use(x)                      (((x) & 0xff00) >> 8)

/* The base of OEM Service */
#define OEM_BASE                             0xc3U

/*
 * Module Group Number Define, must be named as follow:
 * BL31_MODULE_GROUP
 */
#define BL31_BASE_GROUP                      0x00
#define BL31_EFUSE_GROUP                     0x01
#define BL31_SHMEM_GROUP                     0x03
#define BL31_HPC_GROUP                       0x04
#define BL31_IP_REGULATOR_GROUP              0x05
#define BL31_PACBTI_GROUP                    0x06
#define BL31_IPC_GROUP                       0x08
#define BL31_DDR_GROUP                       0x09
#define BL31_UART_SWITCH_GROUP               0x0a
#define BL31_PM_INFO_GROUP                   0x0b
#define BL31_PERF_FLOWCTRL_GROUP             0x0c
#define BL31_VOTE_MNG_GROUP                  0x0d
#define BL31_PTDUMP_GROUP                    0x0e
#define BL31_ISP_GROUP                       0x0f
#define BL31_OCM_GROUP                       0x10
#define BL31_CLK_GROUP                       0x11
#define BL31_FW_TEST_GROUP                   0x12
#define BL31_SPMI_GROUP                      0x13
#define BL31_RESET_GROUP                     0x14
#define BL31_CPU_REG_GROUP                   0x15
#define BL31_PERF_MONITOR_GROUP              0x16
#define BL31_DPU_GROUP                       0x17
#define BL31_RAS_GROUP                       0x18
#define BL31_TZPC_GROUP                      0x19
#define BL31_MNTN_GROUP                      0x1a
#define BL31_FLOWCTRL_VOTE_GROUP             0x1b
#define BL31_PMIC_REGULATOR_GROUP            0x1c
#define BL31_IOC_GROUP                       0x1d
#define BL31_MITEE_GROUP                     0x1e
#define ATF_MDR_GROUP                        0x1f
#define BL31_XRSE_AGENT_GROUP                0x20
#define BL31_CPU_GROUP                       0x21
#define BL31_HWLOCK_GROUP                    0x22
#define BL31_GPU_PROFILE_GROUP               0x23
#define BL31_CPU_PROFILE_GROUP               0x24
#define BL31_NPU_PROFILE_GROUP               0x25
#define FFA_XRSP_TEST_GROUP                  0x26
#define BL31_XRSE_GROUP                      0x27
#define BL31_XSPM_GROUP                      0x28
#define SECAP_RIP_GROUP                      0x29
#define BL31_XHEE_GROUP                      0x2a
#define BL31_GIC_GROUP                       0x2b
#define BL31_SPI_MISC_GROUP                  0x2c
#define BL31_AVS_GROUP                       0x2d

#define BL31_TEST_GROUP                      0x2e
#define FID_ATF_MAXGROUP                     0x2f /* max group */

/* Get FID from group and id */
#define make_fid(group, id)                  ((((OEM_BASE << 16) + group) << \
                                             8) + id)

/*
 * Module Fid Number Define, must be named as follow:
 * FID_BL31_MODULE_FUNC
 */
/*
 * bl31:
 * 0xc3000000 ~ 0xc30000ff
 */
#define FID_BL31_SMC_TEST                    make_fid(BL31_BASE_GROUP, 0x00)

/*
 * efuse:
 * 0xc3000100 ~ 0xc30001ff
 */
#define FID_BL31_EFUSE_RD                    make_fid(BL31_EFUSE_GROUP, 0x00)
#define FID_BL31_EFUSE_WR                    make_fid(BL31_EFUSE_GROUP, 0x01)
#define FID_BL31_EFUSE_WR_CFG_SWITCH         make_fid(BL31_EFUSE_GROUP, 0x02)

/*
 * atf mdr:
 * 0xc3001f00 ~ 0xc3001fff
 */
#define FID_ATF_MDR_TEST                     make_fid(ATF_MDR_GROUP, 0x00)
#define FID_ATF_MDR_XHEE                     make_fid(ATF_MDR_GROUP, 0x01)
#define FID_ATF_MDR_XHEE_LOG                 make_fid(ATF_MDR_GROUP, 0x02)
#define FID_ATF_MDR_XSPM_LOG                 make_fid(ATF_MDR_GROUP, 0x03)
#define FID_ATF_MDR_XSP_LOG                  make_fid(ATF_MDR_GROUP, 0x04)

/*
 * bl31 shmem:
 * 0xc3000300 ~ 0xc30003ff
 */
#define FID_BL31_SHMEM_INFO                  make_fid(BL31_SHMEM_GROUP, 0x00)
#define FID_BL31_SHMEM_TEST_IN               make_fid(BL31_SHMEM_GROUP, 0x01)
#define FID_BL31_SHMEM_TEST_OUT              make_fid(BL31_SHMEM_GROUP, 0x02)
#define FID_BL31_SHMEM_TEST_INOUT            make_fid(BL31_SHMEM_GROUP, 0x03)

/*
 * bl31 hpc driver:
 * 0xc3000400 ~ 0xc30004ff
 */
#define FID_BL31_HPC_XRP_PWON                make_fid(BL31_HPC_GROUP, 0x00)
#define FID_BL31_HPC_XRP_PWOFF               make_fid(BL31_HPC_GROUP, 0x01)
#define FID_BL31_HPC_XRP_DISRESET            make_fid(BL31_HPC_GROUP, 0x02)
#define FID_BL31_HPC_XRP_RESET               make_fid(BL31_HPC_GROUP, 0x03)
#define FID_BL31_HPC_XRP_HALT                make_fid(BL31_HPC_GROUP, 0x04)
#define FID_BL31_HPC_XRP_RELEASE             make_fid(BL31_HPC_GROUP, 0x05)
#define FID_BL31_HPC_XRP_READREG             make_fid(BL31_HPC_GROUP, 0x06)
#define FID_BL31_HPC_BOOT_INFO               make_fid(BL31_HPC_GROUP, 0x07)
#define FID_BL31_HPC_BOOT                    make_fid(BL31_HPC_GROUP, 0x08)
#define FID_BL31_HPC_SHUTDOWN                make_fid(BL31_HPC_GROUP, 0x09)

#define FID_BL31_IP_REGULATOR_OFF            make_fid(BL31_IP_REGULATOR_GROUP, 0x00)
#define FID_BL31_IP_REGULATOR_ON             make_fid(BL31_IP_REGULATOR_GROUP, 0x01)

/*
 * bl31 pacbti:
 * 0xc3000600 ~ 0xc30006ff
 */
#define FID_BL31_PACIASP_TEST_NORMAL         make_fid(BL31_PACBTI_GROUP, 0x00)
#define FID_BL31_PACIASP_TEST_WRONG          make_fid(BL31_PACBTI_GROUP, 0x01)
#define FID_BL31_PACDA_TEST_NORMAL           make_fid(BL31_PACBTI_GROUP, 0x02)
#define FID_BL31_PACDA_TEST_WRONG            make_fid(BL31_PACBTI_GROUP, 0x03)
#define FID_BL31_PACIAX17_TEST_NORMAL        make_fid(BL31_PACBTI_GROUP, 0x04)
#define FID_BL31_PACIAX17_TEST_WRONG         make_fid(BL31_PACBTI_GROUP, 0x05)
#define FID_BL31_BTI_TEST_WRONG              make_fid(BL31_PACBTI_GROUP, 0x06)

/*
 * bl31 ipc:
 * 0xc3000800 ~ 0xc3008ff
 */
#define FID_BL31_IPC_RX_REG_TEST              make_fid(BL31_IPC_GROUP, 0x00)
#define FID_BL31_IPC_RX_UNREG_TEST            make_fid(BL31_IPC_GROUP, 0x01)
#define FID_BL31_IPC_SYNC_SEND_TEST           make_fid(BL31_IPC_GROUP, 0x02)
#define FID_BL31_IPC_ASYNC_SEND_TEST          make_fid(BL31_IPC_GROUP, 0x03)
#define FID_BL31_IPC_RESP_SEND_TEST           make_fid(BL31_IPC_GROUP, 0x04)
#define FID_BL31_IPC_LEN_OVERSIZE_TEST        make_fid(BL31_IPC_GROUP, 0x05)
#define FID_BL31_IPC_INVALIED_MSG_TEST        make_fid(BL31_IPC_GROUP, 0x06)
#define FID_BL31_IPC_PROT_SHORT_MSG_TEST      make_fid(BL31_IPC_GROUP, 0x07)
#define FID_BL31_IPC_PROT_LONG_MSG_TEST       make_fid(BL31_IPC_GROUP, 0x08)
#define FID_BL31_IPC_PROT_LONG_MSG_PRES_TEST  make_fid(BL31_IPC_GROUP, 0x09)

/*
 * bl31 ddr driver:
 * 0xc3000900 ~ 0xc30009ff
 */
#define FID_BL31_DDR_PATTERN_TRACE            make_fid(BL31_DDR_GROUP, 0x00)
#define FID_BL31_DDR_CLOCK_GATE               make_fid(BL31_DDR_GROUP, 0x01)
#define FID_BL31_DDR_PROFILE_MONITOR          make_fid(BL31_DDR_GROUP, 0x02)

/*
 * bl31 uart switch:
 * 0xc3000a00 ~ 0xc3000aff
 */
#define FID_BL31_UART_SWITCH                  make_fid(BL31_UART_SWITCH_GROUP, 0x00)

/*
 * bl31 pm info:
 * 0xc3000b00 ~ 0xc3000bff
 */
#define FID_BL31_PM_INFO                     make_fid(BL31_PM_INFO_GROUP, 0x00)
#define FID_BL31_PM_WRITE                    make_fid(BL31_PM_INFO_GROUP, 0x01)

/*
 * bl31 perf flowctrl driver:
 * 0xc3000c00 ~ 0xc3000cff
 */
#define FID_BL31_PERF_FLOWCTRL_CFG           make_fid(BL31_PERF_FLOWCTRL_GROUP, 0x00)


/*
 * bl31 vote mng:
 * 0xc3000d00 ~ 0xc3000dff
 */
#define FID_BL31_VOTE_MNG_GET                 make_fid(BL31_VOTE_MNG_GROUP, 0x00)

/*
 * bl31 ptdump:
 * 0xc3000e00 ~ 0xc3000eff
 */
#define FID_BL31_PTDUMP_DUMP                  make_fid(BL31_PTDUMP_GROUP, 0x00)
#define FID_BL31_PTDUMP_INFO                  make_fid(BL31_PTDUMP_GROUP, 0x01)
#define FID_BL31_PTDUMP_COPY                  make_fid(BL31_PTDUMP_GROUP, 0x02)

/*
 * bl31 isp driver:
 * 0xc3000f00 ~ 0xc3000fff
 */
#define FID_BL31_ISP_READ_REG                make_fid(BL31_ISP_GROUP, 0x00)
#define FID_BL31_ISP_WRITE_REG               make_fid(BL31_ISP_GROUP, 0x01)
#define FID_BL31_ISP_DMA_RESET               make_fid(BL31_ISP_GROUP, 0x02)

/*
 * bl31 ocm:
 * 0xc3001000 ~ 0xc30010ff
 */
#define FID_BL31_OCM_SET_ATTR                  make_fid(BL31_OCM_GROUP, 0x00)
#define FID_BL31_OCM_CLR_ATTR                  make_fid(BL31_OCM_GROUP, 0x01)

/*
 * bl31 firmware test driver:
 * 0xc3001200 ~ 0xc30012ff
 */
#define FID_BL31_FW_TEST_ASAN                  make_fid(BL31_FW_TEST_GROUP, 0x00)

/*
 * bl31 spmi:
 * 0xc3001300 ~ 0xc30013ff
 */
#define FID_SPMI_BURST_READ                    make_fid(BL31_SPMI_GROUP, 0x00)
#define FID_SPMI_BURST_WRITE                   make_fid(BL31_SPMI_GROUP, 0x01)

/*
 * bl31 reset:
 * 0xc3001400 ~ 0xc30014ff
 */
#define FID_BL31_RESET_SET                    make_fid(BL31_RESET_GROUP, 0x00)
#define FID_BL31_RESET_GET                    make_fid(BL31_RESET_GROUP, 0x01)

/*
 * bl31 cpu_reg driver:
 * 0xc3001500 ~ 0xc30015ff
 */
#define FID_BL31_CPU_REG_READ_EXTREG          make_fid(BL31_CPU_REG_GROUP, 0x00)
#define FID_BL31_CPU_REG_WRITE_EXTREG         make_fid(BL31_CPU_REG_GROUP, 0x01)
#define FID_BL31_CPU_REG_READ_SYSREG          make_fid(BL31_CPU_REG_GROUP, 0x02)
#define FID_BL31_CPU_REG_WRITE_SYSREG         make_fid(BL31_CPU_REG_GROUP, 0x03)

/*
 * bl31 perf monitor driver:
 * 0xc3001600 ~ 0xc30016ff
 */
#define FID_BL31_PERF_MONITOR_CFG              make_fid(BL31_PERF_MONITOR_GROUP, 0x00)

/*
 * bl31 dpu driver:
 * 0xc3001700 ~ 0xc30017ff
 */
#define FID_BL31_DPU_READ_REG                   make_fid(BL31_DPU_GROUP, 0x00)
#define FID_BL31_DPU_WRITE_REG                  make_fid(BL31_DPU_GROUP, 0x01)
#define FID_BL31_DPU_PERI_DSI_ON_REG            make_fid(BL31_DPU_GROUP, 0x02)
#define FID_BL31_DPU_DVS_AUTO_GT_CFG            make_fid(BL31_DPU_GROUP, 0x03)
#define FID_BL31_DPU_DP_POWER_ENABLE            make_fid(BL31_DPU_GROUP, 0x04)
#define FID_BL31_DPU_DP_CP_IRQ_SET              make_fid(BL31_DPU_GROUP, 0x05)
#define FID_BL31_DPU_RCH_MID_ENABLE             make_fid(BL31_DPU_GROUP, 0x06)
#define FID_BL31_DPU_WB_MID_ENABLE              make_fid(BL31_DPU_GROUP, 0x07)
#define FID_BL31_DPU_DP_POWER_STATE_HANDLE      make_fid(BL31_DPU_GROUP, 0x08)

/*
 * bl31 ras:
 * 0xc3001800 ~ 0xc30018ff
 */
#define FID_BL31_RAS_EA                        make_fid(BL31_RAS_GROUP, 0x00)
#define FID_BL31_RAS_INTERRUPT_OP              make_fid(BL31_RAS_GROUP, 0x01)

/*
 * XRSE tzpc call:
 * 0xc3001800 ~ 0xc30018ff
 */
#define FID_XRSE_TZPC_SET                      make_fid(BL31_TZPC_GROUP, 0x00)

/*
 * bl31 mntn driver:
 * 0xc3001a00 ~ 0xc3001aff
 */
#define FID_BL31_MNTN_M3_NMI_REG               make_fid(BL31_MNTN_GROUP, 0x00)
#define FID_BL31_MNTN_ATB_SH_CLK               make_fid(BL31_MNTN_GROUP, 0x01)
#define FID_BL31_MNTN_XRSE_IDLE_REG            make_fid(BL31_MNTN_GROUP, 0x02)
#define FID_BL31_MNTN_FLUSH_DFX_CACHE          make_fid(BL31_MNTN_GROUP, 0x03)

/*
 * bl31 perf flowctrl vote driver:
 * 0xc3001b00 ~ 0xc3001bff
 */
#define FID_BL31_PERF_FLOWCTRL_VOTE            make_fid(BL31_FLOWCTRL_VOTE_GROUP, 0x00)

/*
 * Regulator call:
 * 0xc3001c00 ~ 0xc3001cff
 */
#define FID_BL31_PMIC_REGULATOR_ENABLE         make_fid(BL31_PMIC_REGULATOR_GROUP, 0x00)
#define FID_BL31_PMIC_REGULATOR_DISABLE        make_fid(BL31_PMIC_REGULATOR_GROUP, 0x01)
#define FID_BL31_PMIC_REGULATOR_IS_EN          make_fid(BL31_PMIC_REGULATOR_GROUP, 0x02)
#define FID_BL31_PMIC_REGULATOR_SET_MV         make_fid(BL31_PMIC_REGULATOR_GROUP, 0x03)
#define FID_BL31_PMIC_REGULATOR_GET_MV         make_fid(BL31_PMIC_REGULATOR_GROUP, 0x04)

/*
 * ioc cfg/mux call:
 * 0xc3001d00 ~ 0xc3001dff
 */
#define FID_IOC_CONFIG_GET                      make_fid(BL31_IOC_GROUP, 0x00)
#define FID_IOC_CONFIG_SET                      make_fid(BL31_IOC_GROUP, 0x01)
#define FID_IOC_FUNC_GET                        make_fid(BL31_IOC_GROUP, 0x02)
#define FID_IOC_FUNC_SET                        make_fid(BL31_IOC_GROUP, 0x03)

/*
 * bl31 MITEE driver:
 * 0xc3001e00 ~ 0xc3001eff
 */
#define FID_MITEE_EXCEPTION_HANDLE             make_fid(BL31_MITEE_GROUP, 0x00)
#define FID_MITEE_ROT_HANDLE                   make_fid(BL31_MITEE_GROUP, 0x01)
#define FID_MITEE_MAGIC_NUM_HANDLE             make_fid(BL31_MITEE_GROUP, 0x02)

/*
 * bl31 xrse agent:
 * 0xc3001d00 ~ 0xc3001dff
 */
#define FID_XRSE_AGENT_CHIPTEST                make_fid(BL31_XRSE_AGENT_GROUP, 0x00)
#define FID_XRSE_AGENT_GETLOG                  make_fid(BL31_XRSE_AGENT_GROUP, 0x01)

/*
 * bl31 gpu:
 * 0xc3002300 ~ 0xc30023ff
 */
#define FID_BL31_GPU_POWER_GET                 make_fid(BL31_GPU_PROFILE_GROUP, 0x00)
#define FID_BL31_GPU_PROFILE_GET               make_fid(BL31_GPU_PROFILE_GROUP, 0x01)
#define FID_BL31_GPU_USAGE_GET                 make_fid(BL31_GPU_PROFILE_GROUP, 0x02)
#define FID_BL31_IP_FREQ_TRACE_DUMP            make_fid(BL31_GPU_PROFILE_GROUP, 0x03)

/*
 * bl31 npu lp:
 * 0xc3002500 ~ 0xc30025ff
 */
#define FID_BL31_NPU_PROFILE_GET               make_fid(BL31_NPU_PROFILE_GROUP, 0x01)

/*
 * bl31 cpu:
 * 0xc3001c00 ~ 0xc3001cff
 */
#define FID_BL31_CPU_FLUSHCACHE_BY_SETWAY      make_fid(BL31_CPU_GROUP, 0x00)
#define FIC_BL31_CPU_SET_RET_MODE              make_fid(BL31_CPU_GROUP, 0x01)
#define FIC_BL31_CPU_GET_RET_MODE              make_fid(BL31_CPU_GROUP, 0x02)

/*
 * hwlock trylock/unlock call:
 * 0xc3002200 ~ 0xc30022ff
 */
#define FID_HWLOCK_TRYLOCK                      make_fid(BL31_HWLOCK_GROUP, 0x00)
#define FID_HWLOCK_UNLOCK                       make_fid(BL31_HWLOCK_GROUP, 0x01)

/*
 * bl31 cpu profile:
 * 0xc3002400 ~ 0xc30024ff
 */
#define FID_BL31_CPU_PROFILE_GET               make_fid(BL31_CPU_PROFILE_GROUP, 0x00)
#define FID_BL31_CPU_USAGE_GET                 make_fid(BL31_CPU_PROFILE_GROUP, 0x01)

/*
 * ffa xrsp test:
 * 0xc3002600 ~ 0xc30026ff
 */
#define XRSP_GET_VERSION_TEST                  make_fid(FFA_XRSP_TEST_GROUP, 0x00)
#define XRSP_RXTX_MAP_ERR_TEST                 make_fid(FFA_XRSP_TEST_GROUP, 0x01)
#define XRSP_RXTX_UNMAP_ERR_TEST               make_fid(FFA_XRSP_TEST_GROUP, 0x02)
#define XRSP_RXTX_UNMAP_TEST                   make_fid(FFA_XRSP_TEST_GROUP, 0x03)
#define XRSP_GET_ID_TEST                       make_fid(FFA_XRSP_TEST_GROUP, 0x04)
#define XRSP_MEM_SHARE_NORMAL_TEST             make_fid(FFA_XRSP_TEST_GROUP, 0x05)
#define XRSP_MEM_SHARE_FAIL_TEST               make_fid(FFA_XRSP_TEST_GROUP, 0x06)
#define XRSP_MEM_RETRIEVE_NORMAL_TEST          make_fid(FFA_XRSP_TEST_GROUP, 0x07)
#define XRSP_MEM_RETRIEVE_FAIL_TEST            make_fid(FFA_XRSP_TEST_GROUP, 0x08)

/*
 * bl31 xrse:
 * 0xc3002700 ~ 0xc30027ff
 */
#define FID_BL31_XRSE_VOTE                     make_fid(BL31_XRSE_GROUP, 0x00)
#define FID_BL31_XRSE_FUZZ                     make_fid(BL31_XRSE_GROUP, 0x01)

/*
 * bl31 xspm:
 * 0xc3002800 ~ 0xc30028ff
 */
#define FID_BL31_XSPM_MDR                     make_fid(BL31_XSPM_GROUP, 0x00)
#define FID_BL31_XSP_MDR                      make_fid(BL31_XSPM_GROUP, 0x01)

#define FID_BL31_XRSE_RIP_BASE_REQUEST        make_fid(BL31_XRSE_GROUP, 0x02)
#define FID_BL31_XRSE_RIP_REALTIME_REQUEST    make_fid(BL31_XRSE_GROUP, 0x03)
#define FID_BL31_XRSE_RIP_GET_STATUS_REQUEST  make_fid(BL31_XRSE_GROUP, 0x04)

#define FID_BL31_XRSE_TEST                    make_fid(BL31_XRSE_GROUP, 0x05)

/*
 * rip for secap: bl31 xhee xspm xsp
 * 0xc3002900 ~ 0xc30029ff
 */
#define FID_SECAP_RIP_BL31                    make_fid(SECAP_RIP_GROUP, 0x00)
#define FID_SECAP_RIP_XHEE                    make_fid(SECAP_RIP_GROUP, 0x01)
#define FID_SECAP_RIP_XSPM                    make_fid(SECAP_RIP_GROUP, 0x02)
#define FID_SECAP_RIP_XSP                     make_fid(SECAP_RIP_GROUP, 0x03)

/*
 * bl31 xhee:
 * 0xc3002a00 ~ 0xc3002aff
 */
#define FID_XHEE_ENABLED_STATUS               make_fid(BL31_XHEE_GROUP, 0x00)

/*
 * bl31 gic:
 * 0xc3002b00 ~ 0xc3002bff
 */
#define FID_BL31_GIC_E1NWF                    make_fid(BL31_GIC_GROUP, 0x00)
#define FID_BL31_GIC_SET_CLASS                make_fid(BL31_GIC_GROUP, 0x01)

/*
 * bl31 spi:
 * 0xc3002c00 ~ 0xc3002cff
 */
#define FID_SPI_MISC_PINCTRL                  make_fid(BL31_SPI_MISC_GROUP, 0x00)

/*
 * bl31 avs:
 * 0xc3002d00 ~ 0xc3002dff
 */
#define FID_BL31_AVS_GET_HPM_ERROR            make_fid(BL31_AVS_GROUP, 0x00)
#define FID_BL31_AVS_GET_RUNNING_TIME         make_fid(BL31_AVS_GROUP, 0x01)

/*
 * bl31 test group for kernel
 * 0xc3002e00 ~ 0xc3002eff
 */
#define FID_BL31_TEST_PASS_THROUGH            make_fid(BL31_TEST_GROUP, 0x00)
#define FID_BL31_TEST_SEC_MEASURE             make_fid(BL31_TEST_GROUP, 0x01)

#endif
