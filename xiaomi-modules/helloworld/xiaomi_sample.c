#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int xiaomi_init(void) {
	printk(KERN_ALERT "hello, Xiaomi!\n");
	return 0;
}
static void xiaomi_exit(void) {
	printk(KERN_ALERT "good bye, Xiaomi\n");
}

module_init(xiaomi_init);
module_exit(xiaomi_exit);

MODULE_LICENSE("Dual BSD/GPL");
