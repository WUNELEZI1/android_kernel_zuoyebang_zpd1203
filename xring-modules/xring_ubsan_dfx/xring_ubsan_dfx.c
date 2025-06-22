// SPDX-License-Identifier: GPL-2.0-only
/*
 * ubsan dfx enhance
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#define GLOBAL_ARR_OOB_BRK_IMM	0x5512
#define DIV_ZERO_BRK_IMM	0x5503
#define SHIFT_OOB_BRK_IMM	0x5514
#define UNREACHABLE_BRK_IMM	0x5501
#define BOOL_LOAD_BRK_IMM	0x550a
#define STACK_ARR_OOB_BRK_IMM	0x1

struct imm_str {
	unsigned long imm;
	char *str;
};

static struct imm_str str_map[] = {
	{GLOBAL_ARR_OOB_BRK_IMM,	"maybe global array oob"},
	{DIV_ZERO_BRK_IMM,		"maybe div zero"},
	{SHIFT_OOB_BRK_IMM,		"maybe shift oob"},
	{UNREACHABLE_BRK_IMM,		"maybe unreachable branch"},
	{BOOL_LOAD_BRK_IMM,		"maybe non-bool value loaded"},
	{STACK_ARR_OOB_BRK_IMM,		"maybe stack array oob"}
};

static int ubsan_dfx_brk_handler(struct pt_regs *regs, unsigned long esr)
{
	unsigned long imm = esr & 0xffff;
	char *str;
	int i, map_size;

	str = "unexpected imm in ubsan dfx";
	map_size = ARRAY_SIZE(str_map);
	for (i = 0; i < map_size; i++) {
		if (imm == str_map[i].imm) {
			str = str_map[i].str;
			break;
		}
	}
	pr_err("%s", str);

	return 1;
}

static struct break_hook ubsan_dfx_hook_arr[] = {
	{
		.imm = GLOBAL_ARR_OOB_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	},
	{
		.imm = DIV_ZERO_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	},
	{
		.imm = SHIFT_OOB_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	},
	{
		.imm = UNREACHABLE_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	},
	{
		.imm = BOOL_LOAD_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	},
	{
		.imm = STACK_ARR_OOB_BRK_IMM,
		.fn = ubsan_dfx_brk_handler,
	}
};

static int __init xring_ubsan_dfx_init(void)
{
	int i, size;

	size = ARRAY_SIZE(ubsan_dfx_hook_arr);
	for (i = 0; i < size; i++)
		register_kernel_break_hook(&ubsan_dfx_hook_arr[i]);

	return 0;
}
static void __exit xring_ubsan_dfx_exit(void)
{
	int i, size;

	size = ARRAY_SIZE(ubsan_dfx_hook_arr);
	for (i = 0; i < size; i++)
		unregister_kernel_break_hook(&ubsan_dfx_hook_arr[i]);
}

module_init(xring_ubsan_dfx_init);
module_exit(xring_ubsan_dfx_exit);
MODULE_LICENSE("GPL");
