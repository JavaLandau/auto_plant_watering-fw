#include "ch552_uart.h"
#include "ch552_timer.h"
#include "ch552.h"
#include "common.h"

static struct ch552_uart_config configs[CH552_UART_COUNT];

uint8_t ch552_uart_init(uint8_t uart_num, struct ch552_uart_config *config)
{
    //Support UART0 only
    if (uart_num != CH552_UART0)
        return RES_INVALID_PAR;

    uint8_t res = ch552_timer_uart0_init(CH552_TIMER1, config->baudrate);

    if (res != RES_OK)
        return res;

    res = ch552_timer_start(CH552_TIMER1);

    if (res != RES_OK)
        return res;

    //UART configuration
    SM0 = config->is_9bit_wordlen ? 1 : 0;
    SM1 = 1;
    SM2 = 0;
    REN = config->rx_enabled ? 1 : 0;

    //To enable UART0 TX PIN
    TI = 1;

    //Reset all pending interrupts
    RI = 0;

    configs[uart_num] = *config;

    return RES_OK;
}

uint8_t ch552_uart_send(uint8_t uart_num, void *buffer, uint32_t len)
{
    TI = 0;
    if (!SM0 || (SM0 && configs[uart_num].parity != CH552_UART_PARITY_NONE)) {
        uint8_t *data = (uint8_t*)buffer;
        for (uint32_t i = 0; i < len; i++) {
            SBUF = data[i];

            // Generate parity bit
            if (configs[uart_num].parity != CH552_UART_PARITY_NONE) {
                data[i] += 0;
                uint8_t parity_bit = P;
                TB8 = (configs[uart_num].parity == CH552_UART_PARITY_EVEN) ? parity_bit : !parity_bit;
            }

            while(!TI);
            TI = 0;
        }
    } else {
        uint16_t *data = (uint16_t*)buffer;
        for (uint32_t i = 0; i < len; i++) {
            SBUF = (uint8_t)(data[i] & 0xFF);
            TB8 = (data[i] >> 8) & 0x01;

            while(!TI);
            TI = 0;
        }
    }

    return RES_OK;
}