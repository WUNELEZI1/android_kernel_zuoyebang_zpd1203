#ifndef _XIAOMI_KEYBOARD_EVDEV_H_
#define _XIAOMI_KEYBOARD_EVDEV_H_


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/time.h>
#include <linux/sysfs.h>
#include <linux/rtc.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

struct keypad_event
{
    char *input_dev_name;
    unsigned int keycode;
    int keyvalue;
    struct timespec64 event_time;
};

#define LAST_KEYPAD_EVENTS_MAX 1024
struct last_keypad_event
{
    int head;
    struct keypad_event keypad_event_buf[LAST_KEYPAD_EVENTS_MAX];
};

enum contact_state {
    STATE_INIT = 0,
    STATE_FINGER_DOWN = 1,
    STATE_PALM_DOWN = 2,
    STATE_UP = 3
};

struct touchpad_event
{
    char *input_dev_name;
    int slot_num;
    int trcking_id_num;
    bool contact_state;
    int tool_type;
    bool button_state;
    struct timespec64 event_time;
};

#define LAST_TOUCHPAD_EVENTS_MAX 1024
struct last_touchpad_event
{
    int head;
    struct touchpad_event touchpad_event_buf[LAST_TOUCHPAD_EVENTS_MAX];
};

extern int xiaomi_keyboard_evdev_init(void);
extern void xiaomi_keyboard_evdev_exit(void);

extern const struct seq_operations last_keypad_events_seq_ops;

#endif
