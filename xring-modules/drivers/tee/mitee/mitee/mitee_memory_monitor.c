// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: tee memory monitor file
 * Modify time: 2024-12-15
 * Author: Security-TEEOS
 */

#define pr_fmt(fmt) "[tee]:%s " fmt, __func__

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <linux/random.h>
#include "miev/mievent.h"
#include <tee_drv.h>

#include "mitee_memory_monitor.h"

/* memory monitor misight event */
#define TEE_MEMORY_MONITOR_NO     (1000)
#define TEE_MEMORY_MONITOR_APPID  "31000401732"
#define TEE_MEMORY_MONITOR_EVENT  "TeeMemoryMonitor"
#define TEE_MEMORY_MONITOR_PARA_0 "TeeTotalMemory"
#define TEE_MEMORY_MONITOR_PARA_1 "TeeUsedMemory"
#define TEE_MEMORY_MONITOR_PARA_2 "TeeLatestMemoryPeak"

/* memory monitor context */
#define MEMORY_MONITOR_NOT_INIT  (0)
#define MEMORY_MONITOR_INIT_DONE  (1)

#define MEMORY_MONITOR_WORK_DELAY_SHORT_MS  (5 * 60 * 1000)
#define MEMORY_MONITOR_WORK_DELAY_MEDIUM_MS (60 * 60 * 1000)
#define MEMORY_MONITOR_WORK_DELAY_LONG_MS   (11 * 60 * 60 * 1000)

struct fbe_memory_monitor_ctx {
	struct delayed_work memory_monitor_work;
	atomic_t exp_ms;
	atomic_t init_flag;
};

#if IS_ENABLED(CONFIG_XRING_MITEE_MEMORY_MONITOR)
static struct fbe_memory_monitor_ctx m_m_ctx;
#endif

/* kernel ca context */
#define TEE_NUM_PARAMS            (4)
#define TEE_INFO_MEMORY_MONITOR   (0x2)
#define TEE_INFO_TA_UUID \
		{0x01, 0xf2, 0xaa, 0x8a, 0x60, 0x24, 0x00, 0x00, \
		0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x00, 0x00, 0x01 }

#if IS_ENABLED(CONFIG_XRING_MITEE_MEMORY_MONITOR)
static const struct tee_ioctl_open_session_arg g_sess_arg_origin = {
	.uuid = TEE_INFO_TA_UUID,
	.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL,
	.session = 0,
	.num_params = 0
};
static struct tee_ioctl_open_session_arg g_sess_arg;
static struct tee_context *g_tee_ctx;

/* kernel ca/ta connect */
static int tee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	(void)data;

	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int open_context_and_session(void)
{
	int ret;

	if (g_tee_ctx == NULL) {
		/* Open Context */
		g_tee_ctx = tee_client_open_context(NULL, tee_ctx_match, NULL, NULL);
		if (IS_ERR(g_tee_ctx)) {
			g_tee_ctx = NULL;
			pr_err("tee_client_open_context failed\n");
			return -ENODEV;
		}

		/* Open Session */
		g_sess_arg = g_sess_arg_origin;
		ret = tee_client_open_session(g_tee_ctx, &g_sess_arg, NULL);
		if ((ret != 0) || (g_sess_arg.ret != 0)) {
			pr_err("tee_client_open_session failed with code 0x%x origin 0x%x\n",
					g_sess_arg.ret, g_sess_arg.ret_origin);
			tee_client_close_context(g_tee_ctx);
			g_tee_ctx = NULL;
			return -EINVAL;
		}
	}

	return 0;
}

static void close_session_and_context(void)
{
	if (g_tee_ctx != NULL) {
		tee_client_close_session(g_tee_ctx, g_sess_arg.session);
		tee_client_close_context(g_tee_ctx);
		g_tee_ctx = NULL;
	}
}

/* kernel ca to tee-info ta for parsing tee memory status (KiB) */
static int parse_memory_status(int *total, int *used, int *latest_memory_peak)
{
	int ret = 0;
	struct tee_context *tee_ctx;
	struct tee_ioctl_open_session_arg *sess_arg;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[TEE_NUM_PARAMS];

	if ((total == NULL) || (used == NULL))
		return -EINVAL;

	/*invoke command parse tee memory status (KiB) */
	ret = open_context_and_session();
	if (ret != 0)
		return ret;

	tee_ctx = g_tee_ctx;
	sess_arg = &g_sess_arg;
	memset(&inv_arg, 0, sizeof(struct tee_ioctl_invoke_arg));
	inv_arg.func = TEE_INFO_MEMORY_MONITOR;
	inv_arg.session = sess_arg->session;
	inv_arg.num_params = TEE_NUM_PARAMS;

	memset(&param[0], 0, TEE_NUM_PARAMS * sizeof(struct tee_param));
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	ret = tee_client_invoke_func(tee_ctx, &inv_arg, param);
	if ((ret != 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func failed with code 0x%x origin 0x%x\n",
		       inv_arg.ret, inv_arg.ret_origin);
		ret = -EINVAL;
		close_session_and_context();
		return ret;
	}
	*total = param[0].u.value.a;
	*used = param[0].u.value.b;
	*latest_memory_peak = param[1].u.value.a;
	pr_debug("total %d, used %d, latest_memory_peak %d\n", *total, *used, *latest_memory_peak);

	if (g_tee_ctx != NULL)
		close_session_and_context();
	return 0;
}

/* random delay time */
static unsigned int get_random_delay(void)
{
	unsigned int random = 0;

	get_random_bytes(&random, sizeof(unsigned int));
	random = random % MEMORY_MONITOR_WORK_DELAY_LONG_MS;
	random += MEMORY_MONITOR_WORK_DELAY_MEDIUM_MS;
	return random;
}

/* key function: parse tee memory status from tee and write to misight*/
static int memory_monitor_record(void)
{
	int ret = 0;
	int total = 0;
	int used = 0;
	int latest_memory_peak = 0;
	struct misight_mievent *event;

	ret = parse_memory_status(&total, &used, &latest_memory_peak);
	if (ret) {
		pr_err("tee memory monitor from TA error %d\n", ret);
		return ret;
	}

	/* NOTICE: record content */
	event = cdev_tevent_alloc(TEE_MEMORY_MONITOR_NO);
	if (!event) {
		pr_err("misight event is error\n");
		return -ENOMEM;
	}

	cdev_tevent_add_str(event, "appId", TEE_MEMORY_MONITOR_APPID);
	cdev_tevent_add_str(event, "eventName", TEE_MEMORY_MONITOR_EVENT);
	cdev_tevent_add_int(event, TEE_MEMORY_MONITOR_PARA_0, total);
	cdev_tevent_add_int(event, TEE_MEMORY_MONITOR_PARA_1, used);
	cdev_tevent_add_int(event, TEE_MEMORY_MONITOR_PARA_2, latest_memory_peak);
	cdev_tevent_write(event);
	cdev_tevent_destroy(event);

	return ret;
}

/* work entry func. */
static void memory_monitor_func(struct work_struct *work)
{
	int ret = 0;
	struct delayed_work *m_m_work;
	struct fbe_memory_monitor_ctx *priv;

	m_m_work = to_delayed_work(work);
	if (m_m_work == NULL)
		return;
	priv = container_of(m_m_work, struct fbe_memory_monitor_ctx, memory_monitor_work);
	if (priv == NULL)
		return;
	/* init memory monitor context */
	if (atomic_read(&priv->init_flag) != MEMORY_MONITOR_INIT_DONE) {
		atomic_set(&priv->exp_ms, get_random_delay());
		atomic_set(&priv->init_flag, MEMORY_MONITOR_INIT_DONE);
	}

	/* record memory status, retry at next delay moment*/
	ret = memory_monitor_record();
	if (ret)
		pr_err("tee memory monitor record error %d\n", ret);

	/* start the next delay */
	if (!schedule_delayed_work(m_m_work, msecs_to_jiffies(atomic_read(&priv->exp_ms))))
		pr_warn("tee memory monitor work with long delay insert kernel-global workqueue failed!\n");

	pr_debug("next delay %d\n", atomic_read(&priv->exp_ms));
}
#endif
/* init and exit func. */
void mitee_memory_monitor_init(void)
{
#if IS_ENABLED(CONFIG_XRING_MITEE_MEMORY_MONITOR)
	bool ret = true;
	/* A deferrable timer will work normally when the system is busy,
	 * but will not cause a CPU to come out of idle just to service it;
	 */
	INIT_DEFERRABLE_WORK(&m_m_ctx.memory_monitor_work, memory_monitor_func);

	atomic_set(&m_m_ctx.exp_ms, MEMORY_MONITOR_WORK_DELAY_SHORT_MS);
	atomic_set(&m_m_ctx.init_flag, MEMORY_MONITOR_NOT_INIT);
	ret = schedule_delayed_work(&m_m_ctx.memory_monitor_work,
			msecs_to_jiffies(atomic_read(&m_m_ctx.exp_ms)));
	if (!ret)
		pr_warn("tee memory monitor work insert kernel-global workqueue failed!\n");
#endif
}

void mitee_memory_monitor_exit(void)
{
#if IS_ENABLED(CONFIG_XRING_MITEE_MEMORY_MONITOR)
	if (g_tee_ctx != NULL)
		close_session_and_context();

	/* cancle work from kernel-global workqueue */
	cancel_delayed_work_sync(&m_m_ctx.memory_monitor_work);
	atomic_set(&m_m_ctx.exp_ms, MEMORY_MONITOR_WORK_DELAY_SHORT_MS);
	atomic_set(&m_m_ctx.init_flag, MEMORY_MONITOR_NOT_INIT);
#endif
}
