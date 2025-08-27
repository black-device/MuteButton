#ifndef USB_HID_CDC_H
#define USB_HID_CDC_H

#include <stddef.h>


typedef void (*usb_hid_cdc_serial_rx_f)(uint8_t* data, int data_len);


typedef struct {
    
    uint8_t* cdc_rx_buffer;
    int cdc_rx_buffer_size;
    int cdc_rx_bytes_read;
    
    usb_hid_cdc_serial_rx_f cdc_rx_f;
    
    uint8_t* cdc_tx_buffer;
    int cdc_tx_buffer_size;
    int _cdc_tx_pkt_send;
    
} usb_hid_cdc_cfg_t;


void USB_HID_CDC_Initialize(usb_hid_cdc_cfg_t* cfg);

void USB_HID_CDC_Tasks();

int USB_HID_CDC_Send_Scan_Code(uint32_t key);

int USB_HID_CDC_Send_Char(char c);

void USB_HID_CDC_Serial_Tx(uint8_t* data, int data_len);


#endif

