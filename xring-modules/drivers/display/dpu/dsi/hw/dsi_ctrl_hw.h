/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DSI_CTRL_HW_H_
#define _DSI_CTRL_HW_H_

#include <linux/platform_device.h>

#include "dpu_hw_dsi.h"
#include "dsi_hw_ctrl_ops.h"
#include "dpu_reg_ops.h"

/**
 * enum dsi_ctrl_version - DSI Ctrl version enumeration
 * @DSI_CTRL_VERSION_UNKNOWN:    Unknown version.
 * @DSI_CTRL_VERSION_2_0:        version 2_0
 * @DSI_CTRL_VERSION_MAX:
 */
enum dsi_ctrl_version {
	DSI_CTRL_VERSION_UNKNOWN = 0,
	DSI_CTRL_VERSION_2_0,
	DSI_CTRL_VERSION_MAX
};

struct dsi_ctrl_hw;

/**
 * struct dsi_ctrl_hw_ops - operations supported by dsi host hardware
 */
struct dsi_ctrl_hw_ops {
	int (*init)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	void (*deinit)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*clk_config)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*ulps_enter)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*ulps_exit)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*timing_setup)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*engine_setup)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*enable)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*disable)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	void (*reset)(struct dsi_ctrl_hw_blk *hw);


	int (*pre_init)(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
	int (*phy_init)(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

	int (*hw_init)(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
	int (*wait_ready)(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

	int (*send_pkt)(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg);
	int (*send_pkt_cmdlist)(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg);
	int (*read_pkt)(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg, u32 wait_timeout);
	int (*read_pkt_pre_tx)(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg);

	int (*mipi_freq_update)(struct dsi_ctrl_hw_blk *hw,
			struct dsi_ctrl_cfg *cfg);
	int (*dsi_debug)(struct dsi_ctrl_hw_blk *hw,
			enum dsi_debug type, u8 parm_cnt, u8 *parm);
	int (*cri_is_busy)(struct dsi_ctrl_hw_blk *hw, u32 timeout_ms);
	int (*cri_tx_ctrl)(struct dsi_ctrl_hw_blk *hw, u8 cri_mode, u8 cri_hold);
};

/**
 * dsi_ctrl_hw - dsi controller hardware abstraction
 * @ctrl_blk: the ctrl hw blk
 * @sctrl_blk: the sctrl hw blk
 * @dctrl_blk: the ctrl hw blk with the offset of dpu
 * @dsctrl_blk: the sctrl hw blk with the offset of dpu
 * @ctrl_blks: the dsi ctrl hw blk, which is the input param of ops func
 *
 * @version: the ctrl hw version info
 *
 * @ops: ctrl hw operaction functions
 */
struct dsi_ctrl_hw {
	struct dpu_hw_blk ctrl_blk;
	struct dpu_hw_blk sctrl_blk;
	struct dpu_hw_blk dctrl_blk;
	struct dpu_hw_blk dsctrl_blk;

	/* get phy_blk in display probe process */
	struct dsi_ctrl_hw_blk ctrl_blks;

	enum dsi_ctrl_version version;

	struct dsi_ctrl_hw_ops ops;
};

int dsi_ctrl_hw_parse(struct platform_device *pdev,
		struct dsi_ctrl_hw *ctrl_hw);

#endif
