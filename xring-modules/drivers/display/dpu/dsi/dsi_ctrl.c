// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/of.h>

#include "hw/dsi_version_ctrl.h"
#include "hw/dsi_ctrl_hw.h"
#include "dsi_ctrl.h"
#include "dpu_log.h"

static LIST_HEAD(dsi_ctrl_list);
static DEFINE_MUTEX(dsi_ctrl_list_lock);

int dsi_ctrl_cri_tx_ctrl(struct dsi_ctrl *ctrl, u8 cri_mode, u8 cri_hold)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl) {
		DSI_ERROR("invalid parameters, %pK\n", ctrl);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.cri_tx_ctrl) {
		ret = ctrl_hw->ops.cri_tx_ctrl(&ctrl_hw->ctrl_blks, cri_mode, cri_hold);
		if (ret) {
			DSI_DEBUG("failed to set cri_tx_ctrl\n");
			return -EBUSY;
		}
	} else {
		DSI_ERROR("no ctrl operation: cri_tx_ctrl\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_cri_is_busy(struct dsi_ctrl *ctrl, u32 wait_timeout)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl) {
		DSI_ERROR("invalid parameters, %pK\n", ctrl);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.cri_is_busy) {
		ret = ctrl_hw->ops.cri_is_busy(&ctrl_hw->ctrl_blks, wait_timeout);
		if (ret) {
			DSI_WARN("failed to check cri busy\n");
			return -EBUSY;
		}
	} else {
		DSI_ERROR("no ctrl operation: cri_is_busy\n");
		return -EINVAL;
	}

	return 0;
}

inline void dsi_ctrl_lock(struct dsi_ctrl *ctrl)
{
	static int count;

	DSI_DEBUG("request ctrl-%d lock %d\n", dsi_ctrl_hw_blk_id_get(ctrl), count);
	mutex_lock(&ctrl->ctrl_lock);
	count++;
}

inline void dsi_ctrl_unlock(struct dsi_ctrl *ctrl)
{
	static int count;

	DSI_DEBUG("release ctrl-%d lock %d\n", dsi_ctrl_hw_blk_id_get(ctrl), count);
	mutex_unlock(&ctrl->ctrl_lock);
	count++;
}

inline int dsi_ctrl_hw_blk_id_get(struct dsi_ctrl *ctrl)
{
	return ctrl->ctrl_hw.ctrl_blk.blk_id;
}

int dsi_ctrl_check_ulps_state(struct dsi_ctrl *ctrl, bool enter)
{
	struct dsi_ctrl_hw *ctrl_hw;

	if (!ctrl) {
		DSI_ERROR("invalid parameters, %pK\n", ctrl);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	return dsi_hw_check_ulps_state(&ctrl_hw->ctrl_blks, enter);
}

int dsi_ctrl_ulps_enter(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	if (ctrl_hw->ops.ulps_enter) {
		ret = ctrl_hw->ops.ulps_enter(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to process ulps enter\n");
			return ret;
		}
	} else {
		DSI_ERROR("no ctrl operation: ulps_enter\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

int dsi_ctrl_ulps_exit(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.ulps_exit) {
		ret = ctrl_hw->ops.ulps_exit(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to process ulps exit\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: ulps exit\n");
		return -EOPNOTSUPP;
	}

	return 0;
}

int dsi_ctrl_enable(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	if (ctrl->ctrl_hw.ops.enable)
		return ctrl->ctrl_hw.ops.enable(&ctrl->ctrl_hw.ctrl_blks, cfg);

	DSI_DEBUG("no ctrl operation: enable\n");

	return -EINVAL;
}

static inline void dsi_ctrl_get_dsi_msg(struct dsi_cmd_desc *cmd_desc,
		struct dsi_msg *msg)
{
	msg->channel = cmd_desc->msg.channel;
	msg->type = cmd_desc->msg.type;
	msg->flags = cmd_desc->msg.flags;

	msg->tx_len = cmd_desc->msg.tx_len;
	msg->tx_buf = cmd_desc->msg.tx_buf;

	msg->rx_len = cmd_desc->msg.rx_len;
	msg->rx_buf = cmd_desc->msg.rx_buf;
}

int dsi_ctrl_cmdlist_cmd_send(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc)
{
	struct dsi_ctrl_hw *ctrl_hw;
	struct dsi_msg dsi_msg;
	int ret = 0;

	if (!ctrl || !cmd_desc) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cmd_desc);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	dsi_ctrl_get_dsi_msg(cmd_desc, &dsi_msg);

	if (ctrl_hw->ops.send_pkt_cmdlist) {
		ret = ctrl_hw->ops.send_pkt_cmdlist(&ctrl_hw->ctrl_blks, &dsi_msg);
		if (ret) {
			DSI_ERROR("failed to send_pkt_cmdlist\n");
			goto error;
		}
	} else {
		DSI_ERROR("no ctrl operation: send_pkt_cmdlist\n");
		ret = -EOPNOTSUPP;
		goto error;
	}

error:
	return ret;
}

int dsi_ctrl_cmd_read_pre_tx(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd)
{
	struct dsi_ctrl_hw *ctrl_hw;
	struct dsi_msg dsi_msg;
	int ret = 0;

	if (unlikely(!ctrl || !cmd)) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cmd);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	dsi_ctrl_get_dsi_msg(cmd, &dsi_msg);

	if (ctrl_hw->ops.send_pkt) {
		ret = ctrl_hw->ops.read_pkt_pre_tx(&ctrl_hw->ctrl_blks, &dsi_msg);
		if (ret) {
			DSI_ERROR("failed to send read_pkt_pre_tx\n");
			goto error;
		}
	} else {
		DSI_ERROR("no ctrl operation: read_pkt_pre_tx\n");
		ret = -EOPNOTSUPP;
		goto error;
	}

error:
	return ret;
}

int dsi_ctrl_cpu_cmd_send(struct dsi_ctrl *ctrl, struct dsi_cmd_desc *cmd_desc)
{
	struct dsi_ctrl_hw *ctrl_hw;
	struct dsi_msg dsi_msg;
	int ret = 0;

	if (!ctrl || !cmd_desc) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cmd_desc);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	dsi_ctrl_get_dsi_msg(cmd_desc, &dsi_msg);

	if (ctrl_hw->ops.send_pkt) {
		ret = ctrl_hw->ops.send_pkt(&ctrl_hw->ctrl_blks, &dsi_msg);
		if (ret) {
			DSI_ERROR("failed to send pkt\n");
			goto error;
		}
	} else {
		DSI_ERROR("no ctrl operation: send_pkt\n");
		ret = -EOPNOTSUPP;
		goto error;
	}

error:
	return ret;
}

static int dsi_ctrl_cmd_read(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc, u32 wait_timeout)
{
	struct dsi_ctrl_hw *ctrl_hw;
	struct dsi_msg dsi_msg;
	int ret;

	if (!ctrl || !cmd_desc) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cmd_desc);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	dsi_ctrl_get_dsi_msg(cmd_desc, &dsi_msg);

	if (ctrl_hw->ops.read_pkt)
		ret = ctrl_hw->ops.read_pkt(&ctrl_hw->ctrl_blks, &dsi_msg, wait_timeout);
	else {
		DSI_ERROR("no ctrl operation: read_pkt\n");
		ret = -EINVAL;
	}

	return ret;
}

int dsi_ctrl_cmd_transfer(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc, u8 transfer_type, u32 wait_timeout)
{
	int ret = 0;

	if (!cmd_desc->msg.rx_len && !cmd_desc->msg.tx_len) {
		DSI_ERROR("error message format\n");
		ret = -EINVAL;
	} else if (cmd_desc->msg.rx_len > 0) {
		if (transfer_type == USE_CMDLIST)
			DSI_ERROR("get invalid transfer type USE_CMDLIST for read pkt\n");
		ret = dsi_ctrl_cmd_read(ctrl, cmd_desc, wait_timeout);
		if (ret)
			DSI_ERROR("read message failed, ret=%d\n", ret);
	} else {
		dsi_ctrl_cmd_send(ctrl, cmd_desc, transfer_type, ret);
		if (ret)
			DSI_ERROR("send message failed, ret=%d\n", ret);
	}

	return ret;
}

static inline void dsi_ctrl_get_dsi_cmds(struct dsi_cmd_set *cmd_set,
		struct dsi_cmds *dsi_cmds)
{
	dsi_cmds->count = cmd_set->num_cmds;
	dsi_cmds->transfer_type = cmd_set->type;
	dsi_cmds->msg = (struct dsi_msg *)(&cmd_set->cmds->msg);
}

int dsi_ctrl_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.init) {
		ret = ctrl_hw->ops.init(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to init ctrl hw\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: init\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_pre_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.pre_init) {
		ret = ctrl_hw->ops.pre_init(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to pre init ctrl hw\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: pre_init\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_phy_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.phy_init) {
		ret = ctrl_hw->ops.phy_init(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to init ctrl phy\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: phy_init\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_hw_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.hw_init) {
		ret = ctrl_hw->ops.hw_init(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to init ctrl hw\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: hw_init\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_hw_wait_ready(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;

	if (ctrl_hw->ops.wait_ready) {
		ret = ctrl_hw->ops.wait_ready(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("wait dsi ready timeout!\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: wait_ready\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_mipi_freq_update(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", ctrl, cfg);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	if (ctrl_hw->ops.mipi_freq_update) {
		ret = ctrl_hw->ops.mipi_freq_update(&ctrl_hw->ctrl_blks, cfg);
		if (ret) {
			DSI_ERROR("failed to update mipi freq\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: mipi_freq_update\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_ctrl_debug(struct dsi_ctrl *ctrl, enum dsi_debug type,
		u8 parm_cnt, u8 *parm)
{
	struct dsi_ctrl_hw *ctrl_hw;
	int ret = 0;

	if (!ctrl) {
		DSI_ERROR("invalid parameters, %pK\n", ctrl);
		return -EINVAL;
	}

	ctrl_hw = &ctrl->ctrl_hw;
	if (ctrl_hw->ops.dsi_debug) {
		ret = ctrl_hw->ops.dsi_debug(&ctrl_hw->ctrl_blks,
			type, parm_cnt, parm);
		if (ret) {
			DSI_ERROR("dsi debug error\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no ctrl operation: dsi_debug\n");
		return -EINVAL;
	}

	return 0;
}

void dsi_ctrl_add(struct dsi_ctrl *ctrl)
{
	mutex_lock(&dsi_ctrl_list_lock);
	list_add_tail(&ctrl->list, &dsi_ctrl_list);
	mutex_unlock(&dsi_ctrl_list_lock);
}

void dsi_ctrl_remove(struct dsi_ctrl *ctrl)
{
	mutex_lock(&dsi_ctrl_list_lock);
	list_del_init(&ctrl->list);
	mutex_unlock(&dsi_ctrl_list_lock);
}

#ifdef CONFIG_OF
struct dsi_ctrl *of_find_dsi_ctrl(const struct device_node *np)
{
	struct dsi_ctrl *ctrl = NULL;
	bool found = false;

	if (!of_device_is_available(np))
		return ERR_PTR(-ENODEV);

	mutex_lock(&dsi_ctrl_list_lock);
	list_for_each_entry(ctrl, &dsi_ctrl_list, list) {
		if (ctrl->dev->of_node == np) {
			found = true;
			break;
		}
	}
	mutex_unlock(&dsi_ctrl_list_lock);

	if (!found)
		ctrl = ERR_PTR(-ENODEV);

	return ctrl;
}
#endif

int dsi_ctrl_obj_init(struct dsi_ctrl *ctrl, struct platform_device *pdev)
{
	if (!of_device_is_available(pdev->dev.of_node))
		return -ENODEV;

	if (dsi_ctrl_hw_parse(pdev, &ctrl->ctrl_hw)) {
		DSI_ERROR("failed to parse ctrl hardware info\n");
		return -EINVAL;
	}

	if (dsi_ctrl_version_ctrl(&ctrl->ctrl_hw)) {
		DSI_ERROR("failed to process version ctrl\n");
		return -EINVAL;
	}

	mutex_init(&ctrl->ctrl_lock);
	INIT_LIST_HEAD(&ctrl->list);
	ctrl->dev = &pdev->dev;

	return 0;
}

/**
 * dsi_ctrl_obj_deinit - release dsi ctrl reserved resource
 * @ctrl: the dsi phy pointer
 */
static inline void dsi_ctrl_obj_deinit(struct dsi_ctrl *ctrl)
{
}

static int dsi_ctrl_pdev_probe(struct platform_device *pdev)
{
	struct dsi_ctrl *dsi_ctrl;
	int ret;

	dsi_ctrl = kzalloc(sizeof(*dsi_ctrl), GFP_KERNEL);
	if (!dsi_ctrl)
		return -ENOMEM;

	ret = dsi_ctrl_obj_init(dsi_ctrl, pdev);
	if (ret) {
		DSI_ERROR("failed to init dsi ctrl\n");
		goto error;
	}

	dsi_ctrl_add(dsi_ctrl);

	platform_set_drvdata(pdev, dsi_ctrl);

	return 0;

error:
	kfree(dsi_ctrl);
	return ret;
}

static int dsi_ctrl_pdev_remove(struct platform_device *pdev)
{
	struct dsi_ctrl *ctrl;

	ctrl = platform_get_drvdata(pdev);

	dsi_ctrl_remove(ctrl);

	dsi_ctrl_obj_deinit(ctrl);

	kfree(ctrl);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id dsi_ctrl_dt_match[] = {
	{ .compatible = "xring,dsi-ctrl" },
	{},
};

static struct platform_driver dsi_ctrl_platform_driver = {
	.probe = dsi_ctrl_pdev_probe,
	.remove = dsi_ctrl_pdev_remove,
	.driver = {
		.name = "xring_dsi_ctrl",
		.of_match_table = dsi_ctrl_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init dsi_ctrl_drv_register(void)
{
	return platform_driver_register(&dsi_ctrl_platform_driver);
}

void __exit dsi_ctrl_drv_unregister(void)
{
	platform_driver_unregister(&dsi_ctrl_platform_driver);
}
