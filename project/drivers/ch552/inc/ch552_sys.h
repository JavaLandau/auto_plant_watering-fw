#ifndef CH552_SYS_H
#define CH552_SYS_H

#include <stdint.h>

void ch552_sys_delay_ms(uint16_t time);
void ch552_sys_delay_us(uint16_t time);
uint8_t ch552_sys_clk_set(uint32_t clk_value);
uint32_t ch552_sys_clk_get(void);

#define CH552_INTERRUPTS_DISABLE() \
        do {\
            E_DIS = 1;\
        } while(0)

#define CH552_INTERRUPTS_ENABLE() \
        do {\
            E_DIS = 0;\
        } while(0)

#define CH552_SYS_INT_INIT() \
        do {\
            EA = 1;\
        } while(0)

#endif //CH552_SYS_H