
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/cgroup.h>
#include <linux/mmzone.h>
#include <linux/jiffies.h>
#include <linux/sched/cputime.h>
#include <trace/events/sched.h>
#include <trace/events/cma.h>
#include <trace/events/vmscan.h>
#include <trace/events/kmem.h>
#include <trace/hooks/mm.h>
#include <trace/hooks/sched.h>
#include "../../../kernel/sched/sched.h"
#include "internal.h"
#include "pub/kprobe.h"

extern int hook_tracepoint(const char *name, void *probe, void *data);
extern int unhook_tracepoint(const char *name, void *probe, void *data);
////#include "../../../../../../kernel-6.6/mm/internal.h"

#define mem_warn(fmt, ...) \
		pr_info("<xiaomi_mem_monitor><%s> "fmt, __func__, ##__VA_ARGS__)

static atomic_t css_id_top;
static atomic_t css_id_fg;
static atomic_t css_id_bg;
static int mem_enable;

static atomic_t kswapd_wakeup_cnt;
static atomic_t slowpath_cnt;
static atomic_t kcompactd_wakeup_cnt;
static atomic_t page_alloc_cnt;

static char *grp_name[GRP_TYPES] = {"vip", "rt", "oth",};

enum mem_type {
#ifdef TRACK_PAGE_ALLOC	
	PAGE_ALLOC = 0,
#endif	
	CMA_ALLOC,
	DIRECT_RECLAIM,
	SLOWPATH_ALLOC,
	MEM_TYPE_MAX
};

struct mem_config {
	char name[16];
	u32 min;    /*<1ms*/
	u32 low;    /* ms */
	u32 normal; /* ms */
	u32 high;   /* ms */
};

struct mem_config mem_config[MEM_TYPE_MAX] = {
#ifdef TRACK_PAGE_ALLOC		
	{.name = "page_alloc",	.min=1,	  .low = 8,	.normal = 15,	.high = 30},
#endif	
	{.name = "cma_alloc",	.min=1,   .low = 8,	.normal = 15,	.high = 30},
	{.name = "direct_reclaim",	.min=1,   .low = 8,	.normal = 15,	.high = 30},
	{.name = "slowpath_alloc",	.min=1,   .low = 8,	.normal = 15,	.high = 30},
};
/* Stats counts for low,middle,high,fatal */
struct mem_entry {
	atomic_long_t min_counts;
	atomic_long_t l_counts;
	atomic_long_t m_counts;
	atomic_long_t h_counts;
	atomic_long_t f_counts;
	atomic_long_t all_costs_us;
	atomic_long_t all_costs_ms;
	atomic_long_t max_latency_ms;
};

struct mem_entry mem_entry[GRP_TYPES][MEM_TYPE_MAX];
extern int get_task_grp_idx(struct task_struct* task);

static void clear_stats(void)
{
	int i, j;

	for (i = 0; i < GRP_TYPES; i++) {
		for (j = 0; j < MEM_TYPE_MAX; j++) {
			atomic64_set(&mem_entry[i][j].min_counts, 0);
			atomic64_set(&mem_entry[i][j].l_counts, 0);
			atomic64_set(&mem_entry[i][j].m_counts, 0);
 			atomic64_set(&mem_entry[i][j].h_counts, 0);
 			atomic64_set(&mem_entry[i][j].f_counts, 0);
 			atomic64_set(&mem_entry[i][j].all_costs_us, 0);
 			atomic64_set(&mem_entry[i][j].all_costs_ms, 0);
			atomic64_set(&mem_entry[i][j].max_latency_ms, 0);
		}
	}

	atomic_set(&kswapd_wakeup_cnt, 0);
	atomic_set(&slowpath_cnt, 0);
	atomic_set(&kcompactd_wakeup_cnt, 0);
	atomic_set(&page_alloc_cnt, 0);
}

#if 1
static void update_mem_entry(int grp_type, int mem_type, u64 delay)
{
	////u64 update_delay = (delay >> 20); /* ns -> ms */
	u64 update_delay_us = (delay / 1000); /* ns -> ms */
	u64 update_delay_ms = (delay / 1000000); /* ns -> ms */

	if (update_delay_ms >= mem_config[mem_type].high) {
		atomic_long_inc(&mem_entry[grp_type][mem_type].f_counts);
	}else if (update_delay_ms >= mem_config[mem_type].normal && update_delay_ms < mem_config[mem_type].high) {
		atomic_long_inc(&mem_entry[grp_type][mem_type].h_counts);
	}else if (update_delay_ms >= mem_config[mem_type].low && update_delay_ms < mem_config[mem_type].normal) {
		atomic_long_inc(&mem_entry[grp_type][mem_type].m_counts);
	}else if (update_delay_ms >= 1 && update_delay_ms < mem_config[mem_type].low) {
		atomic_long_inc(&mem_entry[grp_type][mem_type].l_counts);
	} else{
		atomic_long_inc(&mem_entry[grp_type][mem_type].min_counts);
	}

	if (update_delay_ms >= 1/*mem_config[mem_type].low*/)
	{
        atomic_long_add(update_delay_ms, &mem_entry[grp_type][mem_type].all_costs_ms);
		if (update_delay_ms > atomic_long_read(&mem_entry[grp_type][mem_type].max_latency_ms))
		{
			atomic_long_set(&mem_entry[grp_type][mem_type].max_latency_ms, update_delay_ms);
		}		
	}
	else
	{
        atomic_long_add(update_delay_us, &mem_entry[grp_type][mem_type].all_costs_us);
	}
}
#endif

static void alloc_pages_slowpath_handler(void *unused, gfp_t *gfp_mask, unsigned int order, unsigned long alloc_start,
		u64 stime, unsigned long did_some_progress, unsigned long pages_reclaimed, int retry_loop_count)
{
	unsigned long delta_jiffies = jiffies - alloc_start;
	int grp_type = get_task_grp_idx(current);
	u64 delta_ns = (delta_jiffies *1000000000)/HZ;
	atomic_inc(&slowpath_cnt);
	update_mem_entry(grp_type, SLOWPATH_ALLOC, delta_ns);	
}

static void vmscan_wakeup_kswapd_handler(void *unused, int nid, int zid, int order, gfp_t gfp_flags)
{
	atomic_inc(&kswapd_wakeup_cnt);
}

static void mm_compaction_wakeup_kcompacted_handler(void *unused, int nid, int zid, enum zone_type highest_zoneidx)
{
    atomic_inc(&kcompactd_wakeup_cnt);
}

static void trace_mm_page_alloc_handler(void *unused, struct page *page, unsigned int order, gfp_t gfp_flags, int migratetype)
{
	atomic_inc(&page_alloc_cnt);
}

static void mem_enable_change(int enable)
{
	int ret;
	if (enable) {
		ret = register_trace_android_vh_alloc_pages_slowpath_end(alloc_pages_slowpath_handler, NULL);
		
		ret = hook_tracepoint("mm_vmscan_wakeup_kswapd", vmscan_wakeup_kswapd_handler, NULL);
		ret = hook_tracepoint("mm_compaction_wakeup_kcompactd", mm_compaction_wakeup_kcompacted_handler, NULL);
		ret = hook_tracepoint("mm_page_alloc", trace_mm_page_alloc_handler, NULL);

		mem_warn("huqinghe_mem register hook handlers, ret is %d\n", ret);
	} else {
		ret = unregister_trace_android_vh_alloc_pages_slowpath_end(alloc_pages_slowpath_handler, NULL);

		ret = unhook_tracepoint("mm_vmscan_wakeup_kswapd", vmscan_wakeup_kswapd_handler, NULL);
		ret = unhook_tracepoint("mm_compaction_wakeup_kcompactd", mm_compaction_wakeup_kcompacted_handler, NULL);
		ret = unhook_tracepoint("mm_page_alloc", trace_mm_page_alloc_handler, NULL);

		mem_warn("huqinghe_mem unregister hook handlers, ret is %d\n", ret);
	}
}

static ssize_t mem_common_write(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int *pval = (int *)pde_data(file_inode(file));
	char kbuf[4] = {0};
	int err;
	if (pval == NULL)
		goto out;
	memset(kbuf, 0, sizeof(kbuf));
	if (count > sizeof(kbuf) - 1)
		count = sizeof(kbuf) - 1;
	if (copy_from_user(kbuf, buf, count)) {
		mem_warn("failed to do copy_from_user\n");
		return -EFAULT;
	}
	err = kstrtoint(strstrip(kbuf), 0, pval);
	if (err < 0) {
		mem_warn("failed to do kstrtoint\n");
		return -EFAULT;
	}
	if (pval == &mem_enable)
		mem_enable_change(*pval);
out:
	return count;
}

static int mem_common_show(struct seq_file *m, void *v)
{
	u64 min_counts, l_counts, m_counts, h_counts, f_counts, all_costs_ms, all_costs_us, direct_reclaim_cnt;
	u64 total_ms, avg_ms, total_us, avg_us, max_ms;
	int i, j;
	if (m->private == &mem_enable) {
		seq_printf(m, "%d\n", *(int*) m->private);
		goto out;
	}
	for (j = 0; j < GRP_TYPES; j++)
	/* m->private is NULL */
	for (i = 0; i < MEM_TYPE_MAX; ++i) {
		struct mem_entry entry = mem_entry[j][i];
		char str[32];
		min_counts = atomic_long_read(&entry.min_counts);
		l_counts  = atomic_long_read(&entry.l_counts);
		m_counts  = atomic_long_read(&entry.m_counts);
		h_counts  = atomic_long_read(&entry.h_counts);
		f_counts  = atomic_long_read(&entry.f_counts);
		max_ms = atomic_long_read(&entry.max_latency_ms);
		all_costs_ms = atomic_long_read(&entry.all_costs_ms);
		total_ms = l_counts + m_counts + h_counts + f_counts;
		avg_ms = total_ms ? (all_costs_ms / total_ms) : 0;
		all_costs_us = atomic_long_read(&entry.all_costs_us);
		total_us = min_counts + l_counts + m_counts + h_counts + f_counts;
		if (i == DIRECT_RECLAIM)
		{
		    direct_reclaim_cnt = total_us;
	    }
		avg_us = total_us ? (all_costs_us / total_us) : 0;

		snprintf(str, sizeof(str), "%s_%s_lat_min_cnt", grp_name[j], mem_config[i].name);
		seq_printf(m, "%-32s: %llu\n", str, min_counts);
		snprintf(str, sizeof(str), "%s_%s_lat_l_cnt", grp_name[j], mem_config[i].name);
		seq_printf(m, "%-32s: %llu\n", str, l_counts);
		snprintf(str, sizeof(str), "%s_%s_lat_m_cnt", grp_name[j], mem_config[i].name);
		seq_printf(m, "%-32s: %llu\n", str, m_counts);
		snprintf(str, sizeof(str), "%s_%s_lat_h_cnt", grp_name[j], mem_config[i].name);
		seq_printf(m, "%-32s: %llu\n", str, h_counts);
		snprintf(str, sizeof(str), "%s_%s_lat_f_cnt", grp_name[j], mem_config[i].name);
		seq_printf(m, "%-32s: %llu\n", str, f_counts);
		snprintf(str, sizeof(str), "%s_%s_lat_avg_%s", grp_name[j], mem_config[i].name, "ms");
		seq_printf(m, "%-32s: %llu\n", str, avg_ms);
		snprintf(str, sizeof(str), "%s_%s_lat_avg_%s", grp_name[j], mem_config[i].name, "us");
		seq_printf(m, "%-32s: %llu\n", str, avg_us);
		snprintf(str, sizeof(str), "%s_%s_lat_max_%s", grp_name[j], mem_config[i].name, "ms");
		seq_printf(m, "%-32s: %llu\n", str, max_ms);		
	}
	
	seq_printf(m, "%-32s: %d\n", "kswapd_cnt", atomic_read(&kswapd_wakeup_cnt));
	seq_printf(m, "%-32s: %d\n", "kcompactd_cnt", atomic_read(&kcompactd_wakeup_cnt));
	seq_printf(m, "%-32s: %llu\n", "direct_reclaim_cnt", direct_reclaim_cnt);
	seq_printf(m, "%-32s: %d\n", "slowpath_cnt", atomic_read(&slowpath_cnt));
	seq_printf(m, "%-32s: %d\n", "page_alloc_cnt", atomic_read(&page_alloc_cnt));
out:
	return 0;
}

static int mem_common_open(struct inode *inode, struct file *file)
{
	return single_open(file, mem_common_show, pde_data(inode));
}

static const struct proc_ops mem_common_proc_ops = {
	.proc_open		= mem_common_open,
	.proc_write		= mem_common_write,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release		= single_release,
};

/*******************************reset enable********************************/

static int reset_show(struct seq_file *m, void *ptr)
{
	seq_printf(m,"%s\n", "Data Reset");
	return 0;
}

static ssize_t reset_store(void *priv, const char __user *buf, size_t count)
{
	int val;
	if (kstrtoint_from_user(buf, count, 0, &val))
		return -EINVAL;

	if (val) 
	{
		clear_stats();
	}

	return count;
}
DEFINE_PROC_ATTRIBUTE_RW(reset);

/*******************************stack trace info********************************/

#ifdef CONFIG_FAIR_GROUP_SCHED
static void init_cgroup_id(void)
{
	struct cgroup_subsys_state *css = &root_task_group.css;
	struct cgroup_subsys_state *top_css = css;
	if (css->cgroup && (css->cgroup->level == 0)) {
		mem_warn("root group, css->id=%d (expected 1)\n", css->id);
	}
	rcu_read_lock();
	css_for_each_child(css, top_css) {
		mem_warn("traversal css, level is %d, name is %-12s, id is %d\n",
				css->cgroup->level, css->cgroup->kn->name,
				css->id);
		if (!strcmp(css->cgroup->kn->name, "top-app"))
			atomic_set(&css_id_top, css->id);
		else if (!strcmp(css->cgroup->kn->name, "foreground"))
			atomic_set(&css_id_fg, css->id);
		else if (!strcmp(css->cgroup->kn->name, "background"))
			atomic_set(&css_id_bg, css->id);
	}
	rcu_read_unlock();
	mem_warn("traversal css end, top is %d, fg is %d, bg is %d\n",
			atomic_read(&css_id_top), atomic_read(&css_id_fg),
			atomic_read(&css_id_bg));
}
#else
static inline void init_cgroup_id(void) {}
#endif

static struct proc_dir_entry *mem_dir;
static void init_procs(void)
{
	mem_dir = xiaomi_proc_mkdir(PROC_TRACE_MEM, NULL);
	if (!mem_dir) {
		printk("huqinghe failed to create dir: proc/mem_monitor\n");
		return;
	}

	proc_create_data("lat_info", S_IRUGO, mem_dir, &mem_common_proc_ops, NULL);
	proc_create_data("enable", S_IRUGO | S_IWUGO, mem_dir, &mem_common_proc_ops, &mem_enable);
	proc_create("reset", S_IRUGO | S_IWUGO, mem_dir, &reset_fops);
}

int xiaomi_mem_trace_init(void)
{
	init_cgroup_id();
	init_procs();
	mem_enable = 1;
	mem_enable_change(mem_enable);
	printk("huqinghe load sch monitor\n");

	return 0;
}

void xiaomi_mem_trace_exit(void)
{
	if (mem_dir) {
		remove_proc_entry("lat_info", mem_dir);
		remove_proc_entry("enable", mem_dir);
		remove_proc_entry("reset", mem_dir);
		remove_proc_subtree(PROC_TRACE_MEM, NULL);
		mem_dir = NULL;
	}
	
	mem_enable = 0;
	mem_enable_change(mem_enable);
	printk("huqinghe de-load sch monitor\n");
	return;
}

