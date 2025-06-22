KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
obj-$(CONFIG_XRING_TIMER) := apb_timer.o
apb_timer-objs := dw_apb_timer.o dw_apb_timer_of.o

KBUILD_OPTIONS += $(KBUILD_EXTRA) # Extra config if any

ccflags-y += -I$(KERNEL_SRC)/kernel/time

# we get the symbols from modules using KBUILD_EXTRA_SYMBOLS to prevent warnings about unknown functions
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M)  KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
