# If you want to compile into modular drive, will need to decide
# according to the following configuration to compile.
# If the CONFIG_XRING_VIDEO_VPU is not defined, vpu module isn't compiled.

ifeq ($(CONFIG_XRING_VIDEO_VPU),y)

obj-y += xring_vpu_memmap/
obj-y += vdec/
obj-y += venc/
obj-y += v4l2_driver/

endif
