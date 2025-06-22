// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: FBE Kernel CA test source file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#define pr_fmt(fmt) "ModelProtect: [%s %d] " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/model_protection.h>


#define MODEL_HEADER_SIZE        96
#define MODEL_KEY_SIZE           32

#define OFFSET_MAGIC             0
#define OFFSET_VERSION           8
#define OFFSET_HDR_SIZE          12
#define OFFSET_TYPE              20
#define OFFSET_ATTR              24
#define OFFSET_KEM_ALG           28
#define OFFSET_ENC_ALG           32

#define VALUE_MAGIC              0x00
#define VALUE_VERSION            2
#define VALUE_HDR_SIZE           0x80
#define VALUE_TYPE               1
#define VALUE_ATTR               1
#define VALUE_KEM_ALG            0xFF
#define VALUE_ENC_ALG            0xFF

#define CONSTRUCT_HEADER(header, offset, value)   do {  \
	memcpy_s(header, sizeof(header), g_model_header_test, MODEL_HEADER_SIZE); \
	header[offset] = value;                             \
} while (0)

/* type define */
typedef int (*model_protection_test_func)(void);

struct model_protection_test_case {
	u32 case_index;
	model_protection_test_func case_func;
};

/* global variable */
static const u8 g_model_header_test[MODEL_HEADER_SIZE] = {
	0x4d, 0x6f, 0x64, 0x65, 0x6c, 0x48, 0x64, 0x72, 0x01, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
	0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x80,
	0x10, 0x08, 0x00, 0x40, 0x32, 0xcf, 0x00, 0xe5, 0x5a, 0xbd, 0x15, 0x96, 0x43, 0xad, 0x15, 0x8b,
	0x29, 0x79, 0x8b, 0x55, 0xcb, 0xb4, 0x61, 0x05, 0x6f, 0x62, 0x43, 0xc8, 0xb6, 0xa5, 0x3c, 0x0e,
	0xa4, 0x2d, 0x79, 0x2a, 0xb6, 0x12, 0x56, 0x57, 0x47, 0x18, 0x41, 0xf8, 0x0d, 0x5d, 0x00, 0x68,
	0xb6, 0xd0, 0x3d, 0x63, 0xbf, 0x3a, 0x1a, 0x4e, 0x56, 0x20, 0xce, 0x9a, 0x61, 0xae, 0xba, 0x82
};
static const u8 g_model_key_test[MODEL_KEY_SIZE] = {
	0x76, 0x3e, 0xaf, 0xbc, 0x2a, 0xc2, 0x23, 0xdf, 0xb0, 0x59, 0x22, 0xf5, 0x3b, 0xce, 0x7a, 0x71,
	0x0c, 0x7f, 0xad, 0x0c, 0xe9, 0xeb, 0x9c, 0x9a, 0xb3, 0x76, 0xe1, 0x20, 0x4f, 0x1a, 0x2c, 0x13
};
static u8 g_model_key[MODEL_KEY_SIZE];


static int model_protection_test_1(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);

	ret = get_model_key(g_model_header_test, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret != 0) {
		pr_err("get_model_key failed\n");
		return ret;
	}

	if (key_size != MODEL_KEY_SIZE) {
		pr_err("Wrong model key size\n");
		return -1;
	}

	if (memcmp(g_model_key_test, g_model_key, MODEL_KEY_SIZE) != 0) {
		pr_err("Wrong model key\n");
		return -1;
	}

	return 0;
}

static int model_protection_test_2(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);

	ret = get_model_key(NULL, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_3(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);

	ret = get_model_key(g_model_header_test, MODEL_HEADER_SIZE - 1, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_4(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);

	ret = get_model_key(g_model_header_test, MODEL_HEADER_SIZE, NULL, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_5(void)
{
	int ret;

	ret = get_model_key(g_model_header_test, MODEL_HEADER_SIZE, g_model_key, NULL);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_6(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key) - 1;

	ret = get_model_key(g_model_header_test, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_7(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_MAGIC, VALUE_MAGIC);
	ret = get_model_key(header, sizeof(header), g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_8(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_VERSION, VALUE_VERSION);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_9(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_HDR_SIZE, VALUE_HDR_SIZE);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_10(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_TYPE, VALUE_TYPE);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_11(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_ATTR, VALUE_ATTR);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_12(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_KEM_ALG, VALUE_KEM_ALG);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int model_protection_test_13(void)
{
	int ret;
	size_t key_size = sizeof(g_model_key);
	u8 header[MODEL_HEADER_SIZE];

	CONSTRUCT_HEADER(header, OFFSET_ENC_ALG, VALUE_ENC_ALG);
	ret = get_model_key(header, MODEL_HEADER_SIZE, g_model_key, &key_size);
	if (ret == 0)
		return -1;

	return 0;
}

static const struct model_protection_test_case model_protection_test[] = {
	{ 1, model_protection_test_1 },
	{ 2, model_protection_test_2 },
	{ 3, model_protection_test_3 },
	{ 4, model_protection_test_4 },
	{ 5, model_protection_test_5 },
	{ 6, model_protection_test_6 },
	{ 7, model_protection_test_7 },
	{ 8, model_protection_test_8 },
	{ 9, model_protection_test_9 },
	{ 10, model_protection_test_10 },
	{ 11, model_protection_test_11 },
	{ 12, model_protection_test_12 },
	{ 13, model_protection_test_13 }
};

void model_test(void)
{
	int ret;
	u32 case_num_total = ARRAY_SIZE(model_protection_test);
	u32 case_num_fail = 0;
	u32 i;

	pr_info("********** Xring Model Protection Test Begin **********\n");

	for (i = 0; i < case_num_total; i++) {
		ret = model_protection_test[i].case_func();
		if (ret == 0) {
			pr_info("********** Test Case %u Pass **********\n",
				model_protection_test[i].case_index);
		} else {
			case_num_fail++;
			pr_info("********** Test Case %u Fail **********\n",
				model_protection_test[i].case_index);
		}
	}

	pr_info("********** Xring Model Protection Test End: %u cases, %u failed **********\n",
		case_num_total, case_num_fail);
}
EXPORT_SYMBOL(model_test);
