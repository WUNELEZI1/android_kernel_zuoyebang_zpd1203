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

#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/panic.h>
#include <linux/dma-mapping.h>
#include <linux/of_address.h>

#include "fk-common-debugfs.h"
#include "../common/fk-audio-ipc.h"
#include "../common/fk-audio-mailbox.h"
#include "../common/fk-audio-log.h"
#include "../acore/fk-acore-utils.h"
#include "../adsp/adsp_boot_driver.h"
#include "../adsp/adsp_ssr_driver.h"
#include "fk-audio-tool-core.h"
#include "fk-audio-pinctrl.h"
#include "fk-audio-timer.h"

#define FRAMES_PER_BLOCK_MAX	(500)

static int audio_session_fs_show(struct seq_file *s, void *unused)
{
	struct acore_client *ac = s->private;
	struct acore_stream *acStrm = NULL;
	int total_sess = 0;

	pr_info("Audio active session as follow:\n");

	list_for_each_entry(acStrm, &ac->ac_strm_list, strm_list) {
		pr_info("....session   .= %x, state=%d.\n",
				acStrm->sess_id, acStrm->state);
		total_sess++;
	}

	pr_info("  current total session is : %d.\n", total_sess);

	return 0;
}

static int audio_session_fs_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_session_fs_show, inode->i_private);
}

static const struct file_operations audio_session_fs_fops = {
	.open    = audio_session_fs_open,
	.read    = seq_read,
	.release = single_release,
};

static int audio_mailbox_info_show(struct seq_file *s, void *unused)
{
	uint32_t val[4] = {0};

	pr_info("mailbox info as follow:\n");

	/* get ap to dsp mailbox info */
	pr_info("    AP_DSP info:\n");
	fk_mbx_msg_param_get(MBX_CH_AP_DSP, MBX_PARAM_PUT_INFO, (void *)&val);
	pr_info("        put info: %d\n", val[0]);
	fk_mbx_msg_param_get(MBX_CH_AP_DSP, MBX_PARAM_GET_INFO, (void *)&val);
	pr_info("        get info: %d\n", val[0]);
	fk_mbx_msg_param_get(MBX_CH_AP_DSP, MBX_PARAM_OV_INFO, (void *)&val);
	pr_info("        over flow info: %d\n", val[0]);

	/* get dsp to ap mailbox info */
	pr_info("    DSP_AP info:\n");
	fk_mbx_msg_param_get(MBX_CH_DSP_AP, MBX_PARAM_PUT_INFO, (void *)&val);
	pr_info("        put info: %d\n", val[0]);
	fk_mbx_msg_param_get(MBX_CH_DSP_AP, MBX_PARAM_GET_INFO, (void *)&val);
	pr_info("        get info: %d\n", val[0]);
	fk_mbx_msg_param_get(MBX_CH_DSP_AP, MBX_PARAM_OV_INFO, (void *)&val);
	pr_info("        over flow info: %d\n", val[0]);

	fk_mbx_msg_dump();

	return 0;
}

static int audio_mailbox_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_mailbox_info_show, inode->i_private);
}

static const struct file_operations audio_mailbox_info_fops = {
	.open    = audio_mailbox_info_open,
	.read    = seq_read,
	.release = single_release,
};

static int audio_heartbeat_show(struct seq_file *s, void *unused)
{
	return 0;
}

static ssize_t audio_heartbeat_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct xring_aud_msg msg = {0};
	char content;
	int val = 1;

	if (copy_from_user(&content, ubuf, 1))
		return -EFAULT;

	if (kstrtoint(&content, 10, &val) < 0) {
		pr_info("%s: %d unexpect.\n", __func__, val);
		val = 1;
	}

	pr_info("%s:val=%d\n", __func__, val);

	msg.header.len = sizeof(struct xring_aud_msg_header);
	msg.header.msg_source = MAX_SOURCE_TYPE_MSG;
	msg.data[0] = val;

	fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)&msg,
			msg.header.len, NULL, NULL);

	mdelay(100);

	return count;
}

static int audio_heartbeat_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_heartbeat_show, inode->i_private);
}

static const struct file_operations audio_heartbeat_fops = {
	.open    = audio_heartbeat_open,
	.read    = seq_read,
	.write   = audio_heartbeat_write,
	.release = single_release,
};

static int audio_compr_cap_perblk_show(struct seq_file *s, void *unused)
{
	struct acore_client *ac = s->private;

	pr_info("%d frames per block.\n", ac->frames_per_blk);

	return 0;
}

static ssize_t audio_compr_cap_perblk_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct acore_client *ac = s->private;
	char content[4];
	int val = 1;

	if (copy_from_user(&content[0], ubuf, 4))
		return -EFAULT;

	if (kstrtoint(&content[0], 10, &val) < 0) {
		pr_err("%s: %d unexpect.\n", __func__, val);
		val = 1;
	}

	if (val > FRAMES_PER_BLOCK_MAX) {
		pr_info("%d over max frames number.\n", val);
	} else {
		ac->frames_per_blk = val;
		pr_info("set %d frames number one block.\n", val);
	}

	return count;
}

static int audio_compr_cap_perblk_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_compr_cap_perblk_show, inode->i_private);
}

static const struct file_operations audio_compr_cap_perblk_fops = {
	.open    = audio_compr_cap_perblk_open,
	.read    = seq_read,
	.write   = audio_compr_cap_perblk_write,
	.release = single_release,
};

static int audio_backdoor_mode_fs_show(struct seq_file *s, void *unused)
{
	struct acore_client *ac = s->private;

	pr_info("Backdoor mode : %d.\n", ac->backdoor_boot);

	return 0;
}

static ssize_t audio_backdoor_mode_fs_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct acore_client *ac = s->private;
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0') {
		ac->backdoor_boot = false;
		pr_info("Backdoor Disable.\n");
	} else {
		ac->backdoor_boot = true;
		pr_info("Backdoor Enable.\n");
	}

	return count;
}

static int audio_backdoor_mode_fs_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_backdoor_mode_fs_show, inode->i_private);
}

static const struct file_operations audio_backdoor_mode_fs_fops = {
	.open    = audio_backdoor_mode_fs_open,
	.read    = seq_read,
	.write   = audio_backdoor_mode_fs_write,
	.release = single_release,
};

static int audio_adsp_test_fs_show(struct seq_file *s, void *unused)
{
	return 0;
}

static ssize_t audio_adsp_test_fs_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char test_c[32];
	size_t buf_size;
	const char *str2 = "adsp_perf_test";
	const char *str3 = "get_time";
	const char *str4 = "dump_ipc";

	buf_size = min(count, (sizeof(test_c)-1));
	if (copy_from_user(test_c, ubuf, buf_size))
		return -EFAULT;

	pr_info("%s:test_c:%s.\n", __func__, test_c);

	if (!strncmp(test_c, str2, strlen(str2))) {
		struct xring_aud_msg msg = {0};

		/* fill close message */
		msg.header.scene_id = ADSP_SCENE_NONE;
		msg.header.ins_id = 0;
		msg.header.cmd_id = ADSP_DEBUG_CTRL;
		msg.header.func_id = ADSP_DEBUG_CTRL_PCM_DUMP_CLOSE;
		msg.header.len = sizeof(struct xring_aud_msg_header);

		get_adsp_time();
		adsp_poweron();
		get_adsp_time();
		fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)&msg,
			msg.header.len, NULL, NULL);
	} else if (!strncmp(test_c, str3, strlen(str3))) {
		get_adsp_time();
		get_utc_time();
	} else if (!strncmp(test_c, str4, strlen(str4))) {
		get_adsp_time();
		fk_audio_dump_ipc();
	}

	return count;
}

static int audio_adsp_test_fs_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_adsp_test_fs_show, inode->i_private);
}

static const struct file_operations audio_adsp_test_fs_fops = {
	.open    = audio_adsp_test_fs_open,
	.read    = seq_read,
	.write   = audio_adsp_test_fs_write,
	.release = single_release,
};

#ifdef ABANDON_FRAME_VER
static int audio_abandon_frame_count_show(struct seq_file *s, void *unused)
{
	struct acore_client *ac = s->private;

	pr_info("%d abandon frame count.\n", ac->abandon_frame_count);

	return 0;
}

static ssize_t audio_abandon_frame_count_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct acore_client *ac = s->private;
	char content[4];
	int val = 0;

	if (copy_from_user(&content[0], ubuf, 4))
		return -EFAULT;

	if (kstrtoint(&content[0], 10, &val) < 0) {
		pr_err("%s: %d unexpect.\n", __func__, val);
		val = 0;
	}

	if (val < 0) {
		ac->abandon_frame_count = 0;
		pr_info("%d over max frames number.\n", val);
	} else {
		ac->abandon_frame_count = val;
		pr_info("set abandon frame count:%d.\n", val);
	}
	fk_audio_ipc_set_abandon_frame_count(val);

	return count;
}

static int audio_abandon_frame_count_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_abandon_frame_count_show, inode->i_private);
}

static const struct file_operations audio_abandon_frame_count_fs_fops = {
	.open    = audio_abandon_frame_count_open,
	.read    = seq_read,
	.write   = audio_abandon_frame_count_write,
	.release = single_release,
};
#endif

#ifdef FK_AUDIO_USB_OFFLOAD
static int audio_usb_conn_fs_show(struct seq_file *s, void *unused)
{
	struct acore_client *ac = s->private;

	pr_info("audio usb connect state : %d", ac->usb_conn);

	return 0;
}

static ssize_t audio_usb_conn_fs_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct acore_client *ac = s->private;
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0') {
		ac->usb_conn = false;
		//audio_usb_connect_set(false);
	} else {
		ac->usb_conn = true;
		//audio_usb_connect_set(true);
	}

	return count;
}

static int audio_usb_conn_fs_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_usb_conn_fs_show, inode->i_private);
}

static const struct file_operations audio_usb_conn_fs_fops = {
	.open    = audio_usb_conn_fs_open,
	.read    = seq_read,
	.write   = audio_usb_conn_fs_write,
	.release = single_release,
};
#endif

static int audio_log_enable_show(struct seq_file *s, void *unused)
{
	aud_log_info_query();

	return 0;
}

static ssize_t audio_log_enable_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	int id = 0;
	int val = 1;
	int ret = 0;
	char content1[16] = {};
	char content2[16] = {};
	char kbuf[16] = {};

	int i, j;

	if (copy_from_user((char *)&kbuf[0], ubuf, 16))
		return -EFAULT;

	for (i = 0; i < 16; i++) {
		if ((kbuf[i] >= '0') && (kbuf[i] <= '9'))
			continue;

		memcpy((char *)&content1[0], (char *)&kbuf[0], i);
		if (kstrtoint(&content1[0], 10, &id) < 0) {
			pr_err("%s: id %d unexpect.\n", __func__, id);
			id = AUD_MAX;
		}
		break;
	}

	pr_info("%s: i = %d", __func__, i);

	for (j = i; j < 16; j++) {
		if ((kbuf[j] < '0') || (kbuf[j] > '9'))
			continue;

		memcpy((char *)&content2[0], (char *)&kbuf[j], (16-j));
		if (kstrtoint(&content2[0], 10, &val) < 0) {
			pr_err("%s: val %d unexpect.\n", __func__, val);
			val = 1;
		}

		//ret = sscanf((char *)&kbuf[0], "%d %d", &id, &val);
		if (id >= AUD_MAX)
			id = AUD_MAX;

		pr_info("%s: id %d. val %d. ret %d\n", __func__,
			id, val, ret);
		aud_log_enable(id, val);

		break;
	}

	return count;
}

static int audio_log_enable_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_log_enable_show, inode->i_private);
}

static const struct file_operations audio_log_enable_fops = {
	.open    = audio_log_enable_open,
	.read    = seq_read,
	.write   = audio_log_enable_write,
	.release = single_release,
};

static int audio_log_lev_show(struct seq_file *s, void *unused)
{
	aud_log_info_query();

	return 0;
}

static ssize_t audio_log_lev_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	int id = 0;
	int val = 1;
	int ret = 0;
	char content1[16] = {};
	char content2[16] = {};
	char kbuf[16] = {};

	int i, j;

	if (copy_from_user((char *)&kbuf[0], ubuf, 16))
		return -EFAULT;

	for (i = 0; i < 16; i++) {
		if ((kbuf[i] >= '0') && (kbuf[i] <= '9'))
			continue;

		memcpy((char *)&content1[0], (char *)&kbuf[0], i);
		if (kstrtoint(&content1[0], 10, &id) < 0) {
			pr_err("%s: id %d unexpect.\n", __func__, id);
			id = AUD_MAX;
		}
		break;
	}

	pr_info("%s: i = %d", __func__, i);

	for (j = i; j < 16; j++) {
		if ((kbuf[j] < '0') || (kbuf[j] > '9'))
			continue;

		memcpy((char *)&content2[0], (char *)&kbuf[j], (16-j));
		if (kstrtoint(&content2[0], 10, &val) < 0) {
			pr_err("%s: val %d unexpect.\n", __func__, val);
			val = 1;
		}

		//ret = sscanf((char *)&kbuf[0], "%d %d", &id, &val);
		if (id >= AUD_MAX)
			id = AUD_MAX;

		pr_info("%s: id %d. val %d. ret %d\n", __func__,
			id, val, ret);
		aud_log_lev_up_set(id, val);

		break;
	}

	return count;
}

static int audio_log_lev_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_log_lev_show, inode->i_private);
}

static const struct file_operations audio_log_lev_fops = {
	.open    = audio_log_lev_open,
	.read    = seq_read,
	.write   = audio_log_lev_write,
	.release = single_release,
};

static int audio_dsp_vote_show(struct seq_file *s, void *unused)
{
	int ret = 0;

	pr_info("adsp vote config: %d power status : %d.\n",
		fk_adsp_vote_config_query(), fk_adsp_power_status_get());

	ret = fk_adsp_vote_info_query();

	return ret;
}

static ssize_t audio_dsp_vote_config(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0') {
		fk_adsp_vote_config(0);
		pr_info("disable adsp power vote.\n");
	} else {
		fk_adsp_vote_config(1);
		pr_info("enable adsp power vote.\n");
	}

	return count;
}

static int audio_dsp_vote_config_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_vote_show, inode->i_private);
}

static const struct file_operations audio_dsp_vote_config_fops = {
	.open    = audio_dsp_vote_config_open,
	.read    = seq_read,
	.write   = audio_dsp_vote_config,
	.release = single_release,
};

static ssize_t audio_dsp_vote(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0') {
		fk_adsp_vote(false);
		pr_info("adsp power off vote.\n");
	} else {
		fk_adsp_vote(true);
		pr_info("adsp power on vote.\n");
	}

	return count;
}

static int audio_dsp_vote_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_vote_show, inode->i_private);
}

static const struct file_operations audio_dsp_vote_fops = {
	.open    = audio_dsp_vote_open,
	.read    = seq_read,
	.write   = audio_dsp_vote,
	.release = single_release,
};

static int audio_pinctrl_func_show(struct seq_file *s, void *unused)
{
	pr_info("audio_pinctrl_func open.\n");

	return 0;
}

static int audio_pinctrl_func_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_pinctrl_func_show, inode->i_private);
}

static ssize_t audio_pinctrl_func_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	int func_id = 0;
	int action = 1;
	int ret = 0;
	char content1[16] = {};
	char content2[16] = {};
	char kbuf[16] = {};

	int i, j;

	if (copy_from_user((char *)&kbuf[0], ubuf, 16))
		return -EFAULT;

	for (i = 0; i < 16; i++) {
		if ((kbuf[i] >= '0') && (kbuf[i] <= '9'))
			continue;

		memcpy((char *)&content1[0], (char *)&kbuf[0], i);
		if (kstrtoint(&content1[0], 10, &func_id) < 0) {
			pr_err("%s: func id %d unexpect.\n", __func__, func_id);
			return -EFAULT;
		}
		break;
	}

	pr_info("%s: i = %d", __func__, i);

	for (j = i; j < 16; j++) {
		if ((kbuf[j] < '0') || (kbuf[j] > '9'))
			continue;

		memcpy((char *)&content2[0], (char *)&kbuf[j], (16-j));
		if (kstrtoint(&content2[0], 10, &action) < 0) {
			pr_err("%s: val %d unexpect.\n", __func__, action);
			return -EFAULT;
		}

		if (func_id >= PINCTRL_FUNC_MAX)
			return -EFAULT;

		pr_info("%s: func_id %d. action %d. ret %d\n", __func__,
			func_id, action, ret);
		ret = fk_audio_pinctrl_switch_state(func_id, action);
		if (ret) {
			pr_err("%s: false", __func__);
			return -EFAULT;
		}
		break;
	}

	return count;
}

static const struct file_operations audio_pinctrl_func_fops = {
	.open    = audio_pinctrl_func_open,
	.read    = seq_read,
	.write   = audio_pinctrl_func_write,
	.release = single_release,
};

static ssize_t audio_codec_reg_config_write(struct file *file,
		const char __user *user_buf,
		size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;
	char *start = buf;
	unsigned long reg, value;
	int ret;

	buf_size = min(count, (sizeof(buf)-1));
	pr_info("%s: buf_size %zu\n", __func__, buf_size);
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = 0;

	while (*start == ' ')
		start++;
	if (*start == 'w') {
		start++;
		while (*start == ' ')
			start++;
		ret = sscanf(start, "%lx %lx", &reg, &value);
		pr_info("%s: num:%d reg:0x%lx value:0x%lx\n", __func__, ret, reg, value);
		codec_reg_debugs_write((unsigned int)reg, (unsigned int)value);
	}

	if (*start == 'r') {
		start++;
		while (*start == ' ')
			start++;
		if (kstrtoul(start, 16, &reg)) {
			pr_info("%s: parse read reg fail\n", __func__);
			return -EINVAL;
		}
		pr_info("%s: codec_reg_config read reg = 0x%lx\n", __func__, reg);
		/* TODO read codec reg*/
		codec_reg_debugs_read((unsigned int)reg);
	}

	/* Userspace has been fiddling around behind the kernel's back */
	add_taint(TAINT_USER, LOCKDEP_STILL_OK);

	return count;
}

static const struct file_operations audio_codec_reg_config_fs_fops = {
	.open    = simple_open,
	.write   = audio_codec_reg_config_write,
	.llseek = default_llseek,
};

#ifdef AUDIO_TOOL_DEBUG
static ssize_t audio_tool_mode_fs_write(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	int ret, size;
	char cmd;

	if (get_user(cmd, ubuf))
		return -EFAULT;

	if (cmd == '0') {
		struct dsp_log_dump_param_info dsp_log_info;

		pr_info("set dsp log dump param level:1, port:ap&uart\n");
		dsp_log_info.log_level = 1;
		dsp_log_info.log_port = 0x3;
		dsp_log_info.log_enable = 1;
		size = sizeof(struct dsp_log_dump_param_info);
		ret = fk_audio_tool_set_log_param(TOOL_TYPE_DUMP_LOG,
			size, &dsp_log_info);

	} else if (cmd == '1') {
		struct dsp_log_dump_param_info dsp_log_info;

		pr_info("set dsp log dump param level:3, port:ap&uart\n");
		dsp_log_info.log_level = 3;
		dsp_log_info.log_port = 0x3;
		dsp_log_info.log_enable = 1;
		size = sizeof(struct dsp_log_dump_param_info);
		ret = fk_audio_tool_set_log_param(TOOL_TYPE_DUMP_LOG,
			size, &dsp_log_info);
	} else if (cmd == '2') {
		struct dsp_log_dump_param_info dsp_log_info;

		pr_info("set dsp log dump param level:3, port:ap.\n");
		dsp_log_info.log_level = 3;
		dsp_log_info.log_port = 0x2;
		dsp_log_info.log_enable = 1;
		size = sizeof(struct dsp_log_dump_param_info);
		ret = fk_audio_tool_set_log_param(TOOL_TYPE_DUMP_LOG,
			size, &dsp_log_info);
	} else if (cmd == '3') {
		struct dsp_log_dump_param_info dsp_log_info;

		pr_info("set dsp log dump param level:3, port:uart.\n");
		dsp_log_info.log_level = 3;
		dsp_log_info.log_port = 0x1;
		dsp_log_info.log_enable = 1;
		size = sizeof(struct dsp_log_dump_param_info);
		ret = fk_audio_tool_set_log_param(TOOL_TYPE_DUMP_LOG,
			size, &dsp_log_info);
	} else if (cmd == '4') {
		struct dsp_log_dump_param_info dsp_log_info;

		pr_info("set dsp log dump param disable.\n");
		dsp_log_info.log_level = 3;
		dsp_log_info.log_port = 0x2;
		dsp_log_info.log_enable = 0;
		size = sizeof(struct dsp_log_dump_param_info);
		ret = fk_audio_tool_set_log_param(TOOL_TYPE_DUMP_LOG,
			size, &dsp_log_info);
	} else if (cmd == '5') {
		struct dsp_pcm_dump_param_info dsp_pcm_info;

		pr_info("set dsp pcm dump param.\n");
		dsp_pcm_info.point1 = 0x1111;
		dsp_pcm_info.point2 = 0x2222;
		dsp_pcm_info.point3 = 0x3333;
		size = sizeof(struct dsp_pcm_dump_param_info);
		ret = fk_audio_tool_set_pcm_param(TOOL_TYPE_DUMP_PCM,
			size, &dsp_pcm_info);
	} else if (cmd == '6') {
		struct dsp_pcm_dump_param_info dsp_pcm_info;

		pr_info("set dsp pcm dump param.\n");
		dsp_pcm_info.point1 = 0x0000;
		dsp_pcm_info.point2 = 0x0000;
		dsp_pcm_info.point3 = 0x0000;
		size = sizeof(struct dsp_pcm_dump_param_info);
		ret = fk_audio_tool_set_pcm_param(TOOL_TYPE_DUMP_PCM,
			size, &dsp_pcm_info);
	}

	return count;
}

static int audio_tool_mode_fs_show(struct seq_file *s, void *unused)
{
	struct dump_blk *dump_blk_t = s->private;
	struct packetheader *packetheader_t;
	void *data = NULL;

	if (!dump_blk_t->vir_addr) {
		pr_info("none dump data.\n");
		return 0;
	}

	if (!dump_blk_t->size) {
		pr_info("none size.\n");
		return 0;
	}

	pr_info("phy_addr:0x%x, size:%d.\n", dump_blk_t->vir_addr,
		dump_blk_t->size);

	data = kmalloc(dump_blk_t->size, GFP_KERNEL);

	memcpy(data, dump_blk_t->vir_addr, dump_blk_t->size);

	packetheader_t = (struct packetheader *)data;
	pr_info("flag:%d,packet_type:%d,packet_len:%d,packet_wr_num:%d,packet_send_num:%d.\n",
		packetheader_t->flag, packetheader_t->packet_type, packetheader_t->packet_len,
		packetheader_t->packet_wr_num, packetheader_t->packet_send_num);

	kfree(data);

	return 0;
}

static int audio_tool_mode_fs_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_tool_mode_fs_show, inode->i_private);
}

static const struct file_operations audio_tool_mode_fs_fops = {
	.open    = audio_tool_mode_fs_open,
	.read    = seq_read,
	.write   = audio_tool_mode_fs_write,
	.release = single_release,
};
#endif

static int audio_dsp_power_show(struct seq_file *s, void *unused)
{
	pr_info("power status : %d.\n", get_adsp_power_status());

	return 0;
}

static ssize_t audio_dsp_power(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0')
		adsp_poweroff();
	else
		adsp_poweron();

	return count;
}

static int audio_dsp_power_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_power_show, inode->i_private);
}

static const struct file_operations audio_dsp_power_fops = {
	.open    = audio_dsp_power_open,
	.read    = seq_read,
	.write   = audio_dsp_power,
	.release = single_release,
};

static int audio_dsp_ssr_show(struct seq_file *s, void *unused)
{
	pr_info("audio_dsp_except open.\n");

	return 0;
}

static ssize_t audio_dsp_ssr(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char flag;

	pr_info("audio dsp active trigger ssr.\n");

	if (get_user(flag, ubuf))
		return -EFAULT;

	get_adsp_time();
	fk_audio_dump_ipc();

	if (flag == '0')
		/* 0: adsp assert */
		adsp_reset(0);
	else if (flag == '1')
		adsp_wdt_exception_injection();
	else if (flag == '2')
		adsp_panic_exception_injection();

	return count;
}

static int audio_dsp_ssr_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_ssr_show, inode->i_private);
}

static const struct file_operations audio_dsp_ssr_fops = {
	.open    = audio_dsp_ssr_open,
	.read    = seq_read,
	.write   = audio_dsp_ssr,
	.release = single_release,
};

static int audio_dsp_reset_switch_show(struct seq_file *s, void *unused)
{
	pr_info("adsp reset switch : %d.\n", get_adsp_reset_switch());

	return 0;
}

static ssize_t audio_dsp_reset_switch(struct file *file,
		const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0')
		set_adsp_reset_switch(false);
	else if (enable == '1')
		set_adsp_reset_switch(true);
	else
		pr_err("invalid parameter\n");

	return count;
}

static int audio_dsp_reset_switch_open(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_reset_switch_show, inode->i_private);
}

static const struct file_operations audio_dsp_reset_switch_fops = {
	.open    = audio_dsp_reset_switch_open,
	.read    = seq_read,
	.write   = audio_dsp_reset_switch,
	.release = single_release,
};

static int audio_dsp_dump_open(struct inode *inode, struct file *file)
{
	return single_open(file, NULL, inode->i_private);
}

static ssize_t audio_dsp_dump_read(struct file *file, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	ssize_t ret = 0;
	void __iomem *run_mem = NULL;
	void __iomem *tcm_mem = NULL;
	void __iomem *audsys_power_state = NULL;
	unsigned int ddr_size = AUDIO_RESERVED_SIZE;
	unsigned int tcm_size = ADSP_TCM_SIZE;

	audsys_power_state = ioremap(AUDSYS_POWER_STATE, IOREMAP_REGISTER_SIZE);
	if (audsys_power_state == NULL) {
		pr_err("ioremap adsp power state failed\n");
		return ret;
	}

	if ((adsp_reg_read32(audsys_power_state) != AUDSYS_POWERON_STATE) &&
		(adsp_reg_read32(audsys_power_state) != AUDSYS_PREPARE_POWERDOWN)) {
		pr_err("adsp sys is poweroff cannot dump\n");
		goto err3;
	}

	run_mem = ioremap(AUDIO_RESERVED_MEM, ddr_size);
	if (run_mem == NULL) {
		pr_err("ioremap adsp run mem failed\n");
		goto err3;
	}

	tcm_mem = ioremap(ADSP_TCM_MEM, tcm_size);
	if (tcm_mem == NULL) {
		pr_err("ioremap adsp tcm mem failed\n");
		goto err2;
	}

	if (copy_to_user(ubuf, run_mem, ddr_size)) {
		pr_err("ddr copy to user failed\n");
		goto err1;
	}
	if (copy_to_user(ubuf + ddr_size, tcm_mem, tcm_size)) {
		pr_err("tcm copy to user failed\n");
		goto err1;
	}
	*ppos = ddr_size + tcm_size;
	ret = ddr_size + tcm_size;

err1:
	iounmap(tcm_mem);
err2:
	iounmap(run_mem);
err3:
	iounmap(audsys_power_state);
	return ret;
}


static const struct file_operations audio_dsp_dump_fops = {
	.open    = audio_dsp_dump_open,
	.read    = audio_dsp_dump_read,
	.write   = NULL,
	.release = single_release,
};

static int audio_dsp_uncachelog_enable_fs_show
	(struct seq_file *s, void *unused)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();

	pr_info("uncache log en flag : %d.\n", pdata->uncache_log_en_flag);

	return 0;
}

static ssize_t audio_dsp_uncachelog_enable_fs_write
		(struct file *file, const char __user *ubuf,
		size_t count, loff_t *ppos)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	char enable;

	if (get_user(enable, ubuf))
		return -EFAULT;

	if (enable == '0') {
		pdata->uncache_log_en_flag = 0;
		pr_info("uncache log disable.\n");
	} else {
		pdata->uncache_log_en_flag = 1;
		pr_info("uncache log enable.\n");
	}

	fk_audio_tool_uncache_log_enable(pdata->uncache_log_en_flag);

	return count;
}

static int audio_dsp_uncachelog_enable_fs_open
	(struct inode *inode, struct file *file)
{
	return single_open(file, audio_dsp_uncachelog_enable_fs_show,
		inode->i_private);
}

static const struct file_operations audio_dsp_uncachelog_enable_fs_fops = {
	.open    = audio_dsp_uncachelog_enable_fs_open,
	.read    = seq_read,
	.write   = audio_dsp_uncachelog_enable_fs_write,
	.release = single_release,
};

int audio_debugfs_init(struct acore_client *ac)
{
	struct dentry *audio_debug_root = NULL;
	struct dentry *file = NULL;
	int ret = 0;

	audio_debug_root = debugfs_create_dir("audio_debug", NULL);
	if (IS_ERR_OR_NULL(audio_debug_root)) {
		pr_err("failed to create audio debug root dir\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_sess", 0644,
			audio_debug_root, (void *)ac, &audio_session_fs_fops);
	if (!file) {
		pr_err("failed to create audio_sess file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_cap_perblk", 0644,
			audio_debug_root, (void *)ac, &audio_compr_cap_perblk_fops);
	if (!file) {
		pr_err("failed to create audio_cap_perblk file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_backdoor", 0644,
			audio_debug_root, (void *)ac, &audio_backdoor_mode_fs_fops);
	if (!file) {
		pr_err("failed to create audio_backdoor file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_adsp_test", 0644,
			audio_debug_root, NULL, &audio_adsp_test_fs_fops);
	if (!file) {
		pr_err("failed to create audio_adsp_test file\n");
		goto error_remove_dir;
	}

#ifdef ABANDON_FRAME_VER
	file = debugfs_create_file("audio_abandon_frame_count", 0644,
			audio_debug_root, (void *)ac, &audio_abandon_frame_count_fs_fops);
	if (!file) {
		pr_err("failed to create audio_abandon_frame_count file\n");
		goto error_remove_dir;
	}
	pr_err("audio_abandon_frame_count file create succ\n");
#endif

	file = debugfs_create_file("aud_mbx_info", 0644,
			audio_debug_root, (void *)ac, &audio_mailbox_info_fops);
	if (!file) {
		pr_err("failed to create aud_mbx_info file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("aud_hb_info", 0644,
			audio_debug_root, (void *)ac, &audio_heartbeat_fops);
	if (!file) {
		pr_err("failed to create aud_hb_info file\n");
		goto error_remove_dir;
	}

#ifdef FK_AUDIO_USB_OFFLOAD
	file = debugfs_create_file("audio_usb", 0644,
			audio_debug_root, (void *)ac, &audio_usb_conn_fs_fops);
	if (!file) {
		pr_err("failed to create audio_usb file\n");
		goto error_remove_dir;
	}
#endif

	file = debugfs_create_file("audio_log_en", 0644,
			audio_debug_root, (void *)ac, &audio_log_enable_fops);
	if (!file) {
		pr_err("failed to create audio_log_enable file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_log_lev", 0644,
			audio_debug_root, (void *)ac, &audio_log_lev_fops);
	if (!file) {
		pr_err("failed to create audio_log_lev file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("adsp_vote_cfg", 0644,
			audio_debug_root, (void *)ac, &audio_dsp_vote_config_fops);
	if (!file) {
		pr_err("failed to create adsp_vote_cfg file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("adsp_vote", 0644,
			audio_debug_root, (void *)ac, &audio_dsp_vote_fops);
	if (!file) {
		pr_err("failed to create adsp_vote file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_pinctrl_func", 0644,
			audio_debug_root, (void *)ac, &audio_pinctrl_func_fops);
	if (!file) {
		pr_err("failed to create audio_pinctrl_func file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_codec_reg_dbg", 0644,
			audio_debug_root, NULL, &audio_codec_reg_config_fs_fops);
	if (!file) {
		pr_err("failed to create audio_codec_reg_dbg file\n");
		goto error_remove_dir;
	}

#ifdef AUDIO_TOOL_DEBUG
	file = debugfs_create_file("audio_tool_dbg", 0644,
			audio_debug_root, (void *)&dump_blk_g, &audio_tool_mode_fs_fops);
	if (!file) {
		pr_err("failed to create audio_codec_reg_dbg file\n");
		goto error_remove_dir;
	}
#endif

	file = debugfs_create_file("audio_dsp_power", 0644,
			audio_debug_root, NULL, &audio_dsp_power_fops);
	if (!file) {
		pr_err("failed to create audio_dsp_power file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_dsp_except", 0644,
			audio_debug_root, NULL, &audio_dsp_ssr_fops);
	if (!file) {
		pr_err("failed to create audio_dsp_except file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_dsp_reset_switch", 0644,
			audio_debug_root, NULL, &audio_dsp_reset_switch_fops);
	if (!file) {
		pr_err("failed to create audio_dsp_reset_switch file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_dsp_dump", 0644,
			audio_debug_root, NULL, &audio_dsp_dump_fops);
	if (!file) {
		pr_err("failed to create audio_dsp_dump_fops file\n");
		goto error_remove_dir;
	}

	file = debugfs_create_file("audio_dsp_uncache_log", 0644,
			audio_debug_root, NULL, &audio_dsp_uncachelog_enable_fs_fops);
	if (!file) {
		pr_err("failed to create audio_dsp_uncache_log_fops file\n");
		goto error_remove_dir;
	}

	ac->dbg_fs_root = audio_debug_root;

	return ret;

error_remove_dir:
	debugfs_remove_recursive(audio_debug_root);
	return ret;
}
EXPORT_SYMBOL(audio_debugfs_init);

void audio_debugfs_deinit(struct acore_client *ac)
{
	debugfs_remove_recursive(ac->dbg_fs_root);
}
EXPORT_SYMBOL(audio_debugfs_deinit);

