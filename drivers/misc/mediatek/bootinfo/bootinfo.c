#include <linux/module.h>
#include <asm/setup.h>
#include <linux/bootconfig.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/sysfs.h>

static int __init bootinfo_init(void)
{
  return 0;
}

static void __exit bootinfo_exit(void)
{
}

module_init(bootinfo_init);
module_exit(bootinfo_exit);
MODULE_AUTHOR("Gui Xu <xugui1@longcheer.com>");
MODULE_DESCRIPTION("Xiaomi Boot Info support");
MODULE_LICENSE("GPL");
