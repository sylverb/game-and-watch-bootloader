##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [4.1.0] date: [Wed Sep 06 16:34:37 PDT 2023]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#	2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = bootloader

######################################
# gnwmanager
######################################
GNWMANAGER = gnwmanager

######################################
# building variables
######################################
ECHO  = echo

# debug build?
DEBUG ?= 0

INTFLASH_ADDRESS = 0x08000000

# optimization
ifeq ($(DEBUG), 1)
OPT ?= -Og
else
OPT ?= -Os
endif


ifeq ($(OS),Windows_NT)
CP = copy
RM = del /Q /F
PATH_SEP = \\
FIX_PATH = $(subst /,\\,$1)
else
CP = cp
RM = rm -rf
PATH_SEP = /
FIX_PATH = $1
endif

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################

# Version and URL for the STM32CubeH7 SDK
SDK_VERSION ?= v1.11.1
SDK_URL ?= https://raw.githubusercontent.com/STMicroelectronics/STM32CubeH7

# Local path for the SDK
SDK_DIR ?= Drivers

# SDK C sources
SDK_C_SOURCES =  \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c

# SDK ASM sources
SDK_ASM_SOURCES =  \
Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b0xx.s

# SDK headers
SDK_HEADERS = \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7b0xx.h \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7xx.h \
Drivers/CMSIS/Device/ST/STM32H7xx/Include/system_stm32h7xx.h \
Drivers/CMSIS/Include/cmsis_compiler.h \
Drivers/CMSIS/Include/cmsis_gcc.h \
Drivers/CMSIS/Include/cmsis_version.h \
Drivers/CMSIS/Include/core_cm7.h \
Drivers/CMSIS/Include/mpu_armv7.h \
Drivers/STM32H7xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_ll_adc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_adc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_adc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_cortex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dac_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dac.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_def.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dma_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dma.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_dma2d.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_exti.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_flash_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_flash.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_gpio_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_gpio.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_hash.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_hash_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_hsem.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_i2c_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_i2c.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_jpeg.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ltdc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ltdc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_mdma.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_ospi.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_pwr_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_pwr.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rcc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rcc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rtc_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_rtc.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_sai_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_sai.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_spi.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_tim.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_tim_ex.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_wwdg.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal.h \
Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_iwdg.h \

# C sources
C_SOURCES =  \
Core/Src/FatFs/ff.c \
Core/Src/FatFs/ffunicode.c \
Core/Src/FatFs/user_diskio_spi.c \
Core/Src/FatFs/user_diskio_softspi.c \
Core/Src/FatFs/user_diskio.c \
Core/Src/flash.c \
Core/Src/lcd.c \
Core/Src/buttons.c \
Core/Src/main.c \
Core/Src/bootloader.c \
Core/Src/overlay.c \
Core/Src/rg_rtc.c \
Core/Src/sdcard.c \
Core/Src/softspi.c \
Core/Src/stm32h7xx_it.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/system_stm32h7xx.c \
Core/Src/timer.c \
$(SDK_C_SOURCES) \



# ASM sources
ASM_SOURCES = $(SDK_ASM_SOURCES)

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
OBJCOPY = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S --verbose

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32H7B0xx

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-ICore/Inc \
-ICore/Src/lzma \
-ICore/Src/FatFs \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/CMSIS/Include


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wfatal-errors -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2 -O0
else
CFLAGS += -Werror -flto
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32H7B0VBTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

ifeq ($(DEBUG), 1)
else
LDFLAGS += -flto
endif
# default action: build all
all: $(BUILD_DIR)/$(TARGET).bin

#######################################
# Flashing
#######################################

ADAPTER ?= stlink

reset:
	@$(GNWMANAGER) start bank1
.PHONY: reset

start_bank_1:
	$(GNWMANAGER) start bank1
.PHONY: start_bank_1

start:  # Start code located at INTFLASH_ADDRESS
	$(GNWMANAGER) start $(INTFLASH_ADDRESS)
.PHONY: start

flash: $(BUILD_DIR)/$(TARGET).bin download_sdk
	$(GNWMANAGER) flash $(INTFLASH_ADDRESS) $< -- start $(INTFLASH_ADDRESS)

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile $(SDK_HEADERS) | $(BUILD_DIR)
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile $(SDK_HEADERS) | $(BUILD_DIR)
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	@$(V)$(BIN) \
		-j .isr_vector \
		-j .text \
		-j .rodata \
		-j .ARM.extab \
		-j .preinit_array \
		-j .init_array \
		-j .fini_array \
		-j .data \
		$< $@

$(BUILD_DIR):
	@mkdir $@

GDB ?= $(PREFIX)gdb

gdb: $(BUILD_DIR)/$(TARGET).elf
	$(GDB) $< -ex "target extended-remote :3333"
.PHONY: debug

#######################################
# download SDK files
#######################################
$(SDK_DIR)/%:
	@$(ECHO) [ WGET ] $(notdir $@)
	@wget -q $(SDK_URL)/$(SDK_VERSION)/$@ -P $(dir $@)

.PHONY: download_sdk
download_sdk: $(SDK_HEADERS) $(SDK_C_SOURCES) $(SDK_ASM_SOURCES)

#######################################
# clean up
#######################################
clean:
	@$(RM) $(BUILD_DIR)
#	@$(RM) -rf $(SDK_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***