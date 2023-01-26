#ifndef CH552_ADC_H
#define CH552_ADC_H

#include <stdint.h>
#include <stdbool.h>

#define CH552_ADC_CH0       0
#define CH552_ADC_CH1       1
#define CH552_ADC_CH2       2
#define CH552_ADC_CH3       3

uint8_t ch552_adc_init(void);
uint8_t ch552_adc_conv(uint8_t adc_channel, uint8_t *result);

#endif //CH552_ADC_H