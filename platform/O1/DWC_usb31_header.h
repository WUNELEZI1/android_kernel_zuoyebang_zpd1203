// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef DWC_USB31_HEADER_H
#define DWC_USB31_HEADER_H

#define DWC_usb31_block_gbl_BaseAddress 0xc100



#define GSBUSCFG0 (DWC_usb31_block_gbl_BaseAddress + 0x0)
#define GSBUSCFG0_RegisterSize 32
#define GSBUSCFG0_RegisterResetValue 0x1
#define GSBUSCFG0_RegisterResetMask 0xffff0cff





#define GSBUSCFG0_INCRBRSTENA_BitAddressOffset 0
#define GSBUSCFG0_INCRBRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR4BRSTENA_BitAddressOffset 1
#define GSBUSCFG0_INCR4BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR8BRSTENA_BitAddressOffset 2
#define GSBUSCFG0_INCR8BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR16BRSTENA_BitAddressOffset 3
#define GSBUSCFG0_INCR16BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR32BRSTENA_BitAddressOffset 4
#define GSBUSCFG0_INCR32BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR64BRSTENA_BitAddressOffset 5
#define GSBUSCFG0_INCR64BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR128BRSTENA_BitAddressOffset 6
#define GSBUSCFG0_INCR128BRSTENA_RegisterSize 1



#define GSBUSCFG0_INCR256BRSTENA_BitAddressOffset 7
#define GSBUSCFG0_INCR256BRSTENA_RegisterSize 1



#define GSBUSCFG0_reserved_9_8_BitAddressOffset 8
#define GSBUSCFG0_reserved_9_8_RegisterSize 2



#define GSBUSCFG0_DESBIGEND_BitAddressOffset 10
#define GSBUSCFG0_DESBIGEND_RegisterSize 1



#define GSBUSCFG0_DATBIGEND_BitAddressOffset 11
#define GSBUSCFG0_DATBIGEND_RegisterSize 1



#define GSBUSCFG0_reserved_15_12_BitAddressOffset 12
#define GSBUSCFG0_reserved_15_12_RegisterSize 4



#define GSBUSCFG0_DESWRREQINFO_BitAddressOffset 16
#define GSBUSCFG0_DESWRREQINFO_RegisterSize 4



#define GSBUSCFG0_DATWRREQINFO_BitAddressOffset 20
#define GSBUSCFG0_DATWRREQINFO_RegisterSize 4



#define GSBUSCFG0_DESRDREQINFO_BitAddressOffset 24
#define GSBUSCFG0_DESRDREQINFO_RegisterSize 4



#define GSBUSCFG0_DATRDREQINFO_BitAddressOffset 28
#define GSBUSCFG0_DATRDREQINFO_RegisterSize 4





#define GSBUSCFG1 (DWC_usb31_block_gbl_BaseAddress + 0x4)
#define GSBUSCFG1_RegisterSize 32
#define GSBUSCFG1_RegisterResetValue 0x2f00
#define GSBUSCFG1_RegisterResetMask 0x1ff00





#define GSBUSCFG1_reserved_7_0_BitAddressOffset 0
#define GSBUSCFG1_reserved_7_0_RegisterSize 8



#define GSBUSCFG1_PipeTransLimit_BitAddressOffset 8
#define GSBUSCFG1_PipeTransLimit_RegisterSize 4



#define GSBUSCFG1_EN1KPAGE_BitAddressOffset 12
#define GSBUSCFG1_EN1KPAGE_RegisterSize 1



#define GSBUSCFG1_ExtdPipeTransLimit_BitAddressOffset 13
#define GSBUSCFG1_ExtdPipeTransLimit_RegisterSize 4



#define GSBUSCFG1_reserved_31_17_BitAddressOffset 17
#define GSBUSCFG1_reserved_31_17_RegisterSize 15





#define GTXTHRCFG (DWC_usb31_block_gbl_BaseAddress + 0x8)
#define GTXTHRCFG_RegisterSize 32
#define GTXTHRCFG_RegisterResetValue 0x0
#define GTXTHRCFG_RegisterResetMask 0x7ffe7ff





#define GTXTHRCFG_UsbMaxTxBurstSize_Prd_BitAddressOffset 0
#define GTXTHRCFG_UsbMaxTxBurstSize_Prd_RegisterSize 5



#define GTXTHRCFG_UsbTxThrNumPkt_Prd_BitAddressOffset 5
#define GTXTHRCFG_UsbTxThrNumPkt_Prd_RegisterSize 5



#define GTXTHRCFG_UsbTxThrNumPktSel_Prd_BitAddressOffset 10
#define GTXTHRCFG_UsbTxThrNumPktSel_Prd_RegisterSize 1



#define GTXTHRCFG_reserved_12_11_BitAddressOffset 11
#define GTXTHRCFG_reserved_12_11_RegisterSize 2



#define GTXTHRCFG_UsbTxThrNumPkt_HS_Prd_BitAddressOffset 13
#define GTXTHRCFG_UsbTxThrNumPkt_HS_Prd_RegisterSize 2



#define GTXTHRCFG_UsbTxThrNumPktSel_HS_Prd_BitAddressOffset 15
#define GTXTHRCFG_UsbTxThrNumPktSel_HS_Prd_RegisterSize 1



#define GTXTHRCFG_UsbMaxTxBurstSize_BitAddressOffset 16
#define GTXTHRCFG_UsbMaxTxBurstSize_RegisterSize 5



#define GTXTHRCFG_UsbTxPktCnt_BitAddressOffset 21
#define GTXTHRCFG_UsbTxPktCnt_RegisterSize 5



#define GTXTHRCFG_UsbTxPktCntSel_BitAddressOffset 26
#define GTXTHRCFG_UsbTxPktCntSel_RegisterSize 1



#define GTXTHRCFG_reserved_31_27_BitAddressOffset 27
#define GTXTHRCFG_reserved_31_27_RegisterSize 5





#define GRXTHRCFG (DWC_usb31_block_gbl_BaseAddress + 0xc)
#define GRXTHRCFG_RegisterSize 32
#define GRXTHRCFG_RegisterResetValue 0x700000
#define GRXTHRCFG_RegisterResetMask 0x7ffe7ff





#define GRXTHRCFG_UsbMaxRxBurstSize_Prd_BitAddressOffset 0
#define GRXTHRCFG_UsbMaxRxBurstSize_Prd_RegisterSize 5



#define GRXTHRCFG_UsbRxThrNumPkt_Prd_BitAddressOffset 5
#define GRXTHRCFG_UsbRxThrNumPkt_Prd_RegisterSize 5



#define GRXTHRCFG_UsbRxThrNumPktSel_Prd_BitAddressOffset 10
#define GRXTHRCFG_UsbRxThrNumPktSel_Prd_RegisterSize 1



#define GRXTHRCFG_reserved_12_11_BitAddressOffset 11
#define GRXTHRCFG_reserved_12_11_RegisterSize 2



#define GRXTHRCFG_UsbRxThrNumPkt_HS_Prd_BitAddressOffset 13
#define GRXTHRCFG_UsbRxThrNumPkt_HS_Prd_RegisterSize 2



#define GRXTHRCFG_UsbRxThrNumPktSel_HS_Prd_BitAddressOffset 15
#define GRXTHRCFG_UsbRxThrNumPktSel_HS_Prd_RegisterSize 1



#define GRXTHRCFG_UsbMaxRxBurstSize_BitAddressOffset 16
#define GRXTHRCFG_UsbMaxRxBurstSize_RegisterSize 5



#define GRXTHRCFG_UsbRxPktCnt_BitAddressOffset 21
#define GRXTHRCFG_UsbRxPktCnt_RegisterSize 5



#define GRXTHRCFG_UsbRxPktCntSel_BitAddressOffset 26
#define GRXTHRCFG_UsbRxPktCntSel_RegisterSize 1



#define GRXTHRCFG_reserved_31_27_BitAddressOffset 27
#define GRXTHRCFG_reserved_31_27_RegisterSize 5





#define GCTL (DWC_usb31_block_gbl_BaseAddress + 0x10)
#define GCTL_RegisterSize 32
#define GCTL_RegisterResetValue 0x112004
#define GCTL_RegisterResetMask 0xfffffbff





#define GCTL_DSBLCLKGTNG_BitAddressOffset 0
#define GCTL_DSBLCLKGTNG_RegisterSize 1



#define GCTL_GblHibernationEn_BitAddressOffset 1
#define GCTL_GblHibernationEn_RegisterSize 1



#define GCTL_U2EXIT_LFPS_BitAddressOffset 2
#define GCTL_U2EXIT_LFPS_RegisterSize 1



#define GCTL_DISSCRAMBLE_BitAddressOffset 3
#define GCTL_DISSCRAMBLE_RegisterSize 1



#define GCTL_SCALEDOWN_BitAddressOffset 4
#define GCTL_SCALEDOWN_RegisterSize 2



#define GCTL_RAMCLKSEL_BitAddressOffset 6
#define GCTL_RAMCLKSEL_RegisterSize 2



#define GCTL_DEBUGATTACH_BitAddressOffset 8
#define GCTL_DEBUGATTACH_RegisterSize 1



#define GCTL_U1U2TimerScale_BitAddressOffset 9
#define GCTL_U1U2TimerScale_RegisterSize 1



#define GCTL_reserved_10_BitAddressOffset 10
#define GCTL_reserved_10_RegisterSize 1



#define GCTL_CORESOFTRESET_BitAddressOffset 11
#define GCTL_CORESOFTRESET_RegisterSize 1



#define GCTL_PRTCAPDIR_BitAddressOffset 12
#define GCTL_PRTCAPDIR_RegisterSize 2



#define GCTL_FRMSCLDWN_BitAddressOffset 14
#define GCTL_FRMSCLDWN_RegisterSize 2



#define GCTL_U2RSTECN_BitAddressOffset 16
#define GCTL_U2RSTECN_RegisterSize 1



#define GCTL_BYPSSETADDR_BitAddressOffset 17
#define GCTL_BYPSSETADDR_RegisterSize 1



#define GCTL_MASTERFILTBYPASS_BitAddressOffset 18
#define GCTL_MASTERFILTBYPASS_RegisterSize 1



#define GCTL_PWRDNSCALE_BitAddressOffset 19
#define GCTL_PWRDNSCALE_RegisterSize 13





#define GPMSTS (DWC_usb31_block_gbl_BaseAddress + 0x14)
#define GPMSTS_RegisterSize 32
#define GPMSTS_RegisterResetValue 0x0
#define GPMSTS_RegisterResetMask 0xf001f3ff





#define GPMSTS_U2Wakeup_BitAddressOffset 0
#define GPMSTS_U2Wakeup_RegisterSize 10



#define GPMSTS_reserved_11_10_BitAddressOffset 10
#define GPMSTS_reserved_11_10_RegisterSize 2



#define GPMSTS_U3Wakeup_BitAddressOffset 12
#define GPMSTS_U3Wakeup_RegisterSize 5



#define GPMSTS_reserved_27_17_BitAddressOffset 17
#define GPMSTS_reserved_27_17_RegisterSize 11



#define GPMSTS_PortSel_BitAddressOffset 28
#define GPMSTS_PortSel_RegisterSize 4





#define GSTS (DWC_usb31_block_gbl_BaseAddress + 0x18)
#define GSTS_RegisterSize 32
#define GSTS_RegisterResetValue 0x7e800000
#define GSTS_RegisterResetMask 0xfff00fd3





#define GSTS_CURMOD_BitAddressOffset 0
#define GSTS_CURMOD_RegisterSize 2



#define GSTS_reserved_3_2_BitAddressOffset 2
#define GSTS_reserved_3_2_RegisterSize 2



#define GSTS_BUSERRADDRVLD_BitAddressOffset 4
#define GSTS_BUSERRADDRVLD_RegisterSize 1



#define GSTS_CSRTimeout_BitAddressOffset 5
#define GSTS_CSRTimeout_RegisterSize 1



#define GSTS_Device_IP_BitAddressOffset 6
#define GSTS_Device_IP_RegisterSize 1



#define GSTS_Host_IP_BitAddressOffset 7
#define GSTS_Host_IP_RegisterSize 1



#define GSTS_reserved_11_8_BitAddressOffset 8
#define GSTS_reserved_11_8_RegisterSize 4



#define GSTS_reserved_19_12_BitAddressOffset 12
#define GSTS_reserved_19_12_RegisterSize 8



#define GSTS_CBELT_BitAddressOffset 20
#define GSTS_CBELT_RegisterSize 12





#define GUCTL1 (DWC_usb31_block_gbl_BaseAddress + 0x1c)
#define GUCTL1_RegisterSize 32
#define GUCTL1_RegisterResetValue 0x1988
#define GUCTL1_RegisterResetMask 0xf9e07dfd





#define GUCTL1_LOA_FILTER_EN_BitAddressOffset 0
#define GUCTL1_LOA_FILTER_EN_RegisterSize 1



#define GUCTL1_reserved_1_BitAddressOffset 1
#define GUCTL1_reserved_1_RegisterSize 1



#define GUCTL1_HC_PARCHK_DISABLE_BitAddressOffset 2
#define GUCTL1_HC_PARCHK_DISABLE_RegisterSize 1



#define GUCTL1_HC_ERRATA_ENABLE_BitAddressOffset 3
#define GUCTL1_HC_ERRATA_ENABLE_RegisterSize 1



#define GUCTL1_L1_SUSP_THRLD_FOR_HOST_BitAddressOffset 4
#define GUCTL1_L1_SUSP_THRLD_FOR_HOST_RegisterSize 4



#define GUCTL1_L1_SUSP_THRLD_EN_FOR_HOST_BitAddressOffset 8
#define GUCTL1_L1_SUSP_THRLD_EN_FOR_HOST_RegisterSize 1



#define GUCTL1_reserved_9_BitAddressOffset 9
#define GUCTL1_reserved_9_RegisterSize 1



#define GUCTL1_RESUME_OPMODE_HS_HOST_BitAddressOffset 10
#define GUCTL1_RESUME_OPMODE_HS_HOST_RegisterSize 1



#define GUCTL1_DisRefClkGtng_BitAddressOffset 11
#define GUCTL1_DisRefClkGtng_RegisterSize 1



#define GUCTL1_DisUSB2RefClkGtng_BitAddressOffset 12
#define GUCTL1_DisUSB2RefClkGtng_RegisterSize 1



#define GUCTL1_HW_LPM_HLE_DISABLE_BitAddressOffset 13
#define GUCTL1_HW_LPM_HLE_DISABLE_RegisterSize 1



#define GUCTL1_HW_LPM_CAP_DISABLE_BitAddressOffset 14
#define GUCTL1_HW_LPM_CAP_DISABLE_RegisterSize 1



#define GUCTL1_reserved_19_15_BitAddressOffset 15
#define GUCTL1_reserved_19_15_RegisterSize 5



#define GUCTL1_reserved_20_BitAddressOffset 20
#define GUCTL1_reserved_20_RegisterSize 1



#define GUCTL1_IP_GAP_ADD_ON_BitAddressOffset 21
#define GUCTL1_IP_GAP_ADD_ON_RegisterSize 3



#define GUCTL1_DEV_L1_EXIT_BY_HW_BitAddressOffset 24
#define GUCTL1_DEV_L1_EXIT_BY_HW_RegisterSize 1



#define GUCTL1_reserved_25_BitAddressOffset 25
#define GUCTL1_reserved_25_RegisterSize 1



#define GUCTL1_reserved_26_BitAddressOffset 26
#define GUCTL1_reserved_26_RegisterSize 1



#define GUCTL1_DEV_TRB_OUT_SPR_IND_BitAddressOffset 27
#define GUCTL1_DEV_TRB_OUT_SPR_IND_RegisterSize 1



#define GUCTL1_TX_IPGAP_LINECHECK_DIS_BitAddressOffset 28
#define GUCTL1_TX_IPGAP_LINECHECK_DIS_RegisterSize 1



#define GUCTL1_FILTER_SE0_FSLS_EOP_BitAddressOffset 29
#define GUCTL1_FILTER_SE0_FSLS_EOP_RegisterSize 1



#define GUCTL1_DS_RXDET_MAX_TOUT_CTRL_BitAddressOffset 30
#define GUCTL1_DS_RXDET_MAX_TOUT_CTRL_RegisterSize 1



#define GUCTL1_DEV_DECOUPLE_L1L2_EVT_BitAddressOffset 31
#define GUCTL1_DEV_DECOUPLE_L1L2_EVT_RegisterSize 1





#define USB31_IP_NAME (DWC_usb31_block_gbl_BaseAddress + 0x20)
#define USB31_IP_NAME_RegisterSize 32
#define USB31_IP_NAME_RegisterResetValue 0x33313130
#define USB31_IP_NAME_RegisterResetMask 0xffffffff





#define USB31_IP_NAME_SYNOPSYSIP_BitAddressOffset 0
#define USB31_IP_NAME_SYNOPSYSIP_RegisterSize 32





#define GGPIO (DWC_usb31_block_gbl_BaseAddress + 0x24)
#define GGPIO_RegisterSize 32
#define GGPIO_RegisterResetValue 0x0
#define GGPIO_RegisterResetMask 0xffffffff





#define GGPIO_GPI_BitAddressOffset 0
#define GGPIO_GPI_RegisterSize 16



#define GGPIO_GPO_BitAddressOffset 16
#define GGPIO_GPO_RegisterSize 16





#define GUID (DWC_usb31_block_gbl_BaseAddress + 0x28)
#define GUID_RegisterSize 32
#define GUID_RegisterResetValue 0x12345678
#define GUID_RegisterResetMask 0xffffffff





#define GUID_USERID_BitAddressOffset 0
#define GUID_USERID_RegisterSize 32





#define GUCTL (DWC_usb31_block_gbl_BaseAddress + 0x2c)
#define GUCTL_RegisterSize 32
#define GUCTL_RegisterResetValue 0xd056802
#define GUCTL_RegisterResetMask 0xffff5fff





#define GUCTL_DTOUT_BitAddressOffset 0
#define GUCTL_DTOUT_RegisterSize 11



#define GUCTL_InsrtExtrFSBODI_BitAddressOffset 11
#define GUCTL_InsrtExtrFSBODI_RegisterSize 1



#define GUCTL_ExtCapSupptEN_BitAddressOffset 12
#define GUCTL_ExtCapSupptEN_RegisterSize 1



#define GUCTL_star_4452686_ctl_BitAddressOffset 13
#define GUCTL_star_4452686_ctl_RegisterSize 1



#define GUCTL_USBHstInImmRetryEn_BitAddressOffset 14
#define GUCTL_USBHstInImmRetryEn_RegisterSize 1



#define GUCTL_reserved_15_BitAddressOffset 15
#define GUCTL_reserved_15_RegisterSize 1



#define GUCTL_ResBwHSEPS_BitAddressOffset 16
#define GUCTL_ResBwHSEPS_RegisterSize 1



#define GUCTL_SprsCtrlTransEn_BitAddressOffset 17
#define GUCTL_SprsCtrlTransEn_RegisterSize 1



#define GUCTL_EN_EXTD_TBC_CAP_BitAddressOffset 18
#define GUCTL_EN_EXTD_TBC_CAP_RegisterSize 1



#define GUCTL_IgnoreHCETimeout_BitAddressOffset 19
#define GUCTL_IgnoreHCETimeout_RegisterSize 1



#define GUCTL_DMAIgnoreHCE_BitAddressOffset 20
#define GUCTL_DMAIgnoreHCE_RegisterSize 1



#define GUCTL_NoExtrDl_BitAddressOffset 21
#define GUCTL_NoExtrDl_RegisterSize 1



#define GUCTL_REFCLKPER_BitAddressOffset 22
#define GUCTL_REFCLKPER_RegisterSize 10





#define GBUSERRADDRLO (DWC_usb31_block_gbl_BaseAddress + 0x30)
#define GBUSERRADDRLO_RegisterSize 32
#define GBUSERRADDRLO_RegisterResetValue 0x0
#define GBUSERRADDRLO_RegisterResetMask 0xffffffff





#define GBUSERRADDRLO_BUSERRADDR_BitAddressOffset 0
#define GBUSERRADDRLO_BUSERRADDR_RegisterSize 32





#define GBUSERRADDRHI (DWC_usb31_block_gbl_BaseAddress + 0x34)
#define GBUSERRADDRHI_RegisterSize 32
#define GBUSERRADDRHI_RegisterResetValue 0x0
#define GBUSERRADDRHI_RegisterResetMask 0xffffffff





#define GBUSERRADDRHI_BUSERRADDR_BitAddressOffset 0
#define GBUSERRADDRHI_BUSERRADDR_RegisterSize 32





#define GPRTBIMAPLO (DWC_usb31_block_gbl_BaseAddress + 0x38)
#define GPRTBIMAPLO_RegisterSize 32
#define GPRTBIMAPLO_RegisterResetValue 0x0
#define GPRTBIMAPLO_RegisterResetMask 0xffffffff





#define GPRTBIMAPLO_BINUM1_BitAddressOffset 0
#define GPRTBIMAPLO_BINUM1_RegisterSize 4



#define GPRTBIMAPLO_BINUM2_BitAddressOffset 4
#define GPRTBIMAPLO_BINUM2_RegisterSize 4



#define GPRTBIMAPLO_BINUM3_BitAddressOffset 8
#define GPRTBIMAPLO_BINUM3_RegisterSize 4



#define GPRTBIMAPLO_BINUM4_BitAddressOffset 12
#define GPRTBIMAPLO_BINUM4_RegisterSize 4



#define GPRTBIMAPLO_BINUM5_BitAddressOffset 16
#define GPRTBIMAPLO_BINUM5_RegisterSize 4



#define GPRTBIMAPLO_BINUM6_BitAddressOffset 20
#define GPRTBIMAPLO_BINUM6_RegisterSize 4



#define GPRTBIMAPLO_BINUM7_BitAddressOffset 24
#define GPRTBIMAPLO_BINUM7_RegisterSize 4



#define GPRTBIMAPLO_BINUM8_BitAddressOffset 28
#define GPRTBIMAPLO_BINUM8_RegisterSize 4





#define GPRTBIMAPHI (DWC_usb31_block_gbl_BaseAddress + 0x3c)
#define GPRTBIMAPHI_RegisterSize 32
#define GPRTBIMAPHI_RegisterResetValue 0x0
#define GPRTBIMAPHI_RegisterResetMask 0xfffffff





#define GPRTBIMAPHI_BINUM9_BitAddressOffset 0
#define GPRTBIMAPHI_BINUM9_RegisterSize 4



#define GPRTBIMAPHI_BINUM10_BitAddressOffset 4
#define GPRTBIMAPHI_BINUM10_RegisterSize 4



#define GPRTBIMAPHI_BINUM11_BitAddressOffset 8
#define GPRTBIMAPHI_BINUM11_RegisterSize 4



#define GPRTBIMAPHI_BINUM12_BitAddressOffset 12
#define GPRTBIMAPHI_BINUM12_RegisterSize 4



#define GPRTBIMAPHI_BINUM13_BitAddressOffset 16
#define GPRTBIMAPHI_BINUM13_RegisterSize 4



#define GPRTBIMAPHI_BINUM14_BitAddressOffset 20
#define GPRTBIMAPHI_BINUM14_RegisterSize 4



#define GPRTBIMAPHI_BINUM15_BitAddressOffset 24
#define GPRTBIMAPHI_BINUM15_RegisterSize 4



#define GPRTBIMAPHI_reserved_31_28_BitAddressOffset 28
#define GPRTBIMAPHI_reserved_31_28_RegisterSize 4





#define GHWPARAMS0 (DWC_usb31_block_gbl_BaseAddress + 0x40)
#define GHWPARAMS0_RegisterSize 32
#define GHWPARAMS0_RegisterResetValue 0x4020804a
#define GHWPARAMS0_RegisterResetMask 0xffffffff





#define GHWPARAMS0_ghwparams0_2_0_BitAddressOffset 0
#define GHWPARAMS0_ghwparams0_2_0_RegisterSize 3



#define GHWPARAMS0_ghwparams0_5_3_BitAddressOffset 3
#define GHWPARAMS0_ghwparams0_5_3_RegisterSize 3



#define GHWPARAMS0_ghwparams0_7_6_BitAddressOffset 6
#define GHWPARAMS0_ghwparams0_7_6_RegisterSize 2



#define GHWPARAMS0_ghwparams0_15_8_BitAddressOffset 8
#define GHWPARAMS0_ghwparams0_15_8_RegisterSize 8



#define GHWPARAMS0_ghwparams0_23_16_BitAddressOffset 16
#define GHWPARAMS0_ghwparams0_23_16_RegisterSize 8



#define GHWPARAMS0_ghwparams0_31_24_BitAddressOffset 24
#define GHWPARAMS0_ghwparams0_31_24_RegisterSize 8





#define GHWPARAMS1 (DWC_usb31_block_gbl_BaseAddress + 0x44)
#define GHWPARAMS1_RegisterSize 32
#define GHWPARAMS1_RegisterResetValue 0x1032485
#define GHWPARAMS1_RegisterResetMask 0xffffffff





#define GHWPARAMS1_ghwparams1_3_0_BitAddressOffset 0
#define GHWPARAMS1_ghwparams1_3_0_RegisterSize 4



#define GHWPARAMS1_ghwparams1_7_4_BitAddressOffset 4
#define GHWPARAMS1_ghwparams1_7_4_RegisterSize 4



#define GHWPARAMS1_ghwparams1_10_8_BitAddressOffset 8
#define GHWPARAMS1_ghwparams1_10_8_RegisterSize 3



#define GHWPARAMS1_ghwparams1_13_11_BitAddressOffset 11
#define GHWPARAMS1_ghwparams1_13_11_RegisterSize 3



#define GHWPARAMS1_ghwparams1_16_14_BitAddressOffset 14
#define GHWPARAMS1_ghwparams1_16_14_RegisterSize 3



#define GHWPARAMS1_ghwparams1_22_17_BitAddressOffset 17
#define GHWPARAMS1_ghwparams1_22_17_RegisterSize 6



#define GHWPARAMS1_ghwparams1_23_BitAddressOffset 23
#define GHWPARAMS1_ghwparams1_23_RegisterSize 1



#define GHWPARAMS1_ghwparams1_25_24_BitAddressOffset 24
#define GHWPARAMS1_ghwparams1_25_24_RegisterSize 2



#define GHWPARAMS1_ghwparams1_26_BitAddressOffset 26
#define GHWPARAMS1_ghwparams1_26_RegisterSize 1



#define GHWPARAMS1_ghwparams1_27_BitAddressOffset 27
#define GHWPARAMS1_ghwparams1_27_RegisterSize 1



#define GHWPARAMS1_ghwparams1_28_BitAddressOffset 28
#define GHWPARAMS1_ghwparams1_28_RegisterSize 1



#define GHWPARAMS1_ghwparams1_29_BitAddressOffset 29
#define GHWPARAMS1_ghwparams1_29_RegisterSize 1



#define GHWPARAMS1_ghwparams1_30_BitAddressOffset 30
#define GHWPARAMS1_ghwparams1_30_RegisterSize 1



#define GHWPARAMS1_ghwparams1_31_BitAddressOffset 31
#define GHWPARAMS1_ghwparams1_31_RegisterSize 1





#define GHWPARAMS2 (DWC_usb31_block_gbl_BaseAddress + 0x48)
#define GHWPARAMS2_RegisterSize 32
#define GHWPARAMS2_RegisterResetValue 0x12345678
#define GHWPARAMS2_RegisterResetMask 0xffffffff





#define GHWPARAMS2_ghwparams2_31_0_BitAddressOffset 0
#define GHWPARAMS2_ghwparams2_31_0_RegisterSize 32





#define GHWPARAMS3 (DWC_usb31_block_gbl_BaseAddress + 0x4c)
#define GHWPARAMS3_RegisterSize 32
#define GHWPARAMS3_RegisterResetValue 0x904200b6
#define GHWPARAMS3_RegisterResetMask 0xffffffff





#define GHWPARAMS3_ghwparams3_1_0_BitAddressOffset 0
#define GHWPARAMS3_ghwparams3_1_0_RegisterSize 2



#define GHWPARAMS3_ghwparams3_3_2_BitAddressOffset 2
#define GHWPARAMS3_ghwparams3_3_2_RegisterSize 2



#define GHWPARAMS3_ghwparams3_4_BitAddressOffset 4
#define GHWPARAMS3_ghwparams3_4_RegisterSize 1



#define GHWPARAMS3_ghwparams3_5_BitAddressOffset 5
#define GHWPARAMS3_ghwparams3_5_RegisterSize 1



#define GHWPARAMS3_ghwparams3_7_6_BitAddressOffset 6
#define GHWPARAMS3_ghwparams3_7_6_RegisterSize 2



#define GHWPARAMS3_ghwparams3_9_8_BitAddressOffset 8
#define GHWPARAMS3_ghwparams3_9_8_RegisterSize 2



#define GHWPARAMS3_ghwparams3_10_BitAddressOffset 10
#define GHWPARAMS3_ghwparams3_10_RegisterSize 1



#define GHWPARAMS3_ghwparams3_11_BitAddressOffset 11
#define GHWPARAMS3_ghwparams3_11_RegisterSize 1



#define GHWPARAMS3_ghwparams3_17_12_BitAddressOffset 12
#define GHWPARAMS3_ghwparams3_17_12_RegisterSize 6



#define GHWPARAMS3_ghwparams3_22_18_BitAddressOffset 18
#define GHWPARAMS3_ghwparams3_22_18_RegisterSize 5



#define GHWPARAMS3_ghwparams3_30_23_BitAddressOffset 23
#define GHWPARAMS3_ghwparams3_30_23_RegisterSize 8



#define GHWPARAMS3_ghwparams3_31_BitAddressOffset 31
#define GHWPARAMS3_ghwparams3_31_RegisterSize 1





#define GHWPARAMS4 (DWC_usb31_block_gbl_BaseAddress + 0x50)
#define GHWPARAMS4_RegisterSize 32
#define GHWPARAMS4_RegisterResetValue 0x48422010
#define GHWPARAMS4_RegisterResetMask 0xffffffff





#define GHWPARAMS4_ghwparams4_5_0_BitAddressOffset 0
#define GHWPARAMS4_ghwparams4_5_0_RegisterSize 6



#define GHWPARAMS4_ghwparams4_12_6_BitAddressOffset 6
#define GHWPARAMS4_ghwparams4_12_6_RegisterSize 7



#define GHWPARAMS4_ghwparams4_16_13_BitAddressOffset 13
#define GHWPARAMS4_ghwparams4_16_13_RegisterSize 4



#define GHWPARAMS4_ghwparams4_20_17_BitAddressOffset 17
#define GHWPARAMS4_ghwparams4_20_17_RegisterSize 4



#define GHWPARAMS4_ghwparams4_21_BitAddressOffset 21
#define GHWPARAMS4_ghwparams4_21_RegisterSize 1



#define GHWPARAMS4_ghwparams4_22_BitAddressOffset 22
#define GHWPARAMS4_ghwparams4_22_RegisterSize 1



#define GHWPARAMS4_ghwparams4_27_23_BitAddressOffset 23
#define GHWPARAMS4_ghwparams4_27_23_RegisterSize 5



#define GHWPARAMS4_ghwparams4_31_28_BitAddressOffset 28
#define GHWPARAMS4_ghwparams4_31_28_RegisterSize 4





#define GHWPARAMS5 (DWC_usb31_block_gbl_BaseAddress + 0x54)
#define GHWPARAMS5_RegisterSize 32
#define GHWPARAMS5_RegisterResetValue 0x643d0410
#define GHWPARAMS5_RegisterResetMask 0xffffffff





#define GHWPARAMS5_ghwparams5_4_0_BitAddressOffset 0
#define GHWPARAMS5_ghwparams5_4_0_RegisterSize 5



#define GHWPARAMS5_ghwparams5_10_5_BitAddressOffset 5
#define GHWPARAMS5_ghwparams5_10_5_RegisterSize 6



#define GHWPARAMS5_ghwparams5_16_11_BitAddressOffset 11
#define GHWPARAMS5_ghwparams5_16_11_RegisterSize 6



#define GHWPARAMS5_ghwparams5_22_17_BitAddressOffset 17
#define GHWPARAMS5_ghwparams5_22_17_RegisterSize 6



#define GHWPARAMS5_ghwparams5_28_23_BitAddressOffset 23
#define GHWPARAMS5_ghwparams5_28_23_RegisterSize 6



#define GHWPARAMS5_ghwparams5_31_29_BitAddressOffset 29
#define GHWPARAMS5_ghwparams5_31_29_RegisterSize 3





#define GHWPARAMS6 (DWC_usb31_block_gbl_BaseAddress + 0x58)
#define GHWPARAMS6_RegisterSize 32
#define GHWPARAMS6_RegisterResetValue 0xfdc803f
#define GHWPARAMS6_RegisterResetMask 0xffffffff





#define GHWPARAMS6_ghwparams6_5_0_BitAddressOffset 0
#define GHWPARAMS6_ghwparams6_5_0_RegisterSize 6



#define GHWPARAMS6_ghwparams6_6_BitAddressOffset 6
#define GHWPARAMS6_ghwparams6_6_RegisterSize 1



#define GHWPARAMS6_ghwparams6_7_BitAddressOffset 7
#define GHWPARAMS6_ghwparams6_7_RegisterSize 1



#define GHWPARAMS6_ghwparams6_9_8_BitAddressOffset 8
#define GHWPARAMS6_ghwparams6_9_8_RegisterSize 2



#define GHWPARAMS6_ghwparams6_14_10_BitAddressOffset 10
#define GHWPARAMS6_ghwparams6_14_10_RegisterSize 5



#define GHWPARAMS6_BusFltrsSupport_BitAddressOffset 15
#define GHWPARAMS6_BusFltrsSupport_RegisterSize 1



#define GHWPARAMS6_ghwparams6_31_16_BitAddressOffset 16
#define GHWPARAMS6_ghwparams6_31_16_RegisterSize 16





#define GHWPARAMS7 (DWC_usb31_block_gbl_BaseAddress + 0x5c)
#define GHWPARAMS7_RegisterSize 32
#define GHWPARAMS7_RegisterResetValue 0x3500882
#define GHWPARAMS7_RegisterResetMask 0xffffffff





#define GHWPARAMS7_ghwparams7_15_0_BitAddressOffset 0
#define GHWPARAMS7_ghwparams7_15_0_RegisterSize 16



#define GHWPARAMS7_ghwparams7_31_16_BitAddressOffset 16
#define GHWPARAMS7_ghwparams7_31_16_RegisterSize 16





#define GDBGFIFOSPACE (DWC_usb31_block_gbl_BaseAddress + 0x60)
#define GDBGFIFOSPACE_RegisterSize 32
#define GDBGFIFOSPACE_RegisterResetValue 0x230000
#define GDBGFIFOSPACE_RegisterResetMask 0xffff01ff





#define GDBGFIFOSPACE_FIFO_QUEUE_SELECT_BitAddressOffset 0
#define GDBGFIFOSPACE_FIFO_QUEUE_SELECT_RegisterSize 9



#define GDBGFIFOSPACE_reserved_15_9_BitAddressOffset 9
#define GDBGFIFOSPACE_reserved_15_9_RegisterSize 7



#define GDBGFIFOSPACE_SPACE_AVAILABLE_BitAddressOffset 16
#define GDBGFIFOSPACE_SPACE_AVAILABLE_RegisterSize 16





#define GBMUCTL (DWC_usb31_block_gbl_BaseAddress + 0x64)
#define GBMUCTL_RegisterSize 32
#define GBMUCTL_RegisterResetValue 0x9cc20026
#define GBMUCTL_RegisterResetMask 0x3f





#define GBMUCTL_reserved_0_BitAddressOffset 0
#define GBMUCTL_reserved_0_RegisterSize 1



#define GBMUCTL_active_id_en_BitAddressOffset 1
#define GBMUCTL_active_id_en_RegisterSize 1



#define GBMUCTL_axi_storder_en_BitAddressOffset 2
#define GBMUCTL_axi_storder_en_RegisterSize 1



#define GBMUCTL_reserved_4_3_BitAddressOffset 3
#define GBMUCTL_reserved_4_3_RegisterSize 2



#define GBMUCTL_separate_psq_en_BitAddressOffset 5
#define GBMUCTL_separate_psq_en_RegisterSize 1



#define GBMUCTL_reserved_15_6_BitAddressOffset 6
#define GBMUCTL_reserved_15_6_RegisterSize 10



#define GBMUCTL_reserved_31_16_BitAddressOffset 16
#define GBMUCTL_reserved_31_16_RegisterSize 16





#define GBMUPSQWMARK (DWC_usb31_block_gbl_BaseAddress + 0x68)
#define GBMUPSQWMARK_RegisterSize 32
#define GBMUPSQWMARK_RegisterResetValue 0x8298
#define GBMUPSQWMARK_RegisterResetMask 0xffffffff





#define GBMUPSQWMARK_psq_water_mark_BitAddressOffset 0
#define GBMUPSQWMARK_psq_water_mark_RegisterSize 7



#define GBMUPSQWMARK_psq_water_mark_cdc_margin_BitAddressOffset 7
#define GBMUPSQWMARK_psq_water_mark_cdc_margin_RegisterSize 4



#define GBMUPSQWMARK_riq_water_mark_BitAddressOffset 11
#define GBMUPSQWMARK_riq_water_mark_RegisterSize 7



#define GBMUPSQWMARK_rsvd_18_31_BitAddressOffset 18
#define GBMUPSQWMARK_rsvd_18_31_RegisterSize 14





#define GDBGBMU (DWC_usb31_block_gbl_BaseAddress + 0x6c)
#define GDBGBMU_RegisterSize 32
#define GDBGBMU_RegisterResetValue 0x0
#define GDBGBMU_RegisterResetMask 0x0





#define GDBGBMU_BMU_CCU_BitAddressOffset 0
#define GDBGBMU_BMU_CCU_RegisterSize 4



#define GDBGBMU_BMU_DCU_BitAddressOffset 4
#define GDBGBMU_BMU_DCU_RegisterSize 4



#define GDBGBMU_BMU_BCU_BitAddressOffset 8
#define GDBGBMU_BMU_BCU_RegisterSize 24





#define GDBGLSPMUX (DWC_usb31_block_gbl_BaseAddress + 0x70)
#define GDBGLSPMUX_RegisterSize 32
#define GDBGLSPMUX_RegisterResetValue 0x3f0000
#define GDBGLSPMUX_RegisterResetMask 0x0





#define GDBGLSPMUX_LSPSELECT_BitAddressOffset 0
#define GDBGLSPMUX_LSPSELECT_RegisterSize 15



#define GDBGLSPMUX_reserved_15_BitAddressOffset 15
#define GDBGLSPMUX_reserved_15_RegisterSize 1



#define GDBGLSPMUX_logic_analyzer_trace_BitAddressOffset 16
#define GDBGLSPMUX_logic_analyzer_trace_RegisterSize 8



#define GDBGLSPMUX_reserved_31_24_BitAddressOffset 24
#define GDBGLSPMUX_reserved_31_24_RegisterSize 8





#define GDBGLSP (DWC_usb31_block_gbl_BaseAddress + 0x74)
#define GDBGLSP_RegisterSize 32
#define GDBGLSP_RegisterResetValue 0x0
#define GDBGLSP_RegisterResetMask 0x0





#define GDBGLSP_LSPDEBUG_BitAddressOffset 0
#define GDBGLSP_LSPDEBUG_RegisterSize 32





#define GDBGEPINFO0 (DWC_usb31_block_gbl_BaseAddress + 0x78)
#define GDBGEPINFO0_RegisterSize 32
#define GDBGEPINFO0_RegisterResetValue 0x0
#define GDBGEPINFO0_RegisterResetMask 0x0





#define GDBGEPINFO0_EPDEBUG_BitAddressOffset 0
#define GDBGEPINFO0_EPDEBUG_RegisterSize 32





#define GDBGEPINFO1 (DWC_usb31_block_gbl_BaseAddress + 0x7c)
#define GDBGEPINFO1_RegisterSize 32
#define GDBGEPINFO1_RegisterResetValue 0x800000
#define GDBGEPINFO1_RegisterResetMask 0x0





#define GDBGEPINFO1_EPDEBUG_BitAddressOffset 0
#define GDBGEPINFO1_EPDEBUG_RegisterSize 32





#define GPRTBIMAP_HSLO (DWC_usb31_block_gbl_BaseAddress + 0x80)
#define GPRTBIMAP_HSLO_RegisterSize 32
#define GPRTBIMAP_HSLO_RegisterResetValue 0x0
#define GPRTBIMAP_HSLO_RegisterResetMask 0xffffffff





#define GPRTBIMAP_HSLO_BINUM1_BitAddressOffset 0
#define GPRTBIMAP_HSLO_BINUM1_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM2_BitAddressOffset 4
#define GPRTBIMAP_HSLO_BINUM2_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM3_BitAddressOffset 8
#define GPRTBIMAP_HSLO_BINUM3_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM4_BitAddressOffset 12
#define GPRTBIMAP_HSLO_BINUM4_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM5_BitAddressOffset 16
#define GPRTBIMAP_HSLO_BINUM5_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM6_BitAddressOffset 20
#define GPRTBIMAP_HSLO_BINUM6_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM7_BitAddressOffset 24
#define GPRTBIMAP_HSLO_BINUM7_RegisterSize 4



#define GPRTBIMAP_HSLO_BINUM8_BitAddressOffset 28
#define GPRTBIMAP_HSLO_BINUM8_RegisterSize 4





#define GPRTBIMAP_HSHI (DWC_usb31_block_gbl_BaseAddress + 0x84)
#define GPRTBIMAP_HSHI_RegisterSize 32
#define GPRTBIMAP_HSHI_RegisterResetValue 0x0
#define GPRTBIMAP_HSHI_RegisterResetMask 0xfffffff





#define GPRTBIMAP_HSHI_BINUM9_BitAddressOffset 0
#define GPRTBIMAP_HSHI_BINUM9_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM10_BitAddressOffset 4
#define GPRTBIMAP_HSHI_BINUM10_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM11_BitAddressOffset 8
#define GPRTBIMAP_HSHI_BINUM11_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM12_BitAddressOffset 12
#define GPRTBIMAP_HSHI_BINUM12_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM13_BitAddressOffset 16
#define GPRTBIMAP_HSHI_BINUM13_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM14_BitAddressOffset 20
#define GPRTBIMAP_HSHI_BINUM14_RegisterSize 4



#define GPRTBIMAP_HSHI_BINUM15_BitAddressOffset 24
#define GPRTBIMAP_HSHI_BINUM15_RegisterSize 4



#define GPRTBIMAP_HSHI_reserved_31_28_BitAddressOffset 28
#define GPRTBIMAP_HSHI_reserved_31_28_RegisterSize 4





#define GPRTBIMAP_FSLO (DWC_usb31_block_gbl_BaseAddress + 0x88)
#define GPRTBIMAP_FSLO_RegisterSize 32
#define GPRTBIMAP_FSLO_RegisterResetValue 0x0
#define GPRTBIMAP_FSLO_RegisterResetMask 0xffffffff





#define GPRTBIMAP_FSLO_BINUM1_BitAddressOffset 0
#define GPRTBIMAP_FSLO_BINUM1_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM2_BitAddressOffset 4
#define GPRTBIMAP_FSLO_BINUM2_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM3_BitAddressOffset 8
#define GPRTBIMAP_FSLO_BINUM3_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM4_BitAddressOffset 12
#define GPRTBIMAP_FSLO_BINUM4_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM5_BitAddressOffset 16
#define GPRTBIMAP_FSLO_BINUM5_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM6_BitAddressOffset 20
#define GPRTBIMAP_FSLO_BINUM6_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM7_BitAddressOffset 24
#define GPRTBIMAP_FSLO_BINUM7_RegisterSize 4



#define GPRTBIMAP_FSLO_BINUM8_BitAddressOffset 28
#define GPRTBIMAP_FSLO_BINUM8_RegisterSize 4





#define GPRTBIMAP_FSHI (DWC_usb31_block_gbl_BaseAddress + 0x8c)
#define GPRTBIMAP_FSHI_RegisterSize 32
#define GPRTBIMAP_FSHI_RegisterResetValue 0x0
#define GPRTBIMAP_FSHI_RegisterResetMask 0xfffffff





#define GPRTBIMAP_FSHI_BINUM9_BitAddressOffset 0
#define GPRTBIMAP_FSHI_BINUM9_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM10_BitAddressOffset 4
#define GPRTBIMAP_FSHI_BINUM10_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM11_BitAddressOffset 8
#define GPRTBIMAP_FSHI_BINUM11_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM12_BitAddressOffset 12
#define GPRTBIMAP_FSHI_BINUM12_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM13_BitAddressOffset 16
#define GPRTBIMAP_FSHI_BINUM13_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM14_BitAddressOffset 20
#define GPRTBIMAP_FSHI_BINUM14_RegisterSize 4



#define GPRTBIMAP_FSHI_BINUM15_BitAddressOffset 24
#define GPRTBIMAP_FSHI_BINUM15_RegisterSize 4



#define GPRTBIMAP_FSHI_reserved_31_28_BitAddressOffset 28
#define GPRTBIMAP_FSHI_reserved_31_28_RegisterSize 4





#define GHMSOCBWOR (DWC_usb31_block_gbl_BaseAddress + 0x90)
#define GHMSOCBWOR_RegisterSize 32
#define GHMSOCBWOR_RegisterResetValue 0x0
#define GHMSOCBWOR_RegisterResetMask 0xffffffff





#define GHMSOCBWOR_ovrd_soc_rd_uF_kB_bandwidth_BitAddressOffset 0
#define GHMSOCBWOR_ovrd_soc_rd_uF_kB_bandwidth_RegisterSize 15



#define GHMSOCBWOR_ovrd_soc_wr_uF_kB_bandwidth_BitAddressOffset 15
#define GHMSOCBWOR_ovrd_soc_wr_uF_kB_bandwidth_RegisterSize 15



#define GHMSOCBWOR_ovrd_common_soc_rd_wr_BitAddressOffset 30
#define GHMSOCBWOR_ovrd_common_soc_rd_wr_RegisterSize 1



#define GHMSOCBWOR_ovrd_port_soc_bw_BitAddressOffset 31
#define GHMSOCBWOR_ovrd_port_soc_bw_RegisterSize 1





#define USB31_VER_NUMBER (DWC_usb31_block_gbl_BaseAddress + 0xa0)
#define USB31_VER_NUMBER_RegisterSize 32
#define USB31_VER_NUMBER_RegisterResetValue 0x3230302a
#define USB31_VER_NUMBER_RegisterResetMask 0xffffffff





#define USB31_VER_NUMBER_VERSION_NUM_BitAddressOffset 0
#define USB31_VER_NUMBER_VERSION_NUM_RegisterSize 32





#define USB31_VER_TYPE (DWC_usb31_block_gbl_BaseAddress + 0xa4)
#define USB31_VER_TYPE_RegisterSize 32
#define USB31_VER_TYPE_RegisterResetValue 0x67612a2a
#define USB31_VER_TYPE_RegisterResetMask 0xffffffff





#define USB31_VER_TYPE_VERSIONTYPE_BitAddressOffset 0
#define USB31_VER_TYPE_VERSIONTYPE_RegisterSize 32





#define GSYSBLKWINCTRL (DWC_usb31_block_gbl_BaseAddress + 0xb0)
#define GSYSBLKWINCTRL_RegisterSize 32
#define GSYSBLKWINCTRL_RegisterResetValue 0x0
#define GSYSBLKWINCTRL_RegisterResetMask 0xffffffff





#define GSYSBLKWINCTRL_beginining_no_blocking_time_BitAddressOffset 0
#define GSYSBLKWINCTRL_beginining_no_blocking_time_RegisterSize 8



#define GSYSBLKWINCTRL_end_no_blocking_time_BitAddressOffset 8
#define GSYSBLKWINCTRL_end_no_blocking_time_RegisterSize 8



#define GSYSBLKWINCTRL_reserved_23_16_BitAddressOffset 16
#define GSYSBLKWINCTRL_reserved_23_16_RegisterSize 8



#define GSYSBLKWINCTRL_erst_prefetching_watermark_BitAddressOffset 24
#define GSYSBLKWINCTRL_erst_prefetching_watermark_RegisterSize 5



#define GSYSBLKWINCTRL_erst_prefetching_en_BitAddressOffset 29
#define GSYSBLKWINCTRL_erst_prefetching_en_RegisterSize 1



#define GSYSBLKWINCTRL_disable_no_blocking_window_BitAddressOffset 30
#define GSYSBLKWINCTRL_disable_no_blocking_window_RegisterSize 1



#define GSYSBLKWINCTRL_sys_blocking_ok_all_the_time_BitAddressOffset 31
#define GSYSBLKWINCTRL_sys_blocking_ok_all_the_time_RegisterSize 1





#define GPCIEL1EXTLAT (DWC_usb31_block_gbl_BaseAddress + 0xb4)
#define GPCIEL1EXTLAT_RegisterSize 32
#define GPCIEL1EXTLAT_RegisterResetValue 0x0
#define GPCIEL1EXTLAT_RegisterResetMask 0xc0000fff





#define GPCIEL1EXTLAT_pcie_l1_exit_latency_BitAddressOffset 0
#define GPCIEL1EXTLAT_pcie_l1_exit_latency_RegisterSize 12



#define GPCIEL1EXTLAT_reserved_29_12_BitAddressOffset 12
#define GPCIEL1EXTLAT_reserved_29_12_RegisterSize 18



#define GPCIEL1EXTLAT_pcie_l1_exit_mode_ctrl_BitAddressOffset 30
#define GPCIEL1EXTLAT_pcie_l1_exit_mode_ctrl_RegisterSize 2





#define GUCTL2_USB4 (DWC_usb31_block_gbl_BaseAddress + 0xb8)
#define GUCTL2_USB4_RegisterSize 32
#define GUCTL2_USB4_RegisterResetValue 0x18044246
#define GUCTL2_USB4_RegisterResetMask 0xffffffff





#define GUCTL2_USB4_SCHEDDULE_LT_THR_BitAddressOffset 0
#define GUCTL2_USB4_SCHEDDULE_LT_THR_RegisterSize 1



#define GUCTL2_USB4_MAX_REISU_CNT_BitAddressOffset 1
#define GUCTL2_USB4_MAX_REISU_CNT_RegisterSize 2



#define GUCTL2_USB4_SVC_OPP_PER_FSLS_BitAddressOffset 3
#define GUCTL2_USB4_SVC_OPP_PER_FSLS_RegisterSize 2



#define GUCTL2_USB4_SVC_OPP_PER_HS_BitAddressOffset 5
#define GUCTL2_USB4_SVC_OPP_PER_HS_RegisterSize 2



#define GUCTL2_USB4_SVC_OPP_PER_SS_BitAddressOffset 7
#define GUCTL2_USB4_SVC_OPP_PER_SS_RegisterSize 2



#define GUCTL2_USB4_SVC_OPP_PER_SSP_BitAddressOffset 9
#define GUCTL2_USB4_SVC_OPP_PER_SSP_RegisterSize 2



#define GUCTL2_USB4_PERIODIC_TXDMA_UF_THR_BitAddressOffset 11
#define GUCTL2_USB4_PERIODIC_TXDMA_UF_THR_RegisterSize 7



#define GUCTL2_USB4_ASYNC_UF_THR_BitAddressOffset 18
#define GUCTL2_USB4_ASYNC_UF_THR_RegisterSize 7



#define GUCTL2_USB4_PERIODIC_UF_THR_BitAddressOffset 25
#define GUCTL2_USB4_PERIODIC_UF_THR_RegisterSize 7





#define GRXTHRCFG_USB4 (DWC_usb31_block_gbl_BaseAddress + 0xbc)
#define GRXTHRCFG_USB4_RegisterSize 32
#define GRXTHRCFG_USB4_RegisterResetValue 0x700000
#define GRXTHRCFG_USB4_RegisterResetMask 0x7ffe7ff





#define GRXTHRCFG_USB4_UsbMaxRxBurstSize_Prd_BitAddressOffset 0
#define GRXTHRCFG_USB4_UsbMaxRxBurstSize_Prd_RegisterSize 5



#define GRXTHRCFG_USB4_UsbRxThrNumPkt_Prd_BitAddressOffset 5
#define GRXTHRCFG_USB4_UsbRxThrNumPkt_Prd_RegisterSize 5



#define GRXTHRCFG_USB4_UsbRxThrNumPktSel_Prd_BitAddressOffset 10
#define GRXTHRCFG_USB4_UsbRxThrNumPktSel_Prd_RegisterSize 1



#define GRXTHRCFG_USB4_reserved_12_11_BitAddressOffset 11
#define GRXTHRCFG_USB4_reserved_12_11_RegisterSize 2



#define GRXTHRCFG_USB4_UsbRxThrNumPkt_HS_Prd_BitAddressOffset 13
#define GRXTHRCFG_USB4_UsbRxThrNumPkt_HS_Prd_RegisterSize 2



#define GRXTHRCFG_USB4_UsbRxThrNumPktSel_HS_Prd_BitAddressOffset 15
#define GRXTHRCFG_USB4_UsbRxThrNumPktSel_HS_Prd_RegisterSize 1



#define GRXTHRCFG_USB4_UsbMaxRxBurstSize_BitAddressOffset 16
#define GRXTHRCFG_USB4_UsbMaxRxBurstSize_RegisterSize 5



#define GRXTHRCFG_USB4_UsbRxPktCnt_BitAddressOffset 21
#define GRXTHRCFG_USB4_UsbRxPktCnt_RegisterSize 5



#define GRXTHRCFG_USB4_UsbRxPktCntSel_BitAddressOffset 26
#define GRXTHRCFG_USB4_UsbRxPktCntSel_RegisterSize 1



#define GRXTHRCFG_USB4_reserved_31_27_BitAddressOffset 27
#define GRXTHRCFG_USB4_reserved_31_27_RegisterSize 5





#define GUSB2PHYCFG_REGS (DWC_usb31_block_gbl_BaseAddress + 0x100)
#define GUSB2PHYCFG_RegisterSize 32
#define GUSB2PHYCFG_RegisterResetValue 0x2102400
#define GUSB2PHYCFG_RegisterResetMask 0xbffeffff





#define GUSB2PHYCFG_TOutCal_BitAddressOffset 0
#define GUSB2PHYCFG_TOutCal_RegisterSize 3



#define GUSB2PHYCFG_PHYIF_BitAddressOffset 3
#define GUSB2PHYCFG_PHYIF_RegisterSize 1



#define GUSB2PHYCFG_ULPI_UTMI_Sel_BitAddressOffset 4
#define GUSB2PHYCFG_ULPI_UTMI_Sel_RegisterSize 1



#define GUSB2PHYCFG_FSINTF_BitAddressOffset 5
#define GUSB2PHYCFG_FSINTF_RegisterSize 1



#define GUSB2PHYCFG_SUSPENDUSB20_BitAddressOffset 6
#define GUSB2PHYCFG_SUSPENDUSB20_RegisterSize 1



#define GUSB2PHYCFG_PHYSEL_BitAddressOffset 7
#define GUSB2PHYCFG_PHYSEL_RegisterSize 1



#define GUSB2PHYCFG_ENBLSLPM_BitAddressOffset 8
#define GUSB2PHYCFG_ENBLSLPM_RegisterSize 1



#define GUSB2PHYCFG_XCVRDLY_BitAddressOffset 9
#define GUSB2PHYCFG_XCVRDLY_RegisterSize 1



#define GUSB2PHYCFG_USBTRDTIM_BitAddressOffset 10
#define GUSB2PHYCFG_USBTRDTIM_RegisterSize 4



#define GUSB2PHYCFG_eUSB2OPMODE_BitAddressOffset 14
#define GUSB2PHYCFG_eUSB2OPMODE_RegisterSize 1



#define GUSB2PHYCFG_ULPIAUTORES_BitAddressOffset 15
#define GUSB2PHYCFG_ULPIAUTORES_RegisterSize 1



#define GUSB2PHYCFG_reserved_16_BitAddressOffset 16
#define GUSB2PHYCFG_reserved_16_RegisterSize 1



#define GUSB2PHYCFG_ULPIEXTVBUSDRV_BitAddressOffset 17
#define GUSB2PHYCFG_ULPIEXTVBUSDRV_RegisterSize 1



#define GUSB2PHYCFG_ULPIEXTVBUSINDIACTOR_BitAddressOffset 18
#define GUSB2PHYCFG_ULPIEXTVBUSINDIACTOR_RegisterSize 1



#define GUSB2PHYCFG_LSIPD_BitAddressOffset 19
#define GUSB2PHYCFG_LSIPD_RegisterSize 3



#define GUSB2PHYCFG_LSTRD_BitAddressOffset 22
#define GUSB2PHYCFG_LSTRD_RegisterSize 3



#define GUSB2PHYCFG_OVRD_FSLS_DISC_TIME_BitAddressOffset 25
#define GUSB2PHYCFG_OVRD_FSLS_DISC_TIME_RegisterSize 1



#define GUSB2PHYCFG_INV_SEL_HSIC_BitAddressOffset 26
#define GUSB2PHYCFG_INV_SEL_HSIC_RegisterSize 1



#define GUSB2PHYCFG_HSIC_CON_WIDTH_ADJ_BitAddressOffset 27
#define GUSB2PHYCFG_HSIC_CON_WIDTH_ADJ_RegisterSize 2



#define GUSB2PHYCFG_ULPI_LPM_WITH_OPMODE_CHK_BitAddressOffset 29
#define GUSB2PHYCFG_ULPI_LPM_WITH_OPMODE_CHK_RegisterSize 1



#define GUSB2PHYCFG_reserved_30_BitAddressOffset 30
#define GUSB2PHYCFG_reserved_30_RegisterSize 1



#define GUSB2PHYCFG_PHYSOFTRST_BitAddressOffset 31
#define GUSB2PHYCFG_PHYSOFTRST_RegisterSize 1





#define GUSB2PHYCFG_0 (DWC_usb31_block_gbl_BaseAddress + 0x100)
#define GUSB2PHYCFG_RegisterSize 32
#define GUSB2PHYCFG_RegisterResetValue 0x2102400
#define GUSB2PHYCFG_RegisterResetMask 0xbffeffff





#define GUSB2PHYCFG_TOutCal_BitAddressOffset 0
#define GUSB2PHYCFG_TOutCal_RegisterSize 3



#define GUSB2PHYCFG_PHYIF_BitAddressOffset 3
#define GUSB2PHYCFG_PHYIF_RegisterSize 1



#define GUSB2PHYCFG_ULPI_UTMI_Sel_BitAddressOffset 4
#define GUSB2PHYCFG_ULPI_UTMI_Sel_RegisterSize 1



#define GUSB2PHYCFG_FSINTF_BitAddressOffset 5
#define GUSB2PHYCFG_FSINTF_RegisterSize 1



#define GUSB2PHYCFG_SUSPENDUSB20_BitAddressOffset 6
#define GUSB2PHYCFG_SUSPENDUSB20_RegisterSize 1



#define GUSB2PHYCFG_PHYSEL_BitAddressOffset 7
#define GUSB2PHYCFG_PHYSEL_RegisterSize 1



#define GUSB2PHYCFG_ENBLSLPM_BitAddressOffset 8
#define GUSB2PHYCFG_ENBLSLPM_RegisterSize 1



#define GUSB2PHYCFG_XCVRDLY_BitAddressOffset 9
#define GUSB2PHYCFG_XCVRDLY_RegisterSize 1



#define GUSB2PHYCFG_USBTRDTIM_BitAddressOffset 10
#define GUSB2PHYCFG_USBTRDTIM_RegisterSize 4



#define GUSB2PHYCFG_eUSB2OPMODE_BitAddressOffset 14
#define GUSB2PHYCFG_eUSB2OPMODE_RegisterSize 1



#define GUSB2PHYCFG_ULPIAUTORES_BitAddressOffset 15
#define GUSB2PHYCFG_ULPIAUTORES_RegisterSize 1



#define GUSB2PHYCFG_reserved_16_BitAddressOffset 16
#define GUSB2PHYCFG_reserved_16_RegisterSize 1



#define GUSB2PHYCFG_ULPIEXTVBUSDRV_BitAddressOffset 17
#define GUSB2PHYCFG_ULPIEXTVBUSDRV_RegisterSize 1



#define GUSB2PHYCFG_ULPIEXTVBUSINDIACTOR_BitAddressOffset 18
#define GUSB2PHYCFG_ULPIEXTVBUSINDIACTOR_RegisterSize 1



#define GUSB2PHYCFG_LSIPD_BitAddressOffset 19
#define GUSB2PHYCFG_LSIPD_RegisterSize 3



#define GUSB2PHYCFG_LSTRD_BitAddressOffset 22
#define GUSB2PHYCFG_LSTRD_RegisterSize 3



#define GUSB2PHYCFG_OVRD_FSLS_DISC_TIME_BitAddressOffset 25
#define GUSB2PHYCFG_OVRD_FSLS_DISC_TIME_RegisterSize 1



#define GUSB2PHYCFG_INV_SEL_HSIC_BitAddressOffset 26
#define GUSB2PHYCFG_INV_SEL_HSIC_RegisterSize 1



#define GUSB2PHYCFG_HSIC_CON_WIDTH_ADJ_BitAddressOffset 27
#define GUSB2PHYCFG_HSIC_CON_WIDTH_ADJ_RegisterSize 2



#define GUSB2PHYCFG_ULPI_LPM_WITH_OPMODE_CHK_BitAddressOffset 29
#define GUSB2PHYCFG_ULPI_LPM_WITH_OPMODE_CHK_RegisterSize 1



#define GUSB2PHYCFG_reserved_30_BitAddressOffset 30
#define GUSB2PHYCFG_reserved_30_RegisterSize 1



#define GUSB2PHYCFG_PHYSOFTRST_BitAddressOffset 31
#define GUSB2PHYCFG_PHYSOFTRST_RegisterSize 1





#define GUSB3PIPECTL_REGS (DWC_usb31_block_gbl_BaseAddress + 0x1c0)
#define GUSB3PIPECTL_RegisterSize 32
#define GUSB3PIPECTL_RegisterResetValue 0x1021102
#define GUSB3PIPECTL_RegisterResetMask 0xffffffff





#define GUSB3PIPECTL_ELASTIC_BUFFER_MODE_BitAddressOffset 0
#define GUSB3PIPECTL_ELASTIC_BUFFER_MODE_RegisterSize 1



#define GUSB3PIPECTL_SS_TX_DE_EMPHASIS_BitAddressOffset 1
#define GUSB3PIPECTL_SS_TX_DE_EMPHASIS_RegisterSize 2



#define GUSB3PIPECTL_TX_MARGIN_BitAddressOffset 3
#define GUSB3PIPECTL_TX_MARGIN_RegisterSize 3



#define GUSB3PIPECTL_TX_SWING_BitAddressOffset 6
#define GUSB3PIPECTL_TX_SWING_RegisterSize 1



#define GUSB3PIPECTL_reserved_7_BitAddressOffset 7
#define GUSB3PIPECTL_reserved_7_RegisterSize 1



#define GUSB3PIPECTL_RX_DETECT_to_Polling_LFPS_Control_BitAddressOffset 8
#define GUSB3PIPECTL_RX_DETECT_to_Polling_LFPS_Control_RegisterSize 1



#define GUSB3PIPECTL_LFPSFILTER_BitAddressOffset 9
#define GUSB3PIPECTL_LFPSFILTER_RegisterSize 1



#define GUSB3PIPECTL_P3ExSigP2_BitAddressOffset 10
#define GUSB3PIPECTL_P3ExSigP2_RegisterSize 1



#define GUSB3PIPECTL_P3P2TranOK_BitAddressOffset 11
#define GUSB3PIPECTL_P3P2TranOK_RegisterSize 1



#define GUSB3PIPECTL_LFPSP0Algn_BitAddressOffset 12
#define GUSB3PIPECTL_LFPSP0Algn_RegisterSize 1



#define GUSB3PIPECTL_SkipRxDet_BitAddressOffset 13
#define GUSB3PIPECTL_SkipRxDet_RegisterSize 1



#define GUSB3PIPECTL_AbortRxDetInU2_BitAddressOffset 14
#define GUSB3PIPECTL_AbortRxDetInU2_RegisterSize 1



#define GUSB3PIPECTL_DATWIDTH_BitAddressOffset 15
#define GUSB3PIPECTL_DATWIDTH_RegisterSize 2



#define GUSB3PIPECTL_SUSPENDENABLE_BitAddressOffset 17
#define GUSB3PIPECTL_SUSPENDENABLE_RegisterSize 1



#define GUSB3PIPECTL_DELAYP1TRANS_BitAddressOffset 18
#define GUSB3PIPECTL_DELAYP1TRANS_RegisterSize 1



#define GUSB3PIPECTL_DelayP1P2P3_BitAddressOffset 19
#define GUSB3PIPECTL_DelayP1P2P3_RegisterSize 3



#define GUSB3PIPECTL_DisRxDetU3RxDet_BitAddressOffset 22
#define GUSB3PIPECTL_DisRxDetU3RxDet_RegisterSize 1



#define GUSB3PIPECTL_StartRxDetU3RxDet_BitAddressOffset 23
#define GUSB3PIPECTL_StartRxDetU3RxDet_RegisterSize 1



#define GUSB3PIPECTL_request_p1p2p3_BitAddressOffset 24
#define GUSB3PIPECTL_request_p1p2p3_RegisterSize 1



#define GUSB3PIPECTL_u1u2exitfail_to_recov_BitAddressOffset 25
#define GUSB3PIPECTL_u1u2exitfail_to_recov_RegisterSize 1



#define GUSB3PIPECTL_ping_enhancement_en_BitAddressOffset 26
#define GUSB3PIPECTL_ping_enhancement_en_RegisterSize 1



#define GUSB3PIPECTL_Ux_exit_in_Px_BitAddressOffset 27
#define GUSB3PIPECTL_Ux_exit_in_Px_RegisterSize 1



#define GUSB3PIPECTL_DisRxDetP3_BitAddressOffset 28
#define GUSB3PIPECTL_DisRxDetP3_RegisterSize 1



#define GUSB3PIPECTL_U2P3ok_BitAddressOffset 29
#define GUSB3PIPECTL_U2P3ok_RegisterSize 1



#define GUSB3PIPECTL_HstPrtCmpl_BitAddressOffset 30
#define GUSB3PIPECTL_HstPrtCmpl_RegisterSize 1



#define GUSB3PIPECTL_PHYSoftRst_BitAddressOffset 31
#define GUSB3PIPECTL_PHYSoftRst_RegisterSize 1





#define GUSB3PIPECTL_0 (DWC_usb31_block_gbl_BaseAddress + 0x1c0)
#define GUSB3PIPECTL_RegisterSize 32
#define GUSB3PIPECTL_RegisterResetValue 0x1021102
#define GUSB3PIPECTL_RegisterResetMask 0xffffffff





#define GUSB3PIPECTL_ELASTIC_BUFFER_MODE_BitAddressOffset 0
#define GUSB3PIPECTL_ELASTIC_BUFFER_MODE_RegisterSize 1



#define GUSB3PIPECTL_SS_TX_DE_EMPHASIS_BitAddressOffset 1
#define GUSB3PIPECTL_SS_TX_DE_EMPHASIS_RegisterSize 2



#define GUSB3PIPECTL_TX_MARGIN_BitAddressOffset 3
#define GUSB3PIPECTL_TX_MARGIN_RegisterSize 3



#define GUSB3PIPECTL_TX_SWING_BitAddressOffset 6
#define GUSB3PIPECTL_TX_SWING_RegisterSize 1



#define GUSB3PIPECTL_reserved_7_BitAddressOffset 7
#define GUSB3PIPECTL_reserved_7_RegisterSize 1



#define GUSB3PIPECTL_RX_DETECT_to_Polling_LFPS_Control_BitAddressOffset 8
#define GUSB3PIPECTL_RX_DETECT_to_Polling_LFPS_Control_RegisterSize 1



#define GUSB3PIPECTL_LFPSFILTER_BitAddressOffset 9
#define GUSB3PIPECTL_LFPSFILTER_RegisterSize 1



#define GUSB3PIPECTL_P3ExSigP2_BitAddressOffset 10
#define GUSB3PIPECTL_P3ExSigP2_RegisterSize 1



#define GUSB3PIPECTL_P3P2TranOK_BitAddressOffset 11
#define GUSB3PIPECTL_P3P2TranOK_RegisterSize 1



#define GUSB3PIPECTL_LFPSP0Algn_BitAddressOffset 12
#define GUSB3PIPECTL_LFPSP0Algn_RegisterSize 1



#define GUSB3PIPECTL_SkipRxDet_BitAddressOffset 13
#define GUSB3PIPECTL_SkipRxDet_RegisterSize 1



#define GUSB3PIPECTL_AbortRxDetInU2_BitAddressOffset 14
#define GUSB3PIPECTL_AbortRxDetInU2_RegisterSize 1



#define GUSB3PIPECTL_DATWIDTH_BitAddressOffset 15
#define GUSB3PIPECTL_DATWIDTH_RegisterSize 2



#define GUSB3PIPECTL_SUSPENDENABLE_BitAddressOffset 17
#define GUSB3PIPECTL_SUSPENDENABLE_RegisterSize 1



#define GUSB3PIPECTL_DELAYP1TRANS_BitAddressOffset 18
#define GUSB3PIPECTL_DELAYP1TRANS_RegisterSize 1



#define GUSB3PIPECTL_DelayP1P2P3_BitAddressOffset 19
#define GUSB3PIPECTL_DelayP1P2P3_RegisterSize 3



#define GUSB3PIPECTL_DisRxDetU3RxDet_BitAddressOffset 22
#define GUSB3PIPECTL_DisRxDetU3RxDet_RegisterSize 1



#define GUSB3PIPECTL_StartRxDetU3RxDet_BitAddressOffset 23
#define GUSB3PIPECTL_StartRxDetU3RxDet_RegisterSize 1



#define GUSB3PIPECTL_request_p1p2p3_BitAddressOffset 24
#define GUSB3PIPECTL_request_p1p2p3_RegisterSize 1



#define GUSB3PIPECTL_u1u2exitfail_to_recov_BitAddressOffset 25
#define GUSB3PIPECTL_u1u2exitfail_to_recov_RegisterSize 1



#define GUSB3PIPECTL_ping_enhancement_en_BitAddressOffset 26
#define GUSB3PIPECTL_ping_enhancement_en_RegisterSize 1



#define GUSB3PIPECTL_Ux_exit_in_Px_BitAddressOffset 27
#define GUSB3PIPECTL_Ux_exit_in_Px_RegisterSize 1



#define GUSB3PIPECTL_DisRxDetP3_BitAddressOffset 28
#define GUSB3PIPECTL_DisRxDetP3_RegisterSize 1



#define GUSB3PIPECTL_U2P3ok_BitAddressOffset 29
#define GUSB3PIPECTL_U2P3ok_RegisterSize 1



#define GUSB3PIPECTL_HstPrtCmpl_BitAddressOffset 30
#define GUSB3PIPECTL_HstPrtCmpl_RegisterSize 1



#define GUSB3PIPECTL_PHYSoftRst_BitAddressOffset 31
#define GUSB3PIPECTL_PHYSoftRst_RegisterSize 1





#define GTXFIFOSIZ_REGS (DWC_usb31_block_gbl_BaseAddress + 0x200)
#define GTXFIFOSIZ0_RegisterSize 32
#define GTXFIFOSIZ0_RegisterResetValue 0x23
#define GTXFIFOSIZ0_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ0_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ0_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ0_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ0_reserved_15_RegisterSize 1



#define GTXFIFOSIZ0_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ0_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ0_0 (DWC_usb31_block_gbl_BaseAddress + 0x200)
#define GTXFIFOSIZ0_RegisterSize 32
#define GTXFIFOSIZ0_RegisterResetValue 0x23
#define GTXFIFOSIZ0_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ0_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ0_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ0_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ0_reserved_15_RegisterSize 1



#define GTXFIFOSIZ0_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ0_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ1_0 (DWC_usb31_block_gbl_BaseAddress + 0x204)
#define GTXFIFOSIZ1_RegisterSize 32
#define GTXFIFOSIZ1_RegisterResetValue 0x238109
#define GTXFIFOSIZ1_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ1_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ1_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ1_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ1_reserved_15_RegisterSize 1



#define GTXFIFOSIZ1_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ1_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ2_0 (DWC_usb31_block_gbl_BaseAddress + 0x208)
#define GTXFIFOSIZ2_RegisterSize 32
#define GTXFIFOSIZ2_RegisterResetValue 0x12c8109
#define GTXFIFOSIZ2_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ2_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ2_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ2_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ2_reserved_15_RegisterSize 1



#define GTXFIFOSIZ2_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ2_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ3_0 (DWC_usb31_block_gbl_BaseAddress + 0x20c)
#define GTXFIFOSIZ3_RegisterSize 32
#define GTXFIFOSIZ3_RegisterResetValue 0x2358109
#define GTXFIFOSIZ3_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ3_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ3_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ3_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ3_reserved_15_RegisterSize 1



#define GTXFIFOSIZ3_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ3_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ4_0 (DWC_usb31_block_gbl_BaseAddress + 0x210)
#define GTXFIFOSIZ4_RegisterSize 32
#define GTXFIFOSIZ4_RegisterResetValue 0x33e8109
#define GTXFIFOSIZ4_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ4_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ4_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ4_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ4_reserved_15_RegisterSize 1



#define GTXFIFOSIZ4_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ4_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ5_0 (DWC_usb31_block_gbl_BaseAddress + 0x214)
#define GTXFIFOSIZ5_RegisterSize 32
#define GTXFIFOSIZ5_RegisterResetValue 0x4478109
#define GTXFIFOSIZ5_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ5_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ5_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ5_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ5_reserved_15_RegisterSize 1



#define GTXFIFOSIZ5_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ5_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ6_0 (DWC_usb31_block_gbl_BaseAddress + 0x218)
#define GTXFIFOSIZ6_RegisterSize 32
#define GTXFIFOSIZ6_RegisterResetValue 0x5508085
#define GTXFIFOSIZ6_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ6_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ6_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ6_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ6_reserved_15_RegisterSize 1



#define GTXFIFOSIZ6_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ6_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ7_0 (DWC_usb31_block_gbl_BaseAddress + 0x21c)
#define GTXFIFOSIZ7_RegisterSize 32
#define GTXFIFOSIZ7_RegisterResetValue 0x5d58085
#define GTXFIFOSIZ7_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ7_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ7_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ7_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ7_reserved_15_RegisterSize 1



#define GTXFIFOSIZ7_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ7_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ8_0 (DWC_usb31_block_gbl_BaseAddress + 0x220)
#define GTXFIFOSIZ8_RegisterSize 32
#define GTXFIFOSIZ8_RegisterResetValue 0x65a8045
#define GTXFIFOSIZ8_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ8_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ8_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ8_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ8_reserved_15_RegisterSize 1



#define GTXFIFOSIZ8_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ8_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ9_0 (DWC_usb31_block_gbl_BaseAddress + 0x224)
#define GTXFIFOSIZ9_RegisterSize 32
#define GTXFIFOSIZ9_RegisterResetValue 0x69f8045
#define GTXFIFOSIZ9_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ9_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ9_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ9_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ9_reserved_15_RegisterSize 1



#define GTXFIFOSIZ9_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ9_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ10_0 (DWC_usb31_block_gbl_BaseAddress + 0x228)
#define GTXFIFOSIZ10_RegisterSize 32
#define GTXFIFOSIZ10_RegisterResetValue 0x6e48045
#define GTXFIFOSIZ10_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ10_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ10_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ10_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ10_reserved_15_RegisterSize 1



#define GTXFIFOSIZ10_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ10_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ11_0 (DWC_usb31_block_gbl_BaseAddress + 0x22c)
#define GTXFIFOSIZ11_RegisterSize 32
#define GTXFIFOSIZ11_RegisterResetValue 0x7298045
#define GTXFIFOSIZ11_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ11_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ11_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ11_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ11_reserved_15_RegisterSize 1



#define GTXFIFOSIZ11_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ11_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ12_0 (DWC_usb31_block_gbl_BaseAddress + 0x230)
#define GTXFIFOSIZ12_RegisterSize 32
#define GTXFIFOSIZ12_RegisterResetValue 0x76e8045
#define GTXFIFOSIZ12_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ12_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ12_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ12_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ12_reserved_15_RegisterSize 1



#define GTXFIFOSIZ12_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ12_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ13_0 (DWC_usb31_block_gbl_BaseAddress + 0x234)
#define GTXFIFOSIZ13_RegisterSize 32
#define GTXFIFOSIZ13_RegisterResetValue 0x7b38045
#define GTXFIFOSIZ13_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ13_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ13_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ13_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ13_reserved_15_RegisterSize 1



#define GTXFIFOSIZ13_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ13_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ14_0 (DWC_usb31_block_gbl_BaseAddress + 0x238)
#define GTXFIFOSIZ14_RegisterSize 32
#define GTXFIFOSIZ14_RegisterResetValue 0x7f88045
#define GTXFIFOSIZ14_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ14_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ14_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ14_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ14_reserved_15_RegisterSize 1



#define GTXFIFOSIZ14_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ14_TXFSTADDR_N_RegisterSize 16





#define GTXFIFOSIZ15_0 (DWC_usb31_block_gbl_BaseAddress + 0x23c)
#define GTXFIFOSIZ15_RegisterSize 32
#define GTXFIFOSIZ15_RegisterResetValue 0x83d8045
#define GTXFIFOSIZ15_RegisterResetMask 0xffff7fff





#define GTXFIFOSIZ15_TXFDEP_N_BitAddressOffset 0
#define GTXFIFOSIZ15_TXFDEP_N_RegisterSize 15



#define GTXFIFOSIZ15_reserved_15_BitAddressOffset 15
#define GTXFIFOSIZ15_reserved_15_RegisterSize 1



#define GTXFIFOSIZ15_TXFSTADDR_N_BitAddressOffset 16
#define GTXFIFOSIZ15_TXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ_REGS (DWC_usb31_block_gbl_BaseAddress + 0x280)
#define GRXFIFOSIZ0_RegisterSize 32
#define GRXFIFOSIZ0_RegisterResetValue 0x109
#define GRXFIFOSIZ0_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ0_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ0_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ0_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ0_reserved_15_RegisterSize 1



#define GRXFIFOSIZ0_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ0_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ0_0 (DWC_usb31_block_gbl_BaseAddress + 0x280)
#define GRXFIFOSIZ0_RegisterSize 32
#define GRXFIFOSIZ0_RegisterResetValue 0x109
#define GRXFIFOSIZ0_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ0_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ0_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ0_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ0_reserved_15_RegisterSize 1



#define GRXFIFOSIZ0_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ0_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ1_0 (DWC_usb31_block_gbl_BaseAddress + 0x284)
#define GRXFIFOSIZ1_RegisterSize 32
#define GRXFIFOSIZ1_RegisterResetValue 0x1098109
#define GRXFIFOSIZ1_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ1_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ1_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ1_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ1_reserved_15_RegisterSize 1



#define GRXFIFOSIZ1_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ1_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ2_0 (DWC_usb31_block_gbl_BaseAddress + 0x288)
#define GRXFIFOSIZ2_RegisterSize 32
#define GRXFIFOSIZ2_RegisterResetValue 0x2120000
#define GRXFIFOSIZ2_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ2_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ2_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ2_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ2_reserved_15_RegisterSize 1



#define GRXFIFOSIZ2_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ2_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ3_0 (DWC_usb31_block_gbl_BaseAddress + 0x28c)
#define GRXFIFOSIZ3_RegisterSize 32
#define GRXFIFOSIZ3_RegisterResetValue 0x2128000
#define GRXFIFOSIZ3_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ3_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ3_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ3_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ3_reserved_15_RegisterSize 1



#define GRXFIFOSIZ3_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ3_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ4_0 (DWC_usb31_block_gbl_BaseAddress + 0x290)
#define GRXFIFOSIZ4_RegisterSize 32
#define GRXFIFOSIZ4_RegisterResetValue 0x2120000
#define GRXFIFOSIZ4_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ4_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ4_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ4_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ4_reserved_15_RegisterSize 1



#define GRXFIFOSIZ4_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ4_RXFSTADDR_N_RegisterSize 16





#define GRXFIFOSIZ5_0 (DWC_usb31_block_gbl_BaseAddress + 0x294)
#define GRXFIFOSIZ5_RegisterSize 32
#define GRXFIFOSIZ5_RegisterResetValue 0x2128000
#define GRXFIFOSIZ5_RegisterResetMask 0xffffffff





#define GRXFIFOSIZ5_RXFDEP_N_BitAddressOffset 0
#define GRXFIFOSIZ5_RXFDEP_N_RegisterSize 15



#define GRXFIFOSIZ5_reserved_15_BitAddressOffset 15
#define GRXFIFOSIZ5_reserved_15_RegisterSize 1



#define GRXFIFOSIZ5_RXFSTADDR_N_BitAddressOffset 16
#define GRXFIFOSIZ5_RXFSTADDR_N_RegisterSize 16





#define GEVNTADRLO_REGS (DWC_usb31_block_gbl_BaseAddress + 0x300)
#define GEVNTADRLO_RegisterSize 32
#define GEVNTADRLO_RegisterResetValue 0x0
#define GEVNTADRLO_RegisterResetMask 0x0





#define GEVNTADRLO_EVNTADRLO_BitAddressOffset 0
#define GEVNTADRLO_EVNTADRLO_RegisterSize 32





#define GEVNTADRLO_0 (DWC_usb31_block_gbl_BaseAddress + 0x300)
#define GEVNTADRLO_RegisterSize 32
#define GEVNTADRLO_RegisterResetValue 0x0
#define GEVNTADRLO_RegisterResetMask 0x0





#define GEVNTADRLO_EVNTADRLO_BitAddressOffset 0
#define GEVNTADRLO_EVNTADRLO_RegisterSize 32





#define GEVNTADRHI_0 (DWC_usb31_block_gbl_BaseAddress + 0x304)
#define GEVNTADRHI_RegisterSize 32
#define GEVNTADRHI_RegisterResetValue 0x0
#define GEVNTADRHI_RegisterResetMask 0x0





#define GEVNTADRHI_EVNTADRHI_BitAddressOffset 0
#define GEVNTADRHI_EVNTADRHI_RegisterSize 32





#define GEVNTSIZ_0 (DWC_usb31_block_gbl_BaseAddress + 0x308)
#define GEVNTSIZ_RegisterSize 32
#define GEVNTSIZ_RegisterResetValue 0x0
#define GEVNTSIZ_RegisterResetMask 0x8000ffff





#define GEVNTSIZ_EVENTSIZ_BitAddressOffset 0
#define GEVNTSIZ_EVENTSIZ_RegisterSize 16



#define GEVNTSIZ_reserved_30_16_BitAddressOffset 16
#define GEVNTSIZ_reserved_30_16_RegisterSize 15



#define GEVNTSIZ_EVNTINTRPTMASK_BitAddressOffset 31
#define GEVNTSIZ_EVNTINTRPTMASK_RegisterSize 1





#define GEVNTCOUNT_0 (DWC_usb31_block_gbl_BaseAddress + 0x30c)
#define GEVNTCOUNT_RegisterSize 32
#define GEVNTCOUNT_RegisterResetValue 0x0
#define GEVNTCOUNT_RegisterResetMask 0x0





#define GEVNTCOUNT_EVNTCOUNT_BitAddressOffset 0
#define GEVNTCOUNT_EVNTCOUNT_RegisterSize 16



#define GEVNTCOUNT_reserved_30_16_BitAddressOffset 16
#define GEVNTCOUNT_reserved_30_16_RegisterSize 15



#define GEVNTCOUNT_EVNT_HANDLER_BUSY_BitAddressOffset 31
#define GEVNTCOUNT_EVNT_HANDLER_BUSY_RegisterSize 1





#define GHWPARAMS8 (DWC_usb31_block_gbl_BaseAddress + 0x500)
#define GHWPARAMS8_RegisterSize 32
#define GHWPARAMS8_RegisterResetValue 0xfdc
#define GHWPARAMS8_RegisterResetMask 0xffffffff





#define GHWPARAMS8_ghwparams8_31_0_BitAddressOffset 0
#define GHWPARAMS8_ghwparams8_31_0_RegisterSize 32





#define GSMACCTL (DWC_usb31_block_gbl_BaseAddress + 0x504)
#define GSMACCTL_RegisterSize 32
#define GSMACCTL_RegisterResetValue 0x700
#define GSMACCTL_RegisterResetMask 0xffffffff





#define GSMACCTL_single_TBT_req_BitAddressOffset 0
#define GSMACCTL_single_TBT_req_RegisterSize 1



#define GSMACCTL_ignore_babble_BitAddressOffset 1
#define GSMACCTL_ignore_babble_RegisterSize 1



#define GSMACCTL_host_mask_nump0_BitAddressOffset 2
#define GSMACCTL_host_mask_nump0_RegisterSize 1



#define GSMACCTL_single_psqdir_en_BitAddressOffset 3
#define GSMACCTL_single_psqdir_en_RegisterSize 1



#define GSMACCTL_ISOC_TIMEOUT_BitAddressOffset 4
#define GSMACCTL_ISOC_TIMEOUT_RegisterSize 8



#define GSMACCTL_ISOCIN_TOUTEN_BitAddressOffset 12
#define GSMACCTL_ISOCIN_TOUTEN_RegisterSize 1



#define GSMACCTL_reserved_15_13_BitAddressOffset 13
#define GSMACCTL_reserved_15_13_RegisterSize 3



#define GSMACCTL_hostout_single_en_BitAddressOffset 16
#define GSMACCTL_hostout_single_en_RegisterSize 1



#define GSMACCTL_hostin_signle_en_BitAddressOffset 17
#define GSMACCTL_hostin_signle_en_RegisterSize 1



#define GSMACCTL_reserved_18_BitAddressOffset 18
#define GSMACCTL_reserved_18_RegisterSize 1



#define GSMACCTL_dev_stallresp_ctrl_BitAddressOffset 19
#define GSMACCTL_dev_stallresp_ctrl_RegisterSize 1



#define GSMACCTL_retry_sts_non_reco_state_BitAddressOffset 20
#define GSMACCTL_retry_sts_non_reco_state_RegisterSize 1



#define GSMACCTL_reserved_25_21_BitAddressOffset 21
#define GSMACCTL_reserved_25_21_RegisterSize 5



#define GSMACCTL_dbg_ptl_addr_BitAddressOffset 26
#define GSMACCTL_dbg_ptl_addr_RegisterSize 4



#define GSMACCTL_dbg_tis_index_BitAddressOffset 30
#define GSMACCTL_dbg_tis_index_RegisterSize 2





#define GUCTL2 (DWC_usb31_block_gbl_BaseAddress + 0x508)
#define GUCTL2_RegisterSize 32
#define GUCTL2_RegisterResetValue 0x18044246
#define GUCTL2_RegisterResetMask 0xffffffff





#define GUCTL2_SCHEDDULE_LT_THR_BitAddressOffset 0
#define GUCTL2_SCHEDDULE_LT_THR_RegisterSize 1



#define GUCTL2_MAX_REISU_CNT_BitAddressOffset 1
#define GUCTL2_MAX_REISU_CNT_RegisterSize 2



#define GUCTL2_SVC_OPP_PER_FSLS_BitAddressOffset 3
#define GUCTL2_SVC_OPP_PER_FSLS_RegisterSize 2



#define GUCTL2_SVC_OPP_PER_HS_BitAddressOffset 5
#define GUCTL2_SVC_OPP_PER_HS_RegisterSize 2



#define GUCTL2_SVC_OPP_PER_SS_BitAddressOffset 7
#define GUCTL2_SVC_OPP_PER_SS_RegisterSize 2



#define GUCTL2_SVC_OPP_PER_SSP_BitAddressOffset 9
#define GUCTL2_SVC_OPP_PER_SSP_RegisterSize 2



#define GUCTL2_PERIODIC_TXDMA_UF_THR_BitAddressOffset 11
#define GUCTL2_PERIODIC_TXDMA_UF_THR_RegisterSize 7



#define GUCTL2_ASYNC_UF_THR_BitAddressOffset 18
#define GUCTL2_ASYNC_UF_THR_RegisterSize 7



#define GUCTL2_PERIODIC_UF_THR_BitAddressOffset 25
#define GUCTL2_PERIODIC_UF_THR_RegisterSize 7





#define GUCTL3 (DWC_usb31_block_gbl_BaseAddress + 0x50c)
#define GUCTL3_RegisterSize 32
#define GUCTL3_RegisterResetValue 0x85560c
#define GUCTL3_RegisterResetMask 0xffffffff





#define GUCTL3_PERIODIC_UF_THR_FSLS_BitAddressOffset 0
#define GUCTL3_PERIODIC_UF_THR_FSLS_RegisterSize 7



#define GUCTL3_INTR_EP_PING_ENABLE_BitAddressOffset 7
#define GUCTL3_INTR_EP_PING_ENABLE_RegisterSize 1



#define GUCTL3_BARB_BURST_ENABLE_BitAddressOffset 8
#define GUCTL3_BARB_BURST_ENABLE_RegisterSize 1



#define GUCTL3_SVC_OPP_PER_HS_SEP_BitAddressOffset 9
#define GUCTL3_SVC_OPP_PER_HS_SEP_RegisterSize 4



#define GUCTL3_reserved_13_BitAddressOffset 13
#define GUCTL3_reserved_13_RegisterSize 1



#define GUCTL3_reserved_14_BitAddressOffset 14
#define GUCTL3_reserved_14_RegisterSize 1



#define GUCTL3_DISABLE_VTIO_CAPABILITY_BitAddressOffset 15
#define GUCTL3_DISABLE_VTIO_CAPABILITY_RegisterSize 1



#define GUCTL3_USB20_RETRY_DISABLE_BitAddressOffset 16
#define GUCTL3_USB20_RETRY_DISABLE_RegisterSize 1



#define GUCTL3_RSVD_PERI_BANDWIDTH_FS_BitAddressOffset 17
#define GUCTL3_RSVD_PERI_BANDWIDTH_FS_RegisterSize 1



#define GUCTL3_DISEXTBUSCLKGT_BitAddressOffset 18
#define GUCTL3_DISEXTBUSCLKGT_RegisterSize 1



#define GUCTL3_DISEXTBUSCLKGT_U1U2L1_BitAddressOffset 19
#define GUCTL3_DISEXTBUSCLKGT_U1U2L1_RegisterSize 1



#define GUCTL3_SSBI_SINGLE_EP_MODE_DISABLE_BitAddressOffset 20
#define GUCTL3_SSBI_SINGLE_EP_MODE_DISABLE_RegisterSize 1



#define GUCTL3_BLOCK_CONCURRENT_IN_CTRL_XFERS_BitAddressOffset 21
#define GUCTL3_BLOCK_CONCURRENT_IN_CTRL_XFERS_RegisterSize 1



#define GUCTL3_CFGEP_CMD_SWITCHING_TIME_CTRL_BitAddressOffset 22
#define GUCTL3_CFGEP_CMD_SWITCHING_TIME_CTRL_RegisterSize 3



#define GUCTL3_reserved_29_22_BitAddressOffset 25
#define GUCTL3_reserved_29_22_RegisterSize 5



#define GUCTL3_reserved_31_BitAddressOffset 31
#define GUCTL3_reserved_31_RegisterSize 1





#define GTXFIFOPRIDEV (DWC_usb31_block_gbl_BaseAddress + 0x510)
#define GTXFIFOPRIDEV_RegisterSize 32
#define GTXFIFOPRIDEV_RegisterResetValue 0x0
#define GTXFIFOPRIDEV_RegisterResetMask 0xf





#define GTXFIFOPRIDEV_gtxfifopridev_BitAddressOffset 0
#define GTXFIFOPRIDEV_gtxfifopridev_RegisterSize 16



#define GTXFIFOPRIDEV_reserved_31_n_BitAddressOffset 16
#define GTXFIFOPRIDEV_reserved_31_n_RegisterSize 16





#define GTXFIFOPRIHST (DWC_usb31_block_gbl_BaseAddress + 0x518)
#define GTXFIFOPRIHST_RegisterSize 32
#define GTXFIFOPRIHST_RegisterResetValue 0x0
#define GTXFIFOPRIHST_RegisterResetMask 0x3f





#define GTXFIFOPRIHST_gtxfifoprihst_BitAddressOffset 0
#define GTXFIFOPRIHST_gtxfifoprihst_RegisterSize 6



#define GTXFIFOPRIHST_reserved_31_y_BitAddressOffset 6
#define GTXFIFOPRIHST_reserved_31_y_RegisterSize 26





#define GRXFIFOPRIHST (DWC_usb31_block_gbl_BaseAddress + 0x51c)
#define GRXFIFOPRIHST_RegisterSize 32
#define GRXFIFOPRIHST_RegisterResetValue 0x0
#define GRXFIFOPRIHST_RegisterResetMask 0x3f





#define GRXFIFOPRIHST_grxfifoprihst_BitAddressOffset 0
#define GRXFIFOPRIHST_grxfifoprihst_RegisterSize 6



#define GRXFIFOPRIHST_reserved_31_y_BitAddressOffset 6
#define GRXFIFOPRIHST_reserved_31_y_RegisterSize 26





#define GDMAHLRATIO (DWC_usb31_block_gbl_BaseAddress + 0x524)
#define GDMAHLRATIO_RegisterSize 32
#define GDMAHLRATIO_RegisterResetValue 0xa0a0101
#define GDMAHLRATIO_RegisterResetMask 0x1f1f1f1f





#define GDMAHLRATIO_hsttxfifo_dma_BitAddressOffset 0
#define GDMAHLRATIO_hsttxfifo_dma_RegisterSize 5



#define GDMAHLRATIO_reserved_7_5_BitAddressOffset 5
#define GDMAHLRATIO_reserved_7_5_RegisterSize 3



#define GDMAHLRATIO_hstrxfifo_dma_BitAddressOffset 8
#define GDMAHLRATIO_hstrxfifo_dma_RegisterSize 5



#define GDMAHLRATIO_reserved_15_13_BitAddressOffset 13
#define GDMAHLRATIO_reserved_15_13_RegisterSize 3



#define GDMAHLRATIO_hsttxfifo_mac_BitAddressOffset 16
#define GDMAHLRATIO_hsttxfifo_mac_RegisterSize 5



#define GDMAHLRATIO_reserved_23_21_BitAddressOffset 21
#define GDMAHLRATIO_reserved_23_21_RegisterSize 3



#define GDMAHLRATIO_hstrxfifo_mac_BitAddressOffset 24
#define GDMAHLRATIO_hstrxfifo_mac_RegisterSize 5



#define GDMAHLRATIO_reserved_31_29_BitAddressOffset 29
#define GDMAHLRATIO_reserved_31_29_RegisterSize 3





#define GOSTDDMA_ASYNC (DWC_usb31_block_gbl_BaseAddress + 0x528)
#define GOSTDDMA_ASYNC_RegisterSize 32
#define GOSTDDMA_ASYNC_RegisterResetValue 0x6060406
#define GOSTDDMA_ASYNC_RegisterResetMask 0xffffffff





#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_BitAddressOffset 0
#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_RegisterSize 8



#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_INACTIVE_BitAddressOffset 8
#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_INACTIVE_RegisterSize 8



#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_ACTIVE_BitAddressOffset 16
#define GOSTDDMA_ASYNC_OSTDTX_ASYNC_ACTIVE_RegisterSize 8



#define GOSTDDMA_ASYNC_OSTDRX_ASYNC_BitAddressOffset 24
#define GOSTDDMA_ASYNC_OSTDRX_ASYNC_RegisterSize 8





#define GOSTDDMA_PRD (DWC_usb31_block_gbl_BaseAddress + 0x52c)
#define GOSTDDMA_PRD_RegisterSize 32
#define GOSTDDMA_PRD_RegisterResetValue 0x8080608
#define GOSTDDMA_PRD_RegisterResetMask 0xffffffff





#define GOSTDDMA_PRD_OSTDTX_PRD_BitAddressOffset 0
#define GOSTDDMA_PRD_OSTDTX_PRD_RegisterSize 8



#define GOSTDDMA_PRD_OSTDTX_PRD_INACTIVE_BitAddressOffset 8
#define GOSTDDMA_PRD_OSTDTX_PRD_INACTIVE_RegisterSize 8



#define GOSTDDMA_PRD_OSTDTX_PRD_ACTIVE_BitAddressOffset 16
#define GOSTDDMA_PRD_OSTDTX_PRD_ACTIVE_RegisterSize 8



#define GOSTDDMA_PRD_OSTDRX_PRD_BitAddressOffset 24
#define GOSTDDMA_PRD_OSTDRX_PRD_RegisterSize 8





#define GFLADJ (DWC_usb31_block_gbl_BaseAddress + 0x530)
#define GFLADJ_RegisterSize 32
#define GFLADJ_RegisterResetValue 0x8c80c820
#define GFLADJ_RegisterResetMask 0xffbfffbf





#define GFLADJ_GFLADJ_30MHZ_BitAddressOffset 0
#define GFLADJ_GFLADJ_30MHZ_RegisterSize 6



#define GFLADJ_reserved_6_BitAddressOffset 6
#define GFLADJ_reserved_6_RegisterSize 1



#define GFLADJ_GFLADJ_30MHZ_SDBND_SEL_BitAddressOffset 7
#define GFLADJ_GFLADJ_30MHZ_SDBND_SEL_RegisterSize 1



#define GFLADJ_GFLADJ_REFCLK_FLADJ_BitAddressOffset 8
#define GFLADJ_GFLADJ_REFCLK_FLADJ_RegisterSize 14



#define GFLADJ_reserved_22_BitAddressOffset 22
#define GFLADJ_reserved_22_RegisterSize 1



#define GFLADJ_GFLADJ_REFCLK_LPM_SEL_BitAddressOffset 23
#define GFLADJ_GFLADJ_REFCLK_LPM_SEL_RegisterSize 1



#define GFLADJ_GFLADJ_REFCLK_240MHZ_DECR_BitAddressOffset 24
#define GFLADJ_GFLADJ_REFCLK_240MHZ_DECR_RegisterSize 7



#define GFLADJ_GFLADJ_REFCLK_240MHZDECR_PLS1_BitAddressOffset 31
#define GFLADJ_GFLADJ_REFCLK_240MHZDECR_PLS1_RegisterSize 1





#define GUCTL4 (DWC_usb31_block_gbl_BaseAddress + 0x534)
#define GUCTL4_RegisterSize 32
#define GUCTL4_RegisterResetValue 0x1ffff
#define GUCTL4_RegisterResetMask 0x7fffffff





#define GUCTL4_CSR_TIMEOUT_VL_BitAddressOffset 0
#define GUCTL4_CSR_TIMEOUT_VL_RegisterSize 17



#define GUCTL4_LOA_EOP_CHECK_CLKS_BYTE_BitAddressOffset 17
#define GUCTL4_LOA_EOP_CHECK_CLKS_BYTE_RegisterSize 2



#define GUCTL4_LOA_EOP_CHECK_CLKS_WORD_BitAddressOffset 19
#define GUCTL4_LOA_EOP_CHECK_CLKS_WORD_RegisterSize 2



#define GUCTL4_DIS_STATUS_STG_TRB_DIR_BitAddressOffset 21
#define GUCTL4_DIS_STATUS_STG_TRB_DIR_RegisterSize 1



#define GUCTL4_DIS_ESS_PING_RETRY_LOGIC_BitAddressOffset 22
#define GUCTL4_DIS_ESS_PING_RETRY_LOGIC_RegisterSize 1



#define GUCTL4_reserved_23_BitAddressOffset 23
#define GUCTL4_reserved_23_RegisterSize 1



#define GUCTL4_DIS_NSR_LINK_REG_BitAddressOffset 24
#define GUCTL4_DIS_NSR_LINK_REG_RegisterSize 1



#define GUCTL4_reserved_25_BitAddressOffset 25
#define GUCTL4_reserved_25_RegisterSize 1



#define GUCTL4_SSP_BWD_OVHD_ADJ_BitAddressOffset 26
#define GUCTL4_SSP_BWD_OVHD_ADJ_RegisterSize 5



#define GUCTL4_mac2_clk_sel_BitAddressOffset 31
#define GUCTL4_mac2_clk_sel_RegisterSize 1





#define GUCTL5 (DWC_usb31_block_gbl_BaseAddress + 0x538)
#define GUCTL5_RegisterSize 32
#define GUCTL5_RegisterResetValue 0x0
#define GUCTL5_RegisterResetMask 0x40007f03





#define GUCTL5_csr_disable_bext_pp_BitAddressOffset 0
#define GUCTL5_csr_disable_bext_pp_RegisterSize 1



#define GUCTL5_ExtdMAX_REISU_CNT_BitAddressOffset 1
#define GUCTL5_ExtdMAX_REISU_CNT_RegisterSize 1



#define GUCTL5_reserved_7_2_BitAddressOffset 2
#define GUCTL5_reserved_7_2_RegisterSize 6



#define GUCTL5_HST_DYNAMIC_DISABLE_DIS_BitAddressOffset 8
#define GUCTL5_HST_DYNAMIC_DISABLE_DIS_RegisterSize 1



#define GUCTL5_FORCE_LOCAL_U3_EXIT_D3D0_BitAddressOffset 9
#define GUCTL5_FORCE_LOCAL_U3_EXIT_D3D0_RegisterSize 1



#define GUCTL5_csr_disable_stream_bstext_BitAddressOffset 10
#define GUCTL5_csr_disable_stream_bstext_RegisterSize 1



#define GUCTL5_csr_disable_evcnxt_mse_fix_BitAddressOffset 11
#define GUCTL5_csr_disable_evcnxt_mse_fix_RegisterSize 1



#define GUCTL5_csr_disable_dbc_hib_ehc_fix_BitAddressOffset 12
#define GUCTL5_csr_disable_dbc_hib_ehc_fix_RegisterSize 1



#define GUCTL5_csr_disable_cfgepcmd_timeout_fix_BitAddressOffset 13
#define GUCTL5_csr_disable_cfgepcmd_timeout_fix_RegisterSize 1



#define GUCTL5_csr_disable_cio_mel_fix_BitAddressOffset 14
#define GUCTL5_csr_disable_cio_mel_fix_RegisterSize 1



#define GUCTL5_csr_disable_rst_typea_fix_BitAddressOffset 15
#define GUCTL5_csr_disable_rst_typea_fix_RegisterSize 1



#define GUCTL5_csr_disable_rst_typec_fix_BitAddressOffset 16
#define GUCTL5_csr_disable_rst_typec_fix_RegisterSize 1



#define GUCTL5_csr_disable_halt_fix_BitAddressOffset 17
#define GUCTL5_csr_disable_halt_fix_RegisterSize 1



#define GUCTL5_reserved_26_18_BitAddressOffset 18
#define GUCTL5_reserved_26_18_RegisterSize 9



#define GUCTL5_csr_disable_link_u1_entry_wo_hshk_BitAddressOffset 27
#define GUCTL5_csr_disable_link_u1_entry_wo_hshk_RegisterSize 1



#define GUCTL5_csr_disable_fake_d3_no_typec_ack_BitAddressOffset 28
#define GUCTL5_csr_disable_fake_d3_no_typec_ack_RegisterSize 1



#define GUCTL5_csr_no_incr_ESSInact_u2u3_rxdet_timer_BitAddressOffset 29
#define GUCTL5_csr_no_incr_ESSInact_u2u3_rxdet_timer_RegisterSize 1



#define GUCTL5_reserved_30_BitAddressOffset 30
#define GUCTL5_reserved_30_RegisterSize 1



#define GUCTL5_force_xHCI_hce_BitAddressOffset 31
#define GUCTL5_force_xHCI_hce_RegisterSize 1





#define GUSB2RHBCTL_REGS (DWC_usb31_block_gbl_BaseAddress + 0x540)
#define GUSB2RHBCTL_RegisterSize 32
#define GUSB2RHBCTL_RegisterResetValue 0x0
#define GUSB2RHBCTL_RegisterResetMask 0x3ffffff





#define GUSB2RHBCTL_OVRD_L1TIMEOUT_BitAddressOffset 0
#define GUSB2RHBCTL_OVRD_L1TIMEOUT_RegisterSize 4



#define GUSB2RHBCTL_OVRD_HS_INT_PKT_DEL_BitAddressOffset 4
#define GUSB2RHBCTL_OVRD_HS_INT_PKT_DEL_RegisterSize 8



#define GUSB2RHBCTL_OVRD_FS_INT_PKT_DEL_BitAddressOffset 12
#define GUSB2RHBCTL_OVRD_FS_INT_PKT_DEL_RegisterSize 8



#define GUSB2RHBCTL_BLOCK_U2_CONNECT_BitAddressOffset 20
#define GUSB2RHBCTL_BLOCK_U2_CONNECT_RegisterSize 1



#define GUSB2RHBCTL_eUSB2EOR_DEL_BitAddressOffset 21
#define GUSB2RHBCTL_eUSB2EOR_DEL_RegisterSize 4



#define GUSB2RHBCTL_eUSB2EOR_ENH_DIS_BitAddressOffset 25
#define GUSB2RHBCTL_eUSB2EOR_ENH_DIS_RegisterSize 1



#define GUSB2RHBCTL_Reserved_31_26_BitAddressOffset 26
#define GUSB2RHBCTL_Reserved_31_26_RegisterSize 6





#define GUSB2RHBCTL_0 (DWC_usb31_block_gbl_BaseAddress + 0x540)
#define GUSB2RHBCTL_RegisterSize 32
#define GUSB2RHBCTL_RegisterResetValue 0x0
#define GUSB2RHBCTL_RegisterResetMask 0x3ffffff





#define GUSB2RHBCTL_OVRD_L1TIMEOUT_BitAddressOffset 0
#define GUSB2RHBCTL_OVRD_L1TIMEOUT_RegisterSize 4



#define GUSB2RHBCTL_OVRD_HS_INT_PKT_DEL_BitAddressOffset 4
#define GUSB2RHBCTL_OVRD_HS_INT_PKT_DEL_RegisterSize 8



#define GUSB2RHBCTL_OVRD_FS_INT_PKT_DEL_BitAddressOffset 12
#define GUSB2RHBCTL_OVRD_FS_INT_PKT_DEL_RegisterSize 8



#define GUSB2RHBCTL_BLOCK_U2_CONNECT_BitAddressOffset 20
#define GUSB2RHBCTL_BLOCK_U2_CONNECT_RegisterSize 1



#define GUSB2RHBCTL_eUSB2EOR_DEL_BitAddressOffset 21
#define GUSB2RHBCTL_eUSB2EOR_DEL_RegisterSize 4



#define GUSB2RHBCTL_eUSB2EOR_ENH_DIS_BitAddressOffset 25
#define GUSB2RHBCTL_eUSB2EOR_ENH_DIS_RegisterSize 1



#define GUSB2RHBCTL_Reserved_31_26_BitAddressOffset 26
#define GUSB2RHBCTL_Reserved_31_26_RegisterSize 6





#define GUCTL6 (DWC_usb31_block_gbl_BaseAddress + 0x580)
#define GUCTL6_RegisterSize 32
#define GUCTL6_RegisterResetValue 0x0
#define GUCTL6_RegisterResetMask 0xffffffff





#define GUCTL6_star_fix_disable_ctl_nsr_31_0_BitAddressOffset 0
#define GUCTL6_star_fix_disable_ctl_nsr_31_0_RegisterSize 32





#define GUCTL7 (DWC_usb31_block_gbl_BaseAddress + 0x584)
#define GUCTL7_RegisterSize 32
#define GUCTL7_RegisterResetValue 0x0
#define GUCTL7_RegisterResetMask 0xffffffff





#define GUCTL7_star_fix_disable_ctl_nsr_63_32_BitAddressOffset 0
#define GUCTL7_star_fix_disable_ctl_nsr_63_32_RegisterSize 32





#define GUCTL8 (DWC_usb31_block_gbl_BaseAddress + 0x588)
#define GUCTL8_RegisterSize 32
#define GUCTL8_RegisterResetValue 0x0
#define GUCTL8_RegisterResetMask 0xffffffff





#define GUCTL8_star_fix_disable_ctl_nsr_95_64_BitAddressOffset 0
#define GUCTL8_star_fix_disable_ctl_nsr_95_64_RegisterSize 32





#define GUCTL9 (DWC_usb31_block_gbl_BaseAddress + 0x58c)
#define GUCTL9_RegisterSize 32
#define GUCTL9_RegisterResetValue 0x0
#define GUCTL9_RegisterResetMask 0xffffffff





#define GUCTL9_star_fix_disable_ctl_nsr_127_96_BitAddressOffset 0
#define GUCTL9_star_fix_disable_ctl_nsr_127_96_RegisterSize 32





#define GUCTL10 (DWC_usb31_block_gbl_BaseAddress + 0x590)
#define GUCTL10_RegisterSize 32
#define GUCTL10_RegisterResetValue 0x0
#define GUCTL10_RegisterResetMask 0xffffffff





#define GUCTL10_star_fix_disable_ctl_ssr_31_0_BitAddressOffset 0
#define GUCTL10_star_fix_disable_ctl_ssr_31_0_RegisterSize 32





#define GHWPARAMS9 (DWC_usb31_block_gbl_BaseAddress + 0x5e0)
#define GHWPARAMS9_RegisterSize 32
#define GHWPARAMS9_RegisterResetValue 0x0
#define GHWPARAMS9_RegisterResetMask 0x3fffff





#define GHWPARAMS9_ghwparams9_20_0_BitAddressOffset 0
#define GHWPARAMS9_ghwparams9_20_0_RegisterSize 21



#define GHWPARAMS9_ghwparams9_21_BitAddressOffset 21
#define GHWPARAMS9_ghwparams9_21_RegisterSize 1



#define GHWPARAMS9_reserved_31_22_BitAddressOffset 22
#define GHWPARAMS9_reserved_31_22_RegisterSize 10



#define DWC_usb31_block_dev_BaseAddress 0xc700



#define DCFG (DWC_usb31_block_dev_BaseAddress + 0x0)
#define DCFG_RegisterSize 32
#define DCFG_RegisterResetValue 0x80805
#define DCFG_RegisterResetMask 0x1fff3ff





#define DCFG_DEVSPD_BitAddressOffset 0
#define DCFG_DEVSPD_RegisterSize 3



#define DCFG_DEVADDR_BitAddressOffset 3
#define DCFG_DEVADDR_RegisterSize 7



#define DCFG_reserved_10_11_BitAddressOffset 10
#define DCFG_reserved_10_11_RegisterSize 2



#define DCFG_INTRNUM_BitAddressOffset 12
#define DCFG_INTRNUM_RegisterSize 5



#define DCFG_NUMP_BitAddressOffset 17
#define DCFG_NUMP_RegisterSize 5



#define DCFG_LPMCAP_BitAddressOffset 22
#define DCFG_LPMCAP_RegisterSize 1



#define DCFG_IgnStrmPP_BitAddressOffset 23
#define DCFG_IgnStrmPP_RegisterSize 1



#define DCFG_reserved_24_BitAddressOffset 24
#define DCFG_reserved_24_RegisterSize 1



#define DCFG_reserved_31_25_BitAddressOffset 25
#define DCFG_reserved_31_25_RegisterSize 7





#define DCTL (DWC_usb31_block_dev_BaseAddress + 0x4)
#define DCTL_RegisterSize 32
#define DCTL_RegisterResetValue 0xf00000
#define DCTL_RegisterResetMask 0xdfff1ffe





#define DCTL_reserved_0_BitAddressOffset 0
#define DCTL_reserved_0_RegisterSize 1



#define DCTL_TSTCTL_BitAddressOffset 1
#define DCTL_TSTCTL_RegisterSize 4



#define DCTL_ULSTCHNGREQ_BitAddressOffset 5
#define DCTL_ULSTCHNGREQ_RegisterSize 4



#define DCTL_ACCEPTU1ENA_BitAddressOffset 9
#define DCTL_ACCEPTU1ENA_RegisterSize 1



#define DCTL_INITU1ENA_BitAddressOffset 10
#define DCTL_INITU1ENA_RegisterSize 1



#define DCTL_ACCEPTU2ENA_BitAddressOffset 11
#define DCTL_ACCEPTU2ENA_RegisterSize 1



#define DCTL_INITU2ENA_BitAddressOffset 12
#define DCTL_INITU2ENA_RegisterSize 1



#define DCTL_reserved_15_13_BitAddressOffset 13
#define DCTL_reserved_15_13_RegisterSize 3



#define DCTL_CSS_BitAddressOffset 16
#define DCTL_CSS_RegisterSize 1



#define DCTL_CRS_BitAddressOffset 17
#define DCTL_CRS_RegisterSize 1



#define DCTL_L1HibernationEn_BitAddressOffset 18
#define DCTL_L1HibernationEn_RegisterSize 1



#define DCTL_KeepConnect_BitAddressOffset 19
#define DCTL_KeepConnect_RegisterSize 1



#define DCTL_LPM_NYET_thres_BitAddressOffset 20
#define DCTL_LPM_NYET_thres_RegisterSize 4



#define DCTL_HIRDTHRES_BitAddressOffset 24
#define DCTL_HIRDTHRES_RegisterSize 5



#define DCTL_reserved_29_BitAddressOffset 29
#define DCTL_reserved_29_RegisterSize 1



#define DCTL_CSFTRST_BitAddressOffset 30
#define DCTL_CSFTRST_RegisterSize 1



#define DCTL_RUN_STOP_BitAddressOffset 31
#define DCTL_RUN_STOP_RegisterSize 1





#define DEVTEN (DWC_usb31_block_dev_BaseAddress + 0x8)
#define DEVTEN_RegisterSize 32
#define DEVTEN_RegisterResetValue 0x0
#define DEVTEN_RegisterResetMask 0xf3ff





#define DEVTEN_DISSCONNEVTEN_BitAddressOffset 0
#define DEVTEN_DISSCONNEVTEN_RegisterSize 1



#define DEVTEN_USBRSTEVTEN_BitAddressOffset 1
#define DEVTEN_USBRSTEVTEN_RegisterSize 1



#define DEVTEN_CONNECTDONEEVTEN_BitAddressOffset 2
#define DEVTEN_CONNECTDONEEVTEN_RegisterSize 1



#define DEVTEN_ULSTCNGEN_BitAddressOffset 3
#define DEVTEN_ULSTCNGEN_RegisterSize 1



#define DEVTEN_WKUPEVTEN_BitAddressOffset 4
#define DEVTEN_WKUPEVTEN_RegisterSize 1



#define DEVTEN_HibernationReqEvtEn_BitAddressOffset 5
#define DEVTEN_HibernationReqEvtEn_RegisterSize 1



#define DEVTEN_U3L2L1SuspEn_BitAddressOffset 6
#define DEVTEN_U3L2L1SuspEn_RegisterSize 1



#define DEVTEN_SOFTEVTEN_BitAddressOffset 7
#define DEVTEN_SOFTEVTEN_RegisterSize 1



#define DEVTEN_L1SUSPEN_BitAddressOffset 8
#define DEVTEN_L1SUSPEN_RegisterSize 1



#define DEVTEN_ERRTICERREVTEN_BitAddressOffset 9
#define DEVTEN_ERRTICERREVTEN_RegisterSize 1



#define DEVTEN_reserved_10_BitAddressOffset 10
#define DEVTEN_reserved_10_RegisterSize 1



#define DEVTEN_reserved_11_BitAddressOffset 11
#define DEVTEN_reserved_11_RegisterSize 1



#define DEVTEN_VENDEVTSTRCVDEN_BitAddressOffset 12
#define DEVTEN_VENDEVTSTRCVDEN_RegisterSize 1



#define DEVTEN_reserved_13_BitAddressOffset 13
#define DEVTEN_reserved_13_RegisterSize 1



#define DEVTEN_L1WKUPEVTEN_BitAddressOffset 14
#define DEVTEN_L1WKUPEVTEN_RegisterSize 1



#define DEVTEN_LDMEVTEN_BitAddressOffset 15
#define DEVTEN_LDMEVTEN_RegisterSize 1



#define DEVTEN_reserved_31_16_BitAddressOffset 16
#define DEVTEN_reserved_31_16_RegisterSize 16





#define DSTS (DWC_usb31_block_dev_BaseAddress + 0xc)
#define DSTS_RegisterSize 32
#define DSTS_RegisterResetValue 0x520004
#define DSTS_RegisterResetMask 0x23c3ffff





#define DSTS_CONNECTSPD_BitAddressOffset 0
#define DSTS_CONNECTSPD_RegisterSize 3



#define DSTS_SOFFN_BitAddressOffset 3
#define DSTS_SOFFN_RegisterSize 14



#define DSTS_RXFIFOEMPTY_BitAddressOffset 17
#define DSTS_RXFIFOEMPTY_RegisterSize 1



#define DSTS_USBLNKST_BitAddressOffset 18
#define DSTS_USBLNKST_RegisterSize 4



#define DSTS_DEVCTRLHLT_BitAddressOffset 22
#define DSTS_DEVCTRLHLT_RegisterSize 1



#define DSTS_COREIDLE_BitAddressOffset 23
#define DSTS_COREIDLE_RegisterSize 1



#define DSTS_SSS_BitAddressOffset 24
#define DSTS_SSS_RegisterSize 1



#define DSTS_RSS_BitAddressOffset 25
#define DSTS_RSS_RegisterSize 1



#define DSTS_reserved_27_26_BitAddressOffset 26
#define DSTS_reserved_27_26_RegisterSize 2



#define DSTS_SRE_BitAddressOffset 28
#define DSTS_SRE_RegisterSize 1



#define DSTS_DCNRD_BitAddressOffset 29
#define DSTS_DCNRD_RegisterSize 1



#define DSTS_reserved_31_30_BitAddressOffset 30
#define DSTS_reserved_31_30_RegisterSize 2





#define DGCMDPAR (DWC_usb31_block_dev_BaseAddress + 0x10)
#define DGCMDPAR_RegisterSize 32
#define DGCMDPAR_RegisterResetValue 0x0
#define DGCMDPAR_RegisterResetMask 0xffffffff





#define DGCMDPAR_PARAMETER_BitAddressOffset 0
#define DGCMDPAR_PARAMETER_RegisterSize 32





#define DGCMD (DWC_usb31_block_dev_BaseAddress + 0x14)
#define DGCMD_RegisterSize 32
#define DGCMD_RegisterResetValue 0x0
#define DGCMD_RegisterResetMask 0xf5ff





#define DGCMD_CMDTYP_BitAddressOffset 0
#define DGCMD_CMDTYP_RegisterSize 8



#define DGCMD_CMDIOC_BitAddressOffset 8
#define DGCMD_CMDIOC_RegisterSize 1



#define DGCMD_reserved_9_BitAddressOffset 9
#define DGCMD_reserved_9_RegisterSize 1



#define DGCMD_CMDACT_BitAddressOffset 10
#define DGCMD_CMDACT_RegisterSize 1



#define DGCMD_reserved_11_BitAddressOffset 11
#define DGCMD_reserved_11_RegisterSize 1



#define DGCMD_CMDSTATUS_BitAddressOffset 12
#define DGCMD_CMDSTATUS_RegisterSize 4



#define DGCMD_reserved_31_16_BitAddressOffset 16
#define DGCMD_reserved_31_16_RegisterSize 16





#define DCTL1 (DWC_usb31_block_dev_BaseAddress + 0x18)
#define DCTL1_RegisterSize 32
#define DCTL1_RegisterResetValue 0x0
#define DCTL1_RegisterResetMask 0x0





#define DCTL1_reserved_0_BitAddressOffset 0
#define DCTL1_reserved_0_RegisterSize 1



#define DCTL1_DIS_CLRSPR_SXFER_BitAddressOffset 1
#define DCTL1_DIS_CLRSPR_SXFER_RegisterSize 1



#define DCTL1_EN_ENDXFER_ON_RJCT_STRM_BitAddressOffset 2
#define DCTL1_EN_ENDXFER_ON_RJCT_STRM_RegisterSize 1



#define DCTL1_reserved_31_3_BitAddressOffset 3
#define DCTL1_reserved_31_3_RegisterSize 29





#define DALEPENA (DWC_usb31_block_dev_BaseAddress + 0x20)
#define DALEPENA_RegisterSize 32
#define DALEPENA_RegisterResetValue 0x0
#define DALEPENA_RegisterResetMask 0xffffffff





#define DALEPENA_USBACTEP_BitAddressOffset 0
#define DALEPENA_USBACTEP_RegisterSize 32





#define DLDMENA (DWC_usb31_block_dev_BaseAddress + 0x24)
#define DLDMENA_RegisterSize 32
#define DLDMENA_RegisterResetValue 0x0
#define DLDMENA_RegisterResetMask 0xffffffff





#define DLDMENA_LDMENA_BitAddressOffset 0
#define DLDMENA_LDMENA_RegisterSize 1



#define DLDMENA_LDMRQS_BitAddressOffset 1
#define DLDMENA_LDMRQS_RegisterSize 3



#define DLDMENA_NOLOWPWRDUR_BitAddressOffset 4
#define DLDMENA_NOLOWPWRDUR_RegisterSize 4



#define DLDMENA_LDMDUR_BitAddressOffset 8
#define DLDMENA_LDMDUR_RegisterSize 8



#define DLDMENA_LDMADJ_BitAddressOffset 16
#define DLDMENA_LDMADJ_RegisterSize 16





#define Rsvd_REGS (DWC_usb31_block_dev_BaseAddress + 0x28)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_0 (DWC_usb31_block_dev_BaseAddress + 0x28)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_1 (DWC_usb31_block_dev_BaseAddress + 0x2c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_2 (DWC_usb31_block_dev_BaseAddress + 0x30)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_3 (DWC_usb31_block_dev_BaseAddress + 0x34)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_4 (DWC_usb31_block_dev_BaseAddress + 0x38)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_5 (DWC_usb31_block_dev_BaseAddress + 0x3c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_6 (DWC_usb31_block_dev_BaseAddress + 0x40)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_7 (DWC_usb31_block_dev_BaseAddress + 0x44)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_8 (DWC_usb31_block_dev_BaseAddress + 0x48)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_9 (DWC_usb31_block_dev_BaseAddress + 0x4c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_10 (DWC_usb31_block_dev_BaseAddress + 0x50)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_11 (DWC_usb31_block_dev_BaseAddress + 0x54)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_12 (DWC_usb31_block_dev_BaseAddress + 0x58)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_13 (DWC_usb31_block_dev_BaseAddress + 0x5c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_14 (DWC_usb31_block_dev_BaseAddress + 0x60)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_15 (DWC_usb31_block_dev_BaseAddress + 0x64)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_16 (DWC_usb31_block_dev_BaseAddress + 0x68)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_17 (DWC_usb31_block_dev_BaseAddress + 0x6c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_18 (DWC_usb31_block_dev_BaseAddress + 0x70)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_19 (DWC_usb31_block_dev_BaseAddress + 0x74)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_20 (DWC_usb31_block_dev_BaseAddress + 0x78)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_21 (DWC_usb31_block_dev_BaseAddress + 0x7c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_22 (DWC_usb31_block_dev_BaseAddress + 0x80)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_23 (DWC_usb31_block_dev_BaseAddress + 0x84)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_24 (DWC_usb31_block_dev_BaseAddress + 0x88)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_25 (DWC_usb31_block_dev_BaseAddress + 0x8c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_26 (DWC_usb31_block_dev_BaseAddress + 0x90)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_27 (DWC_usb31_block_dev_BaseAddress + 0x94)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_28 (DWC_usb31_block_dev_BaseAddress + 0x98)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_29 (DWC_usb31_block_dev_BaseAddress + 0x9c)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_30 (DWC_usb31_block_dev_BaseAddress + 0xa0)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define Rsvd_31 (DWC_usb31_block_dev_BaseAddress + 0xa4)
#define Rsvd_RegisterSize 32
#define Rsvd_RegisterResetValue 0x0
#define Rsvd_RegisterResetMask 0x0





#define Rsvd_reserved_31_0_BitAddressOffset 0
#define Rsvd_reserved_31_0_RegisterSize 32





#define DEPCMDPAR2_REGS (DWC_usb31_block_dev_BaseAddress + 0x100)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR2_0 (DWC_usb31_block_dev_BaseAddress + 0x100)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_0 (DWC_usb31_block_dev_BaseAddress + 0x104)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_0 (DWC_usb31_block_dev_BaseAddress + 0x108)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_0 (DWC_usb31_block_dev_BaseAddress + 0x10c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_1 (DWC_usb31_block_dev_BaseAddress + 0x110)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_1 (DWC_usb31_block_dev_BaseAddress + 0x114)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_1 (DWC_usb31_block_dev_BaseAddress + 0x118)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_1 (DWC_usb31_block_dev_BaseAddress + 0x11c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_2 (DWC_usb31_block_dev_BaseAddress + 0x120)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_2 (DWC_usb31_block_dev_BaseAddress + 0x124)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_2 (DWC_usb31_block_dev_BaseAddress + 0x128)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_2 (DWC_usb31_block_dev_BaseAddress + 0x12c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_3 (DWC_usb31_block_dev_BaseAddress + 0x130)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_3 (DWC_usb31_block_dev_BaseAddress + 0x134)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_3 (DWC_usb31_block_dev_BaseAddress + 0x138)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_3 (DWC_usb31_block_dev_BaseAddress + 0x13c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_4 (DWC_usb31_block_dev_BaseAddress + 0x140)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_4 (DWC_usb31_block_dev_BaseAddress + 0x144)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_4 (DWC_usb31_block_dev_BaseAddress + 0x148)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_4 (DWC_usb31_block_dev_BaseAddress + 0x14c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_5 (DWC_usb31_block_dev_BaseAddress + 0x150)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_5 (DWC_usb31_block_dev_BaseAddress + 0x154)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_5 (DWC_usb31_block_dev_BaseAddress + 0x158)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_5 (DWC_usb31_block_dev_BaseAddress + 0x15c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_6 (DWC_usb31_block_dev_BaseAddress + 0x160)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_6 (DWC_usb31_block_dev_BaseAddress + 0x164)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_6 (DWC_usb31_block_dev_BaseAddress + 0x168)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_6 (DWC_usb31_block_dev_BaseAddress + 0x16c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_7 (DWC_usb31_block_dev_BaseAddress + 0x170)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_7 (DWC_usb31_block_dev_BaseAddress + 0x174)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_7 (DWC_usb31_block_dev_BaseAddress + 0x178)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_7 (DWC_usb31_block_dev_BaseAddress + 0x17c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_8 (DWC_usb31_block_dev_BaseAddress + 0x180)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_8 (DWC_usb31_block_dev_BaseAddress + 0x184)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_8 (DWC_usb31_block_dev_BaseAddress + 0x188)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_8 (DWC_usb31_block_dev_BaseAddress + 0x18c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_9 (DWC_usb31_block_dev_BaseAddress + 0x190)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_9 (DWC_usb31_block_dev_BaseAddress + 0x194)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_9 (DWC_usb31_block_dev_BaseAddress + 0x198)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_9 (DWC_usb31_block_dev_BaseAddress + 0x19c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_10 (DWC_usb31_block_dev_BaseAddress + 0x1a0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_10 (DWC_usb31_block_dev_BaseAddress + 0x1a4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_10 (DWC_usb31_block_dev_BaseAddress + 0x1a8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_10 (DWC_usb31_block_dev_BaseAddress + 0x1ac)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_11 (DWC_usb31_block_dev_BaseAddress + 0x1b0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_11 (DWC_usb31_block_dev_BaseAddress + 0x1b4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_11 (DWC_usb31_block_dev_BaseAddress + 0x1b8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_11 (DWC_usb31_block_dev_BaseAddress + 0x1bc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_12 (DWC_usb31_block_dev_BaseAddress + 0x1c0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_12 (DWC_usb31_block_dev_BaseAddress + 0x1c4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_12 (DWC_usb31_block_dev_BaseAddress + 0x1c8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_12 (DWC_usb31_block_dev_BaseAddress + 0x1cc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_13 (DWC_usb31_block_dev_BaseAddress + 0x1d0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_13 (DWC_usb31_block_dev_BaseAddress + 0x1d4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_13 (DWC_usb31_block_dev_BaseAddress + 0x1d8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_13 (DWC_usb31_block_dev_BaseAddress + 0x1dc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_14 (DWC_usb31_block_dev_BaseAddress + 0x1e0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_14 (DWC_usb31_block_dev_BaseAddress + 0x1e4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_14 (DWC_usb31_block_dev_BaseAddress + 0x1e8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_14 (DWC_usb31_block_dev_BaseAddress + 0x1ec)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_15 (DWC_usb31_block_dev_BaseAddress + 0x1f0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_15 (DWC_usb31_block_dev_BaseAddress + 0x1f4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_15 (DWC_usb31_block_dev_BaseAddress + 0x1f8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_15 (DWC_usb31_block_dev_BaseAddress + 0x1fc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_16 (DWC_usb31_block_dev_BaseAddress + 0x200)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_16 (DWC_usb31_block_dev_BaseAddress + 0x204)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_16 (DWC_usb31_block_dev_BaseAddress + 0x208)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_16 (DWC_usb31_block_dev_BaseAddress + 0x20c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_17 (DWC_usb31_block_dev_BaseAddress + 0x210)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_17 (DWC_usb31_block_dev_BaseAddress + 0x214)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_17 (DWC_usb31_block_dev_BaseAddress + 0x218)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_17 (DWC_usb31_block_dev_BaseAddress + 0x21c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_18 (DWC_usb31_block_dev_BaseAddress + 0x220)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_18 (DWC_usb31_block_dev_BaseAddress + 0x224)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_18 (DWC_usb31_block_dev_BaseAddress + 0x228)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_18 (DWC_usb31_block_dev_BaseAddress + 0x22c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_19 (DWC_usb31_block_dev_BaseAddress + 0x230)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_19 (DWC_usb31_block_dev_BaseAddress + 0x234)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_19 (DWC_usb31_block_dev_BaseAddress + 0x238)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_19 (DWC_usb31_block_dev_BaseAddress + 0x23c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_20 (DWC_usb31_block_dev_BaseAddress + 0x240)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_20 (DWC_usb31_block_dev_BaseAddress + 0x244)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_20 (DWC_usb31_block_dev_BaseAddress + 0x248)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_20 (DWC_usb31_block_dev_BaseAddress + 0x24c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_21 (DWC_usb31_block_dev_BaseAddress + 0x250)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_21 (DWC_usb31_block_dev_BaseAddress + 0x254)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_21 (DWC_usb31_block_dev_BaseAddress + 0x258)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_21 (DWC_usb31_block_dev_BaseAddress + 0x25c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_22 (DWC_usb31_block_dev_BaseAddress + 0x260)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_22 (DWC_usb31_block_dev_BaseAddress + 0x264)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_22 (DWC_usb31_block_dev_BaseAddress + 0x268)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_22 (DWC_usb31_block_dev_BaseAddress + 0x26c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_23 (DWC_usb31_block_dev_BaseAddress + 0x270)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_23 (DWC_usb31_block_dev_BaseAddress + 0x274)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_23 (DWC_usb31_block_dev_BaseAddress + 0x278)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_23 (DWC_usb31_block_dev_BaseAddress + 0x27c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_24 (DWC_usb31_block_dev_BaseAddress + 0x280)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_24 (DWC_usb31_block_dev_BaseAddress + 0x284)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_24 (DWC_usb31_block_dev_BaseAddress + 0x288)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_24 (DWC_usb31_block_dev_BaseAddress + 0x28c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_25 (DWC_usb31_block_dev_BaseAddress + 0x290)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_25 (DWC_usb31_block_dev_BaseAddress + 0x294)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_25 (DWC_usb31_block_dev_BaseAddress + 0x298)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_25 (DWC_usb31_block_dev_BaseAddress + 0x29c)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_26 (DWC_usb31_block_dev_BaseAddress + 0x2a0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_26 (DWC_usb31_block_dev_BaseAddress + 0x2a4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_26 (DWC_usb31_block_dev_BaseAddress + 0x2a8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_26 (DWC_usb31_block_dev_BaseAddress + 0x2ac)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_27 (DWC_usb31_block_dev_BaseAddress + 0x2b0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_27 (DWC_usb31_block_dev_BaseAddress + 0x2b4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_27 (DWC_usb31_block_dev_BaseAddress + 0x2b8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_27 (DWC_usb31_block_dev_BaseAddress + 0x2bc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_28 (DWC_usb31_block_dev_BaseAddress + 0x2c0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_28 (DWC_usb31_block_dev_BaseAddress + 0x2c4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_28 (DWC_usb31_block_dev_BaseAddress + 0x2c8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_28 (DWC_usb31_block_dev_BaseAddress + 0x2cc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_29 (DWC_usb31_block_dev_BaseAddress + 0x2d0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_29 (DWC_usb31_block_dev_BaseAddress + 0x2d4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_29 (DWC_usb31_block_dev_BaseAddress + 0x2d8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_29 (DWC_usb31_block_dev_BaseAddress + 0x2dc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_30 (DWC_usb31_block_dev_BaseAddress + 0x2e0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_30 (DWC_usb31_block_dev_BaseAddress + 0x2e4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_30 (DWC_usb31_block_dev_BaseAddress + 0x2e8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_30 (DWC_usb31_block_dev_BaseAddress + 0x2ec)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEPCMDPAR2_31 (DWC_usb31_block_dev_BaseAddress + 0x2f0)
#define DEPCMDPAR2_RegisterSize 32
#define DEPCMDPAR2_RegisterResetValue 0x0
#define DEPCMDPAR2_RegisterResetMask 0x0





#define DEPCMDPAR2_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR2_PARAMETER_RegisterSize 32





#define DEPCMDPAR1_31 (DWC_usb31_block_dev_BaseAddress + 0x2f4)
#define DEPCMDPAR1_RegisterSize 32
#define DEPCMDPAR1_RegisterResetValue 0x0
#define DEPCMDPAR1_RegisterResetMask 0x0





#define DEPCMDPAR1_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR1_PARAMETER_RegisterSize 32





#define DEPCMDPAR0_31 (DWC_usb31_block_dev_BaseAddress + 0x2f8)
#define DEPCMDPAR0_RegisterSize 32
#define DEPCMDPAR0_RegisterResetValue 0x0
#define DEPCMDPAR0_RegisterResetMask 0x0





#define DEPCMDPAR0_PARAMETER_BitAddressOffset 0
#define DEPCMDPAR0_PARAMETER_RegisterSize 32





#define DEPCMD_31 (DWC_usb31_block_dev_BaseAddress + 0x2fc)
#define DEPCMD_RegisterSize 32
#define DEPCMD_RegisterResetValue 0x0
#define DEPCMD_RegisterResetMask 0x0





#define DEPCMD_CMDTYP_BitAddressOffset 0
#define DEPCMD_CMDTYP_RegisterSize 4



#define DEPCMD_reserved_7_4_BitAddressOffset 4
#define DEPCMD_reserved_7_4_RegisterSize 4



#define DEPCMD_CMDIOC_BitAddressOffset 8
#define DEPCMD_CMDIOC_RegisterSize 1



#define DEPCMD_CMD_BP_PTL_HSHK_BitAddressOffset 9
#define DEPCMD_CMD_BP_PTL_HSHK_RegisterSize 1



#define DEPCMD_CMDACT_BitAddressOffset 10
#define DEPCMD_CMDACT_RegisterSize 1



#define DEPCMD_HIPRI_FORCERM_BitAddressOffset 11
#define DEPCMD_HIPRI_FORCERM_RegisterSize 1



#define DEPCMD_CMDSTATUS_BitAddressOffset 12
#define DEPCMD_CMDSTATUS_RegisterSize 4



#define DEPCMD_COMMANDPARAM_BitAddressOffset 16
#define DEPCMD_COMMANDPARAM_RegisterSize 16





#define DEV_IMOD_REGS (DWC_usb31_block_dev_BaseAddress + 0x300)
#define DEV_IMOD_RegisterSize 32
#define DEV_IMOD_RegisterResetValue 0x0
#define DEV_IMOD_RegisterResetMask 0x0





#define DEV_IMOD_DEVICE_IMODI_BitAddressOffset 0
#define DEV_IMOD_DEVICE_IMODI_RegisterSize 16



#define DEV_IMOD_DEVICE_IMODC_BitAddressOffset 16
#define DEV_IMOD_DEVICE_IMODC_RegisterSize 16





#define DEV_IMOD_0 (DWC_usb31_block_dev_BaseAddress + 0x300)
#define DEV_IMOD_RegisterSize 32
#define DEV_IMOD_RegisterResetValue 0x0
#define DEV_IMOD_RegisterResetMask 0x0





#define DEV_IMOD_DEVICE_IMODI_BitAddressOffset 0
#define DEV_IMOD_DEVICE_IMODI_RegisterSize 16



#define DEV_IMOD_DEVICE_IMODC_BitAddressOffset 16
#define DEV_IMOD_DEVICE_IMODC_RegisterSize 16



#define DWC_usb31_block_rsvd0_BaseAddress 0xcc00



#define rsvd0_reg (DWC_usb31_block_rsvd0_BaseAddress + 0x0)
#define rsvd0_reg_RegisterSize 32
#define rsvd0_reg_RegisterResetValue 0x0
#define rsvd0_reg_RegisterResetMask 0xffffffff





#define rsvd0_reg_rsvd_field_BitAddressOffset 0
#define rsvd0_reg_rsvd_field_RegisterSize 32



#define DWC_usb31_block_link_BaseAddress 0xd000



#define LINK_REGS (DWC_usb31_block_link_BaseAddress + 0x0)
#define LU1LFPSRXTIM_RegisterSize 32
#define LU1LFPSRXTIM_RegisterResetValue 0x30302626
#define LU1LFPSRXTIM_RegisterResetMask 0xffffffff





#define LU1LFPSRXTIM_gen1_u1_exit_rsp_rx_clk_BitAddressOffset 0
#define LU1LFPSRXTIM_gen1_u1_exit_rsp_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen1_u1_lfps_exit_rx_clk_BitAddressOffset 8
#define LU1LFPSRXTIM_gen1_u1_lfps_exit_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen2_u1_exit_rsp_rx_clk_BitAddressOffset 16
#define LU1LFPSRXTIM_gen2_u1_exit_rsp_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen2_u1_lfps_exit_rx_clk_BitAddressOffset 24
#define LU1LFPSRXTIM_gen2_u1_lfps_exit_rx_clk_RegisterSize 8





#define LU1LFPSRXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x0)
#define LU1LFPSRXTIM_RegisterSize 32
#define LU1LFPSRXTIM_RegisterResetValue 0x30302626
#define LU1LFPSRXTIM_RegisterResetMask 0xffffffff





#define LU1LFPSRXTIM_gen1_u1_exit_rsp_rx_clk_BitAddressOffset 0
#define LU1LFPSRXTIM_gen1_u1_exit_rsp_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen1_u1_lfps_exit_rx_clk_BitAddressOffset 8
#define LU1LFPSRXTIM_gen1_u1_lfps_exit_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen2_u1_exit_rsp_rx_clk_BitAddressOffset 16
#define LU1LFPSRXTIM_gen2_u1_exit_rsp_rx_clk_RegisterSize 8



#define LU1LFPSRXTIM_gen2_u1_lfps_exit_rx_clk_BitAddressOffset 24
#define LU1LFPSRXTIM_gen2_u1_lfps_exit_rx_clk_RegisterSize 8





#define LU1LFPSTXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x4)
#define LU1LFPSTXTIM_RegisterSize 32
#define LU1LFPSTXTIM_RegisterResetValue 0x940076
#define LU1LFPSTXTIM_RegisterResetMask 0x3fff3fff





#define LU1LFPSTXTIM_gen1_u1_exit_rsp_tx_clk_BitAddressOffset 0
#define LU1LFPSTXTIM_gen1_u1_exit_rsp_tx_clk_RegisterSize 14



#define LU1LFPSTXTIM_reserved_1_BitAddressOffset 14
#define LU1LFPSTXTIM_reserved_1_RegisterSize 2



#define LU1LFPSTXTIM_gen2_u1_exit_resp_tx_clk_BitAddressOffset 16
#define LU1LFPSTXTIM_gen2_u1_exit_resp_tx_clk_RegisterSize 14



#define LU1LFPSTXTIM_reserved_2_BitAddressOffset 30
#define LU1LFPSTXTIM_reserved_2_RegisterSize 2





#define LU2LFPSRXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x8)
#define LU2LFPSRXTIM_RegisterSize 32
#define LU2LFPSRXTIM_RegisterResetValue 0x26261f1f
#define LU2LFPSRXTIM_RegisterResetMask 0xffffffff





#define LU2LFPSRXTIM_gen1_u2_exit_rsp_rx_clk_BitAddressOffset 0
#define LU2LFPSRXTIM_gen1_u2_exit_rsp_rx_clk_RegisterSize 8



#define LU2LFPSRXTIM_gen1_u2_lfps_exit_rx_clk_BitAddressOffset 8
#define LU2LFPSRXTIM_gen1_u2_lfps_exit_rx_clk_RegisterSize 8



#define LU2LFPSRXTIM_gen2_u2_exit_rsp_rx_clk_BitAddressOffset 16
#define LU2LFPSRXTIM_gen2_u2_exit_rsp_rx_clk_RegisterSize 8



#define LU2LFPSRXTIM_gen2_u2_lfps_exit_rx_clk_BitAddressOffset 24
#define LU2LFPSRXTIM_gen2_u2_lfps_exit_rx_clk_RegisterSize 8





#define LU2LFPSTXTIM_0 (DWC_usb31_block_link_BaseAddress + 0xc)
#define LU2LFPSTXTIM_RegisterSize 32
#define LU2LFPSTXTIM_RegisterResetValue 0x52
#define LU2LFPSTXTIM_RegisterResetMask 0x3fff





#define LU2LFPSTXTIM_u2_exit_rsp_tx_us_BitAddressOffset 0
#define LU2LFPSTXTIM_u2_exit_rsp_tx_us_RegisterSize 14



#define LU2LFPSTXTIM_reserved_1_BitAddressOffset 14
#define LU2LFPSTXTIM_reserved_1_RegisterSize 18





#define LU3LFPSRXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x10)
#define LU3LFPSRXTIM_RegisterSize 32
#define LU3LFPSRXTIM_RegisterResetValue 0x9c067d05
#define LU3LFPSRXTIM_RegisterResetMask 0xffffffff





#define LU3LFPSRXTIM_gen1_u3_exit_rsp_rx_clk_BitAddressOffset 0
#define LU3LFPSRXTIM_gen1_u3_exit_rsp_rx_clk_RegisterSize 8



#define LU3LFPSRXTIM_gen1_u3_lfps_exit_rx_clk_BitAddressOffset 8
#define LU3LFPSRXTIM_gen1_u3_lfps_exit_rx_clk_RegisterSize 8



#define LU3LFPSRXTIM_gen2_u3_exit_rsp_rx_clk_BitAddressOffset 16
#define LU3LFPSRXTIM_gen2_u3_exit_rsp_rx_clk_RegisterSize 8



#define LU3LFPSRXTIM_gen2_u3_lfps_exit_rx_clk_BitAddressOffset 24
#define LU3LFPSRXTIM_gen2_u3_lfps_exit_rx_clk_RegisterSize 8





#define LU3LFPSTXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x14)
#define LU3LFPSTXTIM_RegisterSize 32
#define LU3LFPSTXTIM_RegisterResetValue 0x3fff3b15
#define LU3LFPSTXTIM_RegisterResetMask 0xffffffff





#define LU3LFPSTXTIM_gen1_u3_exit_rsp_tx_clk_BitAddressOffset 0
#define LU3LFPSTXTIM_gen1_u3_exit_rsp_tx_clk_RegisterSize 14



#define LU3LFPSTXTIM_reserved_15_14_BitAddressOffset 14
#define LU3LFPSTXTIM_reserved_15_14_RegisterSize 2



#define LU3LFPSTXTIM_gen2_u3_exit_tx_clk_BitAddressOffset 16
#define LU3LFPSTXTIM_gen2_u3_exit_tx_clk_RegisterSize 14



#define LU3LFPSTXTIM_reserved_31_30_BitAddressOffset 30
#define LU3LFPSTXTIM_reserved_31_30_RegisterSize 2





#define LPINGLFPSTIM_0 (DWC_usb31_block_link_BaseAddress + 0x18)
#define LPINGLFPSTIM_RegisterSize 32
#define LPINGLFPSTIM_RegisterResetValue 0x42843603
#define LPINGLFPSTIM_RegisterResetMask 0xffffffff





#define LPINGLFPSTIM_gen1_lfps_ping_min_clk_BitAddressOffset 0
#define LPINGLFPSTIM_gen1_lfps_ping_min_clk_RegisterSize 4



#define LPINGLFPSTIM_gen1_lfps_ping_max_clk_BitAddressOffset 4
#define LPINGLFPSTIM_gen1_lfps_ping_max_clk_RegisterSize 6



#define LPINGLFPSTIM_gen1_lfps_ping_burst_clk_BitAddressOffset 10
#define LPINGLFPSTIM_gen1_lfps_ping_burst_clk_RegisterSize 6



#define LPINGLFPSTIM_gen2_lfps_ping_min_clk_BitAddressOffset 16
#define LPINGLFPSTIM_gen2_lfps_ping_min_clk_RegisterSize 4



#define LPINGLFPSTIM_gen2_lfps_ping_max_clk_BitAddressOffset 20
#define LPINGLFPSTIM_gen2_lfps_ping_max_clk_RegisterSize 6



#define LPINGLFPSTIM_gen2_lfps_ping_burst_clk_BitAddressOffset 26
#define LPINGLFPSTIM_gen2_lfps_ping_burst_clk_RegisterSize 6





#define LPOLLLFPSTXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x1c)
#define LPOLLLFPSTXTIM_RegisterSize 32
#define LPOLLLFPSTXTIM_RegisterResetValue 0x23287d
#define LPOLLLFPSTXTIM_RegisterResetMask 0xffffffff





#define LPOLLLFPSTXTIM_lfps_poll_burst_clk_BitAddressOffset 0
#define LPOLLLFPSTXTIM_lfps_poll_burst_clk_RegisterSize 11



#define LPOLLLFPSTXTIM_lfps_poll_space_clk_BitAddressOffset 11
#define LPOLLLFPSTXTIM_lfps_poll_space_clk_RegisterSize 11



#define LPOLLLFPSTXTIM_reserved_31_22_BitAddressOffset 22
#define LPOLLLFPSTXTIM_reserved_31_22_RegisterSize 10





#define LSKIPFREQ_0 (DWC_usb31_block_link_BaseAddress + 0x20)
#define LSKIPFREQ_RegisterSize 32
#define LSKIPFREQ_RegisterResetValue 0x4e928162
#define LSKIPFREQ_RegisterResetMask 0x7fffffff





#define LSKIPFREQ_gen1_skp_freq_BitAddressOffset 0
#define LSKIPFREQ_gen1_skp_freq_RegisterSize 12



#define LSKIPFREQ_gen2_skp_freq_BitAddressOffset 12
#define LSKIPFREQ_gen2_skp_freq_RegisterSize 8



#define LSKIPFREQ_pm_entry_timer_us_BitAddressOffset 20
#define LSKIPFREQ_pm_entry_timer_us_RegisterSize 4



#define LSKIPFREQ_pm_lc_timer_us_BitAddressOffset 24
#define LSKIPFREQ_pm_lc_timer_us_RegisterSize 3



#define LSKIPFREQ_en_pm_timer_us_BitAddressOffset 27
#define LSKIPFREQ_en_pm_timer_us_RegisterSize 1



#define LSKIPFREQ_u1_resid_timer_us_BitAddressOffset 28
#define LSKIPFREQ_u1_resid_timer_us_RegisterSize 3



#define LSKIPFREQ_Reserved_1_BitAddressOffset 31
#define LSKIPFREQ_Reserved_1_RegisterSize 1





#define LLUCTL_0 (DWC_usb31_block_link_BaseAddress + 0x24)
#define LLUCTL_RegisterSize 32
#define LLUCTL_RegisterResetValue 0x8b8080
#define LLUCTL_RegisterResetMask 0x7fffffff





#define LLUCTL_tx_ts1_cnt_BitAddressOffset 0
#define LLUCTL_tx_ts1_cnt_RegisterSize 5



#define LLUCTL_no_ux_exit_p0_trans_BitAddressOffset 5
#define LLUCTL_no_ux_exit_p0_trans_RegisterSize 1



#define LLUCTL_delay_ux_after_lpma_BitAddressOffset 6
#define LLUCTL_delay_ux_after_lpma_RegisterSize 1



#define LLUCTL_mask_pipe_reset_BitAddressOffset 7
#define LLUCTL_mask_pipe_reset_RegisterSize 1



#define LLUCTL_gen1_loopback_entry_mode_BitAddressOffset 8
#define LLUCTL_gen1_loopback_entry_mode_RegisterSize 1



#define LLUCTL_gen2_loopback_entry_mode_BitAddressOffset 9
#define LLUCTL_gen2_loopback_entry_mode_RegisterSize 1



#define LLUCTL_force_gen1_BitAddressOffset 10
#define LLUCTL_force_gen1_RegisterSize 1



#define LLUCTL_en_reset_pipe_after_phy_mux_BitAddressOffset 11
#define LLUCTL_en_reset_pipe_after_phy_mux_RegisterSize 1



#define LLUCTL_U2P3CPMok_BitAddressOffset 12
#define LLUCTL_U2P3CPMok_RegisterSize 1



#define LLUCTL_ring_buf_d_delay_BitAddressOffset 13
#define LLUCTL_ring_buf_d_delay_RegisterSize 2



#define LLUCTL_en_us_hp_timer_BitAddressOffset 15
#define LLUCTL_en_us_hp_timer_RegisterSize 1



#define LLUCTL_pending_hp_timer_us_BitAddressOffset 16
#define LLUCTL_pending_hp_timer_us_RegisterSize 5



#define LLUCTL_en_dpp_truncate_BitAddressOffset 21
#define LLUCTL_en_dpp_truncate_RegisterSize 1



#define LLUCTL_force_dpp_truncate_BitAddressOffset 22
#define LLUCTL_force_dpp_truncate_RegisterSize 1



#define LLUCTL_DisRxDet_LTSSM_Timer_Ovrrd_BitAddressOffset 23
#define LLUCTL_DisRxDet_LTSSM_Timer_Ovrrd_RegisterSize 1



#define LLUCTL_delay_tx_gen1_dp_BitAddressOffset 24
#define LLUCTL_delay_tx_gen1_dp_RegisterSize 4



#define LLUCTL_support_p4_BitAddressOffset 28
#define LLUCTL_support_p4_RegisterSize 1



#define LLUCTL_support_p4_pg_BitAddressOffset 29
#define LLUCTL_support_p4_pg_RegisterSize 1



#define LLUCTL_inverse_sync_header_BitAddressOffset 30
#define LLUCTL_inverse_sync_header_RegisterSize 1



#define LLUCTL_Reserved_31_BitAddressOffset 31
#define LLUCTL_Reserved_31_RegisterSize 1





#define LPTMDPDELAY_0 (DWC_usb31_block_link_BaseAddress + 0x28)
#define LPTMDPDELAY_RegisterSize 32
#define LPTMDPDELAY_RegisterResetValue 0xc00
#define LPTMDPDELAY_RegisterResetMask 0xffffffff





#define LPTMDPDELAY_tx_path_delay_BitAddressOffset 0
#define LPTMDPDELAY_tx_path_delay_RegisterSize 5



#define LPTMDPDELAY_rx_path_delay_BitAddressOffset 5
#define LPTMDPDELAY_rx_path_delay_RegisterSize 5



#define LPTMDPDELAY_p3cpmp4_residency_BitAddressOffset 10
#define LPTMDPDELAY_p3cpmp4_residency_RegisterSize 12



#define LPTMDPDELAY_tx_path_delay_gen2_BitAddressOffset 22
#define LPTMDPDELAY_tx_path_delay_gen2_RegisterSize 5



#define LPTMDPDELAY_rx_path_delay_gen2_BitAddressOffset 27
#define LPTMDPDELAY_rx_path_delay_gen2_RegisterSize 5





#define LSCDTIM1_0 (DWC_usb31_block_link_BaseAddress + 0x2c)
#define LSCDTIM1_RegisterSize 32
#define LSCDTIM1_RegisterResetValue 0x47e1f4
#define LSCDTIM1_RegisterResetMask 0xffffff





#define LSCDTIM1_cd_bit0_rpt_min_clk_BitAddressOffset 0
#define LSCDTIM1_cd_bit0_rpt_min_clk_RegisterSize 12



#define LSCDTIM1_scd_bit0_rpt_max_clk_BitAddressOffset 12
#define LSCDTIM1_scd_bit0_rpt_max_clk_RegisterSize 12



#define LSCDTIM1_Reserved_1_BitAddressOffset 24
#define LSCDTIM1_Reserved_1_RegisterSize 8





#define LSCDTIM2_0 (DWC_usb31_block_link_BaseAddress + 0x30)
#define LSCDTIM2_RegisterSize 32
#define LSCDTIM2_RegisterResetValue 0x753546
#define LSCDTIM2_RegisterResetMask 0xffffff





#define LSCDTIM2_cd_bit1_rpt_min_clk_BitAddressOffset 0
#define LSCDTIM2_cd_bit1_rpt_min_clk_RegisterSize 12



#define LSCDTIM2_scd_bit1_rpt_max_clk_BitAddressOffset 12
#define LSCDTIM2_scd_bit1_rpt_max_clk_RegisterSize 12



#define LSCDTIM2_Reserved_1_BitAddressOffset 24
#define LSCDTIM2_Reserved_1_RegisterSize 8





#define LSCDTIM3_0 (DWC_usb31_block_link_BaseAddress + 0x34)
#define LSCDTIM3_RegisterSize 32
#define LSCDTIM3_RegisterResetValue 0x5dc2f07d
#define LSCDTIM3_RegisterResetMask 0xffffffff





#define LSCDTIM3_lfps_scd_burst_clk_BitAddressOffset 0
#define LSCDTIM3_lfps_scd_burst_clk_RegisterSize 8



#define LSCDTIM3_lfps_scd_space0_clk_BitAddressOffset 8
#define LSCDTIM3_lfps_scd_space0_clk_RegisterSize 12



#define LSCDTIM3_lfps_scd_space1_clk_BitAddressOffset 20
#define LSCDTIM3_lfps_scd_space1_clk_RegisterSize 12





#define LSCDTIM4_0 (DWC_usb31_block_link_BaseAddress + 0x38)
#define LSCDTIM4_RegisterSize 32
#define LSCDTIM4_RegisterResetValue 0xea6
#define LSCDTIM4_RegisterResetMask 0xfff





#define LSCDTIM4_lfps_scd_last_space_BitAddressOffset 0
#define LSCDTIM4_lfps_scd_last_space_RegisterSize 12



#define LSCDTIM4_Reserved_1_BitAddressOffset 12
#define LSCDTIM4_Reserved_1_RegisterSize 20





#define LLPBMTIM1_0 (DWC_usb31_block_link_BaseAddress + 0x3c)
#define LLPBMTIM1_RegisterSize 32
#define LLPBMTIM1_RegisterResetValue 0xf0966b32
#define LLPBMTIM1_RegisterResetMask 0xffffffff





#define LLPBMTIM1_lfps_lbps0_burst_min_clk_BitAddressOffset 0
#define LLPBMTIM1_lfps_lbps0_burst_min_clk_RegisterSize 8



#define LLPBMTIM1_lfps_lbps0_burst_max_clk_BitAddressOffset 8
#define LLPBMTIM1_lfps_lbps0_burst_max_clk_RegisterSize 8



#define LLPBMTIM1_lfps_lbps1_burst_min_clk_BitAddressOffset 16
#define LLPBMTIM1_lfps_lbps1_burst_min_clk_RegisterSize 8



#define LLPBMTIM1_lfps_lbps1_burst_max_clk_BitAddressOffset 24
#define LLPBMTIM1_lfps_lbps1_burst_max_clk_RegisterSize 8





#define LLPBMTIM2_0 (DWC_usb31_block_link_BaseAddress + 0x40)
#define LLPBMTIM2_RegisterSize 32
#define LLPBMTIM2_RegisterResetValue 0x26cf5
#define LLPBMTIM2_RegisterResetMask 0x3ffff





#define LLPBMTIM2_lfps_lbpm_tpwm_min_clk_BitAddressOffset 0
#define LLPBMTIM2_lfps_lbpm_tpwm_min_clk_RegisterSize 9



#define LLPBMTIM2_lfps_lbpm_tpwm_max_clk_BitAddressOffset 9
#define LLPBMTIM2_lfps_lbpm_tpwm_max_clk_RegisterSize 9



#define LLPBMTIM2_Reserved_1_BitAddressOffset 18
#define LLPBMTIM2_Reserved_1_RegisterSize 14





#define LLPBMTXTIM_0 (DWC_usb31_block_link_BaseAddress + 0x44)
#define LLPBMTXTIM_RegisterSize 32
#define LLPBMTXTIM_RegisterResetValue 0x1130c850
#define LLPBMTXTIM_RegisterResetMask 0x3ff3ffff





#define LLPBMTXTIM_lfps_lbps_burst0_clk_BitAddressOffset 0
#define LLPBMTXTIM_lfps_lbps_burst0_clk_RegisterSize 8



#define LLPBMTXTIM_lfps_lbps_burst1_clk_BitAddressOffset 8
#define LLPBMTXTIM_lfps_lbps_burst1_clk_RegisterSize 10



#define LLPBMTXTIM_Reserved_1_BitAddressOffset 18
#define LLPBMTXTIM_Reserved_1_RegisterSize 2



#define LLPBMTXTIM_lfps_lpbs_tpwm_clk_BitAddressOffset 20
#define LLPBMTXTIM_lfps_lpbs_tpwm_clk_RegisterSize 10



#define LLPBMTXTIM_Reserved_2_BitAddressOffset 30
#define LLPBMTXTIM_Reserved_2_RegisterSize 2





#define LLINKERRINJ_0 (DWC_usb31_block_link_BaseAddress + 0x48)
#define LLINKERRINJ_RegisterSize 32
#define LLINKERRINJ_RegisterResetValue 0x0
#define LLINKERRINJ_RegisterResetMask 0xffffffff





#define LLINKERRINJ_RX_CRC5_BitAddressOffset 0
#define LLINKERRINJ_RX_CRC5_RegisterSize 1



#define LLINKERRINJ_RX_CRC16_BitAddressOffset 1
#define LLINKERRINJ_RX_CRC16_RegisterSize 1



#define LLINKERRINJ_RX_CRC32_BitAddressOffset 2
#define LLINKERRINJ_RX_CRC32_RegisterSize 1



#define LLINKERRINJ_RX_one_frame_sybl_BitAddressOffset 3
#define LLINKERRINJ_RX_one_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_all_frame_sybl_BitAddressOffset 4
#define LLINKERRINJ_RX_all_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_one_end_frame_sybl_BitAddressOffset 5
#define LLINKERRINJ_RX_one_end_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_all_end_frame_sybl_BitAddressOffset 6
#define LLINKERRINJ_RX_all_end_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_one_srt_frame_sybl_BitAddressOffset 7
#define LLINKERRINJ_RX_one_srt_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_all_start_frame_sybl_BitAddressOffset 8
#define LLINKERRINJ_RX_all_start_frame_sybl_RegisterSize 1



#define LLINKERRINJ_RX_LFR_BitAddressOffset 9
#define LLINKERRINJ_RX_LFR_RegisterSize 1



#define LLINKERRINJ_RX_TSEQ_BitAddressOffset 10
#define LLINKERRINJ_RX_TSEQ_RegisterSize 1



#define LLINKERRINJ_RX_TS1_BitAddressOffset 11
#define LLINKERRINJ_RX_TS1_RegisterSize 1



#define LLINKERRINJ_RX_TS2_BitAddressOffset 12
#define LLINKERRINJ_RX_TS2_RegisterSize 1



#define LLINKERRINJ_RX_insert_delay_BitAddressOffset 13
#define LLINKERRINJ_RX_insert_delay_RegisterSize 1



#define LLINKERRINJ_RX_insert_defer_BitAddressOffset 14
#define LLINKERRINJ_RX_insert_defer_RegisterSize 1



#define LLINKERRINJ_Rx_pipe_rxdata_BitAddressOffset 15
#define LLINKERRINJ_Rx_pipe_rxdata_RegisterSize 1



#define LLINKERRINJ_TX_CRC5_BitAddressOffset 16
#define LLINKERRINJ_TX_CRC5_RegisterSize 1



#define LLINKERRINJ_TX_CRC16_BitAddressOffset 17
#define LLINKERRINJ_TX_CRC16_RegisterSize 1



#define LLINKERRINJ_TX_CRC32_BitAddressOffset 18
#define LLINKERRINJ_TX_CRC32_RegisterSize 1



#define LLINKERRINJ_TX_one_frame_sybl_BitAddressOffset 19
#define LLINKERRINJ_TX_one_frame_sybl_RegisterSize 1



#define LLINKERRINJ_TX_all_frame_sybl_BitAddressOffset 20
#define LLINKERRINJ_TX_all_frame_sybl_RegisterSize 1



#define LLINKERRINJ_TX_one_end_frame_sybl_BitAddressOffset 21
#define LLINKERRINJ_TX_one_end_frame_sybl_RegisterSize 1



#define LLINKERRINJ_TX_all_end_frame_sybl_BitAddressOffset 22
#define LLINKERRINJ_TX_all_end_frame_sybl_RegisterSize 1



#define LLINKERRINJ_TX_one_srt_frame_sybl_BitAddressOffset 23
#define LLINKERRINJ_TX_one_srt_frame_sybl_RegisterSize 1



#define LLINKERRINJ_TX_delay_credit_rl_BitAddressOffset 24
#define LLINKERRINJ_TX_delay_credit_rl_RegisterSize 1



#define LLINKERRINJ_TX_LFR_BitAddressOffset 25
#define LLINKERRINJ_TX_LFR_RegisterSize 1



#define LLINKERRINJ_TX_TSEQ_BitAddressOffset 26
#define LLINKERRINJ_TX_TSEQ_RegisterSize 1



#define LLINKERRINJ_TX_TS1_BitAddressOffset 27
#define LLINKERRINJ_TX_TS1_RegisterSize 1



#define LLINKERRINJ_TX_TS2_BitAddressOffset 28
#define LLINKERRINJ_TX_TS2_RegisterSize 1



#define LLINKERRINJ_TX_insert_delay_BitAddressOffset 29
#define LLINKERRINJ_TX_insert_delay_RegisterSize 1



#define LLINKERRINJ_TX_insert_defer_BitAddressOffset 30
#define LLINKERRINJ_TX_insert_defer_RegisterSize 1



#define LLINKERRINJ_pipe_txdata_BitAddressOffset 31
#define LLINKERRINJ_pipe_txdata_RegisterSize 1





#define LLINKERRINJEN_0 (DWC_usb31_block_link_BaseAddress + 0x4c)
#define LLINKERRINJEN_RegisterSize 32
#define LLINKERRINJEN_RegisterResetValue 0x0
#define LLINKERRINJEN_RegisterResetMask 0xffffffff





#define LLINKERRINJEN_B2B_err_cnt_BitAddressOffset 0
#define LLINKERRINJEN_B2B_err_cnt_RegisterSize 16



#define LLINKERRINJEN_disable_inj_err_cnt_BitAddressOffset 16
#define LLINKERRINJEN_disable_inj_err_cnt_RegisterSize 16





#define GDBGLTSSM_0 (DWC_usb31_block_link_BaseAddress + 0x50)
#define GDBGLTSSM_RegisterSize 32
#define GDBGLTSSM_RegisterResetValue 0x41010440
#define GDBGLTSSM_RegisterResetMask 0xffffffff





#define GDBGLTSSM_TXONESZEROS_BitAddressOffset 0
#define GDBGLTSSM_TXONESZEROS_RegisterSize 1



#define GDBGLTSSM_RXTERMINATION_BitAddressOffset 1
#define GDBGLTSSM_RXTERMINATION_RegisterSize 1



#define GDBGLTSSM_TXSWING_BitAddressOffset 2
#define GDBGLTSSM_TXSWING_RegisterSize 1



#define GDBGLTSSM_LTDBClkState_BitAddressOffset 3
#define GDBGLTSSM_LTDBClkState_RegisterSize 3



#define GDBGLTSSM_TXDEEMPHASIS_BitAddressOffset 6
#define GDBGLTSSM_TXDEEMPHASIS_RegisterSize 2



#define GDBGLTSSM_RXEQTRAIN_BitAddressOffset 8
#define GDBGLTSSM_RXEQTRAIN_RegisterSize 1



#define GDBGLTSSM_POWERDOWN_BitAddressOffset 9
#define GDBGLTSSM_POWERDOWN_RegisterSize 2



#define GDBGLTSSM_LTDBPhyCmdState_BitAddressOffset 11
#define GDBGLTSSM_LTDBPhyCmdState_RegisterSize 3



#define GDBGLTSSM_TxDetRxLoopback_BitAddressOffset 14
#define GDBGLTSSM_TxDetRxLoopback_RegisterSize 1



#define GDBGLTSSM_RXPOLARITY_BitAddressOffset 15
#define GDBGLTSSM_RXPOLARITY_RegisterSize 1



#define GDBGLTSSM_TXELECLDLE_BitAddressOffset 16
#define GDBGLTSSM_TXELECLDLE_RegisterSize 1



#define GDBGLTSSM_ELASTICBUFFERMODE_BitAddressOffset 17
#define GDBGLTSSM_ELASTICBUFFERMODE_RegisterSize 1



#define GDBGLTSSM_LTDBSUBSTATE_BitAddressOffset 18
#define GDBGLTSSM_LTDBSUBSTATE_RegisterSize 4



#define GDBGLTSSM_LTDBLINKSTATE_BitAddressOffset 22
#define GDBGLTSSM_LTDBLINKSTATE_RegisterSize 4



#define GDBGLTSSM_LTDBTIMEOUT_BitAddressOffset 26
#define GDBGLTSSM_LTDBTIMEOUT_RegisterSize 1



#define GDBGLTSSM_reserved1_BitAddressOffset 27
#define GDBGLTSSM_reserved1_RegisterSize 3



#define GDBGLTSSM_RxElecidle_BitAddressOffset 30
#define GDBGLTSSM_RxElecidle_RegisterSize 1



#define GDBGLTSSM_reserved_31_31_BitAddressOffset 31
#define GDBGLTSSM_reserved_31_31_RegisterSize 1





#define GDBGLNMCC_0 (DWC_usb31_block_link_BaseAddress + 0x54)
#define GDBGLNMCC_RegisterSize 32
#define GDBGLNMCC_RegisterResetValue 0x0
#define GDBGLNMCC_RegisterResetMask 0xffffffff





#define GDBGLNMCC_LNMCC_LERC_BitAddressOffset 0
#define GDBGLNMCC_LNMCC_LERC_RegisterSize 9



#define GDBGLNMCC_reserved_16_9_BitAddressOffset 9
#define GDBGLNMCC_reserved_16_9_RegisterSize 7



#define GDBGLNMCC_LNMCC_LSERC_BitAddressOffset 16
#define GDBGLNMCC_LNMCC_LSERC_RegisterSize 16





#define LLINKDBGCTRL_0 (DWC_usb31_block_link_BaseAddress + 0x58)
#define LLINKDBGCTRL_RegisterSize 32
#define LLINKDBGCTRL_RegisterResetValue 0x0
#define LLINKDBGCTRL_RegisterResetMask 0xffffffff





#define LLINKDBGCTRL_txfifo_number_BitAddressOffset 0
#define LLINKDBGCTRL_txfifo_number_RegisterSize 4



#define LLINKDBGCTRL_rxfifo_number_BitAddressOffset 4
#define LLINKDBGCTRL_rxfifo_number_RegisterSize 4



#define LLINKDBGCTRL_peri_rsc_txfifo_number_BitAddressOffset 8
#define LLINKDBGCTRL_peri_rsc_txfifo_number_RegisterSize 4



#define LLINKDBGCTRL_peri_rsc_rxfifo_number_BitAddressOffset 12
#define LLINKDBGCTRL_peri_rsc_rxfifo_number_RegisterSize 4



#define LLINKDBGCTRL_link_state_trigger_BitAddressOffset 16
#define LLINKDBGCTRL_link_state_trigger_RegisterSize 4



#define LLINKDBGCTRL_sub_state_trigger_BitAddressOffset 20
#define LLINKDBGCTRL_sub_state_trigger_RegisterSize 4



#define LLINKDBGCTRL_pipe_phystatus_trigger_BitAddressOffset 24
#define LLINKDBGCTRL_pipe_phystatus_trigger_RegisterSize 1



#define LLINKDBGCTRL_pipe_rxlecidle_trigger_BitAddressOffset 25
#define LLINKDBGCTRL_pipe_rxlecidle_trigger_RegisterSize 1



#define LLINKDBGCTRL_pipe_txdetectrxlb_trigger_BitAddressOffset 26
#define LLINKDBGCTRL_pipe_txdetectrxlb_trigger_RegisterSize 1



#define LLINKDBGCTRL_link_state_trigger_scrambled_BitAddressOffset 27
#define LLINKDBGCTRL_link_state_trigger_scrambled_RegisterSize 1



#define LLINKDBGCTRL_link_state_trigger_descrambled_BitAddressOffset 28
#define LLINKDBGCTRL_link_state_trigger_descrambled_RegisterSize 1



#define LLINKDBGCTRL_trigger_start_sts_BitAddressOffset 29
#define LLINKDBGCTRL_trigger_start_sts_RegisterSize 1



#define LLINKDBGCTRL_trigger_end_sts_BitAddressOffset 30
#define LLINKDBGCTRL_trigger_end_sts_RegisterSize 1



#define LLINKDBGCTRL_retry_DP_BitAddressOffset 31
#define LLINKDBGCTRL_retry_DP_RegisterSize 1





#define LLINKDBGCNTTRIG_0 (DWC_usb31_block_link_BaseAddress + 0x5c)
#define LLINKDBGCNTTRIG_RegisterSize 32
#define LLINKDBGCNTTRIG_RegisterResetValue 0x0
#define LLINKDBGCNTTRIG_RegisterResetMask 0xffffffff





#define LLINKDBGCNTTRIG_srt_cnt_BitAddressOffset 0
#define LLINKDBGCNTTRIG_srt_cnt_RegisterSize 16



#define LLINKDBGCNTTRIG_stp_cnt_BitAddressOffset 16
#define LLINKDBGCNTTRIG_stp_cnt_RegisterSize 16





#define LCSR_TX_DEEMPH_0 (DWC_usb31_block_link_BaseAddress + 0x60)
#define LCSR_TX_DEEMPH_RegisterSize 32
#define LCSR_TX_DEEMPH_RegisterResetValue 0x34c2
#define LCSR_TX_DEEMPH_RegisterResetMask 0xffffffff





#define LCSR_TX_DEEMPH_csr_tx_deemph_field_1_BitAddressOffset 0
#define LCSR_TX_DEEMPH_csr_tx_deemph_field_1_RegisterSize 18



#define LCSR_TX_DEEMPH_Reserved_31_18_BitAddressOffset 18
#define LCSR_TX_DEEMPH_Reserved_31_18_RegisterSize 14





#define LCSR_TX_DEEMPH_1_0 (DWC_usb31_block_link_BaseAddress + 0x64)
#define LCSR_TX_DEEMPH_1_RegisterSize 32
#define LCSR_TX_DEEMPH_1_RegisterResetValue 0x582
#define LCSR_TX_DEEMPH_1_RegisterResetMask 0xffffffff





#define LCSR_TX_DEEMPH_1_csr_tx_deemph_1_field_1_BitAddressOffset 0
#define LCSR_TX_DEEMPH_1_csr_tx_deemph_1_field_1_RegisterSize 18



#define LCSR_TX_DEEMPH_1_Reserved_31_18_BitAddressOffset 18
#define LCSR_TX_DEEMPH_1_Reserved_31_18_RegisterSize 14





#define LCSR_TX_DEEMPH_2_0 (DWC_usb31_block_link_BaseAddress + 0x68)
#define LCSR_TX_DEEMPH_2_RegisterSize 32
#define LCSR_TX_DEEMPH_2_RegisterResetValue 0x3540
#define LCSR_TX_DEEMPH_2_RegisterResetMask 0xffffffff





#define LCSR_TX_DEEMPH_2_csr_tx_deemph_2_field_1_BitAddressOffset 0
#define LCSR_TX_DEEMPH_2_csr_tx_deemph_2_field_1_RegisterSize 18



#define LCSR_TX_DEEMPH_2_Reserved_31_18_BitAddressOffset 18
#define LCSR_TX_DEEMPH_2_Reserved_31_18_RegisterSize 14





#define LCSR_TX_DEEMPH_3_0 (DWC_usb31_block_link_BaseAddress + 0x6c)
#define LCSR_TX_DEEMPH_3_RegisterSize 32
#define LCSR_TX_DEEMPH_3_RegisterResetValue 0x600
#define LCSR_TX_DEEMPH_3_RegisterResetMask 0xffffffff





#define LCSR_TX_DEEMPH_3_csr_tx_deemph_3_field_1_BitAddressOffset 0
#define LCSR_TX_DEEMPH_3_csr_tx_deemph_3_field_1_RegisterSize 18



#define LCSR_TX_DEEMPH_3_Reserved_31_18_BitAddressOffset 18
#define LCSR_TX_DEEMPH_3_Reserved_31_18_RegisterSize 14





#define LCSRPTMDEBUG1_0 (DWC_usb31_block_link_BaseAddress + 0x70)
#define LCSRPTMDEBUG1_RegisterSize 32
#define LCSRPTMDEBUG1_RegisterResetValue 0x0
#define LCSRPTMDEBUG1_RegisterResetMask 0xffffffff





#define LCSRPTMDEBUG1_LDM_T1_BitAddressOffset 0
#define LCSRPTMDEBUG1_LDM_T1_RegisterSize 17



#define LCSRPTMDEBUG1_LDM_T32_BitAddressOffset 17
#define LCSRPTMDEBUG1_LDM_T32_RegisterSize 13



#define LCSRPTMDEBUG1_RESERVED_31_30_BitAddressOffset 30
#define LCSRPTMDEBUG1_RESERVED_31_30_RegisterSize 2





#define LCSRPTMDEBUG2_0 (DWC_usb31_block_link_BaseAddress + 0x74)
#define LCSRPTMDEBUG2_RegisterSize 32
#define LCSRPTMDEBUG2_RegisterResetValue 0x0
#define LCSRPTMDEBUG2_RegisterResetMask 0xffffffff





#define LCSRPTMDEBUG2_LDM_T4_BitAddressOffset 0
#define LCSRPTMDEBUG2_LDM_T4_RegisterSize 17



#define LCSRPTMDEBUG2_RESERVED_31_17_BitAddressOffset 17
#define LCSRPTMDEBUG2_RESERVED_31_17_RegisterSize 15





#define LPTMDPDELAY2_0 (DWC_usb31_block_link_BaseAddress + 0x78)
#define LPTMDPDELAY2_RegisterSize 32
#define LPTMDPDELAY2_RegisterResetValue 0x51565
#define LPTMDPDELAY2_RegisterResetMask 0xfffff





#define LPTMDPDELAY2_link_tx_delay_gen1_BitAddressOffset 0
#define LPTMDPDELAY2_link_tx_delay_gen1_RegisterSize 5



#define LPTMDPDELAY2_link_rx_delay_gen1_BitAddressOffset 5
#define LPTMDPDELAY2_link_rx_delay_gen1_RegisterSize 5



#define LPTMDPDELAY2_link_tx_delay_gen2_BitAddressOffset 10
#define LPTMDPDELAY2_link_tx_delay_gen2_RegisterSize 5



#define LPTMDPDELAY2_link_rx_delay_gen2_BitAddressOffset 15
#define LPTMDPDELAY2_link_rx_delay_gen2_RegisterSize 5



#define LPTMDPDELAY2_Reserved_1_BitAddressOffset 20
#define LPTMDPDELAY2_Reserved_1_RegisterSize 12



#define DWC_usb31_block_debug_BaseAddress 0xd800



#define RHBDBG_REGS (DWC_usb31_block_debug_BaseAddress + 0x0)
#define BU31RHBDBG_RegisterSize 32
#define BU31RHBDBG_RegisterResetValue 0x8
#define BU31RHBDBG_RegisterResetMask 0x40f





#define BU31RHBDBG_ovrcur_BitAddressOffset 0
#define BU31RHBDBG_ovrcur_RegisterSize 1



#define BU31RHBDBG_pcap_BitAddressOffset 1
#define BU31RHBDBG_pcap_RegisterSize 2



#define BU31RHBDBG_tpcfg_tout_ctrl_BitAddressOffset 3
#define BU31RHBDBG_tpcfg_tout_ctrl_RegisterSize 1



#define BU31RHBDBG_reserved_9_4_BitAddressOffset 4
#define BU31RHBDBG_reserved_9_4_RegisterSize 6



#define BU31RHBDBG_cfgwt_dis_BitAddressOffset 10
#define BU31RHBDBG_cfgwt_dis_RegisterSize 1



#define BU31RHBDBG_Reserved_31_11_BitAddressOffset 11
#define BU31RHBDBG_Reserved_31_11_RegisterSize 21





#define BU31RHBDBG_0 (DWC_usb31_block_debug_BaseAddress + 0x0)
#define BU31RHBDBG_RegisterSize 32
#define BU31RHBDBG_RegisterResetValue 0x8
#define BU31RHBDBG_RegisterResetMask 0x40f





#define BU31RHBDBG_ovrcur_BitAddressOffset 0
#define BU31RHBDBG_ovrcur_RegisterSize 1



#define BU31RHBDBG_pcap_BitAddressOffset 1
#define BU31RHBDBG_pcap_RegisterSize 2



#define BU31RHBDBG_tpcfg_tout_ctrl_BitAddressOffset 3
#define BU31RHBDBG_tpcfg_tout_ctrl_RegisterSize 1



#define BU31RHBDBG_reserved_9_4_BitAddressOffset 4
#define BU31RHBDBG_reserved_9_4_RegisterSize 6



#define BU31RHBDBG_cfgwt_dis_BitAddressOffset 10
#define BU31RHBDBG_cfgwt_dis_RegisterSize 1



#define BU31RHBDBG_Reserved_31_11_BitAddressOffset 11
#define BU31RHBDBG_Reserved_31_11_RegisterSize 21





#define BRAMHIADDR (DWC_usb31_block_debug_BaseAddress + 0x4c)
#define BRAMHIADDR_RegisterSize 32
#define BRAMHIADDR_RegisterResetValue 0x0
#define BRAMHIADDR_RegisterResetMask 0x1ff000





#define BRAMHIADDR_Reserved_1_BitAddressOffset 0
#define BRAMHIADDR_Reserved_1_RegisterSize 12



#define BRAMHIADDR_ramhiaddr_BitAddressOffset 12
#define BRAMHIADDR_ramhiaddr_RegisterSize 6



#define BRAMHIADDR_ram_select_BitAddressOffset 18
#define BRAMHIADDR_ram_select_RegisterSize 3



#define BRAMHIADDR_Reserved_2_BitAddressOffset 21
#define BRAMHIADDR_Reserved_2_RegisterSize 11





#define BRSERRCNT (DWC_usb31_block_debug_BaseAddress + 0x50)
#define BRSERRCNT_RegisterSize 32
#define BRSERRCNT_RegisterResetValue 0x0
#define BRSERRCNT_RegisterResetMask 0x0





#define BRSERRCNT_ram0serrcnt_BitAddressOffset 0
#define BRSERRCNT_ram0serrcnt_RegisterSize 6



#define BRSERRCNT_ram1serrcnt_BitAddressOffset 6
#define BRSERRCNT_ram1serrcnt_RegisterSize 6



#define BRSERRCNT_ram2serrcnt_BitAddressOffset 12
#define BRSERRCNT_ram2serrcnt_RegisterSize 6



#define BRSERRCNT_ram3serrcnt_BitAddressOffset 18
#define BRSERRCNT_ram3serrcnt_RegisterSize 6



#define BRSERRCNT_ram4serrcnt_BitAddressOffset 24
#define BRSERRCNT_ram4serrcnt_RegisterSize 6



#define BRSERRCNT_Reserved_1_BitAddressOffset 30
#define BRSERRCNT_Reserved_1_RegisterSize 2





#define BRMERRCNT (DWC_usb31_block_debug_BaseAddress + 0x54)
#define BRMERRCNT_RegisterSize 32
#define BRMERRCNT_RegisterResetValue 0x0
#define BRMERRCNT_RegisterResetMask 0x0





#define BRMERRCNT_ram0merrcnt_BitAddressOffset 0
#define BRMERRCNT_ram0merrcnt_RegisterSize 6



#define BRMERRCNT_ram1merrcnt_BitAddressOffset 6
#define BRMERRCNT_ram1merrcnt_RegisterSize 6



#define BRMERRCNT_ram2merrcnt_BitAddressOffset 12
#define BRMERRCNT_ram2merrcnt_RegisterSize 6



#define BRMERRCNT_ram3merrcnt_BitAddressOffset 18
#define BRMERRCNT_ram3merrcnt_RegisterSize 6



#define BRMERRCNT_ram4merrcnt_BitAddressOffset 24
#define BRMERRCNT_ram4merrcnt_RegisterSize 6



#define BRMERRCNT_Reserved_1_BitAddressOffset 30
#define BRMERRCNT_Reserved_1_RegisterSize 2





#define BRAMECCERR (DWC_usb31_block_debug_BaseAddress + 0x58)
#define BRAMECCERR_RegisterSize 32
#define BRAMECCERR_RegisterResetValue 0x0
#define BRAMECCERR_RegisterResetMask 0x0





#define BRAMECCERR_rammerrvec_BitAddressOffset 0
#define BRAMECCERR_rammerrvec_RegisterSize 5



#define BRAMECCERR_ramserrvec_BitAddressOffset 5
#define BRAMECCERR_ramserrvec_RegisterSize 5



#define BRAMECCERR_rammerr_BitAddressOffset 10
#define BRAMECCERR_rammerr_RegisterSize 1



#define BRAMECCERR_ramserr_BitAddressOffset 11
#define BRAMECCERR_ramserr_RegisterSize 1



#define BRAMECCERR_reserved_31_12_BitAddressOffset 12
#define BRAMECCERR_reserved_31_12_RegisterSize 20





#define BRERRCTL (DWC_usb31_block_debug_BaseAddress + 0x5c)
#define BRERRCTL_RegisterSize 32
#define BRERRCTL_RegisterResetValue 0x0
#define BRERRCTL_RegisterResetMask 0x3





#define BRERRCTL_rmerrclr_BitAddressOffset 0
#define BRERRCTL_rmerrclr_RegisterSize 1



#define BRERRCTL_rserrclr_BitAddressOffset 1
#define BRERRCTL_rserrclr_RegisterSize 1



#define BRERRCTL_Reserved_1_BitAddressOffset 2
#define BRERRCTL_Reserved_1_RegisterSize 30





#define BRAM0ADDRERR (DWC_usb31_block_debug_BaseAddress + 0x60)
#define BRAM0ADDRERR_RegisterSize 32
#define BRAM0ADDRERR_RegisterResetValue 0x0
#define BRAM0ADDRERR_RegisterResetMask 0xffff





#define BRAM0ADDRERR_ram0errloc_BitAddressOffset 0
#define BRAM0ADDRERR_ram0errloc_RegisterSize 16



#define BRAM0ADDRERR_Reserved_1_BitAddressOffset 16
#define BRAM0ADDRERR_Reserved_1_RegisterSize 16





#define BRAM1ADDRERR (DWC_usb31_block_debug_BaseAddress + 0x64)
#define BRAM1ADDRERR_RegisterSize 32
#define BRAM1ADDRERR_RegisterResetValue 0x0
#define BRAM1ADDRERR_RegisterResetMask 0xffff





#define BRAM1ADDRERR_ram1errloc_BitAddressOffset 0
#define BRAM1ADDRERR_ram1errloc_RegisterSize 16



#define BRAM1ADDRERR_Reserved_1_BitAddressOffset 16
#define BRAM1ADDRERR_Reserved_1_RegisterSize 16





#define BRAM2ADDRERR (DWC_usb31_block_debug_BaseAddress + 0x68)
#define BRAM2ADDRERR_RegisterSize 32
#define BRAM2ADDRERR_RegisterResetValue 0x0
#define BRAM2ADDRERR_RegisterResetMask 0xffff





#define BRAM2ADDRERR_ram2errloc_BitAddressOffset 0
#define BRAM2ADDRERR_ram2errloc_RegisterSize 16



#define BRAM2ADDRERR_Reserved_1_BitAddressOffset 16
#define BRAM2ADDRERR_Reserved_1_RegisterSize 16





#define BRAM3ADDRERR (DWC_usb31_block_debug_BaseAddress + 0x6c)
#define BRAM3ADDRERR_RegisterSize 32
#define BRAM3ADDRERR_RegisterResetValue 0x0
#define BRAM3ADDRERR_RegisterResetMask 0xffff





#define BRAM3ADDRERR_ram3errloc_BitAddressOffset 0
#define BRAM3ADDRERR_ram3errloc_RegisterSize 16



#define BRAM3ADDRERR_Reserved_1_BitAddressOffset 16
#define BRAM3ADDRERR_Reserved_1_RegisterSize 16





#define BRAM4ADDRERR (DWC_usb31_block_debug_BaseAddress + 0x70)
#define BRAM4ADDRERR_RegisterSize 32
#define BRAM4ADDRERR_RegisterResetValue 0x0
#define BRAM4ADDRERR_RegisterResetMask 0xffff





#define BRAM4ADDRERR_ram4errloc_BitAddressOffset 0
#define BRAM4ADDRERR_ram4errloc_RegisterSize 16



#define BRAM4ADDRERR_Reserved_1_BitAddressOffset 16
#define BRAM4ADDRERR_Reserved_1_RegisterSize 16





#define BLOOPBCKCTRL (DWC_usb31_block_debug_BaseAddress + 0x100)
#define BLOOPBCKCTRL_RegisterSize 32
#define BLOOPBCKCTRL_RegisterResetValue 0x0
#define BLOOPBCKCTRL_RegisterResetMask 0xffffffff





#define BLOOPBCKCTRL_loopback_mode_en_BitAddressOffset 0
#define BLOOPBCKCTRL_loopback_mode_en_RegisterSize 1



#define BLOOPBCKCTRL_loopback_mode_BitAddressOffset 1
#define BLOOPBCKCTRL_loopback_mode_RegisterSize 1



#define BLOOPBCKCTRL_loopback_level_BitAddressOffset 2
#define BLOOPBCKCTRL_loopback_level_RegisterSize 2



#define BLOOPBCKCTRL_loopback_prtnum_BitAddressOffset 4
#define BLOOPBCKCTRL_loopback_prtnum_RegisterSize 4



#define BLOOPBCKCTRL_reserved_10_8_BitAddressOffset 8
#define BLOOPBCKCTRL_reserved_10_8_RegisterSize 3



#define BLOOPBCKCTRL_reserved_31_11_BitAddressOffset 11
#define BLOOPBCKCTRL_reserved_31_11_RegisterSize 21





#define BLOOPBCKTFERSZ (DWC_usb31_block_debug_BaseAddress + 0x104)
#define BLOOPBCKTFERSZ_RegisterSize 32
#define BLOOPBCKTFERSZ_RegisterResetValue 0x0
#define BLOOPBCKTFERSZ_RegisterResetMask 0xffffffff





#define BLOOPBCKTFERSZ_loopback_xfer_sz_BitAddressOffset 0
#define BLOOPBCKTFERSZ_loopback_xfer_sz_RegisterSize 24



#define BLOOPBCKTFERSZ_reserved_31_24_BitAddressOffset 24
#define BLOOPBCKTFERSZ_reserved_31_24_RegisterSize 8





#define BBISTDATAPATSEED (DWC_usb31_block_debug_BaseAddress + 0x108)
#define BBISTDATAPATSEED_RegisterSize 32
#define BBISTDATAPATSEED_RegisterResetValue 0x0
#define BBISTDATAPATSEED_RegisterResetMask 0xffffffff





#define BBISTDATAPATSEED_BIST_pattern_seed_BitAddressOffset 0
#define BBISTDATAPATSEED_BIST_pattern_seed_RegisterSize 32





#define BBISTCTRL (DWC_usb31_block_debug_BaseAddress + 0x10c)
#define BBISTCTRL_RegisterSize 32
#define BBISTCTRL_RegisterResetValue 0x0
#define BBISTCTRL_RegisterResetMask 0xffffffff





#define BBISTCTRL_BIST_iteration_cnt_BitAddressOffset 0
#define BBISTCTRL_BIST_iteration_cnt_RegisterSize 24



#define BBISTCTRL_BIST_pattern_BitAddressOffset 24
#define BBISTCTRL_BIST_pattern_RegisterSize 3



#define BBISTCTRL_reserved_28_27_BitAddressOffset 27
#define BBISTCTRL_reserved_28_27_RegisterSize 2



#define BBISTCTRL_fail_BitAddressOffset 29
#define BBISTCTRL_fail_RegisterSize 1



#define BBISTCTRL_BIST_test_type_BitAddressOffset 30
#define BBISTCTRL_BIST_test_type_RegisterSize 1



#define BBISTCTRL_start_BIST_test_BitAddressOffset 31
#define BBISTCTRL_start_BIST_test_RegisterSize 1





#define BBISTXFERSTS0 (DWC_usb31_block_debug_BaseAddress + 0x110)
#define BBISTXFERSTS0_RegisterSize 32
#define BBISTXFERSTS0_RegisterResetValue 0x0
#define BBISTXFERSTS0_RegisterResetMask 0xffffff





#define BBISTXFERSTS0_bist_pending_trans_size_BitAddressOffset 0
#define BBISTXFERSTS0_bist_pending_trans_size_RegisterSize 24



#define BBISTXFERSTS0_reserved_31_24_BitAddressOffset 24
#define BBISTXFERSTS0_reserved_31_24_RegisterSize 8





#define BBISTXFERSTS1 (DWC_usb31_block_debug_BaseAddress + 0x114)
#define BBISTXFERSTS1_RegisterSize 32
#define BBISTXFERSTS1_RegisterResetValue 0x0
#define BBISTXFERSTS1_RegisterResetMask 0xffffff





#define BBISTXFERSTS1_failed_pending_trans_size_BitAddressOffset 0
#define BBISTXFERSTS1_failed_pending_trans_size_RegisterSize 24



#define BBISTXFERSTS1_reserved_31_24_BitAddressOffset 24
#define BBISTXFERSTS1_reserved_31_24_RegisterSize 8





#define BBISTXFERSTS2 (DWC_usb31_block_debug_BaseAddress + 0x118)
#define BBISTXFERSTS2_RegisterSize 32
#define BBISTXFERSTS2_RegisterResetValue 0x0
#define BBISTXFERSTS2_RegisterResetMask 0xffffff





#define BBISTXFERSTS2_failed_iteration_BitAddressOffset 0
#define BBISTXFERSTS2_failed_iteration_RegisterSize 24



#define BBISTXFERSTS2_reserved_31_24_BitAddressOffset 24
#define BBISTXFERSTS2_reserved_31_24_RegisterSize 8





#define BBISTXFERSTS3 (DWC_usb31_block_debug_BaseAddress + 0x11c)
#define BBISTXFERSTS3_RegisterSize 32
#define BBISTXFERSTS3_RegisterResetValue 0x0
#define BBISTXFERSTS3_RegisterResetMask 0xf





#define BBISTXFERSTS3_loopback_statemachine_BitAddressOffset 0
#define BBISTXFERSTS3_loopback_statemachine_RegisterSize 4



#define BBISTXFERSTS3_reserved_31_4_BitAddressOffset 4
#define BBISTXFERSTS3_reserved_31_4_RegisterSize 28





#define BBISTEXPDATASTS0 (DWC_usb31_block_debug_BaseAddress + 0x120)
#define BBISTEXPDATASTS0_RegisterSize 32
#define BBISTEXPDATASTS0_RegisterResetValue 0x0
#define BBISTEXPDATASTS0_RegisterResetMask 0xffffffff





#define BBISTEXPDATASTS0_exp_data_BitAddressOffset 0
#define BBISTEXPDATASTS0_exp_data_RegisterSize 32





#define BBISTEXPDATASTS1 (DWC_usb31_block_debug_BaseAddress + 0x124)
#define BBISTEXPDATASTS1_RegisterSize 32
#define BBISTEXPDATASTS1_RegisterResetValue 0x0
#define BBISTEXPDATASTS1_RegisterResetMask 0xffffffff





#define BBISTEXPDATASTS1_exp_data_BitAddressOffset 0
#define BBISTEXPDATASTS1_exp_data_RegisterSize 32





#define BBISTEXPDATASTS2 (DWC_usb31_block_debug_BaseAddress + 0x128)
#define BBISTEXPDATASTS2_RegisterSize 32
#define BBISTEXPDATASTS2_RegisterResetValue 0x0
#define BBISTEXPDATASTS2_RegisterResetMask 0xffffffff





#define BBISTEXPDATASTS2_exp_data_BitAddressOffset 0
#define BBISTEXPDATASTS2_exp_data_RegisterSize 32





#define BBISTEXPDATASTS3 (DWC_usb31_block_debug_BaseAddress + 0x12c)
#define BBISTEXPDATASTS3_RegisterSize 32
#define BBISTEXPDATASTS3_RegisterResetValue 0x0
#define BBISTEXPDATASTS3_RegisterResetMask 0xffffffff





#define BBISTEXPDATASTS3_exp_data_BitAddressOffset 0
#define BBISTEXPDATASTS3_exp_data_RegisterSize 32





#define BBISTRCVDDATASTS0 (DWC_usb31_block_debug_BaseAddress + 0x130)
#define BBISTRCVDDATASTS0_RegisterSize 32
#define BBISTRCVDDATASTS0_RegisterResetValue 0x0
#define BBISTRCVDDATASTS0_RegisterResetMask 0xffffffff





#define BBISTRCVDDATASTS0_received_data_BitAddressOffset 0
#define BBISTRCVDDATASTS0_received_data_RegisterSize 32





#define BBISTRCVDDATASTS1 (DWC_usb31_block_debug_BaseAddress + 0x134)
#define BBISTRCVDDATASTS1_RegisterSize 32
#define BBISTRCVDDATASTS1_RegisterResetValue 0x0
#define BBISTRCVDDATASTS1_RegisterResetMask 0xffffffff





#define BBISTRCVDDATASTS1_received_data_BitAddressOffset 0
#define BBISTRCVDDATASTS1_received_data_RegisterSize 32





#define BBISTRCVDDATASTS2 (DWC_usb31_block_debug_BaseAddress + 0x138)
#define BBISTRCVDDATASTS2_RegisterSize 32
#define BBISTRCVDDATASTS2_RegisterResetValue 0x0
#define BBISTRCVDDATASTS2_RegisterResetMask 0xffffffff





#define BBISTRCVDDATASTS2_received_data_BitAddressOffset 0
#define BBISTRCVDDATASTS2_received_data_RegisterSize 32





#define BBISTRCVDDATASTS3 (DWC_usb31_block_debug_BaseAddress + 0x13c)
#define BBISTRCVDDATASTS3_RegisterSize 32
#define BBISTRCVDDATASTS3_RegisterResetValue 0x0
#define BBISTRCVDDATASTS3_RegisterResetMask 0xffffffff





#define BBISTRCVDDATASTS3_received_data_BitAddressOffset 0
#define BBISTRCVDDATASTS3_received_data_RegisterSize 32



#define DWC_usb31_block_rsvd_BaseAddress 0xda00



#define rsvd_reg (DWC_usb31_block_rsvd_BaseAddress + 0x0)
#define rsvd_reg_RegisterSize 32
#define rsvd_reg_RegisterResetValue 0x0
#define rsvd_reg_RegisterResetMask 0xffffffff





#define rsvd_reg_rsvd_field_BitAddressOffset 0
#define rsvd_reg_rsvd_field_RegisterSize 32



#define DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress 0x0



#define CAPLENGTH (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x0)
#define CAPLENGTH_RegisterSize 32
#define CAPLENGTH_RegisterResetValue 0x1200030
#define CAPLENGTH_RegisterResetMask 0xffff00ff





#define CAPLENGTH_CAPLENGTH_BitAddressOffset 0
#define CAPLENGTH_CAPLENGTH_RegisterSize 8



#define CAPLENGTH_reserved_15_8_BitAddressOffset 8
#define CAPLENGTH_reserved_15_8_RegisterSize 8



#define CAPLENGTH_HCIVERSION_BitAddressOffset 16
#define CAPLENGTH_HCIVERSION_RegisterSize 16





#define HCSPARAMS1 (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x4)
#define HCSPARAMS1_RegisterSize 32
#define HCSPARAMS1_RegisterResetValue 0x2000140
#define HCSPARAMS1_RegisterResetMask 0xff07ffff





#define HCSPARAMS1_MAXSLOTS_BitAddressOffset 0
#define HCSPARAMS1_MAXSLOTS_RegisterSize 8



#define HCSPARAMS1_MAXINTRS_BitAddressOffset 8
#define HCSPARAMS1_MAXINTRS_RegisterSize 11



#define HCSPARAMS1_reserved_23_19_BitAddressOffset 19
#define HCSPARAMS1_reserved_23_19_RegisterSize 5



#define HCSPARAMS1_MAXPORTS_BitAddressOffset 24
#define HCSPARAMS1_MAXPORTS_RegisterSize 8





#define HCSPARAMS2 (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x8)
#define HCSPARAMS2_RegisterSize 32
#define HCSPARAMS2_RegisterResetValue 0x140000f1
#define HCSPARAMS2_RegisterResetMask 0xffe000ff





#define HCSPARAMS2_IST_BitAddressOffset 0
#define HCSPARAMS2_IST_RegisterSize 4



#define HCSPARAMS2_ERSTMAX_BitAddressOffset 4
#define HCSPARAMS2_ERSTMAX_RegisterSize 4



#define HCSPARAMS2_reserved_20_8_BitAddressOffset 8
#define HCSPARAMS2_reserved_20_8_RegisterSize 13



#define HCSPARAMS2_MAXSCRATCHPADBUFS_HI_BitAddressOffset 21
#define HCSPARAMS2_MAXSCRATCHPADBUFS_HI_RegisterSize 5



#define HCSPARAMS2_SPR_BitAddressOffset 26
#define HCSPARAMS2_SPR_RegisterSize 1



#define HCSPARAMS2_MAXSCRATCHPADBUFS_BitAddressOffset 27
#define HCSPARAMS2_MAXSCRATCHPADBUFS_RegisterSize 5





#define HCSPARAMS3 (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0xc)
#define HCSPARAMS3_RegisterSize 32
#define HCSPARAMS3_RegisterResetValue 0x200000a
#define HCSPARAMS3_RegisterResetMask 0xffff00ff





#define HCSPARAMS3_U1_DEVICE_EXIT_LAT_BitAddressOffset 0
#define HCSPARAMS3_U1_DEVICE_EXIT_LAT_RegisterSize 8



#define HCSPARAMS3_reserved_15_8_BitAddressOffset 8
#define HCSPARAMS3_reserved_15_8_RegisterSize 8



#define HCSPARAMS3_U2_DEVICE_EXIT_LAT_BitAddressOffset 16
#define HCSPARAMS3_U2_DEVICE_EXIT_LAT_RegisterSize 16





#define HCCPARAMS1 (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x10)
#define HCCPARAMS1_RegisterSize 32
#define HCCPARAMS1_RegisterResetValue 0x118ffcd
#define HCCPARAMS1_RegisterResetMask 0xffffffff





#define HCCPARAMS1_AC64_BitAddressOffset 0
#define HCCPARAMS1_AC64_RegisterSize 1



#define HCCPARAMS1_BNC_BitAddressOffset 1
#define HCCPARAMS1_BNC_RegisterSize 1



#define HCCPARAMS1_CSZ_BitAddressOffset 2
#define HCCPARAMS1_CSZ_RegisterSize 1



#define HCCPARAMS1_PPC_BitAddressOffset 3
#define HCCPARAMS1_PPC_RegisterSize 1



#define HCCPARAMS1_PIND_BitAddressOffset 4
#define HCCPARAMS1_PIND_RegisterSize 1



#define HCCPARAMS1_LHRC_BitAddressOffset 5
#define HCCPARAMS1_LHRC_RegisterSize 1



#define HCCPARAMS1_LTC_BitAddressOffset 6
#define HCCPARAMS1_LTC_RegisterSize 1



#define HCCPARAMS1_NSS_BitAddressOffset 7
#define HCCPARAMS1_NSS_RegisterSize 1



#define HCCPARAMS1_PAE_BitAddressOffset 8
#define HCCPARAMS1_PAE_RegisterSize 1



#define HCCPARAMS1_SPC_BitAddressOffset 9
#define HCCPARAMS1_SPC_RegisterSize 1



#define HCCPARAMS1_SEC_BitAddressOffset 10
#define HCCPARAMS1_SEC_RegisterSize 1



#define HCCPARAMS1_CFC_BitAddressOffset 11
#define HCCPARAMS1_CFC_RegisterSize 1



#define HCCPARAMS1_MAXPSASIZE_BitAddressOffset 12
#define HCCPARAMS1_MAXPSASIZE_RegisterSize 4



#define HCCPARAMS1_XECP_BitAddressOffset 16
#define HCCPARAMS1_XECP_RegisterSize 16





#define DBOFF (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x14)
#define DBOFF_RegisterSize 32
#define DBOFF_RegisterResetValue 0x2000
#define DBOFF_RegisterResetMask 0xfffffffc





#define DBOFF_reserved_1_0_BitAddressOffset 0
#define DBOFF_reserved_1_0_RegisterSize 2



#define DBOFF_DOORBELL_ARRAY_OFFSET_BitAddressOffset 2
#define DBOFF_DOORBELL_ARRAY_OFFSET_RegisterSize 30





#define RTSOFF (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x18)
#define RTSOFF_RegisterSize 32
#define RTSOFF_RegisterResetValue 0x1000
#define RTSOFF_RegisterResetMask 0xffffffe0





#define RTSOFF_reserved_4_0_BitAddressOffset 0
#define RTSOFF_reserved_4_0_RegisterSize 5



#define RTSOFF_RUNTIME_REG_SPACE_OFFSET_BitAddressOffset 5
#define RTSOFF_RUNTIME_REG_SPACE_OFFSET_RegisterSize 27





#define HCCPARAMS2 (DWC_usb31_block_eXtensible_Host_Cntrl_Cap_Regs_BaseAddress + 0x1c)
#define HCCPARAMS2_RegisterSize 32
#define HCCPARAMS2_RegisterResetValue 0xff
#define HCCPARAMS2_RegisterResetMask 0xffffffff





#define HCCPARAMS2_U3C_BitAddressOffset 0
#define HCCPARAMS2_U3C_RegisterSize 1



#define HCCPARAMS2_CMC_BitAddressOffset 1
#define HCCPARAMS2_CMC_RegisterSize 1



#define HCCPARAMS2_FSC_BitAddressOffset 2
#define HCCPARAMS2_FSC_RegisterSize 1



#define HCCPARAMS2_CTC_BitAddressOffset 3
#define HCCPARAMS2_CTC_RegisterSize 1



#define HCCPARAMS2_LEC_BitAddressOffset 4
#define HCCPARAMS2_LEC_RegisterSize 1



#define HCCPARAMS2_CIC_BitAddressOffset 5
#define HCCPARAMS2_CIC_RegisterSize 1



#define HCCPARAMS2_ETC_BitAddressOffset 6
#define HCCPARAMS2_ETC_RegisterSize 1



#define HCCPARAMS2_ETC_TSC_BitAddressOffset 7
#define HCCPARAMS2_ETC_TSC_RegisterSize 1



#define HCCPARAMS2_reserved_8_BitAddressOffset 8
#define HCCPARAMS2_reserved_8_RegisterSize 1



#define HCCPARAMS2_VTC_BitAddressOffset 9
#define HCCPARAMS2_VTC_RegisterSize 1



#define HCCPARAMS2_reserved_31_10_BitAddressOffset 10
#define HCCPARAMS2_reserved_31_10_RegisterSize 22



#define DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress 0x30



#define USBCMD (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x0)
#define USBCMD_RegisterSize 32
#define USBCMD_RegisterResetValue 0x0
#define USBCMD_RegisterResetMask 0xffffffff





#define USBCMD_R_S_BitAddressOffset 0
#define USBCMD_R_S_RegisterSize 1



#define USBCMD_HCRST_BitAddressOffset 1
#define USBCMD_HCRST_RegisterSize 1



#define USBCMD_INTE_BitAddressOffset 2
#define USBCMD_INTE_RegisterSize 1



#define USBCMD_HSEE_BitAddressOffset 3
#define USBCMD_HSEE_RegisterSize 1



#define USBCMD_reserved_6_4_BitAddressOffset 4
#define USBCMD_reserved_6_4_RegisterSize 3



#define USBCMD_LHCRST_BitAddressOffset 7
#define USBCMD_LHCRST_RegisterSize 1



#define USBCMD_CSS_BitAddressOffset 8
#define USBCMD_CSS_RegisterSize 1



#define USBCMD_CRS_BitAddressOffset 9
#define USBCMD_CRS_RegisterSize 1



#define USBCMD_EWE_BitAddressOffset 10
#define USBCMD_EWE_RegisterSize 1



#define USBCMD_EU3S_BitAddressOffset 11
#define USBCMD_EU3S_RegisterSize 1



#define USBCMD_reserved_12_BitAddressOffset 12
#define USBCMD_reserved_12_RegisterSize 1



#define USBCMD_CME_BitAddressOffset 13
#define USBCMD_CME_RegisterSize 1



#define USBCMD_ETE_BitAddressOffset 14
#define USBCMD_ETE_RegisterSize 1



#define USBCMD_TSC_EN_BitAddressOffset 15
#define USBCMD_TSC_EN_RegisterSize 1



#define USBCMD_reserved_31_17_BitAddressOffset 17
#define USBCMD_reserved_31_17_RegisterSize 15





#define USBSTS (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x4)
#define USBSTS_RegisterSize 32
#define USBSTS_RegisterResetValue 0x801
#define USBSTS_RegisterResetMask 0x1f1d





#define USBSTS_HCH_BitAddressOffset 0
#define USBSTS_HCH_RegisterSize 1



#define USBSTS_reserved_1_BitAddressOffset 1
#define USBSTS_reserved_1_RegisterSize 1



#define USBSTS_HSE_BitAddressOffset 2
#define USBSTS_HSE_RegisterSize 1



#define USBSTS_EINT_BitAddressOffset 3
#define USBSTS_EINT_RegisterSize 1



#define USBSTS_PCD_BitAddressOffset 4
#define USBSTS_PCD_RegisterSize 1



#define USBSTS_reserved_7_5_BitAddressOffset 5
#define USBSTS_reserved_7_5_RegisterSize 3



#define USBSTS_SSS_BitAddressOffset 8
#define USBSTS_SSS_RegisterSize 1



#define USBSTS_RSS_BitAddressOffset 9
#define USBSTS_RSS_RegisterSize 1



#define USBSTS_SRE_BitAddressOffset 10
#define USBSTS_SRE_RegisterSize 1



#define USBSTS_CNR_BitAddressOffset 11
#define USBSTS_CNR_RegisterSize 1



#define USBSTS_HCE_BitAddressOffset 12
#define USBSTS_HCE_RegisterSize 1



#define USBSTS_reserved_31_13_BitAddressOffset 13
#define USBSTS_reserved_31_13_RegisterSize 19





#define PAGESIZE (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x8)
#define PAGESIZE_RegisterSize 32
#define PAGESIZE_RegisterResetValue 0x1
#define PAGESIZE_RegisterResetMask 0xffff





#define PAGESIZE_PAGE_SIZE_BitAddressOffset 0
#define PAGESIZE_PAGE_SIZE_RegisterSize 16



#define PAGESIZE_reserved_31_16_BitAddressOffset 16
#define PAGESIZE_reserved_31_16_RegisterSize 16





#define DNCTRL (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x14)
#define DNCTRL_RegisterSize 32
#define DNCTRL_RegisterResetValue 0x0
#define DNCTRL_RegisterResetMask 0xffff





#define DNCTRL_N0_N15_BitAddressOffset 0
#define DNCTRL_N0_N15_RegisterSize 16



#define DNCTRL_reserved_31_16_BitAddressOffset 16
#define DNCTRL_reserved_31_16_RegisterSize 16





#define CRCR_LO (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x18)
#define CRCR_LO_RegisterSize 32
#define CRCR_LO_RegisterResetValue 0x0
#define CRCR_LO_RegisterResetMask 0xffffffff





#define CRCR_LO_RCS_BitAddressOffset 0
#define CRCR_LO_RCS_RegisterSize 1



#define CRCR_LO_CS_BitAddressOffset 1
#define CRCR_LO_CS_RegisterSize 1



#define CRCR_LO_CA_BitAddressOffset 2
#define CRCR_LO_CA_RegisterSize 1



#define CRCR_LO_CRR_BitAddressOffset 3
#define CRCR_LO_CRR_RegisterSize 1



#define CRCR_LO_reserved_5_4_BitAddressOffset 4
#define CRCR_LO_reserved_5_4_RegisterSize 2



#define CRCR_LO_CMD_RING_PNTR_BitAddressOffset 6
#define CRCR_LO_CMD_RING_PNTR_RegisterSize 26





#define CRCR_HI (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x1c)
#define CRCR_HI_RegisterSize 32
#define CRCR_HI_RegisterResetValue 0x0
#define CRCR_HI_RegisterResetMask 0xffffffff





#define CRCR_HI_CMD_RING_PNTR_BitAddressOffset 0
#define CRCR_HI_CMD_RING_PNTR_RegisterSize 32





#define DCBAAP_LO (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x30)
#define DCBAAP_LO_RegisterSize 32
#define DCBAAP_LO_RegisterResetValue 0x0
#define DCBAAP_LO_RegisterResetMask 0xffffffc0





#define DCBAAP_LO_reserved_5_0_BitAddressOffset 0
#define DCBAAP_LO_reserved_5_0_RegisterSize 6



#define DCBAAP_LO_DEVICE_CONTEXT_BAAP_BitAddressOffset 6
#define DCBAAP_LO_DEVICE_CONTEXT_BAAP_RegisterSize 26





#define DCBAAP_HI (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x34)
#define DCBAAP_HI_RegisterSize 32
#define DCBAAP_HI_RegisterResetValue 0x0
#define DCBAAP_HI_RegisterResetMask 0xffffffff





#define DCBAAP_HI_DEVICE_CONTEXT_BAAP_BitAddressOffset 0
#define DCBAAP_HI_DEVICE_CONTEXT_BAAP_RegisterSize 32





#define CONFIG (DWC_usb31_block_Host_Cntrl_Oper_Regs_BaseAddress + 0x38)
#define CONFIG_RegisterSize 32
#define CONFIG_RegisterResetValue 0x0
#define CONFIG_RegisterResetMask 0x3ff





#define CONFIG_MAXSLOTSEN_BitAddressOffset 0
#define CONFIG_MAXSLOTSEN_RegisterSize 8



#define CONFIG_U3E_BitAddressOffset 8
#define CONFIG_U3E_RegisterSize 1



#define CONFIG_CIE_BitAddressOffset 9
#define CONFIG_CIE_RegisterSize 1



#define CONFIG_reserved_31_10_BitAddressOffset 10
#define CONFIG_reserved_31_10_RegisterSize 22



#define DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress 0x430



#define PORTSC_20_REGS (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x0)
#define PORTSC_20_RegisterSize 32
#define PORTSC_20_RegisterResetValue 0x2a0
#define PORTSC_20_RegisterResetMask 0xefffc12





#define PORTSC_20_CCS_BitAddressOffset 0
#define PORTSC_20_CCS_RegisterSize 1



#define PORTSC_20_PED_BitAddressOffset 1
#define PORTSC_20_PED_RegisterSize 1



#define PORTSC_20_reserved_2_BitAddressOffset 2
#define PORTSC_20_reserved_2_RegisterSize 1



#define PORTSC_20_OCA_BitAddressOffset 3
#define PORTSC_20_OCA_RegisterSize 1



#define PORTSC_20_PR_BitAddressOffset 4
#define PORTSC_20_PR_RegisterSize 1



#define PORTSC_20_PLS_BitAddressOffset 5
#define PORTSC_20_PLS_RegisterSize 4



#define PORTSC_20_PP_BitAddressOffset 9
#define PORTSC_20_PP_RegisterSize 1



#define PORTSC_20_PORTSPEED_BitAddressOffset 10
#define PORTSC_20_PORTSPEED_RegisterSize 4



#define PORTSC_20_PIC_BitAddressOffset 14
#define PORTSC_20_PIC_RegisterSize 2



#define PORTSC_20_LWS_BitAddressOffset 16
#define PORTSC_20_LWS_RegisterSize 1



#define PORTSC_20_CSC_BitAddressOffset 17
#define PORTSC_20_CSC_RegisterSize 1



#define PORTSC_20_PEC_BitAddressOffset 18
#define PORTSC_20_PEC_RegisterSize 1



#define PORTSC_20_reserved_19_BitAddressOffset 19
#define PORTSC_20_reserved_19_RegisterSize 1



#define PORTSC_20_OCC_BitAddressOffset 20
#define PORTSC_20_OCC_RegisterSize 1



#define PORTSC_20_PRC_BitAddressOffset 21
#define PORTSC_20_PRC_RegisterSize 1



#define PORTSC_20_PLC_BitAddressOffset 22
#define PORTSC_20_PLC_RegisterSize 1



#define PORTSC_20_reserved_23_BitAddressOffset 23
#define PORTSC_20_reserved_23_RegisterSize 1



#define PORTSC_20_CAS_BitAddressOffset 24
#define PORTSC_20_CAS_RegisterSize 1



#define PORTSC_20_WCE_BitAddressOffset 25
#define PORTSC_20_WCE_RegisterSize 1



#define PORTSC_20_WDE_BitAddressOffset 26
#define PORTSC_20_WDE_RegisterSize 1



#define PORTSC_20_WOE_BitAddressOffset 27
#define PORTSC_20_WOE_RegisterSize 1



#define PORTSC_20_reserved_29_28_BitAddressOffset 28
#define PORTSC_20_reserved_29_28_RegisterSize 2



#define PORTSC_20_DR_BitAddressOffset 30
#define PORTSC_20_DR_RegisterSize 1



#define PORTSC_20_reserved_31_BitAddressOffset 31
#define PORTSC_20_reserved_31_RegisterSize 1





#define PORTSC_20_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x0)
#define PORTSC_20_RegisterSize 32
#define PORTSC_20_RegisterResetValue 0x2a0
#define PORTSC_20_RegisterResetMask 0xefffc12





#define PORTSC_20_CCS_BitAddressOffset 0
#define PORTSC_20_CCS_RegisterSize 1



#define PORTSC_20_PED_BitAddressOffset 1
#define PORTSC_20_PED_RegisterSize 1



#define PORTSC_20_reserved_2_BitAddressOffset 2
#define PORTSC_20_reserved_2_RegisterSize 1



#define PORTSC_20_OCA_BitAddressOffset 3
#define PORTSC_20_OCA_RegisterSize 1



#define PORTSC_20_PR_BitAddressOffset 4
#define PORTSC_20_PR_RegisterSize 1



#define PORTSC_20_PLS_BitAddressOffset 5
#define PORTSC_20_PLS_RegisterSize 4



#define PORTSC_20_PP_BitAddressOffset 9
#define PORTSC_20_PP_RegisterSize 1



#define PORTSC_20_PORTSPEED_BitAddressOffset 10
#define PORTSC_20_PORTSPEED_RegisterSize 4



#define PORTSC_20_PIC_BitAddressOffset 14
#define PORTSC_20_PIC_RegisterSize 2



#define PORTSC_20_LWS_BitAddressOffset 16
#define PORTSC_20_LWS_RegisterSize 1



#define PORTSC_20_CSC_BitAddressOffset 17
#define PORTSC_20_CSC_RegisterSize 1



#define PORTSC_20_PEC_BitAddressOffset 18
#define PORTSC_20_PEC_RegisterSize 1



#define PORTSC_20_reserved_19_BitAddressOffset 19
#define PORTSC_20_reserved_19_RegisterSize 1



#define PORTSC_20_OCC_BitAddressOffset 20
#define PORTSC_20_OCC_RegisterSize 1



#define PORTSC_20_PRC_BitAddressOffset 21
#define PORTSC_20_PRC_RegisterSize 1



#define PORTSC_20_PLC_BitAddressOffset 22
#define PORTSC_20_PLC_RegisterSize 1



#define PORTSC_20_reserved_23_BitAddressOffset 23
#define PORTSC_20_reserved_23_RegisterSize 1



#define PORTSC_20_CAS_BitAddressOffset 24
#define PORTSC_20_CAS_RegisterSize 1



#define PORTSC_20_WCE_BitAddressOffset 25
#define PORTSC_20_WCE_RegisterSize 1



#define PORTSC_20_WDE_BitAddressOffset 26
#define PORTSC_20_WDE_RegisterSize 1



#define PORTSC_20_WOE_BitAddressOffset 27
#define PORTSC_20_WOE_RegisterSize 1



#define PORTSC_20_reserved_29_28_BitAddressOffset 28
#define PORTSC_20_reserved_29_28_RegisterSize 2



#define PORTSC_20_DR_BitAddressOffset 30
#define PORTSC_20_DR_RegisterSize 1



#define PORTSC_20_reserved_31_BitAddressOffset 31
#define PORTSC_20_reserved_31_RegisterSize 1





#define PORTPMSC_20_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x4)
#define PORTPMSC_20_RegisterSize 32
#define PORTPMSC_20_RegisterResetValue 0x0
#define PORTPMSC_20_RegisterResetMask 0xf001ffff





#define PORTPMSC_20_L1S_BitAddressOffset 0
#define PORTPMSC_20_L1S_RegisterSize 3



#define PORTPMSC_20_RWE_BitAddressOffset 3
#define PORTPMSC_20_RWE_RegisterSize 1



#define PORTPMSC_20_BESL_BitAddressOffset 4
#define PORTPMSC_20_BESL_RegisterSize 4



#define PORTPMSC_20_L1DSLOT_BitAddressOffset 8
#define PORTPMSC_20_L1DSLOT_RegisterSize 8



#define PORTPMSC_20_HLE_BitAddressOffset 16
#define PORTPMSC_20_HLE_RegisterSize 1



#define PORTPMSC_20_reserved_27_17_BitAddressOffset 17
#define PORTPMSC_20_reserved_27_17_RegisterSize 11



#define PORTPMSC_20_PRTTSTCTRL_BitAddressOffset 28
#define PORTPMSC_20_PRTTSTCTRL_RegisterSize 4





#define PORTLI_20_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x8)
#define PORTLI_20_RegisterSize 32
#define PORTLI_20_RegisterResetValue 0x0
#define PORTLI_20_RegisterResetMask 0x0





#define PORTLI_20_reserved_31_0_BitAddressOffset 0
#define PORTLI_20_reserved_31_0_RegisterSize 32





#define PORTHLPMC_20_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0xc)
#define PORTHLPMC_20_RegisterSize 32
#define PORTHLPMC_20_RegisterResetValue 0x0
#define PORTHLPMC_20_RegisterResetMask 0x3fff





#define PORTHLPMC_20_HIRDM_BitAddressOffset 0
#define PORTHLPMC_20_HIRDM_RegisterSize 2



#define PORTHLPMC_20_L1_TIMEOUT_BitAddressOffset 2
#define PORTHLPMC_20_L1_TIMEOUT_RegisterSize 8



#define PORTHLPMC_20_BESLD_BitAddressOffset 10
#define PORTHLPMC_20_BESLD_RegisterSize 4



#define PORTHLPMC_20_reserved_31_14_BitAddressOffset 14
#define PORTHLPMC_20_reserved_31_14_RegisterSize 18





#define PORTSC_30_REGS (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x10)
#define PORTSC_30_RegisterSize 32
#define PORTSC_30_RegisterResetValue 0x2a0
#define PORTSC_30_RegisterResetMask 0xcefffc12





#define PORTSC_30_CCS_BitAddressOffset 0
#define PORTSC_30_CCS_RegisterSize 1



#define PORTSC_30_PED_BitAddressOffset 1
#define PORTSC_30_PED_RegisterSize 1



#define PORTSC_30_reserved_2_BitAddressOffset 2
#define PORTSC_30_reserved_2_RegisterSize 1



#define PORTSC_30_OCA_BitAddressOffset 3
#define PORTSC_30_OCA_RegisterSize 1



#define PORTSC_30_PR_BitAddressOffset 4
#define PORTSC_30_PR_RegisterSize 1



#define PORTSC_30_PLS_BitAddressOffset 5
#define PORTSC_30_PLS_RegisterSize 4



#define PORTSC_30_PP_BitAddressOffset 9
#define PORTSC_30_PP_RegisterSize 1



#define PORTSC_30_PORTSPEED_BitAddressOffset 10
#define PORTSC_30_PORTSPEED_RegisterSize 4



#define PORTSC_30_PIC_BitAddressOffset 14
#define PORTSC_30_PIC_RegisterSize 2



#define PORTSC_30_LWS_BitAddressOffset 16
#define PORTSC_30_LWS_RegisterSize 1



#define PORTSC_30_CSC_BitAddressOffset 17
#define PORTSC_30_CSC_RegisterSize 1



#define PORTSC_30_PEC_BitAddressOffset 18
#define PORTSC_30_PEC_RegisterSize 1



#define PORTSC_30_WRC_BitAddressOffset 19
#define PORTSC_30_WRC_RegisterSize 1



#define PORTSC_30_OCC_BitAddressOffset 20
#define PORTSC_30_OCC_RegisterSize 1



#define PORTSC_30_PRC_BitAddressOffset 21
#define PORTSC_30_PRC_RegisterSize 1



#define PORTSC_30_PLC_BitAddressOffset 22
#define PORTSC_30_PLC_RegisterSize 1



#define PORTSC_30_CEC_BitAddressOffset 23
#define PORTSC_30_CEC_RegisterSize 1



#define PORTSC_30_CAS_BitAddressOffset 24
#define PORTSC_30_CAS_RegisterSize 1



#define PORTSC_30_WCE_BitAddressOffset 25
#define PORTSC_30_WCE_RegisterSize 1



#define PORTSC_30_WDE_BitAddressOffset 26
#define PORTSC_30_WDE_RegisterSize 1



#define PORTSC_30_WOE_BitAddressOffset 27
#define PORTSC_30_WOE_RegisterSize 1



#define PORTSC_30_reserved_29_28_BitAddressOffset 28
#define PORTSC_30_reserved_29_28_RegisterSize 2



#define PORTSC_30_DR_BitAddressOffset 30
#define PORTSC_30_DR_RegisterSize 1



#define PORTSC_30_WPR_BitAddressOffset 31
#define PORTSC_30_WPR_RegisterSize 1





#define PORTSC_30_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x10)
#define PORTSC_30_RegisterSize 32
#define PORTSC_30_RegisterResetValue 0x2a0
#define PORTSC_30_RegisterResetMask 0xcefffc12





#define PORTSC_30_CCS_BitAddressOffset 0
#define PORTSC_30_CCS_RegisterSize 1



#define PORTSC_30_PED_BitAddressOffset 1
#define PORTSC_30_PED_RegisterSize 1



#define PORTSC_30_reserved_2_BitAddressOffset 2
#define PORTSC_30_reserved_2_RegisterSize 1



#define PORTSC_30_OCA_BitAddressOffset 3
#define PORTSC_30_OCA_RegisterSize 1



#define PORTSC_30_PR_BitAddressOffset 4
#define PORTSC_30_PR_RegisterSize 1



#define PORTSC_30_PLS_BitAddressOffset 5
#define PORTSC_30_PLS_RegisterSize 4



#define PORTSC_30_PP_BitAddressOffset 9
#define PORTSC_30_PP_RegisterSize 1



#define PORTSC_30_PORTSPEED_BitAddressOffset 10
#define PORTSC_30_PORTSPEED_RegisterSize 4



#define PORTSC_30_PIC_BitAddressOffset 14
#define PORTSC_30_PIC_RegisterSize 2



#define PORTSC_30_LWS_BitAddressOffset 16
#define PORTSC_30_LWS_RegisterSize 1



#define PORTSC_30_CSC_BitAddressOffset 17
#define PORTSC_30_CSC_RegisterSize 1



#define PORTSC_30_PEC_BitAddressOffset 18
#define PORTSC_30_PEC_RegisterSize 1



#define PORTSC_30_WRC_BitAddressOffset 19
#define PORTSC_30_WRC_RegisterSize 1



#define PORTSC_30_OCC_BitAddressOffset 20
#define PORTSC_30_OCC_RegisterSize 1



#define PORTSC_30_PRC_BitAddressOffset 21
#define PORTSC_30_PRC_RegisterSize 1



#define PORTSC_30_PLC_BitAddressOffset 22
#define PORTSC_30_PLC_RegisterSize 1



#define PORTSC_30_CEC_BitAddressOffset 23
#define PORTSC_30_CEC_RegisterSize 1



#define PORTSC_30_CAS_BitAddressOffset 24
#define PORTSC_30_CAS_RegisterSize 1



#define PORTSC_30_WCE_BitAddressOffset 25
#define PORTSC_30_WCE_RegisterSize 1



#define PORTSC_30_WDE_BitAddressOffset 26
#define PORTSC_30_WDE_RegisterSize 1



#define PORTSC_30_WOE_BitAddressOffset 27
#define PORTSC_30_WOE_RegisterSize 1



#define PORTSC_30_reserved_29_28_BitAddressOffset 28
#define PORTSC_30_reserved_29_28_RegisterSize 2



#define PORTSC_30_DR_BitAddressOffset 30
#define PORTSC_30_DR_RegisterSize 1



#define PORTSC_30_WPR_BitAddressOffset 31
#define PORTSC_30_WPR_RegisterSize 1





#define PORTPMSC_30_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x14)
#define PORTPMSC_30_RegisterSize 32
#define PORTPMSC_30_RegisterResetValue 0x0
#define PORTPMSC_30_RegisterResetMask 0x1ffff





#define PORTPMSC_30_U1_TIMEOUT_BitAddressOffset 0
#define PORTPMSC_30_U1_TIMEOUT_RegisterSize 8



#define PORTPMSC_30_U2_TIMEOUT_BitAddressOffset 8
#define PORTPMSC_30_U2_TIMEOUT_RegisterSize 8



#define PORTPMSC_30_FLA_BitAddressOffset 16
#define PORTPMSC_30_FLA_RegisterSize 1



#define PORTPMSC_30_reserved_31_17_BitAddressOffset 17
#define PORTPMSC_30_reserved_31_17_RegisterSize 15





#define PORTLI_30_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x18)
#define PORTLI_30_RegisterSize 32
#define PORTLI_30_RegisterResetValue 0x0
#define PORTLI_30_RegisterResetMask 0xffffff





#define PORTLI_30_LINK_ERROR_COUNT_BitAddressOffset 0
#define PORTLI_30_LINK_ERROR_COUNT_RegisterSize 16



#define PORTLI_30_RLC_BitAddressOffset 16
#define PORTLI_30_RLC_RegisterSize 4



#define PORTLI_30_TLC_BitAddressOffset 20
#define PORTLI_30_TLC_RegisterSize 4



#define PORTLI_30_reserved_31_24_BitAddressOffset 24
#define PORTLI_30_reserved_31_24_RegisterSize 8





#define PORTHLPMC_30_0 (DWC_usb31_block_Host_Cntrl_Port_Reg_Set_BaseAddress + 0x1c)
#define PORTHLPMC_30_RegisterSize 32
#define PORTHLPMC_30_RegisterResetValue 0x0
#define PORTHLPMC_30_RegisterResetMask 0x0





#define PORTHLPMC_30_reserved_31_0_BitAddressOffset 0
#define PORTHLPMC_30_reserved_31_0_RegisterSize 32



#define DWC_usb31_block_Host_Cntrl_Runtime_Regs_BaseAddress 0x1000



#define MFINDEX (DWC_usb31_block_Host_Cntrl_Runtime_Regs_BaseAddress + 0x0)
#define MFINDEX_RegisterSize 32
#define MFINDEX_RegisterResetValue 0x0
#define MFINDEX_RegisterResetMask 0x3fff





#define MFINDEX_MICROFRAME_INDEX_BitAddressOffset 0
#define MFINDEX_MICROFRAME_INDEX_RegisterSize 14



#define MFINDEX_reserved_31_14_BitAddressOffset 14
#define MFINDEX_reserved_31_14_RegisterSize 18





#define RsvdZ (DWC_usb31_block_Host_Cntrl_Runtime_Regs_BaseAddress + 0x4)
#define RsvdZ_RegisterSize 32
#define RsvdZ_RegisterResetValue 0x0
#define RsvdZ_RegisterResetMask 0x0





#define RsvdZ_reserved_31_0_BitAddressOffset 0
#define RsvdZ_reserved_31_0_RegisterSize 32



#define DWC_usb31_block_Interrupter_Regs_BaseAddress 0x1020



#define IMAN_REGS (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x0)
#define IMAN_RegisterSize 32
#define IMAN_RegisterResetValue 0x0
#define IMAN_RegisterResetMask 0x3





#define IMAN_IP_BitAddressOffset 0
#define IMAN_IP_RegisterSize 1



#define IMAN_IE_BitAddressOffset 1
#define IMAN_IE_RegisterSize 1



#define IMAN_reserved_31_2_BitAddressOffset 2
#define IMAN_reserved_31_2_RegisterSize 30





#define IMAN_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x0)
#define IMAN_RegisterSize 32
#define IMAN_RegisterResetValue 0x0
#define IMAN_RegisterResetMask 0x3





#define IMAN_IP_BitAddressOffset 0
#define IMAN_IP_RegisterSize 1



#define IMAN_IE_BitAddressOffset 1
#define IMAN_IE_RegisterSize 1



#define IMAN_reserved_31_2_BitAddressOffset 2
#define IMAN_reserved_31_2_RegisterSize 30





#define IMOD_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x4)
#define IMOD_RegisterSize 32
#define IMOD_RegisterResetValue 0xfa0
#define IMOD_RegisterResetMask 0xffffffff





#define IMOD_IMODI_BitAddressOffset 0
#define IMOD_IMODI_RegisterSize 16



#define IMOD_IMODC_BitAddressOffset 16
#define IMOD_IMODC_RegisterSize 16





#define ERSTSZ_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x8)
#define ERSTSZ_RegisterSize 32
#define ERSTSZ_RegisterResetValue 0x0
#define ERSTSZ_RegisterResetMask 0xffff





#define ERSTSZ_ERS_TABLE_SIZE_BitAddressOffset 0
#define ERSTSZ_ERS_TABLE_SIZE_RegisterSize 16



#define ERSTSZ_reserved_31_16_BitAddressOffset 16
#define ERSTSZ_reserved_31_16_RegisterSize 16





#define RsvdP_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0xc)
#define RsvdP_RegisterSize 32
#define RsvdP_RegisterResetValue 0x0
#define RsvdP_RegisterResetMask 0x0





#define RsvdP_reserved_31_0_BitAddressOffset 0
#define RsvdP_reserved_31_0_RegisterSize 32





#define ERSTBA_LO_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x10)
#define ERSTBA_LO_RegisterSize 32
#define ERSTBA_LO_RegisterResetValue 0x0
#define ERSTBA_LO_RegisterResetMask 0xffffffc0





#define ERSTBA_LO_reserved_5_0_BitAddressOffset 0
#define ERSTBA_LO_reserved_5_0_RegisterSize 6



#define ERSTBA_LO_ERS_TABLE_BAR_BitAddressOffset 6
#define ERSTBA_LO_ERS_TABLE_BAR_RegisterSize 26





#define ERSTBA_HI_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x14)
#define ERSTBA_HI_RegisterSize 32
#define ERSTBA_HI_RegisterResetValue 0x0
#define ERSTBA_HI_RegisterResetMask 0xffffffff





#define ERSTBA_HI_ERS_TABLE_BAR_BitAddressOffset 0
#define ERSTBA_HI_ERS_TABLE_BAR_RegisterSize 32





#define ERDP_LO_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x18)
#define ERDP_LO_RegisterSize 32
#define ERDP_LO_RegisterResetValue 0x0
#define ERDP_LO_RegisterResetMask 0xfffffff7





#define ERDP_LO_DESI_BitAddressOffset 0
#define ERDP_LO_DESI_RegisterSize 3



#define ERDP_LO_EHB_BitAddressOffset 3
#define ERDP_LO_EHB_RegisterSize 1



#define ERDP_LO_ERD_PNTR_BitAddressOffset 4
#define ERDP_LO_ERD_PNTR_RegisterSize 28





#define ERDP_HI_0 (DWC_usb31_block_Interrupter_Regs_BaseAddress + 0x1c)
#define ERDP_HI_RegisterSize 32
#define ERDP_HI_RegisterResetValue 0x0
#define ERDP_HI_RegisterResetMask 0xffffffff





#define ERDP_HI_ERD_PNTR_BitAddressOffset 0
#define ERDP_HI_ERD_PNTR_RegisterSize 32



#define DWC_usb31_block_Doorbell_Register_BaseAddress 0x2000



#define DB_REGS (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_0 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_1 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_2 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_3 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xc)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_4 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x10)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_5 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x14)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_6 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x18)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_7 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x1c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_8 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x20)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_9 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x24)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_10 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x28)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_11 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x2c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_12 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x30)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_13 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x34)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_14 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x38)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_15 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x3c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_16 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x40)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_17 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x44)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_18 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x48)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_19 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x4c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_20 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x50)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_21 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x54)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_22 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x58)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_23 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x5c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_24 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x60)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_25 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x64)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_26 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x68)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_27 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x6c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_28 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x70)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_29 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x74)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_30 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x78)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_31 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x7c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_32 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x80)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_33 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x84)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_34 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x88)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_35 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x8c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_36 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x90)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_37 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x94)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_38 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x98)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_39 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x9c)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_40 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xa0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_41 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xa4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_42 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xa8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_43 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xac)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_44 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xb0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_45 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xb4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_46 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xb8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_47 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xbc)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_48 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xc0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_49 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xc4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_50 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xc8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_51 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xcc)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_52 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xd0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_53 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xd4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_54 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xd8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_55 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xdc)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_56 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xe0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_57 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xe4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_58 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xe8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_59 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xec)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_60 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xf0)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_61 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xf4)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_62 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xf8)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_63 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0xfc)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16





#define DB_64 (DWC_usb31_block_Doorbell_Register_BaseAddress + 0x100)
#define DB_RegisterSize 32
#define DB_RegisterResetValue 0x0
#define DB_RegisterResetMask 0xffff00ff





#define DB_DB_TARGET_BitAddressOffset 0
#define DB_DB_TARGET_RegisterSize 8



#define DB_reserved_15_8_BitAddressOffset 8
#define DB_reserved_15_8_RegisterSize 8



#define DB_DB_STREAM_ID_BitAddressOffset 16
#define DB_DB_STREAM_ID_RegisterSize 16



#define DWC_usb31_block_HC_Extended_Capability_Register_BaseAddress 0x460



#define USBLEGSUP (DWC_usb31_block_HC_Extended_Capability_Register_BaseAddress + 0x0)
#define USBLEGSUP_RegisterSize 32
#define USBLEGSUP_RegisterResetValue 0x401
#define USBLEGSUP_RegisterResetMask 0x101ffff





#define USBLEGSUP_CAPABILITY_ID_BitAddressOffset 0
#define USBLEGSUP_CAPABILITY_ID_RegisterSize 8



#define USBLEGSUP_NEXT_CAPABILITY_POINTER_BitAddressOffset 8
#define USBLEGSUP_NEXT_CAPABILITY_POINTER_RegisterSize 8



#define USBLEGSUP_HC_BIOS_OWNED_BitAddressOffset 16
#define USBLEGSUP_HC_BIOS_OWNED_RegisterSize 1



#define USBLEGSUP_reserved_23_17_BitAddressOffset 17
#define USBLEGSUP_reserved_23_17_RegisterSize 7



#define USBLEGSUP_HC_OS_OWNED_BitAddressOffset 24
#define USBLEGSUP_HC_OS_OWNED_RegisterSize 1



#define USBLEGSUP_reserved_31_25_BitAddressOffset 25
#define USBLEGSUP_reserved_31_25_RegisterSize 7





#define USBLEGCTLSTS (DWC_usb31_block_HC_Extended_Capability_Register_BaseAddress + 0x4)
#define USBLEGCTLSTS_RegisterSize 32
#define USBLEGCTLSTS_RegisterResetValue 0x0
#define USBLEGCTLSTS_RegisterResetMask 0xe011e011





#define USBLEGCTLSTS_USB_SMI_ENABLE_BitAddressOffset 0
#define USBLEGCTLSTS_USB_SMI_ENABLE_RegisterSize 1



#define USBLEGCTLSTS_reserved_3_1_BitAddressOffset 1
#define USBLEGCTLSTS_reserved_3_1_RegisterSize 3



#define USBLEGCTLSTS_SMI_ON_HOST_E_BitAddressOffset 4
#define USBLEGCTLSTS_SMI_ON_HOST_E_RegisterSize 1



#define USBLEGCTLSTS_reserved_12_5_BitAddressOffset 5
#define USBLEGCTLSTS_reserved_12_5_RegisterSize 8



#define USBLEGCTLSTS_SMI_ON_OS_E_BitAddressOffset 13
#define USBLEGCTLSTS_SMI_ON_OS_E_RegisterSize 1



#define USBLEGCTLSTS_SMI_ON_PCI_E_BitAddressOffset 14
#define USBLEGCTLSTS_SMI_ON_PCI_E_RegisterSize 1



#define USBLEGCTLSTS_SMI_ON_BAR_E_BitAddressOffset 15
#define USBLEGCTLSTS_SMI_ON_BAR_E_RegisterSize 1



#define USBLEGCTLSTS_SMI_ON_EVENT_BitAddressOffset 16
#define USBLEGCTLSTS_SMI_ON_EVENT_RegisterSize 1



#define USBLEGCTLSTS_reserved_19_17_BitAddressOffset 17
#define USBLEGCTLSTS_reserved_19_17_RegisterSize 3



#define USBLEGCTLSTS_SMI_ON_HOST_BitAddressOffset 20
#define USBLEGCTLSTS_SMI_ON_HOST_RegisterSize 1



#define USBLEGCTLSTS_reserved_28_21_BitAddressOffset 21
#define USBLEGCTLSTS_reserved_28_21_RegisterSize 8



#define USBLEGCTLSTS_SMI_ON_OS_BitAddressOffset 29
#define USBLEGCTLSTS_SMI_ON_OS_RegisterSize 1



#define USBLEGCTLSTS_SMI_ON_PCI_BitAddressOffset 30
#define USBLEGCTLSTS_SMI_ON_PCI_RegisterSize 1



#define USBLEGCTLSTS_SMI_ON_BAR_BitAddressOffset 31
#define USBLEGCTLSTS_SMI_ON_BAR_RegisterSize 1



#define DWC_usb31_block_xHCI_Supt_USB20_Prt_Cap_BaseAddress 0x470



#define SUPTPRT2_DW0 (DWC_usb31_block_xHCI_Supt_USB20_Prt_Cap_BaseAddress + 0x0)
#define SUPTPRT2_DW0_RegisterSize 32
#define SUPTPRT2_DW0_RegisterResetValue 0x2000402
#define SUPTPRT2_DW0_RegisterResetMask 0xffffffff





#define SUPTPRT2_DW0_CAPABILITY_ID_BitAddressOffset 0
#define SUPTPRT2_DW0_CAPABILITY_ID_RegisterSize 8



#define SUPTPRT2_DW0_NEXT_CAPABILITY_POINTER_BitAddressOffset 8
#define SUPTPRT2_DW0_NEXT_CAPABILITY_POINTER_RegisterSize 8



#define SUPTPRT2_DW0_MINOR_REVISION_BitAddressOffset 16
#define SUPTPRT2_DW0_MINOR_REVISION_RegisterSize 8



#define SUPTPRT2_DW0_MAJOR_REVISION_BitAddressOffset 24
#define SUPTPRT2_DW0_MAJOR_REVISION_RegisterSize 8





#define SUPTPRT2_DW1 (DWC_usb31_block_xHCI_Supt_USB20_Prt_Cap_BaseAddress + 0x4)
#define SUPTPRT2_DW1_RegisterSize 32
#define SUPTPRT2_DW1_RegisterResetValue 0x20425355
#define SUPTPRT2_DW1_RegisterResetMask 0xffffffff





#define SUPTPRT2_DW1_NAME_STRING_BitAddressOffset 0
#define SUPTPRT2_DW1_NAME_STRING_RegisterSize 32





#define SUPTPRT2_DW2 (DWC_usb31_block_xHCI_Supt_USB20_Prt_Cap_BaseAddress + 0x8)
#define SUPTPRT2_DW2_RegisterSize 32
#define SUPTPRT2_DW2_RegisterResetValue 0x180101
#define SUPTPRT2_DW2_RegisterResetMask 0xffffffff





#define SUPTPRT2_DW2_COMPATIBLE_PORT_OFFSET_BitAddressOffset 0
#define SUPTPRT2_DW2_COMPATIBLE_PORT_OFFSET_RegisterSize 8



#define SUPTPRT2_DW2_COMPATIBLE_PORT_COUNT_BitAddressOffset 8
#define SUPTPRT2_DW2_COMPATIBLE_PORT_COUNT_RegisterSize 8



#define SUPTPRT2_DW2_L1C_BitAddressOffset 16
#define SUPTPRT2_DW2_L1C_RegisterSize 1



#define SUPTPRT2_DW2_HSO_BitAddressOffset 17
#define SUPTPRT2_DW2_HSO_RegisterSize 1



#define SUPTPRT2_DW2_IHI_BitAddressOffset 18
#define SUPTPRT2_DW2_IHI_RegisterSize 1



#define SUPTPRT2_DW2_HLC_BitAddressOffset 19
#define SUPTPRT2_DW2_HLC_RegisterSize 1



#define SUPTPRT2_DW2_BLC_BitAddressOffset 20
#define SUPTPRT2_DW2_BLC_RegisterSize 1



#define SUPTPRT2_DW2_Reserved_24_21_BitAddressOffset 21
#define SUPTPRT2_DW2_Reserved_24_21_RegisterSize 4



#define SUPTPRT2_DW2_MHD_BitAddressOffset 25
#define SUPTPRT2_DW2_MHD_RegisterSize 3



#define SUPTPRT2_DW2_PSIC_BitAddressOffset 28
#define SUPTPRT2_DW2_PSIC_RegisterSize 4





#define SUPTPRT2_DW3 (DWC_usb31_block_xHCI_Supt_USB20_Prt_Cap_BaseAddress + 0xc)
#define SUPTPRT2_DW3_RegisterSize 32
#define SUPTPRT2_DW3_RegisterResetValue 0x0
#define SUPTPRT2_DW3_RegisterResetMask 0x1f





#define SUPTPRT2_DW3_PROTCL_SLT_TY_BitAddressOffset 0
#define SUPTPRT2_DW3_PROTCL_SLT_TY_RegisterSize 5



#define SUPTPRT2_DW3_reserved_31_5_BitAddressOffset 5
#define SUPTPRT2_DW3_reserved_31_5_RegisterSize 27



#define DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress 0x480



#define SUPT_30_REGS (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x0)
#define SUPTPRT3_DW0_RegisterSize 32
#define SUPTPRT3_DW0_RegisterResetValue 0x3100002
#define SUPTPRT3_DW0_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW0_CAPABILITY_ID_BitAddressOffset 0
#define SUPTPRT3_DW0_CAPABILITY_ID_RegisterSize 8



#define SUPTPRT3_DW0_NEXT_CAPABILITY_POINTER_BitAddressOffset 8
#define SUPTPRT3_DW0_NEXT_CAPABILITY_POINTER_RegisterSize 8



#define SUPTPRT3_DW0_MINOR_REVISION_BitAddressOffset 16
#define SUPTPRT3_DW0_MINOR_REVISION_RegisterSize 8



#define SUPTPRT3_DW0_MAJOR_REVISION_BitAddressOffset 24
#define SUPTPRT3_DW0_MAJOR_REVISION_RegisterSize 8





#define SUPTPRT3_DW0_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x0)
#define SUPTPRT3_DW0_RegisterSize 32
#define SUPTPRT3_DW0_RegisterResetValue 0x3100002
#define SUPTPRT3_DW0_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW0_CAPABILITY_ID_BitAddressOffset 0
#define SUPTPRT3_DW0_CAPABILITY_ID_RegisterSize 8



#define SUPTPRT3_DW0_NEXT_CAPABILITY_POINTER_BitAddressOffset 8
#define SUPTPRT3_DW0_NEXT_CAPABILITY_POINTER_RegisterSize 8



#define SUPTPRT3_DW0_MINOR_REVISION_BitAddressOffset 16
#define SUPTPRT3_DW0_MINOR_REVISION_RegisterSize 8



#define SUPTPRT3_DW0_MAJOR_REVISION_BitAddressOffset 24
#define SUPTPRT3_DW0_MAJOR_REVISION_RegisterSize 8





#define SUPTPRT3_DW1_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x4)
#define SUPTPRT3_DW1_RegisterSize 32
#define SUPTPRT3_DW1_RegisterResetValue 0x20425355
#define SUPTPRT3_DW1_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW1_NAME_STRING_BitAddressOffset 0
#define SUPTPRT3_DW1_NAME_STRING_RegisterSize 32





#define SUPTPRT3_DW2_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x8)
#define SUPTPRT3_DW2_RegisterSize 32
#define SUPTPRT3_DW2_RegisterResetValue 0x20000102
#define SUPTPRT3_DW2_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW2_COMPATIBLE_PORT_OFFSET_BitAddressOffset 0
#define SUPTPRT3_DW2_COMPATIBLE_PORT_OFFSET_RegisterSize 8



#define SUPTPRT3_DW2_COMPATIBLE_PORT_COUNT_BitAddressOffset 8
#define SUPTPRT3_DW2_COMPATIBLE_PORT_COUNT_RegisterSize 8



#define SUPTPRT3_DW2_reserved_27_16_BitAddressOffset 16
#define SUPTPRT3_DW2_reserved_27_16_RegisterSize 12



#define SUPTPRT3_DW2_PSIC_BitAddressOffset 28
#define SUPTPRT3_DW2_PSIC_RegisterSize 4





#define SUPTPRT3_DW3_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0xc)
#define SUPTPRT3_DW3_RegisterSize 32
#define SUPTPRT3_DW3_RegisterResetValue 0x0
#define SUPTPRT3_DW3_RegisterResetMask 0x1f





#define SUPTPRT3_DW3_PROTCL_SLT_TY_BitAddressOffset 0
#define SUPTPRT3_DW3_PROTCL_SLT_TY_RegisterSize 5



#define SUPTPRT3_DW3_reserved_31_5_BitAddressOffset 5
#define SUPTPRT3_DW3_reserved_31_5_RegisterSize 27





#define SUPTPRT3_DW4_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x10)
#define SUPTPRT3_DW4_RegisterSize 32
#define SUPTPRT3_DW4_RegisterResetValue 0x50134
#define SUPTPRT3_DW4_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW4_PSIV_BitAddressOffset 0
#define SUPTPRT3_DW4_PSIV_RegisterSize 4



#define SUPTPRT3_DW4_PSIE_BitAddressOffset 4
#define SUPTPRT3_DW4_PSIE_RegisterSize 2



#define SUPTPRT3_DW4_PLT_BitAddressOffset 6
#define SUPTPRT3_DW4_PLT_RegisterSize 2



#define SUPTPRT3_DW4_PFD_BitAddressOffset 8
#define SUPTPRT3_DW4_PFD_RegisterSize 1



#define SUPTPRT3_DW4_reserved_13_9_BitAddressOffset 9
#define SUPTPRT3_DW4_reserved_13_9_RegisterSize 5



#define SUPTPRT3_DW4_LP_BitAddressOffset 14
#define SUPTPRT3_DW4_LP_RegisterSize 2



#define SUPTPRT3_DW4_PSIM_BitAddressOffset 16
#define SUPTPRT3_DW4_PSIM_RegisterSize 16





#define SUPTPRT3_DW5_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x14)
#define SUPTPRT3_DW5_RegisterSize 32
#define SUPTPRT3_DW5_RegisterResetValue 0xa4135
#define SUPTPRT3_DW5_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW5_PSIV_BitAddressOffset 0
#define SUPTPRT3_DW5_PSIV_RegisterSize 4



#define SUPTPRT3_DW5_PSIE_BitAddressOffset 4
#define SUPTPRT3_DW5_PSIE_RegisterSize 2



#define SUPTPRT3_DW5_PLT_BitAddressOffset 6
#define SUPTPRT3_DW5_PLT_RegisterSize 2



#define SUPTPRT3_DW5_PFD_BitAddressOffset 8
#define SUPTPRT3_DW5_PFD_RegisterSize 1



#define SUPTPRT3_DW5_reserved_13_9_BitAddressOffset 9
#define SUPTPRT3_DW5_reserved_13_9_RegisterSize 5



#define SUPTPRT3_DW5_LP_BitAddressOffset 14
#define SUPTPRT3_DW5_LP_RegisterSize 2



#define SUPTPRT3_DW5_PSIM_BitAddressOffset 16
#define SUPTPRT3_DW5_PSIM_RegisterSize 16





#define SUPTPRT3_DW6_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x18)
#define SUPTPRT3_DW6_RegisterSize 32
#define SUPTPRT3_DW6_RegisterResetValue 0x0
#define SUPTPRT3_DW6_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW6_reserved_31_0_BitAddressOffset 0
#define SUPTPRT3_DW6_reserved_31_0_RegisterSize 32





#define SUPTPRT3_DW7_0 (DWC_usb31_block_xHCI_Supt_USB30_Prt_Cap_BaseAddress + 0x1c)
#define SUPTPRT3_DW7_RegisterSize 32
#define SUPTPRT3_DW7_RegisterResetValue 0x0
#define SUPTPRT3_DW7_RegisterResetMask 0xffffffff





#define SUPTPRT3_DW7_reserved_31_0_BitAddressOffset 0
#define SUPTPRT3_DW7_reserved_31_0_RegisterSize 32



#endif
