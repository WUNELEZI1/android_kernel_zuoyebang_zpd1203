/* SPDX-License-Identifier: GPL-2.0-only
 *
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
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/version.h>
#include <uapi/linux/dma-heap.h>

struct audio_memory_dma_buf {
	struct list_head list;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
	struct iosys_map map;
	struct sg_table *sgt;
	dma_addr_t src_dma_addr;
	dma_addr_t dst_dma_addr;
	char *buf;
	char *vaddr;
	int len;
	int fd;
	struct dma_chan *chan;
	struct completion *completion;
	bool data_compare_ok;
};

dma_addr_t get_audio_memory_phys(void);
char *get_audio_memory_vaddr(void);
struct audio_memory_dma_buf *fk_audio_buf_alloc(struct device *dev, size_t bufsz);
void fk_audio_buf_free(struct audio_memory_dma_buf *priv);
char *fk_audio_buffer_alloc(struct device *dev, size_t size, dma_addr_t *dma_handle);
void fk_audio_buffer_free(struct device *dev, size_t size, void *vaddr, dma_addr_t dma_handle);

