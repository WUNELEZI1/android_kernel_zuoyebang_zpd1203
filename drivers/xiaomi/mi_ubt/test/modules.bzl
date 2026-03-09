def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/mi_ubt/test/mi_ubt_test",
        out = "mi_ubt_test.ko",
        config = "CONFIG_MI_UBT_TEST",
        srcs = [
            # do not sort
            "drivers/xiaomi/mi_ubt/test/mi_ubt_test.c",
        ],
        deps = [
            "drivers/xiaomi/mi_ubt",
        ],
    )
