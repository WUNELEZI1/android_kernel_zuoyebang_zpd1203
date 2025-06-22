// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/irqflags.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/bug.h>
#include "printk_ringbuffer.h"

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
struct prb_data_block {
	unsigned long	id;
	char		data[];
};

/*
 * Return the descriptor associated with @n. @n can be either a
 * descriptor ID or a sequence number.
 */
static struct prb_desc *to_desc(struct prb_desc_ring *desc_ring, u64 n)
{
	return &desc_ring->descs[DESC_INDEX(desc_ring, n)];
}

/*
 * Return the printk_info associated with @n. @n can be either a
 * descriptor ID or a sequence number.
 */
static struct printk_info *to_info(struct prb_desc_ring *desc_ring, u64 n)
{
	return &desc_ring->infos[DESC_INDEX(desc_ring, n)];
}

static struct prb_data_block *to_block(struct prb_data_ring *data_ring,
				       unsigned long begin_lpos)
{
	return (void *)&data_ring->data[DATA_INDEX(data_ring, begin_lpos)];
}

/*
 * Increase the data size to account for data block meta data plus any
 * padding so that the adjacent data block is aligned on the ID size.
 */
static unsigned int to_blk_size(unsigned int size)
{
	struct prb_data_block *db = NULL;

	size += sizeof(*db);
	size = ALIGN(size, sizeof(db->id));
	return size;
}

/*
 * Sanity checker for reserve size. The ringbuffer code assumes that a data
 * block does not exceed the maximum possible size that could fit within the
 * ringbuffer. This function provides that basic size check so that the
 * assumption is safe.
 */
static bool data_check_size(struct prb_data_ring *data_ring, unsigned int size)
{
	struct prb_data_block *db = NULL;

	if (size == 0)
		return true;

	/*
	 * Ensure the alignment padded size could possibly fit in the data
	 * array. The largest possible data block must still leave room for
	 * at least the ID of the next block.
	 */
	size = to_blk_size(size);
	if (size > DATA_SIZE(data_ring) - sizeof(db->id))
		return false;

	return true;
}

/* Query the state of a descriptor. */
static enum desc_state get_desc_state(unsigned long id,
				      unsigned long state_val)
{
	if (id != DESC_ID(state_val))
		return desc_miss;

	return DESC_STATE(state_val);
}

/*
 * Get a copy of a specified descriptor and return its queried state. If the
 * descriptor is in an inconsistent state (miss or reserved), the caller can
 * only expect the descriptor's @state_var field to be valid.
 *
 * The sequence number and caller_id can be optionally retrieved. Like all
 * non-state_var data, they are only valid if the descriptor is in a
 * consistent state.
 */
static enum desc_state desc_read(struct prb_desc_ring *desc_ring,
				 unsigned long id, struct prb_desc *desc_out,
				 u64 *seq_out, u32 *caller_id_out)
{
	struct printk_info *info = to_info(desc_ring, id);
	struct prb_desc *desc = to_desc(desc_ring, id);
	atomic_long_t *state_var = &desc->state_var;
	enum desc_state d_state;
	unsigned long state_val;

	/* Check the descriptor state. */
	state_val = atomic_long_read(state_var); /* LMM(desc_read:A) */
	d_state = get_desc_state(id, state_val);
	if (d_state == desc_miss || d_state == desc_reserved) {
		/*
		 * The descriptor is in an inconsistent state. Set at least
		 * @state_var so that the caller can see the details of
		 * the inconsistent state.
		 */
		goto out;
	}

	/*
	 * Guarantee the state is loaded before copying the descriptor
	 * content. This avoids copying obsolete descriptor content that might
	 * not apply to the descriptor state. This pairs with _prb_commit:B.
	 *
	 * Memory barrier involvement:
	 *
	 * If desc_read:A reads from _prb_commit:B, then desc_read:C reads
	 * from _prb_commit:A.
	 *
	 * Relies on:
	 *
	 * WMB from _prb_commit:A to _prb_commit:B
	 *    matching
	 * RMB from desc_read:A to desc_read:C
	 */
	smp_rmb(); /* LMM(desc_read:B) */

	/*
	 * Copy the descriptor data. The data is not valid until the
	 * state has been re-checked. A memcpy() for all of @desc
	 * cannot be used because of the atomic_t @state_var field.
	 */
	if (desc_out) {
		memcpy(&desc_out->text_blk_lpos, &desc->text_blk_lpos,
		       sizeof(desc_out->text_blk_lpos)); /* LMM(desc_read:C) */
	}
	if (seq_out)
		*seq_out = info->seq; /* also part of desc_read:C */
	if (caller_id_out)
		*caller_id_out = info->caller_id; /* also part of desc_read:C */

	/*
	 * 1. Guarantee the descriptor content is loaded before re-checking
	 *    the state. This avoids reading an obsolete descriptor state
	 *    that may not apply to the copied content. This pairs with
	 *    desc_reserve:F.
	 *
	 *    Memory barrier involvement:
	 *
	 *    If desc_read:C reads from desc_reserve:G, then desc_read:E
	 *    reads from desc_reserve:F.
	 *
	 *    Relies on:
	 *
	 *    WMB from desc_reserve:F to desc_reserve:G
	 *       matching
	 *    RMB from desc_read:C to desc_read:E
	 *
	 * 2. Guarantee the record data is loaded before re-checking the
	 *    state. This avoids reading an obsolete descriptor state that may
	 *    not apply to the copied data. This pairs with data_alloc:A and
	 *    data_realloc:A.
	 *
	 *    Memory barrier involvement:
	 *
	 *    If copy_data:A reads from data_alloc:B, then desc_read:E
	 *    reads from desc_make_reusable:A.
	 *
	 *    Relies on:
	 *
	 *    MB from desc_make_reusable:A to data_alloc:B
	 *       matching
	 *    RMB from desc_read:C to desc_read:E
	 *
	 *    Note: desc_make_reusable:A and data_alloc:B can be different
	 *          CPUs. However, the data_alloc:B CPU (which performs the
	 *          full memory barrier) must have previously seen
	 *          desc_make_reusable:A.
	 */
	smp_rmb(); /* LMM(desc_read:D) */

	/*
	 * The data has been copied. Return the current descriptor state,
	 * which may have changed since the load above.
	 */
	state_val = atomic_long_read(state_var); /* LMM(desc_read:E) */
	d_state = get_desc_state(id, state_val);
out:
	if (desc_out)
		atomic_long_set(&desc_out->state_var, state_val);
	return d_state;
}

/*
 * Take a specified descriptor out of the finalized state by attempting
 * the transition from finalized to reusable. Either this context or some
 * other context will have been successful.
 */
static void desc_make_reusable(struct prb_desc_ring *desc_ring,
			       unsigned long id)
{
	unsigned long val_finalized = DESC_SV(id, desc_finalized);
	unsigned long val_reusable = DESC_SV(id, desc_reusable);
	struct prb_desc *desc = to_desc(desc_ring, id);
	atomic_long_t *state_var = &desc->state_var;

	atomic_long_cmpxchg_relaxed(state_var, val_finalized,
				    val_reusable); /* LMM(desc_make_reusable:A) */
}

/*
 * Given the text data ring, put the associated descriptor of each
 * data block from @lpos_begin until @lpos_end into the reusable state.
 *
 * If there is any problem making the associated descriptor reusable, either
 * the descriptor has not yet been finalized or another writer context has
 * already pushed the tail lpos past the problematic data block. Regardless,
 * on error the caller can re-load the tail lpos to determine the situation.
 */
static bool data_make_reusable(struct printk_ringbuffer *rb,
			       unsigned long lpos_begin,
			       unsigned long lpos_end,
			       unsigned long *lpos_out)
{

	struct prb_data_ring *data_ring = &rb->text_data_ring;
	struct prb_desc_ring *desc_ring = &rb->desc_ring;
	struct prb_data_block *blk;
	enum desc_state d_state;
	struct prb_desc desc;
	struct prb_data_blk_lpos *blk_lpos = &desc.text_blk_lpos;
	unsigned long id;

	/* Loop until @lpos_begin has advanced to or beyond @lpos_end. */
	while ((lpos_end - lpos_begin) - 1 < DATA_SIZE(data_ring)) {
		blk = to_block(data_ring, lpos_begin);

		/*
		 * Load the block ID from the data block. This is a data race
		 * against a writer that may have newly reserved this data
		 * area. If the loaded value matches a valid descriptor ID,
		 * the blk_lpos of that descriptor will be checked to make
		 * sure it points back to this data block. If the check fails,
		 * the data area has been recycled by another writer.
		 */
		id = blk->id; /* LMM(data_make_reusable:A) */

		d_state = desc_read(desc_ring, id, &desc,
				    NULL, NULL); /* LMM(data_make_reusable:B) */

		switch (d_state) {
		case desc_miss:
		case desc_reserved:
		case desc_committed:
			return false;
		case desc_finalized:
			/*
			 * This data block is invalid if the descriptor
			 * does not point back to it.
			 */
			if (blk_lpos->begin != lpos_begin)
				return false;
			desc_make_reusable(desc_ring, id);
			break;
		case desc_reusable:
			/*
			 * This data block is invalid if the descriptor
			 * does not point back to it.
			 */
			if (blk_lpos->begin != lpos_begin)
				return false;
			break;
		}

		/* Advance @lpos_begin to the next data block. */
		lpos_begin = blk_lpos->next;
	}

	*lpos_out = lpos_begin;
	return true;
}

/*
 * Advance the data ring tail to at least @lpos. This function puts
 * descriptors into the reusable state if the tail is pushed beyond
 * their associated data block.
 */
static bool data_push_tail(struct printk_ringbuffer *rb, unsigned long lpos)
{
	struct prb_data_ring *data_ring = &rb->text_data_ring;
	unsigned long tail_lpos_new;
	unsigned long tail_lpos;
	unsigned long next_lpos;

	/* If @lpos is from a data-less block, there is nothing to do. */
	if (LPOS_DATALESS(lpos))
		return true;

	/*
	 * Any descriptor states that have transitioned to reusable due to the
	 * data tail being pushed to this loaded value will be visible to this
	 * CPU. This pairs with data_push_tail:D.
	 *
	 * Memory barrier involvement:
	 *
	 * If data_push_tail:A reads from data_push_tail:D, then this CPU can
	 * see desc_make_reusable:A.
	 *
	 * Relies on:
	 *
	 * MB from desc_make_reusable:A to data_push_tail:D
	 *    matches
	 * READFROM from data_push_tail:D to data_push_tail:A
	 *    thus
	 * READFROM from desc_make_reusable:A to this CPU
	 */
	tail_lpos = atomic_long_read(&data_ring->tail_lpos); /* LMM(data_push_tail:A) */

	/*
	 * Loop until the tail lpos is at or beyond @lpos. This condition
	 * may already be satisfied, resulting in no full memory barrier
	 * from data_push_tail:D being performed. However, since this CPU
	 * sees the new tail lpos, any descriptor states that transitioned to
	 * the reusable state must already be visible.
	 */
	while ((lpos - tail_lpos) - 1 < DATA_SIZE(data_ring)) {
		/*
		 * Make all descriptors reusable that are associated with
		 * data blocks before @lpos.
		 */
		if (!data_make_reusable(rb, tail_lpos, lpos, &next_lpos)) {
			/*
			 * 1. Guarantee the block ID loaded in
			 *    data_make_reusable() is performed before
			 *    reloading the tail lpos. The failed
			 *    data_make_reusable() may be due to a newly
			 *    recycled data area causing the tail lpos to
			 *    have been previously pushed. This pairs with
			 *    data_alloc:A and data_realloc:A.
			 *
			 *    Memory barrier involvement:
			 *
			 *    If data_make_reusable:A reads from data_alloc:B,
			 *    then data_push_tail:C reads from
			 *    data_push_tail:D.
			 *
			 *    Relies on:
			 *
			 *    MB from data_push_tail:D to data_alloc:B
			 *       matching
			 *    RMB from data_make_reusable:A to
			 *    data_push_tail:C
			 *
			 *    Note: data_push_tail:D and data_alloc:B can be
			 *          different CPUs. However, the data_alloc:B
			 *          CPU (which performs the full memory
			 *          barrier) must have previously seen
			 *          data_push_tail:D.
			 *
			 * 2. Guarantee the descriptor state loaded in
			 *    data_make_reusable() is performed before
			 *    reloading the tail lpos. The failed
			 *    data_make_reusable() may be due to a newly
			 *    recycled descriptor causing the tail lpos to
			 *    have been previously pushed. This pairs with
			 *    desc_reserve:D.
			 *
			 *    Memory barrier involvement:
			 *
			 *    If data_make_reusable:B reads from
			 *    desc_reserve:F, then data_push_tail:C reads
			 *    from data_push_tail:D.
			 *
			 *    Relies on:
			 *
			 *    MB from data_push_tail:D to desc_reserve:F
			 *       matching
			 *    RMB from data_make_reusable:B to
			 *    data_push_tail:C
			 *
			 *    Note: data_push_tail:D and desc_reserve:F can
			 *          be different CPUs. However, the
			 *          desc_reserve:F CPU (which performs the
			 *          full memory barrier) must have previously
			 *          seen data_push_tail:D.
			 */
			smp_rmb(); /* LMM(data_push_tail:B) */

			tail_lpos_new = atomic_long_read(&data_ring->tail_lpos
							); /* LMM(data_push_tail:C) */
			if (tail_lpos_new == tail_lpos)
				return false;

			/* Another CPU pushed the tail. Try again. */
			tail_lpos = tail_lpos_new;
			continue;
		}

		/*
		 * Guarantee any descriptor states that have transitioned to
		 * reusable are stored before pushing the tail lpos. A full
		 * memory barrier is needed since other CPUs may have made
		 * the descriptor states reusable. This pairs with
		 * data_push_tail:A.
		 */
		if (atomic_long_try_cmpxchg(&data_ring->tail_lpos, &tail_lpos,
					    next_lpos)) { /* LMM(data_push_tail:D) */
			break;
		}
	}

	return true;
}

/* Determine the end of a data block. */
static unsigned long get_next_lpos(struct prb_data_ring *data_ring,
				   unsigned long lpos, unsigned int size)
{
	unsigned long begin_lpos;
	unsigned long next_lpos;

	begin_lpos = lpos;
	next_lpos = lpos + size;

	/* First check if the data block does not wrap. */
	if (DATA_WRAPS(data_ring, begin_lpos) == DATA_WRAPS(data_ring, next_lpos))
		return next_lpos;

	/* Wrapping data blocks store their data at the beginning. */
	return (DATA_THIS_WRAP_START_LPOS(data_ring, next_lpos) + size);
}

/*
 * Allocate a new data block, invalidating the oldest data block(s)
 * if necessary. This function also associates the data block with
 * a specified descriptor.
 */
static char *data_alloc(struct printk_ringbuffer *rb, unsigned int size,
			struct prb_data_blk_lpos *blk_lpos, unsigned long id)
{
	struct prb_data_ring *data_ring = &rb->text_data_ring;
	struct prb_data_block *blk;
	unsigned long begin_lpos;
	unsigned long next_lpos;

	if (size == 0) {
		/* Specify a data-less block. */
		blk_lpos->begin = NO_LPOS;
		blk_lpos->next = NO_LPOS;
		return NULL;
	}

	size = to_blk_size(size);

	begin_lpos = atomic_long_read(&data_ring->head_lpos);

	do {
		next_lpos = get_next_lpos(data_ring, begin_lpos, size);

		if (!data_push_tail(rb, next_lpos - DATA_SIZE(data_ring))) {
			/* Failed to allocate, specify a data-less block. */
			blk_lpos->begin = FAILED_LPOS;
			blk_lpos->next = FAILED_LPOS;
			return NULL;
		}

		/*
		 * 1. Guarantee any descriptor states that have transitioned
		 *    to reusable are stored before modifying the newly
		 *    allocated data area. A full memory barrier is needed
		 *    since other CPUs may have made the descriptor states
		 *    reusable. See data_push_tail:A about why the reusable
		 *    states are visible. This pairs with desc_read:D.
		 *
		 * 2. Guarantee any updated tail lpos is stored before
		 *    modifying the newly allocated data area. Another CPU may
		 *    be in data_make_reusable() and is reading a block ID
		 *    from this area. data_make_reusable() can handle reading
		 *    a garbage block ID value, but then it must be able to
		 *    load a new tail lpos. A full memory barrier is needed
		 *    since other CPUs may have updated the tail lpos. This
		 *    pairs with data_push_tail:B.
		 */
	} while (!atomic_long_try_cmpxchg(&data_ring->head_lpos, &begin_lpos,
					  next_lpos)); /* LMM(data_alloc:A) */

	blk = to_block(data_ring, begin_lpos);
	blk->id = id; /* LMM(data_alloc:B) */

	if (DATA_WRAPS(data_ring, begin_lpos) != DATA_WRAPS(data_ring, next_lpos)) {
		/* Wrapping data blocks store their data at the beginning. */
		blk = to_block(data_ring, 0);

		/*
		 * Store the ID on the wrapped block for consistency.
		 * The printk_ringbuffer does not actually use it.
		 */
		blk->id = id;
	}

	blk_lpos->begin = begin_lpos;
	blk_lpos->next = next_lpos;

	return &blk->data[0];
}

/*
 * Try to resize an existing data block associated with the descriptor
 * specified by @id. If the resized data block should become wrapped, it
 * copies the old data to the new data block. If @size yields a data block
 * with the same or less size, the data block is left as is.
 *
 * Fail if this is not the last allocated data block or if there is not
 * enough space or it is not possible make enough space.
 *
 * Return a pointer to the beginning of the entire data buffer or NULL on
 * failure.
 */
static char *data_realloc(struct printk_ringbuffer *rb, unsigned int size,
			  struct prb_data_blk_lpos *blk_lpos, unsigned long id)
{
	struct prb_data_ring *data_ring = &rb->text_data_ring;
	struct prb_data_block *blk;
	unsigned long head_lpos;
	unsigned long next_lpos;
	bool wrapped;

	/* Reallocation only works if @blk_lpos is the newest data block. */
	head_lpos = atomic_long_read(&data_ring->head_lpos);
	if (head_lpos != blk_lpos->next)
		return NULL;

	/* Keep track if @blk_lpos was a wrapping data block. */
	wrapped = (DATA_WRAPS(data_ring, blk_lpos->begin) != DATA_WRAPS(data_ring, blk_lpos->next));

	size = to_blk_size(size);

	next_lpos = get_next_lpos(data_ring, blk_lpos->begin, size);

	/* If the data block does not increase, there is nothing to do. */
	if (head_lpos - next_lpos < DATA_SIZE(data_ring)) {
		if (wrapped)
			blk = to_block(data_ring, 0);
		else
			blk = to_block(data_ring, blk_lpos->begin);
		return &blk->data[0];
	}

	if (!data_push_tail(rb, next_lpos - DATA_SIZE(data_ring)))
		return NULL;

	/* The memory barrier involvement is the same as data_alloc:A. */
	if (!atomic_long_try_cmpxchg(&data_ring->head_lpos, &head_lpos,
				     next_lpos)) { /* LMM(data_realloc:A) */
		return NULL;
	}

	blk = to_block(data_ring, blk_lpos->begin);

	if (DATA_WRAPS(data_ring, blk_lpos->begin) != DATA_WRAPS(data_ring, next_lpos)) {
		struct prb_data_block *old_blk = blk;

		/* Wrapping data blocks store their data at the beginning. */
		blk = to_block(data_ring, 0);

		/*
		 * Store the ID on the wrapped block for consistency.
		 * The printk_ringbuffer does not actually use it.
		 */
		blk->id = id;

		if (!wrapped) {
			/*
			 * Since the allocated space is now in the newly
			 * created wrapping data block, copy the content
			 * from the old data block.
			 */
			memcpy(&blk->data[0], &old_blk->data[0],
			       (blk_lpos->next - blk_lpos->begin) - sizeof(blk->id));
		}
	}

	blk_lpos->next = next_lpos;

	return &blk->data[0];
}

/* Return the number of bytes used by a data block. */
static unsigned int space_used(struct prb_data_ring *data_ring,
			       struct prb_data_blk_lpos *blk_lpos)
{
	/* Data-less blocks take no space. */
	if (BLK_DATALESS(blk_lpos))
		return 0;

	if (DATA_WRAPS(data_ring, blk_lpos->begin) == DATA_WRAPS(data_ring, blk_lpos->next)) {
		/* Data block does not wrap. */
		return (DATA_INDEX(data_ring, blk_lpos->next) -
			DATA_INDEX(data_ring, blk_lpos->begin));
	}

	/*
	 * For wrapping data blocks, the trailing (wasted) space is
	 * also counted.
	 */
	return (DATA_INDEX(data_ring, blk_lpos->next) +
		DATA_SIZE(data_ring) - DATA_INDEX(data_ring, blk_lpos->begin));
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
static const char *get_data(struct prb_data_ring *data_ring,
			    struct prb_data_blk_lpos *blk_lpos,
			    unsigned int *data_size)
{
	struct prb_data_block *db;

	/* Data-less data block description. */
	if (BLK_DATALESS(blk_lpos)) {
		if (blk_lpos->begin == NO_LPOS && blk_lpos->next == NO_LPOS) {
			*data_size = 0;
			return "";
		}
		return NULL;
	}

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
		WARN_ON_ONCE(1);
		return NULL;
	}

	/* A valid data block will always be aligned to the ID size. */
	if (WARN_ON_ONCE(blk_lpos->begin != ALIGN(blk_lpos->begin, sizeof(db->id))) ||
	    WARN_ON_ONCE(blk_lpos->next != ALIGN(blk_lpos->next, sizeof(db->id)))) {
		return NULL;
	}

	/* A valid data block will always have at least an ID. */
	if (WARN_ON_ONCE(*data_size < sizeof(db->id)))
		return NULL;

	/* Subtract block ID space from size to reflect data size. */
	*data_size -= sizeof(db->id);

	return &db->data[0];
}

/*
 * Attempt to transition the newest descriptor from committed back to reserved
 * so that the record can be modified by a writer again. This is only possible
 * if the descriptor is not yet finalized and the provided @caller_id matches.
 */
static struct prb_desc *desc_reopen_last(struct prb_desc_ring *desc_ring,
					 u32 caller_id, unsigned long *id_out)
{
	unsigned long prev_state_val[2];
	enum desc_state d_state;
	struct prb_desc desc;
	struct prb_desc *d;
	unsigned long id;
	u32 cid;

	id = atomic_long_read(&desc_ring->head_id);

	/*
	 * To reduce unnecessarily reopening, first check if the descriptor
	 * state and caller ID are correct.
	 */
	d_state = desc_read(desc_ring, id, &desc, NULL, &cid);
	if ((d_state != desc_committed && d_state != desc_finalized) || cid != caller_id)
		return NULL;

	d = to_desc(desc_ring, id);

	prev_state_val[0] = DESC_SV(id, desc_committed);
	prev_state_val[1] = DESC_SV(id, desc_finalized);

	/*
	 * Guarantee the reserved state is stored before reading any
	 * record data. A full memory barrier is needed because @state_var
	 * modification is followed by reading. This pairs with _prb_commit:B.
	 *
	 * Memory barrier involvement:
	 *
	 * If desc_reopen_last:A reads from _prb_commit:B, then
	 * prb_reserve_in_last:A reads from _prb_commit:A.
	 *
	 * Relies on:
	 *
	 * WMB from _prb_commit:A to _prb_commit:B
	 *    matching
	 * MB If desc_reopen_last:A to prb_reserve_in_last:A
	 */
	if ((!atomic_long_try_cmpxchg(&d->state_var, &prev_state_val[0],
					DESC_SV(id, desc_reserved)))
			&& (!atomic_long_try_cmpxchg(&d->state_var, &prev_state_val[1],
					DESC_SV(id, desc_reserved)))) { /* LMM(desc_reopen_last:A) */
		return NULL;
	}

	*id_out = id;
	return d;
}

/**
 * prb_reserve_in_last() - Re-reserve and extend the space in the ringbuffer
 *                         used by the newest record.
 *
 * @e:         The entry structure to setup.
 * @rb:        The ringbuffer to re-reserve and extend data in.
 * @r:         The record structure to allocate buffers for.
 * @caller_id: The caller ID of the caller (reserving writer).
 * @max_size:  Fail if the extended size would be greater than this.
 *
 * This is the public function available to writers to re-reserve and extend
 * data.
 *
 * The writer specifies the text size to extend (not the new total size) by
 * setting the @text_buf_size field of @r. To ensure proper initialization
 * of @r, prb_rec_init_wr() should be used.
 *
 * This function will fail if @caller_id does not match the caller ID of the
 * newest record. In that case the caller must reserve new data using
 * prb_reserve().
 *
 * Context: Any context. Disables local interrupts on success.
 * Return: true if text data could be extended, otherwise false.
 *
 * On success:
 *
 *   - @r->text_buf points to the beginning of the entire text buffer.
 *
 *   - @r->text_buf_size is set to the new total size of the buffer.
 *
 *   - @r->info is not touched so that @r->info->text_len could be used
 *     to append the text.
 *
 *   - prb_record_text_space() can be used on @e to query the new
 *     actually used space.
 *
 * Important: All @r->info fields will already be set with the current values
 *            for the record. I.e. @r->info->text_len will be less than
 *            @text_buf_size. Writers can use @r->info->text_len to know
 *            where concatenation begins and writers should update
 *            @r->info->text_len after concatenating.
 */
bool prb_reserve_in_last(struct prb_reserved_entry *e, struct printk_ringbuffer *rb,
			 struct printk_record *r, u32 caller_id, unsigned int max_size)
{
	struct prb_desc_ring *desc_ring = &rb->desc_ring;
	struct printk_info *info;
	unsigned int data_size;
	struct prb_desc *d;
	unsigned long id;

	local_irq_save(e->irqflags);

	/* Transition the newest descriptor back to the reserved state. */
	d = desc_reopen_last(desc_ring, caller_id, &id);
	if (!d) {
		local_irq_restore(e->irqflags);
		goto fail_reopen;
	}

	/* Now the writer has exclusive access: LMM(prb_reserve_in_last:A) */

	info = to_info(desc_ring, id);

	/*
	 * Set the @e fields here so that prb_commit() can be used if
	 * anything fails from now on.
	 */
	e->rb = rb;
	e->id = id;

	/*
	 * desc_reopen_last() checked the caller_id, but there was no
	 * exclusive access at that point. The descriptor may have
	 * changed since then.
	 */
	if (caller_id != info->caller_id)
		goto fail;

	if (BLK_DATALESS(&d->text_blk_lpos)) {
		if (WARN_ON_ONCE(info->text_len != 0)) {
			pr_warn_once("wrong text_len value (%hu, expecting 0)\n",
				     info->text_len);
			info->text_len = 0;
		}

		if (!data_check_size(&rb->text_data_ring, r->text_buf_size))
			goto fail;

		if (r->text_buf_size > max_size)
			goto fail;

		r->text_buf = data_alloc(rb, r->text_buf_size,
					 &d->text_blk_lpos, id);
	} else {
		if (!get_data(&rb->text_data_ring, &d->text_blk_lpos, &data_size))
			goto fail;

		/*
		 * Increase the buffer size to include the original size. If
		 * the meta data (@text_len) is not sane, use the full data
		 * block size.
		 */
		if (WARN_ON_ONCE(info->text_len > data_size)) {
			pr_warn_once("wrong text_len value (%hu, expecting <=%u)\n",
				     info->text_len, data_size);
			info->text_len = data_size;
		}
		r->text_buf_size += info->text_len;

		if (!data_check_size(&rb->text_data_ring, r->text_buf_size))
			goto fail;

		if (r->text_buf_size > max_size)
			goto fail;

		r->text_buf = data_realloc(rb, r->text_buf_size,
					   &d->text_blk_lpos, id);
	}
	if (r->text_buf_size && !r->text_buf)
		goto fail;

	r->info = info;

	e->text_space = space_used(&rb->text_data_ring, &d->text_blk_lpos);

	return true;
fail:
	prb_commit(e);
	/* prb_commit() re-enabled interrupts. */
fail_reopen:
	/* Make it clear to the caller that the re-reserve failed. */
	memset(r, 0, sizeof(*r));
	return false;
}

/*
 * Attempt to finalize a specified descriptor. If this fails, the descriptor
 * is either already final or it will finalize itself when the writer commits.
 */
static void desc_make_final(struct prb_desc_ring *desc_ring, unsigned long id)
{
	unsigned long prev_state_val = DESC_SV(id, desc_committed);
	struct prb_desc *d = to_desc(desc_ring, id);

	atomic_long_cmpxchg_relaxed(&d->state_var, prev_state_val,
			DESC_SV(id, desc_finalized)); /* LMM(desc_make_final:A) */

	/* Best effort to remember the last finalized @id. */
	atomic_long_set(&desc_ring->last_finalized_id, id);
}

/* Commit the data (possibly finalizing it) and restore interrupts. */
static void _prb_commit(struct prb_reserved_entry *e, unsigned long state_val)
{
	struct prb_desc_ring *desc_ring = &e->rb->desc_ring;
	struct prb_desc *d = to_desc(desc_ring, e->id);
	unsigned long prev_state_val = DESC_SV(e->id, desc_reserved);

	/* Now the writer has finished all writing: LMM(_prb_commit:A) */

	/*
	 * Set the descriptor as committed. See "ABA Issues" about why
	 * cmpxchg() instead of set() is used.
	 *
	 * 1  Guarantee all record data is stored before the descriptor state
	 *    is stored as committed. A write memory barrier is sufficient
	 *    for this. This pairs with desc_read:B and desc_reopen_last:A.
	 *
	 * 2. Guarantee the descriptor state is stored as committed before
	 *    re-checking the head ID in order to possibly finalize this
	 *    descriptor. This pairs with desc_reserve:D.
	 *
	 *    Memory barrier involvement:
	 *
	 *    If prb_commit:A reads from desc_reserve:D, then
	 *    desc_make_final:A reads from _prb_commit:B.
	 *
	 *    Relies on:
	 *
	 *    MB _prb_commit:B to prb_commit:A
	 *       matching
	 *    MB desc_reserve:D to desc_make_final:A
	 */
	if (!atomic_long_try_cmpxchg(&d->state_var, &prev_state_val,
			DESC_SV(e->id, state_val))) { /* LMM(_prb_commit:B) */
		WARN_ON_ONCE(1);
	}

	/* Restore interrupts, the reserve/commit window is finished. */
	local_irq_restore(e->irqflags);
}

/**
 * prb_commit() - Commit (previously reserved) data to the ringbuffer.
 *
 * @e: The entry containing the reserved data information.
 *
 * This is the public function available to writers to commit data.
 *
 * Note that the data is not yet available to readers until it is finalized.
 * Finalizing happens automatically when space for the next record is
 * reserved.
 *
 * See prb_final_commit() for a version of this function that finalizes
 * immediately.
 *
 * Context: Any context. Enables local interrupts.
 */
void prb_commit(struct prb_reserved_entry *e)
{
	struct prb_desc_ring *desc_ring = &e->rb->desc_ring;
	unsigned long head_id;

	_prb_commit(e, desc_committed);

	/*
	 * If this descriptor is no longer the head (i.e. a new record has
	 * been allocated), extending the data for this record is no longer
	 * allowed and therefore it must be finalized.
	 */
	head_id = atomic_long_read(&desc_ring->head_id); /* LMM(prb_commit:A) */
	if (head_id != e->id)
		desc_make_final(desc_ring, e->id);
}

/**
 * prb_final_commit() - Commit and finalize (previously reserved) data to
 *                      the ringbuffer.
 *
 * @e: The entry containing the reserved data information.
 *
 * This is the public function available to writers to commit+finalize data.
 *
 * By finalizing, the data is made immediately available to readers.
 *
 * This function should only be used if there are no intentions of extending
 * this data using prb_reserve_in_last().
 *
 * Context: Any context. Enables local interrupts.
 */
void prb_final_commit(struct prb_reserved_entry *e)
{
	struct prb_desc_ring *desc_ring = &e->rb->desc_ring;

	_prb_commit(e, desc_finalized);

	/* Best effort to remember the last finalized @id. */
	atomic_long_set(&desc_ring->last_finalized_id, e->id);
}
