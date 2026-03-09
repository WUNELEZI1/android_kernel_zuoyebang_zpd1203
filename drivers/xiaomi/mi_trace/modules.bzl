def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/mi_trace",
        out = "mi_trace.ko",
        config = "CONFIG_MI_TRACE",
        srcs = [
            # do not sort
            "drivers/xiaomi/mi_trace/mi_trace.c",
            "drivers/xiaomi/mi_trace/mi_trace.h",
        ],
        deps = [
            "drivers/soc/qcom/minidump",
        ],
    )
