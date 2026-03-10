#ifndef _LCD_PROX_GESTURE_KEEP_HIGH_H_
#define _LCD_PROX_GESTURE_KEEP_HIGH_H_
#include <linux/types.h>
#include <stdbool.h>

// 定义屏幕标识
typedef enum {
    MODULE_A,
    MODULE_B,
    MODULE_C
} ModuleType;

// 统一接口
void set_lcd_reset_gpio_keep_high(bool en, int module); //手势
void set_prox_lcd_reset_gpio_keep_high(bool en, int module); //距感

#endif
