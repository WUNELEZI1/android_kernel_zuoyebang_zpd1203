// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright(c) 2024 X-Ring technologies Inc.
 */
#include <linux/f2fs_fs.h>

#include "f2fs.h"
#include "segment.h"

#define SSR_HD_SAPCE_LIMIT		(8 << 20)
#define SSR_HD_WATERLINE		(65)
#define SSR_WD_SAPCE_LIMIT		(5 << 20)
#define SSR_WD_WATERLINE		(60)
#define SSR_HD_MAX_FILESIZE		(1 << 16)
#define SSR_WD_MAX_FILESIZE		(1 << 17)

static bool f2fs_need_flexible_ssr(struct f2fs_sb_info *sbi, int type)
{
	u64 total_blocks = MAIN_SEGS(sbi) << sbi->log_blocks_per_seg;
	u64 valid_blocks = sbi->total_valid_block_count;
	u64 remaining_space = (total_blocks - valid_blocks) << 2;
	u64 kbs_per_seg = 1 << (sbi->log_blocks_per_seg + 2);
	unsigned int ovp_segs = overprovision_segments(sbi);
	unsigned int free_segs = free_segments(sbi);
	unsigned int lower_limit = 0;
	unsigned int waterline = 0;

	if(f2fs_lfs_mode(sbi))
		return false;

	if(f2fs_need_SSR(sbi))
		return false;

	if (type == CURSEG_HOT_DATA) {
		lower_limit = sbi->flexible_ssr_params.hot_data_lower_limit;
		waterline = sbi->flexible_ssr_params.hot_data_waterline;
	} else if (type == CURSEG_WARM_DATA) {
		lower_limit = sbi->flexible_ssr_params.warm_data_lower_limit;
		waterline = sbi->flexible_ssr_params.warm_data_waterline;
	} else {
		return false;
	}

	if (remaining_space > (u64)lower_limit)
		return false;

	return 100 * (u64)(free_segs - ovp_segs) * kbs_per_seg <=
							remaining_space * (u64)waterline;
}

void f2fs_init_flexible_ssr_params(struct f2fs_sb_info *sbi)
{
	sbi->flexible_ssr_params.hot_data_lower_limit = SSR_HD_SAPCE_LIMIT;
	sbi->flexible_ssr_params.hot_data_waterline = SSR_HD_WATERLINE;
	sbi->flexible_ssr_params.warm_data_lower_limit = SSR_WD_SAPCE_LIMIT;
	sbi->flexible_ssr_params.warm_data_waterline = SSR_WD_WATERLINE;
	sbi->flexible_ssr_params.hot_data_max_filesize = SSR_HD_MAX_FILESIZE;
	sbi->flexible_ssr_params.warm_data_max_filesize = SSR_WD_MAX_FILESIZE;

	if (f2fs_need_flexible_ssr(sbi, CURSEG_HOT_DATA_FLEX)) {
		f2fs_info(sbi, "Enable hot_data_ssr!");
		sbi->flexible_ssr_params.hot_data_ssr_enable = FLEXIBLE_SSR_ON;
	} else {
		f2fs_info(sbi, "Disable hot_data_ssr!");
		sbi->flexible_ssr_params.hot_data_ssr_enable = FLEXIBLE_SSR_OFF;
	}

	if (f2fs_need_flexible_ssr(sbi, CURSEG_WARM_DATA_FLEX)) {
		f2fs_info(sbi, "Enable warm_data_ssr!");
		sbi->flexible_ssr_params.warm_data_ssr_enable = FLEXIBLE_SSR_ON;
	} else {
		f2fs_info(sbi, "Disable warm_data_ssr!");
		sbi->flexible_ssr_params.warm_data_ssr_enable = FLEXIBLE_SSR_OFF;
	}
}

int __f2fs_init_flexible_ssr_curseg(struct f2fs_sb_info *sbi,
				int type, int seg_type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	int ret = 0;

	f2fs_down_read(&SM_I(sbi)->curseg_lock);

	mutex_lock(&curseg->curseg_mutex);
	down_write(&SIT_I(sbi)->sentry_lock);

	curseg->seg_type = seg_type;

	if (get_ssr_segment(sbi, type, SSR, 0)) {
		struct seg_entry *se = get_seg_entry(sbi, curseg->next_segno);

		curseg->seg_type = se->type;
		ret = change_curseg(sbi, type);
	} else {
		curseg->seg_type = seg_type;
		ret = new_curseg(sbi, type, true);
	}
	stat_inc_seg_type(sbi, curseg);

	up_write(&SIT_I(sbi)->sentry_lock);
	mutex_unlock(&curseg->curseg_mutex);

	f2fs_up_read(&SM_I(sbi)->curseg_lock);
	return ret;
}

static void f2fs_flexible_ssr_curseg_release(struct f2fs_sb_info *sbi, int type)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	int segno = curseg->segno;

	f2fs_down_read(&SM_I(sbi)->curseg_lock);
	mutex_lock(&curseg->curseg_mutex);

	reset_curseg_fields(curseg);

	locate_dirty_segment(sbi, segno);

	mutex_unlock(&curseg->curseg_mutex);
	f2fs_up_read(&SM_I(sbi)->curseg_lock);
}

void f2fs_flexible_ssr_close(struct f2fs_sb_info *sbi)
{
	if (sbi->flexible_ssr_params.hot_data_ssr_enable == FLEXIBLE_SSR_ON) {
		sbi->flexible_ssr_params.hot_data_ssr_enable = FLEXIBLE_SSR_OFF;
		f2fs_flexible_ssr_curseg_release(sbi, CURSEG_HOT_DATA_FLEX);
	}

	if (sbi->flexible_ssr_params.warm_data_ssr_enable == FLEXIBLE_SSR_ON) {
		sbi->flexible_ssr_params.warm_data_ssr_enable = FLEXIBLE_SSR_OFF;
		f2fs_flexible_ssr_curseg_release(sbi, CURSEG_WARM_DATA_FLEX);
	}
}

int f2fs_flexible_ssr_switch(struct f2fs_sb_info *sbi)
{
	int ret = 0;

	if (sbi->flexible_ssr_params.hot_data_ssr_enable == FLEXIBLE_SSR_OFF &&
			f2fs_need_flexible_ssr(sbi, CURSEG_HOT_DATA)) {
		ret = __f2fs_init_flexible_ssr_curseg(sbi, CURSEG_HOT_DATA_FLEX,
												CURSEG_HOT_DATA);
		if (ret) {
			f2fs_err(sbi, "F2FS Flexible HOT DATA SSR failed to enable");
			return ret;
		}
		sbi->flexible_ssr_params.hot_data_ssr_enable = FLEXIBLE_SSR_ON;
	} else if (sbi->flexible_ssr_params.hot_data_ssr_enable == FLEXIBLE_SSR_ON &&
			!f2fs_need_flexible_ssr(sbi, CURSEG_HOT_DATA)) {
		sbi->flexible_ssr_params.hot_data_ssr_enable = FLEXIBLE_SSR_OFF;
		f2fs_flexible_ssr_curseg_release(sbi, CURSEG_HOT_DATA_FLEX);
	}

	if (sbi->flexible_ssr_params.warm_data_ssr_enable == FLEXIBLE_SSR_OFF &&
			f2fs_need_flexible_ssr(sbi, CURSEG_WARM_DATA)) {
		ret = __f2fs_init_flexible_ssr_curseg(sbi, CURSEG_WARM_DATA_FLEX,
												CURSEG_WARM_DATA);
		if (ret) {
			f2fs_err(sbi, "F2FS Flexible WARM DATA SSR failed to enable");
			return ret;
		}
		sbi->flexible_ssr_params.warm_data_ssr_enable = FLEXIBLE_SSR_ON;
	} else if (sbi->flexible_ssr_params.warm_data_ssr_enable == FLEXIBLE_SSR_ON &&
			!f2fs_need_flexible_ssr(sbi, CURSEG_WARM_DATA)) {
		sbi->flexible_ssr_params.warm_data_ssr_enable = FLEXIBLE_SSR_OFF;
		f2fs_flexible_ssr_curseg_release(sbi, CURSEG_WARM_DATA_FLEX);
	}

	return ret;
}

bool f2fs_can_flexible_ssr(struct f2fs_sb_info *sbi,
				int type, loff_t filesize)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	unsigned int ssr_enable = 0;
	loff_t ssr_max_filesize = 0;

	if (type == CURSEG_HOT_DATA) {
		ssr_enable = sbi->flexible_ssr_params.hot_data_ssr_enable;
		ssr_max_filesize = (loff_t)sbi->flexible_ssr_params.hot_data_max_filesize;
	} else if (type == CURSEG_WARM_DATA) {
		ssr_enable = sbi->flexible_ssr_params.warm_data_ssr_enable;
		ssr_max_filesize = (loff_t)sbi->flexible_ssr_params.warm_data_max_filesize;
	} else {
		return false;
	}

	if(!ssr_enable)
		return false;

	if (curseg->alloc_type == SSR)
		return false;

	if (!filesize || filesize > ssr_max_filesize)
		return false;

	return true;
}
