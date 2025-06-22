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
#include "xiaomi_keyboard.h"
#include "xiaomi_keyboard_macro.h"

extern inline struct xiaomi_keyboard_pdata *get_keyboard_pdata(void);
extern inline struct xiaomi_touchpad_pdata *get_touchpad_pdata(void);
extern inline struct xiaomi_keypad_pdata *get_keypad_pdata(void);

/*********************
 * keypad event show *
 *********************/

static void *keypad_events_start(struct seq_file *m, loff_t *p)
{
    struct xiaomi_keypad_pdata *pdata = get_keypad_pdata();
    struct last_keypad_event *events;
    int pos = 0;
    void *ret;

    // MI_KB_LOG("start *p = %d", *p);

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("xiaomi_keypad_pdata err!");
        ret = NULL;
        goto err_pdata;
    }

    spin_lock(&pdata->last_keypad_events_lock);

    events = &pdata->last_keypad_events;

    if (*p >= LAST_KEYPAD_EVENTS_MAX)
    {
        // MI_KB_LOG("*p overflow, *p = %d", *p);
        ret = NULL;
        goto overflow;
    }


    pos = (events->head + *p) & (LAST_KEYPAD_EVENTS_MAX - 1);
    ret = &events->keypad_event_buf[pos];

overflow:
    spin_unlock(&pdata->last_keypad_events_lock);
err_pdata:
    return ret;
}

static void *keypad_events_next(struct seq_file *m, void *v, loff_t *p)
{
    struct xiaomi_keypad_pdata *pdata = get_keypad_pdata();
    struct last_keypad_event *events;
    int pos = 0;
    void *ret;

    // MI_KB_LOG("next *p = %d", *p);

    if (IS_ERR_OR_NULL(pdata))
    {
        // MI_KB_ERR("xiaomi_keypad_pdata err!");
        ret = NULL;
        goto err_pdata;
    }

    spin_lock(&pdata->last_keypad_events_lock);

    events = &pdata->last_keypad_events;

    if (++*p >= LAST_KEYPAD_EVENTS_MAX)
    {
        // MI_KB_LOG("*p overflow, *p = %d", *p);
        ret = NULL;
        goto overflow;
    }

    pos = (events->head + *p) & (LAST_KEYPAD_EVENTS_MAX - 1);
    ret = &events->keypad_event_buf[pos];

overflow:
    spin_unlock(&pdata->last_keypad_events_lock);
err_pdata:
    return ret;
}

static int32_t keypad_events_show(struct seq_file *m, void *v)
{
    struct xiaomi_keypad_pdata *pdata = get_keypad_pdata();
    struct keypad_event *event_buf = (struct keypad_event *)v;
    struct rtc_time tm;
    int32_t ret = 0;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("xiaomi_keypad_pdata err!");
        ret = -EINVAL;
        goto err_pdata;
    }

    // MI_KB_LOG("show");

    spin_lock(&pdata->last_keypad_events_lock);

    if (IS_ERR_OR_NULL(event_buf->input_dev_name))
    {
        ret = 0;
        goto skip_invalid_event;
    }

    rtc_time64_to_tm(event_buf->event_time.tv_sec, &tm);
    /* time device_name keycode keyvalue */
    seq_printf(m, "%d-%02d-%02d %02d:%02d:%02d.%09lu UTC Input device:[%s] Keycode:[%d] Keyvalue:[%d]\n",
               tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
               tm.tm_hour, tm.tm_min, tm.tm_sec, event_buf->event_time.tv_nsec,
               event_buf->input_dev_name, event_buf->keycode, event_buf->keyvalue);

skip_invalid_event:
    spin_unlock(&pdata->last_keypad_events_lock);
err_pdata:
    return ret;
}

static void keypad_events_stop(struct seq_file *m, void *v)
{
    // MI_KB_LOG("stop");
    return;
}

const struct seq_operations last_keypad_events_seq_ops = {
    .start = keypad_events_start,
    .next = keypad_events_next,
    .stop = keypad_events_stop,
    .show = keypad_events_show,
};

/*********************
 * touchpad event show *
 *********************/
static void *touchpad_events_start(struct seq_file *m, loff_t *p)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    struct last_touchpad_event *events;
    int pos = 0;
    void *ret;

    // MI_KB_LOG("start *p = %d", *p);

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("xiaomi_touchpad_pdata err!");
        ret = NULL;
        goto err_pdata;
    }

    spin_lock(&pdata->last_touchpad_events_lock);

    events = &pdata->last_touchpad_events;

    if (*p >= LAST_TOUCHPAD_EVENTS_MAX)
    {
        // MI_KB_LOG("*p overflow, *p = %d", *p);
        ret = NULL;
        goto overflow;
    }


    pos = (events->head + *p) & (LAST_TOUCHPAD_EVENTS_MAX - 1);
    ret = &events->touchpad_event_buf[pos];

overflow:
    spin_unlock(&pdata->last_touchpad_events_lock);
err_pdata:
    return ret;
}

static void *touchpad_events_next(struct seq_file *m, void *v, loff_t *p)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    struct last_touchpad_event *events;
    int pos = 0;
    void *ret;

    // MI_KB_LOG("next *p = %d", *p);

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("xiaomi_touchpad_pdata err!");
        ret = NULL;
        goto err_pdata;
    }

    spin_lock(&pdata->last_touchpad_events_lock);

    events = &pdata->last_touchpad_events;

    if (++*p >= LAST_TOUCHPAD_EVENTS_MAX)
    {
        // MI_KB_LOG("*p overflow, *p = %d", *p);
        ret = NULL;
        goto overflow;
    }

    pos = (events->head + *p) & (LAST_TOUCHPAD_EVENTS_MAX - 1);
    ret = &events->touchpad_event_buf[pos];

overflow:
    spin_unlock(&pdata->last_touchpad_events_lock);
err_pdata:
    return ret;
}

static int32_t touchpad_events_show(struct seq_file *m, void *v)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    struct touchpad_event *event_buf = (struct touchpad_event *)v;
    struct rtc_time tm;
    int32_t ret = 0;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("xiaomi_touchpad_pdata err!");
        ret = -EINVAL;
        goto err_pdata;
    }

    // MI_KB_LOG("show");

    spin_lock(&pdata->last_touchpad_events_lock);

    if (IS_ERR_OR_NULL(event_buf->input_dev_name))
    {
        ret = 0;
        goto skip_invalid_event;
    }

    rtc_time64_to_tm(event_buf->event_time.tv_sec, &tm);
    /* time device_name slot contact_state tool_type button_state trcking_id */
    seq_printf(m, "%d-%02d-%02d %02d:%02d:%02d.%09lu UTC Input device:[%s] slot:[%d] contact_state:[%s] tool_type:[%d] button_state:[%s] id:[%d]\n",
               tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
               tm.tm_hour, tm.tm_min, tm.tm_sec, event_buf->event_time.tv_nsec,
               event_buf->input_dev_name, event_buf->slot_num, event_buf->contact_state ? "P" : "R",
               event_buf->tool_type, event_buf->button_state ? "P" : "R", event_buf->trcking_id_num);

skip_invalid_event:
    spin_unlock(&pdata->last_touchpad_events_lock);
err_pdata:
    return ret;
}

static void touchpad_events_stop(struct seq_file *m, void *v)
{
    // MI_KB_LOG("stop");
    return;
}

const struct seq_operations last_touchpad_events_seq_ops = {
    .start = touchpad_events_start,
    .next = touchpad_events_next,
    .stop = touchpad_events_stop,
    .show = touchpad_events_show,
};

/*****************
 * event collect *
 *****************/

/**
 * @brief keypad event filter
 *
 * @input_dev_name: the name of input device
 * @type: type of the event
 * @code: event code
 * @value: value of the event
 * @return true when got event or false when fail to get event
 */
bool last_keypad_event_filter(char *input_dev_name, unsigned int type, unsigned int code, int value)
{
    struct xiaomi_keypad_pdata *pdata = get_keypad_pdata();
    struct last_keypad_event *events;
    bool ret = false;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("pdata err!");
        goto err_pdata;
    }

    spin_lock(&pdata->last_keypad_events_lock);

    events = &pdata->last_keypad_events;

    if (type == EV_KEY)
    {
        events->keypad_event_buf[events->head].input_dev_name = input_dev_name;
        events->keypad_event_buf[events->head].keycode = code;
        events->keypad_event_buf[events->head].keyvalue = value;
        ktime_get_real_ts64(&events->keypad_event_buf[events->head].event_time);
        events->head++;
        events->head &= LAST_KEYPAD_EVENTS_MAX - 1;
        ret = true;
    }

    spin_unlock(&pdata->last_keypad_events_lock);
err_pdata:

    return ret;
}

/**
 * @brief touchpad event filter
 *
 * @input_dev_name: the name of input device
 * @type: type of the event
 * @code: event code
 * @value: value of the event
 * @return true when got event or false when fail to get event
 */
#define TOUCHPAD_CONTACT_MAX 3
bool last_touchpad_event_filter(char *input_dev_name, unsigned int type, unsigned int code, int value)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    struct last_touchpad_event *events;
    bool ret = false;
    static int slot_num;
    static int trcking_id_num[TOUCHPAD_CONTACT_MAX];
    static bool button_state;
    static bool contact_state[TOUCHPAD_CONTACT_MAX];
    static int contact_tool_type[TOUCHPAD_CONTACT_MAX];
    static bool event_need_refresh = false; // need to construct a new event
    static bool contact_info_changed = false;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("pdata err!");
        goto err_pdata;
    }

    spin_lock(&pdata->last_touchpad_events_lock);
    events = &pdata->last_touchpad_events;

    /* button state */
    if (type == EV_KEY && code == BTN_MOUSE)
    {
        button_state = value;
        event_need_refresh = true;
    }

    if (type == EV_ABS)
    {
        /* switch slot */
        if (code == ABS_MT_SLOT)
        {
            // Switch Slot in one SYN_REPORT Package
            // Special Case: ABS_MT_TRACKING_ID ABS_MT_SLOT ABS_MT_TRACKING_ID SYN_REPORT
            // And we need record last TrackingID state now
            if(contact_info_changed == true)
            {
                events->touchpad_event_buf[events->head].input_dev_name = input_dev_name;
                events->touchpad_event_buf[events->head].slot_num = slot_num;
                events->touchpad_event_buf[events->head].trcking_id_num = trcking_id_num[slot_num];
                events->touchpad_event_buf[events->head].button_state = button_state;
                events->touchpad_event_buf[events->head].tool_type = contact_tool_type[slot_num];
                events->touchpad_event_buf[events->head].contact_state = contact_state[slot_num];

                ktime_get_real_ts64(&events->touchpad_event_buf[events->head].event_time);

                events->head++;
                events->head &= LAST_TOUCHPAD_EVENTS_MAX - 1;

                ret = true;
                event_need_refresh = false;
                contact_info_changed=false;

            }
            slot_num = value;
	}

        /* contact active or not */
        if (code == ABS_MT_TRACKING_ID)
        {
            trcking_id_num[slot_num] = value;
            contact_state[slot_num] = (value == -1 ? 0 : 1);
            contact_info_changed = true;
	}

        /* contact tool type */
        if (code == ABS_MT_TOOL_TYPE)
        {
            contact_tool_type[slot_num] = value;
            contact_info_changed = true;
            MI_KB_LOG("ABS_MT_TOOL_TYPE: %d", value);
        }
    }

    if (type == EV_SYN && code == SYN_REPORT)
    {
        if (contact_info_changed)
        {
            event_need_refresh = true;
            contact_info_changed = false;
        }
    }

    if (event_need_refresh)
    {
        events->touchpad_event_buf[events->head].input_dev_name = input_dev_name;
        events->touchpad_event_buf[events->head].slot_num = slot_num;
        events->touchpad_event_buf[events->head].trcking_id_num = trcking_id_num[slot_num];
        events->touchpad_event_buf[events->head].button_state = button_state;
        events->touchpad_event_buf[events->head].tool_type = contact_tool_type[slot_num];
        events->touchpad_event_buf[events->head].contact_state = contact_state[slot_num];

        ktime_get_real_ts64(&events->touchpad_event_buf[events->head].event_time);

        events->head++;
        events->head &= LAST_TOUCHPAD_EVENTS_MAX - 1;

        event_need_refresh = false;
        ret = true;
    }

    spin_unlock(&pdata->last_touchpad_events_lock);
err_pdata:

    return ret;
}

char input_device_filter_name_list[][128] =
{
    "Xiaomi Touch",     // touchpad
    "Xiaomi Consumer",  // keypad
    "Xiaomi Keyboard"   // keypad
};
const int name_list_count = ARRAY_SIZE(input_device_filter_name_list);

/* create handler */
static void xiaomi_keyboard_input_event(struct input_handle *handle,
        unsigned int type, unsigned int code, int value)
{
    char *input_dev_name = NULL;
    int i = 0;

    for (i = 0; i < name_list_count; i++)
    {
        if (sysfs_streq(handle->dev->name, input_device_filter_name_list[i]))
        {
            input_dev_name = input_device_filter_name_list[i];
        }
    }

    if (IS_ERR_OR_NULL(input_dev_name))
    {
        return;
    }

    if (sysfs_streq(input_dev_name, input_device_filter_name_list[0]))
    {
        last_touchpad_event_filter(input_dev_name, type, code, value);
    }
    else if (sysfs_streq(input_dev_name, input_device_filter_name_list[1]) ||
                sysfs_streq(input_dev_name, input_device_filter_name_list[2]))
    {
        last_keypad_event_filter(input_dev_name, type, code, value);
    }
}

static int xiaomi_keyboard_input_connect(struct input_handler *handler,
        struct input_dev *dev, const struct input_device_id *id)
{
    struct input_handle *handle;
    struct input_dev *match_input_dev = NULL;
    int ret = 0;
    int i;

    for (i = 0; i < name_list_count; i++)
    {
        if (sysfs_streq(dev->name, input_device_filter_name_list[i]))
        {
            MI_KB_LOG("match device: %s!", dev->name);
            match_input_dev = dev;
            break;
        }
    }

    if (IS_ERR_OR_NULL(match_input_dev))
    {
        MI_KB_LOG("not match device: %s", dev->name);
        ret = -EINVAL;
        goto err_not_match_any_device;
    }

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
    {
        ret = -ENOMEM;
        goto err_kzalloc;
    }

    handle->dev = match_input_dev;
    handle->handler = handler;
    handle->name = "xiaomi_keyboard";

    ret = input_register_handle(handle);
    if (ret)
        goto err_input_register_handle;

    ret = input_open_device(handle);
    if (ret)
        goto err_input_open_device;

    return ret;
err_input_open_device:
    input_unregister_handle(handle);
err_input_register_handle:
    kfree(handle);
    handle = NULL;
err_kzalloc:
err_not_match_any_device:
    return ret;
}

static void xiaomi_keyboard_input_disconnect(struct input_handle *handle)
{
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
    handle = NULL;
}

static const struct input_device_id xiaomi_keyboard_ids[] = {
    { .driver_info = 1 },	/* Matches all devices */
    { },
};

static struct input_handler xiaomi_keyboard_input_handler = {
    .event		= xiaomi_keyboard_input_event,
    .connect	= xiaomi_keyboard_input_connect,
    .disconnect	= xiaomi_keyboard_input_disconnect,
    .name		= "xiaomi_keyboard",
    .id_table	= xiaomi_keyboard_ids,
};

int xiaomi_keyboard_evdev_init(void)
{
    struct xiaomi_keyboard_pdata *pdata = get_keyboard_pdata();

    if (IS_ERR_OR_NULL(pdata))
        return -EINVAL;

    spin_lock_init(&pdata->keypad_pdata.last_keypad_events_lock);
    spin_lock_init(&pdata->touchpad_pdata.last_touchpad_events_lock);

    pdata->keypad_pdata.last_keypad_events_proc =
        proc_create_seq("last_keypad_events", 0644, NULL, &last_keypad_events_seq_ops);

    pdata->touchpad_pdata.last_touchpad_events_proc =
        proc_create_seq("last_touchpad_events", 0644, NULL, &last_touchpad_events_seq_ops);

    return input_register_handler(&xiaomi_keyboard_input_handler);
}

void xiaomi_keyboard_evdev_exit(void)
{
    struct xiaomi_keyboard_pdata *pdata = get_keyboard_pdata();

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("pdata err!");
        return;
    }

    remove_proc_entry("last_keypad_events", NULL);
    pdata->keypad_pdata.last_keypad_events_proc = NULL;

    remove_proc_entry("last_touchpad_events", NULL);
    pdata->touchpad_pdata.last_touchpad_events_proc = NULL;

    input_unregister_handler(&xiaomi_keyboard_input_handler);
}
