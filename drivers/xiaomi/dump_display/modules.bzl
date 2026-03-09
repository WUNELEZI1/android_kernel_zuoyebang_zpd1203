def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/dump_display",
        out = "dump_display.ko",
        config = "CONFIG_MI_DUMP_DISPLAY",
        srcs = [
            # do not sort
            "drivers/xiaomi/dump_display/dump_display.c",
        ],
    )
