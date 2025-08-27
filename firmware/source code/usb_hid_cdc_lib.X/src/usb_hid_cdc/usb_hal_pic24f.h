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

#ifndef USB_HAL_PIC24F_H
#define USB_HAL_PIC24F_H

/*****************************************************************************/
/****** include files ********************************************************/
/*****************************************************************************/

#include <xc.h>

#include <stdint.h>
#include <string.h>

#include "usb_config.h"
#include "usb_common.h"

/*****************************************************************************/
/****** Constant definitions *************************************************/
/*****************************************************************************/


//Device specific IECx register count.  Useful during interrupt context save and
//restore operations (such as prior to and after entering sleep on suspend).
//IEC0-IEC5 on PIC24FJ64GB004 Family devices
//IEC0-IEC5 on PIC24FJ256GB110 Family devices
//IEC0-IEC5 on PIC24FJ256GB210 Family devices
//IEC0-IEC6 on PIC24FJ256DA210 Family devices
//IEC0-IEC7 on PIC24FJ128GC010 Family devices
//IEC0-IEC7 on PIC24FJ128GB204 Family devices
//IEC0-IEC7 on PIC24FJ256GB412 Family devices
#if defined(__PIC24FJ64GB004__) || defined(__PIC24FJ64GB002__) || defined(__PIC24FJ32GB004__) || defined(__PIC24FJ32GB002__)    \
    || defined(__PIC24FJ256GB110__) || defined(__PIC24FJ192GB110__) || defined(__PIC24FJ128GB110__) || defined(__PIC24FJ64GB110__) || defined(__PIC24FJ256GB108__) || defined(__PIC24FJ192GB108__) || defined(__PIC24FJ128GB108__) || defined(__PIC24FJ64GB108__)  || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ192GB106__) || defined(__PIC24FJ128GB106__) || defined(__PIC24FJ64GB106__)

    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  6   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()                  //No GPIO driver on VBUS on these devices.

#elif defined(__PIC24FJ256GB210__) || defined(__PIC24FJ128GB210__)  || defined(__PIC24FJ256GB206__) || defined(__PIC24FJ128GB206__)

    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  6   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()    {TRISFbits.TRISF7 = 1;}

#elif defined(__PIC24FJ256DA210__) || defined(__PIC24FJ128DA210__) || defined(__PIC24FJ256DA206__) || defined(__PIC24FJ128DA206__) || defined(__PIC24FJ256DA110__) || defined(__PIC24FJ128DA110__) || defined(__PIC24FJ256DA106__) || defined(__PIC24FJ128DA106__)

    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  7   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()    {TRISFbits.TRISF7 = 1;}

#elif defined(__PIC24FJ128GB204__) || defined(__PIC24FJ64GB204__) || defined(__PIC24FJ128GB202__) || defined(__PIC24FJ64GB202__)

    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  8   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()    {TRISBbits.TRISB6 = 1;}

#elif defined(__PIC24FJ128GC010__) || defined(__PIC24FJ64GC010__) || defined(__PIC24FJ128GC006__) || defined(__PIC24FJ64GC006__)    \
    || defined(__PIC24FJ128GB204__) || defined(__PIC24FJ64GB204__) || defined(__PIC24FJ128GB202__) || defined(__PIC24FJ64GB202__) \
    || defined(__PIC24FJ256GB410__) || defined(__PIC24FJ256GB412__) || defined(__PIC24FJ256GB406__) \
    || defined(__PIC24FJ128GB410__) || defined(__PIC24FJ128GB412__) || defined(__PIC24FJ128GB406__) \
    || defined(__PIC24FJ64GB410__) || defined(__PIC24FJ64GB412__) || defined(__PIC24FJ64GB406__)
    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  8   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()    {TRISFbits.TRISF7 = 1;}
#elif defined(__PIC24FJ1024GB610__) || defined(__PIC24FJ512GB610__) || defined(__PIC24FJ256GB610__) || defined(__PIC24FJ128GB610__) || defined(__PIC24FJ1024GB606__) || defined(__PIC24FJ512GB606__) || defined(__PIC24FJ256GB606__) || defined(__PIC24FJ128GB606__)
    #define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  8   //Number of IECx registers implemented in the microcontroller (varies from device to device, make sure this is set correctly for the intended CPU)
    #define USB_HAL_VBUSTristate()    {TRISFbits.TRISF7 = 1;}
#else
    #error "Unknown processor type selected.  Refer to device datasheet and update the definition for DEVICE_SPECIFIC_IEC_REGISTER_COUNT."
    //#define DEVICE_SPECIFIC_IEC_REGISTER_COUNT  8              // <--- Update this number with the real number for your device and uncomment (and delete the above #error).
    //#define USB_HAL_VBUSTristate()    {TRISxbits.TRISxx = 1;}  // <-- replace the "x" characters with the correct values for the VBUS GPIO pin (if the microcontroller selected has GPIO functionality on VBUS)

#endif




#if (USB_PING_PONG_MODE == USB_PING_PONG__NO_PING_PONG)
    #define BDT_NUM_ENTRIES      ((USB_MAX_EP_NUMBER + 1) * 2)
#elif (USB_PING_PONG_MODE == USB_PING_PONG__EP0_OUT_ONLY)
    #define BDT_NUM_ENTRIES      (((USB_MAX_EP_NUMBER + 1) * 2)+1)
#elif (USB_PING_PONG_MODE == USB_PING_PONG__FULL_PING_PONG)
    #define BDT_NUM_ENTRIES      ((USB_MAX_EP_NUMBER + 1) * 4)
#elif (USB_PING_PONG_MODE == USB_PING_PONG__ALL_BUT_EP0)
    #define BDT_NUM_ENTRIES      (((USB_MAX_EP_NUMBER + 1) * 4)-2)
#else
    #error "No ping pong mode defined."
#endif


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

//----- usb_config.h input definitions -----------------------------------------
#define USB_PULLUP_ENABLE               0x00
//#define USB_PULLUP_DISABLE            0x00

#define USB_INTERNAL_TRANSCEIVER        0x00
#define USB_EXTERNAL_TRANSCEIVER        0x01

#define USB_FULL_SPEED                  0x04
//USB_LOW_SPEED not currently supported in PIC24F USB products

#define USB_OTG_ENABLE                  0x04

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

//----- USB module control bits -----------------------------------------------
#define USBPingPongBufferReset          U1CONbits.PPBRST
#define USBSE0Event                     U1CONbits.SE0
#define USBSuspendControl               U1PWRCbits.USUSPEND
#define USBPacketDisable                U1CONbits.PKTDIS
#define USBResumeControl                U1CONbits.RESUME

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
#define ENDPOINT_MASK                   0b11110000

//----- U1OTGCON bit definitions ----------------------------------------------
#define USB_OTG_DPLUS_ENABLE            0x80

//----- U1EP bit definitions --------------------------------------------------
#define UEP_STALL                       0x0002
// Cfg Control pipe for this ep
#define EP_CTRL                         0x0C


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
typedef union _BD_STAT
{
    struct{
        unsigned            :2;     //Byte count
        unsigned    BSTALL  :1;     //Buffer Stall Enable
        unsigned    DTSEN   :1;     //Data Toggle Synch Enable
        unsigned            :2;     //Reserved - write as 00
        unsigned    DTS     :1;     //Data Toggle Synch Value
        unsigned    UOWN    :1;     //USB Ownership
    };
    struct{
        unsigned            :2;
        unsigned    PID0    :1;
        unsigned    PID1    :1;
        unsigned    PID2    :1;
        unsigned    PID3    :1;
    };
    struct{
        unsigned            :2;
        unsigned    PID     :4;     // Packet Identifier
    };
    uint8_t            Val;
} BD_STAT;                      //Buffer Descriptor Status Register

// BDT Entry Layout
typedef union __BDT
{
    union
    {
        struct
        {
            uint8_t CNT;
            BD_STAT     STAT __attribute__ ((packed));
        };
        struct
        {
            uint16_t        count:10;   //test
            uint8_t        :6;
            uint16_t        ADR; //Buffer Address
        };
    };
    uint32_t           Val;
    uint16_t            v[2];
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

//Macros for fetching parameters from a USTAT_FIELDS variable.
#define USBHALGetLastEndpoint(stat)     stat.endpoint_number
#define USBHALGetLastDirection(stat)    stat.direction
#define USBHALGetLastPingPong(stat)     stat.ping_pong


typedef union _POINTER
{
    struct
    {
        uint8_t bLow;
        uint8_t bHigh;
        //byte bUpper;
    };
    uint16_t _word;                         // bLow & bHigh

    //pFunc _pFunc;                       // Usage: ptr.pFunc(); Init: ptr.pFunc = &<Function>;

    uint8_t* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    uint16_t* wRam;                         // Ram word pointer: 2 bytes pointer pointing
                                        // to 2 bytes of data

    const uint8_t* bRom;                     // Size depends on compiler setting
    const uint16_t* wRom;
    //rom near byte* nbRom;               // Near = 2 bytes pointer
    //rom near word* nwRom;
    //rom far byte* fbRom;                // Far = 3 bytes pointer
    //rom far word* fwRom;
} POINTER;

/*****************************************************************************/
/****** Function prototypes and macro functions ******************************/
/*****************************************************************************/

#define ConvertToPhysicalAddress(a) ((uint16_t)(a))
#define ConvertToVirtualAddress(a)  ((void *)(a))
#define USBClearUSBInterrupt() IFS5bits.USB1IF = 0;
#if defined(USB_INTERRUPT)
    #define USBMaskInterrupts() {IEC5bits.USB1IE = 0;}
    #define USBUnmaskInterrupts() {IEC5bits.USB1IE = 1;}
#else
    #define USBMaskInterrupts()
    #define USBUnmaskInterrupts()
#endif

//STALLIE, IDLEIE, TRNIE, and URSTIE are all enabled by default and are required
#if defined(USB_INTERRUPT)
    #define USBEnableInterrupts() {IEC5bits.USB1IE=1;}
#else
    #define USBEnableInterrupts()
#endif

#define USBDisableInterrupts() {IEC5bits.USB1IE=0;}
#define USBInterruptFlag                IFS5bits.USB1IF



#if (USB_PULLUP_OPTION == USB_PULLUP_ENABLE) || !defined(USB_PULLUP_OPTION)
    #define PullUpConfiguration() U1OTGCONbits.OTGEN = 0;
#else
    #define PullUpConfiguration() U1OTGCONbits.OTGEN = 1; U1OTGCON &= 0xFF0F;
#endif

    #define SetConfigurationOptions()   {\
                                            U1CNFG1 = USB_PING_PONG_MODE;\
                                            U1CNFG2 = USB_TRANSCEIVER_OPTION;\
                                            PullUpConfiguration();\
                                            U1EIE = 0x9F;\
                                            U1IE = 0x99 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT;\
                                            USBT1MSECIE = 1;\
                                        }


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
        This macro is used to power up the USB module if required<br>
        PIC18: defines as nothing<br>
        PIC24: defines as U1PWRCbits.USBPWR = 1;<br>

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
#define USBSetBDTAddress(addr)         U1BDTP1 = (((unsigned int)addr)/256);

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
#define USBClearInterruptRegister(reg) reg = 0xFFFF;

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
#define DisableNonZeroEndpoints(last_ep_num) memset((void*)&U1EP1,0x00,(last_ep_num * 2));


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

#endif //USB_HAL_PIC24F_H
