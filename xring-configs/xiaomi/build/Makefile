KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

EXTRA_SYMBOLS += $(OUT_DIR)/../xring-modules/Module.symvers

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M)  KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
