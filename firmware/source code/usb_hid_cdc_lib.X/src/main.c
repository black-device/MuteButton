#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// This definition must be defined in project properties
#define BOOT_WITHOUT_BOOTLOADERs

#include "hardware.h"
#include "app_config.h"
#include "timers/timers.h"
#include "usb_hid_cdc/usb_hid_cdc.h"






void main(void) {
    
    //INT_Global_Disable();
    
    hardware_init();
    
    APP_CFG_Initialize();
    
    //__ USB Configuration _____________________________________________________
    usb_hid_cdc_cfg_t usb_cfg;
    
    USB_HID_CDC_Initialize(&usb_cfg);
    //__________________________________________________________________________
    
    //INT_Global_Enable();
    //___ Timers initialization ________________________________________________
    TIMER_Initialize();
    //___ Configure timer
    // For a PBCLK of 24MHz -> 24MHz/64 = 2,6667us -> 375*2,6667 = 1ms
    TIMER1_Initialize(TMR1_DIV_BY_64, 375);
    TIMER_On(TIMER1);
    TIMER_Clear(TIMER1);
    TIMER_COUNTER_T main_counter;
    main_counter.timer = TIMER1;
    main_counter.divider = TIMER_X1;
    main_counter.counter_on = true;
    main_counter.counter = 0;

    TIMER_Add_Counter(&main_counter);
    //__________________________________________________________________________
    
    

    static uint32_t btn_cnt = 0;
    
    while (1) {
        USB_HID_CDC_Tasks();
        
        if (!get_pin(button_pin) && main_counter.counter > btn_cnt) {
            static const char msg[] = "!\"$%&/()\r";//"hola holita, vecinito!!!\n";
            static const char* msg_ptr = msg;
            USB_HID_CDC_Send_Key(APP_Keyboard_Ascii_2_KeyCode(*(msg_ptr++)));
            if (*msg_ptr == 0) msg_ptr = msg;
            
            btn_cnt = main_counter.counter + 80;
        }
        
    }
    
    
}