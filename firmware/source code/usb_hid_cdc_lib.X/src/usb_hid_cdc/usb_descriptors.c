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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>

#include "usb.h"
#include "usb_device_hid.h"
#include "usb_device_cdc.h"


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR device_dsc = {
    sizeof(USB_DEVICE_DESCRIPTOR),//0x12,    // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,                // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
//    0x00,                   // Class Code
//    0x00,                   // Subclass code
//    0x00,                   // Protocol code
    0xEF,                   // Class code (Miscellaneous)
    0x02,                   // Subclass code (Common Class)
    0x01,                   // Protocol code (Interface Association Descriptor)
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    MY_VID,                 // Vendor ID
    MY_PID,                 // Product ID: Keyboard fw demo
    0x0001,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const uint8_t configDescriptor1[] = {
    /* Configuration Descriptor */
    9, //sizeof(USB_CFG_DSC),       // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,   // CONFIGURATION descriptor type
    DESC_CONFIG_WORD(107),          // Total length of data for this cfg
    3,//1,                          // Number of interfaces in this cfg
    1,                              // Index value of this configuration
    1,//0,                          // Configuration string index
    _DEFAULT | _SELF,               // Attributes, see usb_device.h
    0x32,//50,                      // Max power consumption (2X mA)

    // HID ---------------------------------------------------------------------
    
    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),       // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,       // INTERFACE descriptor type
    0,                              // Interface Number
    0,                              // Alternate Setting Number
    2,                              // Number of endpoints in this intf
    HID_INTF,                       // Class code
    BOOT_INTF_SUBCLASS,             // Subclass code
    HID_PROTOCOL_KEYBOARD,          // Protocol code
    0,                              // Interface string index

    /* HID Class-Specific Descriptor */
    9,//sizeof(USB_HID_DSC)+3,      // Size of this descriptor in bytes RRoj hack
    DSC_HID,                        // HID descriptor type
    DESC_CONFIG_WORD(0x0111),       // HID Spec Release Number in BCD format (1.11)
    0x00,                           // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,                 // Number of class descriptors, see usbcfg.h
    DSC_RPT,                        // Report descriptor type
    DESC_CONFIG_WORD(63),           // Size of the report descriptor
    
    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    HID_EP | _EP_IN,            //EndpointAddress
    _INTERRUPT,                       //Attributes
    DESC_CONFIG_WORD(8),        //size
    0x01,                        //Interval

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    HID_EP | _EP_OUT,            //EndpointAddress
    _INTERRUPT,                       //Attributes
    DESC_CONFIG_WORD(8),        //size
    0x01,                        //Interval
    
    
    // IAD ---------------------------------------------------------------------
    // Interface Association Descriptor
    8,                                  // Size of this descriptor in bytes
    0x0B,                               // Interface association descriptor type
    0x01,                               // First associated interface
    0x02,                               // Number of contiguous associated interfaces
    COMM_INTF,                          // bInterfaceClass of the first interface
    ABSTRACT_CONTROL_MODEL,             // bInterfaceSubClass of the first interface
    V25TER,                             // bInterfaceProtocol of the first interface
    0x00,                               // Interface string index
    
    
    // CDC ---------------------------------------------------------------------
    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,   // INTERFACE descriptor type
    1,                          // Interface Number
    0,                          // Alternate Setting Number
    1,                          // Number of endpoints in this intf
    COMM_INTF,                  // Class code
    ABSTRACT_CONTROL_MODEL,     // Subclass code
    V25TER,                     // Protocol code
    0,                          // Interface string index

    /* CDC Class-Specific Descriptors */
    sizeof(USB_CDC_HEADER_FN_DSC),
    CS_INTERFACE,
    DSC_FN_HEADER,
    //0x10,0x01,
    0x20, 0x01,                  // bcdCDC

    // Abstract Control Management Functional Descriptor
    sizeof(USB_CDC_ACM_FN_DSC),
    CS_INTERFACE,
    DSC_FN_ACM,
    USB_CDC_ACM_FN_DSC_VAL,

    // Union Functional Descriptor
    sizeof(USB_CDC_UNION_FN_DSC),
    CS_INTERFACE,
    DSC_FN_UNION,
//    CDC_COMM_INTF_ID,
//    CDC_DATA_INTF_ID,
    0x01,                               // bControlInterface
    0x02,                               // bSubordinateInterface0

    // Call Management Functional Descriptor
    sizeof(USB_CDC_CALL_MGT_FN_DSC),
    CS_INTERFACE,
    DSC_FN_CALL_MGT,
    0x00,
//    CDC_DATA_INTF_ID,
    0x02,                               // bDataInterface

    /* Endpoint Descriptor */
    7,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,            // Endpoint Descriptor
    _EP02_IN,                           // EndpointAddress
    _INTERRUPT,                         // Attributes
//    0x0A,0x00,                          // size
    CDC_COMM_IN_EP_SIZE, 0x00,          // size
    0x02,                               // Interval

    /* CDC Data interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,   // INTERFACE descriptor type
    2,                          // Interface Number
    0,                          // Alternate Setting Number
    2,                          // Number of endpoints in this intf
    DATA_INTF,                  // Class code
    0,                          // Subclass code
    NO_PROTOCOL,                // Protocol code
    0,                          // Interface string index
    
    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    // Endpoint Descriptor
    _EP03_OUT,                  // EndpointAddress
    _BULK,                      // Attributes
    DESC_CONFIG_WORD(0x40),     // size
    0x00,                       // Interval

    /* Endpoint Descriptor */
    7,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    // Endpoint Descriptor
    _EP03_IN,                   // EndpointAddress
    _BULK,                      // Attributes
    DESC_CONFIG_WORD(0x40),     // size
    0x00,                       // Interval

};

//Language code string descriptor
const struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[1];
} sd000 = {
    sizeof(sd000),
    USB_DESCRIPTOR_STRING,
    {0x0409} //0x0409 = Language ID code for US English
};

//Manufacturer string descriptor
const struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[11];
} sd001 = {
    sizeof(sd001), 
    USB_DESCRIPTOR_STRING, 
    { 'B', 'l', 'a', 'c', 'k', 'd', 'e', 'v', 'i', 'c', 'e'}
};


const struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[11];
} sd002 = {
    sizeof(sd002), 
    USB_DESCRIPTOR_STRING, 
    { 'M', 'u', 't', 'e', ' ', 'b', 'u', 't', 't', 'o', 'n'}
};


//Array of configuration descriptors
const uint8_t *const USB_CD_Ptr[]=
{
    (const uint8_t *const)&configDescriptor1
};

//Array of string descriptors
const uint8_t *const USB_SD_Ptr[]=
{
    (const uint8_t *const)&sd000,
    (const uint8_t *const)&sd001,
    (const uint8_t *const)&sd002,
};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
 End of File
*/

