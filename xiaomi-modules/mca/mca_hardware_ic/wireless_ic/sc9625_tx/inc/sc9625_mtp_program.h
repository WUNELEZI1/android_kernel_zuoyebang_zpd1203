// SPDX-License-Identifier: GPL-2.0
/*
 * sc9625_mtp_program.h
 *
 * wireless ic driver
 *
 * Copyright (c) 2023-2023 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SC9625_MTP_PROGRAM_H__
#define __SC9625_MTP_PROGRAM_H__

/// -----------------------------------------------------------------
///                      Program Reg
/// -----------------------------------------------------------------
#define SC9625_PASSWD               0xFF00
#define SC9625_PID0                 0xFF70
#define SC9625_PID1                 0xFF71
#define SC9625_PID2                 0xFF72
#define SC9625_PID3                 0xFF73
#define SC9625_PID4                 0xFF74
#define SC9625_PID5                 0xFF75
#define SC9625_TM_ST                0xFF7F
#define SC9625_AD_MSB               0xFF80
#define SC9625_AD_LSB               0xFF81
#define SC9625_DIN0_B3              0xFF82
#define SC9625_DIN_CRC8             0xFF92
#define SC9625_DOUT0_B3             0xFF93
#define SC9625_DOUT_CRC_B3          0xFFA3
#define SC9625_OP_NUM_MSB           0xFFA7
#define SC9625_OP_NUM_LSB           0xFFA8
#define SC9625_CTRL                 0xFFA9
#define SC9625_MTP_OP_SEL           0xFFAA
#define SC9625_REG_RSV              0xFFAB
#define SC9625_CP                   0xFFAC
#define SC9625_MTP_ST               0xFFAD
#define SC9625_TPGHF                0xFFAE
#define SC9625_ATE_EN               0xFFB0
#define SC9625_SRAM_BIST_CTRL0      0xFFB1
#define SC9625_SRAM_BIST_CTRL1      0xFFC0
#define SC9625_SRAM_BIST_ST0        0xFFC1
#define SC9625_SRAM_BIST_ST1        0xFFC2
#define SC9625_HADDR_MSB            0xFFE0
#define SC9625_HADDR_LSB            0xFFE1
#define SC9625_MONITOR_PC_B0        0xFFF0
#define SC9625_TM_IO                0xFFFC
#define SC9625_MAIN_ST              0xFFFD
#define SC9625_ATE_ST               0xFFFE
#define SC9625_ST                   0xFFFF

#define SC9625_PASSWD0              0x00
#define SC9625_PASSWD1              0x53
#define SC9625_PASSWD2              0x45
#define SC9625_PASSWD3              0x41
#define SC9625_PASSWD4              0x4E

#define ERASE_OP_TIME               20  //erase time = (20+0xAE)ms , MTP spec:20-50ms
#define WRITE_OP_TIME               14
#define MTP_START_ADDR              0x0000
#define MAIN_BKS                    0
#define INFO_BKS                    1

#define TRIM_WORD_LEN               8
#define INFO_WORD_LEN               16
#define TRIM0_WORD_ADDR             0x00
#define INFO0_WORD_ADDR             0x10
#define TRIM1_WORD_ADDR             0x40
#define INFO1_WORD_ADDR             0x48

#define INFOR_COMPARE_WORD_SIZE     (TRIM_WORD_LEN + INFO_WORD_LEN)
#define INFOR_READ_LEN              ((INFOR_COMPARE_WORD_SIZE + 3) & ~3UL)

#define SWAP_UINT32(x)              ((((x) >> 24) & 0xff) |     \
                                     (((x) >> 8) & 0xff00) |    \
                                     (((x) << 8) & 0xff0000) |  \
                                     ((x) << 24))

typedef struct {
    uint32_t  word_addr;
    uint32_t  word_len;
} trim_info_space_t;

enum {
    CP_VOL_97       = 0,
    CP_VOL_100,
    CP_VOL_103,
    CP_VOL_106,
};

enum {
    MTP_CLR_MODE    = 0x00,
    MTP_WR_MODE     = 0x5A,
    MTP_WV_MODE     = 0xA5,
    MTP_RD_MODE     = 0xC3,
    MTP_RV_MODE     = 0x3C,
    MTP_ERS_MODE    = 0x69,
    MTP_CERS_MODE   = 0x96,
    MTP_CRC_MODE    = 0x87,
};

enum {
    MARGIN0         = 0,
    MARGIN1,
    MARGIN2,
    MARGIN3,
};

typedef union {
    uint8_t value;
    struct {
        uint8_t dig_tm      : 1;
        uint8_t ana_tm      : 1;
        uint8_t test_mode   : 1;
        uint8_t rsv         : 5;
    };
}SC9625_tm_st_e;
 
typedef union {
    uint8_t value;
    struct {
        uint8_t iic_cfg     : 1;
        uint8_t iic_cfg1    : 1;
        uint8_t cp_vol      : 2;
        uint8_t pdn         : 1;
        uint8_t trf         : 1;
        uint8_t lckn        : 1;
        uint8_t mtp_en      : 1;
    };
}SC9625_ctrl_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t iic_mtp_start : 1;
        uint8_t rsv         : 1;
    };
}SC9625_reg_rsv_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t marrd       : 2;
        uint8_t cp_en       : 1;
        uint8_t rsv         : 5;
    };
}SC9625_cp_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t wr_avb      : 1;
        uint8_t rd_avb      : 1;
        uint8_t mtp_busy    : 1;
        uint8_t mtp_fail    : 1;
        uint8_t rsv         : 1;
        uint8_t din_crc_fail: 1;
        uint8_t rsv1        : 2;
    };
}SC9625_mtp_st_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t val         : 4;
        uint8_t rsv         : 4;
    };
}SC9625_tpghf_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t en_ate      : 1;
        uint8_t iic_mtp_en  : 1;
        uint8_t rsv         : 6;
    };
}SC9625_ate_en_e;

typedef union {
    uint8_t value;
    struct {
        uint8_t mcu_en      : 1;
        uint8_t ate_hirc_en : 1;
        uint8_t uirc_en     : 1;
        uint8_t sleep_mode  : 1;
        uint8_t hirc_ok     : 1;
        uint8_t uirc_ok     : 1;
        uint8_t warmup_done : 1;
        uint8_t rsv         : 1;
    };
}SC9625_st_e;

bool mtp_chip_crc_check(struct sc9625 *sc, uint32_t mtp_len);
int sc9625_mtp_program(struct sc9625 *sc, uint8_t *image, uint32_t len);
//
#endif