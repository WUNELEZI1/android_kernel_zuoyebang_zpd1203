// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "pmtest", __func__, __LINE__

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/driver.h>
#include <linux/debugfs.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>
#include <soc/xring/ocm_wrapper.h>
#include <linux/err.h>
#include "xrisp_log.h"

struct rgltr_config {
	struct regulator *rgltr;
	struct mutex rgltr_lock;
	char name[16];
	bool is_poweron;
};

struct xrisp_rgltr_mgnt {
	struct device *dev;
	struct rgltr_config csi;
	struct rgltr_config npu_ocm;
	struct dentry *debugfs_dir;
};

struct xrisp_rgltr_mgnt *pm_mgnt;
static struct platform_device *regulator_consumer;

int xrisp_rgltr_enable(struct rgltr_config *rg_cfg)
{
	int ret = 0;

	if (!rg_cfg || !rg_cfg->rgltr) {
		XRISP_PR_ERROR("regulator %s is NULL, enable failed", rg_cfg->name);
		return -EINVAL;
	}

	mutex_lock(&rg_cfg->rgltr_lock);
	if (rg_cfg->is_poweron) {
		XRISP_PR_INFO("regulator %s already enable, not need to enable again",
			rg_cfg->name);
		goto rgltr_enable_out;
	}

	if (regulator_enable(rg_cfg->rgltr)) {
		XRISP_PR_ERROR("regulator %s enable failed", rg_cfg->name);
		ret = -EINVAL;
		goto rgltr_enable_out;
	}

	rg_cfg->is_poweron = true;
	XRISP_PR_INFO("regulator %s enable success", rg_cfg->name);
rgltr_enable_out:
	mutex_unlock(&rg_cfg->rgltr_lock);
	return ret;
}

int xrisp_rgltr_disable(struct rgltr_config *rg_cfg)
{
	if (!rg_cfg || !rg_cfg->rgltr) {
		XRISP_PR_ERROR("regulator %s is NULL, enable failed", rg_cfg->name);
		return -EINVAL;
	}

	mutex_lock(&rg_cfg->rgltr_lock);
	if (rg_cfg->is_poweron == false) {
		XRISP_PR_INFO("regulator %s already disable, not need to disable again",
			rg_cfg->name);
		goto rgltr_disable_out;
	}

	regulator_disable(rg_cfg->rgltr);
	rg_cfg->is_poweron = false;
	XRISP_PR_INFO("regulator %s disable success", rg_cfg->name);

rgltr_disable_out:
	mutex_unlock(&rg_cfg->rgltr_lock);
	return 0;
}

static int xrisp_rgltr_init(struct rgltr_config *rg_cfg, const char *name)
{
	if (!rg_cfg || !name) {
		XRISP_PR_ERROR("input parm invalid ");
		return -EINVAL;
	}

	rg_cfg->rgltr = NULL;
	rg_cfg->is_poweron = false;
	mutex_init(&rg_cfg->rgltr_lock);
	strscpy(rg_cfg->name, name, sizeof(rg_cfg->name));
	return 0;
}

static int xrisp_rgltr_get(struct rgltr_config *rg_cfg)
{
	if (!rg_cfg) {
		XRISP_PR_ERROR("input parm invalid");
		return -EINVAL;
	}

	if (rg_cfg->rgltr) {
		XRISP_PR_INFO("regulator %s already get, not get again", rg_cfg->name);
		return 0;
	}

	rg_cfg->rgltr = regulator_get(pm_mgnt->dev, rg_cfg->name);
	if (!rg_cfg->rgltr) {
		XRISP_PR_ERROR("regulator %s get failed", rg_cfg->name);
		return -EINVAL;
	}
	XRISP_PR_INFO("regulator %s get success", rg_cfg->name);
	return 0;
}

static void xrisp_rgltr_put(struct rgltr_config *rg_cfg)
{
	if (!rg_cfg) {
		XRISP_PR_ERROR("input parm invalid");
		return;
	}

	if (!rg_cfg->rgltr) {
		XRISP_PR_INFO("regulator %s already put, not put again", rg_cfg->name);
		return;
	}

	if (rg_cfg->is_poweron) {
		XRISP_PR_INFO("regulator %s must disable before put, now disable",
			rg_cfg->name);
		xrisp_rgltr_disable(rg_cfg);
	}

	regulator_put(rg_cfg->rgltr);
	rg_cfg->rgltr = NULL;
}

static ssize_t debug_rgltr(struct file *file, const char __user *user_buf, size_t count,
			   loff_t *ppos)
{
	ssize_t ret = 0;
	struct rgltr_config *rg_cfg;
	char cmd[16] = { 0 };
	char rgltr_ctl[16] = { 0 };
	int i;

	if (!count || count > sizeof(cmd))
		return -EINVAL;

	ret = copy_from_user(cmd, user_buf, count);
	if (ret)
		return -EFAULT;

	for (i = 0; i < 16 && cmd[i] != '\0'; i++) {
		if (cmd[i] == ' ')
			break;
	}

	if (i == 16)
		return -EINVAL;

	strncpy(rgltr_ctl, cmd + i + 1, sizeof(cmd) - i - 1);
	rgltr_ctl[15] = '\0';

	if (!strncmp(cmd, "csi", i))
		rg_cfg = &pm_mgnt->csi;
	else if (!strncmp(cmd, "ocm", i))
		rg_cfg = &pm_mgnt->npu_ocm;
	else
		return -EINVAL;

	if (!strncmp(rgltr_ctl, "enable", sizeof(rgltr_ctl))) {
		ret = xrisp_rgltr_enable(rg_cfg);
	} else if (!strncmp(rgltr_ctl, "disable", sizeof(rgltr_ctl))) {
		ret = xrisp_rgltr_disable(rg_cfg);
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}

	return ret < 0 ? ret : count;
}

static const struct file_operations regulator_ops = {
	.write = debug_rgltr,
};

static int debug_ocm_set_attr(void)
{
	struct arm_smccc_res res = { 0 };

	arm_smccc_smc(FID_BL31_OCM_SET_ATTR, 0x840000000 >> PAGE_SHIFT,
		      0x440000 >> PAGE_SHIFT, XRING_ISP_DMA, 0, 0, 0, 0, &res);

	XRISP_PR_INFO("ocm buf smc set attr %s: %lu\n", res.a0 ? "failed" : "success", res.a0);

	return res.a0;
}

static int debug_ocm_clr_attr(void)
{
	struct arm_smccc_res res = { 0 };

	arm_smccc_smc(FID_BL31_OCM_CLR_ATTR, 0x840000000 >> PAGE_SHIFT,
		      0x440000 >> PAGE_SHIFT, XRING_ISP_DMA, 0, 0, 0, 0, &res);

	XRISP_PR_INFO("ocm buf smc clr attr %s: %lu\n", res.a0 ? "failed" : "success", res.a0);

	return res.a0;
}

static int debug_ocm_attr(void *data, u64 val)
{
	int ret = 0;
	(void)data;

	if (val)
		ret = debug_ocm_set_attr();
	else
		ret = debug_ocm_clr_attr();

	return ret;
}
DEFINE_DEBUGFS_ATTRIBUTE(ocm_attr_ops, NULL, debug_ocm_attr, "%llu\n");

static int regulator_consumer_probe(struct platform_device *pdev)
{
	struct dentry *debugfs_dir;
	struct dentry *debugfs_file;

	pm_mgnt = devm_kzalloc(&pdev->dev, sizeof(*pm_mgnt), GFP_KERNEL);
	if (!pm_mgnt)
		return -ENOMEM;
	pm_mgnt->dev = &pdev->dev;
	xrisp_rgltr_init(&pm_mgnt->csi, "isp_csi");
	xrisp_rgltr_init(&pm_mgnt->npu_ocm, "npu_ocm");

	if (xrisp_rgltr_get(&pm_mgnt->csi))
		goto csi_get_err;

	if (xrisp_rgltr_get(&pm_mgnt->npu_ocm))
		goto ocm_get_err;

	debugfs_dir = debugfs_create_dir("xrisp-rgltr", NULL);

	if (IS_ERR_OR_NULL(debugfs_dir)) {
		XRISP_PR_INFO("xrisp-rgltr debugfs_dir create failed, ret=%ld\n",
			PTR_ERR(debugfs_dir));
		goto debugfs_dir_err;
	}
	pm_mgnt->debugfs_dir = debugfs_dir;

	debugfs_file =
		debugfs_create_file("rgltr", 0600, debugfs_dir, NULL, &regulator_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("rgltr debugfs create failed, ret=%ld\n",
			PTR_ERR(debugfs_file));
		goto debugfs_file_err;
	}

	debugfs_file =
		debugfs_create_file("ocm_attr", 0200, debugfs_dir, NULL, &ocm_attr_ops);
	if (IS_ERR_OR_NULL(debugfs_file)) {
		XRISP_PR_INFO("ocm_attr debugfs create failed  ret=%ld\n",
			PTR_ERR(debugfs_file));
		goto debugfs_file_err;
	}
	XRISP_PR_INFO("probe success");
	return 0;

debugfs_file_err:
	debugfs_remove(debugfs_dir);
debugfs_dir_err:
	xrisp_rgltr_put(&pm_mgnt->npu_ocm);
ocm_get_err:
	xrisp_rgltr_put(&pm_mgnt->csi);
csi_get_err:
	devm_kfree(&pdev->dev, pm_mgnt);
	pm_mgnt = NULL;
	XRISP_PR_INFO("probe failed");
	return -EINVAL;
}

static int regulator_consumer_remove(struct platform_device *pdev)
{
	if (pm_mgnt) {
		xrisp_rgltr_put(&pm_mgnt->csi);
		xrisp_rgltr_put(&pm_mgnt->npu_ocm);
	} else {
		XRISP_PR_ERROR("pm_mgnt is null, remove exit");
		return -EINVAL;
	}

	debugfs_remove(pm_mgnt->debugfs_dir);
	XRISP_PR_INFO("xrisp pm test remove\n");
	return 0;
}

static const struct of_device_id of_xring_regulator_match_tbl[] = {
	/* regulators */
	{
		.compatible = "xring,regulator_isp_test",
	},
	{ /* end */ }
};

static struct platform_driver regulator_consumer_driver = {
	.probe		= regulator_consumer_probe,
	.remove     = regulator_consumer_remove,
	.driver		= {
		.name		= "xrisp_pm_consumer",
		.owner		= THIS_MODULE,
		.of_match_table = of_xring_regulator_match_tbl,
	},
};

static int regulator_consumer_init(void)
{
	int ret;

	XRISP_PR_INFO("regulator consumer init\n");
	regulator_consumer = platform_device_alloc("xrisp_pm_consumer", -1);
	if (!regulator_consumer) {
		XRISP_PR_ERROR("Failed to allocate dummy regulator consumer!\n");
		return -1;
	}
	ret = platform_device_add(regulator_consumer);
	if (ret != 0) {
		XRISP_PR_ERROR("Failed to register dummy regulator consumer: %d\n", ret);
		platform_device_put(regulator_consumer);
		return -1;
	}

	ret = platform_driver_register(&regulator_consumer_driver);
	if (ret != 0) {
		XRISP_PR_ERROR("Failed to register dummy regulator consumer: %d\n", ret);
		platform_device_unregister(regulator_consumer);
	}
	return ret;
}

static void regulator_consumer_exit(void)
{
	XRISP_PR_INFO("regulator consumer exit\n");
	platform_device_unregister(regulator_consumer);
	platform_driver_unregister(&regulator_consumer_driver);
}
module_init(regulator_consumer_init);
module_exit(regulator_consumer_exit);
MODULE_DESCRIPTION("Regulator Test for CSI");
MODULE_LICENSE("GPL v2");
