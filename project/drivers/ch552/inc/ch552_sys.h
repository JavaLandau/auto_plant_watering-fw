#ifndef CH552_SYS_H
#define CH552_SYS_H

#include <stdint.h>

void ch552_sys_delay_ms(uint16_t time);
void ch552_sys_delay_us(uint16_t time);
uint8_t ch552_sys_clk_set(uint32_t clk_value);

#endif //CH552_SYS_H