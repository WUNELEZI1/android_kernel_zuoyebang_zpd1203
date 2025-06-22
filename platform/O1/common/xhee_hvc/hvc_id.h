/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: hvc fid definitions
 * This is a document file for reference
 *
 * 0xc3000000-0xc300FFFF HVC64: OEM Service Calls
 *
 * 0xc300 0000  ~  0xc300 FFFF +------> sip fid define
 *                        | |
 *                        | |
 *                        | +----------> Fid in module  (0 ~ 255)
 *                        |
 *                        +------------> Module Group   (0 ~ 255)
 *
 *
 * 0xc6000000-0xc600FFFF HVC64: vendor specific hypervisor service calls
 *
 * 0xc600 0000  ~  0xc600 FFFF +------> sip fid define
 *                        | |
 *                        | |
 *                        | +----------> Fid in module  (0 ~ 255)
 *                        |
 *                        +------------> Module Group   (0 ~ 255)
 *
 * check usage:
 *              FID_USE(fid) == XHEE_BASE_GROUP
 *
 * Modify time: 2024-05-15
 * Author: Security-AP
 *
 */
#pragma once

/*
 * Service Call Ranges:
 * bit 31  30  29  28  27  26  25  24 ... 0
 * val 1   1   0   0   0   1   1   0  ... 0
 * according to SMC Calling Convention document:
 * - bit31: always set to 1 for fast calls.
 * - bit30: set to 1 for SMC64/HVC64.
 * - bit29-24: Owning Entity Number.
 *               - set to 0x3 for oem service calls.
 *               - set to 0x6 for vendor specific hypervisor service calls.
 */
#define OEM_BASE                           0xc3U
#define VENDOR_SPECIFIC_HYP_BASE           0xc6U
#define VENDOR_SPECIFIC_HYP32_BASE         0x86U

/* Get FID from group and id */
#define make_hvc_fid(oen, group, id)        (((((oen) << 16) + group) << 8) + id)
#define make_oem_fid(group, id)             make_hvc_fid(OEM_BASE, group, id)
#define make_vendor_spec_hyp_fid(group, id) make_hvc_fid(VENDOR_SPECIFIC_HYP_BASE, group, id)
#define make_vendor_spec_hyp32_fid(grp, id) make_hvc_fid(VENDOR_SPECIFIC_HYP32_BASE, grp, id)

/*
 * OEM Service Module Group Number Define, must be named as follow:
 * XHEE_MODULE_GROUP
 */
#define XHEE_BASE_GROUP    0x00
#define XHEE_KMEM_PROTECT_GROUP 0x02
#define XHEE_SCHEDGENIUS_REGULATE_GROUP 0x03
#define XHEE_RIP_GROUP     0x04
#define XHEE_QUIRK_MEM_GROUP 0x06
#define FID_XHEE_MAXGROUP  0xff /* max group */

/*
 * Start and end FID of OEM
 */
#define FID_OEM_START make_oem_fid(XHEE_BASE_GROUP, 0x00)
#define FID_OEM_END   make_oem_fid(FID_XHEE_MAXGROUP, 0xFF)

/*
 * xhee xkip control:
 * 0xc3000200 ~ 0xc30002ff
 */
#define FID_KMEM_PROTECT_DEBUG  make_oem_fid(XHEE_KMEM_PROTECT_GROUP, 0x00)
#define FID_KMOD_BE_INIT        make_oem_fid(XHEE_KMEM_PROTECT_GROUP, 0x01)
#define FID_KMOD_AF_INIT        make_oem_fid(XHEE_KMEM_PROTECT_GROUP, 0x02)
#define FID_KMOD_FR_MOD         make_oem_fid(XHEE_KMEM_PROTECT_GROUP, 0x03)
#define FID_KMOD_LOADED         make_oem_fid(XHEE_KMEM_PROTECT_GROUP, 0x04)

/*
 * xhee schedgenius regulate:
 * 0xc3000300 ~ 0xc30003ff
 */
#define FID_XHEE_SCHEDGENIUS_REGULATE make_oem_fid(XHEE_SCHEDGENIUS_REGULATE_GROUP, 0x00)
#define FID_XHEE_SCHEDGENIUS_DISCOUNT make_oem_fid(XHEE_SCHEDGENIUS_REGULATE_GROUP, 0x01)

/*
 * xhee rip regulate:
 * 0xc3000400 ~ 0xc30004ff
 */
#define FID_XHEE_RIP_BASE                   make_oem_fid(XHEE_RIP_GROUP, 0x00)
#define FID_XHEE_RIP_REALTIME               make_oem_fid(XHEE_RIP_GROUP, 0x01)
#define FID_XHEE_RIP_GET_STATUS             make_oem_fid(XHEE_RIP_GROUP, 0x02)

/*
 * xhee quirk memory handle:
 * 0xc3000600 ~ 0xc30006ff
 */
#define FID_XHEE_MEM_RECLAIM                make_oem_fid(XHEE_QUIRK_MEM_GROUP, 0x00)
#define FID_XHEE_MEM_LEND                   make_oem_fid(XHEE_QUIRK_MEM_GROUP, 0x01)
