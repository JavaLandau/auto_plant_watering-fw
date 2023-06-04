#include "ch552.h"
#include "ch552_pwm.h"
#include "common.h"

//No remaping
uint8_t ch552_pwm_init(uint8_t pwm_num, uint8_t div_clk, uint8_t polarity)
{
    //PWM1 supported only
    if (pwm_num != CH552_PWM1)
        return RES_INVALID_PAR;

    PWM_CK_SE = div_clk;
    PWM_CTRL |= bPWM_CLR_ALL;
    PWM_CTRL &= ~bPWM_CLR_ALL;

    if (polarity)
        PWM_CTRL |= bPWM1_POLAR;
    else
        PWM_CTRL &= ~bPWM1_POLAR;

    PWM_DATA1 = 0;
    PWM_CTRL |= bPWM1_OUT_EN;

    return RES_OK;
}

uint8_t ch552_pwm_change(uint8_t pwm_num, uint8_t pwm)
{
    //PWM1 supported only
    if (pwm_num != CH552_PWM1)
        return RES_INVALID_PAR;

    PWM_DATA1 = pwm;

    return RES_OK;
}
