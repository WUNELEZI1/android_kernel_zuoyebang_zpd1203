/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __HOOK_RINGBUFFER_API_H__
#define __HOOK_RINGBUFFER_API_H__

#include <linux/types.h>

#define AP_KEYS_MAX 63

struct hook_ringbuffer {
	u32 max_num;
	u32 field_count;
	u32 rear;
	u32 read_idx;
	u32 count;
	u32 is_full;
	char keys[AP_KEYS_MAX + 1];
	u64 addr;
};

void ringbuffer_write(struct hook_ringbuffer *buffer, u8 *element);
#endif /* __HOOK_RINGBUFFER_API_H__ */
