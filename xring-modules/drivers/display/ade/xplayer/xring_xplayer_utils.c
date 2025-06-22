// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include <linux/dma-buf.h>
#include <linux/errno.h>
#include <linux/dma-resv.h>

#include "xring_xplayer_drv.h"
#include "xring_xplayer_utils.h"

int g_xplayer_msg_level = 2;

void xplayer_slice_enc_done(struct dma_buf *buf, int offset,
		int length, int slice_index, u64 timestamp)
{
	struct xplayer_source_data tmp;
	struct xplayer_data *priv;
	size_t ret;

	priv = platform_get_drvdata(xplayer_get_device());
	if (!(priv->flags & XPLAYER_FLAG_SOURCE)) {
		XRING_XPLAYER_WARN("not init by source");
		return;
	}

	XRING_XPLAYER_DBG("buf: %p, offset: %d, length: %d, slice_index: %d",
			buf, offset, length, slice_index);
	tmp.buf = buf;
	tmp.info.type = XPLAYER_INFO_ENC_SLICE;
	tmp.info.slice_info.offset = offset;
	tmp.info.slice_info.length = length;
	tmp.info.slice_info.slice_index = slice_index;
	tmp.info.slice_info.pts = timestamp;

	if (kfifo_is_full(&priv->info_work.data)) {
		XRING_XPLAYER_ERR("kfifo is full");
		return;
	}

	ret = kfifo_in_locked(&priv->info_work.data, &tmp, sizeof(tmp),
			&priv->info_work.lock);
	if (ret != sizeof(tmp))
		XRING_XPLAYER_ERR("kfifo in error");
	else
		wake_up_interruptible(&priv->info_work.wq);
}
EXPORT_SYMBOL(xplayer_slice_enc_done);

void xplayer_set_display_dma_buf(struct dma_buf *buf,
		struct xplayer_cmdlist_config *config,
		struct xplayer_iommu_format_info *iommu_info)
{
	struct xplayer_data *priv;
	int tlb_index;

	priv = platform_get_drvdata(xplayer_get_device());
	if (!(priv->flags & XPLAYER_FLAG_SINK)) {
		XRING_XPLAYER_WARN("not init by sink");
		return;
	}

	if (!iommu_info) {
		XRING_XPLAYER_WARN("iommu_info is NULL");
		return;
	}

	dpu_wfd_sink_get_cmdlist_config(config->write_value, &tlb_index);
	priv->sink_work.data.buf = buf;
	priv->sink_work.data.tlb_index = tlb_index;
	memcpy(&priv->sink_work.data.iommu_info, iommu_info, sizeof(*iommu_info));

	XRING_XPLAYER_DBG("buf: %p, tlb index: %d", buf, tlb_index);

	queue_work(priv->wq, &priv->sink_work.work);
}
EXPORT_SYMBOL(xplayer_set_display_dma_buf);

void xplayer_get_cmdlist(struct xplayer_cmdlist_addr *info)
{
	struct xplayer_data *priv;

	priv = platform_get_drvdata(xplayer_get_device());
	if (!(priv->flags & XPLAYER_FLAG_SINK)) {
		XRING_XPLAYER_WARN("not init by sink");
		return;
	}

	dpu_wfd_sink_get_cmdlist_addr(info->phy_addr);
	XRING_XPLAYER_DBG("info->phy_addr: 0x%llx, 0x%llx, 0x%llx, 0x%llx",
			info->phy_addr[0], info->phy_addr[1], info->phy_addr[2], info->phy_addr[3]);
}
EXPORT_SYMBOL(xplayer_get_cmdlist);

void xplayer_notify_event(enum xplayer_event ev)
{
	struct xplayer_source_data tmp;
	struct xplayer_data *priv;
	size_t ret;

	XRING_XPLAYER_DBG("!");

	priv = platform_get_drvdata(xplayer_get_device());
	tmp.info.type = (enum xplayer_info_type)ev;
	if (kfifo_is_full(&priv->info_work.data)) {
		XRING_XPLAYER_DBG("kfifo is full");
		return;
	}

	ret = kfifo_in_locked(&priv->info_work.data, &tmp, sizeof(tmp),
			&priv->info_work.lock);
	if (ret != sizeof(tmp))
		XRING_XPLAYER_DBG("kfifo in error");
	else
		wake_up_interruptible(&priv->info_work.wq);
}

MODULE_IMPORT_NS(DMA_BUF);
