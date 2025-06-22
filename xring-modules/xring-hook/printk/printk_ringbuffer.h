/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _KERNEL_PRINTK_RINGBUFFER_H
#define _KERNEL_PRINTK_RINGBUFFER_H

#include <linux/atomic.h>
#include <linux/dev_printk.h>

#if IS_ENABLED(CONFIG_XRING_DEBUG)
struct print_overflow_data {
	u64 ts_start;
	u64 ts_end;
	u64 char_count;
	int required_baud;
};
#define STANDARD_BAUDRATE 460800
#endif

#define PREFIX_LEN 24

/*
 * Meta information about each stored message.
 *
 * All fields are set by the printk code except for @seq, which is
 * set by the ringbuffer code.
 */
struct printk_info {
	u64	seq;		/* sequence number */
	u64	ts_nsec;	/* timestamp in nanoseconds */
	u16	text_len;	/* length of text message */
	u8	facility;	/* syslog facility */
	u8	flags:5;	/* internal record flags */
	u8	level:3;	/* syslog level */
	u32	caller_id;	/* thread id or processor id */

	struct dev_printk_info	dev_info;
};

/*
 * A structure providing the buffers, used by writers and readers.
 *
 * Writers:
 * Using prb_rec_init_wr(), a writer sets @text_buf_size before calling
 * prb_reserve(). On success, prb_reserve() sets @info and @text_buf to
 * buffers reserved for that writer.
 */
struct printk_record {
	struct printk_info	*info;
	char			*text_buf;
	unsigned int		text_buf_size;
};

/* Specifies the logical position and span of a data block. */
struct prb_data_blk_lpos {
	unsigned long	begin;
	unsigned long	next;
};

/*
 * A descriptor: the complete meta-data for a record.
 *
 * @state_var: A bitwise combination of descriptor ID and descriptor state.
 */
struct prb_desc {
	atomic_long_t			state_var;
	struct prb_data_blk_lpos	text_blk_lpos;
};

/* A ringbuffer of "ID + data" elements. */
struct prb_data_ring {
	unsigned int	size_bits;
	char		*data;
	atomic_long_t	head_lpos;
	atomic_long_t	tail_lpos;
};

/* A ringbuffer of "struct prb_desc" elements. */
struct prb_desc_ring {
	unsigned int		count_bits;
	struct prb_desc		*descs;
	struct printk_info	*infos;
	atomic_long_t		head_id;
	atomic_long_t		tail_id;
	atomic_long_t		last_finalized_id;
};

/*
 * The high level structure representing the printk ringbuffer.
 *
 * @fail: Count of failed prb_reserve() calls where not even a data-less
 *        record was created.
 */
struct printk_ringbuffer {
	struct prb_desc_ring	desc_ring;
	struct prb_data_ring	text_data_ring;
	atomic_long_t		fail;
};

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
struct prb_reserved_entry {
	struct printk_ringbuffer	*rb;
	unsigned long			irqflags;
	unsigned long			id;
	unsigned int			text_space;
};

/* The possible responses of a descriptor state-query. */
enum desc_state {
	desc_miss	=  -1,	/* ID mismatch (pseudo state) */
	desc_reserved	= 0x0,	/* reserved, in use by writer */
	desc_committed	= 0x1,	/* committed by writer, could get reopened */
	desc_finalized	= 0x2,	/* committed, no further modification allowed */
	desc_reusable	= 0x3,	/* free, not yet used by any writer */
};

#define _DATA_SIZE(sz_bits)	(1UL << (sz_bits))
#define _DESCS_COUNT(ct_bits)	(1U << (ct_bits))
#define DESC_SV_BITS		(sizeof(unsigned long) * 8)
#define DESC_FLAGS_SHIFT	(DESC_SV_BITS - 2)
#define DESC_FLAGS_MASK		(3UL << DESC_FLAGS_SHIFT)
#define DESC_STATE(sv)		(3UL & (sv >> DESC_FLAGS_SHIFT))
#define DESC_SV(id, state)	(((unsigned long)state << DESC_FLAGS_SHIFT) | id)
#define DESC_ID_MASK		(~DESC_FLAGS_MASK)
#define DESC_ID(sv)		((sv) & DESC_ID_MASK)
#define FAILED_LPOS		0x1
#define NO_LPOS			0x3

#define FAILED_BLK_LPOS	\
{				\
	.begin	= FAILED_LPOS,	\
	.next	= FAILED_LPOS,	\
}

#define BLK0_LPOS(sz_bits)	(-(_DATA_SIZE(sz_bits)))
#define DESC0_ID(ct_bits)	DESC_ID(-(_DESCS_COUNT(ct_bits) + 1))
#define DESC0_SV(ct_bits)	DESC_SV(DESC0_ID(ct_bits), desc_reusable)


/* Writer Interface */

/**
 * prb_rec_init_wr() - Initialize a buffer for writing records.
 *
 * @r:             The record to initialize.
 * @text_buf_size: The needed text buffer size.
 */
static inline void prb_rec_init_wr(struct printk_record *r,
				   unsigned int text_buf_size)
{
	r->info = NULL;
	r->text_buf = NULL;
	r->text_buf_size = text_buf_size;
}

bool prb_reserve_in_last(struct prb_reserved_entry *e, struct printk_ringbuffer *rb,
			 struct printk_record *r, u32 caller_id, unsigned int max_size);
void prb_commit(struct prb_reserved_entry *e);
void prb_final_commit(struct prb_reserved_entry *e);

#endif /* _KERNEL_PRINTK_RINGBUFFER_H */
