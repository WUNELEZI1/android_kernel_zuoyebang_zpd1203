load(
    "//build/kernel/kleaf:kernel.bzl",
    "kernel_abi",
    "kernel_abi_dist",
    "kernel_build",
    "kernel_module",
    "kernel_build_config",
    "kernel_compile_commands",
    "kernel_filegroup",
    "kernel_images",
    "kernel_kythe",
    "kernel_modules_install",
    "kernel_unstripped_modules_archive",
    "merged_kernel_uapi_headers",
)
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("//xring-modules:image_opts.bzl", "boot_image_opts")
load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")
load("//xring-modules:modules_list.bzl","MODULE_OUTS")
load("//build/kernel/kleaf:constants.bzl", "aarch64_outs")
load("//common:modules.bzl", "get_gki_modules_list","get_kunit_modules_list")

def generate_pack_kolist(
    name,
    srcs,
    list_name,
    product_type,
):
    native.genrule(
        name = name,
        srcs = srcs,
        outs = ["modules.{}.{}".format(list_name, name)],
        cmd_bash = """
          for list in $(SRCS); do
            cat "$$list" >> "$@"
          done
        """
    )

def _define_build_config(
    name,
    boot_image_opts,
    build_config_fragments = []):
    """Creates a kernel_build_config for an MSM target

    Creates a `kernel_build_config` for input to a `kernel_build` rule.

    Args:
    target: name of target platform (e.g. "shennong")
    variant: variant of kernel to build (e.g. "gki")
    """
    native.genrule(
        name = "{}_set_platform".format(name),
        outs = ["build.config.{}.set_platfom".format(name)],
        cmd_bash = """
            TARGET_PRODUCT=$${TARGET_PRODUCT:-O1_asic}
            echo "TARGET_PRODUCT=$$TARGET_PRODUCT" > "$@"
            DTC_FLAGS=$${DTC_FLAGS:-''}
            echo "DTC_FLAGS='$$DTC_FLAGS'" >> "$@"
            ODM_FLAGS=$${ODM_FLAGS:-''}
            echo "ODM_FLAGS='$$ODM_FLAGS'" >> "$@"
        """
    )

    write_file(
        name = "{}_build_config_bazel".format(name),
        out = "build.config.{}.generated".format(name),
        content = [
        "BOOT_IMAGE_HEADER_VERSION={}".format(boot_image_opts.boot_image_header_version),
        "BASE_ADDRESS=0x%X" % boot_image_opts.base_address,
        "KERNEL_OFFSET=0x%X" % boot_image_opts.kernel_offset,
        "DTB_OFFSET=0x%X" % boot_image_opts.dtb_offset,
        "RAMDISK_OFFSET=0x%X" % boot_image_opts.ramdisk_offset,
        "PAGE_SIZE={}".format(boot_image_opts.page_size),
        "LZ4_RAMDISK={}".format(int(boot_image_opts.lz4_ramdisk)),
        "KERNEL_VENDOR_CMDLINE+=' {} '".format(" ".join(boot_image_opts.kernel_vendor_cmdline)),
        "VENDOR_BOOTCONFIG+=' {} '".format(" ".join(boot_image_opts.kernel_vendor_bootconfig)),
        "",  # Needed for newline at end of file
        ],
    )

    # Concatenate build config fragments to form the final config
    kernel_build_config(
        name = "{}_build_config".format(name),
        srcs = [
        "{}_set_platform".format(name),
        ":{}_build_config_bazel".format(name),
        ] + [fragment for fragment in build_config_fragments],
    )

_COLLECT_UNSTRIPPED_MODULES = True
def define_xring_single(
        name = None,
        soc_type = None,
        version = None,
        product_type = None,
        srcs = None,
        outs = None,
        additional_module_outs=None,
        build_config = None,
        make_goals = None,
        define_abi_targets = None,
        kmi_symbol_list = None,
        additional_kmi_symbol_lists = None,
        kmi_symbol_list_add_only = None,
        module_grouping = None,
        defconfig_fragments = None,
        dtstree = None,
        build_dtbo = None,
        build_initramfs = None,
        build_vendor_boot = None,
        build_vendor_dlkm = None,
        dtbo_srcs = None,
        vendor_boot_modules = None,
        modules_blocklist = None,
        modules_recovery_list = None,
        vendor_dlkm_modules = None,
        vendor_dlkm_modules_blocklist = None,
        vendor_dlkm_props = None,
        dist_dir = None,
        gki_modules = None,
        boot_image_opts = None,
        ):
    """Define target for O1.

    Note: This is a mixed build.

    Requires [`define_common_kernels`](#define_common_kernels) to be called in the same package.

    **Deprecated**. Use [`kernel_build`](#kernel_build) directly.

    Args:
        name: name of target. Usually `"O1"`.

        product_type: name of product.

        srcs: see [kernel_build.srcs] and [kernel_module.srcs]

        build_config: If `None`, default to `["//xring-configs:build.config.O1.aarch64"]`.

        outs: See [kernel_build.outs](#kernel_build-outs).

        make_goals: A list of strings defining targets for the kernel build.

        define_abi_targets: See [kernel_abi.define_abi_targets](#kernel_abi-define_abi_targets).

        kmi_symbol_list: See [kernel_build.kmi_symbol_list](#kernel_build-kmi_symbol_list).

        kmi_symbol_list_add_only: See [kernel_abi.kmi_symbol_list_add_only](#kernel_abi-kmi_symbol_list_add_only).

        module_grouping: See [kernel_abi.module_grouping](#kernel_abi-module_grouping).

        dtstree: see [kernel_build.dtstree]

        build_dtbo: Whether to build dtboimage, see [kernel_images.build_dtbo]

        build_initramfs: Whether to build initramfs, see [kernel_images.build_initramfs]

        build_vendor_boot: Whether to build vendorbootimage, see [kernel_images.build_vendor_boot]

        build_vendor_dlkm: Whether to build vendordlkmimage, see [kernel_images.build_vendor_dlkm]

        dtbo_srcs: Build dtboimage's srcs, see [kernel_images.dtbo_srcs]

        vendor_boot_modules: Vendorbootimage's modules list, see [kernel_images.modules_list]

        modules_blocklist: Vendorbootimage's modules blocklist, see [kernel_images.modules_blocklist]

        modules_recovery_list: see [kernel_images.modules_recovery_list]
            Recovery Mode contains drivers include :
                vendor_boot_modules + modules_recovery_list

        vendor_dlkm_modules: Vendordlkmimage's modules list, see [kernel_images.vendor_dlkm_modules]

        vendor_dlkm_modules_blocklist: Vendordlkmimage's module block list, see [kernel_images.vendor_dlkm_modules_blocklist]

        vendor_dlkm_props: a text file containing the properties to be used for creation of a `vendor_dlkm` image

        dist_dir: Argument to `copy_to_dist_dir`. If `None`, default is `"out/{name}/dist"`.

    Deprecated:
        Use [`kernel_build`](#kernel_build) directly.
    """
    _define_build_config(
        name = name,
        boot_image_opts = boot_image_opts,
        build_config_fragments = build_config)

    if kmi_symbol_list == None:
        kmi_symbol_list = "//common:android/abi_gki_aarch64_xiaomi" if define_abi_targets else None

    if kmi_symbol_list_add_only == None:
        kmi_symbol_list_add_only = True if define_abi_targets else None

    if build_dtbo == None:
        build_dtbo = bool(dtbo_srcs)

    if dist_dir == None:
        dist_dir = "out/android15-6.6/dist"

    if defconfig_fragments == None:
        defconfig_fragments = ["//build/kernel/kleaf/impl:empty_filegroup"]

    implicit_outs = []
    if define_abi_targets == False:
        outs = outs + aarch64_outs
        make_goals = make_goals + [
            "Image",
            "Image.lz4",
            "Image.gz",
        ]
        implicit_outs = implicit_outs + [
            "scripts/sign-file",
            "certs/signing_key.pem",
            "certs/signing_key.x509",
        ]
    common_module_outs = MODULE_OUTS["{}".format(soc_type)]["common_modules"]
    if version in ["kasan", "eng", "factory", "slt"]:
        common_module_outs = common_module_outs + MODULE_OUTS["{}".format(soc_type)]["common_modules_eng"]
    kernel_build(
        name = name,
        srcs = srcs,
        outs = outs,
        module_outs = common_module_outs,
        build_config = name + "_build_config",
        base_kernel = "//common:kernel_aarch64" if define_abi_targets else None,
        dtstree = dtstree,
        kmi_symbol_list = kmi_symbol_list,
        additional_kmi_symbol_lists = additional_kmi_symbol_lists,
        collect_unstripped_modules = _COLLECT_UNSTRIPPED_MODULES,
        strip_modules = True,
        make_goals = make_goals,
        kconfig_ext = ":Kconfig.ext",
        defconfig_fragments = defconfig_fragments,
        module_implicit_outs = None if define_abi_targets else get_gki_modules_list("arm64") + get_kunit_modules_list("arm64"),
        implicit_outs = implicit_outs,

    )
    # Also refer to the list of ext modules for ABI monitoring targets
    _XRING_EXT_MODULES = [
        name + "_external_modules",
	]
    if soc_type == "O1":
        _XRING_EXT_MODULES = _XRING_EXT_MODULES + [
            "//xring-modules/drivers/connectivity/connfem:connfem_{}".format(name),
            "//xring-modules/drivers/connectivity/conninfra:conninfra_{}".format(name),
            "//xring-modules/drivers/connectivity/connadp:connadp_{}".format(name),
            "//xring-modules/drivers/connectivity/wlan/adaptor/build/connac3x:wmt_chrdev_wifi_connac3_{}".format(name),
            "//xring-modules/drivers/connectivity/wlan/adaptor/wlan_page_pool:wlan_page_pool_{}".format(name),
            "//xring-modules/drivers/connectivity/wlan/core/gen4m/build/connac3x/eap_6639:wlan_drv_gen4m_eap_6639_{}".format(name),
            "//xring-modules/drivers/connectivity/bt/linux_v2:btmtk_uart_unify_{}".format(name),
            "//xiaomi-modules:xiaomi_modules_{}".format(name),
	]

    # enable ABI Monitoring
    # based on the instructions here:
    # https://android.googlesource.com/kernel/build/+/refs/heads/main/kleaf/docs/abi_device.md
    # https://android-review.googlesource.com/c/kernel/build/+/2308912
    kernel_abi(
        name = name + "_abi",
        kernel_build = name,
        define_abi_targets = define_abi_targets,
        kernel_modules = _XRING_EXT_MODULES,
        kmi_symbol_list_add_only = kmi_symbol_list_add_only,
        module_grouping = module_grouping,
    )
    module_outs = MODULE_OUTS["{}".format(soc_type)]["module_outs"]
    if version in ["kasan", "eng", "factory", "slt"]:
        module_outs = module_outs + MODULE_OUTS["{}".format(soc_type)]["module_outs_eng"]
    if additional_module_outs:
        if len(additional_module_outs["exclude_module"]):
            module_outs = [module for module in module_outs if module not in additional_module_outs["exclude_module"] ]
        module_outs = module_outs + additional_module_outs["private_module"]

    kernel_module(
        name = name + "_external_modules",
        srcs = srcs,
        outs = module_outs,
        kernel_build = name,
        visibility = ["//visibility:public",],
    )
    kernel_modules_install(
        name = name + "_modules_install",
        kernel_build = name,
        # List of external modules.
        kernel_modules = _XRING_EXT_MODULES,
    )

    kernel_unstripped_modules_archive(
        name = name + "_unstripped_modules_archive",
        kernel_build = name,
        kernel_modules = _XRING_EXT_MODULES,
    )

    merged_kernel_uapi_headers(
        name = name + "_merged_kernel_uapi_headers",
        kernel_build = name,
        kernel_modules = _XRING_EXT_MODULES,
    )
    kernel_compile_commands(
        name = name+"_compile_commands",
        kernel_build = name,
    )
    # Generate the vendor_boot list
    native.genrule(
        name = "{}_vendor_boot_modules_list_generated".format(name),
        srcs = vendor_boot_modules,
        outs = ["modules.list.vendor_boot.{}".format(name)],
        cmd_bash = """
          for list in $(SRCS); do
            cat "$$list" >> "$@"
          done
        """
    )
    # Generate the vendor_dlkm list
    native.genrule(
        name = "{}_vendor_dlkm_modules_list_generated".format(name),
        srcs = vendor_dlkm_modules,
        outs = ["modules.list.vendor_dlkm.{}".format(name)],
        cmd_bash = """
          for list in $(SRCS); do
            cat "$$list" >> "$@"
          done
        """
    )
    if modules_blocklist != None:
        native.genrule(
            name = "{}_vendor_boot_blocklist_generated".format(name),
            srcs = modules_blocklist,
            outs = ["modules.list.vendor_boot_blocklist.{}".format(name)],
            cmd_bash = """
            for list in $(SRCS); do
                cat "$$list" >> "$@"
            done
            """
        )

    # Generate the vendor_dlkm blocklist
    native.genrule(
        name = "{}_vendor_dlkm_blocklist_generated".format(name),
        srcs = vendor_dlkm_modules_blocklist,
        outs = ["modules.list.vendor_dlkm_blocklist.{}".format(name)],
        cmd_bash = """
          for list in $(SRCS); do
            cat "$$list" >> "$@"
          done
        """
    )

    # Generate the recovery module list
    native.genrule(
        name = "{}_recovery_modules_list_generated".format(name),
        srcs = modules_recovery_list,
        outs = ["modules.list.recovery.{}".format(name)],
        cmd_bash = """
          for list in $(SRCS); do
            cat "$$list" >> "$@"
          done
        """
    )

    kernel_images(
        name = name + "_images",
        kernel_build = name,
        kernel_modules_install = name + "_modules_install",
        build_boot = False if define_abi_targets else True,
        build_dtbo = build_dtbo,
        build_initramfs = build_initramfs,
        ramdisk_compression = "lz4",
        build_vendor_boot = build_vendor_boot,
        build_vendor_dlkm = build_vendor_dlkm,
        vendor_dlkm_archive = True,
        build_system_dlkm = False if define_abi_targets else True,
        system_dlkm_modules_list = "//common:android/gki_system_dlkm_modules_arm64",
        system_dlkm_modules_blocklist = "//xring-configs/{}:system_dlkm_blocklist.xring_{}".format(soc_type, soc_type),
        dtbo_srcs = dtbo_srcs,
        modules_list = "{}_vendor_boot_modules_list_generated".format(name),
        modules_blocklist = "{}_vendor_boot_blocklist_generated".format(name) if modules_blocklist != None else None,
        modules_recovery_list = "{}_recovery_modules_list_generated".format(name),
        vendor_dlkm_modules_list = "{}_vendor_dlkm_modules_list_generated".format(name),
        vendor_dlkm_modules_blocklist = "{}_vendor_dlkm_blocklist_generated".format(name),
        vendor_dlkm_props = vendor_dlkm_props,
        deps = [
            "//xring-configs:xring_configs",
        ],
    )

    native.genrule(
        name = name+"_system_dlkm_module_blocklist",
        srcs = ["//xring-configs/{}:system_dlkm_blocklist.xring_{}".format(soc_type, soc_type)],
        outs = ["{}/system_dlkm.blocklist".format(name)],
        cmd = """
            mkdir -p "$$(dirname "$@")"
            sed -e '/^#/d' -e '/^$$/d' $(SRCS) > "$@"
            """,
    )

    dist_targets = [
        name,
        name+"_system_dlkm_module_blocklist",
        name + "_images",
        name + "_modules_install",
        name + "_merged_kernel_uapi_headers",
        name + "_unstripped_modules_archive",
    ] + gki_modules

    if define_abi_targets == True:
        dist_targets = dist_targets + [
            "//common:kernel_aarch64",
            "//common:kernel_aarch64_additional_artifacts",
        ]

    copy_to_dist_dir(
        name = "{}_dist".format(name),
        data = dist_targets,
        dist_dir = dist_dir,
        flat = True,
        log = "info",
    )
