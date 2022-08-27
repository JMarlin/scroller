#include "webshim.h"
#include "gpu.h"

WS_Display display;

void render(GPU* gpu) {
    static GPU gpu = { 0 };

    for(uint16_t addr = 0; addr < (256*240); addr++)
        display.fb[addr] = lookUpPixel(&gpu, addr);

    WS_SubmitDisplay(display);
}

int main(int argc, char* argv[]) {
    display = WS_CreateDisplay(256, 240);
    WS_SetRenderLoopProc(render);
    WS_StartRenderLoop();
}