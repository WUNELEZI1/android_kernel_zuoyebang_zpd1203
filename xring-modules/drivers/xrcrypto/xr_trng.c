// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/hw_random.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <soc/xring/securelib/securec.h>
#include "xr_crypto_common.h"
#include "xsp_ffa.h"

#define DEVICE_NAME "xr_trng"
#define FFA_GET_TRNG_MAX_NUM 32
#define FFA_TRNG_FID 0xc3002900

static int crypto_hwrng_init(struct hwrng *rng)
{
	return 0;
}

static void crypto_hwrng_cleanup(struct hwrng *rng) {}

static int crypto_ffa_direct_message(struct xsp_ffa_msg *msg)
{
#if IS_ENABLED(CONFIG_XRING_RSP)
	int ret;

	msg->fid = FFA_TRNG_FID;
	ret = xrsp_ffa_direct_message(msg);
	if (ret != 0) {
		crypto_err("ffa message not arrive in respondent, ret = %d\n", ret);
		return ret;
	}
	if (msg->ret != 0) {
		pr_err("%s: error: ret = %#lx\n", __func__, msg->ret);
		return -1;
	}
	/* trng failed to obtain */
	if ((msg->data0 | msg->data1 | msg->data2 | msg->data3) == 0) {
		crypto_err("%s: trng get failed\n", __func__);
		return -1;
	}
#endif
	return 0;
}

static int crypto_hwrng_read(struct hwrng *rng, void *data, size_t max, bool wait)
{
	int ret;
	u8 *buf = data;
	struct xsp_ffa_msg msg = { 0 };
	uint8_t trng_pool[FFA_GET_TRNG_MAX_NUM];
	u64 *pool_ptr64 = (u64 *)trng_pool;

	if (data == NULL || max == 0) {
		crypto_err("data is NUll or max is 0\n");
		return 0;
	}
	if (max > FFA_GET_TRNG_MAX_NUM)
		max = FFA_GET_TRNG_MAX_NUM;
	ret = crypto_ffa_direct_message(&msg);
	if (ret) {
		crypto_err("crypto_ffa_direct_message failed, ret = %d\n", ret);
		return 0;
	}
	pool_ptr64[0] = msg.data0;
	pool_ptr64[1] = msg.data1;
	pool_ptr64[2] = msg.data2;
	pool_ptr64[3] = msg.data3;
	ret = memcpy_s(buf, max, trng_pool, max);
	if (ret != EOK) {
		crypto_err("memcpy_s failed, ret = %d\n", ret);
		return 0;
	}
	return max;
}

static struct hwrng crypto_trng_dev = {
	.name              = DEVICE_NAME,
	.init              = crypto_hwrng_init,
	.cleanup           = crypto_hwrng_cleanup,
	.data_present      = NULL,
	.data_read         = NULL,
	.read              = crypto_hwrng_read,
	.priv              = 0,
	.quality           = 1024,
};

static int crypto_trng_init(void)
{
	int ret;

	ret = hwrng_register(&crypto_trng_dev);
	if (ret) {
		crypto_err("failed to register hwrng.\n");
		return ret;
	}
	return 0;
}

static void crypto_trng_exit(void)
{
	hwrng_unregister(&crypto_trng_dev);
}

module_init(crypto_trng_init);
module_exit(crypto_trng_exit);
MODULE_LICENSE("Dual BSD/GPL");
