/** ***************************************************************************
 * @file nano_timer.c
 *
 * @brief nanosic timer file
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

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "nano_macro.h"
#include "nanosic_trace.h"

// time out set to 20s
#define PM_TIMER_TIMEOUT_PANEL_ON  20000
#define PM_TIMER_TIMEOUT_PANEL_OFF 1000

/** ************************************************************************
 *  @func Nanosic_PM_Sleep
 *
 *  @brief use to control 803 go to sleep
 ** */
void Nanosic_PM_sleep(void)
{
    /*output low level of sleep pin, make 803 go to sleep*/
    Nanosic_GPIO_sleep(false);
}

/** ************************************************************************
 *  @func Nanosic_PM_try_wakeup
 *
 *  @brief try to control 803 wakeup from lower power mode
 ** */
bool Nanosic_PM_try_wakeup(struct nano_i2c_client *i2c_client)
{
    int level = 1;
    int irq_pin = -1;
    int retry_times = 100;
    int ret = 1;

    ATRACE_BEGIN(__func__);

    ATRACE_BEGIN("wait for pm_mutex");
    mutex_lock(&i2c_client->pm_mutex);
    ATRACE_END();
    /*get gpio_irq_pin's level , high level present 803 in running state*/
    irq_pin = Nanosic_GPIO_irqpin_get();

    ATRACE_BEGIN("gpio status check");
    if (gpio_is_valid(irq_pin)){
        level = gpio_get_value(irq_pin);
        dbgprint(DEBUG_LEVEL, "gpio level: %d\n", level);
    }
    else{
        dbgprint(ERROR_LEVEL, "gpio pin not valid\n");
    }

    ATRACE_END();

    ATRACE_BEGIN("mod_timer");
    pm_stay_awake(i2c_client->dev);
    dbgprint(DEBUG_LEVEL, "pm_stay_awake\n");
    if (g_panel_status)
    {
        mod_timer(&i2c_client->pm_timer, jiffies + msecs_to_jiffies(PM_TIMER_TIMEOUT_PANEL_ON));
        dbgprint(DEBUG_LEVEL, "pm_timer timeout: %d\n", PM_TIMER_TIMEOUT_PANEL_ON);
    }
    else
    {
        mod_timer(&i2c_client->pm_timer, jiffies + msecs_to_jiffies(PM_TIMER_TIMEOUT_PANEL_OFF));
        dbgprint(DEBUG_LEVEL, "pm_timer timeout: %d\n", PM_TIMER_TIMEOUT_PANEL_OFF);
    }
    ATRACE_END();

    ATRACE_BEGIN("exit from sleep");
    /*output high level of sleep pin, make 803 back to work*/
    Nanosic_GPIO_sleep(true);
    ATRACE_END();

    if (level <= 0)
    {
        ATRACE_BEGIN("waiting for wakeup...");
        dbgprint(INFO_LEVEL, "waiting for wakeup...\n");
        while (retry_times-- > 0)
        {
            ATRACE_BEGIN("retry loop");

            mdelay(1);

            if (gpio_get_value(irq_pin))
                break;

            if (retry_times == 0)
            {
                ATRACE_BEGIN("reset mcu and reload firmware...");
                dbgprint(ERROR_LEVEL, "reset mcu and reload firmware...");
                Nanosic_PM_free(i2c_client);
                Nanosic_GPIO_reset();
                Nanosic_firmware_init(i2c_client);
                // delay 500ms for mcu ready
                mdelay(500);
                Nanosic_PM_init(i2c_client);
                ATRACE_END();
            }
            ATRACE_END();
        }
        // wait 5ms for 803 i2c controller
        dbgprint(INFO_LEVEL, "waiting for 803 i2c controller...\n");
        mdelay(5);
        ATRACE_END();
    }

    mutex_unlock(&i2c_client->pm_mutex);

    ATRACE_END();

    return ret;
}

/** **************************************************************************
 * @func  Nanosic_PM_expire
 *
 * @brief Handler for timer expire
 *
 ** */
void Nanosic_PM_expire(struct timer_list *timer)
{
    struct nano_i2c_client *i2c_client = from_timer(i2c_client, timer, pm_timer);

    dbgprint(DEBUG_LEVEL, "going to sleep\n");
    /*go to sleep*/
    Nanosic_PM_sleep();
    pm_relax(i2c_client->dev);
}

/** ************************************************************************
 *  @func Nanosic_timer_register
 *
 *  @brief create a timer
 *
 ** */
void Nanosic_PM_init(struct nano_i2c_client *i2c_client)
{
    timer_setup(&i2c_client->pm_timer, Nanosic_PM_expire, 0);
    pm_stay_awake(i2c_client->dev);
    dbgprint(DEBUG_LEVEL, "pm_stay_awake\n");
    if (g_panel_status)
    {
        mod_timer(&i2c_client->pm_timer, jiffies + msecs_to_jiffies(PM_TIMER_TIMEOUT_PANEL_ON));
        dbgprint(DEBUG_LEVEL, "pm_timer timeout: %d\n", PM_TIMER_TIMEOUT_PANEL_ON);
    }
    else
    {
        mod_timer(&i2c_client->pm_timer, jiffies + msecs_to_jiffies(PM_TIMER_TIMEOUT_PANEL_OFF));
        dbgprint(DEBUG_LEVEL, "pm_timer timeout: %d\n", PM_TIMER_TIMEOUT_PANEL_OFF);
    }

    dbgprint(DEBUG_LEVEL, "PM module initial\n");
}

/** ************************************************************************
 *  @func Nanosic_timer_exit
 *
 *  @brief  destroy the timer
 *
 ** */
void Nanosic_PM_free(struct nano_i2c_client *i2c_client)
{
    if (timer_pending(&i2c_client->pm_timer))
    {
        del_timer(&i2c_client->pm_timer);
        pm_relax(i2c_client->dev);
    }

    dbgprint(DEBUG_LEVEL, "PM module release\n");
}
