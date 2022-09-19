#ifndef GPU_H
#define GPU_H

#include <inttypes.h>

#define ATTR_PALETTE(a) ((a) & 0x03)
#define ATTR_HMIRRORED(a) (((a) & 0x04) ? 0xFF : 0x00)
#define ATTR_VMIRRORED(a) (((a) & 0x08) ? 0xFF : 0x00)

#define SPRITE_X(s) ((uint8_t)(((s) >> 24) & 0xFF))
#define SPRITE_Y(s) ((uint8_t)(((s) >> 16) & 0xFF))
#define SPRITE_TILE_INDEX(s) ((uint8_t)(((s) >> 8) & 0xFF))
#define SPRITE_ATTR(s) ((uint8_t)((s) & 0xFF))

#define VIDEO_ADDR_XPOS(a) ((uint8_t)((a) & 0xFF))
#define VIDEO_ADDR_YPOS(a) ((uint8_t)(((a) >> 8) & 0XFF))

typedef struct GPU_S {
    uint8_t map[64 * 60]; 
    uint8_t attr[32 * 60]; 
    uint8_t tile[256 * 16]; 
    uint32_t palette[4][3]; 
    uint32_t sprites[32];
    uint32_t bgColor; 
} GPU;

uint16_t videoAddressToMapIndex(uint16_t addr);
uint8_t tileByteOffsetFromCoordinatesAndAttribute(uint8_t x, uint8_t y, uint8_t attribute);
uint16_t tileBaseOffsetFromTileIndex(GPU* gpu, uint16_t tileMapIndex);
uint8_t attributeFromTileMapIndex(GPU* gpu, uint16_t tileMapIndex);
uint8_t tileByteFromTileMapIndexCoordinatesAndAttribute(GPU* gpu, uint16_t tileMapIndex, uint8_t x, uint8_t y, uint8_t attribute);
uint8_t tilePixelPaletteIndexFromTileByteCoordinatesAndAttribute(uint8_t tileByte, uint8_t x, uint8_t y, uint8_t attribute);
uint32_t pixelColorFromPaletteIndexAndAttribute(GPU* gpu, uint8_t tilePixelPaletteIndex, uint8_t attribute);
uint32_t tilePixelColorFromTileAndVideoAddress(GPU* gpu, uint16_t tileMapIndex, uint16_t addr);
uint32_t lookUpPixel(GPU* gpu, uint16_t addr);

#endif //GPU_H