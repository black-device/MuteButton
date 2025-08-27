#include "encoder.h"


static encoder_cfg_t* lib_cfg = NULL;



void ENCODER_Initialize(encoder_cfg_t* cfg) {
    lib_cfg = cfg;
    
    set_pin_cfg_input(lib_cfg->btn_pin, GPIO_PULLUP);
    set_pin_cfg_input(lib_cfg->enc_a_pin, GPIO_PULLUP);
    set_pin_cfg_input(lib_cfg->enc_b_pin, GPIO_PULLUP);
    
    
}


void ENCODER_Tasks() {
#define BTN_FLAG            (0x01)
#define BTN_MASK            (~BTN_FLAG)
#define ENC_B_FLAG          (0x02)
#define ENC_B_MASK          (~ENC_B_FLAG)
#define BTN_ENTRY_MASK      (0x04)
#define ENC_A_ENTRY_MASK    (0x02)
#define ENC_B_ENTRY_MASK    (0x01)
    static uint8_t flags = 0;
    static uint8_t last_signal = 0xff;
    static uint8_t pre_curr_signal = 0;
    static uint32_t pre_curr_signal_time = 0;
    
    // read the input pins
    uint8_t curr_signal = 
            ((get_pin(lib_cfg->btn_pin) == 0) * BTN_ENTRY_MASK) | 
            ((get_pin(lib_cfg->enc_a_pin) != 0) * ENC_A_ENTRY_MASK) | 
            ((get_pin(lib_cfg->enc_b_pin) == 0) * ENC_B_ENTRY_MASK);
    
    // prefilter to avoid bounces. Expected 2ms of stability
    if (pre_curr_signal != curr_signal) {
        pre_curr_signal = curr_signal;
        pre_curr_signal_time = lib_cfg->tmr_200us->counter;
        return;
    } else if ((lib_cfg->tmr_200us->counter - pre_curr_signal_time) < 2*5) return;
    
    // signal changed?
    if (last_signal == 0xff) last_signal = curr_signal; // first execution
    if (last_signal == curr_signal) return;
    
    // yes, it changed
    last_signal = curr_signal;
    
    if (!lib_cfg->ev_f) return; // no event handler
    
    // button stuff
    if ((curr_signal & BTN_ENTRY_MASK) && (flags & BTN_FLAG) == 0) {
        lib_cfg->ev_f(Encoder_Event_Button_Push);
        flags |= BTN_FLAG;
    } else if ((curr_signal & BTN_ENTRY_MASK) == 0 && (flags & BTN_FLAG)) {
        lib_cfg->ev_f(Encoder_Event_Button_Release);
        flags &= BTN_MASK;
    }
    
    // encoder stuff
    curr_signal &= (ENC_A_ENTRY_MASK|ENC_B_ENTRY_MASK);
    
    if ((flags & ENC_B_FLAG) == 0) {
        if (curr_signal == (ENC_A_ENTRY_MASK|ENC_B_ENTRY_MASK)) {
            lib_cfg->ev_f(Encoder_Event_Counterclockwise);
            flags |= ENC_B_FLAG;
        } else if (curr_signal == ENC_B_ENTRY_MASK) {
            lib_cfg->ev_f(Encoder_Event_Clockwise);
            flags |= ENC_B_FLAG;
        }        
    } 
    
    // clear (or not) the flags of signal processed
    if ((curr_signal & ENC_B_ENTRY_MASK) == 0) flags &= ENC_B_MASK;
    
}