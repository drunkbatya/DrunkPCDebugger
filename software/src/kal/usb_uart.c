#include <kal/usb_uart.h>

#include <kal/delay.h>

#include <main.h>
#include <usbd_cdc.h>
#include <usbd_cdc_if.h>
#include <usbd_ctlreq.h>
#include <usbd_def.h>

#define KAL_USB_UART_RX_BUFFER_MASK (KAL_USB_UART_RX_BUFFER_SIZE - 1U)

extern USBD_HandleTypeDef hUsbDeviceHS;

static uint8_t kal_usb_uart_rx_buffer[KAL_USB_UART_RX_BUFFER_SIZE];
static volatile uint32_t kal_usb_uart_rx_head = 0;
static volatile uint32_t kal_usb_uart_rx_tail = 0;
static volatile bool kal_usb_uart_rx_overrun = false;

static USBD_CDC_HandleTypeDef* kal_usb_uart_get_cdc_handle(void) {
    return (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
}

static size_t kal_usb_uart_get_rx_count(void) {
    return kal_usb_uart_rx_head - kal_usb_uart_rx_tail;
}

static size_t kal_usb_uart_get_rx_space(void) {
    return KAL_USB_UART_RX_BUFFER_SIZE - kal_usb_uart_get_rx_count();
}

static void kal_usb_uart_rx_reset(void) {
    kal_usb_uart_rx_head = 0;
    kal_usb_uart_rx_tail = 0;
    kal_usb_uart_rx_overrun = false;
}

static void kal_usb_uart_rx_push(const uint8_t* data, size_t size) {
    for(size_t i = 0; i < size; i++) {
        if(kal_usb_uart_get_rx_space() == 0) {
            kal_usb_uart_rx_overrun = true;
            return;
        }
        kal_usb_uart_rx_buffer[kal_usb_uart_rx_head & KAL_USB_UART_RX_BUFFER_MASK] = data[i];
        kal_usb_uart_rx_head++;
    }
}

static size_t kal_usb_uart_rx_pop(uint8_t* data, size_t size) {
    size_t count = kal_usb_uart_get_rx_count();
    if(count > size) count = size;

    for(size_t i = 0; i < count; i++) {
        data[i] = kal_usb_uart_rx_buffer[kal_usb_uart_rx_tail & KAL_USB_UART_RX_BUFFER_MASK];
        kal_usb_uart_rx_tail++;
    }
    return count;
}

void kal_usb_uart_on_rx(const uint8_t* data, size_t size) {
    kal_usb_uart_rx_push(data, size);
}

//static char kal_usb_uart_get_hex_digit(uint32_t value) {
//    return value < 10U ? (char)('0' + value) : (char)('A' + value - 10U);
//}

//static uint32_t kal_usb_uart_get_device_id(void) {
//    const volatile uint32_t* uid = (const volatile uint32_t*)UID_BASE;
//    return uid[0] ^ uid[1] ^ uid[2];
//}

void kal_usb_uart_get_serial_descriptor(uint8_t* descriptor, uint16_t* length) {
    static const char prefix[] = KAL_USB_UART_SERIAL_PREFIX;
    USBD_GetString((uint8_t*)prefix, descriptor, length);
}

void kal_usb_uart_init(void) {
    kal_usb_uart_rx_reset();
}

bool kal_usb_uart_is_connected(void) {
    return hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED;
}

size_t kal_usb_uart_get_rx_available(void) {
    return kal_usb_uart_get_rx_count();
}

size_t kal_usb_uart_rx(uint8_t* data, size_t size, uint32_t timeout_ms) {
    const uint32_t started_at = kal_delay_get_cycle_counter();
    size_t received = 0;

    while(received < size) {
        received += kal_usb_uart_rx_pop(data + received, size - received);
        if(received == size) break;
        if(kal_delay_is_expired(started_at, timeout_ms)) break;
    }
    return received;
}

void kal_usb_uart_rx_flush(void) {
    kal_usb_uart_rx_tail = kal_usb_uart_rx_head;
}

bool kal_usb_uart_take_rx_overrun(void) {
    const bool overrun = kal_usb_uart_rx_overrun;
    kal_usb_uart_rx_overrun = false;
    return overrun;
}

static bool kal_usb_uart_is_tx_busy(void) {
    const USBD_CDC_HandleTypeDef* hcdc = kal_usb_uart_get_cdc_handle();
    if(hcdc == NULL) return false;
    return hcdc->TxState != 0;
}

static bool kal_usb_uart_tx_wait_idle(uint32_t started_at, uint32_t timeout_ms) {
    while(kal_usb_uart_is_tx_busy()) {
        if(!kal_usb_uart_is_connected()) return false;
        if(kal_delay_is_expired(started_at, timeout_ms)) return false;
    }
    return true;
}

static bool kal_usb_uart_tx_packet(
    const uint8_t* data,
    size_t size,
    uint32_t started_at,
    uint32_t timeout_ms) {
    if(!kal_usb_uart_tx_wait_idle(started_at, timeout_ms)) return false;
    if(!kal_usb_uart_is_connected()) return false;

    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, (uint8_t*)data, (uint16_t)size);
    return USBD_CDC_TransmitPacket(&hUsbDeviceHS) == USBD_OK;
}

static size_t kal_usb_uart_get_tx_packet_size(size_t remaining) {
    if(remaining > CDC_DATA_HS_MAX_PACKET_SIZE) return CDC_DATA_HS_MAX_PACKET_SIZE;
    return remaining;
}

static bool kal_usb_uart_needs_zero_length_packet(size_t size) {
    return size != 0 && (size % CDC_DATA_HS_MAX_PACKET_SIZE) == 0;
}

bool kal_usb_uart_tx(const uint8_t* data, size_t size, uint32_t timeout_ms) {
    const uint32_t started_at = kal_delay_get_cycle_counter();
    size_t sent = 0;

    while(sent < size) {
        const size_t packet_size = kal_usb_uart_get_tx_packet_size(size - sent);
        if(!kal_usb_uart_tx_packet(data + sent, packet_size, started_at, timeout_ms)) return false;
        sent += packet_size;
    }

    if(kal_usb_uart_needs_zero_length_packet(size)) {
        if(!kal_usb_uart_tx_packet(data, 0, started_at, timeout_ms)) return false;
    }

    return kal_usb_uart_tx_wait_idle(started_at, timeout_ms);
}
