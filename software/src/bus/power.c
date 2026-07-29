#include <bus/power.h>

#include <kal/gpio.h>
#include <kal/resources.h>

void bus_power_set_enabled(bool enabled) {
    const bool level = enabled ? BUS_PWR_ENABLED_LEVEL : !BUS_PWR_ENABLED_LEVEL;
    kal_gpio_write(&gpio_bus_pwr_pin, level);
}

bool bus_power_is_enabled(void) {
    return kal_gpio_read(&gpio_bus_pwr_pin) == BUS_PWR_ENABLED_LEVEL;
}
