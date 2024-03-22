#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"

typedef struct {
    uint64_t latched_time;
    int write_protect;
} rtc_t;

#define RTC_TIMEL 0
#define RTC_TIMEH 4
#define RTC_CTRL  8

rtc_t* rtc_create();
void rtc_init(rtc_t*);
int rtc_query_access_cycles(void*);
uint32_t rtc_read32(uint32_t, void*);
uint32_t rtc_read16(uint32_t, void*);
uint32_t rtc_read8(uint32_t, void*);
void rtc_write32(uint32_t, uint32_t, void*);
void rtc_write16(uint32_t, uint32_t, void*);
void rtc_write8(uint32_t, uint32_t, void*);
void rtc_destroy(rtc_t*);
void rtc_init_bus_device(rtc_t*, bus_device_t*);

#endif