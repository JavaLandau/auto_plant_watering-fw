#ifndef CH552_TIMER_H
#define CH552_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#include "ch552.h"

#define CH552_TIMER0         0
#define CH552_TIMER1         1
#define CH552_TIMER2         2
#define CH552_TIMER_COUNT    3

struct ch552_timer_config {
    uint32_t period;
    void (*timer_isr_cb)(uint8_t timer_num, void *params);
    void *params;
};

uint8_t ch552_timer_init(uint8_t timer_num, struct ch552_timer_config *config);
uint8_t ch552_timer_uart0_init(uint8_t timer_num, uint32_t baudrate);

uint8_t ch552_timer_start(uint8_t timer_num);
uint8_t ch552_timer_stop(uint8_t timer_num);

//Interrupts should be provided for main.c to include them to interrupt vector
void TMR0_ISR(void) __interrupt(INT_NO_TMR0);
void TMR1_ISR(void) __interrupt(INT_NO_TMR1);


#endif //CH552_TIMER_H