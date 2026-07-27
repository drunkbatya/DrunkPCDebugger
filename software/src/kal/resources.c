#include <kal/resources.h>

#include <main.h>

const GpioPin gpio_led_pin = {.port = LED_GPIO_Port, .pin = LED_Pin};
const GpioPin gpio_bus_pwr_pin = {.port = BUS_PWR_GPIO_Port, .pin = BUS_PWR_Pin};

const GpioPin gpio_addr_bus_dir_pin = {.port = ADDR_BUS_DIR_GPIO_Port, .pin = ADDR_BUS_DIR_Pin};
const GpioPin gpio_data_bus_dir_pin = {.port = DATA_BUS_DIR_GPIO_Port, .pin = DATA_BUS_DIR_Pin};
const GpioPin gpio_ctrl_bus_dir_pin = {.port = CTRL_BUS_DIR_GPIO_Port, .pin = CTRL_BUS_DIR_Pin};
const GpioPin gpio_bus_oe_pin = {.port = BUS_OE_GPIO_Port, .pin = BUS_OE_Pin};

const GpioPin gpio_wr_pin = {.port = _WR_GPIO_Port, .pin = _WR_Pin};
const GpioPin gpio_rd_pin = {.port = _RD_GPIO_Port, .pin = _RD_Pin};
const GpioPin gpio_iorq_pin = {.port = _IORQ_GPIO_Port, .pin = _IORQ_Pin};
const GpioPin gpio_memrq_pin = {.port = _MEMRQ_GPIO_Port, .pin = _MEMRQ_Pin};
const GpioPin gpio_m1_pin = {.port = _M1_GPIO_Port, .pin = _M1_Pin};

const GpioPin gpio_busreq_pin = {.port = _BUSREQ_GPIO_Port, .pin = _BUSREQ_Pin};
const GpioPin gpio_busack_pin = {.port = _BUSACK_GPIO_Port, .pin = _BUSACK_Pin};
const GpioPin gpio_cpurst_pin = {.port = _CPURST_GPIO_Port, .pin = _CPURST_Pin};
const GpioPin gpio_wait_pin = {.port = _WAIT_GPIO_Port, .pin = _WAIT_Pin};
const GpioPin gpio_nmi_pin = {.port = _NMI_GPIO_Port, .pin = _NMI_Pin};
const GpioPin gpio_int_pin = {.port = _INT_GPIO_Port, .pin = _INT_Pin};
const GpioPin gpio_cpu_clk_pin = {.port = _CPU_CLK_GPIO_Port, .pin = _CPU_CLK_Pin};

const GpioPin gpio_addr_pins[BUS_ADDR_WIDTH] = {
    {.port = A0_GPIO_Port, .pin = A0_Pin},
    {.port = A1_GPIO_Port, .pin = A1_Pin},
    {.port = A2_GPIO_Port, .pin = A2_Pin},
    {.port = A3_GPIO_Port, .pin = A3_Pin},
    {.port = A4_GPIO_Port, .pin = A4_Pin},
    {.port = A5_GPIO_Port, .pin = A5_Pin},
    {.port = A6_GPIO_Port, .pin = A6_Pin},
    {.port = A7_GPIO_Port, .pin = A7_Pin},
    {.port = A8_GPIO_Port, .pin = A8_Pin},
    {.port = A9_GPIO_Port, .pin = A9_Pin},
    {.port = A10_GPIO_Port, .pin = A10_Pin},
    {.port = A11_GPIO_Port, .pin = A11_Pin},
    {.port = A12_GPIO_Port, .pin = A12_Pin},
    {.port = A13_GPIO_Port, .pin = A13_Pin},
    {.port = A14_GPIO_Port, .pin = A14_Pin},
    {.port = A15_GPIO_Port, .pin = A15_Pin},
};

const GpioPin gpio_data_pins[BUS_DATA_WIDTH] = {
    {.port = D0_GPIO_Port, .pin = D0_Pin},
    {.port = D1_GPIO_Port, .pin = D1_Pin},
    {.port = D2_GPIO_Port, .pin = D2_Pin},
    {.port = D3_GPIO_Port, .pin = D3_Pin},
    {.port = D4_GPIO_Port, .pin = D4_Pin},
    {.port = D5_GPIO_Port, .pin = D5_Pin},
    {.port = D6_GPIO_Port, .pin = D6_Pin},
    {.port = D7_GPIO_Port, .pin = D7_Pin},
};
