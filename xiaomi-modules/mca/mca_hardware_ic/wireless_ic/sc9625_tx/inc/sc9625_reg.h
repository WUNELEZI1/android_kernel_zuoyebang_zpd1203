// SPDX-License-Identifier: GPL-2.0
/*
 * sc9625_reg.h
 *
 * wireless TRX ic driver
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

#ifndef __SC9625_REG__
#define __SC9625_REG__
#include <linux/alarmtimer.h>

/// -----------------------------------------------------------------
///                      AP Command
/// -----------------------------------------------------------------
typedef enum {
	// RX_CMD_CLEAR_INT      = BIT(0),
	RX_CHIP_RESET     = BIT(1),
	RX_SEND_PPP       = BIT(2),
	RX_LVP_CHANGE     = BIT(3),
	RX_OVP_CHANGE     = BIT(4),
	RX_OCP_CHANGE     = BIT(5),
	RX_ILIMIT_CHANGE  = BIT(6),
	RX_OTP_CHANGE     = BIT(7),
	RX_HVWDGT_DISABLE = BIT(10),
	RX_HVWDGT_FEED    = BIT(11),
	RX_SEND_EPT       = BIT(12),
	RX_SEND_RENEG     = BIT(13),
	RX_VOUT_CHANGE    = BIT(14),
	RX_VOUT_ENABLE    = BIT(15),
	RX_VOUT_DISABLE   = BIT(16),
	RX_RPP_8BIT       = BIT(17),
	RX_RPP_24BIT      = BIT(18),
	RX_ONE_MASK       = BIT(31)
} RX_CMD;

typedef enum{
	SPEC_RX_CMD_FAST_CHARGE    = BIT(19),
	// SPEC_RX_VOLT_LIMIT         = BIT(29),
	SPEC_RX_CMD_M2_TP_SEND     = BIT(28),
	SPEC_RX_CMD_M2_RENEG       = BIT(30),
	SPEC_RX_CMD_CMD_ONE_MASK   = BIT(31)
} SPEC_RX_CMD;
/// -----------------------------------------------------------------
///                      TX Command
/// -----------------------------------------------------------------
typedef enum {
	TX_CLEAR_INT       = BIT(0),
	TX_CHIP_RESET      = BIT(1),
	TX_SEND_PPP        = BIT(2),
	TX_LVP_CHANGE      = BIT(3),
	TX_OVP_CHANGE      = BIT(4),
	TX_OCP_CHANGE      = BIT(5),
	TX_ILIMIT_CHANGE   = BIT(6),
	TX_OTP_CHANGE      = BIT(7),
	TX_HVWDGT_DISABLE  = BIT(10),
	TX_HVWDGT_FEED     = BIT(11),
	TX_FRQ_SHIFT       = BIT(8),
	TX_DISABLE         = BIT(18),
	TX_ENABLE          = BIT(19),
	TX_FPWM_AUTO       = BIT(20),
	TX_FPWM_MANUAL     = BIT(21),
	TX_FPWM_UPDATE     = BIT(22),
	TX_FPWM_CONFIG     = BIT(23),
	TX_FOD_ENABLE      = BIT(24),
	TX_FOD_DISABLE     = BIT(25),
	TX_PING_OCP_CHANGE = BIT(26),
	TX_PING_OVP_CHANGE = BIT(27),
	TX_OPEN_LOOP       = BIT(28),
	TX_REFRESH         = BIT(31)
} TX_CMD;

typedef enum {
    STANDBY_MODE              = BIT(0),
    RX_MODE                   = BIT(1),
    TX_MODE                   = BIT(2),
    BPP_MODE                  = BIT(3),
    EPP_MODE                  = BIT(4),
    PRIVATE_MODE              = BIT(5),
    RPP24bit_MODE             = BIT(6),
    VSYS_MODE                 = BIT(7),
    AC_MODE                   = BIT(8),
    SLEEP_MODE                = BIT(9),
    HVWDGT_MODE               = BIT(10),
    FOD_MODE                  = BIT(11),
    BRG_HALF                  = BIT(12),
    PPP_BUSY                  = BIT(13),
    TX_PING_MODE              = BIT(14),
    TX_PT_MODE                = BIT(15),
    TX_POWER_VOUT             = BIT(16)
} WORK_MODE_E;

typedef enum _PROJECT_FLAG_E {
    PROJECT_FLAG_ID                     = BIT(0),
    PROJECT_FLAG_AUTH                   = BIT(1),
    PROJECT_FLAG_ADAPTER_TYPE           = BIT(2),
    PROJECT_FLAG_UUID                   = BIT(3),
    PROJECT_FLAG_FAST_CHARGE            = BIT(4),
    PROJECT_FLAG_RENEG                  = BIT(5),
    // PROJECT_FLAG_VOLT_LIMIT             = BIT(6),
    PROJECT_FLAG_TP_SEND                = BIT(8),
} _PROJECT_FLAG_E;
/// -----------------------------------------------------------------
///                      sys mode
/// -----------------------------------------------------------------
typedef union {
    uint32_t value;
    struct {
        uint32_t STANDBY        : 1;
        uint32_t RECEIVER       : 1;
        uint32_t TRANSMITTER    : 1;
        uint32_t BPP_MODE       : 1;
        uint32_t EPP_MODE       : 1;
        uint32_t PREIVATE_MODE  : 1;
        uint32_t RPP_24BIT      : 1;
        uint32_t VDD_STATUS     : 1;
        uint32_t AC_MODE        : 1;
        uint32_t SLEEP_MODE     : 1;
        uint32_t HVWDGT_STATUS  : 1;
        uint32_t FOD_STATUS     : 1;
        uint32_t BRG_HALF       : 1;
        uint32_t PPP_BUSY       : 1;
        uint32_t TX_PING_MODE   : 1;
        uint32_t TX_PT_MODE     : 1;
        uint32_t TX_POWER_VOUT  : 1;
    };
} SYSMODE;
/// -----------------------------------------------------------------
///                      EPT RESON
/// -----------------------------------------------------------------
typedef enum {
    UNKOWN              = 0X00,
    CHARGE_COMPLETE     = 0X01,
    INTERNAL_FAULT      = 0X02,
    OVER_TEMPERATURE    = 0X03,
    OVER_VOLTAGE        = 0X04,
    OVER_CURRENT        = 0X05,
    BATTERY_FAILURE     = 0X06,
    RESERVED1           = 0X07,
    NO_RESPONSE         = 0X08,
    RESERVED2           = 0X09,
    NEGO_FAILURE        = 0X0A,
    RESTART_PT          = 0X0B,
} EPT_RESON;
/// -----------------------------------------------------------------
///                      EPT TYPE
/// -----------------------------------------------------------------
enum{
    EPT_CMD             = BIT(0),
    EPT_SEL             = BIT(1),
    EPT_SS              = BIT(2),
    EPT_ID              = BIT(3),
    EPT_XID             = BIT(4),
    EPT_CFG             = BIT(5),
    EPT_CFG_COUNT_ERR   = BIT(6),
    EPT_PCH             = BIT(7),
    EPT_NEG             = BIT(8),
    EPT_CALI            = BIT(9),
    EPT_FIRSTCEP        = BIT(10),
    EPT_TIMEOUT         = BIT(11),
    EPT_CEP_TIMEOUT     = BIT(12),
    EPT_RPP_TIMEOUT     = BIT(13),
    EPT_LCP             = BIT(14),
    EPT_OCP             = BIT(15),
    EPT_OVP             = BIT(16),
    EPT_LVP             = BIT(17),
    EPT_FOD             = BIT(18),
    EPT_OTP             = BIT(19),
    EPT_PING_OCP        = BIT(20),
    EPT_PING_OVP        = BIT(21),
    EPT_CLAMP_OVP       = BIT(22),
    EPT_NGATE_OVP       = BIT(23),
    EPT_AC_DET          = BIT(24),
    EPT_PKTERR          = BIT(25),
};

/// -----------------------------------------------------------------
///                      INT FLAG
/// -----------------------------------------------------------------
// public
#define   INT_NONE                      0
#define   INT_OCP                       BIT(0)
#define   INT_OVP                       BIT(1)
#define   INT_CLAMP_OVP                 BIT(2)
#define   INT_NGATE_OVP                 BIT(3)
#define   INT_LVP                       BIT(4)
#define   INT_OTP                       BIT(5)
#define   INT_OTP_160                   BIT(6)
#define   INT_SLEEP                     BIT(7)
#define   INT_MODE_CHANGE               BIT(8)
#define   INT_PKT_RECV                  BIT(9)
#define   INT_PPP_TIMEOUT               BIT(10)
#define   INT_PPP_SUCCESS               BIT(11)
#define   INT_AFC                       BIT(12)
#define   INT_EPP_BPP                   BIT(13)
// RX
#define   INT_POWER_ON                  BIT(14)
#define   INT_SS_PKT                    BIT(15)
#define   INT_ID_PKT                    BIT(16)
#define   INT_CONFIG_PKT                BIT(17)
#define   INT_RX_READY                  BIT(18)
#define   INT_LDO_ON                    BIT(19)
#define   INT_LDO_OFF                   BIT(20)
#define   INT_PLDO                      BIT(21)
#define   INT_SCP                       BIT(22)
#define   IRQ_RENEG_SUCCESS             BIT(23)
#define   IRQ_RENEG_TIMEOUT             BIT(24)
#define   IRQ_RENEG_FAIL                BIT(25)
#define   IRQ_AUTH                      BIT(27)
#define   IRQ_FAST_CHARGE_SUCCESS       BIT(28)
#define   IRQ_FAST_CHARGE_TIMEOUT       BIT(29)
#define   IRQ_FAST_CHARGE_FAIL          BIT(30)
#define   IRQ_ONE_MASK                  BIT(31)
// TX
#define   INT_DET_RX                    BIT(14)
#define   INT_REMOVE_POEWR              BIT(15)
#define   INT_POWER_TRANS               BIT(16)
#define   INT_FOD                       BIT(17)
#define   INT_DET_AC                    BIT(18)
#define   INT_CEP_TIMEOUT               BIT(19)
#define   INT_RPP_TIMEOUT               BIT(20)
#define   INT_PING                      BIT(21)
#define   INT_TX_ID_PKT                 BIT(23)
#define   INT_TX_CONFIG_PKT             BIT(24)
#define   INT_TX_BLE_CONNECT            BIT(25)
#define   INT_TX_POWER_ON               BIT(26)
#define   INT_TX_CHS_UPDATE             BIT(27)

/// -----------------------------------------------------------------
///                              WPC TYPE
///
/// -----------------------------------------------------------------
#define MAX_ASK_SIZE            22
#define MAX_FSK_SIZE            10
typedef struct {
    uint8_t header;
    uint8_t max_power             : 6;
    uint8_t power_class           : 2;
    uint8_t reserved0;
    uint8_t count                 : 3;
    uint8_t ZERO                  : 1;
    uint8_t reserved1             : 3;
    uint8_t prop                  : 1;
    uint8_t window_offset         : 3;
    uint8_t window_size           : 5;
    uint8_t reserved2             : 4;
    uint8_t depth                 : 2;
    uint8_t polarity              : 1;
    uint8_t neg                   : 1;
} CfgPktType;
typedef struct {
    uint8_t  header;
    uint8_t  minor_version        : 4;
    uint8_t  major_version        : 4;
    uint16_t manufacturer_code;
    uint32_t basic_device_identifier3 : 7;
    uint32_t ext                      : 1;
    uint32_t basic_device_identifier2 : 8;
    uint32_t basic_device_identifier1 : 8;
    uint32_t basic_device_identifier0 : 8;
} IdPktType;
typedef struct {
    uint8_t SS;
    uint8_t PCH;
    uint16_t RSV;
} SSPktType;
typedef struct {
    uint8_t cep;
    uint8_t rsv;
    uint8_t rpp;
} PTpktType;
typedef struct {
    uint16_t mcode;
    uint8_t  minor                : 4;
    uint8_t  major                : 4;
    uint8_t  afc;
} TxInfoType;
typedef union {
    uint8_t value;
    struct {
        uint8_t depth             : 2;
        uint8_t polarity          : 1;
        uint8_t reserved          : 5;
    };
} FSKParametersType;
typedef union {              // power transfer contract
    uint32_t value;
    struct {
        uint8_t RPPTHeader;
        uint8_t guaranteed_power; // The value in this field is in units of 0.5 W.
        uint8_t max_power;
        FSKParametersType fsk;
    };
} ContractType;
typedef struct {
    uint8_t guaranteed_power      : 6;
    uint8_t power_class           : 2;
    uint8_t potential_power       : 6;
    uint8_t reserved0             : 2;
    uint8_t not_ressens           : 1;
    uint8_t WPID                  : 1;
    uint8_t reserved1             : 6;
} CapabilityType;
typedef struct {
    uint8_t ping                  : 1;
    uint8_t ptr                   : 1;
    uint8_t rsv0                  : 6;
    uint8_t rsv1;
} BRGManualType;
typedef struct {
    union {
        uint8_t buf[MAX_ASK_SIZE];
        struct {
            uint8_t header;
            uint8_t msg[MAX_ASK_SIZE-1];
        };
    };
} AskType;
typedef struct {
    union {
        uint8_t buf[MAX_FSK_SIZE];
        struct {
            uint8_t header;
            uint8_t msg[MAX_FSK_SIZE-1];
        };
    };
} FskType;
typedef struct {
    uint8_t G;
    int8_t Offs;
} FodType;
/// -----------------------------------------------------------------
///                      Customer Registers
///            SRAM address: 0X20000000 ~ 0X20000200
/// -----------------------------------------------------------------
#define CUSTOMER_REGISTERS_BASE_ADDR      0x20000000
typedef struct {                    // <offset>
	uint16_t         ChipID;          // 0X00
	uint16_t         CustID;          // 0X02
	uint32_t         FwVer;           // 0X04
	uint32_t         HwVer;           // 0X08
	uint32_t         GitVer;          // 0X0C
	uint16_t         mCode;           // 0X10
	uint16_t         ONE_MASK0012;    // 0X12
	uint32_t         ONE_MASK0014;    // 0X14
	uint32_t         ONE_MASK0018;    // 0X18
	uint32_t         firmware_check;  // 0X1C
	uint16_t         StartCurr;       // 0X20
	uint16_t         StartOVP;        // 0X22
	uint16_t         Ilimit;          // 0X24
	uint16_t         RxOCP;           // 0X26
	uint16_t         RxOVP;           // 0X28
	uint16_t         PLdo;            // 0X2A
	uint16_t         RxClampOvp;      // 0X2C
	uint16_t         RxnGateOvp;      // 0X2E
	uint16_t         Vtarget;         // 0X30
	uint16_t         MaxVrect;        // 0X32
	uint16_t         VoutSET;         // 0X34
	uint16_t         ONE_MASK0036;    // 0x36
	uint16_t         EPPVout[4];      // 0X38
	uint16_t         RECMode2F;       // 0X40
	uint16_t         RECMode2H;       // 0X42
	uint16_t         RecFull;         // 0X44
	uint16_t         RecHalf;         // 0X46
	uint16_t         RecHys;          // 0X48
	uint16_t         ONE_MASK004A;    // 0X4A
	uint16_t         VrectX1;         // 0X4C
	uint16_t         VrectX2;         // 0X4E
	uint16_t         VrectY1;         // 0X50
	uint16_t         VrectY2;         // 0X52
	uint16_t         VrectAdj;        // 0X54
	uint16_t         ONE_MASK0056;    // 0x56
	uint16_t         CEPInterval;     // 0X58
	uint16_t         RPPInterval;     // 0X5A
	uint32_t         ONE_MASK005C;    // 0X5C
	FodType        fod[8];          // 0X60
	FodType        eppfod[8];       // 0X70
	uint8_t          fskDepth;        // 0X80
	uint8_t          fskPolarity;     // 0X81
	uint8_t          fskThreshold;    // 0X82
	uint8_t          ONE_MASK0083;    // 0X83
	uint8_t          RxOTP;           // 0X84
	uint8_t          ONE_MASK0085[3]; // 0X85
	uint16_t         WakeTime;        // 0X88
	uint16_t         ONE_MASK008A;    // 0X8A
	uint8_t          ASKcfg;          // 0X8C
	uint8_t          ONE_MASK008D[3]; // 0X8D
	uint8_t          VDD;             // 0X90
	uint8_t          ONE_MASK0091[15];// 0X91
	uint8_t          TxSetting[0x60]; // 0XA0
	uint32_t         Cmd;             // 0X0100
	uint32_t         IntEn;           // 0X0104
	uint32_t         IntFlag;         // 0X0108
	uint32_t         IntClr;          // 0X010C
	uint32_t         CEPCnt;          // 0X0110
	uint32_t         Mode;            // 0X0114
	uint32_t         ONE_MASK0118;    // 0X0118
	uint32_t         HvWdgtCnt;       // 0X011C
	uint32_t         ONE_MASK0120;    // 0X0120
	uint32_t         Random[2];       // 0X0124
	uint32_t         ONE_MASK012C;    // 0X012C
	AskType        ask;             // 0X0130
	uint8_t          ONE_MASK0146[10];// 0X0146
	FskType        fsk;             // 0X0150
	uint16_t         ONE_MASK015A[3]; // 0X015A
	uint16_t         Voltage;         // 0X0160
	uint16_t         ONE_MASK0162;    // 0X0162
	uint16_t         Current;         // 0X0164
	uint16_t         ONE_MASK0166;    // 0X0166
	uint16_t         Vrect;           // 0X0168
	uint16_t         TargetVrect;     // 0X016A
	uint16_t         Vping;           // 0X016C
	uint16_t         ONE_MASK016E;    // 0X016E
	uint16_t         PRx;             // 0X0170
	uint16_t         PTx;             // 0X0172
	uint16_t         frequency;       // 0X0174
	uint16_t         mperiod;         // 0X0176
	int16_t          Tdie;            // 0X0178
	uint16_t         ONE_MASK017A;    // 0X017A
	uint8_t          Duty;            // 0X017C
	uint8_t          ONE_MASK017D[3]; // 0X017D
	uint32_t         ONE_MASK0180[3]; // 0X0180
	uint8_t          Syncstate;       // 0X018C
	uint8_t          Rectstate;       // 0X018D
	uint16_t         ONE_MASK018E;    // 0X018E
	uint8_t          EPTReson;        // 0X0190
	uint8_t          ONE_MASK0191[3]; // 0X0191
	uint32_t         EPTType;         // 0X0194
	IdPktType      IdPkt;           // 0X0198
	uint8_t          ssVal;           // 0X01A0
	uint8_t          PCH;             // 0X01A1
	uint16_t         ONE_MASK01A2;    // 0X01A2
	CfgPktType     CfgPkt;          // 0X01A4
	uint16_t         ONE_MASK01AA;    // 0X01AA
	ContractType   ReqContract;     // 0X01AC
	ContractType   CurContract;     // 0X01B0
	int8_t           cepVal;          // 0X01B4
	uint8_t          CHS;             // 0X01B5
	uint16_t         RPP;             // 0X01B6
	uint32_t         ONE_MASK01B8[2]; // 0X01B8
	uint8_t          MaxPower;        // 0X01C0
	uint8_t          ONE_MASK[3];     // 0X01C1
	uint16_t         TxManufCode;     // 0X01C4
	uint8_t          TxRev;           // 0X01C6
	uint8_t          afcVer;          // 0X01C7
	CapabilityType capability;      // 0X01C8
	uint8_t          ONE_MASK01CB;    // 0X01CB
	uint8_t          ONE_MASK01CC[3]; // 0X01CC
    uint8_t          iic_check;       // 0X01CF
    struct{
        uint16_t         Tx_UUID_L;       // 0X01D0
        uint16_t         ONE_MASK01D2;    // 0X01D2
        uint16_t         Tx_UUID_H;       // 0X01D4
        uint16_t         ONE_MASK01D6;    // 0X01D6
        uint16_t         fun_flag;        // 0X01D8
        uint8_t          adapter_type;    // 0X01DA
        uint8_t          ONE_MASK01DB;    // 0X01DB
        uint16_t         fc_volt;         // 0X01DC
        uint8_t          reneg_param;     // 0X01DE
        uint8_t          ONE_MASK0X01DF;  // 0X01DF
        uint32_t         cmd;             // 0X01E0
        uint16_t         volt_limit;      // 0X01E4
        uint16_t         ONE_MASK01E6;    // 0X01E6
        uint8_t          ONE_MASK01E0[24];// 0X01E8
    }mim2;
} RXCustType;
typedef struct {                    // <offset>
	uint16_t         ChipID;          // 0X00
	uint16_t         CustID;          // 0X02
	uint32_t         FwVer;           // 0X04
	uint32_t         HwVer;           // 0X08
	uint32_t         GitVer;          // 0X0C
	uint16_t         mCode;           // 0X10
	uint16_t         ONE_MASK0012;    // 0X12
	uint32_t         ONE_MASK0014;    // 0X14
	uint32_t         ONE_MASK0018;    // 0X18
	uint32_t         firmware_check;  // 0X1C
	uint8_t          RXSetting[0x80]; // 0X0020
	uint16_t         MinFreq;         // 0XA0
	uint16_t         MaxFreq;         // 0XA2
	uint16_t         TxLVP;           // 0XA4
	uint16_t         ONE_MASK00A6;    // 0XA6
	uint16_t         PingOCP;         // 0XA8
	uint16_t         TxOCP;           // 0XAA
	uint16_t         PingOVP;         // 0XAC
	uint16_t         TxOVP;           // 0XAE
	uint16_t         TxClampOvp;      // 0XB0
	uint16_t         TxnGateOvp;      // 0XB2
	uint8_t          TxOTP;           // 0XB4
	uint8_t          ONE_MASK00B5[3]; // 0XB5
	uint32_t         ONE_MASK00B8;    // 0XB8
	uint16_t         PingInterval;    // 0XBC
	uint16_t         PingTimeout;     // 0XBE
	uint16_t         PingFreq;        // 0XC0
	uint8_t          PingDuty;        // 0XC2
	uint8_t          ONE_MASK00C3;    // 0XC3
	uint32_t         ONE_MASK00C4[3]; // 0XC4
	uint8_t          DeadTime;        // 0XD0
	uint8_t          MinDuty;         // 0XD1
	uint16_t         ONE_MASK00D2;    // 0XD2
	uint8_t          BrgMode;         // 0XD4
	uint8_t          ONE_MASK00D5;    // 0XD5
	uint16_t         SwitchVoltage;   // 0XD6
	uint16_t         SwitchCurr;      // 0XD8
	uint16_t         SwitchHys;       // 0XDA
	uint16_t         SwitchFreq;      // 0XDC
	uint8_t          SwitchDuty;      // 0XDE
	uint8_t          ONE_MASK00DF;    // 0XDF
	uint8_t          FodIndex;        // 0XE0
	uint8_t          FodCnt;          // 0XE1
	uint16_t         ONE_MASK00E2;    // 0XE2
	uint16_t         FodPloss[6];     // 0XE4
	uint32_t         ONE_MASK00F0[4]; // 0XF0
	uint32_t         Cmd;             // 0X0100
	uint32_t         IntEn;           // 0X0104
	uint32_t         IntFlag;         // 0X0108
	uint32_t         IntClr;          // 0X010C
	uint32_t         CEPCnt;          // 0X0110
	uint32_t         Mode;            // 0X0114
	uint32_t         ONE_MASK0118;    // 0X0118
	uint32_t         HvWdgtCnt;       // 0X011C
	uint32_t         ONE_MASK0120;    // 0X0120
	uint32_t         Random[2];       // 0X0124
	uint32_t         ONE_MASK012C;    // 0X012C
	AskType        ask;             // 0X0130
	uint8_t          ONE_MASK0146[10];// 0X0146
	FskType        fsk;             // 0X0150
	uint16_t         ONE_MASK015A[3]; // 0X015A
	uint16_t         Voltage;         // 0X0160
	uint16_t         ONE_MASK0162;    // 0X0162
	uint16_t         Current;         // 0X0164
	uint16_t         ONE_MASK0166;    // 0X0166
	uint16_t         Vrect;           // 0X0168
	uint16_t         TargetVrect;     // 0X016A
	uint16_t         Vping;           // 0X016C
	uint16_t         ONE_MASK016E;    // 0X016E
	uint16_t         PRx;             // 0X0170
	uint16_t         PTx;             // 0X0172
	uint16_t         frequency;       // 0X0174
	uint16_t         mperiod;         // 0X0176
	int16_t          Tdie;            // 0X0178
	uint16_t         ONE_MASK017A;    // 0X017A
	uint8_t          Duty;            // 0X017C
	uint8_t          ONE_MASK017D[3]; // 0X017D
	uint32_t         ONE_MASK0180[3]; // 0X0180
	uint8_t          Syncstate;       // 0X018C
	uint8_t          Rectstate;       // 0X018D
	uint16_t         ONE_MASK018E;    // 0X018E
	uint8_t          EPTReson;        // 0X0190
	uint8_t          ONE_MASK0191[3]; // 0X0191
	uint32_t         EPTType;         // 0X0194
	IdPktType      IdPkt;           // 0X0198
	uint8_t          ssVal;           // 0X01A0
	uint8_t          PCH;             // 0X01A1
	uint16_t         ONE_MASK01A2;    // 0X01A2
	CfgPktType     CfgPkt;          // 0X01A4
	uint16_t         ONE_MASK01AA;    // 0X01AA
	ContractType   ReqContract;     // 0X01AC
	ContractType   CurContract;     // 0X01B0
	int8_t           cepVal;          // 0X01B4
	uint8_t          CHS;             // 0X01B5
	uint16_t         RPP;             // 0X01B6
	uint32_t         ONE_MASK01B8[2]; // 0X01B8
	uint8_t          MaxPower;        // 0X01C0
	uint8_t          ONE_MASK[3];     // 0X01C1
	uint16_t         TxManufCode;     // 0X01C4
	uint8_t          TxRev;           // 0X01C6
	uint8_t          afcVer;          // 0X01C7
	CapabilityType capability;      // 0X01C8
	uint8_t          ONE_MASK01CB;    // 0X01CB
	uint8_t          ONE_MASK01CC[3]; // 0X01CC
	uint8_t          iic_check;       // 0X01CF
	uint8_t          BATTERY_LEVEL;   // 0X01D0
	uint8_t          PEN_TYPE;        // 0X01D1
	uint8_t          RSC01D2[6];      // 0X01D2
	uint8_t          CHARGE_TYPE;     // 0X01D8
	uint8_t          RSV01D9[3];      // 0X01D9
	uint8_t          RSV01DC[2];      // 0X01DC
	uint8_t          BLE_MAC1[2];     // 0X01DE
	uint8_t          BLE_MAC2[4];     // 0X01E0
	uint8_t          RSV01E4[28];     // 0X01E4
} TXCustType;

#define sc_err(fmt, ...)    do { pr_err("[sc9625]:%s:" fmt, __func__, ##__VA_ARGS__); } while(0)
#define sc_info(fmt, ...)    do { pr_info("[sc9625]:%s:" fmt, __func__, ##__VA_ARGS__); } while(0)
#define sc_dbg(fmt, ...)    do { pr_dbg("[sc9625]:%s:" fmt, __func__, ##__VA_ARGS__); } while(0)

#define cust_rx     (*((RXCustType*)(0)))
#define cust_tx     (*((TXCustType*)(0)))

#define readCust2   readCust
#define readCust(sc, member, p)     \
	sc9625_read_block(sc, (uint64_t)(&member) + CUSTOMER_REGISTERS_BASE_ADDR, (uint8_t *)(p), (uint32_t)sizeof(member)) 

#define writeCust(sc, member, p_val) \
	sc9625_write_block(sc, (uint64_t)(&member) + CUSTOMER_REGISTERS_BASE_ADDR, (uint8_t *)(p_val), (uint32_t)sizeof(member))

/// -----------------------------------------------------------------
///                      customer command
/// -----------------------------------------------------------------
typedef enum {
    PRIVATE_CMD_NONE               = 0x00,
    PRIVATE_CMD_READ_HW_VER        = 0x01,
    PRIVATE_CMD_READ_TEMP          = 0x02,
    PRIVATE_CMD_SET_FREQ           = 0x03,
    PRIVATE_CMD_GET_FREQ           = 0x04,
    PRIVATE_CMD_READ_FW_VER        = 0x05,
    PRIVATE_CMD_READ_Iin           = 0x06,
    PRIVATE_CMD_READ_Vin           = 0x07,
    PRIVATE_CMD_DEV_AUTH_CONFIRM   = 0x08,
    PRIVATE_CMD_SET_Vin            = 0x0A,
    PRIVATE_CMD_ADAPTER_TYPE       = 0x0B,
    PRIVATE_CMD_AUTH_REQ           = 0x3B,
    PRIVATE_CMD_GET_BPP_TX_UUID    = 0x3F,
    PRIVATE_CMD_GET_EPP_TX_UUID    = 0x4C,
    PRIVATE_CMD_CTRL_PRI_PKT       = 0x63,
    PRIVATE_CMD_GET_TX_MAC_ADDR_L  = 0xB6,
    PRIVATE_CMD_GET_TX_MAC_ADDR_H  = 0xB7,
    PRIVATE_CMD_SET_MIN_FREQ       = 0xD3,
    PRIVATE_CMD_SET_AUTO_VLIMIT    = 0xD5,
    PRIVATE_CMD_SET_TX_VLIMIT      = 0xD6,
} PRIVATE_CMD_E;
typedef enum PRIVATE_PACKET_E{
    PRIVATE_PACKET_NONE=0,
    PRIVATE_PACKET_VOLTAGE_LIMIT,
    PRIVATE_PACKET_MIN_FREQ_LIMIT,
    PRIVATE_PACKET_CHARGE_STATUS,
    PRIVATE_PACKET_FAN_SPEED,
}PRIVATE_PACKET_E;

typedef enum tx_irq_flag_t {
    RTX_INT_UNKNOWN,
    RTX_INT_PING,
    RTX_INT_GET_RX,
    RTX_INT_CEP_TIMEOUT,
    RTX_INT_EPT,
    RTX_INT_PROTECTION,
    RTX_INT_GET_TX,
    RTX_INT_REVERSE_TEST_READY,
    RTX_INT_REVERSE_TEST_DONE,
    RTX_INT_FOD,
    RTX_INT_EPT_PKT,
    RTX_INT_ERR_CODE,
    RTX_INT_TX_DET_RX,
    RTX_INT_TX_CONFIG,
    RTX_INT_TX_CHS_UPDATE,
    RTX_INT_TX_BLE_CONNECT,
} tx_irq_flag_t;

#define DECL_INTERRUPT_MAP(regval, redir_irq) {\
    .irq_regval = regval,\
    .irq_flag = redir_irq, \
}

typedef struct int_map_t {
    uint32_t irq_regval;
    int irq_flag;
} int_map_t;

struct wls_fw_parameters {
	u8 fw_rx_id;
	u8 fw_tx_id;
	u8 fw_boot_id;
	u8 hw_id_h;
	u8 hw_id_l;
};

struct sc9625_chg_proc_data {
    int int_flag;
    int ss_voltage;
};

#define RX_CHECK_SUCCESS (1 << 0)
#define TX_CHECK_SUCCESS (1 << 1)
#define BOOT_CHECK_SUCCESS (1 << 2)

typedef enum {
    PPE_NONE,
    PPE_SS,
    PPE_HALL,
}PEN_PLACE_ERR;

struct sc9625 {
    struct device *dev;
    struct i2c_client *client;
    struct regmap *regmap;
    // irq and gpio
    int irq;
    int irq_gpio;
    int hall3_irq;
    int hall3_gpio;
    int hall4_irq;
    int hall4_gpio;
    int hall3_s_irq;
    int hall3_s_gpio;
    int hall4_s_irq;
    int hall4_s_gpio;
    int hall_ppe_n_irq;
    int hall_ppe_n_gpio;
    int hall_ppe_s_irq;
    int hall_ppe_s_gpio;
    int reverse_txon_gpio;
    int reverse_boost_gpio;
    // fw
    bool fw_program;
    int fw_data_size;
    unsigned char *fw_data_ptr;
    struct wls_fw_parameters *wls_fw_data;
    int fw_bin_length;
    unsigned char fw_bin[32768];
    // dts prop
    int project_vendor;
    int rx_role;
    int fw_version_index;
    // context
    struct sc9625_chg_proc_data proc_data;
    // lock
    struct mutex i2c_rw_lock;
    struct mutex wireless_chg_int_lock;
    // delay work
    struct delayed_work interrupt_work;
    struct delayed_work hall_irq_work;
    struct delayed_work ppe_hall_irq_work;
    // pen info
    int reverse_pen_soc;
    int reverse_pen_full;
    int reverse_vout;
    int reverse_iout;
    int reverse_ss;
    bool reverse_boost_en;
    u8 pen_mac_data[6];
    int hall3_online;
    int hall4_online;
    int hall3_s_val;
    int hall4_s_val;
    int hall_ppe_n_val;
    int hall_ppe_s_val;
    int pen_place_err;
    bool pen_soc_update;
};

#endif
