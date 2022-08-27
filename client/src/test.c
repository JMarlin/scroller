#include "gpu.h"
#include <stdio.h>
#include <string.h>

#define XY2ADDR(x, y) ((y * 256) + x)

typedef struct PixelRenderCase_S {
    uint8_t pixelCoordIn[2];
    uint16_t expectedValue;
    int valid;
} PixelRenderCase;

#define PRC(x, y, e) { \
    .pixelCoordIn = { (x), (y) }, \
    .expectedValue = (e), \
    .valid = 1 \
} \

const PixelRenderCase tileLookupRenderCases[] = {
    PRC(0, 0, 0x0000),
    PRC(7, 0, 0x0000),
    PRC(0, 7, 0x0000),
    PRC(8, 0, 0x0001),
    PRC(0, 8, 0x0040),
    PRC(8, 8, 0x0041),
    PRC(256 - 8, 0, 0x001F),
    { 0 }
};

const PixelRenderCase tileByteLookupRenderCases[] = {
    PRC(0, 0, 0x0000),
    PRC(1, 0, 0x0000),
    PRC(0, 1, 0x0002),
    PRC(4, 0, 0x0001),
    { 0 }
};

typedef uint16_t (*GPUTestMethod)(GPU* gpu, uint16_t addr);

void processGpuTestCase(GPU* gpu, const char* testName, GPUTestMethod testMethod, PixelRenderCase* pixelRenderCase) {
    uint16_t addr = XY2ADDR(pixelRenderCase->pixelCoordIn[0], pixelRenderCase->pixelCoordIn[1]);
    uint16_t result = testMethod(gpu, addr);

    printf(
        "%s test:\n"
        "    Input coords: (%d, %d) [0x%04x]\n"
        "    Result: 0x%04x (expected 0x%04x) %s\n",
        testName,
        pixelRenderCase->pixelCoordIn[0], pixelRenderCase->pixelCoordIn[1], addr,
        result, pixelRenderCase->expectedValue, pixelRenderCase->expectedValue == result ? "success" : "FAIL"
    );
}

void processGpuTestCases(GPU* gpu, const char* testName, GPUTestMethod testMethod, const PixelRenderCase* pixelRenderCase) {
    for(
        ;
        pixelRenderCase->valid;
        pixelRenderCase++
    ) processGpuTestCase(gpu, testName, testMethod, pixelRenderCase);
}

uint16_t doMapLookupTest(GPU* gpu, uint16_t addr) {
    return videoAddressToMapIndex(addr);
}

void doMapLookupTests(GPU* gpu) {
    processGpuTestCases(gpu, "Map lookup", doMapLookupTest, tileLookupRenderCases);
}

uint16_t doTileByteLookupTest(GPU* gpu, uint16_t addr) {
    return tileByteOffsetFromVideoAddress(addr);
}

void doTileByteLookupTests(GPU* gpu) {
    processGpuTestCases(gpu, "Tile pixel lookup", doTileByteLookupTest, tileByteLookupRenderCases);
}

void initializeTestGPU(GPU* gpu) {
    memset(gpu->map, 0xAA, 64 * 30);
    memset(gpu->tile, 0x55, 16 * 256);
}

int main(int argc, char* argv[]) {
    GPU gpu = { 0 };
    initializeTestGPU(&gpu);
    doMapLookupTests(&gpu);
    doTileByteLookupTests(&gpu);
    //doAttributeLookupTests();

    return 0;
}
