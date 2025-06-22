// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include "mtk_common.h"
#include "mtk_dev.h"
#include "mtk_debug.h"

int mtk_pcimsg_send_msg_to_user(struct mtk_md_dev *mdev, int msg_id)
{
	return 0;
}

int mtk_pcimsg_messenger_init(struct mtk_md_dev *mdev)
{
	return 0;
}

int mtk_pcimsg_messenger_exit(struct mtk_md_dev *mdev)
{
	return 0;
}

bool mtk_pcimsg_pci_user_is_busy(struct mtk_md_dev *mdev)
{
	return false;
}
