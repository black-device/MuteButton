#include "app_config.h"


static const uint32_t __attribute__((space(prog),address(app_version_addr))) app_version = ((uint32_t)Software_Version)<<16 | (uint32_t)Hardware_Version;


/**
 Pin configuration
 */
const GPIO_T rs485_tx_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_7 };
const GPIO_T rs485_tx_en_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_14 };
const GPIO_T rs485_rx_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_15 };
const GPIO_T bus_sel_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_13 };

const GPIO_T rs485_tx_led_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_12 };

const GPIO_T rgb_led_r_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_4 };
const GPIO_T rgb_led_g_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_4 };
const GPIO_T rgb_led_b_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_2 };

const GPIO_T dimmer_1_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_3 };
const GPIO_T dimmer_2_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_9 };

const GPIO_T presence_led_pin = { .port = GPIO_PORT_D, .pin_bit = PIN_BIT_0 };
const GPIO_T banner_led_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_3 };

const GPIO_T h_phase_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_2 };
const GPIO_T l_phase_pin = { .port = GPIO_PORT_B, .pin_bit = PIN_BIT_2 };

const GPIO_T encoder_1_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_1 };
const GPIO_T encoder_2_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_0 };
const GPIO_T encoder_4_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_8 };
const GPIO_T encoder_8_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_5 };

const GPIO_T button_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_4 };

//const GPIO_T led_status_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_4 };
//const GPIO_T led_activity_pin = { .port = GPIO_PORT_C, .pin_bit = PIN_BIT_5 };
//const GPIO_T push_pin = { .port = GPIO_PORT_A, .pin_bit = PIN_BIT_6 };
    




/**
 * Library initialization
 */
void APP_CFG_Initialize() {
    
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
    LATA = 0x0000;
    LATB = 0x0000;
    LATC = 0x0000;
    LATD = 0x0000;

    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISA = 0x85CB;
    TRISB = 0xEC47;
    TRISC = 0x113B;
    TRISD = 0x0001;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
    CNPDA = 0x0000;
    CNPDB = 0x0000;
    CNPDC = 0x0000;
    CNPDD = 0x0000;
    CNPUA = 0x0000;
    CNPUB = 0x0000;
    CNPUC = 0x0000;
    CNPUD = 0x0000;

    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
    ODCA = 0x0000;
    ODCB = 0x0000;
    ODCC = 0x0000;
    ODCD = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    ANSELA = ANSELB = ANSELC = 0x0000;
    set_pin_cfg_input((GPIO_T){GPIO_PORT_A, PIN_BIT_0}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_A, PIN_BIT_1}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_A, PIN_BIT_3}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_A, PIN_BIT_6}, GPIO_ANALOG);
    
    //set_pin_cfg_input((GPIO_T){GPIO_PORT_B, PIN_BIT_2}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_B, PIN_BIT_13}, GPIO_ANALOG);
    //set_pin_cfg_input((GPIO_T){GPIO_PORT_B, PIN_BIT_14}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_B, PIN_BIT_15}, GPIO_ANALOG);
    
    set_pin_cfg_input((GPIO_T){GPIO_PORT_C, PIN_BIT_0}, GPIO_ANALOG);
    set_pin_cfg_input((GPIO_T){GPIO_PORT_C, PIN_BIT_1}, GPIO_ANALOG); // AN13
    set_pin_cfg_input((GPIO_T){GPIO_PORT_C, PIN_BIT_5}, GPIO_ANALOG); // AN15 -> pot 2
    set_pin_cfg_input((GPIO_T){GPIO_PORT_C, PIN_BIT_8}, GPIO_ANALOG); // AN14 -> pot 1

    /****************************************************************************
     * Set the PPS
     ***************************************************************************/
    SYSTEM_RegUnlock(); // unlock PPS
    RPCONbits.IOLOCK = 0;
    
    // UART2 setup 
    RPINR9bits.U2RXR = 0b10001;  // RP17/RB15->UART2:U2RX
    RPOR5bits.RP21R = 0x0004;    // RA7<-UART2:U2TX

    // Sensor wires
    RPOR3bits.RP13R = 0x0001;    //RB8<-CMP1:C1OUT [l_swpulse]
    RPOR2bits.RP12R = 0x0011;    //RB7<-CLC1:CLC1OUT [h_swpulse]
    RPOR3bits.RP14R = 0x0011;    //RB9<-CLC1:CLC1OUT [vt_pulse]
    
    RPINR12bits.CLCINAR = 0b01000; // RP8/RB2->CLCINA [l_phase]
    RPINR12bits.CLCINBR = 0b10011; // RP19/RC2->CLCINB [h_phase]
    
    RPOR5bits.RP23R = 18;    //RC6<-CLC2OUT IO_C6 [**DEBUG**]
    
    // Relay output
    RPOR2bits.RP11R = 0x000D;    //RB5<-SCCP6:OCM6
    
    // RGB Leds
    RPOR0bits.RP3R = 0x000C;    //RA2<-SCCP5:OCM5 [RBG Led: Blue]
    RPOR2bits.RP10R = 0x000B;    //RB4<-SCCP4:OCM4 [RBG Led: Red]
    RPOR1bits.RP5R = 15;    //RA4<-SCCP8:OCM8 [RGB Led: Green]
    
    // Dimmers
    RPOR2bits.RP9R = 14;    //RB3<-SCCP7:OCM7 [Dimmer 1]
    RPOR5bits.RP24R = 16;    //RA9<-SCCP9:OCM9 [Dimmer 2]
    //RPOR5bits.RP24R = 0;
    
    //_______
    

    RPCONbits.IOLOCK = 1; // lock   PPS
    SYSTEM_RegLock();
    
    
    // setup the RS485 lines (AN off; input/output)
    set_pin(rs485_tx_pin, 0);
    set_pin(rs485_tx_en_pin, 0);
    get_pin(rs485_rx_pin);
    
    set_pin(rgb_led_r_pin, 0);
    set_pin(rgb_led_g_pin, 0);
    set_pin(rgb_led_b_pin, 0);
    
    set_pin(dimmer_1_pin, 0);
    set_pin(dimmer_2_pin, 0);
    
    set_pin(presence_led_pin, 0);
    set_pin(banner_led_pin, 0);
    
    get_pin(h_phase_pin);
    get_pin(l_phase_pin);

}
