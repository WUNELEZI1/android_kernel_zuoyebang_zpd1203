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

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include "dp_aux.h"
#include "dpu_hw_common.h"
#include "dp_display.h"

#define AUX_REPLY_TIMEOUT_32MS                               32000

/* BIT(AUX_REQ_I2C_SHIFT) = 0 indicates i2c, 1 indicates native */
# define AUX_REQ_I2C_SHIFT                                   3
# define AUX_REQ_MOT_SHIFT                                   2
# define AUX_REQ_RW_SHIFT                                    0
# define AUX_REQ_RW_LEN                                      2
# define AUX_REQ_READ                                        0x1
# define AUX_REQ_WRITE                                       0x0
# define AUX_REQ_WRITE_STATUS_UPDATE                         0x2

static const char * const dp_aux_rw_str[] = {
	[AUX_REQ_WRITE] = "write",
	[AUX_REQ_READ] = "read",
	[AUX_REQ_WRITE_STATUS_UPDATE] = "write_status_update",
};

static inline void dp_aux_dump_msg(struct dp_aux_request_info *rinfo,
		u8 reply_type, u8 payload_size)
{
	DP_INFO("aux addr:0x%08x, aux rw:%s, is_i2c:%d, is_mot:%d,"
			" req size:%zu, reply type:0x%04x, payload size:%d\n",
			rinfo->address, dp_aux_rw_str[rinfo->rw], rinfo->is_i2c,
			rinfo->is_mot, rinfo->size, reply_type, payload_size);
}

static const char *__reply_type_str(u8 reply_type)
{
	switch (reply_type) {
	case AUX_REPLY_ACK:
		return "ACK";
	case AUX_REPLY_NACK:
		return "NACK";
	case AUX_REPLY_DEFER:
		return "DEFER";
	case AUX_REPLY_I2C_NACK:
		return "I2C_NACK";
	case AUX_REPLY_I2C_DEFER:
		return "I2C_DEFER";
	default:
		return "unknown";
	}
}

static void dp_aux_parse_request(struct dp_aux *aux, struct drm_dp_aux_msg *msg)
{
	struct dp_aux_request_info *rinfo = &aux->request_info;

	rinfo->is_i2c = !(msg->request & BIT(AUX_REQ_I2C_SHIFT));
	rinfo->is_mot = !!(msg->request & BIT(AUX_REQ_MOT_SHIFT));
	rinfo->rw = GET_BITS_VAL(msg->request, AUX_REQ_RW_SHIFT, AUX_REQ_RW_LEN);
	rinfo->request = msg->request;
	rinfo->size = msg->size;
	rinfo->buffer = msg->buffer;
	rinfo->address = msg->address;
}

static int dp_aux_msg_valid(struct dp_aux *aux)
{
	struct dp_aux_request_info *rinfo = &aux->request_info;

	if (rinfo->buffer == NULL && rinfo->size != 0) {
		DP_ERROR("aux rw buffer is NULL\n");
		return -EINVAL;
	}

	if ((!rinfo->is_i2c) && rinfo->is_mot) {
		DP_ERROR("mot = %d, is not allowed in native aux request\n", rinfo->is_mot);
		return -EINVAL;
	}

	if (rinfo->rw != AUX_REQ_READ && rinfo->rw != AUX_REQ_WRITE &&
			rinfo->rw != AUX_REQ_WRITE_STATUS_UPDATE) {
		DP_ERROR("unknown request type: 0x%x\n", rinfo->request);
		return -EINVAL;
	}

	if (rinfo->size > AUX_MAX_BYTES_SIZE) {
		DP_ERROR("size = %zu, should be less than %d\n",
				rinfo->size, AUX_MAX_BYTES_SIZE);
		return -EINVAL;
	} else if (rinfo->size > 0) {
		if (rinfo->rw == AUX_REQ_WRITE_STATUS_UPDATE ||
				(rinfo->is_i2c && !rinfo->is_mot)) {
			DP_ERROR("size = %zu, should be equal to 0\n", rinfo->size);
			return -EINVAL;
		}
	} else {  /* rinfo->size == 0 */
		if (!rinfo->is_i2c) {
			DP_ERROR("size = %zu, is not allowed in native aux request\n",
					rinfo->size);
			return -EINVAL;
		}
	}

	return 0;
}

static int dp_aux_check_reply_status(struct dp_aux *aux)
{
	struct dp_aux_reply_status *reply_status = &aux->reply_status;

	if (reply_status->err_status) {
		if (reply_status->err_status & DP_AUX_REPLY_STATUS_CMD_INVALID)
			DP_WARN("aux request cmd is invalid\n");

		if (reply_status->err_status & DP_AUX_REPLY_STATUS_ERR)
			DP_WARN("failed to get aux reply, error code:0x%x\n",
					reply_status->err_code);

		if (reply_status->err_status & DP_AUX_REPLY_STATUS_TIMEOUT)
			DP_WARN("timeout in waiting aux reply\n");

		if (reply_status->err_status & DP_AUX_REPLY_STATUS_DISCONNECTED)
			DP_WARN("sink is disconnected in waiting aux reply\n");

		return -EINVAL;
	}

	if (!reply_status->reply_received) {
		DP_WARN("aux reply is not received\n");
		return -EINVAL;
	}

	return 0;
}

static void dp_aux_handle_reply(struct dp_aux *aux, u8 *reply_type, u8 *payload_size)
{
	struct dp_aux_request_info *rinfo = &aux->request_info;
	struct dp_aux_reply_status *reply_status = &aux->reply_status;

	switch (reply_status->reply_type) {
	case AUX_REPLY_ACK:
		if (rinfo->rw == AUX_REQ_WRITE || rinfo->rw == AUX_REQ_WRITE_STATUS_UPDATE)
			/**
			 * if bytes_read = 1 (reply is reply_type7:4|0000), AUX_M is
			 * not existed, all bytes have been written.
			 */
			*payload_size = reply_status->bytes_read == 1 ? rinfo->size : reply_status->aux_m;
		else if (rinfo->rw == AUX_REQ_READ)
			*payload_size = reply_status->bytes_read - 1;
		break;
	case AUX_REPLY_I2C_NACK:
	case AUX_REPLY_NACK:
		if (rinfo->rw == AUX_REQ_WRITE || rinfo->rw == AUX_REQ_WRITE_STATUS_UPDATE)
			/* if bytes_read = 1, it means sink nack and no bytes was wrote. */
			*payload_size = reply_status->bytes_read == 1 ? 0 : reply_status->aux_m;
		else if (rinfo->rw == AUX_REQ_READ)
			*payload_size = 0;
		break;
	case AUX_REPLY_I2C_DEFER:
	case AUX_REPLY_DEFER:
		*payload_size = 0;
		break;
	default:
		DP_ERROR("unknown reply type: 0x%x\n", reply_status->reply_type);
		*payload_size = 0;
	}

	*reply_type = reply_status->reply_type;
}

static int _dp_aux_rw(struct dp_aux *aux, u8 *reply_type, u8 *payload_size)
{
	struct dp_aux_request_info *rinfo = &aux->request_info;
	struct dp_hw_ctrl *ctrl = aux->hw_ctrl;
	int ret;

	reinit_completion(&aux->reply_completed);

	if (rinfo->rw == AUX_REQ_WRITE)
		ctrl->ops->write_aux_data(&ctrl->hw, rinfo->buffer, rinfo->size);

	/* the request is sent to sink after AUX_CMD is been wrote */
	ctrl->ops->send_aux_request(&ctrl->hw, rinfo->request, rinfo->address, rinfo->size);

	/* wait for an aux irq */
	ret = wait_for_completion_timeout(&aux->reply_completed,
			usecs_to_jiffies(AUX_REPLY_TIMEOUT_32MS));
	if (!ret) {
		*payload_size = 0;
		*reply_type = rinfo->is_i2c ? AUX_REPLY_I2C_DEFER : AUX_REPLY_DEFER;

		DP_INFO("timeout in waiting aux intr\n");
		ret = -ETIMEDOUT;
		goto error;
	}

	ret = dp_aux_check_reply_status(aux);
	if (ret) {
		*payload_size = 0;
		goto error;
	}

	/* parse aux reply, get reply_type and payload_size */
	dp_aux_handle_reply(aux, reply_type, payload_size);

	if (*reply_type != AUX_REPLY_ACK) {
		DP_WARN("aux reply type is %s\n", __reply_type_str(*reply_type));
		dp_aux_dump_msg(rinfo, *reply_type, *payload_size);
	}

	if (*payload_size < rinfo->size)
		DP_DEBUG("data transferred incompletely, payload size %d, msg size %zu\n",
				*payload_size, rinfo->size);

	if (*payload_size > rinfo->size) {
		DP_WARN("wrong payload size %d, msg size is %zu\n",
				*payload_size, rinfo->size);
		ret = -EPROTO;
		goto error;
	}

	if (rinfo->rw == AUX_REQ_READ)
		ctrl->ops->read_aux_data(&ctrl->hw, rinfo->buffer, *payload_size);

	return 0;

error:
	dp_aux_dump_msg(rinfo, *reply_type, *payload_size);
	return ret;
}

ssize_t dp_aux_transfer(struct drm_dp_aux *drm_aux,
		struct drm_dp_aux_msg *msg)
{
	struct dp_aux *dp_aux;
	u8 payload_size;
	int ret = 0;

	dp_aux = container_of(drm_aux, struct dp_aux, base);

	dp_aux_parse_request(dp_aux, msg);

	ret = dp_aux_msg_valid(dp_aux);
	if (ret)
		goto error;

	ret = _dp_aux_rw(dp_aux, &msg->reply, &payload_size);
	if (ret)
		goto error;

	return payload_size;

error:
	return ret;
}

/* aux interrupt operation */
static int dp_aux_irq_enable(void *data, bool enable)
{
	struct dp_aux *aux = data;

	aux->hw_ctrl->ops->enable_intr(&aux->hw_ctrl->hw, DP_INTR_AUX, enable);

	return 0;
}

static bool dp_aux_irq_is_triggered(void *data)
{
	struct dp_aux *aux = data;

	return aux->hw_ctrl->ops->get_intr_state(&aux->hw_ctrl->hw, DP_INTR_AUX);
}

static int dp_aux_irq_clear(void *data)
{
	struct dp_aux *aux = data;

	aux->hw_ctrl->ops->clear_intr_state(&aux->hw_ctrl->hw, DP_INTR_AUX);

	return 0;
}

static int dp_aux_irq_handler(void *data)
{
	struct dp_aux *aux = data;

	aux->hw_ctrl->ops->get_aux_reply_status(&aux->hw_ctrl->hw, &aux->reply_status);

	complete(&aux->reply_completed);

	return 0;
}

int dp_aux_init(struct dp_display *display, struct dp_aux **aux)
{
	struct dp_aux *aux_priv;

	aux_priv = kzalloc(sizeof(*aux_priv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(aux_priv))
		return -ENOMEM;

	aux_priv->aux_intr_ops = kzalloc(sizeof(*aux_priv->aux_intr_ops), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(aux_priv->aux_intr_ops)) {
		kfree(aux_priv);
		return -ENOMEM;
	}

	aux_priv->hw_ctrl = display->hw_modules.hw_ctrl;
	aux_priv->display = display;
	init_completion(&aux_priv->reply_completed);

	/* register aux interrupt service */
	aux_priv->aux_intr_ops->data          = aux_priv;
	aux_priv->aux_intr_ops->enable        = dp_aux_irq_enable;
	aux_priv->aux_intr_ops->is_triggered  = dp_aux_irq_is_triggered;
	aux_priv->aux_intr_ops->clear         = dp_aux_irq_clear;
	aux_priv->aux_intr_ops->handler       = dp_aux_irq_handler;

	*aux = aux_priv;
	dp_display_irq_register(display, (*aux)->aux_intr_ops);

	return 0;
}

void dp_aux_deinit(struct dp_aux *aux)
{
	dp_display_irq_unregister(aux->display, aux->aux_intr_ops);
	kfree(aux->aux_intr_ops);
	kfree(aux);
}
