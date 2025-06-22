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
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <soc/xring/xr-clk-provider.h>
#include "xr-dvfs-private.h"
#include "xr-dvfs-vote.h"

#define MAX_VOTE_NAME_LEN         30

static struct task_struct *vote_owner;
static int g_vote_refcnt;
static LIST_HEAD(vote_list);
static DEFINE_MUTEX(vote_lock);

static void dvfs_vote_lock(void)
{
	if (!mutex_trylock(&vote_lock)) {
		if (vote_owner == current) {
			g_vote_refcnt++;
			return;
		}
		mutex_lock(&vote_lock);
	}
	WARN_ON_ONCE(vote_owner != NULL);
	WARN_ON_ONCE(g_vote_refcnt != 0);
	vote_owner = current;
	g_vote_refcnt = 1;
}

static void dvfs_vote_unlock(void)
{
	WARN_ON_ONCE(vote_owner != current);
	WARN_ON_ONCE(g_vote_refcnt == 0);

	if (--g_vote_refcnt)
		return;
	vote_owner = NULL;
	mutex_unlock(&vote_lock);
}

static struct dvfs_volt_vote *__vote_lookup(unsigned int dev_id,
		const char *name)
{
	struct dvfs_volt_vote *vote_inst = NULL;
	unsigned int vote_name_len;

	if (name != NULL) {
		vote_name_len = strnlen(name, MAX_VOTE_NAME_LEN);
		list_for_each_entry(vote_inst, &vote_list, node) {
			unsigned int tmp = max_value(vote_name_len,
				strnlen(vote_inst->name, MAX_VOTE_NAME_LEN));

			if (!strncmp(vote_inst->name, name, tmp))
				return vote_inst;
		}
	}

	if (dev_id) {
		list_for_each_entry(vote_inst, &vote_list, node)
			if (dev_id == vote_inst->dev_id)
				return vote_inst;
	}

	return NULL;
}

struct dvfs_volt_vote *dvfs_vote_get(unsigned int dev_id, const char *name)
{
	struct dvfs_volt_vote *vote_inst = NULL;

	if (!dev_id && name == NULL)
		return NULL;

	dvfs_vote_lock();

	vote_inst = __vote_lookup(dev_id, name);

	dvfs_vote_unlock();

	return vote_inst;
}
EXPORT_SYMBOL_GPL(dvfs_vote_get);

/*
 * dvfs_get_volt - return the volt of dvfs
 * @vote_inst: the dvfs_volt_vote whose volt is being returned.
 */
unsigned int dvfs_get_volt(struct dvfs_volt_vote *vote_inst)
{
	unsigned int volt = 0;

	if (vote_inst == NULL)
		return (unsigned int)(-EINVAL);

	dvfs_vote_lock();

	if (vote_inst->ops && vote_inst->ops->get_volt)
		volt = vote_inst->ops->get_volt(vote_inst);

	dvfs_vote_unlock();

	return volt;
}
EXPORT_SYMBOL_GPL(dvfs_get_volt);

unsigned int dvfs_get_volt_by_level(struct dvfs_volt_vote *vote_inst,
		unsigned int volt_level)
{
	unsigned int volt = 0;

	if (vote_inst == NULL)
		return (unsigned int)(-EINVAL);

	dvfs_vote_lock();

	if (vote_inst->ops && vote_inst->ops->get_volt_by_level)
		volt = vote_inst->ops->get_volt_by_level(vote_inst->dev_id, volt_level);

	dvfs_vote_unlock();

	return volt;
}
EXPORT_SYMBOL_GPL(dvfs_get_volt_by_level);

static int __dvfs_set_volt(struct dvfs_volt_vote *vote_inst,
		unsigned int volt_level)
{
	int ret = 0;

	if (vote_inst->ops && vote_inst->ops->set_volt)
		ret = vote_inst->ops->set_volt(vote_inst, volt_level);

	return ret;
}

/*
 * dvfs_set_volt - specify a new volt for vote
 * @vote_inst: the dvfs_volt_vote whose volt is being changed
 * @volt_level: the new volt level for vote
 *
 * Returns 0 on success, otherwise an error code.
 */
int dvfs_set_volt(struct dvfs_volt_vote *vote_inst, unsigned int volt_level)
{
	int ret;

	if (vote_inst == NULL)
		return -EINVAL;

	dvfs_vote_lock();

	ret = __dvfs_set_volt(vote_inst, volt_level);
	if (ret) {
		clkerr("%s dvfs set volt fail!\n", vote_inst->name);
		goto out;
	}
	vote_inst->volt_level = volt_level;

out:
	dvfs_vote_unlock();
	return ret;
}
EXPORT_SYMBOL_GPL(dvfs_set_volt);

int dvfs_wait_completed(struct dvfs_volt_vote *vote_inst)
{
	unsigned int volt = 0;

	if (vote_inst == NULL)
		return (unsigned int)(-EINVAL);

	dvfs_vote_lock();

	if (vote_inst->ops && vote_inst->ops->wait_completed)
		volt = vote_inst->ops->wait_completed(vote_inst);

	dvfs_vote_unlock();

	return volt;
}
EXPORT_SYMBOL_GPL(dvfs_wait_completed);

/*
 * dvfs_is_low_temprature - get current temperature
 * Returns 0 on normal temperature, 1 on low temperature.
 */
int dvfs_is_low_temperature(struct dvfs_volt_vote *vote_inst)
{
	int ret = 0;

	if (vote_inst == NULL)
		return -EINVAL;

	dvfs_vote_lock();

	if (vote_inst->ops && vote_inst->ops->is_low_temperature)
		ret = vote_inst->ops->is_low_temperature();

	dvfs_vote_unlock();

	return ret;
}
EXPORT_SYMBOL_GPL(dvfs_is_low_temperature);

static int __dvfs_vote_register(struct dvfs_volt_vote *vote_inst)
{
	int ret = 0;

	dvfs_vote_lock();

	if (__vote_lookup(vote_inst->dev_id, vote_inst->name)) {
		clkerr("vote_inst %u %s already initialized\n",
			vote_inst->dev_id, vote_inst->name);
		ret = -EEXIST;
		goto out;
	}

	list_add(&vote_inst->node, &vote_list);

out:
	dvfs_vote_unlock();
	return ret;
}

/*
 * dvfs_vote_register - register a dvfs_volt_vote.
 * Returns 0 on success, otherwise an error code.
 */
int dvfs_vote_register(struct dvfs_volt_vote *vote_inst)
{
	int ret;

	if (vote_inst == NULL)
		return -EINVAL;

	ret = __dvfs_vote_register(vote_inst);
	if (ret)
		clkerr("dvfs vote inst init faild, %d!", ret);

	return ret;
}
EXPORT_SYMBOL_GPL(dvfs_vote_register);
