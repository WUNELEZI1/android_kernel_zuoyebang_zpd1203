// SPDX-License-Identifier: GPL-2.0-or-later
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
#include <linux/module.h>
#include <linux/genalloc.h>
#include <linux/iommu.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include "hpc_internal.h"
#include "soc/xring/xring_smmu_wrapper.h"
#include <soc/xring/ocm_wrapper.h>
#if KERNEL_VERSION_GREATER_THAN_6_6_0
MODULE_IMPORT_NS(DMA_BUF);
#endif

#define HDEV_MEM_HEAP_ALLOC \
	_IOR('X', 0, s32)

struct hpc_mem_heap_info {
	int id;
	struct list_head node;
};
struct hpc_mem_heap_dev {
	struct list_head hmp;
	struct mutex lock;
};

struct hpc_mem_heap_ctrl g_mem_ctrl;

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int hpc_log_level = HPC_LOG_LEVEL_INFO;
module_param(hpc_log_level, int, 0644);
MODULE_PARM_DESC(hpc_log_level, "Control log level (0=none, 1=info, ...)");
#endif


static int hpc_mem_heap_check_valid(struct hpc_mem_heap *heap)
{
	struct hpc_mem_heap *hp = NULL;
	int ret = -EINVAL;
	int id;

	if (heap && (!heap->dynamic))
		return 0;

	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	idr_for_each_entry(&g_mem_ctrl.idr_head, hp, id) {
		if (heap != hp)
			continue;

		ret = 0;

		goto out;
	}

out:
	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

	return ret;
}

int hpc_dma_buf_map_vaddr(struct hpc_dma_buf *buf)
{
	int ret;

	ret = dma_buf_vmap(buf->dmabuf, &buf->map);
	if (ret < 0)
		hpcerr("dma buf vmap failed: %d\n", ret);

	return ret;
}
EXPORT_SYMBOL_GPL(hpc_dma_buf_map_vaddr);

void hpc_dma_buf_unmap_vaddr(struct hpc_dma_buf *buf)
{
#if KERNEL_VERSION_LESS_THAN_6_6_0
	struct dma_buf_map *map = &buf->map;
#else
	struct iosys_map *map = &buf->map;
#endif
	if (map->vaddr) {
		dma_buf_vunmap(buf->dmabuf, map);
		map->vaddr = NULL;
	}
}
EXPORT_SYMBOL_GPL(hpc_dma_buf_unmap_vaddr);

struct hpc_dma_buf *hpc_dma_buf_fill(struct device *dev, int fd, size_t size)
{
	struct hpc_dma_buf *buf;
	void *ret = NULL;

	hpcdbg("entry\n");
	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf) {
		hpcerr("zalloc hpc dma buf failed, fd %d\n", fd);
		return NULL;
	}

	buf->fd = fd;
	buf->size = size;
	buf->dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(buf->dmabuf)) {
		hpcerr("dma buf get failed, fd %d\n", fd);
		goto err_put_zalloc;
	}

	if (buf->dmabuf->size < ALIGN(size, PAGE_SIZE)) {
		hpcerr("dma buf size 0x%lx less then size 0x%lx", buf->dmabuf->size, size);
		goto err_put_dma_buf;
	}

	buf->attach = dma_buf_attach(buf->dmabuf, dev);
	if (IS_ERR_OR_NULL(buf->attach)) {
		hpcerr("dma buf attach failed, fd %d\n", fd);
		ret = HEAP_IS_NOT_ENOUGH;
		goto err_put_dma_buf;

	}

	buf->sgt = dma_buf_map_attachment(buf->attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(buf->sgt)) {
		hpcerr("dma buf map attachment failed, fd %d\n", fd);
		ret = HEAP_IS_NOT_ENOUGH;
		goto err_put_dma_attach;
	}
	hpcdbg("end\n");
	return buf;

err_put_dma_attach:
	dma_buf_detach(buf->dmabuf, buf->attach);
err_put_dma_buf:
	dma_buf_put(buf->dmabuf);
err_put_zalloc:
	kfree(buf);

	return ret;
}

int hpc_dma_buf_clean(struct hpc_dma_buf *buf)
{
	if (!buf->attach || !buf->sgt) {
		hpcerr("no attachmemt or no buf, fd %d\n", buf->fd);
		return -EINVAL;
	}
	dma_buf_unmap_attachment(buf->attach, buf->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf->dmabuf, buf->attach);
	dma_buf_put(buf->dmabuf);
	kfree(buf);

	return 0;
}
EXPORT_SYMBOL_GPL(hpc_dma_buf_clean);

void hpc_mem_heap_check_and_free(void)
{
	struct hpc_mem_heap *heap = NULL;
	int id;

	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	idr_for_each_entry(&g_mem_ctrl.idr_head, heap, id) {
		if (!heap->ref)
			continue;

		if (!heap->map_ref) {
			heap->ref = 0;
			heap->pid = 0;
			hpcdbg("free heap [%pK]", (void *)heap);
		}
	}

	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);
}
EXPORT_SYMBOL_GPL(hpc_mem_heap_check_and_free);

static int hpc_mem_heap_alloc(void)
{
	struct hpc_mem_heap *heap = NULL;
	int id = 0;

	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	idr_for_each_entry(&g_mem_ctrl.idr_head, heap, id) {
		if (heap->ref) {
			hpcdbg("pid[%d] occupies the current heap", heap->pid);
			continue;
		}

		heap->ref = 1;
		heap->pid = current->pid;
		goto out;
	}

	hpcerr("no left heap\n");
	id = -1;

out:
	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

	return id;
}

static int hpc_mem_heap_free(int id)
{
	struct hpc_mem_heap *heap = NULL;

	heap = idr_find(&g_mem_ctrl.idr_head, id);
	if (heap == NULL) {
		hpcerr("id: %d without correspond heap\n", id);
		goto out;
	}
	if (!heap->ref) {
		hpcerr("heap has been freed\n");
		goto out;
	}
	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	heap->ref = 0;
	heap->pid = 0;
	hpcdbg("heap is not exist\n");
	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

out:
	return 0;
}

static int hpc_mem_map(struct hpc_mem_heap *heap, unsigned long *iova, phys_addr_t paddr,
		size_t size, int prot)
{
	int ret = 0;

	ret = hpc_mem_heap_check_valid(heap);
	if (ret != 0)
		return ret;

	mutex_lock(&heap->mutex);
	*iova = gen_pool_alloc(heap->pool, size);
	if (*iova == 0) {
		hpcerr("gen pool alloc iova failed\n");
		ret = -ENOMEM;
		goto out;
	}
#if KERNEL_VERSION_LESS_THAN_6_6_0
	ret = iommu_map(heap->domain, *iova, paddr, size, prot);
#else
	ret = iommu_map(heap->domain, *iova, paddr, size, prot, GFP_KERNEL);
#endif
	if (ret != 0) {
		hpcerr("iommu map failed: %d\n", ret);
		gen_pool_free(heap->pool, *iova, size);
		goto out;
	}

	heap->map_ref++;

out:
	mutex_unlock(&heap->mutex);

	return ret;
}

int hpc_mem_unmap(struct hpc_mem_heap *heap, unsigned long iova, size_t size)
{

	size_t unmapped_size;
	int ret;

	ret = hpc_mem_heap_check_valid(heap);
	if (ret != 0)
		return ret;

	mutex_lock(&heap->mutex);
	unmapped_size = iommu_unmap(heap->domain, iova, size);
	if (unmapped_size != size) {
		hpcerr("iommu unmap failed\n");
		goto out;
	}

	gen_pool_free(heap->pool, iova, size);
	heap->map_ref--;
	mutex_unlock(&heap->mutex);
	return 0;

out:
	mutex_unlock(&heap->mutex);

	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(hpc_mem_unmap);

static int hpc_mem_map_iova(struct hpc_mem_heap *heap, struct hpc_dma_buf *buf,
		unsigned long *iova, int prot, bool flagadd4k)
{
	size_t size = 0;
	int ret = 0;

	ret = hpc_mem_heap_check_valid(heap);
	if (ret != 0)
		return ret;

	if (buf->dmabuf->size < ALIGN(buf->size, PAGE_SIZE)) {
		hpcerr("dma buf size 0x%lx less then size 0x%lx", buf->dmabuf->size, buf->size);
		return -EINVAL;
	}

	mutex_lock(&heap->mutex);
	*iova = gen_pool_alloc(heap->pool, buf->size + (flagadd4k ? PAGE_SIZE : 0));
	if (*iova == 0) {
		hpcerr("gen pool alloc iova failed\n");
		ret = -ENOMEM;
		goto out;
	}

	size = iommu_map_sgtable(heap->domain, *iova, buf->sgt, prot);
	if (size > buf->dmabuf->size) {
		hpcerr("iommu map sgtable failed, map size 0x%lx, sgt size 0x%lx\n", size, buf->size);
		gen_pool_free(heap->pool, *iova, buf->size);
		ret = -ENOMEM;
	}

	heap->map_ref++;

out:
	mutex_unlock(&heap->mutex);

	return ret;
}

static int hpc_mem_unmap_iova(struct hpc_mem_heap *heap, struct hpc_dma_buf *buf,
		unsigned long iova, bool flagadd4k)
{
	int ret = 0;
	size_t size = 0;

	ret = hpc_mem_heap_check_valid(heap);
	if (ret != 0)
		return ret;

	if (buf->dmabuf->size < ALIGN(buf->size, PAGE_SIZE)) {
		hpcerr("dma buf size 0x%lx less then size 0x%lx", buf->dmabuf->size, buf->size);
		return -EINVAL;
	}

	mutex_lock(&heap->mutex);
	size = iommu_unmap(heap->domain, iova, buf->dmabuf->size);
	if (size > buf->dmabuf->size) {
		hpcerr("iommu unmap failed, map size 0x%lx, sgt size 0x%lx\n", size, buf->size);
		ret = -ENOMEM;
		goto out;
	}
	if (flagadd4k) {
		size = iommu_unmap(heap->domain, iova+buf->dmabuf->size, PAGE_SIZE);
		if (size > PAGE_SIZE) {
			hpcerr("iommu unmap failed, map size 0x%lx, sgt size 0x%lx\n", size, PAGE_SIZE);
			ret = -ENOMEM;
			goto out;
		}
	}
	gen_pool_free(heap->pool, iova, buf->size + (flagadd4k ? PAGE_SIZE : 0));
	heap->map_ref--;

out:
	mutex_unlock(&heap->mutex);

	return ret;
}

int hpc_mem_map_al_iova(u64 pa, size_t size, unsigned long iova, int prot)
{
	struct hpc_mem_heap *heap = NULL;
	int ret = 0, id;
	size_t sz;

	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	idr_for_each_entry(&g_mem_ctrl.idr_head, heap, id) {
#if KERNEL_VERSION_LESS_THAN_6_6_0
		ret = iommu_map(heap->domain, iova, pa, size, prot);
#else
		ret = iommu_map(heap->domain, iova, pa, size, prot, GFP_KERNEL);
#endif
		if (ret != 0) {
			hpcerr("iommu map al iova failed, heap name %s\n",
					dev_name(&heap->pdev->dev));
			goto out;
		}

		heap->al_ref = 1;
	}

	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

	return 0;

out:
	idr_for_each_entry(&g_mem_ctrl.idr_head, heap, id) {
		if (!heap->al_ref)
			continue;

		sz = iommu_unmap(heap->domain, iova, size);
		if (sz != size) {
			hpcerr("failed, unmap size 0x%lx, size 0x%lx, %s\n",
					sz, size, dev_name(&heap->pdev->dev));
		}

		heap->al_ref = 0;
	}

	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(hpc_mem_map_al_iova);

int hpc_mem_unmap_al_iova(unsigned long iova, size_t size)
{
	struct hpc_mem_heap *heap = NULL;
	int ret = 0, id;
	size_t sz;

	mutex_lock(&g_mem_ctrl.dyn_heap_lock);
	idr_for_each_entry(&g_mem_ctrl.idr_head, heap, id) {
		if (!heap->al_ref)
			continue;

		sz = iommu_unmap(heap->domain, iova, size);
		if (sz != size) {
			hpcerr("failed, unmapped size 0x%lx, size 0x%lx, %s\n",
					sz, size, dev_name(&heap->pdev->dev));
		}

		heap->al_ref = 0;
	}

	mutex_unlock(&g_mem_ctrl.dyn_heap_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(hpc_mem_unmap_al_iova);

int hpc_dma_alloc(struct hpc_device *hdev)
{
	void *va;
	dma_addr_t dma_handle;
	size_t size = PAGE_SIZE;
	gfp_t gfp = GFP_KERNEL;
	struct device *dev = &hdev->pdev->dev;

	va = dma_alloc_coherent(dev, size, &dma_handle, gfp);
	if (!va) {
		hpcerr("Failed to allocate coherent DMA memory\n");
		return -ENOMEM;
	}

	hdev->dma_handle = dma_handle;
	hdev->dma_va = va;

	return 0;
}
EXPORT_SYMBOL_GPL(hpc_dma_alloc);

void hpc_dma_free(struct hpc_device *hdev)
{
	size_t size = PAGE_SIZE;
	struct device *dev = &hdev->pdev->dev;

	if (hdev->dma_va) {
		dma_free_coherent(dev, size, hdev->dma_va, hdev->dma_handle);
		hdev->dma_va = NULL;
		hdev->dma_handle = 0;
	}
}
EXPORT_SYMBOL_GPL(hpc_dma_free);

int hrpdev_dma_buf_map(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem)
{
	struct hpc_mem_heap *heap = platform_get_drvdata(hdev->hmp_pdev);
	struct hpc_mem_heap *vip_heap = NULL;
	int ret;
	unsigned long mda, vda;

	vip_heap = idr_find(&g_mem_ctrl.idr_head, mem->info.idr);
	if (vip_heap == NULL) {
		hpcerr("heap info id: %d without correspond heap\n", mem->info.idr);
		return -EINVAL;
	}
	mem->buf = hpc_dma_buf_fill(&hdev->pdev->dev, mem->info.fd, mem->info.size);
	if (!mem->buf) {
		hpcerr("hdev dma buf fill failed\n");
		return -ENOMEM;
	}

	if (mem->buf == HEAP_IS_NOT_ENOUGH) {
		hpcerr("heap is not enough\n");
		return -ENOMEM;
	}

	if (mem->info.vip.flag) {
		ret = hpc_mem_map_iova(vip_heap, mem->buf,
			&vda, mem->info.vip.prot, mem->info.vip.flag4k);
		if (ret != 0) {
			hpcerr("map vip iova failed\n");
			goto put_map_vip_iova;
		}
		mem->info.vip.da = (u32)vda;
		if (mem->info.vip.flag4k) {
#if KERNEL_VERSION_LESS_THAN_6_6_0
			ret = iommu_map(vip_heap->domain, mem->info.vip.da + mem->buf->dmabuf->size,
				hdev->dma_handle, PAGE_SIZE, IOMMU_READ | IOMMU_WRITE);
#else
			ret = iommu_map(vip_heap->domain, mem->info.vip.da + mem->buf->dmabuf->size,
				hdev->dma_handle, PAGE_SIZE, IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#endif
			if (ret != 0) {
				hpcerr("iommu map failed: %d\n", ret);
				goto put_map_flag4k_iova;
			}
		}
	}

	mem->info.vip.sid = vip_heap->sid;
	mem->info.vip.ssid = vip_heap->ssid;
	hpcdbg("vip fd %d, da %pK, size 0x%x, sid %d, ssid %d\n",
				mem->buf->fd, (void *)(long)mem->info.vip.da, mem->info.size,
				mem->info.vip.sid, mem->info.vip.ssid);

	if (mem->info.mcu.flag) {
		ret = hpc_mem_map_iova(heap, mem->buf,
				&mda, mem->info.mcu.prot, false);
		if (ret != 0) {
			hpcerr("map mcu iova failed\n");
			goto put_map_mcu_iova;
		}
		mem->info.mcu.da = (u32)mda;
		mem->info.mcu.sid = heap->sid;
		mem->info.mcu.ssid = heap->ssid;
		hpcdbg("mcu fd %d, da %pK, size 0x%x, sid %d, ssid %d\n",
				mem->buf->fd, (void *)(long)mem->info.mcu.da, mem->info.size,
				mem->info.mcu.sid, mem->info.mcu.ssid);
	}

	hpcdbg("inc heap[%pK]  map_ref[%d]", (void *)vip_heap, vip_heap->map_ref);

	return 0;

put_map_mcu_iova:
	iommu_unmap(vip_heap->domain, mem->info.vip.da + mem->buf->size, PAGE_SIZE);

put_map_flag4k_iova:
	(void)hpc_mem_unmap_iova(vip_heap, mem->buf,
				(unsigned long)mem->info.vip.da, false);

put_map_vip_iova:
	(void)hpc_dma_buf_clean(mem->buf);

	return ret;
}
EXPORT_SYMBOL_GPL(hrpdev_dma_buf_map);

int hrpdev_dma_buf_unmap(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem)
{
	struct hpc_mem_heap *heap = platform_get_drvdata(hdev->hmp_pdev);
	struct hpc_mem_heap *vip_heap = NULL;
	int ret;

	vip_heap = idr_find(&g_mem_ctrl.idr_head, mem->info.idr);
	if (vip_heap == NULL) {
		hpcerr("heap info id: %d without correspond heap\n", mem->info.idr);
		return -EINVAL;
	}
	if (mem->info.mcu.flag && mem->info.mcu.da) {
		hpcdbg("mcu fd %d, da %pK, size 0x%x, sid %d, ssid %d\n",
				mem->buf->fd, (void *)(long)mem->info.mcu.da, mem->info.size,
				mem->info.mcu.sid, mem->info.mcu.ssid);
		ret = hpc_mem_unmap_iova(heap, mem->buf, mem->info.mcu.da, false);
		if (ret != 0) {
			hpcerr("unmap mcu iova failed: %d\n", ret);
			return ret;
		}
		mem->info.mcu.da = 0;
	}
	if (mem->info.vip.flag && mem->info.vip.da) {
		hpcdbg("vip fd %d, da %pK, size 0x%x, sid %d, ssid %d\n",
					mem->buf->fd, (void *)(long)mem->info.vip.da, mem->info.size,
					mem->info.vip.sid, mem->info.vip.ssid);
		ret = hpc_mem_unmap_iova(vip_heap, mem->buf, mem->info.vip.da, mem->info.vip.flag4k);
		if (ret != 0) {
			hpcerr("unmap vip iova failed: %d\n", ret);
			return ret;
		}
		mem->info.vip.da = 0;
	}

	ret = hpc_dma_buf_clean(mem->buf);
	if (ret != 0) {
		hpcerr("hdev dma buf clean failed\n");
		return ret;
	}

	hpcdbg("dec heap[%pK]  map_ref[%d]", (void *)vip_heap, vip_heap->map_ref);

	return 0;
}
EXPORT_SYMBOL_GPL(hrpdev_dma_buf_unmap);

int hrpdev_ocm_buf_map(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem)
{
	int ret;
	struct hpc_mem_heap *heap = NULL;
	unsigned long vda;

	heap = idr_find(&g_mem_ctrl.idr_head, mem->info.idr);
	if (heap == NULL) {
		hpcerr("heap info id: %d without correspond heap\n", mem->info.idr);
		return -EINVAL;
	}

	mem->ob = ocm_buf_get(mem->info.fd);
	if (!mem->ob) {
		hpcerr("hdev ocm buf get failed\n");
		return -ENOMEM;
	}

	ret = ocm_buf_set_attr(mem->ob, XRING_NPU);
	if (ret != 0) {
		hpcerr("hpc set ocm buf attr failed: %d\n", ret);
		return ret;
	}

	ret = hpc_mem_map(heap, &vda, mem->ob->addr,
		mem->ob->size, mem->info.vip.prot);
	if (ret != 0) {
		hpcerr("hpc map ocm buffer failed: %d\n", ret);
		(void)ocm_buf_clr_attr(mem->ob, XRING_NPU);
		return ret;
	}
	mem->info.vip.da = (u32)vda;

	hpcdbg("ocm fd is %d, da is 0x%x, sid is %d, ssid is %d\n",
			mem->info.fd, mem->info.vip.da, mem->info.vip.sid, mem->info.vip.ssid);

	return 0;
}
EXPORT_SYMBOL_GPL(hrpdev_ocm_buf_map);

int hrpdev_ocm_buf_unmap(struct hpc_device *hdev, struct hpc_rpmsg_mem_node *mem)
{
	int ret;
	struct hpc_mem_heap *heap = NULL;

	heap = idr_find(&g_mem_ctrl.idr_head, mem->info.idr);
	if (heap == NULL) {
		hpcerr("heap info id: %d without correspond heap\n", mem->info.idr);
		return -EFAULT;
	}

	ret = hpc_mem_unmap(heap, mem->info.vip.da, mem->ob->size);
	if (ret != 0) {
		hpcerr("hpc unmap ocm buffer failed: %d\n", ret);
		return ret;
	}

	(void)ocm_buf_put(mem->info.fd);
	return 0;
}
EXPORT_SYMBOL_GPL(hrpdev_ocm_buf_unmap);

static atomic_t npu_if_exception = ATOMIC_INIT(0);

void set_npu_excp_true(void)
{
	atomic_set(&npu_if_exception, 1);
}
EXPORT_SYMBOL_GPL(set_npu_excp_true);

void set_npu_excp_false(void)
{
	atomic_set(&npu_if_exception, 0);
}
EXPORT_SYMBOL_GPL(set_npu_excp_false);

bool is_npu_excp_true(void)
{
	return atomic_read(&npu_if_exception) != 0;
}
EXPORT_SYMBOL_GPL(is_npu_excp_true);

static int hpc_mem_of_parse_node(struct hpc_mem_heap *hmp, struct device *dev)
{
	struct device_node *np = dev->of_node;
	int ret;

	ret = of_property_read_u32(np, "iova-start", &hmp->region.iova);
	if (ret < 0) {
		hpcerr("%s get iova failed: %d\n", dev_name(dev), ret);
		return ret;
	}

	ret = of_property_read_u32(np, "iova-len", (u32 *)&hmp->region.size);
	if (ret < 0) {
		hpcerr("%s get iova len failed: %d\n", dev_name(dev), ret);
		return ret;
	}

	ret = of_property_read_u32(np, "sid", &hmp->sid);
	if (ret < 0) {
		hpcerr("%s get sid failed: %d\n", dev_name(dev), ret);
		return ret;
	}

	ret = of_property_read_u32(np, "ssid", &hmp->ssid);
	if (ret < 0) {
		hpcerr("%s get ssid failed: %d\n", dev_name(dev), ret);
		return ret;
	}

	ret = of_property_read_u32(np, "dynamic", &hmp->dynamic);
	if (ret < 0) {
		hpcerr("%s get dynamic flag failed: %d\n", dev_name(dev), ret);
		return ret;
	}

	return ret;
}

static int hpc_mem_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hpc_mem_heap *hmp = NULL;
	int ret;
	int fd;

	hpcdbg("%s hpc mem probe!\n", dev_name(dev));
	hmp = devm_kzalloc(&pdev->dev, sizeof(*hmp), GFP_KERNEL);
	if (!hmp) {
		hpcerr("%s zalloc failed\n", dev_name(dev));
		return -ENOMEM;
	}

	ret = hpc_mem_of_parse_node(hmp, dev);
	if (ret != 0)
		return ret;
	hmp->pool = gen_pool_create(12, -1);
	if (!hmp->pool) {
		hpcerr("%s gen pool create failed: %d\n", dev_name(dev), ret);
		return ret;
	}
	ret = gen_pool_add(hmp->pool, hmp->region.iova, hmp->region.size, -1);
	if (ret != 0) {
		hpcerr("%s gen pool add failed: %d\n", dev_name(dev), ret);
		gen_pool_destroy(hmp->pool);
		return ret;
	}

	hmp->domain = iommu_get_domain_for_dev(dev);
	if (!hmp->domain) {
		hpcerr("%s get iommu domain failed\n", dev_name(dev));
		gen_pool_destroy(hmp->pool);
		return -ENODEV;
	}

	if (hmp->dynamic) {
		mutex_lock(&g_mem_ctrl.dyn_heap_lock);
		fd = idr_alloc(&g_mem_ctrl.idr_head, hmp, 1, 8, GFP_KERNEL);
		if (fd < 0)
			hpcerr("idr alloc failed, heap->fd: %d\n", fd);
		hpcdbg("idr alloc success, heap->fd: %d\n", fd);
		mutex_unlock(&g_mem_ctrl.dyn_heap_lock);
	}

	mutex_init(&hmp->mutex);
	hmp->pdev = pdev;
	platform_set_drvdata(pdev, hmp);
	hpcdbg("%s hpc mem probe end!\n", dev_name(dev));

	return 0;
}

static int hpc_mem_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hpc_mem_heap *hmp = platform_get_drvdata(pdev);

	hpcdbg("%s entry\n", dev_name(dev));
	mutex_destroy(&hmp->mutex);
	if (hmp->dynamic)
		list_del(&hmp->node);
	gen_pool_destroy(hmp->pool);
	hpcdbg("%s end\n", dev_name(dev));

	return 0;
}

static const struct of_device_id hpc_mem_id_table[] = {
	{ .compatible = "xring,hpc_heap0" },
	{ .compatible = "xring,hpc_heap1" },
	{ .compatible = "xring,hpc_heap2" },
	{ .compatible = "xring,hpc_heap3" },
	{ .compatible = "xring,hpc_heap4" },
	{ .compatible = "xring,hpc_heap5" },
	{ .compatible = "xring,hpc_heap6" },
	{ .compatible = "xring,hpc_heap7" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_mem_id_table);

static struct platform_driver hpc_mem_driver = {
	.probe = hpc_mem_probe,
	.remove = hpc_mem_remove,
	.driver = {
		.name = "hpc-mem-dev",
		.of_match_table = of_match_ptr(hpc_mem_id_table),
	},
};

static int hpc_mem_ctrl_heap_free_all(struct hpc_mem_heap_ctrl *mem_ctrl,
		struct hpc_mem_heap_dev *hmem_dev)
{
	int ret = 0;
	struct hpc_mem_heap_info *heap_info = NULL, *tmp = NULL;

	hpcdbg("entry\n");
	list_for_each_entry_safe(heap_info, tmp, &hmem_dev->hmp, node) {
		hpcdbg("found heap node id: %d\n", heap_info->id);
		ret = hpc_mem_heap_free(heap_info->id);
		if (ret != 0) {
			hpcerr("hpc heap free failed: %d\n", ret);
			continue;
		}
		mutex_lock(&hmem_dev->lock);
		list_del(&heap_info->node);
		kfree(heap_info);
		mutex_unlock(&hmem_dev->lock);
	}

	hpcdbg("end\n");
	return ret;
}

static int hpc_mem_ctrl_heap_alloc(struct hpc_mem_heap_ctrl *mem_ctrl,
		struct hpc_mem_heap_dev *hmem_dev, void *argp)
{
	struct hpc_mem_heap_info *heap_info = NULL;
	int ret = 0;
	int id = 0;

	heap_info = kzalloc(sizeof(*heap_info), GFP_KERNEL);
	if (!heap_info) {
		hpcerr("malloc device heap info node failed\n");
		ret = -ENOMEM;
		goto out;
	}

	id = hpc_mem_heap_alloc();
	if (id < 0) {
		hpcerr("hpc heap alloc failed\n");
		ret = -ENOMEM;
		goto err_idr;
	}

	ret = copy_to_user(argp, &id, sizeof(s32));
	if (ret != 0) {
		hpcerr("copy_to_user failed: %d\n", ret);
		goto err_copy;
	}

	heap_info->id = id;
	hpcdbg("alloc heap success, heap->fd: %d\n", id);
	mutex_lock(&hmem_dev->lock);
	list_add_tail(&heap_info->node, &hmem_dev->hmp);
	mutex_unlock(&hmem_dev->lock);

	goto out;
err_copy:
	hpc_mem_heap_free(id);
err_idr:
	kfree(heap_info);
out:
	return ret;

}

static long hpc_mem_ctrl_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	struct hpc_mem_heap_dev *hmem_dev;
	int ret = 0;
	void __user *argp = (void __user *)arg;

	hpcdbg("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmem_dev = filp->private_data;

	switch (cmd) {
	case HDEV_MEM_HEAP_ALLOC:
		ret = hpc_mem_ctrl_heap_alloc(&g_mem_ctrl, hmem_dev, argp);
		break;
	default:
		hpcerr("unsupported type\n");
		ret = -EINVAL;
	}
	hpcdbg("end\n");

	return ret;
}


static int hpc_mem_ctrl_open(struct inode *inode, struct file *filp)
{
	struct hpc_mem_heap_dev *hmem_dev;
	int ret = 0;

	hpcinfo("entry\n");
	hmem_dev = devm_kzalloc(&g_mem_ctrl.pdev->dev, sizeof(*hmem_dev), GFP_KERNEL);
	if (!hmem_dev) {
		hpcerr("hpc mem dev zalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}
	filp->private_data = hmem_dev;
	INIT_LIST_HEAD(&hmem_dev->hmp);
	mutex_init(&hmem_dev->lock);

	hpcinfo("end\n");
err:
	return ret;
}

static int hpc_mem_ctrl_release(struct inode *inode, struct file *filp)
{
	struct hpc_mem_heap_dev *hmem_dev;
	int ret = 0;

	hpcinfo("entry\n");
	if (!filp->private_data) {
		hpcerr("filp->private_data is null");
		return -EPERM;
	}
	hmem_dev = filp->private_data;
	ret = hpc_mem_ctrl_heap_free_all(&g_mem_ctrl, hmem_dev);
	if (ret != 0)
		hpcerr("heap free all failed: %d\n", ret);
	devm_kfree(&g_mem_ctrl.pdev->dev, hmem_dev);

	return ret;
}

static const struct file_operations hpc_mem_ctrl_fops = {
	.owner = THIS_MODULE,
	.open = hpc_mem_ctrl_open,
	.release = hpc_mem_ctrl_release,
	.unlocked_ioctl = hpc_mem_ctrl_ioctl,
};

static int hpc_mem_ctrl_probe(struct platform_device *pdev)
{
	struct miscdevice *miscdev = NULL;
	int ret = 0;

	hpcdbg("entry\n");
	g_mem_ctrl.pdev = pdev;
	mutex_init(&g_mem_ctrl.dyn_heap_lock);
	idr_init(&g_mem_ctrl.idr_head);
	ret = platform_driver_register(&hpc_mem_driver);
	if (ret) {
		hpcerr("regist mem heap driver failed");
		goto err;
	}
	ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (ret) {
		hpcerr("%s probe child device failed: %d\n", dev_name(&pdev->dev), ret);
		ret = -EINVAL;
		goto err;
	}
	miscdev = &g_mem_ctrl.miscdev;
	miscdev->parent = &pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "hpc-heap";
	miscdev->fops = &hpc_mem_ctrl_fops;
	ret = misc_register(miscdev);
	if (ret != 0) {
		hpcerr("register hpc cdev failed: %d\n", ret);
		ret = -EINVAL;
		goto err;
	}
	platform_set_drvdata(pdev, &g_mem_ctrl);

	hpcdbg("end\n");
	goto out;
err:
	idr_destroy(&g_mem_ctrl.idr_head);
	mutex_destroy(&g_mem_ctrl.dyn_heap_lock);
out:
	return ret;
}

static int hpc_mem_ctrl_remove(struct platform_device *pdev)
{
	struct hpc_mem_heap_ctrl *mem_ctrl = NULL;
	int ret = 0;

	mem_ctrl = platform_get_drvdata(pdev);
	if (mem_ctrl == NULL) {
		hpcerr("get mem ctrl error\n");
		return -EINVAL;
	}
	mutex_destroy(&mem_ctrl->dyn_heap_lock);
	idr_destroy(&mem_ctrl->idr_head);
	misc_deregister(&mem_ctrl->miscdev);
	platform_driver_unregister(&hpc_mem_driver);

	return ret;
}

static const struct of_device_id hpc_mem_ctrl_table[] = {
	{ .compatible = "xring,hpc_heap_ctrl" },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, hpc_mem_ctrl_table);

static struct platform_driver hpc_mem_ctrl_driver = {
	.probe = hpc_mem_ctrl_probe,
	.remove = hpc_mem_ctrl_remove,
	.driver = {
		.name = "hpc-mem-ctrl-dev",
		.of_match_table = of_match_ptr(hpc_mem_ctrl_table),
	},
};

module_platform_driver(hpc_mem_ctrl_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC Memory Heap Driver");
MODULE_LICENSE("GPL v2");
