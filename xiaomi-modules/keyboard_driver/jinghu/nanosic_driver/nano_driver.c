/** ***************************************************************************
 * @file nano_driver.c
 *
 * @brief provided interface of initialza and release nanosic driver .
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
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "nano_macro.h"
// #include <linux/regulator/consumer.h>
// #include <drm/drm_notifier_mi.h>
#include <drm/drm_panel.h>
#ifdef KEYBOARD_PLATFORM_XRING
#include <soc/xring/display/panel_event_notifier.h>
#else
#include <linux/soc/qcom/panel_event_notifier.h>
#endif // KEYBOARD_PLATFORM_XRING
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#define CREATE_TRACE_POINTS
#include "nanosic_trace.h"

extern int of_get_named_gpio(const struct device_node *np, const char *propname, int index);

static struct xiaomi_keyboard_data *mdata;
static bool initial = false;
struct nano_i2c_client *gI2c_client = NULL;

static struct drm_panel *active_panel = NULL;
keyboard_hall_notifier_handler notifier_handler= NULL;
struct device * sensor_dev = NULL;
static struct device_node *panel_node = NULL;


/* for pogo charge start */
int Nanosic_get_hall_status(void)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    hall_state ret = HALL_AWAY ;
    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client err");
        return ret;
    }
    if(atomic_read(&i2c_client->holster_standby_hall_check))
        ret = HALL_HOLSTER;
    else if( atomic_read(&i2c_client->keyboard_connected_check))
        ret = HALL_KEYBOARD;
    return ret;
}
EXPORT_SYMBOL_GPL(Nanosic_get_hall_status);

bool keyboard_hall_notifier_register(struct device *dev,keyboard_hall_notifier_handler handler)
{
    struct nano_i2c_client *i2c_client = gI2c_client;

    if(dev==NULL||handler==NULL)
    {
        dbgprint(ERROR_LEVEL, "Input parameter error");
        return false;
    }
    if(!i2c_client)
    {
        dbgprint(ERROR_LEVEL, "i2c_client err,save temporary!");
        notifier_handler = handler;
        sensor_dev = dev;
        return false;
    }
    if(i2c_client->sensor_dev||i2c_client->notifier_handler)
    {
        dbgprint(ERROR_LEVEL, "handler is not empty");
        return false;
    }
    i2c_client->notifier_handler = handler;
    i2c_client->sensor_dev = dev;
    dbgprint(INFO_LEVEL, "keyboard_hall_notifier_handler register success");
    return true;
};
EXPORT_SYMBOL_GPL(keyboard_hall_notifier_register);

int Nanosic_charge_i2c_write_callback(void *buf, size_t len)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    int ret = 0;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client err");
        ret = -ENODEV;
        goto _out;
    }

    rawdata_show("_write_pogo_charge_cmd_", buf, len);
    ret = Nanosic_i2c_write(i2c_client, buf, len);

_out:
    return ret;
}

/* for pogo charge end */


#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
void factory_auto_test_wakeup(struct nano_i2c_client *i2c_client, atomic_t *condition)
{
    if (!i2c_client->factory_auto_test_enable)
        return;

    atomic_inc(condition);
    wake_up(&i2c_client->factory_auto_test_waitq);
}
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST

static int send_screen_state(screen_state state)
{
    int ret = 0;
    int i = 0;
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (i2c_client)
    {
        char cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4E, 0x31, 0x80, FIELD_176X, 0x25, 0x01};
        cmd[8] = (char)state;
        for (i = 2; i < 9; i++)
        {
            cmd[9] += cmd[i]; /*cal sum*/
        }
        rawdata_show("write screen state cmd", cmd, sizeof(cmd));
        ret = Nanosic_i2c_write(i2c_client, cmd, sizeof(cmd));
    }
    return ret;
}

static int keyboard_check_panel(struct device_node *np)
{
    int i;
    int count;
    struct device_node *node;
    struct drm_panel *panel;

    count = of_count_phandle_with_args(np, "dsi-panel", NULL);
    dbgprint(ALERT_LEVEL, "panel count=%d\n", count);
    if (count <= 0)
        return -ENODEV;

    for (i = 0; i < count; i++)
    {
        node = of_parse_phandle(np, "dsi-panel", i);
        panel = of_drm_find_panel(node);
        panel_node = node;
        if (!IS_ERR(panel))
        {
            active_panel = panel;
            if (active_panel)
            {
                dbgprint(ALERT_LEVEL, "check panel ok\n");
            }
            return 0;
        }
        else
        {
            of_node_put(node);
            active_panel = NULL;
        }
    }
    return PTR_ERR(panel);
}

#ifdef KEYBOARD_PLATFORM_XRING
static void keyboard_drm_notifier_callback(enum xring_panel_event_tag tag,
		        struct xring_panel_event_notification *notification, void *client_data)
{
    struct xiaomi_keyboard_data *mclient_data = (struct xiaomi_keyboard_data *)client_data;

    if (!notification)
    {
        dbgprint(ERROR_LEVEL, "Invalid notification\n");
        return;
    }

    switch (notification->type)
    {
    case DRM_PANEL_EVENT_UNBLANK:
        /* NOTE: uncomment after display adapted
            if (notification->data.early_trigger)
            {
                queue_work(mclient_data->event_wq, &mclient_data->early_resume_work);
                dbgprint(ALERT_LEVEL, "early_trigger keyboard resume\n");
            }
            else
            {
                queue_work(mclient_data->event_wq, &mclient_data->resume_work);
                dbgprint(ALERT_LEVEL, "keyboard resume\n");
            }
        */
        dbgprint(ALERT_LEVEL, "keyboard resume\n");
        queue_work(mclient_data->event_wq, &mclient_data->early_resume_work);
        queue_work(mclient_data->event_wq, &mclient_data->resume_work);
        break;
    case DRM_PANEL_EVENT_BLANK:
        /* NOTE: uncomment after display adapted
            if (notification->data.early_trigger)
            {
                queue_work(mclient_data->event_wq, &mclient_data->early_suspend_work);
                dbgprint(ALERT_LEVEL, "early_trigger keyboard suspend\n");
            }
            else
            {
                queue_work(mclient_data->event_wq, &mclient_data->suspend_work);
                dbgprint(ALERT_LEVEL, "keyboard suspend\n");
            }
        */
        dbgprint(ALERT_LEVEL, "keyboard suspend\n");
        queue_work(mclient_data->event_wq, &mclient_data->early_suspend_work);
        queue_work(mclient_data->event_wq, &mclient_data->suspend_work);
        break;
    default:
        dbgprint(DEBUG_LEVEL, "notif_type=%d\n", notification->type);
        break;
    }
}
#else
static void keyboard_drm_notifier_callback(enum panel_event_notifier_tag notifier_tag,
                                           struct panel_event_notification *notification, void *client_data)
{
    struct xiaomi_keyboard_data *mclient_data = (struct xiaomi_keyboard_data *)client_data;

    if (!notification)
    {
        dbgprint(ERROR_LEVEL, "Invalid notification\n");
        return;
    }

    switch (notification->notif_type)
    {
    case DRM_PANEL_EVENT_UNBLANK:
        if (notification->notif_data.early_trigger)
        {
            queue_work(mclient_data->event_wq, &mclient_data->early_resume_work);
            dbgprint(ALERT_LEVEL, "early_trigger keyboard resume\n");
        }
        else
        {
            queue_work(mclient_data->event_wq, &mclient_data->resume_work);
            dbgprint(ALERT_LEVEL, "keyboard resume\n");
        }
        break;
    case DRM_PANEL_EVENT_BLANK:
    case DRM_PANEL_EVENT_BLANK_LP:
        if (notification->notif_data.early_trigger)
        {
            queue_work(mclient_data->event_wq, &mclient_data->early_suspend_work);
            dbgprint(ALERT_LEVEL, "early_trigger keyboard suspend\n");
        }
        else
        {
            queue_work(mclient_data->event_wq, &mclient_data->suspend_work);
            dbgprint(ALERT_LEVEL, "keyboard suspend\n");
        }
        break;
    default:
        dbgprint(DEBUG_LEVEL, "notif_type=%d\n", notification->notif_type);
        break;
    }
}
#endif // KEYBOARD_PLATFORM_XRING

static int Nanosic_803_firmware_version_read(struct nano_i2c_client *i2c_client)
{
    int i = 2;
    int ret = 0;
    char read_803_vers_cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4F, 0x30, 0x80, FIELD_803X, 0x01, 0x00, 0x00};

    memset(i2c_client->vers_803x, 0, sizeof(i2c_client->vers_803x));
    for (; i < 8; i++)
        read_803_vers_cmd[8] += read_803_vers_cmd[i]; /*cal sum*/

    rawdata_show("request 803 version", read_803_vers_cmd, sizeof(read_803_vers_cmd));
    ret = Nanosic_i2c_write(i2c_client, read_803_vers_cmd, sizeof(read_803_vers_cmd));

    return ret;
}

static int nanosic_803_probe(struct i2c_client *client)
{
    int ret = 0;
    struct device_node *of_node;
    int irq_pin = -1;
    u32 irq_flags = 0;
    int reset_pin = -1;
    int status_pin = -1;
    int sleep_pin = -1;
    int i2c_ls_en_pin = -1;
    struct nano_i2c_client *I2client = kzalloc(sizeof(struct nano_i2c_client), GFP_KERNEL);

    if (!I2client)
    {
        dbgprint(ERROR_LEVEL, "Could not alloc memory\n");
        ret = -ENOMEM;
        goto _err_kzalloc;
    }

    if (IS_ERR_OR_NULL(client))
    {
        dbgprint(ERROR_LEVEL, "nanosic_803_probe client IS_ERR_OR_NULL\n");
        ret = -ENODEV;
        goto _err_no_client;
    }

    dbgprint(ALERT_LEVEL, "probe adapter nr %d, addr 0x%x\n", client->adapter->nr, client->addr);

    of_node = client->dev.of_node;
    if (!of_node)
    {
        dbgprint(ERROR_LEVEL, "nanosic_803_probe of_node == 0\n");
        ret = -ENODEV;
        goto _err_no_of_node;
    }

    ret = keyboard_check_panel(of_node);
    if (ret == -EPROBE_DEFER)
    {
        dbgprint(INFO_LEVEL, "nanosic_803_probe, panel not ready, wait retry!!!\n");
        goto _err_check_panel;
    }
    // if (ret)
    // {
    //     dbgprint(ERROR_LEVEL, "nanosic_803_probe ret=%d\n", ret);
    //     ret = -ENODEV;
    //     goto _err_check_panel;
    // }

    /* regulator init */
    ret = Nanosic_regulator_init(I2client, of_node, client);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Failed to init nanosic regulator: %d\n", ret);
        goto _err_init_regulator;
    }

    /* regulator enable */
    ret = Nanosic_switch_regulator(I2client, true);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Failed to enable nanosic regulator: %d\n", ret);
        goto _err_switch_regulator;
    }


    ret = of_property_read_u32(of_node, "irq_flags", &irq_flags);
    if (ret < 0) {
        dbgprint(ERROR_LEVEL, "nanosic_803_probe, irq_flags read error, ret=%d\n !!!", ret);
    }
    irq_pin = of_get_named_gpio(of_node, "irq_pin", 0);
    dbgprint(ALERT_LEVEL, "irq_pin=%d,irq_flags=%d\n", irq_pin, irq_flags);
    reset_pin = of_get_named_gpio(of_node, "reset_pin", 0);
    dbgprint(ALERT_LEVEL, "reset_pin=%d\n", reset_pin);
    status_pin = of_get_named_gpio(of_node, "status_pin", 0);
    dbgprint(ALERT_LEVEL, "status_pin=%d\n", status_pin);
    sleep_pin = of_get_named_gpio(of_node, "sleep_pin", 0);
    dbgprint(ALERT_LEVEL, "sleep_pin=%d\n", sleep_pin);
    i2c_ls_en_pin = of_get_named_gpio(of_node, "i2c_ls_en_pin", 0);
    dbgprint(ALERT_LEVEL, "i2c_ls_en_pin=%d\n", i2c_ls_en_pin);

    ret = Nanosic_GPIO_register(reset_pin, status_pin, irq_pin, sleep_pin, i2c_ls_en_pin);
    if (ret < 0)
        dbgprint(ERROR_LEVEL, "GPIO register ERROR!\n\n");

#ifdef HAVE_TOUCHPAD
    ret = xiaomi_touchpad_edge_filter_param_init(of_node);
    if (ret)
        dbgprint(ERROR_LEVEL, "xiaomi_touchpad_edge_filter_param_init fail!");
#endif // HAVE_TOUCHPAD

    /*initialize chardev module*/
    ret = Nanosic_chardev_register();
    if (ret < 0)
        goto _err_Nanosic_chardev_register;

    /*initialize input module*/
    ret = Nanosic_input_register();
    if (ret < 0)
        goto _err_Nanosic_input_register;

#if 0
    ret = Nanosic_i2c_detect(client,NULL);
    if(ret < 0){
        dbgprint(ERROR_LEVEL,"I2C communication ERROR!\n\n");
        return -1;
    }
#endif

    mutex_init(&I2client->pm_mutex);
    Nanosic_PM_init(I2client);

    /* keyboard_status init before irq init */
    I2client->keyboard_status = 0;
    memset(I2client->vers_803x, 0, sizeof(I2client->vers_803x));
    memset(I2client->vers_176x, 0, sizeof(I2client->vers_176x));
    memset(I2client->vers_forceic, 0, sizeof(I2client->vers_forceic));
    memset(I2client->vers_touchpad, 0, sizeof(I2client->vers_touchpad));
    atomic_set(&I2client->holster_standby_hall_check, 0);
    atomic_set(&I2client->keyboard_connected_check, 0);
    if(I2client->sensor_dev && I2client->notifier_handler)
        I2client->notifier_handler(I2client->sensor_dev,0);

#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
    atomic_set(&I2client->vers803x_pkts, 0);
    atomic_set(&I2client->vers176x_pkts, 0);
    I2client->factory_auto_test_enable = false;
    init_waitqueue_head(&I2client->factory_auto_test_waitq);
#endif

    /*initialize i2c module*/
    ret = Nanosic_i2c_register(I2client, irq_pin, irq_flags, client->adapter->nr, client->addr);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Nanosic_i2c_register ERROR!\n");
        goto _err_Nanosic_i2c_register;
    }

    i2c_set_clientdata(client, I2client);
    I2client->dev = &(client->dev);

    /*i2c device can wakeup system*/
    device_init_wakeup(I2client->dev, 1);
    xiaomi_keyboard_init(I2client);

    ret = Nanosic_cache_init();
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "Nanosic cache init ERROR!\n");
        goto _err_Nanosic_cache_init;
    }
#ifdef ENABLE_POGO_CHARGE
    ret = Nanosic_charge_i2c_write_callback_register(Nanosic_charge_i2c_write_callback);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "Nanosic_charge_i2c_write_callback_register ERROR!\n");
        goto _err_Nanosic_charge_i2c_write_callback_register;
    }
#endif
    if(sensor_dev != NULL && notifier_handler != NULL)
    {
        I2client->sensor_dev = sensor_dev;
        I2client->notifier_handler = notifier_handler;
        dbgprint(INFO_LEVEL, "temporary notifier handler register success!\n");
    }

    gI2c_client = I2client;

    initial = true;

    /* test if mcu alive */
    Nanosic_803_firmware_version_read(I2client);

    dbgprint(ALERT_LEVEL, "probe nanosic driver\n");

    return 0;
#ifdef  ENABLE_POGO_CHARGE
    Nanosic_charge_i2c_write_callback_unregister();
_err_Nanosic_charge_i2c_write_callback_register:
#endif
    Nanosic_cache_release();
_err_Nanosic_cache_init:
    device_init_wakeup(gI2c_client->dev, 0);
    Nanosic_i2c_release(gI2c_client);
_err_Nanosic_i2c_register:
    Nanosic_PM_free(I2client);
    Nanosic_input_release();
_err_Nanosic_input_register:
    Nanosic_chardev_release();
_err_Nanosic_chardev_register:
    Nanosic_switch_regulator(I2client, false);
_err_switch_regulator:
_err_init_regulator:
_err_check_panel:
_err_no_of_node:
_err_no_client:
    kfree(I2client);
_err_kzalloc:

    return ret;
}

static void nanosic_803_remove(struct i2c_client *client)
{
    struct nano_i2c_client *I2client = NULL;

    dbgprint(ALERT_LEVEL, "remove\n");
    if (initial == false)
        return;

    I2client = i2c_get_clientdata(client);
#ifdef  ENABLE_POGO_CHARGE
    Nanosic_charge_i2c_write_callback_unregister();
#endif
    Nanosic_PM_free(I2client);

    /*release chardev module*/
    Nanosic_chardev_release();

    /*release input module*/
    Nanosic_input_release();

    /*release i2c module*/
    Nanosic_i2c_release(I2client);

    /*release timer module*/
    // Nanosic_timer_release();//apply timer module instead of interrupt

    Nanosic_GPIO_release();

    Nanosic_cache_release();

    device_init_wakeup(I2client->dev, 0);

    Nanosic_switch_regulator(I2client, false);

    initial = false;

    kfree(I2client);

    dbgprint(ALERT_LEVEL, "remove nanosic driver\n");

    return;
}

static const struct of_device_id nanosic_803_of_match[] = {
    {
        .compatible = "nanosic,803",
    },
    {/* sentinel */}};
MODULE_DEVICE_TABLE(of, nanosic_803_of_match);

static const struct i2c_device_id nanosic_803_i2c_id[] = {
    {"nanosic,803", 0},
    {}};
MODULE_DEVICE_TABLE(i2c, nanosic_803_i2c_id);

static struct i2c_driver nanosic_803_driver = {
    .probe = nanosic_803_probe,
    .remove = nanosic_803_remove,
    .driver = {
        .name = "nanosic,803",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(nanosic_803_of_match),
        .pm = &xiaomi_keyboard_pm_ops,
    },
    .id_table = nanosic_803_i2c_id,
    .detect = Nanosic_i2c_detect,
};

static __init int nanosic_driver_init(void)
{
    int ret;

    ret = i2c_add_driver(&nanosic_803_driver);
    dbgprint(ALERT_LEVEL, "nanosic_driver_init, ret = %d\n", ret);

    return ret;
}

static __exit void nanosic_driver_exit(void)
{
    i2c_del_driver(&nanosic_803_driver);
    dbgprint(ALERT_LEVEL, "nanosic_driver_exit");
}

static int xiaomi_keyboard_init(struct nano_i2c_client *i2c_client)
{
    // struct xiaomi_keyboard_platdata *pdata;
    int ret = 0;
    dbgprint(ALERT_LEVEL, "enter\n");
    mdata = kzalloc(sizeof(struct xiaomi_keyboard_data), GFP_KERNEL);
    i2c_client->mdata = mdata;
    mdata->dev_pm_suspend = false;
    init_completion(&mdata->dev_pm_suspend_completion);
    mdata->irq = i2c_client->irqno;
    dbgprint(ALERT_LEVEL, "irq:%d", mdata->irq);
    mdata->event_wq = create_singlethread_workqueue("kb-event-queue");
    if (!mdata->event_wq)
    {
        dbgprint(ALERT_LEVEL, "Can not create work thread for suspend/resume!!");
        // ret = -ENOMEM;
    }

    INIT_WORK(&mdata->early_resume_work, keyboard_early_resume_work);
    INIT_WORK(&mdata->resume_work, keyboard_resume_work);
    INIT_WORK(&mdata->early_suspend_work, keyboard_early_suspend_work);
    INIT_WORK(&mdata->suspend_work, keyboard_suspend_work);

    if (active_panel)
    {
#ifdef KEYBOARD_PLATFORM_XRING
        mdata->notifier_cookie = xring_panel_event_notifier_register(XRING_PANEL_EVENT_TAG_PRIMARY,
                                                                XRING_PANEL_EVENT_NOTIFIER_CLIENT_KEYBOARD, panel_node,
                                                                keyboard_drm_notifier_callback, (void *)mdata);
        of_node_put(panel_node);
#else
        mdata->notifier_cookie = panel_event_notifier_register(PANEL_EVENT_NOTIFICATION_PRIMARY,
                                                               PANEL_EVENT_NOTIFIER_CLIENT_KEYBOARD, active_panel,
                                                               keyboard_drm_notifier_callback, (void *)mdata);
#endif // KEYBOARD_PLATFORM_XRING
        if (!mdata->notifier_cookie)
        {
            dbgprint(ALERT_LEVEL, "register drm_notifier failed. cookie=%s\n", (char *)mdata->notifier_cookie);
            goto err_register_drm_notif_failed;
        }
    }
    dbgprint(ALERT_LEVEL, "success. \n");
    return ret;

err_register_drm_notif_failed:
    if (mdata->event_wq)
    {
        destroy_workqueue(mdata->event_wq);
    }
    return ret;
}

static void keyboard_early_resume_work(struct work_struct *work)
{
    int ret = 0;
    struct nano_i2c_client *i2c_client = gI2c_client;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client err!\n");
        return;
    }

    dbgprint(ALERT_LEVEL, "enter\n");
    ret = Nanosic_cache_put();
    if (ret < 0)
        dbgprint(ERROR_LEVEL, "Nanosic_cache_put err:%d\n", ret);
    g_panel_status = true;
    i2c_client->ic_state = NANOSIC_IC_RESUME_IN;
    Nanosic_GPIO_sleep(true);
#ifdef USE_KWORKER
    Nanosic_workQueue_schedule(i2c_client->worker);
#else
    Nanosic_i2c_read_handler(i2c_client);
#endif // USE_KWORKER
}

static void keyboard_resume_work(struct work_struct *work)
{
    // send display state to keyborad
    struct nano_i2c_client *I2client = gI2c_client;
    int ret = 0;

    if (IS_ERR_OR_NULL(I2client))
    {
        dbgprint(ERROR_LEVEL, "I2client err!\n");
        return;
    }

    ret = send_screen_state(SCREEN_ON);
    dbgprint(ALERT_LEVEL, "send screen on:%d\n", ret);
    I2client->ic_state = NANOSIC_IC_RESUME_OUT;
}

static void keyboard_early_suspend_work(struct work_struct *work)
{
    // send display state to keyborad
    int ret = 0;
    struct nano_i2c_client *I2client = gI2c_client;

    if (IS_ERR_OR_NULL(I2client))
    {
        dbgprint(ERROR_LEVEL, "I2client err!\n");
        return;
    }

    gI2c_client->ic_state = NANOSIC_IC_SUSPEND_IN;
    ret = send_screen_state(SCREEN_OFF);
    dbgprint(ALERT_LEVEL, "send screen off:%d\n", ret);
    // In Xring Platform, there is no time delay between SCREEN_OFF and Nanosic_GPIO_sleep(false);
    // So keyboard backlight will be turned on after Screen Off beascuse AP does not have 
    // enought time to receive and handle keyboard response IIC data packet.
    // msleep ensures that there have engouth time for AP to receive keyboard response IIC data packet;
    // msleep can avoid Nanosic_GPIO_sleep(false) to be affected by the response packet of the keyboard
    // and thus avoid the phenomenon that the keyboard will be awaked after Nanosic_GPIO_sleep(false)
    msleep(150);
}

static void keyboard_suspend_work(struct work_struct *work)
{
    struct nano_i2c_client *I2client = gI2c_client;

    if (IS_ERR_OR_NULL(I2client))
    {
        dbgprint(ERROR_LEVEL, "I2client err!\n");
        return;
    }
    dbgprint(ALERT_LEVEL, "enter\n");
    g_panel_status = false;
    gI2c_client->ic_state = NANOSIC_IC_SUSPEND_OUT;
    Nanosic_GPIO_sleep(false);
    // CTS requires that there should be no active wakelock
    // within 12 seconds after the screen is suspended, so
    // the wakelock is released in advance to prevent the
    // wakelock from being released after 20 seconds due
    // to the timer not being reset after sleep.
    pm_relax(I2client->dev);
}

static int xiaomi_keyboard_pm_suspend(struct device *dev)
{
    int ret = 0;
    dbgprint(ALERT_LEVEL, "enter, enable_irq_wake\n");
    ret = enable_irq_wake(mdata->irq); // i2c_client->irqno
    if (ret < 0)
        dbgprint(ALERT_LEVEL, "enter, enable_irq_wake irq failed\n");
    ret = enable_irq_wake(g_wakeup_irqno);
    if (ret < 0)
        dbgprint(ALERT_LEVEL, "enter, enable_irq_wake wakeup_irqno failed\n");

    mdata->dev_pm_suspend = true;
    reinit_completion(&mdata->dev_pm_suspend_completion);

    return ret;
}

static int xiaomi_keyboard_pm_resume(struct device *dev)
{
    int ret = 0;
    dbgprint(ALERT_LEVEL, "disable_irq_wake\n");
    ret = disable_irq_wake(mdata->irq); // i2c_client->irqno
    if (ret < 0)
        dbgprint(ALERT_LEVEL, "disable_irq_wake irq failed\n");
    ret = disable_irq_wake(g_wakeup_irqno);
    if (ret < 0)
        dbgprint(ALERT_LEVEL, "disable_irq_wake wakeup_irqno failed\n");

    mdata->dev_pm_suspend = false;
    complete(&mdata->dev_pm_suspend_completion);

    return ret;
}

static const struct dev_pm_ops xiaomi_keyboard_pm_ops = {
    .suspend = xiaomi_keyboard_pm_suspend,
    .resume = xiaomi_keyboard_pm_resume,
};

static int xiaomi_keyboard_remove(void)
{
    dbgprint(ALERT_LEVEL, "enter\n");
    if (active_panel && mdata->notifier_cookie)
    {
#ifdef KEYBOARD_PLATFORM_XRING
        xring_panel_event_notifier_unregister(mdata->notifier_cookie);
#else
        panel_event_notifier_unregister(mdata->notifier_cookie);
#endif // KEYBOARD_PLATFORM_XRING
    }
    destroy_workqueue(mdata->event_wq);
    if (mdata)
    {
        kfree(mdata);
        mdata = NULL;
    }
    return 0;
}

late_initcall(nanosic_driver_init);
module_exit(nanosic_driver_exit);
MODULE_LICENSE("GPL");
