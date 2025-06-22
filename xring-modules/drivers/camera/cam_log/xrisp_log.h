/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_ISP_LOG_H_
#define _XR_ISP_LOG_H_

#include <linux/device.h>

enum xrisp_log_level_e {
	XRISP_LOG_ERROR = 0,
	XRISP_LOG_WARN,
	XRISP_LOG_INFO,
	XRISP_LOG_DEBUG,
};

#define XRISP_PR_ERROR(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_ERROR ? pr_err(fmt, ##__VA_ARGS__) : 0)
#define XRISP_PR_WARN(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_WARN ? pr_warn(fmt, ##__VA_ARGS__) : 0)
#define XRISP_PR_INFO(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_INFO ? pr_info(fmt, ##__VA_ARGS__) : 0)
#define XRISP_PR_DEBUG(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_DEBUG ? pr_debug(fmt, ##__VA_ARGS__) : 0)

#define XRISP_DEV_ERROR(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_ERROR ? dev_err(fmt, ##__VA_ARGS__) : 0)
#define XRISP_DEV_WARN(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_WARN ? dev_warn(fmt, ##__VA_ARGS__) : 0)
#define XRISP_DEV_INFO(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_INFO ? dev_info(fmt, ##__VA_ARGS__) : 0)
#define XRISP_DEV_DEBUG(fmt, ...) \
	(cam_log_get_level() >= XRISP_LOG_DEBUG ? dev_dbg(fmt, ##__VA_ARGS__) : 0)

struct xrisp_log_data {
	int32_t log_level;
	struct device *dev;
};

int cam_log_init(struct device *dev);
int cam_log_set_level(int level);
int cam_log_get_level(void);
void cam_log_exit(void);

#endif /* _XR_ISP_LOG_H_ */
