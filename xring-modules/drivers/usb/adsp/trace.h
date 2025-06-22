/* SPDX-License-Identifier: GPL-2.0 */
/*
 * XRing USB ADSP Driver.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM usb-adsp

/*
 * The TRACE_SYSTEM_VAR defaults to TRACE_SYSTEM, but must be a
 * legitimate C variable. It is not exported to user space.
 */
#undef TRACE_SYSTEM_VAR
#define TRACE_SYSTEM_VAR usb_adsp

#if !defined(__XR_USB_ADSP_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define __XR_USB_ADSP_TRACE_H

#include <linux/tracepoint.h>
#include "core.h"

DECLARE_EVENT_CLASS(usb_adsp_log_msg,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf),
	TP_STRUCT__entry(__dynamic_array(char, msg, XR_USB_TRACE_MSG_MAX)),
	TP_fast_assign(
		vsnprintf(__get_str(msg), XR_USB_TRACE_MSG_MAX, vaf->fmt, *vaf->va);
	),
	TP_printk("%s", __get_str(msg))
);

DEFINE_EVENT(usb_adsp_log_msg, usb_adsp_hc_driver,
	TP_PROTO(struct va_format *vaf),
	TP_ARGS(vaf)
);

#endif /* __XR_USB_ADSP_TRACE_H */

/* this part must be outside header guard */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace

#include <trace/define_trace.h>
