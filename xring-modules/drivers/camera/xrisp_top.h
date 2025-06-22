/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_MAIN_H_
#define _XRISP_MAIN_H_

#include "xrisp_tee_ca_api.h"

struct xrisp_driver_submodule {
	char *submodule_name;
	int (*init)(void);
	void (*exit)(void);
};

extern int xrisp_debugfs_init(void);
extern void xrisp_debugfs_exit(void);

extern int xrisp_cdm_init(void);
extern void xrisp_cdm_exit(void);

extern int  xrisp_cbm_init(void);
extern void  xrisp_cbm_exit(void);

extern int xrisp_ipcm_init(void);
extern void xrisp_ipcm_exit(void);

extern int xrisp_rproc_init(void);
extern void xrisp_rproc_exit(void);

extern int xrisp_rpmsg_ping_init(void);
extern void xrisp_rpmsg_ping_exit(void);

extern int xrisp_rpmsg_epts_init(void);
extern void xrisp_rpmsg_epts_exit(void);

extern int xrisp_clk_init(void);
extern void xrisp_clk_exit(void);

extern int xrisp_regulator_init(void);
extern void xrisp_regulator_exit(void);

extern int xrisp_flash_init(void);
extern void xrisp_flash_exit(void);

extern int xrisp_tof_init(void);
extern void xrisp_tof_exit(void);

extern int xrisp_multi_tof_init(void);
extern void xrisp_multi_tof_exit(void);

extern int xrisp_ca_init(void);
extern void xrisp_ca_exit(void);

extern int xrisp_sensor_init(void);
extern void xrisp_sensor_exit(void);

extern int xrisp_csiphy_init(void);
extern void xrisp_csiphy_exit(void);

extern int xring_edr_event_init(void);
extern void xring_edr_event_exit(void);

extern int xring_edr_wdt_init(void);
extern void xring_edr_wdt_exit(void);

extern int xring_edr_ramlog_init(void);
extern void xring_edr_ramlog_exit(void);

#ifdef CONFIG_XRING_ISP_EDR_TRACE
extern int xring_edr_ramtrace_init(void);
extern void xring_edr_ramtrace_exit(void);
#endif

extern int xring_mdr_init(void);
extern void xring_mdr_exit(void);

extern int xrisp_thermal_init(void);
extern void xrisp_thermal_exit(void);

extern int xrisp_privacy_led_init(void);
extern void xrisp_privacy_led_exit(void);
#endif /* _XRISP_MAIN_H_ */
