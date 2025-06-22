/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */

#ifndef MITEE_SAMPLE_KERNEL_CA_H
#define MITEE_SAMPLE_KERNEL_CA_H

#define TEST_SUCCESS    0x00000000
#define TEST_FAIL       0x00000001

#define TEST_BUF_SIZE   32
#define TEST_MEM_SIZE   (1024 * 1024)

#define TEST_IN         0
#define TEST_OUT        (TEST_MEM_SIZE / 2)
#define TEST_INOUT      (TEST_MEM_SIZE - TEST_BUF_SIZE)

enum ta_cmd_id {
	/*
	 * Test TEE_PARAM_TYPE_VALUE_*
	 *
	 * [in]      value[0]        Input
	 * [out]     value[1]        Output
	 * [inout]   value[2]        Input and output
	 */
	TA_CMD_TEST_VALUE = 0,
	/*
	 * Test TEE_PARAM_TYPE_MEMREF_*
	 *
	 * [in]      memref[0]        Input
	 * [out]     memref[1]        Output
	 * [inout]   memref[2]        Input and output
	 */
	TA_CMD_TEST_MEMREF,
};

#endif // MITEE_SAMPLE_KERNEL_CA_H
