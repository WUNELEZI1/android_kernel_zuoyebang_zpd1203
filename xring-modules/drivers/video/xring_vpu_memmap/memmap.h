/* SPDX-License-Identifier: GPL-2.0 */
/**
 * xring dec support smmu
 *
 */

#ifndef __MEMMAP__H__
#define __MEMMAP__H__

#include <linux/kernel.h>
#include <linux/dma-buf.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "xring_vpu_log.h"

struct vpu_dma_buf {
	int fd;
	unsigned long iova;
	int flag;
};

extern int vsi_kloglvl;
#define memmap_klog(lvl, fmt, ...) {	\
	if (lvl <= LOGLVL_INFO) {	\
		vpu_klog_base("[VPU  %s][MEMMAP]", lvl, fmt, ##__VA_ARGS__);	\
	} else if (lvl <= vsi_kloglvl) {	\
		vpu_klog_base("[VPU  %s][MEMMAP]", lvl, fmt, ##__VA_ARGS__);	\
	}	\
}

#define MEMMAP_IOC_MAGIC 'V'
#define MEMMAP_IOVA _IOWR(MEMMAP_IOC_MAGIC, 1, struct vpu_dma_buf)
#define MEMUNMAP_IOVA  _IOWR(MEMMAP_IOC_MAGIC, 2, struct vpu_dma_buf)
#define MEMMAP_ABNORMAL  _IO(MEMMAP_IOC_MAGIC, 3)
#define MEMMAP_IOC_MAXNR 3

#endif //__VDEC_SMMU__H__
