#ifndef _ENCODER_H_
#define _ENCODER_H_


#include <stddef.h>

#include "../hardware.h"
#include "../timers/timers.h"


typedef enum {
    Encoder_Event_Button_Push,
    Encoder_Event_Button_Release,
    Encoder_Event_Clockwise,
    Encoder_Event_Counterclockwise
} encoder_event_t;


typedef void (*encoder_event_f)(encoder_event_t ev);


typedef struct {
    GPIO_T btn_pin;
    GPIO_T enc_a_pin;
    GPIO_T enc_b_pin;
    
    TIMER_COUNTER_T* tmr_200us;
    
    encoder_event_f ev_f;
    
} encoder_cfg_t;


void ENCODER_Initialize(encoder_cfg_t* cfg);


void ENCODER_Tasks();



#endif