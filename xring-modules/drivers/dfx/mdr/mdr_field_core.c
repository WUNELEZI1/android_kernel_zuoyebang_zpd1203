// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include "mdr_print.h"
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/of.h>
#include "mdr_field_core.h"
#include "mdr.h"

static struct mdr_struct_s *pbb;
static struct mdr_struct_s *tmp_pbb;

u64 g_mdr_area_mem_addr[MDR_AREA_MAXIMUM + 1];
u32 g_mdr_area_mem_size[MDR_AREA_MAXIMUM] = {0};

static u32 g_reboot_type;
static struct mdr_data *g_data;

u64 mdr_reserved_phymem_addr(void)
{
	return g_data->reserved_mdr_phymem_addr;
}

u64 mdr_reserved_phymem_size(void)
{
	return g_data->reserved_mdr_phymem_size;
}

u32 mdr_get_reboot_type(void)
{
	return g_reboot_type;
}


void *mdr_map(phys_addr_t paddr, size_t size)
{
	void *vaddr = NULL;
	u32 reserved_mdr_phymem_addr = g_data->reserved_mdr_phymem_addr;
	u32 reserved_mdr_phymem_size = g_data->reserved_mdr_phymem_size;

	if (paddr < g_data->reserved_mdr_phymem_addr || !size ||
	    (paddr + size) > (reserved_mdr_phymem_addr + reserved_mdr_phymem_size)) {
		MDR_PRINT_ERR("Error BBox memory.\n");
		return NULL;
	}

	if (pfn_valid(reserved_mdr_phymem_addr >> PAGE_SHIFT))
		vaddr = mdr_vmap(paddr, size);
	else
		vaddr = ioremap_wc(paddr, size);

	return vaddr;
}
EXPORT_SYMBOL(mdr_map);

void mdr_unmap(const void *vaddr)
{
	if (pfn_valid(g_data->reserved_mdr_phymem_addr >> PAGE_SHIFT))
		vunmap(vaddr);
	else
		iounmap((void __iomem *)vaddr);
}
EXPORT_SYMBOL(mdr_unmap);


#define PAGE_ALIGN(addr)    (((addr)+PAGE_SIZE-1)&PAGE_MASK)

void *mdr_vmap(phys_addr_t paddr, size_t size)
{
	int i;
	void *vaddr = NULL;
	pgprot_t pgprot;
	unsigned long offset;
	int pages_count;
	struct page **pages;

	offset = paddr & ~PAGE_MASK;
	paddr &= PAGE_MASK;
	pages_count = PAGE_ALIGN(size + offset) / PAGE_SIZE;

	pages = kcalloc(pages_count, sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		return NULL;

	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for (i = 0; i < pages_count; i++)
		*(pages + i) = phys_to_page(paddr + PAGE_SIZE * i);

	vaddr = vmap(pages, pages_count, VM_MAP, pgprot);
	kfree(pages);
	if (vaddr == NULL)
		return NULL;

	return offset + (char *)vaddr;
}

#define MDR_BUILD_DATE_TIME_LEN 16

#define TIMELEN 8
#define DATELEN 11

int get_kernel_build_time(char *blddt, int dtlen, char *bldtm, int tmlen)
{
	return 0;
}

void mdr_get_builddatetime(u8 *out)
{
	u8 *pout = out;
	u8 *p = NULL;
	u8 date[DATELEN  + 1] = {0};
	u8 time[TIMELEN + 1] = {0};
	int cnt = MDR_BUILD_DATE_TIME_LEN;
	int ret = -EINVAL;

	if (out == NULL) {
		MDR_PRINT_ERR("out is null!\n");
		return;
	}
	memset((void *)out, 0, MDR_BUILD_DATE_TIME_LEN);

	ret = get_kernel_build_time(date, DATELEN, time, TIMELEN);
	if (ret) {
		MDR_PRINT_ERR("get kernel build time failed!\n");
		goto error;
	}
	date[DATELEN] = '\0';
	time[TIMELEN] = '\0';

	p = date;
	while (*p) {
		if (!cnt)
			goto error;
		if (*p != ' ') {
			*pout++ = *p++;
			cnt--;
		} else {
			p++;
		}
	}

	p = time;
	while (*p) {
		if (!cnt)
			goto error;
		if (*p != ':') {
			*pout++ = *p++;
			cnt--;
		} else {
			p++;
		}
	}

error:
	out[MDR_BUILD_DATE_TIME_LEN - 1] = '\0';
}

void mdr_set_area_info(int index, u32 size)
{
	g_mdr_area_mem_size[index] = size;
}

struct mdr_struct_s *mdr_get_pbb(void)
{
	return pbb;
}

struct mdr_struct_s *mdr_get_tmppbb(void)
{
	return tmp_pbb;
}

void mdr_clear_tmppbb(void)
{
	if (tmp_pbb) {
		vfree(tmp_pbb);
		tmp_pbb = NULL;
	}
}

u64 mdr_get_pbb_size(void)
{
	return mdr_reserved_phymem_size();
}

int mdr_get_areainfo(enum MDR_AREA_LIST area,
		     struct mdr_register_module_result *retinfo)
{
	if (area >= MDR_AREA_MAXIMUM)
		return -1;

	retinfo->log_addr = pbb->area_info[area].offset;
	retinfo->log_len = pbb->area_info[area].length;

	return 0;
}

void mdr_field_baseinfo_init(void)
{
	pbb->base_info.modid = 0;
	pbb->base_info.arg1 = 0;
	pbb->base_info.arg2 = 0;
	pbb->base_info.e_core = 0;
	pbb->base_info.e_type = 0;
	pbb->base_info.e_subtype = 0;
	pbb->base_info.start_flag = 0;
	pbb->base_info.savefile_flag = 0;
	pbb->base_info.reboot_flag = 0;
	memset(pbb->base_info.e_module, 0, MODULE_NAME_LEN);
	memset(pbb->base_info.e_desc, 0, STR_EXCEPTIONDESC_MAXLEN);
	memset(pbb->base_info.datetime, 0, DATATIME_MAXLEN);
}

void mdr_field_baseinfo_reinit(void)
{
	MDR_PRINT_START();
	pbb->base_info.modid = 0;
	pbb->base_info.arg1 = 0;
	pbb->base_info.arg2 = 0;
	pbb->base_info.e_core = 0;
	pbb->base_info.e_type = 0;
	pbb->base_info.e_subtype = 0;
	pbb->base_info.start_flag = MDR_PROC_EXEC_START;
	pbb->base_info.savefile_flag = MDR_DUMP_LOG_START;
	memset(pbb->base_info.datetime, 0, DATATIME_MAXLEN);

	MDR_PRINT_END();
}
EXPORT_SYMBOL(mdr_field_baseinfo_reinit);

void mdr_field_areainfo_init(void)
{
	int index;

	for (index = 0; index < MDR_AREA_MAXIMUM; index++) {
		pbb->area_info[index].offset = g_mdr_area_mem_addr[index];
		pbb->area_info[index].length = g_mdr_area_mem_size[index];
	}
}

char *mdr_field_get_datetime(void)
{
	return (char *)(pbb->base_info.datetime);
}

void mdr_field_dumplog_done(void)
{
	pbb->base_info.savefile_flag = MDR_DUMP_LOG_DONE;
}

void mdr_field_procexec_done(void)
{
	pbb->base_info.start_flag = MDR_PROC_EXEC_DONE;
}

void mdr_field_reboot_done(void)
{
	pbb->base_info.reboot_flag = MDR_REBOOT_DONE;
}

void mdr_field_top_init(void)
{
	pbb->top_head.magic = FILE_MAGIC;
	pbb->top_head.version = MDR_VERSION;
	pbb->top_head.area_number = MDR_AREA_MAXIMUM;

	mdr_get_builddatetime(pbb->top_head.build_time);
	memcpy(pbb->top_head.product_name, MDR_PRODUCT, strlen(MDR_PRODUCT) > 16 ? 16 : strlen(MDR_PRODUCT));
	memcpy(pbb->top_head.product_version, MDR_PRODUCT_VERSION,
	       strlen(MDR_PRODUCT_VERSION) > 16 ? 16 : strlen(MDR_PRODUCT_VERSION));
}

int mdr_field_init(void)
{
	int ret = 0;
	int index;
	u32 last = 0;
	unsigned int fpga_flag = 0;
	struct device_node *np;

	g_data = get_mdrdata();

	pbb = vmalloc(mdr_reserved_phymem_size());
	if (pbb == NULL) {
		MDR_PRINT_ERR("hisi_bbox_map pbb faild.");
		ret = -1;
		goto out;
	}

	tmp_pbb = vmalloc(mdr_reserved_phymem_size());
	if (tmp_pbb == NULL) {
		MDR_PRINT_ERR("vmalloc tmp_pbb faild.");
		ret = -1;
		mdr_unmap(pbb);
		pbb = NULL;
		goto out;
	}

	mdr_show_base_info(1);	/* show pbb */
	memcpy(tmp_pbb, pbb, mdr_reserved_phymem_size());
	mdr_show_base_info(0);	/* show tmpbb */

	np = of_find_compatible_node(NULL, NULL, "xring,mdr");
	if (!np) {
		MDR_PRINT_ERR("NOT FOUND device node 'xring,mdr'!\n");
		return -ENXIO;
	}
	ret = of_property_read_u32(np, "fpga_flag", &fpga_flag);
	if (ret) {
		MDR_PRINT_ERR("failed to get fpga_flag resource.\n");
		return -ENXIO;
	}

	/* if the power_up of phone is the first time,
	 * need clear reserved memory.
	 */
	if ((mdr_get_reboot_type() == AP_S_COLDBOOT) && (fpga_flag != 1))
		memset(pbb, 0, mdr_reserved_phymem_size());
	else
		memset(pbb, 0, MDR_BASEINFO_SIZE);

	last = g_mdr_area_mem_size[0];
	g_mdr_area_mem_addr[last] = mdr_reserved_phymem_addr() + mdr_reserved_phymem_size();
	for (index = last - 1; index > 0; index--)
		g_mdr_area_mem_addr[index] = g_mdr_area_mem_addr[index + 1] - g_mdr_area_mem_size[index];

	g_mdr_area_mem_addr[0] = mdr_reserved_phymem_addr() + MDR_BASEINFO_SIZE;
	g_mdr_area_mem_size[0] = g_mdr_area_mem_addr[1] - MDR_BASEINFO_SIZE - mdr_reserved_phymem_addr();

	mdr_field_top_init();
	mdr_field_baseinfo_init();
	mdr_field_areainfo_init();
out:
	return ret;
}

void mdr_save_args(u32 modid, u32 arg1, u32 arg2)
{
	MDR_PRINT_START();
	pbb->base_info.modid = modid;
	pbb->base_info.arg1 = arg1;
	pbb->base_info.arg2 = arg2;

	MDR_PRINT_END();
}
EXPORT_SYMBOL(mdr_save_args);

void mdr_fill_edata(struct mdr_exception_info_s *e, const char *date)
{
	MDR_PRINT_START();
	pbb->base_info.e_core = e->e_from_core;
	pbb->base_info.e_type = e->e_exce_type;
	pbb->base_info.e_subtype = e->e_exce_subtype;
	memcpy(pbb->base_info.datetime, date, DATATIME_MAXLEN);
	memcpy(pbb->base_info.e_module, e->e_from_module, MODULE_NAME_LEN);
	memcpy(pbb->base_info.e_desc, e->e_desc, STR_EXCEPTIONDESC_MAXLEN);
	MDR_PRINT_END();
}

void mdr_show_base_info(int flag)
{
	struct mdr_struct_s *p = NULL;
	int index;

	if (flag == 1)
		p = mdr_get_pbb();
	else
		p = mdr_get_tmppbb();

	if (p == NULL)
		return;

	if (p->top_head.magic != FILE_MAGIC)
		return;

	p->base_info.datetime[DATATIME_MAXLEN - 1] = '\0';
	p->base_info.e_module[MODULE_NAME_LEN - 1] = '\0';
	p->base_info.e_desc[STR_EXCEPTIONDESC_MAXLEN - 1] = '\0';
	p->top_head.build_time[MDR_BUILD_DATE_TIME_LEN - 1] = '\0';

	pr_info("========= print baseinfo start =========\n");
	pr_info("modid        :[0x%x]\n", p->base_info.modid);
	pr_info("arg1         :[0x%x]\n", p->base_info.arg1);
	pr_info("arg2         :[0x%x]\n", p->base_info.arg2);
	pr_info("coreid       :[0x%x]\n", p->base_info.e_core);
	pr_info("reason       :[0x%x]\n", p->base_info.e_type);
	pr_info("subtype      :[0x%x]\n", p->base_info.e_subtype);
	pr_info("e data       :[%s]\n", p->base_info.datetime);
	pr_info("e module     :[%s]\n", p->base_info.e_module);
	pr_info("e desc       :[%s]\n", p->base_info.e_desc);
	pr_info("e start_flag :[%d]\n", p->base_info.start_flag);
	pr_info("e save_flag  :[%d]\n", p->base_info.savefile_flag);
	pr_info("========= print baseinfo e n d =========\n");

	pr_info("========= print top head start =========\n");
	pr_info("maigc        :[0x%x]\n", p->top_head.magic);
	pr_info("version      :[0x%x]\n", p->top_head.version);
	pr_info("area num     :[0x%x]\n", p->top_head.area_number);
	pr_info("reserve      :[0x%x]\n", p->top_head.reserve);
	pr_info("buildtime    :[%s]\n",   p->top_head.build_time);
	pr_info("========= print top head e n d =========\n");

	pr_info("========= print areainfo start =========\n");
	for (index = 0; index < MDR_AREA_MAXIMUM; index++) {
		pr_info("area[%d] addr[0x%llx] size[0x%x]\n",
			index, pbb->area_info[index].offset,
			pbb->area_info[index].length);
	}
	pr_info("========= print areainfo e n d =========\n");
}

MODULE_LICENSE("GPL");
