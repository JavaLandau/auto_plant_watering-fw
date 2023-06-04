#include "ch552.h"
#include "common.h"
#include "ch552_dataflash.h"

uint8_t ch552_dataflash_write(uint8_t offset, uint8_t *data, uint8_t len)
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;

    GLOBAL_CFG |= bCODE_WE | bDATA_WE;

    for (uint8_t i = 0; i < len; i++) {
        uint16_t addr = 0xC000 | ((offset + i) << 1);
        ROM_ADDR = addr;

        ROM_DATA_L = data[i];
        ROM_CTRL = 0x9A;
    }

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;

    GLOBAL_CFG &= (!bCODE_WE & ~bDATA_WE);

    SAFE_MOD = 0x00;

    return RES_OK;
}

uint8_t ch552_dataflash_read(uint8_t offset, uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        uint16_t addr = 0xC000 | ((offset + i) << 1);
        ROM_ADDR = addr;

        ROM_CTRL = 0x8E;
        data[i] = ROM_DATA_L;
    }

    return RES_OK;
}