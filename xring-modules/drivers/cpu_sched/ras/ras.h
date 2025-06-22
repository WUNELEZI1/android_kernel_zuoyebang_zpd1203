/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: ras driver
 */

#ifndef __PLAT_RAS_H__
#define __PLAT_RAS_H__

enum {
	ERRSELR_SEL_DSU = 0,
	ERRSELR_SEL_CORE,
	ERRSELR_SEL_COMPLEX,
	ERRSELR_SEL_MAX
};

enum {
	ERRTYPE_CORRECTED = 0,
	ERRTYPE_DEFERRED,
	ERRTYPE_CRITICAL,
	ERRTYPE_UEO,
	ERRTYPE_UER,
	ERRTYPE_UEU,
	ERRTYPE_UC,
	ERRTYPE_MAX
};

typedef union {
	struct {
		u64 ed:2;            //ED, bits [1:0]
		u64 imp_defined3:2;  //IMPLEMENTATION DEFINED, bits [3:2]
		u64 ui:2;            //UI, bits [5:4]
		u64 fi:2;            //FI, bits [7:6]
		u64 ue:2;            //UE, bits [9:8]
		u64 cfi:2;           //CFI, bits [11:10]
		u64 cec:3;           //CEC, bits [14:12]
		u64 rp:1;            //RP, bit [15]
		u64 dui:2;           //DUI, bits [17:16]
		u64 ceo:2;           //CEO, bits [19:18]
		u64 inj:2;           //INJ, bits [21:20]
		u64 ci:2;            //CI, bits [23:22]
		u64 ts:2;            //TS, bits [25:24]
		u64 resv0:5;         //Bits [30:26], reserved
		u64 frx:1;           //FRX, bit [31]

		u64 imp_defined2:16;//IMPLEMENTATION DEFINED, bits [47:32]
		u64 uc:1;           //UC, bit [48]
		u64 ueu:1;          //UEU, bit [49]
		u64 uer:1;          //UER, bit [50]
		u64 ueo:1;          //UEO, bit [51]
		u64 de:1;           //DE, bit [52]
		u64 ce:2;           //CE, bits [54:53]
		u64 imp_defined1:1; //IMPLEMENTATION DEFINED, bit [55]
		u64 imp_defined0:8; //IMPLEMENTATION DEFINED, bits [63:56]
	};
	u64 raw;
} ras_err_fr_reg_t;

typedef union {
	struct {
		u64 num:16;          //NUM, bits [15:0]
		u64 resv:48;         //Bits [63:16], reserved
	};
	u64 raw;
} ras_err_idr_reg_t;

typedef union {
	struct {
		u64 of:1;            //OF, bit [0]
		u64 uc:1;            //UC, bit [1]
		u64 ueu:1;           //UEU, bit [2]
		u64 uer:1;           //UER, bit [3]
		u64 ueo:1;           //UEO, bit [4]
		u64 de:1;            //DE, bit [5]
		u64 ce:2;            //CE, bits [7:6]
		u64 ci:1;            //CI, bit [8]
		u64 er:1;            //ER, bit [9]
		u64 pn:1;            //PN, bit [10]
		u64 av:1;            //AV, bit [11]
		u64 mv:1;            //MV, bit [12]
		u64 resv1:15;        //Bits [27:13], reserved
		u64 na:1;            //NA, bit [28]
		u64 syn:1;           //SYN, bit [29]
		u64 r:1;             //R, bit [30]
		u64 resv0:33;        //Bits [63:31], reserved
	};
	u64 raw;
} ras_err_pfgf_reg_t;

typedef union {
	struct {
		u64 of:1;            //OF, bit [0]
		u64 uc:1;            //UC, bit [1]
		u64 ueu:1;           //UEU, bit [2]
		u64 uer:1;           //UER, bit [3]
		u64 ueo:1;           //UEO, bit [4]
		u64 de:1;            //DE, bit [5]
		u64 ce:2;            //CE, bits [7:6]
		u64 ci:1;            //CI, bit [8]
		u64 er:1;            //ER, bit [9]
		u64 pn:1;            //PN, bit [10]
		u64 av:1;            //AV, bit [11]
		u64 mv:1;            //MV, bit [12]
		u64 resv1:17;        //Bits [29:13], reserved
		u64 r:1;             //R, bit [30]
		u64 cdnen:1;         //CDNEN, bit [31]
		u64 resv0:32;        //Bits [63:32], reserved
	};
	u64 raw;
} ras_err_pfgctl_reg_t;

typedef union {
	struct {
		u64 cdn:32;          //CDN, bits [31:0]
		u64 resv:32;         //Bits [63:32], reserved
	};
	u64 raw;
} ras_err_pfgcdn_reg_t;

typedef union {
	struct {
		u64 serr:8;          //SERR, bits [7:0]
		u64 ierr:8;          //IERR, bits [15:8]
		u64 resv1:3;         //Bits [18:16], reserved
		u64 ci:1;            //CI, bit [19]
		u64 uet:2;           //UET, bits [21:20]
		u64 pn:1;            //PN, bit [22]
		u64 de:1;            //DE, bit [23]
		u64 ce:2;            //CE, bits [25:24]
		u64 mv:1;            //MV, bit [26]
		u64 of:1;            //OF, bit [27]
		u64 er:1;            //ER, bit [28]
		u64 ue:1;            //UE, bit [29]
		u64 v:1;             //V, bit [30]
		u64 av:1;            //AV, bit [31]
		u64 resv0:32;        //Bits [63:32] ,reserved
	};
	u64 raw;
} ras_err_status_reg_t;

#define SYS_ERXPFGCDN_EL1			sys_reg(3, 0, 5, 4, 6)
#define SYS_ERXPFGCTL_EL1			sys_reg(3, 0, 5, 4, 5)
#define SYS_ERXPFGF_EL1				sys_reg(3, 0, 5, 4, 4)


#endif
