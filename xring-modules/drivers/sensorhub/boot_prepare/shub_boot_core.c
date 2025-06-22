// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <soc/xring/sensorhub/shub_notifier.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>
#include "shub_private_dts.h"
#include "soc/xring/sensorhub/sensor_power/shub_sensor_power.h"
#include <linux/delay.h>

#define SH_SHM_PRINTF_NUM       0x36
#define SH_SWITCH_ON            0xAB34
#define BOARD_APP_INIT_END      0xCB12
#define SH_SYS_STATUS_BIT       29

enum {
	GET_BSP_DTS_ADDR = 1,
	GET_APP_DTS_ADDR,
	GET_DTS_INFO,
	GET_SHUB_BOOT_STATE,
	GET_SHUB_SWITCH_STATE
};

static atomic_t g_shub_boot_state = ATOMIC_INIT(0);
static atomic_t g_shub_dts_bringup_state = ATOMIC_INIT(0);

bool is_shub_boot_done(void)
{
	uint32_t sh_sys_status_val1;
	uint32_t sh_sys_status_val2;
	bool sh_sys_status;
	uint32_t *shmem_sh_sys_status_addr = NULL;

	if (!is_shub_dts_enable())
		return false;

	shmem_sh_sys_status_addr = (uint32_t *)ioremap(ACPU_LPIS_ACTRL + LPIS_ACTRL_SH_DFX_SYS_STATUS, 4);
	if (shmem_sh_sys_status_addr == NULL) {
		pr_err("ioremap shub mem failed\n");
		return false;
	}

	sh_sys_status_val1 = readl(shmem_sh_sys_status_addr) & (1 << SH_SYS_STATUS_BIT);
	udelay(3);
	sh_sys_status_val2 = readl(shmem_sh_sys_status_addr) & (1 << SH_SYS_STATUS_BIT);
	sh_sys_status = sh_sys_status_val1 & sh_sys_status_val2 ? true : false;

	iounmap(shmem_sh_sys_status_addr);

	atomic_read(&g_shub_boot_state);
	return ((g_shub_boot_state.counter == BOARD_APP_INIT_END) && sh_sys_status) ? true : false;
}
EXPORT_SYMBOL(is_shub_boot_done);

bool is_shub_dts_enable(void)
{
	atomic_read(&g_shub_dts_bringup_state);

	return (g_shub_dts_bringup_state.counter == SH_SWITCH_ON) ? true : false;
}
EXPORT_SYMBOL(is_shub_dts_enable);

static int shub_boot_atomic_cb(struct notifier_block *nb, unsigned long action, void *data)
{
	pr_warn("boot_state_cb action = 0x%lx\n", action);
	atomic_set(&g_shub_boot_state, (int)action);
	return 0;
}

static struct notifier_block g_boot_state_notifier = {
	.notifier_call = shub_boot_atomic_cb,
	.priority = 1,
};

static int shub_boot_prepare_drv_init(void)
{
	int ret = 0;
	struct device_node *shub_node = NULL;

	shub_node = of_find_compatible_node(NULL, NULL, "xring,sensorhub");
	if (!shub_node) {
		pr_err("sensorhub node is not found\n");
		ret = -EOPNOTSUPP;
		goto _err_node;
	}

	if (!of_device_is_available(shub_node)) {
		pr_err("%s,status property is not okay\n", __func__);
		ret = -EOPNOTSUPP;
		goto _err_node;
	}

	atomic_set(&g_shub_dts_bringup_state, SH_SWITCH_ON);
	shub_shm_addr_map();
	shub_run_bsp_dts_cb();
	sensor_power_init();
	sensor_power_on_all();
	ret = shub_boot_atomic_notifier_register(&g_boot_state_notifier);
	if (ret < 0)
		pr_err("notifier register failed,ret=0x%x!\n", ret);

	pr_info("%s end\n", __func__);

_err_node:
	return ret;
}

static void shub_boot_prepare_drv_exit(void)
{
	int ret = 0;

	sensor_power_off_all();
	sensor_power_deinit();
	shub_shm_addr_unmap();
	ret = shub_boot_atomic_notifier_unregister(&g_boot_state_notifier);
	if (ret < 0)
		pr_err("notifier unregister failed!,ret=0x%x\n", ret);

	pr_info("%s exit\n", __func__);
}

module_init(shub_boot_prepare_drv_init);
module_exit(shub_boot_prepare_drv_exit);
MODULE_LICENSE("Dual BSD/GPL");
