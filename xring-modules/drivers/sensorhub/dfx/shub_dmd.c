// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)            "[shub_dmd]:" fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <soc/xring/dmd.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dfx.h>
#include "shub_dfx_priv.h"

#define SHUB_DMD_MODULE_NAME             "SHUB"
#define SHUB_DMD_FAULT_LEVEL_CRITICAL    "CRITICAL"
#define SHUB_DMD_FAULT_LEVEL_WARNING     "WARNING"
#define SHUB_DMD_FAULT_LEVEL_INFO        "INFO"
#define SHUB_DMD_DATA_LEN_MAX            (64)
#define SHUB_DMD_REGS_LEN_MAX            (16)
#define SHUB_DMD_IPC_LEN_MIN             (16)
#define SHUB_DMD_BUFF_LEN_MAX            (256)
#define MAX_UINT32_HEX_NUMBER            (10)

struct sh_dmd_member {
	const uint32_t fault_id;
	const char data[SHUB_DMD_DATA_LEN_MAX];
};

static struct sh_dmd_member g_sh_dmd_array[] = {
	[0] = {
			.fault_id = SHUB_ID_DFX_TEST,
			.data     = "shub dmd err, fault0:?, fault1:?, pc:?",
	},
};

static struct dmd_dev g_sh_dmd_dev[] = {
	[0] = {
		.name        = "shub_dmd_test",
		.module_name = SHUB_DMD_MODULE_NAME,
		.buff_size   = SHUB_DMD_BUFF_LEN_MAX,
		.fault_level = SHUB_DMD_FAULT_LEVEL_CRITICAL,
	},
};

static struct dmd_client *g_sh_dmd_client[] = {NULL,};

int sh_dmd_register(void)
{
	uint32_t i;
	uint32_t size_dmd_dev, size_dmd_array;

	size_dmd_dev = ARRAY_SIZE(g_sh_dmd_dev);
	size_dmd_array = ARRAY_SIZE(g_sh_dmd_array);
	if (size_dmd_dev != size_dmd_array) {
		pr_err("para check err, size_dmd_dev=%u, size_dmd_array=%u.\n", size_dmd_dev, size_dmd_array);
		return -1;
	}

	pr_info("size_dmd_dev=%u.\n", size_dmd_dev);
	for (i = 0; i < size_dmd_dev; i++) {
		g_sh_dmd_client[i] = dmd_register_client(&g_sh_dmd_dev[i]);
		if (!g_sh_dmd_client[i]) {
			pr_err("regist fail, index=%u.\n", i);
			continue;
		}
	}

	return 0;
}

int sh_dmd_unregister(void)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(g_sh_dmd_dev); i++)
		dmd_unregister_client(g_sh_dmd_client[i], &g_sh_dmd_dev[i]);

	return 0;
}

static char *sh_dmd_form_str(const char str[], uint32_t *data_ptr, uint16_t data_num)
{
	char *str_res;
	char *str_part;
	char str_reg[SHUB_DMD_REGS_LEN_MAX] = {0};
	char str_copy[SHUB_DMD_DATA_LEN_MAX] = {0};
	char *str_base = str_copy;
	uint16_t index = 0;
	errno_t ret;

	if ((!str) || (strlen(str) == 0) || (!data_ptr) || (data_num == 0)) {
		pr_err("form str input err.\n");
		return NULL;
	}

	ret = memcpy_s(str_copy, SHUB_DMD_DATA_LEN_MAX, str, strlen(str));
	if (ret != EOK) {
		pr_err("form str memcpy err=%d.\n", ret);
		return NULL;
	}

	pr_debug("form str_copy=%s, len=%lu.\n", str_copy, strlen(str_copy));
	for (int i = 0; i < data_num; i++)
		pr_debug("form str num[%d]=%u.\n", i, *(data_ptr + i));

	str_res = kzalloc(SHUB_DMD_BUFF_LEN_MAX, GFP_ATOMIC);
	if (!str_res) {
		pr_err("form str zalloc err.\n");
		return NULL;
	}

	str_part = strsep(&str_base, "?");
	pr_debug("form str zalloc success, str_part=%s.\n", str_part);

	while ((str_part != NULL) && (strlen(str_part) != 0)) {
		ret = strcat_s(str_res, SHUB_DMD_BUFF_LEN_MAX, str_part);
		if (ret != EOK) {
			pr_err("form str strcat1 err=%d.\n", ret);
			goto _err;
		}
		pr_debug("strcat1 str_res=%s.\n", str_res);
		if (index < data_num) {
			ret = memset_s(str_reg, sizeof(str_reg), 0, sizeof(str_reg));
			if (ret != EOK) {
				pr_err("form str strcat2 err=%d.\n", ret);
				goto _err;
			}
			ret = snprintf_s(str_reg, SHUB_DMD_REGS_LEN_MAX, MAX_UINT32_HEX_NUMBER, "0x%x", *(data_ptr + index));
			if (ret == -1) {
				pr_err("form str snprintf err=%d.\n", ret);
				goto _err;
			}
			pr_debug("snprintf str_reg=%s.\n", str_reg);
			ret = strcat_s(str_res, SHUB_DMD_BUFF_LEN_MAX, str_reg);
			if (ret != EOK) {
				pr_err("form str strcat2 err=%d.\n", ret);
				goto _err;
			}
			pr_debug("strcat2 str_res=%s.\n", str_res);
		}
		str_part = strsep(&str_base, "?");
		pr_debug("strsep str_part=%s.\n", str_part);
		index++;
	}
	pr_debug("form str end str=%s.\n", str_res);
	return str_res;

_err:
	kfree(str_res);
	return NULL;
}

void sh_dmd_report(struct ipc_pkt *pkt)
{
	uint32_t index;
	char *str_dmd;
	char *ipc_body;
	uint16_t data_len;
	uint16_t data_num;
	uint32_t *ptr_data;
	uint32_t data_offset;
	uint32_t size_dmd_array = ARRAY_SIZE(g_sh_dmd_array);
	struct shub_dfx_alarm_head *head;

	if (pkt == NULL) {
		pr_err("report get null pkt.\n");
		return;
	}

	ipc_body = (char *)pkt + sizeof(struct ipc_pkt_header);
	data_len = pkt->header.data_len;
	if (data_len < SHUB_DMD_IPC_LEN_MIN) {
		pr_err("ipc valid data_len=%u too short.\n", data_len);
		return;
	}

	ptr_data = (uint32_t *)ipc_body;
	head = (struct shub_dfx_alarm_head *)ipc_body;
	data_offset = sizeof(struct shub_dfx_alarm_head) / sizeof(uint32_t);
	data_num = data_len / sizeof(uint32_t) - data_offset;

	pr_info("fault_id=%u, time-low=%u, time-high=%u, num=%u\n",
		head->fault_id, head->time_low, head->time_high, data_num);

	for (index = 0; index < size_dmd_array; index++) {
		if (head->fault_id == g_sh_dmd_array[index].fault_id) {
			pr_debug("match fault_id, index=%u, orig_str=%s.\n", index, g_sh_dmd_array[index].data);
			str_dmd = sh_dmd_form_str(g_sh_dmd_array[index].data, (ptr_data + data_offset), data_num);
			if (str_dmd != NULL) {
				pr_debug("form_str=%s.\n", str_dmd);
				dmd_client_record(g_sh_dmd_client[index], head->fault_id, str_dmd);
				kfree(str_dmd);
			}
			break;
		}
	}
}
