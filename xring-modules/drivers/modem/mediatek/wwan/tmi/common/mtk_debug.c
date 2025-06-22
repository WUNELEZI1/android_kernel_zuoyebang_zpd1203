// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/export.h>
#include <linux/kernel.h>
#include "mtk_debug.h"

#define TAG "REG_DUMP"

#define MTK_REGSCNT_PER_LINE 4
#define MTK_BYTES_PER_LINE 16
#define MTK_LOG_BUFF_SIZE 256

#define __mtk_log(level)                                                        \
void __mtk_##level(struct device *dev, const char *fmt, ...) \
{                                                                                                \
	struct va_format vaf = {                                                  \
		.fmt = fmt,                                                               \
	};                                                                                      \
	va_list args;                                                                    \
	va_start(args, fmt);                                                      \
	vaf.va = &args;                                                             \
	dev_##level(dev, "%pV", &vaf);                                   \
	trace_mtk_##level(dev, &vaf);                                    \
	va_end(args);                                                               \
}

static unsigned int mtk_log_mask = MTK_DBG_NONE;

__mtk_log(info)
__mtk_log(warn)
__mtk_log(err)

void __mtk_dbg(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
	       enum mtk_memlog_region_id rg_id, const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	va_start(args, fmt);
	vaf.va = &args;

	if (mtk_log_mask & dbg_mask)
		dev_dbg(mdev->dev, "%pV", &vaf);

	mtk_memlog_write(mdev, rg_id, "%pV", &vaf);
	trace_mtk_debug(mdev->dev, &vaf);

	va_end(args);
}

void __mtk_hex_dump(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
		    enum mtk_memlog_region_id rg_id, const char *msg,
		    const void *mem, size_t len)
{
	char buf[MTK_LOG_BUFF_SIZE];
	size_t buf_len;
	const void *ptr;

	if (msg)
		__mtk_dbg(mdev, dbg_mask, rg_id, "%s\n", msg);

	for (ptr = mem; (ptr - mem) < len; ptr += MTK_BYTES_PER_LINE) {
		memset(buf, 0, MTK_LOG_BUFF_SIZE);
		buf_len = 0;
		buf_len = scnprintf(buf + buf_len, sizeof(buf) - buf_len,
					"%08x:", (unsigned int)(ptr - mem));
		hex_dump_to_buffer(ptr, len - (ptr - mem), MTK_BYTES_PER_LINE, 1,
				   buf + buf_len,
				   sizeof(buf) - buf_len, false);
		__mtk_dbg(mdev, dbg_mask, rg_id, "%s\n", buf);
	}

	trace_mtk_debug_dump(mdev->dev, msg, buf, len);
}

#define MTK_REG_OFFSET_STR_LEN 7
#define MTK_REG_VAL_STR_LEN 9
void __mtk_regs_dump(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
		     enum mtk_memlog_region_id rg_id, const char *msg,
		     unsigned long long addr, size_t len)
{
	int i = 0;
	unsigned long long base_addr = 0;
	/* Up multiples of MTK_REGSCNT_PER_LINE */
	int round_up_len = round_up(len, MTK_REGSCNT_PER_LINE);
	int b4_fix_num = round_up_len / MTK_REGSCNT_PER_LINE;
	int b16_fix_num = b4_fix_num / MTK_REGSCNT_PER_LINE;
	int b16_tail_num = b4_fix_num % MTK_REGSCNT_PER_LINE;
	unsigned char buf[MTK_LOG_BUFF_SIZE] = {0};

	if (len <= 0 || !mdev) {
		pr_err("Invalid parameters!\n");
		return;
	}

	if (msg)
		__mtk_dbg(mdev, dbg_mask, rg_id, "%s\n", msg);

	__mtk_dbg(mdev, dbg_mask, rg_id, "===start address:0x%lx, len(32bit):0x%x===\n", addr, len);
	for (i = 0; i < b16_fix_num; i++) {
		base_addr = addr + i * MTK_BYTES_PER_LINE;
		__mtk_dbg(mdev, dbg_mask, rg_id, "0x%04x: %08x %08x %08x %08x\n",
			  i * MTK_BYTES_PER_LINE,
			  mtk_hw_read32(mdev, base_addr + 0),
			  mtk_hw_read32(mdev, base_addr + 4),
			  mtk_hw_read32(mdev, base_addr + 8),
			  mtk_hw_read32(mdev, base_addr + 12));
	}

	if (b16_tail_num) {
		base_addr = addr + i * MTK_BYTES_PER_LINE;
		sprintf(buf, "0x%04x:", i * MTK_BYTES_PER_LINE);
		for (i = 0; i < b16_tail_num; i++)
			sprintf((buf + MTK_REG_OFFSET_STR_LEN) + i * MTK_REG_VAL_STR_LEN,
				" %08x", mtk_hw_read32(mdev, base_addr + i * 4));

		__mtk_dbg(mdev, dbg_mask, rg_id, "%s\n", buf);
	}
}

void __mtk_dbg_ratelimited(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
			   enum mtk_memlog_region_id rg_id, const char *fmt, ...)
{
	static DEFINE_RATELIMIT_STATE(_rs,
		DEFAULT_RATELIMIT_INTERVAL,
		DEFAULT_RATELIMIT_BURST);
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	if (!__ratelimit(&_rs))
		return;

	va_start(args, fmt);
	vaf.va = &args;

	if (mtk_log_mask & dbg_mask)
		dev_dbg(mdev->dev, "%pV", &vaf);

	mtk_memlog_write(mdev, rg_id, "%pV", &vaf);
	trace_mtk_debug(mdev->dev, &vaf);

	va_end(args);
}

void __mtk_info_ratelimited(struct device *dev, const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	va_start(args, fmt);
	vaf.va = &args;

	dev_info_ratelimited(dev, "%pV", &vaf);
	trace_mtk_debug(dev, &vaf);

	va_end(args);
}

void __mtk_warn_ratelimited(struct device *dev, const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	va_start(args, fmt);
	vaf.va = &args;

	dev_warn_ratelimited(dev, "%pV", &vaf);
	trace_mtk_warn(dev, &vaf);

	va_end(args);
}

void __mtk_err_ratelimited(struct device *dev, const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	va_start(args, fmt);
	vaf.va = &args;

	dev_err_ratelimited(dev, "%pV", &vaf);
	trace_mtk_err(dev, &vaf);

	va_end(args);
}

module_param(mtk_log_mask, uint, 0644);
MODULE_PARM_DESC(mtk_log_mask, "The value is used to control mtk log mask.");

