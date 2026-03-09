load("//build/kernel/kleaf:kernel.bzl", "ddk_headers")
def export_xiaomi_headers():
    ddk_headers(
        name = "hwid_headers",
        hdrs = [
            "drivers/misc/hwid/hwid.h",
        ],
        includes = [
            "drivers/misc/hwid",
        ],
        visibility = ["//visibility:public"],
    )

    
    
    

    ddk_headers(
        name = "debug_symbol_headers",
        hdrs = [
            "drivers/soc/qcom/debug_symbol.h"
        ],
        includes = [
            "drivers/soc/qcom",
        ],
        visibility = ["//visibility:public"],
    )