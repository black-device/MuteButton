// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright 2015 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license),
please contact mla_licensing@microchip.com
*******************************************************************************/
//DOM-IGNORE-END

#if !defined(USB_HAL_PIC32MM_H)
#define USB_HAL_PIC32MM_H

/*****************************************************************************/
/****** include files ********************************************************/
/*****************************************************************************/

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb_config.h"

/*****************************************************************************/
/****** Constant definitions *************************************************/
/*****************************************************************************/


//Device specific IECx register count.  Useful during interrupt context save and
//restore operations (such as prior to and after entering sleep on suspend).
#if defined(__32MM0256GPM064__) || defined(__32MM0128GPM064__) || defined(__32MM0064GPM064__) || \
    defined(__32MM0256GPM048__) || defined(__32MM0128GPM048__) || defined(__32MM0064GPM048__) || \
    defined(__32MM0256GPM036__) || defined(__32MM0128GPM036__) || defined(__32MM0064GPM036__) || \
    defined(__32MM0256GPM028__) || defined(__32MM0128GPM028__) || defined(__32MM0064GPM028__)

    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  4   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()              {TRISBbits.TRISB6 = 1;}

#else
    #warning "Add processor specific definitions here."
#endif




#if (USB_PING_PONG_MODE != USB_PING_PONG__FULL_PING_PONG)
    #error "PIC32 only supports full ping pong mode.  A different mode other than full ping pong is selected in the usb_config.h file."
#endif
#define BDT_NUM_ENTRIES      ((USB_MAX_EP_NUMBER + 1) * 4)


//----- USBEnableEndpoint() input definitions ----------------------------------
#define USB_HANDSHAKE_ENABLED           0x01
#define USB_HANDSHAKE_DISABLED          0x00

#define USB_OUT_ENABLED                 0x08
#define USB_OUT_DISABLED                0x00

#define USB_IN_ENABLED                  0x04
#define USB_IN_DISABLED                 0x00

#define USB_ALLOW_SETUP                 0x00
#define USB_DISALLOW_SETUP              0x10

#define USB_STALL_ENDPOINT              0x02

#define USB_PULLUP_ENABLE 0x00
#define USB_PULLUP_DISABLE 0x04

#define USB_INTERNAL_TRANSCEIVER        0x00
#define USB_EXTERNAL_TRANSCEIVER        0x01

#define USB_FULL_SPEED                  0x00
#define USB_LOW_SPEED                   0x08      //Note: Low speed device mode is only supported on certain devices.


#define USBPingPongBufferReset          U1CONbits.PPBRST

#define USB_OTG_ENABLE                  0x04
#define USB_OTG_DPLUS_ENABLE            0x80


//----- Interrupt Flag definitions --------------------------------------------
#define USBTransactionCompleteIE        U1IEbits.TRNIE
#define USBTransactionCompleteIF        U1IRbits.TRNIF
#define USBTransactionCompleteIFReg     U1IR
#define USBTransactionCompleteIFBitNum  3

#define USBResetIE                      U1IEbits.URSTIE
#define USBResetIF                      U1IRbits.URSTIF
#define USBResetIFReg                   U1IR
#define USBResetIFBitNum                0

#define USBIdleIE                       U1IEbits.IDLEIE
#define USBIdleIF                       U1IRbits.IDLEIF
#define USBIdleIFReg                    U1IR
#define USBIdleIFBitNum                 4

#define USBActivityIE                   U1OTGIEbits.ACTVIE
#define USBActivityIF                   U1OTGIRbits.ACTVIF
#define USBActivityIFReg                U1OTGIR
#define USBActivityIFBitNum             4

#define USBSOFIE                        U1IEbits.SOFIE
#define USBSOFIF                        U1IRbits.SOFIF
#define USBSOFIFReg                     U1IR
#define USBSOFIFBitNum                  2

#define USBStallIE                      U1IEbits.STALLIE
#define USBStallIF                      U1IRbits.STALLIF
#define USBStallIFReg                   U1IR
#define USBStallIFBitNum                7

#define USBErrorIE                      U1IEbits.UERRIE
#define USBErrorIF                      U1IRbits.UERRIF
#define USBErrorIFReg                   U1IR
#define USBErrorIFBitNum                1

#define USBSE0Event                     0// U1IRbits.URSTIF//  U1CONbits.SE0
#define USBSuspendControl               U1PWRCbits.USUSPEND
#define USBPacketDisable                U1CONbits.PKTDIS
#define USBResumeControl                U1CONbits.RESUME

#define USBT1MSECIE                     U1OTGIEbits.T1MSECIE
#define USBT1MSECIF                     U1OTGIRbits.T1MSECIF
#define USBT1MSECIFReg                  U1OTGIR
#define USBT1MSECIFBitNum               6

#define USBIDIE                         U1OTGIEbits.IDIE
#define USBIDIF                         U1OTGIRbits.IDIF
#define USBIDIFReg                      U1OTGIR
#define USBIDIFBitNum                   7

#define USBSESVDIE                      U1OTGIEbits.SESVDIE
#define USBSESVDIF                      U1OTGIRbits.SESVDIF
#define USBSESVDReg                     U1OTGIR
#define USBSESVDBitNum                  3

#define USBRESUMEIE                     U1IEbits.RESUMEIE
#define USBRESUMEIF                     U1IRbits.RESUMEIF
#define USBRESUMEIFReg                  U1IR
#define USBRESUMEIFBitNum               5

//----- Event call back definitions --------------------------------------------
#if defined(USB_DISABLE_SOF_HANDLER)
    #define USB_SOF_INTERRUPT           0x00
#else
    #define USB_SOF_INTERRUPT           0x04
#endif
#if defined(USB_DISABLE_ERROR_HANDLER)
    #define USB_ERROR_INTERRUPT         0x02
#else
    #define USB_ERROR_INTERRUPT         0x02
#endif

//STALLIE, IDLEIE, TRNIE, and URSTIE are all enabled by default and are required
#if defined(USB_INTERRUPT)
    #define USBEnableInterrupts() {\
        IEC0SET = _IEC0_USBIE_MASK;\
        IPC7CLR = 0x0000FF00;\
        IPC7SET = 0x00001000;\
        __builtin_enable_interrupts();\
    }
#else
    #define USBEnableInterrupts()
#endif

#define USBDisableInterrupts()      {IEC0CLR = _IEC0_USBIE_MASK;}

#if defined(USB_INTERRUPT)
    #define USBMaskInterrupts()     {IEC0CLR = _IEC0_USBIE_MASK;}
    #define USBUnmaskInterrupts()   {IEC0SET = _IEC0_USBIE_MASK;}
#else
    #define USBMaskInterrupts()
    #define USBUnmaskInterrupts()
#endif



//----- BDnSTAT bit definitions -----------------------------------------------
#define _BSTALL                         0x04        //Buffer Stall enable
#define _DTSEN                          0x08        //Data Toggle Synch enable
#define _DAT0                           0x00        //DATA0 packet expected next
#define _DAT1                           0x40        //DATA1 packet expected next
#define _DTSMASK                        0x40        //DTS Mask
#define _USIE                           0x80        //SIE owns buffer
#define _UCPU                           0x00        //CPU owns buffer
#define _STAT_MASK                      0xFC

//----- USTAT bit definitions -------------------------------------------------
#define USTAT_EP0_PP_MASK               ~0x04
#define USTAT_EP_MASK                   0xFC
#define USTAT_EP0_OUT                   0x00
#define USTAT_EP0_OUT_EVEN              0x00
#define USTAT_EP0_OUT_ODD               0x04
#define USTAT_EP0_IN                    0x08
#define USTAT_EP0_IN_EVEN               0x08
#define USTAT_EP0_IN_ODD                0x0C
#define ENDPOINT_MASK                   0xF0

//----- U1EP bit definitions --------------------------------------------------
#define UEP_STALL                       0x0002
// Cfg Control pipe for this ep
#define EP_CTRL                         0x0C
#define EP_OUT                          0x18            // Cfg OUT only pipe for this ep
#define EP_IN                           0x14            // Cfg IN only pipe for this ep
#define EP_OUT_IN                       0x1C            // Cfg both OUT & IN pipes for this ep
#define HSHK_EN                         0x01            // Enable handshake packet
                                                        // Handshake should be disable for isoch


#define BDT_BASE_ADDR_TAG   __attribute__ ((aligned (512)))
#define CTRL_TRF_SETUP_ADDR_TAG
#define CTRL_TRF_DATA_ADDR_TAG

//----- Depricated defintions - will be removed at some point of time----------
//--------- Depricated in v2.2
#define _LS                             0x00        // Use Low-Speed USB Mode
#define _FS                             0x00        // Use Full-Speed USB Mode
#define _TRINT                          0x00        // Use internal transceiver
#define _TREXT                          0x00        // Use external transceiver
#define _PUEN                           0x00        // Use internal pull-up resistor
#define _OEMON                          0x00        // Use SIE output indicator

/*****************************************************************************/
/****** Type definitions *****************************************************/
/*****************************************************************************/

// Buffer Descriptor Status Register layout.
typedef union __attribute__ ((packed)) _BD_STAT
{
    struct __attribute__ ((packed)){
        unsigned            :2;
        unsigned    BSTALL  :1;     //Buffer Stall Enable
        unsigned    DTSEN   :1;     //Data Toggle Synch Enable
        unsigned            :2;     //Reserved - write as 00
        unsigned    DTS     :1;     //Data Toggle Synch Value
        unsigned    UOWN    :1;     //USB Ownership
    };
    struct __attribute__ ((packed)){
        unsigned            :2;
        unsigned    PID0    :1;
        unsigned    PID1    :1;
        unsigned    PID2    :1;
        unsigned    PID3    :1;

    };
    struct __attribute__ ((packed)){
        unsigned            :2;
        unsigned    PID     :4;         //Packet Identifier
    };
    uint16_t           Val;
} BD_STAT;

// BDT Entry Layout
typedef union __attribute__ ((packed))__BDT
{
    struct __attribute__ ((packed))
    {
        BD_STAT     STAT;
        uint16_t        CNT:10;
        uint32_t       ADR;                      //Buffer Address
    };
    struct __attribute__ ((packed))
    {
        uint32_t       res  :16;
        uint32_t       count:10;
    };
    uint32_t           w[2];
    uint16_t            v[4];
    uint64_t           Val;
} BDT_ENTRY;

// USTAT Register Layout
typedef union __USTAT
{
    struct
    {
        unsigned char filler1           :2;
        unsigned char ping_pong         :1;
        unsigned char direction         :1;
        unsigned char endpoint_number   :4;
    };
    uint8_t Val;
} USTAT_FIELDS;

//Macros for fetching parameters from USTAT_FIELDS variable.
#define USBHALGetLastEndpoint(stat)     stat.endpoint_number
#define USBHALGetLastDirection(stat)    stat.direction
#define USBHALGetLastPingPong(stat)     stat.ping_pong


typedef union _POINTER
{
    struct
    {
        uint8_t bLow;
        uint8_t bHigh;
        //uint8_t bUpper;
    };
    uint16_t _uint16_t;                         // bLow & bHigh

    //pFunc _pFunc;                       // Usage: ptr.pFunc(); Init: ptr.pFunc = &<Function>;

    uint8_t* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    uint16_t* wRam;                         // Ram word pointer: 2 bytes pointer pointing
                                        // to 2 bytes of data

    const uint8_t* bRom;                     // Size depends on compiler setting
    const uint16_t* wRom;
    //const near uint8_t* nbRom;               // Near = 2 bytes pointer
    //const near uint16_t* nwRom;
    //const far uint8_t* fbRom;                // Far = 3 bytes pointer
    //const far uint16_t* fwRom;
} POINTER;

/*****************************************************************************/
/****** Function prototypes and macro functions ******************************/
/*****************************************************************************/


#define ConvertToPhysicalAddress(a)   ((uint32_t)KVA_TO_PA(a))
#define ConvertToVirtualAddress(a)    PA_TO_KVA1(a)

#if defined(__PIC32__)
    #if defined(_IFS1_USBIF_MASK)
        #define _ClearUSBIF()   {IFS1CLR = _ISF1_USBIF_MASK;}
        #define _SetUSBIF()     {IEC1SET = _IEC1_USBIE_MASK;}
		#define USBClearUSBInterrupt()        {IFS1CLR = _IFS1_USBIF_MASK;}
    #elif defined(_IFS0_USBIF_MASK)
        #define _ClearUSBIF()   {IFS0CLR = _IFS0_USBIF_MASK;}
        #define _SetUSBIE()     {IEC0SET = _IEC0_USBIE_MASK;}
		#define USBClearUSBInterrupt()        {IFS0CLR = _IFS0_USBIF_MASK;}
    #else
        #error Cannot clear USB interrupt.
    #endif
#endif


#ifndef KVA_TO_PA
    #define KVA_TO_PA(kva) ((uint32_t)(kva) & 0x1fffffff)
#endif

#ifndef PA_TO_KVA1
    #define PA_TO_KVA1(pa)    ((uint32_t)(pa) | 0xA0000000)
#endif

#define USBInterruptFlag        IFS0bits.USBIF
#define USBInterruptEnableBit   IEC0bits.USBIE
#define USBIPLBits              IPC7bits.USBIP


/********************************************************************
 * Function (macro): void USBClearInterruptFlag(register, uint8_t if_flag_offset)
 *
 * PreCondition:    None
 *
 * Input:
 *   register - the register mnemonic for the register holding the interrupt
 *				flag to be "kleared"
 *   uint8_t if_flag_offset - the bit position offset (for the interrupt flag to
 *							"klear") fconst the "right of the register"
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Klears the specified USB interrupt flag.
 *
 * Note:    		Individual USB interrupt flag bits are "Kleared" by writing
 *					'1' to the bit
 *******************************************************************/
#define USBClearInterruptFlag(reg_name, if_flag_offset)	(reg_name = (1 << if_flag_offset))




#if !defined(Sleep)
    #define REGISTER_UNLOCK_SEQUENCE()  {SYSKEY = 0; SYSKEY = 0xAA996655; SYSKEY = 0x556699AA;}
    #define REGISTER_LOCK_SEQUENCE()    {SYSKEY = 0;}
    #define WRITE_OSCCON(newVal)        {REGISTER_UNLOCK_SEQUENCE(); OSCCON = newVal; REGISTER_LOCK_SEQUENCE();}
    #define Sleep()                     {WRITE_OSCCON(OSCCON | 0x00000010); _wait();}       //Set OSCCON<SLPEN> = 1 (to sleep instead of idle), then execute wait instruction)
#endif


#define SetConfigurationOptions()       {U1CNFG1 = USB_SPEED_OPTION; U1EIE = 0x9F; U1IE = 0x99 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT; U1OTGCON &= 0x000F;  U1OTGCON |= USB_PULLUP_OPTION;}


/********************************************************************
Function:
    bool USBSleepOnSuspend(void)

Summary:
    Places the core into sleep and sets up the USB module
    to wake up the device on USB activity (either USB host resume or
    USB VBUS going away, such as due to USB cable detach).

PreCondition:
    The USBDeviceInit() function should have been called at least once and the
    USB host should have suspended the USB device.

Parameters:
    None

Return Values:
    true  - if entered sleep successfully
    false - if there was an error entering sleep

Remarks:
    Please note that before calling this function that it is the
    responsibility of the application to place all of the other
    peripherals or board features into a lower power state if
    required.

    Based on the USB specifications, upon detection of the suspend condition, a
    USB device should promptly put itself into a low power mode (such that it consumes
    no more than 2.5mA from the USB host (unless the host is a USB type-C host and
    is actively advertising higher than standard USB spec current capability).

*******************************************************************/
bool USBSleepOnSuspend(void);

/****************************************************************
    Function:
        void USBPowerModule(void)

    Description:
        This macro is used to power up the USB module if required:
        PIC16: defines as nothing
        PIC18: defines as nothing
        PIC24: defines as U1PWRCbits.USBPWR = 1;
        PIC32: defines as U1PWRCbits.USBPWR = 1;

    Parameters:
        None

    Return Values:
        None

    Remarks:
        None

  ****************************************************************/
#define USBPowerModule() U1PWRCbits.USBPWR = 1;

/****************************************************************
    Function:
        void USBModuleDisable(void)

    Description:
        This macro is used to disable the USB module.  This will perform a
        USB soft detach operation from the host, if the device was already plugged
        into the host at the time of calling this function.  All USB module
        features including the internal USB 1ms timer and the USB VBUS
        monitoring comparators will be disabled.

    Parameters:
        None

    Return Values:
        None

    Remarks:
        None

  ****************************************************************/
#define USBModuleDisable() {\
    U1CON = 0;\
    U1IE = 0;\
    U1OTGIE = 0;\
    U1PWRCbits.USUSPND = 0;\
    U1PWRCbits.USBPWR = 0;\
    USBDeviceState = DETACHED_STATE;\
}

/****************************************************************
    Function:
        USBSetBDTAddress(addr)

    Description:
        This macro is used to power up the USB module if required

    Parameters:
        None

    Return Values:
        None

    Remarks:
        None

  ****************************************************************/
#define USBSetBDTAddress(addr)         {U1BDTP3 = (((uint32_t)KVA_TO_PA(addr)) >> 24); U1BDTP2 = (((uint32_t)KVA_TO_PA(addr)) >> 16); U1BDTP1 = (((uint32_t)KVA_TO_PA(addr)) >> 8);}

/****************************************************************
    Function:
        void USBClearInterruptRegister(int register)

    Description:
        Clears all of the interrupts in the requested register

    Parameters:
        register - the register that needs to be cleared.

    Return Values:
        None

    Remarks:
        Note that on these devices to clear an interrupt you must
        write a '1' to the interrupt location.

  ****************************************************************/
#define USBClearInterruptRegister(reg)  {reg = 0xFFFFFFFF;}

/********************************************************************
    Function:
        void USBClearInterruptFlag(register, uint8_t if_flag_offset)

    Summary:
        Clears the specified USB interrupt flag.

    PreCondition:
        None

    Parameters:
        register - the register mnemonic for the register holding the interrupt
                   flag to be cleared
        uint8_t if_flag_offset - the bit position offset (for the interrupt flag to
                   clear) from the "right of the register"

    Return Values:
        None

    Remarks:
        Individual USB interrupt flag bits are cleared by writing '1' to the
        bit, in a word write operation.

 *******************************************************************/
#define USBClearInterruptFlag(reg_name, if_flag_offset)	(reg_name = (1 << if_flag_offset))

/********************************************************************
    Function:
        void DisableNonZeroEndpoints(UINT8 last_ep_num)

    Summary:
        Clears the control registers for the specified non-zero endpoints

    PreCondition:
        None

    Parameters:
        UINT8 last_ep_num - the last endpoint number to clear.  This
        number should include all endpoints used in any configuration.

    Return Values:
        None

    Remarks:
        None

 *******************************************************************/
#define DisableNonZeroEndpoints(last_ep_num)          {\
            uint8_t i;\
            uint32_t *p = (uint32_t*)&U1EP1;\
            for(i=0;i<last_ep_num;i++)\
            {\
                *p = 0;\
                p += 4;\
            }\
        }

/********************************************************************
Function:
    bool USBRemoteWakeupAssertBlocking(void)

Summary:
    Checks if it is currently legal to send remote wakeup signaling to the
    host, and if so, it sends it.  This is a blocking function that takes ~5-6ms
    to execute.

PreCondition:

Parameters:
    None

Return Values:
    true  - if it was legal to send remote wakeup signaling and the signaling was sent
    false - if it was not legal to send remote wakeup signaling (in this case, no signaling gets sent)

Remarks:
    To successfully use remote wakeup in an application, the device must first
    let the host know that it supports remote wakeup (by setting the appropriate
    bit in the attributes field of the USB configuration descriptor).  Additionally,
    the end user must allow/enable remote wakeup for the device.  Hosts are not
    obligated to always allow remote wakeup, and the end user can typically enable/disable this
    feature for each device at their discretion.  Under Windows, devices supporting
    remote wakeup will normally have an extra check-box setting under the device
    manager properties pages for the device, which the user can change.

    Additionally, remote wakeup capability requires driver support, in the USB
    class drivers being used for the device on the host.  Under Windows 7, the CDC
    drivers do not support remote wakeup capability, while other drivers like HID
    and WinUSB fully support this capability.  If the intended application is
    CDC or based on some other driver not supporting remote wakeup, it is potentially
    possible to make the device a composite device (ex: CDC+HID) and then use the
    HID driver as a means of achieving the remote wakeup effect of the host.

    Sometime prior to entering USB suspend (usually just prior), if the host and
    driver(s) support remote wakeup, and the user has enabled the feature for the
    device, then the host will send notification to the device firmware letting it
    know that remote wakeup is legal.  The USBGetRemoteWakeupStatus() API function
    can be used to check if the host has allowed remote wakeup or not.  The application
    firmware should not send remote wakeup signaling to the host, unless both
    USBGetRemoteWakeupStatus() and USBIsBusSuspended() return true.  Attempting to
    send the remote wakeup signaling to the host when it is not legal to do so
    will normally not wake up the host, and will instead create a violation of
    the USB compliance criteria.  Additionally, devices should not send remote
    wakeup signaling immediately upon entry into the suspended bus state.  Based on
    the USB specifications, the bus must be continuously idle for at least 5ms before
    a device may send remote wakeup signaling.  After the signaling is sent, the
    device must become fully ready for normal USB communication/request
    processing within 10ms.
 *******************************************************************/
bool USBRemoteWakeupAssertBlocking(void);


/********************************************************************
Function:
    int8_t USBVBUSSessionValidStateGet(bool AllowInvasiveReads)

Summary:
    This function tries to check the current VBUS voltage level to see if it is
    above the session valid threshold or not.  This is useful for detecting if the
    device is currently attached to a host or hub that is actively supplying
    power on the +5V VBUS net (instead of being powered down).

PreCondition:
    This function assumes that USBDeviceInit() has been called at least once,
    although it is not necessary for the USB module to be already enabled, if
    "invasive reads" are allowed.  However, it is always the caller's responsibility
    to make sure that the microcontroller system clock settings have been
    pre-configured for an operating mode that are compatible with USB operation
    at the right frequency (ex: the USB module must have 48MHz clocking available
    internally), prior to calling this function.

Parameters:
    bool AllowInvasiveReads - Specify false if you want this function to perform
    a "passive" read attempt, which will not block or change the state of the USB module.
    If this parameter is false, the function can return a negative number, indicating
    that the read could not be performed (ex: if the comparators are currently off).

    Specify true, if you want this function to perform a forceful read operation.
    In this case, this function will turn on the USB module and/or unsuspend it if
    needed, in order to read the actual value.  The function will always return 0 or 1
    in this case.  However, the function may block for as much time as required to
    ensure that any necessary analog startup/settling/propagation times have
    elapsed, so as to get an accurate reading.  If invasive reads are allowed, and
    this function turns on the USB module or unsuspends it, the module will remain
    on and unsuspended subsequent to returning from this function.
    It is the caller's responsibility to turn the USB module off if
    desired/appropriate for the application (ex: because the returned value was
    0, indicating VBUS is not currently powered, in which case the application may
    wish to shut things down for lower power consumption).


Return Values:
    Returns a signed 8-bit byte indicating the current VBUS level.  If VBUS is above
    the session valid state (ex: because the cable is attached and a host or hub is actively
    sourcing VBUS power), the return value will be 1.  If the VBUS level is below
    the session valid level (ex: because the cable is detached or the host/hub is
    powered down and not sourcing anything on VBUS), the return value will be 0.
    If the function could not conclusively determine the state of the VBUS line
    (ex: because the VBUS sensing comparator wasn't powered on for example), the
    return value will be a negative number (ex: return value of -1 or less, with
    the actual value indicating a code representing the reason why the value
    couldn't be read).

Remarks:
    This function may block for upwards of ~5ms if AllowInvasiveReads was true
    and the USB module wasn't previously turned or or was recently (or is still) suspended.

 *******************************************************************/
int8_t USBVBUSSessionValidStateGet(bool AllowInvasiveReads);



/********************************************************************
Function:
    void USBMaskAllUSBInterrupts(void)

Summary:
    This function saves the current USBIE bit state, and then clears it
    to prevent any USB interrupt vectoring.

PreCondition:
    None

Parameters:
    None

Return Values:
    None

Remarks:
     This function should always be called in a exact 1:1 ratio with the
     corresponding USBRestoreUSBInterrupts() function (which restores the
     setting saved by this function).  This function should not be called
     more than once, prior to calling USBRestoreUSBInterrupts(),
     as this will cause the previously saved value to get overwritten.
  *******************************************************************/
void USBMaskAllUSBInterrupts(void);



/********************************************************************
Function:
    void USBRestoreUSBInterrupts(void)

Summary:
    This function restores the previous USB interrupt setting that was
    in effect prior to calling the corresponding USBMaskAllUSBInterrupts().

PreCondition:
    None

Parameters:
    None

Return Values:
    None

Remarks:
 This function should only be called in an exact 1:1 ratio with the
 USBMaskAllUSBInterrupts() function.  This function should never be
 called without first being preceded by a call to USBMaskAllUSBInterrupts().
  *******************************************************************/
void USBRestoreUSBInterrupts(void);





/*****************************************************************************/
/****** Compiler checks ******************************************************/
/*****************************************************************************/
#ifndef USB_PING_PONG_MODE
    #error "No ping pong mode defined."
#endif

/*****************************************************************************/
/****** Extern variable definitions ******************************************/
/*****************************************************************************/

#if defined(USB_SUPPORT_DEVICE) | defined(USB_SUPPORT_OTG)
    #if !defined(USBDEVICE_C)
        //extern USB_VOLATILE USB_DEVICE_STATE USBDeviceState;
        extern USB_VOLATILE uint8_t USBActiveConfiguration;
        extern USB_VOLATILE IN_PIPE inPipes[1];
        extern USB_VOLATILE OUT_PIPE outPipes[1];
    #endif
	extern volatile BDT_ENTRY* pBDTEntryOut[USB_MAX_EP_NUMBER+1];
	extern volatile BDT_ENTRY* pBDTEntryIn[USB_MAX_EP_NUMBER+1];
#endif

#endif  //USB_HAL_PIC32MM_H
