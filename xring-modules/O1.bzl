# Copyright (C) 2021 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load(":image_opts.bzl", "boot_image_opts")
load(":xring_soc_type.bzl", "SOC_SETS","O1_PRODUCT_TYEPS")
load("//xring-modules:modules_list.bzl","MODULE_OUTS")
load("//xring-dts:dtb_src.bzl","get_dtbo_list","get_dtb_list")
load("//xring-modules:xring_kernel_single.bzl","define_xring_single")

_XIRNG_OUTS = [".config", "Module.symvers"]
_XRING_MAKE_GOALS = [
    "modules",
    "dtbs",
    ]
_GKI_MODULES = []

def get_pack_module(
    soc_type,
    product_type,
    version
):
    # -------------------xring's pcak module list-------------------------------------
    vendor_boot_modules = [
        "//xring-configs/{}:vendor_boot_modules.xring_{}".format(soc_type, soc_type),
    ]
    vendor_dlkm_modules = [
        "//xring-configs/{}:vendor_dlkm_modules.xring_{}".format(soc_type, soc_type),
    ]
    vendor_boot_modules_blocklist = []
    vendor_dlkm_modules_blocklist = [
        "//xring-configs/{}:vendor_dlkm_blocklist.xring_{}".format(soc_type, soc_type)
    ]
    if version in ["kasan", "eng", "factory", "slt"]:
        vendor_boot_modules = vendor_boot_modules + [
            "//xring-configs/{}:vendor_boot_modules_eng.xring_{}".format(soc_type, soc_type),
        ]
        vendor_dlkm_modules = vendor_dlkm_modules + [
            "//xring-configs/{}:vendor_dlkm_modules_eng.xring_{}".format(soc_type, soc_type),
        ]
    modules_recovery_list = vendor_boot_modules + [
        "//xring-configs/{}:recovery_modules.xring_{}".format(soc_type, soc_type),
    ]
    # dijun and jinghu
    if product_type in ["dijun", "jinghu"]:
        vendor_boot_modules = vendor_boot_modules + [
            "//xring-configs/xiaomi/{}:vendor_boot_modules.{}".format(product_type, product_type),
        ]
        vendor_dlkm_modules = vendor_dlkm_modules + [
            "//xring-configs/xiaomi/{}:vendor_dlkm_modules.{}".format(product_type, product_type),
        ]
        modules_recovery_list = modules_recovery_list + [
            "//xring-configs/xiaomi/{}:recovery_modules.{}".format(product_type, product_type),
        ]
        vendor_dlkm_modules_blocklist = vendor_dlkm_modules_blocklist + [
            "//xring-configs/xiaomi/{}:vendor_dlkm_blocklist.{}".format(product_type, product_type)
        ]
        if version in ["eng", "slt"]:
            vendor_boot_modules = vendor_boot_modules + [
                "//xring-configs/xiaomi/{}:vendor_boot_modules_eng.{}".format(product_type, product_type),
            ]
            vendor_dlkm_modules = vendor_dlkm_modules + [
                "//xring-configs/xiaomi/{}:vendor_dlkm_modules_eng.{}".format(product_type, product_type),
            ]
    else:
        vendor_boot_modules = vendor_boot_modules + [
            "//xring-configs/xiaomi/dijun:vendor_boot_modules.dijun",
        ]
        vendor_dlkm_modules = vendor_dlkm_modules + [
            "//xring-configs/xiaomi/dijun:vendor_dlkm_modules.dijun",
        ]
        modules_recovery_list = modules_recovery_list + [
            "//xring-configs/xiaomi/dijun:recovery_modules.dijun",
        ]
        if version in ["eng", "slt"]:
            vendor_boot_modules = vendor_boot_modules + [
                "//xring-configs/xiaomi/dijun:vendor_boot_modules_eng.dijun",
            ]
            vendor_dlkm_modules = vendor_dlkm_modules + [
                "//xring-configs/xiaomi/dijun:vendor_dlkm_modules_eng.dijun",
            ]
    return {
        "vendor_boot_modules": vendor_boot_modules,
        "vendor_dlkm_modules": vendor_dlkm_modules,
        "modules_recovery_list": modules_recovery_list,
        "vendor_boot_modules_blocklist": vendor_boot_modules_blocklist,
        "vendor_dlkm_modules_blocklist": vendor_dlkm_modules_blocklist,
    }

# for xring Software products developers
def product_entry(
    # soc_set,
    version,
    dtstree,
    soc_type,
    product_types,
    build_config,
    define_abi_targets,
    defconfig_fragments,
    diff_cmdline,
    diff_bootconfig,
    outs,
):
    for product_type in product_types:
        additional_module_outs = {}
        in_defconfig_fragments = defconfig_fragments;
        pcak_module = get_pack_module(soc_type, product_type, version)
        if product_type in O1_PRODUCT_TYEPS:
            kmi_symbol_list = "//common:android/abi_gki_aarch64_xiaomi"
            additional_kmi_symbol_lists = ["//common:android/abi_gki_aarch64_xiaomi2"]
        else:
            kmi_symbol_list = "//common:android/abi_gki_aarch64_xiaomi2"
            additional_kmi_symbol_lists = ["//common:android/abi_gki_aarch64_xiaomi"]
        define_xring_single(
            name = "xring_{}_{}_{}".format(soc_type, product_type, version),
            soc_type = soc_type,
            version = version,
            product_type = product_type,
            srcs = ["xring_device_common_sources"],
            outs = outs,
            additional_module_outs = additional_module_outs if additional_module_outs else None,
            build_config = build_config,
            make_goals = _XRING_MAKE_GOALS,
            define_abi_targets = define_abi_targets,
            kmi_symbol_list = kmi_symbol_list,
            additional_kmi_symbol_lists = additional_kmi_symbol_lists,
            module_grouping = False,
            defconfig_fragments = in_defconfig_fragments,
            dtstree = dtstree,
            build_initramfs = True,
            build_vendor_boot = True,
            build_vendor_dlkm = True,
            vendor_boot_modules = pcak_module["vendor_boot_modules"],
            modules_recovery_list = pcak_module["modules_recovery_list"],
            vendor_dlkm_modules = pcak_module["vendor_dlkm_modules"],
            modules_blocklist = pcak_module["vendor_boot_modules_blocklist"] if pcak_module["vendor_boot_modules_blocklist"] else None,
            vendor_dlkm_modules_blocklist = pcak_module["vendor_dlkm_modules_blocklist"],
            gki_modules = _GKI_MODULES,
            boot_image_opts = boot_image_opts(
                soc_type = soc_type,
                kernel_vendor_cmdline = diff_cmdline,
                kernel_vendor_bootconfig = diff_bootconfig,
            )
        )

def define_O1():
    soc_set = SOC_SETS[0]
    soc_type = soc_set["soc_type"]
    boot_modes = soc_set["boot_modes"]
    product_types = soc_set["product_types"] + boot_modes
    build_config = soc_set["build_config"]
    dtstree = "//xring-dts/{}:{}_dt".format(soc_type,soc_type)
    outs = _XIRNG_OUTS + get_dtbo_list(soc_type) + get_dtb_list(soc_type)
    for version, config in  soc_set["diff_version_config"].items():
        defconfig_fragments = []
        define_abi_targets = config["define_abi_targets"]
        if version == "eng":
            defconfig_fragments = [
                "//xring-configs/{}:{}_eng_defconfig".format(soc_type,soc_type),
            ]
        elif version == "kasan":
            defconfig_fragments = [
                "//xring-configs:xring_kasan_defconfig",
            ]
        elif version == "slt":
            defconfig_fragments = [
                "//xring-configs/{}:{}_eng_defconfig".format(soc_type,soc_type),
                "//xring-configs:xring_slt_defconfig",
            ]
        elif version == "factory":
            defconfig_fragments = [
                "//xring-configs/{}:{}_eng_defconfig".format(soc_type,soc_type),
                "//xring-configs:xring_factory_defconfig",
            ]
        diff_cmdline=config["diff_cmdline"]
        diff_bootconfig=config["diff_bootconfig"]
        product_entry(
            # soc_set=soc_set,
            version=version,
            dtstree=dtstree,
            soc_type=soc_type,
            product_types=product_types,
            build_config=build_config,
            define_abi_targets=define_abi_targets,
            defconfig_fragments=defconfig_fragments,
            diff_cmdline=diff_cmdline,
            diff_bootconfig=diff_bootconfig,
            outs=outs
        )
