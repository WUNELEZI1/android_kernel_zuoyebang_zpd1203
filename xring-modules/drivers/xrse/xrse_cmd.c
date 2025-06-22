// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#include "xrse_internal.h"
#include "xrse_cmd.h"
#include <linux/list.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <soc/xring/securelib/securec.h>

static char s_xrse_res_buf[XRSE_RES_BUF_LENGTH_MAX];

static int s_xrse_res_length;

#if defined CONFIG_XRING_XRSE_SECBOOT && IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
extern struct xrse_module_cmd secboot_module;
#endif

struct xrse_module_cmd *g_xrse_cmd[] = {
#if defined CONFIG_XRING_XRSE_SECBOOT && IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	&secboot_module,
#endif
};

static int xrse_exec_case_store(struct xrse_module_cmd *module, char *argv_content[], int argv_num)
{
	int i;
	int ret = -EINVAL;
	int case_num = module->num_cases;
	struct module_case *my_case = module->cases;

	/* Step1. Find the right case. */
	for (i = 0; i < case_num; i++, my_case++) {
		ret = strncmp(my_case->name, argv_content[0], XRSE_PER_CMD_LENGTH_MAX);
		if (ret)
			continue;

		return my_case->store_handle(argv_num-1, argv_content+1);
	}

	xrse_err("invalid case name\n");

	return -EINVAL;
}

int xrse_parse_cmd_store(const char *buff, u32 buff_size)
{
	int i = 0;
	int argc = 0;
	int ret = -EINVAL;
	char *parse_pos = kmalloc(buff_size + 1, GFP_KERNEL);
	struct xrse_module_cmd *module_tmp = NULL;
	char *argv_content[XRSE_PER_CMD_LENGTH_MAX];

	if (parse_pos == NULL) {
		xrse_err("malloc cmd buffer failed\n");
		return -ENOMEM;
	}

	(void)memset_s(s_xrse_res_buf, XRSE_RES_BUF_LENGTH_MAX, 0, XRSE_RES_BUF_LENGTH_MAX);

	(void)memcpy_s(parse_pos, buff_size, buff, buff_size);

	/* Step1. Parse cmd line into: [module cmd params] */
	while (i < buff_size) {
		while (i < buff_size && (parse_pos[i] == ' '))
			i++;

		if (i < buff_size) {
			if (argc >= XRSE_PER_CMD_LENGTH_MAX) {
				xrse_err("num %d over %d\n", argc, XRSE_PER_CMD_LENGTH_MAX);
				goto free;
			}

			argv_content[argc++] = &parse_pos[i];

			while (i < buff_size && (parse_pos[i] != ' '))
				i++;

			if (i < buff_size) {
				parse_pos[i] = '\0';
				i++;
			}
		}
	}

	parse_pos[i] = '\0';

	/* Step2. Get the module name and dispatch to handle */
	for (i = 0; i < ARRAYSIZE(g_xrse_cmd); i++) {
		ret = strncmp(g_xrse_cmd[i]->module_name, argv_content[0], XRSE_PER_CMD_LENGTH_MAX);
		if (ret)
			continue;

		module_tmp = g_xrse_cmd[i];

		ret = xrse_exec_case_store(module_tmp, argv_content + 1, argc - 1);
		if (ret)
			xrse_err("xrse_exec_case_store fail\n");
		else
			xrse_info("xrse_exec_case_store success\n");

		goto free;
	}

	xrse_err("invalid cmd:%s\n", argv_content[0]);
	ret = -EINVAL;

free:
	kfree(parse_pos);
	parse_pos = NULL;

	return ret;
}

int xrse_cmd_res_store(char *buf, int buf_size)
{
	int ret = -EINVAL;

	if (buf_size > (XRSE_RES_BUF_LENGTH_MAX - 1)) { /* reserve 1byte for '\0' */
		xrse_err("res buf_size:%d failed\n", buf_size);
		return -EINVAL;
	}

	s_xrse_res_length = buf_size;

	ret = strcpy_s(s_xrse_res_buf, XRSE_RES_BUF_LENGTH_MAX, buf);
	if (ret) {
		xrse_err("res strcpy_s failed\n");
		return ret;
	}

	xrse_info("res store success\n");

	return ret;
}

int xrse_cmd_res_show(char *buf)
{
	int ret = -EINVAL;

	xrse_debug("cmd res show:%s\n", s_xrse_res_buf);
	ret = strcpy_s(buf, s_xrse_res_length + 1, s_xrse_res_buf);
	if (ret) {
		xrse_err("res strcpy_s failed. ret:%d\n", ret);
		return -EINVAL;
	}

	xrse_debug("after copy res:%s\n", buf);
	xrse_info("res show success\n");

	return s_xrse_res_length + 1;
}
