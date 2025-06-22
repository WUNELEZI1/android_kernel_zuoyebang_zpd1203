load ("//xring-configs:img_common_cfg.bzl","get_basic_img_info")
def boot_image_opts(
    soc_type,
    kernel_vendor_cmdline = [""],
    kernel_vendor_bootconfig = [""],
    ):
    if soc_type == None:
        print("soc_type must set!!")
    basic_info = get_basic_img_info(soc_type)
    return struct(
    boot_image_header_version = basic_info["boot_image_header_version"],
    base_address = basic_info["base_address"],
    kernel_offset= basic_info["kernel_offset"],
    dtb_offset = basic_info["dtb_offset"],
    ramdisk_offset = basic_info["ramdisk_offset"],
    page_size = basic_info["page_size"],
    lz4_ramdisk = basic_info["lz4_ramdisk"],
    # KERNEL_VENDOR_CMDLINE
    kernel_vendor_cmdline = kernel_vendor_cmdline+basic_info["common_cmdline"],
    # VENDOR_BOOTCONFIG
    kernel_vendor_bootconfig = kernel_vendor_bootconfig+basic_info["common_bootconfig"],
    )