/*
 * Hardware Initialization
 *
 */

#ifndef HARDWARE_H__
#define HARDWARE_H__

#include <xc.h>
#include <stdbool.h>

typedef enum {
    PIN_BIT_0  = 0x00000001,
    PIN_BIT_1  = 0x00000002,
    PIN_BIT_2  = 0x00000004,
    PIN_BIT_3  = 0x00000008,
    PIN_BIT_4  = 0x00000010,
    PIN_BIT_5  = 0x00000020,
    PIN_BIT_6  = 0x00000040,
    PIN_BIT_7  = 0x00000080,
            
    PIN_BIT_8  = 0x00000100,
    PIN_BIT_9  = 0x00000200,
    PIN_BIT_10 = 0x00000400,
    PIN_BIT_11 = 0x00000800,
    PIN_BIT_12 = 0x00001000,
    PIN_BIT_13 = 0x00002000,
    PIN_BIT_14 = 0x00004000,
    PIN_BIT_15 = 0x00008000,
            
    PIN_BIT_16 = 0x00010000,
    PIN_BIT_17 = 0x00020000,
    PIN_BIT_18 = 0x00040000,
    PIN_BIT_19 = 0x00080000,
    PIN_BIT_20 = 0x00100000,
    PIN_BIT_21 = 0x00200000,
    PIN_BIT_22 = 0x00400000,
    PIN_BIT_23 = 0x00800000,
            
    PIN_BIT_24 = 0x01000000,
    PIN_BIT_25 = 0x02000000,
    PIN_BIT_26 = 0x04000000,
    PIN_BIT_27 = 0x08000000,
    PIN_BIT_28 = 0x10000000,
    PIN_BIT_29 = 0x20000000,
    PIN_BIT_30 = 0x40000000,
    PIN_BIT_31 = 0x80000000,  
} PIN_BIT_T;

typedef enum {
    GPIO_PORT_A = 0,
    GPIO_PORT_B = 1,
    GPIO_PORT_C = 2,
    GPIO_PORT_D = 3,
    GPIO_NO_PORT = 0xff,
} GPIO_PORT_T;


typedef enum {
    GPIO_PULLUP = 0,
    GPIO_PULLDOWN = 1,
    GPIO_PULLFLOAT = 2,
    GPIO_ANALOG = 3,
} GPIO_INPUT_CFG_T;





typedef struct {
    GPIO_PORT_T port;
    PIN_BIT_T pin_bit;
} GPIO_T;


typedef struct {
    uint8_t udid[20];
    uint32_t crc32;
}PIC_UDID_t;



#define WATCHDOG_CLR_KEY                0x5743

/*
 * Core speed. In Hz
 */
#define SYS_FREQ        (24000000L)//(37500000L)

   
/*
 * Peripheral bus speed
 */
#define PBCLK             (SYS_FREQ/1)//(SYS_FREQ / 2)


/*
 * Ticks per us
 */
#define CT_US_COUNT     ((SYS_FREQ / 2) / 1000000)


/*
 * Ticks per ms
 */
#define CT_MS_COUNT     ((SYS_FREQ / 2) / 1000)



/* Initialize the hardware.  This of course varies widely by MCU and board */
void hardware_init(void);


void INT_Global_Disable(void);


void INT_Global_Enable(void);


void Delay_us(uint16_t delay);

void Delay_ms(uint16_t delay);

 
/* Enables Watch Dog Timer (WDT) using the software bit.
 * @example
 * <code>
 * WATCHDOG_TimerSoftwareEnable();
 * </code>
 */
inline static void WDT_SoftwareEnable(void) { WDTCONbits.ON = 1; }


/* Disables Watch Dog Timer (WDT) using the software bit.
 * @example
 * <code>
 * WATCHDOG_TimerSoftwareDisable();
 * </code>
 */
inline static void WDT_SoftwareDisable(void) { WDTCONbits.ON = 0; }


/* Clears the Watch Dog Timer (WDT).
 * @example
 * <code>
 * WATCHDOG_TimerClear();
 * </code>
 */
inline static void WDT_Clear(void) { WDTCONbits.WDTCLRKEY = WATCHDOG_CLR_KEY; }

inline static void SYSTEM_RegUnlock(void) {
    SYSKEY = 0x0; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write Key1 to SYSKEY
    SYSKEY = 0x556699AA; //write Key2 to SYSKEY
}


inline static void SYSTEM_RegLock(void) { SYSKEY = 0x00000000; }


void set_pin(GPIO_T p, uint8_t v);

uint8_t get_pin(GPIO_T p);

uint8_t get_pin_port_value(GPIO_T p);

uint8_t get_pin_latch_value(GPIO_T p);

void set_pin_cfg_input(GPIO_T p, GPIO_INPUT_CFG_T pull);

void set_pin_open_drain(GPIO_T p, bool open_drain_enabled);

void soft_reset();

void PIC_Get_UDID(PIC_UDID_t* ret);

#endif /* HARDWARE_H__ */
