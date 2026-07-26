#include <eeprom/at28c256.h>

#include <bus/bus.h>
#include <kal/delay.h>

#define AT28C256_POLL_BIT            (0x80U)
#define AT28C256_BYTE_LOAD_WINDOW_US (200U)
#define AT28C256_WRITE_TIMEOUT_MS    (20U)

static size_t at28c256_page_space(uint16_t address) {
    return AT28C256_PAGE_SIZE - (address % AT28C256_PAGE_SIZE);
}

static size_t at28c256_chunk_size(uint16_t address, size_t remaining) {
    const size_t space = at28c256_page_space(address);
    return space < remaining ? space : remaining;
}

static void at28c256_load_page(uint16_t address, const uint8_t* data, size_t size) {
    for(size_t i = 0; i < size; i++) {
        bus_write_memory((uint16_t)(address + i), data[i]);
    }
}

static bool at28c256_poll_write_complete(uint16_t address, uint8_t expected) {
    const uint8_t expected_bit = expected & AT28C256_POLL_BIT;
    const uint32_t started_at = kal_delay_get_cycle_counter();

    while((bus_read_memory(address) & AT28C256_POLL_BIT) != expected_bit) {
        if(kal_delay_is_expired(started_at, AT28C256_WRITE_TIMEOUT_MS)) return false;
    }
    return true;
}

static bool at28c256_write_page(uint16_t address, const uint8_t* data, size_t size) {
    at28c256_load_page(address, data, size);
    kal_delay_us(AT28C256_BYTE_LOAD_WINDOW_US);

    return at28c256_poll_write_complete((uint16_t)(address + size - 1), data[size - 1]);
}

bool at28c256_write(uint16_t address, const uint8_t* data, size_t size) {
    if(!bus_is_acquired()) return false;

    size_t written = 0;
    while(written < size) {
        const uint16_t page_address = (uint16_t)(address + written);
        const size_t chunk = at28c256_chunk_size(page_address, size - written);

        if(!at28c256_write_page(page_address, data + written, chunk)) return false;

        written += chunk;
    }
    return true;
}

bool at28c256_read(uint16_t address, uint8_t* data, size_t size) {
    if(!bus_is_acquired()) return false;

    for(size_t i = 0; i < size; i++) {
        data[i] = bus_read_memory((uint16_t)(address + i));
    }
    return true;
}
