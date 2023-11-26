#include "uart.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uint8_t handle_rhr_read(uart_t* uart) {
    uint8_t rhr = uart->rhr;

    fflush(stdout);
    fflush(stdin);
    
    uart->lsr &= ~LSR_RX_READY;
    uart->rhr = -1;

    return rhr;
}

void handle_thr_write(uart_t* uart, uint8_t data) {
    if (uart->lcr & LCR_DLAB)
        return;

    uart->thr = 0;
    uart->lsr |= LSR_TX_EMPTY;

    if (uart->tx_event)
        uart->tx_event(data);
}

uart_t* uart_create() {
    return (uart_t*)malloc(sizeof(uart_t));
}

void uart_init(uart_t* uart, uart_tx_event_t tx_event) {
    memset(uart, 0, sizeof(uart_t));

    uart->lsr = LSR_TX_EMPTY;
    uart->tx_event = tx_event;
}

int uart_query_access_cycles(void* udata) {
    return 0;
}

void uart_send_byte(uart_t* uart, uint8_t data) {
    uart->rhr = data;
    uart->lsr |= LSR_RX_READY;

    // To-do: Implement IRQs
}

uint32_t uart_read32(uint32_t addr, void* udata) {
    uart_t* uart = (uart_t*)udata;

    return 0xbaadf00d;
}

uint32_t uart_read16(uint32_t addr, void* udata) {
    uart_t* uart = (uart_t*)udata;

    return 0xf00d;
}

uint32_t uart_read8(uint32_t addr, void* udata) {
    uart_t* uart = (uart_t*)udata;

    switch (addr) {
        case UART_RHR: return handle_rhr_read(uart);
        case UART_IER: return uart->ier;
        case UART_IIR: return uart->iir;
        case UART_LCR: return uart->lcr;
        case UART_LSR: return uart->lsr;
    }

    return 0;
}

void uart_write32(uint32_t addr, uint32_t data, void* udata) {
    uart_t* uart = (uart_t*)udata;

    return;
}

void uart_write16(uint32_t addr, uint32_t data, void* udata) {
    uart_t* uart = (uart_t*)udata;

    return;
}

void uart_write8(uint32_t addr, uint32_t data, void* udata) {
    uart_t* uart = (uart_t*)udata;

    switch (addr) {
        case UART_THR: handle_thr_write(uart, data); break;
        case UART_IER: if (!(uart->lcr & LCR_DLAB)) uart->ier = data; break;
        case UART_FCR: uart->fcr = data; break;
        case UART_LCR: uart->lcr = data; break;
    }
}

void uart_destroy(uart_t* uart) {
    free(uart);
}

void uart_init_bus_device(uart_t* uart, bus_device_t* dev) {
    dev->query_access_cycles = uart_query_access_cycles;
    dev->read32 = uart_read32;
    dev->read16 = uart_read16;
    dev->read8 = uart_read8;
    dev->write32 = uart_write32;
    dev->write16 = uart_write16;
    dev->write8 = uart_write8;
    dev->udata = uart;
}