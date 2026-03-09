def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/swinfo",
        out = "swinfo.ko",
        config = "CONFIG_MI_SOFTWARE_INFO",
        srcs = [
            # do not sort
            "drivers/xiaomi/swinfo/swinfo_func.c",
        ],

        deps = [
            "drivers/soc/qcom/minidump",
        ],

    )
