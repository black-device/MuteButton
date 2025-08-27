/*
 * Hardware Initialization
 *
 */

#include <xc.h>
#include <stdint.h>
#include "hardware.h"
#include "crc/crc.h"


static volatile uint32_t* port_addr[] = { &PORTA, &PORTB, &PORTC, &PORTD };
static volatile uint32_t* tris_addr[] = { &TRISA, &TRISB, &TRISC, &TRISD };
static volatile uint32_t* lat_addr[] = { &LATA, &LATB, &LATC, &LATD };
static volatile uint32_t* an_addr[] = { &ANSELA, &ANSELB, &ANSELC, &ANSELD };
static volatile uint32_t* pullup_addr[] = { &CNPUA, &CNPUB, &CNPUC, &CNPUD };
static volatile uint32_t* pulldown_addr[] = { &CNPDA, &CNPDB, &CNPDC, &CNPDD };
static volatile uint32_t* odc_addr[] = { &ODCA, &ODCB, &ODCC, &ODCD };

static uint32_t IEC_bkp[4];

#ifdef BOOT_WITHOUT_BOOTLOADER //··· BOOT_WITHOUT_BOOTLOADER defined in project profiles
#if defined(__32MM0256GPM064__) || defined(__32MM0256GPM048__)

// FDEVOPT
#pragma config SOSCHP = OFF             //Secondary Oscillator High Power Enable bit->SOSC oprerates in normal power mode.
#pragma config ALTI2C = OFF             //Alternate I2C1 Pins Location Enable bit->Primary I2C1 pins are used
#pragma config FUSBIDIO = ON           
#pragma config FVBUSIO = ON            

// FICD
#pragma config JTAGEN = OFF             //JTAG Enable bit->JTAG is disabled
#pragma config ICS = PGx1               //ICE/ICD Communication Channel Selection bits->Communicate on PGEC2/PGED2

// FPOR
#pragma config BOREN = BOR3             //Brown-out Reset Enable bits->Brown-out Reset enabled in hardware; SBOREN bit disabled
#pragma config RETVR = OFF              //Retention Voltage Regulator Enable bit->Retention regulator is disabled
#pragma config LPBOREN = ON             //Downside Voltage Protection Enable bit->Low power BOR is enabled, when main BOR is disabled

// FWDT
#pragma config SWDTPS = PS1048576       //Sleep Mode Watchdog Timer Postscale Selection bits->1:1048576
#pragma config FWDTWINSZ = PS25_0       //Watchdog Timer Window Size bits->Watchdog timer window size is 25%
#pragma config WINDIS = OFF             //Windowed Watchdog Timer Disable bit->Watchdog timer is in non-window mode
#pragma config RWDTPS = PS1048576       //Run Mode Watchdog Timer Postscale Selection bits->1:1048576
#pragma config RCLKSEL = LPRC           //Run Mode Watchdog Timer Clock Source Selection bits->Clock source is LPRC (same as for sleep mode)
#pragma config FWDTEN = OFF             //Watchdog Timer Enable bit->WDT is disabled

// FOSCSEL
#pragma config FNOSC = FRCDIV           //Oscillator Selection bits->FRCDIV
#pragma config PLLSRC = FRC             //System PLL Input Clock Selection bit->FRC oscillator is selected as PLL reference input on device reset
#pragma config SOSCEN = OFF             //Secondary Oscillator Enable bit->Secondary oscillator is disbled
#pragma config IESO = ON                //Two Speed Startup Enable bit->Two speed startup is enabled
#pragma config POSCMOD = OFF            //Primary Oscillator Selection bit->Primary oscillator is disabled
#pragma config OSCIOFNC = OFF           //System Clock on CLKO Pin Enable bit->OSCO pin operates as a normal I/O
#pragma config SOSCSEL = ON            //Secondary Oscillator External Clock Enable bit->SCLKI for digital mode
#pragma config FCKSM = CSECMD           //Clock Switching and Fail-Safe Clock Monitor Enable bits->Clock switching is enabled; Fail-safe clock monitor is disabled

// FSEC
#pragma config CP = OFF                 //Code Protection Enable bit->Code protection is disabled

#else
	#error "Config flags for your device not defined"
#endif
#endif

void hardware_init(void) {
#ifdef BOOT_WITHOUT_BOOTLOADER

    /* refer to DS60001112 6.3.7.2 OSCILLATOR SWITCHING SEQUENCE */
    /* Perform unlock sequence of SYSKEY writes (no interrupts or dma allowed while doing this) */
    SYSTEM_RegUnlock();
    OSCTUN = 0x9000; /* FRC osc runs at nominal 8MHz tuned by USB 1ms start-of-frame */
    SPLLCON = 0x2050080; /* PLLODIV 1:4; PLLMULT 12x; PLLICLK FRC; => 24MHz SPLL system clock */
    /* Note the 96MHz to the USB module is internally divided to 48MHz */
    PWRCON = 0x00;
    OSCCON = (0x100 | _OSCCON_OSWEN_MASK); /* OSCCONbits.NOSC = 1; PLL clock source, SOSCEN disabled */
    
    SYSTEM_RegLock();
    
    while (OSCCONbits.OSWEN == 1); /* wait for clock switch */
    while (CLKSTATbits.SPLLRDY != 1);
    REFO1CON = 0x00; /* reference oscillator output disabled */
    REFO1TRIM = 0x00;
#endif
    
    SYSTEM_RegUnlock();
    OSCTUN = 0x9000; /* FRC osc runs at nominal 8MHz tuned by USB 1ms start-of-frame */
    SYSTEM_RegLock();
    
    /* Configure interrupts, per architecture */
    // Enable Multi Vector Configuration
    INTCONbits.MVEC = 1;

    // USBI: USB
    // Priority: 1
    IPC7bits.USBIP = 1;
    // Sub Priority: 0
    IPC7bits.USBIS = 0;
    __asm volatile("ei");
}


void INT_Global_Disable(void) { 
    IEC_bkp[0] = IEC0;
    IEC_bkp[1] = IEC1;
    IEC_bkp[2] = IEC2;
    IEC_bkp[3] = IEC3;
    IEC0 = IEC1 = IEC2 = IEC3 = 0;
}

void INT_Global_Enable(void) {    
    IEC0 |= IEC_bkp[0];
    IEC1 |= IEC_bkp[1];
    IEC2 |= IEC_bkp[2];
    IEC3 |= IEC_bkp[3];
}

void Delay_us(uint16_t delay) {
     uint32_t stop_time = CT_US_COUNT * delay;
     uint32_t start_time = _CP0_GET_COUNT();
    while ((_CP0_GET_COUNT() - start_time) < (stop_time)); // wait for timer to expire
 }

 
 void Delay_ms(uint16_t delay) {
     uint32_t stop_time = CT_MS_COUNT * delay;
     uint32_t start_time = _CP0_GET_COUNT();
     while ((_CP0_GET_COUNT() - start_time) < (stop_time)); // wait for timer to expire
 }


void set_pin_cfg_input(GPIO_T p, GPIO_INPUT_CFG_T pull) {
    if (p.port == GPIO_NO_PORT) return;
    
    volatile uint32_t* ptr_pullup = pullup_addr[p.port];
    volatile uint32_t* ptr_pulldown = pulldown_addr[p.port];
    volatile uint32_t* ptr_an = an_addr[p.port];
    volatile uint32_t* ptr_tris = tris_addr[p.port];
    
    *ptr_pullup &= (~p.pin_bit); // pull-up pin OFF
    *ptr_pulldown &= (~p.pin_bit); // pull-down pin OFF
    *ptr_an &= (~p.pin_bit); // analog -> OFF
    *ptr_tris |= p.pin_bit; // tris -> input
    
    if (pull == GPIO_PULLUP) *ptr_pullup |= p.pin_bit; // pull-up pin ON
    else if (pull == GPIO_PULLDOWN) *ptr_pulldown |= p.pin_bit; // pull-down pin ON
    else if (pull == GPIO_ANALOG) *ptr_an |= p.pin_bit; // analog -> ON
    
}


void set_pin_open_drain(GPIO_T p, bool open_drain_enabled) {
    if (p.port == GPIO_NO_PORT) return;
    
    volatile uint32_t* ptr_odc = odc_addr[p.port];
    
    if (open_drain_enabled) *ptr_odc |= p.pin_bit;
    else *ptr_odc &= (~p.pin_bit);
}


void set_pin(GPIO_T p, uint8_t v) {
    if (p.port == GPIO_NO_PORT) return;
    
    volatile uint32_t* ptr_lat = lat_addr[p.port];
    volatile uint32_t* ptr_tris = tris_addr[p.port];
    volatile uint32_t* ptr_an = an_addr[p.port];
    
    *ptr_an &= (~p.pin_bit); // analog -> OFF
    *ptr_tris &= (~p.pin_bit); // tris -> output
    if (v) *ptr_lat |= p.pin_bit; // set LAT pin to 1
    else *ptr_lat &= (~p.pin_bit); // set LAT pin to 0
}


uint8_t get_pin(GPIO_T p) {
    if (p.port == GPIO_NO_PORT) return 0;
    
    volatile uint32_t* ptr_port = port_addr[p.port];
    volatile uint32_t* ptr_tris = tris_addr[p.port];
    volatile uint32_t* ptr_an = an_addr[p.port];
    
    *ptr_an &= (~p.pin_bit); // analog -> OFF
    *ptr_tris |= p.pin_bit; // tris -> input
    return (*ptr_port & p.pin_bit) != 0; // read PORT pin
}


uint8_t get_pin_port_value(GPIO_T p) {
    if (p.port == GPIO_NO_PORT) return 0;
    
    volatile uint32_t* ptr_port = port_addr[p.port];
    return (*ptr_port & p.pin_bit) != 0; // read PORT pin
}


uint8_t get_pin_latch_value(GPIO_T p) {
    if (p.port == GPIO_NO_PORT) return 0;
    
    volatile uint32_t* ptr_lat = lat_addr[p.port];
    return (*ptr_lat & p.pin_bit) != 0; // read LATCH pin
}


void soft_reset() {
    /* perform a system unlock sequence */
    // starting critical sequence
    SYSKEY = 0x00000000; //write invalid key to force lock
    SYSKEY = 0xAA996655; //write key1 to SYSKEY
    SYSKEY = 0x556699AA; //write key2 to SYSKEY

    /* set SWRST bit to arm reset */
    RSWRSTSET = 1;

    /* read RSWRST register to trigger reset */
    unsigned int dummy; dummy = RSWRST;
    /* prevent any unwanted code execution until reset occurs*/
    while(1);
}


void PIC_Get_UDID(PIC_UDID_t* ret) {
    uint32_t* ptr = (uint32_t*)0xBFC41840;
    uint8_t* ret_ptr = ret->udid;
    while (1) {
        uint32_t v = *ptr;
        for (int i = 0; i < 4; i++) {
            *(ret_ptr++) = (v & 0xff);
            v >>= 8;
        }
        
        ptr++;
        if (ptr > (uint32_t*)0xBFC41850) break;
    }
    ret->crc32 = get_new_crc32(ret->udid, sizeof(ret->udid));
}

