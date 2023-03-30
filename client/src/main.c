#include <stdio.h>
#include "webshim.h"
#include "gpu.h"
#include "module.h"
#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

WS_Display display;

void render(Engine* engine) {

    GPU* gpu = engine->gpu;
    GameModule module = engine->module;

    module.render(engine);

    for(uint16_t addr = 0; addr < (256*240); addr++)
        display.fb[addr] = lookUpPixel(gpu, addr);

    WS_SubmitDisplay(display);
}

int handleEvent(Engine* engine, Event* event) {

    engine->module.event(engine, event);

    return 0;
}

GPU* initGpu() { 

    printf("gpu init\n");
    GPU* gpu = (GPU*)malloc(sizeof(GPU));
    memset(gpu, 0x00, sizeof(GPU));

    return gpu;
};

void startGame(GameModule module) {

    GPU* gpu = initGpu();
    Engine* engine = (Engine*)malloc(sizeof(Engine));

    printf("core gpu->sprites[0]: %d\n", gpu->sprites[0]);

    engine->gpu = gpu;
    engine->module = module;

    module.init(engine);

    display = WS_CreateDisplay(256, 240);

    WS_StartEventDispatch(handleEvent, engine);
    WS_SetRenderLoopProc(render);
    WS_StartRenderLoop(engine);
}

int main(int argc, char* argv[]) {

    loadModule("game.so", startGame); 
}
