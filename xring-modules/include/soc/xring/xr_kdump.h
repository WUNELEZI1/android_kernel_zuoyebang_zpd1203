/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xr kdump header
 */

#ifndef __XR_KDUMP_H__
#define __XR_KDUMP_H__

#pragma pack(4)
struct xr_kdump_cb {
	u64 magic; /* kdump magic, default 0xdeadbeefdeadbeef */
	u64 text_phy_base; /* physcal address of text */
	u64 text_virt_base; /* virtual address of text */
	u64 text_size; /* size of text */
	u64 pcpu_phy_base; /* physcal address of pcpu */
	u64 pcpu_virt_base; /* virtual address of pcpu */
	u64 pcpu_size; /* size of pcpu */
	u64 dfx_switch;
	u32 crc;
};
#pragma pack()

extern void get_text_section(uintptr_t *text_start, uintptr_t *text_end);
extern int pcpu_base_size;

int xr_kdump_save(void);

#endif /* __XR_KDUMP_H__ */
