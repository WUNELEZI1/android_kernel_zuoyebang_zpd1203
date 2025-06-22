#include "xiaomi_keyboard_macro.h"
#include "xiaomi_keyboard.h"

static struct xiaomi_keyboard_pdata *keyboard_pdata = NULL;
int keyboard_major;
const char *name = "xiaomi_keyboard";
struct file_operations keyboard_fops = {
    .owner = THIS_MODULE,
};

inline struct xiaomi_keyboard_pdata *get_keyboard_pdata(void)
{
    return keyboard_pdata;
}

inline struct xiaomi_touchpad_pdata *get_touchpad_pdata(void)
{
    if (IS_ERR_OR_NULL(keyboard_pdata))
    {
        return NULL;
    }
    else
    {
        return &keyboard_pdata->touchpad_pdata;
    }
}

inline struct xiaomi_keypad_pdata *get_keypad_pdata(void)
{
    if (IS_ERR_OR_NULL(keyboard_pdata))
    {
        return NULL;
    }
    else
    {
        return &keyboard_pdata->keypad_pdata;
    }
}

static int __init xiaomi_keyboard_init(void)
{
    struct xiaomi_keyboard_pdata *pdata;
    int ret = 0;

    MI_KB_LOG("xiaomi_keyboard_probe!");

    pdata = kzalloc(sizeof(struct xiaomi_keyboard_pdata), GFP_KERNEL);
    if (!pdata)
    {
        ret = -ENOMEM;
        goto err_kzalloc;
    }

    pdata->class = class_create("keyboard");
    if (IS_ERR_OR_NULL(pdata->class))
    {
        ret = -ENODEV;
        goto err_class_create;
    }

    /* get dev_t */
    keyboard_major = register_chrdev(0, name, &keyboard_fops);
    if (keyboard_major < 0)
    {
        ret = keyboard_major;
        goto err_major_create;
    }

    ret = xiaomi_keypad_init(pdata->class, MKDEV(keyboard_major, 0), &pdata->keypad_pdata);
    if (ret)
    {
        MI_KB_ERR("xiaomi_keypad_init fail! ret = %d", ret);
        goto err_xiaomi_keypad_init;
    }

    ret = xiaomi_touchpad_init(pdata->class, MKDEV(keyboard_major, 1), &pdata->touchpad_pdata);
    if (ret)
    {
        MI_KB_ERR("xiaomi_touchpad_init fail! ret = %d", ret);
        goto err_xiaomi_touchpad_init;
    }

    keyboard_pdata = pdata;

    xiaomi_keyboard_evdev_init();


    return 0;

    xiaomi_touchpad_deinit(&pdata->touchpad_pdata);
err_xiaomi_touchpad_init:
    xiaomi_keypad_deinit(&pdata->keypad_pdata);
err_xiaomi_keypad_init:
    unregister_chrdev(keyboard_major, name);
err_major_create:
    class_destroy(pdata->class);
err_class_create:
    kfree(pdata);
err_kzalloc:
    return ret;
}

static void __exit xiaomi_keyboard_exit(void)
{
    struct xiaomi_keyboard_pdata *pdata = keyboard_pdata;

    MI_KB_LOG("xiaomi_keyboard_remove!");

    if (IS_ERR_OR_NULL(keyboard_pdata))
    {
        return;
    }


    xiaomi_keyboard_evdev_exit();
    xiaomi_touchpad_deinit(&pdata->touchpad_pdata);
    xiaomi_keypad_deinit(&pdata->keypad_pdata);
    unregister_chrdev(keyboard_major, name);
    class_destroy(pdata->class);
}

subsys_initcall(xiaomi_keyboard_init);
module_exit(xiaomi_keyboard_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangcheng8@xiaomi.com");
