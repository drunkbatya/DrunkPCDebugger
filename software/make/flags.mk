CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard
MCU := $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

C_DEFS := \
	-DUSE_FULL_LL_DRIVER \
	-DUSE_PWR_LDO_SUPPLY \
	-DUSE_HAL_DRIVER \
	-DSTM32H723xx

C_DEFS += \
	'-DGIT_COMMIT="$(GIT_COMMIT)"' \
	'-DGIT_BRANCH="$(GIT_BRANCH)"' \
	'-DGIT_BRANCH_NUM="$(GIT_BRANCH_NUM)"' \
	'-DBUILD_DATE="$(BUILD_DATE)"' \
	'-DVERSION="$(VERSION)"' \
	'-DBUILD_DIRTY=$(GIT_DIRTY)' \
	'-DFIRMWARE_ORIGIN="DrunkDashboard"' \
	'-DGIT_ORIGIN="$(GIT_ORIGIN)"' \
	-DTARGET=$(HW_TARGET_ID)

ifeq ($(DEBUG),1)
C_DEFS += -DDEBUG -DFURI_DEBUG -DFURI_HAL_DEBUG
endif

C_INCLUDES := \
	-I$(PROTO_OUT_DIR) \
	-Isrc/lib \
	-Isrc/lib/nanopb \
	-Isrc \
	-Isrc/applications/services \
	-Isrc/targets/$(HW_TARGET)/stm/Core/Inc \
	-Isrc/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Inc \
	-Isrc/targets/$(HW_TARGET)/stm/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
	-Isrc/targets/$(HW_TARGET)/stm/Drivers/CMSIS/Device/ST/STM32H7xx/Include \
	-Isrc/targets/$(HW_TARGET)/stm/Drivers/CMSIS/Include \
	-Isrc/targets/$(HW_TARGET)/stm/Drivers/CMSIS/Include \
	-Isrc/targets/$(HW_TARGET)/stm/USB_DEVICE/App \
	-Isrc/targets/$(HW_TARGET)/stm/USB_DEVICE/Target \
	-Isrc/targets/$(HW_TARGET)/stm/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
	-Isrc/targets/$(HW_TARGET)/stm/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Werror -fdata-sections -ffunction-sections

ifeq ($(DEBUG),1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

LIBS := -lc -lm -lnosys
LIBDIR :=
LDFLAGS := $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) \
	-Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
	-Wl,--gc-sections
LDFLAGS += -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_calloc_r -Wl,--wrap,_realloc_r
