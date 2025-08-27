#ifndef _APP_CONFIG_H    /* Guard against multiple inclusion */
#define _APP_CONFIG_H

#include "hardware.h"

// Physical address where Software and Hardware version is saved
// The reason is to let the bootloader access at this information without execute the app
#define app_version_addr            0xBD03F700//0x9D03F700

/*
 Hardware and Software version definition
 */
#define Hw_Base_Version             (0x03)
#define Software_Version            (8)
/*----------------------------------------*/

#define Hardware_Version            ((uint32_t)Hw_Base_Version<<4 | (0))



/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif
    
    extern const GPIO_T rs485_tx_led_pin;
    extern const GPIO_T rs485_tx_en_pin;
    extern const GPIO_T bus_sel_pin;
    
    extern const GPIO_T rgb_led_r_pin;
    extern const GPIO_T rgb_led_g_pin;
    extern const GPIO_T rgb_led_b_pin;
    
    extern const GPIO_T presence_led_pin;
    extern const GPIO_T banner_led_pin;
    
    extern const GPIO_T h_phase_pin;
    extern const GPIO_T l_phase_pin;
    
    extern const GPIO_T encoder_1_pin;
    extern const GPIO_T encoder_2_pin;
    extern const GPIO_T encoder_4_pin;
    extern const GPIO_T encoder_8_pin;
    
    extern const GPIO_T button_pin;
    
    
    #define an_input_potentiometer                      (15)
    #define an_input_v_bus                              (0)
    #define an_input_v_dcin                             (1)
    #define an_input_sens                               (6)      
    //extern const GPIO_T led_status_pin;
    //extern const GPIO_T led_activity_pin;
    //extern const GPIO_T push_pin;

    


    void APP_CFG_Initialize();


/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif 

/* *****************************************************************************
 End of File
 */
