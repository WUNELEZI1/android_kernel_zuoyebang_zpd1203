O1_BASIC = {
    "boot_image_header_version": 4,
    "base_address": 0x00,
    "kernel_offset": 0x200000,
    "dtb_offset": 0x4200000,
    "ramdisk_offset": 0x4400000,
    "page_size": 4096,
    "lz4_ramdisk": True,
    "common_cmdline": [
        "log_buf_len=1M",
        "firmware_class.path=/vendor/firmware,/odm/firmware,",
        "loglevel=7",
        "fw_devlink.strict=1",
        "workqueue.power_efficient=true",
        "erofs.reserved_pages=64",
        "pcie_port_pm=force",
        "ftrace_dump_on_oops",
        "loop.max_part=7"
    ],
    "common_bootconfig": [
        "androidboot.boot_device_root=super",
        "androidboot.force_super_partition=super",
        "androidboot.super_partition=super"
    ],
}

IMG_INFO = {
    "O1": O1_BASIC,
}
def get_basic_img_info(soc_type):
        return IMG_INFO[soc_type]


