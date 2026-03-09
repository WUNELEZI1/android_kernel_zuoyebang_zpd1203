#include "powerinsight_cpu_stats.h"

#include <linux/init.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched/stat.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/sched/signal.h>
#endif

#include "powerinsight_ioctl.h"

#include "cpu_stats/powerinsight_cpu_stats_common.h"
#include "utils/powerinsight_hashtable.h"
#include "utils/powerinsight_utils.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
typedef u64 powerinsight_cputime_t;
#else
typedef cputime_t powerinsight_cputime_t;
#endif

#define IOC_PROC_CPUTIME_ENABLE POWERINSIGHT_CPU_DIR_IOC(_IOC_WRITE, 1, int)
// #define IOC_PROC_CPUTIME_GET POWERINSIGHT_CPU_DIR_IOC(_IOC_READ, 2, struct powerinsight_cpu_stats)
#define IOC_PROC_CPUTIME_GET POWERINSIGHT_CPU_DIR_IOC(_IOC_READ, 2, struct powerinsight_cpu_stats*)
#define IOC_TASK_CPUPOWER_SUPPORT POWERINSIGHT_CPU_DIR_IOC(_IOC_READ, 3, bool)
#define IOC_PROC_DECOMPOSE_SET POWERINSIGHT_CPU_DIR_IOC(_IOC_WRITE, 4, struct dev_transmit_t)

#define PROC_HASH_BITS			15
#define THREAD_HASH_BITS		10
#define BASE_COUNT				1500
#define MAX_PROC_AVAIL_COUNT	1024
#define MAX_THREAD_AVAIL_COUNT	2048
#define RCU_LOCK_BREAK_TIMEOUT	(HZ / 10)

#define powerinsight_check_entry_state(entry, expected)	((entry)->state == (expected))
#define powerinsight_check_update_entry_state(entry, expected, updated) \
	({ \
		bool ret = powerinsight_check_entry_state(entry, expected); \
		if (!ret) \
			(entry)->state = (updated); \
		ret; \
	})

enum {
	CMDLINE_NEED_TO_GET = 0,
	CMDLINE_PROCESS,
	CMDLINE_THREAD,
};

enum {
	PROCESS_STATE_DEAD = 0,
	PROCESS_STATE_ACTIVE,
};

enum {
	POWERINSIGHT_STATE_DEAD,
	POWERINSIGHT_STATE_ALIVE,
	POWERINSIGHT_STATE_UNSETTLED,
};

struct powerinsight_thread_entry {
	pid_t pid;
	powerinsight_cputime_t time;
#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
	unsigned long long power;
#endif
	int8_t state;
	char name[NAME_LEN];
	struct hlist_node hash;
};

struct powerinsight_proc_entry {
	pid_t tgid;
	uid_t uid;
	powerinsight_cputime_t dead_time;
	powerinsight_cputime_t active_time;
#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
	unsigned long long dead_power;
	unsigned long long active_power;
#endif
	int8_t state;
	bool decomposed;
	bool cmdline;
	char name[NAME_LEN];
	struct powerinsight_dynamic_hashtable *threads;
	struct hlist_node hash;
};

struct powerinsight_cputime_transmit {
	long long timestamp;
	int type;
	int count;
	unsigned char value[0];
} __packed;

static int powerinsight_proc_cputime_enable(void __user *argp);
static int powerinsight_get_task_cpupower_enable(void __user *argp);
static int powerinsight_get_proc_cputime(void __user *argp);
static int powerinsight_get_stats(struct powerinsight_cpu_stats *stats);

static DECLARE_POWERINSIGHT_HASHTABLE(proc_hash_table, PROC_HASH_BITS);
static DEFINE_MUTEX(powerinsight_proc_lock);
static atomic_t proc_cputime_enable;
static atomic_t dead_count;
static atomic_t active_count;
static pid_t powerinsightd_tgid;
static int proc_entry_avail_cnt;
static int thread_entry_avail_cnt;
static struct powerinsight_proc_entry *proc_entry_avail_pool[MAX_PROC_AVAIL_COUNT];
static struct powerinsight_thread_entry *thread_entry_avail_pool[MAX_THREAD_AVAIL_COUNT];
static bool update_active_succ = true;

#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
static const atomic_t task_power_enable = ATOMIC_INIT(1);
#else
static const atomic_t task_power_enable = ATOMIC_INIT(0);
#endif

long powerinsight_ioctl_cpu(unsigned int cmd, void __user *argp)
{
	int rc = 0;
	powerinsight_info("Received ioctl: cmd=0x%X, size=%u, dir=%d, type=%d, nr=%d",
		cmd, _IOC_SIZE(cmd), _IOC_DIR(cmd),
		_IOC_TYPE(cmd), _IOC_NR(cmd));

	// 使用 if-else 结构代替 switch
	if (cmd == IOC_PROC_CPUTIME_ENABLE) {
		rc = powerinsight_proc_cputime_enable(argp);
		powerinsight_info("IOC_PROC_CPUTIME_ENABLE handled, result: %d", rc);
	} else if (cmd == IOC_PROC_CPUTIME_GET) {
		powerinsight_info("cputime get. Enable state: %d",
			atomic_read(&proc_cputime_enable));
		if (!atomic_read(&proc_cputime_enable)) {
			powerinsight_err("Permission denied: stats not enabled");
			return -EPERM;
		}
		rc = powerinsight_get_proc_cputime(argp);
		powerinsight_info("IOC_PROC_CPUTIME_GET handled, result: %d", rc);
	} else if (cmd == IOC_TASK_CPUPOWER_SUPPORT) {
		rc = powerinsight_get_task_cpupower_enable(argp);
		powerinsight_info("IOC_TASK_CPUPOWER_SUPPORT handled, result: %d", rc);
	} else if (cmd == IOC_PROC_DECOMPOSE_SET) {
		rc = powerinsight_set_proc_decompose(argp);
		powerinsight_info("IOC_PROC_DECOMPOSE_SET handled, result: %d", rc);
	} else {
		powerinsight_err("Invalid ioctl command: 0x%x", cmd);
		rc = -ENOTTY;
	}

	return rc;
}

static inline unsigned long long powerinsight_cputime_to_msecs(powerinsight_cputime_t time)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	return ((unsigned long long)ktime_to_ms(time));
#else
	return ((unsigned long long)jiffies_to_msecs(cputime_to_jiffies(time)));
#endif
}

static inline bool powerinsight_is_proc_entry_decomposed(struct powerinsight_proc_entry *entry)
{
	return (entry->decomposed && entry->threads);
}

static void powerinsight_copy_name(char *to, const char *from, size_t len)
{
	char *p = NULL;

	if (strlen(from) <= len) {
		strncpy(to, from, len);
		return;
	}

	p = strrchr(from, '/');
	if (p != NULL && (*(p + 1) != '\0'))
		strncpy(to, p + 1, len);
	else
		strncpy(to, from, len);
}

static struct powerinsight_proc_entry *powerinsight_get_avail_proc_entry(void)
{
	struct powerinsight_proc_entry *entry = NULL;

	if ((proc_entry_avail_cnt > 0) && (proc_entry_avail_cnt <= MAX_PROC_AVAIL_COUNT)) {
		entry = proc_entry_avail_pool[proc_entry_avail_cnt - 1];
		proc_entry_avail_pool[proc_entry_avail_cnt - 1] = NULL;
		proc_entry_avail_cnt--;
	}

	return entry;
}

static struct powerinsight_thread_entry *powerinsight_get_avail_thread_entry(void)
{
	struct powerinsight_thread_entry *entry = NULL;

	if ((thread_entry_avail_cnt > 0) && (thread_entry_avail_cnt <= MAX_THREAD_AVAIL_COUNT)) {
		entry = thread_entry_avail_pool[thread_entry_avail_cnt - 1];
		thread_entry_avail_pool[thread_entry_avail_cnt - 1] = NULL;
		thread_entry_avail_cnt--;
	}

	return entry;
}

static void powerinsight_free_proc_entry(struct powerinsight_proc_entry *entry)
{
	hash_del(&entry->hash);
	powerinsight_free_dynamic_hashtable(entry->threads);
	if (proc_entry_avail_cnt == MAX_PROC_AVAIL_COUNT) {
		kfree(entry);
		return;
	}
	memset(entry, 0, sizeof(struct powerinsight_proc_entry));
	proc_entry_avail_pool[proc_entry_avail_cnt] = entry;
	proc_entry_avail_cnt++;
}

static void powerinsight_free_thread_entry(struct powerinsight_thread_entry *entry)
{
	hash_del(&entry->hash);
	if (thread_entry_avail_cnt == MAX_THREAD_AVAIL_COUNT) {
		kfree(entry);
		return;
	}
	memset(entry, 0, sizeof(struct powerinsight_thread_entry));
	thread_entry_avail_pool[thread_entry_avail_cnt] = entry;
	thread_entry_avail_cnt++;
}

static struct powerinsight_thread_entry *powerinsight_find_or_register_thread_entry_locked(pid_t pid,
		const char *comm, struct powerinsight_proc_entry *entry)
{
	struct powerinsight_thread_entry *thread = NULL;

	if (!powerinsight_is_proc_entry_decomposed(entry))
		return NULL;

	powerinsight_dynamic_hash_for_each_possible(entry->threads, thread, hash, pid) {
		if (thread->pid == pid)
			goto update_thread_name;
	}

	thread = powerinsight_get_avail_thread_entry();
	if (!thread) {
		thread = kzalloc(sizeof(struct powerinsight_thread_entry), GFP_ATOMIC);
		if (!thread) {
			powerinsight_err("Failed to allocate memory");
			return NULL;
		}
	}
	thread->pid = pid;
	thread->state = POWERINSIGHT_STATE_ALIVE;
	powerinsight_dynamic_hash_add(entry->threads, &thread->hash, pid);
	atomic_inc(&active_count);

update_thread_name:
	snprintf(thread->name, NAME_LEN - 1, "%s_%s", entry->name, comm);

	return thread;
}

static struct powerinsight_proc_entry *powerinsight_find_proc_entry_locked(pid_t tgid)
{
	struct powerinsight_proc_entry *entry = NULL;

	powerinsight_hash_for_each_possible(proc_hash_table, entry, hash, tgid) {
		if (entry->tgid == tgid)
			return entry;
	}

	return NULL;
}

static struct powerinsight_proc_entry *powerinsight_register_proc_entry_locked(uid_t uid, pid_t tgid, bool cmdline, const char *name)
{
	struct powerinsight_proc_entry *entry = NULL;

	entry = powerinsight_get_avail_proc_entry();
	if (!entry) {
		entry = kzalloc(sizeof(struct powerinsight_proc_entry), GFP_ATOMIC);
		if (!entry) {
			powerinsight_err("Failed to allocate memory");
			return NULL;
		}
	}
	entry->tgid = tgid;
	entry->uid = uid;
	entry->state = POWERINSIGHT_STATE_ALIVE;
	entry->decomposed = false;
	entry->cmdline = cmdline;
	strncpy(entry->name, name, NAME_LEN - 1);
	entry->threads = NULL;
	powerinsight_hash_add(proc_hash_table, &entry->hash, tgid);
	atomic_inc(&active_count);

	return entry;
}

static struct powerinsight_proc_entry *powerinsight_find_or_register_proc_entry_locked(pid_t tgid, struct task_struct *task)
{
	uid_t uid;
	const char *name = NULL;
	struct powerinsight_proc_entry *entry = NULL;

	name = powerinsight_get_task_normalized_name(task);
	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	entry = powerinsight_find_proc_entry_locked(tgid);
	if (entry == NULL)
		return powerinsight_register_proc_entry_locked(uid, tgid, powerinsight_is_kthread(task), name);

	// always update uid and name because they are mutable sometimes.
	entry->uid = uid;
	if (!entry->cmdline && strncmp(entry->name, name, NAME_LEN - 1))
		strncpy(entry->name, name, NAME_LEN - 1);

	return entry;
}

void powerinsight_set_proc_entry_decompose(pid_t tgid, struct task_struct *task, const char *name)
{
	struct powerinsight_proc_entry *entry = NULL;

	mutex_lock(&powerinsight_proc_lock);
	entry = powerinsight_find_or_register_proc_entry_locked(tgid, task);
	if (!entry)
		goto out;

	entry->threads = powerinsight_alloc_dynamic_hashtable(THREAD_HASH_BITS);
	if (entry->threads) {
		entry->decomposed = true;
		entry->cmdline = true;
		strncpy(entry->name, name, NAME_LEN - 1);
		powerinsight_info("Set proc entry decomposed, tgid: %d, %s", entry->tgid, entry->name);
	}

out:
	mutex_unlock(&powerinsight_proc_lock);
}

/*
 * To avoid extending the RCU grace period for an unbounded amount of time,
 * periodically exit the critical section and enter a new one.
 * For preemptible RCU it is sufficient to call rcu_read_unlock in order
 * to exit the grace period. For classic RCU, a reschedule is required.
 */
static bool powerinsight_rcu_lock_break(struct task_struct *g, struct task_struct *t)
{
	bool can_cont = true;

	get_task_struct(g);
	get_task_struct(t);
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
	can_cont = pid_alive(g) && pid_alive(t);
	put_task_struct(t);
	put_task_struct(g);

	return can_cont;
}

static int powerinsight_get_stats(struct powerinsight_cpu_stats *stats)
{
    unsigned long bkt, tbkt;
    struct powerinsight_proc_entry *entry = NULL;
    struct powerinsight_thread_entry *thread = NULL;
    struct task_struct *task = NULL, *temp = NULL;
    powerinsight_cputime_t utime, stime, time;
    unsigned long last_break;
    u64 cal_time;
    int proc_cnt = 0;
    
    // 初始化输出结构
    memset(stats, 0, sizeof(struct powerinsight_cpu_stats));
    
    powerinsight_info("Collecting CPU stats...");
    
    // 第一部分：收集死进程/线程的统计信息
    mutex_lock(&powerinsight_proc_lock);
    
    powerinsight_hash_for_each(proc_hash_table, bkt, entry, hash) {
        bool decomposed = powerinsight_is_proc_entry_decomposed(entry);
        
        // 处理死线程
        if (decomposed) {
            powerinsight_dynamic_hash_for_each(entry->threads, tbkt, thread, hash) {
                if (!powerinsight_check_entry_state(thread, POWERINSIGHT_STATE_DEAD) ||
                    thread->time == 0)
                    continue;
                
                // 添加到输出
                if (stats->count < MAX_STAT_ENTRIES) {
                    stats->entries[stats->count].uid = entry->uid;
                    stats->entries[stats->count].pid = thread->pid;
                    stats->entries[stats->count].time = powerinsight_cputime_to_msecs(thread->time);
                    #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
                    stats->entries[stats->count].power = thread->power;
                    #endif
                    stats->entries[stats->count].cmdline = CMDLINE_THREAD;
                    powerinsight_copy_name(stats->entries[stats->count].name, thread->name, NAME_LEN - 1);
                    stats->count++;
                }
                
                // 清除死线程
                powerinsight_free_thread_entry(thread);
            }
        }
        
        // 处理死进程
        if (!powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_DEAD) ||
            entry->dead_time == 0)
            continue;
        
        // 添加到输出
        if (stats->count < MAX_STAT_ENTRIES) {
            stats->entries[stats->count].uid = entry->uid;
            stats->entries[stats->count].pid = entry->tgid;
            stats->entries[stats->count].time = powerinsight_cputime_to_msecs(entry->dead_time);
            #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
            stats->entries[stats->count].power = entry->dead_power;
            #endif
            stats->entries[stats->count].cmdline = entry->cmdline ? CMDLINE_PROCESS : CMDLINE_NEED_TO_GET;
            powerinsight_copy_name(stats->entries[stats->count].name, entry->name, NAME_LEN - 1);
            stats->count++;
        }
        
        // 清除死进程
        if (!decomposed || powerinsight_dynamic_hash_empty(entry->threads)) {
            powerinsight_free_proc_entry(entry);
        }
    }
    
    atomic_set(&dead_count, 0);
    powerinsight_info("Collected %d dead processes/threads", stats->count);
    
    // 第二部分：更新活动进程/线程的统计信息
    update_active_succ = true;
    rcu_read_lock();
    cal_time = ktime_get_ns();
    last_break = jiffies;
    
    for_each_process_thread(temp, task) {
        pid_t tgid;
        struct powerinsight_thread_entry *thread_entry = NULL;
        
        if (!powerinsight_is_task_alive(task))
            continue;
        
        tgid = powerinsight_get_task_normalized_tgid(task);
        
        // 定期释放RCU锁防止阻塞太久
        if (((proc_cnt % 10) == 0) && 
            time_is_before_jiffies(last_break + RCU_LOCK_BREAK_TIMEOUT)) {
            if (!powerinsight_rcu_lock_break(temp, task)) {
                powerinsight_err("Cannot continue to update cpu stats");
                update_active_succ = false;
                break;
            }
            last_break = jiffies;
        }
        proc_cnt++;
        
        // 查找或创建进程条目
        if (!entry || entry->tgid != tgid) {
            entry = powerinsight_find_or_register_proc_entry_locked(tgid, task);
            if (!entry) {
                powerinsight_err("Failed to find/create proc entry for tgid %d", tgid);
                continue;
            }
        }
        
        // 获取任务CPU时间
        task_cputime_adjusted(task, &utime, &stime);
        time = utime + stime;
        
        // 更新进程统计
        if (!powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_DEAD)) {
            entry->state = POWERINSIGHT_STATE_ALIVE;
            entry->active_time += time;
            #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
            entry->active_power += task->cpu_power;
            #endif
        }
        
        // 处理线程
        thread_entry = powerinsight_find_or_register_thread_entry_locked(
            task->pid, task->comm, entry);
        
        if (thread_entry && 
            !powerinsight_check_entry_state(thread_entry, POWERINSIGHT_STATE_DEAD)) {
            thread_entry->state = POWERINSIGHT_STATE_ALIVE;
            thread_entry->time = time;
            #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
            thread_entry->power = task->cpu_power;
            #endif
        }
    }
    
    powerinsight_info("CPU stats update took %lld ms", 
                     (ktime_get_ns() - cal_time) / NSEC_PER_MSEC);
    rcu_read_unlock();
    
    // 第三部分：收集活动进程/线程的统计信息
    powerinsight_hash_for_each(proc_hash_table, bkt, entry, hash) {
        // 跳过未正确更新的条目
        if (powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_UNSETTLED) ||
            (!update_active_succ && 
             powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_ALIVE)) ||
            entry->active_time == 0) {
            continue;
        }
        
        // 处理未分解的进程
        if (!powerinsight_is_proc_entry_decomposed(entry)) {
            if (stats->count < MAX_STAT_ENTRIES) {
                stats->entries[stats->count].uid = entry->uid;
                stats->entries[stats->count].pid = entry->tgid;
                stats->entries[stats->count].time = powerinsight_cputime_to_msecs(entry->active_time);
                #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
                stats->entries[stats->count].power = entry->active_power;
                #endif
                stats->entries[stats->count].cmdline = entry->cmdline ? CMDLINE_PROCESS : CMDLINE_NEED_TO_GET;
                powerinsight_copy_name(stats->entries[stats->count].name, entry->name, NAME_LEN - 1);
                stats->count++;
            }
            continue;
        }
        
        // 处理线程
        powerinsight_dynamic_hash_for_each(entry->threads, tbkt, thread, hash) {
            if (powerinsight_check_entry_state(thread, POWERINSIGHT_STATE_UNSETTLED) ||
                (!update_active_succ && 
                 powerinsight_check_entry_state(thread, POWERINSIGHT_STATE_ALIVE)) ||
                thread->time == 0) {
                continue;
            }
            
            if (stats->count < MAX_STAT_ENTRIES) {
                stats->entries[stats->count].uid = entry->uid;
                stats->entries[stats->count].pid = thread->pid;
                stats->entries[stats->count].time = powerinsight_cputime_to_msecs(thread->time);
                #ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
                stats->entries[stats->count].power = thread->power;
                #endif
                stats->entries[stats->count].cmdline = CMDLINE_THREAD;
                powerinsight_copy_name(stats->entries[stats->count].name, thread->name, NAME_LEN - 1);
                stats->count++;
            }
        }
    }
    
    powerinsight_info("Total collected stats: %d entries", stats->count);
    mutex_unlock(&powerinsight_proc_lock);
    
    return stats->count > 0 ? 0 : -ENODATA;
}

/*
 * if there are dead processes in the list,
 * we should clear these dead processes
 * in case of pid reused
 */
static int powerinsight_get_proc_cputime(void __user *argp)
{
	int ret = 0;
	struct powerinsight_cpu_stats *stats;
	struct ioctl_memfd_request req;
	if (!atomic_read(&proc_cputime_enable)) {
		powerinsight_info("IOC_PROC_CPUTIME_REQUEST denied: stats disabled");
		return -EPERM;
	}

	// 从用户空间获取结构体
	if (copy_from_user(&req, argp, sizeof(req))) {
		powerinsight_err("Failed to copy request from user space");
		return -EFAULT;
	}

	// 获取CPU统计信息
	stats = kmalloc(sizeof(struct powerinsight_cpu_stats), GFP_KERNEL);
	if (!stats) {
		powerinsight_err("Failed to allocate memory for CPU stats");
		return -ENOMEM;
	}
	memset(stats, 0, sizeof(struct powerinsight_cpu_stats));
	ret = powerinsight_get_stats(stats);
	if (ret < 0) {
		powerinsight_err("Failed to get CPU stats: %d", ret);
		kfree(stats);
		return ret;
	}

	// 写回用户空间指针
	if (copy_to_user(req.user_ptr, stats, sizeof(struct powerinsight_cpu_stats))) {
		powerinsight_err("Failed to copy stats to user space");
		ret = -EFAULT;
	} else {
		ret = stats->count;
	}
	kfree(stats);
	return ret;
}

static void powerinsight_proc_cputime_reset(void)
{
	int i;
	unsigned long bkt, tbkt;
	struct powerinsight_proc_entry *entry = NULL;
	struct powerinsight_thread_entry *thread = NULL;
	struct hlist_node *tmp = NULL, *ttmp = NULL;

	mutex_lock(&powerinsight_proc_lock);
	powerinsight_hash_for_each_safe(proc_hash_table, bkt, tmp, entry, hash) {
		if (powerinsight_is_proc_entry_decomposed(entry)) {
			powerinsight_dynamic_hash_for_each_safe(entry->threads, tbkt, ttmp, thread, hash)
				powerinsight_free_thread_entry(thread);
		}
		powerinsight_free_proc_entry(entry);
	}
	for (i = 0; i < MAX_PROC_AVAIL_COUNT; i++) {
		if (proc_entry_avail_pool[i])
			kfree(proc_entry_avail_pool[i]);
	}
	for (i = 0; i < MAX_THREAD_AVAIL_COUNT; i++) {
		if (thread_entry_avail_pool[i])
			kfree(thread_entry_avail_pool[i]);
	}
	atomic_set(&dead_count, 0);
	atomic_set(&active_count, 0);
	atomic_set(&proc_cputime_enable, 0);
	powerinsight_hash_init(proc_hash_table);
	powerinsightd_tgid = -1;
	proc_entry_avail_cnt = 0;
	thread_entry_avail_cnt = 0;
	memset(proc_entry_avail_pool, 0, sizeof(proc_entry_avail_pool));
	memset(thread_entry_avail_pool, 0, sizeof(thread_entry_avail_pool));
	mutex_unlock(&powerinsight_proc_lock);
	powerinsight_info("cpu process stats reset");
}

static bool is_same_process(const struct powerinsight_proc_entry *entry, struct task_struct *task)
{
	uid_t uid;

	if (!powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_DEAD))
		return true;

	// pid has been reused by another task
	if (unlikely(entry->tgid == task->pid) || powerinsight_is_task_alive(task->group_leader))
		return false;

	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	if (uid != entry->uid)
		return false;

	return true;
}

static void powerinsight_update_proc_dead(struct powerinsight_proc_entry *entry, struct task_struct *task, powerinsight_cputime_t time)
{
	if (!is_same_process(entry, task))
		return;

	entry->dead_time += time;
#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
	if (task->cpu_power != ULLONG_MAX)
		entry->dead_power += task->cpu_power;
#endif

	// process has died
	if ((entry->tgid == task->pid) && !powerinsight_check_entry_state(entry, POWERINSIGHT_STATE_DEAD)) {
		entry->state = POWERINSIGHT_STATE_DEAD;
		atomic_dec(&active_count);
		atomic_inc(&dead_count);
	}
}

static void powerinsight_update_thread_dead(struct powerinsight_thread_entry *thread, struct task_struct *task, powerinsight_cputime_t time)
{
	if (powerinsight_check_entry_state(thread, POWERINSIGHT_STATE_DEAD))
		return;

	thread->time = time;
#ifdef CONFIG_POWERINSIGHT_TASK_CPU_POWER
	if (task->cpu_power != ULLONG_MAX)
		thread->power = task->cpu_power;
#endif
	thread->state = POWERINSIGHT_STATE_DEAD;
	atomic_dec(&active_count);
	atomic_inc(&dead_count);
}

static int powerinsight_process_notifier(struct notifier_block __always_unused *self,
	unsigned long __always_unused cmd, void *v)
{
	struct task_struct *task = v;
	pid_t tgid;
	struct powerinsight_proc_entry *entry = NULL;
	struct powerinsight_thread_entry *thread = NULL;
	char cmdline[CMDLINE_LEN] = {0};
	powerinsight_cputime_t utime, stime, time;
	bool group_dying = false, decomposed = false;

	if (task == NULL || !atomic_read(&proc_cputime_enable))
		return NOTIFY_OK;

	group_dying = powerinsight_thread_group_dying(task);
	tgid = powerinsight_get_task_normalized_tgid(task);
	if (!powerinsight_is_kthread(task) && group_dying) {
		if (powerinsight_get_cmdline(task, cmdline, CMDLINE_LEN) < 0)
			cmdline[0] = '\0';
	}

	// powerinsight_info("Processing process notifier for tgid: %d, pid: %d", tgid, task->pid);
	mutex_lock(&powerinsight_proc_lock);
	if (group_dying || powerinsight_is_task_alive(task->group_leader)) {
		entry = powerinsight_find_or_register_proc_entry_locked(tgid, task);
	} else {
		entry = powerinsight_find_proc_entry_locked(tgid);
	}
	if (entry == NULL)
		goto exit;
	if (strlen(cmdline) > 0) {
		powerinsight_copy_name(entry->name, cmdline, NAME_LEN - 1);
		entry->cmdline = true;
	}
	decomposed = powerinsight_is_proc_entry_decomposed(entry);
	task_cputime_adjusted(task, &utime, &stime);
	time = utime + stime;
	powerinsight_update_proc_dead(entry, task, time);
	thread = powerinsight_find_or_register_thread_entry_locked(task->pid, task->comm, entry);
	if (thread)
		powerinsight_update_thread_dead(thread, task, time);

	// powerinsight_info("Updated stats for tgid: %d, pid: %d, time: %llu", tgid, task->pid, time);

exit:
	mutex_unlock(&powerinsight_proc_lock);
	if (group_dying && !powerinsight_is_kernel_tgid(tgid)) {
		if (unlikely(decomposed))
			powerinsight_remove_proc_decompose(tgid);
		if (unlikely(tgid == powerinsightd_tgid))
			powerinsight_proc_cputime_reset();
	}

	return NOTIFY_OK;
}

static struct notifier_block process_notifier_block = {
	.notifier_call	= powerinsight_process_notifier,
	.priority = INT_MAX,
};


static int powerinsight_proc_cputime_enable(void __user *argp)
{
	int ret;
	int enable = 0;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
        powerinsightd_tgid = current->tgid;
        atomic_set(&proc_cputime_enable, enable ? 1 : 0);
        
        // 添加详细日志
        powerinsight_info("CPU stats %s by process %d (%s)",
                         enable ? "enabled" : "disabled",
                         current->tgid, current->comm);
        powerinsight_info("Current enable state: %d", 
                         atomic_read(&proc_cputime_enable));
    } else {
        powerinsight_err("Failed to get enable value: %d", ret);
    }


	return ret;
}

static int powerinsight_get_task_cpupower_enable(void __user *argp)
{
	int enable;

	enable = !!atomic_read(&task_power_enable);
	if (copy_to_user(argp, &enable, sizeof(int)))
		return -EFAULT;

	return 0;
}

void powerinsight_proc_cputime_init(void)
{
	int ret;

	powerinsight_proc_cputime_reset();
	ret = profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
	if (ret)
		powerinsight_err("Failed to register task exit notifier");
}

void powerinsight_proc_cputime_exit(void)
{
	profile_event_unregister(PROFILE_TASK_EXIT, &process_notifier_block);
	powerinsight_proc_cputime_reset();
}
