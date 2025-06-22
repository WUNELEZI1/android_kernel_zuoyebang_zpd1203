#define CREATE_TRACE_POINTS
#include "codec-trace.h"

EXPORT_TRACEPOINT_SYMBOL_GPL(codec_event);

void codec_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
	...)
{
	struct va_format vaf;
	va_list args;

	//pr_err("%s: enter\n", __func__);

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	trace(&vaf);
	va_end(args);

	//pr_err("%s: exit\n", __func__);
}
EXPORT_SYMBOL(codec_dbg_trace);
