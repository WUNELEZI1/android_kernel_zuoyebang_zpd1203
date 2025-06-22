// SPDX-License-Identifier: GPL-2.0-or-later

#define pr_fmt(fmt)	"[xr_dfx][trace_hook]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/stddef.h>

#include "hook_ringbuffer.h"

int ringbuffer_init(struct hook_ringbuffer *buffer, u32 bytes, u32 fieldcnt, const char *keys)
{
	if (!buffer) {
		pr_err("hook ringbuffer head is null!\n");
		return -EINVAL;
	}

	if (bytes < (sizeof(*buffer) + sizeof(u8) * fieldcnt)) {
		pr_err("hook ringbuffer size [0x%x] is too short!\n", bytes);
		return -EINVAL;
	}

	/* max_num: records count */
	buffer->max_num = (bytes - sizeof(*buffer)) / (sizeof(u8) * fieldcnt);
	buffer->rear = 0;
	buffer->read_idx = 0;
	buffer->count = 0;
	buffer->is_full = 0;
	buffer->field_count = fieldcnt; /* How many u8 in ONE record */

	memset((void *)(buffer->keys), 0, AP_KEYS_MAX + 1);

	if (keys)
		strncpy(buffer->keys, keys, AP_KEYS_MAX);

	buffer->addr = (u64) buffer + offsetof(struct hook_ringbuffer, addr);
	return 0;
}
EXPORT_SYMBOL(ringbuffer_init);

void ringbuffer_write(struct hook_ringbuffer *buffer, u8 *element)
{
	if (IS_ERR_OR_NULL(buffer) || IS_ERR_OR_NULL(element)) {
		pr_err("parameter buffer or element is null when write in ringbuffer\n");
		return;
	}

	buffer->rear++;
	if (buffer->rear >= buffer->max_num) {
		buffer->rear = 1;
		buffer->is_full = 1;
	}

	memcpy((void *)(buffer->addr + sizeof(u64) + (buffer->rear - 1) * buffer->field_count),
			(void *)element, buffer->field_count * sizeof(*element));

	buffer->count++;
	if (buffer->count >= buffer->max_num)
		buffer->count = buffer->max_num;
}
EXPORT_SYMBOL(ringbuffer_write);

/* Return:  success: = 0 ;  fail: other */
int ringbuffer_read(struct hook_ringbuffer *buffer, u8 *element, u32 len)
{
	u32 ridx;

	if (IS_ERR_OR_NULL(buffer)) {
		pr_err("parameter buffer ringbuffer is null!\n");
		return -1;
	}

	if (IS_ERR_OR_NULL(element)) {
		pr_err("parameter element is null!\n");
		return -1;
	}

	if (buffer->count == 0)
		return -1;
	buffer->count--;

	if (buffer->count >= buffer->max_num)
		buffer->read_idx = buffer->rear;

	if (buffer->read_idx >= buffer->max_num)
		buffer->read_idx = 0;

	ridx = buffer->read_idx++;

	memcpy((void *)element, (void *)&buffer->addr + sizeof(u64) + (long)ridx * buffer->field_count,
			buffer->field_count * sizeof(*element));

	return 0;
}

int is_ringbuffer_full(const void *buffer_addr)
{
	if (!buffer_addr)
		return -1;

	return (int)(((struct hook_ringbuffer *)buffer_addr)->is_full);
}

void get_ringbuffer_start_end(struct hook_ringbuffer *buffer, u32 *start, u32 *end)
{
	if (IS_ERR_OR_NULL(buffer) || IS_ERR_OR_NULL(start) || IS_ERR_OR_NULL(end)) {
		pr_err("parameter buffer 0x%pK start 0x%pK end 0x%pK is null!\n",
			buffer, start, end);
		return;
	}

	if (buffer->is_full) {
		if ((buffer->rear >= buffer->max_num) || (buffer->rear <= 0)) {
			*start = 0;
			*end = buffer->max_num - 1;
		} else if (buffer->rear) {
			*start = buffer->rear;
			*end = buffer->rear - 1 + buffer->max_num;
		}
	} else {
		*start = 0;
		*end = buffer->rear - 1;
	}
}

bool is_ringbuffer_empty(struct hook_ringbuffer *buffer)
{
	if (IS_ERR_OR_NULL(buffer)) {
		pr_err("parameter buffer is null!\n");
		return true;
	}

	if ((buffer->is_full == 0) && (buffer->rear == 0))
		return true;

	return false;
}

bool is_ringbuffer_invalid(u32 field_count, u32 len, struct hook_ringbuffer *buffer)
{
	if (IS_ERR_OR_NULL(buffer)) {
		pr_err("parameter buffer ringbuffer is null!\n");
		return true;
	}

	if (unlikely(buffer->field_count != field_count)) {
		pr_err("ringbuffer field_count %u != %u\n", buffer->field_count, field_count);
		return true;
	}

	if (unlikely(buffer->rear > buffer->max_num)) {
		pr_err("buffer->rear %u > buffer->max_num %u\n", buffer->rear, buffer->max_num);
		return true;
	}

	if (unlikely(
			(buffer->max_num <= 0) || (field_count <= 0) || (len <= sizeof(*buffer) ||
			(buffer->max_num > ((len - sizeof(*buffer)) / field_count))))) {
		pr_err("ringbuffer max_num %u field_count %u len %u\n", buffer->max_num,
			field_count, len);
		return true;
	}

	return false;
}

