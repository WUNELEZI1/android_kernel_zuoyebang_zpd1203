/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_CAM_RPC_BP_H_
#define _XRISP_CAM_RPC_BP_H_

#include <linux/workqueue.h>
#include <media/v4l2-event.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

extern int xrisp_pipe_power_down(struct xrisp_power_down *powerup_set);
extern int xrisp_pipe_power_up(struct xrisp_power_up *powerup_set);

int xrisp_cam_rpc_run_func(u8 *data, size_t size);

#endif
