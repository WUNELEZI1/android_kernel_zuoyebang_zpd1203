// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	"[xr_dfx][dmd]:%s:%d " fmt, __func__, __LINE__

#include <soc/xring/dmd.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/string.h>
#include <soc/xring/securelib/securec.h>
#include <linux/delay.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"
#include "dt-bindings/xring/platform-specific/common/mdr/include/dfx_kernel_user.h"

static struct dmd_server g_dmd_server;
static struct delayed_work g_dmd_delay_work;

static void report_event(const struct dmd_client *client)
{
	u32 size = strlen(client->dump_buff);

	if (mdr_send_nl((char *)client->dump_buff, size) < 0)
		dmd_log_err("dmd log send error\n");
}

static void dmd_client_set_idle(struct dmd_client *client)
{
	if (!client) {
		dmd_log_err("client is null\n");
		return;
	}

	client->error_no = 0;
	memset_s(client->dump_buff, client->buff_size, 0, client->buff_size);
	clear_bit(CBUFF_READY_BIT, &client->buff_flag);
	clear_bit(DMD_CLIENT_NOTIFY_BIT, &g_dmd_server.client_flag[client->client_id]);
}

static void dmd_delay_work_fn(struct work_struct *work)
{
	int i;
	struct dmd_client *client = NULL;

	mutex_lock(&g_dmd_server.mtx_lock);

	if (!is_mdr_nlpid_valid()) {
		pr_err("save log process not exist\n");
		queue_delayed_work(system_wq, &g_dmd_delay_work, msecs_to_jiffies(DMD_WORK_DELAY_TIME));
		mutex_unlock(&g_dmd_server.mtx_lock);
		return;
	}

	for (i = 0; i < CLIENT_SIZE; i++) {
		if (test_bit(DMD_CLIENT_VAILD_BIT, &g_dmd_server.client_flag[i])) {
			if ((!test_bit(DMD_CLIENT_NOTIFY_BIT, &g_dmd_server.client_flag[i])))
				continue;

			dmd_log_info("No.%d client name %s flag 0x%lx\n", i,
					g_dmd_server.client_list[i]->client_name,
					g_dmd_server.client_flag[i]);

			client = g_dmd_server.client_list[i];

			report_event(client);
			dmd_log_info("%s finish notify\n", client->client_name);
			dmd_client_set_idle(client);
		}
	}
	mutex_unlock(&g_dmd_server.mtx_lock);
}

static void dmd_client_notify(struct dmd_client *client, int error_no)
{
	int ret;

	if (client) {
		client->error_no = error_no;
		set_bit(CBUFF_READY_BIT, &client->buff_flag);
		set_bit(DMD_CLIENT_NOTIFY_BIT,
				&g_dmd_server.client_flag[client->client_id]);

		ret = queue_delayed_work(system_wq, &g_dmd_delay_work, 0);
		if (!ret)
			dmd_log_err("work was already on the queue.\n");
	}
}

/*
 * This interface can be called at most once in 2s for the same module.
 * LOG_SIZE < 256
 *
 */
int dmd_client_record(struct dmd_client *client, int error_no, const char *fmt, ...)
{
	va_list args;
	int size;
	size_t avail;
	char dmd_log[DMD_MAX_LOG_SIZE];
	char date[DATATIME_MAXLEN];

	if (!client) {
		dmd_log_err("no client to record\n");
		return 0;
	}

	memset_s(dmd_log, DMD_MAX_LOG_SIZE, 0, DMD_MAX_LOG_SIZE);
	memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);

	avail = client->buff_size - 1;

	va_start(args, fmt);
	size = vsnprintf(dmd_log, DMD_MAX_LOG_SIZE, fmt, args);
	va_end(args);

	if (unlikely(size > DMD_MAX_LOG_SIZE)) {
		dmd_log_err("dmd log size error, size: %d!\n", size);
		return -EINVAL;
	}

	size = snprintf(date, DATATIME_MAXLEN - 1, "%s.%04d",
		mdr_get_timestamp(), mdr_get_tick());
	if (unlikely(size > DATATIME_MAXLEN)) {
		dmd_log_err("snprintf size %d!\n", size);
		return -EINVAL;
	}

	size = snprintf(client->dump_buff, avail,
		"Fault_ID[%s:%d], Module[%s], Time[%s], Fault_Level[%s], Data[%s]\n",
		client->client_name, error_no,
		client->module_name,
		date,
		client->fault_level,
		dmd_log);

	if (unlikely(size > avail)) {
		dmd_log_err("dmd dump buff size error, size: %d, avail: %zu!\n", size, avail);
		return -EINVAL;
	}

	dmd_client_notify(client, error_no);

	return size;
}
EXPORT_SYMBOL(dmd_client_record);

/*
 * buff_size < 1024
 * fault_level = CRITICAL/WARNING/INFO
 * clent name len  < 32
 * moudle name len < 32
 *
 */
struct dmd_client *dmd_register_client(struct dmd_dev *dev)
{
	int i;
	int ret;
	int conflict = -1;
	struct dmd_client *ptr = NULL;

	if ((!dev) || (dev->buff_size > DMD_EXTERN_CLIENT_MAX_BUF_SIZE)) {
		dmd_log_err("dmd_dev is NULL or buffer size is too big\n");
		return NULL;
	}

	/*
	 * before add lock , ensure read and write operations
	 * of g_dmd_server are ordered
	 */
	smp_rmb();
	mutex_lock(&g_dmd_server.mtx_lock);

	if (g_dmd_server.client_count >= CLIENT_SIZE) {
		dmd_log_info("clients has full\n");
		goto out;
	}

	ptr = vzalloc(sizeof(*ptr) + dev->buff_size);
	if (!ptr)
		goto out;
	dmd_log_info("clients malloc success\n");

	for (i = 0; i < CLIENT_SIZE; i++) {
		if (!test_bit(DMD_CLIENT_VAILD_BIT, &g_dmd_server.client_flag[i]))
			break;

		if (!dev->name) {
			dmd_log_err("Please specify the dmd device name\n");
			vfree(ptr);
			ptr = NULL;
			goto out;
		}

		conflict = strncmp(g_dmd_server.client_list[i]->client_name,
				dev->name,
				CLIENT_NAME_LEN);
		if (!conflict) {
			dmd_log_err("new client %s conflict with No.%d client %s\n",
					dev->name, i,
					g_dmd_server.client_list[i]->client_name);
			break;
		}
	}

	if ((i >= CLIENT_SIZE) || (!conflict)) {
		dmd_log_err("clients register failed, index %d, conflict %d\n",
				i, conflict);
		vfree(ptr);
		ptr = NULL;
		goto out;
	}

	ret = strncpy_s(ptr->client_name, CLIENT_NAME_LEN - 1, dev->name, strlen(dev->name));
	if (ret != 0) {
		dmd_log_err("strncpy client name failed\n");
		vfree(ptr);
		ptr = NULL;
		goto out;
	}

	if (dev->module_name) {
		ret = strncpy_s(ptr->module_name, DMD_MAX_MODULE_NAME_LEN - 1,
				dev->module_name, strlen(dev->module_name));
		if (ret != 0) {
			dmd_log_err("strncpy module name failed\n");
			vfree(ptr);
			ptr = NULL;
			goto out;
		}
	}

	if (dev->fault_level) {
		ret = strncpy_s(ptr->fault_level, DMD_MAX_FAULT_LEVEL - 1,
				dev->fault_level, strlen(dev->fault_level));
		if (ret != 0) {
			dmd_log_err("strncpy fault level failed\n");
			vfree(ptr);
			ptr = NULL;
			goto out;
		}

	}

	ptr->client_id = i;
	ptr->buff_size = dev->buff_size;
	g_dmd_server.client_list[i] = ptr;
	set_bit(DMD_CLIENT_VAILD_BIT, &g_dmd_server.client_flag[i]);
	g_dmd_server.client_count++;
	dmd_log_info("client %s register success\n", ptr->client_name);

	/*
	 * before release lock. ensure read and write operations
	 * of g_dmd_server are ordered
	 */
	smp_wmb();
out:
	mutex_unlock(&g_dmd_server.mtx_lock);
	return ptr;
}
EXPORT_SYMBOL(dmd_register_client);

void dmd_unregister_client(struct dmd_client *dmd_client, struct dmd_dev *dev)
{
	int i;
	int conflict;

	if ((!dmd_client) || (!dev) || (!dev->name)) {
		dmd_log_err("pointer is NULL, please check the parameters\n");
		return;
	}

	mutex_lock(&g_dmd_server.mtx_lock);
	for (i = 0; i < CLIENT_SIZE; i++) {
		/* find the client and free it */
		if (g_dmd_server.client_list[i] == NULL)
			continue;
		conflict = strncmp(g_dmd_server.client_list[i]->client_name,
				dev->name,
				CLIENT_NAME_LEN);
		if (!conflict) {
			__clear_bit(DMD_CLIENT_VAILD_BIT,
					&g_dmd_server.client_flag[i]);
			g_dmd_server.client_list[i] = NULL;
			g_dmd_server.client_count--;
			vfree(dmd_client);
			dmd_client = NULL;
			break;
		}
	}
	mutex_unlock(&g_dmd_server.mtx_lock);
}
EXPORT_SYMBOL(dmd_unregister_client);

static int dmd_init(void)
{
	memset(&g_dmd_server, 0, sizeof(struct dmd_server));
	mutex_init(&g_dmd_server.mtx_lock);
	INIT_DELAYED_WORK(&g_dmd_delay_work, dmd_delay_work_fn);

	dmd_log_info("dmd init success\n");

	return 0;
}

static void dmd_exit(void)
{
	pr_info("dmd exit!\n");
}

module_init(dmd_init);
module_exit(dmd_exit);

MODULE_AUTHOR("Ren Jie");
MODULE_DESCRIPTION("dmd driver");
MODULE_LICENSE("GPL");
