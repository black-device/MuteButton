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

#ifndef __USB_HAL_16BIT_C
#define __USB_HAL_16BIT_C

#include "usb.h"

//The code in this file is only intended for use with the 16-bit PIC24/dsPIC devices.
//See other hal file for other microcontrollers.
#if defined(__XC16__) || defined(__C30__)




//Private prototypes - do not call directly from application code.
static void USBSaveAndPrepareInterruptsForSleep(void);
static void USBRestorePreviousInterruptSettings(void);


//Private static variables needed for context saving operations.  Do not use/touch
//outside of the context of the implemented APIs.
static unsigned int CPUIPLSave;
static unsigned int IECRegSaves[DEVICE_SPECIFIC_IEC_REGISTER_COUNT];
static unsigned int U1IE_save;
static unsigned int U1OTGIE_save;
static unsigned int USBIPLSave;
static unsigned char USBIESave;




/********************************************************************
Function:
    bool USBSleepOnSuspend(void)

Summary:
    Places the core into sleep and sets up the USB module
    to wake up the device on USB activity.

PreCondition:

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
bool USBSleepOnSuspend(void)
{
    //This function needs to reconfigure the device interrupt settings so it can
    //properly wake up from suspend on USB activity, or, upon remote wakeup trigger source
    //events (when the host has allowed remote wakeup).  In order to achieve this,
    //we need to context save all interrupt settings, then disable everything, except
    //the very minimum needed to wake the microcontroller.
    USBSaveAndPrepareInterruptsForSleep();


    //Enable remote wakeup interrupt now, but only if applicable/legal to do so.
    if((USBGetRemoteWakeupStatus() == true) && (USBIsBusSuspended() == true))
    {
        //If using remote wakeup capability, you must add code here to enable the remote
        //wakeup stimulus to wake the microcontroller from sleep here.
        //Do not enable any other interrupt sources here, except the remote wakeup stimulus
        //source.  Example code for enabling a user pushbutton for example might look
        //something like follows:
        //CNEN1 = 0;
        //CNEN2 = 0;
        //CNEN3 = 0;
        //CNEN4 = 0;
        //CNEN5 = 0;
        //CNEN6 = 0;
        //CNENxbits.CNxxIE = 1; //The individual pin interrupt source to use as the remote wakeup stimulus
        //IFS1bits.CNIF = 0;
        //IPC4bits.CNIP = 4;    //IP >= 1, to allow it to cause wake from sleep.
        //IEC1bits.CNIE = 1;
    }


    //Stop clocks and put the microcontroller core to sleep now.
    Sleep();


    //Check wakeup source, to make sure it was due to host sending resume signalling
    //(or the device becoming detached from the host)
    if(USBRESUMEIF == 0)
    {
        //We woke up by some means other than resume from the host.  Figure out what that source was.

        //Check the VBUS level.  If VBUS is no longer present, then a user detach event
        //must have occurred, or, the cable is still plugged in, but the host itself
        //powered down.
        if(USBVBUSSessionValidStateGet(true) == 0)
        {
            //A detach event has occurred.  You may optionally stay awake and
            //continue executing code in this case, or, go back to sleep until
            //the next re-attachment event.

        }


        //Check if the wakeup was due to the (optional) remote wakeup source, and if
        //it is actually legal to perform a remote wakeup.
        #if 0
        if(YOUR_REMOTE_WAKEUP_TRIGGER_SOURCE_ASSERTED)      //Replace YOUR_REMOTE_WAKEUP_TRIGGER_SOURCE_ASSERTED with your proper interrupt source (such as: _CNIF == 1)
        {
            //Try to wake up the host.
            USBRemoteWakeupAssertBlocking();
        }
        #endif
    }


    //Restore all interrupt settings back to what they were before at the start
    //of this function.  According to the USB specs, USB devices should return
    //to their exact same USB state/operating condition after the end of suspend,
    //as they were in prior to entering suspend.
    USBRestorePreviousInterruptSettings();

    return true;
}






/********************************************************************
Function:
    bool USBRemoteWakeupAssertBlocking(void)

Summary:
    Checks if it is currently legal to send remote wakeup signalling to the
    host, and if so, it sends it.  This is a blocking function that takes ~5-6ms
    to execute.

PreCondition:

Parameters:
    None

Return Values:
    true  - if it was legal to send remote wakeup signalling and the signalling was sent
    false - if it was not legal to send remote wakeup signalling (in this case, no signalling gets sent)

Remarks:
    To successfully use remote wakeup in an application, the device must first
    let the host know that it supports remote wakeup (by setting the appropriate
    bit in the attributes field of the USB configuration descriptor).  Additionally,
    the end user must allow/enable remote wakeup for the device.  Hosts are not
    obligated to always allow remote wakeup, and the end user can typically enable/disable this
    feature for each device at their discretion.  Under Windows, devices supporting
    remote wakeup will normally have an extra checkbox setting under the device
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
    firmware should not send remote wakeup signalling to the host, unless both
    USBGetRemoteWakeupStatus() and USBIsBusSuspended() return true.  Attempting to
    send the remote wakeup signalling to the host when it is not legal to do so
    will nomrally not wake up the host, and will instead create a violation of
    the USB compliance criteria.  Additionally, devices should not send remote
    wakeup signalling immediately upon entry into the suspended bus state.  Based on
    the USB specifications, the bus must be continuously idle for at least 5ms before
    a device may send remote wakeup signalling.  After the signalling is sent, the
    device must become fully ready for normal USB communication/request
    processing within 10ms.
 *******************************************************************/
bool USBRemoteWakeupAssertBlocking(void)
{
    //Make sure we are in a state where it is legal to send remote wakeup signalling
    if((USBGetRemoteWakeupStatus() == true) && (USBIsBusSuspended() == true))
    {
        //Make sure the USB module is not suspended.  We need it to be clocked and
        //fully operational.
        if(USBSuspendControl == 1)
        {
            USBSuspendControl = 0;
        }

        USBMaskAllUSBInterrupts();  //Temporarily disable USB interrupts, to prevent the stack from using/gobbling up the T1MSECIF events if USB interrupts are enabled/used.

        //USB specs require the device not send resume signalling within the first 5ms of bus idle time.
        //Since it takes 3ms of continuous idle for the suspend condition to occur,
        //this means the firmware must wait a minimum of two additional milliseconds
        //from the point that suspend is detected before sending the resume signalling.
        //To ensure that this requirement is always met, we add deliberate delay
        //below, to wait 2-3ms of extra time, before starting to send the resume signalling.
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 0-1 ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms

        //Begin sending the actual resume K-state signalling to the host
        USBResumeControl = 1;       //Start RESUME signaling

        //USB specs require the RESUME signalling to persist for 1-15ms in duration.
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 0-1 ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms
        USBResumeControl = 0;       //Stop driving resume signalling

        //Restore previous interrupt settings that we may have changed.
        USBRestoreUSBInterrupts();

        //We sent the signalling and the host should be waking up now.
        return true;
    }

    //Wasn't legal to send remote wakeup signalling to the host.  We must return
    //without actually doing anything.
    return false;
}





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
int8_t USBVBUSSessionValidStateGet(bool AllowInvasiveReads)
{
    unsigned char i;
    int8_t retValue;

    //Check if module is already on and non-suspended
    if((U1PWRCbits.USBPWR == 1) && (USBSuspendControl == 0) && (USBGetTicksSinceSuspendEnd() >= 4))
    {
        return U1OTGSTATbits.SESVD;
    }
    else
    {
        //The module wasn't in a state where we can rely on the SESVD bit value yet.
        if(AllowInvasiveReads == true)
        {
            //Turn on the module and make sure it is not suspended.
            USBSuspendControl = 0;
            USBPowerModule();

            //Make sure USB interrupt vectoring is disabled, to prevent interrupt
            //handler from "stealing" the T1MSECIF events and clearing the flag before returning.
            USBMaskAllUSBInterrupts();

            //Wait ~4-5ms analog settling time for bandgap and comparators to
            //power up, and at least one comparator propagation delay has elapsed,
            //so as to provide trustworthy output status.
            for(i = 0; i < 5; i++)
            {
                USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
                while(USBT1MSECIF == 0);   //Wait for assertion
            }

            //The SESVD bit value should now be trustworthy to read.
            retValue = 0;
            if(U1OTGSTATbits.SESVD == 1)
            {
                retValue = 1;
            }

            //Restore normal USB interrupt settings.
            USBRestoreUSBInterrupts();

            return retValue;
        }
        else
        {
            //Couldn't read the value...  Module is not in a state where the
            //value can be meaningful, and the caller didn't allow us to make
            //module setting changes.

            if(U1PWRCbits.USBPWR == 0)
            {
                return -1;  //-1 indicates the USB module wasn't powered
            }
            else if(USBSuspendControl == 0)
            {
                return -2;   //-2 indicates the USB module was powered, but was suspended
            }
            else if(USBGetTicksSinceSuspendEnd() < 4)
            {
                return -3;  //-3 indicates the USB module was powered and not suspended,
                            //but that insufficient settling time has elapsed yet to
                            //get a trustworthy reading.
            }

            //Shouldn't get here.
            return -4;
        }
    }
}





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
void USBMaskAllUSBInterrupts(void)
{
    //Save and clear the USBIE bit to prevent USB interrupt vectoring.
    USBIESave = 0;
    __builtin_disi(16); //Temporarily disable all interrupts, to prevent an
                        //ISR from modifying the xxxIE bit between our read
                        //of the value, and when we clear it.
    if(_USB1IE == 1)
    {
        USBIESave = 1;
    }
    _USB1IE = 0;        //Disable USB interrupt vectoring.
}



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
void USBRestoreUSBInterrupts(void)
{
    //Restore the previous USBIE setting, that was saved when USBMaskAllUSBInterrupts() was called.
    if(USBIESave)
    {
        _USB1IE = 1;
    }
}





/********************************************************************
Function:
    static void USBSaveAndPrepareInterruptsForSleep(void)

Summary:
    This is a private function that should never be called directly by
    user application code.  This function saves the state of all IECx interupt
    enable register bits, and then clears/disables all interrupts.  It then
    re-enables only the very specific ones needed for wakeup from sleep due
    to USB activity.

PreCondition:
    Must only be called by USB stack code in the context of a USB suspend or
    USB detach event handler.

Parameters:
    None

Return Values:
    None

Remarks:
    Application code should not call or modify this function.  If in the
    application it is desired to wake up from sleep by additional sources,
    such as for remote wakeup purposes, only these additional interrupt sources
    must be enabled after returning from this function, and prior to executing
    the sleep instruction.  When called, this function must always be called in
    an exact 1:1 ratio with the USBRestorePreviousInterruptSettings() function, in
    order to restore the application interrupt settings to their previous values.
    Calling this function more than one (without calling USBRestorePreviousInterruptSettings()
    will result in a loss of state information.
  *******************************************************************/
static void USBSaveAndPrepareInterruptsForSleep(void)
{
    unsigned int i;
    volatile unsigned int* pRegister;

    //Disable all (maskable) interrupts by setting the CPU "interrupt" priority level to maximum
    __builtin_disi(30); //Disable interrupts long enough to reliably save the current SR<IPL> bit values, and then disable interrupts.
    CPUIPLSave = SR & 0x00E0;   //Save IPL bits only
    SRbits.IPL = 7;             //Set CPU to maximum, so as to disable lower priority interrupt vectoring.

    //Now save and disable all other interrupt enable bits settings.  Note: This code
    //assumes all IECx registers are packed together little endian with no other registers intermingled
    //(make sure this is true if using a hypothetical device where this might not be the case).
    pRegister = &IEC0;
    for(i = 0; i < DEVICE_SPECIFIC_IEC_REGISTER_COUNT; i++)
    {
        IECRegSaves[i] = *pRegister;    //Save the current IECx register contents
        *pRegister++ = 0;               //Clear out the current IECx register
    }
    U1IE_save = U1IE;    //Also save and clear USB module sub-interrupts
    U1IE = 0;
    U1OTGIE_save = U1OTGIE;
    U1OTGIE = 0;
    USBIPLSave = _USB1IP;

    //Now enable only the USBIE + ACVIE + SOFIE + URSTIE interrupt sources, plus possible remote
    //wakeup stimulus interrupt source(s) (but only when legal to perform remote wakeup),
    //so that they make wake the microcontroller from sleep mode (but not to vector, due to CPU IPL = 7)
    USBClearInterruptFlag(USBActivityIFReg, USBActivityIFBitNum);
    USBClearInterruptFlag(USBRESUMEIFReg, USBRESUMEIFBitNum);

    _USB1IF = 0;
    _ACTVIE = 1;
    _USB1IP = 4;    //Make sure USB module IPL is > 0, so it can at least wake the device from sleep
    _USB1IE = 1;    //Enable the top level USB module IE bit now, to enable wake from sleep.
}



/********************************************************************
Function:
    static void USBRestorePreviousInterruptSettings(void)

Summary:
    This is a private function that should never be called directly by
    user application code.  This function restores the state of all IECx interrupt
    enable register bits that were previously modified by the
    USBSaveAndPrepareInterruptsForSleep() function.

PreCondition:
    Must only be called by USB stack code in the context of a USB suspend or
    USB detach event handler, after the event has ended and it is time to
    restore the previous operating status.

Parameters:
    None

Return Values:
    None

Remarks:
    Application code should not call or modify this function.  This function
    must always be called in an exact 1:1 ratio with the
    USBSaveAndPrepareInterruptsForSleep() function.
  *******************************************************************/
static void USBRestorePreviousInterruptSettings(void)
{
    unsigned int i;
    volatile unsigned int* pRegister;

    //Restore the previously saved interrupt settings.

    //Restore all the previous application interrupt settings that we modified
    //previously by the USBSaveAndPrepareInterruptsForSleep() function.
    pRegister = &IEC0;
    for(i = 0; i < DEVICE_SPECIFIC_IEC_REGISTER_COUNT; i++)
    {
        *pRegister++ = IECRegSaves[i];
    }
    U1IE = U1IE_save;
    U1OTGIE = U1OTGIE_save;
    _USB1IP = USBIPLSave;


    //Restore CPU interrupt priority level to allow normal vectoring again.
    CPUIPLSave = (SR & 0xFF1F) | CPUIPLSave;
    SR = CPUIPLSave;
}




//-------------------------------------------------------------------------------------------
#endif //#if defined(__XC16__) || defined(__C30__)
#endif //__USB_HAL_16BIT_C