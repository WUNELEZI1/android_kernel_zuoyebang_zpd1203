// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UAPI_XRFLASH_H__
#define __UAPI_XRFLASH_H__

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/media.h>

#define FlashMaxLEDNumber 2

struct flash_cmd_header
{
    __u32 cmdType;
    __u32 payloadSize;
    __u32 reserved;
};

typedef enum flash_cmd_type
{
    FlashCmdTypeInvalid = 0,
    FlashCmdTypeInit,
    FlashCmdTypeBring,
    FlashCmdTypeRER,
    FlashCmdTypeQueryCurrent,
    FlashCmdTypeWidget,
    FlashCmdTypeDeInit,
    FlashCmdTypeMax,
} flash_cmd_type_t;

typedef struct xrisp_flash_info_cmd
{
    __u32 flashType;
    __u16 cmdType;
    __u16 reserved;
} xrisp_flash_info_cmd_t;

typedef enum flash_opstatus
{
    FlashOpstatusInvaild = 0,
    FlashOpstatusOff,
    FlashOpstatusBringLow,
    FlashOpstatusBringHigh,
    FlashOpstatusBringPrecision,
    FlashOpstatusMax,
} flash_opstatus_t;

typedef struct xrisp_flash_bring_cmd
{
    __u32 count;
    __u8  opstatus;
    __u8  reserved;
    __u16 cmdType;
    __u32 currentMilliampere[FlashMaxLEDNumber];
    __u64 flashDuration;
    __u64 flashOnWaitTime;
} xrisp_flash_bring_cmd_t;

typedef struct xrisp_flash_querry_current_cmd
{
    __u16 reserved;
    __u16 cmdType;
    __u32 currentMilliampere[FlashMaxLEDNumber];
} xrisp_flash_querry_current_cmd_t;

typedef struct xrisp_flash_rercmd
{
    __u32 count;
    __u16 cmdType;
    __u16 numberOfIteration;
    __u32 flashOnDelayMillisecond;
    __u32 flashOffDelayMillisecond;
    __u32 currentMilliampere[FlashMaxLEDNumber];
    __u32 reserved;
} xrisp_flash_rercmd_t;

#pragma pack(1)
typedef struct xrisp_flash_connect_cmd
{
    __u32                    sensor_id;
    struct flash_cmd_header  header;
    union
    {
        xrisp_flash_info_cmd_t           flash_info;
        xrisp_flash_bring_cmd_t          flash_bring;
        xrisp_flash_querry_current_cmd_t querry_current;
        xrisp_flash_rercmd_t             flash_rercmd;
    } upayload;
} xrisp_flash_connect_cmd_t;
#pragma pack()


#endif
