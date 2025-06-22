# SPDX-License-Identifier: GPL-2.0-only
ifeq ($(KERNEL_SRC),)
CONFIG_MTK_TX00=m
CONFIG_MTK_T700?=n
CONFIG_MTK_T800?=y
CONFIG_MTK_T3X?=y
CONFIG_MTK_TX00_TRACE=y
CONFIG_MTK_TX00_DEBUGFS=y
CONFIG_SUPPORT_WWAN=y
CONFIG_MTK_REG_ACCESS_OPTIMIZE=y
KERN_VER=5.15.0-27-generic
# CONFIG_MTK_WWAN_PWRCTL_SUPPORT: n, gpio power control driver for ARM platform
else
CONFIG_MTK_TX00=m
CONFIG_MTK_T800=y
CONFIG_SUPPORT_WWAN=n
CONFIG_MTK_REG_ACCESS_OPTIMIZE=y
CONFIG_MTK_DATA_CPU_LOADING_OPTIMIZE=y
CONFIG_MTK_ENABLE_WQ_GRO=y
CONFIG_MTK_WWAN_PWRCTL_SUPPORT=y
endif

ifeq ($(DEVICE_MODULES_PATH),)
DEVICE_MODULES_PATH = $(srctree)
endif

CUR_DIR := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(CUR_DIR)))

ifneq ($(KERNELRELEASE),)

ifeq ($(CONFIG_MTK_NEXT), y)
MODULE_NAME := mtk_pcie_wwan_next
else ifeq ($(OPENWRT_LINUX_BUILD),)
MODULE_NAME := mtk_pcie_wwan
else
MODULE_NAME := $(MODULE_NAME)
endif

ifeq ($(CONFIG_MTK_REG_ACCESS_OPTIMIZE), y)
ccflags-y += -DCONFIG_REG_ACCESS_OPTIMIZE
endif

ifeq ($(CONFIG_MTK_DATA_CPU_LOADING_OPTIMIZE), y)
ccflags-y += -DCONFIG_DATA_CPU_LOADING_OPTIMIZE
endif

ifeq ($(CONFIG_MTK_ENABLE_WQ_GRO), y)
ccflags-y += -DCONFIG_MTK_ENABLE_WQ_GRO
endif

ifeq ($(CONFIG_MTK_DATA_TEST), y)
ccflags-y += -DCONFIG_DATA_TEST_MODE
endif

obj-$(CONFIG_MTK_TX00) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += mtk_pci.o mtk_dev.o mtk_debug.o mtk_memlog.o mtk_trace.o mtk_fsm.o mtk_ethtool.o mtk_bm.o mtk_dpmaif.o mtk_cldma.o mtk_ctrl_plane.o mtk_except.o mtk_pm.o mtk_port.o mtk_port_io.o

ifeq ($(CONFIG_MTK_AUDIODSP_SUPPORT),)
${MODULE_NAME}-objs += mtk_pcimsg.o
else
${MODULE_NAME}-objs += mtk_pcimsg_adsp.o
ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/adsp/include
ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/audio_ipi/include
endif

ifeq ($(CONFIG_SUPPORT_WWAN), n)
${MODULE_NAME}-objs += mtk_wwan_legacy.o
else
${MODULE_NAME}-objs += mtk_wwan.o
ccflags-y += -DCONFIG_SUPPORT_WWAN
ifeq ($(CONFIG_SUPPORT_RELAYFS), y)
ccflags-y += -DCONFIG_SUPPORT_RELAYFS
endif
endif

ifeq ($(CONFIG_MTK_DEVLINK), y)
${MODULE_NAME}-objs += mtk_devlink.o
ccflags-y += -DCONFIG_MTK_DEVLINK
ifeq ($(CONFIG_MTK_DEVLINK_COREDUMP_SUPPORT), y)
ccflags-y += -DCONFIG_MTK_DEVLINK_COREDUMP_SUPPORT
endif
ifeq ($(CONFIG_MTK_DEVLINK_LOGDUMP_SUPPORT), y)
ccflags-y += -DCONFIG_MTK_DEVLINK_LOGDUMP_SUPPORT
endif
ifeq ($(CONFIG_MTK_DEVLINK_FLASH_SUPPORT), y)
ccflags-y += -DCONFIG_MTK_DEVLINK_FLASH_SUPPORT
endif
endif

${MODULE_NAME}-$(CONFIG_DEBUG_FS) += mtk_debugfs.o

ifeq ($(CONFIG_MTK_T700), y)
MTK_T700_OBJS := $(notdir $(wildcard $(CUR_DIR)/../t700/*.c))
MTK_T700_OBJS := $(addprefix ../t700/,$(MTK_T700_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_T700_OBJS)
label_product += t700_0x4d75
ccflags-y += -I$(CUR_DIR)/../t700/
endif

ifeq ($(CONFIG_MTK_T800), y)
MTK_T800_OBJS := $(notdir $(wildcard $(CUR_DIR)/../t800/*.c))
MTK_T800_OBJS := $(addprefix ../t800/,$(MTK_T800_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_T800_OBJS)
label_product += t800_0x0800
ccflags-y += -I$(CUR_DIR)/../t800/
endif

ifeq ($(CONFIG_MTK_T3X), y)
MTK_T3X_OBJS := $(notdir $(wildcard $(CUR_DIR)/../t3x/*.c))
MTK_T3X_OBJS := $(addprefix ../t3x/,$(MTK_T3X_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_T3X_OBJS)
label_product += t3x_0x0300
ccflags-y += -I$(CUR_DIR)/../t3x/
endif

$(src)/mtk_cldma.o: $(src)/mtk_cldma_hw_interface.h
$(src)/mtk_dpmaif.o: $(src)/mtk_dpmaif_drv_interface.h
$(src)/mtk_pci.o: $(src)/mtk_pci_dev_cfg.h
$(src)/mtk_cldma_hw_interface.h $(src)/mtk_dpmaif_drv_interface.h $(src)/mtk_pci_dev_cfg.h: $(src)/mtk_auto_gen_hw_interface.pl
	$< $@ "$(label_product)"
clean-files := mtk_cldma_hw_interface.h mtk_dpmaif_drv_interface.h mtk_pci_dev_cfg.h

ifeq ($(CONFIG_IGNORE_BUILD_WARNING),)
ccflags-y += -Werror
endif

ccflags-y += -I$(src)/
ccflags-y += -I$(CUR_DIR)/
CFLAGS_mtk_trace.o = -I$(src)

ifneq ($(CONFIG_MTK_WWAN_PWRCTL_SUPPORT),)
ccflags-y += -DCONFIG_MTK_WWAN_PWRCTL_SUPPORT
ccflags-y += -I$(CUR_DIR)/../../pwrctl/device/
ccflags-y += -I$(CUR_DIR)/../../gpio_pwrctl/
ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/pci/controller
endif

else
ifeq ($(KERNEL_SRC),)
COMPILER = "gcc"
LINKER = "ld"
TARGET_ARCH := x86_64

# supports alternative linux kernel version
ifneq ($(KERNEL),)
KDIR=$(KERNEL)
else
ifeq ($(KERN_VER),)
KDIR=/lib/modules/$(shell uname -r)/build
else
KDIR=/lib/modules/$(KERN_VER)/build
endif
endif

MODULE_STR_TIME = $(shell grep 'MODULE_INFO.BUILD_TIME' $(CUR_DIR)/mtk_pci.c)
MODULE_STR_LABEL = $(shell grep 'MODULE_INFO.BUILD_LABEL' $(CUR_DIR)/mtk_pci.c)
# Depends on how the label is defined
LABEL ?= $(shell cat $(CUR_DIR)/label.ini)
OUT ?= $(CUR_DIR)/../out

define clean_ops
	@rm -rf $(CUR_DIR)/*.o
	@rm -rf $(CUR_DIR)/.*.o.cmd
	@rm -rf $(CUR_DIR)/../t700/*.o
	@rm -rf $(CUR_DIR)/../t700/.*.o.cmd
	@rm -rf $(CUR_DIR)/../t800/*.o
	@rm -rf $(CUR_DIR)/../t800/.*.o.cmd
	@rm -rf $(CUR_DIR)/../t3x/*.o
	@rm -rf $(CUR_DIR)/../t3x/.*.o.cmd
	@rm -rf $(OUT)
endef

all:
	@mkdir -p $(OUT)/scripts/
	$(if $(MODULE_STR_LABEL), sed -i 's/^MODULE_INFO.BUILD_LABEL.*$$/MODULE_INFO(BUILD_LABEL, "$(LABEL)");/' $(CUR_DIR)/mtk_pci.c, sed -i '$$i MODULE_INFO(BUILD_LABEL, "$(LABEL)");' $(CUR_DIR)/mtk_pci.c)
	$(if $(MODULE_STR_TIME), sed -i 's/^MODULE_INFO.BUILD_TIME.*$$/MODULE_INFO(BUILD_TIME, "$(shell date +%Y-%m-%d,%H:%M:%S)");/' $(CUR_DIR)/mtk_pci.c,sed -i '$$i MODULE_INFO(BUILD_TIME, "$(shell date +%Y-%m-%d,%H:%M:%S)");' $(CUR_DIR)/mtk_pci.c)
	make -C $(KDIR) M=$(CUR_DIR) modules CC=$(COMPILER) LD=$(LINKER) ARCH=$(TARGET_ARCH)  2>&1 | tee $(OUT)/build_driver.log
	md5sum $(CUR_DIR)/*.ko >> $(OUT)/build_driver.log
	mv -f $(CUR_DIR)/*.ko $(OUT)/
	cp -rf $(CUR_DIR)/../scripts/*.sh $(OUT)/scripts/
	cp -rf $(CUR_DIR)/../scripts/sample/ $(OUT)/scripts/
	@echo $(DATE)$(shell date +%Y-%m-%d,%H:%M:%S) >> $(OUT)/build_driver.log
#	@echo $(LABEL)$(shell ./script/get_label.sh) >> $(OUT)/build_driver.log

.PHONY clean:
	@echo "clean tmp file"
	-make -C $(KDIR) M=$(CUR_DIR) clean 2>&1
	sed -i '/BUILD_LABEL/, /BUILD_TIME/d' $(CUR_DIR)/mtk_pci.c
	sed -i '/BUILD_LABEL/, /BUILD_LABEL/d' $(CUR_DIR)/mtk_pci.c
	$(clean_ops)
else

ifneq ($(wildcard $(abspath $(O)/../kernel/kernel_device_modules-6.1/Module.symvers)), )
extra_symbols += $(abspath $(O)/../kernel/kernel_device_modules-6.1/Module.symvers)
endif

ifneq ($(wildcard $(abspath $(O)/../vendor/mediatek/kernel_modules/wwan/pwrctl/common/Module.symvers)), )
extra_symbols += $(abspath $(O)/../vendor/mediatek/kernel_modules/wwan/pwrctl/common/Module.symvers)
else
ifneq ($(wildcard $(abspath $(O)/../vendor/mediatek/kernel_modules/wwan/gpio_pwrctl/Module.symvers)), )
extra_symbols += $(abspath $(O)/../vendor/mediatek/kernel_modules/wwan/gpio_pwrctl/Module.symvers)
endif
endif

all: PRIVATE_SYMBOLS := $(extra_symbols)
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS) KBUILD_EXTRA_SYMBOLS="$(PRIVATE_SYMBOLS)"

modules_install:
	$(MAKE) M=$(M) -C $(KERNEL_SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean

endif #KERNEL_SRC
endif #KERNEL_RELEASE
