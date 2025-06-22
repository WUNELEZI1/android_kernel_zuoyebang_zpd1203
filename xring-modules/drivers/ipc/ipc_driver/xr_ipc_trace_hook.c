// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/notifier.h>
#include "soc/xring/xr_timestamp.h"
#include "soc/xring/trace_hook_set.h"
#include "soc/xring/hook_ringbuffer_api.h"
#include "xr_ipc_trace_hook.h"
#include "xr_mbox.h"

struct mntn_ipc_notifier_info {
	struct hook_ringbuffer *hook_rbuf;
	struct notifier_block nb;
};

struct mntn_ipc_hook_dev {
	struct device			dev;
	struct mntn_ipc_notifier_info	*ni;
};
static struct mntn_ipc_hook_dev g_ipc_hook_dev;

static inline struct mntn_ipc_hook_dev *get_ipc_hook_dev(void)
{
	return &g_ipc_hook_dev;
}

void ipc_trace_hook_call_chain(u8 vc_id, u8 irq_sta, u8 dir)
{
	struct ipc_hook_info hook_info;
	struct xr_ipc *xdev = NULL;

	xdev = xr_ipc_get_xdev();
	if (!xdev) {
		pr_err("[ipc_trace] Fail to get xdev");
		return;
	}

	hook_info.vc_id = vc_id;
	hook_info.irq_sta = irq_sta;
	hook_info.dir = dir;

	atomic_notifier_call_chain(&xdev->hook_nb, 0, (void *)&hook_info);
}

static int ipc_hook_notifier_call(struct notifier_block *nb,
			unsigned long action, void *hook_info)
{
	struct mntn_ipc_notifier_info *ni;
	struct ipc_hook_info *record = (struct ipc_hook_info *)hook_info;

	ni = container_of(nb, struct mntn_ipc_notifier_info, nb);

	record->clock = xr_timestamp_gettime();

	ringbuffer_write(ni->hook_rbuf, (u8 *)record);

	return 0;
}

static inline void ipc_trace_hook_dev_release(struct device *dev)
{
}

int ipc_trace_hook_init(void)
{
	struct mntn_ipc_hook_dev *hook_dev = get_ipc_hook_dev();
	struct trace_hook_root *hook_root = NULL;
	struct mntn_ipc_notifier_info *ni;
	struct xr_ipc *xdev = NULL;
	int ret;

	device_initialize(&hook_dev->dev);
	dev_set_name(&hook_dev->dev, "ipc_trace");
	hook_dev->dev.release = ipc_trace_hook_dev_release;
	ret = device_add(&hook_dev->dev);
	if (ret < 0) {
		pr_err("[ipc_trace] Fail to add device ipc_trace");
		return ret;
	}

	hook_root = trace_hook_root_get();
	if (IS_ERR_OR_NULL(hook_root)) {
		dev_err(&hook_dev->dev, "ipc get hook root failed!\n");
		return -EFAULT;
	}

	ni = kzalloc(sizeof(*ni), GFP_KERNEL);
	if (!ni) {
		dev_err(&hook_dev->dev, "Fail to malloc ipc notifier info");
		return -ENOMEM;
	}
	hook_dev->ni = ni;

	xdev = xr_ipc_get_xdev();
	if (!xdev) {
		dev_err(&hook_dev->dev, "Fail to get xdev");
		return -ENODEV;
	}

	ni->hook_rbuf = (struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_IPC];

	ATOMIC_INIT_NOTIFIER_HEAD(&xdev->hook_nb);
	ni->nb.notifier_call = ipc_hook_notifier_call;
	ni->nb.next = NULL;
	ni->nb.priority = 0;
	atomic_notifier_chain_register(&xdev->hook_nb, &ni->nb);

	dev_info(&hook_dev->dev, "ipc_trace_hook init success\n");

	return ret;
}

void ipc_trace_hook_exit(void)
{
	struct mntn_ipc_hook_dev *hook_dev = get_ipc_hook_dev();
	struct mntn_ipc_notifier_info *ni = hook_dev->ni;
	struct xr_ipc *xdev = NULL;

	xdev = xr_ipc_get_xdev();
	if (!xdev) {
		dev_err(&hook_dev->dev, "Fail to get xdev");
		return;
	}

	atomic_notifier_chain_unregister(&xdev->hook_nb, &ni->nb);
	kfree(ni);
	device_unregister(&hook_dev->dev);
}

