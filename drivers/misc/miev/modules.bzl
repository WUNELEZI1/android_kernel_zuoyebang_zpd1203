def register_modules(registry):
    registry.register(
        name = "drivers/misc/miev",
        out = "miev.ko",
        config = "CONFIG_MIEV",
        srcs = [
        ],
    )
