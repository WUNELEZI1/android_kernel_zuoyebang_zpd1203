/** ***************************************************************************
 * @file nano_sysfs.c
 *
 * @brief Create device sysfs node
 *        Create /sys/class/nanodev/nanodev0/_debuglevel
 *        Create /sys/class/nanodev/nanodev0/_schedule
 *        Create /sys/class/nanodev/nanodev0/_versioncode
 *
 * <em>Copyright (C) 2010, Nanosic, Inc.  All rights reserved.</em>
 * Author : Bin.yuan bin.yuan@nanosic.com
 * */

/*
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/etherdevice.h>
#include "nano_macro.h"

int debuglevel = INFO_LEVEL;
struct attribute_group keyboard_attrs_group;

#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
/** ************************************************************************
 * @func factry_test_show
 *
 * @brief
 ** */
static ssize_t
factory_auto_test_keyboard_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    int read_version_cnt = CONFIG_AUTO_TEST_KEYBOARD_LIMIT;
    int i = 0, received = 0, retval;
    int count = 0;

    dbgprint(INFO_LEVEL, "++");

    if (!i2c_client)
    {
        count += sprintf(buf, "Keyboard Test = < FAIL > (i2c device not found)\n");
        goto _out;
    }

    atomic_set(&i2c_client->vers176x_pkts, 0);

    if (!i2c_client->keyboard_status)
    {
        count += sprintf(buf, "Keyboard Test = < FAIL > (keyboard hall disconnect)\n");
        goto _out;
    }

    i2c_client->factory_auto_test_enable = true;

    Nanosic_PM_try_wakeup(i2c_client);
    /* delay for mcu ready */
    msleep(50);

    while (read_version_cnt-- > 0)
    {
        char read_keypad_state_cmd[I2C_DATA_LENGTH_WRITE]={0x32,0x00,0x4E,0x31,FIELD_HOST,FIELD_176X,0x30,0x01,0x22};

        for (i = 2; i < 9; i++)
            read_keypad_state_cmd[9] += read_keypad_state_cmd[i]; /*sum*/

        rawdata_show("request keypad state",read_keypad_state_cmd,sizeof(read_keypad_state_cmd));
        Nanosic_i2c_write(i2c_client,read_keypad_state_cmd,sizeof(read_keypad_state_cmd));

        retval = wait_event_interruptible_timeout(i2c_client->factory_auto_test_waitq, received != atomic_read(&i2c_client->vers176x_pkts), 20);
        if (signal_pending(current))
        {
            dbgprint(INFO_LEVEL, "stop factory mode\n");
            break;
        }

        if (retval > 0)
            received++;
        else
            dbgprint(ERROR_LEVEL, "no valid response!! retval = %d", retval);
    }

    i2c_client->factory_auto_test_enable = false;

    if (received != CONFIG_AUTO_TEST_KEYBOARD_LIMIT)
    {
        count += sprintf(buf, "Keyboard Test = < FAIL > (received %d)\n", received);
        goto _out;
    }
    else
    {
        count += sprintf(buf, "Keyboard Test = < PASS >\n");
        goto _out;
    }

_out:
    dbgprint(INFO_LEVEL, "%s", buf);

    dbgprint(INFO_LEVEL, "--");

    return count;
}

/** ************************************************************************
 * @func factory_auto_test_mcu_show
 *
 * @brief
 ** */
static ssize_t
factory_auto_test_mcu_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    int read_version_cnt = CONFIG_AUTO_TEST_MCU_LIMIT;
    int i = 0, received = 0, retval;
    int count = 0;

    dbgprint(INFO_LEVEL, "++");

    if (!i2c_client)
    {
        count += sprintf(buf, "MCU Test = < FAIL > (i2c device not found)\n");
        goto _out;
    }

    i2c_client->factory_auto_test_enable = true;

    atomic_set(&i2c_client->vers803x_pkts, 0);

    Nanosic_PM_try_wakeup(i2c_client);
    /* delay for mcu ready */
    msleep(50);

    while (read_version_cnt-- > 0)
    {
        char read_803_vers_cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4F, 0x30, 0x80, FIELD_803X, 0x01, 0x00, 0x00};

        for (i = 2; i < 8; i++)
            read_803_vers_cmd[8] += read_803_vers_cmd[i]; /*sum*/

        rawdata_show("request 803 version", read_803_vers_cmd, sizeof(read_803_vers_cmd));
        Nanosic_i2c_write(i2c_client, read_803_vers_cmd, sizeof(read_803_vers_cmd));

        retval = wait_event_interruptible_timeout(i2c_client->factory_auto_test_waitq, received != atomic_read(&i2c_client->vers803x_pkts), 300);
        if (signal_pending(current))
        {
            dbgprint(INFO_LEVEL, "stop factory mode\n");
            break;
        }

        if (retval > 0)
            received++;
        else
            dbgprint(ERROR_LEVEL, "no valid response!! retval = %d", retval);
    }

    i2c_client->factory_auto_test_enable = false;

    if (received != CONFIG_AUTO_TEST_MCU_LIMIT)
    {
        count += sprintf(buf, "MCU Test = < FAIL > (received %d)\n", received);
        goto _out;
    }
    else
    {
        count += sprintf(buf, "MCU Test = < PASS > %d\n", received);
        goto _out;
    }

_out:
    dbgprint(INFO_LEVEL, "%s", buf);

    dbgprint(INFO_LEVEL, "--");

    return count;
}
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST

/** ************************************************************************
 * @func debuglevel_show
 *
 * @brief null
 ** */
static ssize_t
debuglevel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "debuglevel=%d\n", debuglevel);
}

/** ************************************************************************
 * @func debuglevel_store
 *
 * @brief null
 ** */
static ssize_t
debuglevel_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    unsigned int i;
    unsigned char debuglevel_data[16] = {0x27, 0x00, 0xff, 0xff, 0xff};

    ret = kstrtouint(buf, 10, &i);
    if (ret)
        return 0;

    debuglevel = i;
    debuglevel_data[1] = (unsigned char)i;
    Nanosic_chardev_client_write(debuglevel_data, 16);

    return count;
}

/** ************************************************************************
 * @func schedule_show
 *
 * @brief null
 ** */
static ssize_t
schedule_show(struct device *dev, struct device_attribute *attr, char *buf)
{
#ifdef USE_KWORKER
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (!IS_ERR_OR_NULL(i2c_client) && !IS_ERR_OR_NULL(i2c_client->worker))
        return sprintf(buf, "schedule=%d i2c-read=%d i2c-read-error=%d\n",
                       atomic_read(&i2c_client->worker->schedule_count), atomic_read(&i2c_client->i2c_read_count),
                       atomic_read(&i2c_client->i2c_error_count));
    else
        return sprintf(buf, "schedule error\n");
#else
    return 0;
#endif

}

/** ************************************************************************
 * @func schedule_store
 *
 * @brief 通过echo 5 >  /sys/class/nanodev/nanodev0/_schedule 方式来模拟i2c中断的次数,并执行5次i2c_read
 ** */
static ssize_t
schedule_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#ifdef USE_KWORKER
    int ret;
    unsigned int i;
    unsigned int schedule_count;
    struct nano_i2c_client *i2c_client = gI2c_client;

    ret = kstrtouint(buf, 10, &i);
    if (ret)
        return 0;

    if (IS_ERR_OR_NULL(i2c_client))
        return count;

    schedule_count = i > 10000 ? 10000 : i;

    while (schedule_count)
    {
        Nanosic_workQueue_schedule(i2c_client->worker);
        schedule_count--;
    }

    //    Nanosic_GPIO_set(i?true:false);
#endif // USE_KWORKER


    return count;
}

/** ************************************************************************
 * @func version_code_show
 *
 * @brief null
 ** */
static ssize_t
version_SDK_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "SDK %s\n", DRV_VERSION);
}

/** ************************************************************************
 * @func version_code_store
 *
 * @brief 查看驱动版本号
 ** */
static ssize_t
version_SDK_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/** ************************************************************************
 * @func version_803x_show
 *
 * @brief 查看803x版本
 ** */
static ssize_t
version_803x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client err!\n");
        return -EINVAL;
    }
    return sprintf(buf, "version803x=%s\n", strlen(i2c_client->vers_803x) > 0 ? i2c_client->vers_803x : "null");
}

/** ************************************************************************
 * @func version_803x_store
 *
 * @brief 发送读803x版本命令
 ** */
static ssize_t
version_803x_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (i2c_client)
    {
        int i = 2;
        char read_803_vers_cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4F, 0x30, 0x80, FIELD_803X, 0x01, 0x00, 0x00};
        memset(i2c_client->vers_803x, 0, sizeof(i2c_client->vers_803x));
        for (; i < 8; i++)
            read_803_vers_cmd[8] += read_803_vers_cmd[i]; /*cal sum*/

        rawdata_show("request 803 version", read_803_vers_cmd, sizeof(read_803_vers_cmd));
        Nanosic_i2c_write(i2c_client, read_803_vers_cmd, sizeof(read_803_vers_cmd));
    }

    return count;
}

/** ************************************************************************
 * @func version_176x_show
 *
 * @brief 查看keyboard版本命令
 ** */
static ssize_t
version_176x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    bool keyboard_conneted = 0;
    bool keyboard_power = 0;
    bool keyboard_pogo_pin = 0;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client is NULL\n");
        return -EINVAL;
    }

    keyboard_conneted = (i2c_client->keyboard_status >> 0) & 0x01;
    keyboard_power = (i2c_client->keyboard_status >> 1) & 0x01;
    keyboard_pogo_pin = (i2c_client->keyboard_status >> 6) & 0x01;

    if (keyboard_conneted & keyboard_power)
    {
        sprintf(buf, "Connected=[%d] Power=[%d] POGOPIN=[%s] versionKeyboard=[%s] versionTouchpad=[%s] versionForceIC=[%s] HALL_HOLSTER=[%s]\n",
                keyboard_conneted, keyboard_power, keyboard_pogo_pin ? "ERROR" : "OK", strlen(i2c_client->vers_176x) > 0 ? i2c_client->vers_176x : "null",
                strlen(i2c_client->vers_touchpad) > 0 ? i2c_client->vers_touchpad : "null", strlen(i2c_client->vers_forceic) > 0 ? i2c_client->vers_forceic : "null",
                atomic_read(&i2c_client->holster_standby_hall_check) ? "NEAR" : "FAR");
    }
    else
    {
        sprintf(buf, "Connected=[%d] Power=[%d] HALL_HOLSTER=[%s]\n", keyboard_conneted, keyboard_power, atomic_read(&i2c_client->holster_standby_hall_check)  ? "NEAR" : "FAR");
    }

    return strlen(buf);
}

/** ************************************************************************
 * @func version_176x_store
 *
 * @brief 发送读keyboard版本命令
 ** */
static ssize_t
version_176x_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (i2c_client)
    {
        int i = 0;

        char read_keyboard_status[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4E, 0x31, FIELD_HOST, FIELD_176X, 0xA1, 0x01, 0x01, 0x00};
        char read_keyboard_vers_cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4F, 0x30, FIELD_HOST, FIELD_176X, 0x01, 0x00};

        for (i = 2; i < 9; i++)
            read_keyboard_status[9] += read_keyboard_status[i]; /*cal sum*/

        rawdata_show("request keyboard hall status", read_keyboard_status, sizeof(read_keyboard_status));
        Nanosic_i2c_write(i2c_client, read_keyboard_status, sizeof(read_keyboard_status));

        msleep(10);

        memset(i2c_client->vers_176x, 0, sizeof(i2c_client->vers_176x));
        memset(i2c_client->vers_forceic, 0, sizeof(i2c_client->vers_forceic));
        memset(i2c_client->vers_touchpad, 0, sizeof(i2c_client->vers_touchpad));
        for (i = 2; i < 8; i++)
            read_keyboard_vers_cmd[8] += read_keyboard_vers_cmd[i]; /*cal sum*/

        rawdata_show("request keyboard version", read_keyboard_vers_cmd, sizeof(read_keyboard_vers_cmd));
        Nanosic_i2c_write(i2c_client, read_keyboard_vers_cmd, sizeof(read_keyboard_vers_cmd));
    }

    return count;
}

/** ************************************************************************
 * @func sleep_803x_show
 *
 * @brief 查看803x版本
 ** */
static ssize_t
sleep_803x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "sleep 803x\n");
}

/** ************************************************************************
 * @func sleep_803x_store
 *
 * @brief 发送读803x版本命令
 ** */
static ssize_t
sleep_803x_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    unsigned int i;

    ret = kstrtouint(buf, 16, &i);
    if (ret)
        return 0;

    Nanosic_GPIO_sleep(i > 0 ? true : false);

    return count;
}

/** ************************************************************************
 * @func gpio_set_show
 *
 * @brief gpio set help
 ** */
static ssize_t
gpio_set_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "usage echo pin level > _gpioset\n");
}

/** ************************************************************************
 * @func gpio_set_store
 *
 * @brief 设置gpio pin电压
 ** */
static ssize_t
gpio_set_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int gpio_pin = 0;
    int gpio_value = 0;
    char delim[] = " ";
    char *str = kstrdup(buf, GFP_KERNEL);
    char *token = NULL;
    int ret = 0;

    if (!str)
        return count;

    dbgprint(DEBUG_LEVEL, "gpio set %s\n", str);

    token = strsep(&str, delim);
    if (token != NULL)
        ret = kstrtouint(token, 10, &gpio_pin);

    token = strsep(&str, delim);
    if (token != NULL)
        ret = kstrtouint(token, 10, &gpio_value);

    dbgprint(DEBUG_LEVEL, "gpio set %d %d\n", gpio_pin, gpio_value);

    if (str)
        kfree(str);

    Nanosic_GPIO_set(gpio_pin, gpio_value > 0 ? true : false);

    return count;
}

/** ************************************************************************
 * @func dispatch_keycode_show
 *
 * @brief null
 ** */
static ssize_t
dispatch_keycode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "dispatch_keycode_show\n");
}

/** ************************************************************************
 * @func dispatch_keycode_store
 *
 * @brief write keycode to input system for test
 ** */
static ssize_t
dispatch_keycode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    unsigned int i;
    unsigned char down[12] = {0x57, 0x00, 0x39, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char up[12] = {0x57, 0x00, 0x39, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ret = kstrtouint(buf, 16, &i);
    if (ret)
        return 0;

    down[6] = i;

    Nanosic_i2c_parse(down, sizeof(down));
    Nanosic_i2c_parse(up, sizeof(up));

    return count;
}

/** ************************************************************************
 * @func reset_store
 *
 * @brief echo 1 > /sys/class/nanodev/nanodev0/_reset8030 to reset 8030
 ** */
static ssize_t
reset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int i;
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client is NULL !");
        return -EINVAL;
    }

    if (sscanf(buf, "%d", &i) < 0)
    {
        dbgprint(DEBUG_LEVEL, "sscanf err, buf is %s\n", buf);
        return -EINVAL;
    }

    if (i == 1)
    {
        Nanosic_GPIO_sleep(true);
        dbgprint(ERROR_LEVEL, "reset mcu and reload firmware...");
        Nanosic_PM_free(i2c_client);
        Nanosic_GPIO_reset();
        Nanosic_firmware_init(i2c_client);
        // delay 500ms for mcu ready
        mdelay(500);
        Nanosic_PM_init(i2c_client);
    }
    else
    {
        dbgprint(DEBUG_LEVEL, "inval cmd %d\n", i);
    }

    return count;
}

/*设置调试级别*/
static DEVICE_ATTR(_debuglevel, 0600, debuglevel_show, debuglevel_store);

/*统计workqueue运行次数*/
static DEVICE_ATTR(_schedule, 0600, schedule_show, schedule_store);

/*查看sdk版本号*/
static DEVICE_ATTR(_versionSDK, 0600, version_SDK_show, version_SDK_store);

/*键盘测试*/
static DEVICE_ATTR(_keycode, 0600, dispatch_keycode_show, dispatch_keycode_store);

/*查看803版本号*/
static DEVICE_ATTR(_version803x, 0600, version_803x_show, version_803x_store);

/*查看keyboard版本号*/
static DEVICE_ATTR(_version176x, 0600, version_176x_show, version_176x_store);

/*控制803睡眠*/
static DEVICE_ATTR(_sleep803x, 0600, sleep_803x_show, sleep_803x_store);

/*gpio set method*/
static DEVICE_ATTR(_gpioset, 0600, gpio_set_show, gpio_set_store);

/*reset 8030*/
static DEVICE_ATTR(_reset8030, 0200, NULL, reset_store);

#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
static DEVICE_ATTR(_factory_auto_test_keyboard, 0600, factory_auto_test_keyboard_show, NULL);
static DEVICE_ATTR(_factory_auto_test_mcu, 0600, factory_auto_test_mcu_show, NULL);
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST

static struct attribute *keyboard_attrs[] = {
    &dev_attr__debuglevel.attr,
    &dev_attr__schedule.attr,
    &dev_attr__versionSDK.attr,
    &dev_attr__keycode.attr,
    &dev_attr__version803x.attr,
    &dev_attr__version176x.attr,
    &dev_attr__sleep803x.attr,
    &dev_attr__gpioset.attr,
    &dev_attr__reset8030.attr,
    &dev_attr__factory_auto_test_keyboard.attr,
    &dev_attr__factory_auto_test_mcu.attr,
    NULL,
};

/** ************************************************************************
 * @func Nanosic_Sysfs_create
 *
 * @brief create sysfs node for nanosic i2c-hid driver
 */
void Nanosic_sysfs_create(struct device *dev)
{
    int ret = 0;

    keyboard_attrs_group.attrs = keyboard_attrs;

    ret = sysfs_create_group(&dev->kobj, &keyboard_attrs_group);

    if (ret)
        dbgprint(ERROR_LEVEL, "sysfs_create_group err, ret = %d", ret);
}

/** ************************************************************************
 * @func Nanosic_Sysfs_release
 *
 * @brief
 */
void Nanosic_sysfs_release(struct device *dev)
{
    sysfs_remove_group(&dev->kobj, &keyboard_attrs_group);
}
