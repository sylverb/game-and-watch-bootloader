TARGET = gw_bootloader

DEBUG = 0

OPT = -O2 -ggdb3

# To enable verbose, append VERBOSE=1 to make, e.g.:
# make VERBOSE=1
ifneq ($(strip $(VERBOSE)),1)
V = @
endif

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Src/gw_buttons.c \
Core/Src/gw_sdcard.c \
Core/Src/gw_timer.c \
Core/Src/porting/gw_alloc.c \
Core/Src/main.c \
Core/Src/bootloader.c \
Core/Src/syscalls.c \
Core/Src/bq24072.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/system_stm32h7xx.c \
Core/src/softspi.c


#Core/Src/porting/common.c \

FATFS_C_SOURCES = \
Core/Src/porting/lib/FatFs/user_diskio.c \
Core/Src/porting/lib/FatFs/ff.c \
Core/Src/porting/lib/FatFs/ffsystem.c \
Core/Src/porting/lib/FatFs/ffunicode.c \
Core/Src/porting/lib/FatFs/user_diskio_spi.c \
Core/Src/porting/lib/FatFs/user_diskio_softspi.c

C_INCLUDES +=  \
-ICore/Inc \
-ICore/Src/porting/lib \
-ICore/Src/porting/lib/lzma \
-ICore/Src/porting/lib/FatFs \
-Iretro-go-stm32/components/odroid \
-I./

FATFS_INCLUDES += \
-ICore/Src/porting/lib/FatFs

include Makefile.common

$(BUILD_DIR)/$(TARGET)_intflash.bin: $(BUILD_DIR)/$(TARGET).elf | $(BUILD_DIR)
	$(V)$(ECHO) [ BIN ] $(notdir $@)
	$(V)$(BIN) -j .isr_vector -j .text -j .rodata -j .ARM.extab -j .preinit_array -j .init_array -j .fini_array -j .data $< $(BUILD_DIR)/$(TARGET)_intflash.bin
