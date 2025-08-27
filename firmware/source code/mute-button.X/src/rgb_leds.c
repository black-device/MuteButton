#include "rgb_leds.h"

#define byte2percent(b)         ((b*100)>>8)


static void tmr_callback();


static rgb_leds_cfg_t* lib_cfg = NULL;
static TIMER_EXT_FUNC_T tmr_f;





void RGB_LEDS_Initialize(rgb_leds_cfg_t* cfg) {
    lib_cfg = cfg;
    
    //___ Configure timer
    // For a PBCLK of 24MHz -> 24MHz/32 = 1,3333us -> 75*1,3333 = 100us
    TIMER3_Initialize(TMR23_DIV_BY_32, 75);
    TIMER_On(TIMER3);
    TIMER_Clear(TIMER3);
    
    tmr_f = (TIMER_EXT_FUNC_T){
        .timer = TIMER3,
        .ext_func = tmr_callback,
        .div = TIMER_X1,
    };
    
    TIMER_Add_Function(&tmr_f);
}

void RGB_LEDS_Tasks() {

}


void RGB_LEDS_Set_Color(rgb_color_t col) {
    for (int i = 0; i < rgb_leds_max_leds; i++) lib_cfg->led_list[i].color = col;
}


static void tmr_callback() {
    static int cnt_tmr = 0;
    
    for (int i = 0; i < rgb_leds_max_leds; i++) {
        rgb_led_t* l = &lib_cfg->led_list[i];
        set_pin(l->r_pin, (l->color.r > cnt_tmr));
        set_pin(l->g_pin, (l->color.g > cnt_tmr));
        set_pin(l->b_pin, (l->color.b > cnt_tmr));
    }    
    
    if (++cnt_tmr > 0x3f) cnt_tmr = 0;
}