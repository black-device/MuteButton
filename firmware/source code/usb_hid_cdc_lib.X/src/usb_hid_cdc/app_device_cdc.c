/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

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

/** INCLUDES *******************************************************/
//#include "system.h"

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "usb.h"

#include "app_device_cdc.h"
#include "usb_config.h"

/** VARIABLES ******************************************************/

//static uint8_t readBuffer[CDC_DATA_OUT_EP_SIZE];
//static uint8_t writeBuffer[CDC_DATA_IN_EP_SIZE];

/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceCDCInitialize() {   
    line_coding.bCharFormat = 0;
    line_coding.bDataBits = 8;
    line_coding.bParityType = 0;
    line_coding.dwDTERate = 9600;
}

/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceCDCBasicDemoInitialize() and APP_DeviceCDCBasicDemoStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
int APP_DeviceCDCTasks(uint8_t* rx_buffer, int rx_buffer_size, uint8_t* tx_buffer, int tx_buffer_size) {
    /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if(USBGetDeviceState() < CONFIGURED_STATE ) return 0;
    

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if(USBIsDeviceSuspended() == true) return 0;
        
    /* If the user has pressed the button associated with this demo, then we
     * are going to send a "Button Pressed" message to the terminal.
     */
//    if(0)//BUTTON_IsPressed(BUTTON_DEVICE_CDC_BASIC_DEMO) == true)
//    {
//        /* Make sure that we only send the message once per button press and
//         * not continuously as the button is held.
//         */
////        if(buttonPressed == false)
////        {
////            /* Make sure that the CDC driver is ready for a transmission.
////             */
////            if(mUSBUSARTIsTxTrfReady() == true)
////            {
////                putrsUSBUSART(buttonMessage);
////                buttonPressed = true;
////            }
////        }
//    }
//    else
//    {
//        /* If the button is released, we can then allow a new message to be
//         * sent the next time the button is pressed.
//         */
////        buttonPressed = false;
//    }
    
    int ret = -1;
    if( USBUSARTIsTxTrfReady() == true) {
        ret = 0;
        if (rx_buffer && rx_buffer_size) ret = getsUSBUSART(rx_buffer, rx_buffer_size);
        if (tx_buffer && tx_buffer_size) putUSBUSART(tx_buffer, tx_buffer_size);
    }
    

//    /* Check to see if there is a transmission in progress, if there isn't, then
//     * we can see about performing an echo response to data received.
//     */
//    if( USBUSARTIsTxTrfReady() == true)
//    {
//        uint8_t i;
//        uint8_t numBytesRead;
//
//        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
//
//        /* For every byte that was read... */
//        for(i=0; i<numBytesRead; i++)
//        {
//            switch(readBuffer[i])
//            {
//                /* If we receive new line or line feed commands, just echo
//                 * them direct.
//                 */
//                case 0x0A:
//                case 0x0D:
//                    writeBuffer[i] = readBuffer[i];
//                    break;
//
//                /* If we receive something else, then echo it plus one
//                 * so that if we receive 'a', we echo 'b' so that the
//                 * user knows that it isn't the echo enabled on their
//                 * terminal program.
//                 */
//                default:
//                    writeBuffer[i] = readBuffer[i] + 1;
//                    break;
//            }
//        }
//
//        if(numBytesRead > 0)
//        {
//            /* After processing all of the received data, we need to send out
//             * the "echo" data now.
//             */
//            putUSBUSART(writeBuffer,numBytesRead);
//        }
//    }

    CDCTxService();
    
    return ret;
}