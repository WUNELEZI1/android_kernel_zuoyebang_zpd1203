// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */


#ifndef __XRISP_BASE_H__
#define __XRISP_BASE_H__

#if defined(_LINUX) || defined(__linux__)
typedef long xrisp_long;
typedef unsigned long xrisp_ulong;
#else
#include <stdatomic.h>

typedef atomic_long xrisp_long;
typedef atomic_ulong xrisp_ulong;
#endif




#define RAMLOG_IPC_CMD_LOG_ENABLE 0x1
#define RAMLOG_IPC_CMD_LOG_LEVEL  0x2

typedef struct {
	uint8_t cmd;
	uint8_t val;
} ramlog_ipc_t;





#define EDR_WDT_BUF_OFFSET (DATA_RING_BUF_OFFSET + DATA_RING_BUF_SIZE + RAMLOG_GAP)





#define RAMLOG_DATA_ADDR_RECORD_SIZE (16)


#define RAMLOG_GAP 0x100

#define DATA_RING_BUF_SIZE 0x200000

#define RAMLOG_MAX_COUNTS     0x4000
#define RAMLOG_RINGBUF_OFFSET 0x0

#define RAMLOG_RINGBUF_SIZE 0x100
#define DESCS_OFFSET	    (RAMLOG_RINGBUF_OFFSET + RAMLOG_RINGBUF_SIZE + RAMLOG_GAP)

#define DESCS_SIZE   0x60000
#define INFOS_OFFSET (DESCS_OFFSET + DESCS_SIZE + RAMLOG_GAP)

#define INFOS_SIZE	     0x60000
#define DATA_RING_BUF_OFFSET (INFOS_OFFSET + INFOS_SIZE + RAMLOG_GAP)


#define _DATA_SIZE(sz_bits)   (1UL << (sz_bits))
#define _DESCS_COUNT(ct_bits) (1U << (ct_bits))
#define DESC_SV_BITS	      (sizeof(unsigned long) * 8)
#define DESC_FLAGS_SHIFT      (DESC_SV_BITS - 2)
#define DESC_FLAGS_MASK	      (3UL << DESC_FLAGS_SHIFT)
#define DESC_STATE(sv)	      (3UL & (sv >> DESC_FLAGS_SHIFT))
#define DESC_SV(id, state)    (((unsigned long)state << DESC_FLAGS_SHIFT) | id)
#define DESC_ID_MASK	      (~DESC_FLAGS_MASK)
#define DESC_ID(sv)	      ((sv) & DESC_ID_MASK)
#define FAILED_LPOS	      0x1
#define NO_LPOS		      0x3


struct lrb_info {
	unsigned long seq;
	unsigned long ts_usec;
	unsigned short text_len;
	unsigned char level;
	unsigned char cpu_id;
	unsigned int thread_id;
};


struct lrb_record {
	struct lrb_info *info;
	char *text_buf;
	unsigned int text_buf_size;
};


struct lrb_data_blk_lpos {
	unsigned long begin;
	unsigned long next;
};


struct lrb_desc {
	xrisp_long state_var;
	struct lrb_data_blk_lpos text_blk_lpos;
};


struct lrb_data_ring {
	uint32_t size_bits;
	char *data;
	xrisp_ulong head_lpos;
	xrisp_ulong tail_lpos;
};


struct lrb_desc_ring {
	unsigned int count_bits;
	struct lrb_desc *descs;
	struct lrb_info *infos;
	xrisp_ulong head_id;
	xrisp_ulong tail_id;
	xrisp_ulong last_finalized_id;
};


struct ramlog_ringbuffer {
	struct lrb_desc_ring desc_ring;
	struct lrb_data_ring text_data_ring;
	xrisp_ulong fail;
	unsigned char reset_flag;
	unsigned char ack_flag;
	unsigned char log_level;
	unsigned char log_enable;
};


enum desc_state {
	DESC_MISS = -1,
	DESC_RESERVED = 0x0,
	DESC_COMMITTED = 0x1,
	DESC_FINALIZED = 0x2,
	DESC_REUSABLE = 0x3,
};


#define BLK0_LPOS(sz_bits) (-(_DATA_SIZE(sz_bits)))
#define DESC0_ID(ct_bits)  DESC_ID(-(_DESCS_COUNT(ct_bits) + 1))
#define DESC0_SV(ct_bits)  DESC_SV(DESC0_ID(ct_bits), DESC_REUSABLE)



#endif
