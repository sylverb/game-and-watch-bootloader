#ifndef _GW_TIMER_H_
#define _GW_TIMER_H_
#include <stdint.h>

void gw_timer_on(uint8_t timer_index,uint32_t waitTicks);
uint8_t gw_timer_status(uint8_t timer_index);
#endif