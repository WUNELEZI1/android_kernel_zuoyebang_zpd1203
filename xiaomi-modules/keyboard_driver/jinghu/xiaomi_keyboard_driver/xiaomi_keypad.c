#include "xiaomi_keyboard_macro.h"
#include "xiaomi_keypad.h"
#include "xiaomi_keypad_sysfs.h"
#include "xiaomi_keyboard.h"

extern inline struct xiaomi_keyboard_pdata *get_keyboard_pdata(void);
extern inline struct xiaomi_touchpad_pdata *get_touchpad_pdata(void);
extern inline struct xiaomi_keypad_pdata *get_keypad_pdata(void);


void xiaomi_keypad_notify(int keyboard_state, short keyboard_firmware_version){
    struct xiaomi_keypad_pdata* pdata = get_keypad_pdata();
    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("get_keypad_pdata fail!\n");
        return ;
    }
    pdata->info_node.keyboard_state = keyboard_state;
    pdata->info_node.keyboard_firmware_version = keyboard_firmware_version;
    sysfs_notify(&pdata->dev->kobj, NULL, "keyboard_status_param");
}
EXPORT_SYMBOL_GPL(xiaomi_keypad_notify);


int xiaomi_keypad_init(struct class *class, dev_t dev_num, struct xiaomi_keypad_pdata *pdata)
{
    int ret = 0;

    pdata->class = class;
    pdata->dev_num = dev_num;
    pdata->dev = device_create(pdata->class, NULL, dev_num, pdata, "keypad");
    pdata->info_node.keyboard_state = 0;
    pdata->info_node.keyboard_firmware_version = 0;
    if (IS_ERR_OR_NULL(pdata->dev))
    {
        ret = -ENODEV;
        goto err_keypad_create;
    }

    ret = xiaomi_keypad_sysfs_init(pdata);
    if (ret)
    {
        MI_KB_ERR("xiaomi_keypad_sysfs_init fail! ret = %d", ret);
        goto err_xiaomi_keypad_sysfs_init;
    }

    MI_KB_LOG("xiaomi_keypad_init success!");

    return 0;

    xiaomi_keypad_sysfs_deinit(pdata);
err_xiaomi_keypad_sysfs_init:
    device_destroy(pdata->class, pdata->dev_num);
err_keypad_create:
    return ret;
}

void xiaomi_keypad_deinit(struct xiaomi_keypad_pdata *pdata)
{
    xiaomi_keypad_sysfs_deinit(pdata);
    device_destroy(pdata->class, pdata->dev_num);
    MI_KB_LOG("xiaomi_keypad_deinit success!");
}

