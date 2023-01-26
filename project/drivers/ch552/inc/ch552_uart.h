#ifndef CH552_UART_H
#define CH552_UART_H

#include <stdint.h>
#include <stdbool.h>

#define CH552_UART0         0
#define CH552_UART1         1
#define CH552_UART_COUNT    2

#define CH552_UART_PARITY_EVEN      0
#define CH552_UART_PARITY_ODD       1
#define CH552_UART_PARITY_NONE      2

struct ch552_uart_config {
    uint32_t baudrate;
    bool is_9bit_wordlen;
    uint8_t parity;
    bool rx_enabled;
};

uint8_t ch552_uart_init(uint8_t uart_num, struct ch552_uart_config *config);
uint8_t ch552_uart_send(uint8_t uart_num, void *buffer, uint32_t len);

#endif //CH552_UART_H