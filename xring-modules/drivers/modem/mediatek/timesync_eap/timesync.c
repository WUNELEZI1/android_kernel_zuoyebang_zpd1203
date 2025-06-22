// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
* Copyright (c) 2022 - 2023 MediaTek Inc.
*/

#include <linux/module.h>
#include <linux/timekeeping.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>

#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/version.h>

#include "libtimesync/timesync_lib.h"

#define TIMESYNC_COMPATIBLE_NAME    ("mediatek,timesync")
#define TIMESYNC_IRQ_NAME           ("MD2AP_TIMETICK_SYNC")
#define TIMESYNC_IRQ_FLAGS          (0)
#define TIMESYNC_IRQ_INDEX          (0)
#define TIMESYNC_IRQ_NULL           (0)

#define TIMESYNC_THREAD_NAME        ("THREAD_TIMESYNC_EAP")
#define TIMESYNC_INTR_LOW           (0)
#define TIMESYNC_INTR_HIGH          (1)
#define TIMESYNC_SIG_SRC_KERNEL     (1)

#define TIMESYNC_GPIO_NUM_NAME      "mdm-gnss-sync"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
#ifndef ARCH_NR_GPIOS
#if defined(CONFIG_ARCH_NR_GPIO) && CONFIG_ARCH_NR_GPIO > 0
#define ARCH_NR_GPIOS CONFIG_ARCH_NR_GPIO
#else
#define ARCH_NR_GPIOS          512
#endif
#endif
#endif

typedef struct {
    int                 gpio_num;
    unsigned int        gpio_irq;
    atomic_t            intr_sig;
    wait_queue_head_t   intr_wq;
    struct task_struct  *ptr_timesync_thread;
    ktime_t             intr_boottime;
} TIMESYNC_CTRL_BLOCK_T;
static TIMESYNC_CTRL_BLOCK_T s_ctrl_block;

static irqreturn_t timesync_isr_func(int irq, void *dev)
{
    TIMESYNC_CTRL_BLOCK_T *ptr_ctrl_block = (TIMESYNC_CTRL_BLOCK_T *) dev;

    ptr_ctrl_block->intr_boottime = ktime_get_boottime();
    atomic_set(&ptr_ctrl_block->intr_sig, TIMESYNC_INTR_HIGH);
    wake_up(&ptr_ctrl_block->intr_wq);
    // TIMESYNC_LOG_NOTICE("receive gpio with 0x%llx\n", ptr_ctrl_block->intr_boottime);
    return IRQ_HANDLED;
}

static int timesync_thread_func(void *arg)
{
    TIMESYNC_CTRL_BLOCK_T       *ptr_ctrl_block = (TIMESYNC_CTRL_BLOCK_T *) arg;
    MSG_CMD_T                   msg_cmd;
    TIMESYNC_TIMESTAMPS_T       timestamps = {0};
    int                         ret;
    void                        *ptr_data;
    unsigned int                data_len;

    // register the interrupt signal listened by the thread (i.e. be waken up by SIGKILL)
    allow_signal(SIGKILL);
    TIMESYNC_LOG_NOTICE("thread is starting!\n");

    // init port in thread due to the possibility of port failure at init stage
    if (port_trxn_init(PORT_TIMEOUT_INF) == 0) {
        port_trxn_reset(PORT_TRXN_CLEAR_QUEUE_EN);
    }
    while (!kthread_should_stop()) {
        ret = port_trxn_cmd(
                TRXN_CMD_READ,
                MSG_CHN_TIMESYNC,
                MSG_CMD_REQ,
                &timestamps, sizeof(timestamps));
        if (ret == -EREMCHG) {
            TIMESYNC_LOG_WARN("receive reset signal\n");
            port_trxn_reset(PORT_TRXN_CLEAR_QUEUE_DIS);
            continue;
        }
        if (ret == -EINVAL) {
            break;
        }
        TIMESYNC_LOG_NOTICE("read request with ret=%d/id:0x%x...\n", ret, timestamps.session_id);

        if (ret >= 0) {
            timestamps.req_eap_recv_boottime = ktime_get_boottime();
            ret = wait_event_killable_timeout(
                    ptr_ctrl_block->intr_wq,
                    atomic_cmpxchg(&ptr_ctrl_block->intr_sig, TIMESYNC_INTR_HIGH, TIMESYNC_INTR_LOW) == TIMESYNC_INTR_HIGH,
                    msecs_to_jiffies(TIMEOUT_MSEC));
            if (ret <= 0) {
                TIMESYNC_LOG_ERR("wait gpio intr timeout with ret=%d\n", ret);
                ret = -ENETUNREACH;
            }
        }

        // for rmmod kernel module to terminate the thread
        if (signal_pending(current)) {
            TIMESYNC_LOG_WARN("catch SIGKILL(%d) & ret=%d to exit!\n", SIGKILL, ret);
            break;
        }

        if (ret >= 0) {
            timestamps.intr_eap_recv_boottime = ptr_ctrl_block->intr_boottime;
            timestamps.resp_eap_send_boottime = ktime_get_boottime();
            ptr_data = &timestamps;
            data_len = sizeof(timestamps);
            msg_cmd = MSG_CMD_RESP;
            ret = 0;
            TIMESYNC_LOG_NOTICE("id:0x%x result:0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx\n", timestamps.session_id,
                    timestamps.req_sap_send_boottime, timestamps.req_eap_recv_boottime,
                    timestamps.intr_sap_send_boottime, timestamps.intr_eap_recv_boottime,
                    timestamps.resp_eap_send_boottime, timestamps.resp_sap_recv_boottime);
        } else {
            ptr_data = &ret;
            data_len = sizeof(ret);
            msg_cmd = MSG_CMD_ERROR;
        }
        // error handling of timestamp reset for gpio/transaction failure
        atomic_set(&s_ctrl_block.intr_sig, TIMESYNC_INTR_LOW);
        s_ctrl_block.intr_boottime = 0;

        TIMESYNC_LOG_NOTICE("process request with ret=%d\n", ret);
        ret = port_trxn_cmd(
                TRXN_CMD_WRITE,
                MSG_CHN_TIMESYNC,
                msg_cmd,
                ptr_data, data_len);
        TIMESYNC_LOG_NOTICE("write response with ret=%d...\n", ret);
    }
    TIMESYNC_LOG_NOTICE("thread is stopping!\n");

    return 0;
}

static void
timesync_exit(void)
{
    if (s_ctrl_block.gpio_irq != TIMESYNC_IRQ_NULL) {
        disable_irq(s_ctrl_block.gpio_irq);
        free_irq(s_ctrl_block.gpio_irq, &s_ctrl_block);
        s_ctrl_block.gpio_irq = TIMESYNC_IRQ_NULL;
    }

    if (s_ctrl_block.ptr_timesync_thread != NULL) {
        send_sig(SIGKILL, s_ctrl_block.ptr_timesync_thread, TIMESYNC_SIG_SRC_KERNEL);
        kthread_stop(s_ctrl_block.ptr_timesync_thread);
        s_ctrl_block.ptr_timesync_thread = NULL;
    }

    port_trxn_exit();

    TIMESYNC_LOG_NOTICE("kernel module is removed!\n");
}

static int __init
timesync_init(void)
{
    int ret;
    struct device_node *ptr_node;
    const char *ptr_str;

    s_ctrl_block.gpio_irq = TIMESYNC_IRQ_NULL;
    s_ctrl_block.ptr_timesync_thread = NULL;

    if ((ptr_node = of_find_compatible_node(NULL, NULL, TIMESYNC_COMPATIBLE_NAME)) == NULL) {
        TIMESYNC_LOG_ERR("get device node failed\n");
        timesync_exit();
        return -EINVAL;
    }
    s_ctrl_block.gpio_num = of_get_named_gpio(ptr_node, TIMESYNC_GPIO_NUM_NAME, 0);
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
       if (s_ctrl_block.gpio_num < 0) {
            TIMESYNC_LOG_ERR("The GPIO <%s> is probed fail, %d\n", TIMESYNC_GPIO_NUM_NAME, s_ctrl_block.gpio_num);
            timesync_exit();
            return -EINVAL;
        }
    #else
    if (s_ctrl_block.gpio_num < 0 || s_ctrl_block.gpio_num >= ARCH_NR_GPIOS) {
        TIMESYNC_LOG_ERR("The GPIO <%s> is probed fail, %d\n", TIMESYNC_GPIO_NUM_NAME, s_ctrl_block.gpio_num);
        timesync_exit();
        return -EINVAL;
    }
    #endif
    ret = gpio_direction_input(s_ctrl_block.gpio_num);
    if (ret) {
        TIMESYNC_LOG_ERR("set gpio to input failed, ret=%d\n", ret);
        timesync_exit();
        return -EINVAL;
    }
    s_ctrl_block.gpio_irq = gpio_to_irq(s_ctrl_block.gpio_num);
    if (s_ctrl_block.gpio_irq <= 0) {
        TIMESYNC_LOG_ERR("Failed to get irq of GPIO<%d>\n", s_ctrl_block.gpio_num);
        s_ctrl_block.gpio_irq = TIMESYNC_IRQ_NULL;
        timesync_exit();
        return IRQ_NOTCONNECTED;
    }
    atomic_set(&s_ctrl_block.intr_sig, TIMESYNC_INTR_LOW);
    init_waitqueue_head(&s_ctrl_block.intr_wq);
    ret = request_irq(s_ctrl_block.gpio_irq,
            timesync_isr_func,
            IRQF_TRIGGER_RISING,
            TIMESYNC_IRQ_NAME,
            &s_ctrl_block);
    if (ret != 0) {
        TIMESYNC_LOG_ERR("register ISR failed with ret=%d\n", ret);
        s_ctrl_block.gpio_irq = TIMESYNC_IRQ_NULL;
        timesync_exit();
    }

    if (ret == 0) {
        s_ctrl_block.ptr_timesync_thread = kthread_run(timesync_thread_func, &s_ctrl_block, TIMESYNC_THREAD_NAME);
        if (IS_ERR(s_ctrl_block.ptr_timesync_thread)) {
            TIMESYNC_LOG_ERR("create thread failed\n");
            ret = PTR_ERR(s_ctrl_block.ptr_timesync_thread);
            s_ctrl_block.ptr_timesync_thread = NULL;
            timesync_exit();
        }
    }

    if (ret == 0) {
        // to avoid unbalanced enable for IRQ
        disable_irq(s_ctrl_block.gpio_irq);
        enable_irq(s_ctrl_block.gpio_irq);

        TIMESYNC_LOG_NOTICE("kernel module is inserted with gpio_irq=%u thread=0x%lx dts=%d/str=%s!\n",
                s_ctrl_block.gpio_irq, (unsigned long) s_ctrl_block.ptr_timesync_thread,
                of_property_read_string(ptr_node, "status", &ptr_str), ptr_str);
    }

    return ret;
}

module_init(timesync_init);
module_exit(timesync_exit);

MODULE_DESCRIPTION("TIMESYNC eAP Driver");
MODULE_AUTHOR("Wei Chiou <wei.chiou@mediatek.com>");
MODULE_LICENSE("Dual MIT/GPL");
