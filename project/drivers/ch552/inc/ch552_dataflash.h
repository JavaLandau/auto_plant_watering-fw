#ifndef CH552_DATAFLASH_H
#define CH552_DATAFLASH_H

#include <stdint.h>
#include <stdbool.h>

uint8_t ch552_dataflash_write(uint8_t offset, uint8_t *data, uint8_t len);
uint8_t ch552_dataflash_read(uint8_t offset, uint8_t *data, uint8_t len);

#endif //CH552_DATAFLASH_H