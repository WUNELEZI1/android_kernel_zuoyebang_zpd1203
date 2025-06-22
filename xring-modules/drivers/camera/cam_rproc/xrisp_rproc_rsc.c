// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/stddef.h>
#include <linux/virtio_ids.h>
#include "xrisp_rproc.h"

const struct xrisp_rproc_rsc_table rproc_rsc_table = {
	.rsc_tbl_hdr = {
		.ver = 1,
		.num = 2,
	},
	.offset = {
		offsetof(struct xrisp_rproc_rsc_table, log_trace_hdr),
		offsetof(struct xrisp_rproc_rsc_table, rpmsg_vdev_hdr),
	},
	.log_trace_hdr.type = XRISP_RSC_VENDOR_RAMLOG,
	.log_trace = {
	/* ap handle the log_trace as vendor rsc */
		.da = FW_RSC_ADDR_ANY,
		.len = 0,
		.name = "ramlog",
	},
	.rpmsg_vdev_hdr.type = XRISP_RSC_VENDOR_VDEV,
	.rpmsg_vdev = {
		.id = VIRTIO_ID_RPMSG,
		.dfeatures  = 1 << VIRTIO_RPMSG_F_NS,
		.config_len = 8,
		.num_of_vrings = 2,
		.notifyid = 31,
	},
	.rpmsg_vring0  = {
		.da = FW_RSC_ADDR_ANY,
		.align = 64,
		.num = 128,
		.notifyid = FW_RSC_ADDR_ANY,
	},
	.rpmsg_vring1 = {
		.da = FW_RSC_ADDR_ANY,
		.align = 64,
		.num = 128,
		.notifyid = FW_RSC_ADDR_ANY,
	},
	.rpmsg_vbuffer = {
		.da = FW_RSC_ADDR_ANY,
		.pa = FW_RSC_ADDR_ANY,
	},
	.bd_info = {
		.mcu_clock = FW_RSC_ADDR_ANY,
		.uart_clock = FW_RSC_ADDR_ANY,
		.timer_clock = FW_RSC_ADDR_ANY,
	}
};
