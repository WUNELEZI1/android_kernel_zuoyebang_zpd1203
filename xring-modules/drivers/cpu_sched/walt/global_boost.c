// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
 */

#include <trace/hooks/sched.h>
#include "walt.h"

#ifdef CONFIG_SCHED_AUTOGROUP
static inline bool task_group_is_autogroup(struct task_group *tg)
{
	return !!tg->autogroup;
}
#else /* !CONFIG_SCHED_AUTOGROUP */
static inline bool task_group_is_autogroup(struct task_group *tg)
{
	return 0;
}
#endif /* CONFIG_SCHED_AUTOGROUP */

/* Integer rounded range for each bucket */
#define UCLAMP_BUCKET_DELTA DIV_ROUND_CLOSEST(SCHED_CAPACITY_SCALE, UCLAMP_BUCKETS)

static inline unsigned int uclamp_bucket_id(unsigned int clamp_value)
{
	return min_t(unsigned int, clamp_value / UCLAMP_BUCKET_DELTA, UCLAMP_BUCKETS - 1);
}

static inline void uclamp_se_set(struct uclamp_se *uc_se,
				 unsigned int value, bool user_defined)
{
	uc_se->value = value;
	uc_se->bucket_id = uclamp_bucket_id(value);
	uc_se->user_defined = user_defined;
}

static inline struct uclamp_se
uclamp_tg_restrict(struct task_struct *p, enum uclamp_id clamp_id)
{
	/* Copy by value as we could modify it */
	struct uclamp_se uc_req = p->uclamp_req[clamp_id];
#ifdef CONFIG_UCLAMP_TASK_GROUP
	unsigned int tg_min, tg_max, value;

	/*
	 * Tasks in autogroups or root task group will be
	 * restricted by system defaults.
	 */
	if (task_group_is_autogroup(task_group(p)))
		return uc_req;
	if (task_group(p) == &root_task_group)
		return uc_req;

	tg_min = task_group(p)->uclamp[UCLAMP_MIN].value;
	tg_max = task_group(p)->uclamp[UCLAMP_MAX].value;
	value = uc_req.value;
	value = clamp(value, tg_min, tg_max);
	uclamp_se_set(&uc_req, value, false);
#endif

	return uc_req;
}

static void android_rvh_uclamp_eff_get(void *unused, struct task_struct *p,
				       enum uclamp_id clamp_id,
				       struct uclamp_se *uc_max,
				       struct uclamp_se *uc_eff, int *ret)
{
	unsigned int global_boost_min = sysctl_sched_global_boost_min_uclamp;

	*uc_eff = uclamp_tg_restrict(p, clamp_id);
	if (uc_eff->value >= global_boost_min)
		return;

	*ret = true;
	uclamp_se_set(uc_eff, global_boost_min, false);

	/* System default restrictions always apply */
	if (unlikely(uc_eff->value > uc_max->value))
		*uc_eff = *uc_max;
}

void global_boost_init(void)
{
	register_trace_android_rvh_uclamp_eff_get(android_rvh_uclamp_eff_get, NULL);
}
