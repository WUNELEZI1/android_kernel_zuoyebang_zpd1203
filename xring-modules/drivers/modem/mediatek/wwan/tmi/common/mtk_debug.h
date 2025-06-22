/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#ifndef __MTK_DEBUG_H
#define __MTK_DEBUG_H

#include <linux/bitfield.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/printk.h>
#include "mtk_dev.h"
#include "mtk_trace.h"
#include "mtk_memlog.h"

#define MTK_DBG_ASSERT

enum mtk_debug_mask {
	MTK_DBG_NONE = 0,
	/* Module mask :bit0~bit14 */
	MTK_DBG_WWAN = BIT(0),
	MTK_DBG_DPMF = BIT(1),
	MTK_DBG_PCIE = BIT(2),
	MTK_DBG_MHCCIF = BIT(3),
	MTK_DBG_RGU = BIT(4),
	MTK_DBG_CLDMA = BIT(5),
	MTK_DBG_PORT = BIT(6),
	MTK_DBG_FSM = BIT(7),
	/* Flow mask :bit15~bit31 */
	MTK_DBG_DATA_IRQ = BIT(15),
	MTK_DBG_INTR = BIT(18),
	MTK_DBG_INIT = BIT(19),
	MTK_DBG_PM = BIT(20),
	MTK_DBG_RESET = BIT(21),
	MTK_DBG_CTRL_TX = BIT(22),
	MTK_DBG_CTRL_RX = BIT(23),
	MTK_DBG_DEVLINK = BIT(24),
	MTK_DBG_RDIT = BIT(25),
	MTK_DBG_ANY = 0xffffffff,
};

void __mtk_dbg(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
	       enum mtk_memlog_region_id rg_id, const char *fmt, ...);
void __mtk_info(struct device *dev, const char *fmt, ...);
void __mtk_warn(struct device *dev, const char *fmt, ...);
void __mtk_err(struct device *dev, const char *fmt, ...);
void __mtk_hex_dump(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
		    enum mtk_memlog_region_id rg_id,
		    const char *msg, const void *buf, size_t len);
void __mtk_regs_dump(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
		     enum mtk_memlog_region_id rg_id,
		     const char *msg, unsigned long long addr, size_t len);
void __mtk_dbg_ratelimited(struct mtk_md_dev *mdev, enum mtk_debug_mask dbg_mask,
			   enum mtk_memlog_region_id rg_id, const char *fmt, ...);
void __mtk_info_ratelimited(struct device *dev, const char *fmt, ...);
void __mtk_warn_ratelimited(struct device *dev, const char *fmt, ...);
void __mtk_err_ratelimited(struct device *dev, const char *fmt, ...);

#define MTK_DBG(mdev, dbg_mask, rg_id, fmt, args...) \
		__mtk_dbg(mdev, dbg_mask, rg_id, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_INFO(mdev, fmt, args...) \
	__mtk_info((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_WARN(mdev, fmt, args...)\
	__mtk_warn((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_ERR(mdev, fmt, args...) \
	__mtk_err((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_HEX_DUMP(mdev, dbg_mask, rg_id, msg, mem, len) \
	__mtk_hex_dump(mdev, dbg_mask, rg_id, msg, mem, len)

#define MTK_REGS_DUMP(mdev, dbg_mask, rg_id, msg, addr, len) \
	__mtk_regs_dump(mdev, dbg_mask, rg_id, msg, addr, len)

#define MTK_BUG_ON() \
do {\
	char *str = NULL; \
	*str = 1; \
} while (0)

#ifdef MTK_DBG_ASSERT
#define MTK_BUG(mdev) \
do { \
	struct mtk_md_dev *_mdev = mdev; \
	dev_err((_mdev)->dev,\
		"mtk assert, mdev data address: %llx, file: %s, function: %s, line %d\n",\
		(u64)(_mdev), __FILE__, __func__, __LINE__); \
	MTK_BUG_ON(); \
} while (0)
#else
#define MTK_BUG(mdev) \
	dev_err((mdev)->dev,\
		"mtk assert, mdev data address: %llx, file: %s, function: %s, line %d\n", \
		(u64)(_mdev), __FILE__, __func__, __LINE__)
#endif

#define MTK_DBG_RATELIMITED(mdev, dbg_mask, rg_id, fmt, args...) \
	__mtk_dbg_ratelimited(mdev, dbg_mask, rg_id, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_INFO_RATELIMITED(mdev, fmt, args...) \
	__mtk_info_ratelimited((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_WARN_RATELIMITED(mdev, fmt, args...) \
	__mtk_warn_ratelimited((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#define MTK_ERR_RATELIMITED(mdev, fmt, args...) \
	__mtk_err_ratelimited((mdev)->dev, "[%s][%s][%d]:" fmt, \
		TAG, __func__, __LINE__, ##args)

#endif /* __MTK_DEBUG_H */

