// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)            "[shub_reset]:" fmt

#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <soc/xring/sensorhub/shub_notifier.h>
#include <soc/xring/sensorhub/ipc/shub_ipc_drv.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_cmds_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dfx.h>
#include "firq.h"
#include "shub_util.h"
#include "shub_dfx_priv.h"

#define READ_FLAG_TIMES        (200)

#define SH_OUT_INTR_NODE_NAME  ("xring,sh_out_intr")
#define SH_WDT_INTR_NAME       ("sh_wdt")
#define SH_DFX_WORKQUEUE_NAME  ("shub_dfx_wq")
#define SH_LOCKUP_INTR_NAME    ("sh_lockup")
#define WAIT_SLEEP_MIN         (10000)
#define WAIT_SLEEP_MAX         (15000)
#define RESEND_TIMES           (10)
#define MASK_ALL_NSINTR        (NSINTR_MASK_ALL_USED | NSINTR_MASK_BM_SET)

enum {
	SH_HARDFAULT_NUM = 3,
	SH_MEMFAULT_NUM = 4,
	SH_BUSFAULT_NUM = 5,
	SH_USGFAULT_NUM = 6
};

enum {
	SH_STATE_DEFAULT = 0,
	SH_STATE_RESET_BEGIN = 1,
	SH_STATE_RESET_END = 2
};

static int sh_dfx_sh_ipc_cb(struct ipc_pkt *pkt, uint16_t tag, uint8_t cmd, void *arg);
struct sh_dfx_work_struct {
	struct work_struct dfx_work;
	unsigned long action;
};

static uint8_t *g_sh_nmi_base_addr;
static uint8_t *g_sh_wdt_config_addr;
static uint8_t *g_sh_actrl_dfx_sys_addr;
static atomic_t g_sh_modid = ATOMIC_INIT(MODID_SHUB_EXCEPTION_START);
static atomic_t g_sh_reset_state = ATOMIC_INIT(SH_STATE_DEFAULT);
static int g_sh_wdt_irq_num = -1;
static int g_sh_lockup_irq_num = -1;
static struct workqueue_struct *g_sh_dfx_wq;
static struct ipc_notifier_info g_sh_dfx_msg = {
	.callback = sh_dfx_sh_ipc_cb,
	.arg = NULL,
};

static void enable_sh_out_intr_mask(void)
{
	writel(MASK_ALL_NSINTR, g_sh_nmi_base_addr + NSINTR_MASK_OFFSET);
}

static void send_nmi_to_sh(void)
{
	writel(SH_CFG_SW_NMI_TRIGGER, g_sh_nmi_base_addr);
}

static uint32_t read_nmi_flag_reg(void)
{
	return readl(g_sh_nmi_base_addr + NMI_FLAG_OFFSET);
}

static void sh_wdt_restart(void)
{
	writel(SH_WDT_REST_VAL, g_sh_wdt_config_addr + SH_WDT_CRR_OFFSET);
}

static uint32_t read_actrl_dfx(uint32_t offset)
{
	return readl(g_sh_actrl_dfx_sys_addr + offset);
}

static void sh_recovery_action_worker(struct work_struct *work)
{
	int ret = 0;
	unsigned long action;
	struct sh_dfx_work_struct *data;

	data = container_of(work, struct sh_dfx_work_struct, dfx_work);
	action = data->action;

	if (action == ACTION_SH_RECOVERY_OS_READY)
		atomic_set_release(&g_sh_reset_state, SH_STATE_RESET_END);

	ret = sh_recovery_notifier_call_chain(action);
	pr_info("call recovery chain ret = %d, action=0x%lx\n", ret, action);
	kfree(data);

	if (action == ACTION_SH_RECOVERY_ALL_READY) {
		for (int i = 0; i < RESEND_TIMES; i++) {
			ret = sh_ipc_send_msg_sync(SH_DFX_AP_SH_VC_ID, TAG_SH_PM_AP_SR, CMD_SH_PM_AP_SR_ON, NULL, 0);
			if (ret == -EAGAIN || ret == -EBUSY) {
				usleep_range(1000, 1500);
				continue;
			} else {
				if (ret < 0)
					pr_err("recovery pm ipc failed, ret=%d\n", ret);
				break;
			}
		}
	}
}

int recovery_msg_handler(unsigned long action)
{
	bool ret;
	struct sh_dfx_work_struct *data;

	pr_warn("recovery handler action=0x%lx\n", action);
	if (g_sh_dfx_wq == NULL)
		return -EINVAL;

	data = kzalloc(sizeof(struct sh_dfx_work_struct), GFP_ATOMIC);
	if (data == NULL)
		return -ENOMEM;

	data->action = action;
	INIT_WORK(&data->dfx_work, sh_recovery_action_worker);
	ret = queue_work(g_sh_dfx_wq, &data->dfx_work);
	pr_warn("recovery handler exit ret=%d\n", ret);
	return 0;
}
EXPORT_SYMBOL(recovery_msg_handler);

static int sh_dfx_sh_ipc_cb(struct ipc_pkt *pkt, uint16_t tag, uint8_t cmd, void *arg)
{
	int ret = 0;

	pr_info("get sh ipc tag = %#X, cmd = %#X!\n", tag, cmd);

	if (tag != TAG_SH_DFX) {
		pr_warn("tag=%u not dfx tag\n", tag);
		return -1;
	}

	pr_info("call work queue\n");
	switch (cmd) {
	case CMD_SH_DFX_RECOVERY:
		ret = recovery_msg_handler(ACTION_SH_RECOVERY_OS_READY);
		break;
	case CMD_SH_DFX_ALARM:
		sh_dmd_report(pkt);
		break;
	default:
		pr_warn("unknown dfx cmd:%u\n", cmd);
	}

	return ret;
}

static void sh_reset_notifier_action(void)
{
	int ret = 0;

	ret = sh_reset_notifier_call_chain(ACTION_SH_RESET);
	pr_info("call reset chain ret = %d!\n", ret);
}

static void send_ipc_to_lp(int cmd_type)
{
	int ret = 0;

	pr_info("send ipc to lpcore and cmd_type = %d!\n", cmd_type);
	ret = ap_ipc_send_sync(SH_DFX_AP_LP_VC_ID, TAG_LP_SH_RST, cmd_type, NULL, 0);
	pr_info("send ipc ret = %d!\n", ret);
}

static int query_nmi_done(uint32_t value, uint16_t try_times)
{
	int ret = -1;
	uint32_t value_flag_done;

	do {
		value_flag_done = read_nmi_flag_reg() & SH_FLAG_LOGS_MASK;
		if (value_flag_done == value) {
			ret = 0;
			break;
		}
		usleep_range(WAIT_SLEEP_MIN, WAIT_SLEEP_MAX);
	} while ((try_times > 0) && (try_times--));

	return ret;
}

void sh_notify_to_reset(void)
{
	pr_info("call shub reset.\n");
	atomic_read_acquire(&g_sh_reset_state);
	if (g_sh_reset_state.counter == SH_STATE_RESET_BEGIN) {
		pr_warn("call reset, but shub reset is running, ignore!\n");
		return;
	}

	enable_sh_out_intr_mask();
	sh_wdt_restart();
	sh_reset_notifier_action();

	atomic_set_release(&g_sh_reset_state, SH_STATE_RESET_BEGIN);
	send_ipc_to_lp(CMD_LP_SH_RST);
}

static irqreturn_t sh_reset_thread(int irq, void *data)
{
	pr_info("thread go, irq num = %d\n", irq);
	atomic_read_acquire(&g_sh_modid);
	sh_mdr_error_notify(g_sh_modid.counter);
	return IRQ_HANDLED;
}

void sh_trigger_loop_dump(char *path)
{
	uint16_t time_out = READ_FLAG_TIMES;
	int ret = -1;
	uint32_t var_dfx = read_actrl_dfx(0);
	uint32_t var_flag = read_actrl_dfx(SH_RESET_FLAG_OFFSET);
	uint32_t var_status = read_actrl_dfx(SH_STATUS_OFFSET);

	pr_info("call shub dump, dfx:%#x, flag:%#x, status:%#x.\n", var_dfx, var_flag, var_status);
	atomic_read_acquire(&g_sh_reset_state);
	if (g_sh_reset_state.counter == SH_STATE_RESET_BEGIN) {
		pr_warn("call dump, but shub reset is running, ignore!\n");
		return;
	}
	enable_sh_out_intr_mask();
	sh_wdt_restart();
	send_nmi_to_sh();

	pr_info("nmi flag reg=%#x after ap send nmi.\n", read_nmi_flag_reg());
	ret = query_nmi_done(SH_LOGS_DONE_SH, time_out);
	pr_info("nmi flag reg=%#x after sh save logs.\n", read_nmi_flag_reg());

	if (ret != 0) {
		pr_warn("send ipc to lpcore to save log!\n");
		send_ipc_to_lp(CMD_LP_SH_LOG);
		ret = query_nmi_done(SH_LOGS_DONE_LP, time_out);
		pr_info("nmi flag reg=%#x after lp save logs.\n", read_nmi_flag_reg());
	}
	sh_notify_hal_savelogs(SH_SAVE_TYPE_ALL, path);
	usleep_range(WAIT_SLEEP_MIN, WAIT_SLEEP_MAX);
	pr_info("loop ret = %d!\n", ret);
}

static int sh_get_fault_errcode(uint32_t irq_num)
{
	int ret = MODID_SHUB_WDT;

	switch (irq_num) {
	case SH_HARDFAULT_NUM:
		ret = MODID_SHUB_HARDFAULT;
		break;
	case SH_MEMFAULT_NUM:
		ret = MODID_SHUB_MEMFAULT;
		break;
	case SH_BUSFAULT_NUM:
		ret = MODID_SHUB_BUSFAULT;
		break;
	case SH_USGFAULT_NUM:
		ret = MODID_SHUB_USGFAULT;
		break;
	default:
		pr_warn("no related errcode for sh irq=%u, use default errcode!\n", irq_num);
	}

	return ret;
}

static irqreturn_t sh_wdt_handler(int irq, void *data)
{
	uint32_t value_irq;

	pr_debug("sh_wdt_irq trigger, irq num = %d !\n", irq);
	enable_sh_out_intr_mask();
	sh_wdt_restart();

	atomic_read_acquire(&g_sh_reset_state);
	if (g_sh_reset_state.counter == SH_STATE_RESET_BEGIN)
		atomic_set_release(&g_sh_reset_state, SH_STATE_RESET_END);

	value_irq = read_nmi_flag_reg() & SH_FLAG_IRQ_NUM_MASK;
	atomic_set_release(&g_sh_modid, sh_get_fault_errcode(value_irq));
	return IRQ_WAKE_THREAD;
}

static irqreturn_t sh_lockup_handler(int irq, void *data)
{
	uint32_t var = read_actrl_dfx(SH_RESET_FLAG_OFFSET);

	pr_debug("sh_lockup_irq trigger, irq num = %d !\n", irq);
	if (var == FLAG_SH_RESET_POWER_DOWN) {
		pr_warn("sh power down, ig rst\n");
		return IRQ_HANDLED;
	}
	enable_sh_out_intr_mask();
	sh_wdt_restart();
	atomic_read_acquire(&g_sh_reset_state);
	if (g_sh_reset_state.counter == SH_STATE_RESET_BEGIN)
		atomic_set_release(&g_sh_reset_state, SH_STATE_RESET_END);

	atomic_set_release(&g_sh_modid, (int)(MODID_SHUB_LOCKUP));
	return IRQ_WAKE_THREAD;
}

static int sh_firq_cb(uint32_t buf_addr, uint32_t buf_size)
{
	sh_notify_hal_savelogs(SH_SAVE_TYPE_LOG, NULL);
	return 0;
}

static int sh_dfx_wq_init(void)
{
	g_sh_dfx_wq = create_singlethread_workqueue(SH_DFX_WORKQUEUE_NAME);
	if (g_sh_dfx_wq == NULL) {
		pr_err("dfx create wq failed\n");
		return -EAGAIN;
	}
	return 0;
}

static int sh_dfx_ipc_register(void)
{
	int ret;

	ret = sh_register_recv_notifier(SH_DFX_AP_SH_VC_ID, TAG_SH_DFX, &g_sh_dfx_msg);
	if (ret != 0)
		pr_err("sh ipc register ret = %d!\n", ret);

	return ret;
}

static int sh_reset_init(void)
{
	int ret = 0;
	struct device_node *sh_irq_node = NULL;
	uint32_t irqflags = IRQF_TRIGGER_HIGH | IRQF_ONESHOT;

	pr_info("init go!\n");

	g_sh_nmi_base_addr = (uint8_t *)ioremap(SH_CFG_NMI_INTR_ADDR, SH_CFG_DFX_REGS_SIZE);
	if (g_sh_nmi_base_addr == NULL) {
		pr_err("%s sh nmi ioremap fail!\n", __func__);
		ret = -EINVAL;
		goto _err_nmi_map;
	}

	g_sh_wdt_config_addr = (uint8_t *)ioremap(SH_WDT_CRR_ADDR, SH_WDT_CRR_REGS_SIZE);
	if (g_sh_wdt_config_addr == NULL) {
		pr_err("%s sh wdt ioremap fail!\n", __func__);
		ret = -EINVAL;
		goto _err_wdt_map;
	}

	g_sh_actrl_dfx_sys_addr = (uint8_t *)ioremap(LPIS_ACTRL_DFX_SYS_ADDR, LPIS_ACTRL_DFX_REGS_SIZE);
	if (g_sh_actrl_dfx_sys_addr == NULL) {
		pr_err("%s sh actrl ioremap fail!\n", __func__);
		ret = -EINVAL;
		goto _err_actrl_map;
	}

	sh_irq_node = of_find_compatible_node(NULL, NULL, SH_OUT_INTR_NODE_NAME);
	if (sh_irq_node == NULL) {
		pr_err("%s of_find_compatible_node fail!\n", __func__);
		ret = -EINVAL;
		goto _err_node;
	}

	g_sh_wdt_irq_num = of_irq_get(sh_irq_node, 1);
	g_sh_lockup_irq_num = of_irq_get(sh_irq_node, 0);
	if ((g_sh_wdt_irq_num <= 0) || (g_sh_lockup_irq_num <= 0)) {
		pr_err("fail wdt=%d, lockup=%d!\n", g_sh_wdt_irq_num, g_sh_lockup_irq_num);
		ret = -EINVAL;
		goto _err_node;
	}

	if (request_threaded_irq(g_sh_wdt_irq_num, sh_wdt_handler, sh_reset_thread,
				irqflags, SH_WDT_INTR_NAME, NULL)) {
		pr_err("%s failure requesting sh_wdt irq!\n", __func__);
		ret = -EINVAL;
		goto _err_node;
	}

	if (request_threaded_irq(g_sh_lockup_irq_num, sh_lockup_handler, sh_reset_thread,
				irqflags, SH_LOCKUP_INTR_NAME, NULL)) {
		pr_err("%s failure requesting sh_lockup irq!\n", __func__);
		ret = -EINVAL;
		goto _err_req;
	}

	ret = sh_dfx_wq_init();
	if (ret < 0)
		goto _err_wq_init;

	ret = sh_dfx_ipc_register();
	if (ret != 0)
		goto _err_recovery_init;

	ret = sh_mdr_register();
	if (ret < 0)
		pr_warn("mdr reg ret = %d!\n", ret);

	ret = sh_dmd_register();
	if (ret < 0)
		pr_warn("dmd reg ret = %d!\n", ret);

	ret = sh_log_dev_init();
	if (ret < 0)
		pr_warn("log dev init ret = %d!\n", ret);

	ret = firq_recv_cb_register(SH_DFX_FIRQ_CHANNEL, sh_firq_cb);
	if (ret < 0)
		pr_warn("firq register ret = %d!\n", ret);

	pr_info("init success !\n");
	return 0;

_err_recovery_init:
	destroy_workqueue(g_sh_dfx_wq);
_err_wq_init:
	free_irq(g_sh_lockup_irq_num, NULL);
_err_req:
	free_irq(g_sh_wdt_irq_num, NULL);
_err_node:
	iounmap(g_sh_actrl_dfx_sys_addr);
_err_actrl_map:
	iounmap(g_sh_wdt_config_addr);
_err_wdt_map:
	iounmap(g_sh_nmi_base_addr);
_err_nmi_map:
	return ret;
}

static void sh_reset_exit(void)
{
	int ret = 0;

	pr_info("sh reset exit\n");
	destroy_workqueue(g_sh_dfx_wq);
	free_irq(g_sh_wdt_irq_num, NULL);
	free_irq(g_sh_lockup_irq_num, NULL);
	iounmap(g_sh_nmi_base_addr);
	iounmap(g_sh_wdt_config_addr);
	iounmap(g_sh_actrl_dfx_sys_addr);
	ret = sh_unregister_recv_notifier(SH_DFX_AP_SH_VC_ID, TAG_SH_DFX, &g_sh_dfx_msg);
	pr_info("call ipc unreg ret = %d!\n", ret);
	ret = firq_recv_cb_unregister(SH_DFX_FIRQ_CHANNEL);
	pr_info("call firq unreg ret = %d!\n", ret);
	sh_mdr_unregister();
	sh_dmd_unregister();
	sh_log_dev_exit();
}

module_init(sh_reset_init);
module_exit(sh_reset_exit);
MODULE_LICENSE("Dual BSD/GPL");
