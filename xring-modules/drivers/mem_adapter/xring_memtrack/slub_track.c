// SPDX-License-Identifier: GPL-2.0
#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <trace/hooks/mm.h>
#include <linux/sort.h>
#include <linux/android_debug_symbols.h>
#include <linux/vmstat.h>
#include <linux/compiler.h>

#include "../mm/slab.h"
#include "memory_track.h"
#include "../xring_meminfo_notify_lmkd.h"
#include "../xring_meminfo_process_val.h"
#include "soc/xring/xring_mem_adapter.h"

#define SLAB_XR_TRACE       0x1UL

struct raw_stack_info {
	unsigned long caller;
	long pid;
	long comm1;
	long comm2;
};

struct slub_ring_buf {
	int type;
	struct raw_stack_info *buf; /* save caller */
	spinlock_t lock; /* protect ringbuf */
	size_t size;
	unsigned long in;
	unsigned long out;
};

struct slub_rb_node {
	struct rb_node node;
	unsigned long caller;
	atomic_t ref;
	long comm1;
	long comm2;
	long min_pid;
	long max_pid;
};

struct slub_track {
	int type; /* kslubtrackd need type to distinct rbtree */
	wait_queue_head_t slub_wait;
	struct mutex slub_mutex; /* protect rbtree */
	struct task_struct *slub_ktrackd;
	struct rb_root slubrb;
};

static char *specified_target;
static int slub_track_flag;
static unsigned long target_start_k;
static unsigned long target_end_k;
static struct slub_track slub_track[NR_SLUB_ID];
static struct slub_ring_buf srb_array[NR_SLUB_ID];
static struct kmem_cache *slub_track_cache;
static struct kmem_cache *max_slub_cache;
static struct mutex *xr_slab_mutex;
static struct list_head *xr_slab_caches;

static inline unsigned long get_ring_buf_len(struct slub_ring_buf *srb)
{
	return (srb->in - srb->out);
}

static inline int ring_buf_is_full(struct slub_ring_buf *srb)
{
	return (get_ring_buf_len(srb) == srb->size);
}

static inline int ring_buf_is_half(struct slub_ring_buf *srb)
{
	return (get_ring_buf_len(srb) >= (srb->size / SLUB_RINGBUF_HALF));
}

static inline int ring_buf_is_empty(struct slub_ring_buf *srb)
{
	return (srb->in == srb->out);
}

static void __set_caller(struct slub_ring_buf *srb, unsigned long caller,
						long pid, long comm1, long comm2)
{
	size_t idx = srb->in & (srb->size - 1);

	/* if srb is not full,
	 * first put the data starting from srb->in to buffer end,
	 * then put the rest (if any) from the beginning of the buffer
	 * update the index, when overflow, turn to 0
	 */

	if (ring_buf_is_full(srb))
		return;
	if (idx < srb->size) {
		(srb->buf + idx)->caller = caller;
		(srb->buf + idx)->pid = pid;
		(srb->buf + idx)->comm1 = comm1;
		(srb->buf + idx)->comm2 = comm2;
	} else {
		srb->buf->caller = caller;
		srb->buf->pid = pid;
		srb->buf->comm1 = comm1;
		srb->buf->comm2 = comm2;
	}

	srb->in++;
}

static int __get_caller(struct slub_ring_buf *srb, unsigned long *caller,
						long *pid, long *comm1, long *comm2)
{
	size_t idx = srb->out & (srb->size - 1);

	if (ring_buf_is_empty(srb))
		return -ENOSPC;
	/*
	 * len: buf with data in it
	 * first get the data from srb->out until the end of the buffer
	 * then get the rest (if any) from the beginning of the buffer
	 * update the index, when overflow, turn to 0
	 */
	if (idx < srb->size) {
		*caller = (srb->buf + idx)->caller;
		*pid = (srb->buf + idx)->pid;
		*comm1 = (srb->buf + idx)->comm1;
		*comm2 = (srb->buf + idx)->comm2;
	} else {
		*caller = srb->buf->caller;
		*pid = srb->buf->pid;
		*comm1 = srb->buf->comm1;
		*comm2 = srb->buf->comm2;
	}

	srb->out++;
	return 0;
}

static void set_caller(struct slub_ring_buf *srb, unsigned long caller,
						long pid, long comm1, long comm2, int type)
{
	if (slub_track_flag) {
		__set_caller(srb, caller, pid, comm1, comm2);
		if (ring_buf_is_half(srb))
			wake_up(&slub_track[type].slub_wait);
	}
}

static depot_stack_handle_t get_track_stack_handle(void)
{
	depot_stack_handle_t handle;
	unsigned long entries[SLUB_STACK_ENTRY_NUM];
	unsigned int nr_entries;

	nr_entries = stack_trace_save(entries, ARRAY_SIZE(entries), SLUB_IGNORED_ENTRY_NUM);
	handle = stack_depot_save(entries, nr_entries, GFP_NOWAIT);

	return handle;
}

void set_free_track(unsigned long caller)
{
	unsigned long flags;
	depot_stack_handle_t handle;
	struct slub_ring_buf *srb = &srb_array[SLUB_FREE];
	long pid = 0;
	long comm1 = 0;
	long comm2 = 0;

	if (!slub_track_flag)
		return;

	handle = get_track_stack_handle();
	if (handle)
		caller = (unsigned long)handle;

	if (!in_interrupt()) {
		pid = current->pid;
		comm1 = *(long *)current->comm;
		comm2 = *(long *)(current->comm + sizeof(long));
	}

	spin_lock_irqsave(&srb->lock, flags);
	set_caller(srb, caller, pid, comm1, comm2, SLUB_FREE);
	spin_unlock_irqrestore(&srb->lock, flags);
}

void set_alloc_track(unsigned long caller)
{
	unsigned long flags;
	depot_stack_handle_t handle;
	struct slub_ring_buf *srb = &srb_array[SLUB_ALLOC];
	long pid = 0;
	long comm1 = 0;
	long comm2 = 0;

	if (!slub_track_flag)
		return;

	handle = get_track_stack_handle();
	if (handle)
		caller = (unsigned long)handle;

	if (!in_interrupt()) {
		pid = current->pid;
		comm1 = *(long *)current->comm;
		comm2 = *(long *)(current->comm + sizeof(long));
	}

	spin_lock_irqsave(&srb->lock, flags);
	set_caller(srb, caller, pid, comm1, comm2, SLUB_ALLOC);
	spin_unlock_irqrestore(&srb->lock, flags);
}

static int getleftright(unsigned long caller, long comm1, long comm2, struct slub_rb_node *entry)
{
	if (caller == entry->caller && comm1 == entry->comm1 && comm2 == entry->comm2)
		return 0;
	else if (caller < entry->caller)
		return -1;
	else if (caller > entry->caller)
		return 1;

	if (comm1 < entry->comm1)
		return -1;
	else if (comm1 > entry->comm1)
		return 1;
	else
		return comm2 < entry->comm2 ? -1 : 1;
}

static void slub_add_node(unsigned long caller, long pid, long comm1, long comm2, int type)
{
	struct rb_node **p = &slub_track[type].slubrb.rb_node;
	struct rb_node *parent = NULL;
	struct slub_rb_node *entry = NULL;
	struct slub_rb_node *rbnode = NULL;
	int compare = 0;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct slub_rb_node, node);
		compare = getleftright(caller, comm1, comm2, entry);
		if (compare == -1) {
			p = &(*p)->rb_left;
		} else if (compare == 1) {
			p = &(*p)->rb_right;
		} else {
			atomic_inc(&entry->ref);
			if (pid < entry->min_pid)
				entry->min_pid = pid;
			else if (pid > entry->max_pid)
				entry->max_pid = pid;
			return;
		}
	}
	rbnode = kmem_cache_alloc(slub_track_cache, GFP_ATOMIC);
	if (rbnode) {
		rbnode->caller = caller;
		rbnode->ref.counter = 0;
		atomic_inc(&rbnode->ref);
		rbnode->comm1 = comm1;
		rbnode->comm2 = comm2;
		rbnode->min_pid = pid;
		rbnode->max_pid = pid;
		rb_link_node(&rbnode->node, parent, p);
		rb_insert_color(&rbnode->node, &slub_track[type].slubrb);
	}
}

static void slub_del_node(struct slub_rb_node *rbnode, int type)
{
	rb_erase(&rbnode->node, &slub_track[type].slubrb);
	kmem_cache_free(slub_track_cache, rbnode);
}

static size_t slub_read_node(struct xr_stack_info *buf,
				const size_t len, int type)
{
	struct rb_node *n = NULL;
	struct slub_rb_node *vnode = NULL;
	size_t length = len;
	size_t cnt = 0;

	if (!buf)
		return 0;

	n = rb_first(&slub_track[type].slubrb);
	while (n) {
		vnode = rb_entry(n, struct slub_rb_node, node);
		if (length && atomic_read(&vnode->ref) >= SLUB_VALID_CALLER_CNT) {
			length--;
			(buf + cnt)->caller = vnode->caller;
			(buf + cnt)->ref.counter = vnode->ref.counter;
			memcpy((buf + cnt)->comm, (char *)&vnode->comm1, sizeof(long));
			memcpy((buf + cnt)->comm + sizeof(long), (char *)&vnode->comm2, sizeof(long));
			(buf + cnt)->min_pid = vnode->min_pid;
			(buf + cnt)->max_pid = vnode->max_pid;
			cnt++;
		}
		n = rb_next(n);
		slub_del_node(vnode, type);
	}
	return cnt;
}

static void __slub_fetch_node(struct slub_ring_buf *srb, int type)
{
	unsigned long caller;
	long pid;
	long comm1;
	long comm2;
	int ret;

	ret = __get_caller(srb, &caller, &pid, &comm1, &comm2);
	if (ret)
		return;
	slub_add_node(caller, pid, comm1, comm2, type);
}

/* move entry from ringbuf to rb tree */
static void slub_fetch_node(int slubtype)
{
	unsigned long i, len, flags;
	struct slub_ring_buf *srb = &srb_array[slubtype];

	spin_lock_irqsave(&srb->lock, flags);
	len = get_ring_buf_len(srb);
	for (i = 0; i < len; i++)
		__slub_fetch_node(srb, slubtype);
	spin_unlock_irqrestore(&srb->lock, flags);
}

static int kslubtrackd(void *p)
{
	DEFINE_WAIT(wait);
	int type;
	struct slub_track *track = (struct slub_track *)p;

	type = track->type;
	while (!kthread_should_stop()) {
		prepare_to_wait(&slub_track[type].slub_wait,
			&wait, TASK_INTERRUPTIBLE);
		schedule();
		finish_wait(&slub_track[type].slub_wait, &wait);
		mutex_lock(&slub_track[type].slub_mutex);
		if (slub_track_flag)
			slub_fetch_node(type);
		mutex_unlock(&slub_track[type].slub_mutex);
	};
	return 0;
}

static void slub_build_rb_tree(int type)
{
	slub_track[type].slubrb = RB_ROOT;
}

static int __slub_create_ringbuf(int type)
{
	void *buf = NULL;
	size_t size = SLUB_RINGBUF_LEN;
	struct slub_ring_buf *srb = &srb_array[type];

	buf = vmalloc(size * sizeof(struct raw_stack_info));
	if (!buf)
		return -ENOMEM;

	srb->in = 0;
	srb->out = 0;
	srb->buf = buf;
	srb->size = size;
	srb->type = type;
	spin_lock_init(&srb->lock);
	return 0;
}

static int slub_create_ringbuf(void)
{
	int ret;

	ret =  __slub_create_ringbuf(SLUB_ALLOC);
	if (ret) {
		xrmem_err("create alloc ringbuf failed\n");
		return -ENOMEM;
	}
	ret = __slub_create_ringbuf(SLUB_FREE);
	if (ret) {
		xrmem_err("create free ringbuf failed\n");
		return -ENOMEM;
	}
	return 0;
}
static void __slub_del_ringbuf(int type)
{
	struct slub_ring_buf *srb = &srb_array[type];

	vfree(srb->buf);
	srb->buf = NULL;
}

static int slub_del_ringbuf(void)
{
	__slub_del_ringbuf(SLUB_ALLOC);
	__slub_del_ringbuf(SLUB_FREE);
	return 0;
}

int get_max_slub(void)
{
	unsigned long max_size = 0;
	struct slabinfo sinfo;
	struct kmem_cache *s = NULL;

	mutex_lock(xr_slab_mutex);
	list_for_each_entry(s, xr_slab_caches, list) {
		if (specified_target &&
			strncmp(s->name, specified_target, strlen(specified_target) + 1) == 0) {
			get_slabinfo(s, &sinfo);
			max_size = sinfo.num_objs * s->size;
			max_slub_cache = s;
			xrmem_info("slub track specify target %s", s->name);
			break;
		}

		if (s->flags & SLAB_RECLAIM_ACCOUNT)
			continue;

		if (strncmp(s->name, "kmemleak_", strlen("kmemleak_")) == 0)
			continue;

		get_slabinfo(s, &sinfo);

		if (sinfo.num_objs * s->size > max_size) {
			max_size = sinfo.num_objs * s->size;
			max_slub_cache = s;
		}
	}
	mutex_unlock(xr_slab_mutex);
	target_start_k = max_size >> 10;
	memcheck_save_top_slub(max_slub_cache->name);
	return 0;
}

/* on->off->open->read->close */
int xr_slub_track_on(char *name)
{
	int ret;
	bool found = false;
	struct kmem_cache *cachep = NULL;

	if (slub_track_flag)
		return 0;

	mutex_lock(xr_slab_mutex);
	list_for_each_entry(cachep, xr_slab_caches, list) {
		/* slub track cache cannot track */
		if (slub_track_cache &&
			strncmp(cachep->name, slub_track_cache->name,
			strlen(cachep->name) + 1) == 0)
			continue;

		if (strncmp(cachep->name, name,
			strlen(cachep->name) + 1) == 0) {
			cachep->flags |= SLAB_XR_TRACE;
			found = true;
			break;
		}
	}
	mutex_unlock(xr_slab_mutex);
	if (!found) {
		xrmem_err("slub name[%s] not found\n", name);
		return -EINVAL;
	}

	ret = slub_create_ringbuf();
	if (ret) {
		xrmem_err("create ringbuf failed\n");
		return -ENOMEM;
	}
	xrmem_info("slub name[%s] ok\n", name);
	slub_track_flag = 1;
	return 0;
}

int xr_slub_track_off(char *name)
{
	bool found = false;
	struct kmem_cache *cachep = NULL;
	struct slabinfo sinfo;

	if (!slub_track_flag)
		return 0;

	mutex_lock(xr_slab_mutex);
	list_for_each_entry(cachep, xr_slab_caches, list) {
		if (strncmp(cachep->name, name,
			strlen(cachep->name) + 1) == 0) {
			cachep->flags &= ~SLAB_XR_TRACE;
			found = true;
			break;
		}
	}
	mutex_unlock(xr_slab_mutex);
	if (!found) {
		xrmem_err("slub name[%s] not found\n", name);
		return -EINVAL;
	}
	slub_track_flag = 0;
	slub_fetch_node(SLUB_ALLOC);
	slub_fetch_node(SLUB_FREE);
	slub_del_ringbuf();
	xrmem_info("slub name[%s] ok\n", name);
	get_slabinfo(max_slub_cache, &sinfo);
	target_end_k = (sinfo.num_objs * max_slub_cache->size) >> 10;
	xrmem_info("%lu+%ld=%lukB", target_start_k, target_end_k - target_start_k, target_end_k);
	return 0;
}

static int compare_same_caller(const void *a, const void *b)
{
	struct xr_stack_info *l = (struct xr_stack_info *)a;
	struct xr_stack_info *r = (struct xr_stack_info *)b;

	if (l->caller != r->caller)
		return l->caller - r->caller;
	else
		return atomic_read(&r->ref) - atomic_read(&l->ref);
}

static int compare_count(const void *a, const void *b)
{
	struct xr_stack_info *l = (struct xr_stack_info *)a;
	struct xr_stack_info *r = (struct xr_stack_info *)b;

	if (r->same_caller_cnt != l->same_caller_cnt)
		return r->same_caller_cnt - l->same_caller_cnt;
	else
		return atomic_read(&r->ref) - atomic_read(&l->ref);
}

static size_t xr_reserve_topn_each_caller(struct xr_stack_info *buf, size_t len, int top)
{
	int i, j;
	int count = 1;
	int same_caller_count = 0;
	int write_index = 0;

	if (buf == NULL || len == 0 || top == -1)
		return len;

	same_caller_count = atomic_read(&buf[0].ref);
	for (i = 1; i < len; i++) {
		if (buf[i].caller == buf[i-1].caller) {
			same_caller_count += atomic_read(&buf[i].ref);
			count++;
			if (count > top) /* invalidate duplicated data */
				atomic_set(&buf[i].ref, 0);
		} else {
			for (j = i - count; j < i; j++)
				buf[j].same_caller_cnt = same_caller_count;
			same_caller_count = atomic_read(&buf[i].ref);
			count = 1;
		}
	}
	/* update last caller */
	for (j = i - count; j < i; j++)
		buf[j].same_caller_cnt = same_caller_count;
	/* remove invalid data */
	for (i = 0; i < len; i++) {
		if (atomic_read(&buf[i].ref)) {
			memcpy(&buf[write_index], &buf[i], sizeof(struct xr_stack_info));
			write_index++;
		}
	}
	return write_index;
}
 /* assume stack_info_list len is SLUB_RINGBUF_LEN */
size_t xr_slub_stack_read(struct xr_stack_info *stack_info_list, size_t len, int type)
{
	size_t cnt;
	int i = 0;
	unsigned long flags;
	struct xr_stack_info *p = NULL;
	struct slub_ring_buf *srb = &srb_array[type];
	struct xr_stack_info *sorted_buf = stack_info_list;

	if (invalid_slub_type(type) || !max_slub_cache) {
		xrmem_err("type %d invalid or max_slub_cache is null\n", type);
		return 0;
	}

	spin_lock_irqsave(&srb->lock, flags);
	cnt = slub_read_node(sorted_buf, len, type);
	spin_unlock_irqrestore(&srb->lock, flags);
	sort(sorted_buf, cnt, sizeof(struct xr_stack_info), compare_same_caller, NULL);
	cnt = xr_reserve_topn_each_caller(sorted_buf, cnt, EACH_CALLER_TOP_NUM);
	sort(sorted_buf, cnt, sizeof(struct xr_stack_info), compare_count, NULL);
	for (i = 0; i < cnt; i++) {
		p = sorted_buf + i;
		xrmem_err("top%d %s %pS cnt:[%d/%lu] size: %d KB pid:[%ld-%ld] %s\n",
			i, type == 0 ? "Alloc" : "Free", (void *)p->caller, atomic_read(&p->ref), p->same_caller_cnt,
			(max_slub_cache->size * atomic_read(&p->ref)) >> 10, p->min_pid, p->max_pid, p->comm);
		if (p->caller) {
			unsigned long *entries;
			unsigned int nr_entries, j;

			nr_entries = stack_depot_fetch(p->caller, &entries);
			for (j = 0; j < nr_entries; j++)
				xrmem_err("        %pS\n", (void *)entries[j]);
		}
	}
	xrmem_err("%s %s %ld items\n", max_slub_cache->name, type == 0 ? "Alloc" : "Free", cnt);
	return cnt;
}

static long slub_create_track(int type)
{
	long err;

	init_waitqueue_head(&slub_track[type].slub_wait);
	mutex_init(&slub_track[type].slub_mutex);
	slub_build_rb_tree(type);
	slub_track[type].type = type;
	slub_track[type].slub_ktrackd = kthread_run(kslubtrackd,
		&slub_track[type], "slub-%s", type == SLUB_ALLOC ? "alloc" : "free");
	if (IS_ERR(slub_track[type].slub_ktrackd)) {
		err = PTR_ERR(slub_track[type].slub_ktrackd);
		xrmem_err("slub_ktrackd failed!\n");
		return err;
	}
	return 0;
}

static long slub_destory_track(int type)
{
	if (slub_track[type].slub_ktrackd) {
		kthread_stop(slub_track[type].slub_ktrackd);
		slub_track[type].slub_ktrackd = NULL;
	}
	return 0;
}

static void maybe_need_notify(void)
{
	static DEFINE_RATELIMIT_STATE(slub_detect_notify, 30 * HZ, 1);

	if (unlikely(xr_should_detect_slab_leak())) {
		/* slub_track_flag may keep to 1 when user thread set it and crashed, so don't use slub_track_flag to judge open or not */
		if (!__ratelimit(&slub_detect_notify))
			return;
		lmkd_notify(SLAB_DETECT_MASK); // notify lmkd slub is too big

		xring_memory_event_report(MM_LEAK_SLAB);
	}
}

static void vh_slab_alloc_node(void *data, void *object, unsigned long addr, struct kmem_cache *s)
{
	if (s == slub_track_cache)
		return;
	maybe_need_notify();
	if (likely(object) && unlikely(s->flags & SLAB_XR_TRACE))
		set_alloc_track(addr);
}

static void vh_slab_free(void *data, unsigned long addr, struct kmem_cache *s)
{
	if (s == slub_track_cache)
		return;
	if (s->flags & SLAB_XR_TRACE)
		set_free_track(addr);
}

static int register_hook(void)
{
	int ret = 0;

	ret |= register_trace_android_vh_slab_alloc_node(vh_slab_alloc_node, NULL);
	ret |= register_trace_android_vh_slab_free(vh_slab_free, NULL);
	return ret;
}

static void unregister_hook(void)
{
	unregister_trace_android_vh_slab_free(vh_slab_free, NULL);
	unregister_trace_android_vh_slab_alloc_node(vh_slab_alloc_node, NULL);
}

int slub_track_init(void)
{
	int ret;

	slub_track_cache = kmem_cache_create("slub_track_cache",
			sizeof(struct slub_rb_node),
			0, 0, NULL);
	if (!slub_track_cache) {
		xrmem_err("create slub_track_cache failed!\n");
		return -EINVAL;
	}
	xr_slab_caches = android_debug_symbol(ADS_SLAB_CACHES);
	if (IS_ERR(xr_slab_caches)) {
		xrmem_err("get xr_slab_caches failed!\n");
		return -EINVAL;
	}
	xr_slab_mutex = android_debug_symbol(ADS_SLAB_MUTEX);
	if (IS_ERR(xr_slab_mutex)) {
		xrmem_err("get xr_slab_mutex failed!\n");
		return -EINVAL;
	}
	ret = register_hook();
	if (ret) {
		xrmem_err("slub track register hook failed!\n");
		return -EINVAL;
	}
	ret = slub_create_track(SLUB_ALLOC);
	if (ret) {
		xrmem_err("slub_create_track SLUB_ALLOC failed!\n");
		return -EINVAL;
	}
	ret = slub_create_track(SLUB_FREE);
	if (ret) {
		xrmem_err("slub_create_track SLUB_FREE failed!\n");
		return -EINVAL;
	}
	return 0;
}

void slub_track_exit(void)
{
	slub_destory_track(SLUB_FREE);
	slub_destory_track(SLUB_ALLOC);
	unregister_hook();
	xr_slab_mutex = NULL;
	xr_slab_caches = NULL;
	kmem_cache_destroy(slub_track_cache);
}

module_param(slub_track_flag, int, 0644);
module_param(specified_target, charp, 0644);
