#include "rtc.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

rtc_t* rtc_create() {
    return (rtc_t*)malloc(sizeof(rtc_t));
}

void rtc_init(rtc_t* rtc) {
    memset(rtc, 0, sizeof(rtc_t));
}

int rtc_query_access_cycles(void* udata) {
    return 0;
}

uint32_t rtc_read32(uint32_t addr, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;

    uint64_t data;

    if (!rtc->latched_time) {
        rtc->latched_time = time(NULL);

        data = rtc->latched_time;
    } else {
        data = rtc->latched_time;

        rtc->latched_time = 0;
    }

    if (addr == RTC_TIMEL) {
        return data & 0xffffffff;
    } else if (addr == RTC_TIMEH) {
        return data >> 32;
    }

    return rtc->write_protect;
}

uint32_t rtc_read16(uint32_t addr, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;

    return 0xf00d;
}

uint32_t rtc_read8(uint32_t addr, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;

    return 0x0d;
}

void rtc_write32(uint32_t addr, uint32_t data, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;

    if (addr == RTC_CTRL) {
        rtc->write_protect = data & 1;

        return;
    }

    rtc->latched_time = 0;
}

void rtc_write16(uint32_t addr, uint32_t data, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;
}

void rtc_write8(uint32_t addr, uint32_t data, void* udata) {
    rtc_t* rtc = (rtc_t*)udata;
}

void rtc_destroy(rtc_t* rtc) {
    free(rtc);
}

void rtc_init_bus_device(rtc_t* rtc, bus_device_t* dev) {
    dev->query_access_cycles = rtc_query_access_cycles;
    dev->read32 = rtc_read32;
    dev->read16 = rtc_read16;
    dev->read8 = rtc_read8;
    dev->write32 = rtc_write32;
    dev->write16 = rtc_write16;
    dev->write8 = rtc_write8;
    dev->udata = rtc;
}