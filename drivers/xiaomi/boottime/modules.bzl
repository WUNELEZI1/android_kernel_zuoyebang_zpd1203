def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/boottime",
        out = "boottime.ko",
        config = "CONFIG_MI_BOOT_TIME",
        srcs = [
            # do not sort
            "drivers/xiaomi/boottime/boottime.c",
            "drivers/xiaomi/boottime/boottime.h",
        ],
    )
