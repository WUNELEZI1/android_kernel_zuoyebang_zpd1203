EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/Module.symvers

MODULE_PWD=../xring-modules/drivers/connectivity/connadp

modules modules_install clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(MODULE_PWD) $(KBUILD_OPTIONS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" $(@)

ifneq ($(MODULE_PWD), $(M))
        mkdir -p $(O)/$(M)
        cp -f $(O)/$(MODULE_PWD)/Module.symvers $(O)/$(M)/Module.symvers
        cp -f $(O)/$(MODULE_PWD)/*.ko $(O)/$(M)
endif
