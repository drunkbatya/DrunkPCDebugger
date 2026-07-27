#pragma once

#include <kal/gpio.h>

#include <main.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUS_ADDR_PORT  GPIOD
#define BUS_ADDR_MASK  (0xFFFFU)
#define BUS_ADDR_WIDTH (16U)

#define BUS_DATA_PORT  GPIOE
#define BUS_DATA_MASK  (0x00FFU)
#define BUS_DATA_WIDTH (8U)

#define BUS_CONTROL_PORT GPIOB
#define BUS_CONTROL_MASK (_WR_Pin | _RD_Pin | _IORQ_Pin | _MEMRQ_Pin | _NMI_Pin | _INT_Pin)

#define BUS_OE_ENABLED_LEVEL  (false)
#define BUS_DIR_TO_MCU_LEVEL  (true)
#define BUS_DIR_TO_CPU_LEVEL  (false)
#define BUS_PWR_ENABLED_LEVEL (true)

#define BUSREQ_ACTIVE_LEVEL (false)
#define BUSACK_ACTIVE_LEVEL (false)
#define CPURST_ACTIVE_LEVEL (false)
#define WAIT_ACTIVE_LEVEL   (false)
#define NMI_ACTIVE_LEVEL    (false)
#define INT_ACTIVE_LEVEL    (false)
#define M1_ACTIVE_LEVEL     (false)
#define RD_ACTIVE_LEVEL     (false)
#define WR_ACTIVE_LEVEL     (false)
#define IORQ_ACTIVE_LEVEL   (false)
#define MEMRQ_ACTIVE_LEVEL  (false)

extern const GpioPin gpio_led_pin;
extern const GpioPin gpio_bus_pwr_pin;

extern const GpioPin gpio_addr_bus_dir_pin;
extern const GpioPin gpio_data_bus_dir_pin;
extern const GpioPin gpio_ctrl_bus_dir_pin;
extern const GpioPin gpio_bus_oe_pin;

extern const GpioPin gpio_wr_pin;
extern const GpioPin gpio_rd_pin;
extern const GpioPin gpio_iorq_pin;
extern const GpioPin gpio_memrq_pin;
extern const GpioPin gpio_m1_pin;

extern const GpioPin gpio_busreq_pin;
extern const GpioPin gpio_busack_pin;
extern const GpioPin gpio_cpurst_pin;
extern const GpioPin gpio_wait_pin;
extern const GpioPin gpio_nmi_pin;
extern const GpioPin gpio_int_pin;
extern const GpioPin gpio_cpu_clk_pin;

extern const GpioPin gpio_addr_pins[BUS_ADDR_WIDTH];
extern const GpioPin gpio_data_pins[BUS_DATA_WIDTH];

#ifdef __cplusplus
}
#endif
