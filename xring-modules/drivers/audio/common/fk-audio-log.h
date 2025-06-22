/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _FK_AUD_LOG_
#define _FK_AUD_LOG_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/types.h>
#include <linux/slab.h>

enum aud_log_module {
	AUD_SOC = 0,
	AUD_CORE,
	AUD_COMM,
	AUD_DSP,
	AUD_MAX,
};

void aud_log_info_query(void);
int aud_log_check(int id);
void aud_log_enable(int id, int val);
int aud_log_lev_up_get(int id);
void aud_log_lev_up_set(int id, int lev);
char *aud_log_mod_name(int id);

void aud_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
		   ...);

#define AUD_LOG_ERR(id, fmt, args...) \
	pr_err("[E][aud][%s]:%s:" fmt "\n", \
	aud_log_mod_name(id), __func__, ##args)

#define AUD_LOG_INFO(id, fmt, args...) do { \
	int lev = aud_log_lev_up_get(id); \
	if (lev == 0) \
		pr_info("[I][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else \
		pr_err("[I][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
} while (0)

#define AUD_LOG_DBG(id, fmt, args...) do { \
	int lev = aud_log_lev_up_get(id); \
	if (lev == 0) \
		pr_debug("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else if (lev == 1) \
		pr_info("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else \
		pr_err("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
} while (0)

#define AUD_LOG_ERR_LIM(id, fmt, args...) \
	pr_err_ratelimited("[E][aud][%s]:%s:" fmt "\n", \
	aud_log_mod_name(id), __func__, ##args)

#define AUD_LOG_INFO_LIM(id, fmt, args...) do { \
	int lev = aud_log_lev_up_get(id); \
	if (lev == 0) \
		pr_info_ratelimited("[I][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else \
		pr_err_ratelimited("[I][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
} while (0)

#define AUD_LOG_DBG_LIM(id, fmt, args...) do { \
	int lev = aud_log_lev_up_get(id); \
	if (lev == 0) \
		pr_debug_ratelimited("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else if (lev == 1) \
		pr_info_ratelimited("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
	else \
		pr_err_ratelimited("[D][aud][%s]:%s:" fmt "\n", \
		aud_log_mod_name(id), __func__, ##args); \
} while (0)

#define AUD_TRACE(fmt, args...) \
	aud_dbg_trace(trace_audio_event, "[%s]:" fmt, \
	__func__, ##args)

#endif //_FK_AUD_LOG_
