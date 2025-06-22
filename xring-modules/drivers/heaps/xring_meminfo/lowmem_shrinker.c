// SPDX-License-Identifier: GPL-2.0
#include <linux/types.h>
#include <linux/nodemask.h>
#include <linux/shrinker.h>
#include <linux/swap.h>
#include <linux/sysinfo.h>
#include <linux/version.h>

#include "xring_meminfo.h"

static unsigned int free_swap_limit;
module_param(free_swap_limit, uint, 0644);

static unsigned int total_swap_limit = 100000;
module_param(total_swap_limit, uint, 0644);

static unsigned long lowmem_count(struct shrinker *s, struct shrink_control *sc)
{
	struct sysinfo i;

	si_swapinfo(&i);
	if (K(i.totalswap) >= total_swap_limit && K(get_nr_swap_pages()) <= free_swap_limit)
		xr_meminfo_show(false, FROM_SHRINKER);
	return 0;
}

static unsigned long lowmem_scan(struct shrinker *s, struct shrink_control *sc)
{
	return 0;
}


static struct shrinker lowmem_shrinker = {
	.count_objects = lowmem_count,
	.scan_objects = lowmem_scan,
};

int xr_lowmem_shrinker_init(void)
{
#if KERNEL_VERSION(6, 0, 0) > LINUX_VERSION_CODE
	register_shrinker(&lowmem_shrinker);
#else
	register_shrinker(&lowmem_shrinker, "xring-lowmem");
#endif
	return 0;
}

void xr_lowmem_shrinker_exit(void)
{
	unregister_shrinker(&lowmem_shrinker);
}
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(MINIDUMP);
