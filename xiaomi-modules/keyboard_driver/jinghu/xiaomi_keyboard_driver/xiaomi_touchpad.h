#ifndef _XIAOMI_TOUCHPAD_H_
#define _XIAOMI_TOUCHPAD_H_

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
#include <linux/types.h>
#include "xiaomi_keyboard_evdev.h"


#define TOUCHPAD_PACKAGE_LENGTH 27
#define MAX_POINTS_IN_PACKAGE 3

// (x0-x)^2 + (y0-y)^2
#define MY_SQUARE(x) ((long)(x) * (long)(x))
#define MY_DISTANCE(x_0 , y_0, x, y) (MY_SQUARE((x_0 - x)) + MY_SQUARE((y_0 - y)))
#define MY_ABS(num) ((num < 0) ? (-num) : (num))


struct touchpad_point_hid_raw
{
    bool tip_switch : 1;
    bool in_range : 1;
    bool touch_valid : 1;
    unsigned char : 5;
    unsigned char contact_identifier;
    unsigned short tip_pressure;
    unsigned short pos_x;
    unsigned short pos_y;
};

union touchpad_report_hid_raw
{
    struct
    {
        /* 2 byte */
        unsigned char report_id;
        bool button_1 : 1;
        bool button_2 : 1;
        unsigned char : 6;
        /* 8 byte * 3 */
        struct touchpad_point_hid_raw point_hid_raw[3];
        /* 2 byte */
        unsigned char contact_count;
    };
    unsigned char buf[28];
};


#define STRING_MAX_LENGTH 128
extern const char param_list[][STRING_MAX_LENGTH];
extern const int param_list_string_count;
extern const char string_point_state[][STRING_MAX_LENGTH];
extern const char string_slot_state[][STRING_MAX_LENGTH];
extern const char string_gesture_type[][STRING_MAX_LENGTH];

enum point_states
{
    POINT_STATE_INIT = 0,
    POINT_STATE_DOWN = 1,
    POINT_STATE_MOVE = 2,
    POINT_STATE_UP = 3
};

enum slot_states
{
    SLOT_STATE_INIT = 0,
    SLOT_STATE_RELEASE = 1,
    SLOT_STATE_NORMAL = 2,
    SLOT_STATE_EDGE_ACTIVE = 3,
    SLOT_STATE_EDGE_SHIELD = 4
};

enum gesture_type
{
    GESTURE_TYPE_NONE = 0,
    GESTURE_TYPE_ONE_SWIPE = 1,
    GESTURE_TYPE_TWO_SWIPE = 2,
    GESTURE_TYPE_TWO_PINCH = 3,
    GESTURE_TYPE_THREE_SWIPE = 4
};

struct touchpad_slot
{
   int point_should_delete_count;
   long edge_point_move_distance;
   enum point_states point_state;
   enum slot_states slot_state;
   bool slot_info_need_report;
   struct touchpad_point_hid_raw point;
   struct touchpad_point_hid_raw first_edge_point;
};

struct touchpad_edge_filter_param
{
    u32 edge_area_left_min;
    u32 edge_area_left_max;
    u32 edge_area_right_min;
    u32 edge_area_right_max;
    u32 edge_area_top_min;
    u32 edge_area_top_max;
    u32 edge_area_bottom_min;
    u32 edge_area_bottom_max;
    u32 shield_frame_count_limit;
    u32 one_swipe_dis_limit;
    u32 two_swipe_dis_limit;
    u32 two_pinch_dis_limit;
    u32 three_swipe_dis_limit;
    u32 two_fingers_gesture_dis;
};


struct xiaomi_touchpad_pdata
{
    struct class *class;
    dev_t dev_num;
    struct device *dev;
    struct attribute_group attrs;

    bool edge_filter_enable;
    bool cdgain_robot_test_mode;
    struct touchpad_slot slots[MAX_POINTS_IN_PACKAGE];
    struct mutex edge_filter_mutex;
    struct touchpad_edge_filter_param edge_filter_param;

    struct spinlock last_touchpad_events_lock;
    struct proc_dir_entry  *last_touchpad_events_proc;
    struct last_touchpad_event last_touchpad_events;
};

extern int xiaomi_touchpad_edge_filter_param_init(struct device_node *np);

extern int xiaomi_touchpad_edge_filter(char *buf);
extern int xiaomi_touchpad_init(struct class *class, dev_t dev_num, struct xiaomi_touchpad_pdata *touchpad_pdata);
extern void xiaomi_touchpad_deinit(struct xiaomi_touchpad_pdata *pdata);


#endif