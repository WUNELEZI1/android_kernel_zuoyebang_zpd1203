// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/bug.h>
#include <linux/sched/signal.h>
#include <asm/io.h>

#include "xrisp_ringbuffer.h"

#define DATA_SIZE(data_ring)		_DATA_SIZE((data_ring)->size_bits)
#define DATA_SIZE_MASK(data_ring)	(DATA_SIZE(data_ring) - 1)

#define DESCS_COUNT(desc_ring)		_DESCS_COUNT((desc_ring)->count_bits)
#define DESCS_COUNT_MASK(desc_ring)	(DESCS_COUNT(desc_ring) - 1)

/* Determine the data array index from a logical position. */
#define DATA_INDEX(data_ring, lpos)	((lpos) & DATA_SIZE_MASK(data_ring))

/* Determine the desc array index from an ID or sequence number. */
#define DESC_INDEX(desc_ring, n)	((n) & DESCS_COUNT_MASK(desc_ring))

/* Determine how many times the data array has wrapped. */
#define DATA_WRAPS(data_ring, lpos)	((lpos) >> (data_ring)->size_bits)

/* Determine if a logical position refers to a data-less block. */
#define LPOS_DATALESS(lpos)		((lpos) & 1UL)
#define BLK_DATALESS(blk)		(LPOS_DATALESS((blk)->begin) && \
					 LPOS_DATALESS((blk)->next))

/* Get the logical position at index 0 of the current wrap. */
#define DATA_THIS_WRAP_START_LPOS(data_ring, lpos) \
((lpos) & ~DATA_SIZE_MASK(data_ring))

/* Get the ID for the same index of the previous wrap as the given ID. */
#define DESC_ID_PREV_WRAP(desc_ring, id) \
DESC_ID((id) - DESCS_COUNT(desc_ring))

/*
 * A data block: mapped directly to the beginning of the data block area
 * specified as a logical position within the data ring.
 *
 * @id:   the ID of the associated descriptor
 * @data: the writer data
 *
 * Note that the size of a data block is only known by its associated
 * descriptor.
 */
struct lrb_data_block {
	unsigned long	id;
	char		data[];
};

/*
 * descs/infos/log data buffer used in AP side
 * cannot use buffer addr read from lrb because it's addr of FW
 */
static struct lrb_desc    *ap_descs;
static struct lrb_info    *ap_infos;
static char *ap_data_rb;

/*
 * Return the descriptor associated with @n. @n can be either a
 * descriptor ID or a sequence number.
 */
static struct lrb_desc *to_desc(struct lrb_desc_ring *desc_ring, u64 n)
{
	return &ap_descs[DESC_INDEX(desc_ring, n)];
}

/*
 * Return the lrb_info associated with @n. @n can be either a
 * descriptor ID or a sequence number.
 */
static struct lrb_info *to_info(struct lrb_desc_ring *desc_ring, u64 n)
{
	return &ap_infos[DESC_INDEX(desc_ring, n)];
}

static struct lrb_data_block *to_block(struct lrb_data_ring *data_ring,
				       unsigned long begin_lpos)
{
	return (void *)&ap_data_rb[DATA_INDEX(data_ring, begin_lpos)];
}

/* Query the state of a descriptor. */
static enum desc_state get_desc_state(unsigned long id,
				      unsigned long state_val)
{
	if (atomic_read(&rproc_stop) == 1)
		return DESC_MISS;

	if (id != DESC_ID(state_val))
		return DESC_MISS;

	return DESC_STATE(state_val);
}

/*
 * Get a copy of a specified descriptor and return its queried state. If the
 * descriptor is in an inconsistent state (miss or reserved), the caller can
 * only expect the descriptor's @state_var field to be valid.
 *
 * The sequence number and thread_id can be optionally retrieved. Like all
 * non-state_var data, they are only valid if the descriptor is in a
 * consistent state.
 */
static enum desc_state desc_read(struct lrb_desc_ring *desc_ring,
				 unsigned long id, struct lrb_desc *desc_out,
				 u64 *seq_out, u32 *thread_id_out)
{
	if (atomic_read(&rproc_stop) == 1)
		return DESC_MISS;

	struct lrb_info *info = to_info(desc_ring, id);
	struct lrb_desc *desc = to_desc(desc_ring, id);
	//long *state_var = &desc->state_var;
	enum desc_state d_state;
	unsigned long state_val;

	if (atomic_read(&rproc_stop) == 1)
		return DESC_MISS;
	/* Check the descriptor state. */
	state_val = desc->state_var;
	d_state = get_desc_state(id, state_val);
	if (d_state == DESC_MISS || d_state == DESC_RESERVED) {
		/*
		 * The descriptor is in an inconsistent state. Set at least
		 * @state_var so that the caller can see the details of
		 * the inconsistent state.
		 */
		goto out;
	}
	if (atomic_read(&rproc_stop) == 1)
		return DESC_MISS;
	/*
	 * Copy the descriptor data. The data is not valid until the
	 * state has been re-checked. A memcpy() for all of @desc
	 * cannot be used because of the atomic_t @state_var field.
	 */
	if (desc_out) {
		memcpy_fromio(&desc_out->text_blk_lpos, &desc->text_blk_lpos,
		       sizeof(desc_out->text_blk_lpos));
	}
	if (atomic_read(&rproc_stop) == 1)
		return DESC_MISS;
	if (seq_out)
		*seq_out = info->seq; /* also part of desc_read:C */
	if (thread_id_out)
		*thread_id_out = info->thread_id; /* also part of desc_read:C */

	/*
	 * The data has been copied. Return the current descriptor state,
	 * which may have changed since the load above.
	 */
	state_val = desc->state_var;
	d_state = get_desc_state(id, state_val);
out:
	if (desc_out)
		desc_out->state_var = state_val;
	return d_state;
}

/*
 * Given @blk_lpos, return a pointer to the writer data from the data block
 * and calculate the size of the data part. A NULL pointer is returned if
 * @blk_lpos specifies values that could never be legal.
 *
 * This function (used by readers) performs strict validation on the lpos
 * values to possibly detect bugs in the writer code. A WARN_ON_ONCE() is
 * triggered if an internal error is detected.
 */
static const char *get_data(struct lrb_data_ring *data_ring,
			    struct lrb_data_blk_lpos *blk_lpos,
			    unsigned int *data_size)
{
	struct lrb_data_block *db;

#if RAMLOG_DEBUG
	ramlog_debug("begin:%lx, next:%lx\n", blk_lpos->begin, blk_lpos->next);
#endif
	/* Data-less data block description. */
	if (BLK_DATALESS(blk_lpos)) {
		if (blk_lpos->begin == NO_LPOS && blk_lpos->next == NO_LPOS) {
			*data_size = 0;
			return "";
		}
		return NULL;
	}
	if (atomic_read(&rproc_stop) == 1)
		return NULL;
	/* Regular data block: @begin less than @next and in same wrap. */
	if (DATA_WRAPS(data_ring, blk_lpos->begin) == DATA_WRAPS(data_ring, blk_lpos->next) &&
	    blk_lpos->begin < blk_lpos->next) {
		db = to_block(data_ring, blk_lpos->begin);
		*data_size = blk_lpos->next - blk_lpos->begin;

		/* Wrapping data block: @begin is one wrap behind @next. */
	} else if (DATA_WRAPS(data_ring, blk_lpos->begin + DATA_SIZE(data_ring)) ==
		   DATA_WRAPS(data_ring, blk_lpos->next)) {
		db = to_block(data_ring, 0);
		*data_size = DATA_INDEX(data_ring, blk_lpos->next);

		/* Illegal block description. */
	} else {
		//WARN_ON_ONCE(1);
		return NULL;
	}

	/* A valid data block will always be aligned to the ID size. */
	if (WARN_ON_ONCE(blk_lpos->begin != ALIGN(blk_lpos->begin, sizeof(db->id))) ||
	    WARN_ON_ONCE(blk_lpos->next != ALIGN(blk_lpos->next, sizeof(db->id)))) {
		return NULL;
	}

	/* A valid data block will always have at least an ID. */
	if (*data_size < sizeof(db->id))
		return NULL;

	/* Subtract block ID space from size to reflect data size. */
	*data_size -= sizeof(db->id);

	return &db->data[0];
}

/*
 * Count the number of lines in provided text. All text has at least 1 line
 * (even if @text_size is 0). Each '\n' processed is counted as an additional
 * line.
 */
static unsigned int count_lines(const char *text, unsigned int text_size)
{
	unsigned int next_size = text_size;
	unsigned int line_count = 1;
	const char *next = text;

	while (next_size) {
		next = memchr(next, '\n', next_size);
		if (!next)
			break;
		line_count++;
		next++;
		next_size = text_size - (next - text);
	}

	return line_count;
}

/*
 * Given @blk_lpos, copy an expected @len of data into the provided buffer.
 * If @line_count is provided, count the number of lines in the data.
 *
 * This function (used by readers) performs strict validation on the data
 * size to possibly detect bugs in the writer code. A WARN_ON_ONCE() is
 * triggered if an internal error is detected.
 */
static bool copy_data(struct lrb_data_ring *data_ring,
		      struct lrb_data_blk_lpos *blk_lpos, u16 len, char *buf,
		      unsigned int buf_size, unsigned int *line_count)
{
	unsigned int data_size;
	const char *data;

	/* Caller might not want any data. */
	if ((!buf || !buf_size) && !line_count)
		return true;

	if (atomic_read(&rproc_stop) == 1)
		return false;

	data = get_data(data_ring, blk_lpos, &data_size);
	if (!data)
		return false;
	/*
	 * Actual cannot be less than expected. It can be more than expected
	 * because of the trailing alignment padding.
	 *
	 * Note that invalid @len values can occur because the caller loads
	 * the value during an allowed data race.
	 */
	if (data_size < (unsigned int)len) {
		ramlog_warn("data_size:%d less than len:%d\n", data_size, len);
		return false;
	}

	/* Caller interested in the line count? */
	if (line_count)
		*line_count = count_lines(data, len);

	/* Caller interested in the data content? */
	if (!buf || !buf_size) {
		ramlog_warn("no buf or buf_size, return\n");
		return true;
	}

	data_size = min_t(u16, buf_size, len);

	if (atomic_read(&rproc_stop) == 1)
		return false;

	memcpy_fromio(&buf[0], data, data_size);
	return true;
}

/*
 * This is an extended version of desc_read(). It gets a copy of a specified
 * descriptor. However, it also verifies that the record is finalized and has
 * the sequence number @seq. On success, 0 is returned.
 *
 * Error return values:
 * -EINVAL: A finalized record with sequence number @seq does not exist.
 * -ENOENT: A finalized record with sequence number @seq exists, but its data
 *          is not available. This is a valid record, so readers should
 *          continue with the next record.
 */
static int desc_read_finalized_seq(struct lrb_desc_ring *desc_ring,
				   unsigned long id, u64 seq,
				   struct lrb_desc *desc_out)
{
	struct lrb_data_blk_lpos *blk_lpos = &desc_out->text_blk_lpos;
	enum desc_state d_state;
	u64 s;

	if (atomic_read(&rproc_stop) == 1)
		return -EINVAL;
	d_state = desc_read(desc_ring, id, desc_out, &s, NULL);

	/*
	 * An unexpected @id (DESC_MISS) or @seq mismatch means the record
	 * does not exist. A descriptor in the reserved or committed state
	 * means the record does not yet exist for the reader.
	 */
	if (d_state == DESC_MISS ||
	    d_state == DESC_RESERVED ||
	    d_state == DESC_COMMITTED ||
	    s != seq) {
#if RAMLOG_DEBUG
		ramlog_debug("seq:0x%lx, s:0x%lx, d_state:%d, EINVAL\n",
				seq, s, d_state);
#endif
		return -EINVAL;
	}

	/*
	 * A descriptor in the reusable state may no longer have its data
	 * available; report it as existing but with lost data. Or the record
	 * may actually be a record with lost data.
	 */
	if (d_state == DESC_REUSABLE ||
	    (blk_lpos->begin == FAILED_LPOS && blk_lpos->next == FAILED_LPOS)) {
#if RAMLOG_DEBUG
		ramlog_debug("id:%lx, seq:%lx, d_state:%d, begin:0x%lx, next:0x%lx, ENOENT\n",
				id, seq, d_state, blk_lpos->begin, blk_lpos->next);
#endif
		return -ENOENT;
	}

	return 0;
}

/*
 * Copy the ringbuffer data from the record with @seq to the provided
 * @r buffer. On success, 0 is returned.
 *
 * See desc_read_finalized_seq() for error return values.
 */
static int lrb_read(struct ramlog_ringbuffer *rb, u64 seq,
		    struct lrb_record *r, unsigned int *line_count)
{
	if (atomic_read(&rproc_stop) == 1)
		return -ENXIO;

	struct lrb_desc_ring *desc_ring = &rb->desc_ring;
	struct lrb_info *info = to_info(desc_ring, seq);
	struct lrb_desc *rdesc = to_desc(desc_ring, seq);
	struct lrb_desc desc;
	unsigned long id;
	int err;

#if RAMLOG_DEBUG
	ramlog_debug("seq:0x%x\n", seq);
#endif
	if (atomic_read(&rproc_stop) == 1)
		return -ENXIO;
	/* Extract the ID, used to specify the descriptor to read. */
	id = DESC_ID(rdesc->state_var);

	if (atomic_read(&rproc_stop) == 1)
		return -ENXIO;
	/* Get a local copy of the correct descriptor (if available). */
	err = desc_read_finalized_seq(desc_ring, id, seq, &desc);

	/*
	 * If @r is NULL, the caller is only interested in the availability
	 * of the record.
	 */
	if (err || !r)
		return err;

	if (atomic_read(&rproc_stop) == 1)
		return -ENOENT;
	/* If requested, copy meta data. */
	if (r->info)
		memcpy_fromio(r->info, info, sizeof(*(r->info)));

	/* Copy text data. If it fails, this is a data-less record. */
	if (!copy_data(&rb->text_data_ring, &desc.text_blk_lpos, info->text_len,
		       r->text_buf, r->text_buf_size, line_count)) {
		//ramlog_err("copy_data fail\n");
		return -ENOENT;
	}
	if (atomic_read(&rproc_stop) == 1)
		return -ENOENT;
	/* Ensure the record is still finalized and has the same @seq. */
	return desc_read_finalized_seq(desc_ring, id, seq, &desc);
}

/* Get the sequence number of the tail descriptor. */
static u64 lrb_first_seq(struct ramlog_ringbuffer *rb)
{
	struct lrb_desc_ring *desc_ring = &rb->desc_ring;
	enum desc_state d_state;
	struct lrb_desc desc;
	unsigned long start_id;
	u64 seq = 0;

	for (;;) {
		start_id = rb->desc_ring.tail_id;

		if (atomic_read(&rproc_stop) == 1)
			break;

		if (DESC_ID_PREV_WRAP(desc_ring, desc_ring->head_id + 1)
				== desc_ring->tail_id) {
			/*
			 * head caught up the tail
			 * left some descs entries for writer
			 */
			start_id += 20;
		}

		d_state = desc_read(desc_ring, start_id, &desc, &seq, NULL);

		/*
		 * This loop will not be infinite because the tail is
		 * _always_ in the finalized or reusable state.
		 */
		if (d_state == DESC_FINALIZED || d_state == DESC_REUSABLE)
			break;

		if (signal_pending(current) && check_and_handle_signals(current)) {
			ramlog_info("receive signal, exit get first seq\n");
			break;
		}
	}

	return seq;
}

/*
 * Non-blocking read of a record. Updates @seq to the last finalized record
 * (which may have no data available).
 *
 * See the description of lrb_read_valid() and lrb_read_valid_info()
 * for details.
 */
static bool _lrb_read_valid(struct ramlog_ringbuffer *rb, u64 *seq,
			    struct lrb_record *r, unsigned int *line_count)
{
	u64 tail_seq;
	int err;

	if (rb == NULL) {
		ramlog_info("rb is null, return");
		return false;
	}
	if (atomic_read(&rproc_stop) == 1)
		return false;
	while ((err = lrb_read(rb, *seq, r, line_count))) {
		if (signal_pending(current) && check_and_handle_signals(current)) {
			ramlog_info("receive signal, exit lrb read");
			return false;
		}
		if (err == -ENXIO)
			return false;
		if (atomic_read(&rproc_stop) == 1)
			return false;
		tail_seq = lrb_first_seq(rb);

		if (*seq < tail_seq) {
			/*
			 * Behind the tail. Catch up and try again. This
			 * can happen for -ENOENT and -EINVAL cases.
			 */
			*seq = tail_seq;

		} else if (err == -ENOENT) {
			/* Record exists, but no data available. Skip. */
			(*seq)++;

		} else {
			/* Non-existent/non-finalized record. Must stop. */
			return false;
		}
	}

	return true;
}

/**
 * lrb_read_valid() - Non-blocking read of a requested record or (if gone)
 *                    the next available record.
 *
 * @rb:  The ringbuffer to read from.
 * @seq: The sequence number of the record to read.
 * @r:   A record data buffer to store the read record to.
 *
 * This is the public function available to readers to read a record.
 *
 * The reader provides the @info and @text_buf buffers of @r to be
 * filled in. Any of the buffer pointers can be set to NULL if the reader
 * is not interested in that data. To ensure proper initialization of @r,
 * lrb_rec_init_rd() should be used.
 *
 * Return: true if a record was read, otherwise false.
 *
 * On success, the reader must check r->info.seq to see which record was
 * actually read. This allows the reader to detect dropped records.
 *
 * Failure means @seq refers to a not yet written record.
 */
bool lrb_read_valid(struct ramlog_ringbuffer *rb, u64 seq,
		    struct lrb_record *r)
{
	if (atomic_read(&rproc_stop) == 1)
		return 0;

	return _lrb_read_valid(rb, &seq, r, NULL);
}

bool lrb_read_valid_seq(struct ramlog_ringbuffer *rb, u64 *seq,
		    struct lrb_record *r)
{
	if (atomic_read(&rproc_stop) == 1)
		return 0;

	return _lrb_read_valid(rb, seq, r, NULL);
}

/**
 * lrb_read_valid_info() - Non-blocking read of meta data for a requested
 *                         record or (if gone) the next available record.
 *
 * @rb:         The ringbuffer to read from.
 * @seq:        The sequence number of the record to read.
 * @info:       A buffer to store the read record meta data to.
 * @line_count: A buffer to store the number of lines in the record text.
 *
 * This is the public function available to readers to read only the
 * meta data of a record.
 *
 * The reader provides the @info, @line_count buffers to be filled in.
 * Either of the buffer pointers can be set to NULL if the reader is not
 * interested in that data.
 *
 * Return: true if a record's meta data was read, otherwise false.
 *
 * On success, the reader must check info->seq to see which record meta data
 * was actually read. This allows the reader to detect dropped records.
 *
 * Failure means @seq refers to a not yet written record.
 */
bool lrb_read_valid_info(struct ramlog_ringbuffer *rb, u64 seq,
			 struct lrb_info *info, unsigned int *line_count)
{
	struct lrb_record r;

	lrb_rec_init_rd(&r, info, NULL, 0);

	return _lrb_read_valid(rb, &seq, &r, line_count);
}

/**
 * lrb_first_valid_seq() - Get the sequence number of the oldest available
 *                         record.
 *
 * @rb: The ringbuffer to get the sequence number from.
 *
 * This is the public function available to readers to see what the
 * first/oldest valid sequence number is.
 *
 * This provides readers a starting point to begin iterating the ringbuffer.
 *
 * Return: The sequence number of the first/oldest record or, if the
 *         ringbuffer is empty, 0 is returned.
 */
u64 lrb_first_valid_seq(struct ramlog_ringbuffer *rb)
{
	u64 seq = 0;

	if (!_lrb_read_valid(rb, &seq, NULL, NULL))
		return 0;

	return seq;
}

/**
 * lrb_next_seq() - Get the sequence number after the last available record.
 *
 * @rb:  The ringbuffer to get the sequence number from.
 *
 * This is the public function available to readers to see what the next
 * newest sequence number available to readers will be.
 *
 * This provides readers a sequence number to jump to if all currently
 * available records should be skipped.
 *
 * Return: The sequence number of the next newest (not yet available) record
 *         for readers.
 */
u64 lrb_next_seq(struct ramlog_ringbuffer *rb)
{
	struct lrb_desc_ring *desc_ring = &rb->desc_ring;
	enum desc_state d_state;
	unsigned long id;
	u64 seq;

	/* Check if the cached @id still points to a valid @seq. */
	id = desc_ring->last_finalized_id;
	d_state = desc_read(desc_ring, id, NULL, &seq, NULL);

	if (d_state == DESC_FINALIZED || d_state == DESC_REUSABLE) {
		/*
		 * Begin searching after the last finalized record.
		 *
		 * On 0, the search must begin at 0 because of hack#2
		 * of the bootstrapping phase it is not known if a
		 * record at index 0 exists.
		 */
		if (seq != 0)
			seq++;
	} else {
		/*
		 * The information about the last finalized sequence number
		 * has gone. It should happen only when there is a flood of
		 * new messages and the ringbuffer is rapidly recycled.
		 * Give up and start from the beginning.
		 */
		seq = 0;
	}

	/*
	 * The information about the last finalized @seq might be inaccurate.
	 * Search forward to find the current one.
	 */
	while (_lrb_read_valid(rb, &seq, NULL, NULL))
		seq++;

	return seq;
}

u64 lrb_get_first_seq(struct ramlog_ringbuffer *rb)
{
	ramlog_info("head_id:0x%lx, tail_id:0x%lx, last_finalized_id:0x%lx\n",
			rb->desc_ring.head_id, rb->desc_ring.tail_id,
			rb->desc_ring.last_finalized_id);
	if (rb->reset_flag && !rb->ack_flag) {
		rb->ack_flag = 1;
		ramlog_info("ISP reset occur, set syslog_seq to 0\n");
		return 0;
	} else {
		return lrb_first_seq(rb);
	}
}

/*
 * init lrb/ap_descs/ap_infos/ap_data_rb
 * should use AP addr for these buffers
 */
void xrisp_ramlog_ringbuffer_init(void *rb_addr, void *descs, void *infos)
{
	if (rb_addr && descs && infos) {
		ap_descs = (struct lrb_desc *)descs;
		ap_infos = (struct lrb_info *)infos;
		ap_data_rb = (char *)rb_addr;

		ramlog_info("ramlog: ap_descs:%pK, ap_infos:%pK, ap_data_rb:%pK\n",
				ap_descs, ap_infos, ap_data_rb);
	} else {
		ramlog_err("ramlog buffer not ready, init fail\n");
	}
}
