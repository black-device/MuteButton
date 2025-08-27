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

#ifndef __USB_HAL_32BIT_C
#define __USB_HAL_32BIT_C

#include "usb.h"


//The code in this file is only intended for use with the 16-bit PIC24/dsPIC devices.
//See other hal file for other microcontrollers.
#if defined(__XC32__)




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


    //Stop clocks and put the microcontroller core to sleep now.  Note: This is done
    //in a loop, since in some wake up scenarios (ex: wake by periodic WDT timeout),
    //the USB bus may still be suspended, and the device should go back to sleep.
    while(1)
    {
        Sleep();

        //Check wakeup source.  It may have been due to USB activity from the host (ex: resume from suspend),
        //or it could have been due to some other source.
        if(USBActivityIF == 1)
        {
            //The device woke up due to some kind of USB activity.  This could have been
            //either due to the USB host sending resume signaling, or, it could be due
            //to VBUS falling below the SESSVD threshold, due to either a USB detach event,
            //or the host powering itself down fully (and thereby turning off the +5V VBUS supply).

            //Check the VBUS level.  If VBUS is no longer present, then a user detach event
            //must have occurred, or, the cable is still plugged in, but the host itself
            //powered down.
            if(USBVBUSSessionValidStateGet(true) == 0)
            {
                //A detach event must have occurred.  You may optionally stay awake and
                //continue executing code in this case, or, go back to sleep until
                //the next re-attachment event.


                //Uncomment below break statement, if you want to continue executing
                //code during the detached interval.
                //break;

                //If the user left the above break statement commented, then
                //just clear the prior wake up event and go back to sleep.
                USBClearInterruptFlag(USBActivityIFReg, USBActivityIFBitNum);
                USBInterruptFlag = 0;
            }
            else
            {
                //VBUS is still above SESSVD, implying that the device just woke up
                //due to the host sending resume (or other non-idle) bus signaling
                //to the device.  In this case, we need to fully wake up and go back
                //into a state ready to receive new USB packets/commands from the host.

                break;  //Exit the while(1) sleep loop, so we can restore normal execution of this firmware.
            }
        }
        else
        {
            //We woke up by some means other than resume from the host.  Figure out what that source was.

            //Check if the wakeup was due to the (optional) remote wakeup source, and if
            //it is actually legal to perform a remote wakeup.
            #if 0
            if(YOUR_REMOTE_WAKEUP_TRIGGER_SOURCE_ASSERTED)      //Replace YOUR_REMOTE_WAKEUP_TRIGGER_SOURCE_ASSERTED with your proper interrupt source (such as: IFS0bits.CNAIF == 1)
            {
                //Clear the wakeup source interrupt status flag, since we are now consuming/processing it.
                YOUR_REMOTE_WAKEUP_TRIGGER_SOURCE_FLAG = 0;     //Replace with real code (ex: IFS0CLR = _IFS0_CNAIF_MASK;, etc.)

                //Try to wake up the host.
                if(USBRemoteWakeupAssertBlocking() == true)
                {
                    //It was legal to send remote wakeup signaling, and it was sent.
                    //Exit the while(1) loop, so we stay out of sleep.
                    break;
                }

            }
            #endif
        }
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
bool USBRemoteWakeupAssertBlocking(void)
{
    //Make sure we are in a state where it is legal to send remote wakeup signaling
    if((USBGetRemoteWakeupStatus() == true) && (USBIsBusSuspended() == true))
    {
        //Make sure the USB module is not suspended.  We need it to be clocked and
        //fully operational.
        if(USBSuspendControl == 1)
        {
            USBSuspendControl = 0;
        }

        USBMaskAllUSBInterrupts();  //Temporarily disable USB interrupts, to prevent the stack from using/gobbling up the T1MSECIF events if USB interrupts are enabled/used.

        //USB specs require the device not send resume signaling within the first 5ms of bus idle time.
        //Since it takes 3ms of continuous idle for the suspend condition to occur,
        //this means the firmware must wait a minimum of two additional milliseconds
        //from the point that suspend is detected before sending the resume signaling.
        //To ensure that this requirement is always met, we add deliberate delay
        //below, to wait 2-3ms of extra time, before starting to send the resume signaling.
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 0-1 ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms

        //Begin sending the actual resume K-state signaling to the host
        USBResumeControl = 1;       //Start RESUME signaling

        //USB specs require the RESUME signaling to persist for 1-15ms in duration.
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 0-1 ms
        USBClearInterruptFlag(USBT1MSECIFReg, USBT1MSECIFBitNum);
        while(U1OTGIRbits.T1MSECIF == 0);   //Wait 1 full ms
        USBResumeControl = 0;       //Stop driving resume signaling

        //Restore previous interrupt settings that we may have changed.
        USBRestoreUSBInterrupts();

        //We sent the signaling and the host should be waking up now.
        return true;
    }

    //Wasn't legal to send remote wakeup signaling to the host.  We must return
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
    unsigned int tempInterruptStatus;

    //Save and clear the USBIE bit to prevent USB interrupt vectoring.
    tempInterruptStatus = __builtin_get_isr_state();
    __builtin_disable_interrupts(); //Disable all interrupts to make sure the USB interrupt enable state doesn't change in an interrupt handler
    USBIESave = 0;
    if(USBInterruptEnableBit == 1)
    {
        USBIESave = 1;
    }
    USBInterruptEnableBit = 0;        //Disable USB interrupt vectoring.

    //Restore global interrupt enable status.
    __builtin_set_isr_state(tempInterruptStatus);
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
        USBInterruptEnableBit = 1;
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


    //Save status and then disable all (maskable) interrupts
    CPUIPLSave = __builtin_get_isr_state();     //Save the current interrupt enable and CPU IPL state info
    __builtin_disable_interrupts();             //Disable all maskable interrupts

    //Set CPU priority level to 0, so that it can be woken up by a new USB interrupt
    //(at IPL = 4, which is higher than the CPU priority) but without going to the interrupt vector location.
    //Note: If the USB stack is operated in USB_INTERRUPT mode, then the current
    //CPU IPL may currently be 4, since this code is already operating within the interrupt context
    //of the USB interrupt (ex: IDLEIF asserted, causing the USB stack to call
    //the user suspend handler, which then called the USBSleepOnSuspend() handler,
    //which then called this function).  In this case, we need to lower the CPU priority
    //level to < USB IPL, so that the CPU can wake back up from sleep.
    __builtin_set_isr_state(0);     //Interrupts disabled, with CPU IPL = 0 (which is < USB IPL)

    //Now save and disable all other interrupt enable bits settings.  Note: This code
    //assumes all IECx registers are packed together little endian with no other registers intermingled
    //(make sure this is true if using a hypothetical device where this might not be the case).
    pRegister = &IEC0;
    for(i = 0; i < DEVICE_SPECIFIC_IEC_REGISTER_COUNT; i++)
    {
        IECRegSaves[i] = *pRegister;    //Save the current IECx register contents
        *pRegister = 0;                 //Clear out the current IECx register
        pRegister += 4;                 //Add 4 to skip pass inv/set/clr versions of the same IECx register
    }
    U1IE_save = U1IE;    //Also save and clear USB module sub-interrupts
    U1IE = 0;
    U1OTGIE_save = U1OTGIE;
    U1OTGIE = 0;
    USBIPLSave = USBIPLBits;

    //Now enable only the USBIE + ACVIE interrupt sources, plus possible remote
    //wakeup stimulus interrupt source(s) (but only when legal to perform remote wakeup),
    //so that they make wake the microcontroller from sleep mode (but not to vector, due to the CP0 reg 12 global interrupt enable bit being clear)
    USBInterruptFlag = 0;
    USBActivityIE = 1;
    USBIPLBits = 4;                 //Make sure USB module IPL is > 0, so it can at least wake the device from sleep
    USBInterruptEnableBit = 1;      //Enable the top level USB module IE bit now, to enable wake from sleep.
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
        *pRegister = IECRegSaves[i];
        pRegister += 4;                 //Add 4 to skip pass inv/set/clr versions of the same IECx register
    }
    U1IE = U1IE_save;
    U1OTGIE = U1OTGIE_save;
    USBIPLBits = USBIPLSave;


    //Restore CPU interrupt priority level to allow normal vectoring again (if they were enabled previously).
    __builtin_set_isr_state(CPUIPLSave);
}




//-------------------------------------------------------------------------------------------
#endif //#if defined(__XC32__)
#endif //__USB_HAL_32BIT_C