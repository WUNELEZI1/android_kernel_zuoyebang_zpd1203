/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */
#ifndef MITEE_HPC_TEE_CA_H
#define MITEE_HPC_TEE_CA_H
#define HPC_CA_DRIVER_NAME	"hpc-ree-kernel-ca"

#define TEE_IOCTL_PARAM_NUM    4

#define TEST_BUF_SIZE	32
#define TEST_MEM_SIZE	(1024 * 1024)

#define TEST_IN		0
#define TEST_OUT	(TEST_MEM_SIZE / 2)
#define TEST_INOUT	(TEST_MEM_SIZE - TEST_BUF_SIZE)

enum ta_cmd_id {
	/*
	 * TEE hpc boot hardware device
	 *
	 * [in]      value[0]         Input - Core number
	 * [in]	    value[1]         Input - Nonsecurity firmware ddr address
	 */
	TA_CMD_HPC_BOOT,
	/*
	 * TEE hpc boot hardware device
	 *
	 * [in]      value[0]         Input - Core number
	 */
	TA_CMD_HPC_SHUTDOWN,
	/*
	 * TEE hpc load npuaon firmware
	 *
	 * [in]      value[0]         Input - Core number
	 */
	TA_CMD_HPC_LOAD_AONFW,
};

#endif
