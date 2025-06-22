// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __APP_ROUTER_BUFFER__
#define __APP_ROUTER_BUFFER__

#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#define BUFFER_MAX 65535

struct buffer_s {
	uint32_t size;
	uint8_t *buffer;
	uint8_t *begin;
	uint8_t *end;
	spinlock_t read_lock;
	spinlock_t write_lock;
	struct completion ready;
};

typedef uint32_t (*rw_data)(uint8_t *first_clip, uint32_t first_len,
			    uint8_t *second_clip, uint32_t second_len,
			    void *arg);

bool buffer_empty(struct buffer_s const *const buffer);

bool buffer_full(struct buffer_s const *const buffer);

uint32_t buffer_usage(struct buffer_s const *const buffer);

uint32_t buffer_left(struct buffer_s const *const buffer);

int buffer_init(struct buffer_s **buffer, const uint32_t size);

int buffer_deinit(struct buffer_s *buffer);

int buffer_reset(struct buffer_s *buffer);

int buffer_read(struct buffer_s *buffer, rw_data read_cb, void *arg);

int buffer_read_with_lock(struct buffer_s *buffer, rw_data read_cb, void *arg);

int buffer_write(struct buffer_s *buffer, rw_data write_cb, void *arg);

int buffer_write_with_lock(struct buffer_s *buffer, rw_data write_cb,
			   void *arg);

#endif
