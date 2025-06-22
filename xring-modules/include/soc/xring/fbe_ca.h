/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: FBE Kernel CA header file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#ifndef XRING_FBE_CA_H
#define XRING_FBE_CA_H

#include <linux/types.h>

#define TEE_NUM_PARAMS          4
#define ICE_CRY_CAP_NUM         128
#define ICE_KEYSLOT_NUM         64
#define KEY_MAX_SIZE            128
#define WRAPPED_KEY_SIZE        48
#define AES_256_KEY_SIZE        32
#define AES_256_XTS_KEY_SIZE    64

enum xring_ice_cipher_alg {
	XRING_ICE_CIPHER_AES_128_XTS = 0,
	XRING_ICE_CIPHER_AES_256_XTS = 1,
};

enum xing_fbe_cmd_id {
	/*
	 * Program UFS Encryption Key
	 *
	 * [in]      param[0].u.value
	 * [in]      param[1].u.memref
	 * [in]      param[2].u.value
	 */
	TEEC_FBE_PROGRAM_ENCRYPTION_KEY = 0,

	/*
	 * Evict UFS Encryption Key
	 *
	 * [in]      param[0].u.value
	 */
	TEEC_FBE_EVICT_ENCRYPTION_KEY = 1,

	/*
	 * Derive sw_secret Key
	 *
	 * [in]      param[0].u.memref
	 * [inout]   param[1].u.memref
	 */
	TEEC_FBE_DERIVE_SW_SECKET = 2,

	TEEC_FBE_CMD_MAX = 3,
};

int xring_ice_program_key(u32 cap_idx,
			  u32 index,
			  const u8 *wrapped_key,
			  u32 wrapped_key_size,
			  enum xring_ice_cipher_alg cipher_alg,
			  u32 data_unit_size);

int xring_ice_evict_key(u32 index);

int xring_derive_sw_secret(const u8 *wrapped_key,
			   u32 wrapped_key_size,
			   u8 *secret,
			   u32 *secret_size);

#endif // FBE_CA_H
