#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AT28C256_PAGE_SIZE (64U)

bool at28c256_write(uint16_t address, const uint8_t* data, size_t size);

bool at28c256_read(uint16_t address, uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif
