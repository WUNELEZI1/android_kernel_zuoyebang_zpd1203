/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRISP_TEE_CA_API_H__
#define __XRISP_TEE_CA_API_H__
#include <linux/kernel.h>
#include <linux/firmware.h>
#include <linux/dma-buf.h>

enum ta_secmode_flag {
	TA_SECMODE_UNSAFE = 1,
	TA_SECMODE_SAFE = 2,
};

enum ta_secmode_item {
	TA_SECMODE_ISP      = 0x1, //BIT(0)
	TA_SECMODE_CSI      = 0x2, //BIT(1)
	TA_SECMODE_CSIWDMA  = 0x4, //BIT(2)
	TA_SECMODE_ALL      = 0x7,
};

enum ta_dma_buf_smmu_mmap_type {
	TA_SMMU_MMAP_TYPE_FREE_IOVA = 1,
	TA_SMMU_MMAP_TYPE_FIX_IOVA = 2,
};

enum ta_cmd_id {
	TA_CMD_SWITCH_MODE = 0,
	TA_CMD_LOAD_FW,
	TA_CMD_UNLOAD_FW,
	TA_CMD_SMMU_ATTACH,
	TA_CMD_SMMU_DETACH,
	TA_CMD_MMAP, //5
	TA_CMD_MUNMAP,
	TA_CMD_SMMU_UNSAFE_MMAP,
	TA_CMD_SMMU_UNSAFE_MUNMAP, //8
};

enum ta_dma_buf_map_type {
	TA_DMA_BUF_MAP_TYPE_MMU = 1,
	TA_DMA_BUF_MAP_TYPE_SMMU = 2,
	TA_DMA_BUF_MAP_TYPE_MMU_SMMU = 3,
};

enum ta_dma_buf_cache_type {
	TA_DMA_BUF_CACHE_TYPE_UNCACHED = 1,
	TA_DMA_BUF_CACHE_TYPE_CACHED = 2,
};

enum ta_dma_data_direction {
	TA_DMA_BIDIRECTIONAL = 0,
	TA_DMA_TO_DEVICE = 1,
	TA_DMA_FROM_DEVICE = 2,
	TA_DMA_NONE = 3,
};

enum ta_dma_data_attrs {
	TA_DMA_IOMMU_READ = 1,
	TA_DMA_IOMMU_WRITE = 2,
	TA_DMA_IOMMU_CACHE = 4,
	TA_DMA_IOMMU_NOEXEC = 8,
	TA_DMA_IOMMU_MMIO = 16,
	TA_DMA_IOMMU_PRIV = 32,
	TA_DMA_IOMMU_SYS_CACHE = 64,
	TA_DMA_IOMMU_SYS_CACHE_NWA = 128,
};

struct sg_table_pa {
	uint64_t paddr;
	uint64_t len;
};

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
int xrisp_ca_init(void);
void xrisp_ca_exit(void);

int xrisp_ca_switch_mode(enum ta_secmode_item item, enum ta_secmode_flag mode);
int xrisp_ca_attach(bool is_attach);

int xrisp_ca_context_build(void);
void xrisp_ca_context_unbuild(void);

int xrisp_sec_fw_load(void);
int xrisp_sec_fw_unload(void);

int xrisp_secbuf_map(uint32_t sfd, uint32_t size, uint32_t flags, dma_addr_t *iova);
int xrisp_secbuf_unmap(uint32_t sfd, uint32_t size);

int xrisp_ns_cmabuf_ssmmu_map(phys_addr_t paddr, uint32_t size, uint64_t *iova);
int xrisp_ns_cmabuf_ssmmu_unmap(phys_addr_t paddr, uint32_t size, uint64_t *iova);

int xrisp_ns_dmabuf_ssmmu_map(struct sg_table *sgt, uint64_t *siova, uint32_t mmap_type);
int xrisp_ns_dmabuf_ssmmu_unmap(struct sg_table *sgt, uint64_t *siova, uint32_t mmap_type);
#else
static inline int xrisp_ca_init(void)
{
	return 0;
}
static inline void xrisp_ca_exit(void)
{
}
static inline int xrisp_ca_switch_mode(enum ta_secmode_item item, enum ta_secmode_flag mode)
{
	return -ENODEV;
}
static inline int xrisp_ca_attach(bool is_attach)
{
	return -ENODEV;
}
static inline int xrisp_ca_context_build(void)
{
	return -ENODEV;
}
static inline void xrisp_ca_context_unbuild(void)
{
}
static inline int xrisp_sec_fw_load(void)
{
	return -ENODEV;
}
static inline int xrisp_sec_fw_unload(void)
{
	return -ENODEV;
}
static inline int xrisp_secbuf_map(uint32_t sfd, uint32_t size, uint32_t flags, dma_addr_t *iova)
{
	return -ENODEV;
}
static inline int xrisp_secbuf_unmap(uint32_t sfd, uint32_t size)
{
	return -ENODEV;
}
static inline int xrisp_ns_cmabuf_ssmmu_map(phys_addr_t paddr, uint32_t size, uint64_t *iova)
{
	return -ENODEV;
}
static inline int xrisp_ns_cmabuf_ssmmu_unmap(phys_addr_t paddr, uint32_t size, uint64_t *iova)
{
	return -ENODEV;
}
static inline int xrisp_ns_dmabuf_ssmmu_map(struct sg_table *sgt, uint64_t *siova,
					    uint32_t mmap_type)
{
	return -ENODEV;
}
static inline int xrisp_ns_dmabuf_ssmmu_unmap(struct sg_table *sgt, uint64_t *siova,
					      uint32_t mmap_type)
{
	return -ENODEV;
}
#endif
#endif
