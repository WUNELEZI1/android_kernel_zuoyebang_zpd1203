/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __PLATFORM_DVS_RESOURCE_H__
#define __PLATFORM_DVS_RESOURCE_H__

/* dvs wait ack timeout */
#define EDVS_TMOT           0xE335
/* dvs current volt does not meet expectations */
#define EDVS_FAIL           0xE336
/* dvs timeout irq happend */
#define EDVS_TMOT_IRQ       0xE337

#define DVS_CH_NUM          16
#define DVS_VLD_TIMEOUT     500
#define DVS_UNSEC_CH_NUM    8
#define DVS_CH_OFFSET(ch)                                                      \
	(((ch) >= DVS_UNSEC_CH_NUM)                                            \
	? (DVS_VOLT_REQ_8 + ((ch) - DVS_UNSEC_CH_NUM) * 0x4U)                    \
	: (DVS_VOLT_REQ_0 + (ch)*0x4U))

struct dvs_hw {
        unsigned int base;
        unsigned char ch;
};

union dvs_sw_reg {
	struct {
		unsigned int volt_req       : 8;
		unsigned int req_valid      : 1;
		unsigned int rsvd1          : 7;
                unsigned int volt_req_mask  : 8;
                unsigned int req_valid_mask : 1;
                unsigned int rsvd2          : 7;
	} mem;
	unsigned int data;
};

union dvs_cur_state_reg {
	struct {
		unsigned int arbit_state  : 2;
		unsigned int ctrl_state   : 3;
                unsigned int volt_req     : 8;
                unsigned int volt_arbit   : 8;
                unsigned int volt_cur     : 8;
		unsigned int rsvd         : 3;
	} mem;
	unsigned int data;
};

#endif
