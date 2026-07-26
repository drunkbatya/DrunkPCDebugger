#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KAL_DELAY_MAX_TIMEOUT_MS (5000U)

void kal_delay_init(void);

uint32_t kal_delay_get_cycle_counter(void);

bool kal_delay_is_expired(uint32_t started_at, uint32_t timeout_ms);

void kal_delay_cycles(uint32_t cycles);

void kal_delay_ns(uint32_t ns);

void kal_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif
