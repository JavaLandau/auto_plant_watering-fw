#include <stdio.h>

#include "common.h"
#include "ch552.h"
#include "ch552_adc.h"
#include "ch552_gpio.h"
#include "ch552_sys.h"
#include "ch552_uart.h"
#include "ch552_timer.h"
#include "ch552_pwm.h"
#include "ch552_dataflash.h"

#define FW_VERSION          "1.1"

#define FSYS                6000000

#define LED_PIN             GPIO_PIN_7
#define PUMP_PIN            GPIO_PIN_5

#define DATAFLASH_CTX_SIZE  3

#define MIN_TO_SEC(D)       ((D) * 60UL)
#define HOURS_TO_SEC(D)     ((D) * 3600UL)
#define DAY_TO_SEC(D)       ((D) * 86400UL)

#define SEC_TO_MIN(D)       ((D) / 60UL)
#define SEC_TO_HOURS(D)     ((D) / 3600UL)
#define SEC_TO_DAY(D)       ((D) / 86400UL)

#define VCC_IN_DV           50UL

#define PUMP_MAX_VAL_SEC     MIN_TO_SEC(5)
#define MIN_PUMP_VAL_DV      0
#define MAX_PUMP_VAL_DV      VCC_IN_DV

#define DELAY_MAX_VAL_SEC     DAY_TO_SEC(7)
#define MIN_DELAY_VAL_DV      0
#define MAX_DELAY_VAL_DV      VCC_IN_DV
#define DELAY_STEP            DAY_TO_SEC(1)

#define PUMP_PWM_DEFAULT      255

#define POLL_PERIOD_MS        100
#define TRACE_PERIOD_MS       1000

static uint8_t cnt_100ms = 0;
static uint32_t cnt_sec = 0;

#define TIMER_RESET()       do {\
                                CH552_INTERRUPTS_DISABLE();\
                                cnt_sec = 0;\
                                cnt_100ms = 0;\
                                CH552_INTERRUPTS_ENABLE();\
                            } while(0)

static uint8_t dataflash_buff[DATAFLASH_CTX_SIZE] = {0};

static void pump_pwm_store(uint8_t pwm_data)
{
    dataflash_buff[0] = 0x55;
    dataflash_buff[1] = 0xAA;
    dataflash_buff[2] = pwm_data;
    ch552_dataflash_write(0, dataflash_buff, DATAFLASH_CTX_SIZE);
    printf_tiny("STORE PWM %d\r\n", pwm_data);
}

static uint8_t pump_pwm_restore(void)
{
    ch552_dataflash_read(0, dataflash_buff, DATAFLASH_CTX_SIZE);

    uint8_t pump_pwm_data = dataflash_buff[2];

    if (dataflash_buff[0] != 0x55 || dataflash_buff[1] != 0xAA) {
        printf_tiny("INVALID DATAFLASH [%d %d %d]\r\n", dataflash_buff[0], dataflash_buff[1], dataflash_buff[2]);
        pump_pwm_data = PUMP_PWM_DEFAULT;
        pump_pwm_store(pump_pwm_data);
    }

    return pump_pwm_data;
}

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

    //PWM initialization
    ch552_pwm_init(CH552_PWM1, 255, 0);

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

    uint8_t pump_pwm = pump_pwm_restore();

    uint8_t adc_result = 0;
    uint8_t prev_pump_pwm = 0;
    uint8_t trace_cnt = 0;
    uint32_t delay_time = 0;
    uint32_t pump_time = 0;
    bool pump_enabled = false;
    bool pump_adjust_enabled = false;

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

    printf_tiny("PWM PUMP %d", pump_pwm);
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
        //Get potentiometer's data
        ch552_adc_conv(CH552_ADC_CH3, &adc_result);
        uint32_t val_dv1 = (VCC_IN_DV * adc_result) / 255UL;

        if (val_dv1 <= MIN_DELAY_VAL_DV)
            val_dv1 = MIN_DELAY_VAL_DV;

        if (val_dv1 >= MAX_DELAY_VAL_DV)
            val_dv1 = MAX_DELAY_VAL_DV;

        ch552_adc_conv(CH552_ADC_CH1, &adc_result);
        uint32_t val_dv2 = (VCC_IN_DV * adc_result) / 255UL;

        if (val_dv2 <= MIN_PUMP_VAL_DV)
            val_dv2 = MIN_PUMP_VAL_DV;

        if (val_dv2 >= MAX_PUMP_VAL_DV)
            val_dv2 = MAX_PUMP_VAL_DV;

        //Pump logic
        if (!pump_adjust_enabled) {
            if (val_dv1 == MIN_DELAY_VAL_DV) {
                pump_adjust_enabled = true;
                continue;
            }

            delay_time = (DELAY_MAX_VAL_SEC * (val_dv1 - MIN_DELAY_VAL_DV)) / (MAX_DELAY_VAL_DV - MIN_DELAY_VAL_DV);
            delay_time = DELAY_STEP * (delay_time / DELAY_STEP)  + ((delay_time < DELAY_MAX_VAL_SEC) ? DELAY_STEP : 0);
            pump_time = (PUMP_MAX_VAL_SEC * (val_dv2 - MIN_PUMP_VAL_DV)) / (MAX_PUMP_VAL_DV - MIN_PUMP_VAL_DV);

            uint32_t expired_time = pump_enabled ? pump_time : delay_time;
            if (expired_time <= cnt_sec) {
                TIMER_RESET();
                pump_enabled = !pump_enabled;
                ch552_pwm_change(CH552_PWM1, pump_enabled ? pump_pwm : 0);
            }
        // Pump adjustment
        } else {
            uint8_t cur_pump_pwm = (255UL * (val_dv2 - MIN_PUMP_VAL_DV)) / (MAX_PUMP_VAL_DV - MIN_PUMP_VAL_DV);

            if (cur_pump_pwm != prev_pump_pwm) {
                prev_pump_pwm = cur_pump_pwm;
                ch552_pwm_change(CH552_PWM1, prev_pump_pwm);
            }

            if (val_dv1 == MAX_DELAY_VAL_DV) {
                pump_adjust_enabled = false;

                if (prev_pump_pwm) {
                    pump_pwm = prev_pump_pwm;
                    pump_pwm_store(pump_pwm);
                }

                pump_enabled = false;
                ch552_pwm_change(CH552_PWM1, 0);
                TIMER_RESET();
            }
        }

        if (++trace_cnt >= (TRACE_PERIOD_MS / POLL_PERIOD_MS)) {
            printf_tiny("ADC %d/%d\r\n", (uint16_t)val_dv1, (uint16_t)val_dv2);

            if (!pump_adjust_enabled) {
                printf_tiny("MODE: %s\r\n", pump_enabled ? "PUMP" : "DELAY");
                sec_format_printf("TIME", cnt_sec);
                sec_format_printf("TARGET", pump_enabled ? pump_time : delay_time);
            } else {
                printf_tiny("PWM PUMP %d\r\n", prev_pump_pwm);
            }

            printf_tiny("\r\n");

            trace_cnt = 0;
        }

        ch552_sys_delay_ms(POLL_PERIOD_MS);
    }
}