#ifndef GPU_H
#define GPU_H

#include <inttypes.h>

typedef struct GPU_S {
    uint8_t map[64 * 60]; // = ;
    uint8_t attr[32 * 60]; // = ;
    uint8_t tile[256 * 16]; // = ;
    uint32_t palette[4][3]; // = ;
    uint32_t bgColor; // = 0;
} GPU;

uint16_t videoAddressToMapIndex(uint16_t addr);
uint8_t tilePixelFromTileAndVideoAddress(GPU* gpu, uint16_t tileMapIndex, uint16_t addr);
uint8_t tileByteOffsetFromVideoAddress(uint16_t addr);
uint32_t lookUpPixel(GPU* gpu, uint16_t addr);

#endif //GPU_H