#ifndef _XIAOMI_KEYBOARD_MACRO_H_
#define _XIAOMI_KEYBOARD_MACRO_H_

#include <linux/rtc.h>

#define MI_KB_TAG "xiaomi-keyboard"

// #define ENABLE_XIAOMI_KEYBOARD_DEBUG

#ifdef ENABLE_XIAOMI_KEYBOARD_DEBUG
#define MI_KB_DEBUG(fmt, args...)                                   \
do {                                                                \
    struct timespec64 tv;                                           \
    unsigned long local_time;                                       \
    struct rtc_time tm;                                             \
    ktime_get_real_ts64(&tv);                                       \
    local_time = (u32)(tv.tv_sec - (sys_tz.tz_minuteswest * 60));   \
    rtc_time64_to_tm(local_time, &tm);                              \
    pr_info("[%02d:%02d:%02d.%03zu] [%s] %s %d: " fmt, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_nsec/1000000, MI_KB_TAG, __func__, __LINE__, ##args);	\
} while (0)
#else
#define MI_KB_DEBUG(fmt, args...) do {} while (0)
#endif // ENABLE_XIAOMI_KEYBOARD_DEBUG

#define MI_KB_LOG(fmt, args...)                                     \
do {                                                                \
    struct timespec64 tv;                                           \
    unsigned long local_time;                                       \
    struct rtc_time tm;                                             \
    ktime_get_real_ts64(&tv);                                       \
    local_time = (u32)(tv.tv_sec - (sys_tz.tz_minuteswest * 60));   \
    rtc_time64_to_tm(local_time, &tm);                              \
    pr_info("[%02d:%02d:%02d.%03zu] [%s] %s %d: " fmt, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_nsec/1000000, MI_KB_TAG, __func__, __LINE__, ##args);	\
} while (0)

#define MI_KB_ERR(fmt, args...)                                     \
do {                                                                \
    struct timespec64 tv;                                           \
    unsigned long local_time;                                       \
    struct rtc_time tm;                                             \
    ktime_get_real_ts64(&tv);                                       \
    local_time = (u32)(tv.tv_sec - (sys_tz.tz_minuteswest * 60));   \
    rtc_time64_to_tm(local_time, &tm);                              \
    pr_err("[%02d:%02d:%02d.%03zu] [%s] %s %d: [DIS-TF-KEYBOARD] " fmt, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_nsec/1000000, MI_KB_TAG, __func__, __LINE__, ##args);	\
} while (0)

static inline void rawdata_show(const char *descp, char *buf, size_t size)
{
    int i = 0;
    char display[300] = {0};

    if (!descp)
        return;

    strcat(display, "[[[");
    for (i = 0; i < (size > 30 ? 30 : size); i++)
    {
        char str[4] = {0};
        snprintf(str, sizeof(str), "%02X", buf[i]);
        strcat(display, str);
    }
    strcat(display, "]]]");

    MI_KB_DEBUG("%s -> %s\n", descp, display);
}

#endif
