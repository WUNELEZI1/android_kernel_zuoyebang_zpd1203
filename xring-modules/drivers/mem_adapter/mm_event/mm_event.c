// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>

#include <trace/hooks/mm.h>
#include <trace/hooks/vmscan.h>

#include "mm_event.h"
#include "soc/xring/xring_mem_adapter.h"

#ifdef CONFIG_XRING_MM_EVENT
static struct work_struct report_work;
static struct timer_list report_timer;

#define MM_EVENT (912888001)
#define REPORT_PER_MIN  (HZ * 60)
#define REPORT_PER_HOUR (HZ * 60 * 60)
#define REPORT_PER_DAY  (HZ * 60 * 60 * 24)

static bool config_enabled;
static int config_level;

static ssize_t enabled_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", config_enabled);
}

static ssize_t enabled_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret)
		return ret;

	if (config_enabled != (!!val)) {
		config_enabled = !!val;

		if (config_enabled)
			mod_timer(&report_timer, jiffies);
		else
			del_timer_sync(&report_timer);
	}

	return count;
}

static ssize_t level_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", config_level);
}

static ssize_t level_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count)
{
	int val, ret;

	ret = kstrtoint(buf, 10, &val);
	if (ret)
		return ret;

	if (config_level != val) {
		config_level = val;

		if (config_enabled) {
			del_timer_sync(&report_timer);
			mod_timer(&report_timer, jiffies);
		}
	}

	return count;
}

static struct kobj_attribute enabled_attr =
	__ATTR(enabled, 0644, enabled_show,  enabled_store);

static struct kobj_attribute level_attr =
	__ATTR(level, 0644, level_show,  level_store);

static struct attribute *mm_event_attr[] = {
	&enabled_attr.attr,
	&level_attr.attr,
	NULL,
};

static const struct attribute_group mm_event_attr_group = {
	.attrs = mm_event_attr,
};

static unsigned long record_kswapd_cnt;
static unsigned long record_direct_reclaim_cnt;
static unsigned long record_slowpath_cost_jiffies;
static unsigned long mm_event_arr[NR_MM_EVENT] = {0};

void xring_memory_event_report(unsigned int code)
{
	mm_event_arr[code]++;
}
EXPORT_SYMBOL_GPL(xring_memory_event_report);

static void record_kswapd_done(void *data, int node_id,
	unsigned int highest_zone_idx, unsigned int alloc_order,
	unsigned int reclaim_order)
{
	record_kswapd_cnt++;
}

static void record_direct_reclaim(void *data, unsigned int order)
{
	record_direct_reclaim_cnt++;
}

static void record_slowpath(void *data, gfp_t gfp_mask,
	unsigned int order, unsigned long alloc_start)
{
	record_slowpath_cost_jiffies += (jiffies - alloc_start);
}

static void mm_unregister_hooks(void)
{
	unregister_trace_android_vh_vmscan_kswapd_done(
		record_kswapd_done, NULL);
	unregister_trace_android_vh_alloc_pages_slowpath(
		record_slowpath, NULL);
	unregister_trace_android_vh_mm_direct_reclaim_enter(
		record_direct_reclaim, NULL);
}

static int mm_register_hooks(void)
{
	int ret;

	ret = register_trace_android_vh_vmscan_kswapd_done(
		record_kswapd_done, NULL);
	if (ret)
		goto err;

	ret = register_trace_android_vh_alloc_pages_slowpath(
			record_slowpath, NULL);
	if (ret)
		goto err;

	ret = register_trace_android_vh_mm_direct_reclaim_enter(
			record_direct_reclaim, NULL);
	if (ret)
		goto err;

	return 0;

err:
	pr_err("register hook failed!");
	mm_unregister_hooks();
	return -1;
}

static void do_report(struct work_struct *work)
{
	struct misight_mievent *event;

	event = cdev_tevent_alloc(MM_EVENT);
	if (!event) {
		pr_err("misight event is error\n");
		return;
	}
	cdev_tevent_add_int(event, "free_page", xr_get_free_page());
	cdev_tevent_add_int(event, "file_page", xr_get_file_page());
	cdev_tevent_add_int(event, "anon_page", xr_get_anon_page());
	cdev_tevent_add_int(event, "zram_page", xr_get_zram_page());
	cdev_tevent_add_int(event, "kswapd_cnt", record_kswapd_cnt);
	cdev_tevent_add_int(event, "direct_reclaim_cnt", record_kswapd_cnt);
	cdev_tevent_add_int(event, "slowpath_cost_jiffies", record_slowpath_cost_jiffies);

	for (int i = 0; i < NR_MM_EVENT; ++i) {
		cdev_tevent_add_int(event, "record_event", mm_event_arr[i]);
		mm_event_arr[i] = 0;
	}

	cdev_tevent_write(event);
	cdev_tevent_destroy(event);

	record_kswapd_cnt = 0;
	record_direct_reclaim_cnt = 0;
	record_slowpath_cost_jiffies = 0;
}

void timer_callback(struct timer_list *t)
{
	if (config_level == 2)
		mod_timer(&report_timer, jiffies + REPORT_PER_MIN);
	else if (config_level == 1)
		mod_timer(&report_timer, jiffies + REPORT_PER_HOUR);
	else
		mod_timer(&report_timer, jiffies + REPORT_PER_DAY);

	schedule_work(&report_work);
}

int mm_event_init(void)
{
	struct kobject *mm_event_kobj;
	int err;

	mm_event_kobj = kobject_create_and_add("mm_event", kernel_kobj);
	if (unlikely(!mm_event_kobj)) {
		pr_err("failed to create mm_event kobject\n");
		return -ENOMEM;
	}

	err = sysfs_create_group(mm_event_kobj, &mm_event_attr_group);
	if (err) {
		pr_err("failed to register mm_event group\n");
		kobject_put(mm_event_kobj);
		return -ENOMEM;
	}

	mm_register_hooks();

	INIT_WORK(&report_work, do_report);
	timer_setup(&report_timer, timer_callback, 0);

	return 0;
}

void mm_event_exit(void)
{
	mm_unregister_hooks();

	del_timer_sync(&report_timer);
	cancel_work_sync(&report_work);
}

#else
int mm_event_init(void)
{
	return 0;
};

void mm_event_exit(void)
{
};

void xring_memory_event_report(unsigned int code)
{
};
EXPORT_SYMBOL_GPL(xring_memory_event_report);

#endif /* CONFIG_XRING_MM_EVENT */
