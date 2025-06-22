/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __HOOK_RINGBUFFER_H__
#define __HOOK_RINGBUFFER_H__

#include <soc/xring/hook_ringbuffer_api.h>

int ringbuffer_init(struct hook_ringbuffer *buffer, u32 bytes, u32 fieldcnt, const char *keys);
int  ringbuffer_read(struct hook_ringbuffer *buffer, u8 *element, u32 len);
int  is_ringbuffer_full(const void *buffer_addr);
void get_ringbuffer_start_end(struct hook_ringbuffer *buffer, u32 *start, u32 *end);
bool is_ringbuffer_empty(struct hook_ringbuffer *buffer);
bool is_ringbuffer_invalid(u32 field_count, u32 len, struct hook_ringbuffer *buffer);

#endif /* __HOOK_RINGBUFFER_H__ */
