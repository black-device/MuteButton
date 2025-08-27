/*
 *
 * Librería de alto nivel para gestión de los timers
 * FDguez. 20151216
 *
 */
#ifndef TIMERS_H
#define	TIMERS_H

#include "xc.h"
#include <stdint.h>
#include <stdbool.h>


/*
 * Timer catalog
 */
typedef enum {
    TIMER1,
    TIMER2,
    TIMER3,
} TIMERS;




/*
 * Dividers
 */
typedef enum {
    TIMER_X1,
    TIMER_X2,
    TIMER_X10,
    TIMER_X1K,
} TIMER_DIVIDERS;



/*
 * Prescalers 
 */
typedef enum {
    TMR1_DIV_BY_1 = 0x00,
    TMR1_DIV_BY_8 = 0x01,
    TMR1_DIV_BY_64 = 0x02,
    TMR1_DIV_BY_256 = 0x03
} TIMER1_PRESCALERS;

typedef enum {
    TMR23_DIV_BY_1 = 0x00,
    TMR23_DIV_BY_2 = 0x01,
    TMR23_DIV_BY_4 = 0x02,
    TMR23_DIV_BY_8 = 0x03,
    TMR23_DIV_BY_16 = 0x04,
    TMR23_DIV_BY_32 = 0x05,
    TMR23_DIV_BY_64 = 0x06,
    TMR23_DIV_BY_256 = 0x07
} TIMER23_PRESCALERS;




typedef struct {
    TIMERS timer;
    uint32_t counter;
    TIMER_DIVIDERS divider;
    bool changed;
    bool counter_on;
} TIMER_COUNTER_T;



typedef void (*TIMER_EXTERNAL_FUNC)();



typedef struct {
    TIMERS timer;
    TIMER_EXTERNAL_FUNC ext_func;
    TIMER_DIVIDERS div;
} TIMER_EXT_FUNC_T;


#define TIMER_MAX_COUNTERS     32
#define TIMER_MAX_EXTERNAL_FUNC     32


/**
 * Library initialization
 */
void TIMER_Initialize();

/*
 * Disable a timer
 */
void TIMER_Off(TIMERS t);


/*
 * Enable a timer
 */
void TIMER_On(TIMERS t);



/*
 * clears a timer
 */
void TIMER_Clear(TIMERS t);


/**
 * Obtains the value of the timer
 * @param t
 * @return 
 */
uint32_t TIMER_Get(TIMERS t);



/*
 * TIMER1 configuration
 * prescaler: sets the divider over PBCLK. 0=1:1, 1=1:8, 2=1:64, 3=1:256 
 * period: sets the value of the 16-bit counter to fire the interrupt (0..65535)
 * Example: 1-Second trigger for a PBCLK of 16MHz:
 *     prescaler = 3 (16MHz/256 = 62,5KHz => 16us)
 *     period = 62500 (16us * 62500 = 1s)
 * 
 */
void TIMER1_Initialize(TIMER1_PRESCALERS prescaler, uint16_t period);



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
void TIMER2_Initialize(TIMER23_PRESCALERS prescaler, uint32_t period, bool tmr32bits);


/*
 * TIMER3 configuration
 * prescaler: sets the divider over PBCLK. 0=1:1, 1=1:8, 2=1:64, 3=1:256 
 * period: sets the value of the 16-bit counter to fire the interrupt (0..65535)
 * Example: 1-Second trigger for a PBCLK of 16MHz:
 *     prescaler = 3 (16MHz/256 = 62,5KHz => 16us)
 *     period = 62500 (16us * 62500 = 1s)
 * 
 */
void TIMER3_Initialize(TIMER23_PRESCALERS prescaler, uint16_t period);


/* 
 * Adds a software counter
 * Returns 1 if an error occured
 */
uint8_t TIMER_Add_Counter(TIMER_COUNTER_T *counter);


/* 
 * Removes a software counter
 * Returns 1 if an error occured
 */
uint8_t TIMER_Del_Counter(TIMER_COUNTER_T *counter);
        

/* 
 * Adds a callback function associated to a timer
 * Returns 1 if an error occured
 */
uint8_t TIMER_Add_Function(TIMER_EXT_FUNC_T *func);


/* 
 * Removes a callback function associated to a timer
 * Returns 1 if an error occured
 */
uint8_t TIMER_Del_Function(TIMER_EXT_FUNC_T *func);


#endif // TIMERS32_H


