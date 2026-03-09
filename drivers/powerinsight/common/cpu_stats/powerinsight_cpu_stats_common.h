#ifndef POWERINSIGHT_CPU_STATS_COMMON_H
#define POWERINSIGHT_CPU_STATS_COMMON_H

#include <linux/sched.h>
#include <linux/sched/signal.h>

#define KERNEL_TGID				0
#define KERNEL_NAME				"kernel"
#define PREFIX_LEN				32
#define MAX_CPU_FREQ_NR			64

static inline bool powerinsight_is_kthread(struct task_struct *task)
{
	return task->flags & PF_KTHREAD;
}

static inline bool powerinsight_is_kernel_tgid(pid_t tgid)
{
	return tgid == KERNEL_TGID;
}

// combine kernel thread to same entry which tgid is 0
static inline int powerinsight_get_task_normalized_tgid(struct task_struct *task)
{
	return (powerinsight_is_kthread(task) ? KERNEL_TGID : task->tgid);
}

static inline const char *powerinsight_get_task_normalized_name(struct task_struct *task)
{
	return (powerinsight_is_kthread(task) ? KERNEL_NAME : (task->group_leader ?
		task->group_leader->comm : task->comm));
}

static inline bool powerinsight_thread_group_dying(struct task_struct *task)
{
	return thread_group_leader(task);
}

static inline bool powerinsight_is_task_alive(struct task_struct *task)
{
#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
	// uid_sys_stats.c will set @cpu_power as ULLONG_MAX when task is exiting
	if (task->cpu_power == ULLONG_MAX)
		return false;
#endif
	return !(task->flags & PF_EXITING);
}

int powerinsight_set_proc_decompose(const void __user *argp);
void powerinsight_remove_proc_decompose(pid_t tgid);
void powerinsight_set_proc_entry_decompose(pid_t tgid, struct task_struct *task, const char *name);

#endif // POWERINSIGHT_CPU_STATS_COMMON_H
