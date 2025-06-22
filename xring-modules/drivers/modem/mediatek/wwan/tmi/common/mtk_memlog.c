// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2023, MediaTek Inc.
 */

#include <linux/sched/clock.h>
#include <linux/version.h>
#include <linux/rtc.h>
#include "mtk_memlog.h"
#include "mtk_debug.h"
#include "mtk_debugfs.h"

#ifdef CONFIG_TX00_UT_MEMLOG
#include "ut_memlog_fake.h"
#endif

#define MTK_BUFFER_BASE_SIZE				(1 * 1024 * 1024)
#define MTK_BUDDY_MAX_SIZE					(4 * 1024 * 1024)
#define MTK_BUFFER_MIN_SIZE					MTK_BUFFER_BASE_SIZE
#define MTK_BUFFER_DIVIDE_MAX				(MTK_BUDDY_MAX_SIZE / MTK_BUFFER_BASE_SIZE)
#define MTK_MEMLOG_LINE_MAX_LENGTH			(1024)
#define MTK_DFLT_MEMLOG_ATTR_NAME_LEN		(20)
#define MTK_DFLT_REGION_NAME_HEADER_LEN		(64)
#define MTK_DFLT_REGION_TIME_HEADER_LEN		(64 * MTK_BUFFER_DIVIDE_MAX)
#define MTK_DFLT_MAX_MEMLOG_DEV_CNT			(10)
#define MTK_MAX_MEMLOG_BUF_CNT				(8)

#define TAG					"MEMLOG"

static unsigned long mtk_memlog_mask = 0xffffffff;
static unsigned int mtk_memlog_resize = 4;			// 原来是2
static bool mtk_memlog_clear_enable;
static DEFINE_IDA(memlog_dev_ids);

/* total size: 4MB - time info header length*/
static struct mtk_memlog_region_cfg region_cfg[] = {
	{"REGION_0", MTK_MEMLOG_RG_0, 0x100000 - MTK_DFLT_REGION_TIME_HEADER_LEN,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_ONESHOT},
	{"REGION_1", MTK_MEMLOG_RG_1, 0xC0000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING},
	{"REGION_2", MTK_MEMLOG_RG_2, 0x80000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_3", MTK_MEMLOG_RG_3, 0x80000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_4", MTK_MEMLOG_RG_4, 0x8000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING },
	{"REGION_5", MTK_MEMLOG_RG_5, 0x40000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_ONESHOT | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_6", MTK_MEMLOG_RG_6, 0x8000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_7", MTK_MEMLOG_RG_7, 0x8000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_8", MTK_MEMLOG_RG_8, 0x8000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING | MTK_MEMLOG_F_EXCLUSIVE},
	{"REGION_9", MTK_MEMLOG_RG_9, 0x60000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING},
	{"REGION_10", MTK_MEMLOG_RG_10, 0x80000,
		MTK_MEMLOG_F_ADDINFO | MTK_MEMLOG_F_RING},
};

#if IS_ENABLED(CONFIG_MTK_AEE_IPANIC)
extern int mrdump_mini_add_extra_file(unsigned long vaddr, unsigned long paddr,
				      unsigned long size, const char *name);
#else
static int mrdump_mini_add_extra_file(unsigned long vaddr, unsigned long paddr,
				      unsigned long size, const char *name)
{
	return 0;
}
#endif

void mtk_memlog_reset(struct mtk_md_dev *mdev, enum mtk_memlog_region_id region_id)
{
	struct mtk_memlog_region *region;
	struct mtk_memlog *mtk_mlog;
	unsigned char buf_idx;

	if (!test_bit(region_id, &mtk_memlog_mask))
		return;

	if (unlikely(region_id >= ARRAY_SIZE(region_cfg)))
		return;

	mtk_mlog = mdev->memlog;
	if (unlikely(!mtk_mlog))
		return;

	region = &mtk_mlog->region[region_id];

	if (!(region->flag & MTK_MEMLOG_F_EXCLUSIVE)) {
		spin_lock_bh(&region->lock);
		for (buf_idx = 0; buf_idx < mtk_mlog->total_buf_cnt; buf_idx++) {
			memset(mtk_mlog->buffer[buf_idx] + region->base_offset, 0, region->len);
			region->pos = 0;
		}
		spin_unlock_bh(&region->lock);
	} else {
		for (buf_idx = 0; buf_idx < mtk_mlog->total_buf_cnt; buf_idx++) {
			memset(mtk_mlog->buffer[buf_idx] + region->base_offset, 0, region->len);
			region->pos = 0;
		}
	}
}

void mtk_memlog_update_time(struct mtk_memlog *mtk_mlog)
{
	struct timespec64 ts64 = {0};
	struct rtc_time rt;
	u64 ts_s, ts_us;

	ktime_get_real_ts64(&ts64);
	ts64.tv_sec = ts64.tv_sec - (time64_t)sys_tz.tz_minuteswest * 60;
	rtc_time64_to_tm(ts64.tv_sec, &rt);
	ts_s = ktime_to_us(ktime_get_boottime());
	ts_us = do_div(ts_s, USEC_PER_SEC);
	snprintf(mtk_mlog->buffer[0], MTK_DFLT_REGION_TIME_HEADER_LEN >> mtk_mlog->buf_divide,
		 "[%d-%02d-%02d %02d:%02d:%02d.%03d],[%5lu.%06lu]\n",
		 rt.tm_year + 1900, rt.tm_mon + 1, rt.tm_mday,
		 rt.tm_hour, rt.tm_min, rt.tm_sec, (unsigned int)ts64.tv_nsec / 1000,
		 (unsigned long)ts_s, (unsigned long)ts_us);
}

static void mtk_memlog_write_actual(struct mtk_memlog *mtk_mlog, struct mtk_memlog_region *region,
				    const char *fmt, ...)
{
	u32 actual_write, can_be_write, write_len = 0;
	unsigned char cur_buf_idx, next_buf_idx;
	u64 ts_s, ts_us;
	va_list args;
	int this_cpu;

	if (region->flag & MTK_MEMLOG_F_ADDINFO) {
		ts_s = ktime_to_us(ktime_get_boottime());
		ts_us = do_div(ts_s, USEC_PER_SEC);
		this_cpu = get_cpu();
		put_cpu();
		write_len = snprintf(region->tmp_log, MTK_MEMLOG_LINE_MAX_LENGTH,
				    "[%5lu.%06lu](%d)[%d:%s]",
				    (unsigned long)ts_s, (unsigned long)ts_us, this_cpu,
				    current->pid, current->comm);
	}

	va_start(args, fmt);
	write_len += vsnprintf(region->tmp_log + write_len,
		MTK_MEMLOG_LINE_MAX_LENGTH - write_len, fmt, args);
	va_end(args);

	cur_buf_idx = region->buf_idx;
	if (region->pos + write_len > region->len) {
		if (cur_buf_idx + 1 == mtk_mlog->total_buf_cnt) {
			mtk_memlog_update_time(mtk_mlog);
			next_buf_idx = 0;
		} else {
			next_buf_idx = cur_buf_idx + 1;
		}
	}

	if (region->flag & MTK_MEMLOG_F_ONESHOT) {
		if (cur_buf_idx + 1 < mtk_mlog->total_buf_cnt &&
		    region->pos + write_len > region->len) {
			can_be_write = region->len - region->pos;
			memcpy(mtk_mlog->buffer[cur_buf_idx] + region->base_offset +
				   region->pos, region->tmp_log, can_be_write);
			region->pos = 0;
			memcpy(mtk_mlog->buffer[next_buf_idx] + region->base_offset +
			       region->pos, region->tmp_log + can_be_write,
			       write_len - can_be_write);
			region->pos += write_len - can_be_write;
			region->buf_idx = next_buf_idx;
		} else {
			can_be_write = region->len - region->pos;
			actual_write = min(can_be_write, write_len);
			memcpy(mtk_mlog->buffer[cur_buf_idx] + region->base_offset +
			       region->pos, region->tmp_log, actual_write);
			region->pos += actual_write;
		}
	} else if (region->flag & MTK_MEMLOG_F_RING) {
		if (region->pos + write_len > region->len) {
			can_be_write = region->len - region->pos;
			memcpy(mtk_mlog->buffer[cur_buf_idx] + region->base_offset +
			       region->pos, region->tmp_log, can_be_write);
			region->pos = 0;
			memcpy(mtk_mlog->buffer[next_buf_idx] + region->base_offset +
			       region->pos, region->tmp_log + can_be_write,
			       write_len - can_be_write);
			region->pos += write_len - can_be_write;
			region->buf_idx = next_buf_idx;
		} else {
			memcpy(mtk_mlog->buffer[cur_buf_idx] + region->base_offset +
			       region->pos, region->tmp_log, write_len);
			region->pos += write_len;
		}
	}
}

void mtk_memlog_write(struct mtk_md_dev *mdev,
		      enum mtk_memlog_region_id region_id, const char *fmt, ...)
{
	struct mtk_memlog_region *region;
	struct mtk_memlog *mtk_mlog;
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	if (!test_bit(region_id, &mtk_memlog_mask))
		return;

	if (unlikely(region_id >= ARRAY_SIZE(region_cfg)))
		return;

	if (unlikely(!mdev))
		return;

	mtk_mlog = mdev->memlog;
	if (unlikely(!mtk_mlog))
		return;

	region = &mtk_mlog->region[region_id];

	va_start(args, fmt);
	vaf.va = &args;
	if (!(region->flag & MTK_MEMLOG_F_EXCLUSIVE)) {
		spin_lock_bh(&region->lock);
		mtk_memlog_write_actual(mtk_mlog, region, "%pV", &vaf);
		spin_unlock_bh(&region->lock);
	} else {
		mtk_memlog_write_actual(mtk_mlog, region, "%pV", &vaf);
	}
	va_end(args);
}

static int mtk_memlog_proc_open(struct inode *inode, struct file *file)
{
#if (KERNEL_VERSION(5, 17, 0) <= LINUX_VERSION_CODE)
	struct mtk_memlog *mtk_mlog = pde_data(inode);
#else
	struct mtk_memlog *mtk_mlog = PDE_DATA(inode);
#endif

	file->private_data = mtk_mlog;
	nonseekable_open(inode, file);

	return 0;
}

static int mtk_memlog_proc_close(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static ssize_t mtk_memlog_proc_read(struct file *file, char __user *buf,
				    size_t size, loff_t *ppos)
{
	struct mtk_memlog *mtk_mlog = file->private_data;
	u32 buf_offset, total_left;
	size_t buf_left, to_read;
	unsigned char buf_idx;
	int ret;

	if (*ppos >= mtk_mlog->len)
		return 0;

	total_left = mtk_mlog->len - *ppos;
	buf_idx = *ppos / mtk_mlog->single_buf_size;
	buf_offset = *ppos % mtk_mlog->single_buf_size;
	buf_left = total_left % mtk_mlog->single_buf_size;
	if (buf_left == 0)
		buf_left = mtk_mlog->single_buf_size;

	to_read = min(buf_left, size);
	/* Last log line may be incomplete since the spin lock is not held here */
	ret = copy_to_user(buf, mtk_mlog->buffer[buf_idx] + buf_offset, to_read);
	if (ret) {
		MTK_ERR(mtk_mlog->mdev, "Failed to copy data to user ret=%d", ret);
		return -EFAULT;
	}

	*ppos += to_read;
	return to_read;
}

static __poll_t mtk_memlog_proc_poll(struct file *file, struct poll_table_struct *poll)
{
	__poll_t mask;

	mask = EPOLLIN | EPOLLRDNORM;
	return mask;
}

#if (KERNEL_VERSION(5, 6, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops mtk_memlog_fops = {
	.proc_open = mtk_memlog_proc_open,
	.proc_release = mtk_memlog_proc_close,
	.proc_read = mtk_memlog_proc_read,
	.proc_poll = mtk_memlog_proc_poll,
};
#else
static const struct file_operations mtk_memlog_fops = {
	.open = mtk_memlog_proc_open,
	.release = mtk_memlog_proc_close,
	.read = mtk_memlog_proc_read,
	.poll = mtk_memlog_proc_poll,
};
#endif

static ssize_t memlog_dbg_ctrl(void *data, const char *buf, ssize_t cnt)
{
	struct mtk_memlog *mtk_mlog = data;
	int err, cmd, i;

	err = kstrtoint(buf, 10, &cmd);
	if (err)
		goto err_exit;

	switch (cmd) {
	case 1:
		for (i = 0; i < MTK_MEMLOG_RG_CNT; i++)
			mtk_memlog_reset(mtk_mlog->mdev, i);
		break;
	default:
		break;
	}

err_exit:
	return cnt;
}

MTK_DBGFS(memlog_ctrl, NULL, memlog_dbg_ctrl);

static inline void mtk_memlog_dbgfs_init(struct mtk_memlog *mtk_mlog)
{
#define MEMLOG_DBGFS_NAME_LEN	32
	char name[MEMLOG_DBGFS_NAME_LEN] = {0};

	snprintf(name, MEMLOG_DBGFS_NAME_LEN, "tmi_log_ctrl%d", mtk_mlog->dev_id);
	mtk_mlog->dentry = mtk_dbgfs_create_dir(mtk_get_dev_dentry(mtk_mlog->mdev), name);
	if (!mtk_mlog->dentry)
		return;

	mtk_dbgfs_create_file(mtk_mlog->dentry, &mtk_dbgfs_memlog_ctrl, mtk_mlog);
}

static inline void mtk_memlog_dbgfs_exit(struct mtk_memlog *mtk_mlog)
{
	mtk_dbgfs_remove(mtk_mlog->dentry);
}

static int mtk_memlog_alloc_buffer(struct mtk_memlog *mtk_mlog,
				   unsigned char orig_buf_cnt, u32 orig_buf_size)
{
	unsigned char buf_cnt;
	int kmalloced_buffer = 1;

retry_alloc:
	buf_cnt = 0;
	while (buf_cnt < orig_buf_cnt << mtk_mlog->buf_divide) {
		if (kmalloced_buffer) {
			mtk_mlog->buffer[buf_cnt] = kzalloc(orig_buf_size >>
						    mtk_mlog->buf_divide, GFP_KERNEL);
		} else {
			mtk_mlog->buffer[buf_cnt] = vzalloc(orig_buf_size >>
							    mtk_mlog->buf_divide);
		}
		if (unlikely(!mtk_mlog->buffer[buf_cnt])) {
			MTK_ERR(mtk_mlog->mdev, "Failed to create memlog buffer, length:%d\n",
				orig_buf_size >> mtk_mlog->buf_divide);
			for (; buf_cnt > 0; buf_cnt--) {
				if (kmalloced_buffer)
					kfree(mtk_mlog->buffer[buf_cnt - 1]);
				else
					vfree(mtk_mlog->buffer[buf_cnt - 1]);
			}
			mtk_mlog->buf_divide++;
			if ((orig_buf_size >> mtk_mlog->buf_divide) < MTK_BUFFER_MIN_SIZE ||
			    (orig_buf_cnt << mtk_mlog->buf_divide) > MTK_MAX_MEMLOG_BUF_CNT) {
				if (kmalloced_buffer) {
					kmalloced_buffer = 0;
					mtk_mlog->buf_divide = 0;
				} else {
					return -ENOMEM;
				}
				//goto retry_alloc; /*deleted by xiaomi for failed case*/
			}
			goto retry_alloc; /*added by xiaomi for failed case*/
		}
		buf_cnt++;
	}
	return kmalloced_buffer;
}

int mtk_memlog_init(struct mtk_md_dev *mdev)
{
	char name[MTK_DFLT_MEMLOG_ATTR_NAME_LEN];
	unsigned char buf_cnt = 0, orig_buf_cnt;
	int i, ret = 0;
	struct mtk_memlog *mtk_mlog;
	u32 pos = 0, orig_buf_size;

	if (mtk_memlog_resize < 0 || mtk_memlog_resize > 4) {
		MTK_ERR(mdev, "Failed to resize memlog, out of range, valid range:0 ~ 4\n");
		goto err_resize;
	}

	mtk_mlog = kzalloc(sizeof(*mtk_mlog) + sizeof(char *) * MTK_MAX_MEMLOG_BUF_CNT,
			   GFP_KERNEL);
	if (unlikely(!mtk_mlog)) {
		MTK_ERR(mdev, "Failed to create memlog\n");
		goto err_alloc_mlog;
	}

	mtk_mlog->mdev = mdev;
	mdev->memlog = mtk_mlog;

	mtk_mlog->buf_divide = 0;
	for (i = 0; i < ARRAY_SIZE(region_cfg); i++)
		region_cfg[i].region_size = ALIGN(region_cfg[i].region_size, L1_CACHE_BYTES);

	mtk_mlog->len = MTK_BUFFER_BASE_SIZE << mtk_memlog_resize;
	orig_buf_cnt = mtk_mlog->len / MTK_BUDDY_MAX_SIZE;
	if (!orig_buf_cnt) {
		orig_buf_cnt = 1;
		orig_buf_size = mtk_mlog->len;
	} else {
		orig_buf_size = mtk_mlog->len / orig_buf_cnt;
	}

	ret = mtk_memlog_alloc_buffer(mtk_mlog, orig_buf_cnt, orig_buf_size);
	if (ret < 0)
		goto err_alloc_buffer;

	mtk_mlog->kmalloced_buffer = ret;
	mtk_mlog->total_buf_cnt = orig_buf_cnt << mtk_mlog->buf_divide;
	mtk_mlog->single_buf_size = orig_buf_size >> mtk_mlog->buf_divide;
	while (buf_cnt < mtk_mlog->total_buf_cnt) {
		snprintf(name, MTK_DFLT_MEMLOG_ATTR_NAME_LEN, "tmi_buffer%d", buf_cnt);
		mrdump_mini_add_extra_file((unsigned long)mtk_mlog->buffer[buf_cnt],
					   mtk_mlog->kmalloced_buffer ?
					   __pa_nodebug(mtk_mlog->buffer[buf_cnt]) : 0,
					   mtk_mlog->single_buf_size, name);
		MTK_INFO(mdev, "memlog buffer[%d] addr:%p, size:%dKB\n", buf_cnt,
			 mtk_mlog->buffer[buf_cnt], mtk_mlog->single_buf_size >> 10);
		buf_cnt++;
	}

	mtk_memlog_update_time(mtk_mlog);

	pos += MTK_DFLT_REGION_TIME_HEADER_LEN >> mtk_mlog->buf_divide;
	for (i = 0; i < ARRAY_SIZE(region_cfg); i++) {
		mtk_mlog->region[i].base_offset = pos;
		buf_cnt = 0;
		while (buf_cnt < mtk_mlog->total_buf_cnt) {
			snprintf(mtk_mlog->buffer[buf_cnt] + mtk_mlog->region[i].base_offset,
				 MTK_DFLT_REGION_NAME_HEADER_LEN, "\n==========%s 0x%x==========\n",
				 region_cfg[i].name, region_cfg[i].flag);
			buf_cnt++;
		}
		mtk_mlog->region[i].base_offset += MTK_DFLT_REGION_NAME_HEADER_LEN;
		mtk_mlog->region[i].tmp_log = kzalloc(MTK_MEMLOG_LINE_MAX_LENGTH, GFP_KERNEL);
		if (unlikely(!mtk_mlog->region[i].tmp_log)) {
			MTK_ERR(mdev, "Failed to create memlog tmp line buffer\n");
			goto err_memlog;
		}

		mtk_mlog->region[i].buf_idx = 0;
		mtk_mlog->region[i].flag = region_cfg[i].flag;
		mtk_mlog->region[i].len = (region_cfg[i].region_size /
					  (MTK_BUDDY_MAX_SIZE / mtk_mlog->single_buf_size)) -
					  MTK_DFLT_REGION_NAME_HEADER_LEN;
		if (!(mtk_mlog->region[i].flag & MTK_MEMLOG_F_EXCLUSIVE))
			spin_lock_init(&mtk_mlog->region[i].lock);
		pos += mtk_mlog->region[i].len + MTK_DFLT_REGION_NAME_HEADER_LEN;
	}

	mtk_mlog->dev_id = ida_alloc_range(&memlog_dev_ids, 0,
					   MTK_DFLT_MAX_MEMLOG_DEV_CNT - 1,
					   GFP_KERNEL);
	snprintf(name, MTK_DFLT_MEMLOG_ATTR_NAME_LEN, "tmi_log%d", mtk_mlog->dev_id);

	if (mtk_memlog_clear_enable)
		mtk_memlog_dbgfs_init(mtk_mlog);
	mtk_mlog->proc_entry = proc_create_data(name, 0444, NULL, &mtk_memlog_fops, mtk_mlog);
	if (unlikely(!mtk_mlog->proc_entry)) {
		MTK_ERR(mdev, "Failed to create memlog proc entry\n");
		goto err_memlog;
	}

	return 0;

err_memlog:
	ida_free(&memlog_dev_ids, mtk_mlog->dev_id);
	for (i--; i >= 0; i--)
		kfree(mtk_mlog->region[i].tmp_log);
	for (buf_cnt = 0; buf_cnt < mtk_mlog->total_buf_cnt; buf_cnt++) {
		if (mtk_mlog->kmalloced_buffer)
			kfree(mtk_mlog->buffer[buf_cnt]);
		else
			vfree(mtk_mlog->buffer[buf_cnt]);
	}
err_alloc_buffer:
	kfree(mtk_mlog);
err_resize:
err_alloc_mlog:
	return -ENOMEM;
}

void mtk_memlog_exit(struct mtk_md_dev *mdev)
{
	struct mtk_memlog *mtk_mlog;
	unsigned char buf_cnt;
	int i;

	mtk_mlog = mdev->memlog;
	if (mtk_memlog_clear_enable)
		mtk_memlog_dbgfs_exit(mtk_mlog);
	ida_free(&memlog_dev_ids, mtk_mlog->dev_id);
	proc_remove(mtk_mlog->proc_entry);
	for (i = 0; i < ARRAY_SIZE(region_cfg); i++)
		kfree(mtk_mlog->region[i].tmp_log);
	for (buf_cnt = 0; buf_cnt < mtk_mlog->total_buf_cnt; buf_cnt++) {
		if (mtk_mlog->kmalloced_buffer)
			kfree(mtk_mlog->buffer[buf_cnt]);
		else
			vfree(mtk_mlog->buffer[buf_cnt]);
	}
	kfree(mtk_mlog);
}

module_param(mtk_memlog_resize, uint, 0644);
MODULE_PARM_DESC(mtk_memlog_resize, "The value is used to resize tmi log buffer.");
module_param(mtk_memlog_mask, ulong, 0644);
MODULE_PARM_DESC(mtk_memlog_mask, "The value is used to control mtk tmi log.");
module_param(mtk_memlog_clear_enable, bool, 0644);
MODULE_PARM_DESC(mtk_memlog_clear_enable, "This value is used to enable memlog clear\n");
