#ifndef __XIAOMI__TOUCH_H
#define __XIAOMI__TOUCH_H
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


#define MI_TAG  "[mi-touch]"

/*Xiaomi Touch driver log level
  *error    : 0
  *info     : 1
  *notice   : 2
  *debug    : 3
*/
extern int mi_log_level;

#define 	TOUCH_ERROR    0
#define 	TOUCH_INFO     1
#define 	TOUCH_NOTICE   2
#define 	TOUCH_DEBUG    3

#define XIAOMI_TOUCH_DEVICE_NAME "xiaomi-touch"
#define KEY_INPUT_DEVICE_PHYS "xiaomi-touch/input0"
/*Xiaomi Special Touch Event Code*/
#define BTN_TAP 0x153

#define MI_TOUCH_LOGD(level, fmt, args...) \
do { \
	if (mi_log_level == TOUCH_DEBUG && level == 1) \
		pr_info(fmt, ##args); \
} while (0)

#define MI_TOUCH_LOGN(level, fmt, args...) \
do { \
	if (mi_log_level >= TOUCH_NOTICE && level == 1) \
		pr_info(fmt, ##args); \
} while (0)

#define MI_TOUCH_LOGI(level, fmt, args...) \
do { \
	if (mi_log_level >= TOUCH_INFO && level == 1) \
		pr_info(fmt, ##args); \
} while (0)

#define MI_TOUCH_LOGE(level, fmt, args...) \
do { \
	if (level == 1) \
		pr_err(fmt, ##args); \
} while (0)


#define XIAOMI_ROI	1

#if XIAOMI_ROI
#define DIFF_SENSE_NODE 7
#define DIFF_FORCE_NODE 7

struct xiaomi_diff_data {
	u8 flag;
	u8 x;
	u8 y;
	u8 frame;
	s16 data[DIFF_SENSE_NODE * DIFF_FORCE_NODE];
};
#endif

/*CUR,DEFAULT,MIN,MAX*/
#define VALUE_TYPE_SIZE 6
#define VALUE_GRIP_SIZE 9
#define MAX_BUF_SIZE 256
enum MODE_CMD {
	SET_CUR_VALUE = 0,
	GET_CUR_VALUE,
	GET_DEF_VALUE,
	GET_MIN_VALUE,
	GET_MAX_VALUE,
	GET_MODE_VALUE,
	RESET_MODE,
	SET_LONG_VALUE,
};

enum MODE_TYPE {
	TOUCH_0 = 0,
	TOUCH_1 = 1,
	TOUCH_2 = 2,
	TOUCH_3 = 3,
	TOUCH_4	= 4,
	TOUCH_5	= 5,
	TOUCH_6	= 6,
	TOUCH_7 = 7,
	TOUCH_8 = 8,
	TOUCH_9 = 9,
	TOUCH_10 = 10,
	TOUCH_11 = 11,
	TOUCH_12 = 12,
	TOUCH_13 = 13,
	TOUCH_14 = 14,
	TOUCH_15 = 15,
	TOUCH_16 = 16,
	TOUCH_17 = 17,
	TOUCH_18 = 18,
	TOUCH_19 = 19,
	TOUCH_20 = 20,
	TOUCH_21 = 21,
	TOUCH_22 = 22,
	TOUCH_23 = 23,
	TOUCH_24 = 24,
	Touch_Mode_NUM = 25,
};

struct xiaomi_touch_interface {
	int touch_mode[Touch_Mode_NUM][VALUE_TYPE_SIZE];
	int (*setModeValue)(int Mode, int value);
	int (*setModeLongValue)(int Mode, int value_len, int *value);
	int (*getModeValue)(int Mode, int value_type);
	int (*getModeAll)(int Mode, int *modevalue);
	int (*resetMode)(int Mode);
	int (*p_sensor_read)(void);
	int (*p_sensor_write)(int on);
	int (*palm_sensor_read)(void);
	int (*palm_sensor_write)(int on);
	u8 (*panel_vendor_read)(void);
	u8 (*panel_color_read)(void);
	u8 (*panel_display_read)(void);
	char (*touch_vendor_read)(void);
	int (*get_touch_super_resolution_factor)(void);
#if XIAOMI_ROI
	int (*partial_diff_data_read)(struct xiaomi_diff_data *data);
#endif
	int long_mode_len;
	int long_mode_value[MAX_BUF_SIZE];
};

struct xiaomi_touch {
	struct miscdevice 	misc_dev;
	struct input_dev *key_input_dev;
	struct device *dev;
	struct class *class;
	struct attribute_group attrs;
	struct mutex  mutex;
	struct mutex  palm_mutex;
	struct mutex  psensor_mutex;
#ifdef CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY
	struct mutex pen_connect_strategy_mutex;
#endif // CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY
	wait_queue_head_t 	wait_queue;
};

/* dump last touch events setup */
#define MAX_TOUCH_ID 12
#define PEN_HOVER_ID MAX_TOUCH_ID - 2
#define PEN_INK_ID   MAX_TOUCH_ID - 1	  //the last one is ink id of pen 
#define LAST_TOUCH_EVENTS_MAX 512

enum touch_state {
	EVENT_INIT,
	EVENT_DOWN,
	EVENT_UP,
};

struct touch_event {
	u32 slot;
	enum touch_state state;
	struct timespec64 touch_time;
};

struct last_touch_event {
	int head;
	struct touch_event touch_event_buf[LAST_TOUCH_EVENTS_MAX];
};
/* dump last touch events setup end */

struct xiaomi_touch_pdata{
	struct xiaomi_touch *device;
	struct xiaomi_touch_interface *touch_data;
	int palm_value;
	bool palm_changed;
	int psensor_value;
	bool psensor_changed;
	const char *name;
	u8 debug_log;
#if XIAOMI_ROI
	struct xiaomi_diff_data *diff_data;
	bool debug_roi_flag;
#endif
	/* dump last touch events setup */
	struct proc_dir_entry  *last_touch_events_proc;
	struct last_touch_event *last_touch_events;
	/* dump last touch events setup end */
#ifdef CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY
	bool pen_active;
#endif // CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY
};

struct xiaomi_touch *xiaomi_touch_dev_get(int minor);

extern struct class *get_xiaomi_touch_class(void);

extern struct device *get_xiaomi_touch_dev(void);

extern int update_palm_sensor_value(int value);

extern int update_p_sensor_value(int value);

int xiaomitouch_register_modedata(struct xiaomi_touch_interface *data);

void last_touch_events_collect(int slot, int state);

#ifdef CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY
int update_pen_connect_strategy_value(bool pen_active);
#endif //CONFIG_TOUCHSCREEN_NEW_PEN_CONNECT_STRATEGY

#endif
