/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xr dfx header
 */

#ifndef __XR_DFX_MAIN_H__
#define __XR_DFX_MAIN_H__

#if IS_ENABLED(CONFIG_XRING_TRACE_HOOK)
int trace_hook_init(void);
#else
static inline int trace_hook_init(void) { return 0; }
#endif

#if IS_ENABLED(CONFIG_XRING_DFX_SWITCH)
int dfx_switch_driver_init(void);
void dfx_switch_driver_exit(void);
#else
static inline int dfx_switch_driver_init(void) { return 0; }
static inline void dfx_switch_driver_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_MDR)
int mdr_init(void);
#else
static inline int mdr_init(void) { return 0; }
#endif

#if IS_ENABLED(CONFIG_XRING_REBOOT_REASON)
int reboot_reason_init(void);
void reboot_reason_exit(void);
#else
static inline int reboot_reason_init(void) { return 0; }
static inline void reboot_reason_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_FASTBOOTLOG_SAVE)
int fastbootlog_init(void);
void fastbootlog_exit(void);
#else
static inline int fastbootlog_init(void) { return 0; }
static inline void fastbootlog_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_MEMDUMP)
int memdump_init(void);
void memdump_exit(void);
#else
static inline int memdump_init(void) { return 0; }
static inline void memdump_exit(void) { }
#endif

#endif /* __XR_DFX_MAIN_H__ */
