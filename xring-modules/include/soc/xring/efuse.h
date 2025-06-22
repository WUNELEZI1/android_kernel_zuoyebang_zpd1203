/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 * Description: efuse driver
 * Modify time: 2023-03-13
 */

#pragma once

#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/version.h>

ssize_t bl31_efuse_read(u32 filed_id, u32 data_len, u8 *data);
ssize_t bl31_efuse_write(u32 data_len, u8 *data);
