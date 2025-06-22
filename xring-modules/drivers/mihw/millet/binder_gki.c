// MIUI ADD: Power_ProcessFreeze
/*
 * Copyright (c) Xiaomi Technologies Co., Ltd. 2020. All rights reserved.
 *
 * File name: oem_binder.c
 * Description: millet-binder-driver
 * Author: guchao1@xiaomi.com
 * Version: 1.0
 * Date:  2020/9/9
 */
#define pr_fmt(fmt) "millet-binder_gki: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/freezer.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/trace_clock.h>
#include "millet.h"
#include "binder_oem.h"
#include <trace/hooks/binder.h>
#include <../../../../drivers/android/binder_internal.h>

extern bool millet_binder_switch;
extern unsigned int millet_debug;
static struct hlist_head * get_binder_hhead = NULL;
static struct mutex * get_binder_lock = NULL;
static bool bgot = false;
static ktime_t last_check_buffer_time = { 0 };

struct oem_binder_hook oem_binder_hook_set = {
	.oem_wahead_thresh = 0,
	.oem_wahead_space = 0,
	.oem_reply_hook = NULL,
	.oem_trans_hook = NULL,
	.oem_wait4_hook = NULL,
	.oem_query_st_hook = NULL,
	.oem_buf_overflow_hook = NULL,
};

/**
 * binder_inner_proc_lock() - Acquire inner lock for given binder_proc
 * @proc:         struct binder_proc to acquire
 *
 * Acquires proc->inner_lock. Used to protect todo lists
 */
#define binder_inner_proc_lock(proc) _binder_inner_proc_lock(proc, __LINE__)
static void
_binder_inner_proc_lock(struct binder_proc *proc, int line)
	__acquires(&proc->inner_lock)
{
	spin_lock(&proc->inner_lock);
}

/**
 * binder_inner_proc_unlock() - Release inner lock for given binder_proc
 * @proc:         struct binder_proc to acquire
 *
 * Release lock acquired via binder_inner_proc_lock()
 */
#define binder_inner_proc_unlock(proc) _binder_inner_proc_unlock(proc, __LINE__)
static void
_binder_inner_proc_unlock(struct binder_proc *proc, int line)
	__releases(&proc->inner_lock)
{
	spin_unlock(&proc->inner_lock);
}

static bool binder_worklist_empty_ilocked(struct list_head *list)
{
	return list_empty(list);
}

static enum BINDER_STAT query_binder_stat(struct binder_proc *proc)
{
	struct rb_node *n = NULL;
	struct binder_thread *thread = NULL;
	int pid, tid, uid = 0;
	enum BINDER_STAT stat;
	struct task_struct *tsk;

	if (!oem_binder_hook_set.oem_query_st_hook)
		return BINDER_IN_IDLE;

	if (proc->tsk)
		uid = task_uid(proc->tsk).val;
	else
		return BINDER_IN_IDLE;
	binder_inner_proc_lock(proc);
	if (proc->tsk && !binder_worklist_empty_ilocked(&proc->todo)) {
		tsk = proc->tsk;
		tid = tsk->pid;
		pid = task_pid_nr(tsk);
		stat = BINDER_PROC_IN_BUSY;
		goto busy;
	}

	for (n = rb_first(&proc->threads); n != NULL; n = rb_next(n)) {
		thread = rb_entry(n, struct binder_thread, rb_node);
		if (!thread->task)
			continue;

		if (!binder_worklist_empty_ilocked(&thread->todo)) {
			tsk = thread->task;
			pid = task_tgid_nr(tsk);
			tid = thread->pid;
			stat = BINDER_THREAD_IN_BUSY;
			goto busy;
		}

		if (!thread->transaction_stack)
			continue;

		spin_lock(&thread->transaction_stack->lock);
		if (thread->transaction_stack->to_thread == thread) {
			tsk = thread->task;
			pid = task_tgid_nr(tsk);
			tid = thread->pid;
			stat = BINDER_IN_TRANSACTION;
			spin_unlock(&thread->transaction_stack->lock);
			goto busy;
		}
		spin_unlock(&thread->transaction_stack->lock);
	}

	binder_inner_proc_unlock(proc);
	return BINDER_IN_IDLE;
busy:
	binder_inner_proc_unlock(proc);
//	oem_binder_hook_set.oem_query_st_hook(uid, tsk, tid, pid, stat);
	return stat;
}


static enum BINDER_STAT query_binder_stat_new(struct binder_proc *proc)
{
	struct rb_node *n = NULL;
	struct binder_thread *thread = NULL;
	int pid, tid, uid = 0;
	enum BINDER_STAT stat;
	struct task_struct *tsk;
	struct binder_work *w = NULL;
	struct binder_transaction *btrans = NULL;
	int need_reply = -1;

	if (!oem_binder_hook_set.oem_query_st_hook)
		return BINDER_IN_IDLE;

	if (proc->tsk)
		uid = task_uid(proc->tsk).val;
	else
		return BINDER_IN_IDLE;
	binder_inner_proc_lock(proc);
	if (proc->tsk && !binder_worklist_empty_ilocked(&proc->todo)) {
		list_for_each_entry(w, &proc->todo, entry) {
			if (w != NULL && w->type == BINDER_WORK_TRANSACTION) {
				btrans = container_of(w, struct binder_transaction, work);
				spin_lock(&btrans->lock);
				if (btrans != NULL && btrans->to_thread == thread) {
					need_reply = (int)(btrans->need_reply == 1);
				}
				spin_unlock(&btrans->lock);
				if (need_reply == 1) {
					tsk = proc->tsk;
					tid = tsk->pid;
					pid = task_pid_nr(tsk);
					stat = BINDER_PROC_IN_BUSY;
					goto busy;
				}
			}
		}
	}

	for (n = rb_first(&proc->threads); n != NULL; n = rb_next(n)) {
		thread = rb_entry(n, struct binder_thread, rb_node);
		if (!thread->task)
			continue;

		if (!binder_worklist_empty_ilocked(&thread->todo)) {
				list_for_each_entry(w, &thread->todo, entry) {
					if (w != NULL && w->type == BINDER_WORK_TRANSACTION) {
						btrans = container_of(w, struct binder_transaction, work);
						spin_lock(&btrans->lock);
						if (btrans != NULL && btrans->to_thread == thread) {
							need_reply = (int)(btrans->need_reply == 1);
						}
						spin_unlock(&btrans->lock);
						if (need_reply == 1) {
							tsk = thread->task;
							pid = task_tgid_nr(tsk);
							tid = thread->pid;
							stat = BINDER_THREAD_IN_BUSY;
							goto busy;
						}
					}
				}
		}

		btrans = thread->transaction_stack;
		if (!btrans)
			continue;

		spin_lock(&thread->transaction_stack->lock);
		if (thread->transaction_stack->to_thread == thread) {
			need_reply = (int)(btrans->need_reply == 1);
			if (need_reply == 1) {
				tsk = thread->task;
				pid = task_tgid_nr(tsk);
				tid = thread->pid;
				stat = BINDER_IN_TRANSACTION;
				spin_unlock(&thread->transaction_stack->lock);
				goto busy;
			}
		}
		spin_unlock(&thread->transaction_stack->lock);
	}

	binder_inner_proc_unlock(proc);
	return BINDER_IN_IDLE;
busy:
	binder_inner_proc_unlock(proc);
//	oem_binder_hook_set.oem_query_st_hook(uid, tsk, tid, pid, stat);
	return stat;
}

void query_binder_app_stat(int uid)
{
	struct binder_proc *proc;
	bool idle_f = true;
	enum BINDER_STAT stat;
	if (!oem_binder_hook_set.oem_query_st_hook ||
		!get_binder_lock ||!get_binder_hhead)
		return;
	mutex_lock(get_binder_lock);
	hlist_for_each_entry(proc, get_binder_hhead, proc_node) {
		if (proc != NULL && proc->tsk
			&& (task_uid(proc->tsk).val == uid)) {
			if (millet_binder_switch) {
				stat = query_binder_stat_new(proc);
			} else {
				stat = query_binder_stat(proc);
			}
			if (stat != BINDER_IN_IDLE)
				idle_f = false;
		}
	}

	if (idle_f)
		stat = BINDER_IN_IDLE;
	else
		stat = BINDER_IN_BUSY;

	oem_binder_hook_set.oem_query_st_hook(uid, current, 0, current->pid, stat);
	mutex_unlock(get_binder_lock);
}
EXPORT_SYMBOL_GPL(query_binder_app_stat);


struct task_struct *binder_buff_owner(size_t *free_async_space)
{
	struct binder_alloc *alloc = NULL;
	struct binder_proc *proc = NULL;

	if (!free_async_space)
		return NULL;

	alloc = container_of(free_async_space, struct binder_alloc,
				free_async_space);
	proc = container_of(alloc, struct binder_proc, alloc);
	return proc->tsk;
}

static inline struct binder_buffer *binder_buffer_next(struct binder_buffer *buffer)
{
    return list_entry(buffer->entry.next, struct binder_buffer, entry);
}

static inline size_t binder_alloc_buffer_size(struct binder_alloc *alloc,
               struct binder_buffer *buffer)
{
    if (list_is_last(&buffer->entry, &alloc->buffers)) {
        return alloc->buffer + alloc->buffer_size - buffer->user_data;
    }
    return binder_buffer_next(buffer)->user_data - buffer->user_data;
}

void mi_check_buffer_size_locked(size_t size, size_t *free_async_space, int is_async)
{
    struct binder_alloc *alloc;
    struct rb_node *n;
    struct binder_buffer *buffer = NULL;
    size_t total_alloc_size = 0;
    size_t num_buffers = 0;
    size_t total_async_alloc_size = 0;
    size_t num_async_buffers = 0;

    struct binder_transaction *t = NULL;
    struct binder_proc *to_proc = NULL;
    int to_pid = 0;
    int to_tid = 0;
    ktime_t current_time = ktime_get();

    if (!is_async || ktime_ms_delta(current_time, last_check_buffer_time) < BUFFER_CHECK_INTERVAL_MS) {
        return;
    }
    alloc = container_of(free_async_space, struct binder_alloc, free_async_space);

    if (!alloc || (size + CHECK_BUFFER_DETAIL_SAPCE) < alloc->free_async_space) {
        return;
    }

    pr_info("MIUI async transaction from %5d:%5d to %5d:0 apply:%zd free:%zd, may fail!\n",
            current->tgid, current->pid,
            alloc->pid,
            size,
            alloc->free_async_space);

    pr_info("MIUI large buffers(>32K):\n");

    for (n = rb_first(&alloc->allocated_buffers); n != NULL; n = rb_next(n)) {
	size_t temp_size = 0;
	buffer = rb_entry(n, struct binder_buffer, rb_node);

	temp_size = binder_alloc_buffer_size(alloc, buffer) + sizeof(struct binder_buffer);
	total_alloc_size += temp_size;
	num_buffers++;
	if (buffer->async_transaction) {
	total_async_alloc_size += temp_size;
	num_async_buffers++;
	}

	if (temp_size < LARGE_BUFFER_THRESHOLD) {
		continue;
	}

	t = buffer->transaction;
	if (!t) {
		continue;
	}

	spin_lock(&t->lock);
	to_proc = t->to_proc;
	to_pid = to_proc ? to_proc->pid : 0;
to_tid = t->to_thread ? t->to_thread->pid : 0;

	pr_info("MIUI pending %s transaction from %5d:%5d to %5d:%5d node:%d code:%x"
	" size:%zd duration:%lldms\n",
	buffer->async_transaction ? "async" : t->from ? "call" : "reply",
	t->from_pid, t->from_tid,
	to_pid, to_tid,
	buffer->target_node ? buffer->target_node->debug_id : 0,
	t->code, temp_size,
	ktime_ms_delta(current_time, t->start_time));
	spin_unlock(&t->lock);
    }
    if (num_buffers > 0 || num_async_buffers > 0) {
	pr_info("MIUI total num:%zd total async num:%zd,total size:%zd total async size:%zd\n",
	num_buffers, num_async_buffers,
	total_alloc_size, total_async_alloc_size);
    }
    last_check_buffer_time = current_time;
}

void mi_binder_alloc_new_buf_locked(void *data, size_t size, size_t *free_async_space, int is_async, bool *should_fail)
{
    mi_check_buffer_size_locked(size, free_async_space, is_async);
	if (oem_binder_hook_set.oem_buf_overflow_hook && is_async
		&& ((*free_async_space < oem_binder_hook_set.oem_wahead_thresh
		* (size + sizeof(struct binder_buffer)))
		|| (*free_async_space < oem_binder_hook_set.oem_wahead_space))) {
			struct task_struct *owner;

			owner = binder_buff_owner(free_async_space);

			if (owner)
				oem_binder_hook_set.oem_buf_overflow_hook(owner, current,
						current->pid, false, 0);
	}
}

void mi_binder_replay(void *data, struct binder_proc *target_proc, struct binder_proc *proc,
	struct binder_thread *thread, struct binder_transaction_data *tr)
{
	if (oem_binder_hook_set.oem_reply_hook && target_proc->tsk)
		oem_binder_hook_set.oem_reply_hook(target_proc->tsk, proc->tsk,
				thread->pid, tr->flags & TF_ONE_WAY,
				tr->code);
}

void mi_binder_transaction(void *data, struct binder_proc *target_proc, struct binder_proc *proc,
	struct binder_thread *thread, struct binder_transaction_data *tr)
{
	if (oem_binder_hook_set.oem_trans_hook && target_proc && target_proc->tsk)
		oem_binder_hook_set.oem_trans_hook(target_proc->tsk, proc->tsk,
				thread->pid, tr->flags & TF_ONE_WAY,
				tr->code);
	if (millet_debug) {
		char buf_data[TOKEN_BUFF_SIZE];
		size_t buf_data_size;
		char buf[TOKEN_BUFF_SIZE] = {0};
		int i = 0;
		int j = 0;
		if ((tr->flags & TF_ONE_WAY)
			&& target_proc
			&& (task_uid(target_proc->tsk).val > 10000)
			&& frozen_task_group(target_proc->tsk)) {
			buf_data_size = tr->data_size > TOKEN_BUFF_SIZE ? TOKEN_BUFF_SIZE : tr->data_size;
			if (!copy_from_user(buf_data, (char *)tr->data.ptr.buffer, buf_data_size)) {
				if (buf_data_size > TOKEN_OFFSET) {
					char *p = (char *)(buf_data) + TOKEN_OFFSET;
					j = TOKEN_OFFSET + 1;
					while (i < TOKEN_BUFF_SIZE && j < buf_data_size && *p != '\0') {
						buf[i++] = *p;
						j += 2;
						p += 2;
					}
					if (i == TOKEN_BUFF_SIZE)
						buf[i - 1] = '\0';
				}
				printk(KERN_ERR "millet_kernel calleruid:%d callerPid:%d targetUid:%d targetPid:%d  code:%d  desc:%s", task_uid(proc->tsk).val, task_tgid_nr(proc->tsk),
						task_uid(target_proc->tsk).val, task_tgid_nr(target_proc->tsk), tr->code, buf);
			}
		}
	}
}

void mi_binder_wait_for_work(void *data, bool do_proc_work, struct binder_thread *thread, struct binder_proc *proc)
{
	struct task_struct *dst;

	if (!thread->transaction_stack)
		return;

	spin_lock(&thread->transaction_stack->lock);
	if (oem_binder_hook_set.oem_wait4_hook
			&& !thread->is_dead
			&& thread->transaction_stack
			&& thread->transaction_stack->to_proc
			&& thread->transaction_stack->to_proc->tsk){
		dst = thread->transaction_stack->to_proc->tsk;
		get_task_struct(dst);
		spin_unlock(&thread->transaction_stack->lock);
	} else {
		spin_unlock(&thread->transaction_stack->lock);
		return;
	}
	oem_binder_hook_set.oem_wait4_hook(dst,
				proc->tsk,
				thread->pid,
				thread->transaction_stack->flags & TF_ONE_WAY,
				thread->transaction_stack->code);
	put_task_struct(dst);
}

void mi_get_hhead_and_lock(void *data, struct hlist_head *hhead, struct mutex *lock, struct binder_proc *proc)
{
	if (!bgot) {
		if (hhead)
			get_binder_hhead = hhead;
		if (lock)
			get_binder_lock = lock;
		bgot = true;
	}
}

void oem_register_binder_hook(struct oem_binder_hook *set)
{
	if (!set)
		return;

	oem_binder_hook_set.oem_wahead_thresh = set->oem_wahead_thresh;
	oem_binder_hook_set.oem_wahead_space = set->oem_wahead_space;
	oem_binder_hook_set.oem_reply_hook = set->oem_reply_hook;
	oem_binder_hook_set.oem_trans_hook = set->oem_trans_hook;
	oem_binder_hook_set.oem_wait4_hook = set->oem_wait4_hook;
	oem_binder_hook_set.oem_query_st_hook = set->oem_query_st_hook;
	oem_binder_hook_set.oem_buf_overflow_hook = set->oem_buf_overflow_hook;
}
EXPORT_SYMBOL_GPL(oem_register_binder_hook);


static int __init init_binder_gki(void)
{
	pr_err("enter init_binder_gki func!\n");
	register_trace_android_vh_binder_alloc_new_buf_locked(mi_binder_alloc_new_buf_locked, NULL);
	register_trace_android_vh_binder_reply(mi_binder_replay, NULL);
	register_trace_android_vh_binder_trans(mi_binder_transaction, NULL);
	register_trace_android_vh_binder_wait_for_work(mi_binder_wait_for_work, NULL);
	register_trace_android_vh_binder_preset(mi_get_hhead_and_lock, NULL);

	return 0;
}

module_init(init_binder_gki);

MODULE_LICENSE("GPL");
// END Power_ProcessFreeze
