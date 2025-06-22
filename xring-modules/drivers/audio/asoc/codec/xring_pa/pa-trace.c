// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define CREATE_TRACE_POINTS
#include "pa-trace.h"

EXPORT_TRACEPOINT_SYMBOL_GPL(pa_event);

void pa_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
	...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	trace(&vaf);
	va_end(args);
}
EXPORT_SYMBOL(pa_dbg_trace);

