def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/mi_stack",
        out = "mi_stack.ko",
        config = "CONFIG_MI_STACK",
        srcs = [
            # do not sort
            "drivers/xiaomi/mi_stack/mi_stack.c",
        ],
    )
