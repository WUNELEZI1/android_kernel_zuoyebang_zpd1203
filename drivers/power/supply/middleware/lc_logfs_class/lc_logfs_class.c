#include "inc/lc_logfs_class.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/printk.h>

/* 全局变量 */
static struct kobject *lc_logfs_kobj = NULL;
char global_tag[] = "LC_CHARGER";
lc_log_level_t current_level = LOG_INFO; //LOG_DEBUG for debug
bool logfs_enabled = true;
EXPORT_SYMBOL(logfs_enabled);
EXPORT_SYMBOL(current_level);
EXPORT_SYMBOL(global_tag);

/* 日志等级字符串映射 */
const char *log_level_str[] = {
    "ERROR",
    "INFO",
    "DEBUG",
};
EXPORT_SYMBOL(log_level_str);


/* sysfs节点读写操作函数 */

/**
 * @brief 读取全局日志开关状态
 */
static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", logfs_enabled);
}

/**
 * @brief 设置全局日志开关状态
 */
static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t count) {
    int value;
    int ret;

    ret = kstrtoint(buf, 10, &value);
    if (ret < 0) {
        pr_err("Invalid input: not an integer\n");
        return ret; // 返回错误码（如 -EINVAL）
    }
    logfs_enabled = !!value;
    pr_err("%s %s set logfs_enabled %d \n",global_tag, __func__, logfs_enabled);
    return count;
}

/**
 * @brief 读取当前日志等级
 */
static ssize_t log_level_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    int i;
    int size = 0;
    for(i=0; i < ARRAY_SIZE(log_level_str); i++) {
        if(i==current_level) {
            size += sprintf((buf+size), " [%s]", log_level_str[i]);
        } else {
            size += sprintf((buf+size), " %s", log_level_str[i]);
        }
    }
    size += sprintf((buf+size), "%s", "\n");
    return size;
}

/**
 * @brief 设置新的日志等级
 */
static ssize_t log_level_store(struct kobject *kobj, struct kobj_attribute *attr,
                               const char *buf, size_t count) {
    int ret = -EINVAL;
    char *end;
    long val;
    int i;

    /* 尝试解析为数字 */
    val = simple_strtol(buf, &end, 10);
    if (*end == '\n' && end != buf && val >= LOG_ERROR && val <= LOG_DEBUG) {
        current_level = (lc_log_level_t)val;
        ret = count;
    }

    /* 尝试解析为字符串 */
    for (i = 0; i < ARRAY_SIZE(log_level_str); i++) {
        if (strncasecmp(buf, log_level_str[i], strlen(log_level_str[i])) == 0) {
            current_level = (lc_log_level_t)i;
            ret = count;
        }
    }
    if(ret == count){
        pr_err("%s %s set log level to %d \n",global_tag, __func__, current_level);
    }

    return ret;
}

/* sysfs节点属性结构体 */
static struct kobj_attribute log_enable_attr =
    __ATTR(enable, 0644, enable_show, enable_store);

static struct kobj_attribute log_level_attr =
    __ATTR(log_level, 0644, log_level_show, log_level_store);


int __init lc_logfs_init(void) {
    printk(KERN_INFO "[%s] %s enter.\n", global_tag, __func__);
    lc_logfs_kobj = kobject_create_and_add("lc_logfs", kernel_kobj);
    if (!lc_logfs_kobj) {
        printk(KERN_INFO "%s create kobject fail\n", __func__);
        return -ENOMEM;
    }
    /* 创建sysfs节点 */
    // sysfs node path: /sys/kernel/lc_logfs/*
    if(sysfs_create_file(lc_logfs_kobj, &log_enable_attr.attr)){
        printk(KERN_INFO "[%s] %s create log_enable node error!\n", global_tag, __func__);
    }
    if(sysfs_create_file(lc_logfs_kobj, &log_level_attr.attr)){
        printk(KERN_INFO "[%s] %s create log_level node error!\n", global_tag, __func__);
    }
    return 0;
}


void __exit lc_logfs_exit(void) {
    printk(KERN_INFO "[%s] %s enter.\n", global_tag, __func__);
    if(lc_logfs_kobj){
        sysfs_remove_file(lc_logfs_kobj, &log_enable_attr.attr);
        sysfs_remove_file(lc_logfs_kobj, &log_level_attr.attr);
        kobject_put(lc_logfs_kobj);
    }
}

module_init(lc_logfs_init);
module_exit(lc_logfs_exit);

/* 模块描述 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Longcheer");
MODULE_DESCRIPTION("A flexible and configurable logging driver");