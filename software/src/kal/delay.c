#include <kal/delay.h>

#include <kal/kal.h>

#include <main.h>

#define KAL_DELAY_DWT_LAR_ADDRESS (0xE0001FB0U)
#define KAL_DELAY_DWT_LAR_UNLOCK  (0xC5ACCE55U)

static void kal_delay_unlock_dwt(void) {
    *(volatile uint32_t*)KAL_DELAY_DWT_LAR_ADDRESS = KAL_DELAY_DWT_LAR_UNLOCK;
}

void kal_delay_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    kal_delay_unlock_dwt();
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t kal_delay_get_cycle_counter(void) {
    return DWT->CYCCNT;
}

void kal_delay_cycles(uint32_t cycles) {
    const uint32_t started_at = DWT->CYCCNT;
    while((DWT->CYCCNT - started_at) < cycles) {
    }
}

void kal_delay_ns(uint32_t ns) {
    kal_delay_cycles((uint32_t)(((uint64_t)ns * SystemCoreClock) / 1000000000ULL) + 1U);
}

void kal_delay_us(uint32_t us) {
    kal_delay_cycles((uint32_t)(((uint64_t)us * SystemCoreClock) / 1000000ULL) + 1U);
}

bool kal_delay_is_expired(uint32_t started_at, uint32_t timeout_ms) {
    if(timeout_ms > KAL_DELAY_MAX_TIMEOUT_MS) kal_crash();

    const uint32_t timeout_cycles = timeout_ms * (SystemCoreClock / 1000U);
    return (DWT->CYCCNT - started_at) >= timeout_cycles;
}
