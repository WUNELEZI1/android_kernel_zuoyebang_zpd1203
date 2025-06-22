// SPDX-License-Identifier: GPL-2.0
/*
 * Authors:
 */
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/tracepoint.h>
#include <linux/proc_fs.h>
#include <uapi/scsi/scsi_bsg_ufs.h>
#include <soc/xring/trace_hook_set.h>
#include <soc/xring/hook_ringbuffer_api.h>
#include <soc/xring/xr_timestamp.h>
#include "ufshcd.h"
#include "ufs-xring-debug.h"
#include "ufshci.h"
#define CREATE_TRACE_POINTS
#include <trace/events/ufs.h>

#undef CREATE_TRACE_POINTS
#include <trace/hooks/ufshcd.h>

static spinlock_t cmd_hist_lock;
static struct ufs_hba *ufshba;
static bool cmd_hist_enabled;

static void probe_android_vh_ufs_send_tm_command(void *data,
						struct ufs_hba *hba,
						int tag, int str_t)
{
	u8 tm_func;
	int lun, task_tag;
	enum cmd_hist_event event = CMD_UNKNOWN;
	enum ufs_trace_str_t _str_t = str_t;
	struct utp_task_req_desc *d = &hba->utmrdl_base_addr[tag];
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	lun = (be32_to_cpu(d->upiu_req.req_header.dword_0) >> 8) & 0xFF;
	task_tag = be32_to_cpu(d->upiu_req.req_header.dword_0) & 0xFF;
	tm_func = (be32_to_cpu(d->upiu_req.req_header.dword_1) >> 16) & 0xFFFF;

	switch (_str_t){
	case UFS_TM_SEND:
		event = CMD_TM_SEND;
		break;
	case UFS_TM_COMP:
		event = CMD_TM_COMPLETED;
		break;
	case UFS_TM_ERR:
		event = CMD_TM_COMPLETED_ERR;
		break;
	default:
		pr_notice("%s: undefined TM command (0x%x)", __func__, _str_t);
		break;
	}

	info.cpu = smp_processor_id();
	info.duration = 0;
	info.pid = current->pid;
	info.clock = xr_timestamp_gettime();
	info.event = event;
	info.cmd.tm.lun = lun;
	info.cmd.tm.tag = tag;
	info.cmd.tm.task_tag = task_tag;
	info.cmd.tm.tm_func = tm_func;

	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
}

#define MAX_OUTSTADING_REQS_NUM 63
static u64 outstanding_reqs_time[MAX_OUTSTADING_REQS_NUM] = {0};
static void probe_ufshcd_command(void *data, const char *dev_name,
				enum ufs_trace_str_t str_t, unsigned int tag,
				u32 doorbell, u32 hwq_id, int transfer_len,
				u32 intr, u64 lba, u8 opcode, u8 group_id)
{
	unsigned long flags;
	enum cmd_hist_event event;
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	spin_lock_irqsave(&cmd_hist_lock, flags);
	if (str_t == UFS_CMD_SEND)
		event = CMD_SEND;
	else if (str_t == UFS_CMD_COMP)
		event = CMD_COMPLETED;
	else
		goto out_unlock;
	info.cpu = smp_processor_id();
	info.duration = 0;
	info.pid = current->pid;
	info.clock = xr_timestamp_gettime();
	if (event == CMD_SEND && tag < MAX_OUTSTADING_REQS_NUM)
		outstanding_reqs_time[tag] = info.clock;
	info.event = event;
	info.cmd.utp.tag = tag;
	info.cmd.utp.transfer_len = transfer_len;
	info.cmd.utp.lba = lba;
	info.cmd.utp.opcode = opcode;
	info.cmd.utp.doorbell = doorbell;
	info.cmd.utp.intr = intr;
	if (event == CMD_COMPLETED && tag < MAX_OUTSTADING_REQS_NUM) {
		if (outstanding_reqs_time[tag] != 0) {
			info.duration = xr_timestamp_gettime() - outstanding_reqs_time[tag];
			outstanding_reqs_time[tag] = 0;
		}
	}

	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
out_unlock:
	spin_unlock_irqrestore(&cmd_hist_lock, flags);
}

struct prev_uic_cmd {
	u64 clock;
	u8 cmd;
};
static struct prev_uic_cmd prev_cmd = {0, 0};
static void probe_ufshcd_uic_command(void *data, const char *dev_name,
				enum ufs_trace_str_t str_t, u32 cmd,
				u32 arg1, u32 arg2, u32 arg3)
{
	enum cmd_hist_event event;
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	if (str_t == UFS_CMD_SEND)
		event = CMD_UIC_SEND;
	else
		event = CMD_UIC_CMPL_GENERAL;
	info.cpu = smp_processor_id();
	info.duration = 0;
	info.pid = current->pid;
	info.clock = xr_timestamp_gettime();
	if (event == CMD_UIC_SEND) {
		prev_cmd.clock = info.clock;
		prev_cmd.cmd = cmd;
	}
	info.event = event;
	info.cmd.uic.cmd = cmd;
	info.cmd.uic.arg1 = arg1;
	info.cmd.uic.arg2 = arg2;
	info.cmd.uic.arg3 = arg3;
	if (event == CMD_UIC_CMPL_GENERAL && prev_cmd.cmd == cmd)
		info.duration = xr_timestamp_gettime() - prev_cmd.clock;

	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
}

static void probe_ufshcd_clk_gating(void *data, const char *dev_name,
				int state)
{
	unsigned long flags;
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	spin_lock_irqsave(&cmd_hist_lock, flags);
	info.cpu = smp_processor_id();
	info.duration = 0;
	info.pid = current->pid;
	info.clock = xr_timestamp_gettime();
	info.event = CMD_CLK_GATING;
	info.cmd.clk_gating.state = state;
	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
	spin_unlock_irqrestore(&cmd_hist_lock, flags);
}

static void probe_ufshcd_profile_clk_scaling(void *data, const char *dev_name,
					const char *profile_info, s64 time_us,
					int err)
{
	unsigned long flags;
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	spin_lock_irqsave(&cmd_hist_lock, flags);
	info.event = CMD_CLK_SCALING;
	if (!strcmp(profile_info, "up"))
		info.cmd.clk_scaling.state = CLKS_SCALE_UP;
	else
		info.cmd.clk_scaling.state = CLKS_SCALE_DOWN;
	info.cmd.clk_scaling.err = err;
	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
	spin_unlock_irqrestore(&cmd_hist_lock, flags);
}

static void cmd_hist_add_dev_cmd(struct ufs_hba *hba, struct ufshcd_lrb *lbrbp,
				enum cmd_hist_event event)
{
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	info.event = event;
	info.cmd.dev.type = hba->dev_cmd.type;
	if (hba->dev_cmd.type == DEV_CMD_TYPE_NOP)
		return;
	info.cmd.dev.tag = lbrbp->task_tag;
	info.cmd.dev.opcode =
			hba->dev_cmd.query.request.upiu_req.opcode;
	info.cmd.dev.idn =
			hba->dev_cmd.query.request.upiu_req.idn;
	info.cmd.dev.index =
			hba->dev_cmd.query.request.upiu_req.index;
	info.cmd.dev.selector =
			hba->dev_cmd.query.request.upiu_req.selector;
	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
}

static void probe_android_vh_ufs_send_command(void *data, struct ufs_hba *hba,
						struct ufshcd_lrb *lrbp)
{
	if (lrbp->cmd)
		return;

	if (!cmd_hist_enabled)
		return;

	cmd_hist_add_dev_cmd(hba, lrbp, CMD_DEV_SEND);
}

static void probe_android_vh_ufs_compl_command(void *data, struct ufs_hba *hba,
						struct ufshcd_lrb *lrbp)
{
	if (lrbp->cmd)
		return;

	if (!cmd_hist_enabled)
		return;

	cmd_hist_add_dev_cmd(hba, lrbp, CMD_DEV_COMPLETED);
}

static void probe_ufshcd_pm(void *data, const char *dev_name, int err,
				s64 time_us, int pwr_mode, int link_state,
				enum ufsdbg_pm_state state)
{
	unsigned long flags;
	struct ufs_hook_info info;
	struct trace_hook_root *hook_root = trace_hook_root_get();

	if (!cmd_hist_enabled)
		return;

	spin_lock_irqsave(&cmd_hist_lock, flags);
	info.event = CMD_PM;
	info.cmd.pm.state = state;
	info.cmd.pm.err = err;
	info.cmd.pm.time_us = time_us;
	info.cmd.pm.pwr_mode = pwr_mode;
	info.cmd.pm.link_state = link_state;
	ringbuffer_write((struct hook_ringbuffer *)hook_root->hook_buffer_addr[HOOK_UFS],
					(u8 *)&info);
	spin_unlock_irqrestore(&cmd_hist_lock, flags);
}

static void probe_ufshcd_runtime_suspend(void *data, const char *dev_name,
					int err, s64 time_us,
					int pwr_mode, int link_state)
{
	probe_ufshcd_pm(data, dev_name, err, time_us, pwr_mode, link_state,
					UFSDBG_RUNTIME_SUSPEND);
}

static void probe_ufshcd_runtime_resume(void *data, const char *dev_name,
					int err, s64 time_us,
					int pwr_mode, int link_state)
{
	probe_ufshcd_pm(data, dev_name, err, time_us, pwr_mode, link_state,
					UFSDBG_RUNTIME_RESUME);
}

static void probe_ufshcd_system_suspend(void *data, const char *dev_name,
					int err, s64 time_us,
					int pwr_mode, int link_state)
{
	probe_ufshcd_pm(data, dev_name, err, time_us, pwr_mode, link_state,
					UFSDBG_SYSTEM_SUSPEND);
}

static void probe_ufshcd_system_resume(void *data, const char *dev_name,
					int err, s64 time_us,
					int pwr_mode, int link_state)
{
	probe_ufshcd_pm(data, dev_name, err, time_us, pwr_mode, link_state,
					UFSDBG_SYSTEM_RESUME);
}

static ssize_t ufs_debug_proc_write(struct file *file, const char *buf,
									size_t count, loff_t *data)
{
	char *tmp = kzalloc((count+1), GFP_KERNEL);

	if (!tmp)
		return -ENOMEM;

	if (copy_from_user(tmp, buf, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	if (!strncmp(tmp, "true", 4))
		cmd_hist_enabled = true;
	else if (!strncmp(tmp, "false", 5))
		cmd_hist_enabled = false;
	else
		dev_err(ufshba->dev, "enter true/false to enable xr ufs debug\n");

	kfree(tmp);
	return count;
}

static int ufs_debug_proc_show(struct seq_file *seq, void *v)
{
	if (cmd_hist_enabled)
		seq_puts(seq, "true\n");
	else
		seq_puts(seq, "false\n");

	return 0;
}

static int ufs_debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_debug_proc_show, inode->i_private);
}

static const struct proc_ops xr_ufs_debug_fops = {
	.proc_open = ufs_debug_proc_open,
	.proc_write = ufs_debug_proc_write,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int ufs_xring_dbg_register(struct ufs_hba *hba)
{
	int ret = 0;
	struct proc_dir_entry *proc_entry;

	if (IS_ERR_OR_NULL(hba)) {
		pr_err("%s hba is null\n", __func__);
		return -EINVAL;
	}

	ufshba = hba;
	spin_lock_init(&cmd_hist_lock);

	REGISTER_TRACE(register_trace_ufshcd_command, probe_ufshcd_command, "probe_ufshcd_command", ret);
	REGISTER_TRACE(register_trace_ufshcd_uic_command, probe_ufshcd_uic_command,
					"probe_ufshcd_uic_command", ret);
	REGISTER_TRACE(register_trace_ufshcd_clk_gating, probe_ufshcd_clk_gating, "probe_ufshcd_clk_gating", ret);
	REGISTER_TRACE(register_trace_android_vh_ufs_send_tm_command, probe_android_vh_ufs_send_tm_command,
					"probe_android_vh_ufs_send_tm_command", ret);
	REGISTER_TRACE(register_trace_ufshcd_profile_clk_scaling, probe_ufshcd_profile_clk_scaling,
					"probe_ufshcd_profile_clk_scaling", ret);
	REGISTER_TRACE(register_trace_android_vh_ufs_send_command, probe_android_vh_ufs_send_command,
					"probe_android_vh_ufs_send_command", ret);
	REGISTER_TRACE(register_trace_android_vh_ufs_compl_command, probe_android_vh_ufs_compl_command,
					"probe_android_vh_ufs_compl_command", ret);
	REGISTER_TRACE(register_trace_ufshcd_runtime_suspend, probe_ufshcd_runtime_suspend,
					"probe_ufshcd_runtime_suspend", ret);
	REGISTER_TRACE(register_trace_ufshcd_runtime_resume, probe_ufshcd_runtime_resume,
					"probe_ufshcd_runtime_resume", ret);
	REGISTER_TRACE(register_trace_ufshcd_system_suspend, probe_ufshcd_system_suspend,
					"probe_ufshcd_system_suspend", ret);
	REGISTER_TRACE(register_trace_ufshcd_system_resume, probe_ufshcd_system_resume,
					"probe_ufshcd_system_resume", ret);

	proc_entry = proc_create("xr_ufs_debug", PROC_PERM, NULL, &xr_ufs_debug_fops);
	if (IS_ERR(proc_entry)) {
		ret = PTR_ERR(proc_entry);
		dev_err(ufshba->dev, "create xr_ufs_debug procfs fail(%d)\n", ret);
	}

	dev_info(hba->dev, "%s success\n", __func__);

	return 0;
}
EXPORT_SYMBOL_GPL(ufs_xring_dbg_register);

int ufs_xring_dbg_unregister(void)
{
	int ret = 0;

	UNREGISTER_TRACE(unregister_trace_ufshcd_command, probe_ufshcd_command, "probe_ufshcd_command", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_uic_command, probe_ufshcd_uic_command, "probe_ufshcd_uic_command", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_clk_gating, probe_ufshcd_clk_gating, "probe_ufshcd_clk_gating", ret);
	UNREGISTER_TRACE(unregister_trace_android_vh_ufs_send_tm_command, probe_android_vh_ufs_send_tm_command,
					"probe_android_vh_ufs_send_tm_command", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_profile_clk_scaling, probe_ufshcd_profile_clk_scaling,
					"probe_ufshcd_profile_clk_scaling", ret);
	UNREGISTER_TRACE(unregister_trace_android_vh_ufs_send_command, probe_android_vh_ufs_send_command,
					"probe_android_vh_ufs_send_command", ret);
	UNREGISTER_TRACE(unregister_trace_android_vh_ufs_compl_command, probe_android_vh_ufs_compl_command,
					"probe_android_vh_ufs_compl_command", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_runtime_suspend, probe_ufshcd_runtime_suspend,
					"probe_ufshcd_runtime_suspend", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_runtime_resume, probe_ufshcd_runtime_resume,
					"probe_ufshcd_runtime_resume", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_system_suspend, probe_ufshcd_system_suspend,
					"probe_ufshcd_system_suspend", ret);
	UNREGISTER_TRACE(unregister_trace_ufshcd_system_resume, probe_ufshcd_system_resume,
					"probe_ufshcd_system_resume", ret);

	return 0;
}
EXPORT_SYMBOL_GPL(ufs_xring_dbg_unregister);
