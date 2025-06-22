/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_RPMB_H__
#define __UFS_XRING_RPMB_H__

#include "ufshcd.h"

#if IS_ENABLED(CONFIG_XRING_UFS_RPMB)
void ufs_xring_rpmb_probe(struct ufs_hba *hba);
void ufs_xring_rpmb_remove(struct ufs_hba *hba);
#else
void ufs_xring_rpmb_probe(struct ufs_hba *hba) {}
void ufs_xring_rpmb_remove(struct ufs_hba *hba) {}
#endif

#endif
