// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, Linaro Limited
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": [%s %d] " fmt, __func__, __LINE__

#include "arm_ffa.h"
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/scatterlist.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/platform_device.h>

#include <tee_drv.h>
#include "optee_ffa.h"
#include "optee_private.h"
#include "optee_rpc_cmd.h"
#include "optee_bench.h"
#include "optee_smc.h"
#include "mitee_memlog.h"
#include "mitee_smc_notify.h"
#include "rpc_callback.h"
#include "dynamic_mem.h"

/*
 * This file implement the FF-A ABI used when communicating with secure world
 * OP-TEE OS via FF-A.
 * This file is divided into the following sections:
 * 1. Maintain a hash table for lookup of a global FF-A memory handle
 * 2. Convert between struct tee_param and struct fmem optee_msg_param
 * 3. Low level support functions to register shared memory in secure world
 * 4. Do a normal scheduled call into secure world
 * 5. FF-A Driver initialization.
 */

static struct {
	struct ffa_device *dev;
	const struct ffa_ops *ops;
	struct ffa_device_id device_id;
	struct mutex mutex;
	struct rhashtable global_ids;
} mitee_ffa_ctx = {
	.dev = NULL,
	.ops = NULL,
	/* 06bb3aea-7b38-4dad-a70ff59986572611 */
	.device_id = { UUID_INIT(0x06bb3aea, 0x7b38, 0x4dad, 0xa7, 0x0f, 0xf5,
				 0x99, 0x86, 0x57, 0x26, 0x11) },
};

/*
 * 1. Maintain a hash table for lookup of a global FF-A memory handle
 *
 * FF-A assigns a global memory handle for each piece shared memory.
 * This handle is then used when communicating with secure world.
 *
 * Main functions are optee_shm_add_ffa_handle() and optee_shm_rem_ffa_handle()
 */
struct shm_rhash {
	struct tee_shm *shm;
	u64 global_id;
	struct rhash_head linkage;
};

static void rh_free_fn(void *ptr, void *arg)
{
	kfree(ptr);
}

static const struct rhashtable_params shm_rhash_params = {
	.head_offset = offsetof(struct shm_rhash, linkage),
	.key_len = sizeof(u64),
	.key_offset = offsetof(struct shm_rhash, global_id),
	.automatic_shrinking = true,
};

static struct tee_shm *optee_shm_from_ffa_handle(struct optee *optee,
						 u64 global_id)
{
	struct tee_shm *shm = NULL;
	struct shm_rhash *r;

	mutex_lock(&mitee_ffa_ctx.mutex);
	r = rhashtable_lookup_fast(&mitee_ffa_ctx.global_ids, &global_id,
				   shm_rhash_params);
	if (r)
		shm = r->shm;
	mutex_unlock(&mitee_ffa_ctx.mutex);

	return shm;
}

static int optee_shm_add_ffa_handle(struct optee *optee, struct tee_shm *shm,
				    u64 global_id)
{
	struct shm_rhash *r;
	int rc;

	r = kmalloc(sizeof(*r), GFP_KERNEL);
	if (!r)
		return -ENOMEM;
	r->shm = shm;
	r->global_id = global_id;

	mutex_lock(&mitee_ffa_ctx.mutex);
	rc = rhashtable_lookup_insert_fast(&mitee_ffa_ctx.global_ids,
					   &r->linkage, shm_rhash_params);
	mutex_unlock(&mitee_ffa_ctx.mutex);

	if (rc)
		kfree(r);

	return rc;
}

static int optee_shm_rem_ffa_handle(u64 global_id)
{
	struct shm_rhash *r;
	int rc = -ENOENT;

	mutex_lock(&mitee_ffa_ctx.mutex);
	r = rhashtable_lookup_fast(&mitee_ffa_ctx.global_ids, &global_id,
				   shm_rhash_params);
	if (r)
		rc = rhashtable_remove_fast(&mitee_ffa_ctx.global_ids,
					    &r->linkage, shm_rhash_params);
	mutex_unlock(&mitee_ffa_ctx.mutex);

	if (!rc)
		kfree(r);

	return rc;
}

/*
 * 2. Convert between struct tee_param and struct optee_msg_param
 *
 * from_msg_param_ffa_mem() and to_msg_param_ffa_mem() are the main
 * functions.
 */
void from_msg_param_ffa_mem(struct optee *optee, struct tee_param *p, u32 attr,
				const struct optee_msg_param *mp)
{
	struct tee_shm *shm = NULL;
	u64 offs_high = 0;
	u64 offs_low = 0;

	p->attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT + attr -
		  OPTEE_MSG_ATTR_TYPE_FMEM_INPUT;
	p->u.memref.size = mp->u.fmem.size;

	if (mp->u.fmem.global_id != OPTEE_MSG_FMEM_INVALID_GLOBAL_ID)
		shm = optee_shm_from_ffa_handle(optee, mp->u.fmem.global_id);
	p->u.memref.shm = shm;

	if (shm) {
		offs_low = mp->u.fmem.offs_low;
		offs_high = mp->u.fmem.offs_high;
	}
	p->u.memref.shm_offs = offs_low | offs_high << 32;
}

int to_msg_param_ffa_mem(struct optee_msg_param *mp, const struct tee_param *p)
{
	struct tee_shm *shm = p->u.memref.shm;

	mp->attr = OPTEE_MSG_ATTR_TYPE_FMEM_INPUT + p->attr -
		   TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;

	if (shm) {
		u64 shm_offs = p->u.memref.shm_offs;

		mp->u.fmem.internal_offs = shm->offset;

		mp->u.fmem.offs_low = shm_offs;
		mp->u.fmem.offs_high = shm_offs >> 32;
		/* Check that the entire offset could be stored. */
		if (mp->u.fmem.offs_high != shm_offs >> 32) {
			pr_err("val is error\n");
			return -EINVAL;
		}

		mp->u.fmem.global_id = shm->sec_world_id;
	} else {
		memset(&mp->u, 0, sizeof(mp->u));
		mp->u.fmem.global_id = OPTEE_MSG_FMEM_INVALID_GLOBAL_ID;
	}
	mp->u.fmem.size = p->u.memref.size;

	return 0;
}

/*
 * 3. Low level support functions to register shared memory in secure world
 *
 * Functions to register and unregister shared memory both for normal
 * clients and for tee-supplicant.
 */

static int optee_ffa_shm_register(struct tee_context *ctx, struct tee_shm *shm,
				  struct page **pages, size_t num_pages,
				  unsigned long start)
{
	struct optee *optee = tee_get_drvdata(ctx->teedev);
	struct ffa_device *ffa_dev = mitee_ffa_ctx.dev;
	const struct ffa_mem_ops *mem_ops = mitee_ffa_ctx.ops->mem_ops;
	struct ffa_mem_region_attributes mem_attr = {
		.receiver = ffa_dev->vm_id,
		.attrs = FFA_MEM_RW,
	};
	struct ffa_mem_ops_args args = {
		.use_txbuf = true,
		.attrs = &mem_attr,
		.nattrs = 1,
	};
	struct sg_table sgt;
	int rc;
#ifdef MITEE_SHM_DEBUG
	int iter = 0;
#endif

	if (optee->supp_teedev == ctx->teedev) {
		pr_err("do not support supp!\n");
		return -EACCES;
	}

	rc = optee_check_mem_type(start, num_pages);
	if (rc) {
		pr_err("optee_check_mem_type is failed\n");
		return rc;
	}

#ifdef MITEE_SHM_DEBUG
	pr_info("%s dump page list\n", __func__);
	for (iter = 0; iter < num_pages; iter++)
		pr_info("page pfn: %#lx\n", page_to_pfn(pages[iter]));
#endif

	rc = sg_alloc_table_from_pages(&sgt, pages, num_pages, 0,
				       num_pages * PAGE_SIZE, GFP_KERNEL);
	if (rc) {
		pr_err("sg_alloc_table_from_pages is failed\n");
		return rc;
	}
	args.sg = sgt.sgl;
	rc = mem_ops->memory_share(&args);
	sg_free_table(&sgt);
	if (rc) {
		pr_err("memory_share is failed\n");
		return rc;
	}

	rc = optee_shm_add_ffa_handle(optee, shm, args.g_handle);
	if (rc) {
		pr_err("%s add handle failed: %d!\n", __func__, rc);
		mem_ops->memory_reclaim(args.g_handle, 0);
		return rc;
	}

#ifdef MITEE_SHM_DEBUG
	pr_info("%s get handle: %#llx!\n", __func__, args.g_handle);
#endif
	shm->sec_world_id = args.g_handle;

	return 0;
}

static int optee_ffa_shm_unregister(struct tee_context *ctx,
				    struct tee_shm *shm)
{
	struct optee *optee = tee_get_drvdata(ctx->teedev);
	const struct ffa_mem_ops *mem_ops = mitee_ffa_ctx.ops->mem_ops;
	u64 global_handle = shm->sec_world_id;
	struct optee_msg_arg *msg_arg = NULL;
	size_t sz = OPTEE_MSG_GET_ARG_SIZE(1);
	int rc;

	msg_arg = kzalloc(sz, GFP_KERNEL);
	if (!msg_arg)
		return -ENOMEM;
	msg_arg->num_params = 1;
	msg_arg->cmd = OPTEE_MSG_CMD_UNREGISTER_SHM;
	msg_arg->params[0].attr = OPTEE_MSG_ATTR_TYPE_FMEM_INPUT;
	msg_arg->params[0].u.fmem.global_id = global_handle;

	optee_shm_rem_ffa_handle(global_handle);
	shm->sec_world_id = 0;

	rc = optee->ops->do_call_with_arg(ctx, msg_arg);
	if (rc)
		pr_err("Unregister SHM id 0x%llx rc %d\n",
			global_handle, rc);

	rc = mem_ops->memory_reclaim(global_handle, 0);
	if (rc)
		pr_err("mem_reclain: 0x%llx %d",
			global_handle, rc);

	kfree(msg_arg);

	return rc;
}

static int optee_ffa_shm_unregister_supp(struct tee_context *ctx,
					 struct tee_shm *shm)
{
	const struct ffa_mem_ops *mem_ops;
	u64 global_handle = shm->sec_world_id;
	int rc;

	/*
	 * We're skipping the OPTEE_FFA_YIELDING_CALL_UNREGISTER_SHM call
	 * since this is OP-TEE freeing via RPC so it has already retired
	 * this ID.
	 */

	optee_shm_rem_ffa_handle(global_handle);
	mem_ops = mitee_ffa_ctx.ops->mem_ops;
	rc = mem_ops->memory_reclaim(global_handle, 0);
	if (rc)
		pr_err("mem_reclain: 0x%llx %d",
			global_handle, rc);

	shm->sec_world_id = 0;

	return rc;
}

/*
 * 5. FF-A Driver initialization
 *
 * Functions to register ffa device (mitee) into system ffa bus.
 */

int mitee_dynamic_mem_free_ffa(uint64_t mem_handle)
{
	int rc;
	struct sg_table *sgt;
	struct mem_desc *desc = NULL;
	uint32_t mem_size = 0;
	const struct ffa_mem_ops *mem_ops = mitee_ffa_ctx.ops->mem_ops;

	desc = mitee_dynamic_mem_find_node(mem_handle);
	if (!desc) {
		pr_err("mitee: free memory with handle(0x%llx) failed\n", mem_handle);
		return -EINVAL;
	}

	sgt = desc->sgt;
	mem_size = desc->mem_size;
	rc = mem_ops->memory_reclaim(mem_handle, 0);
	mitee_free_memory_sgt(mem_size, sgt);
	mitee_dynamic_mem_remove_node(mem_handle);
	return 0;
}

int mitee_dynamic_mem_allocate_ffa(uint32_t mem_size, uint64_t *mem_handle, void *buf,
				uint32_t size_in, uint32_t *size_out)
{
	uint32_t size_aligned = (roundup(mem_size, PAGE_SIZE));
	struct sg_table *sgt = NULL;
	int rc;
	struct ffa_device *ffa_dev = mitee_ffa_ctx.dev;
	const struct ffa_mem_ops *mem_ops = mitee_ffa_ctx.ops->mem_ops;
	struct ffa_mem_region_attributes mem_attr = {
		.receiver = ffa_dev->vm_id,
		.attrs = FFA_MEM_RW,
	};
	struct ffa_mem_ops_args args = {
		.use_txbuf = true,
		.attrs = &mem_attr,
		.nattrs = 1,
	};

	rc = mitee_alloc_memory_sgt(size_aligned, &sgt);
	if (rc) {
		pr_err("mitee: failed to allocate 0x%xB dynamic memory with %d\n", size_aligned, rc);
		return rc;
	}

	args.sg = sgt->sgl;
	rc = mem_ops->memory_lend(&args);
	if (rc) {
		pr_err("mitee: failed to lend memory with %d\n", rc);
		goto err_out2;
	}

	rc = mitee_dynamic_mem_add_node(args.g_handle, sgt, size_aligned);
	if (rc) {
		pr_err("mitee: failed to add dynamic mem node with %d\n", rc);
		goto err_out1;
	}

	*mem_handle = args.g_handle;
	return 0;
err_out1:
	mem_ops->memory_reclaim(args.g_handle, 0);
err_out2:
	mitee_free_memory_sgt(size_aligned, sgt);
	return rc;
}

uint32_t mitee_rpc_callback(struct optee_msg_param_value *value, void *buf,
				   uint32_t size_in, uint32_t *size_out)
{

	uint32_t module_id, sub_cmd, mem_size;
	uint64_t mem_handle;
	uint32_t ret = 0;

	if (!value) {
		pr_err("mitee rpc call: invalid value\n");
		return TEEC_ERROR_BAD_PARAMETERS;
	}

	module_id = (uint32_t)value->b;
	sub_cmd = value->a;
	switch (sub_cmd) {
	case OPTEE_REE_CALLBACK_ALLOCATE_NONSECMEM:
		if (module_id != REE_CALLBACK_MODULE_TEE_FRAMEWORK) {
			ret = TEEC_ERROR_BAD_PARAMETERS;
			break;
		}

		mem_size = (uint32_t)value->c;
		if (mitee_dynamic_mem_allocate_ffa(mem_size, &mem_handle,
						   buf, size_in, size_out)) {
			ret = TEEC_ERROR_BAD_PARAMETERS;
			break;
		}

		value->a = mem_handle;
		value->b = mem_size;
		break;
	case OPTEE_REE_CALLBACK_FREE_NONSECMEM:
		if (module_id != REE_CALLBACK_MODULE_TEE_FRAMEWORK) {
			ret = TEEC_ERROR_BAD_PARAMETERS;
			break;
		}

		mem_handle = value->c;
		mitee_dynamic_mem_free_ffa(mem_handle);
		break;
	//only for test OPTEE_REE_CALLBACK_CALL
	default:
		pr_err("mitee rpc call: unknown cmd 0x%x\n", sub_cmd);
		ret = TEEC_ERROR_BAD_PARAMETERS;
		break;
	}
	return ret;
}

/*
 * 5. FF-A Driver initialization
 *
 * Functions to register ffa device (mitee) into system ffa bus.
 */
static void mitee_ffa_remove(struct ffa_device *ffa_dev)
{
	mutex_destroy(&mitee_ffa_ctx.mutex);
	rhashtable_free_and_destroy(&mitee_ffa_ctx.global_ids, rh_free_fn,
				    NULL);
}

static int mitee_ffa_probe(struct ffa_device *ffa_dev)
{
	const struct ffa_ops *ffa_ops = NULL;
	int rc = 0;

	ffa_ops = ffa_dev->ops;
	if (IS_ERR_OR_NULL(ffa_ops)) {
		rc = PTR_ERR(ffa_ops);
		pr_err("failed to obtain FFA ops: %d", rc);
		return rc;
	}

	mitee_ffa_ctx.dev = ffa_dev;
	mitee_ffa_ctx.ops = ffa_ops;

	mutex_init(&mitee_ffa_ctx.mutex);
	/*mitee smc notify init*/
	mitee_smc_notify_init(ffa_dev);
	rc = rhashtable_init(&mitee_ffa_ctx.global_ids, &shm_rhash_params);
	if (rc) {
		pr_err("rhashtable_init error %d\n", rc);
		goto err_ffa_mutex_destory;
	}
	return 0;

err_ffa_mutex_destory:
	mutex_destroy(&mitee_ffa_ctx.mutex);
	return rc;
}

static struct ffa_driver optee_ffa_driver = {
	.name = "mitee",
	.probe = mitee_ffa_probe,
	.remove = mitee_ffa_remove,
	.id_table = &mitee_ffa_ctx.device_id,
};

int mitee_ffa_register(void)
{
	pr_info("%s\n", __func__);
	if (IS_REACHABLE(CONFIG_XRING_ARM_FFA_TRANSPORT))
		return ffa_register(&optee_ffa_driver);
	else
		return -EOPNOTSUPP;
}

void mitee_ffa_unregister(void)
{
	pr_info("%s\n", __func__);
	if (IS_REACHABLE(CONFIG_XRING_ARM_FFA_TRANSPORT))
		ffa_unregister(&optee_ffa_driver);
}

/* Wrapper around FFA_MSG_SEND_DIRECT_REQ */
int mitee_ffa_call(union common_args *args)
{
	int rc;
	struct ffa_send_direct_data data = {
		.data0 = args->in.cmd,
		.data1 = args->in.param[0],
		.data2 = args->in.param[1],
		.data3 = args->in.param[2],
		.data4 = args->in.param[3],
	};

	rc = mitee_ffa_ctx.ops->msg_ops->sync_send_receive(mitee_ffa_ctx.dev,
							   &data);
	if (rc) {
		pr_err("error %d\n", rc);
		return rc;
	}

	/* Copy out */
	args->out.resp = data.data0;
	args->out.param[0] = data.data1;
	args->out.param[1] = data.data2;
	args->out.param[2] = data.data3;
	args->out.param[3] = data.data4;

	return 0;
}

const struct mitee_comm_ops mitee_ffa_comm_ops = {
	.id = ARM_FFA,
	.init = mitee_ffa_register,
	.deinit = mitee_ffa_unregister,
	.call = mitee_ffa_call,
	.from_msg_param = from_msg_param_ffa_mem,
	.to_msg_param = to_msg_param_ffa_mem,
	.shm_register = optee_ffa_shm_register,
	.shm_unregister = optee_ffa_shm_unregister,
	.shm_unregister_supp = optee_ffa_shm_unregister_supp,
};
