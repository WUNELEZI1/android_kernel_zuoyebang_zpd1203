// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 * Description: atf shmem test code
 * Modify time: 2023-03-13
 */

#include <soc/xring/atf_shmem.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/thread_info.h>
#include <linux/kernel_stat.h>
#include <linux/sched/cputime.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <asm/thread_info.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/kernel.h>

/*
 * Usage:
 * In adb shell:
 * "echo 0 > proc/atf_shmem_test" to test atf_shm_in
 * "echo 1 > proc/atf_shmem_test" to test atf_shm_out
 * "echo 2 > proc/atf_shmem_test" to test atf_shm_inout
 */

/* function declaration */
static int atf_shm_in(int data);
static int atf_shm_out(int data);
static int atf_shm_inout(int data);

#define SHARE_DATA_IN         0x25
#define SHARE_DATA_OUT        0x52
#define MIN(a, b)             ((a) < (b) ? (a) : (b))

struct test_handle {
	int id;
	int (*func)(int data);
	char func_name[64];
};

enum {
	ATF_SHM_IN = 0,
	ATF_SHM_OUT,
	ATF_SHM_INOUT,
};

static uint8_t g_data_buf[4096] = { 0 };
static char g_local_buf[4096] = { 0 };

static struct test_handle test_handle_arr[] = {
	[ATF_SHM_IN] = {
		ATF_SHM_IN, atf_shm_in, "atf_shm_in"
	},
	[ATF_SHM_OUT] = {
		ATF_SHM_OUT, atf_shm_out, "atf_shm_out"
	},
	[ATF_SHM_INOUT] = {
		ATF_SHM_INOUT, atf_shm_inout, "atf_shm_inout"
	},
};

static int atf_shm_in(int data)
{
	u64 size = 0;
	int ret;
	int i;

	pr_err("%s test\n", __func__);
	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		pr_err("%s get_atf_shm_size error, size: %llx\n", __func__, size);
		return -EPERM;
	}

	size = MIN(size, sizeof(g_data_buf));

	for (i = 0; i < (int)size; i++)
		g_data_buf[i] = SHARE_DATA_IN;

	ret = smc_shm_mode(FID_BL31_SHMEM_TEST_IN, (char *)g_data_buf, size, TYPE_IN, 0);
	if (ret != 0) {
		pr_err("%s smc_shm_mode error, ret: %d\n", __func__, ret);
		return -EPERM;
	}

	pr_err("%s test ok!\n", __func__);
	return 0;
}

static int atf_shm_out(int data)
{
	u64 size = 0;
	int ret;
	int i;

	pr_err("%s test\n", __func__);
	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		pr_err("%s get_atf_shm_size error, size: %llx\n", __func__, size);
		return -EPERM;
	}

	size = MIN(size, sizeof(g_data_buf));

	ret = smc_shm_mode(FID_BL31_SHMEM_TEST_OUT, (char *)g_data_buf, size, TYPE_OUT, 0);
	if (ret != 0) {
		pr_err("%s smc_shm_mode error, ret: %d\n", __func__, ret);
		return -EPERM;
	}

	for (i = 0; i < (int)size; i++) {
		if (g_data_buf[i] != SHARE_DATA_OUT) {
			pr_err("%s test error\n", __func__);
			return -EPERM;
		}
	}

	pr_err("%s test ok!\n", __func__);
	return ret;
}

static int atf_shm_inout(int data)
{
	u64 size = 0;
	int ret;
	int i;

	pr_err("%s test\n", __func__);
	ret = get_atf_shm_size(&size);
	if (ret != 0) {
		pr_err("%s get_atf_shm_size error, size: %llx\n", __func__, size);
		return -EPERM;
	}

	size = MIN(size, sizeof(g_data_buf));

	for (i = 0; i < (int)size; i++)
		g_data_buf[i] = SHARE_DATA_IN;

	ret = smc_shm_mode(FID_BL31_SHMEM_TEST_INOUT, (char *)g_data_buf, size, TYPE_INOUT, 0);
	if (ret != 0) {
		pr_err("%s smc_shm_mode error, ret: %d\n", __func__, ret);
		return -EPERM;
	}

	for (i = 0; i < (int)size; i++) {
		if (g_data_buf[i] != SHARE_DATA_OUT) {
			pr_err("%s out error\n", __func__);
			return -EPERM;
		}
	}

	pr_err("%s test ok!\n", __func__);
	return ret;
}

static ssize_t shmem_test_write(
	struct file *filp, const char __user *buf,
	size_t len, loff_t *ppos)
{
	int ret;
	ssize_t buf_size;
	int test_data;

	if (!buf) {
		pr_err("%s: error, buf is NULL\n", __func__);
		return -EFAULT;
	}
	buf_size = (ssize_t)min(len, (size_t)(sizeof(g_local_buf) - 1));
	if (copy_from_user(g_local_buf, buf, buf_size))
		return -EFAULT;

	ret = kstrtoint(g_local_buf, 0, &test_data);
	if (ret)
		goto out;

	if (test_data < 0 || (u32)test_data >= ARRAY_SIZE(test_handle_arr)) {
		pr_err("input number %d error\n", test_data);
		goto out;
	}

	if (!test_handle_arr[test_data].func) {
		pr_err("test case %d not implement\n", test_data);
		goto out;
	}

	ret = test_handle_arr[test_data].func(test_data);
	if (ret)
		pr_err("%s: test_handle_arr return %d\n", test_handle_arr[test_data].func_name, ret);

out:
	return buf_size;
}

static int shmem_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t shmem_test_read(
	struct file *fid, char __user *buf,
	size_t size, loff_t *ppos)
{
	pr_err("shmem_test: fake read\n");
	return size;
}

static const struct proc_ops shmem_test_proc_ops = {
	.proc_open           = shmem_test_open,
	.proc_read           = shmem_test_read,
	.proc_write          = shmem_test_write,
};

static int __init shmem_test_init(void)
{
	proc_create("atf_shmem_test", 0640, NULL, &shmem_test_proc_ops);
	return 0;
}
fs_initcall(shmem_test_init);
MODULE_DESCRIPTION("XRing atf shmem test driver");
MODULE_LICENSE("GPL v2");
