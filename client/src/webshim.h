#ifndef WEBSHIM_H
#define WEBSHIM_H

#include <inttypes.h>
#include "event.h"

typedef void (*RenderProcedure)(void* payload);
typedef int (*EventDispatchProcedure)(void* payload, Event* event);

typedef struct S_WS_Display {
    int32_t w;
    int32_t h;
    uint32_t* fb;
    int id;
} WS_Display;

void WS_InitEvents();
void WS_SetRenderLoopProc(RenderProcedure render_proc);
void WS_StopRenderLoop();
void WS_StartRenderLoop(void* payload);
void WS_StartEventDispatch(EventDispatchProcedure dispatch_proc, void* payload);
WS_Display WS_CreateDisplay(uint32_t w, uint32_t h);
void WS_DestroyDisplay(WS_Display display);
void WS_SubmitDisplay(WS_Display display);
void WS_ClearDisplay(WS_Display display);
void WS_UpdateDisplay(WS_Display display);

#endif //WEBSHIM_H
