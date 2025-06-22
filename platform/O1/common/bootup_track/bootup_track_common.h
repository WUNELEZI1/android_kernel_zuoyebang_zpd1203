// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __BOOTUP_TRACK_COMMON_H__
#define __BOOTUP_TRACK_COMMON_H__

#define BOOTUP_TRACK_DATA_HDR_MAGIC 0x5A5A5A5A

#define BOOTROM_THRESHOLD 600
#define XLOADER_THRESHOLD 600
#define UEFI_THRESHOLD 3000
#define KERNEL_THRESHOLD 1700
#define ANDROID_THRESHOLD 20000
#define DDR_FULL_BOOT_TIME 6000

enum bootup_stage {
    STAGE_BOOTROM = 0,
    STAGE_XLOADER,
    STAGE_UEFI,
    STAGE_KERNEL,
    STAGE_ANDROID,
    STAGE_MAX
};

enum bootup_track_event {
    BOOTUP_TRACK_EVENT_AUTH,
    BOOTUP_TRACK_EVENT_BOOT_TIMEOUT,
};

struct bootup_track_data {
    uint32_t magic;
    uint32_t auth_state;
    uint64_t boot_time[STAGE_MAX];
    uint8_t  stayed_in_fastboot;
    uint8_t  ddr_fullboot;
} __attribute__((packed));

#endif
