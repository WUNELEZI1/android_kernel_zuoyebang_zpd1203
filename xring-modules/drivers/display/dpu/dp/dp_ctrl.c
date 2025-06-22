// SPDX-License-Identifier: GPL-2.0
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

#include "dp_ctrl.h"
#include "dp_display.h"

static int dp_ctrl_start_link_training(struct dp_ctrl *ctrl)
{
	struct dp_link *link = ctrl->link;
	bool is_success = false;
	int ret;

	DP_DEBUG("start to link training\n");
	if (link->funcs->training_init(link))
		goto exit;

	while (1) {
		DP_DEBUG("link training cr...\n");
		ret = link->funcs->training_cr(link);
		if (ret == -EAGAIN)
			continue;
		else if (ret < 0)
			goto exit;

		DP_DEBUG("link training eq...\n");
		ret = link->funcs->training_eq(link);
		if (ret == -EAGAIN)
			continue;
		else if (ret < 0)
			goto exit;

		break;
	}

	is_success = true;

exit:
	return link->funcs->training_completed(link, is_success);
}

void dp_ctrl_enable_video(struct dp_ctrl *ctrl, bool enable)
{
	struct dp_hw_ctrl *hw_ctrl = ctrl->hw_ctrl;
	struct dp_link *link = ctrl->link;

	hw_ctrl->ops->enable_video_transfer(&hw_ctrl->hw, enable);

	link->funcs->dump_link_status(link);
}

void dp_ctrl_enable_audio(struct dp_ctrl *ctrl,
		struct dp_audio_fmt_info *ainfo, bool enable)
{
	struct dp_hw_ctrl *hw_ctrl = ctrl->hw_ctrl;

	if (enable)
		hw_ctrl->ops->config_audio_info(&hw_ctrl->hw, ainfo);

	hw_ctrl->ops->enable_audio_transfer(&hw_ctrl->hw, enable);
}

static struct dp_ctrl_funcs dp_ctrl_funcs = {
	.enable_video                 = dp_ctrl_enable_video,
	.enable_audio                 = dp_ctrl_enable_audio,
	.start_link_training          = dp_ctrl_start_link_training,
};

int dp_ctrl_init(struct dp_display *display, struct dp_ctrl **ctrl)
{
	struct dp_ctrl *ctrl_priv;
	struct dp_link *link;
	int ret;

	ctrl_priv = kzalloc(sizeof(*ctrl_priv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(ctrl_priv))
		return -ENOMEM;

	ret = dp_link_init(display->aux, display->hw_modules.hw_ctrl,
			display->hw_modules.hw_sctrl, display->hw_modules.hw_msgbus, &link);
	if (ret)
		goto link_err;

	ctrl_priv->display = display;
	ctrl_priv->link = link;
	ctrl_priv->power = display->power;
	ctrl_priv->hw_ctrl = display->hw_modules.hw_ctrl;

	ctrl_priv->funcs = &dp_ctrl_funcs;

	*ctrl = ctrl_priv;

	return 0;

link_err:
	kfree(ctrl_priv);

	return ret;
}

void dp_ctrl_deinit(struct dp_ctrl *ctrl)
{
	dp_link_deinit(ctrl->link);
	kfree(ctrl);
}
