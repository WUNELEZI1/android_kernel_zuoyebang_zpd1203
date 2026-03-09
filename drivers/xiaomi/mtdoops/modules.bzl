def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/mtdoops",
        out = "mtdoops.ko",
        config = "CONFIG_MI_MTDOOPS",
        srcs = [
            # do not sort
            "drivers/xiaomi/mtdoops/mtdoops.c",
            "drivers/xiaomi/mtdoops/mtdoops.h",
        ],
        deps = [
            "drivers/xiaomi/swinfo",
            "drivers/input/misc/pm8941-pwrkey",
        ],
    )
