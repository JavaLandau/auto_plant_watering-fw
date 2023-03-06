#include <stdio.h>

#include "common.h"
#include "ch552.h"
#include "ch552_adc.h"
#include "ch552_gpio.h"
#include "ch552_sys.h"
#include "ch552_uart.h"
#include "ch552_timer.h"

#define FW_VERSION          "1.0-RC1"

#define FSYS                6000000

#define LED_PIN             GPIO_PIN_7
#define PUMP_PIN            GPIO_PIN_5

#define MIN_TO_SEC(D)       ((D) * 60UL)
#define HOURS_TO_SEC(D)     ((D) * 3600UL)
#define DAY_TO_SEC(D)       ((D) * 86400UL)

#define SEC_TO_MIN(D)       ((D) / 60UL)
#define SEC_TO_HOURS(D)     ((D) / 3600UL)
#define SEC_TO_DAY(D)       ((D) / 86400UL)

#define VCC_IN_DV           50

#define PUMP_MAX_VAL_SEC     MIN_TO_SEC(1)
#define MIN_PUMP_VAL_DV      0
#define MAX_PUMP_VAL_DV      VCC_IN_DV

#define DELAY_MAX_VAL_SEC     DAY_TO_SEC(1)
#define MIN_DELAY_VAL_DV      0
#define MAX_DELAY_VAL_DV      VCC_IN_DV

#define POLL_PERIOD_MS        100
#define TRACE_PERIOD_MS       1000

static uint8_t cnt_100ms = 0;
static uint32_t cnt_sec = 0;

static void timer_callback(uint8_t timer_num, void *params)
{
    (void)timer_num;
    (void)params;

    cnt_100ms++;

    if (cnt_100ms == 10) {
        cnt_sec++;
        cnt_100ms = 0;
    }
}

int putchar(int c)
{
    ch552_uart_send(CH552_UART0, (void*)&c, 1);
    return c;
}

static void sec_format_printf(const char *label, uint32_t sec_count)
{
    uint8_t day = 0, hours = 0, min = 0, sec = 0;

    day = SEC_TO_DAY(sec_count);
    hours = SEC_TO_HOURS(sec_count - DAY_TO_SEC(day));
    min = SEC_TO_MIN(sec_count - DAY_TO_SEC(day) - HOURS_TO_SEC(hours));
    sec = sec_count - DAY_TO_SEC(day) - HOURS_TO_SEC(hours) - MIN_TO_SEC(min);
    printf_tiny("%s %d-%d%d:%d%d:%d%d\r\n", label, day, hours / 10, hours % 10, min / 10, min % 10, sec / 10, sec % 10);
}

static void main(void) {
    //MCU Initialization
    CH552_SYS_INT_INIT();
    ch552_sys_clk_set(FSYS);

    //LED & PUMP pins
    ch552_gpio_set(GPIO_PORT_P1, LED_PIN, 1);
    ch552_gpio_set(GPIO_PORT_P1, PUMP_PIN, 0);

    ch552_gpio_init(GPIO_PORT_P1, LED_PIN, CH552_GPIO_PP_OUTPUT);
    ch552_gpio_init(GPIO_PORT_P1, PUMP_PIN, CH552_GPIO_PP_OUTPUT);

    //ADC initialization
    ch552_gpio_init(GPIO_PORT_P3, GPIO_PIN_2, CH552_GPIO_INPUT);
    ch552_gpio_init(GPIO_PORT_P1, GPIO_PIN_4, CH552_GPIO_INPUT);
    ch552_adc_init();

    //UART initialization
    struct ch552_uart_config uart_config = {.baudrate = 9600,
                                            .is_9bit_wordlen = false,
                                            .parity = CH552_UART_PARITY_NONE,
                                            .rx_enabled = false};
    ch552_uart_init(CH552_UART0, &uart_config);

    //Timer initialization
    struct ch552_timer_config timer_config = {.period = 100,
                                              .timer_isr_cb = timer_callback,
                                              .params = NULL};

    ch552_timer_init(CH552_TIMER0, &timer_config);
    ch552_timer_start(CH552_TIMER0);

    uint8_t adc_result = 0;
    uint8_t trace_cnt = 0;
    uint32_t delay_time = 0;
    uint32_t pump_time = 0;
    bool pump_enabled = false;

    //Startup display
    ch552_sys_delay_ms(500);

    printf_tiny("============================\r\n");

    printf_tiny("\r\n");
    printf_tiny("Autowatering VER. %s\r\n", FW_VERSION);
    printf_tiny("\r\n");

    printf_tiny("DELAY RANGES [%d %d]\r\n", MIN_DELAY_VAL_DV, MAX_DELAY_VAL_DV);
    printf_tiny("PUMP RANGES [%d %d]\r\n", MIN_PUMP_VAL_DV, MAX_PUMP_VAL_DV);
    printf_tiny("\r\n");

    sec_format_printf("MAX DELAY", DELAY_MAX_VAL_SEC);
    sec_format_printf("MAX PUMP", PUMP_MAX_VAL_SEC);
    printf_tiny("\r\n");

    printf_tiny("============================\r\n");
    printf_tiny("\r\n");

    //LED startup
    uint8_t led_state = 1;
    for (uint8_t i = 0; i < 4; i++) {
        led_state = 1 - led_state;
        ch552_gpio_set(GPIO_PORT_P1, LED_PIN, led_state);
        ch552_sys_delay_ms(250);
    }

    while (1) {
        //Get delay time
        ch552_adc_conv(CH552_ADC_CH3, &adc_result);
        uint32_t val_dv = (VCC_IN_DV * adc_result) / 255;

        if (val_dv <= MIN_DELAY_VAL_DV)
            val_dv = MIN_DELAY_VAL_DV;

        if (val_dv >= MAX_DELAY_VAL_DV)
            val_dv = MAX_DELAY_VAL_DV;

        delay_time = (DELAY_MAX_VAL_SEC * (val_dv - MIN_DELAY_VAL_DV)) / (MAX_DELAY_VAL_DV - MIN_DELAY_VAL_DV);
        uint32_t val_dv1 = val_dv;

        //Get pump duration
        ch552_adc_conv(CH552_ADC_CH1, &adc_result);
        val_dv = (VCC_IN_DV * adc_result) / 255;

        if (val_dv <= MIN_PUMP_VAL_DV)
            val_dv = MIN_PUMP_VAL_DV;

        if (val_dv >= MAX_PUMP_VAL_DV)
            val_dv = MAX_PUMP_VAL_DV;

        pump_time = (PUMP_MAX_VAL_SEC * (val_dv - MIN_PUMP_VAL_DV)) / (MAX_PUMP_VAL_DV - MIN_PUMP_VAL_DV);

        //Pump logic
        uint32_t expired_time = pump_enabled ? pump_time : delay_time;
        if (expired_time <= cnt_sec) {
            CH552_INTERRUPTS_DISABLE();
            cnt_sec = 0;
            cnt_100ms = 0;
            CH552_INTERRUPTS_ENABLE();

            pump_enabled = !pump_enabled;
            ch552_gpio_set(GPIO_PORT_P1, PUMP_PIN, pump_enabled);
        }

        if (++trace_cnt >= (TRACE_PERIOD_MS / POLL_PERIOD_MS)) {
            printf_tiny("ADC %d/%d\r\n", (uint16_t)val_dv1, (uint16_t)val_dv);
            sec_format_printf(pump_enabled ? "PUMP" : "DELAY", cnt_sec);
            printf_tiny("\r\n");

            trace_cnt = 0;
        }

        ch552_sys_delay_ms(POLL_PERIOD_MS);
    }
}