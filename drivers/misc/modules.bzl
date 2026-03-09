load(":drivers/misc/lkdtm/modules.bzl", register_lkdtm = "register_modules")
load(":drivers/misc/hwid/modules.bzl", register_hwid = "register_modules")
load(":drivers/misc/dio_dma_mapper/modules.bzl", register_dio_dma_mapper= "register_modules")
load(":drivers/misc/miev/modules.bzl", register_miev = "register_modules")
def register_modules(registry):
    register_lkdtm(registry)
    register_hwid(registry)
    register_dio_dma_mapper(registry)
    register_miev(registry)
    registry.register(
        name = "drivers/misc/qseecom_proxy",
        out = "qseecom_proxy.ko",
        config = "CONFIG_QSEECOM_PROXY",
        srcs = [
            # do not sort
            "drivers/misc/qseecom_proxy.c",
        ],
    )
