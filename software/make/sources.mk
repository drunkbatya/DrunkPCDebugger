C_SOURCES := \
	src/targets/$(HW_TARGET)/stm/Core/Src/main.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/stm32h7xx_it.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/stm32h7xx_hal_msp.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_exti.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/system_stm32h7xx.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/sysmem.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/syscalls.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c \
	src/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/gpio.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/tim.c \
	src/targets/$(HW_TARGET)/stm/Core/Src/usb_otg.c


C_SOURCES += $(shell find src \
	-type d -name '.*' -prune -o \
	-path 'src/lib/nanopb' -prune -o \
	-path "src/targets/$(HW_TARGET)/stm" -prune -o \
	-type f -name '*.c' -print)

ASM_SOURCES = src/targets/$(HW_TARGET)/stm/startup_stm32h723xx.s

C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_OBJECTS := $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
OBJECT_DIRS := $(sort $(dir $(OBJECTS)))

LDSCRIPT := $(shell find "src/targets/$(HW_TARGET)" -type f -name '*flash.ld')
