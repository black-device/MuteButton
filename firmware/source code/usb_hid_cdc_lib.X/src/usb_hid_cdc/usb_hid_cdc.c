#include <xc.h>
#include <p32xxxx.h>
#include <sys/attribs.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>



#include "usb_hid_cdc.h"
#include "usb.h"
#include "usb_device_hid.h"
#include "usb_device_cdc.h"
#include "app_device_keyboard.h"
#include "app_device_cdc.h"


static usb_hid_cdc_cfg_t* lib_cfg = NULL;



void USB_HID_CDC_Initialize(usb_hid_cdc_cfg_t* cfg) {
    lib_cfg = cfg;
    lib_cfg->_cdc_tx_pkt_send = 0;
    
    USBDeviceInit();
    USBDeviceAttach();
    
    APP_DeviceCDCInitialize();
}


void USB_HID_CDC_Tasks() {
    /* Run the keyboard tasks. */
    APP_KeyboardTasks();
    
    //Application specific tasks
    static uint8_t cdc_rx_buffer[CDC_DATA_OUT_EP_SIZE];
    lib_cfg->cdc_rx_bytes_read = 0;
    uint8_t* tx_ptr = lib_cfg->cdc_tx_buffer;
    int tx_bytes_left = lib_cfg->_cdc_tx_pkt_send;
    int retries = 0;
    while (1) {
        int tx_bytes_pkt_sent = (tx_bytes_left > CDC_DATA_IN_EP_SIZE) ? CDC_DATA_IN_EP_SIZE : tx_bytes_left;
        int cdc_bytes_read = APP_DeviceCDCTasks(cdc_rx_buffer, sizeof(cdc_rx_buffer), tx_ptr, tx_bytes_pkt_sent);
        
        if (cdc_bytes_read == 0 && retries == 0 && tx_bytes_left == 0) break;
        
        if (cdc_bytes_read > 0 && (lib_cfg->cdc_rx_bytes_read + cdc_bytes_read) <= lib_cfg->cdc_rx_buffer_size) {
            memcpy(&lib_cfg->cdc_rx_buffer[lib_cfg->cdc_rx_bytes_read], cdc_rx_buffer, cdc_bytes_read);
            lib_cfg->cdc_rx_bytes_read += cdc_bytes_read;
            retries = 20;
        }
        if (tx_bytes_left) {
            tx_ptr += tx_bytes_pkt_sent;
            tx_bytes_left -= tx_bytes_pkt_sent;
            while (APP_DeviceCDCTasks(NULL, 0, NULL, 0) < 0) {}
        }
        if (retries) retries--;
    }
    lib_cfg->_cdc_tx_pkt_send = 0;
    if (lib_cfg->cdc_rx_bytes_read && lib_cfg->cdc_rx_f) lib_cfg->cdc_rx_f(lib_cfg->cdc_rx_buffer, lib_cfg->cdc_rx_bytes_read);
}


void USB_HID_CDC_Serial_Tx(uint8_t* data, int data_len) {
    
    lib_cfg->_cdc_tx_pkt_send = (data_len > lib_cfg->cdc_tx_buffer_size) ? lib_cfg->cdc_tx_buffer_size : data_len;
    memcpy(lib_cfg->cdc_tx_buffer, data, lib_cfg->_cdc_tx_pkt_send);
}


int USB_HID_CDC_Send_Scan_Code(uint32_t key) { return APP_Keyboard_Send_Key(key); }


int USB_HID_CDC_Send_Char(char c) { return USB_HID_CDC_Send_Scan_Code(APP_Keyboard_Ascii_2_ScanKeyCode(c)); }





//void __attribute__((vector (_USB_VECTOR), interrupt(IPL1SOFT))) USB_ISR() {
//    USBDeviceTasks();
//}

static void __ISR(_USB_VECTOR) _USB1Interrupt() {
    USBDeviceTasks();
}