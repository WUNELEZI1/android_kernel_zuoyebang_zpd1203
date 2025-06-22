#include "xiaomi_keyboard_macro.h"
#include "xiaomi_keypad_sysfs.h"
static ssize_t keyboard_status_param_show(struct device *dev, struct device_attribute *attr, char *buf){
    struct xiaomi_keypad_pdata *pdata = dev_get_drvdata(dev);
    int ret = 0;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("get_keypad_pdata fail!\n");
        return -1;
    }
    memcpy(buf,&pdata->info_node,sizeof(pdata->info_node));
    ret = sizeof(pdata->info_node);
    return ret;
}

static DEVICE_ATTR(keyboard_status_param,(S_IRUGO | S_IWUSR | S_IWGRP),
                   keyboard_status_param_show, NULL);

static struct attribute *keypad_attr_group[] = {
    &dev_attr_keyboard_status_param.attr,
    NULL,
};

int xiaomi_keypad_sysfs_init(struct xiaomi_keypad_pdata *pdata)
{
    int ret;

    pdata->attrs.attrs = keypad_attr_group;
    ret = sysfs_create_group(&pdata->dev->kobj, &pdata->attrs);
    if (ret)
    {
        MI_KB_ERR("Cannot create sysfs structure! ret = %d\n", ret);
        ret = -ENODEV;
    }

    MI_KB_LOG("xiaomi_keypad_sysfs_init success!");

    return ret;
}

void xiaomi_keypad_sysfs_deinit(struct xiaomi_keypad_pdata *pdata)
{
    sysfs_remove_group(&pdata->dev->kobj, &pdata->attrs);

    MI_KB_LOG("xiaomi_keypad_sysfs_deinit success!");
}


