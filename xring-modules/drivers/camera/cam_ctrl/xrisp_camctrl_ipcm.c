// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "cam_ipcm", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>

#include "cam_rproc/xrisp_rproc.h"
#include "xrisp_camctrl_ipcm.h"
#include "xrisp_cam_ctrl.h"
#include "xrisp_rproc_api.h"
#include "xrisp_ipcm_api.h"
#include "xrisp_debug.h"
#include "cam_rpc/xrisp_cam_rpc_service.h"
#include "xrisp_sensor_ctrl.h"
#include "xrisp_log.h"

static struct dentry *debugfs_dir;

static struct xrisp_ipc_trace ipc_trace;
struct mempool_s *work_mempool;
static struct ipcm_ctrl_t *ipcm_ctrl_g;

inline struct ipcm_ctrl_t *get_ipcm_ctrl(void)
{
	return ipcm_ctrl_g;
}

__maybe_unused void ipc_print_recv_msg(void *data, unsigned int len)
{
	char msg[512] = { 0 };
	u8 *pr_data = data;
	uint32_t size = 0;
	int rowcount;
	int i;

	if (len == 0 || len >= RPMSG_BUF_MAXLEN) {
		XRISP_PR_ERROR("msg len is err, len=%d", len);
		return;
	}

	if (IS_ERR_OR_NULL(data)) {
		XRISP_PR_ERROR("msg addr is err");
		return;
	}

	rowcount = (len / 8) * 8;
	for (i = 0; i < rowcount; i += 8)
		size += sprintf(msg + size, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
				*(pr_data + i), *(pr_data + i + 1), *(pr_data + i + 2),
				*(pr_data + i + 3), *(pr_data + i + 4), *(pr_data + i + 5),
				*(pr_data + i + 6), *(pr_data + i + 7));

	for (; i < len; i++)
		size += sprintf(msg + size, "%02x ", *(pr_data + i));

	XRISP_PR_INFO("recv data:\n%s", msg);
}

void ipc_trace_cnt_init(void)
{
	atomic64_set(&ipc_trace.msg_recv_cnt, 0);
	atomic64_set(&ipc_trace.msg_send_cnt, 0);
	ipc_trace.recv_max_time = 0;
	ipc_trace.recv_avg_time = 0;
	ipc_trace.recv_total_time = 0;
	ipc_trace.send_max_time = 0;
	ipc_trace.send_avg_time = 0;
	ipc_trace.send_total_time = 0;
}

s64 ipc_trace_send_add(void)
{
	return atomic64_inc_return(&ipc_trace.msg_send_cnt);
}

s64 ipc_trace_recv_add(void)
{
	return atomic64_inc_return(&ipc_trace.msg_recv_cnt);
}

void ipc_trace_print(bool is_send, uint8_t ipc_type, int ret, u64 usetime, u64 cnt, const char *fmt,
		     ...)
{
	va_list args;
	uint32_t size;
	char trace_info[256] = { 0 };
	const char *dir = (is_send) ? "AP2FW send" : "FW2AP recv";
	const char *type = ((ipc_type == IPC_RPMSG) ? "rpmsg" : "mbox");
	const char *result = (!!ret) ? "fail" : "success";
	__maybe_unused uint64_t max_time, avg_time;

	size = snprintf(trace_info, sizeof(trace_info), "%s [%s] msg %s, ", dir, type, result);

	va_start(args, fmt);
	size += vsnprintf(trace_info + size, sizeof(trace_info) - size, fmt, args);
	va_end(args);

#ifdef IPC_EXACT_TIME_TRACE_DEBUG
	mutex_lock(&ipc_trace.trace_lock);
	if (is_send) {
		if (usetime > ipc_trace.send_max_time)
			ipc_trace.send_max_time = usetime;
		ipc_trace.send_total_time += usetime;
		ipc_trace.send_avg_time = ipc_trace.send_total_time / cnt;

		max_time = ipc_trace.send_max_time;
		avg_time = ipc_trace.send_avg_time;
	} else {
		if (usetime > ipc_trace.recv_max_time)
			ipc_trace.recv_max_time = usetime;
		ipc_trace.recv_total_time += usetime;
		ipc_trace.recv_avg_time = ipc_trace.recv_total_time / cnt;

		max_time = ipc_trace.recv_max_time;
		avg_time = ipc_trace.recv_avg_time;
	}
	mutex_unlock(&ipc_trace.trace_lock);

	size += sprintf(trace_info + size, " avg_time[%llu]us max_time[%llu]us", avg_time,
			max_time);
#endif

	if (usetime > IPC_MAX_USE_TIME_US)
		size += sprintf(trace_info + size, ", use long time\n");

	if (ret == -ENOMEM && !is_send)
		return;

	if (ret)
		XRISP_PR_ERROR("%s", trace_info);
	else if (usetime > IPC_MAX_USE_TIME_US)
		XRISP_PR_INFO("%s", trace_info);
	// else
	//	XRISP_PR_DEBUG("%s", trace_info);
}

inline uint64_t xrisp_get_msg_id(void *msg)
{
	return (*(uint64_t *)msg);
}

static void *xrisp_ipcm_alloc_ipc_work(void)
{
	return mempool_alloc(work_mempool, GFP_ATOMIC);
}

static void xrisp_ipcm_free_ipc_work(void *ptr)
{
	mempool_free(ptr, work_mempool);
}

static struct workqueue_struct *xrisp_ipcm_get_post_wq(struct topic_handle_t *topic_handle)
{
	struct ipcm_ctrl_t *ipcm_ctrl = get_ipcm_ctrl();

	if (!topic_handle || !ipcm_ctrl)
		return NULL;

	switch (topic_handle->mode) {
	case IPCM_ORDERED_HANDLE:
		return ipcm_ctrl->ipcm_order_wq;

	case SYSTEM_HIGHPRI_HANDLE:
		return system_highpri_wq;

	case FAST_ORDERED_HANDLE:
		return ipcm_ctrl->fast_order_wq;

	default:
		return NULL;
	}
}

static struct topic_handle_t *xrisp_ipcm_get_topic_handle(uint8_t topic)
{
	struct ipcm_ctrl_t *ipcm_ctrl = get_ipcm_ctrl();

	if (!ipcm_ctrl)
		return NULL;

	if (likely(ipcm_ctrl->topic_handle_list[topic].is_registered))
		return &ipcm_ctrl->topic_handle_list[topic];
	else
		return NULL;
}

static void xrisp_ipcm_msg_worker(struct work_struct *work)
{
	struct ipc_work_t *ipc_w = container_of(work, struct ipc_work_t, workq);
	u64 sched_time;
	u64 use_time;
	int ret = 0;

	if (IS_ERR_OR_NULL(ipc_w)) {
		XRISP_PR_ERROR("get ipc msg invalid");
		return;
	}

	sched_time = ktime_us_delta(ktime_get(), ipc_w->start_time);

	if (likely(ipc_w->cb))
		ret = ipc_w->cb(ipc_w->data, ipc_w->len);
	else
		ret = -EINVAL;

	use_time = ktime_us_delta(ktime_get(), ipc_w->start_time);

	ipc_trace_print(
		false, ipc_w->ipc_type, ret, use_time, ipc_w->cnt,
		"topic=0x%02x msg_id=%llu len=%d, schedtime[%llu]us usetime[%llu]us, cnt[%llu]",
		ipc_w->topic, xrisp_get_msg_id(ipc_w->data), ipc_w->len, sched_time, use_time,
		ipc_w->cnt);

	xrisp_ipcm_free_ipc_work(ipc_w);
}

static void xrisp_ipcm_recv_handle_wq_post(void *data, int len, enum xrisp_ipc_type ipc_type,
					   uint8_t topic, struct topic_handle_t *hdl, s64 cnt,
					   ktime_t start)
{
	struct workqueue_struct *wq;
	struct ipc_work_t *ipc_w;

	wq = xrisp_ipcm_get_post_wq(hdl);
	if (unlikely(!wq)) {
		XRISP_PR_ERROR("post wq not find, msg_id=%llu cnt[%llu]", xrisp_get_msg_id(data),
			       cnt);
		return;
	}

	ipc_w = (struct ipc_work_t *)xrisp_ipcm_alloc_ipc_work();
	if (unlikely(!ipc_w)) {
		XRISP_PR_ERROR("no free work for alloc, msg_id=%llu cnt[%llu]",
			       xrisp_get_msg_id(data), cnt);
		return;
	}
	memset(ipc_w, 0, sizeof(struct ipc_work_t));
	memcpy(ipc_w->data, data, len);
	ipc_w->cnt = cnt;
	ipc_w->len = len;
	ipc_w->ipc_type = ipc_type;
	ipc_w->topic = topic;
	ipc_w->cb = hdl->cb;
	ipc_w->start_time = start;
	INIT_WORK(&ipc_w->workq, xrisp_ipcm_msg_worker);
	queue_work(wq, &ipc_w->workq);
}

static int xrisp_ipcm_recv_handle(void *data, uint32_t len, enum xrisp_ipc_type ipc_type)
{
	s64 cnt = ipc_trace_recv_add();
	ktime_t start_time = ktime_get();
	struct topic_handle_t *topic_hdl;
	void *msg_data;
	int msg_len;
	uint8_t topic;
	int ret = 0;

	if (IS_ERR_OR_NULL(data) || len > RPMSG_BUF_MAXLEN) {
		XRISP_PR_ERROR("msg is invalid, cnt[%llu]", cnt);
		return -EINVAL;
	}

	msg_data = (void *)data + 1;
	msg_len = len - 1;

	topic = *(uint8_t *)data;
	topic_hdl = xrisp_ipcm_get_topic_handle(topic);
	if (unlikely(!topic_hdl)) {
		XRISP_PR_ERROR("topic 0x%02x handle not find, msg_id=%llu cnt[%llu]", topic,
			       xrisp_get_msg_id(msg_data), cnt);
		return -ENODEV;
	}

	switch (topic_hdl->mode) {
	case FAST_DIRECT_HANDLE:
		ret = topic_hdl->cb(msg_data, msg_len);
		if (unlikely(ret))
			XRISP_PR_ERROR(
				"msg mbox recv error, topic=0x%02x msg_id=%llu len=%d usetime[%llu]us cnt[%llu]",
				topic, xrisp_get_msg_id(msg_data), msg_len,
				ktime_us_delta(ktime_get(), start_time), cnt);
		break;

	case IPCM_ORDERED_HANDLE:
	case FAST_ORDERED_HANDLE:
	case SYSTEM_HIGHPRI_HANDLE:
		xrisp_ipcm_recv_handle_wq_post(msg_data, msg_len, ipc_type, topic, topic_hdl, cnt,
					       start_time);
		break;

	default:
		XRISP_PR_ERROR("topic 0x%02x handle mode %d invalid, msg_id=%llu, cnt[%llu]", topic,
			       topic_hdl->mode, xrisp_get_msg_id(msg_data), cnt);
		return -EINVAL;
	}

	return ret;
}

int xrisp_topic_handle_register(uint8_t topic, msg_cb cb, uint32_t mode)
{
	struct ipcm_ctrl_t *ipcm_ctrl = get_ipcm_ctrl();
	struct topic_handle_t *topic_handle = NULL;
	int ret = 0;

	if (!ipcm_ctrl)
		return -ENOMEM;

	if (topic >= XRISP_TOPIC_MAX) {
		XRISP_PR_ERROR("invalid topic %d", topic);
		return -EINVAL;
	}

	if (mode >= MSG_HANDLE_MODE_MAX) {
		XRISP_PR_ERROR("invalid mode %d", mode);
		return -EINVAL;
	}

	if (!cb) {
		XRISP_PR_ERROR("invalid cb");
		return -EINVAL;
	}

	if (xrisp_rproc_is_running())
		return -EBUSY;

	mutex_lock(&ipcm_ctrl->ipcm_mtx);

	topic_handle = &ipcm_ctrl->topic_handle_list[topic];
	if (topic_handle->is_registered) {
		XRISP_PR_ERROR("topic 0x%02x has been registered, busy", topic);
		ret = -EBUSY;
		goto register_out;
	}

	topic_handle->cb = cb;
	topic_handle->mode = mode;
	topic_handle->is_registered = true;

register_out:
	mutex_unlock(&ipcm_ctrl->ipcm_mtx);
	if (!ret)
		XRISP_PR_INFO("topic 0x%02x handle register success", topic);
	else
		XRISP_PR_ERROR("topic 0x%02x handle register failed", topic);
	return ret;
}

void xrisp_topic_handle_unregister(uint8_t topic)
{
	struct ipcm_ctrl_t *ipcm_ctrl = get_ipcm_ctrl();

	if (!ipcm_ctrl)
		return;

	if (topic >= XRISP_TOPIC_MAX) {
		XRISP_PR_ERROR("invalid topic %d", topic);
		return;
	}

	if (xrisp_rproc_is_running())
		return;

	mutex_lock(&ipcm_ctrl->ipcm_mtx);

	if (ipcm_ctrl->topic_handle_list[topic].is_registered) {
		memset(&ipcm_ctrl->topic_handle_list[topic], 0, sizeof(struct topic_handle_t));
		XRISP_PR_INFO("topic 0x%02x handle unregister success", topic);
	}

	mutex_unlock(&ipcm_ctrl->ipcm_mtx);
}

static void xrisp_topic_handle_clean(void)
{
	struct ipcm_ctrl_t *ipcm_ctrl = get_ipcm_ctrl();
	int i;

	if (!ipcm_ctrl)
		return;

	mutex_lock(&ipcm_ctrl->ipcm_mtx);
	for (i = 0; i < XRISP_TOPIC_MAX; i++)
		memset(&ipcm_ctrl->topic_handle_list[i], 0, sizeof(struct topic_handle_t));

	mutex_unlock(&ipcm_ctrl->ipcm_mtx);
}

int ipc_ppm_recv_handle_cb(void *data, unsigned int len)
{
	return xrisp_send_event_to_hal(data, len, RING_CAMERA_V4L2_EVENT_TYPE);
}

int ipc_rpc_hal_recv_handle_cb(void *data, unsigned int len)
{
	return xrisp_send_event_to_hal(data, len, RING_CAMERA_V4L2_RPC_EVENT_TYPE);
}

static int xrisp_ipcm_create_ipc_work_mempool(void)
{
	struct kmem_cache *cache;

	cache = kmem_cache_create("isp_ipc_work_pool", sizeof(struct ipc_work_t), 0,
				  SLAB_HWCACHE_ALIGN, NULL);
	if (!cache) {
		XRISP_PR_ERROR("create kmem_cache failed");
		return -ENOMEM;
	}

	work_mempool = mempool_create_slab_pool(MEMPOOL_WORK_ENTITY_MAX, cache);
	if (!work_mempool) {
		kmem_cache_destroy(cache);
		XRISP_PR_ERROR("create memory pool failed");
		return -ENOMEM;
	}

	XRISP_PR_INFO("create ipc_work memory pool success");
	return 0;
}

static void xrisp_ipcm_destroy_ipc_work_mempool(void)
{
	mempool_destroy(work_mempool);
	work_mempool = NULL;
}

int xrisp_ipcm_init(void)
{
	int ret = 0;

	ipcm_ctrl_g = kzalloc(sizeof(struct ipcm_ctrl_t), GFP_KERNEL);
	if (!ipcm_ctrl_g) {
		XRISP_PR_ERROR("alloc ipcm ctrl failed");
		return -ENOMEM;
	}

	ipcm_ctrl_g->ipcm_order_wq = alloc_ordered_workqueue(
		"isp_norm_msg_wq", WQ_HIGHPRI | WQ_MEM_RECLAIM | WQ_FREEZABLE);
	if (!ipcm_ctrl_g->ipcm_order_wq) {
		XRISP_PR_ERROR("isp_norm_msg_wq alloc failed");
		ret = -EINVAL;
		goto free_mem;
	}

	ipcm_ctrl_g->fast_order_wq = alloc_ordered_workqueue(
		"isp_fast_msg_wq", WQ_HIGHPRI | WQ_MEM_RECLAIM | WQ_FREEZABLE);
	if (!ipcm_ctrl_g->fast_order_wq) {
		XRISP_PR_ERROR("isp_fast_msg_wq alloc failed");
		ret = -EINVAL;
		goto destroy_norm_wq;
	}

	ret = xrisp_ipcm_create_ipc_work_mempool();
	if (ret)
		goto destroy_fast_wq;

	mutex_init(&ipcm_ctrl_g->ipcm_mtx);

	ret = xrisp_topic_handle_register(TOPIC_PPM, ipc_ppm_recv_handle_cb, IPCM_ORDERED_HANDLE);
	if (ret)
		goto destroy_mempool;

	ret = xrisp_topic_handle_register(TOPIC_DRV, xrisp_ipc_cpc_handler, IPCM_ORDERED_HANDLE);
	if (ret)
		goto unregister_ppm;

	ret = xrisp_topic_handle_register(TOPIC_ISP_RPC_HAL, ipc_rpc_hal_recv_handle_cb,
					  IPCM_ORDERED_HANDLE);
	if (ret)
		goto unregister_drv;

	mutex_init(&ipc_trace.trace_lock);

#ifdef XRISP_MBOX_DEBUG_CONFIG
	xrisp_mbox_debug_init();
#endif

	return 0;

unregister_drv:
	xrisp_topic_handle_unregister(TOPIC_DRV);
unregister_ppm:
	xrisp_topic_handle_unregister(TOPIC_PPM);
destroy_mempool:
	xrisp_ipcm_destroy_ipc_work_mempool();
	mutex_destroy(&ipcm_ctrl_g->ipcm_mtx);

destroy_fast_wq:
	destroy_workqueue(ipcm_ctrl_g->fast_order_wq);
destroy_norm_wq:
	destroy_workqueue(ipcm_ctrl_g->ipcm_order_wq);
free_mem:
	kfree(ipcm_ctrl_g);
	ipcm_ctrl_g = NULL;

	return ret;
}

void xrisp_ipcm_exit(void)
{
	if (!ipcm_ctrl_g)
		return;

#ifdef XRISP_MBOX_DEBUG_CONFIG
	xrisp_mbox_debug_exit();
#endif

	mutex_destroy(&ipc_trace.trace_lock);
	xrisp_topic_handle_unregister(TOPIC_ISP_RPC_HAL);
	xrisp_topic_handle_unregister(TOPIC_DRV);
	xrisp_topic_handle_unregister(TOPIC_PPM);

	xrisp_topic_handle_clean();
	xrisp_ipcm_destroy_ipc_work_mempool();
	destroy_workqueue(ipcm_ctrl_g->fast_order_wq);
	destroy_workqueue(ipcm_ctrl_g->ipcm_order_wq);
	mutex_destroy(&ipcm_ctrl_g->ipcm_mtx);
	kfree(ipcm_ctrl_g);
	ipcm_ctrl_g = NULL;
}

static void xrisp_norm_mbox_cb(void *rx_buff, void *arg)
{
	xrisp_ipcm_recv_handle(rx_buff, MBOX_BUF_MAXLEN, IPC_MBOX);
}

static void xrisp_fast_mbox_cb(void *rx_buff, void *arg)
{
	xrisp_ipcm_recv_handle(rx_buff, MBOX_BUF_MAXLEN, IPC_MBOX);
}

int xrisp_ctrl_mbox_register(bool *register_done)
{
	if (IS_ERR_OR_NULL(register_done))
		return -EINVAL;

	if (xr_rproc_fast_ack_recv_register(CTRL_MBOX_CH, xrisp_norm_mbox_cb, NULL)) {
		XRISP_PR_ERROR("norm_mbox_channel register fail");
		return -EINVAL;
	}

	if (xr_rproc_fast_ack_recv_register(FAST_MBOX_CH, xrisp_fast_mbox_cb, NULL)) {
		XRISP_PR_ERROR("fast_mbox_channel register fail");
		xr_rproc_fast_ack_recv_unregister(CTRL_MBOX_CH);
		return -EINVAL;
	}
	*register_done = true;
	XRISP_PR_INFO("mbox recv_cb register success");
	return 0;
}

int xrisp_ctrl_mbox_unregister(bool *need_unregister)
{
	int ret = 0;
	bool unregister_error = false;

	if (IS_ERR_OR_NULL(need_unregister))
		return -EINVAL;

	if (*need_unregister == false)
		return 0;

	ret = xr_rproc_fast_ack_recv_unregister(CTRL_MBOX_CH);
	if (ret) {
		unregister_error = true;
		XRISP_PR_ERROR("common_mbox_channel unregister fail, ret=%d", ret);
	}

	ret = xr_rproc_fast_ack_recv_unregister(FAST_MBOX_CH);
	if (ret) {
		unregister_error = true;
		XRISP_PR_ERROR("fast_mbox_channel unregister fail, ret=%d", ret);
	}

	*need_unregister = false;
	if (unlikely(unregister_error)) {
		XRISP_PR_INFO("mbox recv_cb unregister failed");
		return -EINVAL;
	}
	XRISP_PR_INFO("mbox recv_cb unregister success");
	return 0;
}

static int xrisp_ctrl_mbox_send(unsigned int *tx_buffer, unsigned int len, unsigned int type)
{
	if (likely(type == CTRL_MBOX_CH))
		return xr_rproc_send_sync(CTRL_MBOX_CH, tx_buffer, len);
	else
		return xr_rproc_send_sync(FAST_MBOX_CH, tx_buffer, len);
}

static int xrisp_rpmsg_recv_handle(void *data, uint32_t len, enum xrisp_ipc_type ipc_type)
{
	s64 cnt = ipc_trace_recv_add();
	ktime_t start_time = ktime_get();
	ktime_t use_time;
	struct topic_handle_t *topic_hdl;
	void *msg_data;
	int msg_len;
	uint8_t topic;
	int ret = 0;

	if (IS_ERR_OR_NULL(data) || len > RPMSG_BUF_MAXLEN) {
		XRISP_PR_ERROR("msg is invalid, cnt[%llu]", cnt);
		return -EINVAL;
	}

	msg_data = (void *)data + 1;
	msg_len = len - 1;

	topic = *(uint8_t *)data;
	topic_hdl = xrisp_ipcm_get_topic_handle(topic);
	if (unlikely(!topic_hdl)) {
		XRISP_PR_ERROR("topic 0x%02x handle not find, msg_id=%llu cnt[%llu]", topic,
			       xrisp_get_msg_id(msg_data), cnt);
		return -ENODEV;
	}

	ret = topic_hdl->cb(msg_data, msg_len);

	use_time = ktime_us_delta(ktime_get(), start_time);

	ipc_trace_print(false, ipc_type, ret, use_time, cnt,
			"topic=0x%02x msg_id=%llu len=%d,  usetime[%llu]us, cnt[%llu]", topic,
			xrisp_get_msg_id(msg_data), len, use_time, cnt);

	return 0;
}

void xrisp_ipcm_rpmsg_recv_cb(void *priv, void *data, int len)
{
	xrisp_rpmsg_recv_handle(data, len, IPC_RPMSG);
}

void xrisp_rpmsg_register(void)
{
	static int priv;

	xrisp_rpmsg_register_recv_cb(&priv, xrisp_ipcm_rpmsg_recv_cb);
	XRISP_PR_INFO("rpmsg_recv_cb register success\n");
}

int xrisp_msg_send_core(void *data, uint32_t len, uint8_t topic, bool data_from_user)
{
	u8 buffer[512] = { 0 };
	ktime_t start = ktime_get();
	u64 usetime;
	uint8_t ipc_type;
	u64 cnt = ipc_trace_send_add();
	int ret = 0;

	if (len == 0 | len >= RPMSG_BUF_MAXLEN) {
		XRISP_PR_ERROR("input msg len over limit, len=%d", len);
		return -EFAULT;
	}

	if (data_from_user == true) {
		if (!access_ok(data, len)) {
			XRISP_PR_ERROR("input msg is err");
			return -EFAULT;
		}

		if (copy_from_user((buffer + 1), data, len)) {
			XRISP_PR_ERROR("failed to copying from user");
			return -EFAULT;
		}
	} else {
		if (IS_ERR_OR_NULL(data)) {
			XRISP_PR_ERROR("input msg addr is err");
			return -EINVAL;
		}

		memcpy((buffer + 1), data, len);
	}
	buffer[0] = topic;

	if (len < MBOX_BUF_MAXLEN) {
		ipc_type = IPC_MBOX;
		ret = xrisp_ctrl_mbox_send((void *)buffer, len + 1, CTRL_MBOX_CH);
	} else {
		ipc_type = IPC_RPMSG;
		ret = xrisp_rpmsg_send(buffer, len + 1);
	}

	usetime = ktime_us_delta(ktime_get(), start);

	ipc_trace_print(true, ipc_type, ret, usetime, cnt,
			"topic=0x%02x msg_id=%llu len=%d, usetime[%llu]us, cnt[%llu]", topic,
			xrisp_get_msg_id(&buffer[1]), len + 1, usetime, cnt);

	return ret;
}

int xrisp_msg_send(void *data, unsigned int len, uint8_t topic)
{
	int ret = 0;

	switch (topic) {
	case TOPIC_PPM:
	case TOPIC_ISP_RPC:
		ret = xrisp_msg_send_core(data, len, topic, true);
		break;

	case TOPIC_EDR:
	case TOPIC_DRV:
	case TOPIC_LOG:
		ret = xrisp_msg_send_core(data, len, topic, false);
		break;

	default:
		XRISP_PR_ERROR("input unknown topic 0x%02x", topic);
		return -EINVAL;
	}

	return ret;
}

int xrisp_cam_rpc_send_kernel_msg(void *data, unsigned int len)
{
	return xrisp_msg_send_core(data, len, TOPIC_ISP_RPC, false);
}
EXPORT_SYMBOL(xrisp_cam_rpc_send_kernel_msg);

static int ois_ipc_recv_callback(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	uint32_t sensor_id;
	uint16_t sensor_num;
	int i;

	if (IS_ERR_OR_NULL(pkt))
		return -EFAULT;

	if (tag != TAG_SH_OIS)
		return -EINVAL;

	sensor_num = pkt->header.data_len / 4;
	for (i = 0; i < sensor_num; i++) {
		sensor_id = *(uint32_t *)(pkt->data + 4 * i);
		xrisp_sensor_ois_complete(sensor_id);
	}

	return 0;
}

static struct ipc_notifier_info ois_notifier_block = {
	.callback = ois_ipc_recv_callback,
	.arg = NULL,
};

int xrisp_ois_mbox_register(bool *register_done)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(register_done))
		return -EINVAL;

	ret = sh_register_recv_notifier(SHUB_MBOX_CH, TAG_SH_OIS, &ois_notifier_block);
	if (ret)
		XRISP_PR_ERROR("ois mbox register fail");
	else
		*register_done = true;
	return ret;
}

int xrisp_ois_mbox_unregister(bool *need_unregister)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(need_unregister))
		return -EINVAL;

	if (*need_unregister) {
		ret = sh_unregister_recv_notifier(SHUB_MBOX_CH, TAG_SH_OIS, &ois_notifier_block);
		if (ret)
			XRISP_PR_ERROR("ois mbox unregister fail");
	}

	*need_unregister = false;
	return ret;
}

/* ipcm debug func */
__maybe_unused void debug_rpmsg_recv_cb(void *priv, void *data, int len)
{
	XRISP_PR_INFO("debug: rpmsg recv msg, topic=0x%02x, len=%d, msg_id=%llu", *(u8 *)data, len,
		      xrisp_get_msg_id(data + 1));
	ipc_print_recv_msg(data + 1, len - 1);
}

__maybe_unused int xrisp_debug_send_msg(void *data, unsigned int len)
{
	int ret = 0;

	if (len <= MBOX_BUF_MAXLEN) {
		ret = xrisp_ctrl_mbox_send(data, len, CTRL_MBOX_CH);
		XRISP_PR_INFO("AP2FW ppm_mbox_msg send finish");
	} else {
		ret = xrisp_rpmsg_send(data, len);
		XRISP_PR_INFO("AP2FW ppm_rpmsg_msg send finish");
	}
	if (ret)
		XRISP_PR_ERROR("AP2FW send ppm_msg failed");
	return ret;
}

static ssize_t cam_ipcm_debug(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	u8 buf[RPMSG_BUF_MAXLEN] = { 0 };
	static bool is_register;
	static int priv;
	uint32_t lens = 0;
	int i = 0;

	if (kstrtouint_from_user(ubuf, cnt, 10, &lens)) {
		XRISP_PR_ERROR("input msg lens invalid: %d", lens);
		return -EINVAL;
	}

	if (lens >= RPMSG_BUF_MAXLEN) {
		XRISP_PR_ERROR("input msg lens over limit");
		return -EINVAL;
	}

	if (xrisp_rproc_is_running() != true) {
		XRISP_PR_ERROR("isp not power on, test failed");
		is_register = false;
		return -EINVAL;
	}

	if (lens != 0) {
		XRISP_PR_INFO("send msg, len=%d", lens);
		buf[0] = TOPIC_PPM;
		for (i = 0; i < lens; i++)
			buf[i + 1] = i;

		xrisp_debug_send_msg(buf, lens + 1);
	} else { //register recv
		if (is_register == true) {
			XRISP_PR_ERROR(
				"debug rpmsg_recv_cb already register, not need to register again");
			return -EINVAL;
		}
		is_register = true;
		xrisp_rpmsg_register_recv_cb(&priv, debug_rpmsg_recv_cb);
		XRISP_PR_INFO("debug rpmsg_recv_cb register finish\n");
	}

	return cnt;
}

#define MODULE_DEBUGFS_DEFINE(func_name)                                                           \
	static const struct file_operations func_name##_fops = {                                   \
		.write = func_name##_debug,                                                        \
	}

MODULE_DEBUGFS_DEFINE(cam_ipcm);

int xrisp_camctrl_debugfs_init(void)
{
	struct dentry *debugfs_file = NULL;

	debugfs_dir = debugfs_create_dir("cam_ctrl", xrisp_debugfs_get_root());

	if (IS_ERR_OR_NULL(debugfs_dir)) {
		XRISP_PR_INFO("cam_ctrl dir create failed %ld\n", PTR_ERR(debugfs_dir));
		return -EINVAL;
	}

	debugfs_file = debugfs_create_file("ipcm_test", 0600, debugfs_dir, NULL, &cam_ipcm_fops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("ipcm_test debugfs create failed %ld\n", PTR_ERR(debugfs_file));
		debugfs_remove(debugfs_dir);
		return -EINVAL;
	}

	XRISP_PR_INFO("camctrl ipcm debugfs init finish");
	return 0;
}

void xrisp_camctrl_debugfs_exit(void)
{
	debugfs_remove(debugfs_dir);
	XRISP_PR_INFO("camctrl ipcm debugfs exit finish");
}
