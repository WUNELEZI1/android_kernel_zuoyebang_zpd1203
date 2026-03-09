def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/mi_ubt",
        out = "mi_ubt.ko",
        config = "CONFIG_MI_UBT",
        srcs = [
            # do not sort
            "drivers/xiaomi/mi_ubt/mi_ubt.c",
        ],
    )
