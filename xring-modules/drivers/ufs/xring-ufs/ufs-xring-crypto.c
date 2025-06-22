// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Xring, Inc. All rights reserved.
 */
#include <linux/platform_device.h>
#include <linux/blk-crypto.h>
#include <ufs/ufshcd.h>
#include "ufs-xring-crypto.h"
#include "soc/xring/fbe_ca.h"
#include "ufshcd-priv.h"

#define MINIMUM_DUN_SIZE 512
#define MAXIMUM_DUN_SIZE 65536

/* Blk-crypto modes supported by UFS crypto */
static const struct ufs_crypto_alg_entry {
	enum ufs_crypto_alg ufs_alg;
	enum ufs_crypto_key_size ufs_key_size;
} ufs_crypto_algs[BLK_ENCRYPTION_MODE_MAX] = {
	[BLK_ENCRYPTION_MODE_AES_256_XTS] = {
		.ufs_alg = UFS_CRYPTO_ALG_AES_XTS,
		.ufs_key_size = UFS_CRYPTO_KEY_SIZE_256,
	},
};

static int ufshcd_crypto_xring_keyslot_program(struct blk_crypto_profile *profile,
					const struct blk_crypto_key *key,
					unsigned int slot)
{
	struct ufs_hba *hba = (struct ufs_hba *)container_of(profile, struct ufs_hba, crypto_profile);
	u8 data_unit_mask = -1;
	const struct ufs_crypto_alg_entry *alg;
	const union ufs_crypto_cap_entry *ccap_array = hba->crypto_cap_array;
	enum xring_ice_cipher_alg ca_cipher = -1;
	int err;
	int i;
	int cap_idx = -1;

	if (!ufshcd_is_hba_active(hba)) {
		dev_warn(hba->dev, "%s ufs hc is not activate\n", __func__);
		return 0;
	}

	data_unit_mask = key->crypto_cfg.data_unit_size / MINIMUM_DUN_SIZE;
	alg = &ufs_crypto_algs[key->crypto_cfg.crypto_mode];
	if (alg->ufs_key_size == UFS_CRYPTO_KEY_SIZE_256) {
		ca_cipher = XRING_ICE_CIPHER_AES_256_XTS;
	} else {
		dev_err(hba->dev, "alg keysize if invalid\n");
		return -EINVAL;
	}

	BUILD_BUG_ON(UFS_CRYPTO_KEY_SIZE_INVALID != 0);
	for (i = 0; i < hba->crypto_capabilities.num_crypto_cap; i++) {
		if (ccap_array[i].algorithm_id == alg->ufs_alg &&
			ccap_array[i].key_size == alg->ufs_key_size &&
			(ccap_array[i].sdus_mask & data_unit_mask)) {
			cap_idx = i;
			break;
		}
	}

	if (WARN_ON(cap_idx < 0))
		return -EOPNOTSUPP;

	err = xring_ice_program_key(cap_idx, slot, key->raw, key->size, ca_cipher,
								key->crypto_cfg.data_unit_size);
	if (err) {
		dev_err(hba->dev, "ufs xring crypt program key fail.\n");
		return err;
	}

	return err;
}

static int ufshcd_crypto_xring_keyslot_evict(struct blk_crypto_profile *profile,
						const struct blk_crypto_key *key,
						unsigned int slot)
{
	struct ufs_hba *hba = (struct ufs_hba *)container_of(profile, struct ufs_hba, crypto_profile);
	int err = 0;

	down(&hba->host_sem);
	if (!ufshcd_is_user_access_allowed(hba)) {
		err = -EBUSY;
		goto out;
	}

	if (!ufshcd_is_hba_active(hba)) {
		dev_warn_once(hba->dev, "%s ufs hc is not activate\n", __func__);
		goto out;
	}

	err = xring_ice_evict_key(slot);
	if (err) {
		dev_err(hba->dev, "ufs xring crypt evit key fail,slot=%u.\n", slot);
		goto out;
	}

out:
	up(&hba->host_sem);
	return err;
}

static int ufshcd_crypto_xring_derive_raw_secret(struct blk_crypto_profile *profile,
						const u8 *wrapped_key,
						size_t wrapped_key_size,
						u8 secret[BLK_CRYPTO_SW_SECRET_SIZE])
{
	struct ufs_hba *hba = (struct ufs_hba *)container_of(profile, struct ufs_hba, crypto_profile);
	int err;
	u32 derive_secret_size = BLK_CRYPTO_SW_SECRET_SIZE;

	err = xring_derive_sw_secret(wrapped_key, wrapped_key_size, secret, &derive_secret_size);
	if (err) {
		dev_err(hba->dev, "ufs xring crypt derive raw key fail.\n");
		return err;
	}

	return err;
}

static const struct blk_crypto_ll_ops ufshcd_xring_ksm_ops = {
	.keyslot_program	= ufshcd_crypto_xring_keyslot_program,
	.keyslot_evict		= ufshcd_crypto_xring_keyslot_evict,
	.derive_sw_secret	= ufshcd_crypto_xring_derive_raw_secret,
};

static enum blk_crypto_mode_num
ufshcd_find_blk_crypto_mode(union ufs_crypto_cap_entry cap)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ufs_crypto_algs); i++) {
		BUILD_BUG_ON(UFS_CRYPTO_KEY_SIZE_INVALID != 0);
		if (ufs_crypto_algs[i].ufs_alg == cap.algorithm_id &&
			ufs_crypto_algs[i].ufs_key_size == cap.key_size) {
			return i;
		}
	}
	return BLK_ENCRYPTION_MODE_INVALID;
}

/**
 * ufshcd_xring_hba_init_crypto_capabilities - Read crypto capabilities, init crypto
 *					 fields in hba
 * @hba: Per adapter instance
 *
 * Return: 0 if crypto was initialized or is not supported, else a -errno value.
 */
int ufshcd_xring_hba_init_crypto_capabilities(struct ufs_hba *hba)
{
	int cap_idx;
	int err;
	enum blk_crypto_mode_num blk_mode_num;

	/*
	 * Don't use crypto if either the hardware doesn't advertise the
	 * standard crypto capability bit *or* if the vendor specific driver
	 * hasn't advertised that crypto is supported.
	 */

	if (!(ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES) &
		MASK_CRYPTO_SUPPORT)) {
		err = -EOPNOTSUPP;
		dev_err(hba->dev, "Ufs hc doesn't support crypto\n");
		goto out;
	}
	if (!(hba->caps & UFSHCD_CAP_CRYPTO)) {
		err = -EOPNOTSUPP;
		dev_err(hba->dev, "Ufs doesn't support hw crypto\n");
		goto out;
	}

	hba->crypto_capabilities.reg_val =
			cpu_to_le32(ufshcd_readl(hba, REG_UFS_CCAP));
	/* x-CRYPTOCONFIG(#0)addrs */
	hba->crypto_cfg_register =
		(u32)hba->crypto_capabilities.config_array_ptr * 0x100;
	hba->crypto_cap_array =
		devm_kcalloc(hba->dev, hba->crypto_capabilities.num_crypto_cap,
				sizeof(hba->crypto_cap_array[0]), GFP_KERNEL);
	if (IS_ERR(hba->crypto_cap_array)) {
		dev_err(hba->dev, "Alloc crypto cap array fail\n");
		err = -ENOMEM;
		goto out;
	}

	/* The actual number of configurations supported is (CFGC+1) */
	err = devm_blk_crypto_profile_init(hba->dev, &hba->crypto_profile,
			hba->crypto_capabilities.config_count + 1);
	if (err)
		goto out;

	hba->crypto_profile.ll_ops = ufshcd_xring_ksm_ops;
	/* UFS only supports 8 bytes for any DUN */
	hba->crypto_profile.max_dun_bytes_supported = 8;
	hba->crypto_profile.key_types_supported = BLK_CRYPTO_KEY_TYPE_HW_WRAPPED;
	hba->crypto_profile.dev = hba->dev;

	/*
	 * Cache all the UFS crypto capabilities and advertise the supported
	 * crypto modes and data unit sizes to the block layer.
	 */
	for (cap_idx = 0; cap_idx < hba->crypto_capabilities.num_crypto_cap;
	     cap_idx++) {
		hba->crypto_cap_array[cap_idx].reg_val =
			cpu_to_le32(ufshcd_readl(hba,
						 REG_UFS_CRYPTOCAP +
						 cap_idx * sizeof(__le32)));
		blk_mode_num = ufshcd_find_blk_crypto_mode(
						hba->crypto_cap_array[cap_idx]);
		if (blk_mode_num != BLK_ENCRYPTION_MODE_INVALID)
			hba->crypto_profile.modes_supported[blk_mode_num] |=
				hba->crypto_cap_array[cap_idx].sdus_mask * 512;
	}

	return 0;

out:
	/* Indicate that init failed by clearing UFSHCD_CAP_CRYPTO */
	hba->caps &= ~UFSHCD_CAP_CRYPTO;
	return err;
}
EXPORT_SYMBOL(ufshcd_xring_hba_init_crypto_capabilities);
