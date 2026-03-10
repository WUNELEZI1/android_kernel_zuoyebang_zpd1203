#ifndef __LC_LOGFS_H__
#define __LC_LOGFS_H__

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>

typedef enum {
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
} lc_log_level_t;


 extern char global_tag[];
 extern lc_log_level_t current_level;
 extern bool logfs_enabled;

// 日志等级字符串映射
extern const char *log_level_str[];

/**
 * @brief 设置日志等级
 * @param level 日志等级（LOG_ERROR, LOG_INFO, LOG_DEBUG）
 */
extern void logfs_set_log_level(lc_log_level_t level);

/**
 * @brief 控制全局日志开关
 * @param enable 启用（true）或禁用（false）
 */
extern void lc_logfs_enable_all_logs(bool enable);

/**
 * @brief 打印日志的核心函数
 * @param level 日志等级
 * @param module_tag 模块自定义标签
 * @param file 文件名
 * @param func 函数名
 * @param line 行号
 * @param fmt 格式化字符串
 */

/**
 * @brief 重新定义标准日志接口
 */

#ifndef MODULE_TAG
#define MODULE_TAG "ADD_TAG"
#endif


#ifndef CHG_LOGE
#define CHG_LOGE(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_ERROR <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt, \
                    log_level_str[LOG_ERROR], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#define chg_loge(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_ERROR <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_ERROR], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif // CHG_LOGE

#ifndef CHG_LOGI
#define CHG_LOGI(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_INFO <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_INFO], global_tag, MODULE_TAG,  __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#define chg_logi(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_INFO <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_INFO], global_tag, MODULE_TAG,  __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif //CHG_LOGI

#ifndef CHG_LOGD
#define CHG_LOGD(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_DEBUG <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_DEBUG], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#define chg_logd(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_DEBUG <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_DEBUG], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif // CHG_LOGD


#ifndef pr_err
//#define pr_err(fmt, ...) logfs_print(LOG_ERROR, MODULE_TAG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_ERROR <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_ERROR], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif

#ifndef pr_info
//#define pr_info(fmt, ...) logfs_print(LOG_INFO, MODULE_TAG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_INFO <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_INFO], global_tag, MODULE_TAG,  __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif

#ifndef pr_debug
//#define pr_debug(fmt, ...) logfs_print(LOG_DEBUG, MODULE_TAG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) \
    do { \
            if (logfs_enabled && (LOG_DEBUG <= current_level)) { \
                printk(KERN_INFO "[%-5s][%-8s][%-8s] %s:%d: " fmt , \
                    log_level_str[LOG_DEBUG], global_tag, MODULE_TAG, __func__, __LINE__, ##__VA_ARGS__); \
            } \
        } while (0)

#endif

#endif /* _LOGFS_H */