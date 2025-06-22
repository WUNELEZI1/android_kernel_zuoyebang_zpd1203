// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _VDEC_LOG_H_
#define _VDEC_LOG_H_

#include "xring_vpu_log.h"

// Set log level of all driver groups to Verbose
#define VDEC_LOGMASK_DEFAULT 0xFFFF;

extern int vsi_kloglvl;
extern ulong vsi_klogMask;

enum {
	LOGMASK_VDEC  = 0,		// log vdec driver
	LOGMASK_VCMD  = 4,		// log vcmd
	LOGMASK_PM    = 8,		// log vdec power manager
	LOGMASK_CM    = 12,		// log clk manager
	LOGMASK_AXIFE = 16,		// log axife
	LOGMASK_MDR   = 20,		// log mdr
	LOGMASK_MMU   = 24,		// log mmu
	LOGMASK_REG   = 28,		// log register operations
	LOGMASK_Max   = 60,
};

#define vdec_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][VDEC ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VDEC) & 0xF))	\
			vpu_klog_base("[VDEC %s][VDEC ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vcmd_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][VCMD ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VCMD) & 0xF))	\
			vpu_klog_base("[VDEC %s][VCMD ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_pm_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][PM   ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_PM) & 0xF))	\
			vpu_klog_base("[VDEC %s][PM   ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_cm_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][CM   ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_CM) & 0xF))	\
			vpu_klog_base("[VDEC %s][CM   ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_axife_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][AXIFE]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_AXIFE) & 0xF))	\
			vpu_klog_base("[VDEC %s][AXIFE]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_init_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_INFO)	{	\
		vpu_klog_base("[VDEC %s][INIT ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VDEC) & 0xF))	\
			vpu_klog_base("[VDEC %s][VDEC ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_mdr_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][MDR  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_MDR) & 0xF))	\
			vpu_klog_base("[VDEC %s][MDR  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_mmu_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][MMU  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_MMU) & 0xF))	\
			vpu_klog_base("[VDEC %s][MMU  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vdec_reg_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VDEC %s][REG  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_REG) & 0xF))	\
			vpu_klog_base("[VDEC %s][REG  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}
#endif