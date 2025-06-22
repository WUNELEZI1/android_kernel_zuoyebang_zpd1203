// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xr-kdump driver
 */

#define pr_fmt(fmt)	"[xr_kdump]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/of_reserved_mem.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/dfx_switch.h>
#include <soc/xring/xr_kdump.h>
#include <linux/android_debug_symbols.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <linux/mmzone.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>

#define XR_KDUMP_CB_MAGIC	0xDEADBEEFDEADBEEF
#define XR_KDUMP_ENABLE		0x5A5A5A5A5A5A5A5A

#define CRC_SIZE		4
#define MAX_KDUMP_REGION_CNT	50

#define INVALID_STATUS		0x0
#define GET_REGION_INFO		0x11
#define DUMP_REGION_DATA	0x22
#define READ_REGION_DATA	0x33

#define LOAD_TYPE			0
#define NOTE_TYPE			1
#define SYSRAM_TYPE			2
#define NODUMP_TYPE			3
#define PRESENT_TYPE		4

#define INIT_REGION_SIZE	100
#define CHECK_SUM_WIDTH		32
#define LOW_32_BIT_MASK		0xffffffff

struct addr_region {
	phys_addr_t base;
	phys_addr_t size;
};

struct mem_region {
	u64 cnt;
	u64 max;
	phys_addr_t total_size;
	struct addr_region *regions;
};

struct mem_region g_cma_region;

#pragma pack(4)
struct kd_region {
	u64	pbase_addr;
	u64	vbase_addr;
	u64	size;
	u64	type;
};

struct kd_header {
	u64	magic;
	u64	dfx_switch;
	u64 base_page_phys;
	u64 vmemmap_addr;
	u64 kimage_voffset;
	u64 vabits_actual_addr;
	u64 phys_offset;
	u64 virt_offset;
	u32 pg_oem_reserved_1;
	u32 pg_slab;
	u32 pg_head;
	u32 pg_buddy;
	u32 pg_swapcache;
	u32 pg_swapbacked;
	u32 pg_lru;
	u32 reserved;
	u32	status;
	u32	region_cnt;
	struct kd_region	kd_regions[MAX_KDUMP_REGION_CNT];
	u32	crc;
};
#pragma pack()

static void kdump_init_header(struct kd_header *header)
{
	header->magic = XR_KDUMP_CB_MAGIC;
	header->dfx_switch = (is_switch_on(SWITCH_KDUMP)) ? (XR_KDUMP_ENABLE) : (0);
	header->region_cnt = 0;
	header->crc = 0;
}

static u32 checksum32(u32 *addr, u32 count)
{
	u64 sum = 0;
	u32 left = 0;
	u32 i;

	while (count > (sizeof(u32) - 1)) {
		sum += *(addr++);
		count -= sizeof(u32);
	}

	if (count > 0) {
		left = 0;
		i = 0;
		while (i <= count) {
			*((u8 *)&left + i) = *((u8 *)addr + i);
			i++;
		}
		sum += left;
	}

	while (sum >> CHECK_SUM_WIDTH)
		sum = (sum & LOW_32_BIT_MASK) + (sum >> CHECK_SUM_WIDTH);

	return (~sum);
}

static int page_virt_to_phys(u64 virt_addr, u64 *phys_addr)
{
	pgd_t *pgd;
	p4d_t *p4dp;
	pud_t *pud;
	pmd_t *pmd;
	struct mm_struct *init_mm = (struct mm_struct *)android_debug_symbol(ADS_INIT_MM);

	pgd = pgd_offset(init_mm, virt_addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		pr_err("bad pgd\n");
		return -1;
	}

	p4dp = p4d_offset(pgd, virt_addr);
	if (p4d_none(*p4dp) || p4d_bad(*p4dp)) {
		pr_err("bad p4dp\n");
		return -1;
	}

	pud = pud_offset(p4dp, virt_addr);
	if (pud_none(*pud) || pud_bad(*pud)) {
		pr_err("bad pud\n");
		return -1;
	}

	pmd = pmd_offset(pud, virt_addr);
	if (pmd_none(*pmd)) {
		pr_err("bad pmd\n");
		return -1;
	}

	*phys_addr = __pmd_to_phys(*pmd) | (virt_addr & ~PMD_MASK);
	return 0;
}

static int init_memregion_info(void)
{
	u64 init_size;

	g_cma_region.total_size = 0;
	g_cma_region.cnt = 0;
	g_cma_region.max = INIT_REGION_SIZE;
	init_size = g_cma_region.max * sizeof(struct addr_region);
	g_cma_region.regions = kmalloc(init_size, GFP_KERNEL);
	if (g_cma_region.regions == NULL) {
		pr_err("kmalloc failed in init memregion info\n");
		return -ENOMEM;
	}
	return 0;
}

static void destroy_memregion_info(void)
{
	kfree(g_cma_region.regions);
	g_cma_region.regions = NULL;
}

static int memregion_double_array(struct mem_region *type)
{
	int ret;
	struct addr_region *new_array, *old_array;
	phys_addr_t old_size, new_size;

	old_size = type->max * sizeof(struct addr_region);
	new_size = old_size << 1;

	new_array = kzalloc(new_size, GFP_KERNEL);
	if (new_array == NULL)
		return -ENOMEM;

	ret = memcpy_s(new_array, new_size, type->regions, old_size);
	if (ret != EOK) {
		pr_err("memcpy_s in %s return error:%d\n", __func__, ret);
		kfree(new_array);
		return -1;
	}
	old_array = type->regions;
	type->regions = new_array;
	type->max <<= 1;
	kfree(old_array);

	return 0;
}

static int mem_insert_region(struct mem_region *type, int idx, phys_addr_t base, phys_addr_t size)
{
	int ret;
	struct addr_region *rgn = &type->regions[idx];

	ret = memmove_s(rgn + 1, (type->max - idx - 1) * sizeof(*rgn),
					rgn, (type->cnt - idx) * sizeof(*rgn));
	if (ret != EOK) {
		pr_err("memmove_s in %s return error:%d\n", __func__, ret);
		return -1;
	}
	rgn->base = base;
	rgn->size = size;
	type->cnt++;
	type->total_size += size;
	return 0;
}

static int mem_merge_regions(struct mem_region *type)
{
	int i = 0;
	int ret;

	while (i < type->cnt - 1) {
		struct addr_region *this = &type->regions[i];
		struct addr_region *next = &type->regions[i + 1];

		if (this->base + this->size != next->base) {
			i++;
			continue;
		}
		this->size += next->size;
		ret = memmove_s(next, (type->max - (i + 1)) * sizeof(*next),
						next + 1, (type->cnt - (i + 2)) * sizeof(*next));
		if (ret != EOK) {
			pr_err("memmove_s return err %d\n", ret);
			return -1;
		}
		type->cnt--;
	}
	return 0;
}

static void mem_add_first_region(struct mem_region *type, phys_addr_t base, phys_addr_t size)
{
	type->regions[0].base = base;
	type->regions[0].size = size;
	type->total_size = size;
	type->cnt = 1;
}

static int mem_add_region(struct mem_region *type, phys_addr_t base, phys_addr_t size)
{
	phys_addr_t end = base + size;
	int i;
	int ret;

	if (!size)
		return 0;

	if (type->regions[0].size == 0) {
		mem_add_first_region(type, base, size);
		return 0;
	}

	if (type->cnt + 1 > type->max) {
		ret = memregion_double_array(type);
		if (ret < 0)
			return ret;
	}

	for (i = 0; i < type->cnt; i++) {
		struct addr_region *rgn = &type->regions[i];
		phys_addr_t rbase = rgn->base;
		phys_addr_t rend = rbase + rgn->size;

		if (rbase >= end)
			break;
		if (rend <= base)
			continue;

		if (rbase > base) {
			ret = mem_insert_region(type, i++, base, rbase - base);
			if (ret < 0)
				return ret;
		}
		base = min(rend, end);
	}

	if (base < end) {
		ret = mem_insert_region(type, i, base, end - base);
		if (ret < 0)
			return ret;
	}

	return mem_merge_regions(type);
}

static int kdump_register_region(const struct kd_region *region, struct kd_header *header)
{
	struct kd_region *cur_region;
	int region_cnt = header->region_cnt;

	if (region_cnt >= MAX_KDUMP_REGION_CNT) {
		pr_err("kdump header region count too large\n");
		return -1;
	}

	cur_region = &header->kd_regions[region_cnt];
	header->region_cnt++;
	cur_region->pbase_addr = region->pbase_addr;
	cur_region->vbase_addr = region->vbase_addr;
	cur_region->size = region->size;
	cur_region->type = region->type;

	return 0;
}

static int kdump_get_base_page_info(struct kd_header *header)
{
	struct page *page;
	int sec_nr;
	struct memblock *memblock;

	memblock = (struct memblock *)android_debug_symbol(ADS_MEM_BLOCK);
	sec_nr = pfn_to_section_nr(memblock->memory.regions[0].base >> PAGE_SHIFT);
	page = pfn_to_page(section_nr_to_pfn(sec_nr));
	return page_virt_to_phys((u64)page, &header->base_page_phys);
}

static void kdump_get_kernel_para(struct kd_header *header)
{
	struct memblock *memblock;

	memblock = (struct memblock *)android_debug_symbol(ADS_MEM_BLOCK);
	header->vmemmap_addr = (u64)vmemmap;
	header->kimage_voffset = (u64)kimage_voffset;
	header->phys_offset = (u64)memblock->memory.regions[0].base;
	header->virt_offset = (u64)kaslr_offset();
	header->vabits_actual_addr = (u64)VA_BITS;
	header->pg_oem_reserved_1 = PG_oem_reserved_1;
	header->pg_buddy = PG_buddy;
	header->pg_head = PG_head;
	header->pg_lru = PG_lru;
	header->pg_slab = PG_slab;
	header->pg_swapbacked = PG_swapbacked;
	header->pg_swapcache = PG_swapcache;
}

static int kdump_register_cma_region(struct kd_header *header)
{
	struct memblock_region *region;
	phys_addr_t addr;
	int i = 0;
	struct page *page;
	int ret;

	struct memblock *memblock = (struct memblock *)android_debug_symbol(ADS_MEM_BLOCK);

	for (region = memblock->memory.regions;
		region < (memblock->memory.regions + memblock->memory.cnt);
		region++) {
		if (!memblock_is_nomap(region)) {
			for (addr = region->base;
				addr < region->base + region->size;
				addr += PAGE_SIZE) {
				page = pfn_to_page(addr >> PAGE_SHIFT);
				if (!is_migrate_cma_page(page))
					continue;
				ret = mem_add_region(&g_cma_region, addr, PAGE_SIZE);
				if (ret < 0) {
					pr_err("failed to add cma ram page on mem_add_region\n");
					return ret;
				}
			}
		}
	}

	for (i = 0; i < g_cma_region.cnt; i++) {
		struct kd_region kd_region;

		kd_region.vbase_addr = (u64)phys_to_virt(g_cma_region.regions[i].base);
		kd_region.pbase_addr = g_cma_region.regions[i].base;
		kd_region.size = g_cma_region.regions[i].size;
		kd_region.type = NODUMP_TYPE;
		ret = kdump_register_region(&kd_region, header);
		if (ret < 0) {
			pr_err("failed to add cma ram region in kdump\n");
			return ret;
		}
	}
	return 0;
}

static int kdump_register_sysram_region(struct kd_header *header)
{
	int ret;
	struct memblock_region *region;
	struct memblock *memblock = (struct memblock *)android_debug_symbol(ADS_MEM_BLOCK);
	struct kd_region kd_region;

	for (region = memblock->memory.regions;
		region < (memblock->memory.regions + memblock->memory.cnt);
		region++) {
		kd_region.vbase_addr = 0;
		kd_region.pbase_addr = region->base;
		kd_region.size = region->size;
		kd_region.type = PRESENT_TYPE;
		ret = kdump_register_region(&kd_region, header);
		if (ret < 0) {
			pr_err("failed to add present ram region in kdump\n");
			return ret;
		}
		if (!memblock_is_nomap(region)) {
			kd_region.vbase_addr = (u64)phys_to_virt(region->base);
			kd_region.pbase_addr = region->base;
			kd_region.size = region->size;
			kd_region.type = SYSRAM_TYPE;
			ret = kdump_register_region(&kd_region, header);
			if (ret < 0) {
				pr_err("failed to add sys ram region in kdump\n");
				return ret;
			}
		}
	}
	return 0;
}

static int kdump_register_kernel_info(void)
{
	int ret;
	struct kd_header *header;

	header = (struct kd_header *)memremap(DFX_MEM_KDUMP_ADDR, DFX_MEM_KDUMP_SIZE, MEMREMAP_WC);
	if (header == NULL) {
		pr_err("map kdump reserve mem failed!\n");
		return -ENOMEM;
	}
	header->status = INVALID_STATUS;
	kdump_init_header(header);
	ret = kdump_register_sysram_region(header);
	if (ret < 0)
		goto OUT;
	ret = kdump_register_cma_region(header);
	if (ret < 0)
		goto OUT;
	ret = kdump_get_base_page_info(header);
	if (ret < 0)
		goto OUT;
	kdump_get_kernel_para(header);
	header->status = GET_REGION_INFO;
	header->crc = checksum32((u32 *)header, sizeof(struct kd_header) - CRC_SIZE);
OUT:
	memunmap(header);
	return ret;
}

static int xr_kdump_probe(struct platform_device *pdev)
{
	int ret = 0;

	pr_info("enter xr_kdump probe\n");
	ret = init_memregion_info();
	if (ret < 0)
		return ret;

	ret = kdump_register_kernel_info();
	destroy_memregion_info();
	if (ret < 0) {
		pr_err("kdump register kernel info failed.\n");
		return ret;
	}

	return 0;
}

static int xr_kdump_remove(struct platform_device *pdev)
{
	pr_info("xr_kdump module remove\n");
	return 0;
}

static const struct of_device_id xr_kdump_of_match[] = {
	{ .compatible = "xring,kdump", },
	{},
};

static struct platform_driver xr_kdump_driver = {
	.probe = xr_kdump_probe,
	.remove = xr_kdump_remove,
	.driver = {
		.name = "xr_kdump",
		.of_match_table = xr_kdump_of_match,
	},
};

module_platform_driver(xr_kdump_driver);

MODULE_IMPORT_NS(MINIDUMP);
MODULE_DESCRIPTION("XRing kdmup driver");
MODULE_LICENSE("GPL v2");
