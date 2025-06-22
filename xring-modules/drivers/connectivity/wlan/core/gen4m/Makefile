
EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/drivers/connectivity/conninfra/Module.symvers
EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/drivers/connectivity/wlan/adaptor/build/connac3x/Module.symvers
EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/drivers/connectivity/wlan/adaptor/wlan_page_pool/Module.symvers
EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/drivers/connectivity/connfem/Module.symvers
EXTRA_SYMBOLS += ../xring-modules/Module.symvers

SEGMENT := SP

MODULE_PWD=../xring-modules/drivers/connectivity/wlan/core/gen4m
$(info gen4m depends on following symbols:[${EXTRA_SYMBOLS}])

modules modules_install clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(MODULE_PWD) $(KBUILD_OPTIONS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" MODULE_NAME=$(_MODULE_NAME) SEGMENT=$(SEGMENT) $(@)
	mkdir -p $(O)/$(M)
	cp -f $(O)/$(MODULE_PWD)/Module.symvers $(O)/$(M)/Module.symvers
	cp -f $(O)/$(MODULE_PWD)/*.ko $(O)/$(M)
