/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __MITEE_CRYPTO_INTERNEL__
#define __MITEE_CRYPTO_INTERNEL__

#include <linux/types.h>
#include <linux/virtio.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/irqreturn.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/virtio.h>
#include <linux/regulator/consumer.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/tee_drv.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <linux/version.h>
#if (LINUX_VERSION_MAJOR > 6 || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL > 6) || \
	(LINUX_VERSION_MAJOR == 6 && LINUX_VERSION_PATCHLEVEL == 6 && LINUX_VERSION_SUBLEVEL > 0))
#include <linux/iosys-map.h>
#endif
#define CRYPTO_LOG_LEVEL_ASSERT		0
#define CRYPTO_LOG_LEVEL_ERROR		1
#define CRYPTO_LOG_LEVEL_WARN		2
#define CRYPTO_LOG_LEVEL_INFO		3
#define CRYPTO_LOG_LEVEL_DEBUG		4
#define CRYPTO_LOG_LEVEL_MAX		5
#define CRYPTO_LOG_LEVEL_DEF		CRYPTO_LOG_LEVEL_INFO

extern int crypto_log_level;

#if IS_ENABLED(CONFIG_XRING_DEBUG)
#define cryptoerr(fmt, ...)                                                 \
do {                                                                     \
	if (crypto_log_level >= (int)CRYPTO_LOG_LEVEL_ERROR)                          \
		pr_err("CRYPTO-DEV [%s][E]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define cryptowarn(fmt, ...)                                                \
do {                                                                     \
	if (crypto_log_level >= (int)CRYPTO_LOG_LEVEL_WARN)                           \
		pr_warn("CRYPTO-DEV [%s][W]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define cryptoinfo(fmt, ...)                                                \
do {                                                                     \
	if (crypto_log_level >= (int)CRYPTO_LOG_LEVEL_INFO)                           \
		pr_info("CRYPTO-DEV [%s][I]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)

#define cryptodbg(fmt, ...)                                                \
do {                                                                     \
	if (crypto_log_level >= (int)CRYPTO_LOG_LEVEL_DEBUG)                           \
		pr_debug("CRYPTO-DEV [%s][D]: " fmt, __func__, ##__VA_ARGS__); \
} while (0)
#else
#define cryptoerr(fmt, ...) \
		pr_err("CRYPTO-DEV [%s][E]: " fmt, __func__, ##__VA_ARGS__);

#define cryptowarn(fmt, ...) \
		pr_warn("CRYPTO-DEV [%s][W]: " fmt, __func__, ##__VA_ARGS__);

#define cryptoinfo(fmt, ...) \
		pr_info("CRYPTO-DEV [%s][I]: " fmt, __func__, ##__VA_ARGS__);

#define cryptodbg(fmt, ...) \
		pr_debug("CRYPTO-DEV [%s][D]: " fmt, __func__, ##__VA_ARGS__);
#endif

#endif
