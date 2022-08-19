#include "webshim.h"

WS_Display display;

void render() {
    for(int i = 0; i < (320 * 240 * 4); i += 4) {
        display.fb[i + 0] = display.fb[i + 0] + 1;
        display.fb[i + 1] = display.fb[i + 1] + 1;
        display.fb[i + 2] = display.fb[i + 2] + 1;
        display.fb[i + 3] = 0xFF;
    }

    WS_SubmitDisplay(display);
}

int main(int argc, char* argv[]) {
    display = WS_CreateDisplay(320, 240);
    WS_SetRenderLoopProc(render);
    WS_StartRenderLoop();
}