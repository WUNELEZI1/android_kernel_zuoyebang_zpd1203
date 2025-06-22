// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include "mtk_bm.h"
#include "mtk_ctrl_plane.h"
#include "mtk_data_plane.h"
#include "mtk_debug.h"
#include "mtk_debugfs.h"
#include "mtk_dev.h"
#include "mtk_devlink.h"
#include "mtk_fsm.h"

#define TAG "DEV"

int mtk_dev_init(struct mtk_md_dev *mdev)
{
	int ret;

	/* do not add error handling here,dbgfs may not support in system */
	mdev->dev_dentry = mtk_dbgfs_create_dir(mtk_get_drv_dentry(), mdev->dev_str);
	ret = mtk_memlog_init(mdev);
	if (ret)
		goto exit;

	ret = mtk_fsm_init(mdev);
	if (ret)
		goto free_dbgfs_and_memlog;

	ret = mtk_pm_init(mdev);
	if (ret)
		goto free_fsm;

	ret = mtk_bm_init(mdev);
	if (ret)
		goto free_pm;

	ret = mtk_ctrl_init(mdev);
	if (ret)
		goto free_bm;

	ret = mtk_data_init(mdev);
	if (ret)
		goto free_ctrl_plane;

	ret = mtk_devlink_init(mdev);
	if (ret)
		goto free_data_plane;

	ret = mtk_except_init(mdev);
	if (ret)
		goto free_devlink;

	return 0;
free_devlink:
	mtk_devlink_exit(mdev);
free_data_plane:
	mtk_data_exit(mdev);
free_ctrl_plane:
	mtk_ctrl_exit(mdev);
free_bm:
	mtk_bm_exit(mdev);
free_pm:
	mtk_pm_exit(mdev);
free_fsm:
	mtk_fsm_exit(mdev);
free_dbgfs_and_memlog:
	mtk_memlog_exit(mdev);
exit:
	mtk_dbgfs_remove(mdev->dev_dentry);
	return ret;
}

void mtk_dev_exit(struct mtk_md_dev *mdev)
{
	mtk_fsm_evt_submit(mdev, FSM_EVT_DEV_RM, 0, NULL, 0,
			   EVT_MODE_BLOCKING | EVT_MODE_TOHEAD);
	mtk_devlink_exit(mdev);
	mtk_pm_exit_early(mdev);
	mtk_data_exit(mdev);
	mtk_ctrl_exit(mdev);
	mtk_bm_exit(mdev);
	mtk_pm_exit(mdev);
	mtk_except_exit(mdev);
	mtk_fsm_exit(mdev);
	mtk_memlog_exit(mdev);
	mtk_dbgfs_remove(mdev->dev_dentry);
}

int mtk_dev_start(struct mtk_md_dev *mdev)
{
	mtk_fsm_evt_submit(mdev, FSM_EVT_DEV_ADD, 0, NULL, 0, 0);
	mtk_fsm_start(mdev);
	return 0;
}

void mtk_dev_dump(struct mtk_md_dev *mdev)
{
	mtk_hw_dbg_dump(mdev);
	mtk_ctrl_dump(mdev);
}
