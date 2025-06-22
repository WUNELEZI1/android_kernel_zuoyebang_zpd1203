// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/mutex.h>
#include <linux/tracepoint.h>
#include <soc/xring/xr_dmabuf_helper.h>

#include "dksm_mem_pool.h"
#include "dksm_iommu.h"
#include "dpu_log.h"
#include "dpu_format.h"

#define IOVA_START_ADDR 0x10000000
#define IOVA_POOL_SIZE  0x200000000 /* 8G */
#define IOVA_RESERVED_BASE 0x80000000ULL

static struct dksm_iommu_domain g_iommu_domain;

struct dksm_iommu_domain *dksm_iommu_get_instance(void)
{
	return &g_iommu_domain;
}

static struct gen_pool *dksm_iommu_iova_pool_setup(unsigned long start,
		unsigned long size, unsigned long align)
{
	struct gen_pool *pool = NULL;
	int ret;

	/*
	 * iova start should not be 0, because return
	 * 0 when alloc iova is considered as error
	 */
	if (!start || !size) {
		DPU_ERROR("iova start or size should not be 0!\n");
		return NULL;
	}

	pool = gen_pool_create(order_base_2(align), -1);
	if (!pool) {
		DPU_ERROR("create gen pool failed!\n");
		return NULL;
	}

	ret = gen_pool_add(pool, start, size, -1);
	if (ret) {
		DPU_ERROR("gen pool add failed!\n");
		gen_pool_destroy(pool);
		return NULL;
	}

	DPU_DEBUG("create pool start:0x%lx, size:%#lx, align:0x%lx\n", start, size, align);
	return pool;
}

static unsigned long dksm_iommu_alloc_iova(struct gen_pool *iova_pool,
		size_t size, unsigned long align)
{
	if (iova_pool->min_alloc_order >= 0) {
		if (align > (1UL << (unsigned long)iova_pool->min_alloc_order))
			DPU_WARN("iommu domain cant align to 0x%lx\n", align);
	} else {
		DPU_WARN("The min_alloc_order of iova_pool is negative!\n");
		return 0;
	}

	return gen_pool_alloc(iova_pool, size);
}

static void dksm_iommu_free_iova(struct gen_pool *iova_pool,
		unsigned long iova, size_t size)
{
	if (!iova_pool)
		return;

	gen_pool_free(iova_pool, iova, size);
}

static void dksm_iommu_iova_add(struct rb_root *rb_root,
		struct iova_domain *iova_dom)
{
	struct rb_node **p = &rb_root->rb_node;
	struct rb_node *parent = NULL;
	struct iova_domain *entry = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct iova_domain, node);
		if (entry->key == iova_dom->key)
			DPU_ERROR("ERROR! dmabuf-iova mapping already exists!\n"
				   "iova=0x%lx, size=0x%x, key=0x%llx, pid=%d\n",
				   entry->iova, entry->size, entry->key, entry->pid);

		if (iova_dom < entry) {
			p = &(*p)->rb_left;
		} else if (iova_dom > entry) {
			p = &(*p)->rb_right;
		} else {
			DPU_ERROR("iova already in tree\n");
			return;
		}
	}

	rb_link_node(&iova_dom->node, parent, p);
	rb_insert_color(&iova_dom->node, rb_root);
}

static struct iova_domain *dksm_iommu_iova_domain_get(struct rb_root *rbroot,
		struct dma_buf *dmabuf)
{
	struct rb_node *n = NULL;
	struct iova_domain *iova_dom = NULL;
	u64 key = (u64)(uintptr_t)dmabuf;

	for (n = rb_first(rbroot); n; n = rb_next(n)) {
		iova_dom = rb_entry(n, struct iova_domain, node);
		if (iova_dom->key == key)
			return iova_dom;
	}

	return NULL;
}

struct iova_domain *dksm_iommu_get_iova_domain_by_dmabuf(struct dksm_iommu_domain *iommu,
		struct dma_buf *dmabuf)
{
	struct iova_domain *iova_dom = NULL;

	spin_lock(&iommu->iova_lock);
	iova_dom = dksm_iommu_iova_domain_get(&iommu->iova_root, dmabuf);
	if (!iova_dom) {
		spin_unlock(&iommu->iova_lock);
		return NULL;
	}
	atomic64_inc(&iova_dom->ref);
	spin_unlock(&iommu->iova_lock);

	return iova_dom;
}

static struct iova_domain *dksm_iommu_init_iova_domain(struct dma_buf *dmabuf)
{
	struct iova_domain *iova_dom = kzalloc(sizeof(*iova_dom), GFP_KERNEL);

	if (!iova_dom)
		return ERR_PTR(-ENOMEM);

	atomic64_set(&iova_dom->ref, 1);
	iova_dom->key = (u64)(uintptr_t)dmabuf;
	iova_dom->pid = current->pid;

	return iova_dom;
}

static struct sg_table *dksm_iommu_dmabuf_get_sgt(struct device *dev,
		struct dma_buf *dmabuf, struct dma_buf_attachment **attach)
{
	struct sg_table *sgt = NULL;

	*attach = dma_buf_attach(dmabuf, dev);
	if (IS_ERR_OR_NULL(*attach)) {
		DPU_ERROR("failed to attach the dmabuf\n");
		return NULL;
	}

	sgt = dma_buf_map_attachment(*attach, DMA_TO_DEVICE);
	if (IS_ERR_OR_NULL(sgt)) {
		DPU_ERROR("failed to map dma buf to get sgt\n");
		dma_buf_detach(dmabuf, *attach);
		return NULL;
	}

	return sgt;
}

static void dksm_iommu_release_dmabuf_attach(struct dma_buf *dmabuf,
		struct dma_buf_attachment *attach,
		struct sg_table *sgt)
{
	if (attach) {
		dma_buf_unmap_attachment(attach, sgt, DMA_FROM_DEVICE);
		dma_buf_detach(dmabuf, attach);
	}
}

static u32 dksm_iommu_get_sgt_size(struct sg_table *sgt)
{
	struct scatterlist *sg;
	size_t i;
	u32 length = 0;

	for_each_sgtable_sg(sgt, sg, i)
		length += sg->length;

	return length;
}

/**
 * afbc scramble only works on payload.
 *
 * PAGE_SHIFT + BUS_INTERLEAVE_FLAG_BIT = 38
 * mmu Page table 38bit set to 1 to enable afbc scramble
 */
#define dksm_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, \
		afbc_header_page_count, is_addr_inc) \
do { \
	if (!is_scramble_en || afbc_header_page_count > 0) { \
		*tlb_vaddr++ = addr; \
		if (afbc_header_page_count > 0) \
			afbc_header_page_count--; \
	} else { \
		*tlb_vaddr++ = addr | BUS_INTERLEAVE_FLAG_BIT; \
	} \
	addr += is_addr_inc; \
} while (0)

void dksm_iommu_fill_pt_entry(struct dma_buf *dma_buf, struct sg_table *sgt,
		struct iommu_format_info *format_info, struct iova_domain *iova_dom)
{
	u32 page_num_offset[IOMMU_PLANE_MAX + 1] = {0};
	s64 afbc_header_page_count = 0;
	bool is_scramble_en = false;
	u64 last_valid_entry;
	struct scatterlist *sg;
	u32 page_num_i = 0;
	u8 plane_id = 0;
	u32 *tlb_vaddr;
	dma_addr_t addr;
	u32 length;
	size_t i;

	if (format_info == NULL || iova_dom == NULL) {
		DPU_ERROR("format_info/iova_dom param is null!\n");
		return;
	}

	if (dma_buf) {
		afbc_header_page_count = xr_dmabuf_get_headersize(dma_buf);
		if (afbc_header_page_count > 0)  {
			is_scramble_en = true;
		} else {
			is_scramble_en = false;
			if (afbc_header_page_count < 0)
				DPU_ERROR("get afbc header error!");
		}
	}

	iova_dom->plane_num = 1;
	for (i = 1; i < IOMMU_PLANE_MAX && format_info->offsets[i]; i++) {
		iova_dom->iova_offsets[i] = format_info->offsets[i];
		iova_dom->plane_num++;
		page_num_offset[i] = format_info->offsets[i] >> PAGE_SHIFT;
	}
	if (format_info->is_afbc) {
		/**
		 * afbc format plane_num set 2, uesd in tbu hw reg config.
		 * afbc need map on one plane, so page_num_offset[1] set 0.
		 */
		iova_dom->plane_num = 2;
		page_num_offset[1] = 0;
	}

	IOMMU_DEBUG("iommu info: plane_num = %d, is_scramble_en = %d\n",
			iova_dom->plane_num, is_scramble_en);
	for (int i = 0; i < IOMMU_PLANE_MAX; i++)
		IOMMU_DEBUG("\toffset[%d]=0x%x, page_num_offset[%d]=0x%x\n",
				i, iova_dom->iova_offsets[i], i, page_num_offset[i]);

	tlb_vaddr = iova_dom->tlb_vaddr;

	for_each_sgtable_sg(sgt, sg, i) {
		length = sg->length >> PAGE_SHIFT;
		addr = sg_phys(sg) >> PAGE_SHIFT;
		do {
			if (page_num_offset[plane_id + 1] == 0  || page_num_i < page_num_offset[plane_id + 1]) {
				dksm_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 1);
			} else {
				if (iova_dom->iova_offsets[plane_id + 1] & ((1 << PAGE_SHIFT) - 1))
					dksm_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 0);

				last_valid_entry = *(tlb_vaddr - 1);
				while ((unsigned long)tlb_vaddr & 0xf)
					*tlb_vaddr++ = last_valid_entry;

				plane_id++;
				iova_dom->tlb_offsets[plane_id] = (unsigned long)tlb_vaddr - (unsigned long)iova_dom->tlb_vaddr;
				dksm_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 1);
			}
			page_num_i++;
		} while (--length);
	}

	/**
	 * Due to the tlb alignment requirement, fill more mmu entries to
	 * avoid ddrc filter error. We simply use the last valid entry.
	 */
	last_valid_entry = *(tlb_vaddr - 1);
	for (i = 0; i < EXTRA_ENTRY; i++)
		*tlb_vaddr++ = last_valid_entry;
}

int dksm_iommu_create_iova_domain(u32 size, struct iova_domain *iova_dom)
{
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();
	u32 tlb_buf_size;

	if (!iova_dom) {
		DPU_ERROR("iova_dom is null\n");
		return -EINVAL;
	}

	tlb_buf_size = GET_TLB_SIZE(size);
	iova_dom->tlb_vaddr = dksm_mem_pool_alloc(iommu->tlb_mem_pool, tlb_buf_size,
		&iova_dom->tlb_pa);
	if (!iova_dom->tlb_vaddr) {
		DPU_ERROR("fatal error, failed to alloc tlb buf\n");
		return -EINVAL;
	}

	iova_dom->iova = dksm_iommu_alloc_iova(iommu->iova_pool, size, iommu->iova_align);
	if (!iova_dom->iova) {
		iova_dom->iova = IOVA_RESERVED_BASE;
		DPU_INFO("some exception happened, use reserved iova!\n");
	}
	iova_dom->size = size;

	IOMMU_DEBUG("create iova : tlb_vaddr %pK, tlb_pa %#llx, iova %#lx size:%#x tlb_size:%#x\n",
			iova_dom->tlb_vaddr, iova_dom->tlb_pa, iova_dom->iova, size, tlb_buf_size);
	return 0;
}

static int dksm_iommu_do_map_dmabuf_sg(struct dksm_iommu_domain *iommu,
		struct dma_buf *dmabuf,
		struct sg_table *sgt,
		struct iova_domain *iova_dom,
		struct iommu_format_info *format_info)
{
	u32 len = dksm_iommu_get_sgt_size(sgt);
	int ret;

	if (len != dmabuf->size)
		DPU_INFO("dmabuf size(%zu) is not equal to layer size(%u)\n", dmabuf->size, len);

	ret = dksm_iommu_create_iova_domain(len, iova_dom);
	if (ret) {
		DPU_ERROR("create iova domain failed\n");
		return ret;
	}

	dksm_iommu_fill_pt_entry(dmabuf, sgt, format_info, iova_dom);

	return 0;
}

struct iova_domain *dksm_iommu_map_dmabuf(struct dma_buf *dmabuf, struct sg_table *sgt,
		struct iommu_format_info *format_info)
{
	int ret;
	struct iova_domain *iova_dom = NULL;
	struct dma_buf_attachment *attach = NULL;
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();

	if (!dmabuf) {
		DPU_ERROR("dmabuf is null\n");
		return ERR_PTR(-EINVAL);
	}

	iova_dom = dksm_iommu_get_iova_domain_by_dmabuf(iommu, dmabuf);
	if (iova_dom != NULL)
		return iova_dom;

	if (!sgt) {
		sgt = dksm_iommu_dmabuf_get_sgt(iommu->dev, dmabuf, &attach);
		if (!sgt) {
			DPU_ERROR("dksm_iommu_dmabuf_get_sgt fail!\n");
			return ERR_PTR(-ENOMEM);
		}
	}

	iova_dom = dksm_iommu_init_iova_domain(dmabuf);
	if (!iova_dom) {
		DPU_ERROR("init_iova_dom fail!\n");
		goto release_attach;
	}
	iova_dom->dev = iommu->dev;

	ret = dksm_iommu_do_map_dmabuf_sg(iommu, dmabuf, sgt, iova_dom, format_info);
	if (ret) {
		DPU_ERROR("dksm_iommu_do_map_dmabuf_sg fail!\n");
		goto free_iova_dom;
	}

	spin_lock(&iommu->iova_lock);
	dksm_iommu_iova_add(&iommu->iova_root, iova_dom);
	spin_unlock(&iommu->iova_lock);

	dksm_iommu_release_dmabuf_attach(dmabuf, attach, sgt);

	IOMMU_DEBUG("map for dmabuf:0x%llx, size:%#lx\n",
		 (u64)(uintptr_t)dmabuf, dmabuf->size);

	return iova_dom;

free_iova_dom:
	kfree(iova_dom);
release_attach:
	dksm_iommu_release_dmabuf_attach(dmabuf, attach, sgt);
	return NULL;
}

void dksm_iommu_destory_iova_domain(struct iova_domain *iova_dom)
{
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();
	u32 tlb_buf_size;
	int ret;

	if (!iova_dom) {
		DPU_ERROR("iova_dom is null\n");
		return;
	}

	tlb_buf_size = GET_TLB_SIZE(iova_dom->size);
	dksm_mem_pool_free(iommu->tlb_mem_pool, iova_dom->tlb_vaddr, tlb_buf_size);

	IOMMU_DEBUG("destroy: tlb_vaddr %pK, tlb_pa %#llx, iova %#lx size:%#x tlb_size:%#x\n",
			iova_dom->tlb_vaddr, iova_dom->tlb_pa, iova_dom->iova, iova_dom->size, tlb_buf_size);

	ret = gen_pool_has_addr(iommu->iova_pool, iova_dom->iova, iova_dom->size);
	if (!ret) {
		DPU_INFO("illegal param! iova = %lx, size = %x\n",
				iova_dom->iova, iova_dom->size);
		return;
	}

	if (iova_dom->iova != IOVA_RESERVED_BASE)
		dksm_iommu_free_iova(iommu->iova_pool, iova_dom->iova, iova_dom->size);
}

void dksm_iommu_unmap_dmabuf(struct dma_buf *dmabuf)
{
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();
	struct iova_domain *iova_dom = NULL;

	if (!dmabuf)
		return;

	spin_lock(&iommu->iova_lock);
	iova_dom = dksm_iommu_iova_domain_get(&iommu->iova_root, dmabuf);
	if (!iova_dom) {
		spin_unlock(&iommu->iova_lock);
		return;
	}
	IOMMU_DEBUG("unmap for dmabuf:0x%llx, size:%#lx\n",
		 (u64)(uintptr_t)dmabuf, dmabuf->size);

	rb_erase(&iova_dom->node, &iommu->iova_root);
	spin_unlock(&iommu->iova_lock);

	dksm_iommu_destory_iova_domain(iova_dom);
	kfree(iova_dom);
}

static int dksm_iommu_map_iova_buffer(struct file *file, unsigned long arg)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct map_dma_buf buf_data = {0};
	struct dksm_iommu_domain *iommu;
	struct iova_domain *iova_dom;
	struct dma_buf *dma_buf;

	if (!argp) {
		DPU_ERROR("argp is nullptr");
		return -EINVAL;
	}

	iommu = (struct dksm_iommu_domain *)(file->private_data);
	if (IS_ERR_OR_NULL(iommu)) {
		DPU_ERROR("iommu dev is null\n");
		return -EINVAL;
	}

	if (copy_from_user(&buf_data, argp, sizeof(struct map_dma_buf))) {
		DPU_ERROR("iova_info copy_from_user fail");
		return -EINVAL;
	}

	if (unlikely(buf_data.size == 0)) {
		DPU_ERROR("size is 0!");
		return -EINVAL;
	}

	if (unlikely(buf_data.share_fd < 0)) {
		DPU_ERROR("share_fd is less than 0!");
		return -EINVAL;
	}

	dma_buf = dma_buf_get(buf_data.share_fd);
	if (IS_ERR_OR_NULL(dma_buf)) {
		DPU_ERROR("get dma buf fail, ret = %ld sharefd = %d",
				PTR_ERR(dma_buf), buf_data.share_fd);
		return -EINVAL;
	}

	iova_dom = dksm_iommu_map_dmabuf(dma_buf, NULL, NULL);
	if (IS_ERR_OR_NULL(iova_dom)) {
		DPU_ERROR("map failed, sharefd:%d size:%#x dma_buf:0x%llx\n",
				buf_data.share_fd, buf_data.size, (u64)(uintptr_t)dma_buf);
		return PTR_ERR(iova_dom);
	}

	buf_data.tlb_pa = iova_dom->tlb_pa;
	buf_data.iova = iova_dom->iova;

	if (copy_to_user(argp, &buf_data, sizeof(struct map_dma_buf)) != 0) {
		DPU_ERROR("copy_to_user failed");
		dksm_iommu_unmap_dmabuf(dma_buf);
		return -EFAULT;
	}
	dma_buf_put(dma_buf);

	return 0;
}

static int dksm_iommu_unmap_iova_buffer(struct file *file, unsigned long arg)
{
	const void __user *argp = (void __user *)(uintptr_t)arg;
	struct map_dma_buf buf_data = {0};
	struct dksm_iommu_domain *iommu;
	struct dma_buf *dmabuf;

	if (!argp) {
		DPU_ERROR("argp is nullptr\n");
		return -EINVAL;
	}

	iommu = (struct dksm_iommu_domain *)(file->private_data);
	if (IS_ERR_OR_NULL(iommu)) {
		DPU_ERROR("iommu dev is null\n");
		return -EINVAL;
	}

	if (copy_from_user(&buf_data, argp, sizeof(struct map_dma_buf))) {
		DPU_ERROR("copy_from_user failed\n");
		return -EINVAL;
	}

	dmabuf = dma_buf_get(buf_data.share_fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		DPU_ERROR("get dma buf fail, ret = %ld sharefd = %d\n",
				PTR_ERR(dmabuf), buf_data.share_fd);
		return -EINVAL;
	}

	dksm_iommu_unmap_dmabuf(dmabuf);

	dma_buf_put(dmabuf);
	return 0;
}

static long dksm_iommu_dev_ioctl(struct file *file, u32 cmd, unsigned long argp)
{
	switch (cmd) {
	case IOMMU_MAP_IOVA:
		return dksm_iommu_map_iova_buffer(file, argp);
	case IOMMU_UNMAP_IOVA:
		return dksm_iommu_unmap_iova_buffer(file, argp);
	default:
		return -EINVAL;
	}

	return 0;
}

static int dksm_iommu_open(struct inode *inode, struct file *file)
{
	struct dksm_iommu_domain *iommu;

	if (!inode || !file)
		return -EINVAL;

	file->private_data = dksm_iommu_get_instance();
	iommu = (struct dksm_iommu_domain *)(file->private_data);
	if (IS_ERR_OR_NULL(iommu)) {
		DPU_ERROR("iommu dev is null\n");
		return -EINVAL;
	}

	atomic_inc(&iommu->refcnt);
	DPU_INFO("iommu dev refcnt:%u\n", atomic_read(&iommu->refcnt));
	return 0;
}

static int dksm_iommu_release(struct inode *inode, struct file *file)
{
	struct dksm_iommu_domain *iommu;

	if (!inode || !file)
		return -EINVAL;

	iommu = (struct dksm_iommu_domain *)(file->private_data);
	if (IS_ERR_OR_NULL(iommu)) {
		DPU_ERROR("iommu dev is null\n");
		return -EINVAL;
	}

	if (atomic_read(&iommu->refcnt) == 0) {
		DPU_INFO("iommu dev is not opened, cannot release\n");
		return 0;
	}

	atomic_sub(1, &iommu->refcnt);
	DPU_INFO("iommu dev refcnt:%u\n", atomic_read(&iommu->refcnt));

	return 0;
}

static const struct file_operations dksm_iommu_fops = {
	.owner = THIS_MODULE,
	.open = dksm_iommu_open,
	.release = dksm_iommu_release,
	.compat_ioctl = dksm_iommu_dev_ioctl,
	.unlocked_ioctl = dksm_iommu_dev_ioctl,
};

static int dksm_iommu_dev_init(struct dksm_iommu_domain *iommu)
{
	struct dksm_chrdev *chrdev;
	int ret;

	chrdev = &iommu->chrdev;
	chrdev->name = "dpu_iommu_dev";
	chrdev->fops = &dksm_iommu_fops;
	chrdev->drv_data = iommu;
	ret = dksm_create_chrdev(chrdev);
	if (ret) {
		DPU_ERROR("create wb dev failed\n");
		return ret;
	}

	return 0;
}

static void dkms_iommu_dmabuf_unmap_hook_func(void *data, struct dma_buf *dmabuf)
{
	dksm_iommu_unmap_dmabuf(dmabuf);
}

struct __find_tracepoint_cb_data {
	const char *tp_name;
	struct tracepoint *tpoint;
};

static void __find_tracepoint_cb(struct tracepoint *tp, void *priv)
{
	struct __find_tracepoint_cb_data *data = priv;

	if (!data->tpoint && !strcmp(data->tp_name, tp->name))
		data->tpoint = tp;
}

static struct tracepoint *find_tracepoint(const char *tp_name)
{
	struct __find_tracepoint_cb_data data = {
		.tp_name = tp_name,
	};

	for_each_kernel_tracepoint(__find_tracepoint_cb, &data);

	return data.tpoint;
}

int dksm_iommu_context_init(struct device *dev, struct dksm_mem_pool *mem_pool)
{
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();
	int ret;

	if (iommu->inited) {
		DPU_INFO("iommu context has already initialized!\n");
		return 0;
	}

	iommu->dev = dev;
	iommu->tlb_mem_pool = mem_pool;
	spin_lock_init(&iommu->iova_lock);
	iommu->iova_root = RB_ROOT;

	iommu->iova_start = IOVA_START_ADDR;
	iommu->iova_size = IOVA_POOL_SIZE;
	iommu->iova_align = IOVE_ALIGN;
	iommu->iova_pool = dksm_iommu_iova_pool_setup(iommu->iova_start, iommu->iova_size, iommu->iova_align);
	if (!iommu->iova_pool) {
		DPU_ERROR("create iova_pool failed\n");
		return PTR_ERR(iommu->iova_pool);
	}

	ret = dksm_iommu_dev_init(iommu);
	if (ret) {
		DPU_ERROR("iommu dev init failed\n");
		return ret;
	}

	atomic_set(&iommu->refcnt, 0);
	tracepoint_probe_register(
		find_tracepoint("android_vh_dma_buf_release"),
		dkms_iommu_dmabuf_unmap_hook_func,
		NULL);

	iommu->inited = true;
	return 0;
}

void dksm_iommu_context_deinit(void)
{
	struct dksm_iommu_domain *iommu = dksm_iommu_get_instance();

	if (!iommu->inited)
		return;

	tracepoint_probe_unregister(
		find_tracepoint("android_vh_dma_buf_release"),
		dkms_iommu_dmabuf_unmap_hook_func,
		NULL);

	dksm_destroy_chrdev(&iommu->chrdev);
	gen_pool_destroy(iommu->iova_pool);
}
