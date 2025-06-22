/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DPU_HW_REG_MAP_H_
#define _DPU_HW_REG_MAP_H_

#define RDMA_TOP_G0_START                                             0x1000
#define RDMA_TOP_G0_END                                               0x13FF
#define RDMA_PREPQ_G0_START                                           0x1400
#define RDMA_PREPQ_G0_END                                             0x1FFF

#define RDMA_TOP_V0_START                                             0x2000
#define RDMA_TOP_V0_END                                               0x23FF
#define RDMA_PREPQ_V0_START                                           0x2400
#define RDMA_PREPQ_V0_END                                             0xAFFF

#define RDMA_TOP_G1_START                                             0xB000
#define RDMA_TOP_G1_END                                               0xB3FF
#define RDMA_PREPQ_G1_START                                           0xB400
#define RDMA_PREPQ_G1_END                                             0xBFFF

#define RDMA_TOP_V1_START                                             0xC000
#define RDMA_TOP_V1_END                                               0xC3FF
#define RDMA_PREPQ_V1_START                                           0xC400
#define RDMA_PREPQ_V1_END                                             0xCFFF

#define RDMA_TOP_G2_START                                             0xD000
#define RDMA_TOP_G2_END                                               0xD3FF
#define RDMA_PREPQ_G2_START                                           0xD400
#define RDMA_PREPQ_G2_END                                             0xDFFF

#define RDMA_TOP_G3_START                                             0xE000
#define RDMA_TOP_G3_END                                               0xE3FF
#define RDMA_PREPQ_G3_START                                           0xE400
#define RDMA_PREPQ_G3_END                                             0xEFFF

#define RDMA_TOP_G4_START                                             0x10000
#define RDMA_TOP_G4_END                                               0x103FF
#define RDMA_PREPQ_G4_START                                           0x10400
#define RDMA_PREPQ_G4_END                                             0x10FFF

#define RDMA_TOP_V2_START                                             0x11000
#define RDMA_TOP_V2_END                                               0x113FF
#define RDMA_PREPQ_V2_START                                           0x11400
#define RDMA_PREPQ_V2_END                                             0x11FFF

#define RDMA_TOP_V3_START                                             0x12000
#define RDMA_TOP_V3_END                                               0x123FF
#define RDMA_PREPQ_V3_START                                           0x12400
#define RDMA_PREPQ_V3_END                                             0x12FFF

#define RDMA_TOP_G5_START                                             0x13000
#define RDMA_TOP_G5_END                                               0x133FF
#define RDMA_PREPQ_G5_START                                           0x13400
#define RDMA_PREPQ_G5_END                                             0x13FFF

#define COMP0_START                                                   0x18000
#define COMP0_END                                                     0x1827F
#define COMP0_POST_SCALER_START                                       0x29800
#define COMP0_POST_SCALER_END                                         0x2B7FF

#define COMP1_START                                                   0x30000
#define COMP1_END                                                     0x3027F
#define RDMA_2DSCL_V0_START                                           0x41800
#define RDMA_2DSCL_V0_END                                             0x437FF
#define RDMA_2DSCL_V1_START                                           0x43800
#define RDMA_2DSCL_V1_END                                             0x457FF

#define COMP2_START                                                   0x48000
#define COMP2_END                                                     0x48FFF
#define RDMA_2DSCL_V2_START                                           0x49000
#define RDMA_2DSCL_V2_END                                             0x4AFFF
#define RDMA_2DSCL_V3_START                                           0x4B000
#define RDMA_2DSCL_V3_END                                             0x4CFFF

#endif /* _DPU_HW_REG_MAP_H_ */
