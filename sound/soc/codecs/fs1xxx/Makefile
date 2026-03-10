###
# Copyright (C) Fourier Semiconductor Inc. 2016-2020. All rights reserved.
#

ifneq ($(KERNELRELEASE),)

# TOPLEVEL=$(PWD)/..
# EXTRA_CFLAGS += -I$(TOPLEVEL)

# EXTRA_CFLAGS += -DDEBUG -DFSM_DEBUG
EXTRA_CFLAGS += -Wall -Werror
MODFLAGS = -fno-pic
CFLAGS_MODULE = $(MODFLAGS)
AFLAGS_MODULE = $(MODFLAGS)

snd-soc-fs181x-objs := frsm-i2ca.o
snd-soc-fs181x-objs += frsm-amp-drv.o
snd-soc-fs181x-objs += frsm-i2ca-2in1.o
obj-$(CONFIG_SND_SOC_FS1XXX) += snd-soc-fs181x.o

else # ndef KERNELRELEASE

PWD := $(shell pwd)

MAKEARCH := $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

all:
	@echo CONFIG_SND_SOC_FS1XXX = $(CONFIG_SND_SOC_FS1XXX)
	$(MAKEARCH) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKEARCH) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *symvers

endif

