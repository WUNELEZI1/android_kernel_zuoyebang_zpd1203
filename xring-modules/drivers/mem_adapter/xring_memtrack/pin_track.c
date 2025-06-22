// SPDX-License-Identifier: GPL-2.0-only
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/kprobes.h>
#include <linux/of.h>
#include "../xring_mem_adapter.h"

#define FOLL_PIN	(1 << 19)

static unsigned long last_print_time;
static unsigned long print_interval = 5 * HZ;

static inline void show_pinner(struct page *page, unsigned int flags, const char *reason)
{
	if (!(flags & FOLL_PIN))
		return;
	if ((strncmp(current->comm, "fio", sizeof("fio")) == 0) ||
		(strncmp(current->comm, "crash_dump64", sizeof("crash_dump64")) == 0) ||
		(strncmp(current->comm, "Signal Catcher", sizeof("Signal Catcher")) == 0) ||
		(strncmp(current->comm, "RenderThread", sizeof("RenderThread")) == 0))
		return;
	if (get_pageblock_migratetype(page) == MIGRATE_CMA) {
		if (time_after(jiffies, last_print_time + print_interval)) {
			xrmem_info("%s %s [0x%lx-0x%lx]\n", current->comm, reason,
				page_to_pfn(page), page_to_pfn(page) + folio_nr_pages(page_folio(page)) - 1);
			last_print_time = jiffies;
		}
	}
}

static int __kprobes folio_add_pin_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct folio *folio;
	struct page *page;

	folio = (struct folio *)regs->regs[0];
	page = folio_page(folio, 0);
	show_pinner(page, FOLL_PIN, "folio_add_pin");

	return 0;
}

static int __kprobes try_grab_page_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct page *page;
	unsigned int flags;

	page = (struct page *)regs->regs[0];
	flags = (unsigned int)regs->regs[1];
	show_pinner(page, flags, "try_grab_page");

	return 0;
}

static int __kprobes try_grab_folio_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct page *page;
	unsigned int flags;

	page = (struct page *)regs->regs[0];
	flags = (unsigned int)regs->regs[2];
	show_pinner(page, flags, "try_grab_folio");

	return 0;
}

static struct kprobe folio_add_pin_kp = {
	.symbol_name	= "folio_add_pin",
	.pre_handler = folio_add_pin_handler_pre,
};

static struct kprobe try_grab_page_kp = {
	.symbol_name	= "try_grab_page",
	.pre_handler = try_grab_page_handler_pre,
};

static struct kprobe try_grab_folio_kp = {
	.symbol_name	= "try_grab_folio",
	.pre_handler = try_grab_folio_handler_pre,
};

int pin_track_init(void)
{
	int ret;

	ret = register_kprobe(&folio_add_pin_kp);
	if (ret < 0) {
		pr_err("folio_add_pin_kp register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	xrmem_info("folio_add_pin_kp Planted kprobe at 0x%pK\n", folio_add_pin_kp.addr);

	ret = register_kprobe(&try_grab_page_kp);
	if (ret < 0) {
		pr_err("try_grab_page_kp register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	xrmem_info("try_grab_page_kp Planted kprobe at 0x%pK\n", try_grab_page_kp.addr);

	ret = register_kprobe(&try_grab_folio_kp);
	if (ret < 0) {
		pr_err("try_grab_folio_kp register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	xrmem_info("try_grab_folio_kp Planted kprobe at 0x%pK\n", try_grab_folio_kp.addr);

	return 0;
}

void pin_track_exit(void)
{
	unregister_kprobe(&try_grab_folio_kp);
	unregister_kprobe(&try_grab_page_kp);
	unregister_kprobe(&folio_add_pin_kp);
}
