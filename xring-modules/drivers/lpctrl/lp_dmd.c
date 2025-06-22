// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <linux/of.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include "ipc_resource.h"
#include "ipc_tags_lpctrl_ap.h"
#include "ipc_cmds_lpctrl_ap.h"
#else
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_lpctrl_ap.h>
#endif
#include <soc/xring/securelib/securec.h>
#include <soc/xring/securelib/securectype.h>
#include <soc/xring/dmd.h>
#include "lp_dmd.h"

#ifdef LP_DMD_DEBUG
#define lp_dmd_debug(fmt, ...)         \
	pr_err("[%s][E]: " fmt, __func__, ##__VA_ARGS__)
#else
#define lp_dmd_debug(fmt, ...)
#endif

#define VC_ID_FROM_LP	IPC_VC_PERI_NS_AP_LP_COMMON_1
#define LP_DMD_BUFF_SIZE	512
#define LP_DMD_INFO_SIZE	200
#define MAX_REG_GET	8

struct lp_dmd_data {
	struct dmd_client *client;
	struct dmd_dev device;
	int info_len;
	char info[LP_DMD_INFO_SIZE];
};

static struct lp_dmd_data g_lp_dmd_list[] = {
	[CMD_LP_DMD_PLL_ERR] = {
		.client = NULL,
		.device = {
			.name = "pll",
			.module_name = "lpcore",
			.buff_size = LP_DMD_BUFF_SIZE,
			.fault_level = "CRITICAL",
		},
		.info_len = 0,
	},
	[CMD_LP_DMD_PMU_ERR] = {
		.client = NULL,
		.device = {
			.name = "cpmic",
			.module_name = "lpcore",
			.buff_size = LP_DMD_BUFF_SIZE,
			.fault_level = "CRITICAL",
		},
		.info_len = 0,
	},
};

static struct work_struct lpctrl_work;

static void lpcore_work_func(struct work_struct *work)
{
	struct lp_dmd_data *dmd_data = NULL;

	lp_dmd_debug("lpcore dmd work run\n");

	for (int i = 0; i < ARRAY_SIZE(g_lp_dmd_list); i++) {
		dmd_data = &g_lp_dmd_list[i];
		if (!dmd_data->client)
			continue;
		if (dmd_data->info_len > 0) {
			dmd_client_record(dmd_data->client, DMD_LPCORE_ERROR_NO | i, dmd_data->info);
			dmd_data->info_len = 0;
		}
	}
}

static int lpcore_dmd_handle(struct ipc_pkt *pkt, u16 ipc_tag, u8 cmd, void *arg)
{
	int ret;
	int size;
	int ipc_data[MAX_REG_GET];
	struct lp_dmd_data *dmd_data = NULL;

	if (pkt == NULL)
		return -EINVAL;

	lp_dmd_debug(" ipc_tag:%u cmd:%u len:%d\n", ipc_tag, cmd, pkt->header.data_len);

	if (cmd >= ARRAY_SIZE(g_lp_dmd_list)) {
		pr_err("lpcore wrong dmd cmd:%d\n", cmd);
		return -EINVAL;
	}
	dmd_data = &g_lp_dmd_list[cmd];
	if (!dmd_data->client) {
		pr_err("lpcore dmd not init\n");
		return -EINVAL;
	}

	ret = memcpy_s((void *)ipc_data, MAX_REG_GET * sizeof(int),
			pkt->data, pkt->header.data_len * sizeof(char));
	if (ret != 0) {
		pr_err("lpcore dmd memcpy fail:%d\n", ret);
		return ret;
	}

	size = 0;
	size += snprintf(dmd_data->info + size, LP_DMD_INFO_SIZE - size, "type=%d ", ipc_data[0]);
	for (int i = 1; i < pkt->header.data_len / sizeof(int); i++) {
		size += snprintf(dmd_data->info + size, LP_DMD_INFO_SIZE - size,
				 "REG[%d]=%d ", i, ipc_data[i]);

		if (size >= LP_DMD_INFO_SIZE) {
			pr_warn("lpcore dmd buffer truncated:%d\n", size);
			size = LP_DMD_INFO_SIZE - 1;
			break;
		}
	}

	size += snprintf(dmd_data->info + size, LP_DMD_INFO_SIZE - size, "%s", "\0");
	dmd_data->info_len = size;

	lp_dmd_debug("lp dmd info[%d]:%s\n", dmd_data->info_len, dmd_data->info);

	queue_work(system_wq, &lpctrl_work);

	return 0;
}

void lpcore_dmd_init(void)
{
	int ret;
	struct lp_dmd_data *dmd_data = NULL;

	lp_dmd_debug("lpcore dmd driver init\n");
	ret = ap_ipc_recv_register(VC_ID_FROM_LP, TAG_LP_DMD, lpcore_dmd_handle, NULL);
	if (ret != 0) {
		pr_err("lpcore dmd ipc register fail:%d\n", ret);
		return;
	}

	INIT_WORK(&lpctrl_work, lpcore_work_func);

	for (int i = 0; i < ARRAY_SIZE(g_lp_dmd_list); i++) {
		dmd_data = &g_lp_dmd_list[i];
		dmd_data->client = dmd_register_client(&dmd_data->device);
		if (!dmd_data->client)
			pr_err("lpcore register dmd %d failed\n", i);
	}
}
EXPORT_SYMBOL_GPL(lpcore_dmd_init);

void lpcore_dmd_exit(void)
{
	struct lp_dmd_data *dmd_data = NULL;

	ap_ipc_recv_unregister(VC_ID_FROM_LP, TAG_LP_DMD, lpcore_dmd_handle);

	for (int i = 0; i < ARRAY_SIZE(g_lp_dmd_list); i++) {
		dmd_data = &g_lp_dmd_list[i];
		if (dmd_data->client)
			dmd_unregister_client(dmd_data->client, &dmd_data->device);
	}
}
EXPORT_SYMBOL_GPL(lpcore_dmd_exit);

MODULE_SOFTDEP("pre: dmd");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dmd driver for lp");
MODULE_AUTHOR("xiaomi.com");