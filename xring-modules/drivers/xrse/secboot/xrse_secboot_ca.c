// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "../xrse_internal.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/version.h>
#include <tee_drv.h>
#include <linux/uuid.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>
#include <dt-bindings/xring/platform-specific/xrse_secboot.h>
#include "soc/xring/xrse/secboot_verify.h"

#define ADDR_NUM 2
#define TEE_NUM_PARAMS          4
#define TA_UUID \
	UUID_INIT(0x82d80280, 0x2185, 0x4e69, 0xb5, 0x05, 0x9a, 0xcc, 0x9b, 0x32, 0xf8, 0x7e)
#define TEE_ERROR_TARGET_DEAD    0xFFFF3024

/**
 * struct sample_kernel_ca_private - TEE private data
 * @ctx:    TEE context handler.
 * @session:    TA session identifier.
 */
struct secboot_verify_ca_private {
	struct tee_context *ctx;
	u32 session;
};

static struct secboot_verify_ca_private g_sbver_prv;

static int mitee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int initialize_private_data(void)
{
	struct tee_ioctl_open_session_arg sess_arg;
	int ret = 0;

	(void)memset_s(&sess_arg, sizeof(sess_arg), 0, sizeof(sess_arg));

	/* Step1. Open context with MITEE driver. */
	g_sbver_prv.ctx = tee_client_open_context(NULL, mitee_ctx_match, NULL, NULL);
	if (IS_ERR(g_sbver_prv.ctx)) {
		xrse_err("tee_client_open_context error\n");
		return -ENODEV;
	}

	/* Step2. Open session with secboot_verify TA */
	export_uuid(sess_arg.uuid, &TA_UUID);
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(g_sbver_prv.ctx, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		xrse_err("tee_client_open_session error: %x, %x\n", ret, sess_arg.ret);
		tee_client_close_context(g_sbver_prv.ctx);
		return -EINVAL;
	}

	g_sbver_prv.session = sess_arg.session;

	return 0;
}

static void finish_private_data(void)
{
	tee_client_close_session(g_sbver_prv.ctx, g_sbver_prv.session);
	tee_client_close_context(g_sbver_prv.ctx);
}

static void secboot_pack_params(struct tee_param *param, struct tee_shm *sb_shm, u32 sb_size, u32 img_id, u32 subimg_id)
{
	(void)memset_s(param, sizeof(struct tee_param) * TEE_NUM_PARAMS, 0, sizeof(struct tee_param) * TEE_NUM_PARAMS);
	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = sb_shm;
	param[0].u.memref.size = sb_size;
	param[0].u.memref.shm_offs = 0;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = img_id;
	param[1].u.value.b = subimg_id;
}

static void secboot_set_invoke_arg(struct tee_ioctl_invoke_arg *arg, u8 cmd)
{
	(void)memset_s(arg, sizeof(struct tee_ioctl_invoke_arg), 0, sizeof(struct tee_ioctl_invoke_arg));
	arg->func = cmd;
	arg->session = g_sbver_prv.session;
	arg->num_params = TEE_NUM_PARAMS;
}

static inline void *kzalloc_page_align(size_t size, gfp_t flags)
{
	uint32_t num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	void *ptr;

	ptr = kzalloc(num_pages * PAGE_SIZE, flags);
	if (!ptr)
		return NULL;

	return ptr;
}

static int secboot_alloc_shared_mem(struct sb_addr_info *data,
				u32 size,
				u8 **pbuf,
				struct tee_shm **pshm)
{
	*pbuf = (u8 *)kzalloc_page_align(size, GFP_KERNEL);
	if (*pbuf == NULL) {
		xrse_err("kzalloc_page_align() failed\n");
		return -ENOMEM;
	}

	*pshm = tee_shm_register(g_sbver_prv.ctx,
				 (unsigned long)*pbuf,
				 size,
				 TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(*pshm)) {
		kfree(*pbuf);
		xrse_err("tee_shm_register() failed\n");
		return -EINVAL;
	}

	if (data != NULL)
		(void)memcpy_s(*pbuf, size, data, size);

	return 0;
}

static void secboot_free_shared_mem(u8 *buf, struct tee_shm *shm)
{
	tee_shm_free(shm);
	kfree(buf);
}

static int xrse_secboot_verify_core(u8 cmd, u32 image_id, u32 subimage_id,
				const struct sb_addr_info *src, const struct sb_addr_info *dst)
{
	int ret;
	u8 *sb_buf = NULL;
	struct tee_shm *sb_shm = NULL;
	int shm_length;
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_param param[TEE_NUM_PARAMS];
	bool retried = false;

	/* Step1. Prepare the verify params. */
	struct sb_addr_info addrs[ADDR_NUM] = {
		{.addr = src->addr,
		 .size = src->size,
		 .in_ddr = src->in_ddr},
		{.addr = dst->addr,
		 .size = dst->size,
		 .in_ddr = dst->in_ddr}
	};

	shm_length = sizeof(struct sb_addr_info) * ADDR_NUM;

	/* Step2. Alloc shared memory for secboot boot verify params */
	ret = secboot_alloc_shared_mem(addrs, shm_length, &sb_buf, &sb_shm);
	if (ret != 0) {
		xrse_err("fail to alloc shared mem\n");
		return ret;
	}

	/* Step3. Prepare tee params for communicate with TA. */
	secboot_set_invoke_arg(&inv_arg, cmd);
	secboot_pack_params(param, sb_shm, shm_length, image_id, subimage_id);

retry:
	if (retried) {
		ret = initialize_private_data();
		if (ret) {
			xrse_err("initialize_private_data failed with code 0x%x\n", ret);
			goto free_mem;
		}
	}

	/* Step4. Communicate with TA. */
	ret = tee_client_invoke_func(g_sbver_prv.ctx, &inv_arg, param);
	if ((ret != 0) || ((inv_arg.ret != 0))) {
		xrse_err("tee_client_invoke_func() failed with code 0x%x origin 0x%x\n",
			   inv_arg.ret, inv_arg.ret_origin);
		ret = -EINVAL;
		if ((inv_arg.ret == TEE_ERROR_TARGET_DEAD) && !retried) {
			retried = true;
			finish_private_data();
			goto retry;
		}
	}

free_mem:
	secboot_free_shared_mem(sb_buf, sb_shm);

	return ret;
}

#define SUBIMG_ID_MAX (6)
static int xrse_secboot_check_input(u8 cmd, u8 image_id, u8 subimage_id,
				const struct sb_addr_info *src, const struct sb_addr_info *dst)
{
	if (cmd < IPC_SECBOOT_CMD_VERIFY_SINGLE_IMG || cmd > IPC_SECBOOT_CMD_VERIFY_MULTI_SUBIMG_FINAL) {
		xrse_err("cmd:%d not supported!\n", cmd);
		return -EINVAL;
	}

	if (image_id <= SEC_IMG_ID_XRSE_FIRMWARE || image_id >= SEC_IMG_ID_MAX) {
		xrse_err("image_id:%d not supported!\n", image_id);
		return -EINVAL;
	}

	if (subimage_id >= SUBIMG_ID_MAX) {
		xrse_err("subimage_id:%d not supported!\n", subimage_id);
		return -EINVAL;
	}

	if (src == NULL || dst == NULL) {
		xrse_err("invalid addr pointer!\n");
		return -EINVAL;
	}

	if (src->in_ddr > SECBOOT_ADDR_TYPE_DDR || dst->in_ddr > SECBOOT_ADDR_TYPE_DDR) {
		xrse_err("invalid addr param in_ddr, src in_ddr%d:, dst in_ddr:%d!\n",
					   src->in_ddr, dst->in_ddr);
		return -EINVAL;
	}

	return 0;
}

int xrse_secboot_verify(u8 cmd, u32 image_id, u32 subimage_id,
			const struct sb_addr_info *src, const struct sb_addr_info *dst)
{
	int ret;

	ret = xrse_secboot_check_input(cmd, image_id, subimage_id, src, dst);
	if (ret) {
		xrse_err("error input!\n");
		return ret;
	}

	ret = xrse_secboot_verify_core(cmd, image_id, subimage_id, src, dst);
	if (ret) {
		xrse_err("cmd:%d, imgid:%d, subimgid:%d. verify fail!\n", cmd, image_id, subimage_id);
		return ret;
	}

	xrse_info("cmd:%d, imgid:%d, subimgid:%d. verify success!\n", cmd, image_id, subimage_id);

	return ret;
}
EXPORT_SYMBOL(xrse_secboot_verify);

int secboot_verify_ca_init(void)
{
	int ret;

	xrse_info("secboot ca init\n");

	ret = initialize_private_data();
	if (ret < 0) {
		xrse_err("initialize_private_data failed\n");
		return ret;
	}

	return ret;
}

void secboot_verify_ca_exit(void)
{
	xrse_info("secboot ca exit\n");
	finish_private_data();
}
