#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KAL_USB_UART_RX_BUFFER_SIZE (4096U)
#define KAL_USB_UART_SERIAL_PREFIX  "_dpcdbg_"

void kal_usb_uart_init(void);

bool kal_usb_uart_is_connected(void);

size_t kal_usb_uart_get_rx_available(void);

size_t kal_usb_uart_rx(uint8_t* data, size_t size, uint32_t timeout_ms);

void kal_usb_uart_rx_flush(void);

bool kal_usb_uart_take_rx_overrun(void);

bool kal_usb_uart_tx(const uint8_t* data, size_t size, uint32_t timeout_ms);

void kal_usb_uart_on_rx(const uint8_t* data, size_t size);

void kal_usb_uart_get_serial_descriptor(uint8_t* descriptor, uint16_t* length);

#ifdef __cplusplus
}
#endif
