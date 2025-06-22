// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <soc/xring/sched.h>
#include <soc/xring/xr_tsens.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/common/tsens/tsens_id.h>

#define CREATE_TRACE_POINTS
#include "trace_walt_thermal.h"

enum {
	PP_TEMP_TRIP = 0,
	PP_TEMP_HYST,
	PP_TEMP_NUM,
};

/* Start temp check work by set sysctl:task_pp_delay; Disable by default */
static unsigned int sysctl_task_pp_delay = 100;
static unsigned int sysctl_task_pp_temp[PP_TEMP_NUM] = {95000, 90000};

static struct delayed_work poll_queue;

static void walt_temp_set_polling(unsigned int delay)
{
	if (delay)
		mod_delayed_work(system_freezable_power_efficient_wq,
				&poll_queue, delay);
	else
		cancel_delayed_work(&poll_queue);
}

int task_pp_temp_handler(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp,
				loff_t *ppos)
{
	int ret, i;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);
	int val[PP_TEMP_NUM];
	struct ctl_table tmp = {
		.data	= &val,
		.maxlen	= sizeof(int) * PP_TEMP_NUM,
		.mode	= table->mode,
	};

	mutex_lock(&mutex);

	if (!write) {
		ret = proc_dointvec(table, write, buffer, lenp, ppos);
		goto unlock;
	}

	ret = proc_dointvec(&tmp, write, buffer, lenp, ppos);
	if (ret)
		goto unlock;

	/* check if valid pct values are passed in */
	for (i = 0; i < PP_TEMP_NUM; i++) {
		if (val[i] < 1000) {
			ret = -EINVAL;
			goto unlock;
		}
	}

	memcpy_s(data, PP_TEMP_NUM * sizeof(unsigned int),
			val, PP_TEMP_NUM * sizeof(unsigned int));

unlock:
	mutex_unlock(&mutex);

	return ret;
}

int task_pp_delay_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	static DEFINE_MUTEX(mutex);
	int ret = 0, *val = (unsigned int *)table->data;
	unsigned int old_val;

	mutex_lock(&mutex);
	old_val = sysctl_task_pp_delay;

	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (ret || !write || (old_val == *val))
		goto unlock;

	walt_temp_set_polling(*val);
	if (!*val)
		clear_pingpong_flag(PINGPONG_THERM);

unlock:
	mutex_unlock(&mutex);
	return ret;
}

static struct ctl_table_header *pp_table_header;
static struct ctl_table task_pingpong_table[] = {
	{
		.procname	= "task_pp_temp",
		.data		= &sysctl_task_pp_temp,
		.maxlen		= sizeof(unsigned int) * PP_TEMP_NUM,
		.mode		= 0644,
		.proc_handler	= task_pp_temp_handler,
	},
	{
		.procname	= "task_pp_delay",
		.data		= &sysctl_task_pp_delay,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= task_pp_delay_handler,
	},
	{}
};

static int walt_get_cpub_max_temp(int *temp)
{
	int ret, temp1, temp2;

	ret = xr_tsens_read_temp(TSENS_ID_CPU_B_CORE8, &temp1);
	ret += xr_tsens_read_temp(TSENS_ID_CPU_B_CORE9, &temp2);

	*temp = max(temp1, temp2);

	return ret;
}

static void walt_temp_check(struct work_struct *work)
{
	int temp, ret;

	walt_temp_set_polling(sysctl_task_pp_delay);

	/* Get cpub tz temp */
	ret = walt_get_cpub_max_temp(&temp);
	if (!ret && temp >= sysctl_task_pp_temp[PP_TEMP_TRIP])
		set_pingpong_flag(PINGPONG_THERM);
	else if (!ret && temp <= sysctl_task_pp_temp[PP_TEMP_HYST])
		clear_pingpong_flag(PINGPONG_THERM);

	trace_walt_temp_check(ret, temp, sysctl_task_pp_delay, sysctl_task_pp_temp);
}

int walt_thermal_init(void)
{
	INIT_DELAYED_WORK(&poll_queue, walt_temp_check);
	pp_table_header = register_sysctl("walt", task_pingpong_table);
	if (!pp_table_header)
		pr_err("task pingpong sysctl register fail\n");

	return 0;
}

void walt_thermal_exit(void)
{
	unregister_sysctl_table(pp_table_header);
	walt_temp_set_polling(0);
	clear_pingpong_flag(PINGPONG_THERM);
}
