// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <soc/xring/ipc/xr_rproc.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_lpctrl_ap.h>
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include "adsp_boot_driver.h"
#include "adsp_ssr_driver.h"
#include "../common/fk-audio-ipc.h"
#include "../acore/fk-acore-define.h"
#include "../common/fk-audio-log.h"
#include "../common/fk-audio-mailbox.h"
#include "../common/fk-common-ctrl.h"
#include "xrisp_common.h"

static struct xr_adsp_ssr_dev g_adsp_ssr_dev;

static struct adsp_mdr_item adsp_mdr_talbe[] = {
	{.val = 0, .mdr_exception_type = MODID_ADSP_ILLEGAL_INSTRUCTION},
	{.val = 2, .mdr_exception_type = MODID_ADSP_FETCH_ERR},
	{.val = 3, .mdr_exception_type = MODID_ADSP_LOAD_ERR},
	{.val = 6, .mdr_exception_type = MODID_ADSP_DIVIDE_BY_ZERO},
	{.val = 7, .mdr_exception_type = MODID_ADSP_ILLEGAL_NEXT_PC_VALUE},
	{.val = 9, .mdr_exception_type = MODID_ADSP_UNALIGNED_LOAD},
	{.val = 20, .mdr_exception_type = MODID_ADSP_MEM_NOT_FETCH},
	{.val = 28, .mdr_exception_type = MODID_ADSP_MEM_NOT_LOAD},
	{.val = 29, .mdr_exception_type = MODID_ADSP_MEM_NOT_STORE},
	{.val = 32, .mdr_exception_type = MODID_ADSP_WRITE_ERR},
	{.val = 33, .mdr_exception_type = MODID_ADSP_WDT},
	{.val = 34, .mdr_exception_type = MODID_ADSP_IPC_TIMEOUT},
	{.val = 35, .mdr_exception_type = MODID_ADSP_VOTEMSG_ERROR},
};

static uint32_t get_adsp_mdr_num_by_type(int subtype)
{
	int table_size = 0;
	int i = 0;

	table_size = sizeof(adsp_mdr_talbe) / sizeof(struct adsp_mdr_item);
	for (i = 0; i < table_size; i++) {
		if (adsp_mdr_talbe[i].val == subtype)
			return adsp_mdr_talbe[i].mdr_exception_type;
	}

	return MODID_ADSP_OTHER;
}

int get_adsp_reset_switch(void)
{
	return g_adsp_ssr_dev.reset_switch;
}

void set_adsp_reset_switch(bool enable)
{
	g_adsp_ssr_dev.reset_switch = enable;
}

bool get_adsp_reset_process(void)
{
	return g_adsp_ssr_dev.adsp_reset_process;
}

static int fk_adsp_uevent_notify_user(const char *euvent_info)
{
	struct device *dev = &g_adsp_ssr_dev.pdev->dev;
	char buf[ADSP_SSR_UEVENT_BUF_MAXLEN];
	char *envp_ext[2];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, ADSP_SSR_UEVENT_BUF_MAXLEN, "audio_ssr=%s", euvent_info);
	envp_ext[0] = buf;
	envp_ext[1] = NULL;
	kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp_ext);
	AUD_LOG_INFO(AUD_DSP, "uevent notify: %s", envp_ext[0]);
	return 0;
}

static void adsp_clear_mark_reg(void)
{
	unsigned int adsp_mark_reg;

	adsp_mark_reg = adsp_reg_read32(g_adsp_ssr_dev.adsp_mark_reg);
	adsp_mark_reg &= ~1;
	adsp_reg_write32(g_adsp_ssr_dev.adsp_mark_reg, adsp_mark_reg);
}

static void adsp_reset_func(void)
{
	int ret;
	int index = 0;
	int adsp_a_flag;
	int adsp_power_status;
	int lpcore_notify = 1;

	AUD_LOG_INFO(AUD_DSP, "adsp reset work start");

	/* 1. adsp poweroff */
	adsp_power_status = get_adsp_power_status();
	if (adsp_power_status == true) {
		ret = adsp_poweroff();
		if (ret < 0) {
			AUD_LOG_ERR(AUD_DSP, "acpu vote off adsp failed, ret = %d", ret);
			return;
		}
	}

	/* 2. send ipc to lpcore */
	ret = ap_ipc_send_sync(IPC_VC_PERI_NS_AP_LP_COMMON_0, TAG_LP_AUDIO_RST, 0,
		&lpcore_notify, sizeof(int));
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "send ipc msg to lpcore failed");
		return;
	}

	/* 3. wait lpcore reset done */
	index = 0;
	adsp_a_flag = adsp_reg_read32(g_adsp_ssr_dev.adsp_mark_reg) & 0x1;  /* 0x1 the first bit*/
	while (!adsp_a_flag) {
		usleep_range(1000, 2000);
		index++;
		/* 0x1 the first bit*/
		adsp_a_flag = adsp_reg_read32(g_adsp_ssr_dev.adsp_mark_reg) & 0x1;
		if (index >= 50) {
			AUD_LOG_ERR(AUD_DSP, "lpcore not response adsp reset request");
			return;
		}
	}

	/* 4. clear adsp_mark_reg */
	adsp_clear_mark_reg();

	/* 5. clear adsp power status */
	adsp_reg_write32(g_adsp_ssr_dev.audsys_power_state, 0x0);

	/* 6. reload adsp img from load mem to running mem */
	ret = copy_adsp_img_to_running_memory();
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "reload adsp img failed");
		return;
	}

	/* 7. poweron adsp */
	if (adsp_power_status == true) {
		ret = adsp_poweron();
		if (ret < 0) {
			AUD_LOG_ERR(AUD_DSP, "acpu vote on adsp failed, please notice this problem");
			return;
		}
	}

	AUD_LOG_INFO(AUD_DSP, "adsp reset work finish");
}

static void adsp_recovery_init(void)
{
	atomic_set(&g_adsp_ssr_dev.ssr_client.ac_num, 0);
	INIT_LIST_HEAD(&g_adsp_ssr_dev.ssr_client.ac_list);
	mutex_init(&g_adsp_ssr_dev.ssr_client.list_lock);
}

static int adsp_ssr_pre_recovery(void)
{
	struct adsp_ssr_action *ssr_action = NULL;
	int user_fixed = 0;
	int index = 0;

	AUD_LOG_INFO(AUD_DSP, "adsp ssr pre recovery enter");

	mutex_lock(&g_adsp_ssr_dev.ssr_client.list_lock);

	if (atomic_read(&g_adsp_ssr_dev.ssr_client.ac_num) == 0) {
		AUD_LOG_DBG(AUD_DSP, "no action need to done");
		mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);
		return 0;
	}

	list_for_each_entry(ssr_action, &g_adsp_ssr_dev.ssr_client.ac_list, action_list) {
		if ((ssr_action == NULL) || (ssr_action->ops == NULL)) {
			AUD_LOG_INFO(AUD_DSP, "ssr_action is NULL or ssr_action_ops is NULL");
			continue;
		}
		if (ssr_action->ops->pre_recovery)
			ssr_action->ops->pre_recovery(ssr_action->priv);
	}

	mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);

	fk_adsp_reset_done_set(user_fixed);
	/*reset ipc fifo*/
	fk_audio_reset_ipc_fifo();

	fk_adsp_uevent_notify_user(ADSP_UEVENT_RESET_OPEN);

	user_fixed = fk_adsp_reset_done_query();
	while (!user_fixed) {
		usleep_range(1000, 2000);
		index++;

		user_fixed = fk_adsp_reset_done_query();
		if (index >= 5000) {
			AUD_LOG_ERR(AUD_DSP, "hal process adsp assert timeout");
			return -EAGAIN;
		}
	}

	AUD_LOG_INFO(AUD_DSP, "adsp ssr pre recovery exit");
	return 0;
}

static int adsp_ssr_post_recovery(void)
{
	struct adsp_ssr_action *ssr_action = NULL;

	AUD_LOG_INFO(AUD_DSP, "adsp ssr post recovery enter");

	mutex_lock(&g_adsp_ssr_dev.ssr_client.list_lock);
	if (atomic_read(&g_adsp_ssr_dev.ssr_client.ac_num) == 0) {
		AUD_LOG_DBG(AUD_DSP, "no action need to done");
		mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);
		return 0;
	}

	list_for_each_entry(ssr_action, &g_adsp_ssr_dev.ssr_client.ac_list, action_list) {
		if ((ssr_action == NULL) || (ssr_action->ops == NULL)) {
			AUD_LOG_INFO(AUD_DSP, "ssr_action is NULL or ssr_action_ops is NULL");
			continue;
		}
		if (ssr_action->ops->post_recovery)
			ssr_action->ops->post_recovery(ssr_action->priv);
	}
	mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);

	fk_adsp_uevent_notify_user(ADSP_UEVENT_RESET_CLOSE);
	g_adsp_ssr_dev.adsp_reset_process = false;

	AUD_LOG_INFO(AUD_DSP, "adsp ssr post recovery exit");
	return 0;
}

struct adsp_ssr_action *fk_adsp_ssr_register(struct recovery_ops *ops, void *priv)
{
	struct adsp_ssr_action *ssr_action = NULL;

	ssr_action = kzalloc(sizeof(struct adsp_ssr_action), GFP_KERNEL);
	if (!ssr_action) {
		AUD_LOG_ERR(AUD_DSP, "alloc struct fail.");
		return NULL;
	}

	ssr_action->ops = ops;
	ssr_action->priv = priv;
	mutex_lock(&g_adsp_ssr_dev.ssr_client.list_lock);
	list_add_tail(&ssr_action->action_list, &g_adsp_ssr_dev.ssr_client.ac_list);
	atomic_inc(&g_adsp_ssr_dev.ssr_client.ac_num);
	mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);
	return ssr_action;
}
EXPORT_SYMBOL(fk_adsp_ssr_register);

int fk_adsp_ssr_unregister(struct adsp_ssr_action *action)
{
	struct adsp_ssr_action *ssr_action = NULL;
	struct adsp_ssr_action *ssr_action_temp = NULL;

	mutex_lock(&g_adsp_ssr_dev.ssr_client.list_lock);
	list_for_each_entry_safe(ssr_action, ssr_action_temp, &g_adsp_ssr_dev.ssr_client.ac_list, action_list) {
		if (ssr_action == action) {
			list_del(&ssr_action->action_list);
			atomic_dec(&g_adsp_ssr_dev.ssr_client.ac_num);
			AUD_LOG_INFO(AUD_DSP, "ac_num %d", atomic_read(&g_adsp_ssr_dev.ssr_client.ac_num));
			kfree(ssr_action);
			mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);
			return 0;
		}
	}
	mutex_unlock(&g_adsp_ssr_dev.ssr_client.list_lock);

	return -EINVAL;
}
EXPORT_SYMBOL(fk_adsp_ssr_unregister);

static void mdr_adsp_dump(u32 modid, u32 etype, u64 coreid, char *log_path,
				pfn_cb_dump_done pfn_cb, void *data)

{
	int ret;
	void *log_addr = ioremap_wc(AUDIO_DSP_UNCACHELOG_ADDR, AUDIO_DSP_UNCACHELOG_SIZE);
	void *mdr_addr = ioremap_wc(DFX_MEM_AUDIO_ADDR, DFX_MEM_AUDIO_SIZE);

	memset(mdr_addr, 0, DFX_MEM_AUDIO_SIZE);
	memcpy(mdr_addr, log_addr, AUDIO_DSP_UNCACHELOG_SIZE);

	ret = edr_drv_submit_api_sync(XRING_S_AUDIO, AUDIO_PANIC, EDR_LOGCAT, log_path);
	if (ret < 0)
		AUD_LOG_ERR(AUD_DSP, "catch logcat failed.");
	else
		AUD_LOG_INFO(AUD_DSP, "catch logcat success.");

	AUD_LOG_DBG(AUD_DSP, "mdr adsp dump");
}

static void mdr_adsp_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	AUD_LOG_DBG(AUD_DSP, "mdr adsp reset enter");

	if (g_adsp_ssr_dev.reset_switch) {
		adsp_ssr_pre_recovery();
		adsp_reset_func();
		adsp_ssr_post_recovery();
	}

	AUD_LOG_DBG(AUD_DSP, "mdr adsp reset exit");
}

static int adsp_register_mdr(void)
{
	int ret = 0;
	int i;

	g_adsp_ssr_dev.mdr_adsp_ops.ops_dump = mdr_adsp_dump;
	g_adsp_ssr_dev.mdr_adsp_ops.ops_reset = mdr_adsp_reset;
	g_adsp_ssr_dev.mdr_adsp_ops.ops_data = NULL;

	for (i = 0; i < ADSP_MDR_EXP_TYPE_MAX; i++) {
		ret = mdr_register_exception(&g_adsp_exp[i]);
		if (ret == 0) {
			AUD_LOG_ERR(AUD_DSP, "exception %d register fail, ret %d.", i, ret);
			return ret;
		}
	}

	ret = mdr_register_module_ops(MDR_AUDIO, &g_adsp_ssr_dev.mdr_adsp_ops, &g_adsp_ssr_dev.mdr_retinfo);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "module register fail, ret %d.", ret);
		return ret;
	}

	ret = mdr_filesys_write_log(MDR_AUDIO, "/proc/audio_dsp", "audio_dump.bin", 10 * 1024*1024);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_DSP, "mdr_filesys_write_log fail, ret %d.\n", ret);
		return ret;
	}

	return ret;
}

int adsp_reset(int subtype)
{
	uint32_t modid_adsp_exception;

	if (g_adsp_ssr_dev.adsp_reset_process == true)
		return 0;

	modid_adsp_exception = get_adsp_mdr_num_by_type(subtype);

	g_adsp_ssr_dev.adsp_reset_process = true;

	/* trigger adsp reset */
	mdr_system_error(modid_adsp_exception, 0, 0);

	return 0;
}

irqreturn_t adsp_wdt_timeout_inr_handler(int irq, void *dev_id)
{
	AUD_LOG_DBG(AUD_DSP, "acpu receive wdt timeout intr from adsp irq = %d", irq);

	/* 1.clear wdt timeout intr */
	adsp_reg_write32(g_adsp_ssr_dev.audio_sys_wdt_restart_reg, WDOG_COUNTER_RESTART_KICK_VALUE);

	/* 2.schedule adsp ssr reset work, 33 is wdt size */
	adsp_reset(33);

	return IRQ_HANDLED;
}

int adsp_send_debug_intr_reset(void)
{
	/*1. trigger debug intr to adsp, close mask & write INTR_FROM_ACPU reg*/
	adsp_reg_write32(g_adsp_ssr_dev.adsp_debug_intr_mask,
		AUDIO_SYS_INTR_DEBUG_FROM_ACPU_MASK_OFF);
	adsp_reg_write32(g_adsp_ssr_dev.adsp_debug_intr_trigger_reg,
		AUDIO_SYS_M_INTR_FROM_ACPU_VALUE);

	return 0;
}

int adsp_wdt_exception_injection(void)
{
	int ret;
	struct xring_aud_msg adsp_wdt_except_inject_msg = {0};

	adsp_wdt_except_inject_msg.header.scene_id = (uint8_t)ADSP_SCENE_NONE;
	adsp_wdt_except_inject_msg.header.ins_id = 0;
	adsp_wdt_except_inject_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	adsp_wdt_except_inject_msg.header.func_id = ADSP_DEBUG_CTRL_ASK_DSP_WDT;
	adsp_wdt_except_inject_msg.header.len = sizeof(struct xring_aud_msg_header);
	adsp_wdt_except_inject_msg.header.result = 0;

	ret = fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)&adsp_wdt_except_inject_msg,
		adsp_wdt_except_inject_msg.header.len, NULL, NULL);

	AUD_LOG_INFO(AUD_DSP, "trigger adsp wdt exception");
	return 0;
}

int adsp_panic_exception_injection(void)
{
	int ret;
	struct xring_aud_msg adsp_panic_except_inject_msg = {0};

	adsp_panic_except_inject_msg.header.scene_id = (uint8_t)ADSP_SCENE_NONE;
	adsp_panic_except_inject_msg.header.ins_id = 0;
	adsp_panic_except_inject_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	adsp_panic_except_inject_msg.header.func_id = ADSP_DEBUG_CTRL_ASK_DSP_ASSERT;
	adsp_panic_except_inject_msg.header.len = sizeof(struct xring_aud_msg_header);
	adsp_panic_except_inject_msg.header.result = 0;

	ret = fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)&adsp_panic_except_inject_msg,
		adsp_panic_except_inject_msg.header.len, NULL, NULL);

	AUD_LOG_INFO(AUD_DSP, "trigger adsp panic exception");
	return 0;
}

static int adsp_proc_show(struct seq_file *m, void *v)
{
	return 0;
}

static int adsp_dump_open(struct inode *inode, struct file *file)
{
	return single_open(file, adsp_proc_show, NULL);
}

static ssize_t adsp_dump_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	void *data_addr = ioremap_wc(AUDIO_RESERVED_MEM, AUDIO_RESERVED_SIZE);
	void *tcm_addr = ioremap_wc(ADSP_TCM_MEM, ADSP_TCM_SIZE);
	void __iomem *audsys_power_state = ioremap(AUDSYS_POWER_STATE, IOREMAP_REGISTER_SIZE);
	void __iomem *acpu2adsp_interupt_lock_reg = ioremap(ACPU2ADSP_INTERRPUT_LOCK,
			IOREMAP_REGISTER_SIZE);

	AUD_LOG_INFO(AUD_DSP, "enter, size %zu", size);

	if (ppos == NULL) {
		AUD_LOG_ERR(AUD_DSP, "ppos is null");
		return 0;
	}

	if (buf == NULL) {
		AUD_LOG_ERR(AUD_DSP, "buf is null");
		return 0;
	}

	if (data_addr == NULL) {
		AUD_LOG_ERR(AUD_DSP, "data_addr is NULL, ioremap_wc failed");
		return 0;
	}

	if (tcm_addr == NULL) {
		AUD_LOG_ERR(AUD_DSP, "tcm_addr is NULL, ioremap_wc failed");
		return 0;
	}

	if (audsys_power_state == NULL || acpu2adsp_interupt_lock_reg == NULL) {
		AUD_LOG_ERR(AUD_DSP, "audsys_power_state or audsys_power_state is NULL, ioremap failed");
		return 0;
	}

	adsp_reg_write32(acpu2adsp_interupt_lock_reg, 0x1);

	if ((adsp_reg_read32(audsys_power_state) != AUDSYS_POWERON_STATE) &&
		(adsp_reg_read32(audsys_power_state) != AUDSYS_PREPARE_POWERDOWN)) {
		AUD_LOG_ERR(AUD_DSP, "adsp sys is poweroff cannot dump\n");
		return 0;
	}

	if (copy_to_user(buf, data_addr, AUDIO_RESERVED_SIZE)) {
		AUD_LOG_ERR(AUD_DSP, "copy to user failed");
		return -EFAULT;
	}

	if (copy_to_user(buf + AUDIO_RESERVED_SIZE, tcm_addr, ADSP_TCM_SIZE)) {
		AUD_LOG_ERR(AUD_DSP, "copy to user failed");
		return -EFAULT;
	}

	AUD_LOG_INFO(AUD_DSP, "exit");

	return (AUDIO_RESERVED_SIZE + ADSP_TCM_SIZE);
}

static struct proc_ops adsp_proc_fops = {
	.proc_open      = adsp_dump_open,
	.proc_read      = adsp_dump_read,
	.proc_release   = single_release,
};

static int adsp_proc_node_init(void)
{
	struct proc_dir_entry *de;

	de = proc_create("audio_dsp", 0440, NULL, &adsp_proc_fops);
	if (!de) {
		AUD_LOG_ERR(AUD_DSP, "audio_dsp proc created failed");
		return -ENOENT;
	}

	return 0;
}

static int adsp_ssr_drv_probe(struct platform_device *pdev)
{
	int ret;

	struct xr_adsp_ssr_dev *adsp_ssr_dev = &g_adsp_ssr_dev;


	adsp_ssr_dev->reset_switch = true;

	adsp_ssr_dev->adsp_reset_process = false;

	ret = adsp_register_mdr();
	if (ret) {
		AUD_LOG_ERR(AUD_DSP, "adsp register mdr failed, ret = %d", ret);
		ret = -EINVAL;
		goto err0;
	}

	adsp_ssr_dev->irqno_wdt = platform_get_irq_optional(pdev, 0);
	if (adsp_ssr_dev->irqno_wdt < 0) {
		AUD_LOG_ERR(AUD_DSP, "cannot get irq, irq = %d", adsp_ssr_dev->irqno_wdt);
		ret = -IRQ_NONE;
		goto err0;
	}

	ret = request_irq(adsp_ssr_dev->irqno_wdt, adsp_wdt_timeout_inr_handler,
		IRQF_SHARED, pdev->name, &pdev->dev);
	if (ret) {
		AUD_LOG_ERR(AUD_DSP, "cannot request intr, irq = %d, ret = %d", adsp_ssr_dev->irqno_wdt, ret);
		ret = -IRQ_NONE;
		goto err0;
	}

	adsp_ssr_dev->audio_sys_wdt_restart_reg = ioremap(AUDIO_SYS_WDT_CRR_REG,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_ssr_dev->audio_sys_wdt_restart_reg) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDIO_SYS_WDT_CRR_REG");
		ret = -EINVAL;
		goto err0;
	}

	adsp_ssr_dev->adsp_mark_reg = ioremap(ADSP_MARK_REG, IOREMAP_REGISTER_SIZE);
	if (!adsp_ssr_dev->adsp_mark_reg) {
		AUD_LOG_ERR(AUD_DSP, "can't remap ADSP_MARK_REG");
		ret = -EINVAL;
		iounmap(adsp_ssr_dev->audio_sys_wdt_restart_reg);
		goto err0;
	}

	adsp_ssr_dev->adsp_debug_intr_trigger_reg = ioremap(AUDIO_SYS_M_INTR_FROM_ACPU_REG,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_ssr_dev->adsp_debug_intr_trigger_reg) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDIO_SYS_M_INTR_FROM_ACPU_REG");
		ret = -EINVAL;
		iounmap(adsp_ssr_dev->audio_sys_wdt_restart_reg);
		iounmap(adsp_ssr_dev->adsp_mark_reg);
		goto err0;
	}

	adsp_ssr_dev->adsp_debug_intr_mask = ioremap(AUDIO_SYS_M_INTR_FROM_ACPU_MASK,
		IOREMAP_REGISTER_SIZE);
	if (!adsp_ssr_dev->adsp_debug_intr_mask) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDIO_SYS_M_INTR_FROM_ACPU_MASK");
		ret = -EINVAL;
		iounmap(adsp_ssr_dev->audio_sys_wdt_restart_reg);
		iounmap(adsp_ssr_dev->adsp_mark_reg);
		iounmap(adsp_ssr_dev->adsp_debug_intr_trigger_reg);
		goto err0;
	}

	adsp_ssr_dev->audsys_power_state = ioremap(AUDSYS_POWER_STATE,
			IOREMAP_REGISTER_SIZE);
	if (!adsp_ssr_dev->audsys_power_state) {
		AUD_LOG_ERR(AUD_DSP, "can't remap AUDSYS_POWEROFF_STATE");
		ret = -EINVAL;
		iounmap(adsp_ssr_dev->audio_sys_wdt_restart_reg);
		iounmap(adsp_ssr_dev->adsp_mark_reg);
		iounmap(adsp_ssr_dev->adsp_debug_intr_trigger_reg);
		iounmap(adsp_ssr_dev->adsp_debug_intr_mask);
		goto err0;
	}

	adsp_proc_node_init();

	adsp_ssr_dev->pdev = pdev;

err0:
	return ret;
}

static int adsp_ssr_drv_remove(struct platform_device *pdev)
{
	int irq;

	irq = platform_get_irq_optional(pdev, 0);

	free_irq(irq, &pdev->dev);

	if (g_adsp_ssr_dev.audio_sys_wdt_restart_reg)
		iounmap(g_adsp_ssr_dev.audio_sys_wdt_restart_reg);
	if (g_adsp_ssr_dev.adsp_mark_reg)
		iounmap(g_adsp_ssr_dev.adsp_mark_reg);
	if (g_adsp_ssr_dev.adsp_debug_intr_trigger_reg)
		iounmap(g_adsp_ssr_dev.adsp_debug_intr_trigger_reg);
	if (g_adsp_ssr_dev.adsp_debug_intr_mask)
		iounmap(g_adsp_ssr_dev.adsp_debug_intr_mask);
	if (g_adsp_ssr_dev.audsys_power_state)
		iounmap(g_adsp_ssr_dev.audsys_power_state);

	return 0;
}

static const struct of_device_id adsp_wdt_of_match[] = {
	{ .compatible = "xring,adsp-wdt", },
	{ /* sentinel */ }
};

static struct platform_driver adsp_ssr_driver = {
	.probe     = adsp_ssr_drv_probe,
	.remove    = adsp_ssr_drv_remove,
	.driver    = {
		.name  = "adsp_ssr",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_wdt_of_match),
	},
};

int __init adsp_ssr_init(void)
{
	adsp_recovery_init();

	return platform_driver_register(&adsp_ssr_driver);
}

void adsp_ssr_exit(void)
{
	platform_driver_unregister(&adsp_ssr_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING ADSP SSR driver");
MODULE_LICENSE("Dual BSD/GPL");

