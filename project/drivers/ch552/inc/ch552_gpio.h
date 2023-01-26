#ifndef CH552_GPIO_H
#define CH552_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ch552.h"

#define GPIO_PIN_0      0
#define GPIO_PIN_1      1
#define GPIO_PIN_2      2
#define GPIO_PIN_3      3
#define GPIO_PIN_4      4
#define GPIO_PIN_5      5
#define GPIO_PIN_6      6
#define GPIO_PIN_7      7

#define GPIO_PORT_P1    BYTE_P1
//#define GPIO_PORT_P2    BYTE_P2
#define GPIO_PORT_P3    BYTE_P3

enum ch552_gpio_type {
    CH552_GPIO_INPUT = 0,
    CH552_GPIO_PP_OUTPUT = 1,
    CH552_GPIO_OD_OUTPUT = 2,
    CH552_GPIO_QUASI_BIDIR = 3,
    CH552_GPIO_TYPE_MAX = 4
};

uint8_t ch552_gpio_init(const uint8_t gpio_port, const uint8_t gpio_num, const enum ch552_gpio_type type);
uint8_t ch552_gpio_set(const uint8_t gpio_port, const uint8_t gpio_num, const uint8_t level);


#endif //CH552_GPIO_H