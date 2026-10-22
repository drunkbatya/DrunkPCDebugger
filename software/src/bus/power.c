#include <bus/power.h>

#include <kal/delay.h>
#include <kal/gpio.h>
#include <kal/resources.h>

#define BUS_POWER_RISE_TIMEOUT_MS (250U)

static bool bus_power_wait_present(uint32_t timeout_ms) {
    const uint32_t started_at = kal_delay_get_cycle_counter();

    while(!bus_power_is_present()) {
        if(kal_delay_is_expired(started_at, timeout_ms)) return false;
    }
    return true;
}

void bus_power_set_enabled(bool enabled) {
    const bool level = enabled ? BUS_PWR_ENABLED_LEVEL : !BUS_PWR_ENABLED_LEVEL;
    kal_gpio_write(&gpio_bus_pwr_pin, level);
}

bool bus_power_is_enabled(void) {
    return kal_gpio_read(&gpio_bus_pwr_pin) == BUS_PWR_ENABLED_LEVEL;
}

bool bus_power_is_present(void) {
    return kal_gpio_read(&gpio_bus_5v_sense_pin) == BUS_5V_PRESENT_LEVEL;
}

bool bus_power_turn_on(void) {
    bus_power_set_enabled(true);
    if(bus_power_wait_present(BUS_POWER_RISE_TIMEOUT_MS)) return true;

    bus_power_set_enabled(false);
    return false;
}
