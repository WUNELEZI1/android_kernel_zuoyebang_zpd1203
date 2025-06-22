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
#include <linux/delay.h>
#include <linux/gpio.h>
#include "nano_macro.h"
#include <linux/interrupt.h>
#include "nanosic_trace.h"
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
#include <uapi/linux/sched/types.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/mi_sched.h>
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

static bool registered = false;
static int gpio_reset_pin = -1;
static int gpio_status_pin = -1;
static int gpio_irq_pin = -1;
static int gpio_sleep_pin = -1;
static int gpio_i2c_ls_en_pin = -1;
int g_wakeup_irqno = -1;

bool g_panel_status = false; /*1:on, 0:off*/
#define SUPPORT_GPIO_SLEEP_FUNCTION

/** ************************************************************************
 * @brief   Nanosic_i2c_irq
 *          GPIO 唤醒中断处理程序
 ** */
irqreturn_t Nanosic_wakeup_irq(int irq, void *dev_id)
{
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
    struct cpumask irq_thread_cpu_mask;
    static struct task_struct *keyboard_task = NULL;
    struct sched_param par = { .sched_priority = MAX_RT_PRIO - 1 };
    int cpu;
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

    ATRACE_BEGIN(__func__);

#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
    if (keyboard_task == NULL) {
        /* keyboard prio improve */
        keyboard_task = current;
        dbgprint(INFO_LEVEL, "%s prio improve to %d", __func__, MAX_RT_PRIO - 1);
        sched_setscheduler_nocheck(keyboard_task, SCHED_FIFO, &par);

        cpumask_clear(&irq_thread_cpu_mask);
        for (cpu = 0; cpu <= 2; cpu++) {
            cpumask_set_cpu(cpu, &irq_thread_cpu_mask);
        }
        set_cpus_allowed_ptr(keyboard_task, &irq_thread_cpu_mask);
        dbgprint(INFO_LEVEL, "%s CPU affinity %d ", __func__, keyboard_task->nr_cpus_allowed);
#ifdef BALANCE_IRQ
        if (set_mi_rq_balance_irq_task(keyboard_task)) {
            dbgprint(INFO_LEVEL, "set %s as rq balance vip task ", __func__);
        }
#endif
    }
#endif // USE_IRQ_THREAD_QUICK_RESPONSE


    if (g_panel_status == false && gI2c_client != NULL)
    {
        ATRACE_BEGIN("report key_wakeup");
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
        cpu_latency_qos_add_request(&gI2c_client->pm_qos_req_wakeup_irq, 0);
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

        pm_wakeup_event(gI2c_client->dev, 1000);

        spin_lock(&gI2c_client->input_report_lock);

        input_report_key(gI2c_client->input_dev, KEY_WAKEUP, 1);
        input_sync(gI2c_client->input_dev);
        input_report_key(gI2c_client->input_dev, KEY_WAKEUP, 0);
        input_sync(gI2c_client->input_dev);

        spin_unlock(&gI2c_client->input_report_lock);
        dbgprint(INFO_LEVEL, "Nanosic_wakeup_irq wakeup panel\n");

#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
    cpu_latency_qos_remove_request(&gI2c_client->pm_qos_req_wakeup_irq);
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

        ATRACE_END();
    }

    ATRACE_END();

    return IRQ_HANDLED;
}

/** ************************************************************************
 *  @func Nanosic_GPIO_register
 *
 *  @brief gpio申请及配置
 *
 ** */
int Nanosic_GPIO_register(int reset_pin, int status_pin, int irq_pin, int sleep_pin, int i2c_ls_en_pin)
{
    int err;

    if (reset_pin < 0 || status_pin < 0 || irq_pin < 0 || sleep_pin < 0 || i2c_ls_en_pin < 0)
    {
        dbgprint(ERROR_LEVEL, "invalid pin value\n");
        return -EFAULT;
    }

    gpio_reset_pin = reset_pin;
    gpio_status_pin = status_pin;
    gpio_irq_pin = irq_pin;
#ifdef SUPPORT_GPIO_SLEEP_FUNCTION
    gpio_sleep_pin = sleep_pin;
#endif
    gpio_i2c_ls_en_pin = i2c_ls_en_pin;

    if (registered == true)
    {
        dbgprint(ERROR_LEVEL, "need register first\n");
        return -EFAULT;
    }
    err = gpio_request(gpio_reset_pin, NULL);
    if (err)
    {
        dbgprint(ERROR_LEVEL, "request reset gpio fail\n");
        return -EFAULT;
    }
    err = gpio_request(gpio_status_pin, NULL);
    if (err)
    {
        dbgprint(ERROR_LEVEL, "request status gpio fail\n");
        goto _err1;
    }

#ifdef SUPPORT_GPIO_SLEEP_FUNCTION
    err = gpio_request(gpio_sleep_pin, NULL);
    if (err)
    {
        dbgprint(ERROR_LEVEL, "request sleep gpio fail\n");
        goto _err2;
    }
#endif

    if (gpio_is_valid(gpio_irq_pin))
    {
        // err = gpio_request_one(gpio_irq_pin, GPIOF_IN, "8030_io_irq");
        err = gpio_request(gpio_irq_pin, NULL);
        if (err)
        {
            dbgprint(ERROR_LEVEL, "Failed to request gpio_irq_pin GPIO:%d\n", gpio_irq_pin);
            goto _err3;
        }
        err = gpio_direction_input(gpio_irq_pin);
        if (err)
        {
            dbgprint(ERROR_LEVEL, "set gpio to input&output fail\n");
            goto _err3;
        }
    }
    else
    {
        dbgprint(ERROR_LEVEL, "gpio_irq_pin is not valid\n");
    }

    gpio_direction_output(gpio_reset_pin, 0); /*output 0 from reset pin*/
    gpio_direction_output(gpio_sleep_pin, 0); /*output 0 from sleep pin*/
    gpio_direction_output(gpio_i2c_ls_en_pin, 0); /*output 0 from sleep pin*/
    gpio_direction_input(gpio_status_pin); /*input mode for status pin*/
    gpio_direction_input(gpio_irq_pin);    /*input mode for irq pin*/

    gpio_set_value(gpio_i2c_ls_en_pin, 1); /*output 1 from i2c_ls_en_pin*/

    msleep(2);

    gpio_set_value(gpio_reset_pin, 1); /*output 1 from reset pin*/

    msleep(500);

    gpio_set_value(gpio_sleep_pin, 1); /*output 1 from reset pin*/

    /*waiting mcu reset commplete*/
    msleep(50);

    if (gpio_is_valid(gpio_status_pin))
    {
        g_wakeup_irqno = gpio_to_irq(gpio_status_pin);
        err = request_threaded_irq(g_wakeup_irqno, NULL, Nanosic_wakeup_irq, IRQF_TRIGGER_RISING | IRQF_ONESHOT, "8030_io_wakeup", (void *)&gpio_status_pin);
        if (err < 0)
        {
            dbgprint(ERROR_LEVEL, "Could not register for %s interrupt, irq = %d, ret = %d\n", "8030_io_wakeup", g_wakeup_irqno, err);
            goto _err4;
        }
    }

    registered = true;
    g_panel_status = true;

    dbgprint(INFO_LEVEL, "register gpio succeed\n");

    return 0;

_err4:
    gpio_free(gpio_irq_pin);
_err3:
#ifdef SUPPORT_GPIO_SLEEP_FUNCTION
    gpio_free(gpio_sleep_pin);
#endif
_err2:
    gpio_free(gpio_status_pin);
_err1:
    gpio_free(gpio_reset_pin);

    return -EFAULT;
}

/** ************************************************************************
 *  @func Nanosic_GPIO_sleep
 *
 *  @brief switch to sleep mode , low level valid 低电平有效
 *  bool sleep : 0 -> 进入睡眠模式
 *               1 -> 退出睡眠模式
 ** */
void Nanosic_GPIO_sleep(bool sleep)
{
    static int sleep_pin_status = -1;

    if (registered == false)
    {
        dbgprint(ERROR_LEVEL, "need register first\n");
        return;
    }

    if ((sleep == true && sleep_pin_status == 1) || (sleep == false && sleep_pin_status == 0))
    {
        return;
    }
    if(sleep == true)
        msleep(10);
    /*GPIO_SLEEP set*/
    gpio_set_value(gpio_sleep_pin, sleep);
    sleep_pin_status = sleep ? 1 : 0;
    dbgprint(INFO_LEVEL, "set gpio sleep pin %d\n", sleep);
}

/** ************************************************************************
 *  @func Nanosic_GPIO_test
 *
 *  @brief set gpio pin level
 *
 ** */
void Nanosic_GPIO_set(int gpio_pin, bool gpio_level)
{
    int gpio_pin_value;
    gpio_pin_value = gpio_pin + 301;
    if (registered == false)
    {
        dbgprint(ERROR_LEVEL, "need register first\n");
        return;
    }

    if (!gpio_is_valid(gpio_pin))
    {
        dbgprint(ERROR_LEVEL, "invalid gpio pin %d\n", gpio_pin);
        return;
    }
    gpio_set_value(gpio_pin_value, gpio_level);
    dbgprint(DEBUG_LEVEL, "set gpio pin %d level %d\n", gpio_pin_value, gpio_level);
}

int Nanosic_GPIO_irqpin_get(void)
{
    return gpio_irq_pin;
}

void Nanosic_GPIO_reset(void)
{
    // output reset pin 0 for 100ms
    gpio_set_value(gpio_reset_pin, 0);
    mdelay(100);
    gpio_set_value(gpio_reset_pin, 1);

    // delay 500ms for iic ready
    mdelay(500);
    dbgprint(ALERT_LEVEL, "reset 8030 done!\n");
}

/** ************************************************************************
 *  @func Nanosic_Gpio_Release
 *
 *  @brief gpio free
 *
 ** */
void Nanosic_GPIO_release(void)
{
    if (registered == true)
    {
#ifdef SUPPORT_GPIO_IRQ_FUNCTION
        gpio_free(gpio_irq_pin);
#endif
#ifdef SUPPORT_GPIO_SLEEP_FUNCTION
        gpio_free(gpio_sleep_pin);
#endif
        if (gpio_is_valid(gpio_status_pin))
        {
            free_irq(gpio_to_irq(gpio_status_pin), &gpio_status_pin);
        }
        gpio_free(gpio_status_pin);
        gpio_free(gpio_reset_pin);
        registered = false;
    }
}
