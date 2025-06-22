// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 * Description: atf shmem driver
 * Modify time: 2023-03-13
 */
#include <soc/xring/atf_shmem.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/arm-smccc.h>
#include <linux/atomic.h>
#include <linux/preempt.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/debugfs.h>

#define SHMEM_UNINIT    0x0
#define SHMEM_INITED    0xa5a5a5a5

static int atf_shmem_init(void);

static struct shm_info_t g_shm_info = {
	.inited = SHMEM_UNINIT,
	.buff_status = {0},
	.buff_lock = __SPIN_LOCK_UNLOCKED(g_shm_info.buff_lock),
	.init_lock = __SPIN_LOCK_UNLOCKED(g_shm_info.init_lock),
};

static s32 get_shm_info(struct shm_info_t *info)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_SHMEM_INFO, 0, 0, 0,
		      0, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("get shmem smc info failed\n");
		return -EINVAL;
	}
	info->paddr = (phys_addr_t)res.a1;
	info->size = (u64)res.a2;
	info->buff_nums = (u64)res.a3;

	return 0;
}

static s32 get_idle_buffer(void)
{
	unsigned long flags;
	u32 i;

	if (g_shm_info.inited != SHMEM_INITED)
		if (atf_shmem_init() != 0)
			return -EINVAL;

	spin_lock_irqsave(&g_shm_info.buff_lock, flags);

	/*
	 * Now it will only be searched once,
	 * and it can be expanded to multiple searches later
	 */
	for (i = 0; i < g_shm_info.buff_nums; i++) {
		if (g_shm_info.buff_status[i] == STATUS_IDLE) {
			g_shm_info.buff_status[i] = STATUS_BUSY;
			break;
		}
	}

	spin_unlock_irqrestore(&g_shm_info.buff_lock, flags);
	if (i >= g_shm_info.buff_nums) {
		pr_err("atf_shmem find idle buff fail. all buff busy\n");
		return -EFAULT;
	}

	return (s32)i;
}

static void free_idle_buffer(u64 index)
{
	if (index >= (int)g_shm_info.buff_nums)
		return;

	/* for free one buffer, no need lock here */
	g_shm_info.buff_status[index] = STATUS_IDLE;
}

static s32 valid_shm_type(enum shmem_type type)
{
	if (type != TYPE_IN && type != TYPE_OUT && type != TYPE_INOUT)
		return -EINVAL;
	return 0;
}

static s32 shm_id2addr(u32 index, u64 *buff_addr)
{
	*buff_addr = (u64)((uintptr_t)g_shm_info.vaddr + g_shm_info.per_size * (u64)index);

	if (*buff_addr < (u64)(uintptr_t)g_shm_info.vaddr ||
	    g_shm_info.per_size * (u64)index < (u64)index) {
		pr_err("atf_shmem buff_addr overflow\n");
		return -EPERM;
	}

	return 0;
}

/* atf shmem main proc */
s32 smc_shm_mode(u64 fid, char *buf, size_t buf_size, enum shmem_type shmem_type, u64 x4)
{
	struct arm_smccc_res res = {0};
	u64 shm_buff_addr;
	s32 index;
	s32 ret;

	/* 1. param check */
	if (buf == NULL || buf_size == 0 || buf_size > g_shm_info.per_size) {
		pr_err("atf_shmem prams check failed\n");
		return -EINVAL;
	}
	if (valid_shm_type(shmem_type)) {
		pr_err("atf_shmem invalid type\n");
		return -EINVAL;
	}

	/* 2. must can't be in interrupt handling */
	if (unlikely(in_interrupt())) {
		pr_err("do not support share memory in irq\n");
		return -EPERM;
	}

	/* 3. check inited status */
	if (g_shm_info.inited != SHMEM_INITED)
		if (atf_shmem_init() != 0)
			return -EPERM;

	/* 4. get idel ns_shmem */
	index = get_idle_buffer();
	if (index < 0 || index >= (s32)g_shm_info.buff_nums) {
		pr_err("atf_shmem get idle buffer error: id=%d\n", index);
		/* error index no need free */
		return -EPERM;
	}

	/* 5. get idle buffer addr */
	ret = shm_id2addr(index, &shm_buff_addr);
	if (ret != 0)
		goto free_buffer;

	/* 6. have IN/INOUT, will cpy into */
	if (shmem_type & TYPE_IN)
		memcpy((void *)(uintptr_t)shm_buff_addr, buf, buf_size);

	/* 7. smc call into bl31 */
	arm_smccc_smc(fid, index, buf_size, shmem_type,
		      x4, 0, 0, 0, &res);
	if (res.a0 != 0) {
		pr_err("atf_shmem smc failed\n");
		ret = -EPERM;
		goto clean_buffer;
	}

	/* 8. have OUT/INOUT, will cpy out */
	if (shmem_type & TYPE_OUT)
		memcpy(buf, (void *)(uintptr_t)shm_buff_addr, buf_size);

	/* 9. go out */
	ret = 0;
clean_buffer:
	/* 10. clean buffer data */
	memset((void *)(uintptr_t)shm_buff_addr, 0, g_shm_info.per_size);
free_buffer:
	/* 11. free buffer */
	free_idle_buffer(index);
	return ret;
}
EXPORT_SYMBOL(smc_shm_mode);

s32 get_atf_shm_size(u64 *size)
{
	if (!size) {
		pr_err("atf shmem info error\n");
		return -EINVAL;
	}

	if (g_shm_info.inited != SHMEM_INITED)
		if (atf_shmem_init() != 0)
			return -EINVAL;

	*size = g_shm_info.per_size;

	return 0;
}
EXPORT_SYMBOL(get_atf_shm_size);

static int atf_shmem_init(void)
{
	s32 ret;

	pr_info("atf shmem driver init\n");

	if (g_shm_info.inited == SHMEM_INITED) {
		pr_info("atf shmem driver has been inited\n");
		return 0;
	}

	ret = get_shm_info(&g_shm_info);
	if (ret || g_shm_info.paddr == 0 ||
	    g_shm_info.size == 0 || g_shm_info.buff_nums == 0 ||
	    g_shm_info.buff_nums > ATF_SHM_MAX_BUF_CNT) {
		pr_err("atf shmem info error\n");
		ret = EINVAL;
		goto out;
	}

	g_shm_info.per_size = g_shm_info.size / g_shm_info.buff_nums;
	if (!g_shm_info.vaddr)
		g_shm_info.vaddr = ioremap_cache(g_shm_info.paddr, g_shm_info.size);
	if (!g_shm_info.vaddr) {
		pr_err("Failed to remap atf shmem addr\n");
		ret = EINVAL;
		goto out;
	}

	g_shm_info.inited = SHMEM_INITED;
	ret = 0;
out:

	return ret;
}

static int __init __atf_shmem_init(void)
{
	return atf_shmem_init();
}

static void __exit __atf_shmem_exit(void)
{
	pr_info("atf shmem driver exit\n");
	if (g_shm_info.vaddr)
		iounmap((void __iomem *)g_shm_info.vaddr);
}

MODULE_DESCRIPTION("XRing atf shmem driver");
MODULE_LICENSE("GPL v2");
module_init(__atf_shmem_init);
module_exit(__atf_shmem_exit);
