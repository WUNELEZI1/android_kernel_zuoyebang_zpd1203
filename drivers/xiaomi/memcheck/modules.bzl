def register_modules(registry):
    registry.register(
        name = "drivers/xiaomi/memcheck/mi_memcheck",
        out = "mi_memcheck.ko",
        config = "CONFIG_MI_MEMCHECK",
        srcs = [
            # do not sort
            "drivers/xiaomi/memcheck/memcheck_mod.c",
            "drivers/xiaomi/memcheck/memcheck_account.c",
            "drivers/xiaomi/memcheck/memcheck_account.h",
            "drivers/soc/qcom/debug_symbol.h",
            "drivers/xiaomi/hypsys_netlink/hypsys_netlink.h",
            "drivers/xiaomi/memcheck/memcheck_ioctl.c",
            "drivers/xiaomi/memcheck/memcheck_ioctl.h",
        ],
        conditional_srcs = {
            "CONFIG_MI_MEMCHECK_PROCESS_MEM": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_process_mem.c",
                    "drivers/xiaomi/memcheck/memcheck_process_mem.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_ASHMEM": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_ashmem.c",
                    "drivers/xiaomi/memcheck/memcheck_ashmem.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_CMA": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_cma.c",
                    "drivers/xiaomi/memcheck/memcheck_cma.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_FD_FENCE": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_fd_fence.c",
                    "drivers/xiaomi/memcheck/memcheck_fd_fence.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_FD_PIPE": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_fd_pipe.c",
                    "drivers/xiaomi/memcheck/memcheck_fd_pipe.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_FD_SOCKET": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_fd_socket.c",
                    "drivers/xiaomi/memcheck/memcheck_fd_socket.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_DMABUF": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_dma-buf.c",
                    "drivers/xiaomi/memcheck/memcheck_dma-buf.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_GPUMEM": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_gpumem.c",
                    "drivers/xiaomi/memcheck/memcheck_gpumem.h",
                    "include/linux/msm_sysstats.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_SLAB": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_slab.c",
                    "drivers/xiaomi/memcheck/memcheck_slab.h",
                ],
            },
            "CONFIG_MI_MEMCHECK_VMALLOC": {
                 True: [
                    "drivers/xiaomi/memcheck/memcheck_vmalloc.c",
                    "drivers/xiaomi/memcheck/memcheck_vmalloc.h",
                ],
            },
        },
        deps = [
            "drivers/soc/qcom/debug_symbol",
            "kernel/msm_sysstats",
            "drivers/xiaomi/hypsys_netlink/hypsys_netlink",
        ],
    )
