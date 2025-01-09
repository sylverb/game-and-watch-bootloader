#ifndef _GW_SDCARD_H_
#define _GW_SDCARD_H_

#include "stm32h7xx_hal.h"

extern bool fs_mounted;

void sdcard_init(void);

void sdcard_init_spi1();
void sdcard_deinit_spi1();
void sdcard_init_ospi1();
void sdcard_deinit_ospi1();
void switch_ospi_gpio(uint8_t ToOspi);

#endif
