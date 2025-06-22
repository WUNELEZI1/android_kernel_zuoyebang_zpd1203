/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: Model Protection CA header file
 * Modify time: 2024-8-12
 * Author: Security-TEEOS
 */

#ifndef XRING_MODEL_PROTECTION_CA_H
#define XRING_MODEL_PROTECTION_CA_H

#include <linux/types.h>

#define TEE_NUM_PARAMS          4

enum xing_model_protection_cmd_id {
	/*
	 * Get Model Protection Key
	 *
	 * [in]      param[0].u.memref
	 * [out]     param[1].u.memref
	 */
	TEEC_CMD_GET_MODEL_KEY = 0
};

/*
 * Description: The get_model_key function returns the model key which is used to encrypt the model
 * Parameters:
 *     header      - The input model header
 *     header_size - The length of model header
 *     key         - Used to contain the returned model key
 *     key_size    - The length of "key" buffer when input, the actual length of returned model key
 *                   when output
 * Return:	0 if no error
 */
int get_model_key(const u8 *header, size_t header_size, u8 *key, size_t *key_size);

#endif // XRING_MODEL_PROTECTION_CA_H
