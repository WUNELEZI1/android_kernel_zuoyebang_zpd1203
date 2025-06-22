// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Linaro Limited
 * Copyright (C) 2021 XiaoMi, Inc.
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <tee_drv.h>
#include <linux/uuid.h>

#include "sample_kernel_ca.h"

#define TA_UUID \
	UUID_INIT(0xc452c518, 0x68d5, 0x11ee, 0x8c, 0x99, 0x02, 0x42, 0xac, 0x12, 0x00, 0x02)

#define TEST_CHECK(func) \
	do { \
		pr_info("%s start\n", #func); \
		if (func() != TEST_SUCCESS) { \
			pr_err("%s failed\n", #func); \
		} else { \
			pr_info("%s success\n", #func); \
		} \
	} while (0)

#define TEST_VALUE_INPUT	0
#define TEST_VALUE_OUTPUT	1

static const u8 g_test_buf_input[TEST_BUF_SIZE] = {
	0xf7, 0x4c, 0xff, 0x93, 0x71, 0x57, 0x64, 0xd0,
	0x6e, 0x22, 0x04, 0xf0, 0x82, 0x7a, 0xc6, 0x99,
	0xbd, 0xb1, 0x54, 0xb6, 0x1e, 0xfb, 0x63, 0x16,
	0x9b, 0xb8, 0x55, 0x2d, 0x6b, 0xa5, 0xd5, 0xe5,
};
static const u8 g_test_buf_output[TEST_BUF_SIZE] = {
	0x1d, 0x51, 0xee, 0x14, 0x44, 0xc1, 0xeb, 0xfe,
	0xce, 0x2d, 0x33, 0xbe, 0xb4, 0x84, 0xd1, 0xea,
	0xc3, 0xc9, 0xf9, 0xd3, 0x5d, 0x0e, 0x39, 0x84,
	0x37, 0xc3, 0xe1, 0x13, 0xaf, 0x13, 0x69, 0xe5,
};

/**
 * struct sample_kernel_ca_private - TEE private data
 * @ctx:	TEE context handler.
 * @session:	sample_test TA session identifier.
 */
struct sample_kernel_ca_private {
	struct tee_context *ctx;
	u32 session;
};

static struct sample_kernel_ca_private g_pvt_data;

static int mitee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int do_test_value(void)
{
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	int ret = 0;
	int result = TEST_SUCCESS;

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));

	/* Invoke commands */
	inv_arg.func = TA_CMD_TEST_VALUE;
	inv_arg.session = g_pvt_data.session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = TEST_VALUE_INPUT;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INOUT;
	param[2].u.value.a = TEST_VALUE_INPUT;

	ret = tee_client_invoke_func(g_pvt_data.ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func error: %x, %x\n", ret, inv_arg.ret);
		return TEST_FAIL;
	}

	if (param[1].u.value.a != TEST_VALUE_OUTPUT) {
		pr_err("value[1] failed: %llx (expected %x)\n",
			param[1].u.value.a, TEST_VALUE_OUTPUT);
		result = TEST_FAIL;
	}

	if (param[2].u.value.a != TEST_VALUE_OUTPUT) {
		pr_err("value[2] failed: %llx (expected %x)\n",
			param[2].u.value.a, TEST_VALUE_OUTPUT);
		result = TEST_FAIL;
	}

	return result;
}

static int do_test_memref(void)
{
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[4];
	struct tee_shm *test_shm = NULL;
	u8 *temp_buf = NULL;
	int ret = 0;
	int result = TEST_SUCCESS;

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));

	temp_buf = kzalloc(TEST_MEM_SIZE, GFP_KERNEL);
	if (!temp_buf) {
		pr_err("temp_buf kzalloc failed\n");
		return TEST_FAIL;
	}

	/* Register share memory */
	test_shm = tee_shm_register(g_pvt_data.ctx, (unsigned long)temp_buf, TEST_MEM_SIZE,
		TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(test_shm)) {
		pr_err("tee_shm_register failed\n");
		result = TEST_FAIL;
		goto free_temp;
	}

	memcpy(temp_buf + TEST_IN, g_test_buf_input, TEST_BUF_SIZE);
	memset(temp_buf + TEST_OUT, 0, TEST_BUF_SIZE);
	memcpy(temp_buf + TEST_INOUT, g_test_buf_input, TEST_BUF_SIZE);

	/* Invoke commands */
	inv_arg.func = TA_CMD_TEST_MEMREF;
	inv_arg.session = g_pvt_data.session;
	inv_arg.num_params = 4;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = test_shm;
	param[0].u.memref.size = TEST_BUF_SIZE;
	param[0].u.memref.shm_offs = TEST_IN;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param[1].u.memref.shm = test_shm;
	param[1].u.memref.size = TEST_BUF_SIZE;
	param[1].u.memref.shm_offs = TEST_OUT;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[2].u.memref.shm = test_shm;
	param[2].u.memref.size = TEST_BUF_SIZE;
	param[2].u.memref.shm_offs = TEST_INOUT;

	ret = tee_client_invoke_func(g_pvt_data.ctx, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func error: %x, %x\n", ret, inv_arg.ret);
		result = TEST_FAIL;
		goto free_shm;
	}

	if (memcmp(temp_buf + TEST_OUT, g_test_buf_output, TEST_BUF_SIZE) != 0) {
		print_hex_dump(KERN_ERR, "memref[1] failed:",
			DUMP_PREFIX_NONE, 16, 1, temp_buf + TEST_OUT, TEST_BUF_SIZE, 0);
		print_hex_dump(KERN_ERR, "memref[1] expected:",
			DUMP_PREFIX_NONE, 16, 1, g_test_buf_output, TEST_BUF_SIZE, 0);
		result = TEST_FAIL;
	}

	if (memcmp(temp_buf + TEST_INOUT, g_test_buf_output, TEST_BUF_SIZE) != 0) {
		print_hex_dump(KERN_ERR, "memref[2] failed:",
			DUMP_PREFIX_NONE, 16, 1, temp_buf + TEST_INOUT, TEST_BUF_SIZE, 0);
		print_hex_dump(KERN_ERR, "memref[2] expected:",
			DUMP_PREFIX_NONE, 16, 1, g_test_buf_output, TEST_BUF_SIZE, 0);
		result = TEST_FAIL;
	}

free_shm:
	tee_shm_free(test_shm);
free_temp:
	kfree(temp_buf);

	return result;
}

static int initialize_pvt_data(void)
{
	struct tee_ioctl_open_session_arg sess_arg;
	int ret = 0;

	memset(&sess_arg, 0, sizeof(sess_arg));

	/* Open context with MITEE driver */
	g_pvt_data.ctx = tee_client_open_context(NULL, mitee_ctx_match, NULL, NULL);
	if (IS_ERR(g_pvt_data.ctx)) {
		pr_err("tee_client_open_context error\n");
		return -ENODEV;
	}

	/* Open session with sample_test TA */
	export_uuid(sess_arg.uuid, &TA_UUID);
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(g_pvt_data.ctx, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		pr_err("tee_client_open_session error: %x, %x\n", ret, sess_arg.ret);
		tee_client_close_context(g_pvt_data.ctx);
		return -EINVAL;
	}

	g_pvt_data.session = sess_arg.session;
	return 0;
}

static void finish_pvt_data(void)
{
	tee_client_close_session(g_pvt_data.ctx, g_pvt_data.session);
	tee_client_close_context(g_pvt_data.ctx);
}

static int __init sample_kernel_ca_init(void)
{
	int ret;

	pr_info("sample_kernel_ca_init\n");

	ret = initialize_pvt_data();
	if (ret < 0) {
		pr_err("initialize_pvt_data failed\n");
		return ret;
	}

	TEST_CHECK(do_test_value);
	TEST_CHECK(do_test_memref);
	return ret;
}

static void __exit sample_kernel_ca_exit(void)
{
	pr_info("sample_kernel_ca_exit\n");
	finish_pvt_data();
}

module_init(sample_kernel_ca_init);
module_exit(sample_kernel_ca_exit);
MODULE_LICENSE("GPL");
