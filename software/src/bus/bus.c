#include <bus/bus.h>

#include <kal/delay.h>
#include <kal/gpio.h>
#include <kal/resources.h>

#define BUS_ACQUIRE_TIMEOUT_MS (3000U)
#define BUS_RELEASE_TIMEOUT_MS (3000U)

#define BUS_ADDRESS_SETUP_NS   (100U)
#define BUS_WRITE_PULSE_NS     (200U)
#define BUS_WRITE_HOLD_NS      (100U)
#define BUS_READ_ACCESS_NS     (300U)
#define BUS_DIRECTION_SETUP_NS (50U)
#define BUS_RECOVERY_NS        (100U)

static void bus_transceivers_disable(void) {
    kal_gpio_write(&gpio_bus_oe_pin, !BUS_OE_ENABLED_LEVEL);
}

static void bus_transceivers_enable(void) {
    kal_gpio_write(&gpio_bus_oe_pin, BUS_OE_ENABLED_LEVEL);
}

static void bus_addr_set_output(void) {
    kal_gpio_write(&gpio_addr_bus_dir_pin, BUS_DIR_TO_CPU_LEVEL);
    kal_gpio_set_port_mode_output(BUS_ADDR_PORT);
}

static void bus_addr_set_input(void) {
    kal_gpio_set_port_mode_input(BUS_ADDR_PORT);
    kal_gpio_write(&gpio_addr_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL);
}

static void bus_data_set_output(void) {
    kal_gpio_write(&gpio_data_bus_dir_pin, BUS_DIR_TO_CPU_LEVEL);
    kal_gpio_set_port_mode_output(BUS_DATA_PORT);
}

static void bus_data_set_input(void) {
    kal_gpio_set_port_mode_input(BUS_DATA_PORT);
    kal_gpio_write(&gpio_data_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL);
}

static void bus_control_deassert_all(void) {
    kal_gpio_set_pins_high(BUS_CONTROL_PORT, BUS_CONTROL_MASK);
}

static void bus_control_set_output(void) {
    kal_gpio_write(&gpio_ctrl_bus_dir_pin, BUS_DIR_TO_CPU_LEVEL);
    bus_control_deassert_all();
    kal_gpio_init_pins(
        BUS_CONTROL_PORT, BUS_CONTROL_MASK, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
}

static void bus_control_set_input(void) {
    kal_gpio_init_pins(
        BUS_CONTROL_PORT, BUS_CONTROL_MASK, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    kal_gpio_write(&gpio_ctrl_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL);
}

static void bus_take_control(void) {
    bus_transceivers_disable();
    bus_control_set_output();
    bus_addr_set_output();
    bus_data_set_input();
    bus_transceivers_enable();
}

static void bus_give_control(void) {
    bus_transceivers_disable();
    bus_addr_set_input();
    bus_data_set_input();
    bus_control_set_input();
}

static bool bus_wait_busack(bool level, uint32_t timeout_ms) {
    const uint32_t started_at = kal_delay_get_cycle_counter();

    while(kal_gpio_read(&gpio_busack_pin) != level) {
        if(kal_delay_is_expired(started_at, timeout_ms)) return false;
    }
    return true;
}

void bus_init(void) {
    bus_give_control();
    kal_gpio_write(&gpio_busreq_pin, !BUSREQ_ACTIVE_LEVEL);
}

bool bus_is_acquired(void) {
    return kal_gpio_read(&gpio_busack_pin) == BUSACK_ACTIVE_LEVEL;
}

bool bus_acquire(void) {
    kal_gpio_write(&gpio_busreq_pin, BUSREQ_ACTIVE_LEVEL);

    if(!bus_wait_busack(BUSACK_ACTIVE_LEVEL, BUS_ACQUIRE_TIMEOUT_MS)) {
        kal_gpio_write(&gpio_busreq_pin, !BUSREQ_ACTIVE_LEVEL);
        return false;
    }

    bus_take_control();
    return true;
}

bool bus_release(void) {
    bus_give_control();
    kal_gpio_write(&gpio_busreq_pin, !BUSREQ_ACTIVE_LEVEL);

    return bus_wait_busack(!BUSACK_ACTIVE_LEVEL, BUS_RELEASE_TIMEOUT_MS);
}

void bus_write_memory(uint16_t address, uint8_t data) {
    bus_data_set_output();
    kal_gpio_write_port_value(BUS_ADDR_PORT, BUS_ADDR_MASK, address);
    kal_gpio_write_port_value(BUS_DATA_PORT, BUS_DATA_MASK, data);
    kal_delay_ns(BUS_ADDRESS_SETUP_NS);

    kal_gpio_write(&gpio_memrq_pin, MEMRQ_ACTIVE_LEVEL);
    kal_gpio_write(&gpio_wr_pin, WR_ACTIVE_LEVEL);
    kal_delay_ns(BUS_WRITE_PULSE_NS);

    kal_gpio_write(&gpio_wr_pin, !WR_ACTIVE_LEVEL);
    kal_delay_ns(BUS_WRITE_HOLD_NS);

    kal_gpio_write(&gpio_memrq_pin, !MEMRQ_ACTIVE_LEVEL);
    kal_delay_ns(BUS_RECOVERY_NS);
}

uint8_t bus_read_memory(uint16_t address) {
    bus_data_set_input();
    kal_delay_ns(BUS_DIRECTION_SETUP_NS);

    kal_gpio_write_port_value(BUS_ADDR_PORT, BUS_ADDR_MASK, address);
    kal_delay_ns(BUS_ADDRESS_SETUP_NS);

    kal_gpio_write(&gpio_memrq_pin, MEMRQ_ACTIVE_LEVEL);
    kal_gpio_write(&gpio_rd_pin, RD_ACTIVE_LEVEL);
    kal_delay_ns(BUS_READ_ACCESS_NS);

    const uint8_t data = (uint8_t)kal_gpio_read_port_value(BUS_DATA_PORT, BUS_DATA_MASK);

    kal_gpio_write(&gpio_rd_pin, !RD_ACTIVE_LEVEL);
    kal_gpio_write(&gpio_memrq_pin, !MEMRQ_ACTIVE_LEVEL);
    kal_delay_ns(BUS_RECOVERY_NS);

    return data;
}
