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
#include <string.h>

//#include "system.h"
#include "usb.h"
#include "usb_device_hid.h"
#include "app_device_keyboard.h"

#if defined(__XC8)
    #define PACKED
#else
    #define PACKED __attribute__((packed))
#endif

//#define KEY_MOD_LCTRL       0x01
//#define KEY_MOD_LSHIFT      0x02
//#define KEY_MOD_LALT        0x04
//#define KEY_MOD_LMETA       0x08
//#define KEY_MOD_RCTRL       0x10
//#define KEY_MOD_RSHIFT      0x20
//#define KEY_MOD_RALT        0x40
//#define KEY_MOD_RMETA       0x80
#define KEY_MOD_LCTRL       0xc0
#define KEY_MOD_LSHIFT      0xc1
#define KEY_MOD_LALT        0xc2
#define KEY_MOD_LMETA       0xc3
#define KEY_MOD_RCTRL       0xc4
#define KEY_MOD_RSHIFT      0xc5
#define KEY_MOD_RALT        0xc6
#define KEY_MOD_RMETA       0xc7

#define KEY_ENTER           0x28 // Keyboard Return (ENTER)
#define KEY_ESC             0x29 // Keyboard ESCAPE
#define KEY_BACKSPACE       0x2a // Keyboard DELETE (Backspace)
#define KEY_TAB             0x2b // Keyboard Tab
#define KEY_SPACE           0x2c // Keyboard Spacebar
#define KEY_MINUS           0x2d // Keyboard - and _
#define KEY_EQUAL           0x2e // Keyboard = and +
#define KEY_LEFTBRACE       0x2f // Keyboard [ and {
#define KEY_RIGHTBRACE      0x30 // Keyboard ] and }
#define KEY_BACKSLASH       0x31 // Keyboard \ and |
#define KEY_HASHTILDE       0x32 // Keyboard Non-US # and ~
#define KEY_SEMICOLON       0x33 // Keyboard ; and :
#define KEY_APOSTROPHE      0x34 // Keyboard ' and "
#define KEY_GRAVE           0x35 // Keyboard ` and ~
#define KEY_COMMA           0x36 // Keyboard , and <
#define KEY_DOT             0x37 // Keyboard . and >
#define KEY_SLASH           0x38 // Keyboard / and ?
#define KEY_CAPSLOCK        0x39 // Keyboard Caps Lock

#define KShft(k)            (((uint16_t)k)<<8|KEY_MOD_LSHIFT)
#define KAlt(k)             (((uint16_t)k)<<8|KEY_MOD_RALT)


const uint16_t ascii_2_keycode[] = {
    /* 00 */ 0x0000, /* 01 */ 0x0000, /* 02 */ 0x0000, /* 03 */ 0x0000,
    /* 04 */ 0x0000, /* 05 */ 0x0000, /* 06 */ 0x0000, /* 07 */ 0x0000,
    /* 08 */ 0x0000, /* 09 */ 0x0000, /* 0a */ KEY_ENTER, /* 0b */ 0x0000,
    /* 0c */ 0x0000, /* 0d */ KEY_ENTER, /* 0e */ 0x0000, /* 0f */ 0x0000,
             
    /* 10 */ 0x0000, /* 11 */ 0x0000, /* 12 */ 0x0000, /* 13 */ 0x0000,
    /* 14 */ 0x0000, /* 15 */ 0x0000, /* 16 */ 0x0000, /* 17 */ 0x0000,
    /* 18 */ 0x0000, /* 19 */ 0x0000, /* 1a */ 0x0000, /* 1b */ KEY_ESC,
    /* 1c */ 0x0000, /* 1d */ 0x0000, /* 1e */ 0x0000, /* 1f */ 0x0000,
             
    /* 20 */ KEY_SPACE, /* 21 */ KShft(0x1e), /* 22 */ KShft(KEY_APOSTROPHE), /* 23 */ KShft(0x20),
    /* 24 */ KShft(0x21), /* 25 */ KShft(0x22), /* 26 */ KShft(0x24), /* 27 */ KEY_APOSTROPHE,
    /* 28 */ KShft(0x26), /* 29 */ KShft(0x27), /* 2a */ KShft(0x25), /* 2b */ KShft(KEY_EQUAL),
    /* 2c */ KEY_COMMA, /* 2d */ KEY_MINUS, /* 2e */ KEY_DOT, /* 2f */ KEY_SLASH,
             
    /* 30 */ 0x0027, /* 31 */ 0x001e, /* 32 */ 0x001f, /* 33 */ 0x0020,
    /* 34 */ 0x0021, /* 35 */ 0x0022, /* 36 */ 0x0023, /* 37 */ 0x0024,
    /* 38 */ 0x0025, /* 39 */ 0x0026, /* 3a */ KShft(KEY_SEMICOLON), /* 3b */ KEY_SEMICOLON,
    /* 3c */ KShft(KEY_COMMA), /* 3d */ KEY_EQUAL, /* 3e */ KShft(KEY_DOT), /* 3f */ KShft(KEY_SLASH),
             
    /* 40 */ KAlt(0x1f), /* 41 */ KShft(0x04), /* 42 */ KShft(0x05), /* 43 */ KShft(0x06),
    /* 44 */ KShft(0x07), /* 45 */ KShft(0x08), /* 46 */ KShft(0x09), /* 47 */ KShft(0x0a),
    /* 48 */ KShft(0x0b), /* 49 */ KShft(0x0c), /* 4a */ KShft(0x0d), /* 4b */ KShft(0x0e),
    /* 4c */ KShft(0x0f), /* 4d */ KShft(0x10), /* 4e */ KShft(0x11), /* 4f */ KShft(0x12),
             
    /* 50 */ KShft(0x13), /* 51 */ KShft(0x14), /* 52 */ KShft(0x15), /* 53 */ KShft(0x16),
    /* 54 */ KShft(0x17), /* 55 */ KShft(0x18), /* 56 */ KShft(0x19), /* 57 */ KShft(0x1a),
    /* 58 */ KShft(0x1b), /* 59 */ KShft(0x1c), /* 5a */ KShft(0x1d), /* 5b */ KEY_LEFTBRACE,
    /* 5c */ KEY_BACKSLASH, /* 5d */ KEY_RIGHTBRACE, /* 5e */ 0x0000, /* 5f */ KShft(KEY_MINUS),
             
    /* 60 */ KEY_GRAVE, /* 61 */ 0x0004, /* 62 */ 0x0005, /* 63 */ 0x0006,
    /* 64 */ 0x0007, /* 65 */ 0x0008, /* 66 */ 0x0009, /* 67 */ 0x000a,
    /* 68 */ 0x000b, /* 69 */ 0x000c, /* 6a */ 0x000d, /* 6b */ 0x000e,
    /* 6c */ 0x000f, /* 6d */ 0x0010, /* 6e */ 0x0011, /* 6f */ 0x0012,
             
    /* 70 */ 0x0013, /* 71 */ 0x0014, /* 72 */ 0x0015, /* 73 */ 0x0016,
    /* 74 */ 0x0017, /* 75 */ 0x0018, /* 76 */ 0x0019, /* 77 */ 0x001a,
    /* 78 */ 0x001b, /* 79 */ 0x001c, /* 7a */ 0x001d, /* 7b */ KShft(KEY_LEFTBRACE),
    /* 7c */ KShft(KEY_BACKSLASH), /* 7d */ KShft(KEY_RIGHTBRACE), /* 7e */ KShft(KEY_GRAVE), /* 7f */ KEY_SPACE,
                     
    
};



// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

//Class specific descriptor - HID Keyboard
const hid_rpt_t hid_rpt01 = {
//    //.report = 
//    { 
//        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
//        0x09, 0x06,                    // USAGE (Keyboard)
//        0xa1, 0x01,                    // COLLECTION (Application)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
//        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x95, 0x08,                    //   REPORT_COUNT (8)
//        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
//        0x95, 0x05,                    //   REPORT_COUNT (5)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x05, 0x08,                    //   USAGE_PAGE (LEDs)
//        0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
//        0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
//        0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x03,                    //   REPORT_SIZE (3)
//        0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
//        0x95, 0x06,                    //   REPORT_COUNT (6)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
//        0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
//        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
//        0xc0,                          // End Collection
//    },
    .report = { 
//        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
//        0x09, 0x06,                    // USAGE (Keyboard)
//        0xa1, 0x01,                    // COLLECTION (Application)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
//        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x95, 0x08,                    //   REPORT_COUNT (8)
//        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
//        0x95, 0x05,                    //   REPORT_COUNT (5)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x05, 0x08,                    //   USAGE_PAGE (LEDs)
//        0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
//        0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
//        0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x03,                    //   REPORT_SIZE (3)
//        0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
//        0x95, 0x06,                    //   REPORT_COUNT (6)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x90,                    //   ** LOGICAL_MAXIMUM (255)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
//        0x29, 0x90,                    //   ** USAGE_MAXIMUM (Keyboard Application)
//        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
//        0xc0,                          // End Collection
        
//        // 78 bytes
//        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
//        0x09, 0x06,        // Usage (Keyboard)
//        0xA1, 0x01,        // Collection (Application)
//        0x85, 0x01,        //   Report ID (1)
//        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
//        0x75, 0x01,        //   Report Size (1)
//        0x95, 0x08,        //   Report Count (8)
//        0x19, 0xE0,        //   Usage Minimum (0xE0)
//        0x29, 0xE7,        //   Usage Maximum (0xE7)
//        0x15, 0x00,        //   Logical Minimum (0)
//        0x25, 0x01,        //   Logical Maximum (1)
//        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//        0x95, 0x03,        //   Report Count (3)
//        0x75, 0x08,        //   Report Size (8)
//        0x15, 0x00,        //   Logical Minimum (0)
//        0x25, 0x64,        //   Logical Maximum (100)
//        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
//        0x19, 0x00,        //   Usage Minimum (0x00)
//        0x29, 0x65,        //   Usage Maximum (0x65)
//        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
//        0xC0,              // End Collection
//        0x05, 0x0C,        // Usage Page (Consumer)
//        0x09, 0x01,        // Usage (Consumer Control)
//        0xA1, 0x01,        // Collection (Application)
//        0x85, 0x02,        //   Report ID (2)
//        0x05, 0x0C,        //   Usage Page (Consumer)
//        0x15, 0x00,        //   Logical Minimum (0)
//        0x25, 0x01,        //   Logical Maximum (1)
//        0x75, 0x01,        //   Report Size (1)
//        0x95, 0x07,        //   Report Count (7)
//        0x09, 0xB5,        //   Usage (Scan Next Track)
//        0x09, 0xB6,        //   Usage (Scan Previous Track)
//        0x09, 0xB7,        //   Usage (Stop)
//        0x09, 0xB8,        //   Usage (Eject)
//        0x09, 0xCD,        //   Usage (Play/Pause)
//        0x09, 0xE2,        //   Usage (Mute)
//        0x09, 0xE9,        //   Usage (Volume Increment)
//        0x09, 0xEA,        //   Usage (Volume Decrement)
//        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//        0xC0,              // End Collection
        
        //hid report descriptor for interface 1 (keyboard)
        0x05, 0x01, //usage page (generic desktop)   //52, 53
        0x09, 0x06, //usage (keyboard)   //54, 55
        0xA1, 0x01, //collection (application) //56, 57
        0x85, 0x01,  /*   Report ID1  */
        0x05, 0x07, //usage page (key codes)   //58, 59
        0x19, 0xE0, //usage min (224) //60, 61
        0x29, 0xE7, //usage max (231) //62, 63
        0x15, 0x00, //logical min (0) //64, 65
        0x25, 0x01, //logical max (1) //66, 67
        0x75, 0x01, //report size (1) //68, 69
        0x95, 0x08, //report count (8)   //70, 71
        0x81, 0x02, //input (data, variable, absolute) [modifier byte] //72, 73
        0x95, 0x01, //report count (1)   //74, 75
        0x75, 0x08, //report size (8)    //76, 77
        0x81, 0x01, //input (constant) [reserved byte]  //78, 79
        0x95, 0x05, //report count (5)   //80, 81
        0x75, 0x01, //report size (1)    //82, 83
        0x05, 0x08, //usage page (page# for leds) //84, 85
        0x19, 0x01, //usage min (1)   //86, 87
        0x29, 0x05, //usage max (5)   //88, 89
        0x91, 0x02, //output (data, var, abs) [led report] //90, 91
        0x95, 0x01, //report count (1)   //92, 93
        0x75, 0x03, //report size (3) //94, 95
        0x91, 0x01, //output (constant) [led report padding]  //96, 97
        0x95, 0x05, //report count (5)   //98, 99
        0x75, 0x08, //report size (8) //100, 101
        0x15, 0x00, //logical min (0) //102, 103
        0x25, 0x65, //logical max (101)  //104, 105
        0x05, 0x07, //usage page (key codes)   //106, 107
        0x19, 0x00, //usage min (0)   //108, 109
        0x29, 0x65, //usage max (101) //110, 111
        0x81, 0x00, //input (data, array)   //112, 113
        0xC0,        //end collection  //114
     
        // 64+1
        //
        0x05, 0x0C,    /*      Usage Page (Consumer Devices)      */
        0x09, 0x01,    /*      Usage (Consumer Control)         */
        0xA1, 0x01,    /*      Collection (Application)         */
        0x85, 0x02,    /*      Report ID=2                     */
        0x05, 0x0C,    /*      Usage Page (Consumer Devices)      */
        0x15, 0x00,    /*      Logical Minimum (0)               */
        0x25, 0x01,    /*      Logical Maximum (1)               */
        0x75, 0x01,    /*      Report Size (1)                  */
        0x95, 0x07,    /*      Report Count (7)               */
        0x09, 0xB5,    /*      Usage (Scan Next Track)            */
        0x09, 0xB6,    /*      Usage (Scan Previous Track)         */
        0x09, 0xB7,    /*      Usage (Stop)                  */
        0x09, 0xCD,    /*      Usage (Play / Pause)            */
        0x09, 0xE2,    /*      Usage (Mute)                  */
        0x09, 0xE9,    /*      Usage (Volume Up)               */
        0x09, 0xEA,    /*      Usage (Volume Down)               */
        0x81, 0x02,    /*      Input (Data, Variable, Absolute)   */
        0x95, 0x01,    /*      Report Count (1)               */
        0x81, 0x01,    /*      Input (Constant)               */
        0xC0,          /*      End Collection                  */   
        // 38+1
    } 
};
//const struct {
//    uint8_t report[HID_RPT01_SIZE];
//} id_rpt01 = {
//    //.report = 
//    { 
//        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
//        0x09, 0x06,                    // USAGE (Keyboard)
//        0xa1, 0x01,                    // COLLECTION (Application)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
//        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x95, 0x08,                    //   REPORT_COUNT (8)
//        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
//        0x95, 0x05,                    //   REPORT_COUNT (5)
//        0x75, 0x01,                    //   REPORT_SIZE (1)
//        0x05, 0x08,                    //   USAGE_PAGE (LEDs)
//        0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
//        0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
//        0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
//        0x95, 0x01,                    //   REPORT_COUNT (1)
//        0x75, 0x03,                    //   REPORT_SIZE (3)
//        0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
//        0x95, 0x06,                    //   REPORT_COUNT (6)
//        0x75, 0x08,                    //   REPORT_SIZE (8)
//        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//        0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
//        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
//        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
//        0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
//        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
//        0xc0,                          // End Collection
//    }     
////    .report = { 
////        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
////        0x09, 0x06,                    // USAGE (Keyboard)
////        0xa1, 0x01,                    // COLLECTION (Application)
////        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
////        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
////        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
////        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
////        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
////        0x75, 0x01,                    //   REPORT_SIZE (1)
////        0x95, 0x08,                    //   REPORT_COUNT (8)
////        0x81, 0x02,                    //   INPUT (Data,Var,Abs)
////        0x95, 0x01,                    //   REPORT_COUNT (1)
////        0x75, 0x08,                    //   REPORT_SIZE (8)
////        0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
////        0x95, 0x05,                    //   REPORT_COUNT (5)
////        0x75, 0x01,                    //   REPORT_SIZE (1)
////        0x05, 0x08,                    //   USAGE_PAGE (LEDs)
////        0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
////        0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
////        0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
////        0x95, 0x01,                    //   REPORT_COUNT (1)
////        0x75, 0x03,                    //   REPORT_SIZE (3)
////        0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
////        0x95, 0x06,                    //   REPORT_COUNT (6)
////        0x75, 0x08,                    //   REPORT_SIZE (8)
////        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
////        0x25, 0xff,                    //   LOGICAL_MAXIMUM (255)
////        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
////        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
////        0x29, 0xff,                    //   USAGE_MAXIMUM (Keyboard Application)
////        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
////        0xc0,                          // End Collection
////    }     
//};


// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************

/* This typedef defines the only INPUT report found in the HID report
 * descriptor and gives an easy way to create the OUTPUT report. */
typedef struct PACKED {
    /* The union below represents the first byte of the INPUT report.  It is
     * formed by the following HID report items:
     *
     *  0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
     *  0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
     *  0x15, 0x00, //   LOGICAL_MINIMUM (0)
     *  0x25, 0x01, //   LOGICAL_MAXIMUM (1)
     *  0x75, 0x01, //   REPORT_SIZE (1)
     *  0x95, 0x08, //   REPORT_COUNT (8)
     *  0x81, 0x02, //   INPUT (Data,Var,Abs)
     *
     * The report size is 1 specifying 1 bit per entry.
     * The report count is 8 specifying there are 8 entries.
     * These entries represent the Usage items between Left Control (the usage
     * minimum) and Right GUI (the usage maximum).
     */
    union PACKED {
        uint8_t value;
        struct PACKED {
            unsigned leftControl    :1;
            unsigned leftShift      :1;
            unsigned leftAlt        :1;
            unsigned leftGUI        :1;
            unsigned rightControl   :1;
            unsigned rightShift     :1;
            unsigned rightAlt       :1;
            unsigned rightGUI       :1;
        } bits;
    } modifiers;

    /* There is one byte of constant data/padding that is specified in the
     * input report:
     *
     *  0x95, 0x01,                    //   REPORT_COUNT (1)
     *  0x75, 0x08,                    //   REPORT_SIZE (8)
     *  0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
     */
    unsigned :8;

    /* The last INPUT item in the INPUT report is an array type.  This array
     * contains an entry for each of the keys that are currently pressed until
     * the array limit, in this case 6 concurent key presses.
     *
     *  0x95, 0x06,                    //   REPORT_COUNT (6)
     *  0x75, 0x08,                    //   REPORT_SIZE (8)
     *  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
     *  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
     *  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
     *  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
     *  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
     *
     * Report count is 6 indicating that the array has 6 total entries.
     * Report size is 8 indicating each entry in the array is one byte.
     * The usage minimum indicates the lowest key value (Reserved/no event)
     * The usage maximum indicates the highest key value (Application button)
     * The logical minimum indicates the remapped value for the usage minimum:
     *   No Event has a logical value of 0.
     * The logical maximum indicates the remapped value for the usage maximum:
     *   Application button has a logical value of 101.
     *
     * In this case the logical min/max match the usage min/max so the logical
     * remapping doesn't actually change the values.
     *
     * To send a report with the 'a' key pressed (usage value of 0x04, logical
     * value in this example of 0x04 as well), then the array input would be the
     * following:
     *
     * LSB [0x04][0x00][0x00][0x00][0x00][0x00] MSB
     *
     * If the 'b' button was then pressed with the 'a' button still held down,
     * the report would then look like this:
     *
     * LSB [0x04][0x05][0x00][0x00][0x00][0x00] MSB
     *
     * If the 'a' button was then released with the 'b' button still held down,
     * the resulting array would be the following:
     *
     * LSB [0x05][0x00][0x00][0x00][0x00][0x00] MSB
     *
     * The 'a' key was removed from the array and all other items in the array
     * were shifted down. */
    uint8_t keys[6];
} KEYBOARD_INPUT_REPORT;




/* This typedef defines the only OUTPUT report found in the HID report
 * descriptor and gives an easy way to parse the OUTPUT report. */
typedef union PACKED
{
    /* The OUTPUT report is comprised of only one byte of data. */
    uint8_t value;
    struct
    {
        /* There are two report items that form the one byte of OUTPUT report
         * data.  The first report item defines 5 LED indicators:
         *
         *  0x95, 0x05,                    //   REPORT_COUNT (5)
         *  0x75, 0x01,                    //   REPORT_SIZE (1)
         *  0x05, 0x08,                    //   USAGE_PAGE (LEDs)
         *  0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
         *  0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
         *  0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
         *
         * The report count indicates there are 5 entries.
         * The report size is 1 indicating each entry is just one bit.
         * These items are located on the LED usage page
         * These items are all of the usages between Num Lock (the usage
         * minimum) and Kana (the usage maximum).
         */
        unsigned numLock        :1;
        unsigned capsLock       :1;
        unsigned scrollLock     :1;
        unsigned compose        :1;
        unsigned kana           :1;

        /* The second OUTPUT report item defines 3 bits of constant data
         * (padding) used to make a complete byte:
         *
         *  0x95, 0x01,                    //   REPORT_COUNT (1)
         *  0x75, 0x03,                    //   REPORT_SIZE (3)
         *  0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
         *
         * Report count of 1 indicates that there is one entry
         * Report size of 3 indicates the entry is 3 bits long. */
        unsigned                :3;
    } leds;
} KEYBOARD_OUTPUT_REPORT;


/* This creates a storage type for all of the information required to track the
 * current state of the keyboard. */
typedef struct
{
    USB_HANDLE lastINTransmission;
    USB_HANDLE lastOUTTransmission;
    unsigned char key;
    bool waitingForRelease;
} KEYBOARD;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Variables
// *****************************************************************************
// *****************************************************************************
static KEYBOARD keyboard;

#if !defined(KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
//static KEYBOARD_INPUT_REPORT inputReport KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;

#if !defined(KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
//static volatile KEYBOARD_OUTPUT_REPORT outputReport KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;

static uint8_t dataIn[32];

#define buffer_keys_size        (32)
static uint32_t buffer_keys[buffer_keys_size];
static int buffer_keys_len;


// *****************************************************************************
// *****************************************************************************
// Section: Private Prototypes
// *****************************************************************************
// *****************************************************************************
static void APP_KeyboardProcessOutputReport(void);


//Exteranl variables declared in other .c files
//extern volatile signed int SOFCounter;


//Application variables that need wide scope
//KEYBOARD_INPUT_REPORT oldInputReport;
signed int keyboardIdleRate;
//signed int LocalSOFCount;
//static signed int OldSOFCount;




// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************
void APP_KeyboardInit(void)
{
    //initialize the variable holding the handle for the last
    // transmission
    keyboard.lastINTransmission = 0;
    
    keyboard.key = 4;
    keyboard.waitingForRelease = false;

    //Set the default idle rate to 500ms (until the host sends a SET_IDLE request to change it to a new value)
    keyboardIdleRate = 500;

    //Copy the (possibly) interrupt context SOFCounter value into a local variable.
    //Using a while() loop to do this since the SOFCounter isn't necessarily atomically
    //updated and therefore we need to read it a minimum of twice to ensure we captured the correct value.
//    while(OldSOFCount != SOFCounter)
//    {
//        OldSOFCount = SOFCounter;
//    }

    //enable the HID endpoint
    USBEnableEndpoint(HID_EP, USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    //Arm OUT endpoint so we can receive caps lock, num lock, etc. info from host
    //keyboard.lastOUTTransmission = HIDRxPacket(HID_EP,(uint8_t*)&outputReport, sizeof(outputReport) );
    keyboard.lastOUTTransmission = HIDRxPacket(HID_EP, dataIn, sizeof(dataIn));    
    
    buffer_keys_len = 0;
}


int APP_Keyboard_Send_Key(uint32_t key) {
    
    if (buffer_keys_len >= buffer_keys_size) return -1;
    
    buffer_keys[buffer_keys_len++] = key;
    
    return 0;
}

uint16_t APP_Keyboard_Ascii_2_ScanKeyCode(char c) {
    if (c > 0x7f) return 0x0000;
    return ascii_2_keycode[c];
}


void APP_KeyboardTasks(void)
{
//    signed int TimeDeltaMilliseconds;
//    unsigned char i;
//    bool needToSendNewReportPacket;

    /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if(USBGetDeviceState() < CONFIGURED_STATE) return;

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if(USBIsDeviceSuspended() == true) return;
    
    
    //Copy the (possibly) interrupt context SOFCounter value into a local variable.
    //Using a while() loop to do this since the SOFCounter isn't necessarily atomically
    //updated and we need to read it a minimum of twice to ensure we captured the correct value.
//    while(LocalSOFCount != SOFCounter)
//    {
//        LocalSOFCount = SOFCounter;
//    }
//
//    //Compute the elapsed time since the last input report was sent (we need
//    //this info for properly obeying the HID idle rate set by the host).
//    TimeDeltaMilliseconds = LocalSOFCount - OldSOFCount;
//    //Check for negative value due to count wraparound back to zero.
//    if(TimeDeltaMilliseconds < 0)
//    {
//        TimeDeltaMilliseconds = (32767 - OldSOFCount) + LocalSOFCount;
//    }
//    //Check if the TimeDelay is quite large.  If the idle rate is == 0 (which represents "infinity"),
//    //then the TimeDeltaMilliseconds could also become infinity (which would cause overflow)
//    //if there is no recent button presses or other changes occurring on the keyboard.
//    //Therefore, saturate the TimeDeltaMilliseconds if it gets too large, by virtue
//    //of updating the OldSOFCount, even if we haven't actually sent a packet recently.
//    if(TimeDeltaMilliseconds > 5000)
//    {
//        OldSOFCount = LocalSOFCount - 5000;
//    }


    /* Check if the IN endpoint is busy, and if it isn't check if we want to send
     * keystroke data to the host. */
    if(HIDTxHandleBusy(keyboard.lastINTransmission) == false) {
        static bool clear_kb = false;
        static uint8_t report[16];
        static int report_len = 0;
        
        if (buffer_keys_len > 0) {
            /* Clear the INPUT report buffer.  Set to all zeros. */
            //memset(&inputReport, 0, sizeof(inputReport));
            memset(report, 0, sizeof(report));

            int keys_cnt = 0;

            for (int i = 0; i < sizeof(uint32_t); i++) {
                uint8_t k = buffer_keys[0] & 0xff;
                buffer_keys[0] >>= 8;
                if (k >= KEY_MEDIA_SCAN_NEXT && k <= KEY_MEDIA_VOL_DOWN) {
                    report[0] = 2;
                    report[1] = 1 << (k & 0x0f);
                    report_len = 2;
                    break;
                } else {
                    report[0] = 1;
                    report_len = 5;
                    
                    if (k >= KEY_MOD_LCTRL && k <= KEY_MOD_RMETA) {
                        report[1] |= 1 << (k & 0x0f);
                    } else report[++keys_cnt] = k;
                }
                    
//                if (k == KEY_MOD_LSHIFT) inputReport.modifiers.bits.leftShift = 1;
//                else if (k == KEY_MOD_RSHIFT) inputReport.modifiers.bits.rightShift = 1;
//                else if (k == KEY_MOD_LALT) inputReport.modifiers.bits.leftAlt = 1;
//                else if (k == KEY_MOD_RALT) inputReport.modifiers.bits.rightAlt = 1;
//                else if (k == KEY_MOD_LCTRL) inputReport.modifiers.bits.leftControl = 1;
//                else if (k == KEY_MOD_RCTRL) inputReport.modifiers.bits.leftControl = 1;
//                else if (k == KEY_MOD_LMETA) inputReport.modifiers.bits.leftGUI = 1;
//                else if (k == KEY_MOD_RMETA) inputReport.modifiers.bits.rightGUI = 1;
//                else {
//                    inputReport.keys[keys_cnt++] = k;
//                }
            }

            for (int i = 1; i < buffer_keys_len; i++) buffer_keys[i - i] = buffer_keys[i];
            buffer_keys_len--;

            //keyboard.lastINTransmission = HIDTxPacket(HID_EP, (uint8_t*)&inputReport, sizeof(inputReport));
            keyboard.lastINTransmission = HIDTxPacket(HID_EP, report, 8);//report_len);
            clear_kb = true;
            
        } else if (clear_kb) {
            //memset(&inputReport, 0, sizeof(inputReport));
            memset(report, 0, sizeof(report));
            //keyboard.lastINTransmission = HIDTxPacket(HID_EP, (uint8_t*)&inputReport, sizeof(inputReport));
            keyboard.lastINTransmission = HIDTxPacket(HID_EP, report, 8);
            clear_kb = false;
        }
            
       
//        if(0)//BUTTON_IsPressed(BUTTON_USB_DEVICE_HID_KEYBOARD_KEY) == true)
//        {
//            if(keyboard.waitingForRelease == false)
//            {
//                keyboard.waitingForRelease = true;
//
//                /* Set the only important data, the key press data. */
//                inputReport.keys[0] = keyboard.key++;
//
//                //In this simulated keyboard, if the last key pressed exceeds the a-z + 0-9,
//                //then wrap back around so we send 'a' again.
//                if(keyboard.key == 40)
//                {
//                    keyboard.key = 4;
//                }
//            }
//        }
//        else
//        {
//            keyboard.waitingForRelease = false;
//        }

        //Check to see if the new packet contents are somehow different from the most
        //recently sent packet contents.
//        needToSendNewReportPacket = false;
//        for(i = 0; i < sizeof(inputReport); i++) {
//            if(*((uint8_t*)&oldInputReport + i) != *((uint8_t*)&inputReport + i))
//            {
//                needToSendNewReportPacket = true;
//                break;
//            }
//        }
//
//        //Check if the host has set the idle rate to something other than 0 (which is effectively "infinite").
//        //If the idle rate is non-infinite, check to see if enough time has elapsed since
//        //the last packet was sent, and it is time to send a new repeated packet or not.
//        if(keyboardIdleRate != 0) {
//            //Check if the idle rate time limit is met.  If so, need to send another HID input report packet to the host
//            if(TimeDeltaMilliseconds >= keyboardIdleRate) {
//                needToSendNewReportPacket = true;
//            }
//        }
//
//        //Now send the new input report packet, if it is appropriate to do so (ex: new data is
//        //present or the idle rate limit was met).
//        if(needToSendNewReportPacket == true) {
//            //Save the old input report packet contents.  We do this so we can detect changes in report packet content
//            //useful for determining when something has changed and needs to get re-sent to the host when using
//            //infinite idle rate setting.
//            oldInputReport = inputReport;
//
//            /* Send the 8 byte packet over USB to the host. */
//            keyboard.lastINTransmission = HIDTxPacket(HID_EP, (uint8_t*)&inputReport, sizeof(inputReport));
////            OldSOFCount = LocalSOFCount;    //Save the current time, so we know when to send the next packet (which depends in part on the idle rate setting)
//        }

    }


    /* Check if any data was sent from the PC to the keyboard device.  Report
     * descriptor allows host to send 1 byte of data.  Bits 0-4 are LED states,
     * bits 5-7 are unused pad bits.  The host can potentially send this OUT
     * report data through the HID OUT endpoint (EP1 OUT), or, alternatively,
     * the host may try to send LED state information by sending a SET_REPORT
     * control transfer on EP0.  See the USBHIDCBSetReportHandler() function. */
    if(HIDRxHandleBusy(keyboard.lastOUTTransmission) == false)
    {
        
        int rx_len = USBHandleGetLength(keyboard.lastOUTTransmission);
        if (rx_len > 0) {
            int i = 0;
        }
        APP_KeyboardProcessOutputReport();

        //keyboard.lastOUTTransmission = HIDRxPacket(HID_EP,(uint8_t*)&outputReport,sizeof(outputReport));
        keyboard.lastOUTTransmission = HIDRxPacket(HID_EP, dataIn, sizeof(dataIn));
    }
    
    return;		
}


static void APP_KeyboardProcessOutputReport(void) {
//    if(outputReport.leds.capsLock)
//    {
//        LED_On(LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK);
//    }
//    else
//    {
//        LED_Off(LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK);
//    }
}


static void USBHIDCBSetReportComplete(void) {
    /* 1 byte of LED state data should now be in the CtrlTrfData buffer.  Copy
     * it to the OUTPUT report buffer for processing */
//    outputReport.value = CtrlTrfData[0];

    /* Process the OUTPUT report. */
    APP_KeyboardProcessOutputReport();
}


void USBHIDCBSetReportHandler(void) {
    /* Prepare to receive the keyboard LED state data through a SET_REPORT
     * control transfer on endpoint 0.  The host should only send 1 byte,
     * since this is all that the report descriptor allows it to send. */
    USBEP0Receive((uint8_t*)&CtrlTrfData, USB_EP0_BUFF_SIZE, USBHIDCBSetReportComplete);
}


//Callback function called by the USB stack, whenever the host sends a new SET_IDLE
//command.
void USBHIDCBSetIdleRateHandler(uint8_t reportID, uint8_t newIdleRate) {
    //Make sure the report ID matches the keyboard input report id number.
    //If however the firmware doesn't implement/use report ID numbers,
    //then it should be == 0.
    if(reportID == 0) {
        keyboardIdleRate = newIdleRate;
    }
}


/*******************************************************************************
 End of File
*/
