#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <stm32h7xx_ll_exti.h>
#include <stm32h7xx_ll_gpio.h>
#include <stm32h7xx_ll_system.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_NUMBER (16U)

typedef void (*GpioExtiCallback)(void* context);

typedef struct {
    GpioExtiCallback callback;
    void* context;
} GpioInterrupt;

typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
    GpioModeAltFunctionPushPull,
    GpioModeAltFunctionOpenDrain,
    GpioModeAnalog,
    GpioModeInterruptRise,
    GpioModeInterruptFall,
    GpioModeInterruptRiseFall,
    GpioModeEventRise,
    GpioModeEventFall,
    GpioModeEventRiseFall,
} GpioMode;

typedef enum {
    GpioPullNo,
    GpioPullUp,
    GpioPullDown,
} GpioPull;

typedef enum {
    GpioSpeedLow,
    GpioSpeedMedium,
    GpioSpeedHigh,
    GpioSpeedVeryHigh,
} GpioSpeed;

typedef enum {
    GpioAltFn0SYS = 0,

    GpioAltFn1TIM1 = 1,
    GpioAltFn1TIM2 = 1,
    GpioAltFn1TIM16 = 1,
    GpioAltFn1TIM17 = 1,

    GpioAltFn2TIM3 = 2,
    GpioAltFn2TIM4 = 2,
    GpioAltFn2TIM5 = 2,

    GpioAltFn3TIM8 = 3,
    GpioAltFn3TIM12 = 3,

    GpioAltFn4I2C1 = 4,
    GpioAltFn4I2C2 = 4,

    GpioAltFn5SPI1 = 5,
    GpioAltFn5SPI2 = 5,

    GpioAltFn7USART1 = 7,
    GpioAltFn7USART2 = 7,
    GpioAltFn7USART3 = 7,

    GpioAltFn10USB_OTG_HS = 10,

    GpioAltFn15EVENTOUT = 15,

    GpioAltFnUnused = 16,
} GpioAltFn;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} GpioPin;

void kal_gpio_init_simple(const GpioPin* gpio, const GpioMode mode);

void kal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

void kal_gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn);

void kal_gpio_init_pins(
    GPIO_TypeDef* port,
    const uint16_t mask,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

void kal_gpio_init_port(
    GPIO_TypeDef* port,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

void kal_gpio_add_int_callback(const GpioPin* gpio, GpioExtiCallback cb, void* ctx);

void kal_gpio_enable_int_callback(const GpioPin* gpio);

void kal_gpio_disable_int_callback(const GpioPin* gpio);

void kal_gpio_remove_int_callback(const GpioPin* gpio);

static inline void kal_gpio_write(const GpioPin* gpio, const bool state) {
    if(state == true) {
        gpio->port->BSRR = gpio->pin;
    } else {
        gpio->port->BSRR = (uint32_t)gpio->pin << GPIO_NUMBER;
    }
}

static inline bool kal_gpio_read(const GpioPin* gpio) {
    return (gpio->port->IDR & gpio->pin) != 0x00U;
}

static inline void kal_gpio_write_port_pin(GPIO_TypeDef* port, uint16_t pin, const bool state) {
    if(state == true) {
        port->BSRR = pin;
    } else {
        port->BSRR = (uint32_t)pin << GPIO_NUMBER;
    }
}

static inline bool kal_gpio_read_port_pin(GPIO_TypeDef* port, uint16_t pin) {
    return (port->IDR & pin) != 0x00U;
}

static inline void kal_gpio_set_pins_high(GPIO_TypeDef* port, uint16_t mask) {
    port->BSRR = mask;
}

static inline void kal_gpio_set_pins_low(GPIO_TypeDef* port, uint16_t mask) {
    port->BSRR = (uint32_t)mask << GPIO_NUMBER;
}

static inline void kal_gpio_write_port_value(GPIO_TypeDef* port, uint16_t mask, uint16_t value) {
    port->BSRR = (uint32_t)(value & mask) | ((uint32_t)((uint16_t)~value & mask) << GPIO_NUMBER);
}

static inline uint16_t kal_gpio_read_port_value(GPIO_TypeDef* port, uint16_t mask) {
    return (uint16_t)(port->IDR) & mask;
}

static inline void kal_gpio_set_port_mode_input(GPIO_TypeDef* port) {
    port->MODER = 0x00000000U;
}

static inline void kal_gpio_set_port_mode_output(GPIO_TypeDef* port) {
    port->MODER = 0x55555555U;
}

#ifdef __cplusplus
}
#endif
