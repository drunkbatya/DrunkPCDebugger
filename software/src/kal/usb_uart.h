#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KAL_USB_UART_RX_BUFFER_SIZE (4096U)
#define KAL_USB_UART_WAIT_FOREVER   (0xFFFFFFFFU)

void kal_usb_uart_init(void);

bool kal_usb_uart_is_connected(void);

size_t kal_usb_uart_rx_available(void);

size_t kal_usb_uart_rx(uint8_t* data, size_t size, uint32_t timeout_ms);

void kal_usb_uart_rx_flush(void);

bool kal_usb_uart_rx_take_overrun(void);

bool kal_usb_uart_tx(const uint8_t* data, size_t size, uint32_t timeout_ms);

void kal_usb_uart_on_rx(const uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif
