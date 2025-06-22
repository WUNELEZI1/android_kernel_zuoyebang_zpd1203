/* SPDX-License-Identifier: GPL-2.0 */
/*
 */
#ifndef __UFS_XRING_DEBUG_H__
#define __UFS_XRING_DEBUG_H__

#if IS_ENABLED(CONFIG_XRING_DEBUG)
#include "ufshcd.h"
#include <linux/device.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/version.h>

#define PROC_PERM		0660

#define REGISTER_TRACE(trace_fn, probe_fn, error_msg, ret)   \
	do {                                                \
		ret = trace_fn(probe_fn, NULL);                 \
		if (ret) {                                      \
			pr_err("UFS:Failed to register " error_msg ":%d\n", ret); \
			return ret;                                 \
		}                                               \
	} while (0)

#define UNREGISTER_TRACE(trace_fn, probe_fn, error_msg, ret)   \
	do {                                                \
		ret = trace_fn(probe_fn, NULL);                 \
		if (ret) {                                      \
			pr_err("UFS:Failed to unregister " error_msg ":%d\n", ret); \
			return ret;                                 \
		}                                               \
	} while (0)

enum clk_scaling_state {
	CLKS_SCALE_DOWN,
	CLKS_SCALE_UP,
	CLKS_SCALE_UNKNOWN
};

enum ufsdbg_pm_state {
	UFSDBG_RUNTIME_SUSPEND,
	UFSDBG_RUNTIME_RESUME,
	UFSDBG_SYSTEM_SUSPEND,
	UFSDBG_SYSTEM_RESUME
};

int ufs_xring_dbg_register(struct ufs_hba *hba);
int ufs_xring_dbg_unregister(void);
#else
static inline int ufs_xring_dbg_register(struct ufs_hba *hba)
{
	return 0;
}

static inline int ufs_xring_dbg_unregister(void)
{
	return 0;
}

#endif
#endif
