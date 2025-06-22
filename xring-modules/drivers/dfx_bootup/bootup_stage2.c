// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	"[xr_dfx][bootup]:%s:%d " fmt, __func__, __LINE__

#include <linux/string.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <soc/xring/dfx_bootup.h>

int bootup_stage2_init(void)
{
	int ret;

	ret = bootup_keypoint_addr_init();
	if (ret) {
		pr_err("%s init keypoint addr failed!\n", __func__);
		return ret;
	}
	ret = set_bootup_status(STAGE_KERNEL_STAGE_TWO_START);
	if (ret) {
		pr_err("%s set_bootup_status failed!\n", __func__);
		return ret;
	}
	return 0;
}

static void __exit bootup_stage2_exit(void)
{
	pr_info("remove bootup_stage2.ko");
}


module_init(bootup_stage2_init);
module_exit(bootup_stage2_exit);

MODULE_LICENSE("GPL");
