// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
 */

#define pr_fmt(fmt) "dvfs_gov: " fmt

#include <linux/device.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <soc/xring/vote_mng.h>

#include "dvfs_gov.h"

struct dvfs_gov {
	struct mutex mutex;
	struct vote_mng *vote_mng;
	u32 cluster_num;
	u8 inited;
};

static struct dvfs_gov g_dvfs_gov;

int dvfs_gov_msg_trans(struct vote_msg *msg)
{
	int ret;

	if (msg == NULL) {
		pr_err("invald msg\n");
		return -1;
	}

	mutex_lock(&g_dvfs_gov.mutex);
	ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)msg,
				VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
	mutex_unlock(&g_dvfs_gov.mutex);
	if (ret < 0)
		pr_err("vote mng msg send failed\n");

	return ret;
}
EXPORT_SYMBOL(dvfs_gov_msg_trans);

int dvfs_gov_data_send(struct vote_msg *header, u8 *data)
{
	int ret;
	struct vote_msg dst;
	u8 *src = data;
	u32 size;

	if (header == NULL || data == NULL) {
		pr_err("invald header or data\n");
		return -1;
	}

	size = header->data_size;
	mutex_lock(&g_dvfs_gov.mutex);
	ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)header,
				VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
	if (ret < 0)
		goto err_exit;

	while (size > VOTE_MSG_SIZE) {
		memcpy(&dst, src, VOTE_MSG_SIZE);
		ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)&dst,
					VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
		if (ret < 0)
			goto err_exit;

		src += VOTE_MSG_SIZE;
		size -= VOTE_MSG_SIZE;
	}
	if (size > 0) {
		memcpy(&dst, src, size);
		ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)&dst,
					VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
	}
err_exit:
	mutex_unlock(&g_dvfs_gov.mutex);
	if (ret < 0)
		pr_err("vote mng msg send failed\n");

	return ret;
}
EXPORT_SYMBOL(dvfs_gov_data_send);

int dvfs_gov_data_receive(struct vote_msg *header, u8 *buf, u32 buf_size)
{
	int ret;
	struct vote_msg src;
	u8 *dst = buf;
	u32 size;

	if (header == NULL || buf == NULL || buf_size <= 0) {
		pr_err("invald header or data buf\n");
		return -1;
	}

	mutex_lock(&g_dvfs_gov.mutex);
	ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)header,
				VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
	if (ret < 0)
		goto err_exit;

	size = header->data_size;
	if (size > buf_size)
		goto err_exit;

	while (size > VOTE_MSG_SIZE) {
		ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)&src,
					VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
		if (ret < 0)
			goto err_exit;

		memcpy(dst, &src, VOTE_MSG_SIZE);
		dst += VOTE_MSG_SIZE;
		size -= VOTE_MSG_SIZE;
	}
	if (size > 0) {
		ret = vote_mng_msg_send(g_dvfs_gov.vote_mng, (u32 *)&src,
					VOTE_MSG_SIZE / 4, VOTE_MNG_MSG_SYNC);
		if (ret < 0)
			goto err_exit;

		memcpy(dst, &src, size);
	}

err_exit:
	mutex_unlock(&g_dvfs_gov.mutex);
	if (ret < 0)
		pr_err("vote data receive failed\n");

	return ret;
}
EXPORT_SYMBOL(dvfs_gov_data_receive);

u32 get_cluster_num(void)
{
	return g_dvfs_gov.cluster_num;
}
EXPORT_SYMBOL(get_cluster_num);

static int dvfs_gov_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *of_node = NULL;

	BUILD_BUG_ON(sizeof(struct vote_msg) != VOTE_MSG_SIZE);

	if (g_dvfs_gov.inited) {
		dev_err(dev, "only one dvfs gov device allowed\n");
		return -EINVAL;
	}
	g_dvfs_gov.inited = 1;
	mutex_init(&g_dvfs_gov.mutex);
	g_dvfs_gov.vote_mng = vote_mng_get(dev, "vote_mng");
	if (!g_dvfs_gov.vote_mng) {
		dev_err(dev, "get vote mng failed\n");
		return -ENOMEM;
	}
	of_node = of_find_node_by_name(NULL, "cpu-map");
	if (of_node != NULL) {
		g_dvfs_gov.cluster_num = of_get_available_child_count(of_node);
	} else {
		dev_err(dev, "failed to get cpu topology\n");
		return -EINVAL;
	}
	if (of_get_available_child_count(dev->of_node))
		of_platform_populate(dev->of_node, NULL, NULL, dev);

	return 0;
}

static const struct of_device_id match_table[] = {
	{
		.compatible = "xring,dvfs-gov",
	},
	{},
};

static struct platform_driver g_dvfs_gov_driver = {
	.probe = dvfs_gov_probe,
	.driver = {
		.name = "dvfs-gov",
		.owner = THIS_MODULE,
		.of_match_table = match_table,
	},
};

static inline int dvfs_gov_dev_init(void)
{
	return platform_driver_register(&g_dvfs_gov_driver);
}

static int __init dvfs_gov_init(void)
{
	int ret;

	ret = dvfs_gov_dev_init();
	if (ret) {
		pr_err("dvfs gov init failed\n");
		return ret;
	}
	ret = xsee_init();
	if (ret) {
		pr_err("xsee init failed\n");
		return ret;
	}
	ret = memlat_init();
	if (ret) {
		pr_err("memlat init failed\n");
		return ret;
	}

	return ret;
}
module_init(dvfs_gov_init);

MODULE_AUTHOR("Shaohua Fan <fanshaohua@xiaomi.com>");
MODULE_DESCRIPTION("XRing DVFS GOV Driver");
MODULE_SOFTDEP("pre: xr_vote_mng");
MODULE_LICENSE("GPL v2");
