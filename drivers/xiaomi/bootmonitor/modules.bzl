def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/bootmonitor",
        out = "bootmonitor.ko",
        config = "CONFIG_MI_BOOT_MONITOR",
        srcs = [
            # do not sort
            "drivers/xiaomi/bootmonitor/bm_device.c",
            "drivers/xiaomi/bootmonitor/bm_netlink.c",
            "drivers/xiaomi/bootmonitor/boot_fail.h",
            "drivers/xiaomi/bootmonitor/boot_monitor.c",
            "drivers/xiaomi/bootmonitor/boot_monitor.h",
            "drivers/xiaomi/boottime/boottime.h",
        ],
        deps = [
            "drivers/xiaomi/swinfo",
            "drivers/xiaomi/boottime",
        ],
    )
