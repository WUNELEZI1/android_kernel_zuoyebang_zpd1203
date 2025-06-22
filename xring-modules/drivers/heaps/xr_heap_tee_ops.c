// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring Secure opearter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the XRING heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/arm-smccc.h>
#include <soc/xring/xhee_status.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>

#include "xr_heap_tee_ops.h"

#define PAGE_CONTAIN_ITEM_MAX_NUM	((PAGE_SIZE - sizeof(struct xhee_page_header)) / sizeof(struct xhee_each_item))

struct xhee_page_header {
	u64 batch_size;
	u64 padding;
};

struct xhee_each_item {
	u64 addr; /* physical address */
	u64 nr_pages; /* continuous */
};

struct xhee_page_content {
	struct xhee_page_header header;
	struct xhee_each_item items[];
};

#ifdef CONFIG_XRING_SECURE_HEAP
static struct tee_info *tee_info;

static int mitee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
	if (ver->impl_id == TEE_IMPL_ID_OPTEE)
		return 1;
	else
		return 0;
}

static int secmem_tee_exec_cmd(struct tee_context *context, u32 session,
		       struct mem_chunk_list *mcl, u32 cmd)
{
	struct tee_ioctl_invoke_arg inv_arg;
	struct tee_shm *xr_shm = NULL;
	struct tee_param param[4];
	int ret;

	memset(&inv_arg, 0, sizeof(inv_arg));
	memset(&param, 0, sizeof(param));

	inv_arg.func = cmd;
	inv_arg.session = session;
	inv_arg.num_params = 4;

	switch (cmd) {
	case XRING_SEC_CMD_ALLOC:
	case XRING_SEC_CMD_ALLOC_TEE:
	case XRING_SEC_CMD_TABLE_SET:
	case XRING_SEC_CMD_SEND_HEAPINFO:
		/* register share memory for ree and tee */
		xrheap_debug("size: 0x%x\n", mcl->size);
		xr_shm = tee_shm_register(context, (unsigned long)mcl->phys_addr, mcl->size,
							TEE_SHM_DMA_BUF | TEE_SHM_KERNEL_MAPPED);
		if (IS_ERR(xr_shm)) {
			xrheap_err("xr_shm: 0x%lx, tee_shm_register failed\n",
				(unsigned long)xr_shm);
			ret = -ENOMEM;
			goto fail;
		}

		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
		param[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
		param[0].u.memref.shm = xr_shm;
		param[0].u.memref.size = mcl->size;
		param[0].u.memref.shm_offs = 0;
		param[1].u.value.a = mcl->nents;
		param[1].u.value.b = mcl->type;
		break;
	case XRING_SEC_CMD_FREE:
	case XRING_SEC_CMD_FREE_TEE:
	case XRING_SEC_CMD_TABLE_CLEAN:
		param[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
		param[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;
		param[0].u.value.a = mcl->buff_id;
		break;
	default:
		xrheap_err("invalid cma\n");
		ret = -EINVAL;
		goto fail;
	}

	xrheap_info("size: 0x%zx, nents: 0x%llx, type: %llu, cmd: 0x%x\n",
		param[0].u.memref.size, param[1].u.value.a, param[1].u.value.b, cmd);
	ret = tee_client_invoke_func(context, &inv_arg, param);
	if ((ret < 0) || ((inv_arg.ret != 0))) {
		xrheap_err("tee_client_invoke_func failed: %x, %x\n", ret, inv_arg.ret);
		ret = -EINVAL;
		goto free_shm;
	}

	/* return sfd if cmd is ALLOC, ALLOC_TEE, TABLE_SET	*/
	if ((cmd & XRING_TEE_ALLOC) != 0) {
		mcl->buff_id = param[3].u.value.b;
		xrheap_debug("sfd: 0x%x\n", mcl->buff_id);
	}

	/* according tee return value adjust success or fail */
	if (param[3].u.value.a != 0)
		xrheap_err("Exec TEE CMD fail\n");
	else
		xrheap_info("Exec TEE CMD success\n");

free_shm:
	if (xr_shm != NULL)
		tee_shm_free(xr_shm);
fail:
	return ret;
}

static int fill_one_page_for_hvc(u64 *page_virt, unsigned long long page_phys, int batch_size,
		struct tz_info *info, int start_nent, int xhee_cmd)
{
	int i;
	struct arm_smccc_res res = { 0 };
	struct xhee_page_content *content = (struct xhee_page_content *)page_virt;

	content->header.batch_size = batch_size;
	for (i = 0; i < batch_size; i++) {
		content->items[i].addr = info->pageinfo[start_nent + i].addr;
		content->items[i].nr_pages = info->pageinfo[start_nent + i].nr_pages;
	}

	arm_smccc_hvc(xhee_cmd, page_phys, PAGE_SIZE, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		xrheap_err("xhee_cmd:%d execute fail, ret=%lu\n", xhee_cmd, res.a0);

	return res.a0;
}

int xr_xhee_action(struct tz_info *info, int xhee_cmd)
{
	struct page *page;
	void *page_virt;
	int cur;
	unsigned long remaining;
	int batch_size;
	int ret = 0;

	if (!xhee_status_enabled())
		return 0;

	if (!info) {
		xrheap_err("info is NULL\n");
		return -EFAULT;
	}

	page = alloc_pages(GFP_KERNEL | __GFP_ZERO, 0);
	if (!page)
		return -ENOMEM;
	page_virt = kmap_local_page(page);

	remaining = info->nents;
	cur = 0;

	while (remaining) {
		batch_size = min(remaining, PAGE_CONTAIN_ITEM_MAX_NUM);
		ret = fill_one_page_for_hvc(page_virt, page_to_phys(page), batch_size, info, cur, xhee_cmd);
		if (ret) {
			ret = -EINVAL;
			break;
		}

		cur += batch_size;
		remaining -= batch_size;
	}

	kunmap_local(page_virt);
	__free_pages(page, 0);

	return ret;
}

int xr_change_mem_prot_tee(struct tz_info *info, u32 cmd)
{
	struct mem_chunk_list mcl;
	int ret;

	memset(&mcl, 0, sizeof(mcl));

	/* protect or not */
	mcl.type = (info->type == CPA_HEAP) ? 1 : 0;
	if ((cmd & XRING_TEE_ALLOC) != 0) {
		mcl.phys_addr = (void *)info->pageinfo;
		mcl.nents = info->nents;
		mcl.size = info->size;
	} else {
		mcl.buff_id = info->sfd;
		mcl.phys_addr = NULL;
	}

	ret = secmem_tee_exec_cmd(tee_info->context, tee_info->session, &mcl, cmd);
	if (ret < 0) {
		xrheap_err("exec cmd[%d] fail\n", cmd);
		return ret;
	}

	if ((cmd & XRING_TEE_ALLOC) != 0) {
		info->sfd = mcl.buff_id;
		xrheap_debug("sfd: 0x%x\n", info->sfd);
	}

	return 0;
}

int xr_send_heapinfo(struct xring_secure_heap_data *xring_secure_heap_list, int nents)
{
	struct mem_chunk_list mcl;
	int ret;

	memset(&mcl, 0, sizeof(mcl));

	mcl.type = 0;
	mcl.phys_addr = xring_secure_heap_list;
	mcl.nents = nents;
	mcl.size = nents * sizeof(*xring_secure_heap_list);

	ret = secmem_tee_exec_cmd(tee_info->context, tee_info->session,
				&mcl, XRING_SEC_CMD_SEND_HEAPINFO);
	if (ret < 0)
		xrheap_err("exec cmd[%d] fail\n", XRING_SEC_CMD_SEND_HEAPINFO);

	return ret;
}
EXPORT_SYMBOL(xr_send_heapinfo);

int xr_secmem_tee_init(void)
{
	struct tee_ioctl_open_session_arg sess_arg;
	int ret;

	tee_info = kzalloc(sizeof(struct tee_info), GFP_KERNEL);
	if (!tee_info) {
		ret = -ENOMEM;
		goto fail;
	}

	tee_info->context = tee_client_open_context(NULL, mitee_ctx_match, NULL, NULL);
	if (IS_ERR(tee_info->context)) {
		xrheap_err("open context failed\n");
		ret = -ENODEV;
		goto fail_context;
	}

	memset(&sess_arg, 0, sizeof(sess_arg));
	export_uuid(sess_arg.uuid, &TA_UUID);
	sess_arg.clnt_login = TEE_IOCTL_LOGIN_PUBLIC;
	sess_arg.num_params = 0;

	ret = tee_client_open_session(tee_info->context, &sess_arg, NULL);
	if ((ret < 0) || (sess_arg.ret != 0)) {
		xrheap_err("ret: %x, open session failed: %x\n", ret, sess_arg.ret);
		ret = -EINVAL;
		goto fail_session;
	}

	tee_info->session = sess_arg.session;
	xrheap_info("TA init success!\n");
	return 0;

fail_session:
	tee_client_close_context(tee_info->context);
fail_context:
	kfree(tee_info);
fail:
	return ret;
}

void xr_secmem_tee_destroy(void)
{
	if (!tee_info->context || !tee_info->session) {
		xrheap_err("invalid context or session\n");
		return;
	}

	tee_client_close_session(tee_info->context, tee_info->session);
	tee_client_close_context(tee_info->context);
	kfree(tee_info);
	xrheap_info("TA closed !\n");
}
#else
int xr_secmem_tee_init(void)
{
	return 0;
}

void xr_secmem_tee_destroy(void)
{
}

int xr_change_mem_prot_tee(struct tz_info *info, u32 cmd)
{
	return 0;
}

int xr_send_heapinfo(struct xring_secure_heap_data *xring_secure_heap_list, int nents)
{
	return 0;
}
#endif
MODULE_LICENSE("GPL v2");
