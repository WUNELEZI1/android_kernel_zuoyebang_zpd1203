#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int mca_init(void) {
        printk(KERN_ALERT "hello, Xiaomi!\n");
        return 0;
}
static void mca_exit(void) {
        printk(KERN_ALERT "good bye, Xiaomi\n");
}

module_init(mca_init);
module_exit(mca_exit);

MODULE_LICENSE("mca Dual BSD/GPL");
