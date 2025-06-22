/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2023, MediaTek Inc.
 */

#ifndef __MTK_MEMLOG_H__
#define __MTK_MEMLOG_H__

#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include "mtk_dev.h"

#define MTK_DFLT_MEMLOG_NAME_LEN			(20)

/* enum mtk_memlog_region_id - enumerate the ID of region.
 * @MTK_MEMLOG_RG_0: common region for MTK_DBG log.
 * @MTK_MEMLOG_RG_1: all TX data traffic log.
 * @MTK_MEMLOG_RG_2: rxq0 data traffic log.
 * @MTK_MEMLOG_RG_3: rxq1 data traffic log.
 * @MTK_MEMLOG_RG_4: bat and doorbell regain, low frequency of use.
 * @MTK_MEMLOG_RG_5: dump data information.
 * @MTK_MEMLOG_RG_6: IRQ_SRC_DPMAIF irq tophalf log.
 * @MTK_MEMLOG_RG_7: IRQ_SRC_DPMAIF2 irq tophalf log.
 * @MTK_MEMLOG_RG_8: IRQ_SRC_DPMAIF3 irq tophalf log.
 * @MTK_MEMLOG_RG_9: dump ctrl information.
 * @MTK_MEMLOG_RG_10: ctrl TX/RX packet info.
 */
enum mtk_memlog_region_id {
	MTK_MEMLOG_RG_0 = 0,
	MTK_MEMLOG_RG_1,
	MTK_MEMLOG_RG_2,
	MTK_MEMLOG_RG_3,
	MTK_MEMLOG_RG_4,
	MTK_MEMLOG_RG_5,
	MTK_MEMLOG_RG_6,
	MTK_MEMLOG_RG_7,
	MTK_MEMLOG_RG_8,
	MTK_MEMLOG_RG_9,
	MTK_MEMLOG_RG_10,
	MTK_MEMLOG_RG_CNT,
};

enum mtk_memlog_flag {
	MTK_MEMLOG_F_ADDINFO = BIT(0),
	MTK_MEMLOG_F_EXCLUSIVE = BIT(1),
	MTK_MEMLOG_F_RING = BIT(2),
	MTK_MEMLOG_F_ONESHOT = BIT(3),
};

struct mtk_memlog_region {
	u32 base_offset;
	u32 pos;
	u32 len;
	unsigned char buf_idx;
	unsigned int flag;
	char *tmp_log;
	/* protects the buffer write operation */
	spinlock_t lock;
};

struct mtk_memlog_region_cfg {
	char name[MTK_DFLT_MEMLOG_NAME_LEN];
	enum mtk_memlog_region_id region_id;
	u32 region_size;
	enum mtk_memlog_flag flag;
};

struct mtk_memlog {
	struct mtk_memlog_region region[MTK_MEMLOG_RG_CNT];
	struct proc_dir_entry *proc_entry;
	struct mtk_md_dev *mdev;
	struct dentry *dentry;
	int dev_id;
	unsigned char buf_divide;
	unsigned char total_buf_cnt;
	bool kmalloced_buffer;
	u32 single_buf_size;
	u32 len;
	char *buffer[];
};

void mtk_memlog_write(struct mtk_md_dev *mdev, enum mtk_memlog_region_id region_id,
		      const char *fmt, ...);
int mtk_memlog_init(struct mtk_md_dev *mdev);
void mtk_memlog_exit(struct mtk_md_dev *mdev);
void mtk_memlog_reset(struct mtk_md_dev *mdev, enum mtk_memlog_region_id region_id);

#endif

