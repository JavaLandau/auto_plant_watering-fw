#include "ch552.h"
#include "ch552_sys.h"

#define LED_PIN 7
SBIT(LED, 0x90, LED_PIN);

void main() {
    // Configure pin 1.7 as GPIO output
    P1_DIR_PU &= 0x0C;
    P1_MOD_OC &= (~(1<<LED_PIN));
    P1_DIR_PU |= (1<<LED_PIN);

    while (1) {
        ch552_sys_delay_ms(500);
        LED = !LED;
    }
}