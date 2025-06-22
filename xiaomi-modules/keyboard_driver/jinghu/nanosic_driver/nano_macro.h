/** ***************************************************************************
 * @file nano_macro.h
 *
 */
#ifndef _NANO_MACRO_H_
#define _NANO_MACRO_H_

#include <linux/device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/regulator/consumer.h>
#include <linux/power_supply.h>
#include <linux/pm_qos.h>
#include "../xiaomi_keyboard_driver/xiaomi_keyboard.h"

/*SDK版本号*/
#define DRV_VERSION  "1.4.2"

#define DRV_TOKEN "nanodev"
#define DRV_TAG " [nanodev]"

#define DEBUG_LEVEL (7)
#define INFO_LEVEL (5)
#define ERROR_LEVEL (3)
#define ALERT_LEVEL (1)

#define ENABLE_SUPPORT_FACTORY_AUTO_TEST
// #define ENABLE_PMIC_DUMP
// #define USE_KWORKER
// #define USE_IRQ_THREAD_QUICK_RESPONSE
#define KEYBOARD_PLATFORM_XRING

#define HAVE_TOUCHPAD

/*设备挂载的i2c bus id*/
#define I2C_BUS_ID (2)

/*设备的i2c addr*/
#define I2C_SLAVE_ADDR (0x4c) /*high 7bits   从设备地址，第一个字节的高7位，后一位是读写位*/

/*默认写的i2c长度是66bytes*/
#define I2C_DATA_LENGTH_WRITE (66)

/*默认读的i2c长度是67bytes*/
#define I2C_DATA_LENGTH_READ (68)

/*I2C GPIO中断线的中断编号 , 需要依据平台重新设置*/
#define I2C_GPIO_IRQNO (60)

#define FIELD_HOST (0x80)
#define FIELD_803X (0x18)
#define FIELD_176X (0x38)

/* the offset of forceic version compared to touchpad version */
#define OFFSET_FORCEIC_VERSION (7)

struct xiaomi_keyboard_data
{
    // struct notifier_block drm_notif;
    void *notifier_cookie;
    bool dev_pm_suspend;
    int irq;
    struct workqueue_struct *event_wq;
    struct work_struct early_resume_work;
    struct work_struct resume_work;
    struct work_struct early_suspend_work;
    struct work_struct suspend_work;
    struct completion dev_pm_suspend_completion;
};

typedef enum
{
    NANOSIC_IC_SUSPEND_IN = 0,
    NANOSIC_IC_SUSPEND_OUT = 1,
    NANOSIC_IC_RESUME_IN = 2,
    NANOSIC_IC_RESUME_OUT = 3,
    NANOSIC_IC_INIT = 4,
} nanosic_ic_state;

/*input设备结构体*/
struct nano_input_create_dev
{
    __u8 name[128];
    __u8 phys[64];
    __u8 uniq[64];
    __u8 *rd_data;
    __u16 rd_size;
    __u16 bus;
    __u32 vendor;
    __u32 product;
    __u32 version;
    __u32 country;
} __attribute__((__packed__));

typedef int (*handler_expired)(void);
// typedef int (*handler_tasklet)(void);
typedef int (*handler_workqueue)(void *data);
typedef int (*i2c_write_cb)(void *buf, size_t len);


/*worker 结构体信息*/
struct nano_worker_client
{
    struct work_struct worker;
    struct workqueue_struct *worker_queue;
    struct delayed_work worker_delay;
    handler_workqueue worker_func;
    void *worker_data;
    atomic_t schedule_count;
    atomic_t schedule_delay_count;
};

typedef void (*keyboard_hall_notifier_handler)(struct device *dev,int hall_state);
/*i2c 设备信息*/
struct nano_i2c_client
{
    handler_workqueue func;   /*i2c read function*/
    atomic_t i2c_read_count;  /*i2c读次数*/
    atomic_t i2c_error_count; /*错误包数*/
    int i2c_bus_id;           /*I2C bus id*/
    int i2c_slave_addr;       /*I2C slave device address*/
    int irqno;
    int irqflags;
    struct device *dev;
#ifdef USE_IRQ_THREAD_QUICK_RESPONSE
#endif // USE_IRQ_THREAD_QUICK_RESPONSE
    struct pm_qos_request pm_qos_req_i2c_irq;
    struct pm_qos_request pm_qos_req_wakeup_irq;
#ifdef USE_KWORKER
    struct nano_worker_client *worker;
#endif // USE_KWORKER
    struct mutex read_mutex;
    struct mutex write_mutex;
    struct input_dev *input_dev;
    struct spinlock input_report_lock;

    const char *vdd_name;
    const char *dvdd_name;
    struct regulator *vdd_reg;
    struct regulator *dvdd_reg;
#ifdef ENABLE_PMIC_DUMP
#define L13B_ADDR_BASE 0xCD00
#define L12B_ADDR_BASE 0xCC00
#define L6B_ADDR_BASE  0xC600
    int vdd_addr_base;
    int dvdd_addr_base;
#define REGMAP_NAME "0-01"
    struct regmap *map;
#endif // ENABLE_PMIC_DUMP

    short keyboard_status;
    atomic_t  holster_standby_hall_check;
    atomic_t  keyboard_connected_check;
    struct device * sensor_dev;
    keyboard_hall_notifier_handler  notifier_handler;
    nanosic_ic_state ic_state;
    char vers_803x[21];
    char vers_176x[5];
    char vers_forceic[5];
    char vers_touchpad[5];

    struct timer_list pm_timer;
    struct mutex pm_mutex;

#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
    atomic_t vers176x_pkts;
    atomic_t vers803x_pkts;
    wait_queue_head_t factory_auto_test_waitq;
    bool factory_auto_test_enable;
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST

    int work_buf;
    struct xiaomi_keyboard_data *mdata;

};

/*input数据包类型*/
typedef enum
{
    EM_PACKET_KEYBOARD = 0, /*键盘*/
    EM_PACKET_CONSUMER,     /*多媒体*/
    EM_PACKET_MOUSE,        /*鼠标*/
    EM_PACKET_TOUCH,        /*触摸*/
    EM_PACKET_VENDOR,       /*vendor*/
    EM_PACKET_UNKOWN,       /*未知*/
} EM_PacketType;
;

/* 屏幕状态类型 */
typedef enum
{
    SCREEN_OFF = 0,
    SCREEN_ON = 1
} screen_state;

/* 霍尔状态类型 */
typedef enum
{
    HALL_AWAY = 0,
    HALL_HOLSTER = 1,
    HALL_KEYBOARD = 2
} hall_state;
/*
 * Logging and Debugging
 */
/* The main debug macro.  Uncomment to enable debug messages */
#define DEBUG

/* Make sure we have a per-module token */
#ifndef DRV_TOKEN
#error Module must #define DRV_TOKEN to be a short nickname of the module.
#endif
/* Error messages -- always sent to log */
#undef errprint
#undef errmsg
#define errprint(__format_and_args...) \
    pr_info(DRV_TOKEN ": ERROR: " __format_and_args)

#define errmsg(__msg...) errprint(DRV_TOKEN, ##__msg)

// module_param(debug,int,S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP);
// MODULE_PARM_DESC(debug, "Set internal debugging level, higher is more verbose");

/*设置默认打印级别*/
extern int debuglevel;

#undef dbgprint
#undef dbgmsg
#define dbgprint(__level, __format, __args...)                                                                                                                          \
    do                                                                                                                                                                  \
    {                                                                                                                                                                   \
        if ((int)debuglevel >= (int)__level)                                                                                                                            \
        {                                                                                                                                                               \
            struct timespec64 ts;                                                                                                                                       \
            unsigned long local_time;                                                                                                                                   \
            struct rtc_time tm;                                                                                                                                         \
            ktime_get_real_ts64(&ts);                                                                                                                                   \
            local_time = (u32)(ts.tv_sec - (sys_tz.tz_minuteswest * 60));                                                                                               \
            rtc_time64_to_tm(local_time, &tm);                                                                                                                          \
            switch ((int)__level)                                                                                                                                    \
            {                                                                                                                                                           \
            case 0:                                                                                                                                                     \
            case 1:                                                                                                                                                     \
            case 2:                                                                                                                                                     \
                pr_info("[%02d:%02d:%02d.%03zu] <2>" DRV_TAG " %s %d: [DIS-TF-KEYBOARD] " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            case 3:                                                                                                                                                     \
                pr_info("[%02d:%02d:%02d.%03zu] <3>" DRV_TAG " %s %d: [DIS-TF-KEYBOARD] " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            case 4:                                                                                                                                                     \
                pr_info("[%02d:%02d:%02d.%03zu] <4>" DRV_TAG " %s %d: " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            case 5:                                                                                                                                                     \
                pr_info("[%02d:%02d:%02d.%03zu] <5>" DRV_TAG " %s %d: " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            case 6:                                                                                                                                                     \
                pr_info("[%02d:%02d:%02d.%03zu] <6>" DRV_TAG " %s %d: " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            default:                                                                                                                                                    \
                pr_info("[%02d:%02d:%02d.%03zu] <7>" DRV_TAG " %s %d: " __format, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec / 1000000, __func__, __LINE__, ##__args); \
                break;                                                                                                                                                  \
            }                                                                                                                                                           \
        }                                                                                                                                                               \
    } while (0)

#ifdef DEBUG
/**
 * @brief Send a debug message to the log facility.
 * @ingroup albrxdoc_util
 * @details This macro sends a message to the logging facility with a
 * simple token prefix.  This message is delivered iff the __level
 * parameter is not greater than the value of the \c debug static variable.
 * @note If \c DEBUG is undefined (i.e. the \#define is not active), no
 * debug messages are sent to the logging facility, regardless of level.
 */
#define dbgmsg(__level, __msg...) dbgprint(__level, DRV_TOKEN, ##__msg)
#else /* !DEBUG */
#define dbgmsg(__level, __msg...)
#endif
/* Assertions -- only checked if DEBUG is defined */
#undef ASSERT
#ifdef DEBUG
#define ASSERT(__condition, __fail_action)                                   \
    do                                                                       \
    {                                                                        \
        if (!(__condition))                                                  \
        {                                                                    \
            errmsg("ASSERT(%s:%d): %s\n", __FILE__, __LINE__, #__condition); \
            __fail_action;                                                   \
        }                                                                    \
    } while (0)
#else
#define ASSERT(__c, __f)
#endif

extern struct nano_i2c_client *gI2c_client;

/*打印raw data数组*/
static inline void rawdata_show(const char *descp, char *buf, size_t size)
{
    int i = 0;
    char display[300] = {0};

    if (!descp)
        return;

    strcat(display, "[[[");
    for (i = 0; i < (size > 40 ? 40 : size); i++)
    {
        char str[4] = {0};
        snprintf(str, sizeof(str), "%02X", buf[i]);
        strcat(display, str);
    }
    strcat(display, "]]]");

    dbgprint(INFO_LEVEL, "%s -> %s\n", descp, display);
}

extern int Nanosic_chardev_register(void);
extern void Nanosic_chardev_release(void);
extern int Nanosic_chardev_client_write(char *data, int datalen);
extern int Nanosic_chardev_client_notify_close(void);
extern void Nanosic_timer_register(struct nano_i2c_client *i2c_client);
extern void Nanosic_timer_release(void);
extern void Nanosic_sysfs_create(struct device *dev);
extern void Nanosic_sysfs_release(struct device *dev);
extern struct nano_worker_client *
Nanosic_workQueue_register(struct nano_i2c_client *i2c_client);
extern void Nanosic_workQueue_release(struct nano_worker_client *worker_client);
extern void Nanosic_workQueue_schedule(struct nano_worker_client *worker_client);
extern int Nanosic_i2c_read(struct nano_i2c_client *i2c_client, void *buf, size_t len);
extern int Nanosic_i2c_write(struct nano_i2c_client *i2c_client, void *buf, size_t len);
extern int Nanosic_i2c_parse(char *data, int datalen);
extern int Nanosic_i2c_read_handler(void *data);
extern int Nanosic_i2c_register(struct nano_i2c_client *i2c_client, int irqno, u32 irq_flags, int i2c_bus_id, int i2c_slave_addr);
extern void Nanosic_i2c_release(struct nano_i2c_client *i2c_client);
extern int Nanosic_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
extern int Nanosic_i2c_read_version(struct i2c_client *client);
extern irqreturn_t
Nanosic_i2c_irq(int irq, void *dev_id);
extern int Nanosic_input_register(void);
extern int Nanosic_input_release(void);
extern int Nanosic_input_write(EM_PacketType type, void *buf, size_t len);
extern int Nanosic_GPIO_register(int reset_pin, int status_pin, int irq_pin, int sleep_pin, int i2c_ls_en_pin);
extern void Nanosic_GPIO_release(void);
extern void Nanosic_GPIO_sleep(bool sleep);
extern void Nanosic_GPIO_reset(void);
extern void Nanosic_GPIO_set(int gpio_pin, bool gpio_level);
extern int Nanosic_GPIO_irqpin_get(void);
extern void Nanosic_cache_expire(struct timer_list *t);
extern int Nanosic_cache_insert(EM_PacketType type, void *data, size_t datalen);
extern int Nanosic_cache_init(void);
extern int Nanosic_cache_release(void);
extern int Nanosic_cache_put(void);
extern void Nanosic_PM_sleep(void);
extern bool Nanosic_PM_try_wakeup(struct nano_i2c_client* i2c_client);
extern void Nanosic_PM_init(struct nano_i2c_client *i2c_client);
extern void Nanosic_PM_free(struct nano_i2c_client *i2c_client);
extern int  Nanosic_firmware_write(struct nano_i2c_client* i2c_client, char reg_addr, void *buf, size_t len);
extern int  Nanosic_firmware_read(struct nano_i2c_client* i2c_client ,char reg_addr, void *buf, size_t len);
extern int  Nanosic_firmware_write_header(struct nano_i2c_client* i2c_client, uint8_t* head_ptr, int32_t head_size);
extern int  Nanosic_firmware_write_bin(struct nano_i2c_client* i2c_client, uint8_t* AppImage, int32_t AppImageSize);
extern int  Nanosic_firmware_write_start(struct nano_i2c_client* i2c_client);
extern int  Nanosic_firmware_upgrade(struct nano_i2c_client* i2c_client, uint8_t* code, uint32_t size);
extern int  Nanosic_firmware_init(struct nano_i2c_client* i2c_client);
#ifdef  ENABLE_POGO_CHARGE
/* for pogo charge start */
extern int Nanosic_charge_i2c_write_callback_register(i2c_write_cb cb);
extern int Nanosic_charge_i2c_write_callback_unregister(void);
/**
 * @brief vendor raw data :
 *        VendorOne(ReportID=0x22, 16 Byte);
 *        VendorOne(ReportID=0x23, 32 Byte);
 *        VendorOne(ReportID=0x24, 64 Byte);
 */
extern int Nanosic_vendor_raw_data_inject(void *buf, size_t len);
/* for pogo charge end */
#endif
#ifdef ENABLE_SUPPORT_FACTORY_AUTO_TEST
#define CONFIG_AUTO_TEST_MCU_LIMIT (10)
#define CONFIG_AUTO_TEST_KEYBOARD_LIMIT (100)
extern void factory_auto_test_wakeup( struct nano_i2c_client *i2c_client, atomic_t *condition);
#endif // ENABLE_SUPPORT_FACTORY_AUTO_TEST
extern int Nanosic_regulator_init(struct nano_i2c_client *I2client, struct device_node *of_node, struct i2c_client *client);
extern int Nanosic_switch_regulator(struct nano_i2c_client *I2client, bool on);
#ifdef ENABLE_PMIC_DUMP
extern void dump_regmap(struct nano_i2c_client *i2c_client, int base);
#endif // ENABLE_PMIC_DUMP
extern void Nanosic_GPIO_sleep(bool sleep);
__attribute__((unused)) static int xiaomi_keyboard_init(struct nano_i2c_client *i2c_client);
__attribute__((unused)) static void keyboard_early_resume_work(struct work_struct *work);
__attribute__((unused)) static void keyboard_resume_work(struct work_struct *work);
__attribute__((unused)) static void keyboard_early_suspend_work(struct work_struct *work);
__attribute__((unused)) static void keyboard_suspend_work(struct work_struct *work);
__attribute__((unused)) static int xiaomi_keyboard_pm_suspend(struct device *dev);
__attribute__((unused)) static int xiaomi_keyboard_pm_resume(struct device *dev);
__attribute__((unused)) static int xiaomi_keyboard_remove(void);
__attribute__((unused)) static const struct dev_pm_ops xiaomi_keyboard_pm_ops;

extern bool g_panel_status;
extern int g_wakeup_irqno;

#if 0
extern void Nanosic_GPIO_set(bool status);
#endif

#define STREAM_TO_UINT8(u8, p)      \
    {                               \
        u8 = (unsigned char)(*(p)); \
        (p) += 1;                   \
    }

#define STREAM_TO_UINT16(u16, p)                                                \
    {                                                                           \
        u16 = ((unsigned short)(*(p)) + (((unsigned short)(*((p) + 1))) << 8)); \
        (p) += 2;                                                               \
    }

#define SPECIFY_POS_TO_UINT16(u16, p, offset) \
    {                                                                                                   \
        u16 = ((unsigned short)(*((p) + (offset))) + (((unsigned short)(*((p) + (offset) + 1))) << 8)); \
    }

#endif /* _ALBRX_H_ */
