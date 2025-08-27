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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_MEDIA_SCAN_NEXT         0xe0
#define KEY_MEDIA_SCAN_PREV         0xe1
#define KEY_MEDIA_STOP              0xe2
#define KEY_MEDIA_EJECT             0xe3
#define KEY_MEDIA_PAUSE             0xe4
#define KEY_MEDIA_MUTE              0xe5
#define KEY_MEDIA_VOL_UP            0xe6
#define KEY_MEDIA_VOL_DOWN          0xe7


void APP_KeyboardInit(void);
void APP_KeyboardTasks(void);

int APP_Keyboard_Send_Key(uint32_t key);

uint16_t APP_Keyboard_Ascii_2_ScanKeyCode(char c);

#endif
