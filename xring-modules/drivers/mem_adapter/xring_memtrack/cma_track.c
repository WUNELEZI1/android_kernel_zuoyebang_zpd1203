// SPDX-License-Identifier: GPL-2.0
#include <linux/mm.h>
#include <linux/cma.h>
#include <linux/stackdepot.h>
#include <linux/stacktrace.h>
#include <linux/list_sort.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <trace/events/cma.h>
#include "../mm/cma.h"
#include "../xring_mem_adapter.h"

#define CMA_IGNORED_ENTRY_NUM		3
#define CMA_STACK_ENTRY_NUM			11
#define CMA_SHOW_PAGE_ONCE_NUM		20
#define CMA_SHOW_FAIL_PAGE_NUM		3

struct cma_stackinfo {
	char name[CMA_MAX_NAME];
	unsigned long start; // pfn start
	unsigned long end; // pfn end
	depot_stack_handle_t handle;
	struct list_head list;
};

static int cma_show_backtrace;
static int cma_warn_limit_permillage = 850;
static int cma_region_count;
static int cma_target_index;
static char *cma_target = "linux,cma";
static struct cma *cma_regions[MAX_CMA_AREAS];
static struct dentry *cma_debugfs_root;
static LIST_HEAD(cmainfolist);
static spinlock_t cma_lock;

static int get_cma_regions(struct cma *cma, void *data)
{
	if (strcmp(cma_target, cma->name) == 0)
		cma_target_index = cma_region_count;
	cma_regions[cma_region_count++] = cma;
	return 0;
}

static int get_target_index(char *name)
{
	int i;

	for (i = 0; i < cma_region_count; i++)
		if (!strcmp(cma_regions[i]->name, name))
			return i;
	return i;
}

static depot_stack_handle_t get_track_stack_handle(void)
{
	depot_stack_handle_t handle;
	unsigned long entries[CMA_STACK_ENTRY_NUM];
	unsigned int nr_entries;
	int i;

	nr_entries = stack_trace_save(entries, ARRAY_SIZE(entries), CMA_IGNORED_ENTRY_NUM);
	if (unlikely(cma_show_backtrace)) {
		for (i = 0; i < nr_entries; i++)
			xrmem_info("        %pS\n", (void *)entries[i]);
	}
	handle = stack_depot_save(entries, nr_entries, GFP_NOWAIT);

	return handle;
}

static void show_info(struct cma_stackinfo *c)
{
	xrmem_info("%s [%lu-%lu] count:%lu\n", c->name, c->start, c->end, c->end - c->start + 1);
	if (c->handle) {
		unsigned long *entries;
		unsigned int nr_entries, i;

		nr_entries = stack_depot_fetch(c->handle, &entries);
		for (i = 0; i < nr_entries; i++)
			xrmem_info("        %pS\n", (void *)entries[i]);
	}
}

int cmalist_compare(
	void *priv,
	const struct list_head *a,
	const struct list_head *b)
{
	struct cma_stackinfo *ra;
	struct cma_stackinfo *rb;

	ra = container_of(a, struct cma_stackinfo, list);
	rb = container_of(b, struct cma_stackinfo, list);

	return ra->start - rb->start;
}

static void show_brief(char *name, unsigned long pfn, unsigned long count)
{
	int cma_index;
	unsigned long used, bitmap_maxno;

	cma_index = get_target_index(name);
	if (cma_index == cma_region_count)
		return;

	bitmap_maxno = cma_bitmap_maxno(cma_regions[cma_index]);
	used = bitmap_weight(cma_regions[cma_index]->bitmap, (int)bitmap_maxno);

	xrmem_info("%s valid_pfn=[%lu-%lu] count:%lu used:%lu req:%lu lasttry-pfn:0x%lx\n",
		name, cma_regions[cma_index]->base_pfn,
		cma_regions[cma_index]->base_pfn + cma_regions[cma_index]->count - 1,
		cma_regions[cma_index]->count, used, count, pfn);
}

void show_cma_pages(char *name, unsigned long pfn, unsigned long count)
{
	int i, j;
	unsigned long target_pfn, nr_pages;
	struct page *page;

	if (count == 0)
		return;

	if (pfn == -1)
		return;

	for (i = 0; i < count;) {
		target_pfn = pfn + i;
		page = pfn_to_page(target_pfn);
		if (PageBuddy(page)) {
			nr_pages = 1 << page_private(page);
			xrmem_info("FreePage:[0x%lx-0x%lx]\n", target_pfn, target_pfn + nr_pages - 1);
			i += nr_pages;
			continue;
		}
		break;
	}
	/* i is the index of fail page, dump_page(fail/next/nextnext) */
	for (j = i; j >= 0 && j < (i + CMA_SHOW_FAIL_PAGE_NUM) && j < count; j++) {
		target_pfn = pfn + j;
		page = pfn_to_page(target_pfn);
		dump_page(page, "cma_alloc fail");
	}
}

static void show_stackinfos(char *name, unsigned long pfn, unsigned long count)
{
	struct cma_stackinfo *c;
	unsigned long flags;

	if (!name)
		return;

	spin_lock_irqsave(&cma_lock, flags);
	list_sort(NULL, &cmainfolist, cmalist_compare);
	spin_unlock_irqrestore(&cma_lock, flags);

	list_for_each_entry(c, &cmainfolist, list)
		if (!strcmp(name, c->name))
			show_info(c);

	show_cma_pages(name, pfn, count);
}

static void show_stackinfos_ratelimit(char *name, unsigned long pfn, unsigned long count)
{
	static DEFINE_RATELIMIT_STATE(ratelimit, 30 * HZ, 1);

	show_brief(name, pfn, count);
	if (!__ratelimit(&ratelimit))
		return;

	show_stackinfos(name, pfn, count);
}

static void cma_show_when_exceed(const char *name)
{
	unsigned long used, bitmap_maxno;
	static bool print_it = true;

	/* only show cma_target */
	if (strcmp(name, cma_target) == 0) {
		/* update cma_target_index when user change cma_target*/
		if (strcmp(cma_regions[cma_target_index]->name, cma_target)) {
			/* each type only print once */
			print_it = true;
			cma_target_index = get_target_index(cma_target);
			if (cma_target_index == cma_region_count)
				return;
		}
		/* each type only print once */
		if (!print_it)
			return;
		bitmap_maxno = cma_bitmap_maxno(cma_regions[cma_target_index]);
		used = bitmap_weight(cma_regions[cma_target_index]->bitmap, (int)bitmap_maxno);
		if (used > cma_warn_limit_permillage * (bitmap_maxno >> 10)) {
			xrmem_info("cma warn %s has been used exceed %d/1024\n",
				cma_target, cma_warn_limit_permillage);
			show_stackinfos_ratelimit(cma_target, 0, 0);
			print_it = false;
		}
	}
}

static void add_cma_stackinfo(const char *name, unsigned long pfn, unsigned long count)
{
	struct cma_stackinfo *cma_info = NULL;
	unsigned long flags;

	cma_info = kzalloc(sizeof(*cma_info), GFP_ATOMIC);
	memcpy(cma_info->name, name, CMA_MAX_NAME);
	cma_info->start = pfn;
	cma_info->end = pfn + count - 1;
	cma_info->handle = get_track_stack_handle();
	INIT_LIST_HEAD(&cma_info->list);
	spin_lock_irqsave(&cma_lock, flags);
	list_add(&cma_info->list, &cmainfolist);
	spin_unlock_irqrestore(&cma_lock, flags);
	if (unlikely(cma_show_backtrace))
		xrmem_info("%s [%lu-%lu] add\n", name, cma_info->start, cma_info->end);
	cma_show_when_exceed(name);
}

static void __check_and_del(const char *name, unsigned long start, unsigned long end)
{
	struct cma_stackinfo *c, *next;

	if (unlikely(cma_show_backtrace)) {
		get_track_stack_handle();
		xrmem_info("%s [%lu-%lu] del\n", name, start, end);
	}
	list_for_each_entry_safe(c, next, &cmainfolist, list) {
		if (strcmp(name, c->name))
			continue;
		if (start == c->start || end == c->end) {
			list_del(&c->list);
			kfree(c);
			return;
		}
		if (c->start <= end && c->end >= start) {
			if (unlikely(cma_show_backtrace))
				xrmem_info("[%lu-%lu] overlaps existed [%lu, %lu]\n", start, end, c->start, c->end);
			list_del(&c->list);
			kfree(c);
			return;
		}
	}
}

static void check_and_del(const char *name, unsigned long start, unsigned long end)
{
	unsigned long flags;

	spin_lock_irqsave(&cma_lock, flags);
	__check_and_del(name, start, end);
	spin_unlock_irqrestore(&cma_lock, flags);
}

static void vh_cma_alloc(void *data, const char *name, unsigned long pfn, const struct page *page,
		unsigned long count, unsigned int align, int errorno)
{
	if (likely(!errorno))
		add_cma_stackinfo(name, pfn, count);
	else {
		xrmem_info("detect cma_alloc fail, ret=%d\n", errorno);
		show_stackinfos_ratelimit((char *)name, pfn, count);
	}
}

static void vh_cma_release(void *data, const char *name, unsigned long pfn, const struct page *page,
		unsigned long count)
{
	check_and_del(name, pfn, pfn + count - 1);
}

static int cma_show_get(void *data, u64 *val)
{
	char *name = data;

	show_brief(name, 0, 0);
	show_stackinfos(name, 0, 0);
	*val = 0;

	return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(cma_show_fops, cma_show_get, NULL, "%llu\n");

static void xring_cma_debugfs_add_one(void *name, struct dentry *root_dentry)
{
	struct dentry *tmp;

	tmp = debugfs_create_dir(name, root_dentry);
	debugfs_create_file("show", 0444, tmp, name, &cma_show_fops);
}

static void cma_debugfs_init(void)
{
	int i;

	cma_debugfs_root = debugfs_create_dir("xring_cma", NULL);

	for (i = 0; i < cma_region_count; i++)
		xring_cma_debugfs_add_one(&cma_regions[i]->name, cma_debugfs_root);
}

static void cma_debugfs_exit(void)
{
	debugfs_remove_recursive(cma_debugfs_root);
}

int cma_track_init(void)
{
	int ret = 0;

	spin_lock_init(&cma_lock);
	cma_for_each_area(get_cma_regions, NULL);
	ret |= register_trace_cma_alloc_finish(vh_cma_alloc, NULL);
	ret |= register_trace_cma_release(vh_cma_release, NULL);
	cma_debugfs_init();
	return ret;
}

void cma_track_exit(void)
{
	cma_debugfs_exit();
	unregister_trace_cma_release(vh_cma_release, NULL);
	unregister_trace_cma_alloc_finish(vh_cma_alloc, NULL);
}

module_param(cma_show_backtrace, int, 0644);
module_param(cma_warn_limit_permillage, int, 0644);
module_param(cma_target, charp, 0644);
