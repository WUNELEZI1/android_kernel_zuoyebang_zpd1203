// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/file.h>
#include <linux/sync_file.h>
#include "dpu_fence_core.h"
#include "dpu_log.h"

#define DPU_FENCE_DIFF_MS_MAX 2000

#define KTIME_GET_DIFF_MS(timestamp) \
	(ktime_to_ms(ktime_get_real()) - ktime_to_ms(timestamp))
#define CHECK_FORWARD_STAGE(forward, type) \
	(((type) - 1) == atomic_read(&forward->cnt))
#define CHECK_FORWARD_DONE(forward) \
	(atomic_read(&forward->cnt) == DPU_FENCE_FORWARD_DONE)

struct dpu_fence_forward {
	atomic_t cnt;
	bool processed;
	struct list_head head;
};

static LIST_HEAD(fence_ctx_list);

static struct dpu_fence_ctx *dpu_fence_ctx_get(struct dpu_fence_ctx *ctx);
static void dpu_fence_ctx_put(struct dpu_fence_ctx *ctx);

static inline char *get_fence_str(enum dpu_fence_type type)
{
	if (type == DPU_FENCE_PRESENT)
		return "dpu_present";
	else if (type == DPU_FENCE_RELEASE)
		return "dpu_release";
	else if (type == DPU_FENCE_START)
		return "dpu_start";
	else
		return "dpu_unknow";
}

static const char *dpu_fence_get_driver_name(struct dma_fence *fence)
{
	struct dpu_fence *f = to_dpu_fence(fence);

	return f->name;
}

static const char *dpu_fence_get_timeline_name(struct dma_fence *fence)
{
	struct dpu_fence *f = to_dpu_fence(fence);

	return f->ctx->name;
}

static bool dpu_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static bool dpu_fence_signaled(struct dma_fence *fence)
{
	struct dpu_fence *f = to_dpu_fence(fence);

	return (f->ctx->signaled_seqno >= fence->seqno);
}

static void dpu_fence_release(struct dma_fence *fence)
{
	struct dpu_fence *f = to_dpu_fence(fence);

	dpu_fence_ctx_put(f->ctx);

	kfree(f);
}

static void dpu_fence_value_str(struct dma_fence *fence, char *str, int size)
{
	if (!fence || !str)
		return;

	snprintf(str, size, "%llu", fence->seqno);
}

static void dpu_fence_timeline_value_str(struct dma_fence *fence, char *str,
		int size)
{
	struct dpu_fence *f = to_dpu_fence(fence);

	snprintf(str, size, "%llu", f->ctx->signaled_seqno);
}

static struct dma_fence_ops dpu_fence_ops = {
	.use_64bit_seqno = true,
	.get_driver_name = dpu_fence_get_driver_name,
	.get_timeline_name = dpu_fence_get_timeline_name,
	.enable_signaling = dpu_fence_enable_signaling,
	.signaled = dpu_fence_signaled,
	.release = dpu_fence_release,
	.fence_value_str = dpu_fence_value_str,
	.timeline_value_str = dpu_fence_timeline_value_str,
};

static inline void dpu_fence_state_dump(struct dma_fence *fence)
{
	int fence_status;

	fence_status = dma_fence_get_status(fence);

	DPU_ERROR("fence status: %d, fence flag: 0x%lx\n", fence_status, fence->flags);
	DPU_ERROR("fence seqno: %llu, driver name: %s, timeline name: %s\n",
			fence->seqno,
			fence->ops->get_driver_name(fence),
			fence->ops->get_timeline_name(fence));
}

int dpu_fence_wait(struct dma_fence *fence, bool intr, uint32_t timeout_ms)
{
	int ret;

	ret = dma_fence_wait_timeout(fence, intr, msecs_to_jiffies(timeout_ms));
	if (!ret)
		ret = -ETIMEDOUT;

	if (ret < 0) {
		dpu_fence_state_dump(fence);
		DPU_ERROR("wait fence error: %d, current timeout value: %u ms\n",
				ret, timeout_ms);
	}

	return ret > 0 ? 0 : ret;
}

void dpu_fence_forward(struct dpu_fence_ctx *ctx, enum dpu_fence_forward_type type)
{
	struct dpu_fence_forward *forward = NULL;
	unsigned long flags;

	spin_lock_irqsave(&ctx->list_lock, flags);
	if (list_empty(&ctx->fence_list))
		goto exit;

	list_for_each_entry(forward, &ctx->forward_list, head) {
		switch (type) {
		case DPU_FENCE_FORWARD_PREPARE:
		case DPU_FENCE_FORWARD_DONE:
			if (CHECK_FORWARD_STAGE(forward, type))
				atomic_inc(&forward->cnt);
			break;
		default:
			DPU_ERROR("unknow forward type %d\n", type);
			break;
		};
	}
exit:
	spin_unlock_irqrestore(&ctx->list_lock, flags);
}

static inline void __dpu_fence_signal(struct dpu_fence_ctx *ctx,
		struct dma_fence *fence, bool force)
{
	struct dpu_fence *f = to_dpu_fence(fence);
	struct dpu_fence_forward *forward = NULL;

	if (force && (fence->seqno > ctx->signaled_seqno)) {
		DPU_SYNC_DEBUG("force to signal %s, signaled: %llu, seqno: %llu\n",
				ctx->name, ctx->signaled_seqno, fence->seqno);

		ctx->signaled_seqno = max(fence->seqno, ctx->commit_seqno);
		ctx->commit_seqno = max(fence->seqno, ctx->commit_seqno);
		ctx->unexpected_count++;
	}

	if (f->timestamp && (KTIME_GET_DIFF_MS(f->timestamp) > DPU_FENCE_DIFF_MS_MAX))
		ctx->expired_count++;

	dma_fence_signal(fence);
	list_del_init(&f->head);
	dma_fence_put(fence);

	forward = list_first_entry_or_null(&ctx->forward_list,
			struct dpu_fence_forward, head);
	if (forward) {
		list_del_init(&forward->head);
		kfree(forward);
	}
}

void dpu_fence_signal(struct dpu_fence_ctx *ctx, bool force)
{
	struct dpu_fence_forward *forward = NULL;
	struct dpu_fence *fence, *temp;
	unsigned long flags;

	if (!ctx) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	spin_lock_irqsave(&ctx->list_lock, flags);
	if (list_empty(&ctx->fence_list))
		goto exit;

	forward = list_first_entry_or_null(&ctx->forward_list,
			struct dpu_fence_forward, head);
	if (!forward) {
		DPU_SYNC_DEBUG("invalid forward pointer\n");
		force = true;
	}

	if (!force) {
		list_for_each_entry(forward, &ctx->forward_list, head) {
			if (!CHECK_FORWARD_DONE(forward))
				goto process;

			if (!forward->processed) {
				ctx->signaled_seqno++;
				forward->processed = true;
			}
		}
	}

process:
	list_for_each_entry_safe(fence, temp, &ctx->fence_list, head)
		if (fence->base.ops->signaled(&fence->base) || force)
			__dpu_fence_signal(ctx, &fence->base, force);
exit:
	spin_unlock_irqrestore(&ctx->list_lock, flags);
}

int dpu_fence_create(struct dpu_fence_ctx *ctx, u32 offset)
{
	struct dpu_fence_forward *forward;
	struct sync_file *sync_file;
	struct dpu_fence *fence;
	unsigned long flags;
	u64 seqno;
	int fd;
	int ret;

	if (!ctx) {
		DPU_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	fence = kzalloc(sizeof(*fence), GFP_KERNEL);
	if (!fence)
		return -ENOMEM;

	forward = kzalloc(sizeof(*forward), GFP_KERNEL);
	if (!forward) {
		kfree(fence);
		return -ENOMEM;
	}

	fd = get_unused_fd_flags(0);
	if (fd < 0) {
		DPU_ERROR("failed to get fd\n");
		kfree(fence);
		kfree(forward);
		return fd;
	}

	fence->ctx = ctx;
	seqno = ctx->commit_seqno + offset;
	snprintf(fence->name, FENCE_NAME_LEN, "%s-%llu", ctx->name, seqno);
	dma_fence_init(&fence->base, &dpu_fence_ops, &ctx->lock,
			ctx->context, seqno);

	sync_file = sync_file_create(&fence->base);
	if (sync_file == NULL) {
		DPU_ERROR("failed to create sync file\n");
		ret = -EINVAL;
		goto err_file;
	}

	atomic_set(&forward->cnt, 0);
	fd_install(fd, sync_file->file);
	dpu_fence_ctx_get(ctx);

	spin_lock_irqsave(&ctx->list_lock, flags);
	list_add_tail(&forward->head, &ctx->forward_list);
	list_add_tail(&fence->head, &ctx->fence_list);
	ctx->commit_seqno++;
	spin_unlock_irqrestore(&ctx->list_lock, flags);

	if (offset <= 1)
		fence->timestamp = ktime_get_real();

	DPU_SYNC_DEBUG("FENCE: name:%s commit_seqno:%llu signaled_seqno:%llu seqno:%llu\n",
			ctx->name, ctx->commit_seqno, ctx->signaled_seqno, seqno);
	return fd;

err_file:
	kfree(forward);
	put_unused_fd(fd);
	dma_fence_put(&fence->base);
	return ret;
}

static void dpu_fence_ctx_release(struct kref *refcount)
{
	struct dpu_fence_ctx *ctx = container_of(refcount,
			struct dpu_fence_ctx, refcount);

	list_del_init(&ctx->head);
	kfree(ctx);
}

static struct dpu_fence_ctx *dpu_fence_ctx_get(struct dpu_fence_ctx *ctx)
{
	if (ctx)
		kref_get(&ctx->refcount);

	return ctx;
}

static void dpu_fence_ctx_put(struct dpu_fence_ctx *ctx)
{
	if (ctx)
		kref_put(&ctx->refcount, &dpu_fence_ctx_release);
}

int dpu_fence_ctx_init(struct dpu_fence_ctx **ctx,
		enum dpu_fence_type type,
		int index)
{
	struct dpu_fence_ctx *fence_ctx;

	if (!ctx || (type >= DPU_FENCE_MAX)) {
		DPU_ERROR("invalid parameters, fence type: %d\n", type);
		return -EINVAL;
	}

	fence_ctx = kzalloc(sizeof(*fence_ctx), GFP_KERNEL);
	if (!fence_ctx)
		return -ENOMEM;

	snprintf(fence_ctx->name, FENCE_NAME_LEN, "%s_ctx_%d",
			get_fence_str(type), index);

	fence_ctx->type = type;
	fence_ctx->context = dma_fence_context_alloc(1);
	kref_init(&fence_ctx->refcount);
	INIT_LIST_HEAD(&fence_ctx->fence_list);
	INIT_LIST_HEAD(&fence_ctx->forward_list);
	spin_lock_init(&fence_ctx->list_lock);
	spin_lock_init(&fence_ctx->lock);

	list_add_tail(&fence_ctx->head, &fence_ctx_list);
	*ctx = fence_ctx;

	return 0;
}

void dpu_fence_ctx_deinit(struct dpu_fence_ctx *ctx)
{
	dpu_fence_ctx_put(ctx);
}

void dpu_fence_ctx_state_dump(struct seq_file *s)
{
	struct dpu_fence_ctx *fence_ctx;
	unsigned long flags;

	seq_puts(s, "name | commit | signaled | unexpected | expired\n");
	seq_puts(s, "-----------------------------------------------\n");

	list_for_each_entry(fence_ctx, &fence_ctx_list, head) {
		spin_lock_irqsave(&fence_ctx->list_lock, flags);
		seq_printf(s, "%s | %8llu | %8llu | %8llu | %8llu\n",
				fence_ctx->name,
				fence_ctx->commit_seqno,
				fence_ctx->signaled_seqno,
				fence_ctx->unexpected_count,
				fence_ctx->expired_count);
		spin_unlock_irqrestore(&fence_ctx->list_lock, flags);
	}
}
