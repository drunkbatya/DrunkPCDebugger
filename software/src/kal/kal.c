#include <kal/kal.h>

#include <kal/delay.h>
#include <kal/gpio.h>
#include <kal/interrupt.h>
#include <kal/resources.h>
#include <kal/usb_uart.h>

#include <gpio.h>
#include <main.h>
#include <stm32h7xx_ll_bus.h>

static void kal_enable_gpio_port_clocks(void) {
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
}

static void kal_init_pin_as_output(const GpioPin* gpio, bool level, GpioSpeed speed) {
    kal_gpio_write(gpio, level);
    kal_gpio_init(gpio, GpioModeOutputPushPull, GpioPullNo, speed);
}

static void kal_init_transceivers_disabled(void) {
    kal_init_pin_as_output(&gpio_bus_oe_pin, !BUS_OE_ENABLED_LEVEL, GpioSpeedLow);
    kal_init_pin_as_output(&gpio_addr_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL, GpioSpeedLow);
    kal_init_pin_as_output(&gpio_data_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL, GpioSpeedLow);
    kal_init_pin_as_output(&gpio_ctrl_bus_dir_pin, BUS_DIR_TO_MCU_LEVEL, GpioSpeedLow);
}

static void kal_init_address_bus_as_input(void) {
    kal_gpio_init_port(BUS_ADDR_PORT, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
}

static void kal_init_data_bus_as_input(void) {
    kal_gpio_init_port(BUS_DATA_PORT, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
}

static void kal_init_control_bus_as_input(void) {
    kal_gpio_init_pins(
        BUS_CONTROL_PORT, BUS_CONTROL_MASK, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
}

static void kal_init_cpu_status_as_input(void) {
    kal_gpio_init(&gpio_busack_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    kal_gpio_init(&gpio_wait_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    kal_gpio_init(&gpio_m1_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    kal_gpio_init(&gpio_cpu_clk_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
}

static void kal_init_cpu_control_as_output(void) {
    kal_init_pin_as_output(&gpio_busreq_pin, !BUSREQ_ACTIVE_LEVEL, GpioSpeedLow);
    kal_init_pin_as_output(&gpio_cpurst_pin, !CPURST_ACTIVE_LEVEL, GpioSpeedLow);
}

static void kal_init_bus_power_disabled(void) {
    kal_init_pin_as_output(&gpio_bus_pwr_pin, !BUS_PWR_ENABLED_LEVEL, GpioSpeedLow);
}

static void kal_init_safe_state(void) {
    kal_enable_gpio_port_clocks();
    kal_init_transceivers_disabled();

    kal_init_address_bus_as_input();
    kal_init_data_bus_as_input();
    kal_init_control_bus_as_input();
    kal_init_cpu_status_as_input();

    kal_init_cpu_control_as_output();
    kal_init_bus_power_disabled();
}

void kal_crash(void) {
    static volatile bool crashing = false;

    __disable_irq();

    if(!crashing) {
        crashing = true;
        kal_gpio_set_pins_high(BUS_CONTROL_PORT, BUS_CONTROL_MASK);
        kal_init_safe_state();
    }

    while(1) {
    }
}

void MX_GPIO_Init(void) {
    kal_init_safe_state();
}

void kal_init(void) {
    kal_delay_init();
    kal_interrupt_init();
    kal_init_safe_state();
    kal_usb_uart_init();
}
