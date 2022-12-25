#include <stdio.h>
#include "webshim.h"
#include "gpu.h"
#include "module.h"
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

WS_Display display;

typedef struct GameAndEngine_S {
    GPU* gpu;
    GameModule module;
} GameAndEngine;

void render(void* payload) {
    GameAndEngine* gae = (GameAndEngine*)payload;
    GPU* gpu = gae->gpu;
    GameModule module = gae->module;

    module.render(gpu);

    for(uint16_t addr = 0; addr < (256*240); addr++)
        display.fb[addr] = lookUpPixel(gpu, addr);

    WS_SubmitDisplay(display);
}

int handleEvent(void* payload, Event* event) {
    GameAndEngine* gae = (GameAndEngine*)payload;

    gae->module.event(gae->gpu, event);

    return 0;
}

typedef void (*GameDispatchFunction)(GPU*, int cmd, uint32_t a, uint32_t b);

GPU* initGpu() { 
    printf("gpu init\n");
    GPU* gpu = (GPU*)malloc(sizeof(GPU));
    memset(gpu, 0x00, sizeof(GPU));

    return gpu;
};

void startGame(GameModule module) {
    GPU* gpu = initGpu();
    GameAndEngine* gameAndEngine = (GameAndEngine*)malloc(sizeof(GameAndEngine));
    printf("core gpu->sprites[0]: %d\n", gpu->sprites[0]);
    module.init(gpu);
    gameAndEngine->gpu = gpu;
    gameAndEngine->module = module;
    display = WS_CreateDisplay(256, 240);
    WS_StartEventDispatch(handleEvent, (void*)gameAndEngine);
    WS_SetRenderLoopProc(render);
    WS_StartRenderLoop((void*)gameAndEngine);
}

int main(int argc, char* argv[]) {
    loadModule("game.so", startGame); 
}
