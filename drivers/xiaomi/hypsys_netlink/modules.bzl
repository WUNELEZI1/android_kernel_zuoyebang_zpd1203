def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/hypsys_netlink/hypsys_netlink",
        out = "hypsys_netlink.ko",
        config = "CONFIG_MI_HYPSYS_NETLINK",
        srcs = [
            # do not sort
            "drivers/xiaomi/hypsys_netlink/hypsys_netlink.c",
            "drivers/xiaomi/hypsys_netlink/hypsys_netlink.h",
        ],
    )
