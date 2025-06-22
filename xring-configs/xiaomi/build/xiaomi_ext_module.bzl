load(
    "//build/kernel/kleaf:kernel.bzl",
    "kernel_module",
)

load("//xring-modules:xring_soc_type.bzl", "SOC_SETS")

load(":modules_list.bzl", "F1_XIAOMI_MODULE_DICT")

def get_module_list(
    soc_set = None,
    product_type = None,
    version = None,
):
    if product_type == None or version == None:
        print("incorrect product_type or incorrect version")

    if product_type in soc_set["boot_modes"]:
        product_type="xring"
    mod_list= F1_XIAOMI_MODULE_DICT[product_type]["module_outs"]
    if version == "eng":
        mod_list = mod_list + F1_XIAOMI_MODULE_DICT[product_type]["module_outs_eng"]
    return mod_list


def xiaomi_ext_module(
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
    for soc_set in SOC_SETS:
        soc_type=soc_set["soc_type"]
        product_types=soc_set["product_types"] + soc_set["boot_modes"]
        for version, config in  soc_set["diff_version_config"].items():
            for product_type in product_types:
                # xring_{O1/...}_{miniboot/fullboot/dijun/...}_{eng/user/kasan}
                kernel_build_name = "xring_{}_{}_{}".format(soc_type, product_type, version)
                mod_outs = get_module_list(
                    soc_set=soc_set,
                    product_type=product_type,
                    version=version
                )
                kernel_module(
                    name = "{}_{}".format(name, kernel_build_name),
                    srcs = srcs,
                    outs = mod_outs,
                    kernel_build = "//xring-modules:{}".format(kernel_build_name),
                    deps = [
                        "//xring-modules:{}_external_modules".format(kernel_build_name),
                    ] + [
                        "{}_{}".format(dep, kernel_build_name) for dep in deps
                    ],
                )
