#include "ch552_sys.h"
#include "ch552.h"
#include "common.h"

static uint32_t cur_sys_clk = 6000000;

void ch552_sys_delay_ms(uint16_t time)
{
    while (time) {
#ifdef CH552_DELAY_MS_HW
        while (!(TKEY_CTRL & bTKC_IF));
        while (TKEY_CTRL & bTKC_IF);
#else
        ch552_sys_delay_us(1000);
#endif
        time--;
    }
}

void ch552_sys_delay_us(uint16_t time)
{
    if (cur_sys_clk <= 750000)
        time /= 16;
    else if (cur_sys_clk <= 3000000)
        time /= 4;
    else if(cur_sys_clk <= 6000000)
        time /= 2;
    else if (cur_sys_clk >= 32000000)
        time = (time * 8) / 3;
    else if (cur_sys_clk >= 30000000)
        time = (time * 15) / 6;
    else if (cur_sys_clk >= 28000000)
        time = (time * 7) / 3;
    else if (cur_sys_clk >= 26000000)
        time = (time * 13) / 6;
    else if (cur_sys_clk >= 24000000)
        time *= 2;
    else if (cur_sys_clk >= 22000000)
        time = (time * 11) / 6;
    else if (cur_sys_clk >= 20000000)
        time = (time * 5) / 3;
    else if (cur_sys_clk >= 18000000)
        time = (time * 3) / 2;
    else if (cur_sys_clk >= 16000000)
        time = (time * 4) / 3;
    else if (cur_sys_clk >= 14000000)
        time = (time * 7) / 6;

    while (time) {  // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
        ++SAFE_MOD;  // 2 Fsys cycles
        time--;
    }
}

uint8_t ch552_sys_clk_set(uint32_t clk_value)
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
//    CLOCK_CFG |= bOSC_EN_XT;      // Enable external crystal
//    CLOCK_CFG & = ~bOSC_EN_INT;   // Turn off the internal crystal

    PCON |= SMOD;

    uint8_t sys_clk_mask = 0x7;

    if (clk_value == 24000000)
        sys_clk_mask = 0x06;
    else if (clk_value == 16000000)
        sys_clk_mask = 0x05;
    else if (clk_value == 12000000)
        sys_clk_mask = 0x04;
    else if (clk_value == 6000000)
        sys_clk_mask = 0x03;
    else if (clk_value == 3000000)
        sys_clk_mask = 0x02;
    else if (clk_value == 750000)
        sys_clk_mask = 0x01;
    else if (clk_value == 187500)
        sys_clk_mask = 0x00;
    else
        return RES_INVALID_PAR;

    cur_sys_clk = clk_value;

    CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | sys_clk_mask;

    SAFE_MOD = 0x00;

    return RES_OK;
}

uint32_t ch552_sys_clk_get(void)
{
    return cur_sys_clk;
}