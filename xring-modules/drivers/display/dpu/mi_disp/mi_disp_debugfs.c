// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 XiaoMi, Inc. All rights reserved.
 */

#define pr_fmt(fmt)	"mi-disp-debugfs:[%s:%d] " fmt, __func__, __LINE__
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>

#include "mi_disp.h"

#include "dsi_panel.h"
#include "mi_disp_config.h"
#include "mi_disp_print.h"
#include "mi_disp_core.h"
#include "mi_disp_feature.h"

#if MI_DISP_DEBUGFS_ENABLE

#define DEBUG_LOG_DEBUGFS_NAME      "debug_log"
#define BACKLIGHT_LOG_DEBUGFS_NAME  "backlight_log"

const char *esd_sw_str[MI_DISP_MAX] = {
	[MI_DISP_PRIMARY] = "esd_sw_prim",
	[MI_DISP_SECONDARY] = "esd_sw_sec",
};

struct disp_debugfs_t {
	bool debug_log_initialized;
	bool debug_log_enabled;
	bool backlight_log_initialized;
	u32 backlight_log_mask;
	struct dentry *esd_sw[MI_DISP_MAX];
};

static struct disp_debugfs_t disp_debugfs = {0};

bool is_enable_debug_log(void)
{
	if (disp_debugfs.debug_log_enabled)
		return true;
	else
		return false;
}

u32 mi_get_backlight_log_mask(void)
{
	return disp_debugfs.backlight_log_mask;
}

static int mi_disp_debugfs_debug_log_init(void)
{
	struct disp_core *disp_core = mi_get_disp_core();
	int ret = 0;

	if (!disp_core) {
		DISP_ERROR("invalid disp_display or disp_core ptr\n");
		return -EINVAL;
	}

	if (disp_debugfs.debug_log_initialized) {
		DISP_DEBUG("debugfs entry %s already created, return!\n", DEBUG_LOG_DEBUGFS_NAME);
		return 0;
	}

	debugfs_create_bool(DEBUG_LOG_DEBUGFS_NAME, 0644, disp_core->debugfs_dir,
			&disp_debugfs.debug_log_enabled);

	disp_debugfs.debug_log_initialized = true;
	DISP_INFO("create debugfs %s success!\n", DEBUG_LOG_DEBUGFS_NAME);

	return ret;
}

static int mi_disp_debugfs_backlight_log_init(void)
{
	struct disp_core *disp_core = mi_get_disp_core();


	if (!disp_core) {
		DISP_ERROR("invalid disp_display or disp_core ptr\n");
		return -EINVAL;
	}

	if (disp_debugfs.backlight_log_initialized) {
		DISP_DEBUG("debugfs entry %s already created, return!\n", BACKLIGHT_LOG_DEBUGFS_NAME);
		return 0;
	}

	debugfs_create_u32(BACKLIGHT_LOG_DEBUGFS_NAME, 0644,
		disp_core->debugfs_dir, &disp_debugfs.backlight_log_mask);

	disp_debugfs.backlight_log_initialized = true;
	DISP_INFO("create debugfs %s success!\n", BACKLIGHT_LOG_DEBUGFS_NAME);


	return 0;
}

static int mi_disp_debugfs_esd_sw_show(struct seq_file *m, void *data)
{
	struct disp_display *dd_ptr =  m->private;

	seq_printf(m, "parameter: %s\n", "1 or true");
	seq_printf(m, "for example: echo 1 > /d/mi_display/esd_sw_%s\n",
			(dd_ptr->disp_id == MI_DISP_PRIMARY) ? "prim" : "sec");
	seq_puts(m, "\n");

	return 0;
}

static int mi_disp_debugfs_esd_sw_open(struct inode *inode, struct file *file)
{
	struct disp_display *dd_ptr = inode->i_private;

	return single_open(file, mi_disp_debugfs_esd_sw_show, dd_ptr);
}

static ssize_t mi_disp_debugfs_esd_sw_write(struct file *file,
			const char __user *p, size_t count, loff_t *ppos)
{
	struct seq_file *m = file->private_data;
	struct disp_display *dd_ptr =  m->private;
	char *input;
	int ret = 0;

	if (dd_ptr->intf_type != MI_INTF_DSI) {
		DISP_ERROR("unsupported display(%s intf)\n",
			get_disp_intf_type_name(dd_ptr->intf_type));
		return -EINVAL;
	}

	input = kmalloc(count + 1, GFP_KERNEL);
	if (!input)
		return -ENOMEM;

	if (copy_from_user(input, p, count)) {
		DISP_ERROR("copy from user failed\n");
		ret = -EFAULT;
		goto exit;
	}
	input[count] = '\0';
	DISP_INFO("[esd-test]intput: %s\n", input);

	if (!strncmp(input, "1", 1) || !strncmp(input, "on", 2) ||
		!strncmp(input, "true", 4))
		DISP_INFO("[esd-test]panel esd irq trigging\n");
	else
		goto exit;

exit:
	kfree(input);
	return ret ? ret : count;
}

static const struct file_operations esd_sw_debugfs_fops = {
	.owner = THIS_MODULE,
	.open  = mi_disp_debugfs_esd_sw_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = mi_disp_debugfs_esd_sw_write,
};

static int mi_disp_debugfs_esd_sw_init(void *d_display, int disp_id)
{
	struct disp_display *dd_ptr = (struct disp_display *)d_display;
	struct disp_core *disp_core = mi_get_disp_core();
	int ret = 0;

	if (!dd_ptr || !disp_core) {
		DISP_ERROR("invalid disp_display or disp_core ptr\n");
		return -EINVAL;
	}

	if (dd_ptr->intf_type != MI_INTF_DSI) {
		DISP_ERROR("unsupported %s display(%s intf)\n", get_disp_id_name(disp_id),
			get_disp_intf_type_name(dd_ptr->intf_type));
		return -EINVAL;
	}

	if (is_support_disp_id(disp_id)) {
		disp_debugfs.esd_sw[disp_id] = debugfs_create_file(esd_sw_str[disp_id],
			0222, disp_core->debugfs_dir,
			dd_ptr, &esd_sw_debugfs_fops);

		if (IS_ERR_OR_NULL(disp_debugfs.esd_sw[disp_id])) {
			DISP_ERROR("create debugfs entry failed for %s\n", esd_sw_str[disp_id]);
			ret = -ENODEV;
		} else {
			DISP_INFO("create debugfs %s success!\n", esd_sw_str[disp_id]);
			ret = 0;
		}
	} else {
		DISP_ERROR("unknown display id\n");
		ret = -EINVAL;
	}

	return ret;
}

static int mi_disp_debugfs_esd_sw_deinit(void *d_display, int disp_id)
{
	struct disp_display *dd_ptr = (struct disp_display *)d_display;
	int ret = 0;

	if (!dd_ptr) {
		DISP_ERROR("invalid disp_display ptr\n");
		return -EINVAL;
	}

	if (dd_ptr->intf_type != MI_INTF_DSI) {
		DISP_ERROR("unsupported %s display(%s intf)\n", get_disp_id_name(disp_id),
			get_disp_intf_type_name(dd_ptr->intf_type));
		return -EINVAL;
	}

	if (is_support_disp_id(disp_id)) {
		debugfs_remove(disp_debugfs.esd_sw[disp_id]);
		disp_debugfs.esd_sw[disp_id] = NULL;
	} else {
		DISP_ERROR("unknown display id\n");
		ret = -EINVAL;
	}

	return ret;
}

int mi_disp_debugfs_init(void *d_display, int disp_id)
{
	int ret = 0;

	ret = mi_disp_debugfs_debug_log_init();
	ret |= mi_disp_debugfs_backlight_log_init();
	ret |= mi_disp_debugfs_esd_sw_init(d_display, disp_id);

	return ret;
}

int mi_disp_debugfs_deinit(void *d_display, int disp_id)
{
	int ret = 0;

	ret = mi_disp_debugfs_esd_sw_deinit(d_display, disp_id);

	return ret;
}
#endif
