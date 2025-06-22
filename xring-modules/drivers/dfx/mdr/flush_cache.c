// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xring flush cache
 */

#include "mdr_print.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/arm-smccc.h>
#include <linux/libnvdimm.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/logbuf.h>

/*
 * The followning note is from "Arm ®  Architecture Reference Manual"
 * ------------------------------------------------------------------------------------
 * Because the set/way instructions operate only locally, there is no guarantee of the
 * atomicity of cache maintenance between different PEs, even if those different PEs
 * are each executing the same cache maintenance instructions at the same time.
 * Because any cacheable line can be allocated into the cache at any time, it is
 * possible for a cache line to migrate from an entry in the cache of one PE to the
 * cache of a different PE in a way that means the cache line is not affected by
 * set/way based cache maintenance. Therefore, Arm strongly discourages the use of
 * set/way instructions to manage coherency in coherent systems. The expected use of
 * the cache maintenance instructions that operate by set/way is limited to the cache
 * maintenance associated with the powerdown and powerup of caches, if this is required
 * by the implementation.
 * ------------------------------------------------------------------------------------
 *
 * Note: The set/way cache flushing interface can be used only when the system is
 * abnormally reset and restarted. This interface cannot be used in other scenarios.
 *
 */
void flush_cache_all_by_setway(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_CPU_FLUSHCACHE_BY_SETWAY, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("smc return fail.\n");
}
EXPORT_SYMBOL(flush_cache_all_by_setway);

static void flush_logbuf_cache(void)
{
	u64 logbuf_addr = get_logbuf_addr();
	size_t logbuf_size = get_logbuf_size();

	if (!logbuf_addr) {
		pr_err("wrong logbuf addr\n");
		return;
	}

	if (!logbuf_size) {
		pr_err("wrong logbuf size\n");
		return;
	}

	pr_info("flush logbuf cache[size:0x%lx]\n", logbuf_size);
	arch_wb_cache_pmem((void *)logbuf_addr, logbuf_size);
}

void flush_dfx_cache(void)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_MNTN_FLUSH_DFX_CACHE, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("flush dfx cache fail\n");

	pr_info("flush dfx cache succ\n");

	flush_logbuf_cache();
}
EXPORT_SYMBOL(flush_dfx_cache);
