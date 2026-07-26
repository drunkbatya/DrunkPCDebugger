#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUS_ADDRESS_MAX   (0xFFFFU)
#define BUS_ADDRESS_SPACE (0x10000U)

void bus_init(void);

bool bus_acquire(void);

bool bus_release(void);

bool bus_is_acquired(void);

void bus_write_memory(uint16_t address, uint8_t data);

uint8_t bus_read_memory(uint16_t address);

#ifdef __cplusplus
}
#endif
