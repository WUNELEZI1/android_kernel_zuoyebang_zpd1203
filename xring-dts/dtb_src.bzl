def get_dtbo_list(
    soc_type = None,
):
    if soc_type == None:
        return []

    return DTB_LIST[soc_type]["dtbo_list"]

def get_dtb_list(
    soc_type = None,
):
    if soc_type == None:
        return []

    return DTB_LIST[soc_type]["dtb_list"]
O1_DTBOS = [
    "overlay/product/phone/phone_o2s/o2s_phone_overlay.dtbo",
    "overlay/product/pad/pad_o80/o80_pad_overlay.dtbo",
]
O1_DTBS = [
    "soc/O1_basic.dtb",
]

DTB_LIST = {
    "O1": {
        "dtb_list": O1_DTBS,
        "dtbo_list": O1_DTBOS,
    },
}