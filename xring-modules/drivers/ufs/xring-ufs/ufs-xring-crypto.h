/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __UFS_XRING_CRYPT_H
#define __UFS_XRING_CRYPT_H

#if !IS_ENABLED(CONFIG_XRING_UFS_CRYPTO)
static inline int ufshcd_xring_hba_init_crypto_capabilities(struct ufs_hba *hba)
{
	return 0;
}
#else
int ufshcd_xring_hba_init_crypto_capabilities(struct ufs_hba *hba);

#endif
#endif
