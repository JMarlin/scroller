#include "gpu.h"

uint16_t videoAddressToMapIndex(uint16_t addr) {
#warning TODO: Add x and y scrolling
    return
        (addr >> 5) & 0xFFC0 |
        (addr >> 3) & 0x001F;
}

uint8_t tileByteOffsetFromCoordinatesAndAttribute(uint8_t x, uint8_t y, uint8_t attribute) {
#warning TODO: Add x and y scrolling
    uint8_t tileXHalf = ((x & 0x0004) >> 2) ^ (0x01 & ATTR_HMIRRORED(attribute));
    uint8_t tileYOffset = ((y & 0x0007) ^ (0x07 & ATTR_VMIRRORED(attribute))) << 1;

    return tileYOffset | tileXHalf;
}

uint16_t tileBaseOffsetFromTileIndex(GPU* gpu, uint16_t tileMapIndex) {
    return ((uint16_t)gpu->map[tileMapIndex]) << 4;
}

uint8_t attributeFromTileMapIndex(GPU* gpu, uint16_t tileMapIndex) {
    return (gpu->attr[tileMapIndex >> 1] >> (tileMapIndex & 0x0001)) & 0x0F;
}

uint8_t tileByteFromTileMapIndexCoordinatesAndAttribute(GPU* gpu, uint16_t tileMapIndex, uint8_t x, uint8_t y, uint8_t attribute) {
    return gpu->tile[tileBaseOffsetFromTileIndex(gpu, tileMapIndex) + tileByteOffsetFromCoordinatesAndAttribute(x, y, attribute)];
}

uint8_t tilePixelPaletteIndexFromTileByteAddressAndAttribute(uint8_t tileByte, uint16_t addr, uint8_t attribute) {
    uint8_t tileXSubAddr = (((uint8_t)(addr & 0x0003)) ^ (0x03 & ATTR_HMIRRORED(attribute))) << 1;

    return (tileByte >> (6 - tileXSubAddr)) & 0x03;
}

uint32_t pixelColorFromPaletteIndexAndAttribute(GPU* gpu, uint8_t tilePixelPaletteIndex, uint8_t attribute) {
    return tilePixelPaletteIndex
        ? gpu->palette[ATTR_PALETTE(attribute)][tilePixelPaletteIndex - 1]
        : gpu->bgColor;
}

uint32_t tilePixelIndexFromTileAndVideoAddress(GPU* gpu, uint16_t tileMapIndex, uint16_t addr) {
    uint8_t attribute = attributeFromTileMapIndex(gpu, tileMapIndex);
    uint8_t tileByte = tileByteFromTileMapIndexCoordinatesAndAttribute(gpu, tileMapIndex, VIDEO_ADDR_XPOS(addr), VIDEO_ADDR_YPOS(addr), attribute);

    return tilePixelPaletteIndexFromTileByteAddressAndAttribute(tileByte, addr, attribute);
}

uint8_t videoAddressToSpriteIndex(GPU* gpu, int* spriteFound, uint16_t addr) {
    *spriteFound = 1;

    uint8_t xpos = VIDEO_ADDR_XPOS(addr);
    uint8_t ypos = VIDEO_ADDR_YPOS(addr);

    //TODO: Support multi-tile sprites
    for(uint8_t i = 0; i < 32; i++) if(
        xpos >= SPRITE_X(gpu->sprites[i]) &&
        xpos < (SPRITE_X(gpu->sprites[i]) + 8) &&
        ypos >= SPRITE_Y(gpu->sprites[i]) &&
        ypos < (SPRITE_Y(gpu->sprites[i]) + 8)
    ) return i;

    *spriteFound = 0;

    return 0;
}

uint8_t spritePixelIndexFromIndexAndVideoAddress(GPU* gpu, uint8_t spriteIndex, uint16_t addr) {
    
    uint8_t tileByte = tileByteFromTileMapIndexCoordinatesAndAttribute(
        gpu,
        SPRITE_TILE_INDEX(gpu->sprites[spriteIndex]),
        VIDEO_ADDR_XPOS(addr) - SPRITE_X(gpu->sprites[spriteIndex]),
        VIDEO_ADDR_YPOS(addr) - SPRITE_Y(gpu->sprites[spriteIndex]),
        SPRITE_ATTR(gpu->sprites[spriteIndex])
    );
}

uint32_t lookUpPixel(GPU* gpu, uint16_t addr) {
    int spriteFound;
    uint8_t spriteIndex = videoAddressToSpriteIndex(gpu, &spriteFound, addr);

    uint16_t pixelIndex = spriteFound
        ? spritePixelIndexFromIndexAndVideoAddress(gpu, spriteIndex, addr)
        : 0;

    int isSpritePixel = pixelIndex;

    uint16_t tileMapIndex = isSpritePixel
        ? 0
        : videoAddressToMapIndex(addr);

    pixelIndex = pixelIndex
        ? pixelIndex
        : tilePixelIndexFromTileAndVideoAddress(gpu, tileMapIndex, addr);

    return pixelColorFromIndexAndAttribute(
        gpu,
        pixelIndex,
        isSpritePixel
            ? SPRITE_ATTR(gpu->sprites[spriteIndex])
            : attributeFromTileMapIndex(gpu, tileMapIndex));
}