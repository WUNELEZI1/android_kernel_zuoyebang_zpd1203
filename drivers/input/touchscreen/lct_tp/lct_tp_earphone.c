#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

#define PROC_NODE_NAME "tp_earphone_mode"
#define MAX_BUF_SIZE   32

struct tp_earphone {
    bool current_mode;
    struct proc_dir_entry *proc_entry;
    int (*set_mode_cb)(bool mode);
};

static struct tp_earphone *lct_tp;

/*
 * Proc文件操作
 *********************************************************************************/
static ssize_t lct_tp_earphone_mode_read(struct file *file, char __user *buf,
                        size_t size, loff_t *ppos)
{
    char tmp_buf[MAX_BUF_SIZE];
    int len;

    if (*ppos > 0)
        return 0;

    len = scnprintf(tmp_buf, sizeof(tmp_buf), "%s\n",
                   lct_tp->current_mode ? "true" : "false");

    if (copy_to_user(buf, tmp_buf, len)) {
        pr_err("Failed to copy data to user\n");
        return -EFAULT;
    }

    *ppos = len;
    return len;
}

static ssize_t lct_tp_earphone_mode_write(struct file *file, const char __user *buf,
                         size_t size, loff_t *ppos)
{
    char tmp_buf[MAX_BUF_SIZE];
    bool new_mode;
    int ret;

    if (size >= sizeof(tmp_buf))
        return -EINVAL;

    if (copy_from_user(tmp_buf, buf, size)) {
        pr_err("Failed to copy data from user\n");
        return -EFAULT;
    }

    tmp_buf[size] = '\0';

    // 支持多种输入格式
    if (!strncmp(tmp_buf, "1", 1) ||
        !strcasecmp(tmp_buf, "true")) {
        new_mode = true;
    } else if (!strncmp(tmp_buf, "0", 1) ||
             !strcasecmp(tmp_buf, "false")) {
        new_mode = false;
    } else {
        pr_err("Invalid input: %s\n", tmp_buf);
        return -EINVAL;
    }

    if ((ret = lct_tp->set_mode_cb(new_mode))) {
        pr_err("Set mode failed: %d\n", ret);
        return ret;
    }

    lct_tp->current_mode = new_mode;
    return size;
}

static const struct proc_ops mode_fops = {
    .proc_read = lct_tp_earphone_mode_read,
    .proc_write = lct_tp_earphone_mode_write,
};


int tp_earphone_init(int (*set_cb)(bool))
{
    if (!set_cb)
        return -EINVAL;

    lct_tp = kzalloc(sizeof(*lct_tp), GFP_KERNEL);
    if (!lct_tp)
        return -ENOMEM;

    lct_tp->set_mode_cb = set_cb;

    lct_tp->proc_entry = proc_create(PROC_NODE_NAME, 0666,
                                        NULL, &mode_fops);
    if (!lct_tp->proc_entry) {
        kfree(lct_tp);
        pr_err("Failed to create proc entry\n");
        return -ENOMEM;
    }

    pr_info("Earphone mode driver initialized\n");
    return 0;
}
EXPORT_SYMBOL(tp_earphone_init);

void tp_earphone_uninit(void)
{
    if (lct_tp) {
        if (lct_tp->proc_entry) {
            remove_proc_entry(PROC_NODE_NAME, NULL);
            lct_tp->proc_entry = NULL;
        }
        kfree(lct_tp);
    }
    pr_info("Earphone mode driver unloaded\n");
}
EXPORT_SYMBOL(tp_earphone_uninit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
