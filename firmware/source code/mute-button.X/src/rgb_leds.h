#ifndef _RGB_LEDS_H    /* Guard against multiple inclusion */
#define _RGB_LEDS_H

#include <stddef.h>

#include "hardware.h"
#include "timers/timers.h"


#define rgb_leds_max_leds           (3)

#define RGB_LEDS_MAX_VALUE          (0x3f)


/**
 Colors has 6-bit precission (0-63)
 */
typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t _a_;
    };
    uint32_t rgb;
} rgb_color_t;


typedef struct {
    GPIO_T r_pin;
    GPIO_T g_pin;
    GPIO_T b_pin;
    
    rgb_color_t color;
} rgb_led_t;


typedef struct {
    rgb_led_t led_list[rgb_leds_max_leds];
} rgb_leds_cfg_t;


void RGB_LEDS_Initialize(rgb_leds_cfg_t* cfg);


void RGB_LEDS_Tasks();


void RGB_LEDS_Set_Color(rgb_color_t col);

#endif
    