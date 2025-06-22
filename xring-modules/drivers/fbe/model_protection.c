// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 * Description: FBE Kernel CA source file
 * Modify time: 2023-11-15
 * Author: Security-TEEOS
 */

#define pr_fmt(fmt) "ModelProtect: [%s %d] " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/uuid.h>
#include <tee_drv.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/model_protection.h>


#define MODEL_PROTECTION_TA_UUID \
	UUID_INIT(0xefff4308, 0xe6cb, 0x42b0, 0xac, 0x5a, 0xa9, 0x7e, 0xee, 0x95, 0x9d, 0xc5)


static int tee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	(void)data;

	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int open_context_and_session(struct tee_context **ctx, struct tee_ioctl_open_session_arg *arg)
{
	int ret;

	/* Open Context */
	*ctx = tee_client_open_context(NULL, tee_ctx_match, NULL, NULL);
	if (IS_ERR(*ctx)) {
		*ctx = NULL;
		pr_err("tee_client_open_context failed\n");
		return -ENODEV;
	}

	/* Open Session */
	export_uuid(arg->uuid, &MODEL_PROTECTION_TA_UUID);
	arg->clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;

	ret = tee_client_open_session(*ctx, arg, NULL);
	if ((ret != 0) || (arg->ret != 0)) {
		pr_err("tee_client_open_session failed with code 0x%x origin 0x%x\n",
		       arg->ret, arg->ret_origin);
		tee_client_close_context(*ctx);
		*ctx = NULL;
		return -EINVAL;
	}

	return 0;
}

static void close_session_and_context(struct tee_context *ctx, struct tee_ioctl_open_session_arg *arg)
{
	if (ctx != NULL) {
		tee_client_close_session(ctx, arg->session);
		tee_client_close_context(ctx);
	}
}

static int alloc_shared_mem(struct tee_context *ctx,
			    const u8 *data,
			    u32 size,
			    u8 **pbuf,
			    struct tee_shm **pshm)
{
	errno_t errno;

	*pbuf = kzalloc(size, GFP_KERNEL);
	if (*pbuf == NULL) {
		pr_err("kzalloc failed\n");
		return -ENOMEM;
	}

	*pshm = tee_shm_register(ctx,
				 (unsigned long)*pbuf,
				 size,
				 TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(*pshm)) {
		kfree(*pbuf);
		pr_err("tee_shm_register failed\n");
		return -EINVAL;
	}

	if (data != NULL) {
		errno = memcpy_s(*pbuf, size, data, size);
		if (errno != EOK) {
			tee_shm_free(*pshm);
			kfree(*pbuf);
			pr_err("memcpy_s failed\n");
			return errno;
		}
	}

	return 0;
}

static void free_shared_mem(u8 *buf, struct tee_shm *shm)
{
	tee_shm_free(shm);
	kfree(buf);
}

static void set_invoke_arg(struct tee_ioctl_invoke_arg *arg, enum xing_model_protection_cmd_id cmd, u32 session)
{
	memset_s(arg, sizeof(struct tee_ioctl_invoke_arg), 0, sizeof(struct tee_ioctl_invoke_arg));
	arg->func = cmd;
	arg->session = session;
	arg->num_params = TEE_NUM_PARAMS;
}

static void set_invoke_param(struct tee_param *param,
			     struct tee_shm *header_shm,
			     size_t header_size,
			     struct tee_shm *key_shm,
			     size_t key_size)
{
	memset_s(param, sizeof(struct tee_param) * TEE_NUM_PARAMS, 0, sizeof(struct tee_param) * TEE_NUM_PARAMS);

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = header_shm;
	param[0].u.memref.size = header_size;
	param[0].u.memref.shm_offs = 0;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param[1].u.memref.shm = key_shm;
	param[1].u.memref.size = key_size;
	param[1].u.memref.shm_offs = 0;
}

int get_model_key(const u8 *header, size_t header_size, u8 *key, size_t *key_size)
{
	int ret;
	errno_t errno;
	struct tee_context *tee_ctx = NULL;
	struct tee_ioctl_open_session_arg sess_arg = { 0 };
	struct tee_ioctl_invoke_arg inv_arg = { 0 };
	struct tee_param param[TEE_NUM_PARAMS];
	struct tee_shm *header_shm = NULL;
	struct tee_shm *key_shm = NULL;
	u8 *header_buf = NULL;
	u8 *key_buf = NULL;

	if (header == NULL || key == NULL || key_size == NULL || header_size == 0 || *key_size == 0) {
		pr_err("Invalid parameter: header %s NULL, header_size = %zu, key %s NULL, key_size %s NULL\n",
		       (header == NULL) ? "=" : "!=", header_size, (key == NULL) ? "=" : "!=",
		       (key_size == NULL) ? "=" : "!=");
		if (key_size != NULL)
			pr_err("*key_size = %zu\n", *key_size);
		return -EINVAL;
	}

	ret = open_context_and_session(&tee_ctx, &sess_arg);
	if (ret != 0) {
		pr_err("open_context_and_session failed\n");
		return ret;
	}

	ret = alloc_shared_mem(tee_ctx, header, header_size, &header_buf, &header_shm);
	if (ret != 0) {
		pr_err("alloc_shared_mem failed\n");
		goto error;
	}

	ret = alloc_shared_mem(tee_ctx, NULL, *key_size, &key_buf, &key_shm);
	if (ret != 0) {
		pr_err("alloc_shared_mem failed\n");
		free_shared_mem(header_buf, header_shm);
		goto error;
	}

	set_invoke_arg(&inv_arg, TEEC_CMD_GET_MODEL_KEY, sess_arg.session);
	set_invoke_param(param, header_shm, header_size, key_shm, *key_size);

	ret = tee_client_invoke_func(tee_ctx, &inv_arg, param);
	if ((ret != 0) || ((inv_arg.ret != 0))) {
		pr_err("tee_client_invoke_func failed with code 0x%x origin 0x%x\n",
		       inv_arg.ret, inv_arg.ret_origin);
		ret = -EINVAL;
	} else {
		errno = memcpy_s(key, *key_size, key_buf, param[1].u.memref.size);
		if (errno != EOK) {
			pr_err("memcpy_s failed\n");
			ret = -EINVAL;
		} else {
			*key_size = param[1].u.memref.size;
		}
	}

	free_shared_mem(key_buf, key_shm);
	free_shared_mem(header_buf, header_shm);

error:
	close_session_and_context(tee_ctx, &sess_arg);
	return ret;
}
EXPORT_SYMBOL(get_model_key);
