#ifndef CH552_PWM_H
#define CH552_PWM_H

#include <stdint.h>
#include <stdbool.h>

#define CH552_PWM1          1
#define CH552_PWM2          2
#define CH552_PWM_COUNT     2

uint8_t ch552_pwm_init(uint8_t pwm_num, uint8_t div_clk, uint8_t polarity);
uint8_t ch552_pwm_change(uint8_t pwm_num, uint8_t pwm);

#endif //CH552_PWM_H