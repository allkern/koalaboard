#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"

#define UART_RHR        0x00
#define UART_THR        0x00
#define UART_IER        0x01
#define UART_IIR        0x02
#define UART_FCR        0x02
#define UART_ISR        0x02
#define UART_LCR        0x03
#define UART_LSR        0x05
#define IER_RX_IRQ      0x01
#define IER_TX_IRQ      0x02
#define FCR_FIFO_ENABLE 0x01
#define FCR_FIFO_CLEAR  0x06
#define LCR_WORD_LEN    0x03
#define LCR_DLAB        0x80
#define LSR_RX_READY    0x01
#define LSR_TX_EMPTY    0x20

typedef void (*uart_tx_event_t)(uint8_t);

typedef struct {
    uart_tx_event_t tx_event;
    uint8_t rhr;
    uint8_t thr;
    uint8_t ier;
    uint8_t iir;
    uint8_t fcr;
    uint8_t lcr;
    uint8_t lsr;
} uart_t;

uart_t* uart_create();
void uart_init(uart_t*, uart_tx_event_t);
void uart_send_byte(uart_t*, uint8_t);
int uart_query_access_cycles(void*);
uint32_t uart_read32(uint32_t, void*);
uint32_t uart_read16(uint32_t, void*);
uint32_t uart_read8(uint32_t, void*);
void uart_write32(uint32_t, uint32_t, void*);
void uart_write16(uint32_t, uint32_t, void*);
void uart_write8(uint32_t, uint32_t, void*);
void uart_destroy(uart_t*);
void uart_init_bus_device(uart_t*, bus_device_t*);

#endif