// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 XiaoMi, Inc.
 * All Rights Reserved.
 */

#include <linux/dma-heap.h>
#include <uapi/linux/dma-heap.h>
#include "soc/xring/xr_dmabuf_helper.h"
#include "tui_hal_xring.h"

static struct tui_alloc_buffer_t g_font_buffer = { 0 };

uint32_t hal_tui_alloc(const char *alloc_name,
	struct tui_alloc_buffer_t *alloc_buffer, size_t alloc_size)
{
	uint32_t ret = TUI_ERR_INTERNAL_ERROR;
	struct dma_heap *dma_heap = NULL;
	struct dma_buf *dma_buf = NULL;
	uint32_t sfd;

	tui_dev_info("%s: alloc for %s\n", __func__, alloc_name);

	if (!alloc_buffer) {
		ret = TUI_ERR_BAD_PARAMETERS;
		tui_dev_err(ret, "%s(%d): alloc_buffer is null\n", __func__, __LINE__);
		return ret;
	}

	if ((size_t)alloc_size == 0) {
		pr_notice("%s(%d): Nothing to allocate\n", __func__, __LINE__);
		return TUI_ERR_BAD_PARAMETERS;
	}

	dma_heap = dma_heap_find(alloc_name);
	if (IS_ERR_OR_NULL(dma_heap)) {
		ret = TUI_ERR_INTERNAL_ERROR;
		tui_dev_err(ret, "heap find failed!\n");
		return ret;
	}

	dma_buf = dma_heap_buffer_alloc(dma_heap, alloc_size, O_RDWR, 0);
	if (IS_ERR(dma_buf)) {
		ret = TUI_ERR_OUT_OF_MEMORY;
		tui_dev_err(ret, "%s, alloc buffer fail, heap:%s", __func__,
				dma_heap_get_name(dma_heap));
		return ret;
	}

	sfd = xr_dmabuf_helper_get_sfd(dma_buf);
	if (sfd == 0) {
		ret = TUI_ERR_INTERNAL_ERROR;
		tui_dev_err(ret, "%s, get tui frame buffer secure handle failed!\n", __func__);
		goto dma_buf_free;
	}

	alloc_buffer->dma_buf = dma_buf;
	alloc_buffer->pa = sfd;
	xr_dmabuf_kernel_account(dma_buf, XR_DMABUF_KERNEL_MITEE_TUI);
	tui_dev_devel("%s(%d):handle 0x%llx\n", __func__, __LINE__, alloc_buffer->pa);
	return TUI_OK;

dma_buf_free:
	dma_heap_buffer_free(dma_buf);
	return ret;
}

void hal_tui_free(void)
{
	struct tui_alloc_buffer_t *fb_buffer = NULL;

	tui_dev_devel("[TUI-HAL] %s\n", __func__);

	hal_get_fb_buffer(&fb_buffer);
	if (fb_buffer->dma_buf != NULL) {
		xr_dmabuf_kernel_unaccount(fb_buffer->dma_buf, XR_DMABUF_KERNEL_MITEE_TUI);
		dma_heap_buffer_free(fb_buffer->dma_buf);
		fb_buffer->dma_buf = NULL;
	}
	if (g_font_buffer.dma_buf != NULL) {
		xr_dmabuf_kernel_unaccount(g_font_buffer.dma_buf, XR_DMABUF_KERNEL_MITEE_TUI);
		dma_heap_buffer_free(g_font_buffer.dma_buf);
		g_font_buffer.dma_buf = NULL;
	}
}

int tui_get_panel_vendor(void)
{
	int value = 0;
	return value;
}

uint32_t hal_alloc_font_buffer(uint32_t font_size)
{
	uint32_t ret = TUI_OK;

	if (g_font_buffer.pa == 0) {
		ret = hal_tui_alloc("xring_tui_font", &g_font_buffer, font_size);
		if (ret != TUI_OK) {
			tui_dev_err(ret, "tui alloc failed. err = %d\n", ret);
			return ret;
		}
	}

	return ret;
}

void hal_get_font_buffer(struct tui_alloc_buffer_t **buffer)
{
	*buffer = &g_font_buffer;

}
