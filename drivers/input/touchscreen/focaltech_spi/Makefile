# Makefile for the focaltech touchscreen drivers.


ccflags-$(CONFIG_DRM_MEDIATEK_V2) += -I$(srctree)/drivers/gpu/drm/mediatek/mediatek_v2/
ccflags-$(CONFIG_DRM_MEDIATEK_V2) += -I$(srctree)/drivers/misc/mediatek/scp/include/

obj-$(CONFIG_TOUCHSCREEN_FTS) += focaltech_spi.o
focaltech_spi-y += focaltech_core.o
focaltech_spi-y += focaltech_ex_fun.o
focaltech_spi-y += focaltech_ex_mode.o
focaltech_spi-y += focaltech_gesture.o
focaltech_spi-y += focaltech_esdcheck.o
focaltech_spi-y += focaltech_point_report_check.o
focaltech_spi-y += focaltech_proximity.o
focaltech_spi-y += focaltech_flash_spi.o
focaltech_spi-y += focaltech_spi_v2.o
focaltech_spi-y += focaltech_debug.o
focaltech_spi-y += focaltech_hp.o
focaltech_spi-y += focaltech_test/focaltech_test.o
focaltech_spi-y += focaltech_test/focaltech_test_ini.o
focaltech_spi-y += focaltech_test/supported_ic/focaltech_test_ft8057m.o