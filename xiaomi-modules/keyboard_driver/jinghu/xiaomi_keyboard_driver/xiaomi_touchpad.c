#include "xiaomi_keyboard_macro.h"
#include "xiaomi_touchpad.h"
#include "xiaomi_touchpad_sysfs.h"

extern inline struct xiaomi_touchpad_pdata *get_touchpad_pdata(void);

const char param_list[][STRING_MAX_LENGTH] = {
    "touchpad,edge-area-left-min",
    "touchpad,edge-area-left-max",
    "touchpad,edge-area-right-min",
    "touchpad,edge-area-right-max",
    "touchpad,edge-area-top-min",
    "touchpad,edge-area-top-max",
    "touchpad,edge-area-bottom-min",
    "touchpad,edge-area-bottom-max",
    "touchpad,shield-frame-count-limit",
    "touchpad,one-swipe-dis-limit",
    "touchpad,two-swipe-dis-limit",
    "touchpad,two-pinch-dis-limit",
    "touchpad,three-swipe-dis-limit",
    "touchpad,two-fingers-gesture-dis"
};
const int param_list_string_count = ARRAY_SIZE(param_list);

const char string_slot_state[][STRING_MAX_LENGTH] = {
    "INIT",
    "RELEASE",
    "NORMAL",
    "EDGE_ACTIVE",
    "EDGE_SHIELD"
};

const char string_point_state[][STRING_MAX_LENGTH] = {
    "INIT",
    "DOWN",
    "MOVE",
    "UP"
};

const char string_gesture_type[][STRING_MAX_LENGTH] = {
    "NONE",
    "ONE_SWIPE",
    "TWO_SWIPE",
    "TWO_PINCH",
    "THREE_SWIPE"
};

int xiaomi_touchpad_edge_filter_param_init(struct device_node *np)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    struct touchpad_edge_filter_param *param;
    int ret = 0;
    u32 value;
    int i;

    if (IS_ERR_OR_NULL(pdata))
    {
        ret = -ENODEV;
        MI_KB_ERR("pdata ERROR");
        return ret;
    }

    param = &pdata->edge_filter_param;

    if (IS_ERR_OR_NULL(np))
    {
        ret = -EINVAL;
        MI_KB_ERR("np ERROR");
        return ret;
    }

    mutex_lock(&pdata->edge_filter_mutex);

    for (i = 0; i < param_list_string_count; i++)
    {
        if (of_property_read_u32(np, param_list[i], &value) == 0)
        {
            *((u32 *)param + i) = value;
            MI_KB_LOG("%s: %d", param_list[i], value);
        }
        else
        {
            MI_KB_ERR("Failed to read %s from device tree\n", param_list[i]);
            ret = -EINVAL;
            goto out;
        }
    }

    pdata->edge_filter_enable = 1;
    pdata->cdgain_robot_test_mode = 0;
    MI_KB_LOG("xiaomi_touchpad_edge_filter_param_init success!");

out:
    mutex_unlock(&pdata->edge_filter_mutex);

    return ret;
}
EXPORT_SYMBOL_GPL(xiaomi_touchpad_edge_filter_param_init);


int xiaomi_touchpad_init(struct class *class, dev_t dev_num, struct xiaomi_touchpad_pdata *pdata)
{
    int ret = 0;

    pdata->class = class;
    pdata->dev_num = dev_num;
    pdata->dev = device_create(pdata->class, NULL, dev_num, pdata, "touchpad");
    if (IS_ERR_OR_NULL(pdata->dev))
    {
        ret = -ENODEV;
        goto err_touchpad_create;
    }

    ret = xiaomi_touchpad_sysfs_init(pdata);
    if (ret)
    {
        MI_KB_ERR("xiaomi_touchpad_sysfs_init fail! ret = %d", ret);
        goto err_xiaomi_touchpad_sysfs_init;
    }

    mutex_init(&pdata->edge_filter_mutex);

    MI_KB_LOG("xiaomi_touchpad_init success!");

    return 0;

    xiaomi_touchpad_sysfs_deinit(pdata);
err_xiaomi_touchpad_sysfs_init:
    device_destroy(pdata->class, pdata->dev_num);
err_touchpad_create:
    return ret;
}

void xiaomi_touchpad_deinit(struct xiaomi_touchpad_pdata *pdata)
{
    xiaomi_touchpad_sysfs_deinit(pdata);
    device_destroy(pdata->class, pdata->dev_num);

    MI_KB_LOG("xiaomi_touchpad_sysfs_deinit success!");
}

static inline int is_gesture_slot_in_package(struct touchpad_slot *this_slot, struct touchpad_edge_filter_param *param)
{
    int is_gesture_slot = 0;

    if (this_slot->slot_state == SLOT_STATE_EDGE_ACTIVE)
    {
        is_gesture_slot = 1;
        goto exit;
    }

exit:
    return is_gesture_slot;
}

static enum gesture_type find_gesture_type(union touchpad_report_hid_raw *report_package, struct touchpad_slot *slots, struct touchpad_edge_filter_param *param)
{
    int gesture_slot_index[MAX_POINTS_IN_PACKAGE];
    long dis_between_two_fingers = 0;
    int gesture_slots_count = 0;
    enum gesture_type type = GESTURE_TYPE_NONE;
    int slot_index = 0;
    u32 dis_limit = 0;
    int i = 0;

    /* search and mark gesture_slot */
    for (i = 0; i < MAX_POINTS_IN_PACKAGE; i++)
    {
        if (slots[i].slot_state != SLOT_STATE_EDGE_ACTIVE &&
                slots[i].slot_state != SLOT_STATE_NORMAL)
        {
            continue;
        }
        gesture_slot_index[gesture_slots_count] = i;
        // MI_KB_DEBUG("mark gesture slot: %d, slot state: %s", i, string_slot_state[slots[i].slot_state]);
        gesture_slots_count++;
    }

    if (gesture_slots_count == 0)
    {
        type = GESTURE_TYPE_NONE;
        goto exit;
    }
    else if (gesture_slots_count == 1)
    {
        type = GESTURE_TYPE_ONE_SWIPE;
        dis_limit = param->one_swipe_dis_limit;
    }
    else if (gesture_slots_count == 2)
    {
        /* two fingers distance */
        dis_between_two_fingers =
            MY_DISTANCE(report_package->point_hid_raw[gesture_slot_index[0]].pos_x,
                        report_package->point_hid_raw[gesture_slot_index[0]].pos_y,
                        report_package->point_hid_raw[gesture_slot_index[1]].pos_x,
                        report_package->point_hid_raw[gesture_slot_index[1]].pos_y);
        if (dis_between_two_fingers >= param->two_fingers_gesture_dis)
        {
            type = GESTURE_TYPE_TWO_PINCH;
            dis_limit = param->two_pinch_dis_limit;
        }
        else
        {
            type = GESTURE_TYPE_TWO_SWIPE;
            dis_limit = param->two_swipe_dis_limit;
        }
    }
    else if (gesture_slots_count == 3)
    {
        type = GESTURE_TYPE_THREE_SWIPE;
        dis_limit = param->three_swipe_dis_limit;
    }

    /* search slot in roi */
    for (i = 0; i < gesture_slots_count; i++)
    {
        slot_index = gesture_slot_index[i];
        /* only compute swipe distance in edge area */
        if (slots[slot_index].slot_state == SLOT_STATE_EDGE_ACTIVE)
        {
            /* compute swipe distance */
            slots[slot_index].edge_point_move_distance =
                MY_DISTANCE(slots[slot_index].first_edge_point.pos_x,
                            slots[slot_index].first_edge_point.pos_y,
                            slots[slot_index].point.pos_x,
                            slots[slot_index].point.pos_y);

            // MI_KB_DEBUG("slot:%d dis: %ld", slot_index, slots[slot_index].edge_point_move_distance);
            if (MY_ABS(slots[slot_index].edge_point_move_distance) <= dis_limit)
            {
                type = GESTURE_TYPE_NONE;
                goto exit;
            }
        }
    }


exit:
    return type;

}

static inline int is_point_in_edge_area(int x, int y, struct touchpad_edge_filter_param *param, bool cdgain_robot_test_mode)
{
    /*
     * In cdgain_robot_test_mode
     * the width of the suppression area is 0
     */
    if (cdgain_robot_test_mode)
        return 0;
    return (x >= param->edge_area_left_min && x <= param->edge_area_left_max) ||
           (x >= param->edge_area_right_min && x <= param->edge_area_right_max) ||
           (y >= param->edge_area_top_min && y <= param->edge_area_top_max) ||
           (y >= param->edge_area_bottom_min && y <= param->edge_area_bottom_max);
}

static inline void clear_slot_except_point(struct touchpad_slot *slot)
{
    slot->point_should_delete_count = 0;
    slot->edge_point_move_distance = 0;
    memset(&slot->first_edge_point, 0, sizeof(struct touchpad_point_hid_raw));
}

static void pack_hid_data(union touchpad_report_hid_raw *report_package, struct touchpad_slot *slots, unsigned char button_state)
{
    int i = 0;

    memset(report_package->buf, 0, TOUCHPAD_PACKAGE_LENGTH);
    report_package->report_id = 0x19;
    report_package->buf[1] = button_state;
    for (i = 0; i < MAX_POINTS_IN_PACKAGE; i++)
    {
        if (slots[i].slot_info_need_report && slots[i].point.contact_identifier <= 2){
            memcpy(&report_package->point_hid_raw[report_package->contact_count], &slots[i].point, sizeof(struct touchpad_point_hid_raw));
            report_package->point_hid_raw[report_package->contact_count].contact_identifier = i;
            report_package->contact_count++;
            slots[i].slot_info_need_report = 0;
        }
    }
}

static inline bool is_point_init(struct touchpad_point_hid_raw *point)
{
    return (point->tip_switch == 0) &&
           (point->pos_x == 0) &&
           (point->pos_y == 0);
}

static inline bool is_button_valid(int button_state)
{
    return (button_state != 0);
}

/**
 * @brief remove the points that need to be filtered in buf
 *        according to the edge suppression strategy
 *
 * @param buf: hid raw data
 * @return the number of points if successful otherwise an
 *         error code
 */
int xiaomi_touchpad_edge_filter(char *buf)
{
    struct xiaomi_touchpad_pdata *pdata = get_touchpad_pdata();
    union touchpad_report_hid_raw report_package = {0};
    struct touchpad_edge_filter_param *param = NULL;
    enum gesture_type package_gesture_type = GESTURE_TYPE_NONE;
    int valid_points_in_package_count = 0;
    unsigned char button_state = 0;
    struct touchpad_slot *slots;
    int slot_index = 0;
    int ret = 0;
    int i = 0;

    if (IS_ERR_OR_NULL(pdata))
    {
        MI_KB_ERR("pdata is not ready!");
        return -EINVAL;
    }
    else
    {
        param = &pdata->edge_filter_param;
        slots = pdata->slots;
    }

    // printk("Joseph timestamp ++");
    mutex_lock(&pdata->edge_filter_mutex);

    if(!pdata->edge_filter_enable)
    {
        ret = -EINVAL;
        goto out;
    }

    /* unpack touchpad HID data  */
    memcpy(report_package.buf, buf, TOUCHPAD_PACKAGE_LENGTH);

    rawdata_show("touchpad report++", report_package.buf, TOUCHPAD_PACKAGE_LENGTH);

    /* parse package */
    button_state = report_package.buf[1];
    /* cdgain_robot_test_mode should disable button */
    if (pdata->cdgain_robot_test_mode)
        button_state = 0;
    valid_points_in_package_count = report_package.contact_count;
    for (i = 0; i < valid_points_in_package_count; i++)
    {
        slot_index = report_package.point_hid_raw[i].contact_identifier;

        /* store point */
        memcpy(&slots[slot_index].point, &report_package.point_hid_raw[i], sizeof(struct touchpad_point_hid_raw));

        /* point state */
        switch (slots[slot_index].point_state)
        {
        case POINT_STATE_INIT:
            slots[slot_index].point_state = report_package.point_hid_raw[i].tip_switch ? POINT_STATE_DOWN : POINT_STATE_INIT;

            if (slots[slot_index].point_state != POINT_STATE_INIT)
                MI_KB_DEBUG("slot: %d, point_state: %s",
                    slot_index, string_point_state[slots[slot_index].point_state]);

            break;
        case POINT_STATE_DOWN:
            slots[slot_index].point_state = report_package.point_hid_raw[i].tip_switch ? POINT_STATE_MOVE : POINT_STATE_UP;

            if (slots[slot_index].point_state != POINT_STATE_DOWN)
                MI_KB_DEBUG("slot: %d, point_state: %s",
                    slot_index, string_point_state[slots[slot_index].point_state]);

            break;

        case POINT_STATE_MOVE:
            slots[slot_index].point_state = report_package.point_hid_raw[i].tip_switch ? POINT_STATE_MOVE : POINT_STATE_UP;

            if (slots[slot_index].point_state != POINT_STATE_MOVE)
                MI_KB_DEBUG("slot: %d, point_state: %s",
                    slot_index, string_point_state[slots[slot_index].point_state]);

            break;

        case POINT_STATE_UP:
            slots[slot_index].point_state = report_package.point_hid_raw[i].tip_switch ? POINT_STATE_DOWN : POINT_STATE_UP;
            if (is_point_init(&slots[slot_index].point))
                slots[slot_index].point_state = POINT_STATE_INIT;

            if (slots[slot_index].point_state != POINT_STATE_UP)
                MI_KB_DEBUG("slot: %d, point_state: %s",
                    slot_index, string_point_state[slots[slot_index].point_state]);

            break;

        default:
            break;
        }
    }

    /* parse slots */
    for (i = 0; i < MAX_POINTS_IN_PACKAGE; i++)
    {
        /* slot state */
        switch (slots[i].slot_state)
        {
        case SLOT_STATE_INIT:
            if (slots[i].point_state == POINT_STATE_DOWN)
            {
                /* button valid */
                if (is_button_valid(button_state))
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, button valid", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                /* point down in normal area */
                if (!is_point_in_edge_area(slots[i].point.pos_x, slots[i].point.pos_y, param, pdata->cdgain_robot_test_mode))
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, down in normal area", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                else
                {
                    /* point down in edge area */
                    memcpy(&slots[i].first_edge_point, &slots[i].point, sizeof(struct touchpad_point_hid_raw));
                    slots[i].slot_state = SLOT_STATE_EDGE_ACTIVE;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, down in edge area", i, string_slot_state[slots[i].slot_state]);
                    slots[i].point_should_delete_count++;
                    break;
                }
                break;
            }

            break;
        case SLOT_STATE_RELEASE:
            if (slots[i].point_state == POINT_STATE_DOWN)
            {
                /* button valid */
                if (is_button_valid(button_state))
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, button valid", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                /* point down in normal area */
                if (!is_point_in_edge_area(slots[i].point.pos_x, slots[i].point.pos_y, param, pdata->cdgain_robot_test_mode))
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, down in normal area", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                else
                {
                    /* point down in edge area */
                    memcpy(&slots[i].first_edge_point, &slots[i].point, sizeof(struct touchpad_point_hid_raw));
                    slots[i].slot_state = SLOT_STATE_EDGE_ACTIVE;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, down in edge area", i, string_slot_state[slots[i].slot_state]);
                    slots[i].point_should_delete_count++;
                    break;
                }
                break;
            }
            if (slots[i].point_state == POINT_STATE_INIT)
            {
                slots[i].slot_state = SLOT_STATE_INIT;
                slots[i].slot_info_need_report = 1;
                MI_KB_DEBUG("slot: %d, slot_state: %s, init", i, string_slot_state[slots[i].slot_state]);
                break;
            }

            break;

        case SLOT_STATE_NORMAL:
            /* point up */
            if (slots[i].point_state == POINT_STATE_UP)
            {
                clear_slot_except_point(&slots[i]);
                slots[i].slot_state = SLOT_STATE_RELEASE;
                slots[i].slot_info_need_report = 1;
                MI_KB_DEBUG("slot: %d, slot_state: %s, up", i, string_slot_state[slots[i].slot_state]);
                break;
            }
            slots[i].slot_info_need_report = 1;
            break;

        case SLOT_STATE_EDGE_ACTIVE:
            /* button valid */
            if (is_button_valid(button_state))
            {
                slots[i].slot_state = SLOT_STATE_NORMAL;
                slots[i].slot_info_need_report = 1;
                MI_KB_DEBUG("slot: %d, slot_state: %s, button valid", i, string_slot_state[slots[i].slot_state]);
                break;
            }

            /* point up */
            if (slots[i].point_state == POINT_STATE_UP)
            {
                clear_slot_except_point(&slots[i]);
                slots[i].slot_state = SLOT_STATE_RELEASE;
                MI_KB_DEBUG("slot: %d, slot_state: %s, up", i, string_slot_state[slots[i].slot_state]);
                break;
            }

            if (slots[i].point_state == POINT_STATE_MOVE)
            {
                slots[i].point_should_delete_count++;
                /* move in edge area for a long time */
                if (slots[i].point_should_delete_count >= param->shield_frame_count_limit)
                {
                    slots[i].slot_state = SLOT_STATE_EDGE_SHIELD;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, should shield", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                /* move out from edge are */
                if (!is_point_in_edge_area(slots[i].point.pos_x, slots[i].point.pos_y, param, pdata->cdgain_robot_test_mode))
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, move out from edge area", i, string_slot_state[slots[i].slot_state]);
                    break;
                }
                /* move can establish gesture */
                /* notice:
                 * gesture judgment needs to be combined
                 * with slot data and multi-point data of
                 * the current package for judgment. The
                 * result of the judgment will affect the
                 * status of all slots.
                 */
                if (package_gesture_type != 0)
                {
                    slots[i].slot_state = SLOT_STATE_NORMAL;
                    slots[i].slot_info_need_report = 1;
                    MI_KB_DEBUG("slot: %d, slot_state: %s, gesture_type: %s, dis: %ld",
                              i, string_slot_state[slots[i].slot_state], string_gesture_type[package_gesture_type], slots[i].edge_point_move_distance);
                    break;
                }
                else
                {
                    package_gesture_type = find_gesture_type(&report_package, slots, param);
                    if (package_gesture_type != 0)
                    {
                        slots[i].slot_state = SLOT_STATE_NORMAL;
                        slots[i].slot_info_need_report = 1;
                        MI_KB_DEBUG("slot: %d, slot_state: %s, gesture_type: %s, dis: %ld",
                              i, string_slot_state[slots[i].slot_state], string_gesture_type[package_gesture_type], slots[i].edge_point_move_distance);
                        break;
                    }
                }
            }
            break;

        case SLOT_STATE_EDGE_SHIELD:
            /* button valid */
            if (is_button_valid(button_state))
            {
                slots[i].slot_state = SLOT_STATE_NORMAL;
                slots[i].slot_info_need_report = 1;
                MI_KB_DEBUG("slot: %d, slot_state: %s, button valid", i, string_slot_state[slots[i].slot_state]);
                break;
            }
            /* point up */
            if (slots[i].point_state == POINT_STATE_UP)
            {
                clear_slot_except_point(&slots[i]);
                slots[i].slot_state = SLOT_STATE_RELEASE;
                MI_KB_DEBUG("slot: %d, slot_state: %s, up", i, string_slot_state[slots[i].slot_state]);
                break;
            }
            break;

        default:
            break;
        }
    }

    /* pack touchpad HID data */
    pack_hid_data(&report_package, slots, button_state);
    rawdata_show("touchpad report--", report_package.buf, TOUCHPAD_PACKAGE_LENGTH);

    memcpy(buf, report_package.buf, TOUCHPAD_PACKAGE_LENGTH);

    ret = report_package.contact_count;

out:
    mutex_unlock(&pdata->edge_filter_mutex);
    // printk("Joseph timestamp --");
    return ret;
}
EXPORT_SYMBOL_GPL(xiaomi_touchpad_edge_filter);
