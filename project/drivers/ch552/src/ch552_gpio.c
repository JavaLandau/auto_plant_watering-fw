#include "ch552_gpio.h"
#include "ch552.h"
#include "common.h"

uint8_t ch552_gpio_init(const uint8_t gpio_port, const uint8_t gpio_num, const enum ch552_gpio_type type)
{
    switch (gpio_port) {
    case GPIO_PORT_P1:
        P1_DIR_PU &= ~(0x1 << gpio_num);
        P1_MOD_OC &= ~(0x1 << gpio_num);
        P1_DIR_PU |= ((type & 0x01) << gpio_num);
        P1_MOD_OC |= (((type >> 1) & 0x01) << gpio_num);

        break;

    //P2 is not present in CH552

    case GPIO_PORT_P3:
        P3_DIR_PU &= ~(0x1 << gpio_num);
        P3_MOD_OC &= ~(0x1 << gpio_num);
        P3_DIR_PU |= ((type & 0x01) << gpio_num);
        P3_MOD_OC |= (((type >> 1) & 0x01) << gpio_num);

        break;

    default:
        return RES_INVALID_PAR;
    }


    return RES_OK;
}

uint8_t ch552_gpio_set(const uint8_t gpio_port, const uint8_t gpio_num, const uint8_t level)
{
    switch (gpio_port) {
    case GPIO_PORT_P1:
        if (!level)
            P1 &= ~(1 << gpio_num);
        else
            P1 |= (1 << gpio_num);

        break;

    //P2 is not present in CH552

    case GPIO_PORT_P3:
        if (!level)
            P3 &= ~(1 << gpio_num);
        else
            P3 |= (1 << gpio_num);

        break;

    default:
        return RES_INVALID_PAR;
    }

    return RES_OK;
}