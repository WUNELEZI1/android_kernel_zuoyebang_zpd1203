def merge_arrays(kolist1, kolist2):
    result = list(kolist1)
    for item in kolist2:
        if item not in result:
            result.append(item)
    return result

dijun_module_list = [
    "consumerir/ir_spi.ko",
    "touch_driver/o2s/xiaomi_touch_dijun.ko",
    "touch_driver/o2s/synaptics_tcm2.ko",
    "fingerprint/mi_fp/mi_fp.ko",
    "vendor_stability/bootinfo/bootinfo.ko",
    "vendor_stability/dump_display/dump_display.ko",
    "vendor_stability/hardwareinfo/hardwareinfo.ko",
    "mi_net/slm/sla.ko",
    "mi_net/minet/minet.ko",
    "mi_net/miicmpfilter/miicmpfilter.ko",
    "mi_net/miwillp/miwill.ko",
    "mi_net/miwillp/miwill_mode_redudancy.ko",
    "mi_net/ftun/ftun.ko",
    "mi_dynamic_readahead/dynamic_readahead.ko",
    "vendor_stability/mtd/parsers/ofpart.ko",
    "vendor_stability/mtd/mtd_blkdevs.ko",
    "vendor_stability/mtd/mtdoops.ko",
    "vendor_stability/mtd/devices/block2mtd.ko",
    "vendor_stability/mtd/chips/chipreg.ko",
    "vendor_stability/mtd/mtdblock.ko",
    "vendor_stability/mtd/mtd.ko",
    "vendor_stability/boottime/boottime.ko",
]

dijun_eng_module_list = [

]

jinghu_module_list = [
    "consumerir/ir_spi.ko",
    "touch_driver/o80/xiaomi_touch_jinghu.ko",
    "touch_driver/o80/goodix_core.ko",
    "keyboard_driver/jinghu/xiaomi_keyboard_driver.ko",
    "keyboard_driver/jinghu/nanosic_driver.ko",
    "vendor_stability/dump_display/dump_display.ko",
    "vendor_stability/hardwareinfo/hardwareinfo.ko",
    "mi_net/slm/sla.ko",
    "mi_net/minet/minet.ko",
    "mi_net/miicmpfilter/miicmpfilter.ko",
    "mi_net/miwillp/miwill.ko",
    "mi_net/miwillp/miwill_mode_redudancy.ko",
    "mi_net/ftun/ftun.ko",
    "mi_dynamic_readahead/dynamic_readahead.ko",
    "fingerprint/mi_fp/mi_fp.ko",
    "vendor_stability/mtd/parsers/ofpart.ko",
    "vendor_stability/mtd/mtd_blkdevs.ko",
    "vendor_stability/bootinfo/bootinfo.ko",
    "vendor_stability/mtd/mtdoops.ko",
    "vendor_stability/mtd/devices/block2mtd.ko",
    "vendor_stability/mtd/chips/chipreg.ko",
    "vendor_stability/mtd/mtdblock.ko",
    "vendor_stability/mtd/mtd.ko",
    "mi_hall/hall_key.ko",
    "vendor_stability/boottime/boottime.ko",
]

jinghu_eng_module_list = [

]

F1_XIAOMI_MODULE_DICT = {
    "xring":{
        "module_outs": dijun_module_list,
        "module_outs_eng": dijun_eng_module_list,
    },
    "dijun":{
        "module_outs": dijun_module_list,
        "module_outs_eng": dijun_eng_module_list,
    },
    "jinghu":{
        "module_outs": jinghu_module_list,
        "module_outs_eng": jinghu_eng_module_list,
    },
}

F3_XIAOMI_MODULE_DICT = {
    "xring":{
        "module_outs": [],
        "module_outs_eng": [],
    },

}
