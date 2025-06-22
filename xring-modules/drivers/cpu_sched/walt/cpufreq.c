// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "walt.h"


DEFINE_PER_CPU(struct xresgov_callback *, xresgov_cb_data);

/*before call the function, caller should hold the cpu_rq(cpu)->__lock*/
void xresgov_run_callback(unsigned int cpu, unsigned int flags)
{
	struct xresgov_callback *cb;
	struct rq *rq = cpu_rq(cpu);

	lockdep_assert_held(&rq->__lock);
	cb = rcu_dereference_sched(*per_cpu_ptr(&xresgov_cb_data, cpu));
	if (cb)
		cb->func(cb, walt_sched_clock(), flags);
}
EXPORT_SYMBOL(xresgov_run_callback);

void xresgov_add_callback(int cpu, struct xresgov_callback *cb,
			void (*func)(struct xresgov_callback *cb, u64 time,
			unsigned int flags))
{
	if (WARN_ON(!cb || !func))
		return;

	if (WARN_ON(per_cpu(xresgov_cb_data, cpu)))
		return;

	cb->func = func;
	rcu_assign_pointer(per_cpu(xresgov_cb_data, cpu), cb);
}
EXPORT_SYMBOL(xresgov_add_callback);

void xresgov_remove_callback(int cpu)
{
	rcu_assign_pointer(per_cpu(xresgov_cb_data, cpu), NULL);
}
EXPORT_SYMBOL(xresgov_remove_callback);

