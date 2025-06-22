# SPDX-License-Identifier: GPL-2.0 only.

# Copyright (c) 2023 MediaTek Inc.

# for debug Link error
ifeq ($(CONFIG_MTK_PCIE_LINK_ERR_DEBUG_SUPPORT), y)
ccflags-y += -DCONFIG_MTK_PCIE_LINK_ERR_DEBUG_SUPPORT
endif
# for test
ifeq ($(CONFIG_MTK_THINMD_SLT_ENABLE), y)
ccflags-y += -DCONFIG_MTK_THINMD_SLT_ENABLE
endif

CUR_DIR := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(CUR_DIR)))

ifneq ($(KERNEL_SRC),)
##################################################################################################1
# find device and interface folder
PWRCTL_DEV_FOLDER :=$(shell find $(CUR_DIR)/../device/ -maxdepth 1 -mindepth 1 -type d)
PWRCTL_INTERFACE_FOLDER :=$(shell find $(CUR_DIR)/../interface/ -maxdepth 1 -mindepth 1 -type d)
PWRCTL_DEV_FOLDER := $(foreach folderd,$(PWRCTL_DEV_FOLDER),$(notdir $(folderd)))
PWRCTL_INTERFACE_FOLDER := $(foreach folderi,$(PWRCTL_INTERFACE_FOLDER),$(notdir $(folderi)))

$(info "the KERNEL_SRC=$(KERNEL_SRC)")
$(info "the DEVICE_MODULES_REL_DIR=$(DEVICE_MODULES_REL_DIR)")
ccflags-y += -I$(KERNEL_SRC)/$(DEVICE_MODULES_REL_DIR)/drivers/pci/controller/
ccflags-y += -I$(CUR_DIR)/../../gpio_sap_ctrl/
ccflags-y += -I$(srctree)/include/
ccflags-y += -I$(CUR_DIR)/
ccflags-y += -I$(KERNEL_SRC)/../xring-modules/include/

ccflags-y += -mcmodel=large

#module
MODULE_NAME := wwan_gpio_pwrctl
obj-m += ${MODULE_NAME}.o

${MODULE_NAME}-objs = mtk_pwrctl_common.o

# interface
#MTK_INTERFACE_OBJS := $(notdir $(wildcard $(CUR_DIR)/../interface/$(PWRCTL_INTERFACE_FOLDER)/*.c))
#MTK_INTERFACE_OBJS := $(addprefix ../interface/$(PWRCTL_INTERFACE_FOLDER)/,$(MTK_INTERFACE_OBJS:.c=.o))
#${MODULE_NAME}-objs += $(MTK_INTERFACE_OBJS)
#ccflags-y += -I$(CUR_DIR)/../interface/$(PWRCTL_INTERFACE_FOLDER)/
MTK_INTERFACE_OBJS := $(notdir $(wildcard $(CUR_DIR)/../interface/*.c))
MTK_INTERFACE_OBJS := $(addprefix ../interface/,$(MTK_INTERFACE_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_INTERFACE_OBJS)
ccflags-y += -I$(CUR_DIR)/../interface/


#device
#MTK_DEV_OBJS := $(notdir $(wildcard $(CUR_DIR)/../device/$(PWRCTL_DEV_FOLDER)/*.c))
#MTK_DEV_OBJS := $(addprefix ../device/$(PWRCTL_DEV_FOLDER)/,$(MTK_DEV_OBJS:.c=.o))
#${MODULE_NAME}-objs += $(MTK_DEV_OBJS)
#ccflags-y += -I$(CUR_DIR)/../device/$(PWRCTL_DEV_FOLDER)/
MTK_DEV_OBJS := $(notdir $(wildcard $(CUR_DIR)/../device/*.c))
MTK_DEV_OBJS := $(addprefix ../device/,$(MTK_DEV_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_DEV_OBJS)
ccflags-y += -I$(CUR_DIR)/../device/


#test
$(info "the CONFIG_MTK_THINMD_SLT_ENABLE=$(CONFIG_MTK_THINMD_SLT_ENABLE)")
$(info "the CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER=$(CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER)")
ifeq ($(CONFIG_MTK_THINMD_SLT_ENABLE), y)
ifneq ($(CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER),)
MTK_TEST_OBJS := $(notdir $(wildcard $(CUR_DIR)/../$(CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER)/*.c))
MTK_TEST_OBJS := $(addprefix ../$(CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER)/,$(MTK_TEST_OBJS:.c=.o))
$(info "MTK_TEST_OBJS=$(MTK_TEST_OBJS)")
${MODULE_NAME}-objs += $(MTK_TEST_OBJS)
ccflags-y += -I$(CUR_DIR)/../$(CONFIG_MTK_THINMD_SLT_PWRCTL_FOLDER)/
endif
endif

$(info OUT_DIR=$(OUT_DIR))
$(info KERNEL_SRC=$(KERNEL_SRC))
ifneq ($(wildcard $(KERNEL_SRC)/$(DEVICE_MODULES_REL_DIR)/Makefile.include),)
include $(wildcard $(KERNEL_SRC)/$(DEVICE_MODULES_REL_DIR)/Makefile.include)
endif

export AUTOCONF_H=$(0)include/generated/autoconf.h
ifneq ($(wildcard $(OUT_DIR)/../vendor/mediatek/kernel_modules/gpio_sap_ctrl/Module.symvers), )
EXTRA_SYMBOLS +=  $(OUT_DIR)/../vendor/mediatek/kernel_modules/gpio_sap_ctrl/Module.symvers
endif

# add for sensorhub state
EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/Module.symvers

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS) KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)"

modules_install:
	$(MAKE) M=$(M) -C $(KERNEL_SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean

else
##################################################################################################2
$(info "-----------CONFIG_TARGET_SUBTARGET = $(CONFIG_TARGET_SUBTARGET)")
$(info "the INTERFACE_FOLDER=$(PWRCTL_INTERFACE_FOLDER)")
$(info "the PWRCTL_DEV_FOLDER=$(PWRCTL_DEV_FOLDER)")

ccflags-y += -I$(CUR_DIR)/

#module
MODULE_NAME := wwan_gpio_pwrctl
obj-m += ${MODULE_NAME}.o

#common
${MODULE_NAME}-objs = mtk_pwrctl_common.o

#interface
#MTK_INTERFACE_OBJS := $(notdir $(wildcard $(CUR_DIR)/../interface/$(PWRCTL_INTERFACE_FOLDER)/*.c))
#MTK_INTERFACE_OBJS := $(addprefix ../interface/$(PWRCTL_INTERFACE_FOLDER)/,$(MTK_INTERFACE_OBJS:.c=.o))
#${MODULE_NAME}-objs += $(MTK_INTERFACE_OBJS)
#ccflags-y += -I$(CUR_DIR)/../interface/$(PWRCTL_INTERFACE_FOLDER)/
MTK_INTERFACE_OBJS := $(notdir $(wildcard $(CUR_DIR)/../interface/*.c))
MTK_INTERFACE_OBJS := $(addprefix ../interface/,$(MTK_INTERFACE_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_INTERFACE_OBJS)
ccflags-y += -I$(CUR_DIR)/../interface/

#device
#MTK_DEV_OBJS := $(notdir $(wildcard $(CUR_DIR)/../device/$(PWRCTL_DEV_FOLDER)/*.c))
#MTK_DEV_OBJS := $(addprefix ../device/$(PWRCTL_DEV_FOLDER)/,$(MTK_DEV_OBJS:.c=.o))
#${MODULE_NAME}-objs += $(MTK_DEV_OBJS)
#ccflags-y += -I$(CUR_DIR)/../device/$(PWRCTL_DEV_FOLDER)/
MTK_DEV_OBJS := $(notdir $(wildcard $(CUR_DIR)/../device/*.c))
MTK_DEV_OBJS := $(addprefix ../device/,$(MTK_DEV_OBJS:.c=.o))
${MODULE_NAME}-objs += $(MTK_DEV_OBJS)
ccflags-y += -I$(CUR_DIR)/../device/


#test
ifeq ($(CONFIG_MTK_THINMD_SLT_ENABLE), y)
ifneq ($(PWRCTL_TEST_FOLDER),)
MTK_TEST_OBJS := $(notdir $(wildcard $(CUR_DIR)/../$(PWRCTL_TEST_FOLDER)/*.c))
MTK_TEST_OBJS := $(addprefix ../$(PWRCTL_TEST_FOLDER)/,$(MTK_TEST_OBJS:.c=.o))
$(info "MTK_TEST_OBJS=$(MTK_TEST_OBJS)")
${MODULE_NAME}-objs += $(MTK_TEST_OBJS)
ccflags-y += -I$(CUR_DIR)/../$(PWRCTL_TEST_FOLDER)/
endif
endif

endif #KERNEL_SRC
