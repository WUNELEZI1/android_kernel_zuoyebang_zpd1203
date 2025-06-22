// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#define DWC_ufshc_block_BaseAddress 0x0



#define CAP (DWC_ufshc_block_BaseAddress + 0x0)
#define CAP_RegisterSize 32
#define CAP_RegisterResetValue 0x43870f1f
#define CAP_RegisterResetMask 0xffffffff





#define CAP_NUTRS_BitAddressOffset 0
#define CAP_NUTRS_RegisterSize 8



#define CAP_NORTTS_BitAddressOffset 8
#define CAP_NORTTS_RegisterSize 8



#define CAP_NUTMRS_BitAddressOffset 16
#define CAP_NUTMRS_RegisterSize 3



#define CAP_CAP_RSVD_21_19_BitAddressOffset 19
#define CAP_CAP_RSVD_21_19_RegisterSize 3



#define CAP_EHSLUTRDS_BitAddressOffset 22
#define CAP_EHSLUTRDS_RegisterSize 1



#define CAP_AUTOH8_BitAddressOffset 23
#define CAP_AUTOH8_RegisterSize 1



#define CAP_AS64_BitAddressOffset 24
#define CAP_AS64_RegisterSize 1



#define CAP_OODDS_BitAddressOffset 25
#define CAP_OODDS_RegisterSize 1



#define CAP_UICDMETMS_BitAddressOffset 26
#define CAP_UICDMETMS_RegisterSize 1



#define CAP_CS_BitAddressOffset 28
#define CAP_CS_RegisterSize 1



#define CAP_SDBS_BitAddressOffset 29
#define CAP_SDBS_RegisterSize 1



#define CAP_MCQS_BitAddressOffset 30
#define CAP_MCQS_RegisterSize 1



#define CAP_ESI_BitAddressOffset 31
#define CAP_ESI_RegisterSize 1





#define MCQCAP (DWC_ufshc_block_BaseAddress + 0x4)
#define MCQCAP_RegisterSize 32
#define MCQCAP_RegisterResetValue 0x1018070f
#define MCQCAP_RegisterResetMask 0xffffffff





#define MCQCAP_MAXQ_BitAddressOffset 0
#define MCQCAP_MAXQ_RegisterSize 8



#define MCQCAP_SP_BitAddressOffset 8
#define MCQCAP_SP_RegisterSize 1



#define MCQCAP_RRP_BitAddressOffset 9
#define MCQCAP_RRP_RegisterSize 1



#define MCQCAP_EIS_BitAddressOffset 10
#define MCQCAP_EIS_RegisterSize 1



#define MCQCAP_MCQCAP_RSVD_15_11_BitAddressOffset 11
#define MCQCAP_MCQCAP_RSVD_15_11_RegisterSize 5



#define MCQCAP_QCFGPTR_BitAddressOffset 16
#define MCQCAP_QCFGPTR_RegisterSize 8



#define MCQCAP_MIAG_BitAddressOffset 24
#define MCQCAP_MIAG_RegisterSize 8





#define VER (DWC_ufshc_block_BaseAddress + 0x8)
#define VER_RegisterSize 32
#define VER_RegisterResetValue 0x400
#define VER_RegisterResetMask 0xffffffff





#define VER_VS_BitAddressOffset 0
#define VER_VS_RegisterSize 4



#define VER_MNR_BitAddressOffset 4
#define VER_MNR_RegisterSize 4



#define VER_MJR_BitAddressOffset 8
#define VER_MJR_RegisterSize 8



#define VER_VER_RSVD_31_16_BitAddressOffset 16
#define VER_VER_RSVD_31_16_RegisterSize 16





#define EXT_CAP (DWC_ufshc_block_BaseAddress + 0xc)
#define EXT_CAP_RegisterSize 32
#define EXT_CAP_RegisterResetValue 0x80
#define EXT_CAP_RegisterResetMask 0xffffffff





#define EXT_CAP_HostHintCacheSize_BitAddressOffset 0
#define EXT_CAP_HostHintCacheSize_RegisterSize 16



#define EXT_CAP_EXT_CAP_RSVD_31_16_BitAddressOffset 16
#define EXT_CAP_EXT_CAP_RSVD_31_16_RegisterSize 16





#define HCPID (DWC_ufshc_block_BaseAddress + 0x10)
#define HCPID_RegisterSize 32
#define HCPID_RegisterResetValue 0x0
#define HCPID_RegisterResetMask 0xffffffff





#define HCPID_PID_BitAddressOffset 0
#define HCPID_PID_RegisterSize 32





#define HCMID (DWC_ufshc_block_BaseAddress + 0x14)
#define HCMID_RegisterSize 32
#define HCMID_RegisterResetValue 0x5e6
#define HCMID_RegisterResetMask 0xffffffff





#define HCMID_MIC_BitAddressOffset 0
#define HCMID_MIC_RegisterSize 8



#define HCMID_BI_BitAddressOffset 8
#define HCMID_BI_RegisterSize 8



#define HCMID_HCMID_RSVD_31_16_BitAddressOffset 16
#define HCMID_HCMID_RSVD_31_16_RegisterSize 16





#define AHIT (DWC_ufshc_block_BaseAddress + 0x18)
#define AHIT_RegisterSize 32
#define AHIT_RegisterResetValue 0x0
#define AHIT_RegisterResetMask 0xffffffff





#define AHIT_AH8ITV_BitAddressOffset 0
#define AHIT_AH8ITV_RegisterSize 10



#define AHIT_TS_BitAddressOffset 10
#define AHIT_TS_RegisterSize 3



#define AHIT_AHIT_RSVD_31_13_BitAddressOffset 13
#define AHIT_AHIT_RSVD_31_13_RegisterSize 19





#define IS (DWC_ufshc_block_BaseAddress + 0x20)
#define IS_RegisterSize 32
#define IS_RegisterResetValue 0x0
#define IS_RegisterResetMask 0xffffffff





#define IS_UTRCS_BitAddressOffset 0
#define IS_UTRCS_RegisterSize 1



#define IS_UDEPRI_BitAddressOffset 1
#define IS_UDEPRI_RegisterSize 1



#define IS_UE_BitAddressOffset 2
#define IS_UE_RegisterSize 1



#define IS_UTMS_BitAddressOffset 3
#define IS_UTMS_RegisterSize 1



#define IS_UPMS_BitAddressOffset 4
#define IS_UPMS_RegisterSize 1



#define IS_UHXS_BitAddressOffset 5
#define IS_UHXS_RegisterSize 1



#define IS_UHES_BitAddressOffset 6
#define IS_UHES_RegisterSize 1



#define IS_ULLS_BitAddressOffset 7
#define IS_ULLS_RegisterSize 1



#define IS_ULSS_BitAddressOffset 8
#define IS_ULSS_RegisterSize 1



#define IS_UTMRCS_BitAddressOffset 9
#define IS_UTMRCS_RegisterSize 1



#define IS_UCCS_BitAddressOffset 10
#define IS_UCCS_RegisterSize 1



#define IS_DFES_BitAddressOffset 11
#define IS_DFES_RegisterSize 1



#define IS_UTPES_BitAddressOffset 12
#define IS_UTPES_RegisterSize 1



#define IS_IS_RSVD_15_13_BitAddressOffset 13
#define IS_IS_RSVD_15_13_RegisterSize 3



#define IS_HCFES_BitAddressOffset 16
#define IS_HCFES_RegisterSize 1



#define IS_SBFES_BitAddressOffset 17
#define IS_SBFES_RegisterSize 1



#define IS_CEFES_BitAddressOffset 18
#define IS_CEFES_RegisterSize 1



#define IS_SQES_BitAddressOffset 19
#define IS_SQES_RegisterSize 1



#define IS_CQES_BitAddressOffset 20
#define IS_CQES_RegisterSize 1



#define IS_IAGES_BitAddressOffset 21
#define IS_IAGES_RegisterSize 1



#define IS_IS_RSVD_28_22_BitAddressOffset 22
#define IS_IS_RSVD_28_22_RegisterSize 7





#define IE (DWC_ufshc_block_BaseAddress + 0x24)
#define IE_RegisterSize 32
#define IE_RegisterResetValue 0x0
#define IE_RegisterResetMask 0xffffffff





#define IE_UTRCE_BitAddressOffset 0
#define IE_UTRCE_RegisterSize 1



#define IE_UDEPRIE_BitAddressOffset 1
#define IE_UDEPRIE_RegisterSize 1



#define IE_UEE_BitAddressOffset 2
#define IE_UEE_RegisterSize 1



#define IE_UTMSE_BitAddressOffset 3
#define IE_UTMSE_RegisterSize 1



#define IE_UPMSE_BitAddressOffset 4
#define IE_UPMSE_RegisterSize 1



#define IE_UHXSE_BitAddressOffset 5
#define IE_UHXSE_RegisterSize 1



#define IE_UHESE_BitAddressOffset 6
#define IE_UHESE_RegisterSize 1



#define IE_ULLSE_BitAddressOffset 7
#define IE_ULLSE_RegisterSize 1



#define IE_ULSSE_BitAddressOffset 8
#define IE_ULSSE_RegisterSize 1



#define IE_UTMRCE_BitAddressOffset 9
#define IE_UTMRCE_RegisterSize 1



#define IE_UCCE_BitAddressOffset 10
#define IE_UCCE_RegisterSize 1



#define IE_DFEE_BitAddressOffset 11
#define IE_DFEE_RegisterSize 1



#define IE_UTPEE_BitAddressOffset 12
#define IE_UTPEE_RegisterSize 1



#define IE_IE_RSVD_15_13_BitAddressOffset 13
#define IE_IE_RSVD_15_13_RegisterSize 3



#define IE_HCFEE_BitAddressOffset 16
#define IE_HCFEE_RegisterSize 1



#define IE_SBFEE_BitAddressOffset 17
#define IE_SBFEE_RegisterSize 1



#define IE_CEFEE_BitAddressOffset 18
#define IE_CEFEE_RegisterSize 1



#define IE_SQEE_BitAddressOffset 19
#define IE_SQEE_RegisterSize 1



#define IE_CQEE_BitAddressOffset 20
#define IE_CQEE_RegisterSize 1



#define IE_IAGEE_BitAddressOffset 21
#define IE_IAGEE_RegisterSize 1



#define IE_IE_RSVD_28_22_BitAddressOffset 22
#define IE_IE_RSVD_28_22_RegisterSize 7





#define HCSEXT (DWC_ufshc_block_BaseAddress + 0x2c)
#define HCSEXT_RegisterSize 32
#define HCSEXT_RegisterResetValue 0x0
#define HCSEXT_RegisterResetMask 0xffffffff





#define HCSEXT_IIDUTPE_BitAddressOffset 0
#define HCSEXT_IIDUTPE_RegisterSize 4



#define HCSEXT_EXT_IIDUTPE_BitAddressOffset 4
#define HCSEXT_EXT_IIDUTPE_RegisterSize 4



#define HCSEXT_HCSEXT_RSVD_31_8_BitAddressOffset 8
#define HCSEXT_HCSEXT_RSVD_31_8_RegisterSize 24





#define HCS (DWC_ufshc_block_BaseAddress + 0x30)
#define HCS_RegisterSize 32
#define HCS_RegisterResetValue 0x0
#define HCS_RegisterResetMask 0xffffffff





#define HCS_DP_BitAddressOffset 0
#define HCS_DP_RegisterSize 1



#define HCS_UTRLRDY_BitAddressOffset 1
#define HCS_UTRLRDY_RegisterSize 1



#define HCS_UTMRLRDY_BitAddressOffset 2
#define HCS_UTMRLRDY_RegisterSize 1



#define HCS_UCRDY_BitAddressOffset 3
#define HCS_UCRDY_RegisterSize 1



#define HCS_HCS_RSVD_7_4_BitAddressOffset 4
#define HCS_HCS_RSVD_7_4_RegisterSize 4



#define HCS_UPMCRS_BitAddressOffset 8
#define HCS_UPMCRS_RegisterSize 3



#define HCS_UTPEC_BitAddressOffset 12
#define HCS_UTPEC_RegisterSize 4



#define HCS_TTAGUTPE_BitAddressOffset 16
#define HCS_TTAGUTPE_RegisterSize 8



#define HCS_TLUNUTPE_BitAddressOffset 24
#define HCS_TLUNUTPE_RegisterSize 8





#define HCE (DWC_ufshc_block_BaseAddress + 0x34)
#define HCE_RegisterSize 32
#define HCE_RegisterResetValue 0x0
#define HCE_RegisterResetMask 0xffffffff





#define HCE_HCE_BitAddressOffset 0
#define HCE_HCE_RegisterSize 1



#define HCE_CGE_BitAddressOffset 1
#define HCE_CGE_RegisterSize 1



#define HCE_HCE_RSVD_31_2_BitAddressOffset 2
#define HCE_HCE_RSVD_31_2_RegisterSize 30





#define UECPA (DWC_ufshc_block_BaseAddress + 0x38)
#define UECPA_RegisterSize 32
#define UECPA_RegisterResetValue 0x0
#define UECPA_RegisterResetMask 0xffffffff





#define UECPA_EC_BitAddressOffset 0
#define UECPA_EC_RegisterSize 5



#define UECPA_UECPA_RSVD_30_5_BitAddressOffset 5
#define UECPA_UECPA_RSVD_30_5_RegisterSize 26



#define UECPA_ERR_BitAddressOffset 31
#define UECPA_ERR_RegisterSize 1





#define UECDL (DWC_ufshc_block_BaseAddress + 0x3c)
#define UECDL_RegisterSize 32
#define UECDL_RegisterResetValue 0x0
#define UECDL_RegisterResetMask 0xffffffff





#define UECDL_EC_BitAddressOffset 0
#define UECDL_EC_RegisterSize 16



#define UECDL_UECDL_RSVD_BitAddressOffset 16
#define UECDL_UECDL_RSVD_RegisterSize 15



#define UECDL_ERR_BitAddressOffset 31
#define UECDL_ERR_RegisterSize 1





#define UECN (DWC_ufshc_block_BaseAddress + 0x40)
#define UECN_RegisterSize 32
#define UECN_RegisterResetValue 0x0
#define UECN_RegisterResetMask 0xffffffff





#define UECN_EC_BitAddressOffset 0
#define UECN_EC_RegisterSize 3



#define UECN_UECN_RSVD_30_3_BitAddressOffset 3
#define UECN_UECN_RSVD_30_3_RegisterSize 28



#define UECN_ERR_BitAddressOffset 31
#define UECN_ERR_RegisterSize 1





#define UECT (DWC_ufshc_block_BaseAddress + 0x44)
#define UECT_RegisterSize 32
#define UECT_RegisterResetValue 0x0
#define UECT_RegisterResetMask 0xffffffff





#define UECT_EC_BitAddressOffset 0
#define UECT_EC_RegisterSize 7



#define UECT_UECT_RSVD_30_7_BitAddressOffset 7
#define UECT_UECT_RSVD_30_7_RegisterSize 24



#define UECT_ERR_BitAddressOffset 31
#define UECT_ERR_RegisterSize 1





#define UECDME (DWC_ufshc_block_BaseAddress + 0x48)
#define UECDME_RegisterSize 32
#define UECDME_RegisterResetValue 0x0
#define UECDME_RegisterResetMask 0xffffffff





#define UECDME_EC_BitAddressOffset 0
#define UECDME_EC_RegisterSize 4



#define UECDME_UECDME_RSVD_BitAddressOffset 4
#define UECDME_UECDME_RSVD_RegisterSize 27



#define UECDME_ERR_BitAddressOffset 31
#define UECDME_ERR_RegisterSize 1





#define UTRIACR (DWC_ufshc_block_BaseAddress + 0x4c)
#define UTRIACR_RegisterSize 32
#define UTRIACR_RegisterResetValue 0x0
#define UTRIACR_RegisterResetMask 0xffffffff





#define UTRIACR_IATOVAL_BitAddressOffset 0
#define UTRIACR_IATOVAL_RegisterSize 8



#define UTRIACR_IACTH_BitAddressOffset 8
#define UTRIACR_IACTH_RegisterSize 5



#define UTRIACR_UTRIACR_RSVD_15_13_BitAddressOffset 13
#define UTRIACR_UTRIACR_RSVD_15_13_RegisterSize 3



#define UTRIACR_CTR_BitAddressOffset 16
#define UTRIACR_CTR_RegisterSize 1



#define UTRIACR_UTRIACR_RSVD_19_17_BitAddressOffset 17
#define UTRIACR_UTRIACR_RSVD_19_17_RegisterSize 3



#define UTRIACR_IASB_BitAddressOffset 20
#define UTRIACR_IASB_RegisterSize 1



#define UTRIACR_UTRIACR_RSVD_23_21_BitAddressOffset 21
#define UTRIACR_UTRIACR_RSVD_23_21_RegisterSize 3



#define UTRIACR_IAPWEN_BitAddressOffset 24
#define UTRIACR_IAPWEN_RegisterSize 1



#define UTRIACR_UTRIACR_RSVD_30_25_BitAddressOffset 25
#define UTRIACR_UTRIACR_RSVD_30_25_RegisterSize 6



#define UTRIACR_IAEN_BitAddressOffset 31
#define UTRIACR_IAEN_RegisterSize 1





#define UTRLBA (DWC_ufshc_block_BaseAddress + 0x50)
#define UTRLBA_RegisterSize 32
#define UTRLBA_RegisterResetValue 0x0
#define UTRLBA_RegisterResetMask 0xffffffff





#define UTRLBA_UTRLBA_RSVD_9_0_BitAddressOffset 0
#define UTRLBA_UTRLBA_RSVD_9_0_RegisterSize 10



#define UTRLBA_UTRLBA_BitAddressOffset 10
#define UTRLBA_UTRLBA_RegisterSize 22





#define UTRLBAU (DWC_ufshc_block_BaseAddress + 0x54)
#define UTRLBAU_RegisterSize 32
#define UTRLBAU_RegisterResetValue 0x0
#define UTRLBAU_RegisterResetMask 0xffffffff





#define UTRLBAU_UTRLBAU_BitAddressOffset 0
#define UTRLBAU_UTRLBAU_RegisterSize 32





#define UTRLDBR (DWC_ufshc_block_BaseAddress + 0x58)
#define UTRLDBR_RegisterSize 32
#define UTRLDBR_RegisterResetValue 0x0
#define UTRLDBR_RegisterResetMask 0xffffffff





#define UTRLDBR_UTRLDBR_BitAddressOffset 0
#define UTRLDBR_UTRLDBR_RegisterSize 32





#define UTRLCLR (DWC_ufshc_block_BaseAddress + 0x5c)
#define UTRLCLR_RegisterSize 32
#define UTRLCLR_RegisterResetValue 0x0
#define UTRLCLR_RegisterResetMask 0xffffffff





#define UTRLCLR_UTRLCLR_BitAddressOffset 0
#define UTRLCLR_UTRLCLR_RegisterSize 32





#define UTRLRSR (DWC_ufshc_block_BaseAddress + 0x60)
#define UTRLRSR_RegisterSize 32
#define UTRLRSR_RegisterResetValue 0x0
#define UTRLRSR_RegisterResetMask 0xffffffff





#define UTRLRSR_UTRLRSR_BitAddressOffset 0
#define UTRLRSR_UTRLRSR_RegisterSize 1



#define UTRLRSR_UTRLRSR_RSVD_31_1_BitAddressOffset 1
#define UTRLRSR_UTRLRSR_RSVD_31_1_RegisterSize 31





#define UTRLCNR (DWC_ufshc_block_BaseAddress + 0x64)
#define UTRLCNR_RegisterSize 32
#define UTRLCNR_RegisterResetValue 0x0
#define UTRLCNR_RegisterResetMask 0xffffffff





#define UTRLCNR_UTRLCNR_BitAddressOffset 0
#define UTRLCNR_UTRLCNR_RegisterSize 32





#define UTMRLBA (DWC_ufshc_block_BaseAddress + 0x70)
#define UTMRLBA_RegisterSize 32
#define UTMRLBA_RegisterResetValue 0x0
#define UTMRLBA_RegisterResetMask 0xffffffff





#define UTMRLBA_UTMRLBA_RSVD_9_0_BitAddressOffset 0
#define UTMRLBA_UTMRLBA_RSVD_9_0_RegisterSize 10



#define UTMRLBA_UTMRLBA_BitAddressOffset 10
#define UTMRLBA_UTMRLBA_RegisterSize 22





#define UTMRLBAU (DWC_ufshc_block_BaseAddress + 0x74)
#define UTMRLBAU_RegisterSize 32
#define UTMRLBAU_RegisterResetValue 0x0
#define UTMRLBAU_RegisterResetMask 0xffffffff





#define UTMRLBAU_UTMRLBAU_BitAddressOffset 0
#define UTMRLBAU_UTMRLBAU_RegisterSize 32





#define UTMRLDBR (DWC_ufshc_block_BaseAddress + 0x78)
#define UTMRLDBR_RegisterSize 32
#define UTMRLDBR_RegisterResetValue 0x0
#define UTMRLDBR_RegisterResetMask 0xffffffff





#define UTMRLDBR_UTMRLDBR_BitAddressOffset 0
#define UTMRLDBR_UTMRLDBR_RegisterSize 8



#define UTMRLDBR_UTMRLDBR_RSVD_31_8_BitAddressOffset 8
#define UTMRLDBR_UTMRLDBR_RSVD_31_8_RegisterSize 24





#define UTMRLCLR (DWC_ufshc_block_BaseAddress + 0x7c)
#define UTMRLCLR_RegisterSize 32
#define UTMRLCLR_RegisterResetValue 0x0
#define UTMRLCLR_RegisterResetMask 0xffffffff





#define UTMRLCLR_UTMRLCLR_BitAddressOffset 0
#define UTMRLCLR_UTMRLCLR_RegisterSize 8



#define UTMRLCLR_UTMRLCLR_RSVD_31_8_BitAddressOffset 8
#define UTMRLCLR_UTMRLCLR_RSVD_31_8_RegisterSize 24





#define UTMRLRSR (DWC_ufshc_block_BaseAddress + 0x80)
#define UTMRLRSR_RegisterSize 32
#define UTMRLRSR_RegisterResetValue 0x0
#define UTMRLRSR_RegisterResetMask 0xffffffff





#define UTMRLRSR_UTMRLRSR_BitAddressOffset 0
#define UTMRLRSR_UTMRLRSR_RegisterSize 1



#define UTMRLRSR_UTMRLRSR_RSVD_31_1_BitAddressOffset 1
#define UTMRLRSR_UTMRLRSR_RSVD_31_1_RegisterSize 31





#define UICCMD (DWC_ufshc_block_BaseAddress + 0x90)
#define UICCMD_RegisterSize 32
#define UICCMD_RegisterResetValue 0x0
#define UICCMD_RegisterResetMask 0xffffffff





#define UICCMD_CMDOP_BitAddressOffset 0
#define UICCMD_CMDOP_RegisterSize 8



#define UICCMD_UICCMD_RSVD_31_8_BitAddressOffset 8
#define UICCMD_UICCMD_RSVD_31_8_RegisterSize 24





#define UICCMDARG1 (DWC_ufshc_block_BaseAddress + 0x94)
#define UICCMDARG1_RegisterSize 32
#define UICCMDARG1_RegisterResetValue 0x0
#define UICCMDARG1_RegisterResetMask 0xffffffff





#define UICCMDARG1_ARG1_BitAddressOffset 0
#define UICCMDARG1_ARG1_RegisterSize 32





#define UICCMDARG2 (DWC_ufshc_block_BaseAddress + 0x98)
#define UICCMDARG2_RegisterSize 32
#define UICCMDARG2_RegisterResetValue 0x0
#define UICCMDARG2_RegisterResetMask 0xffffffff





#define UICCMDARG2_ARG2_7_0_BitAddressOffset 0
#define UICCMDARG2_ARG2_7_0_RegisterSize 8



#define UICCMDARG2_ARG2_23_16_BitAddressOffset 16
#define UICCMDARG2_ARG2_23_16_RegisterSize 8





#define UICCMDARG3 (DWC_ufshc_block_BaseAddress + 0x9c)
#define UICCMDARG3_RegisterSize 32
#define UICCMDARG3_RegisterResetValue 0x0
#define UICCMDARG3_RegisterResetMask 0xffffffff





#define UICCMDARG3_ARG3_BitAddressOffset 0
#define UICCMDARG3_ARG3_RegisterSize 32





#define BUSTHRTL (DWC_ufshc_block_BaseAddress + 0xc0)
#define BUSTHRTL_RegisterSize 32
#define BUSTHRTL_RegisterResetValue 0x107f7000
#define BUSTHRTL_RegisterResetMask 0xffffffff





#define BUSTHRTL_BUSDLY_BitAddressOffset 0
#define BUSTHRTL_BUSDLY_RegisterSize 8



#define BUSTHRTL_CGE_BitAddressOffset 12
#define BUSTHRTL_CGE_RegisterSize 1



#define BUSTHRTL_MBL_BitAddressOffset 13
#define BUSTHRTL_MBL_RegisterSize 3



#define BUSTHRTL_LP_PGE_BitAddressOffset 16
#define BUSTHRTL_LP_PGE_RegisterSize 1



#define BUSTHRTL_LP_AH8_PGE_BitAddressOffset 17
#define BUSTHRTL_LP_AH8_PGE_RegisterSize 1



#define BUSTHRTL_MAXOBUSR_BitAddressOffset 18
#define BUSTHRTL_MAXOBUSR_RegisterSize 6



#define BUSTHRTL_BUSTHRTL_RSVD_27_24_BitAddressOffset 24
#define BUSTHRTL_BUSTHRTL_RSVD_27_24_RegisterSize 4



#define BUSTHRTL_DME_CTRL_LOGIC_EN_BitAddressOffset 28
#define BUSTHRTL_DME_CTRL_LOGIC_EN_RegisterSize 1



#define BUSTHRTL_BUSTHRTL_RSVD_31_30_BitAddressOffset 30
#define BUSTHRTL_BUSTHRTL_RSVD_31_30_RegisterSize 2





#define FEIE (DWC_ufshc_block_BaseAddress + 0xc8)
#define FEIE_RegisterSize 32
#define FEIE_RegisterResetValue 0x0
#define FEIE_RegisterResetMask 0xffffffff





#define FEIE_VSDFEE_BitAddressOffset 11
#define FEIE_VSDFEE_RegisterSize 1



#define FEIE_FEIE_RSVD_15_13_BitAddressOffset 13
#define FEIE_FEIE_RSVD_15_13_RegisterSize 3



#define FEIE_VSHCFEE_BitAddressOffset 16
#define FEIE_VSHCFEE_RegisterSize 1



#define FEIE_VSSBFEE_BitAddressOffset 17
#define FEIE_VSSBFEE_RegisterSize 1



#define FEIE_FEIE_RSVD_26_18_BitAddressOffset 18
#define FEIE_FEIE_RSVD_26_18_RegisterSize 9





#define SECURITY (DWC_ufshc_block_BaseAddress + 0xcc)
#define SECURITY_RegisterSize 32
#define SECURITY_RegisterResetValue 0x0
#define SECURITY_RegisterResetMask 0xffffffff





#define SECURITY_KEY_ACC_SEL_BitAddressOffset 0
#define SECURITY_KEY_ACC_SEL_RegisterSize 1



#define SECURITY_SECURITY_RSVD_31_1_BitAddressOffset 1
#define SECURITY_SECURITY_RSVD_31_1_RegisterSize 31





#define UFSHC_VER_ID_REG (DWC_ufshc_block_BaseAddress + 0xd0)
#define UFSHC_VER_ID_REG_RegisterSize 32
#define UFSHC_VER_ID_REG_RegisterResetValue 0x3130312a
#define UFSHC_VER_ID_REG_RegisterResetMask 0xffffffff





#define UFSHC_VER_ID_REG_UFSHC_VER_ID_BitAddressOffset 0
#define UFSHC_VER_ID_REG_UFSHC_VER_ID_RegisterSize 32





#define UFSHC_VER_TYPE_REG (DWC_ufshc_block_BaseAddress + 0xd4)
#define UFSHC_VER_TYPE_REG_RegisterSize 32
#define UFSHC_VER_TYPE_REG_RegisterResetValue 0x65613035
#define UFSHC_VER_TYPE_REG_RegisterResetMask 0xffffffff





#define UFSHC_VER_TYPE_REG_UFSHC_VER_TYPE_BitAddressOffset 0
#define UFSHC_VER_TYPE_REG_UFSHC_VER_TYPE_RegisterSize 32





#define VS_OOO_CONFIG (DWC_ufshc_block_BaseAddress + 0xd8)
#define VS_OOO_CONFIG_RegisterSize 32
#define VS_OOO_CONFIG_RegisterResetValue 0x1
#define VS_OOO_CONFIG_RegisterResetMask 0xffffffff





#define VS_OOO_CONFIG_LBS_BitAddressOffset 0
#define VS_OOO_CONFIG_LBS_RegisterSize 4



#define VS_OOO_CONFIG_VS_OOO_CONFIG_RSVD_31_4_BitAddressOffset 4
#define VS_OOO_CONFIG_VS_OOO_CONFIG_RSVD_31_4_RegisterSize 28





#define VS_IAG (DWC_ufshc_block_BaseAddress + 0xdc)
#define VS_IAG_RegisterSize 32
#define VS_IAG_RegisterResetValue 0x0
#define VS_IAG_RegisterResetMask 0xffffffff





#define VS_IAG_IAG_ID_BitAddressOffset 0
#define VS_IAG_IAG_ID_RegisterSize 16



#define VS_IAG_VS_IAG_RSVD_BitAddressOffset 16
#define VS_IAG_VS_IAG_RSVD_RegisterSize 16





#define VS_CQES (DWC_ufshc_block_BaseAddress + 0xe0)
#define VS_CQES_RegisterSize 32
#define VS_CQES_RegisterResetValue 0x0
#define VS_CQES_RegisterResetMask 0xffffffff





#define VS_CQES_CQES_CQID_BitAddressOffset 0
#define VS_CQES_CQES_CQID_RegisterSize 16



#define VS_CQES_VS_CQES_RSVD_BitAddressOffset 16
#define VS_CQES_VS_CQES_RSVD_RegisterSize 16





#define HCLKDIV (DWC_ufshc_block_BaseAddress + 0xf8)
#define HCLKDIV_RegisterSize 32
#define HCLKDIV_RegisterResetValue 0x190
#define HCLKDIV_RegisterResetMask 0xffffffff





#define HCLKDIV_HCLKDIV_BitAddressOffset 0
#define HCLKDIV_HCLKDIV_RegisterSize 16



#define HCLKDIV_HCLKDIV_RSVD_31_16_BitAddressOffset 16
#define HCLKDIV_HCLKDIV_RSVD_31_16_RegisterSize 16





#define EXTND_VENDOR_REG_POINTER (DWC_ufshc_block_BaseAddress + 0xfc)
#define EXTND_VENDOR_REG_POINTER_RegisterSize 32
#define EXTND_VENDOR_REG_POINTER_RegisterResetValue 0x200
#define EXTND_VENDOR_REG_POINTER_RegisterResetMask 0xffffffff





#define EXTND_VENDOR_REG_POINTER_SECTION_OFFSET_BitAddressOffset 0
#define EXTND_VENDOR_REG_POINTER_SECTION_OFFSET_RegisterSize 32





#define CCAP (DWC_ufshc_block_BaseAddress + 0x100)
#define CCAP_RegisterSize 32
#define CCAP_RegisterResetValue 0x10003f02
#define CCAP_RegisterResetMask 0xffffffff





#define CCAP_CC_BitAddressOffset 0
#define CCAP_CC_RegisterSize 8



#define CCAP_CFGC_BitAddressOffset 8
#define CCAP_CFGC_RegisterSize 8



#define CCAP_CCAP_RSVD_23_16_BitAddressOffset 16
#define CCAP_CCAP_RSVD_23_16_RegisterSize 8



#define CCAP_CFGPTR_BitAddressOffset 24
#define CCAP_CFGPTR_RegisterSize 8





#define CRYPTOCAP_0 (DWC_ufshc_block_BaseAddress + 0x104)
#define CRYPTOCAP_0_RegisterSize 32
#define CRYPTOCAP_0_RegisterResetValue 0x17f00
#define CRYPTOCAP_0_RegisterResetMask 0xffffffff





#define CRYPTOCAP_0_ALGID_BitAddressOffset 0
#define CRYPTOCAP_0_ALGID_RegisterSize 8



#define CRYPTOCAP_0_SDUSB_BitAddressOffset 8
#define CRYPTOCAP_0_SDUSB_RegisterSize 8



#define CRYPTOCAP_0_KS_BitAddressOffset 16
#define CRYPTOCAP_0_KS_RegisterSize 8



#define CRYPTOCAP_0_CRYPTOCAP_0_RSVD_31_24_BitAddressOffset 24
#define CRYPTOCAP_0_CRYPTOCAP_0_RSVD_31_24_RegisterSize 8





#define CRYPTOCAP_1 (DWC_ufshc_block_BaseAddress + 0x108)
#define CRYPTOCAP_1_RegisterSize 32
#define CRYPTOCAP_1_RegisterResetValue 0x37f00
#define CRYPTOCAP_1_RegisterResetMask 0xffffffff





#define CRYPTOCAP_1_ALGID_BitAddressOffset 0
#define CRYPTOCAP_1_ALGID_RegisterSize 8



#define CRYPTOCAP_1_SDUSB_BitAddressOffset 8
#define CRYPTOCAP_1_SDUSB_RegisterSize 8



#define CRYPTOCAP_1_KS_BitAddressOffset 16
#define CRYPTOCAP_1_KS_RegisterSize 8



#define CRYPTOCAP_1_CRYPTOCAP_1_RSVD_31_24_BitAddressOffset 24
#define CRYPTOCAP_1_CRYPTOCAP_1_RSVD_31_24_RegisterSize 8





#define UFS_SFTY_MECHANISM_MONITOR_EN_REG (DWC_ufshc_block_BaseAddress + 0x214)
#define UFS_SFTY_MECHANISM_MONITOR_EN_REG_RegisterSize 32
#define UFS_SFTY_MECHANISM_MONITOR_EN_REG_RegisterResetValue 0x0
#define UFS_SFTY_MECHANISM_MONITOR_EN_REG_RegisterResetMask 0xffffffff





#define Config (DWC_ufshc_block_BaseAddress + 0x300)
#define Config_RegisterSize 32
#define Config_RegisterResetValue 0x0
#define Config_RegisterResetMask 0xffffffff





#define Config_QT_BitAddressOffset 0
#define Config_QT_RegisterSize 1



#define Config_ESIE_BitAddressOffset 1
#define Config_ESIE_RegisterSize 1



#define Config_DW2PRDTEN_BitAddressOffset 2
#define Config_DW2PRDTEN_RegisterSize 1



#define Config_Config_RSVD_31_3_BitAddressOffset 3
#define Config_Config_RSVD_31_3_RegisterSize 29





#define MCQConfig (DWC_ufshc_block_BaseAddress + 0x380)
#define MCQConfig_RegisterSize 32
#define MCQConfig_RegisterResetValue 0x1f00
#define MCQConfig_RegisterResetMask 0xffffffff





#define MCQConfig_AS_BitAddressOffset 0
#define MCQConfig_AS_RegisterSize 2



#define MCQConfig_MCQConfig_RSVD_7_2_BitAddressOffset 2
#define MCQConfig_MCQConfig_RSVD_7_2_RegisterSize 6



#define MCQConfig_MAC_BitAddressOffset 8
#define MCQConfig_MAC_RegisterSize 9



#define MCQConfig_MCQConfig_RSVD_31_17_BitAddressOffset 17
#define MCQConfig_MCQConfig_RSVD_31_17_RegisterSize 15





#define CRYPTOCFG_0_0 (DWC_ufshc_block_BaseAddress + 0x1000)
#define CRYPTOCFG_0_0_RegisterSize 32
#define CRYPTOCFG_0_0_RegisterResetValue 0x0
#define CRYPTOCFG_0_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_0_CRYPTOKEY_0_0_BitAddressOffset 0
#define CRYPTOCFG_0_0_CRYPTOKEY_0_0_RegisterSize 32





#define CRYPTOCFG_0_1 (DWC_ufshc_block_BaseAddress + 0x1004)
#define CRYPTOCFG_0_1_RegisterSize 32
#define CRYPTOCFG_0_1_RegisterResetValue 0x0
#define CRYPTOCFG_0_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_1_CRYPTOKEY_0_1_BitAddressOffset 0
#define CRYPTOCFG_0_1_CRYPTOKEY_0_1_RegisterSize 32





#define CRYPTOCFG_0_2 (DWC_ufshc_block_BaseAddress + 0x1008)
#define CRYPTOCFG_0_2_RegisterSize 32
#define CRYPTOCFG_0_2_RegisterResetValue 0x0
#define CRYPTOCFG_0_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_2_CRYPTOKEY_0_2_BitAddressOffset 0
#define CRYPTOCFG_0_2_CRYPTOKEY_0_2_RegisterSize 32





#define CRYPTOCFG_0_3 (DWC_ufshc_block_BaseAddress + 0x100c)
#define CRYPTOCFG_0_3_RegisterSize 32
#define CRYPTOCFG_0_3_RegisterResetValue 0x0
#define CRYPTOCFG_0_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_3_CRYPTOKEY_0_3_BitAddressOffset 0
#define CRYPTOCFG_0_3_CRYPTOKEY_0_3_RegisterSize 32





#define CRYPTOCFG_0_4 (DWC_ufshc_block_BaseAddress + 0x1010)
#define CRYPTOCFG_0_4_RegisterSize 32
#define CRYPTOCFG_0_4_RegisterResetValue 0x0
#define CRYPTOCFG_0_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_4_CRYPTOKEY_0_4_BitAddressOffset 0
#define CRYPTOCFG_0_4_CRYPTOKEY_0_4_RegisterSize 32





#define CRYPTOCFG_0_5 (DWC_ufshc_block_BaseAddress + 0x1014)
#define CRYPTOCFG_0_5_RegisterSize 32
#define CRYPTOCFG_0_5_RegisterResetValue 0x0
#define CRYPTOCFG_0_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_5_CRYPTOKEY_0_5_BitAddressOffset 0
#define CRYPTOCFG_0_5_CRYPTOKEY_0_5_RegisterSize 32





#define CRYPTOCFG_0_6 (DWC_ufshc_block_BaseAddress + 0x1018)
#define CRYPTOCFG_0_6_RegisterSize 32
#define CRYPTOCFG_0_6_RegisterResetValue 0x0
#define CRYPTOCFG_0_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_6_CRYPTOKEY_0_6_BitAddressOffset 0
#define CRYPTOCFG_0_6_CRYPTOKEY_0_6_RegisterSize 32





#define CRYPTOCFG_0_7 (DWC_ufshc_block_BaseAddress + 0x101c)
#define CRYPTOCFG_0_7_RegisterSize 32
#define CRYPTOCFG_0_7_RegisterResetValue 0x0
#define CRYPTOCFG_0_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_7_CRYPTOKEY_0_7_BitAddressOffset 0
#define CRYPTOCFG_0_7_CRYPTOKEY_0_7_RegisterSize 32





#define CRYPTOCFG_0_8 (DWC_ufshc_block_BaseAddress + 0x1020)
#define CRYPTOCFG_0_8_RegisterSize 32
#define CRYPTOCFG_0_8_RegisterResetValue 0x0
#define CRYPTOCFG_0_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_8_CRYPTOKEY_0_8_BitAddressOffset 0
#define CRYPTOCFG_0_8_CRYPTOKEY_0_8_RegisterSize 32





#define CRYPTOCFG_0_9 (DWC_ufshc_block_BaseAddress + 0x1024)
#define CRYPTOCFG_0_9_RegisterSize 32
#define CRYPTOCFG_0_9_RegisterResetValue 0x0
#define CRYPTOCFG_0_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_9_CRYPTOKEY_0_9_BitAddressOffset 0
#define CRYPTOCFG_0_9_CRYPTOKEY_0_9_RegisterSize 32





#define CRYPTOCFG_0_10 (DWC_ufshc_block_BaseAddress + 0x1028)
#define CRYPTOCFG_0_10_RegisterSize 32
#define CRYPTOCFG_0_10_RegisterResetValue 0x0
#define CRYPTOCFG_0_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_10_CRYPTOKEY_0_10_BitAddressOffset 0
#define CRYPTOCFG_0_10_CRYPTOKEY_0_10_RegisterSize 32





#define CRYPTOCFG_0_11 (DWC_ufshc_block_BaseAddress + 0x102c)
#define CRYPTOCFG_0_11_RegisterSize 32
#define CRYPTOCFG_0_11_RegisterResetValue 0x0
#define CRYPTOCFG_0_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_11_CRYPTOKEY_0_11_BitAddressOffset 0
#define CRYPTOCFG_0_11_CRYPTOKEY_0_11_RegisterSize 32





#define CRYPTOCFG_0_12 (DWC_ufshc_block_BaseAddress + 0x1030)
#define CRYPTOCFG_0_12_RegisterSize 32
#define CRYPTOCFG_0_12_RegisterResetValue 0x0
#define CRYPTOCFG_0_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_12_CRYPTOKEY_0_12_BitAddressOffset 0
#define CRYPTOCFG_0_12_CRYPTOKEY_0_12_RegisterSize 32





#define CRYPTOCFG_0_13 (DWC_ufshc_block_BaseAddress + 0x1034)
#define CRYPTOCFG_0_13_RegisterSize 32
#define CRYPTOCFG_0_13_RegisterResetValue 0x0
#define CRYPTOCFG_0_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_13_CRYPTOKEY_0_13_BitAddressOffset 0
#define CRYPTOCFG_0_13_CRYPTOKEY_0_13_RegisterSize 32





#define CRYPTOCFG_0_14 (DWC_ufshc_block_BaseAddress + 0x1038)
#define CRYPTOCFG_0_14_RegisterSize 32
#define CRYPTOCFG_0_14_RegisterResetValue 0x0
#define CRYPTOCFG_0_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_14_CRYPTOKEY_0_14_BitAddressOffset 0
#define CRYPTOCFG_0_14_CRYPTOKEY_0_14_RegisterSize 32





#define CRYPTOCFG_0_15 (DWC_ufshc_block_BaseAddress + 0x103c)
#define CRYPTOCFG_0_15_RegisterSize 32
#define CRYPTOCFG_0_15_RegisterResetValue 0x0
#define CRYPTOCFG_0_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_15_CRYPTOKEY_0_15_BitAddressOffset 0
#define CRYPTOCFG_0_15_CRYPTOKEY_0_15_RegisterSize 32





#define CRYPTOCFG_0_16 (DWC_ufshc_block_BaseAddress + 0x1040)
#define CRYPTOCFG_0_16_RegisterSize 32
#define CRYPTOCFG_0_16_RegisterResetValue 0x0
#define CRYPTOCFG_0_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_16_DUSIZE_0_BitAddressOffset 0
#define CRYPTOCFG_0_16_DUSIZE_0_RegisterSize 8



#define CRYPTOCFG_0_16_CAPIDX_0_BitAddressOffset 8
#define CRYPTOCFG_0_16_CAPIDX_0_RegisterSize 8



#define CRYPTOCFG_0_16_CFGE_0_BitAddressOffset 31
#define CRYPTOCFG_0_16_CFGE_0_RegisterSize 1





#define CRYPTOCFG_0_17 (DWC_ufshc_block_BaseAddress + 0x1044)
#define CRYPTOCFG_0_17_RegisterSize 32
#define CRYPTOCFG_0_17_RegisterResetValue 0x0
#define CRYPTOCFG_0_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_17_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_17_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_18 (DWC_ufshc_block_BaseAddress + 0x1048)
#define CRYPTOCFG_0_18_RegisterSize 32
#define CRYPTOCFG_0_18_RegisterResetValue 0x0
#define CRYPTOCFG_0_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_18_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_18_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_19 (DWC_ufshc_block_BaseAddress + 0x104c)
#define CRYPTOCFG_0_19_RegisterSize 32
#define CRYPTOCFG_0_19_RegisterResetValue 0x0
#define CRYPTOCFG_0_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_19_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_19_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_20 (DWC_ufshc_block_BaseAddress + 0x1050)
#define CRYPTOCFG_0_20_RegisterSize 32
#define CRYPTOCFG_0_20_RegisterResetValue 0x0
#define CRYPTOCFG_0_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_20_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_20_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_21 (DWC_ufshc_block_BaseAddress + 0x1054)
#define CRYPTOCFG_0_21_RegisterSize 32
#define CRYPTOCFG_0_21_RegisterResetValue 0x0
#define CRYPTOCFG_0_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_21_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_21_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_22 (DWC_ufshc_block_BaseAddress + 0x1058)
#define CRYPTOCFG_0_22_RegisterSize 32
#define CRYPTOCFG_0_22_RegisterResetValue 0x0
#define CRYPTOCFG_0_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_22_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_22_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_23 (DWC_ufshc_block_BaseAddress + 0x105c)
#define CRYPTOCFG_0_23_RegisterSize 32
#define CRYPTOCFG_0_23_RegisterResetValue 0x0
#define CRYPTOCFG_0_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_23_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_23_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_24 (DWC_ufshc_block_BaseAddress + 0x1060)
#define CRYPTOCFG_0_24_RegisterSize 32
#define CRYPTOCFG_0_24_RegisterResetValue 0x0
#define CRYPTOCFG_0_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_24_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_24_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_25 (DWC_ufshc_block_BaseAddress + 0x1064)
#define CRYPTOCFG_0_25_RegisterSize 32
#define CRYPTOCFG_0_25_RegisterResetValue 0x0
#define CRYPTOCFG_0_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_25_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_25_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_26 (DWC_ufshc_block_BaseAddress + 0x1068)
#define CRYPTOCFG_0_26_RegisterSize 32
#define CRYPTOCFG_0_26_RegisterResetValue 0x0
#define CRYPTOCFG_0_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_26_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_26_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_27 (DWC_ufshc_block_BaseAddress + 0x106c)
#define CRYPTOCFG_0_27_RegisterSize 32
#define CRYPTOCFG_0_27_RegisterResetValue 0x0
#define CRYPTOCFG_0_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_27_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_27_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_28 (DWC_ufshc_block_BaseAddress + 0x1070)
#define CRYPTOCFG_0_28_RegisterSize 32
#define CRYPTOCFG_0_28_RegisterResetValue 0x0
#define CRYPTOCFG_0_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_28_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_28_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_29 (DWC_ufshc_block_BaseAddress + 0x1074)
#define CRYPTOCFG_0_29_RegisterSize 32
#define CRYPTOCFG_0_29_RegisterResetValue 0x0
#define CRYPTOCFG_0_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_29_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_29_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_30 (DWC_ufshc_block_BaseAddress + 0x1078)
#define CRYPTOCFG_0_30_RegisterSize 32
#define CRYPTOCFG_0_30_RegisterResetValue 0x0
#define CRYPTOCFG_0_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_30_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_30_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_0_31 (DWC_ufshc_block_BaseAddress + 0x107c)
#define CRYPTOCFG_0_31_RegisterSize 32
#define CRYPTOCFG_0_31_RegisterResetValue 0x0
#define CRYPTOCFG_0_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_0_31_RESERVED_0_BitAddressOffset 0
#define CRYPTOCFG_0_31_RESERVED_0_RegisterSize 32





#define CRYPTOCFG_1_0 (DWC_ufshc_block_BaseAddress + 0x1080)
#define CRYPTOCFG_1_0_RegisterSize 32
#define CRYPTOCFG_1_0_RegisterResetValue 0x0
#define CRYPTOCFG_1_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_0_CRYPTOKEY_1_0_BitAddressOffset 0
#define CRYPTOCFG_1_0_CRYPTOKEY_1_0_RegisterSize 32





#define CRYPTOCFG_1_1 (DWC_ufshc_block_BaseAddress + 0x1084)
#define CRYPTOCFG_1_1_RegisterSize 32
#define CRYPTOCFG_1_1_RegisterResetValue 0x0
#define CRYPTOCFG_1_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_1_CRYPTOKEY_1_1_BitAddressOffset 0
#define CRYPTOCFG_1_1_CRYPTOKEY_1_1_RegisterSize 32





#define CRYPTOCFG_1_2 (DWC_ufshc_block_BaseAddress + 0x1088)
#define CRYPTOCFG_1_2_RegisterSize 32
#define CRYPTOCFG_1_2_RegisterResetValue 0x0
#define CRYPTOCFG_1_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_2_CRYPTOKEY_1_2_BitAddressOffset 0
#define CRYPTOCFG_1_2_CRYPTOKEY_1_2_RegisterSize 32





#define CRYPTOCFG_1_3 (DWC_ufshc_block_BaseAddress + 0x108c)
#define CRYPTOCFG_1_3_RegisterSize 32
#define CRYPTOCFG_1_3_RegisterResetValue 0x0
#define CRYPTOCFG_1_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_3_CRYPTOKEY_1_3_BitAddressOffset 0
#define CRYPTOCFG_1_3_CRYPTOKEY_1_3_RegisterSize 32





#define CRYPTOCFG_1_4 (DWC_ufshc_block_BaseAddress + 0x1090)
#define CRYPTOCFG_1_4_RegisterSize 32
#define CRYPTOCFG_1_4_RegisterResetValue 0x0
#define CRYPTOCFG_1_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_4_CRYPTOKEY_1_4_BitAddressOffset 0
#define CRYPTOCFG_1_4_CRYPTOKEY_1_4_RegisterSize 32





#define CRYPTOCFG_1_5 (DWC_ufshc_block_BaseAddress + 0x1094)
#define CRYPTOCFG_1_5_RegisterSize 32
#define CRYPTOCFG_1_5_RegisterResetValue 0x0
#define CRYPTOCFG_1_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_5_CRYPTOKEY_1_5_BitAddressOffset 0
#define CRYPTOCFG_1_5_CRYPTOKEY_1_5_RegisterSize 32





#define CRYPTOCFG_1_6 (DWC_ufshc_block_BaseAddress + 0x1098)
#define CRYPTOCFG_1_6_RegisterSize 32
#define CRYPTOCFG_1_6_RegisterResetValue 0x0
#define CRYPTOCFG_1_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_6_CRYPTOKEY_1_6_BitAddressOffset 0
#define CRYPTOCFG_1_6_CRYPTOKEY_1_6_RegisterSize 32





#define CRYPTOCFG_1_7 (DWC_ufshc_block_BaseAddress + 0x109c)
#define CRYPTOCFG_1_7_RegisterSize 32
#define CRYPTOCFG_1_7_RegisterResetValue 0x0
#define CRYPTOCFG_1_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_7_CRYPTOKEY_1_7_BitAddressOffset 0
#define CRYPTOCFG_1_7_CRYPTOKEY_1_7_RegisterSize 32





#define CRYPTOCFG_1_8 (DWC_ufshc_block_BaseAddress + 0x10a0)
#define CRYPTOCFG_1_8_RegisterSize 32
#define CRYPTOCFG_1_8_RegisterResetValue 0x0
#define CRYPTOCFG_1_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_8_CRYPTOKEY_1_8_BitAddressOffset 0
#define CRYPTOCFG_1_8_CRYPTOKEY_1_8_RegisterSize 32





#define CRYPTOCFG_1_9 (DWC_ufshc_block_BaseAddress + 0x10a4)
#define CRYPTOCFG_1_9_RegisterSize 32
#define CRYPTOCFG_1_9_RegisterResetValue 0x0
#define CRYPTOCFG_1_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_9_CRYPTOKEY_1_9_BitAddressOffset 0
#define CRYPTOCFG_1_9_CRYPTOKEY_1_9_RegisterSize 32





#define CRYPTOCFG_1_10 (DWC_ufshc_block_BaseAddress + 0x10a8)
#define CRYPTOCFG_1_10_RegisterSize 32
#define CRYPTOCFG_1_10_RegisterResetValue 0x0
#define CRYPTOCFG_1_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_10_CRYPTOKEY_1_10_BitAddressOffset 0
#define CRYPTOCFG_1_10_CRYPTOKEY_1_10_RegisterSize 32





#define CRYPTOCFG_1_11 (DWC_ufshc_block_BaseAddress + 0x10ac)
#define CRYPTOCFG_1_11_RegisterSize 32
#define CRYPTOCFG_1_11_RegisterResetValue 0x0
#define CRYPTOCFG_1_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_11_CRYPTOKEY_1_11_BitAddressOffset 0
#define CRYPTOCFG_1_11_CRYPTOKEY_1_11_RegisterSize 32





#define CRYPTOCFG_1_12 (DWC_ufshc_block_BaseAddress + 0x10b0)
#define CRYPTOCFG_1_12_RegisterSize 32
#define CRYPTOCFG_1_12_RegisterResetValue 0x0
#define CRYPTOCFG_1_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_12_CRYPTOKEY_1_12_BitAddressOffset 0
#define CRYPTOCFG_1_12_CRYPTOKEY_1_12_RegisterSize 32





#define CRYPTOCFG_1_13 (DWC_ufshc_block_BaseAddress + 0x10b4)
#define CRYPTOCFG_1_13_RegisterSize 32
#define CRYPTOCFG_1_13_RegisterResetValue 0x0
#define CRYPTOCFG_1_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_13_CRYPTOKEY_1_13_BitAddressOffset 0
#define CRYPTOCFG_1_13_CRYPTOKEY_1_13_RegisterSize 32





#define CRYPTOCFG_1_14 (DWC_ufshc_block_BaseAddress + 0x10b8)
#define CRYPTOCFG_1_14_RegisterSize 32
#define CRYPTOCFG_1_14_RegisterResetValue 0x0
#define CRYPTOCFG_1_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_14_CRYPTOKEY_1_14_BitAddressOffset 0
#define CRYPTOCFG_1_14_CRYPTOKEY_1_14_RegisterSize 32





#define CRYPTOCFG_1_15 (DWC_ufshc_block_BaseAddress + 0x10bc)
#define CRYPTOCFG_1_15_RegisterSize 32
#define CRYPTOCFG_1_15_RegisterResetValue 0x0
#define CRYPTOCFG_1_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_15_CRYPTOKEY_1_15_BitAddressOffset 0
#define CRYPTOCFG_1_15_CRYPTOKEY_1_15_RegisterSize 32





#define CRYPTOCFG_1_16 (DWC_ufshc_block_BaseAddress + 0x10c0)
#define CRYPTOCFG_1_16_RegisterSize 32
#define CRYPTOCFG_1_16_RegisterResetValue 0x0
#define CRYPTOCFG_1_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_16_DUSIZE_1_BitAddressOffset 0
#define CRYPTOCFG_1_16_DUSIZE_1_RegisterSize 8



#define CRYPTOCFG_1_16_CAPIDX_1_BitAddressOffset 8
#define CRYPTOCFG_1_16_CAPIDX_1_RegisterSize 8



#define CRYPTOCFG_1_16_CFGE_1_BitAddressOffset 31
#define CRYPTOCFG_1_16_CFGE_1_RegisterSize 1





#define CRYPTOCFG_1_17 (DWC_ufshc_block_BaseAddress + 0x10c4)
#define CRYPTOCFG_1_17_RegisterSize 32
#define CRYPTOCFG_1_17_RegisterResetValue 0x0
#define CRYPTOCFG_1_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_17_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_17_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_18 (DWC_ufshc_block_BaseAddress + 0x10c8)
#define CRYPTOCFG_1_18_RegisterSize 32
#define CRYPTOCFG_1_18_RegisterResetValue 0x0
#define CRYPTOCFG_1_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_18_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_18_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_19 (DWC_ufshc_block_BaseAddress + 0x10cc)
#define CRYPTOCFG_1_19_RegisterSize 32
#define CRYPTOCFG_1_19_RegisterResetValue 0x0
#define CRYPTOCFG_1_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_19_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_19_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_20 (DWC_ufshc_block_BaseAddress + 0x10d0)
#define CRYPTOCFG_1_20_RegisterSize 32
#define CRYPTOCFG_1_20_RegisterResetValue 0x0
#define CRYPTOCFG_1_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_20_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_20_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_21 (DWC_ufshc_block_BaseAddress + 0x10d4)
#define CRYPTOCFG_1_21_RegisterSize 32
#define CRYPTOCFG_1_21_RegisterResetValue 0x0
#define CRYPTOCFG_1_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_21_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_21_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_22 (DWC_ufshc_block_BaseAddress + 0x10d8)
#define CRYPTOCFG_1_22_RegisterSize 32
#define CRYPTOCFG_1_22_RegisterResetValue 0x0
#define CRYPTOCFG_1_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_22_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_22_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_23 (DWC_ufshc_block_BaseAddress + 0x10dc)
#define CRYPTOCFG_1_23_RegisterSize 32
#define CRYPTOCFG_1_23_RegisterResetValue 0x0
#define CRYPTOCFG_1_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_23_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_23_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_24 (DWC_ufshc_block_BaseAddress + 0x10e0)
#define CRYPTOCFG_1_24_RegisterSize 32
#define CRYPTOCFG_1_24_RegisterResetValue 0x0
#define CRYPTOCFG_1_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_24_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_24_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_25 (DWC_ufshc_block_BaseAddress + 0x10e4)
#define CRYPTOCFG_1_25_RegisterSize 32
#define CRYPTOCFG_1_25_RegisterResetValue 0x0
#define CRYPTOCFG_1_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_25_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_25_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_26 (DWC_ufshc_block_BaseAddress + 0x10e8)
#define CRYPTOCFG_1_26_RegisterSize 32
#define CRYPTOCFG_1_26_RegisterResetValue 0x0
#define CRYPTOCFG_1_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_26_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_26_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_27 (DWC_ufshc_block_BaseAddress + 0x10ec)
#define CRYPTOCFG_1_27_RegisterSize 32
#define CRYPTOCFG_1_27_RegisterResetValue 0x0
#define CRYPTOCFG_1_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_27_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_27_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_28 (DWC_ufshc_block_BaseAddress + 0x10f0)
#define CRYPTOCFG_1_28_RegisterSize 32
#define CRYPTOCFG_1_28_RegisterResetValue 0x0
#define CRYPTOCFG_1_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_28_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_28_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_29 (DWC_ufshc_block_BaseAddress + 0x10f4)
#define CRYPTOCFG_1_29_RegisterSize 32
#define CRYPTOCFG_1_29_RegisterResetValue 0x0
#define CRYPTOCFG_1_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_29_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_29_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_30 (DWC_ufshc_block_BaseAddress + 0x10f8)
#define CRYPTOCFG_1_30_RegisterSize 32
#define CRYPTOCFG_1_30_RegisterResetValue 0x0
#define CRYPTOCFG_1_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_30_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_30_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_1_31 (DWC_ufshc_block_BaseAddress + 0x10fc)
#define CRYPTOCFG_1_31_RegisterSize 32
#define CRYPTOCFG_1_31_RegisterResetValue 0x0
#define CRYPTOCFG_1_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_1_31_RESERVED_1_BitAddressOffset 0
#define CRYPTOCFG_1_31_RESERVED_1_RegisterSize 32





#define CRYPTOCFG_2_0 (DWC_ufshc_block_BaseAddress + 0x1100)
#define CRYPTOCFG_2_0_RegisterSize 32
#define CRYPTOCFG_2_0_RegisterResetValue 0x0
#define CRYPTOCFG_2_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_0_CRYPTOKEY_2_0_BitAddressOffset 0
#define CRYPTOCFG_2_0_CRYPTOKEY_2_0_RegisterSize 32





#define CRYPTOCFG_2_1 (DWC_ufshc_block_BaseAddress + 0x1104)
#define CRYPTOCFG_2_1_RegisterSize 32
#define CRYPTOCFG_2_1_RegisterResetValue 0x0
#define CRYPTOCFG_2_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_1_CRYPTOKEY_2_1_BitAddressOffset 0
#define CRYPTOCFG_2_1_CRYPTOKEY_2_1_RegisterSize 32





#define CRYPTOCFG_2_2 (DWC_ufshc_block_BaseAddress + 0x1108)
#define CRYPTOCFG_2_2_RegisterSize 32
#define CRYPTOCFG_2_2_RegisterResetValue 0x0
#define CRYPTOCFG_2_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_2_CRYPTOKEY_2_2_BitAddressOffset 0
#define CRYPTOCFG_2_2_CRYPTOKEY_2_2_RegisterSize 32





#define CRYPTOCFG_2_3 (DWC_ufshc_block_BaseAddress + 0x110c)
#define CRYPTOCFG_2_3_RegisterSize 32
#define CRYPTOCFG_2_3_RegisterResetValue 0x0
#define CRYPTOCFG_2_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_3_CRYPTOKEY_2_3_BitAddressOffset 0
#define CRYPTOCFG_2_3_CRYPTOKEY_2_3_RegisterSize 32





#define CRYPTOCFG_2_4 (DWC_ufshc_block_BaseAddress + 0x1110)
#define CRYPTOCFG_2_4_RegisterSize 32
#define CRYPTOCFG_2_4_RegisterResetValue 0x0
#define CRYPTOCFG_2_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_4_CRYPTOKEY_2_4_BitAddressOffset 0
#define CRYPTOCFG_2_4_CRYPTOKEY_2_4_RegisterSize 32





#define CRYPTOCFG_2_5 (DWC_ufshc_block_BaseAddress + 0x1114)
#define CRYPTOCFG_2_5_RegisterSize 32
#define CRYPTOCFG_2_5_RegisterResetValue 0x0
#define CRYPTOCFG_2_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_5_CRYPTOKEY_2_5_BitAddressOffset 0
#define CRYPTOCFG_2_5_CRYPTOKEY_2_5_RegisterSize 32





#define CRYPTOCFG_2_6 (DWC_ufshc_block_BaseAddress + 0x1118)
#define CRYPTOCFG_2_6_RegisterSize 32
#define CRYPTOCFG_2_6_RegisterResetValue 0x0
#define CRYPTOCFG_2_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_6_CRYPTOKEY_2_6_BitAddressOffset 0
#define CRYPTOCFG_2_6_CRYPTOKEY_2_6_RegisterSize 32





#define CRYPTOCFG_2_7 (DWC_ufshc_block_BaseAddress + 0x111c)
#define CRYPTOCFG_2_7_RegisterSize 32
#define CRYPTOCFG_2_7_RegisterResetValue 0x0
#define CRYPTOCFG_2_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_7_CRYPTOKEY_2_7_BitAddressOffset 0
#define CRYPTOCFG_2_7_CRYPTOKEY_2_7_RegisterSize 32





#define CRYPTOCFG_2_8 (DWC_ufshc_block_BaseAddress + 0x1120)
#define CRYPTOCFG_2_8_RegisterSize 32
#define CRYPTOCFG_2_8_RegisterResetValue 0x0
#define CRYPTOCFG_2_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_8_CRYPTOKEY_2_8_BitAddressOffset 0
#define CRYPTOCFG_2_8_CRYPTOKEY_2_8_RegisterSize 32





#define CRYPTOCFG_2_9 (DWC_ufshc_block_BaseAddress + 0x1124)
#define CRYPTOCFG_2_9_RegisterSize 32
#define CRYPTOCFG_2_9_RegisterResetValue 0x0
#define CRYPTOCFG_2_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_9_CRYPTOKEY_2_9_BitAddressOffset 0
#define CRYPTOCFG_2_9_CRYPTOKEY_2_9_RegisterSize 32





#define CRYPTOCFG_2_10 (DWC_ufshc_block_BaseAddress + 0x1128)
#define CRYPTOCFG_2_10_RegisterSize 32
#define CRYPTOCFG_2_10_RegisterResetValue 0x0
#define CRYPTOCFG_2_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_10_CRYPTOKEY_2_10_BitAddressOffset 0
#define CRYPTOCFG_2_10_CRYPTOKEY_2_10_RegisterSize 32





#define CRYPTOCFG_2_11 (DWC_ufshc_block_BaseAddress + 0x112c)
#define CRYPTOCFG_2_11_RegisterSize 32
#define CRYPTOCFG_2_11_RegisterResetValue 0x0
#define CRYPTOCFG_2_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_11_CRYPTOKEY_2_11_BitAddressOffset 0
#define CRYPTOCFG_2_11_CRYPTOKEY_2_11_RegisterSize 32





#define CRYPTOCFG_2_12 (DWC_ufshc_block_BaseAddress + 0x1130)
#define CRYPTOCFG_2_12_RegisterSize 32
#define CRYPTOCFG_2_12_RegisterResetValue 0x0
#define CRYPTOCFG_2_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_12_CRYPTOKEY_2_12_BitAddressOffset 0
#define CRYPTOCFG_2_12_CRYPTOKEY_2_12_RegisterSize 32





#define CRYPTOCFG_2_13 (DWC_ufshc_block_BaseAddress + 0x1134)
#define CRYPTOCFG_2_13_RegisterSize 32
#define CRYPTOCFG_2_13_RegisterResetValue 0x0
#define CRYPTOCFG_2_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_13_CRYPTOKEY_2_13_BitAddressOffset 0
#define CRYPTOCFG_2_13_CRYPTOKEY_2_13_RegisterSize 32





#define CRYPTOCFG_2_14 (DWC_ufshc_block_BaseAddress + 0x1138)
#define CRYPTOCFG_2_14_RegisterSize 32
#define CRYPTOCFG_2_14_RegisterResetValue 0x0
#define CRYPTOCFG_2_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_14_CRYPTOKEY_2_14_BitAddressOffset 0
#define CRYPTOCFG_2_14_CRYPTOKEY_2_14_RegisterSize 32





#define CRYPTOCFG_2_15 (DWC_ufshc_block_BaseAddress + 0x113c)
#define CRYPTOCFG_2_15_RegisterSize 32
#define CRYPTOCFG_2_15_RegisterResetValue 0x0
#define CRYPTOCFG_2_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_15_CRYPTOKEY_2_15_BitAddressOffset 0
#define CRYPTOCFG_2_15_CRYPTOKEY_2_15_RegisterSize 32





#define CRYPTOCFG_2_16 (DWC_ufshc_block_BaseAddress + 0x1140)
#define CRYPTOCFG_2_16_RegisterSize 32
#define CRYPTOCFG_2_16_RegisterResetValue 0x0
#define CRYPTOCFG_2_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_16_DUSIZE_2_BitAddressOffset 0
#define CRYPTOCFG_2_16_DUSIZE_2_RegisterSize 8



#define CRYPTOCFG_2_16_CAPIDX_2_BitAddressOffset 8
#define CRYPTOCFG_2_16_CAPIDX_2_RegisterSize 8



#define CRYPTOCFG_2_16_CFGE_2_BitAddressOffset 31
#define CRYPTOCFG_2_16_CFGE_2_RegisterSize 1





#define CRYPTOCFG_2_17 (DWC_ufshc_block_BaseAddress + 0x1144)
#define CRYPTOCFG_2_17_RegisterSize 32
#define CRYPTOCFG_2_17_RegisterResetValue 0x0
#define CRYPTOCFG_2_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_17_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_17_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_18 (DWC_ufshc_block_BaseAddress + 0x1148)
#define CRYPTOCFG_2_18_RegisterSize 32
#define CRYPTOCFG_2_18_RegisterResetValue 0x0
#define CRYPTOCFG_2_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_18_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_18_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_19 (DWC_ufshc_block_BaseAddress + 0x114c)
#define CRYPTOCFG_2_19_RegisterSize 32
#define CRYPTOCFG_2_19_RegisterResetValue 0x0
#define CRYPTOCFG_2_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_19_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_19_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_20 (DWC_ufshc_block_BaseAddress + 0x1150)
#define CRYPTOCFG_2_20_RegisterSize 32
#define CRYPTOCFG_2_20_RegisterResetValue 0x0
#define CRYPTOCFG_2_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_20_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_20_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_21 (DWC_ufshc_block_BaseAddress + 0x1154)
#define CRYPTOCFG_2_21_RegisterSize 32
#define CRYPTOCFG_2_21_RegisterResetValue 0x0
#define CRYPTOCFG_2_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_21_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_21_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_22 (DWC_ufshc_block_BaseAddress + 0x1158)
#define CRYPTOCFG_2_22_RegisterSize 32
#define CRYPTOCFG_2_22_RegisterResetValue 0x0
#define CRYPTOCFG_2_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_22_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_22_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_23 (DWC_ufshc_block_BaseAddress + 0x115c)
#define CRYPTOCFG_2_23_RegisterSize 32
#define CRYPTOCFG_2_23_RegisterResetValue 0x0
#define CRYPTOCFG_2_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_23_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_23_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_24 (DWC_ufshc_block_BaseAddress + 0x1160)
#define CRYPTOCFG_2_24_RegisterSize 32
#define CRYPTOCFG_2_24_RegisterResetValue 0x0
#define CRYPTOCFG_2_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_24_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_24_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_25 (DWC_ufshc_block_BaseAddress + 0x1164)
#define CRYPTOCFG_2_25_RegisterSize 32
#define CRYPTOCFG_2_25_RegisterResetValue 0x0
#define CRYPTOCFG_2_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_25_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_25_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_26 (DWC_ufshc_block_BaseAddress + 0x1168)
#define CRYPTOCFG_2_26_RegisterSize 32
#define CRYPTOCFG_2_26_RegisterResetValue 0x0
#define CRYPTOCFG_2_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_26_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_26_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_27 (DWC_ufshc_block_BaseAddress + 0x116c)
#define CRYPTOCFG_2_27_RegisterSize 32
#define CRYPTOCFG_2_27_RegisterResetValue 0x0
#define CRYPTOCFG_2_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_27_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_27_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_28 (DWC_ufshc_block_BaseAddress + 0x1170)
#define CRYPTOCFG_2_28_RegisterSize 32
#define CRYPTOCFG_2_28_RegisterResetValue 0x0
#define CRYPTOCFG_2_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_28_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_28_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_29 (DWC_ufshc_block_BaseAddress + 0x1174)
#define CRYPTOCFG_2_29_RegisterSize 32
#define CRYPTOCFG_2_29_RegisterResetValue 0x0
#define CRYPTOCFG_2_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_29_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_29_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_30 (DWC_ufshc_block_BaseAddress + 0x1178)
#define CRYPTOCFG_2_30_RegisterSize 32
#define CRYPTOCFG_2_30_RegisterResetValue 0x0
#define CRYPTOCFG_2_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_30_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_30_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_2_31 (DWC_ufshc_block_BaseAddress + 0x117c)
#define CRYPTOCFG_2_31_RegisterSize 32
#define CRYPTOCFG_2_31_RegisterResetValue 0x0
#define CRYPTOCFG_2_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_2_31_RESERVED_2_BitAddressOffset 0
#define CRYPTOCFG_2_31_RESERVED_2_RegisterSize 32





#define CRYPTOCFG_3_0 (DWC_ufshc_block_BaseAddress + 0x1180)
#define CRYPTOCFG_3_0_RegisterSize 32
#define CRYPTOCFG_3_0_RegisterResetValue 0x0
#define CRYPTOCFG_3_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_0_CRYPTOKEY_3_0_BitAddressOffset 0
#define CRYPTOCFG_3_0_CRYPTOKEY_3_0_RegisterSize 32





#define CRYPTOCFG_3_1 (DWC_ufshc_block_BaseAddress + 0x1184)
#define CRYPTOCFG_3_1_RegisterSize 32
#define CRYPTOCFG_3_1_RegisterResetValue 0x0
#define CRYPTOCFG_3_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_1_CRYPTOKEY_3_1_BitAddressOffset 0
#define CRYPTOCFG_3_1_CRYPTOKEY_3_1_RegisterSize 32





#define CRYPTOCFG_3_2 (DWC_ufshc_block_BaseAddress + 0x1188)
#define CRYPTOCFG_3_2_RegisterSize 32
#define CRYPTOCFG_3_2_RegisterResetValue 0x0
#define CRYPTOCFG_3_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_2_CRYPTOKEY_3_2_BitAddressOffset 0
#define CRYPTOCFG_3_2_CRYPTOKEY_3_2_RegisterSize 32





#define CRYPTOCFG_3_3 (DWC_ufshc_block_BaseAddress + 0x118c)
#define CRYPTOCFG_3_3_RegisterSize 32
#define CRYPTOCFG_3_3_RegisterResetValue 0x0
#define CRYPTOCFG_3_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_3_CRYPTOKEY_3_3_BitAddressOffset 0
#define CRYPTOCFG_3_3_CRYPTOKEY_3_3_RegisterSize 32





#define CRYPTOCFG_3_4 (DWC_ufshc_block_BaseAddress + 0x1190)
#define CRYPTOCFG_3_4_RegisterSize 32
#define CRYPTOCFG_3_4_RegisterResetValue 0x0
#define CRYPTOCFG_3_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_4_CRYPTOKEY_3_4_BitAddressOffset 0
#define CRYPTOCFG_3_4_CRYPTOKEY_3_4_RegisterSize 32





#define CRYPTOCFG_3_5 (DWC_ufshc_block_BaseAddress + 0x1194)
#define CRYPTOCFG_3_5_RegisterSize 32
#define CRYPTOCFG_3_5_RegisterResetValue 0x0
#define CRYPTOCFG_3_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_5_CRYPTOKEY_3_5_BitAddressOffset 0
#define CRYPTOCFG_3_5_CRYPTOKEY_3_5_RegisterSize 32





#define CRYPTOCFG_3_6 (DWC_ufshc_block_BaseAddress + 0x1198)
#define CRYPTOCFG_3_6_RegisterSize 32
#define CRYPTOCFG_3_6_RegisterResetValue 0x0
#define CRYPTOCFG_3_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_6_CRYPTOKEY_3_6_BitAddressOffset 0
#define CRYPTOCFG_3_6_CRYPTOKEY_3_6_RegisterSize 32





#define CRYPTOCFG_3_7 (DWC_ufshc_block_BaseAddress + 0x119c)
#define CRYPTOCFG_3_7_RegisterSize 32
#define CRYPTOCFG_3_7_RegisterResetValue 0x0
#define CRYPTOCFG_3_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_7_CRYPTOKEY_3_7_BitAddressOffset 0
#define CRYPTOCFG_3_7_CRYPTOKEY_3_7_RegisterSize 32





#define CRYPTOCFG_3_8 (DWC_ufshc_block_BaseAddress + 0x11a0)
#define CRYPTOCFG_3_8_RegisterSize 32
#define CRYPTOCFG_3_8_RegisterResetValue 0x0
#define CRYPTOCFG_3_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_8_CRYPTOKEY_3_8_BitAddressOffset 0
#define CRYPTOCFG_3_8_CRYPTOKEY_3_8_RegisterSize 32





#define CRYPTOCFG_3_9 (DWC_ufshc_block_BaseAddress + 0x11a4)
#define CRYPTOCFG_3_9_RegisterSize 32
#define CRYPTOCFG_3_9_RegisterResetValue 0x0
#define CRYPTOCFG_3_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_9_CRYPTOKEY_3_9_BitAddressOffset 0
#define CRYPTOCFG_3_9_CRYPTOKEY_3_9_RegisterSize 32





#define CRYPTOCFG_3_10 (DWC_ufshc_block_BaseAddress + 0x11a8)
#define CRYPTOCFG_3_10_RegisterSize 32
#define CRYPTOCFG_3_10_RegisterResetValue 0x0
#define CRYPTOCFG_3_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_10_CRYPTOKEY_3_10_BitAddressOffset 0
#define CRYPTOCFG_3_10_CRYPTOKEY_3_10_RegisterSize 32





#define CRYPTOCFG_3_11 (DWC_ufshc_block_BaseAddress + 0x11ac)
#define CRYPTOCFG_3_11_RegisterSize 32
#define CRYPTOCFG_3_11_RegisterResetValue 0x0
#define CRYPTOCFG_3_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_11_CRYPTOKEY_3_11_BitAddressOffset 0
#define CRYPTOCFG_3_11_CRYPTOKEY_3_11_RegisterSize 32





#define CRYPTOCFG_3_12 (DWC_ufshc_block_BaseAddress + 0x11b0)
#define CRYPTOCFG_3_12_RegisterSize 32
#define CRYPTOCFG_3_12_RegisterResetValue 0x0
#define CRYPTOCFG_3_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_12_CRYPTOKEY_3_12_BitAddressOffset 0
#define CRYPTOCFG_3_12_CRYPTOKEY_3_12_RegisterSize 32





#define CRYPTOCFG_3_13 (DWC_ufshc_block_BaseAddress + 0x11b4)
#define CRYPTOCFG_3_13_RegisterSize 32
#define CRYPTOCFG_3_13_RegisterResetValue 0x0
#define CRYPTOCFG_3_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_13_CRYPTOKEY_3_13_BitAddressOffset 0
#define CRYPTOCFG_3_13_CRYPTOKEY_3_13_RegisterSize 32





#define CRYPTOCFG_3_14 (DWC_ufshc_block_BaseAddress + 0x11b8)
#define CRYPTOCFG_3_14_RegisterSize 32
#define CRYPTOCFG_3_14_RegisterResetValue 0x0
#define CRYPTOCFG_3_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_14_CRYPTOKEY_3_14_BitAddressOffset 0
#define CRYPTOCFG_3_14_CRYPTOKEY_3_14_RegisterSize 32





#define CRYPTOCFG_3_15 (DWC_ufshc_block_BaseAddress + 0x11bc)
#define CRYPTOCFG_3_15_RegisterSize 32
#define CRYPTOCFG_3_15_RegisterResetValue 0x0
#define CRYPTOCFG_3_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_15_CRYPTOKEY_3_15_BitAddressOffset 0
#define CRYPTOCFG_3_15_CRYPTOKEY_3_15_RegisterSize 32





#define CRYPTOCFG_3_16 (DWC_ufshc_block_BaseAddress + 0x11c0)
#define CRYPTOCFG_3_16_RegisterSize 32
#define CRYPTOCFG_3_16_RegisterResetValue 0x0
#define CRYPTOCFG_3_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_16_DUSIZE_3_BitAddressOffset 0
#define CRYPTOCFG_3_16_DUSIZE_3_RegisterSize 8



#define CRYPTOCFG_3_16_CAPIDX_3_BitAddressOffset 8
#define CRYPTOCFG_3_16_CAPIDX_3_RegisterSize 8



#define CRYPTOCFG_3_16_CFGE_3_BitAddressOffset 31
#define CRYPTOCFG_3_16_CFGE_3_RegisterSize 1





#define CRYPTOCFG_3_17 (DWC_ufshc_block_BaseAddress + 0x11c4)
#define CRYPTOCFG_3_17_RegisterSize 32
#define CRYPTOCFG_3_17_RegisterResetValue 0x0
#define CRYPTOCFG_3_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_17_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_17_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_18 (DWC_ufshc_block_BaseAddress + 0x11c8)
#define CRYPTOCFG_3_18_RegisterSize 32
#define CRYPTOCFG_3_18_RegisterResetValue 0x0
#define CRYPTOCFG_3_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_18_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_18_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_19 (DWC_ufshc_block_BaseAddress + 0x11cc)
#define CRYPTOCFG_3_19_RegisterSize 32
#define CRYPTOCFG_3_19_RegisterResetValue 0x0
#define CRYPTOCFG_3_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_19_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_19_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_20 (DWC_ufshc_block_BaseAddress + 0x11d0)
#define CRYPTOCFG_3_20_RegisterSize 32
#define CRYPTOCFG_3_20_RegisterResetValue 0x0
#define CRYPTOCFG_3_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_20_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_20_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_21 (DWC_ufshc_block_BaseAddress + 0x11d4)
#define CRYPTOCFG_3_21_RegisterSize 32
#define CRYPTOCFG_3_21_RegisterResetValue 0x0
#define CRYPTOCFG_3_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_21_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_21_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_22 (DWC_ufshc_block_BaseAddress + 0x11d8)
#define CRYPTOCFG_3_22_RegisterSize 32
#define CRYPTOCFG_3_22_RegisterResetValue 0x0
#define CRYPTOCFG_3_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_22_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_22_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_23 (DWC_ufshc_block_BaseAddress + 0x11dc)
#define CRYPTOCFG_3_23_RegisterSize 32
#define CRYPTOCFG_3_23_RegisterResetValue 0x0
#define CRYPTOCFG_3_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_23_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_23_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_24 (DWC_ufshc_block_BaseAddress + 0x11e0)
#define CRYPTOCFG_3_24_RegisterSize 32
#define CRYPTOCFG_3_24_RegisterResetValue 0x0
#define CRYPTOCFG_3_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_24_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_24_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_25 (DWC_ufshc_block_BaseAddress + 0x11e4)
#define CRYPTOCFG_3_25_RegisterSize 32
#define CRYPTOCFG_3_25_RegisterResetValue 0x0
#define CRYPTOCFG_3_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_25_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_25_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_26 (DWC_ufshc_block_BaseAddress + 0x11e8)
#define CRYPTOCFG_3_26_RegisterSize 32
#define CRYPTOCFG_3_26_RegisterResetValue 0x0
#define CRYPTOCFG_3_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_26_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_26_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_27 (DWC_ufshc_block_BaseAddress + 0x11ec)
#define CRYPTOCFG_3_27_RegisterSize 32
#define CRYPTOCFG_3_27_RegisterResetValue 0x0
#define CRYPTOCFG_3_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_27_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_27_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_28 (DWC_ufshc_block_BaseAddress + 0x11f0)
#define CRYPTOCFG_3_28_RegisterSize 32
#define CRYPTOCFG_3_28_RegisterResetValue 0x0
#define CRYPTOCFG_3_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_28_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_28_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_29 (DWC_ufshc_block_BaseAddress + 0x11f4)
#define CRYPTOCFG_3_29_RegisterSize 32
#define CRYPTOCFG_3_29_RegisterResetValue 0x0
#define CRYPTOCFG_3_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_29_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_29_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_30 (DWC_ufshc_block_BaseAddress + 0x11f8)
#define CRYPTOCFG_3_30_RegisterSize 32
#define CRYPTOCFG_3_30_RegisterResetValue 0x0
#define CRYPTOCFG_3_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_30_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_30_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_3_31 (DWC_ufshc_block_BaseAddress + 0x11fc)
#define CRYPTOCFG_3_31_RegisterSize 32
#define CRYPTOCFG_3_31_RegisterResetValue 0x0
#define CRYPTOCFG_3_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_3_31_RESERVED_3_BitAddressOffset 0
#define CRYPTOCFG_3_31_RESERVED_3_RegisterSize 32





#define CRYPTOCFG_4_0 (DWC_ufshc_block_BaseAddress + 0x1200)
#define CRYPTOCFG_4_0_RegisterSize 32
#define CRYPTOCFG_4_0_RegisterResetValue 0x0
#define CRYPTOCFG_4_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_0_CRYPTOKEY_4_0_BitAddressOffset 0
#define CRYPTOCFG_4_0_CRYPTOKEY_4_0_RegisterSize 32





#define CRYPTOCFG_4_1 (DWC_ufshc_block_BaseAddress + 0x1204)
#define CRYPTOCFG_4_1_RegisterSize 32
#define CRYPTOCFG_4_1_RegisterResetValue 0x0
#define CRYPTOCFG_4_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_1_CRYPTOKEY_4_1_BitAddressOffset 0
#define CRYPTOCFG_4_1_CRYPTOKEY_4_1_RegisterSize 32





#define CRYPTOCFG_4_2 (DWC_ufshc_block_BaseAddress + 0x1208)
#define CRYPTOCFG_4_2_RegisterSize 32
#define CRYPTOCFG_4_2_RegisterResetValue 0x0
#define CRYPTOCFG_4_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_2_CRYPTOKEY_4_2_BitAddressOffset 0
#define CRYPTOCFG_4_2_CRYPTOKEY_4_2_RegisterSize 32





#define CRYPTOCFG_4_3 (DWC_ufshc_block_BaseAddress + 0x120c)
#define CRYPTOCFG_4_3_RegisterSize 32
#define CRYPTOCFG_4_3_RegisterResetValue 0x0
#define CRYPTOCFG_4_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_3_CRYPTOKEY_4_3_BitAddressOffset 0
#define CRYPTOCFG_4_3_CRYPTOKEY_4_3_RegisterSize 32





#define CRYPTOCFG_4_4 (DWC_ufshc_block_BaseAddress + 0x1210)
#define CRYPTOCFG_4_4_RegisterSize 32
#define CRYPTOCFG_4_4_RegisterResetValue 0x0
#define CRYPTOCFG_4_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_4_CRYPTOKEY_4_4_BitAddressOffset 0
#define CRYPTOCFG_4_4_CRYPTOKEY_4_4_RegisterSize 32





#define CRYPTOCFG_4_5 (DWC_ufshc_block_BaseAddress + 0x1214)
#define CRYPTOCFG_4_5_RegisterSize 32
#define CRYPTOCFG_4_5_RegisterResetValue 0x0
#define CRYPTOCFG_4_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_5_CRYPTOKEY_4_5_BitAddressOffset 0
#define CRYPTOCFG_4_5_CRYPTOKEY_4_5_RegisterSize 32





#define CRYPTOCFG_4_6 (DWC_ufshc_block_BaseAddress + 0x1218)
#define CRYPTOCFG_4_6_RegisterSize 32
#define CRYPTOCFG_4_6_RegisterResetValue 0x0
#define CRYPTOCFG_4_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_6_CRYPTOKEY_4_6_BitAddressOffset 0
#define CRYPTOCFG_4_6_CRYPTOKEY_4_6_RegisterSize 32





#define CRYPTOCFG_4_7 (DWC_ufshc_block_BaseAddress + 0x121c)
#define CRYPTOCFG_4_7_RegisterSize 32
#define CRYPTOCFG_4_7_RegisterResetValue 0x0
#define CRYPTOCFG_4_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_7_CRYPTOKEY_4_7_BitAddressOffset 0
#define CRYPTOCFG_4_7_CRYPTOKEY_4_7_RegisterSize 32





#define CRYPTOCFG_4_8 (DWC_ufshc_block_BaseAddress + 0x1220)
#define CRYPTOCFG_4_8_RegisterSize 32
#define CRYPTOCFG_4_8_RegisterResetValue 0x0
#define CRYPTOCFG_4_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_8_CRYPTOKEY_4_8_BitAddressOffset 0
#define CRYPTOCFG_4_8_CRYPTOKEY_4_8_RegisterSize 32





#define CRYPTOCFG_4_9 (DWC_ufshc_block_BaseAddress + 0x1224)
#define CRYPTOCFG_4_9_RegisterSize 32
#define CRYPTOCFG_4_9_RegisterResetValue 0x0
#define CRYPTOCFG_4_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_9_CRYPTOKEY_4_9_BitAddressOffset 0
#define CRYPTOCFG_4_9_CRYPTOKEY_4_9_RegisterSize 32





#define CRYPTOCFG_4_10 (DWC_ufshc_block_BaseAddress + 0x1228)
#define CRYPTOCFG_4_10_RegisterSize 32
#define CRYPTOCFG_4_10_RegisterResetValue 0x0
#define CRYPTOCFG_4_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_10_CRYPTOKEY_4_10_BitAddressOffset 0
#define CRYPTOCFG_4_10_CRYPTOKEY_4_10_RegisterSize 32





#define CRYPTOCFG_4_11 (DWC_ufshc_block_BaseAddress + 0x122c)
#define CRYPTOCFG_4_11_RegisterSize 32
#define CRYPTOCFG_4_11_RegisterResetValue 0x0
#define CRYPTOCFG_4_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_11_CRYPTOKEY_4_11_BitAddressOffset 0
#define CRYPTOCFG_4_11_CRYPTOKEY_4_11_RegisterSize 32





#define CRYPTOCFG_4_12 (DWC_ufshc_block_BaseAddress + 0x1230)
#define CRYPTOCFG_4_12_RegisterSize 32
#define CRYPTOCFG_4_12_RegisterResetValue 0x0
#define CRYPTOCFG_4_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_12_CRYPTOKEY_4_12_BitAddressOffset 0
#define CRYPTOCFG_4_12_CRYPTOKEY_4_12_RegisterSize 32





#define CRYPTOCFG_4_13 (DWC_ufshc_block_BaseAddress + 0x1234)
#define CRYPTOCFG_4_13_RegisterSize 32
#define CRYPTOCFG_4_13_RegisterResetValue 0x0
#define CRYPTOCFG_4_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_13_CRYPTOKEY_4_13_BitAddressOffset 0
#define CRYPTOCFG_4_13_CRYPTOKEY_4_13_RegisterSize 32





#define CRYPTOCFG_4_14 (DWC_ufshc_block_BaseAddress + 0x1238)
#define CRYPTOCFG_4_14_RegisterSize 32
#define CRYPTOCFG_4_14_RegisterResetValue 0x0
#define CRYPTOCFG_4_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_14_CRYPTOKEY_4_14_BitAddressOffset 0
#define CRYPTOCFG_4_14_CRYPTOKEY_4_14_RegisterSize 32





#define CRYPTOCFG_4_15 (DWC_ufshc_block_BaseAddress + 0x123c)
#define CRYPTOCFG_4_15_RegisterSize 32
#define CRYPTOCFG_4_15_RegisterResetValue 0x0
#define CRYPTOCFG_4_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_15_CRYPTOKEY_4_15_BitAddressOffset 0
#define CRYPTOCFG_4_15_CRYPTOKEY_4_15_RegisterSize 32





#define CRYPTOCFG_4_16 (DWC_ufshc_block_BaseAddress + 0x1240)
#define CRYPTOCFG_4_16_RegisterSize 32
#define CRYPTOCFG_4_16_RegisterResetValue 0x0
#define CRYPTOCFG_4_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_16_DUSIZE_4_BitAddressOffset 0
#define CRYPTOCFG_4_16_DUSIZE_4_RegisterSize 8



#define CRYPTOCFG_4_16_CAPIDX_4_BitAddressOffset 8
#define CRYPTOCFG_4_16_CAPIDX_4_RegisterSize 8



#define CRYPTOCFG_4_16_CFGE_4_BitAddressOffset 31
#define CRYPTOCFG_4_16_CFGE_4_RegisterSize 1





#define CRYPTOCFG_4_17 (DWC_ufshc_block_BaseAddress + 0x1244)
#define CRYPTOCFG_4_17_RegisterSize 32
#define CRYPTOCFG_4_17_RegisterResetValue 0x0
#define CRYPTOCFG_4_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_17_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_17_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_18 (DWC_ufshc_block_BaseAddress + 0x1248)
#define CRYPTOCFG_4_18_RegisterSize 32
#define CRYPTOCFG_4_18_RegisterResetValue 0x0
#define CRYPTOCFG_4_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_18_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_18_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_19 (DWC_ufshc_block_BaseAddress + 0x124c)
#define CRYPTOCFG_4_19_RegisterSize 32
#define CRYPTOCFG_4_19_RegisterResetValue 0x0
#define CRYPTOCFG_4_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_19_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_19_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_20 (DWC_ufshc_block_BaseAddress + 0x1250)
#define CRYPTOCFG_4_20_RegisterSize 32
#define CRYPTOCFG_4_20_RegisterResetValue 0x0
#define CRYPTOCFG_4_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_20_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_20_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_21 (DWC_ufshc_block_BaseAddress + 0x1254)
#define CRYPTOCFG_4_21_RegisterSize 32
#define CRYPTOCFG_4_21_RegisterResetValue 0x0
#define CRYPTOCFG_4_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_21_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_21_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_22 (DWC_ufshc_block_BaseAddress + 0x1258)
#define CRYPTOCFG_4_22_RegisterSize 32
#define CRYPTOCFG_4_22_RegisterResetValue 0x0
#define CRYPTOCFG_4_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_22_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_22_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_23 (DWC_ufshc_block_BaseAddress + 0x125c)
#define CRYPTOCFG_4_23_RegisterSize 32
#define CRYPTOCFG_4_23_RegisterResetValue 0x0
#define CRYPTOCFG_4_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_23_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_23_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_24 (DWC_ufshc_block_BaseAddress + 0x1260)
#define CRYPTOCFG_4_24_RegisterSize 32
#define CRYPTOCFG_4_24_RegisterResetValue 0x0
#define CRYPTOCFG_4_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_24_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_24_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_25 (DWC_ufshc_block_BaseAddress + 0x1264)
#define CRYPTOCFG_4_25_RegisterSize 32
#define CRYPTOCFG_4_25_RegisterResetValue 0x0
#define CRYPTOCFG_4_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_25_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_25_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_26 (DWC_ufshc_block_BaseAddress + 0x1268)
#define CRYPTOCFG_4_26_RegisterSize 32
#define CRYPTOCFG_4_26_RegisterResetValue 0x0
#define CRYPTOCFG_4_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_26_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_26_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_27 (DWC_ufshc_block_BaseAddress + 0x126c)
#define CRYPTOCFG_4_27_RegisterSize 32
#define CRYPTOCFG_4_27_RegisterResetValue 0x0
#define CRYPTOCFG_4_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_27_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_27_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_28 (DWC_ufshc_block_BaseAddress + 0x1270)
#define CRYPTOCFG_4_28_RegisterSize 32
#define CRYPTOCFG_4_28_RegisterResetValue 0x0
#define CRYPTOCFG_4_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_28_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_28_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_29 (DWC_ufshc_block_BaseAddress + 0x1274)
#define CRYPTOCFG_4_29_RegisterSize 32
#define CRYPTOCFG_4_29_RegisterResetValue 0x0
#define CRYPTOCFG_4_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_29_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_29_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_30 (DWC_ufshc_block_BaseAddress + 0x1278)
#define CRYPTOCFG_4_30_RegisterSize 32
#define CRYPTOCFG_4_30_RegisterResetValue 0x0
#define CRYPTOCFG_4_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_30_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_30_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_4_31 (DWC_ufshc_block_BaseAddress + 0x127c)
#define CRYPTOCFG_4_31_RegisterSize 32
#define CRYPTOCFG_4_31_RegisterResetValue 0x0
#define CRYPTOCFG_4_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_4_31_RESERVED_4_BitAddressOffset 0
#define CRYPTOCFG_4_31_RESERVED_4_RegisterSize 32





#define CRYPTOCFG_5_0 (DWC_ufshc_block_BaseAddress + 0x1280)
#define CRYPTOCFG_5_0_RegisterSize 32
#define CRYPTOCFG_5_0_RegisterResetValue 0x0
#define CRYPTOCFG_5_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_0_CRYPTOKEY_5_0_BitAddressOffset 0
#define CRYPTOCFG_5_0_CRYPTOKEY_5_0_RegisterSize 32





#define CRYPTOCFG_5_1 (DWC_ufshc_block_BaseAddress + 0x1284)
#define CRYPTOCFG_5_1_RegisterSize 32
#define CRYPTOCFG_5_1_RegisterResetValue 0x0
#define CRYPTOCFG_5_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_1_CRYPTOKEY_5_1_BitAddressOffset 0
#define CRYPTOCFG_5_1_CRYPTOKEY_5_1_RegisterSize 32





#define CRYPTOCFG_5_2 (DWC_ufshc_block_BaseAddress + 0x1288)
#define CRYPTOCFG_5_2_RegisterSize 32
#define CRYPTOCFG_5_2_RegisterResetValue 0x0
#define CRYPTOCFG_5_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_2_CRYPTOKEY_5_2_BitAddressOffset 0
#define CRYPTOCFG_5_2_CRYPTOKEY_5_2_RegisterSize 32





#define CRYPTOCFG_5_3 (DWC_ufshc_block_BaseAddress + 0x128c)
#define CRYPTOCFG_5_3_RegisterSize 32
#define CRYPTOCFG_5_3_RegisterResetValue 0x0
#define CRYPTOCFG_5_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_3_CRYPTOKEY_5_3_BitAddressOffset 0
#define CRYPTOCFG_5_3_CRYPTOKEY_5_3_RegisterSize 32





#define CRYPTOCFG_5_4 (DWC_ufshc_block_BaseAddress + 0x1290)
#define CRYPTOCFG_5_4_RegisterSize 32
#define CRYPTOCFG_5_4_RegisterResetValue 0x0
#define CRYPTOCFG_5_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_4_CRYPTOKEY_5_4_BitAddressOffset 0
#define CRYPTOCFG_5_4_CRYPTOKEY_5_4_RegisterSize 32





#define CRYPTOCFG_5_5 (DWC_ufshc_block_BaseAddress + 0x1294)
#define CRYPTOCFG_5_5_RegisterSize 32
#define CRYPTOCFG_5_5_RegisterResetValue 0x0
#define CRYPTOCFG_5_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_5_CRYPTOKEY_5_5_BitAddressOffset 0
#define CRYPTOCFG_5_5_CRYPTOKEY_5_5_RegisterSize 32





#define CRYPTOCFG_5_6 (DWC_ufshc_block_BaseAddress + 0x1298)
#define CRYPTOCFG_5_6_RegisterSize 32
#define CRYPTOCFG_5_6_RegisterResetValue 0x0
#define CRYPTOCFG_5_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_6_CRYPTOKEY_5_6_BitAddressOffset 0
#define CRYPTOCFG_5_6_CRYPTOKEY_5_6_RegisterSize 32





#define CRYPTOCFG_5_7 (DWC_ufshc_block_BaseAddress + 0x129c)
#define CRYPTOCFG_5_7_RegisterSize 32
#define CRYPTOCFG_5_7_RegisterResetValue 0x0
#define CRYPTOCFG_5_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_7_CRYPTOKEY_5_7_BitAddressOffset 0
#define CRYPTOCFG_5_7_CRYPTOKEY_5_7_RegisterSize 32





#define CRYPTOCFG_5_8 (DWC_ufshc_block_BaseAddress + 0x12a0)
#define CRYPTOCFG_5_8_RegisterSize 32
#define CRYPTOCFG_5_8_RegisterResetValue 0x0
#define CRYPTOCFG_5_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_8_CRYPTOKEY_5_8_BitAddressOffset 0
#define CRYPTOCFG_5_8_CRYPTOKEY_5_8_RegisterSize 32





#define CRYPTOCFG_5_9 (DWC_ufshc_block_BaseAddress + 0x12a4)
#define CRYPTOCFG_5_9_RegisterSize 32
#define CRYPTOCFG_5_9_RegisterResetValue 0x0
#define CRYPTOCFG_5_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_9_CRYPTOKEY_5_9_BitAddressOffset 0
#define CRYPTOCFG_5_9_CRYPTOKEY_5_9_RegisterSize 32





#define CRYPTOCFG_5_10 (DWC_ufshc_block_BaseAddress + 0x12a8)
#define CRYPTOCFG_5_10_RegisterSize 32
#define CRYPTOCFG_5_10_RegisterResetValue 0x0
#define CRYPTOCFG_5_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_10_CRYPTOKEY_5_10_BitAddressOffset 0
#define CRYPTOCFG_5_10_CRYPTOKEY_5_10_RegisterSize 32





#define CRYPTOCFG_5_11 (DWC_ufshc_block_BaseAddress + 0x12ac)
#define CRYPTOCFG_5_11_RegisterSize 32
#define CRYPTOCFG_5_11_RegisterResetValue 0x0
#define CRYPTOCFG_5_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_11_CRYPTOKEY_5_11_BitAddressOffset 0
#define CRYPTOCFG_5_11_CRYPTOKEY_5_11_RegisterSize 32





#define CRYPTOCFG_5_12 (DWC_ufshc_block_BaseAddress + 0x12b0)
#define CRYPTOCFG_5_12_RegisterSize 32
#define CRYPTOCFG_5_12_RegisterResetValue 0x0
#define CRYPTOCFG_5_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_12_CRYPTOKEY_5_12_BitAddressOffset 0
#define CRYPTOCFG_5_12_CRYPTOKEY_5_12_RegisterSize 32





#define CRYPTOCFG_5_13 (DWC_ufshc_block_BaseAddress + 0x12b4)
#define CRYPTOCFG_5_13_RegisterSize 32
#define CRYPTOCFG_5_13_RegisterResetValue 0x0
#define CRYPTOCFG_5_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_13_CRYPTOKEY_5_13_BitAddressOffset 0
#define CRYPTOCFG_5_13_CRYPTOKEY_5_13_RegisterSize 32





#define CRYPTOCFG_5_14 (DWC_ufshc_block_BaseAddress + 0x12b8)
#define CRYPTOCFG_5_14_RegisterSize 32
#define CRYPTOCFG_5_14_RegisterResetValue 0x0
#define CRYPTOCFG_5_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_14_CRYPTOKEY_5_14_BitAddressOffset 0
#define CRYPTOCFG_5_14_CRYPTOKEY_5_14_RegisterSize 32





#define CRYPTOCFG_5_15 (DWC_ufshc_block_BaseAddress + 0x12bc)
#define CRYPTOCFG_5_15_RegisterSize 32
#define CRYPTOCFG_5_15_RegisterResetValue 0x0
#define CRYPTOCFG_5_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_15_CRYPTOKEY_5_15_BitAddressOffset 0
#define CRYPTOCFG_5_15_CRYPTOKEY_5_15_RegisterSize 32





#define CRYPTOCFG_5_16 (DWC_ufshc_block_BaseAddress + 0x12c0)
#define CRYPTOCFG_5_16_RegisterSize 32
#define CRYPTOCFG_5_16_RegisterResetValue 0x0
#define CRYPTOCFG_5_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_16_DUSIZE_5_BitAddressOffset 0
#define CRYPTOCFG_5_16_DUSIZE_5_RegisterSize 8



#define CRYPTOCFG_5_16_CAPIDX_5_BitAddressOffset 8
#define CRYPTOCFG_5_16_CAPIDX_5_RegisterSize 8



#define CRYPTOCFG_5_16_CFGE_5_BitAddressOffset 31
#define CRYPTOCFG_5_16_CFGE_5_RegisterSize 1





#define CRYPTOCFG_5_17 (DWC_ufshc_block_BaseAddress + 0x12c4)
#define CRYPTOCFG_5_17_RegisterSize 32
#define CRYPTOCFG_5_17_RegisterResetValue 0x0
#define CRYPTOCFG_5_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_17_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_17_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_18 (DWC_ufshc_block_BaseAddress + 0x12c8)
#define CRYPTOCFG_5_18_RegisterSize 32
#define CRYPTOCFG_5_18_RegisterResetValue 0x0
#define CRYPTOCFG_5_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_18_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_18_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_19 (DWC_ufshc_block_BaseAddress + 0x12cc)
#define CRYPTOCFG_5_19_RegisterSize 32
#define CRYPTOCFG_5_19_RegisterResetValue 0x0
#define CRYPTOCFG_5_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_19_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_19_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_20 (DWC_ufshc_block_BaseAddress + 0x12d0)
#define CRYPTOCFG_5_20_RegisterSize 32
#define CRYPTOCFG_5_20_RegisterResetValue 0x0
#define CRYPTOCFG_5_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_20_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_20_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_21 (DWC_ufshc_block_BaseAddress + 0x12d4)
#define CRYPTOCFG_5_21_RegisterSize 32
#define CRYPTOCFG_5_21_RegisterResetValue 0x0
#define CRYPTOCFG_5_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_21_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_21_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_22 (DWC_ufshc_block_BaseAddress + 0x12d8)
#define CRYPTOCFG_5_22_RegisterSize 32
#define CRYPTOCFG_5_22_RegisterResetValue 0x0
#define CRYPTOCFG_5_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_22_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_22_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_23 (DWC_ufshc_block_BaseAddress + 0x12dc)
#define CRYPTOCFG_5_23_RegisterSize 32
#define CRYPTOCFG_5_23_RegisterResetValue 0x0
#define CRYPTOCFG_5_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_23_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_23_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_24 (DWC_ufshc_block_BaseAddress + 0x12e0)
#define CRYPTOCFG_5_24_RegisterSize 32
#define CRYPTOCFG_5_24_RegisterResetValue 0x0
#define CRYPTOCFG_5_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_24_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_24_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_25 (DWC_ufshc_block_BaseAddress + 0x12e4)
#define CRYPTOCFG_5_25_RegisterSize 32
#define CRYPTOCFG_5_25_RegisterResetValue 0x0
#define CRYPTOCFG_5_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_25_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_25_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_26 (DWC_ufshc_block_BaseAddress + 0x12e8)
#define CRYPTOCFG_5_26_RegisterSize 32
#define CRYPTOCFG_5_26_RegisterResetValue 0x0
#define CRYPTOCFG_5_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_26_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_26_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_27 (DWC_ufshc_block_BaseAddress + 0x12ec)
#define CRYPTOCFG_5_27_RegisterSize 32
#define CRYPTOCFG_5_27_RegisterResetValue 0x0
#define CRYPTOCFG_5_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_27_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_27_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_28 (DWC_ufshc_block_BaseAddress + 0x12f0)
#define CRYPTOCFG_5_28_RegisterSize 32
#define CRYPTOCFG_5_28_RegisterResetValue 0x0
#define CRYPTOCFG_5_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_28_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_28_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_29 (DWC_ufshc_block_BaseAddress + 0x12f4)
#define CRYPTOCFG_5_29_RegisterSize 32
#define CRYPTOCFG_5_29_RegisterResetValue 0x0
#define CRYPTOCFG_5_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_29_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_29_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_30 (DWC_ufshc_block_BaseAddress + 0x12f8)
#define CRYPTOCFG_5_30_RegisterSize 32
#define CRYPTOCFG_5_30_RegisterResetValue 0x0
#define CRYPTOCFG_5_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_30_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_30_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_5_31 (DWC_ufshc_block_BaseAddress + 0x12fc)
#define CRYPTOCFG_5_31_RegisterSize 32
#define CRYPTOCFG_5_31_RegisterResetValue 0x0
#define CRYPTOCFG_5_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_5_31_RESERVED_5_BitAddressOffset 0
#define CRYPTOCFG_5_31_RESERVED_5_RegisterSize 32





#define CRYPTOCFG_6_0 (DWC_ufshc_block_BaseAddress + 0x1300)
#define CRYPTOCFG_6_0_RegisterSize 32
#define CRYPTOCFG_6_0_RegisterResetValue 0x0
#define CRYPTOCFG_6_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_0_CRYPTOKEY_6_0_BitAddressOffset 0
#define CRYPTOCFG_6_0_CRYPTOKEY_6_0_RegisterSize 32





#define CRYPTOCFG_6_1 (DWC_ufshc_block_BaseAddress + 0x1304)
#define CRYPTOCFG_6_1_RegisterSize 32
#define CRYPTOCFG_6_1_RegisterResetValue 0x0
#define CRYPTOCFG_6_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_1_CRYPTOKEY_6_1_BitAddressOffset 0
#define CRYPTOCFG_6_1_CRYPTOKEY_6_1_RegisterSize 32





#define CRYPTOCFG_6_2 (DWC_ufshc_block_BaseAddress + 0x1308)
#define CRYPTOCFG_6_2_RegisterSize 32
#define CRYPTOCFG_6_2_RegisterResetValue 0x0
#define CRYPTOCFG_6_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_2_CRYPTOKEY_6_2_BitAddressOffset 0
#define CRYPTOCFG_6_2_CRYPTOKEY_6_2_RegisterSize 32





#define CRYPTOCFG_6_3 (DWC_ufshc_block_BaseAddress + 0x130c)
#define CRYPTOCFG_6_3_RegisterSize 32
#define CRYPTOCFG_6_3_RegisterResetValue 0x0
#define CRYPTOCFG_6_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_3_CRYPTOKEY_6_3_BitAddressOffset 0
#define CRYPTOCFG_6_3_CRYPTOKEY_6_3_RegisterSize 32





#define CRYPTOCFG_6_4 (DWC_ufshc_block_BaseAddress + 0x1310)
#define CRYPTOCFG_6_4_RegisterSize 32
#define CRYPTOCFG_6_4_RegisterResetValue 0x0
#define CRYPTOCFG_6_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_4_CRYPTOKEY_6_4_BitAddressOffset 0
#define CRYPTOCFG_6_4_CRYPTOKEY_6_4_RegisterSize 32





#define CRYPTOCFG_6_5 (DWC_ufshc_block_BaseAddress + 0x1314)
#define CRYPTOCFG_6_5_RegisterSize 32
#define CRYPTOCFG_6_5_RegisterResetValue 0x0
#define CRYPTOCFG_6_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_5_CRYPTOKEY_6_5_BitAddressOffset 0
#define CRYPTOCFG_6_5_CRYPTOKEY_6_5_RegisterSize 32





#define CRYPTOCFG_6_6 (DWC_ufshc_block_BaseAddress + 0x1318)
#define CRYPTOCFG_6_6_RegisterSize 32
#define CRYPTOCFG_6_6_RegisterResetValue 0x0
#define CRYPTOCFG_6_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_6_CRYPTOKEY_6_6_BitAddressOffset 0
#define CRYPTOCFG_6_6_CRYPTOKEY_6_6_RegisterSize 32





#define CRYPTOCFG_6_7 (DWC_ufshc_block_BaseAddress + 0x131c)
#define CRYPTOCFG_6_7_RegisterSize 32
#define CRYPTOCFG_6_7_RegisterResetValue 0x0
#define CRYPTOCFG_6_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_7_CRYPTOKEY_6_7_BitAddressOffset 0
#define CRYPTOCFG_6_7_CRYPTOKEY_6_7_RegisterSize 32





#define CRYPTOCFG_6_8 (DWC_ufshc_block_BaseAddress + 0x1320)
#define CRYPTOCFG_6_8_RegisterSize 32
#define CRYPTOCFG_6_8_RegisterResetValue 0x0
#define CRYPTOCFG_6_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_8_CRYPTOKEY_6_8_BitAddressOffset 0
#define CRYPTOCFG_6_8_CRYPTOKEY_6_8_RegisterSize 32





#define CRYPTOCFG_6_9 (DWC_ufshc_block_BaseAddress + 0x1324)
#define CRYPTOCFG_6_9_RegisterSize 32
#define CRYPTOCFG_6_9_RegisterResetValue 0x0
#define CRYPTOCFG_6_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_9_CRYPTOKEY_6_9_BitAddressOffset 0
#define CRYPTOCFG_6_9_CRYPTOKEY_6_9_RegisterSize 32





#define CRYPTOCFG_6_10 (DWC_ufshc_block_BaseAddress + 0x1328)
#define CRYPTOCFG_6_10_RegisterSize 32
#define CRYPTOCFG_6_10_RegisterResetValue 0x0
#define CRYPTOCFG_6_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_10_CRYPTOKEY_6_10_BitAddressOffset 0
#define CRYPTOCFG_6_10_CRYPTOKEY_6_10_RegisterSize 32





#define CRYPTOCFG_6_11 (DWC_ufshc_block_BaseAddress + 0x132c)
#define CRYPTOCFG_6_11_RegisterSize 32
#define CRYPTOCFG_6_11_RegisterResetValue 0x0
#define CRYPTOCFG_6_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_11_CRYPTOKEY_6_11_BitAddressOffset 0
#define CRYPTOCFG_6_11_CRYPTOKEY_6_11_RegisterSize 32





#define CRYPTOCFG_6_12 (DWC_ufshc_block_BaseAddress + 0x1330)
#define CRYPTOCFG_6_12_RegisterSize 32
#define CRYPTOCFG_6_12_RegisterResetValue 0x0
#define CRYPTOCFG_6_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_12_CRYPTOKEY_6_12_BitAddressOffset 0
#define CRYPTOCFG_6_12_CRYPTOKEY_6_12_RegisterSize 32





#define CRYPTOCFG_6_13 (DWC_ufshc_block_BaseAddress + 0x1334)
#define CRYPTOCFG_6_13_RegisterSize 32
#define CRYPTOCFG_6_13_RegisterResetValue 0x0
#define CRYPTOCFG_6_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_13_CRYPTOKEY_6_13_BitAddressOffset 0
#define CRYPTOCFG_6_13_CRYPTOKEY_6_13_RegisterSize 32





#define CRYPTOCFG_6_14 (DWC_ufshc_block_BaseAddress + 0x1338)
#define CRYPTOCFG_6_14_RegisterSize 32
#define CRYPTOCFG_6_14_RegisterResetValue 0x0
#define CRYPTOCFG_6_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_14_CRYPTOKEY_6_14_BitAddressOffset 0
#define CRYPTOCFG_6_14_CRYPTOKEY_6_14_RegisterSize 32





#define CRYPTOCFG_6_15 (DWC_ufshc_block_BaseAddress + 0x133c)
#define CRYPTOCFG_6_15_RegisterSize 32
#define CRYPTOCFG_6_15_RegisterResetValue 0x0
#define CRYPTOCFG_6_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_15_CRYPTOKEY_6_15_BitAddressOffset 0
#define CRYPTOCFG_6_15_CRYPTOKEY_6_15_RegisterSize 32





#define CRYPTOCFG_6_16 (DWC_ufshc_block_BaseAddress + 0x1340)
#define CRYPTOCFG_6_16_RegisterSize 32
#define CRYPTOCFG_6_16_RegisterResetValue 0x0
#define CRYPTOCFG_6_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_16_DUSIZE_6_BitAddressOffset 0
#define CRYPTOCFG_6_16_DUSIZE_6_RegisterSize 8



#define CRYPTOCFG_6_16_CAPIDX_6_BitAddressOffset 8
#define CRYPTOCFG_6_16_CAPIDX_6_RegisterSize 8



#define CRYPTOCFG_6_16_CFGE_6_BitAddressOffset 31
#define CRYPTOCFG_6_16_CFGE_6_RegisterSize 1





#define CRYPTOCFG_6_17 (DWC_ufshc_block_BaseAddress + 0x1344)
#define CRYPTOCFG_6_17_RegisterSize 32
#define CRYPTOCFG_6_17_RegisterResetValue 0x0
#define CRYPTOCFG_6_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_17_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_17_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_18 (DWC_ufshc_block_BaseAddress + 0x1348)
#define CRYPTOCFG_6_18_RegisterSize 32
#define CRYPTOCFG_6_18_RegisterResetValue 0x0
#define CRYPTOCFG_6_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_18_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_18_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_19 (DWC_ufshc_block_BaseAddress + 0x134c)
#define CRYPTOCFG_6_19_RegisterSize 32
#define CRYPTOCFG_6_19_RegisterResetValue 0x0
#define CRYPTOCFG_6_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_19_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_19_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_20 (DWC_ufshc_block_BaseAddress + 0x1350)
#define CRYPTOCFG_6_20_RegisterSize 32
#define CRYPTOCFG_6_20_RegisterResetValue 0x0
#define CRYPTOCFG_6_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_20_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_20_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_21 (DWC_ufshc_block_BaseAddress + 0x1354)
#define CRYPTOCFG_6_21_RegisterSize 32
#define CRYPTOCFG_6_21_RegisterResetValue 0x0
#define CRYPTOCFG_6_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_21_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_21_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_22 (DWC_ufshc_block_BaseAddress + 0x1358)
#define CRYPTOCFG_6_22_RegisterSize 32
#define CRYPTOCFG_6_22_RegisterResetValue 0x0
#define CRYPTOCFG_6_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_22_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_22_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_23 (DWC_ufshc_block_BaseAddress + 0x135c)
#define CRYPTOCFG_6_23_RegisterSize 32
#define CRYPTOCFG_6_23_RegisterResetValue 0x0
#define CRYPTOCFG_6_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_23_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_23_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_24 (DWC_ufshc_block_BaseAddress + 0x1360)
#define CRYPTOCFG_6_24_RegisterSize 32
#define CRYPTOCFG_6_24_RegisterResetValue 0x0
#define CRYPTOCFG_6_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_24_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_24_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_25 (DWC_ufshc_block_BaseAddress + 0x1364)
#define CRYPTOCFG_6_25_RegisterSize 32
#define CRYPTOCFG_6_25_RegisterResetValue 0x0
#define CRYPTOCFG_6_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_25_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_25_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_26 (DWC_ufshc_block_BaseAddress + 0x1368)
#define CRYPTOCFG_6_26_RegisterSize 32
#define CRYPTOCFG_6_26_RegisterResetValue 0x0
#define CRYPTOCFG_6_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_26_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_26_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_27 (DWC_ufshc_block_BaseAddress + 0x136c)
#define CRYPTOCFG_6_27_RegisterSize 32
#define CRYPTOCFG_6_27_RegisterResetValue 0x0
#define CRYPTOCFG_6_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_27_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_27_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_28 (DWC_ufshc_block_BaseAddress + 0x1370)
#define CRYPTOCFG_6_28_RegisterSize 32
#define CRYPTOCFG_6_28_RegisterResetValue 0x0
#define CRYPTOCFG_6_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_28_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_28_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_29 (DWC_ufshc_block_BaseAddress + 0x1374)
#define CRYPTOCFG_6_29_RegisterSize 32
#define CRYPTOCFG_6_29_RegisterResetValue 0x0
#define CRYPTOCFG_6_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_29_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_29_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_30 (DWC_ufshc_block_BaseAddress + 0x1378)
#define CRYPTOCFG_6_30_RegisterSize 32
#define CRYPTOCFG_6_30_RegisterResetValue 0x0
#define CRYPTOCFG_6_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_30_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_30_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_6_31 (DWC_ufshc_block_BaseAddress + 0x137c)
#define CRYPTOCFG_6_31_RegisterSize 32
#define CRYPTOCFG_6_31_RegisterResetValue 0x0
#define CRYPTOCFG_6_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_6_31_RESERVED_6_BitAddressOffset 0
#define CRYPTOCFG_6_31_RESERVED_6_RegisterSize 32





#define CRYPTOCFG_7_0 (DWC_ufshc_block_BaseAddress + 0x1380)
#define CRYPTOCFG_7_0_RegisterSize 32
#define CRYPTOCFG_7_0_RegisterResetValue 0x0
#define CRYPTOCFG_7_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_0_CRYPTOKEY_7_0_BitAddressOffset 0
#define CRYPTOCFG_7_0_CRYPTOKEY_7_0_RegisterSize 32





#define CRYPTOCFG_7_1 (DWC_ufshc_block_BaseAddress + 0x1384)
#define CRYPTOCFG_7_1_RegisterSize 32
#define CRYPTOCFG_7_1_RegisterResetValue 0x0
#define CRYPTOCFG_7_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_1_CRYPTOKEY_7_1_BitAddressOffset 0
#define CRYPTOCFG_7_1_CRYPTOKEY_7_1_RegisterSize 32





#define CRYPTOCFG_7_2 (DWC_ufshc_block_BaseAddress + 0x1388)
#define CRYPTOCFG_7_2_RegisterSize 32
#define CRYPTOCFG_7_2_RegisterResetValue 0x0
#define CRYPTOCFG_7_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_2_CRYPTOKEY_7_2_BitAddressOffset 0
#define CRYPTOCFG_7_2_CRYPTOKEY_7_2_RegisterSize 32





#define CRYPTOCFG_7_3 (DWC_ufshc_block_BaseAddress + 0x138c)
#define CRYPTOCFG_7_3_RegisterSize 32
#define CRYPTOCFG_7_3_RegisterResetValue 0x0
#define CRYPTOCFG_7_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_3_CRYPTOKEY_7_3_BitAddressOffset 0
#define CRYPTOCFG_7_3_CRYPTOKEY_7_3_RegisterSize 32





#define CRYPTOCFG_7_4 (DWC_ufshc_block_BaseAddress + 0x1390)
#define CRYPTOCFG_7_4_RegisterSize 32
#define CRYPTOCFG_7_4_RegisterResetValue 0x0
#define CRYPTOCFG_7_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_4_CRYPTOKEY_7_4_BitAddressOffset 0
#define CRYPTOCFG_7_4_CRYPTOKEY_7_4_RegisterSize 32





#define CRYPTOCFG_7_5 (DWC_ufshc_block_BaseAddress + 0x1394)
#define CRYPTOCFG_7_5_RegisterSize 32
#define CRYPTOCFG_7_5_RegisterResetValue 0x0
#define CRYPTOCFG_7_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_5_CRYPTOKEY_7_5_BitAddressOffset 0
#define CRYPTOCFG_7_5_CRYPTOKEY_7_5_RegisterSize 32





#define CRYPTOCFG_7_6 (DWC_ufshc_block_BaseAddress + 0x1398)
#define CRYPTOCFG_7_6_RegisterSize 32
#define CRYPTOCFG_7_6_RegisterResetValue 0x0
#define CRYPTOCFG_7_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_6_CRYPTOKEY_7_6_BitAddressOffset 0
#define CRYPTOCFG_7_6_CRYPTOKEY_7_6_RegisterSize 32





#define CRYPTOCFG_7_7 (DWC_ufshc_block_BaseAddress + 0x139c)
#define CRYPTOCFG_7_7_RegisterSize 32
#define CRYPTOCFG_7_7_RegisterResetValue 0x0
#define CRYPTOCFG_7_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_7_CRYPTOKEY_7_7_BitAddressOffset 0
#define CRYPTOCFG_7_7_CRYPTOKEY_7_7_RegisterSize 32





#define CRYPTOCFG_7_8 (DWC_ufshc_block_BaseAddress + 0x13a0)
#define CRYPTOCFG_7_8_RegisterSize 32
#define CRYPTOCFG_7_8_RegisterResetValue 0x0
#define CRYPTOCFG_7_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_8_CRYPTOKEY_7_8_BitAddressOffset 0
#define CRYPTOCFG_7_8_CRYPTOKEY_7_8_RegisterSize 32





#define CRYPTOCFG_7_9 (DWC_ufshc_block_BaseAddress + 0x13a4)
#define CRYPTOCFG_7_9_RegisterSize 32
#define CRYPTOCFG_7_9_RegisterResetValue 0x0
#define CRYPTOCFG_7_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_9_CRYPTOKEY_7_9_BitAddressOffset 0
#define CRYPTOCFG_7_9_CRYPTOKEY_7_9_RegisterSize 32





#define CRYPTOCFG_7_10 (DWC_ufshc_block_BaseAddress + 0x13a8)
#define CRYPTOCFG_7_10_RegisterSize 32
#define CRYPTOCFG_7_10_RegisterResetValue 0x0
#define CRYPTOCFG_7_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_10_CRYPTOKEY_7_10_BitAddressOffset 0
#define CRYPTOCFG_7_10_CRYPTOKEY_7_10_RegisterSize 32





#define CRYPTOCFG_7_11 (DWC_ufshc_block_BaseAddress + 0x13ac)
#define CRYPTOCFG_7_11_RegisterSize 32
#define CRYPTOCFG_7_11_RegisterResetValue 0x0
#define CRYPTOCFG_7_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_11_CRYPTOKEY_7_11_BitAddressOffset 0
#define CRYPTOCFG_7_11_CRYPTOKEY_7_11_RegisterSize 32





#define CRYPTOCFG_7_12 (DWC_ufshc_block_BaseAddress + 0x13b0)
#define CRYPTOCFG_7_12_RegisterSize 32
#define CRYPTOCFG_7_12_RegisterResetValue 0x0
#define CRYPTOCFG_7_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_12_CRYPTOKEY_7_12_BitAddressOffset 0
#define CRYPTOCFG_7_12_CRYPTOKEY_7_12_RegisterSize 32





#define CRYPTOCFG_7_13 (DWC_ufshc_block_BaseAddress + 0x13b4)
#define CRYPTOCFG_7_13_RegisterSize 32
#define CRYPTOCFG_7_13_RegisterResetValue 0x0
#define CRYPTOCFG_7_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_13_CRYPTOKEY_7_13_BitAddressOffset 0
#define CRYPTOCFG_7_13_CRYPTOKEY_7_13_RegisterSize 32





#define CRYPTOCFG_7_14 (DWC_ufshc_block_BaseAddress + 0x13b8)
#define CRYPTOCFG_7_14_RegisterSize 32
#define CRYPTOCFG_7_14_RegisterResetValue 0x0
#define CRYPTOCFG_7_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_14_CRYPTOKEY_7_14_BitAddressOffset 0
#define CRYPTOCFG_7_14_CRYPTOKEY_7_14_RegisterSize 32





#define CRYPTOCFG_7_15 (DWC_ufshc_block_BaseAddress + 0x13bc)
#define CRYPTOCFG_7_15_RegisterSize 32
#define CRYPTOCFG_7_15_RegisterResetValue 0x0
#define CRYPTOCFG_7_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_15_CRYPTOKEY_7_15_BitAddressOffset 0
#define CRYPTOCFG_7_15_CRYPTOKEY_7_15_RegisterSize 32





#define CRYPTOCFG_7_16 (DWC_ufshc_block_BaseAddress + 0x13c0)
#define CRYPTOCFG_7_16_RegisterSize 32
#define CRYPTOCFG_7_16_RegisterResetValue 0x0
#define CRYPTOCFG_7_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_16_DUSIZE_7_BitAddressOffset 0
#define CRYPTOCFG_7_16_DUSIZE_7_RegisterSize 8



#define CRYPTOCFG_7_16_CAPIDX_7_BitAddressOffset 8
#define CRYPTOCFG_7_16_CAPIDX_7_RegisterSize 8



#define CRYPTOCFG_7_16_CFGE_7_BitAddressOffset 31
#define CRYPTOCFG_7_16_CFGE_7_RegisterSize 1





#define CRYPTOCFG_7_17 (DWC_ufshc_block_BaseAddress + 0x13c4)
#define CRYPTOCFG_7_17_RegisterSize 32
#define CRYPTOCFG_7_17_RegisterResetValue 0x0
#define CRYPTOCFG_7_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_17_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_17_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_18 (DWC_ufshc_block_BaseAddress + 0x13c8)
#define CRYPTOCFG_7_18_RegisterSize 32
#define CRYPTOCFG_7_18_RegisterResetValue 0x0
#define CRYPTOCFG_7_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_18_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_18_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_19 (DWC_ufshc_block_BaseAddress + 0x13cc)
#define CRYPTOCFG_7_19_RegisterSize 32
#define CRYPTOCFG_7_19_RegisterResetValue 0x0
#define CRYPTOCFG_7_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_19_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_19_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_20 (DWC_ufshc_block_BaseAddress + 0x13d0)
#define CRYPTOCFG_7_20_RegisterSize 32
#define CRYPTOCFG_7_20_RegisterResetValue 0x0
#define CRYPTOCFG_7_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_20_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_20_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_21 (DWC_ufshc_block_BaseAddress + 0x13d4)
#define CRYPTOCFG_7_21_RegisterSize 32
#define CRYPTOCFG_7_21_RegisterResetValue 0x0
#define CRYPTOCFG_7_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_21_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_21_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_22 (DWC_ufshc_block_BaseAddress + 0x13d8)
#define CRYPTOCFG_7_22_RegisterSize 32
#define CRYPTOCFG_7_22_RegisterResetValue 0x0
#define CRYPTOCFG_7_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_22_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_22_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_23 (DWC_ufshc_block_BaseAddress + 0x13dc)
#define CRYPTOCFG_7_23_RegisterSize 32
#define CRYPTOCFG_7_23_RegisterResetValue 0x0
#define CRYPTOCFG_7_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_23_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_23_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_24 (DWC_ufshc_block_BaseAddress + 0x13e0)
#define CRYPTOCFG_7_24_RegisterSize 32
#define CRYPTOCFG_7_24_RegisterResetValue 0x0
#define CRYPTOCFG_7_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_24_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_24_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_25 (DWC_ufshc_block_BaseAddress + 0x13e4)
#define CRYPTOCFG_7_25_RegisterSize 32
#define CRYPTOCFG_7_25_RegisterResetValue 0x0
#define CRYPTOCFG_7_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_25_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_25_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_26 (DWC_ufshc_block_BaseAddress + 0x13e8)
#define CRYPTOCFG_7_26_RegisterSize 32
#define CRYPTOCFG_7_26_RegisterResetValue 0x0
#define CRYPTOCFG_7_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_26_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_26_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_27 (DWC_ufshc_block_BaseAddress + 0x13ec)
#define CRYPTOCFG_7_27_RegisterSize 32
#define CRYPTOCFG_7_27_RegisterResetValue 0x0
#define CRYPTOCFG_7_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_27_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_27_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_28 (DWC_ufshc_block_BaseAddress + 0x13f0)
#define CRYPTOCFG_7_28_RegisterSize 32
#define CRYPTOCFG_7_28_RegisterResetValue 0x0
#define CRYPTOCFG_7_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_28_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_28_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_29 (DWC_ufshc_block_BaseAddress + 0x13f4)
#define CRYPTOCFG_7_29_RegisterSize 32
#define CRYPTOCFG_7_29_RegisterResetValue 0x0
#define CRYPTOCFG_7_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_29_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_29_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_30 (DWC_ufshc_block_BaseAddress + 0x13f8)
#define CRYPTOCFG_7_30_RegisterSize 32
#define CRYPTOCFG_7_30_RegisterResetValue 0x0
#define CRYPTOCFG_7_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_30_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_30_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_7_31 (DWC_ufshc_block_BaseAddress + 0x13fc)
#define CRYPTOCFG_7_31_RegisterSize 32
#define CRYPTOCFG_7_31_RegisterResetValue 0x0
#define CRYPTOCFG_7_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_7_31_RESERVED_7_BitAddressOffset 0
#define CRYPTOCFG_7_31_RESERVED_7_RegisterSize 32





#define CRYPTOCFG_8_0 (DWC_ufshc_block_BaseAddress + 0x1400)
#define CRYPTOCFG_8_0_RegisterSize 32
#define CRYPTOCFG_8_0_RegisterResetValue 0x0
#define CRYPTOCFG_8_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_0_CRYPTOKEY_8_0_BitAddressOffset 0
#define CRYPTOCFG_8_0_CRYPTOKEY_8_0_RegisterSize 32





#define CRYPTOCFG_8_1 (DWC_ufshc_block_BaseAddress + 0x1404)
#define CRYPTOCFG_8_1_RegisterSize 32
#define CRYPTOCFG_8_1_RegisterResetValue 0x0
#define CRYPTOCFG_8_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_1_CRYPTOKEY_8_1_BitAddressOffset 0
#define CRYPTOCFG_8_1_CRYPTOKEY_8_1_RegisterSize 32





#define CRYPTOCFG_8_2 (DWC_ufshc_block_BaseAddress + 0x1408)
#define CRYPTOCFG_8_2_RegisterSize 32
#define CRYPTOCFG_8_2_RegisterResetValue 0x0
#define CRYPTOCFG_8_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_2_CRYPTOKEY_8_2_BitAddressOffset 0
#define CRYPTOCFG_8_2_CRYPTOKEY_8_2_RegisterSize 32





#define CRYPTOCFG_8_3 (DWC_ufshc_block_BaseAddress + 0x140c)
#define CRYPTOCFG_8_3_RegisterSize 32
#define CRYPTOCFG_8_3_RegisterResetValue 0x0
#define CRYPTOCFG_8_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_3_CRYPTOKEY_8_3_BitAddressOffset 0
#define CRYPTOCFG_8_3_CRYPTOKEY_8_3_RegisterSize 32





#define CRYPTOCFG_8_4 (DWC_ufshc_block_BaseAddress + 0x1410)
#define CRYPTOCFG_8_4_RegisterSize 32
#define CRYPTOCFG_8_4_RegisterResetValue 0x0
#define CRYPTOCFG_8_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_4_CRYPTOKEY_8_4_BitAddressOffset 0
#define CRYPTOCFG_8_4_CRYPTOKEY_8_4_RegisterSize 32





#define CRYPTOCFG_8_5 (DWC_ufshc_block_BaseAddress + 0x1414)
#define CRYPTOCFG_8_5_RegisterSize 32
#define CRYPTOCFG_8_5_RegisterResetValue 0x0
#define CRYPTOCFG_8_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_5_CRYPTOKEY_8_5_BitAddressOffset 0
#define CRYPTOCFG_8_5_CRYPTOKEY_8_5_RegisterSize 32





#define CRYPTOCFG_8_6 (DWC_ufshc_block_BaseAddress + 0x1418)
#define CRYPTOCFG_8_6_RegisterSize 32
#define CRYPTOCFG_8_6_RegisterResetValue 0x0
#define CRYPTOCFG_8_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_6_CRYPTOKEY_8_6_BitAddressOffset 0
#define CRYPTOCFG_8_6_CRYPTOKEY_8_6_RegisterSize 32





#define CRYPTOCFG_8_7 (DWC_ufshc_block_BaseAddress + 0x141c)
#define CRYPTOCFG_8_7_RegisterSize 32
#define CRYPTOCFG_8_7_RegisterResetValue 0x0
#define CRYPTOCFG_8_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_7_CRYPTOKEY_8_7_BitAddressOffset 0
#define CRYPTOCFG_8_7_CRYPTOKEY_8_7_RegisterSize 32





#define CRYPTOCFG_8_8 (DWC_ufshc_block_BaseAddress + 0x1420)
#define CRYPTOCFG_8_8_RegisterSize 32
#define CRYPTOCFG_8_8_RegisterResetValue 0x0
#define CRYPTOCFG_8_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_8_CRYPTOKEY_8_8_BitAddressOffset 0
#define CRYPTOCFG_8_8_CRYPTOKEY_8_8_RegisterSize 32





#define CRYPTOCFG_8_9 (DWC_ufshc_block_BaseAddress + 0x1424)
#define CRYPTOCFG_8_9_RegisterSize 32
#define CRYPTOCFG_8_9_RegisterResetValue 0x0
#define CRYPTOCFG_8_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_9_CRYPTOKEY_8_9_BitAddressOffset 0
#define CRYPTOCFG_8_9_CRYPTOKEY_8_9_RegisterSize 32





#define CRYPTOCFG_8_10 (DWC_ufshc_block_BaseAddress + 0x1428)
#define CRYPTOCFG_8_10_RegisterSize 32
#define CRYPTOCFG_8_10_RegisterResetValue 0x0
#define CRYPTOCFG_8_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_10_CRYPTOKEY_8_10_BitAddressOffset 0
#define CRYPTOCFG_8_10_CRYPTOKEY_8_10_RegisterSize 32





#define CRYPTOCFG_8_11 (DWC_ufshc_block_BaseAddress + 0x142c)
#define CRYPTOCFG_8_11_RegisterSize 32
#define CRYPTOCFG_8_11_RegisterResetValue 0x0
#define CRYPTOCFG_8_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_11_CRYPTOKEY_8_11_BitAddressOffset 0
#define CRYPTOCFG_8_11_CRYPTOKEY_8_11_RegisterSize 32





#define CRYPTOCFG_8_12 (DWC_ufshc_block_BaseAddress + 0x1430)
#define CRYPTOCFG_8_12_RegisterSize 32
#define CRYPTOCFG_8_12_RegisterResetValue 0x0
#define CRYPTOCFG_8_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_12_CRYPTOKEY_8_12_BitAddressOffset 0
#define CRYPTOCFG_8_12_CRYPTOKEY_8_12_RegisterSize 32





#define CRYPTOCFG_8_13 (DWC_ufshc_block_BaseAddress + 0x1434)
#define CRYPTOCFG_8_13_RegisterSize 32
#define CRYPTOCFG_8_13_RegisterResetValue 0x0
#define CRYPTOCFG_8_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_13_CRYPTOKEY_8_13_BitAddressOffset 0
#define CRYPTOCFG_8_13_CRYPTOKEY_8_13_RegisterSize 32





#define CRYPTOCFG_8_14 (DWC_ufshc_block_BaseAddress + 0x1438)
#define CRYPTOCFG_8_14_RegisterSize 32
#define CRYPTOCFG_8_14_RegisterResetValue 0x0
#define CRYPTOCFG_8_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_14_CRYPTOKEY_8_14_BitAddressOffset 0
#define CRYPTOCFG_8_14_CRYPTOKEY_8_14_RegisterSize 32





#define CRYPTOCFG_8_15 (DWC_ufshc_block_BaseAddress + 0x143c)
#define CRYPTOCFG_8_15_RegisterSize 32
#define CRYPTOCFG_8_15_RegisterResetValue 0x0
#define CRYPTOCFG_8_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_15_CRYPTOKEY_8_15_BitAddressOffset 0
#define CRYPTOCFG_8_15_CRYPTOKEY_8_15_RegisterSize 32





#define CRYPTOCFG_8_16 (DWC_ufshc_block_BaseAddress + 0x1440)
#define CRYPTOCFG_8_16_RegisterSize 32
#define CRYPTOCFG_8_16_RegisterResetValue 0x0
#define CRYPTOCFG_8_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_16_DUSIZE_8_BitAddressOffset 0
#define CRYPTOCFG_8_16_DUSIZE_8_RegisterSize 8



#define CRYPTOCFG_8_16_CAPIDX_8_BitAddressOffset 8
#define CRYPTOCFG_8_16_CAPIDX_8_RegisterSize 8



#define CRYPTOCFG_8_16_CFGE_8_BitAddressOffset 31
#define CRYPTOCFG_8_16_CFGE_8_RegisterSize 1





#define CRYPTOCFG_8_17 (DWC_ufshc_block_BaseAddress + 0x1444)
#define CRYPTOCFG_8_17_RegisterSize 32
#define CRYPTOCFG_8_17_RegisterResetValue 0x0
#define CRYPTOCFG_8_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_17_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_17_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_18 (DWC_ufshc_block_BaseAddress + 0x1448)
#define CRYPTOCFG_8_18_RegisterSize 32
#define CRYPTOCFG_8_18_RegisterResetValue 0x0
#define CRYPTOCFG_8_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_18_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_18_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_19 (DWC_ufshc_block_BaseAddress + 0x144c)
#define CRYPTOCFG_8_19_RegisterSize 32
#define CRYPTOCFG_8_19_RegisterResetValue 0x0
#define CRYPTOCFG_8_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_19_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_19_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_20 (DWC_ufshc_block_BaseAddress + 0x1450)
#define CRYPTOCFG_8_20_RegisterSize 32
#define CRYPTOCFG_8_20_RegisterResetValue 0x0
#define CRYPTOCFG_8_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_20_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_20_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_21 (DWC_ufshc_block_BaseAddress + 0x1454)
#define CRYPTOCFG_8_21_RegisterSize 32
#define CRYPTOCFG_8_21_RegisterResetValue 0x0
#define CRYPTOCFG_8_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_21_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_21_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_22 (DWC_ufshc_block_BaseAddress + 0x1458)
#define CRYPTOCFG_8_22_RegisterSize 32
#define CRYPTOCFG_8_22_RegisterResetValue 0x0
#define CRYPTOCFG_8_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_22_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_22_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_23 (DWC_ufshc_block_BaseAddress + 0x145c)
#define CRYPTOCFG_8_23_RegisterSize 32
#define CRYPTOCFG_8_23_RegisterResetValue 0x0
#define CRYPTOCFG_8_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_23_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_23_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_24 (DWC_ufshc_block_BaseAddress + 0x1460)
#define CRYPTOCFG_8_24_RegisterSize 32
#define CRYPTOCFG_8_24_RegisterResetValue 0x0
#define CRYPTOCFG_8_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_24_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_24_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_25 (DWC_ufshc_block_BaseAddress + 0x1464)
#define CRYPTOCFG_8_25_RegisterSize 32
#define CRYPTOCFG_8_25_RegisterResetValue 0x0
#define CRYPTOCFG_8_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_25_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_25_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_26 (DWC_ufshc_block_BaseAddress + 0x1468)
#define CRYPTOCFG_8_26_RegisterSize 32
#define CRYPTOCFG_8_26_RegisterResetValue 0x0
#define CRYPTOCFG_8_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_26_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_26_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_27 (DWC_ufshc_block_BaseAddress + 0x146c)
#define CRYPTOCFG_8_27_RegisterSize 32
#define CRYPTOCFG_8_27_RegisterResetValue 0x0
#define CRYPTOCFG_8_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_27_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_27_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_28 (DWC_ufshc_block_BaseAddress + 0x1470)
#define CRYPTOCFG_8_28_RegisterSize 32
#define CRYPTOCFG_8_28_RegisterResetValue 0x0
#define CRYPTOCFG_8_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_28_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_28_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_29 (DWC_ufshc_block_BaseAddress + 0x1474)
#define CRYPTOCFG_8_29_RegisterSize 32
#define CRYPTOCFG_8_29_RegisterResetValue 0x0
#define CRYPTOCFG_8_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_29_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_29_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_30 (DWC_ufshc_block_BaseAddress + 0x1478)
#define CRYPTOCFG_8_30_RegisterSize 32
#define CRYPTOCFG_8_30_RegisterResetValue 0x0
#define CRYPTOCFG_8_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_30_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_30_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_8_31 (DWC_ufshc_block_BaseAddress + 0x147c)
#define CRYPTOCFG_8_31_RegisterSize 32
#define CRYPTOCFG_8_31_RegisterResetValue 0x0
#define CRYPTOCFG_8_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_8_31_RESERVED_8_BitAddressOffset 0
#define CRYPTOCFG_8_31_RESERVED_8_RegisterSize 32





#define CRYPTOCFG_9_0 (DWC_ufshc_block_BaseAddress + 0x1480)
#define CRYPTOCFG_9_0_RegisterSize 32
#define CRYPTOCFG_9_0_RegisterResetValue 0x0
#define CRYPTOCFG_9_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_0_CRYPTOKEY_9_0_BitAddressOffset 0
#define CRYPTOCFG_9_0_CRYPTOKEY_9_0_RegisterSize 32





#define CRYPTOCFG_9_1 (DWC_ufshc_block_BaseAddress + 0x1484)
#define CRYPTOCFG_9_1_RegisterSize 32
#define CRYPTOCFG_9_1_RegisterResetValue 0x0
#define CRYPTOCFG_9_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_1_CRYPTOKEY_9_1_BitAddressOffset 0
#define CRYPTOCFG_9_1_CRYPTOKEY_9_1_RegisterSize 32





#define CRYPTOCFG_9_2 (DWC_ufshc_block_BaseAddress + 0x1488)
#define CRYPTOCFG_9_2_RegisterSize 32
#define CRYPTOCFG_9_2_RegisterResetValue 0x0
#define CRYPTOCFG_9_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_2_CRYPTOKEY_9_2_BitAddressOffset 0
#define CRYPTOCFG_9_2_CRYPTOKEY_9_2_RegisterSize 32





#define CRYPTOCFG_9_3 (DWC_ufshc_block_BaseAddress + 0x148c)
#define CRYPTOCFG_9_3_RegisterSize 32
#define CRYPTOCFG_9_3_RegisterResetValue 0x0
#define CRYPTOCFG_9_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_3_CRYPTOKEY_9_3_BitAddressOffset 0
#define CRYPTOCFG_9_3_CRYPTOKEY_9_3_RegisterSize 32





#define CRYPTOCFG_9_4 (DWC_ufshc_block_BaseAddress + 0x1490)
#define CRYPTOCFG_9_4_RegisterSize 32
#define CRYPTOCFG_9_4_RegisterResetValue 0x0
#define CRYPTOCFG_9_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_4_CRYPTOKEY_9_4_BitAddressOffset 0
#define CRYPTOCFG_9_4_CRYPTOKEY_9_4_RegisterSize 32





#define CRYPTOCFG_9_5 (DWC_ufshc_block_BaseAddress + 0x1494)
#define CRYPTOCFG_9_5_RegisterSize 32
#define CRYPTOCFG_9_5_RegisterResetValue 0x0
#define CRYPTOCFG_9_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_5_CRYPTOKEY_9_5_BitAddressOffset 0
#define CRYPTOCFG_9_5_CRYPTOKEY_9_5_RegisterSize 32





#define CRYPTOCFG_9_6 (DWC_ufshc_block_BaseAddress + 0x1498)
#define CRYPTOCFG_9_6_RegisterSize 32
#define CRYPTOCFG_9_6_RegisterResetValue 0x0
#define CRYPTOCFG_9_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_6_CRYPTOKEY_9_6_BitAddressOffset 0
#define CRYPTOCFG_9_6_CRYPTOKEY_9_6_RegisterSize 32





#define CRYPTOCFG_9_7 (DWC_ufshc_block_BaseAddress + 0x149c)
#define CRYPTOCFG_9_7_RegisterSize 32
#define CRYPTOCFG_9_7_RegisterResetValue 0x0
#define CRYPTOCFG_9_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_7_CRYPTOKEY_9_7_BitAddressOffset 0
#define CRYPTOCFG_9_7_CRYPTOKEY_9_7_RegisterSize 32





#define CRYPTOCFG_9_8 (DWC_ufshc_block_BaseAddress + 0x14a0)
#define CRYPTOCFG_9_8_RegisterSize 32
#define CRYPTOCFG_9_8_RegisterResetValue 0x0
#define CRYPTOCFG_9_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_8_CRYPTOKEY_9_8_BitAddressOffset 0
#define CRYPTOCFG_9_8_CRYPTOKEY_9_8_RegisterSize 32





#define CRYPTOCFG_9_9 (DWC_ufshc_block_BaseAddress + 0x14a4)
#define CRYPTOCFG_9_9_RegisterSize 32
#define CRYPTOCFG_9_9_RegisterResetValue 0x0
#define CRYPTOCFG_9_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_9_CRYPTOKEY_9_9_BitAddressOffset 0
#define CRYPTOCFG_9_9_CRYPTOKEY_9_9_RegisterSize 32





#define CRYPTOCFG_9_10 (DWC_ufshc_block_BaseAddress + 0x14a8)
#define CRYPTOCFG_9_10_RegisterSize 32
#define CRYPTOCFG_9_10_RegisterResetValue 0x0
#define CRYPTOCFG_9_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_10_CRYPTOKEY_9_10_BitAddressOffset 0
#define CRYPTOCFG_9_10_CRYPTOKEY_9_10_RegisterSize 32





#define CRYPTOCFG_9_11 (DWC_ufshc_block_BaseAddress + 0x14ac)
#define CRYPTOCFG_9_11_RegisterSize 32
#define CRYPTOCFG_9_11_RegisterResetValue 0x0
#define CRYPTOCFG_9_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_11_CRYPTOKEY_9_11_BitAddressOffset 0
#define CRYPTOCFG_9_11_CRYPTOKEY_9_11_RegisterSize 32





#define CRYPTOCFG_9_12 (DWC_ufshc_block_BaseAddress + 0x14b0)
#define CRYPTOCFG_9_12_RegisterSize 32
#define CRYPTOCFG_9_12_RegisterResetValue 0x0
#define CRYPTOCFG_9_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_12_CRYPTOKEY_9_12_BitAddressOffset 0
#define CRYPTOCFG_9_12_CRYPTOKEY_9_12_RegisterSize 32





#define CRYPTOCFG_9_13 (DWC_ufshc_block_BaseAddress + 0x14b4)
#define CRYPTOCFG_9_13_RegisterSize 32
#define CRYPTOCFG_9_13_RegisterResetValue 0x0
#define CRYPTOCFG_9_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_13_CRYPTOKEY_9_13_BitAddressOffset 0
#define CRYPTOCFG_9_13_CRYPTOKEY_9_13_RegisterSize 32





#define CRYPTOCFG_9_14 (DWC_ufshc_block_BaseAddress + 0x14b8)
#define CRYPTOCFG_9_14_RegisterSize 32
#define CRYPTOCFG_9_14_RegisterResetValue 0x0
#define CRYPTOCFG_9_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_14_CRYPTOKEY_9_14_BitAddressOffset 0
#define CRYPTOCFG_9_14_CRYPTOKEY_9_14_RegisterSize 32





#define CRYPTOCFG_9_15 (DWC_ufshc_block_BaseAddress + 0x14bc)
#define CRYPTOCFG_9_15_RegisterSize 32
#define CRYPTOCFG_9_15_RegisterResetValue 0x0
#define CRYPTOCFG_9_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_15_CRYPTOKEY_9_15_BitAddressOffset 0
#define CRYPTOCFG_9_15_CRYPTOKEY_9_15_RegisterSize 32





#define CRYPTOCFG_9_16 (DWC_ufshc_block_BaseAddress + 0x14c0)
#define CRYPTOCFG_9_16_RegisterSize 32
#define CRYPTOCFG_9_16_RegisterResetValue 0x0
#define CRYPTOCFG_9_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_16_DUSIZE_9_BitAddressOffset 0
#define CRYPTOCFG_9_16_DUSIZE_9_RegisterSize 8



#define CRYPTOCFG_9_16_CAPIDX_9_BitAddressOffset 8
#define CRYPTOCFG_9_16_CAPIDX_9_RegisterSize 8



#define CRYPTOCFG_9_16_CFGE_9_BitAddressOffset 31
#define CRYPTOCFG_9_16_CFGE_9_RegisterSize 1





#define CRYPTOCFG_9_17 (DWC_ufshc_block_BaseAddress + 0x14c4)
#define CRYPTOCFG_9_17_RegisterSize 32
#define CRYPTOCFG_9_17_RegisterResetValue 0x0
#define CRYPTOCFG_9_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_17_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_17_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_18 (DWC_ufshc_block_BaseAddress + 0x14c8)
#define CRYPTOCFG_9_18_RegisterSize 32
#define CRYPTOCFG_9_18_RegisterResetValue 0x0
#define CRYPTOCFG_9_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_18_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_18_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_19 (DWC_ufshc_block_BaseAddress + 0x14cc)
#define CRYPTOCFG_9_19_RegisterSize 32
#define CRYPTOCFG_9_19_RegisterResetValue 0x0
#define CRYPTOCFG_9_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_19_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_19_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_20 (DWC_ufshc_block_BaseAddress + 0x14d0)
#define CRYPTOCFG_9_20_RegisterSize 32
#define CRYPTOCFG_9_20_RegisterResetValue 0x0
#define CRYPTOCFG_9_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_20_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_20_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_21 (DWC_ufshc_block_BaseAddress + 0x14d4)
#define CRYPTOCFG_9_21_RegisterSize 32
#define CRYPTOCFG_9_21_RegisterResetValue 0x0
#define CRYPTOCFG_9_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_21_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_21_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_22 (DWC_ufshc_block_BaseAddress + 0x14d8)
#define CRYPTOCFG_9_22_RegisterSize 32
#define CRYPTOCFG_9_22_RegisterResetValue 0x0
#define CRYPTOCFG_9_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_22_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_22_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_23 (DWC_ufshc_block_BaseAddress + 0x14dc)
#define CRYPTOCFG_9_23_RegisterSize 32
#define CRYPTOCFG_9_23_RegisterResetValue 0x0
#define CRYPTOCFG_9_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_23_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_23_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_24 (DWC_ufshc_block_BaseAddress + 0x14e0)
#define CRYPTOCFG_9_24_RegisterSize 32
#define CRYPTOCFG_9_24_RegisterResetValue 0x0
#define CRYPTOCFG_9_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_24_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_24_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_25 (DWC_ufshc_block_BaseAddress + 0x14e4)
#define CRYPTOCFG_9_25_RegisterSize 32
#define CRYPTOCFG_9_25_RegisterResetValue 0x0
#define CRYPTOCFG_9_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_25_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_25_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_26 (DWC_ufshc_block_BaseAddress + 0x14e8)
#define CRYPTOCFG_9_26_RegisterSize 32
#define CRYPTOCFG_9_26_RegisterResetValue 0x0
#define CRYPTOCFG_9_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_26_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_26_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_27 (DWC_ufshc_block_BaseAddress + 0x14ec)
#define CRYPTOCFG_9_27_RegisterSize 32
#define CRYPTOCFG_9_27_RegisterResetValue 0x0
#define CRYPTOCFG_9_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_27_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_27_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_28 (DWC_ufshc_block_BaseAddress + 0x14f0)
#define CRYPTOCFG_9_28_RegisterSize 32
#define CRYPTOCFG_9_28_RegisterResetValue 0x0
#define CRYPTOCFG_9_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_28_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_28_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_29 (DWC_ufshc_block_BaseAddress + 0x14f4)
#define CRYPTOCFG_9_29_RegisterSize 32
#define CRYPTOCFG_9_29_RegisterResetValue 0x0
#define CRYPTOCFG_9_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_29_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_29_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_30 (DWC_ufshc_block_BaseAddress + 0x14f8)
#define CRYPTOCFG_9_30_RegisterSize 32
#define CRYPTOCFG_9_30_RegisterResetValue 0x0
#define CRYPTOCFG_9_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_30_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_30_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_9_31 (DWC_ufshc_block_BaseAddress + 0x14fc)
#define CRYPTOCFG_9_31_RegisterSize 32
#define CRYPTOCFG_9_31_RegisterResetValue 0x0
#define CRYPTOCFG_9_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_9_31_RESERVED_9_BitAddressOffset 0
#define CRYPTOCFG_9_31_RESERVED_9_RegisterSize 32





#define CRYPTOCFG_10_0 (DWC_ufshc_block_BaseAddress + 0x1500)
#define CRYPTOCFG_10_0_RegisterSize 32
#define CRYPTOCFG_10_0_RegisterResetValue 0x0
#define CRYPTOCFG_10_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_0_CRYPTOKEY_10_0_BitAddressOffset 0
#define CRYPTOCFG_10_0_CRYPTOKEY_10_0_RegisterSize 32





#define CRYPTOCFG_10_1 (DWC_ufshc_block_BaseAddress + 0x1504)
#define CRYPTOCFG_10_1_RegisterSize 32
#define CRYPTOCFG_10_1_RegisterResetValue 0x0
#define CRYPTOCFG_10_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_1_CRYPTOKEY_10_1_BitAddressOffset 0
#define CRYPTOCFG_10_1_CRYPTOKEY_10_1_RegisterSize 32





#define CRYPTOCFG_10_2 (DWC_ufshc_block_BaseAddress + 0x1508)
#define CRYPTOCFG_10_2_RegisterSize 32
#define CRYPTOCFG_10_2_RegisterResetValue 0x0
#define CRYPTOCFG_10_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_2_CRYPTOKEY_10_2_BitAddressOffset 0
#define CRYPTOCFG_10_2_CRYPTOKEY_10_2_RegisterSize 32





#define CRYPTOCFG_10_3 (DWC_ufshc_block_BaseAddress + 0x150c)
#define CRYPTOCFG_10_3_RegisterSize 32
#define CRYPTOCFG_10_3_RegisterResetValue 0x0
#define CRYPTOCFG_10_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_3_CRYPTOKEY_10_3_BitAddressOffset 0
#define CRYPTOCFG_10_3_CRYPTOKEY_10_3_RegisterSize 32





#define CRYPTOCFG_10_4 (DWC_ufshc_block_BaseAddress + 0x1510)
#define CRYPTOCFG_10_4_RegisterSize 32
#define CRYPTOCFG_10_4_RegisterResetValue 0x0
#define CRYPTOCFG_10_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_4_CRYPTOKEY_10_4_BitAddressOffset 0
#define CRYPTOCFG_10_4_CRYPTOKEY_10_4_RegisterSize 32





#define CRYPTOCFG_10_5 (DWC_ufshc_block_BaseAddress + 0x1514)
#define CRYPTOCFG_10_5_RegisterSize 32
#define CRYPTOCFG_10_5_RegisterResetValue 0x0
#define CRYPTOCFG_10_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_5_CRYPTOKEY_10_5_BitAddressOffset 0
#define CRYPTOCFG_10_5_CRYPTOKEY_10_5_RegisterSize 32





#define CRYPTOCFG_10_6 (DWC_ufshc_block_BaseAddress + 0x1518)
#define CRYPTOCFG_10_6_RegisterSize 32
#define CRYPTOCFG_10_6_RegisterResetValue 0x0
#define CRYPTOCFG_10_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_6_CRYPTOKEY_10_6_BitAddressOffset 0
#define CRYPTOCFG_10_6_CRYPTOKEY_10_6_RegisterSize 32





#define CRYPTOCFG_10_7 (DWC_ufshc_block_BaseAddress + 0x151c)
#define CRYPTOCFG_10_7_RegisterSize 32
#define CRYPTOCFG_10_7_RegisterResetValue 0x0
#define CRYPTOCFG_10_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_7_CRYPTOKEY_10_7_BitAddressOffset 0
#define CRYPTOCFG_10_7_CRYPTOKEY_10_7_RegisterSize 32





#define CRYPTOCFG_10_8 (DWC_ufshc_block_BaseAddress + 0x1520)
#define CRYPTOCFG_10_8_RegisterSize 32
#define CRYPTOCFG_10_8_RegisterResetValue 0x0
#define CRYPTOCFG_10_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_8_CRYPTOKEY_10_8_BitAddressOffset 0
#define CRYPTOCFG_10_8_CRYPTOKEY_10_8_RegisterSize 32





#define CRYPTOCFG_10_9 (DWC_ufshc_block_BaseAddress + 0x1524)
#define CRYPTOCFG_10_9_RegisterSize 32
#define CRYPTOCFG_10_9_RegisterResetValue 0x0
#define CRYPTOCFG_10_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_9_CRYPTOKEY_10_9_BitAddressOffset 0
#define CRYPTOCFG_10_9_CRYPTOKEY_10_9_RegisterSize 32





#define CRYPTOCFG_10_10 (DWC_ufshc_block_BaseAddress + 0x1528)
#define CRYPTOCFG_10_10_RegisterSize 32
#define CRYPTOCFG_10_10_RegisterResetValue 0x0
#define CRYPTOCFG_10_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_10_CRYPTOKEY_10_10_BitAddressOffset 0
#define CRYPTOCFG_10_10_CRYPTOKEY_10_10_RegisterSize 32





#define CRYPTOCFG_10_11 (DWC_ufshc_block_BaseAddress + 0x152c)
#define CRYPTOCFG_10_11_RegisterSize 32
#define CRYPTOCFG_10_11_RegisterResetValue 0x0
#define CRYPTOCFG_10_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_11_CRYPTOKEY_10_11_BitAddressOffset 0
#define CRYPTOCFG_10_11_CRYPTOKEY_10_11_RegisterSize 32





#define CRYPTOCFG_10_12 (DWC_ufshc_block_BaseAddress + 0x1530)
#define CRYPTOCFG_10_12_RegisterSize 32
#define CRYPTOCFG_10_12_RegisterResetValue 0x0
#define CRYPTOCFG_10_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_12_CRYPTOKEY_10_12_BitAddressOffset 0
#define CRYPTOCFG_10_12_CRYPTOKEY_10_12_RegisterSize 32





#define CRYPTOCFG_10_13 (DWC_ufshc_block_BaseAddress + 0x1534)
#define CRYPTOCFG_10_13_RegisterSize 32
#define CRYPTOCFG_10_13_RegisterResetValue 0x0
#define CRYPTOCFG_10_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_13_CRYPTOKEY_10_13_BitAddressOffset 0
#define CRYPTOCFG_10_13_CRYPTOKEY_10_13_RegisterSize 32





#define CRYPTOCFG_10_14 (DWC_ufshc_block_BaseAddress + 0x1538)
#define CRYPTOCFG_10_14_RegisterSize 32
#define CRYPTOCFG_10_14_RegisterResetValue 0x0
#define CRYPTOCFG_10_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_14_CRYPTOKEY_10_14_BitAddressOffset 0
#define CRYPTOCFG_10_14_CRYPTOKEY_10_14_RegisterSize 32





#define CRYPTOCFG_10_15 (DWC_ufshc_block_BaseAddress + 0x153c)
#define CRYPTOCFG_10_15_RegisterSize 32
#define CRYPTOCFG_10_15_RegisterResetValue 0x0
#define CRYPTOCFG_10_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_15_CRYPTOKEY_10_15_BitAddressOffset 0
#define CRYPTOCFG_10_15_CRYPTOKEY_10_15_RegisterSize 32





#define CRYPTOCFG_10_16 (DWC_ufshc_block_BaseAddress + 0x1540)
#define CRYPTOCFG_10_16_RegisterSize 32
#define CRYPTOCFG_10_16_RegisterResetValue 0x0
#define CRYPTOCFG_10_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_16_DUSIZE_10_BitAddressOffset 0
#define CRYPTOCFG_10_16_DUSIZE_10_RegisterSize 8



#define CRYPTOCFG_10_16_CAPIDX_10_BitAddressOffset 8
#define CRYPTOCFG_10_16_CAPIDX_10_RegisterSize 8



#define CRYPTOCFG_10_16_CFGE_10_BitAddressOffset 31
#define CRYPTOCFG_10_16_CFGE_10_RegisterSize 1





#define CRYPTOCFG_10_17 (DWC_ufshc_block_BaseAddress + 0x1544)
#define CRYPTOCFG_10_17_RegisterSize 32
#define CRYPTOCFG_10_17_RegisterResetValue 0x0
#define CRYPTOCFG_10_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_17_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_17_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_18 (DWC_ufshc_block_BaseAddress + 0x1548)
#define CRYPTOCFG_10_18_RegisterSize 32
#define CRYPTOCFG_10_18_RegisterResetValue 0x0
#define CRYPTOCFG_10_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_18_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_18_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_19 (DWC_ufshc_block_BaseAddress + 0x154c)
#define CRYPTOCFG_10_19_RegisterSize 32
#define CRYPTOCFG_10_19_RegisterResetValue 0x0
#define CRYPTOCFG_10_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_19_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_19_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_20 (DWC_ufshc_block_BaseAddress + 0x1550)
#define CRYPTOCFG_10_20_RegisterSize 32
#define CRYPTOCFG_10_20_RegisterResetValue 0x0
#define CRYPTOCFG_10_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_20_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_20_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_21 (DWC_ufshc_block_BaseAddress + 0x1554)
#define CRYPTOCFG_10_21_RegisterSize 32
#define CRYPTOCFG_10_21_RegisterResetValue 0x0
#define CRYPTOCFG_10_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_21_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_21_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_22 (DWC_ufshc_block_BaseAddress + 0x1558)
#define CRYPTOCFG_10_22_RegisterSize 32
#define CRYPTOCFG_10_22_RegisterResetValue 0x0
#define CRYPTOCFG_10_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_22_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_22_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_23 (DWC_ufshc_block_BaseAddress + 0x155c)
#define CRYPTOCFG_10_23_RegisterSize 32
#define CRYPTOCFG_10_23_RegisterResetValue 0x0
#define CRYPTOCFG_10_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_23_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_23_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_24 (DWC_ufshc_block_BaseAddress + 0x1560)
#define CRYPTOCFG_10_24_RegisterSize 32
#define CRYPTOCFG_10_24_RegisterResetValue 0x0
#define CRYPTOCFG_10_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_24_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_24_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_25 (DWC_ufshc_block_BaseAddress + 0x1564)
#define CRYPTOCFG_10_25_RegisterSize 32
#define CRYPTOCFG_10_25_RegisterResetValue 0x0
#define CRYPTOCFG_10_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_25_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_25_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_26 (DWC_ufshc_block_BaseAddress + 0x1568)
#define CRYPTOCFG_10_26_RegisterSize 32
#define CRYPTOCFG_10_26_RegisterResetValue 0x0
#define CRYPTOCFG_10_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_26_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_26_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_27 (DWC_ufshc_block_BaseAddress + 0x156c)
#define CRYPTOCFG_10_27_RegisterSize 32
#define CRYPTOCFG_10_27_RegisterResetValue 0x0
#define CRYPTOCFG_10_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_27_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_27_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_28 (DWC_ufshc_block_BaseAddress + 0x1570)
#define CRYPTOCFG_10_28_RegisterSize 32
#define CRYPTOCFG_10_28_RegisterResetValue 0x0
#define CRYPTOCFG_10_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_28_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_28_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_29 (DWC_ufshc_block_BaseAddress + 0x1574)
#define CRYPTOCFG_10_29_RegisterSize 32
#define CRYPTOCFG_10_29_RegisterResetValue 0x0
#define CRYPTOCFG_10_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_29_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_29_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_30 (DWC_ufshc_block_BaseAddress + 0x1578)
#define CRYPTOCFG_10_30_RegisterSize 32
#define CRYPTOCFG_10_30_RegisterResetValue 0x0
#define CRYPTOCFG_10_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_30_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_30_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_10_31 (DWC_ufshc_block_BaseAddress + 0x157c)
#define CRYPTOCFG_10_31_RegisterSize 32
#define CRYPTOCFG_10_31_RegisterResetValue 0x0
#define CRYPTOCFG_10_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_10_31_RESERVED_10_BitAddressOffset 0
#define CRYPTOCFG_10_31_RESERVED_10_RegisterSize 32





#define CRYPTOCFG_11_0 (DWC_ufshc_block_BaseAddress + 0x1580)
#define CRYPTOCFG_11_0_RegisterSize 32
#define CRYPTOCFG_11_0_RegisterResetValue 0x0
#define CRYPTOCFG_11_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_0_CRYPTOKEY_11_0_BitAddressOffset 0
#define CRYPTOCFG_11_0_CRYPTOKEY_11_0_RegisterSize 32





#define CRYPTOCFG_11_1 (DWC_ufshc_block_BaseAddress + 0x1584)
#define CRYPTOCFG_11_1_RegisterSize 32
#define CRYPTOCFG_11_1_RegisterResetValue 0x0
#define CRYPTOCFG_11_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_1_CRYPTOKEY_11_1_BitAddressOffset 0
#define CRYPTOCFG_11_1_CRYPTOKEY_11_1_RegisterSize 32





#define CRYPTOCFG_11_2 (DWC_ufshc_block_BaseAddress + 0x1588)
#define CRYPTOCFG_11_2_RegisterSize 32
#define CRYPTOCFG_11_2_RegisterResetValue 0x0
#define CRYPTOCFG_11_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_2_CRYPTOKEY_11_2_BitAddressOffset 0
#define CRYPTOCFG_11_2_CRYPTOKEY_11_2_RegisterSize 32





#define CRYPTOCFG_11_3 (DWC_ufshc_block_BaseAddress + 0x158c)
#define CRYPTOCFG_11_3_RegisterSize 32
#define CRYPTOCFG_11_3_RegisterResetValue 0x0
#define CRYPTOCFG_11_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_3_CRYPTOKEY_11_3_BitAddressOffset 0
#define CRYPTOCFG_11_3_CRYPTOKEY_11_3_RegisterSize 32





#define CRYPTOCFG_11_4 (DWC_ufshc_block_BaseAddress + 0x1590)
#define CRYPTOCFG_11_4_RegisterSize 32
#define CRYPTOCFG_11_4_RegisterResetValue 0x0
#define CRYPTOCFG_11_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_4_CRYPTOKEY_11_4_BitAddressOffset 0
#define CRYPTOCFG_11_4_CRYPTOKEY_11_4_RegisterSize 32





#define CRYPTOCFG_11_5 (DWC_ufshc_block_BaseAddress + 0x1594)
#define CRYPTOCFG_11_5_RegisterSize 32
#define CRYPTOCFG_11_5_RegisterResetValue 0x0
#define CRYPTOCFG_11_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_5_CRYPTOKEY_11_5_BitAddressOffset 0
#define CRYPTOCFG_11_5_CRYPTOKEY_11_5_RegisterSize 32





#define CRYPTOCFG_11_6 (DWC_ufshc_block_BaseAddress + 0x1598)
#define CRYPTOCFG_11_6_RegisterSize 32
#define CRYPTOCFG_11_6_RegisterResetValue 0x0
#define CRYPTOCFG_11_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_6_CRYPTOKEY_11_6_BitAddressOffset 0
#define CRYPTOCFG_11_6_CRYPTOKEY_11_6_RegisterSize 32





#define CRYPTOCFG_11_7 (DWC_ufshc_block_BaseAddress + 0x159c)
#define CRYPTOCFG_11_7_RegisterSize 32
#define CRYPTOCFG_11_7_RegisterResetValue 0x0
#define CRYPTOCFG_11_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_7_CRYPTOKEY_11_7_BitAddressOffset 0
#define CRYPTOCFG_11_7_CRYPTOKEY_11_7_RegisterSize 32





#define CRYPTOCFG_11_8 (DWC_ufshc_block_BaseAddress + 0x15a0)
#define CRYPTOCFG_11_8_RegisterSize 32
#define CRYPTOCFG_11_8_RegisterResetValue 0x0
#define CRYPTOCFG_11_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_8_CRYPTOKEY_11_8_BitAddressOffset 0
#define CRYPTOCFG_11_8_CRYPTOKEY_11_8_RegisterSize 32





#define CRYPTOCFG_11_9 (DWC_ufshc_block_BaseAddress + 0x15a4)
#define CRYPTOCFG_11_9_RegisterSize 32
#define CRYPTOCFG_11_9_RegisterResetValue 0x0
#define CRYPTOCFG_11_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_9_CRYPTOKEY_11_9_BitAddressOffset 0
#define CRYPTOCFG_11_9_CRYPTOKEY_11_9_RegisterSize 32





#define CRYPTOCFG_11_10 (DWC_ufshc_block_BaseAddress + 0x15a8)
#define CRYPTOCFG_11_10_RegisterSize 32
#define CRYPTOCFG_11_10_RegisterResetValue 0x0
#define CRYPTOCFG_11_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_10_CRYPTOKEY_11_10_BitAddressOffset 0
#define CRYPTOCFG_11_10_CRYPTOKEY_11_10_RegisterSize 32





#define CRYPTOCFG_11_11 (DWC_ufshc_block_BaseAddress + 0x15ac)
#define CRYPTOCFG_11_11_RegisterSize 32
#define CRYPTOCFG_11_11_RegisterResetValue 0x0
#define CRYPTOCFG_11_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_11_CRYPTOKEY_11_11_BitAddressOffset 0
#define CRYPTOCFG_11_11_CRYPTOKEY_11_11_RegisterSize 32





#define CRYPTOCFG_11_12 (DWC_ufshc_block_BaseAddress + 0x15b0)
#define CRYPTOCFG_11_12_RegisterSize 32
#define CRYPTOCFG_11_12_RegisterResetValue 0x0
#define CRYPTOCFG_11_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_12_CRYPTOKEY_11_12_BitAddressOffset 0
#define CRYPTOCFG_11_12_CRYPTOKEY_11_12_RegisterSize 32





#define CRYPTOCFG_11_13 (DWC_ufshc_block_BaseAddress + 0x15b4)
#define CRYPTOCFG_11_13_RegisterSize 32
#define CRYPTOCFG_11_13_RegisterResetValue 0x0
#define CRYPTOCFG_11_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_13_CRYPTOKEY_11_13_BitAddressOffset 0
#define CRYPTOCFG_11_13_CRYPTOKEY_11_13_RegisterSize 32





#define CRYPTOCFG_11_14 (DWC_ufshc_block_BaseAddress + 0x15b8)
#define CRYPTOCFG_11_14_RegisterSize 32
#define CRYPTOCFG_11_14_RegisterResetValue 0x0
#define CRYPTOCFG_11_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_14_CRYPTOKEY_11_14_BitAddressOffset 0
#define CRYPTOCFG_11_14_CRYPTOKEY_11_14_RegisterSize 32





#define CRYPTOCFG_11_15 (DWC_ufshc_block_BaseAddress + 0x15bc)
#define CRYPTOCFG_11_15_RegisterSize 32
#define CRYPTOCFG_11_15_RegisterResetValue 0x0
#define CRYPTOCFG_11_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_15_CRYPTOKEY_11_15_BitAddressOffset 0
#define CRYPTOCFG_11_15_CRYPTOKEY_11_15_RegisterSize 32





#define CRYPTOCFG_11_16 (DWC_ufshc_block_BaseAddress + 0x15c0)
#define CRYPTOCFG_11_16_RegisterSize 32
#define CRYPTOCFG_11_16_RegisterResetValue 0x0
#define CRYPTOCFG_11_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_16_DUSIZE_11_BitAddressOffset 0
#define CRYPTOCFG_11_16_DUSIZE_11_RegisterSize 8



#define CRYPTOCFG_11_16_CAPIDX_11_BitAddressOffset 8
#define CRYPTOCFG_11_16_CAPIDX_11_RegisterSize 8



#define CRYPTOCFG_11_16_CFGE_11_BitAddressOffset 31
#define CRYPTOCFG_11_16_CFGE_11_RegisterSize 1





#define CRYPTOCFG_11_17 (DWC_ufshc_block_BaseAddress + 0x15c4)
#define CRYPTOCFG_11_17_RegisterSize 32
#define CRYPTOCFG_11_17_RegisterResetValue 0x0
#define CRYPTOCFG_11_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_17_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_17_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_18 (DWC_ufshc_block_BaseAddress + 0x15c8)
#define CRYPTOCFG_11_18_RegisterSize 32
#define CRYPTOCFG_11_18_RegisterResetValue 0x0
#define CRYPTOCFG_11_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_18_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_18_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_19 (DWC_ufshc_block_BaseAddress + 0x15cc)
#define CRYPTOCFG_11_19_RegisterSize 32
#define CRYPTOCFG_11_19_RegisterResetValue 0x0
#define CRYPTOCFG_11_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_19_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_19_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_20 (DWC_ufshc_block_BaseAddress + 0x15d0)
#define CRYPTOCFG_11_20_RegisterSize 32
#define CRYPTOCFG_11_20_RegisterResetValue 0x0
#define CRYPTOCFG_11_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_20_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_20_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_21 (DWC_ufshc_block_BaseAddress + 0x15d4)
#define CRYPTOCFG_11_21_RegisterSize 32
#define CRYPTOCFG_11_21_RegisterResetValue 0x0
#define CRYPTOCFG_11_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_21_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_21_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_22 (DWC_ufshc_block_BaseAddress + 0x15d8)
#define CRYPTOCFG_11_22_RegisterSize 32
#define CRYPTOCFG_11_22_RegisterResetValue 0x0
#define CRYPTOCFG_11_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_22_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_22_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_23 (DWC_ufshc_block_BaseAddress + 0x15dc)
#define CRYPTOCFG_11_23_RegisterSize 32
#define CRYPTOCFG_11_23_RegisterResetValue 0x0
#define CRYPTOCFG_11_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_23_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_23_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_24 (DWC_ufshc_block_BaseAddress + 0x15e0)
#define CRYPTOCFG_11_24_RegisterSize 32
#define CRYPTOCFG_11_24_RegisterResetValue 0x0
#define CRYPTOCFG_11_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_24_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_24_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_25 (DWC_ufshc_block_BaseAddress + 0x15e4)
#define CRYPTOCFG_11_25_RegisterSize 32
#define CRYPTOCFG_11_25_RegisterResetValue 0x0
#define CRYPTOCFG_11_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_25_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_25_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_26 (DWC_ufshc_block_BaseAddress + 0x15e8)
#define CRYPTOCFG_11_26_RegisterSize 32
#define CRYPTOCFG_11_26_RegisterResetValue 0x0
#define CRYPTOCFG_11_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_26_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_26_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_27 (DWC_ufshc_block_BaseAddress + 0x15ec)
#define CRYPTOCFG_11_27_RegisterSize 32
#define CRYPTOCFG_11_27_RegisterResetValue 0x0
#define CRYPTOCFG_11_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_27_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_27_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_28 (DWC_ufshc_block_BaseAddress + 0x15f0)
#define CRYPTOCFG_11_28_RegisterSize 32
#define CRYPTOCFG_11_28_RegisterResetValue 0x0
#define CRYPTOCFG_11_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_28_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_28_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_29 (DWC_ufshc_block_BaseAddress + 0x15f4)
#define CRYPTOCFG_11_29_RegisterSize 32
#define CRYPTOCFG_11_29_RegisterResetValue 0x0
#define CRYPTOCFG_11_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_29_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_29_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_30 (DWC_ufshc_block_BaseAddress + 0x15f8)
#define CRYPTOCFG_11_30_RegisterSize 32
#define CRYPTOCFG_11_30_RegisterResetValue 0x0
#define CRYPTOCFG_11_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_30_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_30_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_11_31 (DWC_ufshc_block_BaseAddress + 0x15fc)
#define CRYPTOCFG_11_31_RegisterSize 32
#define CRYPTOCFG_11_31_RegisterResetValue 0x0
#define CRYPTOCFG_11_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_11_31_RESERVED_11_BitAddressOffset 0
#define CRYPTOCFG_11_31_RESERVED_11_RegisterSize 32





#define CRYPTOCFG_12_0 (DWC_ufshc_block_BaseAddress + 0x1600)
#define CRYPTOCFG_12_0_RegisterSize 32
#define CRYPTOCFG_12_0_RegisterResetValue 0x0
#define CRYPTOCFG_12_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_0_CRYPTOKEY_12_0_BitAddressOffset 0
#define CRYPTOCFG_12_0_CRYPTOKEY_12_0_RegisterSize 32





#define CRYPTOCFG_12_1 (DWC_ufshc_block_BaseAddress + 0x1604)
#define CRYPTOCFG_12_1_RegisterSize 32
#define CRYPTOCFG_12_1_RegisterResetValue 0x0
#define CRYPTOCFG_12_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_1_CRYPTOKEY_12_1_BitAddressOffset 0
#define CRYPTOCFG_12_1_CRYPTOKEY_12_1_RegisterSize 32





#define CRYPTOCFG_12_2 (DWC_ufshc_block_BaseAddress + 0x1608)
#define CRYPTOCFG_12_2_RegisterSize 32
#define CRYPTOCFG_12_2_RegisterResetValue 0x0
#define CRYPTOCFG_12_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_2_CRYPTOKEY_12_2_BitAddressOffset 0
#define CRYPTOCFG_12_2_CRYPTOKEY_12_2_RegisterSize 32





#define CRYPTOCFG_12_3 (DWC_ufshc_block_BaseAddress + 0x160c)
#define CRYPTOCFG_12_3_RegisterSize 32
#define CRYPTOCFG_12_3_RegisterResetValue 0x0
#define CRYPTOCFG_12_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_3_CRYPTOKEY_12_3_BitAddressOffset 0
#define CRYPTOCFG_12_3_CRYPTOKEY_12_3_RegisterSize 32





#define CRYPTOCFG_12_4 (DWC_ufshc_block_BaseAddress + 0x1610)
#define CRYPTOCFG_12_4_RegisterSize 32
#define CRYPTOCFG_12_4_RegisterResetValue 0x0
#define CRYPTOCFG_12_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_4_CRYPTOKEY_12_4_BitAddressOffset 0
#define CRYPTOCFG_12_4_CRYPTOKEY_12_4_RegisterSize 32





#define CRYPTOCFG_12_5 (DWC_ufshc_block_BaseAddress + 0x1614)
#define CRYPTOCFG_12_5_RegisterSize 32
#define CRYPTOCFG_12_5_RegisterResetValue 0x0
#define CRYPTOCFG_12_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_5_CRYPTOKEY_12_5_BitAddressOffset 0
#define CRYPTOCFG_12_5_CRYPTOKEY_12_5_RegisterSize 32





#define CRYPTOCFG_12_6 (DWC_ufshc_block_BaseAddress + 0x1618)
#define CRYPTOCFG_12_6_RegisterSize 32
#define CRYPTOCFG_12_6_RegisterResetValue 0x0
#define CRYPTOCFG_12_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_6_CRYPTOKEY_12_6_BitAddressOffset 0
#define CRYPTOCFG_12_6_CRYPTOKEY_12_6_RegisterSize 32





#define CRYPTOCFG_12_7 (DWC_ufshc_block_BaseAddress + 0x161c)
#define CRYPTOCFG_12_7_RegisterSize 32
#define CRYPTOCFG_12_7_RegisterResetValue 0x0
#define CRYPTOCFG_12_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_7_CRYPTOKEY_12_7_BitAddressOffset 0
#define CRYPTOCFG_12_7_CRYPTOKEY_12_7_RegisterSize 32





#define CRYPTOCFG_12_8 (DWC_ufshc_block_BaseAddress + 0x1620)
#define CRYPTOCFG_12_8_RegisterSize 32
#define CRYPTOCFG_12_8_RegisterResetValue 0x0
#define CRYPTOCFG_12_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_8_CRYPTOKEY_12_8_BitAddressOffset 0
#define CRYPTOCFG_12_8_CRYPTOKEY_12_8_RegisterSize 32





#define CRYPTOCFG_12_9 (DWC_ufshc_block_BaseAddress + 0x1624)
#define CRYPTOCFG_12_9_RegisterSize 32
#define CRYPTOCFG_12_9_RegisterResetValue 0x0
#define CRYPTOCFG_12_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_9_CRYPTOKEY_12_9_BitAddressOffset 0
#define CRYPTOCFG_12_9_CRYPTOKEY_12_9_RegisterSize 32





#define CRYPTOCFG_12_10 (DWC_ufshc_block_BaseAddress + 0x1628)
#define CRYPTOCFG_12_10_RegisterSize 32
#define CRYPTOCFG_12_10_RegisterResetValue 0x0
#define CRYPTOCFG_12_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_10_CRYPTOKEY_12_10_BitAddressOffset 0
#define CRYPTOCFG_12_10_CRYPTOKEY_12_10_RegisterSize 32





#define CRYPTOCFG_12_11 (DWC_ufshc_block_BaseAddress + 0x162c)
#define CRYPTOCFG_12_11_RegisterSize 32
#define CRYPTOCFG_12_11_RegisterResetValue 0x0
#define CRYPTOCFG_12_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_11_CRYPTOKEY_12_11_BitAddressOffset 0
#define CRYPTOCFG_12_11_CRYPTOKEY_12_11_RegisterSize 32





#define CRYPTOCFG_12_12 (DWC_ufshc_block_BaseAddress + 0x1630)
#define CRYPTOCFG_12_12_RegisterSize 32
#define CRYPTOCFG_12_12_RegisterResetValue 0x0
#define CRYPTOCFG_12_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_12_CRYPTOKEY_12_12_BitAddressOffset 0
#define CRYPTOCFG_12_12_CRYPTOKEY_12_12_RegisterSize 32





#define CRYPTOCFG_12_13 (DWC_ufshc_block_BaseAddress + 0x1634)
#define CRYPTOCFG_12_13_RegisterSize 32
#define CRYPTOCFG_12_13_RegisterResetValue 0x0
#define CRYPTOCFG_12_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_13_CRYPTOKEY_12_13_BitAddressOffset 0
#define CRYPTOCFG_12_13_CRYPTOKEY_12_13_RegisterSize 32





#define CRYPTOCFG_12_14 (DWC_ufshc_block_BaseAddress + 0x1638)
#define CRYPTOCFG_12_14_RegisterSize 32
#define CRYPTOCFG_12_14_RegisterResetValue 0x0
#define CRYPTOCFG_12_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_14_CRYPTOKEY_12_14_BitAddressOffset 0
#define CRYPTOCFG_12_14_CRYPTOKEY_12_14_RegisterSize 32





#define CRYPTOCFG_12_15 (DWC_ufshc_block_BaseAddress + 0x163c)
#define CRYPTOCFG_12_15_RegisterSize 32
#define CRYPTOCFG_12_15_RegisterResetValue 0x0
#define CRYPTOCFG_12_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_15_CRYPTOKEY_12_15_BitAddressOffset 0
#define CRYPTOCFG_12_15_CRYPTOKEY_12_15_RegisterSize 32





#define CRYPTOCFG_12_16 (DWC_ufshc_block_BaseAddress + 0x1640)
#define CRYPTOCFG_12_16_RegisterSize 32
#define CRYPTOCFG_12_16_RegisterResetValue 0x0
#define CRYPTOCFG_12_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_16_DUSIZE_12_BitAddressOffset 0
#define CRYPTOCFG_12_16_DUSIZE_12_RegisterSize 8



#define CRYPTOCFG_12_16_CAPIDX_12_BitAddressOffset 8
#define CRYPTOCFG_12_16_CAPIDX_12_RegisterSize 8



#define CRYPTOCFG_12_16_CFGE_12_BitAddressOffset 31
#define CRYPTOCFG_12_16_CFGE_12_RegisterSize 1





#define CRYPTOCFG_12_17 (DWC_ufshc_block_BaseAddress + 0x1644)
#define CRYPTOCFG_12_17_RegisterSize 32
#define CRYPTOCFG_12_17_RegisterResetValue 0x0
#define CRYPTOCFG_12_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_17_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_17_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_18 (DWC_ufshc_block_BaseAddress + 0x1648)
#define CRYPTOCFG_12_18_RegisterSize 32
#define CRYPTOCFG_12_18_RegisterResetValue 0x0
#define CRYPTOCFG_12_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_18_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_18_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_19 (DWC_ufshc_block_BaseAddress + 0x164c)
#define CRYPTOCFG_12_19_RegisterSize 32
#define CRYPTOCFG_12_19_RegisterResetValue 0x0
#define CRYPTOCFG_12_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_19_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_19_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_20 (DWC_ufshc_block_BaseAddress + 0x1650)
#define CRYPTOCFG_12_20_RegisterSize 32
#define CRYPTOCFG_12_20_RegisterResetValue 0x0
#define CRYPTOCFG_12_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_20_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_20_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_21 (DWC_ufshc_block_BaseAddress + 0x1654)
#define CRYPTOCFG_12_21_RegisterSize 32
#define CRYPTOCFG_12_21_RegisterResetValue 0x0
#define CRYPTOCFG_12_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_21_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_21_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_22 (DWC_ufshc_block_BaseAddress + 0x1658)
#define CRYPTOCFG_12_22_RegisterSize 32
#define CRYPTOCFG_12_22_RegisterResetValue 0x0
#define CRYPTOCFG_12_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_22_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_22_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_23 (DWC_ufshc_block_BaseAddress + 0x165c)
#define CRYPTOCFG_12_23_RegisterSize 32
#define CRYPTOCFG_12_23_RegisterResetValue 0x0
#define CRYPTOCFG_12_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_23_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_23_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_24 (DWC_ufshc_block_BaseAddress + 0x1660)
#define CRYPTOCFG_12_24_RegisterSize 32
#define CRYPTOCFG_12_24_RegisterResetValue 0x0
#define CRYPTOCFG_12_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_24_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_24_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_25 (DWC_ufshc_block_BaseAddress + 0x1664)
#define CRYPTOCFG_12_25_RegisterSize 32
#define CRYPTOCFG_12_25_RegisterResetValue 0x0
#define CRYPTOCFG_12_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_25_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_25_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_26 (DWC_ufshc_block_BaseAddress + 0x1668)
#define CRYPTOCFG_12_26_RegisterSize 32
#define CRYPTOCFG_12_26_RegisterResetValue 0x0
#define CRYPTOCFG_12_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_26_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_26_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_27 (DWC_ufshc_block_BaseAddress + 0x166c)
#define CRYPTOCFG_12_27_RegisterSize 32
#define CRYPTOCFG_12_27_RegisterResetValue 0x0
#define CRYPTOCFG_12_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_27_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_27_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_28 (DWC_ufshc_block_BaseAddress + 0x1670)
#define CRYPTOCFG_12_28_RegisterSize 32
#define CRYPTOCFG_12_28_RegisterResetValue 0x0
#define CRYPTOCFG_12_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_28_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_28_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_29 (DWC_ufshc_block_BaseAddress + 0x1674)
#define CRYPTOCFG_12_29_RegisterSize 32
#define CRYPTOCFG_12_29_RegisterResetValue 0x0
#define CRYPTOCFG_12_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_29_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_29_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_30 (DWC_ufshc_block_BaseAddress + 0x1678)
#define CRYPTOCFG_12_30_RegisterSize 32
#define CRYPTOCFG_12_30_RegisterResetValue 0x0
#define CRYPTOCFG_12_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_30_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_30_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_12_31 (DWC_ufshc_block_BaseAddress + 0x167c)
#define CRYPTOCFG_12_31_RegisterSize 32
#define CRYPTOCFG_12_31_RegisterResetValue 0x0
#define CRYPTOCFG_12_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_12_31_RESERVED_12_BitAddressOffset 0
#define CRYPTOCFG_12_31_RESERVED_12_RegisterSize 32





#define CRYPTOCFG_13_0 (DWC_ufshc_block_BaseAddress + 0x1680)
#define CRYPTOCFG_13_0_RegisterSize 32
#define CRYPTOCFG_13_0_RegisterResetValue 0x0
#define CRYPTOCFG_13_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_0_CRYPTOKEY_13_0_BitAddressOffset 0
#define CRYPTOCFG_13_0_CRYPTOKEY_13_0_RegisterSize 32





#define CRYPTOCFG_13_1 (DWC_ufshc_block_BaseAddress + 0x1684)
#define CRYPTOCFG_13_1_RegisterSize 32
#define CRYPTOCFG_13_1_RegisterResetValue 0x0
#define CRYPTOCFG_13_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_1_CRYPTOKEY_13_1_BitAddressOffset 0
#define CRYPTOCFG_13_1_CRYPTOKEY_13_1_RegisterSize 32





#define CRYPTOCFG_13_2 (DWC_ufshc_block_BaseAddress + 0x1688)
#define CRYPTOCFG_13_2_RegisterSize 32
#define CRYPTOCFG_13_2_RegisterResetValue 0x0
#define CRYPTOCFG_13_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_2_CRYPTOKEY_13_2_BitAddressOffset 0
#define CRYPTOCFG_13_2_CRYPTOKEY_13_2_RegisterSize 32





#define CRYPTOCFG_13_3 (DWC_ufshc_block_BaseAddress + 0x168c)
#define CRYPTOCFG_13_3_RegisterSize 32
#define CRYPTOCFG_13_3_RegisterResetValue 0x0
#define CRYPTOCFG_13_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_3_CRYPTOKEY_13_3_BitAddressOffset 0
#define CRYPTOCFG_13_3_CRYPTOKEY_13_3_RegisterSize 32





#define CRYPTOCFG_13_4 (DWC_ufshc_block_BaseAddress + 0x1690)
#define CRYPTOCFG_13_4_RegisterSize 32
#define CRYPTOCFG_13_4_RegisterResetValue 0x0
#define CRYPTOCFG_13_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_4_CRYPTOKEY_13_4_BitAddressOffset 0
#define CRYPTOCFG_13_4_CRYPTOKEY_13_4_RegisterSize 32





#define CRYPTOCFG_13_5 (DWC_ufshc_block_BaseAddress + 0x1694)
#define CRYPTOCFG_13_5_RegisterSize 32
#define CRYPTOCFG_13_5_RegisterResetValue 0x0
#define CRYPTOCFG_13_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_5_CRYPTOKEY_13_5_BitAddressOffset 0
#define CRYPTOCFG_13_5_CRYPTOKEY_13_5_RegisterSize 32





#define CRYPTOCFG_13_6 (DWC_ufshc_block_BaseAddress + 0x1698)
#define CRYPTOCFG_13_6_RegisterSize 32
#define CRYPTOCFG_13_6_RegisterResetValue 0x0
#define CRYPTOCFG_13_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_6_CRYPTOKEY_13_6_BitAddressOffset 0
#define CRYPTOCFG_13_6_CRYPTOKEY_13_6_RegisterSize 32





#define CRYPTOCFG_13_7 (DWC_ufshc_block_BaseAddress + 0x169c)
#define CRYPTOCFG_13_7_RegisterSize 32
#define CRYPTOCFG_13_7_RegisterResetValue 0x0
#define CRYPTOCFG_13_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_7_CRYPTOKEY_13_7_BitAddressOffset 0
#define CRYPTOCFG_13_7_CRYPTOKEY_13_7_RegisterSize 32





#define CRYPTOCFG_13_8 (DWC_ufshc_block_BaseAddress + 0x16a0)
#define CRYPTOCFG_13_8_RegisterSize 32
#define CRYPTOCFG_13_8_RegisterResetValue 0x0
#define CRYPTOCFG_13_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_8_CRYPTOKEY_13_8_BitAddressOffset 0
#define CRYPTOCFG_13_8_CRYPTOKEY_13_8_RegisterSize 32





#define CRYPTOCFG_13_9 (DWC_ufshc_block_BaseAddress + 0x16a4)
#define CRYPTOCFG_13_9_RegisterSize 32
#define CRYPTOCFG_13_9_RegisterResetValue 0x0
#define CRYPTOCFG_13_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_9_CRYPTOKEY_13_9_BitAddressOffset 0
#define CRYPTOCFG_13_9_CRYPTOKEY_13_9_RegisterSize 32





#define CRYPTOCFG_13_10 (DWC_ufshc_block_BaseAddress + 0x16a8)
#define CRYPTOCFG_13_10_RegisterSize 32
#define CRYPTOCFG_13_10_RegisterResetValue 0x0
#define CRYPTOCFG_13_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_10_CRYPTOKEY_13_10_BitAddressOffset 0
#define CRYPTOCFG_13_10_CRYPTOKEY_13_10_RegisterSize 32





#define CRYPTOCFG_13_11 (DWC_ufshc_block_BaseAddress + 0x16ac)
#define CRYPTOCFG_13_11_RegisterSize 32
#define CRYPTOCFG_13_11_RegisterResetValue 0x0
#define CRYPTOCFG_13_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_11_CRYPTOKEY_13_11_BitAddressOffset 0
#define CRYPTOCFG_13_11_CRYPTOKEY_13_11_RegisterSize 32





#define CRYPTOCFG_13_12 (DWC_ufshc_block_BaseAddress + 0x16b0)
#define CRYPTOCFG_13_12_RegisterSize 32
#define CRYPTOCFG_13_12_RegisterResetValue 0x0
#define CRYPTOCFG_13_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_12_CRYPTOKEY_13_12_BitAddressOffset 0
#define CRYPTOCFG_13_12_CRYPTOKEY_13_12_RegisterSize 32





#define CRYPTOCFG_13_13 (DWC_ufshc_block_BaseAddress + 0x16b4)
#define CRYPTOCFG_13_13_RegisterSize 32
#define CRYPTOCFG_13_13_RegisterResetValue 0x0
#define CRYPTOCFG_13_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_13_CRYPTOKEY_13_13_BitAddressOffset 0
#define CRYPTOCFG_13_13_CRYPTOKEY_13_13_RegisterSize 32





#define CRYPTOCFG_13_14 (DWC_ufshc_block_BaseAddress + 0x16b8)
#define CRYPTOCFG_13_14_RegisterSize 32
#define CRYPTOCFG_13_14_RegisterResetValue 0x0
#define CRYPTOCFG_13_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_14_CRYPTOKEY_13_14_BitAddressOffset 0
#define CRYPTOCFG_13_14_CRYPTOKEY_13_14_RegisterSize 32





#define CRYPTOCFG_13_15 (DWC_ufshc_block_BaseAddress + 0x16bc)
#define CRYPTOCFG_13_15_RegisterSize 32
#define CRYPTOCFG_13_15_RegisterResetValue 0x0
#define CRYPTOCFG_13_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_15_CRYPTOKEY_13_15_BitAddressOffset 0
#define CRYPTOCFG_13_15_CRYPTOKEY_13_15_RegisterSize 32





#define CRYPTOCFG_13_16 (DWC_ufshc_block_BaseAddress + 0x16c0)
#define CRYPTOCFG_13_16_RegisterSize 32
#define CRYPTOCFG_13_16_RegisterResetValue 0x0
#define CRYPTOCFG_13_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_16_DUSIZE_13_BitAddressOffset 0
#define CRYPTOCFG_13_16_DUSIZE_13_RegisterSize 8



#define CRYPTOCFG_13_16_CAPIDX_13_BitAddressOffset 8
#define CRYPTOCFG_13_16_CAPIDX_13_RegisterSize 8



#define CRYPTOCFG_13_16_CFGE_13_BitAddressOffset 31
#define CRYPTOCFG_13_16_CFGE_13_RegisterSize 1





#define CRYPTOCFG_13_17 (DWC_ufshc_block_BaseAddress + 0x16c4)
#define CRYPTOCFG_13_17_RegisterSize 32
#define CRYPTOCFG_13_17_RegisterResetValue 0x0
#define CRYPTOCFG_13_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_17_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_17_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_18 (DWC_ufshc_block_BaseAddress + 0x16c8)
#define CRYPTOCFG_13_18_RegisterSize 32
#define CRYPTOCFG_13_18_RegisterResetValue 0x0
#define CRYPTOCFG_13_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_18_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_18_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_19 (DWC_ufshc_block_BaseAddress + 0x16cc)
#define CRYPTOCFG_13_19_RegisterSize 32
#define CRYPTOCFG_13_19_RegisterResetValue 0x0
#define CRYPTOCFG_13_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_19_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_19_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_20 (DWC_ufshc_block_BaseAddress + 0x16d0)
#define CRYPTOCFG_13_20_RegisterSize 32
#define CRYPTOCFG_13_20_RegisterResetValue 0x0
#define CRYPTOCFG_13_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_20_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_20_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_21 (DWC_ufshc_block_BaseAddress + 0x16d4)
#define CRYPTOCFG_13_21_RegisterSize 32
#define CRYPTOCFG_13_21_RegisterResetValue 0x0
#define CRYPTOCFG_13_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_21_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_21_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_22 (DWC_ufshc_block_BaseAddress + 0x16d8)
#define CRYPTOCFG_13_22_RegisterSize 32
#define CRYPTOCFG_13_22_RegisterResetValue 0x0
#define CRYPTOCFG_13_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_22_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_22_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_23 (DWC_ufshc_block_BaseAddress + 0x16dc)
#define CRYPTOCFG_13_23_RegisterSize 32
#define CRYPTOCFG_13_23_RegisterResetValue 0x0
#define CRYPTOCFG_13_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_23_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_23_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_24 (DWC_ufshc_block_BaseAddress + 0x16e0)
#define CRYPTOCFG_13_24_RegisterSize 32
#define CRYPTOCFG_13_24_RegisterResetValue 0x0
#define CRYPTOCFG_13_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_24_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_24_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_25 (DWC_ufshc_block_BaseAddress + 0x16e4)
#define CRYPTOCFG_13_25_RegisterSize 32
#define CRYPTOCFG_13_25_RegisterResetValue 0x0
#define CRYPTOCFG_13_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_25_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_25_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_26 (DWC_ufshc_block_BaseAddress + 0x16e8)
#define CRYPTOCFG_13_26_RegisterSize 32
#define CRYPTOCFG_13_26_RegisterResetValue 0x0
#define CRYPTOCFG_13_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_26_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_26_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_27 (DWC_ufshc_block_BaseAddress + 0x16ec)
#define CRYPTOCFG_13_27_RegisterSize 32
#define CRYPTOCFG_13_27_RegisterResetValue 0x0
#define CRYPTOCFG_13_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_27_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_27_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_28 (DWC_ufshc_block_BaseAddress + 0x16f0)
#define CRYPTOCFG_13_28_RegisterSize 32
#define CRYPTOCFG_13_28_RegisterResetValue 0x0
#define CRYPTOCFG_13_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_28_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_28_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_29 (DWC_ufshc_block_BaseAddress + 0x16f4)
#define CRYPTOCFG_13_29_RegisterSize 32
#define CRYPTOCFG_13_29_RegisterResetValue 0x0
#define CRYPTOCFG_13_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_29_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_29_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_30 (DWC_ufshc_block_BaseAddress + 0x16f8)
#define CRYPTOCFG_13_30_RegisterSize 32
#define CRYPTOCFG_13_30_RegisterResetValue 0x0
#define CRYPTOCFG_13_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_30_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_30_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_13_31 (DWC_ufshc_block_BaseAddress + 0x16fc)
#define CRYPTOCFG_13_31_RegisterSize 32
#define CRYPTOCFG_13_31_RegisterResetValue 0x0
#define CRYPTOCFG_13_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_13_31_RESERVED_13_BitAddressOffset 0
#define CRYPTOCFG_13_31_RESERVED_13_RegisterSize 32





#define CRYPTOCFG_14_0 (DWC_ufshc_block_BaseAddress + 0x1700)
#define CRYPTOCFG_14_0_RegisterSize 32
#define CRYPTOCFG_14_0_RegisterResetValue 0x0
#define CRYPTOCFG_14_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_0_CRYPTOKEY_14_0_BitAddressOffset 0
#define CRYPTOCFG_14_0_CRYPTOKEY_14_0_RegisterSize 32





#define CRYPTOCFG_14_1 (DWC_ufshc_block_BaseAddress + 0x1704)
#define CRYPTOCFG_14_1_RegisterSize 32
#define CRYPTOCFG_14_1_RegisterResetValue 0x0
#define CRYPTOCFG_14_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_1_CRYPTOKEY_14_1_BitAddressOffset 0
#define CRYPTOCFG_14_1_CRYPTOKEY_14_1_RegisterSize 32





#define CRYPTOCFG_14_2 (DWC_ufshc_block_BaseAddress + 0x1708)
#define CRYPTOCFG_14_2_RegisterSize 32
#define CRYPTOCFG_14_2_RegisterResetValue 0x0
#define CRYPTOCFG_14_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_2_CRYPTOKEY_14_2_BitAddressOffset 0
#define CRYPTOCFG_14_2_CRYPTOKEY_14_2_RegisterSize 32





#define CRYPTOCFG_14_3 (DWC_ufshc_block_BaseAddress + 0x170c)
#define CRYPTOCFG_14_3_RegisterSize 32
#define CRYPTOCFG_14_3_RegisterResetValue 0x0
#define CRYPTOCFG_14_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_3_CRYPTOKEY_14_3_BitAddressOffset 0
#define CRYPTOCFG_14_3_CRYPTOKEY_14_3_RegisterSize 32





#define CRYPTOCFG_14_4 (DWC_ufshc_block_BaseAddress + 0x1710)
#define CRYPTOCFG_14_4_RegisterSize 32
#define CRYPTOCFG_14_4_RegisterResetValue 0x0
#define CRYPTOCFG_14_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_4_CRYPTOKEY_14_4_BitAddressOffset 0
#define CRYPTOCFG_14_4_CRYPTOKEY_14_4_RegisterSize 32





#define CRYPTOCFG_14_5 (DWC_ufshc_block_BaseAddress + 0x1714)
#define CRYPTOCFG_14_5_RegisterSize 32
#define CRYPTOCFG_14_5_RegisterResetValue 0x0
#define CRYPTOCFG_14_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_5_CRYPTOKEY_14_5_BitAddressOffset 0
#define CRYPTOCFG_14_5_CRYPTOKEY_14_5_RegisterSize 32





#define CRYPTOCFG_14_6 (DWC_ufshc_block_BaseAddress + 0x1718)
#define CRYPTOCFG_14_6_RegisterSize 32
#define CRYPTOCFG_14_6_RegisterResetValue 0x0
#define CRYPTOCFG_14_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_6_CRYPTOKEY_14_6_BitAddressOffset 0
#define CRYPTOCFG_14_6_CRYPTOKEY_14_6_RegisterSize 32





#define CRYPTOCFG_14_7 (DWC_ufshc_block_BaseAddress + 0x171c)
#define CRYPTOCFG_14_7_RegisterSize 32
#define CRYPTOCFG_14_7_RegisterResetValue 0x0
#define CRYPTOCFG_14_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_7_CRYPTOKEY_14_7_BitAddressOffset 0
#define CRYPTOCFG_14_7_CRYPTOKEY_14_7_RegisterSize 32





#define CRYPTOCFG_14_8 (DWC_ufshc_block_BaseAddress + 0x1720)
#define CRYPTOCFG_14_8_RegisterSize 32
#define CRYPTOCFG_14_8_RegisterResetValue 0x0
#define CRYPTOCFG_14_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_8_CRYPTOKEY_14_8_BitAddressOffset 0
#define CRYPTOCFG_14_8_CRYPTOKEY_14_8_RegisterSize 32





#define CRYPTOCFG_14_9 (DWC_ufshc_block_BaseAddress + 0x1724)
#define CRYPTOCFG_14_9_RegisterSize 32
#define CRYPTOCFG_14_9_RegisterResetValue 0x0
#define CRYPTOCFG_14_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_9_CRYPTOKEY_14_9_BitAddressOffset 0
#define CRYPTOCFG_14_9_CRYPTOKEY_14_9_RegisterSize 32





#define CRYPTOCFG_14_10 (DWC_ufshc_block_BaseAddress + 0x1728)
#define CRYPTOCFG_14_10_RegisterSize 32
#define CRYPTOCFG_14_10_RegisterResetValue 0x0
#define CRYPTOCFG_14_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_10_CRYPTOKEY_14_10_BitAddressOffset 0
#define CRYPTOCFG_14_10_CRYPTOKEY_14_10_RegisterSize 32





#define CRYPTOCFG_14_11 (DWC_ufshc_block_BaseAddress + 0x172c)
#define CRYPTOCFG_14_11_RegisterSize 32
#define CRYPTOCFG_14_11_RegisterResetValue 0x0
#define CRYPTOCFG_14_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_11_CRYPTOKEY_14_11_BitAddressOffset 0
#define CRYPTOCFG_14_11_CRYPTOKEY_14_11_RegisterSize 32





#define CRYPTOCFG_14_12 (DWC_ufshc_block_BaseAddress + 0x1730)
#define CRYPTOCFG_14_12_RegisterSize 32
#define CRYPTOCFG_14_12_RegisterResetValue 0x0
#define CRYPTOCFG_14_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_12_CRYPTOKEY_14_12_BitAddressOffset 0
#define CRYPTOCFG_14_12_CRYPTOKEY_14_12_RegisterSize 32





#define CRYPTOCFG_14_13 (DWC_ufshc_block_BaseAddress + 0x1734)
#define CRYPTOCFG_14_13_RegisterSize 32
#define CRYPTOCFG_14_13_RegisterResetValue 0x0
#define CRYPTOCFG_14_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_13_CRYPTOKEY_14_13_BitAddressOffset 0
#define CRYPTOCFG_14_13_CRYPTOKEY_14_13_RegisterSize 32





#define CRYPTOCFG_14_14 (DWC_ufshc_block_BaseAddress + 0x1738)
#define CRYPTOCFG_14_14_RegisterSize 32
#define CRYPTOCFG_14_14_RegisterResetValue 0x0
#define CRYPTOCFG_14_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_14_CRYPTOKEY_14_14_BitAddressOffset 0
#define CRYPTOCFG_14_14_CRYPTOKEY_14_14_RegisterSize 32





#define CRYPTOCFG_14_15 (DWC_ufshc_block_BaseAddress + 0x173c)
#define CRYPTOCFG_14_15_RegisterSize 32
#define CRYPTOCFG_14_15_RegisterResetValue 0x0
#define CRYPTOCFG_14_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_15_CRYPTOKEY_14_15_BitAddressOffset 0
#define CRYPTOCFG_14_15_CRYPTOKEY_14_15_RegisterSize 32





#define CRYPTOCFG_14_16 (DWC_ufshc_block_BaseAddress + 0x1740)
#define CRYPTOCFG_14_16_RegisterSize 32
#define CRYPTOCFG_14_16_RegisterResetValue 0x0
#define CRYPTOCFG_14_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_16_DUSIZE_14_BitAddressOffset 0
#define CRYPTOCFG_14_16_DUSIZE_14_RegisterSize 8



#define CRYPTOCFG_14_16_CAPIDX_14_BitAddressOffset 8
#define CRYPTOCFG_14_16_CAPIDX_14_RegisterSize 8



#define CRYPTOCFG_14_16_CFGE_14_BitAddressOffset 31
#define CRYPTOCFG_14_16_CFGE_14_RegisterSize 1





#define CRYPTOCFG_14_17 (DWC_ufshc_block_BaseAddress + 0x1744)
#define CRYPTOCFG_14_17_RegisterSize 32
#define CRYPTOCFG_14_17_RegisterResetValue 0x0
#define CRYPTOCFG_14_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_17_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_17_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_18 (DWC_ufshc_block_BaseAddress + 0x1748)
#define CRYPTOCFG_14_18_RegisterSize 32
#define CRYPTOCFG_14_18_RegisterResetValue 0x0
#define CRYPTOCFG_14_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_18_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_18_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_19 (DWC_ufshc_block_BaseAddress + 0x174c)
#define CRYPTOCFG_14_19_RegisterSize 32
#define CRYPTOCFG_14_19_RegisterResetValue 0x0
#define CRYPTOCFG_14_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_19_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_19_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_20 (DWC_ufshc_block_BaseAddress + 0x1750)
#define CRYPTOCFG_14_20_RegisterSize 32
#define CRYPTOCFG_14_20_RegisterResetValue 0x0
#define CRYPTOCFG_14_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_20_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_20_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_21 (DWC_ufshc_block_BaseAddress + 0x1754)
#define CRYPTOCFG_14_21_RegisterSize 32
#define CRYPTOCFG_14_21_RegisterResetValue 0x0
#define CRYPTOCFG_14_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_21_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_21_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_22 (DWC_ufshc_block_BaseAddress + 0x1758)
#define CRYPTOCFG_14_22_RegisterSize 32
#define CRYPTOCFG_14_22_RegisterResetValue 0x0
#define CRYPTOCFG_14_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_22_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_22_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_23 (DWC_ufshc_block_BaseAddress + 0x175c)
#define CRYPTOCFG_14_23_RegisterSize 32
#define CRYPTOCFG_14_23_RegisterResetValue 0x0
#define CRYPTOCFG_14_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_23_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_23_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_24 (DWC_ufshc_block_BaseAddress + 0x1760)
#define CRYPTOCFG_14_24_RegisterSize 32
#define CRYPTOCFG_14_24_RegisterResetValue 0x0
#define CRYPTOCFG_14_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_24_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_24_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_25 (DWC_ufshc_block_BaseAddress + 0x1764)
#define CRYPTOCFG_14_25_RegisterSize 32
#define CRYPTOCFG_14_25_RegisterResetValue 0x0
#define CRYPTOCFG_14_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_25_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_25_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_26 (DWC_ufshc_block_BaseAddress + 0x1768)
#define CRYPTOCFG_14_26_RegisterSize 32
#define CRYPTOCFG_14_26_RegisterResetValue 0x0
#define CRYPTOCFG_14_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_26_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_26_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_27 (DWC_ufshc_block_BaseAddress + 0x176c)
#define CRYPTOCFG_14_27_RegisterSize 32
#define CRYPTOCFG_14_27_RegisterResetValue 0x0
#define CRYPTOCFG_14_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_27_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_27_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_28 (DWC_ufshc_block_BaseAddress + 0x1770)
#define CRYPTOCFG_14_28_RegisterSize 32
#define CRYPTOCFG_14_28_RegisterResetValue 0x0
#define CRYPTOCFG_14_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_28_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_28_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_29 (DWC_ufshc_block_BaseAddress + 0x1774)
#define CRYPTOCFG_14_29_RegisterSize 32
#define CRYPTOCFG_14_29_RegisterResetValue 0x0
#define CRYPTOCFG_14_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_29_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_29_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_30 (DWC_ufshc_block_BaseAddress + 0x1778)
#define CRYPTOCFG_14_30_RegisterSize 32
#define CRYPTOCFG_14_30_RegisterResetValue 0x0
#define CRYPTOCFG_14_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_30_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_30_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_14_31 (DWC_ufshc_block_BaseAddress + 0x177c)
#define CRYPTOCFG_14_31_RegisterSize 32
#define CRYPTOCFG_14_31_RegisterResetValue 0x0
#define CRYPTOCFG_14_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_14_31_RESERVED_14_BitAddressOffset 0
#define CRYPTOCFG_14_31_RESERVED_14_RegisterSize 32





#define CRYPTOCFG_15_0 (DWC_ufshc_block_BaseAddress + 0x1780)
#define CRYPTOCFG_15_0_RegisterSize 32
#define CRYPTOCFG_15_0_RegisterResetValue 0x0
#define CRYPTOCFG_15_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_0_CRYPTOKEY_15_0_BitAddressOffset 0
#define CRYPTOCFG_15_0_CRYPTOKEY_15_0_RegisterSize 32





#define CRYPTOCFG_15_1 (DWC_ufshc_block_BaseAddress + 0x1784)
#define CRYPTOCFG_15_1_RegisterSize 32
#define CRYPTOCFG_15_1_RegisterResetValue 0x0
#define CRYPTOCFG_15_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_1_CRYPTOKEY_15_1_BitAddressOffset 0
#define CRYPTOCFG_15_1_CRYPTOKEY_15_1_RegisterSize 32





#define CRYPTOCFG_15_2 (DWC_ufshc_block_BaseAddress + 0x1788)
#define CRYPTOCFG_15_2_RegisterSize 32
#define CRYPTOCFG_15_2_RegisterResetValue 0x0
#define CRYPTOCFG_15_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_2_CRYPTOKEY_15_2_BitAddressOffset 0
#define CRYPTOCFG_15_2_CRYPTOKEY_15_2_RegisterSize 32





#define CRYPTOCFG_15_3 (DWC_ufshc_block_BaseAddress + 0x178c)
#define CRYPTOCFG_15_3_RegisterSize 32
#define CRYPTOCFG_15_3_RegisterResetValue 0x0
#define CRYPTOCFG_15_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_3_CRYPTOKEY_15_3_BitAddressOffset 0
#define CRYPTOCFG_15_3_CRYPTOKEY_15_3_RegisterSize 32





#define CRYPTOCFG_15_4 (DWC_ufshc_block_BaseAddress + 0x1790)
#define CRYPTOCFG_15_4_RegisterSize 32
#define CRYPTOCFG_15_4_RegisterResetValue 0x0
#define CRYPTOCFG_15_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_4_CRYPTOKEY_15_4_BitAddressOffset 0
#define CRYPTOCFG_15_4_CRYPTOKEY_15_4_RegisterSize 32





#define CRYPTOCFG_15_5 (DWC_ufshc_block_BaseAddress + 0x1794)
#define CRYPTOCFG_15_5_RegisterSize 32
#define CRYPTOCFG_15_5_RegisterResetValue 0x0
#define CRYPTOCFG_15_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_5_CRYPTOKEY_15_5_BitAddressOffset 0
#define CRYPTOCFG_15_5_CRYPTOKEY_15_5_RegisterSize 32





#define CRYPTOCFG_15_6 (DWC_ufshc_block_BaseAddress + 0x1798)
#define CRYPTOCFG_15_6_RegisterSize 32
#define CRYPTOCFG_15_6_RegisterResetValue 0x0
#define CRYPTOCFG_15_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_6_CRYPTOKEY_15_6_BitAddressOffset 0
#define CRYPTOCFG_15_6_CRYPTOKEY_15_6_RegisterSize 32





#define CRYPTOCFG_15_7 (DWC_ufshc_block_BaseAddress + 0x179c)
#define CRYPTOCFG_15_7_RegisterSize 32
#define CRYPTOCFG_15_7_RegisterResetValue 0x0
#define CRYPTOCFG_15_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_7_CRYPTOKEY_15_7_BitAddressOffset 0
#define CRYPTOCFG_15_7_CRYPTOKEY_15_7_RegisterSize 32





#define CRYPTOCFG_15_8 (DWC_ufshc_block_BaseAddress + 0x17a0)
#define CRYPTOCFG_15_8_RegisterSize 32
#define CRYPTOCFG_15_8_RegisterResetValue 0x0
#define CRYPTOCFG_15_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_8_CRYPTOKEY_15_8_BitAddressOffset 0
#define CRYPTOCFG_15_8_CRYPTOKEY_15_8_RegisterSize 32





#define CRYPTOCFG_15_9 (DWC_ufshc_block_BaseAddress + 0x17a4)
#define CRYPTOCFG_15_9_RegisterSize 32
#define CRYPTOCFG_15_9_RegisterResetValue 0x0
#define CRYPTOCFG_15_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_9_CRYPTOKEY_15_9_BitAddressOffset 0
#define CRYPTOCFG_15_9_CRYPTOKEY_15_9_RegisterSize 32





#define CRYPTOCFG_15_10 (DWC_ufshc_block_BaseAddress + 0x17a8)
#define CRYPTOCFG_15_10_RegisterSize 32
#define CRYPTOCFG_15_10_RegisterResetValue 0x0
#define CRYPTOCFG_15_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_10_CRYPTOKEY_15_10_BitAddressOffset 0
#define CRYPTOCFG_15_10_CRYPTOKEY_15_10_RegisterSize 32





#define CRYPTOCFG_15_11 (DWC_ufshc_block_BaseAddress + 0x17ac)
#define CRYPTOCFG_15_11_RegisterSize 32
#define CRYPTOCFG_15_11_RegisterResetValue 0x0
#define CRYPTOCFG_15_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_11_CRYPTOKEY_15_11_BitAddressOffset 0
#define CRYPTOCFG_15_11_CRYPTOKEY_15_11_RegisterSize 32





#define CRYPTOCFG_15_12 (DWC_ufshc_block_BaseAddress + 0x17b0)
#define CRYPTOCFG_15_12_RegisterSize 32
#define CRYPTOCFG_15_12_RegisterResetValue 0x0
#define CRYPTOCFG_15_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_12_CRYPTOKEY_15_12_BitAddressOffset 0
#define CRYPTOCFG_15_12_CRYPTOKEY_15_12_RegisterSize 32





#define CRYPTOCFG_15_13 (DWC_ufshc_block_BaseAddress + 0x17b4)
#define CRYPTOCFG_15_13_RegisterSize 32
#define CRYPTOCFG_15_13_RegisterResetValue 0x0
#define CRYPTOCFG_15_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_13_CRYPTOKEY_15_13_BitAddressOffset 0
#define CRYPTOCFG_15_13_CRYPTOKEY_15_13_RegisterSize 32





#define CRYPTOCFG_15_14 (DWC_ufshc_block_BaseAddress + 0x17b8)
#define CRYPTOCFG_15_14_RegisterSize 32
#define CRYPTOCFG_15_14_RegisterResetValue 0x0
#define CRYPTOCFG_15_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_14_CRYPTOKEY_15_14_BitAddressOffset 0
#define CRYPTOCFG_15_14_CRYPTOKEY_15_14_RegisterSize 32





#define CRYPTOCFG_15_15 (DWC_ufshc_block_BaseAddress + 0x17bc)
#define CRYPTOCFG_15_15_RegisterSize 32
#define CRYPTOCFG_15_15_RegisterResetValue 0x0
#define CRYPTOCFG_15_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_15_CRYPTOKEY_15_15_BitAddressOffset 0
#define CRYPTOCFG_15_15_CRYPTOKEY_15_15_RegisterSize 32





#define CRYPTOCFG_15_16 (DWC_ufshc_block_BaseAddress + 0x17c0)
#define CRYPTOCFG_15_16_RegisterSize 32
#define CRYPTOCFG_15_16_RegisterResetValue 0x0
#define CRYPTOCFG_15_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_16_DUSIZE_15_BitAddressOffset 0
#define CRYPTOCFG_15_16_DUSIZE_15_RegisterSize 8



#define CRYPTOCFG_15_16_CAPIDX_15_BitAddressOffset 8
#define CRYPTOCFG_15_16_CAPIDX_15_RegisterSize 8



#define CRYPTOCFG_15_16_CFGE_15_BitAddressOffset 31
#define CRYPTOCFG_15_16_CFGE_15_RegisterSize 1





#define CRYPTOCFG_15_17 (DWC_ufshc_block_BaseAddress + 0x17c4)
#define CRYPTOCFG_15_17_RegisterSize 32
#define CRYPTOCFG_15_17_RegisterResetValue 0x0
#define CRYPTOCFG_15_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_17_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_17_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_18 (DWC_ufshc_block_BaseAddress + 0x17c8)
#define CRYPTOCFG_15_18_RegisterSize 32
#define CRYPTOCFG_15_18_RegisterResetValue 0x0
#define CRYPTOCFG_15_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_18_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_18_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_19 (DWC_ufshc_block_BaseAddress + 0x17cc)
#define CRYPTOCFG_15_19_RegisterSize 32
#define CRYPTOCFG_15_19_RegisterResetValue 0x0
#define CRYPTOCFG_15_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_19_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_19_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_20 (DWC_ufshc_block_BaseAddress + 0x17d0)
#define CRYPTOCFG_15_20_RegisterSize 32
#define CRYPTOCFG_15_20_RegisterResetValue 0x0
#define CRYPTOCFG_15_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_20_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_20_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_21 (DWC_ufshc_block_BaseAddress + 0x17d4)
#define CRYPTOCFG_15_21_RegisterSize 32
#define CRYPTOCFG_15_21_RegisterResetValue 0x0
#define CRYPTOCFG_15_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_21_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_21_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_22 (DWC_ufshc_block_BaseAddress + 0x17d8)
#define CRYPTOCFG_15_22_RegisterSize 32
#define CRYPTOCFG_15_22_RegisterResetValue 0x0
#define CRYPTOCFG_15_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_22_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_22_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_23 (DWC_ufshc_block_BaseAddress + 0x17dc)
#define CRYPTOCFG_15_23_RegisterSize 32
#define CRYPTOCFG_15_23_RegisterResetValue 0x0
#define CRYPTOCFG_15_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_23_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_23_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_24 (DWC_ufshc_block_BaseAddress + 0x17e0)
#define CRYPTOCFG_15_24_RegisterSize 32
#define CRYPTOCFG_15_24_RegisterResetValue 0x0
#define CRYPTOCFG_15_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_24_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_24_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_25 (DWC_ufshc_block_BaseAddress + 0x17e4)
#define CRYPTOCFG_15_25_RegisterSize 32
#define CRYPTOCFG_15_25_RegisterResetValue 0x0
#define CRYPTOCFG_15_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_25_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_25_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_26 (DWC_ufshc_block_BaseAddress + 0x17e8)
#define CRYPTOCFG_15_26_RegisterSize 32
#define CRYPTOCFG_15_26_RegisterResetValue 0x0
#define CRYPTOCFG_15_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_26_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_26_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_27 (DWC_ufshc_block_BaseAddress + 0x17ec)
#define CRYPTOCFG_15_27_RegisterSize 32
#define CRYPTOCFG_15_27_RegisterResetValue 0x0
#define CRYPTOCFG_15_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_27_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_27_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_28 (DWC_ufshc_block_BaseAddress + 0x17f0)
#define CRYPTOCFG_15_28_RegisterSize 32
#define CRYPTOCFG_15_28_RegisterResetValue 0x0
#define CRYPTOCFG_15_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_28_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_28_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_29 (DWC_ufshc_block_BaseAddress + 0x17f4)
#define CRYPTOCFG_15_29_RegisterSize 32
#define CRYPTOCFG_15_29_RegisterResetValue 0x0
#define CRYPTOCFG_15_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_29_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_29_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_30 (DWC_ufshc_block_BaseAddress + 0x17f8)
#define CRYPTOCFG_15_30_RegisterSize 32
#define CRYPTOCFG_15_30_RegisterResetValue 0x0
#define CRYPTOCFG_15_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_30_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_30_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_15_31 (DWC_ufshc_block_BaseAddress + 0x17fc)
#define CRYPTOCFG_15_31_RegisterSize 32
#define CRYPTOCFG_15_31_RegisterResetValue 0x0
#define CRYPTOCFG_15_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_15_31_RESERVED_15_BitAddressOffset 0
#define CRYPTOCFG_15_31_RESERVED_15_RegisterSize 32





#define CRYPTOCFG_16_0 (DWC_ufshc_block_BaseAddress + 0x1800)
#define CRYPTOCFG_16_0_RegisterSize 32
#define CRYPTOCFG_16_0_RegisterResetValue 0x0
#define CRYPTOCFG_16_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_0_CRYPTOKEY_16_0_BitAddressOffset 0
#define CRYPTOCFG_16_0_CRYPTOKEY_16_0_RegisterSize 32





#define CRYPTOCFG_16_1 (DWC_ufshc_block_BaseAddress + 0x1804)
#define CRYPTOCFG_16_1_RegisterSize 32
#define CRYPTOCFG_16_1_RegisterResetValue 0x0
#define CRYPTOCFG_16_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_1_CRYPTOKEY_16_1_BitAddressOffset 0
#define CRYPTOCFG_16_1_CRYPTOKEY_16_1_RegisterSize 32





#define CRYPTOCFG_16_2 (DWC_ufshc_block_BaseAddress + 0x1808)
#define CRYPTOCFG_16_2_RegisterSize 32
#define CRYPTOCFG_16_2_RegisterResetValue 0x0
#define CRYPTOCFG_16_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_2_CRYPTOKEY_16_2_BitAddressOffset 0
#define CRYPTOCFG_16_2_CRYPTOKEY_16_2_RegisterSize 32





#define CRYPTOCFG_16_3 (DWC_ufshc_block_BaseAddress + 0x180c)
#define CRYPTOCFG_16_3_RegisterSize 32
#define CRYPTOCFG_16_3_RegisterResetValue 0x0
#define CRYPTOCFG_16_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_3_CRYPTOKEY_16_3_BitAddressOffset 0
#define CRYPTOCFG_16_3_CRYPTOKEY_16_3_RegisterSize 32





#define CRYPTOCFG_16_4 (DWC_ufshc_block_BaseAddress + 0x1810)
#define CRYPTOCFG_16_4_RegisterSize 32
#define CRYPTOCFG_16_4_RegisterResetValue 0x0
#define CRYPTOCFG_16_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_4_CRYPTOKEY_16_4_BitAddressOffset 0
#define CRYPTOCFG_16_4_CRYPTOKEY_16_4_RegisterSize 32





#define CRYPTOCFG_16_5 (DWC_ufshc_block_BaseAddress + 0x1814)
#define CRYPTOCFG_16_5_RegisterSize 32
#define CRYPTOCFG_16_5_RegisterResetValue 0x0
#define CRYPTOCFG_16_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_5_CRYPTOKEY_16_5_BitAddressOffset 0
#define CRYPTOCFG_16_5_CRYPTOKEY_16_5_RegisterSize 32





#define CRYPTOCFG_16_6 (DWC_ufshc_block_BaseAddress + 0x1818)
#define CRYPTOCFG_16_6_RegisterSize 32
#define CRYPTOCFG_16_6_RegisterResetValue 0x0
#define CRYPTOCFG_16_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_6_CRYPTOKEY_16_6_BitAddressOffset 0
#define CRYPTOCFG_16_6_CRYPTOKEY_16_6_RegisterSize 32





#define CRYPTOCFG_16_7 (DWC_ufshc_block_BaseAddress + 0x181c)
#define CRYPTOCFG_16_7_RegisterSize 32
#define CRYPTOCFG_16_7_RegisterResetValue 0x0
#define CRYPTOCFG_16_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_7_CRYPTOKEY_16_7_BitAddressOffset 0
#define CRYPTOCFG_16_7_CRYPTOKEY_16_7_RegisterSize 32





#define CRYPTOCFG_16_8 (DWC_ufshc_block_BaseAddress + 0x1820)
#define CRYPTOCFG_16_8_RegisterSize 32
#define CRYPTOCFG_16_8_RegisterResetValue 0x0
#define CRYPTOCFG_16_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_8_CRYPTOKEY_16_8_BitAddressOffset 0
#define CRYPTOCFG_16_8_CRYPTOKEY_16_8_RegisterSize 32





#define CRYPTOCFG_16_9 (DWC_ufshc_block_BaseAddress + 0x1824)
#define CRYPTOCFG_16_9_RegisterSize 32
#define CRYPTOCFG_16_9_RegisterResetValue 0x0
#define CRYPTOCFG_16_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_9_CRYPTOKEY_16_9_BitAddressOffset 0
#define CRYPTOCFG_16_9_CRYPTOKEY_16_9_RegisterSize 32





#define CRYPTOCFG_16_10 (DWC_ufshc_block_BaseAddress + 0x1828)
#define CRYPTOCFG_16_10_RegisterSize 32
#define CRYPTOCFG_16_10_RegisterResetValue 0x0
#define CRYPTOCFG_16_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_10_CRYPTOKEY_16_10_BitAddressOffset 0
#define CRYPTOCFG_16_10_CRYPTOKEY_16_10_RegisterSize 32





#define CRYPTOCFG_16_11 (DWC_ufshc_block_BaseAddress + 0x182c)
#define CRYPTOCFG_16_11_RegisterSize 32
#define CRYPTOCFG_16_11_RegisterResetValue 0x0
#define CRYPTOCFG_16_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_11_CRYPTOKEY_16_11_BitAddressOffset 0
#define CRYPTOCFG_16_11_CRYPTOKEY_16_11_RegisterSize 32





#define CRYPTOCFG_16_12 (DWC_ufshc_block_BaseAddress + 0x1830)
#define CRYPTOCFG_16_12_RegisterSize 32
#define CRYPTOCFG_16_12_RegisterResetValue 0x0
#define CRYPTOCFG_16_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_12_CRYPTOKEY_16_12_BitAddressOffset 0
#define CRYPTOCFG_16_12_CRYPTOKEY_16_12_RegisterSize 32





#define CRYPTOCFG_16_13 (DWC_ufshc_block_BaseAddress + 0x1834)
#define CRYPTOCFG_16_13_RegisterSize 32
#define CRYPTOCFG_16_13_RegisterResetValue 0x0
#define CRYPTOCFG_16_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_13_CRYPTOKEY_16_13_BitAddressOffset 0
#define CRYPTOCFG_16_13_CRYPTOKEY_16_13_RegisterSize 32





#define CRYPTOCFG_16_14 (DWC_ufshc_block_BaseAddress + 0x1838)
#define CRYPTOCFG_16_14_RegisterSize 32
#define CRYPTOCFG_16_14_RegisterResetValue 0x0
#define CRYPTOCFG_16_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_14_CRYPTOKEY_16_14_BitAddressOffset 0
#define CRYPTOCFG_16_14_CRYPTOKEY_16_14_RegisterSize 32





#define CRYPTOCFG_16_15 (DWC_ufshc_block_BaseAddress + 0x183c)
#define CRYPTOCFG_16_15_RegisterSize 32
#define CRYPTOCFG_16_15_RegisterResetValue 0x0
#define CRYPTOCFG_16_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_15_CRYPTOKEY_16_15_BitAddressOffset 0
#define CRYPTOCFG_16_15_CRYPTOKEY_16_15_RegisterSize 32





#define CRYPTOCFG_16_16 (DWC_ufshc_block_BaseAddress + 0x1840)
#define CRYPTOCFG_16_16_RegisterSize 32
#define CRYPTOCFG_16_16_RegisterResetValue 0x0
#define CRYPTOCFG_16_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_16_DUSIZE_16_BitAddressOffset 0
#define CRYPTOCFG_16_16_DUSIZE_16_RegisterSize 8



#define CRYPTOCFG_16_16_CAPIDX_16_BitAddressOffset 8
#define CRYPTOCFG_16_16_CAPIDX_16_RegisterSize 8



#define CRYPTOCFG_16_16_CFGE_16_BitAddressOffset 31
#define CRYPTOCFG_16_16_CFGE_16_RegisterSize 1





#define CRYPTOCFG_16_17 (DWC_ufshc_block_BaseAddress + 0x1844)
#define CRYPTOCFG_16_17_RegisterSize 32
#define CRYPTOCFG_16_17_RegisterResetValue 0x0
#define CRYPTOCFG_16_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_17_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_17_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_18 (DWC_ufshc_block_BaseAddress + 0x1848)
#define CRYPTOCFG_16_18_RegisterSize 32
#define CRYPTOCFG_16_18_RegisterResetValue 0x0
#define CRYPTOCFG_16_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_18_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_18_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_19 (DWC_ufshc_block_BaseAddress + 0x184c)
#define CRYPTOCFG_16_19_RegisterSize 32
#define CRYPTOCFG_16_19_RegisterResetValue 0x0
#define CRYPTOCFG_16_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_19_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_19_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_20 (DWC_ufshc_block_BaseAddress + 0x1850)
#define CRYPTOCFG_16_20_RegisterSize 32
#define CRYPTOCFG_16_20_RegisterResetValue 0x0
#define CRYPTOCFG_16_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_20_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_20_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_21 (DWC_ufshc_block_BaseAddress + 0x1854)
#define CRYPTOCFG_16_21_RegisterSize 32
#define CRYPTOCFG_16_21_RegisterResetValue 0x0
#define CRYPTOCFG_16_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_21_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_21_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_22 (DWC_ufshc_block_BaseAddress + 0x1858)
#define CRYPTOCFG_16_22_RegisterSize 32
#define CRYPTOCFG_16_22_RegisterResetValue 0x0
#define CRYPTOCFG_16_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_22_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_22_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_23 (DWC_ufshc_block_BaseAddress + 0x185c)
#define CRYPTOCFG_16_23_RegisterSize 32
#define CRYPTOCFG_16_23_RegisterResetValue 0x0
#define CRYPTOCFG_16_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_23_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_23_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_24 (DWC_ufshc_block_BaseAddress + 0x1860)
#define CRYPTOCFG_16_24_RegisterSize 32
#define CRYPTOCFG_16_24_RegisterResetValue 0x0
#define CRYPTOCFG_16_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_24_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_24_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_25 (DWC_ufshc_block_BaseAddress + 0x1864)
#define CRYPTOCFG_16_25_RegisterSize 32
#define CRYPTOCFG_16_25_RegisterResetValue 0x0
#define CRYPTOCFG_16_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_25_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_25_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_26 (DWC_ufshc_block_BaseAddress + 0x1868)
#define CRYPTOCFG_16_26_RegisterSize 32
#define CRYPTOCFG_16_26_RegisterResetValue 0x0
#define CRYPTOCFG_16_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_26_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_26_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_27 (DWC_ufshc_block_BaseAddress + 0x186c)
#define CRYPTOCFG_16_27_RegisterSize 32
#define CRYPTOCFG_16_27_RegisterResetValue 0x0
#define CRYPTOCFG_16_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_27_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_27_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_28 (DWC_ufshc_block_BaseAddress + 0x1870)
#define CRYPTOCFG_16_28_RegisterSize 32
#define CRYPTOCFG_16_28_RegisterResetValue 0x0
#define CRYPTOCFG_16_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_28_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_28_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_29 (DWC_ufshc_block_BaseAddress + 0x1874)
#define CRYPTOCFG_16_29_RegisterSize 32
#define CRYPTOCFG_16_29_RegisterResetValue 0x0
#define CRYPTOCFG_16_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_29_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_29_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_30 (DWC_ufshc_block_BaseAddress + 0x1878)
#define CRYPTOCFG_16_30_RegisterSize 32
#define CRYPTOCFG_16_30_RegisterResetValue 0x0
#define CRYPTOCFG_16_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_30_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_30_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_16_31 (DWC_ufshc_block_BaseAddress + 0x187c)
#define CRYPTOCFG_16_31_RegisterSize 32
#define CRYPTOCFG_16_31_RegisterResetValue 0x0
#define CRYPTOCFG_16_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_16_31_RESERVED_16_BitAddressOffset 0
#define CRYPTOCFG_16_31_RESERVED_16_RegisterSize 32





#define CRYPTOCFG_17_0 (DWC_ufshc_block_BaseAddress + 0x1880)
#define CRYPTOCFG_17_0_RegisterSize 32
#define CRYPTOCFG_17_0_RegisterResetValue 0x0
#define CRYPTOCFG_17_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_0_CRYPTOKEY_17_0_BitAddressOffset 0
#define CRYPTOCFG_17_0_CRYPTOKEY_17_0_RegisterSize 32





#define CRYPTOCFG_17_1 (DWC_ufshc_block_BaseAddress + 0x1884)
#define CRYPTOCFG_17_1_RegisterSize 32
#define CRYPTOCFG_17_1_RegisterResetValue 0x0
#define CRYPTOCFG_17_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_1_CRYPTOKEY_17_1_BitAddressOffset 0
#define CRYPTOCFG_17_1_CRYPTOKEY_17_1_RegisterSize 32





#define CRYPTOCFG_17_2 (DWC_ufshc_block_BaseAddress + 0x1888)
#define CRYPTOCFG_17_2_RegisterSize 32
#define CRYPTOCFG_17_2_RegisterResetValue 0x0
#define CRYPTOCFG_17_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_2_CRYPTOKEY_17_2_BitAddressOffset 0
#define CRYPTOCFG_17_2_CRYPTOKEY_17_2_RegisterSize 32





#define CRYPTOCFG_17_3 (DWC_ufshc_block_BaseAddress + 0x188c)
#define CRYPTOCFG_17_3_RegisterSize 32
#define CRYPTOCFG_17_3_RegisterResetValue 0x0
#define CRYPTOCFG_17_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_3_CRYPTOKEY_17_3_BitAddressOffset 0
#define CRYPTOCFG_17_3_CRYPTOKEY_17_3_RegisterSize 32





#define CRYPTOCFG_17_4 (DWC_ufshc_block_BaseAddress + 0x1890)
#define CRYPTOCFG_17_4_RegisterSize 32
#define CRYPTOCFG_17_4_RegisterResetValue 0x0
#define CRYPTOCFG_17_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_4_CRYPTOKEY_17_4_BitAddressOffset 0
#define CRYPTOCFG_17_4_CRYPTOKEY_17_4_RegisterSize 32





#define CRYPTOCFG_17_5 (DWC_ufshc_block_BaseAddress + 0x1894)
#define CRYPTOCFG_17_5_RegisterSize 32
#define CRYPTOCFG_17_5_RegisterResetValue 0x0
#define CRYPTOCFG_17_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_5_CRYPTOKEY_17_5_BitAddressOffset 0
#define CRYPTOCFG_17_5_CRYPTOKEY_17_5_RegisterSize 32





#define CRYPTOCFG_17_6 (DWC_ufshc_block_BaseAddress + 0x1898)
#define CRYPTOCFG_17_6_RegisterSize 32
#define CRYPTOCFG_17_6_RegisterResetValue 0x0
#define CRYPTOCFG_17_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_6_CRYPTOKEY_17_6_BitAddressOffset 0
#define CRYPTOCFG_17_6_CRYPTOKEY_17_6_RegisterSize 32





#define CRYPTOCFG_17_7 (DWC_ufshc_block_BaseAddress + 0x189c)
#define CRYPTOCFG_17_7_RegisterSize 32
#define CRYPTOCFG_17_7_RegisterResetValue 0x0
#define CRYPTOCFG_17_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_7_CRYPTOKEY_17_7_BitAddressOffset 0
#define CRYPTOCFG_17_7_CRYPTOKEY_17_7_RegisterSize 32





#define CRYPTOCFG_17_8 (DWC_ufshc_block_BaseAddress + 0x18a0)
#define CRYPTOCFG_17_8_RegisterSize 32
#define CRYPTOCFG_17_8_RegisterResetValue 0x0
#define CRYPTOCFG_17_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_8_CRYPTOKEY_17_8_BitAddressOffset 0
#define CRYPTOCFG_17_8_CRYPTOKEY_17_8_RegisterSize 32





#define CRYPTOCFG_17_9 (DWC_ufshc_block_BaseAddress + 0x18a4)
#define CRYPTOCFG_17_9_RegisterSize 32
#define CRYPTOCFG_17_9_RegisterResetValue 0x0
#define CRYPTOCFG_17_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_9_CRYPTOKEY_17_9_BitAddressOffset 0
#define CRYPTOCFG_17_9_CRYPTOKEY_17_9_RegisterSize 32





#define CRYPTOCFG_17_10 (DWC_ufshc_block_BaseAddress + 0x18a8)
#define CRYPTOCFG_17_10_RegisterSize 32
#define CRYPTOCFG_17_10_RegisterResetValue 0x0
#define CRYPTOCFG_17_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_10_CRYPTOKEY_17_10_BitAddressOffset 0
#define CRYPTOCFG_17_10_CRYPTOKEY_17_10_RegisterSize 32





#define CRYPTOCFG_17_11 (DWC_ufshc_block_BaseAddress + 0x18ac)
#define CRYPTOCFG_17_11_RegisterSize 32
#define CRYPTOCFG_17_11_RegisterResetValue 0x0
#define CRYPTOCFG_17_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_11_CRYPTOKEY_17_11_BitAddressOffset 0
#define CRYPTOCFG_17_11_CRYPTOKEY_17_11_RegisterSize 32





#define CRYPTOCFG_17_12 (DWC_ufshc_block_BaseAddress + 0x18b0)
#define CRYPTOCFG_17_12_RegisterSize 32
#define CRYPTOCFG_17_12_RegisterResetValue 0x0
#define CRYPTOCFG_17_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_12_CRYPTOKEY_17_12_BitAddressOffset 0
#define CRYPTOCFG_17_12_CRYPTOKEY_17_12_RegisterSize 32





#define CRYPTOCFG_17_13 (DWC_ufshc_block_BaseAddress + 0x18b4)
#define CRYPTOCFG_17_13_RegisterSize 32
#define CRYPTOCFG_17_13_RegisterResetValue 0x0
#define CRYPTOCFG_17_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_13_CRYPTOKEY_17_13_BitAddressOffset 0
#define CRYPTOCFG_17_13_CRYPTOKEY_17_13_RegisterSize 32





#define CRYPTOCFG_17_14 (DWC_ufshc_block_BaseAddress + 0x18b8)
#define CRYPTOCFG_17_14_RegisterSize 32
#define CRYPTOCFG_17_14_RegisterResetValue 0x0
#define CRYPTOCFG_17_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_14_CRYPTOKEY_17_14_BitAddressOffset 0
#define CRYPTOCFG_17_14_CRYPTOKEY_17_14_RegisterSize 32





#define CRYPTOCFG_17_15 (DWC_ufshc_block_BaseAddress + 0x18bc)
#define CRYPTOCFG_17_15_RegisterSize 32
#define CRYPTOCFG_17_15_RegisterResetValue 0x0
#define CRYPTOCFG_17_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_15_CRYPTOKEY_17_15_BitAddressOffset 0
#define CRYPTOCFG_17_15_CRYPTOKEY_17_15_RegisterSize 32





#define CRYPTOCFG_17_16 (DWC_ufshc_block_BaseAddress + 0x18c0)
#define CRYPTOCFG_17_16_RegisterSize 32
#define CRYPTOCFG_17_16_RegisterResetValue 0x0
#define CRYPTOCFG_17_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_16_DUSIZE_17_BitAddressOffset 0
#define CRYPTOCFG_17_16_DUSIZE_17_RegisterSize 8



#define CRYPTOCFG_17_16_CAPIDX_17_BitAddressOffset 8
#define CRYPTOCFG_17_16_CAPIDX_17_RegisterSize 8



#define CRYPTOCFG_17_16_CFGE_17_BitAddressOffset 31
#define CRYPTOCFG_17_16_CFGE_17_RegisterSize 1





#define CRYPTOCFG_17_17 (DWC_ufshc_block_BaseAddress + 0x18c4)
#define CRYPTOCFG_17_17_RegisterSize 32
#define CRYPTOCFG_17_17_RegisterResetValue 0x0
#define CRYPTOCFG_17_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_17_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_17_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_18 (DWC_ufshc_block_BaseAddress + 0x18c8)
#define CRYPTOCFG_17_18_RegisterSize 32
#define CRYPTOCFG_17_18_RegisterResetValue 0x0
#define CRYPTOCFG_17_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_18_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_18_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_19 (DWC_ufshc_block_BaseAddress + 0x18cc)
#define CRYPTOCFG_17_19_RegisterSize 32
#define CRYPTOCFG_17_19_RegisterResetValue 0x0
#define CRYPTOCFG_17_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_19_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_19_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_20 (DWC_ufshc_block_BaseAddress + 0x18d0)
#define CRYPTOCFG_17_20_RegisterSize 32
#define CRYPTOCFG_17_20_RegisterResetValue 0x0
#define CRYPTOCFG_17_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_20_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_20_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_21 (DWC_ufshc_block_BaseAddress + 0x18d4)
#define CRYPTOCFG_17_21_RegisterSize 32
#define CRYPTOCFG_17_21_RegisterResetValue 0x0
#define CRYPTOCFG_17_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_21_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_21_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_22 (DWC_ufshc_block_BaseAddress + 0x18d8)
#define CRYPTOCFG_17_22_RegisterSize 32
#define CRYPTOCFG_17_22_RegisterResetValue 0x0
#define CRYPTOCFG_17_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_22_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_22_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_23 (DWC_ufshc_block_BaseAddress + 0x18dc)
#define CRYPTOCFG_17_23_RegisterSize 32
#define CRYPTOCFG_17_23_RegisterResetValue 0x0
#define CRYPTOCFG_17_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_23_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_23_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_24 (DWC_ufshc_block_BaseAddress + 0x18e0)
#define CRYPTOCFG_17_24_RegisterSize 32
#define CRYPTOCFG_17_24_RegisterResetValue 0x0
#define CRYPTOCFG_17_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_24_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_24_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_25 (DWC_ufshc_block_BaseAddress + 0x18e4)
#define CRYPTOCFG_17_25_RegisterSize 32
#define CRYPTOCFG_17_25_RegisterResetValue 0x0
#define CRYPTOCFG_17_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_25_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_25_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_26 (DWC_ufshc_block_BaseAddress + 0x18e8)
#define CRYPTOCFG_17_26_RegisterSize 32
#define CRYPTOCFG_17_26_RegisterResetValue 0x0
#define CRYPTOCFG_17_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_26_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_26_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_27 (DWC_ufshc_block_BaseAddress + 0x18ec)
#define CRYPTOCFG_17_27_RegisterSize 32
#define CRYPTOCFG_17_27_RegisterResetValue 0x0
#define CRYPTOCFG_17_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_27_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_27_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_28 (DWC_ufshc_block_BaseAddress + 0x18f0)
#define CRYPTOCFG_17_28_RegisterSize 32
#define CRYPTOCFG_17_28_RegisterResetValue 0x0
#define CRYPTOCFG_17_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_28_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_28_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_29 (DWC_ufshc_block_BaseAddress + 0x18f4)
#define CRYPTOCFG_17_29_RegisterSize 32
#define CRYPTOCFG_17_29_RegisterResetValue 0x0
#define CRYPTOCFG_17_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_29_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_29_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_30 (DWC_ufshc_block_BaseAddress + 0x18f8)
#define CRYPTOCFG_17_30_RegisterSize 32
#define CRYPTOCFG_17_30_RegisterResetValue 0x0
#define CRYPTOCFG_17_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_30_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_30_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_17_31 (DWC_ufshc_block_BaseAddress + 0x18fc)
#define CRYPTOCFG_17_31_RegisterSize 32
#define CRYPTOCFG_17_31_RegisterResetValue 0x0
#define CRYPTOCFG_17_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_17_31_RESERVED_17_BitAddressOffset 0
#define CRYPTOCFG_17_31_RESERVED_17_RegisterSize 32





#define CRYPTOCFG_18_0 (DWC_ufshc_block_BaseAddress + 0x1900)
#define CRYPTOCFG_18_0_RegisterSize 32
#define CRYPTOCFG_18_0_RegisterResetValue 0x0
#define CRYPTOCFG_18_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_0_CRYPTOKEY_18_0_BitAddressOffset 0
#define CRYPTOCFG_18_0_CRYPTOKEY_18_0_RegisterSize 32





#define CRYPTOCFG_18_1 (DWC_ufshc_block_BaseAddress + 0x1904)
#define CRYPTOCFG_18_1_RegisterSize 32
#define CRYPTOCFG_18_1_RegisterResetValue 0x0
#define CRYPTOCFG_18_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_1_CRYPTOKEY_18_1_BitAddressOffset 0
#define CRYPTOCFG_18_1_CRYPTOKEY_18_1_RegisterSize 32





#define CRYPTOCFG_18_2 (DWC_ufshc_block_BaseAddress + 0x1908)
#define CRYPTOCFG_18_2_RegisterSize 32
#define CRYPTOCFG_18_2_RegisterResetValue 0x0
#define CRYPTOCFG_18_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_2_CRYPTOKEY_18_2_BitAddressOffset 0
#define CRYPTOCFG_18_2_CRYPTOKEY_18_2_RegisterSize 32





#define CRYPTOCFG_18_3 (DWC_ufshc_block_BaseAddress + 0x190c)
#define CRYPTOCFG_18_3_RegisterSize 32
#define CRYPTOCFG_18_3_RegisterResetValue 0x0
#define CRYPTOCFG_18_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_3_CRYPTOKEY_18_3_BitAddressOffset 0
#define CRYPTOCFG_18_3_CRYPTOKEY_18_3_RegisterSize 32





#define CRYPTOCFG_18_4 (DWC_ufshc_block_BaseAddress + 0x1910)
#define CRYPTOCFG_18_4_RegisterSize 32
#define CRYPTOCFG_18_4_RegisterResetValue 0x0
#define CRYPTOCFG_18_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_4_CRYPTOKEY_18_4_BitAddressOffset 0
#define CRYPTOCFG_18_4_CRYPTOKEY_18_4_RegisterSize 32





#define CRYPTOCFG_18_5 (DWC_ufshc_block_BaseAddress + 0x1914)
#define CRYPTOCFG_18_5_RegisterSize 32
#define CRYPTOCFG_18_5_RegisterResetValue 0x0
#define CRYPTOCFG_18_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_5_CRYPTOKEY_18_5_BitAddressOffset 0
#define CRYPTOCFG_18_5_CRYPTOKEY_18_5_RegisterSize 32





#define CRYPTOCFG_18_6 (DWC_ufshc_block_BaseAddress + 0x1918)
#define CRYPTOCFG_18_6_RegisterSize 32
#define CRYPTOCFG_18_6_RegisterResetValue 0x0
#define CRYPTOCFG_18_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_6_CRYPTOKEY_18_6_BitAddressOffset 0
#define CRYPTOCFG_18_6_CRYPTOKEY_18_6_RegisterSize 32





#define CRYPTOCFG_18_7 (DWC_ufshc_block_BaseAddress + 0x191c)
#define CRYPTOCFG_18_7_RegisterSize 32
#define CRYPTOCFG_18_7_RegisterResetValue 0x0
#define CRYPTOCFG_18_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_7_CRYPTOKEY_18_7_BitAddressOffset 0
#define CRYPTOCFG_18_7_CRYPTOKEY_18_7_RegisterSize 32





#define CRYPTOCFG_18_8 (DWC_ufshc_block_BaseAddress + 0x1920)
#define CRYPTOCFG_18_8_RegisterSize 32
#define CRYPTOCFG_18_8_RegisterResetValue 0x0
#define CRYPTOCFG_18_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_8_CRYPTOKEY_18_8_BitAddressOffset 0
#define CRYPTOCFG_18_8_CRYPTOKEY_18_8_RegisterSize 32





#define CRYPTOCFG_18_9 (DWC_ufshc_block_BaseAddress + 0x1924)
#define CRYPTOCFG_18_9_RegisterSize 32
#define CRYPTOCFG_18_9_RegisterResetValue 0x0
#define CRYPTOCFG_18_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_9_CRYPTOKEY_18_9_BitAddressOffset 0
#define CRYPTOCFG_18_9_CRYPTOKEY_18_9_RegisterSize 32





#define CRYPTOCFG_18_10 (DWC_ufshc_block_BaseAddress + 0x1928)
#define CRYPTOCFG_18_10_RegisterSize 32
#define CRYPTOCFG_18_10_RegisterResetValue 0x0
#define CRYPTOCFG_18_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_10_CRYPTOKEY_18_10_BitAddressOffset 0
#define CRYPTOCFG_18_10_CRYPTOKEY_18_10_RegisterSize 32





#define CRYPTOCFG_18_11 (DWC_ufshc_block_BaseAddress + 0x192c)
#define CRYPTOCFG_18_11_RegisterSize 32
#define CRYPTOCFG_18_11_RegisterResetValue 0x0
#define CRYPTOCFG_18_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_11_CRYPTOKEY_18_11_BitAddressOffset 0
#define CRYPTOCFG_18_11_CRYPTOKEY_18_11_RegisterSize 32





#define CRYPTOCFG_18_12 (DWC_ufshc_block_BaseAddress + 0x1930)
#define CRYPTOCFG_18_12_RegisterSize 32
#define CRYPTOCFG_18_12_RegisterResetValue 0x0
#define CRYPTOCFG_18_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_12_CRYPTOKEY_18_12_BitAddressOffset 0
#define CRYPTOCFG_18_12_CRYPTOKEY_18_12_RegisterSize 32





#define CRYPTOCFG_18_13 (DWC_ufshc_block_BaseAddress + 0x1934)
#define CRYPTOCFG_18_13_RegisterSize 32
#define CRYPTOCFG_18_13_RegisterResetValue 0x0
#define CRYPTOCFG_18_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_13_CRYPTOKEY_18_13_BitAddressOffset 0
#define CRYPTOCFG_18_13_CRYPTOKEY_18_13_RegisterSize 32





#define CRYPTOCFG_18_14 (DWC_ufshc_block_BaseAddress + 0x1938)
#define CRYPTOCFG_18_14_RegisterSize 32
#define CRYPTOCFG_18_14_RegisterResetValue 0x0
#define CRYPTOCFG_18_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_14_CRYPTOKEY_18_14_BitAddressOffset 0
#define CRYPTOCFG_18_14_CRYPTOKEY_18_14_RegisterSize 32





#define CRYPTOCFG_18_15 (DWC_ufshc_block_BaseAddress + 0x193c)
#define CRYPTOCFG_18_15_RegisterSize 32
#define CRYPTOCFG_18_15_RegisterResetValue 0x0
#define CRYPTOCFG_18_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_15_CRYPTOKEY_18_15_BitAddressOffset 0
#define CRYPTOCFG_18_15_CRYPTOKEY_18_15_RegisterSize 32





#define CRYPTOCFG_18_16 (DWC_ufshc_block_BaseAddress + 0x1940)
#define CRYPTOCFG_18_16_RegisterSize 32
#define CRYPTOCFG_18_16_RegisterResetValue 0x0
#define CRYPTOCFG_18_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_16_DUSIZE_18_BitAddressOffset 0
#define CRYPTOCFG_18_16_DUSIZE_18_RegisterSize 8



#define CRYPTOCFG_18_16_CAPIDX_18_BitAddressOffset 8
#define CRYPTOCFG_18_16_CAPIDX_18_RegisterSize 8



#define CRYPTOCFG_18_16_CFGE_18_BitAddressOffset 31
#define CRYPTOCFG_18_16_CFGE_18_RegisterSize 1





#define CRYPTOCFG_18_17 (DWC_ufshc_block_BaseAddress + 0x1944)
#define CRYPTOCFG_18_17_RegisterSize 32
#define CRYPTOCFG_18_17_RegisterResetValue 0x0
#define CRYPTOCFG_18_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_17_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_17_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_18 (DWC_ufshc_block_BaseAddress + 0x1948)
#define CRYPTOCFG_18_18_RegisterSize 32
#define CRYPTOCFG_18_18_RegisterResetValue 0x0
#define CRYPTOCFG_18_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_18_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_18_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_19 (DWC_ufshc_block_BaseAddress + 0x194c)
#define CRYPTOCFG_18_19_RegisterSize 32
#define CRYPTOCFG_18_19_RegisterResetValue 0x0
#define CRYPTOCFG_18_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_19_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_19_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_20 (DWC_ufshc_block_BaseAddress + 0x1950)
#define CRYPTOCFG_18_20_RegisterSize 32
#define CRYPTOCFG_18_20_RegisterResetValue 0x0
#define CRYPTOCFG_18_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_20_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_20_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_21 (DWC_ufshc_block_BaseAddress + 0x1954)
#define CRYPTOCFG_18_21_RegisterSize 32
#define CRYPTOCFG_18_21_RegisterResetValue 0x0
#define CRYPTOCFG_18_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_21_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_21_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_22 (DWC_ufshc_block_BaseAddress + 0x1958)
#define CRYPTOCFG_18_22_RegisterSize 32
#define CRYPTOCFG_18_22_RegisterResetValue 0x0
#define CRYPTOCFG_18_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_22_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_22_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_23 (DWC_ufshc_block_BaseAddress + 0x195c)
#define CRYPTOCFG_18_23_RegisterSize 32
#define CRYPTOCFG_18_23_RegisterResetValue 0x0
#define CRYPTOCFG_18_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_23_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_23_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_24 (DWC_ufshc_block_BaseAddress + 0x1960)
#define CRYPTOCFG_18_24_RegisterSize 32
#define CRYPTOCFG_18_24_RegisterResetValue 0x0
#define CRYPTOCFG_18_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_24_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_24_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_25 (DWC_ufshc_block_BaseAddress + 0x1964)
#define CRYPTOCFG_18_25_RegisterSize 32
#define CRYPTOCFG_18_25_RegisterResetValue 0x0
#define CRYPTOCFG_18_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_25_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_25_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_26 (DWC_ufshc_block_BaseAddress + 0x1968)
#define CRYPTOCFG_18_26_RegisterSize 32
#define CRYPTOCFG_18_26_RegisterResetValue 0x0
#define CRYPTOCFG_18_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_26_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_26_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_27 (DWC_ufshc_block_BaseAddress + 0x196c)
#define CRYPTOCFG_18_27_RegisterSize 32
#define CRYPTOCFG_18_27_RegisterResetValue 0x0
#define CRYPTOCFG_18_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_27_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_27_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_28 (DWC_ufshc_block_BaseAddress + 0x1970)
#define CRYPTOCFG_18_28_RegisterSize 32
#define CRYPTOCFG_18_28_RegisterResetValue 0x0
#define CRYPTOCFG_18_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_28_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_28_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_29 (DWC_ufshc_block_BaseAddress + 0x1974)
#define CRYPTOCFG_18_29_RegisterSize 32
#define CRYPTOCFG_18_29_RegisterResetValue 0x0
#define CRYPTOCFG_18_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_29_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_29_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_30 (DWC_ufshc_block_BaseAddress + 0x1978)
#define CRYPTOCFG_18_30_RegisterSize 32
#define CRYPTOCFG_18_30_RegisterResetValue 0x0
#define CRYPTOCFG_18_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_30_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_30_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_18_31 (DWC_ufshc_block_BaseAddress + 0x197c)
#define CRYPTOCFG_18_31_RegisterSize 32
#define CRYPTOCFG_18_31_RegisterResetValue 0x0
#define CRYPTOCFG_18_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_18_31_RESERVED_18_BitAddressOffset 0
#define CRYPTOCFG_18_31_RESERVED_18_RegisterSize 32





#define CRYPTOCFG_19_0 (DWC_ufshc_block_BaseAddress + 0x1980)
#define CRYPTOCFG_19_0_RegisterSize 32
#define CRYPTOCFG_19_0_RegisterResetValue 0x0
#define CRYPTOCFG_19_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_0_CRYPTOKEY_19_0_BitAddressOffset 0
#define CRYPTOCFG_19_0_CRYPTOKEY_19_0_RegisterSize 32





#define CRYPTOCFG_19_1 (DWC_ufshc_block_BaseAddress + 0x1984)
#define CRYPTOCFG_19_1_RegisterSize 32
#define CRYPTOCFG_19_1_RegisterResetValue 0x0
#define CRYPTOCFG_19_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_1_CRYPTOKEY_19_1_BitAddressOffset 0
#define CRYPTOCFG_19_1_CRYPTOKEY_19_1_RegisterSize 32





#define CRYPTOCFG_19_2 (DWC_ufshc_block_BaseAddress + 0x1988)
#define CRYPTOCFG_19_2_RegisterSize 32
#define CRYPTOCFG_19_2_RegisterResetValue 0x0
#define CRYPTOCFG_19_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_2_CRYPTOKEY_19_2_BitAddressOffset 0
#define CRYPTOCFG_19_2_CRYPTOKEY_19_2_RegisterSize 32





#define CRYPTOCFG_19_3 (DWC_ufshc_block_BaseAddress + 0x198c)
#define CRYPTOCFG_19_3_RegisterSize 32
#define CRYPTOCFG_19_3_RegisterResetValue 0x0
#define CRYPTOCFG_19_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_3_CRYPTOKEY_19_3_BitAddressOffset 0
#define CRYPTOCFG_19_3_CRYPTOKEY_19_3_RegisterSize 32





#define CRYPTOCFG_19_4 (DWC_ufshc_block_BaseAddress + 0x1990)
#define CRYPTOCFG_19_4_RegisterSize 32
#define CRYPTOCFG_19_4_RegisterResetValue 0x0
#define CRYPTOCFG_19_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_4_CRYPTOKEY_19_4_BitAddressOffset 0
#define CRYPTOCFG_19_4_CRYPTOKEY_19_4_RegisterSize 32





#define CRYPTOCFG_19_5 (DWC_ufshc_block_BaseAddress + 0x1994)
#define CRYPTOCFG_19_5_RegisterSize 32
#define CRYPTOCFG_19_5_RegisterResetValue 0x0
#define CRYPTOCFG_19_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_5_CRYPTOKEY_19_5_BitAddressOffset 0
#define CRYPTOCFG_19_5_CRYPTOKEY_19_5_RegisterSize 32





#define CRYPTOCFG_19_6 (DWC_ufshc_block_BaseAddress + 0x1998)
#define CRYPTOCFG_19_6_RegisterSize 32
#define CRYPTOCFG_19_6_RegisterResetValue 0x0
#define CRYPTOCFG_19_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_6_CRYPTOKEY_19_6_BitAddressOffset 0
#define CRYPTOCFG_19_6_CRYPTOKEY_19_6_RegisterSize 32





#define CRYPTOCFG_19_7 (DWC_ufshc_block_BaseAddress + 0x199c)
#define CRYPTOCFG_19_7_RegisterSize 32
#define CRYPTOCFG_19_7_RegisterResetValue 0x0
#define CRYPTOCFG_19_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_7_CRYPTOKEY_19_7_BitAddressOffset 0
#define CRYPTOCFG_19_7_CRYPTOKEY_19_7_RegisterSize 32





#define CRYPTOCFG_19_8 (DWC_ufshc_block_BaseAddress + 0x19a0)
#define CRYPTOCFG_19_8_RegisterSize 32
#define CRYPTOCFG_19_8_RegisterResetValue 0x0
#define CRYPTOCFG_19_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_8_CRYPTOKEY_19_8_BitAddressOffset 0
#define CRYPTOCFG_19_8_CRYPTOKEY_19_8_RegisterSize 32





#define CRYPTOCFG_19_9 (DWC_ufshc_block_BaseAddress + 0x19a4)
#define CRYPTOCFG_19_9_RegisterSize 32
#define CRYPTOCFG_19_9_RegisterResetValue 0x0
#define CRYPTOCFG_19_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_9_CRYPTOKEY_19_9_BitAddressOffset 0
#define CRYPTOCFG_19_9_CRYPTOKEY_19_9_RegisterSize 32





#define CRYPTOCFG_19_10 (DWC_ufshc_block_BaseAddress + 0x19a8)
#define CRYPTOCFG_19_10_RegisterSize 32
#define CRYPTOCFG_19_10_RegisterResetValue 0x0
#define CRYPTOCFG_19_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_10_CRYPTOKEY_19_10_BitAddressOffset 0
#define CRYPTOCFG_19_10_CRYPTOKEY_19_10_RegisterSize 32





#define CRYPTOCFG_19_11 (DWC_ufshc_block_BaseAddress + 0x19ac)
#define CRYPTOCFG_19_11_RegisterSize 32
#define CRYPTOCFG_19_11_RegisterResetValue 0x0
#define CRYPTOCFG_19_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_11_CRYPTOKEY_19_11_BitAddressOffset 0
#define CRYPTOCFG_19_11_CRYPTOKEY_19_11_RegisterSize 32





#define CRYPTOCFG_19_12 (DWC_ufshc_block_BaseAddress + 0x19b0)
#define CRYPTOCFG_19_12_RegisterSize 32
#define CRYPTOCFG_19_12_RegisterResetValue 0x0
#define CRYPTOCFG_19_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_12_CRYPTOKEY_19_12_BitAddressOffset 0
#define CRYPTOCFG_19_12_CRYPTOKEY_19_12_RegisterSize 32





#define CRYPTOCFG_19_13 (DWC_ufshc_block_BaseAddress + 0x19b4)
#define CRYPTOCFG_19_13_RegisterSize 32
#define CRYPTOCFG_19_13_RegisterResetValue 0x0
#define CRYPTOCFG_19_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_13_CRYPTOKEY_19_13_BitAddressOffset 0
#define CRYPTOCFG_19_13_CRYPTOKEY_19_13_RegisterSize 32





#define CRYPTOCFG_19_14 (DWC_ufshc_block_BaseAddress + 0x19b8)
#define CRYPTOCFG_19_14_RegisterSize 32
#define CRYPTOCFG_19_14_RegisterResetValue 0x0
#define CRYPTOCFG_19_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_14_CRYPTOKEY_19_14_BitAddressOffset 0
#define CRYPTOCFG_19_14_CRYPTOKEY_19_14_RegisterSize 32





#define CRYPTOCFG_19_15 (DWC_ufshc_block_BaseAddress + 0x19bc)
#define CRYPTOCFG_19_15_RegisterSize 32
#define CRYPTOCFG_19_15_RegisterResetValue 0x0
#define CRYPTOCFG_19_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_15_CRYPTOKEY_19_15_BitAddressOffset 0
#define CRYPTOCFG_19_15_CRYPTOKEY_19_15_RegisterSize 32





#define CRYPTOCFG_19_16 (DWC_ufshc_block_BaseAddress + 0x19c0)
#define CRYPTOCFG_19_16_RegisterSize 32
#define CRYPTOCFG_19_16_RegisterResetValue 0x0
#define CRYPTOCFG_19_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_16_DUSIZE_19_BitAddressOffset 0
#define CRYPTOCFG_19_16_DUSIZE_19_RegisterSize 8



#define CRYPTOCFG_19_16_CAPIDX_19_BitAddressOffset 8
#define CRYPTOCFG_19_16_CAPIDX_19_RegisterSize 8



#define CRYPTOCFG_19_16_CFGE_19_BitAddressOffset 31
#define CRYPTOCFG_19_16_CFGE_19_RegisterSize 1





#define CRYPTOCFG_19_17 (DWC_ufshc_block_BaseAddress + 0x19c4)
#define CRYPTOCFG_19_17_RegisterSize 32
#define CRYPTOCFG_19_17_RegisterResetValue 0x0
#define CRYPTOCFG_19_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_17_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_17_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_18 (DWC_ufshc_block_BaseAddress + 0x19c8)
#define CRYPTOCFG_19_18_RegisterSize 32
#define CRYPTOCFG_19_18_RegisterResetValue 0x0
#define CRYPTOCFG_19_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_18_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_18_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_19 (DWC_ufshc_block_BaseAddress + 0x19cc)
#define CRYPTOCFG_19_19_RegisterSize 32
#define CRYPTOCFG_19_19_RegisterResetValue 0x0
#define CRYPTOCFG_19_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_19_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_19_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_20 (DWC_ufshc_block_BaseAddress + 0x19d0)
#define CRYPTOCFG_19_20_RegisterSize 32
#define CRYPTOCFG_19_20_RegisterResetValue 0x0
#define CRYPTOCFG_19_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_20_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_20_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_21 (DWC_ufshc_block_BaseAddress + 0x19d4)
#define CRYPTOCFG_19_21_RegisterSize 32
#define CRYPTOCFG_19_21_RegisterResetValue 0x0
#define CRYPTOCFG_19_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_21_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_21_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_22 (DWC_ufshc_block_BaseAddress + 0x19d8)
#define CRYPTOCFG_19_22_RegisterSize 32
#define CRYPTOCFG_19_22_RegisterResetValue 0x0
#define CRYPTOCFG_19_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_22_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_22_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_23 (DWC_ufshc_block_BaseAddress + 0x19dc)
#define CRYPTOCFG_19_23_RegisterSize 32
#define CRYPTOCFG_19_23_RegisterResetValue 0x0
#define CRYPTOCFG_19_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_23_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_23_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_24 (DWC_ufshc_block_BaseAddress + 0x19e0)
#define CRYPTOCFG_19_24_RegisterSize 32
#define CRYPTOCFG_19_24_RegisterResetValue 0x0
#define CRYPTOCFG_19_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_24_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_24_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_25 (DWC_ufshc_block_BaseAddress + 0x19e4)
#define CRYPTOCFG_19_25_RegisterSize 32
#define CRYPTOCFG_19_25_RegisterResetValue 0x0
#define CRYPTOCFG_19_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_25_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_25_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_26 (DWC_ufshc_block_BaseAddress + 0x19e8)
#define CRYPTOCFG_19_26_RegisterSize 32
#define CRYPTOCFG_19_26_RegisterResetValue 0x0
#define CRYPTOCFG_19_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_26_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_26_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_27 (DWC_ufshc_block_BaseAddress + 0x19ec)
#define CRYPTOCFG_19_27_RegisterSize 32
#define CRYPTOCFG_19_27_RegisterResetValue 0x0
#define CRYPTOCFG_19_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_27_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_27_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_28 (DWC_ufshc_block_BaseAddress + 0x19f0)
#define CRYPTOCFG_19_28_RegisterSize 32
#define CRYPTOCFG_19_28_RegisterResetValue 0x0
#define CRYPTOCFG_19_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_28_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_28_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_29 (DWC_ufshc_block_BaseAddress + 0x19f4)
#define CRYPTOCFG_19_29_RegisterSize 32
#define CRYPTOCFG_19_29_RegisterResetValue 0x0
#define CRYPTOCFG_19_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_29_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_29_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_30 (DWC_ufshc_block_BaseAddress + 0x19f8)
#define CRYPTOCFG_19_30_RegisterSize 32
#define CRYPTOCFG_19_30_RegisterResetValue 0x0
#define CRYPTOCFG_19_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_30_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_30_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_19_31 (DWC_ufshc_block_BaseAddress + 0x19fc)
#define CRYPTOCFG_19_31_RegisterSize 32
#define CRYPTOCFG_19_31_RegisterResetValue 0x0
#define CRYPTOCFG_19_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_19_31_RESERVED_19_BitAddressOffset 0
#define CRYPTOCFG_19_31_RESERVED_19_RegisterSize 32





#define CRYPTOCFG_20_0 (DWC_ufshc_block_BaseAddress + 0x1a00)
#define CRYPTOCFG_20_0_RegisterSize 32
#define CRYPTOCFG_20_0_RegisterResetValue 0x0
#define CRYPTOCFG_20_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_0_CRYPTOKEY_20_0_BitAddressOffset 0
#define CRYPTOCFG_20_0_CRYPTOKEY_20_0_RegisterSize 32





#define CRYPTOCFG_20_1 (DWC_ufshc_block_BaseAddress + 0x1a04)
#define CRYPTOCFG_20_1_RegisterSize 32
#define CRYPTOCFG_20_1_RegisterResetValue 0x0
#define CRYPTOCFG_20_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_1_CRYPTOKEY_20_1_BitAddressOffset 0
#define CRYPTOCFG_20_1_CRYPTOKEY_20_1_RegisterSize 32





#define CRYPTOCFG_20_2 (DWC_ufshc_block_BaseAddress + 0x1a08)
#define CRYPTOCFG_20_2_RegisterSize 32
#define CRYPTOCFG_20_2_RegisterResetValue 0x0
#define CRYPTOCFG_20_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_2_CRYPTOKEY_20_2_BitAddressOffset 0
#define CRYPTOCFG_20_2_CRYPTOKEY_20_2_RegisterSize 32





#define CRYPTOCFG_20_3 (DWC_ufshc_block_BaseAddress + 0x1a0c)
#define CRYPTOCFG_20_3_RegisterSize 32
#define CRYPTOCFG_20_3_RegisterResetValue 0x0
#define CRYPTOCFG_20_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_3_CRYPTOKEY_20_3_BitAddressOffset 0
#define CRYPTOCFG_20_3_CRYPTOKEY_20_3_RegisterSize 32





#define CRYPTOCFG_20_4 (DWC_ufshc_block_BaseAddress + 0x1a10)
#define CRYPTOCFG_20_4_RegisterSize 32
#define CRYPTOCFG_20_4_RegisterResetValue 0x0
#define CRYPTOCFG_20_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_4_CRYPTOKEY_20_4_BitAddressOffset 0
#define CRYPTOCFG_20_4_CRYPTOKEY_20_4_RegisterSize 32





#define CRYPTOCFG_20_5 (DWC_ufshc_block_BaseAddress + 0x1a14)
#define CRYPTOCFG_20_5_RegisterSize 32
#define CRYPTOCFG_20_5_RegisterResetValue 0x0
#define CRYPTOCFG_20_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_5_CRYPTOKEY_20_5_BitAddressOffset 0
#define CRYPTOCFG_20_5_CRYPTOKEY_20_5_RegisterSize 32





#define CRYPTOCFG_20_6 (DWC_ufshc_block_BaseAddress + 0x1a18)
#define CRYPTOCFG_20_6_RegisterSize 32
#define CRYPTOCFG_20_6_RegisterResetValue 0x0
#define CRYPTOCFG_20_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_6_CRYPTOKEY_20_6_BitAddressOffset 0
#define CRYPTOCFG_20_6_CRYPTOKEY_20_6_RegisterSize 32





#define CRYPTOCFG_20_7 (DWC_ufshc_block_BaseAddress + 0x1a1c)
#define CRYPTOCFG_20_7_RegisterSize 32
#define CRYPTOCFG_20_7_RegisterResetValue 0x0
#define CRYPTOCFG_20_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_7_CRYPTOKEY_20_7_BitAddressOffset 0
#define CRYPTOCFG_20_7_CRYPTOKEY_20_7_RegisterSize 32





#define CRYPTOCFG_20_8 (DWC_ufshc_block_BaseAddress + 0x1a20)
#define CRYPTOCFG_20_8_RegisterSize 32
#define CRYPTOCFG_20_8_RegisterResetValue 0x0
#define CRYPTOCFG_20_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_8_CRYPTOKEY_20_8_BitAddressOffset 0
#define CRYPTOCFG_20_8_CRYPTOKEY_20_8_RegisterSize 32





#define CRYPTOCFG_20_9 (DWC_ufshc_block_BaseAddress + 0x1a24)
#define CRYPTOCFG_20_9_RegisterSize 32
#define CRYPTOCFG_20_9_RegisterResetValue 0x0
#define CRYPTOCFG_20_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_9_CRYPTOKEY_20_9_BitAddressOffset 0
#define CRYPTOCFG_20_9_CRYPTOKEY_20_9_RegisterSize 32





#define CRYPTOCFG_20_10 (DWC_ufshc_block_BaseAddress + 0x1a28)
#define CRYPTOCFG_20_10_RegisterSize 32
#define CRYPTOCFG_20_10_RegisterResetValue 0x0
#define CRYPTOCFG_20_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_10_CRYPTOKEY_20_10_BitAddressOffset 0
#define CRYPTOCFG_20_10_CRYPTOKEY_20_10_RegisterSize 32





#define CRYPTOCFG_20_11 (DWC_ufshc_block_BaseAddress + 0x1a2c)
#define CRYPTOCFG_20_11_RegisterSize 32
#define CRYPTOCFG_20_11_RegisterResetValue 0x0
#define CRYPTOCFG_20_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_11_CRYPTOKEY_20_11_BitAddressOffset 0
#define CRYPTOCFG_20_11_CRYPTOKEY_20_11_RegisterSize 32





#define CRYPTOCFG_20_12 (DWC_ufshc_block_BaseAddress + 0x1a30)
#define CRYPTOCFG_20_12_RegisterSize 32
#define CRYPTOCFG_20_12_RegisterResetValue 0x0
#define CRYPTOCFG_20_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_12_CRYPTOKEY_20_12_BitAddressOffset 0
#define CRYPTOCFG_20_12_CRYPTOKEY_20_12_RegisterSize 32





#define CRYPTOCFG_20_13 (DWC_ufshc_block_BaseAddress + 0x1a34)
#define CRYPTOCFG_20_13_RegisterSize 32
#define CRYPTOCFG_20_13_RegisterResetValue 0x0
#define CRYPTOCFG_20_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_13_CRYPTOKEY_20_13_BitAddressOffset 0
#define CRYPTOCFG_20_13_CRYPTOKEY_20_13_RegisterSize 32





#define CRYPTOCFG_20_14 (DWC_ufshc_block_BaseAddress + 0x1a38)
#define CRYPTOCFG_20_14_RegisterSize 32
#define CRYPTOCFG_20_14_RegisterResetValue 0x0
#define CRYPTOCFG_20_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_14_CRYPTOKEY_20_14_BitAddressOffset 0
#define CRYPTOCFG_20_14_CRYPTOKEY_20_14_RegisterSize 32





#define CRYPTOCFG_20_15 (DWC_ufshc_block_BaseAddress + 0x1a3c)
#define CRYPTOCFG_20_15_RegisterSize 32
#define CRYPTOCFG_20_15_RegisterResetValue 0x0
#define CRYPTOCFG_20_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_15_CRYPTOKEY_20_15_BitAddressOffset 0
#define CRYPTOCFG_20_15_CRYPTOKEY_20_15_RegisterSize 32





#define CRYPTOCFG_20_16 (DWC_ufshc_block_BaseAddress + 0x1a40)
#define CRYPTOCFG_20_16_RegisterSize 32
#define CRYPTOCFG_20_16_RegisterResetValue 0x0
#define CRYPTOCFG_20_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_16_DUSIZE_20_BitAddressOffset 0
#define CRYPTOCFG_20_16_DUSIZE_20_RegisterSize 8



#define CRYPTOCFG_20_16_CAPIDX_20_BitAddressOffset 8
#define CRYPTOCFG_20_16_CAPIDX_20_RegisterSize 8



#define CRYPTOCFG_20_16_CFGE_20_BitAddressOffset 31
#define CRYPTOCFG_20_16_CFGE_20_RegisterSize 1





#define CRYPTOCFG_20_17 (DWC_ufshc_block_BaseAddress + 0x1a44)
#define CRYPTOCFG_20_17_RegisterSize 32
#define CRYPTOCFG_20_17_RegisterResetValue 0x0
#define CRYPTOCFG_20_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_17_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_17_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_18 (DWC_ufshc_block_BaseAddress + 0x1a48)
#define CRYPTOCFG_20_18_RegisterSize 32
#define CRYPTOCFG_20_18_RegisterResetValue 0x0
#define CRYPTOCFG_20_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_18_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_18_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_19 (DWC_ufshc_block_BaseAddress + 0x1a4c)
#define CRYPTOCFG_20_19_RegisterSize 32
#define CRYPTOCFG_20_19_RegisterResetValue 0x0
#define CRYPTOCFG_20_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_19_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_19_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_20 (DWC_ufshc_block_BaseAddress + 0x1a50)
#define CRYPTOCFG_20_20_RegisterSize 32
#define CRYPTOCFG_20_20_RegisterResetValue 0x0
#define CRYPTOCFG_20_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_20_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_20_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_21 (DWC_ufshc_block_BaseAddress + 0x1a54)
#define CRYPTOCFG_20_21_RegisterSize 32
#define CRYPTOCFG_20_21_RegisterResetValue 0x0
#define CRYPTOCFG_20_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_21_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_21_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_22 (DWC_ufshc_block_BaseAddress + 0x1a58)
#define CRYPTOCFG_20_22_RegisterSize 32
#define CRYPTOCFG_20_22_RegisterResetValue 0x0
#define CRYPTOCFG_20_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_22_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_22_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_23 (DWC_ufshc_block_BaseAddress + 0x1a5c)
#define CRYPTOCFG_20_23_RegisterSize 32
#define CRYPTOCFG_20_23_RegisterResetValue 0x0
#define CRYPTOCFG_20_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_23_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_23_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_24 (DWC_ufshc_block_BaseAddress + 0x1a60)
#define CRYPTOCFG_20_24_RegisterSize 32
#define CRYPTOCFG_20_24_RegisterResetValue 0x0
#define CRYPTOCFG_20_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_24_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_24_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_25 (DWC_ufshc_block_BaseAddress + 0x1a64)
#define CRYPTOCFG_20_25_RegisterSize 32
#define CRYPTOCFG_20_25_RegisterResetValue 0x0
#define CRYPTOCFG_20_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_25_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_25_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_26 (DWC_ufshc_block_BaseAddress + 0x1a68)
#define CRYPTOCFG_20_26_RegisterSize 32
#define CRYPTOCFG_20_26_RegisterResetValue 0x0
#define CRYPTOCFG_20_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_26_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_26_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_27 (DWC_ufshc_block_BaseAddress + 0x1a6c)
#define CRYPTOCFG_20_27_RegisterSize 32
#define CRYPTOCFG_20_27_RegisterResetValue 0x0
#define CRYPTOCFG_20_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_27_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_27_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_28 (DWC_ufshc_block_BaseAddress + 0x1a70)
#define CRYPTOCFG_20_28_RegisterSize 32
#define CRYPTOCFG_20_28_RegisterResetValue 0x0
#define CRYPTOCFG_20_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_28_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_28_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_29 (DWC_ufshc_block_BaseAddress + 0x1a74)
#define CRYPTOCFG_20_29_RegisterSize 32
#define CRYPTOCFG_20_29_RegisterResetValue 0x0
#define CRYPTOCFG_20_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_29_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_29_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_30 (DWC_ufshc_block_BaseAddress + 0x1a78)
#define CRYPTOCFG_20_30_RegisterSize 32
#define CRYPTOCFG_20_30_RegisterResetValue 0x0
#define CRYPTOCFG_20_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_30_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_30_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_20_31 (DWC_ufshc_block_BaseAddress + 0x1a7c)
#define CRYPTOCFG_20_31_RegisterSize 32
#define CRYPTOCFG_20_31_RegisterResetValue 0x0
#define CRYPTOCFG_20_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_20_31_RESERVED_20_BitAddressOffset 0
#define CRYPTOCFG_20_31_RESERVED_20_RegisterSize 32





#define CRYPTOCFG_21_0 (DWC_ufshc_block_BaseAddress + 0x1a80)
#define CRYPTOCFG_21_0_RegisterSize 32
#define CRYPTOCFG_21_0_RegisterResetValue 0x0
#define CRYPTOCFG_21_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_0_CRYPTOKEY_21_0_BitAddressOffset 0
#define CRYPTOCFG_21_0_CRYPTOKEY_21_0_RegisterSize 32





#define CRYPTOCFG_21_1 (DWC_ufshc_block_BaseAddress + 0x1a84)
#define CRYPTOCFG_21_1_RegisterSize 32
#define CRYPTOCFG_21_1_RegisterResetValue 0x0
#define CRYPTOCFG_21_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_1_CRYPTOKEY_21_1_BitAddressOffset 0
#define CRYPTOCFG_21_1_CRYPTOKEY_21_1_RegisterSize 32





#define CRYPTOCFG_21_2 (DWC_ufshc_block_BaseAddress + 0x1a88)
#define CRYPTOCFG_21_2_RegisterSize 32
#define CRYPTOCFG_21_2_RegisterResetValue 0x0
#define CRYPTOCFG_21_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_2_CRYPTOKEY_21_2_BitAddressOffset 0
#define CRYPTOCFG_21_2_CRYPTOKEY_21_2_RegisterSize 32





#define CRYPTOCFG_21_3 (DWC_ufshc_block_BaseAddress + 0x1a8c)
#define CRYPTOCFG_21_3_RegisterSize 32
#define CRYPTOCFG_21_3_RegisterResetValue 0x0
#define CRYPTOCFG_21_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_3_CRYPTOKEY_21_3_BitAddressOffset 0
#define CRYPTOCFG_21_3_CRYPTOKEY_21_3_RegisterSize 32





#define CRYPTOCFG_21_4 (DWC_ufshc_block_BaseAddress + 0x1a90)
#define CRYPTOCFG_21_4_RegisterSize 32
#define CRYPTOCFG_21_4_RegisterResetValue 0x0
#define CRYPTOCFG_21_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_4_CRYPTOKEY_21_4_BitAddressOffset 0
#define CRYPTOCFG_21_4_CRYPTOKEY_21_4_RegisterSize 32





#define CRYPTOCFG_21_5 (DWC_ufshc_block_BaseAddress + 0x1a94)
#define CRYPTOCFG_21_5_RegisterSize 32
#define CRYPTOCFG_21_5_RegisterResetValue 0x0
#define CRYPTOCFG_21_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_5_CRYPTOKEY_21_5_BitAddressOffset 0
#define CRYPTOCFG_21_5_CRYPTOKEY_21_5_RegisterSize 32





#define CRYPTOCFG_21_6 (DWC_ufshc_block_BaseAddress + 0x1a98)
#define CRYPTOCFG_21_6_RegisterSize 32
#define CRYPTOCFG_21_6_RegisterResetValue 0x0
#define CRYPTOCFG_21_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_6_CRYPTOKEY_21_6_BitAddressOffset 0
#define CRYPTOCFG_21_6_CRYPTOKEY_21_6_RegisterSize 32





#define CRYPTOCFG_21_7 (DWC_ufshc_block_BaseAddress + 0x1a9c)
#define CRYPTOCFG_21_7_RegisterSize 32
#define CRYPTOCFG_21_7_RegisterResetValue 0x0
#define CRYPTOCFG_21_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_7_CRYPTOKEY_21_7_BitAddressOffset 0
#define CRYPTOCFG_21_7_CRYPTOKEY_21_7_RegisterSize 32





#define CRYPTOCFG_21_8 (DWC_ufshc_block_BaseAddress + 0x1aa0)
#define CRYPTOCFG_21_8_RegisterSize 32
#define CRYPTOCFG_21_8_RegisterResetValue 0x0
#define CRYPTOCFG_21_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_8_CRYPTOKEY_21_8_BitAddressOffset 0
#define CRYPTOCFG_21_8_CRYPTOKEY_21_8_RegisterSize 32





#define CRYPTOCFG_21_9 (DWC_ufshc_block_BaseAddress + 0x1aa4)
#define CRYPTOCFG_21_9_RegisterSize 32
#define CRYPTOCFG_21_9_RegisterResetValue 0x0
#define CRYPTOCFG_21_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_9_CRYPTOKEY_21_9_BitAddressOffset 0
#define CRYPTOCFG_21_9_CRYPTOKEY_21_9_RegisterSize 32





#define CRYPTOCFG_21_10 (DWC_ufshc_block_BaseAddress + 0x1aa8)
#define CRYPTOCFG_21_10_RegisterSize 32
#define CRYPTOCFG_21_10_RegisterResetValue 0x0
#define CRYPTOCFG_21_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_10_CRYPTOKEY_21_10_BitAddressOffset 0
#define CRYPTOCFG_21_10_CRYPTOKEY_21_10_RegisterSize 32





#define CRYPTOCFG_21_11 (DWC_ufshc_block_BaseAddress + 0x1aac)
#define CRYPTOCFG_21_11_RegisterSize 32
#define CRYPTOCFG_21_11_RegisterResetValue 0x0
#define CRYPTOCFG_21_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_11_CRYPTOKEY_21_11_BitAddressOffset 0
#define CRYPTOCFG_21_11_CRYPTOKEY_21_11_RegisterSize 32





#define CRYPTOCFG_21_12 (DWC_ufshc_block_BaseAddress + 0x1ab0)
#define CRYPTOCFG_21_12_RegisterSize 32
#define CRYPTOCFG_21_12_RegisterResetValue 0x0
#define CRYPTOCFG_21_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_12_CRYPTOKEY_21_12_BitAddressOffset 0
#define CRYPTOCFG_21_12_CRYPTOKEY_21_12_RegisterSize 32





#define CRYPTOCFG_21_13 (DWC_ufshc_block_BaseAddress + 0x1ab4)
#define CRYPTOCFG_21_13_RegisterSize 32
#define CRYPTOCFG_21_13_RegisterResetValue 0x0
#define CRYPTOCFG_21_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_13_CRYPTOKEY_21_13_BitAddressOffset 0
#define CRYPTOCFG_21_13_CRYPTOKEY_21_13_RegisterSize 32





#define CRYPTOCFG_21_14 (DWC_ufshc_block_BaseAddress + 0x1ab8)
#define CRYPTOCFG_21_14_RegisterSize 32
#define CRYPTOCFG_21_14_RegisterResetValue 0x0
#define CRYPTOCFG_21_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_14_CRYPTOKEY_21_14_BitAddressOffset 0
#define CRYPTOCFG_21_14_CRYPTOKEY_21_14_RegisterSize 32





#define CRYPTOCFG_21_15 (DWC_ufshc_block_BaseAddress + 0x1abc)
#define CRYPTOCFG_21_15_RegisterSize 32
#define CRYPTOCFG_21_15_RegisterResetValue 0x0
#define CRYPTOCFG_21_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_15_CRYPTOKEY_21_15_BitAddressOffset 0
#define CRYPTOCFG_21_15_CRYPTOKEY_21_15_RegisterSize 32





#define CRYPTOCFG_21_16 (DWC_ufshc_block_BaseAddress + 0x1ac0)
#define CRYPTOCFG_21_16_RegisterSize 32
#define CRYPTOCFG_21_16_RegisterResetValue 0x0
#define CRYPTOCFG_21_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_16_DUSIZE_21_BitAddressOffset 0
#define CRYPTOCFG_21_16_DUSIZE_21_RegisterSize 8



#define CRYPTOCFG_21_16_CAPIDX_21_BitAddressOffset 8
#define CRYPTOCFG_21_16_CAPIDX_21_RegisterSize 8



#define CRYPTOCFG_21_16_CFGE_21_BitAddressOffset 31
#define CRYPTOCFG_21_16_CFGE_21_RegisterSize 1





#define CRYPTOCFG_21_17 (DWC_ufshc_block_BaseAddress + 0x1ac4)
#define CRYPTOCFG_21_17_RegisterSize 32
#define CRYPTOCFG_21_17_RegisterResetValue 0x0
#define CRYPTOCFG_21_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_17_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_17_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_18 (DWC_ufshc_block_BaseAddress + 0x1ac8)
#define CRYPTOCFG_21_18_RegisterSize 32
#define CRYPTOCFG_21_18_RegisterResetValue 0x0
#define CRYPTOCFG_21_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_18_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_18_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_19 (DWC_ufshc_block_BaseAddress + 0x1acc)
#define CRYPTOCFG_21_19_RegisterSize 32
#define CRYPTOCFG_21_19_RegisterResetValue 0x0
#define CRYPTOCFG_21_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_19_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_19_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_20 (DWC_ufshc_block_BaseAddress + 0x1ad0)
#define CRYPTOCFG_21_20_RegisterSize 32
#define CRYPTOCFG_21_20_RegisterResetValue 0x0
#define CRYPTOCFG_21_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_20_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_20_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_21 (DWC_ufshc_block_BaseAddress + 0x1ad4)
#define CRYPTOCFG_21_21_RegisterSize 32
#define CRYPTOCFG_21_21_RegisterResetValue 0x0
#define CRYPTOCFG_21_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_21_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_21_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_22 (DWC_ufshc_block_BaseAddress + 0x1ad8)
#define CRYPTOCFG_21_22_RegisterSize 32
#define CRYPTOCFG_21_22_RegisterResetValue 0x0
#define CRYPTOCFG_21_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_22_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_22_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_23 (DWC_ufshc_block_BaseAddress + 0x1adc)
#define CRYPTOCFG_21_23_RegisterSize 32
#define CRYPTOCFG_21_23_RegisterResetValue 0x0
#define CRYPTOCFG_21_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_23_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_23_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_24 (DWC_ufshc_block_BaseAddress + 0x1ae0)
#define CRYPTOCFG_21_24_RegisterSize 32
#define CRYPTOCFG_21_24_RegisterResetValue 0x0
#define CRYPTOCFG_21_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_24_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_24_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_25 (DWC_ufshc_block_BaseAddress + 0x1ae4)
#define CRYPTOCFG_21_25_RegisterSize 32
#define CRYPTOCFG_21_25_RegisterResetValue 0x0
#define CRYPTOCFG_21_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_25_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_25_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_26 (DWC_ufshc_block_BaseAddress + 0x1ae8)
#define CRYPTOCFG_21_26_RegisterSize 32
#define CRYPTOCFG_21_26_RegisterResetValue 0x0
#define CRYPTOCFG_21_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_26_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_26_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_27 (DWC_ufshc_block_BaseAddress + 0x1aec)
#define CRYPTOCFG_21_27_RegisterSize 32
#define CRYPTOCFG_21_27_RegisterResetValue 0x0
#define CRYPTOCFG_21_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_27_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_27_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_28 (DWC_ufshc_block_BaseAddress + 0x1af0)
#define CRYPTOCFG_21_28_RegisterSize 32
#define CRYPTOCFG_21_28_RegisterResetValue 0x0
#define CRYPTOCFG_21_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_28_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_28_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_29 (DWC_ufshc_block_BaseAddress + 0x1af4)
#define CRYPTOCFG_21_29_RegisterSize 32
#define CRYPTOCFG_21_29_RegisterResetValue 0x0
#define CRYPTOCFG_21_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_29_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_29_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_30 (DWC_ufshc_block_BaseAddress + 0x1af8)
#define CRYPTOCFG_21_30_RegisterSize 32
#define CRYPTOCFG_21_30_RegisterResetValue 0x0
#define CRYPTOCFG_21_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_30_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_30_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_21_31 (DWC_ufshc_block_BaseAddress + 0x1afc)
#define CRYPTOCFG_21_31_RegisterSize 32
#define CRYPTOCFG_21_31_RegisterResetValue 0x0
#define CRYPTOCFG_21_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_21_31_RESERVED_21_BitAddressOffset 0
#define CRYPTOCFG_21_31_RESERVED_21_RegisterSize 32





#define CRYPTOCFG_22_0 (DWC_ufshc_block_BaseAddress + 0x1b00)
#define CRYPTOCFG_22_0_RegisterSize 32
#define CRYPTOCFG_22_0_RegisterResetValue 0x0
#define CRYPTOCFG_22_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_0_CRYPTOKEY_22_0_BitAddressOffset 0
#define CRYPTOCFG_22_0_CRYPTOKEY_22_0_RegisterSize 32





#define CRYPTOCFG_22_1 (DWC_ufshc_block_BaseAddress + 0x1b04)
#define CRYPTOCFG_22_1_RegisterSize 32
#define CRYPTOCFG_22_1_RegisterResetValue 0x0
#define CRYPTOCFG_22_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_1_CRYPTOKEY_22_1_BitAddressOffset 0
#define CRYPTOCFG_22_1_CRYPTOKEY_22_1_RegisterSize 32





#define CRYPTOCFG_22_2 (DWC_ufshc_block_BaseAddress + 0x1b08)
#define CRYPTOCFG_22_2_RegisterSize 32
#define CRYPTOCFG_22_2_RegisterResetValue 0x0
#define CRYPTOCFG_22_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_2_CRYPTOKEY_22_2_BitAddressOffset 0
#define CRYPTOCFG_22_2_CRYPTOKEY_22_2_RegisterSize 32





#define CRYPTOCFG_22_3 (DWC_ufshc_block_BaseAddress + 0x1b0c)
#define CRYPTOCFG_22_3_RegisterSize 32
#define CRYPTOCFG_22_3_RegisterResetValue 0x0
#define CRYPTOCFG_22_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_3_CRYPTOKEY_22_3_BitAddressOffset 0
#define CRYPTOCFG_22_3_CRYPTOKEY_22_3_RegisterSize 32





#define CRYPTOCFG_22_4 (DWC_ufshc_block_BaseAddress + 0x1b10)
#define CRYPTOCFG_22_4_RegisterSize 32
#define CRYPTOCFG_22_4_RegisterResetValue 0x0
#define CRYPTOCFG_22_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_4_CRYPTOKEY_22_4_BitAddressOffset 0
#define CRYPTOCFG_22_4_CRYPTOKEY_22_4_RegisterSize 32





#define CRYPTOCFG_22_5 (DWC_ufshc_block_BaseAddress + 0x1b14)
#define CRYPTOCFG_22_5_RegisterSize 32
#define CRYPTOCFG_22_5_RegisterResetValue 0x0
#define CRYPTOCFG_22_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_5_CRYPTOKEY_22_5_BitAddressOffset 0
#define CRYPTOCFG_22_5_CRYPTOKEY_22_5_RegisterSize 32





#define CRYPTOCFG_22_6 (DWC_ufshc_block_BaseAddress + 0x1b18)
#define CRYPTOCFG_22_6_RegisterSize 32
#define CRYPTOCFG_22_6_RegisterResetValue 0x0
#define CRYPTOCFG_22_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_6_CRYPTOKEY_22_6_BitAddressOffset 0
#define CRYPTOCFG_22_6_CRYPTOKEY_22_6_RegisterSize 32





#define CRYPTOCFG_22_7 (DWC_ufshc_block_BaseAddress + 0x1b1c)
#define CRYPTOCFG_22_7_RegisterSize 32
#define CRYPTOCFG_22_7_RegisterResetValue 0x0
#define CRYPTOCFG_22_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_7_CRYPTOKEY_22_7_BitAddressOffset 0
#define CRYPTOCFG_22_7_CRYPTOKEY_22_7_RegisterSize 32





#define CRYPTOCFG_22_8 (DWC_ufshc_block_BaseAddress + 0x1b20)
#define CRYPTOCFG_22_8_RegisterSize 32
#define CRYPTOCFG_22_8_RegisterResetValue 0x0
#define CRYPTOCFG_22_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_8_CRYPTOKEY_22_8_BitAddressOffset 0
#define CRYPTOCFG_22_8_CRYPTOKEY_22_8_RegisterSize 32





#define CRYPTOCFG_22_9 (DWC_ufshc_block_BaseAddress + 0x1b24)
#define CRYPTOCFG_22_9_RegisterSize 32
#define CRYPTOCFG_22_9_RegisterResetValue 0x0
#define CRYPTOCFG_22_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_9_CRYPTOKEY_22_9_BitAddressOffset 0
#define CRYPTOCFG_22_9_CRYPTOKEY_22_9_RegisterSize 32





#define CRYPTOCFG_22_10 (DWC_ufshc_block_BaseAddress + 0x1b28)
#define CRYPTOCFG_22_10_RegisterSize 32
#define CRYPTOCFG_22_10_RegisterResetValue 0x0
#define CRYPTOCFG_22_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_10_CRYPTOKEY_22_10_BitAddressOffset 0
#define CRYPTOCFG_22_10_CRYPTOKEY_22_10_RegisterSize 32





#define CRYPTOCFG_22_11 (DWC_ufshc_block_BaseAddress + 0x1b2c)
#define CRYPTOCFG_22_11_RegisterSize 32
#define CRYPTOCFG_22_11_RegisterResetValue 0x0
#define CRYPTOCFG_22_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_11_CRYPTOKEY_22_11_BitAddressOffset 0
#define CRYPTOCFG_22_11_CRYPTOKEY_22_11_RegisterSize 32





#define CRYPTOCFG_22_12 (DWC_ufshc_block_BaseAddress + 0x1b30)
#define CRYPTOCFG_22_12_RegisterSize 32
#define CRYPTOCFG_22_12_RegisterResetValue 0x0
#define CRYPTOCFG_22_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_12_CRYPTOKEY_22_12_BitAddressOffset 0
#define CRYPTOCFG_22_12_CRYPTOKEY_22_12_RegisterSize 32





#define CRYPTOCFG_22_13 (DWC_ufshc_block_BaseAddress + 0x1b34)
#define CRYPTOCFG_22_13_RegisterSize 32
#define CRYPTOCFG_22_13_RegisterResetValue 0x0
#define CRYPTOCFG_22_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_13_CRYPTOKEY_22_13_BitAddressOffset 0
#define CRYPTOCFG_22_13_CRYPTOKEY_22_13_RegisterSize 32





#define CRYPTOCFG_22_14 (DWC_ufshc_block_BaseAddress + 0x1b38)
#define CRYPTOCFG_22_14_RegisterSize 32
#define CRYPTOCFG_22_14_RegisterResetValue 0x0
#define CRYPTOCFG_22_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_14_CRYPTOKEY_22_14_BitAddressOffset 0
#define CRYPTOCFG_22_14_CRYPTOKEY_22_14_RegisterSize 32





#define CRYPTOCFG_22_15 (DWC_ufshc_block_BaseAddress + 0x1b3c)
#define CRYPTOCFG_22_15_RegisterSize 32
#define CRYPTOCFG_22_15_RegisterResetValue 0x0
#define CRYPTOCFG_22_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_15_CRYPTOKEY_22_15_BitAddressOffset 0
#define CRYPTOCFG_22_15_CRYPTOKEY_22_15_RegisterSize 32





#define CRYPTOCFG_22_16 (DWC_ufshc_block_BaseAddress + 0x1b40)
#define CRYPTOCFG_22_16_RegisterSize 32
#define CRYPTOCFG_22_16_RegisterResetValue 0x0
#define CRYPTOCFG_22_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_16_DUSIZE_22_BitAddressOffset 0
#define CRYPTOCFG_22_16_DUSIZE_22_RegisterSize 8



#define CRYPTOCFG_22_16_CAPIDX_22_BitAddressOffset 8
#define CRYPTOCFG_22_16_CAPIDX_22_RegisterSize 8



#define CRYPTOCFG_22_16_CFGE_22_BitAddressOffset 31
#define CRYPTOCFG_22_16_CFGE_22_RegisterSize 1





#define CRYPTOCFG_22_17 (DWC_ufshc_block_BaseAddress + 0x1b44)
#define CRYPTOCFG_22_17_RegisterSize 32
#define CRYPTOCFG_22_17_RegisterResetValue 0x0
#define CRYPTOCFG_22_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_17_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_17_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_18 (DWC_ufshc_block_BaseAddress + 0x1b48)
#define CRYPTOCFG_22_18_RegisterSize 32
#define CRYPTOCFG_22_18_RegisterResetValue 0x0
#define CRYPTOCFG_22_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_18_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_18_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_19 (DWC_ufshc_block_BaseAddress + 0x1b4c)
#define CRYPTOCFG_22_19_RegisterSize 32
#define CRYPTOCFG_22_19_RegisterResetValue 0x0
#define CRYPTOCFG_22_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_19_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_19_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_20 (DWC_ufshc_block_BaseAddress + 0x1b50)
#define CRYPTOCFG_22_20_RegisterSize 32
#define CRYPTOCFG_22_20_RegisterResetValue 0x0
#define CRYPTOCFG_22_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_20_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_20_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_21 (DWC_ufshc_block_BaseAddress + 0x1b54)
#define CRYPTOCFG_22_21_RegisterSize 32
#define CRYPTOCFG_22_21_RegisterResetValue 0x0
#define CRYPTOCFG_22_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_21_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_21_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_22 (DWC_ufshc_block_BaseAddress + 0x1b58)
#define CRYPTOCFG_22_22_RegisterSize 32
#define CRYPTOCFG_22_22_RegisterResetValue 0x0
#define CRYPTOCFG_22_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_22_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_22_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_23 (DWC_ufshc_block_BaseAddress + 0x1b5c)
#define CRYPTOCFG_22_23_RegisterSize 32
#define CRYPTOCFG_22_23_RegisterResetValue 0x0
#define CRYPTOCFG_22_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_23_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_23_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_24 (DWC_ufshc_block_BaseAddress + 0x1b60)
#define CRYPTOCFG_22_24_RegisterSize 32
#define CRYPTOCFG_22_24_RegisterResetValue 0x0
#define CRYPTOCFG_22_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_24_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_24_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_25 (DWC_ufshc_block_BaseAddress + 0x1b64)
#define CRYPTOCFG_22_25_RegisterSize 32
#define CRYPTOCFG_22_25_RegisterResetValue 0x0
#define CRYPTOCFG_22_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_25_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_25_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_26 (DWC_ufshc_block_BaseAddress + 0x1b68)
#define CRYPTOCFG_22_26_RegisterSize 32
#define CRYPTOCFG_22_26_RegisterResetValue 0x0
#define CRYPTOCFG_22_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_26_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_26_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_27 (DWC_ufshc_block_BaseAddress + 0x1b6c)
#define CRYPTOCFG_22_27_RegisterSize 32
#define CRYPTOCFG_22_27_RegisterResetValue 0x0
#define CRYPTOCFG_22_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_27_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_27_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_28 (DWC_ufshc_block_BaseAddress + 0x1b70)
#define CRYPTOCFG_22_28_RegisterSize 32
#define CRYPTOCFG_22_28_RegisterResetValue 0x0
#define CRYPTOCFG_22_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_28_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_28_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_29 (DWC_ufshc_block_BaseAddress + 0x1b74)
#define CRYPTOCFG_22_29_RegisterSize 32
#define CRYPTOCFG_22_29_RegisterResetValue 0x0
#define CRYPTOCFG_22_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_29_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_29_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_30 (DWC_ufshc_block_BaseAddress + 0x1b78)
#define CRYPTOCFG_22_30_RegisterSize 32
#define CRYPTOCFG_22_30_RegisterResetValue 0x0
#define CRYPTOCFG_22_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_30_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_30_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_22_31 (DWC_ufshc_block_BaseAddress + 0x1b7c)
#define CRYPTOCFG_22_31_RegisterSize 32
#define CRYPTOCFG_22_31_RegisterResetValue 0x0
#define CRYPTOCFG_22_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_22_31_RESERVED_22_BitAddressOffset 0
#define CRYPTOCFG_22_31_RESERVED_22_RegisterSize 32





#define CRYPTOCFG_23_0 (DWC_ufshc_block_BaseAddress + 0x1b80)
#define CRYPTOCFG_23_0_RegisterSize 32
#define CRYPTOCFG_23_0_RegisterResetValue 0x0
#define CRYPTOCFG_23_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_0_CRYPTOKEY_23_0_BitAddressOffset 0
#define CRYPTOCFG_23_0_CRYPTOKEY_23_0_RegisterSize 32





#define CRYPTOCFG_23_1 (DWC_ufshc_block_BaseAddress + 0x1b84)
#define CRYPTOCFG_23_1_RegisterSize 32
#define CRYPTOCFG_23_1_RegisterResetValue 0x0
#define CRYPTOCFG_23_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_1_CRYPTOKEY_23_1_BitAddressOffset 0
#define CRYPTOCFG_23_1_CRYPTOKEY_23_1_RegisterSize 32





#define CRYPTOCFG_23_2 (DWC_ufshc_block_BaseAddress + 0x1b88)
#define CRYPTOCFG_23_2_RegisterSize 32
#define CRYPTOCFG_23_2_RegisterResetValue 0x0
#define CRYPTOCFG_23_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_2_CRYPTOKEY_23_2_BitAddressOffset 0
#define CRYPTOCFG_23_2_CRYPTOKEY_23_2_RegisterSize 32





#define CRYPTOCFG_23_3 (DWC_ufshc_block_BaseAddress + 0x1b8c)
#define CRYPTOCFG_23_3_RegisterSize 32
#define CRYPTOCFG_23_3_RegisterResetValue 0x0
#define CRYPTOCFG_23_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_3_CRYPTOKEY_23_3_BitAddressOffset 0
#define CRYPTOCFG_23_3_CRYPTOKEY_23_3_RegisterSize 32





#define CRYPTOCFG_23_4 (DWC_ufshc_block_BaseAddress + 0x1b90)
#define CRYPTOCFG_23_4_RegisterSize 32
#define CRYPTOCFG_23_4_RegisterResetValue 0x0
#define CRYPTOCFG_23_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_4_CRYPTOKEY_23_4_BitAddressOffset 0
#define CRYPTOCFG_23_4_CRYPTOKEY_23_4_RegisterSize 32





#define CRYPTOCFG_23_5 (DWC_ufshc_block_BaseAddress + 0x1b94)
#define CRYPTOCFG_23_5_RegisterSize 32
#define CRYPTOCFG_23_5_RegisterResetValue 0x0
#define CRYPTOCFG_23_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_5_CRYPTOKEY_23_5_BitAddressOffset 0
#define CRYPTOCFG_23_5_CRYPTOKEY_23_5_RegisterSize 32





#define CRYPTOCFG_23_6 (DWC_ufshc_block_BaseAddress + 0x1b98)
#define CRYPTOCFG_23_6_RegisterSize 32
#define CRYPTOCFG_23_6_RegisterResetValue 0x0
#define CRYPTOCFG_23_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_6_CRYPTOKEY_23_6_BitAddressOffset 0
#define CRYPTOCFG_23_6_CRYPTOKEY_23_6_RegisterSize 32





#define CRYPTOCFG_23_7 (DWC_ufshc_block_BaseAddress + 0x1b9c)
#define CRYPTOCFG_23_7_RegisterSize 32
#define CRYPTOCFG_23_7_RegisterResetValue 0x0
#define CRYPTOCFG_23_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_7_CRYPTOKEY_23_7_BitAddressOffset 0
#define CRYPTOCFG_23_7_CRYPTOKEY_23_7_RegisterSize 32





#define CRYPTOCFG_23_8 (DWC_ufshc_block_BaseAddress + 0x1ba0)
#define CRYPTOCFG_23_8_RegisterSize 32
#define CRYPTOCFG_23_8_RegisterResetValue 0x0
#define CRYPTOCFG_23_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_8_CRYPTOKEY_23_8_BitAddressOffset 0
#define CRYPTOCFG_23_8_CRYPTOKEY_23_8_RegisterSize 32





#define CRYPTOCFG_23_9 (DWC_ufshc_block_BaseAddress + 0x1ba4)
#define CRYPTOCFG_23_9_RegisterSize 32
#define CRYPTOCFG_23_9_RegisterResetValue 0x0
#define CRYPTOCFG_23_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_9_CRYPTOKEY_23_9_BitAddressOffset 0
#define CRYPTOCFG_23_9_CRYPTOKEY_23_9_RegisterSize 32





#define CRYPTOCFG_23_10 (DWC_ufshc_block_BaseAddress + 0x1ba8)
#define CRYPTOCFG_23_10_RegisterSize 32
#define CRYPTOCFG_23_10_RegisterResetValue 0x0
#define CRYPTOCFG_23_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_10_CRYPTOKEY_23_10_BitAddressOffset 0
#define CRYPTOCFG_23_10_CRYPTOKEY_23_10_RegisterSize 32





#define CRYPTOCFG_23_11 (DWC_ufshc_block_BaseAddress + 0x1bac)
#define CRYPTOCFG_23_11_RegisterSize 32
#define CRYPTOCFG_23_11_RegisterResetValue 0x0
#define CRYPTOCFG_23_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_11_CRYPTOKEY_23_11_BitAddressOffset 0
#define CRYPTOCFG_23_11_CRYPTOKEY_23_11_RegisterSize 32





#define CRYPTOCFG_23_12 (DWC_ufshc_block_BaseAddress + 0x1bb0)
#define CRYPTOCFG_23_12_RegisterSize 32
#define CRYPTOCFG_23_12_RegisterResetValue 0x0
#define CRYPTOCFG_23_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_12_CRYPTOKEY_23_12_BitAddressOffset 0
#define CRYPTOCFG_23_12_CRYPTOKEY_23_12_RegisterSize 32





#define CRYPTOCFG_23_13 (DWC_ufshc_block_BaseAddress + 0x1bb4)
#define CRYPTOCFG_23_13_RegisterSize 32
#define CRYPTOCFG_23_13_RegisterResetValue 0x0
#define CRYPTOCFG_23_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_13_CRYPTOKEY_23_13_BitAddressOffset 0
#define CRYPTOCFG_23_13_CRYPTOKEY_23_13_RegisterSize 32





#define CRYPTOCFG_23_14 (DWC_ufshc_block_BaseAddress + 0x1bb8)
#define CRYPTOCFG_23_14_RegisterSize 32
#define CRYPTOCFG_23_14_RegisterResetValue 0x0
#define CRYPTOCFG_23_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_14_CRYPTOKEY_23_14_BitAddressOffset 0
#define CRYPTOCFG_23_14_CRYPTOKEY_23_14_RegisterSize 32





#define CRYPTOCFG_23_15 (DWC_ufshc_block_BaseAddress + 0x1bbc)
#define CRYPTOCFG_23_15_RegisterSize 32
#define CRYPTOCFG_23_15_RegisterResetValue 0x0
#define CRYPTOCFG_23_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_15_CRYPTOKEY_23_15_BitAddressOffset 0
#define CRYPTOCFG_23_15_CRYPTOKEY_23_15_RegisterSize 32





#define CRYPTOCFG_23_16 (DWC_ufshc_block_BaseAddress + 0x1bc0)
#define CRYPTOCFG_23_16_RegisterSize 32
#define CRYPTOCFG_23_16_RegisterResetValue 0x0
#define CRYPTOCFG_23_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_16_DUSIZE_23_BitAddressOffset 0
#define CRYPTOCFG_23_16_DUSIZE_23_RegisterSize 8



#define CRYPTOCFG_23_16_CAPIDX_23_BitAddressOffset 8
#define CRYPTOCFG_23_16_CAPIDX_23_RegisterSize 8



#define CRYPTOCFG_23_16_CFGE_23_BitAddressOffset 31
#define CRYPTOCFG_23_16_CFGE_23_RegisterSize 1





#define CRYPTOCFG_23_17 (DWC_ufshc_block_BaseAddress + 0x1bc4)
#define CRYPTOCFG_23_17_RegisterSize 32
#define CRYPTOCFG_23_17_RegisterResetValue 0x0
#define CRYPTOCFG_23_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_17_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_17_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_18 (DWC_ufshc_block_BaseAddress + 0x1bc8)
#define CRYPTOCFG_23_18_RegisterSize 32
#define CRYPTOCFG_23_18_RegisterResetValue 0x0
#define CRYPTOCFG_23_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_18_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_18_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_19 (DWC_ufshc_block_BaseAddress + 0x1bcc)
#define CRYPTOCFG_23_19_RegisterSize 32
#define CRYPTOCFG_23_19_RegisterResetValue 0x0
#define CRYPTOCFG_23_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_19_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_19_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_20 (DWC_ufshc_block_BaseAddress + 0x1bd0)
#define CRYPTOCFG_23_20_RegisterSize 32
#define CRYPTOCFG_23_20_RegisterResetValue 0x0
#define CRYPTOCFG_23_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_20_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_20_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_21 (DWC_ufshc_block_BaseAddress + 0x1bd4)
#define CRYPTOCFG_23_21_RegisterSize 32
#define CRYPTOCFG_23_21_RegisterResetValue 0x0
#define CRYPTOCFG_23_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_21_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_21_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_22 (DWC_ufshc_block_BaseAddress + 0x1bd8)
#define CRYPTOCFG_23_22_RegisterSize 32
#define CRYPTOCFG_23_22_RegisterResetValue 0x0
#define CRYPTOCFG_23_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_22_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_22_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_23 (DWC_ufshc_block_BaseAddress + 0x1bdc)
#define CRYPTOCFG_23_23_RegisterSize 32
#define CRYPTOCFG_23_23_RegisterResetValue 0x0
#define CRYPTOCFG_23_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_23_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_23_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_24 (DWC_ufshc_block_BaseAddress + 0x1be0)
#define CRYPTOCFG_23_24_RegisterSize 32
#define CRYPTOCFG_23_24_RegisterResetValue 0x0
#define CRYPTOCFG_23_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_24_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_24_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_25 (DWC_ufshc_block_BaseAddress + 0x1be4)
#define CRYPTOCFG_23_25_RegisterSize 32
#define CRYPTOCFG_23_25_RegisterResetValue 0x0
#define CRYPTOCFG_23_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_25_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_25_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_26 (DWC_ufshc_block_BaseAddress + 0x1be8)
#define CRYPTOCFG_23_26_RegisterSize 32
#define CRYPTOCFG_23_26_RegisterResetValue 0x0
#define CRYPTOCFG_23_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_26_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_26_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_27 (DWC_ufshc_block_BaseAddress + 0x1bec)
#define CRYPTOCFG_23_27_RegisterSize 32
#define CRYPTOCFG_23_27_RegisterResetValue 0x0
#define CRYPTOCFG_23_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_27_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_27_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_28 (DWC_ufshc_block_BaseAddress + 0x1bf0)
#define CRYPTOCFG_23_28_RegisterSize 32
#define CRYPTOCFG_23_28_RegisterResetValue 0x0
#define CRYPTOCFG_23_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_28_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_28_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_29 (DWC_ufshc_block_BaseAddress + 0x1bf4)
#define CRYPTOCFG_23_29_RegisterSize 32
#define CRYPTOCFG_23_29_RegisterResetValue 0x0
#define CRYPTOCFG_23_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_29_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_29_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_30 (DWC_ufshc_block_BaseAddress + 0x1bf8)
#define CRYPTOCFG_23_30_RegisterSize 32
#define CRYPTOCFG_23_30_RegisterResetValue 0x0
#define CRYPTOCFG_23_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_30_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_30_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_23_31 (DWC_ufshc_block_BaseAddress + 0x1bfc)
#define CRYPTOCFG_23_31_RegisterSize 32
#define CRYPTOCFG_23_31_RegisterResetValue 0x0
#define CRYPTOCFG_23_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_23_31_RESERVED_23_BitAddressOffset 0
#define CRYPTOCFG_23_31_RESERVED_23_RegisterSize 32





#define CRYPTOCFG_24_0 (DWC_ufshc_block_BaseAddress + 0x1c00)
#define CRYPTOCFG_24_0_RegisterSize 32
#define CRYPTOCFG_24_0_RegisterResetValue 0x0
#define CRYPTOCFG_24_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_0_CRYPTOKEY_24_0_BitAddressOffset 0
#define CRYPTOCFG_24_0_CRYPTOKEY_24_0_RegisterSize 32





#define CRYPTOCFG_24_1 (DWC_ufshc_block_BaseAddress + 0x1c04)
#define CRYPTOCFG_24_1_RegisterSize 32
#define CRYPTOCFG_24_1_RegisterResetValue 0x0
#define CRYPTOCFG_24_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_1_CRYPTOKEY_24_1_BitAddressOffset 0
#define CRYPTOCFG_24_1_CRYPTOKEY_24_1_RegisterSize 32





#define CRYPTOCFG_24_2 (DWC_ufshc_block_BaseAddress + 0x1c08)
#define CRYPTOCFG_24_2_RegisterSize 32
#define CRYPTOCFG_24_2_RegisterResetValue 0x0
#define CRYPTOCFG_24_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_2_CRYPTOKEY_24_2_BitAddressOffset 0
#define CRYPTOCFG_24_2_CRYPTOKEY_24_2_RegisterSize 32





#define CRYPTOCFG_24_3 (DWC_ufshc_block_BaseAddress + 0x1c0c)
#define CRYPTOCFG_24_3_RegisterSize 32
#define CRYPTOCFG_24_3_RegisterResetValue 0x0
#define CRYPTOCFG_24_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_3_CRYPTOKEY_24_3_BitAddressOffset 0
#define CRYPTOCFG_24_3_CRYPTOKEY_24_3_RegisterSize 32





#define CRYPTOCFG_24_4 (DWC_ufshc_block_BaseAddress + 0x1c10)
#define CRYPTOCFG_24_4_RegisterSize 32
#define CRYPTOCFG_24_4_RegisterResetValue 0x0
#define CRYPTOCFG_24_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_4_CRYPTOKEY_24_4_BitAddressOffset 0
#define CRYPTOCFG_24_4_CRYPTOKEY_24_4_RegisterSize 32





#define CRYPTOCFG_24_5 (DWC_ufshc_block_BaseAddress + 0x1c14)
#define CRYPTOCFG_24_5_RegisterSize 32
#define CRYPTOCFG_24_5_RegisterResetValue 0x0
#define CRYPTOCFG_24_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_5_CRYPTOKEY_24_5_BitAddressOffset 0
#define CRYPTOCFG_24_5_CRYPTOKEY_24_5_RegisterSize 32





#define CRYPTOCFG_24_6 (DWC_ufshc_block_BaseAddress + 0x1c18)
#define CRYPTOCFG_24_6_RegisterSize 32
#define CRYPTOCFG_24_6_RegisterResetValue 0x0
#define CRYPTOCFG_24_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_6_CRYPTOKEY_24_6_BitAddressOffset 0
#define CRYPTOCFG_24_6_CRYPTOKEY_24_6_RegisterSize 32





#define CRYPTOCFG_24_7 (DWC_ufshc_block_BaseAddress + 0x1c1c)
#define CRYPTOCFG_24_7_RegisterSize 32
#define CRYPTOCFG_24_7_RegisterResetValue 0x0
#define CRYPTOCFG_24_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_7_CRYPTOKEY_24_7_BitAddressOffset 0
#define CRYPTOCFG_24_7_CRYPTOKEY_24_7_RegisterSize 32





#define CRYPTOCFG_24_8 (DWC_ufshc_block_BaseAddress + 0x1c20)
#define CRYPTOCFG_24_8_RegisterSize 32
#define CRYPTOCFG_24_8_RegisterResetValue 0x0
#define CRYPTOCFG_24_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_8_CRYPTOKEY_24_8_BitAddressOffset 0
#define CRYPTOCFG_24_8_CRYPTOKEY_24_8_RegisterSize 32





#define CRYPTOCFG_24_9 (DWC_ufshc_block_BaseAddress + 0x1c24)
#define CRYPTOCFG_24_9_RegisterSize 32
#define CRYPTOCFG_24_9_RegisterResetValue 0x0
#define CRYPTOCFG_24_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_9_CRYPTOKEY_24_9_BitAddressOffset 0
#define CRYPTOCFG_24_9_CRYPTOKEY_24_9_RegisterSize 32





#define CRYPTOCFG_24_10 (DWC_ufshc_block_BaseAddress + 0x1c28)
#define CRYPTOCFG_24_10_RegisterSize 32
#define CRYPTOCFG_24_10_RegisterResetValue 0x0
#define CRYPTOCFG_24_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_10_CRYPTOKEY_24_10_BitAddressOffset 0
#define CRYPTOCFG_24_10_CRYPTOKEY_24_10_RegisterSize 32





#define CRYPTOCFG_24_11 (DWC_ufshc_block_BaseAddress + 0x1c2c)
#define CRYPTOCFG_24_11_RegisterSize 32
#define CRYPTOCFG_24_11_RegisterResetValue 0x0
#define CRYPTOCFG_24_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_11_CRYPTOKEY_24_11_BitAddressOffset 0
#define CRYPTOCFG_24_11_CRYPTOKEY_24_11_RegisterSize 32





#define CRYPTOCFG_24_12 (DWC_ufshc_block_BaseAddress + 0x1c30)
#define CRYPTOCFG_24_12_RegisterSize 32
#define CRYPTOCFG_24_12_RegisterResetValue 0x0
#define CRYPTOCFG_24_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_12_CRYPTOKEY_24_12_BitAddressOffset 0
#define CRYPTOCFG_24_12_CRYPTOKEY_24_12_RegisterSize 32





#define CRYPTOCFG_24_13 (DWC_ufshc_block_BaseAddress + 0x1c34)
#define CRYPTOCFG_24_13_RegisterSize 32
#define CRYPTOCFG_24_13_RegisterResetValue 0x0
#define CRYPTOCFG_24_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_13_CRYPTOKEY_24_13_BitAddressOffset 0
#define CRYPTOCFG_24_13_CRYPTOKEY_24_13_RegisterSize 32





#define CRYPTOCFG_24_14 (DWC_ufshc_block_BaseAddress + 0x1c38)
#define CRYPTOCFG_24_14_RegisterSize 32
#define CRYPTOCFG_24_14_RegisterResetValue 0x0
#define CRYPTOCFG_24_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_14_CRYPTOKEY_24_14_BitAddressOffset 0
#define CRYPTOCFG_24_14_CRYPTOKEY_24_14_RegisterSize 32





#define CRYPTOCFG_24_15 (DWC_ufshc_block_BaseAddress + 0x1c3c)
#define CRYPTOCFG_24_15_RegisterSize 32
#define CRYPTOCFG_24_15_RegisterResetValue 0x0
#define CRYPTOCFG_24_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_15_CRYPTOKEY_24_15_BitAddressOffset 0
#define CRYPTOCFG_24_15_CRYPTOKEY_24_15_RegisterSize 32





#define CRYPTOCFG_24_16 (DWC_ufshc_block_BaseAddress + 0x1c40)
#define CRYPTOCFG_24_16_RegisterSize 32
#define CRYPTOCFG_24_16_RegisterResetValue 0x0
#define CRYPTOCFG_24_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_16_DUSIZE_24_BitAddressOffset 0
#define CRYPTOCFG_24_16_DUSIZE_24_RegisterSize 8



#define CRYPTOCFG_24_16_CAPIDX_24_BitAddressOffset 8
#define CRYPTOCFG_24_16_CAPIDX_24_RegisterSize 8



#define CRYPTOCFG_24_16_CFGE_24_BitAddressOffset 31
#define CRYPTOCFG_24_16_CFGE_24_RegisterSize 1





#define CRYPTOCFG_24_17 (DWC_ufshc_block_BaseAddress + 0x1c44)
#define CRYPTOCFG_24_17_RegisterSize 32
#define CRYPTOCFG_24_17_RegisterResetValue 0x0
#define CRYPTOCFG_24_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_17_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_17_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_18 (DWC_ufshc_block_BaseAddress + 0x1c48)
#define CRYPTOCFG_24_18_RegisterSize 32
#define CRYPTOCFG_24_18_RegisterResetValue 0x0
#define CRYPTOCFG_24_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_18_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_18_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_19 (DWC_ufshc_block_BaseAddress + 0x1c4c)
#define CRYPTOCFG_24_19_RegisterSize 32
#define CRYPTOCFG_24_19_RegisterResetValue 0x0
#define CRYPTOCFG_24_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_19_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_19_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_20 (DWC_ufshc_block_BaseAddress + 0x1c50)
#define CRYPTOCFG_24_20_RegisterSize 32
#define CRYPTOCFG_24_20_RegisterResetValue 0x0
#define CRYPTOCFG_24_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_20_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_20_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_21 (DWC_ufshc_block_BaseAddress + 0x1c54)
#define CRYPTOCFG_24_21_RegisterSize 32
#define CRYPTOCFG_24_21_RegisterResetValue 0x0
#define CRYPTOCFG_24_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_21_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_21_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_22 (DWC_ufshc_block_BaseAddress + 0x1c58)
#define CRYPTOCFG_24_22_RegisterSize 32
#define CRYPTOCFG_24_22_RegisterResetValue 0x0
#define CRYPTOCFG_24_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_22_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_22_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_23 (DWC_ufshc_block_BaseAddress + 0x1c5c)
#define CRYPTOCFG_24_23_RegisterSize 32
#define CRYPTOCFG_24_23_RegisterResetValue 0x0
#define CRYPTOCFG_24_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_23_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_23_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_24 (DWC_ufshc_block_BaseAddress + 0x1c60)
#define CRYPTOCFG_24_24_RegisterSize 32
#define CRYPTOCFG_24_24_RegisterResetValue 0x0
#define CRYPTOCFG_24_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_24_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_24_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_25 (DWC_ufshc_block_BaseAddress + 0x1c64)
#define CRYPTOCFG_24_25_RegisterSize 32
#define CRYPTOCFG_24_25_RegisterResetValue 0x0
#define CRYPTOCFG_24_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_25_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_25_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_26 (DWC_ufshc_block_BaseAddress + 0x1c68)
#define CRYPTOCFG_24_26_RegisterSize 32
#define CRYPTOCFG_24_26_RegisterResetValue 0x0
#define CRYPTOCFG_24_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_26_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_26_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_27 (DWC_ufshc_block_BaseAddress + 0x1c6c)
#define CRYPTOCFG_24_27_RegisterSize 32
#define CRYPTOCFG_24_27_RegisterResetValue 0x0
#define CRYPTOCFG_24_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_27_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_27_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_28 (DWC_ufshc_block_BaseAddress + 0x1c70)
#define CRYPTOCFG_24_28_RegisterSize 32
#define CRYPTOCFG_24_28_RegisterResetValue 0x0
#define CRYPTOCFG_24_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_28_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_28_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_29 (DWC_ufshc_block_BaseAddress + 0x1c74)
#define CRYPTOCFG_24_29_RegisterSize 32
#define CRYPTOCFG_24_29_RegisterResetValue 0x0
#define CRYPTOCFG_24_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_29_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_29_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_30 (DWC_ufshc_block_BaseAddress + 0x1c78)
#define CRYPTOCFG_24_30_RegisterSize 32
#define CRYPTOCFG_24_30_RegisterResetValue 0x0
#define CRYPTOCFG_24_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_30_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_30_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_24_31 (DWC_ufshc_block_BaseAddress + 0x1c7c)
#define CRYPTOCFG_24_31_RegisterSize 32
#define CRYPTOCFG_24_31_RegisterResetValue 0x0
#define CRYPTOCFG_24_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_24_31_RESERVED_24_BitAddressOffset 0
#define CRYPTOCFG_24_31_RESERVED_24_RegisterSize 32





#define CRYPTOCFG_25_0 (DWC_ufshc_block_BaseAddress + 0x1c80)
#define CRYPTOCFG_25_0_RegisterSize 32
#define CRYPTOCFG_25_0_RegisterResetValue 0x0
#define CRYPTOCFG_25_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_0_CRYPTOKEY_25_0_BitAddressOffset 0
#define CRYPTOCFG_25_0_CRYPTOKEY_25_0_RegisterSize 32





#define CRYPTOCFG_25_1 (DWC_ufshc_block_BaseAddress + 0x1c84)
#define CRYPTOCFG_25_1_RegisterSize 32
#define CRYPTOCFG_25_1_RegisterResetValue 0x0
#define CRYPTOCFG_25_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_1_CRYPTOKEY_25_1_BitAddressOffset 0
#define CRYPTOCFG_25_1_CRYPTOKEY_25_1_RegisterSize 32





#define CRYPTOCFG_25_2 (DWC_ufshc_block_BaseAddress + 0x1c88)
#define CRYPTOCFG_25_2_RegisterSize 32
#define CRYPTOCFG_25_2_RegisterResetValue 0x0
#define CRYPTOCFG_25_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_2_CRYPTOKEY_25_2_BitAddressOffset 0
#define CRYPTOCFG_25_2_CRYPTOKEY_25_2_RegisterSize 32





#define CRYPTOCFG_25_3 (DWC_ufshc_block_BaseAddress + 0x1c8c)
#define CRYPTOCFG_25_3_RegisterSize 32
#define CRYPTOCFG_25_3_RegisterResetValue 0x0
#define CRYPTOCFG_25_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_3_CRYPTOKEY_25_3_BitAddressOffset 0
#define CRYPTOCFG_25_3_CRYPTOKEY_25_3_RegisterSize 32





#define CRYPTOCFG_25_4 (DWC_ufshc_block_BaseAddress + 0x1c90)
#define CRYPTOCFG_25_4_RegisterSize 32
#define CRYPTOCFG_25_4_RegisterResetValue 0x0
#define CRYPTOCFG_25_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_4_CRYPTOKEY_25_4_BitAddressOffset 0
#define CRYPTOCFG_25_4_CRYPTOKEY_25_4_RegisterSize 32





#define CRYPTOCFG_25_5 (DWC_ufshc_block_BaseAddress + 0x1c94)
#define CRYPTOCFG_25_5_RegisterSize 32
#define CRYPTOCFG_25_5_RegisterResetValue 0x0
#define CRYPTOCFG_25_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_5_CRYPTOKEY_25_5_BitAddressOffset 0
#define CRYPTOCFG_25_5_CRYPTOKEY_25_5_RegisterSize 32





#define CRYPTOCFG_25_6 (DWC_ufshc_block_BaseAddress + 0x1c98)
#define CRYPTOCFG_25_6_RegisterSize 32
#define CRYPTOCFG_25_6_RegisterResetValue 0x0
#define CRYPTOCFG_25_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_6_CRYPTOKEY_25_6_BitAddressOffset 0
#define CRYPTOCFG_25_6_CRYPTOKEY_25_6_RegisterSize 32





#define CRYPTOCFG_25_7 (DWC_ufshc_block_BaseAddress + 0x1c9c)
#define CRYPTOCFG_25_7_RegisterSize 32
#define CRYPTOCFG_25_7_RegisterResetValue 0x0
#define CRYPTOCFG_25_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_7_CRYPTOKEY_25_7_BitAddressOffset 0
#define CRYPTOCFG_25_7_CRYPTOKEY_25_7_RegisterSize 32





#define CRYPTOCFG_25_8 (DWC_ufshc_block_BaseAddress + 0x1ca0)
#define CRYPTOCFG_25_8_RegisterSize 32
#define CRYPTOCFG_25_8_RegisterResetValue 0x0
#define CRYPTOCFG_25_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_8_CRYPTOKEY_25_8_BitAddressOffset 0
#define CRYPTOCFG_25_8_CRYPTOKEY_25_8_RegisterSize 32





#define CRYPTOCFG_25_9 (DWC_ufshc_block_BaseAddress + 0x1ca4)
#define CRYPTOCFG_25_9_RegisterSize 32
#define CRYPTOCFG_25_9_RegisterResetValue 0x0
#define CRYPTOCFG_25_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_9_CRYPTOKEY_25_9_BitAddressOffset 0
#define CRYPTOCFG_25_9_CRYPTOKEY_25_9_RegisterSize 32





#define CRYPTOCFG_25_10 (DWC_ufshc_block_BaseAddress + 0x1ca8)
#define CRYPTOCFG_25_10_RegisterSize 32
#define CRYPTOCFG_25_10_RegisterResetValue 0x0
#define CRYPTOCFG_25_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_10_CRYPTOKEY_25_10_BitAddressOffset 0
#define CRYPTOCFG_25_10_CRYPTOKEY_25_10_RegisterSize 32





#define CRYPTOCFG_25_11 (DWC_ufshc_block_BaseAddress + 0x1cac)
#define CRYPTOCFG_25_11_RegisterSize 32
#define CRYPTOCFG_25_11_RegisterResetValue 0x0
#define CRYPTOCFG_25_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_11_CRYPTOKEY_25_11_BitAddressOffset 0
#define CRYPTOCFG_25_11_CRYPTOKEY_25_11_RegisterSize 32





#define CRYPTOCFG_25_12 (DWC_ufshc_block_BaseAddress + 0x1cb0)
#define CRYPTOCFG_25_12_RegisterSize 32
#define CRYPTOCFG_25_12_RegisterResetValue 0x0
#define CRYPTOCFG_25_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_12_CRYPTOKEY_25_12_BitAddressOffset 0
#define CRYPTOCFG_25_12_CRYPTOKEY_25_12_RegisterSize 32





#define CRYPTOCFG_25_13 (DWC_ufshc_block_BaseAddress + 0x1cb4)
#define CRYPTOCFG_25_13_RegisterSize 32
#define CRYPTOCFG_25_13_RegisterResetValue 0x0
#define CRYPTOCFG_25_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_13_CRYPTOKEY_25_13_BitAddressOffset 0
#define CRYPTOCFG_25_13_CRYPTOKEY_25_13_RegisterSize 32





#define CRYPTOCFG_25_14 (DWC_ufshc_block_BaseAddress + 0x1cb8)
#define CRYPTOCFG_25_14_RegisterSize 32
#define CRYPTOCFG_25_14_RegisterResetValue 0x0
#define CRYPTOCFG_25_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_14_CRYPTOKEY_25_14_BitAddressOffset 0
#define CRYPTOCFG_25_14_CRYPTOKEY_25_14_RegisterSize 32





#define CRYPTOCFG_25_15 (DWC_ufshc_block_BaseAddress + 0x1cbc)
#define CRYPTOCFG_25_15_RegisterSize 32
#define CRYPTOCFG_25_15_RegisterResetValue 0x0
#define CRYPTOCFG_25_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_15_CRYPTOKEY_25_15_BitAddressOffset 0
#define CRYPTOCFG_25_15_CRYPTOKEY_25_15_RegisterSize 32





#define CRYPTOCFG_25_16 (DWC_ufshc_block_BaseAddress + 0x1cc0)
#define CRYPTOCFG_25_16_RegisterSize 32
#define CRYPTOCFG_25_16_RegisterResetValue 0x0
#define CRYPTOCFG_25_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_16_DUSIZE_25_BitAddressOffset 0
#define CRYPTOCFG_25_16_DUSIZE_25_RegisterSize 8



#define CRYPTOCFG_25_16_CAPIDX_25_BitAddressOffset 8
#define CRYPTOCFG_25_16_CAPIDX_25_RegisterSize 8



#define CRYPTOCFG_25_16_CFGE_25_BitAddressOffset 31
#define CRYPTOCFG_25_16_CFGE_25_RegisterSize 1





#define CRYPTOCFG_25_17 (DWC_ufshc_block_BaseAddress + 0x1cc4)
#define CRYPTOCFG_25_17_RegisterSize 32
#define CRYPTOCFG_25_17_RegisterResetValue 0x0
#define CRYPTOCFG_25_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_17_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_17_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_18 (DWC_ufshc_block_BaseAddress + 0x1cc8)
#define CRYPTOCFG_25_18_RegisterSize 32
#define CRYPTOCFG_25_18_RegisterResetValue 0x0
#define CRYPTOCFG_25_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_18_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_18_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_19 (DWC_ufshc_block_BaseAddress + 0x1ccc)
#define CRYPTOCFG_25_19_RegisterSize 32
#define CRYPTOCFG_25_19_RegisterResetValue 0x0
#define CRYPTOCFG_25_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_19_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_19_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_20 (DWC_ufshc_block_BaseAddress + 0x1cd0)
#define CRYPTOCFG_25_20_RegisterSize 32
#define CRYPTOCFG_25_20_RegisterResetValue 0x0
#define CRYPTOCFG_25_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_20_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_20_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_21 (DWC_ufshc_block_BaseAddress + 0x1cd4)
#define CRYPTOCFG_25_21_RegisterSize 32
#define CRYPTOCFG_25_21_RegisterResetValue 0x0
#define CRYPTOCFG_25_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_21_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_21_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_22 (DWC_ufshc_block_BaseAddress + 0x1cd8)
#define CRYPTOCFG_25_22_RegisterSize 32
#define CRYPTOCFG_25_22_RegisterResetValue 0x0
#define CRYPTOCFG_25_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_22_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_22_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_23 (DWC_ufshc_block_BaseAddress + 0x1cdc)
#define CRYPTOCFG_25_23_RegisterSize 32
#define CRYPTOCFG_25_23_RegisterResetValue 0x0
#define CRYPTOCFG_25_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_23_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_23_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_24 (DWC_ufshc_block_BaseAddress + 0x1ce0)
#define CRYPTOCFG_25_24_RegisterSize 32
#define CRYPTOCFG_25_24_RegisterResetValue 0x0
#define CRYPTOCFG_25_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_24_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_24_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_25 (DWC_ufshc_block_BaseAddress + 0x1ce4)
#define CRYPTOCFG_25_25_RegisterSize 32
#define CRYPTOCFG_25_25_RegisterResetValue 0x0
#define CRYPTOCFG_25_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_25_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_25_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_26 (DWC_ufshc_block_BaseAddress + 0x1ce8)
#define CRYPTOCFG_25_26_RegisterSize 32
#define CRYPTOCFG_25_26_RegisterResetValue 0x0
#define CRYPTOCFG_25_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_26_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_26_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_27 (DWC_ufshc_block_BaseAddress + 0x1cec)
#define CRYPTOCFG_25_27_RegisterSize 32
#define CRYPTOCFG_25_27_RegisterResetValue 0x0
#define CRYPTOCFG_25_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_27_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_27_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_28 (DWC_ufshc_block_BaseAddress + 0x1cf0)
#define CRYPTOCFG_25_28_RegisterSize 32
#define CRYPTOCFG_25_28_RegisterResetValue 0x0
#define CRYPTOCFG_25_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_28_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_28_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_29 (DWC_ufshc_block_BaseAddress + 0x1cf4)
#define CRYPTOCFG_25_29_RegisterSize 32
#define CRYPTOCFG_25_29_RegisterResetValue 0x0
#define CRYPTOCFG_25_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_29_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_29_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_30 (DWC_ufshc_block_BaseAddress + 0x1cf8)
#define CRYPTOCFG_25_30_RegisterSize 32
#define CRYPTOCFG_25_30_RegisterResetValue 0x0
#define CRYPTOCFG_25_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_30_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_30_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_25_31 (DWC_ufshc_block_BaseAddress + 0x1cfc)
#define CRYPTOCFG_25_31_RegisterSize 32
#define CRYPTOCFG_25_31_RegisterResetValue 0x0
#define CRYPTOCFG_25_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_25_31_RESERVED_25_BitAddressOffset 0
#define CRYPTOCFG_25_31_RESERVED_25_RegisterSize 32





#define CRYPTOCFG_26_0 (DWC_ufshc_block_BaseAddress + 0x1d00)
#define CRYPTOCFG_26_0_RegisterSize 32
#define CRYPTOCFG_26_0_RegisterResetValue 0x0
#define CRYPTOCFG_26_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_0_CRYPTOKEY_26_0_BitAddressOffset 0
#define CRYPTOCFG_26_0_CRYPTOKEY_26_0_RegisterSize 32





#define CRYPTOCFG_26_1 (DWC_ufshc_block_BaseAddress + 0x1d04)
#define CRYPTOCFG_26_1_RegisterSize 32
#define CRYPTOCFG_26_1_RegisterResetValue 0x0
#define CRYPTOCFG_26_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_1_CRYPTOKEY_26_1_BitAddressOffset 0
#define CRYPTOCFG_26_1_CRYPTOKEY_26_1_RegisterSize 32





#define CRYPTOCFG_26_2 (DWC_ufshc_block_BaseAddress + 0x1d08)
#define CRYPTOCFG_26_2_RegisterSize 32
#define CRYPTOCFG_26_2_RegisterResetValue 0x0
#define CRYPTOCFG_26_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_2_CRYPTOKEY_26_2_BitAddressOffset 0
#define CRYPTOCFG_26_2_CRYPTOKEY_26_2_RegisterSize 32





#define CRYPTOCFG_26_3 (DWC_ufshc_block_BaseAddress + 0x1d0c)
#define CRYPTOCFG_26_3_RegisterSize 32
#define CRYPTOCFG_26_3_RegisterResetValue 0x0
#define CRYPTOCFG_26_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_3_CRYPTOKEY_26_3_BitAddressOffset 0
#define CRYPTOCFG_26_3_CRYPTOKEY_26_3_RegisterSize 32





#define CRYPTOCFG_26_4 (DWC_ufshc_block_BaseAddress + 0x1d10)
#define CRYPTOCFG_26_4_RegisterSize 32
#define CRYPTOCFG_26_4_RegisterResetValue 0x0
#define CRYPTOCFG_26_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_4_CRYPTOKEY_26_4_BitAddressOffset 0
#define CRYPTOCFG_26_4_CRYPTOKEY_26_4_RegisterSize 32





#define CRYPTOCFG_26_5 (DWC_ufshc_block_BaseAddress + 0x1d14)
#define CRYPTOCFG_26_5_RegisterSize 32
#define CRYPTOCFG_26_5_RegisterResetValue 0x0
#define CRYPTOCFG_26_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_5_CRYPTOKEY_26_5_BitAddressOffset 0
#define CRYPTOCFG_26_5_CRYPTOKEY_26_5_RegisterSize 32





#define CRYPTOCFG_26_6 (DWC_ufshc_block_BaseAddress + 0x1d18)
#define CRYPTOCFG_26_6_RegisterSize 32
#define CRYPTOCFG_26_6_RegisterResetValue 0x0
#define CRYPTOCFG_26_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_6_CRYPTOKEY_26_6_BitAddressOffset 0
#define CRYPTOCFG_26_6_CRYPTOKEY_26_6_RegisterSize 32





#define CRYPTOCFG_26_7 (DWC_ufshc_block_BaseAddress + 0x1d1c)
#define CRYPTOCFG_26_7_RegisterSize 32
#define CRYPTOCFG_26_7_RegisterResetValue 0x0
#define CRYPTOCFG_26_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_7_CRYPTOKEY_26_7_BitAddressOffset 0
#define CRYPTOCFG_26_7_CRYPTOKEY_26_7_RegisterSize 32





#define CRYPTOCFG_26_8 (DWC_ufshc_block_BaseAddress + 0x1d20)
#define CRYPTOCFG_26_8_RegisterSize 32
#define CRYPTOCFG_26_8_RegisterResetValue 0x0
#define CRYPTOCFG_26_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_8_CRYPTOKEY_26_8_BitAddressOffset 0
#define CRYPTOCFG_26_8_CRYPTOKEY_26_8_RegisterSize 32





#define CRYPTOCFG_26_9 (DWC_ufshc_block_BaseAddress + 0x1d24)
#define CRYPTOCFG_26_9_RegisterSize 32
#define CRYPTOCFG_26_9_RegisterResetValue 0x0
#define CRYPTOCFG_26_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_9_CRYPTOKEY_26_9_BitAddressOffset 0
#define CRYPTOCFG_26_9_CRYPTOKEY_26_9_RegisterSize 32





#define CRYPTOCFG_26_10 (DWC_ufshc_block_BaseAddress + 0x1d28)
#define CRYPTOCFG_26_10_RegisterSize 32
#define CRYPTOCFG_26_10_RegisterResetValue 0x0
#define CRYPTOCFG_26_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_10_CRYPTOKEY_26_10_BitAddressOffset 0
#define CRYPTOCFG_26_10_CRYPTOKEY_26_10_RegisterSize 32





#define CRYPTOCFG_26_11 (DWC_ufshc_block_BaseAddress + 0x1d2c)
#define CRYPTOCFG_26_11_RegisterSize 32
#define CRYPTOCFG_26_11_RegisterResetValue 0x0
#define CRYPTOCFG_26_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_11_CRYPTOKEY_26_11_BitAddressOffset 0
#define CRYPTOCFG_26_11_CRYPTOKEY_26_11_RegisterSize 32





#define CRYPTOCFG_26_12 (DWC_ufshc_block_BaseAddress + 0x1d30)
#define CRYPTOCFG_26_12_RegisterSize 32
#define CRYPTOCFG_26_12_RegisterResetValue 0x0
#define CRYPTOCFG_26_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_12_CRYPTOKEY_26_12_BitAddressOffset 0
#define CRYPTOCFG_26_12_CRYPTOKEY_26_12_RegisterSize 32





#define CRYPTOCFG_26_13 (DWC_ufshc_block_BaseAddress + 0x1d34)
#define CRYPTOCFG_26_13_RegisterSize 32
#define CRYPTOCFG_26_13_RegisterResetValue 0x0
#define CRYPTOCFG_26_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_13_CRYPTOKEY_26_13_BitAddressOffset 0
#define CRYPTOCFG_26_13_CRYPTOKEY_26_13_RegisterSize 32





#define CRYPTOCFG_26_14 (DWC_ufshc_block_BaseAddress + 0x1d38)
#define CRYPTOCFG_26_14_RegisterSize 32
#define CRYPTOCFG_26_14_RegisterResetValue 0x0
#define CRYPTOCFG_26_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_14_CRYPTOKEY_26_14_BitAddressOffset 0
#define CRYPTOCFG_26_14_CRYPTOKEY_26_14_RegisterSize 32





#define CRYPTOCFG_26_15 (DWC_ufshc_block_BaseAddress + 0x1d3c)
#define CRYPTOCFG_26_15_RegisterSize 32
#define CRYPTOCFG_26_15_RegisterResetValue 0x0
#define CRYPTOCFG_26_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_15_CRYPTOKEY_26_15_BitAddressOffset 0
#define CRYPTOCFG_26_15_CRYPTOKEY_26_15_RegisterSize 32





#define CRYPTOCFG_26_16 (DWC_ufshc_block_BaseAddress + 0x1d40)
#define CRYPTOCFG_26_16_RegisterSize 32
#define CRYPTOCFG_26_16_RegisterResetValue 0x0
#define CRYPTOCFG_26_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_16_DUSIZE_26_BitAddressOffset 0
#define CRYPTOCFG_26_16_DUSIZE_26_RegisterSize 8



#define CRYPTOCFG_26_16_CAPIDX_26_BitAddressOffset 8
#define CRYPTOCFG_26_16_CAPIDX_26_RegisterSize 8



#define CRYPTOCFG_26_16_CFGE_26_BitAddressOffset 31
#define CRYPTOCFG_26_16_CFGE_26_RegisterSize 1





#define CRYPTOCFG_26_17 (DWC_ufshc_block_BaseAddress + 0x1d44)
#define CRYPTOCFG_26_17_RegisterSize 32
#define CRYPTOCFG_26_17_RegisterResetValue 0x0
#define CRYPTOCFG_26_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_17_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_17_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_18 (DWC_ufshc_block_BaseAddress + 0x1d48)
#define CRYPTOCFG_26_18_RegisterSize 32
#define CRYPTOCFG_26_18_RegisterResetValue 0x0
#define CRYPTOCFG_26_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_18_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_18_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_19 (DWC_ufshc_block_BaseAddress + 0x1d4c)
#define CRYPTOCFG_26_19_RegisterSize 32
#define CRYPTOCFG_26_19_RegisterResetValue 0x0
#define CRYPTOCFG_26_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_19_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_19_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_20 (DWC_ufshc_block_BaseAddress + 0x1d50)
#define CRYPTOCFG_26_20_RegisterSize 32
#define CRYPTOCFG_26_20_RegisterResetValue 0x0
#define CRYPTOCFG_26_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_20_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_20_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_21 (DWC_ufshc_block_BaseAddress + 0x1d54)
#define CRYPTOCFG_26_21_RegisterSize 32
#define CRYPTOCFG_26_21_RegisterResetValue 0x0
#define CRYPTOCFG_26_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_21_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_21_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_22 (DWC_ufshc_block_BaseAddress + 0x1d58)
#define CRYPTOCFG_26_22_RegisterSize 32
#define CRYPTOCFG_26_22_RegisterResetValue 0x0
#define CRYPTOCFG_26_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_22_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_22_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_23 (DWC_ufshc_block_BaseAddress + 0x1d5c)
#define CRYPTOCFG_26_23_RegisterSize 32
#define CRYPTOCFG_26_23_RegisterResetValue 0x0
#define CRYPTOCFG_26_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_23_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_23_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_24 (DWC_ufshc_block_BaseAddress + 0x1d60)
#define CRYPTOCFG_26_24_RegisterSize 32
#define CRYPTOCFG_26_24_RegisterResetValue 0x0
#define CRYPTOCFG_26_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_24_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_24_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_25 (DWC_ufshc_block_BaseAddress + 0x1d64)
#define CRYPTOCFG_26_25_RegisterSize 32
#define CRYPTOCFG_26_25_RegisterResetValue 0x0
#define CRYPTOCFG_26_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_25_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_25_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_26 (DWC_ufshc_block_BaseAddress + 0x1d68)
#define CRYPTOCFG_26_26_RegisterSize 32
#define CRYPTOCFG_26_26_RegisterResetValue 0x0
#define CRYPTOCFG_26_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_26_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_26_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_27 (DWC_ufshc_block_BaseAddress + 0x1d6c)
#define CRYPTOCFG_26_27_RegisterSize 32
#define CRYPTOCFG_26_27_RegisterResetValue 0x0
#define CRYPTOCFG_26_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_27_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_27_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_28 (DWC_ufshc_block_BaseAddress + 0x1d70)
#define CRYPTOCFG_26_28_RegisterSize 32
#define CRYPTOCFG_26_28_RegisterResetValue 0x0
#define CRYPTOCFG_26_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_28_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_28_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_29 (DWC_ufshc_block_BaseAddress + 0x1d74)
#define CRYPTOCFG_26_29_RegisterSize 32
#define CRYPTOCFG_26_29_RegisterResetValue 0x0
#define CRYPTOCFG_26_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_29_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_29_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_30 (DWC_ufshc_block_BaseAddress + 0x1d78)
#define CRYPTOCFG_26_30_RegisterSize 32
#define CRYPTOCFG_26_30_RegisterResetValue 0x0
#define CRYPTOCFG_26_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_30_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_30_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_26_31 (DWC_ufshc_block_BaseAddress + 0x1d7c)
#define CRYPTOCFG_26_31_RegisterSize 32
#define CRYPTOCFG_26_31_RegisterResetValue 0x0
#define CRYPTOCFG_26_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_26_31_RESERVED_26_BitAddressOffset 0
#define CRYPTOCFG_26_31_RESERVED_26_RegisterSize 32





#define CRYPTOCFG_27_0 (DWC_ufshc_block_BaseAddress + 0x1d80)
#define CRYPTOCFG_27_0_RegisterSize 32
#define CRYPTOCFG_27_0_RegisterResetValue 0x0
#define CRYPTOCFG_27_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_0_CRYPTOKEY_27_0_BitAddressOffset 0
#define CRYPTOCFG_27_0_CRYPTOKEY_27_0_RegisterSize 32





#define CRYPTOCFG_27_1 (DWC_ufshc_block_BaseAddress + 0x1d84)
#define CRYPTOCFG_27_1_RegisterSize 32
#define CRYPTOCFG_27_1_RegisterResetValue 0x0
#define CRYPTOCFG_27_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_1_CRYPTOKEY_27_1_BitAddressOffset 0
#define CRYPTOCFG_27_1_CRYPTOKEY_27_1_RegisterSize 32





#define CRYPTOCFG_27_2 (DWC_ufshc_block_BaseAddress + 0x1d88)
#define CRYPTOCFG_27_2_RegisterSize 32
#define CRYPTOCFG_27_2_RegisterResetValue 0x0
#define CRYPTOCFG_27_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_2_CRYPTOKEY_27_2_BitAddressOffset 0
#define CRYPTOCFG_27_2_CRYPTOKEY_27_2_RegisterSize 32





#define CRYPTOCFG_27_3 (DWC_ufshc_block_BaseAddress + 0x1d8c)
#define CRYPTOCFG_27_3_RegisterSize 32
#define CRYPTOCFG_27_3_RegisterResetValue 0x0
#define CRYPTOCFG_27_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_3_CRYPTOKEY_27_3_BitAddressOffset 0
#define CRYPTOCFG_27_3_CRYPTOKEY_27_3_RegisterSize 32





#define CRYPTOCFG_27_4 (DWC_ufshc_block_BaseAddress + 0x1d90)
#define CRYPTOCFG_27_4_RegisterSize 32
#define CRYPTOCFG_27_4_RegisterResetValue 0x0
#define CRYPTOCFG_27_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_4_CRYPTOKEY_27_4_BitAddressOffset 0
#define CRYPTOCFG_27_4_CRYPTOKEY_27_4_RegisterSize 32





#define CRYPTOCFG_27_5 (DWC_ufshc_block_BaseAddress + 0x1d94)
#define CRYPTOCFG_27_5_RegisterSize 32
#define CRYPTOCFG_27_5_RegisterResetValue 0x0
#define CRYPTOCFG_27_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_5_CRYPTOKEY_27_5_BitAddressOffset 0
#define CRYPTOCFG_27_5_CRYPTOKEY_27_5_RegisterSize 32





#define CRYPTOCFG_27_6 (DWC_ufshc_block_BaseAddress + 0x1d98)
#define CRYPTOCFG_27_6_RegisterSize 32
#define CRYPTOCFG_27_6_RegisterResetValue 0x0
#define CRYPTOCFG_27_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_6_CRYPTOKEY_27_6_BitAddressOffset 0
#define CRYPTOCFG_27_6_CRYPTOKEY_27_6_RegisterSize 32





#define CRYPTOCFG_27_7 (DWC_ufshc_block_BaseAddress + 0x1d9c)
#define CRYPTOCFG_27_7_RegisterSize 32
#define CRYPTOCFG_27_7_RegisterResetValue 0x0
#define CRYPTOCFG_27_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_7_CRYPTOKEY_27_7_BitAddressOffset 0
#define CRYPTOCFG_27_7_CRYPTOKEY_27_7_RegisterSize 32





#define CRYPTOCFG_27_8 (DWC_ufshc_block_BaseAddress + 0x1da0)
#define CRYPTOCFG_27_8_RegisterSize 32
#define CRYPTOCFG_27_8_RegisterResetValue 0x0
#define CRYPTOCFG_27_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_8_CRYPTOKEY_27_8_BitAddressOffset 0
#define CRYPTOCFG_27_8_CRYPTOKEY_27_8_RegisterSize 32





#define CRYPTOCFG_27_9 (DWC_ufshc_block_BaseAddress + 0x1da4)
#define CRYPTOCFG_27_9_RegisterSize 32
#define CRYPTOCFG_27_9_RegisterResetValue 0x0
#define CRYPTOCFG_27_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_9_CRYPTOKEY_27_9_BitAddressOffset 0
#define CRYPTOCFG_27_9_CRYPTOKEY_27_9_RegisterSize 32





#define CRYPTOCFG_27_10 (DWC_ufshc_block_BaseAddress + 0x1da8)
#define CRYPTOCFG_27_10_RegisterSize 32
#define CRYPTOCFG_27_10_RegisterResetValue 0x0
#define CRYPTOCFG_27_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_10_CRYPTOKEY_27_10_BitAddressOffset 0
#define CRYPTOCFG_27_10_CRYPTOKEY_27_10_RegisterSize 32





#define CRYPTOCFG_27_11 (DWC_ufshc_block_BaseAddress + 0x1dac)
#define CRYPTOCFG_27_11_RegisterSize 32
#define CRYPTOCFG_27_11_RegisterResetValue 0x0
#define CRYPTOCFG_27_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_11_CRYPTOKEY_27_11_BitAddressOffset 0
#define CRYPTOCFG_27_11_CRYPTOKEY_27_11_RegisterSize 32





#define CRYPTOCFG_27_12 (DWC_ufshc_block_BaseAddress + 0x1db0)
#define CRYPTOCFG_27_12_RegisterSize 32
#define CRYPTOCFG_27_12_RegisterResetValue 0x0
#define CRYPTOCFG_27_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_12_CRYPTOKEY_27_12_BitAddressOffset 0
#define CRYPTOCFG_27_12_CRYPTOKEY_27_12_RegisterSize 32





#define CRYPTOCFG_27_13 (DWC_ufshc_block_BaseAddress + 0x1db4)
#define CRYPTOCFG_27_13_RegisterSize 32
#define CRYPTOCFG_27_13_RegisterResetValue 0x0
#define CRYPTOCFG_27_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_13_CRYPTOKEY_27_13_BitAddressOffset 0
#define CRYPTOCFG_27_13_CRYPTOKEY_27_13_RegisterSize 32





#define CRYPTOCFG_27_14 (DWC_ufshc_block_BaseAddress + 0x1db8)
#define CRYPTOCFG_27_14_RegisterSize 32
#define CRYPTOCFG_27_14_RegisterResetValue 0x0
#define CRYPTOCFG_27_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_14_CRYPTOKEY_27_14_BitAddressOffset 0
#define CRYPTOCFG_27_14_CRYPTOKEY_27_14_RegisterSize 32





#define CRYPTOCFG_27_15 (DWC_ufshc_block_BaseAddress + 0x1dbc)
#define CRYPTOCFG_27_15_RegisterSize 32
#define CRYPTOCFG_27_15_RegisterResetValue 0x0
#define CRYPTOCFG_27_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_15_CRYPTOKEY_27_15_BitAddressOffset 0
#define CRYPTOCFG_27_15_CRYPTOKEY_27_15_RegisterSize 32





#define CRYPTOCFG_27_16 (DWC_ufshc_block_BaseAddress + 0x1dc0)
#define CRYPTOCFG_27_16_RegisterSize 32
#define CRYPTOCFG_27_16_RegisterResetValue 0x0
#define CRYPTOCFG_27_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_16_DUSIZE_27_BitAddressOffset 0
#define CRYPTOCFG_27_16_DUSIZE_27_RegisterSize 8



#define CRYPTOCFG_27_16_CAPIDX_27_BitAddressOffset 8
#define CRYPTOCFG_27_16_CAPIDX_27_RegisterSize 8



#define CRYPTOCFG_27_16_CFGE_27_BitAddressOffset 31
#define CRYPTOCFG_27_16_CFGE_27_RegisterSize 1





#define CRYPTOCFG_27_17 (DWC_ufshc_block_BaseAddress + 0x1dc4)
#define CRYPTOCFG_27_17_RegisterSize 32
#define CRYPTOCFG_27_17_RegisterResetValue 0x0
#define CRYPTOCFG_27_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_17_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_17_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_18 (DWC_ufshc_block_BaseAddress + 0x1dc8)
#define CRYPTOCFG_27_18_RegisterSize 32
#define CRYPTOCFG_27_18_RegisterResetValue 0x0
#define CRYPTOCFG_27_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_18_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_18_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_19 (DWC_ufshc_block_BaseAddress + 0x1dcc)
#define CRYPTOCFG_27_19_RegisterSize 32
#define CRYPTOCFG_27_19_RegisterResetValue 0x0
#define CRYPTOCFG_27_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_19_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_19_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_20 (DWC_ufshc_block_BaseAddress + 0x1dd0)
#define CRYPTOCFG_27_20_RegisterSize 32
#define CRYPTOCFG_27_20_RegisterResetValue 0x0
#define CRYPTOCFG_27_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_20_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_20_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_21 (DWC_ufshc_block_BaseAddress + 0x1dd4)
#define CRYPTOCFG_27_21_RegisterSize 32
#define CRYPTOCFG_27_21_RegisterResetValue 0x0
#define CRYPTOCFG_27_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_21_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_21_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_22 (DWC_ufshc_block_BaseAddress + 0x1dd8)
#define CRYPTOCFG_27_22_RegisterSize 32
#define CRYPTOCFG_27_22_RegisterResetValue 0x0
#define CRYPTOCFG_27_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_22_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_22_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_23 (DWC_ufshc_block_BaseAddress + 0x1ddc)
#define CRYPTOCFG_27_23_RegisterSize 32
#define CRYPTOCFG_27_23_RegisterResetValue 0x0
#define CRYPTOCFG_27_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_23_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_23_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_24 (DWC_ufshc_block_BaseAddress + 0x1de0)
#define CRYPTOCFG_27_24_RegisterSize 32
#define CRYPTOCFG_27_24_RegisterResetValue 0x0
#define CRYPTOCFG_27_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_24_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_24_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_25 (DWC_ufshc_block_BaseAddress + 0x1de4)
#define CRYPTOCFG_27_25_RegisterSize 32
#define CRYPTOCFG_27_25_RegisterResetValue 0x0
#define CRYPTOCFG_27_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_25_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_25_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_26 (DWC_ufshc_block_BaseAddress + 0x1de8)
#define CRYPTOCFG_27_26_RegisterSize 32
#define CRYPTOCFG_27_26_RegisterResetValue 0x0
#define CRYPTOCFG_27_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_26_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_26_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_27 (DWC_ufshc_block_BaseAddress + 0x1dec)
#define CRYPTOCFG_27_27_RegisterSize 32
#define CRYPTOCFG_27_27_RegisterResetValue 0x0
#define CRYPTOCFG_27_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_27_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_27_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_28 (DWC_ufshc_block_BaseAddress + 0x1df0)
#define CRYPTOCFG_27_28_RegisterSize 32
#define CRYPTOCFG_27_28_RegisterResetValue 0x0
#define CRYPTOCFG_27_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_28_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_28_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_29 (DWC_ufshc_block_BaseAddress + 0x1df4)
#define CRYPTOCFG_27_29_RegisterSize 32
#define CRYPTOCFG_27_29_RegisterResetValue 0x0
#define CRYPTOCFG_27_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_29_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_29_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_30 (DWC_ufshc_block_BaseAddress + 0x1df8)
#define CRYPTOCFG_27_30_RegisterSize 32
#define CRYPTOCFG_27_30_RegisterResetValue 0x0
#define CRYPTOCFG_27_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_30_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_30_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_27_31 (DWC_ufshc_block_BaseAddress + 0x1dfc)
#define CRYPTOCFG_27_31_RegisterSize 32
#define CRYPTOCFG_27_31_RegisterResetValue 0x0
#define CRYPTOCFG_27_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_27_31_RESERVED_27_BitAddressOffset 0
#define CRYPTOCFG_27_31_RESERVED_27_RegisterSize 32





#define CRYPTOCFG_28_0 (DWC_ufshc_block_BaseAddress + 0x1e00)
#define CRYPTOCFG_28_0_RegisterSize 32
#define CRYPTOCFG_28_0_RegisterResetValue 0x0
#define CRYPTOCFG_28_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_0_CRYPTOKEY_28_0_BitAddressOffset 0
#define CRYPTOCFG_28_0_CRYPTOKEY_28_0_RegisterSize 32





#define CRYPTOCFG_28_1 (DWC_ufshc_block_BaseAddress + 0x1e04)
#define CRYPTOCFG_28_1_RegisterSize 32
#define CRYPTOCFG_28_1_RegisterResetValue 0x0
#define CRYPTOCFG_28_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_1_CRYPTOKEY_28_1_BitAddressOffset 0
#define CRYPTOCFG_28_1_CRYPTOKEY_28_1_RegisterSize 32





#define CRYPTOCFG_28_2 (DWC_ufshc_block_BaseAddress + 0x1e08)
#define CRYPTOCFG_28_2_RegisterSize 32
#define CRYPTOCFG_28_2_RegisterResetValue 0x0
#define CRYPTOCFG_28_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_2_CRYPTOKEY_28_2_BitAddressOffset 0
#define CRYPTOCFG_28_2_CRYPTOKEY_28_2_RegisterSize 32





#define CRYPTOCFG_28_3 (DWC_ufshc_block_BaseAddress + 0x1e0c)
#define CRYPTOCFG_28_3_RegisterSize 32
#define CRYPTOCFG_28_3_RegisterResetValue 0x0
#define CRYPTOCFG_28_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_3_CRYPTOKEY_28_3_BitAddressOffset 0
#define CRYPTOCFG_28_3_CRYPTOKEY_28_3_RegisterSize 32





#define CRYPTOCFG_28_4 (DWC_ufshc_block_BaseAddress + 0x1e10)
#define CRYPTOCFG_28_4_RegisterSize 32
#define CRYPTOCFG_28_4_RegisterResetValue 0x0
#define CRYPTOCFG_28_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_4_CRYPTOKEY_28_4_BitAddressOffset 0
#define CRYPTOCFG_28_4_CRYPTOKEY_28_4_RegisterSize 32





#define CRYPTOCFG_28_5 (DWC_ufshc_block_BaseAddress + 0x1e14)
#define CRYPTOCFG_28_5_RegisterSize 32
#define CRYPTOCFG_28_5_RegisterResetValue 0x0
#define CRYPTOCFG_28_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_5_CRYPTOKEY_28_5_BitAddressOffset 0
#define CRYPTOCFG_28_5_CRYPTOKEY_28_5_RegisterSize 32





#define CRYPTOCFG_28_6 (DWC_ufshc_block_BaseAddress + 0x1e18)
#define CRYPTOCFG_28_6_RegisterSize 32
#define CRYPTOCFG_28_6_RegisterResetValue 0x0
#define CRYPTOCFG_28_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_6_CRYPTOKEY_28_6_BitAddressOffset 0
#define CRYPTOCFG_28_6_CRYPTOKEY_28_6_RegisterSize 32





#define CRYPTOCFG_28_7 (DWC_ufshc_block_BaseAddress + 0x1e1c)
#define CRYPTOCFG_28_7_RegisterSize 32
#define CRYPTOCFG_28_7_RegisterResetValue 0x0
#define CRYPTOCFG_28_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_7_CRYPTOKEY_28_7_BitAddressOffset 0
#define CRYPTOCFG_28_7_CRYPTOKEY_28_7_RegisterSize 32





#define CRYPTOCFG_28_8 (DWC_ufshc_block_BaseAddress + 0x1e20)
#define CRYPTOCFG_28_8_RegisterSize 32
#define CRYPTOCFG_28_8_RegisterResetValue 0x0
#define CRYPTOCFG_28_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_8_CRYPTOKEY_28_8_BitAddressOffset 0
#define CRYPTOCFG_28_8_CRYPTOKEY_28_8_RegisterSize 32





#define CRYPTOCFG_28_9 (DWC_ufshc_block_BaseAddress + 0x1e24)
#define CRYPTOCFG_28_9_RegisterSize 32
#define CRYPTOCFG_28_9_RegisterResetValue 0x0
#define CRYPTOCFG_28_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_9_CRYPTOKEY_28_9_BitAddressOffset 0
#define CRYPTOCFG_28_9_CRYPTOKEY_28_9_RegisterSize 32





#define CRYPTOCFG_28_10 (DWC_ufshc_block_BaseAddress + 0x1e28)
#define CRYPTOCFG_28_10_RegisterSize 32
#define CRYPTOCFG_28_10_RegisterResetValue 0x0
#define CRYPTOCFG_28_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_10_CRYPTOKEY_28_10_BitAddressOffset 0
#define CRYPTOCFG_28_10_CRYPTOKEY_28_10_RegisterSize 32





#define CRYPTOCFG_28_11 (DWC_ufshc_block_BaseAddress + 0x1e2c)
#define CRYPTOCFG_28_11_RegisterSize 32
#define CRYPTOCFG_28_11_RegisterResetValue 0x0
#define CRYPTOCFG_28_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_11_CRYPTOKEY_28_11_BitAddressOffset 0
#define CRYPTOCFG_28_11_CRYPTOKEY_28_11_RegisterSize 32





#define CRYPTOCFG_28_12 (DWC_ufshc_block_BaseAddress + 0x1e30)
#define CRYPTOCFG_28_12_RegisterSize 32
#define CRYPTOCFG_28_12_RegisterResetValue 0x0
#define CRYPTOCFG_28_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_12_CRYPTOKEY_28_12_BitAddressOffset 0
#define CRYPTOCFG_28_12_CRYPTOKEY_28_12_RegisterSize 32





#define CRYPTOCFG_28_13 (DWC_ufshc_block_BaseAddress + 0x1e34)
#define CRYPTOCFG_28_13_RegisterSize 32
#define CRYPTOCFG_28_13_RegisterResetValue 0x0
#define CRYPTOCFG_28_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_13_CRYPTOKEY_28_13_BitAddressOffset 0
#define CRYPTOCFG_28_13_CRYPTOKEY_28_13_RegisterSize 32





#define CRYPTOCFG_28_14 (DWC_ufshc_block_BaseAddress + 0x1e38)
#define CRYPTOCFG_28_14_RegisterSize 32
#define CRYPTOCFG_28_14_RegisterResetValue 0x0
#define CRYPTOCFG_28_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_14_CRYPTOKEY_28_14_BitAddressOffset 0
#define CRYPTOCFG_28_14_CRYPTOKEY_28_14_RegisterSize 32





#define CRYPTOCFG_28_15 (DWC_ufshc_block_BaseAddress + 0x1e3c)
#define CRYPTOCFG_28_15_RegisterSize 32
#define CRYPTOCFG_28_15_RegisterResetValue 0x0
#define CRYPTOCFG_28_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_15_CRYPTOKEY_28_15_BitAddressOffset 0
#define CRYPTOCFG_28_15_CRYPTOKEY_28_15_RegisterSize 32





#define CRYPTOCFG_28_16 (DWC_ufshc_block_BaseAddress + 0x1e40)
#define CRYPTOCFG_28_16_RegisterSize 32
#define CRYPTOCFG_28_16_RegisterResetValue 0x0
#define CRYPTOCFG_28_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_16_DUSIZE_28_BitAddressOffset 0
#define CRYPTOCFG_28_16_DUSIZE_28_RegisterSize 8



#define CRYPTOCFG_28_16_CAPIDX_28_BitAddressOffset 8
#define CRYPTOCFG_28_16_CAPIDX_28_RegisterSize 8



#define CRYPTOCFG_28_16_CFGE_28_BitAddressOffset 31
#define CRYPTOCFG_28_16_CFGE_28_RegisterSize 1





#define CRYPTOCFG_28_17 (DWC_ufshc_block_BaseAddress + 0x1e44)
#define CRYPTOCFG_28_17_RegisterSize 32
#define CRYPTOCFG_28_17_RegisterResetValue 0x0
#define CRYPTOCFG_28_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_17_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_17_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_18 (DWC_ufshc_block_BaseAddress + 0x1e48)
#define CRYPTOCFG_28_18_RegisterSize 32
#define CRYPTOCFG_28_18_RegisterResetValue 0x0
#define CRYPTOCFG_28_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_18_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_18_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_19 (DWC_ufshc_block_BaseAddress + 0x1e4c)
#define CRYPTOCFG_28_19_RegisterSize 32
#define CRYPTOCFG_28_19_RegisterResetValue 0x0
#define CRYPTOCFG_28_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_19_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_19_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_20 (DWC_ufshc_block_BaseAddress + 0x1e50)
#define CRYPTOCFG_28_20_RegisterSize 32
#define CRYPTOCFG_28_20_RegisterResetValue 0x0
#define CRYPTOCFG_28_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_20_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_20_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_21 (DWC_ufshc_block_BaseAddress + 0x1e54)
#define CRYPTOCFG_28_21_RegisterSize 32
#define CRYPTOCFG_28_21_RegisterResetValue 0x0
#define CRYPTOCFG_28_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_21_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_21_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_22 (DWC_ufshc_block_BaseAddress + 0x1e58)
#define CRYPTOCFG_28_22_RegisterSize 32
#define CRYPTOCFG_28_22_RegisterResetValue 0x0
#define CRYPTOCFG_28_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_22_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_22_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_23 (DWC_ufshc_block_BaseAddress + 0x1e5c)
#define CRYPTOCFG_28_23_RegisterSize 32
#define CRYPTOCFG_28_23_RegisterResetValue 0x0
#define CRYPTOCFG_28_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_23_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_23_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_24 (DWC_ufshc_block_BaseAddress + 0x1e60)
#define CRYPTOCFG_28_24_RegisterSize 32
#define CRYPTOCFG_28_24_RegisterResetValue 0x0
#define CRYPTOCFG_28_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_24_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_24_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_25 (DWC_ufshc_block_BaseAddress + 0x1e64)
#define CRYPTOCFG_28_25_RegisterSize 32
#define CRYPTOCFG_28_25_RegisterResetValue 0x0
#define CRYPTOCFG_28_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_25_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_25_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_26 (DWC_ufshc_block_BaseAddress + 0x1e68)
#define CRYPTOCFG_28_26_RegisterSize 32
#define CRYPTOCFG_28_26_RegisterResetValue 0x0
#define CRYPTOCFG_28_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_26_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_26_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_27 (DWC_ufshc_block_BaseAddress + 0x1e6c)
#define CRYPTOCFG_28_27_RegisterSize 32
#define CRYPTOCFG_28_27_RegisterResetValue 0x0
#define CRYPTOCFG_28_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_27_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_27_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_28 (DWC_ufshc_block_BaseAddress + 0x1e70)
#define CRYPTOCFG_28_28_RegisterSize 32
#define CRYPTOCFG_28_28_RegisterResetValue 0x0
#define CRYPTOCFG_28_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_28_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_28_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_29 (DWC_ufshc_block_BaseAddress + 0x1e74)
#define CRYPTOCFG_28_29_RegisterSize 32
#define CRYPTOCFG_28_29_RegisterResetValue 0x0
#define CRYPTOCFG_28_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_29_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_29_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_30 (DWC_ufshc_block_BaseAddress + 0x1e78)
#define CRYPTOCFG_28_30_RegisterSize 32
#define CRYPTOCFG_28_30_RegisterResetValue 0x0
#define CRYPTOCFG_28_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_30_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_30_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_28_31 (DWC_ufshc_block_BaseAddress + 0x1e7c)
#define CRYPTOCFG_28_31_RegisterSize 32
#define CRYPTOCFG_28_31_RegisterResetValue 0x0
#define CRYPTOCFG_28_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_28_31_RESERVED_28_BitAddressOffset 0
#define CRYPTOCFG_28_31_RESERVED_28_RegisterSize 32





#define CRYPTOCFG_29_0 (DWC_ufshc_block_BaseAddress + 0x1e80)
#define CRYPTOCFG_29_0_RegisterSize 32
#define CRYPTOCFG_29_0_RegisterResetValue 0x0
#define CRYPTOCFG_29_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_0_CRYPTOKEY_29_0_BitAddressOffset 0
#define CRYPTOCFG_29_0_CRYPTOKEY_29_0_RegisterSize 32





#define CRYPTOCFG_29_1 (DWC_ufshc_block_BaseAddress + 0x1e84)
#define CRYPTOCFG_29_1_RegisterSize 32
#define CRYPTOCFG_29_1_RegisterResetValue 0x0
#define CRYPTOCFG_29_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_1_CRYPTOKEY_29_1_BitAddressOffset 0
#define CRYPTOCFG_29_1_CRYPTOKEY_29_1_RegisterSize 32





#define CRYPTOCFG_29_2 (DWC_ufshc_block_BaseAddress + 0x1e88)
#define CRYPTOCFG_29_2_RegisterSize 32
#define CRYPTOCFG_29_2_RegisterResetValue 0x0
#define CRYPTOCFG_29_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_2_CRYPTOKEY_29_2_BitAddressOffset 0
#define CRYPTOCFG_29_2_CRYPTOKEY_29_2_RegisterSize 32





#define CRYPTOCFG_29_3 (DWC_ufshc_block_BaseAddress + 0x1e8c)
#define CRYPTOCFG_29_3_RegisterSize 32
#define CRYPTOCFG_29_3_RegisterResetValue 0x0
#define CRYPTOCFG_29_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_3_CRYPTOKEY_29_3_BitAddressOffset 0
#define CRYPTOCFG_29_3_CRYPTOKEY_29_3_RegisterSize 32





#define CRYPTOCFG_29_4 (DWC_ufshc_block_BaseAddress + 0x1e90)
#define CRYPTOCFG_29_4_RegisterSize 32
#define CRYPTOCFG_29_4_RegisterResetValue 0x0
#define CRYPTOCFG_29_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_4_CRYPTOKEY_29_4_BitAddressOffset 0
#define CRYPTOCFG_29_4_CRYPTOKEY_29_4_RegisterSize 32





#define CRYPTOCFG_29_5 (DWC_ufshc_block_BaseAddress + 0x1e94)
#define CRYPTOCFG_29_5_RegisterSize 32
#define CRYPTOCFG_29_5_RegisterResetValue 0x0
#define CRYPTOCFG_29_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_5_CRYPTOKEY_29_5_BitAddressOffset 0
#define CRYPTOCFG_29_5_CRYPTOKEY_29_5_RegisterSize 32





#define CRYPTOCFG_29_6 (DWC_ufshc_block_BaseAddress + 0x1e98)
#define CRYPTOCFG_29_6_RegisterSize 32
#define CRYPTOCFG_29_6_RegisterResetValue 0x0
#define CRYPTOCFG_29_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_6_CRYPTOKEY_29_6_BitAddressOffset 0
#define CRYPTOCFG_29_6_CRYPTOKEY_29_6_RegisterSize 32





#define CRYPTOCFG_29_7 (DWC_ufshc_block_BaseAddress + 0x1e9c)
#define CRYPTOCFG_29_7_RegisterSize 32
#define CRYPTOCFG_29_7_RegisterResetValue 0x0
#define CRYPTOCFG_29_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_7_CRYPTOKEY_29_7_BitAddressOffset 0
#define CRYPTOCFG_29_7_CRYPTOKEY_29_7_RegisterSize 32





#define CRYPTOCFG_29_8 (DWC_ufshc_block_BaseAddress + 0x1ea0)
#define CRYPTOCFG_29_8_RegisterSize 32
#define CRYPTOCFG_29_8_RegisterResetValue 0x0
#define CRYPTOCFG_29_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_8_CRYPTOKEY_29_8_BitAddressOffset 0
#define CRYPTOCFG_29_8_CRYPTOKEY_29_8_RegisterSize 32





#define CRYPTOCFG_29_9 (DWC_ufshc_block_BaseAddress + 0x1ea4)
#define CRYPTOCFG_29_9_RegisterSize 32
#define CRYPTOCFG_29_9_RegisterResetValue 0x0
#define CRYPTOCFG_29_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_9_CRYPTOKEY_29_9_BitAddressOffset 0
#define CRYPTOCFG_29_9_CRYPTOKEY_29_9_RegisterSize 32





#define CRYPTOCFG_29_10 (DWC_ufshc_block_BaseAddress + 0x1ea8)
#define CRYPTOCFG_29_10_RegisterSize 32
#define CRYPTOCFG_29_10_RegisterResetValue 0x0
#define CRYPTOCFG_29_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_10_CRYPTOKEY_29_10_BitAddressOffset 0
#define CRYPTOCFG_29_10_CRYPTOKEY_29_10_RegisterSize 32





#define CRYPTOCFG_29_11 (DWC_ufshc_block_BaseAddress + 0x1eac)
#define CRYPTOCFG_29_11_RegisterSize 32
#define CRYPTOCFG_29_11_RegisterResetValue 0x0
#define CRYPTOCFG_29_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_11_CRYPTOKEY_29_11_BitAddressOffset 0
#define CRYPTOCFG_29_11_CRYPTOKEY_29_11_RegisterSize 32





#define CRYPTOCFG_29_12 (DWC_ufshc_block_BaseAddress + 0x1eb0)
#define CRYPTOCFG_29_12_RegisterSize 32
#define CRYPTOCFG_29_12_RegisterResetValue 0x0
#define CRYPTOCFG_29_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_12_CRYPTOKEY_29_12_BitAddressOffset 0
#define CRYPTOCFG_29_12_CRYPTOKEY_29_12_RegisterSize 32





#define CRYPTOCFG_29_13 (DWC_ufshc_block_BaseAddress + 0x1eb4)
#define CRYPTOCFG_29_13_RegisterSize 32
#define CRYPTOCFG_29_13_RegisterResetValue 0x0
#define CRYPTOCFG_29_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_13_CRYPTOKEY_29_13_BitAddressOffset 0
#define CRYPTOCFG_29_13_CRYPTOKEY_29_13_RegisterSize 32





#define CRYPTOCFG_29_14 (DWC_ufshc_block_BaseAddress + 0x1eb8)
#define CRYPTOCFG_29_14_RegisterSize 32
#define CRYPTOCFG_29_14_RegisterResetValue 0x0
#define CRYPTOCFG_29_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_14_CRYPTOKEY_29_14_BitAddressOffset 0
#define CRYPTOCFG_29_14_CRYPTOKEY_29_14_RegisterSize 32





#define CRYPTOCFG_29_15 (DWC_ufshc_block_BaseAddress + 0x1ebc)
#define CRYPTOCFG_29_15_RegisterSize 32
#define CRYPTOCFG_29_15_RegisterResetValue 0x0
#define CRYPTOCFG_29_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_15_CRYPTOKEY_29_15_BitAddressOffset 0
#define CRYPTOCFG_29_15_CRYPTOKEY_29_15_RegisterSize 32





#define CRYPTOCFG_29_16 (DWC_ufshc_block_BaseAddress + 0x1ec0)
#define CRYPTOCFG_29_16_RegisterSize 32
#define CRYPTOCFG_29_16_RegisterResetValue 0x0
#define CRYPTOCFG_29_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_16_DUSIZE_29_BitAddressOffset 0
#define CRYPTOCFG_29_16_DUSIZE_29_RegisterSize 8



#define CRYPTOCFG_29_16_CAPIDX_29_BitAddressOffset 8
#define CRYPTOCFG_29_16_CAPIDX_29_RegisterSize 8



#define CRYPTOCFG_29_16_CFGE_29_BitAddressOffset 31
#define CRYPTOCFG_29_16_CFGE_29_RegisterSize 1





#define CRYPTOCFG_29_17 (DWC_ufshc_block_BaseAddress + 0x1ec4)
#define CRYPTOCFG_29_17_RegisterSize 32
#define CRYPTOCFG_29_17_RegisterResetValue 0x0
#define CRYPTOCFG_29_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_17_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_17_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_18 (DWC_ufshc_block_BaseAddress + 0x1ec8)
#define CRYPTOCFG_29_18_RegisterSize 32
#define CRYPTOCFG_29_18_RegisterResetValue 0x0
#define CRYPTOCFG_29_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_18_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_18_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_19 (DWC_ufshc_block_BaseAddress + 0x1ecc)
#define CRYPTOCFG_29_19_RegisterSize 32
#define CRYPTOCFG_29_19_RegisterResetValue 0x0
#define CRYPTOCFG_29_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_19_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_19_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_20 (DWC_ufshc_block_BaseAddress + 0x1ed0)
#define CRYPTOCFG_29_20_RegisterSize 32
#define CRYPTOCFG_29_20_RegisterResetValue 0x0
#define CRYPTOCFG_29_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_20_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_20_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_21 (DWC_ufshc_block_BaseAddress + 0x1ed4)
#define CRYPTOCFG_29_21_RegisterSize 32
#define CRYPTOCFG_29_21_RegisterResetValue 0x0
#define CRYPTOCFG_29_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_21_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_21_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_22 (DWC_ufshc_block_BaseAddress + 0x1ed8)
#define CRYPTOCFG_29_22_RegisterSize 32
#define CRYPTOCFG_29_22_RegisterResetValue 0x0
#define CRYPTOCFG_29_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_22_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_22_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_23 (DWC_ufshc_block_BaseAddress + 0x1edc)
#define CRYPTOCFG_29_23_RegisterSize 32
#define CRYPTOCFG_29_23_RegisterResetValue 0x0
#define CRYPTOCFG_29_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_23_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_23_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_24 (DWC_ufshc_block_BaseAddress + 0x1ee0)
#define CRYPTOCFG_29_24_RegisterSize 32
#define CRYPTOCFG_29_24_RegisterResetValue 0x0
#define CRYPTOCFG_29_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_24_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_24_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_25 (DWC_ufshc_block_BaseAddress + 0x1ee4)
#define CRYPTOCFG_29_25_RegisterSize 32
#define CRYPTOCFG_29_25_RegisterResetValue 0x0
#define CRYPTOCFG_29_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_25_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_25_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_26 (DWC_ufshc_block_BaseAddress + 0x1ee8)
#define CRYPTOCFG_29_26_RegisterSize 32
#define CRYPTOCFG_29_26_RegisterResetValue 0x0
#define CRYPTOCFG_29_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_26_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_26_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_27 (DWC_ufshc_block_BaseAddress + 0x1eec)
#define CRYPTOCFG_29_27_RegisterSize 32
#define CRYPTOCFG_29_27_RegisterResetValue 0x0
#define CRYPTOCFG_29_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_27_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_27_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_28 (DWC_ufshc_block_BaseAddress + 0x1ef0)
#define CRYPTOCFG_29_28_RegisterSize 32
#define CRYPTOCFG_29_28_RegisterResetValue 0x0
#define CRYPTOCFG_29_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_28_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_28_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_29 (DWC_ufshc_block_BaseAddress + 0x1ef4)
#define CRYPTOCFG_29_29_RegisterSize 32
#define CRYPTOCFG_29_29_RegisterResetValue 0x0
#define CRYPTOCFG_29_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_29_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_29_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_30 (DWC_ufshc_block_BaseAddress + 0x1ef8)
#define CRYPTOCFG_29_30_RegisterSize 32
#define CRYPTOCFG_29_30_RegisterResetValue 0x0
#define CRYPTOCFG_29_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_30_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_30_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_29_31 (DWC_ufshc_block_BaseAddress + 0x1efc)
#define CRYPTOCFG_29_31_RegisterSize 32
#define CRYPTOCFG_29_31_RegisterResetValue 0x0
#define CRYPTOCFG_29_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_29_31_RESERVED_29_BitAddressOffset 0
#define CRYPTOCFG_29_31_RESERVED_29_RegisterSize 32





#define CRYPTOCFG_30_0 (DWC_ufshc_block_BaseAddress + 0x1f00)
#define CRYPTOCFG_30_0_RegisterSize 32
#define CRYPTOCFG_30_0_RegisterResetValue 0x0
#define CRYPTOCFG_30_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_0_CRYPTOKEY_30_0_BitAddressOffset 0
#define CRYPTOCFG_30_0_CRYPTOKEY_30_0_RegisterSize 32





#define CRYPTOCFG_30_1 (DWC_ufshc_block_BaseAddress + 0x1f04)
#define CRYPTOCFG_30_1_RegisterSize 32
#define CRYPTOCFG_30_1_RegisterResetValue 0x0
#define CRYPTOCFG_30_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_1_CRYPTOKEY_30_1_BitAddressOffset 0
#define CRYPTOCFG_30_1_CRYPTOKEY_30_1_RegisterSize 32





#define CRYPTOCFG_30_2 (DWC_ufshc_block_BaseAddress + 0x1f08)
#define CRYPTOCFG_30_2_RegisterSize 32
#define CRYPTOCFG_30_2_RegisterResetValue 0x0
#define CRYPTOCFG_30_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_2_CRYPTOKEY_30_2_BitAddressOffset 0
#define CRYPTOCFG_30_2_CRYPTOKEY_30_2_RegisterSize 32





#define CRYPTOCFG_30_3 (DWC_ufshc_block_BaseAddress + 0x1f0c)
#define CRYPTOCFG_30_3_RegisterSize 32
#define CRYPTOCFG_30_3_RegisterResetValue 0x0
#define CRYPTOCFG_30_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_3_CRYPTOKEY_30_3_BitAddressOffset 0
#define CRYPTOCFG_30_3_CRYPTOKEY_30_3_RegisterSize 32





#define CRYPTOCFG_30_4 (DWC_ufshc_block_BaseAddress + 0x1f10)
#define CRYPTOCFG_30_4_RegisterSize 32
#define CRYPTOCFG_30_4_RegisterResetValue 0x0
#define CRYPTOCFG_30_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_4_CRYPTOKEY_30_4_BitAddressOffset 0
#define CRYPTOCFG_30_4_CRYPTOKEY_30_4_RegisterSize 32





#define CRYPTOCFG_30_5 (DWC_ufshc_block_BaseAddress + 0x1f14)
#define CRYPTOCFG_30_5_RegisterSize 32
#define CRYPTOCFG_30_5_RegisterResetValue 0x0
#define CRYPTOCFG_30_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_5_CRYPTOKEY_30_5_BitAddressOffset 0
#define CRYPTOCFG_30_5_CRYPTOKEY_30_5_RegisterSize 32





#define CRYPTOCFG_30_6 (DWC_ufshc_block_BaseAddress + 0x1f18)
#define CRYPTOCFG_30_6_RegisterSize 32
#define CRYPTOCFG_30_6_RegisterResetValue 0x0
#define CRYPTOCFG_30_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_6_CRYPTOKEY_30_6_BitAddressOffset 0
#define CRYPTOCFG_30_6_CRYPTOKEY_30_6_RegisterSize 32





#define CRYPTOCFG_30_7 (DWC_ufshc_block_BaseAddress + 0x1f1c)
#define CRYPTOCFG_30_7_RegisterSize 32
#define CRYPTOCFG_30_7_RegisterResetValue 0x0
#define CRYPTOCFG_30_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_7_CRYPTOKEY_30_7_BitAddressOffset 0
#define CRYPTOCFG_30_7_CRYPTOKEY_30_7_RegisterSize 32





#define CRYPTOCFG_30_8 (DWC_ufshc_block_BaseAddress + 0x1f20)
#define CRYPTOCFG_30_8_RegisterSize 32
#define CRYPTOCFG_30_8_RegisterResetValue 0x0
#define CRYPTOCFG_30_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_8_CRYPTOKEY_30_8_BitAddressOffset 0
#define CRYPTOCFG_30_8_CRYPTOKEY_30_8_RegisterSize 32





#define CRYPTOCFG_30_9 (DWC_ufshc_block_BaseAddress + 0x1f24)
#define CRYPTOCFG_30_9_RegisterSize 32
#define CRYPTOCFG_30_9_RegisterResetValue 0x0
#define CRYPTOCFG_30_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_9_CRYPTOKEY_30_9_BitAddressOffset 0
#define CRYPTOCFG_30_9_CRYPTOKEY_30_9_RegisterSize 32





#define CRYPTOCFG_30_10 (DWC_ufshc_block_BaseAddress + 0x1f28)
#define CRYPTOCFG_30_10_RegisterSize 32
#define CRYPTOCFG_30_10_RegisterResetValue 0x0
#define CRYPTOCFG_30_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_10_CRYPTOKEY_30_10_BitAddressOffset 0
#define CRYPTOCFG_30_10_CRYPTOKEY_30_10_RegisterSize 32





#define CRYPTOCFG_30_11 (DWC_ufshc_block_BaseAddress + 0x1f2c)
#define CRYPTOCFG_30_11_RegisterSize 32
#define CRYPTOCFG_30_11_RegisterResetValue 0x0
#define CRYPTOCFG_30_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_11_CRYPTOKEY_30_11_BitAddressOffset 0
#define CRYPTOCFG_30_11_CRYPTOKEY_30_11_RegisterSize 32





#define CRYPTOCFG_30_12 (DWC_ufshc_block_BaseAddress + 0x1f30)
#define CRYPTOCFG_30_12_RegisterSize 32
#define CRYPTOCFG_30_12_RegisterResetValue 0x0
#define CRYPTOCFG_30_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_12_CRYPTOKEY_30_12_BitAddressOffset 0
#define CRYPTOCFG_30_12_CRYPTOKEY_30_12_RegisterSize 32





#define CRYPTOCFG_30_13 (DWC_ufshc_block_BaseAddress + 0x1f34)
#define CRYPTOCFG_30_13_RegisterSize 32
#define CRYPTOCFG_30_13_RegisterResetValue 0x0
#define CRYPTOCFG_30_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_13_CRYPTOKEY_30_13_BitAddressOffset 0
#define CRYPTOCFG_30_13_CRYPTOKEY_30_13_RegisterSize 32





#define CRYPTOCFG_30_14 (DWC_ufshc_block_BaseAddress + 0x1f38)
#define CRYPTOCFG_30_14_RegisterSize 32
#define CRYPTOCFG_30_14_RegisterResetValue 0x0
#define CRYPTOCFG_30_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_14_CRYPTOKEY_30_14_BitAddressOffset 0
#define CRYPTOCFG_30_14_CRYPTOKEY_30_14_RegisterSize 32





#define CRYPTOCFG_30_15 (DWC_ufshc_block_BaseAddress + 0x1f3c)
#define CRYPTOCFG_30_15_RegisterSize 32
#define CRYPTOCFG_30_15_RegisterResetValue 0x0
#define CRYPTOCFG_30_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_15_CRYPTOKEY_30_15_BitAddressOffset 0
#define CRYPTOCFG_30_15_CRYPTOKEY_30_15_RegisterSize 32





#define CRYPTOCFG_30_16 (DWC_ufshc_block_BaseAddress + 0x1f40)
#define CRYPTOCFG_30_16_RegisterSize 32
#define CRYPTOCFG_30_16_RegisterResetValue 0x0
#define CRYPTOCFG_30_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_16_DUSIZE_30_BitAddressOffset 0
#define CRYPTOCFG_30_16_DUSIZE_30_RegisterSize 8



#define CRYPTOCFG_30_16_CAPIDX_30_BitAddressOffset 8
#define CRYPTOCFG_30_16_CAPIDX_30_RegisterSize 8



#define CRYPTOCFG_30_16_CFGE_30_BitAddressOffset 31
#define CRYPTOCFG_30_16_CFGE_30_RegisterSize 1





#define CRYPTOCFG_30_17 (DWC_ufshc_block_BaseAddress + 0x1f44)
#define CRYPTOCFG_30_17_RegisterSize 32
#define CRYPTOCFG_30_17_RegisterResetValue 0x0
#define CRYPTOCFG_30_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_17_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_17_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_18 (DWC_ufshc_block_BaseAddress + 0x1f48)
#define CRYPTOCFG_30_18_RegisterSize 32
#define CRYPTOCFG_30_18_RegisterResetValue 0x0
#define CRYPTOCFG_30_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_18_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_18_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_19 (DWC_ufshc_block_BaseAddress + 0x1f4c)
#define CRYPTOCFG_30_19_RegisterSize 32
#define CRYPTOCFG_30_19_RegisterResetValue 0x0
#define CRYPTOCFG_30_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_19_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_19_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_20 (DWC_ufshc_block_BaseAddress + 0x1f50)
#define CRYPTOCFG_30_20_RegisterSize 32
#define CRYPTOCFG_30_20_RegisterResetValue 0x0
#define CRYPTOCFG_30_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_20_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_20_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_21 (DWC_ufshc_block_BaseAddress + 0x1f54)
#define CRYPTOCFG_30_21_RegisterSize 32
#define CRYPTOCFG_30_21_RegisterResetValue 0x0
#define CRYPTOCFG_30_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_21_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_21_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_22 (DWC_ufshc_block_BaseAddress + 0x1f58)
#define CRYPTOCFG_30_22_RegisterSize 32
#define CRYPTOCFG_30_22_RegisterResetValue 0x0
#define CRYPTOCFG_30_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_22_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_22_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_23 (DWC_ufshc_block_BaseAddress + 0x1f5c)
#define CRYPTOCFG_30_23_RegisterSize 32
#define CRYPTOCFG_30_23_RegisterResetValue 0x0
#define CRYPTOCFG_30_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_23_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_23_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_24 (DWC_ufshc_block_BaseAddress + 0x1f60)
#define CRYPTOCFG_30_24_RegisterSize 32
#define CRYPTOCFG_30_24_RegisterResetValue 0x0
#define CRYPTOCFG_30_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_24_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_24_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_25 (DWC_ufshc_block_BaseAddress + 0x1f64)
#define CRYPTOCFG_30_25_RegisterSize 32
#define CRYPTOCFG_30_25_RegisterResetValue 0x0
#define CRYPTOCFG_30_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_25_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_25_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_26 (DWC_ufshc_block_BaseAddress + 0x1f68)
#define CRYPTOCFG_30_26_RegisterSize 32
#define CRYPTOCFG_30_26_RegisterResetValue 0x0
#define CRYPTOCFG_30_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_26_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_26_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_27 (DWC_ufshc_block_BaseAddress + 0x1f6c)
#define CRYPTOCFG_30_27_RegisterSize 32
#define CRYPTOCFG_30_27_RegisterResetValue 0x0
#define CRYPTOCFG_30_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_27_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_27_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_28 (DWC_ufshc_block_BaseAddress + 0x1f70)
#define CRYPTOCFG_30_28_RegisterSize 32
#define CRYPTOCFG_30_28_RegisterResetValue 0x0
#define CRYPTOCFG_30_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_28_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_28_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_29 (DWC_ufshc_block_BaseAddress + 0x1f74)
#define CRYPTOCFG_30_29_RegisterSize 32
#define CRYPTOCFG_30_29_RegisterResetValue 0x0
#define CRYPTOCFG_30_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_29_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_29_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_30 (DWC_ufshc_block_BaseAddress + 0x1f78)
#define CRYPTOCFG_30_30_RegisterSize 32
#define CRYPTOCFG_30_30_RegisterResetValue 0x0
#define CRYPTOCFG_30_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_30_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_30_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_30_31 (DWC_ufshc_block_BaseAddress + 0x1f7c)
#define CRYPTOCFG_30_31_RegisterSize 32
#define CRYPTOCFG_30_31_RegisterResetValue 0x0
#define CRYPTOCFG_30_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_30_31_RESERVED_30_BitAddressOffset 0
#define CRYPTOCFG_30_31_RESERVED_30_RegisterSize 32





#define CRYPTOCFG_31_0 (DWC_ufshc_block_BaseAddress + 0x1f80)
#define CRYPTOCFG_31_0_RegisterSize 32
#define CRYPTOCFG_31_0_RegisterResetValue 0x0
#define CRYPTOCFG_31_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_0_CRYPTOKEY_31_0_BitAddressOffset 0
#define CRYPTOCFG_31_0_CRYPTOKEY_31_0_RegisterSize 32





#define CRYPTOCFG_31_1 (DWC_ufshc_block_BaseAddress + 0x1f84)
#define CRYPTOCFG_31_1_RegisterSize 32
#define CRYPTOCFG_31_1_RegisterResetValue 0x0
#define CRYPTOCFG_31_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_1_CRYPTOKEY_31_1_BitAddressOffset 0
#define CRYPTOCFG_31_1_CRYPTOKEY_31_1_RegisterSize 32





#define CRYPTOCFG_31_2 (DWC_ufshc_block_BaseAddress + 0x1f88)
#define CRYPTOCFG_31_2_RegisterSize 32
#define CRYPTOCFG_31_2_RegisterResetValue 0x0
#define CRYPTOCFG_31_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_2_CRYPTOKEY_31_2_BitAddressOffset 0
#define CRYPTOCFG_31_2_CRYPTOKEY_31_2_RegisterSize 32





#define CRYPTOCFG_31_3 (DWC_ufshc_block_BaseAddress + 0x1f8c)
#define CRYPTOCFG_31_3_RegisterSize 32
#define CRYPTOCFG_31_3_RegisterResetValue 0x0
#define CRYPTOCFG_31_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_3_CRYPTOKEY_31_3_BitAddressOffset 0
#define CRYPTOCFG_31_3_CRYPTOKEY_31_3_RegisterSize 32





#define CRYPTOCFG_31_4 (DWC_ufshc_block_BaseAddress + 0x1f90)
#define CRYPTOCFG_31_4_RegisterSize 32
#define CRYPTOCFG_31_4_RegisterResetValue 0x0
#define CRYPTOCFG_31_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_4_CRYPTOKEY_31_4_BitAddressOffset 0
#define CRYPTOCFG_31_4_CRYPTOKEY_31_4_RegisterSize 32





#define CRYPTOCFG_31_5 (DWC_ufshc_block_BaseAddress + 0x1f94)
#define CRYPTOCFG_31_5_RegisterSize 32
#define CRYPTOCFG_31_5_RegisterResetValue 0x0
#define CRYPTOCFG_31_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_5_CRYPTOKEY_31_5_BitAddressOffset 0
#define CRYPTOCFG_31_5_CRYPTOKEY_31_5_RegisterSize 32





#define CRYPTOCFG_31_6 (DWC_ufshc_block_BaseAddress + 0x1f98)
#define CRYPTOCFG_31_6_RegisterSize 32
#define CRYPTOCFG_31_6_RegisterResetValue 0x0
#define CRYPTOCFG_31_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_6_CRYPTOKEY_31_6_BitAddressOffset 0
#define CRYPTOCFG_31_6_CRYPTOKEY_31_6_RegisterSize 32





#define CRYPTOCFG_31_7 (DWC_ufshc_block_BaseAddress + 0x1f9c)
#define CRYPTOCFG_31_7_RegisterSize 32
#define CRYPTOCFG_31_7_RegisterResetValue 0x0
#define CRYPTOCFG_31_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_7_CRYPTOKEY_31_7_BitAddressOffset 0
#define CRYPTOCFG_31_7_CRYPTOKEY_31_7_RegisterSize 32





#define CRYPTOCFG_31_8 (DWC_ufshc_block_BaseAddress + 0x1fa0)
#define CRYPTOCFG_31_8_RegisterSize 32
#define CRYPTOCFG_31_8_RegisterResetValue 0x0
#define CRYPTOCFG_31_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_8_CRYPTOKEY_31_8_BitAddressOffset 0
#define CRYPTOCFG_31_8_CRYPTOKEY_31_8_RegisterSize 32





#define CRYPTOCFG_31_9 (DWC_ufshc_block_BaseAddress + 0x1fa4)
#define CRYPTOCFG_31_9_RegisterSize 32
#define CRYPTOCFG_31_9_RegisterResetValue 0x0
#define CRYPTOCFG_31_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_9_CRYPTOKEY_31_9_BitAddressOffset 0
#define CRYPTOCFG_31_9_CRYPTOKEY_31_9_RegisterSize 32





#define CRYPTOCFG_31_10 (DWC_ufshc_block_BaseAddress + 0x1fa8)
#define CRYPTOCFG_31_10_RegisterSize 32
#define CRYPTOCFG_31_10_RegisterResetValue 0x0
#define CRYPTOCFG_31_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_10_CRYPTOKEY_31_10_BitAddressOffset 0
#define CRYPTOCFG_31_10_CRYPTOKEY_31_10_RegisterSize 32





#define CRYPTOCFG_31_11 (DWC_ufshc_block_BaseAddress + 0x1fac)
#define CRYPTOCFG_31_11_RegisterSize 32
#define CRYPTOCFG_31_11_RegisterResetValue 0x0
#define CRYPTOCFG_31_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_11_CRYPTOKEY_31_11_BitAddressOffset 0
#define CRYPTOCFG_31_11_CRYPTOKEY_31_11_RegisterSize 32





#define CRYPTOCFG_31_12 (DWC_ufshc_block_BaseAddress + 0x1fb0)
#define CRYPTOCFG_31_12_RegisterSize 32
#define CRYPTOCFG_31_12_RegisterResetValue 0x0
#define CRYPTOCFG_31_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_12_CRYPTOKEY_31_12_BitAddressOffset 0
#define CRYPTOCFG_31_12_CRYPTOKEY_31_12_RegisterSize 32





#define CRYPTOCFG_31_13 (DWC_ufshc_block_BaseAddress + 0x1fb4)
#define CRYPTOCFG_31_13_RegisterSize 32
#define CRYPTOCFG_31_13_RegisterResetValue 0x0
#define CRYPTOCFG_31_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_13_CRYPTOKEY_31_13_BitAddressOffset 0
#define CRYPTOCFG_31_13_CRYPTOKEY_31_13_RegisterSize 32





#define CRYPTOCFG_31_14 (DWC_ufshc_block_BaseAddress + 0x1fb8)
#define CRYPTOCFG_31_14_RegisterSize 32
#define CRYPTOCFG_31_14_RegisterResetValue 0x0
#define CRYPTOCFG_31_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_14_CRYPTOKEY_31_14_BitAddressOffset 0
#define CRYPTOCFG_31_14_CRYPTOKEY_31_14_RegisterSize 32





#define CRYPTOCFG_31_15 (DWC_ufshc_block_BaseAddress + 0x1fbc)
#define CRYPTOCFG_31_15_RegisterSize 32
#define CRYPTOCFG_31_15_RegisterResetValue 0x0
#define CRYPTOCFG_31_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_15_CRYPTOKEY_31_15_BitAddressOffset 0
#define CRYPTOCFG_31_15_CRYPTOKEY_31_15_RegisterSize 32





#define CRYPTOCFG_31_16 (DWC_ufshc_block_BaseAddress + 0x1fc0)
#define CRYPTOCFG_31_16_RegisterSize 32
#define CRYPTOCFG_31_16_RegisterResetValue 0x0
#define CRYPTOCFG_31_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_16_DUSIZE_31_BitAddressOffset 0
#define CRYPTOCFG_31_16_DUSIZE_31_RegisterSize 8



#define CRYPTOCFG_31_16_CAPIDX_31_BitAddressOffset 8
#define CRYPTOCFG_31_16_CAPIDX_31_RegisterSize 8



#define CRYPTOCFG_31_16_CFGE_31_BitAddressOffset 31
#define CRYPTOCFG_31_16_CFGE_31_RegisterSize 1





#define CRYPTOCFG_31_17 (DWC_ufshc_block_BaseAddress + 0x1fc4)
#define CRYPTOCFG_31_17_RegisterSize 32
#define CRYPTOCFG_31_17_RegisterResetValue 0x0
#define CRYPTOCFG_31_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_17_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_17_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_18 (DWC_ufshc_block_BaseAddress + 0x1fc8)
#define CRYPTOCFG_31_18_RegisterSize 32
#define CRYPTOCFG_31_18_RegisterResetValue 0x0
#define CRYPTOCFG_31_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_18_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_18_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_19 (DWC_ufshc_block_BaseAddress + 0x1fcc)
#define CRYPTOCFG_31_19_RegisterSize 32
#define CRYPTOCFG_31_19_RegisterResetValue 0x0
#define CRYPTOCFG_31_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_19_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_19_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_20 (DWC_ufshc_block_BaseAddress + 0x1fd0)
#define CRYPTOCFG_31_20_RegisterSize 32
#define CRYPTOCFG_31_20_RegisterResetValue 0x0
#define CRYPTOCFG_31_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_20_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_20_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_21 (DWC_ufshc_block_BaseAddress + 0x1fd4)
#define CRYPTOCFG_31_21_RegisterSize 32
#define CRYPTOCFG_31_21_RegisterResetValue 0x0
#define CRYPTOCFG_31_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_21_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_21_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_22 (DWC_ufshc_block_BaseAddress + 0x1fd8)
#define CRYPTOCFG_31_22_RegisterSize 32
#define CRYPTOCFG_31_22_RegisterResetValue 0x0
#define CRYPTOCFG_31_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_22_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_22_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_23 (DWC_ufshc_block_BaseAddress + 0x1fdc)
#define CRYPTOCFG_31_23_RegisterSize 32
#define CRYPTOCFG_31_23_RegisterResetValue 0x0
#define CRYPTOCFG_31_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_23_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_23_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_24 (DWC_ufshc_block_BaseAddress + 0x1fe0)
#define CRYPTOCFG_31_24_RegisterSize 32
#define CRYPTOCFG_31_24_RegisterResetValue 0x0
#define CRYPTOCFG_31_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_24_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_24_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_25 (DWC_ufshc_block_BaseAddress + 0x1fe4)
#define CRYPTOCFG_31_25_RegisterSize 32
#define CRYPTOCFG_31_25_RegisterResetValue 0x0
#define CRYPTOCFG_31_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_25_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_25_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_26 (DWC_ufshc_block_BaseAddress + 0x1fe8)
#define CRYPTOCFG_31_26_RegisterSize 32
#define CRYPTOCFG_31_26_RegisterResetValue 0x0
#define CRYPTOCFG_31_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_26_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_26_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_27 (DWC_ufshc_block_BaseAddress + 0x1fec)
#define CRYPTOCFG_31_27_RegisterSize 32
#define CRYPTOCFG_31_27_RegisterResetValue 0x0
#define CRYPTOCFG_31_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_27_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_27_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_28 (DWC_ufshc_block_BaseAddress + 0x1ff0)
#define CRYPTOCFG_31_28_RegisterSize 32
#define CRYPTOCFG_31_28_RegisterResetValue 0x0
#define CRYPTOCFG_31_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_28_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_28_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_29 (DWC_ufshc_block_BaseAddress + 0x1ff4)
#define CRYPTOCFG_31_29_RegisterSize 32
#define CRYPTOCFG_31_29_RegisterResetValue 0x0
#define CRYPTOCFG_31_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_29_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_29_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_30 (DWC_ufshc_block_BaseAddress + 0x1ff8)
#define CRYPTOCFG_31_30_RegisterSize 32
#define CRYPTOCFG_31_30_RegisterResetValue 0x0
#define CRYPTOCFG_31_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_30_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_30_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_31_31 (DWC_ufshc_block_BaseAddress + 0x1ffc)
#define CRYPTOCFG_31_31_RegisterSize 32
#define CRYPTOCFG_31_31_RegisterResetValue 0x0
#define CRYPTOCFG_31_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_31_31_RESERVED_31_BitAddressOffset 0
#define CRYPTOCFG_31_31_RESERVED_31_RegisterSize 32





#define CRYPTOCFG_32_0 (DWC_ufshc_block_BaseAddress + 0x2000)
#define CRYPTOCFG_32_0_RegisterSize 32
#define CRYPTOCFG_32_0_RegisterResetValue 0x0
#define CRYPTOCFG_32_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_0_CRYPTOKEY_32_0_BitAddressOffset 0
#define CRYPTOCFG_32_0_CRYPTOKEY_32_0_RegisterSize 32





#define CRYPTOCFG_32_1 (DWC_ufshc_block_BaseAddress + 0x2004)
#define CRYPTOCFG_32_1_RegisterSize 32
#define CRYPTOCFG_32_1_RegisterResetValue 0x0
#define CRYPTOCFG_32_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_1_CRYPTOKEY_32_1_BitAddressOffset 0
#define CRYPTOCFG_32_1_CRYPTOKEY_32_1_RegisterSize 32





#define CRYPTOCFG_32_2 (DWC_ufshc_block_BaseAddress + 0x2008)
#define CRYPTOCFG_32_2_RegisterSize 32
#define CRYPTOCFG_32_2_RegisterResetValue 0x0
#define CRYPTOCFG_32_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_2_CRYPTOKEY_32_2_BitAddressOffset 0
#define CRYPTOCFG_32_2_CRYPTOKEY_32_2_RegisterSize 32





#define CRYPTOCFG_32_3 (DWC_ufshc_block_BaseAddress + 0x200c)
#define CRYPTOCFG_32_3_RegisterSize 32
#define CRYPTOCFG_32_3_RegisterResetValue 0x0
#define CRYPTOCFG_32_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_3_CRYPTOKEY_32_3_BitAddressOffset 0
#define CRYPTOCFG_32_3_CRYPTOKEY_32_3_RegisterSize 32





#define CRYPTOCFG_32_4 (DWC_ufshc_block_BaseAddress + 0x2010)
#define CRYPTOCFG_32_4_RegisterSize 32
#define CRYPTOCFG_32_4_RegisterResetValue 0x0
#define CRYPTOCFG_32_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_4_CRYPTOKEY_32_4_BitAddressOffset 0
#define CRYPTOCFG_32_4_CRYPTOKEY_32_4_RegisterSize 32





#define CRYPTOCFG_32_5 (DWC_ufshc_block_BaseAddress + 0x2014)
#define CRYPTOCFG_32_5_RegisterSize 32
#define CRYPTOCFG_32_5_RegisterResetValue 0x0
#define CRYPTOCFG_32_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_5_CRYPTOKEY_32_5_BitAddressOffset 0
#define CRYPTOCFG_32_5_CRYPTOKEY_32_5_RegisterSize 32





#define CRYPTOCFG_32_6 (DWC_ufshc_block_BaseAddress + 0x2018)
#define CRYPTOCFG_32_6_RegisterSize 32
#define CRYPTOCFG_32_6_RegisterResetValue 0x0
#define CRYPTOCFG_32_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_6_CRYPTOKEY_32_6_BitAddressOffset 0
#define CRYPTOCFG_32_6_CRYPTOKEY_32_6_RegisterSize 32





#define CRYPTOCFG_32_7 (DWC_ufshc_block_BaseAddress + 0x201c)
#define CRYPTOCFG_32_7_RegisterSize 32
#define CRYPTOCFG_32_7_RegisterResetValue 0x0
#define CRYPTOCFG_32_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_7_CRYPTOKEY_32_7_BitAddressOffset 0
#define CRYPTOCFG_32_7_CRYPTOKEY_32_7_RegisterSize 32





#define CRYPTOCFG_32_8 (DWC_ufshc_block_BaseAddress + 0x2020)
#define CRYPTOCFG_32_8_RegisterSize 32
#define CRYPTOCFG_32_8_RegisterResetValue 0x0
#define CRYPTOCFG_32_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_8_CRYPTOKEY_32_8_BitAddressOffset 0
#define CRYPTOCFG_32_8_CRYPTOKEY_32_8_RegisterSize 32





#define CRYPTOCFG_32_9 (DWC_ufshc_block_BaseAddress + 0x2024)
#define CRYPTOCFG_32_9_RegisterSize 32
#define CRYPTOCFG_32_9_RegisterResetValue 0x0
#define CRYPTOCFG_32_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_9_CRYPTOKEY_32_9_BitAddressOffset 0
#define CRYPTOCFG_32_9_CRYPTOKEY_32_9_RegisterSize 32





#define CRYPTOCFG_32_10 (DWC_ufshc_block_BaseAddress + 0x2028)
#define CRYPTOCFG_32_10_RegisterSize 32
#define CRYPTOCFG_32_10_RegisterResetValue 0x0
#define CRYPTOCFG_32_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_10_CRYPTOKEY_32_10_BitAddressOffset 0
#define CRYPTOCFG_32_10_CRYPTOKEY_32_10_RegisterSize 32





#define CRYPTOCFG_32_11 (DWC_ufshc_block_BaseAddress + 0x202c)
#define CRYPTOCFG_32_11_RegisterSize 32
#define CRYPTOCFG_32_11_RegisterResetValue 0x0
#define CRYPTOCFG_32_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_11_CRYPTOKEY_32_11_BitAddressOffset 0
#define CRYPTOCFG_32_11_CRYPTOKEY_32_11_RegisterSize 32





#define CRYPTOCFG_32_12 (DWC_ufshc_block_BaseAddress + 0x2030)
#define CRYPTOCFG_32_12_RegisterSize 32
#define CRYPTOCFG_32_12_RegisterResetValue 0x0
#define CRYPTOCFG_32_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_12_CRYPTOKEY_32_12_BitAddressOffset 0
#define CRYPTOCFG_32_12_CRYPTOKEY_32_12_RegisterSize 32





#define CRYPTOCFG_32_13 (DWC_ufshc_block_BaseAddress + 0x2034)
#define CRYPTOCFG_32_13_RegisterSize 32
#define CRYPTOCFG_32_13_RegisterResetValue 0x0
#define CRYPTOCFG_32_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_13_CRYPTOKEY_32_13_BitAddressOffset 0
#define CRYPTOCFG_32_13_CRYPTOKEY_32_13_RegisterSize 32





#define CRYPTOCFG_32_14 (DWC_ufshc_block_BaseAddress + 0x2038)
#define CRYPTOCFG_32_14_RegisterSize 32
#define CRYPTOCFG_32_14_RegisterResetValue 0x0
#define CRYPTOCFG_32_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_14_CRYPTOKEY_32_14_BitAddressOffset 0
#define CRYPTOCFG_32_14_CRYPTOKEY_32_14_RegisterSize 32





#define CRYPTOCFG_32_15 (DWC_ufshc_block_BaseAddress + 0x203c)
#define CRYPTOCFG_32_15_RegisterSize 32
#define CRYPTOCFG_32_15_RegisterResetValue 0x0
#define CRYPTOCFG_32_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_15_CRYPTOKEY_32_15_BitAddressOffset 0
#define CRYPTOCFG_32_15_CRYPTOKEY_32_15_RegisterSize 32





#define CRYPTOCFG_32_16 (DWC_ufshc_block_BaseAddress + 0x2040)
#define CRYPTOCFG_32_16_RegisterSize 32
#define CRYPTOCFG_32_16_RegisterResetValue 0x0
#define CRYPTOCFG_32_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_16_DUSIZE_32_BitAddressOffset 0
#define CRYPTOCFG_32_16_DUSIZE_32_RegisterSize 8



#define CRYPTOCFG_32_16_CAPIDX_32_BitAddressOffset 8
#define CRYPTOCFG_32_16_CAPIDX_32_RegisterSize 8



#define CRYPTOCFG_32_16_CFGE_32_BitAddressOffset 31
#define CRYPTOCFG_32_16_CFGE_32_RegisterSize 1





#define CRYPTOCFG_32_17 (DWC_ufshc_block_BaseAddress + 0x2044)
#define CRYPTOCFG_32_17_RegisterSize 32
#define CRYPTOCFG_32_17_RegisterResetValue 0x0
#define CRYPTOCFG_32_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_17_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_17_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_18 (DWC_ufshc_block_BaseAddress + 0x2048)
#define CRYPTOCFG_32_18_RegisterSize 32
#define CRYPTOCFG_32_18_RegisterResetValue 0x0
#define CRYPTOCFG_32_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_18_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_18_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_19 (DWC_ufshc_block_BaseAddress + 0x204c)
#define CRYPTOCFG_32_19_RegisterSize 32
#define CRYPTOCFG_32_19_RegisterResetValue 0x0
#define CRYPTOCFG_32_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_19_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_19_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_20 (DWC_ufshc_block_BaseAddress + 0x2050)
#define CRYPTOCFG_32_20_RegisterSize 32
#define CRYPTOCFG_32_20_RegisterResetValue 0x0
#define CRYPTOCFG_32_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_20_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_20_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_21 (DWC_ufshc_block_BaseAddress + 0x2054)
#define CRYPTOCFG_32_21_RegisterSize 32
#define CRYPTOCFG_32_21_RegisterResetValue 0x0
#define CRYPTOCFG_32_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_21_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_21_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_22 (DWC_ufshc_block_BaseAddress + 0x2058)
#define CRYPTOCFG_32_22_RegisterSize 32
#define CRYPTOCFG_32_22_RegisterResetValue 0x0
#define CRYPTOCFG_32_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_22_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_22_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_23 (DWC_ufshc_block_BaseAddress + 0x205c)
#define CRYPTOCFG_32_23_RegisterSize 32
#define CRYPTOCFG_32_23_RegisterResetValue 0x0
#define CRYPTOCFG_32_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_23_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_23_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_24 (DWC_ufshc_block_BaseAddress + 0x2060)
#define CRYPTOCFG_32_24_RegisterSize 32
#define CRYPTOCFG_32_24_RegisterResetValue 0x0
#define CRYPTOCFG_32_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_24_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_24_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_25 (DWC_ufshc_block_BaseAddress + 0x2064)
#define CRYPTOCFG_32_25_RegisterSize 32
#define CRYPTOCFG_32_25_RegisterResetValue 0x0
#define CRYPTOCFG_32_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_25_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_25_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_26 (DWC_ufshc_block_BaseAddress + 0x2068)
#define CRYPTOCFG_32_26_RegisterSize 32
#define CRYPTOCFG_32_26_RegisterResetValue 0x0
#define CRYPTOCFG_32_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_26_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_26_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_27 (DWC_ufshc_block_BaseAddress + 0x206c)
#define CRYPTOCFG_32_27_RegisterSize 32
#define CRYPTOCFG_32_27_RegisterResetValue 0x0
#define CRYPTOCFG_32_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_27_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_27_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_28 (DWC_ufshc_block_BaseAddress + 0x2070)
#define CRYPTOCFG_32_28_RegisterSize 32
#define CRYPTOCFG_32_28_RegisterResetValue 0x0
#define CRYPTOCFG_32_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_28_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_28_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_29 (DWC_ufshc_block_BaseAddress + 0x2074)
#define CRYPTOCFG_32_29_RegisterSize 32
#define CRYPTOCFG_32_29_RegisterResetValue 0x0
#define CRYPTOCFG_32_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_29_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_29_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_30 (DWC_ufshc_block_BaseAddress + 0x2078)
#define CRYPTOCFG_32_30_RegisterSize 32
#define CRYPTOCFG_32_30_RegisterResetValue 0x0
#define CRYPTOCFG_32_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_30_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_30_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_32_31 (DWC_ufshc_block_BaseAddress + 0x207c)
#define CRYPTOCFG_32_31_RegisterSize 32
#define CRYPTOCFG_32_31_RegisterResetValue 0x0
#define CRYPTOCFG_32_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_32_31_RESERVED_32_BitAddressOffset 0
#define CRYPTOCFG_32_31_RESERVED_32_RegisterSize 32





#define CRYPTOCFG_33_0 (DWC_ufshc_block_BaseAddress + 0x2080)
#define CRYPTOCFG_33_0_RegisterSize 32
#define CRYPTOCFG_33_0_RegisterResetValue 0x0
#define CRYPTOCFG_33_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_0_CRYPTOKEY_33_0_BitAddressOffset 0
#define CRYPTOCFG_33_0_CRYPTOKEY_33_0_RegisterSize 32





#define CRYPTOCFG_33_1 (DWC_ufshc_block_BaseAddress + 0x2084)
#define CRYPTOCFG_33_1_RegisterSize 32
#define CRYPTOCFG_33_1_RegisterResetValue 0x0
#define CRYPTOCFG_33_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_1_CRYPTOKEY_33_1_BitAddressOffset 0
#define CRYPTOCFG_33_1_CRYPTOKEY_33_1_RegisterSize 32





#define CRYPTOCFG_33_2 (DWC_ufshc_block_BaseAddress + 0x2088)
#define CRYPTOCFG_33_2_RegisterSize 32
#define CRYPTOCFG_33_2_RegisterResetValue 0x0
#define CRYPTOCFG_33_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_2_CRYPTOKEY_33_2_BitAddressOffset 0
#define CRYPTOCFG_33_2_CRYPTOKEY_33_2_RegisterSize 32





#define CRYPTOCFG_33_3 (DWC_ufshc_block_BaseAddress + 0x208c)
#define CRYPTOCFG_33_3_RegisterSize 32
#define CRYPTOCFG_33_3_RegisterResetValue 0x0
#define CRYPTOCFG_33_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_3_CRYPTOKEY_33_3_BitAddressOffset 0
#define CRYPTOCFG_33_3_CRYPTOKEY_33_3_RegisterSize 32





#define CRYPTOCFG_33_4 (DWC_ufshc_block_BaseAddress + 0x2090)
#define CRYPTOCFG_33_4_RegisterSize 32
#define CRYPTOCFG_33_4_RegisterResetValue 0x0
#define CRYPTOCFG_33_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_4_CRYPTOKEY_33_4_BitAddressOffset 0
#define CRYPTOCFG_33_4_CRYPTOKEY_33_4_RegisterSize 32





#define CRYPTOCFG_33_5 (DWC_ufshc_block_BaseAddress + 0x2094)
#define CRYPTOCFG_33_5_RegisterSize 32
#define CRYPTOCFG_33_5_RegisterResetValue 0x0
#define CRYPTOCFG_33_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_5_CRYPTOKEY_33_5_BitAddressOffset 0
#define CRYPTOCFG_33_5_CRYPTOKEY_33_5_RegisterSize 32





#define CRYPTOCFG_33_6 (DWC_ufshc_block_BaseAddress + 0x2098)
#define CRYPTOCFG_33_6_RegisterSize 32
#define CRYPTOCFG_33_6_RegisterResetValue 0x0
#define CRYPTOCFG_33_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_6_CRYPTOKEY_33_6_BitAddressOffset 0
#define CRYPTOCFG_33_6_CRYPTOKEY_33_6_RegisterSize 32





#define CRYPTOCFG_33_7 (DWC_ufshc_block_BaseAddress + 0x209c)
#define CRYPTOCFG_33_7_RegisterSize 32
#define CRYPTOCFG_33_7_RegisterResetValue 0x0
#define CRYPTOCFG_33_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_7_CRYPTOKEY_33_7_BitAddressOffset 0
#define CRYPTOCFG_33_7_CRYPTOKEY_33_7_RegisterSize 32





#define CRYPTOCFG_33_8 (DWC_ufshc_block_BaseAddress + 0x20a0)
#define CRYPTOCFG_33_8_RegisterSize 32
#define CRYPTOCFG_33_8_RegisterResetValue 0x0
#define CRYPTOCFG_33_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_8_CRYPTOKEY_33_8_BitAddressOffset 0
#define CRYPTOCFG_33_8_CRYPTOKEY_33_8_RegisterSize 32





#define CRYPTOCFG_33_9 (DWC_ufshc_block_BaseAddress + 0x20a4)
#define CRYPTOCFG_33_9_RegisterSize 32
#define CRYPTOCFG_33_9_RegisterResetValue 0x0
#define CRYPTOCFG_33_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_9_CRYPTOKEY_33_9_BitAddressOffset 0
#define CRYPTOCFG_33_9_CRYPTOKEY_33_9_RegisterSize 32





#define CRYPTOCFG_33_10 (DWC_ufshc_block_BaseAddress + 0x20a8)
#define CRYPTOCFG_33_10_RegisterSize 32
#define CRYPTOCFG_33_10_RegisterResetValue 0x0
#define CRYPTOCFG_33_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_10_CRYPTOKEY_33_10_BitAddressOffset 0
#define CRYPTOCFG_33_10_CRYPTOKEY_33_10_RegisterSize 32





#define CRYPTOCFG_33_11 (DWC_ufshc_block_BaseAddress + 0x20ac)
#define CRYPTOCFG_33_11_RegisterSize 32
#define CRYPTOCFG_33_11_RegisterResetValue 0x0
#define CRYPTOCFG_33_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_11_CRYPTOKEY_33_11_BitAddressOffset 0
#define CRYPTOCFG_33_11_CRYPTOKEY_33_11_RegisterSize 32





#define CRYPTOCFG_33_12 (DWC_ufshc_block_BaseAddress + 0x20b0)
#define CRYPTOCFG_33_12_RegisterSize 32
#define CRYPTOCFG_33_12_RegisterResetValue 0x0
#define CRYPTOCFG_33_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_12_CRYPTOKEY_33_12_BitAddressOffset 0
#define CRYPTOCFG_33_12_CRYPTOKEY_33_12_RegisterSize 32





#define CRYPTOCFG_33_13 (DWC_ufshc_block_BaseAddress + 0x20b4)
#define CRYPTOCFG_33_13_RegisterSize 32
#define CRYPTOCFG_33_13_RegisterResetValue 0x0
#define CRYPTOCFG_33_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_13_CRYPTOKEY_33_13_BitAddressOffset 0
#define CRYPTOCFG_33_13_CRYPTOKEY_33_13_RegisterSize 32





#define CRYPTOCFG_33_14 (DWC_ufshc_block_BaseAddress + 0x20b8)
#define CRYPTOCFG_33_14_RegisterSize 32
#define CRYPTOCFG_33_14_RegisterResetValue 0x0
#define CRYPTOCFG_33_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_14_CRYPTOKEY_33_14_BitAddressOffset 0
#define CRYPTOCFG_33_14_CRYPTOKEY_33_14_RegisterSize 32





#define CRYPTOCFG_33_15 (DWC_ufshc_block_BaseAddress + 0x20bc)
#define CRYPTOCFG_33_15_RegisterSize 32
#define CRYPTOCFG_33_15_RegisterResetValue 0x0
#define CRYPTOCFG_33_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_15_CRYPTOKEY_33_15_BitAddressOffset 0
#define CRYPTOCFG_33_15_CRYPTOKEY_33_15_RegisterSize 32





#define CRYPTOCFG_33_16 (DWC_ufshc_block_BaseAddress + 0x20c0)
#define CRYPTOCFG_33_16_RegisterSize 32
#define CRYPTOCFG_33_16_RegisterResetValue 0x0
#define CRYPTOCFG_33_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_16_DUSIZE_33_BitAddressOffset 0
#define CRYPTOCFG_33_16_DUSIZE_33_RegisterSize 8



#define CRYPTOCFG_33_16_CAPIDX_33_BitAddressOffset 8
#define CRYPTOCFG_33_16_CAPIDX_33_RegisterSize 8



#define CRYPTOCFG_33_16_CFGE_33_BitAddressOffset 31
#define CRYPTOCFG_33_16_CFGE_33_RegisterSize 1





#define CRYPTOCFG_33_17 (DWC_ufshc_block_BaseAddress + 0x20c4)
#define CRYPTOCFG_33_17_RegisterSize 32
#define CRYPTOCFG_33_17_RegisterResetValue 0x0
#define CRYPTOCFG_33_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_17_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_17_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_18 (DWC_ufshc_block_BaseAddress + 0x20c8)
#define CRYPTOCFG_33_18_RegisterSize 32
#define CRYPTOCFG_33_18_RegisterResetValue 0x0
#define CRYPTOCFG_33_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_18_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_18_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_19 (DWC_ufshc_block_BaseAddress + 0x20cc)
#define CRYPTOCFG_33_19_RegisterSize 32
#define CRYPTOCFG_33_19_RegisterResetValue 0x0
#define CRYPTOCFG_33_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_19_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_19_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_20 (DWC_ufshc_block_BaseAddress + 0x20d0)
#define CRYPTOCFG_33_20_RegisterSize 32
#define CRYPTOCFG_33_20_RegisterResetValue 0x0
#define CRYPTOCFG_33_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_20_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_20_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_21 (DWC_ufshc_block_BaseAddress + 0x20d4)
#define CRYPTOCFG_33_21_RegisterSize 32
#define CRYPTOCFG_33_21_RegisterResetValue 0x0
#define CRYPTOCFG_33_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_21_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_21_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_22 (DWC_ufshc_block_BaseAddress + 0x20d8)
#define CRYPTOCFG_33_22_RegisterSize 32
#define CRYPTOCFG_33_22_RegisterResetValue 0x0
#define CRYPTOCFG_33_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_22_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_22_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_23 (DWC_ufshc_block_BaseAddress + 0x20dc)
#define CRYPTOCFG_33_23_RegisterSize 32
#define CRYPTOCFG_33_23_RegisterResetValue 0x0
#define CRYPTOCFG_33_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_23_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_23_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_24 (DWC_ufshc_block_BaseAddress + 0x20e0)
#define CRYPTOCFG_33_24_RegisterSize 32
#define CRYPTOCFG_33_24_RegisterResetValue 0x0
#define CRYPTOCFG_33_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_24_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_24_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_25 (DWC_ufshc_block_BaseAddress + 0x20e4)
#define CRYPTOCFG_33_25_RegisterSize 32
#define CRYPTOCFG_33_25_RegisterResetValue 0x0
#define CRYPTOCFG_33_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_25_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_25_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_26 (DWC_ufshc_block_BaseAddress + 0x20e8)
#define CRYPTOCFG_33_26_RegisterSize 32
#define CRYPTOCFG_33_26_RegisterResetValue 0x0
#define CRYPTOCFG_33_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_26_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_26_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_27 (DWC_ufshc_block_BaseAddress + 0x20ec)
#define CRYPTOCFG_33_27_RegisterSize 32
#define CRYPTOCFG_33_27_RegisterResetValue 0x0
#define CRYPTOCFG_33_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_27_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_27_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_28 (DWC_ufshc_block_BaseAddress + 0x20f0)
#define CRYPTOCFG_33_28_RegisterSize 32
#define CRYPTOCFG_33_28_RegisterResetValue 0x0
#define CRYPTOCFG_33_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_28_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_28_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_29 (DWC_ufshc_block_BaseAddress + 0x20f4)
#define CRYPTOCFG_33_29_RegisterSize 32
#define CRYPTOCFG_33_29_RegisterResetValue 0x0
#define CRYPTOCFG_33_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_29_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_29_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_30 (DWC_ufshc_block_BaseAddress + 0x20f8)
#define CRYPTOCFG_33_30_RegisterSize 32
#define CRYPTOCFG_33_30_RegisterResetValue 0x0
#define CRYPTOCFG_33_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_30_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_30_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_33_31 (DWC_ufshc_block_BaseAddress + 0x20fc)
#define CRYPTOCFG_33_31_RegisterSize 32
#define CRYPTOCFG_33_31_RegisterResetValue 0x0
#define CRYPTOCFG_33_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_33_31_RESERVED_33_BitAddressOffset 0
#define CRYPTOCFG_33_31_RESERVED_33_RegisterSize 32





#define CRYPTOCFG_34_0 (DWC_ufshc_block_BaseAddress + 0x2100)
#define CRYPTOCFG_34_0_RegisterSize 32
#define CRYPTOCFG_34_0_RegisterResetValue 0x0
#define CRYPTOCFG_34_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_0_CRYPTOKEY_34_0_BitAddressOffset 0
#define CRYPTOCFG_34_0_CRYPTOKEY_34_0_RegisterSize 32





#define CRYPTOCFG_34_1 (DWC_ufshc_block_BaseAddress + 0x2104)
#define CRYPTOCFG_34_1_RegisterSize 32
#define CRYPTOCFG_34_1_RegisterResetValue 0x0
#define CRYPTOCFG_34_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_1_CRYPTOKEY_34_1_BitAddressOffset 0
#define CRYPTOCFG_34_1_CRYPTOKEY_34_1_RegisterSize 32





#define CRYPTOCFG_34_2 (DWC_ufshc_block_BaseAddress + 0x2108)
#define CRYPTOCFG_34_2_RegisterSize 32
#define CRYPTOCFG_34_2_RegisterResetValue 0x0
#define CRYPTOCFG_34_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_2_CRYPTOKEY_34_2_BitAddressOffset 0
#define CRYPTOCFG_34_2_CRYPTOKEY_34_2_RegisterSize 32





#define CRYPTOCFG_34_3 (DWC_ufshc_block_BaseAddress + 0x210c)
#define CRYPTOCFG_34_3_RegisterSize 32
#define CRYPTOCFG_34_3_RegisterResetValue 0x0
#define CRYPTOCFG_34_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_3_CRYPTOKEY_34_3_BitAddressOffset 0
#define CRYPTOCFG_34_3_CRYPTOKEY_34_3_RegisterSize 32





#define CRYPTOCFG_34_4 (DWC_ufshc_block_BaseAddress + 0x2110)
#define CRYPTOCFG_34_4_RegisterSize 32
#define CRYPTOCFG_34_4_RegisterResetValue 0x0
#define CRYPTOCFG_34_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_4_CRYPTOKEY_34_4_BitAddressOffset 0
#define CRYPTOCFG_34_4_CRYPTOKEY_34_4_RegisterSize 32





#define CRYPTOCFG_34_5 (DWC_ufshc_block_BaseAddress + 0x2114)
#define CRYPTOCFG_34_5_RegisterSize 32
#define CRYPTOCFG_34_5_RegisterResetValue 0x0
#define CRYPTOCFG_34_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_5_CRYPTOKEY_34_5_BitAddressOffset 0
#define CRYPTOCFG_34_5_CRYPTOKEY_34_5_RegisterSize 32





#define CRYPTOCFG_34_6 (DWC_ufshc_block_BaseAddress + 0x2118)
#define CRYPTOCFG_34_6_RegisterSize 32
#define CRYPTOCFG_34_6_RegisterResetValue 0x0
#define CRYPTOCFG_34_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_6_CRYPTOKEY_34_6_BitAddressOffset 0
#define CRYPTOCFG_34_6_CRYPTOKEY_34_6_RegisterSize 32





#define CRYPTOCFG_34_7 (DWC_ufshc_block_BaseAddress + 0x211c)
#define CRYPTOCFG_34_7_RegisterSize 32
#define CRYPTOCFG_34_7_RegisterResetValue 0x0
#define CRYPTOCFG_34_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_7_CRYPTOKEY_34_7_BitAddressOffset 0
#define CRYPTOCFG_34_7_CRYPTOKEY_34_7_RegisterSize 32





#define CRYPTOCFG_34_8 (DWC_ufshc_block_BaseAddress + 0x2120)
#define CRYPTOCFG_34_8_RegisterSize 32
#define CRYPTOCFG_34_8_RegisterResetValue 0x0
#define CRYPTOCFG_34_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_8_CRYPTOKEY_34_8_BitAddressOffset 0
#define CRYPTOCFG_34_8_CRYPTOKEY_34_8_RegisterSize 32





#define CRYPTOCFG_34_9 (DWC_ufshc_block_BaseAddress + 0x2124)
#define CRYPTOCFG_34_9_RegisterSize 32
#define CRYPTOCFG_34_9_RegisterResetValue 0x0
#define CRYPTOCFG_34_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_9_CRYPTOKEY_34_9_BitAddressOffset 0
#define CRYPTOCFG_34_9_CRYPTOKEY_34_9_RegisterSize 32





#define CRYPTOCFG_34_10 (DWC_ufshc_block_BaseAddress + 0x2128)
#define CRYPTOCFG_34_10_RegisterSize 32
#define CRYPTOCFG_34_10_RegisterResetValue 0x0
#define CRYPTOCFG_34_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_10_CRYPTOKEY_34_10_BitAddressOffset 0
#define CRYPTOCFG_34_10_CRYPTOKEY_34_10_RegisterSize 32





#define CRYPTOCFG_34_11 (DWC_ufshc_block_BaseAddress + 0x212c)
#define CRYPTOCFG_34_11_RegisterSize 32
#define CRYPTOCFG_34_11_RegisterResetValue 0x0
#define CRYPTOCFG_34_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_11_CRYPTOKEY_34_11_BitAddressOffset 0
#define CRYPTOCFG_34_11_CRYPTOKEY_34_11_RegisterSize 32





#define CRYPTOCFG_34_12 (DWC_ufshc_block_BaseAddress + 0x2130)
#define CRYPTOCFG_34_12_RegisterSize 32
#define CRYPTOCFG_34_12_RegisterResetValue 0x0
#define CRYPTOCFG_34_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_12_CRYPTOKEY_34_12_BitAddressOffset 0
#define CRYPTOCFG_34_12_CRYPTOKEY_34_12_RegisterSize 32





#define CRYPTOCFG_34_13 (DWC_ufshc_block_BaseAddress + 0x2134)
#define CRYPTOCFG_34_13_RegisterSize 32
#define CRYPTOCFG_34_13_RegisterResetValue 0x0
#define CRYPTOCFG_34_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_13_CRYPTOKEY_34_13_BitAddressOffset 0
#define CRYPTOCFG_34_13_CRYPTOKEY_34_13_RegisterSize 32





#define CRYPTOCFG_34_14 (DWC_ufshc_block_BaseAddress + 0x2138)
#define CRYPTOCFG_34_14_RegisterSize 32
#define CRYPTOCFG_34_14_RegisterResetValue 0x0
#define CRYPTOCFG_34_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_14_CRYPTOKEY_34_14_BitAddressOffset 0
#define CRYPTOCFG_34_14_CRYPTOKEY_34_14_RegisterSize 32





#define CRYPTOCFG_34_15 (DWC_ufshc_block_BaseAddress + 0x213c)
#define CRYPTOCFG_34_15_RegisterSize 32
#define CRYPTOCFG_34_15_RegisterResetValue 0x0
#define CRYPTOCFG_34_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_15_CRYPTOKEY_34_15_BitAddressOffset 0
#define CRYPTOCFG_34_15_CRYPTOKEY_34_15_RegisterSize 32





#define CRYPTOCFG_34_16 (DWC_ufshc_block_BaseAddress + 0x2140)
#define CRYPTOCFG_34_16_RegisterSize 32
#define CRYPTOCFG_34_16_RegisterResetValue 0x0
#define CRYPTOCFG_34_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_16_DUSIZE_34_BitAddressOffset 0
#define CRYPTOCFG_34_16_DUSIZE_34_RegisterSize 8



#define CRYPTOCFG_34_16_CAPIDX_34_BitAddressOffset 8
#define CRYPTOCFG_34_16_CAPIDX_34_RegisterSize 8



#define CRYPTOCFG_34_16_CFGE_34_BitAddressOffset 31
#define CRYPTOCFG_34_16_CFGE_34_RegisterSize 1





#define CRYPTOCFG_34_17 (DWC_ufshc_block_BaseAddress + 0x2144)
#define CRYPTOCFG_34_17_RegisterSize 32
#define CRYPTOCFG_34_17_RegisterResetValue 0x0
#define CRYPTOCFG_34_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_17_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_17_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_18 (DWC_ufshc_block_BaseAddress + 0x2148)
#define CRYPTOCFG_34_18_RegisterSize 32
#define CRYPTOCFG_34_18_RegisterResetValue 0x0
#define CRYPTOCFG_34_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_18_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_18_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_19 (DWC_ufshc_block_BaseAddress + 0x214c)
#define CRYPTOCFG_34_19_RegisterSize 32
#define CRYPTOCFG_34_19_RegisterResetValue 0x0
#define CRYPTOCFG_34_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_19_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_19_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_20 (DWC_ufshc_block_BaseAddress + 0x2150)
#define CRYPTOCFG_34_20_RegisterSize 32
#define CRYPTOCFG_34_20_RegisterResetValue 0x0
#define CRYPTOCFG_34_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_20_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_20_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_21 (DWC_ufshc_block_BaseAddress + 0x2154)
#define CRYPTOCFG_34_21_RegisterSize 32
#define CRYPTOCFG_34_21_RegisterResetValue 0x0
#define CRYPTOCFG_34_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_21_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_21_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_22 (DWC_ufshc_block_BaseAddress + 0x2158)
#define CRYPTOCFG_34_22_RegisterSize 32
#define CRYPTOCFG_34_22_RegisterResetValue 0x0
#define CRYPTOCFG_34_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_22_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_22_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_23 (DWC_ufshc_block_BaseAddress + 0x215c)
#define CRYPTOCFG_34_23_RegisterSize 32
#define CRYPTOCFG_34_23_RegisterResetValue 0x0
#define CRYPTOCFG_34_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_23_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_23_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_24 (DWC_ufshc_block_BaseAddress + 0x2160)
#define CRYPTOCFG_34_24_RegisterSize 32
#define CRYPTOCFG_34_24_RegisterResetValue 0x0
#define CRYPTOCFG_34_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_24_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_24_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_25 (DWC_ufshc_block_BaseAddress + 0x2164)
#define CRYPTOCFG_34_25_RegisterSize 32
#define CRYPTOCFG_34_25_RegisterResetValue 0x0
#define CRYPTOCFG_34_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_25_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_25_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_26 (DWC_ufshc_block_BaseAddress + 0x2168)
#define CRYPTOCFG_34_26_RegisterSize 32
#define CRYPTOCFG_34_26_RegisterResetValue 0x0
#define CRYPTOCFG_34_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_26_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_26_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_27 (DWC_ufshc_block_BaseAddress + 0x216c)
#define CRYPTOCFG_34_27_RegisterSize 32
#define CRYPTOCFG_34_27_RegisterResetValue 0x0
#define CRYPTOCFG_34_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_27_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_27_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_28 (DWC_ufshc_block_BaseAddress + 0x2170)
#define CRYPTOCFG_34_28_RegisterSize 32
#define CRYPTOCFG_34_28_RegisterResetValue 0x0
#define CRYPTOCFG_34_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_28_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_28_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_29 (DWC_ufshc_block_BaseAddress + 0x2174)
#define CRYPTOCFG_34_29_RegisterSize 32
#define CRYPTOCFG_34_29_RegisterResetValue 0x0
#define CRYPTOCFG_34_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_29_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_29_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_30 (DWC_ufshc_block_BaseAddress + 0x2178)
#define CRYPTOCFG_34_30_RegisterSize 32
#define CRYPTOCFG_34_30_RegisterResetValue 0x0
#define CRYPTOCFG_34_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_30_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_30_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_34_31 (DWC_ufshc_block_BaseAddress + 0x217c)
#define CRYPTOCFG_34_31_RegisterSize 32
#define CRYPTOCFG_34_31_RegisterResetValue 0x0
#define CRYPTOCFG_34_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_34_31_RESERVED_34_BitAddressOffset 0
#define CRYPTOCFG_34_31_RESERVED_34_RegisterSize 32





#define CRYPTOCFG_35_0 (DWC_ufshc_block_BaseAddress + 0x2180)
#define CRYPTOCFG_35_0_RegisterSize 32
#define CRYPTOCFG_35_0_RegisterResetValue 0x0
#define CRYPTOCFG_35_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_0_CRYPTOKEY_35_0_BitAddressOffset 0
#define CRYPTOCFG_35_0_CRYPTOKEY_35_0_RegisterSize 32





#define CRYPTOCFG_35_1 (DWC_ufshc_block_BaseAddress + 0x2184)
#define CRYPTOCFG_35_1_RegisterSize 32
#define CRYPTOCFG_35_1_RegisterResetValue 0x0
#define CRYPTOCFG_35_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_1_CRYPTOKEY_35_1_BitAddressOffset 0
#define CRYPTOCFG_35_1_CRYPTOKEY_35_1_RegisterSize 32





#define CRYPTOCFG_35_2 (DWC_ufshc_block_BaseAddress + 0x2188)
#define CRYPTOCFG_35_2_RegisterSize 32
#define CRYPTOCFG_35_2_RegisterResetValue 0x0
#define CRYPTOCFG_35_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_2_CRYPTOKEY_35_2_BitAddressOffset 0
#define CRYPTOCFG_35_2_CRYPTOKEY_35_2_RegisterSize 32





#define CRYPTOCFG_35_3 (DWC_ufshc_block_BaseAddress + 0x218c)
#define CRYPTOCFG_35_3_RegisterSize 32
#define CRYPTOCFG_35_3_RegisterResetValue 0x0
#define CRYPTOCFG_35_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_3_CRYPTOKEY_35_3_BitAddressOffset 0
#define CRYPTOCFG_35_3_CRYPTOKEY_35_3_RegisterSize 32





#define CRYPTOCFG_35_4 (DWC_ufshc_block_BaseAddress + 0x2190)
#define CRYPTOCFG_35_4_RegisterSize 32
#define CRYPTOCFG_35_4_RegisterResetValue 0x0
#define CRYPTOCFG_35_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_4_CRYPTOKEY_35_4_BitAddressOffset 0
#define CRYPTOCFG_35_4_CRYPTOKEY_35_4_RegisterSize 32





#define CRYPTOCFG_35_5 (DWC_ufshc_block_BaseAddress + 0x2194)
#define CRYPTOCFG_35_5_RegisterSize 32
#define CRYPTOCFG_35_5_RegisterResetValue 0x0
#define CRYPTOCFG_35_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_5_CRYPTOKEY_35_5_BitAddressOffset 0
#define CRYPTOCFG_35_5_CRYPTOKEY_35_5_RegisterSize 32





#define CRYPTOCFG_35_6 (DWC_ufshc_block_BaseAddress + 0x2198)
#define CRYPTOCFG_35_6_RegisterSize 32
#define CRYPTOCFG_35_6_RegisterResetValue 0x0
#define CRYPTOCFG_35_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_6_CRYPTOKEY_35_6_BitAddressOffset 0
#define CRYPTOCFG_35_6_CRYPTOKEY_35_6_RegisterSize 32





#define CRYPTOCFG_35_7 (DWC_ufshc_block_BaseAddress + 0x219c)
#define CRYPTOCFG_35_7_RegisterSize 32
#define CRYPTOCFG_35_7_RegisterResetValue 0x0
#define CRYPTOCFG_35_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_7_CRYPTOKEY_35_7_BitAddressOffset 0
#define CRYPTOCFG_35_7_CRYPTOKEY_35_7_RegisterSize 32





#define CRYPTOCFG_35_8 (DWC_ufshc_block_BaseAddress + 0x21a0)
#define CRYPTOCFG_35_8_RegisterSize 32
#define CRYPTOCFG_35_8_RegisterResetValue 0x0
#define CRYPTOCFG_35_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_8_CRYPTOKEY_35_8_BitAddressOffset 0
#define CRYPTOCFG_35_8_CRYPTOKEY_35_8_RegisterSize 32





#define CRYPTOCFG_35_9 (DWC_ufshc_block_BaseAddress + 0x21a4)
#define CRYPTOCFG_35_9_RegisterSize 32
#define CRYPTOCFG_35_9_RegisterResetValue 0x0
#define CRYPTOCFG_35_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_9_CRYPTOKEY_35_9_BitAddressOffset 0
#define CRYPTOCFG_35_9_CRYPTOKEY_35_9_RegisterSize 32





#define CRYPTOCFG_35_10 (DWC_ufshc_block_BaseAddress + 0x21a8)
#define CRYPTOCFG_35_10_RegisterSize 32
#define CRYPTOCFG_35_10_RegisterResetValue 0x0
#define CRYPTOCFG_35_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_10_CRYPTOKEY_35_10_BitAddressOffset 0
#define CRYPTOCFG_35_10_CRYPTOKEY_35_10_RegisterSize 32





#define CRYPTOCFG_35_11 (DWC_ufshc_block_BaseAddress + 0x21ac)
#define CRYPTOCFG_35_11_RegisterSize 32
#define CRYPTOCFG_35_11_RegisterResetValue 0x0
#define CRYPTOCFG_35_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_11_CRYPTOKEY_35_11_BitAddressOffset 0
#define CRYPTOCFG_35_11_CRYPTOKEY_35_11_RegisterSize 32





#define CRYPTOCFG_35_12 (DWC_ufshc_block_BaseAddress + 0x21b0)
#define CRYPTOCFG_35_12_RegisterSize 32
#define CRYPTOCFG_35_12_RegisterResetValue 0x0
#define CRYPTOCFG_35_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_12_CRYPTOKEY_35_12_BitAddressOffset 0
#define CRYPTOCFG_35_12_CRYPTOKEY_35_12_RegisterSize 32





#define CRYPTOCFG_35_13 (DWC_ufshc_block_BaseAddress + 0x21b4)
#define CRYPTOCFG_35_13_RegisterSize 32
#define CRYPTOCFG_35_13_RegisterResetValue 0x0
#define CRYPTOCFG_35_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_13_CRYPTOKEY_35_13_BitAddressOffset 0
#define CRYPTOCFG_35_13_CRYPTOKEY_35_13_RegisterSize 32





#define CRYPTOCFG_35_14 (DWC_ufshc_block_BaseAddress + 0x21b8)
#define CRYPTOCFG_35_14_RegisterSize 32
#define CRYPTOCFG_35_14_RegisterResetValue 0x0
#define CRYPTOCFG_35_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_14_CRYPTOKEY_35_14_BitAddressOffset 0
#define CRYPTOCFG_35_14_CRYPTOKEY_35_14_RegisterSize 32





#define CRYPTOCFG_35_15 (DWC_ufshc_block_BaseAddress + 0x21bc)
#define CRYPTOCFG_35_15_RegisterSize 32
#define CRYPTOCFG_35_15_RegisterResetValue 0x0
#define CRYPTOCFG_35_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_15_CRYPTOKEY_35_15_BitAddressOffset 0
#define CRYPTOCFG_35_15_CRYPTOKEY_35_15_RegisterSize 32





#define CRYPTOCFG_35_16 (DWC_ufshc_block_BaseAddress + 0x21c0)
#define CRYPTOCFG_35_16_RegisterSize 32
#define CRYPTOCFG_35_16_RegisterResetValue 0x0
#define CRYPTOCFG_35_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_16_DUSIZE_35_BitAddressOffset 0
#define CRYPTOCFG_35_16_DUSIZE_35_RegisterSize 8



#define CRYPTOCFG_35_16_CAPIDX_35_BitAddressOffset 8
#define CRYPTOCFG_35_16_CAPIDX_35_RegisterSize 8



#define CRYPTOCFG_35_16_CFGE_35_BitAddressOffset 31
#define CRYPTOCFG_35_16_CFGE_35_RegisterSize 1





#define CRYPTOCFG_35_17 (DWC_ufshc_block_BaseAddress + 0x21c4)
#define CRYPTOCFG_35_17_RegisterSize 32
#define CRYPTOCFG_35_17_RegisterResetValue 0x0
#define CRYPTOCFG_35_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_17_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_17_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_18 (DWC_ufshc_block_BaseAddress + 0x21c8)
#define CRYPTOCFG_35_18_RegisterSize 32
#define CRYPTOCFG_35_18_RegisterResetValue 0x0
#define CRYPTOCFG_35_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_18_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_18_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_19 (DWC_ufshc_block_BaseAddress + 0x21cc)
#define CRYPTOCFG_35_19_RegisterSize 32
#define CRYPTOCFG_35_19_RegisterResetValue 0x0
#define CRYPTOCFG_35_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_19_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_19_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_20 (DWC_ufshc_block_BaseAddress + 0x21d0)
#define CRYPTOCFG_35_20_RegisterSize 32
#define CRYPTOCFG_35_20_RegisterResetValue 0x0
#define CRYPTOCFG_35_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_20_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_20_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_21 (DWC_ufshc_block_BaseAddress + 0x21d4)
#define CRYPTOCFG_35_21_RegisterSize 32
#define CRYPTOCFG_35_21_RegisterResetValue 0x0
#define CRYPTOCFG_35_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_21_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_21_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_22 (DWC_ufshc_block_BaseAddress + 0x21d8)
#define CRYPTOCFG_35_22_RegisterSize 32
#define CRYPTOCFG_35_22_RegisterResetValue 0x0
#define CRYPTOCFG_35_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_22_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_22_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_23 (DWC_ufshc_block_BaseAddress + 0x21dc)
#define CRYPTOCFG_35_23_RegisterSize 32
#define CRYPTOCFG_35_23_RegisterResetValue 0x0
#define CRYPTOCFG_35_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_23_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_23_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_24 (DWC_ufshc_block_BaseAddress + 0x21e0)
#define CRYPTOCFG_35_24_RegisterSize 32
#define CRYPTOCFG_35_24_RegisterResetValue 0x0
#define CRYPTOCFG_35_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_24_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_24_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_25 (DWC_ufshc_block_BaseAddress + 0x21e4)
#define CRYPTOCFG_35_25_RegisterSize 32
#define CRYPTOCFG_35_25_RegisterResetValue 0x0
#define CRYPTOCFG_35_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_25_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_25_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_26 (DWC_ufshc_block_BaseAddress + 0x21e8)
#define CRYPTOCFG_35_26_RegisterSize 32
#define CRYPTOCFG_35_26_RegisterResetValue 0x0
#define CRYPTOCFG_35_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_26_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_26_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_27 (DWC_ufshc_block_BaseAddress + 0x21ec)
#define CRYPTOCFG_35_27_RegisterSize 32
#define CRYPTOCFG_35_27_RegisterResetValue 0x0
#define CRYPTOCFG_35_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_27_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_27_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_28 (DWC_ufshc_block_BaseAddress + 0x21f0)
#define CRYPTOCFG_35_28_RegisterSize 32
#define CRYPTOCFG_35_28_RegisterResetValue 0x0
#define CRYPTOCFG_35_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_28_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_28_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_29 (DWC_ufshc_block_BaseAddress + 0x21f4)
#define CRYPTOCFG_35_29_RegisterSize 32
#define CRYPTOCFG_35_29_RegisterResetValue 0x0
#define CRYPTOCFG_35_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_29_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_29_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_30 (DWC_ufshc_block_BaseAddress + 0x21f8)
#define CRYPTOCFG_35_30_RegisterSize 32
#define CRYPTOCFG_35_30_RegisterResetValue 0x0
#define CRYPTOCFG_35_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_30_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_30_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_35_31 (DWC_ufshc_block_BaseAddress + 0x21fc)
#define CRYPTOCFG_35_31_RegisterSize 32
#define CRYPTOCFG_35_31_RegisterResetValue 0x0
#define CRYPTOCFG_35_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_35_31_RESERVED_35_BitAddressOffset 0
#define CRYPTOCFG_35_31_RESERVED_35_RegisterSize 32





#define CRYPTOCFG_36_0 (DWC_ufshc_block_BaseAddress + 0x2200)
#define CRYPTOCFG_36_0_RegisterSize 32
#define CRYPTOCFG_36_0_RegisterResetValue 0x0
#define CRYPTOCFG_36_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_0_CRYPTOKEY_36_0_BitAddressOffset 0
#define CRYPTOCFG_36_0_CRYPTOKEY_36_0_RegisterSize 32





#define CRYPTOCFG_36_1 (DWC_ufshc_block_BaseAddress + 0x2204)
#define CRYPTOCFG_36_1_RegisterSize 32
#define CRYPTOCFG_36_1_RegisterResetValue 0x0
#define CRYPTOCFG_36_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_1_CRYPTOKEY_36_1_BitAddressOffset 0
#define CRYPTOCFG_36_1_CRYPTOKEY_36_1_RegisterSize 32





#define CRYPTOCFG_36_2 (DWC_ufshc_block_BaseAddress + 0x2208)
#define CRYPTOCFG_36_2_RegisterSize 32
#define CRYPTOCFG_36_2_RegisterResetValue 0x0
#define CRYPTOCFG_36_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_2_CRYPTOKEY_36_2_BitAddressOffset 0
#define CRYPTOCFG_36_2_CRYPTOKEY_36_2_RegisterSize 32





#define CRYPTOCFG_36_3 (DWC_ufshc_block_BaseAddress + 0x220c)
#define CRYPTOCFG_36_3_RegisterSize 32
#define CRYPTOCFG_36_3_RegisterResetValue 0x0
#define CRYPTOCFG_36_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_3_CRYPTOKEY_36_3_BitAddressOffset 0
#define CRYPTOCFG_36_3_CRYPTOKEY_36_3_RegisterSize 32





#define CRYPTOCFG_36_4 (DWC_ufshc_block_BaseAddress + 0x2210)
#define CRYPTOCFG_36_4_RegisterSize 32
#define CRYPTOCFG_36_4_RegisterResetValue 0x0
#define CRYPTOCFG_36_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_4_CRYPTOKEY_36_4_BitAddressOffset 0
#define CRYPTOCFG_36_4_CRYPTOKEY_36_4_RegisterSize 32





#define CRYPTOCFG_36_5 (DWC_ufshc_block_BaseAddress + 0x2214)
#define CRYPTOCFG_36_5_RegisterSize 32
#define CRYPTOCFG_36_5_RegisterResetValue 0x0
#define CRYPTOCFG_36_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_5_CRYPTOKEY_36_5_BitAddressOffset 0
#define CRYPTOCFG_36_5_CRYPTOKEY_36_5_RegisterSize 32





#define CRYPTOCFG_36_6 (DWC_ufshc_block_BaseAddress + 0x2218)
#define CRYPTOCFG_36_6_RegisterSize 32
#define CRYPTOCFG_36_6_RegisterResetValue 0x0
#define CRYPTOCFG_36_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_6_CRYPTOKEY_36_6_BitAddressOffset 0
#define CRYPTOCFG_36_6_CRYPTOKEY_36_6_RegisterSize 32





#define CRYPTOCFG_36_7 (DWC_ufshc_block_BaseAddress + 0x221c)
#define CRYPTOCFG_36_7_RegisterSize 32
#define CRYPTOCFG_36_7_RegisterResetValue 0x0
#define CRYPTOCFG_36_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_7_CRYPTOKEY_36_7_BitAddressOffset 0
#define CRYPTOCFG_36_7_CRYPTOKEY_36_7_RegisterSize 32





#define CRYPTOCFG_36_8 (DWC_ufshc_block_BaseAddress + 0x2220)
#define CRYPTOCFG_36_8_RegisterSize 32
#define CRYPTOCFG_36_8_RegisterResetValue 0x0
#define CRYPTOCFG_36_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_8_CRYPTOKEY_36_8_BitAddressOffset 0
#define CRYPTOCFG_36_8_CRYPTOKEY_36_8_RegisterSize 32





#define CRYPTOCFG_36_9 (DWC_ufshc_block_BaseAddress + 0x2224)
#define CRYPTOCFG_36_9_RegisterSize 32
#define CRYPTOCFG_36_9_RegisterResetValue 0x0
#define CRYPTOCFG_36_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_9_CRYPTOKEY_36_9_BitAddressOffset 0
#define CRYPTOCFG_36_9_CRYPTOKEY_36_9_RegisterSize 32





#define CRYPTOCFG_36_10 (DWC_ufshc_block_BaseAddress + 0x2228)
#define CRYPTOCFG_36_10_RegisterSize 32
#define CRYPTOCFG_36_10_RegisterResetValue 0x0
#define CRYPTOCFG_36_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_10_CRYPTOKEY_36_10_BitAddressOffset 0
#define CRYPTOCFG_36_10_CRYPTOKEY_36_10_RegisterSize 32





#define CRYPTOCFG_36_11 (DWC_ufshc_block_BaseAddress + 0x222c)
#define CRYPTOCFG_36_11_RegisterSize 32
#define CRYPTOCFG_36_11_RegisterResetValue 0x0
#define CRYPTOCFG_36_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_11_CRYPTOKEY_36_11_BitAddressOffset 0
#define CRYPTOCFG_36_11_CRYPTOKEY_36_11_RegisterSize 32





#define CRYPTOCFG_36_12 (DWC_ufshc_block_BaseAddress + 0x2230)
#define CRYPTOCFG_36_12_RegisterSize 32
#define CRYPTOCFG_36_12_RegisterResetValue 0x0
#define CRYPTOCFG_36_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_12_CRYPTOKEY_36_12_BitAddressOffset 0
#define CRYPTOCFG_36_12_CRYPTOKEY_36_12_RegisterSize 32





#define CRYPTOCFG_36_13 (DWC_ufshc_block_BaseAddress + 0x2234)
#define CRYPTOCFG_36_13_RegisterSize 32
#define CRYPTOCFG_36_13_RegisterResetValue 0x0
#define CRYPTOCFG_36_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_13_CRYPTOKEY_36_13_BitAddressOffset 0
#define CRYPTOCFG_36_13_CRYPTOKEY_36_13_RegisterSize 32





#define CRYPTOCFG_36_14 (DWC_ufshc_block_BaseAddress + 0x2238)
#define CRYPTOCFG_36_14_RegisterSize 32
#define CRYPTOCFG_36_14_RegisterResetValue 0x0
#define CRYPTOCFG_36_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_14_CRYPTOKEY_36_14_BitAddressOffset 0
#define CRYPTOCFG_36_14_CRYPTOKEY_36_14_RegisterSize 32





#define CRYPTOCFG_36_15 (DWC_ufshc_block_BaseAddress + 0x223c)
#define CRYPTOCFG_36_15_RegisterSize 32
#define CRYPTOCFG_36_15_RegisterResetValue 0x0
#define CRYPTOCFG_36_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_15_CRYPTOKEY_36_15_BitAddressOffset 0
#define CRYPTOCFG_36_15_CRYPTOKEY_36_15_RegisterSize 32





#define CRYPTOCFG_36_16 (DWC_ufshc_block_BaseAddress + 0x2240)
#define CRYPTOCFG_36_16_RegisterSize 32
#define CRYPTOCFG_36_16_RegisterResetValue 0x0
#define CRYPTOCFG_36_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_16_DUSIZE_36_BitAddressOffset 0
#define CRYPTOCFG_36_16_DUSIZE_36_RegisterSize 8



#define CRYPTOCFG_36_16_CAPIDX_36_BitAddressOffset 8
#define CRYPTOCFG_36_16_CAPIDX_36_RegisterSize 8



#define CRYPTOCFG_36_16_CFGE_36_BitAddressOffset 31
#define CRYPTOCFG_36_16_CFGE_36_RegisterSize 1





#define CRYPTOCFG_36_17 (DWC_ufshc_block_BaseAddress + 0x2244)
#define CRYPTOCFG_36_17_RegisterSize 32
#define CRYPTOCFG_36_17_RegisterResetValue 0x0
#define CRYPTOCFG_36_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_17_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_17_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_18 (DWC_ufshc_block_BaseAddress + 0x2248)
#define CRYPTOCFG_36_18_RegisterSize 32
#define CRYPTOCFG_36_18_RegisterResetValue 0x0
#define CRYPTOCFG_36_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_18_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_18_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_19 (DWC_ufshc_block_BaseAddress + 0x224c)
#define CRYPTOCFG_36_19_RegisterSize 32
#define CRYPTOCFG_36_19_RegisterResetValue 0x0
#define CRYPTOCFG_36_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_19_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_19_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_20 (DWC_ufshc_block_BaseAddress + 0x2250)
#define CRYPTOCFG_36_20_RegisterSize 32
#define CRYPTOCFG_36_20_RegisterResetValue 0x0
#define CRYPTOCFG_36_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_20_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_20_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_21 (DWC_ufshc_block_BaseAddress + 0x2254)
#define CRYPTOCFG_36_21_RegisterSize 32
#define CRYPTOCFG_36_21_RegisterResetValue 0x0
#define CRYPTOCFG_36_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_21_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_21_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_22 (DWC_ufshc_block_BaseAddress + 0x2258)
#define CRYPTOCFG_36_22_RegisterSize 32
#define CRYPTOCFG_36_22_RegisterResetValue 0x0
#define CRYPTOCFG_36_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_22_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_22_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_23 (DWC_ufshc_block_BaseAddress + 0x225c)
#define CRYPTOCFG_36_23_RegisterSize 32
#define CRYPTOCFG_36_23_RegisterResetValue 0x0
#define CRYPTOCFG_36_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_23_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_23_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_24 (DWC_ufshc_block_BaseAddress + 0x2260)
#define CRYPTOCFG_36_24_RegisterSize 32
#define CRYPTOCFG_36_24_RegisterResetValue 0x0
#define CRYPTOCFG_36_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_24_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_24_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_25 (DWC_ufshc_block_BaseAddress + 0x2264)
#define CRYPTOCFG_36_25_RegisterSize 32
#define CRYPTOCFG_36_25_RegisterResetValue 0x0
#define CRYPTOCFG_36_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_25_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_25_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_26 (DWC_ufshc_block_BaseAddress + 0x2268)
#define CRYPTOCFG_36_26_RegisterSize 32
#define CRYPTOCFG_36_26_RegisterResetValue 0x0
#define CRYPTOCFG_36_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_26_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_26_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_27 (DWC_ufshc_block_BaseAddress + 0x226c)
#define CRYPTOCFG_36_27_RegisterSize 32
#define CRYPTOCFG_36_27_RegisterResetValue 0x0
#define CRYPTOCFG_36_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_27_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_27_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_28 (DWC_ufshc_block_BaseAddress + 0x2270)
#define CRYPTOCFG_36_28_RegisterSize 32
#define CRYPTOCFG_36_28_RegisterResetValue 0x0
#define CRYPTOCFG_36_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_28_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_28_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_29 (DWC_ufshc_block_BaseAddress + 0x2274)
#define CRYPTOCFG_36_29_RegisterSize 32
#define CRYPTOCFG_36_29_RegisterResetValue 0x0
#define CRYPTOCFG_36_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_29_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_29_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_30 (DWC_ufshc_block_BaseAddress + 0x2278)
#define CRYPTOCFG_36_30_RegisterSize 32
#define CRYPTOCFG_36_30_RegisterResetValue 0x0
#define CRYPTOCFG_36_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_30_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_30_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_36_31 (DWC_ufshc_block_BaseAddress + 0x227c)
#define CRYPTOCFG_36_31_RegisterSize 32
#define CRYPTOCFG_36_31_RegisterResetValue 0x0
#define CRYPTOCFG_36_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_36_31_RESERVED_36_BitAddressOffset 0
#define CRYPTOCFG_36_31_RESERVED_36_RegisterSize 32





#define CRYPTOCFG_37_0 (DWC_ufshc_block_BaseAddress + 0x2280)
#define CRYPTOCFG_37_0_RegisterSize 32
#define CRYPTOCFG_37_0_RegisterResetValue 0x0
#define CRYPTOCFG_37_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_0_CRYPTOKEY_37_0_BitAddressOffset 0
#define CRYPTOCFG_37_0_CRYPTOKEY_37_0_RegisterSize 32





#define CRYPTOCFG_37_1 (DWC_ufshc_block_BaseAddress + 0x2284)
#define CRYPTOCFG_37_1_RegisterSize 32
#define CRYPTOCFG_37_1_RegisterResetValue 0x0
#define CRYPTOCFG_37_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_1_CRYPTOKEY_37_1_BitAddressOffset 0
#define CRYPTOCFG_37_1_CRYPTOKEY_37_1_RegisterSize 32





#define CRYPTOCFG_37_2 (DWC_ufshc_block_BaseAddress + 0x2288)
#define CRYPTOCFG_37_2_RegisterSize 32
#define CRYPTOCFG_37_2_RegisterResetValue 0x0
#define CRYPTOCFG_37_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_2_CRYPTOKEY_37_2_BitAddressOffset 0
#define CRYPTOCFG_37_2_CRYPTOKEY_37_2_RegisterSize 32





#define CRYPTOCFG_37_3 (DWC_ufshc_block_BaseAddress + 0x228c)
#define CRYPTOCFG_37_3_RegisterSize 32
#define CRYPTOCFG_37_3_RegisterResetValue 0x0
#define CRYPTOCFG_37_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_3_CRYPTOKEY_37_3_BitAddressOffset 0
#define CRYPTOCFG_37_3_CRYPTOKEY_37_3_RegisterSize 32





#define CRYPTOCFG_37_4 (DWC_ufshc_block_BaseAddress + 0x2290)
#define CRYPTOCFG_37_4_RegisterSize 32
#define CRYPTOCFG_37_4_RegisterResetValue 0x0
#define CRYPTOCFG_37_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_4_CRYPTOKEY_37_4_BitAddressOffset 0
#define CRYPTOCFG_37_4_CRYPTOKEY_37_4_RegisterSize 32





#define CRYPTOCFG_37_5 (DWC_ufshc_block_BaseAddress + 0x2294)
#define CRYPTOCFG_37_5_RegisterSize 32
#define CRYPTOCFG_37_5_RegisterResetValue 0x0
#define CRYPTOCFG_37_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_5_CRYPTOKEY_37_5_BitAddressOffset 0
#define CRYPTOCFG_37_5_CRYPTOKEY_37_5_RegisterSize 32





#define CRYPTOCFG_37_6 (DWC_ufshc_block_BaseAddress + 0x2298)
#define CRYPTOCFG_37_6_RegisterSize 32
#define CRYPTOCFG_37_6_RegisterResetValue 0x0
#define CRYPTOCFG_37_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_6_CRYPTOKEY_37_6_BitAddressOffset 0
#define CRYPTOCFG_37_6_CRYPTOKEY_37_6_RegisterSize 32





#define CRYPTOCFG_37_7 (DWC_ufshc_block_BaseAddress + 0x229c)
#define CRYPTOCFG_37_7_RegisterSize 32
#define CRYPTOCFG_37_7_RegisterResetValue 0x0
#define CRYPTOCFG_37_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_7_CRYPTOKEY_37_7_BitAddressOffset 0
#define CRYPTOCFG_37_7_CRYPTOKEY_37_7_RegisterSize 32





#define CRYPTOCFG_37_8 (DWC_ufshc_block_BaseAddress + 0x22a0)
#define CRYPTOCFG_37_8_RegisterSize 32
#define CRYPTOCFG_37_8_RegisterResetValue 0x0
#define CRYPTOCFG_37_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_8_CRYPTOKEY_37_8_BitAddressOffset 0
#define CRYPTOCFG_37_8_CRYPTOKEY_37_8_RegisterSize 32





#define CRYPTOCFG_37_9 (DWC_ufshc_block_BaseAddress + 0x22a4)
#define CRYPTOCFG_37_9_RegisterSize 32
#define CRYPTOCFG_37_9_RegisterResetValue 0x0
#define CRYPTOCFG_37_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_9_CRYPTOKEY_37_9_BitAddressOffset 0
#define CRYPTOCFG_37_9_CRYPTOKEY_37_9_RegisterSize 32





#define CRYPTOCFG_37_10 (DWC_ufshc_block_BaseAddress + 0x22a8)
#define CRYPTOCFG_37_10_RegisterSize 32
#define CRYPTOCFG_37_10_RegisterResetValue 0x0
#define CRYPTOCFG_37_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_10_CRYPTOKEY_37_10_BitAddressOffset 0
#define CRYPTOCFG_37_10_CRYPTOKEY_37_10_RegisterSize 32





#define CRYPTOCFG_37_11 (DWC_ufshc_block_BaseAddress + 0x22ac)
#define CRYPTOCFG_37_11_RegisterSize 32
#define CRYPTOCFG_37_11_RegisterResetValue 0x0
#define CRYPTOCFG_37_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_11_CRYPTOKEY_37_11_BitAddressOffset 0
#define CRYPTOCFG_37_11_CRYPTOKEY_37_11_RegisterSize 32





#define CRYPTOCFG_37_12 (DWC_ufshc_block_BaseAddress + 0x22b0)
#define CRYPTOCFG_37_12_RegisterSize 32
#define CRYPTOCFG_37_12_RegisterResetValue 0x0
#define CRYPTOCFG_37_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_12_CRYPTOKEY_37_12_BitAddressOffset 0
#define CRYPTOCFG_37_12_CRYPTOKEY_37_12_RegisterSize 32





#define CRYPTOCFG_37_13 (DWC_ufshc_block_BaseAddress + 0x22b4)
#define CRYPTOCFG_37_13_RegisterSize 32
#define CRYPTOCFG_37_13_RegisterResetValue 0x0
#define CRYPTOCFG_37_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_13_CRYPTOKEY_37_13_BitAddressOffset 0
#define CRYPTOCFG_37_13_CRYPTOKEY_37_13_RegisterSize 32





#define CRYPTOCFG_37_14 (DWC_ufshc_block_BaseAddress + 0x22b8)
#define CRYPTOCFG_37_14_RegisterSize 32
#define CRYPTOCFG_37_14_RegisterResetValue 0x0
#define CRYPTOCFG_37_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_14_CRYPTOKEY_37_14_BitAddressOffset 0
#define CRYPTOCFG_37_14_CRYPTOKEY_37_14_RegisterSize 32





#define CRYPTOCFG_37_15 (DWC_ufshc_block_BaseAddress + 0x22bc)
#define CRYPTOCFG_37_15_RegisterSize 32
#define CRYPTOCFG_37_15_RegisterResetValue 0x0
#define CRYPTOCFG_37_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_15_CRYPTOKEY_37_15_BitAddressOffset 0
#define CRYPTOCFG_37_15_CRYPTOKEY_37_15_RegisterSize 32





#define CRYPTOCFG_37_16 (DWC_ufshc_block_BaseAddress + 0x22c0)
#define CRYPTOCFG_37_16_RegisterSize 32
#define CRYPTOCFG_37_16_RegisterResetValue 0x0
#define CRYPTOCFG_37_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_16_DUSIZE_37_BitAddressOffset 0
#define CRYPTOCFG_37_16_DUSIZE_37_RegisterSize 8



#define CRYPTOCFG_37_16_CAPIDX_37_BitAddressOffset 8
#define CRYPTOCFG_37_16_CAPIDX_37_RegisterSize 8



#define CRYPTOCFG_37_16_CFGE_37_BitAddressOffset 31
#define CRYPTOCFG_37_16_CFGE_37_RegisterSize 1





#define CRYPTOCFG_37_17 (DWC_ufshc_block_BaseAddress + 0x22c4)
#define CRYPTOCFG_37_17_RegisterSize 32
#define CRYPTOCFG_37_17_RegisterResetValue 0x0
#define CRYPTOCFG_37_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_17_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_17_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_18 (DWC_ufshc_block_BaseAddress + 0x22c8)
#define CRYPTOCFG_37_18_RegisterSize 32
#define CRYPTOCFG_37_18_RegisterResetValue 0x0
#define CRYPTOCFG_37_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_18_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_18_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_19 (DWC_ufshc_block_BaseAddress + 0x22cc)
#define CRYPTOCFG_37_19_RegisterSize 32
#define CRYPTOCFG_37_19_RegisterResetValue 0x0
#define CRYPTOCFG_37_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_19_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_19_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_20 (DWC_ufshc_block_BaseAddress + 0x22d0)
#define CRYPTOCFG_37_20_RegisterSize 32
#define CRYPTOCFG_37_20_RegisterResetValue 0x0
#define CRYPTOCFG_37_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_20_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_20_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_21 (DWC_ufshc_block_BaseAddress + 0x22d4)
#define CRYPTOCFG_37_21_RegisterSize 32
#define CRYPTOCFG_37_21_RegisterResetValue 0x0
#define CRYPTOCFG_37_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_21_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_21_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_22 (DWC_ufshc_block_BaseAddress + 0x22d8)
#define CRYPTOCFG_37_22_RegisterSize 32
#define CRYPTOCFG_37_22_RegisterResetValue 0x0
#define CRYPTOCFG_37_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_22_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_22_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_23 (DWC_ufshc_block_BaseAddress + 0x22dc)
#define CRYPTOCFG_37_23_RegisterSize 32
#define CRYPTOCFG_37_23_RegisterResetValue 0x0
#define CRYPTOCFG_37_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_23_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_23_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_24 (DWC_ufshc_block_BaseAddress + 0x22e0)
#define CRYPTOCFG_37_24_RegisterSize 32
#define CRYPTOCFG_37_24_RegisterResetValue 0x0
#define CRYPTOCFG_37_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_24_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_24_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_25 (DWC_ufshc_block_BaseAddress + 0x22e4)
#define CRYPTOCFG_37_25_RegisterSize 32
#define CRYPTOCFG_37_25_RegisterResetValue 0x0
#define CRYPTOCFG_37_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_25_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_25_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_26 (DWC_ufshc_block_BaseAddress + 0x22e8)
#define CRYPTOCFG_37_26_RegisterSize 32
#define CRYPTOCFG_37_26_RegisterResetValue 0x0
#define CRYPTOCFG_37_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_26_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_26_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_27 (DWC_ufshc_block_BaseAddress + 0x22ec)
#define CRYPTOCFG_37_27_RegisterSize 32
#define CRYPTOCFG_37_27_RegisterResetValue 0x0
#define CRYPTOCFG_37_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_27_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_27_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_28 (DWC_ufshc_block_BaseAddress + 0x22f0)
#define CRYPTOCFG_37_28_RegisterSize 32
#define CRYPTOCFG_37_28_RegisterResetValue 0x0
#define CRYPTOCFG_37_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_28_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_28_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_29 (DWC_ufshc_block_BaseAddress + 0x22f4)
#define CRYPTOCFG_37_29_RegisterSize 32
#define CRYPTOCFG_37_29_RegisterResetValue 0x0
#define CRYPTOCFG_37_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_29_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_29_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_30 (DWC_ufshc_block_BaseAddress + 0x22f8)
#define CRYPTOCFG_37_30_RegisterSize 32
#define CRYPTOCFG_37_30_RegisterResetValue 0x0
#define CRYPTOCFG_37_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_30_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_30_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_37_31 (DWC_ufshc_block_BaseAddress + 0x22fc)
#define CRYPTOCFG_37_31_RegisterSize 32
#define CRYPTOCFG_37_31_RegisterResetValue 0x0
#define CRYPTOCFG_37_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_37_31_RESERVED_37_BitAddressOffset 0
#define CRYPTOCFG_37_31_RESERVED_37_RegisterSize 32





#define CRYPTOCFG_38_0 (DWC_ufshc_block_BaseAddress + 0x2300)
#define CRYPTOCFG_38_0_RegisterSize 32
#define CRYPTOCFG_38_0_RegisterResetValue 0x0
#define CRYPTOCFG_38_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_0_CRYPTOKEY_38_0_BitAddressOffset 0
#define CRYPTOCFG_38_0_CRYPTOKEY_38_0_RegisterSize 32





#define CRYPTOCFG_38_1 (DWC_ufshc_block_BaseAddress + 0x2304)
#define CRYPTOCFG_38_1_RegisterSize 32
#define CRYPTOCFG_38_1_RegisterResetValue 0x0
#define CRYPTOCFG_38_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_1_CRYPTOKEY_38_1_BitAddressOffset 0
#define CRYPTOCFG_38_1_CRYPTOKEY_38_1_RegisterSize 32





#define CRYPTOCFG_38_2 (DWC_ufshc_block_BaseAddress + 0x2308)
#define CRYPTOCFG_38_2_RegisterSize 32
#define CRYPTOCFG_38_2_RegisterResetValue 0x0
#define CRYPTOCFG_38_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_2_CRYPTOKEY_38_2_BitAddressOffset 0
#define CRYPTOCFG_38_2_CRYPTOKEY_38_2_RegisterSize 32





#define CRYPTOCFG_38_3 (DWC_ufshc_block_BaseAddress + 0x230c)
#define CRYPTOCFG_38_3_RegisterSize 32
#define CRYPTOCFG_38_3_RegisterResetValue 0x0
#define CRYPTOCFG_38_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_3_CRYPTOKEY_38_3_BitAddressOffset 0
#define CRYPTOCFG_38_3_CRYPTOKEY_38_3_RegisterSize 32





#define CRYPTOCFG_38_4 (DWC_ufshc_block_BaseAddress + 0x2310)
#define CRYPTOCFG_38_4_RegisterSize 32
#define CRYPTOCFG_38_4_RegisterResetValue 0x0
#define CRYPTOCFG_38_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_4_CRYPTOKEY_38_4_BitAddressOffset 0
#define CRYPTOCFG_38_4_CRYPTOKEY_38_4_RegisterSize 32





#define CRYPTOCFG_38_5 (DWC_ufshc_block_BaseAddress + 0x2314)
#define CRYPTOCFG_38_5_RegisterSize 32
#define CRYPTOCFG_38_5_RegisterResetValue 0x0
#define CRYPTOCFG_38_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_5_CRYPTOKEY_38_5_BitAddressOffset 0
#define CRYPTOCFG_38_5_CRYPTOKEY_38_5_RegisterSize 32





#define CRYPTOCFG_38_6 (DWC_ufshc_block_BaseAddress + 0x2318)
#define CRYPTOCFG_38_6_RegisterSize 32
#define CRYPTOCFG_38_6_RegisterResetValue 0x0
#define CRYPTOCFG_38_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_6_CRYPTOKEY_38_6_BitAddressOffset 0
#define CRYPTOCFG_38_6_CRYPTOKEY_38_6_RegisterSize 32





#define CRYPTOCFG_38_7 (DWC_ufshc_block_BaseAddress + 0x231c)
#define CRYPTOCFG_38_7_RegisterSize 32
#define CRYPTOCFG_38_7_RegisterResetValue 0x0
#define CRYPTOCFG_38_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_7_CRYPTOKEY_38_7_BitAddressOffset 0
#define CRYPTOCFG_38_7_CRYPTOKEY_38_7_RegisterSize 32





#define CRYPTOCFG_38_8 (DWC_ufshc_block_BaseAddress + 0x2320)
#define CRYPTOCFG_38_8_RegisterSize 32
#define CRYPTOCFG_38_8_RegisterResetValue 0x0
#define CRYPTOCFG_38_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_8_CRYPTOKEY_38_8_BitAddressOffset 0
#define CRYPTOCFG_38_8_CRYPTOKEY_38_8_RegisterSize 32





#define CRYPTOCFG_38_9 (DWC_ufshc_block_BaseAddress + 0x2324)
#define CRYPTOCFG_38_9_RegisterSize 32
#define CRYPTOCFG_38_9_RegisterResetValue 0x0
#define CRYPTOCFG_38_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_9_CRYPTOKEY_38_9_BitAddressOffset 0
#define CRYPTOCFG_38_9_CRYPTOKEY_38_9_RegisterSize 32





#define CRYPTOCFG_38_10 (DWC_ufshc_block_BaseAddress + 0x2328)
#define CRYPTOCFG_38_10_RegisterSize 32
#define CRYPTOCFG_38_10_RegisterResetValue 0x0
#define CRYPTOCFG_38_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_10_CRYPTOKEY_38_10_BitAddressOffset 0
#define CRYPTOCFG_38_10_CRYPTOKEY_38_10_RegisterSize 32





#define CRYPTOCFG_38_11 (DWC_ufshc_block_BaseAddress + 0x232c)
#define CRYPTOCFG_38_11_RegisterSize 32
#define CRYPTOCFG_38_11_RegisterResetValue 0x0
#define CRYPTOCFG_38_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_11_CRYPTOKEY_38_11_BitAddressOffset 0
#define CRYPTOCFG_38_11_CRYPTOKEY_38_11_RegisterSize 32





#define CRYPTOCFG_38_12 (DWC_ufshc_block_BaseAddress + 0x2330)
#define CRYPTOCFG_38_12_RegisterSize 32
#define CRYPTOCFG_38_12_RegisterResetValue 0x0
#define CRYPTOCFG_38_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_12_CRYPTOKEY_38_12_BitAddressOffset 0
#define CRYPTOCFG_38_12_CRYPTOKEY_38_12_RegisterSize 32





#define CRYPTOCFG_38_13 (DWC_ufshc_block_BaseAddress + 0x2334)
#define CRYPTOCFG_38_13_RegisterSize 32
#define CRYPTOCFG_38_13_RegisterResetValue 0x0
#define CRYPTOCFG_38_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_13_CRYPTOKEY_38_13_BitAddressOffset 0
#define CRYPTOCFG_38_13_CRYPTOKEY_38_13_RegisterSize 32





#define CRYPTOCFG_38_14 (DWC_ufshc_block_BaseAddress + 0x2338)
#define CRYPTOCFG_38_14_RegisterSize 32
#define CRYPTOCFG_38_14_RegisterResetValue 0x0
#define CRYPTOCFG_38_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_14_CRYPTOKEY_38_14_BitAddressOffset 0
#define CRYPTOCFG_38_14_CRYPTOKEY_38_14_RegisterSize 32





#define CRYPTOCFG_38_15 (DWC_ufshc_block_BaseAddress + 0x233c)
#define CRYPTOCFG_38_15_RegisterSize 32
#define CRYPTOCFG_38_15_RegisterResetValue 0x0
#define CRYPTOCFG_38_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_15_CRYPTOKEY_38_15_BitAddressOffset 0
#define CRYPTOCFG_38_15_CRYPTOKEY_38_15_RegisterSize 32





#define CRYPTOCFG_38_16 (DWC_ufshc_block_BaseAddress + 0x2340)
#define CRYPTOCFG_38_16_RegisterSize 32
#define CRYPTOCFG_38_16_RegisterResetValue 0x0
#define CRYPTOCFG_38_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_16_DUSIZE_38_BitAddressOffset 0
#define CRYPTOCFG_38_16_DUSIZE_38_RegisterSize 8



#define CRYPTOCFG_38_16_CAPIDX_38_BitAddressOffset 8
#define CRYPTOCFG_38_16_CAPIDX_38_RegisterSize 8



#define CRYPTOCFG_38_16_CFGE_38_BitAddressOffset 31
#define CRYPTOCFG_38_16_CFGE_38_RegisterSize 1





#define CRYPTOCFG_38_17 (DWC_ufshc_block_BaseAddress + 0x2344)
#define CRYPTOCFG_38_17_RegisterSize 32
#define CRYPTOCFG_38_17_RegisterResetValue 0x0
#define CRYPTOCFG_38_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_17_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_17_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_18 (DWC_ufshc_block_BaseAddress + 0x2348)
#define CRYPTOCFG_38_18_RegisterSize 32
#define CRYPTOCFG_38_18_RegisterResetValue 0x0
#define CRYPTOCFG_38_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_18_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_18_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_19 (DWC_ufshc_block_BaseAddress + 0x234c)
#define CRYPTOCFG_38_19_RegisterSize 32
#define CRYPTOCFG_38_19_RegisterResetValue 0x0
#define CRYPTOCFG_38_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_19_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_19_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_20 (DWC_ufshc_block_BaseAddress + 0x2350)
#define CRYPTOCFG_38_20_RegisterSize 32
#define CRYPTOCFG_38_20_RegisterResetValue 0x0
#define CRYPTOCFG_38_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_20_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_20_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_21 (DWC_ufshc_block_BaseAddress + 0x2354)
#define CRYPTOCFG_38_21_RegisterSize 32
#define CRYPTOCFG_38_21_RegisterResetValue 0x0
#define CRYPTOCFG_38_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_21_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_21_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_22 (DWC_ufshc_block_BaseAddress + 0x2358)
#define CRYPTOCFG_38_22_RegisterSize 32
#define CRYPTOCFG_38_22_RegisterResetValue 0x0
#define CRYPTOCFG_38_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_22_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_22_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_23 (DWC_ufshc_block_BaseAddress + 0x235c)
#define CRYPTOCFG_38_23_RegisterSize 32
#define CRYPTOCFG_38_23_RegisterResetValue 0x0
#define CRYPTOCFG_38_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_23_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_23_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_24 (DWC_ufshc_block_BaseAddress + 0x2360)
#define CRYPTOCFG_38_24_RegisterSize 32
#define CRYPTOCFG_38_24_RegisterResetValue 0x0
#define CRYPTOCFG_38_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_24_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_24_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_25 (DWC_ufshc_block_BaseAddress + 0x2364)
#define CRYPTOCFG_38_25_RegisterSize 32
#define CRYPTOCFG_38_25_RegisterResetValue 0x0
#define CRYPTOCFG_38_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_25_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_25_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_26 (DWC_ufshc_block_BaseAddress + 0x2368)
#define CRYPTOCFG_38_26_RegisterSize 32
#define CRYPTOCFG_38_26_RegisterResetValue 0x0
#define CRYPTOCFG_38_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_26_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_26_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_27 (DWC_ufshc_block_BaseAddress + 0x236c)
#define CRYPTOCFG_38_27_RegisterSize 32
#define CRYPTOCFG_38_27_RegisterResetValue 0x0
#define CRYPTOCFG_38_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_27_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_27_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_28 (DWC_ufshc_block_BaseAddress + 0x2370)
#define CRYPTOCFG_38_28_RegisterSize 32
#define CRYPTOCFG_38_28_RegisterResetValue 0x0
#define CRYPTOCFG_38_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_28_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_28_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_29 (DWC_ufshc_block_BaseAddress + 0x2374)
#define CRYPTOCFG_38_29_RegisterSize 32
#define CRYPTOCFG_38_29_RegisterResetValue 0x0
#define CRYPTOCFG_38_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_29_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_29_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_30 (DWC_ufshc_block_BaseAddress + 0x2378)
#define CRYPTOCFG_38_30_RegisterSize 32
#define CRYPTOCFG_38_30_RegisterResetValue 0x0
#define CRYPTOCFG_38_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_30_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_30_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_38_31 (DWC_ufshc_block_BaseAddress + 0x237c)
#define CRYPTOCFG_38_31_RegisterSize 32
#define CRYPTOCFG_38_31_RegisterResetValue 0x0
#define CRYPTOCFG_38_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_38_31_RESERVED_38_BitAddressOffset 0
#define CRYPTOCFG_38_31_RESERVED_38_RegisterSize 32





#define CRYPTOCFG_39_0 (DWC_ufshc_block_BaseAddress + 0x2380)
#define CRYPTOCFG_39_0_RegisterSize 32
#define CRYPTOCFG_39_0_RegisterResetValue 0x0
#define CRYPTOCFG_39_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_0_CRYPTOKEY_39_0_BitAddressOffset 0
#define CRYPTOCFG_39_0_CRYPTOKEY_39_0_RegisterSize 32





#define CRYPTOCFG_39_1 (DWC_ufshc_block_BaseAddress + 0x2384)
#define CRYPTOCFG_39_1_RegisterSize 32
#define CRYPTOCFG_39_1_RegisterResetValue 0x0
#define CRYPTOCFG_39_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_1_CRYPTOKEY_39_1_BitAddressOffset 0
#define CRYPTOCFG_39_1_CRYPTOKEY_39_1_RegisterSize 32





#define CRYPTOCFG_39_2 (DWC_ufshc_block_BaseAddress + 0x2388)
#define CRYPTOCFG_39_2_RegisterSize 32
#define CRYPTOCFG_39_2_RegisterResetValue 0x0
#define CRYPTOCFG_39_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_2_CRYPTOKEY_39_2_BitAddressOffset 0
#define CRYPTOCFG_39_2_CRYPTOKEY_39_2_RegisterSize 32





#define CRYPTOCFG_39_3 (DWC_ufshc_block_BaseAddress + 0x238c)
#define CRYPTOCFG_39_3_RegisterSize 32
#define CRYPTOCFG_39_3_RegisterResetValue 0x0
#define CRYPTOCFG_39_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_3_CRYPTOKEY_39_3_BitAddressOffset 0
#define CRYPTOCFG_39_3_CRYPTOKEY_39_3_RegisterSize 32





#define CRYPTOCFG_39_4 (DWC_ufshc_block_BaseAddress + 0x2390)
#define CRYPTOCFG_39_4_RegisterSize 32
#define CRYPTOCFG_39_4_RegisterResetValue 0x0
#define CRYPTOCFG_39_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_4_CRYPTOKEY_39_4_BitAddressOffset 0
#define CRYPTOCFG_39_4_CRYPTOKEY_39_4_RegisterSize 32





#define CRYPTOCFG_39_5 (DWC_ufshc_block_BaseAddress + 0x2394)
#define CRYPTOCFG_39_5_RegisterSize 32
#define CRYPTOCFG_39_5_RegisterResetValue 0x0
#define CRYPTOCFG_39_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_5_CRYPTOKEY_39_5_BitAddressOffset 0
#define CRYPTOCFG_39_5_CRYPTOKEY_39_5_RegisterSize 32





#define CRYPTOCFG_39_6 (DWC_ufshc_block_BaseAddress + 0x2398)
#define CRYPTOCFG_39_6_RegisterSize 32
#define CRYPTOCFG_39_6_RegisterResetValue 0x0
#define CRYPTOCFG_39_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_6_CRYPTOKEY_39_6_BitAddressOffset 0
#define CRYPTOCFG_39_6_CRYPTOKEY_39_6_RegisterSize 32





#define CRYPTOCFG_39_7 (DWC_ufshc_block_BaseAddress + 0x239c)
#define CRYPTOCFG_39_7_RegisterSize 32
#define CRYPTOCFG_39_7_RegisterResetValue 0x0
#define CRYPTOCFG_39_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_7_CRYPTOKEY_39_7_BitAddressOffset 0
#define CRYPTOCFG_39_7_CRYPTOKEY_39_7_RegisterSize 32





#define CRYPTOCFG_39_8 (DWC_ufshc_block_BaseAddress + 0x23a0)
#define CRYPTOCFG_39_8_RegisterSize 32
#define CRYPTOCFG_39_8_RegisterResetValue 0x0
#define CRYPTOCFG_39_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_8_CRYPTOKEY_39_8_BitAddressOffset 0
#define CRYPTOCFG_39_8_CRYPTOKEY_39_8_RegisterSize 32





#define CRYPTOCFG_39_9 (DWC_ufshc_block_BaseAddress + 0x23a4)
#define CRYPTOCFG_39_9_RegisterSize 32
#define CRYPTOCFG_39_9_RegisterResetValue 0x0
#define CRYPTOCFG_39_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_9_CRYPTOKEY_39_9_BitAddressOffset 0
#define CRYPTOCFG_39_9_CRYPTOKEY_39_9_RegisterSize 32





#define CRYPTOCFG_39_10 (DWC_ufshc_block_BaseAddress + 0x23a8)
#define CRYPTOCFG_39_10_RegisterSize 32
#define CRYPTOCFG_39_10_RegisterResetValue 0x0
#define CRYPTOCFG_39_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_10_CRYPTOKEY_39_10_BitAddressOffset 0
#define CRYPTOCFG_39_10_CRYPTOKEY_39_10_RegisterSize 32





#define CRYPTOCFG_39_11 (DWC_ufshc_block_BaseAddress + 0x23ac)
#define CRYPTOCFG_39_11_RegisterSize 32
#define CRYPTOCFG_39_11_RegisterResetValue 0x0
#define CRYPTOCFG_39_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_11_CRYPTOKEY_39_11_BitAddressOffset 0
#define CRYPTOCFG_39_11_CRYPTOKEY_39_11_RegisterSize 32





#define CRYPTOCFG_39_12 (DWC_ufshc_block_BaseAddress + 0x23b0)
#define CRYPTOCFG_39_12_RegisterSize 32
#define CRYPTOCFG_39_12_RegisterResetValue 0x0
#define CRYPTOCFG_39_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_12_CRYPTOKEY_39_12_BitAddressOffset 0
#define CRYPTOCFG_39_12_CRYPTOKEY_39_12_RegisterSize 32





#define CRYPTOCFG_39_13 (DWC_ufshc_block_BaseAddress + 0x23b4)
#define CRYPTOCFG_39_13_RegisterSize 32
#define CRYPTOCFG_39_13_RegisterResetValue 0x0
#define CRYPTOCFG_39_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_13_CRYPTOKEY_39_13_BitAddressOffset 0
#define CRYPTOCFG_39_13_CRYPTOKEY_39_13_RegisterSize 32





#define CRYPTOCFG_39_14 (DWC_ufshc_block_BaseAddress + 0x23b8)
#define CRYPTOCFG_39_14_RegisterSize 32
#define CRYPTOCFG_39_14_RegisterResetValue 0x0
#define CRYPTOCFG_39_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_14_CRYPTOKEY_39_14_BitAddressOffset 0
#define CRYPTOCFG_39_14_CRYPTOKEY_39_14_RegisterSize 32





#define CRYPTOCFG_39_15 (DWC_ufshc_block_BaseAddress + 0x23bc)
#define CRYPTOCFG_39_15_RegisterSize 32
#define CRYPTOCFG_39_15_RegisterResetValue 0x0
#define CRYPTOCFG_39_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_15_CRYPTOKEY_39_15_BitAddressOffset 0
#define CRYPTOCFG_39_15_CRYPTOKEY_39_15_RegisterSize 32





#define CRYPTOCFG_39_16 (DWC_ufshc_block_BaseAddress + 0x23c0)
#define CRYPTOCFG_39_16_RegisterSize 32
#define CRYPTOCFG_39_16_RegisterResetValue 0x0
#define CRYPTOCFG_39_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_16_DUSIZE_39_BitAddressOffset 0
#define CRYPTOCFG_39_16_DUSIZE_39_RegisterSize 8



#define CRYPTOCFG_39_16_CAPIDX_39_BitAddressOffset 8
#define CRYPTOCFG_39_16_CAPIDX_39_RegisterSize 8



#define CRYPTOCFG_39_16_CFGE_39_BitAddressOffset 31
#define CRYPTOCFG_39_16_CFGE_39_RegisterSize 1





#define CRYPTOCFG_39_17 (DWC_ufshc_block_BaseAddress + 0x23c4)
#define CRYPTOCFG_39_17_RegisterSize 32
#define CRYPTOCFG_39_17_RegisterResetValue 0x0
#define CRYPTOCFG_39_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_17_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_17_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_18 (DWC_ufshc_block_BaseAddress + 0x23c8)
#define CRYPTOCFG_39_18_RegisterSize 32
#define CRYPTOCFG_39_18_RegisterResetValue 0x0
#define CRYPTOCFG_39_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_18_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_18_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_19 (DWC_ufshc_block_BaseAddress + 0x23cc)
#define CRYPTOCFG_39_19_RegisterSize 32
#define CRYPTOCFG_39_19_RegisterResetValue 0x0
#define CRYPTOCFG_39_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_19_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_19_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_20 (DWC_ufshc_block_BaseAddress + 0x23d0)
#define CRYPTOCFG_39_20_RegisterSize 32
#define CRYPTOCFG_39_20_RegisterResetValue 0x0
#define CRYPTOCFG_39_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_20_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_20_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_21 (DWC_ufshc_block_BaseAddress + 0x23d4)
#define CRYPTOCFG_39_21_RegisterSize 32
#define CRYPTOCFG_39_21_RegisterResetValue 0x0
#define CRYPTOCFG_39_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_21_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_21_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_22 (DWC_ufshc_block_BaseAddress + 0x23d8)
#define CRYPTOCFG_39_22_RegisterSize 32
#define CRYPTOCFG_39_22_RegisterResetValue 0x0
#define CRYPTOCFG_39_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_22_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_22_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_23 (DWC_ufshc_block_BaseAddress + 0x23dc)
#define CRYPTOCFG_39_23_RegisterSize 32
#define CRYPTOCFG_39_23_RegisterResetValue 0x0
#define CRYPTOCFG_39_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_23_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_23_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_24 (DWC_ufshc_block_BaseAddress + 0x23e0)
#define CRYPTOCFG_39_24_RegisterSize 32
#define CRYPTOCFG_39_24_RegisterResetValue 0x0
#define CRYPTOCFG_39_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_24_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_24_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_25 (DWC_ufshc_block_BaseAddress + 0x23e4)
#define CRYPTOCFG_39_25_RegisterSize 32
#define CRYPTOCFG_39_25_RegisterResetValue 0x0
#define CRYPTOCFG_39_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_25_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_25_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_26 (DWC_ufshc_block_BaseAddress + 0x23e8)
#define CRYPTOCFG_39_26_RegisterSize 32
#define CRYPTOCFG_39_26_RegisterResetValue 0x0
#define CRYPTOCFG_39_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_26_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_26_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_27 (DWC_ufshc_block_BaseAddress + 0x23ec)
#define CRYPTOCFG_39_27_RegisterSize 32
#define CRYPTOCFG_39_27_RegisterResetValue 0x0
#define CRYPTOCFG_39_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_27_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_27_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_28 (DWC_ufshc_block_BaseAddress + 0x23f0)
#define CRYPTOCFG_39_28_RegisterSize 32
#define CRYPTOCFG_39_28_RegisterResetValue 0x0
#define CRYPTOCFG_39_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_28_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_28_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_29 (DWC_ufshc_block_BaseAddress + 0x23f4)
#define CRYPTOCFG_39_29_RegisterSize 32
#define CRYPTOCFG_39_29_RegisterResetValue 0x0
#define CRYPTOCFG_39_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_29_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_29_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_30 (DWC_ufshc_block_BaseAddress + 0x23f8)
#define CRYPTOCFG_39_30_RegisterSize 32
#define CRYPTOCFG_39_30_RegisterResetValue 0x0
#define CRYPTOCFG_39_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_30_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_30_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_39_31 (DWC_ufshc_block_BaseAddress + 0x23fc)
#define CRYPTOCFG_39_31_RegisterSize 32
#define CRYPTOCFG_39_31_RegisterResetValue 0x0
#define CRYPTOCFG_39_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_39_31_RESERVED_39_BitAddressOffset 0
#define CRYPTOCFG_39_31_RESERVED_39_RegisterSize 32





#define CRYPTOCFG_40_0 (DWC_ufshc_block_BaseAddress + 0x2400)
#define CRYPTOCFG_40_0_RegisterSize 32
#define CRYPTOCFG_40_0_RegisterResetValue 0x0
#define CRYPTOCFG_40_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_0_CRYPTOKEY_40_0_BitAddressOffset 0
#define CRYPTOCFG_40_0_CRYPTOKEY_40_0_RegisterSize 32





#define CRYPTOCFG_40_1 (DWC_ufshc_block_BaseAddress + 0x2404)
#define CRYPTOCFG_40_1_RegisterSize 32
#define CRYPTOCFG_40_1_RegisterResetValue 0x0
#define CRYPTOCFG_40_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_1_CRYPTOKEY_40_1_BitAddressOffset 0
#define CRYPTOCFG_40_1_CRYPTOKEY_40_1_RegisterSize 32





#define CRYPTOCFG_40_2 (DWC_ufshc_block_BaseAddress + 0x2408)
#define CRYPTOCFG_40_2_RegisterSize 32
#define CRYPTOCFG_40_2_RegisterResetValue 0x0
#define CRYPTOCFG_40_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_2_CRYPTOKEY_40_2_BitAddressOffset 0
#define CRYPTOCFG_40_2_CRYPTOKEY_40_2_RegisterSize 32





#define CRYPTOCFG_40_3 (DWC_ufshc_block_BaseAddress + 0x240c)
#define CRYPTOCFG_40_3_RegisterSize 32
#define CRYPTOCFG_40_3_RegisterResetValue 0x0
#define CRYPTOCFG_40_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_3_CRYPTOKEY_40_3_BitAddressOffset 0
#define CRYPTOCFG_40_3_CRYPTOKEY_40_3_RegisterSize 32





#define CRYPTOCFG_40_4 (DWC_ufshc_block_BaseAddress + 0x2410)
#define CRYPTOCFG_40_4_RegisterSize 32
#define CRYPTOCFG_40_4_RegisterResetValue 0x0
#define CRYPTOCFG_40_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_4_CRYPTOKEY_40_4_BitAddressOffset 0
#define CRYPTOCFG_40_4_CRYPTOKEY_40_4_RegisterSize 32





#define CRYPTOCFG_40_5 (DWC_ufshc_block_BaseAddress + 0x2414)
#define CRYPTOCFG_40_5_RegisterSize 32
#define CRYPTOCFG_40_5_RegisterResetValue 0x0
#define CRYPTOCFG_40_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_5_CRYPTOKEY_40_5_BitAddressOffset 0
#define CRYPTOCFG_40_5_CRYPTOKEY_40_5_RegisterSize 32





#define CRYPTOCFG_40_6 (DWC_ufshc_block_BaseAddress + 0x2418)
#define CRYPTOCFG_40_6_RegisterSize 32
#define CRYPTOCFG_40_6_RegisterResetValue 0x0
#define CRYPTOCFG_40_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_6_CRYPTOKEY_40_6_BitAddressOffset 0
#define CRYPTOCFG_40_6_CRYPTOKEY_40_6_RegisterSize 32





#define CRYPTOCFG_40_7 (DWC_ufshc_block_BaseAddress + 0x241c)
#define CRYPTOCFG_40_7_RegisterSize 32
#define CRYPTOCFG_40_7_RegisterResetValue 0x0
#define CRYPTOCFG_40_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_7_CRYPTOKEY_40_7_BitAddressOffset 0
#define CRYPTOCFG_40_7_CRYPTOKEY_40_7_RegisterSize 32





#define CRYPTOCFG_40_8 (DWC_ufshc_block_BaseAddress + 0x2420)
#define CRYPTOCFG_40_8_RegisterSize 32
#define CRYPTOCFG_40_8_RegisterResetValue 0x0
#define CRYPTOCFG_40_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_8_CRYPTOKEY_40_8_BitAddressOffset 0
#define CRYPTOCFG_40_8_CRYPTOKEY_40_8_RegisterSize 32





#define CRYPTOCFG_40_9 (DWC_ufshc_block_BaseAddress + 0x2424)
#define CRYPTOCFG_40_9_RegisterSize 32
#define CRYPTOCFG_40_9_RegisterResetValue 0x0
#define CRYPTOCFG_40_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_9_CRYPTOKEY_40_9_BitAddressOffset 0
#define CRYPTOCFG_40_9_CRYPTOKEY_40_9_RegisterSize 32





#define CRYPTOCFG_40_10 (DWC_ufshc_block_BaseAddress + 0x2428)
#define CRYPTOCFG_40_10_RegisterSize 32
#define CRYPTOCFG_40_10_RegisterResetValue 0x0
#define CRYPTOCFG_40_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_10_CRYPTOKEY_40_10_BitAddressOffset 0
#define CRYPTOCFG_40_10_CRYPTOKEY_40_10_RegisterSize 32





#define CRYPTOCFG_40_11 (DWC_ufshc_block_BaseAddress + 0x242c)
#define CRYPTOCFG_40_11_RegisterSize 32
#define CRYPTOCFG_40_11_RegisterResetValue 0x0
#define CRYPTOCFG_40_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_11_CRYPTOKEY_40_11_BitAddressOffset 0
#define CRYPTOCFG_40_11_CRYPTOKEY_40_11_RegisterSize 32





#define CRYPTOCFG_40_12 (DWC_ufshc_block_BaseAddress + 0x2430)
#define CRYPTOCFG_40_12_RegisterSize 32
#define CRYPTOCFG_40_12_RegisterResetValue 0x0
#define CRYPTOCFG_40_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_12_CRYPTOKEY_40_12_BitAddressOffset 0
#define CRYPTOCFG_40_12_CRYPTOKEY_40_12_RegisterSize 32





#define CRYPTOCFG_40_13 (DWC_ufshc_block_BaseAddress + 0x2434)
#define CRYPTOCFG_40_13_RegisterSize 32
#define CRYPTOCFG_40_13_RegisterResetValue 0x0
#define CRYPTOCFG_40_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_13_CRYPTOKEY_40_13_BitAddressOffset 0
#define CRYPTOCFG_40_13_CRYPTOKEY_40_13_RegisterSize 32





#define CRYPTOCFG_40_14 (DWC_ufshc_block_BaseAddress + 0x2438)
#define CRYPTOCFG_40_14_RegisterSize 32
#define CRYPTOCFG_40_14_RegisterResetValue 0x0
#define CRYPTOCFG_40_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_14_CRYPTOKEY_40_14_BitAddressOffset 0
#define CRYPTOCFG_40_14_CRYPTOKEY_40_14_RegisterSize 32





#define CRYPTOCFG_40_15 (DWC_ufshc_block_BaseAddress + 0x243c)
#define CRYPTOCFG_40_15_RegisterSize 32
#define CRYPTOCFG_40_15_RegisterResetValue 0x0
#define CRYPTOCFG_40_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_15_CRYPTOKEY_40_15_BitAddressOffset 0
#define CRYPTOCFG_40_15_CRYPTOKEY_40_15_RegisterSize 32





#define CRYPTOCFG_40_16 (DWC_ufshc_block_BaseAddress + 0x2440)
#define CRYPTOCFG_40_16_RegisterSize 32
#define CRYPTOCFG_40_16_RegisterResetValue 0x0
#define CRYPTOCFG_40_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_16_DUSIZE_40_BitAddressOffset 0
#define CRYPTOCFG_40_16_DUSIZE_40_RegisterSize 8



#define CRYPTOCFG_40_16_CAPIDX_40_BitAddressOffset 8
#define CRYPTOCFG_40_16_CAPIDX_40_RegisterSize 8



#define CRYPTOCFG_40_16_CFGE_40_BitAddressOffset 31
#define CRYPTOCFG_40_16_CFGE_40_RegisterSize 1





#define CRYPTOCFG_40_17 (DWC_ufshc_block_BaseAddress + 0x2444)
#define CRYPTOCFG_40_17_RegisterSize 32
#define CRYPTOCFG_40_17_RegisterResetValue 0x0
#define CRYPTOCFG_40_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_17_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_17_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_18 (DWC_ufshc_block_BaseAddress + 0x2448)
#define CRYPTOCFG_40_18_RegisterSize 32
#define CRYPTOCFG_40_18_RegisterResetValue 0x0
#define CRYPTOCFG_40_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_18_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_18_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_19 (DWC_ufshc_block_BaseAddress + 0x244c)
#define CRYPTOCFG_40_19_RegisterSize 32
#define CRYPTOCFG_40_19_RegisterResetValue 0x0
#define CRYPTOCFG_40_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_19_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_19_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_20 (DWC_ufshc_block_BaseAddress + 0x2450)
#define CRYPTOCFG_40_20_RegisterSize 32
#define CRYPTOCFG_40_20_RegisterResetValue 0x0
#define CRYPTOCFG_40_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_20_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_20_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_21 (DWC_ufshc_block_BaseAddress + 0x2454)
#define CRYPTOCFG_40_21_RegisterSize 32
#define CRYPTOCFG_40_21_RegisterResetValue 0x0
#define CRYPTOCFG_40_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_21_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_21_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_22 (DWC_ufshc_block_BaseAddress + 0x2458)
#define CRYPTOCFG_40_22_RegisterSize 32
#define CRYPTOCFG_40_22_RegisterResetValue 0x0
#define CRYPTOCFG_40_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_22_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_22_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_23 (DWC_ufshc_block_BaseAddress + 0x245c)
#define CRYPTOCFG_40_23_RegisterSize 32
#define CRYPTOCFG_40_23_RegisterResetValue 0x0
#define CRYPTOCFG_40_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_23_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_23_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_24 (DWC_ufshc_block_BaseAddress + 0x2460)
#define CRYPTOCFG_40_24_RegisterSize 32
#define CRYPTOCFG_40_24_RegisterResetValue 0x0
#define CRYPTOCFG_40_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_24_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_24_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_25 (DWC_ufshc_block_BaseAddress + 0x2464)
#define CRYPTOCFG_40_25_RegisterSize 32
#define CRYPTOCFG_40_25_RegisterResetValue 0x0
#define CRYPTOCFG_40_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_25_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_25_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_26 (DWC_ufshc_block_BaseAddress + 0x2468)
#define CRYPTOCFG_40_26_RegisterSize 32
#define CRYPTOCFG_40_26_RegisterResetValue 0x0
#define CRYPTOCFG_40_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_26_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_26_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_27 (DWC_ufshc_block_BaseAddress + 0x246c)
#define CRYPTOCFG_40_27_RegisterSize 32
#define CRYPTOCFG_40_27_RegisterResetValue 0x0
#define CRYPTOCFG_40_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_27_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_27_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_28 (DWC_ufshc_block_BaseAddress + 0x2470)
#define CRYPTOCFG_40_28_RegisterSize 32
#define CRYPTOCFG_40_28_RegisterResetValue 0x0
#define CRYPTOCFG_40_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_28_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_28_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_29 (DWC_ufshc_block_BaseAddress + 0x2474)
#define CRYPTOCFG_40_29_RegisterSize 32
#define CRYPTOCFG_40_29_RegisterResetValue 0x0
#define CRYPTOCFG_40_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_29_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_29_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_30 (DWC_ufshc_block_BaseAddress + 0x2478)
#define CRYPTOCFG_40_30_RegisterSize 32
#define CRYPTOCFG_40_30_RegisterResetValue 0x0
#define CRYPTOCFG_40_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_30_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_30_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_40_31 (DWC_ufshc_block_BaseAddress + 0x247c)
#define CRYPTOCFG_40_31_RegisterSize 32
#define CRYPTOCFG_40_31_RegisterResetValue 0x0
#define CRYPTOCFG_40_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_40_31_RESERVED_40_BitAddressOffset 0
#define CRYPTOCFG_40_31_RESERVED_40_RegisterSize 32





#define CRYPTOCFG_41_0 (DWC_ufshc_block_BaseAddress + 0x2480)
#define CRYPTOCFG_41_0_RegisterSize 32
#define CRYPTOCFG_41_0_RegisterResetValue 0x0
#define CRYPTOCFG_41_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_0_CRYPTOKEY_41_0_BitAddressOffset 0
#define CRYPTOCFG_41_0_CRYPTOKEY_41_0_RegisterSize 32





#define CRYPTOCFG_41_1 (DWC_ufshc_block_BaseAddress + 0x2484)
#define CRYPTOCFG_41_1_RegisterSize 32
#define CRYPTOCFG_41_1_RegisterResetValue 0x0
#define CRYPTOCFG_41_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_1_CRYPTOKEY_41_1_BitAddressOffset 0
#define CRYPTOCFG_41_1_CRYPTOKEY_41_1_RegisterSize 32





#define CRYPTOCFG_41_2 (DWC_ufshc_block_BaseAddress + 0x2488)
#define CRYPTOCFG_41_2_RegisterSize 32
#define CRYPTOCFG_41_2_RegisterResetValue 0x0
#define CRYPTOCFG_41_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_2_CRYPTOKEY_41_2_BitAddressOffset 0
#define CRYPTOCFG_41_2_CRYPTOKEY_41_2_RegisterSize 32





#define CRYPTOCFG_41_3 (DWC_ufshc_block_BaseAddress + 0x248c)
#define CRYPTOCFG_41_3_RegisterSize 32
#define CRYPTOCFG_41_3_RegisterResetValue 0x0
#define CRYPTOCFG_41_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_3_CRYPTOKEY_41_3_BitAddressOffset 0
#define CRYPTOCFG_41_3_CRYPTOKEY_41_3_RegisterSize 32





#define CRYPTOCFG_41_4 (DWC_ufshc_block_BaseAddress + 0x2490)
#define CRYPTOCFG_41_4_RegisterSize 32
#define CRYPTOCFG_41_4_RegisterResetValue 0x0
#define CRYPTOCFG_41_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_4_CRYPTOKEY_41_4_BitAddressOffset 0
#define CRYPTOCFG_41_4_CRYPTOKEY_41_4_RegisterSize 32





#define CRYPTOCFG_41_5 (DWC_ufshc_block_BaseAddress + 0x2494)
#define CRYPTOCFG_41_5_RegisterSize 32
#define CRYPTOCFG_41_5_RegisterResetValue 0x0
#define CRYPTOCFG_41_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_5_CRYPTOKEY_41_5_BitAddressOffset 0
#define CRYPTOCFG_41_5_CRYPTOKEY_41_5_RegisterSize 32





#define CRYPTOCFG_41_6 (DWC_ufshc_block_BaseAddress + 0x2498)
#define CRYPTOCFG_41_6_RegisterSize 32
#define CRYPTOCFG_41_6_RegisterResetValue 0x0
#define CRYPTOCFG_41_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_6_CRYPTOKEY_41_6_BitAddressOffset 0
#define CRYPTOCFG_41_6_CRYPTOKEY_41_6_RegisterSize 32





#define CRYPTOCFG_41_7 (DWC_ufshc_block_BaseAddress + 0x249c)
#define CRYPTOCFG_41_7_RegisterSize 32
#define CRYPTOCFG_41_7_RegisterResetValue 0x0
#define CRYPTOCFG_41_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_7_CRYPTOKEY_41_7_BitAddressOffset 0
#define CRYPTOCFG_41_7_CRYPTOKEY_41_7_RegisterSize 32





#define CRYPTOCFG_41_8 (DWC_ufshc_block_BaseAddress + 0x24a0)
#define CRYPTOCFG_41_8_RegisterSize 32
#define CRYPTOCFG_41_8_RegisterResetValue 0x0
#define CRYPTOCFG_41_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_8_CRYPTOKEY_41_8_BitAddressOffset 0
#define CRYPTOCFG_41_8_CRYPTOKEY_41_8_RegisterSize 32





#define CRYPTOCFG_41_9 (DWC_ufshc_block_BaseAddress + 0x24a4)
#define CRYPTOCFG_41_9_RegisterSize 32
#define CRYPTOCFG_41_9_RegisterResetValue 0x0
#define CRYPTOCFG_41_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_9_CRYPTOKEY_41_9_BitAddressOffset 0
#define CRYPTOCFG_41_9_CRYPTOKEY_41_9_RegisterSize 32





#define CRYPTOCFG_41_10 (DWC_ufshc_block_BaseAddress + 0x24a8)
#define CRYPTOCFG_41_10_RegisterSize 32
#define CRYPTOCFG_41_10_RegisterResetValue 0x0
#define CRYPTOCFG_41_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_10_CRYPTOKEY_41_10_BitAddressOffset 0
#define CRYPTOCFG_41_10_CRYPTOKEY_41_10_RegisterSize 32





#define CRYPTOCFG_41_11 (DWC_ufshc_block_BaseAddress + 0x24ac)
#define CRYPTOCFG_41_11_RegisterSize 32
#define CRYPTOCFG_41_11_RegisterResetValue 0x0
#define CRYPTOCFG_41_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_11_CRYPTOKEY_41_11_BitAddressOffset 0
#define CRYPTOCFG_41_11_CRYPTOKEY_41_11_RegisterSize 32





#define CRYPTOCFG_41_12 (DWC_ufshc_block_BaseAddress + 0x24b0)
#define CRYPTOCFG_41_12_RegisterSize 32
#define CRYPTOCFG_41_12_RegisterResetValue 0x0
#define CRYPTOCFG_41_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_12_CRYPTOKEY_41_12_BitAddressOffset 0
#define CRYPTOCFG_41_12_CRYPTOKEY_41_12_RegisterSize 32





#define CRYPTOCFG_41_13 (DWC_ufshc_block_BaseAddress + 0x24b4)
#define CRYPTOCFG_41_13_RegisterSize 32
#define CRYPTOCFG_41_13_RegisterResetValue 0x0
#define CRYPTOCFG_41_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_13_CRYPTOKEY_41_13_BitAddressOffset 0
#define CRYPTOCFG_41_13_CRYPTOKEY_41_13_RegisterSize 32





#define CRYPTOCFG_41_14 (DWC_ufshc_block_BaseAddress + 0x24b8)
#define CRYPTOCFG_41_14_RegisterSize 32
#define CRYPTOCFG_41_14_RegisterResetValue 0x0
#define CRYPTOCFG_41_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_14_CRYPTOKEY_41_14_BitAddressOffset 0
#define CRYPTOCFG_41_14_CRYPTOKEY_41_14_RegisterSize 32





#define CRYPTOCFG_41_15 (DWC_ufshc_block_BaseAddress + 0x24bc)
#define CRYPTOCFG_41_15_RegisterSize 32
#define CRYPTOCFG_41_15_RegisterResetValue 0x0
#define CRYPTOCFG_41_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_15_CRYPTOKEY_41_15_BitAddressOffset 0
#define CRYPTOCFG_41_15_CRYPTOKEY_41_15_RegisterSize 32





#define CRYPTOCFG_41_16 (DWC_ufshc_block_BaseAddress + 0x24c0)
#define CRYPTOCFG_41_16_RegisterSize 32
#define CRYPTOCFG_41_16_RegisterResetValue 0x0
#define CRYPTOCFG_41_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_16_DUSIZE_41_BitAddressOffset 0
#define CRYPTOCFG_41_16_DUSIZE_41_RegisterSize 8



#define CRYPTOCFG_41_16_CAPIDX_41_BitAddressOffset 8
#define CRYPTOCFG_41_16_CAPIDX_41_RegisterSize 8



#define CRYPTOCFG_41_16_CFGE_41_BitAddressOffset 31
#define CRYPTOCFG_41_16_CFGE_41_RegisterSize 1





#define CRYPTOCFG_41_17 (DWC_ufshc_block_BaseAddress + 0x24c4)
#define CRYPTOCFG_41_17_RegisterSize 32
#define CRYPTOCFG_41_17_RegisterResetValue 0x0
#define CRYPTOCFG_41_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_17_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_17_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_18 (DWC_ufshc_block_BaseAddress + 0x24c8)
#define CRYPTOCFG_41_18_RegisterSize 32
#define CRYPTOCFG_41_18_RegisterResetValue 0x0
#define CRYPTOCFG_41_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_18_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_18_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_19 (DWC_ufshc_block_BaseAddress + 0x24cc)
#define CRYPTOCFG_41_19_RegisterSize 32
#define CRYPTOCFG_41_19_RegisterResetValue 0x0
#define CRYPTOCFG_41_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_19_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_19_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_20 (DWC_ufshc_block_BaseAddress + 0x24d0)
#define CRYPTOCFG_41_20_RegisterSize 32
#define CRYPTOCFG_41_20_RegisterResetValue 0x0
#define CRYPTOCFG_41_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_20_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_20_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_21 (DWC_ufshc_block_BaseAddress + 0x24d4)
#define CRYPTOCFG_41_21_RegisterSize 32
#define CRYPTOCFG_41_21_RegisterResetValue 0x0
#define CRYPTOCFG_41_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_21_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_21_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_22 (DWC_ufshc_block_BaseAddress + 0x24d8)
#define CRYPTOCFG_41_22_RegisterSize 32
#define CRYPTOCFG_41_22_RegisterResetValue 0x0
#define CRYPTOCFG_41_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_22_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_22_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_23 (DWC_ufshc_block_BaseAddress + 0x24dc)
#define CRYPTOCFG_41_23_RegisterSize 32
#define CRYPTOCFG_41_23_RegisterResetValue 0x0
#define CRYPTOCFG_41_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_23_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_23_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_24 (DWC_ufshc_block_BaseAddress + 0x24e0)
#define CRYPTOCFG_41_24_RegisterSize 32
#define CRYPTOCFG_41_24_RegisterResetValue 0x0
#define CRYPTOCFG_41_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_24_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_24_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_25 (DWC_ufshc_block_BaseAddress + 0x24e4)
#define CRYPTOCFG_41_25_RegisterSize 32
#define CRYPTOCFG_41_25_RegisterResetValue 0x0
#define CRYPTOCFG_41_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_25_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_25_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_26 (DWC_ufshc_block_BaseAddress + 0x24e8)
#define CRYPTOCFG_41_26_RegisterSize 32
#define CRYPTOCFG_41_26_RegisterResetValue 0x0
#define CRYPTOCFG_41_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_26_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_26_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_27 (DWC_ufshc_block_BaseAddress + 0x24ec)
#define CRYPTOCFG_41_27_RegisterSize 32
#define CRYPTOCFG_41_27_RegisterResetValue 0x0
#define CRYPTOCFG_41_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_27_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_27_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_28 (DWC_ufshc_block_BaseAddress + 0x24f0)
#define CRYPTOCFG_41_28_RegisterSize 32
#define CRYPTOCFG_41_28_RegisterResetValue 0x0
#define CRYPTOCFG_41_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_28_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_28_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_29 (DWC_ufshc_block_BaseAddress + 0x24f4)
#define CRYPTOCFG_41_29_RegisterSize 32
#define CRYPTOCFG_41_29_RegisterResetValue 0x0
#define CRYPTOCFG_41_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_29_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_29_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_30 (DWC_ufshc_block_BaseAddress + 0x24f8)
#define CRYPTOCFG_41_30_RegisterSize 32
#define CRYPTOCFG_41_30_RegisterResetValue 0x0
#define CRYPTOCFG_41_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_30_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_30_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_41_31 (DWC_ufshc_block_BaseAddress + 0x24fc)
#define CRYPTOCFG_41_31_RegisterSize 32
#define CRYPTOCFG_41_31_RegisterResetValue 0x0
#define CRYPTOCFG_41_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_41_31_RESERVED_41_BitAddressOffset 0
#define CRYPTOCFG_41_31_RESERVED_41_RegisterSize 32





#define CRYPTOCFG_42_0 (DWC_ufshc_block_BaseAddress + 0x2500)
#define CRYPTOCFG_42_0_RegisterSize 32
#define CRYPTOCFG_42_0_RegisterResetValue 0x0
#define CRYPTOCFG_42_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_0_CRYPTOKEY_42_0_BitAddressOffset 0
#define CRYPTOCFG_42_0_CRYPTOKEY_42_0_RegisterSize 32





#define CRYPTOCFG_42_1 (DWC_ufshc_block_BaseAddress + 0x2504)
#define CRYPTOCFG_42_1_RegisterSize 32
#define CRYPTOCFG_42_1_RegisterResetValue 0x0
#define CRYPTOCFG_42_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_1_CRYPTOKEY_42_1_BitAddressOffset 0
#define CRYPTOCFG_42_1_CRYPTOKEY_42_1_RegisterSize 32





#define CRYPTOCFG_42_2 (DWC_ufshc_block_BaseAddress + 0x2508)
#define CRYPTOCFG_42_2_RegisterSize 32
#define CRYPTOCFG_42_2_RegisterResetValue 0x0
#define CRYPTOCFG_42_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_2_CRYPTOKEY_42_2_BitAddressOffset 0
#define CRYPTOCFG_42_2_CRYPTOKEY_42_2_RegisterSize 32





#define CRYPTOCFG_42_3 (DWC_ufshc_block_BaseAddress + 0x250c)
#define CRYPTOCFG_42_3_RegisterSize 32
#define CRYPTOCFG_42_3_RegisterResetValue 0x0
#define CRYPTOCFG_42_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_3_CRYPTOKEY_42_3_BitAddressOffset 0
#define CRYPTOCFG_42_3_CRYPTOKEY_42_3_RegisterSize 32





#define CRYPTOCFG_42_4 (DWC_ufshc_block_BaseAddress + 0x2510)
#define CRYPTOCFG_42_4_RegisterSize 32
#define CRYPTOCFG_42_4_RegisterResetValue 0x0
#define CRYPTOCFG_42_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_4_CRYPTOKEY_42_4_BitAddressOffset 0
#define CRYPTOCFG_42_4_CRYPTOKEY_42_4_RegisterSize 32





#define CRYPTOCFG_42_5 (DWC_ufshc_block_BaseAddress + 0x2514)
#define CRYPTOCFG_42_5_RegisterSize 32
#define CRYPTOCFG_42_5_RegisterResetValue 0x0
#define CRYPTOCFG_42_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_5_CRYPTOKEY_42_5_BitAddressOffset 0
#define CRYPTOCFG_42_5_CRYPTOKEY_42_5_RegisterSize 32





#define CRYPTOCFG_42_6 (DWC_ufshc_block_BaseAddress + 0x2518)
#define CRYPTOCFG_42_6_RegisterSize 32
#define CRYPTOCFG_42_6_RegisterResetValue 0x0
#define CRYPTOCFG_42_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_6_CRYPTOKEY_42_6_BitAddressOffset 0
#define CRYPTOCFG_42_6_CRYPTOKEY_42_6_RegisterSize 32





#define CRYPTOCFG_42_7 (DWC_ufshc_block_BaseAddress + 0x251c)
#define CRYPTOCFG_42_7_RegisterSize 32
#define CRYPTOCFG_42_7_RegisterResetValue 0x0
#define CRYPTOCFG_42_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_7_CRYPTOKEY_42_7_BitAddressOffset 0
#define CRYPTOCFG_42_7_CRYPTOKEY_42_7_RegisterSize 32





#define CRYPTOCFG_42_8 (DWC_ufshc_block_BaseAddress + 0x2520)
#define CRYPTOCFG_42_8_RegisterSize 32
#define CRYPTOCFG_42_8_RegisterResetValue 0x0
#define CRYPTOCFG_42_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_8_CRYPTOKEY_42_8_BitAddressOffset 0
#define CRYPTOCFG_42_8_CRYPTOKEY_42_8_RegisterSize 32





#define CRYPTOCFG_42_9 (DWC_ufshc_block_BaseAddress + 0x2524)
#define CRYPTOCFG_42_9_RegisterSize 32
#define CRYPTOCFG_42_9_RegisterResetValue 0x0
#define CRYPTOCFG_42_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_9_CRYPTOKEY_42_9_BitAddressOffset 0
#define CRYPTOCFG_42_9_CRYPTOKEY_42_9_RegisterSize 32





#define CRYPTOCFG_42_10 (DWC_ufshc_block_BaseAddress + 0x2528)
#define CRYPTOCFG_42_10_RegisterSize 32
#define CRYPTOCFG_42_10_RegisterResetValue 0x0
#define CRYPTOCFG_42_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_10_CRYPTOKEY_42_10_BitAddressOffset 0
#define CRYPTOCFG_42_10_CRYPTOKEY_42_10_RegisterSize 32





#define CRYPTOCFG_42_11 (DWC_ufshc_block_BaseAddress + 0x252c)
#define CRYPTOCFG_42_11_RegisterSize 32
#define CRYPTOCFG_42_11_RegisterResetValue 0x0
#define CRYPTOCFG_42_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_11_CRYPTOKEY_42_11_BitAddressOffset 0
#define CRYPTOCFG_42_11_CRYPTOKEY_42_11_RegisterSize 32





#define CRYPTOCFG_42_12 (DWC_ufshc_block_BaseAddress + 0x2530)
#define CRYPTOCFG_42_12_RegisterSize 32
#define CRYPTOCFG_42_12_RegisterResetValue 0x0
#define CRYPTOCFG_42_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_12_CRYPTOKEY_42_12_BitAddressOffset 0
#define CRYPTOCFG_42_12_CRYPTOKEY_42_12_RegisterSize 32





#define CRYPTOCFG_42_13 (DWC_ufshc_block_BaseAddress + 0x2534)
#define CRYPTOCFG_42_13_RegisterSize 32
#define CRYPTOCFG_42_13_RegisterResetValue 0x0
#define CRYPTOCFG_42_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_13_CRYPTOKEY_42_13_BitAddressOffset 0
#define CRYPTOCFG_42_13_CRYPTOKEY_42_13_RegisterSize 32





#define CRYPTOCFG_42_14 (DWC_ufshc_block_BaseAddress + 0x2538)
#define CRYPTOCFG_42_14_RegisterSize 32
#define CRYPTOCFG_42_14_RegisterResetValue 0x0
#define CRYPTOCFG_42_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_14_CRYPTOKEY_42_14_BitAddressOffset 0
#define CRYPTOCFG_42_14_CRYPTOKEY_42_14_RegisterSize 32





#define CRYPTOCFG_42_15 (DWC_ufshc_block_BaseAddress + 0x253c)
#define CRYPTOCFG_42_15_RegisterSize 32
#define CRYPTOCFG_42_15_RegisterResetValue 0x0
#define CRYPTOCFG_42_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_15_CRYPTOKEY_42_15_BitAddressOffset 0
#define CRYPTOCFG_42_15_CRYPTOKEY_42_15_RegisterSize 32





#define CRYPTOCFG_42_16 (DWC_ufshc_block_BaseAddress + 0x2540)
#define CRYPTOCFG_42_16_RegisterSize 32
#define CRYPTOCFG_42_16_RegisterResetValue 0x0
#define CRYPTOCFG_42_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_16_DUSIZE_42_BitAddressOffset 0
#define CRYPTOCFG_42_16_DUSIZE_42_RegisterSize 8



#define CRYPTOCFG_42_16_CAPIDX_42_BitAddressOffset 8
#define CRYPTOCFG_42_16_CAPIDX_42_RegisterSize 8



#define CRYPTOCFG_42_16_CFGE_42_BitAddressOffset 31
#define CRYPTOCFG_42_16_CFGE_42_RegisterSize 1





#define CRYPTOCFG_42_17 (DWC_ufshc_block_BaseAddress + 0x2544)
#define CRYPTOCFG_42_17_RegisterSize 32
#define CRYPTOCFG_42_17_RegisterResetValue 0x0
#define CRYPTOCFG_42_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_17_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_17_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_18 (DWC_ufshc_block_BaseAddress + 0x2548)
#define CRYPTOCFG_42_18_RegisterSize 32
#define CRYPTOCFG_42_18_RegisterResetValue 0x0
#define CRYPTOCFG_42_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_18_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_18_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_19 (DWC_ufshc_block_BaseAddress + 0x254c)
#define CRYPTOCFG_42_19_RegisterSize 32
#define CRYPTOCFG_42_19_RegisterResetValue 0x0
#define CRYPTOCFG_42_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_19_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_19_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_20 (DWC_ufshc_block_BaseAddress + 0x2550)
#define CRYPTOCFG_42_20_RegisterSize 32
#define CRYPTOCFG_42_20_RegisterResetValue 0x0
#define CRYPTOCFG_42_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_20_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_20_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_21 (DWC_ufshc_block_BaseAddress + 0x2554)
#define CRYPTOCFG_42_21_RegisterSize 32
#define CRYPTOCFG_42_21_RegisterResetValue 0x0
#define CRYPTOCFG_42_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_21_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_21_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_22 (DWC_ufshc_block_BaseAddress + 0x2558)
#define CRYPTOCFG_42_22_RegisterSize 32
#define CRYPTOCFG_42_22_RegisterResetValue 0x0
#define CRYPTOCFG_42_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_22_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_22_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_23 (DWC_ufshc_block_BaseAddress + 0x255c)
#define CRYPTOCFG_42_23_RegisterSize 32
#define CRYPTOCFG_42_23_RegisterResetValue 0x0
#define CRYPTOCFG_42_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_23_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_23_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_24 (DWC_ufshc_block_BaseAddress + 0x2560)
#define CRYPTOCFG_42_24_RegisterSize 32
#define CRYPTOCFG_42_24_RegisterResetValue 0x0
#define CRYPTOCFG_42_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_24_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_24_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_25 (DWC_ufshc_block_BaseAddress + 0x2564)
#define CRYPTOCFG_42_25_RegisterSize 32
#define CRYPTOCFG_42_25_RegisterResetValue 0x0
#define CRYPTOCFG_42_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_25_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_25_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_26 (DWC_ufshc_block_BaseAddress + 0x2568)
#define CRYPTOCFG_42_26_RegisterSize 32
#define CRYPTOCFG_42_26_RegisterResetValue 0x0
#define CRYPTOCFG_42_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_26_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_26_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_27 (DWC_ufshc_block_BaseAddress + 0x256c)
#define CRYPTOCFG_42_27_RegisterSize 32
#define CRYPTOCFG_42_27_RegisterResetValue 0x0
#define CRYPTOCFG_42_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_27_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_27_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_28 (DWC_ufshc_block_BaseAddress + 0x2570)
#define CRYPTOCFG_42_28_RegisterSize 32
#define CRYPTOCFG_42_28_RegisterResetValue 0x0
#define CRYPTOCFG_42_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_28_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_28_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_29 (DWC_ufshc_block_BaseAddress + 0x2574)
#define CRYPTOCFG_42_29_RegisterSize 32
#define CRYPTOCFG_42_29_RegisterResetValue 0x0
#define CRYPTOCFG_42_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_29_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_29_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_30 (DWC_ufshc_block_BaseAddress + 0x2578)
#define CRYPTOCFG_42_30_RegisterSize 32
#define CRYPTOCFG_42_30_RegisterResetValue 0x0
#define CRYPTOCFG_42_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_30_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_30_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_42_31 (DWC_ufshc_block_BaseAddress + 0x257c)
#define CRYPTOCFG_42_31_RegisterSize 32
#define CRYPTOCFG_42_31_RegisterResetValue 0x0
#define CRYPTOCFG_42_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_42_31_RESERVED_42_BitAddressOffset 0
#define CRYPTOCFG_42_31_RESERVED_42_RegisterSize 32





#define CRYPTOCFG_43_0 (DWC_ufshc_block_BaseAddress + 0x2580)
#define CRYPTOCFG_43_0_RegisterSize 32
#define CRYPTOCFG_43_0_RegisterResetValue 0x0
#define CRYPTOCFG_43_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_0_CRYPTOKEY_43_0_BitAddressOffset 0
#define CRYPTOCFG_43_0_CRYPTOKEY_43_0_RegisterSize 32





#define CRYPTOCFG_43_1 (DWC_ufshc_block_BaseAddress + 0x2584)
#define CRYPTOCFG_43_1_RegisterSize 32
#define CRYPTOCFG_43_1_RegisterResetValue 0x0
#define CRYPTOCFG_43_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_1_CRYPTOKEY_43_1_BitAddressOffset 0
#define CRYPTOCFG_43_1_CRYPTOKEY_43_1_RegisterSize 32





#define CRYPTOCFG_43_2 (DWC_ufshc_block_BaseAddress + 0x2588)
#define CRYPTOCFG_43_2_RegisterSize 32
#define CRYPTOCFG_43_2_RegisterResetValue 0x0
#define CRYPTOCFG_43_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_2_CRYPTOKEY_43_2_BitAddressOffset 0
#define CRYPTOCFG_43_2_CRYPTOKEY_43_2_RegisterSize 32





#define CRYPTOCFG_43_3 (DWC_ufshc_block_BaseAddress + 0x258c)
#define CRYPTOCFG_43_3_RegisterSize 32
#define CRYPTOCFG_43_3_RegisterResetValue 0x0
#define CRYPTOCFG_43_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_3_CRYPTOKEY_43_3_BitAddressOffset 0
#define CRYPTOCFG_43_3_CRYPTOKEY_43_3_RegisterSize 32





#define CRYPTOCFG_43_4 (DWC_ufshc_block_BaseAddress + 0x2590)
#define CRYPTOCFG_43_4_RegisterSize 32
#define CRYPTOCFG_43_4_RegisterResetValue 0x0
#define CRYPTOCFG_43_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_4_CRYPTOKEY_43_4_BitAddressOffset 0
#define CRYPTOCFG_43_4_CRYPTOKEY_43_4_RegisterSize 32





#define CRYPTOCFG_43_5 (DWC_ufshc_block_BaseAddress + 0x2594)
#define CRYPTOCFG_43_5_RegisterSize 32
#define CRYPTOCFG_43_5_RegisterResetValue 0x0
#define CRYPTOCFG_43_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_5_CRYPTOKEY_43_5_BitAddressOffset 0
#define CRYPTOCFG_43_5_CRYPTOKEY_43_5_RegisterSize 32





#define CRYPTOCFG_43_6 (DWC_ufshc_block_BaseAddress + 0x2598)
#define CRYPTOCFG_43_6_RegisterSize 32
#define CRYPTOCFG_43_6_RegisterResetValue 0x0
#define CRYPTOCFG_43_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_6_CRYPTOKEY_43_6_BitAddressOffset 0
#define CRYPTOCFG_43_6_CRYPTOKEY_43_6_RegisterSize 32





#define CRYPTOCFG_43_7 (DWC_ufshc_block_BaseAddress + 0x259c)
#define CRYPTOCFG_43_7_RegisterSize 32
#define CRYPTOCFG_43_7_RegisterResetValue 0x0
#define CRYPTOCFG_43_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_7_CRYPTOKEY_43_7_BitAddressOffset 0
#define CRYPTOCFG_43_7_CRYPTOKEY_43_7_RegisterSize 32





#define CRYPTOCFG_43_8 (DWC_ufshc_block_BaseAddress + 0x25a0)
#define CRYPTOCFG_43_8_RegisterSize 32
#define CRYPTOCFG_43_8_RegisterResetValue 0x0
#define CRYPTOCFG_43_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_8_CRYPTOKEY_43_8_BitAddressOffset 0
#define CRYPTOCFG_43_8_CRYPTOKEY_43_8_RegisterSize 32





#define CRYPTOCFG_43_9 (DWC_ufshc_block_BaseAddress + 0x25a4)
#define CRYPTOCFG_43_9_RegisterSize 32
#define CRYPTOCFG_43_9_RegisterResetValue 0x0
#define CRYPTOCFG_43_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_9_CRYPTOKEY_43_9_BitAddressOffset 0
#define CRYPTOCFG_43_9_CRYPTOKEY_43_9_RegisterSize 32





#define CRYPTOCFG_43_10 (DWC_ufshc_block_BaseAddress + 0x25a8)
#define CRYPTOCFG_43_10_RegisterSize 32
#define CRYPTOCFG_43_10_RegisterResetValue 0x0
#define CRYPTOCFG_43_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_10_CRYPTOKEY_43_10_BitAddressOffset 0
#define CRYPTOCFG_43_10_CRYPTOKEY_43_10_RegisterSize 32





#define CRYPTOCFG_43_11 (DWC_ufshc_block_BaseAddress + 0x25ac)
#define CRYPTOCFG_43_11_RegisterSize 32
#define CRYPTOCFG_43_11_RegisterResetValue 0x0
#define CRYPTOCFG_43_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_11_CRYPTOKEY_43_11_BitAddressOffset 0
#define CRYPTOCFG_43_11_CRYPTOKEY_43_11_RegisterSize 32





#define CRYPTOCFG_43_12 (DWC_ufshc_block_BaseAddress + 0x25b0)
#define CRYPTOCFG_43_12_RegisterSize 32
#define CRYPTOCFG_43_12_RegisterResetValue 0x0
#define CRYPTOCFG_43_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_12_CRYPTOKEY_43_12_BitAddressOffset 0
#define CRYPTOCFG_43_12_CRYPTOKEY_43_12_RegisterSize 32





#define CRYPTOCFG_43_13 (DWC_ufshc_block_BaseAddress + 0x25b4)
#define CRYPTOCFG_43_13_RegisterSize 32
#define CRYPTOCFG_43_13_RegisterResetValue 0x0
#define CRYPTOCFG_43_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_13_CRYPTOKEY_43_13_BitAddressOffset 0
#define CRYPTOCFG_43_13_CRYPTOKEY_43_13_RegisterSize 32





#define CRYPTOCFG_43_14 (DWC_ufshc_block_BaseAddress + 0x25b8)
#define CRYPTOCFG_43_14_RegisterSize 32
#define CRYPTOCFG_43_14_RegisterResetValue 0x0
#define CRYPTOCFG_43_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_14_CRYPTOKEY_43_14_BitAddressOffset 0
#define CRYPTOCFG_43_14_CRYPTOKEY_43_14_RegisterSize 32





#define CRYPTOCFG_43_15 (DWC_ufshc_block_BaseAddress + 0x25bc)
#define CRYPTOCFG_43_15_RegisterSize 32
#define CRYPTOCFG_43_15_RegisterResetValue 0x0
#define CRYPTOCFG_43_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_15_CRYPTOKEY_43_15_BitAddressOffset 0
#define CRYPTOCFG_43_15_CRYPTOKEY_43_15_RegisterSize 32





#define CRYPTOCFG_43_16 (DWC_ufshc_block_BaseAddress + 0x25c0)
#define CRYPTOCFG_43_16_RegisterSize 32
#define CRYPTOCFG_43_16_RegisterResetValue 0x0
#define CRYPTOCFG_43_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_16_DUSIZE_43_BitAddressOffset 0
#define CRYPTOCFG_43_16_DUSIZE_43_RegisterSize 8



#define CRYPTOCFG_43_16_CAPIDX_43_BitAddressOffset 8
#define CRYPTOCFG_43_16_CAPIDX_43_RegisterSize 8



#define CRYPTOCFG_43_16_CFGE_43_BitAddressOffset 31
#define CRYPTOCFG_43_16_CFGE_43_RegisterSize 1





#define CRYPTOCFG_43_17 (DWC_ufshc_block_BaseAddress + 0x25c4)
#define CRYPTOCFG_43_17_RegisterSize 32
#define CRYPTOCFG_43_17_RegisterResetValue 0x0
#define CRYPTOCFG_43_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_17_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_17_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_18 (DWC_ufshc_block_BaseAddress + 0x25c8)
#define CRYPTOCFG_43_18_RegisterSize 32
#define CRYPTOCFG_43_18_RegisterResetValue 0x0
#define CRYPTOCFG_43_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_18_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_18_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_19 (DWC_ufshc_block_BaseAddress + 0x25cc)
#define CRYPTOCFG_43_19_RegisterSize 32
#define CRYPTOCFG_43_19_RegisterResetValue 0x0
#define CRYPTOCFG_43_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_19_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_19_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_20 (DWC_ufshc_block_BaseAddress + 0x25d0)
#define CRYPTOCFG_43_20_RegisterSize 32
#define CRYPTOCFG_43_20_RegisterResetValue 0x0
#define CRYPTOCFG_43_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_20_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_20_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_21 (DWC_ufshc_block_BaseAddress + 0x25d4)
#define CRYPTOCFG_43_21_RegisterSize 32
#define CRYPTOCFG_43_21_RegisterResetValue 0x0
#define CRYPTOCFG_43_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_21_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_21_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_22 (DWC_ufshc_block_BaseAddress + 0x25d8)
#define CRYPTOCFG_43_22_RegisterSize 32
#define CRYPTOCFG_43_22_RegisterResetValue 0x0
#define CRYPTOCFG_43_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_22_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_22_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_23 (DWC_ufshc_block_BaseAddress + 0x25dc)
#define CRYPTOCFG_43_23_RegisterSize 32
#define CRYPTOCFG_43_23_RegisterResetValue 0x0
#define CRYPTOCFG_43_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_23_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_23_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_24 (DWC_ufshc_block_BaseAddress + 0x25e0)
#define CRYPTOCFG_43_24_RegisterSize 32
#define CRYPTOCFG_43_24_RegisterResetValue 0x0
#define CRYPTOCFG_43_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_24_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_24_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_25 (DWC_ufshc_block_BaseAddress + 0x25e4)
#define CRYPTOCFG_43_25_RegisterSize 32
#define CRYPTOCFG_43_25_RegisterResetValue 0x0
#define CRYPTOCFG_43_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_25_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_25_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_26 (DWC_ufshc_block_BaseAddress + 0x25e8)
#define CRYPTOCFG_43_26_RegisterSize 32
#define CRYPTOCFG_43_26_RegisterResetValue 0x0
#define CRYPTOCFG_43_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_26_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_26_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_27 (DWC_ufshc_block_BaseAddress + 0x25ec)
#define CRYPTOCFG_43_27_RegisterSize 32
#define CRYPTOCFG_43_27_RegisterResetValue 0x0
#define CRYPTOCFG_43_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_27_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_27_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_28 (DWC_ufshc_block_BaseAddress + 0x25f0)
#define CRYPTOCFG_43_28_RegisterSize 32
#define CRYPTOCFG_43_28_RegisterResetValue 0x0
#define CRYPTOCFG_43_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_28_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_28_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_29 (DWC_ufshc_block_BaseAddress + 0x25f4)
#define CRYPTOCFG_43_29_RegisterSize 32
#define CRYPTOCFG_43_29_RegisterResetValue 0x0
#define CRYPTOCFG_43_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_29_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_29_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_30 (DWC_ufshc_block_BaseAddress + 0x25f8)
#define CRYPTOCFG_43_30_RegisterSize 32
#define CRYPTOCFG_43_30_RegisterResetValue 0x0
#define CRYPTOCFG_43_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_30_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_30_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_43_31 (DWC_ufshc_block_BaseAddress + 0x25fc)
#define CRYPTOCFG_43_31_RegisterSize 32
#define CRYPTOCFG_43_31_RegisterResetValue 0x0
#define CRYPTOCFG_43_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_43_31_RESERVED_43_BitAddressOffset 0
#define CRYPTOCFG_43_31_RESERVED_43_RegisterSize 32





#define CRYPTOCFG_44_0 (DWC_ufshc_block_BaseAddress + 0x2600)
#define CRYPTOCFG_44_0_RegisterSize 32
#define CRYPTOCFG_44_0_RegisterResetValue 0x0
#define CRYPTOCFG_44_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_0_CRYPTOKEY_44_0_BitAddressOffset 0
#define CRYPTOCFG_44_0_CRYPTOKEY_44_0_RegisterSize 32





#define CRYPTOCFG_44_1 (DWC_ufshc_block_BaseAddress + 0x2604)
#define CRYPTOCFG_44_1_RegisterSize 32
#define CRYPTOCFG_44_1_RegisterResetValue 0x0
#define CRYPTOCFG_44_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_1_CRYPTOKEY_44_1_BitAddressOffset 0
#define CRYPTOCFG_44_1_CRYPTOKEY_44_1_RegisterSize 32





#define CRYPTOCFG_44_2 (DWC_ufshc_block_BaseAddress + 0x2608)
#define CRYPTOCFG_44_2_RegisterSize 32
#define CRYPTOCFG_44_2_RegisterResetValue 0x0
#define CRYPTOCFG_44_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_2_CRYPTOKEY_44_2_BitAddressOffset 0
#define CRYPTOCFG_44_2_CRYPTOKEY_44_2_RegisterSize 32





#define CRYPTOCFG_44_3 (DWC_ufshc_block_BaseAddress + 0x260c)
#define CRYPTOCFG_44_3_RegisterSize 32
#define CRYPTOCFG_44_3_RegisterResetValue 0x0
#define CRYPTOCFG_44_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_3_CRYPTOKEY_44_3_BitAddressOffset 0
#define CRYPTOCFG_44_3_CRYPTOKEY_44_3_RegisterSize 32





#define CRYPTOCFG_44_4 (DWC_ufshc_block_BaseAddress + 0x2610)
#define CRYPTOCFG_44_4_RegisterSize 32
#define CRYPTOCFG_44_4_RegisterResetValue 0x0
#define CRYPTOCFG_44_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_4_CRYPTOKEY_44_4_BitAddressOffset 0
#define CRYPTOCFG_44_4_CRYPTOKEY_44_4_RegisterSize 32





#define CRYPTOCFG_44_5 (DWC_ufshc_block_BaseAddress + 0x2614)
#define CRYPTOCFG_44_5_RegisterSize 32
#define CRYPTOCFG_44_5_RegisterResetValue 0x0
#define CRYPTOCFG_44_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_5_CRYPTOKEY_44_5_BitAddressOffset 0
#define CRYPTOCFG_44_5_CRYPTOKEY_44_5_RegisterSize 32





#define CRYPTOCFG_44_6 (DWC_ufshc_block_BaseAddress + 0x2618)
#define CRYPTOCFG_44_6_RegisterSize 32
#define CRYPTOCFG_44_6_RegisterResetValue 0x0
#define CRYPTOCFG_44_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_6_CRYPTOKEY_44_6_BitAddressOffset 0
#define CRYPTOCFG_44_6_CRYPTOKEY_44_6_RegisterSize 32





#define CRYPTOCFG_44_7 (DWC_ufshc_block_BaseAddress + 0x261c)
#define CRYPTOCFG_44_7_RegisterSize 32
#define CRYPTOCFG_44_7_RegisterResetValue 0x0
#define CRYPTOCFG_44_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_7_CRYPTOKEY_44_7_BitAddressOffset 0
#define CRYPTOCFG_44_7_CRYPTOKEY_44_7_RegisterSize 32





#define CRYPTOCFG_44_8 (DWC_ufshc_block_BaseAddress + 0x2620)
#define CRYPTOCFG_44_8_RegisterSize 32
#define CRYPTOCFG_44_8_RegisterResetValue 0x0
#define CRYPTOCFG_44_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_8_CRYPTOKEY_44_8_BitAddressOffset 0
#define CRYPTOCFG_44_8_CRYPTOKEY_44_8_RegisterSize 32





#define CRYPTOCFG_44_9 (DWC_ufshc_block_BaseAddress + 0x2624)
#define CRYPTOCFG_44_9_RegisterSize 32
#define CRYPTOCFG_44_9_RegisterResetValue 0x0
#define CRYPTOCFG_44_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_9_CRYPTOKEY_44_9_BitAddressOffset 0
#define CRYPTOCFG_44_9_CRYPTOKEY_44_9_RegisterSize 32





#define CRYPTOCFG_44_10 (DWC_ufshc_block_BaseAddress + 0x2628)
#define CRYPTOCFG_44_10_RegisterSize 32
#define CRYPTOCFG_44_10_RegisterResetValue 0x0
#define CRYPTOCFG_44_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_10_CRYPTOKEY_44_10_BitAddressOffset 0
#define CRYPTOCFG_44_10_CRYPTOKEY_44_10_RegisterSize 32





#define CRYPTOCFG_44_11 (DWC_ufshc_block_BaseAddress + 0x262c)
#define CRYPTOCFG_44_11_RegisterSize 32
#define CRYPTOCFG_44_11_RegisterResetValue 0x0
#define CRYPTOCFG_44_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_11_CRYPTOKEY_44_11_BitAddressOffset 0
#define CRYPTOCFG_44_11_CRYPTOKEY_44_11_RegisterSize 32





#define CRYPTOCFG_44_12 (DWC_ufshc_block_BaseAddress + 0x2630)
#define CRYPTOCFG_44_12_RegisterSize 32
#define CRYPTOCFG_44_12_RegisterResetValue 0x0
#define CRYPTOCFG_44_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_12_CRYPTOKEY_44_12_BitAddressOffset 0
#define CRYPTOCFG_44_12_CRYPTOKEY_44_12_RegisterSize 32





#define CRYPTOCFG_44_13 (DWC_ufshc_block_BaseAddress + 0x2634)
#define CRYPTOCFG_44_13_RegisterSize 32
#define CRYPTOCFG_44_13_RegisterResetValue 0x0
#define CRYPTOCFG_44_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_13_CRYPTOKEY_44_13_BitAddressOffset 0
#define CRYPTOCFG_44_13_CRYPTOKEY_44_13_RegisterSize 32





#define CRYPTOCFG_44_14 (DWC_ufshc_block_BaseAddress + 0x2638)
#define CRYPTOCFG_44_14_RegisterSize 32
#define CRYPTOCFG_44_14_RegisterResetValue 0x0
#define CRYPTOCFG_44_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_14_CRYPTOKEY_44_14_BitAddressOffset 0
#define CRYPTOCFG_44_14_CRYPTOKEY_44_14_RegisterSize 32





#define CRYPTOCFG_44_15 (DWC_ufshc_block_BaseAddress + 0x263c)
#define CRYPTOCFG_44_15_RegisterSize 32
#define CRYPTOCFG_44_15_RegisterResetValue 0x0
#define CRYPTOCFG_44_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_15_CRYPTOKEY_44_15_BitAddressOffset 0
#define CRYPTOCFG_44_15_CRYPTOKEY_44_15_RegisterSize 32





#define CRYPTOCFG_44_16 (DWC_ufshc_block_BaseAddress + 0x2640)
#define CRYPTOCFG_44_16_RegisterSize 32
#define CRYPTOCFG_44_16_RegisterResetValue 0x0
#define CRYPTOCFG_44_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_16_DUSIZE_44_BitAddressOffset 0
#define CRYPTOCFG_44_16_DUSIZE_44_RegisterSize 8



#define CRYPTOCFG_44_16_CAPIDX_44_BitAddressOffset 8
#define CRYPTOCFG_44_16_CAPIDX_44_RegisterSize 8



#define CRYPTOCFG_44_16_CFGE_44_BitAddressOffset 31
#define CRYPTOCFG_44_16_CFGE_44_RegisterSize 1





#define CRYPTOCFG_44_17 (DWC_ufshc_block_BaseAddress + 0x2644)
#define CRYPTOCFG_44_17_RegisterSize 32
#define CRYPTOCFG_44_17_RegisterResetValue 0x0
#define CRYPTOCFG_44_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_17_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_17_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_18 (DWC_ufshc_block_BaseAddress + 0x2648)
#define CRYPTOCFG_44_18_RegisterSize 32
#define CRYPTOCFG_44_18_RegisterResetValue 0x0
#define CRYPTOCFG_44_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_18_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_18_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_19 (DWC_ufshc_block_BaseAddress + 0x264c)
#define CRYPTOCFG_44_19_RegisterSize 32
#define CRYPTOCFG_44_19_RegisterResetValue 0x0
#define CRYPTOCFG_44_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_19_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_19_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_20 (DWC_ufshc_block_BaseAddress + 0x2650)
#define CRYPTOCFG_44_20_RegisterSize 32
#define CRYPTOCFG_44_20_RegisterResetValue 0x0
#define CRYPTOCFG_44_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_20_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_20_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_21 (DWC_ufshc_block_BaseAddress + 0x2654)
#define CRYPTOCFG_44_21_RegisterSize 32
#define CRYPTOCFG_44_21_RegisterResetValue 0x0
#define CRYPTOCFG_44_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_21_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_21_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_22 (DWC_ufshc_block_BaseAddress + 0x2658)
#define CRYPTOCFG_44_22_RegisterSize 32
#define CRYPTOCFG_44_22_RegisterResetValue 0x0
#define CRYPTOCFG_44_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_22_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_22_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_23 (DWC_ufshc_block_BaseAddress + 0x265c)
#define CRYPTOCFG_44_23_RegisterSize 32
#define CRYPTOCFG_44_23_RegisterResetValue 0x0
#define CRYPTOCFG_44_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_23_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_23_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_24 (DWC_ufshc_block_BaseAddress + 0x2660)
#define CRYPTOCFG_44_24_RegisterSize 32
#define CRYPTOCFG_44_24_RegisterResetValue 0x0
#define CRYPTOCFG_44_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_24_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_24_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_25 (DWC_ufshc_block_BaseAddress + 0x2664)
#define CRYPTOCFG_44_25_RegisterSize 32
#define CRYPTOCFG_44_25_RegisterResetValue 0x0
#define CRYPTOCFG_44_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_25_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_25_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_26 (DWC_ufshc_block_BaseAddress + 0x2668)
#define CRYPTOCFG_44_26_RegisterSize 32
#define CRYPTOCFG_44_26_RegisterResetValue 0x0
#define CRYPTOCFG_44_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_26_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_26_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_27 (DWC_ufshc_block_BaseAddress + 0x266c)
#define CRYPTOCFG_44_27_RegisterSize 32
#define CRYPTOCFG_44_27_RegisterResetValue 0x0
#define CRYPTOCFG_44_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_27_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_27_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_28 (DWC_ufshc_block_BaseAddress + 0x2670)
#define CRYPTOCFG_44_28_RegisterSize 32
#define CRYPTOCFG_44_28_RegisterResetValue 0x0
#define CRYPTOCFG_44_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_28_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_28_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_29 (DWC_ufshc_block_BaseAddress + 0x2674)
#define CRYPTOCFG_44_29_RegisterSize 32
#define CRYPTOCFG_44_29_RegisterResetValue 0x0
#define CRYPTOCFG_44_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_29_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_29_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_30 (DWC_ufshc_block_BaseAddress + 0x2678)
#define CRYPTOCFG_44_30_RegisterSize 32
#define CRYPTOCFG_44_30_RegisterResetValue 0x0
#define CRYPTOCFG_44_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_30_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_30_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_44_31 (DWC_ufshc_block_BaseAddress + 0x267c)
#define CRYPTOCFG_44_31_RegisterSize 32
#define CRYPTOCFG_44_31_RegisterResetValue 0x0
#define CRYPTOCFG_44_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_44_31_RESERVED_44_BitAddressOffset 0
#define CRYPTOCFG_44_31_RESERVED_44_RegisterSize 32





#define CRYPTOCFG_45_0 (DWC_ufshc_block_BaseAddress + 0x2680)
#define CRYPTOCFG_45_0_RegisterSize 32
#define CRYPTOCFG_45_0_RegisterResetValue 0x0
#define CRYPTOCFG_45_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_0_CRYPTOKEY_45_0_BitAddressOffset 0
#define CRYPTOCFG_45_0_CRYPTOKEY_45_0_RegisterSize 32





#define CRYPTOCFG_45_1 (DWC_ufshc_block_BaseAddress + 0x2684)
#define CRYPTOCFG_45_1_RegisterSize 32
#define CRYPTOCFG_45_1_RegisterResetValue 0x0
#define CRYPTOCFG_45_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_1_CRYPTOKEY_45_1_BitAddressOffset 0
#define CRYPTOCFG_45_1_CRYPTOKEY_45_1_RegisterSize 32





#define CRYPTOCFG_45_2 (DWC_ufshc_block_BaseAddress + 0x2688)
#define CRYPTOCFG_45_2_RegisterSize 32
#define CRYPTOCFG_45_2_RegisterResetValue 0x0
#define CRYPTOCFG_45_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_2_CRYPTOKEY_45_2_BitAddressOffset 0
#define CRYPTOCFG_45_2_CRYPTOKEY_45_2_RegisterSize 32





#define CRYPTOCFG_45_3 (DWC_ufshc_block_BaseAddress + 0x268c)
#define CRYPTOCFG_45_3_RegisterSize 32
#define CRYPTOCFG_45_3_RegisterResetValue 0x0
#define CRYPTOCFG_45_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_3_CRYPTOKEY_45_3_BitAddressOffset 0
#define CRYPTOCFG_45_3_CRYPTOKEY_45_3_RegisterSize 32





#define CRYPTOCFG_45_4 (DWC_ufshc_block_BaseAddress + 0x2690)
#define CRYPTOCFG_45_4_RegisterSize 32
#define CRYPTOCFG_45_4_RegisterResetValue 0x0
#define CRYPTOCFG_45_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_4_CRYPTOKEY_45_4_BitAddressOffset 0
#define CRYPTOCFG_45_4_CRYPTOKEY_45_4_RegisterSize 32





#define CRYPTOCFG_45_5 (DWC_ufshc_block_BaseAddress + 0x2694)
#define CRYPTOCFG_45_5_RegisterSize 32
#define CRYPTOCFG_45_5_RegisterResetValue 0x0
#define CRYPTOCFG_45_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_5_CRYPTOKEY_45_5_BitAddressOffset 0
#define CRYPTOCFG_45_5_CRYPTOKEY_45_5_RegisterSize 32





#define CRYPTOCFG_45_6 (DWC_ufshc_block_BaseAddress + 0x2698)
#define CRYPTOCFG_45_6_RegisterSize 32
#define CRYPTOCFG_45_6_RegisterResetValue 0x0
#define CRYPTOCFG_45_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_6_CRYPTOKEY_45_6_BitAddressOffset 0
#define CRYPTOCFG_45_6_CRYPTOKEY_45_6_RegisterSize 32





#define CRYPTOCFG_45_7 (DWC_ufshc_block_BaseAddress + 0x269c)
#define CRYPTOCFG_45_7_RegisterSize 32
#define CRYPTOCFG_45_7_RegisterResetValue 0x0
#define CRYPTOCFG_45_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_7_CRYPTOKEY_45_7_BitAddressOffset 0
#define CRYPTOCFG_45_7_CRYPTOKEY_45_7_RegisterSize 32





#define CRYPTOCFG_45_8 (DWC_ufshc_block_BaseAddress + 0x26a0)
#define CRYPTOCFG_45_8_RegisterSize 32
#define CRYPTOCFG_45_8_RegisterResetValue 0x0
#define CRYPTOCFG_45_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_8_CRYPTOKEY_45_8_BitAddressOffset 0
#define CRYPTOCFG_45_8_CRYPTOKEY_45_8_RegisterSize 32





#define CRYPTOCFG_45_9 (DWC_ufshc_block_BaseAddress + 0x26a4)
#define CRYPTOCFG_45_9_RegisterSize 32
#define CRYPTOCFG_45_9_RegisterResetValue 0x0
#define CRYPTOCFG_45_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_9_CRYPTOKEY_45_9_BitAddressOffset 0
#define CRYPTOCFG_45_9_CRYPTOKEY_45_9_RegisterSize 32





#define CRYPTOCFG_45_10 (DWC_ufshc_block_BaseAddress + 0x26a8)
#define CRYPTOCFG_45_10_RegisterSize 32
#define CRYPTOCFG_45_10_RegisterResetValue 0x0
#define CRYPTOCFG_45_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_10_CRYPTOKEY_45_10_BitAddressOffset 0
#define CRYPTOCFG_45_10_CRYPTOKEY_45_10_RegisterSize 32





#define CRYPTOCFG_45_11 (DWC_ufshc_block_BaseAddress + 0x26ac)
#define CRYPTOCFG_45_11_RegisterSize 32
#define CRYPTOCFG_45_11_RegisterResetValue 0x0
#define CRYPTOCFG_45_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_11_CRYPTOKEY_45_11_BitAddressOffset 0
#define CRYPTOCFG_45_11_CRYPTOKEY_45_11_RegisterSize 32





#define CRYPTOCFG_45_12 (DWC_ufshc_block_BaseAddress + 0x26b0)
#define CRYPTOCFG_45_12_RegisterSize 32
#define CRYPTOCFG_45_12_RegisterResetValue 0x0
#define CRYPTOCFG_45_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_12_CRYPTOKEY_45_12_BitAddressOffset 0
#define CRYPTOCFG_45_12_CRYPTOKEY_45_12_RegisterSize 32





#define CRYPTOCFG_45_13 (DWC_ufshc_block_BaseAddress + 0x26b4)
#define CRYPTOCFG_45_13_RegisterSize 32
#define CRYPTOCFG_45_13_RegisterResetValue 0x0
#define CRYPTOCFG_45_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_13_CRYPTOKEY_45_13_BitAddressOffset 0
#define CRYPTOCFG_45_13_CRYPTOKEY_45_13_RegisterSize 32





#define CRYPTOCFG_45_14 (DWC_ufshc_block_BaseAddress + 0x26b8)
#define CRYPTOCFG_45_14_RegisterSize 32
#define CRYPTOCFG_45_14_RegisterResetValue 0x0
#define CRYPTOCFG_45_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_14_CRYPTOKEY_45_14_BitAddressOffset 0
#define CRYPTOCFG_45_14_CRYPTOKEY_45_14_RegisterSize 32





#define CRYPTOCFG_45_15 (DWC_ufshc_block_BaseAddress + 0x26bc)
#define CRYPTOCFG_45_15_RegisterSize 32
#define CRYPTOCFG_45_15_RegisterResetValue 0x0
#define CRYPTOCFG_45_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_15_CRYPTOKEY_45_15_BitAddressOffset 0
#define CRYPTOCFG_45_15_CRYPTOKEY_45_15_RegisterSize 32





#define CRYPTOCFG_45_16 (DWC_ufshc_block_BaseAddress + 0x26c0)
#define CRYPTOCFG_45_16_RegisterSize 32
#define CRYPTOCFG_45_16_RegisterResetValue 0x0
#define CRYPTOCFG_45_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_16_DUSIZE_45_BitAddressOffset 0
#define CRYPTOCFG_45_16_DUSIZE_45_RegisterSize 8



#define CRYPTOCFG_45_16_CAPIDX_45_BitAddressOffset 8
#define CRYPTOCFG_45_16_CAPIDX_45_RegisterSize 8



#define CRYPTOCFG_45_16_CFGE_45_BitAddressOffset 31
#define CRYPTOCFG_45_16_CFGE_45_RegisterSize 1





#define CRYPTOCFG_45_17 (DWC_ufshc_block_BaseAddress + 0x26c4)
#define CRYPTOCFG_45_17_RegisterSize 32
#define CRYPTOCFG_45_17_RegisterResetValue 0x0
#define CRYPTOCFG_45_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_17_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_17_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_18 (DWC_ufshc_block_BaseAddress + 0x26c8)
#define CRYPTOCFG_45_18_RegisterSize 32
#define CRYPTOCFG_45_18_RegisterResetValue 0x0
#define CRYPTOCFG_45_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_18_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_18_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_19 (DWC_ufshc_block_BaseAddress + 0x26cc)
#define CRYPTOCFG_45_19_RegisterSize 32
#define CRYPTOCFG_45_19_RegisterResetValue 0x0
#define CRYPTOCFG_45_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_19_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_19_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_20 (DWC_ufshc_block_BaseAddress + 0x26d0)
#define CRYPTOCFG_45_20_RegisterSize 32
#define CRYPTOCFG_45_20_RegisterResetValue 0x0
#define CRYPTOCFG_45_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_20_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_20_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_21 (DWC_ufshc_block_BaseAddress + 0x26d4)
#define CRYPTOCFG_45_21_RegisterSize 32
#define CRYPTOCFG_45_21_RegisterResetValue 0x0
#define CRYPTOCFG_45_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_21_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_21_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_22 (DWC_ufshc_block_BaseAddress + 0x26d8)
#define CRYPTOCFG_45_22_RegisterSize 32
#define CRYPTOCFG_45_22_RegisterResetValue 0x0
#define CRYPTOCFG_45_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_22_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_22_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_23 (DWC_ufshc_block_BaseAddress + 0x26dc)
#define CRYPTOCFG_45_23_RegisterSize 32
#define CRYPTOCFG_45_23_RegisterResetValue 0x0
#define CRYPTOCFG_45_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_23_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_23_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_24 (DWC_ufshc_block_BaseAddress + 0x26e0)
#define CRYPTOCFG_45_24_RegisterSize 32
#define CRYPTOCFG_45_24_RegisterResetValue 0x0
#define CRYPTOCFG_45_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_24_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_24_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_25 (DWC_ufshc_block_BaseAddress + 0x26e4)
#define CRYPTOCFG_45_25_RegisterSize 32
#define CRYPTOCFG_45_25_RegisterResetValue 0x0
#define CRYPTOCFG_45_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_25_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_25_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_26 (DWC_ufshc_block_BaseAddress + 0x26e8)
#define CRYPTOCFG_45_26_RegisterSize 32
#define CRYPTOCFG_45_26_RegisterResetValue 0x0
#define CRYPTOCFG_45_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_26_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_26_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_27 (DWC_ufshc_block_BaseAddress + 0x26ec)
#define CRYPTOCFG_45_27_RegisterSize 32
#define CRYPTOCFG_45_27_RegisterResetValue 0x0
#define CRYPTOCFG_45_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_27_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_27_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_28 (DWC_ufshc_block_BaseAddress + 0x26f0)
#define CRYPTOCFG_45_28_RegisterSize 32
#define CRYPTOCFG_45_28_RegisterResetValue 0x0
#define CRYPTOCFG_45_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_28_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_28_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_29 (DWC_ufshc_block_BaseAddress + 0x26f4)
#define CRYPTOCFG_45_29_RegisterSize 32
#define CRYPTOCFG_45_29_RegisterResetValue 0x0
#define CRYPTOCFG_45_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_29_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_29_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_30 (DWC_ufshc_block_BaseAddress + 0x26f8)
#define CRYPTOCFG_45_30_RegisterSize 32
#define CRYPTOCFG_45_30_RegisterResetValue 0x0
#define CRYPTOCFG_45_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_30_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_30_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_45_31 (DWC_ufshc_block_BaseAddress + 0x26fc)
#define CRYPTOCFG_45_31_RegisterSize 32
#define CRYPTOCFG_45_31_RegisterResetValue 0x0
#define CRYPTOCFG_45_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_45_31_RESERVED_45_BitAddressOffset 0
#define CRYPTOCFG_45_31_RESERVED_45_RegisterSize 32





#define CRYPTOCFG_46_0 (DWC_ufshc_block_BaseAddress + 0x2700)
#define CRYPTOCFG_46_0_RegisterSize 32
#define CRYPTOCFG_46_0_RegisterResetValue 0x0
#define CRYPTOCFG_46_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_0_CRYPTOKEY_46_0_BitAddressOffset 0
#define CRYPTOCFG_46_0_CRYPTOKEY_46_0_RegisterSize 32





#define CRYPTOCFG_46_1 (DWC_ufshc_block_BaseAddress + 0x2704)
#define CRYPTOCFG_46_1_RegisterSize 32
#define CRYPTOCFG_46_1_RegisterResetValue 0x0
#define CRYPTOCFG_46_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_1_CRYPTOKEY_46_1_BitAddressOffset 0
#define CRYPTOCFG_46_1_CRYPTOKEY_46_1_RegisterSize 32





#define CRYPTOCFG_46_2 (DWC_ufshc_block_BaseAddress + 0x2708)
#define CRYPTOCFG_46_2_RegisterSize 32
#define CRYPTOCFG_46_2_RegisterResetValue 0x0
#define CRYPTOCFG_46_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_2_CRYPTOKEY_46_2_BitAddressOffset 0
#define CRYPTOCFG_46_2_CRYPTOKEY_46_2_RegisterSize 32





#define CRYPTOCFG_46_3 (DWC_ufshc_block_BaseAddress + 0x270c)
#define CRYPTOCFG_46_3_RegisterSize 32
#define CRYPTOCFG_46_3_RegisterResetValue 0x0
#define CRYPTOCFG_46_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_3_CRYPTOKEY_46_3_BitAddressOffset 0
#define CRYPTOCFG_46_3_CRYPTOKEY_46_3_RegisterSize 32





#define CRYPTOCFG_46_4 (DWC_ufshc_block_BaseAddress + 0x2710)
#define CRYPTOCFG_46_4_RegisterSize 32
#define CRYPTOCFG_46_4_RegisterResetValue 0x0
#define CRYPTOCFG_46_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_4_CRYPTOKEY_46_4_BitAddressOffset 0
#define CRYPTOCFG_46_4_CRYPTOKEY_46_4_RegisterSize 32





#define CRYPTOCFG_46_5 (DWC_ufshc_block_BaseAddress + 0x2714)
#define CRYPTOCFG_46_5_RegisterSize 32
#define CRYPTOCFG_46_5_RegisterResetValue 0x0
#define CRYPTOCFG_46_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_5_CRYPTOKEY_46_5_BitAddressOffset 0
#define CRYPTOCFG_46_5_CRYPTOKEY_46_5_RegisterSize 32





#define CRYPTOCFG_46_6 (DWC_ufshc_block_BaseAddress + 0x2718)
#define CRYPTOCFG_46_6_RegisterSize 32
#define CRYPTOCFG_46_6_RegisterResetValue 0x0
#define CRYPTOCFG_46_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_6_CRYPTOKEY_46_6_BitAddressOffset 0
#define CRYPTOCFG_46_6_CRYPTOKEY_46_6_RegisterSize 32





#define CRYPTOCFG_46_7 (DWC_ufshc_block_BaseAddress + 0x271c)
#define CRYPTOCFG_46_7_RegisterSize 32
#define CRYPTOCFG_46_7_RegisterResetValue 0x0
#define CRYPTOCFG_46_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_7_CRYPTOKEY_46_7_BitAddressOffset 0
#define CRYPTOCFG_46_7_CRYPTOKEY_46_7_RegisterSize 32





#define CRYPTOCFG_46_8 (DWC_ufshc_block_BaseAddress + 0x2720)
#define CRYPTOCFG_46_8_RegisterSize 32
#define CRYPTOCFG_46_8_RegisterResetValue 0x0
#define CRYPTOCFG_46_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_8_CRYPTOKEY_46_8_BitAddressOffset 0
#define CRYPTOCFG_46_8_CRYPTOKEY_46_8_RegisterSize 32





#define CRYPTOCFG_46_9 (DWC_ufshc_block_BaseAddress + 0x2724)
#define CRYPTOCFG_46_9_RegisterSize 32
#define CRYPTOCFG_46_9_RegisterResetValue 0x0
#define CRYPTOCFG_46_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_9_CRYPTOKEY_46_9_BitAddressOffset 0
#define CRYPTOCFG_46_9_CRYPTOKEY_46_9_RegisterSize 32





#define CRYPTOCFG_46_10 (DWC_ufshc_block_BaseAddress + 0x2728)
#define CRYPTOCFG_46_10_RegisterSize 32
#define CRYPTOCFG_46_10_RegisterResetValue 0x0
#define CRYPTOCFG_46_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_10_CRYPTOKEY_46_10_BitAddressOffset 0
#define CRYPTOCFG_46_10_CRYPTOKEY_46_10_RegisterSize 32





#define CRYPTOCFG_46_11 (DWC_ufshc_block_BaseAddress + 0x272c)
#define CRYPTOCFG_46_11_RegisterSize 32
#define CRYPTOCFG_46_11_RegisterResetValue 0x0
#define CRYPTOCFG_46_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_11_CRYPTOKEY_46_11_BitAddressOffset 0
#define CRYPTOCFG_46_11_CRYPTOKEY_46_11_RegisterSize 32





#define CRYPTOCFG_46_12 (DWC_ufshc_block_BaseAddress + 0x2730)
#define CRYPTOCFG_46_12_RegisterSize 32
#define CRYPTOCFG_46_12_RegisterResetValue 0x0
#define CRYPTOCFG_46_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_12_CRYPTOKEY_46_12_BitAddressOffset 0
#define CRYPTOCFG_46_12_CRYPTOKEY_46_12_RegisterSize 32





#define CRYPTOCFG_46_13 (DWC_ufshc_block_BaseAddress + 0x2734)
#define CRYPTOCFG_46_13_RegisterSize 32
#define CRYPTOCFG_46_13_RegisterResetValue 0x0
#define CRYPTOCFG_46_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_13_CRYPTOKEY_46_13_BitAddressOffset 0
#define CRYPTOCFG_46_13_CRYPTOKEY_46_13_RegisterSize 32





#define CRYPTOCFG_46_14 (DWC_ufshc_block_BaseAddress + 0x2738)
#define CRYPTOCFG_46_14_RegisterSize 32
#define CRYPTOCFG_46_14_RegisterResetValue 0x0
#define CRYPTOCFG_46_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_14_CRYPTOKEY_46_14_BitAddressOffset 0
#define CRYPTOCFG_46_14_CRYPTOKEY_46_14_RegisterSize 32





#define CRYPTOCFG_46_15 (DWC_ufshc_block_BaseAddress + 0x273c)
#define CRYPTOCFG_46_15_RegisterSize 32
#define CRYPTOCFG_46_15_RegisterResetValue 0x0
#define CRYPTOCFG_46_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_15_CRYPTOKEY_46_15_BitAddressOffset 0
#define CRYPTOCFG_46_15_CRYPTOKEY_46_15_RegisterSize 32





#define CRYPTOCFG_46_16 (DWC_ufshc_block_BaseAddress + 0x2740)
#define CRYPTOCFG_46_16_RegisterSize 32
#define CRYPTOCFG_46_16_RegisterResetValue 0x0
#define CRYPTOCFG_46_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_16_DUSIZE_46_BitAddressOffset 0
#define CRYPTOCFG_46_16_DUSIZE_46_RegisterSize 8



#define CRYPTOCFG_46_16_CAPIDX_46_BitAddressOffset 8
#define CRYPTOCFG_46_16_CAPIDX_46_RegisterSize 8



#define CRYPTOCFG_46_16_CFGE_46_BitAddressOffset 31
#define CRYPTOCFG_46_16_CFGE_46_RegisterSize 1





#define CRYPTOCFG_46_17 (DWC_ufshc_block_BaseAddress + 0x2744)
#define CRYPTOCFG_46_17_RegisterSize 32
#define CRYPTOCFG_46_17_RegisterResetValue 0x0
#define CRYPTOCFG_46_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_17_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_17_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_18 (DWC_ufshc_block_BaseAddress + 0x2748)
#define CRYPTOCFG_46_18_RegisterSize 32
#define CRYPTOCFG_46_18_RegisterResetValue 0x0
#define CRYPTOCFG_46_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_18_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_18_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_19 (DWC_ufshc_block_BaseAddress + 0x274c)
#define CRYPTOCFG_46_19_RegisterSize 32
#define CRYPTOCFG_46_19_RegisterResetValue 0x0
#define CRYPTOCFG_46_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_19_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_19_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_20 (DWC_ufshc_block_BaseAddress + 0x2750)
#define CRYPTOCFG_46_20_RegisterSize 32
#define CRYPTOCFG_46_20_RegisterResetValue 0x0
#define CRYPTOCFG_46_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_20_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_20_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_21 (DWC_ufshc_block_BaseAddress + 0x2754)
#define CRYPTOCFG_46_21_RegisterSize 32
#define CRYPTOCFG_46_21_RegisterResetValue 0x0
#define CRYPTOCFG_46_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_21_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_21_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_22 (DWC_ufshc_block_BaseAddress + 0x2758)
#define CRYPTOCFG_46_22_RegisterSize 32
#define CRYPTOCFG_46_22_RegisterResetValue 0x0
#define CRYPTOCFG_46_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_22_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_22_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_23 (DWC_ufshc_block_BaseAddress + 0x275c)
#define CRYPTOCFG_46_23_RegisterSize 32
#define CRYPTOCFG_46_23_RegisterResetValue 0x0
#define CRYPTOCFG_46_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_23_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_23_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_24 (DWC_ufshc_block_BaseAddress + 0x2760)
#define CRYPTOCFG_46_24_RegisterSize 32
#define CRYPTOCFG_46_24_RegisterResetValue 0x0
#define CRYPTOCFG_46_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_24_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_24_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_25 (DWC_ufshc_block_BaseAddress + 0x2764)
#define CRYPTOCFG_46_25_RegisterSize 32
#define CRYPTOCFG_46_25_RegisterResetValue 0x0
#define CRYPTOCFG_46_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_25_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_25_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_26 (DWC_ufshc_block_BaseAddress + 0x2768)
#define CRYPTOCFG_46_26_RegisterSize 32
#define CRYPTOCFG_46_26_RegisterResetValue 0x0
#define CRYPTOCFG_46_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_26_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_26_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_27 (DWC_ufshc_block_BaseAddress + 0x276c)
#define CRYPTOCFG_46_27_RegisterSize 32
#define CRYPTOCFG_46_27_RegisterResetValue 0x0
#define CRYPTOCFG_46_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_27_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_27_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_28 (DWC_ufshc_block_BaseAddress + 0x2770)
#define CRYPTOCFG_46_28_RegisterSize 32
#define CRYPTOCFG_46_28_RegisterResetValue 0x0
#define CRYPTOCFG_46_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_28_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_28_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_29 (DWC_ufshc_block_BaseAddress + 0x2774)
#define CRYPTOCFG_46_29_RegisterSize 32
#define CRYPTOCFG_46_29_RegisterResetValue 0x0
#define CRYPTOCFG_46_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_29_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_29_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_30 (DWC_ufshc_block_BaseAddress + 0x2778)
#define CRYPTOCFG_46_30_RegisterSize 32
#define CRYPTOCFG_46_30_RegisterResetValue 0x0
#define CRYPTOCFG_46_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_30_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_30_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_46_31 (DWC_ufshc_block_BaseAddress + 0x277c)
#define CRYPTOCFG_46_31_RegisterSize 32
#define CRYPTOCFG_46_31_RegisterResetValue 0x0
#define CRYPTOCFG_46_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_46_31_RESERVED_46_BitAddressOffset 0
#define CRYPTOCFG_46_31_RESERVED_46_RegisterSize 32





#define CRYPTOCFG_47_0 (DWC_ufshc_block_BaseAddress + 0x2780)
#define CRYPTOCFG_47_0_RegisterSize 32
#define CRYPTOCFG_47_0_RegisterResetValue 0x0
#define CRYPTOCFG_47_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_0_CRYPTOKEY_47_0_BitAddressOffset 0
#define CRYPTOCFG_47_0_CRYPTOKEY_47_0_RegisterSize 32





#define CRYPTOCFG_47_1 (DWC_ufshc_block_BaseAddress + 0x2784)
#define CRYPTOCFG_47_1_RegisterSize 32
#define CRYPTOCFG_47_1_RegisterResetValue 0x0
#define CRYPTOCFG_47_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_1_CRYPTOKEY_47_1_BitAddressOffset 0
#define CRYPTOCFG_47_1_CRYPTOKEY_47_1_RegisterSize 32





#define CRYPTOCFG_47_2 (DWC_ufshc_block_BaseAddress + 0x2788)
#define CRYPTOCFG_47_2_RegisterSize 32
#define CRYPTOCFG_47_2_RegisterResetValue 0x0
#define CRYPTOCFG_47_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_2_CRYPTOKEY_47_2_BitAddressOffset 0
#define CRYPTOCFG_47_2_CRYPTOKEY_47_2_RegisterSize 32





#define CRYPTOCFG_47_3 (DWC_ufshc_block_BaseAddress + 0x278c)
#define CRYPTOCFG_47_3_RegisterSize 32
#define CRYPTOCFG_47_3_RegisterResetValue 0x0
#define CRYPTOCFG_47_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_3_CRYPTOKEY_47_3_BitAddressOffset 0
#define CRYPTOCFG_47_3_CRYPTOKEY_47_3_RegisterSize 32





#define CRYPTOCFG_47_4 (DWC_ufshc_block_BaseAddress + 0x2790)
#define CRYPTOCFG_47_4_RegisterSize 32
#define CRYPTOCFG_47_4_RegisterResetValue 0x0
#define CRYPTOCFG_47_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_4_CRYPTOKEY_47_4_BitAddressOffset 0
#define CRYPTOCFG_47_4_CRYPTOKEY_47_4_RegisterSize 32





#define CRYPTOCFG_47_5 (DWC_ufshc_block_BaseAddress + 0x2794)
#define CRYPTOCFG_47_5_RegisterSize 32
#define CRYPTOCFG_47_5_RegisterResetValue 0x0
#define CRYPTOCFG_47_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_5_CRYPTOKEY_47_5_BitAddressOffset 0
#define CRYPTOCFG_47_5_CRYPTOKEY_47_5_RegisterSize 32





#define CRYPTOCFG_47_6 (DWC_ufshc_block_BaseAddress + 0x2798)
#define CRYPTOCFG_47_6_RegisterSize 32
#define CRYPTOCFG_47_6_RegisterResetValue 0x0
#define CRYPTOCFG_47_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_6_CRYPTOKEY_47_6_BitAddressOffset 0
#define CRYPTOCFG_47_6_CRYPTOKEY_47_6_RegisterSize 32





#define CRYPTOCFG_47_7 (DWC_ufshc_block_BaseAddress + 0x279c)
#define CRYPTOCFG_47_7_RegisterSize 32
#define CRYPTOCFG_47_7_RegisterResetValue 0x0
#define CRYPTOCFG_47_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_7_CRYPTOKEY_47_7_BitAddressOffset 0
#define CRYPTOCFG_47_7_CRYPTOKEY_47_7_RegisterSize 32





#define CRYPTOCFG_47_8 (DWC_ufshc_block_BaseAddress + 0x27a0)
#define CRYPTOCFG_47_8_RegisterSize 32
#define CRYPTOCFG_47_8_RegisterResetValue 0x0
#define CRYPTOCFG_47_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_8_CRYPTOKEY_47_8_BitAddressOffset 0
#define CRYPTOCFG_47_8_CRYPTOKEY_47_8_RegisterSize 32





#define CRYPTOCFG_47_9 (DWC_ufshc_block_BaseAddress + 0x27a4)
#define CRYPTOCFG_47_9_RegisterSize 32
#define CRYPTOCFG_47_9_RegisterResetValue 0x0
#define CRYPTOCFG_47_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_9_CRYPTOKEY_47_9_BitAddressOffset 0
#define CRYPTOCFG_47_9_CRYPTOKEY_47_9_RegisterSize 32





#define CRYPTOCFG_47_10 (DWC_ufshc_block_BaseAddress + 0x27a8)
#define CRYPTOCFG_47_10_RegisterSize 32
#define CRYPTOCFG_47_10_RegisterResetValue 0x0
#define CRYPTOCFG_47_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_10_CRYPTOKEY_47_10_BitAddressOffset 0
#define CRYPTOCFG_47_10_CRYPTOKEY_47_10_RegisterSize 32





#define CRYPTOCFG_47_11 (DWC_ufshc_block_BaseAddress + 0x27ac)
#define CRYPTOCFG_47_11_RegisterSize 32
#define CRYPTOCFG_47_11_RegisterResetValue 0x0
#define CRYPTOCFG_47_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_11_CRYPTOKEY_47_11_BitAddressOffset 0
#define CRYPTOCFG_47_11_CRYPTOKEY_47_11_RegisterSize 32





#define CRYPTOCFG_47_12 (DWC_ufshc_block_BaseAddress + 0x27b0)
#define CRYPTOCFG_47_12_RegisterSize 32
#define CRYPTOCFG_47_12_RegisterResetValue 0x0
#define CRYPTOCFG_47_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_12_CRYPTOKEY_47_12_BitAddressOffset 0
#define CRYPTOCFG_47_12_CRYPTOKEY_47_12_RegisterSize 32





#define CRYPTOCFG_47_13 (DWC_ufshc_block_BaseAddress + 0x27b4)
#define CRYPTOCFG_47_13_RegisterSize 32
#define CRYPTOCFG_47_13_RegisterResetValue 0x0
#define CRYPTOCFG_47_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_13_CRYPTOKEY_47_13_BitAddressOffset 0
#define CRYPTOCFG_47_13_CRYPTOKEY_47_13_RegisterSize 32





#define CRYPTOCFG_47_14 (DWC_ufshc_block_BaseAddress + 0x27b8)
#define CRYPTOCFG_47_14_RegisterSize 32
#define CRYPTOCFG_47_14_RegisterResetValue 0x0
#define CRYPTOCFG_47_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_14_CRYPTOKEY_47_14_BitAddressOffset 0
#define CRYPTOCFG_47_14_CRYPTOKEY_47_14_RegisterSize 32





#define CRYPTOCFG_47_15 (DWC_ufshc_block_BaseAddress + 0x27bc)
#define CRYPTOCFG_47_15_RegisterSize 32
#define CRYPTOCFG_47_15_RegisterResetValue 0x0
#define CRYPTOCFG_47_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_15_CRYPTOKEY_47_15_BitAddressOffset 0
#define CRYPTOCFG_47_15_CRYPTOKEY_47_15_RegisterSize 32





#define CRYPTOCFG_47_16 (DWC_ufshc_block_BaseAddress + 0x27c0)
#define CRYPTOCFG_47_16_RegisterSize 32
#define CRYPTOCFG_47_16_RegisterResetValue 0x0
#define CRYPTOCFG_47_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_16_DUSIZE_47_BitAddressOffset 0
#define CRYPTOCFG_47_16_DUSIZE_47_RegisterSize 8



#define CRYPTOCFG_47_16_CAPIDX_47_BitAddressOffset 8
#define CRYPTOCFG_47_16_CAPIDX_47_RegisterSize 8



#define CRYPTOCFG_47_16_CFGE_47_BitAddressOffset 31
#define CRYPTOCFG_47_16_CFGE_47_RegisterSize 1





#define CRYPTOCFG_47_17 (DWC_ufshc_block_BaseAddress + 0x27c4)
#define CRYPTOCFG_47_17_RegisterSize 32
#define CRYPTOCFG_47_17_RegisterResetValue 0x0
#define CRYPTOCFG_47_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_17_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_17_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_18 (DWC_ufshc_block_BaseAddress + 0x27c8)
#define CRYPTOCFG_47_18_RegisterSize 32
#define CRYPTOCFG_47_18_RegisterResetValue 0x0
#define CRYPTOCFG_47_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_18_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_18_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_19 (DWC_ufshc_block_BaseAddress + 0x27cc)
#define CRYPTOCFG_47_19_RegisterSize 32
#define CRYPTOCFG_47_19_RegisterResetValue 0x0
#define CRYPTOCFG_47_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_19_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_19_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_20 (DWC_ufshc_block_BaseAddress + 0x27d0)
#define CRYPTOCFG_47_20_RegisterSize 32
#define CRYPTOCFG_47_20_RegisterResetValue 0x0
#define CRYPTOCFG_47_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_20_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_20_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_21 (DWC_ufshc_block_BaseAddress + 0x27d4)
#define CRYPTOCFG_47_21_RegisterSize 32
#define CRYPTOCFG_47_21_RegisterResetValue 0x0
#define CRYPTOCFG_47_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_21_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_21_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_22 (DWC_ufshc_block_BaseAddress + 0x27d8)
#define CRYPTOCFG_47_22_RegisterSize 32
#define CRYPTOCFG_47_22_RegisterResetValue 0x0
#define CRYPTOCFG_47_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_22_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_22_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_23 (DWC_ufshc_block_BaseAddress + 0x27dc)
#define CRYPTOCFG_47_23_RegisterSize 32
#define CRYPTOCFG_47_23_RegisterResetValue 0x0
#define CRYPTOCFG_47_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_23_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_23_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_24 (DWC_ufshc_block_BaseAddress + 0x27e0)
#define CRYPTOCFG_47_24_RegisterSize 32
#define CRYPTOCFG_47_24_RegisterResetValue 0x0
#define CRYPTOCFG_47_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_24_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_24_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_25 (DWC_ufshc_block_BaseAddress + 0x27e4)
#define CRYPTOCFG_47_25_RegisterSize 32
#define CRYPTOCFG_47_25_RegisterResetValue 0x0
#define CRYPTOCFG_47_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_25_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_25_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_26 (DWC_ufshc_block_BaseAddress + 0x27e8)
#define CRYPTOCFG_47_26_RegisterSize 32
#define CRYPTOCFG_47_26_RegisterResetValue 0x0
#define CRYPTOCFG_47_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_26_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_26_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_27 (DWC_ufshc_block_BaseAddress + 0x27ec)
#define CRYPTOCFG_47_27_RegisterSize 32
#define CRYPTOCFG_47_27_RegisterResetValue 0x0
#define CRYPTOCFG_47_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_27_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_27_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_28 (DWC_ufshc_block_BaseAddress + 0x27f0)
#define CRYPTOCFG_47_28_RegisterSize 32
#define CRYPTOCFG_47_28_RegisterResetValue 0x0
#define CRYPTOCFG_47_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_28_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_28_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_29 (DWC_ufshc_block_BaseAddress + 0x27f4)
#define CRYPTOCFG_47_29_RegisterSize 32
#define CRYPTOCFG_47_29_RegisterResetValue 0x0
#define CRYPTOCFG_47_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_29_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_29_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_30 (DWC_ufshc_block_BaseAddress + 0x27f8)
#define CRYPTOCFG_47_30_RegisterSize 32
#define CRYPTOCFG_47_30_RegisterResetValue 0x0
#define CRYPTOCFG_47_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_30_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_30_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_47_31 (DWC_ufshc_block_BaseAddress + 0x27fc)
#define CRYPTOCFG_47_31_RegisterSize 32
#define CRYPTOCFG_47_31_RegisterResetValue 0x0
#define CRYPTOCFG_47_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_47_31_RESERVED_47_BitAddressOffset 0
#define CRYPTOCFG_47_31_RESERVED_47_RegisterSize 32





#define CRYPTOCFG_48_0 (DWC_ufshc_block_BaseAddress + 0x2800)
#define CRYPTOCFG_48_0_RegisterSize 32
#define CRYPTOCFG_48_0_RegisterResetValue 0x0
#define CRYPTOCFG_48_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_0_CRYPTOKEY_48_0_BitAddressOffset 0
#define CRYPTOCFG_48_0_CRYPTOKEY_48_0_RegisterSize 32





#define CRYPTOCFG_48_1 (DWC_ufshc_block_BaseAddress + 0x2804)
#define CRYPTOCFG_48_1_RegisterSize 32
#define CRYPTOCFG_48_1_RegisterResetValue 0x0
#define CRYPTOCFG_48_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_1_CRYPTOKEY_48_1_BitAddressOffset 0
#define CRYPTOCFG_48_1_CRYPTOKEY_48_1_RegisterSize 32





#define CRYPTOCFG_48_2 (DWC_ufshc_block_BaseAddress + 0x2808)
#define CRYPTOCFG_48_2_RegisterSize 32
#define CRYPTOCFG_48_2_RegisterResetValue 0x0
#define CRYPTOCFG_48_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_2_CRYPTOKEY_48_2_BitAddressOffset 0
#define CRYPTOCFG_48_2_CRYPTOKEY_48_2_RegisterSize 32





#define CRYPTOCFG_48_3 (DWC_ufshc_block_BaseAddress + 0x280c)
#define CRYPTOCFG_48_3_RegisterSize 32
#define CRYPTOCFG_48_3_RegisterResetValue 0x0
#define CRYPTOCFG_48_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_3_CRYPTOKEY_48_3_BitAddressOffset 0
#define CRYPTOCFG_48_3_CRYPTOKEY_48_3_RegisterSize 32





#define CRYPTOCFG_48_4 (DWC_ufshc_block_BaseAddress + 0x2810)
#define CRYPTOCFG_48_4_RegisterSize 32
#define CRYPTOCFG_48_4_RegisterResetValue 0x0
#define CRYPTOCFG_48_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_4_CRYPTOKEY_48_4_BitAddressOffset 0
#define CRYPTOCFG_48_4_CRYPTOKEY_48_4_RegisterSize 32





#define CRYPTOCFG_48_5 (DWC_ufshc_block_BaseAddress + 0x2814)
#define CRYPTOCFG_48_5_RegisterSize 32
#define CRYPTOCFG_48_5_RegisterResetValue 0x0
#define CRYPTOCFG_48_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_5_CRYPTOKEY_48_5_BitAddressOffset 0
#define CRYPTOCFG_48_5_CRYPTOKEY_48_5_RegisterSize 32





#define CRYPTOCFG_48_6 (DWC_ufshc_block_BaseAddress + 0x2818)
#define CRYPTOCFG_48_6_RegisterSize 32
#define CRYPTOCFG_48_6_RegisterResetValue 0x0
#define CRYPTOCFG_48_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_6_CRYPTOKEY_48_6_BitAddressOffset 0
#define CRYPTOCFG_48_6_CRYPTOKEY_48_6_RegisterSize 32





#define CRYPTOCFG_48_7 (DWC_ufshc_block_BaseAddress + 0x281c)
#define CRYPTOCFG_48_7_RegisterSize 32
#define CRYPTOCFG_48_7_RegisterResetValue 0x0
#define CRYPTOCFG_48_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_7_CRYPTOKEY_48_7_BitAddressOffset 0
#define CRYPTOCFG_48_7_CRYPTOKEY_48_7_RegisterSize 32





#define CRYPTOCFG_48_8 (DWC_ufshc_block_BaseAddress + 0x2820)
#define CRYPTOCFG_48_8_RegisterSize 32
#define CRYPTOCFG_48_8_RegisterResetValue 0x0
#define CRYPTOCFG_48_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_8_CRYPTOKEY_48_8_BitAddressOffset 0
#define CRYPTOCFG_48_8_CRYPTOKEY_48_8_RegisterSize 32





#define CRYPTOCFG_48_9 (DWC_ufshc_block_BaseAddress + 0x2824)
#define CRYPTOCFG_48_9_RegisterSize 32
#define CRYPTOCFG_48_9_RegisterResetValue 0x0
#define CRYPTOCFG_48_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_9_CRYPTOKEY_48_9_BitAddressOffset 0
#define CRYPTOCFG_48_9_CRYPTOKEY_48_9_RegisterSize 32





#define CRYPTOCFG_48_10 (DWC_ufshc_block_BaseAddress + 0x2828)
#define CRYPTOCFG_48_10_RegisterSize 32
#define CRYPTOCFG_48_10_RegisterResetValue 0x0
#define CRYPTOCFG_48_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_10_CRYPTOKEY_48_10_BitAddressOffset 0
#define CRYPTOCFG_48_10_CRYPTOKEY_48_10_RegisterSize 32





#define CRYPTOCFG_48_11 (DWC_ufshc_block_BaseAddress + 0x282c)
#define CRYPTOCFG_48_11_RegisterSize 32
#define CRYPTOCFG_48_11_RegisterResetValue 0x0
#define CRYPTOCFG_48_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_11_CRYPTOKEY_48_11_BitAddressOffset 0
#define CRYPTOCFG_48_11_CRYPTOKEY_48_11_RegisterSize 32





#define CRYPTOCFG_48_12 (DWC_ufshc_block_BaseAddress + 0x2830)
#define CRYPTOCFG_48_12_RegisterSize 32
#define CRYPTOCFG_48_12_RegisterResetValue 0x0
#define CRYPTOCFG_48_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_12_CRYPTOKEY_48_12_BitAddressOffset 0
#define CRYPTOCFG_48_12_CRYPTOKEY_48_12_RegisterSize 32





#define CRYPTOCFG_48_13 (DWC_ufshc_block_BaseAddress + 0x2834)
#define CRYPTOCFG_48_13_RegisterSize 32
#define CRYPTOCFG_48_13_RegisterResetValue 0x0
#define CRYPTOCFG_48_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_13_CRYPTOKEY_48_13_BitAddressOffset 0
#define CRYPTOCFG_48_13_CRYPTOKEY_48_13_RegisterSize 32





#define CRYPTOCFG_48_14 (DWC_ufshc_block_BaseAddress + 0x2838)
#define CRYPTOCFG_48_14_RegisterSize 32
#define CRYPTOCFG_48_14_RegisterResetValue 0x0
#define CRYPTOCFG_48_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_14_CRYPTOKEY_48_14_BitAddressOffset 0
#define CRYPTOCFG_48_14_CRYPTOKEY_48_14_RegisterSize 32





#define CRYPTOCFG_48_15 (DWC_ufshc_block_BaseAddress + 0x283c)
#define CRYPTOCFG_48_15_RegisterSize 32
#define CRYPTOCFG_48_15_RegisterResetValue 0x0
#define CRYPTOCFG_48_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_15_CRYPTOKEY_48_15_BitAddressOffset 0
#define CRYPTOCFG_48_15_CRYPTOKEY_48_15_RegisterSize 32





#define CRYPTOCFG_48_16 (DWC_ufshc_block_BaseAddress + 0x2840)
#define CRYPTOCFG_48_16_RegisterSize 32
#define CRYPTOCFG_48_16_RegisterResetValue 0x0
#define CRYPTOCFG_48_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_16_DUSIZE_48_BitAddressOffset 0
#define CRYPTOCFG_48_16_DUSIZE_48_RegisterSize 8



#define CRYPTOCFG_48_16_CAPIDX_48_BitAddressOffset 8
#define CRYPTOCFG_48_16_CAPIDX_48_RegisterSize 8



#define CRYPTOCFG_48_16_CFGE_48_BitAddressOffset 31
#define CRYPTOCFG_48_16_CFGE_48_RegisterSize 1





#define CRYPTOCFG_48_17 (DWC_ufshc_block_BaseAddress + 0x2844)
#define CRYPTOCFG_48_17_RegisterSize 32
#define CRYPTOCFG_48_17_RegisterResetValue 0x0
#define CRYPTOCFG_48_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_17_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_17_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_18 (DWC_ufshc_block_BaseAddress + 0x2848)
#define CRYPTOCFG_48_18_RegisterSize 32
#define CRYPTOCFG_48_18_RegisterResetValue 0x0
#define CRYPTOCFG_48_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_18_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_18_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_19 (DWC_ufshc_block_BaseAddress + 0x284c)
#define CRYPTOCFG_48_19_RegisterSize 32
#define CRYPTOCFG_48_19_RegisterResetValue 0x0
#define CRYPTOCFG_48_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_19_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_19_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_20 (DWC_ufshc_block_BaseAddress + 0x2850)
#define CRYPTOCFG_48_20_RegisterSize 32
#define CRYPTOCFG_48_20_RegisterResetValue 0x0
#define CRYPTOCFG_48_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_20_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_20_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_21 (DWC_ufshc_block_BaseAddress + 0x2854)
#define CRYPTOCFG_48_21_RegisterSize 32
#define CRYPTOCFG_48_21_RegisterResetValue 0x0
#define CRYPTOCFG_48_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_21_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_21_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_22 (DWC_ufshc_block_BaseAddress + 0x2858)
#define CRYPTOCFG_48_22_RegisterSize 32
#define CRYPTOCFG_48_22_RegisterResetValue 0x0
#define CRYPTOCFG_48_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_22_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_22_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_23 (DWC_ufshc_block_BaseAddress + 0x285c)
#define CRYPTOCFG_48_23_RegisterSize 32
#define CRYPTOCFG_48_23_RegisterResetValue 0x0
#define CRYPTOCFG_48_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_23_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_23_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_24 (DWC_ufshc_block_BaseAddress + 0x2860)
#define CRYPTOCFG_48_24_RegisterSize 32
#define CRYPTOCFG_48_24_RegisterResetValue 0x0
#define CRYPTOCFG_48_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_24_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_24_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_25 (DWC_ufshc_block_BaseAddress + 0x2864)
#define CRYPTOCFG_48_25_RegisterSize 32
#define CRYPTOCFG_48_25_RegisterResetValue 0x0
#define CRYPTOCFG_48_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_25_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_25_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_26 (DWC_ufshc_block_BaseAddress + 0x2868)
#define CRYPTOCFG_48_26_RegisterSize 32
#define CRYPTOCFG_48_26_RegisterResetValue 0x0
#define CRYPTOCFG_48_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_26_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_26_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_27 (DWC_ufshc_block_BaseAddress + 0x286c)
#define CRYPTOCFG_48_27_RegisterSize 32
#define CRYPTOCFG_48_27_RegisterResetValue 0x0
#define CRYPTOCFG_48_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_27_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_27_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_28 (DWC_ufshc_block_BaseAddress + 0x2870)
#define CRYPTOCFG_48_28_RegisterSize 32
#define CRYPTOCFG_48_28_RegisterResetValue 0x0
#define CRYPTOCFG_48_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_28_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_28_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_29 (DWC_ufshc_block_BaseAddress + 0x2874)
#define CRYPTOCFG_48_29_RegisterSize 32
#define CRYPTOCFG_48_29_RegisterResetValue 0x0
#define CRYPTOCFG_48_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_29_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_29_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_30 (DWC_ufshc_block_BaseAddress + 0x2878)
#define CRYPTOCFG_48_30_RegisterSize 32
#define CRYPTOCFG_48_30_RegisterResetValue 0x0
#define CRYPTOCFG_48_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_30_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_30_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_48_31 (DWC_ufshc_block_BaseAddress + 0x287c)
#define CRYPTOCFG_48_31_RegisterSize 32
#define CRYPTOCFG_48_31_RegisterResetValue 0x0
#define CRYPTOCFG_48_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_48_31_RESERVED_48_BitAddressOffset 0
#define CRYPTOCFG_48_31_RESERVED_48_RegisterSize 32





#define CRYPTOCFG_49_0 (DWC_ufshc_block_BaseAddress + 0x2880)
#define CRYPTOCFG_49_0_RegisterSize 32
#define CRYPTOCFG_49_0_RegisterResetValue 0x0
#define CRYPTOCFG_49_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_0_CRYPTOKEY_49_0_BitAddressOffset 0
#define CRYPTOCFG_49_0_CRYPTOKEY_49_0_RegisterSize 32





#define CRYPTOCFG_49_1 (DWC_ufshc_block_BaseAddress + 0x2884)
#define CRYPTOCFG_49_1_RegisterSize 32
#define CRYPTOCFG_49_1_RegisterResetValue 0x0
#define CRYPTOCFG_49_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_1_CRYPTOKEY_49_1_BitAddressOffset 0
#define CRYPTOCFG_49_1_CRYPTOKEY_49_1_RegisterSize 32





#define CRYPTOCFG_49_2 (DWC_ufshc_block_BaseAddress + 0x2888)
#define CRYPTOCFG_49_2_RegisterSize 32
#define CRYPTOCFG_49_2_RegisterResetValue 0x0
#define CRYPTOCFG_49_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_2_CRYPTOKEY_49_2_BitAddressOffset 0
#define CRYPTOCFG_49_2_CRYPTOKEY_49_2_RegisterSize 32





#define CRYPTOCFG_49_3 (DWC_ufshc_block_BaseAddress + 0x288c)
#define CRYPTOCFG_49_3_RegisterSize 32
#define CRYPTOCFG_49_3_RegisterResetValue 0x0
#define CRYPTOCFG_49_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_3_CRYPTOKEY_49_3_BitAddressOffset 0
#define CRYPTOCFG_49_3_CRYPTOKEY_49_3_RegisterSize 32





#define CRYPTOCFG_49_4 (DWC_ufshc_block_BaseAddress + 0x2890)
#define CRYPTOCFG_49_4_RegisterSize 32
#define CRYPTOCFG_49_4_RegisterResetValue 0x0
#define CRYPTOCFG_49_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_4_CRYPTOKEY_49_4_BitAddressOffset 0
#define CRYPTOCFG_49_4_CRYPTOKEY_49_4_RegisterSize 32





#define CRYPTOCFG_49_5 (DWC_ufshc_block_BaseAddress + 0x2894)
#define CRYPTOCFG_49_5_RegisterSize 32
#define CRYPTOCFG_49_5_RegisterResetValue 0x0
#define CRYPTOCFG_49_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_5_CRYPTOKEY_49_5_BitAddressOffset 0
#define CRYPTOCFG_49_5_CRYPTOKEY_49_5_RegisterSize 32





#define CRYPTOCFG_49_6 (DWC_ufshc_block_BaseAddress + 0x2898)
#define CRYPTOCFG_49_6_RegisterSize 32
#define CRYPTOCFG_49_6_RegisterResetValue 0x0
#define CRYPTOCFG_49_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_6_CRYPTOKEY_49_6_BitAddressOffset 0
#define CRYPTOCFG_49_6_CRYPTOKEY_49_6_RegisterSize 32





#define CRYPTOCFG_49_7 (DWC_ufshc_block_BaseAddress + 0x289c)
#define CRYPTOCFG_49_7_RegisterSize 32
#define CRYPTOCFG_49_7_RegisterResetValue 0x0
#define CRYPTOCFG_49_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_7_CRYPTOKEY_49_7_BitAddressOffset 0
#define CRYPTOCFG_49_7_CRYPTOKEY_49_7_RegisterSize 32





#define CRYPTOCFG_49_8 (DWC_ufshc_block_BaseAddress + 0x28a0)
#define CRYPTOCFG_49_8_RegisterSize 32
#define CRYPTOCFG_49_8_RegisterResetValue 0x0
#define CRYPTOCFG_49_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_8_CRYPTOKEY_49_8_BitAddressOffset 0
#define CRYPTOCFG_49_8_CRYPTOKEY_49_8_RegisterSize 32





#define CRYPTOCFG_49_9 (DWC_ufshc_block_BaseAddress + 0x28a4)
#define CRYPTOCFG_49_9_RegisterSize 32
#define CRYPTOCFG_49_9_RegisterResetValue 0x0
#define CRYPTOCFG_49_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_9_CRYPTOKEY_49_9_BitAddressOffset 0
#define CRYPTOCFG_49_9_CRYPTOKEY_49_9_RegisterSize 32





#define CRYPTOCFG_49_10 (DWC_ufshc_block_BaseAddress + 0x28a8)
#define CRYPTOCFG_49_10_RegisterSize 32
#define CRYPTOCFG_49_10_RegisterResetValue 0x0
#define CRYPTOCFG_49_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_10_CRYPTOKEY_49_10_BitAddressOffset 0
#define CRYPTOCFG_49_10_CRYPTOKEY_49_10_RegisterSize 32





#define CRYPTOCFG_49_11 (DWC_ufshc_block_BaseAddress + 0x28ac)
#define CRYPTOCFG_49_11_RegisterSize 32
#define CRYPTOCFG_49_11_RegisterResetValue 0x0
#define CRYPTOCFG_49_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_11_CRYPTOKEY_49_11_BitAddressOffset 0
#define CRYPTOCFG_49_11_CRYPTOKEY_49_11_RegisterSize 32





#define CRYPTOCFG_49_12 (DWC_ufshc_block_BaseAddress + 0x28b0)
#define CRYPTOCFG_49_12_RegisterSize 32
#define CRYPTOCFG_49_12_RegisterResetValue 0x0
#define CRYPTOCFG_49_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_12_CRYPTOKEY_49_12_BitAddressOffset 0
#define CRYPTOCFG_49_12_CRYPTOKEY_49_12_RegisterSize 32





#define CRYPTOCFG_49_13 (DWC_ufshc_block_BaseAddress + 0x28b4)
#define CRYPTOCFG_49_13_RegisterSize 32
#define CRYPTOCFG_49_13_RegisterResetValue 0x0
#define CRYPTOCFG_49_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_13_CRYPTOKEY_49_13_BitAddressOffset 0
#define CRYPTOCFG_49_13_CRYPTOKEY_49_13_RegisterSize 32





#define CRYPTOCFG_49_14 (DWC_ufshc_block_BaseAddress + 0x28b8)
#define CRYPTOCFG_49_14_RegisterSize 32
#define CRYPTOCFG_49_14_RegisterResetValue 0x0
#define CRYPTOCFG_49_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_14_CRYPTOKEY_49_14_BitAddressOffset 0
#define CRYPTOCFG_49_14_CRYPTOKEY_49_14_RegisterSize 32





#define CRYPTOCFG_49_15 (DWC_ufshc_block_BaseAddress + 0x28bc)
#define CRYPTOCFG_49_15_RegisterSize 32
#define CRYPTOCFG_49_15_RegisterResetValue 0x0
#define CRYPTOCFG_49_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_15_CRYPTOKEY_49_15_BitAddressOffset 0
#define CRYPTOCFG_49_15_CRYPTOKEY_49_15_RegisterSize 32





#define CRYPTOCFG_49_16 (DWC_ufshc_block_BaseAddress + 0x28c0)
#define CRYPTOCFG_49_16_RegisterSize 32
#define CRYPTOCFG_49_16_RegisterResetValue 0x0
#define CRYPTOCFG_49_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_16_DUSIZE_49_BitAddressOffset 0
#define CRYPTOCFG_49_16_DUSIZE_49_RegisterSize 8



#define CRYPTOCFG_49_16_CAPIDX_49_BitAddressOffset 8
#define CRYPTOCFG_49_16_CAPIDX_49_RegisterSize 8



#define CRYPTOCFG_49_16_CFGE_49_BitAddressOffset 31
#define CRYPTOCFG_49_16_CFGE_49_RegisterSize 1





#define CRYPTOCFG_49_17 (DWC_ufshc_block_BaseAddress + 0x28c4)
#define CRYPTOCFG_49_17_RegisterSize 32
#define CRYPTOCFG_49_17_RegisterResetValue 0x0
#define CRYPTOCFG_49_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_17_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_17_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_18 (DWC_ufshc_block_BaseAddress + 0x28c8)
#define CRYPTOCFG_49_18_RegisterSize 32
#define CRYPTOCFG_49_18_RegisterResetValue 0x0
#define CRYPTOCFG_49_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_18_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_18_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_19 (DWC_ufshc_block_BaseAddress + 0x28cc)
#define CRYPTOCFG_49_19_RegisterSize 32
#define CRYPTOCFG_49_19_RegisterResetValue 0x0
#define CRYPTOCFG_49_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_19_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_19_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_20 (DWC_ufshc_block_BaseAddress + 0x28d0)
#define CRYPTOCFG_49_20_RegisterSize 32
#define CRYPTOCFG_49_20_RegisterResetValue 0x0
#define CRYPTOCFG_49_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_20_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_20_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_21 (DWC_ufshc_block_BaseAddress + 0x28d4)
#define CRYPTOCFG_49_21_RegisterSize 32
#define CRYPTOCFG_49_21_RegisterResetValue 0x0
#define CRYPTOCFG_49_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_21_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_21_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_22 (DWC_ufshc_block_BaseAddress + 0x28d8)
#define CRYPTOCFG_49_22_RegisterSize 32
#define CRYPTOCFG_49_22_RegisterResetValue 0x0
#define CRYPTOCFG_49_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_22_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_22_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_23 (DWC_ufshc_block_BaseAddress + 0x28dc)
#define CRYPTOCFG_49_23_RegisterSize 32
#define CRYPTOCFG_49_23_RegisterResetValue 0x0
#define CRYPTOCFG_49_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_23_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_23_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_24 (DWC_ufshc_block_BaseAddress + 0x28e0)
#define CRYPTOCFG_49_24_RegisterSize 32
#define CRYPTOCFG_49_24_RegisterResetValue 0x0
#define CRYPTOCFG_49_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_24_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_24_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_25 (DWC_ufshc_block_BaseAddress + 0x28e4)
#define CRYPTOCFG_49_25_RegisterSize 32
#define CRYPTOCFG_49_25_RegisterResetValue 0x0
#define CRYPTOCFG_49_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_25_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_25_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_26 (DWC_ufshc_block_BaseAddress + 0x28e8)
#define CRYPTOCFG_49_26_RegisterSize 32
#define CRYPTOCFG_49_26_RegisterResetValue 0x0
#define CRYPTOCFG_49_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_26_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_26_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_27 (DWC_ufshc_block_BaseAddress + 0x28ec)
#define CRYPTOCFG_49_27_RegisterSize 32
#define CRYPTOCFG_49_27_RegisterResetValue 0x0
#define CRYPTOCFG_49_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_27_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_27_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_28 (DWC_ufshc_block_BaseAddress + 0x28f0)
#define CRYPTOCFG_49_28_RegisterSize 32
#define CRYPTOCFG_49_28_RegisterResetValue 0x0
#define CRYPTOCFG_49_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_28_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_28_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_29 (DWC_ufshc_block_BaseAddress + 0x28f4)
#define CRYPTOCFG_49_29_RegisterSize 32
#define CRYPTOCFG_49_29_RegisterResetValue 0x0
#define CRYPTOCFG_49_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_29_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_29_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_30 (DWC_ufshc_block_BaseAddress + 0x28f8)
#define CRYPTOCFG_49_30_RegisterSize 32
#define CRYPTOCFG_49_30_RegisterResetValue 0x0
#define CRYPTOCFG_49_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_30_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_30_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_49_31 (DWC_ufshc_block_BaseAddress + 0x28fc)
#define CRYPTOCFG_49_31_RegisterSize 32
#define CRYPTOCFG_49_31_RegisterResetValue 0x0
#define CRYPTOCFG_49_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_49_31_RESERVED_49_BitAddressOffset 0
#define CRYPTOCFG_49_31_RESERVED_49_RegisterSize 32





#define CRYPTOCFG_50_0 (DWC_ufshc_block_BaseAddress + 0x2900)
#define CRYPTOCFG_50_0_RegisterSize 32
#define CRYPTOCFG_50_0_RegisterResetValue 0x0
#define CRYPTOCFG_50_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_0_CRYPTOKEY_50_0_BitAddressOffset 0
#define CRYPTOCFG_50_0_CRYPTOKEY_50_0_RegisterSize 32





#define CRYPTOCFG_50_1 (DWC_ufshc_block_BaseAddress + 0x2904)
#define CRYPTOCFG_50_1_RegisterSize 32
#define CRYPTOCFG_50_1_RegisterResetValue 0x0
#define CRYPTOCFG_50_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_1_CRYPTOKEY_50_1_BitAddressOffset 0
#define CRYPTOCFG_50_1_CRYPTOKEY_50_1_RegisterSize 32





#define CRYPTOCFG_50_2 (DWC_ufshc_block_BaseAddress + 0x2908)
#define CRYPTOCFG_50_2_RegisterSize 32
#define CRYPTOCFG_50_2_RegisterResetValue 0x0
#define CRYPTOCFG_50_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_2_CRYPTOKEY_50_2_BitAddressOffset 0
#define CRYPTOCFG_50_2_CRYPTOKEY_50_2_RegisterSize 32





#define CRYPTOCFG_50_3 (DWC_ufshc_block_BaseAddress + 0x290c)
#define CRYPTOCFG_50_3_RegisterSize 32
#define CRYPTOCFG_50_3_RegisterResetValue 0x0
#define CRYPTOCFG_50_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_3_CRYPTOKEY_50_3_BitAddressOffset 0
#define CRYPTOCFG_50_3_CRYPTOKEY_50_3_RegisterSize 32





#define CRYPTOCFG_50_4 (DWC_ufshc_block_BaseAddress + 0x2910)
#define CRYPTOCFG_50_4_RegisterSize 32
#define CRYPTOCFG_50_4_RegisterResetValue 0x0
#define CRYPTOCFG_50_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_4_CRYPTOKEY_50_4_BitAddressOffset 0
#define CRYPTOCFG_50_4_CRYPTOKEY_50_4_RegisterSize 32





#define CRYPTOCFG_50_5 (DWC_ufshc_block_BaseAddress + 0x2914)
#define CRYPTOCFG_50_5_RegisterSize 32
#define CRYPTOCFG_50_5_RegisterResetValue 0x0
#define CRYPTOCFG_50_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_5_CRYPTOKEY_50_5_BitAddressOffset 0
#define CRYPTOCFG_50_5_CRYPTOKEY_50_5_RegisterSize 32





#define CRYPTOCFG_50_6 (DWC_ufshc_block_BaseAddress + 0x2918)
#define CRYPTOCFG_50_6_RegisterSize 32
#define CRYPTOCFG_50_6_RegisterResetValue 0x0
#define CRYPTOCFG_50_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_6_CRYPTOKEY_50_6_BitAddressOffset 0
#define CRYPTOCFG_50_6_CRYPTOKEY_50_6_RegisterSize 32





#define CRYPTOCFG_50_7 (DWC_ufshc_block_BaseAddress + 0x291c)
#define CRYPTOCFG_50_7_RegisterSize 32
#define CRYPTOCFG_50_7_RegisterResetValue 0x0
#define CRYPTOCFG_50_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_7_CRYPTOKEY_50_7_BitAddressOffset 0
#define CRYPTOCFG_50_7_CRYPTOKEY_50_7_RegisterSize 32





#define CRYPTOCFG_50_8 (DWC_ufshc_block_BaseAddress + 0x2920)
#define CRYPTOCFG_50_8_RegisterSize 32
#define CRYPTOCFG_50_8_RegisterResetValue 0x0
#define CRYPTOCFG_50_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_8_CRYPTOKEY_50_8_BitAddressOffset 0
#define CRYPTOCFG_50_8_CRYPTOKEY_50_8_RegisterSize 32





#define CRYPTOCFG_50_9 (DWC_ufshc_block_BaseAddress + 0x2924)
#define CRYPTOCFG_50_9_RegisterSize 32
#define CRYPTOCFG_50_9_RegisterResetValue 0x0
#define CRYPTOCFG_50_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_9_CRYPTOKEY_50_9_BitAddressOffset 0
#define CRYPTOCFG_50_9_CRYPTOKEY_50_9_RegisterSize 32





#define CRYPTOCFG_50_10 (DWC_ufshc_block_BaseAddress + 0x2928)
#define CRYPTOCFG_50_10_RegisterSize 32
#define CRYPTOCFG_50_10_RegisterResetValue 0x0
#define CRYPTOCFG_50_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_10_CRYPTOKEY_50_10_BitAddressOffset 0
#define CRYPTOCFG_50_10_CRYPTOKEY_50_10_RegisterSize 32





#define CRYPTOCFG_50_11 (DWC_ufshc_block_BaseAddress + 0x292c)
#define CRYPTOCFG_50_11_RegisterSize 32
#define CRYPTOCFG_50_11_RegisterResetValue 0x0
#define CRYPTOCFG_50_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_11_CRYPTOKEY_50_11_BitAddressOffset 0
#define CRYPTOCFG_50_11_CRYPTOKEY_50_11_RegisterSize 32





#define CRYPTOCFG_50_12 (DWC_ufshc_block_BaseAddress + 0x2930)
#define CRYPTOCFG_50_12_RegisterSize 32
#define CRYPTOCFG_50_12_RegisterResetValue 0x0
#define CRYPTOCFG_50_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_12_CRYPTOKEY_50_12_BitAddressOffset 0
#define CRYPTOCFG_50_12_CRYPTOKEY_50_12_RegisterSize 32





#define CRYPTOCFG_50_13 (DWC_ufshc_block_BaseAddress + 0x2934)
#define CRYPTOCFG_50_13_RegisterSize 32
#define CRYPTOCFG_50_13_RegisterResetValue 0x0
#define CRYPTOCFG_50_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_13_CRYPTOKEY_50_13_BitAddressOffset 0
#define CRYPTOCFG_50_13_CRYPTOKEY_50_13_RegisterSize 32





#define CRYPTOCFG_50_14 (DWC_ufshc_block_BaseAddress + 0x2938)
#define CRYPTOCFG_50_14_RegisterSize 32
#define CRYPTOCFG_50_14_RegisterResetValue 0x0
#define CRYPTOCFG_50_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_14_CRYPTOKEY_50_14_BitAddressOffset 0
#define CRYPTOCFG_50_14_CRYPTOKEY_50_14_RegisterSize 32





#define CRYPTOCFG_50_15 (DWC_ufshc_block_BaseAddress + 0x293c)
#define CRYPTOCFG_50_15_RegisterSize 32
#define CRYPTOCFG_50_15_RegisterResetValue 0x0
#define CRYPTOCFG_50_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_15_CRYPTOKEY_50_15_BitAddressOffset 0
#define CRYPTOCFG_50_15_CRYPTOKEY_50_15_RegisterSize 32





#define CRYPTOCFG_50_16 (DWC_ufshc_block_BaseAddress + 0x2940)
#define CRYPTOCFG_50_16_RegisterSize 32
#define CRYPTOCFG_50_16_RegisterResetValue 0x0
#define CRYPTOCFG_50_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_16_DUSIZE_50_BitAddressOffset 0
#define CRYPTOCFG_50_16_DUSIZE_50_RegisterSize 8



#define CRYPTOCFG_50_16_CAPIDX_50_BitAddressOffset 8
#define CRYPTOCFG_50_16_CAPIDX_50_RegisterSize 8



#define CRYPTOCFG_50_16_CFGE_50_BitAddressOffset 31
#define CRYPTOCFG_50_16_CFGE_50_RegisterSize 1





#define CRYPTOCFG_50_17 (DWC_ufshc_block_BaseAddress + 0x2944)
#define CRYPTOCFG_50_17_RegisterSize 32
#define CRYPTOCFG_50_17_RegisterResetValue 0x0
#define CRYPTOCFG_50_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_17_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_17_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_18 (DWC_ufshc_block_BaseAddress + 0x2948)
#define CRYPTOCFG_50_18_RegisterSize 32
#define CRYPTOCFG_50_18_RegisterResetValue 0x0
#define CRYPTOCFG_50_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_18_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_18_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_19 (DWC_ufshc_block_BaseAddress + 0x294c)
#define CRYPTOCFG_50_19_RegisterSize 32
#define CRYPTOCFG_50_19_RegisterResetValue 0x0
#define CRYPTOCFG_50_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_19_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_19_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_20 (DWC_ufshc_block_BaseAddress + 0x2950)
#define CRYPTOCFG_50_20_RegisterSize 32
#define CRYPTOCFG_50_20_RegisterResetValue 0x0
#define CRYPTOCFG_50_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_20_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_20_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_21 (DWC_ufshc_block_BaseAddress + 0x2954)
#define CRYPTOCFG_50_21_RegisterSize 32
#define CRYPTOCFG_50_21_RegisterResetValue 0x0
#define CRYPTOCFG_50_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_21_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_21_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_22 (DWC_ufshc_block_BaseAddress + 0x2958)
#define CRYPTOCFG_50_22_RegisterSize 32
#define CRYPTOCFG_50_22_RegisterResetValue 0x0
#define CRYPTOCFG_50_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_22_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_22_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_23 (DWC_ufshc_block_BaseAddress + 0x295c)
#define CRYPTOCFG_50_23_RegisterSize 32
#define CRYPTOCFG_50_23_RegisterResetValue 0x0
#define CRYPTOCFG_50_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_23_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_23_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_24 (DWC_ufshc_block_BaseAddress + 0x2960)
#define CRYPTOCFG_50_24_RegisterSize 32
#define CRYPTOCFG_50_24_RegisterResetValue 0x0
#define CRYPTOCFG_50_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_24_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_24_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_25 (DWC_ufshc_block_BaseAddress + 0x2964)
#define CRYPTOCFG_50_25_RegisterSize 32
#define CRYPTOCFG_50_25_RegisterResetValue 0x0
#define CRYPTOCFG_50_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_25_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_25_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_26 (DWC_ufshc_block_BaseAddress + 0x2968)
#define CRYPTOCFG_50_26_RegisterSize 32
#define CRYPTOCFG_50_26_RegisterResetValue 0x0
#define CRYPTOCFG_50_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_26_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_26_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_27 (DWC_ufshc_block_BaseAddress + 0x296c)
#define CRYPTOCFG_50_27_RegisterSize 32
#define CRYPTOCFG_50_27_RegisterResetValue 0x0
#define CRYPTOCFG_50_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_27_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_27_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_28 (DWC_ufshc_block_BaseAddress + 0x2970)
#define CRYPTOCFG_50_28_RegisterSize 32
#define CRYPTOCFG_50_28_RegisterResetValue 0x0
#define CRYPTOCFG_50_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_28_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_28_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_29 (DWC_ufshc_block_BaseAddress + 0x2974)
#define CRYPTOCFG_50_29_RegisterSize 32
#define CRYPTOCFG_50_29_RegisterResetValue 0x0
#define CRYPTOCFG_50_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_29_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_29_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_30 (DWC_ufshc_block_BaseAddress + 0x2978)
#define CRYPTOCFG_50_30_RegisterSize 32
#define CRYPTOCFG_50_30_RegisterResetValue 0x0
#define CRYPTOCFG_50_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_30_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_30_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_50_31 (DWC_ufshc_block_BaseAddress + 0x297c)
#define CRYPTOCFG_50_31_RegisterSize 32
#define CRYPTOCFG_50_31_RegisterResetValue 0x0
#define CRYPTOCFG_50_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_50_31_RESERVED_50_BitAddressOffset 0
#define CRYPTOCFG_50_31_RESERVED_50_RegisterSize 32





#define CRYPTOCFG_51_0 (DWC_ufshc_block_BaseAddress + 0x2980)
#define CRYPTOCFG_51_0_RegisterSize 32
#define CRYPTOCFG_51_0_RegisterResetValue 0x0
#define CRYPTOCFG_51_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_0_CRYPTOKEY_51_0_BitAddressOffset 0
#define CRYPTOCFG_51_0_CRYPTOKEY_51_0_RegisterSize 32





#define CRYPTOCFG_51_1 (DWC_ufshc_block_BaseAddress + 0x2984)
#define CRYPTOCFG_51_1_RegisterSize 32
#define CRYPTOCFG_51_1_RegisterResetValue 0x0
#define CRYPTOCFG_51_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_1_CRYPTOKEY_51_1_BitAddressOffset 0
#define CRYPTOCFG_51_1_CRYPTOKEY_51_1_RegisterSize 32





#define CRYPTOCFG_51_2 (DWC_ufshc_block_BaseAddress + 0x2988)
#define CRYPTOCFG_51_2_RegisterSize 32
#define CRYPTOCFG_51_2_RegisterResetValue 0x0
#define CRYPTOCFG_51_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_2_CRYPTOKEY_51_2_BitAddressOffset 0
#define CRYPTOCFG_51_2_CRYPTOKEY_51_2_RegisterSize 32





#define CRYPTOCFG_51_3 (DWC_ufshc_block_BaseAddress + 0x298c)
#define CRYPTOCFG_51_3_RegisterSize 32
#define CRYPTOCFG_51_3_RegisterResetValue 0x0
#define CRYPTOCFG_51_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_3_CRYPTOKEY_51_3_BitAddressOffset 0
#define CRYPTOCFG_51_3_CRYPTOKEY_51_3_RegisterSize 32





#define CRYPTOCFG_51_4 (DWC_ufshc_block_BaseAddress + 0x2990)
#define CRYPTOCFG_51_4_RegisterSize 32
#define CRYPTOCFG_51_4_RegisterResetValue 0x0
#define CRYPTOCFG_51_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_4_CRYPTOKEY_51_4_BitAddressOffset 0
#define CRYPTOCFG_51_4_CRYPTOKEY_51_4_RegisterSize 32





#define CRYPTOCFG_51_5 (DWC_ufshc_block_BaseAddress + 0x2994)
#define CRYPTOCFG_51_5_RegisterSize 32
#define CRYPTOCFG_51_5_RegisterResetValue 0x0
#define CRYPTOCFG_51_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_5_CRYPTOKEY_51_5_BitAddressOffset 0
#define CRYPTOCFG_51_5_CRYPTOKEY_51_5_RegisterSize 32





#define CRYPTOCFG_51_6 (DWC_ufshc_block_BaseAddress + 0x2998)
#define CRYPTOCFG_51_6_RegisterSize 32
#define CRYPTOCFG_51_6_RegisterResetValue 0x0
#define CRYPTOCFG_51_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_6_CRYPTOKEY_51_6_BitAddressOffset 0
#define CRYPTOCFG_51_6_CRYPTOKEY_51_6_RegisterSize 32





#define CRYPTOCFG_51_7 (DWC_ufshc_block_BaseAddress + 0x299c)
#define CRYPTOCFG_51_7_RegisterSize 32
#define CRYPTOCFG_51_7_RegisterResetValue 0x0
#define CRYPTOCFG_51_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_7_CRYPTOKEY_51_7_BitAddressOffset 0
#define CRYPTOCFG_51_7_CRYPTOKEY_51_7_RegisterSize 32





#define CRYPTOCFG_51_8 (DWC_ufshc_block_BaseAddress + 0x29a0)
#define CRYPTOCFG_51_8_RegisterSize 32
#define CRYPTOCFG_51_8_RegisterResetValue 0x0
#define CRYPTOCFG_51_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_8_CRYPTOKEY_51_8_BitAddressOffset 0
#define CRYPTOCFG_51_8_CRYPTOKEY_51_8_RegisterSize 32





#define CRYPTOCFG_51_9 (DWC_ufshc_block_BaseAddress + 0x29a4)
#define CRYPTOCFG_51_9_RegisterSize 32
#define CRYPTOCFG_51_9_RegisterResetValue 0x0
#define CRYPTOCFG_51_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_9_CRYPTOKEY_51_9_BitAddressOffset 0
#define CRYPTOCFG_51_9_CRYPTOKEY_51_9_RegisterSize 32





#define CRYPTOCFG_51_10 (DWC_ufshc_block_BaseAddress + 0x29a8)
#define CRYPTOCFG_51_10_RegisterSize 32
#define CRYPTOCFG_51_10_RegisterResetValue 0x0
#define CRYPTOCFG_51_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_10_CRYPTOKEY_51_10_BitAddressOffset 0
#define CRYPTOCFG_51_10_CRYPTOKEY_51_10_RegisterSize 32





#define CRYPTOCFG_51_11 (DWC_ufshc_block_BaseAddress + 0x29ac)
#define CRYPTOCFG_51_11_RegisterSize 32
#define CRYPTOCFG_51_11_RegisterResetValue 0x0
#define CRYPTOCFG_51_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_11_CRYPTOKEY_51_11_BitAddressOffset 0
#define CRYPTOCFG_51_11_CRYPTOKEY_51_11_RegisterSize 32





#define CRYPTOCFG_51_12 (DWC_ufshc_block_BaseAddress + 0x29b0)
#define CRYPTOCFG_51_12_RegisterSize 32
#define CRYPTOCFG_51_12_RegisterResetValue 0x0
#define CRYPTOCFG_51_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_12_CRYPTOKEY_51_12_BitAddressOffset 0
#define CRYPTOCFG_51_12_CRYPTOKEY_51_12_RegisterSize 32





#define CRYPTOCFG_51_13 (DWC_ufshc_block_BaseAddress + 0x29b4)
#define CRYPTOCFG_51_13_RegisterSize 32
#define CRYPTOCFG_51_13_RegisterResetValue 0x0
#define CRYPTOCFG_51_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_13_CRYPTOKEY_51_13_BitAddressOffset 0
#define CRYPTOCFG_51_13_CRYPTOKEY_51_13_RegisterSize 32





#define CRYPTOCFG_51_14 (DWC_ufshc_block_BaseAddress + 0x29b8)
#define CRYPTOCFG_51_14_RegisterSize 32
#define CRYPTOCFG_51_14_RegisterResetValue 0x0
#define CRYPTOCFG_51_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_14_CRYPTOKEY_51_14_BitAddressOffset 0
#define CRYPTOCFG_51_14_CRYPTOKEY_51_14_RegisterSize 32





#define CRYPTOCFG_51_15 (DWC_ufshc_block_BaseAddress + 0x29bc)
#define CRYPTOCFG_51_15_RegisterSize 32
#define CRYPTOCFG_51_15_RegisterResetValue 0x0
#define CRYPTOCFG_51_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_15_CRYPTOKEY_51_15_BitAddressOffset 0
#define CRYPTOCFG_51_15_CRYPTOKEY_51_15_RegisterSize 32





#define CRYPTOCFG_51_16 (DWC_ufshc_block_BaseAddress + 0x29c0)
#define CRYPTOCFG_51_16_RegisterSize 32
#define CRYPTOCFG_51_16_RegisterResetValue 0x0
#define CRYPTOCFG_51_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_16_DUSIZE_51_BitAddressOffset 0
#define CRYPTOCFG_51_16_DUSIZE_51_RegisterSize 8



#define CRYPTOCFG_51_16_CAPIDX_51_BitAddressOffset 8
#define CRYPTOCFG_51_16_CAPIDX_51_RegisterSize 8



#define CRYPTOCFG_51_16_CFGE_51_BitAddressOffset 31
#define CRYPTOCFG_51_16_CFGE_51_RegisterSize 1





#define CRYPTOCFG_51_17 (DWC_ufshc_block_BaseAddress + 0x29c4)
#define CRYPTOCFG_51_17_RegisterSize 32
#define CRYPTOCFG_51_17_RegisterResetValue 0x0
#define CRYPTOCFG_51_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_17_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_17_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_18 (DWC_ufshc_block_BaseAddress + 0x29c8)
#define CRYPTOCFG_51_18_RegisterSize 32
#define CRYPTOCFG_51_18_RegisterResetValue 0x0
#define CRYPTOCFG_51_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_18_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_18_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_19 (DWC_ufshc_block_BaseAddress + 0x29cc)
#define CRYPTOCFG_51_19_RegisterSize 32
#define CRYPTOCFG_51_19_RegisterResetValue 0x0
#define CRYPTOCFG_51_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_19_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_19_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_20 (DWC_ufshc_block_BaseAddress + 0x29d0)
#define CRYPTOCFG_51_20_RegisterSize 32
#define CRYPTOCFG_51_20_RegisterResetValue 0x0
#define CRYPTOCFG_51_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_20_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_20_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_21 (DWC_ufshc_block_BaseAddress + 0x29d4)
#define CRYPTOCFG_51_21_RegisterSize 32
#define CRYPTOCFG_51_21_RegisterResetValue 0x0
#define CRYPTOCFG_51_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_21_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_21_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_22 (DWC_ufshc_block_BaseAddress + 0x29d8)
#define CRYPTOCFG_51_22_RegisterSize 32
#define CRYPTOCFG_51_22_RegisterResetValue 0x0
#define CRYPTOCFG_51_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_22_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_22_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_23 (DWC_ufshc_block_BaseAddress + 0x29dc)
#define CRYPTOCFG_51_23_RegisterSize 32
#define CRYPTOCFG_51_23_RegisterResetValue 0x0
#define CRYPTOCFG_51_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_23_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_23_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_24 (DWC_ufshc_block_BaseAddress + 0x29e0)
#define CRYPTOCFG_51_24_RegisterSize 32
#define CRYPTOCFG_51_24_RegisterResetValue 0x0
#define CRYPTOCFG_51_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_24_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_24_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_25 (DWC_ufshc_block_BaseAddress + 0x29e4)
#define CRYPTOCFG_51_25_RegisterSize 32
#define CRYPTOCFG_51_25_RegisterResetValue 0x0
#define CRYPTOCFG_51_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_25_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_25_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_26 (DWC_ufshc_block_BaseAddress + 0x29e8)
#define CRYPTOCFG_51_26_RegisterSize 32
#define CRYPTOCFG_51_26_RegisterResetValue 0x0
#define CRYPTOCFG_51_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_26_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_26_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_27 (DWC_ufshc_block_BaseAddress + 0x29ec)
#define CRYPTOCFG_51_27_RegisterSize 32
#define CRYPTOCFG_51_27_RegisterResetValue 0x0
#define CRYPTOCFG_51_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_27_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_27_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_28 (DWC_ufshc_block_BaseAddress + 0x29f0)
#define CRYPTOCFG_51_28_RegisterSize 32
#define CRYPTOCFG_51_28_RegisterResetValue 0x0
#define CRYPTOCFG_51_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_28_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_28_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_29 (DWC_ufshc_block_BaseAddress + 0x29f4)
#define CRYPTOCFG_51_29_RegisterSize 32
#define CRYPTOCFG_51_29_RegisterResetValue 0x0
#define CRYPTOCFG_51_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_29_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_29_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_30 (DWC_ufshc_block_BaseAddress + 0x29f8)
#define CRYPTOCFG_51_30_RegisterSize 32
#define CRYPTOCFG_51_30_RegisterResetValue 0x0
#define CRYPTOCFG_51_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_30_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_30_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_51_31 (DWC_ufshc_block_BaseAddress + 0x29fc)
#define CRYPTOCFG_51_31_RegisterSize 32
#define CRYPTOCFG_51_31_RegisterResetValue 0x0
#define CRYPTOCFG_51_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_51_31_RESERVED_51_BitAddressOffset 0
#define CRYPTOCFG_51_31_RESERVED_51_RegisterSize 32





#define CRYPTOCFG_52_0 (DWC_ufshc_block_BaseAddress + 0x2a00)
#define CRYPTOCFG_52_0_RegisterSize 32
#define CRYPTOCFG_52_0_RegisterResetValue 0x0
#define CRYPTOCFG_52_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_0_CRYPTOKEY_52_0_BitAddressOffset 0
#define CRYPTOCFG_52_0_CRYPTOKEY_52_0_RegisterSize 32





#define CRYPTOCFG_52_1 (DWC_ufshc_block_BaseAddress + 0x2a04)
#define CRYPTOCFG_52_1_RegisterSize 32
#define CRYPTOCFG_52_1_RegisterResetValue 0x0
#define CRYPTOCFG_52_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_1_CRYPTOKEY_52_1_BitAddressOffset 0
#define CRYPTOCFG_52_1_CRYPTOKEY_52_1_RegisterSize 32





#define CRYPTOCFG_52_2 (DWC_ufshc_block_BaseAddress + 0x2a08)
#define CRYPTOCFG_52_2_RegisterSize 32
#define CRYPTOCFG_52_2_RegisterResetValue 0x0
#define CRYPTOCFG_52_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_2_CRYPTOKEY_52_2_BitAddressOffset 0
#define CRYPTOCFG_52_2_CRYPTOKEY_52_2_RegisterSize 32





#define CRYPTOCFG_52_3 (DWC_ufshc_block_BaseAddress + 0x2a0c)
#define CRYPTOCFG_52_3_RegisterSize 32
#define CRYPTOCFG_52_3_RegisterResetValue 0x0
#define CRYPTOCFG_52_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_3_CRYPTOKEY_52_3_BitAddressOffset 0
#define CRYPTOCFG_52_3_CRYPTOKEY_52_3_RegisterSize 32





#define CRYPTOCFG_52_4 (DWC_ufshc_block_BaseAddress + 0x2a10)
#define CRYPTOCFG_52_4_RegisterSize 32
#define CRYPTOCFG_52_4_RegisterResetValue 0x0
#define CRYPTOCFG_52_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_4_CRYPTOKEY_52_4_BitAddressOffset 0
#define CRYPTOCFG_52_4_CRYPTOKEY_52_4_RegisterSize 32





#define CRYPTOCFG_52_5 (DWC_ufshc_block_BaseAddress + 0x2a14)
#define CRYPTOCFG_52_5_RegisterSize 32
#define CRYPTOCFG_52_5_RegisterResetValue 0x0
#define CRYPTOCFG_52_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_5_CRYPTOKEY_52_5_BitAddressOffset 0
#define CRYPTOCFG_52_5_CRYPTOKEY_52_5_RegisterSize 32





#define CRYPTOCFG_52_6 (DWC_ufshc_block_BaseAddress + 0x2a18)
#define CRYPTOCFG_52_6_RegisterSize 32
#define CRYPTOCFG_52_6_RegisterResetValue 0x0
#define CRYPTOCFG_52_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_6_CRYPTOKEY_52_6_BitAddressOffset 0
#define CRYPTOCFG_52_6_CRYPTOKEY_52_6_RegisterSize 32





#define CRYPTOCFG_52_7 (DWC_ufshc_block_BaseAddress + 0x2a1c)
#define CRYPTOCFG_52_7_RegisterSize 32
#define CRYPTOCFG_52_7_RegisterResetValue 0x0
#define CRYPTOCFG_52_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_7_CRYPTOKEY_52_7_BitAddressOffset 0
#define CRYPTOCFG_52_7_CRYPTOKEY_52_7_RegisterSize 32





#define CRYPTOCFG_52_8 (DWC_ufshc_block_BaseAddress + 0x2a20)
#define CRYPTOCFG_52_8_RegisterSize 32
#define CRYPTOCFG_52_8_RegisterResetValue 0x0
#define CRYPTOCFG_52_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_8_CRYPTOKEY_52_8_BitAddressOffset 0
#define CRYPTOCFG_52_8_CRYPTOKEY_52_8_RegisterSize 32





#define CRYPTOCFG_52_9 (DWC_ufshc_block_BaseAddress + 0x2a24)
#define CRYPTOCFG_52_9_RegisterSize 32
#define CRYPTOCFG_52_9_RegisterResetValue 0x0
#define CRYPTOCFG_52_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_9_CRYPTOKEY_52_9_BitAddressOffset 0
#define CRYPTOCFG_52_9_CRYPTOKEY_52_9_RegisterSize 32





#define CRYPTOCFG_52_10 (DWC_ufshc_block_BaseAddress + 0x2a28)
#define CRYPTOCFG_52_10_RegisterSize 32
#define CRYPTOCFG_52_10_RegisterResetValue 0x0
#define CRYPTOCFG_52_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_10_CRYPTOKEY_52_10_BitAddressOffset 0
#define CRYPTOCFG_52_10_CRYPTOKEY_52_10_RegisterSize 32





#define CRYPTOCFG_52_11 (DWC_ufshc_block_BaseAddress + 0x2a2c)
#define CRYPTOCFG_52_11_RegisterSize 32
#define CRYPTOCFG_52_11_RegisterResetValue 0x0
#define CRYPTOCFG_52_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_11_CRYPTOKEY_52_11_BitAddressOffset 0
#define CRYPTOCFG_52_11_CRYPTOKEY_52_11_RegisterSize 32





#define CRYPTOCFG_52_12 (DWC_ufshc_block_BaseAddress + 0x2a30)
#define CRYPTOCFG_52_12_RegisterSize 32
#define CRYPTOCFG_52_12_RegisterResetValue 0x0
#define CRYPTOCFG_52_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_12_CRYPTOKEY_52_12_BitAddressOffset 0
#define CRYPTOCFG_52_12_CRYPTOKEY_52_12_RegisterSize 32





#define CRYPTOCFG_52_13 (DWC_ufshc_block_BaseAddress + 0x2a34)
#define CRYPTOCFG_52_13_RegisterSize 32
#define CRYPTOCFG_52_13_RegisterResetValue 0x0
#define CRYPTOCFG_52_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_13_CRYPTOKEY_52_13_BitAddressOffset 0
#define CRYPTOCFG_52_13_CRYPTOKEY_52_13_RegisterSize 32





#define CRYPTOCFG_52_14 (DWC_ufshc_block_BaseAddress + 0x2a38)
#define CRYPTOCFG_52_14_RegisterSize 32
#define CRYPTOCFG_52_14_RegisterResetValue 0x0
#define CRYPTOCFG_52_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_14_CRYPTOKEY_52_14_BitAddressOffset 0
#define CRYPTOCFG_52_14_CRYPTOKEY_52_14_RegisterSize 32





#define CRYPTOCFG_52_15 (DWC_ufshc_block_BaseAddress + 0x2a3c)
#define CRYPTOCFG_52_15_RegisterSize 32
#define CRYPTOCFG_52_15_RegisterResetValue 0x0
#define CRYPTOCFG_52_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_15_CRYPTOKEY_52_15_BitAddressOffset 0
#define CRYPTOCFG_52_15_CRYPTOKEY_52_15_RegisterSize 32





#define CRYPTOCFG_52_16 (DWC_ufshc_block_BaseAddress + 0x2a40)
#define CRYPTOCFG_52_16_RegisterSize 32
#define CRYPTOCFG_52_16_RegisterResetValue 0x0
#define CRYPTOCFG_52_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_16_DUSIZE_52_BitAddressOffset 0
#define CRYPTOCFG_52_16_DUSIZE_52_RegisterSize 8



#define CRYPTOCFG_52_16_CAPIDX_52_BitAddressOffset 8
#define CRYPTOCFG_52_16_CAPIDX_52_RegisterSize 8



#define CRYPTOCFG_52_16_CFGE_52_BitAddressOffset 31
#define CRYPTOCFG_52_16_CFGE_52_RegisterSize 1





#define CRYPTOCFG_52_17 (DWC_ufshc_block_BaseAddress + 0x2a44)
#define CRYPTOCFG_52_17_RegisterSize 32
#define CRYPTOCFG_52_17_RegisterResetValue 0x0
#define CRYPTOCFG_52_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_17_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_17_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_18 (DWC_ufshc_block_BaseAddress + 0x2a48)
#define CRYPTOCFG_52_18_RegisterSize 32
#define CRYPTOCFG_52_18_RegisterResetValue 0x0
#define CRYPTOCFG_52_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_18_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_18_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_19 (DWC_ufshc_block_BaseAddress + 0x2a4c)
#define CRYPTOCFG_52_19_RegisterSize 32
#define CRYPTOCFG_52_19_RegisterResetValue 0x0
#define CRYPTOCFG_52_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_19_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_19_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_20 (DWC_ufshc_block_BaseAddress + 0x2a50)
#define CRYPTOCFG_52_20_RegisterSize 32
#define CRYPTOCFG_52_20_RegisterResetValue 0x0
#define CRYPTOCFG_52_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_20_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_20_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_21 (DWC_ufshc_block_BaseAddress + 0x2a54)
#define CRYPTOCFG_52_21_RegisterSize 32
#define CRYPTOCFG_52_21_RegisterResetValue 0x0
#define CRYPTOCFG_52_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_21_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_21_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_22 (DWC_ufshc_block_BaseAddress + 0x2a58)
#define CRYPTOCFG_52_22_RegisterSize 32
#define CRYPTOCFG_52_22_RegisterResetValue 0x0
#define CRYPTOCFG_52_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_22_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_22_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_23 (DWC_ufshc_block_BaseAddress + 0x2a5c)
#define CRYPTOCFG_52_23_RegisterSize 32
#define CRYPTOCFG_52_23_RegisterResetValue 0x0
#define CRYPTOCFG_52_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_23_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_23_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_24 (DWC_ufshc_block_BaseAddress + 0x2a60)
#define CRYPTOCFG_52_24_RegisterSize 32
#define CRYPTOCFG_52_24_RegisterResetValue 0x0
#define CRYPTOCFG_52_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_24_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_24_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_25 (DWC_ufshc_block_BaseAddress + 0x2a64)
#define CRYPTOCFG_52_25_RegisterSize 32
#define CRYPTOCFG_52_25_RegisterResetValue 0x0
#define CRYPTOCFG_52_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_25_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_25_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_26 (DWC_ufshc_block_BaseAddress + 0x2a68)
#define CRYPTOCFG_52_26_RegisterSize 32
#define CRYPTOCFG_52_26_RegisterResetValue 0x0
#define CRYPTOCFG_52_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_26_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_26_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_27 (DWC_ufshc_block_BaseAddress + 0x2a6c)
#define CRYPTOCFG_52_27_RegisterSize 32
#define CRYPTOCFG_52_27_RegisterResetValue 0x0
#define CRYPTOCFG_52_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_27_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_27_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_28 (DWC_ufshc_block_BaseAddress + 0x2a70)
#define CRYPTOCFG_52_28_RegisterSize 32
#define CRYPTOCFG_52_28_RegisterResetValue 0x0
#define CRYPTOCFG_52_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_28_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_28_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_29 (DWC_ufshc_block_BaseAddress + 0x2a74)
#define CRYPTOCFG_52_29_RegisterSize 32
#define CRYPTOCFG_52_29_RegisterResetValue 0x0
#define CRYPTOCFG_52_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_29_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_29_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_30 (DWC_ufshc_block_BaseAddress + 0x2a78)
#define CRYPTOCFG_52_30_RegisterSize 32
#define CRYPTOCFG_52_30_RegisterResetValue 0x0
#define CRYPTOCFG_52_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_30_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_30_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_52_31 (DWC_ufshc_block_BaseAddress + 0x2a7c)
#define CRYPTOCFG_52_31_RegisterSize 32
#define CRYPTOCFG_52_31_RegisterResetValue 0x0
#define CRYPTOCFG_52_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_52_31_RESERVED_52_BitAddressOffset 0
#define CRYPTOCFG_52_31_RESERVED_52_RegisterSize 32





#define CRYPTOCFG_53_0 (DWC_ufshc_block_BaseAddress + 0x2a80)
#define CRYPTOCFG_53_0_RegisterSize 32
#define CRYPTOCFG_53_0_RegisterResetValue 0x0
#define CRYPTOCFG_53_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_0_CRYPTOKEY_53_0_BitAddressOffset 0
#define CRYPTOCFG_53_0_CRYPTOKEY_53_0_RegisterSize 32





#define CRYPTOCFG_53_1 (DWC_ufshc_block_BaseAddress + 0x2a84)
#define CRYPTOCFG_53_1_RegisterSize 32
#define CRYPTOCFG_53_1_RegisterResetValue 0x0
#define CRYPTOCFG_53_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_1_CRYPTOKEY_53_1_BitAddressOffset 0
#define CRYPTOCFG_53_1_CRYPTOKEY_53_1_RegisterSize 32





#define CRYPTOCFG_53_2 (DWC_ufshc_block_BaseAddress + 0x2a88)
#define CRYPTOCFG_53_2_RegisterSize 32
#define CRYPTOCFG_53_2_RegisterResetValue 0x0
#define CRYPTOCFG_53_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_2_CRYPTOKEY_53_2_BitAddressOffset 0
#define CRYPTOCFG_53_2_CRYPTOKEY_53_2_RegisterSize 32





#define CRYPTOCFG_53_3 (DWC_ufshc_block_BaseAddress + 0x2a8c)
#define CRYPTOCFG_53_3_RegisterSize 32
#define CRYPTOCFG_53_3_RegisterResetValue 0x0
#define CRYPTOCFG_53_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_3_CRYPTOKEY_53_3_BitAddressOffset 0
#define CRYPTOCFG_53_3_CRYPTOKEY_53_3_RegisterSize 32





#define CRYPTOCFG_53_4 (DWC_ufshc_block_BaseAddress + 0x2a90)
#define CRYPTOCFG_53_4_RegisterSize 32
#define CRYPTOCFG_53_4_RegisterResetValue 0x0
#define CRYPTOCFG_53_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_4_CRYPTOKEY_53_4_BitAddressOffset 0
#define CRYPTOCFG_53_4_CRYPTOKEY_53_4_RegisterSize 32





#define CRYPTOCFG_53_5 (DWC_ufshc_block_BaseAddress + 0x2a94)
#define CRYPTOCFG_53_5_RegisterSize 32
#define CRYPTOCFG_53_5_RegisterResetValue 0x0
#define CRYPTOCFG_53_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_5_CRYPTOKEY_53_5_BitAddressOffset 0
#define CRYPTOCFG_53_5_CRYPTOKEY_53_5_RegisterSize 32





#define CRYPTOCFG_53_6 (DWC_ufshc_block_BaseAddress + 0x2a98)
#define CRYPTOCFG_53_6_RegisterSize 32
#define CRYPTOCFG_53_6_RegisterResetValue 0x0
#define CRYPTOCFG_53_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_6_CRYPTOKEY_53_6_BitAddressOffset 0
#define CRYPTOCFG_53_6_CRYPTOKEY_53_6_RegisterSize 32





#define CRYPTOCFG_53_7 (DWC_ufshc_block_BaseAddress + 0x2a9c)
#define CRYPTOCFG_53_7_RegisterSize 32
#define CRYPTOCFG_53_7_RegisterResetValue 0x0
#define CRYPTOCFG_53_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_7_CRYPTOKEY_53_7_BitAddressOffset 0
#define CRYPTOCFG_53_7_CRYPTOKEY_53_7_RegisterSize 32





#define CRYPTOCFG_53_8 (DWC_ufshc_block_BaseAddress + 0x2aa0)
#define CRYPTOCFG_53_8_RegisterSize 32
#define CRYPTOCFG_53_8_RegisterResetValue 0x0
#define CRYPTOCFG_53_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_8_CRYPTOKEY_53_8_BitAddressOffset 0
#define CRYPTOCFG_53_8_CRYPTOKEY_53_8_RegisterSize 32





#define CRYPTOCFG_53_9 (DWC_ufshc_block_BaseAddress + 0x2aa4)
#define CRYPTOCFG_53_9_RegisterSize 32
#define CRYPTOCFG_53_9_RegisterResetValue 0x0
#define CRYPTOCFG_53_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_9_CRYPTOKEY_53_9_BitAddressOffset 0
#define CRYPTOCFG_53_9_CRYPTOKEY_53_9_RegisterSize 32





#define CRYPTOCFG_53_10 (DWC_ufshc_block_BaseAddress + 0x2aa8)
#define CRYPTOCFG_53_10_RegisterSize 32
#define CRYPTOCFG_53_10_RegisterResetValue 0x0
#define CRYPTOCFG_53_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_10_CRYPTOKEY_53_10_BitAddressOffset 0
#define CRYPTOCFG_53_10_CRYPTOKEY_53_10_RegisterSize 32





#define CRYPTOCFG_53_11 (DWC_ufshc_block_BaseAddress + 0x2aac)
#define CRYPTOCFG_53_11_RegisterSize 32
#define CRYPTOCFG_53_11_RegisterResetValue 0x0
#define CRYPTOCFG_53_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_11_CRYPTOKEY_53_11_BitAddressOffset 0
#define CRYPTOCFG_53_11_CRYPTOKEY_53_11_RegisterSize 32





#define CRYPTOCFG_53_12 (DWC_ufshc_block_BaseAddress + 0x2ab0)
#define CRYPTOCFG_53_12_RegisterSize 32
#define CRYPTOCFG_53_12_RegisterResetValue 0x0
#define CRYPTOCFG_53_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_12_CRYPTOKEY_53_12_BitAddressOffset 0
#define CRYPTOCFG_53_12_CRYPTOKEY_53_12_RegisterSize 32





#define CRYPTOCFG_53_13 (DWC_ufshc_block_BaseAddress + 0x2ab4)
#define CRYPTOCFG_53_13_RegisterSize 32
#define CRYPTOCFG_53_13_RegisterResetValue 0x0
#define CRYPTOCFG_53_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_13_CRYPTOKEY_53_13_BitAddressOffset 0
#define CRYPTOCFG_53_13_CRYPTOKEY_53_13_RegisterSize 32





#define CRYPTOCFG_53_14 (DWC_ufshc_block_BaseAddress + 0x2ab8)
#define CRYPTOCFG_53_14_RegisterSize 32
#define CRYPTOCFG_53_14_RegisterResetValue 0x0
#define CRYPTOCFG_53_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_14_CRYPTOKEY_53_14_BitAddressOffset 0
#define CRYPTOCFG_53_14_CRYPTOKEY_53_14_RegisterSize 32





#define CRYPTOCFG_53_15 (DWC_ufshc_block_BaseAddress + 0x2abc)
#define CRYPTOCFG_53_15_RegisterSize 32
#define CRYPTOCFG_53_15_RegisterResetValue 0x0
#define CRYPTOCFG_53_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_15_CRYPTOKEY_53_15_BitAddressOffset 0
#define CRYPTOCFG_53_15_CRYPTOKEY_53_15_RegisterSize 32





#define CRYPTOCFG_53_16 (DWC_ufshc_block_BaseAddress + 0x2ac0)
#define CRYPTOCFG_53_16_RegisterSize 32
#define CRYPTOCFG_53_16_RegisterResetValue 0x0
#define CRYPTOCFG_53_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_16_DUSIZE_53_BitAddressOffset 0
#define CRYPTOCFG_53_16_DUSIZE_53_RegisterSize 8



#define CRYPTOCFG_53_16_CAPIDX_53_BitAddressOffset 8
#define CRYPTOCFG_53_16_CAPIDX_53_RegisterSize 8



#define CRYPTOCFG_53_16_CFGE_53_BitAddressOffset 31
#define CRYPTOCFG_53_16_CFGE_53_RegisterSize 1





#define CRYPTOCFG_53_17 (DWC_ufshc_block_BaseAddress + 0x2ac4)
#define CRYPTOCFG_53_17_RegisterSize 32
#define CRYPTOCFG_53_17_RegisterResetValue 0x0
#define CRYPTOCFG_53_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_17_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_17_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_18 (DWC_ufshc_block_BaseAddress + 0x2ac8)
#define CRYPTOCFG_53_18_RegisterSize 32
#define CRYPTOCFG_53_18_RegisterResetValue 0x0
#define CRYPTOCFG_53_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_18_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_18_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_19 (DWC_ufshc_block_BaseAddress + 0x2acc)
#define CRYPTOCFG_53_19_RegisterSize 32
#define CRYPTOCFG_53_19_RegisterResetValue 0x0
#define CRYPTOCFG_53_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_19_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_19_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_20 (DWC_ufshc_block_BaseAddress + 0x2ad0)
#define CRYPTOCFG_53_20_RegisterSize 32
#define CRYPTOCFG_53_20_RegisterResetValue 0x0
#define CRYPTOCFG_53_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_20_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_20_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_21 (DWC_ufshc_block_BaseAddress + 0x2ad4)
#define CRYPTOCFG_53_21_RegisterSize 32
#define CRYPTOCFG_53_21_RegisterResetValue 0x0
#define CRYPTOCFG_53_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_21_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_21_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_22 (DWC_ufshc_block_BaseAddress + 0x2ad8)
#define CRYPTOCFG_53_22_RegisterSize 32
#define CRYPTOCFG_53_22_RegisterResetValue 0x0
#define CRYPTOCFG_53_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_22_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_22_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_23 (DWC_ufshc_block_BaseAddress + 0x2adc)
#define CRYPTOCFG_53_23_RegisterSize 32
#define CRYPTOCFG_53_23_RegisterResetValue 0x0
#define CRYPTOCFG_53_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_23_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_23_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_24 (DWC_ufshc_block_BaseAddress + 0x2ae0)
#define CRYPTOCFG_53_24_RegisterSize 32
#define CRYPTOCFG_53_24_RegisterResetValue 0x0
#define CRYPTOCFG_53_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_24_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_24_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_25 (DWC_ufshc_block_BaseAddress + 0x2ae4)
#define CRYPTOCFG_53_25_RegisterSize 32
#define CRYPTOCFG_53_25_RegisterResetValue 0x0
#define CRYPTOCFG_53_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_25_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_25_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_26 (DWC_ufshc_block_BaseAddress + 0x2ae8)
#define CRYPTOCFG_53_26_RegisterSize 32
#define CRYPTOCFG_53_26_RegisterResetValue 0x0
#define CRYPTOCFG_53_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_26_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_26_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_27 (DWC_ufshc_block_BaseAddress + 0x2aec)
#define CRYPTOCFG_53_27_RegisterSize 32
#define CRYPTOCFG_53_27_RegisterResetValue 0x0
#define CRYPTOCFG_53_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_27_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_27_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_28 (DWC_ufshc_block_BaseAddress + 0x2af0)
#define CRYPTOCFG_53_28_RegisterSize 32
#define CRYPTOCFG_53_28_RegisterResetValue 0x0
#define CRYPTOCFG_53_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_28_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_28_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_29 (DWC_ufshc_block_BaseAddress + 0x2af4)
#define CRYPTOCFG_53_29_RegisterSize 32
#define CRYPTOCFG_53_29_RegisterResetValue 0x0
#define CRYPTOCFG_53_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_29_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_29_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_30 (DWC_ufshc_block_BaseAddress + 0x2af8)
#define CRYPTOCFG_53_30_RegisterSize 32
#define CRYPTOCFG_53_30_RegisterResetValue 0x0
#define CRYPTOCFG_53_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_30_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_30_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_53_31 (DWC_ufshc_block_BaseAddress + 0x2afc)
#define CRYPTOCFG_53_31_RegisterSize 32
#define CRYPTOCFG_53_31_RegisterResetValue 0x0
#define CRYPTOCFG_53_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_53_31_RESERVED_53_BitAddressOffset 0
#define CRYPTOCFG_53_31_RESERVED_53_RegisterSize 32





#define CRYPTOCFG_54_0 (DWC_ufshc_block_BaseAddress + 0x2b00)
#define CRYPTOCFG_54_0_RegisterSize 32
#define CRYPTOCFG_54_0_RegisterResetValue 0x0
#define CRYPTOCFG_54_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_0_CRYPTOKEY_54_0_BitAddressOffset 0
#define CRYPTOCFG_54_0_CRYPTOKEY_54_0_RegisterSize 32





#define CRYPTOCFG_54_1 (DWC_ufshc_block_BaseAddress + 0x2b04)
#define CRYPTOCFG_54_1_RegisterSize 32
#define CRYPTOCFG_54_1_RegisterResetValue 0x0
#define CRYPTOCFG_54_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_1_CRYPTOKEY_54_1_BitAddressOffset 0
#define CRYPTOCFG_54_1_CRYPTOKEY_54_1_RegisterSize 32





#define CRYPTOCFG_54_2 (DWC_ufshc_block_BaseAddress + 0x2b08)
#define CRYPTOCFG_54_2_RegisterSize 32
#define CRYPTOCFG_54_2_RegisterResetValue 0x0
#define CRYPTOCFG_54_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_2_CRYPTOKEY_54_2_BitAddressOffset 0
#define CRYPTOCFG_54_2_CRYPTOKEY_54_2_RegisterSize 32





#define CRYPTOCFG_54_3 (DWC_ufshc_block_BaseAddress + 0x2b0c)
#define CRYPTOCFG_54_3_RegisterSize 32
#define CRYPTOCFG_54_3_RegisterResetValue 0x0
#define CRYPTOCFG_54_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_3_CRYPTOKEY_54_3_BitAddressOffset 0
#define CRYPTOCFG_54_3_CRYPTOKEY_54_3_RegisterSize 32





#define CRYPTOCFG_54_4 (DWC_ufshc_block_BaseAddress + 0x2b10)
#define CRYPTOCFG_54_4_RegisterSize 32
#define CRYPTOCFG_54_4_RegisterResetValue 0x0
#define CRYPTOCFG_54_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_4_CRYPTOKEY_54_4_BitAddressOffset 0
#define CRYPTOCFG_54_4_CRYPTOKEY_54_4_RegisterSize 32





#define CRYPTOCFG_54_5 (DWC_ufshc_block_BaseAddress + 0x2b14)
#define CRYPTOCFG_54_5_RegisterSize 32
#define CRYPTOCFG_54_5_RegisterResetValue 0x0
#define CRYPTOCFG_54_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_5_CRYPTOKEY_54_5_BitAddressOffset 0
#define CRYPTOCFG_54_5_CRYPTOKEY_54_5_RegisterSize 32





#define CRYPTOCFG_54_6 (DWC_ufshc_block_BaseAddress + 0x2b18)
#define CRYPTOCFG_54_6_RegisterSize 32
#define CRYPTOCFG_54_6_RegisterResetValue 0x0
#define CRYPTOCFG_54_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_6_CRYPTOKEY_54_6_BitAddressOffset 0
#define CRYPTOCFG_54_6_CRYPTOKEY_54_6_RegisterSize 32





#define CRYPTOCFG_54_7 (DWC_ufshc_block_BaseAddress + 0x2b1c)
#define CRYPTOCFG_54_7_RegisterSize 32
#define CRYPTOCFG_54_7_RegisterResetValue 0x0
#define CRYPTOCFG_54_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_7_CRYPTOKEY_54_7_BitAddressOffset 0
#define CRYPTOCFG_54_7_CRYPTOKEY_54_7_RegisterSize 32





#define CRYPTOCFG_54_8 (DWC_ufshc_block_BaseAddress + 0x2b20)
#define CRYPTOCFG_54_8_RegisterSize 32
#define CRYPTOCFG_54_8_RegisterResetValue 0x0
#define CRYPTOCFG_54_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_8_CRYPTOKEY_54_8_BitAddressOffset 0
#define CRYPTOCFG_54_8_CRYPTOKEY_54_8_RegisterSize 32





#define CRYPTOCFG_54_9 (DWC_ufshc_block_BaseAddress + 0x2b24)
#define CRYPTOCFG_54_9_RegisterSize 32
#define CRYPTOCFG_54_9_RegisterResetValue 0x0
#define CRYPTOCFG_54_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_9_CRYPTOKEY_54_9_BitAddressOffset 0
#define CRYPTOCFG_54_9_CRYPTOKEY_54_9_RegisterSize 32





#define CRYPTOCFG_54_10 (DWC_ufshc_block_BaseAddress + 0x2b28)
#define CRYPTOCFG_54_10_RegisterSize 32
#define CRYPTOCFG_54_10_RegisterResetValue 0x0
#define CRYPTOCFG_54_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_10_CRYPTOKEY_54_10_BitAddressOffset 0
#define CRYPTOCFG_54_10_CRYPTOKEY_54_10_RegisterSize 32





#define CRYPTOCFG_54_11 (DWC_ufshc_block_BaseAddress + 0x2b2c)
#define CRYPTOCFG_54_11_RegisterSize 32
#define CRYPTOCFG_54_11_RegisterResetValue 0x0
#define CRYPTOCFG_54_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_11_CRYPTOKEY_54_11_BitAddressOffset 0
#define CRYPTOCFG_54_11_CRYPTOKEY_54_11_RegisterSize 32





#define CRYPTOCFG_54_12 (DWC_ufshc_block_BaseAddress + 0x2b30)
#define CRYPTOCFG_54_12_RegisterSize 32
#define CRYPTOCFG_54_12_RegisterResetValue 0x0
#define CRYPTOCFG_54_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_12_CRYPTOKEY_54_12_BitAddressOffset 0
#define CRYPTOCFG_54_12_CRYPTOKEY_54_12_RegisterSize 32





#define CRYPTOCFG_54_13 (DWC_ufshc_block_BaseAddress + 0x2b34)
#define CRYPTOCFG_54_13_RegisterSize 32
#define CRYPTOCFG_54_13_RegisterResetValue 0x0
#define CRYPTOCFG_54_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_13_CRYPTOKEY_54_13_BitAddressOffset 0
#define CRYPTOCFG_54_13_CRYPTOKEY_54_13_RegisterSize 32





#define CRYPTOCFG_54_14 (DWC_ufshc_block_BaseAddress + 0x2b38)
#define CRYPTOCFG_54_14_RegisterSize 32
#define CRYPTOCFG_54_14_RegisterResetValue 0x0
#define CRYPTOCFG_54_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_14_CRYPTOKEY_54_14_BitAddressOffset 0
#define CRYPTOCFG_54_14_CRYPTOKEY_54_14_RegisterSize 32





#define CRYPTOCFG_54_15 (DWC_ufshc_block_BaseAddress + 0x2b3c)
#define CRYPTOCFG_54_15_RegisterSize 32
#define CRYPTOCFG_54_15_RegisterResetValue 0x0
#define CRYPTOCFG_54_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_15_CRYPTOKEY_54_15_BitAddressOffset 0
#define CRYPTOCFG_54_15_CRYPTOKEY_54_15_RegisterSize 32





#define CRYPTOCFG_54_16 (DWC_ufshc_block_BaseAddress + 0x2b40)
#define CRYPTOCFG_54_16_RegisterSize 32
#define CRYPTOCFG_54_16_RegisterResetValue 0x0
#define CRYPTOCFG_54_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_16_DUSIZE_54_BitAddressOffset 0
#define CRYPTOCFG_54_16_DUSIZE_54_RegisterSize 8



#define CRYPTOCFG_54_16_CAPIDX_54_BitAddressOffset 8
#define CRYPTOCFG_54_16_CAPIDX_54_RegisterSize 8



#define CRYPTOCFG_54_16_CFGE_54_BitAddressOffset 31
#define CRYPTOCFG_54_16_CFGE_54_RegisterSize 1





#define CRYPTOCFG_54_17 (DWC_ufshc_block_BaseAddress + 0x2b44)
#define CRYPTOCFG_54_17_RegisterSize 32
#define CRYPTOCFG_54_17_RegisterResetValue 0x0
#define CRYPTOCFG_54_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_17_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_17_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_18 (DWC_ufshc_block_BaseAddress + 0x2b48)
#define CRYPTOCFG_54_18_RegisterSize 32
#define CRYPTOCFG_54_18_RegisterResetValue 0x0
#define CRYPTOCFG_54_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_18_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_18_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_19 (DWC_ufshc_block_BaseAddress + 0x2b4c)
#define CRYPTOCFG_54_19_RegisterSize 32
#define CRYPTOCFG_54_19_RegisterResetValue 0x0
#define CRYPTOCFG_54_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_19_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_19_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_20 (DWC_ufshc_block_BaseAddress + 0x2b50)
#define CRYPTOCFG_54_20_RegisterSize 32
#define CRYPTOCFG_54_20_RegisterResetValue 0x0
#define CRYPTOCFG_54_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_20_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_20_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_21 (DWC_ufshc_block_BaseAddress + 0x2b54)
#define CRYPTOCFG_54_21_RegisterSize 32
#define CRYPTOCFG_54_21_RegisterResetValue 0x0
#define CRYPTOCFG_54_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_21_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_21_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_22 (DWC_ufshc_block_BaseAddress + 0x2b58)
#define CRYPTOCFG_54_22_RegisterSize 32
#define CRYPTOCFG_54_22_RegisterResetValue 0x0
#define CRYPTOCFG_54_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_22_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_22_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_23 (DWC_ufshc_block_BaseAddress + 0x2b5c)
#define CRYPTOCFG_54_23_RegisterSize 32
#define CRYPTOCFG_54_23_RegisterResetValue 0x0
#define CRYPTOCFG_54_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_23_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_23_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_24 (DWC_ufshc_block_BaseAddress + 0x2b60)
#define CRYPTOCFG_54_24_RegisterSize 32
#define CRYPTOCFG_54_24_RegisterResetValue 0x0
#define CRYPTOCFG_54_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_24_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_24_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_25 (DWC_ufshc_block_BaseAddress + 0x2b64)
#define CRYPTOCFG_54_25_RegisterSize 32
#define CRYPTOCFG_54_25_RegisterResetValue 0x0
#define CRYPTOCFG_54_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_25_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_25_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_26 (DWC_ufshc_block_BaseAddress + 0x2b68)
#define CRYPTOCFG_54_26_RegisterSize 32
#define CRYPTOCFG_54_26_RegisterResetValue 0x0
#define CRYPTOCFG_54_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_26_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_26_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_27 (DWC_ufshc_block_BaseAddress + 0x2b6c)
#define CRYPTOCFG_54_27_RegisterSize 32
#define CRYPTOCFG_54_27_RegisterResetValue 0x0
#define CRYPTOCFG_54_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_27_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_27_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_28 (DWC_ufshc_block_BaseAddress + 0x2b70)
#define CRYPTOCFG_54_28_RegisterSize 32
#define CRYPTOCFG_54_28_RegisterResetValue 0x0
#define CRYPTOCFG_54_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_28_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_28_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_29 (DWC_ufshc_block_BaseAddress + 0x2b74)
#define CRYPTOCFG_54_29_RegisterSize 32
#define CRYPTOCFG_54_29_RegisterResetValue 0x0
#define CRYPTOCFG_54_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_29_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_29_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_30 (DWC_ufshc_block_BaseAddress + 0x2b78)
#define CRYPTOCFG_54_30_RegisterSize 32
#define CRYPTOCFG_54_30_RegisterResetValue 0x0
#define CRYPTOCFG_54_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_30_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_30_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_54_31 (DWC_ufshc_block_BaseAddress + 0x2b7c)
#define CRYPTOCFG_54_31_RegisterSize 32
#define CRYPTOCFG_54_31_RegisterResetValue 0x0
#define CRYPTOCFG_54_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_54_31_RESERVED_54_BitAddressOffset 0
#define CRYPTOCFG_54_31_RESERVED_54_RegisterSize 32





#define CRYPTOCFG_55_0 (DWC_ufshc_block_BaseAddress + 0x2b80)
#define CRYPTOCFG_55_0_RegisterSize 32
#define CRYPTOCFG_55_0_RegisterResetValue 0x0
#define CRYPTOCFG_55_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_0_CRYPTOKEY_55_0_BitAddressOffset 0
#define CRYPTOCFG_55_0_CRYPTOKEY_55_0_RegisterSize 32





#define CRYPTOCFG_55_1 (DWC_ufshc_block_BaseAddress + 0x2b84)
#define CRYPTOCFG_55_1_RegisterSize 32
#define CRYPTOCFG_55_1_RegisterResetValue 0x0
#define CRYPTOCFG_55_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_1_CRYPTOKEY_55_1_BitAddressOffset 0
#define CRYPTOCFG_55_1_CRYPTOKEY_55_1_RegisterSize 32





#define CRYPTOCFG_55_2 (DWC_ufshc_block_BaseAddress + 0x2b88)
#define CRYPTOCFG_55_2_RegisterSize 32
#define CRYPTOCFG_55_2_RegisterResetValue 0x0
#define CRYPTOCFG_55_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_2_CRYPTOKEY_55_2_BitAddressOffset 0
#define CRYPTOCFG_55_2_CRYPTOKEY_55_2_RegisterSize 32





#define CRYPTOCFG_55_3 (DWC_ufshc_block_BaseAddress + 0x2b8c)
#define CRYPTOCFG_55_3_RegisterSize 32
#define CRYPTOCFG_55_3_RegisterResetValue 0x0
#define CRYPTOCFG_55_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_3_CRYPTOKEY_55_3_BitAddressOffset 0
#define CRYPTOCFG_55_3_CRYPTOKEY_55_3_RegisterSize 32





#define CRYPTOCFG_55_4 (DWC_ufshc_block_BaseAddress + 0x2b90)
#define CRYPTOCFG_55_4_RegisterSize 32
#define CRYPTOCFG_55_4_RegisterResetValue 0x0
#define CRYPTOCFG_55_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_4_CRYPTOKEY_55_4_BitAddressOffset 0
#define CRYPTOCFG_55_4_CRYPTOKEY_55_4_RegisterSize 32





#define CRYPTOCFG_55_5 (DWC_ufshc_block_BaseAddress + 0x2b94)
#define CRYPTOCFG_55_5_RegisterSize 32
#define CRYPTOCFG_55_5_RegisterResetValue 0x0
#define CRYPTOCFG_55_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_5_CRYPTOKEY_55_5_BitAddressOffset 0
#define CRYPTOCFG_55_5_CRYPTOKEY_55_5_RegisterSize 32





#define CRYPTOCFG_55_6 (DWC_ufshc_block_BaseAddress + 0x2b98)
#define CRYPTOCFG_55_6_RegisterSize 32
#define CRYPTOCFG_55_6_RegisterResetValue 0x0
#define CRYPTOCFG_55_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_6_CRYPTOKEY_55_6_BitAddressOffset 0
#define CRYPTOCFG_55_6_CRYPTOKEY_55_6_RegisterSize 32





#define CRYPTOCFG_55_7 (DWC_ufshc_block_BaseAddress + 0x2b9c)
#define CRYPTOCFG_55_7_RegisterSize 32
#define CRYPTOCFG_55_7_RegisterResetValue 0x0
#define CRYPTOCFG_55_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_7_CRYPTOKEY_55_7_BitAddressOffset 0
#define CRYPTOCFG_55_7_CRYPTOKEY_55_7_RegisterSize 32





#define CRYPTOCFG_55_8 (DWC_ufshc_block_BaseAddress + 0x2ba0)
#define CRYPTOCFG_55_8_RegisterSize 32
#define CRYPTOCFG_55_8_RegisterResetValue 0x0
#define CRYPTOCFG_55_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_8_CRYPTOKEY_55_8_BitAddressOffset 0
#define CRYPTOCFG_55_8_CRYPTOKEY_55_8_RegisterSize 32





#define CRYPTOCFG_55_9 (DWC_ufshc_block_BaseAddress + 0x2ba4)
#define CRYPTOCFG_55_9_RegisterSize 32
#define CRYPTOCFG_55_9_RegisterResetValue 0x0
#define CRYPTOCFG_55_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_9_CRYPTOKEY_55_9_BitAddressOffset 0
#define CRYPTOCFG_55_9_CRYPTOKEY_55_9_RegisterSize 32





#define CRYPTOCFG_55_10 (DWC_ufshc_block_BaseAddress + 0x2ba8)
#define CRYPTOCFG_55_10_RegisterSize 32
#define CRYPTOCFG_55_10_RegisterResetValue 0x0
#define CRYPTOCFG_55_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_10_CRYPTOKEY_55_10_BitAddressOffset 0
#define CRYPTOCFG_55_10_CRYPTOKEY_55_10_RegisterSize 32





#define CRYPTOCFG_55_11 (DWC_ufshc_block_BaseAddress + 0x2bac)
#define CRYPTOCFG_55_11_RegisterSize 32
#define CRYPTOCFG_55_11_RegisterResetValue 0x0
#define CRYPTOCFG_55_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_11_CRYPTOKEY_55_11_BitAddressOffset 0
#define CRYPTOCFG_55_11_CRYPTOKEY_55_11_RegisterSize 32





#define CRYPTOCFG_55_12 (DWC_ufshc_block_BaseAddress + 0x2bb0)
#define CRYPTOCFG_55_12_RegisterSize 32
#define CRYPTOCFG_55_12_RegisterResetValue 0x0
#define CRYPTOCFG_55_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_12_CRYPTOKEY_55_12_BitAddressOffset 0
#define CRYPTOCFG_55_12_CRYPTOKEY_55_12_RegisterSize 32





#define CRYPTOCFG_55_13 (DWC_ufshc_block_BaseAddress + 0x2bb4)
#define CRYPTOCFG_55_13_RegisterSize 32
#define CRYPTOCFG_55_13_RegisterResetValue 0x0
#define CRYPTOCFG_55_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_13_CRYPTOKEY_55_13_BitAddressOffset 0
#define CRYPTOCFG_55_13_CRYPTOKEY_55_13_RegisterSize 32





#define CRYPTOCFG_55_14 (DWC_ufshc_block_BaseAddress + 0x2bb8)
#define CRYPTOCFG_55_14_RegisterSize 32
#define CRYPTOCFG_55_14_RegisterResetValue 0x0
#define CRYPTOCFG_55_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_14_CRYPTOKEY_55_14_BitAddressOffset 0
#define CRYPTOCFG_55_14_CRYPTOKEY_55_14_RegisterSize 32





#define CRYPTOCFG_55_15 (DWC_ufshc_block_BaseAddress + 0x2bbc)
#define CRYPTOCFG_55_15_RegisterSize 32
#define CRYPTOCFG_55_15_RegisterResetValue 0x0
#define CRYPTOCFG_55_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_15_CRYPTOKEY_55_15_BitAddressOffset 0
#define CRYPTOCFG_55_15_CRYPTOKEY_55_15_RegisterSize 32





#define CRYPTOCFG_55_16 (DWC_ufshc_block_BaseAddress + 0x2bc0)
#define CRYPTOCFG_55_16_RegisterSize 32
#define CRYPTOCFG_55_16_RegisterResetValue 0x0
#define CRYPTOCFG_55_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_16_DUSIZE_55_BitAddressOffset 0
#define CRYPTOCFG_55_16_DUSIZE_55_RegisterSize 8



#define CRYPTOCFG_55_16_CAPIDX_55_BitAddressOffset 8
#define CRYPTOCFG_55_16_CAPIDX_55_RegisterSize 8



#define CRYPTOCFG_55_16_CFGE_55_BitAddressOffset 31
#define CRYPTOCFG_55_16_CFGE_55_RegisterSize 1





#define CRYPTOCFG_55_17 (DWC_ufshc_block_BaseAddress + 0x2bc4)
#define CRYPTOCFG_55_17_RegisterSize 32
#define CRYPTOCFG_55_17_RegisterResetValue 0x0
#define CRYPTOCFG_55_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_17_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_17_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_18 (DWC_ufshc_block_BaseAddress + 0x2bc8)
#define CRYPTOCFG_55_18_RegisterSize 32
#define CRYPTOCFG_55_18_RegisterResetValue 0x0
#define CRYPTOCFG_55_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_18_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_18_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_19 (DWC_ufshc_block_BaseAddress + 0x2bcc)
#define CRYPTOCFG_55_19_RegisterSize 32
#define CRYPTOCFG_55_19_RegisterResetValue 0x0
#define CRYPTOCFG_55_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_19_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_19_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_20 (DWC_ufshc_block_BaseAddress + 0x2bd0)
#define CRYPTOCFG_55_20_RegisterSize 32
#define CRYPTOCFG_55_20_RegisterResetValue 0x0
#define CRYPTOCFG_55_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_20_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_20_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_21 (DWC_ufshc_block_BaseAddress + 0x2bd4)
#define CRYPTOCFG_55_21_RegisterSize 32
#define CRYPTOCFG_55_21_RegisterResetValue 0x0
#define CRYPTOCFG_55_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_21_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_21_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_22 (DWC_ufshc_block_BaseAddress + 0x2bd8)
#define CRYPTOCFG_55_22_RegisterSize 32
#define CRYPTOCFG_55_22_RegisterResetValue 0x0
#define CRYPTOCFG_55_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_22_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_22_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_23 (DWC_ufshc_block_BaseAddress + 0x2bdc)
#define CRYPTOCFG_55_23_RegisterSize 32
#define CRYPTOCFG_55_23_RegisterResetValue 0x0
#define CRYPTOCFG_55_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_23_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_23_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_24 (DWC_ufshc_block_BaseAddress + 0x2be0)
#define CRYPTOCFG_55_24_RegisterSize 32
#define CRYPTOCFG_55_24_RegisterResetValue 0x0
#define CRYPTOCFG_55_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_24_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_24_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_25 (DWC_ufshc_block_BaseAddress + 0x2be4)
#define CRYPTOCFG_55_25_RegisterSize 32
#define CRYPTOCFG_55_25_RegisterResetValue 0x0
#define CRYPTOCFG_55_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_25_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_25_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_26 (DWC_ufshc_block_BaseAddress + 0x2be8)
#define CRYPTOCFG_55_26_RegisterSize 32
#define CRYPTOCFG_55_26_RegisterResetValue 0x0
#define CRYPTOCFG_55_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_26_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_26_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_27 (DWC_ufshc_block_BaseAddress + 0x2bec)
#define CRYPTOCFG_55_27_RegisterSize 32
#define CRYPTOCFG_55_27_RegisterResetValue 0x0
#define CRYPTOCFG_55_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_27_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_27_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_28 (DWC_ufshc_block_BaseAddress + 0x2bf0)
#define CRYPTOCFG_55_28_RegisterSize 32
#define CRYPTOCFG_55_28_RegisterResetValue 0x0
#define CRYPTOCFG_55_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_28_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_28_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_29 (DWC_ufshc_block_BaseAddress + 0x2bf4)
#define CRYPTOCFG_55_29_RegisterSize 32
#define CRYPTOCFG_55_29_RegisterResetValue 0x0
#define CRYPTOCFG_55_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_29_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_29_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_30 (DWC_ufshc_block_BaseAddress + 0x2bf8)
#define CRYPTOCFG_55_30_RegisterSize 32
#define CRYPTOCFG_55_30_RegisterResetValue 0x0
#define CRYPTOCFG_55_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_30_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_30_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_55_31 (DWC_ufshc_block_BaseAddress + 0x2bfc)
#define CRYPTOCFG_55_31_RegisterSize 32
#define CRYPTOCFG_55_31_RegisterResetValue 0x0
#define CRYPTOCFG_55_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_55_31_RESERVED_55_BitAddressOffset 0
#define CRYPTOCFG_55_31_RESERVED_55_RegisterSize 32





#define CRYPTOCFG_56_0 (DWC_ufshc_block_BaseAddress + 0x2c00)
#define CRYPTOCFG_56_0_RegisterSize 32
#define CRYPTOCFG_56_0_RegisterResetValue 0x0
#define CRYPTOCFG_56_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_0_CRYPTOKEY_56_0_BitAddressOffset 0
#define CRYPTOCFG_56_0_CRYPTOKEY_56_0_RegisterSize 32





#define CRYPTOCFG_56_1 (DWC_ufshc_block_BaseAddress + 0x2c04)
#define CRYPTOCFG_56_1_RegisterSize 32
#define CRYPTOCFG_56_1_RegisterResetValue 0x0
#define CRYPTOCFG_56_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_1_CRYPTOKEY_56_1_BitAddressOffset 0
#define CRYPTOCFG_56_1_CRYPTOKEY_56_1_RegisterSize 32





#define CRYPTOCFG_56_2 (DWC_ufshc_block_BaseAddress + 0x2c08)
#define CRYPTOCFG_56_2_RegisterSize 32
#define CRYPTOCFG_56_2_RegisterResetValue 0x0
#define CRYPTOCFG_56_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_2_CRYPTOKEY_56_2_BitAddressOffset 0
#define CRYPTOCFG_56_2_CRYPTOKEY_56_2_RegisterSize 32





#define CRYPTOCFG_56_3 (DWC_ufshc_block_BaseAddress + 0x2c0c)
#define CRYPTOCFG_56_3_RegisterSize 32
#define CRYPTOCFG_56_3_RegisterResetValue 0x0
#define CRYPTOCFG_56_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_3_CRYPTOKEY_56_3_BitAddressOffset 0
#define CRYPTOCFG_56_3_CRYPTOKEY_56_3_RegisterSize 32





#define CRYPTOCFG_56_4 (DWC_ufshc_block_BaseAddress + 0x2c10)
#define CRYPTOCFG_56_4_RegisterSize 32
#define CRYPTOCFG_56_4_RegisterResetValue 0x0
#define CRYPTOCFG_56_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_4_CRYPTOKEY_56_4_BitAddressOffset 0
#define CRYPTOCFG_56_4_CRYPTOKEY_56_4_RegisterSize 32





#define CRYPTOCFG_56_5 (DWC_ufshc_block_BaseAddress + 0x2c14)
#define CRYPTOCFG_56_5_RegisterSize 32
#define CRYPTOCFG_56_5_RegisterResetValue 0x0
#define CRYPTOCFG_56_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_5_CRYPTOKEY_56_5_BitAddressOffset 0
#define CRYPTOCFG_56_5_CRYPTOKEY_56_5_RegisterSize 32





#define CRYPTOCFG_56_6 (DWC_ufshc_block_BaseAddress + 0x2c18)
#define CRYPTOCFG_56_6_RegisterSize 32
#define CRYPTOCFG_56_6_RegisterResetValue 0x0
#define CRYPTOCFG_56_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_6_CRYPTOKEY_56_6_BitAddressOffset 0
#define CRYPTOCFG_56_6_CRYPTOKEY_56_6_RegisterSize 32





#define CRYPTOCFG_56_7 (DWC_ufshc_block_BaseAddress + 0x2c1c)
#define CRYPTOCFG_56_7_RegisterSize 32
#define CRYPTOCFG_56_7_RegisterResetValue 0x0
#define CRYPTOCFG_56_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_7_CRYPTOKEY_56_7_BitAddressOffset 0
#define CRYPTOCFG_56_7_CRYPTOKEY_56_7_RegisterSize 32





#define CRYPTOCFG_56_8 (DWC_ufshc_block_BaseAddress + 0x2c20)
#define CRYPTOCFG_56_8_RegisterSize 32
#define CRYPTOCFG_56_8_RegisterResetValue 0x0
#define CRYPTOCFG_56_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_8_CRYPTOKEY_56_8_BitAddressOffset 0
#define CRYPTOCFG_56_8_CRYPTOKEY_56_8_RegisterSize 32





#define CRYPTOCFG_56_9 (DWC_ufshc_block_BaseAddress + 0x2c24)
#define CRYPTOCFG_56_9_RegisterSize 32
#define CRYPTOCFG_56_9_RegisterResetValue 0x0
#define CRYPTOCFG_56_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_9_CRYPTOKEY_56_9_BitAddressOffset 0
#define CRYPTOCFG_56_9_CRYPTOKEY_56_9_RegisterSize 32





#define CRYPTOCFG_56_10 (DWC_ufshc_block_BaseAddress + 0x2c28)
#define CRYPTOCFG_56_10_RegisterSize 32
#define CRYPTOCFG_56_10_RegisterResetValue 0x0
#define CRYPTOCFG_56_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_10_CRYPTOKEY_56_10_BitAddressOffset 0
#define CRYPTOCFG_56_10_CRYPTOKEY_56_10_RegisterSize 32





#define CRYPTOCFG_56_11 (DWC_ufshc_block_BaseAddress + 0x2c2c)
#define CRYPTOCFG_56_11_RegisterSize 32
#define CRYPTOCFG_56_11_RegisterResetValue 0x0
#define CRYPTOCFG_56_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_11_CRYPTOKEY_56_11_BitAddressOffset 0
#define CRYPTOCFG_56_11_CRYPTOKEY_56_11_RegisterSize 32





#define CRYPTOCFG_56_12 (DWC_ufshc_block_BaseAddress + 0x2c30)
#define CRYPTOCFG_56_12_RegisterSize 32
#define CRYPTOCFG_56_12_RegisterResetValue 0x0
#define CRYPTOCFG_56_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_12_CRYPTOKEY_56_12_BitAddressOffset 0
#define CRYPTOCFG_56_12_CRYPTOKEY_56_12_RegisterSize 32





#define CRYPTOCFG_56_13 (DWC_ufshc_block_BaseAddress + 0x2c34)
#define CRYPTOCFG_56_13_RegisterSize 32
#define CRYPTOCFG_56_13_RegisterResetValue 0x0
#define CRYPTOCFG_56_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_13_CRYPTOKEY_56_13_BitAddressOffset 0
#define CRYPTOCFG_56_13_CRYPTOKEY_56_13_RegisterSize 32





#define CRYPTOCFG_56_14 (DWC_ufshc_block_BaseAddress + 0x2c38)
#define CRYPTOCFG_56_14_RegisterSize 32
#define CRYPTOCFG_56_14_RegisterResetValue 0x0
#define CRYPTOCFG_56_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_14_CRYPTOKEY_56_14_BitAddressOffset 0
#define CRYPTOCFG_56_14_CRYPTOKEY_56_14_RegisterSize 32





#define CRYPTOCFG_56_15 (DWC_ufshc_block_BaseAddress + 0x2c3c)
#define CRYPTOCFG_56_15_RegisterSize 32
#define CRYPTOCFG_56_15_RegisterResetValue 0x0
#define CRYPTOCFG_56_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_15_CRYPTOKEY_56_15_BitAddressOffset 0
#define CRYPTOCFG_56_15_CRYPTOKEY_56_15_RegisterSize 32





#define CRYPTOCFG_56_16 (DWC_ufshc_block_BaseAddress + 0x2c40)
#define CRYPTOCFG_56_16_RegisterSize 32
#define CRYPTOCFG_56_16_RegisterResetValue 0x0
#define CRYPTOCFG_56_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_16_DUSIZE_56_BitAddressOffset 0
#define CRYPTOCFG_56_16_DUSIZE_56_RegisterSize 8



#define CRYPTOCFG_56_16_CAPIDX_56_BitAddressOffset 8
#define CRYPTOCFG_56_16_CAPIDX_56_RegisterSize 8



#define CRYPTOCFG_56_16_CFGE_56_BitAddressOffset 31
#define CRYPTOCFG_56_16_CFGE_56_RegisterSize 1





#define CRYPTOCFG_56_17 (DWC_ufshc_block_BaseAddress + 0x2c44)
#define CRYPTOCFG_56_17_RegisterSize 32
#define CRYPTOCFG_56_17_RegisterResetValue 0x0
#define CRYPTOCFG_56_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_17_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_17_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_18 (DWC_ufshc_block_BaseAddress + 0x2c48)
#define CRYPTOCFG_56_18_RegisterSize 32
#define CRYPTOCFG_56_18_RegisterResetValue 0x0
#define CRYPTOCFG_56_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_18_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_18_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_19 (DWC_ufshc_block_BaseAddress + 0x2c4c)
#define CRYPTOCFG_56_19_RegisterSize 32
#define CRYPTOCFG_56_19_RegisterResetValue 0x0
#define CRYPTOCFG_56_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_19_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_19_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_20 (DWC_ufshc_block_BaseAddress + 0x2c50)
#define CRYPTOCFG_56_20_RegisterSize 32
#define CRYPTOCFG_56_20_RegisterResetValue 0x0
#define CRYPTOCFG_56_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_20_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_20_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_21 (DWC_ufshc_block_BaseAddress + 0x2c54)
#define CRYPTOCFG_56_21_RegisterSize 32
#define CRYPTOCFG_56_21_RegisterResetValue 0x0
#define CRYPTOCFG_56_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_21_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_21_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_22 (DWC_ufshc_block_BaseAddress + 0x2c58)
#define CRYPTOCFG_56_22_RegisterSize 32
#define CRYPTOCFG_56_22_RegisterResetValue 0x0
#define CRYPTOCFG_56_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_22_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_22_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_23 (DWC_ufshc_block_BaseAddress + 0x2c5c)
#define CRYPTOCFG_56_23_RegisterSize 32
#define CRYPTOCFG_56_23_RegisterResetValue 0x0
#define CRYPTOCFG_56_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_23_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_23_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_24 (DWC_ufshc_block_BaseAddress + 0x2c60)
#define CRYPTOCFG_56_24_RegisterSize 32
#define CRYPTOCFG_56_24_RegisterResetValue 0x0
#define CRYPTOCFG_56_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_24_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_24_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_25 (DWC_ufshc_block_BaseAddress + 0x2c64)
#define CRYPTOCFG_56_25_RegisterSize 32
#define CRYPTOCFG_56_25_RegisterResetValue 0x0
#define CRYPTOCFG_56_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_25_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_25_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_26 (DWC_ufshc_block_BaseAddress + 0x2c68)
#define CRYPTOCFG_56_26_RegisterSize 32
#define CRYPTOCFG_56_26_RegisterResetValue 0x0
#define CRYPTOCFG_56_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_26_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_26_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_27 (DWC_ufshc_block_BaseAddress + 0x2c6c)
#define CRYPTOCFG_56_27_RegisterSize 32
#define CRYPTOCFG_56_27_RegisterResetValue 0x0
#define CRYPTOCFG_56_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_27_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_27_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_28 (DWC_ufshc_block_BaseAddress + 0x2c70)
#define CRYPTOCFG_56_28_RegisterSize 32
#define CRYPTOCFG_56_28_RegisterResetValue 0x0
#define CRYPTOCFG_56_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_28_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_28_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_29 (DWC_ufshc_block_BaseAddress + 0x2c74)
#define CRYPTOCFG_56_29_RegisterSize 32
#define CRYPTOCFG_56_29_RegisterResetValue 0x0
#define CRYPTOCFG_56_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_29_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_29_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_30 (DWC_ufshc_block_BaseAddress + 0x2c78)
#define CRYPTOCFG_56_30_RegisterSize 32
#define CRYPTOCFG_56_30_RegisterResetValue 0x0
#define CRYPTOCFG_56_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_30_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_30_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_56_31 (DWC_ufshc_block_BaseAddress + 0x2c7c)
#define CRYPTOCFG_56_31_RegisterSize 32
#define CRYPTOCFG_56_31_RegisterResetValue 0x0
#define CRYPTOCFG_56_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_56_31_RESERVED_56_BitAddressOffset 0
#define CRYPTOCFG_56_31_RESERVED_56_RegisterSize 32





#define CRYPTOCFG_57_0 (DWC_ufshc_block_BaseAddress + 0x2c80)
#define CRYPTOCFG_57_0_RegisterSize 32
#define CRYPTOCFG_57_0_RegisterResetValue 0x0
#define CRYPTOCFG_57_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_0_CRYPTOKEY_57_0_BitAddressOffset 0
#define CRYPTOCFG_57_0_CRYPTOKEY_57_0_RegisterSize 32





#define CRYPTOCFG_57_1 (DWC_ufshc_block_BaseAddress + 0x2c84)
#define CRYPTOCFG_57_1_RegisterSize 32
#define CRYPTOCFG_57_1_RegisterResetValue 0x0
#define CRYPTOCFG_57_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_1_CRYPTOKEY_57_1_BitAddressOffset 0
#define CRYPTOCFG_57_1_CRYPTOKEY_57_1_RegisterSize 32





#define CRYPTOCFG_57_2 (DWC_ufshc_block_BaseAddress + 0x2c88)
#define CRYPTOCFG_57_2_RegisterSize 32
#define CRYPTOCFG_57_2_RegisterResetValue 0x0
#define CRYPTOCFG_57_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_2_CRYPTOKEY_57_2_BitAddressOffset 0
#define CRYPTOCFG_57_2_CRYPTOKEY_57_2_RegisterSize 32





#define CRYPTOCFG_57_3 (DWC_ufshc_block_BaseAddress + 0x2c8c)
#define CRYPTOCFG_57_3_RegisterSize 32
#define CRYPTOCFG_57_3_RegisterResetValue 0x0
#define CRYPTOCFG_57_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_3_CRYPTOKEY_57_3_BitAddressOffset 0
#define CRYPTOCFG_57_3_CRYPTOKEY_57_3_RegisterSize 32





#define CRYPTOCFG_57_4 (DWC_ufshc_block_BaseAddress + 0x2c90)
#define CRYPTOCFG_57_4_RegisterSize 32
#define CRYPTOCFG_57_4_RegisterResetValue 0x0
#define CRYPTOCFG_57_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_4_CRYPTOKEY_57_4_BitAddressOffset 0
#define CRYPTOCFG_57_4_CRYPTOKEY_57_4_RegisterSize 32





#define CRYPTOCFG_57_5 (DWC_ufshc_block_BaseAddress + 0x2c94)
#define CRYPTOCFG_57_5_RegisterSize 32
#define CRYPTOCFG_57_5_RegisterResetValue 0x0
#define CRYPTOCFG_57_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_5_CRYPTOKEY_57_5_BitAddressOffset 0
#define CRYPTOCFG_57_5_CRYPTOKEY_57_5_RegisterSize 32





#define CRYPTOCFG_57_6 (DWC_ufshc_block_BaseAddress + 0x2c98)
#define CRYPTOCFG_57_6_RegisterSize 32
#define CRYPTOCFG_57_6_RegisterResetValue 0x0
#define CRYPTOCFG_57_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_6_CRYPTOKEY_57_6_BitAddressOffset 0
#define CRYPTOCFG_57_6_CRYPTOKEY_57_6_RegisterSize 32





#define CRYPTOCFG_57_7 (DWC_ufshc_block_BaseAddress + 0x2c9c)
#define CRYPTOCFG_57_7_RegisterSize 32
#define CRYPTOCFG_57_7_RegisterResetValue 0x0
#define CRYPTOCFG_57_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_7_CRYPTOKEY_57_7_BitAddressOffset 0
#define CRYPTOCFG_57_7_CRYPTOKEY_57_7_RegisterSize 32





#define CRYPTOCFG_57_8 (DWC_ufshc_block_BaseAddress + 0x2ca0)
#define CRYPTOCFG_57_8_RegisterSize 32
#define CRYPTOCFG_57_8_RegisterResetValue 0x0
#define CRYPTOCFG_57_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_8_CRYPTOKEY_57_8_BitAddressOffset 0
#define CRYPTOCFG_57_8_CRYPTOKEY_57_8_RegisterSize 32





#define CRYPTOCFG_57_9 (DWC_ufshc_block_BaseAddress + 0x2ca4)
#define CRYPTOCFG_57_9_RegisterSize 32
#define CRYPTOCFG_57_9_RegisterResetValue 0x0
#define CRYPTOCFG_57_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_9_CRYPTOKEY_57_9_BitAddressOffset 0
#define CRYPTOCFG_57_9_CRYPTOKEY_57_9_RegisterSize 32





#define CRYPTOCFG_57_10 (DWC_ufshc_block_BaseAddress + 0x2ca8)
#define CRYPTOCFG_57_10_RegisterSize 32
#define CRYPTOCFG_57_10_RegisterResetValue 0x0
#define CRYPTOCFG_57_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_10_CRYPTOKEY_57_10_BitAddressOffset 0
#define CRYPTOCFG_57_10_CRYPTOKEY_57_10_RegisterSize 32





#define CRYPTOCFG_57_11 (DWC_ufshc_block_BaseAddress + 0x2cac)
#define CRYPTOCFG_57_11_RegisterSize 32
#define CRYPTOCFG_57_11_RegisterResetValue 0x0
#define CRYPTOCFG_57_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_11_CRYPTOKEY_57_11_BitAddressOffset 0
#define CRYPTOCFG_57_11_CRYPTOKEY_57_11_RegisterSize 32





#define CRYPTOCFG_57_12 (DWC_ufshc_block_BaseAddress + 0x2cb0)
#define CRYPTOCFG_57_12_RegisterSize 32
#define CRYPTOCFG_57_12_RegisterResetValue 0x0
#define CRYPTOCFG_57_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_12_CRYPTOKEY_57_12_BitAddressOffset 0
#define CRYPTOCFG_57_12_CRYPTOKEY_57_12_RegisterSize 32





#define CRYPTOCFG_57_13 (DWC_ufshc_block_BaseAddress + 0x2cb4)
#define CRYPTOCFG_57_13_RegisterSize 32
#define CRYPTOCFG_57_13_RegisterResetValue 0x0
#define CRYPTOCFG_57_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_13_CRYPTOKEY_57_13_BitAddressOffset 0
#define CRYPTOCFG_57_13_CRYPTOKEY_57_13_RegisterSize 32





#define CRYPTOCFG_57_14 (DWC_ufshc_block_BaseAddress + 0x2cb8)
#define CRYPTOCFG_57_14_RegisterSize 32
#define CRYPTOCFG_57_14_RegisterResetValue 0x0
#define CRYPTOCFG_57_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_14_CRYPTOKEY_57_14_BitAddressOffset 0
#define CRYPTOCFG_57_14_CRYPTOKEY_57_14_RegisterSize 32





#define CRYPTOCFG_57_15 (DWC_ufshc_block_BaseAddress + 0x2cbc)
#define CRYPTOCFG_57_15_RegisterSize 32
#define CRYPTOCFG_57_15_RegisterResetValue 0x0
#define CRYPTOCFG_57_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_15_CRYPTOKEY_57_15_BitAddressOffset 0
#define CRYPTOCFG_57_15_CRYPTOKEY_57_15_RegisterSize 32





#define CRYPTOCFG_57_16 (DWC_ufshc_block_BaseAddress + 0x2cc0)
#define CRYPTOCFG_57_16_RegisterSize 32
#define CRYPTOCFG_57_16_RegisterResetValue 0x0
#define CRYPTOCFG_57_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_16_DUSIZE_57_BitAddressOffset 0
#define CRYPTOCFG_57_16_DUSIZE_57_RegisterSize 8



#define CRYPTOCFG_57_16_CAPIDX_57_BitAddressOffset 8
#define CRYPTOCFG_57_16_CAPIDX_57_RegisterSize 8



#define CRYPTOCFG_57_16_CFGE_57_BitAddressOffset 31
#define CRYPTOCFG_57_16_CFGE_57_RegisterSize 1





#define CRYPTOCFG_57_17 (DWC_ufshc_block_BaseAddress + 0x2cc4)
#define CRYPTOCFG_57_17_RegisterSize 32
#define CRYPTOCFG_57_17_RegisterResetValue 0x0
#define CRYPTOCFG_57_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_17_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_17_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_18 (DWC_ufshc_block_BaseAddress + 0x2cc8)
#define CRYPTOCFG_57_18_RegisterSize 32
#define CRYPTOCFG_57_18_RegisterResetValue 0x0
#define CRYPTOCFG_57_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_18_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_18_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_19 (DWC_ufshc_block_BaseAddress + 0x2ccc)
#define CRYPTOCFG_57_19_RegisterSize 32
#define CRYPTOCFG_57_19_RegisterResetValue 0x0
#define CRYPTOCFG_57_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_19_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_19_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_20 (DWC_ufshc_block_BaseAddress + 0x2cd0)
#define CRYPTOCFG_57_20_RegisterSize 32
#define CRYPTOCFG_57_20_RegisterResetValue 0x0
#define CRYPTOCFG_57_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_20_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_20_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_21 (DWC_ufshc_block_BaseAddress + 0x2cd4)
#define CRYPTOCFG_57_21_RegisterSize 32
#define CRYPTOCFG_57_21_RegisterResetValue 0x0
#define CRYPTOCFG_57_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_21_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_21_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_22 (DWC_ufshc_block_BaseAddress + 0x2cd8)
#define CRYPTOCFG_57_22_RegisterSize 32
#define CRYPTOCFG_57_22_RegisterResetValue 0x0
#define CRYPTOCFG_57_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_22_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_22_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_23 (DWC_ufshc_block_BaseAddress + 0x2cdc)
#define CRYPTOCFG_57_23_RegisterSize 32
#define CRYPTOCFG_57_23_RegisterResetValue 0x0
#define CRYPTOCFG_57_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_23_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_23_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_24 (DWC_ufshc_block_BaseAddress + 0x2ce0)
#define CRYPTOCFG_57_24_RegisterSize 32
#define CRYPTOCFG_57_24_RegisterResetValue 0x0
#define CRYPTOCFG_57_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_24_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_24_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_25 (DWC_ufshc_block_BaseAddress + 0x2ce4)
#define CRYPTOCFG_57_25_RegisterSize 32
#define CRYPTOCFG_57_25_RegisterResetValue 0x0
#define CRYPTOCFG_57_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_25_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_25_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_26 (DWC_ufshc_block_BaseAddress + 0x2ce8)
#define CRYPTOCFG_57_26_RegisterSize 32
#define CRYPTOCFG_57_26_RegisterResetValue 0x0
#define CRYPTOCFG_57_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_26_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_26_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_27 (DWC_ufshc_block_BaseAddress + 0x2cec)
#define CRYPTOCFG_57_27_RegisterSize 32
#define CRYPTOCFG_57_27_RegisterResetValue 0x0
#define CRYPTOCFG_57_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_27_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_27_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_28 (DWC_ufshc_block_BaseAddress + 0x2cf0)
#define CRYPTOCFG_57_28_RegisterSize 32
#define CRYPTOCFG_57_28_RegisterResetValue 0x0
#define CRYPTOCFG_57_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_28_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_28_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_29 (DWC_ufshc_block_BaseAddress + 0x2cf4)
#define CRYPTOCFG_57_29_RegisterSize 32
#define CRYPTOCFG_57_29_RegisterResetValue 0x0
#define CRYPTOCFG_57_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_29_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_29_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_30 (DWC_ufshc_block_BaseAddress + 0x2cf8)
#define CRYPTOCFG_57_30_RegisterSize 32
#define CRYPTOCFG_57_30_RegisterResetValue 0x0
#define CRYPTOCFG_57_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_30_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_30_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_57_31 (DWC_ufshc_block_BaseAddress + 0x2cfc)
#define CRYPTOCFG_57_31_RegisterSize 32
#define CRYPTOCFG_57_31_RegisterResetValue 0x0
#define CRYPTOCFG_57_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_57_31_RESERVED_57_BitAddressOffset 0
#define CRYPTOCFG_57_31_RESERVED_57_RegisterSize 32





#define CRYPTOCFG_58_0 (DWC_ufshc_block_BaseAddress + 0x2d00)
#define CRYPTOCFG_58_0_RegisterSize 32
#define CRYPTOCFG_58_0_RegisterResetValue 0x0
#define CRYPTOCFG_58_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_0_CRYPTOKEY_58_0_BitAddressOffset 0
#define CRYPTOCFG_58_0_CRYPTOKEY_58_0_RegisterSize 32





#define CRYPTOCFG_58_1 (DWC_ufshc_block_BaseAddress + 0x2d04)
#define CRYPTOCFG_58_1_RegisterSize 32
#define CRYPTOCFG_58_1_RegisterResetValue 0x0
#define CRYPTOCFG_58_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_1_CRYPTOKEY_58_1_BitAddressOffset 0
#define CRYPTOCFG_58_1_CRYPTOKEY_58_1_RegisterSize 32





#define CRYPTOCFG_58_2 (DWC_ufshc_block_BaseAddress + 0x2d08)
#define CRYPTOCFG_58_2_RegisterSize 32
#define CRYPTOCFG_58_2_RegisterResetValue 0x0
#define CRYPTOCFG_58_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_2_CRYPTOKEY_58_2_BitAddressOffset 0
#define CRYPTOCFG_58_2_CRYPTOKEY_58_2_RegisterSize 32





#define CRYPTOCFG_58_3 (DWC_ufshc_block_BaseAddress + 0x2d0c)
#define CRYPTOCFG_58_3_RegisterSize 32
#define CRYPTOCFG_58_3_RegisterResetValue 0x0
#define CRYPTOCFG_58_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_3_CRYPTOKEY_58_3_BitAddressOffset 0
#define CRYPTOCFG_58_3_CRYPTOKEY_58_3_RegisterSize 32





#define CRYPTOCFG_58_4 (DWC_ufshc_block_BaseAddress + 0x2d10)
#define CRYPTOCFG_58_4_RegisterSize 32
#define CRYPTOCFG_58_4_RegisterResetValue 0x0
#define CRYPTOCFG_58_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_4_CRYPTOKEY_58_4_BitAddressOffset 0
#define CRYPTOCFG_58_4_CRYPTOKEY_58_4_RegisterSize 32





#define CRYPTOCFG_58_5 (DWC_ufshc_block_BaseAddress + 0x2d14)
#define CRYPTOCFG_58_5_RegisterSize 32
#define CRYPTOCFG_58_5_RegisterResetValue 0x0
#define CRYPTOCFG_58_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_5_CRYPTOKEY_58_5_BitAddressOffset 0
#define CRYPTOCFG_58_5_CRYPTOKEY_58_5_RegisterSize 32





#define CRYPTOCFG_58_6 (DWC_ufshc_block_BaseAddress + 0x2d18)
#define CRYPTOCFG_58_6_RegisterSize 32
#define CRYPTOCFG_58_6_RegisterResetValue 0x0
#define CRYPTOCFG_58_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_6_CRYPTOKEY_58_6_BitAddressOffset 0
#define CRYPTOCFG_58_6_CRYPTOKEY_58_6_RegisterSize 32





#define CRYPTOCFG_58_7 (DWC_ufshc_block_BaseAddress + 0x2d1c)
#define CRYPTOCFG_58_7_RegisterSize 32
#define CRYPTOCFG_58_7_RegisterResetValue 0x0
#define CRYPTOCFG_58_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_7_CRYPTOKEY_58_7_BitAddressOffset 0
#define CRYPTOCFG_58_7_CRYPTOKEY_58_7_RegisterSize 32





#define CRYPTOCFG_58_8 (DWC_ufshc_block_BaseAddress + 0x2d20)
#define CRYPTOCFG_58_8_RegisterSize 32
#define CRYPTOCFG_58_8_RegisterResetValue 0x0
#define CRYPTOCFG_58_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_8_CRYPTOKEY_58_8_BitAddressOffset 0
#define CRYPTOCFG_58_8_CRYPTOKEY_58_8_RegisterSize 32





#define CRYPTOCFG_58_9 (DWC_ufshc_block_BaseAddress + 0x2d24)
#define CRYPTOCFG_58_9_RegisterSize 32
#define CRYPTOCFG_58_9_RegisterResetValue 0x0
#define CRYPTOCFG_58_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_9_CRYPTOKEY_58_9_BitAddressOffset 0
#define CRYPTOCFG_58_9_CRYPTOKEY_58_9_RegisterSize 32





#define CRYPTOCFG_58_10 (DWC_ufshc_block_BaseAddress + 0x2d28)
#define CRYPTOCFG_58_10_RegisterSize 32
#define CRYPTOCFG_58_10_RegisterResetValue 0x0
#define CRYPTOCFG_58_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_10_CRYPTOKEY_58_10_BitAddressOffset 0
#define CRYPTOCFG_58_10_CRYPTOKEY_58_10_RegisterSize 32





#define CRYPTOCFG_58_11 (DWC_ufshc_block_BaseAddress + 0x2d2c)
#define CRYPTOCFG_58_11_RegisterSize 32
#define CRYPTOCFG_58_11_RegisterResetValue 0x0
#define CRYPTOCFG_58_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_11_CRYPTOKEY_58_11_BitAddressOffset 0
#define CRYPTOCFG_58_11_CRYPTOKEY_58_11_RegisterSize 32





#define CRYPTOCFG_58_12 (DWC_ufshc_block_BaseAddress + 0x2d30)
#define CRYPTOCFG_58_12_RegisterSize 32
#define CRYPTOCFG_58_12_RegisterResetValue 0x0
#define CRYPTOCFG_58_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_12_CRYPTOKEY_58_12_BitAddressOffset 0
#define CRYPTOCFG_58_12_CRYPTOKEY_58_12_RegisterSize 32





#define CRYPTOCFG_58_13 (DWC_ufshc_block_BaseAddress + 0x2d34)
#define CRYPTOCFG_58_13_RegisterSize 32
#define CRYPTOCFG_58_13_RegisterResetValue 0x0
#define CRYPTOCFG_58_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_13_CRYPTOKEY_58_13_BitAddressOffset 0
#define CRYPTOCFG_58_13_CRYPTOKEY_58_13_RegisterSize 32





#define CRYPTOCFG_58_14 (DWC_ufshc_block_BaseAddress + 0x2d38)
#define CRYPTOCFG_58_14_RegisterSize 32
#define CRYPTOCFG_58_14_RegisterResetValue 0x0
#define CRYPTOCFG_58_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_14_CRYPTOKEY_58_14_BitAddressOffset 0
#define CRYPTOCFG_58_14_CRYPTOKEY_58_14_RegisterSize 32





#define CRYPTOCFG_58_15 (DWC_ufshc_block_BaseAddress + 0x2d3c)
#define CRYPTOCFG_58_15_RegisterSize 32
#define CRYPTOCFG_58_15_RegisterResetValue 0x0
#define CRYPTOCFG_58_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_15_CRYPTOKEY_58_15_BitAddressOffset 0
#define CRYPTOCFG_58_15_CRYPTOKEY_58_15_RegisterSize 32





#define CRYPTOCFG_58_16 (DWC_ufshc_block_BaseAddress + 0x2d40)
#define CRYPTOCFG_58_16_RegisterSize 32
#define CRYPTOCFG_58_16_RegisterResetValue 0x0
#define CRYPTOCFG_58_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_16_DUSIZE_58_BitAddressOffset 0
#define CRYPTOCFG_58_16_DUSIZE_58_RegisterSize 8



#define CRYPTOCFG_58_16_CAPIDX_58_BitAddressOffset 8
#define CRYPTOCFG_58_16_CAPIDX_58_RegisterSize 8



#define CRYPTOCFG_58_16_CFGE_58_BitAddressOffset 31
#define CRYPTOCFG_58_16_CFGE_58_RegisterSize 1





#define CRYPTOCFG_58_17 (DWC_ufshc_block_BaseAddress + 0x2d44)
#define CRYPTOCFG_58_17_RegisterSize 32
#define CRYPTOCFG_58_17_RegisterResetValue 0x0
#define CRYPTOCFG_58_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_17_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_17_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_18 (DWC_ufshc_block_BaseAddress + 0x2d48)
#define CRYPTOCFG_58_18_RegisterSize 32
#define CRYPTOCFG_58_18_RegisterResetValue 0x0
#define CRYPTOCFG_58_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_18_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_18_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_19 (DWC_ufshc_block_BaseAddress + 0x2d4c)
#define CRYPTOCFG_58_19_RegisterSize 32
#define CRYPTOCFG_58_19_RegisterResetValue 0x0
#define CRYPTOCFG_58_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_19_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_19_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_20 (DWC_ufshc_block_BaseAddress + 0x2d50)
#define CRYPTOCFG_58_20_RegisterSize 32
#define CRYPTOCFG_58_20_RegisterResetValue 0x0
#define CRYPTOCFG_58_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_20_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_20_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_21 (DWC_ufshc_block_BaseAddress + 0x2d54)
#define CRYPTOCFG_58_21_RegisterSize 32
#define CRYPTOCFG_58_21_RegisterResetValue 0x0
#define CRYPTOCFG_58_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_21_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_21_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_22 (DWC_ufshc_block_BaseAddress + 0x2d58)
#define CRYPTOCFG_58_22_RegisterSize 32
#define CRYPTOCFG_58_22_RegisterResetValue 0x0
#define CRYPTOCFG_58_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_22_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_22_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_23 (DWC_ufshc_block_BaseAddress + 0x2d5c)
#define CRYPTOCFG_58_23_RegisterSize 32
#define CRYPTOCFG_58_23_RegisterResetValue 0x0
#define CRYPTOCFG_58_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_23_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_23_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_24 (DWC_ufshc_block_BaseAddress + 0x2d60)
#define CRYPTOCFG_58_24_RegisterSize 32
#define CRYPTOCFG_58_24_RegisterResetValue 0x0
#define CRYPTOCFG_58_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_24_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_24_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_25 (DWC_ufshc_block_BaseAddress + 0x2d64)
#define CRYPTOCFG_58_25_RegisterSize 32
#define CRYPTOCFG_58_25_RegisterResetValue 0x0
#define CRYPTOCFG_58_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_25_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_25_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_26 (DWC_ufshc_block_BaseAddress + 0x2d68)
#define CRYPTOCFG_58_26_RegisterSize 32
#define CRYPTOCFG_58_26_RegisterResetValue 0x0
#define CRYPTOCFG_58_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_26_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_26_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_27 (DWC_ufshc_block_BaseAddress + 0x2d6c)
#define CRYPTOCFG_58_27_RegisterSize 32
#define CRYPTOCFG_58_27_RegisterResetValue 0x0
#define CRYPTOCFG_58_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_27_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_27_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_28 (DWC_ufshc_block_BaseAddress + 0x2d70)
#define CRYPTOCFG_58_28_RegisterSize 32
#define CRYPTOCFG_58_28_RegisterResetValue 0x0
#define CRYPTOCFG_58_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_28_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_28_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_29 (DWC_ufshc_block_BaseAddress + 0x2d74)
#define CRYPTOCFG_58_29_RegisterSize 32
#define CRYPTOCFG_58_29_RegisterResetValue 0x0
#define CRYPTOCFG_58_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_29_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_29_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_30 (DWC_ufshc_block_BaseAddress + 0x2d78)
#define CRYPTOCFG_58_30_RegisterSize 32
#define CRYPTOCFG_58_30_RegisterResetValue 0x0
#define CRYPTOCFG_58_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_30_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_30_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_58_31 (DWC_ufshc_block_BaseAddress + 0x2d7c)
#define CRYPTOCFG_58_31_RegisterSize 32
#define CRYPTOCFG_58_31_RegisterResetValue 0x0
#define CRYPTOCFG_58_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_58_31_RESERVED_58_BitAddressOffset 0
#define CRYPTOCFG_58_31_RESERVED_58_RegisterSize 32





#define CRYPTOCFG_59_0 (DWC_ufshc_block_BaseAddress + 0x2d80)
#define CRYPTOCFG_59_0_RegisterSize 32
#define CRYPTOCFG_59_0_RegisterResetValue 0x0
#define CRYPTOCFG_59_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_0_CRYPTOKEY_59_0_BitAddressOffset 0
#define CRYPTOCFG_59_0_CRYPTOKEY_59_0_RegisterSize 32





#define CRYPTOCFG_59_1 (DWC_ufshc_block_BaseAddress + 0x2d84)
#define CRYPTOCFG_59_1_RegisterSize 32
#define CRYPTOCFG_59_1_RegisterResetValue 0x0
#define CRYPTOCFG_59_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_1_CRYPTOKEY_59_1_BitAddressOffset 0
#define CRYPTOCFG_59_1_CRYPTOKEY_59_1_RegisterSize 32





#define CRYPTOCFG_59_2 (DWC_ufshc_block_BaseAddress + 0x2d88)
#define CRYPTOCFG_59_2_RegisterSize 32
#define CRYPTOCFG_59_2_RegisterResetValue 0x0
#define CRYPTOCFG_59_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_2_CRYPTOKEY_59_2_BitAddressOffset 0
#define CRYPTOCFG_59_2_CRYPTOKEY_59_2_RegisterSize 32





#define CRYPTOCFG_59_3 (DWC_ufshc_block_BaseAddress + 0x2d8c)
#define CRYPTOCFG_59_3_RegisterSize 32
#define CRYPTOCFG_59_3_RegisterResetValue 0x0
#define CRYPTOCFG_59_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_3_CRYPTOKEY_59_3_BitAddressOffset 0
#define CRYPTOCFG_59_3_CRYPTOKEY_59_3_RegisterSize 32





#define CRYPTOCFG_59_4 (DWC_ufshc_block_BaseAddress + 0x2d90)
#define CRYPTOCFG_59_4_RegisterSize 32
#define CRYPTOCFG_59_4_RegisterResetValue 0x0
#define CRYPTOCFG_59_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_4_CRYPTOKEY_59_4_BitAddressOffset 0
#define CRYPTOCFG_59_4_CRYPTOKEY_59_4_RegisterSize 32





#define CRYPTOCFG_59_5 (DWC_ufshc_block_BaseAddress + 0x2d94)
#define CRYPTOCFG_59_5_RegisterSize 32
#define CRYPTOCFG_59_5_RegisterResetValue 0x0
#define CRYPTOCFG_59_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_5_CRYPTOKEY_59_5_BitAddressOffset 0
#define CRYPTOCFG_59_5_CRYPTOKEY_59_5_RegisterSize 32





#define CRYPTOCFG_59_6 (DWC_ufshc_block_BaseAddress + 0x2d98)
#define CRYPTOCFG_59_6_RegisterSize 32
#define CRYPTOCFG_59_6_RegisterResetValue 0x0
#define CRYPTOCFG_59_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_6_CRYPTOKEY_59_6_BitAddressOffset 0
#define CRYPTOCFG_59_6_CRYPTOKEY_59_6_RegisterSize 32





#define CRYPTOCFG_59_7 (DWC_ufshc_block_BaseAddress + 0x2d9c)
#define CRYPTOCFG_59_7_RegisterSize 32
#define CRYPTOCFG_59_7_RegisterResetValue 0x0
#define CRYPTOCFG_59_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_7_CRYPTOKEY_59_7_BitAddressOffset 0
#define CRYPTOCFG_59_7_CRYPTOKEY_59_7_RegisterSize 32





#define CRYPTOCFG_59_8 (DWC_ufshc_block_BaseAddress + 0x2da0)
#define CRYPTOCFG_59_8_RegisterSize 32
#define CRYPTOCFG_59_8_RegisterResetValue 0x0
#define CRYPTOCFG_59_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_8_CRYPTOKEY_59_8_BitAddressOffset 0
#define CRYPTOCFG_59_8_CRYPTOKEY_59_8_RegisterSize 32





#define CRYPTOCFG_59_9 (DWC_ufshc_block_BaseAddress + 0x2da4)
#define CRYPTOCFG_59_9_RegisterSize 32
#define CRYPTOCFG_59_9_RegisterResetValue 0x0
#define CRYPTOCFG_59_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_9_CRYPTOKEY_59_9_BitAddressOffset 0
#define CRYPTOCFG_59_9_CRYPTOKEY_59_9_RegisterSize 32





#define CRYPTOCFG_59_10 (DWC_ufshc_block_BaseAddress + 0x2da8)
#define CRYPTOCFG_59_10_RegisterSize 32
#define CRYPTOCFG_59_10_RegisterResetValue 0x0
#define CRYPTOCFG_59_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_10_CRYPTOKEY_59_10_BitAddressOffset 0
#define CRYPTOCFG_59_10_CRYPTOKEY_59_10_RegisterSize 32





#define CRYPTOCFG_59_11 (DWC_ufshc_block_BaseAddress + 0x2dac)
#define CRYPTOCFG_59_11_RegisterSize 32
#define CRYPTOCFG_59_11_RegisterResetValue 0x0
#define CRYPTOCFG_59_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_11_CRYPTOKEY_59_11_BitAddressOffset 0
#define CRYPTOCFG_59_11_CRYPTOKEY_59_11_RegisterSize 32





#define CRYPTOCFG_59_12 (DWC_ufshc_block_BaseAddress + 0x2db0)
#define CRYPTOCFG_59_12_RegisterSize 32
#define CRYPTOCFG_59_12_RegisterResetValue 0x0
#define CRYPTOCFG_59_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_12_CRYPTOKEY_59_12_BitAddressOffset 0
#define CRYPTOCFG_59_12_CRYPTOKEY_59_12_RegisterSize 32





#define CRYPTOCFG_59_13 (DWC_ufshc_block_BaseAddress + 0x2db4)
#define CRYPTOCFG_59_13_RegisterSize 32
#define CRYPTOCFG_59_13_RegisterResetValue 0x0
#define CRYPTOCFG_59_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_13_CRYPTOKEY_59_13_BitAddressOffset 0
#define CRYPTOCFG_59_13_CRYPTOKEY_59_13_RegisterSize 32





#define CRYPTOCFG_59_14 (DWC_ufshc_block_BaseAddress + 0x2db8)
#define CRYPTOCFG_59_14_RegisterSize 32
#define CRYPTOCFG_59_14_RegisterResetValue 0x0
#define CRYPTOCFG_59_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_14_CRYPTOKEY_59_14_BitAddressOffset 0
#define CRYPTOCFG_59_14_CRYPTOKEY_59_14_RegisterSize 32





#define CRYPTOCFG_59_15 (DWC_ufshc_block_BaseAddress + 0x2dbc)
#define CRYPTOCFG_59_15_RegisterSize 32
#define CRYPTOCFG_59_15_RegisterResetValue 0x0
#define CRYPTOCFG_59_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_15_CRYPTOKEY_59_15_BitAddressOffset 0
#define CRYPTOCFG_59_15_CRYPTOKEY_59_15_RegisterSize 32





#define CRYPTOCFG_59_16 (DWC_ufshc_block_BaseAddress + 0x2dc0)
#define CRYPTOCFG_59_16_RegisterSize 32
#define CRYPTOCFG_59_16_RegisterResetValue 0x0
#define CRYPTOCFG_59_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_16_DUSIZE_59_BitAddressOffset 0
#define CRYPTOCFG_59_16_DUSIZE_59_RegisterSize 8



#define CRYPTOCFG_59_16_CAPIDX_59_BitAddressOffset 8
#define CRYPTOCFG_59_16_CAPIDX_59_RegisterSize 8



#define CRYPTOCFG_59_16_CFGE_59_BitAddressOffset 31
#define CRYPTOCFG_59_16_CFGE_59_RegisterSize 1





#define CRYPTOCFG_59_17 (DWC_ufshc_block_BaseAddress + 0x2dc4)
#define CRYPTOCFG_59_17_RegisterSize 32
#define CRYPTOCFG_59_17_RegisterResetValue 0x0
#define CRYPTOCFG_59_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_17_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_17_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_18 (DWC_ufshc_block_BaseAddress + 0x2dc8)
#define CRYPTOCFG_59_18_RegisterSize 32
#define CRYPTOCFG_59_18_RegisterResetValue 0x0
#define CRYPTOCFG_59_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_18_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_18_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_19 (DWC_ufshc_block_BaseAddress + 0x2dcc)
#define CRYPTOCFG_59_19_RegisterSize 32
#define CRYPTOCFG_59_19_RegisterResetValue 0x0
#define CRYPTOCFG_59_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_19_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_19_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_20 (DWC_ufshc_block_BaseAddress + 0x2dd0)
#define CRYPTOCFG_59_20_RegisterSize 32
#define CRYPTOCFG_59_20_RegisterResetValue 0x0
#define CRYPTOCFG_59_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_20_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_20_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_21 (DWC_ufshc_block_BaseAddress + 0x2dd4)
#define CRYPTOCFG_59_21_RegisterSize 32
#define CRYPTOCFG_59_21_RegisterResetValue 0x0
#define CRYPTOCFG_59_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_21_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_21_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_22 (DWC_ufshc_block_BaseAddress + 0x2dd8)
#define CRYPTOCFG_59_22_RegisterSize 32
#define CRYPTOCFG_59_22_RegisterResetValue 0x0
#define CRYPTOCFG_59_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_22_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_22_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_23 (DWC_ufshc_block_BaseAddress + 0x2ddc)
#define CRYPTOCFG_59_23_RegisterSize 32
#define CRYPTOCFG_59_23_RegisterResetValue 0x0
#define CRYPTOCFG_59_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_23_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_23_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_24 (DWC_ufshc_block_BaseAddress + 0x2de0)
#define CRYPTOCFG_59_24_RegisterSize 32
#define CRYPTOCFG_59_24_RegisterResetValue 0x0
#define CRYPTOCFG_59_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_24_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_24_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_25 (DWC_ufshc_block_BaseAddress + 0x2de4)
#define CRYPTOCFG_59_25_RegisterSize 32
#define CRYPTOCFG_59_25_RegisterResetValue 0x0
#define CRYPTOCFG_59_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_25_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_25_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_26 (DWC_ufshc_block_BaseAddress + 0x2de8)
#define CRYPTOCFG_59_26_RegisterSize 32
#define CRYPTOCFG_59_26_RegisterResetValue 0x0
#define CRYPTOCFG_59_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_26_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_26_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_27 (DWC_ufshc_block_BaseAddress + 0x2dec)
#define CRYPTOCFG_59_27_RegisterSize 32
#define CRYPTOCFG_59_27_RegisterResetValue 0x0
#define CRYPTOCFG_59_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_27_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_27_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_28 (DWC_ufshc_block_BaseAddress + 0x2df0)
#define CRYPTOCFG_59_28_RegisterSize 32
#define CRYPTOCFG_59_28_RegisterResetValue 0x0
#define CRYPTOCFG_59_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_28_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_28_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_29 (DWC_ufshc_block_BaseAddress + 0x2df4)
#define CRYPTOCFG_59_29_RegisterSize 32
#define CRYPTOCFG_59_29_RegisterResetValue 0x0
#define CRYPTOCFG_59_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_29_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_29_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_30 (DWC_ufshc_block_BaseAddress + 0x2df8)
#define CRYPTOCFG_59_30_RegisterSize 32
#define CRYPTOCFG_59_30_RegisterResetValue 0x0
#define CRYPTOCFG_59_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_30_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_30_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_59_31 (DWC_ufshc_block_BaseAddress + 0x2dfc)
#define CRYPTOCFG_59_31_RegisterSize 32
#define CRYPTOCFG_59_31_RegisterResetValue 0x0
#define CRYPTOCFG_59_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_59_31_RESERVED_59_BitAddressOffset 0
#define CRYPTOCFG_59_31_RESERVED_59_RegisterSize 32





#define CRYPTOCFG_60_0 (DWC_ufshc_block_BaseAddress + 0x2e00)
#define CRYPTOCFG_60_0_RegisterSize 32
#define CRYPTOCFG_60_0_RegisterResetValue 0x0
#define CRYPTOCFG_60_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_0_CRYPTOKEY_60_0_BitAddressOffset 0
#define CRYPTOCFG_60_0_CRYPTOKEY_60_0_RegisterSize 32





#define CRYPTOCFG_60_1 (DWC_ufshc_block_BaseAddress + 0x2e04)
#define CRYPTOCFG_60_1_RegisterSize 32
#define CRYPTOCFG_60_1_RegisterResetValue 0x0
#define CRYPTOCFG_60_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_1_CRYPTOKEY_60_1_BitAddressOffset 0
#define CRYPTOCFG_60_1_CRYPTOKEY_60_1_RegisterSize 32





#define CRYPTOCFG_60_2 (DWC_ufshc_block_BaseAddress + 0x2e08)
#define CRYPTOCFG_60_2_RegisterSize 32
#define CRYPTOCFG_60_2_RegisterResetValue 0x0
#define CRYPTOCFG_60_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_2_CRYPTOKEY_60_2_BitAddressOffset 0
#define CRYPTOCFG_60_2_CRYPTOKEY_60_2_RegisterSize 32





#define CRYPTOCFG_60_3 (DWC_ufshc_block_BaseAddress + 0x2e0c)
#define CRYPTOCFG_60_3_RegisterSize 32
#define CRYPTOCFG_60_3_RegisterResetValue 0x0
#define CRYPTOCFG_60_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_3_CRYPTOKEY_60_3_BitAddressOffset 0
#define CRYPTOCFG_60_3_CRYPTOKEY_60_3_RegisterSize 32





#define CRYPTOCFG_60_4 (DWC_ufshc_block_BaseAddress + 0x2e10)
#define CRYPTOCFG_60_4_RegisterSize 32
#define CRYPTOCFG_60_4_RegisterResetValue 0x0
#define CRYPTOCFG_60_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_4_CRYPTOKEY_60_4_BitAddressOffset 0
#define CRYPTOCFG_60_4_CRYPTOKEY_60_4_RegisterSize 32





#define CRYPTOCFG_60_5 (DWC_ufshc_block_BaseAddress + 0x2e14)
#define CRYPTOCFG_60_5_RegisterSize 32
#define CRYPTOCFG_60_5_RegisterResetValue 0x0
#define CRYPTOCFG_60_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_5_CRYPTOKEY_60_5_BitAddressOffset 0
#define CRYPTOCFG_60_5_CRYPTOKEY_60_5_RegisterSize 32





#define CRYPTOCFG_60_6 (DWC_ufshc_block_BaseAddress + 0x2e18)
#define CRYPTOCFG_60_6_RegisterSize 32
#define CRYPTOCFG_60_6_RegisterResetValue 0x0
#define CRYPTOCFG_60_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_6_CRYPTOKEY_60_6_BitAddressOffset 0
#define CRYPTOCFG_60_6_CRYPTOKEY_60_6_RegisterSize 32





#define CRYPTOCFG_60_7 (DWC_ufshc_block_BaseAddress + 0x2e1c)
#define CRYPTOCFG_60_7_RegisterSize 32
#define CRYPTOCFG_60_7_RegisterResetValue 0x0
#define CRYPTOCFG_60_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_7_CRYPTOKEY_60_7_BitAddressOffset 0
#define CRYPTOCFG_60_7_CRYPTOKEY_60_7_RegisterSize 32





#define CRYPTOCFG_60_8 (DWC_ufshc_block_BaseAddress + 0x2e20)
#define CRYPTOCFG_60_8_RegisterSize 32
#define CRYPTOCFG_60_8_RegisterResetValue 0x0
#define CRYPTOCFG_60_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_8_CRYPTOKEY_60_8_BitAddressOffset 0
#define CRYPTOCFG_60_8_CRYPTOKEY_60_8_RegisterSize 32





#define CRYPTOCFG_60_9 (DWC_ufshc_block_BaseAddress + 0x2e24)
#define CRYPTOCFG_60_9_RegisterSize 32
#define CRYPTOCFG_60_9_RegisterResetValue 0x0
#define CRYPTOCFG_60_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_9_CRYPTOKEY_60_9_BitAddressOffset 0
#define CRYPTOCFG_60_9_CRYPTOKEY_60_9_RegisterSize 32





#define CRYPTOCFG_60_10 (DWC_ufshc_block_BaseAddress + 0x2e28)
#define CRYPTOCFG_60_10_RegisterSize 32
#define CRYPTOCFG_60_10_RegisterResetValue 0x0
#define CRYPTOCFG_60_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_10_CRYPTOKEY_60_10_BitAddressOffset 0
#define CRYPTOCFG_60_10_CRYPTOKEY_60_10_RegisterSize 32





#define CRYPTOCFG_60_11 (DWC_ufshc_block_BaseAddress + 0x2e2c)
#define CRYPTOCFG_60_11_RegisterSize 32
#define CRYPTOCFG_60_11_RegisterResetValue 0x0
#define CRYPTOCFG_60_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_11_CRYPTOKEY_60_11_BitAddressOffset 0
#define CRYPTOCFG_60_11_CRYPTOKEY_60_11_RegisterSize 32





#define CRYPTOCFG_60_12 (DWC_ufshc_block_BaseAddress + 0x2e30)
#define CRYPTOCFG_60_12_RegisterSize 32
#define CRYPTOCFG_60_12_RegisterResetValue 0x0
#define CRYPTOCFG_60_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_12_CRYPTOKEY_60_12_BitAddressOffset 0
#define CRYPTOCFG_60_12_CRYPTOKEY_60_12_RegisterSize 32





#define CRYPTOCFG_60_13 (DWC_ufshc_block_BaseAddress + 0x2e34)
#define CRYPTOCFG_60_13_RegisterSize 32
#define CRYPTOCFG_60_13_RegisterResetValue 0x0
#define CRYPTOCFG_60_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_13_CRYPTOKEY_60_13_BitAddressOffset 0
#define CRYPTOCFG_60_13_CRYPTOKEY_60_13_RegisterSize 32





#define CRYPTOCFG_60_14 (DWC_ufshc_block_BaseAddress + 0x2e38)
#define CRYPTOCFG_60_14_RegisterSize 32
#define CRYPTOCFG_60_14_RegisterResetValue 0x0
#define CRYPTOCFG_60_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_14_CRYPTOKEY_60_14_BitAddressOffset 0
#define CRYPTOCFG_60_14_CRYPTOKEY_60_14_RegisterSize 32





#define CRYPTOCFG_60_15 (DWC_ufshc_block_BaseAddress + 0x2e3c)
#define CRYPTOCFG_60_15_RegisterSize 32
#define CRYPTOCFG_60_15_RegisterResetValue 0x0
#define CRYPTOCFG_60_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_15_CRYPTOKEY_60_15_BitAddressOffset 0
#define CRYPTOCFG_60_15_CRYPTOKEY_60_15_RegisterSize 32





#define CRYPTOCFG_60_16 (DWC_ufshc_block_BaseAddress + 0x2e40)
#define CRYPTOCFG_60_16_RegisterSize 32
#define CRYPTOCFG_60_16_RegisterResetValue 0x0
#define CRYPTOCFG_60_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_16_DUSIZE_60_BitAddressOffset 0
#define CRYPTOCFG_60_16_DUSIZE_60_RegisterSize 8



#define CRYPTOCFG_60_16_CAPIDX_60_BitAddressOffset 8
#define CRYPTOCFG_60_16_CAPIDX_60_RegisterSize 8



#define CRYPTOCFG_60_16_CFGE_60_BitAddressOffset 31
#define CRYPTOCFG_60_16_CFGE_60_RegisterSize 1





#define CRYPTOCFG_60_17 (DWC_ufshc_block_BaseAddress + 0x2e44)
#define CRYPTOCFG_60_17_RegisterSize 32
#define CRYPTOCFG_60_17_RegisterResetValue 0x0
#define CRYPTOCFG_60_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_17_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_17_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_18 (DWC_ufshc_block_BaseAddress + 0x2e48)
#define CRYPTOCFG_60_18_RegisterSize 32
#define CRYPTOCFG_60_18_RegisterResetValue 0x0
#define CRYPTOCFG_60_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_18_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_18_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_19 (DWC_ufshc_block_BaseAddress + 0x2e4c)
#define CRYPTOCFG_60_19_RegisterSize 32
#define CRYPTOCFG_60_19_RegisterResetValue 0x0
#define CRYPTOCFG_60_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_19_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_19_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_20 (DWC_ufshc_block_BaseAddress + 0x2e50)
#define CRYPTOCFG_60_20_RegisterSize 32
#define CRYPTOCFG_60_20_RegisterResetValue 0x0
#define CRYPTOCFG_60_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_20_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_20_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_21 (DWC_ufshc_block_BaseAddress + 0x2e54)
#define CRYPTOCFG_60_21_RegisterSize 32
#define CRYPTOCFG_60_21_RegisterResetValue 0x0
#define CRYPTOCFG_60_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_21_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_21_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_22 (DWC_ufshc_block_BaseAddress + 0x2e58)
#define CRYPTOCFG_60_22_RegisterSize 32
#define CRYPTOCFG_60_22_RegisterResetValue 0x0
#define CRYPTOCFG_60_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_22_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_22_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_23 (DWC_ufshc_block_BaseAddress + 0x2e5c)
#define CRYPTOCFG_60_23_RegisterSize 32
#define CRYPTOCFG_60_23_RegisterResetValue 0x0
#define CRYPTOCFG_60_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_23_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_23_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_24 (DWC_ufshc_block_BaseAddress + 0x2e60)
#define CRYPTOCFG_60_24_RegisterSize 32
#define CRYPTOCFG_60_24_RegisterResetValue 0x0
#define CRYPTOCFG_60_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_24_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_24_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_25 (DWC_ufshc_block_BaseAddress + 0x2e64)
#define CRYPTOCFG_60_25_RegisterSize 32
#define CRYPTOCFG_60_25_RegisterResetValue 0x0
#define CRYPTOCFG_60_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_25_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_25_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_26 (DWC_ufshc_block_BaseAddress + 0x2e68)
#define CRYPTOCFG_60_26_RegisterSize 32
#define CRYPTOCFG_60_26_RegisterResetValue 0x0
#define CRYPTOCFG_60_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_26_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_26_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_27 (DWC_ufshc_block_BaseAddress + 0x2e6c)
#define CRYPTOCFG_60_27_RegisterSize 32
#define CRYPTOCFG_60_27_RegisterResetValue 0x0
#define CRYPTOCFG_60_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_27_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_27_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_28 (DWC_ufshc_block_BaseAddress + 0x2e70)
#define CRYPTOCFG_60_28_RegisterSize 32
#define CRYPTOCFG_60_28_RegisterResetValue 0x0
#define CRYPTOCFG_60_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_28_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_28_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_29 (DWC_ufshc_block_BaseAddress + 0x2e74)
#define CRYPTOCFG_60_29_RegisterSize 32
#define CRYPTOCFG_60_29_RegisterResetValue 0x0
#define CRYPTOCFG_60_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_29_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_29_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_30 (DWC_ufshc_block_BaseAddress + 0x2e78)
#define CRYPTOCFG_60_30_RegisterSize 32
#define CRYPTOCFG_60_30_RegisterResetValue 0x0
#define CRYPTOCFG_60_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_30_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_30_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_60_31 (DWC_ufshc_block_BaseAddress + 0x2e7c)
#define CRYPTOCFG_60_31_RegisterSize 32
#define CRYPTOCFG_60_31_RegisterResetValue 0x0
#define CRYPTOCFG_60_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_60_31_RESERVED_60_BitAddressOffset 0
#define CRYPTOCFG_60_31_RESERVED_60_RegisterSize 32





#define CRYPTOCFG_61_0 (DWC_ufshc_block_BaseAddress + 0x2e80)
#define CRYPTOCFG_61_0_RegisterSize 32
#define CRYPTOCFG_61_0_RegisterResetValue 0x0
#define CRYPTOCFG_61_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_0_CRYPTOKEY_61_0_BitAddressOffset 0
#define CRYPTOCFG_61_0_CRYPTOKEY_61_0_RegisterSize 32





#define CRYPTOCFG_61_1 (DWC_ufshc_block_BaseAddress + 0x2e84)
#define CRYPTOCFG_61_1_RegisterSize 32
#define CRYPTOCFG_61_1_RegisterResetValue 0x0
#define CRYPTOCFG_61_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_1_CRYPTOKEY_61_1_BitAddressOffset 0
#define CRYPTOCFG_61_1_CRYPTOKEY_61_1_RegisterSize 32





#define CRYPTOCFG_61_2 (DWC_ufshc_block_BaseAddress + 0x2e88)
#define CRYPTOCFG_61_2_RegisterSize 32
#define CRYPTOCFG_61_2_RegisterResetValue 0x0
#define CRYPTOCFG_61_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_2_CRYPTOKEY_61_2_BitAddressOffset 0
#define CRYPTOCFG_61_2_CRYPTOKEY_61_2_RegisterSize 32





#define CRYPTOCFG_61_3 (DWC_ufshc_block_BaseAddress + 0x2e8c)
#define CRYPTOCFG_61_3_RegisterSize 32
#define CRYPTOCFG_61_3_RegisterResetValue 0x0
#define CRYPTOCFG_61_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_3_CRYPTOKEY_61_3_BitAddressOffset 0
#define CRYPTOCFG_61_3_CRYPTOKEY_61_3_RegisterSize 32





#define CRYPTOCFG_61_4 (DWC_ufshc_block_BaseAddress + 0x2e90)
#define CRYPTOCFG_61_4_RegisterSize 32
#define CRYPTOCFG_61_4_RegisterResetValue 0x0
#define CRYPTOCFG_61_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_4_CRYPTOKEY_61_4_BitAddressOffset 0
#define CRYPTOCFG_61_4_CRYPTOKEY_61_4_RegisterSize 32





#define CRYPTOCFG_61_5 (DWC_ufshc_block_BaseAddress + 0x2e94)
#define CRYPTOCFG_61_5_RegisterSize 32
#define CRYPTOCFG_61_5_RegisterResetValue 0x0
#define CRYPTOCFG_61_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_5_CRYPTOKEY_61_5_BitAddressOffset 0
#define CRYPTOCFG_61_5_CRYPTOKEY_61_5_RegisterSize 32





#define CRYPTOCFG_61_6 (DWC_ufshc_block_BaseAddress + 0x2e98)
#define CRYPTOCFG_61_6_RegisterSize 32
#define CRYPTOCFG_61_6_RegisterResetValue 0x0
#define CRYPTOCFG_61_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_6_CRYPTOKEY_61_6_BitAddressOffset 0
#define CRYPTOCFG_61_6_CRYPTOKEY_61_6_RegisterSize 32





#define CRYPTOCFG_61_7 (DWC_ufshc_block_BaseAddress + 0x2e9c)
#define CRYPTOCFG_61_7_RegisterSize 32
#define CRYPTOCFG_61_7_RegisterResetValue 0x0
#define CRYPTOCFG_61_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_7_CRYPTOKEY_61_7_BitAddressOffset 0
#define CRYPTOCFG_61_7_CRYPTOKEY_61_7_RegisterSize 32





#define CRYPTOCFG_61_8 (DWC_ufshc_block_BaseAddress + 0x2ea0)
#define CRYPTOCFG_61_8_RegisterSize 32
#define CRYPTOCFG_61_8_RegisterResetValue 0x0
#define CRYPTOCFG_61_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_8_CRYPTOKEY_61_8_BitAddressOffset 0
#define CRYPTOCFG_61_8_CRYPTOKEY_61_8_RegisterSize 32





#define CRYPTOCFG_61_9 (DWC_ufshc_block_BaseAddress + 0x2ea4)
#define CRYPTOCFG_61_9_RegisterSize 32
#define CRYPTOCFG_61_9_RegisterResetValue 0x0
#define CRYPTOCFG_61_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_9_CRYPTOKEY_61_9_BitAddressOffset 0
#define CRYPTOCFG_61_9_CRYPTOKEY_61_9_RegisterSize 32





#define CRYPTOCFG_61_10 (DWC_ufshc_block_BaseAddress + 0x2ea8)
#define CRYPTOCFG_61_10_RegisterSize 32
#define CRYPTOCFG_61_10_RegisterResetValue 0x0
#define CRYPTOCFG_61_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_10_CRYPTOKEY_61_10_BitAddressOffset 0
#define CRYPTOCFG_61_10_CRYPTOKEY_61_10_RegisterSize 32





#define CRYPTOCFG_61_11 (DWC_ufshc_block_BaseAddress + 0x2eac)
#define CRYPTOCFG_61_11_RegisterSize 32
#define CRYPTOCFG_61_11_RegisterResetValue 0x0
#define CRYPTOCFG_61_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_11_CRYPTOKEY_61_11_BitAddressOffset 0
#define CRYPTOCFG_61_11_CRYPTOKEY_61_11_RegisterSize 32





#define CRYPTOCFG_61_12 (DWC_ufshc_block_BaseAddress + 0x2eb0)
#define CRYPTOCFG_61_12_RegisterSize 32
#define CRYPTOCFG_61_12_RegisterResetValue 0x0
#define CRYPTOCFG_61_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_12_CRYPTOKEY_61_12_BitAddressOffset 0
#define CRYPTOCFG_61_12_CRYPTOKEY_61_12_RegisterSize 32





#define CRYPTOCFG_61_13 (DWC_ufshc_block_BaseAddress + 0x2eb4)
#define CRYPTOCFG_61_13_RegisterSize 32
#define CRYPTOCFG_61_13_RegisterResetValue 0x0
#define CRYPTOCFG_61_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_13_CRYPTOKEY_61_13_BitAddressOffset 0
#define CRYPTOCFG_61_13_CRYPTOKEY_61_13_RegisterSize 32





#define CRYPTOCFG_61_14 (DWC_ufshc_block_BaseAddress + 0x2eb8)
#define CRYPTOCFG_61_14_RegisterSize 32
#define CRYPTOCFG_61_14_RegisterResetValue 0x0
#define CRYPTOCFG_61_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_14_CRYPTOKEY_61_14_BitAddressOffset 0
#define CRYPTOCFG_61_14_CRYPTOKEY_61_14_RegisterSize 32





#define CRYPTOCFG_61_15 (DWC_ufshc_block_BaseAddress + 0x2ebc)
#define CRYPTOCFG_61_15_RegisterSize 32
#define CRYPTOCFG_61_15_RegisterResetValue 0x0
#define CRYPTOCFG_61_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_15_CRYPTOKEY_61_15_BitAddressOffset 0
#define CRYPTOCFG_61_15_CRYPTOKEY_61_15_RegisterSize 32





#define CRYPTOCFG_61_16 (DWC_ufshc_block_BaseAddress + 0x2ec0)
#define CRYPTOCFG_61_16_RegisterSize 32
#define CRYPTOCFG_61_16_RegisterResetValue 0x0
#define CRYPTOCFG_61_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_16_DUSIZE_61_BitAddressOffset 0
#define CRYPTOCFG_61_16_DUSIZE_61_RegisterSize 8



#define CRYPTOCFG_61_16_CAPIDX_61_BitAddressOffset 8
#define CRYPTOCFG_61_16_CAPIDX_61_RegisterSize 8



#define CRYPTOCFG_61_16_CFGE_61_BitAddressOffset 31
#define CRYPTOCFG_61_16_CFGE_61_RegisterSize 1





#define CRYPTOCFG_61_17 (DWC_ufshc_block_BaseAddress + 0x2ec4)
#define CRYPTOCFG_61_17_RegisterSize 32
#define CRYPTOCFG_61_17_RegisterResetValue 0x0
#define CRYPTOCFG_61_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_17_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_17_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_18 (DWC_ufshc_block_BaseAddress + 0x2ec8)
#define CRYPTOCFG_61_18_RegisterSize 32
#define CRYPTOCFG_61_18_RegisterResetValue 0x0
#define CRYPTOCFG_61_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_18_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_18_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_19 (DWC_ufshc_block_BaseAddress + 0x2ecc)
#define CRYPTOCFG_61_19_RegisterSize 32
#define CRYPTOCFG_61_19_RegisterResetValue 0x0
#define CRYPTOCFG_61_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_19_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_19_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_20 (DWC_ufshc_block_BaseAddress + 0x2ed0)
#define CRYPTOCFG_61_20_RegisterSize 32
#define CRYPTOCFG_61_20_RegisterResetValue 0x0
#define CRYPTOCFG_61_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_20_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_20_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_21 (DWC_ufshc_block_BaseAddress + 0x2ed4)
#define CRYPTOCFG_61_21_RegisterSize 32
#define CRYPTOCFG_61_21_RegisterResetValue 0x0
#define CRYPTOCFG_61_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_21_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_21_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_22 (DWC_ufshc_block_BaseAddress + 0x2ed8)
#define CRYPTOCFG_61_22_RegisterSize 32
#define CRYPTOCFG_61_22_RegisterResetValue 0x0
#define CRYPTOCFG_61_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_22_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_22_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_23 (DWC_ufshc_block_BaseAddress + 0x2edc)
#define CRYPTOCFG_61_23_RegisterSize 32
#define CRYPTOCFG_61_23_RegisterResetValue 0x0
#define CRYPTOCFG_61_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_23_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_23_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_24 (DWC_ufshc_block_BaseAddress + 0x2ee0)
#define CRYPTOCFG_61_24_RegisterSize 32
#define CRYPTOCFG_61_24_RegisterResetValue 0x0
#define CRYPTOCFG_61_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_24_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_24_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_25 (DWC_ufshc_block_BaseAddress + 0x2ee4)
#define CRYPTOCFG_61_25_RegisterSize 32
#define CRYPTOCFG_61_25_RegisterResetValue 0x0
#define CRYPTOCFG_61_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_25_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_25_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_26 (DWC_ufshc_block_BaseAddress + 0x2ee8)
#define CRYPTOCFG_61_26_RegisterSize 32
#define CRYPTOCFG_61_26_RegisterResetValue 0x0
#define CRYPTOCFG_61_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_26_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_26_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_27 (DWC_ufshc_block_BaseAddress + 0x2eec)
#define CRYPTOCFG_61_27_RegisterSize 32
#define CRYPTOCFG_61_27_RegisterResetValue 0x0
#define CRYPTOCFG_61_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_27_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_27_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_28 (DWC_ufshc_block_BaseAddress + 0x2ef0)
#define CRYPTOCFG_61_28_RegisterSize 32
#define CRYPTOCFG_61_28_RegisterResetValue 0x0
#define CRYPTOCFG_61_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_28_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_28_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_29 (DWC_ufshc_block_BaseAddress + 0x2ef4)
#define CRYPTOCFG_61_29_RegisterSize 32
#define CRYPTOCFG_61_29_RegisterResetValue 0x0
#define CRYPTOCFG_61_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_29_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_29_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_30 (DWC_ufshc_block_BaseAddress + 0x2ef8)
#define CRYPTOCFG_61_30_RegisterSize 32
#define CRYPTOCFG_61_30_RegisterResetValue 0x0
#define CRYPTOCFG_61_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_30_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_30_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_61_31 (DWC_ufshc_block_BaseAddress + 0x2efc)
#define CRYPTOCFG_61_31_RegisterSize 32
#define CRYPTOCFG_61_31_RegisterResetValue 0x0
#define CRYPTOCFG_61_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_61_31_RESERVED_61_BitAddressOffset 0
#define CRYPTOCFG_61_31_RESERVED_61_RegisterSize 32





#define CRYPTOCFG_62_0 (DWC_ufshc_block_BaseAddress + 0x2f00)
#define CRYPTOCFG_62_0_RegisterSize 32
#define CRYPTOCFG_62_0_RegisterResetValue 0x0
#define CRYPTOCFG_62_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_0_CRYPTOKEY_62_0_BitAddressOffset 0
#define CRYPTOCFG_62_0_CRYPTOKEY_62_0_RegisterSize 32





#define CRYPTOCFG_62_1 (DWC_ufshc_block_BaseAddress + 0x2f04)
#define CRYPTOCFG_62_1_RegisterSize 32
#define CRYPTOCFG_62_1_RegisterResetValue 0x0
#define CRYPTOCFG_62_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_1_CRYPTOKEY_62_1_BitAddressOffset 0
#define CRYPTOCFG_62_1_CRYPTOKEY_62_1_RegisterSize 32





#define CRYPTOCFG_62_2 (DWC_ufshc_block_BaseAddress + 0x2f08)
#define CRYPTOCFG_62_2_RegisterSize 32
#define CRYPTOCFG_62_2_RegisterResetValue 0x0
#define CRYPTOCFG_62_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_2_CRYPTOKEY_62_2_BitAddressOffset 0
#define CRYPTOCFG_62_2_CRYPTOKEY_62_2_RegisterSize 32





#define CRYPTOCFG_62_3 (DWC_ufshc_block_BaseAddress + 0x2f0c)
#define CRYPTOCFG_62_3_RegisterSize 32
#define CRYPTOCFG_62_3_RegisterResetValue 0x0
#define CRYPTOCFG_62_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_3_CRYPTOKEY_62_3_BitAddressOffset 0
#define CRYPTOCFG_62_3_CRYPTOKEY_62_3_RegisterSize 32





#define CRYPTOCFG_62_4 (DWC_ufshc_block_BaseAddress + 0x2f10)
#define CRYPTOCFG_62_4_RegisterSize 32
#define CRYPTOCFG_62_4_RegisterResetValue 0x0
#define CRYPTOCFG_62_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_4_CRYPTOKEY_62_4_BitAddressOffset 0
#define CRYPTOCFG_62_4_CRYPTOKEY_62_4_RegisterSize 32





#define CRYPTOCFG_62_5 (DWC_ufshc_block_BaseAddress + 0x2f14)
#define CRYPTOCFG_62_5_RegisterSize 32
#define CRYPTOCFG_62_5_RegisterResetValue 0x0
#define CRYPTOCFG_62_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_5_CRYPTOKEY_62_5_BitAddressOffset 0
#define CRYPTOCFG_62_5_CRYPTOKEY_62_5_RegisterSize 32





#define CRYPTOCFG_62_6 (DWC_ufshc_block_BaseAddress + 0x2f18)
#define CRYPTOCFG_62_6_RegisterSize 32
#define CRYPTOCFG_62_6_RegisterResetValue 0x0
#define CRYPTOCFG_62_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_6_CRYPTOKEY_62_6_BitAddressOffset 0
#define CRYPTOCFG_62_6_CRYPTOKEY_62_6_RegisterSize 32





#define CRYPTOCFG_62_7 (DWC_ufshc_block_BaseAddress + 0x2f1c)
#define CRYPTOCFG_62_7_RegisterSize 32
#define CRYPTOCFG_62_7_RegisterResetValue 0x0
#define CRYPTOCFG_62_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_7_CRYPTOKEY_62_7_BitAddressOffset 0
#define CRYPTOCFG_62_7_CRYPTOKEY_62_7_RegisterSize 32





#define CRYPTOCFG_62_8 (DWC_ufshc_block_BaseAddress + 0x2f20)
#define CRYPTOCFG_62_8_RegisterSize 32
#define CRYPTOCFG_62_8_RegisterResetValue 0x0
#define CRYPTOCFG_62_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_8_CRYPTOKEY_62_8_BitAddressOffset 0
#define CRYPTOCFG_62_8_CRYPTOKEY_62_8_RegisterSize 32





#define CRYPTOCFG_62_9 (DWC_ufshc_block_BaseAddress + 0x2f24)
#define CRYPTOCFG_62_9_RegisterSize 32
#define CRYPTOCFG_62_9_RegisterResetValue 0x0
#define CRYPTOCFG_62_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_9_CRYPTOKEY_62_9_BitAddressOffset 0
#define CRYPTOCFG_62_9_CRYPTOKEY_62_9_RegisterSize 32





#define CRYPTOCFG_62_10 (DWC_ufshc_block_BaseAddress + 0x2f28)
#define CRYPTOCFG_62_10_RegisterSize 32
#define CRYPTOCFG_62_10_RegisterResetValue 0x0
#define CRYPTOCFG_62_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_10_CRYPTOKEY_62_10_BitAddressOffset 0
#define CRYPTOCFG_62_10_CRYPTOKEY_62_10_RegisterSize 32





#define CRYPTOCFG_62_11 (DWC_ufshc_block_BaseAddress + 0x2f2c)
#define CRYPTOCFG_62_11_RegisterSize 32
#define CRYPTOCFG_62_11_RegisterResetValue 0x0
#define CRYPTOCFG_62_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_11_CRYPTOKEY_62_11_BitAddressOffset 0
#define CRYPTOCFG_62_11_CRYPTOKEY_62_11_RegisterSize 32





#define CRYPTOCFG_62_12 (DWC_ufshc_block_BaseAddress + 0x2f30)
#define CRYPTOCFG_62_12_RegisterSize 32
#define CRYPTOCFG_62_12_RegisterResetValue 0x0
#define CRYPTOCFG_62_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_12_CRYPTOKEY_62_12_BitAddressOffset 0
#define CRYPTOCFG_62_12_CRYPTOKEY_62_12_RegisterSize 32





#define CRYPTOCFG_62_13 (DWC_ufshc_block_BaseAddress + 0x2f34)
#define CRYPTOCFG_62_13_RegisterSize 32
#define CRYPTOCFG_62_13_RegisterResetValue 0x0
#define CRYPTOCFG_62_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_13_CRYPTOKEY_62_13_BitAddressOffset 0
#define CRYPTOCFG_62_13_CRYPTOKEY_62_13_RegisterSize 32





#define CRYPTOCFG_62_14 (DWC_ufshc_block_BaseAddress + 0x2f38)
#define CRYPTOCFG_62_14_RegisterSize 32
#define CRYPTOCFG_62_14_RegisterResetValue 0x0
#define CRYPTOCFG_62_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_14_CRYPTOKEY_62_14_BitAddressOffset 0
#define CRYPTOCFG_62_14_CRYPTOKEY_62_14_RegisterSize 32





#define CRYPTOCFG_62_15 (DWC_ufshc_block_BaseAddress + 0x2f3c)
#define CRYPTOCFG_62_15_RegisterSize 32
#define CRYPTOCFG_62_15_RegisterResetValue 0x0
#define CRYPTOCFG_62_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_15_CRYPTOKEY_62_15_BitAddressOffset 0
#define CRYPTOCFG_62_15_CRYPTOKEY_62_15_RegisterSize 32





#define CRYPTOCFG_62_16 (DWC_ufshc_block_BaseAddress + 0x2f40)
#define CRYPTOCFG_62_16_RegisterSize 32
#define CRYPTOCFG_62_16_RegisterResetValue 0x0
#define CRYPTOCFG_62_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_16_DUSIZE_62_BitAddressOffset 0
#define CRYPTOCFG_62_16_DUSIZE_62_RegisterSize 8



#define CRYPTOCFG_62_16_CAPIDX_62_BitAddressOffset 8
#define CRYPTOCFG_62_16_CAPIDX_62_RegisterSize 8



#define CRYPTOCFG_62_16_CFGE_62_BitAddressOffset 31
#define CRYPTOCFG_62_16_CFGE_62_RegisterSize 1





#define CRYPTOCFG_62_17 (DWC_ufshc_block_BaseAddress + 0x2f44)
#define CRYPTOCFG_62_17_RegisterSize 32
#define CRYPTOCFG_62_17_RegisterResetValue 0x0
#define CRYPTOCFG_62_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_17_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_17_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_18 (DWC_ufshc_block_BaseAddress + 0x2f48)
#define CRYPTOCFG_62_18_RegisterSize 32
#define CRYPTOCFG_62_18_RegisterResetValue 0x0
#define CRYPTOCFG_62_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_18_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_18_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_19 (DWC_ufshc_block_BaseAddress + 0x2f4c)
#define CRYPTOCFG_62_19_RegisterSize 32
#define CRYPTOCFG_62_19_RegisterResetValue 0x0
#define CRYPTOCFG_62_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_19_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_19_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_20 (DWC_ufshc_block_BaseAddress + 0x2f50)
#define CRYPTOCFG_62_20_RegisterSize 32
#define CRYPTOCFG_62_20_RegisterResetValue 0x0
#define CRYPTOCFG_62_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_20_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_20_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_21 (DWC_ufshc_block_BaseAddress + 0x2f54)
#define CRYPTOCFG_62_21_RegisterSize 32
#define CRYPTOCFG_62_21_RegisterResetValue 0x0
#define CRYPTOCFG_62_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_21_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_21_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_22 (DWC_ufshc_block_BaseAddress + 0x2f58)
#define CRYPTOCFG_62_22_RegisterSize 32
#define CRYPTOCFG_62_22_RegisterResetValue 0x0
#define CRYPTOCFG_62_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_22_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_22_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_23 (DWC_ufshc_block_BaseAddress + 0x2f5c)
#define CRYPTOCFG_62_23_RegisterSize 32
#define CRYPTOCFG_62_23_RegisterResetValue 0x0
#define CRYPTOCFG_62_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_23_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_23_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_24 (DWC_ufshc_block_BaseAddress + 0x2f60)
#define CRYPTOCFG_62_24_RegisterSize 32
#define CRYPTOCFG_62_24_RegisterResetValue 0x0
#define CRYPTOCFG_62_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_24_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_24_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_25 (DWC_ufshc_block_BaseAddress + 0x2f64)
#define CRYPTOCFG_62_25_RegisterSize 32
#define CRYPTOCFG_62_25_RegisterResetValue 0x0
#define CRYPTOCFG_62_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_25_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_25_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_26 (DWC_ufshc_block_BaseAddress + 0x2f68)
#define CRYPTOCFG_62_26_RegisterSize 32
#define CRYPTOCFG_62_26_RegisterResetValue 0x0
#define CRYPTOCFG_62_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_26_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_26_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_27 (DWC_ufshc_block_BaseAddress + 0x2f6c)
#define CRYPTOCFG_62_27_RegisterSize 32
#define CRYPTOCFG_62_27_RegisterResetValue 0x0
#define CRYPTOCFG_62_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_27_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_27_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_28 (DWC_ufshc_block_BaseAddress + 0x2f70)
#define CRYPTOCFG_62_28_RegisterSize 32
#define CRYPTOCFG_62_28_RegisterResetValue 0x0
#define CRYPTOCFG_62_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_28_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_28_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_29 (DWC_ufshc_block_BaseAddress + 0x2f74)
#define CRYPTOCFG_62_29_RegisterSize 32
#define CRYPTOCFG_62_29_RegisterResetValue 0x0
#define CRYPTOCFG_62_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_29_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_29_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_30 (DWC_ufshc_block_BaseAddress + 0x2f78)
#define CRYPTOCFG_62_30_RegisterSize 32
#define CRYPTOCFG_62_30_RegisterResetValue 0x0
#define CRYPTOCFG_62_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_30_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_30_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_62_31 (DWC_ufshc_block_BaseAddress + 0x2f7c)
#define CRYPTOCFG_62_31_RegisterSize 32
#define CRYPTOCFG_62_31_RegisterResetValue 0x0
#define CRYPTOCFG_62_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_62_31_RESERVED_62_BitAddressOffset 0
#define CRYPTOCFG_62_31_RESERVED_62_RegisterSize 32





#define CRYPTOCFG_63_0 (DWC_ufshc_block_BaseAddress + 0x2f80)
#define CRYPTOCFG_63_0_RegisterSize 32
#define CRYPTOCFG_63_0_RegisterResetValue 0x0
#define CRYPTOCFG_63_0_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_0_CRYPTOKEY_63_0_BitAddressOffset 0
#define CRYPTOCFG_63_0_CRYPTOKEY_63_0_RegisterSize 32





#define CRYPTOCFG_63_1 (DWC_ufshc_block_BaseAddress + 0x2f84)
#define CRYPTOCFG_63_1_RegisterSize 32
#define CRYPTOCFG_63_1_RegisterResetValue 0x0
#define CRYPTOCFG_63_1_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_1_CRYPTOKEY_63_1_BitAddressOffset 0
#define CRYPTOCFG_63_1_CRYPTOKEY_63_1_RegisterSize 32





#define CRYPTOCFG_63_2 (DWC_ufshc_block_BaseAddress + 0x2f88)
#define CRYPTOCFG_63_2_RegisterSize 32
#define CRYPTOCFG_63_2_RegisterResetValue 0x0
#define CRYPTOCFG_63_2_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_2_CRYPTOKEY_63_2_BitAddressOffset 0
#define CRYPTOCFG_63_2_CRYPTOKEY_63_2_RegisterSize 32





#define CRYPTOCFG_63_3 (DWC_ufshc_block_BaseAddress + 0x2f8c)
#define CRYPTOCFG_63_3_RegisterSize 32
#define CRYPTOCFG_63_3_RegisterResetValue 0x0
#define CRYPTOCFG_63_3_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_3_CRYPTOKEY_63_3_BitAddressOffset 0
#define CRYPTOCFG_63_3_CRYPTOKEY_63_3_RegisterSize 32





#define CRYPTOCFG_63_4 (DWC_ufshc_block_BaseAddress + 0x2f90)
#define CRYPTOCFG_63_4_RegisterSize 32
#define CRYPTOCFG_63_4_RegisterResetValue 0x0
#define CRYPTOCFG_63_4_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_4_CRYPTOKEY_63_4_BitAddressOffset 0
#define CRYPTOCFG_63_4_CRYPTOKEY_63_4_RegisterSize 32





#define CRYPTOCFG_63_5 (DWC_ufshc_block_BaseAddress + 0x2f94)
#define CRYPTOCFG_63_5_RegisterSize 32
#define CRYPTOCFG_63_5_RegisterResetValue 0x0
#define CRYPTOCFG_63_5_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_5_CRYPTOKEY_63_5_BitAddressOffset 0
#define CRYPTOCFG_63_5_CRYPTOKEY_63_5_RegisterSize 32





#define CRYPTOCFG_63_6 (DWC_ufshc_block_BaseAddress + 0x2f98)
#define CRYPTOCFG_63_6_RegisterSize 32
#define CRYPTOCFG_63_6_RegisterResetValue 0x0
#define CRYPTOCFG_63_6_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_6_CRYPTOKEY_63_6_BitAddressOffset 0
#define CRYPTOCFG_63_6_CRYPTOKEY_63_6_RegisterSize 32





#define CRYPTOCFG_63_7 (DWC_ufshc_block_BaseAddress + 0x2f9c)
#define CRYPTOCFG_63_7_RegisterSize 32
#define CRYPTOCFG_63_7_RegisterResetValue 0x0
#define CRYPTOCFG_63_7_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_7_CRYPTOKEY_63_7_BitAddressOffset 0
#define CRYPTOCFG_63_7_CRYPTOKEY_63_7_RegisterSize 32





#define CRYPTOCFG_63_8 (DWC_ufshc_block_BaseAddress + 0x2fa0)
#define CRYPTOCFG_63_8_RegisterSize 32
#define CRYPTOCFG_63_8_RegisterResetValue 0x0
#define CRYPTOCFG_63_8_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_8_CRYPTOKEY_63_8_BitAddressOffset 0
#define CRYPTOCFG_63_8_CRYPTOKEY_63_8_RegisterSize 32





#define CRYPTOCFG_63_9 (DWC_ufshc_block_BaseAddress + 0x2fa4)
#define CRYPTOCFG_63_9_RegisterSize 32
#define CRYPTOCFG_63_9_RegisterResetValue 0x0
#define CRYPTOCFG_63_9_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_9_CRYPTOKEY_63_9_BitAddressOffset 0
#define CRYPTOCFG_63_9_CRYPTOKEY_63_9_RegisterSize 32





#define CRYPTOCFG_63_10 (DWC_ufshc_block_BaseAddress + 0x2fa8)
#define CRYPTOCFG_63_10_RegisterSize 32
#define CRYPTOCFG_63_10_RegisterResetValue 0x0
#define CRYPTOCFG_63_10_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_10_CRYPTOKEY_63_10_BitAddressOffset 0
#define CRYPTOCFG_63_10_CRYPTOKEY_63_10_RegisterSize 32





#define CRYPTOCFG_63_11 (DWC_ufshc_block_BaseAddress + 0x2fac)
#define CRYPTOCFG_63_11_RegisterSize 32
#define CRYPTOCFG_63_11_RegisterResetValue 0x0
#define CRYPTOCFG_63_11_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_11_CRYPTOKEY_63_11_BitAddressOffset 0
#define CRYPTOCFG_63_11_CRYPTOKEY_63_11_RegisterSize 32





#define CRYPTOCFG_63_12 (DWC_ufshc_block_BaseAddress + 0x2fb0)
#define CRYPTOCFG_63_12_RegisterSize 32
#define CRYPTOCFG_63_12_RegisterResetValue 0x0
#define CRYPTOCFG_63_12_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_12_CRYPTOKEY_63_12_BitAddressOffset 0
#define CRYPTOCFG_63_12_CRYPTOKEY_63_12_RegisterSize 32





#define CRYPTOCFG_63_13 (DWC_ufshc_block_BaseAddress + 0x2fb4)
#define CRYPTOCFG_63_13_RegisterSize 32
#define CRYPTOCFG_63_13_RegisterResetValue 0x0
#define CRYPTOCFG_63_13_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_13_CRYPTOKEY_63_13_BitAddressOffset 0
#define CRYPTOCFG_63_13_CRYPTOKEY_63_13_RegisterSize 32





#define CRYPTOCFG_63_14 (DWC_ufshc_block_BaseAddress + 0x2fb8)
#define CRYPTOCFG_63_14_RegisterSize 32
#define CRYPTOCFG_63_14_RegisterResetValue 0x0
#define CRYPTOCFG_63_14_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_14_CRYPTOKEY_63_14_BitAddressOffset 0
#define CRYPTOCFG_63_14_CRYPTOKEY_63_14_RegisterSize 32





#define CRYPTOCFG_63_15 (DWC_ufshc_block_BaseAddress + 0x2fbc)
#define CRYPTOCFG_63_15_RegisterSize 32
#define CRYPTOCFG_63_15_RegisterResetValue 0x0
#define CRYPTOCFG_63_15_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_15_CRYPTOKEY_63_15_BitAddressOffset 0
#define CRYPTOCFG_63_15_CRYPTOKEY_63_15_RegisterSize 32





#define CRYPTOCFG_63_16 (DWC_ufshc_block_BaseAddress + 0x2fc0)
#define CRYPTOCFG_63_16_RegisterSize 32
#define CRYPTOCFG_63_16_RegisterResetValue 0x0
#define CRYPTOCFG_63_16_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_16_DUSIZE_63_BitAddressOffset 0
#define CRYPTOCFG_63_16_DUSIZE_63_RegisterSize 8



#define CRYPTOCFG_63_16_CAPIDX_63_BitAddressOffset 8
#define CRYPTOCFG_63_16_CAPIDX_63_RegisterSize 8



#define CRYPTOCFG_63_16_CFGE_63_BitAddressOffset 31
#define CRYPTOCFG_63_16_CFGE_63_RegisterSize 1





#define CRYPTOCFG_63_17 (DWC_ufshc_block_BaseAddress + 0x2fc4)
#define CRYPTOCFG_63_17_RegisterSize 32
#define CRYPTOCFG_63_17_RegisterResetValue 0x0
#define CRYPTOCFG_63_17_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_17_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_17_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_18 (DWC_ufshc_block_BaseAddress + 0x2fc8)
#define CRYPTOCFG_63_18_RegisterSize 32
#define CRYPTOCFG_63_18_RegisterResetValue 0x0
#define CRYPTOCFG_63_18_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_18_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_18_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_19 (DWC_ufshc_block_BaseAddress + 0x2fcc)
#define CRYPTOCFG_63_19_RegisterSize 32
#define CRYPTOCFG_63_19_RegisterResetValue 0x0
#define CRYPTOCFG_63_19_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_19_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_19_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_20 (DWC_ufshc_block_BaseAddress + 0x2fd0)
#define CRYPTOCFG_63_20_RegisterSize 32
#define CRYPTOCFG_63_20_RegisterResetValue 0x0
#define CRYPTOCFG_63_20_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_20_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_20_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_21 (DWC_ufshc_block_BaseAddress + 0x2fd4)
#define CRYPTOCFG_63_21_RegisterSize 32
#define CRYPTOCFG_63_21_RegisterResetValue 0x0
#define CRYPTOCFG_63_21_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_21_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_21_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_22 (DWC_ufshc_block_BaseAddress + 0x2fd8)
#define CRYPTOCFG_63_22_RegisterSize 32
#define CRYPTOCFG_63_22_RegisterResetValue 0x0
#define CRYPTOCFG_63_22_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_22_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_22_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_23 (DWC_ufshc_block_BaseAddress + 0x2fdc)
#define CRYPTOCFG_63_23_RegisterSize 32
#define CRYPTOCFG_63_23_RegisterResetValue 0x0
#define CRYPTOCFG_63_23_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_23_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_23_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_24 (DWC_ufshc_block_BaseAddress + 0x2fe0)
#define CRYPTOCFG_63_24_RegisterSize 32
#define CRYPTOCFG_63_24_RegisterResetValue 0x0
#define CRYPTOCFG_63_24_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_24_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_24_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_25 (DWC_ufshc_block_BaseAddress + 0x2fe4)
#define CRYPTOCFG_63_25_RegisterSize 32
#define CRYPTOCFG_63_25_RegisterResetValue 0x0
#define CRYPTOCFG_63_25_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_25_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_25_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_26 (DWC_ufshc_block_BaseAddress + 0x2fe8)
#define CRYPTOCFG_63_26_RegisterSize 32
#define CRYPTOCFG_63_26_RegisterResetValue 0x0
#define CRYPTOCFG_63_26_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_26_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_26_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_27 (DWC_ufshc_block_BaseAddress + 0x2fec)
#define CRYPTOCFG_63_27_RegisterSize 32
#define CRYPTOCFG_63_27_RegisterResetValue 0x0
#define CRYPTOCFG_63_27_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_27_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_27_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_28 (DWC_ufshc_block_BaseAddress + 0x2ff0)
#define CRYPTOCFG_63_28_RegisterSize 32
#define CRYPTOCFG_63_28_RegisterResetValue 0x0
#define CRYPTOCFG_63_28_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_28_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_28_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_29 (DWC_ufshc_block_BaseAddress + 0x2ff4)
#define CRYPTOCFG_63_29_RegisterSize 32
#define CRYPTOCFG_63_29_RegisterResetValue 0x0
#define CRYPTOCFG_63_29_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_29_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_29_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_30 (DWC_ufshc_block_BaseAddress + 0x2ff8)
#define CRYPTOCFG_63_30_RegisterSize 32
#define CRYPTOCFG_63_30_RegisterResetValue 0x0
#define CRYPTOCFG_63_30_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_30_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_30_RESERVED_63_RegisterSize 32





#define CRYPTOCFG_63_31 (DWC_ufshc_block_BaseAddress + 0x2ffc)
#define CRYPTOCFG_63_31_RegisterSize 32
#define CRYPTOCFG_63_31_RegisterResetValue 0x0
#define CRYPTOCFG_63_31_RegisterResetMask 0xffffffff





#define CRYPTOCFG_63_31_RESERVED_63_BitAddressOffset 0
#define CRYPTOCFG_63_31_RESERVED_63_RegisterSize 32





#define SQATTR0 (DWC_ufshc_block_BaseAddress + 0x3000)
#define SQATTR0_RegisterSize 32
#define SQATTR0_RegisterResetValue 0xff
#define SQATTR0_RegisterResetMask 0xffffffff





#define SQATTR0_SIZE_BitAddressOffset 0
#define SQATTR0_SIZE_RegisterSize 16



#define SQATTR0_CQID_BitAddressOffset 16
#define SQATTR0_CQID_RegisterSize 4



#define SQATTR0_SQATTR0_RSVD_27_24_BitAddressOffset 20
#define SQATTR0_SQATTR0_RSVD_27_24_RegisterSize 8



#define SQATTR0_SQPL_BitAddressOffset 28
#define SQATTR0_SQPL_RegisterSize 3



#define SQATTR0_SQEN_BitAddressOffset 31
#define SQATTR0_SQEN_RegisterSize 1





#define SQLBA0 (DWC_ufshc_block_BaseAddress + 0x3004)
#define SQLBA0_RegisterSize 32
#define SQLBA0_RegisterResetValue 0x0
#define SQLBA0_RegisterResetMask 0xffffffff





#define SQLBA0_SQLBA0_RSVD_9_0_BitAddressOffset 0
#define SQLBA0_SQLBA0_RSVD_9_0_RegisterSize 10



#define SQLBA0_SQLBA_BitAddressOffset 10
#define SQLBA0_SQLBA_RegisterSize 22





#define SQUBA0 (DWC_ufshc_block_BaseAddress + 0x3008)
#define SQUBA0_RegisterSize 32
#define SQUBA0_RegisterResetValue 0x0
#define SQUBA0_RegisterResetMask 0xffffffff





#define SQUBA0_SQUBA_BitAddressOffset 0
#define SQUBA0_SQUBA_RegisterSize 32





#define SQDAO0 (DWC_ufshc_block_BaseAddress + 0x300c)
#define SQDAO0_RegisterSize 32
#define SQDAO0_RegisterResetValue 0x0
#define SQDAO0_RegisterResetMask 0xffffffff





#define SQDAO0_SQDAO_BitAddressOffset 0
#define SQDAO0_SQDAO_RegisterSize 32





#define SQISAO0 (DWC_ufshc_block_BaseAddress + 0x3010)
#define SQISAO0_RegisterSize 32
#define SQISAO0_RegisterResetValue 0x0
#define SQISAO0_RegisterResetMask 0xffffffff





#define SQISAO0_SQISAO_BitAddressOffset 0
#define SQISAO0_SQISAO_RegisterSize 32





#define SQCFG0 (DWC_ufshc_block_BaseAddress + 0x3014)
#define SQCFG0_RegisterSize 32
#define SQCFG0_RegisterResetValue 0x0
#define SQCFG0_RegisterResetMask 0xffffffff





#define SQCFG0_IAG_BitAddressOffset 0
#define SQCFG0_IAG_RegisterSize 5



#define SQCFG0_SQCFG0_RSVD_7_5_BitAddressOffset 5
#define SQCFG0_SQCFG0_RSVD_7_5_RegisterSize 3



#define SQCFG0_IAGVLD_BitAddressOffset 8
#define SQCFG0_IAGVLD_RegisterSize 1



#define SQCFG0_SQCFG0_RSVD_31_9_BitAddressOffset 9
#define SQCFG0_SQCFG0_RSVD_31_9_RegisterSize 23





#define CQATTR0 (DWC_ufshc_block_BaseAddress + 0x3020)
#define CQATTR0_RegisterSize 32
#define CQATTR0_RegisterResetValue 0xff
#define CQATTR0_RegisterResetMask 0xffffffff





#define CQATTR0_SIZE_BitAddressOffset 0
#define CQATTR0_SIZE_RegisterSize 16



#define CQATTR0_CQATTR0_RSVD_30_16_BitAddressOffset 16
#define CQATTR0_CQATTR0_RSVD_30_16_RegisterSize 15



#define CQATTR0_CQEN_BitAddressOffset 31
#define CQATTR0_CQEN_RegisterSize 1





#define CQLBA0 (DWC_ufshc_block_BaseAddress + 0x3024)
#define CQLBA0_RegisterSize 32
#define CQLBA0_RegisterResetValue 0x0
#define CQLBA0_RegisterResetMask 0xffffffff





#define CQLBA0_CQLBA0_RSVD_9_0_BitAddressOffset 0
#define CQLBA0_CQLBA0_RSVD_9_0_RegisterSize 10



#define CQLBA0_CQLBA_BitAddressOffset 10
#define CQLBA0_CQLBA_RegisterSize 22





#define CQUBA0 (DWC_ufshc_block_BaseAddress + 0x3028)
#define CQUBA0_RegisterSize 32
#define CQUBA0_RegisterResetValue 0x0
#define CQUBA0_RegisterResetMask 0xffffffff





#define CQUBA0_CQUBA_BitAddressOffset 0
#define CQUBA0_CQUBA_RegisterSize 32





#define CQDAO0 (DWC_ufshc_block_BaseAddress + 0x302c)
#define CQDAO0_RegisterSize 32
#define CQDAO0_RegisterResetValue 0x0
#define CQDAO0_RegisterResetMask 0xffffffff





#define CQDAO0_CQDAO_BitAddressOffset 0
#define CQDAO0_CQDAO_RegisterSize 32





#define CQISAO0 (DWC_ufshc_block_BaseAddress + 0x3030)
#define CQISAO0_RegisterSize 32
#define CQISAO0_RegisterResetValue 0x0
#define CQISAO0_RegisterResetMask 0xffffffff





#define CQISAO0_CQISAO_BitAddressOffset 0
#define CQISAO0_CQISAO_RegisterSize 32





#define CQCFG0 (DWC_ufshc_block_BaseAddress + 0x3034)
#define CQCFG0_RegisterSize 32
#define CQCFG0_RegisterResetValue 0x0
#define CQCFG0_RegisterResetMask 0xffffffff





#define CQCFG0_IAG_BitAddressOffset 0
#define CQCFG0_IAG_RegisterSize 5



#define CQCFG0_CQCFG0_RSVD_7_5_BitAddressOffset 5
#define CQCFG0_CQCFG0_RSVD_7_5_RegisterSize 3



#define CQCFG0_IAGVLD_BitAddressOffset 8
#define CQCFG0_IAGVLD_RegisterSize 1



#define CQCFG0_CQCFG0_RSVD_31_9_BitAddressOffset 9
#define CQCFG0_CQCFG0_RSVD_31_9_RegisterSize 23





#define SQATTR1 (DWC_ufshc_block_BaseAddress + 0x3040)
#define SQATTR1_RegisterSize 32
#define SQATTR1_RegisterResetValue 0xff
#define SQATTR1_RegisterResetMask 0xffffffff





#define SQATTR1_SIZE_BitAddressOffset 0
#define SQATTR1_SIZE_RegisterSize 16



#define SQATTR1_CQID_BitAddressOffset 16
#define SQATTR1_CQID_RegisterSize 4



#define SQATTR1_SQATTR1_RSVD_27_24_BitAddressOffset 20
#define SQATTR1_SQATTR1_RSVD_27_24_RegisterSize 8



#define SQATTR1_SQPL_BitAddressOffset 28
#define SQATTR1_SQPL_RegisterSize 3



#define SQATTR1_SQEN_BitAddressOffset 31
#define SQATTR1_SQEN_RegisterSize 1





#define SQLBA1 (DWC_ufshc_block_BaseAddress + 0x3044)
#define SQLBA1_RegisterSize 32
#define SQLBA1_RegisterResetValue 0x0
#define SQLBA1_RegisterResetMask 0xffffffff





#define SQLBA1_SQLBA1_RSVD_9_0_BitAddressOffset 0
#define SQLBA1_SQLBA1_RSVD_9_0_RegisterSize 10



#define SQLBA1_SQLBA_BitAddressOffset 10
#define SQLBA1_SQLBA_RegisterSize 22





#define SQUBA1 (DWC_ufshc_block_BaseAddress + 0x3048)
#define SQUBA1_RegisterSize 32
#define SQUBA1_RegisterResetValue 0x0
#define SQUBA1_RegisterResetMask 0xffffffff





#define SQUBA1_SQUBA_BitAddressOffset 0
#define SQUBA1_SQUBA_RegisterSize 32





#define SQDAO1 (DWC_ufshc_block_BaseAddress + 0x304c)
#define SQDAO1_RegisterSize 32
#define SQDAO1_RegisterResetValue 0x0
#define SQDAO1_RegisterResetMask 0xffffffff





#define SQDAO1_SQDAO_BitAddressOffset 0
#define SQDAO1_SQDAO_RegisterSize 32





#define SQISAO1 (DWC_ufshc_block_BaseAddress + 0x3050)
#define SQISAO1_RegisterSize 32
#define SQISAO1_RegisterResetValue 0x0
#define SQISAO1_RegisterResetMask 0xffffffff





#define SQISAO1_SQISAO_BitAddressOffset 0
#define SQISAO1_SQISAO_RegisterSize 32





#define SQCFG1 (DWC_ufshc_block_BaseAddress + 0x3054)
#define SQCFG1_RegisterSize 32
#define SQCFG1_RegisterResetValue 0x0
#define SQCFG1_RegisterResetMask 0xffffffff





#define SQCFG1_IAG_BitAddressOffset 0
#define SQCFG1_IAG_RegisterSize 5



#define SQCFG1_SQCFG1_RSVD_7_5_BitAddressOffset 5
#define SQCFG1_SQCFG1_RSVD_7_5_RegisterSize 3



#define SQCFG1_IAGVLD_BitAddressOffset 8
#define SQCFG1_IAGVLD_RegisterSize 1



#define SQCFG1_SQCFG1_RSVD_31_9_BitAddressOffset 9
#define SQCFG1_SQCFG1_RSVD_31_9_RegisterSize 23





#define CQATTR1 (DWC_ufshc_block_BaseAddress + 0x3060)
#define CQATTR1_RegisterSize 32
#define CQATTR1_RegisterResetValue 0xff
#define CQATTR1_RegisterResetMask 0xffffffff





#define CQATTR1_SIZE_BitAddressOffset 0
#define CQATTR1_SIZE_RegisterSize 16



#define CQATTR1_CQATTR1_RSVD_30_16_BitAddressOffset 16
#define CQATTR1_CQATTR1_RSVD_30_16_RegisterSize 15



#define CQATTR1_CQEN_BitAddressOffset 31
#define CQATTR1_CQEN_RegisterSize 1





#define CQLBA1 (DWC_ufshc_block_BaseAddress + 0x3064)
#define CQLBA1_RegisterSize 32
#define CQLBA1_RegisterResetValue 0x0
#define CQLBA1_RegisterResetMask 0xffffffff





#define CQLBA1_CQLBA1_RSVD_9_0_BitAddressOffset 0
#define CQLBA1_CQLBA1_RSVD_9_0_RegisterSize 10



#define CQLBA1_CQLBA_BitAddressOffset 10
#define CQLBA1_CQLBA_RegisterSize 22





#define CQUBA1 (DWC_ufshc_block_BaseAddress + 0x3068)
#define CQUBA1_RegisterSize 32
#define CQUBA1_RegisterResetValue 0x0
#define CQUBA1_RegisterResetMask 0xffffffff





#define CQUBA1_CQUBA_BitAddressOffset 0
#define CQUBA1_CQUBA_RegisterSize 32





#define CQDAO1 (DWC_ufshc_block_BaseAddress + 0x306c)
#define CQDAO1_RegisterSize 32
#define CQDAO1_RegisterResetValue 0x0
#define CQDAO1_RegisterResetMask 0xffffffff





#define CQDAO1_CQDAO_BitAddressOffset 0
#define CQDAO1_CQDAO_RegisterSize 32





#define CQISAO1 (DWC_ufshc_block_BaseAddress + 0x3070)
#define CQISAO1_RegisterSize 32
#define CQISAO1_RegisterResetValue 0x0
#define CQISAO1_RegisterResetMask 0xffffffff





#define CQISAO1_CQISAO_BitAddressOffset 0
#define CQISAO1_CQISAO_RegisterSize 32





#define CQCFG1 (DWC_ufshc_block_BaseAddress + 0x3074)
#define CQCFG1_RegisterSize 32
#define CQCFG1_RegisterResetValue 0x0
#define CQCFG1_RegisterResetMask 0xffffffff





#define CQCFG1_IAG_BitAddressOffset 0
#define CQCFG1_IAG_RegisterSize 5



#define CQCFG1_CQCFG1_RSVD_7_5_BitAddressOffset 5
#define CQCFG1_CQCFG1_RSVD_7_5_RegisterSize 3



#define CQCFG1_IAGVLD_BitAddressOffset 8
#define CQCFG1_IAGVLD_RegisterSize 1



#define CQCFG1_CQCFG1_RSVD_31_9_BitAddressOffset 9
#define CQCFG1_CQCFG1_RSVD_31_9_RegisterSize 23





#define SQATTR2 (DWC_ufshc_block_BaseAddress + 0x3080)
#define SQATTR2_RegisterSize 32
#define SQATTR2_RegisterResetValue 0xff
#define SQATTR2_RegisterResetMask 0xffffffff





#define SQATTR2_SIZE_BitAddressOffset 0
#define SQATTR2_SIZE_RegisterSize 16



#define SQATTR2_CQID_BitAddressOffset 16
#define SQATTR2_CQID_RegisterSize 4



#define SQATTR2_SQATTR2_RSVD_27_24_BitAddressOffset 20
#define SQATTR2_SQATTR2_RSVD_27_24_RegisterSize 8



#define SQATTR2_SQPL_BitAddressOffset 28
#define SQATTR2_SQPL_RegisterSize 3



#define SQATTR2_SQEN_BitAddressOffset 31
#define SQATTR2_SQEN_RegisterSize 1





#define SQLBA2 (DWC_ufshc_block_BaseAddress + 0x3084)
#define SQLBA2_RegisterSize 32
#define SQLBA2_RegisterResetValue 0x0
#define SQLBA2_RegisterResetMask 0xffffffff





#define SQLBA2_SQLBA2_RSVD_9_0_BitAddressOffset 0
#define SQLBA2_SQLBA2_RSVD_9_0_RegisterSize 10



#define SQLBA2_SQLBA_BitAddressOffset 10
#define SQLBA2_SQLBA_RegisterSize 22





#define SQUBA2 (DWC_ufshc_block_BaseAddress + 0x3088)
#define SQUBA2_RegisterSize 32
#define SQUBA2_RegisterResetValue 0x0
#define SQUBA2_RegisterResetMask 0xffffffff





#define SQUBA2_SQUBA_BitAddressOffset 0
#define SQUBA2_SQUBA_RegisterSize 32





#define SQDAO2 (DWC_ufshc_block_BaseAddress + 0x308c)
#define SQDAO2_RegisterSize 32
#define SQDAO2_RegisterResetValue 0x0
#define SQDAO2_RegisterResetMask 0xffffffff





#define SQDAO2_SQDAO_BitAddressOffset 0
#define SQDAO2_SQDAO_RegisterSize 32





#define SQISAO2 (DWC_ufshc_block_BaseAddress + 0x3090)
#define SQISAO2_RegisterSize 32
#define SQISAO2_RegisterResetValue 0x0
#define SQISAO2_RegisterResetMask 0xffffffff





#define SQISAO2_SQISAO_BitAddressOffset 0
#define SQISAO2_SQISAO_RegisterSize 32





#define SQCFG2 (DWC_ufshc_block_BaseAddress + 0x3094)
#define SQCFG2_RegisterSize 32
#define SQCFG2_RegisterResetValue 0x0
#define SQCFG2_RegisterResetMask 0xffffffff





#define SQCFG2_IAG_BitAddressOffset 0
#define SQCFG2_IAG_RegisterSize 5



#define SQCFG2_SQCFG2_RSVD_7_5_BitAddressOffset 5
#define SQCFG2_SQCFG2_RSVD_7_5_RegisterSize 3



#define SQCFG2_IAGVLD_BitAddressOffset 8
#define SQCFG2_IAGVLD_RegisterSize 1



#define SQCFG2_SQCFG2_RSVD_31_9_BitAddressOffset 9
#define SQCFG2_SQCFG2_RSVD_31_9_RegisterSize 23





#define CQATTR2 (DWC_ufshc_block_BaseAddress + 0x30a0)
#define CQATTR2_RegisterSize 32
#define CQATTR2_RegisterResetValue 0xff
#define CQATTR2_RegisterResetMask 0xffffffff





#define CQATTR2_SIZE_BitAddressOffset 0
#define CQATTR2_SIZE_RegisterSize 16



#define CQATTR2_CQATTR2_RSVD_30_16_BitAddressOffset 16
#define CQATTR2_CQATTR2_RSVD_30_16_RegisterSize 15



#define CQATTR2_CQEN_BitAddressOffset 31
#define CQATTR2_CQEN_RegisterSize 1





#define CQLBA2 (DWC_ufshc_block_BaseAddress + 0x30a4)
#define CQLBA2_RegisterSize 32
#define CQLBA2_RegisterResetValue 0x0
#define CQLBA2_RegisterResetMask 0xffffffff





#define CQLBA2_CQLBA2_RSVD_9_0_BitAddressOffset 0
#define CQLBA2_CQLBA2_RSVD_9_0_RegisterSize 10



#define CQLBA2_CQLBA_BitAddressOffset 10
#define CQLBA2_CQLBA_RegisterSize 22





#define CQUBA2 (DWC_ufshc_block_BaseAddress + 0x30a8)
#define CQUBA2_RegisterSize 32
#define CQUBA2_RegisterResetValue 0x0
#define CQUBA2_RegisterResetMask 0xffffffff





#define CQUBA2_CQUBA_BitAddressOffset 0
#define CQUBA2_CQUBA_RegisterSize 32





#define CQDAO2 (DWC_ufshc_block_BaseAddress + 0x30ac)
#define CQDAO2_RegisterSize 32
#define CQDAO2_RegisterResetValue 0x0
#define CQDAO2_RegisterResetMask 0xffffffff





#define CQDAO2_CQDAO_BitAddressOffset 0
#define CQDAO2_CQDAO_RegisterSize 32





#define CQISAO2 (DWC_ufshc_block_BaseAddress + 0x30b0)
#define CQISAO2_RegisterSize 32
#define CQISAO2_RegisterResetValue 0x0
#define CQISAO2_RegisterResetMask 0xffffffff





#define CQISAO2_CQISAO_BitAddressOffset 0
#define CQISAO2_CQISAO_RegisterSize 32





#define CQCFG2 (DWC_ufshc_block_BaseAddress + 0x30b4)
#define CQCFG2_RegisterSize 32
#define CQCFG2_RegisterResetValue 0x0
#define CQCFG2_RegisterResetMask 0xffffffff





#define CQCFG2_IAG_BitAddressOffset 0
#define CQCFG2_IAG_RegisterSize 5



#define CQCFG2_CQCFG2_RSVD_7_5_BitAddressOffset 5
#define CQCFG2_CQCFG2_RSVD_7_5_RegisterSize 3



#define CQCFG2_IAGVLD_BitAddressOffset 8
#define CQCFG2_IAGVLD_RegisterSize 1



#define CQCFG2_CQCFG2_RSVD_31_9_BitAddressOffset 9
#define CQCFG2_CQCFG2_RSVD_31_9_RegisterSize 23





#define SQATTR3 (DWC_ufshc_block_BaseAddress + 0x30c0)
#define SQATTR3_RegisterSize 32
#define SQATTR3_RegisterResetValue 0xff
#define SQATTR3_RegisterResetMask 0xffffffff





#define SQATTR3_SIZE_BitAddressOffset 0
#define SQATTR3_SIZE_RegisterSize 16



#define SQATTR3_CQID_BitAddressOffset 16
#define SQATTR3_CQID_RegisterSize 4



#define SQATTR3_SQATTR3_RSVD_27_24_BitAddressOffset 20
#define SQATTR3_SQATTR3_RSVD_27_24_RegisterSize 8



#define SQATTR3_SQPL_BitAddressOffset 28
#define SQATTR3_SQPL_RegisterSize 3



#define SQATTR3_SQEN_BitAddressOffset 31
#define SQATTR3_SQEN_RegisterSize 1





#define SQLBA3 (DWC_ufshc_block_BaseAddress + 0x30c4)
#define SQLBA3_RegisterSize 32
#define SQLBA3_RegisterResetValue 0x0
#define SQLBA3_RegisterResetMask 0xffffffff





#define SQLBA3_SQLBA3_RSVD_9_0_BitAddressOffset 0
#define SQLBA3_SQLBA3_RSVD_9_0_RegisterSize 10



#define SQLBA3_SQLBA_BitAddressOffset 10
#define SQLBA3_SQLBA_RegisterSize 22





#define SQUBA3 (DWC_ufshc_block_BaseAddress + 0x30c8)
#define SQUBA3_RegisterSize 32
#define SQUBA3_RegisterResetValue 0x0
#define SQUBA3_RegisterResetMask 0xffffffff





#define SQUBA3_SQUBA_BitAddressOffset 0
#define SQUBA3_SQUBA_RegisterSize 32





#define SQDAO3 (DWC_ufshc_block_BaseAddress + 0x30cc)
#define SQDAO3_RegisterSize 32
#define SQDAO3_RegisterResetValue 0x0
#define SQDAO3_RegisterResetMask 0xffffffff





#define SQDAO3_SQDAO_BitAddressOffset 0
#define SQDAO3_SQDAO_RegisterSize 32





#define SQISAO3 (DWC_ufshc_block_BaseAddress + 0x30d0)
#define SQISAO3_RegisterSize 32
#define SQISAO3_RegisterResetValue 0x0
#define SQISAO3_RegisterResetMask 0xffffffff





#define SQISAO3_SQISAO_BitAddressOffset 0
#define SQISAO3_SQISAO_RegisterSize 32





#define SQCFG3 (DWC_ufshc_block_BaseAddress + 0x30d4)
#define SQCFG3_RegisterSize 32
#define SQCFG3_RegisterResetValue 0x0
#define SQCFG3_RegisterResetMask 0xffffffff





#define SQCFG3_IAG_BitAddressOffset 0
#define SQCFG3_IAG_RegisterSize 5



#define SQCFG3_SQCFG3_RSVD_7_5_BitAddressOffset 5
#define SQCFG3_SQCFG3_RSVD_7_5_RegisterSize 3



#define SQCFG3_IAGVLD_BitAddressOffset 8
#define SQCFG3_IAGVLD_RegisterSize 1



#define SQCFG3_SQCFG3_RSVD_31_9_BitAddressOffset 9
#define SQCFG3_SQCFG3_RSVD_31_9_RegisterSize 23





#define CQATTR3 (DWC_ufshc_block_BaseAddress + 0x30e0)
#define CQATTR3_RegisterSize 32
#define CQATTR3_RegisterResetValue 0xff
#define CQATTR3_RegisterResetMask 0xffffffff





#define CQATTR3_SIZE_BitAddressOffset 0
#define CQATTR3_SIZE_RegisterSize 16



#define CQATTR3_CQATTR3_RSVD_30_16_BitAddressOffset 16
#define CQATTR3_CQATTR3_RSVD_30_16_RegisterSize 15



#define CQATTR3_CQEN_BitAddressOffset 31
#define CQATTR3_CQEN_RegisterSize 1





#define CQLBA3 (DWC_ufshc_block_BaseAddress + 0x30e4)
#define CQLBA3_RegisterSize 32
#define CQLBA3_RegisterResetValue 0x0
#define CQLBA3_RegisterResetMask 0xffffffff





#define CQLBA3_CQLBA3_RSVD_9_0_BitAddressOffset 0
#define CQLBA3_CQLBA3_RSVD_9_0_RegisterSize 10



#define CQLBA3_CQLBA_BitAddressOffset 10
#define CQLBA3_CQLBA_RegisterSize 22





#define CQUBA3 (DWC_ufshc_block_BaseAddress + 0x30e8)
#define CQUBA3_RegisterSize 32
#define CQUBA3_RegisterResetValue 0x0
#define CQUBA3_RegisterResetMask 0xffffffff





#define CQUBA3_CQUBA_BitAddressOffset 0
#define CQUBA3_CQUBA_RegisterSize 32





#define CQDAO3 (DWC_ufshc_block_BaseAddress + 0x30ec)
#define CQDAO3_RegisterSize 32
#define CQDAO3_RegisterResetValue 0x0
#define CQDAO3_RegisterResetMask 0xffffffff





#define CQDAO3_CQDAO_BitAddressOffset 0
#define CQDAO3_CQDAO_RegisterSize 32





#define CQISAO3 (DWC_ufshc_block_BaseAddress + 0x30f0)
#define CQISAO3_RegisterSize 32
#define CQISAO3_RegisterResetValue 0x0
#define CQISAO3_RegisterResetMask 0xffffffff





#define CQISAO3_CQISAO_BitAddressOffset 0
#define CQISAO3_CQISAO_RegisterSize 32





#define CQCFG3 (DWC_ufshc_block_BaseAddress + 0x30f4)
#define CQCFG3_RegisterSize 32
#define CQCFG3_RegisterResetValue 0x0
#define CQCFG3_RegisterResetMask 0xffffffff





#define CQCFG3_IAG_BitAddressOffset 0
#define CQCFG3_IAG_RegisterSize 5



#define CQCFG3_CQCFG3_RSVD_7_5_BitAddressOffset 5
#define CQCFG3_CQCFG3_RSVD_7_5_RegisterSize 3



#define CQCFG3_IAGVLD_BitAddressOffset 8
#define CQCFG3_IAGVLD_RegisterSize 1



#define CQCFG3_CQCFG3_RSVD_31_9_BitAddressOffset 9
#define CQCFG3_CQCFG3_RSVD_31_9_RegisterSize 23





#define SQATTR4 (DWC_ufshc_block_BaseAddress + 0x3100)
#define SQATTR4_RegisterSize 32
#define SQATTR4_RegisterResetValue 0xff
#define SQATTR4_RegisterResetMask 0xffffffff





#define SQATTR4_SIZE_BitAddressOffset 0
#define SQATTR4_SIZE_RegisterSize 16



#define SQATTR4_CQID_BitAddressOffset 16
#define SQATTR4_CQID_RegisterSize 4



#define SQATTR4_SQATTR4_RSVD_27_24_BitAddressOffset 20
#define SQATTR4_SQATTR4_RSVD_27_24_RegisterSize 8



#define SQATTR4_SQPL_BitAddressOffset 28
#define SQATTR4_SQPL_RegisterSize 3



#define SQATTR4_SQEN_BitAddressOffset 31
#define SQATTR4_SQEN_RegisterSize 1





#define SQLBA4 (DWC_ufshc_block_BaseAddress + 0x3104)
#define SQLBA4_RegisterSize 32
#define SQLBA4_RegisterResetValue 0x0
#define SQLBA4_RegisterResetMask 0xffffffff





#define SQLBA4_SQLBA4_RSVD_9_0_BitAddressOffset 0
#define SQLBA4_SQLBA4_RSVD_9_0_RegisterSize 10



#define SQLBA4_SQLBA_BitAddressOffset 10
#define SQLBA4_SQLBA_RegisterSize 22





#define SQUBA4 (DWC_ufshc_block_BaseAddress + 0x3108)
#define SQUBA4_RegisterSize 32
#define SQUBA4_RegisterResetValue 0x0
#define SQUBA4_RegisterResetMask 0xffffffff





#define SQUBA4_SQUBA_BitAddressOffset 0
#define SQUBA4_SQUBA_RegisterSize 32





#define SQDAO4 (DWC_ufshc_block_BaseAddress + 0x310c)
#define SQDAO4_RegisterSize 32
#define SQDAO4_RegisterResetValue 0x0
#define SQDAO4_RegisterResetMask 0xffffffff





#define SQDAO4_SQDAO_BitAddressOffset 0
#define SQDAO4_SQDAO_RegisterSize 32





#define SQISAO4 (DWC_ufshc_block_BaseAddress + 0x3110)
#define SQISAO4_RegisterSize 32
#define SQISAO4_RegisterResetValue 0x0
#define SQISAO4_RegisterResetMask 0xffffffff





#define SQISAO4_SQISAO_BitAddressOffset 0
#define SQISAO4_SQISAO_RegisterSize 32





#define SQCFG4 (DWC_ufshc_block_BaseAddress + 0x3114)
#define SQCFG4_RegisterSize 32
#define SQCFG4_RegisterResetValue 0x0
#define SQCFG4_RegisterResetMask 0xffffffff





#define SQCFG4_IAG_BitAddressOffset 0
#define SQCFG4_IAG_RegisterSize 5



#define SQCFG4_SQCFG4_RSVD_7_5_BitAddressOffset 5
#define SQCFG4_SQCFG4_RSVD_7_5_RegisterSize 3



#define SQCFG4_IAGVLD_BitAddressOffset 8
#define SQCFG4_IAGVLD_RegisterSize 1



#define SQCFG4_SQCFG4_RSVD_31_9_BitAddressOffset 9
#define SQCFG4_SQCFG4_RSVD_31_9_RegisterSize 23





#define CQATTR4 (DWC_ufshc_block_BaseAddress + 0x3120)
#define CQATTR4_RegisterSize 32
#define CQATTR4_RegisterResetValue 0xff
#define CQATTR4_RegisterResetMask 0xffffffff





#define CQATTR4_SIZE_BitAddressOffset 0
#define CQATTR4_SIZE_RegisterSize 16



#define CQATTR4_CQATTR4_RSVD_30_16_BitAddressOffset 16
#define CQATTR4_CQATTR4_RSVD_30_16_RegisterSize 15



#define CQATTR4_CQEN_BitAddressOffset 31
#define CQATTR4_CQEN_RegisterSize 1





#define CQLBA4 (DWC_ufshc_block_BaseAddress + 0x3124)
#define CQLBA4_RegisterSize 32
#define CQLBA4_RegisterResetValue 0x0
#define CQLBA4_RegisterResetMask 0xffffffff





#define CQLBA4_CQLBA4_RSVD_9_0_BitAddressOffset 0
#define CQLBA4_CQLBA4_RSVD_9_0_RegisterSize 10



#define CQLBA4_CQLBA_BitAddressOffset 10
#define CQLBA4_CQLBA_RegisterSize 22





#define CQUBA4 (DWC_ufshc_block_BaseAddress + 0x3128)
#define CQUBA4_RegisterSize 32
#define CQUBA4_RegisterResetValue 0x0
#define CQUBA4_RegisterResetMask 0xffffffff





#define CQUBA4_CQUBA_BitAddressOffset 0
#define CQUBA4_CQUBA_RegisterSize 32





#define CQDAO4 (DWC_ufshc_block_BaseAddress + 0x312c)
#define CQDAO4_RegisterSize 32
#define CQDAO4_RegisterResetValue 0x0
#define CQDAO4_RegisterResetMask 0xffffffff





#define CQDAO4_CQDAO_BitAddressOffset 0
#define CQDAO4_CQDAO_RegisterSize 32





#define CQISAO4 (DWC_ufshc_block_BaseAddress + 0x3130)
#define CQISAO4_RegisterSize 32
#define CQISAO4_RegisterResetValue 0x0
#define CQISAO4_RegisterResetMask 0xffffffff





#define CQISAO4_CQISAO_BitAddressOffset 0
#define CQISAO4_CQISAO_RegisterSize 32





#define CQCFG4 (DWC_ufshc_block_BaseAddress + 0x3134)
#define CQCFG4_RegisterSize 32
#define CQCFG4_RegisterResetValue 0x0
#define CQCFG4_RegisterResetMask 0xffffffff





#define CQCFG4_IAG_BitAddressOffset 0
#define CQCFG4_IAG_RegisterSize 5



#define CQCFG4_CQCFG4_RSVD_7_5_BitAddressOffset 5
#define CQCFG4_CQCFG4_RSVD_7_5_RegisterSize 3



#define CQCFG4_IAGVLD_BitAddressOffset 8
#define CQCFG4_IAGVLD_RegisterSize 1



#define CQCFG4_CQCFG4_RSVD_31_9_BitAddressOffset 9
#define CQCFG4_CQCFG4_RSVD_31_9_RegisterSize 23





#define SQATTR5 (DWC_ufshc_block_BaseAddress + 0x3140)
#define SQATTR5_RegisterSize 32
#define SQATTR5_RegisterResetValue 0xff
#define SQATTR5_RegisterResetMask 0xffffffff





#define SQATTR5_SIZE_BitAddressOffset 0
#define SQATTR5_SIZE_RegisterSize 16



#define SQATTR5_CQID_BitAddressOffset 16
#define SQATTR5_CQID_RegisterSize 4



#define SQATTR5_SQATTR5_RSVD_27_24_BitAddressOffset 20
#define SQATTR5_SQATTR5_RSVD_27_24_RegisterSize 8



#define SQATTR5_SQPL_BitAddressOffset 28
#define SQATTR5_SQPL_RegisterSize 3



#define SQATTR5_SQEN_BitAddressOffset 31
#define SQATTR5_SQEN_RegisterSize 1





#define SQLBA5 (DWC_ufshc_block_BaseAddress + 0x3144)
#define SQLBA5_RegisterSize 32
#define SQLBA5_RegisterResetValue 0x0
#define SQLBA5_RegisterResetMask 0xffffffff





#define SQLBA5_SQLBA5_RSVD_9_0_BitAddressOffset 0
#define SQLBA5_SQLBA5_RSVD_9_0_RegisterSize 10



#define SQLBA5_SQLBA_BitAddressOffset 10
#define SQLBA5_SQLBA_RegisterSize 22





#define SQUBA5 (DWC_ufshc_block_BaseAddress + 0x3148)
#define SQUBA5_RegisterSize 32
#define SQUBA5_RegisterResetValue 0x0
#define SQUBA5_RegisterResetMask 0xffffffff





#define SQUBA5_SQUBA_BitAddressOffset 0
#define SQUBA5_SQUBA_RegisterSize 32





#define SQDAO5 (DWC_ufshc_block_BaseAddress + 0x314c)
#define SQDAO5_RegisterSize 32
#define SQDAO5_RegisterResetValue 0x0
#define SQDAO5_RegisterResetMask 0xffffffff





#define SQDAO5_SQDAO_BitAddressOffset 0
#define SQDAO5_SQDAO_RegisterSize 32





#define SQISAO5 (DWC_ufshc_block_BaseAddress + 0x3150)
#define SQISAO5_RegisterSize 32
#define SQISAO5_RegisterResetValue 0x0
#define SQISAO5_RegisterResetMask 0xffffffff





#define SQISAO5_SQISAO_BitAddressOffset 0
#define SQISAO5_SQISAO_RegisterSize 32





#define SQCFG5 (DWC_ufshc_block_BaseAddress + 0x3154)
#define SQCFG5_RegisterSize 32
#define SQCFG5_RegisterResetValue 0x0
#define SQCFG5_RegisterResetMask 0xffffffff





#define SQCFG5_IAG_BitAddressOffset 0
#define SQCFG5_IAG_RegisterSize 5



#define SQCFG5_SQCFG5_RSVD_7_5_BitAddressOffset 5
#define SQCFG5_SQCFG5_RSVD_7_5_RegisterSize 3



#define SQCFG5_IAGVLD_BitAddressOffset 8
#define SQCFG5_IAGVLD_RegisterSize 1



#define SQCFG5_SQCFG5_RSVD_31_9_BitAddressOffset 9
#define SQCFG5_SQCFG5_RSVD_31_9_RegisterSize 23





#define CQATTR5 (DWC_ufshc_block_BaseAddress + 0x3160)
#define CQATTR5_RegisterSize 32
#define CQATTR5_RegisterResetValue 0xff
#define CQATTR5_RegisterResetMask 0xffffffff





#define CQATTR5_SIZE_BitAddressOffset 0
#define CQATTR5_SIZE_RegisterSize 16



#define CQATTR5_CQATTR5_RSVD_30_16_BitAddressOffset 16
#define CQATTR5_CQATTR5_RSVD_30_16_RegisterSize 15



#define CQATTR5_CQEN_BitAddressOffset 31
#define CQATTR5_CQEN_RegisterSize 1





#define CQLBA5 (DWC_ufshc_block_BaseAddress + 0x3164)
#define CQLBA5_RegisterSize 32
#define CQLBA5_RegisterResetValue 0x0
#define CQLBA5_RegisterResetMask 0xffffffff





#define CQLBA5_CQLBA5_RSVD_9_0_BitAddressOffset 0
#define CQLBA5_CQLBA5_RSVD_9_0_RegisterSize 10



#define CQLBA5_CQLBA_BitAddressOffset 10
#define CQLBA5_CQLBA_RegisterSize 22





#define CQUBA5 (DWC_ufshc_block_BaseAddress + 0x3168)
#define CQUBA5_RegisterSize 32
#define CQUBA5_RegisterResetValue 0x0
#define CQUBA5_RegisterResetMask 0xffffffff





#define CQUBA5_CQUBA_BitAddressOffset 0
#define CQUBA5_CQUBA_RegisterSize 32





#define CQDAO5 (DWC_ufshc_block_BaseAddress + 0x316c)
#define CQDAO5_RegisterSize 32
#define CQDAO5_RegisterResetValue 0x0
#define CQDAO5_RegisterResetMask 0xffffffff





#define CQDAO5_CQDAO_BitAddressOffset 0
#define CQDAO5_CQDAO_RegisterSize 32





#define CQISAO5 (DWC_ufshc_block_BaseAddress + 0x3170)
#define CQISAO5_RegisterSize 32
#define CQISAO5_RegisterResetValue 0x0
#define CQISAO5_RegisterResetMask 0xffffffff





#define CQISAO5_CQISAO_BitAddressOffset 0
#define CQISAO5_CQISAO_RegisterSize 32





#define CQCFG5 (DWC_ufshc_block_BaseAddress + 0x3174)
#define CQCFG5_RegisterSize 32
#define CQCFG5_RegisterResetValue 0x0
#define CQCFG5_RegisterResetMask 0xffffffff





#define CQCFG5_IAG_BitAddressOffset 0
#define CQCFG5_IAG_RegisterSize 5



#define CQCFG5_CQCFG5_RSVD_7_5_BitAddressOffset 5
#define CQCFG5_CQCFG5_RSVD_7_5_RegisterSize 3



#define CQCFG5_IAGVLD_BitAddressOffset 8
#define CQCFG5_IAGVLD_RegisterSize 1



#define CQCFG5_CQCFG5_RSVD_31_9_BitAddressOffset 9
#define CQCFG5_CQCFG5_RSVD_31_9_RegisterSize 23





#define SQATTR6 (DWC_ufshc_block_BaseAddress + 0x3180)
#define SQATTR6_RegisterSize 32
#define SQATTR6_RegisterResetValue 0xff
#define SQATTR6_RegisterResetMask 0xffffffff





#define SQATTR6_SIZE_BitAddressOffset 0
#define SQATTR6_SIZE_RegisterSize 16



#define SQATTR6_CQID_BitAddressOffset 16
#define SQATTR6_CQID_RegisterSize 4



#define SQATTR6_SQATTR6_RSVD_27_24_BitAddressOffset 20
#define SQATTR6_SQATTR6_RSVD_27_24_RegisterSize 8



#define SQATTR6_SQPL_BitAddressOffset 28
#define SQATTR6_SQPL_RegisterSize 3



#define SQATTR6_SQEN_BitAddressOffset 31
#define SQATTR6_SQEN_RegisterSize 1





#define SQLBA6 (DWC_ufshc_block_BaseAddress + 0x3184)
#define SQLBA6_RegisterSize 32
#define SQLBA6_RegisterResetValue 0x0
#define SQLBA6_RegisterResetMask 0xffffffff





#define SQLBA6_SQLBA6_RSVD_9_0_BitAddressOffset 0
#define SQLBA6_SQLBA6_RSVD_9_0_RegisterSize 10



#define SQLBA6_SQLBA_BitAddressOffset 10
#define SQLBA6_SQLBA_RegisterSize 22





#define SQUBA6 (DWC_ufshc_block_BaseAddress + 0x3188)
#define SQUBA6_RegisterSize 32
#define SQUBA6_RegisterResetValue 0x0
#define SQUBA6_RegisterResetMask 0xffffffff





#define SQUBA6_SQUBA_BitAddressOffset 0
#define SQUBA6_SQUBA_RegisterSize 32





#define SQDAO6 (DWC_ufshc_block_BaseAddress + 0x318c)
#define SQDAO6_RegisterSize 32
#define SQDAO6_RegisterResetValue 0x0
#define SQDAO6_RegisterResetMask 0xffffffff





#define SQDAO6_SQDAO_BitAddressOffset 0
#define SQDAO6_SQDAO_RegisterSize 32





#define SQISAO6 (DWC_ufshc_block_BaseAddress + 0x3190)
#define SQISAO6_RegisterSize 32
#define SQISAO6_RegisterResetValue 0x0
#define SQISAO6_RegisterResetMask 0xffffffff





#define SQISAO6_SQISAO_BitAddressOffset 0
#define SQISAO6_SQISAO_RegisterSize 32





#define SQCFG6 (DWC_ufshc_block_BaseAddress + 0x3194)
#define SQCFG6_RegisterSize 32
#define SQCFG6_RegisterResetValue 0x0
#define SQCFG6_RegisterResetMask 0xffffffff





#define SQCFG6_IAG_BitAddressOffset 0
#define SQCFG6_IAG_RegisterSize 5



#define SQCFG6_SQCFG6_RSVD_7_5_BitAddressOffset 5
#define SQCFG6_SQCFG6_RSVD_7_5_RegisterSize 3



#define SQCFG6_IAGVLD_BitAddressOffset 8
#define SQCFG6_IAGVLD_RegisterSize 1



#define SQCFG6_SQCFG6_RSVD_31_9_BitAddressOffset 9
#define SQCFG6_SQCFG6_RSVD_31_9_RegisterSize 23





#define CQATTR6 (DWC_ufshc_block_BaseAddress + 0x31a0)
#define CQATTR6_RegisterSize 32
#define CQATTR6_RegisterResetValue 0xff
#define CQATTR6_RegisterResetMask 0xffffffff





#define CQATTR6_SIZE_BitAddressOffset 0
#define CQATTR6_SIZE_RegisterSize 16



#define CQATTR6_CQATTR6_RSVD_30_16_BitAddressOffset 16
#define CQATTR6_CQATTR6_RSVD_30_16_RegisterSize 15



#define CQATTR6_CQEN_BitAddressOffset 31
#define CQATTR6_CQEN_RegisterSize 1





#define CQLBA6 (DWC_ufshc_block_BaseAddress + 0x31a4)
#define CQLBA6_RegisterSize 32
#define CQLBA6_RegisterResetValue 0x0
#define CQLBA6_RegisterResetMask 0xffffffff





#define CQLBA6_CQLBA6_RSVD_9_0_BitAddressOffset 0
#define CQLBA6_CQLBA6_RSVD_9_0_RegisterSize 10



#define CQLBA6_CQLBA_BitAddressOffset 10
#define CQLBA6_CQLBA_RegisterSize 22





#define CQUBA6 (DWC_ufshc_block_BaseAddress + 0x31a8)
#define CQUBA6_RegisterSize 32
#define CQUBA6_RegisterResetValue 0x0
#define CQUBA6_RegisterResetMask 0xffffffff





#define CQUBA6_CQUBA_BitAddressOffset 0
#define CQUBA6_CQUBA_RegisterSize 32





#define CQDAO6 (DWC_ufshc_block_BaseAddress + 0x31ac)
#define CQDAO6_RegisterSize 32
#define CQDAO6_RegisterResetValue 0x0
#define CQDAO6_RegisterResetMask 0xffffffff





#define CQDAO6_CQDAO_BitAddressOffset 0
#define CQDAO6_CQDAO_RegisterSize 32





#define CQISAO6 (DWC_ufshc_block_BaseAddress + 0x31b0)
#define CQISAO6_RegisterSize 32
#define CQISAO6_RegisterResetValue 0x0
#define CQISAO6_RegisterResetMask 0xffffffff





#define CQISAO6_CQISAO_BitAddressOffset 0
#define CQISAO6_CQISAO_RegisterSize 32





#define CQCFG6 (DWC_ufshc_block_BaseAddress + 0x31b4)
#define CQCFG6_RegisterSize 32
#define CQCFG6_RegisterResetValue 0x0
#define CQCFG6_RegisterResetMask 0xffffffff





#define CQCFG6_IAG_BitAddressOffset 0
#define CQCFG6_IAG_RegisterSize 5



#define CQCFG6_CQCFG6_RSVD_7_5_BitAddressOffset 5
#define CQCFG6_CQCFG6_RSVD_7_5_RegisterSize 3



#define CQCFG6_IAGVLD_BitAddressOffset 8
#define CQCFG6_IAGVLD_RegisterSize 1



#define CQCFG6_CQCFG6_RSVD_31_9_BitAddressOffset 9
#define CQCFG6_CQCFG6_RSVD_31_9_RegisterSize 23





#define SQATTR7 (DWC_ufshc_block_BaseAddress + 0x31c0)
#define SQATTR7_RegisterSize 32
#define SQATTR7_RegisterResetValue 0xff
#define SQATTR7_RegisterResetMask 0xffffffff





#define SQATTR7_SIZE_BitAddressOffset 0
#define SQATTR7_SIZE_RegisterSize 16



#define SQATTR7_CQID_BitAddressOffset 16
#define SQATTR7_CQID_RegisterSize 4



#define SQATTR7_SQATTR7_RSVD_27_24_BitAddressOffset 20
#define SQATTR7_SQATTR7_RSVD_27_24_RegisterSize 8



#define SQATTR7_SQPL_BitAddressOffset 28
#define SQATTR7_SQPL_RegisterSize 3



#define SQATTR7_SQEN_BitAddressOffset 31
#define SQATTR7_SQEN_RegisterSize 1





#define SQLBA7 (DWC_ufshc_block_BaseAddress + 0x31c4)
#define SQLBA7_RegisterSize 32
#define SQLBA7_RegisterResetValue 0x0
#define SQLBA7_RegisterResetMask 0xffffffff





#define SQLBA7_SQLBA7_RSVD_9_0_BitAddressOffset 0
#define SQLBA7_SQLBA7_RSVD_9_0_RegisterSize 10



#define SQLBA7_SQLBA_BitAddressOffset 10
#define SQLBA7_SQLBA_RegisterSize 22





#define SQUBA7 (DWC_ufshc_block_BaseAddress + 0x31c8)
#define SQUBA7_RegisterSize 32
#define SQUBA7_RegisterResetValue 0x0
#define SQUBA7_RegisterResetMask 0xffffffff





#define SQUBA7_SQUBA_BitAddressOffset 0
#define SQUBA7_SQUBA_RegisterSize 32





#define SQDAO7 (DWC_ufshc_block_BaseAddress + 0x31cc)
#define SQDAO7_RegisterSize 32
#define SQDAO7_RegisterResetValue 0x0
#define SQDAO7_RegisterResetMask 0xffffffff





#define SQDAO7_SQDAO_BitAddressOffset 0
#define SQDAO7_SQDAO_RegisterSize 32





#define SQISAO7 (DWC_ufshc_block_BaseAddress + 0x31d0)
#define SQISAO7_RegisterSize 32
#define SQISAO7_RegisterResetValue 0x0
#define SQISAO7_RegisterResetMask 0xffffffff





#define SQISAO7_SQISAO_BitAddressOffset 0
#define SQISAO7_SQISAO_RegisterSize 32





#define SQCFG7 (DWC_ufshc_block_BaseAddress + 0x31d4)
#define SQCFG7_RegisterSize 32
#define SQCFG7_RegisterResetValue 0x0
#define SQCFG7_RegisterResetMask 0xffffffff





#define SQCFG7_IAG_BitAddressOffset 0
#define SQCFG7_IAG_RegisterSize 5



#define SQCFG7_SQCFG7_RSVD_7_5_BitAddressOffset 5
#define SQCFG7_SQCFG7_RSVD_7_5_RegisterSize 3



#define SQCFG7_IAGVLD_BitAddressOffset 8
#define SQCFG7_IAGVLD_RegisterSize 1



#define SQCFG7_SQCFG7_RSVD_31_9_BitAddressOffset 9
#define SQCFG7_SQCFG7_RSVD_31_9_RegisterSize 23





#define CQATTR7 (DWC_ufshc_block_BaseAddress + 0x31e0)
#define CQATTR7_RegisterSize 32
#define CQATTR7_RegisterResetValue 0xff
#define CQATTR7_RegisterResetMask 0xffffffff





#define CQATTR7_SIZE_BitAddressOffset 0
#define CQATTR7_SIZE_RegisterSize 16



#define CQATTR7_CQATTR7_RSVD_30_16_BitAddressOffset 16
#define CQATTR7_CQATTR7_RSVD_30_16_RegisterSize 15



#define CQATTR7_CQEN_BitAddressOffset 31
#define CQATTR7_CQEN_RegisterSize 1





#define CQLBA7 (DWC_ufshc_block_BaseAddress + 0x31e4)
#define CQLBA7_RegisterSize 32
#define CQLBA7_RegisterResetValue 0x0
#define CQLBA7_RegisterResetMask 0xffffffff





#define CQLBA7_CQLBA7_RSVD_9_0_BitAddressOffset 0
#define CQLBA7_CQLBA7_RSVD_9_0_RegisterSize 10



#define CQLBA7_CQLBA_BitAddressOffset 10
#define CQLBA7_CQLBA_RegisterSize 22





#define CQUBA7 (DWC_ufshc_block_BaseAddress + 0x31e8)
#define CQUBA7_RegisterSize 32
#define CQUBA7_RegisterResetValue 0x0
#define CQUBA7_RegisterResetMask 0xffffffff





#define CQUBA7_CQUBA_BitAddressOffset 0
#define CQUBA7_CQUBA_RegisterSize 32





#define CQDAO7 (DWC_ufshc_block_BaseAddress + 0x31ec)
#define CQDAO7_RegisterSize 32
#define CQDAO7_RegisterResetValue 0x0
#define CQDAO7_RegisterResetMask 0xffffffff





#define CQDAO7_CQDAO_BitAddressOffset 0
#define CQDAO7_CQDAO_RegisterSize 32





#define CQISAO7 (DWC_ufshc_block_BaseAddress + 0x31f0)
#define CQISAO7_RegisterSize 32
#define CQISAO7_RegisterResetValue 0x0
#define CQISAO7_RegisterResetMask 0xffffffff





#define CQISAO7_CQISAO_BitAddressOffset 0
#define CQISAO7_CQISAO_RegisterSize 32





#define CQCFG7 (DWC_ufshc_block_BaseAddress + 0x31f4)
#define CQCFG7_RegisterSize 32
#define CQCFG7_RegisterResetValue 0x0
#define CQCFG7_RegisterResetMask 0xffffffff





#define CQCFG7_IAG_BitAddressOffset 0
#define CQCFG7_IAG_RegisterSize 5



#define CQCFG7_CQCFG7_RSVD_7_5_BitAddressOffset 5
#define CQCFG7_CQCFG7_RSVD_7_5_RegisterSize 3



#define CQCFG7_IAGVLD_BitAddressOffset 8
#define CQCFG7_IAGVLD_RegisterSize 1



#define CQCFG7_CQCFG7_RSVD_31_9_BitAddressOffset 9
#define CQCFG7_CQCFG7_RSVD_31_9_RegisterSize 23





#define SQATTR8 (DWC_ufshc_block_BaseAddress + 0x3200)
#define SQATTR8_RegisterSize 32
#define SQATTR8_RegisterResetValue 0xff
#define SQATTR8_RegisterResetMask 0xffffffff





#define SQATTR8_SIZE_BitAddressOffset 0
#define SQATTR8_SIZE_RegisterSize 16



#define SQATTR8_CQID_BitAddressOffset 16
#define SQATTR8_CQID_RegisterSize 4



#define SQATTR8_SQATTR8_RSVD_27_24_BitAddressOffset 20
#define SQATTR8_SQATTR8_RSVD_27_24_RegisterSize 8



#define SQATTR8_SQPL_BitAddressOffset 28
#define SQATTR8_SQPL_RegisterSize 3



#define SQATTR8_SQEN_BitAddressOffset 31
#define SQATTR8_SQEN_RegisterSize 1





#define SQLBA8 (DWC_ufshc_block_BaseAddress + 0x3204)
#define SQLBA8_RegisterSize 32
#define SQLBA8_RegisterResetValue 0x0
#define SQLBA8_RegisterResetMask 0xffffffff





#define SQLBA8_SQLBA8_RSVD_9_0_BitAddressOffset 0
#define SQLBA8_SQLBA8_RSVD_9_0_RegisterSize 10



#define SQLBA8_SQLBA_BitAddressOffset 10
#define SQLBA8_SQLBA_RegisterSize 22





#define SQUBA8 (DWC_ufshc_block_BaseAddress + 0x3208)
#define SQUBA8_RegisterSize 32
#define SQUBA8_RegisterResetValue 0x0
#define SQUBA8_RegisterResetMask 0xffffffff





#define SQUBA8_SQUBA_BitAddressOffset 0
#define SQUBA8_SQUBA_RegisterSize 32





#define SQDAO8 (DWC_ufshc_block_BaseAddress + 0x320c)
#define SQDAO8_RegisterSize 32
#define SQDAO8_RegisterResetValue 0x0
#define SQDAO8_RegisterResetMask 0xffffffff





#define SQDAO8_SQDAO_BitAddressOffset 0
#define SQDAO8_SQDAO_RegisterSize 32





#define SQISAO8 (DWC_ufshc_block_BaseAddress + 0x3210)
#define SQISAO8_RegisterSize 32
#define SQISAO8_RegisterResetValue 0x0
#define SQISAO8_RegisterResetMask 0xffffffff





#define SQISAO8_SQISAO_BitAddressOffset 0
#define SQISAO8_SQISAO_RegisterSize 32





#define SQCFG8 (DWC_ufshc_block_BaseAddress + 0x3214)
#define SQCFG8_RegisterSize 32
#define SQCFG8_RegisterResetValue 0x0
#define SQCFG8_RegisterResetMask 0xffffffff





#define SQCFG8_IAG_BitAddressOffset 0
#define SQCFG8_IAG_RegisterSize 5



#define SQCFG8_SQCFG8_RSVD_7_5_BitAddressOffset 5
#define SQCFG8_SQCFG8_RSVD_7_5_RegisterSize 3



#define SQCFG8_IAGVLD_BitAddressOffset 8
#define SQCFG8_IAGVLD_RegisterSize 1



#define SQCFG8_SQCFG8_RSVD_31_9_BitAddressOffset 9
#define SQCFG8_SQCFG8_RSVD_31_9_RegisterSize 23





#define CQATTR8 (DWC_ufshc_block_BaseAddress + 0x3220)
#define CQATTR8_RegisterSize 32
#define CQATTR8_RegisterResetValue 0xff
#define CQATTR8_RegisterResetMask 0xffffffff





#define CQATTR8_SIZE_BitAddressOffset 0
#define CQATTR8_SIZE_RegisterSize 16



#define CQATTR8_CQATTR8_RSVD_30_16_BitAddressOffset 16
#define CQATTR8_CQATTR8_RSVD_30_16_RegisterSize 15



#define CQATTR8_CQEN_BitAddressOffset 31
#define CQATTR8_CQEN_RegisterSize 1





#define CQLBA8 (DWC_ufshc_block_BaseAddress + 0x3224)
#define CQLBA8_RegisterSize 32
#define CQLBA8_RegisterResetValue 0x0
#define CQLBA8_RegisterResetMask 0xffffffff





#define CQLBA8_CQLBA8_RSVD_9_0_BitAddressOffset 0
#define CQLBA8_CQLBA8_RSVD_9_0_RegisterSize 10



#define CQLBA8_CQLBA_BitAddressOffset 10
#define CQLBA8_CQLBA_RegisterSize 22





#define CQUBA8 (DWC_ufshc_block_BaseAddress + 0x3228)
#define CQUBA8_RegisterSize 32
#define CQUBA8_RegisterResetValue 0x0
#define CQUBA8_RegisterResetMask 0xffffffff





#define CQUBA8_CQUBA_BitAddressOffset 0
#define CQUBA8_CQUBA_RegisterSize 32





#define CQDAO8 (DWC_ufshc_block_BaseAddress + 0x322c)
#define CQDAO8_RegisterSize 32
#define CQDAO8_RegisterResetValue 0x0
#define CQDAO8_RegisterResetMask 0xffffffff





#define CQDAO8_CQDAO_BitAddressOffset 0
#define CQDAO8_CQDAO_RegisterSize 32





#define CQISAO8 (DWC_ufshc_block_BaseAddress + 0x3230)
#define CQISAO8_RegisterSize 32
#define CQISAO8_RegisterResetValue 0x0
#define CQISAO8_RegisterResetMask 0xffffffff





#define CQISAO8_CQISAO_BitAddressOffset 0
#define CQISAO8_CQISAO_RegisterSize 32





#define CQCFG8 (DWC_ufshc_block_BaseAddress + 0x3234)
#define CQCFG8_RegisterSize 32
#define CQCFG8_RegisterResetValue 0x0
#define CQCFG8_RegisterResetMask 0xffffffff





#define CQCFG8_IAG_BitAddressOffset 0
#define CQCFG8_IAG_RegisterSize 5



#define CQCFG8_CQCFG8_RSVD_7_5_BitAddressOffset 5
#define CQCFG8_CQCFG8_RSVD_7_5_RegisterSize 3



#define CQCFG8_IAGVLD_BitAddressOffset 8
#define CQCFG8_IAGVLD_RegisterSize 1



#define CQCFG8_CQCFG8_RSVD_31_9_BitAddressOffset 9
#define CQCFG8_CQCFG8_RSVD_31_9_RegisterSize 23





#define SQATTR9 (DWC_ufshc_block_BaseAddress + 0x3240)
#define SQATTR9_RegisterSize 32
#define SQATTR9_RegisterResetValue 0xff
#define SQATTR9_RegisterResetMask 0xffffffff





#define SQATTR9_SIZE_BitAddressOffset 0
#define SQATTR9_SIZE_RegisterSize 16



#define SQATTR9_CQID_BitAddressOffset 16
#define SQATTR9_CQID_RegisterSize 4



#define SQATTR9_SQATTR9_RSVD_27_24_BitAddressOffset 20
#define SQATTR9_SQATTR9_RSVD_27_24_RegisterSize 8



#define SQATTR9_SQPL_BitAddressOffset 28
#define SQATTR9_SQPL_RegisterSize 3



#define SQATTR9_SQEN_BitAddressOffset 31
#define SQATTR9_SQEN_RegisterSize 1





#define SQLBA9 (DWC_ufshc_block_BaseAddress + 0x3244)
#define SQLBA9_RegisterSize 32
#define SQLBA9_RegisterResetValue 0x0
#define SQLBA9_RegisterResetMask 0xffffffff





#define SQLBA9_SQLBA9_RSVD_9_0_BitAddressOffset 0
#define SQLBA9_SQLBA9_RSVD_9_0_RegisterSize 10



#define SQLBA9_SQLBA_BitAddressOffset 10
#define SQLBA9_SQLBA_RegisterSize 22





#define SQUBA9 (DWC_ufshc_block_BaseAddress + 0x3248)
#define SQUBA9_RegisterSize 32
#define SQUBA9_RegisterResetValue 0x0
#define SQUBA9_RegisterResetMask 0xffffffff





#define SQUBA9_SQUBA_BitAddressOffset 0
#define SQUBA9_SQUBA_RegisterSize 32





#define SQDAO9 (DWC_ufshc_block_BaseAddress + 0x324c)
#define SQDAO9_RegisterSize 32
#define SQDAO9_RegisterResetValue 0x0
#define SQDAO9_RegisterResetMask 0xffffffff





#define SQDAO9_SQDAO_BitAddressOffset 0
#define SQDAO9_SQDAO_RegisterSize 32





#define SQISAO9 (DWC_ufshc_block_BaseAddress + 0x3250)
#define SQISAO9_RegisterSize 32
#define SQISAO9_RegisterResetValue 0x0
#define SQISAO9_RegisterResetMask 0xffffffff





#define SQISAO9_SQISAO_BitAddressOffset 0
#define SQISAO9_SQISAO_RegisterSize 32





#define SQCFG9 (DWC_ufshc_block_BaseAddress + 0x3254)
#define SQCFG9_RegisterSize 32
#define SQCFG9_RegisterResetValue 0x0
#define SQCFG9_RegisterResetMask 0xffffffff





#define SQCFG9_IAG_BitAddressOffset 0
#define SQCFG9_IAG_RegisterSize 5



#define SQCFG9_SQCFG9_RSVD_7_5_BitAddressOffset 5
#define SQCFG9_SQCFG9_RSVD_7_5_RegisterSize 3



#define SQCFG9_IAGVLD_BitAddressOffset 8
#define SQCFG9_IAGVLD_RegisterSize 1



#define SQCFG9_SQCFG9_RSVD_31_9_BitAddressOffset 9
#define SQCFG9_SQCFG9_RSVD_31_9_RegisterSize 23





#define CQATTR9 (DWC_ufshc_block_BaseAddress + 0x3260)
#define CQATTR9_RegisterSize 32
#define CQATTR9_RegisterResetValue 0xff
#define CQATTR9_RegisterResetMask 0xffffffff





#define CQATTR9_SIZE_BitAddressOffset 0
#define CQATTR9_SIZE_RegisterSize 16



#define CQATTR9_CQATTR9_RSVD_30_16_BitAddressOffset 16
#define CQATTR9_CQATTR9_RSVD_30_16_RegisterSize 15



#define CQATTR9_CQEN_BitAddressOffset 31
#define CQATTR9_CQEN_RegisterSize 1





#define CQLBA9 (DWC_ufshc_block_BaseAddress + 0x3264)
#define CQLBA9_RegisterSize 32
#define CQLBA9_RegisterResetValue 0x0
#define CQLBA9_RegisterResetMask 0xffffffff





#define CQLBA9_CQLBA9_RSVD_9_0_BitAddressOffset 0
#define CQLBA9_CQLBA9_RSVD_9_0_RegisterSize 10



#define CQLBA9_CQLBA_BitAddressOffset 10
#define CQLBA9_CQLBA_RegisterSize 22





#define CQUBA9 (DWC_ufshc_block_BaseAddress + 0x3268)
#define CQUBA9_RegisterSize 32
#define CQUBA9_RegisterResetValue 0x0
#define CQUBA9_RegisterResetMask 0xffffffff





#define CQUBA9_CQUBA_BitAddressOffset 0
#define CQUBA9_CQUBA_RegisterSize 32





#define CQDAO9 (DWC_ufshc_block_BaseAddress + 0x326c)
#define CQDAO9_RegisterSize 32
#define CQDAO9_RegisterResetValue 0x0
#define CQDAO9_RegisterResetMask 0xffffffff





#define CQDAO9_CQDAO_BitAddressOffset 0
#define CQDAO9_CQDAO_RegisterSize 32





#define CQISAO9 (DWC_ufshc_block_BaseAddress + 0x3270)
#define CQISAO9_RegisterSize 32
#define CQISAO9_RegisterResetValue 0x0
#define CQISAO9_RegisterResetMask 0xffffffff





#define CQISAO9_CQISAO_BitAddressOffset 0
#define CQISAO9_CQISAO_RegisterSize 32





#define CQCFG9 (DWC_ufshc_block_BaseAddress + 0x3274)
#define CQCFG9_RegisterSize 32
#define CQCFG9_RegisterResetValue 0x0
#define CQCFG9_RegisterResetMask 0xffffffff





#define CQCFG9_IAG_BitAddressOffset 0
#define CQCFG9_IAG_RegisterSize 5



#define CQCFG9_CQCFG9_RSVD_7_5_BitAddressOffset 5
#define CQCFG9_CQCFG9_RSVD_7_5_RegisterSize 3



#define CQCFG9_IAGVLD_BitAddressOffset 8
#define CQCFG9_IAGVLD_RegisterSize 1



#define CQCFG9_CQCFG9_RSVD_31_9_BitAddressOffset 9
#define CQCFG9_CQCFG9_RSVD_31_9_RegisterSize 23





#define SQATTR10 (DWC_ufshc_block_BaseAddress + 0x3280)
#define SQATTR10_RegisterSize 32
#define SQATTR10_RegisterResetValue 0xff
#define SQATTR10_RegisterResetMask 0xffffffff





#define SQATTR10_SIZE_BitAddressOffset 0
#define SQATTR10_SIZE_RegisterSize 16



#define SQATTR10_CQID_BitAddressOffset 16
#define SQATTR10_CQID_RegisterSize 4



#define SQATTR10_SQATTR10_RSVD_27_24_BitAddressOffset 20
#define SQATTR10_SQATTR10_RSVD_27_24_RegisterSize 8



#define SQATTR10_SQPL_BitAddressOffset 28
#define SQATTR10_SQPL_RegisterSize 3



#define SQATTR10_SQEN_BitAddressOffset 31
#define SQATTR10_SQEN_RegisterSize 1





#define SQLBA10 (DWC_ufshc_block_BaseAddress + 0x3284)
#define SQLBA10_RegisterSize 32
#define SQLBA10_RegisterResetValue 0x0
#define SQLBA10_RegisterResetMask 0xffffffff





#define SQLBA10_SQLBA10_RSVD_9_0_BitAddressOffset 0
#define SQLBA10_SQLBA10_RSVD_9_0_RegisterSize 10



#define SQLBA10_SQLBA_BitAddressOffset 10
#define SQLBA10_SQLBA_RegisterSize 22





#define SQUBA10 (DWC_ufshc_block_BaseAddress + 0x3288)
#define SQUBA10_RegisterSize 32
#define SQUBA10_RegisterResetValue 0x0
#define SQUBA10_RegisterResetMask 0xffffffff





#define SQUBA10_SQUBA_BitAddressOffset 0
#define SQUBA10_SQUBA_RegisterSize 32





#define SQDAO10 (DWC_ufshc_block_BaseAddress + 0x328c)
#define SQDAO10_RegisterSize 32
#define SQDAO10_RegisterResetValue 0x0
#define SQDAO10_RegisterResetMask 0xffffffff





#define SQDAO10_SQDAO_BitAddressOffset 0
#define SQDAO10_SQDAO_RegisterSize 32





#define SQISAO10 (DWC_ufshc_block_BaseAddress + 0x3290)
#define SQISAO10_RegisterSize 32
#define SQISAO10_RegisterResetValue 0x0
#define SQISAO10_RegisterResetMask 0xffffffff





#define SQISAO10_SQISAO_BitAddressOffset 0
#define SQISAO10_SQISAO_RegisterSize 32





#define SQCFG10 (DWC_ufshc_block_BaseAddress + 0x3294)
#define SQCFG10_RegisterSize 32
#define SQCFG10_RegisterResetValue 0x0
#define SQCFG10_RegisterResetMask 0xffffffff





#define SQCFG10_IAG_BitAddressOffset 0
#define SQCFG10_IAG_RegisterSize 5



#define SQCFG10_SQCFG10_RSVD_7_5_BitAddressOffset 5
#define SQCFG10_SQCFG10_RSVD_7_5_RegisterSize 3



#define SQCFG10_IAGVLD_BitAddressOffset 8
#define SQCFG10_IAGVLD_RegisterSize 1



#define SQCFG10_SQCFG10_RSVD_31_9_BitAddressOffset 9
#define SQCFG10_SQCFG10_RSVD_31_9_RegisterSize 23





#define CQATTR10 (DWC_ufshc_block_BaseAddress + 0x32a0)
#define CQATTR10_RegisterSize 32
#define CQATTR10_RegisterResetValue 0xff
#define CQATTR10_RegisterResetMask 0xffffffff





#define CQATTR10_SIZE_BitAddressOffset 0
#define CQATTR10_SIZE_RegisterSize 16



#define CQATTR10_CQATTR10_RSVD_30_16_BitAddressOffset 16
#define CQATTR10_CQATTR10_RSVD_30_16_RegisterSize 15



#define CQATTR10_CQEN_BitAddressOffset 31
#define CQATTR10_CQEN_RegisterSize 1





#define CQLBA10 (DWC_ufshc_block_BaseAddress + 0x32a4)
#define CQLBA10_RegisterSize 32
#define CQLBA10_RegisterResetValue 0x0
#define CQLBA10_RegisterResetMask 0xffffffff





#define CQLBA10_CQLBA10_RSVD_9_0_BitAddressOffset 0
#define CQLBA10_CQLBA10_RSVD_9_0_RegisterSize 10



#define CQLBA10_CQLBA_BitAddressOffset 10
#define CQLBA10_CQLBA_RegisterSize 22





#define CQUBA10 (DWC_ufshc_block_BaseAddress + 0x32a8)
#define CQUBA10_RegisterSize 32
#define CQUBA10_RegisterResetValue 0x0
#define CQUBA10_RegisterResetMask 0xffffffff





#define CQUBA10_CQUBA_BitAddressOffset 0
#define CQUBA10_CQUBA_RegisterSize 32





#define CQDAO10 (DWC_ufshc_block_BaseAddress + 0x32ac)
#define CQDAO10_RegisterSize 32
#define CQDAO10_RegisterResetValue 0x0
#define CQDAO10_RegisterResetMask 0xffffffff





#define CQDAO10_CQDAO_BitAddressOffset 0
#define CQDAO10_CQDAO_RegisterSize 32





#define CQISAO10 (DWC_ufshc_block_BaseAddress + 0x32b0)
#define CQISAO10_RegisterSize 32
#define CQISAO10_RegisterResetValue 0x0
#define CQISAO10_RegisterResetMask 0xffffffff





#define CQISAO10_CQISAO_BitAddressOffset 0
#define CQISAO10_CQISAO_RegisterSize 32





#define CQCFG10 (DWC_ufshc_block_BaseAddress + 0x32b4)
#define CQCFG10_RegisterSize 32
#define CQCFG10_RegisterResetValue 0x0
#define CQCFG10_RegisterResetMask 0xffffffff





#define CQCFG10_IAG_BitAddressOffset 0
#define CQCFG10_IAG_RegisterSize 5



#define CQCFG10_CQCFG10_RSVD_7_5_BitAddressOffset 5
#define CQCFG10_CQCFG10_RSVD_7_5_RegisterSize 3



#define CQCFG10_IAGVLD_BitAddressOffset 8
#define CQCFG10_IAGVLD_RegisterSize 1



#define CQCFG10_CQCFG10_RSVD_31_9_BitAddressOffset 9
#define CQCFG10_CQCFG10_RSVD_31_9_RegisterSize 23





#define SQATTR11 (DWC_ufshc_block_BaseAddress + 0x32c0)
#define SQATTR11_RegisterSize 32
#define SQATTR11_RegisterResetValue 0xff
#define SQATTR11_RegisterResetMask 0xffffffff





#define SQATTR11_SIZE_BitAddressOffset 0
#define SQATTR11_SIZE_RegisterSize 16



#define SQATTR11_CQID_BitAddressOffset 16
#define SQATTR11_CQID_RegisterSize 4



#define SQATTR11_SQATTR11_RSVD_27_24_BitAddressOffset 20
#define SQATTR11_SQATTR11_RSVD_27_24_RegisterSize 8



#define SQATTR11_SQPL_BitAddressOffset 28
#define SQATTR11_SQPL_RegisterSize 3



#define SQATTR11_SQEN_BitAddressOffset 31
#define SQATTR11_SQEN_RegisterSize 1





#define SQLBA11 (DWC_ufshc_block_BaseAddress + 0x32c4)
#define SQLBA11_RegisterSize 32
#define SQLBA11_RegisterResetValue 0x0
#define SQLBA11_RegisterResetMask 0xffffffff





#define SQLBA11_SQLBA11_RSVD_9_0_BitAddressOffset 0
#define SQLBA11_SQLBA11_RSVD_9_0_RegisterSize 10



#define SQLBA11_SQLBA_BitAddressOffset 10
#define SQLBA11_SQLBA_RegisterSize 22





#define SQUBA11 (DWC_ufshc_block_BaseAddress + 0x32c8)
#define SQUBA11_RegisterSize 32
#define SQUBA11_RegisterResetValue 0x0
#define SQUBA11_RegisterResetMask 0xffffffff





#define SQUBA11_SQUBA_BitAddressOffset 0
#define SQUBA11_SQUBA_RegisterSize 32





#define SQDAO11 (DWC_ufshc_block_BaseAddress + 0x32cc)
#define SQDAO11_RegisterSize 32
#define SQDAO11_RegisterResetValue 0x0
#define SQDAO11_RegisterResetMask 0xffffffff





#define SQDAO11_SQDAO_BitAddressOffset 0
#define SQDAO11_SQDAO_RegisterSize 32





#define SQISAO11 (DWC_ufshc_block_BaseAddress + 0x32d0)
#define SQISAO11_RegisterSize 32
#define SQISAO11_RegisterResetValue 0x0
#define SQISAO11_RegisterResetMask 0xffffffff





#define SQISAO11_SQISAO_BitAddressOffset 0
#define SQISAO11_SQISAO_RegisterSize 32





#define SQCFG11 (DWC_ufshc_block_BaseAddress + 0x32d4)
#define SQCFG11_RegisterSize 32
#define SQCFG11_RegisterResetValue 0x0
#define SQCFG11_RegisterResetMask 0xffffffff





#define SQCFG11_IAG_BitAddressOffset 0
#define SQCFG11_IAG_RegisterSize 5



#define SQCFG11_SQCFG11_RSVD_7_5_BitAddressOffset 5
#define SQCFG11_SQCFG11_RSVD_7_5_RegisterSize 3



#define SQCFG11_IAGVLD_BitAddressOffset 8
#define SQCFG11_IAGVLD_RegisterSize 1



#define SQCFG11_SQCFG11_RSVD_31_9_BitAddressOffset 9
#define SQCFG11_SQCFG11_RSVD_31_9_RegisterSize 23





#define CQATTR11 (DWC_ufshc_block_BaseAddress + 0x32e0)
#define CQATTR11_RegisterSize 32
#define CQATTR11_RegisterResetValue 0xff
#define CQATTR11_RegisterResetMask 0xffffffff





#define CQATTR11_SIZE_BitAddressOffset 0
#define CQATTR11_SIZE_RegisterSize 16



#define CQATTR11_CQATTR11_RSVD_30_16_BitAddressOffset 16
#define CQATTR11_CQATTR11_RSVD_30_16_RegisterSize 15



#define CQATTR11_CQEN_BitAddressOffset 31
#define CQATTR11_CQEN_RegisterSize 1





#define CQLBA11 (DWC_ufshc_block_BaseAddress + 0x32e4)
#define CQLBA11_RegisterSize 32
#define CQLBA11_RegisterResetValue 0x0
#define CQLBA11_RegisterResetMask 0xffffffff





#define CQLBA11_CQLBA11_RSVD_9_0_BitAddressOffset 0
#define CQLBA11_CQLBA11_RSVD_9_0_RegisterSize 10



#define CQLBA11_CQLBA_BitAddressOffset 10
#define CQLBA11_CQLBA_RegisterSize 22





#define CQUBA11 (DWC_ufshc_block_BaseAddress + 0x32e8)
#define CQUBA11_RegisterSize 32
#define CQUBA11_RegisterResetValue 0x0
#define CQUBA11_RegisterResetMask 0xffffffff





#define CQUBA11_CQUBA_BitAddressOffset 0
#define CQUBA11_CQUBA_RegisterSize 32





#define CQDAO11 (DWC_ufshc_block_BaseAddress + 0x32ec)
#define CQDAO11_RegisterSize 32
#define CQDAO11_RegisterResetValue 0x0
#define CQDAO11_RegisterResetMask 0xffffffff





#define CQDAO11_CQDAO_BitAddressOffset 0
#define CQDAO11_CQDAO_RegisterSize 32





#define CQISAO11 (DWC_ufshc_block_BaseAddress + 0x32f0)
#define CQISAO11_RegisterSize 32
#define CQISAO11_RegisterResetValue 0x0
#define CQISAO11_RegisterResetMask 0xffffffff





#define CQISAO11_CQISAO_BitAddressOffset 0
#define CQISAO11_CQISAO_RegisterSize 32





#define CQCFG11 (DWC_ufshc_block_BaseAddress + 0x32f4)
#define CQCFG11_RegisterSize 32
#define CQCFG11_RegisterResetValue 0x0
#define CQCFG11_RegisterResetMask 0xffffffff





#define CQCFG11_IAG_BitAddressOffset 0
#define CQCFG11_IAG_RegisterSize 5



#define CQCFG11_CQCFG11_RSVD_7_5_BitAddressOffset 5
#define CQCFG11_CQCFG11_RSVD_7_5_RegisterSize 3



#define CQCFG11_IAGVLD_BitAddressOffset 8
#define CQCFG11_IAGVLD_RegisterSize 1



#define CQCFG11_CQCFG11_RSVD_31_9_BitAddressOffset 9
#define CQCFG11_CQCFG11_RSVD_31_9_RegisterSize 23





#define SQATTR12 (DWC_ufshc_block_BaseAddress + 0x3300)
#define SQATTR12_RegisterSize 32
#define SQATTR12_RegisterResetValue 0xff
#define SQATTR12_RegisterResetMask 0xffffffff





#define SQATTR12_SIZE_BitAddressOffset 0
#define SQATTR12_SIZE_RegisterSize 16



#define SQATTR12_CQID_BitAddressOffset 16
#define SQATTR12_CQID_RegisterSize 4



#define SQATTR12_SQATTR12_RSVD_27_24_BitAddressOffset 20
#define SQATTR12_SQATTR12_RSVD_27_24_RegisterSize 8



#define SQATTR12_SQPL_BitAddressOffset 28
#define SQATTR12_SQPL_RegisterSize 3



#define SQATTR12_SQEN_BitAddressOffset 31
#define SQATTR12_SQEN_RegisterSize 1





#define SQLBA12 (DWC_ufshc_block_BaseAddress + 0x3304)
#define SQLBA12_RegisterSize 32
#define SQLBA12_RegisterResetValue 0x0
#define SQLBA12_RegisterResetMask 0xffffffff





#define SQLBA12_SQLBA12_RSVD_9_0_BitAddressOffset 0
#define SQLBA12_SQLBA12_RSVD_9_0_RegisterSize 10



#define SQLBA12_SQLBA_BitAddressOffset 10
#define SQLBA12_SQLBA_RegisterSize 22





#define SQUBA12 (DWC_ufshc_block_BaseAddress + 0x3308)
#define SQUBA12_RegisterSize 32
#define SQUBA12_RegisterResetValue 0x0
#define SQUBA12_RegisterResetMask 0xffffffff





#define SQUBA12_SQUBA_BitAddressOffset 0
#define SQUBA12_SQUBA_RegisterSize 32





#define SQDAO12 (DWC_ufshc_block_BaseAddress + 0x330c)
#define SQDAO12_RegisterSize 32
#define SQDAO12_RegisterResetValue 0x0
#define SQDAO12_RegisterResetMask 0xffffffff





#define SQDAO12_SQDAO_BitAddressOffset 0
#define SQDAO12_SQDAO_RegisterSize 32





#define SQISAO12 (DWC_ufshc_block_BaseAddress + 0x3310)
#define SQISAO12_RegisterSize 32
#define SQISAO12_RegisterResetValue 0x0
#define SQISAO12_RegisterResetMask 0xffffffff





#define SQISAO12_SQISAO_BitAddressOffset 0
#define SQISAO12_SQISAO_RegisterSize 32





#define SQCFG12 (DWC_ufshc_block_BaseAddress + 0x3314)
#define SQCFG12_RegisterSize 32
#define SQCFG12_RegisterResetValue 0x0
#define SQCFG12_RegisterResetMask 0xffffffff





#define SQCFG12_IAG_BitAddressOffset 0
#define SQCFG12_IAG_RegisterSize 5



#define SQCFG12_SQCFG12_RSVD_7_5_BitAddressOffset 5
#define SQCFG12_SQCFG12_RSVD_7_5_RegisterSize 3



#define SQCFG12_IAGVLD_BitAddressOffset 8
#define SQCFG12_IAGVLD_RegisterSize 1



#define SQCFG12_SQCFG12_RSVD_31_9_BitAddressOffset 9
#define SQCFG12_SQCFG12_RSVD_31_9_RegisterSize 23





#define CQATTR12 (DWC_ufshc_block_BaseAddress + 0x3320)
#define CQATTR12_RegisterSize 32
#define CQATTR12_RegisterResetValue 0xff
#define CQATTR12_RegisterResetMask 0xffffffff





#define CQATTR12_SIZE_BitAddressOffset 0
#define CQATTR12_SIZE_RegisterSize 16



#define CQATTR12_CQATTR12_RSVD_30_16_BitAddressOffset 16
#define CQATTR12_CQATTR12_RSVD_30_16_RegisterSize 15



#define CQATTR12_CQEN_BitAddressOffset 31
#define CQATTR12_CQEN_RegisterSize 1





#define CQLBA12 (DWC_ufshc_block_BaseAddress + 0x3324)
#define CQLBA12_RegisterSize 32
#define CQLBA12_RegisterResetValue 0x0
#define CQLBA12_RegisterResetMask 0xffffffff





#define CQLBA12_CQLBA12_RSVD_9_0_BitAddressOffset 0
#define CQLBA12_CQLBA12_RSVD_9_0_RegisterSize 10



#define CQLBA12_CQLBA_BitAddressOffset 10
#define CQLBA12_CQLBA_RegisterSize 22





#define CQUBA12 (DWC_ufshc_block_BaseAddress + 0x3328)
#define CQUBA12_RegisterSize 32
#define CQUBA12_RegisterResetValue 0x0
#define CQUBA12_RegisterResetMask 0xffffffff





#define CQUBA12_CQUBA_BitAddressOffset 0
#define CQUBA12_CQUBA_RegisterSize 32





#define CQDAO12 (DWC_ufshc_block_BaseAddress + 0x332c)
#define CQDAO12_RegisterSize 32
#define CQDAO12_RegisterResetValue 0x0
#define CQDAO12_RegisterResetMask 0xffffffff





#define CQDAO12_CQDAO_BitAddressOffset 0
#define CQDAO12_CQDAO_RegisterSize 32





#define CQISAO12 (DWC_ufshc_block_BaseAddress + 0x3330)
#define CQISAO12_RegisterSize 32
#define CQISAO12_RegisterResetValue 0x0
#define CQISAO12_RegisterResetMask 0xffffffff





#define CQISAO12_CQISAO_BitAddressOffset 0
#define CQISAO12_CQISAO_RegisterSize 32





#define CQCFG12 (DWC_ufshc_block_BaseAddress + 0x3334)
#define CQCFG12_RegisterSize 32
#define CQCFG12_RegisterResetValue 0x0
#define CQCFG12_RegisterResetMask 0xffffffff





#define CQCFG12_IAG_BitAddressOffset 0
#define CQCFG12_IAG_RegisterSize 5



#define CQCFG12_CQCFG12_RSVD_7_5_BitAddressOffset 5
#define CQCFG12_CQCFG12_RSVD_7_5_RegisterSize 3



#define CQCFG12_IAGVLD_BitAddressOffset 8
#define CQCFG12_IAGVLD_RegisterSize 1



#define CQCFG12_CQCFG12_RSVD_31_9_BitAddressOffset 9
#define CQCFG12_CQCFG12_RSVD_31_9_RegisterSize 23





#define SQATTR13 (DWC_ufshc_block_BaseAddress + 0x3340)
#define SQATTR13_RegisterSize 32
#define SQATTR13_RegisterResetValue 0xff
#define SQATTR13_RegisterResetMask 0xffffffff





#define SQATTR13_SIZE_BitAddressOffset 0
#define SQATTR13_SIZE_RegisterSize 16



#define SQATTR13_CQID_BitAddressOffset 16
#define SQATTR13_CQID_RegisterSize 4



#define SQATTR13_SQATTR13_RSVD_27_24_BitAddressOffset 20
#define SQATTR13_SQATTR13_RSVD_27_24_RegisterSize 8



#define SQATTR13_SQPL_BitAddressOffset 28
#define SQATTR13_SQPL_RegisterSize 3



#define SQATTR13_SQEN_BitAddressOffset 31
#define SQATTR13_SQEN_RegisterSize 1





#define SQLBA13 (DWC_ufshc_block_BaseAddress + 0x3344)
#define SQLBA13_RegisterSize 32
#define SQLBA13_RegisterResetValue 0x0
#define SQLBA13_RegisterResetMask 0xffffffff





#define SQLBA13_SQLBA13_RSVD_9_0_BitAddressOffset 0
#define SQLBA13_SQLBA13_RSVD_9_0_RegisterSize 10



#define SQLBA13_SQLBA_BitAddressOffset 10
#define SQLBA13_SQLBA_RegisterSize 22





#define SQUBA13 (DWC_ufshc_block_BaseAddress + 0x3348)
#define SQUBA13_RegisterSize 32
#define SQUBA13_RegisterResetValue 0x0
#define SQUBA13_RegisterResetMask 0xffffffff





#define SQUBA13_SQUBA_BitAddressOffset 0
#define SQUBA13_SQUBA_RegisterSize 32





#define SQDAO13 (DWC_ufshc_block_BaseAddress + 0x334c)
#define SQDAO13_RegisterSize 32
#define SQDAO13_RegisterResetValue 0x0
#define SQDAO13_RegisterResetMask 0xffffffff





#define SQDAO13_SQDAO_BitAddressOffset 0
#define SQDAO13_SQDAO_RegisterSize 32





#define SQISAO13 (DWC_ufshc_block_BaseAddress + 0x3350)
#define SQISAO13_RegisterSize 32
#define SQISAO13_RegisterResetValue 0x0
#define SQISAO13_RegisterResetMask 0xffffffff





#define SQISAO13_SQISAO_BitAddressOffset 0
#define SQISAO13_SQISAO_RegisterSize 32





#define SQCFG13 (DWC_ufshc_block_BaseAddress + 0x3354)
#define SQCFG13_RegisterSize 32
#define SQCFG13_RegisterResetValue 0x0
#define SQCFG13_RegisterResetMask 0xffffffff





#define SQCFG13_IAG_BitAddressOffset 0
#define SQCFG13_IAG_RegisterSize 5



#define SQCFG13_SQCFG13_RSVD_7_5_BitAddressOffset 5
#define SQCFG13_SQCFG13_RSVD_7_5_RegisterSize 3



#define SQCFG13_IAGVLD_BitAddressOffset 8
#define SQCFG13_IAGVLD_RegisterSize 1



#define SQCFG13_SQCFG13_RSVD_31_9_BitAddressOffset 9
#define SQCFG13_SQCFG13_RSVD_31_9_RegisterSize 23





#define CQATTR13 (DWC_ufshc_block_BaseAddress + 0x3360)
#define CQATTR13_RegisterSize 32
#define CQATTR13_RegisterResetValue 0xff
#define CQATTR13_RegisterResetMask 0xffffffff





#define CQATTR13_SIZE_BitAddressOffset 0
#define CQATTR13_SIZE_RegisterSize 16



#define CQATTR13_CQATTR13_RSVD_30_16_BitAddressOffset 16
#define CQATTR13_CQATTR13_RSVD_30_16_RegisterSize 15



#define CQATTR13_CQEN_BitAddressOffset 31
#define CQATTR13_CQEN_RegisterSize 1





#define CQLBA13 (DWC_ufshc_block_BaseAddress + 0x3364)
#define CQLBA13_RegisterSize 32
#define CQLBA13_RegisterResetValue 0x0
#define CQLBA13_RegisterResetMask 0xffffffff





#define CQLBA13_CQLBA13_RSVD_9_0_BitAddressOffset 0
#define CQLBA13_CQLBA13_RSVD_9_0_RegisterSize 10



#define CQLBA13_CQLBA_BitAddressOffset 10
#define CQLBA13_CQLBA_RegisterSize 22





#define CQUBA13 (DWC_ufshc_block_BaseAddress + 0x3368)
#define CQUBA13_RegisterSize 32
#define CQUBA13_RegisterResetValue 0x0
#define CQUBA13_RegisterResetMask 0xffffffff





#define CQUBA13_CQUBA_BitAddressOffset 0
#define CQUBA13_CQUBA_RegisterSize 32





#define CQDAO13 (DWC_ufshc_block_BaseAddress + 0x336c)
#define CQDAO13_RegisterSize 32
#define CQDAO13_RegisterResetValue 0x0
#define CQDAO13_RegisterResetMask 0xffffffff





#define CQDAO13_CQDAO_BitAddressOffset 0
#define CQDAO13_CQDAO_RegisterSize 32





#define CQISAO13 (DWC_ufshc_block_BaseAddress + 0x3370)
#define CQISAO13_RegisterSize 32
#define CQISAO13_RegisterResetValue 0x0
#define CQISAO13_RegisterResetMask 0xffffffff





#define CQISAO13_CQISAO_BitAddressOffset 0
#define CQISAO13_CQISAO_RegisterSize 32





#define CQCFG13 (DWC_ufshc_block_BaseAddress + 0x3374)
#define CQCFG13_RegisterSize 32
#define CQCFG13_RegisterResetValue 0x0
#define CQCFG13_RegisterResetMask 0xffffffff





#define CQCFG13_IAG_BitAddressOffset 0
#define CQCFG13_IAG_RegisterSize 5



#define CQCFG13_CQCFG13_RSVD_7_5_BitAddressOffset 5
#define CQCFG13_CQCFG13_RSVD_7_5_RegisterSize 3



#define CQCFG13_IAGVLD_BitAddressOffset 8
#define CQCFG13_IAGVLD_RegisterSize 1



#define CQCFG13_CQCFG13_RSVD_31_9_BitAddressOffset 9
#define CQCFG13_CQCFG13_RSVD_31_9_RegisterSize 23





#define SQATTR14 (DWC_ufshc_block_BaseAddress + 0x3380)
#define SQATTR14_RegisterSize 32
#define SQATTR14_RegisterResetValue 0xff
#define SQATTR14_RegisterResetMask 0xffffffff





#define SQATTR14_SIZE_BitAddressOffset 0
#define SQATTR14_SIZE_RegisterSize 16



#define SQATTR14_CQID_BitAddressOffset 16
#define SQATTR14_CQID_RegisterSize 4



#define SQATTR14_SQATTR14_RSVD_27_24_BitAddressOffset 20
#define SQATTR14_SQATTR14_RSVD_27_24_RegisterSize 8



#define SQATTR14_SQPL_BitAddressOffset 28
#define SQATTR14_SQPL_RegisterSize 3



#define SQATTR14_SQEN_BitAddressOffset 31
#define SQATTR14_SQEN_RegisterSize 1





#define SQLBA14 (DWC_ufshc_block_BaseAddress + 0x3384)
#define SQLBA14_RegisterSize 32
#define SQLBA14_RegisterResetValue 0x0
#define SQLBA14_RegisterResetMask 0xffffffff





#define SQLBA14_SQLBA14_RSVD_9_0_BitAddressOffset 0
#define SQLBA14_SQLBA14_RSVD_9_0_RegisterSize 10



#define SQLBA14_SQLBA_BitAddressOffset 10
#define SQLBA14_SQLBA_RegisterSize 22





#define SQUBA14 (DWC_ufshc_block_BaseAddress + 0x3388)
#define SQUBA14_RegisterSize 32
#define SQUBA14_RegisterResetValue 0x0
#define SQUBA14_RegisterResetMask 0xffffffff





#define SQUBA14_SQUBA_BitAddressOffset 0
#define SQUBA14_SQUBA_RegisterSize 32





#define SQDAO14 (DWC_ufshc_block_BaseAddress + 0x338c)
#define SQDAO14_RegisterSize 32
#define SQDAO14_RegisterResetValue 0x0
#define SQDAO14_RegisterResetMask 0xffffffff





#define SQDAO14_SQDAO_BitAddressOffset 0
#define SQDAO14_SQDAO_RegisterSize 32





#define SQISAO14 (DWC_ufshc_block_BaseAddress + 0x3390)
#define SQISAO14_RegisterSize 32
#define SQISAO14_RegisterResetValue 0x0
#define SQISAO14_RegisterResetMask 0xffffffff





#define SQISAO14_SQISAO_BitAddressOffset 0
#define SQISAO14_SQISAO_RegisterSize 32





#define SQCFG14 (DWC_ufshc_block_BaseAddress + 0x3394)
#define SQCFG14_RegisterSize 32
#define SQCFG14_RegisterResetValue 0x0
#define SQCFG14_RegisterResetMask 0xffffffff





#define SQCFG14_IAG_BitAddressOffset 0
#define SQCFG14_IAG_RegisterSize 5



#define SQCFG14_SQCFG14_RSVD_7_5_BitAddressOffset 5
#define SQCFG14_SQCFG14_RSVD_7_5_RegisterSize 3



#define SQCFG14_IAGVLD_BitAddressOffset 8
#define SQCFG14_IAGVLD_RegisterSize 1



#define SQCFG14_SQCFG14_RSVD_31_9_BitAddressOffset 9
#define SQCFG14_SQCFG14_RSVD_31_9_RegisterSize 23





#define CQATTR14 (DWC_ufshc_block_BaseAddress + 0x33a0)
#define CQATTR14_RegisterSize 32
#define CQATTR14_RegisterResetValue 0xff
#define CQATTR14_RegisterResetMask 0xffffffff





#define CQATTR14_SIZE_BitAddressOffset 0
#define CQATTR14_SIZE_RegisterSize 16



#define CQATTR14_CQATTR14_RSVD_30_16_BitAddressOffset 16
#define CQATTR14_CQATTR14_RSVD_30_16_RegisterSize 15



#define CQATTR14_CQEN_BitAddressOffset 31
#define CQATTR14_CQEN_RegisterSize 1





#define CQLBA14 (DWC_ufshc_block_BaseAddress + 0x33a4)
#define CQLBA14_RegisterSize 32
#define CQLBA14_RegisterResetValue 0x0
#define CQLBA14_RegisterResetMask 0xffffffff





#define CQLBA14_CQLBA14_RSVD_9_0_BitAddressOffset 0
#define CQLBA14_CQLBA14_RSVD_9_0_RegisterSize 10



#define CQLBA14_CQLBA_BitAddressOffset 10
#define CQLBA14_CQLBA_RegisterSize 22





#define CQUBA14 (DWC_ufshc_block_BaseAddress + 0x33a8)
#define CQUBA14_RegisterSize 32
#define CQUBA14_RegisterResetValue 0x0
#define CQUBA14_RegisterResetMask 0xffffffff





#define CQUBA14_CQUBA_BitAddressOffset 0
#define CQUBA14_CQUBA_RegisterSize 32





#define CQDAO14 (DWC_ufshc_block_BaseAddress + 0x33ac)
#define CQDAO14_RegisterSize 32
#define CQDAO14_RegisterResetValue 0x0
#define CQDAO14_RegisterResetMask 0xffffffff





#define CQDAO14_CQDAO_BitAddressOffset 0
#define CQDAO14_CQDAO_RegisterSize 32





#define CQISAO14 (DWC_ufshc_block_BaseAddress + 0x33b0)
#define CQISAO14_RegisterSize 32
#define CQISAO14_RegisterResetValue 0x0
#define CQISAO14_RegisterResetMask 0xffffffff





#define CQISAO14_CQISAO_BitAddressOffset 0
#define CQISAO14_CQISAO_RegisterSize 32





#define CQCFG14 (DWC_ufshc_block_BaseAddress + 0x33b4)
#define CQCFG14_RegisterSize 32
#define CQCFG14_RegisterResetValue 0x0
#define CQCFG14_RegisterResetMask 0xffffffff





#define CQCFG14_IAG_BitAddressOffset 0
#define CQCFG14_IAG_RegisterSize 5



#define CQCFG14_CQCFG14_RSVD_7_5_BitAddressOffset 5
#define CQCFG14_CQCFG14_RSVD_7_5_RegisterSize 3



#define CQCFG14_IAGVLD_BitAddressOffset 8
#define CQCFG14_IAGVLD_RegisterSize 1



#define CQCFG14_CQCFG14_RSVD_31_9_BitAddressOffset 9
#define CQCFG14_CQCFG14_RSVD_31_9_RegisterSize 23





#define SQATTR15 (DWC_ufshc_block_BaseAddress + 0x33c0)
#define SQATTR15_RegisterSize 32
#define SQATTR15_RegisterResetValue 0xff
#define SQATTR15_RegisterResetMask 0xffffffff





#define SQATTR15_SIZE_BitAddressOffset 0
#define SQATTR15_SIZE_RegisterSize 16



#define SQATTR15_CQID_BitAddressOffset 16
#define SQATTR15_CQID_RegisterSize 4



#define SQATTR15_SQATTR15_RSVD_27_24_BitAddressOffset 20
#define SQATTR15_SQATTR15_RSVD_27_24_RegisterSize 8



#define SQATTR15_SQPL_BitAddressOffset 28
#define SQATTR15_SQPL_RegisterSize 3



#define SQATTR15_SQEN_BitAddressOffset 31
#define SQATTR15_SQEN_RegisterSize 1





#define SQLBA15 (DWC_ufshc_block_BaseAddress + 0x33c4)
#define SQLBA15_RegisterSize 32
#define SQLBA15_RegisterResetValue 0x0
#define SQLBA15_RegisterResetMask 0xffffffff





#define SQLBA15_SQLBA15_RSVD_9_0_BitAddressOffset 0
#define SQLBA15_SQLBA15_RSVD_9_0_RegisterSize 10



#define SQLBA15_SQLBA_BitAddressOffset 10
#define SQLBA15_SQLBA_RegisterSize 22





#define SQUBA15 (DWC_ufshc_block_BaseAddress + 0x33c8)
#define SQUBA15_RegisterSize 32
#define SQUBA15_RegisterResetValue 0x0
#define SQUBA15_RegisterResetMask 0xffffffff





#define SQUBA15_SQUBA_BitAddressOffset 0
#define SQUBA15_SQUBA_RegisterSize 32





#define SQDAO15 (DWC_ufshc_block_BaseAddress + 0x33cc)
#define SQDAO15_RegisterSize 32
#define SQDAO15_RegisterResetValue 0x0
#define SQDAO15_RegisterResetMask 0xffffffff





#define SQDAO15_SQDAO_BitAddressOffset 0
#define SQDAO15_SQDAO_RegisterSize 32





#define SQISAO15 (DWC_ufshc_block_BaseAddress + 0x33d0)
#define SQISAO15_RegisterSize 32
#define SQISAO15_RegisterResetValue 0x0
#define SQISAO15_RegisterResetMask 0xffffffff





#define SQISAO15_SQISAO_BitAddressOffset 0
#define SQISAO15_SQISAO_RegisterSize 32





#define SQCFG15 (DWC_ufshc_block_BaseAddress + 0x33d4)
#define SQCFG15_RegisterSize 32
#define SQCFG15_RegisterResetValue 0x0
#define SQCFG15_RegisterResetMask 0xffffffff





#define SQCFG15_IAG_BitAddressOffset 0
#define SQCFG15_IAG_RegisterSize 5



#define SQCFG15_SQCFG15_RSVD_7_5_BitAddressOffset 5
#define SQCFG15_SQCFG15_RSVD_7_5_RegisterSize 3



#define SQCFG15_IAGVLD_BitAddressOffset 8
#define SQCFG15_IAGVLD_RegisterSize 1



#define SQCFG15_SQCFG15_RSVD_31_9_BitAddressOffset 9
#define SQCFG15_SQCFG15_RSVD_31_9_RegisterSize 23





#define CQATTR15 (DWC_ufshc_block_BaseAddress + 0x33e0)
#define CQATTR15_RegisterSize 32
#define CQATTR15_RegisterResetValue 0xff
#define CQATTR15_RegisterResetMask 0xffffffff





#define CQATTR15_SIZE_BitAddressOffset 0
#define CQATTR15_SIZE_RegisterSize 16



#define CQATTR15_CQATTR15_RSVD_30_16_BitAddressOffset 16
#define CQATTR15_CQATTR15_RSVD_30_16_RegisterSize 15



#define CQATTR15_CQEN_BitAddressOffset 31
#define CQATTR15_CQEN_RegisterSize 1





#define CQLBA15 (DWC_ufshc_block_BaseAddress + 0x33e4)
#define CQLBA15_RegisterSize 32
#define CQLBA15_RegisterResetValue 0x0
#define CQLBA15_RegisterResetMask 0xffffffff





#define CQLBA15_CQLBA15_RSVD_9_0_BitAddressOffset 0
#define CQLBA15_CQLBA15_RSVD_9_0_RegisterSize 10



#define CQLBA15_CQLBA_BitAddressOffset 10
#define CQLBA15_CQLBA_RegisterSize 22





#define CQUBA15 (DWC_ufshc_block_BaseAddress + 0x33e8)
#define CQUBA15_RegisterSize 32
#define CQUBA15_RegisterResetValue 0x0
#define CQUBA15_RegisterResetMask 0xffffffff





#define CQUBA15_CQUBA_BitAddressOffset 0
#define CQUBA15_CQUBA_RegisterSize 32





#define CQDAO15 (DWC_ufshc_block_BaseAddress + 0x33ec)
#define CQDAO15_RegisterSize 32
#define CQDAO15_RegisterResetValue 0x0
#define CQDAO15_RegisterResetMask 0xffffffff





#define CQDAO15_CQDAO_BitAddressOffset 0
#define CQDAO15_CQDAO_RegisterSize 32





#define CQISAO15 (DWC_ufshc_block_BaseAddress + 0x33f0)
#define CQISAO15_RegisterSize 32
#define CQISAO15_RegisterResetValue 0x0
#define CQISAO15_RegisterResetMask 0xffffffff





#define CQISAO15_CQISAO_BitAddressOffset 0
#define CQISAO15_CQISAO_RegisterSize 32





#define CQCFG15 (DWC_ufshc_block_BaseAddress + 0x33f4)
#define CQCFG15_RegisterSize 32
#define CQCFG15_RegisterResetValue 0x0
#define CQCFG15_RegisterResetMask 0xffffffff





#define CQCFG15_IAG_BitAddressOffset 0
#define CQCFG15_IAG_RegisterSize 5



#define CQCFG15_CQCFG15_RSVD_7_5_BitAddressOffset 5
#define CQCFG15_CQCFG15_RSVD_7_5_RegisterSize 3



#define CQCFG15_IAGVLD_BitAddressOffset 8
#define CQCFG15_IAGVLD_RegisterSize 1



#define CQCFG15_CQCFG15_RSVD_31_9_BitAddressOffset 9
#define CQCFG15_CQCFG15_RSVD_31_9_RegisterSize 23
