// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _VENC_LOG_H_
#define _VENC_LOG_H_

#include "xring_vpu_log.h"

// Set log level of all driver groups to Verbose
#define VENC_LOGMASK_DEFAULT 0xFFFF;

extern int vsi_kloglvl;
extern ulong vsi_klogMask;

enum {
	LOGMASK_VENC  = 0,		// log vdec driver
	LOGMASK_VCMD  = 4,		// log vcmd
	LOGMASK_PM    = 8,		// log vdec power manager
	LOGMASK_CM    = 12,		// log clk manager
	LOGMASK_AXIFE = 16,		// log axife
	LOGMASK_MDR   = 20,		// log mdr
	LOGMASK_MMU   = 24,		// log mmu
	LOGMASK_REG   = 28,		// log register operations
	LOGMASK_Max   = 60,
};

#define venc_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][VENC ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VENC) & 0xF))	\
			vpu_klog_base("[VENC %s][VENC ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define vcmd_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][VCMD ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VCMD) & 0xF))	\
			vpu_klog_base("[VENC %s][VCMD ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_pm_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][PM   ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_PM) & 0xF))	\
			vpu_klog_base("[VENC %s][PM   ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_cm_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][CM   ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_CM) & 0xF))	\
			vpu_klog_base("[VENC %s][CM   ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_axife_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][AXIFE]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_AXIFE) & 0xF))	\
			vpu_klog_base("[VENC %s][AXIFE]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_init_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_INFO)	{	\
		vpu_klog_base("[VENC %s][INIT ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_VENC) & 0xF))	\
			vpu_klog_base("[VENC %s][VENC ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_mdr_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][MDR  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_MDR) & 0xF))	\
			vpu_klog_base("[VENC %s][MDR  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_mmu_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][MMU  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_MMU) & 0xF))	\
			vpu_klog_base("[VENC %s][MMU  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define venc_reg_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_WARNING)	{	\
		vpu_klog_base("[VENC %s][REG  ]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		if (lvl <= ((vsi_klogMask >> LOGMASK_REG) & 0xF))	\
			vpu_klog_base("[VENC %s][REG  ]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}
#endif