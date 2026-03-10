#include "lcd_prox_gesture_keep_high.h"
#include <linux/export.h>

// 一供屏的状态
bool lcd_reset_keep_high_zeroa = false;
bool prox_lcd_reset_keep_high_zeroa = false;
EXPORT_SYMBOL(lcd_reset_keep_high_zeroa);
EXPORT_SYMBOL(prox_lcd_reset_keep_high_zeroa);

// 二供屏的状态
bool lcd_reset_keep_high_zerob = false;
bool prox_lcd_reset_keep_high_zerob = false;
EXPORT_SYMBOL(lcd_reset_keep_high_zerob);
EXPORT_SYMBOL(prox_lcd_reset_keep_high_zerob);

// 3供屏的状态
bool lcd_reset_keep_high_zeroc = false;
bool prox_lcd_reset_keep_high_zeroc = false;
EXPORT_SYMBOL(lcd_reset_keep_high_zeroc);
EXPORT_SYMBOL(prox_lcd_reset_keep_high_zeroc);

//手势接口
void set_lcd_reset_gpio_keep_high(bool en, int module)
{
    switch (module) {
        case MODULE_A:
            lcd_reset_keep_high_zeroa = en;
            break;
        case MODULE_B:
            lcd_reset_keep_high_zerob = en;
            break;
        case MODULE_C:
            lcd_reset_keep_high_zeroc = en;
            break;
        default:
            break;
    }
}
//距感接口
void set_prox_lcd_reset_gpio_keep_high(bool en, int module)
{
    switch (module) {
        case MODULE_A:
            prox_lcd_reset_keep_high_zeroa = en;
            break;
        case MODULE_B:
            prox_lcd_reset_keep_high_zerob = en;
            break;
        case MODULE_C:
            prox_lcd_reset_keep_high_zeroc = en;
            break;
        default:
            break;
    }
}

EXPORT_SYMBOL(set_prox_lcd_reset_gpio_keep_high);
EXPORT_SYMBOL(set_lcd_reset_gpio_keep_high);
