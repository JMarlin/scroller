#include "gpu.h"

uint16_t videoAddressToMapIndex(uint16_t addr) {
#warning TODO: Add x and y scrolling
    return
        (addr >> 5) & 0xFFC0 |
        (addr >> 3) & 0x001F;
}

uint8_t tileByteOffsetFromVideoAddress(uint16_t addr) {
#warning TODO: Add x and y scrolling
    int tileXHalf = (addr & 0x0004) >> 2;
    int tileYOffset = ((addr >> 8) & 0x0007) << 1;

    return tileYOffset | tileXHalf;
}

uint16_t tileBaseOffsetFromTileIndex(GPU* gpu, uint16_t tileMapIndex) {
    return ((uint16_t)gpu->map[tileMapIndex]) << 4;
}

uint8_t tilePixelFromTileAndVideoAddress(GPU* gpu, uint16_t tileMapIndex, uint16_t addr) {
    int tileXSubAddr = addr & 0x0003;
    uint8_t tileByte = gpu->tile[tileBaseOffsetFromTileIndex(gpu, tileMapIndex) + tileByteOffsetFromVideoAddress(addr)];
    uint8_t tilePixelValue = (tileByte << (3 - tileXSubAddr)) & 0x03;

    return tilePixelValue;
}

uint32_t lookUpPixel(GPU* gpu, uint16_t addr) {
    uint16_t row = addr >> 8;
    uint16_t col = addr & 0xFF;

    uint16_t tileMapIndex = videoAddressToMapIndex(addr);
    uint8_t tilePixelValue = tilePixelFromTileAndVideoAddress(gpu, tileMapIndex, addr);

    //Look up attribute cell on attribute map
#warning TODO: Support h-flip attribute
#warning TODO: Support v-flip attribute
    uint16_t attrAddress = ((addr >> 3) & 0xFFE0) | ((addr >> 1) & 0x001F);
    uint8_t attrValue = (gpu->attr[attrAddress] >> (addr >> 1)) & 0x0F;
}