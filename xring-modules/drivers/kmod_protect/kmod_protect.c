// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 * Description: kernel module protect driver
 * Modify time: 2024-08-21
 */
#include <asm/memory.h>
#include <asm/barrier.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <linux/arm-smccc.h>
#include <trace/hooks/module.h>
#include <asm/page-def.h>
#include <linux/mm.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>
#include <linux/kprobes.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/xhee_status.h>

/*
 * the size of struct kmod_section_info should be page aligned
 * (8 * MAX_KMOD_SEC_ITEM + MODULE_NAME_SIZE + 8) should be multiple of page size
 */
#define MAX_KMOD_SEC_ITEM 2039ULL
#define MODULE_NAME_SIZE  64

static DEFINE_SPINLOCK(kmod_buffer_lock);

struct kmod_section_info {
	uint64_t page_num;
	uint64_t page_start[MAX_KMOD_SEC_ITEM];
	char name[MODULE_NAME_SIZE];
};

static struct kmod_section_info *sect_buf[MOD_MEM_NUM_TYPES] = { 0ULL };
static uint64_t buf_addr[MOD_MEM_NUM_TYPES] = { 0ULL };

static const char * const sect_names[] = { "MOD_TEXT", "MOD_DATA", "MOD_RODATA", "MOD_RO_AFTER_INIT",
	"MOD_INIT_TEXT", "MOD_INIT_DATA", "MOD_INIT_RODATA" };

struct kprobe kmod_kallsym_kp = { .symbol_name = "kallsyms_lookup_name" };
/* Stores address to kernel function 'kallsyms_lookup_name'
 * as 'kallsyms_lookup_name' is no longer exported from 5.7 kernel.
 */
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t kmod_ksym_lookup_name;

static void kmod_register_ksym_probe(void)
{
	if (!(register_kprobe(&kmod_kallsym_kp))) {
		/* After kernel 5.7, 'kallsyms_lookup_name' is no longer
		 * exported. So we need this workaround to get the
		 * addr of 'kallsyms_lookup_name'. This will be used later
		 * in kprobe handler function to call the registered
		 * handler for a probe from the name passed from userspace.
		 */
		kmod_ksym_lookup_name = (kallsyms_lookup_name_t)kmod_kallsym_kp.addr;
		unregister_kprobe(&kmod_kallsym_kp);
	} else
		pr_info("kallsyms_lookup_name addr not available\n");
}

static void transfer_to_xhee(uint64_t func_id, const struct module *mod)
{
	int i;
	int cur;
	uint64_t addr;
	struct arm_smccc_res res = { 0 };
	struct kmod_section_info *cur_sect = NULL;
	int ret;

	cur = get_cpu();
	/* make sure the module addr info is complete before use it */
	smp_mb();
	for_each_mod_mem_type(type) {
		const struct module_memory *mod_mem = &mod->mem[type];

		cur_sect = sect_buf[type];
		cur_sect->page_num = 0ULL;
		/* store module name */
		ret = strncpy_s(cur_sect->name, MODULE_NAME_LEN, mod->name, strlen(mod->name));
		if (ret != 0)
			pr_err("kmod_protect fail to store mod name %s\n", mod->name);

		if (mod_mem->size == 0)
			continue;

		if (mod_mem->size % PAGE_SIZE != 0) {
			pr_err("kmod_protect mem size error! %llx %s\n", func_id, mod->name);
			continue;
		}

		addr = (uint64_t)mod_mem->base;
		if (addr % PAGE_SIZE != 0) {
			pr_err("kmod_protect mem addr error! %llx %s\n", func_id, mod->name);
			continue;
		}
		cur_sect->page_num = mod_mem->size / PAGE_SIZE;
		if (cur_sect->page_num > MAX_KMOD_SEC_ITEM) {
			pr_err("kmod_protect error! %llx %s, cur secs:0x%llx max secs:0x%llx\n",
				func_id, mod->name, cur_sect->page_num, MAX_KMOD_SEC_ITEM);
			cur_sect->page_num = 0;
			continue;
		}
		for (i = 0; i < cur_sect->page_num; i++) {
			cur_sect->page_start[i] = vmalloc_to_pfn((void *)addr) << PAGE_SHIFT;
			addr += PAGE_SIZE;
		}
	}

	arm_smccc_hvc(func_id, buf_addr[MOD_TEXT], buf_addr[MOD_DATA], buf_addr[MOD_RODATA],
			buf_addr[MOD_RO_AFTER_INIT], buf_addr[MOD_INIT_TEXT],
			buf_addr[MOD_INIT_DATA], buf_addr[MOD_INIT_RODATA], &res);
	/* make sure the hvc return value is complete before use it */
	smp_mb();
	put_cpu();
	if (res.a0 != 0)
		pr_err("mod mem transfer to xhee error! %s\n", mod->name);
}

static void probe_free_mod_mem(void *unused, const struct module *mod)
{
	spin_lock(&kmod_buffer_lock);
	transfer_to_xhee(FID_KMOD_FR_MOD, mod);
	spin_unlock(&kmod_buffer_lock);
}

static void probe_set_mod_before_init(void *unused, const struct module *mod)
{
	spin_lock(&kmod_buffer_lock);
	transfer_to_xhee(FID_KMOD_BE_INIT, mod);
	spin_unlock(&kmod_buffer_lock);
}

static void kmod_get_kdata_secs(phys_addr_t *sdata_phys, phys_addr_t *edata_phys, phys_addr_t *rodata_start_phys,
		phys_addr_t *rodata_end_phys, phys_addr_t *rodata_af_start_phys, phys_addr_t *rodata_af_end_phys,
		phys_addr_t *text_start_phys, phys_addr_t *text_end_phys, phys_addr_t *init_stext_phys,
		phys_addr_t *init_etext_phys)
{
	unsigned long sdata_addr;
	unsigned long edata_addr;
	unsigned long rodata_start;
	unsigned long rodata_end;
	unsigned long rodata_af_start;
	unsigned long rodata_af_end;
	unsigned long text_start;
	unsigned long text_end;
	unsigned long init_text_start;
	unsigned long init_text_end;

	sdata_addr = kmod_ksym_lookup_name("_sdata");
	edata_addr = kmod_ksym_lookup_name("_edata");
	rodata_start = kmod_ksym_lookup_name("__start_rodata");
	rodata_end = kmod_ksym_lookup_name("__end_rodata");
	rodata_af_start = kmod_ksym_lookup_name("__start_ro_after_init");
	rodata_af_end = kmod_ksym_lookup_name("__end_ro_after_init");
	text_start = kmod_ksym_lookup_name("_stext");
	text_end = kmod_ksym_lookup_name("_etext");
	init_text_start = kmod_ksym_lookup_name("_sinittext");
	init_text_end = kmod_ksym_lookup_name("_einittext");
	*sdata_phys = __pa(sdata_addr);
	*edata_phys = __pa(edata_addr);
	*rodata_start_phys = __pa(rodata_start);
	*rodata_end_phys = __pa(rodata_end);
	*rodata_af_start_phys = __pa(rodata_af_start);
	*rodata_af_end_phys = __pa(rodata_af_end);
	*text_start_phys = __pa(text_start);
	*text_end_phys = __pa(text_end);
	*init_stext_phys = __pa(init_text_start);
	*init_etext_phys = __pa(init_text_end);
}

static void probe_set_mod_after_init(void *unused, const struct module *mod)
{
	struct arm_smccc_1_2_regs args = { 0 };
	struct arm_smccc_1_2_regs res = { 0 };
	phys_addr_t sdata;
	phys_addr_t edata;
	phys_addr_t ro_start;
	phys_addr_t ro_end;
	phys_addr_t ro_af_start;
	phys_addr_t ro_af_end;
	phys_addr_t text_start;
	phys_addr_t text_end;
	phys_addr_t init_stext;
	phys_addr_t init_etext;

	if (mod == THIS_MODULE) {
		kmod_get_kdata_secs(&sdata, &edata, &ro_start, &ro_end, &ro_af_start, &ro_af_end, &text_start,
				&text_end, &init_stext, &init_etext);
		args.a0 = FID_KMOD_LOADED;
		args.a1 = (uint64_t)sdata;
		args.a2 = (uint64_t)edata;
		args.a3 = (uint64_t)ro_start;
		args.a4 = (uint64_t)ro_end;
		args.a5 = (uint64_t)ro_af_start;
		args.a6 = (uint64_t)ro_af_end;
		args.a7 = (uint64_t)text_start;
		args.a8 = (uint64_t)text_end;
		args.a9 = (uint64_t)init_stext;
		args.a10 = (uint64_t)init_etext;
		arm_smccc_1_2_hvc(&args, &res);
	}
	spin_lock(&kmod_buffer_lock);
	transfer_to_xhee(FID_KMOD_AF_INIT, mod);
	spin_unlock(&kmod_buffer_lock);
}

static int __init kmod_protect_init(void)
{
	int ret;

	if (!xhee_status_enabled()) {
		pr_notice("xhee is disabled!\n");
		return 0;
	}

	kmod_register_ksym_probe();
	for_each_mod_mem_type(type) {
		sect_buf[type] = kmalloc(sizeof(struct kmod_section_info), GFP_KERNEL);
		if (sect_buf[type] == NULL)
			return -1;
		buf_addr[type] = (uint64_t)virt_to_phys((void *)sect_buf[type]);
	}

	ret = register_trace_android_vh_free_mod_mem(probe_free_mod_mem, NULL);
	if (ret < 0) {
		pr_err("kmod_protect free_mod_mem register fail, ret %d.\n", ret);
		return ret;
	}

	ret = register_trace_android_vh_set_mod_perm_before_init(probe_set_mod_before_init, NULL);
	if (ret < 0) {
		pr_err("kmod_protect set_mod_perm_before_init fail, ret %d.\n", ret);
		return ret;
	}

	ret = register_trace_android_vh_set_mod_perm_after_init(probe_set_mod_after_init, NULL);
	if (ret < 0) {
		pr_err("kmod_protect set_mod_perm_after_init fail, ret %d.\n", ret);
		return ret;
	}
	pr_notice("kmod_protect module init success\n");

	return 0;
}

/* This is called when the module is removed. */
static void __exit kmod_protect_exit(void)
{
}

MODULE_DESCRIPTION("XRing kmod protect driver");
MODULE_LICENSE("GPL v2");
module_init(kmod_protect_init);
module_exit(kmod_protect_exit);
