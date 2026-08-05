#include <kal/gpio.h>
#include <kal/interrupt.h>

#include <kal/kal.h>

#include <main.h>
#include <stm32h7xx_ll_bus.h>

static uint32_t kal_gpio_invalid_argument_crash(void) {
    kal_crash();
    return 0;
}

#define GPIO_PORT_MAP(port, prefix)    \
    (((port) == (GPIOA)) ? prefix##A : \
     ((port) == (GPIOB)) ? prefix##B : \
     ((port) == (GPIOC)) ? prefix##C : \
     ((port) == (GPIOD)) ? prefix##D : \
     ((port) == (GPIOE)) ? prefix##E : \
     ((port) == (GPIOF)) ? prefix##F : \
     ((port) == (GPIOG)) ? prefix##G : \
     ((port) == (GPIOH)) ? prefix##H : \
                           kal_gpio_invalid_argument_crash())

#define GPIO_PIN_MAP(pin, prefix)               \
    (((pin) == (LL_GPIO_PIN_0))  ? prefix##0 :  \
     ((pin) == (LL_GPIO_PIN_1))  ? prefix##1 :  \
     ((pin) == (LL_GPIO_PIN_2))  ? prefix##2 :  \
     ((pin) == (LL_GPIO_PIN_3))  ? prefix##3 :  \
     ((pin) == (LL_GPIO_PIN_4))  ? prefix##4 :  \
     ((pin) == (LL_GPIO_PIN_5))  ? prefix##5 :  \
     ((pin) == (LL_GPIO_PIN_6))  ? prefix##6 :  \
     ((pin) == (LL_GPIO_PIN_7))  ? prefix##7 :  \
     ((pin) == (LL_GPIO_PIN_8))  ? prefix##8 :  \
     ((pin) == (LL_GPIO_PIN_9))  ? prefix##9 :  \
     ((pin) == (LL_GPIO_PIN_10)) ? prefix##10 : \
     ((pin) == (LL_GPIO_PIN_11)) ? prefix##11 : \
     ((pin) == (LL_GPIO_PIN_12)) ? prefix##12 : \
     ((pin) == (LL_GPIO_PIN_13)) ? prefix##13 : \
     ((pin) == (LL_GPIO_PIN_14)) ? prefix##14 : \
     ((pin) == (LL_GPIO_PIN_15)) ? prefix##15 : \
                                   kal_gpio_invalid_argument_crash())

#define GET_SYSCFG_EXTI_PORT(port) GPIO_PORT_MAP(port, LL_SYSCFG_EXTI_PORT)
#define GET_SYSCFG_EXTI_LINE(pin)  GPIO_PIN_MAP(pin, LL_SYSCFG_EXTI_LINE)
#define GET_EXTI_LINE(pin)         GPIO_PIN_MAP(pin, LL_EXTI_LINE_)

static volatile GpioInterrupt gpio_interrupt[GPIO_NUMBER];

static uint8_t kal_gpio_get_pin_num(const GpioPin* gpio) {
    for(uint8_t pin_num = 0; pin_num < GPIO_NUMBER; pin_num++) {
        if(gpio->pin & (1U << pin_num)) return pin_num;
    }
    return kal_gpio_invalid_argument_crash();
}

static void kal_gpio_set_speed(const GpioPin* gpio, const GpioSpeed speed) {
    switch(speed) {
    case GpioSpeedLow:
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_LOW);
        break;
    case GpioSpeedMedium:
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_MEDIUM);
        break;
    case GpioSpeedHigh:
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_HIGH);
        break;
    case GpioSpeedVeryHigh:
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        break;
    default:
        kal_crash();
    }
}

static void kal_gpio_set_pull(const GpioPin* gpio, const GpioPull pull) {
    switch(pull) {
    case GpioPullNo:
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_NO);
        break;
    case GpioPullUp:
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_UP);
        break;
    case GpioPullDown:
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_DOWN);
        break;
    default:
        kal_crash();
    }
}

static void kal_gpio_set_alt_fn(const GpioPin* gpio, const GpioAltFn alt_fn) {
    if(kal_gpio_get_pin_num(gpio) < 8) {
        LL_GPIO_SetAFPin_0_7(gpio->port, gpio->pin, alt_fn);
    } else {
        LL_GPIO_SetAFPin_8_15(gpio->port, gpio->pin, alt_fn);
    }
}

static void kal_gpio_set_exti_mode(const GpioPin* gpio, const GpioMode mode) {
    const uint32_t exti_line = GET_EXTI_LINE(gpio->pin);

    LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);
    LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_INPUT);
    LL_SYSCFG_SetEXTISource(GET_SYSCFG_EXTI_PORT(gpio->port), GET_SYSCFG_EXTI_LINE(gpio->pin));

    if(mode == GpioModeInterruptRise || mode == GpioModeInterruptRiseFall) {
        LL_EXTI_EnableRisingTrig_0_31(exti_line);
    }
    if(mode == GpioModeInterruptFall || mode == GpioModeInterruptRiseFall) {
        LL_EXTI_EnableFallingTrig_0_31(exti_line);
    }
    if(mode == GpioModeEventRise || mode == GpioModeEventRiseFall) {
        LL_EXTI_EnableEvent_0_31(exti_line);
        LL_EXTI_EnableRisingTrig_0_31(exti_line);
    }
    if(mode == GpioModeEventFall || mode == GpioModeEventRiseFall) {
        LL_EXTI_EnableEvent_0_31(exti_line);
        LL_EXTI_EnableFallingTrig_0_31(exti_line);
    }
}

static void kal_gpio_clear_exti_mode(const GpioPin* gpio) {
    const uint32_t sys_exti_port = GET_SYSCFG_EXTI_PORT(gpio->port);
    const uint32_t sys_exti_line = GET_SYSCFG_EXTI_LINE(gpio->pin);
    const uint32_t exti_line = GET_EXTI_LINE(gpio->pin);

    if(LL_SYSCFG_GetEXTISource(sys_exti_line) != sys_exti_port) return;
    if(!LL_EXTI_IsEnabledIT_0_31(exti_line)) return;

    LL_EXTI_DisableIT_0_31(exti_line);
    LL_EXTI_ClearFlag_0_31(exti_line);
    LL_EXTI_DisableRisingTrig_0_31(exti_line);
    LL_EXTI_DisableFallingTrig_0_31(exti_line);
}

static void kal_gpio_set_mode(const GpioPin* gpio, const GpioMode mode) {
    switch(mode) {
    case GpioModeInput:
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_INPUT);
        break;
    case GpioModeOutputPushPull:
        LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_OUTPUT);
        break;
    case GpioModeOutputOpenDrain:
        LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_OPENDRAIN);
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_OUTPUT);
        break;
    case GpioModeAltFunctionPushPull:
        LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_ALTERNATE);
        break;
    case GpioModeAltFunctionOpenDrain:
        LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_OPENDRAIN);
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_ALTERNATE);
        break;
    case GpioModeAnalog:
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_ANALOG);
        break;
    default:
        kal_crash();
    }
}

void kal_gpio_init_simple(const GpioPin* gpio, const GpioMode mode) {
    kal_gpio_init(gpio, mode, GpioPullNo, GpioSpeedLow);
}

void kal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    if(mode == GpioModeAltFunctionPushPull || mode == GpioModeAltFunctionOpenDrain) {
        kal_crash();
    }
    kal_gpio_init_ex(gpio, mode, pull, speed, GpioAltFnUnused);
}

void kal_gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn) {
    kal_interrupt_critical_enter();

    kal_gpio_set_speed(gpio, speed);
    kal_gpio_set_pull(gpio, pull);

    if(mode >= GpioModeInterruptRise) {
        kal_gpio_set_exti_mode(gpio, mode);
    } else {
        kal_gpio_clear_exti_mode(gpio);
        if(mode == GpioModeAltFunctionPushPull || mode == GpioModeAltFunctionOpenDrain) {
            kal_gpio_set_alt_fn(gpio, alt_fn);
        }
        kal_gpio_set_mode(gpio, mode);
    }

    kal_interrupt_critical_exit();
}

void kal_gpio_init_pins(
    GPIO_TypeDef* port,
    const uint16_t mask,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    for(uint8_t pin_num = 0; pin_num < GPIO_NUMBER; pin_num++) {
        const uint16_t pin = 1U << pin_num;
        if((mask & pin) == 0) continue;

        const GpioPin gpio = {.port = port, .pin = pin};
        kal_gpio_init(&gpio, mode, pull, speed);
    }
}

static uint32_t kal_gpio_repeat_2bit(uint32_t value) {
    return value * 0x55555555U;
}

static uint32_t kal_gpio_get_port_moder(const GpioMode mode) {
    switch(mode) {
    case GpioModeInput:
        return kal_gpio_repeat_2bit(LL_GPIO_MODE_INPUT);
    case GpioModeOutputPushPull:
    case GpioModeOutputOpenDrain:
        return kal_gpio_repeat_2bit(LL_GPIO_MODE_OUTPUT);
    case GpioModeAnalog:
        return kal_gpio_repeat_2bit(LL_GPIO_MODE_ANALOG);
    default:
        return kal_gpio_invalid_argument_crash();
    }
}

static uint32_t kal_gpio_get_port_pupdr(const GpioPull pull) {
    switch(pull) {
    case GpioPullNo:
        return kal_gpio_repeat_2bit(LL_GPIO_PULL_NO);
    case GpioPullUp:
        return kal_gpio_repeat_2bit(LL_GPIO_PULL_UP);
    case GpioPullDown:
        return kal_gpio_repeat_2bit(LL_GPIO_PULL_DOWN);
    default:
        return kal_gpio_invalid_argument_crash();
    }
}

static uint32_t kal_gpio_get_port_ospeedr(const GpioSpeed speed) {
    switch(speed) {
    case GpioSpeedLow:
        return kal_gpio_repeat_2bit(LL_GPIO_SPEED_FREQ_LOW);
    case GpioSpeedMedium:
        return kal_gpio_repeat_2bit(LL_GPIO_SPEED_FREQ_MEDIUM);
    case GpioSpeedHigh:
        return kal_gpio_repeat_2bit(LL_GPIO_SPEED_FREQ_HIGH);
    case GpioSpeedVeryHigh:
        return kal_gpio_repeat_2bit(LL_GPIO_SPEED_FREQ_VERY_HIGH);
    default:
        return kal_gpio_invalid_argument_crash();
    }
}

static uint32_t kal_gpio_get_port_otyper(const GpioMode mode) {
    return mode == GpioModeOutputOpenDrain ? 0x0000FFFFU : 0x00000000U;
}

void kal_gpio_init_port(
    GPIO_TypeDef* port,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    kal_interrupt_critical_enter();

    port->OSPEEDR = kal_gpio_get_port_ospeedr(speed);
    port->OTYPER = kal_gpio_get_port_otyper(mode);
    port->PUPDR = kal_gpio_get_port_pupdr(pull);
    port->MODER = kal_gpio_get_port_moder(mode);

    kal_interrupt_critical_exit();
}

void kal_gpio_add_int_callback(const GpioPin* gpio, GpioExtiCallback cb, void* ctx) {
    if(gpio == NULL || cb == NULL) kal_crash();

    kal_interrupt_critical_enter();

    const uint8_t pin_num = kal_gpio_get_pin_num(gpio);
    if(gpio_interrupt[pin_num].callback != NULL) kal_crash();

    gpio_interrupt[pin_num].callback = cb;
    gpio_interrupt[pin_num].context = ctx;
    LL_EXTI_EnableIT_0_31(GET_EXTI_LINE(gpio->pin));

    kal_interrupt_critical_exit();
}

void kal_gpio_enable_int_callback(const GpioPin* gpio) {
    if(gpio == NULL) kal_crash();

    kal_interrupt_critical_enter();
    LL_EXTI_EnableIT_0_31(GET_EXTI_LINE(gpio->pin));
    kal_interrupt_critical_exit();
}

void kal_gpio_disable_int_callback(const GpioPin* gpio) {
    if(gpio == NULL) kal_crash();

    const uint32_t exti_line = GET_EXTI_LINE(gpio->pin);

    kal_interrupt_critical_enter();
    LL_EXTI_DisableIT_0_31(exti_line);
    LL_EXTI_ClearFlag_0_31(exti_line);
    kal_interrupt_critical_exit();
}

void kal_gpio_remove_int_callback(const GpioPin* gpio) {
    if(gpio == NULL) kal_crash();

    const uint32_t exti_line = GET_EXTI_LINE(gpio->pin);

    kal_interrupt_critical_enter();

    LL_EXTI_DisableIT_0_31(exti_line);
    LL_EXTI_ClearFlag_0_31(exti_line);

    const uint8_t pin_num = kal_gpio_get_pin_num(gpio);
    gpio_interrupt[pin_num].callback = NULL;
    gpio_interrupt[pin_num].context = NULL;

    kal_interrupt_critical_exit();
}

static inline void kal_gpio_int_call(uint16_t pin_num) {
    if(gpio_interrupt[pin_num].callback) {
        gpio_interrupt[pin_num].callback(gpio_interrupt[pin_num].context);
    }
}

static inline void kal_gpio_int_handle_line(uint32_t exti_line, uint16_t pin_num) {
    if(LL_EXTI_IsActiveFlag_0_31(exti_line)) {
        LL_EXTI_ClearFlag_0_31(exti_line);
        kal_gpio_int_call(pin_num);
    }
}

void EXTI0_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_0, 0);
}

void EXTI1_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_1, 1);
}

void EXTI2_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_2, 2);
}

void EXTI3_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_3, 3);
}

void EXTI4_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_4, 4);
}

void EXTI9_5_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_5, 5);
    kal_gpio_int_handle_line(LL_EXTI_LINE_6, 6);
    kal_gpio_int_handle_line(LL_EXTI_LINE_7, 7);
    kal_gpio_int_handle_line(LL_EXTI_LINE_8, 8);
    kal_gpio_int_handle_line(LL_EXTI_LINE_9, 9);
}

void EXTI15_10_IRQHandler(void) {
    kal_gpio_int_handle_line(LL_EXTI_LINE_10, 10);
    kal_gpio_int_handle_line(LL_EXTI_LINE_11, 11);
    kal_gpio_int_handle_line(LL_EXTI_LINE_12, 12);
    kal_gpio_int_handle_line(LL_EXTI_LINE_13, 13);
    kal_gpio_int_handle_line(LL_EXTI_LINE_14, 14);
    kal_gpio_int_handle_line(LL_EXTI_LINE_15, 15);
}
