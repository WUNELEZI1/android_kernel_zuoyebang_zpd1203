#include "xiaomi_keyboard_macro.h"
#include "xiaomi_touchpad_sysfs.h"

extern inline struct xiaomi_touchpad_pdata *get_touchpad_pdata(void);

static ssize_t edge_filter_param_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);
    struct touchpad_edge_filter_param *param = &pdata->edge_filter_param;
    char str[128] = {0};
    ssize_t count = 0;
    int i = 0;

    mutex_lock(&pdata->edge_filter_mutex);

    if (buf != NULL)
    {
        for (i = 0; i < param_list_string_count; i++)
        {
            memset(str, 0 , sizeof(str));
            count += snprintf(str, sizeof(str), "%s: %d\n", param_list[i], *((u32 *)param + i));
            strcat(buf, str); 
        }

        for (i = 0; i < param_list_string_count; i++)
        {
            memset(str, 0 , sizeof(str));
            count += snprintf(str, sizeof(str), "%d ", *((u32 *)param + i));
            strcat(buf, str);
        }
        memset(str, 0 , sizeof(str));
        count += snprintf(str, sizeof(str), "\n");
        strcat(buf, str);
    }
    else
    {
        MI_KB_ERR("buf is NULL\n");
    }

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static ssize_t edge_filter_param_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);
    struct touchpad_edge_filter_param *param = &pdata->edge_filter_param;
    const char *ptr = buf;
    u32 values[100];
    int values_index = 0;
    int i = 0;

    /* pick params */
    MI_KB_LOG("inject params %s", buf);
    while (sscanf(ptr, "%u", &values[values_index]) == 1)
    {
        values_index++;

        while (*ptr != ' ' && *ptr != '\0')
            ptr++;

        while (*ptr == ' ')
            ptr++;

        if (*ptr == '\0')
            break;
    }

    mutex_lock(&pdata->edge_filter_mutex);

    for (i = 0; i < param_list_string_count; i++)
        *((u32 *)param + i) = values[i];

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static ssize_t edge_filter_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);


    mutex_lock(&pdata->edge_filter_mutex);

    if (sysfs_streq(buf, "1"))
        pdata->edge_filter_enable = 1;
    else if(sysfs_streq(buf, "0"))
        pdata->edge_filter_enable = 0;

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static ssize_t edge_filter_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);
    int count = 0;

    mutex_lock(&pdata->edge_filter_mutex);

    count += sprintf(buf, "%s\n", pdata->edge_filter_enable ? "enable" : "disable");

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static ssize_t cdgain_robot_test_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);


    mutex_lock(&pdata->edge_filter_mutex);

    if (sysfs_streq(buf, "1"))
    {
        pdata->cdgain_robot_test_mode = 1;
        MI_KB_LOG("cdgain_robot_test_mode = %d", pdata->cdgain_robot_test_mode);
    }
    else if(sysfs_streq(buf, "0"))
    {
        pdata->cdgain_robot_test_mode = 0;
        MI_KB_LOG("cdgain_robot_test_mode = %d", pdata->cdgain_robot_test_mode);
    }

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static ssize_t cdgain_robot_test_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct xiaomi_touchpad_pdata *pdata = dev_get_drvdata(dev);
    int count = 0;

    mutex_lock(&pdata->edge_filter_mutex);

    count += sprintf(buf, "%s\n", pdata->cdgain_robot_test_mode ? "enable" : "disable");
    MI_KB_LOG("cdgain_robot_test_mode = %d", pdata->cdgain_robot_test_mode);

    mutex_unlock(&pdata->edge_filter_mutex);

    return count;
}

static DEVICE_ATTR(edge_filter_param, (S_IRUGO | S_IWUSR | S_IWGRP),
                   edge_filter_param_show, edge_filter_param_store);

static DEVICE_ATTR(edge_filter_enable, (S_IRUGO | S_IWUSR | S_IWGRP),
                   edge_filter_enable_show, edge_filter_enable_store);

static DEVICE_ATTR(cdgain_robot_test_mode, (S_IRUGO | S_IWUSR | S_IWGRP),
                   cdgain_robot_test_mode_show, cdgain_robot_test_mode_store);


static struct attribute *touchpad_attr_group[] = {
    &dev_attr_edge_filter_param.attr,
    &dev_attr_edge_filter_enable.attr,
    &dev_attr_cdgain_robot_test_mode.attr,
    NULL,
};

int xiaomi_touchpad_sysfs_init(struct xiaomi_touchpad_pdata *pdata)
{
    int ret;

    pdata->attrs.attrs = touchpad_attr_group;
    ret = sysfs_create_group(&pdata->dev->kobj, &pdata->attrs);
    if (ret)
    {
        MI_KB_ERR("Cannot create sysfs structure! ret = %d\n", ret);
        ret = -ENODEV;
    }

    MI_KB_LOG("xiaomi_touchpad_sysfs_init success!");

    return ret;
}

void xiaomi_touchpad_sysfs_deinit(struct xiaomi_touchpad_pdata *pdata)
{
    sysfs_remove_group(&pdata->dev->kobj, &pdata->attrs);

    MI_KB_LOG("xiaomi_touchpad_sysfs_deinit success!");
}
