def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/printk_enhance",
        out = "printk_enhance.ko",
        config = "CONFIG_PRINTK_ENHANCE",
        srcs = [
            # do not sort
            "drivers/xiaomi/printk_enhance/printk_enhance.c",
        ],
    )
