#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hardware.h"
#include "app_config.h"
#include "timers/timers.h"
#include "usb_hid_cdc.h"
#include "rgb_leds.h"
#include "encoder/encoder.h"



//const uint32_t keys_mute = 0x07007f;//0xe023;//0xe0a0e023;//0x0000007f;//0x20e0;//0xe020;
//const uint32_t keys_vol_up = 0x070080;//0xe0b0;//0xe030e0b0;//0x00000080;////0x00000001;
//const uint32_t keys_vol_down = 0x070081;//0xe0ae;//0xe020e0ae;//0x00000081;//0x00000002;
const uint32_t keys_mute = 0x04;// 0xe5;//0xe023;//0xe0a0e023;//0x0000007f;//0x20e0;//0xe020;
const uint32_t keys_vol_up = 0xe6;//0xe0b0;//0xe030e0b0;//0x00000080;////0x00000001;
const uint32_t keys_vol_down = 0xe7;//0xe0ae;//0xe020e0ae;//0x00000081;//0x00000002;

const int short_press_time_ms = 1000;


static rgb_color_t state_color[] = {
    (rgb_color_t){ .r = 0x00, .g = 0x1f, .b = 0x00 }, // base color for mute off
    (rgb_color_t){ .r = 0x1f, .g = 0x00, .b = 0x00 }, // base color for mute on
    (rgb_color_t){ .r = 0x20, .g = 0x00, .b = 0x30 },
    (rgb_color_t){ .r = 0x57, .g = 0x4c, .b = 0x3f },
    (rgb_color_t){ .r = 0x34, .g = 0x65, .b = 0x96 },
    (rgb_color_t){ .r = 0x20, .g = 0x00, .b = 0x30 },
    (rgb_color_t){ .r = 0x30, .g = 0x00, .b = 0x30 },
};
#define max_states                      (sizeof(state_color)/sizeof(rgb_color_t))

static TIMER_COUNTER_T main_counter;
static uint8_t current_state = (max_states - 1);
static uint8_t current_state_last = 0xff;
static int current_volume = 0;
static uint32_t last_comm_time = 0;




void intro(rgb_leds_cfg_t* leds, TIMER_COUNTER_T* cnt) {
    typedef struct { uint32_t tmr; rgb_led_t* led; rgb_color_t col; } intro_reg_t;
    
    intro_reg_t intro[] = {
        (intro_reg_t){ .tmr = 1, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x000000} },
        (intro_reg_t){ .tmr = 1, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x000000} },
        (intro_reg_t){ .tmr = 1, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x000000} },
        
        (intro_reg_t){ .tmr = 100, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x000009} },
        (intro_reg_t){ .tmr = 200, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x000009} },
        (intro_reg_t){ .tmr = 300, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x000009} },
        
        (intro_reg_t){ .tmr = 400, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x000010} },
        (intro_reg_t){ .tmr = 500, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x000010} },
        (intro_reg_t){ .tmr = 600, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x000010} },
        
        (intro_reg_t){ .tmr = 700, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x000020} },
        (intro_reg_t){ .tmr = 800, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x000020} },
        (intro_reg_t){ .tmr = 900, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x000020} },
        
        (intro_reg_t){ .tmr = 1000, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x000900} },
        (intro_reg_t){ .tmr = 1100, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x000900} },
        (intro_reg_t){ .tmr = 1200, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x000900} },
        
        (intro_reg_t){ .tmr = 1300, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x001000} },
        (intro_reg_t){ .tmr = 1400, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x001000} },
        (intro_reg_t){ .tmr = 1500, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x001000} },
        
        (intro_reg_t){ .tmr = 1600, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x002000} },
        (intro_reg_t){ .tmr = 1700, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x002000} },
        (intro_reg_t){ .tmr = 1800, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x002000} },
        
        (intro_reg_t){ .tmr = 1900, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x090000} },
        (intro_reg_t){ .tmr = 2000, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x090000} },
        (intro_reg_t){ .tmr = 2100, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x090000} },
        
        (intro_reg_t){ .tmr = 2200, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x100000} },
        (intro_reg_t){ .tmr = 2300, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x100000} },
        (intro_reg_t){ .tmr = 2400, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x100000} },
        
        (intro_reg_t){ .tmr = 2500, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x200000} },
        (intro_reg_t){ .tmr = 2600, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x200000} },
        (intro_reg_t){ .tmr = 2700, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x200000} },
        
        (intro_reg_t){ .tmr = 2800, .led = &leds->led_list[0], (rgb_color_t){.rgb=0x202020} },
        (intro_reg_t){ .tmr = 2900, .led = &leds->led_list[1], (rgb_color_t){.rgb=0x202020} },
        (intro_reg_t){ .tmr = 3000, .led = &leds->led_list[2], (rgb_color_t){.rgb=0x202020} },
        
        // end of the intro
        (intro_reg_t){ .tmr = 0, .led = NULL, (rgb_color_t){.rgb=0x000000} }, 
    };
    
    intro_reg_t* intro_ptr = intro;
    uint32_t cnt_zero = cnt->counter;
    while (1) {
        
        WDT_Clear();
        
        RGB_LEDS_Tasks();
        
        if (intro_ptr->tmr == 0 || intro_ptr->led == NULL) break;
        
        uint32_t cnt_ms = (cnt->counter - cnt_zero)/3;
        
        if (cnt_ms > intro_ptr->tmr) {
            intro_ptr->led->color = intro_ptr->col;
            intro_ptr++;
        }
    }
}
//

void encoder_event_cb(encoder_event_t ev) {
    
//    uint32_t keys_to_send =
//            (ev == Encoder_Event_Button_Push) ? keys_mute :
//            (ev == Encoder_Event_Clockwise) ? keys_vol_up :
//            (ev == Encoder_Event_Counterclockwise) ? keys_vol_down : 0x00000000;
//    
//    USB_HID_CDC_Send_Scan_Code(keys_to_send);
    
    static uint32_t btn_tmr = 0;
    uint8_t k = 0;
    
    if (ev == Encoder_Event_Button_Push) {
        k = (current_state == 1) ? 0xf1 : 0xf2;
        btn_tmr = main_counter.counter;
    } else if (ev == Encoder_Event_Button_Release && (current_state == 1) && (main_counter.counter - btn_tmr) > short_press_time_ms*5) {
        // long press in muted mode-> walkie-talkie mode
        k = 0xf1; // unmute
    } else if (ev == Encoder_Event_Clockwise) k = 0xf8;
    else if (ev == Encoder_Event_Counterclockwise) k = 0xf9;
    
    if (k) {
        USB_HID_CDC_Serial_Tx(&k, 1);
        USB_HID_CDC_Tasks();
    }
}

void set_new_color_val(rgb_color_t* base_col, rgb_color_t* new_col, int div) {
    if (div > 100) div = 100;
    new_col->r = (int)(base_col->r)*div/100;
    new_col->g = (int)(base_col->g)*div/100;
    new_col->b = (int)(base_col->b)*div/100;
    
    if (new_col->r == 0 && base_col->r) new_col->r = 1;
    if (new_col->g == 0 && base_col->g) new_col->g = 1;
    if (new_col->b == 0 && base_col->b) new_col->b = 1;
}


void usb_cdc_rx_cb(uint8_t* data, int data_len) {
    
    last_comm_time = main_counter.counter;
    
    if (data_len == 4) {
        // still alive frame
        if (data[0] == 0x11 && data[1] == 0x22 && data[2] == 0x33 && data[3] == 0x44) {
            uint8_t res = 0x55;
            USB_HID_CDC_Serial_Tx(&res, 1);
        }
    } else if (data_len == 2) {
        // state frame. byte 0: state; byte 1: current volume
        current_state = (data[0] >= max_states) ? (max_states - 1) : data[0];
        current_volume = data[1];
        current_state_last = 0xff;
        
    } else if (data_len >= 3 && (data_len % 3) == 0) {
        // color setup frame
        for (int i = 0; i < data_len; i++) if (data[i] > RGB_LEDS_MAX_VALUE) data[i] = RGB_LEDS_MAX_VALUE;
        int j = 0;
        for (int i = 0; i < data_len; i += 3) {
            state_color[j++] = (rgb_color_t){ .r = data[i], .g = data[i + 1], .b = data[i + 2] }; 
            if (j >= sizeof(state_color)/sizeof(rgb_color_t)) break;
        }
        current_state_last = 0xff;
    }
}






void main(void) {
    
    INT_Global_Disable();
    
    hardware_init();
    
    APP_CFG_Initialize();

    INT_Global_Enable();
    
    //___ Timers initialization ________________________________________________
    TIMER_Initialize();
    //___ Configure timer
    // For a PBCLK of 24MHz -> 24MHz/64 = 2,6667us -> 375*2,6667 = 1ms
    // For a PBCLK of 24MHz -> 24MHz/64 = 2,6667us -> 75*2,6667 = 200us
    TIMER1_Initialize(TMR1_DIV_BY_64, 75);
    TIMER_On(TIMER1);
    TIMER_Clear(TIMER1);
    main_counter.timer = TIMER1;
    main_counter.divider = TIMER_X1;
    main_counter.counter_on = true;
    main_counter.counter = 0;

    TIMER_Add_Counter(&main_counter);
    //__________________________________________________________________________
    
    //__ USB Configuration _____________________________________________________
    uint8_t cdc_rx_buffer[1024];
    uint8_t cdc_tx_buffer[1024];
    usb_hid_cdc_cfg_t usb_cfg;
    usb_cfg.cdc_rx_buffer = cdc_rx_buffer;
    usb_cfg.cdc_rx_buffer_size = sizeof(cdc_rx_buffer);
    usb_cfg.cdc_tx_buffer = cdc_tx_buffer;
    usb_cfg.cdc_tx_buffer_size = sizeof(cdc_tx_buffer);
    usb_cfg.cdc_rx_f = usb_cdc_rx_cb;
    
    USB_HID_CDC_Initialize(&usb_cfg);
    //__________________________________________________________________________
    
    //__ RGB Leds ______________________________________________________________
    rgb_leds_cfg_t rgb_cfg;
    rgb_cfg.led_list[0].r_pin = (GPIO_T){ .port = GPIO_PORT_B, .pin_bit = PIN_BIT_9 };
    rgb_cfg.led_list[0].g_pin = (GPIO_T){ .port = GPIO_PORT_C, .pin_bit = PIN_BIT_9 };
    rgb_cfg.led_list[0].b_pin = (GPIO_T){ .port = GPIO_PORT_B, .pin_bit = PIN_BIT_8 };
    rgb_cfg.led_list[0].color.rgb = 0x00000000;
    
    rgb_cfg.led_list[1].r_pin = (GPIO_T){ .port = GPIO_PORT_A, .pin_bit = PIN_BIT_8 };
    rgb_cfg.led_list[1].g_pin = (GPIO_T){ .port = GPIO_PORT_D, .pin_bit = PIN_BIT_0 };
    rgb_cfg.led_list[1].b_pin = (GPIO_T){ .port = GPIO_PORT_A, .pin_bit = PIN_BIT_3 };
    rgb_cfg.led_list[1].color.rgb = 0x00000000;
    
    rgb_cfg.led_list[2].r_pin = (GPIO_T){ .port = GPIO_PORT_B, .pin_bit = PIN_BIT_13 };
    rgb_cfg.led_list[2].g_pin = (GPIO_T){ .port = GPIO_PORT_B, .pin_bit = PIN_BIT_15 };
    rgb_cfg.led_list[2].b_pin = (GPIO_T){ .port = GPIO_PORT_A, .pin_bit = PIN_BIT_10 };
    rgb_cfg.led_list[2].color.rgb = 0x00000000;
       
    RGB_LEDS_Initialize(&rgb_cfg);
    //__________________________________________________________________________
    
    //__ encoder _______________________________________________________________
    encoder_cfg_t enc_cfg;
    enc_cfg.btn_pin = (GPIO_T){ .port = GPIO_PORT_A, .pin_bit = PIN_BIT_4 };
    enc_cfg.enc_a_pin = (GPIO_T){ .port = GPIO_PORT_C, .pin_bit = PIN_BIT_4 };
    enc_cfg.enc_b_pin = (GPIO_T){ .port = GPIO_PORT_C, .pin_bit = PIN_BIT_3 };
    enc_cfg.tmr_200us = &main_counter;
    enc_cfg.ev_f = encoder_event_cb;
    
    ENCODER_Initialize(&enc_cfg);
    //__________________________________________________________________________
    
  
    
    //__ main loop _____________________________________________________________
    
    intro(&rgb_cfg, &main_counter);

    
    
    while (1) {
        
        WDT_Clear();
        
        USB_HID_CDC_Tasks();
        
        RGB_LEDS_Tasks();
        
        ENCODER_Tasks();
        
        if (current_state != current_state_last) {
            current_state_last = current_state;
            rgb_color_t c = state_color[current_state];
            if (current_state <= 1) set_new_color_val(&state_color[current_state], &c, current_volume);
            RGB_LEDS_Set_Color(c);
        }
        
        // communication watchdog
        if (main_counter.counter > (last_comm_time + 5*3000)) current_state = (max_states - 1);
        
        //__debug___
//        static uint32_t btn_cnt = 0;
//        if (!get_pin(button_pin) && main_counter.counter > btn_cnt) {
//            USB_HID_CDC_Send_Scan_Code(0xe020);
//            
////            static const char msg[] = "Hola holita, vecinito!!!\r";
////            static const char* msg_ptr = msg;
////            USB_HID_CDC_Send_Char(*(msg_ptr++));
////            if (*msg_ptr == 0) msg_ptr = msg;
//            
//            btn_cnt = main_counter.counter + 30*5;
//        }
        //__debug__
        
    }
    
    
}