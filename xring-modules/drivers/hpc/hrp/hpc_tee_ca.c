// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Linaro Limited
 * Copyright (C) 2021 XiaoMi, Inc.
 * Copyright (C) 2022-2025, X-Ring technologies Inc.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#if (LINUX_VERSION_MAJOR < 6 || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL < 6))
#include <linux/tee_drv.h>
#else
#include <tee_drv.h>
#endif
#include <linux/uuid.h>

#include "hpc_tee_ca.h"
#include "hpc_internal.h"
#include <linux/dma-heap.h>
#include <soc/xring/xr_dmabuf_helper.h>

#define HPC_TA_UUID \
	UUID_INIT(0x8aaaf201, 0x2480, 0x0000, 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x18)

/**
 * struct hpc_kernel_ca_private - TEE private data
 * @dev:	TEE based tee_client_device device.
 * @ctx:	TEE context handler.
 * @session:	sample_test TA session identifier.
 */
struct hpc_tee_device {
	struct tee_context *ctx;
	struct tee_shm *xr_shm;
	u32 size;
	u32 session;
};

static struct hpc_tee_device g_htdev;

static int mitee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

int hree_ca_boot(int boot_flag)
{
	struct hpc_tee_device *htdev = &g_htdev;
	struct tee_ioctl_invoke_arg inv_arg = {0};
	struct tee_param param[TEE_IOCTL_PARAM_NUM] = {0};
	int ret;

	hpcdbg("entry\n");
	inv_arg.func = TA_CMD_HPC_BOOT;
	inv_arg.session = htdev->session;
	inv_arg.num_params = TEE_IOCTL_PARAM_NUM;

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = htdev->xr_shm;
	param[0].u.memref.size = htdev->size;
	param[0].u.memref.shm_offs = 0;

	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = boot_flag;
	hpcdbg("boot_flag : %d\n", boot_flag);

	ret = tee_client_invoke_func(htdev->ctx, &inv_arg, param);
	if ((ret < 0) || (inv_arg.ret != 0)) {
		hpcerr("i_func error: %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);
		ret = -ENODEV;
	}
	hpcdbg("s_invoke_func : %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);
	hpcdbg("exit\n");

	return ret;
}
EXPORT_SYMBOL_GPL(hree_ca_boot);

int hree_ca_shutdown(void)
{
	struct hpc_tee_device *htdev = &g_htdev;
	struct tee_ioctl_invoke_arg inv_arg = {0};
	struct tee_param param[TEE_IOCTL_PARAM_NUM] = {0};
	int ret = 0;

	hpcdbg("entry\n");
	inv_arg.func = TA_CMD_HPC_SHUTDOWN;
	inv_arg.session = htdev->session;
	inv_arg.num_params = TEE_IOCTL_PARAM_NUM;

	ret = tee_client_invoke_func(htdev->ctx, &inv_arg, param);
	if ((ret < 0) || (inv_arg.ret != 0)) {
		hpcerr("i_func error: %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);
		return -EINVAL;
	}
	hpcdbg("s_invoke_func : %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);
	hpcdbg("exit\n");

	return ret;
}
EXPORT_SYMBOL_GPL(hree_ca_shutdown);

int hree_ca_load_aonfw(struct hpc_aon_device *hadev)
{
	struct hpc_tee_device *htdev = &g_htdev;
	struct tee_ioctl_invoke_arg inv_arg = {0};
	struct tee_param param[TEE_IOCTL_PARAM_NUM] = {0};
	int ret;

	hpcinfo("entry\n");
	inv_arg.func = TA_CMD_HPC_LOAD_AONFW;
	inv_arg.session = htdev->session;
	inv_arg.num_params = TEE_IOCTL_PARAM_NUM;

	if (!hadev->imgshm_va) {
		hpcerr("hadev->imgshm_va is null\n");
		return -ENOMEM;
	}

	//register shm
	hadev->imgshm_tee = tee_shm_register(htdev->ctx, (unsigned long)hadev->imgshm_va,
			hadev->imgshm_size, TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(hadev->imgshm_tee)) {
		hpcerr("tee shm register failed\n");
		return -ENOMEM;
	}

	param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = hadev->imgshm_tee;
	param[0].u.memref.size = hadev->imgshm_size;
	param[0].u.memref.shm_offs = 0;
	param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = hadev->imgsrc_sfd;
	param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[2].u.value.a = hadev->imgrun_sfd;
	param[2].u.value.b = hadev->cipherimg_size;
	hpcinfo("imgsrc_sfd : %d\n",  hadev->imgsrc_sfd);
	hpcinfo("imgrun_sfd : %d\n",  hadev->imgrun_sfd);
	hpcinfo("cipherimg_size : %d\n",  hadev->cipherimg_size);
	ret = tee_client_invoke_func(htdev->ctx, &inv_arg, param);
	if ((ret < 0) || (inv_arg.ret != 0)) {
		hpcerr("i_func error: %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);
		ret = -ENODEV;
	}
	hpcinfo("s_invoke_func : %x, %x, %x\n", ret, inv_arg.ret, inv_arg.ret_origin);

	//release shm
	if (hadev->imgshm_tee) {
		tee_shm_free(hadev->imgshm_tee);
		hadev->imgshm_tee = NULL;
	}

	hpcinfo("exit\n");

	return ret;
}
EXPORT_SYMBOL_GPL(hree_ca_load_aonfw);

static int hree_ca_probe(struct hpc_tee_device *htdev,
		struct hpc_mem_info *mem, u32 size)
{
	struct tee_ioctl_open_session_arg sess_arg = {0};
	int ret;

	hpcdbg("entry\n");
	/* Open context with OP-TEE driver */
	htdev->ctx = tee_client_open_context(NULL, mitee_ctx_match, NULL, NULL);
	if (IS_ERR(htdev->ctx)) {
		hpcerr("tee_client_open_context error\n");
		return -ENODEV;
	}
	hpcdbg("tee client open context success\n");

	/* Open session with sample_test TA */
	export_uuid(sess_arg.uuid, &HPC_TA_UUID);
	hpcdbg("export uuid success\n");
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(htdev->ctx, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		hpcerr("tee_client_open_session error: %x, %x\n", ret, sess_arg.ret);
		tee_client_close_context(htdev->ctx);
		htdev->ctx = NULL;
		return -EINVAL;
	}

	hpcdbg("tee client op en session success\n");
	htdev->session = sess_arg.session;

	htdev->xr_shm = tee_shm_register(htdev->ctx, (unsigned long)mem, size,
			TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
	if (IS_ERR(htdev->xr_shm)) {
		hpcerr("tee shm register failed\n");
		(void)tee_client_close_session(htdev->ctx, htdev->session);
		tee_client_close_context(htdev->ctx);
		return -ENOMEM;
	}
	htdev->size = size;
	hpcdbg("register shm success\n");

	hpcdbg("end\n");

	return 0;
}

static int hree_ca_remove(struct hpc_tee_device *htdev)
{
	int ret;

	hpcinfo("entry\n");
	ret = tee_client_close_session(htdev->ctx, htdev->session);
	if (ret != 0) {
		hpcerr("close session failed, %d\n", ret);
		return ret;
	}

	tee_client_close_context(htdev->ctx);
	htdev->ctx = NULL;
	hpcinfo("end\n");

	return ret;
}

int hree_ca_init(struct hpc_mem_info *mem, u32 size)
{
	struct hpc_tee_device *htdev = &g_htdev;

	return hree_ca_probe(htdev, mem, size);
}
EXPORT_SYMBOL_GPL(hree_ca_init);

int hree_ca_exit(void)
{
	struct hpc_tee_device *htdev = &g_htdev;

	return hree_ca_remove(htdev);
}
EXPORT_SYMBOL_GPL(hree_ca_exit);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC TEE CA");
MODULE_LICENSE("GPL v2");
