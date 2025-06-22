/* SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Copyright (c) 2022, MediaTek Inc.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM mtk_trace

#if !defined(__MTK_TRACE_EVENTS_H) || defined(TRACE_HEADER_MULTI_READ)
#define __MTK_TRACE_EVENTS_H

#include <linux/device.h>
#include <linux/printk.h>
#include <linux/tracepoint.h>

#define MTK_MSG_MAX 110

DECLARE_EVENT_CLASS(mtk_log_event,
		    TP_PROTO(struct device *dev, struct va_format *vaf),
	TP_ARGS(dev, vaf),
	TP_STRUCT__entry(__string(device, dev_name(dev))
	__dynamic_array(char, msg, MTK_MSG_MAX)
	),
	TP_fast_assign(__assign_str(device, dev_name(dev));
		WARN_ON_ONCE(vsnprintf(__get_dynamic_array(msg),
				       MTK_MSG_MAX,
					vaf->fmt,
					*vaf->va) >= MTK_MSG_MAX);
	),

	TP_printk("%s %s", __get_str(device), __get_str(msg))
);

DEFINE_EVENT(mtk_log_event, mtk_debug,
	     TP_PROTO(struct device *dev, struct va_format *vaf),
		  TP_ARGS(dev, vaf)
);

DEFINE_EVENT(mtk_log_event, mtk_info,
	     TP_PROTO(struct device *dev, struct va_format *vaf),
		  TP_ARGS(dev, vaf)
);

DEFINE_EVENT(mtk_log_event, mtk_warn,
	     TP_PROTO(struct device *dev, struct va_format *vaf),
		  TP_ARGS(dev, vaf)
);

DEFINE_EVENT(mtk_log_event, mtk_err,
	     TP_PROTO(struct device *dev, struct va_format *vaf),
		  TP_ARGS(dev, vaf)
);

TRACE_EVENT(mtk_irq_entry,
	    TP_PROTO(int irq_id),

	TP_ARGS(irq_id),

	TP_STRUCT__entry(__field(int, irq_id)),

	TP_fast_assign(__entry->irq_id = irq_id;),

	TP_printk("%d", __entry->irq_id)
);

TRACE_EVENT(mtk_irq_exit,
	    TP_PROTO(int type, int q_id),

	TP_ARGS(type, q_id),

	TP_STRUCT__entry(__field(int, type)
			 __field(int, q_id)),

	TP_fast_assign(__entry->type = type;
		       __entry->q_id = q_id;),

	TP_printk("%d, %d", __entry->type, __entry->q_id)
);

/* data path latency tuning */
#ifdef CONFIG_DATA_CPU_LOADING_OPTIMIZE
TRACE_EVENT(mtk_tput_ds_lock,
	    TP_PROTO(void *str),

		TP_ARGS(str),

		TP_STRUCT__entry(__string(str, str)),

		TP_fast_assign(__assign_str(str, str);),

		TP_printk("%s", __get_str(str))
	);
#endif

TRACE_EVENT(mtk_tput_data_tx,
	    TP_PROTO(void *str),

		TP_ARGS(str),

		TP_STRUCT__entry(__string(str, str)),

		TP_fast_assign(__assign_str(str, str);),

		TP_printk("%s", __get_str(str))
	);

TRACE_EVENT(mtk_tras_data_tx,
	    TP_PROTO(unsigned long long base_ts, unsigned long long next_ts),

		TP_ARGS(base_ts, next_ts),

		TP_STRUCT__entry(__field(unsigned long long, base_ts)
			__field(unsigned long long, next_ts)
		),

		TP_fast_assign(__entry->base_ts = base_ts;
			__entry->next_ts = next_ts;

		),

		TP_printk("%llu, %llu", __entry->base_ts, __entry->next_ts)
	);

TRACE_EVENT(mtk_tput_data_rx,
	    TP_PROTO(int q_id, void *str),

		TP_ARGS(q_id, str),

		TP_STRUCT__entry(__field(int, q_id)
			__string(str, str)
		),

		TP_fast_assign(__entry->q_id = q_id;
			__assign_str(str, str);
		),

		TP_printk("%d, %s", __entry->q_id, __get_str(str))
	);

TRACE_EVENT(mtk_tput_data_drb_rel,
	    TP_PROTO(int q_id, int to_rel, int budget),

		TP_ARGS(q_id, to_rel, budget),

		TP_STRUCT__entry(__field(int, q_id)
			__field(int, to_rel)
			__field(int, budget)
		),

		TP_fast_assign(__entry->q_id = q_id;
			__entry->to_rel = to_rel;
			__entry->budget = budget;
		),

		TP_printk("%d, %d, %d", __entry->q_id,
			  __entry->to_rel, __entry->budget)
	);

TRACE_EVENT(mtk_tput_data_drb_fill,
	    TP_PROTO(int srv_id, int q_id, int pkt_filled, int pkt_to_fill, int available_drb),

		TP_ARGS(srv_id, q_id, pkt_filled, pkt_to_fill, available_drb),

		TP_STRUCT__entry(__field(int, srv_id)
			__field(int, q_id)
			__field(int, pkt_filled)
			__field(int, pkt_to_fill)
			__field(int, available_drb)
		),

		TP_fast_assign(__entry->srv_id = srv_id;
			__entry->q_id = q_id;
			__entry->pkt_filled = pkt_filled;
			__entry->pkt_to_fill = pkt_to_fill;
			__entry->available_drb = available_drb;
		),

		TP_printk("%d, %d, %d, %d, %d",
			  __entry->srv_id, __entry->q_id, __entry->pkt_filled,
			__entry->pkt_to_fill, __entry->available_drb)
	);

TRACE_EVENT(mtk_tput_data_napi,
	    TP_PROTO(int q_id, int rx_pkt),

		TP_ARGS(q_id, rx_pkt),

		TP_STRUCT__entry(__field(int, q_id)
			__field(int, rx_pkt)
		),

		TP_fast_assign(__entry->q_id = q_id;
			__entry->rx_pkt = rx_pkt;
		),

		TP_printk("%d, %d", __entry->q_id, __entry->rx_pkt)
	);

TRACE_EVENT(mtk_tput_data_napi_per_poll,
	    TP_PROTO(int q_id, int pit_cnt),

		TP_ARGS(q_id, pit_cnt),

		TP_STRUCT__entry(__field(int, q_id)
			__field(int, pit_cnt)
		),

		TP_fast_assign(__entry->q_id = q_id;
			__entry->pit_cnt = pit_cnt;
		),

		TP_printk("%d, %d", __entry->q_id, __entry->pit_cnt)
	);

TRACE_EVENT(mtk_tput_data_bat,
	    TP_PROTO(int type, int rel_cnt, int wr_idx, int rd_idx, int rel_rd_idx),

		TP_ARGS(type, rel_cnt, wr_idx, rd_idx, rel_rd_idx),

		TP_STRUCT__entry(__field(int, type)
		__field(int, rel_cnt)
		__field(int, wr_idx)
		__field(int, rd_idx)
		__field(int, rel_rd_idx)
		),

		TP_fast_assign(__entry->type = type;
		__entry->rel_cnt = rel_cnt;
		__entry->wr_idx = wr_idx;
		__entry->rd_idx = rd_idx;
		__entry->rel_rd_idx = rel_rd_idx;
		),

		TP_printk("%d, %d, %d, %d, %d",
			  __entry->type, __entry->rel_cnt, __entry->wr_idx,
			  __entry->rd_idx, __entry->rel_rd_idx)
	);

TRACE_EVENT(mtk_tput_data_pit,
	    TP_PROTO(int id, int pit_rel_rd_idx, int pit_wr_idx, int pit_rd_idx, int pit_rel_cnt),

		TP_ARGS(id, pit_rel_rd_idx, pit_wr_idx, pit_rd_idx, pit_rel_cnt),

		TP_STRUCT__entry(__field(int, id)
		__field(int, pit_rel_rd_idx)
		__field(int, pit_wr_idx)
		__field(int, pit_rd_idx)
		__field(int, pit_rel_cnt)
		),

		TP_fast_assign(__entry->id = id;
		__entry->pit_rel_rd_idx = pit_rel_rd_idx;
		__entry->pit_wr_idx = pit_wr_idx;
		__entry->pit_rd_idx = pit_rd_idx;
		__entry->pit_rel_cnt = pit_rel_cnt;
		),

		TP_printk("%d, %d, %d, %d, %d",
			  __entry->id, __entry->pit_rel_rd_idx, __entry->pit_wr_idx,
			  __entry->pit_rd_idx, __entry->pit_rel_cnt)
	);

TRACE_EVENT(mtk_data_pit_burst_cnt,
	    TP_PROTO(unsigned int id, unsigned int cnt),

		TP_ARGS(id, cnt),

		TP_STRUCT__entry(__field(unsigned int, id)
		__field(unsigned int, cnt)
		),

		TP_fast_assign(__entry->id = id;
		__entry->cnt = cnt;
		),

		TP_printk("%u, %u",
			  __entry->id, __entry->cnt)
	);

TRACE_EVENT(mtk_data_doorbell,
	    TP_PROTO(unsigned int type, unsigned int id, unsigned int cnt),

		TP_ARGS(type, id, cnt),

		TP_STRUCT__entry(__field(unsigned int, type)
				 __field(unsigned int, id)
				 __field(unsigned int, cnt)),

		TP_fast_assign(__entry->type = type;
			       __entry->id = id;
			       __entry->cnt = cnt;),

		TP_printk("%u, %u, %u", __entry->type, __entry->id, __entry->cnt)
);

TRACE_EVENT(mtk_debug_dump,
	    TP_PROTO(struct device *dev, const char *msg, const void *buf, size_t buf_len),
	 TP_ARGS(dev, msg, buf, buf_len),
	 TP_STRUCT__entry(__string(device, dev_name(dev))
		 __string(msg, msg)
		 __field(size_t, buf_len)
		 __dynamic_array(u8, buf, buf_len)
	 ),

	 TP_fast_assign(__assign_str(device, dev_name(dev));
		 __assign_str(msg, msg);
		 __entry->buf_len = buf_len;
		 memcpy(__get_dynamic_array(buf), buf, buf_len);
	 ),

	 TP_printk("%s %s len=%zu, contents=[%s]\n",
		   __get_str(device),
		 __get_str(msg),
		 __entry->buf_len,
		 __print_hex(__get_dynamic_array(buf), __entry->buf_len)
	 )
);

TRACE_EVENT(mtk_tput_data_lro_cnt,
	    TP_PROTO(int rx_qid, unsigned int lro_pkt_cnt),

		TP_ARGS(rx_qid, lro_pkt_cnt),

		TP_STRUCT__entry(__field(int, rx_qid)
		__field(unsigned int, lro_pkt_cnt)
		),

		TP_fast_assign(__entry->rx_qid = rx_qid;
		__entry->lro_pkt_cnt = lro_pkt_cnt;
		),

		TP_printk("%d, %u", __entry->rx_qid, __entry->lro_pkt_cnt)
	);

#endif /* __MTK_TRACE_EVENTS_H */

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE mtk_trace
#include <trace/define_trace.h>

