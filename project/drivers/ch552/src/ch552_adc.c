#include "ch552_adc.h"
#include "ch552.h"
#include "common.h"

uint8_t ch552_adc_init(void)
{
    ADC_CFG = (ADC_CFG & ~bCMP_EN & ~bADC_CLK) | bADC_EN;
    return RES_OK;
}

uint8_t ch552_adc_conv(uint8_t adc_channel, uint8_t *result)
{
    ADC_CHAN0 = adc_channel & 0x01;
    ADC_CHAN1 = (adc_channel >> 1) & 0x01;

    ADC_START = 1;
    while(ADC_START);

    *result = ADC_DATA;

    return RES_OK;
}