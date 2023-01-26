#include "ch552_timer.h"
#include "ch552_sys.h"
#include "ch552.h"
#include "common.h"

static struct ch552_timer_config configs[CH552_TIMER_COUNT];

//Timer2 is not supported yet
uint8_t ch552_timer_init(uint8_t timer_num, struct ch552_timer_config *config)
{
    uint32_t period = ((ch552_sys_clk_get() / 1000) * config->period) / 12;
    period = 0xFFFF - period;

    //Work in 16-bit mode (mode 1)
    //If timer was started it's stopped
    if (timer_num == CH552_TIMER0) {
        TR0 = 0;
        TMOD = (TMOD & ~bT0_M1 & ~bT0_GATE & ~bT0_CT) | bT0_M0;
        T2MOD &= ~bT0_CLK;
        TL0 = (uint8_t)(period & 0xFF);
        TH0 = (uint8_t)((period >> 8) & 0xFF);
        TF0 = 0;
        ET0 = 1;
    } else {
        TR1 = 0;
        TMOD = (TMOD & ~bT1_M1 & ~bT1_GATE & ~bT1_CT) | bT1_M0;
        T2MOD &= ~bT1_CLK;
        TL1 = (uint8_t)(period & 0xFF);
        TH1 = (uint8_t)((period >> 8) & 0xFF);
        TF1 = 0;
        ET1 = 1;
    }

    configs[timer_num] = *config;
    configs[timer_num].period = period;

    return RES_OK;
}

//Support only Timer1
uint8_t ch552_timer_uart0_init(uint8_t timer_num, uint32_t baudrate)
{
    uint32_t uart_div = (PCON & SMOD) ? 16 : 32;
    uint32_t period = (10 * ch552_sys_clk_get()) / (uart_div * baudrate);
    period = (period / 10) + (((period % 10) >= 5) ? 1 : 0);

    //Work in 8-bit mode (mode 2)
    if (timer_num == CH552_TIMER1) {
        TR1 = 0;
        RCLK = 0;
        TCLK = 0;
        TMOD = (TMOD & ~bT1_M0 & ~bT1_GATE & ~bT1_CT) | bT1_M1;
        T2MOD |= bTMR_CLK | bT1_CLK;
        TH1 = 0 - period;
    }

    return RES_OK;
}

uint8_t ch552_timer_start(uint8_t timer_num)
{
    if (timer_num == CH552_TIMER0)
        TR0 = 1;
    else if (timer_num == CH552_TIMER1)
        TR1 = 1;
    else if (timer_num == CH552_TIMER2)
        TR2 = 1;
    else
        return RES_INVALID_PAR;

    return RES_OK;
}

uint8_t ch552_timer_stop(uint8_t timer_num)
{
    if (timer_num == CH552_TIMER0)
        TR0 = 0;
    else if (timer_num == CH552_TIMER1)
        TR1 = 0;
    else if (timer_num == CH552_TIMER2)
        TR2 = 0;
    else
        return RES_INVALID_PAR;

    return RES_OK;
}

void TMR0_ISR(void) __interrupt(INT_NO_TMR0)
{
    if (configs[CH552_TIMER0].timer_isr_cb)
        configs[CH552_TIMER0].timer_isr_cb(CH552_TIMER0, configs[CH552_TIMER0].params);

    TL0 = (uint8_t)(configs[CH552_TIMER0].period & 0xFF);
    TH0 = (uint8_t)((configs[CH552_TIMER0].period >> 8) & 0xFF);
}

void TMR1_ISR(void) __interrupt(INT_NO_TMR1)
{
    if (configs[CH552_TIMER1].timer_isr_cb)
        configs[CH552_TIMER1].timer_isr_cb(CH552_TIMER1, configs[CH552_TIMER1].params);

    TL1 = (uint8_t)(configs[CH552_TIMER1].period & 0xFF);
    TH1 = (uint8_t)((configs[CH552_TIMER1].period >> 8) & 0xFF);
}
