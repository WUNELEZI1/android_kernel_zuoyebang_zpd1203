// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/idr.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/list.h>

#include <linux/completion.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/gfp.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include "fk-audio-log.h"


struct aud_log_info {
	/* define audio module id */
	int id;
	/* module name */
	char name[6];
	/* enable flag */
	int en;
	/* log level up */
	int lev_up;
};

struct aud_log_info log_list[] = {
	{.id = AUD_SOC, .name = "asoc", .en = 1, .lev_up = 0},
	{.id = AUD_CORE, .name = "acore", .en = 1, .lev_up = 0},
	{.id = AUD_COMM, .name = "acomm", .en = 1, .lev_up = 0},
	{.id = AUD_DSP, .name = "adsp", .en = 1, .lev_up = 0},
};

void aud_log_info_query(void)
{
	int i = 0;

	pr_info("audio info:\n");
	for (i = 0; i < AUD_MAX; i++) {
		pr_info("  module(%s): enable(%d) lev_up(%d)\n",
			(char *)&log_list[i].name, log_list[i].en,
			log_list[i].lev_up);
	}
}
EXPORT_SYMBOL(aud_log_info_query);

int aud_log_check(int id)
{
	if (id >= AUD_MAX)
		return -1;

	return log_list[id].en;
}
EXPORT_SYMBOL(aud_log_check);

void aud_log_enable(int id, int val)
{
	int i = 0;

	if (id >= AUD_MAX) {
		for (i = 0; i < AUD_MAX; i++) {
			if (val)
				log_list[i].en = 1;
			else
				log_list[i].en = 0;
		}
	} else {
		if (val)
			log_list[id].en = 1;
		else
			log_list[id].en = 0;
	}
}
EXPORT_SYMBOL(aud_log_enable);

int aud_log_lev_up_get(int id)
{
	if (id >= AUD_MAX)
		return -1;

	return log_list[id].lev_up;
}
EXPORT_SYMBOL(aud_log_lev_up_get);

void aud_log_lev_up_set(int id, int lev)
{
	int i = 0;

	if (lev >= 2)
		lev = 2;

	if (id >= AUD_MAX) {
		for (i = 0; i < AUD_MAX; i++) {
			log_list[i].lev_up = lev;
			pr_info("%s: id(%d) set val %d\n", __func__, i, lev);
		}
	} else {
		log_list[id].lev_up = lev;
		pr_info("%s: id(%d) set val %d\n", __func__, id, lev);
	}
}
EXPORT_SYMBOL(aud_log_lev_up_set);

char *aud_log_mod_name(int id)
{
	if (id >= AUD_MAX)
		return (char *)&log_list[0].name;

	return (char *)&log_list[id].name;
}
EXPORT_SYMBOL(aud_log_mod_name);

void aud_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
		...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	trace(&vaf);
	va_end(args);
}
EXPORT_SYMBOL(aud_dbg_trace);

