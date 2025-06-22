// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "tee", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "tee", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <tee_drv.h>
#include <linux/uuid.h>
#include <linux/dma-heap.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/delay.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_tee_ca.h"
#include "xrisp_tee_ca_api.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "xrisp_log.h"

#define XRISP_TA_UUID                                                                    \
	UUID_INIT(0x3fffcf00, 0xa684, 0x4e03, 0xa6, 0xd1, 0x10, 0x22, 0xee, 0xf4, 0x59,  \
		  0x39)

struct xrisp_ca_private *g_xrisp_pvt;

static struct xrisp_ca_private *xrisp_ca_get_prvdata(void)
{
	if (!g_xrisp_pvt)
		return NULL;
	return g_xrisp_pvt;
}

static bool isp_in_secmode(void)
{
	if (!g_xrisp_pvt)
		return false;

	if (!(g_xrisp_pvt->mode & TA_SECMODE_ISP)) {
		XRISP_PR_ERROR("not in sec mode");
		return false;
	}
	return true;
}

static int to_sec_dmabuf_smmu_dir(uint32_t flags)
{
	if (flags & CAM_BUF_HW_READ_ONLY)
		return TA_DMA_TO_DEVICE;
	else if (flags & CAM_BUF_HW_WRITE_ONLY)
		return TA_DMA_FROM_DEVICE;
	else if (flags & CAM_BUF_HW_READ_WRITE)
		return TA_DMA_BIDIRECTIONAL;

	return -EINVAL;
}

static int to_sec_dmabuf_smmu_attrs(uint32_t flags)
{
	if (flags & CAM_BUF_HW_READ_ONLY)
		return TA_DMA_IOMMU_READ | TA_DMA_IOMMU_NOEXEC;
	else if (flags & CAM_BUF_HW_WRITE_ONLY)
		return TA_DMA_IOMMU_WRITE | TA_DMA_IOMMU_NOEXEC;
	else if (flags & CAM_BUF_HW_READ_WRITE)
		return TA_DMA_IOMMU_READ | TA_DMA_IOMMU_WRITE | TA_DMA_IOMMU_NOEXEC;
	return -EINVAL;
}

static inline void *kzalloc_page_align(size_t size, gfp_t flags)
{
	uint32_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int retry_cnt = 0;
	void *ptr;

	while (retry_cnt++ < CA_KMALLOC_RETRY_TIMES) {
		ptr = kzalloc(num_pages * PAGE_SIZE, flags);
		if (ptr)
			return ptr;

		XRISP_PR_WARN("no free memory, alloc pages %d retrying (%d/%d)", num_pages,
			      retry_cnt, CA_KMALLOC_RETRY_TIMES);
		msleep(CA_KMALLOC_RETRY_DELAY_MS);
	}

	if (!ptr)
		XRISP_PR_ERROR("no free memory, alloc pages %d failed", num_pages);

	return ptr;
}

static int xrisp_ca_send_single_msg(struct xrisp_ca_private *xrisp_pvt, enum ta_cmd_id id,
				    enum ta_secmode_item item,  enum ta_secmode_flag mode)
{
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	int ret;

	if (!xrisp_pvt || !xrisp_pvt->session)
		return -EINVAL;

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));

	// Invoke commands
	inv_arg.func = id;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = item;
	param[0].u.value.b = mode;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("tee_client_invoke_func error: %x, %x\n", ret, inv_arg.ret);
		return -EOPNOTSUPP;
	}

	return 0;
}

/*
 * mode: 2-safe, 1-unsafe
 */
int xrisp_ca_switch_mode(enum ta_secmode_item item, enum ta_secmode_flag mode)
{
	struct xrisp_ca_private *xrisp_pvt;
	int ret = 0;
	int i, item_tmp;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session)
		return -EINVAL;

	if ((mode != TA_SECMODE_UNSAFE) && (mode != TA_SECMODE_SAFE)) {
		XRISP_PR_ERROR("unknown mode %d, exit", mode);
		return -EINVAL;
	}
	if ((item & TA_SECMODE_ALL) == 0) {
		XRISP_PR_ERROR("unknown item 0x%x, exit", item);
		return -EINVAL;
	}

	ret = xrisp_ca_send_single_msg(xrisp_pvt, TA_CMD_SWITCH_MODE, item, mode);
	if (ret) {
		XRISP_PR_ERROR("switch item:0x%x to mode:%d failure, ret= %d", item, mode, ret);
		return -EOPNOTSUPP;
	}

	item_tmp = item;
	for (i = 0; item_tmp != 0; i++) {
		if ((BIT(i) & item_tmp) == 0)
			continue;

		xrisp_pvt->mode &= ~(BIT(i));
		if (mode == TA_SECMODE_SAFE)
			xrisp_pvt->mode |= BIT(i);
		item_tmp &= ~(BIT(i));
	}

	XRISP_PR_INFO("ca switch item:0x%x to %s success, mode:0x%x", item,
		      ((mode == TA_SECMODE_SAFE) ? "sec" : "non-sec"), xrisp_pvt->mode);
	return 0;
}

int xrisp_ca_attach(bool is_attach)
{
	struct tee_ioctl_invoke_arg inv_arg;
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_param param[4];
	int ret;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));

	if (is_attach)
		inv_arg.func = TA_CMD_SMMU_ATTACH;
	else
		inv_arg.func = TA_CMD_SMMU_DETACH;

	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("%s switch failed, tee_client_invoke_func error: %x, %x\n",
			       (is_attach ? "attach" : "detach"), ret, inv_arg.ret);
		return -EOPNOTSUPP;
	}

	XRISP_PR_INFO("%s switch success", (is_attach ? "attach" : "detach"));
	return 0;
}

/*
 * params[0] fw_ram_info_page;
 */
int xrisp_sec_fw_load(void)
{
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	struct ta_xrisp_load_info *fw_ram_info = NULL;
	struct dma_buf *dma_buf = NULL;
	struct tee_shm *meminfo_shm = NULL;
	int ret = 0;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;

	if (!xrisp_pvt->sec_heap) {
		XRISP_PR_ERROR("failed to get sec_heap");
		return -ENOMEM;
	}

	XRISP_PR_DEBUG("alloc fw buf start");
	/*  alloc fw load mem  */
	dma_buf = dma_heap_buffer_alloc(xrisp_pvt->sec_heap, FIRMWARE_LOAD_SIZE, O_RDWR, 0);
	if (IS_ERR(dma_buf)) {
		XRISP_PR_ERROR("failed to alloc fw load_buf from sec_heap");
		return -EINVAL;
	}
	xr_dmabuf_kernel_account(dma_buf, XR_DMABUF_KERNEL_ISP_CA);
	fw_ram_info = (struct ta_xrisp_load_info *)kzalloc_page_align(
		sizeof(struct ta_xrisp_load_info), GFP_KERNEL);
	if (!fw_ram_info) {
		XRISP_PR_ERROR("failed to alloc fw load_buf");
		ret = -ENOMEM;
		goto free_load_buf;
	}

	fw_ram_info->sfd = xr_dmabuf_helper_get_sfd(dma_buf);
	if (!fw_ram_info->sfd) {
		XRISP_PR_ERROR("get load_buf sfd failed");
		ret = -EINVAL;
		goto kfree_mem;
	}
	XRISP_PR_DEBUG("alloc fw buf success");

	fw_ram_info->size = FIRMWARE_LOAD_SIZE;
	fw_ram_info->iova = FIRMWARE_BOOT_ADDR;
	fw_ram_info->fs_load = false;

	meminfo_shm = tee_shm_register(xrisp_pvt->ctx, (unsigned long)fw_ram_info,
				       sizeof(struct ta_xrisp_load_info),
				       TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(meminfo_shm)) {
		XRISP_PR_ERROR("failed to register shm meminfo");
		ret = -EINVAL;
		goto kfree_mem;
	}

	//ca communication
	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));
	inv_arg.func = TA_CMD_LOAD_FW;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = meminfo_shm;
	param[0].u.memref.size = sizeof(struct ta_xrisp_load_info);

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("load fw failed, tee_client_invoke_func error: %x, %x\n",
			       ret, inv_arg.ret);
		ret = -EOPNOTSUPP;
		goto free_meminfo_shm;
	}

	xrisp_pvt->sfd = fw_ram_info->sfd;
	xrisp_pvt->load_buf = dma_buf;
	xrisp_pvt->load_size = fw_ram_info->size;
	XRISP_PR_INFO("fw load success, size=%d, sfd=%d", FIRMWARE_LOAD_SIZE, xrisp_pvt->sfd);

free_meminfo_shm:
	tee_shm_free(meminfo_shm);
kfree_mem:
	kfree(fw_ram_info);
free_load_buf:
	if (ret) {
		xr_dmabuf_kernel_unaccount(dma_buf, XR_DMABUF_KERNEL_ISP_CA);
		dma_heap_buffer_free(dma_buf);
		xrisp_pvt->sfd = 0;
		xrisp_pvt->load_buf = NULL;
		xrisp_pvt->load_size = 0;
	}

	return ret;
}

/*
 * params[0].u.value.a sfd;
 * params[0].u.value.b size;
 */
int xrisp_sec_fw_unload(void)
{
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	int ret = 0;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;

	if (!xrisp_pvt->sfd || IS_ERR_OR_NULL(xrisp_pvt->load_buf) ||
	    !xrisp_pvt->load_size) {
		XRISP_PR_ERROR("fw load abnormal, unload exit, sfd=0x%x, load_size=0x%x, fw_buf_state=%d",
		       xrisp_pvt->sfd, xrisp_pvt->load_size,
		       IS_ERR_OR_NULL(xrisp_pvt->load_buf));
		ret = -EINVAL;
		goto clean_load_state;
	}

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));
	inv_arg.func = TA_CMD_UNLOAD_FW;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = xrisp_pvt->sfd;
	param[0].u.value.b = xrisp_pvt->load_size;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("unload fw failed, tee_client_invoke_func error: %x, %x\n", ret,
		       inv_arg.ret);
		return -EOPNOTSUPP;
	}

	XRISP_PR_INFO("unload fw success");

clean_load_state:
	xrisp_pvt->sfd = 0;
	xrisp_pvt->load_size = 0;
	if (xrisp_pvt->load_buf) {
		xr_dmabuf_kernel_unaccount(xrisp_pvt->load_buf, XR_DMABUF_KERNEL_ISP_CA);
		dma_heap_buffer_free(xrisp_pvt->load_buf);
	}
	xrisp_pvt->load_buf = NULL;
	return ret;
}

/*
 * secbuf smmu map
 * INPUT:
 * params[0].value.a type;
 * params[0].value.b reserved;
 * params[1].value.a sfd;
 * params[1].value.b size;
 * params[2].value.a mmu_attrs
 *     For mmu cache policy genmask(7, 0);
 * params[2].value.b smmu_attrs
 *     For smmu: direction genmask(15, 8); READ&WRITE genmask(7, 0);
 * OUTPUT:
 * params[3].value.a iova_l (For smmu);
 * params[3].value.b iova_h (For smmu);
 */
int xrisp_secbuf_map(uint32_t sfd, uint32_t size, uint32_t flags, dma_addr_t *iova)
{
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	int ret;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;

	if (!sfd || !size || !flags || IS_ERR_OR_NULL(iova)) {
		XRISP_PR_ERROR("Invalid input param");
		return -EINVAL;
	}

	if (to_sec_dmabuf_smmu_attrs(flags) < 0 || to_sec_dmabuf_smmu_dir(flags) < 0) {
		XRISP_PR_ERROR("Invalid secbuf flags=0x%x", flags);
		return -EINVAL;
	}

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));
	inv_arg.func = TA_CMD_MMAP;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = TA_DMA_BUF_MAP_TYPE_SMMU;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = sfd;
	param[1].u.value.b = size;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].u.value.a = 0;
	param[2].u.value.b =
		(to_sec_dmabuf_smmu_dir(flags) << 8) | to_sec_dmabuf_smmu_attrs(flags);

	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("secbuf ssmmu map failed, tee_client_invoke_func error: %x, %x\n",
		       ret, inv_arg.ret);
		return -EOPNOTSUPP;
	}

	*iova = (param[3].u.value.b << 32) + param[3].u.value.a;
	XRISP_PR_INFO("secbuf ssmmu map success, sfd=0x%x, size=%d, flag=0x%x, iova=0x%08llx",
		sfd, size, flags, *iova);

	return 0;
}

/*
 * params[0].u.value.a type;
 * params[0].u.value.b reserved;
 * params[1].u.value.a sfd;
 * params[1].u.value.b size;
 */
int xrisp_secbuf_unmap(uint32_t sfd, uint32_t size)
{
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	int ret;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;
	if (!sfd || !size) {
		XRISP_PR_ERROR("Invalid input param");
		return -EINVAL;
	}

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));
	inv_arg.func = TA_CMD_MUNMAP;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = TA_DMA_BUF_MAP_TYPE_SMMU;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = sfd;
	param[1].u.value.b = size;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("secbuf ssmmu unmap failed sfd=0x%x, tee_client_invoke_func error: %x, %x\n",
		       sfd, ret, inv_arg.ret);
		return -EOPNOTSUPP;
	}

	XRISP_PR_INFO("secbuf ssmmu unmap success, sfd=0x%x, size=%d", sfd, size);
	return 0;
}

static struct sg_table_pa *xrisp_sgt_to_sgt_pa(struct sg_table *sgt)
{
	struct scatterlist *sg = NULL;
	struct sg_table_pa *sgt_pa = NULL;
	int i = 0;

	if (!sgt) {
		XRISP_PR_ERROR("Invalid input param");
		return NULL;
	}

	sgt_pa = kzalloc_page_align(sizeof(struct sg_table_pa) * sgt->nents, GFP_KERNEL);
	if (!sgt_pa) {
		XRISP_PR_ERROR("sgt_pa alloc failed, sg_num=%d", sgt->nents);
		return NULL;
	}

	for_each_sg((sgt)->sgl, sg, (sgt)->nents, i) {
		sgt_pa[i].paddr = sg_phys(sg);
		sgt_pa[i].len = sg->length;
	}

	return sgt_pa;
}

/*
 * INPUT:
 * params[0].memref.size
 * params[0].memref.buffer    struct sg_table_pa;
 * INOUT:
 * params[1].memref.size
 * params[1].memref.buffer    struct ta_dma_buf_nonsec_info;
 */
static int xrisp_nsbuf_ssmmu_map_core(struct sg_table_pa sg_palist[], uint32_t sg_num,
				      uint32_t mmap_type, uint64_t *iova, bool is_map)
{
	struct xrisp_ca_private *xrisp_pvt;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	struct ta_dma_buf_nonsec_info *nsmap_info = NULL;
	struct tee_shm *shm_sg_info = NULL;
	struct tee_shm *shm_mmap_info = NULL;
	size_t sg_palist_size = sizeof(struct sg_table_pa) * sg_num;
	void *sg_palist_info = sg_palist;
	int ret = 0;

	xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode())
		return -EINVAL;

	if (!sg_palist || !sg_num) {
		XRISP_PR_ERROR("Invalid sg parma");
		return -EINVAL;
	}

	if (mmap_type != TA_SMMU_MMAP_TYPE_FIX_IOVA &&
	    mmap_type != TA_SMMU_MMAP_TYPE_FREE_IOVA) {
		XRISP_PR_ERROR("Invalid mmap_type");
		return -EINVAL;
	}

	if (mmap_type == TA_SMMU_MMAP_TYPE_FIX_IOVA && *iova == 0) {
		XRISP_PR_ERROR("Fixed mapping need to input iova");
		return -EINVAL;
	}

	mutex_lock(&xrisp_pvt->nsbuf_map_mtx);

	shm_sg_info = tee_shm_register(xrisp_pvt->ctx, (unsigned long)sg_palist_info,
				       sg_palist_size, TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR_OR_NULL(shm_sg_info)) {
		XRISP_PR_ERROR("shm_sg_info register failed");
		ret = -EINVAL;
		goto unlock_map_mtx;
	}

	nsmap_info = (struct ta_dma_buf_nonsec_info *)kzalloc_page_align(
		sizeof(struct ta_dma_buf_nonsec_info), GFP_KERNEL);
	if (!nsmap_info) {
		XRISP_PR_ERROR("nsmap_info alloc failed");
		ret = -ENOMEM;
		goto free_shm_sg;
	}

	nsmap_info->mmap_type = mmap_type;
	nsmap_info->iova = *iova;

	shm_mmap_info = tee_shm_register(xrisp_pvt->ctx, (unsigned long)nsmap_info,
					 sizeof(struct ta_dma_buf_nonsec_info),
					 TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR_OR_NULL(shm_mmap_info)) {
		XRISP_PR_ERROR("shm_mmap_info register failed");
		ret = -EINVAL;
		goto kfree_nsmap_mem;
	}

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));
	if (is_map)
		inv_arg.func = TA_CMD_SMMU_UNSAFE_MMAP;
	else
		inv_arg.func = TA_CMD_SMMU_UNSAFE_MUNMAP;
	inv_arg.session = xrisp_pvt->session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_sg_info;
	param[0].u.memref.size = sizeof(struct sg_table_pa) * sg_num;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_mmap_info;
	param[1].u.memref.size = sizeof(struct ta_dma_buf_nonsec_info);

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;
	param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_NONE;

	ret = tee_client_invoke_func(xrisp_pvt->ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		XRISP_PR_ERROR("tee_client_invoke_func error: %x, %x\n", ret, inv_arg.ret);
		ret = -EOPNOTSUPP;
	} else {
		if (mmap_type == TA_SMMU_MMAP_TYPE_FREE_IOVA)
			*iova = nsmap_info->iova;
	}

	if (ret < 0)
		XRISP_PR_ERROR("nsbuf ssmmu %s failed, sg_num=%d, mmap_type=%s",
			       (is_map ? "map" : "unmap"), sg_num,
			       (mmap_type == TA_SMMU_MMAP_TYPE_FREE_IOVA ? "dynamic" : "fix"));
	else
		XRISP_PR_INFO("nsbuf ssmmu %s success, sg_num=%d, mmap_type=%s, iova=0x%llx",
			      (is_map ? "map" : "unmap"), sg_num,
			      (mmap_type == TA_SMMU_MMAP_TYPE_FREE_IOVA ? "dynamic" : "fix"),
			      *iova);

	tee_shm_free(shm_mmap_info);
kfree_nsmap_mem:
	kfree(nsmap_info);
free_shm_sg:
	tee_shm_free(shm_sg_info);
unlock_map_mtx:
	mutex_unlock(&xrisp_pvt->nsbuf_map_mtx);
	return ret;
}

static int xrisp_ns_cmabuf_ssmmu_map_core(phys_addr_t paddr, uint32_t size,
					  uint64_t *fixiova, bool ismap)
{
	struct sg_table_pa *sg_pa;
	int ret = 0;

	if (!paddr || !size || !*fixiova) {
		XRISP_PR_ERROR("input parma invalid");
		return -EINVAL;
	}

	sg_pa = kzalloc_page_align(sizeof(struct sg_table_pa), GFP_KERNEL);
	if (!sg_pa) {
		XRISP_PR_ERROR("kzalloc failed, exit");
		return -ENOMEM;
	}

	sg_pa->paddr = paddr;
	sg_pa->len = size;

	ret = xrisp_nsbuf_ssmmu_map_core(sg_pa, 1, TA_SMMU_MMAP_TYPE_FIX_IOVA, fixiova,
					 ismap);
	if (ret) {
		XRISP_PR_ERROR("ns_cmabuf ssmmu %s failed", (ismap ? "map" : "unmap"));
		ret = -EINVAL;
	} else
		XRISP_PR_INFO("ns_cmabuf ssmmu %s success", (ismap ? "map" : "unmap"));

	kfree(sg_pa);
	sg_pa = NULL;
	return ret;
}

//cmabuf smmu map
int xrisp_ns_cmabuf_ssmmu_map(phys_addr_t paddr, uint32_t size, uint64_t *iova)
{
	return xrisp_ns_cmabuf_ssmmu_map_core(paddr, size, iova, true);
}

//cmabuf safe unmap
int xrisp_ns_cmabuf_ssmmu_unmap(phys_addr_t paddr, uint32_t size, uint64_t *iova)
{
	return xrisp_ns_cmabuf_ssmmu_map_core(paddr, size, iova, false);
}

static int xrisp_ns_dmabuf_ssmmu_map_core(struct sg_table *sgt, uint64_t *siova, uint32_t mmap_type,
					  bool is_map)
{
	struct xrisp_ca_private *xrisp_pvt = (struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	struct sg_table_pa *sgt_pa = NULL;
	int ret = 0;

	if (!sgt || !siova) {
		XRISP_PR_ERROR("input parma invalid");
		return -EINVAL;
	}

	if (!xrisp_pvt || !xrisp_pvt->session || !isp_in_secmode()) {
		XRISP_PR_ERROR("session not build");
		return -EINVAL;
	}

	sgt_pa = xrisp_sgt_to_sgt_pa(sgt);
	if (!sgt_pa) {
		XRISP_PR_ERROR("no free memory, get sg_table_pa err, exit");
		return -ENOMEM;
	}

	ret = xrisp_nsbuf_ssmmu_map_core(sgt_pa, sgt->nents, mmap_type, siova, is_map);
	if (ret)
		XRISP_PR_ERROR("ns_dmabuf ssmmu map failed");

	kfree(sgt_pa);
	return ret;
}

int xrisp_ns_dmabuf_ssmmu_map(struct sg_table *sgt, uint64_t *siova, uint32_t mmap_type)
{
	return xrisp_ns_dmabuf_ssmmu_map_core(sgt, siova, mmap_type, true);
}

int xrisp_ns_dmabuf_ssmmu_unmap(struct sg_table *sgt, uint64_t *siova, uint32_t mmap_type)
{
	return xrisp_ns_dmabuf_ssmmu_map_core(sgt, siova, mmap_type, false);
}

static int xrisp_ca_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

int xrisp_ca_context_build(void)
{
	struct xrisp_ca_private *xrisp_pvt =
		(struct xrisp_ca_private *)xrisp_ca_get_prvdata();
	struct tee_ioctl_open_session_arg sess_arg;
	int ret = 0;

	if (!xrisp_pvt)
		return -EINVAL;

	if (atomic_read(&xrisp_pvt->open_cnt)) {
		atomic_inc(&xrisp_pvt->open_cnt);
		pr_info("session already open count:%d.", atomic_read(&xrisp_pvt->open_cnt));
		return 0;
	}
	/* Open context with OP-TEE driver */
	xrisp_pvt->ctx = tee_client_open_context(NULL, xrisp_ca_ctx_match, NULL, NULL);
	if (IS_ERR_OR_NULL(xrisp_pvt->ctx)) {
		XRISP_PR_ERROR("tee_client_open_context error\n");
		return -ENODEV;
	}

	/* Open session with xrisp TA */
	memset(&sess_arg, 0, sizeof(sess_arg));
	export_uuid(sess_arg.uuid, &XRISP_TA_UUID);
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(xrisp_pvt->ctx, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		XRISP_PR_ERROR("tee_client_open_session error: %x, %x\n", ret, sess_arg.ret);
		tee_client_close_context(xrisp_pvt->ctx);
		return -EINVAL;
	}
	xrisp_pvt->session = sess_arg.session;
	atomic_inc(&xrisp_pvt->open_cnt);

	XRISP_PR_INFO("xrisp ca context build finish");
	return 0;
}

void xrisp_ca_context_unbuild(void)
{
	struct xrisp_ca_private *xrisp_pvt =
		(struct xrisp_ca_private *)xrisp_ca_get_prvdata();

	if (!xrisp_pvt || !xrisp_pvt->session)
		return;

	if (atomic_dec_return(&xrisp_pvt->open_cnt)) {
		if (atomic_read(&xrisp_pvt->open_cnt) < 0)
			atomic_set(&xrisp_pvt->open_cnt, 0);

		pr_info("session open count:%d.", atomic_read(&xrisp_pvt->open_cnt));
		return;
	}
	tee_client_close_session(xrisp_pvt->ctx, xrisp_pvt->session);
	tee_client_close_context(xrisp_pvt->ctx);
	xrisp_pvt->session = 0;
	xrisp_pvt->ctx = NULL;
	XRISP_PR_INFO("xrisp ca context unbuild finish");
}

int xrisp_ca_init(void)
{
	int ret = 0;
	struct xrisp_ca_private *xrisp_pvt = NULL;

	XRISP_PR_INFO("xrisp_ca probe start\n");

	xrisp_pvt = kzalloc(sizeof(struct xrisp_ca_private), GFP_KERNEL);
	if (!xrisp_pvt)
		return -ENOMEM;

	atomic_set(&xrisp_pvt->open_cnt, 0);
	xrisp_pvt->sec_heap = dma_heap_find("xring_isp_faceid");
	if (IS_ERR_OR_NULL(xrisp_pvt->sec_heap)) {
		ret = -EINVAL;
		goto free_mem;
	}

	xrisp_ca_debugfs_init(xrisp_pvt);

	mutex_init(&xrisp_pvt->nsbuf_map_mtx);

	g_xrisp_pvt = xrisp_pvt;
	XRISP_PR_INFO("xrisp_ca probe success");
	return 0;

free_mem:
	kfree(xrisp_pvt);
	xrisp_pvt = NULL;
	return ret;
}

void xrisp_ca_exit(void)
{
	XRISP_PR_INFO("xrisp_ca remove\n");
	if (!g_xrisp_pvt)
		return;

	mutex_destroy(&g_xrisp_pvt->nsbuf_map_mtx);

	if (g_xrisp_pvt->sec_heap)
		dma_heap_put(g_xrisp_pvt->sec_heap);

	xrisp_ca_debugfs_exit(g_xrisp_pvt);
	kfree(g_xrisp_pvt);
	g_xrisp_pvt = NULL;
}

MODULE_AUTHOR("Liang Pan<liangpan@xiaomi.com>");
MODULE_AUTHOR("lizexin <lizexin@xiaomi.com>");
MODULE_DESCRIPTION("xring camera isp tee ca");
MODULE_LICENSE("GPL");
