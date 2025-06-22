/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XR_CRYPTO_COMMON__
#define __XR_CRYPTO_COMMON__

#include <linux/types.h>
#include <linux/printk.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)  (sizeof(x) / sizeof((x)[0]))
#endif

#define crypto_err(fmt, args...)           pr_err("[xrcrypto]" fmt, ##args)
#define crypto_info(fmt, args...)          pr_info("[xrcrypto]" fmt, ##args)
#define crypto_debug(fmt, args...)         pr_debug("[xrcrypto]" fmt, ##args)

#endif
