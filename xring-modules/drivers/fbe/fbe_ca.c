// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: FBE Kernel CA source file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": [%s %d] " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <tee_drv.h>
#include <soc/xring/fbe_ca.h>


#define FBE_TA_UUID { \
	0x64, 0x4c, 0x96, 0x48, \
	0x91, 0xe6, 0x49, 0xdb, \
	0x83, 0xca, 0x84, 0xbe, 0xc6, 0x04, 0x31, 0x08 \
}

#define TEE_ERROR_TARGET_DEAD    0xFFFF3024

static const struct tee_ioctl_open_session_arg g_sess_arg_origin = {
	.uuid = FBE_TA_UUID,
	.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL,
	.session = 0,
	.num_params = 0
};
static struct tee_ioctl_open_session_arg g_sess_arg;
static struct tee_context *g_tee_ctx;


static int tee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	(void)data;

	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int open_context_and_session(void)
{
	int ret;

	if (g_tee_ctx == NULL) {
		/* Open Context */
		g_tee_ctx = tee_client_open_context(NULL, tee_ctx_match, NULL, NULL);
		if (IS_ERR(g_tee_ctx)) {
			g_tee_ctx = NULL;
			pr_err("tee_client_open_context failed\n");
			return -ENODEV;
		}

		/* Open Session */
		g_sess_arg = g_sess_arg_origin;
		ret = tee_client_open_session(g_tee_ctx, &g_sess_arg, NULL);
		if ((ret != 0) || (g_sess_arg.ret != 0)) {
			pr_err("tee_client_open_session failed with code 0x%x origin 0x%x\n",
				g_sess_arg.ret, g_sess_arg.ret_origin);
			tee_client_close_context(g_tee_ctx);
			g_tee_ctx = NULL;
			return -EINVAL;
		}
	}

	return 0;
}

static void close_session_and_context(void)
{
	if (g_tee_ctx != NULL) {
		tee_client_close_session(g_tee_ctx, g_sess_arg.session);
		tee_client_close_context(g_tee_ctx);
		g_tee_ctx = NULL;
	}
}

static int alloc_shared_mem(struct tee_context *ctx,
			    const u8 *data,
			    u32 size,
			    u8 **pbuf,
			    struct tee_shm **pshm)
{
	/* Avoid two shared memory blocks on the same page */
	u32 roundup_size = roundup(size, PAGE_SIZE);
	*pbuf = kzalloc(roundup_size, GFP_KERNEL);
	if (*pbuf == NULL) {
		pr_err("kzalloc failed\n");
		return -ENOMEM;
	}

	*pshm = tee_shm_register(ctx,
				 (unsigned long)*pbuf,
				 roundup_size,
				 TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(*pshm)) {
		kfree(*pbuf);
		pr_err("tee_shm_register failed\n");
		return -EINVAL;
	}

	if (data != NULL)
		memcpy(*pbuf, data, size);

	return 0;
}

static void free_shared_mem(u8 *buf, struct tee_shm *shm)
{
	tee_shm_free(shm);
	kfree(buf);
}

static void set_invoke_arg(struct tee_ioctl_invoke_arg *arg, enum xing_fbe_cmd_id cmd, u32 session)
{
	memset(arg, 0, sizeof(struct tee_ioctl_invoke_arg));
	arg->func = cmd;
	arg->session = session;
	arg->num_params = TEE_NUM_PARAMS;
}

static void program_key_set_param(struct tee_param *param,
				  u32 cap_idx,
				  u32 index,
				  struct tee_shm *key_shm,
				  u32 key_size,
				  enum xring_ice_cipher_alg cipher_alg,
				  u32 data_unit_size)
{
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAMS);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = cap_idx;
	param[0].u.value.b = index;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm = key_shm;
	param[1].u.memref.size = key_size;
	param[1].u.memref.shm_offs = 0;

	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].u.value.a = cipher_alg;
	param[2].u.value.b = data_unit_size;
}

static void derive_secret_set_param(struct tee_param *param,
				    struct tee_shm *key_shm,
				    u32 key_size,
				    struct tee_shm *secret_shm,
				    u32 secret_size)
{
	memset(param, 0, sizeof(struct tee_param) * TEE_NUM_PARAMS);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = key_shm;
	param[0].u.memref.size = key_size;
	param[0].u.memref.shm_offs = 0;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param[1].u.memref.shm = secret_shm;
	param[1].u.memref.size = secret_size;
	param[1].u.memref.shm_offs = 0;
}

/****************************** FBE CA Interface ******************************/
int xring_ice_program_key(u32 cap_idx,
			  u32 index,
			  const u8 *wrapped_key,
			  u32 wrapped_key_size,
			  enum xring_ice_cipher_alg cipher_alg,
			  u32 data_unit_size)
{
	int ret;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[TEE_NUM_PARAMS];
	struct tee_shm *key_shm = NULL;
	u8 *key_buf = NULL;
	bool retried = false;

	pr_info("index = %u\n", index);
#ifdef CONFIG_XRING_FBE_TEST_MODULE
	pr_info("cap_idx = %u\n", cap_idx);
	pr_info("wrapped_key_size = %u\n", wrapped_key_size);
	pr_info("cipher_alg = %u\n", cipher_alg);
	pr_info("data_unit_size = %u\n", data_unit_size);
#endif

	if ((cap_idx >= ICE_CRY_CAP_NUM) ||
	    (index >= ICE_KEYSLOT_NUM) ||
	    (wrapped_key == NULL) ||
	    (wrapped_key_size != WRAPPED_KEY_SIZE) ||
	    ((cipher_alg != XRING_ICE_CIPHER_AES_128_XTS) &&
	     (cipher_alg != XRING_ICE_CIPHER_AES_256_XTS)) ||
	    (data_unit_size == 0)) {
		pr_err("Invalid parameter\n");
		return -EINVAL;
	}

retry:
	ret = open_context_and_session();
	if (ret != 0)
		return ret;

	ret = alloc_shared_mem(g_tee_ctx, wrapped_key, wrapped_key_size, &key_buf, &key_shm);
	if (ret != 0) {
		close_session_and_context();
		return ret;
	}

	set_invoke_arg(&inv_arg, TEEC_FBE_PROGRAM_ENCRYPTION_KEY, g_sess_arg.session);
	program_key_set_param(param,
			      cap_idx,
			      index,
			      key_shm,
			      wrapped_key_size,
			      cipher_alg,
			      data_unit_size);

	ret = tee_client_invoke_func(g_tee_ctx, &inv_arg, param);
	free_shared_mem(key_buf, key_shm);

	if ((ret != 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func failed with code 0x%x origin 0x%x\n",
		       inv_arg.ret, inv_arg.ret_origin);
		close_session_and_context();
		ret = -EINVAL;

		if ((inv_arg.ret == TEE_ERROR_TARGET_DEAD) && !retried) {
			pr_err("now retry invoke command\n");
			retried = true;
			goto retry;
		}
	}

	return ret;
}
EXPORT_SYMBOL(xring_ice_program_key);

int xring_ice_evict_key(u32 index)
{
	int ret;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[TEE_NUM_PARAMS];
	bool retried = false;

	pr_info("index = %u\n", index);

	if (index >= ICE_KEYSLOT_NUM) {
		pr_err("Invalid parameter\n");
		return -EINVAL;
	}

retry:
	ret = open_context_and_session();
	if (ret != 0)
		return ret;

	set_invoke_arg(&inv_arg, TEEC_FBE_EVICT_ENCRYPTION_KEY, g_sess_arg.session);

	memset(&param, 0, sizeof(param));
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = index;

	ret = tee_client_invoke_func(g_tee_ctx, &inv_arg, param);
	if ((ret != 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func failed with code 0x%x origin 0x%x\n",
		       inv_arg.ret, inv_arg.ret_origin);
		close_session_and_context();
		ret = -EINVAL;

		if ((inv_arg.ret == TEE_ERROR_TARGET_DEAD) && !retried) {
			pr_err("now retry invoke command\n");
			retried = true;
			goto retry;
		}
	}

	return ret;
}
EXPORT_SYMBOL(xring_ice_evict_key);

int xring_derive_sw_secret(const u8 *wrapped_key,
			   u32 wrapped_key_size,
			   u8 *secret,
			   u32 *secret_size)
{
	int ret;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[TEE_NUM_PARAMS];
	struct tee_shm *key_shm = NULL;
	struct tee_shm *secret_shm = NULL;
	u8 *key_buf = NULL;
	u8 *secret_buf = NULL;
	bool retried = false;

	pr_info("key size = %u\n", wrapped_key_size);
#ifdef CONFIG_XRING_FBE_TEST_MODULE
	pr_info("wrapped_key_size = %u\n", wrapped_key_size);
	if (secret_size != NULL)
		pr_info("*secret_size = %u\n", *secret_size);
	else
		pr_info("secret_size = NULL\n");
#endif

	if ((wrapped_key == NULL) ||
	    (wrapped_key_size == 0) ||
	    (wrapped_key_size != WRAPPED_KEY_SIZE) ||
	    (secret == NULL) ||
	    (secret_size == NULL) ||
	    (*secret_size < AES_256_KEY_SIZE)) {
		pr_err("Invalid parameter\n");
		return -EINVAL;
	}

retry:
	ret = open_context_and_session();
	if (ret != 0)
		return ret;

	ret = alloc_shared_mem(g_tee_ctx, wrapped_key, wrapped_key_size, &key_buf, &key_shm);
	if (ret != 0) {
		close_session_and_context();
		return ret;
	}

	ret = alloc_shared_mem(g_tee_ctx, NULL, KEY_MAX_SIZE, &secret_buf, &secret_shm);
	if (ret != 0) {
		free_shared_mem(key_buf, key_shm);
		close_session_and_context();
		return ret;
	}

	set_invoke_arg(&inv_arg, TEEC_FBE_DERIVE_SW_SECKET, g_sess_arg.session);
	derive_secret_set_param(param, key_shm, wrapped_key_size, secret_shm, KEY_MAX_SIZE);

	ret = tee_client_invoke_func(g_tee_ctx, &inv_arg, param);
	if ((ret == 0) && ((inv_arg.ret == 0))) {
		if (*secret_size < param[1].u.memref.size) {
			ret = -EINVAL;
		} else {
			memcpy(secret, secret_buf, param[1].u.memref.size);
			*secret_size = param[1].u.memref.size;
		}
	}

	free_shared_mem(secret_buf, secret_shm);
	free_shared_mem(key_buf, key_shm);

	if ((ret != 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func failed with code 0x%x origin 0x%x\n",
		       inv_arg.ret, inv_arg.ret_origin);
		close_session_and_context();
		ret = -EINVAL;

		if ((inv_arg.ret == TEE_ERROR_TARGET_DEAD) && !retried) {
			pr_err("now retry invoke command\n");
			retried = true;
			goto retry;
		}
	}

	return ret;
}
EXPORT_SYMBOL(xring_derive_sw_secret);

static int __init xring_fbe_ca_init(void)
{
	return 0;
}

static void __exit xring_fbe_ca_exit(void)
{
	if (g_tee_ctx != NULL)
		close_session_and_context();
}

module_init(xring_fbe_ca_init);
module_exit(xring_fbe_ca_exit);
MODULE_DESCRIPTION("XRing FBE Kernel CA");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: mitee");
