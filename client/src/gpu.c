#include "gpu.h"

uint16_t videoAddressToMapIndex(uint16_t addr) {
#warning TODO: Add x and y scrolling
    return
        (addr >> 5) & 0xFFC0 |
        (addr >> 3) & 0x001F;
}

uint8_t tileByteOffsetFromVideoAddressAndAttribute(uint16_t addr, uint8_t attribute) {
#warning TODO: Add x and y scrolling
    uint8_t tileXHalf = ((addr & 0x0004) >> 2) ^ (0x01 & ATTR_HMIRRORED(attribute));
    uint8_t tileYOffset = (((addr >> 3) & 0x0007) ^ (0x07 & ATTR_VMIRRORED(attribute))) << 1;

    return tileYOffset | tileXHalf;
}

uint16_t tileBaseOffsetFromTileIndex(GPU* gpu, uint16_t tileMapIndex) {
    return ((uint16_t)gpu->map[tileMapIndex]) << 4;
}

uint8_t attributeFromTileMapIndex(GPU* gpu, uint16_t tileMapIndex) {
    return (gpu->attr[tileMapIndex >> 1] >> (tileMapIndex & 0x0001)) & 0x0F;
}

uint8_t tileByteFromTileMapIndexAddressAndAttribute(GPU* gpu, uint16_t tileMapIndex, uint16_t addr, uint8_t attribute) {
    return gpu->tile[tileBaseOffsetFromTileIndex(gpu, tileMapIndex) + tileByteOffsetFromVideoAddressAndAttribute(addr, attribute)];
}
uint8_t tilePixelPaletteIndexFromTileByteAddressAndAttribute(uint8_t tileByte, uint16_t addr, uint8_t attribute) {
    uint8_t tileXSubAddr = (((uint8_t)(addr & 0x0003)) ^ (0x03 & ATTR_HMIRRORED(attribute))) << 1;

    return (tileByte >> (6 - tileXSubAddr)) & 0x03;
}

uint8_t pixelColorFromPaletteIndexAndAttribute(GPU* gpu, uint8_t tilePixelPaletteIndex, uint8_t attribute) {
    return tilePixelPaletteIndex
        ? gpu->palette[ATTR_PALETTE(attribute)][tilePixelPaletteIndex - 1]
        : gpu->bgColor;
}

uint8_t tilePixelFromTileAndVideoAddress(GPU* gpu, uint16_t tileMapIndex, uint16_t addr) {
    uint8_t attribute = attributeFromTileMapIndex(gpu, tileMapIndex);
    uint8_t tileByte = tileByteFromTileMapIndexAddressAndAttribute(gpu, tileMapIndex, addr, attribute);
    uint8_t tilePixelPaletteIndex = tilePixelPaletteIndexFromTileByteAddressAndAttribute(tileByte, addr, attribute);

    return pixelColorFromPaletteIndexAndAttribute(gpu, tilePixelPaletteIndex, attribute);
}

uint32_t lookUpPixel(GPU* gpu, uint16_t addr) {
    uint16_t tileMapIndex = videoAddressToMapIndex(addr);

    return tilePixelColorFromTileAndVideoAddress(gpu, tileMapIndex, addr);
}