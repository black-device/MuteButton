/*
 *
 * Librería de alto nivel para gestión de los timers
 * FDguez. 20151216
 *
 */

#include "timers.h"
#include <stdbool.h>
#include <sys/attribs.h>
#include <proc/p32mm0256gpm048.h>


static TIMER_EXT_FUNC_T *ex_func_list[TIMER_MAX_EXTERNAL_FUNC];
static uint16_t num_ex_func = 0;

static TIMER_COUNTER_T *counter_list[TIMER_MAX_COUNTERS];
static uint16_t num_counter = 0;

static TIMER1_PRESCALERS tmr1_curr_prescaler;
static TIMER23_PRESCALERS tmr2_curr_prescaler, tmr3_curr_prescaler, tmr4_curr_prescaler;
static uint16_t tmr1_curr_period, tmr3_curr_period;
static uint32_t tmr2_curr_period, tmr4_curr_period;
static bool tmr2_curr_32bit_mode, tmr4_curr_32bit_mode;
static uint16_t tmr1_cnt, tmr2_cnt, tmr3_cnt, tmr4_cnt;



/**
 * Library initialization
 */
void TIMER_Initialize() {
    num_counter = 0;
}



void timer1_off() { T1CON = 0; }

void timer2_off() { T2CON = 0; }

void timer3_off() { T3CON = 0; }


void timer1_on() {
    // clear internal counter
    tmr1_cnt = 0;
    
    T1CON = 0;
    T1CONbits.TCKPS = tmr1_curr_prescaler;  // Set timer 1 prescaler (0=1:1, 1=1:8, 2=1:64, 3=1:256)
    PR1 = tmr1_curr_period;          // Set Timer 1 period (max value is 65535)
    
//    IFS0bits.T1IF = 0; // Clear Timer 1 interrupt flag
//    IEC0bits.T1IE = 1; // Enable Timer 1 interrupt  
    IFS0CLR = _IFS0_T1IF_MASK;
    IEC0SET = _IEC0_T1IE_MASK;
    
    IPC4bits.T1IP = 1;
    IPC4bits.T1IS = 0;
    
    T1CONbits.TON = 1;    
}



/*
 * Activa el TIMER2
 */
void timer2_on() {
    // clear internal counter
    tmr2_cnt = 0;
    
    T2CON = 0;
    T2CONbits.T32 = tmr2_curr_32bit_mode; // set 16/32 bits mode
    T2CONbits.TCKPS = tmr2_curr_prescaler;  // Set timer 2 prescaler (0=1:1, 1=1:8, 2=1:64, 3=1:256)
    PR2 = tmr2_curr_period;          // Set Timer 2 period (max value is 65535)
    if (tmr2_curr_32bit_mode) PR3 = (tmr2_curr_period >> 16); // Uses TIMER3 registers in 32 bit mode
    
    IFS0bits.T2IF = 0; // Clear Timer 2 interrupt flag
    IEC0bits.T2IE = 1; // Enable Timer 2 interrupt
    
    IPC4bits.T2IP = 1;
    IPC4bits.T2IS = 0;
     
    T2CONbits.TON = 1;
}



/*
 * Activa el TIMER3
 */
void timer3_on() {
    // clear internal counter
    tmr3_cnt = 0;
    
    T3CON = 0;
    T3CONbits.TCKPS = tmr3_curr_prescaler;  // Set timer 3 prescaler (0=1:1, 1=1:8, 2=1:64, 3=1:256)
    PR3 = tmr3_curr_period;          // Set Timer 3 period (max value is 65535)
    
//    IFS0CLR = _IFS0_T3IF_MASK;
//    IEC0SET = _IEC0_T3IE_MASK;
    
    IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag
    IEC0bits.T3IE = 1; // Enable Timer 3 interrupt
    
    IPC4bits.T3IP = 1;
    IPC4bits.T3IS = 0;
    
    T3CONbits.TON = 1; 
}


/*
 * Proceso que se ejecuta dentro de la interrupción de un timer
 */
void proc_interrupt(TIMERS t, uint16_t *cnt) {

    (*cnt)++;
    
    // process the sw. counters
    uint16_t i;
    for (i = 0; i < num_counter; i++) {
        
        // counter linked with timer?
        if (counter_list[i]->timer != t) continue;
        
        // enabled counter?
        if (!counter_list[i]->counter_on) continue;
        
        TIMER_DIVIDERS div = counter_list[i]->divider;
        if (div == TIMER_X2 && (*cnt % 2) != 0) continue;
        if (div == TIMER_X10 && (*cnt % 10) != 0) continue;
        if (div == TIMER_X1K && (*cnt % 1000) != 0) continue;

        counter_list[i]->counter++;
        counter_list[i]->changed = 1;

        
//        if ((div == TIMER_X1) ||
//            (div == TIMER_X2 && (*cnt % 2) == 0) ||
//            (div == TIMER_X10 && (*cnt % 10) == 0) ||
//            (div == TIMER_X100 && (*cnt % 100) == 0) ||
//            (div == TIMER_X1K && (*cnt % 1000) == 0) ||
//            (div == TIMER_X10K && (*cnt % 10000) == 0)) {
//            counter_list[i]->counter++;
//            counter_list[i]->changed = 1;
//        };
    }
    
    // process callback functions
    for (i = 0; i < num_ex_func; i++) {
        
        // function linked with timer?
        if (ex_func_list[i]->timer != t) continue;
        
        if (ex_func_list[i]->ext_func == 0) continue;
        
        TIMER_DIVIDERS div = ex_func_list[i]->div;
        
        if (div == TIMER_X2 && (*cnt % 2) != 0) continue;
        if (div == TIMER_X10 && (*cnt % 10) != 0) continue;
        if (div == TIMER_X1K && (*cnt % 1000) != 0) continue;
        
        ex_func_list[i]->ext_func();
        
//        if ((div == TIMER_X1) ||
//            (div == TIMER_X2 && (*cnt % 2) == 0) ||
//            (div == TIMER_X10 && (*cnt % 10) == 0) ||
//            (div == TIMER_X100 && (*cnt % 100) == 0) ||
//            (div == TIMER_X1K && (*cnt % 1000) == 0) ||
//            (div == TIMER_X10K && (*cnt % 10000) == 0)) {
//            ex_func_list[i]->ext_func();
//        };
    }
    
    if ((*cnt) >= 10000) (*cnt) = 0;
}



/*
 * Timer 1 service routine
 */
void __attribute__ ((vector(_TIMER_1_VECTOR), interrupt(IPL1SOFT))) TMR1_ISR(void) {

    proc_interrupt(TIMER1, &tmr1_cnt);
    
    IFS0CLR = _IFS0_T1IF_MASK; // Clear Timer 1 interrupt flag
}


/*
 * Timer 2 service routine
 */
void __attribute__ ((vector(_TIMER_2_VECTOR), interrupt(IPL1SOFT))) TMR2_ISR() {
    
    proc_interrupt(TIMER2, &tmr2_cnt);
    
    // Clear Timer 2 interrupt flag
    IFS0CLR = _IFS0_T2IF_MASK;
}


/*
 * Timer 3 service routine
 */
void __attribute__ ((vector(_TIMER_3_VECTOR), interrupt(IPL1SOFT))) TMR3_ISR() {
    
    proc_interrupt(TIMER3, &tmr3_cnt);
    
    // Clear Timer 3 interrupt flag
    IFS0CLR = _IFS0_T3IF_MASK;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/*
 * TIMER1 configuration
 * prescaler: sets the divider over PBCLK. 0=1:1, 1=1:8, 2=1:64, 3=1:256 
 * period: sets the value of the 16-bit counter to fire the interrupt (0..65535)
 * Example: 1-Second trigger for a PBCLK of 16MHz:
 *     prescaler = 3 (16MHz/256 = 62,5KHz => 16us)
 *     period = 62500 (16us * 62500 = 1s)
 * 
 */
void TIMER1_Initialize(TIMER1_PRESCALERS prescaler, uint16_t period) {
    
    tmr1_curr_prescaler = prescaler;
    tmr1_curr_period = period;
    
    timer1_on();
}



/*
 * TIMER2 configuration
 * prescaler: sets the divider over PBCLK. 0=1:1, 1=1:8, 2=1:64, 3=1:256 
 * period: sets the value of the 16/32-bit counter to fire the interrupt
 *    Posible values => (0..65535) for 16-bit; (0-2^32-1) for 32-bit
 * tmr32bits: indicates if the period counter is a 16-bit (0) or 32-bit value (1) field
 *     IMPORTANT: in 32-bit mode, TIMER3 will be used
 * Example: 1-Second trigger for a PBCLK of 16MHz:
 *     prescaler = 3 (16MHz/256 = 62,5KHz => 16us)
 *     period = 62500 (16us * 62500 = 1s)
 * 
 */
void TIMER2_Initialize(TIMER23_PRESCALERS prescaler, uint32_t period, bool tmr32bits) {
    
    // define los nuevos parámetros
    tmr2_curr_prescaler = prescaler;
    tmr2_curr_period = period;
    tmr2_curr_32bit_mode = tmr32bits;
    
    // activa el temporizador
    timer2_on();
}



/*
 * TIMER3 configuration
 * prescaler: sets the divider over PBCLK. 0=1:1, 1=1:8, 2=1:64, 3=1:256 
 * period: sets the value of the 16-bit counter to fire the interrupt (0..65535)
 * Example: 1-Second trigger for a PBCLK of 16MHz:
 *     prescaler = 3 (16MHz/256 = 62,5KHz => 16us)
 *     period = 62500 (16us * 62500 = 1s)
 * 
 */
void TIMER3_Initialize(TIMER23_PRESCALERS prescaler, uint16_t period) {
    
    // define los nuevos parámetros
    tmr3_curr_prescaler = prescaler;
    tmr3_curr_period = period;
    
    // activa el temporizador
    timer3_on();
}


/*
 * Activa un timer
 */
void TIMER_On(TIMERS t) {
    switch(t) {
        case TIMER1:
            timer1_on();
            break;
            
        case TIMER2:
            timer2_on();
        break;

        case TIMER3:
            timer3_on();
        break;
    }
}



/*
 * Desactiva un timer
 */
void TIMER_Off(TIMERS t) {
    switch(t) {
        case TIMER1:
            timer1_off();
            break;
            
        case TIMER2:
            timer2_off();
        break;

        case TIMER3:
            timer3_off();
        break;
    }
}


/*
 * clears a timer
 */
void TIMER_Clear(TIMERS t) {
    if (t == TIMER1) TMR1 = 0;
    else if (t == TIMER2) TMR2 = 0;
    else if (t == TIMER3) TMR3 = 0;
}


/**
 * Obtains the value of the timer
 * @param t
 * @return 
 */
uint32_t TIMER_Get(TIMERS t) {
    if (t == TIMER1) return TMR1;
    if (t == TIMER2) return TMR2;
    if (t == TIMER3) return TMR3;
}



/* 
 * Añade un contador
 * Devuelve 1 si no se ha podido añadir el contador
 */
uint8_t TIMER_Add_Counter(TIMER_COUNTER_T *counter) {
    if ((num_counter + 1) >= TIMER_MAX_COUNTERS) return 1;

    uint16_t i;
    for (i = 0; i < num_counter; i++) {
        if (counter_list[i] == counter) {
            // Ya estaba añadido
            return 0;
        }
    }
    
    counter_list[num_counter++] = counter;
    
    return 0;
}


/*
 * elimina un contador de la lista
 * Devuelve 2 si no ha podido, por ejemplo porque no existía
 */
uint8_t TIMER_Del_Counter(TIMER_COUNTER_T *counter) {
    uint16_t i;
    
    for (i = 0; i < num_counter; i++) {
        if (counter_list[i] == counter) {
            // se ha encontrado el temporizador
            
            TIMERS tmr = counter->timer;
            TIMER_Off(tmr); // se para el temporizador 
            
            //  Se borra desplazando los temporizadores siguientes
            num_counter--;
            for (; i < num_counter; i++) {
                counter_list[i] = counter_list[i + 1];
            }
            
            TIMER_On(tmr); // se reinicia el temporizador
            
            return 0;
        }
    }
    
    // no se ha encontrado el temporizador
    return 1;
}



/* 
 * Añade una función externa a un timer
 * Devuelve 1 si no se ha podido añadir la función
 */
uint8_t TIMER_Add_Function(TIMER_EXT_FUNC_T *func) {
    if ((num_ex_func + 1) >= TIMER_MAX_EXTERNAL_FUNC) return 1;
    
    ex_func_list[num_ex_func++] = func;
    
    return 0;
}


/*
 * elimina una función de la lista
 * Devuelve 2 si no ha podido, por ejemplo porque no existía
 */
uint8_t TIMER_Del_Function(TIMER_EXT_FUNC_T *func) {
    uint16_t i;
    
    for (i = 0; i < num_ex_func; i++) {
        if (&ex_func_list[i] == &func) {
            // se ha encontrado la función
            
            TIMERS tmr = func->timer;
            TIMER_Off(tmr); // se para el temporizador 
            
            
            // Se borra desplazando las fuciones siguientes
            num_ex_func--;
            for (; i < num_ex_func; i++) {
                ex_func_list[i] = ex_func_list[i + 1];
            }
            
            TIMER_On(tmr); // se reinicia el temporizador
            
            return 0;
        }
    }
    
    // no se ha encontrado la función
    return 1;
}


