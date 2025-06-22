// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __IPC_MSG_NPU_SH_H__
#define __IPC_MSG_NPU_SH_H__

#include <stdint.h>


enum in_msg_type_e {
	IN_MSG_MIN = 0,
	IN_MSG_T2S_BOOT,
	IN_MSG_T2S_LOAD,
	IN_MSG_T2S_UNLOAD,
	IN_MSG_S2N_INIT_REQ,
	IN_MSG_N2S_INIT_ACK,
	IN_MSG_S2N_SUBMIT_REQ,
	IN_MSG_N2S_SUBMIT_ACK,
	IN_MSG_S2N_TRIGGER_REQ,
	IN_MSG_N2S_TRIGGER_ACK,
	IN_MSG_S2N_CANCEL_REQ,
	IN_MSG_N2S_CANCEL_ACK,
	IN_MSG_S2N_DEINIT_REQ,
	IN_MSG_N2S_DEINIT_ACK,
	IN_MSG_MAX,
};

#define NN_MODEL_IO_NUM_MAX 3


struct in_msg_boot_t {
	uint32_t img_store_pa;
	uint32_t img_runtime_pa;
	uint32_t img_runtime_da;
	uint32_t img_runtime_size;
	uint32_t img_bin_size;
	uint32_t logbuf_pa;
	uint32_t logbuf_da;
	uint32_t logbuf_size;
};

struct in_msg_load_t {
	uint32_t model_id;
	uint32_t ntd_addr;
	uint32_t ntd_len;
	uint32_t wl_addr;
	uint32_t wl_len;
};

struct in_msg_unload_t {
	uint32_t model_id;
};

struct in_msg_init_t {
	uint32_t wl_addr;
	uint32_t wl_len;
	uint32_t ret;
};

struct in_msg_submit_t {
	uint32_t model_id;
	uint32_t ntd_addr;
	uint32_t ntd_len;
	uint32_t ret;
};

struct in_msg_trigger_t {
	uint32_t model_id;
	uint32_t input_num;
	uint32_t input_addr[NN_MODEL_IO_NUM_MAX];
	uint32_t output_num;
	uint32_t output_addr[NN_MODEL_IO_NUM_MAX];
	uint32_t ret;
};

struct in_msg_cancel_t {
	uint32_t model_id;
	uint32_t ret;
};

struct in_msg_deinit_t {
	uint32_t ret;
};

struct aon_msg_t {
	uint32_t msg_type;
	uint32_t msg_id;
	union {
		struct in_msg_boot_t boot;
		struct in_msg_load_t load;
		struct in_msg_unload_t unload;
		struct in_msg_init_t init;
		struct in_msg_submit_t submit;
		struct in_msg_trigger_t trigger;
		struct in_msg_cancel_t cancel;
		struct in_msg_deinit_t deinit;
	};
};



#endif
