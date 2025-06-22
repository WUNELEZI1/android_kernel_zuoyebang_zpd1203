load(
    "//build/kernel/kleaf:kernel.bzl",
    "kernel_module",
)
load(":xring_soc_type.bzl", "SOC_SETS")

def define_ext_module(
        name,
        srcs = None,
        outs = None,
        deps = []):
    if srcs == None:
        srcs = native.glob(
            [
                "**/*.c",
                "**/*.h",
                "**/Kbuild",
                "**/Makefile",
            ],
            exclude = [
                ".*",
                ".*/**",
            ],
        )
    if outs == None:
        outs = [name + ".ko"]
    for soc_set in SOC_SETS:
        soc_type=soc_set["soc_type"]
        product_types=soc_set["product_types"] + soc_set["boot_modes"]
        for version, config in  soc_set["diff_version_config"].items():
            for product_type in product_types:
                kernel_build_name = "xring_{}_{}_{}".format(soc_type, product_type, version)
                kernel_module(
                    name = "{}_{}".format(name, kernel_build_name),
                    srcs = srcs,
                    outs = outs,
                    kernel_build = "//xring-modules:{}".format(kernel_build_name),
                    deps = [
                        "//xring-modules:{}_external_modules".format(kernel_build_name),
                    ] + [
                        "{}_{}".format(dep, kernel_build_name) for dep in deps
                    ],
                )
