// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 - 2023 MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include "gpio_sap_ctrl.h"


#define GPIO_SAP_CTRL_DEBUG (0)
#if GPIO_SAP_CTRL_DEBUG
#include <linux/device.h>
#endif

#define GPIO_DRIVER_NAME		"gpio_sap_ctrl"
#define GPIO_DEV_NODE_NAME		"gpio_sap_ctrl"
#define PINCTRL_STATE_SAP_STATUS_OFF_NUM    (1)
#define PINCTRL_STATE_SAP_STATUS_DOWNLOAD_NUM    (2)
#define PINCTRL_STATE_SAP_STATUS_WORKING_NUM    (5)
#define PINCTRL_STATE_NUM       (PINCTRL_STATE_SAP_STATUS_OFF_NUM + \
                                PINCTRL_STATE_SAP_STATUS_DOWNLOAD_NUM + \
                                PINCTRL_STATE_SAP_STATUS_WORKING_NUM)
enum sap_status {
    SAP_STATUS_MIN          = -1,
    SAP_STATUS_OFF          = 0,
    SAP_STATUS_DOWNLOAD     = 1,
    SAP_STATUS_WORKING      = 2,
    SAP_STATUS_MAX          = 3,
};

enum pctrl_state {
    PCTRL_STATE_SAP_OFF             = 0,
    PCTRL_STATE_SAP_DOWNLOAD_PD     = 1,
    PCTRL_STATE_SAP_DOWNLOAD_PU     = 2,
    PCTRL_STATE_SAP_WORKING_PD      = 3,
    PCTRL_STATE_SAP_WORKING_PU      = 4,
    PCTRL_STATE_SAP_WORKING_NOPULL  = 5,
    PCTRL_STATE_SAP_WORKING_OUT_0   = 6,
    PCTRL_STATE_SAP_WORKING_OUT_1   = 7,
};

struct pctrl_state_range {
    enum sap_status status;
    int start;
    int end;
};

static char* names[PINCTRL_STATE_NUM] = {
    "sap_off",
    "sap_download_pd",
    "sap_download_pu",
    "sap_working_pd",
    "sap_working_pu",
    "sap_working_nopull",
    "sap_working_out_0",
    "sap_working_out_1"
};

static struct pctrl_state_range pctrl_state_off_range = {
    .status = SAP_STATUS_OFF,
    .start = PCTRL_STATE_SAP_OFF,
    .end = PCTRL_STATE_SAP_OFF,
};

static struct pctrl_state_range pctrl_state_download_range = {
    .status = SAP_STATUS_DOWNLOAD,
    .start = PCTRL_STATE_SAP_DOWNLOAD_PD,
    .end = PCTRL_STATE_SAP_DOWNLOAD_PU,
};

static struct pctrl_state_range pctrl_state_working_range = {
    .status = SAP_STATUS_WORKING,
    .start = PCTRL_STATE_SAP_WORKING_PD,
    .end = PCTRL_STATE_SAP_WORKING_OUT_1,
};

static struct pinctrl *pctrl = NULL;
static struct pinctrl_state *states[PINCTRL_STATE_NUM] = {0};
static int states_init[PINCTRL_STATE_NUM] = {0};

static int mtk_gpio_sap_ctrl_set_status(enum sap_status status)
{
    int ret = 0;
    int ret_1 = 0;
    int i = 0;
    struct pctrl_state_range *range = NULL;

    if (status == SAP_STATUS_OFF)
        range = &pctrl_state_off_range;
    else if (status == SAP_STATUS_DOWNLOAD)
        range = &pctrl_state_download_range;
    else if (status == SAP_STATUS_WORKING)
        range = &pctrl_state_working_range;
    else {
        pr_info("[%s:%d][%d]: unsupport sap_state %d\n",
            GPIO_DEV_NODE_NAME,
            __LINE__,
            status);
        return -1;
    }

    for (i = range->start; i <= range->end; i++) {
        if (states_init[i] == 0)
            continue;
        ret_1 = pinctrl_select_state(pctrl, states[i]);
        if(ret_1)
            pr_info("[%s:%d][%d]: switch state %s fail\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                ret_1,
                names[i]);
        else
            pr_info("[%s:%d]: switch state %s pass\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                names[i]);
        ret += ret_1;
    }

    pr_info("[%s:%d] set sap_status %d ret %d\n",
            GPIO_DEV_NODE_NAME,
            __LINE__,
            status,
            ret);

    return ret;
}

int mtk_gpio_sap_ctrl_set_off(void)
{
    int ret = 0;
    ret += mtk_gpio_sap_ctrl_set_status(SAP_STATUS_OFF);
    /* call other module's SAP_STATUS_OFF control */

    return ret;
}
EXPORT_SYMBOL(mtk_gpio_sap_ctrl_set_off);

int mtk_gpio_sap_ctrl_set_download(void)
{
    int ret = 0;
    ret += mtk_gpio_sap_ctrl_set_status(SAP_STATUS_DOWNLOAD);
    /* call other module's SAP_STATUS_DOWNLOAD control */

    return ret;
}
EXPORT_SYMBOL(mtk_gpio_sap_ctrl_set_download);

int mtk_gpio_sap_ctrl_set_working(void)
{
    int ret = 0;
    ret += mtk_gpio_sap_ctrl_set_status(SAP_STATUS_WORKING);
    /* call other module's SAP_STATUS_WORKING control */

    return ret;
}
EXPORT_SYMBOL(mtk_gpio_sap_ctrl_set_working);

#if GPIO_SAP_CTRL_DEBUG
static ssize_t gpio_sap_ctrl_debug_show(struct device *dev,
                                        struct device_attribute *attr,
                                        char *buf)
{
    return snprintf(buf, 64, "[%s:%d] show\n",
                    GPIO_DEV_NODE_NAME,
                    __LINE__);
}

static ssize_t gpio_sap_ctrl_debug_store(struct device *dev,
                                        struct device_attribute *attr,
                                        const char *buf,
                                        size_t size)
{
    enum sap_status status;
    int ret = 0;
    status = (enum sap_status)(buf[0] - '0');

    if( status > SAP_STATUS_MIN && status < SAP_STATUS_MAX) {
        pr_info("[%s:%d] set enum sap_status %d\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                status);
        ret = mtk_gpio_sap_ctrl_set_status(status);
        pr_info("[%s:%d][%d] set enum sap_status %d\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                ret,
                status);
    } else
        pr_info("[%s:%d] unsupport sap_status %d\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                status);

    return size;
}

static DEVICE_ATTR_RW(gpio_sap_ctrl_debug);

int gpio_sap_ctrl_debug_init(struct platform_device *pdev)
{
    int ret = 0;
    ret = device_create_file(&pdev->dev, &dev_attr_gpio_sap_ctrl_debug);
    dev_info(&pdev->dev, "create debug file ret %d\n", ret);
    return ret;
}
void gpio_sap_ctrl_debug_deinit(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &dev_attr_gpio_sap_ctrl_debug);
    dev_info(&pdev->dev, "remove debug file \n");
}

#else
int gpio_sap_ctrl_debug_init(struct platform_device *pdev) { return 0;}
int gpio_sap_ctrl_debug_deinit(struct platform_device *pdev) { return 0;}
#endif

static int mtk_gpio_sap_ctrl_probe(struct platform_device *pdev)
{
	int ret = 0;
    int i = 0;
	dev_info(&pdev->dev, "probe in\n");

    pctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR_OR_NULL(pctrl)) {
        ret = -1;
        dev_info(&pdev->dev,
                    "[%s:%d][%d]: pinctrl undefined\n",
                    GPIO_DEV_NODE_NAME,
                    __LINE__,
                    ret);
    } else {
        for (i = 0; i < PINCTRL_STATE_NUM; i++) {
            states[i] = pinctrl_lookup_state(pctrl, names[i]);
            if (IS_ERR_OR_NULL(states[i])) {
                dev_info(&pdev->dev, "no state %s\n", names[i]);
                states_init[i] = 0;
            } else
                states_init[i] = 1;
        }
    }

	mtk_gpio_sap_ctrl_set_off();
    gpio_sap_ctrl_debug_init(pdev);
	dev_info(&pdev->dev, "probe done\n");
	return ret;
}

static int mtk_gpio_sap_ctrl_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "mtk_gpio_sap_ctrl removing\n");
    gpio_sap_ctrl_debug_deinit(pdev);
    devm_pinctrl_put(pctrl);
	dev_info(&pdev->dev, "mtk_gpio_sap_ctrl remove done\n");
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mtk_gpio_sap_ctrl_of_ids[] = {
	{.compatible = "mediatek,gpio_sap_ctrl",},
	{},
};
MODULE_DEVICE_TABLE(of, mtk_gpio_sap_ctrl_of_ids);
#endif

static struct platform_driver gpio_sap_ctrl = {
	.driver = {
		.name = GPIO_DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(mtk_gpio_sap_ctrl_of_ids),
	},
	.probe = mtk_gpio_sap_ctrl_probe,
	.remove = mtk_gpio_sap_ctrl_remove,
};

static int __init mtk_gpio_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&gpio_sap_ctrl);
	if (ret < 0) {
		pr_info("[%s:%d][%d]: Register platform driver fail\n",
                GPIO_DEV_NODE_NAME,
                __LINE__,
                ret);
		return ret;
	}
	pr_info("[%s:%d][%d]: Register platform driver done\n",
            GPIO_DEV_NODE_NAME,
            __LINE__,
            ret);
	return ret;
}

static void __exit mtk_gpio_exit(void)
{
	pr_info("[%s:%d]: exit platform driver\n", GPIO_DEV_NODE_NAME, __LINE__);
	platform_driver_unregister(&gpio_sap_ctrl);
}

module_init(mtk_gpio_init);
module_exit(mtk_gpio_exit);

MODULE_DESCRIPTION("GPIO Control For sAP In Differenct State");
MODULE_AUTHOR("Pei-Huai Ciou");
MODULE_LICENSE("GPL");
