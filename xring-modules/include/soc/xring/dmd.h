/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: dmd driver
 *
 */
#ifndef _DMD_H_
#define _DMD_H_

#include <linux/mutex.h>

#define CLIENT_SIZE     128
#define CLIENT_NAME_LEN		32
#define DMD_MAX_LOG_SIZE	256
#define DMD_MAX_MODULE_NAME_LEN 32
#define DMD_MAX_FAULT_LEVEL	16

#define CBUFF_READY_BIT         1
#define DMD_CLIENT_NOTIFY_BIT	0
#define DMD_CLIENT_VAILD_BIT	1
#define DATATIME_MAXLEN		20
#define DMD_WORK_DELAY_TIME	10000

/* dmd error no */
#define DMD_TEST_1_ERROR_NO     921000001
#define DMD_TEST_2_ERROR_NO     921000002
#define DMD_TEST_3_ERROR_NO     921000003

/* clk(dvfs) error no */
#define DMD_PMD_DVS_ERROR_NO    921000101

/* mpmic error no */
#define DMD_MPMIC_VBD_ERROR_NO  921001001
#define DMD_MPMIC_OCP_ERROR_NO  921001002
#define DMD_MPMIC_OTMP_ERROR_NO 921001003

/* lpcore error no */
#define DMD_LPCORE_ERROR_NO     921002000

/* sensorhub error no */
#define DMD_SENSORHUB_ERROR_NO  921003000

/* acpu error no */
#define DMD_ACPU_AVS_HPM_NO	921003002

/* xrse error no */
#define DMD_XRSE_ERROR_NO       921004000
#define DMD_XRSE_EFUSE_ERROR_NO 921004001

/* noc dma error no */
#define DMD_NOC_DMA_ERROR_NO    921005000

struct dmd_server {
	unsigned long client_flag[CLIENT_SIZE];
	struct dmd_client *client_list[CLIENT_SIZE];
	int client_count;
	int server_state;
	struct workqueue_struct *dmd_wq;
	struct mutex mtx_lock;
};

struct dmd_client {
	char client_name[CLIENT_NAME_LEN];
	char module_name[DMD_MAX_MODULE_NAME_LEN];
	char fault_level[DMD_MAX_FAULT_LEVEL];
	int client_id;
	int error_no;
	unsigned long buff_flag;
	size_t buff_size;
	u8 dump_buff[];
};

struct dmd_dev {
	const char *name;
	const char *module_name;
	size_t buff_size;
	const char *fault_level;
};

#define dmd_log_err(args...)    pr_err("<dmd err>"args)
#define dmd_log_info(args...)    pr_info("<dmd info>"args)

struct dmd_client *dmd_register_client(struct dmd_dev *dev);
int dmd_client_record(struct dmd_client *client, int error_no, const char *fmt, ...);
void dmd_unregister_client(struct dmd_client *dmd_client, struct dmd_dev *dev);

#endif
