#include <linux/module.h>
#include <asm/setup.h>
#include <linux/bootconfig.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/of_device.h>

static int __init board_id_init(void)
{
  return 0;
}

static void __exit board_id_exit(void)
{
}

module_init(board_id_init);
module_exit(board_id_exit);
MODULE_AUTHOR("Xinan Luo <luoxinan@longcheer.com>");
MODULE_DESCRIPTION("Xiaomi Board ID support");
MODULE_LICENSE("GPL");
