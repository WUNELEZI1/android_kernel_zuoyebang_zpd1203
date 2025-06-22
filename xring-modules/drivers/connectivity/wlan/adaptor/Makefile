extra_symbols += ../xring-modules/drivers/connectivity/conninfra/Module.symvers

$(info [wlan] WLAN_BUILD_COMMON=$(WLAN_BUILD_COMMON))

all:
ifdef WLAN_BUILD_COMMON
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS) KBUILD_EXTRA_SYMBOLS="$(extra_symbols)"
else

	$(MAKE) -C $(KERNEL_SRC) M=$(M)/build/connac3x modules $(KBUILD_OPTIONS) KBUILD_EXTRA_SYMBOLS="$(extra_symbols)" CONNAC_VER=3_0 MODULE_NAME=wmt_chrdev_wifi_connac3
endif

modules_install:
ifdef WLAN_BUILD_COMMON
	$(MAKE) M=$(M) -C $(KERNEL_SRC) modules_install
else
	$(MAKE) M=$(M)/build/connac3x -C $(KERNEL_SRC) modules_install
endif

clean:
ifdef WLAN_BUILD_COMMON
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
else
	$(MAKE) -C $(KERNEL_SRC) M=$(M)/build/connac3x clean
endif

ifneq ($(MODULE_PWD), $(M))
        mkdir -p $(O)/$(M)
        cp -f $(O)/$(MODULE_PWD)/Module.symvers $(O)/$(M)/Module.symvers
        cp -f $(O)/$(MODULE_PWD)/*.ko $(O)/$(M)
endif

