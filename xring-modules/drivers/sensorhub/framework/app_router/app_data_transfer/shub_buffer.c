// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include "private_inc/shub_buffer.h"
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/string.h>

bool buffer_empty(struct buffer_s const *const buffer)
{
	return buffer->begin == buffer->end;
}

uint32_t buffer_usage(struct buffer_s const *const buffer)
{
	if (buffer->begin <= buffer->end)
		return buffer->end - buffer->begin;
	return buffer->end - buffer->buffer +
	       (buffer->size - (buffer->begin - buffer->buffer) + 1);
}

bool buffer_full(struct buffer_s const *const buffer)
{
	return buffer_usage(buffer) == buffer->size;
}

uint32_t buffer_left(struct buffer_s const *const buffer)
{
	return buffer->size - buffer_usage(buffer);
}

int buffer_init(struct buffer_s **buffer, const uint32_t size)
{
	struct buffer_s *tmp;

	if (!buffer || !size)
		return -EINVAL;
	if (size >= BUFFER_MAX)
		return -EINVAL;

	tmp = kmalloc(sizeof(struct buffer_s), GFP_KERNEL);
	if (!tmp)
		goto fail;

	tmp->buffer = kmalloc(sizeof(uint8_t) * size + 1, GFP_KERNEL);
	if (!tmp->buffer)
		goto fail;

	tmp->size = size;
	tmp->begin = tmp->buffer;
	tmp->end = tmp->buffer;
	init_completion(&tmp->ready);
	spin_lock_init(&tmp->read_lock);
	spin_lock_init(&tmp->write_lock);
	*buffer = tmp;
	return 0;
fail:
	if (!tmp)
		return -ENOMEM;
	kfree(tmp->buffer);
	kfree(tmp);
	return -ENOMEM;
}

int buffer_deinit(struct buffer_s *buffer)
{
	if (!buffer)
		return -EINVAL;
	kfree(buffer->buffer);
	kfree(buffer);
	return 0;
}

int buffer_reset(struct buffer_s *buffer)
{
	if (!buffer)
		return -EINVAL;
	buffer->begin = buffer->buffer;
	buffer->end = buffer->buffer;
	return 0;
}

static inline int handle_read(struct buffer_s const *const buffer,
			      rw_data read_cb, void *arg)
{
	int num = 0;

	if (buffer->begin <= buffer->end)
		num = read_cb(buffer->begin, buffer->end - buffer->begin,
			      buffer->begin, 0, arg);
	else {
		num = read_cb(
			buffer->begin,
			(buffer->size - (buffer->begin - buffer->buffer) + 1),
			buffer->buffer, buffer->end - buffer->buffer, arg);
	}
	return num;
}

int buffer_read(struct buffer_s *buffer, rw_data read_cb, void *arg)
{
	int num;
	int tmp;

	if (buffer_empty(buffer))
		return -ENODATA;
	num = handle_read(buffer, read_cb, arg);
	if (num > buffer_usage(buffer))
		return -ENOSPC;

	tmp = buffer->begin - buffer->buffer;
	tmp = (tmp + num) % (buffer->size + 1);
	wmb();
	buffer->begin = buffer->buffer + tmp;
	return num;
}

int buffer_read_with_lock(struct buffer_s *buffer, rw_data read_cb, void *arg)
{
	int num;
	int tmp;

	spin_lock(&buffer->read_lock);
	if (buffer_empty(buffer)) {
		num = -EAGAIN;
		goto fail;
	}
	num = handle_read(buffer, read_cb, arg);
	if (num > buffer_usage(buffer)) {
		num = -ENOSPC;
		goto fail;
	}
	tmp = buffer->begin - buffer->buffer;
	tmp = (tmp + num) % (buffer->size + 1);
	wmb();
	buffer->begin = buffer->buffer + tmp;
fail:
	spin_unlock(&buffer->read_lock);
	return num;
}

static inline int handle_write(struct buffer_s const *const buffer,
			       rw_data write_cb, void *arg)
{
	int num = 0;

	if (buffer->begin > buffer->end)
		num = write_cb(buffer->end, buffer->begin - buffer->end - 1,
			       buffer->end, 0, arg);
	else {
		if (buffer->begin == buffer->buffer)
			num = write_cb(buffer->end,
				       buffer->size -
					       (buffer->end - buffer->buffer),
				       buffer->buffer, 0, arg);
		else
			num = write_cb(buffer->end,
				       (buffer->size -
					(buffer->end - buffer->buffer) + 1),
				       buffer->buffer,
				       buffer->begin - buffer->buffer - 1, arg);
	}
	return num;
}
int buffer_write(struct buffer_s *buffer, rw_data write_cb, void *arg)
{
	int num;
	int tmp;

	if (buffer_full(buffer))
		return -ENOSPC;

	num = handle_write(buffer, write_cb, arg);

	if (num > buffer_left(buffer))
		return -ENOSPC;
	tmp = buffer->end - buffer->buffer;
	tmp = (tmp + num) % (buffer->size + 1);
	wmb();
	buffer->end = buffer->buffer + tmp;
	return num;
}

int buffer_write_with_lock(struct buffer_s *buffer, rw_data write_cb, void *arg)
{
	int num;
	int tmp;

	spin_lock(&buffer->write_lock);
	if (buffer_full(buffer)) {
		num = -ENOSPC;
		goto fail;
	}

	num = handle_write(buffer, write_cb, arg);

	if (num > buffer_left(buffer)) {
		num = -ENOSPC;
		goto fail;
	}

	tmp = buffer->end - buffer->buffer;
	tmp = (tmp + num) % (buffer->size + 1);
	wmb();
	buffer->end = buffer->buffer + tmp;

fail:
	spin_unlock(&buffer->write_lock);
	return num;
}
