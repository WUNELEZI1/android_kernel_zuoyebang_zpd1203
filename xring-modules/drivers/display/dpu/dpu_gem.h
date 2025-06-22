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

#ifndef _DPU_GEM_H_
#define _DPU_GEM_H_

#include <linux/types.h>
#include <linux/scatterlist.h>
#include <drm/drm_device.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_framebuffer_helper.h>

#include "dpu_hw_dmmu_tbu_ops.h"

/**
 * dpu_gem_object - the dpu gem object structure
 * @base: the drm gem object
 * @sgt: the sg table for dpu gem object
 * @dmabuf: the sg table for dpu gem object
 * @is_dumb_buf: mark true if used dumb ubffer
 * @paddr: the physical address pointer
 * @vaddr: the virtual address pointer
 * @iova_dom: iova domain include tlb_info, iova_addr and iova_size
 * @vmap_use_count: the reference count of vmap
 * @vmap_lock: the lock of vmap accessing
 */
struct dpu_gem_object {
	struct drm_gem_object base;
	struct sg_table *sgt;
	struct dma_buf *dmabuf;
	bool is_dumb_buf;

	dma_addr_t paddr;
	void *vaddr;

	struct iova_domain *iova_dom;

	u32 vmap_use_count;
	struct mutex vmap_lock;
};

#define to_dpu_gem_obj(x) \
	container_of(x, struct dpu_gem_object, base)

/**
 * dpu_gem_dumb_create - the dpu implementation of dumb buffer
 * @file_priv: the drm file private pointer
 * @drm: the drm device pointer
 * @args: the arguments of dumb requirements
 *
 * Return: 0 on success, -ERRNO on failure
 */
int dpu_gem_dumb_create(struct drm_file *file_priv,
		struct drm_device *drm_dev,
		struct drm_mode_create_dumb *args);

/**
 * dpu_gem_prime_import_sg_table - the dpu implementation for
 *                                 import_sg_table callback
 * @dev: the drm device pointer
 * @attach: the pointer of dma_buf attachment
 * @sgt: the sg table pointer
 *
 * Return: valid pointer on success, error pointer on failure
 */
struct drm_gem_object *dpu_gem_prime_import_sg_table(struct drm_device *dev,
		struct dma_buf_attachment *attach,
		struct sg_table *sgt);

/**
 * dpu_drm_gem_fb_create() - the dpu implementation for the
 *                           drm framebuffer create
 * @dev: DRM device
 * @file: DRM file that holds the GEM handle(s) backing the framebuffer
 * @mode_cmd: Metadata from the userspace framebuffer creation request
 *
 * Returns:
 * Pointer to a &drm_framebuffer on success or an error pointer on failure.
 */
struct drm_framebuffer *
dpu_drm_gem_fb_create(struct drm_device *dev, struct drm_file *file,
		const struct drm_mode_fb_cmd2 *mode_cmd);

/**
 * dpu_gem_get_framebuffer_info - convert drm_framebuffer to framebuffer_info
 * @fb: pointer of drm_framebuffer
 * @fb_info: pointer of framebuffer_info
 */
void dpu_gem_get_framebuffer_info(struct drm_framebuffer *fb,
		struct framebuffer_info *fb_info);

#endif /* _DPU_GEM_H_ */
