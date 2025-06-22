#ifndef _XIAOMI_KEYBOARD_H_
#define _XIAOMI_KEYBOARD_H_

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/rtc.h>
#include "xiaomi_touchpad.h"
#include "xiaomi_keypad.h"
#include "xiaomi_keyboard_evdev.h"

struct xiaomi_keyboard_pdata
{
    struct device *dev;
    struct class *class;
    struct xiaomi_touchpad_pdata touchpad_pdata;
    struct xiaomi_keypad_pdata keypad_pdata;
};


#endif