O1_DIFF_VERSION_CONFIG = {
    #user
    "user": {
        "define_abi_targets": True,
        "diff_cmdline": [
        "",
        ],
        "diff_bootconfig": [
            "androidboot.selinux=enforcing",
        ],
    },
    #eng
    "eng": {
        "define_abi_targets": False,
        "diff_cmdline": [
        "",
        ],
        "diff_bootconfig":[
        "androidboot.selinux=enforcing",
        ]
    },
    #slt
    "slt": {
        "define_abi_targets": False,
        "diff_cmdline": [
        "",
        ],
        "diff_bootconfig":[
        "androidboot.selinux=permissive",
        ]
    },
    #factory
    "factory": {
        "define_abi_targets": False,
        "diff_cmdline": [
        "",
        ],
        "diff_bootconfig":[
        "androidboot.selinux=permissive",
        "androidboot.factorybuild=1",
        ]
    },
    #kasan
    "kasan": {
        "define_abi_targets": False,
        "diff_cmdline": [
        "kasan.fault=panic",
        "kasan.stacktrace=on",
        "stack_depot_disable=off",
        "page_owner=on",
        ],
        "diff_bootconfig": [
        "androidboot.selinux=permissive",
    ],
    },
}

O1_PRODUCT_TYEPS = [
    "dijun",
    "jinghu",
]
O1_BOOT_MODE = [
    "fullboot",
]
O1_CONFIG = {
    "soc_type": "O1",
    "boot_modes": O1_BOOT_MODE,
    "build_config": [
            "//xring-configs/O1:build.config.O1.aarch64",
    ],
    "diff_version_config": O1_DIFF_VERSION_CONFIG,
    "product_types": O1_PRODUCT_TYEPS,
}

SOC_SETS=[
    O1_CONFIG,
    # {},
]

