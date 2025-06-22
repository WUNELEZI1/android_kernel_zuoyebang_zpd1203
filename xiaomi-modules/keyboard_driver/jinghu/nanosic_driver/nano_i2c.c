/** ***************************************************************************
 * @file nano_i2c.c
 *
 * @brief nanosic i2c client
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
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
// #include <linux/sys_config.h>

#include "nano_macro.h"
#include "nanosic_trace.h"
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
#include <uapi/linux/sched/types.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/mi_sched.h>
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

// static bool first = true;
/*i2c slave address用单字节表示*/
static const unsigned int const_iaddr_bytes = 1;
/* I2C internal address max length */
#define INT_ADDR_MAX_BYTES 4
#include "nanosic_trace.h"

/** **************************************************************************
 * @brief
 *
 ** */
static void Nanosic_i2c_specified_packets_detect(char *data)
{
    struct nano_i2c_client *i2c_client = gI2c_client;
    static bool keyboard_conneted = 0;
    static bool keyboard_pogo_pin = 0;
    static bool keyboard_power = 0;
    static short version_touchpad;
    short version_forceic;
    static short version_176x;
    char command;
    char source;
    char object;
    char hall_n;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client err!\n");
        return;
    }

    if (!data)
        return;

    STREAM_TO_UINT8(source, data);
    STREAM_TO_UINT8(object, data);
    STREAM_TO_UINT8(command, data);

    if (command == 0x1 && source == FIELD_803X && object == FIELD_HOST)
    {
        memcpy(i2c_client->vers_803x, data + 2, 20);
        dbgprint(INFO_LEVEL, "version803x=%s\n", strlen(i2c_client->vers_803x) > 0 ? i2c_client->vers_803x : "null");

#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
        factory_auto_test_wakeup(i2c_client, &i2c_client->vers803x_pkts);
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST

    }
    else if (command == 0x1 && source == FIELD_176X && object == FIELD_HOST)
    {
        data++; // skip len
        STREAM_TO_UINT16(version_176x, data);
        STREAM_TO_UINT16(version_touchpad, data);
        SPECIFY_POS_TO_UINT16(version_forceic, data, OFFSET_FORCEIC_VERSION);
        snprintf(i2c_client->vers_176x, sizeof(i2c_client->vers_176x), "%x", version_176x);
        snprintf(i2c_client->vers_forceic, sizeof(i2c_client->vers_forceic), "%x", version_forceic);
        snprintf(i2c_client->vers_touchpad, sizeof(i2c_client->vers_touchpad), "%x", version_touchpad);

        if (keyboard_conneted & keyboard_power)
        {
            dbgprint(INFO_LEVEL, "Connected=[%d] Power=[%d] POGOPIN=[%s] versionKeyboard=[%s] versionTouchpad=[%s] versionForceIC=[%s]\n",
                    keyboard_conneted, keyboard_power, keyboard_pogo_pin ? "ERROR" : "OK", strlen(i2c_client->vers_176x) > 0 ? i2c_client->vers_176x : "null",
                    strlen(i2c_client->vers_touchpad) > 0 ? i2c_client->vers_touchpad : "null", strlen(i2c_client->vers_forceic) > 0 ? i2c_client->vers_forceic : "null");
        }
        else
        {
            dbgprint(INFO_LEVEL, "Connected=[%d] Power=[%d]\n", keyboard_conneted, keyboard_power);
        }
    }
    else if (command == 0xA2 && source == FIELD_176X && object == FIELD_HOST)
    {
        data += 4;
        STREAM_TO_UINT8(i2c_client->keyboard_status, data);
        keyboard_conneted = (i2c_client->keyboard_status >> 0) & 0x01;
        keyboard_power = (i2c_client->keyboard_status >> 1) & 0x01;
        keyboard_pogo_pin = (i2c_client->keyboard_status >> 6) & 0x01;
        atomic_set(&i2c_client->keyboard_connected_check,keyboard_conneted);
        if(keyboard_pogo_pin){
            xiaomi_keypad_notify(-1, version_176x);
            dbgprint(INFO_LEVEL, "KeyboardState=[%d] versionKeyboard=[%d]\n",-1, version_176x);
        }else if(keyboard_conneted){
            xiaomi_keypad_notify(1, version_176x);
            dbgprint(INFO_LEVEL, "KeyboardState=[%d] versionKeyboard=[%d]\n",1, version_176x);
        }else{
            xiaomi_keypad_notify(0, version_176x);
            dbgprint(INFO_LEVEL, "KeyboardState=[%d] versionKeyboard=[%d]\n",0, version_176x);
        }
        if(i2c_client->sensor_dev && i2c_client->notifier_handler)
        {
             if(keyboard_conneted)
                i2c_client->notifier_handler(i2c_client->sensor_dev,HALL_KEYBOARD);
             else if(!atomic_read(&i2c_client->holster_standby_hall_check))
                i2c_client->notifier_handler(i2c_client->sensor_dev,HALL_AWAY);
        }
        if (keyboard_conneted & keyboard_power)
        {
            dbgprint(INFO_LEVEL, "Connected=[%d] Power=[%d] POGOPIN=[%s]\n",
                keyboard_conneted, keyboard_power, keyboard_pogo_pin ? "ERROR" : "OK");
        }
        else
        {
            dbgprint(INFO_LEVEL, "Connected=[%d] Power=[%d]\n", keyboard_conneted, keyboard_power);
        }
    }
    else if (command == 0xA4 && source == FIELD_176X && object == FIELD_HOST)
    {
        data = data+3;
        STREAM_TO_UINT8(hall_n, data);
        atomic_set(&i2c_client->holster_standby_hall_check, hall_n);
        if(i2c_client->sensor_dev && i2c_client->notifier_handler)
        {
            if(hall_n)
                i2c_client->notifier_handler(i2c_client->sensor_dev,HALL_HOLSTER);
            else if(!atomic_read(&i2c_client->keyboard_connected_check))
                i2c_client->notifier_handler(i2c_client->sensor_dev,HALL_AWAY);
        }
        dbgprint(INFO_LEVEL, "Connected=[%d] Power=[%d] POGOPIN=[%s] HALL_HOLSTER=[%s]\n",
            keyboard_conneted, keyboard_power, keyboard_pogo_pin ? "ERROR" : "OK",atomic_read(&i2c_client->holster_standby_hall_check)?"NEAR":"FAR");
    }
#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
    else if (command == 0x30 && source == FIELD_176X && object == FIELD_HOST)
    {
        factory_auto_test_wakeup(i2c_client, &i2c_client->vers176x_pkts);
    }
#endif //ENABLE_SUPPORT_FACTORY_AUTO_TEST

}

/** **************************************************************************
 * @brief 驱动probe时的read接口
 *
 ** */
static int Nanosic_i2c_read_boot(struct i2c_client *client, void *buf, size_t len)
{
    unsigned char addr[INT_ADDR_MAX_BYTES];
    struct i2c_msg msg[2];
    int ret;

    if (IS_ERR_OR_NULL(client))
    {
        dbgprint(ERROR_LEVEL, "client is NULL\n");
        return -1;
    }

    addr[0] = client->addr;
    /*
     * Send out the register address...
     */
    msg[0].len = const_iaddr_bytes;
    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].buf = addr;
    /*
     * ...then read back the result.
     */
    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer read error\n");
        len = -1;
    }

    return len;
}

/** **************************************************************************
 * @brief   驱动probe时的write接口
 *
 ** */
static int Nanosic_i2c_write_boot(struct i2c_client *client, void *buf, size_t len)
{
    struct i2c_msg msg;
    unsigned char tmp_buf[128] = {0};
    int ret;

    if (IS_ERR_OR_NULL(client))
    {
        dbgprint(ERROR_LEVEL, "client is NULL\n");
        return -1;
    }

    tmp_buf[0] = client->addr;
    memcpy(tmp_buf + const_iaddr_bytes, buf, len);

    msg.addr = client->addr;
    msg.flags = 0;
    msg.len = len + const_iaddr_bytes;
    msg.buf = tmp_buf;

    ret = i2c_transfer(client->adapter, &msg, 1);
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer write error\n");
        len = -1;
    }

    return len;
}

/** **************************************************************************
 * @brief
 *
 ** */
int Nanosic_i2c_read(struct nano_i2c_client *i2c_client, void *buf, size_t len)
{
    struct i2c_adapter *adap;
    unsigned char addr[INT_ADDR_MAX_BYTES];
    struct i2c_msg msg[2];
    int ret;

    ATRACE_BEGIN(__func__);

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client is NULL\n");
        ret = -1;
        goto out;
    }

    if (IS_ERR_OR_NULL(i2c_client->mdata))
    {
        dbgprint(ERROR_LEVEL, "mdata err of null\n");
        ret = -1;
        goto out;
    }

    if (i2c_client->mdata->dev_pm_suspend) {
        pm_wakeup_event(i2c_client->dev, 5000);
        ret = wait_for_completion_timeout(&i2c_client->mdata->dev_pm_suspend_completion, msecs_to_jiffies(500));
        dbgprint(INFO_LEVEL, "waiting for i2c ...");
        if (!ret) {
            dbgprint(ERROR_LEVEL, "i2c can't finished resuming procedure, skip it\n");
            ret = -1;
            goto out;
        }
    }

    addr[0] = i2c_client->i2c_slave_addr;

    adap = i2c_get_adapter(i2c_client->i2c_bus_id);
    /*
     * Send out the register address...
     */
    msg[0].len = const_iaddr_bytes;
    msg[0].addr = i2c_client->i2c_slave_addr;
    msg[0].flags = 0;
    msg[0].buf = addr;
    /*
     * ...then read back the result.
     */
    msg[1].addr = i2c_client->i2c_slave_addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    mutex_lock(&i2c_client->read_mutex);

    ATRACE_BEGIN("i2c_transfer");
    ret = i2c_transfer(adap, msg, 2);
    ATRACE_END();

    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer read error, ret = %d\n", ret);
        len = -1;
    }
    ret = len;

    mutex_unlock(&i2c_client->read_mutex);

out:
    ATRACE_END();

    return ret;
}

/** **************************************************************************
 * @brief
 *   write data to 8030x
 ** */
int Nanosic_i2c_write(struct nano_i2c_client *i2c_client, void *buf, size_t len)
{
    struct i2c_msg msg;
    struct i2c_adapter *adap;
    unsigned char tmp_buf[128] = {0};
    int ret;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client is NULL\n");
        return -EINVAL;
    }
    if (IS_ERR_OR_NULL(i2c_client->mdata))
    {
        dbgprint(ERROR_LEVEL, "mdata err of null\n");
        return -EINVAL;
    }

    if (!Nanosic_PM_try_wakeup(i2c_client))
        dbgprint(ERROR_LEVEL, "try wakeup fail, i2c_write may be err\n");
    if (i2c_client->mdata->dev_pm_suspend) {
        pm_wakeup_event(i2c_client->dev, 5000);
        ret = wait_for_completion_timeout(&i2c_client->mdata->dev_pm_suspend_completion, msecs_to_jiffies(500));
        dbgprint(INFO_LEVEL, "waiting for i2c ...");
        if (!ret) {
            dbgprint(ERROR_LEVEL, "i2c can't finished resuming procedure, skip it\n");
            return -EINVAL;
        }
    }
    adap = i2c_get_adapter(i2c_client->i2c_bus_id);
    if (IS_ERR_OR_NULL(adap))
    {
        dbgprint(ERROR_LEVEL, "adap is NULL\n");
        return -EINVAL;
    }

    tmp_buf[0] = i2c_client->i2c_slave_addr;
    memcpy(tmp_buf + const_iaddr_bytes, buf, len);

    msg.addr = i2c_client->i2c_slave_addr;
    msg.flags = 0;
    msg.len = len + const_iaddr_bytes;
    msg.buf = tmp_buf;

    mutex_lock(&i2c_client->write_mutex);

    ret = i2c_transfer(adap, &msg, 1);
    if (ret < 0)
    {
        dbgprint(ERROR_LEVEL, "i2c_transfer write error ret = %d\n", ret);
        len = -1;
    }

    mutex_unlock(&i2c_client->write_mutex);

    return len;
}

/** **************************************************************************
 * @brief   i2c上传数据的解析 , 数据格式参见 <IIC总线协议>文挡
 *
 ** */

int Nanosic_i2c_parse(char *data, int datalen)
{
    char *p = data;
    char left;
    unsigned char first_byte;  /*fixed data 0x57*/
    unsigned char second_byte; /*sqe*/
    unsigned char third_byte;  /*fixed data 0x39 , 0x4A , 0x5B , 0x6C*/
    unsigned char fourth_byte; /*fixed data 0x2: mouse  0x5: keyboard 0x19: touch*/
    unsigned short seq = 0;
    int ret = 0;

    ATRACE_BEGIN(__func__);

    if (datalen < 0)
    {
        dbgprint(ERROR_LEVEL, "invalid data length\n");
        ret = -1;
        goto out;
    }


    left = datalen;

    //    atomic_inc(&i2c_received);

    STREAM_TO_UINT8(first_byte, p);
    left--;
    if (first_byte != 0x57 && first_byte != 0xD7)
    {
        dbgprint(ERROR_LEVEL, "first_byte error\n");
        rawdata_show("_first_byte_err_", data, datalen);
        ret = -1;
        goto out;
    }

    STREAM_TO_UINT8(second_byte, p);
    left--;

    STREAM_TO_UINT8(third_byte, p);
    left--;

    if (third_byte == 0)
    {
        /*null packet*/
        dbgprint(INFO_LEVEL, "third_byte is null, invalid data\n");
        // rawdata_show("_third_byte_null_", data, datalen);
        ret = -1;
        goto out;
    }

    if (third_byte != 0x39 && third_byte != 0x4A && third_byte != 0x5B && third_byte != 0x6C)
    {
        /*no packets include*/
        dbgprint(ERROR_LEVEL, "third_byte error\n");
        rawdata_show("_third_byte_error_", data, datalen);
        ret = -1;
        goto out;
    }

    while (left > 0)
    {
        STREAM_TO_UINT8(fourth_byte, p);
        STREAM_TO_UINT16(seq, p);
        p -= 3;

        switch (fourth_byte)
        {
        case 0x5:
            /*05 00 00 52 00 00 00 00 00 上*/
            /*05 00 00 51 00 00 00 00 00 下*/
            if (g_panel_status)
            {
                Nanosic_cache_put();
                Nanosic_input_write(EM_PACKET_KEYBOARD, p, 9);
                p += 9;
                left -= 9;
            }
            else
            {
                Nanosic_cache_insert(EM_PACKET_KEYBOARD, p, 9);
                p += 9;
                left -= 9;
            }
            break;
        case 0x6:
            if (g_panel_status)
            {
                Nanosic_cache_put();
                Nanosic_input_write(EM_PACKET_CONSUMER, p, 5);
                p += 5;
                left -= 5;
            }
            else
            {
                Nanosic_cache_insert(EM_PACKET_CONSUMER, p, 5);
                p += 5;
                left -= 5;
            }
            break;
        case 0x2:
            Nanosic_input_write(EM_PACKET_MOUSE, p, 8);
            p += 8;
            left -= 8;
            break;
        case 0x19:
#ifdef HAVE_TOUCHPAD
            if (xiaomi_touchpad_edge_filter(p) != 0)
                Nanosic_input_write(EM_PACKET_TOUCH, p, 27);
#else
            Nanosic_input_write(EM_PACKET_TOUCH, p, 27);
#endif // HAVE_TOUCHPAD
            p += 27;
            left -= 27;
            break;
        case 0x22:
            Nanosic_i2c_specified_packets_detect(p + 2);
            Nanosic_chardev_client_write(p, 16); /* upstreams data via /dev/nanodev0 to userspace*/
#ifdef  ENABLE_POGO_CHARGE
            ATRACE_BEGIN("Nanosic_vendor_raw_data_inject");
            Nanosic_vendor_raw_data_inject(p, 16);
            ATRACE_END();
#endif
            rawdata_show("_vendor_one_16byte_", p, 16);
            p += 16;
            left -= 16;
            break;
        case 0x23:
            Nanosic_i2c_specified_packets_detect(p + 2);
            Nanosic_chardev_client_write(p, 32);
#ifdef  ENABLE_POGO_CHARGE
            ATRACE_BEGIN("Nanosic_vendor_raw_data_inject");
            Nanosic_vendor_raw_data_inject(p, 32);
            ATRACE_END();
#endif
            Nanosic_input_write(EM_PACKET_VENDOR, p, 32);
            rawdata_show("_vendor_one_32byte_", p, 32);
            p += 32;
            left -= 32;
            break;
        case 0x24:
            Nanosic_i2c_specified_packets_detect(p + 2);
            Nanosic_chardev_client_write(p, left); /* upstreams data via /dev/nanodev0 to userspace*/
#ifdef  ENABLE_POGO_CHARGE
            ATRACE_BEGIN("Nanosic_vendor_raw_data_inject");
            Nanosic_vendor_raw_data_inject(p, left);
            ATRACE_END();
#endif
            rawdata_show("_vendor_one_64byte_", p, left);
            ret = -1;
            goto out;
        case 0x26:
            Nanosic_i2c_specified_packets_detect(p + 2);
            Nanosic_chardev_client_write(p, left); /* upstreams data via /dev/nanodev0 to userspace*/
#ifdef  ENABLE_POGO_CHARGE
            ATRACE_BEGIN("Nanosic_vendor_raw_data_inject");
            Nanosic_vendor_raw_data_inject(p, left);
            ATRACE_END();
#endif
            rawdata_show("_vendor_two_64byte_", p, left);
            ret = -1;
            goto out;
        default:
            left = 0;
            break;
        }
    }

out:
    ATRACE_END();

    return ret;
}

/** ************************************************************************
 * @brief   Nanosic_i2c_read_process
 *          工作队列下半部读取i2c数据并解析
 ** */
int Nanosic_i2c_read_handler(void *data)
{
    struct nano_i2c_client *i2c_client = (struct nano_i2c_client *)data;

    char buf[I2C_DATA_LENGTH_READ] = {0};
    int ret = 0;
    int datalen;

    ATRACE_BEGIN(__func__);

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ALERT_LEVEL, "Nanosic_i2c_read_handler, null\n");
        ret = -1;
        goto out;
    }


    if (!Nanosic_PM_try_wakeup(i2c_client))
        dbgprint(ERROR_LEVEL, "try wakeup fail, i2c_read may be err\n");

    datalen = Nanosic_i2c_read(i2c_client, buf, sizeof(buf));
    if (datalen > 0)
    {
        Nanosic_i2c_parse(buf, datalen);
    }
    else
    {
        atomic_inc(&i2c_client->i2c_error_count);
    }
    atomic_inc(&i2c_client->i2c_read_count);


out:
    ATRACE_END();

    return ret;
}
#ifdef I2C_GPIO_IRQNO
/** ************************************************************************
 * @brief   Nanosic_i2c_irq
 *          I2C GPIO 中断处理程序
 ** */
irqreturn_t Nanosic_i2c_irq(int irq, void *dev_id)
{
    struct nano_i2c_client *i2c_client = (struct nano_i2c_client *)dev_id;
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
    struct cpumask irq_thread_cpu_mask;
    static struct task_struct *keyboard_task = NULL;
    struct sched_param par = { .sched_priority = MAX_RT_PRIO - 1 };
    int cpu;
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

    // ATRACE_BEGIN("803 irq task timestamp start");
    // ATRACE_END();

    ATRACE_BEGIN(__func__);

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "i2c_client is NULL\n");
        return IRQ_HANDLED;
    }
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

    cpu_latency_qos_add_request(&i2c_client->pm_qos_req_i2c_irq, 0);

#endif // USE_IRQ_THREAD_QUICK_RESPONSE


#ifdef USE_KWORKER
    Nanosic_workQueue_schedule(i2c_client->worker);
#else
    Nanosic_i2c_read_handler(i2c_client);
#endif // USE_KWORKER

#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
    cpu_latency_qos_remove_request(&i2c_client->pm_qos_req_i2c_irq);
#endif // USE_IRQ_THREAD_QUICK_RESPONSE

    ATRACE_END();

    // ATRACE_BEGIN("803 irq task timestamp end");
    // ATRACE_END();

    return IRQ_HANDLED;
}
#endif

int Nanosic_i2c_read_version(struct i2c_client *client)
{
    char rsp[I2C_DATA_LENGTH_READ] = {0};
    char cmd[I2C_DATA_LENGTH_WRITE] = {0x32, 0x00, 0x4F, 0x30, 0x80, 0x18, 0x01, 0x00, 0x18};
    u8 retry = 0;
    int ret = -1;

    while (retry++ < 30)
    {
        ret = Nanosic_i2c_write_boot(client, cmd, sizeof(cmd));
        if (ret < 0)
        {
            dbgprint(ALERT_LEVEL, "i2c write cmd failed time %d\n", retry);
            msleep(100);
            continue;
        }
        dbgprint(ALERT_LEVEL, "i2c write OK %d\n", ret);
        msleep(2);
        ret = Nanosic_i2c_read_boot(client, rsp, sizeof(rsp));
        dbgprint(ALERT_LEVEL, "i2c read %d\n", ret);
        if (ret > 0)
        {
            rawdata_show("recv vers", rsp, sizeof(rsp));
            break;
        }
        msleep(2);
    }

    return ret;
}

int Nanosic_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    struct i2c_adapter *adapter = client->adapter;
    int ret = -1;

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
    {
        dbgprint(ALERT_LEVEL, "======return=====\n");
        return -ENODEV;
    }

    dbgprint(ALERT_LEVEL, "%s: addr = %x\n", __func__, client->addr);

    ret = Nanosic_i2c_read_version(client);

    return ret;
}

/** ************************************************************************
 *  @func Nanosic_i2c_register
 *
 *  @brief 注册gpio下降沿中断 , 当有中断触发时去获取i2c数据
 *
 ** */
int Nanosic_i2c_register(struct nano_i2c_client *i2c_client, int irqno, u32 irq_flags, int i2c_bus_id, int i2c_slave_addr)
{
    int ret;

    if (IS_ERR_OR_NULL(i2c_client))
    {
        dbgprint(ERROR_LEVEL, "Could not alloc memory\n");
        return -EINVAL;
    }

    if (i2c_bus_id < 0 || i2c_slave_addr < 0)
    {
        dbgprint(ERROR_LEVEL, "Invalid argments\n");
        return -EINVAL;
    }

    dbgprint(DEBUG_LEVEL, "new i2c %p\n", i2c_client);

    atomic_set(&i2c_client->i2c_read_count, 0);
    atomic_set(&i2c_client->i2c_error_count, 0);
    i2c_client->i2c_bus_id = i2c_bus_id;
    i2c_client->i2c_slave_addr = i2c_slave_addr;
    i2c_client->irqno = gpio_to_irq(irqno);
    i2c_client->irqflags = irq_flags;
    i2c_client->func = Nanosic_i2c_read_handler;
    /*initialize wakeup_irq spinlock*/
    spin_lock_init(&i2c_client->input_report_lock);
    /*initialize i2c read  mutexlock*/
    mutex_init(&i2c_client->read_mutex);
    /*initialize i2c write mutexlock*/
    mutex_init(&i2c_client->write_mutex);


    ret = Nanosic_firmware_init(i2c_client);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "firmware init fail! ret = %d\n", ret);
        return -EINVAL;
    }


    /* delay 200ms for mcu ready  */
    mdelay(200);

    i2c_client->input_dev = input_allocate_device();
    if (!i2c_client->input_dev)
    {
        dbgprint(ERROR_LEVEL, "No such input device defined! \n");
        return -ENOMEM;
    }

    __set_bit(EV_SYN, i2c_client->input_dev->evbit);
    __set_bit(EV_KEY, i2c_client->input_dev->evbit);
    i2c_client->input_dev->name = "xiaomi keyboard WakeUp";
    i2c_client->input_dev->id.product = 0x94;
    i2c_client->input_dev->id.vendor = 0x0827;

    input_set_capability(i2c_client->input_dev, EV_KEY, KEY_WAKEUP);
    // input_set_capability(i2c_client->input_dev, EV_SW, SW_LID);
    // input_set_capability(i2c_client->input_dev, EV_SW, SW_TABLET_MODE);
    ret = input_register_device(i2c_client->input_dev);
    if (ret)
    {
        dbgprint(ERROR_LEVEL, "No such input device\n");
        return ret;
    }

#ifdef USE_KWORKER
    /*initialize workqueue*/
    i2c_client->worker = Nanosic_workQueue_register(i2c_client);
    if (IS_ERR_OR_NULL(i2c_client->worker))
    {
        dbgprint(ERROR_LEVEL, "Could not register workqueue\n");
        return -ENOMEM;
    }

    dbgprint(DEBUG_LEVEL, "new worker %p\n", i2c_client->worker);
#endif

#ifdef I2C_GPIO_IRQNO
    dbgprint(INFO_LEVEL, "irq-gpio:%d irq-no:%d irq_flags:%d \n", irqno, i2c_client->irqno, irq_flags);
    if (i2c_client->irqno > 0)
    {
        // ret = request_irq(i2c_client->irqno, Nanosic_i2c_irq, irq_flags/*IRQF_TRIGGER_FALLING*/, "8030_io_irq", i2c_client);
        ret = request_threaded_irq(i2c_client->irqno, NULL, Nanosic_i2c_irq, irq_flags, "8030_io_irq", i2c_client);
        if (ret < 0)
        {
            dbgprint(ERROR_LEVEL, "Could not register for %s interrupt, irq = %d, ret = %d\n", "8030_io_irq", i2c_client->irqno, ret);
            msleep(30);
            return ret;
        }
    }
    else
    {
        /*initialize timer for test*/
        Nanosic_timer_register(i2c_client);
    }
#endif
    return 0;
}

/** ************************************************************************
 *  @func Nanosic_i2c_release
 *
 *  @brief
 *
 ** */
void Nanosic_i2c_release(struct nano_i2c_client *i2c_client)
{
    if (IS_ERR_OR_NULL(i2c_client))
        return;
#ifdef USE_KWORKER
    Nanosic_workQueue_release(i2c_client->worker);
#endif
#ifdef I2C_GPIO_IRQNO
    if (i2c_client->irqno > 0)
    {
        free_irq(i2c_client->irqno, i2c_client);
    }
    else
    {
        /*release timer module*/
        Nanosic_timer_release();
    }
#endif
}
