// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: FBE Kernel CA test source file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": [%s %d] " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <soc/xring/fbe_ca.h>

#define DATA_UNIT_SIZE 512

typedef int (*fbe_test_func)(void);

struct fbe_test_case {
	u32 case_index;
	fbe_test_func case_func;
};

static const u8 WRAPPED_KEY[48] = {
	0x0D, 0x95, 0xB4, 0x13, 0x8D, 0xC2, 0x93, 0x00,
	0x1E, 0x65, 0xFA, 0x4C, 0x37, 0xF3, 0x9E, 0x31,
	0xC4, 0x29, 0x11, 0xE2, 0xCC, 0x71, 0x36, 0xF9,
	0x87, 0xF3, 0x53, 0x0E, 0xA7, 0x4C, 0xC2, 0x30,
	0x65, 0x20, 0xB2, 0x44, 0xE8, 0xC1, 0xA5, 0x84,
	0x29, 0xD2, 0xA9, 0x74, 0xDB, 0xAD, 0x72, 0xA6
};

static const u8 SECRET_EXPECT[32] = {
	0xb1, 0x29, 0x7f, 0xa5, 0xc2, 0xdf, 0xf3, 0x1e,
	0x81, 0x46, 0xdd, 0xec, 0x8b, 0x5a, 0x38, 0xa5,
	0x4a, 0xf5, 0xdb, 0xe3, 0x10, 0x44, 0xaf, 0xd4,
	0xa9, 0xc1, 0x1b, 0x10, 0xbb, 0x03, 0x88, 0x26
};

static int fbe_test_1(void)
{
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	return xring_ice_program_key(cap_idx,
				     index,
				     WRAPPED_KEY,
				     wrapped_key_size,
				     XRING_ICE_CIPHER_AES_128_XTS,
				     data_unit_size);
}

static int fbe_test_2(void)
{
	u32 cap_idx = 1;
	u32 index = 1;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	return xring_ice_program_key(cap_idx,
				     index,
				     WRAPPED_KEY,
				     wrapped_key_size,
				     XRING_ICE_CIPHER_AES_256_XTS,
				     data_unit_size);
}

static int fbe_test_3(void)
{
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	return xring_ice_program_key(cap_idx,
				     index,
				     WRAPPED_KEY,
				     wrapped_key_size,
				     XRING_ICE_CIPHER_AES_256_XTS,
				     data_unit_size);
}

static int fbe_test_4(void)
{
	u32 cap_idx = 1;
	u32 index = 63;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	return xring_ice_program_key(cap_idx,
				     index,
				     WRAPPED_KEY,
				     wrapped_key_size,
				     XRING_ICE_CIPHER_AES_256_XTS,
				     data_unit_size);
}

static int fbe_test_5(void)
{
	u32 index = 0;

	return xring_ice_evict_key(index);
}

static int fbe_test_6(void)
{
	u32 index = 63;

	return xring_ice_evict_key(index);
}

static int fbe_test_7(void)
{
	int ret;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u8 secret[32];
	u32 secret_size = sizeof(secret);
	u32 i;

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, secret, &secret_size);

	pr_info("secret = ");
	// Print in 4 times, with 8 bytes each time
	for (i = 0; i < 4; i++) {
		pr_info("sw_secret: %02X %02X %02X %02X %02X %02X %02X %02X", secret[i * 8],
			secret[i * 8 + 1], secret[i * 8 + 2], secret[i * 8 + 3], secret[i * 8 + 4],
			secret[i * 8 + 5], secret[i * 8 + 6], secret[i * 8 + 7]);
	}

	if ((ret == 0) && (memcmp(SECRET_EXPECT, secret, secret_size) == 0))
		return 0;

	return -1;
}

static int fbe_test_8(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    WRAPPED_KEY,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS + 1,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_9(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 64;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    WRAPPED_KEY,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_10(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = 49;
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    WRAPPED_KEY,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_11(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = 0;
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    NULL,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_12(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    NULL,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_13(void)
{
	int ret;
	u32 cap_idx = 1;
	u32 index = 0;
	u32 wrapped_key_size = 0;
	u32 data_unit_size = DATA_UNIT_SIZE;

	ret = xring_ice_program_key(cap_idx,
				    index,
				    NULL,
				    wrapped_key_size,
				    XRING_ICE_CIPHER_AES_256_XTS,
				    data_unit_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_14(void)
{
	int ret;
	u32 index = 64;

	ret = xring_ice_evict_key(index);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_15(void)
{
	int ret;
	u32 wrapped_key_size = 49;
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, secret, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_16(void)
{
	int ret;
	u32 wrapped_key_size = 0;
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, secret, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_17(void)
{
	int ret;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	ret = xring_derive_sw_secret(NULL, wrapped_key_size, secret, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_18(void)
{
	int ret;
	u32 wrapped_key_size = 0;
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	ret = xring_derive_sw_secret(NULL, wrapped_key_size, secret, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_19(void)
{
	int ret;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, NULL, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_20(void)
{
	int ret;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u8 secret[32];

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, secret, NULL);
	if (ret == 0)
		return -1;

	return 0;
}

static int fbe_test_21(void)
{
	int ret;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u8 secret[32];
	u32 secret_size = 1;

	ret = xring_derive_sw_secret(WRAPPED_KEY, wrapped_key_size, secret, &secret_size);
	if (ret == 0)
		return -1;

	return 0;
}

const struct fbe_test_case fbe_test[] = {
	{ 1, fbe_test_1 },
	{ 2, fbe_test_2 },
	{ 3, fbe_test_3 },
	{ 4, fbe_test_4 },
	{ 5, fbe_test_5 },
	{ 6, fbe_test_6 },
	{ 7, fbe_test_7 },
	{ 8, fbe_test_8 },
	{ 9, fbe_test_9 },
	{ 10, fbe_test_10 },
	{ 11, fbe_test_11 },
	{ 12, fbe_test_12 },
	{ 13, fbe_test_13 },
	{ 14, fbe_test_14 },
	{ 15, fbe_test_15 },
	{ 16, fbe_test_16 },
	{ 17, fbe_test_17 },
	{ 18, fbe_test_18 },
	{ 19, fbe_test_19 },
	{ 20, fbe_test_20 },
	{ 21, fbe_test_21 }
};

void fbe_ca_test(void)
{
	int ret;
	u32 case_num_total = ARRAY_SIZE(fbe_test);
	u32 case_num_fail = 0;
	u32 i;

	pr_info("********** Xring FBE CA Test Begin **********\n");

	for (i = 0; i < case_num_total; i++) {
		ret = fbe_test[i].case_func();
		if (ret == 0) {
			pr_info("********** Test Case %u Pass **********\n",
				fbe_test[i].case_index);
		} else {
			case_num_fail++;
			pr_info("********** Test Case %u Fail **********\n",
				fbe_test[i].case_index);
		}
	}

	pr_info("********** Xring FBE CA Test End: %u cases, %u failed **********\n",
		case_num_total, case_num_fail);
}
EXPORT_SYMBOL(fbe_ca_test);

void fbe_ca_retry_test(void)
{
	u8 secret[32];
	u32 secret_size = sizeof(secret);

	xring_ice_program_key(1,
			      0,
			      WRAPPED_KEY,
			      sizeof(WRAPPED_KEY),
			      XRING_ICE_CIPHER_AES_256_XTS,
			      DATA_UNIT_SIZE);
	xring_ice_evict_key(0);
	xring_derive_sw_secret(WRAPPED_KEY, sizeof(WRAPPED_KEY), secret, &secret_size);
}
EXPORT_SYMBOL(fbe_ca_retry_test);

void fbe_ca_stress_test(u32 count)
{
	int ret;
	u32 cap_idx = 1;
	u32 index;
	u32 wrapped_key_size = sizeof(WRAPPED_KEY);
	u32 data_unit_size = DATA_UNIT_SIZE;
	u32 count_fail = 0;
	u32 i;

	pr_info("********** Xring FBE CA Stress Test Begin, %u times **********\n", count);

	for (i = 0; i < count; i++) {
		index = i % ICE_KEYSLOT_NUM;
		ret = xring_ice_program_key(cap_idx,
					    index,
					    WRAPPED_KEY,
					    wrapped_key_size,
					    XRING_ICE_CIPHER_AES_256_XTS,
					    data_unit_size);
		if (ret != 0) {
			count_fail++;
			pr_err("********** The %u time Program Key Fail **********\n",
				i + 1);
			continue;
		}

		ret = xring_ice_evict_key(index);
		if (ret != 0) {
			count_fail++;
			pr_err("********** The %u time Evict Key Fail **********\n",
				i + 1);
			continue;
		}

		pr_info("********** The %u time Stress Test Pass **********\n",
			i + 1);
	}

	pr_info("********** Xring FBE CA Stress Test End: %u times, %u failed **********\n",
		count, count_fail);
}
EXPORT_SYMBOL(fbe_ca_stress_test);
