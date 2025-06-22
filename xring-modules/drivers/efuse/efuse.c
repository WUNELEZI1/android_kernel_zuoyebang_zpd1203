// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 * Description: efuse driver
 * Modify time: 2024-12-18
 */

#define pr_fmt(fmt)	"[efuse]: " fmt

#include <soc/xring/atf_shmem.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/dmd.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>
#include <linux/atomic.h>
#include <linux/preempt.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/version.h>

#define EFUSE_DMD_BUFF_SIZE   512

static struct dmd_client *client_efuse;

static struct dmd_dev efuse_mod = {
	.name = "xrse",
	.module_name = "efuse",
	.buff_size = EFUSE_DMD_BUFF_SIZE,
	.fault_level = "CRITICAL",
};

static void dmd_efuse_init(void)
{
	client_efuse = dmd_register_client(&efuse_mod);
	if (!client_efuse)
		dmd_log_err("dmd1 register failed\n");
}

static void dmd_test_exit(void)
{
	if (client_efuse != NULL)
		dmd_unregister_client(client_efuse, &efuse_mod);
}

static void dmd_report_efuse(void)
{
	dmd_client_record(client_efuse, DMD_XRSE_EFUSE_ERROR_NO, "svc efuse failed");
}

ssize_t bl31_efuse_read(u32 field_id, u32 data_len, u8 *data)
{
	u64 size = 0;
	ssize_t ret;

	if (data == NULL || data_len == 0) {
		pr_err("%s input error\n", __func__);
		return -EPERM;
	}

	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		pr_err("%s get_atf_shm_size error, size: %llx\n", __func__, size);
		return -EPERM;
	}

	if (size < data_len) {
		pr_err("et_atf_shm_size error, lem:0x%x,size: 0x%llx\n", data_len, size);
		return -EPERM;
	}

	(void)memset_s(data, data_len, 0, data_len);
	ret = memcpy_s(data, data_len, &field_id, sizeof(u32));
	if (ret != EOK) {
		pr_err("memcpy_s error,datalen:%d\n", data_len);
		return ret;
	}

	ret = smc_shm_mode(FID_BL31_EFUSE_RD, (char *)data, data_len, TYPE_INOUT, 0);
	if (ret) {
		pr_err("efuse read: shmem read error\n");
		dmd_report_efuse();
		/* set count to zero since copied error */
	}

	return ret;
}
EXPORT_SYMBOL(bl31_efuse_read);

ssize_t bl31_efuse_write(u32 data_len, u8 *data)
{
	u64 size = 0;
	ssize_t ret;

	if (data == NULL || data_len == 0) {
		pr_err("%s input error\n", __func__);
		return -EPERM;
	}

	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		pr_err("et_atf_shm_size error, lem:0x%x,size: 0x%llx\n", data_len, size);
		return -EPERM;
	}

	if (size < data_len) {
		pr_err("%s the shm size is not enough, size: 0x%llx\n", __func__, size);
		return -EPERM;
	}

	ret = smc_shm_mode(FID_BL31_EFUSE_WR, (char *)(data), data_len, TYPE_IN, 0);
	if (ret) {
		pr_err("efuse write: shmem error\n");
		/* set count to zero since copied error */
		dmd_report_efuse();
		return ret;
	}

	return ret;
}
EXPORT_SYMBOL(bl31_efuse_write);

static int __init __efuse_init(void)
{
	dmd_efuse_init();
	pr_info("efuse driver init\n");
	return 0;
}

static void __exit __efuse_exit(void)
{
	dmd_test_exit();
	pr_info("efuse driver exit\n");
}

MODULE_DESCRIPTION("XRing efuse driver");
MODULE_LICENSE("GPL v2");
module_init(__efuse_init);
module_exit(__efuse_exit);
