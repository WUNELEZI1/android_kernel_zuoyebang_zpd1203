/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_RINGBUFFER_H
#define _XRISP_RINGBUFFER_H

#include <dt-bindings/xring/platform-specific/xrisp_base.h>
#include "xrisp_log.h"

extern bool check_and_handle_signals(struct task_struct *task);
extern atomic_t rproc_stop;

#define FAILED_BLK_LPOS	\
{				\
	.begin	= FAILED_LPOS,	\
	.next	= FAILED_LPOS,	\
}

#define ramlog_err(fmt, ...) \
	XRISP_PR_ERROR("[XRISP_DRV][edr][ramlog] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramlog_warn(fmt, ...) \
	XRISP_PR_WARN("[XRISP_DRV][edr][ramlog] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramlog_info(fmt, ...) \
	XRISP_PR_INFO("[XRISP_DRV][edr][ramlog] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramlog_debug(fmt, ...) \
	XRISP_PR_DEBUG("[XRISP_DRV][edr][ramlog] %s: "fmt, __func__, ##__VA_ARGS__)

#define RAMLOG_DEBUG	0

/*
 * Used by writers as a reserve/commit handle.
 *
 * @rb:         Ringbuffer where the entry is reserved.
 * @irqflags:   Saved irq flags to restore on entry commit.
 * @id:         ID of the reserved descriptor.
 * @text_space: Total occupied buffer space in the text data ring, including
 *              ID, alignment padding, and wrapping data blocks.
 *
 * This structure is an opaque handle for writers. Its contents are only
 * to be used by the ringbuffer implementation.
 */
struct lrb_reserved_entry {
	struct ramlog_ringbuffer	*rb;
	unsigned long			irqflags;
	unsigned long			id;
	unsigned int			text_space;
};

/* Reader Interface */

/**
 * lrb_rec_init_rd() - Initialize a buffer for reading records.
 *
 * @r:             The record to initialize.
 * @info:          A buffer to store record meta-data.
 * @text_buf:      A buffer to store text data.
 * @text_buf_size: The size of @text_buf.
 *
 * Initialize all the fields that a reader is interested in. All arguments
 * (except @r) are optional. Only record data for arguments that are
 * non-NULL or non-zero will be read.
 */
static inline void lrb_rec_init_rd(struct lrb_record *r,
				   struct lrb_info *info,
				   char *text_buf, unsigned int text_buf_size)
{
	r->info = info;
	r->text_buf = text_buf;
	r->text_buf_size = text_buf_size;
}

/**
 * lrb_for_each_record() - Iterate over the records of a ringbuffer.
 *
 * @from: The sequence number to begin with.
 * @rb:   The ringbuffer to iterate over.
 * @s:    A u64 to store the sequence number on each iteration.
 * @r:    A lrb_record to store the record on each iteration.
 *
 * This is a macro for conveniently iterating over a ringbuffer.
 * Note that @s may not be the sequence number of the record on each
 * iteration. For the sequence number, @r->info->seq should be checked.
 *
 */
#define lrb_for_each_record(from, rb, s, r) \
for ((s) = from; lrb_read_valid(rb, s, r); (s) = (r)->info->seq + 1)

/**
 * lrb_for_each_info() - Iterate over the meta data of a ringbuffer.
 *
 * @from: The sequence number to begin with.
 * @rb:   The ringbuffer to iterate over.
 * @s:    A u64 to store the sequence number on each iteration.
 * @i:    A lrb_info to store the record meta data on each iteration.
 * @lc:   An unsigned int to store the text line count of each record.
 *
 * This is a macro for conveniently iterating over a ringbuffer.
 * Note that @s may not be the sequence number of the record on each
 * iteration. For the sequence number, @r->info->seq should be checked.
 *
 */
#define lrb_for_each_info(from, rb, s, i, lc) \
for ((s) = from; lrb_read_valid_info(rb, s, i, lc); (s) = (i)->seq + 1)

bool lrb_read_valid(struct ramlog_ringbuffer *rb, u64 seq,
		    struct lrb_record *r);
bool lrb_read_valid_seq(struct ramlog_ringbuffer *rb, u64 *seq, struct lrb_record *r);
bool lrb_read_valid_info(struct ramlog_ringbuffer *rb, u64 seq,
			 struct lrb_info *info, unsigned int *line_count);

u64 lrb_first_valid_seq(struct ramlog_ringbuffer *rb);
u64 lrb_next_seq(struct ramlog_ringbuffer *rb);
u64 lrb_get_first_seq(struct ramlog_ringbuffer *rb);

void xrisp_ramlog_ringbuffer_init(void *rb_addr, void *descs, void *infos);
#endif
